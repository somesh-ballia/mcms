// CSyncedElementOngoingConfs.cpp: implementation of the CSyncedElementOngoingConfs class.
//
//////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include "SyncedElementOngoingConfs.h"
#include "TraceStream.h"
#include "FailoverProcess.h"
#include "FailoverCommunication.h"
#include "ManagerApi.h"
#include "OpcodesMcmsInternal.h"


/////////////////////////////////////////////////////////////////////////////
CSyncedElementOngoingConfs::CSyncedElementOngoingConfs()
{
	SetElementFileds();
	SetComplexElementFileds();
	//PrepareBasicGetTransStr();
}

////////////////////////////////////////////////////////////////////////////
void CSyncedElementOngoingConfs::SetElementFileds()
{
	m_transName  = "TRANS_CONF_LIST";
	m_actionName = "GET_FULL_CONF_CHANGE_LS";
}

/////////////////////////////////////////////////////////////////////////////
void CSyncedElementOngoingConfs::SetComplexElementFileds()
{
	m_trans2TransName = "TRANS_CONF_2";
	m_trans2ActionName = "GET";

	m_addOrUpdateOpcode = FAILOVER_CONFPARTY_ADD_OR_UPDATE_CONF_IND;
	m_delOpcode		    = FAILOVER_CONFPARTY_TERMINATE_CONF_IND;
	m_eDestProcess      = eProcessConfParty;

	m_trans2ResponseObjectName = "CONFERENCE";
}

/////////////////////////////////////////////////////////////////////////////
void CSyncedElementOngoingConfs::HandleChangeIfNeeded(CXMLDOMElement *pRootElem, eChangeType changeType, int &numOfChangedConfs)
{
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
			pSpecActionNode->getChildNodeByName(&pSpecActionNode, "FULL_CHANGE_LS");
			if (pSpecActionNode)
			{
				if (eChange_AddOrUpdate == changeType)
				{
					pSpecActionNode->getChildNodeByName(&pListNode, "CHANGED_LIST");
				}
				else // eChange_Delete == changeType
				{
					pSpecActionNode->getChildNodeByName(&pListNode, "DELETED_LIST");
				}

				if (pListNode)
				{
					GET_FIRST_CHILD_NODE(pListNode, "ID", pElemNode);
					DWORD changedConfId = 0;
					char* changedConfIdStr;

					while(pElemNode)
					{
						// ===== 1. count num of deleted confs
						numOfChangedConfs++;

						// ===== 2. send the indication for deleting the conf
						pElemNode->get_nodeValue(&changedConfIdStr);
						changedConfId = atoi(changedConfIdStr);

						if (eChange_AddOrUpdate == changeType)
						{
							SendTrans2( changedConfId);
						}
						else // eChange_Delete == changeType
						{
							SendDelInd(changedConfId);
						}

						GET_NEXT_CHILD_NODE(pListNode, "ID", pElemNode);
					}
				} // end if pListNode
			}//end if pSpecActionNode
		} // end if pSpecActionNode
	} // end if pActionNode
}

