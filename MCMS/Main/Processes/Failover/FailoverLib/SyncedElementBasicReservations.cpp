// CSyncedElementBasicReservations: implementation of the CSyncedElementBasicReservations class.
//
//////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include "SyncedElementBasicReservations.h"
#include "TraceStream.h"
#include "FailoverProcess.h"
#include "FailoverCommunication.h"
#include "ManagerApi.h"
#include "OpcodesMcmsInternal.h"


/////////////////////////////////////////////////////////////////////////////
CSyncedElementBasicReservations::CSyncedElementBasicReservations()
{
	m_transName  = "TRANS_RES_LIST";

	m_responseSummaryLsNodeName   = NULL;
	m_responseSummaryNodeName     = NULL;

	m_trans2TransName          = "TRANS_RES_2";
	m_trans2ResponseObjectName = "RESERVATION";
}

/////////////////////////////////////////////////////////////////////////////
void CSyncedElementBasicReservations::HandleChangeIfNeeded(CXMLDOMElement *pRootElem, eChangeType changeType, int &numOfChanged)
{
	CXMLDOMElement *pActionNode		= NULL,
				   *pSpecActionNode	= NULL,
				   *pDeletesListNode= NULL,
				   *pResSummaryNode	= NULL,
				   *pIdNode		    = NULL,
				   *pChangedNode	= NULL;

	char* resChangedStr;
	
	pRootElem->getChildNodeByName(&pActionNode, "ACTION");
	if (pActionNode)
	{
		pActionNode->getChildNodeByName(&pSpecActionNode, m_actionName);
		if (pSpecActionNode)
		{
			pSpecActionNode->getChildNodeByName(&pSpecActionNode, m_responseSummaryLsNodeName);
			if (pSpecActionNode)
			{
				if (eChange_AddOrUpdate == changeType)
				{
					GET_FIRST_CHILD_NODE(pSpecActionNode, m_responseSummaryNodeName, pResSummaryNode);
					while (pResSummaryNode)
					{
						pResSummaryNode->getChildNodeByName(&pChangedNode, "RES_CHANGE");
						
						if(pChangedNode)
						{
							pChangedNode->get_nodeValue(&resChangedStr);
							
							if (strcmp(resChangedStr, "none") != 0)
							{
								numOfChanged++;

								pResSummaryNode->getChildNodeByName(&pIdNode, "ID");
								if(pIdNode)
								{
									char* changedIdStr;
									pIdNode->get_nodeValue(&changedIdStr);
									DWORD changedId = atoi(changedIdStr);
									SendTrans2(changedId);
								}
							}
						}

						GET_NEXT_CHILD_NODE(pSpecActionNode, m_responseSummaryNodeName, pResSummaryNode);
					}
				}

				else // eChange_Delete == changeType
				{
					pSpecActionNode->getChildNodeByName(&pDeletesListNode, "DELETED_RES_LIST");

					if (pDeletesListNode)
					{
						GET_FIRST_CHILD_NODE(pDeletesListNode, "ID", pIdNode);
						DWORD changedId = 0;
						char* changedIdStr;

						while (pIdNode)
						{
							// ===== 1. count num of deleted res
							numOfChanged++;

							// ===== 2. send the indication for deleting the res
							pIdNode->get_nodeValue(&changedIdStr);
							changedId = atoi(changedIdStr);
							SendDelInd(changedId);

							GET_NEXT_CHILD_NODE(pDeletesListNode, "ID", pIdNode);
						}
					}
				}
			}// end if pSpecActionNode
		} // end if pSpecActionNode
	} // end if pActionNode
}

