// SyncedElementIvrServices.cpp: implementation of the CSyncedElementIvrServices class.
//
//////////////////////////////////////////////////////////////////////


#include "SyncedElementIvrServices.h"
#include "TraceStream.h"
#include "FailoverProcess.h"
#include "FailoverCommunication.h"
#include "ManagerApi.h"
#include "OpcodesMcmsInternal.h"


/////////////////////////////////////////////////////////////////////////////
CSyncedElementIvrServices::CSyncedElementIvrServices()
{
	SetElementFileds();
	//PrepareBasicGetTransStr();
}

////////////////////////////////////////////////////////////////////////////
void CSyncedElementIvrServices::SetElementFileds()
{
	m_transName  = "TRANS_AV_MSG_SERVICE_LIST";
	m_actionName = "GET_IVR_LIST";
}

/////////////////////////////////////////////////////////////////////////////
void CSyncedElementIvrServices::HandleChange(CXMLDOMElement *pRootElem)
{
	TRACESTR(eLevelInfoNormal) << "\nCSyncedElementIvrServices::HandleChange";

	CXMLDOMElement *pActionNode		= NULL,
				   *pSpecActionNode	= NULL,
				   *pListNode		= NULL,
				   *pElemNode		= NULL;;

	
	pRootElem->getChildNodeByName(&pActionNode, "ACTION");
	if (pActionNode)
	{
		pActionNode->getChildNodeByName(&pSpecActionNode, m_actionName);
		if (pSpecActionNode)
		{
			pSpecActionNode->getChildNodeByName(&pListNode, "AV_SERVICE_LIST");

			if (pListNode)
			{
				// ===== 1. prepare the info
				char* elementStr = NULL;
				pListNode->DumpDataAsLongStringEx(&elementStr);

				// ===== 2. insert the string to a segment
				CSegment* pSeg = new CSegment();

				DWORD len = strlen(elementStr);
				*pSeg << len;
				*pSeg << elementStr;

				// ===== 3. send to ConfParty
				CManagerApi api(eProcessConfParty);
				STATUS status = api.SendMsg(pSeg, FAILOVER_CONFPARTY_IVR_SERVICE_LIST_IND);

				TRACESTR(eLevelInfoNormal) << "\nCSyncedElementIvrServices::HandleChange - len = " << len << ", status = " << status;

				DEALLOCBUFFER(elementStr);
			} // end if pListNode
		} // end if pSpecActionNode
	} // end if pActionNode
}
