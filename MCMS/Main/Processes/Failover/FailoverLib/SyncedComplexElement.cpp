// CSyncedComplexElement: implementation of the CSyncedComplexElement class.
//
//////////////////////////////////////////////////////////////////////


#include "SyncedComplexElement.h"
#include "TraceStream.h"
#include "FailoverProcess.h"
#include "FailoverCommunication.h"
#include "ManagerApi.h"
#include "OpcodesMcmsInternal.h"
#include "CommResApi.h"
#include "RsrvManagerApi.h"

/////////////////////////////////////////////////////////////////////////////
CSyncedComplexElement::CSyncedComplexElement()
{
	m_trans2TransName = NULL;
	m_trans2ActionName = NULL;
	m_trans2ResponseObjectName = NULL;

	m_addOrUpdateOpcode = 0;
	m_delOpcode    	    = 0;

	m_eDestProcess = eProcessTypeInvalid;
}

/////////////////////////////////////////////////////////////////////////////
void CSyncedComplexElement::HandleChange(CXMLDOMElement *pRootElem)
{
	int numOfDel=0, numOfChanged=0;

	// ===== 1. new/updated
	HandleChangeIfNeeded(pRootElem, eChange_AddOrUpdate, numOfChanged);
	TRACESTR(eLevelInfoNormal) << "CSyncedComplexElement::HandleChange"
						   << "\n" << numOfChanged	<< " were changed (added/updated)";

	// ===== 2. deleted
	HandleChangeIfNeeded(pRootElem, eChange_Delete, numOfDel);
	TRACESTR(eLevelInfoNormal) << "CSyncedComplexElement::HandleChange"
						   << "\n" << numOfDel << " were deleted";
}

/////////////////////////////////////////////////////////////////////////////
void CSyncedComplexElement::SendTrans2(DWORD id)
{
	TRACESTR(eLevelInfoNormal) << "CSyncedComplexElement::SendTrans2 (action: " << m_trans2ActionName << ", Id: " << id << ")";

	char* trans2Str = NULL;
	PrepareTrans2Str(id, trans2Str);
	SendTrans(trans2Str);
	DEALLOCBUFFER(trans2Str);
}

///////////////////////////////////////////////////////////////////////////
void CSyncedComplexElement::PrepareTrans2Str(DWORD id, char* &transStr)
{
	DWORD mcuToken	= m_pProcess->GetMcuToken();
	DWORD msgId		= m_pProcess->GetAndIncreaseMsgId();

	CXMLDOMElement *pRootNode = NULL;
	pRootNode = new CXMLDOMElement(m_trans2TransName);

	CXMLDOMElement *pTempNode = NULL;
	pTempNode = pRootNode->AddChildNode("TRANS_COMMON_PARAMS");
	pTempNode->AddChildNode("MCU_TOKEN", mcuToken);
	pTempNode->AddChildNode("MCU_USER_TOKEN", mcuToken);
	pTempNode->AddChildNode("MESSAGE_ID", msgId);
	pTempNode = pRootNode->AddChildNode("ACTION");
	pTempNode = pTempNode->AddChildNode(m_trans2ActionName);
	pTempNode->AddChildNode("ID", id);
    pTempNode->AddChildNode("OBJ_TOKEN", -1);

	pRootNode->DumpDataAsLongStringEx(&transStr);

	if (transStr)
	{
		TRACESTR(eLevelInfoNormal) << "CSyncedComplexElement::PrepareTrans2Str"
								<< "\n" << transStr;
	}
	else
	{
		TRACESTR(eLevelInfoNormal) << "CSyncedComplexElement::PrepareTrans2Str"
							   << "\ndelConfStr = NULL";
	}

	PDELETE(pRootNode);
}

/////////////////////////////////////////////////////////////////////////////
void CSyncedComplexElement::SendDelInd(DWORD id)
{
	TRACESTR(eLevelInfoNormal) << "CSyncedComplexElement::SendDelInd (Id: " << id << ")";

	// ===== insert the string to a segment
	CSegment* pSeg = new CSegment();
	*pSeg << id;

	// ===== send to ConfParty / RA
	if (m_eDestProcess == eProcessConfParty)
	{
		CManagerApi api(m_eDestProcess);
		api.SendMsg(pSeg, m_delOpcode);
	}
	else if (m_eDestProcess == eProcessResource)
	{
		CRsrvManagerApi api;
		api.SendMsg(pSeg, m_delOpcode);
	}
	else
	{
		TRACESTR(eLevelInfoNormal) << "SyncedComplexElement::SendDelInd  -  illegal process"
								   << m_eDestProcess << "\n";
		POBJDELETE(pSeg);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CSyncedComplexElement::SendAddOrUpdateInd(CXMLDOMElement *pElemNode)
{
	CSegment* pSeg = new CSegment();

	if (m_eDestProcess == eProcessConfParty)
	{
		char* elementStr = NULL;
		pElemNode->DumpDataAsLongStringEx(&elementStr);
		*pSeg << (DWORD)strlen(elementStr);
		*pSeg << elementStr;

		TRACESTR(eLevelInfoNormal) << "SyncedComplexElement::SendAddOrUpdateInd  to ConfParty -  the string:"
								   << "\n" << elementStr;

		CManagerApi api(eProcessConfParty);
		api.SendMsg(pSeg, m_addOrUpdateOpcode);

		DEALLOCBUFFER(elementStr);
	}

	else if (m_eDestProcess == eProcessResource)
	{
		CSegment *pSeg = new CSegment;

		CCommResApi* pCommResApi = new CCommResApi;
		char szErrorMsg[ERROR_MESSAGE_LEN];
		pCommResApi->DeSerializeXml(pElemNode, szErrorMsg, ADD_RESERVE);

		pCommResApi->Serialize(NATIVE, *pSeg);
		POBJDELETE(pCommResApi);

		POBJDELETE(pCommResApi);
		CRsrvManagerApi api;
		api.SendMsg(pSeg, m_addOrUpdateOpcode);
	}

	else
	{
		TRACESTR(eLevelInfoNormal) << "SyncedComplexElement::SendAddOrUpdateInd  -  illegal process"
							   << m_eDestProcess << "\n";
		POBJDELETE(pSeg);
		return;
	}
}

/////////////////////////////////////////////////////////////////////////////
void CSyncedComplexElement::HandleResponseTrans2(CXMLDOMElement *pRootElem)
{
	CXMLDOMElement *pActionNode		= NULL,
				   *pSpecActionNode	= NULL,
				   *pElemNode		= NULL,
				   *pConfElemNode	= NULL;

	pRootElem->getChildNodeByName(&pActionNode, "ACTION");
	if (pActionNode)
	{
		pActionNode->getChildNodeByName(&pSpecActionNode, m_trans2ActionName);
		if (pSpecActionNode)
		{
			pSpecActionNode->getChildNodeByName(&pElemNode, m_trans2ResponseObjectName);
			if (pElemNode)
			{
				SendAddOrUpdateInd(pElemNode);
			} // end if pElemNode
		} // end if pSpecActionNode
	} // end if pActionNode
}
