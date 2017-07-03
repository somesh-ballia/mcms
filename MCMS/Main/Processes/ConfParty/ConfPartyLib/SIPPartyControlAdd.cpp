//+========================================================================+
//                            SIPPartyControlAdd.CPP                       |
//            Copyright 2005 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPPartyControlAdd.CPP                                      |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+


#include "StatusesGeneral.h"
#include "DataTypes.h"
#include "StateMachine.h"
#include "Segment.h"
#include "NStream.h"
#include "ConfPartyDefines.h"
//#include "IpCommonTypes.h"
#include "IpCommonDefinitions.h"
#include "IpAddressDefinitions.h"
#include "IpScm.h"
#include "ConfPartyOpcodes.h"
#include "ConfDef.h"
#include "Conf.h"
#include "ConfApi.h"
#include "CommModeInfo.h"
#include "Capabilities.h"
#include "TaskApi.h"
#include "TaskApp.h"
#include "PartyApi.h"
#include "SipDefinitions.h"
#include "SipStructures.h"
#include "SipHeadersList.h"
#include "SipCsReq.h"
#include "SipCsInd.h"
#include "SipUtils.h"
#include "SipCaps.h"
#include "SipScm.h"
#include "SipCall.h"
#include "NetSetup.h"
#include "IpNetSetup.h"
#include "IpPartyControl.h"
#include "SipNetSetup.h"
#include "SIPCommon.h"
#include "SIPPartyControl.h"
#include "SIPPartyControlAdd.h"
#include "PartyApi.h"
#include "PartyRsrcDesc.h"
#include "BridgeMoveParams.h"
#include "ApiStatuses.h"
#include "FaultsDefines.h"
#include "HlogApi.h"
#include  "ConfPartyGlobals.h"
#include  "H263VideoMode.h"
#include "WrappersResource.h"  //to be removed noa
#include "ServiceConfigList.h"
#include "IpServiceListManager.h"
#include "SIPInternals.h"
#include "AudioBridgeInterface.h"
#include "ScpHandler.h"
#include "ConfPartyProcess.h"
#include "VideoOperationPointsSet.h"
#include "EnumsToStrings.h"

extern "C" void SipPartyOutCreateEntryPoint(void* appParam);
extern "C" void SipPartyOutEntryPointSimulation(void* appParam);
extern const char* GetStatusAsString(int status);
extern CIpServiceListManager* GetIpServiceListMngr();
//extern void DumpXMLAndHeadersToStream(CObjString* pMsgStr, sipSdpAndHeadersSt& sdpAndHeaders);
extern void DumpXMLToStream(std::ostream &ostr, sipSdpAndHeadersSt* sdpAndHeaders);


extern const char* MOC_PRODUCT_NAME; // This product name is compatible with "Microsoft Office Communicator" and "Microsoft Lync"

extern const char* MOC_PRODUCT_NAME; // This product name is compatible with "Microsoft Office Communicator" and "Microsoft Lync"


#define MFA_CONNECT_TIME	20
#define CONNECT_TIME		100 // all internal MCU units receive 5 seconds to reply, and the network has 50 second -> 5+5+50+5+5 <= 100
#define REALLOCATE_TO_LOWER_VIDEO 2

////////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CSipAddPartyCntl)

	ONEVENT(ALLOCATE_PARTY_RSRC_IND,		ALLOCATE_RSC,		CSipAddPartyCntl::OnPartyRsrcAllocatingRsp)
	ONEVENT(ACK_IND,						CREATE_MPL,			CSipAddPartyCntl::OnPartyMPLCreatingRsp)

	ONEVENT(SIP_PARTY_CHANS_CONNECTED,		PARTY_SETUP,		CSipAddPartyCntl::OnPartyChannelsConnectedSetup)

	ONEVENT(PARTY_AUDIO_CONNECTED,			CONNECT_BRIDGES,	CSipAddPartyCntl::OnAudConnectPartyConnectAudio)
	ONEVENT(PARTY_VIDEO_CONNECTED,			CONNECT_BRIDGES,	CSipAddPartyCntl::OnVideoBrdgConnected)
	ONEVENT(PARTY_VIDEO_IVR_MODE_CONNECTED,	CONNECT_BRIDGES,	CSipAddPartyCntl::OnVideoBrdgConnected)
	ONEVENT(IPPARTYCONNECTED,				CONNECT_BRIDGES,	CSipAddPartyCntl::OnPartyRemoteConnected)
	ONEVENT(PARTY_VIDEO_OUT_UPDATED,		CONNECT_BRIDGES,	CSipAddPartyCntl::OnVideoOutBrdgUpdated)
	ONEVENT(PARTY_VIDEO_DISCONNECTED,		CONNECT_BRIDGES,	CSipAddPartyCntl::OnVideoBrdgDisconnectedConnectBridges)
	ONEVENT(FECC_PARTY_BRIDGE_CONNECTED,	CONNECT_BRIDGES,	CSipAddPartyCntl::OnFeccBrdgCon)
	ONEVENT(PARTY_VIDEO_IN_UPDATED,			UPDATEVIDEO,		CSipPartyCntl::OnVideoInBrdgUpdated)
	ONEVENT(PARTY_VIDEO_OUT_UPDATED,		UPDATEVIDEO,		CSipPartyCntl::OnVideoOutBrdgUpdated)
	ONEVENT(SIP_PARTY_CHANS_CONNECTED,		PARTY_IN_CONNECTING_STATE,	CSipAddPartyCntl::OnPartyChannelsConnectedConnecting)
	ONEVENT(PARTY_VIDEO_IN_UPDATED,			UPDATE_BRIDGES_FOR_CHANNEL_HANDLE,	CSipPartyCntl::OnVideoInBrdgUpdatedChannelHandle)
	ONEVENT(PARTY_VIDEO_OUT_UPDATED,		UPDATE_BRIDGES_FOR_CHANNEL_HANDLE,	CSipPartyCntl::OnVideoOutBrdgUpdatedChannelHandle)

//  ONEVENT(AUDCONNECT,						SETUP,				CSipAddPartyCntl::OnAudioBrdgConnected)

	ONEVENT(PARTY_VIDEO_DISCONNECTED,		NEED_TO_REALLOCATE_RSC, CSipAddPartyCntl::OnVideoBrdgDisconnectedNeedToRealloc)

	ONEVENT(REALLOCATE_PARTY_RSRC_IND,      REALLOCATE_RSC,     CSipAddPartyCntl::OnRsrcReAllocatePartyRspReAllocate)

	ONEVENT(PARTY_VIDEO_CONNECTED,			CONNECT_VIDEO_BRIDGE_AFTER_REALLOC, CSipAddPartyCntl::OnVideoBrdgConnectedAfterRealloc)
	ONEVENT(PARTY_VIDEO_IVR_MODE_CONNECTED,	CONNECT_VIDEO_BRIDGE_AFTER_REALLOC,	CSipAddPartyCntl::OnVideoBrdgConnectedAfterRealloc)

	ONEVENT(CONNECTDELAY,					IDLE,				CSipAddPartyCntl::OnTimerConnectDelay)
	ONEVENT(CONNECTDELAY,					ALLOCATE_RSC,				CSipAddPartyCntl::OnTimerConnectDelay)
	ONEVENT(PARTYRMTCAP,					ANYCASE,			CSipAddPartyCntl::OnPartyRemoteCapsRecieved)

	ONEVENT(MFACONNECTTOUT,					ALLOCATE_RSC,		CSipAddPartyCntl::OnConnectToutAllocate)
	ONEVENT(MFACONNECTTOUT,					CREATE_MPL,			CSipAddPartyCntl::OnConnectToutCreate)
	ONEVENT(CONNECTTOUT,					PARTY_SETUP,		CSipAddPartyCntl::OnConnectToutPartySetup)
	ONEVENT(CONNECTTOUT,					CONNECT_BRIDGES,	CSipAddPartyCntl::OnConnectToutConnectBridges)
	ONEVENT(CONNECTTOUT,					UPDATEVIDEO,		CSipAddPartyCntl::OnConnectToutUpdateVideo)
	ONEVENT(CONNECTTOUT,					NEED_TO_REALLOCATE_RSC,	CSipAddPartyCntl::OnConnectToutNeedToRealloc)
	ONEVENT(CONNECTTOUT,					REALLOCATE_RSC,		CSipAddPartyCntl::OnConnectToutReAllocate)
	ONEVENT(CONNECTTOUT,					CONNECT_VIDEO_BRIDGE_AFTER_REALLOC,		CSipAddPartyCntl::OnConnectToutConnectBridgesAfterRealloc)
	ONEVENT(CONNECTTOUT,					PARTY_IN_CONNECTING_STATE,		CSipAddPartyCntl::OnConnectToutConnectingState)
	ONEVENT(WAIT_FOR_RSRC_AND_ASK_AGAIN,    ALLOCATE_RSC,       CSipAddPartyCntl::OnTimerWaitForRsrcAndAskAgainAllocate)
	ONEVENT(IPPARTYUPDATEBRIDGES,			ANYCASE,			CSipPartyCntl::OnPartyUpdateBridges)

	ONEVENT(PARTY_VIDEO_DISCONNECTED,		UPDATE_LEGACY_STATUS,	CSipAddPartyCntl::OnVideoBrdgDisconnectedUpdateLegacyStatus)
	ONEVENT(CONNECTTOUT,					UPDATE_LEGACY_STATUS,	CSipAddPartyCntl::OnConnectToutUpdateLegacyStatus)
	ONEVENT(PARTY_VIDEO_CONNECTED,			UPDATE_LEGACY_STATUS,	CSipAddPartyCntl::OnVideoBrdgConnectedUpdateLegacyStatus)
	ONEVENT(PARTY_VIDEO_IVR_MODE_CONNECTED,	UPDATE_LEGACY_STATUS,               CSipAddPartyCntl::OnVideoBrdgConnectedUpdateLegacyStatus)
	ONEVENT(PARTY_CONTROL_SLAVE_TO_MASTER_ACK,              ANYCASE,            CSipPartyCntl::OnSlaveToMasterAckMessage)

	ONEVENT(SEND_MRMP_STREAM_IS_MUST_REQ,                   ANYCASE,            CSipPartyCntl::OnMrmpStreamIsMustReq)
	ONEVENT(END_AVC_TO_SVC_ART_TRANSLATOR_DISCONNECTED,     REALLOCATE_RSC,     CPartyCntl::OnEndAvcToSvcArtTranslatorDisconnected)

	ONEVENT(MSORGANIZERENDCONNECT,                          CREATE_AVMCU_MNGRS, CSipAddPartyCntl::OnMSOrganizerEndConnection)
	ONEVENT(MSFOCUSENDCONNECT,                              CREATE_AVMCU_MNGRS, CSipAddPartyCntl::OnMSFocusEndConnection)
	ONEVENT(MSSUBSCRIBERENDCONNECT,                         CREATE_AVMCU_MNGRS, CSipAddPartyCntl::OnMSSubscriberEndConnection)

	ONEVENT(PARTY_CONTROL_MS_SLAVE_TO_MAIN_ACK,             ANYCASE,            CSipPartyCntl::OnMsSlaveToMainAckMessage)
    	ONEVENT(SIP_PARTY_VSR_MSG_IND,				ANYCASE,	    CSipPartyCntl::OnPartyVsrMsgInd)
PEND_MESSAGE_MAP(CSipAddPartyCntl,CSipPartyCntl);


////////////////////////////////////////////////////////////////////////////////
CSipAddPartyCntl::CSipAddPartyCntl()
{
	m_isFaulty = FALSE;
	m_isRecovery = FALSE;
	m_state    = IDLE;
	m_pTargetModeMaxAllocation = NULL;//For DPA
	m_MaxLocalCaps = NULL;
	SetInterfaceType(SIP_INTERFACE_TYPE);



	VALIDATEMESSAGEMAP
}


////////////////////////////////////////////////////////////////////////////////
CSipAddPartyCntl::~CSipAddPartyCntl()
{
     POBJDELETE(m_pTargetModeMaxAllocation);
     POBJDELETE(m_MaxLocalCaps);

}

////////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::Create(PartyControlInitParameters& partyControInitParam, PartyControlDataParameters& partyControlDataParams)
{
	SetupSIPConParameters(partyControInitParam, partyControlDataParams);
	ConnectSIP(partyControInitParam);
}

////////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::Reconnect(PartyControlInitParameters&	 partyControInitParam,PartyControlDataParameters &partyControlDataParams)
{
	PTRACE2PARTYID(eLevelInfoNormal, "CSipAddPartyCntl::Reconnect: Name - ", m_partyConfName, GetPartyRsrcId());
	SetupSIPConParameters(partyControInitParam,partyControlDataParams);
	SetInterfaceType(SIP_INTERFACE_TYPE);

	m_bIsUseSpeakerSsrcForTx_TRUE_sent = false;

	DialOut(partyControInitParam.redialInterval);
}
////////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::DialOut(DWORD redialInterval, BYTE eTransportType)
{
	if (FALSE == CheckNameValidityAndDisconnectIfNeeded())
		return;

	SetupSipEntryPoint();

	COsQueue ConfRcvMbx = m_pConf->GetRcvMbx();

	m_McuNumber = 1;
	m_termNum = 1;
	m_confName = m_pConf->GetName();
	m_pConfRcvMbx = &ConfRcvMbx;
	m_incomingVideoChannelHandle = INVALID_CHANNEL_HANDLE;
	m_outgoingVideoChannelHandle = INVALID_CHANNEL_HANDLE;
	m_isSignalingFirstTransactionCompleted = false;
	m_bVideoRelayOutReady = FALSE;
	m_bVideoRelayInReady = FALSE;
	m_pParty = (CTaskApp*)(m_monitorPartyId + 100);  // just to get unique id for party list

	if ((redialInterval && IsRedialImmediately() == NO) || m_connectDelay)
	{
		// try to reconnect the party after the timeout
		CSegment *pPartyDetails = new CSegment;
		ConfRcvMbx.Serialize(*pPartyDetails);

		*pPartyDetails << (char*)m_confName << m_termNum << m_McuNumber;

		if (m_connectDelay)
		{
			PTRACEPARTYID(eLevelInfoNormal, "CSipAddPartyCntl::DialOut: Blast state (Connection delay)", GetPartyRsrcId());
			StartTimer(CONNECTDELAY, m_connectDelay, pPartyDetails);
		}
		else
		{
			PTRACEPARTYID(eLevelInfoNormal, "CSipAddPartyCntl::DialOut: Redail state", GetPartyRsrcId());
			m_pTaskApi->UpdateDB(m_pParty, PARTYSTATE, PARTY_REDIALING);
			StartTimer(CONNECTDELAY, redialInterval, pPartyDetails);
		}
	}
	else
	{
		//Temporary ID before we get a response from the resource allocator
		StartPartyConnection();
	}
	m_disconnectionCause = 0;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CSipAddPartyCntl::CheckNameValidityAndDisconnectIfNeeded()
{
	const char* strConfName = m_pConf->GetName();
	BYTE bIsGoodName = ::IsGoodConfName(strConfName);

	if (bIsGoodName == NO)
	{
		PTRACE2PARTYID(eLevelInfoNormal,"CSipAddPartyCntl::CheckNameValidityAndDisconnectIfNeeded: Bad SIP name format - ",m_partyConfName, GetPartyRsrcId());

#ifdef LOOKUP_TABLE_DEBUG_TRACE
			TRACEINTO << "D.M. PartyId:" << GetPartyId() << ", monitorPartyId:" << m_monitorPartyId << ", pParty:" << std::hex << m_pParty;
#endif
		if (m_type == DIALOUT) //in case of dial in , we already have m_pParty
			m_pParty = (CTaskApp*)(m_monitorPartyId + 100);  // just to get unique id for party list

		CSegment *pSeg = new CSegment;
		*pSeg << m_name;
		m_pTaskApi->UpdateDB(NULL, DISCAUSE, SIP_BAD_NAME,1,pSeg);
		m_pTaskApi->EndAddParty(m_pParty, statIllegal);
	}

	return bIsGoodName;
}


/////////////////////////////////////////////////////////////////////////////
void  CSipAddPartyCntl::StartPartyConnection()
{
	m_connectingState = IP_CONNECTING;
//		InitTimer(m_pConf->GetRcvMbx());
		//new use for timer in Carmel - for all CH323AddPartyCntl flow - there is now one timer


	CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(m_monitorPartyId);

	if(pConfParty && (eMsftAvmcuNone == pConfParty->GetMsftAvmcuState()))
	{
		StartTimer(MFACONNECTTOUT, MFA_CONNECT_TIME*SECOND);
	}
	else
		TRACEINTO << "AVMCU party - Won't set timer yet";


	AllocatePartyResources();
}

/////////////////////////////////////////////////////////////////////////////
//Send CH323AddPartyCntl event to resource allocator process and changes state to ALLOCATE.
void CSipAddPartyCntl::AllocatePartyResources()
{
#ifdef PERFORMANCE_TEST
	m_Stopper.AddTime();
#endif //PERFORMANCE_TEST

	TRACEINTO << m_partyConfName << ", MonitorConfId:" << m_monitorConfId << ", MonitorPartyId:" << m_monitorPartyId << ", IsVoice:" << (int)m_voice;

	BOOL bEnableFreeVideoResources = GetSystemCfgFlagInt<BOOL>(m_serviceId, CFG_KEY_SIP_FREE_VIDEO_RESOURCES);

	EAllocationPolicy bIsFreeVideoResources = (bEnableFreeVideoResources && IsUndefinedParty() && !GetIsTipCall()) ? eAllowDowngradingToAudioOnly : eNoAllocationPolicy;

	BYTE portGaugeThresholdExceeded = FALSE;

	//WORD tipNumOfScreens = 0;
	eVideoPartyType localVideoPartyType = eVideo_party_type_none;

	if (m_bIsMrcCall)
	{
		if ((m_pConf->GetSessionTypeForResourceAllocator() == eVOICE_session) || (YES == m_voice))
			localVideoPartyType = eVoice_relay_party_type;
		else
			localVideoPartyType = GetRelayResourceLevel(false, NULL); //eVideo_relay_party_type;
	}
	else if (m_pIpInitialMode->GetConfMediaType()==eMixAvcSvcVsw)
	{
		if ((m_pConf->GetSessionTypeForResourceAllocator() == eVOICE_session) || (YES == m_voice))
			localVideoPartyType = eVoice_relay_party_type;
		else
			localVideoPartyType = GetRelayResourceLevel(true, NULL);  //eVSW_relay_party_type;
	}
	else if (m_bIsOfferer)
		localVideoPartyType = GetLocalVideoPartyType();
	else
	{
		eVideoPartyType localVideoPartyTypeWithoutH263 = GetLocalVideoPartyType(NO); // without H263 check
		if (localVideoPartyTypeWithoutH263 < eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type)
		{
			localVideoPartyType = GetLocalVideoPartyType();

			// compare resource type with resource type without H263.
			if ((localVideoPartyType > localVideoPartyTypeWithoutH263) && !m_pSipRemoteCaps->IsCapableOf4CIF())
			{
				// get resources according to 4CIF capabilities.
				localVideoPartyType = localVideoPartyTypeWithoutH263;
			}
		}
		else
		{
			if (m_eMaxVideoPartyTypeForTipFailure != eVideo_party_type_dummy)
				localVideoPartyType = max(localVideoPartyTypeWithoutH263, m_eMaxVideoPartyTypeForTipFailure);
			else
				localVideoPartyType = localVideoPartyTypeWithoutH263;
		}
	}

	TRACEINTO << "LocalVideoPartyType:" << eVideoPartyTypeNames[localVideoPartyType];

	if (!m_bIsOfferer)
	{
		BOOL isPortGaugeFlagOn = NO;

		CServiceConfigList* pServiceSysCfg = CProcessBase::GetProcess()->GetServiceConfigList();
		if (pServiceSysCfg)
			pServiceSysCfg->GetBOOLDataByKey(m_serviceId, "SEND_SIP_BUSY_UPONRESOURCE_THRESHOLD", isPortGaugeFlagOn);

		if (isPortGaugeFlagOn)
		{
			if (localVideoPartyType == eVideo_party_type_none)
			{
				portGaugeThresholdExceeded = TRUE;
				TRACEINTO << "SEND_SIP_BUSY_UPONRESOURCE_THRESHOLD is YES, eVideo_party_type_none";
			}
		}
	}

	ETipPartyTypeAndPosition tipPartyType = eTipNone;
	if (GetIsTipCall())
	{
		tipPartyType = eTipMasterCenter;

		if (DIALIN == m_type)
		{
			const char* strContact = m_pSIPNetSetup->GetRemoteSipContact();
			m_TipNumOfScreens = ExtractNumOfScreensFromContactHeader(strContact);
		}
	}

	m_artCapacity = CalculateArtCapacityAccordingToScm(m_pIpInitialMode, FALSE);
	BYTE bIsIceEnable = (m_pSIPNetSetup->GetEnableSipICE() || m_bIsWebRtcCall);
	CreateAndSendAllocatePartyResources(eIP_network_party_type, localVideoPartyType, bIsFreeVideoResources, portGaugeThresholdExceeded, bIsIceEnable, m_artCapacity, tipPartyType, 0xFFFF, TRUE, m_TipNumOfScreens);
	m_state = ALLOCATE_RSC;
}

//_e_m_
////////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::SetTelepresenseEPInfo()
{
        DWORD tipNumOfScreens = (DWORD)(-1);
	CMedString str = "";

	str<< "IsTipCall " << (int)GetIsTipCall() << "TipPartyType " << (int)m_TipPartyType <<" RoomId " << (int)m_RoomId;
	PTRACE2(eLevelInfoNormal, "CSipAddPartyCntl::SetTelepresenseEPInfo : ", str.GetString());

	PASSERTMSG_AND_RETURN(!m_telepresenseEPInfo, "m_telepresenseEPInfo == NULL");

	//if( GetIsTipCall() && DIALIN == m_type)
	//{
	//	const char* strContact = m_pSIPNetSetup->GetRemoteSipContact();
	//	if(strContact)
	//		tipNumOfScreens = ExtractNumOfScreensFromContactHeader(strContact);
	//}

	m_telepresenseEPInfo->SetNumOfLinks(tipNumOfScreens);
	m_telepresenseEPInfo->SetWaitForUpdate(TRUE);
	m_telepresenseEPInfo->SetRoomID(m_RoomId);
	m_telepresenseEPInfo->SetPartyMonitorID(m_monitorPartyId);
	m_telepresenseEPInfo->SetLinkNum(0);
}

////////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::OnPartyRsrcAllocatingRsp(CSegment* pParam)
{
#ifdef PERFORMANCE_TEST
	m_Stopper.AddTime();
#endif //PERFORMANCE_TEST

	CPartyRsrcDesc partyRsrcDesc;
	partyRsrcDesc.DeSerialize(SERIALEMBD, *pParam);

	if (m_pPartyHWInterface)
		POBJDELETE(m_pPartyHWInterface);

	DWORD status = partyRsrcDesc.GetStatus();
	eNetworkPartyType networkPartyType = partyRsrcDesc.GetNetworkPartyType();
	eVideoPartyType videoPartyType = partyRsrcDesc.GetVideoPartyType();
	eConfMediaType confMediaType = partyRsrcDesc.GetConfMediaType();

	TRACEINTO << m_partyConfName
			<< "\n  PartyId          :" << partyRsrcDesc.GetPartyRsrcId()
			<< "\n  RoomId           :" << partyRsrcDesc.GetRoomId()
			<< "\n  VideoPartyType   :" << eVideoPartyTypeNames[videoPartyType]
			<< "\n  ConfMediaType    :" << ConfMediaTypeToString(confMediaType)
			<< "\n  NetworkPartyType :" << eNetworkPartyTypeNames[networkPartyType]
			<< "\n  Status           :" << CProcessBase::GetProcess()->GetStatusAsString(status).c_str() << " (" << status << ")";

	//============================================================================================
	// BRIDGE-10439
	// PartyCntl must know the rsrcId unconditionally, as even if the resource-allocation fails,
	// the party will try to remove itself from the lists using its resource ID.
	//============================================================================================
	POBJDELETE(m_pPartyAllocatedRsrc);
	m_pPartyAllocatedRsrc = new CPartyRsrcDesc(partyRsrcDesc);

	if (status != STATUS_OK)
	{
		if (DIALOUT == m_type)
			ClearUnusedPartyId(partyRsrcDesc.GetPartyRsrcId());

		DeleteTimer(MFACONNECTTOUT);

	    //=====================================================================================================
	    // The following line got commented. On self kill the party will clear itself from the flag lists,
	    // so its a guaranteed assert to do the following line that will appear when the party will redo it:
	    // ClearUnusedPartyId(pTempPartyAllocatedRsrc->GetPartyRsrcId());
	    //=====================================================================================================

		if (STATUS_WAIT_FOR_RSRC_AND_ASK_AGAIN == status)
		{
			///Resource Allocator tries to fulfill the allocation request by reconfigurating the DSP, we will set a timer and retry to allocate the resources
			if (m_isWaitingForRsrcAndAskAgain)
			{
				WaitForRsrcAndAskAgain();
				return;
			}
			else
			{
				///We only retry to allocate resources in case of STATUS_WAIT_FOR_RSRC_AND_ASK_AGAIN just once
				PASSERTSTREAM(1, "PartyId:" << partyRsrcDesc.GetPartyRsrcId() << " - STATUS_WAIT_FOR_RSRC_AND_ASK_AGAIN but the m_isWaitingForRsrcAndAskAgain is set to NO");
			}
		}
		if (((STATUS_INSUFFICIENT_VIDEO_RSRC == status) || (STATUS_NUMBER_OF_VIDEO_PARTIES_EXCEEDED == status)) && (m_type == DIALOUT) && (m_connectDelayCounter < NUM_OF_CONNECT_DELAY_RETRIES))
		{
			TRACEINTO << "PartyId:" << partyRsrcDesc.GetPartyRsrcId() << " - Temporary video resources deficiency connection delay";

			CSegment *pPartyDetails = new CSegment;
			m_pConfRcvMbx->Serialize(*pPartyDetails);

			*pPartyDetails << (char*)m_confName << m_McuNumber << m_isRecording;

			m_connectDelayCounter++;
			StartTimer(CONNECTDELAY, 400, pPartyDetails); //It took 3sec to deAlloc unNecesary resource(worst case)
		}
		else
		{
			TRACEINTO << "PartyId:" << partyRsrcDesc.GetPartyRsrcId() << " - Allocation failed, do not continue process";

			CSegment seg;
			seg << m_name;
			m_pTaskApi->UpdateDB(NULL, DISCAUSE, RESOURCES_DEFICIENCY, 1, &seg); // Disconnect cause
			m_pTaskApi->EndAddParty(m_pParty, statIllegal);
		}

		partyRsrcDesc.DumpToTrace();
		return;
	}

	if (networkPartyType != eIP_network_party_type)
	{
		PASSERTSTREAM(1, "PartyId:" << partyRsrcDesc.GetPartyRsrcId() << " - Invalid network party type");

		CSegment seg;
		seg << m_name;
		m_pTaskApi->UpdateDB(NULL, DISCAUSE, RESOURCES_DEFICIENCY, 1, &seg); // Disconnect cause
		m_pTaskApi->EndAddParty(m_pParty, statIllegal);
		partyRsrcDesc.DumpToTrace();
		return;
	}

	if (GetIsTipCall())
	{
		if (videoPartyType < eCP_H264_upto_HD720_30FS_Symmetric_video_party_type)
		{
			PASSERTSTREAM(1, "PartyId:" << partyRsrcDesc.GetPartyRsrcId() << " - Invalid video party type");

			CSegment seg;
			seg << m_name;
			m_pTaskApi->UpdateDB(NULL, DISCAUSE, RESOURCES_DEFICIENCY, 1, &seg); // Disconnect cause
			m_pTaskApi->EndAddParty(m_pParty, statIllegal);
			partyRsrcDesc.DumpToTrace();
			return;
		}
	}

	pParam->Get((BYTE*)(&m_udpAddresses), sizeof(UdpAddresses));
	DumpUdpAddresses();
	m_pPartyAllocatedRsrc->DumpToTrace();
	m_RoomId = m_pPartyAllocatedRsrc->GetRoomId();
	//_e_m_
	SetTelepresenseEPInfo();

	ISDN_PARTY_IND_PARAMS_S tempIsdnParams;
	SVC_PARTY_IND_PARAMS_S svcParams;
	MS_SSRC_PARTY_IND_PARAMS_S msSsrcParams;
	pParam->Get((BYTE*)(&tempIsdnParams), sizeof(ISDN_PARTY_IND_PARAMS_S));
	pParam->Get((BYTE*)(&svcParams), sizeof(SVC_PARTY_IND_PARAMS_S));
	pParam->Get((BYTE*)(&msSsrcParams), sizeof(MS_SSRC_PARTY_IND_PARAMS_S));

	m_pIpInitialMode->SetConfMediaType(confMediaType);

	eVideoPartyType allocatedVideoPartyType = m_pPartyAllocatedRsrc->GetVideoPartyType();

	if (CheckResourceAllocationForMixMode() == FALSE)
		return;

	UpdateScmWithResources(svcParams,allocatedVideoPartyType, m_pPartyAllocatedRsrc->GetIsAvcVswInMixedMode());
	UpdateCapsWithMsftResources(msSsrcParams);

	m_MaxLocalCaps = new CSipCaps;  //for DPA save max caps and SCM before changing it according to alloc
	*m_MaxLocalCaps = *m_pSipLocalCaps;

	CheckResourceAllocationTowardsRequest(1);
	CheckResourceAllocationForTipAndUpdateRate(allocatedVideoPartyType);

	m_pTargetModeMaxAllocation = new CIpComMode(*m_pIpInitialMode);

	m_eLastAllocatedVideoPartyType = m_pPartyAllocatedRsrc->GetVideoPartyType();
	m_RoomId = m_pPartyAllocatedRsrc->GetRoomId();

	if (DIALOUT == m_type)
	{
		COsQueue ConfRcvMbx = m_pConf->GetRcvMbx();
		m_confName = m_pConf->GetName();

		// in dial out Now really allocate the party task
		m_pPartyApi = new CPartyApi;
		m_pPartyApi->Create(m_entryPoint, ConfRcvMbx, ConfRcvMbx, m_pPartyAllocatedRsrc->GetPartyRsrcId(), m_monitorPartyId, m_monitorConfId, "", m_name, m_confName, m_serviceId, m_termNum, m_McuNumber, "", m_voice, 1, m_IsGateWay);

		m_pParty = m_pPartyApi->GetTaskAppPtr();
	}

#ifdef LOOKUP_TABLE_DEBUG_TRACE
	TRACEINTO << "D.K." << "  PartyName:" << m_name << ", PartyId:" << m_pParty->GetPartyId() << ", IsDialOut:" << m_type << ", Pointer:" << std::hex << m_pParty;
#endif

	InsertPartyResourcesToGlobalRsrcRoutingTbl();

	CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(m_monitorPartyId);

	if (pConfParty && (eMsftAvmcuNone != pConfParty->GetMsftAvmcuState())) // In case this is AVMCU invasion we will stop the flow here...
	{
		CRsrcParams organizerRsrcParams = m_pPartyAllocatedRsrc->GetRsrcParams(eLogical_ip_sigOrganizer);
		CRsrcParams focusRsrcParams     = m_pPartyAllocatedRsrc->GetRsrcParams(eLogical_ip_sigFocus);

		if ((organizerRsrcParams.GetRsrcDesc()) && (focusRsrcParams.GetRsrcDesc()))
		{
			TRACEINTO << "State:" << enMsftAvmcuStateNames[pConfParty->GetMsftAvmcuState()] << " - This is AVMCU party, create AVMcu managers";
			CreateAVMCUMngrs();
			m_state = CREATE_AVMCU_MNGRS;
		}
		else
		{
			TRACEINTO << "State:" << enMsftAvmcuStateNames[pConfParty->GetMsftAvmcuState()] << " - This is AVMCU party. Error, no Sip Organizer or Focus";
			//ERROR HANDLING - ??
		}
	}
	else
	{
		//we can now send msgs to hw
		m_pPartyHWInterface = new CPartyInterface(m_pPartyAllocatedRsrc->GetPartyRsrcId(), m_pPartyAllocatedRsrc->GetConfRsrcId());
		m_pPartyHWInterface->SetRoomId(m_RoomId);

		m_lastReqId = SendCreateParty(E_NETWORK_TYPE_IP, m_bIsMrcCall);
		m_state = CREATE_MPL;

		//// in dial in we use it to send ringing
		CRsrcParams csRsrcParams = m_pPartyAllocatedRsrc->GetRsrcParams(eLogical_ip_signaling);
		m_pPartyApi->SendCsResourceAllocated(&csRsrcParams);
	}
}


////////////////////////////////////////////////////////////////////////////////
void  CSipAddPartyCntl::OnPartyMPLCreatingRsp(CSegment* pParam)
{
	OPCODE ackOpcode;
	DWORD  ackSeqNum;
	STATUS status;
	*pParam >> ackOpcode >> ackSeqNum >> status;
	CMedString str;

	DeleteTimer(MFACONNECTTOUT);

	if (ackOpcode == CONF_MPL_CREATE_PARTY_REQ)
	{
		if (status == STATUS_OK)
		{
			PTRACE2INT(eLevelInfoNormal, "CSipAddPartyCntl::OnPartyMPLCreatingRsp : CreateParty Ack Msg Received - ", m_disconnectionCause);
			BYTE eTransportType = 0;
			EstablishCall(eTransportType);
		}
		else
		{
			CLargeString cstr;
			cstr << "Party:" << GetPartyRsrcId() << " Conf:" << GetConfRsrcId();
			cstr << " receives Failure Status for opcode: TB_MSG_OPEN_PORT_REQ(CONF_MPL_CREATE_PARTY_REQ) ";
			cstr << " req:" << m_lastReqId;
			cstr << "( Responsibility: embedded )";

			DumpMcuInternalDetailed(cstr,ACK_FAILED);

			PTRACEPARTYID(eLevelInfoNormal, "CSipAddPartyCntl::OnPartyMPLCreatingRsp : ACK with fail : ", GetPartyRsrcId());
			str << "CSipAddPartyCntl::OnMplAckCreate : create Party failed, Party Rsrc ID - " << GetPartyRsrcId();
			//CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT, ACK_FAILED, MAJOR_ERROR_LEVEL, str.GetString(), TRUE);
			DBGPASSERT(status);
			m_isFaulty = 1; // Invoking KillPort process in RA.

			BYTE 	mipHwConn = (BYTE)eMipCardManager;
			BYTE	mipMedia = (BYTE)eMipArt;
			BYTE	mipDirect = eMipNoneDirction;
			BYTE	mipTimerStat = eMipStatusFail;
			BYTE	mipAction = eMipOpen;

			CSegment* pSeg = new CSegment;
			*pSeg << mipHwConn << mipMedia << mipDirect << mipTimerStat << mipAction;
			DWORD MpiErrorNumber = GetMpiErrorNumber(pSeg);
			m_pTaskApi->PartyDisConnect(MCU_INTERNAL_PROBLEM,m_pParty,NULL,MpiErrorNumber);
			POBJDELETE(pSeg);

		}
	}
	else
	{
		str << "CSipAddPartyCntl::OnMplAckCreate : Invalid Opcode, Response Opcode - " << ackOpcode;
		CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT, INVALID_OPCODE_RESPONSE, MAJOR_ERROR_LEVEL, str.GetString(), TRUE);
		PTRACE2INT(eLevelInfoNormal, "CSipAddPartyCntl::OnPartyMPLCreatingRsp : unexpected opcode %d", ackOpcode);
	}
}

////////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::EstablishCall(BYTE eTransportType)
{
#ifdef PERFORMANCE_TEST
	m_Stopper.AddTime();
#endif //PERFORMANCE_TEST

	int status=STATUS_OK;

	PTRACEPARTYID(eLevelInfoNormal,"CSipAddPartyCntl::EstablishCall", GetPartyRsrcId());

	m_state = PARTY_SETUP;

	StartTimer(CONNECTTOUT, CONNECT_TIME*SECOND);

	const char* alternativeAddrStr;

	if (m_strAlternativeAddr.IsEmpty() )
	{
		 alternativeAddrStr = NULL;
	}
	else
	{
		alternativeAddrStr = m_strAlternativeAddr.GetString();
	}

	eVideoPartyType eCurrentVideoType = GetMaxCurrentCallVideoPartyTypeForAllocation(m_pIpInitialMode,TRUE);
	TRACEINTO << "ConfMediaType:" << ConfMediaTypeToString(m_pIpInitialMode->GetConfMediaType()) << ", bIsMrcCall:" << (WORD)m_bIsMrcCall;
	if(m_pIpInitialMode->GetConfMediaType() == eMixAvcSvc)
	{
		TRACEINTO << "mix_mode - already in mix mode, reset flags";
		m_deferUpgrade = false;
		m_bPartyInUpgradeProcess = false;
	}
	//m_pIpInitialMode->Dump("CSipAddPartyCntl::EstablishCall scm ",eLevelInfoNormal);

	CRsrcParams* pMrmpRsrcParams=NULL;
	// BRIDGE-5386
    CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();
	if (m_bIsMrcCall || (eCurrentVideoType != eVideo_party_type_none && eCurrentVideoType!= eCOP_party_type  &&
			       (m_pIpInitialMode->GetConfMediaType() == eMixAvcSvc  || m_pIpInitialMode->GetConfMediaType()==eMixAvcSvcVsw))
			       || (m_pIpInitialMode->GetConfMediaType() == eMixAvcSvc && eProductFamilyRMX == CProcessBase::GetProcess()->GetProductFamily()))
	{
		pMrmpRsrcParams = new CRsrcParams;

		*pMrmpRsrcParams = m_pPartyAllocatedRsrc->GetRsrcParams(eLogical_relay_rtp); // eyaln
		if (pMrmpRsrcParams->GetRsrcDesc())
		{
			if (pMrmpRsrcParams->GetRsrcDesc()->GetConnectionId() == 0)
			{
				PTRACE(eLevelInfoNormal, "SipAddPartyCntl::EstablishCall eLogical_relay_rtp wasn't allocated1");
				if( m_pIpInitialMode->GetConfMediaType() == eMixAvcSvc)
				{
					TRACEINTO<<" dynMixedErr eLogical_relay_rtp wasn't allocated1";
					status=STATUS_FAIL;
				}
				POBJDELETE(pMrmpRsrcParams);
				pMrmpRsrcParams=NULL;
				PASSERT(101);
			}
		}
		else
		{
			PTRACE(eLevelInfoNormal, "CSipAddPartyCntl::EstablishCall eLogical_relay_rtp wasn't allocated2");
			if( m_pIpInitialMode->GetConfMediaType() == eMixAvcSvc)
			{
				TRACEINTO<<" dynMixedErr eLogical_relay_rtp wasn't allocated2";
				status=STATUS_FAIL;
			}

			PASSERT(101);
		}
	}
	else
	{
		TRACEINTO << "ConfMediaType:" << ConfMediaTypeToString(m_pIpInitialMode->GetConfMediaType()) << ", bIsMrcCall:" << (WORD)m_bIsMrcCall << ", CurrentVideoType:" << (int)eCurrentVideoType << " - MRMP not allocated";
	}

	eLogicalResourceTypes logicalType[NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS] =
		{eLogical_relay_avc_to_svc_rtp_with_audio_encoder, eLogical_relay_avc_to_svc_rtp};

	int cnt=0;

	CRsrcParams* avcToSvcTranslatorRsrcParams[NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS];

	for (int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
	{
		avcToSvcTranslatorRsrcParams[i]=NULL;
	}

	if (!m_bIsMrcCall && m_pIpInitialMode->GetConfMediaType() == eMixAvcSvc)
	{
		for (int i=0; i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS; ++i)
		{
			WORD itemNum=1;
			WORD found;
			avcToSvcTranslatorRsrcParams[i]=new CRsrcParams;
			found=m_pPartyAllocatedRsrc->GetRsrcParams(*avcToSvcTranslatorRsrcParams[i],logicalType[i],itemNum);

			if(found)
			{
				cnt++;
				TRACEINTO << "!@# good:  translator: " << i << " will be opened";
			}
			else
			{
				POBJDELETE(avcToSvcTranslatorRsrcParams[i]);
				avcToSvcTranslatorRsrcParams[i]=NULL;
				TRACEINTO << "!@# translator: " << i << " will not be opened";
			}
		}
	}

	CRsrcParams* pMfaRsrcParams = new CRsrcParams;
    *pMfaRsrcParams = m_pPartyAllocatedRsrc->GetRsrcParams(eLogical_rtp); // for content RTP

    CRsrcParams* pCsRsrcParams = new CRsrcParams;
	*pCsRsrcParams = m_pPartyAllocatedRsrc->GetRsrcParams(eLogical_ip_signaling);
	BYTE  bIsIpOnlyConf = TRUE;

	WORD refMcuNumber = 1;
	WORD refTerminalNumber = 1;
	if (m_pTerminalNumberingManager)
   	{
   		m_pTerminalNumberingManager->allocatePartyNumber(m_pParty, refMcuNumber, refTerminalNumber);
   		ON(m_isTerminalNumberingConn);
   	}
	else
		PASSERTMSG(GetPartyRsrcId(),"Terminal numbering manager not valid");

	m_pSipLocalCaps->SetConfUserIdForBfcp(refMcuNumber, refTerminalNumber);
	m_pIpInitialMode->SetConfUserIdForBfcp(refMcuNumber, refTerminalNumber);

	CLargeString *pStr = new CLargeString;
		*pStr << "sipaddparty EstablishCall IP v4	  			 = " << CIPV4Wrapper(m_udpAddresses.IpV4Addr)  <<'\n'
		<< "IP v6 		 		 = " << CIPV6AraryWrapper(m_udpAddresses.IpV6AddrArray)  <<'\n';
	//PTRACE(eLevelInfoNormal, pStr->GetString());
	POBJDELETE(pStr);

	if (status==STATUS_OK)
	{
    	m_pPartyApi->SipConfEstablishCall(avcToSvcTranslatorRsrcParams,pMrmpRsrcParams/*switch *//*pMrmpRsrcParams*/,  pMfaRsrcParams/*pMfaRsrcParams*/, pCsRsrcParams, m_udpAddresses,(CSipNetSetup *) m_pSIPNetSetup,
		m_pSipLocalCaps, m_pIpInitialMode, m_pQos, m_eConfTypeForAdvancedVideoFeatures,
		m_strConfParamInfo.GetString(), eTransportType, refMcuNumber, refTerminalNumber,
		m_pCopVideoTxModes, alternativeAddrStr,m_bNoVideRsrcForVideoParty,
		m_pTargetModeMaxAllocation,m_MaxLocalCaps,m_RoomId, m_SsrcIdsForAvcParty,
		m_bIsMrcCall,m_IsAsSipContentEnable, m_partyContentRate);

		m_pTaskApi->UpdateDB(m_pParty, PARTYSTATE, PARTY_CONNECTING);

		if (m_bIsMrcCall)
		{
			m_pScpHandler->Init(m_pPartyApi);
		}

	}

	for (int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
	{
		POBJDELETE(avcToSvcTranslatorRsrcParams[i]);
	}
	POBJDELETE(pMrmpRsrcParams);
	POBJDELETE(pMfaRsrcParams);
	POBJDELETE(pCsRsrcParams);

}

////////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::OnTimerConnectDelay(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipAddPartyCntl::OnTimerConnectDelay", GetPartyRsrcId());
	m_connectingState = IP_CONNECTING;
	m_connectDelay = 0;

    //Temporary ID before we get a response from the resource allocator
#ifdef LOOKUP_TABLE_DEBUG_TRACE
	TRACEINTO << "D.K. m_pParty:" << std::hex << m_pParty;
#endif
	m_pParty = (CTaskApp*)(m_monitorPartyId + 100);  // just to get unique id for party list
	StartPartyConnection();
}


/////////////////////////////////////////////////////////////////////////////
// The function different between offerer and not offerer (dial in with sdp) since the two types of calls handles resources differently.
void CSipAddPartyCntl::OnPartyChannelsConnectedSetup(CSegment* pParam) // channels connected or updated
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipAddPartyCntl::OnPartyChannelsConnectedSetup", GetPartyRsrcId());
	m_pIpCurrentMode->DeSerialize(NATIVE,*pParam);

	if (m_bIsMrcCall || m_pIpInitialMode->GetConfMediaType() == eMixAvcSvc || m_pIpInitialMode->GetConfMediaType()==eMixAvcSvcVsw)
    {
        *pParam >>m_incomingVideoChannelHandle ;
        *pParam >>m_outgoingVideoChannelHandle ;

        TRACEINTOFUNC << "mix_mode:  m_incomingVideoChannelHandle=" << m_incomingVideoChannelHandle
			<< " m_outgoingVideoChannelHandle=" << m_outgoingVideoChannelHandle;
    }

	if (m_bIsOfferer)
	{
		// In Offerer case (SIP dial out and SIP dial in without SDP) we connect the bridges before we open out channels therefore before sending
		// the bridge the command we duplicate the receive values.
		if (m_pIpCurrentMode->GetConfType() == kCop)
		{
			// In cop, we set the video transmit as the highest level encoder parameters (level 0):
			m_pIpCurrentMode->SetMediaMode((CMediaModeH323&)(*m_pCopVideoTxModes->GetVideoMode(0)), cmCapVideo, cmCapTransmit, kRolePeople);
			m_pIpCurrentMode->SetCopTxLevel(0);
		}
		else // svc check - we need it for svc call???
		{
			m_pIpCurrentMode->CopyMediaModeToOppositeDirection(cmCapVideo,cmCapReceive);
			UpdateVideoTxModeForAsymmetricModes();
		}
		m_pIpCurrentMode->CopyMediaModeToOppositeDirection(cmCapAudio,cmCapReceive);
		m_pIpCurrentMode->CopyMediaModeToOppositeDirection(cmCapData,cmCapReceive);
		ConnectBridgesSetup();
	}
	else
	{
		m_isSignalingFirstTransactionCompleted = true;
		HandleReAllocationForNotOfferer();
	}
}

/////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::HandleReAllocationForNotOfferer()
{
	PTRACE(eLevelInfoNormal,"CSipAddPartyCntl::HandleReAllocationForNotOfferer");

    //in MS enviroment - MOC always keep its resources
    BOOL bEnableFreeVideoResources = GetSystemCfgFlagInt<BOOL>(m_serviceId, CFG_KEY_SIP_FREE_VIDEO_RESOURCES);

	char * bIsMsMoc = strstr(m_productId, MOC_PRODUCT_NAME);

	m_state = REALLOCATE_RSC;
	m_pIpCurrentMode->Dump("CSipAddPartyCntl::HandleReAllocationForNotOfferer");
	eVideoPartyType eCurrentVideoType = GetMaxCurrentCallVideoPartyTypeForAllocation(m_pIpCurrentMode,TRUE);
	TRACEINTO << "Deescalation: eCurrentVideoType=" << eCurrentVideoType;

	if (eCurrentVideoType == eVideo_party_type_none && !m_voice &&
		((CProcessBase::GetProcess()->GetProductType() != eProductTypeNinja) && !IsUndefinedParty()))// in case of m_voice the IsNeedTo... function will return FALSE
	{
		eCurrentVideoType = GetLowestVideoAllocationAccordingToSystemMode (bIsMsMoc);
	    TRACEINTO << "Deescalation: eCurrentVideoType=" << eCurrentVideoType;
	}
    else if (!bEnableFreeVideoResources)
    {
		if (eCurrentVideoType == eVideo_party_type_none)
			eCurrentVideoType = GetLowestVideoAllocationAccordingToSystemMode (bIsMsMoc);

		TRACEINTO << "Deescalation: eCurrentVideoType=" << eCurrentVideoType;
    }

	if (eCurrentVideoType == eVideo_party_type_none)
	 {
	    // romem
		if (m_bNoVideRsrcForVideoParty && !IsRemoteCapNotHaveVideo())
	    {
	        PTRACE2(eLevelInfoNormal, "CSipAddPartyCntl::HandleReAllocationForNotOfferer - Party was already audio only, cancel m_bNoVideRsrcForVideoParty, Name: ",m_partyConfName);
	        m_pPartyApi->UpdateNoResourcesForVideoParty(m_bNoVideRsrcForVideoParty);
	        m_bNoVideRsrcForVideoParty = FALSE;
	     }
	}


	//KW 2402
	CConfParty* pConfParty = NULL;
	BYTE 		protocol   = 0;
	if(m_pConf && m_pConf->GetCommConf())
	{
		pConfParty = m_pConf->GetCommConf()->GetCurrentParty(m_monitorPartyId);
		PASSERTMSG(!pConfParty,"!pConfParty");
		protocol = (pConfParty) ? pConfParty->GetVideoProtocol() : 0;
	}

	eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();

	if (protocol == VIDEO_PROTOCOL_H261  && systemCardsBasedMode == eSystemCardsMode_mpmrx
		&& (((CapEnum)m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapTransmit,kRolePeople) == eH261CapCode)
		|| ((CapEnum)m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapReceive,kRolePeople) == eH261CapCode)))
	{
		PTRACE(eLevelInfoNormal,"CIpPartyCntl::GetLocalVideoPartyType - force H261 resolution on mpmRx- Resource is as HD1080 30");
		eCurrentVideoType = eCP_H261_CIF_equals_H264_HD1080_video_party_type;
	}

	TRACEINTO << "before calling IsNeedToChangeVideoResourceAllocation eCurrentVideoType="
		<< eCurrentVideoType;
	BYTE bChangeResources = IsNeedToChangeVideoResourceAllocation(eCurrentVideoType);

	if (m_pIpInitialMode->GetConfType() == kCp &&  bEnableFreeVideoResources
		&& eCurrentVideoType == eVideo_party_type_none
		&& m_eLastAllocatedVideoPartyType != eVideo_party_type_none && IsUndefinedParty())//DPA
    	bChangeResources  = TRUE;   //DPA

    // we can't reduce to VoIP in re-allocation only in allocation for SIP dial in.
	DWORD artCapacity = CalculateArtCapacityAccordingToScm(m_pIpCurrentMode, TRUE /*add audio + video for current*/);

    if (bChangeResources)
    {
		m_artCapacity = artCapacity;

		CreateAndSendReAllocatePartyResources(eIP_network_party_type, eCurrentVideoType, eAllocateAllRequestedResources,FALSE,0,m_pSIPNetSetup->GetEnableSipICE(),artCapacity, m_TipPartyType);
    }
    else
    {
		if(m_artCapacity != artCapacity)
			CreateAndSendReAllocateArtForParty(eIP_network_party_type ,eCurrentVideoType, eAllocateAllRequestedResources,FALSE/*ICE*/,artCapacity, m_TipPartyType);

		ConnectBridgesSetup();
    }
}

/////////////////////////////////////////////////////////////////////////////
// The function connect bridges (A+V+FECC) after in channels were connected in dial out calls or
// in and out channels were connected in dial in calls.
void CSipAddPartyCntl::ConnectBridgesSetup() // channels connected or updated
{
	PTRACE(eLevelInfoNormal,"CSipAddPartyCntl::ConnectBridgesSetup");

	DWORD initialRate	= m_pIpInitialMode->GetMediaBitRate(cmCapVideo,cmCapReceive);
	DWORD currentRate	= m_pIpCurrentMode->GetMediaBitRate(cmCapVideo,cmCapReceive);
	BYTE  bEqualRates	= (initialRate == currentRate);
	BYTE  bIsCp			= (m_pIpInitialMode->GetConfType() == kCp);// (BYTE) m_pScm->IsFreeVideoRate();
    BYTE  bIsCop        = (m_pIpInitialMode->GetConfType() == kCop);
    BYTE  bIsVswRelay   = (m_pIpInitialMode->GetConfMediaType()==eMixAvcSvcVsw && !m_bIsMrcCall);
	// CSmallString str;
	// str << m_partyConfName << " conf type - " << m_pIpInitialMode->GetConfType() << " init rate - " << initialRate << " current rate - " << currentRate;

	if (!bIsVswRelay && (bIsCp || bEqualRates))
	{// since we copy media to opposite direction we have video rate in transmit direction at all scenarios
		DWORD videoRate = 0;
			videoRate = m_pIpCurrentMode->GetMediaBitRate(cmCapVideo,cmCapTransmit);
		SetNewVideoRates(videoRate);
	}

	EConfType confType = m_pIpInitialMode->GetConfType();
	*m_pIpInitialMode = *m_pIpCurrentMode;
	m_pIpInitialMode->SetConfType(confType);

	// connect bridges
	if (m_eAudBridgeConnState == eBridgeDisconnected) //in SIP we connect both directions
		ConnectPartyToAudioBridge(m_pIpCurrentMode);

	// connect video bridge only on CP or equal mode in VSW
	if (m_pIpCurrentMode->IsMediaOn(cmCapVideo) && m_eConfTypeForAdvancedVideoFeatures == NO &&
		(bIsVswRelay || bIsCp || bIsCop || bEqualRates) && (m_eVidBridgeConnState == eBridgeDisconnected))
		ConnectPartyToVideoBridge(m_pIpCurrentMode);

	else if (m_eConfTypeForAdvancedVideoFeatures)
		PTRACEPARTYID(eLevelInfoNormal,"CSipAddPartyCntl::ConnectBridgesSetup : Video advanced features do not connect the video bridge", GetPartyRsrcId());

	else if (bEqualRates == NO)
		PTRACEPARTYID(eLevelInfoNormal,"CSipAddPartyCntl::ConnectBridgesSetup : Current video is not suit the VSW conf rate. Do not connect the video bridge", GetPartyRsrcId());

	if (!m_isFeccConn)
		ConnectPartyToFECCBridge(m_pIpCurrentMode);

	m_state = CONNECT_BRIDGES;
}

/////////////////////////////////////////////////////////////////////////////
// The function connect party to conf after out channels of Offerer calls were connected
void CSipAddPartyCntl::OnPartyChannelsConnectedConnecting(CSegment* pParam) // channels connected or updated
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipAddPartyCntl::OnPartyChannelsConnectedConnecting", GetPartyRsrcId());
	if(m_bIsMrcCall)
	{	// tmp patch because sip-party don't send updated scm in receive (no update flow with MRMP)
		// so we keep the receive scm
		CIpComMode  tmpIpCurrentMode;
		tmpIpCurrentMode.DeSerialize(NATIVE,*pParam);
		m_pIpCurrentMode->CopyMediaMode(tmpIpCurrentMode,cmCapAudio,cmCapTransmit);
		m_pIpCurrentMode->CopyMediaMode(tmpIpCurrentMode,cmCapVideo,cmCapTransmit);
		m_pIpCurrentMode->CopyMediaMode(tmpIpCurrentMode,cmCapData,cmCapTransmit);
		m_pIpCurrentMode->CopyMediaMode(tmpIpCurrentMode,cmCapVideo,cmCapReceiveAndTransmit,kRoleContentOrPresentation);
	}
	else
		m_pIpCurrentMode->DeSerialize(NATIVE,*pParam);

	unsigned int incomingVideoChannelHandle;
	unsigned int outgoingVideoChannelHandle;
	*pParam >> incomingVideoChannelHandle;
	*pParam >> outgoingVideoChannelHandle;

	if (m_bIsMrcCall || m_pIpInitialMode->GetConfMediaType()==eMixAvcSvcVsw)
	{
	    if (m_incomingVideoChannelHandle != incomingVideoChannelHandle)
		{
			m_incomingVideoChannelHandle = incomingVideoChannelHandle;
			m_bVideoRelayInReady = FALSE;
		}
	    if (m_outgoingVideoChannelHandle != outgoingVideoChannelHandle)
		{
			m_outgoingVideoChannelHandle = outgoingVideoChannelHandle;
			m_bVideoRelayOutReady = FALSE;
		}
	    TRACEINTOFUNC<<"m_incomingVideoChannelHandle=" << m_incomingVideoChannelHandle << " m_outgoingVideoChannelHandle=" << m_outgoingVideoChannelHandle;
	}

	if(!UpdateBridgesForChannelHandle())
		m_pTaskApi->EndAddParty(m_pParty,statOK);
}

////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::HandleVideoBridgeUpdateForChannelHandle(WORD status, EUpdateBridgeMediaAndDirection eUpdatedBridges)
{
	BYTE bVideoBridgeEndUpdate = CheckVideoBridgeEndUpdate(status, eUpdatedBridges);
	TRACEINTO << "bVideoBridgeEndUpdate=" << (int)bVideoBridgeEndUpdate;
	if (bVideoBridgeEndUpdate)
		m_pTaskApi->EndAddParty(m_pParty,statOK);
}

/////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::OnPartyRemoteCapsRecieved(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal, "CSipAddPartyCntl::OnPartyRemoteCapsRecieved", GetPartyRsrcId());
	CSipComMode* pRemoteMode = new CSipComMode;
	pRemoteMode->DeSerialize(NATIVE, *pParam);

	// VNGR-6679 - Solution
	m_pSipRemoteCaps->CleanAll();
	m_pSipRemoteCaps->DeSerialize(NATIVE, *pParam);

	if(m_bIsTipCall && (m_pSipRemoteCaps->GetNumOfCapSets() != 0) && (!::CheckIfRemoteSdpIsTipCompatible(m_pSipRemoteCaps, TRUE) ) )
	{
		PTRACE(eLevelInfoNormal,"CSipAddPartyCntl::OnPartyRemoteCapsRecieved -moving from TIP type to NON TIP");
		m_pIpInitialMode->SetTipMode(eTipModeNone);
		m_bIsTipCall = FALSE;
	}

	if ( kVideoSwitch == m_pIpInitialMode->GetConfType() && m_pIpInitialMode->IsAutoVideoResolution() )
	{
		PTRACEPARTYID(eLevelInfoNormal,"CSipAddPartyCntl::OnPartyRemoteCapsRecieved: Auto VSW", GetPartyRsrcId());
		//No video support in VSW AUTO video mode because no support for change mode for SIP
		//Erase remote video caps from m_pSipRemoteCaps and fill with local mode if exist.
		m_pSipRemoteCaps->CleanMedia(cmCapVideo);
		CBaseCap* pVideoIn = pRemoteMode->GetMediaAsCapClass(cmCapVideo,cmCapReceive);
		if ( pVideoIn )
		{
			m_pSipRemoteCaps->AddCapSet(cmCapVideo, pVideoIn);
		}
		POBJDELETE(pVideoIn);
	}

	m_isRemoteCapReady = YES;
	POBJDELETE(pRemoteMode);
}


/////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::OnPartyRemoteConnected(CSegment* pParam)
{
#ifdef PERFORMANCE_TEST
	m_Stopper.AddTime();
#endif //PERFORMANCE_TEST

	PTRACEPARTYID(eLevelInfoNormal,"CSipAddPartyCntl::OnPartyRemoteConnected", GetPartyRsrcId());
	CSipComMode* pRemoteMode = new CSipComMode;
	pRemoteMode->DeSerialize(NATIVE, *pParam);
	BYTE bIsEndConfChangeMode = FALSE;
	*pParam >> bIsEndConfChangeMode;

	if ( m_bIsOfferer )
	{// case of offerer (dial out and dial in invite no SDP)
		m_isSignalingFirstTransactionCompleted = true;
		OffererPartyConnected(pRemoteMode);
	}
	else
	{// case of dial in with SDP
		AnswererPartyConnected();
	}

	POBJDELETE(pRemoteMode);
}

/////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::OffererPartyConnected(CSipComMode* pRemoteMode)
{
	// meaning we received 200 ok with
	// if guess succeed than remote mode is current mode
	EConfType confType = m_pIpInitialMode->GetConfType();
	eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
	*m_pIpInitialMode = *pRemoteMode;
	m_pIpInitialMode->SetConfType(confType);

	if(m_bIsTipCall && m_pIpInitialMode->GetIsTipMode() == FALSE)
	{
		PTRACE(eLevelInfoNormal,"CSipAddPartyCntl::OffererPartyConnected -moving from TIP type to NON TIP");
		m_bIsTipCall = FALSE;
	}
      //in MS enviroment - MOC always keep its resources
       BOOL bEnableFreeVideoResources = GetSystemCfgFlagInt<BOOL>(m_serviceId, CFG_KEY_SIP_FREE_VIDEO_RESOURCES);
 //      BOOL bMsEnviroment = GetSystemCfgFlagInt<BOOL>(m_serviceId, "MS_ENVIRONMENT");
       BOOL bMsEnviroment = FALSE;
   	CConfIpParameters* pService = ::GetIpServiceListMngr()->FindIpService(m_serviceId);
   	if( pService != NULL && pService->GetConfigurationOfSipServers() )
   	{
   		if(pService->GetSipServerType() == eSipServer_ms)
   			bMsEnviroment = TRUE;
   	}
   	PTRACE2INT(eLevelInfoNormal, "CSipAddPartyCntl::OffererPartyConnected bMsEnviroment ", bMsEnviroment );

       char * bIsMsMoc = strstr(m_productId, MOC_PRODUCT_NAME);
       if (bMsEnviroment && bIsMsMoc)
           bEnableFreeVideoResources = FALSE;
    // set scm with conf video quality
	eVideoQuality vidQuality = m_pConf->GetCommConf()->GetVideoQuality();

	UpdateAudioBridgesIfNeeded();

	eVideoPartyType eCurrentVideoType = GetMaxCurrentCallVideoPartyTypeForAllocation(m_pIpInitialMode,TRUE);
    TRACEINTO << "Deescalation: eCurrentVideoType=" << eCurrentVideoType;

	if (eCurrentVideoType == eVideo_party_type_none && !m_voice &&
		((CProcessBase::GetProcess()->GetProductType() != eProductTypeNinja) && !IsUndefinedParty()))// in case of m_voice the IsNeedTo... function will return FALSE
	{
		eCurrentVideoType = GetLowestVideoAllocationAccordingToSystemMode (bIsMsMoc);
        TRACEINTO << "Deescalation: eCurrentVideoType=" << eCurrentVideoType;
	}
	else if (!bEnableFreeVideoResources && eCurrentVideoType == eVideo_party_type_none)
	{
		eCurrentVideoType = GetLowestVideoAllocationAccordingToSystemMode (bIsMsMoc);
        TRACEINTO << "Deescalation: eCurrentVideoType=" << eCurrentVideoType;
	}

	if(  systemCardsBasedMode == eSystemCardsMode_mpmrx && ( eCurrentVideoType < eCP_H261_CIF_equals_H264_HD1080_video_party_type ) &&
		((CapEnum)m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapTransmit,kRolePeople) == eH261CapCode ) &&
		((CapEnum)m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapTransmit,kRolePeople) == eH261CapCode ) )
	{
		BOOL bH261Support = GetSystemCfgFlagInt<BOOL>("H261_SUPPORT_ALWAYS");
		if( !bH261Support )
		{
			eCurrentVideoType = eVideo_party_type_none;
		}
	}

	//KW 2403
	CConfParty* pConfParty = NULL;
	BYTE 		protocol   = 0;
	if(m_pConf && m_pConf->GetCommConf())
	{
		pConfParty = m_pConf->GetCommConf()->GetCurrentParty(m_monitorPartyId);
		PASSERTMSG(!pConfParty,"!pConfParty");
		protocol = (pConfParty) ? pConfParty->GetVideoProtocol() : 0;
	}

	if(protocol == VIDEO_PROTOCOL_H261  && systemCardsBasedMode == eSystemCardsMode_mpmrx && ( ((CapEnum)m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapTransmit,kRolePeople) == eH261CapCode ) || ((CapEnum)m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapReceive,kRolePeople) == eH261CapCode ) ) )
	{
		PTRACE(eLevelInfoNormal,"CIpPartyCntl::GetLocalVideoPartyType - force H261 resolution on mpmRx- Resource is as HD1080 30");
		eCurrentVideoType = eCP_H261_CIF_equals_H264_HD1080_video_party_type;
        TRACEINTO << "Deescalation: eCurrentVideoType=" << eCurrentVideoType;
	}

    TRACEINTO << "before calling IsNeedToChangeVideoResourceAllocation eCurrentVideoType=" << eCurrentVideoType;
	BYTE bChangeResources = IsNeedToChangeVideoResourceAllocation(eCurrentVideoType);

	PTRACE2INT(eLevelInfoNormal,"***CSipAddPartyCntl::OffererPartyConnected bChangeResources ",bChangeResources);

	// no need to add the part of the free resources for undefined party since in dial out all the calls are defined.
	if (bChangeResources)
	{
		//SD 2- disconnect from video bridge
		if (m_eVidBridgeConnState != eBridgeDisconnected)
		{
			DisconnectPartyFromVideoBridge();
			m_state = NEED_TO_REALLOCATE_RSC;
		}
		else
		{
			m_state = REALLOCATE_RSC;
			DWORD artCapacity = 0;
			artCapacity = CalculateArtCapacityAccordingToScm(m_pIpCurrentMode, TRUE /*add audio + video for current*/);
			m_artCapacity = artCapacity;
			CreateAndSendReAllocatePartyResources(eIP_network_party_type,eCurrentVideoType, eAllocateAllRequestedResources,FALSE,0,m_pSIPNetSetup->GetEnableSipICE(), artCapacity, m_TipPartyType);
		}
	}

	else
	{
		// In order to update the Legacy status, we need to disconnect and reconnect to video bridge.
		// In the future, Bridge will implement update of legacy, without needing disconnect and reconnect.
		DWORD artCapacity = 0;
		artCapacity = CalculateArtCapacityAccordingToScm(m_pIpInitialMode, TRUE /*add audio + video for current*/);
		if(m_artCapacity != artCapacity)
				CreateAndSendReAllocateArtForParty(eIP_network_party_type ,eCurrentVideoType, eAllocateAllRequestedResources,FALSE/*ICE*/,artCapacity, m_TipPartyType);

		BYTE bChangeLegacy = FALSE;
		eProductFamily prodFamily = CProcessBase::GetProcess()->GetProductFamily();
		BOOL bIsSoftMCUSystem = (prodFamily == eProductFamilySoftMcu);

		if (m_pConf->GetCommConf()->IsLegacyShowContentAsVideo() &&	!IsTipSlavePartyType() && !(m_bIsMrcCall || bIsSoftMCUSystem))
		{
			bChangeLegacy = !IsLegacyContentParty(); // Offerer Party was first legacy, so if now it isn't legacy we need to disconnect and reconnect.
		}
		if (bChangeLegacy && (m_eVidBridgeConnState != eBridgeDisconnected))
		{
			PTRACEPARTYID(eLevelInfoNormal,"CSipAddPartyCntl::OffererPartyConnected : Update Legacy", GetPartyRsrcId());
			m_state = UPDATE_LEGACY_STATUS;
			DisconnectPartyFromVideoBridge();
		}
		else
		{
			m_eUpdateState = eNoUpdate;
			BYTE bTakeInitial = TRUE;
			m_eUpdateState = UpdateVideoBridgesIfNeeded(bTakeInitial);
			if((m_eUpdateState & eUpdateVideoIn) || (m_eUpdateState & eUpdateVideoOut))
			{// audio bridge has no response on update, wait for video nridge response
				PTRACEPARTYID(eLevelInfoNormal,"CSipAddPartyCntl::OffererPartyConnected: (Bridge mode was different)", GetPartyRsrcId());
				m_state = UPDATEVIDEO;
				if ((m_pIpInitialMode->IsMediaOn(cmCapVideo, cmCapReceive, kRolePeople))
					&& (m_eFirstRxVideoCapCode != (CapEnum)(m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapReceive, kRolePeople))))
				{
					m_eFirstRxVideoCapCode = (CapEnum)(m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapReceive, kRolePeople));
					if(m_eFirstRxVideoCapCode == eH264CapCode)
							m_eFirstRxVideoProfile = m_pIpInitialMode->GetH264Profile(cmCapReceive);
					TRACEINTO << "Update m_eFirstRxVideoCapCode with real first connection mode - m_eFirstRxVideoCapCode:" << m_eFirstRxVideoCapCode << ", m_eFirstRxVideoProfile:" << m_eFirstRxVideoProfile;
				}
				if( (m_eUpdateState & eUpdateVideoIn) == FALSE )
				{
					BYTE IsSetSmartSwitchAccordingToVendoer = IsSetSmartSwitchForUser(m_productId);
					if( IsSetSmartSwitchAccordingToVendoer && m_pIpCurrentMode->GetConfType() == kCop )
					{
						PTRACEPARTYID(eLevelInfoNormal,"CSipAddPartyCntl::OffererPartyConnected: update bridge that EP is HDX8000 in cop case when no in update only out.", GetPartyRsrcId());
						m_pTaskApi->UpdateRemoteUseSmartSwitchAccordingToVendor(m_pParty);
					}

				}
			}
			else
				{
			            BYTE bIsChangeBridge = ChangeVideoBridgeStateAccordingToNewMode();
						if(bIsChangeBridge)
						{// no change in video bridge state, continue the flow by sending the ACK to the party
							PTRACEPARTYID(eLevelInfoNormal,"CSipAddPartyCntl::OffererPartyConnected: disconnect from video bridge (no video cap match). Wait for response.", GetPartyRsrcId());
						}
						else
						{
							EndOffererConnected();
							BYTE IsSetSmartSwitchAccordingToVendoer = IsSetSmartSwitchForUser(m_productId);
							if( IsSetSmartSwitchAccordingToVendoer &&  m_pIpCurrentMode->GetConfType() == kCop )
							{
								PTRACEPARTYID(eLevelInfoNormal,"CSipAddPartyCntl::OffererPartyConnected: update bridge that EP is HDX8000 in cop case.", GetPartyRsrcId());
								m_pTaskApi->UpdateRemoteUseSmartSwitchAccordingToVendor(m_pParty);
							}
						}
						//HERE IF remote is HDX8000 and this is COP we should know and update cop in bridge.
				}



		}



	}



}


//	AddSlaveParty(eTipSlaveAux, m_RoomId);


/////////////////////////////////////////////////////////////////////////////
//SD 3- disconnect v.b. ack
void CSipAddPartyCntl::OnVideoBrdgDisconnectedNeedToRealloc(CSegment* pParam)
{
	// can happened only in dial out scenario
	PTRACEPARTYID(eLevelInfoNormal,"CSipAddPartyCntl::OnVideoBrdgDisconnectedNeedToRealloc", GetPartyRsrcId());
	eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
    EStat resStat = HandleVideoBridgeDisconnectedInd(pParam);
	if (resStat == statVideoInOutResourceProblem)
	{
		m_pTaskApi->EndAddParty(m_pParty, statVideoInOutResourceProblem);
		DeleteTimer(CONNECTTOUT);
	}
	else
	{
		//SD 4- reallocate resources
        	BOOL bEnableFreeVideoResources = GetSystemCfgFlagInt<BOOL>(m_serviceId, CFG_KEY_SIP_FREE_VIDEO_RESOURCES);
        	//BOOL bMsEnviroment = GetSystemCfgFlagInt<BOOL>(m_serviceId, "MS_ENVIRONMENT");

        	char * bIsMsMoc = strstr(m_productId, MOC_PRODUCT_NAME);
        	//if (bMsEnviroment && bIsMsMoc)
        	 //   bEnableFreeVideoResources = FALSE;
		m_state = REALLOCATE_RSC;
		eVideoPartyType eCurrentVideoType = GetMaxCurrentCallVideoPartyTypeForAllocation(m_pIpInitialMode,TRUE);
		if (eCurrentVideoType == eVideo_party_type_none && !m_voice &&
			((CProcessBase::GetProcess()->GetProductType() != eProductTypeNinja) && !IsUndefinedParty()))// Dial out only case we can't go done to m_voice only secondary
		{
			eCurrentVideoType = GetLowestVideoAllocationAccordingToSystemMode (bIsMsMoc);
		}
                else if (!bEnableFreeVideoResources)
		{
        	        if( eCurrentVideoType == eVideo_party_type_none )
        	        	eCurrentVideoType = GetLowestVideoAllocationAccordingToSystemMode (bIsMsMoc);
		}

		if(systemCardsBasedMode == eSystemCardsMode_mpmrx && ( eCurrentVideoType < eCP_H261_CIF_equals_H264_HD1080_video_party_type ) &&
			((CapEnum)m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapTransmit,kRolePeople) == eH261CapCode ) &&
			((CapEnum)m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapTransmit,kRolePeople) == eH261CapCode ) )
		{
			BOOL bH261Support = GetSystemCfgFlagInt<BOOL>("H261_SUPPORT_ALWAYS");
			if( !bH261Support )
			{
				eCurrentVideoType = eVideo_party_type_none;
			}
		}

		BYTE isAddvideoAndAudio = !(::GetIsCOPdongleSysMode() || (m_pIpInitialMode->GetConfType() == kVSW_Fixed));
		DWORD artCapacity = 0;
		artCapacity = CalculateArtCapacityAccordingToScm(m_pIpInitialMode, TRUE /*add audio + video for current*/);
		m_artCapacity = artCapacity;

		CreateAndSendReAllocatePartyResources(eIP_network_party_type, eCurrentVideoType, eAllocateAllRequestedResources,FALSE,0,m_pSIPNetSetup->GetEnableSipICE(),artCapacity ,m_TipPartyType, m_RoomId);
	}
}


/////////////////////////////////////////////////////////////////////////////
//SD 5- re-alloc allc
void CSipAddPartyCntl::OnRsrcReAllocatePartyRspReAllocate(CSegment* pParam)
{
	BYTE bAllocationFailed = HandleReallocateForSipResponse(pParam);

	eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();

	if (bAllocationFailed == REALLOCATE_TO_LOWER_VIDEO)
	{
		PTRACE(eLevelInfoNormal,"CSipAddPartyCntl::OnRsrcReAllocatePartyRspReAllocate: reallocating again to lowest video resource found");
		return;
	}

	if (bAllocationFailed == FALSE)
	{
	    if (!m_bIsMrcCall)
	    {
            DWORD videoRate = m_pIpInitialMode->GetMediaBitRate(cmCapVideo,cmCapReceive, kRolePeople); //FSN-613: Dynamic Content for SVC/Mix Conf
            eVideoQuality vidQuality = m_pConf->GetCommConf()->GetVideoQuality();

            PTRACE2INT(eLevelInfoNormal,"CSipAddPartyCntl::OnRsrcReAllocatePartyRspReAllocate: 1 videoRate=", videoRate);
            BYTE bIsAudioOnly = (m_eLastAllocatedVideoPartyType == eVideo_party_type_none) ? 1 : 0;
            H264VideoModeDetails h264VidModeDetails = GetH264ModeAccordingToVideoPartyType(m_eLastAllocatedVideoPartyType);

            BYTE cif4Mpi = (BYTE)-1;

            if (m_eLastAllocatedVideoPartyType >= eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type)
            {
                cif4Mpi = CH263VideoMode::GetH263Cif4VideoCardMPI(videoRate, vidQuality);
            }

            if(m_eLastAllocatedVideoPartyType != eVideo_party_type_none)
            {
            	UpdateH264ModeInLocalCaps(h264VidModeDetails);

               	//Update RTV
               	DWORD audioRate 	= (m_pIpInitialMode->GetMediaBitRate(cmCapAudio,cmCapTransmit)) * 10;
        		DWORD callRate		= videoRate;
        		DWORD realVideoRate	= callRate - audioRate;
        		RTVVideoModeDetails rtvVidModeDetails;
           		GetRtvVideoParams(rtvVidModeDetails, callRate*100, vidQuality, h264VidModeDetails.videoModeType);
           		UpdateRtvModeInLocalCaps(rtvVidModeDetails,kRolePeople,realVideoRate); //VNGFE-8982 - Changed to realVideoRate
            }

            MsSvcVideoModeDetails mssvcmodedetails;
            BYTE isRtv = FALSE;

            if (m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapTransmit) == eRtvCapCode)
            {
                isRtv = TRUE;
            }

            // BRIDGE-10064: HDX/GS use the video rate as total call rate
            if (m_bIsOfferer)
            {
                DWORD audioRate = (m_pIpInitialMode->GetMediaBitRate(cmCapAudio,cmCapTransmit))*10;
                PTRACE2INT(eLevelInfoNormal,"CSipAddPartyCntl::OnRsrcReAllocatePartyRspReAllocate: 1 audioRate=", audioRate);
                videoRate += audioRate;
            }
            PTRACE2INT(eLevelInfoNormal,"CSipAddPartyCntl::OnRsrcReAllocatePartyRspReAllocate: 2 videoRate=", videoRate);

            m_pPartyApi->SetCapsValuesAccordingToNewAllocation(h264VidModeDetails, mssvcmodedetails,
                cif4Mpi, bIsAudioOnly, isRtv, FALSE, videoRate);
	    }
        if (m_bIsOfferer)
        {
            //SD 6- connect video bridge
            m_state = CONNECT_VIDEO_BRIDGE_AFTER_REALLOC;

			BYTE bIsChangeBridge = ChangeVideoBridgeStateAccordingToNewMode();

            if (bIsChangeBridge)
                PTRACE(eLevelInfoNormal,"CSipAddPartyCntl::OnRsrcReAllocatePartyRspReAllocate: connect video bridge. Wait for response.");
            else
                EndOffererConnected();
        }
        else
        {
            PTRACE(eLevelInfoNormal,"CSipAddPartyCntl::OnRsrcReAllocatePartyRspReAllocate: finish dial in resource re-allocation, connect bridges");
            ConnectBridgesSetup();
	    }
	}
}

/////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::OnVideoBrdgConnectedAfterRealloc(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipAddPartyCntl::OnVideoBrdgConnectedAfterRealloc", GetPartyRsrcId());
    if (m_eVidBridgeConnState == eBridgeDisconnected)
	{
		DBGPASSERT(GetPartyRsrcId());
		PTRACEPARTYID(eLevelInfoNormal,"CSipAddPartyCntl::OnVideoBrdgConnectedAfterRealloc : Connect has received after disconnect.", GetPartyRsrcId());
	}

	else
	{
		HandleVideoBridgeConnectedInd(pParam);
		EndOffererConnected();
	}
}
/////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::OnFeccBrdgCon(CSegment* pParam)
{
	m_pPartyApi->SendFeccBridgeConnected();
	CIpPartyCntl::OnFeccBrdgCon(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::EndOffererConnected()
{
	PTRACE(eLevelInfoNormal,"***CSipAddPartyCntl::EndOffererConnected: (Bridge mode is OK)");
	ON(m_isFullBitRateConnect);
	DeleteTimer(CONNECTTOUT);
	m_state = PARTY_IN_CONNECTING_STATE;
	// since we update the target mode when party out return now we are updating the current mode
	*m_pIpCurrentMode = *m_pIpInitialMode;
	if(m_pIpInitialMode->GetConfType() == kVSW_Fixed || m_pIpInitialMode->GetConfType() == kVideoSwitch)
	{
		DWORD videoBridgeBitRate = m_pIpCurrentMode->GetVideoBitRate((cmCapDirection)cmCapTransmit, (ERoleLabel)kRolePeople);
		DWORD remoteBitRate = m_pSipRemoteCaps->GetMaxVideoBitRate(cmCapReceiveAndTransmit, kRolePeople);
		TRACESTR(eLevelInfoNormal) << " CSipAddPartyCntl::EndOffererConnected: remoteBitRate - " << remoteBitRate;
		if( remoteBitRate < videoBridgeBitRate )
			m_pIpCurrentMode->SetFlowControlRateConstraint(remoteBitRate);
	}

	UpdateBridgeFlowControlRateIfNeeded();
	m_pPartyApi->SipConfConnectCall();
	UpdateDetailsForParticipantDisconnectInfoCDREvent(m_pIpCurrentMode);
	if (m_bIsMrcCall)
		UpdateSvcSipPartyConnectedInCdr(m_pIpCurrentMode);
}

/////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::AnswererPartyConnected()
{
	PTRACE(eLevelInfoNormal,"***CSipAddPartyCntl::AnswererPartyConnected");
	// meaning we received ack
	ON(m_isFullBitRateConnect);
	if(m_pIpInitialMode->GetConfType() == kVSW_Fixed || m_pIpInitialMode->GetConfType() == kVideoSwitch)
	{
		DWORD videoBridgeBitRate = m_pIpCurrentMode->GetVideoBitRate((cmCapDirection)cmCapTransmit, (ERoleLabel)kRolePeople);
		DWORD remoteBitRate = m_pSipRemoteCaps->GetMaxVideoBitRate(cmCapReceiveAndTransmit, kRolePeople);
		TRACESTR(eLevelInfoNormal) << " CSipAddPartyCntl::AnswererPartyConnected: remoteBitRate - " << remoteBitRate;
		if( remoteBitRate < videoBridgeBitRate )
			m_pIpCurrentMode->SetFlowControlRateConstraint(remoteBitRate);
	}

	UpdateBridgeFlowControlRateIfNeeded();

	m_pTaskApi->EndAddParty(m_pParty, statOK);
	m_connectingState = IP_CONNECTED;
	if (IsSecondary() == NO)
	{
		m_pTaskApi->UpdateDB(m_pParty,PARTYSTATE,PARTY_CONNECTED);
		RemoveSecondaryCause(FALSE);// check if to keep content secondary cause.
		UpdatePartyStateInCdr();
	}
	else
	{
		PTRACE(eLevelInfoNormal,"CSipAddPartyCntl::AnswererPartyConnected: Secondary party.");
	}
	if(IsValidTimer(CONNECTTOUT))

		DeleteTimer(CONNECTTOUT);



}
/////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::ActiveMedia()
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipAddPartyCntl::ActiveMedia", GetPartyRsrcId());

	m_pPartyApi->ActiveMediaForAvMcuLync();

}

/////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::ChangeScm(CIpComMode* pIpScm,BYTE IsAsSipContentEnable)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipAddPartyCntl::ChangeScm", GetPartyRsrcId());


	EConfType confType = m_pIpInitialMode->GetConfType();
	*m_pIpInitialMode = *pIpScm;
	m_pIpInitialMode->SetConfType(confType);

	m_IsAsSipContentEnable = IsAsSipContentEnable;

}

/////////////////////////////////////////////////////////////////////////////
void  CSipAddPartyCntl::OnAudConnectPartyConnectAudio(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CSipAddPartyCntl::OnAudConnectPartyConnectAudio : Name - ",m_partyConfName, GetPartyRsrcId());

	if (m_eAudBridgeConnState == eBridgeDisconnected)
	{
		DBGPASSERT(GetPartyRsrcId());
		PTRACE2PARTYID(eLevelInfoNormal,"CSipAddPartyCntl::OnAudConnectPartyConnectAudio : Connect has received after disconnect. Name - ",m_partyConfName, GetPartyRsrcId());
	}

	else
	{
		/*VNGR-23140:   Have the same logic as H323 part here.  						*/
		/*If the audio is blocked or muted by operator, we mute or block it again		 	*/
        CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
        EOnOff eOnOff = eOff;
		EMediaDirection eMediaDirection = HandleAudioBridgeConnectedInd(pParam);

        if (CPObject::IsValidPObjectPtr(pConfParty))
        {
         	if (eMediaDirection == eMediaIn || eMediaDirection == eMediaInAndOut)
         	{
        		BOOL bMuteStatus = pConfParty->IsAudioMutedByParty();
                eOnOff = bMuteStatus ? eOn : eOff;
				m_pAudioInterface->UpdateMute(GetPartyRsrcId(), eMediaIn, eOnOff, PARTY);

				/*VNGR-23140: In case of failover or redial, maybe the audio was muted/blocked by the operator in the master*/
				if (pConfParty->IsAudioMutedByOperator())
				{
					PTRACEPARTYID(eLevelInfoNormal,"CSipAddPartyCntl::OnAudConnectParty: Audio muted by Operateor", GetPartyRsrcId());
					eOnOff = eOn;
					m_pAudioInterface->UpdateMute(GetPartyRsrcId(), eMediaIn, eOnOff, OPERATOR);
				}
         	}

			if (eMediaDirection == eMediaOut || eMediaDirection == eMediaInAndOut)
			{
				if (pConfParty->IsAudioBlocked())
				{
					PTRACEPARTYID(eLevelInfoNormal,"CSipAddPartyCntl::OnAudConnectParty: Audio blocked by Operateor", GetPartyRsrcId());
					eOnOff = eOn;
					m_pAudioInterface->UpdateMute(GetPartyRsrcId(), eMediaOut, eOnOff, OPERATOR);
				}
            }
        }

		if (AreTwoDirectionsConnectedToAudioBridge())
		{
			if (m_isFullBitRateConnect)
				m_pTaskApi->EndAddParty(m_pParty,statOK);

			m_pPartyApi->SendAudioBridgeConnected();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::OnVideoBrdgConnected(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipAddPartyCntl::OnVideoBrdgConnected", GetPartyRsrcId());
	if (m_eVidBridgeConnState == eBridgeDisconnected)
	{
		DBGPASSERT(GetPartyRsrcId());
		PTRACEPARTYID(eLevelInfoNormal,"CSipAddPartyCntl::OnVideoBrdgConnected : Connect has received after disconnect", GetPartyRsrcId());
	}
	else
	{
		HandleVideoBridgeConnectedInd(pParam);
		if (AreTwoDirectionsConnectedToVideoBridge()
			|| (IsInDirectionConnectedToVideoBridge() && m_pIpCurrentMode->IsMediaOff(cmCapVideo, cmCapTransmit)))
			m_pPartyApi->SendVideoBridgeConnected();
	}
}


/////////////////////////////////////////////////////////////////////////////
// video brdg disconnection indication
void CSipAddPartyCntl::OnVideoBrdgDisconnectedConnectBridges(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipAddPartyCntl::OnVideoBrdgDisconnectedConnectBridges", GetPartyRsrcId());
	EStat resStat = HandleVideoBridgeDisconnectedInd(pParam);
	if (resStat == statVideoInOutResourceProblem)
	{
		m_pTaskApi->EndAddParty(m_pParty, statVideoInOutResourceProblem);
		DeleteTimer(CONNECTTOUT);
	}
	else
		EndOffererConnected();
}


////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::HandleVideoBridgeUpdate(WORD status, EUpdateBridgeMediaAndDirection eUpdatedBridges)
{
	BYTE bVideoBridgeEndUpdate = CheckVideoBridgeEndUpdate(status, eUpdatedBridges);
	if (bVideoBridgeEndUpdate)
		EndOffererConnected();
}
/////////////////////////////////////////////////////////////////////////////////////////
void  CSipAddPartyCntl::OnConnectToutAllocate(CSegment* pParam)
{
	PTRACEPARTYID(eLevelError,"CSipAddPartyCntl::OnConnectToutAllocate, Timer Expired No Rsp from Rsrc Allocator", GetPartyRsrcId());
	DBGPASSERT(101);
	m_pTaskApi->UpdateDB(m_pParty,DISCAUSE,NO_DISCONNECTION_CAUSE);
	m_pTaskApi->EndAddParty(m_pParty,statTout);
}


/////////////////////////////////////////////////////////////////////////////////////////
void  CSipAddPartyCntl::OnConnectToutCreate(CSegment* pParam)
{
	CLargeString cstr;
	cstr << "Party:" << GetPartyRsrcId() << " Conf:" << GetConfRsrcId();
	cstr << " Did not receive ACK  for opcode: : TB_MSG_OPEN_PORT_REQ(CONF_MPL_CREATE_PARTY_REQ) ";
	cstr << " req:" << m_lastReqId;
	cstr << "( Responsibility: embedded )";

	DumpMcuInternalDetailed(cstr,ACK_NOT_RECEIVED);


	PTRACEPARTYID(eLevelError,"CSipAddPartyCntl::OnConnectToutCreate, Timer Expired - No Ack from MPL on Create Req", GetPartyRsrcId());
	DBGPASSERT(101);

	CMedString str;
	str << "CSipAddPartyCntl::OnMplAckCreate : No Ack for Create party Req , Party Rsrc ID - " << GetPartyRsrcId();
	CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT, ACK_NOT_RECEIVED, MAJOR_ERROR_LEVEL, str.GetString(), TRUE);

	m_isFaulty = 1; // Invoking KillPort process in RA.
	m_isRecovery = 1; // Invoking Recovery process in RA.

	BYTE 	mipHwConn = (BYTE)eMipCardManager;
	BYTE	mipMedia = (BYTE)eMipArt;
	BYTE	mipDirect = eMipNoneDirction;
	BYTE	mipTimerStat = eMipTimer;
	BYTE	mipAction = eMipOpen;

	CSegment* pSeg = new CSegment;
	*pSeg << mipHwConn << mipMedia << mipDirect << mipTimerStat << mipAction;
	DWORD MpiErrorNumber = GetMpiErrorNumber(pSeg);
	m_pTaskApi->PartyDisConnect(MCU_INTERNAL_PROBLEM,m_pParty,NULL,MpiErrorNumber);
	POBJDELETE(pSeg);


}

/////////////////////////////////////////////////////////////////////////////////////////
void  CSipAddPartyCntl::OnConnectToutUpdateVideo(CSegment* pParam)
{
	PTRACEPARTYID(eLevelError,"CSipAddPartyCntl::OnConnectToutUpdateVideo, Timer Expired - No Ack from Video Bridge on update request", GetPartyRsrcId());
	DBGPASSERT(101);
	m_pTaskApi->UpdateDB(m_pParty,DISCAUSE,NO_DISCONNECTION_CAUSE);
	m_pTaskApi->EndAddParty(m_pParty,statTout);
}

/////////////////////////////////////////////////////////////////////////////////////////
void  CSipAddPartyCntl::OnConnectToutConnectBridges(CSegment* pParam)
{
	PTRACEPARTYID(eLevelError,"CSipAddPartyCntl::OnConnectToutConnectBridges, Timer Expired - No Ack from Audio or Video Bridge on connect request, or no IP party connected indication from the party", GetPartyRsrcId());
	DBGPASSERT(101);
	m_pTaskApi->UpdateDB(m_pParty,DISCAUSE,NO_DISCONNECTION_CAUSE);
	m_pTaskApi->EndAddParty(m_pParty,statTout);
}

/////////////////////////////////////////////////////////////////////////////////////////
void  CSipAddPartyCntl::OnConnectToutPartySetup(CSegment* pParam)
{
	PTRACEPARTYID(eLevelError,"CSipAddPartyCntl::OnConnectToutPartySetup, Timer Expired - No Rsp from Party", GetPartyRsrcId());
	DBGPASSERT(101);
	m_pTaskApi->UpdateDB(m_pParty,DISCAUSE,NO_DISCONNECTION_CAUSE);
	m_pTaskApi->EndAddParty(m_pParty,statTout);
}

/////////////////////////////////////////////////////////////////////////////////////////
void  CSipAddPartyCntl::OnConnectToutConnectingState(CSegment* pParam)
{
	PTRACEPARTYID(eLevelError,"CSipAddPartyCntl::OnConnectToutConnectingState, Timer Expired - No Rsp from Party", GetPartyRsrcId());
	DBGPASSERT(101);
	m_pTaskApi->UpdateDB(m_pParty,DISCAUSE,NO_DISCONNECTION_CAUSE);
	m_pTaskApi->EndAddParty(m_pParty,statTout);
}

/////////////////////////////////////////////////////////////////////////////////////////
void  CSipAddPartyCntl::OnConnectToutNeedToRealloc(CSegment* pParam)
{
	PTRACEPARTYID(eLevelError,"CSipAddPartyCntl::OnConnectToutNeedToRealloc, Timer Expired - No Rsp from video bridge", GetPartyRsrcId());
	DBGPASSERT(101);
	m_pTaskApi->UpdateDB(m_pParty,DISCAUSE,NO_DISCONNECTION_CAUSE);
	m_pTaskApi->EndAddParty(m_pParty,statTout);
}

/////////////////////////////////////////////////////////////////////////////////////////
void  CSipAddPartyCntl::OnConnectToutReAllocate(CSegment* pParam)
{
	PTRACEPARTYID(eLevelError,"CSipAddPartyCntl::OnConnectToutReAllocate, Timer Expired - No Rsp from resource allocator", GetPartyRsrcId());
	DBGPASSERT(101);
	m_pTaskApi->UpdateDB(m_pParty,DISCAUSE,NO_DISCONNECTION_CAUSE);
	m_pTaskApi->EndAddParty(m_pParty,statTout);
	m_isFaulty = 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
void  CSipAddPartyCntl::OnConnectToutConnectBridgesAfterRealloc(CSegment* pParam)
{
	PTRACEPARTYID(eLevelError,"CSipAddPartyCntl::OnConnectToutConnectBridgesAfterRealloc, Timer Expired - No Ack from video bridge on connect request", GetPartyRsrcId());
	DBGPASSERT(101);
	m_pTaskApi->UpdateDB(m_pParty,DISCAUSE,NO_DISCONNECTION_CAUSE);
	m_pTaskApi->EndAddParty(m_pParty,statTout);
}

/////////////////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::OnTimerWaitForRsrcAndAskAgainAllocate(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CSipAddPartyCntl::OnTimerWaitForRsrcAndAskAgainAllocate, Name - ",m_partyConfName, GetPartyRsrcId());
	if(m_type == DIALOUT)
	{
		//Temporary ID before we get a response from the resource allocator ?
#ifdef LOOKUP_TABLE_DEBUG_TRACE
		TRACEINTO << "D.K. m_pParty:" << std::hex << m_pParty;
#endif
		m_pParty = (CTaskApp*)(m_monitorPartyId + 100);  // just to get unique id for party list
	}
	StartPartyConnection();
}
/////////////////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::UpdateVideoTxModeForAsymmetricModes()
{
	CComModeH323* pScmWithHD720 = new CComModeH323;
	*pScmWithHD720 = *m_pIpInitialMode;
	pScmWithHD720->SetScmToHdCp(eHD720Res, cmCapReceiveAndTransmit);
	const CBaseCap* pModeHD720At30 = pScmWithHD720->GetMediaAsCapClass(cmCapVideo,cmCapTransmit);

	DWORD details	= 0;
	BYTE values		= 0;
	int  arrIndex	= 0;
	DWORD ValuesToCompare = kCapCode|kBitRate|kFormat|kAnnexes|kH264Profile|kH264Level|kH264Additional_FS|kH264Additional_MBPS| kMaxFR | kH264Mode | kPacketizationMode;

	BYTE IsSupportHD720At30 = m_pSipLocalCaps->IsContainingCapSet(cmCapReceive, *pModeHD720At30, ValuesToCompare, &details, &arrIndex);
	BYTE bCurrentSupportHD1080 = m_pIpCurrentMode->IsHd1080Enabled();
	BYTE bIsCp  = (m_pIpCurrentMode->GetConfType() == kCp && m_pIpCurrentMode->GetConfMediaType()!=eMixAvcSvcVsw);

	if (bIsCp && (m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapTransmit) == eH264CapCode) && IsSupportHD720At30 && bCurrentSupportHD1080)
		m_pIpCurrentMode->SetScmToHdCp(eHD1080Res, cmCapTransmit);

    if (bIsCp && m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapTransmit) == eH263CapCode)
    {
        m_pIpInitialMode->Dump ("UpdateVideoTxModeForAsymmetricModes - initial", eLevelInfoNormal);
        m_pIpCurrentMode->Dump ("UpdateVideoTxModeForAsymmetricModes - current", eLevelInfoNormal);
        int tx4CifMpi = m_pSipLocalCaps->Get4CifMpi();
        m_pIpCurrentMode->SetFormatMpi (k4Cif, tx4CifMpi, cmCapTransmit, kRolePeople);
    }
	POBJDELETE(pModeHD720At30);
	POBJDELETE(pScmWithHD720);
    POBJDELETE(pModeHD720At30);
}
////////////////////////////////////////////////////////////////////////////////////////////
BYTE CSipAddPartyCntl::HandleReallocateForSipResponse(CSegment* pParam)
{
	CPartyRsrcDesc tempPartyAllocatedRsrc;

	CPartyRsrcDesc* pTempPartyAllocatedRsrc = &tempPartyAllocatedRsrc;
	pTempPartyAllocatedRsrc->DeSerialize(SERIALEMBD, *pParam);
	DWORD status = pTempPartyAllocatedRsrc->GetStatus();
	pTempPartyAllocatedRsrc->DumpToTrace();
	BYTE bAllocationFailed = FALSE;

	eNetworkPartyType networkPartyType =  pTempPartyAllocatedRsrc->GetNetworkPartyType();

	if ((status != STATUS_OK)||(networkPartyType!=eIP_network_party_type))
	{
		if(networkPartyType!=eIP_network_party_type)
		{
			PTRACE2PARTYID(eLevelInfoNormal, "CSipAddPartyCntl::HandleReallocateForSipResponse eNetworkPartyType!= eIP_network_party_type, eNetworkPartyType = ",eNetworkPartyTypeNames[networkPartyType], GetPartyRsrcId());
			PASSERT(1);
		}
		PTRACE2PARTYID(eLevelInfoNormal, "CSipAddPartyCntl::HandleReallocateForSipResponse : REALLOCATION FAILED!!! do not continue process : ",CProcessBase::GetProcess()->GetStatusAsString(status).c_str(), GetPartyRsrcId());
		bAllocationFailed = TRUE;
	}

	else
	{
		eVideoPartyType requestedVideoPartyType = m_eLastReAllocRequestVideoPartyType;
		eVideoPartyType allocatedVideoPartyType = pTempPartyAllocatedRsrc->GetVideoPartyType();


        TRACEINTO << "requestedVideoPartyType=" << requestedVideoPartyType
            << " allocatedVideoPartyType=" << allocatedVideoPartyType;

        UpdateResourceTableAfterRealloc(pTempPartyAllocatedRsrc);

		if(requestedVideoPartyType >= allocatedVideoPartyType)
		{
            CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();
            if(allocatedVideoPartyType == eVideo_party_type_none)
			{// if its undefined call make it audio only one (remove all none audio media from SCM, set audio only caps, set m_voice member, set ConfParty DB).
				SetPartyToAudioOnly();
			}
            else if (pCommConf && pCommConf->GetConfMediaType() == eMixAvcSvc)
            {
                TRACEINTO << "Handle mix resources";
                std::list <StreamDesc> videoStreams = m_pIpCurrentMode->GetStreamsListForMediaMode(cmCapVideo, cmCapReceive, kRolePeople);
                int i = videoStreams.size();
                if (i == 0)
                {
                    m_pPartyAllocatedRsrc->DeleteRsrcDescAccordingToLogicalResourceType(eLogical_relay_avc_to_svc_rtp);
                    if (eProductFamilyRMX != CProcessBase::GetProcess()->GetProductFamily())
                        m_pPartyAllocatedRsrc->DeleteRsrcDescAccordingToLogicalResourceType(eLogical_relay_avc_to_svc_rtp_with_audio_encoder);
                }
                else if (i == 1)
                {
                    m_pPartyAllocatedRsrc->DeleteRsrcDescAccordingToLogicalResourceType(eLogical_relay_avc_to_svc_rtp);
                }
            }
			m_eLastAllocatedVideoPartyType = allocatedVideoPartyType;
			if (!m_bIsMrcCall)
				UpdateScmWithResources(m_SsrcIdsForAvcParty.m_SsrcIds, allocatedVideoPartyType, pTempPartyAllocatedRsrc->GetIsAvcVswInMixedMode());
		}
		else if (requestedVideoPartyType < allocatedVideoPartyType)
		{
			const char* MOC_PRODUCT_NAME = "Microsoft Office Communicator";
			char * bIsMsMoc = strstr(m_productId, MOC_PRODUCT_NAME);
			BOOL bEnableFreeVideoResources = GetSystemCfgFlagInt<BOOL>(CFG_KEY_SIP_FREE_VIDEO_RESOURCES);
			if(bEnableFreeVideoResources && IsUndefinedParty() && requestedVideoPartyType == eVideo_party_type_none && allocatedVideoPartyType != eVideo_party_type_none && allocatedVideoPartyType == GetLowestVideoAllocationAccordingToSystemMode (bIsMsMoc))
			{
				PTRACE(eLevelInfoNormal, "CSipAddPartyCntl::HandleReallocateForSipResponse  : no audio ports saving video resource ");
				bAllocationFailed = FALSE;
			}
			else if(bEnableFreeVideoResources && IsUndefinedParty() && requestedVideoPartyType == eVideo_party_type_none && allocatedVideoPartyType != eVideo_party_type_none && allocatedVideoPartyType > GetLowestVideoAllocationAccordingToSystemMode(bIsMsMoc) )
			{
				PTRACE(eLevelInfoNormal, "CSipAddPartyCntl::HandleReallocateForSipResponse  : no audio ports saving video resource -reallocating to lowest video resource possible ");
				bAllocationFailed = FALSE;

				DWORD artCapacity = 0;
				artCapacity = CalculateArtCapacityAccordingToScm(m_pIpInitialMode, TRUE /*add audio + video for current*/);
				m_artCapacity = artCapacity;

				CreateAndSendReAllocatePartyResources(eIP_network_party_type, GetLowestVideoAllocationAccordingToSystemMode(bIsMsMoc), eAllocateAllRequestedResources,FALSE,0,m_pSIPNetSetup->GetEnableSipICE(),artCapacity, m_TipPartyType);
				return REALLOCATE_TO_LOWER_VIDEO;
			}
			else
			{
				PTRACE2PARTYID(eLevelInfoNormal, "CSipAddPartyCntl::HandleReallocateForSipResponse : Higher allocation than requested !!! do not continue process : ",CProcessBase::GetProcess()->GetStatusAsString(status).c_str(), GetPartyRsrcId());
				bAllocationFailed = TRUE;
			}
		}
	}

	if (bAllocationFailed)
	{
		m_pTaskApi->UpdateDB(m_pParty,DISCAUSE,RESOURCES_DEFICIENCY,1); // Disconnnect cause
		m_pTaskApi->EndAddParty(m_pParty,statIllegal);
		//m_isFaulty = 1;
	}
	else
		PTRACEPARTYID(eLevelInfoNormal, "CSipAddPartyCntl::HandleReallocateForSipResponse : REAllocation is OK ", GetPartyRsrcId());

	return bAllocationFailed;
}
////////////////////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::OnVideoBrdgDisconnectedUpdateLegacyStatus(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal, "CSipAddPartyCntl::OnVideoBrdgDisconnectedUpdateLegacyStatus", GetPartyRsrcId());
	EStat resStat = HandleVideoBridgeDisconnectedInd(pParam);
	if (resStat == statVideoInOutResourceProblem)
	{
		m_pTaskApi->EndAddParty(m_pParty, statVideoInOutResourceProblem);
		DeleteTimer(CONNECTTOUT);
	}
	else
	{
		if (m_bIsOfferer)
		{
			// Reconnect video bridge
			BYTE bIsChangeBridge = ChangeVideoBridgeStateAccordingToNewMode();
			if (bIsChangeBridge)
				PTRACE(eLevelInfoNormal,"CSipAddPartyCntl::OnVideoBrdgDisconnectedUpdateLegacyStatus: connect video bridge. Wait for response.");
			else
				DBGPASSERT(101); // Video bridge need to be reconnected
		}
		else
			DBGPASSERT(101); // This flow is only for offerer.
	}
}
////////////////////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::OnConnectToutUpdateLegacyStatus(CSegment* pParam)
{
	PTRACEPARTYID(eLevelError, "CSipAddPartyCntl::OnConnectToutUpdateLegacyStatus, Timer Expired - No Rsp from video bridge", GetPartyRsrcId());
	DBGPASSERT(101);
	m_pTaskApi->UpdateDB(m_pParty,DISCAUSE,NO_DISCONNECTION_CAUSE);
	m_pTaskApi->EndAddParty(m_pParty,statTout);
}
////////////////////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::OnVideoBrdgConnectedUpdateLegacyStatus(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipAddPartyCntl::OnVideoBrdgConnectedUpdateLegacyStatus", GetPartyRsrcId());
	if (m_eVidBridgeConnState == eBridgeDisconnected)
	{
		DBGPASSERT(GetPartyRsrcId());
		PTRACEPARTYID(eLevelInfoNormal,"CSipAddPartyCntl::OnVideoBrdgConnectedUpdateLegacyStatus : Connect has received after disconnect.", GetPartyRsrcId());
	}
	else
	{
		HandleVideoBridgeConnectedInd(pParam);
		EndOffererConnected();
	}
}


///////////////////////////////////////////////////////////////////////////////////
DWORD CSipAddPartyCntl::UpdateScmWithResources(SVC_PARTY_IND_PARAMS_S  &aSvcParams,eVideoPartyType allocatedVideoPartyType, BOOL isAvcVswInMixedMode)
{
	m_pIpInitialMode->SetPartyId(GetPartyRsrcId());
	CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();
	DWORD status=STATUS_OK;

	if (m_bIsMrcCall)
	{// Set SSRCs:
		m_pIpInitialMode->SetSsrcIds(cmCapAudio,cmCapReceive,kRolePeople, (unsigned int*)(&(aSvcParams.m_ssrcAudio)), 1, false);
		STREAM_GROUP_S* pSendStreamGroup = m_pIpInitialMode->GetStreamsGroup(cmCapVideo,cmCapReceive,kRolePeople);
		if (pSendStreamGroup)
		{
			TRACEINTO << "mix_mode: number of streams=" <<  (int)pSendStreamGroup->numberOfStreams;
			m_pIpInitialMode->SetSsrcIds(cmCapVideo,cmCapReceive,kRolePeople, (unsigned int*)(&(aSvcParams.m_ssrcVideo)), pSendStreamGroup->numberOfStreams, true);
			m_pSipLocalCaps->SetSsrcIds(cmCapVideo,cmCapReceive,kRolePeople, (unsigned int*)(&(aSvcParams.m_ssrcVideo)), pSendStreamGroup->numberOfStreams, true);
		}
		m_pSipLocalCaps->SetSsrcIds(cmCapAudio,cmCapReceive,kRolePeople, (unsigned int*)(&(aSvcParams.m_ssrcAudio)), 1);

		// Update streams list:
		m_pIpInitialMode->UpdateStreamsAndPayloadType(m_pSipLocalCaps);

		m_pIpInitialMode->Dump("CSipAddPartyCntl::UpdateScmWithResources initial",eLevelInfoNormal);
	}
    else
    {// call parent function
		status=CIpPartyCntl::UpdateScmWithResources(aSvcParams,allocatedVideoPartyType, isAvcVswInMixedMode);
    }
    return status;
}

////////////////////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::SetupSIPConParameters(PartyControlInitParameters& partyControInitParam, PartyControlDataParameters &partyControlDataParams)
{
#ifdef PERFORMANCE_TEST
	m_Stopper.AddTime();
#endif //PERFORMANCE_TEST

	const char*avServiceNameStr = partyControInitParam.pAvServiceNameStr;
	WORD welcomeMsgTime = partyControInitParam.welcomeMsgTime;
	DWORD connectDelay = partyControInitParam.connectDelay;

	CConfParty* pConfParty = partyControInitParam.pConfParty;
	if (!pConfParty)
	{
		PTRACE(eLevelInfoNormal, "CSipAddPartyCntl::SetupSIPConParameters - pConfParty is NULL");
		DBGPASSERT(1124);
		return;
	}

	SetupConnParameters(partyControInitParam, partyControlDataParams);

	const char* strPartyName = pConfParty->GetName();
	BYTE interfaceType = pConfParty->GetNetInterfaceType();
	BYTE bIsH323 = (interfaceType == H323_INTERFACE_TYPE);
	char* strConfGUID = GetUniqueGuid(partyControInitParam.pStrConfGuid, pConfParty);

	TRACEINTO << "PartyName:" << DUMPSTR(strPartyName) << ", ConfGUID:" << strConfGUID;

	CSipNetSetup* pIpNetSetup = NewAndSetupSipNetSetup(pConfParty, partyControInitParam, partyControlDataParams);

	CIpComMode* pPartyScm = NewAndGetPartyCntlScm(partyControInitParam, partyControlDataParams);

	SetMaxBitRateInNetSetup(pIpNetSetup, pConfParty, pPartyScm, partyControInitParam, partyControlDataParams);

	pIpNetSetup->SetSIPConfIdAsGUID(strConfGUID);

	DWORD vidBitrate = 0;
	CSipCaps* pSIPCaps = NewAndGetLocalSipCaps(pPartyScm, pConfParty, pIpNetSetup, vidBitrate, partyControInitParam, partyControlDataParams);

	//N.A. DEBUG VP8
	pPartyScm->Dump("N.A. DEBUG pPartyScm->Dump CSipAddPartyCntl::SetupSIPConParameters");
	//N.A. DEBUG VP8
	CSuperLargeString strCaps1;
	pSIPCaps->DumpToString(strCaps1);
	PTRACE2(eLevelInfoNormal, "N.A. DEBUG pSIPCaps->Dump CSipAddPartyCntl::SetupSIPConParameters", strCaps1.GetString());


	if (pConfParty->GetMsftAvmcuState() == eMsftAvmcu2013 || pConfParty->GetMsftAvmcuState() == eMsftAvmcuUnkown)
	{
		PTRACE2(eLevelError, "CIpPartyCntl::SetupSIPConParameters : MsftAvmcuState= ", enMsftAvmcuStateNames[pConfParty->GetMsftAvmcuState()]);
		pPartyScm->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);
		pPartyScm->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);
		pPartyScm->SetMediaOff(cmCapData, cmCapReceiveAndTransmit, kRolePeople);
		
		//
		pSIPCaps->RemoveCapSet(eOpus_CapCode);
		pSIPCaps->RemoveCapSet(eOpusStereo_CapCode);
		
	}

	UpdateBfcpTransportTypeIfNeeded(pPartyScm, partyControlDataParams.pSipRmtCaps, pSIPCaps);

	CConfIpParameters* pServiceParams = GetConfIpParameters(pConfParty);
	if (pServiceParams == NULL)
	{
		PASSERTMSG(1, "CSipAddPartyCntl::SetupSIPConParameters - IP ServiceList is empty, can't configure Default service!!!");
		return;
	}

	CQoS *pQos = new CQoS(pServiceParams->GetQOS());
	AllocMemory();
	m_pQos = new CQoS;
	m_serviceId = pConfParty->GetServiceId();
	m_monitorConfId = partyControInitParam.monitorConfId;
	m_monitorPartyId = pConfParty->GetPartyId();
	*m_pSipLocalCaps = *pSIPCaps;
	m_voice = pConfParty->GetVoice();
	*m_pQos = *pQos;
	*m_pSIPNetSetup = *pIpNetSetup;
	*m_pIpInitialMode = *pPartyScm;

TRACEINTO << "IP VERSION: " << (DWORD)pIpNetSetup->GetIpVersion();
	///N.A. DEBUG VP8
	CSuperLargeString strCaps2;
	m_pSipLocalCaps->DumpToString(strCaps2);
	PTRACE2(eLevelInfoNormal, "N.A. DEBUG CSipAddPartyCntl::SetupSIPConParameters m_pSipLocalCaps->DumpToString", strCaps2.GetString());
	PTRACE2(eLevelInfoNormal, "CSipAddPartyCntl::SetupSIPConParameters MSConversation id:",m_pSIPNetSetup->GetMsConversationId());

	m_bIsOfferer = partyControlDataParams.bIsOffer;
	m_subServiceId = m_pSIPNetSetup->GetSubServiceId();
	m_bIsLync = partyControlDataParams.bIsLync;
	m_IsAsSipContentEnable = partyControlDataParams.bIsASSIPcontentEnable;
	// VNGR-6679 - Solution
	if (partyControlDataParams.pSipRmtCaps != NULL)
		*m_pSipRemoteCaps = *partyControlDataParams.pSipRmtCaps;

	SetupTIPMode(partyControInitParam, partyControlDataParams);
	SetupFeccMode(strPartyName);

	m_originalConfRate = m_pIpInitialMode->GetCallRate();
	m_eConfTypeForAdvancedVideoFeatures = kNotAnAdvancedVideoConference;

	if (m_pIpInitialMode->GetConfType() != kVideoSwitch && kVSW_Fixed != m_pIpInitialMode->GetConfType() && (m_pIpInitialMode->GetConfMediaType() != eMixAvcSvcVsw || m_bIsMrcCall))
	{
		SetNewVideoRates(vidBitrate);
		BOOL bDisableAvMcuLowRate = GetSystemCfgFlag<BOOL>("DISABLE_LYNC_AV_MCU_128_192_KBPS");
		if (!bDisableAvMcuLowRate && (pPartyScm->GetCallRate() == 128 || pPartyScm->GetCallRate() == 192) && (pConfParty->GetAvMcuLinkType() != eAvMcuLinkNone || pConfParty->GetMsftAvmcuState() == eMsftAvmcu2013 || pConfParty->GetMsftAvmcuState() == eMsftAvmcuUnkown))
		{
			TRACEINTO << "Patch for avmcu changing rate of 128k to 192k for in only as in 128k av-mcu dont send video-put out as 64k back ";
			if (pPartyScm->GetCallRate() == 128)
				m_pIpInitialMode->SetVideoBitRate(640, cmCapTransmit);
			else
				m_pIpInitialMode->SetVideoBitRate(1280, cmCapTransmit);
		}
	}
	m_pIpInitialMode->SetTotalVideoRate(vidBitrate);
	const char* strConfName = m_pConf->GetName();
	m_name = new char[H243_NAME_LEN];
	memset(m_name, '\0', H243_NAME_LEN);
	strncpy(m_name, strPartyName, H243_NAME_LEN - 1);
	SetFullName(strPartyName, strConfName);
	m_IsGateWay = m_pConf->GetIsGateWay();
	SetConfParamInfo();
	POBJDELETE(m_pTaskApi);
	m_pTaskApi = new CConfApi;
	m_pTaskApi->CreateOnlyApi(m_pConf->GetRcvMbx(), this);
	m_pTaskApi->SetLocalMbx(m_pConf->GetLocalQueue());

	POBJDELETE(pSIPCaps);
	POBJDELETE(pPartyScm);
	POBJDELETE(pIpNetSetup);
	POBJDELETE(pQos);
	POBJDELETE(partyControlDataParams.pSipRmtCaps);

	SetIsTipCall(pConfParty->GetIsTipCall());
	if (pConfParty->GetIsTipCall()) //NOA TEMP TIP
		m_TipPartyType = eTipMasterCenter;

	TRACEINTO << m_partyConfName << ", VideoBitRate:" << vidBitrate;

	if (partyControInitParam.bIsRedial)
	{
		ResetRedialParameter();
	}

	m_isMsftEnv = GetIsMsftEnv();
	m_remoteIdentity = partyControlDataParams.epType;

	pConfParty->SetAvMcuLinkType(m_AvMcuLinkType);
}
////////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::ConnectSIP(PartyControlInitParameters& partyControInitParam)
{
	BYTE StandByStart = partyControInitParam.standByStart;

	CConfParty* pConfParty = partyControInitParam.pConfParty;
	//if party is referred, don't use auto-dial out
	if (strstr(m_name, "_(referred)") || strcmp(pConfParty->GetRefferedToUri(), ""))
		StandByStart = YES;

	TRACEINTO << m_partyConfName << ", StandByStart1:" << (int)partyControInitParam.standByStart << ", StandByStart2:" << (int)StandByStart;

	switch (m_type)
	{
		case DIALIN:
		{
			TRACEINTO << m_partyConfName << ", CallType:DIAL-IN";
			if (pConfParty)
			{
				if (m_pSIPNetSetup->IsItPrimaryNetwork())
				{
					pConfParty->SetSubServiceMode(ePrimaryIpSubService);
				}
				else
				{
					pConfParty->SetSubServiceMode(eSecondaryIpSubService);
					pConfParty->SetSubServiceName(SUB_SERVICE_NAME_SECONDARY);
				}
			}

			m_pPartyApi = new CPartyApi;
			m_pPartyApi->CreateOnlyApi(*partyControInitParam.pPartyRcvMbx);
			BYTE bIsValid = CheckNameValidityAndDisconnectIfNeeded();

			POBJDELETE(partyControInitParam.pPartyRcvMbx);

			if (bIsValid)
			{
				PTRACE2PARTYID(eLevelInfoNormal, "CSipAddPartyCntl::ConnectSIP-changing local caps-only cp: Name - ", m_partyConfName, GetPartyRsrcId());
				StartPartyConnection();
			}
			break;
		}

		case DIALOUT:
		{

			m_bIsFirstConnectionAfterHotBackupRestore = pConfParty->IsFirstConnectionAfterHotBackupRestore();
			if (!StandByStart)
			{
				TRACEINTO << m_partyConfName << ", CallType:DIAL-OUT - Dial out now";
				DialOut(0); // no re-dial at create of a call
				m_pTaskApi->UpdateDB(m_pParty, NUMRETRY, (DWORD)m_redial, 1);
			}
			else
			{  // connect and stand by
				TRACEINTO << m_partyConfName << ", CallType:DIAL-OUT - Stand by";
				m_pParty = (CTaskApp*)(m_monitorPartyId + 100);  // just to get unique id for party list
				m_disconnectState = DISCONNECTED;
				m_pTaskApi->EndAddParty(m_pParty, PARTY_STAND_BY);
			}
			break;
		}

		default:
		{
			PTRACEPARTYID(eLevelError, "CSipAddPartyCntl::ConnectSIP : UNKNOWN DIAL TYPE", GetPartyRsrcId());
			break;
		}
	}
}
///////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::SetupTIPMode(PartyControlInitParameters&	 partyControInitParam,PartyControlDataParameters &partyControlDataParams)
{
	CConfParty* pConfParty 		= partyControInitParam.pConfParty;
	const char* strPartyName 		= pConfParty->GetName();

	m_eTelePresenceMode          	= (eTelePresencePartyType)pConfParty->GetTelePresenceMode();
	if (!IsValidPObjectPtr(m_telepresenseEPInfo))
		m_telepresenseEPInfo = new CTelepresenseEPInfo();
		m_telepresenseEPInfo->SetEPtype(m_eTelePresenceMode);
		m_telepresenseEPInfo->SetLinkNum(0);
		m_telepresenseEPInfo->SetLinkRole(0);

	// sip parties with TIP EP types are not multi-cascade links, that is implemented in h323 only.
	// resolve the party's link index (default is 0)
	if (partyControlDataParams.linkType != eRegularParty || m_eTelePresenceMode != eTelePresencePartyNone)
	{
		BYTE name_len = (strPartyName)? strlen(strPartyName) : 0;
		DWORD link_num = (DWORD)-1;
		if ((name_len > 2) && (sscanf(&strPartyName[name_len-1], "%u", &link_num) == 1))
		{
			if (link_num > MAX_CASCADED_LINKS_NUMBER)
			{
				PTRACE(eLevelInfoHigh, "ITP_CASCADE: CSipAddPartyCntl::Create name format matches ITP party, but link number is invalid");
				m_telepresenseEPInfo->SetEPtype(eTelePresencePartyNone);
			}
			else
			{
				TRACEINTO << "ITP_CASCADE: CSipAddPartyCntl::Create found ITP party with room id " << partyControlDataParams.roomID<< ", and link number " << link_num;
				m_telepresenseEPInfo->SetLinkNum(link_num-1);
				m_telepresenseEPInfo->SetNumOfLinks((DWORD)-1);
			}
		}
		else
		{
			PTRACE(eLevelInfoHigh, "ITP_CASCADE: CSipAddPartyCntl::Create name format doesn't match ITP party format, treated as regular EP");
			m_telepresenseEPInfo->SetEPtype(eTelePresencePartyNone);
			m_telepresenseEPInfo->SetLinkNum(0);
			partyControlDataParams.linkType   = eRegularParty;
			pConfParty->SetPartyType(eRegularParty);
		}
	}


}
///////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::SetupFeccMode(const char* strPartyName)
{
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	BOOL bSipEnableFecc = TRUE;
	pSysConfig->GetBOOLDataByKey("SIP_ENABLE_FECC", bSipEnableFecc);
	if(!bSipEnableFecc ||  m_voice)
	{
		PTRACEPARTYID(eLevelError,"CSipAddPartyCntl::SetupFeccMode: SIP_ENABLE_FECC is false", GetPartyRsrcId());
		m_pIpInitialMode->SetMediaOff(cmCapData, cmCapReceiveAndTransmit);
	}

	if(strstr(strPartyName, "##FORCE_MEDIA_WITHOUT_FECC")!=NULL)
	{
		PTRACE2PARTYID(eLevelError,"CSipAddPartyCntl::SetupSIPConParameters: FORCE_MEDIA_WITHOUT_FECC - ", strPartyName, GetPartyRsrcId());
		m_pIpInitialMode->SetMediaOff(cmCapData, cmCapReceiveAndTransmit);
	}

}
////////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::ResetRedialParameter()
{
	PTRACE2PARTYID(eLevelInfoNormal, "CSipAddPartyCntl::ResetRedialParameter: Name - ", m_partyConfName, GetPartyRsrcId());
	//m_connectDelay = 0;
	OFF(m_disconnectMode);
	OFF(m_disconnectState);
	OFF(m_isDisconnect);
	OFF(m_isFullBitRateConnect);
	OFF(m_isCDRPartyConnected);

	//m_connectDelay		= partyControInitParam.connectDelay;
	m_connectDelayCounter	= 0;
	m_disconnectDelay = 0;
	m_isWaitingForRsrcAndAskAgain = YES;

	m_isRemoteCapReady = NO;
	m_isRemoteDBC2 = NO;
	m_avConnectionMode = STATUS_PARTY_NONE;
	m_isFaulty        = FALSE;
	m_isRecovery      = FALSE;
	m_bIsOfferer		= TRUE;
	m_pTargetModeMaxAllocation = NULL;//For DPA
	m_MaxLocalCaps = NULL;
	m_conferenceContentRate = 0;

	m_eLastCopChangeModeParam = COP_decoder_resolution_Last;
	m_eLastCopChangeModeType = eCop_DecoderParams;
	m_lastCopForceEncoderLevel = INVALID_COP_LEVEL;
	m_eFirstRxVideoCapCode = eUnknownAlgorithemCapCode;
	m_eFirstRxVideoProfile = H264_Profile_BaseLine;

	m_isSentH239Out = FALSE;
	m_lastConnectionRate = 0;
	OFF(m_presentationStreamOutIsUpdated); /* In case of reconnect we want to set this to 0, to allow contentBridge reconnection properly */
	m_NumOfChangeModesInSec = 0;

	m_bIsMrcCall				= FALSE;
	m_bIsLync				= FALSE;


}

///////////////////////////////////////////////////////////////////////////
BOOL CSipAddPartyCntl::GetIsNeedToChangeTipResAccordingToConfVidQuality(CConfParty* pConfParty)
{//BRIDGE-5753
	if(!pConfParty)
	{
		PASSERTMSG(!pConfParty, "!pConfParty");
		return FALSE;
	}

	BOOL bIsFlagOn = YES;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("RESOURCE_PRIORITY_IN_REQUIRE", bIsFlagOn);


	BOOL bIsTipCall		= pConfParty->GetIsTipCall();
	BOOL bIsTipHeader  	= pConfParty->GetIsTipHeader();

	if(!bIsFlagOn)
	{
		PTRACE(eLevelInfoNormal,"CConf::GetIsNeedToChangeTipResAccordingToConfVidQuality - TIP_RESOLUTION_ACCORDING_TO_CONF_VID_QUALITY is NO ");
		return FALSE;
	}

	if(!bIsTipCall || !bIsTipHeader)
	{
		PTRACE(eLevelInfoNormal,"CConf::GetIsNeedToChangeTipResAccordingToConfVidQuality - not TIP! ");
		return FALSE;
	}

	return TRUE;;
}

/////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::SetupSipEntryPoint()
{
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	BOOL bIsSipPartyLevelSimulation = 0;
	std::string key = "SIP_PARTY_LEVEL_SIMULATION";
	pSysConfig->GetBOOLDataByKey(key, bIsSipPartyLevelSimulation);
	if(bIsSipPartyLevelSimulation)
		m_entryPoint =  SipPartyOutEntryPointSimulation;// for party control level simulation
	else
		m_entryPoint =  SipPartyOutCreateEntryPoint;

}
/////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::UpdateBfcpTransportTypeIfNeeded(CIpComMode* pPartyScm, CSipCaps* pRmtCaps, CSipCaps* pLocalCaps)
{
	if (!pRmtCaps)
	{
		PTRACE(eLevelInfoNormal, "CSipAddPartyCntl::UpdateBfcpTransportTypeIfNeeded - pRmtCaps is NULL");
		return;
	}

	if (pRmtCaps->GetNumOfCapSets() == 0)
	{
		PTRACE(eLevelInfoNormal, "CSipAddPartyCntl::UpdateBfcpTransportTypeIfNeeded - no media in remote SDP (empty INVITE)");
		return;
	}

	enTransportType remoteBfcpTransType = pRmtCaps->GetBfcpTransportType();
	enTransportType localBfcpTransType 	= pPartyScm->GetBfcpTransportType();

	if (remoteBfcpTransType == eUnknownTransportType)
	{
		PTRACE(eLevelInfoNormal, "CSipAddPartyCntl::UpdateBfcpTransportTypeIfNeeded - unknow remote BFCP transport type");
		return;
	}

	if (remoteBfcpTransType != localBfcpTransType)
	{
		pPartyScm->SetBfcpTransportType(remoteBfcpTransType);
		pLocalCaps->SetBfcpTransportType(remoteBfcpTransType);
	}

}

///////////////////////////////////////////////////////////////////////////////////
DWORD CSipAddPartyCntl::UpdateCapsWithMsftResources(MS_SSRC_PARTY_IND_PARAMS_S  &msSsrcParams)
{
	if (!m_isMsftEnv || isMsftSvc2013Supported() == FALSE )
	{
		PTRACE(eLevelInfoNormal,"CSipAddPartyCntl::UpdateScmWithMsftResources Shmulik DBG m_isMsftEnv is FALSE: ");
		return STATUS_OK;
	}

	DWORD ssrc = msSsrcParams.m_msSsrcFirst;
	PTRACE2INT(eLevelInfoNormal,"CSipAddPartyCntl::UpdateScmWithMsftResources ssrc audio=: ", ssrc);


	m_pSipLocalCaps->setMsftSsrcAudio(ssrc);

	ssrc++;
	for (int i=1; i<=MaxMsftSvcSdpVideoMlines; i++)
	{
		if (ssrc+(MSFT_SVC_MAX_VIDEO_SSRC_IN_SDP_LINE-1) >= msSsrcParams.m_msSsrcLast)
		{
			PTRACE2INT(eLevelInfoNormal,"CSipAddPartyCntl::UpdateScmWithMsftResources ssrc exceeds allocated ssrc: ", ssrc);
			DBGPASSERT(-1);
			return STATUS_FAIL;
		}

		m_pSipLocalCaps->setMsftSsrcVideo(ssrc, ssrc+MSFT_SVC_MAX_VIDEO_SSRC_IN_SDP_LINE-1, i);
		ssrc += MSFT_SVC_MAX_VIDEO_SSRC_IN_SDP_LINE;
	}

	return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////

void CSipAddPartyCntl::CheckResourceAllocationForTipAndUpdateRate(eVideoPartyType allocatedVideoPartyType)
{
	if( GetIsTipCall() )
	{
		if( m_eLastAllocRequestVideoPartyType != allocatedVideoPartyType )
		{
			if( allocatedVideoPartyType == eCP_H264_upto_HD720_30FS_Symmetric_video_party_type )
			{
				int nVidBitRateFor720HD = GetMaxVidBitrateForHD720() ;
				int videoBitRate 		= m_pIpInitialMode->GetVideoBitRate(cmCapReceive, kRolePeople);

				CMedString str = "";
				str << "CSipAddPartyCntl::CheckResourceAllocationForTipAndUpdateRate - (Eh264VideoModeType)nVidBitRateFor720HD " << (int)nVidBitRateFor720HD << " videoBitRate " << videoBitRate;
				PTRACE(eLevelInfoNormal,str.GetString());

				if( videoBitRate > nVidBitRateFor720HD * 10 )
				{
					m_pIpInitialMode->SetVideoBitRate(nVidBitRateFor720HD * 10, cmCapReceiveAndTransmit);
					m_pIpInitialMode->SetTotalVideoRate(nVidBitRateFor720HD * 10);
					m_pIpInitialMode->SetCallRate(nVidBitRateFor720HD);

					CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(m_monitorPartyId);
					if(pConfParty)
						pConfParty->SetVideoRate(nVidBitRateFor720HD);

					m_pSipLocalCaps->SetVideoRateInallCaps(nVidBitRateFor720HD);
					m_MaxLocalCaps->SetVideoRateInallCaps(nVidBitRateFor720HD);

					if(m_pSIPNetSetup)
					{
						m_pSIPNetSetup->SetMaxRate(nVidBitRateFor720HD * 1000);
					}
				}

			}
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::SetMsConfInviteReq(sipSdpAndHeadersSt* pSdpAndHeaders,CSipNetSetup * pNetSetup)   // we can save here only the dynamic section
{

	PTRACE(eLevelInfoNormal,"CSipAddPartyCntl::SetMsConfInviteReq  ");

	if(pSdpAndHeaders)
	{
	DWORD len = sizeof(sipSdpAndHeadersBaseSt) + pSdpAndHeaders->lenOfDynamicSection;
	m_MsConfReq = (sipSdpAndHeadersSt *)new BYTE[len];

	COstrStream ostr;
	::DumpXMLToStream(ostr,pSdpAndHeaders);
	PTRACE2(eLevelInfoNormal,"CSipAddPartyCntl::SetMsConfInviteReq - dump XML",ostr.str().c_str()); //debug


	memset(m_MsConfReq,'\0', len);
	memcpy(m_MsConfReq,pSdpAndHeaders,len);

	COstrStream ostr1;
	::DumpXMLToStream(ostr1,m_MsConfReq);
	PTRACE2(eLevelInfoNormal,"CSipAddPartyCntl::SetMsConfInviteReq - dump XML2",ostr1.str().c_str()); //debug

	CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(m_monitorPartyId);

	if(pConfParty)
	{
		m_pSIPNetSetup->SetCallIndex(pNetSetup->GetCallIndex() );
		m_pSIPNetSetup->SetSrcPartyAddress(pNetSetup->GetToPartyAddress());
		m_pSIPNetSetup->SetLocalSipAddress(pNetSetup->GetToPartyAddress());
		if(pNetSetup->GetOriginalToDmaSipAddress() && (pNetSetup->GetOriginalToDmaSipAddress())[0] )
		{
			PTRACE2(eLevelInfoNormal,"CSipAddPartyCntl::SetMsConfInviteReq - pNetSetup->GetOriginalToDmaSipAddress() ",pNetSetup->GetOriginalToDmaSipAddress() ); //debug
			m_pSIPNetSetup->SetOriginalToDmaSipAddress(pNetSetup->GetOriginalToDmaSipAddress());
		}
		WORD type = pConfParty->GetSipPartyAddressType();
		WORD port = pConfParty->GetCallSignallingPort();//5060 - ema bug.
		m_pSIPNetSetup->SetRemoteDisplayName(pConfParty->GetName());

		m_pSIPNetSetup->SetRemoteSipAddressType(type);
		m_pSIPNetSetup->SetRemoteSignallingPort(port);
		m_pSIPNetSetup->SetMsConversationId(m_MsConfReq);
		TRACEINTO << " m_pSIPNetSetup->GetMsConversationId(): " << m_pSIPNetSetup->GetMsConversationId();
	}
	else
	{
		PASSERTMSG(!pConfParty,"pConfParty == null");
	}

	}
//	const sipMediaLinesEntrySt* pMediaLinesEntry = (const sipMediaLinesEntrySt*) &sdp.capsAndHeaders[sdp.sipMediaLinesOffset];

}
///////////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::SetFocusUri()   // we can save here only the dynamic section
{

	PTRACE(eLevelInfoNormal,"CSipAddPartyCntl::SetFocusUri  ");


	CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(m_monitorPartyId);
	const char* FocusUri = pConfParty->GetFocusUri();
	int FocusUriLen = strlen(FocusUri);

	m_FocusUri = new char[FocusUriLen+1];
	strncpy(m_FocusUri, FocusUri, FocusUriLen);
	m_FocusUri[FocusUriLen] = '\0';

	TRACEINTO << "m_FocusUri:" << FocusUri;

	m_isDMAAVMCU = TRUE;



}

/////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::CreateAVMCUMngrs()
{
	PTRACE(eLevelInfoNormal,"CSipAddPartyCntl::CreateAVMCUMngrs  ");

	if(!m_isDMAAVMCU)
	{	//Organizer
	m_pMsOrganizerMngr = new CMSOrganizerMngr;
	CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(m_monitorPartyId);
	BYTE isRejectForEscalation = FALSE;
	if(pConfParty && pConfParty->GetMsftMediaEscalationStatus() == eMsftEscalationActive)
	{
		PTRACE(eLevelInfoNormal,"CSipAddPartyCntl::CreateAVMCUMngrs-escalate p2p into conf  ");
	}
	CRsrcParams* pOrganizerRsrcParams = new CRsrcParams;
	*pOrganizerRsrcParams = m_pPartyAllocatedRsrc->GetRsrcParams(eLogical_ip_sigOrganizer);
	m_pMsOrganizerMngr->Create(pOrganizerRsrcParams,m_pConf,m_MsConfReq,GetPartyRsrcId(),m_pSIPNetSetup,m_serviceId , isRejectForEscalation);
	POBJDELETE(pOrganizerRsrcParams);
	}
	else
	{
		const CCommConf* pCommConf = m_pConf->GetCommConf();

		BuildNewSrcPartyAddress();

		// Focus Factory
		m_pMsFocusMngr = new CMSFocusMngr;

		CRsrcParams* pFocusRsrcParams = new CRsrcParams;
		*pFocusRsrcParams = m_pPartyAllocatedRsrc->GetRsrcParams(eLogical_ip_sigFocus);
		m_pMsFocusMngr->Create(pFocusRsrcParams, m_pConf,GetPartyRsrcId(),m_pSIPNetSetup,m_serviceId, m_FocusUri,m_strConfParamInfo.GetString());

		POBJDELETE(pFocusRsrcParams);
		//RDP Gw: update the current FocusUri to CommConf
		if(pCommConf)
		{
			TRACEINTO << "current FocusUri :" << m_FocusUri;
			DWORD len = strlen(m_FocusUri);
	   		CSegment* pSeg = new CSegment;
	   		*pSeg << len;
	   		pSeg->Put((unsigned char*)m_FocusUri, len);

 			 m_pTaskApi->UpdateDB((CTaskApp*) 0xffff, UPDATEFOCUSURI, (DWORD) 0, 0, pSeg);
		}
	}


}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::BuildNewSrcPartyAddress()
{
	const CCommConf* pCommConf = m_pConf->GetCommConf();
	const char* confDisplayName = pCommConf->GetDisplayName();

	TRACEINTO << "confDisplayName :" << confDisplayName
			  << "m_pSIPNetSetup->GetSrcPartyAddress() :" << m_pSIPNetSetup->GetSrcPartyAddress();


	const char* pReadPtr1 = strstr(confDisplayName, " ");

	if(pReadPtr1 !=NULL)
	{

		WORD len = pReadPtr1-confDisplayName;

		WORD NewFromlen = len + strlen(m_pSIPNetSetup->GetSrcPartyAddress())+ 2;

		char* FromAddrStr = new char[NewFromlen];
		memset(FromAddrStr, 0, NewFromlen);
		strncpy(FromAddrStr,confDisplayName,len);
		FromAddrStr[NewFromlen-1] = '\0';
		strcat(FromAddrStr,"@");
		strncat(FromAddrStr,m_pSIPNetSetup->GetSrcPartyAddress(),strlen(m_pSIPNetSetup->GetSrcPartyAddress()));
		FromAddrStr[NewFromlen -1 ] = '\0';

		TRACEINTO << "len :" << len
				<< "SrcPartyAddressLen :" << strlen(m_pSIPNetSetup->GetSrcPartyAddress())
				<< "SrcPartyAddress :" << m_pSIPNetSetup->GetSrcPartyAddress()
				<< "NewFromlen :" << NewFromlen
				<< "FromAddrStr :" << FromAddrStr;

		m_pSIPNetSetup->SetSrcPartyAddress(FromAddrStr);
		m_pSIPNetSetup->SetLocalSipAddress(FromAddrStr);
		SetLocalSipHostAddress();
	}

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::OnMSOrganizerEndConnection(CSegment* pParam)
{
	char FocusUri[FOCUS_URI_LEN+1];
	memset(FocusUri, '\0', FOCUS_URI_LEN+1);

	char MsConversationId[MS_CONVERSATION_ID_LEN+1];
	memset(MsConversationId, '\0', MS_CONVERSATION_ID_LEN+1);


	WORD FocusUriLen = 0;
	WORD MsConversationIdLen = 0;
	WORD MsConversationIdNetsetupLen = 0;
	STATUS status = STATUS_OK;


	MsConversationIdNetsetupLen = strlen(m_pSIPNetSetup->GetMsConversationId());
	BOOL isCallThroughDma = FALSE;
	*pParam >>  status >> isCallThroughDma >> FocusUriLen;

	PTRACE2INT(eLevelInfoNormal,"CSipAddPartyCntl::OnMSOrganizerEndConnection  - status: ",status);
	PTRACE2INT(eLevelInfoNormal,"CSipAddPartyCntl::OnMSOrganizerEndConnection  - isCallThroughDma: ",isCallThroughDma);
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();



	PTRACE(eLevelInfoNormal,"CSipAddPartyCntl::OnMSOrganizerEndConnection  - federate debug: ");
	SetLocalSipHostAddress();



	 BOOL bTreatnonDmaAsDmaCall = NO;
	std::string key1 = "MS_DEBUG_CONNECT_WITHOUT_DMA_AS_WITH";
	if (sysConfig)
	 {
		sysConfig->GetBOOLDataByKey(key1, bTreatnonDmaAsDmaCall);
	 }
	if(!isCallThroughDma && bTreatnonDmaAsDmaCall == NO)
	{
		CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(m_monitorPartyId);
		PTRACE(eLevelInfoNormal,"CSipAddPartyCntl::OnMSOrganizerEndConnection   -AV-MCU without DMA work as 2010 in RTV mode: ");
		if(pConfParty)
			pConfParty->SetMsftAvmcuState(eMsftAvmcu2010);
	}
    if(status == STATUS_OK)
    {
		if (!FocusUriLen || FocusUriLen > FOCUS_URI_LEN )
		{
			TRACEINTO << "FocusUriLen:" << FocusUriLen << ",  FocusUri is empty or too long";
			PASSERT_AND_RETURN(1);
		}

		*pParam >> FocusUri;

		m_FocusUri = new char[FocusUriLen+1];
		strncpy(m_FocusUri, FocusUri, FocusUriLen);
		m_FocusUri[FocusUriLen] = '\0';

		TRACEINTO << "m_FocusUri:" << FocusUri;

		TRACEINTO << " m_pSIPNetSetup->GetMsConversationId(): " << m_pSIPNetSetup->GetMsConversationId();


		if(!MsConversationIdNetsetupLen)
		{
			*pParam >>MsConversationIdLen;

			if(MsConversationIdLen && (MsConversationIdLen <= MS_CONVERSATION_ID_LEN) )
			{
				*pParam >> MsConversationId;
				m_pSIPNetSetup->SetMsConversationId(MsConversationId);
			}
			else
			{
				TRACEINTO << "MsConversationIdLen:" << MsConversationIdLen << ", Ms conversation id is empty or too long";
				PASSERT_AND_RETURN(1);
			}

		}



		TRACEINTO << " m_pSIPNetSetup->GetMsConversationId(): " << m_pSIPNetSetup->GetMsConversationId();



		// Focus Factory
		m_pMsFocusMngr = new CMSFocusMngr;

		CRsrcParams* pFocusRsrcParams = new CRsrcParams;
		*pFocusRsrcParams = m_pPartyAllocatedRsrc->GetRsrcParams(eLogical_ip_sigFocus);
		m_pMsFocusMngr->Create(pFocusRsrcParams, m_pConf,GetPartyRsrcId(),m_pSIPNetSetup,m_serviceId, m_FocusUri,m_strConfParamInfo.GetString());

		//RDP Gw: update the current FocusUri to CommConf
		const CCommConf* pCommConf = m_pConf->GetCommConf();
		if(pCommConf)
		{
			TRACEINTO << "current FocusUri :" << m_FocusUri;
			//pCommConf->SetFocusUriCurrently(m_FocusUri);
			DWORD len = strlen(m_FocusUri);
	   		CSegment* pSeg = new CSegment;
	   		*pSeg << len;
	   		pSeg->Put((unsigned char*)m_FocusUri, len);

 			 m_pTaskApi->UpdateDB((CTaskApp*) 0xffff, UPDATEFOCUSURI, (DWORD) 0, 0, pSeg);
		}

		m_EndFocus = FALSE;

		POBJDELETE(pFocusRsrcParams);
    }
    else
    {
    	//const CCommConf* pCommConf = m_pConf->GetCommConf();
    	if (!FocusUriLen)
		{
			TRACEINTO << "Focus URI is empty";

			//PASSERT_AND_RETURN(1);
			DBGPASSERT(101);
		}

    	PTRACE(eLevelInfoNormal, "CSipAddPartyCntl::OnMSOrganizerEndConnection  status fail - disconenct party");
    	m_pTaskApi->UpdateDB(m_pParty,DISCAUSE,NO_DISCONNECTION_CAUSE);
    	m_pTaskApi->EndAddParty(m_pParty,statTout);
   	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::UpdateToStrForSDP(char* ToAddrStr)
{

	PTRACE2(eLevelInfoNormal, "CSipAddPartyCntl::UpdateToStrForSDP ", ToAddrStr);

if(ToAddrStr == NULL)

{
	PTRACE(eLevelInfoNormal, "CSipAddPartyCntl::UpdateToStrForSDP going back");
	return;
}
	char buffSip[1024];
	memset(buffSip, '\0',sizeof(buffSip));
	strncpy(buffSip,ToAddrStr, sizeof(buffSip)-1);//strlen(ToAddrStr)); //KW


	 PTRACE2(eLevelInfoNormal, "CSipAddPartyCntl::UpdateMSConvId ", buffSip);

    char *p1 = NULL;
    char *p2 = NULL;

    char bufId[256]={0};

    //char buffSip[1024] = "sip:Ori01@ilw14.polycom.eng;gruu;opaque=app:conf:focus:id:HMGL1789";

    p1 = (char*) strstr(buffSip, "conf:");

    p2 = (char*) strstr(buffSip, "id:");
    if (p2){
          strncpy(bufId,p2,sizeof(bufId) - 1);
          bufId[sizeof(bufId) - 1] = '\0';
    }

    if(p1 && p2)
    {
           strncpy(p1+5, "audio-video:",12);

           strncpy(p1+5+12, bufId,12);
    }



    PTRACE2(eLevelInfoNormal, "CSipAddPartyCntl::UpdateMSConvId ", buffSip);



}


/*
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::UpdateToStrForSDP(char* ToAddrStr)
{
	PTRACE(eLevelInfoNormal, "CSipAddPartyCntl::UpdateMSConvId  ");


	int len = strlen(ToAddrStr);

	ALLOCBUFFER(TmpToAddStr,len);
	strncpy(TmpToAddStr,ToAddrStr,len);
	TmpToAddStr[len] = '\0';

	const char* pReadPtr1 = strstr(TmpToAddStr, "conf:");
	const char* FirstPtr1 = pReadPtr1 + 5; // should point after conf:
	int FirstBufLen = FirstPtr1-TmpToAddStr;

	PTRACE2INT(eLevelInfoNormal, "CSipAddPartyCntl::UpdateMSConvId ", FirstBufLen);

	ALLOCBUFFER(FirstBuf,FirstBufLen);
	strncpy(FirstBuf,TmpToAddStr,FirstBufLen);  // should hold the number

	const char* pReadPtr2 = strstr(TmpToAddStr,"id:");
	const char* pEndStr = pReadPtr1 + 3; // should point after id:
	int FocusUriNumLen = strlen(pEndStr);

	PTRACE2INT(eLevelInfoNormal, "CSipAddPartyCntl::UpdateMSConvId  FocusUriNumLen", FocusUriNumLen);

	ALLOCBUFFER(FocusNum,FocusUriNumLen);
	strncpy(FocusNum,pEndStr,FocusUriNumLen);  // should hold the number

	PTRACE2(eLevelInfoNormal, "CSipAddPartyCntl::UpdateMSConvId FocusNum ", FocusNum);

	char buffXml[2048];
	memset(buffXml, '\0',2048);
	sprintf("%saudio-video:id:%s",FirstBuf,FocusNum);

	PTRACE2(eLevelInfoNormal, "CSipAddPartyCntl::UpdateMSConvId ", FirstBuf);
	PTRACE2(eLevelInfoNormal, "CSipAddPartyCntl::UpdateMSConvId ", FirstBuf);
	PTRACE2(eLevelInfoNormal, "CSipAddPartyCntl::UpdateMSConvId ", FirstBuf);












	 // should hold the number
}
*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::OnMSFocusEndConnection(CSegment* pParam)
{
	BOOL IsMS2013Server = FALSE;
	STATUS status = STATUS_OK;

	char ToAddrStr[H243_NAME_LEN];
	memset(ToAddrStr, '\0', H243_NAME_LEN);

	WORD ToAddrStrLen = 0;

	*pParam >> IsMS2013Server >> status >> ToAddrStrLen;

	PTRACE2INT(eLevelInfoNormal, "CSipAddPartyCntl::OnMSFocusEndConnection, IsMS2013Server=", IsMS2013Server);

	if(ToAddrStrLen)
	{
		*pParam >> ToAddrStr;

		PTRACE2(eLevelInfoNormal, "CSipAddPartyCntl::OnMSFocusEndConnection ToAddrStr: ",ToAddrStr);
		m_pSIPNetSetup->SetToPartyAddress(ToAddrStr);
		m_pSIPNetSetup->SetRemoteSipAddress(ToAddrStr);
	}

	PTRACE2(eLevelInfoNormal, "CSipAddPartyCntl::OnMSFocusEndConnection AVMCU to:",m_pSIPNetSetup->GetToPartyAddress());

	if(status == STATUS_OK)
	{
		CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(m_monitorPartyId);
		if(!pConfParty)
		{
			PASSERTMSG(!pConfParty, "!pConfParty");
			return;
		}
		if(IsMS2013Server && pConfParty->GetMsftAvmcuState() != eMsftAvmcu2010 && isMsftSvc2013Supported() && m_isMsftEnv)
		{
			PTRACE(eLevelInfoNormal, "CSipAddPartyCntl::OnMSFocusEndConnection AVMCU-2013");
			pConfParty->SetMsftAvmcuState(eMsftAvmcu2013);

			//Event-Package
			const CCommConf* pCommConf = m_pConf->GetCommConf();
			eMsSvcVideoMode avMcuCascadeMode =((eMsSvcVideoMode)(pCommConf->GetMsSvcCascadeMode()));
			PTRACE2INT(eLevelInfoNormal, "CSipAddPartyCntl::OnMSFocusEndConnection  Lync2013 avMcuCascadeMode ",avMcuCascadeMode);
			CreateScmAccordingToAvMcuOptimizeMode();
			m_pMsEventPackageMngr = new CMSSubscriberMngr;

			CRsrcParams* pEventPackageRsrcParams = new CRsrcParams;
			*pEventPackageRsrcParams = m_pPartyAllocatedRsrc->GetRsrcParams(eLogical_ip_sigEventPackage);

			 m_pMsEventPackageMngr->Create(pEventPackageRsrcParams, m_pConf,m_MsConfReq,GetPartyRsrcId(),m_pSIPNetSetup,m_serviceId,m_FocusUri);
			 POBJDELETE(pEventPackageRsrcParams);

			 m_EndSubscriber = FALSE;

		}
		else
		{
			RTVVideoModeDetails rtvVidModeDetails;
			DWORD bitRate = 0;
			m_pSipLocalCaps->GetRtvCap(rtvVidModeDetails, bitRate);

			PTRACE2INT(eLevelInfoNormal, "CSipAddPartyCntl::OnMSFocusEndConnection AVMCU-2010 setting RTV, bitRate=", bitRate);

			// if resolution > VGA --> set it to VGA on AVMCU2010
			if (rtvVidModeDetails.Height*rtvVidModeDetails.Width>640*480)
			{
				PTRACE2INT(eLevelInfoNormal, "CSipAddPartyCntl::OnMSFocusEndConnection AVMCU-2010 changing RTV to VGA, videoModeType=", rtvVidModeDetails.videoModeType);
				rtvVidModeDetails.Height = 480;
				rtvVidModeDetails.Width = 640;
				rtvVidModeDetails.videoModeType = e_rtv_SD30;
//				bitRate = (bitRate>8000) ? 8000 : bitRate;
			}
			m_pIpInitialMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit);
			m_pIpInitialMode->SetRtvVideoParams(rtvVidModeDetails, cmCapReceiveAndTransmit, bitRate);
			m_pIpCurrentMode->SetRtvVideoParams(rtvVidModeDetails, cmCapReceiveAndTransmit, bitRate);

//			m_pIpInitialMode->SetRtvScm(rtvVidModeDetails.Width,rtvVidModeDetails.Height,rtvVidModeDetails.FR,cmCapReceiveAndTransmit, bitRate);
			m_pSipLocalCaps->RemoveCapSet(eMsSvcCapCode);
			m_MaxLocalCaps->RemoveCapSet(eMsSvcCapCode);
			m_pSipLocalCaps->SetRtvParams(rtvVidModeDetails, kRolePeople, bitRate);

			pConfParty->SetMsftAvmcuState(eMsftAvmcu2010);
			ContinueWithDialOutFlow();
		}
	}
	else
	{
		PTRACE(eLevelInfoNormal, "CSipAddPartyCntl::OnMSFocusEndConnection  status fail - disconnect party");
		DBGPASSERT(101);
		m_pTaskApi->UpdateDB(m_pParty,DISCAUSE,NO_DISCONNECTION_CAUSE);
		m_pTaskApi->EndAddParty(m_pParty,statTout);

	}


}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::OnMSSubscriberEndConnection(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSipAddPartyCntl::OnMSSubscriberEndConnection - Continue with dail out flow ");

	ContinueWithDialOutFlow();

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::ContinueWithDialOutFlow()
{
	//we can now send msgs to hw
		m_pPartyHWInterface = new CPartyInterface(m_pPartyAllocatedRsrc->GetPartyRsrcId(), m_pPartyAllocatedRsrc->GetConfRsrcId());
		m_pPartyHWInterface->SetRoomId(m_RoomId);

		m_lastReqId = SendCreateParty(E_NETWORK_TYPE_IP, m_bIsMrcCall);
		m_state = CREATE_MPL;

		//// in dial in we use it to send ringing
		CRsrcParams* pCsRsrcParams = new CRsrcParams;
		*pCsRsrcParams = m_pPartyAllocatedRsrc->GetRsrcParams(eLogical_ip_signaling);
		m_pPartyApi->SendCsResourceAllocated(pCsRsrcParams);
		POBJDELETE(pCsRsrcParams);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//LYNC_AVMCU_1080p30:
void CSipAddPartyCntl::CreateScmAccordingToAvMcuOptimizeMode()
{
	PASSERTMSG_AND_RETURN(!m_pConf, "LYNC_AVMCU_1080p30: CSipAddPartyCntl::CreateScmAccordingToAvMcuOptimizeMode - m_pConf is NULL");

	const CCommConf* pCommConf = m_pConf->GetCommConf();

	PASSERTMSG_AND_RETURN(!pCommConf, "LYNC_AVMCU_1080p30: CSipAddPartyCntl::CreateScmAccordingToAvMcuOptimizeMode - pCommConf is NULL");

	eMsSvcVideoMode      avMcuCascadeMode  =((eMsSvcVideoMode)(pCommConf->GetMsSvcCascadeMode()));
	EVideoResolutionType vidResolutionType = convertVideoTypeToResType(m_eLastAllocatedVideoPartyType);
	CConfParty*          pConfParty        = pCommConf->GetCurrentParty(m_monitorPartyId);

	m_maxResForAvMcu = vidResolutionType;

	PASSERTMSG_AND_RETURN(!pConfParty, "LYNC_AVMCU_1080p30: CSipAddPartyCntl::CreateScmAccordingToAvMcuOptimizeMode - m_pSipCntl is NULL");

	if (vidResolutionType == eCIF_Res || vidResolutionType == eSD_Res)
	{
		TRACEINTO << "LYNC_AVMCU_1080p30: CIF or SD no need to optimized it - avMcuCascadeMode:" << (DWORD)avMcuCascadeMode << ", vidResolutionType:" << vidResolutionType;
		return;
	}

	BOOL isSLynvAvMcu1080p30Enabled = FALSE;
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	pSysConfig->GetBOOLDataByKey("LYNC_AVMCU_1080p30_ENCODE_RESOLUTION", isSLynvAvMcu1080p30Enabled);

	if (avMcuCascadeMode == eMsSvcResourceOptimize && vidResolutionType > eSD_Res)
	{

		TRACEINTO << "LYNC_AVMCU_1080p30: changing main scm to SD - avMcuCascadeMode:eMsSvcResourceOptimize, vidResolutionType(>SD):" << vidResolutionType;

		pConfParty->SetMaxResolution(eSD_Res);
		SetScmForLync2013Call(m_pIpInitialMode,pConfParty);

		m_pIpInitialMode->Dump("CSipAddPartyCntl::CreateScmAccordingToAvMcuOptimizeMode initial",eLevelInfoNormal);

		//CHANGE RTV LOCAL CAPS ALSO TO SD and ms svc.
		m_pSipLocalCaps->CleanMedia(cmCapVideo);
		m_pSipLocalCaps->CleanSdesMedia(cmCapVideo);
		m_pSipLocalCaps->SetVideo(m_pIpInitialMode,m_pIpInitialMode->GetVideoBitRate(cmCapReceive, kRolePeople),
				                  pConfParty->GetServiceId(),eCopVideoFrameRate_None,pConfParty->GetMaxResolution());
		if(m_MaxLocalCaps)
			*m_MaxLocalCaps = *m_pSipLocalCaps;

	}
	else if (isSLynvAvMcu1080p30Enabled && avMcuCascadeMode == eMsSvcVideoOptimize && vidResolutionType > eSD_Res)
	{

	    TRACEINTO << "LYNC_AVMCU_1080p30: eMsSvcVideoOptimize - isSLynvAvMcu1080p30Enabled:TRUE, avMcuCascadeMode:eMsSvcVideoOptimize, vidResolutionType(>SD):"
	    		  << vidResolutionType;

		CMsSvcVideoMode*      MsSvcVidMode = new CMsSvcVideoMode();
		MsSvcVideoModeDetails MsSvcDetails;
		DWORD                 videoBitRate = m_pIpInitialMode->GetVideoBitRate(cmCapReceive, kRolePeople);
		Eh264VideoModeType    maxH264Mode = GetMaxH264VideoModeForMsSvcAccordingToSettings(pConfParty,pCommConf);

		maxH264Mode = min(eHD720Symmetric,maxH264Mode);
		MsSvcVidMode->GetMsSvcVideoParamsByRate(MsSvcDetails,(videoBitRate*100),maxH264Mode,E_VIDEO_RES_ASPECT_RATIO_DUMMY);
		m_pIpInitialMode->SetMsSvcScm(MsSvcDetails,cmCapReceive,videoBitRate);

		POBJDELETE(MsSvcVidMode);

	}

	if(m_maxResForAvMcu == eAuto_Res || m_maxResForAvMcu == eHD1080_Res || m_maxResForAvMcu == eHD1080p60_Res )
		PTRACE2INT(eLevelInfoNormal, "CSipAddPartyCntl::CreateScmAccordingToAvMcuOptimizeMode - LYNC_AVMCU_1080p30: error! shouldn't get max mode res in this val m_maxResForAvMcu ",m_maxResForAvMcu);

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipAddPartyCntl::SetLocalSipHostAddress()
{
	if (m_serviceId)
	{
		CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
		CConfIpParameters* pServiceParams = pIpServiceListManager->FindIpService(m_serviceId);
		if (pServiceParams == NULL)
		{
			PASSERTMSG((DWORD)this, "CSipCntl::SetLocalSipHostAddress - IP Service does not exist!!!");
			return;
		}

		CSmallString host = pServiceParams->GetRegistrarDomainName();
		if (host.IsEmpty())
			host = pServiceParams->GetSipProxyName();
		char*  strHost = const_cast<char*>(host.GetString());
		// In case the domain name is an IpV6 address - Add Brackets to the string.
		if (::isIpV6Str(strHost) == TRUE)
		{
			//ipAddressStruct *tpmAddr = new ipAddressStruct;
			ipAddressStruct tpmAddr, *ptpmAddr = &tpmAddr;
			memset(ptpmAddr,0,sizeof(ipAddressStruct));
			stringToIpV6(ptpmAddr,strHost);
			strHost = ipToString(*ptpmAddr,strHost,1);
		}
		m_pSIPNetSetup->SetLocalHost(const_cast<char*>(strHost));
 	}
}
