#include "TraceStream.h"
#include "NetServicesDB.h"
#include "InternalProcessStatuses.h"
#include "ConnToCardManager.h"
#include "ConfResources.h"
#include "HelperFuncs.h"
#include "RsrcAlloc.h"

////////////////////////////////////////////////////////////////////////////
//                        CNetServicesDB
////////////////////////////////////////////////////////////////////////////
CNetServicesDB::CNetServicesDB()
{
	m_SpanAllocationOrder = eSpanAllocation_LoadBalancing;
}

////////////////////////////////////////////////////////////////////////////
CNetServicesDB::~CNetServicesDB()
{
}

////////////////////////////////////////////////////////////////////////////
STATUS CNetServicesDB::AddDialInReservedPorts(const char* serviceName, WORD nNumOfDialIn)
{
	CNetPortsPerService* pNetPortsPerService = FindNetPortsPerServiceByName(serviceName);
	PASSERT_AND_RETURN_VALUE(!pNetPortsPerService, STATUS_RTM_SERVICE_NOT_FOUND);

	return pNetPortsPerService->AddDialInReservedPorts(nNumOfDialIn);
}

////////////////////////////////////////////////////////////////////////////
STATUS CNetServicesDB::RemoveDialInReservedPorts(const char* serviceName, WORD nNumOfDialIn)
{
	CNetPortsPerService* pNetPortsPerService = FindNetPortsPerServiceByName(serviceName);
	PASSERT_AND_RETURN_VALUE(!pNetPortsPerService, STATUS_RTM_SERVICE_NOT_FOUND);

	return pNetPortsPerService->RemoveDialInReservedPorts(nNumOfDialIn);
}

////////////////////////////////////////////////////////////////////////////
STATUS CNetServicesDB::SpanConfiguredOnService(CSpanRTM* pSpan)
{
	CNetPortsPerService* pNetPortsPerService = FindNetPortsPerServiceByName(pSpan->GetSpanServiceName());
	PASSERT_AND_RETURN_VALUE(!pNetPortsPerService, STATUS_RTM_SERVICE_NOT_FOUND);

	return pNetPortsPerService->SpanConfiguredOnService(pSpan);
}

////////////////////////////////////////////////////////////////////////////
STATUS CNetServicesDB::SpanRemoved(CSpanRTM* pSpan)
{
	CNetPortsPerService* pNetPortsPerService = FindNetPortsPerServiceByName(pSpan->GetSpanServiceName());
	if (pNetPortsPerService == NULL) //this is not a real problem because maybe the span wasn't configured yet
		return STATUS_RTM_SERVICE_NOT_FOUND;

	return pNetPortsPerService->SpanRemoved(pSpan);
}

////////////////////////////////////////////////////////////////////////////
void CNetServicesDB::RTMUpdatePortRequest(UPDATE_ISDN_PORT_S* pRTMPortToUpdate, ACK_UPDATE_ISDN_PORT_S* pResult, BOOL isCalledFromUpdate /* TRUE - from update, FALSE - from boardFull */)
{
	//prepare the response
	memset(pResult, 0, sizeof(ACK_UPDATE_ISDN_PORT_S));
	pResult->monitor_conf_id  = pRTMPortToUpdate->monitor_conf_id;
	pResult->monitor_party_id = pRTMPortToUpdate->monitor_party_id;
	pResult->status           = STATUS_FAIL; //meanwhile status that there's a problem, until we get to the end of the function
	pResult->connection_id    = pRTMPortToUpdate->connection_id;
	pResult->channel_index    = pRTMPortToUpdate->channel_index;

	CConnToCardManager* pConnToCardManager = CHelperFuncs::GetConnToCardManager();
	PASSERT_AND_RETURN(!pConnToCardManager);

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN(!pSystemResources);

	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	PASSERT_AND_RETURN(!pConfRsrcDB);

	CConfRsrc* pConf = (CConfRsrc*)(pConfRsrcDB->GetConfRsrc(pRTMPortToUpdate->monitor_conf_id));
	PASSERT_AND_RETURN(!pConf);

	CPartyRsrc* pParty = (CPartyRsrc*)(pConf->GetParty(pRTMPortToUpdate->monitor_party_id));
	PASSERT_AND_RETURN(!pParty);

	CBoard* pBoard = pSystemResources->GetBoard(pRTMPortToUpdate->board_id);
	PASSERT_AND_RETURN(!pBoard);

	CSpanRTM* pNewSpan = (CSpanRTM*)pBoard->GetRTM(pRTMPortToUpdate->span_id);
	PASSERT_AND_RETURN(!pNewSpan);

	ConnToCardTableEntry entryInSharedMemory;
	STATUS status = STATUS_OK;

	if (pRTMPortToUpdate->connection_id != 0xFFFFFFFF) //dial-out
	{
		//in case of dial-out, we are updating resource descriptors, the shared memory,
		//and also changing the statuses in the spans
		CRsrcDesc* pRsrcDesc = pConf->GetDescFromConnectionId(pRTMPortToUpdate->connection_id);
		PASSERT_AND_RETURN(pRsrcDesc == NULL);
		PASSERT_AND_RETURN(!isCalledFromUpdate && (pRsrcDesc->GetBoardId() == pRTMPortToUpdate->board_id));	// In BoardFull, board must be changed
		PASSERT_AND_RETURN(isCalledFromUpdate && (pRsrcDesc->GetBoardId() != pRTMPortToUpdate->board_id)); // In Update, board mustn't be changed
		PASSERT_AND_RETURN(pRsrcDesc->GetType() != eLogical_net);
		PASSERT_AND_RETURN(pRsrcDesc->GetIsUpdated() == TRUE); //check that it hasn't been updated yet

		WORD portId = 0;
		WORD acceleratorId = 0;
		BOOL bNeedToUpdateDescriptorAndSharedMemory = FALSE;
		WORD newChannelId = 0;

		//check if the actual span has been changed
		if ((pRsrcDesc->GetBoardId() != pRTMPortToUpdate->board_id) || (pRsrcDesc->GetUnitId() != pRTMPortToUpdate->span_id))
		{
			//remove reserved port from old span, and allocate new port in the new span
			CBoard* pBoard = pSystemResources->GetBoard(pRsrcDesc->GetBoardId());
			PASSERT_AND_RETURN(pBoard==NULL);
			CSpanRTM* pOldSpan = (CSpanRTM*)pBoard->GetRTM(pRsrcDesc->GetUnitId());
			PASSERT_AND_RETURN(pOldSpan==NULL);
			status = pOldSpan->RemoveReservedPort(pRsrcDesc->GetFirstPortId());
			PASSERT_AND_RETURN(status != STATUS_OK);

			if (isCalledFromUpdate)
				status = pNewSpan->AllocatePort(pRsrcDesc->GetRsrcConfId(), pRsrcDesc->GetRsrcPartyId(), ePhysical_rtm, portId, FALSE);
			else
			{
				// Deallocate old channelId
				pSystemResources->DeAllocateChannelId(pRsrcDesc->GetBoardId(), pRsrcDesc->GetChannelId());

				// Allocate new channelId
				newChannelId = pSystemResources->AllocateChannelId(pRTMPortToUpdate->board_id);
				PASSERT_AND_RETURN(!newChannelId);

				status = pNewSpan->AllocatePort(pRsrcDesc->GetRsrcConfId(), pRsrcDesc->GetRsrcPartyId(), ePhysical_rtm, portId, TRUE);
			}

			PASSERT_AND_RETURN(status != STATUS_OK);

			bNeedToUpdateDescriptorAndSharedMemory = TRUE;
		}
		else //if the actual span hasn't changed, then just update it's status
		{
			newChannelId = pRsrcDesc->GetChannelId();
			portId = pRsrcDesc->GetFirstPortId();
			acceleratorId = pRsrcDesc->GetAcceleratorId();
			status = pNewSpan->MovePortFromReservedToOccupied(portId, acceleratorId, pConf->GetRsrcConfId(), pParty->GetRsrcPartyId());
			PASSERT_AND_RETURN(status != STATUS_OK);

			if (portId != pRsrcDesc->GetFirstPortId()) //port has been changed due to forcing
			{
				bNeedToUpdateDescriptorAndSharedMemory = TRUE;
			}
		}

		if (bNeedToUpdateDescriptorAndSharedMemory == TRUE)
		{
			//update descriptor
			pRsrcDesc->SetBoardId(pRTMPortToUpdate->board_id);
			pRsrcDesc->SetUnitId(pRTMPortToUpdate->span_id);
			pRsrcDesc->SetFirstPortId(portId);
			pRsrcDesc->SetChannelId(newChannelId);

			//update shared memory
			status = pConnToCardManager->Get(pRTMPortToUpdate->connection_id, entryInSharedMemory);
			PASSERT_AND_RETURN(status != STATUS_OK);

			entryInSharedMemory.boardId = pRTMPortToUpdate->board_id;
			entryInSharedMemory.unitId = pRTMPortToUpdate->span_id;
			entryInSharedMemory.portId = portId;
			entryInSharedMemory.acceleratorId = acceleratorId;
			entryInSharedMemory.channelId = newChannelId;

			pConnToCardManager->Update(entryInSharedMemory);
		}

		//in any case (when called from update) set rsrc descriptor as updated
		if (isCalledFromUpdate)
			pRsrcDesc->SetIsUpdated(TRUE);
	}
	else //dial-in
	{
		//in case of dial-in, we are removing 1 from the reserved ports for dial-in numbers, for that service
		//we allocate a port on the span
		//we are also adding  a resource descriptor and a new entry in shared memory,

		//find service and remove 1 reserved dial-in, also from party
		CNetPortsPerService* pNetPortsPerService = FindNetPortsPerServiceByName(pNewSpan->GetSpanServiceName());
		PASSERT_AND_RETURN(pNetPortsPerService== NULL);
		pNetPortsPerService->RemoveDialInReservedPorts(1);
		int numOfDialInReservedPortsLeft = pParty->GetDialInReservedPorts();
		PASSERT(numOfDialInReservedPortsLeft < 1); //there should be at least one left!
		pParty->SetDialInReservedPorts(numOfDialInReservedPortsLeft - 1);

		ipAddressV4If rtmIpAddr = { 0 };
		CNetServiceRsrcs* pService = (CNetServiceRsrcs*)(FindServiceByName(pNewSpan->GetSpanServiceName()));
		for (int k = 0; k < MAX_NUM_OF_BOARDS; k++)
		{
			if (pService && pService->m_ipAddressesList[k].boardId == pRTMPortToUpdate->board_id)
			{
				rtmIpAddr = pService->m_ipAddressesList[k].ipAdressRTMV4;
				//TRACEINTO << "\n artIpAddrV4 = " << CIPV4Wrapper(rtmIpAddr) <<"\n\n";
				break;
			}
		}

		WORD portId = 0;
		status = pNewSpan->AllocatePort(pConf->GetRsrcConfId(), pParty->GetRsrcPartyId(), ePhysical_rtm, portId, FALSE);

		if (status == STATUS_OK)
		{
			//allocate connection id to return to ConfParty
			pResult->connection_id = pSystemResources->AllocateConnId();
			if (pResult->connection_id == 0)
			{
				PASSERT(1);
				pNewSpan->DeAllocatePort(portId);
				pResult->status = STATUS_INSUFFICIENT_RSRC_CONNECTION_ID;
				return;
			}

			WORD channelId = pSystemResources->AllocateChannelId(pRTMPortToUpdate->board_id);
			if (0 == channelId)
			{
				pNewSpan->DeAllocatePort(portId);
				pSystemResources->DeAllocateConnId(pResult->connection_id);
				pResult->connection_id = 0;
				pResult->status = STATUS_INSUFFICIENT_CHANNEL_ID;
				return;
			}

			CRsrcDesc* pRtmDesc = new CRsrcDesc(pResult->connection_id, eLogical_net, pConf->GetRsrcConfId(), pParty->GetRsrcPartyId(), 1, pRTMPortToUpdate->board_id, 2, pRTMPortToUpdate->span_id, 0/*accelerator id*/, portId, E_NORMAL, // no meaning in RTM descriptor case
			    channelId, TRUE); //true at the end means that it's upodated
			pRtmDesc->SetIpAddresses(rtmIpAddr);
			status = pConf->AddDesc(pRtmDesc);

			if (status != STATUS_OK)
			{
				PASSERT(status);

				pSystemResources->DeAllocateChannelId(pRtmDesc->GetBoardId(), channelId);
				delete pRtmDesc;
				pNewSpan->DeAllocatePort(portId);
				pSystemResources->DeAllocateConnId(pResult->connection_id);
				pResult->connection_id = 0;
				return;
			}

			entryInSharedMemory.rsrc_conf_id = pConf->GetRsrcConfId();
			entryInSharedMemory.rsrc_party_id = pParty->GetRsrcPartyId();
			entryInSharedMemory.boxId = pRtmDesc->GetBoxId();
			entryInSharedMemory.boardId = pRtmDesc->GetBoardId();
			entryInSharedMemory.subBoardId = pRtmDesc->GetSubBoardId();
			entryInSharedMemory.unitId = pRtmDesc->GetUnitId();
			entryInSharedMemory.portId = pRtmDesc->GetFirstPortId();
			entryInSharedMemory.acceleratorId = pRtmDesc->GetAcceleratorId();
			entryInSharedMemory.ipAdress = pRtmDesc->GetIpAddrV4().ip;
			entryInSharedMemory.channelId = channelId;
			entryInSharedMemory.m_id = pResult->connection_id; //that was just allocated
			entryInSharedMemory.rsrcType = eLogical_net;
			entryInSharedMemory.physicalRsrcType = ePhysical_rtm;

			STATUS write_stat = pConnToCardManager->Add(entryInSharedMemory);

			std::ostringstream msg;
			entryInSharedMemory.Dump(msg);
			TRACEINTO << "Status:" << write_stat << "\n" << msg.str().c_str();

			POBJDELETE(pRtmDesc);
		}
	}

	//if we got untill here, then everything is OK
	pResult->status = status;

}

////////////////////////////////////////////////////////////////////////////
const CNetServiceRsrcs* CNetServicesDB::FindServiceByName(const char* name)
{
	if (name)
	{
		NetServicesList::iterator _itr, _end = m_netServicesList.end();
		for (_itr = m_netServicesList.begin(); _itr != _end; ++_itr)
			if (!strcmp(_itr->GetName(), name))
				return &(*_itr);
	}
	return NULL;
}

////////////////////////////////////////////////////////////////////////////
STATUS CNetServicesDB::GetBestSpansListPerBoard(ISDN_SPAN_PARAMS_S& isdn_params, CSpanRTM* best_spans_per_board[MAX_NUM_SPANS_ORDER], int boardId)
{
	CNetPortsPerService* pService = FindNetPortsPerServiceByName((char*)isdn_params.serviceName);
	if (!pService) //not found, internal error
		return STATUS_FAIL;

	pService->GetBestSpansListPerBoard(best_spans_per_board, boardId);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CNetServicesDB::CountPortsPerBoard(ISDN_SPAN_PARAMS_S& isdn_params, DWORD &numFree, DWORD &numDialOutReserved, int zeroBasedBoardId)
{
	CNetPortsPerService* pService = FindNetPortsPerServiceByName((char*)isdn_params.serviceName);
	if (!pService) //not found, internal error
		return STATUS_FAIL;

	pService->CountPortsPerBoard(numFree, numDialOutReserved, zeroBasedBoardId);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
CNetPortsPerService* CNetServicesDB::FindNetPortsPerServiceByName(const char* serviceName)
{
	CNetServiceRsrcs* pNetServiceRsrcs = (CNetServiceRsrcs*)FindServiceByName(serviceName);
	if (!pNetServiceRsrcs)
		return NULL;

	return pNetServiceRsrcs->GetNetPortsPerService();
}

