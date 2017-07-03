// RequestHandler.cpp

#include "RequestHandler.h"

#include "Segment.h"
#include "ProcessBase.h"
#include "Transactions.h"
#include "psosxml.h"
#include "StateMachine.h"
#include "Trace.h"
#include "TaskApi.h"
#include "XmlDefines.h"
#include "Request.h"
#include "OpcodesMcmsCommon.h"
#include "SerializeObject.h"
#include "XmlApi.h"
#include "InitCommonStrings.h"
#include "StatusesGeneral.h"
#include "ApacheDefines.h"
#include "RsrvManagerApi.h"
#include "ResourceManagerApi.h"
#include "ConfPartyManagerApi.h"
#include "InternalProcessStatuses.h"
#include "ApiStatuses.h"
#include "ZipWrapper.h"
#include "IConv.h"
#include "ObjString.h"
#include "UnicodeDefines.h"
#include "NStream.h"
#include "EncodingConvertor.h"
#include "PostXmlHeader.h"
#include "TraceStream.h"
#include "AuditorApi.h"
#include <typeinfo>

CRequestHandler::CRequestHandler()
{}

CRequestHandler::~CRequestHandler()
{}

void CRequestHandler::HandlePostRequest(CSegment* pSeg)
{
    eProcessType processType = CProcessBase::GetProcess()->GetProcessType();
    const char *processName = CProcessBase::GetProcessName(processType);

    STATUS status = STATUS_OK;
    COstrStream statusString;

	// 1. read return mailbox, len and buffer from segment
	COsQueue dualMbx; // mailbox of the dual task (Tx)
	dualMbx.DeSerialize(*pSeg);
    
    m_CurrentMsgHdr.DeSerialize(*pSeg);

    DWORD len = m_CurrentMsgHdr.GetLen();
    char * pContent = new char[len + 1];
    pSeg->Get((BYTE*)pContent, len);
    pContent[len] = '\0';

    bool bCompressed = m_CurrentMsgHdr.GetIsCompressed();
    WORD nAuthorization = m_CurrentMsgHdr.GetAuthorization();
    const string & strCharSet = m_CurrentMsgHdr.GetEncodingCharset();
    DWORD ifNoneMatch = m_CurrentMsgHdr.GetIfNoneMatch();

	ReceiveAdditionalParams(pSeg);
	
	if (bCompressed)
	{
		CZipWrapper ZipWrapper;

        const int pszUnZippedBufferLen = (/*MAX_NON_ZIPPED*/len * 100) + 1;
		char* pszUnZippedBuffer = new char[pszUnZippedBufferLen];
		int nUnZippedLen = ZipWrapper.Inflate((unsigned char*)pContent, len,
                                              (unsigned char*)pszUnZippedBuffer, pszUnZippedBufferLen);
		if(nUnZippedLen <= 0)
		{
			status = STATUS_DECOMPRESSION_FAILED;
			delete [] pszUnZippedBuffer;
		}
		else
		{
			DEALLOCBUFFER(pContent);
			pContent = pszUnZippedBuffer;
            pContent[nUnZippedLen - 1] = '\0';
		}
	}
    
    //to UTF-8
    string encodingTypeFrom = strCharSet;
    
    PASSERTMSG(processType != eProcessResource &&
               processType != eProcessAuthentication &&
               processType != eProcessApacheModule &&
               strCharSet.empty(), "strCharSet is empty");

    bool isNeedToConvert = true;
    if(encodingTypeFrom.empty())
    {
        // means that it is internal transaction, no need for convertion,
        // it was done before.
        isNeedToConvert = false;
        encodingTypeFrom = MCMS_INTERNAL_STRING_ENCODE_TYPE;
    }
    
    if(STATUS_OK == status && isNeedToConvert)
    {
        status = CEncodingConvertor::ConvertValidate(MCMS_INTERNAL_STRING_ENCODE_TYPE,
                                                     encodingTypeFrom,
                                                     pContent,
                                                     statusString);
    }

    CXMLDOMElement * pXmlResponse    = NULL;
    
	if (STATUS_OK == status)
    {
        CXMLDOMDocument* pDom = new CXMLDOMDocument;
        int resParse = pDom->Parse((const char **)&pContent);
        if(SEC_OK == resParse)
        {
            status = ParseHandleTransCommon(pDom,
                                            dualMbx,
                                            nAuthorization,
                                            ifNoneMatch,
                                            encodingTypeFrom,
                                            pXmlResponse,
                                            statusString);
        }
        else
        {
            status = STATUS_TRANSACTION_IS_NOT_WELL_FORMED;
        }

        PDELETE(pDom);
    }
    
	if (STATUS_OK != status)
	{
        const char * tmp_status3 =
            CProcessBase::GetProcess()->GetStatusAsString(status).c_str();
        
		pXmlResponse=::BuildGeneralResponse(status,
											tmp_status3,
											statusString.str().c_str(),
											0,
											UNKNOWN_REQUEST,
											0,
											0);
	}
	
	if (NULL != pXmlResponse)
	{
        char *buffer   = NULL;
        DWORD len       = 0;

        CXMLDOMDocument doc(pXmlResponse, encodingTypeFrom);
        doc.DumpDataAsLongStringEx(&buffer,&len);
        
        status = ConvertToPreviousEncoding(buffer,
                                           encodingTypeFrom,
                                           statusString);
        
		STATUS stat = PostXml(buffer, len, dualMbx);
		if( STATUS_OK != stat )
		{
		   PTRACE2INT(eLevelInfoNormal,
                      "CRequestHandler::HandlePostRequest - PostXml for response failed, status = ", stat);
		}

		PDELETEA(buffer);     
	}
	
	ResetAdditionalParams();
    
	PDELETEA(pContent);
}


/////////////////////////////////////////////////////////////////////////////
CXMLDOMElement* CRequestHandler::HandleRequest(CXMLDOMElement* pXMLRequest,
                                               COsQueue &q,
                                               WORD nAuthorization,DWORD ifNoneMatch,
                                               const string &encodingTypeFrom)
{
	CRequest *pRequest = new CRequest(pXMLRequest, m_requestTransactionsFactory);
	PASSERT_AND_RETURN_VALUE(!pRequest, NULL);


    CProcessBase* pProcess = CProcessBase::GetProcess();
	CTaskApp* pTaskApp = pProcess->GetCurrentTask();
	
	if(pTaskApp == NULL)
	{
		PTRACE(eLevelInfoNormal,"CRequestHandler::HandleRequest pTaskApp is NULL!!");
		PDELETE(pRequest);

		return NULL;
	}

    WORD nAction = SET_REQUEST;
	if(strcmp(pTaskApp->GetTaskName(),"Monitor") == 0)
		nAction = GET_REQUEST;
		
	if (SET_REQUEST == nAction)
	{
		bool isFailoverEnabled	= pProcess->GetIsFailoverFeatureEnabled(),
			 isFailoverSlave	= pProcess->GetIsFailoverSlaveMode();

		if (isFailoverEnabled && isFailoverSlave)
		{
			string sAction = pRequest->GetActionName();

			bool isBlocked = pProcess->IsFailoverBlockTransaction_SlaveMode(sAction);
			if (isBlocked)
			{
				TRACESTR(eLevelInfoNormal) << "\nCRequestHandler::HandleRequest"
									   << "\nAction: " << sAction << " is blocked in Failover Slave mode";
				
				pRequest->SetStatus(STATUS_ACTION_ILLEGAL_AT_FAILOVER_SLAVE_MODE);
			}
		}
	}
		
	eProcessType processType;
	
	if (CPObject::IsValidPObjectPtr(pProcess)) 
	{
        processType = pProcess->GetProcessType();
        if ((eProcessConfParty == processType) && (SET_REQUEST == nAction))
        {
            eMcuState systemState = pProcess->GetSystemState();
            if(eMcuState_Startup == systemState)
            {
                TRACESTR(eLevelInfoNormal) << "\nCRequestHandler::HandleRequest return STATUS_ACTION_ILLEGAL_AT_SYSTEM_STARTUP";


                pRequest->SetStatus(STATUS_ACTION_ILLEGAL_AT_SYSTEM_STARTUP);
            }
        }
	}	
	
	pRequest->SetQueue(q);
	pRequest->SetAuthorization(nAuthorization);
	pRequest->SetIfNoneMatch(ifNoneMatch);
	CXMLDOMElement *pXmlResponse = NULL;
	STATUS stat = STATUS_OK;
    
	if (pRequest->GetStatus() != STATUS_OK)
	{
        const char* tmp_status1 =
            CProcessBase::GetProcess()->GetStatusAsString(pRequest->GetStatus()).c_str();
        
		pXmlResponse = ::BuildGeneralResponse(pRequest->GetStatus(),
											  tmp_status1,
								              pRequest->GetExDescription(),
                                              pRequest->GetHdr(),
                                              nAction,
                                              0,
                                              0);		
	}
	else
	{
		CSerializeObject* pObj = pRequest->GetRequestObject();
		if(pObj)
		{
			HANDLE_REQUEST function = pObj->m_pRequestfunction;

			stat = (this->*(function))(pRequest);

			if ((stat != STATUS_FW_REQUEST_TO_RESOURCE) &&  (stat!= STATUS_FW_REQUEST_TO_CONFPARTY) && (stat != STATUS_WAIT_FOR_AN_ANSWER_FROM_EXTERNAL_DB)
					&&( stat != STATUS_FW_REQUEST_TO_RESOURCE_MANGER))
			{
				if (stat == STATUS_OK)
				{
					pXmlResponse = pRequest->SetConfirmSerializeXml(m_CurrentMsgHdr.GetUserName());
				}
				else
				{
					const char* tmp_status =
							CProcessBase::GetProcess()->GetStatusAsString(pRequest->GetStatus()).c_str();

					pXmlResponse = ::BuildGeneralResponse(pRequest->GetStatus(),
							tmp_status,
							pRequest->GetExDescription(),
							pRequest->GetHdr(),
							nAction,
							0,
							0);
				}
			}
		}
	}

	if (stat == STATUS_FW_REQUEST_TO_RESOURCE)
	{	
		CRsrvManagerApi rsrvManagerApi;
		const COsQueue rservQueue = rsrvManagerApi.GetRcvMbx();

		pXMLRequest = pRequest->SerializeXml();
		char *buffer   = NULL;
		DWORD len       = 0;
		
		CXMLDOMDocument doc(pXMLRequest, encodingTypeFrom);
		doc.DumpDataAsLongStringEx(&buffer, &len);

		PostXml(buffer,len,(COsQueue&)rservQueue,&q,XML_REQUEST,nAuthorization,true);

		PDELETEA(buffer);
		pXMLRequest = NULL;			// it will be destroyed inside doc's dtor
		PDELETE(pXmlResponse);
	}
	else
		if(stat == STATUS_FW_REQUEST_TO_RESOURCE_MANGER)
		{
			CResourceManagerApi ResourceManagerApi;
			const COsQueue ResourceQueue = ResourceManagerApi.GetRcvMbx();

			pXMLRequest = pRequest->SerializeXml();
			char *buffer   = NULL;
			DWORD len       = 0;
			
			CXMLDOMDocument doc(pXMLRequest, encodingTypeFrom);
			doc.DumpDataAsLongStringEx(&buffer, &len);

			PostXml(buffer,len,(COsQueue&)ResourceQueue,&q,XML_REQUEST,nAuthorization,true);

			PDELETEA(buffer);
			pXMLRequest = NULL;			// it will be destroyed inside doc's dtor
			PDELETE(pXmlResponse);

		}
		else
	    if (stat == STATUS_FW_REQUEST_TO_CONFPARTY)
		{	
	    	CConfPartyManagerApi confPartyManagerApi;
			const COsQueue confPartyQueue = confPartyManagerApi.GetRcvMbx();
//			TRACESTR(eLevelInfoNormal) << "CRequestHandler::HandleRequest, confParty apiFormat=" << pRequest->GetConfirmObject()->GetApiFormat();
			if (pRequest && pRequest->GetConfirmObject() && pRequest->GetConfirmObject()->GetApiFormat()==eRestApi)
			{
				std::ostringstream os;
				os << *(pRequest->GetConfirmObject()->GetRestApiObject());
				std::string buffer(os.str());
				PostXml(buffer.c_str(), buffer.size(), (COsQueue&)confPartyQueue,&q,XML_REQUEST,nAuthorization,true);
			}
			else
			{
				char *buffer = NULL;
				DWORD len    = 0;

				pXMLRequest = pRequest->SerializeXml();
				CXMLDOMDocument doc(pXMLRequest, encodingTypeFrom);
				doc.DumpDataAsLongStringEx(&buffer, &len);

				PostXml(buffer,len,(COsQueue&)confPartyQueue,&q,XML_REQUEST,nAuthorization,true);

				PDELETEA(buffer);
				pXMLRequest = NULL;			// it will be destroyed inside doc's dtor
			}

			PDELETE(pXmlResponse);
		}
	    else
	    {
	    	if (stat != STATUS_WAIT_FOR_AN_ANSWER_FROM_EXTERNAL_DB)
	    	{
	    		if(pXmlResponse)
	    		{
	    			SendEventToAuditor(pXMLRequest, pXmlResponse, pRequest->GetXmlActionNodeType());

//	    			TRACESTR(eLevelInfoNormal) << "CRequestHandler::HandleRequest, typeid=" << typeid(pRequest->GetConfirmObject()).name();
//	    			if (pRequest->GetRequestObject() && pRequest->GetRequestObject()->GetApiFormat())
//	    				TRACESTR(eLevelInfoNormal) << "CRequestHandler::HandleRequest, apiFormat=" << pRequest->GetRequestObject()->GetApiFormat();
	    			if (pRequest && pRequest->GetConfirmObject() && pRequest->GetConfirmObject()->GetApiFormat()==eRestApi)
	    			{
	    				std::ostringstream os;
	    				os << *(pRequest->GetConfirmObject()->GetRestApiObject());
	    				std::string buffer(os.str());
	    				stat = PostXml(buffer.c_str(),buffer.size(),q);
	    			}
	    			else
	    			{

	    				char *buffer   = NULL;
	    				DWORD len       = 0;

	    				CXMLDOMDocument doc(pXmlResponse, encodingTypeFrom);
	    				doc.DumpDataAsLongStringEx(&buffer, &len);

	    				pXmlResponse = NULL; // it will be destroyed inside doc's dtor

	    				COstrStream statusString;
	    				stat = ConvertToPreviousEncoding(buffer,
	    						encodingTypeFrom,
	    						statusString);
	    				stat = PostXml(buffer,len,q);
	    				PDELETEA(buffer);
	    			}

	    			if( STATUS_OK != stat )
	    				FTRACESTR(eLevelInfoNormal) << "CRequestHandler::HandleRequest - PostXml for response failed, status = " <<stat;

			   }
			   else
			   {
			     PASSERTMSG(1, "pXmlResponse is NULL");   			   
			   }
		   }
	 }
    
	PDELETE(pRequest);
        
	return pXmlResponse;
}

void CRequestHandler::InitTask()
{
	InitTransactionsFactory();
}

void CRequestHandler::InitTransactionsFactory()
{
	// INIT BASE TRANSACTIONS
}

STATUS CRequestHandler::PostXml(const char *pPostBuff, DWORD postBuffLen,
                                COsQueue& SendQ,COsQueue* pReplyQ,
                                OPCODE opcode,WORD nAuthorization, bool bAddAdditionalParams)
{
	if ((pPostBuff == NULL) || ((opcode == XML_REQUEST) && (pReplyQ == NULL)))
	{
		PASSERT(1);
		return STATUS_FAIL;
	}
    m_audit_more_info.clear();
    
	BYTE bCompressed = false;

	CTaskApi TxApi;
	TxApi.CreateOnlyApi(SendQ);
	CSegment *pSeg = new CSegment;

	*m_pClientRspMbx=TxApi.GetRcvMbx();
	
	if(opcode == XML_REQUEST)
		pReplyQ->Serialize(*pSeg);

    DWORD contentLen = postBuffLen;
    const BYTE *pContent = (BYTE*)pPostBuff;
    char *pszZippedBuffer = NULL;

	if(postBuffLen > MAX_NON_ZIPPED)
	{
		CZipWrapper ZipWrapper;

		pszZippedBuffer = new char[postBuffLen];
		memset(pszZippedBuffer,0,postBuffLen);
		int nZippedLen = ZipWrapper.Deflate((unsigned char*)pPostBuff,postBuffLen,
                                            (unsigned char*)pszZippedBuffer,postBuffLen);
		
		if(nZippedLen != -1)
		{
			bCompressed = true;
            contentLen = nZippedLen;// *pSeg << (DWORD)nZippedLen;
            pContent = (BYTE*)pszZippedBuffer;    // pSeg->Put((BYTE*)pszZippedBuffer,(DWORD)nZippedLen);
		}
        else
        {
            // ZIP failed so we send non zipped data.
            contentLen = postBuffLen; //*pSeg << postBuffLen;
            pContent = (BYTE*)pPostBuff;   //pSeg->Put((BYTE*)pPostBuff,postBuffLen);
            FTRACESTR(eLevelInfoNormal) << "CRequestHandler::PostXml - ZIP failed so we send non zipped data. The buffer = " << pPostBuff;
        }
	}

    CPostXmlHeader postXmlHeader(contentLen,
                                 nAuthorization,
                                 bCompressed,
                                 m_CurrentMsgHdr.GetEncodingCharset(),
                                 m_CurrentMsgHdr.GetWorkStation(),
                                 m_CurrentMsgHdr.GetUserName(),
                                 m_CurrentMsgHdr.GetClientIp(),
                                 false,
                                 "",
                                 "",
                                 "",
                                 m_CurrentMsgHdr.GetIfNoneMatch(),
								 m_CurrentMsgHdr.GetConnId());
    
    postXmlHeader.Serialize(*pSeg);
    pSeg->Put((BYTE*)pContent, contentLen);
    
    DEALLOCBUFFER(pszZippedBuffer);

	if(opcode == XML_RESPONSE || ((opcode == XML_REQUEST) && bAddAdditionalParams))	
	    SendAdditionalParams(pSeg);	
	
	return ResponedClientRequest(opcode,pSeg);
}

/////////////////////////////////////////////////////////////////////////////
STATUS CRequestHandler::ConvertToPreviousEncoding(char *&pFrom,
                                                  const string & encodingTypeTo,
                                                  ostream &errorString)
{
    CObjString::ToUpper((char*)encodingTypeTo.c_str());

    string encodingTypeFrom = MCMS_INTERNAL_STRING_ENCODE_TYPE;
    STATUS statusConvert = CEncodingConvertor::Convert(encodingTypeTo,
                                                       encodingTypeFrom,
                                                       pFrom,
                                                       errorString);
    if(STATUS_OK != statusConvert)
    {
        statusConvert = STATUS_CONVERTION_UTF8_TO_ORIGIN_FAILED;
    }
    
    return statusConvert;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CRequestHandler::ParseHandleTransCommon(CXMLDOMDocument *pDom,
                                               COsQueue &dualMbx,
                                               WORD nAuthorization,DWORD ifNoneMatch,
                                               const string & encodingTypeFrom,
                                               CXMLDOMElement *& pOutXmlResponse,
                                               ostream& outStatusString)
{
    STATUS status = STATUS_OK;
    
    CXMLDOMElement *pRoot = pDom->GetRootElement();

    CXMLDOMElement * pCommonNode = NULL;
    pRoot->getChildNodeByName(&pCommonNode,"TRANS_COMMON_PARAMS");
    if (pCommonNode)
    {
        DWORD mes_header=0;
        DWORD userToken1=0;
        DWORD userToken2=0;
        
        pCommonNode->GetAndVerifyChildNodeValue("MESSAGE_ID",
                                                &mes_header,
                                                m_szErrorMessage,
                                                _0_TO_DWORD);
        CXMLDOMElement * pAsyncNode = NULL;
        pCommonNode->getChildNodeByName(&pAsyncNode,"ASYNC");
        
        if (pAsyncNode)
        {
            pAsyncNode->GetAndVerifyChildNodeValue("YOUR_TOKEN1",
                                                   &userToken1,
                                                   m_szErrorMessage,
                                                   _0_TO_DWORD);
            
            pAsyncNode->GetAndVerifyChildNodeValue("YOUR_TOKEN2",
                                                   &userToken2,
                                                   m_szErrorMessage,
                                                   _0_TO_DWORD);
        }

        CXMLDOMElement * pActionNode = NULL;
        pRoot->getChildNodeByName(&pActionNode,"ACTION");

        CXMLDOMElement * pActionTypeNode = NULL;
        if(pActionNode)
        {
            pActionNode->firstChildNode(&pActionTypeNode);
        }
        else
        {
            pRoot->firstChildNode(&pActionTypeNode);
            pRoot->nextChildNode(&pActionTypeNode);//action node
        }
        
        if(pActionTypeNode)
        {
            char *pNodeName=NULL;
            pActionTypeNode->get_nodeName(&pNodeName);
            if(pNodeName)
            {
                pOutXmlResponse = HandleRequest(pDom->GetRootElement(),
                                                dualMbx,
                                                nAuthorization,ifNoneMatch,
                                                encodingTypeFrom); 
            }
            else
            {
                status       = STATUS_NODE_MISSING;
                outStatusString<< "(Action element)";
            }
        }
        else
        {
            status       = STATUS_NODE_MISSING;
            outStatusString << "(Action element)";
        }
    }
    else
    {
        status       = STATUS_NODE_MISSING;
        outStatusString << ". ('TRANS_COMMON_PARAMS')";
    }
    
    return status;
}

/////////////////////////////////////////////////////////////////////////////
void CRequestHandler::SendEventToAuditor(CXMLDOMElement* pXMLRequest,
                                         CXMLDOMElement* pXMLResponse,
                                         eXmlActionNodeType actionNodeType)
{
    if(!m_CurrentMsgHdr.GetIsAudit())
        return;
    
    bool isGeneralResponse = false;
    string strDescription;
    string strDesciprionEx;
    eAuditEventStatus eventStatus = eAuditEventStatusFail;
    ParseResponseStatus(pXMLResponse, eventStatus, strDescription, strDesciprionEx, isGeneralResponse);

    // it is possible to take action from request, and event it is better but this way
    // (action from response) we can catch general responses, an assert is thrown in such case
    // since general responses are not normal it is good to catch it.
    // yurir 11/12/2007
    string action;
    ParseResponseAction(pXMLResponse, isGeneralResponse, actionNodeType, action);

    bool resShadow = ShadowPrivateData(pXMLRequest, pXMLResponse, action, actionNodeType);
    if(!resShadow)
    {
        PASSERTMSG(TRUE, "Failed to shadow private data, audit event will be discarded");
        return;
    }
    
    char *buffRequest = NULL;
    pXMLRequest->DumpDataAsLongStringEx(&buffRequest, TRUE);
    
    char *buffResponse = NULL;
    pXMLResponse->DumpDataAsLongStringEx(&buffResponse, TRUE);

    //    const char *processName = CProcessBase::GetProcessName(CProcessBase::GetProcess()->GetProcessType());
    
    
    AUDIT_EVENT_HEADER_S auditHdr;
    CAuditorApi::PrepareAuditHeader(auditHdr,
                                    m_CurrentMsgHdr.GetUserName(),
                                    eMcms,
                                    m_CurrentMsgHdr.GetWorkStation(),
                                    m_CurrentMsgHdr.GetClientIp(),
                                    eAuditEventTypeApi,
                                    eventStatus,
                                    m_CurrentMsgHdr.GetTransName(), //action
                                    m_CurrentMsgHdr.GetTransDesc(), 
                                    m_CurrentMsgHdr.GetTransFailureDesc(),
                                    strDesciprionEx);
    CFreeData freeData;
    CAuditorApi::PrepareFreeData(freeData,
                                 "Request Transaction",
                                 eFreeDataTypeXml,
                                 buffRequest,
                                 "Response Transaction",
                                 eFreeDataTypeXml,
                                 buffResponse);
    CAuditorApi api;
    api.SendEventMcms(auditHdr, freeData);

    delete [] buffRequest;
    delete [] buffResponse;
}

// ---------------------------------------------------------------------
// the ParseResponseStatus and ParseResponseAction extract status descriptions and action
// from the response transaction.
// there are 2 types of response: normal response
//                                general response
// ---------------------------------------------------------------------
// 1) normal response, version 1 - with ACTION tag
//
// <RESPONSE_TRANS_MCU>
// 	<RETURN_STATUS>
// 		<ID>0</ID>
// 		<DESCRIPTION>Status OK</DESCRIPTION>
// 		<YOUR_TOKEN1>0</YOUR_TOKEN1>
// 		<YOUR_TOKEN2>0</YOUR_TOKEN2>
// 		<MESSAGE_ID>365</MESSAGE_ID>
// 		<DESCRIPTION_EX></DESCRIPTION_EX>
// 	</RETURN_STATUS>
// 	<ACTION>
// 		<LOGOUT/>
// 	</ACTION>
// </RESPONSE_TRANS_MCU>

// 2) normal response, version 2 - withOUT ACTION tag

// <RESPONSE_TRANS_MCU>
// 	<RETURN_STATUS>
// 		<ID>0</ID>
// 		<DESCRIPTION>Status OK</DESCRIPTION>
// 		<YOUR_TOKEN1>0</YOUR_TOKEN1>
// 		<YOUR_TOKEN2>0</YOUR_TOKEN2>
// 		<MESSAGE_ID>365</MESSAGE_ID>
// 		<DESCRIPTION_EX></DESCRIPTION_EX>
// 	</RETURN_STATUS>
// 	<LOGOUT/>
// </RESPONSE_TRANS_MCU>


// 3) general response

// <RESPONSE_GENERAL>
//         <RETURN_STATUS>
//                 <ID>65586</ID>
//                 <DESCRIPTION>Field value is too short</DESCRIPTION>
//                 <YOUR_TOKEN1>0</YOUR_TOKEN1>
//                 <YOUR_TOKEN2>0</YOUR_TOKEN2>
//                 <MESSAGE_ID>0</MESSAGE_ID>
//                 <DESCRIPTION_EX>Limited to min num of characters (1) (Element: 'USER_NAM E')</DESCRIPTION_EX>
//         </RETURN_STATUS>
//         <ACTION_TYPE>
//                 set_request
//         </ACTION_TYPE>
// </RESPONSE_GENERAL>
/////////////////////////////////////////////////////////////////////////////
void CRequestHandler::ParseResponseStatus(CXMLDOMElement *pXMLResponse,
                                          eAuditEventStatus & outStatus,
                                          string & outDescription,
                                          string & outDesciprionEx,
                                          bool & outIsGeneralResponse)
{
    char *pStrName = NULL;
    if(SEC_OK != pXMLResponse->get_tagName(&pStrName))
    {
        PASSERTMSG(TRUE, "SEC_OK != pXMLResponse->get_tagName(&pStrName)");
        return;
    }
    if(NULL == pStrName)
    {
        PASSERTMSG(TRUE, "NULL == pStrName");
        return;
    }
    
    outIsGeneralResponse = (0 == strcmp(pStrName, "RESPONSE_GENERAL"));
    
    CXMLDOMElement *pRetStatus = NULL;
    GET_FIRST_CHILD_NODE(pXMLResponse, "RETURN_STATUS", pRetStatus);
    if(NULL == pRetStatus)
    {
        PASSERTMSG(TRUE, "NULL == pRetStatus");
        return;
    }
     
    CXMLDOMElement *pDescription = NULL;
    GET_FIRST_CHILD_NODE(pRetStatus, "DESCRIPTION", pDescription);
    if(NULL == pDescription)
    {
        PASSERTMSG(TRUE, "NULL == pDescription 1");
        return;
    }

    char *pStrStatus = NULL;
    if(SEC_OK != pDescription->get_nodeValue(&pStrStatus))
    {
        PASSERTMSG(TRUE, "SEC_OK != pDescription->get_nodeValue(&pStrStatus)");
        return;
    }
  
    outDescription = pStrStatus;
    
    const string & strOk = CProcessBase::GetProcess()->GetStatusAsString(STATUS_OK);
    outStatus = (strOk == pStrStatus
                 ?
                 eAuditEventStatusOk : eAuditEventStatusFail);
      
        
    GET_FIRST_CHILD_NODE(pRetStatus, "DESCRIPTION_EX", pDescription);
    if(NULL == pDescription)
    {
        PASSERTMSG(TRUE, "SEC_OK != NULL == pDescription 2");
        return;
    }
    
     
    char *pDescEx = NULL;
    if(SEC_OK != pDescription->get_nodeValue(&pDescEx))
    {
        PASSERTMSG(TRUE, "SEC_OK != pDescription->get_nodeValue(&pDescEx)");
        return;
    }
    

    outDesciprionEx = pDescEx;
    if (outDesciprionEx == "")
    {
        outDesciprionEx = m_audit_more_info;
    }

}

/////////////////////////////////////////////////////////////////////////////
void CRequestHandler::ParseResponseAction(CXMLDOMElement *pXMLResponse,
                                          bool isGeneralResponse,
                                          eXmlActionNodeType actionNodeType,
                                          string & outAction)
{
    CXMLDOMElement *pAction = GetActionNode(pXMLResponse, isGeneralResponse, actionNodeType);
    if(NULL == pAction)
    {
        PASSERTMSG(TRUE, "NULL == pAction");
        return;
    }

    char *pStrAction = NULL;
    if(isGeneralResponse)
    {
        if(SEC_OK != pAction->get_nodeValue(&pStrAction))
        {
            PASSERTMSG(TRUE, "SEC_OK != pAction->get_nodeValue(pStrAction)");
            return;
        }
    }
    else
    {
        if(SEC_OK != pAction->get_tagName(&pStrAction))
        {
            PASSERTMSG(TRUE, "SEC_OK != pFirstChild->get_tagName(&pStrAction)");
            return;
        }
    }

    outAction = pStrAction;
}

/////////////////////////////////////////////////////////////////////////////
// This function returns either pointer to a specific action(GET_CUCU_LULU) or NULL
CXMLDOMElement* CRequestHandler::GetActionNode(CXMLDOMElement *pXMLResponse,
                                               bool isGeneralResponse,
                                               eXmlActionNodeType actionNodeType)
{
    CXMLDOMElement *pAction = NULL;
    CXMLDOMElement *firstChild = NULL;
    CXMLDOMElement *secondChild = NULL;
    
    // general response
    if(isGeneralResponse)
    {
        GET_FIRST_CHILD_NODE(pXMLResponse, "ACTION_TYPE", pAction);
        return pAction;
    }

    
    // normal response, version 1, with ACTION tag
    if(ACTION_ELEMENT == actionNodeType)
    {
        GET_FIRST_CHILD_NODE(pXMLResponse, "ACTION", pAction);
        if(NULL != pAction)
        {
            pAction->firstChildNode(&firstChild);
        }
        return firstChild;
    }


    // normal response, version 2, withOUT ACTION tag
    if(ACTION_GROUP == actionNodeType)
    {
        HRES hres = pXMLResponse->firstChildNode(&firstChild);
        if(SEC_OK == hres && NULL != firstChild)
        {    
            pXMLResponse->nextChildNode(&secondChild);
        }
        return secondChild;
    }

    
    // bad parameters
    CSmallString message = "Bad response type: ";
    message << "Is general response = " << (isGeneralResponse ? "Yes" : "No") << "\n"
            << "Action node type = " << actionNodeType;
    PASSERTMSG(TRUE, message.GetString());
    
    return NULL;
}


/////////////////////////////////////////////////////////////////////////////
bool CRequestHandler::ShadowPrivateData(CXMLDOMElement *pXMLRequest,
                                        CXMLDOMElement *pXMLResponse,
                                        const string & action,
                                        eXmlActionNodeType actionNodeType)
{
    bool res = true;
    
    if(action == "LOGIN")
    {
        res = ShadowLogin(pXMLRequest, pXMLResponse, actionNodeType);
    }
    else if(action == "NEW_OPERATOR")
    {
        res = ShadowAddNewOperator(pXMLRequest, pXMLResponse, actionNodeType);
    }
    else if(action == "CHANGE_PASSWORD")
    {
        res = ShadowChangePassword(pXMLRequest, pXMLResponse, actionNodeType);
    }
    else if(action == "NEW_IP_SERVICE" || action == "UPDATE_IP_SERVICE")
    {
        res = ShadowIPService(pXMLRequest, pXMLResponse, actionNodeType);
    }
	else if( action == "SET_ENTRY_PASSWORD"  )
	{
		res = ShadowConferenceEntryPwd(pXMLRequest, pXMLResponse, actionNodeType);
	}
	else if( action == "SET_PASSWORD"  )
	{
		res = ShadowConferenceChairpersonPwd(pXMLRequest, pXMLResponse, actionNodeType);
	}
    return res;
}


bool CRequestHandler::ShadowIPService(CXMLDOMElement* pXMLRequest,
                                      CXMLDOMElement* pXMLResponse,
                                      eXmlActionNodeType actionNodeType)
{
  CXMLDOMElement* pIPService = GetActionNode(pXMLRequest, false, actionNodeType);
  PASSERTMSG_AND_RETURN_VALUE(NULL == pIPService, "NULL == pIPService", false);

  CXMLDOMElement* pService = NULL;
  GET_FIRST_CHILD_NODE(pIPService, "IP_SERVICE", pService);
  PASSERTMSG_AND_RETURN_VALUE(NULL == pService, "NULL == IP_SERVICE", false);

  CXMLDOMElement* pSecurity = NULL;
  GET_FIRST_CHILD_NODE(pService, "SECURITY", pSecurity);
  PASSERTMSG_AND_RETURN_VALUE(NULL == pSecurity, "NULL == SECURITY", false);

  CXMLDOMElement* pAthentication = NULL;
  GET_FIRST_CHILD_NODE(pSecurity, "AUTHENTICATION", pAthentication);
  PASSERTMSG_AND_RETURN_VALUE(NULL == pAthentication, "NULL == AUTHENTICATION", false);

  CXMLDOMElement* pDigestList = NULL;
  GET_FIRST_CHILD_NODE(pAthentication, "HTTP_DIGEST_LIST", pDigestList);
  PASSERTMSG_AND_RETURN_VALUE(NULL == pDigestList, "NULL == HTTP_DIGEST_LIST", false);

  CXMLDOMElement* pHttpDigest = NULL;
  GET_FIRST_CHILD_NODE(pDigestList, "HTTP_DIGEST", pHttpDigest);
  if (NULL == pHttpDigest)
  {
    FTRACEINTOFUNC << "HTTP_DIGEST is empty. no need to create shadow password.";
    // continue
  }
  else
  {
    CXMLDOMElement* pPassword = NULL;
    GET_FIRST_CHILD_NODE(pHttpDigest, "PASSWORD", pPassword);
    PASSERTMSG_AND_RETURN_VALUE(NULL == pPassword, "NULL == PASSWORD", false);

    pPassword->SetValue("*********");
  }

  CXMLDOMElement* pV35Gateway = NULL;
  GET_FIRST_CHILD_NODE(pService, "V35_GATEWAY", pV35Gateway);
  if (NULL == pV35Gateway)
  {
    FTRACEINTOFUNC << "V35_GATEWAY is empty. no need to create shadow password.";
    return true;
  }

  CXMLDOMElement* pV35GatewayPassword = NULL;
  GET_FIRST_CHILD_NODE(pV35Gateway, "V35_GATEWAY_PASSWORD", pV35GatewayPassword);
  PASSERTMSG_AND_RETURN_VALUE(NULL == pV35GatewayPassword, "NULL == V35_GATEWAY_PASSWORD", false);

  pV35GatewayPassword->SetValue("*********");

  return true;
}

// -------------------------------------------------------------------------
// ShadowLogin shadows password's value
// -------------------------------------------------------------------------
// <TRANS_MCU>
// 	<TRANS_COMMON_PARAMS>
// 		<MCU_TOKEN>None</MCU_TOKEN>
// 		<MCU_USER_TOKEN>None</MCU_USER_TOKEN>
// 		<ASYNC>
// 			<YOUR_TOKEN1>0</YOUR_TOKEN1>
// 			<YOUR_TOKEN2>0</YOUR_TOKEN2>
// 		</ASYNC>
// 		<MESSAGE_ID>0</MESSAGE_ID>
// 	</TRANS_COMMON_PARAMS>
// 	<ACTION>
// 		<LOGIN>
// 			<MCU_IP>
// 				<IP>127.0.0.1</IP>
// 				<LISTEN_PORT>80</LISTEN_PORT>
// 			</MCU_IP>
// 			<USER_NAME>POLYCOM</USER_NAME>
// 			<PASSWORD>POLYCOM</PASSWORD>
// 			<STATION_NAME>MyStation</STATION_NAME>
// 			<COMPRESSION>false</COMPRESSION>
// 			<CONFERENCE_RECORDER>false</CONFERENCE_RECORDER>
// 		</LOGIN>
// 	</ACTION>
// </TRANS_MCU>
/////////////////////////////////////////////////////////////////////////////
bool CRequestHandler::ShadowLogin(CXMLDOMElement *pXMLRequest, CXMLDOMElement *pXMLResponse, eXmlActionNodeType actionNodeType)
{
    CXMLDOMElement *pLogin = GetActionNode(pXMLRequest, false, actionNodeType);
    if(NULL == pLogin)
    {
        PASSERTMSG(TRUE, "NULL == pLogin");
        return false;
    }

    CXMLDOMElement *pPassword = NULL;
    GET_FIRST_CHILD_NODE(pLogin, "PASSWORD", pPassword);
    if(NULL == pPassword)
    {
        PASSERTMSG(TRUE, "NULL == pPassword");
        return false;
    }
    
    pPassword->SetValue("*********");

    // VNGR-10333
    // New password exposed in Audit file.
    // --------------------------------------------------------
    CXMLDOMElement *pNewPassword = NULL;
    GET_FIRST_CHILD_NODE(pLogin, "NEW_PASSWORD", pNewPassword);
    if(NULL != pNewPassword)
    {
        pNewPassword->SetValue("*********");
    }
    // --------------------------------------------------------

    return true;
}

// -------------------------------------------------------------------------
// ShadowAddNewOperator shadows password's value
// -------------------------------------------------------------------------
// <TRANS_OPERATOR>
// 	<TRANS_COMMON_PARAMS>
// 		<MCU_TOKEN>16</MCU_TOKEN>
// 		<MCU_USER_TOKEN>16</MCU_USER_TOKEN>
// 		<MESSAGE_ID>148</MESSAGE_ID>
// 	</TRANS_COMMON_PARAMS>
// 	<ACTION>
// 		<NEW_OPERATOR>
// 			<OPERATOR>
// 				<USER_NAME>audit</USER_NAME>
// 				<PASSWORD>secret</PASSWORD>
// 				<AUTHORIZATION_GROUP>auditor</AUTHORIZATION_GROUP>
// 			</OPERATOR>
// 		</NEW_OPERATOR>
// 	</ACTION>
// </TRANS_OPERATOR>
/////////////////////////////////////////////////////////////////////////////
bool CRequestHandler::ShadowAddNewOperator(CXMLDOMElement *pXMLRequest, CXMLDOMElement *pXMLResponse, eXmlActionNodeType actionNodeType)
{
    CXMLDOMElement *pNewOperator = GetActionNode(pXMLRequest, false, actionNodeType);
    if(NULL == pNewOperator)
    {
        PASSERTMSG(TRUE, "NULL == pNewOperator");
        return false;
    }

    CXMLDOMElement *pOperator = NULL;
    GET_FIRST_CHILD_NODE(pNewOperator, "OPERATOR", pOperator);
    if(NULL == pOperator)
    {
        PASSERTMSG(TRUE, "NULL == pOperator");
        return false;
    }
    
    CXMLDOMElement *pPassword = NULL;
    GET_FIRST_CHILD_NODE(pOperator, "PASSWORD", pPassword);
    if(NULL == pPassword)
    {
        PASSERTMSG(TRUE, "NULL == pPassword");
        return false;
    }
    
    pPassword->SetValue("*********");
    return true;
}


// -------------------------------------------------------------------------
// ShadowChangePassword shadows password's value
// -------------------------------------------------------------------------
// <TRANS_OPERATOR>
// 	<TRANS_COMMON_PARAMS>
// 		<MCU_TOKEN>2</MCU_TOKEN>
// 		<MCU_USER_TOKEN>2</MCU_USER_TOKEN>
// 		<MESSAGE_ID>159</MESSAGE_ID>
// 	</TRANS_COMMON_PARAMS>
// 	<ACTION>
// 		<CHANGE_PASSWORD>
// 			<USER_NAME>cucu</USER_NAME>
// 			<OLD_PASSWORD>cucu</OLD_PASSWORD>
// 			<NEW_PASSWORD>lulu</NEW_PASSWORD>
// 		</CHANGE_PASSWORD>
// 	</ACTION>
// </TRANS_OPERATOR>

bool CRequestHandler::ShadowChangePassword(CXMLDOMElement *pXMLRequest,
                                           CXMLDOMElement *pXMLResponse,
                                           eXmlActionNodeType actionNodeType)
{
    CXMLDOMElement *pChangePasswordNode = GetActionNode(pXMLRequest, false, actionNodeType);
    if(NULL == pChangePasswordNode)
    {
        PASSERTMSG(TRUE, "NULL == pChangePasswordNode");
        return false;
    }

    CXMLDOMElement *pOldPassword = NULL;
    GET_FIRST_CHILD_NODE(pChangePasswordNode, "OLD_PASSWORD", pOldPassword);
    if(NULL == pOldPassword)
    {
        PASSERTMSG(TRUE, "NULL == pOldPassword");
        return false;
    }
    
    CXMLDOMElement *pNewPassword = NULL;
    GET_FIRST_CHILD_NODE(pChangePasswordNode, "NEW_PASSWORD", pNewPassword);
    if(NULL == pNewPassword)
    {
        PASSERTMSG(TRUE, "NULL == pNewPassword");
        return false;
    }

    pOldPassword->SetValue("*********");
    pNewPassword->SetValue("*********");
  
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CRequestHandler::UpdateUserName(const string & user)
{
    m_CurrentMsgHdr.SetUserName(user);
}

// -------------------------------------------------------------------------
// ShadowConferenceEntryPwd shadows password's value
// -------------------------------------------------------------------------
//	<TRANS_CONF_2>
//		<TRANS_COMMON_PARAMS>
//			<MCU_TOKEN>5</MCU_TOKEN>
//			<MCU_USER_TOKEN>5</MCU_USER_TOKEN>
//			<MESSAGE_ID>191</MESSAGE_ID>
//		</TRANS_COMMON_PARAMS>
//		<ACTION>
//			<SET_ENTRY_PASSWORD>
//				<ID>11</ID>
//				<ENTRY_PASSWORD>11111</ENTRY_PASSWORD>
//			</SET_ENTRY_PASSWORD>
//		</ACTION>
//	</TRANS_CONF_2>
//
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CRequestHandler::ShadowConferenceEntryPwd(CXMLDOMElement *pXMLRequest, CXMLDOMElement *pXMLResponse, eXmlActionNodeType actionNodeType)
{
    CXMLDOMElement *pEntryPasswordNode = GetActionNode(pXMLRequest, false, actionNodeType);
    if(NULL == pEntryPasswordNode)
    {
        PASSERTMSG(TRUE, "NULL == pEntryPasswordNode");
        return false;
    }

    CXMLDOMElement *pPassword = NULL;
    GET_FIRST_CHILD_NODE(pEntryPasswordNode, "ENTRY_PASSWORD", pPassword);
    if(NULL == pPassword)
    {
        PASSERTMSG(TRUE, "NULL == pPassword");
        return false;
    }

    pPassword->SetValue("*********");

    return true;
}

// -------------------------------------------------------------------------
// ShadowConferenceChairpersonPwd shadows password's value
// -------------------------------------------------------------------------
//	<TRANS_CONF_2>
//		<TRANS_COMMON_PARAMS>
//			<MCU_TOKEN>9</MCU_TOKEN>
//			<MCU_USER_TOKEN>9</MCU_USER_TOKEN>
//			<MESSAGE_ID>515</MESSAGE_ID>
//		</TRANS_COMMON_PARAMS>
//		<ACTION>
//			<SET_PASSWORD>
//				<ID>11</ID>
//				<PASSWORD>22222</PASSWORD>
//			</SET_PASSWORD>
//		</ACTION>
//	</TRANS_CONF_2>
//
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CRequestHandler::ShadowConferenceChairpersonPwd(CXMLDOMElement *pXMLRequest, CXMLDOMElement *pXMLResponse, eXmlActionNodeType actionNodeType)
{
    CXMLDOMElement *pPasswordNode = GetActionNode(pXMLRequest, false, actionNodeType);
    if(NULL == pPasswordNode)
    {
        PASSERTMSG(TRUE, "NULL == pPasswordNode");
        return false;
    }

    CXMLDOMElement *pPassword = NULL;
    GET_FIRST_CHILD_NODE(pPasswordNode, "PASSWORD", pPassword);
    if(NULL == pPassword)
    {
        PASSERTMSG(TRUE, "NULL == pPassword");
        return false;
    }

    pPassword->SetValue("*********");

    return true;
}

