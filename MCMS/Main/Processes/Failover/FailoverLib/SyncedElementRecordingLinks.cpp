// CSyncedElementRecordingLinks.cpp: implementation of the CSyncedElementRecordingLinks class.
//
//////////////////////////////////////////////////////////////////////


#include "SyncedElementRecordingLinks.h"
#include "TraceStream.h"
#include "FailoverProcess.h"
#include "FailoverCommunication.h"
#include "ManagerApi.h"
#include "OpcodesMcmsInternal.h"


/////////////////////////////////////////////////////////////////////////////
CSyncedElementRecordingLinks::CSyncedElementRecordingLinks()
{
	SetElementFileds();
	//PrepareBasicGetTransStr();
}

////////////////////////////////////////////////////////////////////////////
void CSyncedElementRecordingLinks::SetElementFileds()
{
	m_transName  = "TRANS_RECORDING_LINKS_LIST";
	m_actionName = "GET";
}

/////////////////////////////////////////////////////////////////////////////
void CSyncedElementRecordingLinks::HandleChange(CXMLDOMElement *pRootElem)
{
	TRACESTR(eLevelInfoNormal) << "\nCSyncedElementRecordingLinks::HandleChange";

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
			pSpecActionNode->getChildNodeByName(&pListNode, "RECORDING_LINKS_LIST");

			if (pListNode)
			{
				// ===== 1. prepare the info
				char* elementStr = NULL;
				pListNode->DumpDataAsLongStringEx(&elementStr);

				TRACESTR(eLevelInfoNormal) << "\nCSyncedElementRecordingLinks::HandleChange  -  the string:"
									   << "\n" << elementStr;

				// ===== 2. insert the string to a segment
				CSegment* pSeg = new CSegment();

				*pSeg << (DWORD)strlen(elementStr);
				*pSeg << elementStr;

				// ===== 3. send to ConfParty
				CManagerApi api(eProcessConfParty);
				api.SendMsg(pSeg, FAILOVER_CONFPARTY_RECORDING_LINKS_LIST_IND);

				DEALLOCBUFFER(elementStr);
			} // end if pListNode
		} // end if pSpecActionNode
	} // end if pActionNode
}
