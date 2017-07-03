// CSyncedElementMngmntService.cpp: implementation of the CSyncedElementMngmntService class.
//
//////////////////////////////////////////////////////////////////////


#include "SyncedElementMngmntService.h"
#include "TraceStream.h"
#include "FailoverProcess.h"
#include "FailoverCommunication.h"
#include "ManagerApi.h"
#include "OpcodesMcmsInternal.h"


/////////////////////////////////////////////////////////////////////////////
CSyncedElementMngmntService::CSyncedElementMngmntService()
{
	SetElementFileds();
	//PrepareBasicGetTransStr();
}

////////////////////////////////////////////////////////////////////////////
void CSyncedElementMngmntService::SetElementFileds()
{
	m_transName  = "TRANS_IP_SERVICE_LIST";
	m_actionName = "GET_MANAGEMENT_NETWORK_LIST";
}

/////////////////////////////////////////////////////////////////////////////
void CSyncedElementMngmntService::HandleChange(CXMLDOMElement *pRootElem)
{
	TRACESTR(eLevelInfoNormal) << "\nCSyncedElementMngmntService::HandleChange";	

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
			pSpecActionNode->getChildNodeByName(&pListNode, "IP_SERVICE_LIST");

			if (pListNode)
			{
				GET_FIRST_CHILD_NODE(pListNode, "IP_SERVICE", pElemNode);

				DWORD changedConfId = 0;
				char* changedConfIdStr;

				while(pElemNode)
				{
					// ===== 1. prepare the info
					char* serviceElementStr = NULL;
					pElemNode->DumpDataAsLongStringEx(&serviceElementStr);
					
					TRACESTR(eLevelInfoNormal) << "\nCSyncedElementMngmntService::HandleChange  -  the string:"
										   << "\n" << serviceElementStr;
					
					// ===== 2. insert the string to a segment
					CSegment* pSeg = new CSegment();

					*pSeg << (DWORD)strlen(serviceElementStr);
					*pSeg << serviceElementStr;

					// ===== 3. send to CSMngr
					CManagerApi api(eProcessMcuMngr);
					api.SendMsg(pSeg, FAILOVER_MCUMNGR_UPDATE_MNGMNT_IND);

					DEALLOCBUFFER(serviceElementStr);

					GET_NEXT_CHILD_NODE(pListNode, "IP_SERVICE", pElemNode);
				}
			} // end if pListNode
		} // end if pSpecActionNode
	} // end if pActionNode
}
