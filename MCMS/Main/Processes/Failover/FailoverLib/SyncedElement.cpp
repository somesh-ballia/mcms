// CSyncedElement.cpp: implementation of the CSyncedElement class.
//
//////////////////////////////////////////////////////////////////////


#include "SyncedElement.h"
#include "TraceStream.h"
#include "FailoverProcess.h"
#include "FailoverCommunication.h"


/////////////////////////////////////////////////////////////////////////////
CSyncedElement::CSyncedElement()
{
	m_pProcess = (CFailoverProcess*)(CProcessBase::GetProcess());
	
	m_objToken			= -1;
	m_basicGetTransStr	= NULL;
	m_transName 		= NULL;
	m_actionName 		= NULL;
}

/////////////////////////////////////////////////////////////////////////////
CSyncedElement::~CSyncedElement()
{
	DEALLOCBUFFER(m_basicGetTransStr);
}

/////////////////////////////////////////////////////////////////////////////
void CSyncedElement::SendBasicGetTrans()
{
	PrepareBasicGetTransStr();
	SendTrans(m_basicGetTransStr);
	DEALLOCBUFFER(m_basicGetTransStr);
}

/////////////////////////////////////////////////////////////////////////////
void CSyncedElement::SendTrans(char* strToSend)
{
	if (strToSend)
	{
		CFailoverCommunication *m_pComm = m_pProcess->GetFailoverCommunication();
		if (m_pComm)
		{
			m_pComm->SendToSocket(strToSend);
		}
		else // m_pComm is NULL
		{
			TRACESTR(eLevelInfoNormal) << "\nCSyncedElement::SendTrans - FAIL - FailoverCommunication from process is NULL";
		}
	}

	else // strToSend is empty
	{
		TRACESTR(eLevelInfoNormal) << "\nCSyncedElement::SendTrans - FAIL - strToSend is empty";
	}

}

/////////////////////////////////////////////////////////////////////////////
void CSyncedElement::SetObjToken(int objToken)
{
	m_objToken = objToken;
}

/////////////////////////////////////////////////////////////////////////////
int CSyncedElement::GetObjToken()
{
	return m_objToken;
}

/////////////////////////////////////////////////////////////////////////////
void CSyncedElement::HandleResponseTransConf(CXMLDOMElement *pRootElem)
{
	return;
}

/////////////////////////////////////////////////////////////////////////////
void CSyncedElement::PrepareBasicGetTransStr()
{
	DWORD mcuToken	= m_pProcess->GetMcuToken();
	DWORD msgId		= m_pProcess->GetAndIncreaseMsgId();

	CXMLDOMElement* pRootNode =  NULL;
	pRootNode = new CXMLDOMElement(m_transName);

	CXMLDOMElement* pTempNode;
	pTempNode = pRootNode->AddChildNode("TRANS_COMMON_PARAMS");
	pTempNode->AddChildNode("MCU_TOKEN", mcuToken);
	pTempNode->AddChildNode("MCU_USER_TOKEN", mcuToken);
	pTempNode->AddChildNode("MESSAGE_ID", msgId);

	pTempNode = pRootNode->AddChildNode("ACTION");
	pTempNode = pTempNode->AddChildNode(m_actionName);
	pTempNode = pTempNode->AddChildNode("OBJ_TOKEN", m_objToken);

	pRootNode->DumpDataAsLongStringEx(&m_basicGetTransStr);

	PDELETE(pRootNode);
}
