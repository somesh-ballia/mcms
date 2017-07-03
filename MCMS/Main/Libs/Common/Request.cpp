#include "Request.h"
#include "SerializeObject.h"
#include "psosxml.h"
#include "XmlDefines.h"
#include "InitCommonStrings.h"
#include "ProcessBase.h"
#include "ApiStatuses.h"
#include "TraceStream.h"
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////
//                        CRequest
////////////////////////////////////////////////////////////////////////////
CRequest::CRequest()
{
	m_mes_header        = 0;
	m_requestObject     = NULL;
	m_confirmObject     = NULL;
	m_conId             = 0;
	m_bXmlStream        = FALSE;
	m_nStatus           = STATUS_OK;
	m_ActionNodeType    = ACTION_GROUP;
	m_pQueue            = new COsQueue;
	m_UserToken1        = 0;
	m_UserToken2        = 0;
	m_ExtDbToken        = 0;
	m_nAuthorization    = SUPER;
	m_ifNoneMatch 		= 0;
	m_szErrorMessage[0] = '\0';
	m_objectFlag 		= STRING_FLAG;
	m_pXMLSetRequest	= NULL;
}

//--------------------------------------------------------------------------
CRequest::~CRequest()
{
	POBJDELETE(m_requestObject);
	POBJDELETE(m_confirmObject);
	POBJDELETE(m_pQueue);
}

//--------------------------------------------------------------------------
CRequest::CRequest(CXMLDOMElement* pXMLSetRequest, const CTransactionsFactory& transFactory)
{
	m_pXMLSetRequest    = pXMLSetRequest;
	m_bXmlStream        = TRUE;
	m_nStatus           = STATUS_OK;
	m_ActionNodeType    = ACTION_GROUP;
	m_pQueue            = new COsQueue;
	m_mes_header        = 0;
	m_UserToken1        = 0;
	m_UserToken2        = 0;
	m_ExtDbToken        = 0;
	m_requestObject     = NULL;
	m_confirmObject     = NULL;
	m_conId             = 0;
	m_nAuthorization    = SUPER;
	m_szErrorMessage[0] = '\0';
	m_ifNoneMatch = 0;
	m_nStatus           = DeserializeXml(transFactory);

}

//--------------------------------------------------------------------------
CXMLDOMElement* CRequest::SerializeXml()
{
	CXMLDOMElement* pXMLRequest      = new CXMLDOMElement;
	CXMLDOMElement* pActionsNode     = NULL;
	CXMLDOMElement* pActionNode      = NULL;
	CXMLDOMElement* pCommonParamNode = NULL;
	CXMLDOMElement* pAsyncNode       = NULL;

	pXMLRequest->set_nodeName(m_transName.c_str());

	pCommonParamNode = pXMLRequest->AddChildNode("TRANS_COMMON_PARAMS");
	pCommonParamNode->AddChildNode("MESSAGE_ID", m_mes_header);

	pAsyncNode = pCommonParamNode->AddChildNode("ASYNC");
	pAsyncNode->AddChildNode("YOUR_TOKEN1", m_UserToken1);
	pAsyncNode->AddChildNode("YOUR_TOKEN2", m_UserToken2);

	if (ACTION_ELEMENT == m_ActionNodeType)
	{
		pActionsNode = pXMLRequest->AddChildNode("ACTION");
		pActionNode  =  pActionsNode->AddChildNode(m_actionName);
	}
	else
	{
		pActionNode =  pXMLRequest->AddChildNode(m_actionName);
	}

	if (m_requestObject)
		m_requestObject->SerializeXml(pActionNode);

	return pXMLRequest;
}

//--------------------------------------------------------------------------
int CRequest::DeserializeXml(const CTransactionsFactory& transFactory)
{
	CXMLDOMElement* pActionsNode     = NULL;
	CXMLDOMElement* pActionNode      = NULL;
	CXMLDOMElement* pCommonParamNode = NULL;
	CXMLDOMElement* pAsyncNode       = NULL;
	char*           pActionName      = NULL;
	char*           pTransName       = NULL;
	BYTE            bActionValid     = TRUE;
	int             nStatus          = 0;
	char            pszError[100]    = {0};

	m_pXMLSetRequest->get_nodeName(&pTransName);
	if (pTransName)
		m_transName = pTransName;

	if (m_transName == "REQUEST_CONF_DETAILS" ||
	    m_transName == "REQUEST_PARTY_DETAILS" ||
	    m_transName == "REQUEST_USER_DETAILS")
		GET_VALIDATE_CHILD(m_pXMLSetRequest, "TOKEN", &m_ExtDbToken, _0_TO_DWORD);

	GET_CHILD_NODE(m_pXMLSetRequest, "ACTION", pActionNode);

	if (pActionNode)
	{
		pActionNode->firstChildNode(&pActionsNode);
		m_ActionNodeType = ACTION_ELEMENT;
	}
	else
	{
		m_pXMLSetRequest->firstChildNode(&pActionsNode);
		m_pXMLSetRequest->nextChildNode(&pActionsNode); // actions node
		m_ActionNodeType = ACTION_GROUP;
	}

	pActionsNode->get_nodeName(&pActionName);

	if (pActionName)
		m_actionName = pActionName;

	GET_CHILD_NODE(m_pXMLSetRequest, "TRANS_COMMON_PARAMS", pCommonParamNode);

	if (pCommonParamNode)
	{

		GET_VALIDATE_CHILD(pCommonParamNode, "MESSAGE_ID", &m_mes_header, _0_TO_DWORD);
		GET_VALIDATE_CHILD(pCommonParamNode,"MCU_TOKEN",&m_conId,_0_TO_DWORD);
		GET_CHILD_NODE(pCommonParamNode, "ASYNC", pAsyncNode);
		if (pAsyncNode)
		{
			GET_VALIDATE_CHILD(pAsyncNode, "YOUR_TOKEN1", &m_UserToken1, _0_TO_DWORD);
			GET_VALIDATE_CHILD(pAsyncNode, "YOUR_TOKEN2", &m_UserToken2, _0_TO_DWORD);
		}
	}
	else
	{
		m_nStatus = (WORD)STATUS_TRANSACTION_DOES_NOT_EXIST;
		if(pTransName)	
			snprintf(m_szErrorMessage, sizeof(m_szErrorMessage), "Trans name: %s", pTransName);
	}

	if (bActionValid == FALSE)
	{
		m_nStatus = (WORD)STATUS_ACTION_NODE_IS_MISSING_OR_INVALID;
		if (pTransName && pActionName)
			snprintf(m_szErrorMessage, sizeof(m_szErrorMessage), "Trans name: %s, Action name: %s", pTransName, pActionName);
	}

	if (pTransName && pActionName)
	{
		m_requestObject = transFactory.CreateTransaction(pTransName, pActionName);

		if (m_requestObject)
		{
			m_nStatus = m_requestObject->DeSerializeXml(pActionsNode, m_szErrorMessage, pActionName);
		}
		else
		{
			TRACESTR(eLevelError) << "CRequest::DeserializeXml - STATUS_NO_REQUEST_OBJECT"
		        	                 << pTransName << pActionName;
			SystemCoreDump(TRUE);
			m_nStatus = STATUS_NO_REQUEST_OBJECT;
		}
	}

	if (m_nStatus == STATUS_OK)
	{
		m_szErrorMessage[0] = '\0';
	}
	else
	{
		if (pTransName && pActionName)
		{
			TRACEINTO << "CRequest::DeserializeXml - Failed"
		  	        << ", Status:"     << m_nStatus
		  	        << ", TransName:"  << pTransName
		   	       << ", ActionName:" << pActionName;
		}
	}

	return m_nStatus;
}

//--------------------------------------------------------------------------
CSerializeObject* CRequest::GetRequestObject() const
{
	return m_requestObject;
}

//--------------------------------------------------------------------------
void CRequest::SetRequestObject(CSerializeObject* pRequestObj)
{
	POBJDELETE(m_requestObject); // Since RequestObject is allocated by Clone() at first.
	m_requestObject = pRequestObj;
}

//--------------------------------------------------------------------------
void CRequest::SetConfirmObject(CSerializeObject* pConfirmObj)
{
	// No new for the object in Skeleton like in Request Object -->
	// NOT really necessary just to be on the safe side
	POBJDELETE(m_confirmObject);
	m_confirmObject = pConfirmObj;
}

//--------------------------------------------------------------------------
CXMLDOMElement* CRequest::SetConfirmSerializeXml(std::string requestUserName)
{
	const char*     pStatus;

	CXMLDOMElement* pXMLResponse             = new CXMLDOMElement;
	CXMLDOMElement* pStatusNode              = NULL;
	CXMLDOMElement* pActionNode              = NULL;
	CXMLDOMElement* pActionsNode             = NULL;
	CXMLDOMElement* pStatusIdNode            = NULL;
	CXMLDOMElement* pStatusDescriptionNode   = NULL;
	CXMLDOMElement* pStatusDescriptionExNode = NULL;

	pStatusNode = pXMLResponse->AddChildNode("RETURN_STATUS");
	BYTE isWarning = NO;
	if (m_nStatus & WARNING_MASK)
	{
		isWarning  = YES;
		m_nStatus ^= WARNING_MASK;
	}

	pStatusIdNode = pStatusNode->AddChildNode("ID", m_nStatus);

	if (m_nStatus > 69999)
	{
		char err_msg[100];
		sprintf(err_msg, "CRequest::SetConfirmSerializeXml: status %d need to be defined in ApiStatuses", m_nStatus);
		// in case the status is internal - add assert.
		FPASSERTMSG(TRUE, err_msg);
	}

	pStatusDescriptionNode = pStatusNode->AddChildNode("DESCRIPTION", CProcessBase::GetProcess()->GetStatusAsString(m_nStatus).c_str());
	pStatusNode->AddChildNode("YOUR_TOKEN1", m_UserToken1);
	pStatusNode->AddChildNode("YOUR_TOKEN2", m_UserToken2);

	if (isWarning)
		pStatusNode->AddChildNode("WARNING", isWarning, _BOOL);

	pStatusNode->AddChildNode("MESSAGE_ID", m_mes_header);

	pStatusDescriptionExNode = pStatusNode->AddChildNode("DESCRIPTION_EX", m_szErrorMessage);

	if (m_ExtDbToken)
		pXMLResponse->AddChildNode("TOKEN", m_ExtDbToken);

	switch (m_ActionNodeType)
	{
		case ACTION_GROUP:
			pActionsNode = pXMLResponse->AddChildNode(m_actionName);
			break;

		case ACTION_ELEMENT:
			pActionNode  = pXMLResponse->AddChildNode("ACTION");
			pActionsNode = pActionNode->AddChildNode(m_actionName);
			break;
	} // switch

	if (m_confirmObject)
	{
		std::string responseTrancsName("RESPONSE_");
		if (m_transName == "TRANS_RES_1" || m_transName == "TRANS_RES_2" ||
		    m_transName == "TRANS_CONF_1" || m_transName == "TRANS_CONF_2")
		{
			int size = m_transName.size()-2;
			responseTrancsName += m_transName.substr(0, size);
		}
		// for External DB simulation - start
		else if (m_transName == "REQUEST_GENERAL")
			responseTrancsName = "CONFIRM_GENERAL";
		else if (m_transName == "REQUEST_CONF_DETAILS")
			responseTrancsName = "CONFIRM_CONF_DETAILS";
		else if (m_transName == "REQUEST_PARTY_DETAILS")
			responseTrancsName = "CONFIRM_PARTY_DETAILS";
		else if (m_transName == "REQUEST_USER_DETAILS")
			responseTrancsName = "CONFIRM_USER_DETAILS";
		// for External DB simulation - end
		else
			responseTrancsName += m_transName;

		pXMLResponse->set_nodeName(responseTrancsName.c_str());

		if (m_nStatus == STATUS_OK || isWarning)
		{
			if (strstr(m_actionName.c_str(), "LOGIN"))
			{
				pActionsNode->AddChildNode("MCU_TOKEN", m_conId);
				pActionsNode->AddChildNode("MCU_USER_TOKEN", m_conId);
			}

			if ("GET_OPER_LIST" == m_actionName ||
			    ("TRANS_CONNECTIONS_LIST" == m_transName && "GET" == m_actionName))
				pActionsNode->AddChildNode("REQ_USER", requestUserName);

			m_confirmObject->SerializeXml(pActionsNode);
		}
	}
	else
	{
		WORD nAction = SET_REQUEST;

		CTaskApp* pTaskApp = CProcessBase::GetProcess()->GetCurrentTask();

		if (pTaskApp && strcmp(pTaskApp->GetTaskName(), "Monitor") == 0)
			nAction = GET_REQUEST;

		pXMLResponse = ::BuildGeneralResponse((STATUS)STATUS_OBJECT_NOT_RECOGNIZED,
		                                      "Unrecognized object",
		                                      NULL,
		                                      m_mes_header,
		                                      nAction,
		                                      m_UserToken1,
		                                      m_UserToken2);
	}

	char* pStrError;

	if (pXMLResponse->MsValidate(&pStrError) == FALSE)
	{
		// returns FALSE only in NT not in pSos
		pStatusIdNode->SetValue(STATUS_FAIL_TO_VALIDATE_RESPONSE);
		pStatus = CProcessBase::GetProcess()->GetStatusAsString(STATUS_FAIL_TO_VALIDATE_RESPONSE).c_str();
		pStatusDescriptionNode->SetValue(pStatus);
		if (pStrError)
		{
			pStatusDescriptionExNode->SetValue(pStrError);
			delete [] pStrError;
		}
	}

	return pXMLResponse;
}

//--------------------------------------------------------------------------
void CRequest::DumpSetRequestAsString(char** ppSetRequestString)
{
	if (m_pXMLSetRequest)
		m_pXMLSetRequest->DumpDataAsLongStringEx(ppSetRequestString);
}
