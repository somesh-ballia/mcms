#include "MplApiSpecialCommandHandler.h"
#include "MplMcmsProtocol.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "OpcodesMcmsCardMngrTB.h"
#include "OpcodesMcmsAudio.h"
#include "OpcodesMcmsAudioCntl.h"
#include "OpcodesMcmsVideo.h"
#include "IpMfaOpcodes.h"
#include "MplApiProcess.h"
#include "MplApiOpcodes.h"
#include "OpcodesMcmsCardMngrIvrCntl.h"
#include "ApiStatuses.h"
#include "MplApiStatuses.h"
#include "IpCmReq.h"
#include "VideoStructs.h"
#include "OpcodesMcmsNetQ931.h"
#include "TraceStream.h"
#include "Q931Structs.h"
#include "OpcodesMcmsPCM.h"
#include "IceCmReq.h"
#include "OpcodesMcmsCardMngrICE.h"
#include "IpPartyMonitorDefinitions.h"
#include "TipStructs.h"
#include "IpRtpReq.h"
#include "OpcodesMcmsCardMngrTIP.h"
#include "OpcodesMcmsCardMngrBFCP.h"
#include "BfcpStructs.h"
#include "LibsCommonHelperFuncs.h"
#include "OpcodesMrmcCardMngrMrc.h"
#include "MrcStructs.h"


extern char* ResourceTypeToString(APIU32 resourceType);
extern char* LogicalResourceTypeToString(APIU32 resourceType);
extern const char* CntrlTypeToString(APIU32 cntrlType);
static const WORD CHANNEL_ID_FROM_RTM = 0xFDFD;


bool CMplApiSpecialCommandHandler::m_IsInit = false;
CMplApiProcess *CMplApiSpecialCommandHandler::m_pMplApiProcess = NULL;
CSpecialHandlerMethodMap CMplApiSpecialCommandHandler::m_SpecialHandlerMap;

pthread_mutex_t mapMutex = PTHREAD_MUTEX_INITIALIZER;


bool CMplApiSpecialCommandHandler::Init()
{
	// lock static variable usage for other threads
	CTaskApp::MutexLocker  locker(mapMutex);

	m_pMplApiProcess = dynamic_cast<CMplApiProcess*>((CProcessBase::GetProcess()));

	m_SpecialHandlerMap[CONF_MPL_CREATE_PARTY_REQ		] = &CMplApiSpecialCommandHandler::CreatePartyReq;
	m_SpecialHandlerMap[TB_MSG_CLOSE_PORT_REQ			] = &CMplApiSpecialCommandHandler::ClosePortReq;
	m_SpecialHandlerMap[CONF_MPL_DELETE_PARTY_REQ		] = &CMplApiSpecialCommandHandler::DeletePartyReq;
	m_SpecialHandlerMap[ACK_IND							] = &CMplApiSpecialCommandHandler::AckInd;
	m_SpecialHandlerMap[TB_MSG_CONNECT_REQ				] = &CMplApiSpecialCommandHandler::TBConnectDisconnectReq;
	m_SpecialHandlerMap[TB_MSG_DISCONNECT_REQ			] = &CMplApiSpecialCommandHandler::TBConnectDisconnectReq;
	m_SpecialHandlerMap[CONFPARTY_CM_UPDATE_UDP_ADDR_REQ		] = &CMplApiSpecialCommandHandler::H323CMUpdateOpenUDPAddrReq;
	m_SpecialHandlerMap[CONFPARTY_CM_OPEN_UDP_PORT_REQ		] = &CMplApiSpecialCommandHandler::H323CMUpdateOpenUDPAddrReq;
	m_SpecialHandlerMap[CONFPARTY_CM_CLOSE_UDP_PORT_REQ		] = &CMplApiSpecialCommandHandler::H323CMCloseUDPPortReq;
	//same CM commands (from SIP party and not from H323 party)
	m_SpecialHandlerMap[SIP_CM_UPDATE_UDP_ADDR_REQ		] = &CMplApiSpecialCommandHandler::H323CMUpdateOpenUDPAddrReq;
	m_SpecialHandlerMap[SIP_CM_OPEN_UDP_PORT_REQ		] = &CMplApiSpecialCommandHandler::H323CMUpdateOpenUDPAddrReq;
	m_SpecialHandlerMap[SIP_CM_CLOSE_UDP_PORT_REQ		] = &CMplApiSpecialCommandHandler::H323CMCloseUDPPortReq;

	m_SpecialHandlerMap[IP_CM_START_PREVIEW_CHANNEL		] = &CMplApiSpecialCommandHandler::TBStartPreviewReq;
	m_SpecialHandlerMap[IP_CM_STOP_PREVIEW_CHANNEL		] = &CMplApiSpecialCommandHandler::TBStopPreviewReq;

	m_SpecialHandlerMap[VIDEO_ENCODER_CHANGE_LAYOUT_REQ	] = &CMplApiSpecialCommandHandler::VideoEncoderChangeLayoutReq;
	m_SpecialHandlerMap[VIDEO_UPDATE_DECODER_RESOLUTION_REQ	] = &CMplApiSpecialCommandHandler::VideoUpdateDecoderResolutionReq;
	m_SpecialHandlerMap[VIDEO_ENCODER_DSP_SMART_SWITCH_CHANGE_LAYOUT_REQ	] = &CMplApiSpecialCommandHandler::VideoEncoderDSPSmartSwitchChangeLayoutReq;

	m_SpecialHandlerMap[AC_OPEN_CONF_REQ				] = &CMplApiSpecialCommandHandler::ACRequest;
	m_SpecialHandlerMap[AC_CLOSE_CONF_REQ				] = &CMplApiSpecialCommandHandler::ACRequest;
	m_SpecialHandlerMap[AC_UPDATE_CONF_PARAMS_REQ		] = &CMplApiSpecialCommandHandler::ACRequest;
	m_SpecialHandlerMap[AC_OPEN_CONF_RESEND             ] = &CMplApiSpecialCommandHandler::ACShadowRequest;
	m_SpecialHandlerMap[AC_LAYOUT_CHANGE_COMPLETE_REQ	] = &CMplApiSpecialCommandHandler::ACRequest;

	m_SpecialHandlerMap[IVR_PLAY_MESSAGE_REQ			] = &CMplApiSpecialCommandHandler::IVRRequest;
	m_SpecialHandlerMap[IVR_STOP_PLAY_MESSAGE_REQ		] = &CMplApiSpecialCommandHandler::IVRRequest;
	m_SpecialHandlerMap[IVR_PLAY_MUSIC_REQ				] = &CMplApiSpecialCommandHandler::IVRRequest;
	m_SpecialHandlerMap[IVR_STOP_PLAY_MUSIC_REQ			] = &CMplApiSpecialCommandHandler::IVRRequest;
	m_SpecialHandlerMap[IVR_START_IVR_REQ				] = &CMplApiSpecialCommandHandler::IVRRequest;
	m_SpecialHandlerMap[IVR_STOP_IVR_REQ				] = &CMplApiSpecialCommandHandler::IVRRequest;
	m_SpecialHandlerMap[IVR_SHOW_SLIDE_REQ				] = &CMplApiSpecialCommandHandler::IVRRequest;
	m_SpecialHandlerMap[IVR_STOP_SHOW_SLIDE_REQ			] = &CMplApiSpecialCommandHandler::IVRRequest;
	m_SpecialHandlerMap[IVR_FAST_UPDATE_REQ				] = &CMplApiSpecialCommandHandler::IVRRequest;
	m_SpecialHandlerMap[IVR_RECORD_ROLL_CALL_REQ		] = &CMplApiSpecialCommandHandler::IVRRequest;
	m_SpecialHandlerMap[IVR_STOP_RECORD_ROLL_CALL_REQ	] = &CMplApiSpecialCommandHandler::IVRRequest;

	m_SpecialHandlerMap[MOVE_RSRC_ART_REQ				] = &CMplApiSpecialCommandHandler::MoveRsrcArt;
	m_SpecialHandlerMap[MOVE_RSRC_VIDEO_ENC_REQ			] = &CMplApiSpecialCommandHandler::MoveRsrcVideoEnc;
	m_SpecialHandlerMap[MOVE_RSRC_VIDEO_DEC_REQ			] = &CMplApiSpecialCommandHandler::MoveRsrcVideoDec;
	m_SpecialHandlerMap[MOVE_RSRC_NET_REQ				] = &CMplApiSpecialCommandHandler::MoveRsrcNet;
	m_SpecialHandlerMap[MOVE_RSRC_MRMP_REQ				] = &CMplApiSpecialCommandHandler::MoveRsrcMrmp;

	//Net special Command Handler
	m_SpecialHandlerMap[NET_SETUP_REQ			] = &CMplApiSpecialCommandHandler::NetSetupReq ;
	m_SpecialHandlerMap[NET_CLEAR_REQ			] = &CMplApiSpecialCommandHandler::NetClearReq ;
	m_SpecialHandlerMap[NET_DISCONNECT_ACK_REQ		] = &CMplApiSpecialCommandHandler::NetDisconnectAckReq ;
	m_SpecialHandlerMap[NET_ALERT_REQ			] = &CMplApiSpecialCommandHandler::NetAlertReq ;
	m_SpecialHandlerMap[NET_CONNECT_REQ			] = &CMplApiSpecialCommandHandler::NetConnectReq ;
	m_SpecialHandlerMap[IP_CM_RTCP_MSG_REQ			] = &CMplApiSpecialCommandHandler::RtcpMsgReq ;
	m_SpecialHandlerMap[IP_CM_RTCP_RTPFB_REQ			] = &CMplApiSpecialCommandHandler::RtcpFlowControlMsgReq ;

	//ICE
	m_SpecialHandlerMap[ICE_MAKE_ANSWER_REQ				] = &CMplApiSpecialCommandHandler::IceGeneralReq ;
	m_SpecialHandlerMap[ICE_MODIFY_SESSION_ANSWER_REQ	] = &CMplApiSpecialCommandHandler::IceGeneralReq ;
	m_SpecialHandlerMap[ICE_MODIFY_SESSION_OFFER_REQ	] = &CMplApiSpecialCommandHandler::IceGeneralReq ;
    m_SpecialHandlerMap[ICE_MAKE_OFFER_REQ				] = &CMplApiSpecialCommandHandler::IceGeneralReq ;
    m_SpecialHandlerMap[ICE_PROCESS_ANSWER_REQ			] = &CMplApiSpecialCommandHandler::IceGeneralReq ;
	m_SpecialHandlerMap[ICE_CLOSE_SESSION_REQ       	] = &CMplApiSpecialCommandHandler::IceGeneralReq ;

	//m_SpecialHandlerMap[PCM_INDICATION			] = &CMplApiSpecialCommandHandler::PCMMessage ;
	//m_SpecialHandlerMap[PCM_CONFIRM				] = &CMplApiSpecialCommandHandler::PCMMessage ;
	m_SpecialHandlerMap[TB_MSG_CONNECT_PCM_REQ	] = &CMplApiSpecialCommandHandler::TBConnectDisconnectPCMReq ;
	m_SpecialHandlerMap[TB_MSG_DISCONNECT_PCM_REQ   ] = &CMplApiSpecialCommandHandler::TBConnectDisconnectPCMReq ;

	//TIP
	m_SpecialHandlerMap[IP_CM_TIP_START_NEGOTIATION_REQ	] = &CMplApiSpecialCommandHandler::TipStartNegotiationReq;
	m_SpecialHandlerMap[IP_CM_TIP_END_NEGOTIATION_REQ	] = &CMplApiSpecialCommandHandler::TipEndNegotiationReq;
	m_SpecialHandlerMap[IP_CM_TIP_CONTENT_MSG_REQ		] = &CMplApiSpecialCommandHandler::TipContentMsgReq;
	m_SpecialHandlerMap[IP_MSG_UPDATE_ON_TIP_CALL_REQ 	] = &CMplApiSpecialCommandHandler::TipUpdateCloseReq;
	m_SpecialHandlerMap[IP_MSG_CLOSE_TIP_CALL_REQ 		] = &CMplApiSpecialCommandHandler::TipUpdateCloseReq;
	m_SpecialHandlerMap[H323_RTP_PARTY_MONITORING_REQ   ] = &CMplApiSpecialCommandHandler::PartyMonitoringReq;
	//m_SpecialHandlerMap[IP_CM_TIP_KILL_OBJECT_REQ 		] = &CMplApiSpecialCommandHandler::TipKillObjectReq;
	m_SpecialHandlerMap[IP_CM_TIP_KILL_TIP_CONTEXT_REQ	] = &CMplApiSpecialCommandHandler::TipKillTipContextReq;

	m_SpecialHandlerMap[IP_CM_RTCP_VIDEO_PREFERENCE_REQ	] = &CMplApiSpecialCommandHandler::TBVideoPreferenceReq ;
	m_SpecialHandlerMap[IP_CM_RTCP_RECEIVER_BANDWIDTH_REQ ] = &CMplApiSpecialCommandHandler::TBReceiverBandWidthReq ;
	m_SpecialHandlerMap[CONF_PARTY_MRMP_OPEN_CHANNEL_REQ] = &CMplApiSpecialCommandHandler::OpenChannelReq ;
	m_SpecialHandlerMap[CONF_PARTY_MRMP_UPDATE_CHANNEL_REQ] = &CMplApiSpecialCommandHandler::UpdateChannelReq ;
	m_SpecialHandlerMap[H323_RTP_FECC_TOKEN_RESPONSE_REQ] = &CMplApiSpecialCommandHandler::RtpFeccTokenResponseReq ;
//	m_SpecialHandlerMap[CONF_PARTY_MRMP_VIDEO_SOURCES_REQ] = &CMplApiSpecialCommandHandler::VideoSourcesReq;

//	m_SpecialHandlerMap[CONF_PARTY_MRMP_RTCP_FIR_REQ] = &CMplApiSpecialCommandHandler::MrmpRtcpFirReq;

	// BFCP
	m_SpecialHandlerMap[IP_CM_BFCP_MESSAGE_REQ] 		= &CMplApiSpecialCommandHandler::TBBfcpMessageReq;

	// VSR
	m_SpecialHandlerMap[IP_CM_RTCP_VSR_REQ] 			= &CMplApiSpecialCommandHandler::VsrMsgReq;

	// MS SVC PLI
	m_SpecialHandlerMap[IP_CM_RTCP_MS_SVC_PLI_REQ] 		= &CMplApiSpecialCommandHandler::MsSvcPliMsgReq;

	// MS SVC MUX/DMUX
	m_SpecialHandlerMap[CONFPARTY_CM_INIT_ON_LYNC_CALL_REQ] 			= &CMplApiSpecialCommandHandler::MsSvcP2PInitReq;
	m_SpecialHandlerMap[CONFPARTY_CM_INIT_ON_AVMCU_CALL_REQ] 			= &CMplApiSpecialCommandHandler::MsSvcAvMcuInitReq;
	m_SpecialHandlerMap[CONFPARTY_CM_MUX_ON_AVMCU_CALL_REQ] 			= &CMplApiSpecialCommandHandler::MsSvcAvMcuMuxReq;
	m_SpecialHandlerMap[CONFPARTY_CM_DMUX_ON_AVMCU_CALL_REQ] 			= &CMplApiSpecialCommandHandler::MsSvcAvMcuDmuxReq;
	m_SpecialHandlerMap[RTP_PARTY_MONITORING_AV_MCU_REQ] 			    = &CMplApiSpecialCommandHandler::MsSvcAvMcuMonitoringReq;

	//FEC
	m_SpecialHandlerMap[ACTIVATE_PACKET_LOSS_REQ] 		= &CMplApiSpecialCommandHandler::TBPacketLossReq;

	m_SpecialHandlerMap[ADD_VIDEO_OPERATION_POINT_SET]  = &CMplApiSpecialCommandHandler::AddRemoveVideoOperationPointSetReq;
	m_SpecialHandlerMap[REMOVE_VIDEO_OPERATION_POINT_SET]= &CMplApiSpecialCommandHandler::AddRemoveVideoOperationPointSetReq;

	// DTLS
	m_SpecialHandlerMap[IP_CM_DTLS_START_REQ]= &CMplApiSpecialCommandHandler::DtlsStartReq;
	m_SpecialHandlerMap[IP_CM_DTLS_CLOSE_REQ]= &CMplApiSpecialCommandHandler::DtlsCloseReq;


	m_IsInit = true;
	return true;
}

CMplApiSpecialCommandHandler::CMplApiSpecialCommandHandler(CMplMcmsProtocol &mplMcmsProt)
:m_MplMcmsProt(mplMcmsProt)
{
	static STATUS oneTimeCall = CMplApiSpecialCommandHandler::Init();
}

CMplApiSpecialCommandHandler::~CMplApiSpecialCommandHandler()
{}

STATUS CMplApiSpecialCommandHandler::HandeSpecialCommand(void* param)
{
	PASSERTMSG_AND_RETURN_VALUE(!m_IsInit,
			"CMplApiSpecialCommandHandler is not initialized",
			STATUS_FAIL);

	STATUS status = STATUS_OK;
	OPCODE opcode = m_MplMcmsProt.getCommonHeaderOpcode();

	HandleSpecialRequest handleMethod = NULL;

	if ( opcode ) // do not remove the condition - it creates scope for MutexLocker
	{
		// lock static variable usage for other threads
		CTaskApp::MutexLocker  locker(mapMutex);

		CSpecialHandlerMethodMap::const_iterator it =  m_SpecialHandlerMap.find(opcode);
		if (m_SpecialHandlerMap.end() != it)
			handleMethod = it->second;
	}
	if (NULL != handleMethod)
		status = (this->*handleMethod)(param);

	return status;

}

STATUS CMplApiSpecialCommandHandler::RtcpFlowControlMsgReq(void* param)
{
	STATUS status = STATUS_OK;

  	int conf_id    = m_MplMcmsProt.getPortDescriptionHeaderConf_id();
	int party_id   = m_MplMcmsProt.getPortDescriptionHeaderParty_id();


	const ConnToCardTableEntry *pConnToCardTableEntry = GetMainARTphysicalInfoHeaderByConfAndPartyID(conf_id, party_id);

	if( pConnToCardTableEntry!=NULL )
	{
	    ConnToCardTableEntry connToCardTableEntry(*pConnToCardTableEntry);


	    TCmRtcpRTPFB tbRtcpRTPFBMsg;
	    memcpy(&tbRtcpRTPFBMsg, m_MplMcmsProt.getpData(), sizeof(TCmRtcpRTPFB));
	    (tbRtcpRTPFBMsg.tCmRtcpHeader).uDspNumber = connToCardTableEntry.unitId;
	    (tbRtcpRTPFBMsg.tCmRtcpHeader).uPortNumber = connToCardTableEntry.portId;

	    connToCardTableEntry.physicalRsrcType=ePhysical_res_none;
	    connToCardTableEntry.unitId=0;
	    SetPhysicalInfoHeader(&connToCardTableEntry);

	    m_MplMcmsProt.AddData( sizeof(TCmRtcpRTPFB),(const char*) &tbRtcpRTPFBMsg);
	}
	else
	{
	  PrintAssert(conf_id, party_id,m_MplMcmsProt.getCommonHeaderOpcode());
		status = STATUS_PROBLEMS_IN_SHARED_MEMORY;
	}
	return status;
}

STATUS CMplApiSpecialCommandHandler::RtcpMsgReq(void* param)
{
	STATUS status = STATUS_OK;

  	int conf_id    = m_MplMcmsProt.getPortDescriptionHeaderConf_id();
	int party_id   = m_MplMcmsProt.getPortDescriptionHeaderParty_id();

	const ConnToCardTableEntry *pConnToCardTableEntry = GetMainARTphysicalInfoHeaderByConfAndPartyID(conf_id, party_id);

	if( pConnToCardTableEntry!=NULL )
	{
	    ConnToCardTableEntry connToCardTableEntry(*pConnToCardTableEntry);


	    TCmRtcpMsg tbRtcpMsg;
	    memcpy(&tbRtcpMsg, m_MplMcmsProt.getpData(), sizeof(TCmRtcpMsg));
	    (tbRtcpMsg.tCmRtcpHeader).uDspNumber = connToCardTableEntry.unitId;
	    (tbRtcpMsg.tCmRtcpHeader).uPortNumber = connToCardTableEntry.portId;

	    connToCardTableEntry.physicalRsrcType=ePhysical_res_none;
	    connToCardTableEntry.unitId=0;
	    SetPhysicalInfoHeader(&connToCardTableEntry);

	    m_MplMcmsProt.AddData( sizeof(TCmRtcpMsg),(const char*) &tbRtcpMsg);
	}
	else
	{
	  PrintAssert(conf_id, party_id,m_MplMcmsProt.getCommonHeaderOpcode());
		status = STATUS_PROBLEMS_IN_SHARED_MEMORY;
	}
	return status;
}

STATUS CMplApiSpecialCommandHandler::TBPacketLossReq(void* param)
{
	PTRACE(eLevelInfoNormal,"CMplApiSpecialCommandHandler::TBPacketLossReq");
	STATUS status = m_MplMcmsProt.ValidateDataSize(	sizeof(TActivePacketLossReq));
		if(STATUS_OK != status)
		{
			return status;
		}
		APIU32 connectionId = 0xffffffff;
		TActivePacketLossReq tbMsgStart;
		memcpy(&tbMsgStart, m_MplMcmsProt.getpData(), sizeof(TActivePacketLossReq));

		connectionId = m_MplMcmsProt.getPortDescriptionHeaderConnection_id();
		if(0xffffffff != connectionId)
		{
			ConnToCardTableEntry connToCardTableEntry;
			status = m_pMplApiProcess->GetSharedMemoryMap()->Get(connectionId, connToCardTableEntry);

			if (STATUS_OK == status)
			{
				connToCardTableEntry.unitId=0;
				SetPhysicalInfoHeader(&connToCardTableEntry);
			}

		}

		m_MplMcmsProt.AddData( sizeof(TActivePacketLossReq),(const char*) &tbMsgStart);

		return status;
}

STATUS CMplApiSpecialCommandHandler::TipKillTipContextReq(void* param)
{
	PTRACE(eLevelInfoNormal,"MplApiSpecialCommandHandler::TipKillTipContextReq");
	STATUS status = STATUS_OK;

  	int conf_id    = m_MplMcmsProt.getPortDescriptionHeaderConf_id();
	int party_id   = m_MplMcmsProt.getPortDescriptionHeaderParty_id();
    eLogicalResourceTypes lrt = (eLogicalResourceTypes) m_MplMcmsProt.getPortDescriptionHeaderLogical_resource_type_1();

	const ConnToCardTableEntry* pConnToCardTableEntry = GetPhysicalInfoHeaderByConfAndPartyID(conf_id,party_id,ePhysical_art, lrt);

	if( pConnToCardTableEntry==NULL )
	{
	    pConnToCardTableEntry=GetPhysicalInfoHeaderByConfAndPartyID(conf_id,party_id,ePhysical_art_light);
	}

	if( pConnToCardTableEntry!=NULL )
	{
	    ConnToCardTableEntry connToCardTableEntry(*pConnToCardTableEntry);

	    connToCardTableEntry.physicalRsrcType=ePhysical_res_none;
	    connToCardTableEntry.unitId=0;
	    SetPhysicalInfoHeader(&connToCardTableEntry);
	}
	else
	{
	  PrintAssert(conf_id, party_id,m_MplMcmsProt.getCommonHeaderOpcode());
		status = STATUS_PROBLEMS_IN_SHARED_MEMORY;
	}
	return status;
}

STATUS CMplApiSpecialCommandHandler::TipStartNegotiationReq(void* param)
{
	PTRACE(eLevelInfoNormal,"MplApiSpecialCommandHandler::TipStartNegotiationReq");
	STATUS status = STATUS_OK;

  	int conf_id    = m_MplMcmsProt.getPortDescriptionHeaderConf_id();
	int party_id   = m_MplMcmsProt.getPortDescriptionHeaderParty_id();
    eLogicalResourceTypes lrt = (eLogicalResourceTypes) m_MplMcmsProt.getPortDescriptionHeaderLogical_resource_type_1();

	const ConnToCardTableEntry* pConnToCardTableEntry = GetPhysicalInfoHeaderByConfAndPartyID(conf_id,party_id,ePhysical_art, lrt);

	if( pConnToCardTableEntry==NULL )
	{
	    pConnToCardTableEntry=GetPhysicalInfoHeaderByConfAndPartyID(conf_id,party_id,ePhysical_art_light);
	}

	if( pConnToCardTableEntry!=NULL )
	{
	    ConnToCardTableEntry connToCardTableEntry(*pConnToCardTableEntry);


	    mcTipStartNegotiationReq tipStartNegotiationReq;
	    memcpy(&tipStartNegotiationReq, m_MplMcmsProt.getpData(), sizeof(mcTipStartNegotiationReq));
	    tipStartNegotiationReq.unitId = connToCardTableEntry.unitId;
	    tipStartNegotiationReq.portId = connToCardTableEntry.portId;

	    connToCardTableEntry.physicalRsrcType=ePhysical_res_none;
	    connToCardTableEntry.unitId=0;
	    SetPhysicalInfoHeader(&connToCardTableEntry);

	    m_MplMcmsProt.AddData( sizeof(mcTipStartNegotiationReq),(const char*) &tipStartNegotiationReq);
	}
	else
	{
	  PrintAssert(conf_id, party_id,m_MplMcmsProt.getCommonHeaderOpcode());
		status = STATUS_PROBLEMS_IN_SHARED_MEMORY;
	}
	return status;
}

STATUS CMplApiSpecialCommandHandler::TipEndNegotiationReq(void* param)
{
	STATUS status = STATUS_OK;

  	int conf_id    = m_MplMcmsProt.getPortDescriptionHeaderConf_id();
	int party_id   = m_MplMcmsProt.getPortDescriptionHeaderParty_id();
    eLogicalResourceTypes lrt = (eLogicalResourceTypes) m_MplMcmsProt.getPortDescriptionHeaderLogical_resource_type_1();

	const ConnToCardTableEntry* pConnToCardTableEntry = GetPhysicalInfoHeaderByConfAndPartyID(conf_id,party_id,ePhysical_art, lrt);

	if( pConnToCardTableEntry==NULL )
	{
	    pConnToCardTableEntry=GetPhysicalInfoHeaderByConfAndPartyID(conf_id,party_id,ePhysical_art_light);
	}

	if( pConnToCardTableEntry!=NULL )
	{
	    ConnToCardTableEntry connToCardTableEntry(*pConnToCardTableEntry);


	    mcTipEndNegotiationReq tipEndNegotiationReq;
	    memcpy(&tipEndNegotiationReq, m_MplMcmsProt.getpData(), sizeof(mcTipEndNegotiationReq));
	    tipEndNegotiationReq.unitId = connToCardTableEntry.unitId;
	    tipEndNegotiationReq.portId = connToCardTableEntry.portId;

	    connToCardTableEntry.physicalRsrcType=ePhysical_res_none;
	    connToCardTableEntry.unitId=0;
	    SetPhysicalInfoHeader(&connToCardTableEntry);

	    m_MplMcmsProt.AddData( sizeof(mcTipEndNegotiationReq),(const char*) &tipEndNegotiationReq);
	}
	else
	{
	  PrintAssert(conf_id, party_id,m_MplMcmsProt.getCommonHeaderOpcode());
		status = STATUS_PROBLEMS_IN_SHARED_MEMORY;
	}
	return status;
}

STATUS CMplApiSpecialCommandHandler::TipContentMsgReq(void* param)
{
	STATUS status = STATUS_OK;

  	int conf_id    = m_MplMcmsProt.getPortDescriptionHeaderConf_id();
	int party_id   = m_MplMcmsProt.getPortDescriptionHeaderParty_id();
    eLogicalResourceTypes lrt = (eLogicalResourceTypes) m_MplMcmsProt.getPortDescriptionHeaderLogical_resource_type_1();

	const ConnToCardTableEntry* pConnToCardTableEntry = GetPhysicalInfoHeaderByConfAndPartyID(conf_id,party_id,ePhysical_art, lrt);

	if( pConnToCardTableEntry==NULL )
	{
	    pConnToCardTableEntry=GetPhysicalInfoHeaderByConfAndPartyID(conf_id,party_id,ePhysical_art_light);
	}

	if( pConnToCardTableEntry!=NULL )
	{
	    ConnToCardTableEntry connToCardTableEntry(*pConnToCardTableEntry);


	    mcTipContentMsgReq tipContentMsgReq;
	    memcpy(&tipContentMsgReq, m_MplMcmsProt.getpData(), sizeof(mcTipContentMsgReq));
	    tipContentMsgReq.unitId = connToCardTableEntry.unitId;
	    tipContentMsgReq.portId = connToCardTableEntry.portId;

	    connToCardTableEntry.physicalRsrcType=ePhysical_res_none;
	    connToCardTableEntry.unitId=0;
	    SetPhysicalInfoHeader(&connToCardTableEntry);

	    m_MplMcmsProt.AddData( sizeof(mcTipContentMsgReq),(const char*) &tipContentMsgReq);
	}
	else
	{
	  PrintAssert(conf_id, party_id,m_MplMcmsProt.getCommonHeaderOpcode());
		status = STATUS_PROBLEMS_IN_SHARED_MEMORY;
	}
	return status;
}

STATUS CMplApiSpecialCommandHandler::TipUpdateCloseReq(void* param)
{
	STATUS status = STATUS_OK;

  	int conf_id    = m_MplMcmsProt.getPortDescriptionHeaderConf_id();
	int party_id   = m_MplMcmsProt.getPortDescriptionHeaderParty_id();
    eLogicalResourceTypes lrt = (eLogicalResourceTypes) m_MplMcmsProt.getPortDescriptionHeaderLogical_resource_type_1();

	mcTipMsgInfoReq tipPartyInfoReq;
    memcpy(&tipPartyInfoReq, m_MplMcmsProt.getpData(), sizeof(mcTipMsgInfoReq));

    const ConnToCardTableEntry* pCenterEntry = GetPhysicalInfoHeaderByConfAndPartyID( conf_id, party_id, ePhysical_art, lrt);

	if( pCenterEntry )
	{
	    tipPartyInfoReq.centerTipChannelPartyId.unDspNum = pCenterEntry->unitId;
	    tipPartyInfoReq.centerTipChannelPartyId.unPortNum = pCenterEntry->portId;

	    unsigned int leftPartyId = tipPartyInfoReq.leftTipChannelDescr.unPartyId;
	    const ConnToCardTableEntry* pLeftEntry = GetPhysicalInfoHeaderByConfAndPartyID( conf_id, leftPartyId, ePhysical_art, lrt);
		if( pLeftEntry )
		{
		    tipPartyInfoReq.leftTipChannelDescr.unDspNum = pLeftEntry->unitId;
		    tipPartyInfoReq.leftTipChannelDescr.unPortNum = pLeftEntry->portId;
		}

	    unsigned int rightPartyId = tipPartyInfoReq.rightTipChannelDescr.unPartyId;
	    const ConnToCardTableEntry* pRightEntry = GetPhysicalInfoHeaderByConfAndPartyID( conf_id, rightPartyId, ePhysical_art, lrt);
		if( pRightEntry )
		{
		    tipPartyInfoReq.rightTipChannelDescr.unDspNum = pRightEntry->unitId;
		    tipPartyInfoReq.rightTipChannelDescr.unPortNum = pRightEntry->portId;
		}

	    unsigned int auxPartyId = tipPartyInfoReq.auxTipChannelDescr.unPartyId;
	    const ConnToCardTableEntry* pAuxEntry = GetPhysicalInfoHeaderByConfAndPartyID( conf_id, auxPartyId, ePhysical_art, lrt);
		if( pAuxEntry )
		{
		    tipPartyInfoReq.auxTipChannelDescr.unDspNum = pAuxEntry->unitId;
		    tipPartyInfoReq.auxTipChannelDescr.unPortNum = pAuxEntry->portId;
		}

	    ConnToCardTableEntry connToCardTableEntry(*pCenterEntry);
	    connToCardTableEntry.physicalRsrcType=ePhysical_res_none;
	    connToCardTableEntry.unitId=0;
	    SetPhysicalInfoHeader(&connToCardTableEntry);

	    m_MplMcmsProt.AddData( sizeof(mcTipMsgInfoReq),(const char*) &tipPartyInfoReq);
	}
	else
	{
	  PrintAssert(conf_id, party_id,m_MplMcmsProt.getCommonHeaderOpcode());
		status = STATUS_PROBLEMS_IN_SHARED_MEMORY;
	}
	return status;
}

STATUS CMplApiSpecialCommandHandler::PartyMonitoringReq(void* param)
{
	STATUS status = STATUS_OK;

  	int conf_id = m_MplMcmsProt.getPortDescriptionHeaderConf_id();

	TPartyMonitoringReq partyInfoReq;
    memcpy(&partyInfoReq, m_MplMcmsProt.getpData(), sizeof(TPartyMonitoringReq));
    int party_id = partyInfoReq.ulTipChannelPartyID;

    if( 0 == party_id )// only in case of TIP it will be equal to slave party id
    {
    	party_id = m_MplMcmsProt.getPortDescriptionHeaderParty_id();
    	partyInfoReq.ulTipChannelPartyID = ILEGAL_PARTY_ID;
    }
    eLogicalResourceTypes lrt = (eLogicalResourceTypes) m_MplMcmsProt.getPortDescriptionHeaderLogical_resource_type_1();

	TRACEINTO << "Sending monitoring request to conf_id: " << conf_id << " party_id: " << party_id << " lrt1: " << lrt;
	const ConnToCardTableEntry* pConnToCardTableEntry = GetPhysicalInfoHeaderByConfAndPartyID(conf_id, party_id, ePhysical_art, lrt);
	if(pConnToCardTableEntry!=NULL)
	{
		SetPhysicalInfoHeader(pConnToCardTableEntry);
	    m_MplMcmsProt.AddData( sizeof(TPartyMonitoringReq),(const char*) &partyInfoReq);
	}
	else
	{
	  PrintAssert(conf_id, party_id,m_MplMcmsProt.getCommonHeaderOpcode());
		status = STATUS_PROBLEMS_IN_SHARED_MEMORY;
	}
	return status;
}

STATUS CMplApiSpecialCommandHandler::TipKillObjectReq(void* param)
{
//	PTRACE(eLevelInfoNormal,"MplApiSpecialCommandHandler::TipKillObjectReq()");
	STATUS status = STATUS_OK;
//
//	int conf_id    = m_MplMcmsProt.getPortDescriptionHeaderConf_id();
//	int party_id   = m_MplMcmsProt.getPortDescriptionHeaderParty_id();
//
//	const ConnToCardTableEntry* pConnToCardTableEntry = GetPhysicalInfoHeaderByConfAndPartyID(conf_id,party_id,ePhysical_art);
//
//	if( pConnToCardTableEntry==NULL )
//	{
//		pConnToCardTableEntry=GetPhysicalInfoHeaderByConfAndPartyID(conf_id,party_id,ePhysical_art_light);
//	}
//
//	if( pConnToCardTableEntry!=NULL )
//	{
//		ConnToCardTableEntry connToCardTableEntry(*pConnToCardTableEntry);
//
//		mcTipKillObjectReq tipKillObjReq;
//
//		memcpy(&tipKillObjReq, m_MplMcmsProt.getpData(), sizeof(mcTipKillObjectReq));
//
//		connToCardTableEntry.physicalRsrcType=ePhysical_res_none;
//		connToCardTableEntry.unitId=0;
//		SetPhysicalInfoHeader(&connToCardTableEntry);
//
//		m_MplMcmsProt.AddData( sizeof(mcTipKillObjectReq),(const char*) &tipKillObjReq);
//	}
//	else
//	{
//		PrintAssert(conf_id, party_id);
//		status = STATUS_PROBLEMS_IN_SHARED_MEMORY;
//	}
	return status;
}

STATUS CMplApiSpecialCommandHandler::CreatePartyReq(void* param)
{
	STATUS status = STATUS_OK;
	m_MplMcmsProt.SetCommonHeaderOpcode(TB_MSG_OPEN_PORT_REQ);

  	int conf_id    = m_MplMcmsProt.getPortDescriptionHeaderConf_id();
	int party_id   = m_MplMcmsProt.getPortDescriptionHeaderParty_id();

	const ConnToCardTableEntry *pConnToCardTableEntry = GetMainARTphysicalInfoHeaderByConfAndPartyID(conf_id, party_id);

	if(pConnToCardTableEntry!=NULL)
	{
		SetPhysicalInfoHeader(pConnToCardTableEntry);
	}
	else
	{
	  PrintAssert(conf_id, party_id,m_MplMcmsProt.getCommonHeaderOpcode());
		status = STATUS_PROBLEMS_IN_SHARED_MEMORY;
	}

	return status;
}

STATUS CMplApiSpecialCommandHandler::ClosePortReq(void* param)
{
	if(m_MplMcmsProt.getPortDescriptionHeaderLogical_resource_type_1() == eLogical_audio_encoder)
	{
		m_MplMcmsProt.SetCommonHeaderOpcode(AUDIO_CLOSE_ENCODER_REQ);
	}
	if(m_MplMcmsProt.getPortDescriptionHeaderLogical_resource_type_2() == eLogical_audio_decoder)
	{
		m_MplMcmsProt.SetCommonHeaderOpcode(AUDIO_CLOSE_DECODER_REQ);
	}

	return STATUS_OK;
}

STATUS CMplApiSpecialCommandHandler::DeletePartyReq(void* param)
{
	STATUS status = STATUS_OK;

	 m_MplMcmsProt.SetCommonHeaderOpcode(TB_MSG_CLOSE_PORT_REQ);

	int conf_id   = m_MplMcmsProt.getPortDescriptionHeaderConf_id();
	int party_id  = m_MplMcmsProt.getPortDescriptionHeaderParty_id();

	const ConnToCardTableEntry *pConnToCardTableEntry = GetMainARTphysicalInfoHeaderByConfAndPartyID(conf_id, party_id);

	if(pConnToCardTableEntry!=NULL)
	{
		SetPhysicalInfoHeader(pConnToCardTableEntry);
	}
	else
	{
	  PrintAssert(conf_id, party_id,m_MplMcmsProt.getCommonHeaderOpcode());
		status = STATUS_PROBLEMS_IN_SHARED_MEMORY;
	}

	return status;
}

STATUS CMplApiSpecialCommandHandler::AckInd(void* param)
{
	STATUS status = m_MplMcmsProt.ValidateDataSize(	sizeof(ACK_IND_S));
	if(STATUS_OK != status)
	{
		return status;
	}

	ACK_IND_S mplAckInd;
	memcpy(&mplAckInd, m_MplMcmsProt.getpData(), sizeof(ACK_IND_S));
	eLogicalResourceTypes lrt = (eLogicalResourceTypes)m_MplMcmsProt.getPortDescriptionHeaderLogical_resource_type_1();
 	if ((m_MplMcmsProt.getPhysicalInfoHeaderResource_type() == ePhysical_art_light)||
		 (m_MplMcmsProt.getPhysicalInfoHeaderResource_type()== ePhysical_art))
	{
		switch(mplAckInd.ack_base.ack_opcode)
		{
		case TB_MSG_OPEN_PORT_REQ:
		{
			if(lrt!=eLogical_relay_avc_to_svc_rtp_with_audio_encoder &&
					lrt!=eLogical_relay_avc_to_svc_rtp )
			{
				mplAckInd.ack_base.ack_opcode = CONF_MPL_CREATE_PARTY_REQ;
				TRACEINTO<<"!@# logicalResourceType:"<<lrt;
			}


			 m_MplMcmsProt.AddData(sizeof(ACK_IND_S), (char*)&mplAckInd);
			 break;
		 }
		case TB_MSG_CLOSE_PORT_REQ:
		 {
				if(lrt!=eLogical_relay_avc_to_svc_rtp_with_audio_encoder &&
						lrt!=eLogical_relay_avc_to_svc_rtp )
				{
					mplAckInd.ack_base.ack_opcode = CONF_MPL_DELETE_PARTY_REQ;
					TRACEINTO<<"!@# logicalResourceType:"<<lrt;
				}


			 m_MplMcmsProt.AddData(sizeof(ACK_IND_S), (char*)&mplAckInd);
			 break;
		 }
		default:
		 {
			 break;
		 }
		}
	}

	return STATUS_OK;
}

STATUS CMplApiSpecialCommandHandler::TBStartPreviewReq(void* param)
{
	PTRACE(eLevelInfoNormal,"CMplApiSpecialCommandHandler::TBStartPreviewReq");
	STATUS status = m_MplMcmsProt.ValidateDataSize(	sizeof(mcReqCmStartPreviewChannel));
	if(STATUS_OK != status)
	{
		return status;
	}
	APIU32 connectionId = 0xffffffff;
	mcReqCmStartPreviewChannel tbMsgStart;
	memcpy(&tbMsgStart, m_MplMcmsProt.getpData(), sizeof(mcReqCmStartPreviewChannel));

	connectionId = m_MplMcmsProt.getPortDescriptionHeaderConnection_id();
	if(0xffffffff != connectionId)
	{
		ConnToCardTableEntry connToCardTableEntry;
		status = m_pMplApiProcess->GetSharedMemoryMap()->Get(connectionId, connToCardTableEntry);

		if (STATUS_OK == status)
		{
			connToCardTableEntry.unitId=0;
			SetPhysicalInfoHeader(&connToCardTableEntry);
		}

	}

	m_MplMcmsProt.AddData( sizeof(mcReqCmStartPreviewChannel),(const char*) &tbMsgStart);

	return status;

}

STATUS CMplApiSpecialCommandHandler::TBStopPreviewReq(void* param)
{

	PTRACE(eLevelInfoNormal,"CMplApiSpecialCommandHandler::TBStopPreviewReq");

	STATUS status = m_MplMcmsProt.ValidateDataSize(	sizeof(mcReqCmCloseUdpPort));
	if(STATUS_OK != status)
	{
		return status;
	}

	APIU32 connectionId = 0xffffffff;

	mcReqCmCloseUdpPort tbMsgStop;
	memcpy(&tbMsgStop, m_MplMcmsProt.getpData(), sizeof(mcReqCmCloseUdpPort));

	connectionId = m_MplMcmsProt.getPortDescriptionHeaderConnection_id();
	if(0xffffffff != connectionId)
	{
		ConnToCardTableEntry connToCardTableEntry;
		status = m_pMplApiProcess->GetSharedMemoryMap()->Get(connectionId, connToCardTableEntry);

		if (STATUS_OK == status)
		{
			connToCardTableEntry.unitId=0;
			SetPhysicalInfoHeader(&connToCardTableEntry);
		}

	}

	m_MplMcmsProt.AddData( sizeof(mcReqCmCloseUdpPort),(const char*) &tbMsgStop);

	return status;
}


STATUS CMplApiSpecialCommandHandler::TBConnectDisconnectReq(void* param)
{

	STATUS status = m_MplMcmsProt.ValidateDataSize(	sizeof(TB_MSG_CONNECT_S));
	if(STATUS_OK != status)
	{
		return status;
	}

	TB_MSG_CONNECT_S tbMsgConnect;
	memcpy(&tbMsgConnect, m_MplMcmsProt.getpData(), sizeof(TB_MSG_CONNECT_S));

	APIU32 ConnectionId1 = tbMsgConnect.physical_port1.connection_id;
	APIU32 ConnectionId2 = tbMsgConnect.physical_port2.connection_id;

	ConnToCardTableEntry connToCardTableEntry;
	if(0xffffffff != ConnectionId1)
   	{
   		status = m_pMplApiProcess->GetSharedMemoryMap()->Get(ConnectionId1, connToCardTableEntry);
	   	if (STATUS_OK == status)
	   	{
			SetPhysicalResourceInfo(&connToCardTableEntry,tbMsgConnect.physical_port1.physical_id);

		   	connToCardTableEntry.physicalRsrcType=ePhysical_res_none;
		   	connToCardTableEntry.unitId=0;
		   	SetPhysicalInfoHeader(&connToCardTableEntry);
	   	}

   	}

   	if(0xffffffff != ConnectionId2)
   	{
		status = m_pMplApiProcess->GetSharedMemoryMap()->Get(ConnectionId2, connToCardTableEntry);
	   	if (STATUS_OK == status)
		{
        	SetPhysicalResourceInfo(&connToCardTableEntry,tbMsgConnect.physical_port2.physical_id);
	   	}

   	}

	m_MplMcmsProt.AddData( sizeof(TB_MSG_CONNECT_S),(const char*) &tbMsgConnect);

	return status;
}

STATUS CMplApiSpecialCommandHandler::H323CMUpdateOpenUDPAddrReq(void* param)
{
	STATUS status = m_MplMcmsProt.ValidateDataSize(	sizeof(TOpenUdpPortOrUpdateUdpAddrMessageStruct));
	if(STATUS_OK != status)
	{
		return status;
	}

	TOpenUdpPortOrUpdateUdpAddrMessageStruct tbMsgConnect;
	memcpy(&tbMsgConnect, m_MplMcmsProt.getpData(), sizeof(TOpenUdpPortOrUpdateUdpAddrMessageStruct));

	APIU32 ConnectionId1 = tbMsgConnect.physicalPort.connection_id;

   	if (0xffffffff != ConnectionId1)
   	{
	  ConnToCardTableEntry connToCardTableEntry;
	  status = m_pMplApiProcess->GetSharedMemoryMap()->Get(ConnectionId1, connToCardTableEntry);
	  if (STATUS_OK == status)
	    {
	      SetPhysicalResourceInfo(&connToCardTableEntry,tbMsgConnect.physicalPort.physical_id);

	      //Identify the PSTN calls
	      PORT_DESCRIPTION_HEADER_S		rPortDescH;
	      m_MplMcmsProt.GetPortDescHeaderCopy(&rPortDescH);
	      if (eLogical_net == rPortDescH.logical_resource_type_1 )
	      {
		        SetPSTNPhysicalInfoHeader(connToCardTableEntry);
				SetPSTNContent(connToCardTableEntry,tbMsgConnect);
	      }
	      else
	      {
			  //For IP only
			  connToCardTableEntry.physicalRsrcType=ePhysical_res_none;
			  connToCardTableEntry.unitId=0;
			  SetPhysicalInfoHeader(&connToCardTableEntry);
		}


	    }
	}

   	m_MplMcmsProt.AddData( sizeof(TOpenUdpPortOrUpdateUdpAddrMessageStruct),(const char*) &tbMsgConnect);

	return status;
}

STATUS CMplApiSpecialCommandHandler::H323CMCloseUDPPortReq(void* param)
{
	STATUS status = m_MplMcmsProt.ValidateDataSize(	sizeof(TCloseUdpPortMessageStruct));
	if(STATUS_OK != status)
	{
		return status;
	}

	TCloseUdpPortMessageStruct tbMsgConnect;
	memcpy(&tbMsgConnect, m_MplMcmsProt.getpData(), sizeof(TCloseUdpPortMessageStruct));

	APIU32 ConnectionId1 = tbMsgConnect.physicalPort.connection_id;
   	if (0xffffffff != ConnectionId1)
   	{
   		ConnToCardTableEntry connToCardTableEntry;
   		status = m_pMplApiProcess->GetSharedMemoryMap()->Get(ConnectionId1, connToCardTableEntry);
	   	if (STATUS_OK == status)
	   	{
		  SetPhysicalResourceInfo(&connToCardTableEntry, tbMsgConnect.physicalPort.physical_id,true);

		  // Identify the PSTN calls
		  PORT_DESCRIPTION_HEADER_S		rPortDescH;
		  m_MplMcmsProt.GetPortDescHeaderCopy(&rPortDescH);
		  if (eLogical_net != rPortDescH.logical_resource_type_1 ) //Ip/SIP calls
		    {
		      connToCardTableEntry.physicalRsrcType=ePhysical_res_none;
		      connToCardTableEntry.unitId=0;
		      SetPhysicalInfoHeader(&connToCardTableEntry);
		    }
		  else //PSTN Call
		    {
		      SetPSTNPhysicalInfoHeader(connToCardTableEntry);
		      SetPSTNContent(connToCardTableEntry,tbMsgConnect);
		    }
	   	}
   	}

	m_MplMcmsProt.AddData( sizeof(TCloseUdpPortMessageStruct),(const char*) &tbMsgConnect);

	return status;
}


STATUS CMplApiSpecialCommandHandler::VideoEncoderChangeLayoutReq(void* param)
{

/*	//Removed since MCMS_CM_CHANGE_LAYOUT_S is now a dynamic structure
	STATUS status = m_MplMcmsProt.ValidateDataSize(	sizeof(MCMS_CM_CHANGE_LAYOUT_S));
	if(STATUS_OK != status)
	{
		return status;
	}
*/
	STATUS status = STATUS_OK;

	MCMS_CM_CHANGE_LAYOUT_S* pChangeLayout = new MCMS_CM_CHANGE_LAYOUT_S;

   	memcpy(pChangeLayout, m_MplMcmsProt.getpData(),sizeof(MCMS_CM_CHANGE_LAYOUT_S));

   	WORD numOfImages = CLibsCommonHelperFuncs::GetNumbSubImg(pChangeLayout->nLayoutType);
   	pChangeLayout->atImageParam = new MCMS_CM_IMAGE_PARAM_S[numOfImages];

   	int sizeOfImageParamArray = numOfImages*(sizeof(MCMS_CM_IMAGE_PARAM_S));
   	int sizeOfChangeLayoutWithoutImages = sizeof(MCMS_CM_CHANGE_LAYOUT_S)-sizeof(MCMS_CM_IMAGE_PARAM_S*);
   	int sizeOfChangeLayoutStruct = sizeOfChangeLayoutWithoutImages + sizeOfImageParamArray;

   	memcpy(&(pChangeLayout->atImageParam[0]), m_MplMcmsProt.getpData()+sizeOfChangeLayoutWithoutImages, sizeOfImageParamArray);

   	for (int i = 0 ; i < numOfImages ; i++)
	{
		APIU32 ConnectionId = pChangeLayout->atImageParam[i].tDecoderPhysicalId.connection_id;
		if (0xffffffff != ConnectionId)
		{
			ConnToCardTableEntry connToCardTableEntry;

			status = m_pMplApiProcess->GetSharedMemoryMap()->Get(ConnectionId, connToCardTableEntry);

			if (STATUS_OK == status)
			{
				SetPhysicalResourceInfo(&connToCardTableEntry, pChangeLayout->atImageParam[i].tDecoderPhysicalId.physical_id);
			}
		}
	}


   	ALLOCBUFFER(tmp,sizeOfChangeLayoutStruct);
   	memcpy(tmp, pChangeLayout, sizeOfChangeLayoutWithoutImages);
   	memcpy(tmp+sizeOfChangeLayoutWithoutImages, &(pChangeLayout->atImageParam[0]), sizeOfImageParamArray);

   	m_MplMcmsProt.AddData( sizeOfChangeLayoutStruct,(const char*) tmp);


	DEALLOCBUFFER(tmp);
	PDELETEA(pChangeLayout->atImageParam);
	POBJDELETE(pChangeLayout);
	return status;
}


STATUS CMplApiSpecialCommandHandler::VideoUpdateDecoderResolutionReq(void* param)
{
	STATUS status = m_MplMcmsProt.ValidateDataSize(	sizeof(UPDATE_DECODER_RESOLUTION_S));
	if(STATUS_OK != status)
	{
		return status;
	}

	UPDATE_DECODER_RESOLUTION_S tUpdateDecoderResolution;
   	memcpy(&tUpdateDecoderResolution, m_MplMcmsProt.getpData(), sizeof(UPDATE_DECODER_RESOLUTION_S));

   	APIU32 ConnectionId = tUpdateDecoderResolution.tDecoderPhysicalId.connection_id;

   	if (0xffffffff != ConnectionId)
	{
		ConnToCardTableEntry connToCardTableEntry;
		status = m_pMplApiProcess->GetSharedMemoryMap()->Get(ConnectionId, connToCardTableEntry);
		if (STATUS_OK == status)
		{
			SetPhysicalResourceInfo(&connToCardTableEntry, tUpdateDecoderResolution.tDecoderPhysicalId.physical_id);
		}
	}


	m_MplMcmsProt.AddData( sizeof(UPDATE_DECODER_RESOLUTION_S),(const char*) &tUpdateDecoderResolution);

	return status;
}
// AC_OPEN_CONF_REQ, AC_CLOSE_CONF_REQ, AC_UPDATE_CONF_PARAMS_REQ
STATUS CMplApiSpecialCommandHandler::ACRequest(void* param)
{
	STATUS status = FillPhysicalHeaderByResourceTypeCntrlType(ePhysical_audio_controller,
                                                              (ECntrlType)(m_MplMcmsProt.GetPhysicalHeaderConst().future_use1));
	m_MplMcmsProt.SetPortDescriptorHeaderLogicRsrcType1(eLogical_audio_controller);

	return status;
}
// resend OPEN_CONF to new slave when inserting a new card
STATUS CMplApiSpecialCommandHandler::ACShadowRequest(void* param)
{
	STATUS status = FillPhysicalHeaderByResourceTypeCntrlType(ePhysical_audio_controller, E_AC_SLAVE);
	m_MplMcmsProt.SetPortDescriptorHeaderLogicRsrcType1(eLogical_audio_controller);
	m_MplMcmsProt.SetCommonHeaderOpcode(AC_OPEN_CONF_REQ);

    return status;
}

STATUS CMplApiSpecialCommandHandler::IVRRequest(void* param)
{
	STATUS status = STATUS_OK;
	if(m_MplMcmsProt.getPortDescriptionHeaderParty_id()==INVALID)//conf level IVR Request sent to Master IVR Controller
	{
		status = FillPhysicalHeaderByResourceType(ePhysical_ivr_controller);
		m_MplMcmsProt.SetPortDescriptorHeaderLogicRsrcType1(eLogical_ivr_controller);
	}
	else //party level IVR Request sent to IVR Controller on party ART card - suited for both video (show slide) and audio (play msg) requests
	{
		int conf_id    = m_MplMcmsProt.getPortDescriptionHeaderConf_id();
		int party_id   = m_MplMcmsProt.getPortDescriptionHeaderParty_id();

		const ConnToCardTableEntry *pConnToCardTableEntry = GetMainARTphysicalInfoHeaderByConfAndPartyID(conf_id, party_id);

		if(pConnToCardTableEntry!=NULL)
		{
			ConnToCardTableEntry connToCardTableEntry(*pConnToCardTableEntry);

			//fill in beginning of msg with physical of ART Port
			PHYSICAL_RESOURCE_INFO_S physicalRsrc;
			memset(&physicalRsrc, 0, sizeof(PHYSICAL_RESOURCE_INFO_S));
			physicalRsrc.physical_unit_params.box_id = connToCardTableEntry.boxId;
			physicalRsrc.physical_unit_params.board_id = connToCardTableEntry.boardId;
			physicalRsrc.physical_unit_params.sub_board_id = connToCardTableEntry.subBoardId;
			physicalRsrc.physical_unit_params.unit_id = connToCardTableEntry.unitId;
			physicalRsrc.accelerator_id = connToCardTableEntry.acceleratorId;
			physicalRsrc.port_id = connToCardTableEntry.portId;
			physicalRsrc.resource_type = connToCardTableEntry.physicalRsrcType;

			if(NULL == m_MplMcmsProt.getpData()) //IVR Message with no content besides the physical resource such as STOP_SHOW_SLIDE
			{
				m_MplMcmsProt.AddData(sizeof(PHYSICAL_RESOURCE_INFO_S),(const char*)&physicalRsrc);
			}
			else //IVR Message with content - we will only overwrite the beggining of content with physical resource struct
			{
				memcpy(m_MplMcmsProt.getpData(), &physicalRsrc,sizeof(PHYSICAL_RESOURCE_INFO_S));
			}

			//fill in physical header with Card Manager on ART Card
			connToCardTableEntry.physicalRsrcType 	= ePhysical_res_none;
			connToCardTableEntry.unitId				= 0;
			SetPhysicalInfoHeader(&connToCardTableEntry);
		}
		else
		{
		  PrintAssert(conf_id, party_id,m_MplMcmsProt.getCommonHeaderOpcode());
			status = STATUS_PROBLEMS_IN_SHARED_MEMORY;
		}
	}
	return status;
}

STATUS CMplApiSpecialCommandHandler::MoveRsrcArt(void* param)
{
	eLogicalResourceTypes lrt = (eLogicalResourceTypes)(int)param;
	TRACEINTO << "logicalResourceType = " << ::LogicalResourceTypeToString(lrt);
	STATUS status = FillPhysicalHeaderByPartyIdResourceType(ePhysical_art, lrt);

	return status;
}

STATUS CMplApiSpecialCommandHandler::MoveRsrcNet(void* param)
{
	STATUS status = FillPhysicalHeaderByPartyIdResourceType(ePhysical_rtm);

	return status;
}

STATUS CMplApiSpecialCommandHandler::MoveRsrcVideoEnc(void* param)
{
	eLogicalResourceTypes lrt = (eLogicalResourceTypes)(int)param;
	TRACEINTO << "logicalResourceType = " << ::LogicalResourceTypeToString(lrt);
	STATUS status = FillPhysicalHeaderByPartyIdResourceType(ePhysical_video_encoder, lrt);

	return status;
}

STATUS CMplApiSpecialCommandHandler::MoveRsrcVideoDec(void* param)
{
	STATUS status = FillPhysicalHeaderByPartyIdResourceType(ePhysical_video_decoder);

	return status;
}

STATUS CMplApiSpecialCommandHandler::MoveRsrcMrmp(void* param)
{
	STATUS status = FillPhysicalHeaderByPartyIdResourceType(ePhysical_mrmp);

	return status;
}

STATUS CMplApiSpecialCommandHandler::NetSetupReq(void* param)
{
  NET_SETUP_REQ_S reqStruct;
  DWORD structSize = sizeof(NET_SETUP_REQ_S);
  return SetNetReqData(structSize,(BYTE *)(&reqStruct), false, true);
}

STATUS CMplApiSpecialCommandHandler::NetClearReq(void* param)
{
  NET_CLEAR_REQ_S reqStruct;
  DWORD structSize = sizeof(NET_CLEAR_REQ_S);
  return SetNetReqData(structSize,(BYTE *)(&reqStruct),true);
}
STATUS CMplApiSpecialCommandHandler::NetDisconnectAckReq(void* param)
{
  NET_DISCONNECT_ACK_REQ_S reqStruct;
  DWORD structSize = sizeof(NET_DISCONNECT_ACK_REQ_S);
  return SetNetReqData(structSize,(BYTE *)(&reqStruct),true);
}

STATUS CMplApiSpecialCommandHandler::NetAlertReq(void* param)
{
  NET_ALERT_REQ_S reqStruct;
  DWORD structSize = sizeof(NET_ALERT_REQ_S);
  return SetNetReqData(structSize,(BYTE *)(&reqStruct), false, false, true);
}

STATUS CMplApiSpecialCommandHandler::NetConnectReq(void* param)
{
  NET_CONNECT_REQ_S reqStruct;
  DWORD structSize = sizeof(NET_CONNECT_REQ_S);
  return SetNetReqData(structSize,(BYTE *)(&reqStruct), false, false, true);
}

//////////////////////////////////////////////////////////////////////////////////
STATUS CMplApiSpecialCommandHandler::SetNetReqData(DWORD structSize, BYTE* pStruct,
												   bool isDescriptorCanBeMissing, bool isNewSetupHeader,bool isVirtualPortFromChannelid)
{
  STATUS status = STATUS_OK;
  int conf_id    = m_MplMcmsProt.getPortDescriptionHeaderConf_id();
  int party_id   = m_MplMcmsProt.getPortDescriptionHeaderParty_id();
  eResourceTypes resourcePhysicalType=ePhysical_rtm;

  //const ConnToCardTableEntry* pConnToCardTableEntry = GetPhysicalInfoHeaderByConfAndPartyID(conf_id,party_id,resourcePhysicalType);

  DWORD connection_id   = m_MplMcmsProt.getPortDescriptionHeaderConnection_id();
  const ConnToCardTableEntry* pConnToCardTableEntry = GetPhysicalInfoHeaderByConnectionID(connection_id);


  if(pConnToCardTableEntry==NULL )
    {
      //In disconnection from the Lobby when we did not create the party
      //we are filling the info in the ConfParty
      if (isDescriptorCanBeMissing)
	{
	  TRACESTR (eLevelInfoNormal)<<"MplApiSpecialCommandHandler::SetNetReqData Descriptor is missing probably Lobby disconnection of party:" <<party_id ;
	  return STATUS_OK;
	}
      else //Problem no record found
	{
	  PrintAssert(conf_id, party_id,m_MplMcmsProt.getCommonHeaderOpcode());
	  return STATUS_PROBLEMS_IN_SHARED_MEMORY;
	}
    }

  //MplApi fill PHYSICAL_INFO_HEADER_S from ConnToCardTable
  SetPhysicalInfoHeader(pConnToCardTableEntry);

  //MplApi fill following fields of NET_COMMON_PARAM_S
  memcpy(pStruct ,m_MplMcmsProt.getpData(), structSize);

  if (isNewSetupHeader) { //olga

	  NET_SETUP_REQ_HEADER_S*  pNetSetupHeader = (NET_SETUP_REQ_HEADER_S*)(pStruct);
	  pNetSetupHeader->spans_order[0]=m_MplMcmsProt.getPhysicalInfoHeaderUnit_id();
	  pNetSetupHeader->virtual_port_number = pConnToCardTableEntry->channelId;//=m_MplMcmsProt.getPhysicalInfoHeaderPort_id();
  }
  else {
	  NET_COMMON_PARAM_S * pNetCommonParams = (NET_COMMON_PARAM_S *)(pStruct);
	  pNetCommonParams->span_id=m_MplMcmsProt.getPhysicalInfoHeaderUnit_id();
      if(isVirtualPortFromChannelid){
          pNetCommonParams->virtual_port_number=pConnToCardTableEntry->channelId;
      }
      else{
          pNetCommonParams->virtual_port_number=m_MplMcmsProt.getPhysicalInfoHeaderPort_id();
      }

  }
  //MplApi change values of unit_id, accelerator_id and port_id fields of PHYSICAL_INFO_HEADER_S
  m_MplMcmsProt.m_MplMcmsPhysicalInfoHeader->unit_id = 0;
  m_MplMcmsProt.m_MplMcmsPhysicalInfoHeader->accelerator_id = 0;
  m_MplMcmsProt.m_MplMcmsPhysicalInfoHeader->port_id = 0xFFFF;

  //Add the updated data
  m_MplMcmsProt.AddData(structSize,(const char*)pStruct);

  return status;
}
//////////////////////////////////////////////////////////////////////////////////
STATUS CMplApiSpecialCommandHandler::TBConnectDisconnectPCMReq(void* param)
{
	STATUS status = m_MplMcmsProt.ValidateDataSize(	sizeof(TB_MSG_CONNECT_PCM_S));
	if(STATUS_OK != status)
	{
		return status;
	}

	TB_MSG_CONNECT_PCM_S tbMsgConnectPcm;
	memcpy(&tbMsgConnectPcm, m_MplMcmsProt.getpData(), sizeof(TB_MSG_CONNECT_PCM_S));

	DWORD encoderConnectionId = tbMsgConnectPcm.physical_port1.connection_id;

	ConnToCardTableEntry encoderConnToCardTableEntry;
	if(0xffffffff != encoderConnectionId)
   	{
   		status = m_pMplApiProcess->GetSharedMemoryMap()->Get(encoderConnectionId, encoderConnToCardTableEntry);
	   	if (STATUS_OK == status)
	   	{
			// fill the encoder params from shared memory inside the struct
	   		SetPhysicalResourceInfo(&encoderConnToCardTableEntry,tbMsgConnectPcm.physical_port1.physical_id);
	   		// in cop we send the connect req to CM (unit id = 0)
	   		if (encoderConnToCardTableEntry.rsrcType == eLogical_COP_PCM_encoder)
	   		{
	   			encoderConnToCardTableEntry.physicalRsrcType=ePhysical_res_none;
	   		    encoderConnToCardTableEntry.unitId=0;
	   		    SetPhysicalInfoHeader(&encoderConnToCardTableEntry);
	   		 }
	   		// in cop we send the connect req to CM (unit id = 0)
	   		if (encoderConnToCardTableEntry.rsrcType == eLogical_COP_PCM_encoder)
	   		{
	   			encoderConnToCardTableEntry.physicalRsrcType=ePhysical_res_none;
	   			encoderConnToCardTableEntry.unitId=0;
	   			SetPhysicalInfoHeader(&encoderConnToCardTableEntry);
	   		}
	   	}
   	}


	m_MplMcmsProt.AddData( sizeof(TB_MSG_CONNECT_PCM_S),(const char*) &tbMsgConnectPcm);

	return status;
}

//////////////////////////////////////////////////////////////////////////////////
STATUS CMplApiSpecialCommandHandler::PCMMessage(void* param)
{
	STATUS status = STATUS_OK;
	const ConnToCardTableEntry *connToCardTableEntry = GetPhysicalInfoHeaderByLogicalRsrcType(eLogical_COP_PCM_encoder);
	if (NULL != connToCardTableEntry)
	{
		SetPhysicalInfoHeader(connToCardTableEntry);
		// MplApi change values of unit_id and resource_type fields of PHYSICAL_INFO_HEADER_S
		// in order to dispatch message to Card Manager and not to a specified unit
		m_MplMcmsProt.m_MplMcmsPhysicalInfoHeader->unit_id = 0;
		m_MplMcmsProt.m_MplMcmsPhysicalInfoHeader->resource_type = ePhysical_res_none;
		//m_MplMcmsProt.m_MplMcmsPhysicalInfoHeader->port_id = 0xFFFF;//??
	}
	else
	{
		PrintAssert(eLogical_COP_PCM_encoder);
		status = STATUS_PROBLEMS_IN_SHARED_MEMORY;
	}

	return status;
}
//////////////////////////////////////////////////////////////////////////////////
STATUS CMplApiSpecialCommandHandler::FillPhysicalHeaderByResourceType(eResourceTypes type)
{
	STATUS status = STATUS_OK;

	const ConnToCardTableEntry *connToCardTableEntry = GetPhysicalInfoHeaderByRsrcType(type);
	if (NULL != connToCardTableEntry)
	{
		SetPhysicalInfoHeader(connToCardTableEntry);
	}
	else
	{
		PrintAssert(type);
		status = STATUS_PROBLEMS_IN_SHARED_MEMORY;
	}

	return status;
}

//////////////////////////////////////////////////////////////////////////////////
STATUS CMplApiSpecialCommandHandler::FillPhysicalHeaderByResourceTypeCntrlType(eResourceTypes resourceType, ECntrlType cntrlType)
{
    STATUS status = STATUS_OK;
	const ConnToCardTableEntry *connToCardTableEntry = GetPhysicalInfoHeaderByRsrcTypeCntrltype(resourceType, cntrlType);
	if (NULL != connToCardTableEntry)
	{
		SetPhysicalInfoHeader(connToCardTableEntry);
	}
	else
	{
//		PrintAssert(resourceType, cntrlType, m_MplMcmsProt.getCommonHeaderOpcode());
		status = STATUS_PROBLEMS_IN_SHARED_MEMORY;
	}

	return status;
}

//////////////////////////////////////////////////////////////////////////////////
STATUS CMplApiSpecialCommandHandler::FillPhysicalHeaderByPartyIdResourceType(eResourceTypes type, eLogicalResourceTypes lrt)
{
	STATUS status = STATUS_OK;
	DWORD partyId = m_MplMcmsProt.getPortDescriptionHeaderParty_id();

	const ConnToCardTableEntry *connToCardTableEntry = GetPhysicalInfoHeaderByPartyIDRsrcType(partyId, type, lrt);
	if (NULL != connToCardTableEntry)
	{
		SetPhysicalInfoHeader(connToCardTableEntry);
	}
	else
	{
		PrintAssert(partyId, type);
		status = STATUS_PROBLEMS_IN_SHARED_MEMORY;
	}

	return status;
}

//////////////////////////////////////////////////////////////////////////////////
const ConnToCardTableEntry* CMplApiSpecialCommandHandler::GetPhysicalInfoHeaderByRsrcType(eResourceTypes rsrcType)
{
	DWORD passedEntries = 0;
	DWORD i = 0;
	CSharedMemMap *pSharedMemoryMap = m_pMplApiProcess->GetSharedMemoryMap();
	while (i < pSharedMemoryMap->m_pHeader->m_maxEntries && passedEntries < pSharedMemoryMap->m_pHeader->m_numEntries)
	{
		if (pSharedMemoryMap->m_pEntries[i].m_id != EMPTY_ENTRY)
		{
			passedEntries++;
            const ConnToCardTableEntry & currentEntry = pSharedMemoryMap->m_pEntries[i];

			if (currentEntry.physicalRsrcType == rsrcType)
            {
	      pSharedMemoryMap->m_pEntries[i].DumpRaw("CMplApiSpecialCommandHandler::GetPhysicalInfoHeaderByRsrcType");
                return &(pSharedMemoryMap->m_pEntries[i]);
            }
		}
		i++;
	}
	PTRACE2COND(SHARED_MEMORY_DEBUG_PRINTS,eLevelInfoNormal,"CMplApiSpecialCommandHandler::GetPhysicalInfoHeaderByRsrcType"," - enty not found");
	return NULL;
}
//////////////////////////////////////////////////////////////////////////////////
const ConnToCardTableEntry* CMplApiSpecialCommandHandler::GetPhysicalInfoHeaderByRsrcTypeAndBoardId(eResourceTypes rsrcType)
{
	DWORD passedEntries = 0;
	DWORD i = 0;
	CSharedMemMap *pSharedMemoryMap = m_pMplApiProcess->GetSharedMemoryMap();
	while (i < pSharedMemoryMap->m_pHeader->m_maxEntries && passedEntries < pSharedMemoryMap->m_pHeader->m_numEntries)
	{
		if (pSharedMemoryMap->m_pEntries[i].m_id != EMPTY_ENTRY)
		{
			passedEntries++;
            const ConnToCardTableEntry & currentEntry = pSharedMemoryMap->m_pEntries[i];

			if (currentEntry.physicalRsrcType == rsrcType && m_MplMcmsProt.m_MplMcmsPhysicalInfoHeader->board_id == currentEntry.boardId)
            {
	      pSharedMemoryMap->m_pEntries[i].DumpRaw("CMplApiSpecialCommandHandler::GetPhysicalInfoHeaderByRsrcTypeAndBoardId");
                return &(pSharedMemoryMap->m_pEntries[i]);
            }
		}
		i++;
	}
	PTRACE2COND(SHARED_MEMORY_DEBUG_PRINTS,eLevelInfoNormal,"CMplApiSpecialCommandHandler::GetPhysicalInfoHeaderByRsrcTypeAndBoardId"," - enty not found");
	return NULL;
}
//////////////////////////////////////////////////////////////////////////////////
const ConnToCardTableEntry* CMplApiSpecialCommandHandler::GetPhysicalInfoHeaderByRsrcTypeCntrltype(eResourceTypes rsrcType, ECntrlType cntrlType)
{
    DWORD passedEntries = 0;
	DWORD i = 0;
	CSharedMemMap *pSharedMemoryMap = m_pMplApiProcess->GetSharedMemoryMap();
	while (i < pSharedMemoryMap->m_pHeader->m_maxEntries && passedEntries < pSharedMemoryMap->m_pHeader->m_numEntries)
	{
		if (pSharedMemoryMap->m_pEntries[i].m_id != EMPTY_ENTRY)
		{
			passedEntries++;
			const ConnToCardTableEntry & currentEntry = pSharedMemoryMap->m_pEntries[i];

			if (currentEntry.physicalRsrcType == rsrcType && currentEntry.rsrcCntlType == cntrlType)
            {
	      pSharedMemoryMap->m_pEntries[i].DumpRaw("CMplApiSpecialCommandHandler::GetPhysicalInfoHeaderByRsrcTypeCntrltype");
                return &(pSharedMemoryMap->m_pEntries[i]);
            }
		}
		i++;
	}
	PTRACE2COND(SHARED_MEMORY_DEBUG_PRINTS,eLevelInfoNormal,"CMplApiSpecialCommandHandler::GetPhysicalInfoHeaderByRsrcTypeCntrltype"," - enty not found");
	return NULL;
}

//////////////////////////////////////////////////////////////////////////////////
// for spread audio controller messages to all slaves
// Amos AC
// input: resourceType(for example ePhysical_audio_controller), searchByCntrlType (0 - search by resource type only), cntrlType (for example E_AC_SLAVE),
// entriesFound - array of const ConnToCardTableEntry for results (NULL initiated), max_entries_to_search - results array size
// return: return value - number of entries actually found, pointers set in entriesFound[]

DWORD CMplApiSpecialCommandHandler::GetPhysicalInfoHeaderByRsrcTypeCntrltype(eResourceTypes resourceType, BYTE searchByCntrlType, ECntrlType cntrlType, ConnToCardTableEntry** entriesFound,DWORD max_entries_to_search)
{
  DWORD num_entries_found = 0;
  DWORD entries_iterator = 0;
  DWORD non_empty_entries_iterator = 0;

  CSharedMemMap *pSharedMemoryMap = m_pMplApiProcess->GetSharedMemoryMap();
  while (entries_iterator < pSharedMemoryMap->m_pHeader->m_maxEntries && non_empty_entries_iterator < pSharedMemoryMap->m_pHeader->m_numEntries && num_entries_found<max_entries_to_search)
    {
      if (pSharedMemoryMap->m_pEntries[entries_iterator].m_id != EMPTY_ENTRY)
	{
	  non_empty_entries_iterator++;
	  const ConnToCardTableEntry & currentEntry = pSharedMemoryMap->m_pEntries[entries_iterator];

	  if (currentEntry.physicalRsrcType == resourceType && (searchByCntrlType==0 || currentEntry.rsrcCntlType == cntrlType) )
            {
	      entriesFound[num_entries_found] =  &(pSharedMemoryMap->m_pEntries[entries_iterator]);
	      num_entries_found++;
	      pSharedMemoryMap->m_pEntries[entries_iterator].DumpRaw("CMplApiSpecialCommandHandler::GetPhysicalInfoHeaderByRsrcTypeCntrltype");
            }
	}
      entries_iterator++;
    }
  PTRACE2INTCOND(SHARED_MEMORY_DEBUG_PRINTS,eLevelInfoNormal,"CMplApiSpecialCommandHandler::GetPhysicalInfoHeaderByRsrcTypeCntrltype, num_entries_found = ",num_entries_found);
  return num_entries_found;
}
//////////////////////////////////////////////////////////////////////////////////
const ConnToCardTableEntry* CMplApiSpecialCommandHandler::GetPhysicalInfoHeaderByConnectionID(DWORD connection_id)
{

	DWORD passedEntries = 0;
	DWORD i = 0;
	CSharedMemMap *pSharedMemoryMap = m_pMplApiProcess->GetSharedMemoryMap();
	while (i < pSharedMemoryMap->m_pHeader->m_maxEntries && passedEntries < pSharedMemoryMap->m_pHeader->m_numEntries)
	{
		if (pSharedMemoryMap->m_pEntries[i].m_id != EMPTY_ENTRY)
		{
			passedEntries++;

			if (pSharedMemoryMap->m_pEntries[i].m_id == connection_id)
				{
				  pSharedMemoryMap->m_pEntries[i].DumpRaw("CMplApiSpecialCommandHandler::GetPhysicalInfoHeaderByConnectionID");
					return &(pSharedMemoryMap->m_pEntries[i]);
				}
		}
		i++;
	}
	PTRACE2COND(SHARED_MEMORY_DEBUG_PRINTS,eLevelInfoNormal,"CMplApiSpecialCommandHandler::GetPhysicalInfoHeaderByConnectionID"," - enty not found");
	return NULL;
}
//////////////////////////////////////////////////////////////////////////////////
const ConnToCardTableEntry* CMplApiSpecialCommandHandler::GetPhysicalInfoHeaderByPartyIDRsrcType(DWORD party_id, eResourceTypes rsrcType, eLogicalResourceTypes lrt)
{
	DWORD passedEntries = 0;
	DWORD i = 0;
	CSharedMemMap *pSharedMemoryMap = m_pMplApiProcess->GetSharedMemoryMap();
	while (i < pSharedMemoryMap->m_pHeader->m_maxEntries && passedEntries < pSharedMemoryMap->m_pHeader->m_numEntries)
	{
		if (pSharedMemoryMap->m_pEntries[i].m_id != EMPTY_ENTRY)
		{
			passedEntries++;
            const ConnToCardTableEntry & currentEntry = pSharedMemoryMap->m_pEntries[i];

			if ((currentEntry.rsrc_party_id 	== party_id) &&
				(currentEntry.physicalRsrcType  == rsrcType) &&
				(eLogical_res_none == lrt || lrt == currentEntry.rsrcType))
			{
				pSharedMemoryMap->m_pEntries[i].DumpRaw("CMplApiSpecialCommandHandler::GetPhysicalInfoHeaderByPartyIDRsrcType");
				return &(pSharedMemoryMap->m_pEntries[i]);
			}
		}
		i++;
	}
	PTRACE2COND(SHARED_MEMORY_DEBUG_PRINTS,eLevelInfoNormal,"CMplApiSpecialCommandHandler::GetPhysicalInfoHeaderByPartyIDRsrcType"," - enty not found");
	return NULL;
}

//////////////////////////////////////////////////////////////////////////////////
const ConnToCardTableEntry* CMplApiSpecialCommandHandler::GetPhysicalInfoHeaderByConfAndPartyID(DWORD conf_id,DWORD party_id, eResourceTypes rsrcType, eLogicalResourceTypes lrt)
{

	DWORD passedEntries = 0;
	DWORD i = 0;
	CSharedMemMap *pSharedMemoryMap = m_pMplApiProcess->GetSharedMemoryMap();
	while (i < pSharedMemoryMap->m_pHeader->m_maxEntries &&
		passedEntries < pSharedMemoryMap->m_pHeader->m_numEntries)
	{
		if (pSharedMemoryMap->m_pEntries[i].m_id != EMPTY_ENTRY)
		{
			passedEntries++;
            const ConnToCardTableEntry & currentEntry = pSharedMemoryMap->m_pEntries[i];

			if ((currentEntry.rsrc_conf_id 	   == conf_id)	&&
				(currentEntry.rsrc_party_id    == party_id) &&
				(currentEntry.physicalRsrcType == rsrcType) &&
				(eLogical_res_none == lrt || lrt == currentEntry.rsrcType))
			{
				pSharedMemoryMap->m_pEntries[i].DumpRaw("CMplApiSpecialCommandHandler::GetPhysicalInfoHeaderByConfAndPartyID");
				return &(pSharedMemoryMap->m_pEntries[i]);
			}
		}
		i++;
	}
	PTRACE2COND(SHARED_MEMORY_DEBUG_PRINTS,eLevelInfoNormal,"CMplApiSpecialCommandHandler::GetPhysicalInfoHeaderByConfAndPartyID"," - enty not found");
	return NULL;
}
//////////////////////////////////////////////////////////////////////////////////
const ConnToCardTableEntry* CMplApiSpecialCommandHandler::GetPhysicalInfoHeaderByLogicalRsrcType(eLogicalResourceTypes logicalResourceType)
{
	DWORD passedEntries = 0;
	DWORD i = 0;
	CSharedMemMap *pSharedMemoryMap = m_pMplApiProcess->GetSharedMemoryMap();
	while (i < pSharedMemoryMap->m_pHeader->m_maxEntries && passedEntries < pSharedMemoryMap->m_pHeader->m_numEntries)
	{
		if (pSharedMemoryMap->m_pEntries[i].m_id != EMPTY_ENTRY)
		{
			passedEntries++;
			const ConnToCardTableEntry& currentEntry = pSharedMemoryMap->m_pEntries[i];

			if (currentEntry.rsrcType== logicalResourceType)
	        {
		  pSharedMemoryMap->m_pEntries[i].DumpRaw("CMplApiSpecialCommandHandler::GetPhysicalInfoHeaderByLogicalRsrcType");
				return &(pSharedMemoryMap->m_pEntries[i]);
	        }
		}
		i++;
	}
	PTRACE2COND(SHARED_MEMORY_DEBUG_PRINTS,eLevelInfoNormal,"CMplApiSpecialCommandHandler::GetPhysicalInfoHeaderByLogicalRsrcType"," - enty not found");
	return NULL;
}
//////////////////////////////////////////////////////////////////////////////////
const ConnToCardTableEntry* CMplApiSpecialCommandHandler::GetPhysicalInfoHeaderByConfAndLogicalRsrcType(DWORD conf_id, eLogicalResourceTypes logicalResourceType)
{
	DWORD passedEntries = 0;
	DWORD i = 0;
	CSharedMemMap *pSharedMemoryMap = m_pMplApiProcess->GetSharedMemoryMap();
	while (i < pSharedMemoryMap->m_pHeader->m_maxEntries && passedEntries < pSharedMemoryMap->m_pHeader->m_numEntries)
	{
		if (pSharedMemoryMap->m_pEntries[i].m_id != EMPTY_ENTRY)
		{
			passedEntries++;
			const ConnToCardTableEntry& currentEntry = pSharedMemoryMap->m_pEntries[i];

			if ((currentEntry.rsrc_conf_id== conf_id )&& (currentEntry.rsrcType== logicalResourceType))
			{
				pSharedMemoryMap->m_pEntries[i].DumpRaw("CMplApiSpecialCommandHandler::GetPhysicalInfoHeaderByLogicalRsrcType");
				return &(pSharedMemoryMap->m_pEntries[i]);
			}
		}
		i++;
	}
	PTRACE2COND(SHARED_MEMORY_DEBUG_PRINTS,eLevelInfoNormal,"CMplApiSpecialCommandHandler::GetPhysicalInfoHeaderByConfAndLogicalRsrcType"," - enty not found");
	return NULL;
}

//////////////////////////////////////////////////////////////////////////////////
void CMplApiSpecialCommandHandler::SetPhysicalInfoHeader(const ConnToCardTableEntry* pConnToCardTableEntry)
{
//	PHYSICAL_INFO_HEADER_S	PysicalInfoHeader;

	m_MplMcmsProt.m_MplMcmsPhysicalInfoHeader->box_id          = pConnToCardTableEntry->boxId;
	m_MplMcmsProt.m_MplMcmsPhysicalInfoHeader->board_id        = pConnToCardTableEntry->boardId;
	m_MplMcmsProt.m_MplMcmsPhysicalInfoHeader->sub_board_id    = pConnToCardTableEntry->subBoardId;
	m_MplMcmsProt.m_MplMcmsPhysicalInfoHeader->unit_id         = pConnToCardTableEntry->unitId;
	m_MplMcmsProt.m_MplMcmsPhysicalInfoHeader->accelerator_id  = pConnToCardTableEntry->acceleratorId;
	m_MplMcmsProt.m_MplMcmsPhysicalInfoHeader->port_id         = pConnToCardTableEntry->portId;
	m_MplMcmsProt.m_MplMcmsPhysicalInfoHeader->resource_type   = pConnToCardTableEntry->physicalRsrcType;

	m_MplMcmsProt.m_MplMcmsPhysicalInfoHeader->future_use1 = 0;
	m_MplMcmsProt.m_MplMcmsPhysicalInfoHeader->future_use2 = 0;
	m_MplMcmsProt.m_MplMcmsPhysicalInfoHeader->reserved[0] = 0;
	m_MplMcmsProt.m_MplMcmsPhysicalInfoHeader->reserved[1] = 0;

	//m_PhysicalInfoHeaderCounter++;
	m_MplMcmsProt.m_PhysicalInfoHeaderCounter = 1;

//	return PysicalInfoHeader;
}

void CMplApiSpecialCommandHandler::SetPhysicalResourceInfo(const ConnToCardTableEntry* pConnToCardTableEntry,
                                                           PHYSICAL_RESOURCE_INFO_S &PhysicalResourceInfo,bool setChannlId)const
{
//	PHYSICAL_RESOURCE_INFO_S	PhysicalResourceInfo;
	// to Michael and Sergey
	PhysicalResourceInfo.physical_unit_params.box_id          = pConnToCardTableEntry->boxId;
	PhysicalResourceInfo.physical_unit_params.board_id        = pConnToCardTableEntry->boardId;
	PhysicalResourceInfo.physical_unit_params.sub_board_id    = pConnToCardTableEntry->subBoardId;
	PhysicalResourceInfo.physical_unit_params.unit_id         = pConnToCardTableEntry->unitId;
	PhysicalResourceInfo.accelerator_id						  = pConnToCardTableEntry->acceleratorId;
	PhysicalResourceInfo.port_id							  = pConnToCardTableEntry->portId;
	PhysicalResourceInfo.resource_type                        = pConnToCardTableEntry->physicalRsrcType;

    if(setChannlId)
    {

        // isdn close port - channel_id
        WORD channel_id = pConnToCardTableEntry->channelId;
        APIU8 channel_id_ones = channel_id-(channel_id/100)*100;
        APIU8 channel_id_hundreds = channel_id/100;
       // For example port 214 will decode:
       // future_use1 = 2;
       // future_use2 = 14;

        PhysicalResourceInfo.future_use1 = channel_id_hundreds;
        PhysicalResourceInfo.future_use2 = channel_id_ones;
    }

    PhysicalResourceInfo.reserved[0] = 0;
    PhysicalResourceInfo.reserved[1] = 0;

//	return PhysicalResourceInfo;
}

void CMplApiSpecialCommandHandler::PrintAssert(eResourceTypes type, ECntrlType cntrlType, OPCODE opcode)const
{
	char buff[512];
	snprintf(buff, sizeof(buff),
            "On Opcode : %s\n\
Resource was not found in tne shared memory table;\n\
Resource : %s, Cntrl Type : %s",
			CProcessBase::GetProcess()->GetOpcodeAsString(opcode).c_str(),
            ::ResourceTypeToString(type),
            ::CntrlTypeToString(cntrlType));

	//PASSERTMSG(1, buff);
	EXCEPTION_TRACE(1, buff);
}

void CMplApiSpecialCommandHandler::PrintAssert(eResourceTypes type)const
{
	char buff[512];
	snprintf(buff, sizeof(buff),
			"Resource was not found in tne shared memory table; Resource : %s",
			::ResourceTypeToString(type));

	//PASSERTMSG(1, buff);
	EXCEPTION_TRACE(1, buff);
}

void CMplApiSpecialCommandHandler::PrintAssert(eLogicalResourceTypes type)const
{
	char buff[512];
	snprintf(buff, sizeof(buff),
			"Logical Resource was not found in tne shared memory table; Logical Resource : %s",
			::LogicalResourceTypeToString(type));

	//PASSERTMSG(1, buff);
	EXCEPTION_TRACE(1, buff);
}

void CMplApiSpecialCommandHandler::PrintAssert(int confId, int partyId, OPCODE opcode)const
{
	char buff[512];
	snprintf(buff, sizeof(buff),
			"Can't find ePhysical_art in rsrc table;\nConf Id : %d\nParty Id : %d\nOpcode : %s",
		 confId, partyId,CProcessBase::GetProcess()->GetOpcodeAsString(opcode).c_str());

	//PASSERTMSG(1, buff);
	EXCEPTION_TRACE(1, buff);

}

void CMplApiSpecialCommandHandler::PrintAssert(DWORD partyId, eResourceTypes type)const
{
	char buff[512];
	snprintf(buff, sizeof(buff),
			"Resource was not found in tne shared memory table; Resource : %s, Party Id : %d",
			::ResourceTypeToString(type), partyId);

	//PASSERTMSG(1, buff);
	EXCEPTION_TRACE(1, buff);
}

void CMplApiSpecialCommandHandler::SetPSTNContent(const ConnToCardTableEntry& localDesc,
												  TOpenUdpPortOrUpdateUdpAddrMessageStruct& openUdpStruct)
{
  int conf_id    = m_MplMcmsProt.getPortDescriptionHeaderConf_id();
  int party_id   = m_MplMcmsProt.getPortDescriptionHeaderParty_id();
  DWORD remoteIpAddress = 0xffffffff;
  eResourceTypes localRsrcType=localDesc.physicalRsrcType;
  eResourceTypes remoteRsrcType=(localRsrcType == ePhysical_rtm ? ePhysical_art : ePhysical_rtm );

  if(remoteRsrcType==ePhysical_rtm){
      TRACESTR (eLevelInfoNormal)<<"CMplApiSpecialCommandHandler::SetPSTNContent remoteRsrcType==ePhysical_rtm ";
  }
  if(localRsrcType==ePhysical_rtm){
      TRACESTR (eLevelInfoNormal)<<"CMplApiSpecialCommandHandler::SetPSTNContent localRsrcType==ePhysical_rtm ";
  }
  if(remoteRsrcType==ePhysical_art){
      TRACESTR (eLevelInfoNormal)<<"CMplApiSpecialCommandHandler::SetPSTNContent remoteRsrcType==ePhysical_art ";
  }
  if(localRsrcType==ePhysical_art){
      TRACESTR (eLevelInfoNormal)<<"CMplApiSpecialCommandHandler::SetPSTNContent localRsrcType==ePhysical_art ";
  }



   const ConnToCardTableEntry* pRemoteDesc = NULL;

   if(remoteRsrcType==ePhysical_rtm)
   {
      DWORD connection_id   = m_MplMcmsProt.getPortDescriptionHeaderConnection_id();
      TRACESTR (eLevelInfoNormal)<<"CMplApiSpecialCommandHandler::SetPSTNContent m_MplMcmsProt.getPortDescriptionHeaderConnection_id =  " << connection_id ;
       pRemoteDesc = GetPhysicalInfoHeaderByConnectionID(connection_id);

   }else{

      pRemoteDesc = GetPhysicalInfoHeaderByConfAndPartyID(conf_id,party_id,remoteRsrcType);
  }

  if( pRemoteDesc == NULL )
    {
      PrintAssert(conf_id, party_id,m_MplMcmsProt.getCommonHeaderOpcode());
      return;
    }

   if(localDesc.channelId == CHANNEL_ID_FROM_RTM){
      TRACESTR (eLevelInfoNormal)<<"CMplApiSpecialCommandHandler::SetPSTNContent localDesc.channelId == CHANNEL_ID_FROM_RTM ";
  }
  if(pRemoteDesc->channelId == CHANNEL_ID_FROM_RTM){
      TRACESTR (eLevelInfoNormal)<<"CMplApiSpecialCommandHandler::SetPSTNContent pRemoteDesc->channelId == CHANNEL_ID_FROM_RTM ";
  }


  //Get the Remote IP
  remoteIpAddress = pRemoteDesc->ipAdress;

  if (0xffffffff ==  remoteIpAddress)
    PASSERT_AND_RETURN(openUdpStruct.physicalPort.connection_id);

  //Set local IP addresses
  openUdpStruct.tCmOpenUdpPortOrUpdateUdpAddr.CmLocalUdpAddressIp.addr.v4.ip=localDesc.ipAdress;

  //Set Remote IP addresses
  openUdpStruct.tCmOpenUdpPortOrUpdateUdpAddr.CmRemoteUdpAddressIp.addr.v4.ip=remoteIpAddress;

  //Set the local port in all rsrsces type
  openUdpStruct.tCmOpenUdpPortOrUpdateUdpAddr.CmLocalUdpAddressIp.port =
	(localDesc.channelId != CHANNEL_ID_FROM_RTM) ? localDesc.channelId : pRemoteDesc->channelId;

  //set the Remote UDP
  openUdpStruct.tCmOpenUdpPortOrUpdateUdpAddr.CmRemoteUdpAddressIp.port =
	(pRemoteDesc->channelId != CHANNEL_ID_FROM_RTM) ? pRemoteDesc->channelId : localDesc.channelId;
}

void CMplApiSpecialCommandHandler::SetPSTNPhysicalInfoHeader(const ConnToCardTableEntry & localDesc)
{
  //Set the Physical-Header
  m_MplMcmsProt.m_MplMcmsPhysicalInfoHeader->box_id=localDesc.boxId;
  m_MplMcmsProt.m_MplMcmsPhysicalInfoHeader->board_id=localDesc.boardId;
  m_MplMcmsProt.m_MplMcmsPhysicalInfoHeader->sub_board_id=localDesc.subBoardId;
  m_MplMcmsProt.m_MplMcmsPhysicalInfoHeader->unit_id=0;
  m_MplMcmsProt.m_MplMcmsPhysicalInfoHeader->accelerator_id=0;
  m_MplMcmsProt.m_MplMcmsPhysicalInfoHeader->port_id=0;
  m_MplMcmsProt.m_MplMcmsPhysicalInfoHeader->resource_type=ePhysical_res_none;

  // isdn close port - channel_id
  WORD channel_id = localDesc.channelId;
  APIU8 channel_id_ones = channel_id-(channel_id/100)*100;
  APIU8 channel_id_hundreds = channel_id/100;
// For example port 214 will decode:
// future_use1 = 2;
// future_use2 = 14;

  m_MplMcmsProt.m_MplMcmsPhysicalInfoHeader->future_use1 = channel_id_hundreds;
  m_MplMcmsProt.m_MplMcmsPhysicalInfoHeader->future_use2 = channel_id_ones;

  m_MplMcmsProt.m_MplMcmsPhysicalInfoHeader->reserved[0] = 0;
  m_MplMcmsProt.m_MplMcmsPhysicalInfoHeader->reserved[1] = 0;
}

void CMplApiSpecialCommandHandler::SetPSTNContent(const ConnToCardTableEntry& localDesc,
												  TCloseUdpPortMessageStruct& openUdpStruct)
{
  int conf_id    = m_MplMcmsProt.getPortDescriptionHeaderConf_id();
  int party_id   = m_MplMcmsProt.getPortDescriptionHeaderParty_id();
  DWORD remoteIpAddress = 0xffffffff;
  eResourceTypes localRsrcType=localDesc.physicalRsrcType;
  eResourceTypes remoteRsrcType=(localRsrcType == ePhysical_rtm ? ePhysical_art : ePhysical_rtm );

  if(remoteRsrcType==ePhysical_rtm){
      TRACESTR (eLevelInfoNormal)<<"CMplApiSpecialCommandHandler::SetPSTNContent remoteRsrcType==ePhysical_rtm ";
  }
  if(localRsrcType==ePhysical_rtm){
      TRACESTR (eLevelInfoNormal)<<"CMplApiSpecialCommandHandler::SetPSTNContent localRsrcType==ePhysical_rtm ";
  }
  if(remoteRsrcType==ePhysical_art){
      TRACESTR (eLevelInfoNormal)<<"CMplApiSpecialCommandHandler::SetPSTNContent remoteRsrcType==ePhysical_art ";
  }
  if(localRsrcType==ePhysical_art){
      TRACESTR (eLevelInfoNormal)<<"CMplApiSpecialCommandHandler::SetPSTNContent localRsrcType==ePhysical_art ";
  }



   const ConnToCardTableEntry* pRemoteDesc = NULL;

   if(remoteRsrcType==ePhysical_rtm)
   {
      DWORD connection_id   = m_MplMcmsProt.getPortDescriptionHeaderConnection_id();
      TRACESTR (eLevelInfoNormal)<<"CMplApiSpecialCommandHandler::SetPSTNContent m_MplMcmsProt.getPortDescriptionHeaderConnection_id =  " << connection_id ;
       pRemoteDesc = GetPhysicalInfoHeaderByConnectionID(connection_id);

   }else{

      pRemoteDesc = GetPhysicalInfoHeaderByConfAndPartyID(conf_id,party_id,remoteRsrcType);
  }

  if( pRemoteDesc == NULL )
    {
      PrintAssert(conf_id, party_id,m_MplMcmsProt.getCommonHeaderOpcode());
      return;
    }

   if(localDesc.channelId == CHANNEL_ID_FROM_RTM){
      TRACESTR (eLevelInfoNormal)<<"CMplApiSpecialCommandHandler::SetPSTNContent localDesc.channelId == CHANNEL_ID_FROM_RTM ";
  }
  if(pRemoteDesc->channelId == CHANNEL_ID_FROM_RTM){
      TRACESTR (eLevelInfoNormal)<<"CMplApiSpecialCommandHandler::SetPSTNContent pRemoteDesc->channelId == CHANNEL_ID_FROM_RTM ";
  }


  //Get the Remote IP
  remoteIpAddress = pRemoteDesc->ipAdress;

  if (0xffffffff ==  remoteIpAddress)
    PASSERT_AND_RETURN(openUdpStruct.physicalPort.connection_id);

  //Set local IP addresses
  openUdpStruct.tCmCloseUdpPort.CmLocalUdpAddressIp.addr.v4.ip=localDesc.ipAdress;

  //Set Remote IP addresses
  openUdpStruct.tCmCloseUdpPort.CmRemoteUdpAddressIp.addr.v4.ip=remoteIpAddress;

  //Set the local port in all rsrsces type
  openUdpStruct.tCmCloseUdpPort.CmLocalUdpAddressIp.port =
	(localDesc.channelId != CHANNEL_ID_FROM_RTM) ? localDesc.channelId : pRemoteDesc->channelId;

  //set the Remote UDP
  openUdpStruct.tCmCloseUdpPort.CmRemoteUdpAddressIp.port =
	(pRemoteDesc->channelId != CHANNEL_ID_FROM_RTM) ? pRemoteDesc->channelId : localDesc.channelId;
}

STATUS CMplApiSpecialCommandHandler::IceGeneralReq(void* param)
{
	const ConnToCardTableEntry* pConnToCardTableEntry = NULL;        
	PTRACE(eLevelInfoNormal,"CMplApiSpecialCommandHandler::IceGeneralReq");

        PTRACE2INT(eLevelInfoNormal,"CMplApiSpecialCommandHandler::IceGeneralReq - Len: ",m_MplMcmsProt.getDataLen() );

        STATUS status = m_MplMcmsProt.ValidateDataSize(      m_MplMcmsProt.getDataLen());
        if(STATUS_OK != status)
        {
		return status;
        }
        
	if(CProcessBase::GetProcess()->GetProductFamily() == eProductFamilySoftMcu)
	{
		//unid id !=0 .Only relevant for softmcu

		int conf_id    = m_MplMcmsProt.getPortDescriptionHeaderConf_id();
		int party_id   = m_MplMcmsProt.getPortDescriptionHeaderParty_id();
		eLogicalResourceTypes lrt = (eLogicalResourceTypes) 		m_MplMcmsProt.getPortDescriptionHeaderLogical_resource_type_1();

		pConnToCardTableEntry = GetPhysicalInfoHeaderByConfAndPartyID(conf_id,party_id,ePhysical_art, lrt);

		if( pConnToCardTableEntry==NULL )
		{
		    pConnToCardTableEntry=GetPhysicalInfoHeaderByConfAndPartyID(conf_id,party_id,ePhysical_art_light);
		}
	}

	APIU32 connectionId = 0xffffffff;
        ICE_GENERAL_REQ_S* IceGeneralReq = (ICE_GENERAL_REQ_S*)new BYTE[m_MplMcmsProt.getDataLen()];

        memcpy(IceGeneralReq, m_MplMcmsProt.getpData(), m_MplMcmsProt.getDataLen());

        connectionId = m_MplMcmsProt.getPortDescriptionHeaderConnection_id();
        if(0xffffffff != connectionId)
        {
                ConnToCardTableEntry connToCardTableEntry;
                status = m_pMplApiProcess->GetSharedMemoryMap()->Get(connectionId, connToCardTableEntry);

                if (STATUS_OK == status)
                {
                        connToCardTableEntry.unitId=0;
			if(CProcessBase::GetProcess()->GetProductFamily() == eProductFamilySoftMcu)
			{                
				//unid id !=0 .Only relevant for softmcu
				if( pConnToCardTableEntry!=NULL )
				{
				    ConnToCardTableEntry newConnToCardTableEntry(*pConnToCardTableEntry);

				    connToCardTableEntry.unitId = newConnToCardTableEntry.unitId;
				    connToCardTableEntry.portId = newConnToCardTableEntry.portId;
				}
			}
                
                        SetPhysicalInfoHeader(&connToCardTableEntry);
                }

        }

        m_MplMcmsProt.AddData( m_MplMcmsProt.getDataLen(),(const char*) IceGeneralReq);

        PDELETEA(IceGeneralReq);

        return status;

}



STATUS CMplApiSpecialCommandHandler::VideoEncoderDSPSmartSwitchChangeLayoutReq(void* param)
{
/*	//Removed since MCMS_CM_CHANGE_LAYOUT_S is now a dynamic structure
	STATUS status = m_MplMcmsProt.ValidateDataSize(	sizeof(MCMS_CM_CHANGE_LAYOUT_S));

	if(STATUS_OK != status)
	{
		return status;
	}
*/

	STATUS status = STATUS_OK;

	MCMS_CM_CHANGE_LAYOUT_S* pChangeLayout = new MCMS_CM_CHANGE_LAYOUT_S;

   	memcpy(pChangeLayout, m_MplMcmsProt.getpData(),sizeof(MCMS_CM_CHANGE_LAYOUT_S));

   	WORD numOfImages = CLibsCommonHelperFuncs::GetNumbSubImg(pChangeLayout->nLayoutType);
   	pChangeLayout->atImageParam = new MCMS_CM_IMAGE_PARAM_S[numOfImages];

   	int sizeOfImageParamArray = numOfImages*(sizeof(MCMS_CM_IMAGE_PARAM_S));
   	int sizeOfChangeLayoutWithoutImages = sizeof(MCMS_CM_CHANGE_LAYOUT_S)-sizeof(MCMS_CM_IMAGE_PARAM_S*);
   	int sizeOfChangeLayoutStruct = sizeOfChangeLayoutWithoutImages + sizeOfImageParamArray;

   	memcpy(&(pChangeLayout->atImageParam[0]), m_MplMcmsProt.getpData()+sizeOfChangeLayoutWithoutImages, sizeOfImageParamArray);

   	for (int i = 0 ; i < numOfImages ; i++)
	{
		APIU32 ConnectionId = pChangeLayout->atImageParam[i].tDecoderPhysicalId.connection_id;
		if (0xffffffff != ConnectionId && 0xfffffff0 != ConnectionId )
		{
			ConnToCardTableEntry connToCardTableEntry;
			status = m_pMplApiProcess->GetSharedMemoryMap()->Get(ConnectionId, connToCardTableEntry);
			if (STATUS_OK == status)
			{
				SetPhysicalResourceInfo(&connToCardTableEntry, pChangeLayout->atImageParam[i].tDecoderPhysicalId.physical_id);
			}
		}
	}

   	ALLOCBUFFER(tmp,sizeOfChangeLayoutStruct);
   	memcpy(tmp, pChangeLayout, sizeOfChangeLayoutWithoutImages);
   	memcpy(tmp+sizeOfChangeLayoutWithoutImages, &(pChangeLayout->atImageParam[0]), sizeOfImageParamArray);

   	m_MplMcmsProt.AddData( sizeOfChangeLayoutStruct,(const char*) tmp);


	DEALLOCBUFFER(tmp);
	PDELETEA(pChangeLayout->atImageParam);
	POBJDELETE(pChangeLayout);
	return status;
}
STATUS CMplApiSpecialCommandHandler::TBVideoPreferenceReq(void* param)
{
	PTRACE(eLevelInfoNormal,"CMplApiSpecialCommandHandler::TBVideoPreferenceReq");
	STATUS status = m_MplMcmsProt.ValidateDataSize(	sizeof(TCmRtcpVideoPreference));
	if(STATUS_OK != status)
	{
		return status;
	}
	APIU32 connectionId = 0xffffffff;
	TCmRtcpVideoPreference tbRtcpMsg;
	memcpy(&tbRtcpMsg, m_MplMcmsProt.getpData(), sizeof(TCmRtcpVideoPreference));

	connectionId = m_MplMcmsProt.getPortDescriptionHeaderConnection_id();
	if(0xffffffff != connectionId)
	{
		ConnToCardTableEntry connToCardTableEntry;
		status = m_pMplApiProcess->GetSharedMemoryMap()->Get(connectionId, connToCardTableEntry);

		if (STATUS_OK == status)
		{
			(tbRtcpMsg.tCmRtcpHeader).uDspNumber = connToCardTableEntry.unitId;
			(tbRtcpMsg.tCmRtcpHeader).uPortNumber = connToCardTableEntry.portId;
			connToCardTableEntry.unitId=0;
			SetPhysicalInfoHeader(&connToCardTableEntry);
		}

	}

	m_MplMcmsProt.AddData( sizeof(TCmRtcpVideoPreference),(const char*) &tbRtcpMsg);

	return status;

}
STATUS CMplApiSpecialCommandHandler::TBReceiverBandWidthReq(void* param)
{
	PTRACE(eLevelInfoNormal,"CMplApiSpecialCommandHandler::TBReceiverBandWidthReq");
	STATUS status = m_MplMcmsProt.ValidateDataSize(	sizeof(TCmRtcpBandwidth));
	if(STATUS_OK != status)
	{
		return status;
	}
	APIU32 connectionId = 0xffffffff;
	TCmRtcpBandwidth tbRtcpMsg;
	memcpy(&tbRtcpMsg, m_MplMcmsProt.getpData(), sizeof(TCmRtcpBandwidth));

	connectionId = m_MplMcmsProt.getPortDescriptionHeaderConnection_id();
	if(0xffffffff != connectionId)
	{
		ConnToCardTableEntry connToCardTableEntry;
		status = m_pMplApiProcess->GetSharedMemoryMap()->Get(connectionId, connToCardTableEntry);

		if (STATUS_OK == status)
		{
			(tbRtcpMsg.tCmRtcpHeader).uDspNumber = connToCardTableEntry.unitId;
			(tbRtcpMsg.tCmRtcpHeader).uPortNumber = connToCardTableEntry.portId;
			connToCardTableEntry.unitId=0;
			SetPhysicalInfoHeader(&connToCardTableEntry);
		}

	}
	m_MplMcmsProt.AddData( sizeof(TCmRtcpBandwidth),(const char*) &tbRtcpMsg);

	return status;


}

STATUS CMplApiSpecialCommandHandler::TBBfcpMessageReq(void* param)
{
	PTRACE(eLevelInfoNormal,"CMplApiSpecialCommandHandler::TBBfcpMessageReq");

	int len = m_MplMcmsProt.getDataLen();

	PTRACE2INT(eLevelInfoNormal,"CMplApiSpecialCommandHandler::TBBfcpMessageReq - Len: ",len );

	STATUS status = m_MplMcmsProt.ValidateDataSize(len);

	if(STATUS_OK != status)
	{
		return status;
	}

	mcReqBfcpMessage *pBfcpMsgReq = (mcReqBfcpMessage*)new BYTE[m_MplMcmsProt.getDataLen()];

	memcpy(pBfcpMsgReq, m_MplMcmsProt.getpData(), m_MplMcmsProt.getDataLen());

	APIU32 connectionId = m_MplMcmsProt.getPortDescriptionHeaderConnection_id();

	if(0xffffffff != connectionId)
	{
		ConnToCardTableEntry connToCardTableEntry;

		status = m_pMplApiProcess->GetSharedMemoryMap()->Get(connectionId, connToCardTableEntry);

		if (STATUS_OK == status)
		{
			connToCardTableEntry.unitId=0;
			SetPhysicalInfoHeader(&connToCardTableEntry);
		}
	}

	m_MplMcmsProt.AddData( m_MplMcmsProt.getDataLen(),(const char*) pBfcpMsgReq);

	PDELETEA(pBfcpMsgReq);

	return status;

}

STATUS CMplApiSpecialCommandHandler::MsSvcInitDesc(SingleStreamDesc* pDesc, const BYTE noOfStreamDescs, const ConnToCardTableEntry& connToCardTableEntry)
{
	STATUS status = STATUS_OK;

	if(pDesc && noOfStreamDescs)
	{
		pDesc -> unPortNum	= connToCardTableEntry.portId;
		pDesc -> unDspNum	= connToCardTableEntry.unitId;
		CSmallString log;
		log << "Port[" << pDesc -> unPortNum << "], Dsp[" << pDesc -> unDspNum << "]";
		PTRACE2(eLevelInfoNormal,"CMplApiSpecialCommandHandler::MsSvcInitDesc - ", log.GetString());

		++pDesc;
		for(BYTE descNdx = 1; descNdx < noOfStreamDescs; ++descNdx, ++pDesc)
		{
			pDesc -> unPortNum	= 0;
			pDesc -> unDspNum	= 0;
		}
	}
	else
	{
		CSmallString log;
		log << "pDesc[" << (DWORD) pDesc << "], noOfStreamDescs[" << noOfStreamDescs << "]";
		PTRACE2(eLevelError,"CMplApiSpecialCommandHandler::MsSvcInitDesc - Arguments invalid: ", log.GetString());
		DBGPASSERT(TRUE);
		status = STATUS_FAIL;
	}

	return status;
}

STATUS CMplApiSpecialCommandHandler::MsSvcMuxDmuxDesc(SingleStreamDesc* pDesc, const BYTE noOfStreamDescs)
{
	STATUS status = STATUS_OK;

	if(pDesc && noOfStreamDescs)
	{
		//======================================================================================
		// Preparing to determine the connection to the card for each slave party individually
		//======================================================================================
		UINT32 conf_id = m_MplMcmsProt.getPortDescriptionHeaderConf_id();

		for(BYTE descNdx = 0; descNdx < noOfStreamDescs; ++descNdx, ++pDesc)
		{
			UINT32 party_id = pDesc -> unPartyId;
			if (party_id == (UINT32) -1 || party_id == 0)
			{
				//===============================================
				// Party is invalid, taking the master party-id
				//===============================================
				pDesc -> unPortNum	= 0;
				pDesc -> unDspNum	= 0;
			}
			else
			{
				//=========================
				// Determining connection
				//=========================
				const ConnToCardTableEntry *pConnToCardTableEntry = GetMainARTphysicalInfoHeaderByConfAndPartyID(conf_id, party_id);
				if (pConnToCardTableEntry)
				{
					pDesc -> unPortNum	= pConnToCardTableEntry -> portId;
					pDesc -> unDspNum	= pConnToCardTableEntry -> unitId;
				}
				else
				{
					pDesc -> unPortNum	= 0;
					pDesc -> unDspNum	= 0;
					PTRACE(eLevelError,"CMplApiSpecialCommandHandler::MsSvcMuxDmuxDesc - GetMainARTphysicalInfoHeaderByConfAndPartyID failed, setting port/dsp to zero");
					status = STATUS_FAIL;
					DBGPASSERT(TRUE);
				}
			}
			CSmallString log;
			log << "DescNdx[" << descNdx << "], Port[" << pDesc -> unPortNum << "], Dsp[" << pDesc -> unDspNum << "]";
			PTRACE2(eLevelInfoNormal,"CMplApiSpecialCommandHandler::MsSvcMuxDmuxDesc - ", log.GetString());
		}
	}
	else
	{
		CSmallString log;
		log << "pDesc[" << (DWORD) pDesc << "], noOfStreamDescs[" << noOfStreamDescs << "]";
		PTRACE2(eLevelError,"CMplApiSpecialCommandHandler::MsSvcMuxDmuxDesc - Arguments invalid: ", log.GetString());
		DBGPASSERT(TRUE);
		status = STATUS_FAIL;
	}

	return status;
}
///////////////////////////////////////////////////////////////////
STATUS CMplApiSpecialCommandHandler::MsSvcDspInfoForPartyMonitoringReqDesc(DspInfoForPartyMonitoringReq* pDesc, const BYTE noOfStreamDescs)
{
	STATUS status = STATUS_OK;

	if(pDesc && noOfStreamDescs)
	{
		//======================================================================================
		// Preparing to determine the connection to the card for each slave party individually
		//======================================================================================
		UINT32 conf_id = m_MplMcmsProt.getPortDescriptionHeaderConf_id();

		for(BYTE descNdx = 0; descNdx < noOfStreamDescs; ++descNdx, ++pDesc)
		{
			UINT32 party_id = pDesc -> unPartyId;
			if (party_id == (UINT32) -1 || party_id == 0)
			{
				//===============================================
				// Party is invalid, taking the master party-id
				//===============================================
				pDesc -> unPortNum	= 0;
				pDesc -> unDspNum	= 0;
			}
			else
			{
				//=========================
				// Determining connection
				//=========================
				const ConnToCardTableEntry *pConnToCardTableEntry = GetMainARTphysicalInfoHeaderByConfAndPartyID(conf_id, party_id);
				if (pConnToCardTableEntry)
				{
					pDesc -> unPortNum	= pConnToCardTableEntry -> portId;
					pDesc -> unDspNum	= pConnToCardTableEntry -> unitId;
				}
				else
				{
					pDesc -> unPortNum	= 0;
					pDesc -> unDspNum	= 0;
					PTRACE(eLevelError,"CMplApiSpecialCommandHandler::MsSvcMuxDmuxDesc - GetMainARTphysicalInfoHeaderByConfAndPartyID failed, setting port/dsp to zero");
					status = STATUS_FAIL;
					DBGPASSERT(TRUE);
				}
			}
			CSmallString log;
			log << "DescNdx[" << descNdx << "], Port[" << pDesc -> unPortNum << "], Dsp[" << pDesc -> unDspNum << "]";
			PTRACE2(eLevelInfoNormal,"CMplApiSpecialCommandHandler::MsSvcMuxDmuxDesc - ", log.GetString());
		}
	}
	else
	{
		CSmallString log;
		log << "pDesc[" << (DWORD) pDesc << "], noOfStreamDescs[" << noOfStreamDescs << "]";
		PTRACE2(eLevelError,"CMplApiSpecialCommandHandler::MsSvcMuxDmuxDesc - Arguments invalid: ", log.GetString());
		DBGPASSERT(TRUE);
		status = STATUS_FAIL;
	}

	return status;
}

STATUS CMplApiSpecialCommandHandler::MsSvcP2PInitReq(void* param)
{
	STATUS status = STATUS_OK;

  	int conf_id    = m_MplMcmsProt.getPortDescriptionHeaderConf_id();
	int party_id   = m_MplMcmsProt.getPortDescriptionHeaderParty_id();

	const ConnToCardTableEntry *pConnToCardTableEntry = GetMainARTphysicalInfoHeaderByConfAndPartyID(conf_id, party_id);

	if( pConnToCardTableEntry!=NULL )
	{
	    ConnToCardTableEntry connToCardTableEntry(*pConnToCardTableEntry);

	    mcBasicLync2013InfoReq msSvcStruct;
	    memcpy(&msSvcStruct, m_MplMcmsProt.getpData(), sizeof(msSvcStruct));
	    MsSvcInitDesc(reinterpret_cast<SingleStreamDesc*>(msSvcStruct.connectedParties), MAX_STREAM_BASIC_LYNC_CONN, connToCardTableEntry);
	    connToCardTableEntry.physicalRsrcType=ePhysical_res_none;
	    connToCardTableEntry.unitId=0;
	    SetPhysicalInfoHeader(&connToCardTableEntry);
	    m_MplMcmsProt.AddData( sizeof(msSvcStruct),(const char*) &msSvcStruct);
	}
	else
	{
	  PrintAssert(conf_id, party_id,m_MplMcmsProt.getCommonHeaderOpcode());
		status = STATUS_PROBLEMS_IN_SHARED_MEMORY;
	}
	return status;
}


STATUS CMplApiSpecialCommandHandler::MsSvcAvMcuInitReq(void* param)
{
	STATUS status = STATUS_OK;

  	int conf_id    = m_MplMcmsProt.getPortDescriptionHeaderConf_id();
	int party_id   = m_MplMcmsProt.getPortDescriptionHeaderParty_id();

	const ConnToCardTableEntry *pConnToCardTableEntry = GetMainARTphysicalInfoHeaderByConfAndPartyID(conf_id, party_id);

	if( pConnToCardTableEntry!=NULL )
	{
	    ConnToCardTableEntry connToCardTableEntry(*pConnToCardTableEntry);

	    mcAvMcuLync2013InfoReq msSvcStruct;
	    memcpy(&msSvcStruct, m_MplMcmsProt.getpData(), sizeof(msSvcStruct));
	    MsSvcInitDesc(reinterpret_cast<SingleStreamDesc*>(msSvcStruct.ConnectedParties), MAX_STREAM_LYNC_2013_CONN, connToCardTableEntry);
	    connToCardTableEntry.physicalRsrcType=ePhysical_res_none;
	    connToCardTableEntry.unitId=0;
	    SetPhysicalInfoHeader(&connToCardTableEntry);
	    m_MplMcmsProt.AddData( sizeof(msSvcStruct),(const char*) &msSvcStruct);
	}
	else
	{
	  PrintAssert(conf_id, party_id,m_MplMcmsProt.getCommonHeaderOpcode());
		status = STATUS_PROBLEMS_IN_SHARED_MEMORY;
	}
	return status;
}


STATUS CMplApiSpecialCommandHandler::MsSvcAvMcuMuxReq(void* param)
{
	STATUS status = STATUS_OK;

  	int conf_id    = m_MplMcmsProt.getPortDescriptionHeaderConf_id();
	int party_id   = m_MplMcmsProt.getPortDescriptionHeaderParty_id();

	const ConnToCardTableEntry *pConnToCardTableEntry = GetMainARTphysicalInfoHeaderByConfAndPartyID(conf_id, party_id);

	if( pConnToCardTableEntry!=NULL )
	{
	    ConnToCardTableEntry connToCardTableEntry(*pConnToCardTableEntry);

	    mcMuxLync2013InfoReq msSvcStruct;
	    memcpy(&msSvcStruct, m_MplMcmsProt.getpData(), sizeof(msSvcStruct));
	    MsSvcMuxDmuxDesc(reinterpret_cast<SingleStreamDesc*>(msSvcStruct.txConnectedParties), MAX_STREAM_MUX_LYNC_CONN);
	    connToCardTableEntry.physicalRsrcType=ePhysical_res_none;
	    connToCardTableEntry.unitId=0;
	    SetPhysicalInfoHeader(&connToCardTableEntry);
	    m_MplMcmsProt.AddData( sizeof(msSvcStruct),(const char*) &msSvcStruct);
	}
	else
	{
	  PrintAssert(conf_id, party_id,m_MplMcmsProt.getCommonHeaderOpcode());
		status = STATUS_PROBLEMS_IN_SHARED_MEMORY;
	}
	return status;
}


STATUS CMplApiSpecialCommandHandler::MsSvcAvMcuDmuxReq(void* param)
{
	STATUS status = STATUS_OK;

  	int conf_id    = m_MplMcmsProt.getPortDescriptionHeaderConf_id();
	int party_id   = m_MplMcmsProt.getPortDescriptionHeaderParty_id();

	const ConnToCardTableEntry *pConnToCardTableEntry = GetMainARTphysicalInfoHeaderByConfAndPartyID(conf_id, party_id);

	if( pConnToCardTableEntry!=NULL )
	{
	    ConnToCardTableEntry connToCardTableEntry(*pConnToCardTableEntry);

	    mcDmuxLync2013InfoReq msSvcStruct;
	    memcpy(&msSvcStruct, m_MplMcmsProt.getpData(), sizeof(msSvcStruct));
	    MsSvcMuxDmuxDesc(reinterpret_cast<SingleStreamDesc*>(msSvcStruct.rxConnectedParties), MAX_STREAM_DMUX_LYNC_CONN);
	    connToCardTableEntry.physicalRsrcType=ePhysical_res_none;
	    connToCardTableEntry.unitId=0;
	    SetPhysicalInfoHeader(&connToCardTableEntry);
	    m_MplMcmsProt.AddData( sizeof(msSvcStruct),(const char*) &msSvcStruct);
	}
	else
	{
	  PrintAssert(conf_id, party_id,m_MplMcmsProt.getCommonHeaderOpcode());
		status = STATUS_PROBLEMS_IN_SHARED_MEMORY;
	}
	return status;
}


STATUS CMplApiSpecialCommandHandler::MsSvcAvMcuMonitoringReq(void* param)
{
	STATUS status = STATUS_OK;

	int conf_id    = m_MplMcmsProt.getPortDescriptionHeaderConf_id();
	int party_id   = m_MplMcmsProt.getPortDescriptionHeaderParty_id();

	const ConnToCardTableEntry *pConnToCardTableEntry = GetMainARTphysicalInfoHeaderByConfAndPartyID(conf_id, party_id);

	if( pConnToCardTableEntry!=NULL )
	{
		ConnToCardTableEntry connToCardTableEntry(*pConnToCardTableEntry);

		cmPartyMonitoringAvMcuReq msSvcStruct;
		memcpy(&msSvcStruct, m_MplMcmsProt.getpData(), sizeof(msSvcStruct));
		MsSvcDspInfoForPartyMonitoringReqDesc(reinterpret_cast<DspInfoForPartyMonitoringReq*>(msSvcStruct.DSPInfoList), MAX_STREAM_AVMCU_CONN);
		connToCardTableEntry.physicalRsrcType=ePhysical_res_none;
		connToCardTableEntry.unitId=0;
		SetPhysicalInfoHeader(&connToCardTableEntry);
		m_MplMcmsProt.AddData( sizeof(msSvcStruct),(const char*) &msSvcStruct);
	}
	else
	{
	  PrintAssert(conf_id, party_id,m_MplMcmsProt.getCommonHeaderOpcode());
		status = STATUS_PROBLEMS_IN_SHARED_MEMORY;
	}
	return status;

}


STATUS CMplApiSpecialCommandHandler::VsrMsgReq(void* param)
{
	STATUS status = STATUS_OK;

  	int conf_id    = m_MplMcmsProt.getPortDescriptionHeaderConf_id();
	int party_id   = m_MplMcmsProt.getPortDescriptionHeaderParty_id();

	const ConnToCardTableEntry *pConnToCardTableEntry = GetMainARTphysicalInfoHeaderByConfAndPartyID(conf_id, party_id);

	if( pConnToCardTableEntry!=NULL )
	{
	    ConnToCardTableEntry connToCardTableEntry(*pConnToCardTableEntry);


	    TCmRtcpVsrMsg cmVsrMsg;
	    memcpy(&cmVsrMsg, m_MplMcmsProt.getpData(), sizeof(cmVsrMsg));
	    (cmVsrMsg.cmRtcpHeader).uDspNumber = connToCardTableEntry.unitId;
	    (cmVsrMsg.cmRtcpHeader).uPortNumber = connToCardTableEntry.portId;

	    connToCardTableEntry.physicalRsrcType=ePhysical_res_none;
	    connToCardTableEntry.unitId=0;
	    SetPhysicalInfoHeader(&connToCardTableEntry);

	    m_MplMcmsProt.AddData( sizeof(cmVsrMsg),(const char*) &cmVsrMsg);
	}
	else
	{
	  PrintAssert(conf_id, party_id,m_MplMcmsProt.getCommonHeaderOpcode());
		status = STATUS_PROBLEMS_IN_SHARED_MEMORY;
	}
	return status;
}


STATUS CMplApiSpecialCommandHandler::MsSvcPliMsgReq(void* param)
{
	STATUS status = STATUS_OK;

  	int conf_id    = m_MplMcmsProt.getPortDescriptionHeaderConf_id();
	int party_id   = m_MplMcmsProt.getPortDescriptionHeaderParty_id();

	const ConnToCardTableEntry *pConnToCardTableEntry = GetMainARTphysicalInfoHeaderByConfAndPartyID(conf_id, party_id);

	if( pConnToCardTableEntry!=NULL )
	{
	    ConnToCardTableEntry connToCardTableEntry(*pConnToCardTableEntry);


	    TCmRtcpMsSvcPLIMsg cmRtcpMsSvcPLIMsg;
	    memcpy(&cmRtcpMsSvcPLIMsg, m_MplMcmsProt.getpData(), sizeof(cmRtcpMsSvcPLIMsg));
	    (cmRtcpMsSvcPLIMsg.cmRtcpHeader).uDspNumber 	= connToCardTableEntry.unitId;
	    (cmRtcpMsSvcPLIMsg.cmRtcpHeader).uPortNumber	= connToCardTableEntry.portId;

	    connToCardTableEntry.physicalRsrcType=ePhysical_res_none;
	    connToCardTableEntry.unitId=0;
	    SetPhysicalInfoHeader(&connToCardTableEntry);

	    m_MplMcmsProt.AddData( sizeof(cmRtcpMsSvcPLIMsg),(const char*) &cmRtcpMsSvcPLIMsg);
	}
	else
	{
	  PrintAssert(conf_id, party_id,m_MplMcmsProt.getCommonHeaderOpcode());
		status = STATUS_PROBLEMS_IN_SHARED_MEMORY;
	}
	return status;
}


STATUS CMplApiSpecialCommandHandler::OpenChannelReq(void* param)
{
	STATUS status = m_MplMcmsProt.ValidateDataSize(	sizeof(MrmpOpenChannelRequestStruct));
	if(STATUS_OK != status)
	{
		PTRACE(eLevelInfoNormal,"CMplApiSpecialCommandHandler::OpenChannelReq STATUS_OK != status");
		return status;
	}
	MrmpOpenChannelRequestStruct openChannelStruct;
	memcpy(&openChannelStruct, m_MplMcmsProt.getpData(), sizeof(MrmpOpenChannelRequestStruct));

	for (unsigned int i = 0; i < openChannelStruct.m_allocatedPhysicalResources; i++)
	{
        APIU32 ConnectionId = openChannelStruct.physicalId[i].connection_id;
        ConnToCardTableEntry connToCardTableEntry;
        bool bFillPhysical = false;
        if(INVALID != ConnectionId)
        {
            status = m_pMplApiProcess->GetSharedMemoryMap()->Get(ConnectionId, connToCardTableEntry);
            if (STATUS_OK == status)
            {
                SetPhysicalResourceInfo(&connToCardTableEntry, openChannelStruct.physicalId[i].physical_id);
                bFillPhysical = true;
            }
        }
        if (!bFillPhysical)
        {
              connToCardTableEntry.physicalRsrcType=ePhysical_res_none;
              connToCardTableEntry.unitId=0;
              SetPhysicalResourceInfo(&connToCardTableEntry,openChannelStruct.physicalId[i].physical_id);
        }

        TRACEINTOFUNC << "connId="  << ConnectionId <<  ",  status= "<< status << ", struct for physicalId[" << i << "]: \n" <<
                "party_id: "<<(WORD)openChannelStruct.m_partyId << ", \n" <<
                "box_id: "<<(WORD)openChannelStruct.physicalId[i].physical_id.physical_unit_params.box_id   << ", \n" <<
                "board_id: "<<(WORD)openChannelStruct.physicalId[i].physical_id.physical_unit_params.board_id  <<", \n" <<
                "sub_board_id: "<<(WORD)openChannelStruct.physicalId[i].physical_id.physical_unit_params.sub_board_id  << ", \n" <<
                "unit_id: "<<(WORD)openChannelStruct.physicalId[i].physical_id.physical_unit_params.unit_id      <<", \n" <<
                "port_id: "<<(WORD)openChannelStruct.physicalId[i].physical_id.port_id			<<", \n" <<
                "resource_type: "<<(WORD)openChannelStruct.physicalId[i].physical_id.resource_type;
	}

   	m_MplMcmsProt.AddData( sizeof(MrmpOpenChannelRequestStruct),(const char*) &openChannelStruct);

	return status;
}


STATUS CMplApiSpecialCommandHandler::UpdateChannelReq(void* param)
{
	PTRACE(eLevelDebug,"CMplApiSpecialCommandHandler::UpdateChannelReq");
	return OpenChannelReq(param);
}

///////////////////////////////////////////////////////////////////////////////
STATUS CMplApiSpecialCommandHandler::AddRemoveVideoOperationPointSetReq(void* param)
{
	STATUS status = STATUS_OK;
	//The messages is send to MRMP, need to set in the physical header ePhysical_mrmp physical type.
	//The MRMP runs over card, relay conference is closed on card inorder to find the physical info box, board, subboard .. to fill we
	//Will see on which card the rtp port was opened


  	int conf_id    = m_MplMcmsProt.getPortDescriptionHeaderConf_id();
    TRACESTR (eLevelInfoNormal)<<"CMplApiSpecialCommandHandler::AddRemoveVideoOperationPointSetReq confId: " << conf_id;

	const ConnToCardTableEntry* pConnToCardTableEntry = GetPhysicalInfoHeaderByConfAndLogicalRsrcType(conf_id,eLogical_rtp);
	if(pConnToCardTableEntry==NULL)
	{
		//we will see maybe the party is ISDN (it may happen in mix conferences)
		pConnToCardTableEntry = GetPhysicalInfoHeaderByConfAndLogicalRsrcType(conf_id,eLogical_net);
	}

	if(pConnToCardTableEntry==NULL)
	{
		PASSERT(conf_id);
		m_MplMcmsProt.AddPhysicalHeader(1,1,1,0,0,0,ePhysical_mrmp);
		status = STATUS_PROBLEMS_IN_SHARED_MEMORY;
	}

	if(pConnToCardTableEntry!=NULL)
	{
		m_MplMcmsProt.AddPhysicalHeader(pConnToCardTableEntry->boxId, pConnToCardTableEntry->boardId, pConnToCardTableEntry->subBoardId,
									    pConnToCardTableEntry->unitId, pConnToCardTableEntry->portId, pConnToCardTableEntry->acceleratorId, ePhysical_mrmp);

	}

	return status;
}
///////////////////////////////////////////////////////////////////////////////
STATUS CMplApiSpecialCommandHandler::RtpFeccTokenResponseReq(void* param)
{

	STATUS status = m_MplMcmsProt.ValidateDataSize(	sizeof(TFeccTokenResponseReq));
	if(STATUS_OK != status)
	{
		return status;
	}

	TFeccTokenResponseReq tFeccTokenRspnsReq;
	memcpy(&tFeccTokenRspnsReq, m_MplMcmsProt.getpData(), sizeof(TFeccTokenResponseReq));

	APIU32 ConnectionId = tFeccTokenRspnsReq.tDestTerminalPhysicalRtp.connection_id;
	APIU32 DestMcuId = tFeccTokenRspnsReq.unDestMcuId;
	APIU32 DestTerminalId = tFeccTokenRspnsReq.unDestTerminalId;
	ConnToCardTableEntry connToCardTableEntry;
  	if(0xffffffff != ConnectionId && (0 < DestMcuId && 0 < DestTerminalId))
   	{
		status = m_pMplApiProcess->GetSharedMemoryMap()->Get(ConnectionId, connToCardTableEntry);
	   	if (STATUS_OK == status)
		{
        	SetPhysicalResourceInfo(&connToCardTableEntry, tFeccTokenRspnsReq.tDestTerminalPhysicalRtp.physical_id);
        	TRACEINTOFUNC<< "PartyId: " << tFeccTokenRspnsReq.tDestTerminalPhysicalRtp.party_id;

	   	}

   	}
  	else
  	{
  		tFeccTokenRspnsReq.tDestTerminalPhysicalRtp.party_id = INVALID;
  		TRACEINTOFUNC<< "PartyId: " << tFeccTokenRspnsReq.tDestTerminalPhysicalRtp.party_id;
  	}

	m_MplMcmsProt.AddData( sizeof(TFeccTokenResponseReq),(const char*) &tFeccTokenRspnsReq);

	return status;
}
///////////////////////////////////////////////////////////////////////////////

STATUS CMplApiSpecialCommandHandler::DtlsStartReq(void* param)
{
	STATUS status = m_MplMcmsProt.ValidateDataSize(	sizeof(TDtlsStartStruct));

	if(STATUS_OK != status)
	{
		return status;
	}

	TDtlsStartStruct dtlsStartReq;
	memcpy(&dtlsStartReq, m_MplMcmsProt.getpData(), sizeof(TDtlsStartStruct));

	APIU32 ConnectionId1 = dtlsStartReq.physicalPort.connection_id;

   	if (0xffffffff != ConnectionId1)
   	{
	  ConnToCardTableEntry connToCardTableEntry;
	  status = m_pMplApiProcess->GetSharedMemoryMap()->Get(ConnectionId1, connToCardTableEntry);
	  if (STATUS_OK == status)
	    {
	      SetPhysicalResourceInfo(&connToCardTableEntry,dtlsStartReq.physicalPort.physical_id);

	      //Identify the PSTN calls
	      PORT_DESCRIPTION_HEADER_S		rPortDescH;
	      m_MplMcmsProt.GetPortDescHeaderCopy(&rPortDescH);
	       //For IP only
		  connToCardTableEntry.physicalRsrcType=ePhysical_res_none;
		  connToCardTableEntry.unitId=0;
		  SetPhysicalInfoHeader(&connToCardTableEntry);
	   }
	}

   	m_MplMcmsProt.AddData( sizeof(TDtlsStartStruct),(const char*) &dtlsStartReq);

	return status;
}
///////////////////////////////////////////////////////////////////////////////

STATUS CMplApiSpecialCommandHandler::DtlsCloseReq(void* param)
{
	STATUS status = m_MplMcmsProt.ValidateDataSize(	sizeof(TDtlsCloseStruct));

	if(STATUS_OK != status)
	{
		return status;
	}

	TDtlsCloseStruct dtlsCloseReq;
	memcpy(&dtlsCloseReq, m_MplMcmsProt.getpData(), sizeof(TDtlsCloseStruct));

	APIU32 ConnectionId1 = dtlsCloseReq.physicalPort.connection_id;

   	if (0xffffffff != ConnectionId1)
   	{
	  ConnToCardTableEntry connToCardTableEntry;
	  status = m_pMplApiProcess->GetSharedMemoryMap()->Get(ConnectionId1, connToCardTableEntry);
	  if (STATUS_OK == status)
	    {
	      SetPhysicalResourceInfo(&connToCardTableEntry,dtlsCloseReq.physicalPort.physical_id);

	      //Identify the PSTN calls
	      PORT_DESCRIPTION_HEADER_S		rPortDescH;
	      m_MplMcmsProt.GetPortDescHeaderCopy(&rPortDescH);
	       //For IP only
		  connToCardTableEntry.physicalRsrcType=ePhysical_res_none;
		  connToCardTableEntry.unitId=0;
		  SetPhysicalInfoHeader(&connToCardTableEntry);
	   }
	}

   	m_MplMcmsProt.AddData( sizeof(TDtlsCloseStruct),(const char*) &dtlsCloseReq);

	return status;
}
///////////////////////////////////////////////////////////////////////////////

const ConnToCardTableEntry * CMplApiSpecialCommandHandler::GetMainARTphysicalInfoHeaderByConfAndPartyID(DWORD conf_id, DWORD party_id)
{
	const ConnToCardTableEntry *pConnToCardTableEntry = GetPhysicalInfoHeaderByConfAndPartyID(conf_id, party_id, ePhysical_art, eLogical_audio_decoder);
	if (pConnToCardTableEntry==NULL)
	{
		pConnToCardTableEntry = GetPhysicalInfoHeaderByConfAndPartyID(conf_id, party_id, ePhysical_art, eLogical_relay_audio_decoder);
		if (pConnToCardTableEntry==NULL)
			pConnToCardTableEntry=GetPhysicalInfoHeaderByConfAndPartyID(conf_id, party_id, ePhysical_art_light);
	}
	return pConnToCardTableEntry;
}

