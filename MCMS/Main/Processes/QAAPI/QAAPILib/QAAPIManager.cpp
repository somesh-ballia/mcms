// DemoManager.cpp: implementation of the CDemoManager class.
//
//////////////////////////////////////////////////////////////////////

#include "QAAPIManager.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "ClientSocket.h"
#include "psosxml.h"
#include "AppServerData.h"
#include "FaultsDefines.h"
#include "IfConfig.h"
#include "TraceStream.h"
#include "UnicodeDefines.h"
#include "NStream.h"
#include "IConv.h"
#include "ManagerApi.h"
#include "QA_ApiStructs.h"
#include "ObjString.h"
#include "SysConfig.h"
#include "SecuredSocketClient.h"
#include "FipsMode.h"
#include "ConfigManagerApi.h"
#include "CurlHTTP.h"
#include "EncodingConvertor.h"
#include "UnicodeDefines.h"
#include "HTTPPars.h"
#include "PartyIdTaskApi.h"

extern std::string	GetNicName(eIpType ipType);
extern char* IpTypeToString(APIU32 ipType, bool caps = false);


/////////////////////////////////////////////////////////////////////////////
//
//   STATES:
//
//const WORD  IDLE        = 0;        // default state -  defined in base class
const WORD  CONNECTING    = 1;
const WORD  STARTUP       = 2;
const WORD  CONFIG        = 3;
const WORD  CONNECTED     = 4;
//const WORD  ANYCASE     = 0xFFFF;   // any other state -  defined in base class


const char * ConnectionStateToString(WORD state)
{
    
    static const char *names [] =
        {
            "IDLE",
            "CONNECTING",
            "STARTUP",
            "CONFIG",
            "CONNECTED",
            "Invalid"
        };
    if(state < 6)
    {
        const char *name = (state <= (sizeof(names) / sizeof(names[0]))
                            ?
                            names[state] : "InvalidState");
        return name;
    }
    else
    {
        FPASSERTMSG(1, "state exceeded"); 
        return NULL;	
    }		
    
}




/////////////////////////////////////////////////////////////////////////////
//
//   EXTERNALS:
//OnSocketConnectedSetup
extern "C" void QAAPIRxEntryPoint(void* appParam);
extern "C" void QAAPITxEntryPoint(void* appParam);


PBEGIN_MESSAGE_MAP(CQAAPIManager)
	// Manager: connect card
	ONEVENT( SOCKET_CONNECT,			IDLE,		CQAAPIManager::OnSocketConnectIdle)
	// Socket: connection established
	ONEVENT( SOCKET_CONNECTED, 			CONNECTING, 	CQAAPIManager::OnSocketConnectedSetup)
	// Socket: connection failed
	ONEVENT( SOCKET_FAILED,   			CONNECTING,	CQAAPIManager::OnSocketFailedSetup)
	// Socket: connection dropped
	ONEVENT( SOCKET_DROPPED,  			CONNECTING, 	CQAAPIManager::OnSocketDroppedSetup)
	ONEVENT( SOCKET_DROPPED,   			CONNECTED, 	CQAAPIManager::OnSocketDroppedConnect)
	// Socket : message received
	ONEVENT( SOCKET_RCV_MSG,  			IDLE,     	CQAAPIManager::OnSocketRcvIdle)
 	ONEVENT( SOCKET_RCV_MSG,   			CONNECTING, 	CQAAPIManager::OnSocketRcvSetup)
	ONEVENT( SOCKET_RCV_MSG,   			CONNECTED,  	CQAAPIManager::OnSocketRcvConnect)
	// Logical module: forward message to external server
	ONEVENT( SEND_REQ_TO_EXT_DB,   			CONNECTED,  	CQAAPIManager::OnSocketSndConnect)
	ONEVENT( SEND_REQ_TO_EXT_DB,   			CONNECTING, 	CQAAPIManager::OnSocketSndConnecting)
	ONEVENT( SEND_REQ_TO_EXT_DB,   			IDLE,   	CQAAPIManager::OnSocketSndIdle)
	ONEVENT( SERVER_KEEP_ALIVE_TIMER,   		CONNECTED,  	CQAAPIManager::OnKeepAliveTimerConnected)
	ONEVENT( SERVER_KEEP_ALIVE_TIMER,   		CONNECTING,  	CQAAPIManager::OnKeepAliveTimerConnected)
	ONEVENT( EXT_DB_REQUEST    ,  			IDLE    , 	CQAAPIManager::HandlePostRequest )
	ONEVENT( EXT_DB_RESPONSE   ,  			ANYCASE,  	CQAAPIManager::OnExtDbResponseAny)


PEND_MESSAGE_MAP(CQAAPIManager,CManagerTask);


////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_SET_TRANSACTION_FACTORY(CQAAPIManager)
//ON_TRANS("TRANS_MCU","LOGIN",COperCfg,CQAAPIManager::HandleOperLogin)
END_TRANSACTION_FACTORY

////////////////////////////////////////////////////////////////////////////
//               Terminal Commands
////////////////////////////////////////////////////////////////////////////
BEGIN_TERMINAL_COMMANDS(CQAAPIManager)
//  ONCOMMAND("ping",CQAAPIManager::HandleTerminalPing,"test terminal commands")
    ONCOMMAND("sendLoginResp",CQAAPIManager::HandleTerminalSendLoginResp,"sends login responce to authenticator")
    ONCOMMAND("conn_state",CQAAPIManager::HandleTerminalGetConnState,"shows connection status to EXT DB")

END_TERMINAL_COMMANDS

extern void QAAPIMonitorEntryPoint(void* appParam);

////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void QAAPIManagerEntryPoint(void* appParam)
{
	CQAAPIManager * pQAAPIManager = new CQAAPIManager;
	pQAAPIManager->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CQAAPIManager::GetMonitorEntryPoint()
{
	return QAAPIMonitorEntryPoint;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CQAAPIManager::CQAAPIManager()
{
	m_pSocketConnection = NULL;
	m_pAppServerData	= NULL;
	m_pKeepAliveReq 	= NULL;
	m_pcurlhttpConn     = NULL;
    ZeroKACnt();
    m_sMngmntNI = "";
    m_curIpType = (eIpType)0;


}

//////////////////////////////////////////////////////////////////////
CQAAPIManager::~CQAAPIManager()
{
	POBJDELETE(m_pSocketConnection);
	POBJDELETE(m_pAppServerData);
	DEALLOCBUFFER(m_pKeepAliveReq);
	POBJDELETE(m_pcurlhttpConn);
}

//////////////////////////////////////////////////////////////////////
void CQAAPIManager::SelfKill()
{
	if(m_pSocketConnection)
	{
//		m_pSocketConnection->Disconnect();
		m_pSocketConnection->Destroy();
	}
	CManagerTask::SelfKill();
}

//////////////////////////////////////////////////////////////////////
void  CQAAPIManager::Create(CSegment& appParam)
{
	CManagerTask::Create(appParam);

	//char  szIpAddress[100];
	//WORD  port;

	//appParam	>> szIpAddress>> port;


}

////////////////////////////////////////////////////////////////////////////
void CQAAPIManager::IncKACnt()
{
    m_KeepAliveCnt++;
}

////////////////////////////////////////////////////////////////////////////
void CQAAPIManager::ZeroKACnt()
{
    m_KeepAliveCnt = 0;
}

////////////////////////////////////////////////////////////////////////////
void CQAAPIManager::OnSocketSndConnecting(CSegment* paramSegment)
{
	FTRACESTR(eLevelInfoNormal) << "CQAAPIManager::OnSocketSndConnecting - send to external database failed !!!!";
	SendReject(paramSegment);
}
void CQAAPIManager::OnSocketSndIdle(CSegment* paramSegment)
{
	FTRACESTR(eLevelInfoNormal) << "CQAAPIManager::OnSocketSndIdle - send to external database failed !!!!";
	SendReject(paramSegment);
}

void CQAAPIManager::SendReject(CSegment* paramSegment)
{
	//Important note !!!
	//In order to avoid the user to wait for a long period
	//of time (in case the connection can no be established)
	//it is prefferable to send the rejection imidiatly and
	//wait for the next keep alive to recover the connection

	DWORD   RetOpCode;
	DWORD len;
	*paramSegment >> RetOpCode;
	*paramSegment >> len;
	char *pXMLRequest=new char [len+1];
	pXMLRequest[len]='\0';
	*paramSegment >> pXMLRequest;

	COsQueue q;
	q.DeSerialize(*paramSegment);

	CSegment* pMsg = new CSegment; // pMsg will delete inside

	*pMsg  << RetOpCode;
	*pMsg  << (DWORD)strlen(pXMLRequest);
	*pMsg  << pXMLRequest;

	CTaskApi api;
	api.CreateOnlyApi(q);
	api.SendMsg(pMsg,EXT_DB_REQUEST_FAILED);
	api.DestroyOnlyApi();

	FTRACESTR(eLevelInfoNormal) << "CQAAPIManager::SendToSocket following request was rejected by QAAPI process\n " << pXMLRequest;

	PDELETEA(pXMLRequest);


}

/////////////////////////////////////////////////////////////////////////////
CXMLDOMElement* CQAAPIManager::GetElementByName(CXMLDOMDocument *pDom, const char *elemName)
{
	CXMLDOMElement *pRootElem = pDom->GetRootElement();
	if(NULL == pRootElem)
	{
		// very bad
		return NULL;
	}

	CXMLDOMElement *pLoginElem = NULL;
	GET_FIRST_CHILD_NODE(pRootElem,"LOGIN",pLoginElem);
	if(NULL == pLoginElem)
	{
		//very bad
		return NULL;
	}

	CXMLDOMElement *pMcuIpElem = NULL;
	GET_FIRST_CHILD_NODE(pLoginElem,"MCU_IP",pMcuIpElem);
	if(NULL == pMcuIpElem)
	{
		//very bad
		return NULL;
	}

	return pMcuIpElem;
}

/////////////////////////////////////////////////////////////////////////////
void CQAAPIManager::ReplaceMcuIp(char *& pXMLRequest, DWORD mcuIp)
{
    CXMLDOMDocument doc;
    int resParse = doc.Parse((const char **)&pXMLRequest);
    if(SEC_OK != resParse)
    {
    	PASSERTMSG(resParse + 100, "Parse did succeed(code = resParse + 100)");
    	return;
    }

    CXMLDOMElement *pMcuIpElem = GetElementByName(&doc, "MCU_IP");
    if(NULL == pMcuIpElem)
    {
    	PASSERTMSG(TRUE, "The McuIP DOM element was not found");
    	return;
    }

 	char strMcuIp [32];
	SystemDWORDToIpString(mcuIp, strMcuIp);
    pMcuIpElem->SetValue(strMcuIp);

    PDELETEA(pXMLRequest);
    DWORD len = 0;
    doc.DumpDataAsLongStringEx(&pXMLRequest,&len);
}

/////////////////////////////////////////////////////////////////////////////
void CQAAPIManager::SendToSocket(char *& pRequest)
{
	const char *mcuIpSearch = strstr(pRequest, "MCU_IP");
	if(NULL != mcuIpSearch)
	{
		ReplaceMcuIp(pRequest, m_MngmntIp);
	}

	CSegment segment;

    //AddEncodingType(pRequest);

	// put parameters to message
	segment  << (DWORD)strlen(pRequest);
	segment  << pRequest;
	segment  << (DWORD)(m_pAppServerData->GetPort());
	segment  << strlen(m_pAppServerData->GetDirectory());
	segment  << m_pAppServerData->GetDirectory();
	segment  << strlen(m_pAppServerData->GetStrIp());
	segment  << m_pAppServerData->GetStrIp();

	if (strstr(pRequest, "<PASSWORD>"))
	{
		ALLOCBUFFER(pszRequestWithShadowPassword, strlen(pRequest)+10);
		memset(pszRequestWithShadowPassword, 0, strlen(pRequest)+10);

		RemovePasswordFromString(pRequest, pszRequestWithShadowPassword, "<PASSWORD>", "</PASSWORD>");
		FTRACESTR(eLevelInfoNormal) << "CQAAPIManager::SendToSocket \nServer Ip - "
								<< m_pAppServerData->GetStrIp() << " using the port - "<<m_pAppServerData->GetPort()<<"\n"
								<< pszRequestWithShadowPassword;
		DEALLOCBUFFER(pszRequestWithShadowPassword);
	}
	else
		FTRACESTR(eLevelInfoNormal) << "CQAAPIManager::SendToSocket \nServer Ip - "
								<< m_pAppServerData->GetStrIp() << " using the port - "<<m_pAppServerData->GetPort()<<"\n"
                            << pRequest;

	if (m_pAppServerData->IsConnectionlessTransport())
	{
		CSegment seg;
		seg << pRequest;
		HandleSendExtDBReqLocaly(&seg);
		//DispatchEvent(SEND_REQ_LOCAL_INQAAPI_PROCCESS,&seg);
	}
	else
		m_pSocketConnection->Send(&segment);
}

/////////////////////////////////////////////////////////////////////////////
void CQAAPIManager::RemovePasswordFromString(char* pszSearchString, char* pszNewString, const char* pszElementOpenTag, const char* pszElementCloseTag)
{
	char* pszStartPassword = strstr(pszSearchString,pszElementOpenTag);
	if (pszStartPassword)	//if password exist in the transaction
	{
		pszStartPassword += strlen(pszElementOpenTag);
		char *pszEndPassword = strstr(pszStartPassword,pszElementCloseTag);

		if(pszEndPassword)
		{
			strncpy(pszNewString, pszSearchString, pszStartPassword-pszSearchString);
			pszNewString[pszStartPassword-pszSearchString] = '\0';
			strcat(pszNewString, "*********");
			char* pszStartPassword2 = strstr(pszEndPassword,pszElementOpenTag);	//look if the password appear twice
			if (pszStartPassword2)
			{
				pszStartPassword2 += strlen(pszElementOpenTag);
				char *pszEndPassword2 = strstr(pszStartPassword2,pszElementCloseTag);

				if (pszEndPassword && pszEndPassword2)
				{
					strncat(pszNewString, pszEndPassword, pszStartPassword2-pszEndPassword);
					strcat(pszNewString, "*********");
					strcat(pszNewString, pszEndPassword2);
				}
			}
			else
				strcat(pszNewString, pszEndPassword);
		}
	}
}
/////////////////////////////////////////////////////////////////////////////

// the assumption is that request does not contain any comment.
void CQAAPIManager::AddEncodingType(char *& pRequest)
{
    string newRequest = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    newRequest += pRequest;

    const DWORD totalLen = newRequest.length();
    char *pCommentRequest = new char[totalLen + 1];
    strncpy(pCommentRequest, newRequest.c_str(), totalLen);
    pCommentRequest[totalLen] = '\0';

    PDELETEA(pRequest);
    pRequest = pCommentRequest;
}

/////////////////////////////////////////////////////////////////////////////
void CQAAPIManager::OnKeepAliveTimerConnected(CSegment* paramSegment)
{
	if(m_state==CONNECTED || m_state == CONNECTING)
	{
		// paramSegment should contain COMMON_HEADER, PHISYCAL_HEADER and data
		if( !CPObject::IsValidPObjectPtr(m_pSocketConnection)  && !m_pAppServerData->IsConnectionlessTransport())
		{
			PASSERT(1);
			return;
		}
		SendToSocket(m_pKeepAliveReq);

        IncKACnt();
        if(GetKACnt() > 2)
        {
            OnKeepAliveFailure(GetKACnt());
        }
        else
        {
            OnKeepAliveOk();
        }
	}

    DWORD timeInterval = 600;
    CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey("QAAPI_KEEP_ALIVE_PERIOD", timeInterval);
    if (m_pAppServerData->IsConnectionlessTransport())//in connectiontless connection ssample each 10 second the Keep Alive status
    {
    	timeInterval = 30;
    }
    StartTimer(SERVER_KEEP_ALIVE_TIMER, timeInterval * SECOND);
}// not needed in case socket is connected (only when reject is sent)

/////////////////////////////////////////////////////////////////////////////
void CQAAPIManager::OnKeepAliveFailure(int cndOfFailure)
{
    CLargeString message;
    message << "External DB have not responsed to keep alive request "
            << cndOfFailure << " times";

    if(IsActiveAlarmExistByErrorCode(AA_CAN_NOT_ESTABLISH_CONNECTION_WITH_APPLICATION_SERVER))
    {
        UpdateActiveAlarmDescriptionByErrorCode(AA_CAN_NOT_ESTABLISH_CONNECTION_WITH_APPLICATION_SERVER,
                                                message.GetString());
    }
    else
    {
        AddActiveAlarm(	FAULT_GENERAL_SUBJECT,
                        AA_CAN_NOT_ESTABLISH_CONNECTION_WITH_APPLICATION_SERVER,
                        MAJOR_ERROR_LEVEL,
                        message.GetString(),
                        true,
                        true);
    }
}

/////////////////////////////////////////////////////////////////////////////
void CQAAPIManager::OnKeepAliveOk()
{
    RemoveActiveAlarmByErrorCode(AA_CAN_NOT_ESTABLISH_CONNECTION_WITH_APPLICATION_SERVER);
}

/////////////////////////////////////////////////////////////////////////////
void CQAAPIManager::OnSocketSndConnect(CSegment* paramSegment)
{
	// paramSegment should contain COMMON_HEADER, PHISYCAL_HEADER and data
	if( !CPObject::IsValidPObjectPtr(m_pSocketConnection) && !m_pAppServerData->IsConnectionlessTransport())
	{
		PASSERT(1);
		return;
	}

	DWORD   RetOpCode;
	DWORD len;

	*paramSegment >> RetOpCode;// not needed in case socket is connected (only when reject is sent)
	*paramSegment >> len;
	char *pXMLRequest=new char [len+1];
	pXMLRequest[len]='\0';
	*paramSegment >> pXMLRequest;

	COsQueue q;  // not needed in case socket is connected (only when reject is sent)
	q.DeSerialize(*paramSegment);

    // validate request. it should be in UTF-8 encoding
    COstrStream errorString;
    bool resValidateUtf8 = CIConv::ValidateStringEncoding(pXMLRequest,
                                                          MCMS_INTERNAL_STRING_ENCODE_TYPE,
                                                          errorString);
    if(!resValidateUtf8)
    {
        FTRACESTR(eLevelInfoNormal) << "CQAAPIManager::OnSocketSndConnect : "
                                << MCMS_INTERNAL_STRING_ENCODE_TYPE
                                << "validation failed\n "
                                << pXMLRequest;
        //STATUS_UTF8_VALIDATION_FAILED
        SendReject(paramSegment);
        PDELETEA(pXMLRequest);
        return;
    }

	SendToSocket(pXMLRequest);
	PDELETEA(pXMLRequest);
}
/////////////////////////////////////////////////////////////////////////////
void CQAAPIManager::HandleSendExtDBReqLocaly(CSegment* pMsg)
{
	std::string pRequest;
	*pMsg >> pRequest;
	CURLcode retVal;
	UnlockRelevantSemaphore();
	bool bReponse = m_pcurlhttpConn->PostRequest(retVal,m_pAppServerData->GetUrl(),pRequest);
	LockRelevantSemaphore();
	CSegment respSeq;
	if(bReponse==false)
	{
		FTRACESTR(eLevelWarn) << "CQAAPIManager::ReceiveFromSocket - bad reply from external server header : =  " << m_pcurlhttpConn->GetResponseHeader().c_str();
		if (m_state==CONNECTING)
		{
			CTaskApi::StaticSendLocalMessage(NULL,SOCKET_FAILED);
		}
		else if (m_state==CONNECTED)
		{
			CTaskApi::StaticSendLocalMessage(NULL,SOCKET_DROPPED);
		    StartTimer(SERVER_KEEP_ALIVE_TIMER, 10 * SECOND);
		}
		return;
	}
	else if (m_state==CONNECTING)
	{
		CTaskApi::StaticSendLocalMessage(NULL,SOCKET_CONNECTED);
	}



	respSeq << m_pcurlhttpConn->GetResponseContent()->length();
	respSeq << m_pcurlhttpConn->GetResponseContent()->c_str();
	HandleConectionLessResponse();
	OnExtDbResponseAny(&respSeq);
}
/////////////////////////////////////////////////////////////////////////////
//the response is sitting in the m_pcurlhttpConn
void CQAAPIManager::HandleConectionLessResponse()
{
        std::string pHttpHeader = m_pcurlhttpConn->GetResponseHeader();
        string charSet;
        CHTTPHeaderParser::GetContentCharset(pHttpHeader.c_str(), charSet);
        if(false == charSet.empty())
        {
            bool isKnown = CEncodingConvertor::IsKnownEncoding(charSet);
            if(false == isKnown)
            {
                // drop the packet
                FTRACEINTO << "\nCQAAPIManager::ReceiveFromSocket\nDrop the packet\nthe encoding is unknown: " << charSet.c_str();
                return;
            }
        }

        DWORD tmpContentLen = m_pcurlhttpConn->GetResponseContent()->length();
        char *pContent = new char[tmpContentLen + 1];
        strcpy(pContent, m_pcurlhttpConn->GetResponseContent()->c_str());
        pContent[tmpContentLen] = 0;

        // convert to UTF-8
        COstrStream statusString;
        string currentEncoding;
        STATUS statusUTF8Convert = CEncodingConvertor::ConvertValidate(MCMS_INTERNAL_STRING_ENCODE_TYPE,
                                                                       currentEncoding,
                                                                       pContent,
                                                                       statusString);
        if(STATUS_OK != statusUTF8Convert)
        {
            // drop the packet
            FTRACEINTO << "\nCQAAPIManager::ReceiveFromSocket Drop the packet\n"
                       << statusString.str().c_str() << "\n"
                       << pContent;
            PDELETEA(pContent);
            return;
        }

		if (pContent)
		{
			FTRACESTR(eLevelInfoNormal) << "CQAAPIManager::ReceiveFromSocket - reply from external server - content... " << pContent;
            CXMLDOMDocument *pDom=new CXMLDOMDocument;
            if (pDom->Parse((const char **)&pContent)==SEC_OK)
            {
                CXMLDOMNode * pNode     = NULL;
				CXMLDOMElement * pActionNode     = NULL;
                CXMLDOMElement *pRoot=pDom->GetRootElement();
                if(pRoot)
                {
					pRoot->getChildNodeByName(&pActionNode,"ACTION");
					if (pActionNode)
					{

						pActionNode->get_firstChild(&pNode);
						if (pNode)
						{
							char *pName=NULL;
							pNode->get_nodeName(&pName);
							if(pName)
							{
								if (strcmp(pName,"KEEP_ALIVE")==0)
								{
									const COsQueue*pQ=CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessQAAPI, eManager);

									CSegment* pMsg = new CSegment; // pMsg will delete inside
									*pMsg  << (DWORD)strlen(pContent);
									*pMsg  << pContent;


									CTaskApi api;
									api.CreateOnlyApi(*pQ);
									api.SendMsg(pMsg,EXT_DB_RESPONSE);
									api.DestroyOnlyApi();
								}
								else if (strcmp(pName,"ADD")==0)
								{
									int token;
									FTRACESTR(eLevelInfoNormal) << "CQAAPIManager::ReceiveFromSocket in add";
									if (pRoot->getChildNodeDecValueByName("TOKEN",&token)==SEC_OK)
									{
										CSegment* pMsg = new CSegment; // pMsg will delete inside
										*pMsg  << (DWORD)EXT_DB_PWD_CONFIRM;
										*pMsg  << (DWORD)strlen(pContent);
										*pMsg  << pContent;

										CPartyIdTaskApi api(token,eProcessConfParty);
										api.SendMsg(pMsg,XML_EVENT);
										api.DestroyOnlyApi();
										FTRACESTR(eLevelInfoNormal) << "CQAAPIManager::ReceiveFromSocket in add , send";
									}
									else
									{
										FTRACESTR(eLevelInfoNormal) << "CQAAPIManager::ReceiveFromSocket - token is missing - content... " << pContent;
									}
								}
								else if (strcmp(pName,"CREATE")==0)
								{
									int token;
									FTRACESTR(eLevelInfoNormal) << "CQAAPIManager::ReceiveFromSocket in CREATE";
									if (pRoot->getChildNodeDecValueByName("TOKEN",&token)==SEC_OK)
									{
										CSegment* pMsg = new CSegment; // pMsg will delete inside
										*pMsg  << (DWORD)EXT_DB_CONF_CONFIRM;
										*pMsg  << (DWORD)strlen(pContent);
										*pMsg  << pContent;

										CPartyIdTaskApi api(token,eProcessConfParty);
										api.SendMsg(pMsg,XML_EVENT);
										api.DestroyOnlyApi();
										FTRACESTR(eLevelInfoNormal) << "CQAAPIManager::ReceiveFromSocket in Create , send";
									}
									else
									{
										FTRACESTR(eLevelInfoNormal) << "CQAAPIManager::ReceiveFromSocket - token is missing - content... " << pContent;
									}
								}
								else if (strcmp(pName,"AUTHENTICATE")==0)
								{
									int token;
									if (pRoot->getChildNodeDecValueByName("TOKEN",&token)==SEC_OK)
									{
										CSegment* pMsg = new CSegment; // pMsg will delete inside
										*pMsg  << (DWORD)strlen(pContent);
										*pMsg  << pContent;

										CTaskApi api;
										const COsQueue* pAuthenticationManager = CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessAuthentication,eManager);
										api.CreateOnlyApi(*pAuthenticationManager);
										//ResponedClientRequest(STATUS_OK,pMsg);
										api.SendMsg(pMsg,EXT_DB_USER_LOGIN_CONFIRM);
										//api.DestroyOnlyApi();
									}
									else
									{
										FTRACESTR(eLevelInfoNormal) << "CQAAPIManager::ReceiveFromSocket - token is missing - content... " << pContent;
									}
								}
							}
						}
					}
                }
            }
            delete pDom;
		}

        PDELETEA(pContent);
}
/////////////////////////////////////////////////////////////////////////////
void CQAAPIManager::OnExtDbResponseAny(CSegment* pMsg)
{
	WORD   RetOpCode;
	DWORD len;

	*pMsg >> len;
	char *pXMLResponse=new char [len+1];
    *pMsg >> pXMLResponse;
	pXMLResponse[len]='\0';

    if(strcmp(pXMLResponse,"KEEP_ALIVE"))
    {
        ZeroKACnt();
    }

	FTRACESTR(eLevelInfoNormal) << "CQAAPIManager::OnExtDbResponseAny \n " << pXMLResponse;

	PDELETEA(pXMLResponse);
}

/////////////////////////////////////////////////////////////////////////////
//establish connection with external server
/////////////////////////////////////////////////////////////////////////////
void CQAAPIManager::OnSocketConnectIdle(CSegment* pMsg)
{
	if( !CPObject::IsValidPObjectPtr(m_pSocketConnection) && !m_pAppServerData->IsConnectionlessTransport())
		return;

	m_state = CONNECTING;
	if (m_pAppServerData->IsConnectionlessTransport())
	{
		CSegment seg;
		seg << m_pKeepAliveReq;
		HandleSendExtDBReqLocaly(&seg);
	}
	else
		m_pSocketConnection->Connect();
}


/////////////////////////////////////////////////////////////////////////////
// Socket: connection established

/////////////////////////////////////////////////////////////////////////////
void CQAAPIManager::OnSocketConnectedSetup(CSegment* pMsg)
{
	FTRACESTR(eLevelInfoNormal) << "CQAAPIManager::OnSocketConnectedSetup - connected to external server - " << m_pAppServerData->GetUrl();
	m_state = CONNECTED;
	if(IsActiveAlarmExistByErrorCode(AA_CAN_NOT_ESTABLISH_CONNECTION_WITH_APPLICATION_SERVER))
		RemoveActiveAlarmByErrorCode(AA_CAN_NOT_ESTABLISH_CONNECTION_WITH_APPLICATION_SERVER);
	StartTimer(SERVER_KEEP_ALIVE_TIMER, SECOND);
}


/////////////////////////////////////////////////////////////////////////////
// Socket: connection failed

/////////////////////////////////////////////////////////////////////////////
void CQAAPIManager::OnSocketFailedSetup(CSegment* pMsg)
{
	FTRACESTR(eLevelInfoNormal) << "CQAAPIManager::OnSocketFailedSetup - cant establish connection with application server - " << m_pAppServerData->GetUrl();

	std::string str;

	str= "Failed to establish connection to server , url = " ;
	str+= m_pAppServerData->GetUrl();

	if(! IsActiveAlarmExistByErrorCode(AA_CAN_NOT_ESTABLISH_CONNECTION_WITH_APPLICATION_SERVER))
	{
		AddActiveAlarm(	FAULT_GENERAL_SUBJECT,
					AA_CAN_NOT_ESTABLISH_CONNECTION_WITH_APPLICATION_SERVER,
					MAJOR_ERROR_LEVEL,
					str.c_str(),
					true,
					true);
	}

	// endless loop
	if (m_pAppServerData->IsConnectionlessTransport())
	{
		StartTimer(SERVER_KEEP_ALIVE_TIMER, 10 * SECOND);
	}
	else if( CPObject::IsValidPObjectPtr(m_pSocketConnection))
		m_pSocketConnection->Connect();
}


/////////////////////////////////////////////////////////////////////////////
// Socket: connection dropped
//

/////////////////////////////////////////////////////////////////////////////
void CQAAPIManager::OnSocketDroppedSetup(CSegment* pMsg)
{
	FTRACESTR(eLevelInfoNormal) << "CQAAPIManager::OnSocketDroppedSetup";
	DBGPASSERT(1);
}

/*/////////////////////////////////////////////////////////////////////////////
void CQAAPIManager::OnSocketPauseConnect(CSegment* pMsg)
{
	PTRACE2(eLevelInfoNormal,"CQAAPIManager::OnSocketPauseConnect - stop socket for testing reasones: server: ",m_pAppServerData->GetUrl());
	OnSocketPause(pMsg);
}*/
/////////////////////////////////////////////////////////////////////////////
void CQAAPIManager::OnSocketDroppedConnect(CSegment* pMsg)
{
	FTRACESTR(eLevelInfoNormal) << "CQAAPIManager::OnSocketDroppedConnect - temporary lose of connection with external server: " << m_pAppServerData->GetUrl();
	OnSocketDropped(pMsg);
}


/////////////////////////////////////////////////////////////////////////////
void CQAAPIManager::OnSocketDropped(CSegment* pMsg)
{
	m_state = CONNECTING;

	if( CPObject::IsValidPObjectPtr(m_pSocketConnection) || m_pAppServerData->IsConnectionlessTransport())
	{
		if (!m_pAppServerData->IsConnectionlessTransport())
			m_pSocketConnection->Disconnect();
		else
		{
			if(! IsActiveAlarmExistByErrorCode(AA_CAN_NOT_ESTABLISH_CONNECTION_WITH_APPLICATION_SERVER))
			{
				std::string str = "temporary lose of connection with external server: ";
				str+= m_pAppServerData->GetUrl();
				AddActiveAlarm(	FAULT_GENERAL_SUBJECT,
							AA_CAN_NOT_ESTABLISH_CONNECTION_WITH_APPLICATION_SERVER,
							MAJOR_ERROR_LEVEL,
							str.c_str(),
							true,
							true);
			}
		}


		if (!m_pAppServerData->IsConnectionlessTransport())
		{
			DWORD delay = 1;
			CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey("QAAPI_RECONNECT_DELAY", delay);
			SystemSleep(delay * SECOND);
			m_pSocketConnection->Connect();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Socket: message received
//

/////////////////////////////////////////////////////////////////////////////
void CQAAPIManager::OnSocketRcvIdle(CSegment* pMsg)
{
	FTRACESTR(eLevelInfoNormal) << "CQAAPIManager::OnSocketRcvIdle";
	DBGPASSERT(1);
}

/////////////////////////////////////////////////////////////////////////////
void CQAAPIManager::OnSocketRcvSetup(CSegment* pMsg)
{
	FTRACESTR(eLevelInfoNormal) << "CQAAPIManager::OnSocketRcvSetup";
	DBGPASSERT(1);
}

/////////////////////////////////////////////////////////////////////////////
void CQAAPIManager::OnSocketRcvConnect(CSegment* pMsg)
{
	FTRACESTR(eLevelInfoNormal) << "CQAAPIManager::OnSocketRcvConnect";
	//add my code in here
}


void CQAAPIManager::BuildKeepAliveRequest()
{

    CXMLDOMElement* pTempNode;
    CXMLDOMElement* pRootNode;
    char *pStrResponse=NULL;
    int status;
	DWORD mngmntIP = 0xffffffff;


	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

	 if (eProductTypeRMX4000 == curProductType || eProductTypeRMX1500 == curProductType)
		 m_sMngmntNI = GetLogicalInterfaceName(eManagmentNetwork,m_curIpType);
	 else
		 m_sMngmntNI = GetLogicalInterfaceName(eManagmentNetwork,eIpType_IpV4);

	if (!IsTarget())
	{
		m_sMngmntNI = "eth0";
	}

	status = GetNiIpV4Addr(m_sMngmntNI.c_str(), &mngmntIP);

	if(IFCONFIG_SOCKET_FAIL == status)
	{
		FTRACEINTO << "CQAAPIManager::BuildKeepAliveRequest FAILED : Failed to open socket during access to Network Interface";
		PASSERTMSG(TRUE, "CQAAPIManager::BuildKeepAliveRequest FAILED : Failed to open socket during access to Network Interface");
	}
	else if(IFCONFIG_IOCTL_FAIL == status)
	{
		FTRACEINTO << "CQAAPIManager::BuildKeepAliveRequest FAILED : Failed to IOCTL during access to Network Interface";
		PASSERTMSG(TRUE, "CQAAPIManager::BuildKeepAliveRequest FAILED : Failed to IOCTL during access to Network Interface");
	}

    pRootNode= new CXMLDOMElement("REQUEST_GENERAL");

    pTempNode = pRootNode->AddChildNode("LOGIN");

/*	if (status == IFCONFIG_OK)
	{
		char ip [16];
		SystemDWORDToIpString(mngmntIP,ip);
		pTempNode->AddChildNode("MCU_IP",ip );
	}
	else*/
		pTempNode->AddChildNode("MCU_IP","" );
    pTempNode->AddChildNode("USER_NAME", m_pAppServerData->Getlogin());
    pTempNode->AddChildNode("PASSWORD", m_pAppServerData->GetPwd());
    pTempNode = pRootNode->AddChildNode("ACTION");
    pTempNode = pTempNode->AddChildNode("KEEP_ALIVE");

    pRootNode->DumpDataAsLongStringEx(&pStrResponse);

    delete pRootNode;

    DEALLOCBUFFER(m_pKeepAliveReq);
    m_pKeepAliveReq=pStrResponse;



}

////////////////////////////////////////////////////////////////////////////
void CQAAPIManager::ManagerPostInitActionsPoint()
{
	m_pAppServerData = new CAppServerData();
	m_pAppServerData->Create();

	TestAndEnterFipsMode();

	CProcessBase* proc = CProcessBase::GetProcess();
	PASSERT_AND_RETURN(NULL == proc);

	proc->m_NetSettings.LoadFromFile();
	m_MngmntIp = proc->m_NetSettings.m_ipv4;
	m_curIpType = proc->m_NetSettings.m_iptype;
	//End Startup Feature

}

/////////////////////////////////////////////////////////////////////
void CQAAPIManager::DeclareStartupConditions()
{
	CActiveAlarm aa(FAULT_GENERAL_SUBJECT,
					AA_SYSTEM_CONFIGURATION,
					MAJOR_ERROR_LEVEL,
					"Invalid external DB configuration",
					true,
					true);
	AddStartupCondition(aa);
}

////////////////////////////////////////////////////////////////////////////
void CQAAPIManager::ManagerStartupActionsPoint()
{
	if(m_pAppServerData->IsExternalDBAccessEnabled())
	{
		if (m_pAppServerData->IsExternalDBCnfgValid())
		{
			if (m_pAppServerData->GetPort()==443 || m_pAppServerData->GetPort()==4433)
				m_isSecured = TRUE;
			else
				m_isSecured = FALSE;

            if (m_isSecured)
            {
            	if (m_pAppServerData->IsConnectionlessTransport())
            		m_pcurlhttpConn = new CurlHTTP(true,true,true);
            	else
            		m_pSocketConnection = new CClientSocket(this,QAAPIRxEntryPoint,QAAPITxEntryPoint,new CSecuredSocketClient);
            }
            else
            {
            	if (m_pAppServerData->IsConnectionlessTransport())
            		m_pcurlhttpConn = new CurlHTTP(false,true,true);
            	else
            		m_pSocketConnection = new CClientSocket(this,QAAPIRxEntryPoint,QAAPITxEntryPoint);
            }
			BuildKeepAliveRequest();
			 eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
			 if((m_pcurlhttpConn != NULL)&&(eProductTypeRMX2000 == curProductType)&& IsTarget())
			 {
				     m_pcurlhttpConn->SetOutGoingInterface(m_sMngmntNI);
					 TRACESTR(eLevelInfoNormal) << " - ManagerStartupActionsPoint  set out going interface - " <<  m_sMngmntNI.c_str();
			 }
			RemoveActiveAlarmByErrorCode(AA_SYSTEM_CONFIGURATION);
			m_state=CONNECTING;
        	if (m_pAppServerData->IsConnectionlessTransport())
        	{
        		//TBD
        		CURLcode retVal;
        		const string strReq = m_pKeepAliveReq;
        		UnlockRelevantSemaphore();
        		m_pcurlhttpConn->PostRequest(retVal,m_pAppServerData->GetUrl(),strReq);
        		LockRelevantSemaphore();
        		if (CURLE_OK == retVal) //start timer
        			OnSocketConnectedSetup(NULL);
        		else
        		{
        			FTRACESTR(eLevelWarn) << "CQAAPIManager::ManagerStartupActionsPoint - connected to external server Failed status: " << retVal;
        			CSegment respSeq;
        			DispatchEvent(SOCKET_FAILED,&respSeq);
        		}
        	}
        	else
        	{
				m_pSocketConnection->Init(m_pAppServerData->GetStrIp(),
										  m_pAppServerData->GetPort());

				m_pSocketConnection->Init(m_pAppServerData->GetServerAddr());
				m_pSocketConnection->SetRetryTime(10 * SECOND);
				m_pSocketConnection->SetConnectionRetriesNumber(6);
				m_pSocketConnection->Connect();

        	}
		}
        else
        {
            UpdateStartupConditionByErrorCode(AA_SYSTEM_CONFIGURATION, eStartupConditionFail);
		}
	}
	else
	{
		RemoveActiveAlarmByErrorCode(AA_SYSTEM_CONFIGURATION);
	}
}

//////////////////////////////////////////////////////////////////////
BOOL	CQAAPIManager::selfKillOnMemoryExhaustion()
{
	FTRACEINTO << "CQAAPIManager::selfKillOnMemoryExhaustion";
	if(m_pSocketConnection)
	{
		m_pSocketConnection->Destroy();
	}
	return TRUE;
}



//////////////////////////////////////////////////////////////////////
STATUS CQAAPIManager::HandleTerminalSendLoginResp(CTerminalCommand & command,std::ostream& answer)
{
    const char *pContent = "<CONFIRM_USER_DETAILS><RETURN_STATUS><ID>8</ID><DESCRIPTION>Status unknown 8, Add it to StatusStringConvertor.cpp.</DESCRIPTION><YOUR_TOKEN1>0</YOUR_TOKEN1><YOUR_TOKEN2>0</YOUR_TOKEN2><MESSAGE_ID>0</MESSAGE_ID><DESCRIPTION_EX></DESCRIPTION_EX></RETURN_STATUS><TOKEN>1</TOKEN><ACTION><AUTHENTICATE/></ACTION></CONFIRM_USER_DETAILS>";

    CSegment* pMsg = new CSegment; // pMsg will delete inside
    *pMsg  << (DWORD)strlen(pContent);
    *pMsg  << pContent;

    CManagerApi api(eProcessAuthentication);
    api.SendMsg(pMsg,EXT_DB_USER_LOGIN_CONFIRM);

    return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CQAAPIManager::HandleTerminalGetConnState(CTerminalCommand & command,std::ostream& answer)
{
    answer << "State : " << ConnectionStateToString(m_state);
    return STATUS_OK;
}




