/*
 * SIPSlavePartyControl.cpp
 *
 *  Created on: Feb 14, 2011
 *      Author: mvolovik
 */

#include "SIPPartyControl.h"
#include "SIPSlavePartyControl.h"
#include "SIPInternals.h"
#include "SipCaps.h"
#include "SipScm.h"
#include "SipNetSetup.h"
#include "ConfPartyOpcodes.h"
#include "ConfDef.h"
#include "Conf.h"
#include "ConfApi.h"
#include "CommModeInfo.h"
#include "Capabilities.h"
#include "TaskApi.h"
#include "TaskApp.h"
#include "PartyApi.h"
#include "FaultsDefines.h"
#include "HlogApi.h"
#include "BridgeMoveParams.h"
#include "ConfPartyProcess.h"

#define MFA_CONNECT_TIME	20
#define CONNECT_TIME		100 // all internal MCU units receive 5 seconds to reply, and the network has 50 second -> 5+5+50+5+5 <= 100

extern "C" void SipSlavePartyEntryPoint(void* appParam);
extern "C" void SipPartyOutCreateEntryPoint(void* appParam);


////////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CSipSlavePartyCntl)

	ONEVENT(ALLOCATE_PARTY_RSRC_IND,		ALLOCATE_RSC,				CSipSlavePartyCntl::OnPartyRsrcAllocatingRsp)
	ONEVENT(ACK_IND,						CREATE_MPL,					CSipSlavePartyCntl::OnPartyMPLCreatingRsp)
	ONEVENT(SIP_PARTY_CHANS_CONNECTED,		PARTY_SETUP,				CSipSlavePartyCntl::OnPartyChannelsConnectedSetup)
	ONEVENT(SIP_PARTY_CHANS_CONNECTED,		PARTY_IN_CONNECTING_STATE,	CSipSlavePartyCntl::OnPartyChannelsConnectedConnecting)
	ONEVENT(SIP_PARTY_CHANS_CONNECTED,		CONNECT_BRIDGES,			CSipSlavePartyCntl::OnPartyChannelsConnectedConnecting)
	ONEVENT(PARTY_AUDIO_CONNECTED,			CONNECT_BRIDGES,			CSipSlavePartyCntl::OnAudConnectPartyConnectAudio)
	ONEVENT(PARTY_AUDIO_CONNECTED,			PARTY_IN_CONNECTING_STATE,	CSipSlavePartyCntl::OnAudConnectPartyConnectAudio)
	ONEVENT(PARTY_VIDEO_CONNECTED,			CONNECT_BRIDGES,			CSipSlavePartyCntl::OnVideoBrdgConnected)
	ONEVENT(PARTY_VIDEO_CONNECTED,			ANYCASE,					CSipSlavePartyCntl::OnVideoBrdgConnected)
	ONEVENT(PARTY_VIDEO_IVR_MODE_CONNECTED,	CONNECT_BRIDGES,			CSipSlavePartyCntl::OnVideoBrdgConnected)
	ONEVENT(PARTY_VIDEO_IVR_MODE_CONNECTED,	ANYCASE,					CSipSlavePartyCntl::OnVideoBrdgConnected)
	ONEVENT(IPPARTYCONNECTED,				CONNECT_BRIDGES,			CSipSlavePartyCntl::OnPartyRemoteConnected)

PEND_MESSAGE_MAP(CSipSlavePartyCntl,CSipPartyCntl);


////////////////////////////////////////////////////////////////////////////////
CSipSlavePartyCntl::CSipSlavePartyCntl()
{
	m_TipPartyType = eTipNone;
	m_RoomId = 0xFFFF;
	m_SentAckToMaster = FALSE;
	SetInterfaceType(SIP_INTERFACE_TYPE);
	m_VideoPartyType = eVideo_party_type_dummy ;
}

////////////////////////////////////////////////////////////////////////////////
CSipSlavePartyCntl::~CSipSlavePartyCntl()
{
}

////////////////////////////////////////////////////////////////////////////////
void CSipSlavePartyCntl::Create(PartyControlInitParameters&	 partyControInitParam,PartyControlDataParameters &partyControlDataParams)
{
	SetupSIPSlaveConParameters(partyControInitParam,partyControlDataParams);
	StartPartyConnection();
}

/////////////////////////////////////////////////////////////////////////////
void  CSipSlavePartyCntl::StartPartyConnection()
{
	m_connectingState = IP_CONNECTING;

	StartTimer(MFACONNECTTOUT, MFA_CONNECT_TIME*SECOND);
	AllocatePartyResources();
}

/////////////////////////////////////////////////////////////////////////////
//Send CH323AddPartyCntl event to resource allocator process and changes state to ALLOCATE.
void CSipSlavePartyCntl::AllocatePartyResources()
{
	PTRACEPARTYID(eLevelInfoNormal, "CSipSlavePartyCntl::AllocatePartyResources", GetPartyRsrcId());
	EAllocationPolicy bIsFreeVideoResources = eAllocateAllRequestedResources;

	BYTE portGaugeThresholdExceeded = FALSE;
	if( m_TipPartyType == eTipSlaveAux )
		m_VideoPartyType = eVideo_party_type_none;

/*	switch(m_TipPartyType)
	{
	    case eTipSlaveLeft:
			m_VideoPartyType = m_VideoPartyType;
			break;
	    case eTipSlaveRigth:
			m_VideoPartyType = m_VideoPartyType;
			break;
		case eTipSlaveAux:
			m_VideoPartyType = eVideo_party_type_none;
			break;
	}
	*/
	SetSlaveTelepresenseEPInfo(); //_e_m_
	CreateAndSendAllocatePartyResources( eIP_network_party_type, m_VideoPartyType, bIsFreeVideoResources,
			                             portGaugeThresholdExceeded,FALSE,0/*art capacit*/ ,m_TipPartyType, m_RoomId);
	m_state = ALLOCATE_RSC;
}


//_e_m_
////////////////////////////////////////////////////////////////////////////////
void CSipSlavePartyCntl::SetSlaveTelepresenseEPInfo()
{
	CMedString str = "";
	str<<"TipPartyType " << m_TipPartyType <<" RoomId " << m_RoomId;
	PTRACE2(eLevelInfoNormal, "CSipSlavePartyCntl::SetSlaveTelepresenseEPInfo : ", str.GetString());

	PASSERTMSG_AND_RETURN(!m_telepresenseEPInfo, "m_telepresenseEPInfo == NULL");

	m_telepresenseEPInfo->SetEPtype(eTelePresencePartyCTS);
	m_telepresenseEPInfo->SetNumOfLinks(3);
	m_telepresenseEPInfo->SetRoomID(m_RoomId);
	m_telepresenseEPInfo->SetPartyMonitorID(m_monitorPartyId);
	m_telepresenseEPInfo->SetWaitForUpdate(FALSE);

	switch (m_TipPartyType)
	{
		case eTipSlaveLeft:
			m_telepresenseEPInfo->SetLinkNum(1);
			break;
		case eTipSlaveRigth:
			m_telepresenseEPInfo->SetLinkNum(2);
			break;
		default:
			m_telepresenseEPInfo->SetLinkNum(-1);
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
void CSipSlavePartyCntl::OnPartyRsrcAllocatingRsp(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal, "CSipSlavePartyCntl::OnPartyRsrcAllocatingRsp", GetPartyRsrcId());
	CPartyRsrcDesc* pTempPartyAllocatedRsrc = new CPartyRsrcDesc;
	pTempPartyAllocatedRsrc->DeSerialize(SERIALEMBD, *pParam);

	if (m_pPartyHWInterface)
		POBJDELETE(m_pPartyHWInterface);

	DWORD status = pTempPartyAllocatedRsrc->GetStatus();
	eVideoPartyType allocatedVideoPartyType = pTempPartyAllocatedRsrc->GetVideoPartyType();

	//============================================================================================
	// BRIDGE-10439
	// PartyCntl must know the rsrcId unconditionally, as even if the resource-allocation fails,
	// the party will try to remove itself from the lists using its resource ID.
	//============================================================================================
	POBJDELETE(m_pPartyAllocatedRsrc);
	m_pPartyAllocatedRsrc = new CPartyRsrcDesc(*pTempPartyAllocatedRsrc);

	if (status != STATUS_OK)
	{
		if (DIALOUT == m_type)
			ClearUnusedPartyId(pTempPartyAllocatedRsrc->GetPartyRsrcId());
		
		DeleteTimer(MFACONNECTTOUT);
		char buf[300];
		sprintf(buf,"CSipSlavePartyCntl::OnPartyRsrcAllocatingRsp: STATUS ( %d )",status);
	    PTRACEPARTYID(eLevelInfoNormal, buf, GetPartyRsrcId());
	    if( STATUS_WAIT_FOR_RSRC_AND_ASK_AGAIN == status)
	    {
	    	///Resource Allocator tries to fulfill the allocation request by reconfigurating the DSP, we will set a timer and retry to allocate the resources
	    	if(m_isWaitingForRsrcAndAskAgain)
	    	{
	    		WaitForRsrcAndAskAgain();
				POBJDELETE(pTempPartyAllocatedRsrc);
	    	  	return;
	    	}
	    	else
	    	{
	    		///We only retry to allocate resources in case of STATUS_WAIT_FOR_RSRC_AND_ASK_AGAIN just once
	    	    PTRACE2PARTYID(eLevelInfoNormal, "CSipSlavePartyCntl::OnPartyRsrcAllocatingRsp: STATUS_WAIT_FOR_RSRC_AND_ASK_AGAIN but the m_isWaitingForRsrcAndAskAgain is set to NO!!!!! party: ", m_partyConfName, GetPartyRsrcId());
	    	    PASSERT(1);
	    	}
	    }
		if( m_VideoPartyType == eVideo_party_type_none && allocatedVideoPartyType != eVideo_party_type_none && status == STATUS_NUMBER_OF_AUDIO_PARTIES_EXCEEDED )
		{
			PTRACE(eLevelInfoNormal, "CSipSlavePartyCntl::OnPartyRsrcAllocatingRsp  : no audio ports saving video resource -reallocating to lowest video resource possible ");
			pTempPartyAllocatedRsrc->DumpToTrace();
			POBJDELETE(pTempPartyAllocatedRsrc);
			CreateAndSendAllocatePartyResources( eIP_network_party_type, eCP_H264_upto_CIF_video_party_type, eAllocateAllRequestedResources,FALSE,FALSE,0/*art cap*/ ,m_TipPartyType, m_RoomId);
			return;
		}
		if (( (STATUS_INSUFFICIENT_VIDEO_RSRC==status) || (STATUS_NUMBER_OF_VIDEO_PARTIES_EXCEEDED==status) ) &&
									 (m_type==DIALOUT) && (m_connectDelayCounter < NUM_OF_CONNECT_DELAY_RETRIES))
		{
			PTRACEPARTYID(eLevelInfoNormal,"CSipSlavePartyCntl::OnPartyRsrcAllocatingRsp :Temporary Video Resources Deficiency Connection Delay ", GetPartyRsrcId());
//			char buf[300];
//			sprintf(buf,"CH323AddPartyCntl::OnRsrcAllocatePartyRspAllocate: Connect Delay ( %d ) ,ConnectDelayTime (%d)",m_connectDelayCounter+1,m_connectDelay);
//	    	PTRACEPARTYID(eLevelInfoNormal, buf);
			//Invoke Connect Delay to allow the system DeAllocation of resources that reserved by a Party in SD
			//Conf that at last need less Resources.
			//m_state=IDLE;
			CSegment *pPartyDetails = new CSegment;
			m_pConfRcvMbx->Serialize(*pPartyDetails);

			*pPartyDetails  << (char*)m_confName
							<< m_McuNumber
							<< m_isRecording;

			m_connectDelayCounter++;
			StartTimer(CONNECTDELAY,400,pPartyDetails); //It tooks 3sec to deAlloc unNecesary resource(worst case)
	    }
		else
		{
			PTRACE2PARTYID(eLevelInfoNormal, "CSipSlavePartyCntl::OnPartyRsrcAllocatingRsp : ALLOCATION FAILED!!! do not continue process : ",CProcessBase::GetProcess()->GetStatusAsString(status).c_str(), GetPartyRsrcId());
			CSegment *pSeg = new CSegment;
			*pSeg << m_name;
			m_pTaskApi->UpdateDB(NULL,DISCAUSE,RESOURCES_DEFICIENCY,1,pSeg); // Disconnnect cause
			m_pTaskApi->EndAddParty(m_pParty,statIllegal);
		}

		pTempPartyAllocatedRsrc->DumpToTrace();
		POBJDELETE(pTempPartyAllocatedRsrc);
		m_pTaskApi->SendAckFromSlaveToMaster(m_MasterRsrcId, statIllegal, DEFAULT_PARTY_ID, m_TipPartyType, eVideo_party_type_dummy);
		return;
	}

	eNetworkPartyType networkPartyType =  pTempPartyAllocatedRsrc->GetNetworkPartyType();
	if(networkPartyType!=eIP_network_party_type)
	{
		PTRACE2PARTYID(eLevelInfoNormal, "CSipSlavePartyCntl::OnPartyRsrcAllocatingRsp ALLOCATION FAILED!!! do not continue process eNetworkPartyType!= eIP_network_party_type, eNetworkPartyType = ",eNetworkPartyTypeNames[networkPartyType], GetPartyRsrcId());
		PASSERT(1);
		CSegment *pSeg = new CSegment;
		*pSeg << m_name;
		m_pTaskApi->UpdateDB(NULL,DISCAUSE,RESOURCES_DEFICIENCY,1,pSeg); // Disconnnect cause
		m_pTaskApi->EndAddParty(m_pParty,statIllegal);
		pTempPartyAllocatedRsrc->DumpToTrace();
		POBJDELETE(pTempPartyAllocatedRsrc);
		m_pTaskApi->SendAckFromSlaveToMaster(m_MasterRsrcId, statIllegal, DEFAULT_PARTY_ID, m_TipPartyType, eVideo_party_type_dummy);
		return;
	}


	PTRACEPARTYID(eLevelInfoNormal, "CSipSlavePartyCntl::OnPartyRsrcAllocatingRsp : Allocation is OK", GetPartyRsrcId());
	POBJDELETE(pTempPartyAllocatedRsrc);

	pParam->Get((BYTE*)(&m_udpAddresses), sizeof(UdpAddresses));

	m_pPartyAllocatedRsrc->DumpToTrace();

	//CheckResourceAllocationTowardsRequest(1);
	m_eLastAllocatedVideoPartyType = m_pPartyAllocatedRsrc->GetVideoPartyType();

	COsQueue ConfRcvMbx = m_pConf->GetRcvMbx();
	m_confName = m_pConf->GetName();
	// in dial out Now really allocate the party task

	m_pPartyApi = new CPartyApi;
	m_pPartyApi->Create(m_entryPoint,
	                    ConfRcvMbx,
	                    ConfRcvMbx,
	                    m_pPartyAllocatedRsrc->GetPartyRsrcId(),
	                    m_monitorPartyId,
	                    m_monitorConfId,
	                    "",
	                    m_name,
	                    m_confName,
	                    m_serviceId,
	                    m_termNum,
	                    m_McuNumber,
	                    "",
	                    m_voice,
	                    1,
	                    m_IsGateWay);

	m_pParty = m_pPartyApi->GetTaskAppPtr();

#ifdef LOOKUP_TABLE_DEBUG_TRACE
	TRACEINTO << "D.K."
			<< "  PartyName:"        << m_name
			<< ", PartyId:"          << m_pParty->GetPartyId() << "(" << m_pPartyAllocatedRsrc->GetPartyRsrcId() << ")"
			<< ", IsDialOut:"        << m_type
			<< ", Pointer:"          << std::hex << m_pParty;
#endif

	InsertPartyResourcesToGlobalRsrcRoutingTbl();

	//we can now send msgs to hw
	m_pPartyHWInterface = new CPartyInterface(m_pPartyAllocatedRsrc->GetPartyRsrcId(), m_pPartyAllocatedRsrc->GetConfRsrcId());
	m_pPartyHWInterface->SetRoomId(m_RoomId);
	SendCreateParty(E_NETWORK_TYPE_IP);
	m_state = CREATE_MPL;

	// in dial in we use it to send ringing
	CRsrcParams* pCsRsrcParams = new CRsrcParams;
	*pCsRsrcParams = m_pPartyAllocatedRsrc->GetRsrcParams(eLogical_ip_signaling);
	m_pPartyApi->SendCsResourceAllocated(pCsRsrcParams);
	POBJDELETE(pCsRsrcParams);
}

////////////////////////////////////////////////////////////////////////////////
void  CSipSlavePartyCntl::OnPartyMPLCreatingRsp(CSegment* pParam)
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
			PTRACE2INT(eLevelInfoNormal, "CSipSlavePartyCntl::OnPartyMPLCreatingRsp : CreateParty Ack Msg Received - ", m_disconnectionCause);
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

			PTRACEPARTYID(eLevelInfoNormal, "CSipSlavePartyCntl::OnPartyMPLCreatingRsp : ACK with fail : ", GetPartyRsrcId());
			str << "CSipSlavePartyCntl::OnMplAckCreate : create Party failed, Party Rsrc ID - " << GetPartyRsrcId();
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
			m_pTaskApi->SendAckFromSlaveToMaster(m_MasterRsrcId, statIllegal, DEFAULT_PARTY_ID, m_TipPartyType, eVideo_party_type_dummy);
			POBJDELETE(pSeg);

		}
	}
	else
	{
		str << "CSipSlavePartyCntl::OnMplAckCreate : Invalid Opcode, Response Opcode - " << ackOpcode;
		CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT, INVALID_OPCODE_RESPONSE, MAJOR_ERROR_LEVEL, str.GetString(), TRUE);
		PTRACE2INT(eLevelInfoNormal, "CSipSlavePartyCntl::OnPartyMPLCreatingRsp : unexpected opcode %d", ackOpcode);
	}
}

////////////////////////////////////////////////////////////////////////////////
void CSipSlavePartyCntl::EstablishCall(BYTE eTransportType)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipSlavePartyCntl::EstablishCall", GetPartyRsrcId());

	PTRACE2INT(eLevelInfoNormal,"CSipSlavePartyCntl::EstablishCall party type ", m_TipPartyType);
	m_state = PARTY_SETUP;

	StartTimer(CONNECTTOUT, CONNECT_TIME*SECOND);

	const char* alternativeAddrStr;

    alternativeAddrStr = NULL;

	CRsrcParams* pRtpRsrcParams = new CRsrcParams;
	*pRtpRsrcParams = m_pPartyAllocatedRsrc->GetRsrcParams(eLogical_rtp);
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

	m_pPartyApi->SipConfEstablishSlaveCall(pRtpRsrcParams, pCsRsrcParams, m_udpAddresses,(CSipNetSetup *) m_pSIPNetSetup,
										m_pSipLocalCaps, m_pIpInitialMode, m_eConfTypeForAdvancedVideoFeatures,
										m_strConfParamInfo.GetString(), eTransportType, refMcuNumber, refTerminalNumber,
										m_bNoVideRsrcForVideoParty, m_RoomId, m_TipPartyType);

	m_pTaskApi->UpdateDB(m_pParty, PARTYSTATE, PARTY_CONNECTING);

	POBJDELETE(pRtpRsrcParams);
	POBJDELETE(pCsRsrcParams);
}

/////////////////////////////////////////////////////////////////////////////
void CSipSlavePartyCntl::OnPartyChannelsConnectedSetup(CSegment* pParam) // channels connected or updated
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipSlavePartyCntl::OnPartyChannelsConnectedSetup", GetPartyRsrcId());
	m_pIpCurrentMode->DeSerialize(NATIVE,*pParam);
	m_pIpCurrentMode->CopyMediaModeToOppositeDirection(cmCapAudio,cmCapReceive);
	m_pIpCurrentMode->CopyMediaModeToOppositeDirection(cmCapVideo,cmCapReceive);
//	UpdateVideoTxModeForAsymmetricModes();
	ConnectBridgesSetup();
	if(m_SentAckToMaster == TRUE)
		m_pTaskApi->SendAckFromSlaveToMaster(m_MasterRsrcId, statOK, GetPartyRsrcId(), m_TipPartyType, m_eLastAllocatedVideoPartyType);
}

/////////////////////////////////////////////////////////////////////////////
// The function connect bridges (A+V+FECC) after in channels were connected in dial out calls or
// in and out channels were connected in dial in calls.
void CSipSlavePartyCntl::ConnectBridgesSetup() // channels connected or updated
{
	PTRACE(eLevelInfoNormal,"CSipSlavePartyCntl::ConnectBridgesSetup");

	DWORD initialRate	= m_pIpInitialMode->GetMediaBitRate(cmCapVideo,cmCapReceive);
	DWORD currentRate	= m_pIpCurrentMode->GetMediaBitRate(cmCapVideo,cmCapReceive);
	BYTE  bEqualRates	= (initialRate == currentRate);
    BYTE  bIsCp         = (m_pIpInitialMode->GetConfType() == kCp && m_pIpInitialMode->GetConfMediaType()!=eMixAvcSvcVsw);
	// CSmallString str;
	// str << m_partyConfName << " conf type - " << m_pIpInitialMode->GetConfType() << " init rate - " << initialRate << " current rate - " << currentRate;

	if (bIsCp || bEqualRates)
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
	if(m_TipPartyType != eTipSlaveAux)
		ConnectPartyToVideoBridge(m_pIpCurrentMode);


	m_state = CONNECT_BRIDGES;
}

/////////////////////////////////////////////////////////////////////////////
void  CSipSlavePartyCntl::OnAudConnectPartyConnectAudio(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CSipSlavePartyCntl::OnAudConnectPartyConnectAudio : Name - ",m_partyConfName, GetPartyRsrcId());

	if (m_eAudBridgeConnState == eBridgeDisconnected)
	{
		DBGPASSERT(GetPartyRsrcId());
		PTRACE2PARTYID(eLevelInfoNormal,"CSipSlavePartyCntl::OnAudConnectPartyConnectAudio : Connect has received after disconnect. Name - ",m_partyConfName, GetPartyRsrcId());
	}

	else
	{
		HandleAudioBridgeConnectedInd(pParam);

		if (AreTwoDirectionsConnectedToAudioBridge())
		{
		//	m_pPartyApi->AudioLoopBack(0);   //turn off party audio loop-back//yael???
			if (m_isFullBitRateConnect)
				m_pTaskApi->EndAddParty(m_pParty,statOK);
			m_pPartyApi->SendAudioBridgeConnected();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CSipSlavePartyCntl::OnPartyRemoteConnected(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipSlavePartyCntl::OnPartyRemoteConnected", GetPartyRsrcId());
	CSipComMode* pRemoteMode = new CSipComMode;
	pRemoteMode->DeSerialize(NATIVE, *pParam);
	BYTE bIsEndConfChangeMode = FALSE;
	*pParam >> bIsEndConfChangeMode;
	OffererPartyConnected(pRemoteMode);
	POBJDELETE(pRemoteMode);

//	if( m_pTaskApi )// do that for all slaves //&& eTipSlaveAux == m_TipPartyType )
//		m_pTaskApi->SendCAMGeneralNotifyCommand( GetPartyRsrcId(), eCAM_EVENT_PARTY_IS_TIP_SLAVE, (DWORD)0 );
}

/////////////////////////////////////////////////////////////////////////////
void CSipSlavePartyCntl::OffererPartyConnected(CSipComMode* pRemoteMode)
{
	// meaning we received 200 ok with
	// if guess succeed than remote mode is current mode
	PTRACE(eLevelInfoNormal,"CSipSlavePartyCntl::OffererPartyConnected: (Bridge mode is OK)");
	EConfType confType = m_pIpInitialMode->GetConfType();
	*m_pIpInitialMode = *pRemoteMode;
	m_pIpInitialMode->SetConfType(confType);

	ON(m_isFullBitRateConnect);
	DeleteTimer(CONNECTTOUT);
	m_state = PARTY_IN_CONNECTING_STATE;
	// since we update the target mode when party out return now we are updating the current mode
	*m_pIpCurrentMode = *m_pIpInitialMode;
	m_pPartyApi->SipConfConnectCall();
}

/////////////////////////////////////////////////////////////////////////////
// The function connect party to conf after out channels of Offerer calls were connected
void CSipSlavePartyCntl::OnPartyChannelsConnectedConnecting(CSegment* pParam) // channels connected or updated
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipSlavePartyCntl::OnPartyChannelsConnectedConnecting", GetPartyRsrcId());
	m_pIpCurrentMode->DeSerialize(NATIVE,*pParam);
	m_pTaskApi->EndAddParty(m_pParty,statOK);

	if(m_SentAckToMaster == FALSE)
	{
		m_pTaskApi->SendAckFromSlaveToMaster(m_MasterRsrcId, statOK, GetPartyRsrcId(), m_TipPartyType, m_eLastAllocatedVideoPartyType);
		m_SentAckToMaster = TRUE;
	}
	else{
		PTRACEPARTYID(eLevelInfoNormal,"CSipSlavePartyCntl::OnPartyChannelsConnectedConnecting ack already sent on slave", GetPartyRsrcId());

	}
}


/////////////////////////////////////////////////////////////////////////////
void CSipSlavePartyCntl::ChangeScm(CIpComMode* pIpScm)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipSlavePartyCntl::ChangeScm", GetPartyRsrcId());
	EConfType confType = m_pIpInitialMode->GetConfType();
	*m_pIpInitialMode = *pIpScm;
	m_pIpInitialMode->SetConfType(confType);
	SetPartyStateUpdateDbAndCdrAfterEndConnected(SECONDARY_CAUSE_NO_VIDEO_CONNECTION);

}

/////////////////////////////////////////////////////////////////////////////
void CSipSlavePartyCntl::OnVideoBrdgConnected(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipSlavePartyCntl::OnVideoBrdgConnected", GetPartyRsrcId());
	if (m_eVidBridgeConnState == eBridgeDisconnected)
	{
		DBGPASSERT(GetPartyRsrcId());
		PTRACEPARTYID(eLevelInfoNormal,"CSipSlavePartyCntl::OnVideoBrdgConnected : Connect has received after disconnect", GetPartyRsrcId());
	}
	else
	{
		HandleVideoBridgeConnectedInd(pParam);
		if (AreTwoDirectionsConnectedToVideoBridge()
			|| (IsInDirectionConnectedToVideoBridge() && m_pIpCurrentMode->IsMediaOff(cmCapVideo, cmCapTransmit)))
		{
			m_pPartyApi->SendVideoBridgeConnected();
			SetPartyStateUpdateDbAndCdrAfterEndConnected(SECONDARY_CAUSE_NO_VIDEO_CONNECTION);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CSipSlavePartyCntl::SetupSIPSlaveConParameters(PartyControlInitParameters&	 partyControInitParam,PartyControlDataParameters &partyControlDataParams)
{
    CConfParty* pConfParty = partyControInitParam.pConfParty;
	if (!pConfParty)
	{
	    PTRACE(eLevelError, "CSipSlavePartyCntl::SetupSIPSlaveConParameters - pConfParty is NULL");
	    DBGPASSERT(1124);
	    return;
	}

	pConfParty->SetIsTipCall(TRUE);
	pConfParty->SetTIPPartySubType( partyControlDataParams.tipPartyType );
	pConfParty->SetRemoteName(pConfParty->GetName());
	pConfParty->SetVisualPartyName(pConfParty->GetName());//michael

	CIpComMode*	pPartyScm = NewAndGetPartyCntlScm(partyControInitParam,partyControlDataParams);
	CSipNetSetup* pIpNetSetup = NewAndSetupSipNetSetup(pConfParty,pPartyScm,partyControInitParam,partyControlDataParams);
	SetSeviceIdForConfParty(pConfParty);

	DWORD vidBitrate = pPartyScm->GetTotalVideoRate();
	CSipCaps* pSIPCaps = NewAndGetLocalSipCaps(pPartyScm,pConfParty,pIpNetSetup,vidBitrate,partyControInitParam,partyControlDataParams);

	AllocMemory();
	m_monitorConfId				= partyControInitParam.monitorConfId;
	m_monitorPartyId			= partyControInitParam.pConfParty->GetPartyId();
	m_TipPartyType 				= partyControlDataParams.tipPartyType;
	m_RoomId 					= partyControlDataParams.roomID;
	m_pConf						= partyControInitParam.pConf;
	m_pParty					= partyControlDataParams.pParty;

	*m_pSipLocalCaps			= *pSIPCaps;
	*m_pSIPNetSetup				= *pIpNetSetup;
	*m_pIpInitialMode 			= *pPartyScm;

	m_serviceId					= pConfParty->GetServiceId();
	m_VideoPartyType 			= partyControlDataParams.masterVideoPartyType;
	m_pTerminalNumberingManager = partyControInitParam.pTerminalNumberingManager;
	m_pAudioInterface			= partyControInitParam.pAudioBridgeInterface;
	m_pVideoBridgeInterface		= partyControInitParam.pVideoBridgeInterface;
	m_pFECCBridge				= partyControInitParam.pFECCBridge;
	m_pContentBridge			= partyControInitParam.pContentBridge;
	m_pConfAppMngrInterface		= partyControInitParam.pConfAppMngrInterface;
	m_MasterRsrcId              = partyControlDataParams.peerPartyRsrcID;

	const char* strConfName = m_pConf->GetName();
	m_name = new char[H243_NAME_LEN];
	memset(m_name, '\0', H243_NAME_LEN);
	strncpy(m_name, partyControlDataParams.pStrPartyName, H243_NAME_LEN - 1);

	m_pIpInitialMode->SetMediaOff(cmCapData, cmCapReceiveAndTransmit);

	m_pIpInitialMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);
	m_pIpInitialMode->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);
	m_pIpInitialMode->SetTipAuxFPS(eTipAuxNone);

	m_pSipLocalCaps->SetTipAuxFPS(eTipAuxNone);
	m_pSipLocalCaps->CleanMedia(cmCapBfcp); // bfcp
	m_pSipLocalCaps->CleanMedia(cmCapVideo, kRolePresentation); // content

	SetFullName(m_name, strConfName);
	PTRACE2PARTYID(eLevelInfoNormal, "CSipSlavePartyCntl::Create: Name - ",
			m_partyConfName, GetPartyRsrcId());

	m_pBridgeMoveParams = new CBridgeMoveParams;

	m_pTaskApi = new CConfApi;
	m_pTaskApi->CreateOnlyApi(m_pConf->GetRcvMbx(), this);
	m_pTaskApi->SetLocalMbx(m_pConf->GetLocalQueue());

	if (m_TipPartyType == eTipSlaveAux)
		SetPartyToAudioOnly();

	m_entryPoint = SipSlavePartyEntryPoint;

	POBJDELETE(pSIPCaps);
	POBJDELETE(pPartyScm);
	POBJDELETE(pIpNetSetup);
}

CIpComMode* CSipSlavePartyCntl::NewAndGetPartyCntlScm(PartyControlInitParameters&	 partyControInitParam,PartyControlDataParameters &partyControlDataParams )
{
	CConfParty* pConfParty = partyControInitParam.pConfParty;
	const char* strPartyName = pConfParty->GetName();

	BYTE interfaceType = pConfParty->GetNetInterfaceType();
	CIpComMode* pPartyScm = new CIpComMode(
			*(partyControlDataParams.pIpMasterInitialMode));
	pPartyScm->SetTipMode(eTipModePossible);

	WORD bIsAudioOnly = pConfParty->GetVoice();
	if (partyControlDataParams.tipPartyType == eTipSlaveAux)
		pPartyScm->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit,
				kRolePeople);

	pPartyScm->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit,
			kRoleContentOrPresentation);

	pPartyScm->SetIsLpr(FALSE);

	return pPartyScm;
}

CSipNetSetup* CSipSlavePartyCntl::NewAndSetupSipNetSetup(
		CConfParty* pConfParty, CIpComMode*pPartyScm,
		PartyControlInitParameters& partyControInitParam,
		PartyControlDataParameters &partyControlDataParams) {

	DWORD confRate = pPartyScm->GetCallRate();
	DWORD vidBitrate = pPartyScm->GetTotalVideoRate();

	char tempName[64];
	memset(&tempName, '\0', IPV6_ADDRESS_LEN);
	ipToString(pConfParty->GetIpAddress(), tempName, 1);

	CMedString msg1;
	msg1 << "vidBitrate:" << vidBitrate << ", confRate:" << confRate << ", pConfParty->GetIpAddress():" << tempName;
	PTRACE2(eLevelInfoNormal, "CSipSlavePartyCntl::NewAndSetupSipNetSetup - ", msg1.GetString());

	CSipNetSetup* pSipNetSetup = new CSipNetSetup;
	const char* strSipAddress = pConfParty->GetSipPartyAddress();

	WORD type = pConfParty->GetSipPartyAddressType();
	WORD port = pConfParty->GetCallSignallingPort(); //5060 - ema bug.

	pSipNetSetup->SetRemoteDisplayName(pConfParty->GetName());
	pSipNetSetup->SetRemoteSipAddress(strSipAddress);
	pSipNetSetup->SetRemoteSipAddressType(type);
	pSipNetSetup->SetRemoteSignallingPort(port);

	CConfPartyProcess* pConfPartyProcess =
			(CConfPartyProcess*) CConfPartyProcess::GetProcess();

	pSipNetSetup->SetMaxRate(confRate);
	pSipNetSetup->SetMinRate(0);
	pSipNetSetup->SetConfId(partyControInitParam.monitorConfId);

	if ((pConfParty->GetNodeType()) == 0) {
		pSipNetSetup->SetEndpointType(2);
	} else {
		if ((pConfParty->GetNodeType()) == 1) {
			pSipNetSetup->SetEndpointType(0);
		}
	}

	InitDisplayNameForNetSetup(pSipNetSetup);

	return pSipNetSetup;
}

CSipCaps* CSipSlavePartyCntl::NewAndGetLocalSipCaps(CIpComMode* pPartyScm,CConfParty* pConfParty,CSipNetSetup* pIpNetSetup,DWORD& vidBitrate,PartyControlInitParameters& partyControInitParam, PartyControlDataParameters& partyControlDataParams)
{
	CSipCaps* pSIPCaps = new CSipCaps;
	SetSIPPartyCapsAndVideoParam(pPartyScm, pSIPCaps, pConfParty, vidBitrate,
			pIpNetSetup->GetEnableSipICE(), pIpNetSetup, 0, FALSE, pConfParty->GetServiceId(),partyControInitParam,partyControlDataParams);

	return pSIPCaps;
}
