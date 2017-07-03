#include <cmath>
#include <set>
#include <map>
#include <utility>
#include <memory>
#include "RsrcAlloc.h"
#include "TraceStream.h"
#include "HostCommonDefinitions.h"
#include "InternalProcessStatuses.h"
#include "ApiStatuses.h"
#include "ResourceManager.h"
#include "SysConfig.h"
#include "ObjString.h"
#include "WrappersResource.h"
#include "HelperFuncs.h"
#include "NetServicesDB.h"
#include "RsrvResources.h"
#include "Reservator.h"
#include "FixedModeResources.h"
#include "AutoModeResources.h"
#include "CardResourceConfig.h"
#include "OpcodesMcmsInternal.h"
#include "SysConfigKeys.h"
#include "PrettyTable.h"

#define CONN_ID_RTM   100
#define CONN_ID_ENC   101
#define CONN_ID_DEC   102

extern char* ResourceTypeToString(APIU32 resourceType);
extern char* RsrcCntlTypeToString(APIU32 rsrcCntlType);
extern const char* ConfMediaTypeToString(eConfMediaType confMediaType);

STATUS SendKillPortRequest(PhysicalPortDesc* pPortDesc, eResourceTypes res_type, BYTE isPortNotResponding = 0);
STATUS SendUdpKillPortRequest(PhysicalPortDesc* pPortDesc, CUdpRsrcDesc* pUdpDesc);
STATUS SendIsdnUdpKillPortRequest(PhysicalPortDesc* pPortDesc, WORD channelId, eResourceTypes res_type);

#undef TRACEINTO
#define TRACEINTO TRACESTRFUNC(eLevelInfoHigh)
////////////////////////////////////////////////////////////////////////////
//                        CRsrcAlloc
////////////////////////////////////////////////////////////////////////////
CRsrcAlloc::CRsrcAlloc()
{
}

////////////////////////////////////////////////////////////////////////////
CRsrcAlloc::~CRsrcAlloc()
{
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrcAlloc::CreateAllocConfRsrc(CSystemResources* pSystemResources, CConfRsrcDB* pConfDB, ConfMonitorID monitor_conf_id, ConfRsrcID& confId, eSessionType sessionType, eConfMediaType confMediaType)
{
	if (STANDALONE_CONF_ID != confId)
		confId = pSystemResources->AllocateRsrcConfId();
	//returned to calling routine as well

	PASSERT_AND_RETURN_VALUE(confId == 0, STATUS_FAIL);

	TRACEINTO << "ConfId:" << confId << ", ConfMediaType:" << ConfMediaTypeToString(confMediaType);
	CConfRsrc* pConfRsrc = new CConfRsrc(monitor_conf_id, sessionType, eLogical_res_none, confMediaType);
	pConfRsrc->SetRsrcConfId(confId);

	if (pConfDB->AddConfRsrc(pConfRsrc) != STATUS_OK)
	{ //should not happen, we checked before function call
		PASSERT(1);
		if (STANDALONE_CONF_ID != confId)
			pSystemResources->DeAllocateRsrcConfId(confId);
		POBJDELETE(pConfRsrc);
		return STATUS_FAIL;
	}

	POBJDELETE(pConfRsrc);
	return STATUS_OK;

}

////////////////////////////////////////////////////////////////////////////
#ifndef MS_LYNC_AVMCU_LINK
STATUS CRsrcAlloc::CreateAllocPartyRsrc(CSystemResources* pSystemResources, CConfRsrcDB* pConfDB, ConfMonitorID monitor_conf_id, PartyMonitorID monitor_party_id, PartyRsrcID& partyId, DWORD& partyCSconnId, PartyDataStruct& partyData, ETipPartyTypeAndPosition tipType, WORD* roomPartyId)
#else
STATUS CRsrcAlloc::CreateAllocPartyRsrc(CSystemResources* pSystemResources, CConfRsrcDB* pConfDB, ConfMonitorID monitor_conf_id, PartyMonitorID monitor_party_id, PartyRsrcID& partyId, DWORD& partySCconnId, PartyDataStruct& partyData, ETipPartyTypeAndPosition tipType, WORD* roomPartyId, SigAvMcuConnIdsStruct* pSigAvMcuConnIds)
#endif
{
	const CConfRsrc* pConfRsrc = pConfDB->GetConfRsrc(monitor_conf_id);
	if (!pConfRsrc)
	{ //should not happen, we checked before function call
		PASSERT(1);
		return STATUS_CONFERENCE_NOT_EXISTS;
	}

	if (((CConfRsrc*)(pConfRsrc))->GetParty(monitor_party_id) != NULL)
	{ //party already exists, error
		PASSERT(1);
		return STATUS_PARTY_ALREADY_EXISTS;
	}

	TRACEINTO << "PartyId:" << partyId;

	if (partyId == 0) //failed to allocate
	{
		PASSERT(1);
		return STATUS_INSUFFICIENT_RSRC_PARTY_ID;
	}

	WORD room_id = 0;

	//**CS connId patch
	eNetworkPartyType networkPartyType = partyData.m_networkPartyType;
	if (CHelperFuncs::IsIPParty(networkPartyType)) // equivalent to: networkPartyType == eIP_network_party_type
	{
		partySCconnId = pSystemResources->AllocateConnId();

		PASSERTSTREAM_AND_RETURN_VALUE(!partySCconnId, "PartyId:" << partyId << " - Failed to allocate CS connection ID for party", STATUS_INSUFFICIENT_RSRC_CONNECTION_ID);

#ifdef MS_LYNC_AVMCU_LINK
		if (NULL != pSigAvMcuConnIds)
		{
			pSigAvMcuConnIds->partySigOrganizerConnId = pSystemResources->AllocateConnId();
			PASSERTSTREAM_AND_RETURN_VALUE(!pSigAvMcuConnIds->partySigOrganizerConnId, "PartyId:" << partyId << " - Failed to allocate signaling Organizer connection ID for party", STATUS_INSUFFICIENT_RSRC_CONNECTION_ID);

			pSigAvMcuConnIds->partySigFocusConnId = pSystemResources->AllocateConnId();
			PASSERTSTREAM_AND_RETURN_VALUE(!pSigAvMcuConnIds->partySigFocusConnId, "PartyId:" << partyId << " - Failed to allocate signaling Focus connection ID for party", STATUS_INSUFFICIENT_RSRC_CONNECTION_ID);

			pSigAvMcuConnIds->partySigEventPackConnId = pSystemResources->AllocateConnId();
			PASSERTSTREAM_AND_RETURN_VALUE(!pSigAvMcuConnIds->partySigEventPackConnId, "PartyId:" << partyId << " - Failed to allocate signaling Event Package connection ID for party", STATUS_INSUFFICIENT_RSRC_CONNECTION_ID);
		}
#endif
	}

	if (roomPartyId) //TIP Cisco
	{
		if (eTipMasterCenter == tipType || eTipNone == tipType)
		{
			if (*roomPartyId != 0xFFFF)
			{
				TRACEINTO << "RoomPartyId:" << *roomPartyId << " - Already allocated, so don't allocate it";
			}
			else
			{
				room_id = pSystemResources->AllocateRoomId();
				if (0 == room_id)
				{
					PASSERT(monitor_party_id);
					pSystemResources->DeAllocateConnId(partySCconnId);
					return STATUS_INSUFFICIENT_ROOM_ID;
				}
				else
					*roomPartyId = room_id;
			}
		}
		else
			room_id = *roomPartyId;
	}

	CPartyRsrc party(monitor_party_id, networkPartyType, partyData.m_videoPartyType, CHelperFuncs::GetNumArtChannels(partyData), partyData.m_partyRole, partyData.m_HdVswTypeInMixAvcSvcMode);

	party.SetRsrcPartyId(partyId);
	party.SetCSconnId(partySCconnId);
#ifdef MS_LYNC_AVMCU_LINK
	if (NULL != pSigAvMcuConnIds)
	{
		party.SetSigOrganizerConnId(pSigAvMcuConnIds->partySigOrganizerConnId);
		party.SetSigFocusConnId(pSigAvMcuConnIds->partySigFocusConnId);
		party.SetSigEventPackConnId(pSigAvMcuConnIds->partySigEventPackConnId);
	}
#endif

	party.SetRoomPartyId(room_id); //TIP Cisco
	party.SetTIPPartyType(tipType);
	party.SetCountPartyAsICEinMFW(partyData.m_countPartyAsICEinMFW);
	party.SetTipNumOfScreens(partyData.m_tipNumOfScreens);

	if (((CConfRsrc*)(pConfRsrc))->AddParty(party) != STATUS_OK)
	{
		PASSERT(1);
		pSystemResources->DeAllocateConnId(partySCconnId);
#ifdef MS_LYNC_AVMCU_LINK
		if (NULL != pSigAvMcuConnIds)
		{
			pSystemResources->DeAllocateConnId(pSigAvMcuConnIds->partySigOrganizerConnId);
			pSystemResources->DeAllocateConnId(pSigAvMcuConnIds->partySigFocusConnId);
			pSystemResources->DeAllocateConnId(pSigAvMcuConnIds->partySigEventPackConnId);
		}
#endif
		return STATUS_FAIL;
	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrcAlloc::DestroyDeAllocConfRsrc(CSystemResources* pSystemResources, CConfRsrcDB* pConfDB, DWORD monitor_conf_id, DWORD deallocateStatus)
{
	const CConfRsrc* pConfRsrc = pConfDB->GetConfRsrc(monitor_conf_id);
	if (pConfRsrc == NULL) //should not happen, we checked earlier
	{
		PASSERT(1);
		return STATUS_CONFERENCE_NOT_EXISTS;
	}
	eSessionType sessionType = ((CConfRsrc*)pConfRsrc)->GetSessionType();
	if (eCOP_HD1080_session == sessionType || eCOP_HD720_50_session == sessionType)
	{
		DestroyDeAllocConfRsrcCOP(pSystemResources, (CConfRsrc*)pConfRsrc, monitor_conf_id, deallocateStatus);
	}
	else if (eVSW_28_session == sessionType || eVSW_56_session == sessionType)
	{
		DestroyDeAllocConfRsrcVSW(pSystemResources, (CConfRsrc*)pConfRsrc, monitor_conf_id);
	}

	DWORD rsrcConfId = ((CConfRsrc*)(pConfRsrc))->GetRsrcConfId();
	if (STANDALONE_CONF_ID != rsrcConfId)
		pSystemResources->DeAllocateRsrcConfId(rsrcConfId);

	PASSERT(pConfDB->RemoveConfRsrc(monitor_conf_id));

	if (0 == pConfDB->GetNumConfRsrcs())
		pSystemResources->FreeAllOccupiedPorts();

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrcAlloc::DestroyDeAllocPartyRsrc(CSystemResources* pSystemResources, CConfRsrcDB* pConfDB, DWORD monitor_conf_id, DWORD monitor_party_id)
{
	CConfRsrc* pConfRsrc = (CConfRsrc*)pConfDB->GetConfRsrc(monitor_conf_id);
	if (!pConfRsrc)  // should not happen, we checked before function call
	{
		PASSERT(1);
		return STATUS_CONFERENCE_NOT_EXISTS;
	}

	const CPartyRsrc* pPartyRsrc = ((CConfRsrc*)pConfRsrc)->GetParty(monitor_party_id);
	if (pPartyRsrc == NULL) // party doesn't exist, error
	{
		PASSERT(1);
		return STATUS_PARTY_NOT_EXISTS;
	}

	PartyRsrcID partyRsrcId = ((CPartyRsrc*)pPartyRsrc)->GetRsrcPartyId();
	eSessionType sessionType = pConfRsrc->GetSessionType();
	eVideoPartyType videoPartyType = ((CPartyRsrc*)pPartyRsrc)->GetVideoPartyType();

	if ((eVSW_28_session == sessionType || eVSW_56_session == sessionType) && videoPartyType != eVideo_party_type_none)
	{
		// Remove the enc/dec from shared memory and return them to pool of the NxM conference resources
		CConnToCardManager* pConnToCardMngr = CHelperFuncs::GetConnToCardManager();
		if (pConnToCardMngr)
		{
			CRsrcDesc* pEncDesc = (CRsrcDesc*)pConfRsrc->GetDesc(partyRsrcId, eLogical_video_encoder, E_NORMAL);
			if (pEncDesc)
			{
				PASSERT(pConnToCardMngr->Remove(pEncDesc->GetConnId()));
				pEncDesc->SetIsUpdated(FALSE);
			}

			CRsrcDesc* pDecDesc = (CRsrcDesc*)pConfRsrc->GetDesc(partyRsrcId, eLogical_video_decoder, E_NORMAL);
			if (pDecDesc)
			{
				PASSERT(pConnToCardMngr->Remove(pDecDesc->GetConnId()));
				pDecDesc->SetIsUpdated(FALSE);
			}
		}
	}
	else if (videoPartyType != eCOP_party_type && !CHelperFuncs::IsVideoSwitchParty(videoPartyType))
	{
		if (pSystemResources->IsPcmMenuIdExist(partyRsrcId))
		{
			TRACEINTO << "PartyId:" << partyRsrcId << " - Pcm Menu Id allocated for the participant";
			DeAllocatePcm(pConfRsrc, partyRsrcId);
		}
	}

	// **CS connId patch
	if (CHelperFuncs::IsIPParty(((CPartyRsrc*)pPartyRsrc)->GetNetworkPartyType()))
	{
		DWORD partyCSconnId = ((CPartyRsrc*)(pPartyRsrc))->GetCSconnId();
		pSystemResources->DeAllocateConnId(partyCSconnId);

#ifdef MS_LYNC_AVMCU_LINK
		//MS Lync - ***TBD - consider add a condition: if (isAvMcuLinkMain), if CPartyRsrc will have such member
		DWORD partySigOrganizerConnId = ((CPartyRsrc*)(pPartyRsrc))->GetSigOrganizerConnId();
		if (partySigOrganizerConnId)
			pSystemResources->DeAllocateConnId(partySigOrganizerConnId);
		DWORD partySigFocusConnId = ((CPartyRsrc*)(pPartyRsrc))->GetSigFocusConnId();
		if (partySigFocusConnId)
			pSystemResources->DeAllocateConnId(partySigFocusConnId);
		DWORD partySigEventPackConnId = ((CPartyRsrc*)(pPartyRsrc))->GetSigEventPackConnId();
		if (partySigEventPackConnId)
			pSystemResources->DeAllocateConnId(partySigEventPackConnId);
#endif

	}

	DWORD roomPartyId = ((CPartyRsrc*)pPartyRsrc)->GetRoomPartyId();
	ETipPartyTypeAndPosition TIPtype = pPartyRsrc->GetTIPPartyType();
	if (roomPartyId && (eTipMasterCenter == TIPtype || eTipNone == TIPtype))  // TIP Cisco
	{
		TRACEINTO << "PartyId:" << roomPartyId << " - Deallocate room party id";
		pSystemResources->DeAllocateRoomId(roomPartyId);
	}

	if (((CConfRsrc*)pConfRsrc)->RemoveParty(monitor_party_id) != STATUS_OK)
	{
		PASSERT(1);
		return STATUS_FAIL;
	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrcAlloc::UpdatePartyType(CConfRsrcDB* pConfDB, ConfMonitorID monitor_conf_id, PartyMonitorID monitor_party_id, eVideoPartyType videoPartyType)
{
	const CConfRsrc* pConfRsrc = pConfDB->GetConfRsrc(monitor_conf_id);
	PASSERT_AND_RETURN_VALUE(!pConfRsrc, STATUS_CONFERENCE_NOT_EXISTS);

	CPartyRsrc* pParty = const_cast<CPartyRsrc*>(pConfRsrc->GetParty(monitor_party_id));
	PASSERT_AND_RETURN_VALUE(!pParty, STATUS_PARTY_NOT_EXISTS);

	bool isEqual = (videoPartyType == pParty->GetVideoPartyType());
	TRACEINTO << "PartyId:" << pParty->GetRsrcPartyId() << ", VideoPartyType:" << eVideoPartyTypeNames[videoPartyType] << (isEqual ? " ==> equal to original" : "");

	pParty->SetVideoPartyType(videoPartyType);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
void CRsrcAlloc::Allocate(ALLOC_PARTY_REQ_PARAMS_S* pParam, ALLOC_PARTY_IND_PARAMS_S* pResult)
{
	// countAlloc = 0;
	// countAlloc++;
	// *** check parameters for validity (pParam)

	// *** if FIRST party in conference - create conference.
	// allocate conf rsrc id and master audio cntler.

	// ***create party. allocate rsrc party id.
	// allocate needed ports (ART, VIDEO, RTM).

	// ***write allocation results to Conn-To-Cards table

	// ***fill in returned struct (pResult)
	PASSERT_AND_RETURN(!pParam);
	PASSERT_AND_RETURN(!pResult);

	ConfMonitorID     monitor_conf_id    = pParam->monitor_conf_id;
	PartyMonitorID    monitor_party_id   = pParam->monitor_party_id;
	WORD              room_id            = pParam->room_id;
	WORD              subServiceId       = pParam->subServiceId;
	eVideoPartyType   reqVideoPartyType  = pParam->videoPartyType;
	ePartyRole        reqPartyRole       = pParam->partyRole;
	eVideoPartyType   videoPartyType     = reqVideoPartyType;
	eNetworkPartyType networkPartyType   = pParam->networkPartyType;
	eSessionType      sessionType        = pParam->sessionType;
	WORD              serviceId          = pParam->serviceId;
	DWORD             optionsMask        = pParam->optionsMask;
	EAllocationPolicy allocPolicy        = pParam->allocationPolicy;
	BYTE              portGaugeThreshold = pParam->bRmxPortGaugeThresholdExceeded;

	TRACEINTO << "PartyId:" << pParam->party_id << ", MonitorConfId:" << monitor_conf_id << ", MonitorPartyId:" << monitor_party_id << " - Enter Alloc";

	CConfRsrcDB*      pConfRsrcDB        = CHelperFuncs::GetConfRsrcDB();
	CSystemResources* pSystemResources   = CHelperFuncs::GetSystemResources();
	CReservator*      pReservator        = CHelperFuncs::GetReservator();

	pResult->allocIndBase.rsrc_party_id = pParam->party_id; // party id is now allocated at ConfParty, return rsrc_party_id to deallocate in case of failure

	pResult->allocIndBase.status = STATUS_FAIL;
	PASSERT_AND_RETURN(!pSystemResources);
	PASSERT_AND_RETURN(!pConfRsrcDB);
	PASSERT_AND_RETURN(!pReservator);
	pResult->allocIndBase.status = STATUS_OK;

	if (FALSE == pSystemResources->IsThereExistUtilizableUnitForAudioController())
	{
		TRACEINTO << "Failed, No utilizable unit for Audio Controller";
		pResult->allocIndBase.status = STATUS_NO_UTILIZABLE_UNIT_FOR_AUDIO_CONTROLLER;
		return;
	}

	BOOL isRelayParty = (CHelperFuncs::IsVideoRelayParty(videoPartyType) || (eVoice_relay_party_type == videoPartyType));
	BOOL bIsIPParty   = CHelperFuncs::IsIPParty(networkPartyType);
	BOOL bIsISDNParty = CHelperFuncs::IsISDNParty(networkPartyType);

	WORD numRsrc = 0, numConfUndefParties = 0;

	// ***UDP patch
	if (bIsISDNParty)
		serviceId = 0xFF; // sets all udp to irrelevant

	// *** for alloc. and output
	DWORD rsrcConfId = 0;
	PartyRsrcID rsrcPartyId = pParam->party_id; // party id is now allocated at ConfParty

	// *** for alloc result
	DWORD connIdAudEnc = 0, connIdAudDec = 0, connIdRTPOrMUX = 0, connIdContentRTP = 0, partySCconnId = 0;
#ifdef MS_LYNC_AVMCU_LINK
	std::auto_ptr<SigAvMcuConnIdsStruct> sigAvMcuConnIds(new SigAvMcuConnIdsStruct()); // MS Lync - AV MCU link
#endif
	const WORD total_max_video_rsrc = pSystemResources->GetTotalMaxVideoResources();
	ConnectionIDs connectionIdsVideo;

	CRsrcDesc* pAudEncDesc = NULL;
	CRsrcDesc* pAudDecDesc = NULL;
	CRsrcDesc* pRtpOrMuxDesc = NULL;
	CRsrcDesc** pVideoDescArray = NULL;
	CRsrcDesc** pRtmDescArray = NULL;
	CUdpRsrcDesc* pUdpDesc = NULL;

	STATUS status = STATUS_OK;
	WORD bestArtBoardId = 0xFFFF, bestVideoBoardId = 0xFFFF;
	ipAddressV4If artIpAddrV4;
	artIpAddrV4.ip = 0;

	eResourceAllocationTypes allocType = pSystemResources->GetResourceAllocationType();
	if (CHelperFuncs::IsLegalCardsAllocationType())
	{
		if (!CHelperFuncs::IsAudioParty(videoPartyType) && !CHelperFuncs::IsLegalVideoParty(videoPartyType))
		{
			pResult->allocIndBase.status = STATUS_FAIL;
			TRACEINTO << "VideoPartyType:" << eVideoPartyTypeNames[videoPartyType] << ", ResourceAllocationType:" << allocType << " - Failed, VideoPartyType is illegal for AllocType";
			return;
		}
	}
	else
	{
		pResult->allocIndBase.status = STATUS_FAIL;
		TRACEINTO << "Failed, UNKNOWN mode";
		return;
	}

	WORD useServiceId = (pSystemResources->GetMultipleIpServices()) ? serviceId : ID_ALL_IP_SERVICES;
	pResult->allocIndBase.isIceParty = pParam->isIceParty;

	BOOL bAddAudioAsVideo = FALSE;
	BOOL countPartyAsICEinMFW = FALSE;
	eProductType prodType = pSystemResources->GetProductType();
	if (eProductTypeSoftMCUMfw == prodType) // VSW & SVC ICE bypass will be limited to 100 participants
	{
		if (pParam->isIceParty || (eMixAvcSvcVsw == pParam->confMediaType))
			countPartyAsICEinMFW = TRUE;
	}
	ResourcesInterface* pResourcesInterface = pSystemResources->GetCurrentResourcesInterface();
	eConfModeTypes confModeType = eNonMix;
	if (pConfRsrcDB->IsExitingConf(monitor_conf_id) == FALSE)   // EQ
	{
		eConfMediaType reqConfMediaType = pParam->confMediaType;
		if (eMixAvcSvc == reqConfMediaType)
		{
			confModeType = eMix;
		}
	}
	else
	{
		confModeType = CHelperFuncs::GetConferenceMode(monitor_conf_id);
	}
	if (CHelperFuncs::IsMode2C() && eVSW_Auto_session == sessionType)
	{
		if (pResourcesInterface && pResourcesInterface->CheckIfOneMorePartyCanBeAddedCOP(pResult) == FALSE)
		{
			pResult->allocIndBase.status = STATUS_FAIL;
			return;
		}
	}
	else if (pSystemResources->CheckIfOneMorePartyCanBeAdded(videoPartyType, reqPartyRole, allocPolicy, pResult, useServiceId, portGaugeThreshold, &bAddAudioAsVideo, confModeType, countPartyAsICEinMFW) == FALSE)
	{
		// fill of result and trace done in function
		return;
	}
	else if (reqVideoPartyType != videoPartyType)
	{
		/*Begin:modified by Richer for BRIDGE-12845, 2014.04.22*/
		if ((videoPartyType < eCP_H264_upto_HD720_30FS_Symmetric_video_party_type) && (eTipNone != pParam->tipPartyType))   //for Tip,if not enough resources, we will not down-grade below HD720_30FS
		{
			videoPartyType = reqVideoPartyType;
			pResult->allocIndBase.status = STATUS_NUMBER_OF_VIDEO_PARTIES_EXCEEDED;
			TRACEINTOLVLERR << "Status:STATUS_NUMBER_OF_VIDEO_PARTIES_EXCEEDED - Failed";
			return;
		}
		/*End:modified by Richer for BRIDGE-12845, 2014.04.22*/

		TRACEINTO << "ReqVideoPartyType:" << eVideoPartyTypeNames[reqVideoPartyType] << ", VideoPartyType:" << eVideoPartyTypeNames[videoPartyType];
	}

	pParam->isIceParty = pResult->allocIndBase.isIceParty; // because func CheckIfOneMorePartyCanBeAdded may change this parameter

	// For isdn party: check if there's enough free RTM channels for the party
	if (bIsISDNParty)
	{
		CNetPortsPerService* pNetPortsPerService = pSystemResources->FindNetPortsPerServiceByName((char*)(pParam->isdn_span_params.serviceName));
		if (pNetPortsPerService == NULL)
		{
			pResult->allocIndBase.status = STATUS_NET_SERVICE_NOT_FOUND;
			TRACEINTO << "ServiceName:" << pParam->isdn_span_params.serviceName << " - Failed, Service not found";
			return;
		}

		PASSERT(pParam->isdn_span_params.num_of_isdn_ports > NUM_E1_PORTS || pParam->isdn_span_params.num_of_isdn_ports < 1);

		DWORD numFree, numDialOutReserved, numDialInReserved, actualFree;
		pNetPortsPerService->CountTotalPorts(numFree, numDialOutReserved, numDialInReserved);

		actualFree = numFree - (numDialOutReserved + numDialInReserved);

		if (actualFree < pParam->isdn_span_params.num_of_isdn_ports)
		{
			TRACEINTOLVLERR << "ServiceName:" << pParam->isdn_span_params.serviceName << ", NumFree:" << numFree << ", NumDialOutReserved:" << numDialOutReserved << ", NumDialInReserved:" << numDialInReserved << ", ActualFree:" << actualFree << " - Failed, not enough RTM ports";

			if (numFree < 2)  // minimum 128k video call
			{
				pResult->allocIndBase.status = STATUS_INSUFFICIENT_RTM_RSRC;
				return;
			}

			// VNGR-23784 we'll allocate all free channels
			TRACEINTOLVLERR << "ActualFree:" << actualFree << " - Failed then will allocate only free channels";
			pParam->isdn_span_params.num_of_isdn_ports = actualFree;
			pResult->isdnParams.num_of_isdn_ports = actualFree;
		}
	}

	if (pConfRsrcDB->IsExitingConf(monitor_conf_id) == FALSE)  // while spreading it has to be done, excluded EQ
	{
		if (eSTANDALONE_session == sessionType)
		{
			rsrcConfId = STANDALONE_CONF_ID;
			eConfMediaType confMediaType = pParam->confMediaType;
			status = CreateAllocConfRsrc(pSystemResources, pConfRsrcDB, monitor_conf_id, rsrcConfId, sessionType, confMediaType);
		}
		else
		{
			pResult->allocIndBase.status = STATUS_ALLOC_REQ_WITHOUT_SPREAD_FOR_NON_EQ_CONF;
			return;
		}
	}
	else
		rsrcConfId = pConfRsrcDB->MonitorToRsrcConfId(monitor_conf_id);	// gets existing rsrcConfId from DB

	int actualNumberOfRTMPortsToAllocate = 0;
	PartyDataStruct partyData;
	memset(&partyData, 0, sizeof(partyData));
	partyData.m_videoPartyType = videoPartyType;
	partyData.m_partyRole = reqPartyRole;
	partyData.m_networkPartyType = networkPartyType;
	partyData.m_allowReconfiguration = isRelayParty ? FALSE : pParam->isWaitForRsrcAndAskAgain;
	partyData.m_monitor_conf_id = monitor_conf_id;
	partyData.m_subServiceId = subServiceId;
	partyData.m_room_id = room_id;
	partyData.m_partyTypeTIP = pParam->tipPartyType;
	partyData.m_countPartyAsICEinMFW = countPartyAsICEinMFW;
	partyData.m_tipNumOfScreens = pParam->tipNumOfScreens;
	partyData.m_artCapacity = pParam->artCapacity;
	partyData.m_confMediaType = pParam->confMediaType;
	partyData.m_HdVswTypeInMixAvcSvcMode = pParam->HdVswTypeInMixAvcSvcMode;

	BestAllocStruct bestAlloction;
	memset(&bestAlloction, 0, sizeof(bestAlloction));
	if (bIsISDNParty)
	{
		partyData.m_pIsdn_Params_Request = &pParam->isdn_span_params;
		bestAlloction.m_pIsdn_Params_Response = &pResult->isdnParams;

		actualNumberOfRTMPortsToAllocate = GetActualNumberOfRTMPortsToAllocate(pParam->isdn_span_params);
	}

	BOOL isTIP = (room_id != 0xFFFF);
	WORD reqBoardId = 0xFFFF, unitIdTIP = 0;
	CConfRsrc* pConfRsrc = pConfRsrcDB->GetConfRsrc(monitor_conf_id);
	if (pConfRsrc && isTIP && CHelperFuncs::IsTIPSlavePartyType(pParam->tipPartyType))  // TIP Cisco
	{
		status = pConfRsrc->GetBoardUnitIdByRoomIdTIP(room_id, reqBoardId, unitIdTIP);
		DBGPASSERT(status != STATUS_OK);
		TRACEINTO << "ConfId:" << rsrcConfId << ", PartyId:" << rsrcPartyId << ", RoomId:" << room_id << ", BoardId:" << reqBoardId << ", UnitId:" << unitIdTIP << ", Status:" << CProcessBase::GetProcess()->GetStatusAsString(status).c_str() << " - TIP slave allocate request";
	}

	bool hasRelayParties = pConfRsrc->CheckIfThereAreRelayParty();
	if (pConfRsrc && hasRelayParties)
	{
		reqBoardId = pConfRsrc->GetBoardIdForRelayParty();
		TRACEINTO << "ConfId:" << rsrcConfId << ", PartyId:" << rsrcPartyId << ", BoardId:" << reqBoardId << " - Not first SVC party in the conference";
	}

	if (pConfRsrc && (eAvMcuLinkSlaveOut == pParam->avMcuLinkType || eAvMcuLinkSlaveIn == pParam->avMcuLinkType))
	{
		status = pConfRsrc->GetBoardUnitIdByAvMcuLinkMain(pParam->mainPartyRsrcID, reqBoardId, unitIdTIP);
		DBGPASSERT(status != STATUS_OK);
		TRACEINTO << "ConfId:" << rsrcConfId << ", PartyId:" << rsrcPartyId << ", BoardId:" << reqBoardId << ", UnitId:" << unitIdTIP << ", Status:" << CProcessBase::GetProcess()->GetStatusAsString(status).c_str() << " - AvMcuLink slave allocate request";
	}
	// SVC AVC dynamic allocation
	BOOL upgradeAvcParties = FALSE, upgradeSvcParties = FALSE;
	BOOL needToMoveToMixMode = CheckIfConfNeedToMoveToMixMode(monitor_conf_id, videoPartyType, upgradeAvcParties, upgradeSvcParties);

	// this called after conf upgraded to eMediaStateMixAvcSvc
	if ((CHelperFuncs::GetConferenceMode(monitor_conf_id) == eMix) || needToMoveToMixMode)
		partyData.m_confModeType = eMix;
	else
		partyData.m_confModeType = eNonMix;

	TRACEINTO << "MonitorPartyId:" << monitor_party_id << ", IsRelayParty:" << (WORD)isRelayParty << ", ConfModeType:" << partyData.m_confModeType << ", IsNeedToMoveToMixMode:" << (int)needToMoveToMixMode;

	partyData.m_reqBoardId = reqBoardId;
	partyData.m_reqArtUnitId = unitIdTIP;

	if (needToMoveToMixMode)
	{
		bool enoughResources = CheckEnoughResourcesTransferConfToMixedMode(*pConfRsrc, partyData);
		if (!enoughResources)
		{
			pResult->allocIndBase.status = STATUS_FAIL;
			TRACEINTO << "Not enough resources to move conf to mix mode - DYNAMIC_ALLOCATION_AVC_SVC";
			return;
		}
	}

	if (status == STATUS_OK)   // create PartyRsrc object (in ConfRsrc ), allocate rsrcPartyId & roomId for TIP
	{
#ifndef MS_LYNC_AVMCU_LINK
		status = CreateAllocPartyRsrc(pSystemResources,
				pConfRsrcDB,
				monitor_conf_id,
				monitor_party_id,
				rsrcPartyId,
				partySCconnId,
				partyData,
				pParam->tipPartyType,
				&room_id); // TIP Cisco
#else
		// In case of an AV MCU link - in addition to all other resources, allocate connection IDs to cccp channels (MS Lync)
		if (eAvMcuLinkMain != pParam->avMcuLinkType)
		{
			sigAvMcuConnIds.reset(NULL);
		}

		status = CreateAllocPartyRsrc(pSystemResources, pConfRsrcDB, monitor_conf_id, monitor_party_id, rsrcPartyId, partySCconnId, partyData, pParam->tipPartyType, &room_id, // TIP Cisco
		    sigAvMcuConnIds.get()); // MS Lync - AV MCU link
#endif

		if (STATUS_CONFERENCE_NOT_EXISTS == status || STATUS_PARTY_ALREADY_EXISTS == status)
		{
			TRACEINTOLVLERR << "MonitorConfId:" << monitor_conf_id << ", MonitorPartyId:" << monitor_party_id << ", Status:" << CProcessBase::GetProcess()->GetStatusAsString(status) << " - Failed";
			pResult->allocIndBase.status = status;
			return;
		}
	}

	TRACEINTO << "PARTY_ALLOC:"
		<< "\n  MonitorConfId   :" << monitor_conf_id
		<< "\n  MonitorPartyId  :" << monitor_party_id
		<< "\n  ConfId          :" << rsrcConfId
		<< "\n  PartyId         :" << rsrcPartyId
		<< "\n  RoomId          :" << room_id
		<< "\n  VideoPartyType  :" << eVideoPartyTypeNames[reqVideoPartyType]
		<< "\n  PartyRole       :" << ePartyRoleNames[pParam->partyRole]
		<< "\n  TipPartyType    :" << eTIPPartyTypeNames[pParam->tipPartyType]
		<< "\n  TipNumOfScreens :" << pParam->tipNumOfScreens;

	// init msSSRC to 0 = illegal value
	pResult->msSsrcParams.m_msSsrcFirst = 0;
	pResult->msSsrcParams.m_msSsrcLast = 0;
	bool always_allocate_MsSsrc = false; // temp for testing

	if (pParam->reqMsSsrc || always_allocate_MsSsrc)
	{
		if (always_allocate_MsSsrc)
		{
			TRACEINTO << " always_allocate_MsSsrc = true temp for testing";
		}
		pSystemResources->AllocateMsSsrc(rsrcPartyId, pResult->msSsrcParams.m_msSsrcFirst, pResult->msSsrcParams.m_msSsrcLast);
	}

	if (status == STATUS_OK)
	{
		status = pSystemResources->GetBestBoards(partyData, bestAlloction, useServiceId);

		// Tsahi TBD: update CanUseReconfigurationForAllocation with MPM-Rx
		if (status != STATUS_OK && pSystemResources->CanUseReconfigurationForAllocation(partyData) == TRUE)
		{
			TRACEINTO << "Fragmentation problem, status = " << CProcessBase::GetProcess()->GetStatusAsString(status) << "\n";
		}

		// if didn't succeed, try downgrading
		if (status != STATUS_OK && partyData.m_videoPartyType != eVideo_party_type_none && allocPolicy == eAllowDowngradingToAudioOnly)
		{
			eConfModeTypes currentConfModeType = partyData.m_confModeType;
			memset(&bestAlloction, 0, sizeof(bestAlloction));
			memset(&partyData, 0, sizeof(partyData));
			videoPartyType = eVideo_party_type_none;
			partyData.m_videoPartyType = videoPartyType;
			partyData.m_networkPartyType = networkPartyType;
			//BRIDGE-12885, confModeType should be set when downgrading to audio only.
			partyData.m_confModeType = currentConfModeType;

			if (pSystemResources->CheckIfOneMorePartyCanBeAdded(videoPartyType, reqPartyRole, allocPolicy, pResult, useServiceId, FALSE, &bAddAudioAsVideo, CHelperFuncs::GetConferenceMode(monitor_conf_id), countPartyAsICEinMFW) == FALSE)
			{
				DestroyDeAllocPartyRsrc(pSystemResources, pConfRsrcDB, monitor_conf_id, monitor_party_id);
				// fill of result and trace done in function
				return;
			}

			TRACEINTO << "Trying to downgrade to audio only, because allocation failed \n";
			status = pSystemResources->GetBestBoards(partyData, bestAlloction, useServiceId);
			if (status == STATUS_OK)
				UpdatePartyType(pConfRsrcDB, monitor_conf_id, monitor_party_id, videoPartyType);
		}
	}

	bestArtBoardId = bestAlloction.m_ArtBoardId;
	bestVideoBoardId = bestAlloction.m_VideoAlloc.m_boardId;

	if (status == STATUS_OK)
	{
		if (bestAlloction.m_NumVideoUnitsToReconfigure > 0 && bestAlloction.m_NumARTUnitsToReconfigure > 0)
		{
			PASSERT(1);
			status = STATUS_WAIT_FOR_RSRC_AND_ASK_AGAIN;
		}
		else if (bestAlloction.m_NumVideoUnitsToReconfigure > 0)
		{
			CBoard* pBoard = pSystemResources->GetBoard(bestVideoBoardId);
			if (pBoard == NULL)
			{
				status = STATUS_FAIL;
			}
			else if (!CHelperFuncs::IsMpmRxOrNinja(pBoard->GetCardType()))
			{
				pBoard->ChangeUnits(bestAlloction.m_NumVideoUnitsToReconfigure, eUnitType_Video, TRUE); // only while allocation
				status = STATUS_WAIT_FOR_RSRC_AND_ASK_AGAIN;
				TRACEINTO << "\nRESOURCE_CAUSE: ChangeUnits for " << bestAlloction.m_NumVideoUnitsToReconfigure << " Video units. status = WAIT_FOR_RSRC_AND_ASK_AGAIN." << "\n";
			}
			else // MPM-Rx
			{
				PASSERT(bestAlloction.m_NumVideoUnitsToReconfigure);
				TRACEINTO << "Tsahi CHECK WHY WE GOT HERE! bestAlloction.m_NumVideoUnitsToReconfigure=" << bestAlloction.m_NumVideoUnitsToReconfigure << "\n";
			}
		}
		else if (bestAlloction.m_NumARTUnitsToReconfigure > 0)
		{
			PASSERT(bestAlloction.m_NumARTUnitsToReconfigure != 1);

			CBoard* pBoard = pSystemResources->GetBoard(bestArtBoardId);
			if (pBoard == NULL)
			{
				status = STATUS_FAIL;
			}
			else if (!CHelperFuncs::IsMpmRxOrNinja(pBoard->GetCardType()))
			{
				pBoard->ChangeUnits(bestAlloction.m_NumARTUnitsToReconfigure, eUnitType_Art, TRUE);
				status = STATUS_WAIT_FOR_RSRC_AND_ASK_AGAIN;
				TRACEINTO << "\nRESOURCE_CAUSE ChangeUnits for " << bestAlloction.m_NumARTUnitsToReconfigure << " ART units. status = WAIT_FOR_RSRC_AND_ASK_AGAIN." << "\n";
			}
			else // MPM-Rx
			{
				PASSERT(bestAlloction.m_NumARTUnitsToReconfigure);
				TRACEINTO << "Tsahi CHECK WHY WE GOT HERE! bestAlloction.m_NumARTUnitsToReconfigure=" << bestAlloction.m_NumARTUnitsToReconfigure << "\n";
			}
		}
	}

	if ((status == STATUS_OK) && (CHelperFuncs::IsAudioParty(videoPartyType)))
	{
		status = AllocateART(monitor_conf_id /*rsrcConfId*/,
                             rsrcPartyId,
                             pParam->artCapacity,
                             serviceId,
                             subServiceId,
                             sessionType,
                             partyData,
                             pAudEncDesc,
                             pAudDecDesc,
                             pRtpOrMuxDesc,
                             pUdpDesc,
                             pParam->isIceParty,  // ICE 4 ports
                             pParam->isBFCP,
                             bestArtBoardId);

		if (STATUS_OK == status && bIsISDNParty)  // PSTN
		{
			status = AllocateISDNResources(monitor_conf_id, rsrcPartyId, pResult->isdnParams, pParam->isdn_span_params, bestArtBoardId, artIpAddrV4, pRtmDescArray, pResult);
			if (partyData.m_reqBoardId == 0xFFFF)
				partyData.m_reqBoardId = bestArtBoardId;

			if (STATUS_OK != status)
			{
				DeAllocateART(monitor_conf_id, rsrcPartyId, connIdAudEnc, connIdAudDec, connIdRTPOrMUX, connIdContentRTP);
			}
		}
	}

	if (status != STATUS_OK) // *** rollback
	{
		DestroyDeAllocPartyRsrc(pSystemResources, pConfRsrcDB, monitor_conf_id, monitor_party_id);
	}

	if (status == STATUS_OK && CHelperFuncs::IsVideoParty(videoPartyType))
	{
		videoPartyType = partyData.m_videoPartyType;
		if (!CHelperFuncs::IsNeedAllocateVideo(videoPartyType, partyData.m_confModeType, prodType))     // no need to allocate video resources for SVC only
		{
			status = STATUS_OK;
			TRACEINTO << "no need to allocate video resources for SVC only";
		}
		else
			status = AllocateVideo(monitor_conf_id, rsrcPartyId, sessionType, bestAlloction.m_VideoAlloc, partyData, pVideoDescArray);

		if (STATUS_OK == status)
		{
			if (reqVideoPartyType != videoPartyType)  // ***party_type is downgraded SD30->SD15->CIF
			{
				UpdatePartyType(pConfRsrcDB, monitor_conf_id, monitor_party_id, videoPartyType);
				// update party type already written in RA internal DB
			}

			// art
			status = AllocateART(monitor_conf_id,
			                     rsrcPartyId,
			                     pParam->artCapacity,
			                     serviceId,
			                     subServiceId,
			                     sessionType,
			                     partyData,
			                     pAudEncDesc,
			                     pAudDecDesc,
			                     pRtpOrMuxDesc,
			                     pUdpDesc,
			                     pParam->isIceParty,
			                     pParam->isBFCP,
			                     bestArtBoardId);
			if (STATUS_OK != status)
			{
				DeallocateVideo(monitor_conf_id, rsrcPartyId, connectionIdsVideo);
				DestroyDeAllocPartyRsrc(pSystemResources, pConfRsrcDB, monitor_conf_id, monitor_party_id);
			}
		}
		else // (status != STATUS_OK) //*** rollback or downgrade to audio only
		{
			DestroyDeAllocPartyRsrc(pSystemResources, pConfRsrcDB, monitor_conf_id, monitor_party_id);
		}

		if (STATUS_OK == status && bIsISDNParty)
		{
			status = AllocateISDNResources(monitor_conf_id, rsrcPartyId, pResult->isdnParams, pParam->isdn_span_params, bestArtBoardId, artIpAddrV4, pRtmDescArray, pResult);
			if (STATUS_OK != status)
			{
				connIdAudEnc = (pAudEncDesc) ? pAudEncDesc->GetConnId() : 0;
				connIdAudDec = (pAudDecDesc) ? pAudDecDesc->GetConnId() : 0;
				connIdRTPOrMUX = (pRtpOrMuxDesc) ? pRtpOrMuxDesc->GetConnId() : 0;
				DeAllocateART(monitor_conf_id, rsrcPartyId, connIdAudEnc, connIdAudDec, connIdRTPOrMUX, connIdContentRTP);
				DeallocateVideo(monitor_conf_id, rsrcPartyId, connectionIdsVideo);
				DestroyDeAllocPartyRsrc(pSystemResources, pConfRsrcDB, monitor_conf_id, monitor_party_id);
			}
		}
	}

	pResult->allocIndBase.status = status;

	if (status != STATUS_OK) // special treatment for STATUS_NO_VIDEO_PORT ???
	{
		POBJDELETE(pAudEncDesc);
		POBJDELETE(pAudDecDesc);
		POBJDELETE(pRtpOrMuxDesc);

		if (pVideoDescArray != NULL)
		{
			for (int i = 0; i < total_max_video_rsrc; i++)
				PDELETE(pVideoDescArray[i]);

			delete[] pVideoDescArray;
			pVideoDescArray = NULL;
		}

		if (pRtmDescArray != NULL)
		{
			for (int i = 0; i < actualNumberOfRTMPortsToAllocate; i++)
				PDELETE(pRtmDescArray[i]);

			delete[] pRtmDescArray;
			pRtmDescArray = NULL;
		}

		POBJDELETE(pUdpDesc);
		return;
	}

	/*conn-to-card write*/
	CConnToCardManager* pConnToCardMngr = CHelperFuncs::GetConnToCardManager();
	if (!pConnToCardMngr)
	{
		PASSERT(1);
		// error handling;
	}

	ConnToCardTableEntry Entry;

	connIdAudEnc = (pAudEncDesc) ? pAudEncDesc->GetConnId() : 0;
	connIdAudDec = (pAudDecDesc) ? pAudDecDesc->GetConnId() : 0;
	connIdRTPOrMUX = (pRtpOrMuxDesc) ? pRtpOrMuxDesc->GetConnId() : 0;

	Entry.rsrc_conf_id = rsrcConfId;
	Entry.rsrc_party_id = rsrcPartyId;
	Entry.room_id = room_id;

	STATUS write_stat = STATUS_OK;

	WORD chanell_id_rtm = 0;

	/*conn-to-card write*/

	std::ostringstream msg;
	ConnToCardEntryDumpHeader(msg, "CRsrcAlloc::Allocate - Shared memory", Entry.rsrc_conf_id, Entry.rsrc_party_id, Entry.room_id);

	if (bIsISDNParty)
	{
		numRsrc += WriteChannelsToSharedMemoryRTM(pRtmDescArray, actualNumberOfRTMPortsToAllocate, pConnToCardMngr, chanell_id_rtm, pResult, Entry, msg);
	}

	// ////////////////// SVC //////////////////////////////
	if (isRelayParty)
	{
		numRsrc += AllocateRelayPartyResources(pParam, pRtpOrMuxDesc, Entry, msg, pResult);
	}
	else if (partyData.m_confModeType == eMix)
	{
		numRsrc += AllocateAvcRelayResources(pParam, partyData, pAudDecDesc, Entry, msg, pResult);
	}

	if (pResult->allocIndBase.status != STATUS_OK)
		DBGPASSERT(rsrcPartyId);

	// ////////////////// SVC //////////////////////////////
	int numberOfEntries = numRsrc;

	if (connIdAudEnc)
	{
		eLogicalResourceTypes lrt = isRelayParty ? eLogical_relay_audio_encoder : eLogical_audio_encoder;
		if (pConnToCardMngr)
		{
			Entry.m_id = connIdAudEnc;
			Entry.rsrcType = lrt;
			Entry.physicalRsrcType = ePhysical_art;  // relayAudioARTphysType;
			Entry.boxId = pAudEncDesc->GetBoxId();
			Entry.boardId = pAudEncDesc->GetBoardId();
			Entry.subBoardId = pAudEncDesc->GetSubBoardId();
			Entry.unitId = pAudEncDesc->GetUnitId();
			Entry.acceleratorId = pAudEncDesc->GetAcceleratorId();
			Entry.portId = pAudEncDesc->GetFirstPortId();

			if (CHelperFuncs::IsVideoISDNParty(networkPartyType, videoPartyType))
			{
				Entry.ipAdress = artIpAddrV4.ip;
				Entry.channelId = DUMMY_UDP_PORT_ID;		// This dummy value indicates that the "real" value should be taken form the corresponding rtm channel entry
			}
			else if (CHelperFuncs::IsPSTNParty(networkPartyType, videoPartyType))
			{
				Entry.ipAdress = artIpAddrV4.ip;
				Entry.channelId = chanell_id_rtm;
			}

			write_stat = AddEntryToSharedMemory(Entry); // pConnToCardMngr->Add(Entry);
			numberOfEntries += ConnToCardEntryDump(msg, Entry);
		}

		if (write_stat == STATUS_OK)                // succeeded to write in conn-to-card table
		{
			pResult->allocIndBase.allocatedRrcs[numRsrc].connectionId = connIdAudEnc;
			pResult->allocIndBase.allocatedRrcs[numRsrc].logicalRsrcType = lrt;
			numRsrc++;
		}
	}

	if (connIdAudDec)
	{
		eLogicalResourceTypes lrt = isRelayParty ? eLogical_relay_audio_decoder : eLogical_audio_decoder;
		if (pConnToCardMngr)
		{
			Entry.m_id = connIdAudDec;
			Entry.rsrcType = lrt;
			Entry.physicalRsrcType = ePhysical_art;  // relayAudioARTphysType;
			Entry.boxId = pAudDecDesc->GetBoxId();
			Entry.boardId = pAudDecDesc->GetBoardId();
			Entry.subBoardId = pAudDecDesc->GetSubBoardId();
			Entry.unitId = pAudDecDesc->GetUnitId();
			Entry.acceleratorId = pAudDecDesc->GetAcceleratorId();
			Entry.portId = pAudDecDesc->GetFirstPortId();
			if (CHelperFuncs::IsVideoISDNParty(networkPartyType, videoPartyType))
			{
				Entry.ipAdress = artIpAddrV4.ip;
				Entry.channelId = DUMMY_UDP_PORT_ID;    // This dummy value indicates that the "real" value should be taken form the corresponding rtm channel entry
			}
			else if (CHelperFuncs::IsPSTNParty(networkPartyType, videoPartyType))
			{
				Entry.ipAdress = artIpAddrV4.ip;
				Entry.channelId = chanell_id_rtm;
			}

			write_stat = AddEntryToSharedMemory(Entry); // pConnToCardMngr->Add(Entry);
			numberOfEntries += ConnToCardEntryDump(msg, Entry);
		}

		if (write_stat == STATUS_OK)                // succeeded to write in conn-to-card table
		{
			pResult->allocIndBase.allocatedRrcs[numRsrc].connectionId = connIdAudDec;
			pResult->allocIndBase.allocatedRrcs[numRsrc].logicalRsrcType = lrt;
			numRsrc++;
		}
	}

	if (connIdRTPOrMUX)
	{
		if (pConnToCardMngr)
		{
			Entry.m_id = connIdRTPOrMUX;
			if (bIsISDNParty)
				Entry.rsrcType = eLogical_mux;
			else
				Entry.rsrcType = eLogical_rtp;

			Entry.physicalRsrcType = ePhysical_art;
			Entry.boxId = pRtpOrMuxDesc->GetBoxId();
			Entry.boardId = pRtpOrMuxDesc->GetBoardId();
			Entry.subBoardId = pRtpOrMuxDesc->GetSubBoardId();
			Entry.unitId = pRtpOrMuxDesc->GetUnitId();
			Entry.acceleratorId = pRtpOrMuxDesc->GetAcceleratorId();
			Entry.portId = pRtpOrMuxDesc->GetFirstPortId();
			Entry.channelId = 0;

			write_stat = AddEntryToSharedMemory(Entry); // pConnToCardMngr->Add(Entry);
			numberOfEntries += ConnToCardEntryDump(msg, Entry);
		}

		if (write_stat == STATUS_OK)   // succeeded to write in conn-to-card table
		{
			pResult->allocIndBase.allocatedRrcs[numRsrc].connectionId = connIdRTPOrMUX;
			if (bIsISDNParty)
				pResult->allocIndBase.allocatedRrcs[numRsrc].logicalRsrcType = eLogical_mux;
			else
				pResult->allocIndBase.allocatedRrcs[numRsrc].logicalRsrcType = eLogical_rtp;

			numRsrc++;
		}
	}

	if (pVideoDescArray != NULL)
	{
		for (int i = 0; i < total_max_video_rsrc; i++)
		{
			if (pVideoDescArray[i] != NULL)
			{
				if (pConnToCardMngr)
				{
					Entry.m_id = pVideoDescArray[i]->GetConnId();
					Entry.rsrcType = pVideoDescArray[i]->GetType();
					Entry.physicalRsrcType = pVideoDescArray[i]->GetPhysicalType();
					Entry.rsrcCntlType = pVideoDescArray[i]->GetCntrlType();
					Entry.boxId = pVideoDescArray[i]->GetBoxId();
					Entry.boardId = pVideoDescArray[i]->GetBoardId();
					Entry.subBoardId = pVideoDescArray[i]->GetSubBoardId();
					Entry.unitId = pVideoDescArray[i]->GetUnitId();
					Entry.acceleratorId = pVideoDescArray[i]->GetAcceleratorId();
					Entry.portId = pVideoDescArray[i]->GetFirstPortId();
					Entry.channelId = 0;

					write_stat = AddEntryToSharedMemory(Entry); // pConnToCardMngr->Add(Entry);
					numberOfEntries += ConnToCardEntryDump(msg, Entry);

					if (write_stat == STATUS_OK)   // succeeded to write in conn-to-card table
					{
						// ConfParty requirs NOT TO ADD slaves to resources returned to ConfParty
						if (pVideoDescArray[i]->GetCntrlType() == E_NORMAL || pVideoDescArray[i]->GetCntrlType() == E_VIDEO_CONTENT_DECODER || CHelperFuncs::IsMasterType(pVideoDescArray[i]->GetCntrlType()))
						{
							pResult->allocIndBase.allocatedRrcs[numRsrc].connectionId = Entry.m_id;
							pResult->allocIndBase.allocatedRrcs[numRsrc].logicalRsrcType = pVideoDescArray[i]->GetType();
							numRsrc++;
						}
					}
				}
			}
		}
	}

	if (numberOfEntries)
		ConnToCardEntryDumpFooter(msg);

	/*conn-to-card write END*/

	pConnToCardMngr->DumpRaw("CRsrcAlloc::Allocate");

	// **CS connId add - presume allocated if 'CreateAllocatePartyRsrc' succeeded
	if (partySCconnId)
	{
		pResult->allocIndBase.allocatedRrcs[numRsrc].connectionId = partySCconnId;
		pResult->allocIndBase.allocatedRrcs[numRsrc].logicalRsrcType = eLogical_ip_signaling;
		numRsrc++;
	}

#ifdef MS_LYNC_AVMCU_LINK
	// MS Lync - AV MCU link
	if (NULL != sigAvMcuConnIds.get())
	{
		if (sigAvMcuConnIds->partySigOrganizerConnId)
		{
			pResult->allocIndBase.allocatedRrcs[numRsrc].connectionId = sigAvMcuConnIds->partySigOrganizerConnId;
			pResult->allocIndBase.allocatedRrcs[numRsrc].logicalRsrcType = eLogical_ip_sigOrganizer;
			numRsrc++;
		}

		if (sigAvMcuConnIds->partySigFocusConnId)
		{
			pResult->allocIndBase.allocatedRrcs[numRsrc].connectionId = sigAvMcuConnIds->partySigFocusConnId;
			pResult->allocIndBase.allocatedRrcs[numRsrc].logicalRsrcType = eLogical_ip_sigFocus;
			numRsrc++;
		}

		if (sigAvMcuConnIds->partySigEventPackConnId)
		{
			pResult->allocIndBase.allocatedRrcs[numRsrc].connectionId = sigAvMcuConnIds->partySigEventPackConnId;
			pResult->allocIndBase.allocatedRrcs[numRsrc].logicalRsrcType = eLogical_ip_sigEventPackage;
			numRsrc++;
		}
	}

#endif // ifdef MS_LYNC_AVMCU_LINK
	// **CS

	// ***udp simulation
	if (bIsIPParty)
	{
		if (IS_UDP_FIXED_PORT_SIM != 0)
			SimulateAllocateUDP(rsrcConfId, rsrcPartyId, videoPartyType, pResult->udpAdresses);

		else if (pUdpDesc)
			memcpy(&pResult->udpAdresses, &pUdpDesc->m_udp, sizeof(UdpAddresses));
	}

	// Tsahi Xcode - Allocate dummy connection id for content transcoding
	if (reqPartyRole == eParty_Role_regular_party && CHelperFuncs::IsVideoParty(videoPartyType))
	{
		// get "Dummy connection id" for the party's video encoder content;
		// there is no actual video encoder content resource allocated for the party, but the number needs to be unique.
		DWORD connIdDummyVideoEnc = pSystemResources->AllocateConnId();
		if (connIdDummyVideoEnc == 0)                       // rollback?
			PASSERT(1);

		CRsrcDesc* pVideoEncodDummy = new CRsrcDesc(connIdDummyVideoEnc, eLogical_video_encoder_content, rsrcConfId, rsrcPartyId);
		PASSERT(pConfRsrc->AddDesc(pVideoEncodDummy));

		if (connIdDummyVideoEnc)
		{
			pResult->allocIndBase.allocatedRrcs[numRsrc].connectionId = connIdDummyVideoEnc;
			pResult->allocIndBase.allocatedRrcs[numRsrc].logicalRsrcType = eLogical_video_encoder_content;
			numRsrc++;
		}

		POBJDELETE(pVideoEncodDummy);
	}

	// *** fill in Result for output

	pResult->allocIndBase.rsrc_conf_id = rsrcConfId;
	// pResult->allocIndBase.rsrc_party_id = rsrcPartyId; moved to start of the function - need to return in case of failure
	pResult->allocIndBase.videoPartyType = videoPartyType;
	pResult->allocIndBase.networkPartyType = networkPartyType;
	pResult->allocIndBase.numRsrcs = numRsrc;
	pResult->allocIndBase.room_id = room_id;
	pResult->allocIndBase.partyRole = reqPartyRole;
	pResult->isdnParams.num_of_isdn_ports = pParam->isdn_span_params.num_of_isdn_ports; // VNGR-23784

	bool hasNotRelayParties = pConfRsrc->CheckIfThereAreNotRelayParty();

	eConfMediaType confMediaType = eConfMediaType_dummy;
	if (eMix == partyData.m_confModeType)
	{
		confMediaType = (eProductTypeSoftMCUMfw == prodType) ? eMixAvcSvcVsw : eMixAvcSvc;
	}
	else
	{
		confMediaType = isRelayParty ? eSvcOnly : eAvcOnly;
	}

	pResult->allocIndBase.confMediaType = (eProductTypeSoftMCUMfw == prodType) ? pParam->confMediaType : confMediaType;

	// Updating current number of parties in the system
	if (STATUS_OK == pResult->allocIndBase.status)
	{
		BOOL is_ong_part = pSystemResources->IsThereAnyParty();

		BasePartyDataStruct addPartyData(videoPartyType, reqPartyRole, rsrcPartyId, bIsISDNParty, bestArtBoardId, bestVideoBoardId, partyData.m_confModeType, countPartyAsICEinMFW, serviceId, bAddAudioAsVideo);
		pSystemResources->AddParty(addPartyData);

		// LED
		if (FALSE == is_ong_part)   // if it was false, turn it on
		{
			CIPMCInterfaceApi ipmcApi;
			ipmcApi.ChangeLedState(eAmber, eTurnOn);
		}
		// LED
	}

	// SVC AVC Dynamic allocation
	if (needToMoveToMixMode)
	{
		DWORD rsrc_conf_id = pConfRsrcDB->MonitorToRsrcConfId(monitor_conf_id);
		if (0 == rsrc_conf_id)
		{
			TRACEINTO << " MonitorToRsrcConfId failed";
			DBGPASSERT(monitor_conf_id);
		}
		else
		{
			STATUS moveToMixModeStatus = UpgradePartiesToAvcSvcMix(rsrc_conf_id, upgradeAvcParties, upgradeSvcParties);
			if (STATUS_OK != moveToMixModeStatus)
			{
				TRACEINTO << " move to mix failed";
				//
			}
		}
	}

	CheckIfAllocOfAVCHDisSupportedAndUpdate(pParam, pResult);

	POBJDELETE(pAudEncDesc);
	POBJDELETE(pAudDecDesc);
	POBJDELETE(pRtpOrMuxDesc);

	if (pVideoDescArray != NULL)
	{
		for (int i = 0; i < total_max_video_rsrc; i++)
			PDELETE(pVideoDescArray[i]);

		delete[] pVideoDescArray;
		pVideoDescArray = NULL;
	}

	POBJDELETE(pUdpDesc);
	if (pRtmDescArray != NULL)
	{
		for (int i = 0; i < actualNumberOfRTMPortsToAllocate; i++)
			PDELETE(pRtmDescArray[i]);

		delete[] pRtmDescArray;
		pRtmDescArray = NULL;
	}
}

////////////////////////////////////////////////////////////////////////////
int CRsrcAlloc::WriteChannelsToSharedMemoryRTM(CRsrcDesc** pRtmDescArray,
                                               int actualNumberOfRTMPortsToAllocate,
                                               CConnToCardManager* pConnToCardMngr,
                                               WORD& chanell_id_rtm,
                                               ALLOC_PARTY_IND_PARAMS_S* pResult,
                                               ConnToCardTableEntry& Entry,
                                               std::ostringstream& msg)
{
	if (!pRtmDescArray || !pConnToCardMngr)
		return 0;

	int numRsrc = 0;

	for (int i = 0; i < actualNumberOfRTMPortsToAllocate; i++)
	{
		if (pRtmDescArray[i])
		{
			Entry.m_id = pRtmDescArray[i]->GetConnId();
			Entry.rsrcType = eLogical_net;
			Entry.physicalRsrcType = ePhysical_rtm;
			Entry.boxId = pRtmDescArray[i]->GetBoxId();
			Entry.boardId = pRtmDescArray[i]->GetBoardId();
			Entry.subBoardId = pRtmDescArray[i]->GetSubBoardId();
			Entry.unitId = pRtmDescArray[i]->GetUnitId();
			Entry.acceleratorId = pRtmDescArray[i]->GetAcceleratorId();
			Entry.portId = pRtmDescArray[i]->GetFirstPortId();
			Entry.ipAdress = pRtmDescArray[i]->GetIpAddrV4().ip;
			Entry.channelId = pRtmDescArray[i]->GetChannelId();
			chanell_id_rtm = Entry.channelId; // Save channel id for audio entries

			STATUS write_stat = AddEntryToSharedMemory(Entry); // pConnToCardMngr->Add(Entry);
			ConnToCardEntryDump(msg, Entry);

			if (write_stat == STATUS_OK)              // succeeded to write in conn-to-card table
			{
				pResult->allocIndBase.allocatedRrcs[pResult->allocIndBase.numRsrcs].connectionId = pRtmDescArray[i]->GetConnId();
				pResult->allocIndBase.allocatedRrcs[pResult->allocIndBase.numRsrcs].logicalRsrcType = eLogical_net;
				pResult->allocIndBase.numRsrcs++;
				numRsrc++;
			}
		}
	}
	return numRsrc;
}

////////////////////////////////////////////////////////////////////////////
void CRsrcAlloc::DeAllocate(DEALLOC_PARTY_REQ_PARAMS_S* pParam, DEALLOC_PARTY_IND_PARAMS_S* pResult)
{
	// *** check parameters for validity (pParam)

	// ***deallocate ports (ART, VIDEO, RTM).
	// delete party. deallocate rsrc party id.

	// *** if LAST party in conference - delete conference.
	// deallocate conf rsrc id and master audio cntler.

	// ***delete allocation results from Conn-To-Cards table

	// ***fill in returned struct (pResult) (only STATUS for now)

	PASSERT_AND_RETURN(!pParam);
	PASSERT_AND_RETURN(!pResult);

	ConfMonitorID  monitorConfId          = pParam->monitor_conf_id;
	PartyMonitorID monitorPartyId         = pParam->monitor_party_id;
	DWORD          numOfRsrcsWithProblems = pParam->numOfRsrcsWithProblems;
	bool           isForceKillAllPorts    = pParam->force_kill_all_ports;

	pResult->status = STATUS_FAIL;

	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	PASSERT_AND_RETURN(!pConfRsrcDB);

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN(!pSystemResources);

	pResult->status = STATUS_OK;

	// error treatment (NULL) (both)
	eNetworkPartyType networkPartyType;
	eVideoPartyType videoPartyType;
	ePartyRole partyRole = eParty_Role_regular_party;
	eSessionType sessionType;
	WORD artChannels;
	bool isFound = pConfRsrcDB->GetPartyType(monitorConfId, monitorPartyId, networkPartyType, videoPartyType, partyRole, artChannels, sessionType);
	if (isFound == FALSE)
	{
		pResult->status = STATUS_PARTY_NOT_EXISTS;
		TRACEINTOLVLERR << "MonitorConfId:" << monitorConfId << ", MonitorPartyId:" << monitorPartyId << " - Enter DeAlloc, Failed, party not found";
		return;
	}

	PartyRsrcID partyId = pConfRsrcDB->MonitorToRsrcPartyId(monitorConfId, monitorPartyId);
	ConfRsrcID  confId  = pConfRsrcDB->MonitorToRsrcConfId(monitorConfId);

	TRACEINTO
		<< "ConfId:"           << confId
		<< ", PartyId:"        << partyId
		<< ", MonitorConfId:"  << monitorConfId
		<< ", MonitorPartyId:" << monitorPartyId
		<< ", VideoPartyType:" << eVideoPartyTypeNames[videoPartyType]
		<< ", ArtChannels:"    << artChannels
		<< " - Enter DeAlloc";

	///////////////////////////////
	// *** output needed for conn-to-card table delete
	DWORD connIdAudEnc = 0, connIdAudDec = 0, connIdRTPOrMux = 0, connIdContentRTP = 0, connIdRelayVideoEncoder = 0;
	ConnectionIDs connectionIdsVideo;

	//WORD num_min = 0, numConfUndefParties = 0;
	//BOOL is_Undef = FALSE;
	int status = STATUS_OK, ret_status = STATUS_OK;
	BYTE is_udp = pParam->is_problem_with_UDP_ports | isForceKillAllPorts;
	///////////////////////////////

	// ms lync ssrc
	pSystemResources->DeAllocateMsSsrc(partyId);

	DWORD* connIdRTMArray = NULL;

	if (CHelperFuncs::IsISDNParty(networkPartyType))
	{
		if (pParam->serviceName[0] == 0)
		{
			// in some cases we won't get the service name
			// this usually should only happen when we are internally using the deallocate. For example in hotswap
			pConfRsrcDB->FillISDNServiceName(monitorConfId, monitorPartyId, (char*)pParam->serviceName);
		}

		connIdRTMArray = new DWORD[MAX_NUM_ALLOCATED_RSRCS_NET];
		for (int i = 0; i < MAX_NUM_ALLOCATED_RSRCS_NET; i++)
			connIdRTMArray[i] = 0;

		status = DeAllocateISDNResources(monitorConfId, partyId, (char*)pParam->serviceName, connIdRTMArray, is_udp);

		if (status != STATUS_OK)
		{
			ret_status = status;
			TRACEINTOLVLERR << "PartyId:" << partyId << ", Status:" << CProcessBase::GetProcess()->GetStatusAsString(status) << " - Failed to remove additional ISDN resources";
		}
	}

	CConfRsrc* pConf = pConfRsrcDB->GetConfRsrc(monitorConfId);
	BoardID audioBoardId = 0;
	BoardID videoBoardId = 0;
	BOOL isCountedPartyAsICEinMFW = FALSE;

	if (pConf)
	{
		if (CHelperFuncs::IsVideoRelayParty(videoPartyType))
			audioBoardId = pConf->GetBoardIdForRelayParty();

		if ((0xFFFF == audioBoardId) || (0 == audioBoardId))
		{
			const CRsrcDesc* pAudDesc = pConf->GetDesc(partyId, eLogical_audio_decoder);
			if (pAudDesc)
				audioBoardId = pAudDesc->GetBoardId();
			else
				TRACEINTO << "PartyId:" << partyId << " - The audio decoder descriptor isn't found for party";
		}

		const CRsrcDesc* pVidDesc = pConf->GetDesc(partyId, eLogical_video_decoder);
		if (pVidDesc)
		{
			videoBoardId = pVidDesc->GetBoardId();
		}
		else
		{
			eLogicalResourceTypes logicalResourceType = (partyRole == eParty_Role_content_encoder) ? eLogical_video_encoder_content : eLogical_video_encoder;
			const CRsrcDesc* pEncVidDesc = pConf->GetDesc(partyId, logicalResourceType);
			if (pEncVidDesc)
				videoBoardId = pEncVidDesc->GetBoardId();
			else
				TRACEINTO << "PartyId:" << partyId << " - The video decoder/encoder descriptor isn't found for party";
		}

		const CPartyRsrc* pPartyRsrc = pConf->GetParty(monitorPartyId);
		isCountedPartyAsICEinMFW = pPartyRsrc ? pPartyRsrc->GetCountPartyAsICEinMFW() : FALSE;
	}

	if (numOfRsrcsWithProblems == 0 && !isForceKillAllPorts)
	{
		status = DeAllocateART(monitorConfId, partyId, connIdAudEnc, connIdAudDec, connIdRTPOrMux, connIdContentRTP);
		DeAllocateAvcRelayResources(monitorConfId, partyId);
	}
	else // there is needed to send kill
	{
		BYTE isPortNotResponding = pParam->resetArtUnitOnKillPort;
		PTRACE(eLevelInfoNormal, "CRsrcAlloc::DeAllocate - isPortNotResponding is True");

		BYTE is_enc = isForceKillAllPorts;
		BYTE is_dec = isForceKillAllPorts;
		BYTE is_rtp = isForceKillAllPorts;
		BYTE is_mux = isForceKillAllPorts;
		BYTE is_mix = isForceKillAllPorts;

		if (!isForceKillAllPorts)
		{
			for (WORD i = 0; i < MAX_NUM_ALLOCATED_RSRCS; i++)
			{
				switch (pParam->rsrcsWithProblems[i].logicalRsrcType)
				{
					case eLogical_audio_encoder:                           { is_enc = TRUE; break; }
					case eLogical_audio_decoder:                           { is_dec = TRUE; break; }
					case eLogical_rtp:                                     { is_rtp = TRUE; break; }
					case eLogical_mux:                                     { is_mux = TRUE; break; }

					case eLogical_legacy_to_SAC_audio_encoder:
					case eLogical_relay_avc_to_svc_rtp:
					case eLogical_relay_avc_to_svc_rtp_with_audio_encoder: { is_mix = TRUE; break; }

					default: break;
				}
			}
		}

		// ICE 4 ports-for pass channels-for ICE kill ports
		if (pParam->rtcp_ice_channels[0])
		{
			CConfRsrc* pSrcConfRsrc = const_cast<CConfRsrc*>(pConfRsrcDB->GetConfRsrc(monitorConfId));
			if (pSrcConfRsrc == NULL)
			{
				PASSERT(1);
				pResult->status = STATUS_FAIL;
				return;
			}

			CUdpRsrcDesc* ptrUdpDesc = const_cast<CUdpRsrcDesc*>(pSrcConfRsrc->GetUdpDesc(partyId));
			if (ptrUdpDesc)
			{
				CUdpRsrcDesc* pUdpDesc = new CUdpRsrcDesc(*ptrUdpDesc);

				for (int index = 0; index < MAX_NUM_OF_ALLOCATED_PARTY_UDP_PORTS; index++)
				{
					pUdpDesc->m_rtp_channels [index] = pParam->rtp_ice_channels [index];
					pUdpDesc->m_rtcp_channels[index] = pParam->rtcp_ice_channels[index];
				}

				pSrcConfRsrc->UpdateUdpDesc(partyId, pUdpDesc);
				POBJDELETE(pUdpDesc);
			}
			else
			{
				TRACEINTOLVLERR << "PartyId:" << partyId << " - Failed, invalid UPD descriptor";
			}
		}

		status = DeAllocateART(monitorConfId, partyId, connIdAudEnc, connIdAudDec, connIdRTPOrMux, connIdContentRTP, is_enc, is_dec, is_rtp, is_mux, is_udp, isPortNotResponding);
		bool isNeedToKill = (is_enc || is_dec || is_rtp || is_mix);
		DeAllocateAvcRelayResources(monitorConfId, partyId, isNeedToKill);
	}

	if (pConf && (eCOP_party_type == videoPartyType || CHelperFuncs::IsVideoRelayParty(videoPartyType))) // Olga - deallocate dummy connID of party video encoder
	{
		eLogicalResourceTypes lrt = (eCOP_party_type == videoPartyType) ? eLogical_COP_dummy_encoder : eLogical_relay_video_encoder;
		const CRsrcDesc* pVideoEncodDummy = pConf->GetDesc(partyId, lrt);
		if (pVideoEncodDummy)
		{
			connIdRelayVideoEncoder = pVideoEncodDummy->GetConnId();

			pSystemResources->DeAllocateConnId(connIdRelayVideoEncoder);
			PASSERT(pConf->RemoveDesc(partyId, lrt));
		}
	}
	else if (pConf && CHelperFuncs::IsMode2C() && CHelperFuncs::IsVideoSwitchParty(videoPartyType) && (sessionType != eVSW_Auto_session))
	{
		const CRsrcDesc* pVideoEncVSWDummy = pConf->GetDesc(partyId, eLogical_VSW_dummy_encoder);
		if (pVideoEncVSWDummy)
		{
			WORD connIdDummyVSWVideoEnc = pVideoEncVSWDummy->GetConnId();
			pSystemResources->DeAllocateConnId(connIdDummyVSWVideoEnc);
			PASSERT(pConf->RemoveDesc(partyId, eLogical_VSW_dummy_encoder));
		}

		const CRsrcDesc* pVideoDecVSWDummy = pConf->GetDesc(partyId, eLogical_VSW_dummy_decoder);
		if (pVideoDecVSWDummy)
		{
			WORD connIdDummyVSWVideoDEc = pVideoDecVSWDummy->GetConnId();
			pSystemResources->DeAllocateConnId(connIdDummyVSWVideoDEc);
			PASSERT(pConf->RemoveDesc(partyId, eLogical_VSW_dummy_decoder));
		}
	}

	if (status != STATUS_OK)
	{
		ret_status = status;
		TRACEINTOLVLERR << "PartyId:" << partyId << ", Status:" << CProcessBase::GetProcess()->GetStatusAsString(status) << " - Failed to deallocate ART";
	}

	if ((CHelperFuncs::IsVideoSwitchParty(videoPartyType) && sessionType == eVSW_Auto_session) || (!CHelperFuncs::IsMode2C() && CHelperFuncs::IsVideoParty(videoPartyType) && (!CHelperFuncs::IsAudioContentParty(videoPartyType))))
	{
		if (numOfRsrcsWithProblems == 0 && !isForceKillAllPorts)
			status = DeallocateVideo(monitorConfId, partyId, connectionIdsVideo);
		else // there is needed to send kill
		{
			BYTE is_enc = isForceKillAllPorts, is_dec = isForceKillAllPorts;

			if (!isForceKillAllPorts)
			{
				for (WORD i = 0; i < MAX_NUM_ALLOCATED_RSRCS; i++)
				{
					switch (pParam->rsrcsWithProblems[i].logicalRsrcType)
					{
						case eLogical_video_encoder:
						case eLogical_video_encoder_content:
						{
							is_enc = TRUE;
							break;
						}

						case eLogical_video_decoder:
						{
							is_dec = TRUE;
							break;
						}

						default:
							break;
					} // switch
				}
			}
			status = DeallocateVideo(monitorConfId, partyId, connectionIdsVideo, is_enc, is_dec);
		}
	}
	else if (CHelperFuncs::IsMode2C() && ((numOfRsrcsWithProblems > 0) || isForceKillAllPorts) && CHelperFuncs::IsVideoSwitchParty(videoPartyType) && (sessionType != eVSW_Auto_session))
	{
		// NxM case: send kill to video ports, but no need to delete resources themselves since they belong to the conference

		eLogicalResourceTypes logic_t = eLogical_res_none;
		BYTE is_enc = isForceKillAllPorts, is_dec = isForceKillAllPorts;

		if (!isForceKillAllPorts)
		{
			for (WORD i = 0; i < MAX_NUM_ALLOCATED_RSRCS; i++)
			{
				logic_t = pParam->rsrcsWithProblems[i].logicalRsrcType;
				if (eLogical_video_encoder == logic_t)
					is_enc = TRUE;
				else if (eLogical_video_decoder == logic_t)
					is_dec = TRUE;
			}
		}

		if (is_enc || is_dec)
			KillVideoPortVSW(monitorConfId, partyId);
	}

	if (status != STATUS_OK)
	{
		ret_status = status;
		TRACEINTOLVLERR << "PartyId:" << partyId << ", Status:" << CProcessBase::GetProcess()->GetStatusAsString(status) << " - Failed to deallocate Video";
	}

	if (IS_UDP_FIXED_PORT_SIM != 0)
		SimulateDeAllocateUDP(confId, partyId);

	// *** deallocates partyId, deletes PartyRsrc object (in ConfRsrc )
	STATUS del_status = DestroyDeAllocPartyRsrc(pSystemResources, pConfRsrcDB, monitorConfId, monitorPartyId);

	// Updating current number of parties.
	// For min participants to reserve back if needed
	if (del_status == STATUS_OK)
	{
		BOOL isIsdnParty = CHelperFuncs::IsISDNParty(networkPartyType);
		BasePartyDataStruct partyData(videoPartyType, partyRole, partyId, isIsdnParty, audioBoardId, videoBoardId, CHelperFuncs::GetConferenceMode(monitorConfId), isCountedPartyAsICEinMFW, 0, FALSE);
		pSystemResources->RemoveParty(partyData);

		if (eProductTypeSoftMCUMfw == pSystemResources->GetProductType() && (0 == pConf->GetNumParties()))
		{
			TRACEINTO << "MFW_DEBUG:  support downgrade, so change ConfMediaState even it already in mixed";
			pConf->UpdateConfMediaStateByPartiesList();
		}
		// LED
		if (pSystemResources->IsThereAnyParty() == FALSE) // last deallocated
		{
			CIPMCInterfaceApi ipmcApi;
			ipmcApi.ChangeLedState(eAmber, eTurnOff);

			CResourceManager* pResourceManager = CHelperFuncs::GetResourceManager();
			pResourceManager->SystemResourcesEmptyRequest();
		}
		// LED
	}
	else
	{
		TRACEINTOLVLERR << "PartyId:" << partyId << ", Status:" << CProcessBase::GetProcess()->GetStatusAsString(del_status) << " - Failed to destroy party resources";
	}

	if (status != STATUS_OK)
	{
		ret_status = status;
		TRACEINTOLVLERR << "PartyId:" << partyId << ", Status:" << CProcessBase::GetProcess()->GetStatusAsString(status) << " - Failed to destroy dealloc party rsrc 1";
	}

	if (STANDALONE_CONF_ID == confId)
	{
		if (pConfRsrcDB->IsEmptyConf(monitorConfId))
			status = DestroyDeAllocConfRsrc(pSystemResources, pConfRsrcDB, monitorConfId);
	}

	if (status != STATUS_OK)
	{
		ret_status = status;
		TRACEINTOLVLERR << "PartyId:" << partyId << ", Status:" << CProcessBase::GetProcess()->GetStatusAsString(status) << " - Failed to destroy dealloc party rsrc 2";
	}

	/* delete conn-to-card entries */
	// ***
	// ***
	CConnToCardManager* pConnToCardMngr = CHelperFuncs::GetConnToCardManager();
	if (!pConnToCardMngr)
	{
		PASSERT_AND_RETURN(1);
		// error handling;
	}

	if (connIdRTMArray)
	{
		BYTE i = 0;
		DWORD* pConnIdRTMArrayTemp = connIdRTMArray;

		while (i < MAX_NUM_ALLOCATED_RSRCS_NET && *pConnIdRTMArrayTemp)
		{
			i++;
			PASSERT(pConnToCardMngr->Remove(*pConnIdRTMArrayTemp));
			pConnIdRTMArrayTemp++;
		}

		if (i > MAX_NUM_ALLOCATED_RSRCS_NET)
			TRACEINTO << "Internal Error - i = " << i << "\n";

		delete[] connIdRTMArray;
		connIdRTMArray = NULL;
	}
	else
	{
		TRACEINTOLVLERR << "PartyId:" << partyId << " - Failed, connIdRTMArray is NULL";
	}

	if (connIdAudEnc)
		PASSERT(pConnToCardMngr->Remove(connIdAudEnc));

	if (connIdAudDec)
		PASSERT(pConnToCardMngr->Remove(connIdAudDec));

	if (connIdRTPOrMux)
		PASSERT(pConnToCardMngr->Remove(connIdRTPOrMux));

	if (connIdContentRTP)
		PASSERT(pConnToCardMngr->Remove(connIdContentRTP));

	if (connIdRelayVideoEncoder && CHelperFuncs::IsVideoRelayParty(videoPartyType))
		PASSERT(pConnToCardMngr->Remove(connIdRelayVideoEncoder));

	ConnectionIDs::iterator _itr, _end = connectionIdsVideo.end();
	for (_itr = connectionIdsVideo.begin(); _itr != _end; ++_itr)
		PASSERTSTREAM(pConnToCardMngr->Remove(*_itr), "ConnId:" << *_itr);

	/* delete conn-to-card entries */

	pConnToCardMngr->DumpRaw("CRsrcAlloc::DeAllocate");

	pResult->status = ret_status;

	// end
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrcAlloc::StartConf(CONF_RSRC_REQ_PARAMS_S* pParams, CONF_RSRC_IND_PARAMS_S* pResult)
{
	STATUS status = STATUS_OK;

	eSessionType sessionType = pParams->sessionType;
	ConfMonitorID monitor_conf_id = pParams->monitor_conf_id;
	eLogicalResourceTypes encoderLogicalType = pParams->logicalTypeList[0];

	std::ostringstream msg;
	msg << "MonitorConfId:" << monitor_conf_id << ", EncoderLogicalType:" << encoderLogicalType << ", SessionType:" << eSessionTypeNames[sessionType] << ", ConfMediaType:" << ConfMediaTypeToString(pParams->confMediaType) << ", MrcMcuId:" << pParams->mrcMcuId;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERTSTREAM_AND_RETURN_VALUE(!pSystemResources, msg.str().c_str(), STATUS_FAIL);

	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	PASSERTSTREAM_AND_RETURN_VALUE(!pConfRsrcDB, msg.str().c_str(), STATUS_FAIL);

	BYTE isCopSessionType = CHelperFuncs::IsSessionTypeCOP(sessionType);

	if (((isCopSessionType && !CHelperFuncs::IsMode2C()) || (!isCopSessionType && CHelperFuncs::IsMode2C())) && (sessionType != eVSW_Auto_session))
	{
		TRACEINTOLVLERR << msg.str().c_str() << " - Failed, Cannot start conference due incompatible session type and dongle restriction type";
		status = STATUS_FAIL;
	}

	if (status == STATUS_OK)
	{
		if (!pConfRsrcDB->IsExitingConf(monitor_conf_id))
		{
			ConfRsrcID confId = pSystemResources->AllocateRsrcConfId();
			pResult->rsrc_conf_id = confId;

			if (!confId)
			{
				PASSERTSTREAM(!confId, msg.str().c_str());
				status = STATUS_INSUFFICIENT_RSRC_CONF_ID;
			}

			msg << ", ConfId:" << confId;

			if (status == STATUS_OK)
			{
				TRACEINTO << msg.str().c_str();

				CConfRsrc* pConfRsrc = new CConfRsrc(monitor_conf_id, sessionType, encoderLogicalType);
				pConfRsrc->SetRsrcConfId(confId);
				pConfRsrc->SetConfMediaType(pParams->confMediaType);
				pConfRsrc->SetMrcMcuId(pParams->mrcMcuId);

				if (pConfRsrcDB->AddConfRsrc(pConfRsrc) != STATUS_OK)
				{ //should not happen, we checked before function call
					PASSERTSTREAM(1, msg.str().c_str());
					//	pSystemResources->DeAllocateRsrcConfId(rsrcConfId);
					//	return STATUS_FAIL;
				}
				if (isCopSessionType)
				{
					status = AllocateConf2C(monitor_conf_id, pResult);

					if (STATUS_OK != status && confId != 0)
					{
						pConfRsrcDB->RemoveConfRsrc(monitor_conf_id);
						pSystemResources->DeAllocateRsrcConfId(confId);
					}
				}
				POBJDELETE(pConfRsrc);
			}
		}
		else
		{
			TRACEINTOLVLERR << msg.str().c_str() << " - Failed, Cannot start conference, due conference already exists";
			status = STATUS_CONFERENCE_ALREADY_EXISTS;
		}
	}

	pResult->status = status;

	if (status)
	{
		pResult->rsrc_conf_id = 0xFFFFFFFF; //not allocated
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrcAlloc::TerminateConf(CONF_RSRC_REQ_PARAMS_S* pParams, CONF_RSRC_IND_PARAMS_S* pResult)
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN_VALUE(!pSystemResources, STATUS_FAIL);

	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	PASSERT_AND_RETURN_VALUE(!pConfRsrcDB, STATUS_FAIL);

	STATUS status = STATUS_OK;

	CConfRsrc* pConfRsrc = pConfRsrcDB->GetConfRsrc(pParams->monitor_conf_id);
	if (!pConfRsrc)
	{
		TRACEINTOLVLERR << "ConfMonitorId:" << pParams->monitor_conf_id << " - Failed, conference doesn't exist";
		status = STATUS_CONFERENCE_NOT_EXISTS;
	}
	else
	{
		if (pConfRsrc->GetNumParties())
		{
			WORD numOfRemovedParties = pConfRsrc->RemoveAllParties(FALSE);
			PASSERTSTREAM(numOfRemovedParties > 0, "NumOfRemovedParties:" << numOfRemovedParties << " - Not all parties were deleted before TERMINATE_CONF_RSRC_REQ was received");
		}
		//now the conference should be empty
		pResult->rsrc_conf_id = pConfRsrcDB->MonitorToRsrcConfId(pParams->monitor_conf_id);

		status = DestroyDeAllocConfRsrc(pSystemResources, pConfRsrcDB, pParams->monitor_conf_id, pParams->status);
	}

	pResult->status = status;

	if (status)
		pResult->rsrc_conf_id = 0xFFFFFFFF; //not allocated

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrcAlloc::UpdateRsrcDesc(CConfRsrc* pSrcConfRsrc, CConfRsrc* pTrgConfRsrc, DWORD rsrcPartyId, eLogicalResourceTypes r_type, DWORD& connId, ECntrlType cntrl, WORD portId, WORD unitId, WORD acceleratorId, WORD boardId)
{
	STATUS status = STATUS_UPDATE_RSRC_DESC_FAIL;

	CRsrcDesc* pSrcDesc = (CRsrcDesc*)(((CConfRsrc*)pSrcConfRsrc)->GetDesc(rsrcPartyId, r_type, cntrl, portId, unitId, acceleratorId, boardId));
	if (pSrcDesc != NULL)
	{
		CRsrcDesc* pTrgDesc = new CRsrcDesc(*pSrcDesc);
		DWORD confTrgId = pTrgConfRsrc->GetRsrcConfId();
		pTrgDesc->SetRsrcConfId(confTrgId);
		connId = pSrcDesc->GetConnId();

		if (STATUS_OK == ((CConfRsrc*)pTrgConfRsrc)->AddDesc(pTrgDesc))
		{

			if (STATUS_OK == (((CConfRsrc*)pSrcConfRsrc)->RemoveDesc(rsrcPartyId, r_type, cntrl, portId, unitId, acceleratorId, boardId)))
				status = STATUS_OK;
			else
				PTRACE(eLevelInfoNormal, "CRsrcAlloc::UpdateRsrcDesc - problem 1");
		}
		else
		{
			PTRACE(eLevelInfoNormal, "CRsrcAlloc::UpdateRsrcDesc - problem 2");
			status = STATUS_UPDATE_RSRC_DESC_FAIL;
		}

		POBJDELETE(pTrgDesc);
	}
	else
	{
		PTRACE(eLevelInfoNormal, "CRsrcAlloc::UpdateRsrcDesc - pSrcDesc is NULL");
		status = STATUS_UPDATE_RSRC_DESC_FAIL;
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrcAlloc::StartPartyMove(PARTY_MOVE_RSRC_REQ_PARAMS_S* pParams, PartyRsrcID& partyId, ConfRsrcID& confId)
{
	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	PASSERTSTREAM_AND_RETURN_VALUE(!pConfRsrcDB, "", STATUS_START_RSRC_PARTY_MOVE_FAIL);

	CConfRsrc* pSourceConfRsrc = pConfRsrcDB->GetConfRsrc(pParams->source_monitor_conf_id);
	TRACECOND_AND_RETURN_VALUE(!pSourceConfRsrc, "MonitorConfId:" << pParams->source_monitor_conf_id << " - Failed, source conference not exist", STATUS_CONFERENCE_NOT_EXISTS);

	CConfRsrc* pTargetConfRsrc = pConfRsrcDB->GetConfRsrc(pParams->target_monitor_conf_id);
	TRACECOND_AND_RETURN_VALUE(!pTargetConfRsrc, "MonitorConfId:" << pParams->target_monitor_conf_id << " - Failed, target conference not exist", STATUS_CONFERENCE_NOT_EXISTS);

	confId = pConfRsrcDB->MonitorToRsrcConfId(pParams->target_monitor_conf_id);
	TRACECOND_AND_RETURN_VALUE(!confId, "MonitorConfId:" << pParams->target_monitor_conf_id << " - Failed, target conference not exist", STATUS_CONFERENCE_NOT_EXISTS);

	CPartyRsrc* pPartyRsrc = const_cast<CPartyRsrc*>(pSourceConfRsrc->GetParty(pParams->source_monitor_party_id));
	TRACECOND_AND_RETURN_VALUE(!pPartyRsrc, "MonitorPartyId:" << pParams->source_monitor_party_id << " - Failed, party not exist", STATUS_CONFERENCE_NOT_EXISTS);

	partyId = pPartyRsrc->GetRsrcPartyId();

	// event mode case
	if (CHelperFuncs::IsMode2C())
	{
		if (!pTargetConfRsrc->CheckIfOneMorePartyCanBeAddedToConf2C(pPartyRsrc->GetVideoPartyType()))
		{
			TRACESTRFUNC(eLevelError) << "Party can't be added to the COP/NxM conf,  target_monitor_conf_id = " << pParams->target_monitor_conf_id;
			return STATUS_START_RSRC_PARTY_MOVE_FAIL;
		}
		return STATUS_OK;
	}

	// Check relay party case - target and source conference must be on the same board
	if (CHelperFuncs::IsVideoRelayParty(pPartyRsrc->GetVideoPartyType()))
	{
		BoardID sourceBoardId = pSourceConfRsrc->GetBoardIdForRelayParty();
		BoardID targetBoardId = pTargetConfRsrc->GetBoardIdForRelayParty(); //may be 0 if no SVC parties in target conference

		if (sourceBoardId != targetBoardId && targetBoardId != 0)
		{
			TRACEINTOLVLERR << "SourceBoardId:" << sourceBoardId << ", TargetBoardId:" << targetBoardId << " - Relay party can't move between different boards";
			return STATUS_START_RSRC_PARTY_MOVE_FAIL;
		}
	}

	if (CHelperFuncs::IsDynamicMixedAvcSvcAllocationMode())
	{
		STATUS status = StartMixedAvcSvcMove(*pSourceConfRsrc, *pTargetConfRsrc, *pPartyRsrc);
		TRACECOND_AND_RETURN_VALUE(status != STATUS_OK, "Dynamic mixed mode, party move failed", STATUS_START_RSRC_PARTY_MOVE_FAIL);
	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrcAlloc::EndPartyMove(PARTY_MOVE_RSRC_REQ_PARAMS_S* pParams, PartyRsrcID& partyId, ConfRsrcID& confId)
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN_VALUE(!pSystemResources, STATUS_FAIL);

	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	PASSERT_AND_RETURN_VALUE(!pConfRsrcDB, STATUS_FAIL);

	CConnToCardManager* pConnToCardMngr = CHelperFuncs::GetConnToCardManager();
	PASSERT_AND_RETURN_VALUE(!pConnToCardMngr, STATUS_FAIL);

	CConfRsrc* pSrcConfRsrc = pConfRsrcDB->GetConfRsrc(pParams->source_monitor_conf_id);
	PASSERTSTREAM_AND_RETURN_VALUE(!pSrcConfRsrc, "MonitorConfId:" << pParams->source_monitor_conf_id, STATUS_CONFERENCE_NOT_EXISTS);

	CConfRsrc* pTrgConfRsrc = pConfRsrcDB->GetConfRsrc(pParams->target_monitor_conf_id);
	PASSERTSTREAM_AND_RETURN_VALUE(!pTrgConfRsrc, "MonitorConfId:" << pParams->target_monitor_conf_id, STATUS_CONFERENCE_NOT_EXISTS);

	CPartyRsrc* pSrcPartyRsrc = const_cast<CPartyRsrc*>(pSrcConfRsrc->GetParty(pParams->source_monitor_party_id));
	PASSERTSTREAM_AND_RETURN_VALUE(!pSrcPartyRsrc, "MonitorPartyId:" << pParams->source_monitor_party_id, STATUS_PARTY_NOT_EXISTS);

	CPartyRsrc* pTrgPartyRsc = new CPartyRsrc(*pSrcPartyRsrc);
	AUTO_DELETE(pTrgPartyRsc);

	pTrgPartyRsc->SetMonitorPartyId(pParams->target_monitor_party_id);

	STATUS status = pTrgConfRsrc->AddParty(*pTrgPartyRsc);
	PASSERTSTREAM_AND_RETURN_VALUE(status != STATUS_OK, "MonitorPartyId:" << pParams->target_monitor_party_id, STATUS_FAIL);

	confId  = pTrgConfRsrc->GetRsrcConfId();
	partyId = pTrgPartyRsc->GetRsrcPartyId();

	// to update all active ports according to target ids.
	// to update all descriptors(delete from source side, add to target side)
	// to update all sheared memory's entities
	// for isdn: also move temporary bonding number
	// remove(delete) from source all needed items

	CRsrcDesc* pRsrcDescArray[MAX_NUM_ALLOCATED_RSRCS];
	for (int i = 0; i < MAX_NUM_ALLOCATED_RSRCS; i++)
		pRsrcDescArray[i] = NULL;

	std::set<BoardPortUnitStruct, CompareBoardUnitPort> movedPorts;
	// go over all logical resources of this party
	for (int i = eLogical_res_none + 1; i < NUM_OF_LOGICAL_RESOURCE_TYPES; i++)
	{
		eLogicalResourceTypes lrt = (eLogicalResourceTypes)i;
		// Get all resource descriptors of lrt per a given party
		int numResourcesInArray = pSrcConfRsrc->GetDescArrayPerResourceTypeByRsrcId(partyId, lrt, (CRsrcDesc**)&pRsrcDescArray, MAX_NUM_ALLOCATED_RSRCS);

		for (int j = 0; j < numResourcesInArray; j++)
		{
			CRsrcDesc*    pRsrcDesc     = pRsrcDescArray[j];
			BoardID       boardId       = pRsrcDesc->GetBoardId();
			UnitID        unitId        = pRsrcDesc->GetUnitId();
			AcceleratorID acceleratorId = pRsrcDesc->GetAcceleratorId();
			PortID        portId        = pRsrcDesc->GetFirstPortId();
			ECntrlType    cntrlType     = pRsrcDesc->GetCntrlType();
			ConnectionID  connId        = -1;

			// update conference id in local rsrc desc db
			status = UpdateRsrcDesc(pSrcConfRsrc, pTrgConfRsrc, partyId, lrt, connId, cntrlType, portId, unitId, acceleratorId, boardId);

			BoardPortUnitStruct tmpBoardPortUnitS = { boardId, unitId, portId };
			std::set<BoardPortUnitStruct, CompareBoardUnitPort>::iterator it = movedPorts.find(tmpBoardPortUnitS);
			if (it == movedPorts.end())
			{
				// Logical_relay_video_encoder is being allocated with unit id 0, but with no active port
				if (tmpBoardPortUnitS.unitId != 0)
				{
					// update active ports db
					if (lrt == eLogical_net)
						status = pSystemResources->UpdateActiveRtmPort(boardId, unitId, portId, confId, partyId);
					else
						status = pSystemResources->UpdateActivePort(boardId, unitId, portId, confId, partyId);
					movedPorts.insert(tmpBoardPortUnitS);
				}
			}

			// update shared memory
			if (connId && eLogical_video_encoder_content != lrt)
			{
				ConnToCardTableEntry EntrySrc;
				status = pConnToCardMngr->Get(connId, EntrySrc);

				TRACECOND(status != STATUS_OK, "ConnectionId:" << connId << ", ResourceType:" << lrt);

				if (status == STATUS_OK)
				{
					EntrySrc.rsrc_conf_id = confId;
					pConnToCardMngr->Update(EntrySrc);
				}
			}
		}
	}

	movedPorts.clear();

	// move udp ports
	if (CHelperFuncs::IsIPParty(pTrgPartyRsc->GetNetworkPartyType()))
	{
		if (eTipMasterCenter == pTrgPartyRsc->GetTIPPartyType() || eTipNone == pTrgPartyRsc->GetTIPPartyType())
		{
			CUdpRsrcDesc* pUdpDesc = const_cast<CUdpRsrcDesc*>(pSrcConfRsrc->GetUdpDesc(partyId));
			if (pUdpDesc)
			{
				CUdpRsrcDesc targetUdpDesc(partyId);
				targetUdpDesc.SetServId(pUdpDesc->GetServId());
				targetUdpDesc.SetsubServId(pUdpDesc->GetsubServId());
				targetUdpDesc.SetIpAddresses(pUdpDesc->m_udp.IpV4Addr, pUdpDesc->m_udp.IpV6AddrArray);
				targetUdpDesc.SetPQMId(pUdpDesc->GetPQMId());

				targetUdpDesc.m_udp.IpType                        = pUdpDesc->m_udp.IpType;
				targetUdpDesc.m_udp.AudioChannelPort              = pUdpDesc->m_udp.AudioChannelPort;
				targetUdpDesc.m_udp.AudioChannelAdditionalPorts   = pUdpDesc->m_udp.AudioChannelAdditionalPorts;
				targetUdpDesc.m_udp.VideoChannelPort              = pUdpDesc->m_udp.VideoChannelPort;
				targetUdpDesc.m_udp.VideoChannelAdditionalPorts   = pUdpDesc->m_udp.VideoChannelAdditionalPorts;
				targetUdpDesc.m_udp.FeccChannelPort               = pUdpDesc->m_udp.FeccChannelPort;
				targetUdpDesc.m_udp.FeccChannelAdditionalPorts    = pUdpDesc->m_udp.FeccChannelAdditionalPorts;
				targetUdpDesc.m_udp.ContentChannelPort            = pUdpDesc->m_udp.ContentChannelPort;
				targetUdpDesc.m_udp.ContentChannelAdditionalPorts = pUdpDesc->m_udp.ContentChannelAdditionalPorts;
				targetUdpDesc.m_udp.BfcpChannelPort               = pUdpDesc->m_udp.BfcpChannelPort;
				targetUdpDesc.m_udp.BfcpChannelAdditionalPorts    = pUdpDesc->m_udp.BfcpChannelAdditionalPorts;

				PASSERT(pTrgConfRsrc->AddUdpDesc(&targetUdpDesc));
				PASSERT(pSrcConfRsrc->RemoveUdpDesc(partyId));
			}
			else
				PTRACE(eLevelError, "CRsrcAlloc::EndPartyMove - pUdpDesc is NULL!!!");
		}
	}

	// Tsahi - Xcode
	CRsrcDesc* pSrcVideoEncodDummy = const_cast<CRsrcDesc*>(pSrcConfRsrc->GetDesc(partyId, eLogical_video_encoder_content));
	if (pSrcVideoEncodDummy)
	{
		// delete a dummy video encoder content of this party from resources list of source conference
		ConnectionID connIdDummyVideoEnc = pSrcVideoEncodDummy->GetConnId();
		PASSERT(pSrcConfRsrc->RemoveDesc(partyId, eLogical_video_encoder_content));

		// add dummy video encoder content descriptor of this party to the target conference
		CRsrcDesc* pTrgVideoEncodDummy = new CRsrcDesc(connIdDummyVideoEnc, eLogical_video_encoder_content, confId, partyId);
		PASSERT(pTrgConfRsrc->AddDesc(pTrgVideoEncodDummy));
		POBJDELETE(pTrgVideoEncodDummy);
	}

	// remove tmp bonding phone number
	if (STATUS_OK == status && CHelperFuncs::IsISDNParty(pSrcPartyRsrc->GetNetworkPartyType()))
		status = RemoveAdditionalISDNResources((char*)pParams->serviceName, pParams->source_monitor_conf_id, partyId);

	if (STATUS_OK == status)
		status = pSrcConfRsrc->RemoveParty(pParams->source_monitor_party_id);

	PASSERTSTREAM_AND_RETURN_VALUE(STATUS_OK != status, "ConfId:" << confId << "PartyId:" << partyId << ", Status:" << status, STATUS_END_RSRC_PARTY_MOVE_FAIL);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrcAlloc::AllocateART (ConfMonitorID confMonitorId,
                                PartyRsrcID partyId,
                                DWORD partyCapacity,
                                WORD serviceId,
                                WORD subServiceId,
                                eSessionType sessionType,
                                PartyDataStruct& partyData,
                                CRsrcDesc*& pAudEncDesc,
                                CRsrcDesc*& pAudDecDesc,
                                CRsrcDesc*& pRtpOrMuxDesc,
                                CUdpRsrcDesc*& pUdpRsrcDesc,
                                BOOL isIceParty,
                                BOOL isBFCPUDP,
                                WORD reqBoardId)//ICE 4 ports
{
	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	PASSERT_AND_RETURN_VALUE(!pConfRsrcDB, STATUS_FAIL);

	CConfRsrc* pConf = pConfRsrcDB->GetConfRsrc(confMonitorId);
	PASSERT_AND_RETURN_VALUE(!pConf, STATUS_FAIL);

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN_VALUE(!pSystemResources, STATUS_FAIL);

	if (partyData.m_partyRole == eParty_Role_content_decoder || partyData.m_partyRole == eParty_Role_content_encoder)
	{
		// Do not allocate ART for content decoder or content encoder party (Content Xconding issue)
		TRACEINTO << "PartyId:" << partyId << ", PartyRole:" << ePartyRoleNames[partyData.m_partyRole] << " - Do not allocate ART for party because of party role";
		return STATUS_OK;
	}

	eNetworkPartyType networkPartyType = partyData.m_networkPartyType;
	eVideoPartyType videoPartyType = partyData.m_videoPartyType;
	ETipPartyTypeAndPosition tipPartyType = partyData.m_partyTypeTIP;

	ConfRsrcID confId = pConf->GetRsrcConfId();

	std::ostringstream msg;
	msg
		<< "CRsrcAlloc::AllocateART:"
		<< "\n  ConfId              :" << confId
		<< "\n  PartyId             :" << partyId
		<< "\n  PartyCapacity       :" << partyCapacity
		<< "\n  ServiceId           :" << serviceId
		<< "\n  SubServiceId        :" << subServiceId
		<< "\n  SessionType         :" << eSessionTypeNames[sessionType]
		<< "\n  NetworkPartyType    :" << eNetworkPartyTypeNames[networkPartyType]
		<< "\n  VideoPartyType      :" << eVideoPartyTypeNames[videoPartyType]
		<< "\n  IsIceParty          :" << (int)isIceParty
		<< "\n  IsBfcpUDP           :" << (int)isBFCPUDP
		<< "\n  ReqBoardId          :" << reqBoardId
		<< "\n  TIPpartyType        :" << eTIPPartyTypeNames[tipPartyType];

	PTRACE(eLevelInfoNormal, msg.str().c_str());

	DWORD connIdAudEnc = pSystemResources->AllocateConnId();
	PASSERT_AND_RETURN_VALUE(!connIdAudEnc, STATUS_INSUFFICIENT_RSRC_CONNECTION_ID);

	DWORD connIdAudDec = pSystemResources->AllocateConnId();
	if (connIdAudDec == 0)
	{
		pSystemResources->DeAllocateConnId(connIdAudEnc);
		PASSERT_AND_RETURN_VALUE(1, STATUS_INSUFFICIENT_RSRC_CONNECTION_ID);
	}

	DWORD connIdRTPOrMux = 0;
	if (CHelperFuncs::IsIPParty(networkPartyType) || CHelperFuncs::IsVideoISDNParty(networkPartyType, videoPartyType))
	{
		connIdRTPOrMux = pSystemResources->AllocateConnId();
		if (connIdRTPOrMux == 0)
		{
			pSystemResources->DeAllocateConnId(connIdAudEnc);
			pSystemResources->DeAllocateConnId(connIdAudDec);
			PASSERT(6);
			return STATUS_INSUFFICIENT_RSRC_CONNECTION_ID;
		}
	}

	PhysicalPortDesc portDesc;

	STATUS status = pSystemResources->AllocateART(confId, partyId, partyCapacity, ePhysical_art, serviceId, subServiceId, &portDesc, partyData, isIceParty, isBFCPUDP, reqBoardId);					//ICE 4 ports
	if (status != STATUS_OK)  // get status and roll-back if needed
	{
		pSystemResources->DeAllocateConnId(connIdAudEnc);
		pSystemResources->DeAllocateConnId(connIdAudDec);
		if (connIdRTPOrMux)
			pSystemResources->DeAllocateConnId(connIdRTPOrMux);

		PTRACE2INT(eLevelInfoNormal, "CRsrcAlloc::AllocateART failed - status - ", status);
		return status;
	}

	//*** create (and insert) descriptors
	pAudEncDesc = new CRsrcDesc(connIdAudEnc, eLogical_audio_encoder, confId, partyId, portDesc.m_boxId, portDesc.m_boardId, portDesc.m_subBoardId, portDesc.m_unitId, 0 /*accelerator_id*/, portDesc.m_portId);
	PASSERT(pConf->AddDesc(pAudEncDesc));

	pAudDecDesc = new CRsrcDesc(connIdAudDec, eLogical_audio_decoder, confId, partyId, portDesc.m_boxId, portDesc.m_boardId, portDesc.m_subBoardId, portDesc.m_unitId, 0 /*accelerator_id*/, portDesc.m_portId);
	PASSERT(pConf->AddDesc(pAudDecDesc));

	if (CHelperFuncs::IsIPParty(networkPartyType))
	{
		pRtpOrMuxDesc = new CRsrcDesc(connIdRTPOrMux, eLogical_rtp, confId, partyId, portDesc.m_boxId, portDesc.m_boardId, portDesc.m_subBoardId, portDesc.m_unitId, 0 /*accelerator_id*/, portDesc.m_portId);
		PASSERT(pConf->AddDesc(pRtpOrMuxDesc));

		//***UDP path - while ART allocation was checked that UDP ok...
		if (IS_UDP_FIXED_PORT_SIM == 0 && (eTipMasterCenter == tipPartyType || eTipNone == tipPartyType)) //TIP Cisco
		{
			pUdpRsrcDesc = new CUdpRsrcDesc(partyId);

			STATUS udp_status = AllocateUDPtoParty(serviceId, subServiceId, portDesc.m_boxId, portDesc.m_boardId, portDesc.m_subBoardId, portDesc.m_unitId, videoPartyType, pUdpRsrcDesc, isIceParty, isBFCPUDP); //ICE 4 ports
			if (udp_status != STATUS_OK) //should be ok, prev. checked while allocating ART
				PASSERT(udp_status);

			PASSERT(pConf->AddUdpDesc(pUdpRsrcDesc));
		}
	}
	else if (CHelperFuncs::IsVideoISDNParty(networkPartyType, videoPartyType))
	{
		pRtpOrMuxDesc = new CRsrcDesc(connIdRTPOrMux, eLogical_mux, confId, partyId, portDesc.m_boxId, portDesc.m_boardId, portDesc.m_subBoardId, portDesc.m_unitId, portDesc.m_acceleratorId, portDesc.m_portId);
		PASSERT(pConf->AddDesc(pRtpOrMuxDesc));
	}
	//*** conn-to-card table writting performed in calling (root allocate) function
	//*** for all descriptors at once
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrcAlloc::DeAllocateART(ConfMonitorID monitorConfId, PartyRsrcID partyId, DWORD& connIdAudEnc, DWORD& connIdAudDec, DWORD& connIdRTPOrMux, DWORD& connIdContentRTP, BYTE is_enc, BYTE is_dec, BYTE is_rtp, BYTE is_mux, BYTE is_udp, BYTE portNotResponding)
{
	BYTE dsbl = FALSE;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN_VALUE(!pSystemResources, STATUS_FAIL);

	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	PASSERT_AND_RETURN_VALUE(!pConfRsrcDB, STATUS_FAIL);

	CConfRsrc* pConf = const_cast<CConfRsrc*>(pConfRsrcDB->GetConfRsrc(monitorConfId));
	PASSERT_AND_RETURN_VALUE(!pConf, STATUS_CONFERENCE_NOT_EXISTS);

	const CPartyRsrc* pParty = pConf->GetPartyRsrcByRsrcPartyId(partyId);
	PASSERT_AND_RETURN_VALUE(!pParty, STATUS_PARTY_NOT_EXISTS);

	#define PARAMS "ConfId:" << pConf->GetRsrcConfId() << ", PartyId:" << partyId

	if (pParty->GetPartyRole() == eParty_Role_content_decoder || pParty->GetPartyRole() == eParty_Role_content_encoder)
	{
		// Do not deallocate ART for content decoder or content encoder party (Content X-coding issue)
		TRACEINTO << PARAMS << ", PartyRole:" << ePartyRoleNames[pParty->GetPartyRole()] << " - Do not deallocate ART for party because of party role";
		return STATUS_OK;
	}

	TRACEINTO << PARAMS;

	eVideoPartyType videoPartyType = pParty->GetVideoPartyType();
	eNetworkPartyType networkPartyType = pParty->GetNetworkPartyType();

	const CRsrcDesc* pEncDesc = pConf->GetDesc(partyId, eLogical_audio_encoder);
	if (pEncDesc)
	{
		connIdAudEnc = pEncDesc->GetConnId();
		pSystemResources->DeAllocateConnId(connIdAudEnc);
	}
	else
	{
		TRACEINTOLVLERR << PARAMS << " - Failed, the audio encoder wasn't found";
		return STATUS_FAIL;
	}

	const CRsrcDesc* pDecDesc = pConf->GetDesc(partyId, eLogical_audio_decoder);
	if (pDecDesc)
	{
		connIdAudDec = pDecDesc->GetConnId();
		pSystemResources->DeAllocateConnId(connIdAudDec);
	}

	//remove party from Preview list
	if (eCOP_HD1080_session == pConf->GetSessionType() || eCOP_HD720_50_session == pConf->GetSessionType())
	{
		CBoard* pBoard = pSystemResources ? pSystemResources->GetBoard(pEncDesc->GetBoardId()) : NULL;
		if (pBoard)
			pBoard->RemoveVideoPreviewOnThisBoard(monitorConfId, pParty->GetMonitorPartyId());
	}

	const CRsrcDesc* pRtpOrMuxDesc = NULL, *pContentRtp = NULL;
	if (CHelperFuncs::IsIPParty(networkPartyType))	  //partyType not PSTN - deallocate RTP connection id as well
	{
		pRtpOrMuxDesc = pConf->GetDesc(partyId, eLogical_rtp);
		if (pRtpOrMuxDesc)
		{
			connIdRTPOrMux = pRtpOrMuxDesc->GetConnId();
			pSystemResources->DeAllocateConnId(connIdRTPOrMux);

			if (IS_UDP_FIXED_PORT_SIM == 0 && (eTipMasterCenter == pParty->GetTIPPartyType() || eTipNone == pParty->GetTIPPartyType())) //TIP Cisco
			{
				CUdpRsrcDesc* pUdpDesc = const_cast<CUdpRsrcDesc*>(pConf->GetUdpDesc(partyId));
				PASSERT(!pUdpDesc);

				if (pUdpDesc)
				{
					if (is_udp == TRUE) // it means that we have to kill UDP ports
					{
						PhysicalPortDesc physicalPortDesc;
						physicalPortDesc.m_boxId         = pRtpOrMuxDesc->GetBoxId();
						physicalPortDesc.m_boardId       = pRtpOrMuxDesc->GetBoardId();
						physicalPortDesc.m_subBoardId    = pRtpOrMuxDesc->GetSubBoardId();
						physicalPortDesc.m_unitId        = pRtpOrMuxDesc->GetUnitId();
						physicalPortDesc.m_portId        = pRtpOrMuxDesc->GetFirstPortId();
						physicalPortDesc.m_acceleratorId = pRtpOrMuxDesc->GetAcceleratorId();
						STATUS kill_udp_status = SendUdpKillPortRequest(&physicalPortDesc, pUdpDesc);
						PASSERT(kill_udp_status);
					}

					DeAllocatePartyUDP(pRtpOrMuxDesc->GetBoxId(), pRtpOrMuxDesc->GetBoardId(), pRtpOrMuxDesc->GetSubBoardId(), pRtpOrMuxDesc->GetUnitId(), pUdpDesc, is_udp);
					pConf->RemoveUdpDesc(partyId);
				}
			}
		}

		pContentRtp = pConf->GetDesc(partyId, eLogical_content_rtp);
		if (pContentRtp)
		{
			connIdContentRTP = pContentRtp->GetConnId();
			pSystemResources->DeAllocateConnId(connIdContentRTP);
		}
	}
	else if (CHelperFuncs::IsVideoISDNParty(networkPartyType, videoPartyType))
	{
		pRtpOrMuxDesc = pConf->GetDesc(partyId, eLogical_mux);
		if (pRtpOrMuxDesc)
		{
			connIdRTPOrMux = pRtpOrMuxDesc->GetConnId();
			pSystemResources->DeAllocateConnId(connIdRTPOrMux);
		}
	}

	const CRsrcDesc* pDesc = NULL;

	if (pEncDesc)
		pDesc = pEncDesc;
	else if (pDecDesc)
		pDesc = pEncDesc;
	else if (pRtpOrMuxDesc)
		pDesc = pRtpOrMuxDesc;

	PASSERT_AND_RETURN_VALUE(!pDesc, STATUS_FAIL); //no descriptors found, no physical data to release, error

	eResourceTypes physType = CHelperFuncs::IsAudioParty(videoPartyType) ? ePhysical_art_light : ePhysical_art;

	PhysicalPortDesc physicalPortDesc;
	physicalPortDesc.m_boxId         = pDesc->GetBoxId();
	physicalPortDesc.m_boardId       = pDesc->GetBoardId();
	physicalPortDesc.m_subBoardId    = pDesc->GetSubBoardId();
	physicalPortDesc.m_unitId        = pDesc->GetUnitId();
	physicalPortDesc.m_portId        = pDesc->GetFirstPortId();
	physicalPortDesc.m_acceleratorId = pDesc->GetAcceleratorId();

	if (is_enc || is_dec || is_rtp || is_mux)
	{
		if (portNotResponding)
			TRACEINTOLVLERR << PARAMS << " - Send Kill Port Request while port not responding";

		STATUS kill_udp_status = SendKillPortRequest(&physicalPortDesc, ePhysical_art, portNotResponding);
		PASSERT(kill_udp_status);

		dsbl = TRUE;
	}

	STATUS status = pSystemResources->DeAllocateART(pConf, partyId, pParty->GetARTChannels(), physType, videoPartyType, &physicalPortDesc, dsbl, pParty->GetTipNumOfScreens());

	if (portNotResponding)
	{
		BOOL isEnableRecoveryOnKillPort = YES;
		CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("ENABLE_DSP_RECOVERY_ON_KILL_PORT", isEnableRecoveryOnKillPort);
		if (isEnableRecoveryOnKillPort)
			pSystemResources->SetAllFreePortsToFreeDisable(&physicalPortDesc);
	}

	//remove descriptors from Conf
	if (pEncDesc)
		PASSERT(pConf->RemoveDesc(partyId, eLogical_audio_encoder));

	if (pDecDesc)
		PASSERT(pConf->RemoveDesc(partyId, eLogical_audio_decoder));

	if (pRtpOrMuxDesc)
	{
		if (CHelperFuncs::IsVideoISDNParty(networkPartyType, videoPartyType))
			PASSERT(pConf->RemoveDesc(partyId, eLogical_mux));
		else
			PASSERT(pConf->RemoveDesc(partyId, eLogical_rtp));
	}
	if (pContentRtp)
		PASSERT(pConf->RemoveDesc(partyId, eLogical_content_rtp));

	return status;
}

////////////////////////////////////////////////////////////////////////////
int CRsrcAlloc::GetActualNumberOfRTMPortsToAllocate(ISDN_SPAN_PARAMS_S& isdn_params)
{
	if (CHelperFuncs::IsDialOutParty(&isdn_params))
		return isdn_params.num_of_isdn_ports;
	else
		return 1;
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrcAlloc::AllocateISDNResources(DWORD monitor_conf_id, DWORD rsrcPartyId, ISDN_PARTY_IND_PARAMS_S& isdn_Params_Response, ISDN_SPAN_PARAMS_S& isdn_params, WORD reqArtBoardId, ipAddressV4If& artIpAddrV4, CRsrcDesc** &pRtmDescArray, ALLOC_PARTY_IND_PARAMS_S* pResult)   //status
{

	STATUS retStatus = STATUS_OK;
	BOOL bIsDialOut = CHelperFuncs::IsDialOutParty(&isdn_params);

	if (isdn_params.num_of_isdn_ports == 0)
	{
		TRACEINTO << "\nCRsrcAlloc::AllocateISDNResources Details STATUS_FAIL :" << " monitor_conf_id = " << monitor_conf_id << " rsrcPartyId = " << rsrcPartyId << " num_of_isdn_ports = " << isdn_params.num_of_isdn_ports << "\n";

		PASSERT(1);
		return STATUS_FAIL;
	}

	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	if (pConfRsrcDB == NULL)
	{
		TRACEINTO << "\nCRsrcAlloc::AllocateISDNResources Details STATUS_FAIL 2 :" << " monitor_conf_id = " << monitor_conf_id << " rsrcPartyId = " << rsrcPartyId << " num_of_isdn_ports = " << isdn_params.num_of_isdn_ports << "\n";

		PASSERT(1);
		return STATUS_FAIL;
	}
	CConfRsrc* pConf = (CConfRsrc*)(pConfRsrcDB->GetConfRsrc(monitor_conf_id));
	if (pConf == NULL)
	{
		TRACEINTO << "\nCRsrcAlloc::AllocateISDNResources Details STATUS_CONFERENCE_NOT_EXISTS :" << " monitor_conf_id = " << monitor_conf_id << " rsrcPartyId = " << rsrcPartyId << " num_of_isdn_ports = " << isdn_params.num_of_isdn_ports << "\n";

		PASSERT(1);
		return STATUS_CONFERENCE_NOT_EXISTS;
	}

	DWORD rsrcConfId = pConf->GetRsrcConfId();

	TRACEINTO << "\nRTM_ALLOC: CRsrcAlloc::AllocateISDNResources Details:" << " monitor_conf_id = " << monitor_conf_id << " rsrcConfId = " << rsrcConfId << " rsrcPartyId = " << rsrcPartyId << " num_of_isdn_ports = " << isdn_params.num_of_isdn_ports << "\n";

	// Temporary Bonding Number Allocation
	if (isdn_params.isBondingTemporaryPhoneNumberNeeded)
	{
		if (bIsDialOut)
			TRACEINTO << "\nWrong input from Conf : no Temporary Bonding Number is needed in case of dial-out\n";
		else
		{
			Phone* tempBondingPhoneAllocated = new Phone;
			retStatus = AllocateBondingTemporaryNumber((const char*)isdn_params.serviceName, rsrcConfId, rsrcPartyId, tempBondingPhoneAllocated, isdn_params.conferencePhoneNumber);
			if (STATUS_OK != retStatus)
			{
				strcpy(pResult->isdnParams.BondingTemporaryPhoneNumber, "");	// Sending empty string back to ConfParty
				TRACEINTO << "\nFailed to allocate bonding temporary phone number for rsrcPartyId = " << rsrcPartyId << ", monitor_conf_id = " << monitor_conf_id << "\n\n";

				retStatus = STATUS_INSUFFICIENT_BONDING_TEMPORARY_PHONE_NUMBERS;
			}
			else
			{
				strncpy(pResult->isdnParams.BondingTemporaryPhoneNumber, tempBondingPhoneAllocated->phone_number, PHONE_NUMBER_DIGITS_LEN);
				TRACEINTO << "\nAllocated bonding temporary phone number : " << tempBondingPhoneAllocated->phone_number << " for rsrcPartyId = " << rsrcPartyId << ", monitor_conf_id = " << monitor_conf_id << "\n\n";
			}
			PDELETE(tempBondingPhoneAllocated);
		}
	}

	if (retStatus == STATUS_OK)
	{
		int actualNumberOfRTMPortsToAllocate = GetActualNumberOfRTMPortsToAllocate(isdn_params);

		CNetServicesDB* pNetServicesDB = CHelperFuncs::GetNetServicesDB();
		if (NULL == pNetServicesDB)
		{
			PASSERTMSG(1, "pNetServicesDB is NULL");
			return STATUS_FAIL;
		}

		if (bIsDialOut == FALSE)	//in dial-in we should only allocate one actual port and the others will be reserved
		{

			//if there is more than 1 port, then reserve them as dial-in ports
			if (isdn_params.num_of_isdn_ports > 1)
			{
				retStatus = pNetServicesDB->AddDialInReservedPorts((char*)isdn_params.serviceName, isdn_params.num_of_isdn_ports - 1);
				if (retStatus != STATUS_OK)
				{
					CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
					if (pSystemResources)
					{
						TRACEINTO << "\nRTM_ALLOC: CRsrcAlloc::AllocateISDNResources FAILED Details:" << CProcessBase::GetProcess()->GetStatusAsString(retStatus).c_str() << " monitor_conf_id = " << monitor_conf_id << " rsrcConfId = " << rsrcConfId << " rsrcPartyId = " << rsrcPartyId << " num_of_isdn_ports = " << isdn_params.num_of_isdn_ports << "\n";

						pSystemResources->DumpRtmSpans();
					}
					return retStatus;
				}

				CPartyRsrc *pParty = (CPartyRsrc *)(pConf->GetPartyRsrcByRsrcPartyId(rsrcPartyId));
				if (pParty)
					pParty->SetDialInReservedPorts(isdn_params.num_of_isdn_ports - 1);
			}
		}

		retStatus = AllocateRTMAllChannels(actualNumberOfRTMPortsToAllocate, monitor_conf_id, rsrcPartyId, isdn_Params_Response, isdn_params, pRtmDescArray, rsrcConfId, bIsDialOut, pConf);

		if (retStatus == STATUS_OK)
		{
			//get artIpAddr from reqArtBoardId
			char* serv_name = (char*)isdn_params.serviceName;
			if (serv_name)
			{
				CNetServiceRsrcs* pService = (CNetServiceRsrcs*)(pNetServicesDB->FindServiceByName(serv_name));
				int l;
				for (l = 0; l < MAX_NUM_OF_BOARDS; l++)
				{
					if (pService && pService->m_ipAddressesList[l].boardId == reqArtBoardId)
					{
						artIpAddrV4 = pService->m_ipAddressesList[l].ipAdressMediaV4;
						break;
					}
				}
				if (MAX_NUM_OF_BOARDS == l)
				{
					PTRACE(eLevelError, "CRsrcAlloc::AllocateISDNResources Can not find board ID for Media IP Addr");
					PASSERT(1);
				}
			}
		}
	}
	if (STATUS_OK != retStatus)
	{
		PTRACE2(eLevelError, "CRsrcAlloc::AllocateISDNResources faild, status = ", CProcessBase::GetProcess()->GetStatusAsString(retStatus).c_str());
		TRACEINTO << "\nRTM_ALLOC: CRsrcAlloc::AllocateISDNResources FAILED Details:" << CProcessBase::GetProcess()->GetStatusAsString(retStatus).c_str() << " monitor_conf_id = " << monitor_conf_id << " rsrcConfId = " << rsrcConfId << " rsrcPartyId = " << rsrcPartyId << " num_of_isdn_ports = " << isdn_params.num_of_isdn_ports << "\n";

		CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
		if (pSystemResources)
			pSystemResources->DumpRtmSpans();

	}
	return retStatus;
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrcAlloc::AllocateRTMAllChannels(int actualNumberOfRTMPortsToAllocate, DWORD monitor_conf_id, DWORD rsrcPartyId, ISDN_PARTY_IND_PARAMS_S& isdn_Params_Response, ISDN_SPAN_PARAMS_S& isdn_params, CRsrcDesc** &pRtmDescArray, DWORD rsrcConfId, BOOL bIsDialOut, CConfRsrc* pConf)
{
	STATUS retStatus = STATUS_OK;
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	if (pSystemResources == NULL)
	{
		PASSERT(1);
		return STATUS_FAIL;
	}

	int numOfPorts = 0;
	pRtmDescArray = new CRsrcDesc*[actualNumberOfRTMPortsToAllocate];
	for (int i = 0; i < actualNumberOfRTMPortsToAllocate; i++)
	{
		pRtmDescArray[i] = NULL;
	}
	CRsrcDesc* pRtmDesc = NULL;

	for (int portNum = 0; portNum < NUM_E1_PORTS; portNum++)
	{
		if (isdn_Params_Response.spans_order.port_spans_list[portNum].board_id == 0)
			break;

		PASSERT(portNum >= actualNumberOfRTMPortsToAllocate);

		numOfPorts++;

		retStatus = AllocateRTMOneChannel(pConf, pSystemResources, rsrcConfId, rsrcPartyId, bIsDialOut, isdn_Params_Response.spans_order.port_spans_list[portNum].board_id, isdn_Params_Response.spans_order.port_spans_list[portNum].spans_list[0], pRtmDesc);

		isdn_Params_Response.spans_order.port_spans_list[portNum].conn_id = (pRtmDesc) ? pRtmDesc->GetConnId() : 0;

		if (retStatus != STATUS_OK)	// Roll-Back of AllocateRTMOneChannel
		{
			DWORD *connIdRTMArray = new DWORD[MAX_NUM_ALLOCATED_RSRCS_NET];
			for (int i = 0; i < MAX_NUM_ALLOCATED_RSRCS_NET; i++)
				connIdRTMArray[i] = 0;
			DeAllocateISDNResources(monitor_conf_id, rsrcPartyId, (char*)isdn_params.serviceName, connIdRTMArray);

			BYTE i = 0;
			DWORD* pConnIdRTMArrayTemp = connIdRTMArray;

			CConnToCardManager* pConnToCardMngr = CHelperFuncs::GetConnToCardManager();
			if (!pConnToCardMngr)
			{
				delete[] connIdRTMArray;
				PASSERT(1);
				return retStatus;
			}

			while (i < MAX_NUM_ALLOCATED_RSRCS_NET && *pConnIdRTMArrayTemp)
			{
				i++;
				PASSERT(pConnToCardMngr->Remove(*pConnIdRTMArrayTemp));
				pConnIdRTMArrayTemp++;
			}

			delete[] connIdRTMArray;
			connIdRTMArray = NULL;
			return retStatus;
		}

		pRtmDescArray[portNum] = pRtmDesc;
	}

	PASSERT(numOfPorts != actualNumberOfRTMPortsToAllocate);

	if (numOfPorts != actualNumberOfRTMPortsToAllocate)
		TRACEINTO << "numOfPorts=" << numOfPorts << ", actualNumberOfRTMPortsToAllocate=" << actualNumberOfRTMPortsToAllocate;

	return retStatus;
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrcAlloc::AllocateRTMOneChannel(CConfRsrc* pConf, CSystemResources* pSystemResources, DWORD rsrcConfId, DWORD rsrcPartyId, BOOL bIsDialOut, WORD reqBoardId, WORD reqUnitId, CRsrcDesc*& pRtmDesc)
{
	DWORD connIdRTM = pSystemResources->AllocateConnId();
	if (connIdRTM == 0)
	{
		PASSERT(1);
		return STATUS_INSUFFICIENT_RSRC_CONNECTION_ID;
	}

	PhysicalPortDesc* pPortDesc = new PhysicalPortDesc;
	STATUS status = pSystemResources->AllocateRTMOneChannel(rsrcConfId, rsrcPartyId, bIsDialOut, reqBoardId, reqUnitId, pPortDesc);

	// *** get status and rollback if needed
	if (status != STATUS_OK)
	{
		pSystemResources->DeAllocateConnId(connIdRTM);
		POBJDELETE(pPortDesc);
		return status;
	}

	WORD channelId = pSystemResources->AllocateChannelId(pPortDesc->m_boardId);
	if (0 == channelId)   // Roll-Back channelId
	{
		PASSERT(rsrcPartyId);
		status = STATUS_INSUFFICIENT_CHANNEL_ID;

		CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();

		if (pConfRsrcDB == NULL)
		{
			POBJDELETE(pPortDesc);
			PASSERT(1);
			return STATUS_FAIL;
		}

		CConfRsrc* pConf = (CConfRsrc*)(pConfRsrcDB->GetConfRsrcByRsrcConfId(rsrcConfId));
		if (pConf == NULL)
		{
			POBJDELETE(pPortDesc);
			PASSERT(rsrcConfId);
			return STATUS_CONFERENCE_NOT_EXISTS;
		}

		status = pSystemResources->DeAllocateRTMOneChannel(pConf, rsrcPartyId, pPortDesc);
		if (STATUS_OK != status)
			TRACEINTO << "Failed during roll-back";

		pSystemResources->DeAllocateConnId(connIdRTM);
		POBJDELETE(pPortDesc);

		return status;
	} // End Roll-Back channelId

	// *** create (and insert) descriptors
	pRtmDesc = new CRsrcDesc(connIdRTM, eLogical_net, rsrcConfId, rsrcPartyId, pPortDesc->m_boxId, pPortDesc->m_boardId, pPortDesc->m_subBoardId, pPortDesc->m_unitId, pPortDesc->m_acceleratorId, pPortDesc->m_portId, E_NORMAL, // no meaning in RTM descriptor case
	    channelId);

	pRtmDesc->SetIpAddresses(pPortDesc->m_IpAddrV4);

	if (bIsDialOut == FALSE) // this means that it's the first dial-in channel
		pRtmDesc->SetIsUpdated(TRUE);

	PASSERT(pConf->AddDesc(pRtmDesc));

	POBJDELETE(pPortDesc);

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrcAlloc::AllocateVideo(DWORD monitor_conf_id, PartyRsrcID partyId, eSessionType sessionType, AllocData& videoAlloc, PartyDataStruct& partyData, CRsrcDesc** &pVideoDescArray)
{
	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	PASSERT_AND_RETURN_VALUE(!pConfRsrcDB, STATUS_FAIL);

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN_VALUE(!pSystemResources, STATUS_FAIL);

	CConfRsrc* pConf = (CConfRsrc*)(pConfRsrcDB->GetConfRsrc(monitor_conf_id));
	PASSERT_AND_RETURN_VALUE(!pConf, STATUS_CONFERENCE_NOT_EXISTS);

	ConfRsrcID confId = pConf->GetRsrcConfId();

	TRACEINTO << "ConfId:" << confId << ", PartyId:" << partyId << ", VideoPartyType:" << eVideoPartyTypeNames[partyData.m_videoPartyType] << ", NetworkPartyType:" << eNetworkPartyTypeNames[partyData.m_networkPartyType] << ", SessionType:" << eSessionTypeNames[sessionType];

	WORD max_units_video = CHelperFuncs::GetMaxUnitsNeededForVideo();
	WORD max_media_ports = CHelperFuncs::GetMaxRequiredPortsPerMediaUnit();
	for (int i = 0; i < max_units_video; i++)
	{
		for (int j = 0; j < max_media_ports; j++)
		{
			if (videoAlloc.m_unitsList[i].m_MediaPortsList[j].m_type != eLogical_res_none)
			{
				DWORD connId = pSystemResources->AllocateConnId();
				if (connId == 0)
				{
					//rollback
					for (int k = 0; k <= i; k++)
					{
						for (int l = 0; ((k < i) && (l < max_media_ports)) || ((k == i) && (l < j)); l++)
						{
							if (videoAlloc.m_unitsList[k].m_MediaPortsList[l].m_type != eLogical_res_none)
							{
								pSystemResources->DeAllocateConnId(videoAlloc.m_unitsList[k].m_MediaPortsList[l].m_connId);
							}
						}
					}

					PASSERT(1);
					return STATUS_INSUFFICIENT_RSRC_CONNECTION_ID;
				}
				else
				{
					videoAlloc.m_unitsList[i].m_MediaPortsList[j].m_connId = connId;
				}
			}
		}
	}

	STATUS status = pSystemResources->AllocateVideo(confId, partyId, videoAlloc, partyData);

	//*** get status and rollback if needed
	if (status != STATUS_OK)
	{
		WORD max_units_video = CHelperFuncs::GetMaxUnitsNeededForVideo();
		WORD max_media_ports = CHelperFuncs::GetMaxRequiredPortsPerMediaUnit();
		for (int i = 0; i < max_units_video; i++)
		{
			for (int j = 0; j < max_media_ports; j++)
			{
				if (videoAlloc.m_unitsList[i].m_MediaPortsList[j].m_type != eLogical_res_none)
				{
					pSystemResources->DeAllocateConnId(videoAlloc.m_unitsList[i].m_MediaPortsList[j].m_connId);
				}
			}
		}

		PTRACE2INT(eLevelInfoNormal, "CRsrcAlloc::AllocateVideo failed - status", status);
		return status;
	}

	//*** create (and insert) descriptors

	const WORD total_max_video_rsrc = pSystemResources->GetTotalMaxVideoResources();
	CRsrcDesc* pRsrcDescriptor;
	pVideoDescArray = new CRsrcDesc*[total_max_video_rsrc];
	for (int i = 0; i < total_max_video_rsrc; i++)
	{
		pVideoDescArray[i] = NULL;
	}
	int descIndex = 0;

	for (int i = 0; i < max_units_video; i++)
	{
		for (int j = 0; j < max_media_ports; j++)
		{
			if (videoAlloc.m_unitsList[i].m_MediaPortsList[j].m_type != eLogical_res_none)
			{
				pRsrcDescriptor = new CRsrcDesc(videoAlloc.m_unitsList[i].m_MediaPortsList[j].m_connId, videoAlloc.m_unitsList[i].m_MediaPortsList[j].m_type, confId, partyId, 1 /*boxid*/, videoAlloc.m_boardId, 1 /*subboard*/, videoAlloc.m_unitsList[i].m_UnitId, videoAlloc.m_unitsList[i].m_MediaPortsList[j].m_acceleratorId, videoAlloc.m_unitsList[i].m_MediaPortsList[j].m_portId, videoAlloc.m_unitsList[i].m_MediaPortsList[j].m_cntrl_type);

				pVideoDescArray[descIndex] = pRsrcDescriptor;
				descIndex++;

				PASSERT(pConf->AddDesc(pRsrcDescriptor));
			}
		}
	}

	//*** conn-to-card table writting performed in calling (root allocate) function
	//*** for all descriptors at once
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrcAlloc::RemoveAdditionalISDNResources(const char* serviceName, DWORD monitor_conf_id, DWORD rsrcPartyId)
{
	STATUS final_status = STATUS_OK;

	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	if (pConfRsrcDB == NULL)
	{
		PASSERT(1);
		return STATUS_FAIL;
	}

	CConfRsrc* pConf = (CConfRsrc*)(pConfRsrcDB->GetConfRsrc(monitor_conf_id));
	if (pConf == NULL)
	{
		PASSERT(1);
		return STATUS_CONFERENCE_NOT_EXISTS;
	}

	CPartyRsrc *pParty = (CPartyRsrc*)(pConf->GetPartyRsrcByRsrcPartyId(rsrcPartyId));
	if (pParty == NULL)
	{
		PASSERT(1);
		return STATUS_PARTY_NOT_EXISTS;
	}

	//treatment for temporary bonding number
	Phone* pPhone = pConf->GetTempBondingPhoneNumberByRsrcPartyId(rsrcPartyId);
	if (pPhone)
	{
		DWORD monitor_party_id = pParty->GetMonitorPartyId();
		final_status = DeAllocateBondingTemporaryNumber(serviceName, monitor_conf_id, monitor_party_id, pPhone->phone_number);
	}

	int numDialInReservedPorts = pParty->GetDialInReservedPorts();
	if (numDialInReservedPorts != 0)
	{
		TRACEINTO << "\nRemoveAdditionalISDNResources, numDialInReservedPorts is not zero: " << numDialInReservedPorts << " for party " << rsrcPartyId << "\n\n";

		pParty->SetDialInReservedPorts(0);
		CNetServicesDB* pNetServicesDB = CHelperFuncs::GetNetServicesDB();
		if (NULL == pNetServicesDB)
		{
			PASSERT(1);
			return STATUS_FAIL;
		}

		pNetServicesDB->RemoveDialInReservedPorts(serviceName, numDialInReservedPorts);
	}

	return final_status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrcAlloc::DeAllocateISDNResources(DWORD monitor_conf_id, DWORD rsrcPartyId, char* serviceName, DWORD *connIdRTMArray, BYTE is_udp)
{
	STATUS status = STATUS_OK;

	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	if (pConfRsrcDB == NULL)
	{
		TRACEINTO << "\nRTM_ALLOC: CRsrcAlloc::DeAllocateISDNResources FAILED Details:" << " monitor_conf_id = " << monitor_conf_id << " rsrcPartyId = " << rsrcPartyId << "\n";
		PASSERT(1);
		return STATUS_FAIL;
	}
	CConfRsrc* pConf = (CConfRsrc*)(pConfRsrcDB->GetConfRsrc(monitor_conf_id));
	if (pConf == NULL)
	{
		TRACEINTO << "\nRTM_ALLOC: CRsrcAlloc::DeAllocateISDNResources FAILED Details:" << " monitor_conf_id = " << monitor_conf_id << " rsrcPartyId = " << rsrcPartyId << "\n";

		PASSERT(1);
		return STATUS_CONFERENCE_NOT_EXISTS;
	}

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	if (pSystemResources == NULL)
	{
		TRACEINTO << "\nRTM_ALLOC: CRsrcAlloc::DeAllocateISDNResources FAILED Details:" << " monitor_conf_id = " << monitor_conf_id << " rsrcPartyId = " << rsrcPartyId << "\n";
		PASSERT(1);
		return STATUS_FAIL;
	}

	DWORD rsrcConfId = pConf->GetRsrcConfId();

	TRACEINTO << "\nRTM_ALLOC: CRsrcAlloc::DeAllocateISDNResources Details:" << " monitor_conf_id = " << monitor_conf_id << " rsrcConfId = " << rsrcConfId << " rsrcPartyId = " << rsrcPartyId << "\n";

	RemoveAdditionalISDNResources(serviceName, monitor_conf_id, rsrcPartyId);

	CRsrcDesc** pRsrcDescArray = new CRsrcDesc*[MAX_NUM_ALLOCATED_RSRCS_NET];

	PASSERT_AND_RETURN_VALUE(NULL == pRsrcDescArray, STATUS_FAIL);

	for (int i = 0; i < MAX_NUM_ALLOCATED_RSRCS_NET; i++)
		pRsrcDescArray[i] = NULL;

	// Get all resource descriptors of RTM per a given party
	pConf->GetDescArrayPerResourceTypeByRsrcId(rsrcPartyId, eLogical_net, pRsrcDescArray, MAX_NUM_ALLOCATED_RSRCS_NET);

	CRsrcDesc** pRsrcDescArrayTemp = pRsrcDescArray;
	DWORD* pConnIdRTMArrayTemp = connIdRTMArray;
	BYTE j = 0;

	while (j < MAX_NUM_ALLOCATED_RSRCS_NET && *pRsrcDescArrayTemp)
	{
		j++;
		*pConnIdRTMArrayTemp = (*pRsrcDescArrayTemp) ? (*pRsrcDescArrayTemp)->GetConnId() : 0;	// Updating connection id array for removing later from shared memory
		DeAllocateRTMOneChannel(pConf, *pRsrcDescArrayTemp, is_udp);
		pRsrcDescArrayTemp++;
		pConnIdRTMArrayTemp++;
	}

	if (j >= MAX_NUM_ALLOCATED_RSRCS_NET)
		TRACEINTO << "\n CRsrcAlloc::DeAllocateISDNResources - Internal Error - j = " << j << "\n";

	if (pRsrcDescArray)
	{
		delete[] pRsrcDescArray;
		pRsrcDescArray = NULL;
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrcAlloc::DeAllocateRTMOneChannel(CConfRsrc* pConf, const CRsrcDesc* pRTMDesc, BYTE is_udp)
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	if (NULL == pSystemResources)
	{
		PASSERT(1);
		return STATUS_FAIL;
	}

	if (NULL == pRTMDesc)
	{
		PASSERT(1);
		return STATUS_FAIL;
	}

	STATUS status = STATUS_OK;

	DWORD rsrcPartyId = pRTMDesc->GetRsrcPartyId();
	BOOL bIsUpdated = pRTMDesc->GetIsUpdated();
	DWORD connIdRTM = pRTMDesc->GetConnId();
	pSystemResources->DeAllocateConnId(connIdRTM);

	WORD channelId = pRTMDesc->GetChannelId();
	pSystemResources->DeAllocateChannelId(pRTMDesc->GetBoardId(), channelId);

	PhysicalPortDesc* pPortDesc = new PhysicalPortDesc;

	pPortDesc->m_boxId = pRTMDesc->GetBoxId();
	pPortDesc->m_boardId = pRTMDesc->GetBoardId();
	pPortDesc->m_subBoardId = pRTMDesc->GetSubBoardId();
	pPortDesc->m_unitId = pRTMDesc->GetUnitId();
	pPortDesc->m_portId = pRTMDesc->GetFirstPortId();
	pPortDesc->m_acceleratorId = pRTMDesc->GetAcceleratorId();

	//kill port ???
	if (TRUE == is_udp)
	{
		// 1. kill "udp" port sent to RTM
		STATUS kill_udp_stat = SendIsdnUdpKillPortRequest(pPortDesc, channelId, ePhysical_rtm);
		if (STATUS_OK != kill_udp_stat)
			PASSERT(kill_udp_stat);

		// kill "udp" port sent to art port
		CRsrcDesc* pEncDesc = (CRsrcDesc*)(pConf->GetDesc(rsrcPartyId, eLogical_audio_encoder));
		if (pEncDesc)
		{
			PhysicalPortDesc* pPortDescArt = new PhysicalPortDesc;
			pPortDescArt->m_boxId = pEncDesc->GetBoxId();
			pPortDescArt->m_boardId = pEncDesc->GetBoardId();
			pPortDescArt->m_subBoardId = pEncDesc->GetSubBoardId();
			pPortDescArt->m_unitId = pEncDesc->GetUnitId();
			pPortDescArt->m_portId = pEncDesc->GetFirstPortId();
			pPortDescArt->m_acceleratorId = pEncDesc->GetAcceleratorId();

			kill_udp_stat = SendIsdnUdpKillPortRequest(pPortDescArt, channelId, ePhysical_art);
			if (STATUS_OK != kill_udp_stat)
				PASSERT(kill_udp_stat);

			POBJDELETE(pPortDescArt);
		}

	}
	status = pSystemResources->DeAllocateRTMOneChannel(pConf, rsrcPartyId, pPortDesc, bIsUpdated);

	PASSERT(pConf->RemoveDesc(rsrcPartyId, eLogical_net, E_NORMAL, pRTMDesc->GetFirstPortId(), pRTMDesc->GetUnitId(), pRTMDesc->GetAcceleratorId(), pRTMDesc->GetBoardId()));

	POBJDELETE(pPortDesc);

	return status;
}

////////////////////////////////////////////////////////////////////////////
void CRsrcAlloc::TestAllocateDeAllocateFixed()
{
	ALLOC_PARTY_REQ_PARAMS_S* pReqA = new ALLOC_PARTY_REQ_PARAMS_S;
	ALLOC_PARTY_IND_PARAMS_S* pResA = new ALLOC_PARTY_IND_PARAMS_S;

	DEALLOC_PARTY_REQ_PARAMS_S* pReqD = new DEALLOC_PARTY_REQ_PARAMS_S;
	DEALLOC_PARTY_IND_PARAMS_S* pResD = new DEALLOC_PARTY_IND_PARAMS_S;

	pReqA->monitor_conf_id = 11;
	pReqA->monitor_party_id = 11;
	pReqA->videoPartyType = eCP_H264_upto_CIF_video_party_type;
	pReqA->networkPartyType = eIP_network_party_type;
	pReqA->sessionType = eCP_session;
	Allocate(pReqA, pResA);

	pReqA->monitor_conf_id = 22;
	pReqA->monitor_party_id = 11;
	Allocate(pReqA, pResA);

	pReqA->videoPartyType = eVideo_party_type_none;
	pReqA->networkPartyType = eIP_network_party_type;
	pReqA->monitor_conf_id = 22;
	pReqA->monitor_party_id = 22;
	Allocate(pReqA, pResA);

	pReqD->monitor_conf_id = 11;
	pReqD->monitor_party_id = 11;
	DeAllocate(pReqD, pResD);

	pReqD->monitor_conf_id = 22;
	pReqD->monitor_party_id = 11;
	DeAllocate(pReqD, pResD);

	pReqA->monitor_conf_id = 11;
	pReqA->monitor_party_id = 11;
	pReqA->videoPartyType = eCP_H264_upto_CIF_video_party_type;
	pReqA->networkPartyType = eIP_network_party_type;

	pReqA->sessionType = eCP_session;
	Allocate(pReqA, pResA);

	pReqA->monitor_conf_id = 22;
	pReqA->monitor_party_id = 11;
	Allocate(pReqA, pResA);
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrcAlloc::SimulateAllocateUDP(DWORD rsrcConfId, DWORD rsrcPartyId, eVideoPartyType videoPartyType, UdpAddresses& udp)
{
	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();

	if (pConfRsrcDB == NULL)
	{
		PASSERT(1);
		return STATUS_FAIL;
	}

	CConfRsrc* pConf = (CConfRsrc*)(pConfRsrcDB->GetConfRsrcByRsrcConfId(rsrcConfId));
	if (pConf == NULL)
	{
		PASSERT(1);
		return STATUS_FAIL;
	}

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();

	if (pSystemResources == NULL)
	{
		PASSERT(1);
		return STATUS_FAIL;
	}

	WORD servId = pSystemResources->SimulateAllocateUDP(udp);
	//'servId' temp used for fixed port index
	if (servId == 0xFF)
	{
		PASSERT(1);
		return STATUS_FAIL;
	}

	CUdpRsrcDesc* pUdpRsrcDesc = new CUdpRsrcDesc(rsrcPartyId);
	AUTO_DELETE(pUdpRsrcDesc);
	pUdpRsrcDesc->SetServId(servId);

	if (videoPartyType == eVideo_party_type_none)	//no video needed for VOIP
	{
		udp.VideoChannelPort = 0;
		udp.VideoChannelAdditionalPorts = 0;	//ICE 4 ports
	}

	memcpy(&(pUdpRsrcDesc->m_udp), &udp, sizeof(UdpAddresses));

	if (pConf->AddUdpDesc(pUdpRsrcDesc) == STATUS_FAIL)
		POBJDELETE(pUdpRsrcDesc);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrcAlloc::SimulateDeAllocateUDP(DWORD rsrcConfId, DWORD rsrcPartyId)
{
	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();

	if (pConfRsrcDB == NULL)
	{
		PASSERT(1);
		return STATUS_FAIL;
	}

	CConfRsrc* pConf = (CConfRsrc*)(pConfRsrcDB->GetConfRsrcByRsrcConfId(rsrcConfId));
	if (pConf == NULL)
	{
		PASSERT(1);
		return STATUS_FAIL;
	}

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();

	if (pSystemResources == NULL)
	{
		PASSERT(1);
		return STATUS_FAIL;
	}

	CUdpRsrcDesc* pUdpRsrcDesc = (CUdpRsrcDesc*)(pConf->GetUdpDesc(rsrcPartyId));
	if (!pUdpRsrcDesc)
	{
		PASSERT(1);
		return STATUS_FAIL;
	}

	WORD servId = pUdpRsrcDesc->GetServId();
	//'servid' temp used for udp fixed port counter
	PASSERT(pSystemResources->SimulateDeAllocateUDP(servId) != STATUS_OK);

	pConf->RemoveUdpDesc(rsrcPartyId);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrcAlloc::AllocateUDPtoParty(WORD servId, WORD subServiceId, WORD boxId, WORD boardId, WORD subBoardId, WORD unitId, eVideoPartyType videoPartyType, CUdpRsrcDesc* pUdpRsrcDesc, bool isIceParty, bool isBFCPUDP)	//ICE 4 ports
{
	pUdpRsrcDesc->SetServId(servId);
	pUdpRsrcDesc->SetsubServId(subServiceId);

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();

	if (!pSystemResources)
	{
		PASSERT(1);
		return STATUS_FAIL;
	}

	CPQperSrvResource* pPQM = pSystemResources->ARTtoPQM(servId, subServiceId, boxId, boardId, subBoardId, unitId);

	if (!pPQM)
	{
		PASSERT(1);
		return STATUS_INSUFFICIENT_UDP_PORTS;
	}

	UdpAddresses& udpa = pUdpRsrcDesc->m_udp;

	STATUS status = pPQM->AllocateUDP(pUdpRsrcDesc->GetRsrcPartyId(), videoPartyType, udpa.AudioChannelPort, udpa.VideoChannelPort, udpa.FeccChannelPort, udpa.ContentChannelPort, udpa.BfcpChannelPort, udpa.AudioChannelAdditionalPorts, udpa.VideoChannelAdditionalPorts, udpa.FeccChannelAdditionalPorts, udpa.ContentChannelAdditionalPorts, udpa.BfcpChannelAdditionalPorts, isIceParty, isBFCPUDP);	//ICE 4 ports

	if (status == STATUS_OK)
	{
		udpa.IpType = pPQM->GetIpType();
		pUdpRsrcDesc->SetIpAddresses(pPQM->GetIpV4Addr(), pPQM->GetIpV6Addr());
		pUdpRsrcDesc->SetPQMId(pPQM->GetPQMId());
	}
	else
	{
		PTRACE(eLevelInfoNormal, "CRsrcAlloc::AllocateUDPtoParty - AllocateUDP failed");
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////
// The functions allocates UDP ports for audio only party upgrading to video
STATUS CRsrcAlloc::AllocateAdditionalUDPtoParty(WORD servId, WORD subServiceId, WORD boxId, WORD boardId, WORD subBoardId, WORD unitId, CUdpRsrcDesc* pUdpRsrcDesc, bool isIceParty, bool isBFCPUDP)	//ICE 4 ports
{
	pUdpRsrcDesc->SetServId(servId);

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();

	if (!pSystemResources)
	{
		PASSERT(1);
		return STATUS_FAIL;
	}

	CPQperSrvResource* pPQM = pSystemResources->ARTtoPQM(servId, subServiceId, boxId, boardId, subBoardId, unitId);

	if (!pPQM)
	{
		PASSERT(1);
		return STATUS_INSUFFICIENT_UDP_PORTS;
	}

	UdpAddresses& udpa = pUdpRsrcDesc->m_udp;

	STATUS status = pPQM->AllocateAdditionalUDP(pUdpRsrcDesc->GetRsrcPartyId(), udpa.VideoChannelPort, udpa.FeccChannelPort, udpa.ContentChannelPort, udpa.BfcpChannelPort, udpa.VideoChannelAdditionalPorts, udpa.FeccChannelAdditionalPorts, udpa.ContentChannelAdditionalPorts, udpa.BfcpChannelAdditionalPorts, isIceParty, isBFCPUDP);	//ICE 4 ports

	if (status != STATUS_OK)
		PTRACE(eLevelInfoNormal, "CRsrcAlloc::AllocateAdditionalUDPtoParty - AllocateUDP failed");

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrcAlloc::DeAllocatePartyUDP(WORD boxId, WORD boardId, WORD subBoardId, WORD unitId, CUdpRsrcDesc* pUdpDesc, bool disable)
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();

	if (!pSystemResources)
	{
		PASSERT(1);
		return STATUS_FAIL;
	}

	if (!IsValidPObjectPtr(pUdpDesc))
	{
		PTRACE(eLevelError, "CRsrcAlloc::DeAllocatePartyUDP - pUdpDesc is NULL");
		return STATUS_FAIL;
	}

	CPQperSrvResource* pPQM = pSystemResources->ARTtoPQM(pUdpDesc->GetServId(), pUdpDesc->GetsubServId(), boxId, boardId, subBoardId, unitId);

	if (!pPQM)
	{
		PASSERT(1);
		return STATUS_FAIL;
	}

	UdpAddresses& udpa = pUdpDesc->m_udp;

	udpa.AudioChannelPort = udpa.VideoChannelPort = udpa.FeccChannelPort = udpa.ContentChannelPort = udpa.BfcpChannelPort = udpa.AudioChannelAdditionalPorts = udpa.VideoChannelAdditionalPorts = udpa.FeccChannelAdditionalPorts = udpa.ContentChannelAdditionalPorts = udpa.BfcpChannelAdditionalPorts = 0;

	return pPQM->DeAllocateUDP(pUdpDesc->GetRsrcPartyId(), disable);
}

////////////////////////////////////////////////////////////////////////////
void CRsrcAlloc::ReAllocate(ALLOC_PARTY_REQ_PARAMS_S* pParam, ALLOC_PARTY_IND_PARAMS_S* pResult)
{
	PASSERT_AND_RETURN(!pParam);
	PASSERT_AND_RETURN(!pResult);

	CConfRsrcDB*      pConfRsrcDB      = CHelperFuncs::GetConfRsrcDB();
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();

	pResult->allocIndBase.status = STATUS_FAIL;
	PASSERT_AND_RETURN(!pConfRsrcDB);
	PASSERT_AND_RETURN(!pSystemResources);
	pResult->allocIndBase.status = STATUS_OK;

	eProductType prodType = pSystemResources->GetProductType();

	ConfMonitorID  monitor_conf_id  = pParam->monitor_conf_id;
	PartyMonitorID monitor_party_id = pParam->monitor_party_id;
	ConfRsrcID     rsrcConfId       = pConfRsrcDB->MonitorToRsrcConfId(monitor_conf_id);
	PartyRsrcID    rsrcPartyId      = pConfRsrcDB->MonitorToRsrcPartyId(monitor_conf_id, monitor_party_id);

	TRACEINTO << "ConfId:" << rsrcConfId << ", PartyId:" << rsrcPartyId << ", MonitorConfId:" << monitor_conf_id << ", MonitorPartyId:" << monitor_party_id << ", VideoPartyType:" << eVideoPartyTypeNames[pParam->videoPartyType];

	CConfRsrc* pConfRsrc = pConfRsrcDB ? pConfRsrcDB->GetConfRsrc(monitor_conf_id) : NULL;

	if (CHelperFuncs::IsISDNParty(pParam->networkPartyType))
		ReAllocateRTM(pParam, pResult);

	if (pResult->allocIndBase.status == STATUS_OK)
	{
		pResult->allocIndBase.confMediaType = pConfRsrc ? pConfRsrc->GetConfMediaType() : eConfMediaType_dummy;

		if (CHelperFuncs::IsAudioParty(pParam->videoPartyType) && (prodType != eProductTypeSoftMCUMfw)) // Downgrade to Audio Only (VoIP or PSTN)
		{
			FreeNonAudioResources(pParam, pResult);
		}
		else if (eTipVideoAux == pParam->tipPartyType)
		{
			AllocateTIPContent(pParam, pResult);
		}
		else if (!CHelperFuncs::IsNeedAllocateVideo(pParam->videoPartyType, CHelperFuncs::GetConferenceMode(monitor_conf_id), prodType))
		{
			ReallocateSoft(pParam, pResult);
		}
		else
			ReAllocateVideo(pParam, pResult);

		CheckIfAllocOfAVCHDisSupportedAndUpdate(pParam, pResult);
	}
}

////////////////////////////////////////////////////////////////////////////
void CRsrcAlloc::ReallocateSoft(ALLOC_PARTY_REQ_PARAMS_S* pParam, ALLOC_PARTY_IND_PARAMS_S* pResult)
{
	PASSERT_AND_RETURN(!pParam);
	PASSERT_AND_RETURN(!pResult);

	CConfRsrcDB*      pConfRsrcDB      = CHelperFuncs::GetConfRsrcDB();
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();

	PASSERT_AND_RETURN(!pConfRsrcDB);
	PASSERT_AND_RETURN(!pSystemResources);

	ResourcesInterface* pResourcesInterface = pSystemResources->GetCurrentResourcesInterface();
	PASSERT_AND_RETURN(!pResourcesInterface);

	ConfMonitorID  monitor_conf_id  = pParam->monitor_conf_id;
	PartyMonitorID monitor_party_id = pParam->monitor_party_id;
	ConfRsrcID     rsrcConfId       = pConfRsrcDB->MonitorToRsrcConfId(monitor_conf_id);
	PartyRsrcID    rsrcPartyId      = pConfRsrcDB->MonitorToRsrcPartyId(monitor_conf_id, monitor_party_id);

	pResult->allocIndBase.rsrc_conf_id     = rsrcConfId;
	pResult->allocIndBase.rsrc_party_id    = rsrcPartyId;
	pResult->allocIndBase.networkPartyType = pParam->networkPartyType;

	CConfRsrc* pConfRsrc = pConfRsrcDB->GetConfRsrc(monitor_conf_id);
	if (pConfRsrc)
	{
		CPartyRsrc* pParty = (CPartyRsrc*)pConfRsrc->GetPartyRsrcByRsrcPartyId(rsrcPartyId);
		if (pParty)
		{
			BOOL bAddAudioAsVideo = FALSE;
			eVideoPartyType partyType = pParty->GetVideoPartyType();
			//Check license restrictions
			if (!pResourcesInterface->CheckIfOnePartyCanBechanged(partyType, pParty->GetPartyRole(), pParam->videoPartyType, pParam->partyRole, &bAddAudioAsVideo, CHelperFuncs::GetConferenceMode(monitor_conf_id)))
			{
				TRACEINTO << "PartyId:" << rsrcPartyId << ", VideoPartyType:" << eVideoPartyTypeNames[partyType] << " - Failed, not enough resources to reallocate party";
				// Dynamic Ports Allocation v7.1 - return current allocation + success, instead of failure status
				pResult->allocIndBase.videoPartyType = partyType;
				return;
			}

			pResult->allocIndBase.videoPartyType = pParam->videoPartyType;

			TRACEINTO << "PartyId:" << rsrcPartyId << ", OldVideoPartyType:" << eVideoPartyTypeNames[partyType] << ", NewVideoPartyType:" << eVideoPartyTypeNames[pParam->videoPartyType];
			UpdatePartyResourceUsage(*pConfRsrc, *pParty, pParam->videoPartyType, bAddAudioAsVideo);
			UpdatePartyType(pConfRsrcDB, monitor_conf_id, monitor_party_id, pParam->videoPartyType);
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CRsrcAlloc::ReAllocateART(ALLOC_PARTY_REQ_PARAMS_S* pParam, ALLOC_PARTY_IND_PARAMS_S* pResult)
{
	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	PASSERT_AND_RETURN(!pConfRsrcDB);

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN(!pSystemResources);

	ConfRsrcID  confId  = pConfRsrcDB->MonitorToRsrcConfId(pParam->monitor_conf_id);
	PartyRsrcID partyId = pConfRsrcDB->MonitorToRsrcPartyId(pParam->monitor_conf_id, pParam->monitor_party_id);

	TRACEINTO
		<< "\n  ConfId                     :" << confId
		<< "\n  PartyId                    :" << partyId
		<< "\n  MonitorConfId              :" << pParam->monitor_conf_id
		<< "\n  MonitorPartyId             :" << pParam->monitor_party_id
		<< "\n  ArtCapacity (Kbytes)       :" << pParam->artCapacity;

	STATUS status = pSystemResources->ReAllocateART(confId, partyId, pParam);
}

////////////////////////////////////////////////////////////////////////////
void CRsrcAlloc::ReAllocateRTMBoardFull(BOARD_FULL_REQ_PARAMS_S* pParam, ALLOC_PARTY_IND_PARAMS_S* pResult)
{
	STATUS status = STATUS_OK;

	DWORD monitor_conf_id = pParam->allocPartyReqParams.monitor_conf_id;
	DWORD monitor_party_id = pParam->allocPartyReqParams.monitor_party_id;

	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	if (!pConfRsrcDB)
	{
		PASSERTMSG(1, "ConfRsrcDB error");
		pResult->allocIndBase.status = STATUS_FAIL;
		return;
	}

	CConfRsrc* pConf = (CConfRsrc*)(pConfRsrcDB->GetConfRsrc(monitor_conf_id));
	if (pConf == NULL)
	{
		PASSERTMSG(1, "Conference not found");
		pResult->allocIndBase.status = STATUS_FAIL;
		return;
	}

	DWORD rsrcConfId = pConf->GetRsrcConfId();

	const CPartyRsrc* pParty = ((CConfRsrc*)(pConf))->GetParty(monitor_party_id);
	if (!pParty)
	{
		PASSERTMSG(1, "Party not found");
		pResult->allocIndBase.status = STATUS_FAIL;
		return;
	}

	DWORD rsrcPartyId = pParty->GetRsrcPartyId();

	// Update result
	pResult->allocIndBase.rsrc_conf_id = rsrcConfId;
	pResult->allocIndBase.rsrc_party_id = rsrcPartyId;

	CSystemResources* pSyst = CHelperFuncs::GetSystemResources();
	if (NULL == pSyst)
	{
		PASSERTMSG(1, "pSyst is NULL");
		pResult->allocIndBase.status = STATUS_FAIL;
		return;
	}

	int boardFullBId = pParam->allocPartyReqParams.isdn_span_params.board_id;

	if (!boardFullBId)
	{
		PASSERTMSG(1, "Illegal Board ID");
		pResult->allocIndBase.status = STATUS_FAIL;
		return;
	}

	eVideoPartyType videoPartyType = pParam->allocPartyReqParams.videoPartyType;
	eNetworkPartyType networkPartyType = pParam->allocPartyReqParams.networkPartyType;

	if (!CHelperFuncs::IsISDNParty(networkPartyType))
	{
		PASSERTMSG(1, "Illegal Party Type");
		pResult->allocIndBase.status = STATUS_FAIL;
		return;
	}

	// Verify that the all connections ids received from ConfParty are indeed belong to channels on the full board and indeed not updated
	BYTE countConnId = 0;
	for (int j = 0; pParam->connectionIdList[j] != 0; j++)
	{
		CRsrcDesc* rsrcDesc = pConf->GetDescFromConnectionId(pParam->connectionIdList[j]);
		if (!rsrcDesc)
		{
			PASSERTMSG(1, "Rsrc Descriptor not found");
			pResult->allocIndBase.status = STATUS_FAIL;
			return;
		}
		if (rsrcDesc->GetBoardId() != boardFullBId)
		{
			PASSERTMSG(1, "Board id indicated as full is different from the party resource board id");
			pResult->allocIndBase.status = STATUS_FAIL;
			return;
		}
		if (rsrcDesc->GetIsUpdated())
		{
			PASSERTMSG(1, "Channel sent in board full request mustn't be updated");
			pResult->allocIndBase.status = STATUS_FAIL;
			return;
		}

		countConnId++;
	}

	int actualNumberOfRTMPortsToAllocate = pParam->allocPartyReqParams.isdn_span_params.num_of_isdn_ports;
	if (countConnId != actualNumberOfRTMPortsToAllocate)
	{
		PASSERTMSG(1, "Mismatch between number of channels and connection id list size");
		pResult->allocIndBase.status = STATUS_FAIL;
		return;
	}

	PartyDataStruct partyData;
	memset(&partyData, 0, sizeof(partyData));
	partyData.m_videoPartyType = videoPartyType;
	partyData.m_networkPartyType = networkPartyType;

	partyData.m_pIsdn_Params_Request = &pParam->allocPartyReqParams.isdn_span_params;

	status = pSyst->GetBestAllocationForRTMOnly(partyData, &pResult->isdnParams, actualNumberOfRTMPortsToAllocate, boardFullBId, 0); // Need to add prefferable Bid support

	// If status is OK, Need to reallocate the already allocated channels
	if (STATUS_OK == status)
	{
		CNetServicesDB* pNetServicesDB = pSyst->GetNetServicesDB();
		if (NULL == pNetServicesDB)
		{
			PASSERTMSG(1, "Error retreiving NetServicesDB");
			pResult->allocIndBase.status = STATUS_FAIL;
			return;
		}

		UPDATE_ISDN_PORT_S pRTMPortToUpdate;
		pRTMPortToUpdate.monitor_conf_id = monitor_conf_id;
		pRTMPortToUpdate.monitor_party_id = monitor_party_id;

		for (int i = 0; i < actualNumberOfRTMPortsToAllocate; i++)
		{
			pRTMPortToUpdate.connection_id = pParam->connectionIdList[i];
			pRTMPortToUpdate.span_id = pResult->isdnParams.spans_order.port_spans_list[i].spans_list[0]; // ???
			pRTMPortToUpdate.board_id = pResult->isdnParams.spans_order.port_spans_list[i].board_id;

			ACK_UPDATE_ISDN_PORT_S pResultOfUpdate;

			pNetServicesDB->RTMUpdatePortRequest(&pRTMPortToUpdate, &pResultOfUpdate, FALSE);

			if (STATUS_OK == pResultOfUpdate.status)
				pResult->isdnParams.spans_order.port_spans_list[i].conn_id = pRTMPortToUpdate.connection_id;
			else
			{
				PASSERTMSG(pRTMPortToUpdate.connection_id, "Failed to update channel");
				pResult->allocIndBase.status = STATUS_FAIL;
				return;
			}
		}
	}

	pResult->allocIndBase.status = status;
}

////////////////////////////////////////////////////////////////////////////
void CRsrcAlloc::FreeNonAudioResources(ALLOC_PARTY_REQ_PARAMS_S* pParam, ALLOC_PARTY_IND_PARAMS_S* pResult)
{
	//** check parameters validity
	//** eliminate descriptor of enc_slave, release active port, connId
	//** remove conn-to-card table entry of enc_slave
	//** update conn-to-card table entry for enc_master
	//** update active ports for enc and dec
	//** update number of ports
	//** fill in the indication

	PTRACE(eLevelInfoNormal, "Enter CRsrcAlloc::FreeNonAudioResources");
	STATUS status = STATUS_OK;

	DWORD monitor_conf_id = pParam->monitor_conf_id;
	DWORD monitor_party_id = pParam->monitor_party_id;
	eNetworkPartyType networkPartyType = pParam->networkPartyType; //SUPPOSE: party type refers to current ( VIDEO )
	DWORD rsrcConfId = 0, rsrcPartyId = 0;

	if (!CHelperFuncs::IsAudioParty(pParam->videoPartyType))
	{
		PASSERT(1);
		PTRACE2INT(eLevelError, "Illegal required party type for FreeNonAudioResources - ", pParam->videoPartyType);
		pResult->allocIndBase.status = STATUS_FAIL;
		return;
	}

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	CConnToCardManager* pConnToCardMngr = CHelperFuncs::GetConnToCardManager();

	if (!pConfRsrcDB || !pSystemResources || !pConnToCardMngr)
	{
		PASSERT(1);
		pResult->allocIndBase.status = STATUS_FAIL;
		return;
	}

	CConfRsrc* pConf = (CConfRsrc*)(pConfRsrcDB->GetConfRsrc(monitor_conf_id));
	if (!pConf)
	{
		PASSERT(1);
		PTRACE2INT(eLevelError, "FreeNonAudioResources req. for non-existing conf., monitorId =", monitor_conf_id);
		pResult->allocIndBase.status = STATUS_CONFERENCE_NOT_EXISTS;
		return;
	}

	rsrcConfId = pConfRsrcDB->MonitorToRsrcConfId(monitor_conf_id);
	rsrcPartyId = pConfRsrcDB->MonitorToRsrcPartyId(monitor_conf_id, monitor_party_id);

	CPartyRsrc* pParty = (CPartyRsrc*)(pConf->GetPartyRsrcByRsrcPartyId(rsrcPartyId));
	if (!pParty)
	{
		PASSERT(1);
		PTRACE2INT(eLevelError, "FreeNonAudioResources req. for non-existing party, monitorId =", monitor_party_id);
		pResult->allocIndBase.status = STATUS_PARTY_NOT_EXISTS;
		return;
	}

	eVideoPartyType oldVideoPartyType = pParty->GetVideoPartyType();
	ePartyRole oldPartyRole = pParty->GetPartyRole();
	eVideoPartyType newVideoPartyType = eVideo_party_type_none;
	ePartyRole newPartyRole = pParam->partyRole;

	if (CHelperFuncs::IsAudioParty(oldVideoPartyType))
	{
		//in ISDN/PSTN we might get to the realloc function because we change the RTM channels and not the party type
		if (CHelperFuncs::IsISDNParty(networkPartyType))
		{
			PTRACE(eLevelInfoNormal, " request for FreeNonAudioResources for a party that is already PSTN");
		}
		else
		{
			PASSERT(1);
			PTRACE2INT(eLevelError, "Illegal old party type party type for FreeNonAudioResources, not for ISDN- ", oldVideoPartyType);
			pResult->allocIndBase.status = STATUS_FAIL;
		}

		return;
	}

	//check if maximum of Audio Parties in System already reached
	if (!pSystemResources->GetCurrentResourcesInterface())
	{
		pResult->allocIndBase.status = STATUS_FAIL;
		PASSERT(1);
		return;
	}

	BOOL bAddAudioAsVideo = FALSE;
	ResourcesInterface* pResourceInterface = pSystemResources->GetCurrentResourcesInterface();
	if (pResourceInterface && !pResourceInterface->CheckIfOnePartyCanBechanged(oldVideoPartyType, oldPartyRole, newVideoPartyType, newPartyRole, &bAddAudioAsVideo, CHelperFuncs::GetConferenceMode(monitor_conf_id)))
	{
		TRACEINTO << "request for FreeNonAudioResources - maximum audio parties in system already reached";
		// Dynamic Ports Allocation v7.1 - return current allocation + success, instead of failure status
		pResult->allocIndBase.status = STATUS_OK;
		pResult->allocIndBase.networkPartyType = pParam->networkPartyType;
		pResult->allocIndBase.rsrc_conf_id = rsrcConfId;
		pResult->allocIndBase.rsrc_party_id = rsrcPartyId;
		pResult->allocIndBase.videoPartyType = oldVideoPartyType;

		return;
	}

	ConnectionIDs connectionIdsVideo;

	WORD videoBoardId = 0xFFFF;
	CRsrcDesc* pVideoDecDesc = (CRsrcDesc*)pConf->GetDesc(rsrcPartyId, eLogical_video_decoder);
	if (!pVideoDecDesc)
	{
		TRACEINTO << "The video decoder descriptor isn't found for party with monitor_party_id=" << monitor_party_id;
	}
	else
	{
		videoBoardId = pVideoDecDesc->GetBoardId();
	}

	status = DeallocateVideo(monitor_conf_id, rsrcPartyId, connectionIdsVideo);

	if (STATUS_OK == status)
	{
		WORD artBoardId = 0xFFFF;
		CRsrcDesc* pAudioDesc = (CRsrcDesc*)pConf->GetDesc(rsrcPartyId, eLogical_audio_encoder);
		if (!pAudioDesc)
			TRACEINTO << "the Audio Encoder descriptor isn't found for party which monitor_party_id = " << monitor_party_id;
		else
		{
			artBoardId = pAudioDesc->GetBoardId();
			CBoard* pBoard = pSystemResources->GetBoard(artBoardId);
			if (pBoard)
				pBoard->RemoveOneVideoParticipantsWithArtOnThisBoard();
		}

		ConnectionIDs::iterator _itr, _end = connectionIdsVideo.end();
		for (_itr = connectionIdsVideo.begin(); _itr != _end; ++_itr)
			PASSERTSTREAM(pConnToCardMngr->Remove(*_itr), "ConnId:" << *_itr);

		UpdatePartyType(pConfRsrcDB, monitor_conf_id, monitor_party_id, newVideoPartyType);

		WORD service_id = ID_ALL_IP_SERVICES;
		if (pSystemResources->GetMultipleIpServices())
		{
			service_id = pSystemResources->GetPartyIpServiceId(rsrcPartyId);
		}

		BOOL isIsdnParty = CHelperFuncs::IsISDNParty(networkPartyType);
		BasePartyDataStruct remPartyData(oldVideoPartyType, oldPartyRole, rsrcPartyId, isIsdnParty, artBoardId, videoBoardId, CHelperFuncs::GetConferenceMode(monitor_conf_id), FALSE, service_id, bAddAudioAsVideo);
		pSystemResources->RemoveParty(remPartyData);

		BasePartyDataStruct addPartyData(newVideoPartyType, newPartyRole, rsrcPartyId, isIsdnParty, artBoardId, 0xFFFF, CHelperFuncs::GetConferenceMode(monitor_conf_id), FALSE, service_id, bAddAudioAsVideo);
		pSystemResources->AddParty(addPartyData);

		// new type
		pResult->allocIndBase.videoPartyType = newVideoPartyType;

		// UDP ports for video - should be freed (only for VOIP)
		if (CHelperFuncs::IsIPParty(networkPartyType))
		{
			CRsrcDesc* pRtpDesc = (CRsrcDesc*)(pConf->GetDesc(rsrcPartyId, eLogical_rtp));
			CUdpRsrcDesc* pUdpDesc = (CUdpRsrcDesc*)(pConf->GetUdpDesc(rsrcPartyId));

			if (!pRtpDesc || !pUdpDesc)
				PASSERT(1);
			else
			{
				WORD servId = pUdpDesc->GetServId();
				WORD subServiceId = pUdpDesc->GetsubServId();

				CPQperSrvResource* pPQM = pSystemResources->ARTtoPQM(servId, subServiceId, pRtpDesc->GetBoxId(), pRtpDesc->GetBoardId(), pRtpDesc->GetSubBoardId(), pRtpDesc->GetUnitId());

				if (!pPQM)
					PASSERT(1);
				else
				{
					UdpAddresses& udpa = pUdpDesc->m_udp;
					WORD dummyPort = 0;

					STATUS res = pPQM->DeAllocateUDP(pUdpDesc->GetRsrcPartyId(), dummyPort, udpa.VideoChannelPort, udpa.FeccChannelPort, udpa.ContentChannelPort, udpa.BfcpChannelPort, dummyPort, udpa.VideoChannelAdditionalPorts, udpa.FeccChannelAdditionalPorts, udpa.ContentChannelAdditionalPorts, udpa.BfcpChannelAdditionalPorts, false);
				}
			}
		}
		// UDP ports for video - - should be freed
	}
	else
		//status not ok
		PASSERT(1);

	if (CHelperFuncs::GetConferenceMode(monitor_conf_id) == eMix) // while reallocate to audio in Mixed-mode need to free ART ports related to AVC translators
	{
		DeallocateAvcToSvcMixedART(pConf, pParty, eLogical_relay_avc_to_svc_rtp, false);

		if (eProductFamilyRMX != CProcessBase::GetProcess()->GetProductFamily())
		{
			DeallocateAvcToSvcMixedART(pConf, pParty, eLogical_relay_avc_to_svc_rtp_with_audio_encoder, false);
		}
	}

	pResult->allocIndBase.rsrc_conf_id = rsrcConfId;
	pResult->allocIndBase.rsrc_party_id = rsrcPartyId;
	pResult->allocIndBase.networkPartyType = pParam->networkPartyType;
	pResult->allocIndBase.status = status;
	// pResult->allocIndBase.numPsrc = 0 by default
}

////////////////////////////////////////////////////////////////////////////
void CRsrcAlloc::ReAllocateVideo(ALLOC_PARTY_REQ_PARAMS_S* pParam, ALLOC_PARTY_IND_PARAMS_S* pResult)
{
	//** check parameters validity
	//** eliminate descriptor of enc_slave, release active port, connId
	//** remove conn-to-card table entry of enc_slave
	//** update conn-to-card table entry for enc_master
	//** update active ports for enc and dec
	//** update number of ports
	//** fill in the indication

	PTRACE(eLevelInfoNormal, "Enter ReAllocateVideo");

	DWORD monitor_conf_id = pParam->monitor_conf_id;
	DWORD monitor_party_id = pParam->monitor_party_id;
	eVideoPartyType reqVideoPartyType = pParam->videoPartyType;
	ePartyRole reqPartyRole = pParam->partyRole;
	DWORD rsrcConfId = 0, rsrcPartyId = 0;
	WORD subServiceId = pParam->subServiceId;

	////////////////////////////////////////////////////////////////////////
	//check validity of all params
	////////////////////////////////////////////////////////////////////////
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	CConnToCardManager* pConnToCardMngr = CHelperFuncs::GetConnToCardManager();

	if (pConfRsrcDB == NULL || pSystemResources == NULL || pConnToCardMngr == NULL)
	{
		PASSERT(1);
		pResult->allocIndBase.status = STATUS_FAIL;
		return;
	}

	CConfRsrc* pConf = (CConfRsrc*)(pConfRsrcDB->GetConfRsrc(monitor_conf_id));
	if (pConf == NULL)
	{
		PASSERT(1);
		PTRACE2INT(eLevelError, "ReAllocateVideo req. for non-existing conf., monitorId =", monitor_conf_id);
		pResult->allocIndBase.status = STATUS_CONFERENCE_NOT_EXISTS;
		return;
	}

	rsrcConfId = pConfRsrcDB->MonitorToRsrcConfId(monitor_conf_id);
	rsrcPartyId = pConfRsrcDB->MonitorToRsrcPartyId(monitor_conf_id, monitor_party_id);

	CPartyRsrc* pParty = (CPartyRsrc*)(pConf->GetPartyRsrcByRsrcPartyId(rsrcPartyId));
	if (pParty == NULL)
	{
		PASSERT(1);
		PTRACE2INT(eLevelError, "ReAllocateVideo req. for non-existing party, monitorId =", monitor_party_id);
		pResult->allocIndBase.status = STATUS_PARTY_NOT_EXISTS;
		return;
	}
	pParam->party_id = rsrcPartyId;

	BOOL bIsdnParty = CHelperFuncs::IsISDNParty(pParty->GetNetworkPartyType());

	ePartyRole currPartyRole = pParty->GetPartyRole();
	eVideoPartyType curVideoPartyType = pParty->GetVideoPartyType();
	if (reqVideoPartyType == curVideoPartyType)
	{
		//in ISDN/PSTN we might get to the realloc function because we change the RTM channels and not the party type
		if (CHelperFuncs::IsISDNParty(pParty->GetNetworkPartyType()))
		{
			PTRACE(eLevelInfoNormal, " request for ReAllocateVideo for the same ISDN party type");
		}
		else
		{
			DBGPASSERT(1);
			PTRACE(eLevelInfoNormal, " request for ReAllocateVideo for the same party type, that is not ISDN");
			pResult->allocIndBase.status = STATUS_OK;
		}
		return;
	}

	if (pParty->GetNetworkPartyType() != pParam->networkPartyType)
	{
		PASSERT(1);
		PTRACE(eLevelInfoNormal, " request for ReAllocateVideo with a change of network party type!!!");
		pResult->allocIndBase.status = STATUS_ILLEGAL;
		return;
	}

	BOOL bIllegalTypeForReallocate = FALSE;

	bIllegalTypeForReallocate = !(CHelperFuncs::IsLegalVideoParty(reqVideoPartyType));
	//tbd zoe - breeze: check if there are new types of participants

	if (bIllegalTypeForReallocate)
	{
		PASSERT(1);
		PTRACE2INT(eLevelError, "Illegal required party type for ReAllocateVideo - ", reqVideoPartyType);
		pResult->allocIndBase.status = STATUS_FAIL;
		return;
	}

	/////////////////////////////////////////////////////////////////////////////////
	//Check license restrictions
	////////////////////////////////////////////////////////////////////////////////
	ResourcesInterface* pResourcesInterface = pSystemResources->GetCurrentResourcesInterface();
	if (pResourcesInterface && !pResourcesInterface->CheckIfOnePartyCanBechanged(curVideoPartyType, currPartyRole, reqVideoPartyType, reqPartyRole, NULL, CHelperFuncs::GetConferenceMode(monitor_conf_id)))
	{
		PTRACE(eLevelInfoNormal, " CRsrcAlloc::ReAllocateVideo - not enough resources - orig type will be returned");
		// Dynamic Ports Allocation v7.1 - return current allocation + success, instead of failure status
		pResult->allocIndBase.status = STATUS_OK;
		pResult->allocIndBase.videoPartyType = curVideoPartyType;
		pResult->allocIndBase.networkPartyType = pParam->networkPartyType;
		pResult->allocIndBase.rsrc_conf_id = rsrcConfId;
		pResult->allocIndBase.rsrc_party_id = rsrcPartyId;

		return;
	}

	////////////////////////////////////////////////////////////////////////////////
	//If old party type is video: Deallocate + remove descriptors for video
	//if it was audio: allocate UDP ports
	////////////////////////////////////////////////////////////////////////////////
	typedef std::map<eLogicalResourceTypes, std::pair<WORD, BYTE> > ConnectionIdMap;

	ConnectionIdMap oldConnIds;

	DWORD encoderConnId = 0;
	int indexOfLastConnId = 0;
	STATUS status = STATUS_OK;
	WORD oldVideoBoardId = 0xFFFF, newVideoBoardId = 0xFFFF, artBoardId = 0xFFFF;

	if (CHelperFuncs::IsVideoParty(curVideoPartyType))
	{
		//if before that it was video, free it, but save the connection ids
		CRsrcDesc* pDecDesc = (CRsrcDesc*)(pConf->GetDesc(rsrcPartyId, eLogical_video_decoder));
		if (pDecDesc)
			oldVideoBoardId = pDecDesc->GetBoardId();
		else
		{
			if (CHelperFuncs::IsVideoRelayParty(curVideoPartyType))
			{
				oldVideoBoardId = pConf->GetBoardIdForRelayParty();
			}
			else
			{
				PASSERT(1);
				pResult->allocIndBase.status = STATUS_FAIL;
				return;
			}
		}

		CBoard *pBoard = pSystemResources->GetBoard(oldVideoBoardId);
		if (pBoard == NULL)
		{
			PASSERT(1);
			pResult->allocIndBase.status = STATUS_FAIL;
			return;
		}

		AllocData videoCurrentAllocData;
		videoCurrentAllocData.m_confModeType = CHelperFuncs::GetConferenceMode(monitor_conf_id);

		//McKinsey: downgrade from 720p to 360p is not supported yet if it will be supported on future need to do as below
		/*
		 if (eMixAvcSvc == pConf->GetConfMediaType() && CHelperFuncs::IsVideoRelayParty(curVideoPartyType) && !pParam->isHdVswEnabledInMixAvcSvcMode && pParty->isHdVswEnabledInMixAvcSvcMode())
		 {
		 //DeAllocateDecoderOf720 TODO
		 //Allocate decoder of 360
		 pBoard->FillVideoUnitsList(curVideoPartyType, videoCurrentAllocData,eParty_Role_regular_party);
		 pParty->SetHD720Supported(false);
		 }
		 else*/

		pBoard->FillVideoUnitsList(curVideoPartyType, videoCurrentAllocData);

		videoCurrentAllocData.m_boardId = oldVideoBoardId;
		newVideoBoardId = oldVideoBoardId;

		CRsrcDesc* pDesc = NULL;
		DWORD connId;

		ECntrlType cntrl_type;
		eLogicalResourceTypes type;

		WORD max_units_video = CHelperFuncs::GetMaxUnitsNeededForVideo();
		WORD max_media_ports = CHelperFuncs::GetMaxRequiredPortsPerMediaUnit();
		for (int i = 0; i < max_units_video; i++)
		{
			for (int j = 0; j < max_media_ports; j++)
			{
				if (videoCurrentAllocData.m_unitsList[i].m_MediaPortsList[j].m_type != eLogical_res_none)
				{
					cntrl_type = videoCurrentAllocData.m_unitsList[i].m_MediaPortsList[j].m_cntrl_type;
					type = videoCurrentAllocData.m_unitsList[i].m_MediaPortsList[j].m_type;
					pDesc = (CRsrcDesc*)(pConf->GetDesc(rsrcPartyId, type, cntrl_type));

					if (pDesc == NULL)
					{
						PASSERT(1 + type);
						pResult->allocIndBase.status = STATUS_FAIL;
						return;
					}

					if (type == eLogical_video_encoder && (cntrl_type == E_NORMAL || CHelperFuncs::IsMasterType(cntrl_type)))
						encoderConnId = pDesc->GetConnId();
					else
					{
						TRACEINTO << "OLGA_DEBUG: save connId=" << pDesc->GetConnId() << " for lrType=" << type;

						oldConnIds.insert(std::make_pair(type, std::make_pair(pDesc->GetConnId(), 0)));
					}

					videoCurrentAllocData.m_unitsList[i].m_MediaPortsList[j].m_connId = pDesc->GetConnId();
					videoCurrentAllocData.m_unitsList[i].m_UnitId = pDesc->GetUnitId();
					videoCurrentAllocData.m_unitsList[i].m_MediaPortsList[j].m_portId = pDesc->GetFirstPortId();
					videoCurrentAllocData.m_unitsList[i].m_MediaPortsList[j].m_acceleratorId = pDesc->GetAcceleratorId();
					PASSERT(pConf->RemoveDesc(rsrcPartyId, videoCurrentAllocData.m_unitsList[i].m_MediaPortsList[j].m_type, videoCurrentAllocData.m_unitsList[i].m_MediaPortsList[j].m_cntrl_type));
				}
			}
		}

		status = pSystemResources->DeAllocateVideo(pConf, rsrcPartyId, videoCurrentAllocData, curVideoPartyType, FALSE);
		PASSERT(status);

		// Save art board id for later
		CRsrcDesc* pAudioDesc = (CRsrcDesc*)pConf->GetDesc(rsrcPartyId, eLogical_audio_encoder);
		if (pAudioDesc)
		{
			artBoardId = pAudioDesc->GetBoardId();
		}
	}
	else
	{
		CRsrcDesc* pAudioDesc = (CRsrcDesc*)pConf->GetDesc(rsrcPartyId, eLogical_audio_encoder);
		if (!pAudioDesc)
		{
			PASSERTMSG(1, "CRsrcAlloc::ReAllocateVideo - Couldn't find audio descriptor");
			pResult->allocIndBase.status = STATUS_FAIL;
			return;
		}

		artBoardId = pAudioDesc->GetBoardId();

		// Save the art board id in order to allocate the video on the same board, as a first priority.
		newVideoBoardId = artBoardId;

		if (CHelperFuncs::IsIPParty(pParam->networkPartyType))
		{
			CUdpRsrcDesc* pUdpRsrcDesc = (CUdpRsrcDesc*)pConf->GetUdpDesc(rsrcPartyId);

			if (pUdpRsrcDesc)
			{
				CUdpRsrcDesc* pUdpRsrcDescCopy = new CUdpRsrcDesc(*pUdpRsrcDesc);
				WORD serviceId = pUdpRsrcDesc->GetServId();

				STATUS udp_status = AllocateAdditionalUDPtoParty(serviceId, subServiceId, pAudioDesc->GetBoxId(), pAudioDesc->GetBoardId(), pAudioDesc->GetSubBoardId(), pAudioDesc->GetUnitId(), pUdpRsrcDesc, pParam->isIceParty, pParam->isBFCP);    //ICE 4 ports

				if (udp_status != STATUS_OK)
				{
					PASSERTMSG(1, "CRsrcAlloc::ReAllocateVideo - Error in AllocateAdditionalUDPtoParty");
					pResult->allocIndBase.status = udp_status;
					return;
				}
				// Update result
				memcpy(&pResult->udpAdresses, &pUdpRsrcDesc->m_udp, sizeof(UdpAddresses));

				PASSERT(pConf->UpdateUdpDesc(rsrcPartyId, pUdpRsrcDescCopy));
			}
			else
				PASSERTMSG(1, "CRsrcAlloc::ReAllocateVideo - Couldn't find UDP resource descriptor");
		}
	}
	////////////////////////////////////////////////////////////////////////////////
	//Try to find appropriate board.
	//If it already was video: try first on same board
	//if it was audio: try where art was
	//if not try on other boards
	////////////////////////////////////////////////////////////////////////////////
	VideoDataPerBoardStruct videoData;
	memset(&videoData, 0, sizeof(videoData));
	PartyDataStruct partyData;
	memset(&partyData, 0, sizeof(partyData));

	partyData.m_reqBoardId = 0xFFFF;

	if (STATUS_OK == status)
	{
		BOOL canVideoRequestBeSatisfied = TRUE; // Not used
		partyData.m_videoPartyType = reqVideoPartyType;
		partyData.m_monitor_conf_id = monitor_conf_id;
		partyData.m_confModeType = CHelperFuncs::GetConferenceMode(monitor_conf_id);
		partyData.m_HdVswTypeInMixAvcSvcMode = pParam->HdVswTypeInMixAvcSvcMode;

		//Upgrade from Audio to Video
		if (CHelperFuncs::IsAudioParty(curVideoPartyType))
		{
			bool hasRelayParties = pConf->CheckIfThereAreRelayParty();
			if (pConf && hasRelayParties)
				partyData.m_reqBoardId = (eMixAvcSvc == pConf->GetConfMediaType()) ? pConf->GetBoardIdForRelayParty() : 0xFFFF;

			TRACEINTO << "Reallocate from Voice to Video type : rsrcConfId=" << rsrcConfId << ", rsrcPartyId=" << rsrcPartyId << ", MIX-mode=" << (WORD)partyData.m_confModeType << ", boardIdSVC=" << partyData.m_reqBoardId;
		}

		pSystemResources->CollectVideoAvailabilityForBoard(videoData, partyData, newVideoBoardId, canVideoRequestBeSatisfied);

		if (videoData.m_bCanBeAllocated == FALSE) //if couldn't allocate on this board, try on others
		{
			for (int i = 1; i <= BOARDS_NUM; i++)
			{
				if (i == newVideoBoardId) //don't try again on same board
					continue;

				memset(&videoData, 0, sizeof(videoData));
				memset(&partyData, 0, sizeof(partyData));
				partyData.m_videoPartyType = reqVideoPartyType;
				partyData.m_HdVswTypeInMixAvcSvcMode = pParam->HdVswTypeInMixAvcSvcMode;

				pSystemResources->CollectVideoAvailabilityForBoard(videoData, partyData, i, canVideoRequestBeSatisfied);

				if (videoData.m_bCanBeAllocated == TRUE) //found another board
					break;
			}
		}

		if (videoData.m_bCanBeAllocated == TRUE)
		{
			// vngr-17042 : reallocate video allocate HD720 although only SD available
			//reqVideoPartyType = partyData.m_videoPartyType;
			reqVideoPartyType = videoData.m_bWasDownGraded ? videoData.m_DownGradedPartyType : partyData.m_videoPartyType;

			newVideoBoardId = videoData.m_VideoAlloc.m_boardId;

			status = pSystemResources->AllocateVideo(rsrcConfId, rsrcPartyId, videoData.m_VideoAlloc, partyData);

			if (CHelperFuncs::IsAudioParty(curVideoPartyType))
			{   // Dynamic Ports Allocation v7.1 - change audio-->video party
				CBoard* pBoard = pSystemResources->GetBoard(newVideoBoardId);
				if (pBoard)
					pBoard->AddOneVideoParticipantsWithArtOnThisBoard();
			}

			//now copy the connIds from the old one, and remove the unnecessary ones
			int lastCurrentConnId = 0;
			WORD max_units_video = CHelperFuncs::GetMaxUnitsNeededForVideo();
			WORD max_media_ports = CHelperFuncs::GetMaxRequiredPortsPerMediaUnit();
			for (int i = 0; i < max_units_video; i++)
			{
				for (int j = 0; j < max_media_ports; j++)
				{
					eLogicalResourceTypes lrType = videoData.m_VideoAlloc.m_unitsList[i].m_MediaPortsList[j].m_type;
					ECntrlType cntType = videoData.m_VideoAlloc.m_unitsList[i].m_MediaPortsList[j].m_cntrl_type;

					if (lrType != eLogical_res_none)
					{
						if (lrType == eLogical_video_encoder && (encoderConnId != 0) && (cntType == E_NORMAL || CHelperFuncs::IsMasterType(cntType)))
						{
							videoData.m_VideoAlloc.m_unitsList[i].m_MediaPortsList[j].m_connId = encoderConnId;
							videoData.m_VideoAlloc.m_unitsList[i].m_MediaPortsList[j].m_bNewEntryInSharedMemory = FALSE;
						}
						else
						{
							ConnectionIdMap::iterator itr = oldConnIds.find(lrType);
							if (itr != oldConnIds.end())
							{
								TRACEINTO << "OLGA_DEBUG: use old connId=" << itr->second.first << " for lrType=" << lrType;
								videoData.m_VideoAlloc.m_unitsList[i].m_MediaPortsList[j].m_connId = itr->second.first;
								itr->second.second = 1; //to mark it as "in use"
								videoData.m_VideoAlloc.m_unitsList[i].m_MediaPortsList[j].m_bNewEntryInSharedMemory = FALSE;
							}
							else
							{
								TRACEINTO << "OLGA_DEBUG: allocate new connId for lrType=" << lrType;
								videoData.m_VideoAlloc.m_unitsList[i].m_MediaPortsList[j].m_connId = pSystemResources->AllocateConnId();
								videoData.m_VideoAlloc.m_unitsList[i].m_MediaPortsList[j].m_bNewEntryInSharedMemory = TRUE;
							}
						}
					}
				}
			}
			for (ConnectionIdMap::iterator itr = oldConnIds.begin(); itr != oldConnIds.end(); ++itr)
			{
				if (0 == itr->second.second) //we didn't use all of them
				{
					TRACEINTO << "OLGA_DEBUG: deallocate old connId=" << itr->second.first << " for lrType=" << itr->first;
					pSystemResources->DeAllocateConnId(itr->second.first);
					pConnToCardMngr->Remove(itr->second.first);
				}
			}
		}
		else
			status = STATUS_INSUFFICIENT_VIDEO_RSRC;

		//if succeeded, add descriptors
		if (STATUS_OK == status)
		{
			CRsrcDesc* pRsrcDescriptor;
			WORD max_units_video = CHelperFuncs::GetMaxUnitsNeededForVideo();
			WORD max_media_ports = CHelperFuncs::GetMaxRequiredPortsPerMediaUnit();
			for (int i = 0; i < max_units_video; i++)
			{
				for (int j = 0; j < max_media_ports; j++)
				{
					if (videoData.m_VideoAlloc.m_unitsList[i].m_MediaPortsList[j].m_type != eLogical_res_none)
					{
						pRsrcDescriptor = new CRsrcDesc(videoData.m_VideoAlloc.m_unitsList[i].m_MediaPortsList[j].m_connId, videoData.m_VideoAlloc.m_unitsList[i].m_MediaPortsList[j].m_type, rsrcConfId, rsrcPartyId, 1 /*boxid*/, videoData.m_VideoAlloc.m_boardId, 1 /*subboard*/, videoData.m_VideoAlloc.m_unitsList[i].m_UnitId, videoData.m_VideoAlloc.m_unitsList[i].m_MediaPortsList[j].m_acceleratorId, videoData.m_VideoAlloc.m_unitsList[i].m_MediaPortsList[j].m_portId, videoData.m_VideoAlloc.m_unitsList[i].m_MediaPortsList[j].m_cntrl_type);

						PASSERT(pConf->AddDesc(pRsrcDescriptor));

						POBJDELETE(pRsrcDescriptor);
					}
				}
			}
		}
		else
		{
			PASSERT(status);
			//deallocate succeeded but allocate didn't.
			//This also means that the ART is still occupied: in the future we should check what happens with the ART allocation
			WORD boardIdSVC = CHelperFuncs::IsVideoRelayParty(curVideoPartyType) ? pConf->GetBoardIdForRelayParty() : artBoardId;
			BasePartyDataStruct partyData(curVideoPartyType, currPartyRole, rsrcPartyId, bIsdnParty, boardIdSVC, oldVideoBoardId, CHelperFuncs::GetConferenceMode(monitor_conf_id), FALSE, 0, FALSE);
			pSystemResources->RemoveParty(partyData);
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////
	// update shared memory
	//////////////////////////////////////////////////////////////////////////////////////
	if (STATUS_OK == status)
	{
		ConnToCardTableEntry EntrySrc;
		WORD max_units_video = CHelperFuncs::GetMaxUnitsNeededForVideo();
		WORD max_media_ports = CHelperFuncs::GetMaxRequiredPortsPerMediaUnit();
		int numberOfEntries = 0;

		std::ostringstream msg;
		ConnToCardEntryDumpHeader(msg, "CRsrcAlloc::ReAllocateVideo - Shared memory", rsrcConfId, rsrcPartyId, pParam->room_id);

		for (int i = 0; i < max_units_video; i++)
		{
			for (int j = 0; j < max_media_ports; j++)
			{
				eLogicalResourceTypes lrType = videoData.m_VideoAlloc.m_unitsList[i].m_MediaPortsList[j].m_type;
				if (lrType != eLogical_res_none)
				{

					pResult->allocIndBase.allocatedRrcs[pResult->allocIndBase.numRsrcs].connectionId = videoData.m_VideoAlloc.m_unitsList[i].m_MediaPortsList[j].m_connId;
					pResult->allocIndBase.allocatedRrcs[pResult->allocIndBase.numRsrcs].logicalRsrcType = lrType;

					BOOL isNewEntry = videoData.m_VideoAlloc.m_unitsList[i].m_MediaPortsList[j].m_bNewEntryInSharedMemory;
					status = (!isNewEntry) ? pConnToCardMngr->Get(videoData.m_VideoAlloc.m_unitsList[i].m_MediaPortsList[j].m_connId, EntrySrc) : STATUS_OK;

					if (status == STATUS_OK)
					{
						ConnToCardTableEntry copyEntrySrc(EntrySrc);

						EntrySrc.boxId = 1;
						EntrySrc.boardId = videoData.m_VideoAlloc.m_boardId;
						EntrySrc.subBoardId = 1;
						EntrySrc.unitId = videoData.m_VideoAlloc.m_unitsList[i].m_UnitId;
						EntrySrc.acceleratorId = videoData.m_VideoAlloc.m_unitsList[i].m_MediaPortsList[j].m_acceleratorId;
						EntrySrc.portId = videoData.m_VideoAlloc.m_unitsList[i].m_MediaPortsList[j].m_portId;
						EntrySrc.rsrcType = lrType;
						EntrySrc.physicalRsrcType = videoData.m_VideoAlloc.m_unitsList[i].m_MediaPortsList[j].GetPhysicalResourceType();
						EntrySrc.rsrcCntlType = videoData.m_VideoAlloc.m_unitsList[i].m_MediaPortsList[j].m_cntrl_type;
						EntrySrc.rsrc_conf_id = rsrcConfId;
						EntrySrc.rsrc_party_id = rsrcPartyId;

						if (!CHelperFuncs::IsLogicalSoftMixVideoEncoderType(lrType) || (copyEntrySrc.boardId == EntrySrc.boardId && copyEntrySrc.acceleratorId == EntrySrc.acceleratorId && copyEntrySrc.unitId == EntrySrc.unitId && copyEntrySrc.portId == EntrySrc.portId))
						{
							EntrySrc.m_id = videoData.m_VideoAlloc.m_unitsList[i].m_MediaPortsList[j].m_connId;
						}
						else //we change a connection ID of AVC Translator when it is moving to another port, because ConfParty must update MRMP in this case
						{
							DWORD oldConnID = videoData.m_VideoAlloc.m_unitsList[i].m_MediaPortsList[j].m_connId;
							if (!isNewEntry)
							{
								TRACEINTO << "OLGA_DEBUG: need to deallocate old connId=" << oldConnID << " for lrType=" << lrType;
								pConnToCardMngr->Remove(oldConnID);
							}
							else
							{
								TRACEINTO << "OLGA_DEBUG: NO need to remove from SM connId=" << oldConnID << " because it's new for lrType=" << lrType;
							}

							pSystemResources->DeAllocateConnId(oldConnID);

							EntrySrc.m_id = pSystemResources->AllocateConnId();

							videoData.m_VideoAlloc.m_unitsList[i].m_MediaPortsList[j].m_connId = EntrySrc.m_id;
							videoData.m_VideoAlloc.m_unitsList[i].m_MediaPortsList[j].m_bNewEntryInSharedMemory = TRUE;

							PASSERT(pConf->RemoveDesc(rsrcPartyId, lrType, videoData.m_VideoAlloc.m_unitsList[i].m_MediaPortsList[j].m_cntrl_type));

							CRsrcDesc rsrcDescriptor(videoData.m_VideoAlloc.m_unitsList[i].m_MediaPortsList[j].m_connId, lrType, rsrcConfId, rsrcPartyId, 1 /*boxid*/, videoData.m_VideoAlloc.m_boardId, 1 /*subboard*/, videoData.m_VideoAlloc.m_unitsList[i].m_UnitId, videoData.m_VideoAlloc.m_unitsList[i].m_MediaPortsList[j].m_acceleratorId, videoData.m_VideoAlloc.m_unitsList[i].m_MediaPortsList[j].m_portId, videoData.m_VideoAlloc.m_unitsList[i].m_MediaPortsList[j].m_cntrl_type);

							PASSERT(pConf->AddDesc(&rsrcDescriptor));

							pResult->allocIndBase.allocatedRrcs[pResult->allocIndBase.numRsrcs].connectionId = videoData.m_VideoAlloc.m_unitsList[i].m_MediaPortsList[j].m_connId;
						}

						if (videoData.m_VideoAlloc.m_unitsList[i].m_MediaPortsList[j].m_bNewEntryInSharedMemory == FALSE)
							status = pConnToCardMngr->Update(EntrySrc);
						else
							status = AddEntryToSharedMemory(EntrySrc); // pConnToCardMngr->Add(EntrySrc);

						PASSERT(status);
						numberOfEntries += ConnToCardEntryDump(msg, EntrySrc);

						pResult->allocIndBase.numRsrcs++;
					}
					else
						PASSERT(status);
				}
			}
		}

		//in MIX-mode
		TRACEINTO << "currPartyRole is: " << currPartyRole;
		if ((eMixAvcSvc == pConf->GetConfMediaType()) && (eParty_Role_content_decoder != currPartyRole) && (eParty_Role_content_encoder != currPartyRole))
		{

			if (pParty->m_ssrcAudio != 0) //in MIX-mode: SSRC and relay_rtp are allocated at the call beginning for all party types (including Voice)
			{
				const CRsrcDesc* pRtpRelayDesc = pConf->GetDesc(rsrcPartyId, eLogical_relay_rtp);
				if (pRtpRelayDesc && pConnToCardMngr)
				{
					WORD numRsrcInResult = pResult->allocIndBase.numRsrcs;
					pResult->allocIndBase.allocatedRrcs[numRsrcInResult].connectionId = pRtpRelayDesc->GetConnId();
					pResult->allocIndBase.allocatedRrcs[numRsrcInResult].logicalRsrcType = eLogical_relay_rtp;
					pResult->allocIndBase.numRsrcs++;
				}

				pResult->svcParams.m_ssrcAudio = pParty->m_ssrcAudio;

				for (int i = 0; i < MAX_NUM_RECV_STREAMS_FOR_VIDEO; i++)
					pResult->svcParams.m_ssrcVideo[i] = pParty->m_ssrcVideo[i];
			}

			BOOL isRelayParty = (CHelperFuncs::IsVideoRelayParty(curVideoPartyType) || (eVoice_relay_party_type == curVideoPartyType));
			if (!isRelayParty) //for AVC party need to allocate additional ART port/s
			{

				ConnToCardTableEntry cmEntry;
				cmEntry.rsrc_conf_id = rsrcConfId;
				cmEntry.rsrc_party_id = rsrcPartyId;
				cmEntry.room_id = pParty->GetRoomPartyId();

				WORD reqBoardId = partyData.m_reqBoardId;
				const CRsrcDesc* pRtpDesc = pConf->GetDesc(rsrcPartyId, eLogical_rtp);
				if (0xFFFF == reqBoardId && pRtpDesc)
					reqBoardId = pRtpDesc->GetBoardId();

				if (CHelperFuncs::IsAudioParty(curVideoPartyType))
				{
					//need to allocate additional ART port (2 for video)
					int numRsrc = AllocateAvcARTResources(pParam, partyData, reqBoardId, cmEntry, msg, pResult);

					pResult->allocIndBase.numRsrcs += numRsrc;
				}
				else
				{
					////////////////////////////////////////////////////////////////////////////////
					//If old party type is HD video and Mixed conf mode: Deallocate + remove descriptors for video enc_2 and ART
					// else  if new type is HD then it is needed to allocate ART
					////////////////////////////////////////////////////////////////////////////////

					BOOL isNeeded = FALSE, isSvc = FALSE, currentIsHD = FALSE, reqIsHD = FALSE;
					CHelperFuncs::GetRequiredAvcSvcTranslatingVideoResorces(curVideoPartyType, isNeeded, isSvc, currentIsHD);
					CHelperFuncs::GetRequiredAvcSvcTranslatingVideoResorces(reqVideoPartyType, isNeeded, isSvc, reqIsHD);

					const CRsrcDesc* pRelayAvcToSvcRtp = pConf->GetDesc(rsrcPartyId, eLogical_relay_avc_to_svc_rtp);

					if (currentIsHD && !reqIsHD)
					{
						TRACEINTO << "Deallocate redundant ART due to downgrade party rsrcPartyId=" << rsrcPartyId << "from curVideoPartyType=" << eVideoPartyTypeNames[curVideoPartyType] << " to reqVideoPartyType=" << eVideoPartyTypeNames[reqVideoPartyType];

						DeallocateAvcToSvcMixedART(pConf, pParty, eLogical_relay_avc_to_svc_rtp, false);
					}
					else if (!currentIsHD && reqIsHD && !pRelayAvcToSvcRtp)
					{
						PartyDataStruct partyData;

						InitPartyDataStructOnUpgradeToAvcSvcMix(partyData, *pConf, *pParty);

						STATUS status_mix_art = AllocateAvcToSvcMixedART(pParam, partyData, reqBoardId, cmEntry, msg, pResult);

						if (status_mix_art != STATUS_OK)
							DBGPASSERT(rsrcPartyId);					// TODO rollback?
					}

					pRelayAvcToSvcRtp = pConf->GetDesc(rsrcPartyId, eLogical_relay_avc_to_svc_rtp);

					if (pRelayAvcToSvcRtp)
					{
						DWORD numRsrcInResult = pResult->allocIndBase.numRsrcs;

						pResult->allocIndBase.allocatedRrcs[numRsrcInResult].connectionId = pRelayAvcToSvcRtp->GetConnId();
						pResult->allocIndBase.allocatedRrcs[numRsrcInResult].logicalRsrcType = eLogical_relay_avc_to_svc_rtp;
						pResult->allocIndBase.numRsrcs += 1;
					}

					//Party request:  to return both mixed ARTs
					const CRsrcDesc* pRelayAvcToSvcRtpWithAudioEnc = pConf->GetDesc(rsrcPartyId, eLogical_relay_avc_to_svc_rtp_with_audio_encoder);
					if (pRelayAvcToSvcRtpWithAudioEnc)
					{
						DWORD numRsrcInResult = pResult->allocIndBase.numRsrcs;

						pResult->allocIndBase.allocatedRrcs[numRsrcInResult].connectionId = pRelayAvcToSvcRtpWithAudioEnc->GetConnId();
						pResult->allocIndBase.allocatedRrcs[numRsrcInResult].logicalRsrcType = eLogical_relay_avc_to_svc_rtp_with_audio_encoder;
						pResult->allocIndBase.numRsrcs += 1;
					}
				}
			}
		}

		if (numberOfEntries)
			ConnToCardEntryDumpFooter(msg);

		//////////////////////////////////////////////////////////////////////////////////////
		// update party type + update number of resources
		//////////////////////////////////////////////////////////////////////////////////////
		if (STATUS_OK == status)
		{
			UpdatePartyType(pConfRsrcDB, monitor_conf_id, monitor_party_id, reqVideoPartyType);

			WORD service_id = ID_ALL_IP_SERVICES;
			if (pSystemResources->GetMultipleIpServices())
			{
				service_id = pSystemResources->GetPartyIpServiceId(rsrcPartyId);
			}

			BasePartyDataStruct remPartyData(curVideoPartyType, currPartyRole, rsrcPartyId, bIsdnParty, artBoardId, oldVideoBoardId, CHelperFuncs::GetConferenceMode(monitor_conf_id), FALSE, service_id, FALSE);
			pSystemResources->RemoveParty(remPartyData);

			BasePartyDataStruct addPartyData(reqVideoPartyType, reqPartyRole, rsrcPartyId, bIsdnParty, artBoardId, newVideoBoardId, CHelperFuncs::GetConferenceMode(monitor_conf_id), FALSE, service_id, FALSE);
			pSystemResources->AddParty(addPartyData);
		}

		//////////////////////////////////////////////////////////////////////////////////////
		// fill in output params
		//////////////////////////////////////////////////////////////////////////////////////
		pResult->allocIndBase.rsrc_conf_id = rsrcConfId;
		pResult->allocIndBase.rsrc_party_id = rsrcPartyId;
		pResult->allocIndBase.videoPartyType = reqVideoPartyType;
		pResult->allocIndBase.networkPartyType = pParty->GetNetworkPartyType(); // shouldn't change, we checked it earlier
	}
	pResult->allocIndBase.status = status;
	return;
}

////////////////////////////////////////////////////////////////////////////
void CRsrcAlloc::ReAllocateRTM(ALLOC_PARTY_REQ_PARAMS_S* pParam, ALLOC_PARTY_IND_PARAMS_S* pResult)
{
	///////////////////////////////////////////////////////////////////
	///Preparing for reallocate RTM
	///////////////////////////////////////////////////////////////////
	pResult->allocIndBase.status = STATUS_FAIL;
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN(pSystemResources == NULL);

	CConnToCardManager* pConnToCardMngr = CHelperFuncs::GetConnToCardManager();
	PASSERT_AND_RETURN(pConnToCardMngr == NULL);

	CNetServicesDB* pNetServicesDB = pSystemResources->GetNetServicesDB();
	PASSERT_AND_RETURN(NULL == pNetServicesDB);

	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	PASSERT_AND_RETURN(pConfRsrcDB == NULL);

	WORD numRsrcsToBeReallocated = pParam->isdn_span_params.num_of_isdn_ports;
	PASSERTMSG_AND_RETURN(!numRsrcsToBeReallocated, "CRsrcAlloc::ReAllocateRTM req. num of resources to be reallocated is 0\n");
	PASSERTMSG_AND_RETURN(numRsrcsToBeReallocated == 1 && eVideo_party_type_none != pParam->videoPartyType, "CRsrcAlloc::ReAllocateRTM req. num of resources to be reallocated is 1 but video party type isn't eVideo_party_type_none\n");

	DWORD monitor_conf_id = pParam->monitor_conf_id;
	DWORD monitor_party_id = pParam->monitor_party_id;
	DWORD rsrcConfId = pConfRsrcDB->MonitorToRsrcConfId(monitor_conf_id);
	DWORD rsrcPartyId = pConfRsrcDB->MonitorToRsrcPartyId(monitor_conf_id, monitor_party_id);
	CConfRsrc* pConf = (CConfRsrc*)(pConfRsrcDB->GetConfRsrc(monitor_conf_id));
	if (pConf == NULL)
	{
		PASSERT(1);
		PTRACE2INT(eLevelError, "CRsrcAlloc::ReAllocateRTM req. for non-existing conf, monitorId =", monitor_conf_id);
		pResult->allocIndBase.status = STATUS_CONFERENCE_NOT_EXISTS;
		return;
	}
	CPartyRsrc* pParty = (CPartyRsrc*)(pConf->GetPartyRsrcByRsrcPartyId(rsrcPartyId));
	if (pParty == NULL)
	{
		PASSERT(1);
		PTRACE2INT(eLevelError, "CRsrcAlloc::ReAllocateRTM req. for non-existing party, monitorId =", monitor_party_id);
		pResult->allocIndBase.status = STATUS_PARTY_NOT_EXISTS;
		return;
	}

	//all preparation done
	pResult->allocIndBase.status = STATUS_OK;
	pResult->allocIndBase.rsrc_conf_id = rsrcConfId;
	pResult->allocIndBase.rsrc_party_id = rsrcPartyId;
	pResult->allocIndBase.networkPartyType = pParam->networkPartyType;
	pResult->allocIndBase.videoPartyType = pParam->videoPartyType;
	eVideoPartyType oldVideoPartyType = pParty->GetVideoPartyType();
	BOOL l_bIsDialOut = CHelperFuncs::IsDialOutParty(&pParam->isdn_span_params);

	/////////////////////////////////////////////////////////////////////////////
	// Get all RTM descriptors - already allocated
	////////////////////////////////////////////////////////////////////////////
	CRsrcDesc** pRTMDescArrayFromSharedMemory = new CRsrcDesc*[MAX_NUM_ALLOCATED_RSRCS_NET];
	if (NULL == pRTMDescArrayFromSharedMemory)
	{
		pResult->allocIndBase.status = STATUS_FAIL;
		PASSERT_AND_RETURN(1);
	}
	for (int i = 0; i < MAX_NUM_ALLOCATED_RSRCS_NET; i++)
		pRTMDescArrayFromSharedMemory[i] = NULL;
	WORD numRsrcsAllocatedInSharedMemory = ((CConfRsrc*)pConf)->GetDescArrayPerResourceTypeByRsrcId(rsrcPartyId, eLogical_net, pRTMDescArrayFromSharedMemory, MAX_NUM_ALLOCATED_RSRCS_NET);
	if (pRTMDescArrayFromSharedMemory[0] == NULL) //there should be at least one port before reallocate!!!
	{
		PASSERT(1);
		TRACEINTO << "\nCRsrcAlloc::ReAllocateRTM. No rsrc descriptors found at all \n";
		delete[] pRTMDescArrayFromSharedMemory;
		pResult->allocIndBase.status = STATUS_FAIL;
		return;
	}

	WORD numTotalRsrcsAlreadyAllocated = 0;
	if (l_bIsDialOut)
	{
		numTotalRsrcsAlreadyAllocated = numRsrcsAllocatedInSharedMemory;
	}
	else
	{
		if (numRsrcsAllocatedInSharedMemory != 1)
		{
			TRACEINTO << "\nCRsrcAlloc::ReAllocateRTM. More than one channel already allocated (allocated: " << numRsrcsAllocatedInSharedMemory << "). ConfParty asked to assert this. \n";
		}
		numTotalRsrcsAlreadyAllocated = numRsrcsAllocatedInSharedMemory + pParty->GetDialInReservedPorts();
	}

	/////////////////////////////////////////////////////////////////////////////
	//Copy all necessary channel Ids to the result array
	//Arrange them: first the updated ones then the others
	//In case it's downgrade, remove superfluous
	////////////////////////////////////////////////////////////////////////////
	DWORD *connIdRTMArray = new DWORD[numRsrcsToBeReallocated];
	if (connIdRTMArray)
	{
		for (int i = 0; i < numRsrcsToBeReallocated; i++)
			connIdRTMArray[i] = 0;
	}
	else
	{
		delete[] pRTMDescArrayFromSharedMemory;
		pResult->allocIndBase.status = STATUS_FAIL;
		PASSERT_AND_RETURN(1);
	}

	WORD connIdRTMArrayIndex = 0;
	WORD connIdRTMArrayIndexReverse = 0;

	WORD numSuperfluousRsrcs = 0;
	WORD numAdditionalRsrcs = 0;

	if (numTotalRsrcsAlreadyAllocated > numRsrcsToBeReallocated) //downgrade
	{
		numSuperfluousRsrcs = numTotalRsrcsAlreadyAllocated - numRsrcsToBeReallocated;
		connIdRTMArrayIndexReverse = numRsrcsToBeReallocated - 1; // Index to end of array for inserting non-updated resources
	}
	else //reallocate upgrade, or no change
	{
		numAdditionalRsrcs = numRsrcsToBeReallocated - numTotalRsrcsAlreadyAllocated;
		connIdRTMArrayIndexReverse = numRsrcsAllocatedInSharedMemory - 1; // Index to end of array for inserting non-updated resources
	}

	WORD l_numUpdated = 0;
	int numOfConnIdsInRTMArray = 0;
	for (int j = numRsrcsAllocatedInSharedMemory - 1; j >= 0; j--)
	{
		PASSERT(pRTMDescArrayFromSharedMemory[j]==NULL); //this shouldn't happen because numRsrcsAllocated is exactly the number that should be written in this array

		if (pRTMDescArrayFromSharedMemory[j]->GetIsUpdated())
		{
			l_numUpdated++;

			// Insert from beginning
			connIdRTMArray[connIdRTMArrayIndex] = (pRTMDescArrayFromSharedMemory[j] ? pRTMDescArrayFromSharedMemory[j]->GetConnId() : 0);
			connIdRTMArrayIndex++;
			numOfConnIdsInRTMArray++;
		}
		else
		{
			if (l_bIsDialOut == FALSE)	// There should be only one descriptor and it must be updated
			{
				TRACEINTO << "\nCRsrcAlloc::ReAllocateRTM. The descriptor found for dial-in must be updated\n";
				continue;
			}

			if (numSuperfluousRsrcs)
			{
				DeAllocateRTMOneChannel(pConf, pRTMDescArrayFromSharedMemory[j]);
				numSuperfluousRsrcs--;
				// Remove from shared memory
				PASSERT(pConnToCardMngr->Remove(pRTMDescArrayFromSharedMemory[j]->GetConnId()));
			}
			else
			{
				// Insert from the end
				connIdRTMArray[connIdRTMArrayIndexReverse] = (pRTMDescArrayFromSharedMemory[j] ? pRTMDescArrayFromSharedMemory[j]->GetConnId() : 0);
				connIdRTMArrayIndexReverse--;
				numOfConnIdsInRTMArray++;
			}
		}
	}

	// Check that we don't have more than one channel updated (if the reallocate was for reallocating RTM - meaning if the number of RTM cHannels has indeed be changed)
	if (l_numUpdated > 1 && numRsrcsToBeReallocated != numTotalRsrcsAlreadyAllocated)
	{
		if (numTotalRsrcsAlreadyAllocated > numRsrcsToBeReallocated && numRsrcsToBeReallocated + l_numUpdated > numTotalRsrcsAlreadyAllocated) //downgrade + number of updated channels is conflict with requested for reallocation
		{
			DBGPASSERT(l_numUpdated);
			TRACEINTO << "\nCRsrcAlloc::ReAllocateRTM. Wrong number of updated channels\n";
		}
	}

	// Update result array with all resources (updated channels should be at the beginning)
	for (int j = 0; j < numOfConnIdsInRTMArray; j++)
	{
		pResult->allocIndBase.allocatedRrcs[pResult->allocIndBase.numRsrcs].connectionId = connIdRTMArray[j];
		pResult->allocIndBase.allocatedRrcs[pResult->allocIndBase.numRsrcs].logicalRsrcType = eLogical_net;
		pResult->allocIndBase.numRsrcs++;
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	///Additional updates: dial-in reserved ports, and in case of upgrade, allocate the new channels
	//////////////////////////////////////////////////////////////////////////////////////////////////
	if (numTotalRsrcsAlreadyAllocated > numRsrcsToBeReallocated) //downgrade
	{
		if (l_bIsDialOut == FALSE)
		{
			// Update number of Dial-In reserved ports for party
			int l_currDialInReservedPorts = pParty->GetDialInReservedPorts();
			pParty->SetDialInReservedPorts(l_currDialInReservedPorts - numSuperfluousRsrcs);

			// Update number of Dial-In reserved ports for service
			pNetServicesDB->RemoveDialInReservedPorts((char*)pParam->isdn_span_params.serviceName, numSuperfluousRsrcs);
		}
	}
	else if (numRsrcsToBeReallocated > numTotalRsrcsAlreadyAllocated) //reallocate upgrade
	{
		if (l_bIsDialOut)
		{
			CRsrcDesc** pAdditionalRtmDescArray = NULL;

			PartyDataStruct partyData;
			memset(&partyData, 0, sizeof(partyData));
			partyData.m_networkPartyType = pParam->networkPartyType;
			partyData.m_videoPartyType = pParam->videoPartyType;
			partyData.m_pIsdn_Params_Request = &pParam->isdn_span_params;
			//we will try to allocate where the first RTM channel was allocated
			STATUS status = pSystemResources->GetBestAllocationForRTMOnly(partyData, &pResult->isdnParams, numAdditionalRsrcs, 0, pRTMDescArrayFromSharedMemory[0]->GetBoardId());

			if (status != STATUS_OK)
			{
				pResult->allocIndBase.status = status;
			}
			else //STATUS_OK
			{
				//alocate connectionids + all normal allocations of RTMs
				status = AllocateRTMAllChannels(numAdditionalRsrcs, monitor_conf_id, rsrcPartyId, pResult->isdnParams, pParam->isdn_span_params, pAdditionalRtmDescArray, rsrcConfId, TRUE, pConf);
				if (status != STATUS_OK)
				{
					pResult->allocIndBase.status = status;
				}
				else
				{
					WORD chanell_id_rtm = 0;
					ConnToCardTableEntry Entry;
					Entry.rsrc_conf_id = rsrcConfId;
					Entry.rsrc_party_id = rsrcPartyId;
					std::ostringstream msg;
					ConnToCardEntryDumpHeader(msg, "CRsrcAlloc::ReAllocateRTM - Shared memory", Entry.rsrc_conf_id, Entry.rsrc_party_id, Entry.room_id);

					//add to shared memory
					int numberOfEntries = WriteChannelsToSharedMemoryRTM(pAdditionalRtmDescArray, numAdditionalRsrcs, pConnToCardMngr, chanell_id_rtm, pResult, Entry, msg);
					if (numberOfEntries)
						ConnToCardEntryDumpFooter(msg);
				}
			}

			delete[] pAdditionalRtmDescArray;
		}
		else //dial-in
		{
			// Update number of Dial-In reserved ports for party
			int l_currDialInReservedPorts = pParty->GetDialInReservedPorts();
			pParty->SetDialInReservedPorts(l_currDialInReservedPorts + numAdditionalRsrcs);

			// Update number of Dial-In reserved ports for service
			STATUS status = pNetServicesDB->AddDialInReservedPorts((char*)pParam->isdn_span_params.serviceName, numAdditionalRsrcs);
			if (status != STATUS_OK)
			{
				pResult->allocIndBase.status = status;
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	///Update the art entries in shared memory in case of downgrade to PSTN, or upgrade from PSTN
	////////////////////////////////////////////////////////////////////////////////
	if (eVideo_party_type_none == pParam->videoPartyType && eVideo_party_type_none != oldVideoPartyType)
	{
		//downgrade to PSTN: asign the value of the single RTM entry left to the art entry instead of "magic number" (DUMMY_UDP_PORT_ID)
		ConnToCardTableEntry EntryRTM;
		if (pConnToCardMngr->Get(connIdRTMArray[0], EntryRTM) == STATUS_OK)
		{
			UpdateChannelIdForAudioEntriesInSharedMemory(rsrcPartyId, pConf, EntryRTM.channelId, pConnToCardMngr);
		}
		else
			PASSERTMSG(1, "CRsrcAlloc::ReAllocateRTM : couldn't get entry of RTM from shared memory");
	}
	else if (eVideo_party_type_none == oldVideoPartyType && eVideo_party_type_none != pParam->videoPartyType)
	{
		// upgrade from PSTN: asign the value "magic number" (DUMMY_UDP_PORT_ID) to the art entry instead of of the single RTM entry
		UpdateChannelIdForAudioEntriesInSharedMemory(rsrcPartyId, pConf, DUMMY_UDP_PORT_ID, pConnToCardMngr);
	}

	////////////////////////////////////////////////////////////////////////////
	//Update usage of ART channels in party and in ART unit
	////////////////////////////////////////////////////////////////////////////
	WORD oldNumARTChannels = pParty->GetARTChannels();
	if (numRsrcsToBeReallocated != oldNumARTChannels)
	{
		pParty->SetARTChannels(numRsrcsToBeReallocated);

		CRsrcDesc* pArtDesc = (CRsrcDesc*)(pConf->GetDesc(rsrcPartyId, eLogical_audio_encoder));
		if (pArtDesc == NULL)
		{
			PASSERTMSG(1, "CRsrcAlloc::ReAllocateRTM : couldn't get ART descriptor");
		}
		else
		{
			CUnitMFA* pARTUnitMFA = pSystemResources->GetUnit(pArtDesc->GetBoardId(), pArtDesc->GetUnitId());
			if (pARTUnitMFA == NULL)
			{
				PASSERTMSG(1, "CRsrcAlloc::ReAllocateRTM : couldn't get ART UNIT");
			}
			else
			{
				//tbd zoe: what if there are not enough resources for that on ART unit???
				pARTUnitMFA->DeAllocateARTChannels(oldNumARTChannels);
				pARTUnitMFA->AllocateARTChannels(numRsrcsToBeReallocated);
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////
	//Clean-up
	////////////////////////////////////////////////////////////////////////////
	if (connIdRTMArray)
		delete []connIdRTMArray;
	if (pRTMDescArrayFromSharedMemory)
		delete []pRTMDescArrayFromSharedMemory;
}

////////////////////////////////////////////////////////////////////////////
void CRsrcAlloc::UpdateChannelIdForAudioEntriesInSharedMemory(DWORD rsrcPartyId, CConfRsrc* pConf, WORD rtmChannelId, CConnToCardManager* pConnToCardMngr)
{
	DWORD connIdAudEnc = 0, connIdAudDec = 0;

	CRsrcDesc* pEncDesc = (CRsrcDesc*)(pConf->GetDesc(rsrcPartyId, eLogical_audio_encoder));
	if (pEncDesc)
		connIdAudEnc = pEncDesc->GetConnId();

	ConnToCardTableEntry EntryAudEnc;
	if (pConnToCardMngr->Get(connIdAudEnc, EntryAudEnc) == STATUS_OK)
	{
		EntryAudEnc.channelId = rtmChannelId;
		PASSERT(pConnToCardMngr->Update(EntryAudEnc));
	}

	CRsrcDesc* pDecDesc = (CRsrcDesc*)(pConf->GetDesc(rsrcPartyId, eLogical_audio_decoder));
	if (pDecDesc)
		connIdAudDec = pDecDesc->GetConnId();

	ConnToCardTableEntry EntryAudDec;
	if (pConnToCardMngr->Get(connIdAudDec, EntryAudDec) == STATUS_OK)
	{
		EntryAudDec.channelId = rtmChannelId;
		PASSERT(pConnToCardMngr->Update(EntryAudDec));
	}
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrcAlloc::AllocateBondingTemporaryNumber(const char* serviceName, DWORD rsrcConfId, DWORD rsrcPartyId, Phone* tempBondingPhoneAllocated, const char* pSimilarToThisString)
{
	STATUS status = STATUS_OK;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();

	CConfRsrc* confRsrc = (NULL != pConfRsrcDB) ? (CConfRsrc*)pConfRsrcDB->GetConfRsrcByRsrcConfId(rsrcConfId) : NULL;
	CServicePhoneStr* pServicePhoneStr = new CServicePhoneStr();
	pServicePhoneStr->SetNetServiceName(serviceName);

	CReservator* pReservator = CHelperFuncs::GetReservator();
	if (pReservator && pReservator->IsInternalReservator())
		status = pReservator->AllocateBondingTemporaryNumber_WithSchedule(pServicePhoneStr, pSimilarToThisString);
	else if (pSystemResources)
		status = pSystemResources->AllocateServicePhones(*pServicePhoneStr, pSimilarToThisString);
	else
		status = STATUS_FAIL;

	if (status == STATUS_OK)
	{
		Phone* phoneAllocated = pServicePhoneStr->GetFirstPhoneNumber();
		if (STATUS_OK == status && NULL != phoneAllocated)
		{
			strncpy(tempBondingPhoneAllocated->phone_number, phoneAllocated->phone_number, PHONE_NUMBER_DIGITS_LEN);

			if (NULL != confRsrc)
			{
				Phone* phone = confRsrc->GetTempBondingPhoneNumberByRsrcPartyId(rsrcPartyId);
				if (phone && tempBondingPhoneAllocated)
				{
					if (!strcmp(phone->phone_number, tempBondingPhoneAllocated->phone_number)) // Check DB self-consistencey
					{
						DBGPASSERT(rsrcPartyId);
						TRACEINTO << "\nTemporary bonding number = " << tempBondingPhoneAllocated->phone_number << "to allocate already exists in internal DB for\n" << "rsrcConfId = " << rsrcConfId << " rsrcPartyId = " << rsrcPartyId << "\n";
					}
				}

				confRsrc->AddTempBondingPhoneNumber(rsrcPartyId, tempBondingPhoneAllocated);
			}
		}
		else
			DBGPASSERT(1);
	}

	POBJDELETE(pServicePhoneStr);

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrcAlloc::DeAllocateBondingTemporaryNumber(const char* serviceName, DWORD monitorConfId, DWORD monitorPartyId, const char* tempBondingPhoneToDeAllocate)
{
	STATUS status = STATUS_OK;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	TRACECOND_AND_RETURN_VALUE(!pSystemResources, "pSystemResources is NULL", STATUS_FAIL);

	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	TRACECOND_AND_RETURN_VALUE(!pConfRsrcDB, "pConfRsrcDB is NULL", STATUS_FAIL);

	CConfRsrc* confRsrc = (CConfRsrc*)pConfRsrcDB->GetConfRsrc(monitorConfId);
	TRACECOND_AND_RETURN_VALUE(!confRsrc, "confRsrc is NULL", STATUS_FAIL);

	DWORD rsrcPartyId = pConfRsrcDB->MonitorToRsrcPartyId(monitorConfId, monitorPartyId);

	CServicePhoneStr* pServicePhoneStr = new CServicePhoneStr();
	pServicePhoneStr->SetNetServiceName(serviceName);
	pServicePhoneStr->AddPhoneNumber(tempBondingPhoneToDeAllocate);

	status = pSystemResources->DeAllocateServicePhones(*pServicePhoneStr);

	if (STATUS_OK == status && NULL != confRsrc)
	{
		Phone* phone = confRsrc->GetTempBondingPhoneNumberByRsrcPartyId(rsrcPartyId);
		if (phone)
		{
			if (strcmp(phone->phone_number, tempBondingPhoneToDeAllocate)) // Check DB self-consistencey
				TRACEINTO << "\nCRsrcAlloc::DeAllocateBondingTemporaryNumber : Couldn't find temporary bonding number = " << tempBondingPhoneToDeAllocate << " to deallocate in internal DB for" << "monitorConfId = " << monitorConfId << " rsrcPartyId = " << rsrcPartyId << "\n";
			else
				TRACEINTO << "\nCRsrcAlloc::DeAllocateBondingTemporaryNumber : Found temporary bonding number = " << tempBondingPhoneToDeAllocate << " to deallocate in internal DB for" << "monitorConfId = " << monitorConfId << " rsrcPartyId = " << rsrcPartyId << "\n";
		}

		confRsrc->RemoveTempBondingPhoneNumber(rsrcPartyId);
	}

	POBJDELETE(pServicePhoneStr);

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrcAlloc::AllocateConf2C(DWORD monitor_conf_id, CONF_RSRC_IND_PARAMS_S* pResult)
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN_VALUE(pSystemResources == NULL, STATUS_FAIL);

	if (FALSE == pSystemResources->IsThereExistUtilizableUnitForAudioController())
	{
		TRACEINTO << "\nCRsrcAlloc::AllocateConf2C failed. No utilizable unit for Audio Controller \n\n";
		pResult->status = STATUS_NO_UTILIZABLE_UNIT_FOR_AUDIO_CONTROLLER;
		return STATUS_NO_UTILIZABLE_UNIT_FOR_AUDIO_CONTROLLER;
	}

	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	CConfRsrc* pConf = (CConfRsrc*)(pConfRsrcDB ? pConfRsrcDB->GetConfRsrc(monitor_conf_id) : NULL);

	BestAllocStruct bestAlloction1, bestAlloction2;
	memset(&bestAlloction1, 0, sizeof(bestAlloction1));
	memset(&bestAlloction2, 0, sizeof(bestAlloction2));

	STATUS status = pConf ? STATUS_OK : STATUS_FAIL;
	eSessionType sessionType = pConf ? pConf->GetSessionType() : esession_type_none;
	if (STATUS_OK == status)
	{
		if (eCOP_HD1080_session == sessionType || eCOP_HD720_50_session == sessionType)
			status = pSystemResources->GetBestBoardsCOP(sessionType, pConf->GetLogicalEncoderTypeCOP(), bestAlloction1);

		else if (eVSW_28_session == sessionType || eVSW_56_session == sessionType)
			status = pSystemResources->GetBestBoardsVSW(sessionType, bestAlloction1, bestAlloction2);
	}

	if (status != STATUS_OK)
	{
		TRACEINTO << "CRsrcAlloc::AllocateConf2C : sessionType = " << eSessionTypeNames[sessionType] << " => GetBestBoards() returns status = " << status;
		pResult->status = STATUS_FAIL;
		return status;
	}

	CRsrcDesc** pVideoDescArray = NULL;
	CRsrcDesc** pVideoDescArray2 = NULL;

	status = AllocateVideo2C(monitor_conf_id, bestAlloction1.m_VideoAlloc, pVideoDescArray);

	if (STATUS_OK == status && eVSW_56_session == sessionType)
	{
		if (bestAlloction2.m_VideoAlloc.m_boardId != 0)
			status = AllocateVideo2C(monitor_conf_id, bestAlloction2.m_VideoAlloc, pVideoDescArray2); //Olga
		else
		{
			CBoard* pBoard = pSystemResources->GetBoard(bestAlloction1.m_VideoAlloc.m_boardId);
			if (pBoard)
				pBoard->AddOneCOPConfOnThisBoard(); //need to call twice because we count 56 like 2x28 conferences
			else
				PTRACE(eLevelError, "CRsrcAlloc::AllocateConf2C - pBoard is NULL");
		}
	}

	WORD reqBoardId = 0;
	STATUS write_stat = STATUS_OK;
	DWORD num_cif_decoders = 0, num_predefined_rsrcs = 0, i = 0;
	WORD total_max_video_rsrc = pSystemResources->GetTotalMaxVideoResources();
	int numberOfEntries = 0;
	CConnToCardManager* pConnToCardMngr = NULL;
	ConnToCardTableEntry Entry;
	DWORD rsrcConfId = 0;
	std::ostringstream msg;

	if (status != STATUS_OK)
	{
		TRACEINTO << "CRsrcAlloc::AllocateConf2C => AllocateVideo2C() returns status = " << status;
		pResult->status = STATUS_FAIL;
		goto CleanUP;
	}

	pConnToCardMngr = CHelperFuncs::GetConnToCardManager();
	if (!pConnToCardMngr)
	{
		PASSERT(1); //error handling;
	}

	rsrcConfId = (pConf) ? pConf->GetRsrcConfId() : 0;
	if (!rsrcConfId)
	{
		pResult->status = STATUS_FAIL;
		goto CleanUP;
	}

	Entry.rsrc_conf_id = rsrcConfId;

	ConnToCardEntryDumpHeader(msg, "CRsrcAlloc::AllocateConf2C - Shared memory", Entry.rsrc_conf_id, Entry.rsrc_party_id, Entry.room_id);

	if (pVideoDescArray != NULL && (eCOP_HD1080_session == sessionType || eCOP_HD720_50_session == sessionType))
	{
		for (i = 0; i < total_max_video_rsrc; i++)
		{
			if (pVideoDescArray[i] != NULL)
			{
				reqBoardId = pVideoDescArray[i]->GetBoardId();
				if (pConnToCardMngr)
				{
					Entry.m_id = pVideoDescArray[i]->GetConnId();
					Entry.rsrcType = pVideoDescArray[i]->GetType();
					Entry.physicalRsrcType = pVideoDescArray[i]->GetPhysicalType();
					Entry.rsrcCntlType = pVideoDescArray[i]->GetCntrlType();
					Entry.boxId = pVideoDescArray[i]->GetBoxId();
					Entry.boardId = pVideoDescArray[i]->GetBoardId();
					Entry.subBoardId = pVideoDescArray[i]->GetSubBoardId();
					Entry.unitId = pVideoDescArray[i]->GetUnitId();
					Entry.acceleratorId = pVideoDescArray[i]->GetAcceleratorId();
					Entry.portId = pVideoDescArray[i]->GetFirstPortId();
					Entry.channelId = 0;
					Entry.rsrc_party_id = pVideoDescArray[i]->GetRsrcPartyId();

					write_stat = AddEntryToSharedMemory(Entry); // pConnToCardMngr->Add(Entry);
					numberOfEntries += ConnToCardEntryDump(msg, Entry);

					if (write_stat == STATUS_OK)                                                // succeeded to write in conn-to-card table
					{
						if (pVideoDescArray[i]->GetCntrlType() == E_NORMAL ||                     // ConfParty requirs NOT TO ADD slaves to
						    CHelperFuncs::IsMasterType(pVideoDescArray[i]->GetCntrlType()))       // resources returned to ConfParty
						{
							if (Entry.rsrcType != eLogical_COP_Dynamic_decoder)
							{
								if (num_predefined_rsrcs < MAX_NUM_ALLOCATED_RSRCS)
								{
									pResult->allocatedRrcs[num_predefined_rsrcs].connectionId = Entry.m_id;
									pResult->allocatedRrcs[num_predefined_rsrcs].logicalRsrcType = Entry.rsrcType;
									pResult->allocatedRrcs[num_predefined_rsrcs].rsrcEntityId = Entry.rsrc_party_id;
									num_predefined_rsrcs++;
								}
								else
									PASSERTMSG(1, "num_predefined_rsrcs exceed MAX_NUM_ALLOCATED_RSRCS");
							}
							else
							{
								if (num_cif_decoders < MAX_NUM_COP_DYNAMIC_RSRCS)
								{
									pResult->dec_conn_id_list[num_cif_decoders].connectionId = Entry.m_id;
									pResult->dec_conn_id_list[num_cif_decoders].logicalRsrcType = Entry.rsrcType;
									pResult->dec_conn_id_list[num_cif_decoders].rsrcEntityId = Entry.rsrc_party_id;
									num_cif_decoders++;
								}
								else
								{
									PASSERTMSG(1, "num_cif_decoders exceed MAX_NUM_COP_DYNAMIC_RSRCS");
								}
							}
						}
					}
				}
			}
		}
	}

	// *** fill in Result for output

	pResult->status = status; // to check if write_stat isn't OK
	pResult->rsrc_conf_id = rsrcConfId;
	if (eCOP_HD1080_session == sessionType || eCOP_HD720_50_session == sessionType)
	{
		if (status != STATUS_OK)
		{
			pResult->num_predefinedRsrcs = 0;
			if (num_predefined_rsrcs < MAX_NUM_ALLOCATED_RSRCS)
			{
				for (i = 0; i < num_predefined_rsrcs; i++)
				{
					pResult->allocatedRrcs[num_predefined_rsrcs].connectionId = 0;
					pResult->allocatedRrcs[num_predefined_rsrcs].logicalRsrcType = eLogical_res_none;
					pResult->allocatedRrcs[num_predefined_rsrcs].rsrcEntityId = 0;
				}
			}
			else
				PASSERTMSG(1, "num_predefined_rsrcs exceed MAX_NUM_ALLOCATED_RSRCS");

			if (num_cif_decoders < MAX_NUM_COP_DYNAMIC_RSRCS)
			{
				for (i = 0; i < num_cif_decoders; i++)
				{
					pResult->dec_conn_id_list[num_cif_decoders].connectionId = 0;
					pResult->dec_conn_id_list[num_cif_decoders].logicalRsrcType = eLogical_res_none;
					pResult->dec_conn_id_list[num_cif_decoders].rsrcEntityId = 0;
				}
			}
			else
				PASSERTMSG(1, "num_cif_decoders exceed MAX_NUM_COP_DYNAMIC_RSRCS");

		}
		else
		{
			// *** PCM manager allocations for COP conf only
			CBoard* pBoard = pSystemResources->GetBoard(reqBoardId);
			if (pBoard)
			{
				DWORD connIdPcmMenu = pSystemResources->AllocateConnId();
				DWORD pcmMenuId = pBoard->AllocatePcmMenuId(connIdPcmMenu);

				if (0 == connIdPcmMenu || DUMMY_CONNECTION_ID == pcmMenuId)
				{
					pSystemResources->DeAllocateConnId(connIdPcmMenu);
					PASSERT(1);
				}
				else
				{
					CRsrcDesc* pPcmMenuDesc = new CRsrcDesc(connIdPcmMenu, eLogical_PCM_manager, rsrcConfId, (WORD)DUMMY_PARTY_ID);
					PASSERT(pConf->AddDesc(pPcmMenuDesc));

					if (pConnToCardMngr) // save to shared memory
					{
						ConnToCardTableEntry Entry;

						Entry.rsrc_conf_id = rsrcConfId;
						Entry.rsrc_party_id = DUMMY_PARTY_ID;
						Entry.m_id = connIdPcmMenu;
						Entry.rsrcType = eLogical_PCM_manager;
						Entry.physicalRsrcType = ePhysical_res_none;
						Entry.boardId = reqBoardId;
						Entry.subBoardId = 1;
						Entry.boxId = 1;
						Entry.unitId = 0;

						write_stat = AddEntryToSharedMemory(Entry); // pConnToCardMngr->Add(Entry);
						numberOfEntries += ConnToCardEntryDump(msg, Entry);

						PASSERT(write_stat != STATUS_OK); //if succeeded to write in conn-to-card table
					}
					else
						PASSERT(100);

					POBJDELETE(pPcmMenuDesc);
				}

				if (num_predefined_rsrcs < MAX_NUM_ALLOCATED_RSRCS)
				{
					pResult->allocatedRrcs[num_predefined_rsrcs].connectionId = connIdPcmMenu;
					pResult->allocatedRrcs[num_predefined_rsrcs].logicalRsrcType = eLogical_PCM_manager;
					pResult->allocatedRrcs[num_predefined_rsrcs].rsrcEntityId = DUMMY_PARTY_ID;
					pResult->pcmMenuId = pcmMenuId;
					num_predefined_rsrcs++;
				}
				else
					PASSERTMSG(1, "num_predefined_rsrcs exceed MAX_NUM_ALLOCATED_RSRCS");
			}

			// *** set predefined resources number
			pResult->num_predefinedRsrcs = num_predefined_rsrcs;
		}
	}

	if (numberOfEntries)
		ConnToCardEntryDumpFooter(msg);

	CleanUP: if (pVideoDescArray != NULL)
	{
		for (int i = 0; i < total_max_video_rsrc; i++)
			PDELETE(pVideoDescArray[i]);

		delete[] pVideoDescArray;
		pVideoDescArray = NULL;
	}

	if (pVideoDescArray2 != NULL)
	{
		for (int i = 0; i < total_max_video_rsrc; i++)
			PDELETE(pVideoDescArray2[i]);

		delete[] pVideoDescArray2;
		pVideoDescArray2 = NULL;
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrcAlloc::AllocateVideo2C(DWORD monitor_conf_id, AllocData& videoAlloc, CRsrcDesc**& pVideoDescArray)
{
	//The macro is use to generate PartyId for encoder/decoder entities.
	//12 bits are reserved for real PartyId that has range 1...4096 (MAX_RSRC_PARTY_ID).
	//Since confId range: 1...1023 (10 bits) and entity index 0...31 (5 bits) with this mask we can get unique EntityId.
#define MAKE_ENTITY_ID(confId, index) (((((DWORD)(index) << 10) & 0x7c00) | ((DWORD)(confId) & 0x3ff)) | 0x8000)

	TRACEINTO << "monitor_conf_id:" << monitor_conf_id;

	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	PASSERT_AND_RETURN_VALUE(pConfRsrcDB == NULL, STATUS_FAIL);

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN_VALUE(pSystemResources == NULL, STATUS_FAIL);

	CConfRsrc* pConf = (CConfRsrc*)(pConfRsrcDB->GetConfRsrc(monitor_conf_id));
	PASSERT_AND_RETURN_VALUE(pConf == NULL, STATUS_CONFERENCE_NOT_EXISTS);

	DWORD rsrcConfId = pConf->GetRsrcConfId();
	eSessionType sessionType = pConf->GetSessionType();

	DWORD connId, rsrcMasterEntityId = DUMMY_PARTY_ID, rsrcVideoDecoderEntityId = DUMMY_PARTY_ID, rsrcEntityId = DUMMY_PARTY_ID;
	WORD max_units_video = CHelperFuncs::GetMaxUnitsNeededForVideo();
	WORD max_media_ports = CHelperFuncs::GetMaxRequiredPortsPerMediaUnit();
	DWORD index = 1;
	for (int i = 0; i < max_units_video; i++)
	{
		for (int j = 0; j < max_media_ports; j++)
		{
			if (videoAlloc.m_unitsList[i].m_MediaPortsList[j].m_type != eLogical_res_none)
			{
				connId = pSystemResources->AllocateConnId();

				if (eCOP_HD1080_session == sessionType || eCOP_HD720_50_session == sessionType) // allocate rsrcEntityId for COP conf
				{
					if (!CHelperFuncs::IsSlaveType(videoAlloc.m_unitsList[i].m_MediaPortsList[j].m_cntrl_type))
					{
						rsrcEntityId = MAKE_ENTITY_ID(rsrcConfId, index);
#ifdef LOOKUP_TABLE_DEBUG_TRACE
						TRACEINTO << "D.K.1 ConfId:" << rsrcConfId << ", ConnId:" << connId << ", entityId:" << rsrcEntityId << "(" << i << "," << j << ")," << index;
#endif
						index++;
						if (CHelperFuncs::IsMasterType(videoAlloc.m_unitsList[i].m_MediaPortsList[j].m_cntrl_type))
							rsrcMasterEntityId = rsrcEntityId;
					}
					else
					{
						if (DUMMY_PARTY_ID == rsrcMasterEntityId)
							PASSERT(1);
						else
							rsrcEntityId = rsrcMasterEntityId;
					}
				}
				else if (eVSW_56_session == sessionType || eVSW_28_session == sessionType) // encoder & decoder should have same rsrcEntityId for NxM conf
				{
					if (videoAlloc.m_unitsList[i].m_MediaPortsList[j].m_type == eLogical_video_decoder)
					{
						rsrcEntityId = MAKE_ENTITY_ID(rsrcConfId, index);
#ifdef LOOKUP_TABLE_DEBUG_TRACE
						TRACEINTO << "D.K.2 ConfId:" << rsrcConfId << ", ConnId:" << connId << ", entityId:" << rsrcEntityId << "(" << i << "," << j << ")," << index;
#endif
						index++;
						rsrcVideoDecoderEntityId = rsrcEntityId;
					}
					else if (videoAlloc.m_unitsList[i].m_MediaPortsList[j].m_type == eLogical_video_encoder)
					{
						rsrcEntityId = rsrcVideoDecoderEntityId;
					}
				}

				if ((connId == 0) || (rsrcEntityId == 0))
				{
					// rollback
					for (int k = 0; k <= i; k++)
					{
						for (int l = 0; ((k < i) && (l < max_media_ports)) || ((k == i) && (l < j)); l++)
						{
							if (videoAlloc.m_unitsList[k].m_MediaPortsList[l].m_type != eLogical_res_none)
							{
								pSystemResources->DeAllocateConnId(videoAlloc.m_unitsList[k].m_MediaPortsList[l].m_connId);
							}
						}
					}

					// Deallocate connId or/and rsrcEntityId on the current unit/media port if allocated
					if (connId != 0)
						pSystemResources->DeAllocateConnId(connId);

					PASSERT(1);
					return STATUS_INSUFFICIENT_RSRC_CONNECTION_ID;
				}
				else
				{
					videoAlloc.m_unitsList[i].m_MediaPortsList[j].m_connId = connId;
					videoAlloc.m_unitsList[i].m_MediaPortsList[j].m_rsrcEntityId = rsrcEntityId;
				}
			}
		}
	}

	STATUS status = pSystemResources->AllocateVideo2C(rsrcConfId, videoAlloc);

	// *** get status and rollback if needed
	if (status != STATUS_OK)
	{
		WORD max_units_video = CHelperFuncs::GetMaxUnitsNeededForVideo();
		WORD max_media_ports = CHelperFuncs::GetMaxRequiredPortsPerMediaUnit();
		for (int i = 0; i < max_units_video; i++)
		{
			for (int j = 0; j < max_media_ports; j++)
			{
				if (videoAlloc.m_unitsList[i].m_MediaPortsList[j].m_type != eLogical_res_none)
				{
					pSystemResources->DeAllocateConnId(videoAlloc.m_unitsList[i].m_MediaPortsList[j].m_connId);
				}
			}
		}

		PTRACE2INT(eLevelInfoNormal, "CRsrcAlloc::AllocateVideo2C failed - status", status);
		return status;
	}

	// *** create (and insert) descriptors
	const WORD total_max_video_rsrc = pSystemResources->GetTotalMaxVideoResources();
	CRsrcDesc* pRsrcDescriptor;
	pVideoDescArray = new CRsrcDesc*[total_max_video_rsrc];
	for (int i = 0; i < total_max_video_rsrc; i++)
		pVideoDescArray[i] = NULL;

	int descIndex = 0;

	for (int i = 0; i < max_units_video; i++)
	{
		for (int j = 0; j < max_media_ports; j++)
		{
			if (videoAlloc.m_unitsList[i].m_MediaPortsList[j].m_type != eLogical_res_none)
			{
				pRsrcDescriptor = new CRsrcDesc(videoAlloc.m_unitsList[i].m_MediaPortsList[j].m_connId, videoAlloc.m_unitsList[i].m_MediaPortsList[j].m_type, rsrcConfId, videoAlloc.m_unitsList[i].m_MediaPortsList[j].m_rsrcEntityId/*rsrcPartyId*/, 1 /*boxid*/, videoAlloc.m_boardId, 1 /*subboard*/, videoAlloc.m_unitsList[i].m_UnitId, videoAlloc.m_unitsList[i].m_MediaPortsList[j].m_acceleratorId, videoAlloc.m_unitsList[i].m_MediaPortsList[j].m_portId, videoAlloc.m_unitsList[i].m_MediaPortsList[j].m_cntrl_type);

				pVideoDescArray[descIndex] = pRsrcDescriptor;
				descIndex++;

				PASSERT(pConf->AddDesc(pRsrcDescriptor));
			}
		}
	}

	CBoard* pBoard = pSystemResources->GetBoard(videoAlloc.m_boardId);
	if (pBoard)
		pBoard->AddOneCOPConfOnThisBoard();   // Olga - check with Sergey

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
void CRsrcAlloc::AllocateParty2C(ALLOC_PARTY_REQ_PARAMS_S* pParam, ALLOC_PARTY_IND_PARAMS_S* pResult)
{
	TRACEINTO << "\nCRsrcAlloc::AllocateParty2C  Details: monitor_conf_id = " << pParam->monitor_conf_id << ", monitor_party_id = " << pParam->monitor_party_id << "\n";
	//*********************

	//*** check parameters for validity (pParam)
	//allocate conf rsrc id and master audio cntler.
	//***create party. allocate rsrc party id.
	//allocate needed ports (ART and UDP(as in video case).
	//***write allocation results to Conn-To-Cards table
	//***fill in returned struct (pResult)

	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	CReservator* pReservator = CHelperFuncs::GetReservator();

	if (pSystemResources == NULL || pConfRsrcDB == NULL || pReservator == NULL)
	{
		pResult->allocIndBase.status = STATUS_FAIL;
		TRACEINTO << "\nCRsrcAlloc::AllocateParty2C failed. pSystemResources or pConfRsrcDB or pReservator is NULL \n";
		return;
	}

	if (FALSE == pSystemResources->IsThereExistUtilizableUnitForAudioController())
	{
		TRACEINTO << "\nCRsrcAlloc::AllocateParty2C failed. No utilizable unit for Audio Controller \n\n";
		pResult->allocIndBase.status = STATUS_NO_UTILIZABLE_UNIT_FOR_AUDIO_CONTROLLER;
		return;
	}

	DWORD monitor_conf_id = pParam->monitor_conf_id;
	DWORD monitor_party_id = pParam->monitor_party_id;
	eVideoPartyType reqVideoPartyType = pParam->videoPartyType;
	eVideoPartyType videoPartyType = reqVideoPartyType;
	eNetworkPartyType networkPartyType = pParam->networkPartyType;
	eSessionType sessionType = pParam->sessionType;
	WORD serviceId = pParam->serviceId;
	WORD subServiceId = pParam->subServiceId;
	WORD room_id = pParam->room_id;

	WORD numRsrc = 0;
	BOOL bIsIPParty = CHelperFuncs::IsIPParty(networkPartyType);

	DWORD rsrcConfId = 0;
	PartyRsrcID rsrcPartyId = pParam->party_id;
	DWORD connIdAudEnc = 0, connIdAudDec = 0, connIdRTPOrMUX = 0, partySCconnId = 0;

	CRsrcDesc* pAudEncDesc = NULL;
	CRsrcDesc* pAudDecDesc = NULL;
	CRsrcDesc* pRtpOrMuxDesc = NULL;
	CUdpRsrcDesc* pUdpDesc = NULL;

	STATUS status = STATUS_OK;
	WORD bestArtBoardId = 0xFFFF;
	DWORD artIpAddr = 0;

	ResourcesInterface* pResourcesInterface = pSystemResources->GetCurrentResourcesInterface();

	// should be only flexible
	if (pResourcesInterface == NULL)
	{
		pResult->allocIndBase.status = STATUS_FAIL;
		PASSERT(1);
		return;
	}

	if (pResourcesInterface->CheckIfOneMorePartyCanBeAddedCOP(pResult) == FALSE)
	{
		//filling of the pResult and trace are done in function
		pResult->allocIndBase.status = STATUS_FAIL;
		TRACEINTO << " CRsrcAlloc::AllocateParty2C failed. Party can't be added to COP/NxM conf";
		return;
	}

	CConfRsrvRsrc* pConf = (CConfRsrvRsrc*)(pReservator->GetConfRsrvRsrcById(monitor_conf_id));

	if (pConfRsrcDB->IsExitingConf(monitor_conf_id) == FALSE) // while spreading it has to be done, excluded EQ
	{
		pResult->allocIndBase.status = STATUS_FAIL;
		PASSERT(1);
		return;
	}
	else
		rsrcConfId = pConfRsrcDB->MonitorToRsrcConfId(monitor_conf_id);	// gets existing rsrcConfId from DB

	CConfRsrc* pConfRsrc = (CConfRsrc*)(pConfRsrcDB->GetConfRsrc(monitor_conf_id));
	PASSERT_AND_RETURN(pConfRsrc == NULL);

	if (pConfRsrc->CheckIfOneMorePartyCanBeAddedToConf2C(videoPartyType) == FALSE)
	{
		pResult->allocIndBase.status = STATUS_FAIL;
		TRACEINTO << " CRsrcAlloc::AllocateParty2C failed. Party can't be added to COP/NxM conf";
		return;
	}
	int actualNumberOfRTMPortsToAllocate = 0;
	PartyDataStruct partyData;
	memset(&partyData, 0, sizeof(partyData));
	partyData.m_videoPartyType = videoPartyType;
	partyData.m_partyRole = pParam->partyRole;
	partyData.m_networkPartyType = networkPartyType;
	partyData.m_allowReconfiguration = FALSE; //pParam->isWaitForRsrcAndAskAgain;
	partyData.m_monitor_conf_id = monitor_conf_id;
	partyData.m_subServiceId = subServiceId;
	partyData.m_room_id = room_id;
	partyData.m_confMediaType = pParam->confMediaType;
	partyData.m_HdVswTypeInMixAvcSvcMode = pParam->HdVswTypeInMixAvcSvcMode;

	if (CHelperFuncs::IsISDNParty(networkPartyType))
	{
		partyData.m_pIsdn_Params_Request = &pParam->isdn_span_params;
		TRACEINTO << "\nCRsrcAlloc::AllocateParty2C failed. Network Party Type is ISDN which isn't supported in RMX2000C !!! ";
	}
	BestAllocStruct bestAlloction;
	memset(&bestAlloction, 0, sizeof(bestAlloction));

	CRsrcDesc* pEncDesc = NULL, *pDecDesc = NULL;

	if (status == STATUS_OK) // creates PartyRsrc object (in ConfRsrc), allocates rsrcPartyId
	{
		if (sessionType == eCOP_HD1080_session || sessionType == eCOP_HD720_50_session) //eCOP_party_type == videoPartyType
		{
			status = CreateAllocPartyRsrc(pSystemResources, pConfRsrcDB, monitor_conf_id, monitor_party_id, rsrcPartyId, partySCconnId, partyData, eTipNone, &room_id);
		}
		else if (sessionType == eVSW_28_session || sessionType == eVSW_56_session)
		{
			status = CreateAllocPartyVSW(pSystemResources, pConfRsrcDB, monitor_conf_id, monitor_party_id, rsrcPartyId, partySCconnId, partyData, pEncDesc, pDecDesc, &room_id);
			if (pEncDesc)
			{
				pResult->allocIndBase.allocatedRrcs[numRsrc].connectionId = pEncDesc->GetConnId();
				pResult->allocIndBase.allocatedRrcs[numRsrc].logicalRsrcType = eLogical_video_encoder;
				numRsrc++;
			}
			if (pDecDesc)
			{
				pResult->allocIndBase.allocatedRrcs[numRsrc].connectionId = pDecDesc->GetConnId();
				pResult->allocIndBase.allocatedRrcs[numRsrc].logicalRsrcType = eLogical_video_decoder;
				numRsrc++;
			}
		}
		else
		{
			pResult->allocIndBase.status = STATUS_FAIL;
			TRACEINTO << " CRsrcAlloc::AllocateParty2C failed. Party of type = " << eVideoPartyTypeNames[videoPartyType] << " can't be added to " << eSessionTypeNames[sessionType] << " conference";
			return;
		}

		TRACEINTO << " CRsrcAlloc::AllocateParty2C  :  for monitor_party_id=" << monitor_party_id << " was allocated the rsrcPartyId=" << rsrcPartyId << " and room_id=" << room_id;
	}

	/*conn-to-card write*/
	CConnToCardManager* pConnToCardMngr = CHelperFuncs::GetConnToCardManager();
	if (!pConnToCardMngr)
	{
		PASSERT(1);  //error handling;
	}
	//get board id of the allocated conf video resources
	if (sessionType == eCOP_HD1080_session || sessionType == eCOP_HD720_50_session)  //eCOP_party_type == videoPartyType
		pEncDesc = (CRsrcDesc*)pConfRsrc->GetDescByType(eLogical_COP_HD720_encoder);
	else if (!pEncDesc)
		pEncDesc = (CRsrcDesc*)pConfRsrc->GetDescByType(eLogical_video_encoder);

	PASSERT_AND_RETURN(pEncDesc == NULL);
	WORD videoRsrcBoardId = pEncDesc->GetBoardId();
	if (status == STATUS_OK)
		status = pSystemResources->GetBestARTBoardCOP(partyData, bestAlloction, videoRsrcBoardId);

	bestArtBoardId = bestAlloction.m_ArtBoardId;
	if (status == STATUS_OK)
	{
		status = AllocateART(monitor_conf_id /*rsrcConfId*/,
                             rsrcPartyId,
                             pParam->artCapacity,
                             serviceId,
                             subServiceId,
                             sessionType,
                             partyData,
                             pAudEncDesc,
                             pAudDecDesc,
                             pRtpOrMuxDesc,
                             pUdpDesc,
                             pParam->isIceParty,//fix!
                             pParam->isBFCP,
                             bestArtBoardId);
	}

	if (status != STATUS_OK) //*** rollback
	{
		TRACEINTO << "\nCRsrcAlloc::AllocateParty2C failed . rollback now\n\n";
		DestroyDeAllocPartyRsrc(pSystemResources, pConfRsrcDB, monitor_conf_id, monitor_party_id);
	}

	pResult->allocIndBase.status = status;
	if (status != STATUS_OK)
	{
		POBJDELETE(pAudEncDesc);
		POBJDELETE(pAudDecDesc);
		POBJDELETE(pRtpOrMuxDesc);
		POBJDELETE(pUdpDesc);
		return;
	}

	//
	//***temp., should be inside RsrcDesc
	eResourceTypes ARTphysType = ePhysical_art;

	ConnToCardTableEntry Entry;

	connIdAudEnc = (pAudEncDesc) ? pAudEncDesc->GetConnId() : 0;
	connIdAudDec = (pAudDecDesc) ? pAudDecDesc->GetConnId() : 0;
	connIdRTPOrMUX = (pRtpOrMuxDesc) ? pRtpOrMuxDesc->GetConnId() : 0;

	Entry.rsrc_conf_id = rsrcConfId;
	Entry.rsrc_party_id = rsrcPartyId;
	Entry.room_id = room_id;

	int numberOfEntries = 0;
	std::ostringstream msg;
	ConnToCardEntryDumpHeader(msg, "CRsrcAlloc::AllocateParty2C - Shared memory", Entry.rsrc_conf_id, Entry.rsrc_party_id, Entry.room_id);

	STATUS write_stat = STATUS_OK;

	/*conn-to-card write*/
	if (connIdAudEnc)
	{
		if (pConnToCardMngr)
		{
			Entry.m_id = connIdAudEnc;
			Entry.rsrcType = eLogical_audio_encoder;
			Entry.physicalRsrcType = ARTphysType;
			Entry.boxId = pAudEncDesc->GetBoxId();
			Entry.boardId = pAudEncDesc->GetBoardId();
			Entry.subBoardId = pAudEncDesc->GetSubBoardId();
			Entry.unitId = pAudEncDesc->GetUnitId();
			Entry.acceleratorId = pAudEncDesc->GetAcceleratorId();
			Entry.portId = pAudEncDesc->GetFirstPortId();

			write_stat = AddEntryToSharedMemory(Entry); // pConnToCardMngr->Add(Entry);
			numberOfEntries += ConnToCardEntryDump(msg, Entry);
		}

		if (write_stat == STATUS_OK)     // succeeded to write in conn-to-card table
		{
			pResult->allocIndBase.allocatedRrcs[numRsrc].connectionId = connIdAudEnc;
			pResult->allocIndBase.allocatedRrcs[numRsrc].logicalRsrcType = eLogical_audio_encoder;
			numRsrc++;
		}
	}

	if (connIdAudDec)
	{
		if (pConnToCardMngr)
		{
			Entry.m_id = connIdAudDec;
			Entry.rsrcType = eLogical_audio_decoder;
			Entry.physicalRsrcType = ARTphysType;
			Entry.boxId = pAudDecDesc->GetBoxId();
			Entry.boardId = pAudDecDesc->GetBoardId();
			Entry.subBoardId = pAudDecDesc->GetSubBoardId();
			Entry.unitId = pAudDecDesc->GetUnitId();
			Entry.acceleratorId = pAudDecDesc->GetAcceleratorId();
			Entry.portId = pAudDecDesc->GetFirstPortId();

			write_stat = AddEntryToSharedMemory(Entry); // pConnToCardMngr->Add(Entry);
			numberOfEntries += ConnToCardEntryDump(msg, Entry);
		}

		if (write_stat == STATUS_OK)     // succeeded to write in conn-to-card table
		{
			pResult->allocIndBase.allocatedRrcs[numRsrc].connectionId = connIdAudDec;
			pResult->allocIndBase.allocatedRrcs[numRsrc].logicalRsrcType = eLogical_audio_decoder;
			numRsrc++;
		}
	}

	if (connIdRTPOrMUX)
	{
		if (pConnToCardMngr)
		{
			Entry.m_id = connIdRTPOrMUX;
			Entry.rsrcType = eLogical_rtp;
			Entry.physicalRsrcType = ARTphysType;
			Entry.boxId = pRtpOrMuxDesc->GetBoxId();
			Entry.boardId = pRtpOrMuxDesc->GetBoardId();
			Entry.subBoardId = pRtpOrMuxDesc->GetSubBoardId();
			Entry.unitId = pRtpOrMuxDesc->GetUnitId();
			Entry.acceleratorId = pRtpOrMuxDesc->GetAcceleratorId();
			Entry.portId = pRtpOrMuxDesc->GetFirstPortId();
			Entry.channelId = 0;

			write_stat = AddEntryToSharedMemory(Entry); // pConnToCardMngr->Add(Entry);
			numberOfEntries += ConnToCardEntryDump(msg, Entry);
		}

		if (write_stat == STATUS_OK)     // succeeded to write in conn-to-card table
		{
			pResult->allocIndBase.allocatedRrcs[numRsrc].connectionId = connIdRTPOrMUX;
			pResult->allocIndBase.allocatedRrcs[numRsrc].logicalRsrcType = eLogical_rtp;
			numRsrc++;
		}
	}

	// **CS connId add - presume allocated if 'CreateAllocatePartyRsrc' succeeded
	if (partySCconnId)
	{
		pResult->allocIndBase.allocatedRrcs[numRsrc].connectionId = partySCconnId;
		pResult->allocIndBase.allocatedRrcs[numRsrc].logicalRsrcType = eLogical_ip_signaling;
		numRsrc++;
	}

	// **CS

	if (numberOfEntries)
		ConnToCardEntryDumpFooter(msg);

	// ***udp simulation
	if (bIsIPParty)
	{
		if (pUdpDesc)
			memcpy(&pResult->udpAdresses, &pUdpDesc->m_udp, sizeof(UdpAddresses));
	}
	else
		PASSERT(1);

	if (eCOP_party_type == videoPartyType)
	{
		// get "Dummy connection id" for the party's video encoder;
		// there is no actual video encoder resource allocated for the party, but the number needs to be unique (by Keren G.)
		DWORD connIdDummyVideoEnc = pSystemResources->AllocateConnId();
		if (connIdDummyVideoEnc == 0)                       // rollback?
			PASSERT(1);

		CRsrcDesc* pVideoEncodDummy = new CRsrcDesc(connIdDummyVideoEnc, eLogical_COP_dummy_encoder, rsrcConfId, rsrcPartyId);
		PASSERT(pConfRsrc->AddDesc(pVideoEncodDummy));      // Olga

		if (connIdDummyVideoEnc)
		{
			pResult->allocIndBase.allocatedRrcs[numRsrc].connectionId = connIdDummyVideoEnc;
			pResult->allocIndBase.allocatedRrcs[numRsrc].logicalRsrcType = eLogical_COP_dummy_encoder;
			numRsrc++;
		}

		POBJDELETE(pVideoEncodDummy);
	}

	// *** fill in Result for output

	pResult->allocIndBase.rsrc_conf_id = rsrcConfId;
	pResult->allocIndBase.rsrc_party_id = rsrcPartyId;
	pResult->allocIndBase.videoPartyType = videoPartyType;
	pResult->allocIndBase.networkPartyType = networkPartyType;
	pResult->allocIndBase.numRsrcs = numRsrc;
	pResult->allocIndBase.room_id = room_id;

	// Updating current number of parties in the system
	if (STATUS_OK == pResult->allocIndBase.status)
	{
		BOOL is_ong_part = pSystemResources->IsThereAnyParty();
		BOOL isIsdnParty = CHelperFuncs::IsISDNParty(networkPartyType);
		BasePartyDataStruct addPartyData(videoPartyType, partyData.m_partyRole, rsrcPartyId, isIsdnParty, bestArtBoardId, 0xFFFF, eNonMix, FALSE, serviceId, FALSE);
		pSystemResources->AddParty(addPartyData);
		// LED
		if (FALSE == is_ong_part)         // if it was false, turn it on
		{
			CIPMCInterfaceApi ipmcApi;
			ipmcApi.ChangeLedState(eAmber, eTurnOn);
		}
		// LED
	}

	POBJDELETE(pAudEncDesc);
	POBJDELETE(pAudDecDesc);
	POBJDELETE(pRtpOrMuxDesc);
	POBJDELETE(pUdpDesc);
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrcAlloc::DestroyDeAllocConfRsrcCOP(CSystemResources* pSystemResources, CConfRsrc* pConfRsrc, DWORD monitor_conf_id, DWORD deallocateStatus)
{
	bool is_kill_port = false;
	if (deallocateStatus != STATUS_OK)
	{
		TRACEINTO << " CRsrcAlloc::DestroyDeAllocConfRsrcCOP Start - Kill Port Required ";
		is_kill_port = true;
	}
	else
	{
		TRACEINTO << " CRsrcAlloc::DestroyDeAllocConfRsrcCOP Start";
	}

	if (is_kill_port)
	{
		SendKillPortsCop(pConfRsrc);
	}

	// get and print conference resource descriptors array
	CRsrcDesc** pRsrcDescArray = NULL;
	int num_rsrc = pConfRsrc->GetDescArray(pRsrcDescArray);
/*
	TRACEINTO << " CRsrcAlloc::DestroyDeAllocConfRsrcCOP Dump Conf CRsrcDesc List - start , num_rsrc = " << num_rsrc;
	for (int rsrc_array_index = 0; rsrc_array_index < num_rsrc; rsrc_array_index++)
	{
		if (pRsrcDescArray[rsrc_array_index])
		{
			CMedString mstr;
			pRsrcDescArray[rsrc_array_index]->Dump(mstr);
			PTRACE2(eLevelInfoNormal, "CRsrcAlloc::DestroyDeAllocConfRsrcCOP Dump CRsrcDesc:\n", mstr.GetString());
		}
	}
	TRACEINTO << " CRsrcAlloc::DestroyDeAllocConfRsrcCOP Dump Conf CRsrcDesc List - end";
*/
	// get board
	CRsrcDesc* pEncoderVideoDesc = (CRsrcDesc*)pConfRsrc->GetDescByType(eLogical_COP_HD720_encoder);
	//PASSERT_AND_RETURN_VALUE(pEncoderVideoDesc == NULL, STATUS_FAIL);
	WORD reqBoardId = pEncoderVideoDesc ? pEncoderVideoDesc->GetBoardId() : 0;
	CBoard* pBoard = pSystemResources->GetBoard(reqBoardId);
	if (!pBoard)
	{
		delete[] pRsrcDescArray;
		PASSERT_AND_RETURN_VALUE(pBoard == NULL, STATUS_FAIL);
	}

	CConnToCardManager* pConnToCardMngr = CHelperFuncs::GetConnToCardManager();
	if (!pConnToCardMngr)
	{
		PASSERT(1);  //error handling;
	}

	// deallocate video resources
	WORD connId = 0;
	DWORD rsrcEntityId = 0;
	eLogicalResourceTypes logicType = eLogical_res_none;

	STATUS deallocate_stat = STATUS_OK;
	STATUS remove_shared_Memory_status = STATUS_OK;
	STATUS remove_conf_desc_status = STATUS_OK;

	for (int i = 0; i < num_rsrc; i++)
	{
		if (!pRsrcDescArray[i])
		{
			continue;
		}

		logicType = pRsrcDescArray[i]->GetType();
		connId = pRsrcDescArray[i]->GetConnId();
		rsrcEntityId = pRsrcDescArray[i]->GetRsrcPartyId();

		if (CHelperFuncs::IsLogicalVideoDecoderType(logicType) || CHelperFuncs::IsLogicalVideoEncoderType(logicType))
		{
			deallocate_stat = pSystemResources->DeAllocateVideo2C(pConfRsrc, pRsrcDescArray[i], FALSE);

			if (pConnToCardMngr)
			{
				remove_shared_Memory_status = pConnToCardMngr->Remove(connId);
			}
			remove_conf_desc_status = pConfRsrc->RemoveDesc(rsrcEntityId, logicType, pRsrcDescArray[i]->GetCntrlType());

			pSystemResources->DeAllocateConnId(connId);

			if (deallocate_stat != STATUS_OK || remove_shared_Memory_status != STATUS_OK || remove_conf_desc_status != STATUS_OK)
			{
				//CMedString mstr;
				if (deallocate_stat != STATUS_OK)
				{
					//mstr << "deallocate_stat = " << deallocate_stat << "\n";
					PASSERT(deallocate_stat);
				}
				if (remove_shared_Memory_status != STATUS_OK)
				{
					//mstr << "remove_shared_Memory_status = " << remove_shared_Memory_status << "\n";
					PASSERT(remove_shared_Memory_status);
				}
				if (remove_conf_desc_status != STATUS_OK)
				{
					//mstr << "remove_conf_desc_status = " << remove_conf_desc_status << "\n";
					PASSERT(remove_conf_desc_status);
				}
				//pRsrcDescArray[i]->Dump(mstr);
				//PTRACE2(eLevelInfoNormal, "CRsrcAlloc::DestroyDeAllocConfRsrcCOP deallocate failed , CRsrcDesc:\n", mstr.GetString());
			}

		}
	}

	//  deallocate PCM
	CRsrcDesc* pPcmMenuDesc = (CRsrcDesc*)pConfRsrc->GetDescByType(eLogical_PCM_manager);
	if (pPcmMenuDesc)
	{
		WORD connIdPcmMenu = pPcmMenuDesc->GetConnId();
		pSystemResources->DeAllocateConnId(connIdPcmMenu);

		pBoard->DeallocatePcmMenuId(connIdPcmMenu);

		if (pConnToCardMngr)
		{
			PASSERT(pConnToCardMngr->Remove(connIdPcmMenu));
		}

		PASSERT(pConfRsrc->RemoveDesc((WORD)DUMMY_PARTY_ID, eLogical_PCM_manager));
	}

	if (STATUS_OK != deallocate_stat)
		TRACEINTO << "\nCRsrcAlloc::DestroyDeAllocConfRsrcCOP - status = " << deallocate_stat << "\n";
	pBoard->RemoveOneCOPConfOnThisBoard();
	// temporary debug
	// pConnToCardMngr->DumpRaw("CRsrcAlloc::DestroyDeAllocConfRsrcCOP - after");

	delete[] pRsrcDescArray;

	TRACEINTO << " CRsrcAlloc::DestroyDeAllocConfRsrcCOP end";
	return deallocate_stat;
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrcAlloc::CreateAllocPartyVSW(CSystemResources* pSystemResources, CConfRsrcDB* pConfDB, DWORD monitor_conf_id, DWORD monitor_party_id, DWORD& partyRsrcId, DWORD& partyCSconnId, PartyDataStruct& partyData, CRsrcDesc*& pEncDesc, CRsrcDesc*& pDecDesc, WORD* roomPartyId)
{
	TRACEINTO << "monitor_conf_id:" << monitor_conf_id << ", monitor_party_id:" << monitor_party_id;

	STATUS status = STATUS_OK;
	CConfRsrc* pConfRsrc = (CConfRsrc*)pConfDB->GetConfRsrc(monitor_conf_id);
	if (!pConfRsrc)
	{
		PASSERT(1);
		return STATUS_CONFERENCE_ALREADY_EXISTS;
	}

	if (pConfRsrc->GetParty(monitor_party_id) != NULL)  // party already exists, error
	{
		PASSERT(1);
		return STATUS_PARTY_ALREADY_EXISTS;
	}

	if (partyData.m_videoPartyType != eVideo_party_type_none)
	{
		// 1. Look for free enc/dec in conf resources list (find enc/dec with common rsrcEntityId and check their "GetIsUpdated()" flag)
		STATUS is_ok = pConfRsrc->GetFreeVideoRsrcVSW(pEncDesc, pDecDesc);
		if (is_ok != STATUS_OK)
			PASSERT(1);

		// 2. Occupied the found resources, i.e. "SetIsUpdated()" of related CRsrcDesc set to be TRUE
		pEncDesc->SetIsUpdated(TRUE);
		pDecDesc->SetIsUpdated(TRUE);

		// 3. Use found common rsrcEntityId as rsrcPartyId of the new participant
		partyRsrcId = pEncDesc->GetRsrcPartyId();
	}
	else
	{
#ifdef LOOKUP_TABLE_DEBUG_TRACE
		TRACEINTO << "D.K. PartyId:" << partyRsrcId;
#endif // ifdef LOOKUP_TABLE_DEBUG_TRACE
		if (partyRsrcId == 0) // failed to allocate
		{
			PASSERT(1);
			return STATUS_INSUFFICIENT_RSRC_PARTY_ID;
		}
	}

	WORD room_id = 0;

	// 4. CS connId patch
	eNetworkPartyType networkPartyType = partyData.m_networkPartyType;
	if (CHelperFuncs::IsIPParty(networkPartyType))
	{
		partyCSconnId = pSystemResources->AllocateConnId();
		if (partyCSconnId == 0) // fail
		{
			PASSERT(1);
			status = STATUS_INSUFFICIENT_RSRC_CONNECTION_ID;
		}
	}

	if (roomPartyId)
	{
		if (*roomPartyId != 0xFFFF)
		{
			TRACEINTO << "roomPartyId != 0xFFFF, so don't allocate room_id";
		}
		else
		{
			room_id = pSystemResources->AllocateRoomId();
			if (0 == room_id)
			{
				PASSERT(monitor_party_id);
				pSystemResources->DeAllocateConnId(partyCSconnId);
				return STATUS_INSUFFICIENT_ROOM_ID;
			}
			else
				*roomPartyId = room_id;
		}
	}

	// 5. Add party
	if (STATUS_OK == status)
	{
		CPartyRsrc party(monitor_party_id, networkPartyType, partyData.m_videoPartyType, CHelperFuncs::GetNumArtChannels(partyData));

		party.SetRsrcPartyId(partyRsrcId);
		party.SetCSconnId(partyCSconnId);

		if (((CConfRsrc*)(pConfRsrc))->AddParty(party) != STATUS_OK)
		{
			PASSERT(1);
			pSystemResources->DeAllocateConnId(partyCSconnId);
			status = STATUS_FAIL;
		}
	}

	// 6. Write the enc/dec conn id to shared memory
	if (STATUS_OK == status && partyData.m_videoPartyType != eVideo_party_type_none)
	{
		CConnToCardManager* pConnToCardMngr = CHelperFuncs::GetConnToCardManager();
		ConnToCardTableEntry Entry;

		Entry.rsrc_conf_id = pConfRsrc->GetRsrcConfId();
		Entry.rsrc_party_id = partyRsrcId;
		int numberOfEntries = 0;

		std::ostringstream msg;
		ConnToCardEntryDumpHeader(msg, "CRsrcAlloc::CreateAllocPartyVSW - Shared memory", Entry.rsrc_conf_id, Entry.rsrc_party_id, Entry.room_id);

		if (pConnToCardMngr)
		{
			STATUS write_stat = STATUS_OK;
			if (pEncDesc)
			{
				Entry.m_id             = pEncDesc->GetConnId();
				Entry.rsrcType         = eLogical_video_encoder;
				Entry.physicalRsrcType = ePhysical_video_encoder;
				Entry.boxId            = pEncDesc->GetBoxId();
				Entry.boardId          = pEncDesc->GetBoardId();
				Entry.subBoardId       = pEncDesc->GetSubBoardId();
				Entry.unitId           = pEncDesc->GetUnitId();
				Entry.acceleratorId    = pEncDesc->GetAcceleratorId();
				Entry.portId           = pEncDesc->GetFirstPortId();

				write_stat             = AddEntryToSharedMemory(Entry); // pConnToCardMngr->Add(Entry);
				numberOfEntries       += ConnToCardEntryDump(msg, Entry);
			}

			if (pDecDesc)
			{
				Entry.m_id             = pDecDesc->GetConnId();
				Entry.rsrcType         = eLogical_video_decoder;
				Entry.physicalRsrcType = ePhysical_video_decoder;
				Entry.boxId            = pDecDesc->GetBoxId();
				Entry.boardId          = pDecDesc->GetBoardId();
				Entry.subBoardId       = pDecDesc->GetSubBoardId();
				Entry.unitId           = pDecDesc->GetUnitId();
				Entry.acceleratorId    = pDecDesc->GetAcceleratorId();
				Entry.portId           = pDecDesc->GetFirstPortId();

				write_stat             = AddEntryToSharedMemory(Entry); // pConnToCardMngr->Add(Entry);
				numberOfEntries       += ConnToCardEntryDump(msg, Entry);
			}
		}

		if (numberOfEntries)
			ConnToCardEntryDumpFooter(msg);
	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrcAlloc::DestroyDeAllocConfRsrcVSW(CSystemResources* pSystemResources, CConfRsrc* pConfRsrc, DWORD monitor_conf_id)
{
	TRACEINTO << "monitor_conf_id:" << monitor_conf_id;
	if (!pConfRsrc)
		return STATUS_OK;

	WORD i = 0;
	CRsrcDesc** pRsrcDescArray = NULL;

	int num_rsrc = pConfRsrc->GetDescArray(pRsrcDescArray);

	DWORD connIdsVideo[num_rsrc];
	for (int i = 0; i < num_rsrc; i++)
		connIdsVideo[i] = 0;

	WORD boardIdNum[BOARDS_NUM];
	for (i = 0; i < BOARDS_NUM; i++)
		boardIdNum[i] = 0;

	WORD connId = 0, connIdIndex = 0, boardId = 0, rsrcEntityId = 0;
	eLogicalResourceTypes logicType = eLogical_res_none;
	STATUS status = STATUS_OK;

	for (i = 0; i < num_rsrc; i++)
	{
		if (!pRsrcDescArray[i])
			continue;

		boardId = pRsrcDescArray[i]->GetBoardId();

		if ((boardId <= BOARDS_NUM) && (boardId > 0))
			boardIdNum[boardId - 1]++;
		else
			PASSERTMSG(1, "GetBoardId return exceed BOARDS_NUM");

		logicType = pRsrcDescArray[i]->GetType();
		rsrcEntityId = pRsrcDescArray[i]->GetRsrcPartyId();

		connId = pRsrcDescArray[i]->GetConnId();
		pSystemResources->DeAllocateConnId(connId);

		if (pRsrcDescArray[i]->GetIsUpdated())
		{
			connIdsVideo[connIdIndex] = connId;
			connIdIndex++;
		}

		status = pSystemResources->DeAllocateVideo2C(pConfRsrc, pRsrcDescArray[i], FALSE);

		// remove descriptors from conf
		PASSERT(pConfRsrc->RemoveDesc(rsrcEntityId, logicType, pRsrcDescArray[i]->GetCntrlType()));
	}

	delete[] pRsrcDescArray;

	eSessionType sessionType = pConfRsrc->GetSessionType();

	for (i = 0; i < BOARDS_NUM; i++)
	{
		if (boardIdNum[i] == 0)
			continue;

		CBoard* pBoard = pSystemResources->GetBoard(i + 1);  // board num is 1-based
		if (pBoard)
		{
			pBoard->RemoveOneCOPConfOnThisBoard();
			if (boardIdNum[i] > MAX_PARTIES_IN_CONF_2C_VSW56)
				pBoard->RemoveOneCOPConfOnThisBoard();     // need to call twice because eVSW_56_session we count like 2x28 conferences
		}
		else
			PASSERTMSG(1, "GetBoard return NULL");
	}

	// deallocate video conn_id's
	CConnToCardManager* pConnToCardMngr = CHelperFuncs::GetConnToCardManager();
	if (!pConnToCardMngr)
	{
		PASSERT(1);  // error handling;
	}
	else
	{
		for (i = 0; i < connIdIndex; i++)
		{
			if (connIdsVideo[i])
				PASSERT(pConnToCardMngr->Remove(connIdsVideo[i]));
		}
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////
void CRsrcAlloc::KillVideoPortVSW(DWORD monitor_conf_id, DWORD rsrcPartyId)
{
	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	CConfRsrc* pConf = (CConfRsrc*)(pConfRsrcDB ? pConfRsrcDB->GetConfRsrc(monitor_conf_id) : NULL);
	PASSERT_AND_RETURN(pConf == NULL);

	PhysicalPortDesc portDesc;
	CRsrcDesc* pVideoEncDesc = (CRsrcDesc*)pConf->GetDesc(rsrcPartyId, eLogical_video_encoder, E_NORMAL);
	if (pVideoEncDesc)
	{
		portDesc.m_boxId         = pVideoEncDesc->GetBoxId();
		portDesc.m_boardId       = pVideoEncDesc->GetBoardId();
		portDesc.m_subBoardId    = pVideoEncDesc->GetSubBoardId();
		portDesc.m_unitId        = pVideoEncDesc->GetUnitId();
		portDesc.m_portId        = pVideoEncDesc->GetFirstPortId();
		portDesc.m_acceleratorId = pVideoEncDesc->GetAcceleratorId();

		STATUS kill_stat = SendKillPortRequest(&portDesc, ePhysical_video_encoder);
		PASSERT(kill_stat);
	}
	CRsrcDesc* pVideoDecDesc = (CRsrcDesc*)pConf->GetDesc(rsrcPartyId, eLogical_video_decoder, E_NORMAL);
	if (pVideoDecDesc)
	{
		portDesc.m_boxId         = pVideoDecDesc->GetBoxId();
		portDesc.m_boardId       = pVideoDecDesc->GetBoardId();
		portDesc.m_subBoardId    = pVideoDecDesc->GetSubBoardId();
		portDesc.m_unitId        = pVideoDecDesc->GetUnitId();
		portDesc.m_portId        = pVideoDecDesc->GetFirstPortId();
		portDesc.m_acceleratorId = pVideoDecDesc->GetAcceleratorId();

		STATUS kill_stat = SendKillPortRequest(&portDesc, ePhysical_video_decoder);
		PASSERT(kill_stat);
	}
}

////////////////////////////////////////////////////////////////////////////
void CRsrcAlloc::AllocatePartyEQ(ALLOC_PARTY_REQ_PARAMS_S* pParam, ALLOC_PARTY_IND_PARAMS_S* pResult)
{
	//*** check parameters for validity (pParam)

	//*** if FIRST party in conference - create conference.
	//allocate conf rsrc id and master audio cntler.

	//***create party. allocate rsrc party id.
	//allocate needed ports (ART, VIDEO, RTM).

	//***write allocation results to Conn-To-Cards table

	//***fill in returned struct (pResult)

	TRACEINTO << "\nCRsrcAlloc::AllocatePartyEQ :  monitor_conf_id = " << pParam->monitor_conf_id << ", monitor_party_id = " << pParam->monitor_party_id << "\n";

	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	CReservator* pReservator = CHelperFuncs::GetReservator();

	if (pSystemResources == NULL || pConfRsrcDB == NULL || pReservator == NULL)
	{
		pResult->allocIndBase.status = STATUS_FAIL;
		TRACEINTO << " CRsrcAlloc::AllocatePartyEQ failed. pSystemResources or pConfRsrcDB or pReservator is NULL " << "\n\n";
		return;
	}

	if (FALSE == pSystemResources->IsThereExistUtilizableUnitForAudioController())
	{
		TRACEINTO << " CRsrcAlloc::AllocatePartyEQ failed. No utilizable unit for Audio Controller \n";
		pResult->allocIndBase.status = STATUS_NO_UTILIZABLE_UNIT_FOR_AUDIO_CONTROLLER;
		return;
	}

	DWORD monitor_conf_id = pParam->monitor_conf_id;
	DWORD monitor_party_id = pParam->monitor_party_id;

	eVideoPartyType reqVideoPartyType = pParam->videoPartyType;
	eVideoPartyType videoPartyType = reqVideoPartyType;
	eNetworkPartyType networkPartyType = pParam->networkPartyType;
	eSessionType sessionType = pParam->sessionType;
	WORD serviceId = pParam->serviceId;
	DWORD optionsMask = pParam->optionsMask;
	WORD subServiceId = pParam->subServiceId;
	EAllocationPolicy allocPolicy = (EAllocationPolicy)(pParam->allocationPolicy);

	WORD numRsrc = 0, numConfUndefParties = 0;

	BOOL bIsIPParty = CHelperFuncs::IsIPParty(networkPartyType);

	//*** for alloc. and output
	DWORD rsrcConfId = 0;
	PartyRsrcID rsrcPartyId = 0;

	//*** for alloc result
	DWORD connIdAudEnc = 0, connIdAudDec = 0, connIdRTPOrMUX = 0, partySCconnId = 0;

	CRsrcDesc* pAudEncDesc = NULL;
	CRsrcDesc* pAudDecDesc = NULL;
	CRsrcDesc* pRtpOrMuxDesc = NULL;
	CUdpRsrcDesc* pUdpDesc = NULL;

	STATUS status = STATUS_OK;
	WORD bestArtBoardId = 0xFFFF, bestVideoBoardId = 0xFFFF;
	DWORD artIpAddr = 0;

	ResourcesInterface* pResourcesInterface = pSystemResources->GetCurrentResourcesInterface();
	if (pResourcesInterface == NULL)
	{
		pResult->allocIndBase.status = STATUS_FAIL;
		PASSERT(1);
		return;
	}

	CConfRsrvRsrc* pConf = (CConfRsrvRsrc*)(pReservator->GetConfRsrvRsrcById(monitor_conf_id));

	if (pConfRsrcDB->IsExitingConf(monitor_conf_id) == FALSE) // while spreading it has to be done, excluded EQ
	{
		if (eSTANDALONE_session == sessionType)
		{
			rsrcConfId = STANDALONE_CONF_ID;
			status = CreateAllocConfRsrc(pSystemResources, pConfRsrcDB, monitor_conf_id, rsrcConfId, sessionType, eConfMediaType_dummy);
		}
		else
		{
			pResult->allocIndBase.status = STATUS_ALLOC_REQ_WITHOUT_SPREAD_FOR_NON_EQ_CONF;
			return;
		}
	}
	else
	{
		rsrcConfId = pConfRsrcDB->MonitorToRsrcConfId(monitor_conf_id);  //***gets existing rsrcConfId from DB
	}

	int actualNumberOfRTMPortsToAllocate = 0;
	PartyDataStruct partyData;
	memset(&partyData, 0, sizeof(partyData));
	partyData.m_videoPartyType = videoPartyType;
	partyData.m_partyRole = pParam->partyRole;
	partyData.m_networkPartyType = networkPartyType;
	partyData.m_allowReconfiguration = pParam->isWaitForRsrcAndAskAgain;
	partyData.m_monitor_conf_id = monitor_conf_id;
	partyData.m_confMediaType = pParam->confMediaType;
	partyData.m_HdVswTypeInMixAvcSvcMode = pParam->HdVswTypeInMixAvcSvcMode;

	BestAllocStruct bestAlloction;
	memset(&bestAlloction, 0, sizeof(bestAlloction));

	if (status == STATUS_OK)
	{
		rsrcPartyId = pParam->party_id;
#ifdef LOOKUP_TABLE_DEBUG_TRACE
		TRACEINTO << "D.K. PartyId:" << rsrcPartyId;
#endif
		status = CreateAllocPartyRsrc(pSystemResources,
		                              pConfRsrcDB,
		                              monitor_conf_id,
		                              monitor_party_id,
		                              rsrcPartyId,
		                              partySCconnId,
		                              partyData);
	}

	if (status == STATUS_OK)
	{
		status = pSystemResources->GetBestARTBoardCOP(partyData, bestAlloction, -1);  //GetBestBoards - Olga?
	}

	bestArtBoardId = bestAlloction.m_ArtBoardId;
	bestVideoBoardId = bestAlloction.m_VideoAlloc.m_boardId;

	if (status == STATUS_OK) //&& ( CHelperFuncs::IsAudioParty(videoPartyType))) //Olga - Sergey?
	{
		status = AllocateART(monitor_conf_id /*rsrcConfId*/, rsrcPartyId, pParam->artCapacity, serviceId, subServiceId, sessionType, partyData, pAudEncDesc, pAudDecDesc, pRtpOrMuxDesc, pUdpDesc, pParam->isIceParty, pParam->isBFCP, bestArtBoardId); //fix!
	}

	if (status != STATUS_OK) //*** rollback
	{
		DestroyDeAllocPartyRsrc(pSystemResources, pConfRsrcDB, monitor_conf_id, monitor_party_id);
	}

	pResult->allocIndBase.status = status;
	if (status != STATUS_OK) //special treatment for STATUS_NO_VIDEO_PORT ???
	{
		POBJDELETE(pAudEncDesc);
		POBJDELETE(pAudDecDesc);
		POBJDELETE(pRtpOrMuxDesc);

		POBJDELETE(pUdpDesc);
		return;
	}

	/*conn-to-card write*/

	CConnToCardManager* pConnToCardMngr = CHelperFuncs::GetConnToCardManager();
	if (!pConnToCardMngr)
		PASSERT(1);		//error handling;

	eResourceTypes ARTphysType = ePhysical_art;

	ConnToCardTableEntry Entry;

	connIdAudEnc = (pAudEncDesc) ? pAudEncDesc->GetConnId() : 0;
	connIdAudDec = (pAudDecDesc) ? pAudDecDesc->GetConnId() : 0;
	connIdRTPOrMUX = (pRtpOrMuxDesc) ? pRtpOrMuxDesc->GetConnId() : 0;

	Entry.rsrc_conf_id = rsrcConfId;
	Entry.rsrc_party_id = rsrcPartyId;
	int numberOfEntries = 0;

	std::ostringstream msg;
	ConnToCardEntryDumpHeader(msg, "CRsrcAlloc::AllocatePartyEQ - Shared memory", Entry.rsrc_conf_id, Entry.rsrc_party_id, Entry.room_id);

	STATUS write_stat = STATUS_OK;

	WORD chanell_id_rtm = 0;

	/*conn-to-card write*/

	if (connIdAudEnc)
	{
		if (pConnToCardMngr)
		{
			Entry.m_id             = connIdAudEnc;
			Entry.rsrcType         = eLogical_audio_encoder;
			Entry.physicalRsrcType = ARTphysType;
			Entry.boxId            = pAudEncDesc->GetBoxId();
			Entry.boardId          = pAudEncDesc->GetBoardId();
			Entry.subBoardId       = pAudEncDesc->GetSubBoardId();
			Entry.unitId           = pAudEncDesc->GetUnitId();
			Entry.acceleratorId    = pAudEncDesc->GetAcceleratorId();
			Entry.portId           = pAudEncDesc->GetFirstPortId();

			write_stat             = AddEntryToSharedMemory(Entry); // pConnToCardMngr->Add(Entry);
			numberOfEntries       += ConnToCardEntryDump(msg, Entry);
		}

		if (write_stat == STATUS_OK)  // succeeded to write in conn-to-card table
		{
			pResult->allocIndBase.allocatedRrcs[numRsrc].connectionId    = connIdAudEnc;
			pResult->allocIndBase.allocatedRrcs[numRsrc].logicalRsrcType = eLogical_audio_encoder;
			numRsrc++;
		}
	}

	if (connIdAudDec)
	{
		if (pConnToCardMngr)
		{
			Entry.m_id             = connIdAudDec;
			Entry.rsrcType         = eLogical_audio_decoder;
			Entry.physicalRsrcType = ARTphysType;
			Entry.boxId            = pAudDecDesc->GetBoxId();
			Entry.boardId          = pAudDecDesc->GetBoardId();
			Entry.subBoardId       = pAudDecDesc->GetSubBoardId();
			Entry.unitId           = pAudDecDesc->GetUnitId();
			Entry.acceleratorId    = pAudDecDesc->GetAcceleratorId();
			Entry.portId           = pAudDecDesc->GetFirstPortId();

			write_stat             = AddEntryToSharedMemory(Entry); // pConnToCardMngr->Add(Entry);
			numberOfEntries       += ConnToCardEntryDump(msg, Entry);
		}

		if (write_stat == STATUS_OK)  // succeeded to write in conn-to-card table
		{
			pResult->allocIndBase.allocatedRrcs[numRsrc].connectionId    = connIdAudDec;
			pResult->allocIndBase.allocatedRrcs[numRsrc].logicalRsrcType = eLogical_audio_decoder;
			numRsrc++;
		}
	}

	if (connIdRTPOrMUX)
	{
		if (pConnToCardMngr)
		{
			Entry.m_id             = connIdRTPOrMUX;
			Entry.rsrcType         = eLogical_rtp;
			Entry.physicalRsrcType = ARTphysType;
			Entry.boxId            = pRtpOrMuxDesc->GetBoxId();
			Entry.boardId          = pRtpOrMuxDesc->GetBoardId();
			Entry.subBoardId       = pRtpOrMuxDesc->GetSubBoardId();
			Entry.unitId           = pRtpOrMuxDesc->GetUnitId();
			Entry.acceleratorId    = pRtpOrMuxDesc->GetAcceleratorId();
			Entry.portId           = pRtpOrMuxDesc->GetFirstPortId();
			Entry.channelId        = 0;

			write_stat             = AddEntryToSharedMemory(Entry); // pConnToCardMngr->Add(Entry);
			numberOfEntries       += ConnToCardEntryDump(msg, Entry);
		}

		if (write_stat == STATUS_OK)  // succeeded to write in conn-to-card table
		{
			pResult->allocIndBase.allocatedRrcs[numRsrc].connectionId    = connIdRTPOrMUX;
			pResult->allocIndBase.allocatedRrcs[numRsrc].logicalRsrcType = eLogical_rtp;
			numRsrc++;
		}
	}

	// **CS connId add - presume allocated if 'CreateAllocatePartyRsrc' succeeded
	if (partySCconnId)
	{
		pResult->allocIndBase.allocatedRrcs[numRsrc].connectionId    = partySCconnId;
		pResult->allocIndBase.allocatedRrcs[numRsrc].logicalRsrcType = eLogical_ip_signaling;
		numRsrc++;
	}

	if (numberOfEntries)
		ConnToCardEntryDumpFooter(msg);

	if (bIsIPParty)
	{
		if (pUdpDesc)
			memcpy(&pResult->udpAdresses, &pUdpDesc->m_udp, sizeof(UdpAddresses));
	}
	else
		PASSERT(1);

	if (eCOP_party_type == videoPartyType)
	{
		// get "Dummy connection id" for the party's video encoder;
		// there is no actual video encoder resource allocated for the party, but the number needs to be unique (by Keren G.)
		DWORD connIdDummyVideoEnc = pSystemResources->AllocateConnId();
		if (connIdDummyVideoEnc == 0)		// rollback?
			PASSERT(1);

		CRsrcDesc* pVideoEncodDummy = new CRsrcDesc(connIdDummyVideoEnc, eLogical_COP_dummy_encoder, rsrcConfId, rsrcPartyId);
		CConfRsrc* pConfRsrc = (CConfRsrc*)(pConfRsrcDB ? pConfRsrcDB->GetConfRsrc(monitor_conf_id) : NULL);

		PASSERT((NULL == pConfRsrc) || pConfRsrc->AddDesc(pVideoEncodDummy));

		if (connIdDummyVideoEnc)
		{
			pResult->allocIndBase.allocatedRrcs[numRsrc].connectionId    = connIdDummyVideoEnc;
			pResult->allocIndBase.allocatedRrcs[numRsrc].logicalRsrcType = eLogical_COP_dummy_encoder;
			numRsrc++;
		}
		POBJDELETE(pVideoEncodDummy);
	}

	else if (eVSW_video_party_type == videoPartyType)
	{
		// get "Dummy connection id" for the party's video encoder;
		// there is no actual video encoder resource allocated for the party, but the number needs to be unique (by Keren G.)
		DWORD connIdDummyVSWVideoEnc = pSystemResources->AllocateConnId();
		DWORD connIdDummyVSWVideoDec = pSystemResources->AllocateConnId();
		if (connIdDummyVSWVideoEnc == 0 || connIdDummyVSWVideoDec == 0)		// rollback?
			PASSERT(1);

		CRsrcDesc* pVideoEncVSWDummy = new CRsrcDesc(connIdDummyVSWVideoEnc, eLogical_VSW_dummy_encoder, rsrcConfId, rsrcPartyId);
		CRsrcDesc* pVideoDecVSWDummy = new CRsrcDesc(connIdDummyVSWVideoDec, eLogical_VSW_dummy_decoder, rsrcConfId, rsrcPartyId);

		CConfRsrc* pConfRsrc = (CConfRsrc*)(pConfRsrcDB ? pConfRsrcDB->GetConfRsrc(monitor_conf_id) : NULL);

		PASSERT((NULL == pConfRsrc) || pConfRsrc->AddDesc(pVideoEncVSWDummy) || pConfRsrc->AddDesc(pVideoDecVSWDummy));
		// rollback
		if (connIdDummyVSWVideoEnc && connIdDummyVSWVideoDec)
		{
			pResult->allocIndBase.allocatedRrcs[numRsrc].connectionId    = connIdDummyVSWVideoEnc;
			pResult->allocIndBase.allocatedRrcs[numRsrc].logicalRsrcType = eLogical_VSW_dummy_encoder;
			numRsrc++;
			pResult->allocIndBase.allocatedRrcs[numRsrc].connectionId    = connIdDummyVSWVideoDec;
			pResult->allocIndBase.allocatedRrcs[numRsrc].logicalRsrcType = eLogical_VSW_dummy_decoder;
			numRsrc++;
		}
		POBJDELETE(pVideoEncVSWDummy);
		POBJDELETE(pVideoDecVSWDummy);
	}

	//*** fill in Result for output

	pResult->allocIndBase.rsrc_conf_id = rsrcConfId;
	pResult->allocIndBase.rsrc_party_id = rsrcPartyId;
	pResult->allocIndBase.videoPartyType = videoPartyType;
	pResult->allocIndBase.networkPartyType = networkPartyType;
	pResult->allocIndBase.numRsrcs = numRsrc;

	//Updating current number of parties in the system
	if (STATUS_OK == pResult->allocIndBase.status)
	{
		BOOL isIsdnParty = CHelperFuncs::IsISDNParty(networkPartyType);
		BasePartyDataStruct addPartyData(videoPartyType, partyData.m_partyRole, rsrcPartyId, isIsdnParty, bestArtBoardId, 0xFFFF, eNonMix, FALSE, serviceId, FALSE);
		pSystemResources->AddParty(addPartyData);

		BOOL is_ong_part = pSystemResources->IsThereAnyParty();

		//LED
		if (FALSE == is_ong_part) //if it was false, turn it on
		{
			CIPMCInterfaceApi ipmcApi;
			ipmcApi.ChangeLedState(eAmber, eTurnOn);
		}
	}

	POBJDELETE(pAudEncDesc);
	POBJDELETE(pAudDecDesc);
	POBJDELETE(pRtpOrMuxDesc);
	POBJDELETE(pUdpDesc);
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrcAlloc::EndPartyMove2C(PARTY_MOVE_RSRC_REQ_PARAMS_S* pParams, PARTY_MOVE_RSRC_IND_PARAMS_S* pResult)
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	STATUS status = STATUS_OK;

	pResult->target_monitor_conf_id = pParams->target_monitor_conf_id;
	pResult->monitor_party_id = pParams->target_monitor_party_id;
	pResult->numRsrcs = 0;
	DWORD numRsrc = 0;

	if (pSystemResources == NULL || pConfRsrcDB == NULL)
	{
		PASSERT(1);
		status = STATUS_FAIL;
	}

	if (status == STATUS_OK)
	{
		DWORD source_monitor_conf_id = pParams->source_monitor_conf_id;
		DWORD target_monitor_conf_id = pParams->target_monitor_conf_id;

		if (!(pConfRsrcDB->IsExitingConf(source_monitor_conf_id) && pConfRsrcDB->IsExitingConf(target_monitor_conf_id)))
		{
			PASSERT(1);
			status = STATUS_CONFERENCE_NOT_EXISTS;
		}

		if (status == STATUS_OK)
		{
			pResult->target_rsrc_conf_id = pConfRsrcDB->MonitorToRsrcConfId(target_monitor_conf_id);
			if (pResult->target_rsrc_conf_id == 0)
			{
				PTRACE(eLevelError, "CRsrcAlloc::EndPartyMove2C : Target Conference isn't found");
				pResult->target_rsrc_conf_id = 0xFFFFFFFF;
			}

			CConfRsrc* pSrcConfRsrc = (CConfRsrc*)pConfRsrcDB->GetConfRsrc(source_monitor_conf_id);
			CConfRsrc* pTrgConfRsrc = (CConfRsrc*)pConfRsrcDB->GetConfRsrc(target_monitor_conf_id);
			if (NULL == pSrcConfRsrc)
			{
				PTRACE(eLevelInfoNormal, "CRsrcAlloc::EndPartyMove2C : pSrcConfRsrc is NULL");
				PASSERT(1);
				return STATUS_END_RSRC_PARTY_MOVE_FAIL;
			}

			if (NULL == pTrgConfRsrc)
			{
				PTRACE(eLevelInfoNormal, "CRsrcAlloc::EndPartyMove2C : pTrgConfRsrc is NULL");
				PASSERT(1);
				return STATUS_END_RSRC_PARTY_MOVE_FAIL;
			}

			DWORD source_monitor_party_id = pParams->source_monitor_party_id;
			DWORD target_monitor_party_id = pParams->target_monitor_party_id;

			const CPartyRsrc* pPartyRsrc = pSrcConfRsrc->GetParty(source_monitor_party_id);
			if (pPartyRsrc == NULL)
			{
				PASSERT(1);
				status = STATUS_PARTY_NOT_EXISTS;
			}

			if (status == STATUS_OK)
			{
				CPartyRsrc* pPartyTrg = new CPartyRsrc(*pPartyRsrc);

				pPartyTrg->SetMonitorPartyId(target_monitor_party_id);

				WORD bId = 0, uId = 0, portId = 0;
				DWORD connIdRTM = 0, connIdAudEnc = 0, connIdAudDec = 0, connIdRTPOrMux = 0;

				if (pTrgConfRsrc->AddParty(*pPartyTrg) != STATUS_OK)
				{
					PASSERT(1);
					status = STATUS_FAIL;
				}

				// to update all active ports according to target ids.
				// to update all descriptors(delete from dest side, add to target side)
				// to update all shared memory's entities
				// remove(delete) from source all needed items

				DWORD rsrcPartyId = pPartyTrg->GetRsrcPartyId();
				pResult->rsrc_party_id = rsrcPartyId;
				DWORD rsrcConfTrgId = pTrgConfRsrc->GetRsrcConfId();
				// ART
				CRsrcDesc* pAEncSrcDesc = (CRsrcDesc*)pSrcConfRsrc->GetDesc(rsrcPartyId, eLogical_audio_encoder);
				if (pAEncSrcDesc)
				{
					bId    = pAEncSrcDesc->GetBoardId();
					uId    = pAEncSrcDesc->GetUnitId();
					portId = pAEncSrcDesc->GetFirstPortId();
				}
				else
					status = STATUS_FAIL;

				if (status == STATUS_OK)
				{
					int numberOfEntries = 0;
					std::ostringstream msg;
					ConnToCardEntryDumpHeader(msg, "CRsrcAlloc::EndPartyMove2C - Shared memory", rsrcConfTrgId, rsrcPartyId, -1);

					CConnToCardManager* pConnToCardMngr = CHelperFuncs::GetConnToCardManager();

					// 1. Audio encoder
					status = UpdateRsrcDesc(pSrcConfRsrc, pTrgConfRsrc, rsrcPartyId, eLogical_audio_encoder, connIdAudEnc);
					// 2. Audio decoder
					status = UpdateRsrcDesc(pSrcConfRsrc, pTrgConfRsrc, rsrcPartyId, eLogical_audio_decoder, connIdAudDec);

					if (CHelperFuncs::IsIPParty(pPartyTrg->GetNetworkPartyType()))
					{
						// 3. RTP
						status = UpdateRsrcDesc(pSrcConfRsrc, pTrgConfRsrc, rsrcPartyId, eLogical_rtp, connIdRTPOrMux);

						// 4. UDP //need to define copy constr.
						CUdpRsrcDesc* pUdpDesc = (CUdpRsrcDesc*)(pSrcConfRsrc->GetUdpDesc(rsrcPartyId));
						if (pUdpDesc)
						{
							CUdpRsrcDesc* pTrgUdpDesc = new CUdpRsrcDesc(rsrcPartyId);
							pTrgUdpDesc->SetServId(pUdpDesc->GetServId());
							memcpy(&(pTrgUdpDesc->m_udp.IpV4Addr), &(pUdpDesc->m_udp.IpV4Addr), sizeof(ipAddressV4If));
							pTrgUdpDesc->m_udp.type               = pUdpDesc->m_udp.type;
							pTrgUdpDesc->SetPQMId(pUdpDesc->GetPQMId());
							pTrgUdpDesc->m_udp.AudioChannelPort   = pUdpDesc->m_udp.AudioChannelPort;
							pTrgUdpDesc->m_udp.VideoChannelPort   = pUdpDesc->m_udp.VideoChannelPort;
							pTrgUdpDesc->m_udp.FeccChannelPort    = pUdpDesc->m_udp.FeccChannelPort;
							pTrgUdpDesc->m_udp.ContentChannelPort = pUdpDesc->m_udp.ContentChannelPort;

							PASSERT(pTrgConfRsrc->AddUdpDesc(pTrgUdpDesc));
							PASSERT(pSrcConfRsrc->RemoveUdpDesc(rsrcPartyId));

							POBJDELETE(pTrgUdpDesc);
						}
						else
							PTRACE(eLevelError, "CRsrcAlloc::EndPartyMove2C - pUdpDesc is NULL!!!");
					}

					// update ART active port
					status = pSystemResources->UpdateActivePort(bId, uId, portId, rsrcConfTrgId, rsrcPartyId);

					// COP - Video
					eSessionType sessionType = pTrgConfRsrc->GetSessionType();
					if (eCOP_HD1080_session == sessionType || eCOP_HD720_50_session == sessionType)
					{
						CRsrcDesc* pSrcVideoEncodDummy = (CRsrcDesc*)pSrcConfRsrc->GetDesc(rsrcPartyId, eLogical_COP_dummy_encoder);
						if (pSrcVideoEncodDummy)
						{
							// delete a dummy video encoder of this party from resources list of source conference
							WORD connIdDummyVideoEnc = pSrcVideoEncodDummy->GetConnId();
							PASSERT(pSrcConfRsrc->RemoveDesc(rsrcPartyId, eLogical_COP_dummy_encoder));

							// add dummy video encoder descriptor of this party to the target conference
							CRsrcDesc* pTrgVideoEncodDummy = new CRsrcDesc(connIdDummyVideoEnc, eLogical_COP_dummy_encoder, rsrcConfTrgId, rsrcPartyId);
							PASSERT(pTrgConfRsrc->AddDesc(pTrgVideoEncodDummy));
							POBJDELETE(pTrgVideoEncodDummy);
						}
					}

					// NxM - Video
					CRsrcDesc* pTrgEncDescVsw = NULL, *pTrgDecDescVsw = NULL;
					if (eVSW_56_session == sessionType || eVSW_28_session == sessionType)
					{
						if (eVideo_party_type_none != ((CPartyRsrc*)pPartyRsrc)->GetVideoPartyType())
						{
							CRsrcDesc* pEncDesc = NULL, *pDecDesc = NULL;

							PASSERT(pSrcConfRsrc->RemoveDesc(rsrcPartyId, eLogical_VSW_dummy_encoder));
							PASSERT(pSrcConfRsrc->RemoveDesc(rsrcPartyId, eLogical_VSW_dummy_decoder));

							// 1. Look for free enc/dec in conf resources list
							STATUS is_ok = pTrgConfRsrc->GetFreeVideoRsrcVSW(pEncDesc, pDecDesc);
							if (is_ok != STATUS_OK)
								PASSERT(1);

							// 2. Occupied the found resources
							WORD old_rsrcPartyId = pEncDesc ? pEncDesc->GetRsrcPartyId() : 0;
							if (pEncDesc)
							{
								eLogicalResourceTypes logicEncRsrcType = pEncDesc->GetType();
								pTrgEncDescVsw = new CRsrcDesc(*pEncDesc);

								pTrgEncDescVsw->SetRsrcPartyId(rsrcPartyId);
								pTrgEncDescVsw->SetIsUpdated(TRUE);

								PASSERT(((CConfRsrc* )pTrgConfRsrc)->RemoveDesc(old_rsrcPartyId, logicEncRsrcType));
								PASSERT(((CConfRsrc* )pTrgConfRsrc)->AddDesc(pTrgEncDescVsw));

								status = pSystemResources->UpdateActivePort(pTrgEncDescVsw->GetBoardId(),
								                                            pTrgEncDescVsw->GetUnitId(),
								                                            pTrgEncDescVsw->GetFirstPortId(),
								                                            rsrcConfTrgId, rsrcPartyId);
							}

							if (pDecDesc)
							{
								eLogicalResourceTypes logicDecRsrcType = pDecDesc->GetType();
								pTrgDecDescVsw = new CRsrcDesc(*pDecDesc);

								pTrgDecDescVsw->SetRsrcPartyId(rsrcPartyId);
								pTrgDecDescVsw->SetIsUpdated(TRUE);

								PASSERT(((CConfRsrc* )pTrgConfRsrc)->RemoveDesc(old_rsrcPartyId, logicDecRsrcType));
								PASSERT(((CConfRsrc* )pTrgConfRsrc)->AddDesc(pTrgDecDescVsw));

								status = pSystemResources->UpdateActivePort(pTrgDecDescVsw->GetBoardId(),
								                                            pTrgDecDescVsw->GetUnitId(),
								                                            pTrgDecDescVsw->GetFirstPortId(),
								                                            rsrcConfTrgId, rsrcPartyId);
							}
						}
					}
					else if (eVSW_Auto_session == sessionType) // Auto-VSW Video
					{
						PTRACE(eLevelInfoNormal, "CRsrcAlloc::EndPartyMove2C : video for Auto-VSW conf");

						if (eVideo_party_type_none != ((CPartyRsrc*)pPartyRsrc)->GetVideoPartyType())
						{
							PASSERT(pSrcConfRsrc->RemoveDesc(rsrcPartyId, eLogical_VSW_dummy_encoder));
							PASSERT(pSrcConfRsrc->RemoveDesc(rsrcPartyId, eLogical_VSW_dummy_decoder));

							PartyDataStruct partyData;
							memset(&partyData, 0, sizeof(partyData));
							partyData.m_videoPartyType = ((CPartyRsrc*)pPartyRsrc)->GetVideoPartyType();
							partyData.m_networkPartyType = pPartyTrg->GetNetworkPartyType();
							partyData.m_allowReconfiguration = FALSE;
							partyData.m_monitor_conf_id = target_monitor_conf_id;
							partyData.m_HdVswTypeInMixAvcSvcMode = pPartyTrg->GetHdVswTypeInMixAvcSvcMode();

							BestAllocStruct bestAlloction;
							memset(&bestAlloction, 0, sizeof(bestAlloction));
							status = pSystemResources->GetBestBoards(partyData, bestAlloction);
							if (status != STATUS_OK)
							{
								TRACEINTO << "Status:" << CProcessBase::GetProcess()->GetStatusAsString(status) << " - Fragmentation problem";
							}

							CRsrcDesc** pVideoDescArray = NULL;
							status = AllocateVideo(target_monitor_conf_id,
							                       rsrcPartyId,
							                       sessionType,
							                       bestAlloction.m_VideoAlloc,
							                       partyData,
							                       pVideoDescArray);
							// no need to call AddDesc because it is done inside of AllocateVideo()

							if (STATUS_OK == status && pConnToCardMngr)
							{
								if (pVideoDescArray != NULL)
								{
									CRsrcDesc* pRsrcDescriptor = NULL;
									ConnToCardTableEntry Entry;
									for (int i = 0; i < TOTAL_MAX_VIDEO_RESOURCES; i++)
									{
										if (pVideoDescArray[i] != NULL)
										{
											Entry.rsrc_conf_id     = rsrcConfTrgId;
											Entry.rsrc_party_id    = rsrcPartyId;
											Entry.m_id             = pVideoDescArray[i]->GetConnId();
											Entry.rsrcType         = pVideoDescArray[i]->GetType();
											Entry.physicalRsrcType = pVideoDescArray[i]->GetPhysicalType();
											Entry.rsrcCntlType     = pVideoDescArray[i]->GetCntrlType();
											Entry.boxId            = pVideoDescArray[i]->GetBoxId();
											Entry.boardId          = pVideoDescArray[i]->GetBoardId();
											Entry.subBoardId       = pVideoDescArray[i]->GetSubBoardId();
											Entry.unitId           = pVideoDescArray[i]->GetUnitId();
											Entry.acceleratorId    = pVideoDescArray[i]->GetAcceleratorId();
											Entry.portId           = pVideoDescArray[i]->GetFirstPortId();
											Entry.channelId        = 0;

											STATUS write_stat = AddEntryToSharedMemory(Entry); // pConnToCardMngr->Add(Entry);
											numberOfEntries  += ConnToCardEntryDump(msg, Entry);

											if (write_stat == STATUS_OK)
											{
												pResult->allocatedRrcs[numRsrc].connectionId    = pVideoDescArray[i]->GetConnId();
												pResult->allocatedRrcs[numRsrc].logicalRsrcType = pVideoDescArray[i]->GetType();
												numRsrc++;
											}
										}
									}
								}
							}

							if (pVideoDescArray != NULL)
							{
								for (int i = 0; i < TOTAL_MAX_VIDEO_RESOURCES; i++)
									PDELETE(pVideoDescArray[i]);

								delete[] pVideoDescArray;
								pVideoDescArray = NULL;
							}

							PDELETE(bestAlloction.m_pIsdn_Params_Response);
						}
					}

					// shared memory's entities
					if (status == STATUS_OK)
					{
						if (pConnToCardMngr)
						{
							ConnToCardTableEntry EntrySrc;
							// while
							if (connIdAudEnc)
							{
								status = pConnToCardMngr->Get(connIdAudEnc, EntrySrc);
								if (status == STATUS_OK)
								{
									EntrySrc.rsrc_conf_id = rsrcConfTrgId;
									status = pConnToCardMngr->Update(EntrySrc);
								}
							}

							if (status == STATUS_OK && connIdAudDec)
							{
								status = pConnToCardMngr->Get(connIdAudDec, EntrySrc);
								if (status == STATUS_OK)
								{
									EntrySrc.rsrc_conf_id = rsrcConfTrgId;
									status = pConnToCardMngr->Update(EntrySrc);
								}
							}

							if (status == STATUS_OK && connIdRTPOrMux)
							{
								status = pConnToCardMngr->Get(connIdRTPOrMux, EntrySrc);
								if (status == STATUS_OK)
								{
									EntrySrc.rsrc_conf_id = rsrcConfTrgId;
									status = pConnToCardMngr->Update(EntrySrc);
								}
							}

							// for VSW ENC
							if (status == STATUS_OK && pTrgEncDescVsw && eVideo_party_type_none != ((CPartyRsrc*)pPartyRsrc)->GetVideoPartyType())
							{
								ConnToCardTableEntry EntryEncVSW;
								EntryEncVSW.rsrc_conf_id     = rsrcConfTrgId;
								EntryEncVSW.rsrc_party_id    = rsrcPartyId;
								EntryEncVSW.m_id             = pTrgEncDescVsw->GetConnId();
								EntryEncVSW.rsrcType         = eLogical_video_encoder;
								EntryEncVSW.physicalRsrcType = ePhysical_video_encoder;
								EntryEncVSW.boxId            = pTrgEncDescVsw->GetBoxId();
								EntryEncVSW.boardId          = pTrgEncDescVsw->GetBoardId();
								EntryEncVSW.subBoardId       = pTrgEncDescVsw->GetSubBoardId();
								EntryEncVSW.unitId           = pTrgEncDescVsw->GetUnitId();
								EntryEncVSW.acceleratorId    = pTrgEncDescVsw->GetAcceleratorId();
								EntryEncVSW.portId           = pTrgEncDescVsw->GetFirstPortId();

								STATUS write_stat = AddEntryToSharedMemory(EntryEncVSW); // pConnToCardMngr->Add(EntryEncVSW);
								numberOfEntries += ConnToCardEntryDump(msg, EntryEncVSW);

								if (STATUS_OK == write_stat)
								{
									pResult->allocatedRrcs[numRsrc].connectionId    = pTrgEncDescVsw->GetConnId();
									pResult->allocatedRrcs[numRsrc].logicalRsrcType = eLogical_video_encoder;
									numRsrc++;
								}
							}

							// for VSW DEC
							if (status == STATUS_OK && pTrgDecDescVsw && eVideo_party_type_none != ((CPartyRsrc*)pPartyRsrc)->GetVideoPartyType())
							{
								ConnToCardTableEntry EntryDecVSW;
								EntryDecVSW.rsrc_conf_id     = rsrcConfTrgId;
								EntryDecVSW.rsrc_party_id    = rsrcPartyId;
								EntryDecVSW.m_id             = pTrgDecDescVsw->GetConnId();
								EntryDecVSW.rsrcType         = eLogical_video_decoder;
								EntryDecVSW.physicalRsrcType = ePhysical_video_decoder;
								EntryDecVSW.boxId            = pTrgDecDescVsw->GetBoxId();
								EntryDecVSW.boardId          = pTrgDecDescVsw->GetBoardId();
								EntryDecVSW.subBoardId       = pTrgDecDescVsw->GetSubBoardId();
								EntryDecVSW.unitId           = pTrgDecDescVsw->GetUnitId();
								EntryDecVSW.acceleratorId    = pTrgDecDescVsw->GetAcceleratorId();
								EntryDecVSW.portId           = pTrgDecDescVsw->GetFirstPortId();

								STATUS write_stat = AddEntryToSharedMemory(EntryDecVSW); // pConnToCardMngr->Add(EntryDecVSW);
								numberOfEntries += ConnToCardEntryDump(msg, EntryDecVSW);

								if (write_stat == STATUS_OK)
								{
									pResult->allocatedRrcs[numRsrc].connectionId    = pTrgDecDescVsw->GetConnId();
									pResult->allocatedRrcs[numRsrc].logicalRsrcType = eLogical_video_decoder;
									numRsrc++;
								}
							}
							// end VSW
						}
						else
						{
							PASSERT(1);
							status = STATUS_FAIL;
						}
					}

					POBJDELETE(pTrgEncDescVsw);
					POBJDELETE(pTrgDecDescVsw);

					if (numberOfEntries)
						ConnToCardEntryDumpFooter(msg);
				}

				POBJDELETE(pPartyTrg);
			}

			if (status == STATUS_OK)
			{
				status = pSrcConfRsrc->RemoveParty(source_monitor_party_id);
				PASSERT(status);
			}
		}
	}

	if (STATUS_OK != status)
	{
		PTRACE2INT(eLevelInfoNormal, "CRsrcAlloc::EndPartyMove2C : failed with status = ", status);
		status = STATUS_END_RSRC_PARTY_MOVE_FAIL;
	}

	pResult->numRsrcs = numRsrc;
	return status;
}

////////////////////////////////////////////////////////////////////////////
void CRsrcAlloc::AllocatePcm(ALLOC_PCM_RSRC_REQ_PARAMS_S* pParam, ALLOC_PCM_RSRC_IND_PARAMS_S* pResult)
{
	DWORD rsrcConfId = pParam->rsrc_conf_id;
	DWORD rsrcPartyId = pParam->rsrc_party_id;

	pResult->status = STATUS_FAIL;
	pResult->rsrc_conf_id = rsrcConfId;
	pResult->rsrc_party_id = rsrcPartyId;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN(pSystemResources == NULL);

	TRACEINTO << " RsrcAlloc::AllocatePcm  for rsrcConfId = " << rsrcConfId << ", rsrc_party_id = " << rsrcPartyId;

	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	CConfRsrc* pConfRsrc = (CConfRsrc*)(pConfRsrcDB ? pConfRsrcDB->GetConfRsrcByRsrcConfId(rsrcConfId) : NULL);

	const CPartyRsrc* pPartyRsrc = pConfRsrc ? pConfRsrc->GetPartyRsrcByRsrcPartyId(rsrcPartyId) : NULL;
	if (pPartyRsrc == NULL) //party doesn't exist, error
	{
		pResult->status = STATUS_PARTY_NOT_EXISTS;
		PASSERT_AND_RETURN(STATUS_PARTY_NOT_EXISTS);
	}

	if (pSystemResources->IsPcmMenuIdExist(rsrcPartyId))
	{
		PASSERT_AND_RETURN(STATUS_FAIL);
	}

	DWORD connIdPcmMenu = pSystemResources->AllocateConnId();
	if (0 == connIdPcmMenu)
	{
		pSystemResources->DeAllocateConnId(connIdPcmMenu);
		PASSERT_AND_RETURN(STATUS_FAIL);
	}
	DWORD pcmMenuId = DUMMY_CONNECTION_ID;
	DWORD boardId = 0;

	for (WORD i = 0; i < BOARDS_NUM; i++)
	{
		DWORD currentBoardId = i + 1; //board num is 1-based
		CBoard* pBoard = pSystemResources->GetBoard(currentBoardId);
		if (pBoard && pSystemResources->IsBoardIdExists(currentBoardId))
		{
			pcmMenuId = pBoard->AllocatePcmMenuId(rsrcPartyId);
			if (DUMMY_CONNECTION_ID == pcmMenuId)
				continue;
			else
			{
				boardId = currentBoardId;
				break;
			}
		}

	}
	if (pcmMenuId != DUMMY_CONNECTION_ID)
	{
		CRsrcDesc* pPcmMenuDesc = new CRsrcDesc(connIdPcmMenu, eLogical_PCM_manager, rsrcConfId, rsrcPartyId);
		pPcmMenuDesc->SetBoardId(boardId);

		PASSERT(pConfRsrc->AddDesc(pPcmMenuDesc));

		CConnToCardManager* pConnToCardMngr = CHelperFuncs::GetConnToCardManager();
		if (pConnToCardMngr) // save to shared memory
		{
			ConnToCardTableEntry Entry;

			Entry.rsrc_conf_id = rsrcConfId;
			Entry.rsrc_party_id = rsrcPartyId;

			Entry.m_id = connIdPcmMenu;
			Entry.rsrcType = eLogical_PCM_manager;
			Entry.physicalRsrcType = ePhysical_res_none;

			Entry.boardId = boardId;
			Entry.boxId = 1;
			Entry.subBoardId = 1;
			Entry.unitId = 0;

			STATUS write_stat = AddEntryToSharedMemory(Entry); // pConnToCardMngr->Add(Entry);

			std::ostringstream msg;
			Entry.Dump(msg);
			TRACEINTO << " RsrcAlloc::AllocatePcm - Status:" << write_stat << "\n" << msg.str().c_str();

			if (write_stat == STATUS_OK) //succeeded to write in conn-to-card table
			{
				pResult->allocatedPcmRsrc.connectionId = connIdPcmMenu;
				pResult->allocatedPcmRsrc.logicalRsrcType = eLogical_PCM_manager;
				pResult->pcmMenuId = pcmMenuId;
				pResult->status = STATUS_OK;
			}
			else
			{
				TRACEINTO << " RsrcAlloc::AllocatePcm : not succeeded to write in shared memory - need to deallocate";
				DeAllocatePcm(pConfRsrc, rsrcPartyId);
				POBJDELETE(pPcmMenuDesc);
				return;
			}
		}
		else
			PASSERT(100);

		POBJDELETE(pPcmMenuDesc);
	}
}

////////////////////////////////////////////////////////////////////////////
void CRsrcAlloc::DeAllocatePcm(ALLOC_PCM_RSRC_REQ_PARAMS_S* pParam, DEALLOC_PARTY_IND_PARAMS_S* pResult)
{
	DWORD rsrcConfId = pParam->rsrc_conf_id;
	DWORD rsrcPartyId = pParam->rsrc_party_id;

	pResult->status = STATUS_FAIL;

	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();

	CConfRsrc* pConfRsrc = (CConfRsrc*)(pConfRsrcDB ? pConfRsrcDB->GetConfRsrcByRsrcConfId(rsrcConfId) : NULL);

	const CPartyRsrc* pPartyRsrc = pConfRsrc ? pConfRsrc->GetPartyRsrcByRsrcPartyId(rsrcPartyId) : NULL;
	if (pPartyRsrc == NULL) //party doesn't exist, error
	{
		TRACEINTO << " RsrcAlloc::DeAllocatePcm - Participant doesn't exist: rsrc_conf_id = " << rsrcConfId << ", rsrc_party_id = " << rsrcPartyId;
		pResult->status = STATUS_PARTY_NOT_EXISTS;
		return; //PASSERT_AND_RETURN(STATUS_PARTY_NOT_EXISTS);
	}

	TRACEINTO << " RsrcAlloc::DeAllocatePcm  for rsrc_conf_id = " << rsrcConfId << ", rsrc_party_id = " << rsrcPartyId;

	DeAllocatePcm(pConfRsrc, rsrcPartyId);

	pResult->status = STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
void CRsrcAlloc::DeAllocatePcm(CConfRsrc* pConfRsrc, DWORD rsrcPartyId)
{
	if (!IsValidPObjectPtr(pConfRsrc))
		return;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN(pSystemResources == NULL);

	CRsrcDesc* pPcmMenuDesc = (CRsrcDesc*)pConfRsrc->GetDesc(rsrcPartyId, eLogical_PCM_manager);
	PASSERT_AND_RETURN(pPcmMenuDesc == NULL);

	WORD connIdPcmMenu = pPcmMenuDesc->GetConnId();
	pSystemResources->DeAllocateConnId(connIdPcmMenu);

	WORD boardId = pPcmMenuDesc->GetBoardId();
	CBoard* pBoard = pSystemResources->GetBoard(boardId);
	if (pBoard)
		pBoard->DeallocatePcmMenuId(rsrcPartyId);

	PASSERT(pConfRsrc->RemoveDesc(rsrcPartyId, eLogical_PCM_manager));

	CConnToCardManager* pConnToCardMngr = CHelperFuncs::GetConnToCardManager();
	if (pConnToCardMngr)
		PASSERT(pConnToCardMngr->Remove(connIdPcmMenu));
}

////////////////////////////////////////////////////////////////////////////
bool CRsrcAlloc::Is1080p60SplitEncoderAllocateOnUnit(BYTE board_id, BYTE unit_id) const
{
	TRACEINTO << " CRsrcAlloc::Is1080p60SplitEncoderAllocateOnUnit board_id = " << (WORD)board_id << " , unit_id = " << unit_id;

	PTRACE(eLevelInfoNormal, "CRsrcAlloc::Is1080p60SplitEncoderAllocateOnUnit");

	bool is1080p60encoder = false;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	if (!pSystemResources)
	{
		PTRACE(eLevelError, "CRsrcAlloc::Is1080p60SplitEncoderAllocateOnUnit - pSystemResources is NULL");
		return false;
	}

	CBoard* pBoard = pSystemResources->GetBoard(board_id);
	if (!pBoard)
	{
		PTRACE2INT(eLevelError, "CRsrcAlloc::Is1080p60SplitEncoderAllocateOnUnit - pBoard is NULL, board_id = ", (DWORD )board_id);
		return false;
	}

	CMediaUnitsList* pMediaUnitslist = pBoard->GetMediaUnitsList();
	if (!pMediaUnitslist)
	{
		PTRACE(eLevelError, "CRsrcAlloc::Is1080p60SplitEncoderAllocateOnUnit -pMediaUnitslist is NULL");
		return false;
	}

	for (std::set<CUnitMFA>::iterator _ii = pMediaUnitslist->begin(); _ii != pMediaUnitslist->end(); ++_ii)
	{
		if (_ii->GetUnitId() == unit_id) // found unt
		{
			TRACEINTO << " CRsrcAlloc::Is1080p60SplitEncoderAllocateOnUnit found unit_id " << unit_id;
			if (eUnitType_Video == _ii->GetUnitType())
			{ // and it is video unit
				is1080p60encoder = _ii->Is1080p60SplitEncoderAllocated();
			}
		}
	}
	return is1080p60encoder;
}

////////////////////////////////////////////////////////////////////////////
void CRsrcAlloc::UnitRecovery(UNIT_RECOVERY_S* pParams, RECOVERY_REPLACEMENT_UNIT_S* pResult)
{
	BYTE box_id       = pParams->unit_recover.box_id;
	BYTE unit_id      = pParams->unit_recover.unit_id;
	BYTE board_id     = pParams->unit_recover.board_id;
	BYTE sub_board_id = pParams->unit_recover.sub_board_id;

	pResult->unit_recover.box_id            = box_id;
	pResult->unit_recover.board_id          = board_id;
	pResult->unit_recover.sub_board_id      = sub_board_id;
	pResult->unit_recover.unit_id           = unit_id;
	pResult->unit_replacement.box_id        = box_id;
	pResult->unit_replacement.board_id      = board_id;
	pResult->unit_replacement.sub_board_id  = sub_board_id;
	pResult->unit_replacement.unit_id       = 0;
	pResult->status                         = STATUS_FAIL;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	FPASSERTMSG_AND_RETURN(!pSystemResources, "CRsrcAlloc::UnitRecovery - Failed, 'pSystemResources' is NULL");

	CBoard* pBoard = pSystemResources->GetBoard(board_id);
	PASSERTSTREAM_AND_RETURN(!pBoard, "CRsrcAlloc::UnitRecovery - Failed, invalid board [id:" << (WORD)board_id << "]");

	WORD artReservedRecovery = pBoard->GetRecoveryReservedArtUnitId();
	WORD audioControllerUnitId = pBoard->GetAudioControllerUnitId();

	CMediaUnitsList* pMediaUnitslist = pBoard->GetMediaUnitsList();
	eUnitType unitType = eUnitType_Generic;
	bool isFound = false;

	eCardType cardType = pBoard->GetCardType();

	// check if the unit that crash itself is empty
	// if yes (isFound) - we don't need replacement
	WORD recovered_physical_unit_id = 0;
	for (std::set<CUnitMFA>::iterator _ii = pMediaUnitslist->begin(); _ii != pMediaUnitslist->end(); ++_ii)
	{
		if (_ii->GetUnitId() == unit_id)
		{
			recovered_physical_unit_id = _ii->GetPhysicalUnitId();
			unitType = _ii->GetUnitType();
			if (_ii->IsEmptyUnit())             // if the unit is not occupied then no need to look for a replacement
			{
				isFound = true;
				pResult->unit_replacement.unit_id = unit_id;  // we just return STATUS_FAIL and Embedded reset the unit by itself (by Yuval T.)
				TRACEINTO << "CRsrcAlloc::UnitRecovery - the unit [id:" << (WORD)unit_id << "] is not occupied, so Embedded reset the unit by itself";
			}
			break;
		}
	}

	if (CHelperFuncs::IsBreeze(cardType))
	{
		// check if the unit that crash is art units 9 or 10
		// if yes (isFound) - we can't use replacement since units 9 and 10 sit on the same bus
		if (eUnitType_Art == unitType)
		{
			// units 9 , 10 can't use swap recovery because they are on the same bus vngr-22438
			if (!CCardResourceConfigBreeze::CanUnitBeArtSwapRecovery(unit_id))
			{
				TRACEINTO << " CRsrcAlloc::UnitRecovery : the unit " << (WORD)unit_id << " not using swap recovery ";
				isFound = true;
			}
		}

		if (eUnitType_Video == unitType)
		{
			if (Is1080p60SplitEncoderAllocateOnUnit(board_id, unit_id))
			{
				TRACEINTO << " CRsrcAlloc::UnitRecovery : the unit " << (WORD)unit_id << " has 1080p60 split encoder allocated, and will perform self recovery ";
				isFound = true;
			}
		}
	}

	// look for unit replacement
	WORD unit_replacement_logical_unit_id = 0;
	WORD unit_replacement_physical_unit_id = 0;

	if (!isFound)
	{
		TRACEINTO << " CRsrcAlloc::UnitRecovery : units list (before)";
		PrintUnitsList(board_id, unitType);

		WORD art_replacement_logical_unit_id = 0;
		WORD art_replacement_physical_unit_id = 0;

		for (std::set<CUnitMFA>::iterator _ii = pMediaUnitslist->begin(); _ii != pMediaUnitslist->end(); ++_ii)
		{
			WORD iter_logical_unit_id = _ii->GetUnitId();
			WORD iter_physical_unit_id = _ii->GetPhysicalUnitId();

			// skip: not free units, disabled units, units not from the same type (art/video)
			// if unit itself is empty - we won't be in that loop (isFound)
			if (_ii->GetUnitType() != unitType || _ii->GetIsEnabled() != TRUE || !_ii->IsEmptyUnit())
				continue;

			if (CHelperFuncs::IsBreeze(cardType))
			{
				if (eUnitType_Art == unitType && !CCardResourceConfigBreeze::CanUnitBeArtSwapRecovery(iter_physical_unit_id))
				{
					// units 9 , 10 can't use swap recovery because they are on the same bus
					continue;
				}

				if ((eUnitType_Video == unitType) && ((CCardResourceConfigBreeze::IsTurboVideoUnit(iter_physical_unit_id)) != (CCardResourceConfigBreeze::IsTurboVideoUnit(recovered_physical_unit_id))))
				{
					// VNGFE-6283 & VNGFE-6294 - can't swap between turbo and regular DSPs.
					continue;
				}
			}

			if (eUnitType_Art == unitType && _ii->IsRecoveryReservedUnit())
			{
				if (art_replacement_logical_unit_id == 0 && art_replacement_physical_unit_id == 0 && iter_logical_unit_id != audioControllerUnitId)  //we can't use eUnitType_Art_Control (by Yuval T.)
				{
					art_replacement_logical_unit_id = iter_logical_unit_id;
					art_replacement_physical_unit_id = iter_physical_unit_id;
					continue;
				}
			}

			// replacement found
			unit_replacement_logical_unit_id = iter_logical_unit_id;
			unit_replacement_physical_unit_id = iter_physical_unit_id;

			break;
		}

		// use Art recovery reserved unit only if not found other free art
		if (eUnitType_Art == unitType && (unit_replacement_logical_unit_id == 0 && unit_replacement_physical_unit_id == 0) && (art_replacement_logical_unit_id != 0 || art_replacement_physical_unit_id != 0))
		{
			unit_replacement_logical_unit_id = art_replacement_logical_unit_id;
			unit_replacement_physical_unit_id = art_replacement_physical_unit_id;
			TRACEINTO << "CRsrcAlloc::UnitRecovery - recovery unit [id:" << (WORD)unit_id << ", type:eUnitType_Art], use reserved unit [id:" << artReservedRecovery << "] other arts occupied";
		}

		// found replacement unit
		if (unit_replacement_logical_unit_id != 0 || unit_replacement_physical_unit_id != 0)
		{
			pResult->unit_replacement.unit_id = (BYTE)unit_replacement_logical_unit_id;
			pResult->status = STATUS_OK;
		}
		else
		{
			TRACEINTO << "CRsrcAlloc::UnitRecovery - Failed, replacement unit isn't found for the unit recovery unit [id:" << (WORD)unit_id << "]";
		}

		if (pResult->status == STATUS_OK)
		{
			CSmallString cstr1;
			cstr1 << "CRsrcAlloc::UnitRecovery - recovery unit [id:" << (WORD)unit_id << ", physical_id:" << recovered_physical_unit_id << ", type:" << unitType;

			if (unitType == eUnitType_Art)
				cstr1 << " eUnitType_Art";
			else if (unitType == eUnitType_Video)
				cstr1 << " eUnitType_Video";
			else
				cstr1 << " other";

			cstr1 << "], replacement unit [id:" << unit_replacement_logical_unit_id << ", physical_id:" << (WORD)unit_replacement_physical_unit_id << "]";
			TRACEINTO << cstr1.GetString();
			cstr1.Clear();

			CUnitMFA* pUnitReplace = pSystemResources->GetUnit(board_id, unit_replacement_logical_unit_id);
			PASSERTSTREAM_AND_RETURN(!pUnitReplace, "CRsrcAlloc::UnitRecovery - Failed, invalid replacement unit [id:" << (WORD)unit_id << "] on board [id:" << (WORD)board_id << "]");

			pUnitReplace->SetRecovery(TRUE, recovered_physical_unit_id);
			pUnitReplace->SetEnabled(FALSE); //replacement unit
		}
	}

	CUnitMFA* pUnitRecovery = pSystemResources->GetUnit(board_id, unit_id);
	PASSERTSTREAM_AND_RETURN(!pUnitRecovery, "CRsrcAlloc::UnitRecovery - Failed, invalid recovery unit [id:" << (WORD)unit_id << "] on board [id:" << (WORD)board_id << "]");

	WORD unitIdToReplace = (pResult->status == STATUS_OK) ? unit_replacement_physical_unit_id : recovered_physical_unit_id;
	pUnitRecovery->SetRecovery(TRUE, unitIdToReplace);  // mark as recovery unit
	pUnitRecovery->SetEnabled(FALSE);                   // mark as disabled unit

	TRACEINTO << " CRsrcAlloc::UnitRecovery : units list (after)";
	PrintUnitsList(board_id, unitType);
}

////////////////////////////////////////////////////////////////////////////
void CRsrcAlloc::PrintUnitsList(BYTE board_id, eUnitType requestedUnitType)
{
	// get media units list
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	FPASSERTMSG_AND_RETURN(!pSystemResources, "CRsrcAlloc::PrintUnitsList - Failed, 'pSystemResources' is NULL");
	CBoard* pBoard = pSystemResources->GetBoard(board_id);
	PASSERTSTREAM_AND_RETURN(!pBoard, "CRsrcAlloc::PrintUnitsList - Failed, invalid board [id:" << (WORD)board_id << "]");
	CMediaUnitsList* pMediaUnitslist = pBoard->GetMediaUnitsList();
	eCardType cardType = pBoard->GetCardType();

	CLargeString mstr;
	mstr << "units list for board id " << (WORD)board_id << " unit type ";
	if (requestedUnitType == eUnitType_Art)
		mstr << "eUnitType_Art";
	else if (requestedUnitType == eUnitType_Video)
		mstr << "eUnitType_Video";
	else
		mstr << "other";

	mstr << "\n";

	for (std::set<CUnitMFA>::iterator _ii = pMediaUnitslist->begin(); _ii != pMediaUnitslist->end(); ++_ii)
	{
		eUnitType unitType = _ii->GetUnitType();
		if (unitType == requestedUnitType)
		{
			WORD physicalUnitId = _ii->GetPhysicalUnitId();
			WORD logicalUnitId = _ii->GetUnitId();
			float freeCapacityAccelerator0 = 0;
			float freeCapacityAccelerator1 = 0;
			float freeCapacityAccelerator2 = 0;

			if (unitType == eUnitType_Video && CHelperFuncs::IsMpmRx(cardType))
			{
				freeCapacityAccelerator0 = _ii->GetFreeCapacity(0);
				freeCapacityAccelerator1 = _ii->GetFreeCapacity(1);
				freeCapacityAccelerator2 = _ii->GetFreeCapacity(2);
				mstr << "physicalUnitId = " << physicalUnitId << " , logicalUnitId = " << logicalUnitId
					<< " , freeCapacityAccelerator0 = " << freeCapacityAccelerator0
					<< " , freeCapacityAccelerator1 = " << freeCapacityAccelerator1
					<< " , freeCapacityAccelerator2 = " << freeCapacityAccelerator2 << "\n";
			}
			else
			{
				freeCapacityAccelerator0 = _ii->GetFreeCapacity();
				mstr << "physicalUnitId = " << physicalUnitId << " , logicalUnitId = " << logicalUnitId
					<< " , freeCapacity = " << freeCapacityAccelerator0 << "\n";
			}
		}
	}
	PTRACE2(eLevelInfoNormal, "CRsrcAlloc::PrintUnitsList - ", mstr.GetString());
}

////////////////////////////////////////////////////////////////////////////
void CRsrcAlloc::UnitRecoveryEnd(UNIT_RECOVERY_S* pParams)
{
	BYTE box_id = pParams->unit_recover.box_id;
	BYTE unit_id = pParams->unit_recover.unit_id;
	BYTE board_id = pParams->unit_recover.board_id;
	BYTE sub_board_id = pParams->unit_recover.sub_board_id;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN(pSystemResources == NULL);
	CUnitMFA* pUnitRecovery = pSystemResources->GetUnit(board_id, unit_id);
	if (pUnitRecovery)
	{
		pUnitRecovery->SetRecovery(FALSE, 0);
		pUnitRecovery->SetEnabled(TRUE);                   //recovery unit
	}
	else
		TRACEINTO << " RsrcAlloc::UnitRecoveryEnd : wrong board id = " << board_id << " or unit id = " << unit_id;
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrcAlloc::SendKillPortsCop(CConfRsrc* pConfRsrc)
{
	STATUS ret_stat = STATUS_OK;
	// get and print conference resource descriptors array
	CRsrcDesc** pRsrcDescArray = NULL;
	int num_rsrc = pConfRsrc->GetDescArray(pRsrcDescArray);

	PhysicalPortDesc portDesc;
	eLogicalResourceTypes logicType = eLogical_res_none;
	eResourceTypes physicalType = ePhysical_res_none;

	for (int i = 0; i < num_rsrc; i++)
	{
		if (!pRsrcDescArray[i])
		{
			continue;
		}
		logicType = pRsrcDescArray[i]->GetType();
		if (CHelperFuncs::IsLogicalVideoEncoderType(logicType))
		{
			physicalType = ePhysical_video_encoder;
		}
		else if (CHelperFuncs::IsLogicalVideoDecoderType(logicType))
		{
			physicalType = ePhysical_video_decoder;
		}

		if(ePhysical_video_decoder == physicalType || ePhysical_video_encoder == physicalType)
		{
			portDesc.m_boxId         = pRsrcDescArray[i]->GetBoxId();
			portDesc.m_boardId       = pRsrcDescArray[i]->GetBoardId();
			portDesc.m_subBoardId    = pRsrcDescArray[i]->GetSubBoardId();
			portDesc.m_unitId        = pRsrcDescArray[i]->GetUnitId();
			portDesc.m_portId        = pRsrcDescArray[i]->GetFirstPortId();
			portDesc.m_acceleratorId = pRsrcDescArray[i]->GetAcceleratorId();

			STATUS kill_stat = SendKillPortRequest( &portDesc, physicalType);
			PASSERT( kill_stat );
			if(kill_stat && ret_stat == STATUS_OK)
			{
				ret_stat = kill_stat;
			}
		}
	}

	delete[] pRsrcDescArray;

	return ret_stat;
}

////////////////////////////////////////////////////////////////////////////
void CRsrcAlloc::AllocateTIPContent(ALLOC_PARTY_REQ_PARAMS_S* pParam, ALLOC_PARTY_IND_PARAMS_S* pResult)
{
	STATUS status = STATUS_OK;
	pResult->allocIndBase.status = status;

	DWORD monitor_conf_id = pParam->monitor_conf_id;
	DWORD monitor_party_id = pParam->monitor_party_id;
	eVideoPartyType videoPartyType = pParam->videoPartyType;

	//check validity of all params
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	CConnToCardManager* pConnToCardMngr = CHelperFuncs::GetConnToCardManager();

	if (!pConfRsrcDB || !pSystemResources || !pConnToCardMngr || pParam->tipPartyType != eTipVideoAux)
	{
		PASSERT(1);
		pResult->allocIndBase.status = STATUS_FAIL;
		return;
	}

	CConfRsrc* pConf = (CConfRsrc*)pConfRsrcDB->GetConfRsrc(monitor_conf_id);
	if (!pConf)
	{
		PASSERT(1);
		PTRACE2INT(eLevelError, "CRsrcAlloc::AllocateTIPContent : non-existing conf, monitorId=", monitor_conf_id);
		pResult->allocIndBase.status = STATUS_CONFERENCE_NOT_EXISTS;
		return;
	}

	DWORD rsrcConfId = pConfRsrcDB->MonitorToRsrcConfId(monitor_conf_id);
	DWORD rsrcPartyId = pConfRsrcDB->MonitorToRsrcPartyId(monitor_conf_id, monitor_party_id);

	CPartyRsrc* pParty = (CPartyRsrc*)pConf->GetPartyRsrcByRsrcPartyId(rsrcPartyId);
	if (!pParty)
	{
		PASSERT(1);
		PTRACE2INT(eLevelError, "CRsrcAlloc::AllocateTIPContent : non-existing party, monitorId=", monitor_party_id);
		pResult->allocIndBase.status = STATUS_PARTY_NOT_EXISTS;
		return;
	}
	PartyDataStruct partyData;
	memset(&partyData, 0, sizeof(partyData));
	partyData.m_monitor_conf_id = monitor_conf_id;
	partyData.m_networkPartyType = pParam->networkPartyType;
	partyData.m_videoPartyType = videoPartyType;
	partyData.m_allowReconfiguration = pParam->isWaitForRsrcAndAskAgain;
	partyData.m_subServiceId = pParam->subServiceId;
	partyData.m_room_id = pParam->room_id;
	partyData.m_partyTypeTIP = pParam->tipPartyType;
	partyData.m_confMediaType = pParam->confMediaType;

	WORD boardIdTIP = 0xFFFF, unitIdTIP = 0;

	status = pConf->GetBoardUnitIdByRoomIdTIP(pParam->room_id, boardIdTIP, unitIdTIP);
	DBGPASSERT(status != STATUS_OK);
	TRACEINTO << " CRsrcAlloc::AllocateTIPContent :  monitor_party_id=" << monitor_party_id << ", room_id=" << pParam->room_id << " ===> boardIdTIP=" << boardIdTIP << ", unitIdTIP=" << unitIdTIP;

	partyData.m_reqBoardId = boardIdTIP;
	partyData.m_reqArtUnitId = unitIdTIP;

	DWORD connIdRTP = pSystemResources->AllocateConnId();
	if (connIdRTP == 0)
	{
		PASSERT(1);
		pResult->allocIndBase.status = STATUS_INSUFFICIENT_RSRC_CONNECTION_ID;
		return;
	}

	PhysicalPortDesc* pPortDesc = new PhysicalPortDesc;
	status = pSystemResources->AllocateART( rsrcConfId,
											rsrcPartyId,
											pParam->artCapacity,
											ePhysical_art,
											pParam->serviceId,
											pParam->subServiceId,
											pPortDesc,
											partyData,
											pParam->isIceParty,
											pParam->isBFCP,
											boardIdTIP);
	if( status != STATUS_OK )
	{
		pSystemResources->DeAllocateConnId(connIdRTP);
		POBJDELETE(pPortDesc);
		PTRACE2INT(eLevelInfoNormal, "CRsrcAlloc::AllocateTIPContent failed : status=", status);
		pResult->allocIndBase.status = status;
		return;
	}
	CRsrcDesc* pRtp = new CRsrcDesc(connIdRTP, eLogical_content_rtp, rsrcConfId, rsrcPartyId, pPortDesc->m_boxId, pPortDesc->m_boardId, pPortDesc->m_subBoardId, pPortDesc->m_unitId, pPortDesc->m_acceleratorId, pPortDesc->m_portId);
	PASSERT(pConf->AddDesc(pRtp));

	ConnToCardTableEntry Entry;
	Entry.m_id = connIdRTP;
	Entry.rsrcType = eLogical_content_rtp;
	Entry.physicalRsrcType = ePhysical_art;
	Entry.boxId = pRtp->GetBoxId();
	Entry.boardId = pRtp->GetBoardId();
	Entry.subBoardId = pRtp->GetSubBoardId();
	Entry.unitId = pRtp->GetUnitId();
	Entry.portId = pRtp->GetFirstPortId();
	Entry.acceleratorId = pRtp->GetAcceleratorId();
	Entry.channelId = 0;

	STATUS write_stat = AddEntryToSharedMemory(Entry); // pConnToCardMngr->Add(Entry);

	std::ostringstream msg;
	Entry.Dump(msg);
	TRACEINTO << " RsrcAlloc::AllocateTIPContent - Status:" << write_stat << "\n" << msg.str().c_str();

	WORD numRsrc = 0;
	if (STATUS_OK == write_stat) //succeeded to write in conn-to-card table
	{
		pResult->allocIndBase.allocatedRrcs[numRsrc].connectionId = connIdRTP;
		pResult->allocIndBase.allocatedRrcs[numRsrc].logicalRsrcType = eLogical_content_rtp;
		numRsrc++;
	}

	//*** fill in Result for output
	pResult->allocIndBase.rsrc_conf_id = rsrcConfId;
	pResult->allocIndBase.rsrc_party_id = rsrcPartyId;
	pResult->allocIndBase.videoPartyType = videoPartyType;
	pResult->allocIndBase.networkPartyType = pParam->networkPartyType;
	pResult->allocIndBase.numRsrcs = numRsrc;
	pResult->allocIndBase.room_id = pParam->room_id;

	POBJDELETE(pRtp);
	POBJDELETE(pPortDesc);
}

////////////////////////////////////////////////////////////////////////////
void CRsrcAlloc::ConnToCardEntryDumpHeader(std::ostringstream& msg, char* headerText, DWORD confId, DWORD partyId, DWORD roomId)
{
	msg << headerText << ", ConfId:" << (int)confId << ", PartyId:" << (int)partyId << ", RoomId:" << (int)roomId << endl;
	msg << " -----+-----+-------+----------+------+-------------+------+---------+---------------------------+-------------------------------------------------+--------------" << endl;
	msg << " Conn | Box | Board | SubBoard | Unit | Accelerator | Port | Channel | PhysicalRsrcType          | RsrcType                                        | RsrcCntlType " << endl;
	msg << " -----+-----+-------+----------+------+-------------+------+---------+---------------------------+-------------------------------------------------+--------------" << endl;
}

////////////////////////////////////////////////////////////////////////////
void CRsrcAlloc::ConnToCardEntryDumpFooter(std::ostringstream& msg)
{
	msg << " -----+-----+-------+----------+------+-------------+------+---------+---------------------------+-------------------------------------------------+--------------";
	FPTRACE(eLevelInfoNormal, msg.str().c_str());
}

////////////////////////////////////////////////////////////////////////////
int CRsrcAlloc::ConnToCardEntryDump(std::ostringstream& msg, ConnToCardTableEntry& Entry)
{
	msg.precision(0);
	msg
		<< " " << setw( 4) << right << (int)Entry.m_id     << " |"
		<< " " << setw( 3) << right << Entry.boxId         << " |"
		<< " " << setw( 5) << right << Entry.boardId       << " |"
		<< " " << setw( 8) << right << Entry.subBoardId    << " |"
		<< " " << setw( 4) << right << Entry.unitId        << " |"
		<< " " << setw(11) << right << Entry.acceleratorId << " |"
		<< " " << setw( 4) << right << Entry.portId        << " |"
		<< " " << setw( 7) << right << Entry.channelId     << " |"
		<< " " << setw(25) << left  << ::ResourceTypeToString(Entry.physicalRsrcType) << " |"
		<< " " << setw(47) << left  << Entry.rsrcType      << " |"
		<< " " <<             left  << ::RsrcCntlTypeToString(Entry.rsrcCntlType) << endl;
	return 1;
}

////////////////////////////////////////////////////////////////////////////
int CRsrcAlloc::AllocateRelayPartyResources(ALLOC_PARTY_REQ_PARAMS_S* pParam,
                                            CRsrcDesc* pAllocatedRtpDesc,
                                            ConnToCardTableEntry& cmEntry,
                                            std::ostringstream& msg,
                                            ALLOC_PARTY_IND_PARAMS_S* pResult)        //OLGA - Soft MCU
{
	PTRACE2INT(eLevelInfoNormal, "CRsrcAlloc::AllocateRelayPartyResources rsrcPartyId=", pParam->party_id);
	DWORD monitor_conf_id = pParam->monitor_conf_id;

	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	CConfRsrc* pConfRsrc = (CConfRsrc*)(pConfRsrcDB ? pConfRsrcDB->GetConfRsrc(pParam->monitor_conf_id) : NULL);
	CPartyRsrc* pParty = (CPartyRsrc*)(pConfRsrc ? pConfRsrc->GetPartyRsrcByRsrcPartyId(pParam->party_id) : NULL);
	if (!pSystemResources || !pConfRsrcDB || !pConfRsrc || !pParty)
		return 0;
	DWORD rsrcConfId = pConfRsrcDB->MonitorToRsrcConfId(pParam->monitor_conf_id);
	eVideoPartyType videoPartyType = pParty->GetVideoPartyType();

	// Allocate SSRC : 32 bits of SSRC are split as following
	// DMA ID - 6 bits, MCU ID - 10 bits, Stream ID - 16 bits
	// we split the Stream ID to consist of <SRC ID, Payload ID>
	// where the Payload ID is the 6 bits (by Avishay H.)

	//unsigned int dma_mrm_ssrc_mask = 0, dmaId = 1, mrmId = 1;
	unsigned int dmaId = 1;
	unsigned short mrmId = pConfRsrc->GetMrcMcuId();

	DWORD ssrc_party = pSystemResources->AllocateSSRCId(dmaId, mrmId);

	unsigned int result_audio = pSystemResources->GetDmaMrmMask(dmaId, mrmId, ssrc_party, AUDIO_PAYLOAD_TYPE);    // dma_mrm_ssrc_mask | AUDIO_PAYLOAD_TYPE;

	pResult->svcParams.m_ssrcAudio = result_audio;
	pParty->m_ssrcAudio = result_audio;

	WORD numRsrcInResult = pResult->allocIndBase.numRsrcs, countRelayRsrc = 0;

	DWORD connIdRelayRTP = pSystemResources->AllocateConnId();
	if (0 == connIdRelayRTP)
		PASSERT(1);
	else
	{
		CRsrcDesc* pRtpRelayDesc = new CRsrcDesc(connIdRelayRTP, eLogical_relay_rtp, rsrcConfId, pParam->party_id, pAllocatedRtpDesc->GetBoxId(), pAllocatedRtpDesc->GetBoardId(), pAllocatedRtpDesc->GetSubBoardId(), pAllocatedRtpDesc->GetUnitId(), 0 /*accelerator id*/, pAllocatedRtpDesc->GetFirstPortId());
		PASSERT(pConfRsrc->AddDesc(pRtpRelayDesc));

		WriteRsrcDescToSharedMemory(pRtpRelayDesc, cmEntry, msg);

		pResult->allocIndBase.allocatedRrcs[numRsrcInResult + countRelayRsrc].connectionId = connIdRelayRTP;
		pResult->allocIndBase.allocatedRrcs[numRsrcInResult + countRelayRsrc].logicalRsrcType = eLogical_relay_rtp;
		countRelayRsrc++;

		POBJDELETE(pRtpRelayDesc);
	}

	if (eVOICE_session == pParam->sessionType)
		return countRelayRsrc;

	int num_video_ssrc = CHelperFuncs::IsVswRelayParty(videoPartyType) ? 1 : MAX_NUM_RECV_STREAMS_FOR_VIDEO;
	int num_content_ssrc = CHelperFuncs::IsVswRelayParty(videoPartyType) ? 1 : MAX_NUM_RECV_STREAMS_FOR_CONTENT;

	for (int i = 0; i < num_video_ssrc; i++)
	{
		unsigned int result_video = pSystemResources->GetDmaMrmMask(dmaId, mrmId, ssrc_party, (VIDEO_PAYLOAD_TYPE + i));    //dma_mrm_ssrc_mask | (VIDEO_PAYLOAD_TYPE + i);

		pResult->svcParams.m_ssrcVideo[i] = result_video;
		pParty->m_ssrcVideo[i] = result_video;
	}
	for (int i = 0; i < num_content_ssrc; i++)
	{
		unsigned int result_content = pSystemResources->GetDmaMrmMask(dmaId, mrmId, ssrc_party, (CONTENT_PAYLOAD_TYPE + i));    //dma_mrm_ssrc_mask | (CONTENT_PAYLOAD_TYPE + i);

		pResult->svcParams.m_ssrcContent[i] = result_content;
		pParty->m_ssrcContent[i] = result_content;
	}

	// svc on RMX (joinMP) - messages to eLogical_relay_video_encoder been sent to relevant MRMp only
	// it means to CardManager of the card the conference is on (svc conf closed in 1 board)
	DWORD connIdRelayVideoEnc = pSystemResources->AllocateConnId();
	if (connIdRelayVideoEnc == 0)
	{    // rollback?
		PASSERT(1);
	}
	else
	{
		// eLogical_relay_video_encoder messages sent to the MRMp on the card manager of the relevant card
		// SVC conf must be on the same card - take board id from allocated Art
		WORD MRMpBoxId = pAllocatedRtpDesc->GetBoxId();
		WORD MRMpBoardId = pAllocatedRtpDesc->GetBoardId();
		WORD MRMpSubBoardId = pAllocatedRtpDesc->GetSubBoardId();
		WORD MRMpUnitId = 0;
		WORD MRMpPortId = 0;
		CRsrcDesc* pVideoEncoderRelayDesc = new CRsrcDesc(connIdRelayVideoEnc, eLogical_relay_video_encoder, rsrcConfId, pParam->party_id, MRMpBoxId, MRMpBoardId, MRMpSubBoardId, MRMpUnitId, 0 /*accelerator_id*/, MRMpPortId);
		PASSERT(pConfRsrc->AddDesc(pVideoEncoderRelayDesc));

		WriteRsrcDescToSharedMemory(pVideoEncoderRelayDesc, cmEntry, msg);

		pResult->allocIndBase.allocatedRrcs[numRsrcInResult + countRelayRsrc].connectionId = connIdRelayVideoEnc;
		pResult->allocIndBase.allocatedRrcs[numRsrcInResult + countRelayRsrc].logicalRsrcType = eLogical_relay_video_encoder;
		countRelayRsrc++;

		POBJDELETE(pVideoEncoderRelayDesc);
	}

	if (CHelperFuncs::IsVideoRelayParty(videoPartyType))
	{
		DWORD connIdSvcToAvcRTP = pSystemResources->AllocateConnId();
		if (0 == connIdSvcToAvcRTP)
			PASSERT(1);
		else
		{
			CRsrcDesc* pRtpRelaySvcToAvc = new CRsrcDesc(connIdSvcToAvcRTP, eLogical_relay_svc_to_avc_rtp, rsrcConfId, pParam->party_id, pAllocatedRtpDesc->GetBoxId(), pAllocatedRtpDesc->GetBoardId(), pAllocatedRtpDesc->GetSubBoardId(), pAllocatedRtpDesc->GetUnitId(), 0 /*accelerator_id*/, pAllocatedRtpDesc->GetFirstPortId());
			PASSERT((NULL == pConfRsrc) || pConfRsrc->AddDesc(pRtpRelaySvcToAvc));

			WriteRsrcDescToSharedMemory(pRtpRelaySvcToAvc, cmEntry, msg);

			pResult->allocIndBase.allocatedRrcs[numRsrcInResult + countRelayRsrc].connectionId = connIdSvcToAvcRTP;
			pResult->allocIndBase.allocatedRrcs[numRsrcInResult + countRelayRsrc].logicalRsrcType = eLogical_relay_svc_to_avc_rtp;
			countRelayRsrc++;

			POBJDELETE(pRtpRelaySvcToAvc);
		}
	}
	return countRelayRsrc;
}

////////////////////////////////////////////////////////////////////////////
int CRsrcAlloc::AllocateAvcRelayResources(ALLOC_PARTY_REQ_PARAMS_S* pParam, PartyDataStruct& partyData, CRsrcDesc* pAllocatedRtpDesc, ConnToCardTableEntry& cmEntry, std::ostringstream& msg, ALLOC_PARTY_IND_PARAMS_S* pResult) //OLGA - Soft MCU
{
	TRACEINTO << "PartyId:" << pParam->party_id;

	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	CConfRsrc* pConfRsrc = (CConfRsrc*)(pConfRsrcDB ? pConfRsrcDB->GetConfRsrc(pParam->monitor_conf_id) : NULL);
	CPartyRsrc* pParty = (CPartyRsrc*)(pConfRsrc ? pConfRsrc->GetPartyRsrcByRsrcPartyId(pParam->party_id) : NULL);
	if (!pSystemResources || !pConfRsrcDB || !pConfRsrc || !pParty)
		return 0;

	if ((eParty_Role_content_decoder == pParty->GetPartyRole()) || (eParty_Role_content_encoder == pParty->GetPartyRole()))
	{
		TRACEINTO << "No need to allocate AVC relay resource due to its party role is: " << pParty->GetPartyRole();
		return 0;
	}

	DWORD rsrcConfId = pConfRsrcDB ? pConfRsrcDB->MonitorToRsrcConfId(pParam->monitor_conf_id) : 0; // gets existing rsrcConfId from DB
	// Allocate SSRC : 32 bits of SSRC are split as following
	// DMA ID - 6 bits, MCU ID - 10 bits, Stream ID - 16 bits
	// we split the Stream ID to consist of <SRC ID, Payload ID>
	// where the Payload ID is the 6 bits (by Avishay H.)

	// unsigned int dma_mrm_ssrc_mask = 0, dmaId = 1, mrmId = 1;
	unsigned int dmaId = 1, mrmId = 1;

	DWORD ssrc_party = pSystemResources->AllocateSSRCId(dmaId, mrmId);

	unsigned int result_audio = pSystemResources->GetDmaMrmMask(dmaId, mrmId, ssrc_party, AUDIO_PAYLOAD_TYPE);    //dma_mrm_ssrc_mask | AUDIO_PAYLOAD_TYPE;
	pResult->svcParams.m_ssrcAudio = result_audio;
	pParty->m_ssrcAudio = result_audio;

	if (eVideo_party_type_none != pParty->GetVideoPartyType())
	{
		for (int i = 0; i < MAX_NUM_RECV_STREAMS_FOR_VIDEO; i++)
		{
			unsigned int result_video = pSystemResources->GetDmaMrmMask(dmaId, mrmId, ssrc_party, (VIDEO_PAYLOAD_TYPE + i)); //dma_mrm_ssrc_mask | (VIDEO_PAYLOAD_TYPE + i);
			pResult->svcParams.m_ssrcVideo[i] = result_video;
			pParty->m_ssrcVideo[i] = result_video;
		}
	}

	//	on RMX and softMcu there will be additional 2 ART: in RMX they'll be connected both to SAC encoder and 2 SVC encoders, for Soft MCU only 2 SVC encoders
	//	Allocate additional audio encoder eLogical_legacy_to_SAC_audio_encoder for translation (allocated on the ART #1) - only for RMX
	//  NOTE: all additional ARTs must be allocated on a board where SVC parties of this conference are allocated, because they should be connected to MRMP(CM)
	WORD numRsrcInResult = pResult->allocIndBase.numRsrcs;
	WORD reqBoardId = ((0xFFFF == partyData.m_reqBoardId) && pAllocatedRtpDesc) ? pAllocatedRtpDesc->GetBoardId() : partyData.m_reqBoardId;

	WORD countRelayRsrc = AllocateAvcARTResources(pParam, partyData, reqBoardId, cmEntry, msg, pResult);

	if (countRelayRsrc != 0)
	{
		//Allocate rtp for Rx
		DWORD connIdRelayRTP = pSystemResources->AllocateConnId();
		CRsrcDesc* pRelayAvcToSvcRtpWithAudioEnc = (CRsrcDesc*)(pConfRsrc->GetDesc(pParam->party_id, eLogical_relay_avc_to_svc_rtp_with_audio_encoder));
		if (0 == connIdRelayRTP || !pRelayAvcToSvcRtpWithAudioEnc)
			DBGPASSERT(1);
		else
		{
			CRsrcDesc* pRtpRelayDesc = new CRsrcDesc(connIdRelayRTP, eLogical_relay_rtp, rsrcConfId, pParam->party_id, pRelayAvcToSvcRtpWithAudioEnc->GetBoxId(), pRelayAvcToSvcRtpWithAudioEnc->GetBoardId(), pRelayAvcToSvcRtpWithAudioEnc->GetSubBoardId(), pRelayAvcToSvcRtpWithAudioEnc->GetUnitId(), 0 /*accelerator id*/, pRelayAvcToSvcRtpWithAudioEnc->GetFirstPortId());
			DBGPASSERT(pConfRsrc->AddDesc(pRtpRelayDesc));

			WriteRsrcDescToSharedMemory(pRtpRelayDesc, cmEntry, msg);

			pResult->allocIndBase.allocatedRrcs[numRsrcInResult + countRelayRsrc].connectionId = connIdRelayRTP;
			pResult->allocIndBase.allocatedRrcs[numRsrcInResult + countRelayRsrc].logicalRsrcType = eLogical_relay_rtp;
			countRelayRsrc++;

			POBJDELETE(pRtpRelayDesc);
		}
		pResult->allocIndBase.numRsrcs += countRelayRsrc;
	}
	return countRelayRsrc;
}

////////////////////////////////////////////////////////////////////////////
// AVC additional 2 arts: 180p + SAC encoder and 360p (SAC encoder - at RMX only)
int CRsrcAlloc::AllocateAvcARTResources(ALLOC_PARTY_REQ_PARAMS_S* pParam, PartyDataStruct& partyData, WORD reqBoardId, ConnToCardTableEntry& cmEntry, std::ostringstream& msg, ALLOC_PARTY_IND_PARAMS_S* pResult)
{
	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	CConfRsrc* pConfRsrc = (CConfRsrc*)(pConfRsrcDB ? pConfRsrcDB->GetConfRsrc(pParam->monitor_conf_id) : NULL);
	CPartyRsrc* pParty = (CPartyRsrc*)(pConfRsrc ? pConfRsrc->GetPartyRsrcByRsrcPartyId(pParam->party_id) : NULL);
	if (!pSystemResources || !pConfRsrcDB || !pConfRsrc || !pParty)
		return 0;

	DWORD rsrcConfId = pConfRsrcDB ? pConfRsrcDB->MonitorToRsrcConfId(pParam->monitor_conf_id) : 0; // gets existing rsrcConfId from DB
	WORD numRsrcInResult = pResult->allocIndBase.numRsrcs, countRelayRsrc = 0;
	if (eVideo_party_type_none != partyData.m_videoPartyType || eProductFamilyRMX == CProcessBase::GetProcess()->GetProductFamily())
	{
		//Allocating just one encoder (and art) in case AVC party is allocated with video resources<HD720
		PhysicalPortDesc rPortDesc1, rPortDesc2;

		STATUS status_alloc_art = pSystemResources->AllocateART(rsrcConfId, pParam->party_id, 0 /*artCapacity*/, ePhysical_art, pParam->serviceId, pParam->subServiceId, &rPortDesc2, partyData, pParam->isIceParty, pParam->isBFCP, reqBoardId);
		if (status_alloc_art == STATUS_OK)  // get status and roll-back if needed?
		{
			DWORD connIdART2 = pSystemResources->AllocateConnId();
			if (0 == connIdART2)
				DBGPASSERT(2);
			else
			{
				CRsrcDesc* pMixArtDesc1 = new CRsrcDesc(connIdART2, eLogical_relay_avc_to_svc_rtp_with_audio_encoder, rsrcConfId, pParam->party_id, rPortDesc2.m_boxId, rPortDesc2.m_boardId, rPortDesc2.m_subBoardId, rPortDesc2.m_unitId, 0 /*accelerator_id*/, rPortDesc2.m_portId);
				STATUS statusAddDesc1 = pConfRsrc->AddDesc(pMixArtDesc1);
				DBGPASSERT(statusAddDesc1 != STATUS_OK);

				if (STATUS_OK == statusAddDesc1)
				{
					WriteRsrcDescToSharedMemory(pMixArtDesc1, cmEntry, msg);

					pResult->allocIndBase.allocatedRrcs[numRsrcInResult + countRelayRsrc].connectionId = connIdART2;
					pResult->allocIndBase.allocatedRrcs[numRsrcInResult + countRelayRsrc].logicalRsrcType = eLogical_relay_avc_to_svc_rtp_with_audio_encoder;
					countRelayRsrc++;
				}
				else
				{
					pSystemResources->DeAllocateConnId(connIdART2);

					CRsrcDesc* pRelayAvcToSvcRtpWithAudioEnc = (CRsrcDesc*)(pConfRsrc->GetDesc(pParam->party_id, eLogical_relay_avc_to_svc_rtp_with_audio_encoder));
					if (pRelayAvcToSvcRtpWithAudioEnc)
					{
						pResult->allocIndBase.allocatedRrcs[numRsrcInResult + countRelayRsrc].connectionId = pRelayAvcToSvcRtpWithAudioEnc->GetConnId();
						pResult->allocIndBase.allocatedRrcs[numRsrcInResult + countRelayRsrc].logicalRsrcType = eLogical_relay_avc_to_svc_rtp_with_audio_encoder;
						countRelayRsrc++;
					}
				}

				if (eProductFamilyRMX == CProcessBase::GetProcess()->GetProductFamily())
				{
					DWORD connIdARTlegacyToSAC = pSystemResources->AllocateConnId();
					if (0 == connIdARTlegacyToSAC)
						DBGPASSERT(3);
					else
					{
						CRsrcDesc* pMixArtDesc3 = new CRsrcDesc(connIdARTlegacyToSAC, eLogical_legacy_to_SAC_audio_encoder, rsrcConfId, pParam->party_id, rPortDesc2.m_boxId, rPortDesc2.m_boardId, rPortDesc2.m_subBoardId, rPortDesc2.m_unitId, 0 /*accelerator_id*/, rPortDesc2.m_portId);
						STATUS statusAddDesc3 = pConfRsrc->AddDesc(pMixArtDesc3);
						DBGPASSERT(statusAddDesc3 != STATUS_OK);

						if (STATUS_OK == statusAddDesc3)
						{
							WriteRsrcDescToSharedMemory(pMixArtDesc3, cmEntry, msg);

							pResult->allocIndBase.allocatedRrcs[numRsrcInResult + countRelayRsrc].connectionId = connIdARTlegacyToSAC;
							pResult->allocIndBase.allocatedRrcs[numRsrcInResult + countRelayRsrc].logicalRsrcType = eLogical_legacy_to_SAC_audio_encoder;
							countRelayRsrc++;
						}
						else
						{
							pSystemResources->DeAllocateConnId(connIdARTlegacyToSAC);

							const CRsrcDesc* pRelayAvcLegacyToSACaudioEnc = pConfRsrc->GetDesc(pParam->party_id, eLogical_legacy_to_SAC_audio_encoder);
							if (pRelayAvcLegacyToSACaudioEnc)
							{
								pResult->allocIndBase.allocatedRrcs[numRsrcInResult + countRelayRsrc].connectionId = pRelayAvcLegacyToSACaudioEnc->GetConnId();
								pResult->allocIndBase.allocatedRrcs[numRsrcInResult + countRelayRsrc].logicalRsrcType = eLogical_legacy_to_SAC_audio_encoder;
								countRelayRsrc++;
							}
						}
						POBJDELETE(pMixArtDesc3);
					}
				}
				POBJDELETE(pMixArtDesc1);
			}

			if (partyData.m_videoPartyType != eCP_H261_CIF_equals_H264_HD1080_video_party_type && partyData.m_videoPartyType > eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type) // allocate two video encoders & ARTs when video resources > HD720
			{
				DWORD prevNumRsrc = pResult->allocIndBase.numRsrcs;
				pResult->allocIndBase.numRsrcs = prevNumRsrc + countRelayRsrc;

				STATUS status_alloc_art = AllocateAvcToSvcMixedART(pParam, partyData, reqBoardId, cmEntry, msg, pResult);

				pResult->allocIndBase.numRsrcs = prevNumRsrc;

				if (status_alloc_art != STATUS_OK)
					pResult->allocIndBase.status = status_alloc_art; //TODO?
				else
					countRelayRsrc++;
			}
		}
		else
		{
			pResult->allocIndBase.status = status_alloc_art; //TODO?
		}
	}
	return countRelayRsrc;
}

////////////////////////////////////////////////////////////////////////////
void CRsrcAlloc::WriteRsrcDescToSharedMemory(CRsrcDesc* pAllocatedRsrcDesc, ConnToCardTableEntry& cmEntry, std::ostringstream& msg)
{
	CConnToCardManager* pConnToCardMngr = CHelperFuncs::GetConnToCardManager();

	if (pConnToCardMngr && pAllocatedRsrcDesc)
	{
		cmEntry.m_id = pAllocatedRsrcDesc->GetConnId();
		cmEntry.rsrcType = pAllocatedRsrcDesc->GetType();
		cmEntry.physicalRsrcType = pAllocatedRsrcDesc->GetPhysicalType();

		cmEntry.boxId = pAllocatedRsrcDesc->GetBoxId();
		cmEntry.boardId = pAllocatedRsrcDesc->GetBoardId();
		cmEntry.subBoardId = pAllocatedRsrcDesc->GetSubBoardId();
		cmEntry.unitId = pAllocatedRsrcDesc->GetUnitId();
		cmEntry.acceleratorId = pAllocatedRsrcDesc->GetAcceleratorId();
		cmEntry.portId = pAllocatedRsrcDesc->GetFirstPortId();
		cmEntry.channelId = 0;

		AddEntryToSharedMemory(cmEntry); // pConnToCardMngr->Add(cmEntry);
		ConnToCardEntryDump(msg, cmEntry);
	}
}

////////////////////////////////////////////////////////////////////////////
BOOL CRsrcAlloc::CheckIfNewPartyHasDifferentType(DWORD monitor_conf_id, eVideoPartyType videoPartyType) // AVC-SVC MIX mode
{
	BOOL need_move_mixed_mode = FALSE;
	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	const CConfRsrc* pConfRsrc = pConfRsrcDB ? pConfRsrcDB->GetConfRsrc(monitor_conf_id) : NULL;
	if (pConfRsrc) // check if the participant will cause transfer of the conference to MIXED mode
	{
		bool hasRelayParties = pConfRsrc->CheckIfThereAreRelayParty();
		bool hasNotRelayParties = pConfRsrc->CheckIfThereAreNotRelayParty();

		if (CHelperFuncs::IsVideoRelayParty(videoPartyType))
		{
			need_move_mixed_mode = (hasNotRelayParties && !hasRelayParties);
		}
		else if (CHelperFuncs::IsVideoParty(videoPartyType))
		{
			need_move_mixed_mode = (hasRelayParties && !hasNotRelayParties);
		}
	}
	return need_move_mixed_mode;
}

////////////////////////////////////////////////////////////////////////////
// deallocate AVC additional resources
void CRsrcAlloc::DeAllocateAvcRelayResources(DWORD monitor_conf_id, DWORD rsrcPartyId, bool needToKillPort)
{

	PTRACE2INT(eLevelInfoNormal, "CRsrcAlloc::DeAllocateAvcRelayResources : rsrcPartyId=", rsrcPartyId);

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	PASSERT_AND_RETURN(!pConfRsrcDB || !pSystemResources);

	CConfRsrc* pConf = (CConfRsrc*)(pConfRsrcDB->GetConfRsrc(monitor_conf_id));
	PASSERT_AND_RETURN(!pConf);

	CPartyRsrc* pParty = (CPartyRsrc*)(pConf->GetPartyRsrcByRsrcPartyId(rsrcPartyId));
	PASSERT_AND_RETURN(!pParty);

	CConnToCardManager* pConnToCardMngr = CHelperFuncs::GetConnToCardManager();
	PASSERT_AND_RETURN(!pConnToCardMngr);

	eVideoPartyType videoPartyType = pParty->GetVideoPartyType();
	eNetworkPartyType networkPartyType = pParty->GetNetworkPartyType();

	if (CHelperFuncs::IsVideoRelayParty(videoPartyType) || (eVoice_relay_party_type == videoPartyType) || (eMixAvcSvc == pConf->GetConfMediaType()))
	{
		CRsrcDesc* pRelayRtp = (CRsrcDesc*)(pConf->GetDesc(rsrcPartyId, eLogical_relay_rtp));
		if (pRelayRtp)
		{
			DWORD connIdRelayRTP = pRelayRtp->GetConnId();
			pSystemResources->DeAllocateConnId(connIdRelayRTP);
			DBGPASSERT(pConf->RemoveDesc(rsrcPartyId, eLogical_relay_rtp));
			DBGPASSERT(pConnToCardMngr->Remove(connIdRelayRTP));

		}
		CRsrcDesc* pRelaySvcToAvcRtp = (CRsrcDesc*)(pConf->GetDesc(rsrcPartyId, eLogical_relay_svc_to_avc_rtp));
		if (pRelaySvcToAvcRtp)
		{
			DWORD connIdSvcToAvcRTP = pRelaySvcToAvcRtp->GetConnId();
			pSystemResources->DeAllocateConnId(connIdSvcToAvcRTP);
			DBGPASSERT(pConf->RemoveDesc(rsrcPartyId, eLogical_relay_svc_to_avc_rtp));
			DBGPASSERT(pConnToCardMngr->Remove(connIdSvcToAvcRTP));
		}
	}
	if (!CHelperFuncs::IsVideoRelayParty(videoPartyType) && (eMixAvcSvc == pConf->GetConfMediaType()))
	{
		CRsrcDesc* pRelayAvcToSvcRtp = (CRsrcDesc*)(pConf->GetDesc(rsrcPartyId, eLogical_relay_avc_to_svc_rtp));
		if (pRelayAvcToSvcRtp)
		{
			PhysicalPortDesc portDesc;
			portDesc.m_boxId      = pRelayAvcToSvcRtp->GetBoxId();
			portDesc.m_boardId    = pRelayAvcToSvcRtp->GetBoardId();
			portDesc.m_subBoardId = pRelayAvcToSvcRtp->GetSubBoardId();
			portDesc.m_unitId     = pRelayAvcToSvcRtp->GetUnitId();
			portDesc.m_portId     = pRelayAvcToSvcRtp->GetFirstPortId();

			BYTE dsbl = FALSE;
			if (needToKillPort)
			{
				STATUS kill_stat = SendKillPortRequest(&portDesc, ePhysical_art /*portNotResponding*/);
				dsbl = TRUE;
				DBGPASSERT(STATUS_OK != kill_stat);
			}

			STATUS status = pSystemResources->DeAllocateART(pConf, rsrcPartyId, pParty->GetARTChannels(), ePhysical_art, videoPartyType, &portDesc, dsbl);
			DBGPASSERT(status != STATUS_OK);

			DWORD connIdAvcToSvcRTP = pRelayAvcToSvcRtp->GetConnId();
			pSystemResources->DeAllocateConnId(connIdAvcToSvcRTP);
			DBGPASSERT(pConf->RemoveDesc(rsrcPartyId, eLogical_relay_avc_to_svc_rtp));
			DBGPASSERT(pConnToCardMngr->Remove(connIdAvcToSvcRTP));
		}

		CRsrcDesc* pRelayAvcToSvcRtpWithAudioEnc = (CRsrcDesc*)(pConf->GetDesc(rsrcPartyId, eLogical_relay_avc_to_svc_rtp_with_audio_encoder));
		if (pRelayAvcToSvcRtpWithAudioEnc)
		{
			PhysicalPortDesc portDesc;
			portDesc.m_boxId      = pRelayAvcToSvcRtpWithAudioEnc->GetBoxId();
			portDesc.m_boardId    = pRelayAvcToSvcRtpWithAudioEnc->GetBoardId();
			portDesc.m_subBoardId = pRelayAvcToSvcRtpWithAudioEnc->GetSubBoardId();
			portDesc.m_unitId     = pRelayAvcToSvcRtpWithAudioEnc->GetUnitId();
			portDesc.m_portId     = pRelayAvcToSvcRtpWithAudioEnc->GetFirstPortId();

			BYTE dsbl = FALSE;
			if (needToKillPort)
			{
				STATUS kill_stat = SendKillPortRequest(&portDesc, ePhysical_art /*portNotResponding*/);
				dsbl = TRUE;
				DBGPASSERT(STATUS_OK != kill_stat);
			}

			STATUS status = pSystemResources->DeAllocateART(pConf, rsrcPartyId, pParty->GetARTChannels(), ePhysical_art, videoPartyType, &portDesc, dsbl);
			DBGPASSERT(status != STATUS_OK);

			DWORD connIdAvcToSvcRTPWithAudioEnc = pRelayAvcToSvcRtpWithAudioEnc->GetConnId();
			pSystemResources->DeAllocateConnId(connIdAvcToSvcRTPWithAudioEnc);
			DBGPASSERT(pConf->RemoveDesc(rsrcPartyId, eLogical_relay_avc_to_svc_rtp_with_audio_encoder));
			DBGPASSERT(pConnToCardMngr->Remove(connIdAvcToSvcRTPWithAudioEnc));
		}

		if (eProductFamilyRMX == CProcessBase::GetProcess()->GetProductFamily())
		{
			const CRsrcDesc* pRelayAvcLegacyToSACaudioEnc = pConf->GetDesc(rsrcPartyId, eLogical_legacy_to_SAC_audio_encoder);
			if (pRelayAvcLegacyToSACaudioEnc)
			{
				DWORD connIdAvcLegacyToSACaudioEnc = pRelayAvcLegacyToSACaudioEnc->GetConnId();
				pSystemResources->DeAllocateConnId(connIdAvcLegacyToSACaudioEnc);
				DBGPASSERT(pConf->RemoveDesc(rsrcPartyId, eLogical_legacy_to_SAC_audio_encoder));
				DBGPASSERT(pConnToCardMngr->Remove(connIdAvcLegacyToSACaudioEnc));
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////
// This function reply to party controll with existing, already allocated resources
// It uses to reply to party controll opcode AVC_SVC_ADDITIONAL_PARTY_RSRC_REQ
void CRsrcAlloc::ResponseWithExistingResources(ALLOC_PARTY_REQ_PARAMS_S* pParam, ALLOC_PARTY_IND_PARAMS_S* pResult)
{
	pResult->allocIndBase.status = STATUS_FAIL;
	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	PASSERT_AND_RETURN(!pConfRsrcDB);

	pResult->allocIndBase.status = STATUS_CONFERENCE_NOT_EXISTS;
	CConfRsrc* pConf = pConfRsrcDB->GetConfRsrc(pParam->monitor_conf_id);
	PASSERTSTREAM_AND_RETURN(!pConf, "MonitorConfId:" << pParam->monitor_conf_id);

	pResult->allocIndBase.status = STATUS_PARTY_NOT_EXISTS;
	CPartyRsrc* pParty = (CPartyRsrc*)(pConf->GetParty(pParam->monitor_party_id));
	PASSERTSTREAM_AND_RETURN(!pParty, "MonitorConfId:" << pParam->monitor_conf_id << ", MonitorPartyId:" << pParam->monitor_party_id);

	// Fill ALLOC_PARTY_IND_PARAMS_S_BASE
	pResult->allocIndBase.status           = STATUS_OK;
	pResult->allocIndBase.rsrc_conf_id     = pConf->GetRsrcConfId();
	pResult->allocIndBase.rsrc_party_id    = pParty->GetRsrcPartyId();
	pResult->allocIndBase.room_id          = pParty->GetRoomPartyId();
	pResult->allocIndBase.networkPartyType = pParty->GetNetworkPartyType();
	pResult->allocIndBase.videoPartyType   = pParty->GetVideoPartyType();
	pResult->allocIndBase.isIceParty       = pParam->isIceParty; // not exist in CPartyRsrc
	pResult->allocIndBase.confMediaType    = pConf->GetConfMediaType();
	pResult->allocIndBase.numRsrcs         = 0;

	CheckIfAllocOfAVCHDisSupportedAndUpdate(pParam, pResult);

	std::ostringstream msg;
	msg << "ConfId:" << pResult->allocIndBase.rsrc_conf_id << ", PartyId:" << pResult->allocIndBase.rsrc_party_id << ", VideoPartyType:" << pResult->allocIndBase.videoPartyType;

	CRsrcDesc** pRsrcDescArray = new CRsrcDesc*[MAX_NUM_ALLOCATED_RSRCS];
	for (int i = 0; i < MAX_NUM_ALLOCATED_RSRCS; i++)
		pRsrcDescArray[i] = NULL;

	WORD num_party_desc = pConf->GetPartyRcrsDescArrayByRsrcId(pParty->GetRsrcPartyId(), pRsrcDescArray, MAX_NUM_ALLOCATED_RSRCS);

	// Fill logical resources
	for (WORD i = 0; i < num_party_desc; ++i)
	{
		// eLogical_net resources not reported in ALLOC_PARTY_IND_PARAMS_S_BASE, but in ISDN_PARTY_IND_PARAMS_S
		if (pRsrcDescArray[i] && eLogical_net != pRsrcDescArray[i]->GetType())
		{
			pResult->allocIndBase.allocatedRrcs[pResult->allocIndBase.numRsrcs].connectionId    = pRsrcDescArray[i]->GetConnId();
			pResult->allocIndBase.allocatedRrcs[pResult->allocIndBase.numRsrcs].logicalRsrcType = pRsrcDescArray[i]->GetType();
			pResult->allocIndBase.numRsrcs++;
		}
	}

	delete[] pRsrcDescArray;

	// cs connection id - Logical_ip_signaling - allocated only connection id, no descriptor
	ConnectionID connectionIdCS = pParty->GetCSconnId();
	if (connectionIdCS && CHelperFuncs::IsIPParty(pParty->GetNetworkPartyType()))
	{
		pResult->allocIndBase.allocatedRrcs[pResult->allocIndBase.numRsrcs].connectionId    = connectionIdCS;
		pResult->allocIndBase.allocatedRrcs[pResult->allocIndBase.numRsrcs].logicalRsrcType = eLogical_ip_signaling;
		pResult->allocIndBase.numRsrcs++;
	}

	TRACEINTO << "1." << msg.str().c_str();

	// Fill UdpAddresses
	bool isIsdnParty = CHelperFuncs::IsISDNParty(pParty->GetNetworkPartyType());
	if (!isIsdnParty)
	{ // PSTN - prevent asserts

		TRACEINTO << "2." << msg.str().c_str();

		const CUdpRsrcDesc* pUdpRsrcDesc = pConf->GetUdpDesc(pParty->GetRsrcPartyId());
		if (pUdpRsrcDesc)
		{
			pResult->udpAdresses = pUdpRsrcDesc->m_udp;
			TRACEINTO << "3." << msg.str().c_str() << ", Udp:" << pResult->udpAdresses.VideoChannelPort;
		}
	}

	// Fill ISDN_PARTY_IND_PARAMS_S
	pResult->isdnParams.num_of_isdn_ports = 0;

	// Fill SVC_PARTY_IND_PARAMS_S
	pResult->svcParams.m_ssrcAudio = pParty->m_ssrcAudio;
	for (WORD i = 0; i < MAX_NUM_RECV_STREAMS_FOR_CONTENT; ++i)
		pResult->svcParams.m_ssrcContent[i] = pParty->m_ssrcContent[i];

	for (WORD i = 0; i < MAX_NUM_RECV_STREAMS_FOR_VIDEO; ++i)
		pResult->svcParams.m_ssrcVideo[i] = pParty->m_ssrcVideo[i];
}

////////////////////////////////////////////////////////////////////////////
// This function Allocates all additional resources for SVC-AVC mix
STATUS CRsrcAlloc::UpgradePartiesToAvcSvcMix(ConfRsrcID confId, BOOL upgradeAvc, BOOL upgradeSvc)
{
	STATUS ret_val = STATUS_OK;
	TRACEINTO << "ConfId:" << confId << ", UpgradeAvc:" << (WORD)upgradeAvc << ", UpgradeSvc:" << (WORD)upgradeSvc << " - (DYNAMIC_ALLOCATION_AVC_SVC)";

	CSystemResources* pSystResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN_VALUE(pSystResources == NULL, STATUS_FAIL);

	// 1) get CConfRsrc from CConfRsrcDB
	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	CConfRsrc* pConfRsrc = pConfRsrcDB ? pConfRsrcDB->GetConfRsrcByRsrcConfId(confId) : NULL;
	if (!pConfRsrc)
	{
		TRACEINTO << " failled to find conference ,rsrcConfId = " << confId;
		return STATUS_CONFERENCE_NOT_EXISTS;
	}

	// 2) go over all participants in the conf
	STATUS party_upgrade_status = STATUS_OK;

	// start parties loop
	const std::set<CPartyRsrc>* pPartiesList = pConfRsrc->GetPartiesList();

	std::set<CPartyRsrc>::iterator partyEnd = pPartiesList->end();
	for (std::set<CPartyRsrc>::iterator partyItr = pPartiesList->begin(); partyItr != partyEnd; ++partyItr)
	{
		PartyRsrcID partyId = partyItr->GetRsrcPartyId();
		CPartyRsrc* pParty = (CPartyRsrc*)(pConfRsrc->GetPartyRsrcByRsrcPartyId(partyId));
		if (pParty)
		{
			eVideoPartyType videoPartyType = ((CPartyRsrc*)(pParty))->GetVideoPartyType();
			if (upgradeAvc && !CHelperFuncs::IsVideoRelayParty(videoPartyType, false))
			{
				// AVC party
				// 3) per party: upgrade Avc party
	    		TRACEINTO << " upgrading Avc party to mix, rsrcPartyId:" << partyId;
	    		if (eProductTypeSoftMCUMfw == pSystResources->GetProductType())
	    			UpgradePartyResourceUsageToMixedMode(*pConfRsrc, *pParty);
	    		else
	    			party_upgrade_status = UpgradeAvcPartyToAvcSvcMix(*pConfRsrc, *pParty);

				if (STATUS_OK != party_upgrade_status)
				{
					DBGPASSERT(party_upgrade_status);
					TRACEINTO << " upgrading Avc party to mix failed, rsrcPartyId = " << partyId << " , party_upgrade_status " << CProcessBase::GetProcess()->GetStatusAsString(party_upgrade_status);
					// TODO: break and implement roll back
				}
			}
			else if (upgradeSvc && CHelperFuncs::IsVideoRelayParty(videoPartyType, false))
			{
				// 3) per party: upgrade Svc party
				// SVC party
				if (eProductTypeSoftMCUMfw == pSystResources->GetProductType())
					UpgradePartyResourceUsageToMixedMode(*pConfRsrc, *pParty);
				else
					party_upgrade_status = UpgradeSvcPartyToAvcSvcMix(*pConfRsrc, *pParty);

				if (STATUS_OK != party_upgrade_status)
				{
					DBGPASSERT(party_upgrade_status);
					TRACEINTO << " upgrading Svc party to mix failed, rsrcPartyId = " << partyId << " , party_upgrade_status " << CProcessBase::GetProcess()->GetStatusAsString(party_upgrade_status);
					// TODO: break and implement roll back
				}
			}
			else
			{
				TRACEINTO << "PartyId:" << partyId << ", VideoPartyType:" << eVideoPartyTypeNames[videoPartyType] << ", UpgradeAvc:" << (WORD)upgradeAvc << ", UpgradeSvc:" << (WORD)upgradeSvc << " - No need upgrade";
			}
		}
		else
		{
			// failed to get party
			DBGPASSERT(partyId);
			TRACEINTO << " Failed to find party, rsrcPartyId = " << partyId;
		}
	} // end parties loop

	TRACEINTO << " Set conf media state to eMediaStateMixAvcSvc";
	pConfRsrc->SetConfMediaState(eMediaStateMixAvcSvc);

	if (pSystResources->GetProductType() != eProductTypeSoftMCUMfw)
		SendUpgradeToAvcSvcMixToConfPartyProcess(pConfRsrc->GetMonitorConfId());

	return ret_val;
}

////////////////////////////////////////////////////////////////////////////
// This function Allocates AVC party additional resources for SVC-AVC mix
STATUS CRsrcAlloc::UpgradeAvcPartyToAvcSvcMix(CConfRsrc& rConfRsrc, CPartyRsrc& rPartyRsrc)
{
	STATUS ret_val = STATUS_OK;
	TRACEINTO << "PartyId:" << rPartyRsrc.GetRsrcPartyId() << " - (DYNAMIC_ALLOCATION_AVC_SVC)";

	if ((eParty_Role_content_decoder == rPartyRsrc.GetPartyRole()) || (eParty_Role_content_encoder == rPartyRsrc.GetPartyRole()))
	{
		TRACEINTO << "No need to upgrade party to mix due to its party role is: " << rPartyRsrc.GetPartyRole();
		return ret_val;
	}

	// we create structs that we can use existing AllocateAvcRelayResources function
	// create ALLOC_PARTY_REQ_PARAMS_S and PartyDataStruct from CConfRsrc and CPartyRsrc
	// since we don't have the exact original ALLOC_PARTY_REQ_PARAMS_S we set the parameters that suite the upgrade request
	ALLOC_PARTY_REQ_PARAMS_S* pAllocReq = new ALLOC_PARTY_REQ_PARAMS_S;
	PartyDataStruct partyData;
	memset(&partyData, 0, sizeof(partyData));

	InitAllocPartyReqOnUpgradeToAvcSvcMix(*pAllocReq, rConfRsrc, rPartyRsrc);
	ISDN_SPAN_PARAMS_S* pIsdn_Params_Request = NULL;
	BOOL bIsISDNParty = CHelperFuncs::IsISDNParty(rPartyRsrc.GetNetworkPartyType());
	if (bIsISDNParty)
	{
		pIsdn_Params_Request = new ISDN_SPAN_PARAMS_S;
		pIsdn_Params_Request->board_id = (WORD)(-1);
		pIsdn_Params_Request->span_id = (WORD)(-1);
		pIsdn_Params_Request->num_of_isdn_ports = 1;
		pIsdn_Params_Request->isBondingTemporaryPhoneNumberNeeded = 0;
	}
	InitPartyDataStructOnUpgradeToAvcSvcMix(partyData, rConfRsrc, rPartyRsrc, pIsdn_Params_Request);

	// fill entry for share memory with IDs
	ConnToCardTableEntry Entry;
	Entry.rsrc_conf_id = rConfRsrc.GetRsrcConfId();
	Entry.rsrc_party_id = rPartyRsrc.GetRsrcPartyId();
	Entry.room_id = rPartyRsrc.GetRoomPartyId();

	// to suite AllocateAvcRelayResources function
	std::ostringstream msg;
	// get current allocated RTP
	CRsrcDesc* pAllocatedRtpDesc = (CRsrcDesc*)rConfRsrc.GetDesc(rPartyRsrc.GetRsrcPartyId(), eLogical_rtp);
	// in pstn - eLogical_rtp was not allocated so we use audio decoder for taking board id.
	if (NULL == pAllocatedRtpDesc)
	{
		pAllocatedRtpDesc = (CRsrcDesc*)rConfRsrc.GetDesc(rPartyRsrc.GetRsrcPartyId(), eLogical_audio_decoder);
	}
	// to suite AllocateAvcRelayResources function
	ALLOC_PARTY_IND_PARAMS_S* pResult = new ALLOC_PARTY_IND_PARAMS_S;
	pResult->allocIndBase.numRsrcs = 0;

	int numRsrcAllocated = 0;

	ConnToCardEntryDumpHeader(msg, "CRsrcAlloc::UpgradeAvcPartyToAvcSvcMix - Shared memory", Entry.rsrc_conf_id, Entry.rsrc_party_id, Entry.room_id);

	numRsrcAllocated += AllocateAvcRelayResources(pAllocReq, partyData, pAllocatedRtpDesc, Entry, msg, pResult);

	AllocatePartyRelayVideoResources(rConfRsrc, rPartyRsrc, partyData, Entry, msg);

	ConnToCardEntryDumpFooter(msg);

	UpgradePartyResourceUsageToMixedMode(rConfRsrc, rPartyRsrc);

	if (bIsISDNParty)
	{
		delete pIsdn_Params_Request;
	}
	delete pAllocReq;
	delete pResult;

	return ret_val;
}

////////////////////////////////////////////////////////////////////////////
// This function Allocates AVC party additional resources for SVC-AVC mix
STATUS CRsrcAlloc::UpgradeSvcPartyToAvcSvcMix(CConfRsrc& rConfRsrc, CPartyRsrc& rPartyRsrc)
{
	STATUS ret_val = STATUS_OK;
	TRACEINTO << " (DYNAMIC_ALLOCATION_AVC_SVC) rsrcPartyId = " << rPartyRsrc.GetRsrcPartyId();

	PartyDataStruct partyData;
	ISDN_SPAN_PARAMS_S* pIsdn_Params_Request = NULL;
	BOOL bIsISDNParty = CHelperFuncs::IsISDNParty(rPartyRsrc.GetNetworkPartyType());
	if (bIsISDNParty)
	{
		pIsdn_Params_Request = new ISDN_SPAN_PARAMS_S;
		pIsdn_Params_Request->board_id = (WORD)(-1);
		pIsdn_Params_Request->span_id = (WORD)(-1);
		pIsdn_Params_Request->num_of_isdn_ports = 1;
		pIsdn_Params_Request->isBondingTemporaryPhoneNumberNeeded = 0;
	}
	InitPartyDataStructOnUpgradeToAvcSvcMix(partyData, rConfRsrc, rPartyRsrc, pIsdn_Params_Request);

	// fill entry for share memory with IDs
	ConnToCardTableEntry Entry;
	Entry.rsrc_conf_id = rConfRsrc.GetRsrcConfId();
	Entry.rsrc_party_id = rPartyRsrc.GetRsrcPartyId();
	;
	Entry.room_id = rPartyRsrc.GetRoomPartyId();
	std::ostringstream msg;

	ConnToCardEntryDumpHeader(msg, "CRsrcAlloc::UpgradeSvcPartyToAvcSvcMix - Shared memory", Entry.rsrc_conf_id, Entry.rsrc_party_id, Entry.room_id);

	AllocatePartyRelayVideoResources(rConfRsrc, rPartyRsrc, partyData, Entry, msg);

	ConnToCardEntryDumpFooter(msg);

	UpgradePartyResourceUsageToMixedMode(rConfRsrc, rPartyRsrc);

	if (bIsISDNParty)
	{
		delete pIsdn_Params_Request;
	}
	return ret_val;
}

////////////////////////////////////////////////////////////////////////////
// This function initiate ALLOC_PARTY_REQ_PARAMS_S from CConfRsrc and CPartyRsrc
// It initiate only appropriate parameters for upgrade party to Avc-Svc mix
void CRsrcAlloc::InitAllocPartyReqOnUpgradeToAvcSvcMix(ALLOC_PARTY_REQ_PARAMS_S& allocPartyReq, CConfRsrc& rConfRsrc, CPartyRsrc& rPartyRsrc)
{
	TRACEINTO << "PartyId:" << rPartyRsrc.GetRsrcPartyId();

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	eSystemCardsMode systemCardsMode = pSystemResources ? pSystemResources->GetSystemCardsMode() : eSystemCardsMode_illegal;

	allocPartyReq.monitor_conf_id = rConfRsrc.GetMonitorConfId();
	allocPartyReq.monitor_party_id = rPartyRsrc.GetMonitorPartyId();
	allocPartyReq.party_id = rPartyRsrc.GetRsrcPartyId();
	allocPartyReq.artCapacity = (eSystemCardsMode_mpmrx == systemCardsMode) ? ART_CAPACITY_FOR_MIX_MODE_TRANSLATORS_MPMRX : 0;
	allocPartyReq.networkPartyType = rPartyRsrc.GetNetworkPartyType();
	allocPartyReq.videoPartyType = rPartyRsrc.GetVideoPartyType();
	allocPartyReq.sessionType = rConfRsrc.GetSessionType();

	BOOL bIsISDNParty = CHelperFuncs::IsISDNParty(rPartyRsrc.GetNetworkPartyType());
	if (bIsISDNParty)
	{ // pstn
		allocPartyReq.serviceId = 0;
		allocPartyReq.subServiceId = 0;
	}
	else
	{
		const CUdpRsrcDesc* pUdpRsrcDesc = rConfRsrc.GetUdpDesc(rPartyRsrc.GetRsrcPartyId());
		if (pUdpRsrcDesc)
		{
			allocPartyReq.serviceId = pUdpRsrcDesc->GetServId();
			allocPartyReq.subServiceId = pUdpRsrcDesc->GetsubServId();
		}
		else
		{
			TRACEINTO << " failed to get service id";
			allocPartyReq.serviceId = 0;
			allocPartyReq.subServiceId = 0;
		}
	}

	// non relevant for Avc-Svc mix
	allocPartyReq.optionsMask = 0;
	allocPartyReq.allocationPolicy = eAllocateAllRequestedResources;
	allocPartyReq.isWaitForRsrcAndAskAgain = FALSE;
	allocPartyReq.bRmxPortGaugeThresholdExceeded = FALSE;
	if (bIsISDNParty)
	{
		allocPartyReq.isdn_span_params.num_of_isdn_ports = 1; // used for PSTN art weight only
	}
	else
	{
		allocPartyReq.isdn_span_params.num_of_isdn_ports = 0; // we don't allocate isdn ports when allocating additional Avc/Svc resources
	}
	allocPartyReq.isIceParty = 0; // not for Avc-Svc mix
	allocPartyReq.isBFCP = 0;

	allocPartyReq.tipPartyType = rPartyRsrc.GetTIPPartyType();
	allocPartyReq.room_id = rPartyRsrc.GetRoomPartyId();

	allocPartyReq.confMediaType = eMixAvcSvc;

	if (bIsISDNParty)
	{ // pstn - we not actually allocate here - just fill the struct to prevent asserts
		allocPartyReq.isdn_span_params.board_id = (WORD)(-1);
		allocPartyReq.isdn_span_params.span_id = (WORD)(-1);
		allocPartyReq.isdn_span_params.num_of_isdn_ports = 1;
		allocPartyReq.isdn_span_params.isBondingTemporaryPhoneNumberNeeded = 0;
	}
}

////////////////////////////////////////////////////////////////////////////
// This function initiate PartyDataStruct from CConfRsrc and CPartyRsrc
// It initiate only appropriate parameters for upgrade party to Avc-Svc mix
void CRsrcAlloc::InitPartyDataStructOnUpgradeToAvcSvcMix(PartyDataStruct& partyData, CConfRsrc& rConfRsrc, CPartyRsrc& rPartyRsrc, ISDN_SPAN_PARAMS_S* pIsdn_Params_Request)
{
	TRACEINTO << "PartyId:" << rPartyRsrc.GetRsrcPartyId();

	partyData.m_monitor_conf_id = rConfRsrc.GetMonitorConfId();
	partyData.m_videoPartyType = rPartyRsrc.GetVideoPartyType();
	partyData.m_networkPartyType = rPartyRsrc.GetNetworkPartyType();
	partyData.m_room_id = rPartyRsrc.GetRoomPartyId();
	partyData.m_partyTypeTIP = rPartyRsrc.GetTIPPartyType();
	partyData.m_confMediaType = rConfRsrc.GetConfMediaType();
	partyData.m_HdVswTypeInMixAvcSvcMode = rPartyRsrc.GetHdVswTypeInMixAvcSvcMode();

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	eSystemCardsMode systemCardsMode = pSystemResources ? pSystemResources->GetSystemCardsMode() : eSystemCardsMode_illegal;

	partyData.m_artCapacity = (eSystemCardsMode_mpmrx == systemCardsMode) ? ART_CAPACITY_FOR_MIX_MODE_TRANSLATORS_MPMRX : 0;

	if (rConfRsrc.GetBoardIdForRelayParty())
	{
		partyData.m_reqBoardId = rConfRsrc.GetBoardIdForRelayParty();
	}
	else
	{
		CRsrcDesc* pAllocatedRtpDesc = (CRsrcDesc*)rConfRsrc.GetDesc(rPartyRsrc.GetRsrcPartyId(), eLogical_rtp);
		if (NULL != pAllocatedRtpDesc)
		{
			partyData.m_reqBoardId = pAllocatedRtpDesc->GetBoardId();
		}
		else
		{
			partyData.m_reqBoardId = 0xFFFF;
		}
	}

	partyData.m_confModeType = eMix;

	partyData.m_allowReconfiguration = FALSE; // on upgrade to mix
	partyData.m_pIsdn_Params_Request = NULL; // no need to allocate ISDN resources (only translators) - not for Avc-Svc mix

	WORD boardTip = 0;
	WORD unitTip = 0;
	rConfRsrc.GetBoardUnitIdByRoomIdTIP(rPartyRsrc.GetRoomPartyId(), boardTip, unitTip);
	partyData.m_reqArtUnitId = unitTip;

	partyData.m_countPartyAsICEinMFW = rPartyRsrc.GetCountPartyAsICEinMFW();

	BOOL bIsISDNParty = CHelperFuncs::IsISDNParty(partyData.m_networkPartyType);
	if (bIsISDNParty)
	{
		partyData.m_subServiceId = 0;
		if (pIsdn_Params_Request)
		{
			partyData.m_pIsdn_Params_Request = pIsdn_Params_Request;
		}
		else
		{
			partyData.m_pIsdn_Params_Request = NULL;
		}
	}
	else
	{
		const CUdpRsrcDesc* pUdpRsrcDesc = rConfRsrc.GetUdpDesc(rPartyRsrc.GetRsrcPartyId());
		if (pUdpRsrcDesc)
		{
			partyData.m_subServiceId = pUdpRsrcDesc->GetsubServId();
		}
		else
		{
			partyData.m_subServiceId = 0;
		}
	}
}

////////////////////////////////////////////////////////////////////////////
int CRsrcAlloc::AllocatePartyRelayVideoResources(CConfRsrc& conf, CPartyRsrc& party, PartyDataStruct& partyData, ConnToCardTableEntry& cmEntry, std::ostringstream& ost)
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN_VALUE(!pSystemResources, 0);

	std::ostringstream msg;
	msg << "PartyId:" << party.GetRsrcPartyId() << ", VideoPartyType:" << party.GetVideoPartyType();

	BOOL isNeeded = FALSE, isSvc = FALSE, isHD = FALSE;
	CHelperFuncs::GetRequiredAvcSvcTranslatingVideoResorces(party.GetVideoPartyType(), isNeeded, isSvc, isHD);
	TRACECOND_AND_RETURN_VALUE(!isNeeded, msg.str().c_str() << " - No need to allocate additional relay video resources", 0);

	TRACEINTO  << msg.str().c_str();

	bool canVideoRequestBeSatisfied = false;
	VideoDataPerBoardStruct videoDataPerBoard;
	memset(&videoDataPerBoard, 0, sizeof(videoDataPerBoard));

	// get boards priority
	BoardID boardIds[BOARDS_NUM];
	WORD numOfValidBoards = GetBoardsOrderForAdditionalVideoResources(conf, party, boardIds);
	PASSERT_AND_RETURN_VALUE(!numOfValidBoards, 0);

	// collect availability by the priority
	for (int i = 0; i < numOfValidBoards && i < BOARDS_NUM; ++i)
	{
		pSystemResources->CollectAdditonalAvcSvcMixVideoAvailabilityForBoard(videoDataPerBoard, partyData, boardIds[i], canVideoRequestBeSatisfied);
		if (canVideoRequestBeSatisfied)
			break;
	}

	TRACECOND_AND_RETURN_VALUE(!canVideoRequestBeSatisfied, msg.str().c_str() << " - Request cannot be satisfied", 0);

	// allocate
	CRsrcDesc** pVideoDescArray = NULL;
	AllocateVideo(conf.GetMonitorConfId(), party.GetRsrcPartyId(), conf.GetSessionType(), videoDataPerBoard.m_VideoAlloc, partyData, pVideoDescArray);

	// write to shared memory
	const WORD total_max_video_rsrc = pSystemResources->GetTotalMaxVideoResources();
	if (pVideoDescArray)
	{
		for (int i = 0; i < total_max_video_rsrc; i++)
		{
			if (pVideoDescArray[i])
			{
				WriteRsrcDescToSharedMemory(pVideoDescArray[i], cmEntry, ost);
			}
		}
	}

	// delete array
	if (pVideoDescArray)
	{
		for (int i = 0; i < total_max_video_rsrc; i++)
			PDELETE(pVideoDescArray[i]);

		delete[] pVideoDescArray;
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////
BOOL CRsrcAlloc::CheckIfConfNeedToMoveToMixMode(DWORD monitor_conf_id, eVideoPartyType videoPartyType, BOOL& upgradeAvcParties, BOOL& upgradeSvcParties) // AVC-SVC MIX mode
{
	BOOL bIsSoftMcu = FALSE;
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	if (pSystemResources)
		bIsSoftMcu = CHelperFuncs::IsSoftMCU(pSystemResources->GetProductType());

	std::ostringstream msg;
	msg << "MonitorConfId:" << monitor_conf_id << ", IsSoftMcu:" << (WORD)bIsSoftMcu;

	bool bIsAvcSvcDymamicAllocation = CHelperFuncs::IsDynamicMixedAvcSvcAllocationMode();
	TRACECOND_AND_RETURN_VALUE(!bIsAvcSvcDymamicAllocation, msg.str().c_str() << ", RC:FALSE - System configuration flag MIX_AVC_SVC_DYNAMIC_ALLOCATION is OFF", FALSE);

	// 1) check if conf configured as mixed mode
	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	const CConfRsrc* pConfRsrc = pConfRsrcDB ? pConfRsrcDB->GetConfRsrc(monitor_conf_id) : NULL;
	TRACECOND_AND_RETURN_VALUE(!pConfRsrc, msg.str().c_str() << ", RC:FALSE - Failed to find conference", FALSE);

	eConfMediaType confMediaType = pConfRsrc->GetConfMediaType();
	msg << ", ConfMediaType:" << ConfMediaTypeToString(pConfRsrc->GetConfMediaType()) << " (" << confMediaType << ")";

	bool isMixedType = (eMixAvcSvc == confMediaType || eMixAvcSvcVsw == confMediaType);
	TRACECOND_AND_RETURN_VALUE(!isMixedType, msg.str().c_str() << ", RC:FALSE - Conference is not set as mixed AVC SVC", FALSE);

	// 2) check if conf not already in mixed mode
	TRACECOND_AND_RETURN_VALUE(eMediaStateMixAvcSvc == pConfRsrc->GetConfMediaState(), msg.str().c_str() << ", RC:FALSE - Conference is already mixed AVC SVC state", FALSE);

	// 3) check if conf is EQ
	TRACECOND_AND_RETURN_VALUE(STANDALONE_CONF_ID == pConfRsrc->GetRsrcConfId(), msg.str().c_str() << ", RC:FALSE - EQ conference not using dynamic allocation", FALSE);

	// 4) check if this party change the conf to mix
	BOOL need_move_mixed_mode = FALSE;
	bool hasRelayParties = pConfRsrc->CheckIfThereAreRelayParty();
	bool hasNotRelayParties = pConfRsrc->CheckIfThereAreNotRelayParty();

	if (eProductTypeSoftMCUMfw == pSystemResources->GetProductType() &&
		(hasNotRelayParties || !CHelperFuncs::IsVideoRelayParty(videoPartyType, false)))
	{
		need_move_mixed_mode = TRUE;
		upgradeAvcParties    = FALSE;
		upgradeSvcParties    = TRUE;
		TRACECOND_AND_RETURN_VALUE(1, msg.str().c_str() << ", RC:TRUE - (MFW DYNAMIC_ALLOCATION_AVC_SVC) - AVC parties", TRUE);
	}

	if (CHelperFuncs::IsVideoRelayParty(videoPartyType))
	{
		if (hasNotRelayParties && !hasRelayParties)
		{
			need_move_mixed_mode = TRUE;
			upgradeAvcParties    = TRUE;
			upgradeSvcParties    = FALSE;
			TRACECOND_AND_RETURN_VALUE(1, msg.str().c_str() << ", RC:TRUE - (DYNAMIC_ALLOCATION_AVC_SVC), First SVC party, need to upgrade AVC parties", TRUE);
		}
		else if (hasRelayParties)
		{
			TRACECOND_AND_RETURN_VALUE(1, msg.str().c_str() << ", RC:FALSE - Not first SVC party, no need to upgrade AVC parties", FALSE);
		}
		else if (!hasNotRelayParties)
		{
			TRACECOND_AND_RETURN_VALUE(1, msg.str().c_str() << ", RC:FALSE - No AVC parties in conference, no need to upgrade AVC parties", FALSE);
		}
	}
	else      // removed if (CHelperFuncs::IsVideoParty(videoPartyType)) because PSTN can upgrade SVC parties
	{
		if (hasRelayParties && !hasNotRelayParties)
		{
			need_move_mixed_mode = TRUE;
			upgradeAvcParties    = FALSE;
			upgradeSvcParties    = TRUE;
			TRACECOND_AND_RETURN_VALUE(1, msg.str().c_str() << ", RC:TRUE - (DYNAMIC_ALLOCATION_AVC_SVC) First AVC party, need to upgrade SVC parties", TRUE);
		}
		else if (hasNotRelayParties)
		{
			TRACECOND_AND_RETURN_VALUE(1, msg.str().c_str() << ", RC:FALSE - Not first AVC party, no need to upgrade SVC parties", FALSE);
		}
		else if (!hasRelayParties)
		{
			TRACECOND_AND_RETURN_VALUE(1, msg.str().c_str() << ", RC:FALSE - No SVC parties in conference, no need to upgrade SVC parties", FALSE);
		}
	}

	TRACEINTO << msg.str().c_str() << ", RC:" << (WORD)need_move_mixed_mode;
	return need_move_mixed_mode;
}

////////////////////////////////////////////////////////////////////////////
void CRsrcAlloc::SendUpgradeToAvcSvcMixToConfPartyProcess(DWORD monitor_conf_id) // AVC-SVC MIX mode
{
	TRACEINTO << " (DYNAMIC_ALLOCATION_AVC_SVC) monitor_conf_id = " << monitor_conf_id;

	CSegment* pSeg = new CSegment();
	*pSeg << (DWORD)eMediaStateMixAvcSvc << monitor_conf_id;

	CManagerApi api(eProcessConfParty);
	api.SendMsg(pSeg, SET_CONF_AVC_SVC_MEDIA_STATE);
}

//Check if we have enough resources for transfer some conference to MIXED mode
////////////////////////////////////////////////////////////////////////////
bool CRsrcAlloc::CheckEnoughResourcesTransferConfToMixedMode(CConfRsrc& rConfRsrc, PartyDataStruct& rPartyRsrc)
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN_VALUE(!pSystemResources, false);

	ResourcesInterface* pResourcesInterface = pSystemResources->GetCurrentResourcesInterface();
	PASSERT_AND_RETURN_VALUE(!pResourcesInterface, false);

	bool hasRelayParties = rConfRsrc.CheckIfThereAreRelayParty();
	bool hasAvcParties = rConfRsrc.CheckIfThereAreNotRelayParty();

	eConfMediaState currentConfMediaState = (hasAvcParties && !hasRelayParties) ? eMediaStateAvcOnly : (hasRelayParties ? eMediaStateSvcOnly : eMediaStateEmpty);

	CSystemResources* pSystResources = CHelperFuncs::GetSystemResources();
    PASSERT_AND_RETURN_VALUE(pSystResources == NULL, false);
	if (eProductTypeSoftMCUMfw == pSystResources->GetProductType())
	{
		if (0 == rConfRsrc.GetNumParties())
			return true;
		bool ret_status = CheckBeforeUpgradePartyResourceUsage(rConfRsrc, rPartyRsrc, currentConfMediaState);
		TRACEINTO << "MFW_DEBUG : CheckBeforeUpgradePartyResourceUsage return " << (ret_status ? "YES" : "NO");
		return ret_status;
	}

	switch (currentConfMediaState)
	{
		case eMediaStateSvcOnly:
		case eMediaStateAvcOnly:
			if (IfEnoughLicensesTransferConfToMixedMode(rConfRsrc, rPartyRsrc, true))
				if (IfEnoughARTResourcesTransferConfToMixedMode(rConfRsrc, rPartyRsrc, true))
					if (IfEnoughVideoResourcesTransferConfToMixedMode(rConfRsrc, rPartyRsrc, true))
						return true;
			break;

		case eMediaStateEmpty:
			TRACEINTO << "currentConfMediaState is eMediaStateEmpty";
			break;
		default:
			TRACEINTO << "currentConfMediaState illegal = " << (DWORD)currentConfMediaState;
			break;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////
bool CRsrcAlloc::IfEnoughLicensesTransferConfToMixedMode(CConfRsrc& rConfRsrc, PartyDataStruct& rPartyRsrc, bool isNewParty)
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN_VALUE(!pSystemResources, false);

	ResourcesInterface* pResourcesInterface = pSystemResources->GetCurrentResourcesInterface();
	PASSERT_AND_RETURN_VALUE(!pResourcesInterface, false);

	float additionalLicenses = pResourcesInterface->CalculateAdditionalLicenseForMixMode(rConfRsrc, rPartyRsrc, isNewParty);

	float totalLicenses = pResourcesInterface->GetTotalAllocatedVideoParties();

	TRACEINTO << "TotalLicenses:" << totalLicenses << ", AdditionalLicenses:" << additionalLicenses << ", Sum:" << totalLicenses+additionalLicenses;

	return true;
}

////////////////////////////////////////////////////////////////////////////
bool CRsrcAlloc::IfEnoughARTResourcesTransferConfToMixedMode(CConfRsrc& rConfRsrc, PartyDataStruct& rPartyRsrc, bool isNewParty)
{
	CSystemResources* pSystResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN_VALUE(!pSystResources, false);

	const std::set<CPartyRsrc>* pPartiesList = rConfRsrc.GetPartiesList();
	PASSERT_AND_RETURN_VALUE(!pPartiesList, false);

	BOOL isNeeded = FALSE, isSvc = FALSE, isHD = FALSE;
	CHelperFuncs::GetRequiredAvcSvcTranslatingVideoResorces(rPartyRsrc.m_videoPartyType, isNeeded, isSvc, isHD);

	// Initialization for new-come party
	WORD num_neededed_art_channels = isNewParty ? CHelperFuncs::GetNumArtChannels(rPartyRsrc) : 0;
	WORD num_neededed_art_ports = isNewParty ? (isSvc ? 1 : (isHD ? 3 : 2)) : 0;

	PartyDataStruct partyData;

	ostringstream ostr;

	std::set<CPartyRsrc>::iterator partyEnd = pPartiesList->end();
	for (std::set<CPartyRsrc>::iterator partyItr = pPartiesList->begin(); partyItr != partyEnd; ++partyItr)
	{
		PartyRsrcID partyId = partyItr->GetRsrcPartyId();
		CPartyRsrc* pParty = const_cast<CPartyRsrc*>(rConfRsrc.GetPartyRsrcByRsrcPartyId(partyId));
		if (pParty)
		{
			memset(&partyData, 0, sizeof(partyData));
			partyData.m_videoPartyType   = pParty->GetVideoPartyType();
			partyData.m_networkPartyType = pParty->GetNetworkPartyType();

			num_neededed_art_channels += CHelperFuncs::GetNumArtChannels(partyData);

			if (!CHelperFuncs::IsVideoRelayParty(partyData.m_videoPartyType))
			{
				isNeeded = isSvc = isHD = FALSE;
				CHelperFuncs::GetRequiredAvcSvcTranslatingVideoResorces(partyData.m_videoPartyType, isNeeded, isSvc, isHD);

				num_neededed_art_ports += (isHD ? 2 : 1);
			}
		}
	}
	ostr << "num_neededed_art_channels:" << num_neededed_art_channels << ", num_neededed_art_ports:" << num_neededed_art_ports;
	bool art_status = false;
	if (num_neededed_art_ports > 0)
	{
		WORD boardIdSVC = rConfRsrc.GetBoardIdForRelayParty();
		if (boardIdSVC > 0 && pSystResources->IsBoardIdExists(boardIdSVC))
		{
			int num_remaining_art_ports;
			STATUS status_art = pSystResources->GetRemainingArtOnBoard(boardIdSVC, num_neededed_art_channels, num_remaining_art_ports, 0, partyData, 0, false); //artCapacity==0 because Translators always 7%
			ostr << ", boardId:" << boardIdSVC << ", num_remaining_art_ports:" << num_remaining_art_ports << ", status:" << status_art;
			if (STATUS_OK == status_art && num_remaining_art_ports >= num_neededed_art_ports)
			{
				ostr << " - OK transfer conference to this board";
				art_status = true;
			}
		}
		else //check all cards
		{
			for (int i = 0; i < BOARDS_NUM; ++i)
			{
				WORD bid = i + 1;
				if (!pSystResources->IsBoardIdExists(bid)) // TODO: check for MultipleIpServices environment ?
					continue;
				int num_remaining_art_ports;
				STATUS status_art = pSystResources->GetRemainingArtOnBoard(bid, num_neededed_art_channels, num_remaining_art_ports, 0, partyData, 0, false); // artCapacity==0 because Translators always 7%
				ostr << ", boardId:" << bid << ", num_remaining_art_ports:" << num_remaining_art_ports << ", status:" << status_art;
				if (STATUS_OK == status_art && num_remaining_art_ports >= num_neededed_art_ports)
				{
					ostr << " - OK transfer conference to this board";
					art_status = true;
					break;
				}
			}
		}
	}
	TRACEINTO << ostr.str().c_str();
	return art_status;
}

////////////////////////////////////////////////////////////////////////////
bool CRsrcAlloc::IfEnoughVideoResourcesTransferConfToMixedMode(CConfRsrc& rConfRsrc, PartyDataStruct& rPartyRsrc, bool isNewParty)
{
	// 1. calculate number of needed CIF, SD and HD ports
	CSystemResources* pSystResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN_VALUE(!pSystResources, false);

	const std::set<CPartyRsrc>* pPartiesList = rConfRsrc.GetPartiesList();
	PASSERT_AND_RETURN_VALUE(!pPartiesList, false);

	BOOL isNeeded = FALSE, isSvc = FALSE, isHD = FALSE;
	CHelperFuncs::GetRequiredAvcSvcTranslatingVideoResorces(rPartyRsrc.m_videoPartyType, isNeeded, isSvc, isHD);

	size_t additionalPorts[ePartyType_Max][NUM_OF_PARTY_RESOURCE_TYPES];
	memset(&additionalPorts, 0, sizeof(additionalPorts));

	// Initialization for new-come party
	additionalPorts[ePartyType_Avc][e_SD30] = (isNeeded && isNewParty && !isSvc && isHD) ? 1 : 0;
	additionalPorts[ePartyType_Avc][e_Cif ] = (isNeeded && isNewParty && !isSvc) ? 1 : 0;

	if (isNeeded && isSvc)
	{
		switch (rPartyRsrc.m_HdVswTypeInMixAvcSvcMode)
		{
			case e_HdVsw720 : additionalPorts[ePartyType_Svc][e_HD720    ] = 1; break;
			case e_HdVsw1080: additionalPorts[ePartyType_Svc][e_HD1080p30] = 1; break;
			case eHdVswNon  : additionalPorts[ePartyType_Svc][e_SD30     ] = 1; break;
			default:
				PASSERT_AND_RETURN_VALUE(1, false);
				break;
		}
	}

	ostringstream msg;

	std::set<CPartyRsrc>::iterator _iiEnd = pPartiesList->end();
	for (std::set<CPartyRsrc>::iterator _ii = pPartiesList->begin(); _ii != _iiEnd; ++_ii)
	{
		PartyRsrcID partyId = _ii->GetRsrcPartyId();
		CPartyRsrc* pParty = const_cast<CPartyRsrc*>(rConfRsrc.GetPartyRsrcByRsrcPartyId(partyId));
		if (pParty)
		{
			isNeeded = isSvc = isHD = FALSE;
			CHelperFuncs::GetRequiredAvcSvcTranslatingVideoResorces(pParty->GetVideoPartyType(), isNeeded, isSvc, isHD);
			if (isNeeded)
			{
				if (isSvc)
				{
					switch (pParty->GetHdVswTypeInMixAvcSvcMode())
					{
						case e_HdVsw720 : additionalPorts[ePartyType_Svc][e_HD720    ] += 1; break;
						case e_HdVsw1080: additionalPorts[ePartyType_Svc][e_HD1080p30] += 1; break;
						case eHdVswNon  : additionalPorts[ePartyType_Svc][e_SD30     ] += 1; break;
						default:
							PASSERTSTREAM(1, "partyId:" << partyId);
							break;
					}
				}
				else //AVC party
				{
					if (isHD)
						additionalPorts[ePartyType_Avc][e_SD30] += 1;
					additionalPorts[ePartyType_Avc][e_Cif] += 1;
				}
			}
		}
	}

	// Dump additional ports info
	std::ostringstream msg1;
	for (int i = ePartyType_Avc; i < ePartyType_Max; ++i)
	{
		msg1 << "\nPartyType:" << (ePartyType)i;
		for (int j = e_Audio; j < NUM_OF_PARTY_RESOURCE_TYPES; ++j)
			if (additionalPorts[i][j])
				msg1 << ", " << (ePartyResourceTypes)j << ":" << additionalPorts[i][j];
	}
	TRACEINTO << "needed capacity for video upgrade:" << msg1.str().c_str();

	if (additionalPorts[ePartyType_Svc][e_HD720] && additionalPorts[ePartyType_Svc][e_HD1080p30])
	{
		TRACEINTO << msg.str().c_str() << " - SVC could not support 720p and 1080p at the same time";
		PASSERT_AND_RETURN_VALUE(3, false);
	}

	STATUS status = pSystResources->CheckIfConfVideoCanBeUpgraded(additionalPorts);
	return (STATUS_OK != status) ? false : true;
}

////////////////////////////////////////////////////////////////////////////
WORD CRsrcAlloc::GetBoardsOrderForAdditionalVideoResources(CConfRsrc& rConfRsrc, CPartyRsrc& rPartyRsrc, WORD* arrBoardsOrder)
{
	ostringstream ostr;

	WORD numOfValidBoards = 0;
	BOOL bIsSoftMcu = FALSE;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN_VALUE(!pSystemResources, numOfValidBoards);

	bIsSoftMcu = CHelperFuncs::IsSoftMCU(pSystemResources->GetProductType());

	if (bIsSoftMcu)
	{ // Soft MCU
		ostr << "\nSoft MCU \n";
		arrBoardsOrder[0] = 1; // soft MCU always uses board ID 1
		numOfValidBoards++;
		ostr << "arrBoardsOrder[0] = 1 \n";
	}
	else
	{ // RMX
		ostr << "\nRMX \n";
		BOOL isRelayParty = CHelperFuncs::IsVideoRelayParty(rPartyRsrc.GetVideoPartyType());
		WORD boardIdSVC = rConfRsrc.GetBoardIdForRelayParty();
		if (isRelayParty)
		{
			ostr << "SVC party \n";
			// SVC party
			// first priority - mrmp board - same boards with arts
			// otherwise - find free board
			if (0 != boardIdSVC)
			{
				// we should be here - board should be already set since it is SVC party
				arrBoardsOrder[0] = boardIdSVC;
				ostr << "arrBoardsOrder[0] = " << boardIdSVC << " , boardIdSVC \n";
				numOfValidBoards++;
			}
			for (int board_index = 0; board_index < BOARDS_NUM; board_index++)
			{
				if (numOfValidBoards < BOARDS_NUM && boardIdSVC != board_index + 1 && pSystemResources->IsBoardIdExists(board_index + 1))
				{
					arrBoardsOrder[numOfValidBoards] = board_index + 1;
					ostr << "arrBoardsOrder[" << numOfValidBoards << "] = " << board_index + 1 << " , available board \n";
					numOfValidBoards++;
				}
			}
		}
		else
		{
			// AVC party
			// first priority - the board with video decoder
			// second priority - mrmp board - same boards with arts
			// otherwise - find free board
			ostr << "SVC party \n";
			WORD boardIdDecoder = 0;
			WORD boardIdRtp = 0;
			CRsrcDesc* pAllocatedDecoderDesc = (CRsrcDesc*)rConfRsrc.GetDesc(rPartyRsrc.GetRsrcPartyId(), eLogical_video_decoder);
			if (NULL != pAllocatedDecoderDesc)
			{
				boardIdDecoder = pAllocatedDecoderDesc->GetBoardId();
			}
			CRsrcDesc* pAllocatedRtpDesc = (CRsrcDesc*)rConfRsrc.GetDesc(rPartyRsrc.GetRsrcPartyId(), eLogical_rtp);
			if (NULL != pAllocatedRtpDesc)
			{
				boardIdRtp = pAllocatedRtpDesc->GetBoardId();
			}

			numOfValidBoards = 0;
			if (0 != boardIdDecoder)
			{
				arrBoardsOrder[numOfValidBoards] = boardIdDecoder;
				ostr << "arrBoardsOrder[" << numOfValidBoards << "] = " << boardIdDecoder << " , boardIdDecoder \n";
				numOfValidBoards++;
			}
			if (0 != boardIdSVC && boardIdSVC != boardIdDecoder)
			{
				arrBoardsOrder[numOfValidBoards] = boardIdSVC;
				ostr << "arrBoardsOrder[" << numOfValidBoards << "] = " << boardIdSVC << " , boardIdSVC \n";
				numOfValidBoards++;
			}
			if (0 != boardIdRtp && boardIdRtp != boardIdDecoder && boardIdRtp != boardIdSVC)
			{
				arrBoardsOrder[numOfValidBoards] = boardIdRtp;
				ostr << "arrBoardsOrder[" << numOfValidBoards << "] = " << boardIdRtp << " , boardIdRtp \n";
				numOfValidBoards++;
			}
			for (int board_index = 0; board_index < BOARDS_NUM; board_index++)
			{
				if (numOfValidBoards < BOARDS_NUM && pSystemResources->IsBoardIdExists(board_index + 1) && boardIdSVC != board_index + 1 && boardIdRtp != board_index + 1 && boardIdDecoder != board_index + 1)
				{
					arrBoardsOrder[numOfValidBoards] = board_index + 1;
					ostr << "arrBoardsOrder[" << numOfValidBoards << "] = " << board_index + 1 << " , available board \n";
					numOfValidBoards++;
				}
			}
		}
	}

	ostr << "numOfValidBoards = " << numOfValidBoards;
	TRACEINTO << ostr.str().c_str();

	return numOfValidBoards;
}

////////////////////////////////////////////////////////////////////////////
void CRsrcAlloc::UpgradePartyResourceUsageToMixedMode(CConfRsrc& rConfRsrc, CPartyRsrc& rPartyRsrc)
{
	CSystemResources* pSystResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN(pSystResources == NULL);

	BOOL bIsdnParty = CHelperFuncs::IsISDNParty(rPartyRsrc.GetNetworkPartyType());
	WORD service_id = ID_ALL_IP_SERVICES;
	if (pSystResources->GetMultipleIpServices())
		service_id = pSystResources->GetPartyIpServiceId(rPartyRsrc.GetRsrcPartyId());

	WORD artBoardId = 0xFFFF, videoBoardId = 0xFFFF;
	CRsrcDesc* pAudioDesc = (CRsrcDesc*)rConfRsrc.GetDesc(rPartyRsrc.GetRsrcPartyId(), eLogical_audio_decoder);
	if (pAudioDesc)
		artBoardId = pAudioDesc->GetBoardId();

	CRsrcDesc* pVideoDecDesc = (CRsrcDesc*)rConfRsrc.GetDesc(rPartyRsrc.GetRsrcPartyId(), eLogical_video_decoder);
	if (pVideoDecDesc)
		videoBoardId = pVideoDecDesc->GetBoardId();

	BasePartyDataStruct remPartyData(rPartyRsrc.GetVideoPartyType(), rPartyRsrc.GetPartyRole(), rPartyRsrc.GetRsrcPartyId(), bIsdnParty, artBoardId, videoBoardId, eNonMix, FALSE, service_id, FALSE/*out*/);
	pSystResources->RemoveParty(remPartyData);

	BasePartyDataStruct addPartyData(rPartyRsrc.GetVideoPartyType(), rPartyRsrc.GetPartyRole(), rPartyRsrc.GetRsrcPartyId(), bIsdnParty, artBoardId, videoBoardId, eMix, FALSE, service_id, remPartyData.m_bAddAudioAsVideo/*in*/);
	pSystResources->AddParty(addPartyData);
}

////////////////////////////////////////////////////////////////////////////
void CRsrcAlloc::UpdatePartyResourceUsage(CConfRsrc& rConfRsrc, CPartyRsrc& rPartyRsrc, eVideoPartyType newVideoPartyType, BOOL bAddAudioAsVideo /*= FALSE*/)
{
	CSystemResources* pSystResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN(!pSystResources);

	PartyRsrcID partyId = rPartyRsrc.GetRsrcPartyId();
	BOOL bIsdnParty = CHelperFuncs::IsISDNParty(rPartyRsrc.GetNetworkPartyType());
	WORD service_id = ID_ALL_IP_SERVICES;
	if (pSystResources->GetMultipleIpServices())
		service_id = pSystResources->GetPartyIpServiceId(partyId);

	WORD audioBoardId = 0xFFFF, videoBoardId = 0xFFFF;
	CRsrcDesc* pAudioDescriptor = const_cast<CRsrcDesc*>(rConfRsrc.GetDesc(partyId, eLogical_audio_decoder));
	if (pAudioDescriptor)
		audioBoardId = pAudioDescriptor->GetBoardId();

	CRsrcDesc* pVideoDescriptor = const_cast<CRsrcDesc*>(rConfRsrc.GetDesc(partyId, eLogical_video_decoder));
	if (pVideoDescriptor)
		videoBoardId = pVideoDescriptor->GetBoardId();

	if (videoBoardId == 0xFFFF && CHelperFuncs::IsVideoRelayParty(rPartyRsrc.GetVideoPartyType()))
		videoBoardId = rConfRsrc.GetBoardIdForRelayParty();

	eConfModeTypes confModeType = CHelperFuncs::GetConferenceMode(rConfRsrc.GetMonitorConfId());
	BasePartyDataStruct remPartyData(rPartyRsrc.GetVideoPartyType(), rPartyRsrc.GetPartyRole(), partyId, bIsdnParty, audioBoardId, videoBoardId, confModeType, FALSE, service_id, FALSE);
	pSystResources->RemoveParty(remPartyData);

	BasePartyDataStruct addPartyData(newVideoPartyType, rPartyRsrc.GetPartyRole(), partyId, bIsdnParty, audioBoardId, videoBoardId, confModeType, FALSE, service_id, bAddAudioAsVideo);
	pSystResources->AddParty(addPartyData);
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrcAlloc::StartMixedAvcSvcMove(CConfRsrc& rSrcConfRsrc, CConfRsrc& rTargetConfRsrc, CPartyRsrc& rPartyRsrc)
{

	DWORD target_monitor_conf_id = rTargetConfRsrc.GetMonitorConfId();
	DWORD target_rsrc_conf_id = rTargetConfRsrc.GetRsrcConfId();
	eVideoPartyType videoPartyType = rPartyRsrc.GetVideoPartyType();
	BOOL needToMoveTargetConfToMixMode = FALSE, upgradeAvcParties = FALSE, upgradeSvcParties = FALSE;
	needToMoveTargetConfToMixMode = CheckIfConfNeedToMoveToMixMode(target_monitor_conf_id, videoPartyType, upgradeAvcParties, upgradeSvcParties);

	BOOL needToAllocatePartyAdditionalAvcSvcMixResources = FALSE;
	if ((CHelperFuncs::GetConferenceMode(target_monitor_conf_id) == eMix) || needToMoveTargetConfToMixMode)
	{
		needToAllocatePartyAdditionalAvcSvcMixResources = TRUE;
	}

	TRACEINTO << " (DYNAMIC_ALLOCATION_AVC_SVC) needToMoveTargetConfToMixMode = " << (WORD)needToMoveTargetConfToMixMode << " , needToAllocatePartyAdditionalAvcSvcMixResources = " << (WORD)needToAllocatePartyAdditionalAvcSvcMixResources;

	// Bridge-7686: We don't need to check resource if we don't need more resource
	if (needToAllocatePartyAdditionalAvcSvcMixResources)
	{
		PartyDataStruct partyData;
		// party still in source conf so using pSrcConfRsrc to init partyData
		InitPartyDataStructOnUpgradeToAvcSvcMix(partyData, rSrcConfRsrc, rPartyRsrc);
		// check the destination conf
		bool enoughResources = CheckEnoughResourcesTransferConfToMixedMode(rTargetConfRsrc, partyData);
		if (!enoughResources)
		{
			TRACESTRFUNC(eLevelError) << " (DYNAMIC_ALLOCATION_AVC_SVC) - Not enough resources to move destination conf to mixed mode";
			return STATUS_START_RSRC_PARTY_MOVE_FAIL;
		}
	}

	// upgrade party to mix
	if (needToAllocatePartyAdditionalAvcSvcMixResources)
	{
		STATUS party_upgrade_status = STATUS_OK;
		if (!CHelperFuncs::IsVideoRelayParty(videoPartyType))
		{
			party_upgrade_status = UpgradeAvcPartyToAvcSvcMix(rSrcConfRsrc, rPartyRsrc);
			if (STATUS_OK != party_upgrade_status)
			{
				DBGPASSERT(party_upgrade_status);
			}
		}
		else
		{
			// TRACEINTO << " upgading Svc party to mix, rsrcPartyId = " << rsrcPartyId;
			party_upgrade_status = UpgradeSvcPartyToAvcSvcMix(rSrcConfRsrc, rPartyRsrc);
			if (STATUS_OK != party_upgrade_status)
			{
				DBGPASSERT(party_upgrade_status);
			}
		}
	}

	// upgrade target conf to mix
	if (needToMoveTargetConfToMixMode)
	{
		STATUS moveToMixModeStatus = UpgradePartiesToAvcSvcMix(target_rsrc_conf_id, upgradeAvcParties, upgradeSvcParties);
		if (STATUS_OK != moveToMixModeStatus)
		{
			TRACESTRFUNC(eLevelError) << " move to mix failed";
		}
	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrcAlloc::DeallocateVideo(ConfMonitorID monitorConfId, PartyRsrcID partyId, ConnectionIDs& connectionIdsVideo, BYTE kill_enc, BYTE kill_dec)
{
	bool dsbl = (kill_enc || kill_dec) ? TRUE : FALSE;

	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	PASSERT_AND_RETURN_VALUE(!pConfRsrcDB, STATUS_FAIL);

	CConfRsrc* pConf = const_cast<CConfRsrc*>(pConfRsrcDB->GetConfRsrc(monitorConfId));
	PASSERT_AND_RETURN_VALUE(!pConf, STATUS_CONFERENCE_NOT_EXISTS);

	CPartyRsrc* pParty = const_cast<CPartyRsrc*>(pConf->GetPartyRsrcByRsrcPartyId(partyId));
	PASSERT_AND_RETURN_VALUE(!pParty, STATUS_PARTY_NOT_EXISTS);

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN_VALUE(!pSystemResources, STATUS_FAIL);

	eVideoPartyType videoPartyType = pParty->GetVideoPartyType();
	if (eCOP_party_type == videoPartyType)
	{
		PASSERT_AND_RETURN_VALUE(partyId, STATUS_VIDEO_DEALLOCATION_FAIL);
	}

	CRsrcDesc** pRsrcDescArray = NULL;
	int num_rsrc = pConf->GetDescArray(pRsrcDescArray);

	PhysicalPortDesc portDesc;
	STATUS return_status = STATUS_OK;

	for (int i = 0; i < num_rsrc; i++)
	{
		CRsrcDesc* pDesc = pRsrcDescArray[i];
		if (!pDesc || partyId != pDesc->GetRsrcPartyId())
			continue;

		eLogicalResourceTypes lrt_type = pDesc->GetType();

		STATUS kill_port_status = STATUS_OK;
		STATUS remove_desc_status = STATUS_OK;
		STATUS dealloc_port_status = STATUS_OK;

		if (CHelperFuncs::IsLogicalVideoEncoderType(lrt_type) || CHelperFuncs::IsLogicalVideoDecoderType(lrt_type))
		{
			ConnectionID connectionId = pDesc->GetConnId();
			pSystemResources->DeAllocateConnId(connectionId);

			if (!(eLogical_video_encoder_content == lrt_type && eParty_Role_regular_party == pParty->GetPartyRole()))
			{
				connectionIdsVideo.push_back(connectionId);

				bool is_encoder_lrt = (eLogical_video_encoder == lrt_type || eLogical_video_encoder_content == lrt_type || CHelperFuncs::IsLogicalSoftMixVideoEncoderType(lrt_type));
				bool is_decoder_lrt = (eLogical_video_decoder == lrt_type);

				if ((kill_enc && is_encoder_lrt) || (kill_dec && is_decoder_lrt))
				{
					portDesc.m_boxId         = pDesc->GetBoxId();
					portDesc.m_boardId       = pDesc->GetBoardId();
					portDesc.m_subBoardId    = pDesc->GetSubBoardId();
					portDesc.m_unitId        = pDesc->GetUnitId();
					portDesc.m_portId        = pDesc->GetFirstPortId();
					portDesc.m_acceleratorId = pDesc->GetAcceleratorId();

					kill_port_status = SendKillPortRequest(&portDesc, (is_encoder_lrt ? ePhysical_video_encoder : ePhysical_video_decoder));
				}

				CUnitMFA* pUnitMFA = pSystemResources->GetUnit(pDesc->GetBoardId(), pDesc->GetUnitId());
				if (pUnitMFA)
				{
					dealloc_port_status = pUnitMFA->DeAllocatePort(pDesc->GetFirstPortId(), videoPartyType, dsbl, pDesc->GetAcceleratorId());
				}
				else
				{
					TRACESTRFUNC(eLevelError) << lrt_type << " - Unit not found : rsrcPartyId=" << partyId << ", boardId=" << pDesc->GetBoardId() << ", unitId=" << pDesc->GetUnitId() << ", portId=" << pDesc->GetFirstPortId();
					PASSERT(pDesc->GetUnitId() + 1);
				}
			}

			std::ostringstream msg;
			msg << *pDesc;

			remove_desc_status = pConf->RemoveDesc(pDesc->GetRsrcPartyId(), lrt_type, pDesc->GetCntrlType());

			if (kill_port_status != STATUS_OK || remove_desc_status != STATUS_OK || dealloc_port_status != STATUS_OK)
			{
				if (dealloc_port_status != STATUS_OK)
				{
					msg << "\n  DeallocatePortStatus    :" << dealloc_port_status;
					PASSERT(dealloc_port_status);
				}

				if (kill_port_status != STATUS_OK)
				{
					msg << "\n  KillPortStatus          :" << kill_port_status;
					PASSERT(kill_port_status);
				}

				if (remove_desc_status != STATUS_OK)
				{
					msg << "\n  RemoveDescStatus        :" << remove_desc_status;
					PASSERT(remove_desc_status);
				}

				TRACEINTOLVLERR << "Failed" << msg.str().c_str();

				return_status = STATUS_VIDEO_DEALLOCATION_FAIL;
			}
		}
	}

	delete[] pRsrcDescArray;

	return return_status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrcAlloc::AddEntryToSharedMemory(ConnToCardTableEntry& rEntry)
{

	CConnToCardManager* pConnToCardMngr = CHelperFuncs::GetConnToCardManager();
	PASSERT_AND_RETURN_VALUE(!pConnToCardMngr, STATUS_FAIL);

	STATUS addStat = pConnToCardMngr->Add(rEntry);
	if (STATUS_OK == addStat)
		return STATUS_OK;

	// error handling
	if (addStat == STATUS_ENTRY_ALREDY_EXISTS) // entry occupied
	{
		ConnToCardTableEntry oldEntry;
		int getStatus = pConnToCardMngr->Get(rEntry.m_id, oldEntry);
		if (STATUS_OK == getStatus) // found old entry
		{

			// error handling - remove old entry if party not exist in shared memory
			CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
			if (!pConfRsrcDB)
			{
				PASSERT(1);
				return addStat;
			}
			bool partyFound = false;
			CConfRsrc* pConfRsrc = pConfRsrcDB->GetConfRsrcByRsrcConfId(oldEntry.rsrc_conf_id);
			if (NULL != pConfRsrc)
			{ // conf found
				const CPartyRsrc* pPartyRsrc = pConfRsrc->GetPartyRsrcByRsrcPartyId(oldEntry.rsrc_party_id);
				if (NULL != pPartyRsrc) // party found
				{
					partyFound = true;
				}
			}
			if (!partyFound)
			{
				// remove old entry
				CLargeString mstr1;
				oldEntry.DumpRaw(mstr1);
				TRACESTR(eLevelError) << " error handling: entry for this id already exists, but party not found , removing old entry: \n" << mstr1.GetString();
				int removeStatus = pConnToCardMngr->Remove(oldEntry.m_id);
				if (STATUS_OK != removeStatus)
				{
					TRACESTR(eLevelError) << " failed to remove old entry: \n";
					PASSERT(removeStatus);
				}
				else
				{
					// try add new entry again
					addStat = pConnToCardMngr->Add(rEntry);
					if (STATUS_OK != addStat)
					{
						PASSERT(addStat);
					}
				}
			}
			else
			{
				CLargeString mstr1;
				oldEntry.DumpRaw(mstr1);
				TRACESTR(eLevelError) << " error handling: entry for this id already exists, and party still exist , old entry: \n" << mstr1.GetString();
			}

		}
	}
	// error handling
	return addStat;
}

////////////////////////////////////////////////////////////////////////////
void CRsrcAlloc::DeallocateAvcToSvcMixedART(CConfRsrc* pConf, const CPartyRsrc* pParty, eLogicalResourceTypes lrt, bool needToKillPort)
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	CConnToCardManager* pConnToCardMngr = CHelperFuncs::GetConnToCardManager();

	PASSERT_AND_RETURN(!pConf || !pParty || !pSystemResources || !pConnToCardMngr);

	WORD rsrcPartyId = pParty->GetRsrcPartyId();
	const CRsrcDesc* pRelayAvcToSvcRtp = pConf->GetDesc(rsrcPartyId, lrt);
	if (pRelayAvcToSvcRtp)
	{
		// to free the ART resource - eLogical_relay_avc_to_svc_rtp
		PhysicalPortDesc portDesc;
		portDesc.m_boxId = pRelayAvcToSvcRtp->GetBoxId();
		portDesc.m_boardId = pRelayAvcToSvcRtp->GetBoardId();
		portDesc.m_subBoardId = pRelayAvcToSvcRtp->GetSubBoardId();
		portDesc.m_acceleratorId = pRelayAvcToSvcRtp->GetAcceleratorId();
		portDesc.m_unitId = pRelayAvcToSvcRtp->GetUnitId();
		portDesc.m_portId = pRelayAvcToSvcRtp->GetFirstPortId();

		BYTE dsbl = FALSE;
		if (needToKillPort)
		{
			STATUS kill_stat = SendKillPortRequest(&portDesc, ePhysical_art /*portNotResponding*/);
			dsbl = TRUE;
			DBGPASSERT(STATUS_OK != kill_stat);
		}

		STATUS status = pSystemResources->DeAllocateART(pConf, rsrcPartyId, pParty->GetARTChannels(), ePhysical_art, pParty->GetVideoPartyType(), &portDesc, dsbl);
		DBGPASSERT(status != STATUS_OK);

		DWORD connIdAvcToSvcRTP = pRelayAvcToSvcRtp->GetConnId();
		pSystemResources->DeAllocateConnId(connIdAvcToSvcRTP);
		DBGPASSERT(pConf->RemoveDesc(rsrcPartyId, lrt));
		DBGPASSERT(pConnToCardMngr->Remove(connIdAvcToSvcRTP));
	}
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrcAlloc::AllocateAvcToSvcMixedART(ALLOC_PARTY_REQ_PARAMS_S* pParam, PartyDataStruct& partyData, WORD reqBoardId, ConnToCardTableEntry& cmEntry, std::ostringstream& msg, ALLOC_PARTY_IND_PARAMS_S* pResult)
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	CConnToCardManager* pConnToCardMngr = CHelperFuncs::GetConnToCardManager();
	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	CConfRsrc* pConf = (CConfRsrc*)(pConfRsrcDB ? pConfRsrcDB->GetConfRsrc(pParam->monitor_conf_id) : NULL);
	CPartyRsrc* pParty = (CPartyRsrc*)(pConf ? pConf->GetPartyRsrcByRsrcPartyId(pParam->party_id) : NULL);

	PASSERT_AND_RETURN_VALUE((!pConf || !pParty || !pSystemResources || !pConnToCardMngr), STATUS_FAIL);

	DWORD rsrcPartyId = pParty->GetRsrcPartyId();
	DWORD rsrcConfId = pConf->GetRsrcConfId();

	PhysicalPortDesc rPortDesc;

	STATUS status_alloc_art = pSystemResources->AllocateART(rsrcConfId, rsrcPartyId, 0 /*artCapacity*/, ePhysical_art, pParam->serviceId, pParam->subServiceId, &rPortDesc, partyData, pParam->isIceParty, pParam->isBFCP, reqBoardId);
	if (status_alloc_art == STATUS_OK)  // get status and roll-back if needed?
	{
		DWORD connIdART = pSystemResources->AllocateConnId();
		if (0 == connIdART)
			DBGPASSERT(2);
		else
		{
			CRsrcDesc* pMixArtDesc = new CRsrcDesc(connIdART, eLogical_relay_avc_to_svc_rtp, rsrcConfId, rsrcPartyId, rPortDesc.m_boxId, rPortDesc.m_boardId, rPortDesc.m_subBoardId, rPortDesc.m_unitId, 0 /*accelerator_id*/, rPortDesc.m_portId);
			STATUS statusAddDesc = pConf->AddDesc(pMixArtDesc);
			DBGPASSERT(statusAddDesc != STATUS_OK);

			WORD numRsrcInResult = pResult->allocIndBase.numRsrcs;
			if (STATUS_OK == statusAddDesc)
			{
				WriteRsrcDescToSharedMemory(pMixArtDesc, cmEntry, msg);

				pResult->allocIndBase.allocatedRrcs[numRsrcInResult].connectionId = connIdART;
				pResult->allocIndBase.allocatedRrcs[numRsrcInResult].logicalRsrcType = eLogical_relay_avc_to_svc_rtp;
				pResult->allocIndBase.numRsrcs += 1;
			}
			else
			{
				pSystemResources->DeAllocateConnId(connIdART);

				const CRsrcDesc* pRelayAvcToSvcRtp = pConf->GetDesc(pParam->party_id, eLogical_relay_avc_to_svc_rtp);
				if (pRelayAvcToSvcRtp)
				{
					pResult->allocIndBase.allocatedRrcs[numRsrcInResult].connectionId = pRelayAvcToSvcRtp->GetConnId();
					pResult->allocIndBase.allocatedRrcs[numRsrcInResult].logicalRsrcType = eLogical_relay_avc_to_svc_rtp;
					pResult->allocIndBase.numRsrcs += 1;
				}
			}
			POBJDELETE(pMixArtDesc);
		}
	}
	return status_alloc_art;
}

////////////////////////////////////////////////////////////////////////////
bool CRsrcAlloc::isAVCHasHDRes(eVideoPartyType videoPartyType)
{
	switch (videoPartyType)
	{
		case eCP_H264_upto_HD720_30FS_Symmetric_video_party_type:
		case eCP_H264_upto_HD720_60FS_Symmetric_video_party_type:
		case eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type:
		case eCP_H264_upto_HD1080_60FS_Asymmetric_video_party_type:
		case eCP_H264_upto_HD1080_60FS_Symmetric_video_party_type:
			return true;
		default:
			return false;
	}
}

////////////////////////////////////////////////////////////////////////////
void CRsrcAlloc::CheckIfAllocOfAVCHDisSupportedAndUpdate(ALLOC_PARTY_REQ_PARAMS_S* pParam, ALLOC_PARTY_IND_PARAMS_S* pResult)
{
	pResult->allocIndBase.isAvcVswInMixedMode = FALSE;
	bool isAVCParty = !CHelperFuncs::IsVideoRelayParty(pParam->videoPartyType);
	if (isAVCParty && isAVCHasHDRes(pParam->videoPartyType) && pParam->HdVswTypeInMixAvcSvcMode)
	{
		CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
		PASSERTMSG_AND_RETURN(!pConfRsrcDB, "CRsrcAlloc::CheckIfAllocOfAVCHDisSupportedAndUpdate Null pointer operation (pConfRsrcDB)");

		DWORD monitor_conf_id = pParam->monitor_conf_id;
		DWORD monitor_party_id = pParam->monitor_party_id;
		DWORD rsrcPartyId = pConfRsrcDB->MonitorToRsrcPartyId(monitor_conf_id, monitor_party_id);
		DWORD rsrcConfId = pConfRsrcDB->MonitorToRsrcConfId(monitor_conf_id);

		CConfRsrc* pConfRsrc = pConfRsrcDB ? pConfRsrcDB->GetConfRsrc(monitor_conf_id) : NULL;

		CPartyRsrc* pPartyRsrc = (CPartyRsrc*)(pConfRsrc->GetPartyRsrcByRsrcPartyId(rsrcPartyId));
		if (pPartyRsrc == NULL)
		{
			PASSERT(1);
			PTRACE2INT(eLevelError, "CRsrcAlloc::CheckIfAllocOfAVCHDisSupportedAndUpdate req. for non-existing party, monitorId =", monitor_party_id);
			pResult->allocIndBase.status = STATUS_PARTY_NOT_EXISTS;
			return;
		}

		CConfRsrc* pConf = (CConfRsrc*)(pConfRsrcDB->GetConfRsrc(monitor_conf_id));
		if (pConf == NULL)
		{
			PASSERT(1);
			PTRACE2INT(eLevelError, "CRsrcAlloc::CheckIfAllocOfAVCHDisSupportedAndUpdate req. for non-existing conf., monitorId =", monitor_conf_id);
			pResult->allocIndBase.status = STATUS_CONFERENCE_NOT_EXISTS;
			return;
		}

		//Get MRMP board id of AVC translator
		int MRMPBoardId = -1;
		CRsrcDesc* pAllocatedMRMPDesc = (CRsrcDesc*)pConfRsrc->GetDesc(pPartyRsrc->GetRsrcPartyId(), eLogical_relay_rtp);
		if (pAllocatedMRMPDesc)
			MRMPBoardId = pAllocatedMRMPDesc->GetBoardId();
		else
		{
			TRACEINTO << " MRMP isn't allocated for AVC party, it is AVC only conference";
			return;
		}

		//Get art board id of AVC
		int artBoardId = -1;
		CPartyRsrc* pParty = (CPartyRsrc*)(pConfRsrc->GetPartyRsrcByRsrcPartyId(rsrcPartyId));

		CRsrcDesc* pAllocatedRtpDesc = (CRsrcDesc*)pConfRsrc->GetDesc(pPartyRsrc->GetRsrcPartyId(), eLogical_rtp);
		// in pstn - eLogical_rtp was not allocated so we use audio decoder for taking board id.
		if (NULL == pAllocatedRtpDesc)
		{
			pAllocatedRtpDesc = (CRsrcDesc*)pConfRsrc->GetDesc(pPartyRsrc->GetRsrcPartyId(), eLogical_audio_decoder);
			if (pAllocatedRtpDesc)
				artBoardId = pAllocatedRtpDesc->GetBoardId();
			else
			{
				TRACEINTO << " art isn't allocated for AVC party it should be allocated";
				return;
			}
		}
		else
			artBoardId = pAllocatedRtpDesc->GetBoardId();

		//Mckinsey: Here we check if the ART and MRMP is allocated on same board in order to know if IpMedia can pass the 720p stream to ART
		if (eProductFamilySoftMcu == CProcessBase::GetProcess()->GetProductFamily() || artBoardId == MRMPBoardId)
		{
			TRACEINTO << "Stream 720p can be passed from AVC to SVC because MRMP and ART on same board ART: " << artBoardId << " or product type is SoftMCU";
			pResult->allocIndBase.isAvcVswInMixedMode = TRUE;
		}
		else
		{
			TRACEINTO << "Stream 720p can't be passed from AVC to SVC because ART and MRMP isn't allocated on same board, ART board: " << artBoardId << " , MRMP board: " << MRMPBoardId << " or the party isn't HD AVC: " << (DWORD)pParam->videoPartyType;
			pResult->allocIndBase.isAvcVswInMixedMode = FALSE;
		}
	}
}

bool CRsrcAlloc::CheckBeforeUpgradePartyResourceUsage(CConfRsrc& rConfRsrc, PartyDataStruct& rPartyRsrc, eConfMediaState currentConfMediaState)
{
    CSystemResources* pSystResources = CHelperFuncs::GetSystemResources();
    ResourcesInterface* pResourcesInterface = pSystResources ? pSystResources->GetCurrentResourcesInterface() : NULL;
    PASSERT_AND_RETURN_VALUE( (!pSystResources || !pResourcesInterface), false);

    float logicalPartyWeightInNonMix = 0.0, logicalPartyWeightInMix = 0.0, totalWeightInMix = 0.0;
    WORD numConnectedParties = rConfRsrc.GetNumParties();

    pResourcesInterface->GetLogicalPartyWeight(rPartyRsrc.m_videoPartyType, logicalPartyWeightInNonMix, logicalPartyWeightInMix);
	float currentPartyWeight = logicalPartyWeightInMix;

    const std::set<CPartyRsrc>* pPartiesList = rConfRsrc.GetPartiesList();
    eVideoPartyType videoPartyType = eVideo_party_type_dummy;

    for (std::set<CPartyRsrc>::iterator partyItr = pPartiesList->begin(); partyItr != pPartiesList->end(); partyItr++)
    {
    	videoPartyType = partyItr->GetVideoPartyType();
    	pResourcesInterface->GetLogicalPartyWeight(videoPartyType, logicalPartyWeightInNonMix, logicalPartyWeightInMix);

    	TRACEINTO << "MFW_DEBUG : logicalPartyWeightInNonMix=" << logicalPartyWeightInNonMix << ", logicalPartyWeightInMix=" << logicalPartyWeightInMix;

    	totalWeightInMix += logicalPartyWeightInMix;
    }
    WORD numHD720PortsAccordingToCards = pSystResources->GetHD720PortsAccordingToCards();
	if (numHD720PortsAccordingToCards < (totalWeightInMix + currentPartyWeight))
	{
		TRACESTRFUNC(eLevelWarn) << "MFW_DEBUG : numHD720PortsAccordingToCards = " << numHD720PortsAccordingToCards << ", move to MIX need totalWeight="
								 << totalWeightInMix << ", currentPartyWeight=" << currentPartyWeight  << "==> Conference can't be moved to MIXED";
		return false;
	}
	//TRACEINTO << "MFW_DEBUG : numHD720PortsAccordingToCards = " << numHD720PortsAccordingToCards << ", move to MIX need totalWeight=" << totalWeightInMix << ", currentPartyWeight=" << currentPartyWeight;
	return true;
}

#undef TRACEINTO
#define TRACEINTO TRACESTRFUNC(eLevelInfoNormal)

