
#include "ConfPartyMplMcmsProtocolTracer.h"
#include "ObjString.h"
// IP Opcodes
#include "IpCsOpcodes.h"
#include "IpMfaOpcodes.h"
// SIP Structures and CS API structures
#include "SipStructures.h"
#include "SipCsReq.h"
#include "SipCsInd.h"
#include "BfcpStructs.h"
// SIP general utils
#include "SipUtils.h"
#include "IpMfaOpcodes.h"
#include "IpMngrOpcodes.h"
#include "IpRtpInd.h"
#include "H323CapabiilityInfo.h"
#include "IpCsOpcodes.h"
#include "Macros.h"
#include "CapClass.h"
#include "CapInfo.h"
#include "H264Util.h"
#include "H323Caps.h"
#include "H323Control.h"
#include "IpCmInd.h"
#include "OpcodesMcmsVideo.h"
#include "OpcodesMcmsBonding.h"
#include "IceCmInd.h"
#include "IceCmReq.h"
#include "OpcodesMcmsCardMngrICE.h"
#include "TipUtils.h"
#include "OpcodesMcmsCardMngrBFCP.h"
#include "EnumsToStrings.h"

// extern functions
extern void DumpSdpAndHeadersToStream(CObjString* pMsgStr, const sipSdpAndHeadersSt& sdpAndHeaders);
extern void DumpXMLAndHeadersToStream(CObjString* pMsgStr, sipSdpAndHeadersSt& sdpAndHeaders);

extern const char* feccKeyToString(feccKeyEnum key);

//////////////////////////////////////////////////////////////////////////////////////////////
CConfPartyMplMcmsProtocolTracer::CConfPartyMplMcmsProtocolTracer(CMplMcmsProtocol &mplMcmsProt)
:CMplMcmsProtocolTracer(mplMcmsProt)
{
}

//////////////////////////////////////////////////////////////////////////////////////////////
CConfPartyMplMcmsProtocolTracer::~CConfPartyMplMcmsProtocolTracer()
{
}


//////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceContent(CObjString* pContentStr1, eProcessType processType)
{
//	CLargeString* pContentStr = (CLargeString*)pContentStr1;
	OPCODE opcode = m_pMplMcmsProt->getCommonHeaderOpcode();
	switch (opcode)
	{
	// IP party --> CS
	case H323_CS_SIG_GET_PORT_REQ: {
		TraceGetPortReq(pContentStr1);
		break;
	}
	case H323_CS_SIG_RELEASE_PORT_REQ: {
		TraceReleasePortReq(pContentStr1);
		break;
	}
	case H323_CS_SIG_CALL_SETUP_REQ: {
		TraceCallSetUpReq(pContentStr1);
		break;
	}
	case H323_CS_SIG_CALL_ANSWER_REQ: {
		TraceCallAnswerReq(pContentStr1);
		break;
	}
	case H323_CS_SIG_CREATE_CNTL_REQ:
	case H323_CS_SIG_RE_CAPABILITIES_REQ: {
		WORD flag = 0;
		if (opcode == H323_CS_SIG_CREATE_CNTL_REQ)
			flag = 1;
		TraceCreateControlReq(pContentStr1,flag);
		break;
	}
	case H323_CS_SIG_INCOMING_CHNL_RESPONSE_REQ: {
		TraceIncomingChannelResponseReq(pContentStr1);
		break;
	}
	case H323_CS_SIG_OUTGOING_CHNL_REQ: {
		TraceOutgoingChannelReq(pContentStr1);
		break;
	}
	case H323_CS_SIG_CHNL_NEW_RATE_REQ: {
		TraceChannelNewRateReq(pContentStr1);
		break;
	}
	case H323_CS_SIG_CHAN_MAX_SKEW_REQ: {
		TraceChannelMaxSkewReq(pContentStr1);
		break;
	}
	case H323_CS_SIG_CHANNEL_OFF_REQ: {
		TraceChannelOffReq(pContentStr1);
		break;
	}
	case H323_CS_SIG_MULTIPOINTMODECOM_TERMINALID_REQ: {
		TraceMultipointModeComTerminalIdMessageReq(pContentStr1);
		break;
	}
	case H323_CS_SIG_CHANNEL_ON_REQ: {
		TraceChannelOnReq(pContentStr1);
		break;
	}
	case H323_CS_SIG_ROLE_TOKEN_REQ: {
		TraceRoleTokenMessageReq(pContentStr1);
		break;
	}
	case H323_CS_SIG_CALL_DROP_TIMER_EXPIRED_REQ: {
		TraceCallDropTimerExpieredReq(pContentStr1);
		break;
	}
	case H323_CS_SIG_CHNL_DROP_REQ: {
		TraceChannelDropReq(pContentStr1);
		break;
	}
	case H323_CS_SIG_CALL_DROP_REQ: {
		TraceCallDropReq(pContentStr1);
		break;
	}
	case H323_CS_SIG_H323_CT_AUTHENTICATION_REQ: {
		TraceAuthenticationReq(pContentStr1);
		break;
	}
	case IP_CS_SIG_STOP_ALL_PROCESSES_REQ: {
		TraceStopAllProcessesReq(pContentStr1);
		break;
	}
	case H323_CS_SIG_CONFERENCE_RES_REQ: {
		TraceConferenceResponseReq(pContentStr1);
		break;
	}
	case H323_CS_SIG_CONFERENCE_IND_REQ: {
		TraceConferenceIndicationReq(pContentStr1);
		break;
	}
	case H323_CS_SIG_UNEXPECTED_MESSAGE_REQ: {
		TraceUnexpectedMessageReq(pContentStr1);
		break;
	}
	case H323_CS_SIG_DBC2_COMMAND_CT_ON_REQ:
	case H323_CS_SIG_DBC2_COMMAND_CT_OFF_REQ: {
		WORD flag = 0;
		if (opcode == H323_CS_SIG_DBC2_COMMAND_CT_ON_REQ)
			flag = 1;
		TraceDbc2OmmOnOffReq(pContentStr1,flag);
		break;
	}
	case H323_CS_DTMF_INPUT_REQ: {
		TraceDtmfInputReq(pContentStr1);
		break;
	}
	case H323_CS_FACILITY_REQ:{
        TraceFacilityReq(pContentStr1);
        break;
    }
	case H323_CS_SIG_LPR_MODE_CHANGE_REQ:{
		TraceLPRModeChangeReq(pContentStr1);
        break;
    }
	case H323_CS_SIG_NEW_ITP_SPEAKER_REQ:{         //added by Jason for ITP-Multiple channels
		TraceNewITPSpeakerReq(pContentStr1);
        break;
    }
	case H323_CS_SIG_RSS_CMD_REQ:{
		*pContentStr1 <<"\nCONTENT: H323_CS_SIG_RSS_CMD_REQ:";
        break;
    }

	// IP party <-- CS
	case H323_CS_SIG_GET_PORT_IND: {
		TraceGetPortInd(pContentStr1);
		break;
	}
	case H323_CS_SIG_CALL_DIAL_TONE_IND:
	case H323_CS_SIG_CALL_PROCEEDING_IND:
	case H323_CS_SIG_CALL_RING_BACK_IND: {
		TraceCallReportInd(pContentStr1, opcode);
		break;
	}
	case H323_CS_SIG_CALL_OFFERING_IND: {
		TraceCallOfferingInd(pContentStr1);
		break;
	}
	case H323_CS_SIG_CALL_CONNECTED_IND: {
		TraceCallConnectedInd(pContentStr1);
		break;
	}
	case H323_CS_SIG_CALL_NEW_RATE_IND: {
		TraceCallNewRateInd(pContentStr1);
		break;
	}
	case H323_CS_SIG_CALL_CNTL_CONNECTED_IND: {
		TraceCallCntlConnectedInd(pContentStr1);
		break;
	}
	case H323_CS_SIG_CAPABILITIES_IND: {
		TraceCapabilitiesInd(pContentStr1);
		break;
	}
	case H323_CS_FACILITY_IND: {
		TraceFacilityInd(pContentStr1);
		break;
	}
	case H323_CS_SIG_CALL_ROLE_TOKEN_IND: {
		TraceCallRoleTokenInd(pContentStr1);
		break;
	}
	case H323_CS_SIG_CALL_BAD_SPONTAN_IND: {
		TraceBadSpontanInd(pContentStr1);
		break;
	}
	case H323_CS_SIG_CALL_AUTHENTICATION_IND: {
		TraceAuthenticationInd(pContentStr1);
		break;
	}
	case H323_CS_SIG_INCOMING_CHANNEL_IND: {
		TraceIncomingChannelInd(pContentStr1);
		break;
	}
	case H323_CS_SIG_INCOMING_CHNL_CONNECTED_IND: {
		TraceIncomingChannelConnectedInd(pContentStr1);
		break;
	}
	case H323_CS_SIG_OUTGOING_CHNL_RESPONSE_IND: {
		TraceOutgoingChannelResponseInd(pContentStr1);
		break;
	}
	case H323_CS_SIG_CHAN_NEW_RATE_IND: {
		TraceChannelNewRateInd(pContentStr1);
		break;
	}
	case H323_CS_SIG_START_CHANNEL_CLOSE_IND: {
		TraceStartChannelCloseInd(pContentStr1);
		break;
	}
	case H323_CS_SIG_CHANNEL_CLOSE_IND: {
		TraceChannelCloseInd(pContentStr1);
		break;
	}
	case H323_CS_SIG_CHAN_MAX_SKEW_IND: {
		TraceChannelMaxSkewInd(pContentStr1);
		break;
	}
	case H323_CS_SIG_FLOW_CONTROL_IND_IND: {
		TraceFlowControlIndInd(pContentStr1);
		break;
	}
	case H323_CS_SIG_DBC2_COMMAND_CT_ON_IND:
	case H323_CS_SIG_DBC2_COMMAND_CT_OFF_IND: {
		WORD flag = 0;
		if (opcode == H323_CS_SIG_DBC2_COMMAND_CT_ON_IND)
			flag = 1;
		TraceDbc2OmmOnOffInd(pContentStr1,flag);
		break;
	}
	case H323_CS_NON_STANDARD_REQ_IND:
	case H323_CS_NON_STANDARD_COM_IND:
	case H323_CS_NON_STANDARD_RES_IND:
	case H323_CS_NON_STANDARD_IND_IND: {
		TraceNonStandardInd(pContentStr1,opcode);
		break;
	}
	case H323_CS_CONFERENCE_REQ_IND: {
		TraceConfReqInd(pContentStr1);
		break;
	}
	case H323_CS_CONFERENCE_COM_IND: {
		TraceConfComInd(pContentStr1);
		break;
	}
	case H323_CS_CONFERENCE_IND_IND: {
		TraceConfIndInd(pContentStr1);
		break;
	}
	case H323_CS_SIG_LPR_MODE_CHANGE_IND: {
		TraceLPRModeChangeInd(pContentStr1);
		break;
	}
	//added by Jason for ITP-Multiple channels
	case H323_CS_SIG_NEW_ITP_SPEAKER_IND: {
		TraceNewITPSpeakerInd(pContentStr1);
		break;
	}

/*	case H323_CS_PARTY_KEEP_ALIVE_IND: {
		TracePartyKeepAliveInd(pContentStr);
		break;
	}*/
	// SIP Party -> CS
	case SIP_CS_SIG_INVITE_REQ: {
		TraceSipInviteReq(pContentStr1);
		break;
	}
	case SIP_CS_SIG_INVITE_ACK_REQ: {
		TraceSipInviteAckReq(pContentStr1);
		break;
	}
	case SIP_CS_SIG_INVITE_RESPONSE_REQ: {
		TraceSipInviteResponseReq(pContentStr1);
		break;
	}
	case SIP_CS_SIG_BYE_REQ: {
		TraceSipByeReq(pContentStr1);
		break;
	}
	case SIP_CS_SIG_BYE_200_OK_REQ: {
		TraceSipBye200OkReq(pContentStr1);
		break;
	}
	case SIP_CS_SIG_RINGING_REQ: {
		TraceSipRingingReq(pContentStr1);
		break;
	}
	case SIP_CS_SIG_CANCEL_REQ: {
		TraceSipCancelReq(pContentStr1);
		break;
	}
	case SIP_CS_SIG_REINVITE_REQ: {
		TraceSipReInviteReq(pContentStr1);
		break;
	}
	case SIP_CS_SIG_INFO_REQ: {
		TraceSipInfoUnionReq(pContentStr1);
		break;
	}

	case SIP_CS_SIG_INFO_RESP_REQ: {
		TraceSipInfoResReq(pContentStr1);
		break;
	}
	// SIP CS --> Party
	case SIP_CS_SIG_INVITE_IND: {
		TraceSipInviteInd(pContentStr1);
		break;
	}
	case  SIP_CS_CCCP_SIG_INVITE_IND: {
		TraceSipCCCPInviteInd(pContentStr1);
		break;
	}
	case  SIP_CS_CCCP_SIG_INVITE_REQ: {
			TraceSipCCCPInviteReq(pContentStr1);
			break;
	}
	case SIP_CS_SIG_INVITE_ACK_IND: {
		TraceSipInviteAckInd(pContentStr1);
		break;
	}
	case SIP_CS_SIG_INVITE_RESPONSE_IND: {
		TraceSipInviteResponseInd(pContentStr1);
		break;
	}
	case SIP_CS_SIG_REINVITE_IND: {
		TraceSipReInviteInd(pContentStr1);
		break;
	}
	case SIP_CS_SIG_CANCEL_IND: {
		TraceSipCancelInd(pContentStr1);
		break;
	}
	case SIP_CS_SIG_BYE_IND: {
		TraceSipByeInd(pContentStr1);
		break;
	}
	case SIP_CS_SIG_BYE_200_OK_IND: {
		TraceSipBye200OkInd(pContentStr1);
		break;
	}
	case SIP_CS_SIG_SESSION_TIMER_EXPIRED_IND:{
		TraceSipSessionTimerExpiredInd(pContentStr1);
		break;
	}
	case SIP_CS_SIG_SESSION_TIMER_REINVITE_IND:{
		TraceSipSessionTimerReinviteInd(pContentStr1);
		break;
	}
	case SIP_CS_SIG_BAD_STATUS_IND: {
		TraceSipBadStatusInd(pContentStr1);
		break;
	}
	case SIP_CS_SIG_TRACE_INFO_IND: {
		TraceSipTraceInfoInd(pContentStr1);
		break;
	}
	case SIP_CS_SIG_INFO_IND: {
		TraceSipInfoUnionInd(pContentStr1);
		break;
	}
	case SIP_CS_SIG_INFO_RESP_IND: {
		TraceSipInfoRespInd(pContentStr1);
		break;
	}
	case SIP_CS_SIG_REFER_IND: {
		TraceSipReferInd(pContentStr1);
		break;
	}
	case SIP_CS_BFCP_MESSAGE_REQ: {
		TraceSipBFCPMsgReq(pContentStr1);
		break;
	}
  case SIP_CS_SIG_DIALOG_RECOVERY_IND: {
    TraceSipDialogRecoveryInd(pContentStr1);
    break;
  }
  case SIP_CS_SIG_DIALOG_RECOVERY_REQ: {
    TraceSipDialogRecoveryReq(pContentStr1);
    break;
  }
  case SIP_CS_SIG_SEND_CRLF_REQ: {
    TraceSipSendCrlfReq(pContentStr1);
    break;
  }
  case SIP_CS_SIG_SOCKET_ACTIVITY_REQ:
  case SIP_CS_SIG_SOCKET_ACTIVITY_IND:
  {
    TraceSipSocketActivityReqInd(pContentStr1);
    break;
  }

  case SIP_CS_SIG_CRLF_ERR_IND:
  {
    TraceCrlfErrInd(pContentStr1);
    break;
  }
  case SIP_CS_PROXY_SUBSCRIBE_REQ:
  {
	  TraceSubscribeReq(pContentStr1);
	  break;
  }

	//RTP:
	case H320_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ: {
		TraceRtpUpdatePortOpenChannelReq(pContentStr1,"H320_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ");
		break;
	}
	case H320_RTP_UPDATE_CHANNEL_REQ: {
	    TraceRtpUpdatePortOpenChannelReq(pContentStr1,"H320_RTP_UPDATE_CHANNEL_REQ"); //TraceRtpUpdateChannelReq()
		break;
	}
	case H320_RTP_UPDATE_CHANNEL_RATE_REQ: {
		TraceRtpUpdateChannelRateReq(pContentStr1,"H320_RTP_UPDATE_CHANNEL_RATE_REQ");
		break;
	}
/*	case H320_MUX_CONTENT_ON_REQ: {
		TraceRtpContentOnOffReq(pContentStr1, "H320_MUX_CONTENT_ON_REQ");
		break;
	}
	case H320_MUX_CONTENT_OFF_REQ: {
		TraceRtpContentOnOffReq(pContentStr1, "H320_MUX_CONTENT_OFF_REQ");
		break;
	}
	case H320_MUX_EVACUATE_REQ: {
		TraceRtpEvacuateStreamReq(pContentStr1, "H320_MUX_EVACUATE_REQ");
		break;
	}
*/
	case H323_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ: {
		TraceRtpUpdatePortOpenChannelReq(pContentStr1,"H323_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ (SIP_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ also use this opcode)");
		break;
	}
	case SIP_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ: {
		TraceRtpUpdatePortOpenChannelReq(pContentStr1,"SIP_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ");
		break;
	}
	case H323_RTP_UPDATE_CHANNEL_REQ: {
		TraceRtpUpdateChannelReq(pContentStr1,"H323_RTP_UPDATE_CHANNEL_REQ (SIP_RTP_UPDATE_CHANNEL_REQ also use this opcode)");
		break;
	}
	case SIP_RTP_UPDATE_CHANNEL_REQ: {
		TraceRtpUpdateChannelReq(pContentStr1,"SIP_RTP_UPDATE_CHANNEL_REQ");
		break;
	}
	case H323_RTP_UPDATE_CHANNEL_RATE_REQ: {
		TraceRtpUpdateChannelRateReq(pContentStr1,"H323_RTP_UPDATE_CHANNEL_RATE_REQ (SIP_RTP_UPDATE_CHANNEL_RATE_REQ also use this opcode)");
		break;
	}
	case SIP_RTP_UPDATE_CHANNEL_RATE_REQ: {
		TraceRtpUpdateChannelRateReq(pContentStr1,"SIP_RTP_UPDATE_CHANNEL_RATE_REQ");
		break;
	}
	case H323_RTP_FECC_TOKEN_RESPONSE_REQ:{
		TraceRtpFeccTokenResponseReq(pContentStr1);
		break;
	}
	case H323_RTP_UPDATE_MT_PAIR_REQ:{
		TraceUpdateMtPairReq(pContentStr1);
		break;
	}
	case H323_RTP_STREAM_ON_REQ:{
		TraceRtpStreamOnReq(pContentStr1, "H323_RTP_STREAM_ON_REQ (SIP_RTP_STREAM_ON_REQ also use this opcode)");
		break;
	}
	case H323_RTP_STREAM_OFF_REQ:{
		TraceRtpStreamOffReq(pContentStr1, "H323_RTP_STREAM_OFF_REQ (SIP_RTP_STREAM_OFF_REQ also use this opcode)");
		break;
	}
	case CONFPARTY_CM_OPEN_UDP_PORT_REQ: {
		TraceCmOpenUdpPortOrUpdateUdpAddrReq(pContentStr1, "H323_CM_OPEN_UDP_PORT_REQ (SIP_CM_OPEN_UDP_PORT_REQ also use this opcode)");
		break;
	}
	case SIP_CM_OPEN_UDP_PORT_REQ: {
		TraceCmOpenUdpPortOrUpdateUdpAddrReq(pContentStr1, "SIP_CM_OPEN_UDP_PORT_REQ");
		break;
	}
	case CONFPARTY_CM_UPDATE_UDP_ADDR_REQ: {
		TraceCmOpenUdpPortOrUpdateUdpAddrReq(pContentStr1, "H323_CM_UPDATE_UDP_ADDR_REQ (SIP_CM_UPDATE_UDP_ADDR_REQ also use this opcode)");
		break;
	}
	case SIP_CM_UPDATE_UDP_ADDR_REQ: {
		TraceCmOpenUdpPortOrUpdateUdpAddrReq(pContentStr1, "SIP_CM_UPDATE_UDP_ADDR_REQ");
		break;
	}
	case CONFPARTY_CM_CLOSE_UDP_PORT_REQ: {
		TraceCmCloseUdpPortReq(pContentStr1, "CONFPARTY_CM_CLOSE_UDP_PORT_REQ (SIP_CM_CLOSE_UDP_PORT_REQ also use this opcode)");
		break;
	}
	case SIP_CM_CLOSE_UDP_PORT_REQ: {
		TraceCmCloseUdpPortReq(pContentStr1, "SIP_CM_CLOSE_UDP_PORT_REQ");
		break;
	}
/*	case H323_RTP_CONTENT_ON_REQ:
	case H323_RTP_CONTENT_OFF_REQ: {
		if (m_pMplMcmsProt->getCommonHeaderOpcode() == H323_RTP_CONTENT_ON_REQ)
			TraceRtpContentOnOffReq(pContentStr1, "H323_RTP_CONTENT_ON_REQ");
		else
			TraceRtpContentOnOffReq(pContentStr1, "H323_RTP_CONTENT_OFF_REQ");
		break;
	}
	case H323_RTP_EVACUATE_REQ: {
		TraceRtpEvacuateStreamReq(pContentStr1, "H323_RTP_EVACUATE_REQ");
		break;
	}
*/
	case ART_CONTENT_ON_REQ:
	case ART_CONTENT_OFF_REQ: {
		if (m_pMplMcmsProt->getCommonHeaderOpcode() == ART_CONTENT_ON_REQ)
			TraceRtpContentOnOffReq(pContentStr1, "ART_CONTENT_ON_REQ");
		else
			TraceRtpContentOnOffReq(pContentStr1, "ART_CONTENT_OFF_REQ");
		break;
	}
	case ART_EVACUATE_REQ: {
		TraceRtpEvacuateStreamReq(pContentStr1, "ART_EVACUATE_REQ");
		break;
	}
	case H323_RTP_LPR_MODE_CHANGE_REQ:
	case H323_RTP_LPR_MODE_RESET_REQ: {
		if (m_pMplMcmsProt->getCommonHeaderOpcode() == H323_RTP_LPR_MODE_CHANGE_REQ)
			TraceRtpLprModeChangeReq(pContentStr1, "H323_RTP_LPR_MODE_CHANGE_REQ (SIP_RTP_LPR_MODE_CHANGE_REQ also use this opcode)");
		else
			TraceRtpLprModeChangeReq(pContentStr1, "H323_RTP_LPR_MODE_RESET_REQ (SIP_RTP_LPR_MODE_RESET_REQ also use this opcode)");
		break;
	}
	case IP_CM_START_PREVIEW_CHANNEL:
	{
		TraceStartPartyPreviewReq(pContentStr1);
		break;
	}
	case IP_CM_STOP_PREVIEW_CHANNEL:
	{
		TraceStopPartyPreviewReq(pContentStr1);
		break;
	}

	//RTP -> IP party:
	case IP_RTP_FECC_TOKEN_IND:{
		TraceIpRtpFeccTokenInd(pContentStr1);
		break;
	}
	case IP_RTP_FECC_KEY_IND:{
		TraceIpRtpFeccKeyInd(pContentStr1);
		break;
	}
	case IP_RTP_DIFF_PAYLOAD_TYPE_IND:{
		TraceIpRtpDifferentPayloadTypeInd(pContentStr1);
		break;
	}
	case IP_RTP_FECC_MUTE_IND:{
		TraceIpRtpFeccMuteInd(pContentStr1);
		break;
	}
	case IP_RTP_VIDEO_UPDATE_PIC_IND:{
		TraceIpRtpVideoFastUpdateInd(pContentStr1);
		break;
	}
	case IP_RTP_BAD_SPONTAN_IND:{
		TraceIpRtpBadSpontaneousInd(pContentStr1);
		break;
	}
	case IP_RTP_STREAM_STATUS_IND:{
		TraceIpRtpStreamStatusInd(pContentStr1);
		break;
	}
	case IP_CM_PARTY_MONITORING_IND:{
		CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
		BOOL isPartyMntrEnable = 0;
		std::string key = "ENABLE_IP_PARTY_MONITORING";
		pSysConfig->GetBOOLDataByKey(key, isPartyMntrEnable);
		if (isPartyMntrEnable)
			TraceIpRtpCmMonitoringInd(pContentStr1, "IP_CM_PARTY_MONITORING_IND");
		break;
	}
	case CONF_PARTY_MRMP_PARTY_MONITORING_IND:{
		TraceIpRtpCmMonitoringInd(pContentStr1, "CONF_PARTY_MRMP_PARTY_MONITORING_IND");
		break;
	}
	case IP_CM_PARTY_VIDEO_CHANNELS_STATISTICS_IND:{ //CDR_MCCF
	    TraceIpRtpCmVideoChannelsStatisticsInd(pContentStr1);
	    break;
	}
	case IP_CM_RTCP_MSG_IND:{
		TraceIpCmRtcpMsgInd(pContentStr1);
		break;
	}
	case IP_CM_RTCP_MSG_REQ:{
		TraceIpCmRtcpMsgReq(pContentStr1);
		break;
	}
	case IP_RTP_ASK_ENCODER_FOR_INTRA_IND:{
		*pContentStr1 <<"\nCONTENT: IP_RTP_ASK_ENCODER_FOR_INTRA_IND:";
		break;
	}
	case H320_RTP_VIDEO_UPDATE_PIC_IND:{
		*pContentStr1 <<"\nCONTENT: H320_RTP_VIDEO_UPDATE_PIC_IND:";
		break;
	}
	case BND_CONNECTION_INIT:{
        TraceBondingInitReq(pContentStr1);
        break;
	}
	case BND_ADD_CHANNEL:{
        TraceBondingAddChannelReq(pContentStr1);
        break;
	}
	case  BND_END_NEGOTIATION:{
        TraceBondingEndNegotiationInd(pContentStr1);
        break;
	}
	case  BND_REMOTE_LOCAL_ALIGNMENT:{
        TraceBondingRemoteLocalAlignmentInd(pContentStr1);
        break;
	}
    case BND_REQ_PARAMS:{
        TraceBondingRequesParamsInd(pContentStr1);
        break;
    }
    case BND_ACK_PARAMS:{
        TraceResponseBondingAckParamsReq(pContentStr1);
        break;
    }
    case IP_RTP_LPR_MODE_CHANGE_IND:{
		TraceIpRtpLprModeChangeInd(pContentStr1);
		break;
	}
    case IP_RTP_DBA_IND:{
		TraceIpRtpDbaInd(pContentStr1);
		break;
	}
	// video bridge to DSP
	//case VIDEO_GRAPHICS_SHOW_TEXT_BOX_REQ:{
	//	TraceGraphicsShowTextBoxParams(pContentStr1);
	//	break;
	//}
    case ICE_MAKE_ANSWER_IND:
    case ICE_MAKE_OFFER_IND:
    case ICE_REINVITE_IND:
    case ICE_MODIFY_SESSION_OFFER_IND:
    case ICE_MODIFY_SESSION_ANSWER_IND:
    case ICE_BANDWIDTH_EVENT_IND:{
        TraceIceGeneralIndication (pContentStr1);
        break;
    }

    case ICE_MAKE_OFFER_REQ:
    case ICE_PROCESS_ANSWER_REQ:
    case ICE_MAKE_ANSWER_REQ:
    case ICE_CLOSE_SESSION_REQ:
    case ICE_MODIFY_SESSION_ANSWER_REQ:
    case ICE_MODIFY_SESSION_OFFER_REQ:{
       TraceIceGeneralRequest (pContentStr1);
       break;
    }

    case ICE_PROCESS_ANSWER_IND:
    case ICE_CLOSE_SESSION_IND:
    case ICE_INSUFFICIENT_BANDWIDTH_IND:
    case ICE_SESSION_INDEX_IND:
    {
    	TraceIceAckIndication(pContentStr1);
    	break;
    }
    case CM_SEND_CNAME_INFO_AS_STRING_IND:
    {
    	*pContentStr1 <<"\nCONTENT: CM_SEND_CNAME_INFO_AS_STRING_IND:";
    	break;
    }

    case ICE_ERR_IND:
    {
    	TraceIceErrIndication(pContentStr1);
    	break;
    }
    case IP_CM_RTCP_VIDEO_PREFERENCE_IND:
    {
      	TraceRtcpVideoPreferencesInd(pContentStr1);
       	break;
    }
    case IP_CM_RTCP_VIDEO_PREFERENCE_REQ:
    {
    	TraceRtcpVideoPreferencesReq(pContentStr1);
    	break;
    }
    case IP_CM_RTCP_RECEIVER_BANDWIDTH_REQ:
    {
      	TraceRtcpReceiverBandwidthReq(pContentStr1);
       	break;
    }
    case IP_CM_RTCP_RECEIVER_BANDWIDTH_IND:
    {
      	TraceRtcpReceiverBandwidthInd(pContentStr1);
      	break;
    }
    case IP_CM_BFCP_MESSAGE_REQ:
	{
		TraceBfcpMessageReq(pContentStr1);
		break;
	}
    case IP_CM_BFCP_MESSAGE_IND:
	{
		TraceBfcpMessageInd(pContentStr1);
		break;
	}
    case IP_CM_RTCP_PACKET_LOSS_STATUS_IND:
	{
		TracePacketLossStatusInd(pContentStr1);
		break;
	}
    case CONFPARTY_CM_INIT_ON_LYNC_CALL_REQ:
	{
		TraceMsSvcP2PInitReq(pContentStr1);
		break;
	}
    case CONFPARTY_CM_INIT_ON_AVMCU_CALL_REQ:
	{
		TraceMsSvcAvMcuInitReq(pContentStr1);
		break;
	}
    case CONFPARTY_CM_MUX_ON_AVMCU_CALL_REQ:
	{
		TraceMsSvcAvMcuMuxReq(pContentStr1);
		break;
	}
    case CONFPARTY_CM_DMUX_ON_AVMCU_CALL_REQ:
	{
		TraceMsSvcAvMcuDmuxReq(pContentStr1);
		break;
	}
    case RTP_PARTY_MONITORING_AV_MCU_REQ:
   	{
   		TraceMsSvcAvMcuMontioringReq(pContentStr1);
   		break;
   	}
    case IP_CM_RTCP_MS_SVC_PLI_REQ:
	{
		TraceMsSvcPliMessageReq(pContentStr1);
		break;
	}
    case IP_CM_RTCP_MS_SVC_PLI_IND:
	{
		TraceMsSvcPliMessageInd(pContentStr1);
		break;
	}
    case IP_CM_RTCP_VSR_REQ:
	{
		TraceVsrMessageReq(pContentStr1);
		break;
	}
    case IP_CM_RTCP_VSR_IND:
	{
		TraceVsrMessageInd(pContentStr1);
		break;
	}
    case IP_CM_BFCP_TCP_TRANSPORT_IND:
	{
		TraceBfcpTcpTransportMessageInd(pContentStr1);
		break;
	}
    case IP_CM_RTCP_RTPFB_IND:
    {
    	TraceRtcpRtpFeedbackInd(pContentStr1);
    	break;
    }
    case IP_CM_RTCP_RTPFB_REQ:
    {
    	TraceRtcpRtpFeedbackReq(pContentStr1);
    	break;
    }
    case ART_FEC_RED_ON_OFF_REQ:
    {
    	TraceRtpFecRedOnOffReq(pContentStr1);
    	break;
    }
    case IP_CM_DTLS_START_REQ:
    {
    	TraceDtlsStartReq(pContentStr1, "IP_CM_DTLS_START_REQ");
    	break;
    }
    case IP_CM_DTLS_CLOSE_REQ:
	{
		TraceDtlsStartReq(pContentStr1, "IP_CM_DTLS_CLOSE_REQ");
		break;
	}
    case IP_CM_DTLS_END_IND:
	{
		TraceDtlsEndInd(pContentStr1);
		break;
	}
 case  IP_CM_MEDIA_DISCONNECTED_IND:
 	{
		TraceIpMediaDetectionInd(pContentStr1);
		break;
 	}
	// DSP to video bridge
	default:	{
    	CMplMcmsProtocolTracer::TraceContent(pContentStr1, processType);
			break;
		}
	}
}


////////////////////////////////////////////////////////////////////////////////////////////
// Sip
////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceSipInviteReq(CObjString* pContentStr)
{
	mcReqInvite* p = (mcReqInvite*)m_pMplMcmsProt->getpData();

	*pContentStr << "\nCONTENT: SIP_CS_SIG_INVITE_REQ: \n " ;
    *pContentStr << "Sip Invite Request:" << "\n";

	*pContentStr << "Is Outbound Proxy In Use: ";
	if (p->bIsOutboundProxyInUse == NO)
		*pContentStr << "NO (Remote Address)\n";
	else
		*pContentStr << "YES (Proxy Address)\n";

	*pContentStr << "Is AVMCU: ";
		if (p->bIsAVMCU == NO)
			*pContentStr << "NO \n";
		else
			*pContentStr << "YES \n";

	if (p->domainName[0] != '\0')
		*pContentStr << "Domain Name: " <<  p->domainName << "\n";

	//add for CG_SoftMCU
	if (p->callGeneratorParams.bIsCallGenerator == 1)
	{
		*pContentStr << "bIsCallGenerator: " <<  p->callGeneratorParams.bIsCallGenerator  << "\n";
		*pContentStr << "eEndpointModel: " <<  p->callGeneratorParams.eEndpointModel << "\n";
	}

	TraceTransportAddrSt(pContentStr, p->transportAddress);

	::DumpSdpAndHeadersToStream(pContentStr,p->sipSdpAndHeaders);
}
////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceSubscribeReq(CObjString* pContentStr)
{
	mcReqSubscribe* p = (mcReqSubscribe*)m_pMplMcmsProt->getpData();

		*pContentStr << "\nCONTENT: SIP_CS_PROXY_SUBSCRIBE_REQ: \n " ;
	    *pContentStr << "Sip subscribe Request:" << "\n";

		*pContentStr << "Is Outbound Proxy In Use: ";



			*pContentStr << "id: " <<  p->id << "\n";
			*pContentStr << "expires: " <<  p->expires << "\n";
			//*pContentStr << "subOpcode: " <<  p->subOpcode << "\n";
			 *pContentStr << "subOpcode: " <<  p->subOpcode << "\n";
			 if (p->domainName[0] != '\0')
			 		*pContentStr << "Domain Name: " <<  p->domainName << "\n";


		TraceTransportAddrSt(pContentStr, p->transportAddr);

	//	::DumpSdpAndHeadersToStream(pContentStr,p->sipHeaders);
		sipMessageHeaders* pHeaders = (sipMessageHeaders *)(&(p->sipHeaders));
		CSipHeaderList headers(*pHeaders);
		headers.DumpToStream(pContentStr);
		*pContentStr << "\n";

}
////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceSipInviteAckReq(CObjString* pContentStr)
{
	*pContentStr << "\nCONTENT: SIP_CS_SIG_INVITE_ACK_REQ: \n " ;
    *pContentStr << "Sip Invite ACK Request" <<"\n";
}
////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceSipInviteResponseReq(CObjString* pContentStr)
{
	mcReqInviteResponse* p = (mcReqInviteResponse*)m_pMplMcmsProt->getpData();

	*pContentStr << "\nCONTENT: SIP_CS_SIG_INVITE_RESPONSE_REQ: \n "   ;

    *pContentStr << "Sip Invite Response Request" <<"\n";
	*pContentStr << "Response : ";
    if (p)
    {
	if (p->status)
		*pContentStr << ::GetRejectReasonStr((enSipCodes)p->status) << "\n";
	else
		*pContentStr << p->status << "\n";

	*pContentStr << "Is Focus: " <<  p->bIsFocus << "\n";

	//add for CG_SoftMCU
	if (p->callGeneratorParams.bIsCallGenerator == 1)
	{
		*pContentStr << "bIsCallGenerator: " <<  p->callGeneratorParams.bIsCallGenerator  << "\n";
		*pContentStr << "eEndpointModel: " <<  p->callGeneratorParams.eEndpointModel << "\n";
	}
	::DumpSdpAndHeadersToStream(pContentStr, p->sipSdpAndHeaders);
    }
}
////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceSipByeReq(CObjString* pContentStr)
{
	*pContentStr << "\nCONTENT: SIP_CS_SIG_BYE_REQ: \n "    ;
    *pContentStr << "Sip Bye Request" <<"\n";
}
////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceSipBye200OkReq(CObjString* pContentStr)
{
	*pContentStr << "\nCONTENT: SIP_CS_SIG_BYE_200_OK_REQ: \n "  ;
    *pContentStr << "Sip Bye 200 Ok Request" <<"\n";
}

////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceSipRingingReq(CObjString* pContentStr)
{
 	mcReqRinging* p = (mcReqRinging*)m_pMplMcmsProt->getpData();

	*pContentStr << "\nCONTENT: SIP_CS_SIG_RINGING_REQ: \n "     ;
    *pContentStr << "Sip Ringing Request" <<"\n";
	sipMessageHeaders* pHeaders = (sipMessageHeaders *)(&(p->sipHeaders));
	CSipHeaderList headers(*pHeaders);
	headers.DumpToStream(pContentStr);
	*pContentStr << "\n";
}

////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceSipCancelReq(CObjString* pContentStr)
{
	*pContentStr << "\nCONTENT: SIP_CS_SIG_CANCEL_REQ: \n "    ;
    *pContentStr << "Sip cancel Request" <<"\n";
}

////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceSipReInviteReq(CObjString* pContentStr)
{
	*pContentStr << "\nCONTENT: SIP_CS_SIG_REINVITE_REQ: \n " ;
    *pContentStr << "Sip Re-Invite Request" <<"\n";

	mcReqReInvite* p = (mcReqReInvite*) m_pMplMcmsProt->getpData();
	::DumpSdpAndHeadersToStream(pContentStr,p->sipSdpAndHeaders);
}
////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceSipBFCPMsgReq(CObjString* pContentStr)
{
	mcReqBfcpMessage * pMsgReq = (mcReqBfcpMessage *)m_pMplMcmsProt->getpData();
	*pContentStr << "\nCONTENT: SIP_CS_BFCP_MESSAGE_REQ: \n ";
	*pContentStr << "Status: " << pMsgReq->status <<"\n " ;
	*pContentStr << "Length: " << pMsgReq->length <<"\n " ;
	APIS8 pOutBinaryBFCPMsg[64];
	memset(pOutBinaryBFCPMsg, 0, 64);
	/*memcpy(pOutBinaryBFCPMsg, &pMsgReq->buffer[0], pMsgReq->length );
	char s[16];
	for(int i=0; i<(int)pMsgReq->length; i++)
	{
	   sprintf(s, "0x%x", pOutBinaryBFCPMsg[i]);
	   *pContentStr << s << " ";
	}*/

}
////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceSipDialogRecoveryInd(CObjString* pContentStr)
{
  mcIndDialogRecovery * pStruct = (mcIndDialogRecovery *)m_pMplMcmsProt->getpData();
  *pContentStr << "\nCONTENT: SIP_CS_SIG_DIALOG_RECOVERY_IND: \n "    ;
  *pContentStr << "SIP Dialog Recovery Indication" << "\n";
  *pContentStr << "Status: " << pStruct->status <<"\n";
}
////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceSipDialogRecoveryReq(CObjString* pContentStr)
{
  mcReqDialogRecovery * pStruct = (mcReqDialogRecovery *)m_pMplMcmsProt->getpData();
  *pContentStr << "\nCONTENT: SIP_CS_SIG_DIALOG_RECOVERY_REQ: \n "    ;
  *pContentStr << "SIP Dialog Recovery Request" << "\n";
  *pContentStr << "Status: " << pStruct->status <<"\n";
}
////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceSipSendCrlfReq(CObjString* pContentStr)
{
  mcReqSendCrlf * pStruct = (mcReqSendCrlf *)m_pMplMcmsProt->getpData();
  *pContentStr << "\nCONTENT: SIP_CS_SIG_SEND_CRLF_REQ: \n "    ;
  *pContentStr << "SIP Send Crlf Request" << "\n";
  *pContentStr << "MsKepAliveTimeOut_Sec: " << pStruct->dwMsKepAliveTimeOut_Sec <<"\n";
}

////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceSipSocketActivityReqInd(CObjString* pContentStr)
{
  mcReqIndSocketStatistics * pStruct = (mcReqIndSocketStatistics *)m_pMplMcmsProt->getpData();
  *pContentStr << "\nCONTENT: SIP_CS_SIG_SOCKET_ACTIVITY_REQ(SIP_CS_SIG_SOCKET_ACTIVITY_IND): \n "    ;
  *pContentStr << "SIP Send/Recv Socket activity" << "\n";
  *pContentStr << "dwSilenceTimeRecv_mSec: " << pStruct->dwSilenceTimeRecv_mSec <<"\n";
  *pContentStr << "dwSilenceTimeSend_mSec: " << pStruct->dwSilenceTimeSend_mSec <<"\n";
}

////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceCrlfErrInd(CObjString* pContentStr)
{
  mcIndSigCrlfError * pStruct = (mcIndSigCrlfError *)m_pMplMcmsProt->getpData();
  *pContentStr << "\nCONTENT: SIP_CS_SIG_CRLF_ERR_IND: \n "    ;
  *pContentStr << "SIP CRLF error indication" << "\n";
  *pContentStr << "Error code: " << pStruct->eCrlfSendingErrorCode <<"\n";
}


////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceSipInfoUnionReq(CObjString* pContentStr)
{
	mcReqInfo * pUnion = (mcReqInfo *)m_pMplMcmsProt->getpData();
	*pContentStr << "\nCONTENT: SIP_CS_SIG_INFO_REQ: \n "  ;
	switch (pUnion->subOpcode)
	{
		case VideoFastUpdate:
		{
			*pContentStr << "\nACTION: Video Fast Update: \n "  ;
			break;
		}
		case VideoFastUpdateV2StreamId:
		{
			*pContentStr << "\nACTION: Video Fast Update V2 stream_id: \n "  ;
			mcReqVideoFastUpdateV2 * pStruct = (mcReqVideoFastUpdateV2 *)pUnion->buff;
			*pContentStr << "Label " << pStruct->label << "\n";
			break;
		}
		case VideoFastUpdate_v2:
		{
			*pContentStr << "\nACTION: Video Fast Update V2: \n "  ;
			mcReqVideoFastUpdateV2 * pStruct = (mcReqVideoFastUpdateV2 *)pUnion->buff;
			/*switch (pStruct->videoType)
			{
				case mainType:
				{
					*pContentStr << "Main video" << "\n";
					break;
				}
				case slideType:
				{
					*pContentStr << "Content video" << "\n";
					break;
				}
				default:
					*pContentStr << "Wrong video type" << "\n";

			}*/
			*pContentStr << "Label " << pStruct->label << "\n";
			break;
		}
		case FlowControl:
		{
			*pContentStr << "\nACTION: Flow control: \n "  ;
			mcReqFlowControl* pStruct = (mcReqFlowControl*)pUnion->buff;
			/*switch (pStruct->videoType)
			{
				case mainType:
				{
					*pContentStr << "Main video" << "\n";
					break;
				}
				case slideType:
				{
					*pContentStr << "Content video" << "\n";
					break;
				}
				default:
					*pContentStr << "Wrong video type" << "\n";

			}*/
			*pContentStr << "Label " << pStruct->label << "\n";

			switch (pStruct->mediaDirection)
			{
				case kInactive:
				{
					*pContentStr << "Media Direction: Inactive" << "\n";
					break;
				}
				case kRecvOnly:
				{
					*pContentStr << "Media Direction: RecvOnly" << "\n";
					break;
				}
				case kSendOnly:
				{
					*pContentStr << "Media Direction: SendOnly" << "\n";
					break;
				}
				case kSendRecv:
				{
					*pContentStr << "Media Direction: SendRecv" << "\n";
					break;
				}
				default:
					*pContentStr << "Wrong media direction" << "\n";
			}
			*pContentStr << "Rate :" << pStruct->rate << "\n";
			break;

		}
		case LprChangeMode:
		{
			*pContentStr << "\nACTION: Lpr Mode Change : \n "  ;
			mcReqSipLPRModeChange* pStruct = (mcReqSipLPRModeChange*)pUnion->buff;
			*pContentStr << "lossProtection :" << pStruct->lossProtection << "\n";
			*pContentStr << "mtbf :" << pStruct->mtbf << "\n";
			*pContentStr << "congestionCeiling :" << pStruct->congestionCeiling << "\n";
			*pContentStr << "fill :" << pStruct->fill << "\n";
			*pContentStr << "modeTimeout :" << pStruct->modeTimeout << "\n";

			break;
		}
		default:
			*pContentStr << "Unknown Info union indication" << "\n";

	}


}
/////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceSipInviteInd(CObjString* pContentStr)
{
	*pContentStr << "\nCONTENT: SIP_CS_SIG_INVITE_IND: \n "  ;
    *pContentStr << "SIP Invite Indication" <<"\n";

	mcIndInvite* p = (mcIndInvite*) m_pMplMcmsProt->getpData();
    *pContentStr << "Status: " << p->status << "\n";
    *pContentStr << "Is Focus: " << p->bIsFocus << "\n";
	::DumpSdpAndHeadersToStream(pContentStr,p->sipSdpAndHeaders);
}
/////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceSipCCCPInviteInd(CObjString* pContentStr)
{
	*pContentStr << "\nCONTENT: SIP_CS_CCCP_SIG_INVITE_IND: \n "  ;
	    *pContentStr << "SIP Invite Indication" <<"\n";

		mcIndInvite* p = (mcIndInvite*) m_pMplMcmsProt->getpData();
	    *pContentStr << "Status: " << p->status << "\n";

		::DumpXMLAndHeadersToStream(pContentStr,p->sipSdpAndHeaders);
}

/////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceSipCCCPInviteReq(CObjString* pContentStr)
{
	*pContentStr << "\nCONTENT: SIP_CS_CCCP_SIG_INVITE_REQ: \n "  ;
	    *pContentStr << "Sip Invite Request" <<"\n";


	    mcReqInvite* p = (mcReqInvite*)m_pMplMcmsProt->getpData();

	   	*pContentStr << "Is Outbound Proxy In Use: ";
	   	if (p->bIsOutboundProxyInUse == NO)
	   		*pContentStr << "NO (Remote Address)\n";
	   	else
	   		*pContentStr << "YES (Proxy Address)\n";

	   	*pContentStr << "Is AVMCU: ";
	   		if (p->bIsAVMCU == NO)
	   				*pContentStr << "NO \n";
	   		else
	   				*pContentStr << "YES \n";


	   	if (p->domainName[0] != '\0')
	   		*pContentStr << "Domain Name: " <<  p->domainName << "\n";

	   	//add for CG_SoftMCU
	   	if (p->callGeneratorParams.bIsCallGenerator == 1)
	   	{
	   		*pContentStr << "bIsCallGenerator: " <<  p->callGeneratorParams.bIsCallGenerator  << "\n";
	   		*pContentStr << "eEndpointModel: " <<  p->callGeneratorParams.eEndpointModel << "\n";
	   	}

	   	TraceTransportAddrSt(pContentStr, p->transportAddress);
	   	::DumpXMLAndHeadersToStream(pContentStr,p->sipSdpAndHeaders);


}
/////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceSipInviteAckInd(CObjString* pContentStr)
{
	mcIndInviteAck * pStruct = (mcIndInviteAck *)m_pMplMcmsProt->getpData();
	*pContentStr << "\nCONTENT: SIP_CS_SIG_INVITE_ACK_IND: \n "   ;
    *pContentStr << "SIP Invite ACK Indication" <<"\n";
	*pContentStr << "Status: " << pStruct->status << "\n";

	::DumpSdpAndHeadersToStream(pContentStr,pStruct->sipSdpAndHeaders);
}
/////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceSipInviteResponseInd(CObjString* pContentStr)
{
	mcIndInviteResponse * pStruct = (mcIndInviteResponse *)m_pMplMcmsProt->getpData();
	*pContentStr << "\nCONTENT: SIP_CS_SIG_INVITE_RESPONSE_IND: \n "   ;
	*pContentStr << "SIP Invite Response Indication" <<"\n";
	*pContentStr << "Response : ";
	if (pStruct->status)
		*pContentStr << ::GetRejectReasonStr((enSipCodes)pStruct->status) << "\n";
	else
		*pContentStr << pStruct->status << "\n";

	::DumpSdpAndHeadersToStream(pContentStr,pStruct->sipSdpAndHeaders);
}

/////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceSipReInviteInd(CObjString* pContentStr)
{
	mcIndReInvite * pStruct = (mcIndReInvite *)m_pMplMcmsProt->getpData();
	*pContentStr << "\nCONTENT: SIP_CS_SIG_REINVITE_IND: \n "   ;
	*pContentStr << "SIP Re-Invite Indication" <<"\n";
	::DumpSdpAndHeadersToStream(pContentStr,pStruct->sipSdpAndHeaders);
}

/////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceSipCancelInd(CObjString* pContentStr)
{
	mcIndCancel * pStruct = (mcIndCancel *)m_pMplMcmsProt->getpData();
	*pContentStr << "\nCONTENT: SIP_CS_SIG_CANCEL_IND: \n " ;
	*pContentStr << "SIP Cancel Indication" <<"\n";
	if (pStruct->status)
		*pContentStr << ::GetRejectReasonStr((enSipCodes)pStruct->status) << "\n";
	else
		*pContentStr << pStruct->status << "\n";
}

/////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceSipByeInd(CObjString* pContentStr)
{
	mcIndBye * pStruct = (mcIndBye *)m_pMplMcmsProt->getpData();
	*pContentStr << "\nCONTENT: SIP_CS_SIG_BYE_IND: \n "    ;
	*pContentStr << "SIP Bye Indication" << "\n";
	*pContentStr << "Status: " << pStruct->status <<"\n";
}

/////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceSipBye200OkInd(CObjString* pContentStr)
{
	mcIndBye200Ok * pStruct = (mcIndBye200Ok *)m_pMplMcmsProt->getpData();
	*pContentStr << "\nCONTENT: SIP_CS_SIG_BYE_200_OK_IND: \n "   ;
	*pContentStr << "SIP 200 Ok Bye Indication" << "\n";
	*pContentStr << "Status: " << pStruct->status <<"\n";
}

/////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceSipBadStatusInd(CObjString* pContentStr)
{
	mcIndBadStatus * pStruct = (mcIndBadStatus *)m_pMplMcmsProt->getpData();
	*pContentStr << "\nCONTENT: SIP_CS_SIG_BAD_STATUS_IND: \n "    ;
	*pContentStr << "SIP Bad Status Indication" << "\n";
	*pContentStr << "Opcode: " << pStruct->FailedrequestOpcode << "\n";
	*pContentStr << "Error: " << pStruct->sErrMsg << "\n";
}

/////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceSipSessionTimerExpiredInd(CObjString* pContentStr)
{
	*pContentStr << "\nCONTENT: SIP_CS_SIG_SESSION_TIMER_EXPIRED_IND: \n "   ;
    *pContentStr << "SIP Session Timer Expired Indication" <<"\n";
}

/////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceSipSessionTimerReinviteInd(CObjString* pContentStr)
{
	*pContentStr << "\nCONTENT: SIP_CS_SIG_SESSION_TIMER_REINVITE_IND: \n "   ;
    *pContentStr << "SIP Session Timer Reinvite Indication" <<"\n";
}

/////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceSipTraceInfoInd(CObjString* pContentStr)
{
	mcIndTraceInfo * pStruct = (mcIndTraceInfo *)m_pMplMcmsProt->getpData();
	*pContentStr << "\nCONTENT: SIP_CS_SIG_TRACE_INFO_IND: \n "  ;
	*pContentStr << "SIP Trace info Indication" << "\n";
	*pContentStr << "Error: " << pStruct->sErrMsg << "\n";
}

/////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceSipInfoUnionInd(CObjString* pContentStr)
{
	mcIndInfo * pUnion = (mcIndInfo *)m_pMplMcmsProt->getpData();
	*pContentStr << "\nCONTENT: SIP_CS_SIG_INFO_IND: \n "  ;
	switch (pUnion->subOpcode)
	{
		case VideoFastUpdate:
		{
			*pContentStr << "\nACTION: Video Fast Update: \n "  ;
			break;
		}
		case VideoFastUpdateV2StreamId:
		{
			*pContentStr << "\nACTION: Video Fast Update V2 stream_id: \n "  ;
			mcIndVideoFastUpdateV2 * pStruct = (mcIndVideoFastUpdateV2 *)pUnion->buff;
			*pContentStr << "Label is " << pStruct->label << "\n";
			break;
		}
		case VideoFastUpdate_v2:
		{
			*pContentStr << "\nACTION: Video Fast Update V2: \n "  ;
			mcIndVideoFastUpdateV2 * pStruct = (mcIndVideoFastUpdateV2 *)pUnion->buff;
			*pContentStr << "Label is " << pStruct->label << "\n";
			/*switch (pStruct->videoType)
			{
				case mainType:
				{
					*pContentStr << "Main video" << "\n";
					break;
				}
				case slideType:
				{
					*pContentStr << "Content video" << "\n";
					break;
				}
				default:
					*pContentStr << "Unknown video type" << "\n";

			}*/
			break;
		}
		case FlowControl:
		{
			*pContentStr << "\nACTION: Flow control: \n "  ;
			mcIndFlowControl* pStruct = (mcIndFlowControl*)(&(pUnion->buff));
			*pContentStr << "Label is " << pStruct->label << "\n";
			/*switch (pStruct->videoType)
			{
				case mainType:
				{
					*pContentStr << "Main video" << "\n";
					break;
				}
				case slideType:
				{
					*pContentStr << "Content video" << "\n";
					break;
				}
				default:
					*pContentStr << "Unknown video type" << "\n";

			}*/
			switch (pStruct->mediaDirection)
			{
				case kInactive:
				{
					*pContentStr << "Media Direction: Inactive" << "\n";
					break;
				}
				case kRecvOnly:
				{
					*pContentStr << "Media Direction: RecvOnly" << "\n";
					break;
				}
				case kSendOnly:
				{
					*pContentStr << "Media Direction: SendOnly" << "\n";
					break;
				}
				case kSendRecv:
				{
					*pContentStr << "Media Direction: SendRecv" << "\n";
					break;
				}
				default:
					*pContentStr << "Unknown media direction" << "\n";
			}
			*pContentStr << "Rate :" << pStruct->rate << "\n";
			break;

		}
		case LprChangeMode:
		{
			*pContentStr << "\nACTION: Lpr Mode Change : \n "  ;
			mcIndSipLPRModeChange* pStruct = (mcIndSipLPRModeChange*)pUnion->buff;
			*pContentStr << "lossProtection :" << pStruct->lossProtection << "\n";
			*pContentStr << "mtbf :" << pStruct->mtbf << "\n";
			*pContentStr << "congestionCeiling :" << pStruct->congestionCeiling << "\n";
			*pContentStr << "fill :" << pStruct->fill << "\n";
			*pContentStr << "modeTimeout :" << pStruct->modeTimeout << "\n";

			break;
		}
		default:
			*pContentStr << "Unknown Info union indication" << "\n";

	}


}
/////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceSipInfoRespInd(CObjString* pContentStr)
{
	*pContentStr << "\nCONTENT: SIP_CS_SIG_INFO_RESP_IND: \n "  ;
}
/////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceSipReferInd(CObjString* pContentStr)
{
	mcIndRefer * pStruct = (mcIndRefer *)m_pMplMcmsProt->getpData();
	*pContentStr << "\nCONTENT: SIP_CS_SIG_REFER_IND: \n "  ;
	*pContentStr << "Sip Refer Indication" <<"\n";
	sipMessageHeaders* pHeaders = (sipMessageHeaders *)(&(pStruct->sipHeaders));
	CSipHeaderList headers(*pHeaders);
	headers.DumpToStream(pContentStr);
	*pContentStr << "\n";
}

void CConfPartyMplMcmsProtocolTracer::TraceSipInfoResReq(CObjString* pContentStr)
{
	*pContentStr << "\nCONTENT: SIP_CS_SIG_INFO_RESP_REQ: \n "  ;
}
/// End SIP trace functions
//////////////////////////////////////////////////////////////////////////////////
// Ip trace functions


////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceGetPortReq(CObjString* pContentStr)
{
	mcReqGetPort getPortReq;
	memcpy(&getPortReq, m_pMplMcmsProt->getpData(),sizeof(mcReqGetPort));

	ALLOCBUFFER(szAddress, 20);
//	SystemDWORDToIpString(getPortReq.srcIpAddress, szAddress);


	*pContentStr << "\nCONTENT: H323_CS_SIG_GET_PORT_REQ: \n "
		<< "Source Ip Address:	";

//	pContentStr->SetFormat("%x");
	*pContentStr <<  getPortReq.srcIpAddress << '\n';
//	pContentStr->SetFormat("%d");
	DEALLOCBUFFER(szAddress);

}

////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceReleasePortReq(CObjString* pContentStr)
{
	mcReqReleasePort releasePortReq;
	memcpy(&releasePortReq, m_pMplMcmsProt->getpData(),sizeof(mcReqReleasePort));


	*pContentStr << "\nCONTENT: H323_CS_SIG_RELEASE_PORT_REQ: \n "
		<< "Port	: "
		<< releasePortReq.port << '\n'
		<< "HaCall	: "
		<< (DWORD)releasePortReq.haCall
		<< '\n';

}

////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceCallSetUpReq(CObjString* pContentStr)
{
	mcReqCallSetup *pCallSetUpReq;
	pCallSetUpReq = (mcReqCallSetup*)new BYTE[m_pMplMcmsProt->getDataLen()];

	// When Encryption is on we will need to recalculate the size of the struct

	DWORD lengthStructure = m_pMplMcmsProt->getDataLen();
//	lengthStructure += (sizeof(encTokensHeaderStruct) - sizeof(encTokensHeaderBasicStruct));
	memcpy(pCallSetUpReq,(BYTE*)m_pMplMcmsProt->getpData(),lengthStructure);
	mcXmlTransportAddress		srcIpAddress;							// Changed for Ipv6.
	mcXmlTransportAddress		destIpAddress;							// Changed for Ipv6.
	APIS8						domainName[MaxDomainNameSize];			// Changed for Ipv6.

	*pContentStr << "\nCONTENT: H323_CS_SIG_CALL_SETUP_REQ: \n";
// IpV6 - Temp
	*pContentStr << "Source Party Address		: \n";
	TraceTransportAddrSt(pContentStr, pCallSetUpReq->srcIpAddress);
	*pContentStr << "Destination Party Address	: \n";
	TraceTransportAddrSt(pContentStr, pCallSetUpReq->destIpAddress);
	*pContentStr << "Source Party Aliases		: "	<< pCallSetUpReq->srcPartyAliases << '\n';
	*pContentStr << "Destination party Aliases	: "	<< pCallSetUpReq->destPartyAliases << '\n';
	*pContentStr << "Conference Id				: ";

	PrintHexNum(pContentStr, pCallSetUpReq->conferenceId, Size16);
	*pContentStr << "Call Id			            : ";
	PrintHexNum(pContentStr, pCallSetUpReq->callId, Size16);

	*pContentStr << "Referene value				: "   << pCallSetUpReq->referenceValue << '\n';
	*pContentStr << "Max rate					: "   << pCallSetUpReq->maxRate << '\n';
	*pContentStr << "Min rate					: "   << pCallSetUpReq->minRate << '\n';

	*pContentStr << "Call Transient				: ";
	TraceMcCallTransient(pContentStr,pCallSetUpReq->callTransient);
	*pContentStr << "\n";

	*pContentStr << "Local EP type				: ";
	::GetEndpointTypeName(pCallSetUpReq->localEndpointType, *pContentStr);
	*pContentStr << '\n';

	*pContentStr << "Type						: ";
	::GetCallTypeName(pCallSetUpReq->type, *pContentStr);
	*pContentStr << '\n';

	*pContentStr << "Conference type				: " << pCallSetUpReq->conferenceType << '\n'
				 << "Is active MC				: " << pCallSetUpReq->bIsActiveMc << '\n';

	*pContentStr << "Call goal					: ";
	::GetConferenceGoalTypeName(pCallSetUpReq->callGoal, *pContentStr);
	*pContentStr << '\n';


	*pContentStr << "Avaya						: \n";
	TraceH460AvayaFeVndrReqSt(pContentStr, pCallSetUpReq->avfFeVndIdReq);
	*pContentStr << '\n';


	if (pCallSetUpReq->destExtraCallInfo[0] != '\0')
	{
		*pContentStr << "Destination extra call info: \n" << pCallSetUpReq->destExtraCallInfo << '\n';
		int i = 0;
		for (i = 0 ; i < MaxNumberOfAliases ; i++)
		if (pCallSetUpReq->destExtraCallInfoTypes[i])
		{
			::GetAliasTypeName(pCallSetUpReq->destExtraCallInfoTypes[i], *pContentStr);
			*pContentStr << ", " ;
		}
		*pContentStr << "\n";
	}
	*pContentStr << "HaCall						: " << (DWORD)pCallSetUpReq->haCall << '\n';
	if (pCallSetUpReq->remoteExtensionAddress[0] != '\0')
		*pContentStr << "Remote extention address   : \n" << pCallSetUpReq->remoteExtensionAddress << '\n';
	*pContentStr << "Authentication				: " << pCallSetUpReq->bAuthentication << '\n'
				 << "Sid[0]						: " << (int)pCallSetUpReq->sid[0] << '\n'
				 << "Sid[1]						: " << (int)pCallSetUpReq->sid[1] << '\n'
				 << "Authentication key        	: ";
		CLargeString trace;
		trace.SetFormat("%x");
		int i = 0;
		for (i=0 ; i < sizeOf128Key ; i++)
		 trace << "0x" << (BYTE)pCallSetUpReq->authKey[i];
	*pContentStr << trace.GetString() << '\n';
	 pContentStr->SetFormat("%d");
	*pContentStr << "EP Identification length    : " << pCallSetUpReq->epIdentLength << '\n'
				 << "EP identification		    : " << pCallSetUpReq->endpointIdent << '\n';
	//add for CG_SoftMCU
	if (pCallSetUpReq->callGeneratorParams.bIsCallGenerator == 1)
	{
		*pContentStr << "bIsCallGenerator: " <<  pCallSetUpReq->callGeneratorParams.bIsCallGenerator  << "\n";
		*pContentStr << "eEndpointModel: " <<  pCallSetUpReq->callGeneratorParams.eEndpointModel << "\n";
	}
	*pContentStr << "Encryption session:\n"
				 << "===================\n";

	TraceEncTokensHeaderStruct(pContentStr, &(pCallSetUpReq->encryTokens));
	PDELETEA(pCallSetUpReq);

}

////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceMcCallTransient(CObjString* callTransientString, mcCallTransient callTransient)
{
	*callTransientString << "Display size			: " << callTransient.sDisplaySize << '\n'
		<< "Display					: " << callTransient.sDisplay << '\n'
		<< "UserUser size			: " << callTransient.userUserSize << '\n'
		<< "UserUser				: " << callTransient.userUser;
}

////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceH460AvayaFeVndrReqSt(CObjString* h460AvayaString, h460AvayaFeVndrReqSt avfFeVndIdReq)
{
	*h460AvayaString << "FsId						: " << avfFeVndIdReq.fsId;
}
////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceH460AvayaFeVndrIndSt(CObjString	*h460AvayaString, h460AvayaFeVndrIndSt avfFeVndIdInd)
{
	*h460AvayaString << "\nFsId                    : " << avfFeVndIdInd.fsId << '\n'
					 << "Country code			: " << avfFeVndIdInd.countryCode << '\n'
					 << "T35 Extention			: " << avfFeVndIdInd.t35Extension << '\n'
					 << "Manufacturer code		: " << avfFeVndIdInd.manfctrCode << '\n'
					 << "Product Id				: " << avfFeVndIdInd.productId << '\n'
					 << "Version Id				: " << avfFeVndIdInd.versionId << '\n'
					 << "Enterprise number		: " << avfFeVndIdInd.enterpriseNum << '\n';

}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceEncTokensHeaderStruct(CObjString* encryptionTokenString, encTokensHeaderStruct *encryTokens)
{
	if (encryTokens->numberOfTokens > 0)
	{
		*encryptionTokenString << "Number of tokens        : " << encryTokens->numberOfTokens << '\n'
			<<"Dynamic token length    : " << encryTokens->dynamicTokensLen << '\n';

		int i = 0;

		for (i = 0 ; i < encryTokens->numberOfTokens ; i++)
		{
			*encryptionTokenString << "Token number			: " << i << '\n'
					 				<< "Generator				: " << ((encryptionToken*)encryTokens->token)->generator << '\n'
					 				<< "Half key length			: " << ((encryptionToken*)encryTokens->token)->hkLen << '\n'
					 				<< "Half key type			: " << ((encryptionToken*)encryTokens->token)->tokenOID << '\n';
			/*CLargeString trace;
			trace.SetFormat("%x");
			int j = 0;
			for (j = 0 ; j < 128 ; j++)
			{//
				trace <<  "0x";
				trace << (BYTE)((encryptionToken*)encryTokens->token)->halfKey[i];
			}
			*encryptionTokenString << "Half key: " << trace.GetString() << '\n';*/
			*encryptionTokenString<< "Half Key				:";
			encryptionTokenString->SetFormat("%x");
			for(int i=0; i<128; i++)
			{
				*encryptionTokenString << " 0x";
				//*pContentStr<<(BYTE)pStruct.aucSessionKey[i];
				*encryptionTokenString << (BYTE)(((encryptionToken*)encryTokens->token)->halfKey[i]);
			}
			*encryptionTokenString << "\n";
		}
	}
	else
		*encryptionTokenString << "Not Encrypted \n";

}

////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceCallAnswerReq(CObjString* pContentStr)
{
	mcReqCallAnswer *pCallAnswerReq;
	pCallAnswerReq = (mcReqCallAnswer*)new BYTE[m_pMplMcmsProt->getDataLen()];
	APIS32 status = m_pMplMcmsProt->getCentralSignalingHeaderStatus();
	DWORD lengthStructure = m_pMplMcmsProt->getDataLen();
	memcpy(pCallAnswerReq,(BYTE*)m_pMplMcmsProt->getpData(),lengthStructure);

	*pContentStr << "\nCONTENT: H323_CS_SIG_CALL_ANSWER_REQ: \n ";
	if(status == STATUS_OK)
	{

		*pContentStr << "Remote Party Address		: \n";
		TraceTransportAddrSt(pContentStr, pCallAnswerReq->remoteAddress);

		*pContentStr << "H245 transport address		: \n";
		TraceTransportAddrSt(pContentStr, pCallAnswerReq->h245Address);
		*pContentStr << "Conference Id			: ";
		PrintHexNum(pContentStr, pCallAnswerReq->conferenceId, Size16);
		*pContentStr << "Call transient			: \n";
		TraceMcCallTransient(pContentStr, pCallAnswerReq->callTransient);
		*pContentStr << "\n";

		*pContentStr << "Local Ep type			: ";
		::GetEndpointTypeName(pCallAnswerReq->localEndpointType, *pContentStr);
		*pContentStr << '\n';
		*pContentStr << "Connected party Alias(RMX alias): "	<< pCallAnswerReq->ConnectedAddressAliases << '\n';

		*pContentStr << "Max rate				: " << pCallAnswerReq->maxRate << '\n';
		*pContentStr << "Conference type			: " << pCallAnswerReq->conferenceType << '\n'
					<< "Authentication			: " << pCallAnswerReq->bAuthentication << '\n'
					<< "Sid[0]					: " << (int)pCallAnswerReq->sid[0] << '\n'
					<< "Sid[1]					: " << (int)pCallAnswerReq->sid[1] << '\n'
					<< "Authentication key		: ";
		CLargeString trace;
		trace.SetFormat("%x");
		int i = 0;
		for (i=0 ; i < sizeOf128Key ; i++)
			trace << "0x" << (BYTE)pCallAnswerReq->authKey[i];
		*pContentStr << trace.GetString() << '\n';
		pContentStr->SetFormat("%d");

		*pContentStr << "HaCall					: " << (DWORD)pCallAnswerReq->haCall << '\n'
					<< "EP Identification length: " << pCallAnswerReq->epIdentLength << '\n'
					<< "EP identification		: " << pCallAnswerReq->endpointIdent << '\n';
		//add for CG_SoftMCU
		if (pCallAnswerReq->callGeneratorParams.bIsCallGenerator == 1)
		{
			*pContentStr << "bIsCallGenerator: " <<  pCallAnswerReq->callGeneratorParams.bIsCallGenerator  << "\n";
			*pContentStr << "eEndpointModel: " <<  pCallAnswerReq->callGeneratorParams.eEndpointModel << "\n";
		}
		*pContentStr<< "Encryption				: \n";
		TraceEncTokensHeaderStruct(pContentStr, &(pCallAnswerReq->encryTokens));
		*pContentStr<< '\n';
	}
	else
	{
		*pContentStr << "Reject call reason		: ";
		::GetRejectReasonTypeName(pCallAnswerReq->rejectCallReason, *pContentStr);
		*pContentStr << '\n';
	}
	PDELETEA(pCallAnswerReq);
}



////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceCreateControlReq(CObjString* pContentStr, WORD flag)
{
	const int SLAVE_NUMBER = 160;
	mcReqCreateControl *pCreateCntlReq;
	pCreateCntlReq = (mcReqCreateControl*) new BYTE[m_pMplMcmsProt->getDataLen()];
	DWORD lengthStructure = m_pMplMcmsProt->getDataLen();
	memcpy(pCreateCntlReq, (BYTE*) m_pMplMcmsProt->getpData(), lengthStructure);

	//ALLOCBUFFER (fullBuffer,100000);
	if (flag == 1)
		*pContentStr << "\nCONTENT: H323_CS_SIG_CREATE_CNTL_REQ:\n";
	else
		*pContentStr << "\nCONTENT: H323_CS_SIG_RE_CAPABILITIES_REQ:\n";

	*pContentStr << "Transport address:\n";
	TraceTransportAddrSt(pContentStr, pCreateCntlReq->h245IpAddress);
	*pContentStr << "\nMaster/Slave type       : " << pCreateCntlReq->masterSlaveTerminalType;

	if (pCreateCntlReq->masterSlaveTerminalType > SLAVE_NUMBER)
		*pContentStr << " MASTER \n";
	else
		*pContentStr << " SLAVE \n";

	*pContentStr << '\n';
	COstrStream msg;

	msg << "Capabilities: " << "\n";
	msg << "=============" << "\n";

	ctCapabilitiesStruct* pCapabilitiesStruct = &(pCreateCntlReq->capabilities);
	capBuffer* pCapBuffer = (capBuffer*)&(pCapabilitiesStruct->caps);
	char* tempPtr = (char*)pCapBuffer;

	for (int i = 0; i < pCapabilitiesStruct->numberOfCaps; i++)
	{
		CCapSetInfo capInfo = (CapEnum)pCapBuffer->capTypeCode;
		msg << "\n----- ";
		capInfo.Dump(msg);
		msg << " -----";

		CBaseCap * pCap = CBaseCap::AllocNewCap(capInfo, pCapBuffer->dataCap);
		if (pCap)
			pCap->Dump(msg);
		PDELETE(pCap);

		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*) tempPtr;
	}
	msg << "=========================" << "\n";
	*pContentStr << msg.str().c_str();

	PDELETEA(pCreateCntlReq);
}

////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceIncomingChannelResponseReq(CObjString* pContentStr)
{
	mcReqIncomingChannelResponse *pIncomingChannelResponse;
	pIncomingChannelResponse = (mcReqIncomingChannelResponse*)new BYTE[m_pMplMcmsProt->getDataLen()];
	DWORD lengthStructure = m_pMplMcmsProt->getDataLen();
	memcpy(pIncomingChannelResponse,(BYTE*)m_pMplMcmsProt->getpData(),lengthStructure);

//	CLargeString* transportAddr = new CLargeString;
//	TraceTransportAddrSt(transportAddr, pIncomingChannelResponse->localRtpAddressIp);

	*pContentStr << "\nCONTENT: H323_CS_SIG_INCOMING_CHNL_RESPONSE_REQ: \n "
		<< "Channel index				: " << pIncomingChannelResponse->channelIndex << '\n'
		<< "Data type					: " << pIncomingChannelResponse->dataType << '\n'
		<< "Channel direction			: " << pIncomingChannelResponse->channelDirection << '\n'
		<< "Local RTP address Ip		: \n";
	TraceTransportAddrSt(pContentStr, pIncomingChannelResponse->localRtpAddressIp);
	*pContentStr << "\nDynamic payload type		: " << pIncomingChannelResponse->dynamicPayloadType << '\n'
		<< "Is encrypted				: " << pIncomingChannelResponse->bIsEncrypted << '\n'
		<< "Encryption algorithm		: " << pIncomingChannelResponse->encryptionAlgorithm << '\n';
	/*
	if (pIncomingChannelResponse->bIsEncrypted)
	{
		CLargeString trace;
		trace.SetFormat("%x");
		trace << "0x";
		int j = 0;
		for (j = 0 ; j < 16 ; j++)
		{
			trace << (BYTE)pIncomingChannelResponse->EncryptedSession235Key[j];
		}
		*pContentStr  << "Encryption session 235 key  : " << trace.GetString() << '\n';
	}
	*/

	PDELETEA(pIncomingChannelResponse);
}

////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceOutgoingChannelReq(CObjString* pContentStr)
{
	mcReqOutgoingChannel *pOutgoingchannel;
	pOutgoingchannel = (mcReqOutgoingChannel*)new BYTE[m_pMplMcmsProt->getDataLen()];
	DWORD lengthStructure = m_pMplMcmsProt->getDataLen();
	memcpy(pOutgoingchannel,(BYTE*)m_pMplMcmsProt->getpData(),lengthStructure);



	*pContentStr << "\nCONTENT: H323_CS_SIG_OUTGOING_CHNL_REQ: \n "
		<< "Source RTP address			: ";
	TraceTransportAddrSt(pContentStr, pOutgoingchannel->SourceRtpAddressIp);
	*pContentStr << "\nPayload type				: " << pOutgoingchannel->payloadType << '\n'
		<< "Channel type				: " << pOutgoingchannel->channelType << '\n'
		<< "Channel index				: " << pOutgoingchannel->channelIndex << '\n'
		<< "Channel direction			: " << pOutgoingchannel->channelDirection << '\n'
		<< "Channel name				: " << pOutgoingchannel->channelName << '\n'
		<< "Cap type code				: " << pOutgoingchannel->capTypeCode << '\n'
		<< "Dynamic payload type		: " << pOutgoingchannel->dynamicPayloadType << '\n'
		<< "Is encrypted				: " << pOutgoingchannel->bIsEncrypted << '\n'
		<< "Encryption algorithm		: " << pOutgoingchannel->encryptionAlgorithm << '\n'
		<< "Is LPR						: " << pOutgoingchannel->bIsLPR << '\n';
	/*
	if (pOutgoingchannel->bIsEncrypted)
	{
		CLargeString trace;
		trace.SetFormat("%x");
		trace << "0x";
		int j = 0;
		for (j = 0 ; j < 16 ; j++)
		{
			trace << (BYTE)pOutgoingchannel->EncryptedSession235Key[j];
		}
		*pContentStr << "Encryption session 235 key for outgoing : " << trace.GetString() << '\n';
	}
	*/
	*pContentStr << "Size of channel params		: " << pOutgoingchannel->sizeOfChannelParams << "\n\n";

	COstrStream msg;
	msg << " Channel Parameters:"<< "\n";
	msg << "=====================" << "\n";

	CCapSetInfo capInfo = (CapEnum)pOutgoingchannel->capTypeCode;
	capInfo.Dump(msg);
	msg << "\n";

	CBaseCap * pCap = CBaseCap::AllocNewCap(capInfo,(BYTE*)(pOutgoingchannel->channelSpecificParams));
	if (pCap)
		pCap->Dump(msg);
	PDELETE(pCap);


	msg << "=========================" << "\n";
	*pContentStr << msg.str().c_str();

	PDELETEA(pOutgoingchannel);


}
////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceChannelNewRateReq(CObjString* pContentStr)
{
	mcReqChannelNewRate channelNewRate;
	memcpy(&channelNewRate, m_pMplMcmsProt->getpData(),sizeof(mcReqChannelNewRate));


	*pContentStr << "\nCONTENT: H323_CS_SIG_CHAN_NEW_RATE_REQ: \n "
		<< "Channel type		: " << channelNewRate.channelType << '\n'
		<< "Channel index		: " << channelNewRate.channelIndex << '\n'
		<< "Channel direction	: " << channelNewRate.channelDirection << '\n'
		<< "Rate				: " << channelNewRate.rate << '\n';

}
////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceChannelMaxSkewReq(CObjString* pContentStr)
{
	mcReqChannelMaxSkew channelMaxSkew;
	memcpy(&channelMaxSkew, m_pMplMcmsProt->getpData(),sizeof(mcReqChannelMaxSkew));


	*pContentStr << "\nCONTENT: H323_CS_SIG_CHAN_MAX_SKEW_REQ: \n "
		<< "Second channel index	: " << channelMaxSkew.secondChannelIndex << '\n'
		<< "Channel type			: " << channelMaxSkew.channelType << '\n'
		<< "Channel index			: " << channelMaxSkew.channelIndex << '\n'
		<< "Channel direction		: " << channelMaxSkew.channelDirection << '\n'
		<< "Skew					: " << channelMaxSkew.skew << '\n';


}
////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceChannelOffReq(CObjString* pContentStr)
{
	mcReqChannelOff channelOff;
	memcpy(&channelOff, m_pMplMcmsProt->getpData(),sizeof(mcReqChannelOff));


	*pContentStr << "\nCONTENT: H323_CS_SIG_CHANNEL_OFF_REQ: \n "
		<< "Channel type		: " << channelOff.channelType << '\n';
}

////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceMultipointModeComTerminalIdMessageReq(CObjString* pContentStr)
{
	mcReqMultipointModeComTerminalIDMessage terminalIdReq;
	memcpy(&terminalIdReq, m_pMplMcmsProt->getpData(),sizeof(mcReqMultipointModeComTerminalIDMessage));

	*pContentStr << "\nCONTENT: H323_CS_SIG_MULTIPOINTMODECOM_TERMINALID_REQ: \n "
				 << "Mcu Id		: " << terminalIdReq.mcuID << '\n'
				 << "Terminal Id: " << terminalIdReq.terminalID << '\n';
}

////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceChannelOnReq(CObjString* pContentStr)
{
	mcReqChannelOn channelOnReq;
	memcpy(&channelOnReq, m_pMplMcmsProt->getpData(),sizeof(mcReqChannelOn));


	*pContentStr << "\nCONTENT: H323_CS_SIG_CHANNEL_ON_REQ: \n "
		<< "Channel type		: " << channelOnReq.channelType << '\n';
}
////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceRoleTokenMessageReq(CObjString* pContentStr)
{
	mcReqRoleTokenMessage roleTokenReq;
	memcpy(&roleTokenReq, m_pMplMcmsProt->getpData(),sizeof(mcReqRoleTokenMessage));


	*pContentStr << "\nCONTENT: H323_CS_SIG_ROLE_TOKEN_REQ: \n "
	    << "Role Token" << '\n';
	BYTE bIsH239 = IsRoleTokenOpcodeH239Type((ERoleTokenOpcode)roleTokenReq.subOpcode);
    if (bIsH239)
		*pContentStr << "H239 message" << '\n';
    else
		*pContentStr << "People Plus Content message" << '\n';

	*pContentStr << "Sub opcode           :" << GetRoleTokenOpcodeStr((ERoleTokenOpcode)roleTokenReq.subOpcode) << '\n'
		<< "Mcu ID               :" << roleTokenReq.mcuID << '\n'
		<< "Terminal ID          :" << roleTokenReq.terminalID << '\n'
        << "Rand Number          :" << roleTokenReq.randNumber << '\n'
        << "Content provider info:" << roleTokenReq.contentProviderInfo << '\n'
	    << "Bit Rate			 :" << roleTokenReq.bitRate << '\n'
	    << "Is Ack				 :" << (int)roleTokenReq.bIsAck << '\n'

        << "Label				 :" <<'\n'
        << "\t* Extension bit    :" << (APIS32)((roleTokenReq.label) & 0x00000080) << '\n'
        << "\t* Sub-Role bit     :" << (APIS32)((roleTokenReq.label) & 0x00000040) << '\n'
        << "\t* Terminator bit   :" << (APIS32)((roleTokenReq.label) & 0x00000020) << '\n'
        << "\t* Role             :" << (APIS32)((roleTokenReq.label) & 0x0000001f) << '\n';

}
////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceCallDropTimerExpieredReq(CObjString* pContentStr)
{
	mcReqCallDropTimerExpired callDropTimerReq;
	callDropTimerReq.rejectCallReason = 0;
	memcpy(&callDropTimerReq, m_pMplMcmsProt->getpData(),sizeof(mcReqCallDropTimerExpired));


	*pContentStr << "\nCONTENT: H323_CS_SIG_CALL_DROP_TIMER_EXPIRED_REQ: \n ";
    switch(callDropTimerReq.rejectCallReason)
	{
	case ct_DRQ_NoAnswer:
		*pContentStr << "Reason	    = DRQ No Answer \n";
		break;
	case ctCallDropNoAnswer:
		*pContentStr << "Reason	    = Call Drop No Answer \n";
		break;
	case ctCallDropWithOpenChannel:
		*pContentStr << "Reason	    = Call Drop With Open Channel \n";
		break;
    }
}

////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceChannelDropReq(CObjString* pContentStr)
{
	mcReqChannelDrop channelDrop;
	memcpy(&channelDrop, m_pMplMcmsProt->getpData(),sizeof(mcReqChannelDrop));


	*pContentStr << "\nCONTENT: H323_CS_SIG_CHNL_DROP_REQ: \n "
		<< "Channel type		: " << channelDrop.channelType << '\n'
		<< "Channel index		: " << channelDrop.channelIndex << '\n'
		<< "Channel direction	: " << channelDrop.channelDirection << '\n';

}
////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceCallDropReq(CObjString* pContentStr)
{
	mcReqCallDrop callDrop;
	memcpy(&callDrop, m_pMplMcmsProt->getpData(),sizeof(mcReqCallDrop));


	*pContentStr << "\nCONTENT: H323_CS_SIG_CALL_DROP_REQ: \n "
		<< "Reason:		" << callDrop.rejectCallReason << '\n';

}
////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceAuthenticationReq(CObjString* pContentStr)
{
	mcReqAuthentication authentication;
	memcpy(&authentication, m_pMplMcmsProt->getpData(),sizeof(mcReqAuthentication));


	*pContentStr << "\nCONTENT: H323_CS_SIG_H323_CT_AUTHENTICATION_REQ: \n "
	    << "indexTblAuth: " << authentication.indexTblAuth << '\n';

	CLargeString trace;
	trace.SetFormat("%x");

	for(int i=0; i<sizeOf128Key; i++)
	{
		trace<<" 0x";
		trace<<(BYTE)authentication.authKey[i];
	}
	*pContentStr << "Auth key    : " << trace.GetString() << '\n';

}
////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceStopAllProcessesReq(CObjString* pContentStr)
{
	mcReqStopAllProcesses stopAllProcesses;
	memcpy(&stopAllProcesses, m_pMplMcmsProt->getpData(),sizeof(mcReqStopAllProcesses));


	*pContentStr << "\nCONTENT: IP_CS_SIG_STOP_ALL_PROCESSES_REQ: \n "
		<< "Reason:	" << stopAllProcesses.stopAllProcessorsReason << '\n';

}
////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceConferenceResponseReq(CObjString* pContentStr)
{
	mcReqConferenceRes conferenceResponseReq;
	memcpy(&conferenceResponseReq, m_pMplMcmsProt->getpData(),sizeof(mcReqConferenceRes));


	*pContentStr << "\nCONTENT: H323_CS_SIG_CONFERENCE_RES_REQ: \n ";

    switch( conferenceResponseReq.ConResOpcode )
    {
    case mCTerminalIDResponse :	 { *pContentStr << "mCTerminalIDResponse";	break; }//-- response to TCP(same as TIP) sent by MC only
    case terminalIDResponse :	   { *pContentStr << "terminalIDResponse";	  break; }//-- response to TCS2 or TCI
    case conferenceIDResponse :	 { *pContentStr << "conferenceIDResponse";	break; }//-- response to TCS3
    case passwordResponse :		 { *pContentStr << "passwordResponse";		break; }//-- response to TCS1
    case terminalListResponse :	 { *pContentStr << "terminalListResponse";	break; }
    case videoCommandRejectResponse :   { *pContentStr << "videoCommandRejectResponse";  break; }//-- same as H.230 VCR
    case terminalDropRejectResponse :   { *pContentStr << "terminalDropRejectResponse";  break; }//-- same as H.230 CIR
    case grantedChairTokenResponse :	{ *pContentStr << "grantedChairTokenResponse";   break; }//-- same as H.230 CIT
    case deniedChairTokenResponse  :	{ *pContentStr << "deniedChairTokenResponse";	break; }//-- same as H.230 CCR
    case extensionAddressResponse :	 { *pContentStr << "extensionAddressResponse";	break; }//-- response to TCS4
    case chairTokenOwnerResponse :	  { *pContentStr << "chairTokenOwnerResponse";	 break; }//-- response to TCA(same as TIR) sent by MC only
    case terminalCertificateResponse :  { *pContentStr << "terminalCertificateResponse"; break; }
    case grantedBroadcastMyLogicalChannelResponse : { *pContentStr << "grantedBroadcastMyLogicalChannelResponse"; break; }// MVA
    case deniedBroadcastMyLogicalChannelResponse :  { *pContentStr << "deniedBroadcastMyLogicalChannelResponse"; break; }// MVR
    case grantedMakeTerminalBroadcasterResponse :   { *pContentStr << "grantedMakeTerminalBroadcasterResponse";  break; }
    case deniedMakeTerminalBroadcasterResponse :	{ *pContentStr << "deniedMakeTerminalBroadcasterResponse";   break; }
    case grantedSendThisSourceResponse :	{ *pContentStr << "grantedSendThisSourceResponse";   break; }
    case deniedSendThisSourceResponse :	 { *pContentStr << "deniedSendThisSourceResponse";	break; }
    case requestAllTerminalIDsResponse :	{ *pContentStr << "requestAllTerminalIDsResponse";   break; }
    case remoteMCResponseAccept :		   { *pContentStr << "remoteMCResponseAccept";		  break; }
    case remoteMCResponseUnspecifiedReject: { *pContentStr << "remoteMCResponseUnspecifiedReject"; break; }
    case remoteMCResponseFunctionNotSupportedReject : { *pContentStr << "remoteMCResponseFunctionNotSupportedReject"; break; }

    default : { *pContentStr << "UNKNOWN !!!"; break; }
    }
	*pContentStr << '\n' << "Val1	 : " << conferenceResponseReq.val1 << '\n'
		<< "Val2	 : " << conferenceResponseReq.val2 << '\n'
		<< "Stream Id: " << conferenceResponseReq.strID << '\n';
}

////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceConferenceIndicationReq(CObjString* pContentStr)
{
	mcReqConferenceInd conferenceIndicationReq;
	memcpy(&conferenceIndicationReq, m_pMplMcmsProt->getpData(), sizeof(mcReqConferenceInd));

	*pContentStr << "\nCONTENT: H323_CS_SIG_CONFERENCE_IND_REQ: \n ";
	if(conferenceIndicationReq.ConIndOpcode == terminalYouAreSeeingIndication)
		*pContentStr << "Sub opcode:	 terminalYouAreSeeingIndication \n";
	else
		*pContentStr << "Sub opcode: " << conferenceIndicationReq.ConIndOpcode << '\n';

    *pContentStr << "Speaker MCU Id		: "  << conferenceIndicationReq.val1 << '\n'
				 << "Speaker terminal Id: " << conferenceIndicationReq.val2 << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceUnexpectedMessageReq(CObjString* pContentStr)
{
	mcReqUnexpectedMessage *pUnexpectedMessage;
	pUnexpectedMessage = (mcReqUnexpectedMessage*)new BYTE[m_pMplMcmsProt->getDataLen()];
	DWORD lengthStructure = m_pMplMcmsProt->getDataLen();
	memcpy(pUnexpectedMessage,(BYTE*)m_pMplMcmsProt->getpData(),lengthStructure);

	*pContentStr << "\nCONTENT: H323_CS_SIG_UNEXPECTED_MESSAGE_REQ: \n "
		<< "Bad opcode:	" << pUnexpectedMessage->badOpcode;

	CLargeString trace;
	for(APIU32 i=0;i<pUnexpectedMessage->sizeMessage;i++)
	{
		trace<<(BYTE)pUnexpectedMessage->message[i];
	}
	*pContentStr << "Message content:	" << trace.GetString() << '\n';

	PDELETEA(pUnexpectedMessage);
}

////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceDbc2OmmOnOffReq(CObjString* pContentStr,WORD flag)
{
	mcReqDBC2Command *pDbc2CommandReq;
	pDbc2CommandReq = (mcReqDBC2Command*)new BYTE[m_pMplMcmsProt->getDataLen()];
	DWORD lengthStructure = m_pMplMcmsProt->getDataLen();
	memcpy(pDbc2CommandReq,(BYTE*)m_pMplMcmsProt->getpData(),lengthStructure);

	if (flag == 1)
		*pContentStr << "\nCONTENT: H323_CS_SIG_DBC2_COMMAND_CT_ON_REQ: \n ";
	else
		*pContentStr << "\nCONTENT: H323_CS_SIG_DBC2_COMMAND_CT_OFF_REQ: \n ";

	*pContentStr << "Refresh rate		:	" << pDbc2CommandReq->refreshRate << '\n'
				 << "Inter leave		:	" << pDbc2CommandReq->interLeave << '\n'
				 << "Mpi limit			:	" << pDbc2CommandReq->mpiLimit << '\n'
				 << "Monitor vector		: 	" << pDbc2CommandReq->motionVector << '\n'
				 << "No encapsulation	:	" << pDbc2CommandReq->noEncapsulation << '\n'
				 << "Overlap			:	" << pDbc2CommandReq->overlap << '\n';

	PDELETEA(pDbc2CommandReq);

}

////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceDtmfInputReq(CObjString* pContentStr)
{
	mcReqDtmfBuff *pDtmfBuff;
	pDtmfBuff = (mcReqDtmfBuff*)new BYTE[m_pMplMcmsProt->getDataLen()];
	DWORD lengthStructure = m_pMplMcmsProt->getDataLen();
	memcpy(pDtmfBuff,(BYTE*)m_pMplMcmsProt->getpData(),lengthStructure);

	*pContentStr << "\nCONTENT: H323_CS_DTMF_INPUT_REQ: \n "
		<< "Source type:	" << pDtmfBuff->sourceType;

	CLargeString trace;
	for(APIU32 i=0;i<(APIU32)DtmfBuffLen;i++)
	{
		trace<<(BYTE)pDtmfBuff->dtmfBuffer[i];
	}
	*pContentStr << "Dtmf buffer:	" << trace.GetString() << '\n';

	PDELETEA(pDtmfBuff);

}

///////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceFacilityReq(CObjString* pContentStr)
{
    mcReqFacility *pFacilityReq;
	pFacilityReq = (mcReqFacility*)new BYTE[m_pMplMcmsProt->getDataLen()];
	DWORD lengthStructure = m_pMplMcmsProt->getDataLen();
	memcpy(pFacilityReq,(BYTE*)m_pMplMcmsProt->getpData(),lengthStructure);
    *pContentStr << "\nCONTENT: H323_CS_FACILITY_REQ: \n "
                 << "\nAVF ID = " << pFacilityReq->avfStandardId
                 << "\nConf ID= " << pFacilityReq->e164ConferenceId;
    PDELETEA(pFacilityReq);
}

///////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceLPRModeChangeReq(CObjString* pContentStr)
{
	mcReqLPRModeChange lPRModeChange;
	memcpy(&lPRModeChange, m_pMplMcmsProt->getpData(),sizeof(mcReqLPRModeChange));

	*pContentStr << "\nCONTENT: H323_CS_SIG_LPR_MODE_CHANGE_REQ: \n "
		<< "lossProtection		:		" << lPRModeChange.lossProtection << '\n'
		<< "mtbf				:		" << lPRModeChange.mtbf << '\n'
		<< "congestionCeiling	:		" << lPRModeChange.congestionCeiling << '\n'
		<< "modeTimeout			:		" << lPRModeChange.modeTimeout << '\n';
}

//added by Jason for ITP-Multiple channels
///////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceNewITPSpeakerReq(CObjString* pContentStr)
{
	mcReqNewITPSpeaker NewITPSpeaker;
	memcpy(&NewITPSpeaker, m_pMplMcmsProt->getpData(),sizeof(mcReqNewITPSpeaker));

	*pContentStr << "\nCONTENT: H323_CS_SIG_NEW_ITP_SPEAKER_REQ: \n "
		<< "ITPType			:		" << NewITPSpeaker.ITPType << '\n'
		<< "numOfActiveLinks	:		" << NewITPSpeaker.numOfActiveLinks << '\n';
}

// Ip CS Ind
////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceGetPortInd(CObjString* pContentStr)
{
	mcIndGetPort getPortInd;
	memcpy(&getPortInd, m_pMplMcmsProt->getpData(),sizeof(mcIndGetPort));


	*pContentStr << "\nCONTENT: H323_CS_SIG_GET_PORT_IND: \n "
		<< "Transport address:	\n";
	TraceTransportAddrSt(pContentStr, getPortInd.srcCallSignalAddress);
	*pContentStr << '\n';



}
////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceCallReportInd(CObjString* pContentStr, DWORD opcode)
{
	mcIndCallReport callReportInd;
	memcpy(&callReportInd, m_pMplMcmsProt->getpData(),sizeof(mcIndCallReport));


	switch(opcode)
	{
		case H323_CS_SIG_CALL_DIAL_TONE_IND: {
			*pContentStr << "\nCONTENT: H323_CS_SIG_CALL_DIAL_TONE_IND: \n ";
			break;
		}
		case H323_CS_SIG_CALL_PROCEEDING_IND: {
			*pContentStr << "\nCONTENT: H323_CS_SIG_CALL_PROCEEDING_IND: \n ";
			break;

		}
		case H323_CS_SIG_CALL_RING_BACK_IND: {
			*pContentStr << "\nCONTENT: H323_CS_SIG_CALL_RING_BACK_IND: \n ";
			break;

		}
		default: {
			PASSERTMSG(opcode,"UNKnown opcode");
		}

	}

	*pContentStr << "Transport address:	\n";
	TraceTransportAddrSt(pContentStr, callReportInd.h245IpAddress);
	*pContentStr << '\n';


}

////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceCallOfferingInd(CObjString* pContentStr)
{
	mcIndCallOffering	*pCallOfferingInd;
	pCallOfferingInd = (mcIndCallOffering*)new BYTE[m_pMplMcmsProt->getDataLen()];
	DWORD lengthStructure = m_pMplMcmsProt->getDataLen();
	memcpy(pCallOfferingInd,(BYTE*)m_pMplMcmsProt->getpData(),lengthStructure);


	*pContentStr << "\nCONTENT: H323_CS_SIG_CALL_OFFERING_IND: \n";
	// IpV6
		*pContentStr << "Source Party Address		: \n";
		TraceTransportAddrSt(pContentStr, pCallOfferingInd->srcIpAddress);
		*pContentStr << "Destination Party Address	: \n";
		TraceTransportAddrSt(pContentStr, pCallOfferingInd->destIpAddress);
		*pContentStr << "Source Party Aliases		: "	<< pCallOfferingInd->srcPartyAliases << '\n';
		*pContentStr << "Destination party Aliases	: "	<< pCallOfferingInd->destPartyAliases << '\n';
		*pContentStr << "Rate			  		    : " << pCallOfferingInd->rate << '\n'
		<< "SDisplay		  		    : " << pCallOfferingInd->sDisplay << '\n'
		<< "User user		  		    : " << pCallOfferingInd->userUser << '\n'
		<< "User user size	  		    : " << pCallOfferingInd->userUserSize << '\n'
		<< "Conference ID	  		    : ";
	PrintHexNum(pContentStr, pCallOfferingInd->conferenceId, Size16);
	*pContentStr << "Call Id					    : ";
	PrintHexNum(pContentStr, pCallOfferingInd->callId, Size16);



	*pContentStr << "Conference goal	  		    : ";
	::GetConferenceGoalTypeName(pCallOfferingInd->conferenceGoal, *pContentStr);
	*pContentStr << '\n';

	*pContentStr << "Reference value	  		    : " << pCallOfferingInd->referenceValue << '\n';

	*pContentStr << "Type			  		    : ";
	::GetCallTypeName(pCallOfferingInd->type, *pContentStr);
	*pContentStr << '\n';

	*pContentStr << "Is active MC	  		    : " << pCallOfferingInd->bIsActiveMc << '\n'
		<< "Is origin		  		    : " << pCallOfferingInd->bIsOrigin << '\n'
		<< "H245 IP address	  		    : \n";
	TraceTransportAddrSt(pContentStr, pCallOfferingInd->h245IpAddress);
	*pContentStr << "H245 Establish	  		    : " << pCallOfferingInd->bH245Establish << '\n';

	*pContentStr << "Src endpoint type 		    : ";
	::GetEndpointTypeName(pCallOfferingInd->srcEndpointType, *pContentStr);
	*pContentStr << '\n';

	*pContentStr << "Local H225 port	  		    : " << pCallOfferingInd->localH225Port << '\n'
		<< "Index tbl authentication    : " << pCallOfferingInd->indexTblAuth << '\n'
		<< "Encryption		  		    : \n";
	TraceEncTokensHeaderStruct(pContentStr, &(pCallOfferingInd->encryTokens));
	*pContentStr << '\n';
	PDELETEA(pCallOfferingInd);
}

////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceCallConnectedInd(CObjString*pContentStr)
{
	mcIndCallConnected	*pCallConnectedInd;
	pCallConnectedInd = (mcIndCallConnected*)new BYTE[m_pMplMcmsProt->getDataLen()];

	DWORD lengthStructure = m_pMplMcmsProt->getDataLen();
	memcpy(pCallConnectedInd,(BYTE*)m_pMplMcmsProt->getpData(),lengthStructure);


	*pContentStr << "\nCONTENT: H323_CS_SIG_CALL_CONNECTED_IND: \n"
		<< "H225 remote address     : ";
	TraceTransportAddrSt(pContentStr, pCallConnectedInd->h225remote);
	*pContentStr << "\nH225 local address      : ";
	TraceTransportAddrSt(pContentStr, pCallConnectedInd->h225local);
	*pContentStr << "\nH245 remote address     : ";
	TraceTransportAddrSt(pContentStr, pCallConnectedInd->h245remote);
	*pContentStr << "\nH245 local address	    : ";
	TraceTransportAddrSt(pContentStr, pCallConnectedInd->h245local);
	*pContentStr << "\nSDisplay		  	    : " << pCallConnectedInd->sDisplay << '\n'
				 << "User user				: " << pCallConnectedInd->userUser << '\n'
				 << "User user size			: " << pCallConnectedInd->userUserSize << '\n';

	*pContentStr << "Remote EP type			: ";
	::GetEndpointTypeName(pCallConnectedInd->remoteEndpointType, *pContentStr);
	*pContentStr << '\n';

	*pContentStr << "H225 remote version 	: " << pCallConnectedInd->h225RemoteVersion << '\n'
				 << "EP network			  	: " << pCallConnectedInd->endPointNetwork << '\n'
				 << "Authentication		  	: " << pCallConnectedInd->bAuthenticated << '\n'
				 << "Avaya				  	: ";

	TraceH460AvayaFeVndrIndSt(pContentStr, pCallConnectedInd->avfFeVndIdInd);
	*pContentStr << "\nCm Vendor " << '\n'
		<< "=========		   		" << '\n'
		<< "Product ID		  		: " << pCallConnectedInd->remoteVendor.productID << '\n'
		<< "Version ID		  		: " << pCallConnectedInd->remoteVendor.versionID << '\n'
		<< "Non standard info 		: " << '\n'
		<< "Country code (t35)		: " << pCallConnectedInd->remoteVendor.info.t35CountryCode << '\n'
		<< "Extention (t35)	  		: " << pCallConnectedInd->remoteVendor.info.t35Extension << '\n'
		<< "Manufacturer code 		: " << pCallConnectedInd->remoteVendor.info.manufacturerCode << '\n'
		<< "Encryption		  		: \n";
	TraceEncTokensHeaderStruct(pContentStr, &(pCallConnectedInd->encryTokens));
	*pContentStr << '\n';

	PDELETEA(pCallConnectedInd);


}
////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceCallNewRateInd(CObjString* pContentStr)
{
	mcIndCallNewRate callNewRateInd;
	memcpy(&callNewRateInd, m_pMplMcmsProt->getpData(),sizeof(mcIndCallNewRate));

	*pContentStr << "\nCONTENT: H323_CS_SIG_CALL_NEW_RATE_IND: \n "
		<< "Rate:	" << callNewRateInd.rate << '\n';

}
////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceCallCntlConnectedInd(CObjString* pContentStr)
{
	mcIndCallControlConnected callCntlConnectedInd;
	memcpy(&callCntlConnectedInd, m_pMplMcmsProt->getpData(),sizeof(mcIndCallControlConnected));


	*pContentStr << "\nCONTENT: H323_CS_SIG_CALL_CNTL_CONNECTED_IND: \n"
		<< "H245 remote address		: ";
	TraceTransportAddrSt(pContentStr, callCntlConnectedInd.remoteH245Address);
	*pContentStr << "\nMaster/Slave status		: ";
	if(callCntlConnectedInd.masterSlaveStatus == cmMSMaster)
		*pContentStr << "MASTER \n";
	else
		*pContentStr << "SLAVE \n";
	*pContentStr << "User user				: " << callCntlConnectedInd.userUser << '\n'
				 << "User user size			: " << callCntlConnectedInd.userUserSize << '\n';

	*pContentStr << "\nH245 local address		: ";
	TraceTransportAddrSt(pContentStr, callCntlConnectedInd.localH245Address);
	*pContentStr << '\n';
}

////////////////////////////////////////////////////////////////////////////////
//Current function prints body of H323_CS_FACILITY_IND message received from CS
void CConfPartyMplMcmsProtocolTracer::TraceFacilityInd(CObjString* pContentStr)
{
	DWORD dwStructLength = m_pMplMcmsProt->getDataLen();
	mcIndFacility* pFacilityInd = (mcIndFacility*)new BYTE[dwStructLength];
	memcpy(pFacilityInd, (BYTE*)m_pMplMcmsProt->getpData(), dwStructLength);

	short  countryCode    = pFacilityInd->avfFeVndIdInd.countryCode;
	short  t35Extension   = pFacilityInd->avfFeVndIdInd.t35Extension;
	short  isAvayaSipCm   = pFacilityInd->avfFeVndIdInd.bSipCM;
	int    manufactorCode = pFacilityInd->avfFeVndIdInd.manfctrCode;

	*pContentStr << "\nCONTENT: H323_CS_FACILITY_IND: \n";
	COstrStream msg;
	//The struct mcIndFacility includes: h460AvayaFeVndrIndSt avfFeVndIdInd, h460AvayaFeMaxNonAudioBitRateInd avfFeMaxNonAudioBitRateInd
	//Print received data
	msg << " Facility message" << "\n";
	msg << "==================================================" << "\n";
	msg << "Feature Set ID			: " << pFacilityInd->avfFeVndIdInd.fsId << "\n";
	msg	<< "Country Code			: " << (short)pFacilityInd->avfFeVndIdInd.countryCode << "\n";
	msg	<< "T35 Extension			: " << (short)pFacilityInd->avfFeVndIdInd.t35Extension << "\n";
	msg	<< "Is SIP CM				: " << (short)pFacilityInd->avfFeVndIdInd.bSipCM << "\n";
	msg	<< "Manufacturer Code		: " << (int)pFacilityInd->avfFeVndIdInd.manfctrCode << "\n";
	if ((pFacilityInd->avfFeVndIdInd).productId != '\0')
		msg	<< "Product ID				: " << (pFacilityInd->avfFeVndIdInd).productId << "\n";
	if ((pFacilityInd->avfFeVndIdInd).versionId != '\0')
		msg << "Version ID				: " << (pFacilityInd->avfFeVndIdInd).versionId << "\n";
	if ((pFacilityInd->avfFeVndIdInd).enterpriseNum != '\0')
		msg << "Enterprise Number		: " << (pFacilityInd->avfFeVndIdInd).enterpriseNum << "\n";
	msg << "==================================================" << "\n";
	//Note: The second structure h460AvayaFeMaxNonAudioBitRateInd currently not used
	*pContentStr << msg.str().c_str();
	PDELETEA(pFacilityInd);
}

////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceCapabilitiesInd(CObjString* pContentStr)
{
	mcIndCapabilities *pCapabilitiesInd;
	pCapabilitiesInd = (mcIndCapabilities*) new BYTE[m_pMplMcmsProt->getDataLen()];

	DWORD lengthStructure = m_pMplMcmsProt->getDataLen();
	memcpy(pCapabilitiesInd, (BYTE*) m_pMplMcmsProt->getpData(), lengthStructure);

	*pContentStr << "\nCONTENT: H323_CS_SIG_CAPABILITIES_IND: \n ";

	COstrStream msg;

	msg << " Remote Capabilities " << "\n";
	msg << "=====================" << "\n";

	ctCapabilitiesStruct* pCapabilitiesStruct = &(pCapabilitiesInd->capabilities);
	capBuffer* pCapBuffer = (capBuffer *) &(pCapabilitiesStruct->caps);
	char* tempPtr = (char*) pCapBuffer;

	for (int i = 0; i < pCapabilitiesStruct->numberOfCaps; i++)
	{
		CCapSetInfo capInfo = (CapEnum) pCapBuffer->capTypeCode;
		msg << "\n----- ";
		capInfo.Dump(msg);
		msg << " -----";

		CBaseCap * pCap = CBaseCap::AllocNewCap(capInfo, pCapBuffer->dataCap);
		if (pCap)
			pCap->Dump(msg);
		PDELETE(pCap);

		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*) tempPtr;
	}
	msg << "=========================" << "\n";

	*pContentStr << msg.str().c_str();
	PDELETEA(pCapabilitiesInd);
}

////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceCallRoleTokenInd(CObjString* pContentStr)
{

	mcIndRoleToken roleTokenInd;
	memcpy(&roleTokenInd, m_pMplMcmsProt->getpData(),sizeof(mcIndRoleToken));

	*pContentStr << "\nCONTENT: H323_CS_SIG_CALL_ROLE_TOKEN_IND: \n ";

	BYTE bIsH239 = IsRoleTokenOpcodeH239Type((ERoleTokenOpcode)roleTokenInd.subOpcode);
	if (bIsH239)
		*pContentStr << "H239 message" << '\n';
	else
		*pContentStr << "People Plus Content message" << '\n';

	*pContentStr << "Sub opcode           :" << GetRoleTokenOpcodeStr((ERoleTokenOpcode)roleTokenInd.subOpcode) << '\n'
		<< "Mcu ID               :" << roleTokenInd.mcuID << '\n'
	    << "Terminal ID          :" << roleTokenInd.terminalID << '\n'
	    << "Rand Number          :" << roleTokenInd.randNumber << '\n'
	    << "Content provider info:" << roleTokenInd.contentProviderInfo << '\n'
	    << "Bit Rate			 :" << roleTokenInd.bitRate << '\n'
	    << "Is Ack			     :" << (int)roleTokenInd.bIsAck << '\n'
	    << "Label				 :" <<'\n'
	    << "\t* Extension bit    :" << (APIS32)((roleTokenInd.label) & 0x00000080) << '\n'
	    << "\t* Sub-Role bit     :" << (APIS32)((roleTokenInd.label) & 0x00000040) << '\n'
	    << "\t* Terminator bit   :" << (APIS32)((roleTokenInd.label) & 0x00000020) << '\n'
	    << "\t* Role             :" << (APIS32)((roleTokenInd.label) & 0x0000001f) << '\n';

}

////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceBadSpontanInd(CObjString* pContentStr)
{

	mcIndBadSpontan badSpontanInd;
	memcpy(&badSpontanInd, m_pMplMcmsProt->getpData(),sizeof(mcIndBadSpontan));

	*pContentStr << "\nCONTENT: H323_CS_SIG_CALL_BAD_SPONTAN_IND: \n ";

	if (badSpontanInd.status < lastBadSpontanIndReason)
		*pContentStr << g_badSpontanIndReasonStrings[badSpontanInd.status];
	else
		*pContentStr << badSpontanInd.status;
	*pContentStr << '\n';

}

////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceAuthenticationInd(CObjString* pContentStr)
{

	mcIndAuthentication authenticationInd;
	memcpy(&authenticationInd, m_pMplMcmsProt->getpData(),sizeof(mcIndAuthentication));

	*pContentStr << "\nCONTENT: H323_CS_SIG_CALL_AUTHENTICATION_IND: \n "
				 << "Is Authenticated: " << authenticationInd.bAuthenticated << '\n';

}

////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceIncomingChannelInd(CObjString* pContentStr)
{
	mcIndIncomingChannel *pIncomingchannel;
	pIncomingchannel = (mcIndIncomingChannel*)new BYTE[m_pMplMcmsProt->getDataLen()];
	DWORD lengthStructure = m_pMplMcmsProt->getDataLen();
	memcpy(pIncomingchannel,(BYTE*)m_pMplMcmsProt->getpData(),lengthStructure);


	*pContentStr << "\nCONTENT: H323_CS_SIG_INCOMING_CHANNEL_IND: \n"
		<< "Remote RTP address\n";
 	TraceTransportAddrSt(pContentStr, pIncomingchannel->rmtRtpAddress);
	*pContentStr << "\nPayload type				: " << pIncomingchannel->payloadType << '\n'
		<< "Data type					: " << pIncomingchannel->dataType << '\n'
		<< "Channel index				: " << pIncomingchannel->channelIndex << '\n'
		<< "Channel direction			: " << pIncomingchannel->channelDirection << '\n'
		<< "Channel name				: " << pIncomingchannel->channelName << '\n'
		<< "Cap type code				: " << pIncomingchannel->capTypeCode << '\n'
		<< "Dynamic payload type		: " << (APIU8)pIncomingchannel->dynamicPayloadType << '\n'
		<< "Is encrypted				: " << pIncomingchannel->bIsEncrypted << '\n'
		<< "Encryption algorithm		: " << pIncomingchannel->encryptionAlgorithm << '\n'
		<< "Rate						: " << pIncomingchannel->rate << '\n'
		<< "Is active					: " << pIncomingchannel->bIsActive << '\n'
		<< "Same session channel index	: " << pIncomingchannel->sameSessionChannelIndex << '\n'
		<< "Session Id					: " << pIncomingchannel->sessionId << '\n'
		<< "Is LPR						: " << pIncomingchannel->bIsLPR << '\n';
	/*
	if (pIncomingchannel->bIsEncrypted)
	{
		CLargeString trace;
		trace.SetFormat("%x");
		trace << "0x";
		int j = 0;
		for (j = 0 ; j < 16 ; j++)
		{
			trace << (BYTE)pIncomingchannel->EncryptedSession235Key[j];
		}
		*pContentStr  << "Encryption session 235 key  : " << trace.GetString() << '\n';
	}
	*/
	*pContentStr << "Size of channel params		: " << pIncomingchannel->sizeOfChannelParams << "\n\n";

	COstrStream msg;
	msg << " Channel Parameters:"<< "\n";
	msg << "=====================" << "\n";

	CCapSetInfo capInfo = (CapEnum)pIncomingchannel->capTypeCode;
	capInfo.Dump(msg);
	msg << "\n";

	CBaseCap * pCap = CBaseCap::AllocNewCap(capInfo,(BYTE*)(pIncomingchannel->channelSpecificParams));
	if (pCap)
		pCap->Dump(msg);
	PDELETE(pCap);


	msg << "=========================" << "\n";
	*pContentStr << msg.str().c_str();

	PDELETEA(pIncomingchannel);

}

////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceIncomingChannelConnectedInd(CObjString* pContentStr)
{

	mcIndIncomingChannelConnected IncomingChannelConnectedInd;
	memcpy(&IncomingChannelConnectedInd, m_pMplMcmsProt->getpData(),sizeof(mcIndIncomingChannelConnected));

	*pContentStr << "\nCONTENT: H323_CS_SIG_INCOMING_CHNL_CONNECTED_IND: \n "
				 << "Channel type				:	" << IncomingChannelConnectedInd.channelType << '\n'
				 << "Channel index				:	" << IncomingChannelConnectedInd.channelIndex << '\n'
				 << "Channel direction			:	" << IncomingChannelConnectedInd.channelDirection << '\n'
				 << "Session Id					:	" << IncomingChannelConnectedInd.sessionId << '\n'
				 << "Same session channel index : 	" << IncomingChannelConnectedInd.sameSessionChannelIndex << '\n'
				 << "Assosiated channel index	:	" << IncomingChannelConnectedInd.associatedChannelIndex << '\n';


}

////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceOutgoingChannelResponseInd(CObjString* pContentStr)
{
	mcIndOutgoingChannelResponse *pOutgoingChannelResponse;
	pOutgoingChannelResponse = (mcIndOutgoingChannelResponse*)new BYTE[m_pMplMcmsProt->getDataLen()];
	DWORD lengthStructure = m_pMplMcmsProt->getDataLen();
	memcpy(pOutgoingChannelResponse,(BYTE*)m_pMplMcmsProt->getpData(),lengthStructure);


	*pContentStr << "\nCONTENT: H323_CS_SIG_OUTGOING_CHNL_RESPONSE_IND: \n "
				 << "Channel type					:	" << pOutgoingChannelResponse->channelType << '\n'
				 << "Channel index					:	" << pOutgoingChannelResponse->channelIndex << '\n'
				 << "Channel direction				:	" << pOutgoingChannelResponse->channelDirection << '\n'
				 << "Destination rtp address		:	";
	TraceTransportAddrSt(pContentStr, pOutgoingChannelResponse->destRtpAddress);
	*pContentStr << "\nSession id						:	" << pOutgoingChannelResponse->sessionId << '\n'
				 << "Payload type					:	" << pOutgoingChannelResponse->payloadType << '\n'
				 << "Dynamic payload type			:	" << (APIU8)pOutgoingChannelResponse->dynamicPayloadType << '\n'
				 << "Same session channel index		:	" << pOutgoingChannelResponse->sameSessionChannelIndex << '\n'
				 << "Associated channel index		:	" << pOutgoingChannelResponse->associatedChannelIndex << '\n';

	*pContentStr << "Is encrypted					: " << pOutgoingChannelResponse->bIsEncrypted << '\n'
				<< "Encryption algorithm			: " << pOutgoingChannelResponse->encryptionAlgorithm << '\n';

	/*
	if (pOutgoingChannelResponse->bIsEncrypted)
	{
		CLargeString trace;
		trace.SetFormat("%x");
		trace << "0x";
		int j = 0;
		for (j = 0 ; j < 16 ; j++)
		{
			trace << (BYTE)pOutgoingChannelResponse->EncryptedSession235Key[j];
		}
		*pContentStr  << "Encryption session 235 key  : " << trace.GetString() << '\n';
	}
	*/

	PDELETEA(pOutgoingChannelResponse);
}


////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceChannelNewRateInd(CObjString* pContentStr)
{

	mcIndChannelNewRate ChannelNewRateInd;
	memcpy(&ChannelNewRateInd, m_pMplMcmsProt->getpData(),sizeof(mcIndChannelNewRate));

	*pContentStr << "\nCONTENT: H323_CS_SIG_CHAN_NEW_RATE_IND: \n "
				 << "Channel type				:	" << ChannelNewRateInd.channelType << '\n'
				 << "Channel index				:	" << ChannelNewRateInd.channelIndex << '\n'
				 << "Channel direction			:	" << ChannelNewRateInd.channelDirection << '\n'
				 << "Rate						:	" << ChannelNewRateInd.rate << '\n';

}


////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceStartChannelCloseInd(CObjString* pContentStr)
{

	mcIndStartChannelClose StarChannelCloseInd;
	memcpy(&StarChannelCloseInd, m_pMplMcmsProt->getpData(),sizeof(mcIndStartChannelClose));

	*pContentStr << "\nCONTENT: H323_CS_SIG_START_CHANNEL_CLOSE_IND: \n "
				 << "Channel type				:	" << StarChannelCloseInd.channelType << '\n'
				 << "Channel index				:	" << StarChannelCloseInd.channelIndex << '\n'
				 << "Channel direction			:	" << StarChannelCloseInd.channelDirection << '\n';

}


////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceChannelCloseInd(CObjString* pContentStr)
{

	mcIndChannelClosed ChannelCloseInd;
	memcpy(&ChannelCloseInd, m_pMplMcmsProt->getpData(),sizeof(mcIndChannelClosed));

	*pContentStr << "\nCONTENT: H323_CS_SIG_CHANNEL_CLOSE_IND: \n "
				 << "Channel type				:	" << ChannelCloseInd.channelType << '\n'
				 << "Channel index				:	" << ChannelCloseInd.channelIndex << '\n'
				 << "Channel direction			:	" << ChannelCloseInd.channelDirection << '\n';
}

////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceChannelMaxSkewInd(CObjString* pContentStr)
{

	mcIndChannelMaxSkew ChannelMaxSkewInd;
	memcpy(&ChannelMaxSkewInd, m_pMplMcmsProt->getpData(),sizeof(mcIndChannelMaxSkew));

	*pContentStr << "\nCONTENT: H323_CS_SIG_CHAN_MAX_SKEW_IND: \n "
				 << "Second channel index	:	" << ChannelMaxSkewInd.secondChannelIndex << '\n'
				 << "Skew					:	" << ChannelMaxSkewInd.skew << '\n';

}

////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceFlowControlIndInd(CObjString* pContentStr)
{

	mcIndFlowControlIndication FlowCntlIndInd;
	memcpy(&FlowCntlIndInd, m_pMplMcmsProt->getpData(),sizeof(mcIndFlowControlIndication));

	*pContentStr << "\nCONTENT: H323_CS_SIG_FLOW_CONTROL_IND_IND: \n "
				 << "Channel type				:	" << FlowCntlIndInd.channelType << '\n'
				 << "Channel index				:	" << FlowCntlIndInd.channelIndex << '\n'
				 << "Channel direction			:	" << FlowCntlIndInd.channelDirection << '\n'
				 << "Rate						:	" << FlowCntlIndInd.rate << '\n';

}

////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceDbc2OmmOnOffInd(CObjString* pContentStr,WORD flag)
{
	mcIndDBC2Command *pDbc2CommandInd;
	pDbc2CommandInd = (mcIndDBC2Command*)new BYTE[m_pMplMcmsProt->getDataLen()];
	DWORD lengthStructure = m_pMplMcmsProt->getDataLen();
	memcpy(pDbc2CommandInd,(BYTE*)m_pMplMcmsProt->getpData(),lengthStructure);

	if (flag == 1)
		*pContentStr << "\nCONTENT: H323_CS_SIG_DBC2_COMMAND_CT_ON_IND: \n ";
	else
		*pContentStr << "\nCONTENT: H323_CS_SIG_DBC2_COMMAND_CT_OFF_IND: \n ";

	*pContentStr << "Refresh rate		:	" << pDbc2CommandInd->refreshRate << '\n'
				 << "Inter leave		:	" << pDbc2CommandInd->interLeave << '\n'
				 << "Mpi limit			:	" << pDbc2CommandInd->mpiLimit << '\n'
				 << "Monitor vector		: 	" << pDbc2CommandInd->motionVector << '\n'
				 << "No encapsulation	:	" << pDbc2CommandInd->noEncapsulation << '\n'
				 << "Overlap			:	" << pDbc2CommandInd->overlap << '\n';

	PDELETEA(pDbc2CommandInd);


}

////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceNonStandardInd(CObjString* pContentStr, DWORD opcode)
{
	mcIndNonStandard *pNonStandardInd;
	pNonStandardInd = (mcIndNonStandard*)new BYTE[m_pMplMcmsProt->getDataLen()];
	DWORD lengthStructure = m_pMplMcmsProt->getDataLen();
	memcpy(pNonStandardInd,(BYTE*)m_pMplMcmsProt->getpData(),lengthStructure);

	switch (opcode)
	{
		case H323_CS_NON_STANDARD_REQ_IND: {
			*pContentStr << "\nCONTENT: H323_CS_NON_STANDARD_REQ_IND: \n ";
			break;
		}
		case H323_CS_NON_STANDARD_COM_IND: {
			*pContentStr << "\nCONTENT: H323_CS_NON_STANDARD_COM_IND: \n ";
			break;
		}
		case H323_CS_NON_STANDARD_RES_IND: {
			*pContentStr << "\nCONTENT: H323_CS_NON_STANDARD_RES_IND: \n ";
			break;
		}
		case H323_CS_NON_STANDARD_IND_IND: {
			*pContentStr << "\nCONTENT: H323_CS_NON_STANDARD_IND_IND: \n ";
			break;
		}
		default :
		{
			PASSERT(opcode);
			PDELETEA(pNonStandardInd);
			
			return;
		}
	}
    ctNonStandardParameterSt*   pNsPar;
    pNsPar = &(pNonStandardInd->nonStandardData);

    if( pNsPar->info.objectLength > 0 )
    {
        // do nothing by Mikhail Karasik
    }
    else
    {
        if ( (BYTE)pNsPar->info.t35CountryCode == 0xB5 &&
             (BYTE)pNsPar->info.t35Extension   == 0x00 &&
             (WORD)pNsPar->info.manufacturerCode == 0x0001 &&
             (BYTE)pNsPar->data[0] == 0x8E )
        {
            // country - USA, manufact - PictureTel, data - ContentVisualization
            *pContentStr << "\n opcode = ContentVisualization;\n";
        }
        else
        {
            *pContentStr << "\n  t35CountryCode = " << (BYTE)pNsPar->info.t35CountryCode   << ";"
                << "\n  t35Extension   = " << (BYTE)pNsPar->info.t35Extension     << ";"
                << "\n  manufacturer   = " << (WORD)pNsPar->info.manufacturerCode << ";"
                << "\n  data :";
            for( int i=0; i<3; i++ )
                *pContentStr << " <0x" << (BYTE)pNsPar->data[i] << ">;";
        }
        *pContentStr << '\n';
    }

    PDELETEA(pNonStandardInd);
}

////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceConfReqInd(CObjString* pContentStr)
{

	mcIndConferenceReq ConfReqInd;
	memcpy(&ConfReqInd, m_pMplMcmsProt->getpData(),sizeof(mcIndConferenceReq));

	*pContentStr << "\nCONTENT: H323_CS_CONFERENCE_REQ_IND: \n ";


    switch( ConfReqInd.ConReqOpcode )
    {
    case terminalListRequest :      { *pContentStr << "terminalListRequest";         break; }//-- same as H.230 TCU (term->MC)
    case makeMeChairRequest :       { *pContentStr << "makeMeChairRequest";          break; }//-- same as H.230 CCA (term->MC)
    case cancelMakeMeChairRequest : { *pContentStr << "cancelMakeMeChairRequest";    break; }//-- same as H.230 CIS (term->MC)
    case dropTerminalRequest :      { *pContentStr << "dropTerminalRequest";         break; }//-- same as H.230 CCD(term->MC)
    case requestTerminalIDRequest : { *pContentStr << "requestTerminalIDRequest";        break; }//-- sames as TCP (term->MC)
    case enterH243PasswordRequest :     { *pContentStr << "enterH243PasswordRequest";    break; }//-- same as H.230 TCS1 (MC->term)
    case enterH243TerminalIDRequest :   { *pContentStr << "enterH243TerminalIDRequest";      break; }//-- same as H.230 TCS2/TCI (MC->term)
    case enterH243ConferenceIDRequest : { *pContentStr << "enterH243ConferenceIDRequest";    break; }//-- same as H.230 TCS3 (MC->term)
    case enterExtensionAddressRequest :     { *pContentStr << "enterExtensionAddressRequest";    break; }//-- same as H.230 TCS4 (GW->term)
    case requestChairTokenOwnerRequest :    { *pContentStr << "requestChairTokenOwnerRequest";       break; }//-- same as H.230 TCA (term->MC)
    case requestTerminalCertificateRequest: { *pContentStr << "requestTerminalCertificateRequest";   break; }
    case broadcastMyLogicalChannelRequest : { *pContentStr << "broadcastMyLogicalChannelRequest";    break; }//-- similar to H.230 MCV
    case makeTerminalBroadcasterRequest :   { *pContentStr << "makeTerminalBroadcasterRequest";      break; }//-- similar to H.230 VCB
    case sendThisSourceRequest :            { *pContentStr << "sendThisSourceRequest";               break; }//-- similar to H.230 VCS
    case requestAllTerminalIDsRequest :     { *pContentStr << "requestAllTerminalIDsRequest";        break; }
    case remoteMCRequestMasterActivateRequest : { *pContentStr << "remoteMCRequestMasterActivateRequest";    break; }
    case remoteMCRequestSlaveActivateRequest :  { *pContentStr << "remoteMCRequestSlaveActivateRequest";     break; }
    case remoteMCRequestDeActivateRequest :     { *pContentStr << "remoteMCRequestDeActivateRequest";        break; }
    default : { *pContentStr << "UNKNOWN !!!"; break; }
    }
    *pContentStr << '\n'
    << "Val 1	: " << ConfReqInd.val1 << '\n'
    << "Val 2	: " << ConfReqInd.val2 << '\n';

}

////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceConfComInd(CObjString* pContentStr)
{
	mcIndConferenceCom ConfComInd;
	memcpy(&ConfComInd, m_pMplMcmsProt->getpData(),sizeof(mcIndConferenceCom));

	*pContentStr << "\nCONTENT: H323_CS_CONFERENCE_COM_IND: \n ";

    switch( ConfComInd.ConComOpcode )
    {
    case broadcastMyLogicalChannelCommand :         { *pContentStr << "broadcastMyLogicalChannelCommand";        break; }//-- similar to H.230 MCV
    case cancelBroadcastMyLogicalChannelCommand :   { *pContentStr << "cancelBroadcastMyLogicalChannelCommand";  break; }//-- similar to H.230 Cancel-MCV
    case makeTerminalBroadcasterCommand :           { *pContentStr << "makeTerminalBroadcasterCommand";          break; }//-- same as H.230 VCB
    case cancelMakeTerminalBroadcasterCommand :     { *pContentStr << "cancelMakeTerminalBroadcasterCommand";    break; }//-- same as H.230 Cancel-VCB
    case sendThisSourceCommand :            { *pContentStr << "sendThisSourceCommand";           break; }//-- same as H.230 VCS
    case cancelSendThisSourceCommand :      { *pContentStr << "cancelSendThisSourceCommand";     break; }//-- same as H.230 cancel VCS
    case dropConferenceCommand :            { *pContentStr << "dropConferenceCommand";           break; }//-- same as H.230 CCK
    case substituteConferenceIDCommand :    { *pContentStr << "substituteConferenceIDCommand";   break; }
    default : { *pContentStr << "UNKNOWN !!!"; break; }
    }
    *pContentStr << '\n'
    << "Val 1	: " << ConfComInd.val1 << '\n'
    << "Val 2	: " << ConfComInd.val2 << '\n'
    << "conID	: " << ConfComInd.conID << '\n';

}

////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceConfIndInd(CObjString* pContentStr)
{
	mcIndConferenceInd ConfIndInd;
	memcpy(&ConfIndInd, m_pMplMcmsProt->getpData(),sizeof(mcIndConferenceInd));

	*pContentStr << "\nCONTENT: H323_CS_CONFERENCE_IND_IND: \n ";

	if(ConfIndInd.ConIndOpcode == terminalNumberAssignIndication)
		*pContentStr << "\n Sub opcode = terminalNumberAssignIndication";
	else
		*pContentStr << "\n Opcode = " << ConfIndInd.ConIndOpcode;

	*pContentStr << "MCU number			:" << ConfIndInd.val1 << '\n';
	*pContentStr << "Terminal number		:" << ConfIndInd.val2 << '\n';

}

////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceLPRModeChangeInd(CObjString* pContentStr1)
{
	mcIndLPRModeChange lPRModeChange;
	memcpy(&lPRModeChange, m_pMplMcmsProt->getpData(),sizeof(mcIndLPRModeChange));

	*pContentStr1 << "\nCONTENT: H323_CS_SIG_LPR_MODE_CHANGE_IND: \n "
		<< "lossProtection		:		" << lPRModeChange.lossProtection << '\n'
		<< "mtbf				:		" << lPRModeChange.mtbf << '\n'
		<< "congestionCeiling	:		" << lPRModeChange.congestionCeiling << '\n'
		<< "modeTimeout			:		" << lPRModeChange.modeTimeout << '\n';
}

//added by Jason for ITP-Multiple channels
////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceNewITPSpeakerInd(CObjString* pContentStr1)
{
	mcIndNewITPSpeaker NewITPSpeaker;
	memcpy(&NewITPSpeaker, m_pMplMcmsProt->getpData(),sizeof(mcIndNewITPSpeaker));

	*pContentStr1 << "\nCONTENT: H323_CS_SIG_NEW_ITP_SPEAKER_IND: \n "
		<< "ITPType			:		" << NewITPSpeaker.ITPType << '\n'
		<< "numOfActiveLinks	:		" << NewITPSpeaker.numOfActiveLinks << '\n';
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceIpRtpCmMonitoringInd(CObjString *pContentStr, const char* opcodeString)
{
	TCmPartyMonitoringInd st;
	memcpy(&st, m_pMplMcmsProt->getpData(),sizeof(TCmPartyMonitoringInd));

	TRtpChannelMonitoringInd *pChannelMonitoring    = (TRtpChannelMonitoringInd *)st.acMonitoringData;

	*pContentStr << "\nCONTENT: " << opcodeString << ":\n";

	for(DWORD i=0; i < st.unNumOfChannels; i++)
	{
		// check if channel monitoring is valid data.
		if (pChannelMonitoring->tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.bunValidChannel != (DWORD)0)
		{
	        *pContentStr <<"-------------------------------------------\n";
		    *pContentStr <<"MC Is valid channel: "<<pChannelMonitoring->tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.bunValidChannel<<"\n";
			*pContentStr << "Channel media: " << pChannelMonitoring->tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unChannelType;
			*pContentStr <<"\nChannel media type: ";
			::GetChannelTypeName(pChannelMonitoring->tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unChannelType,*pContentStr);
			*pContentStr << "\nChannel direction (Numbers): " << pChannelMonitoring->tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unChannelDirection;
		    *pContentStr <<"\nMC Channel Direction: ";
		    ::GetChannelDirectionName(pChannelMonitoring->tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unChannelDirection,*pContentStr);
		   	*pContentStr <<"\nTicks Interval: "<<pChannelMonitoring->tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unTicksInterval<<"\n";
			*pContentStr <<"Media byte: "<<pChannelMonitoring->tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unMediaBytes<<"\n";
			*pContentStr <<"Protocol: "<<pChannelMonitoring->tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unProtocol<<"\n";

			if ((pChannelMonitoring->tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unChannelType == kIpVideoChnlType) ||
				(pChannelMonitoring->tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unChannelType == kIpContentChnlType))
			{
				TRtpVideoChannelMonitoring * pVidChannelMonitoring = (TRtpVideoChannelMonitoring *)pChannelMonitoring;

				//*pContentStr <<"Frames: "<<pVidChannelMonitoring->unFrames<<"\n";
				*pContentStr <<"FrameRate: "<<pVidChannelMonitoring->unFrameRate<<"\n";

				APIS32 resolution = pVidChannelMonitoring->unVideoResolution;
				if((resolution >= kUnknownFormat) && (resolution < kLastFormat))
				{
					*pContentStr <<"Video Resolution: ";
					CCapSetInfo capInfo;
					*pContentStr << capInfo.GetFormatStr((EFormat)resolution) << "\n";
				}
				*pContentStr <<"unVideoWidth: "<<pVidChannelMonitoring->unVideoWidth<<"\n";
				*pContentStr <<"unVideoHeight: "<<pVidChannelMonitoring->unVideoHeight<<"\n";

				*pContentStr <<"Annexes: "<< pVidChannelMonitoring->unAnnexes<<"\n";
				*pContentStr <<"Stream video sync : ";
	//			for (int j=0; j<4; j++)
	//			{
	//				*pContentStr << "Array Enrty = " << j << "\n";
					if (pVidChannelMonitoring->unStreamVideoSync==0)
						*pContentStr << 0 << "\n";
					else
					{
						DWORD streamVideoSyncParams = pVidChannelMonitoring->unStreamVideoSync;
						BOOL isValidBitMaskSet = TRUE;

						if ( !(ValidBitMask & streamVideoSyncParams) )
						{
							*pContentStr << streamVideoSyncParams << "\n";
							isValidBitMaskSet = FALSE;
						}

						if (isValidBitMaskSet)
						{
							DWORD temp = 0;

							// 1). Set BCH params
							if ( streamVideoSyncParams & BchSyncMask )
								*pContentStr << "Bch Sync Flag: " << 1 << "\n";

							temp = streamVideoSyncParams & BchCountMask;
							DWORD bchOutOfSyncCount = temp >> 16;
							*pContentStr << "Bch Out Of SyncCount: " << bchOutOfSyncCount << "\n";

							// 2). Set video protocol params
							if ( streamVideoSyncParams & ProtocolSyncMask )
								*pContentStr << "Protocol Sync Flag: " << 1 << "\n";

							DWORD protocolOutOfSyncCount = streamVideoSyncParams & ProtocolCountMask;
							*pContentStr << "Protocol Out Of Sync Count: " << protocolOutOfSyncCount << "\n";

							// 3). Set Intra Sync Flag
							if ( streamVideoSyncParams & IntraSyncBitMask )
								*pContentStr << "Intra Sync Flag: " << 1 << "\n";
						}
					}
	//			}//for

				TraceIpPartyMonitoringAdvance(*pContentStr,(void *)&(pVidChannelMonitoring->tAdvanceMonitoringResultsSt));
			}
			else if(pChannelMonitoring->tRtpAudioChannelMonitoring.tRtpCommonChannelMonitoring.unChannelType == kIpAudioChnlType)
			{
				TRtpAudioChannelMonitoring * pAudChannelMonitoring = (TRtpAudioChannelMonitoring *)pChannelMonitoring;
				*pContentStr <<"Frame Per Packet: "<<pAudChannelMonitoring->unFramesPerPacket<<"\n";
				TraceIpPartyMonitoringAdvance(*pContentStr,(void *)&(pAudChannelMonitoring->tAdvanceMonitoringResultsSt));
			}
			else if(pChannelMonitoring->tRtpFeccChannelMonitoring.tRtpCommonChannelMonitoring.unChannelType == kIpFeccChnlType)
			{
				TRtpFeccChannelMonitoring * pFeccChannelMonitoring = (TRtpFeccChannelMonitoring *)pChannelMonitoring;
				TraceIpPartyMonitoringAdvance(*pContentStr,(void *)&(pFeccChannelMonitoring->tAdvanceMonitoringResultsSt));
			}
		}

		else
			*pContentStr <<"Information is not valid for this specified channel: "<< i <<".\n";

		// Move the pointer to the next channel
		pChannelMonitoring = (TRtpChannelMonitoringInd *)((BYTE *)pChannelMonitoring + sizeof(TRtpVideoChannelMonitoring));
	}

	*pContentStr <<"-------------------------------------------\n";
}

////////////////////////////////////////////////////////////////////////////////////////////
//for CDR_MCCF:
void CConfPartyMplMcmsProtocolTracer::TraceIpRtpCmVideoChannelsStatisticsInd(CObjString *pContentStr)
{
    TCmPartyInfoStatisticsInd st;
    memcpy(&st, m_pMplMcmsProt->getpData(),sizeof(TCmPartyInfoStatisticsInd));

    TRtpChannelMonitoringInd *pChannelMonitoring = (TRtpChannelMonitoringInd *)st.acMonitoringData;

    *pContentStr << "\nCDR_MCCF: CONTENT: IP_CM_PARTY_VIDEO_CHANNELS_STATISTICS_IND: \n ";

    if (st.unNumOfChannels > MAX_SIP_CHANNELS)// it means that they are no statistics yet
    {
       	PTRACE2INT(eLevelInfoNormal,"CConfPartyMplMcmsProtocolTracer::TraceIpRtpCmVideoChannelsStatisticsInd, number of channels is:", st.unNumOfChannels);
       	return;
    }

    for(APIU32 i=0; i < st.unNumOfChannels; i++)
    {
        // check if channel monitoring is valid data.
        if (pChannelMonitoring->tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.bunValidChannel != (DWORD)0)
        {
            *pContentStr <<"-------------------------------------------\n";
            *pContentStr <<"MC Is valid channel: "<<pChannelMonitoring->tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.bunValidChannel<<"\n";
            *pContentStr << "Channel media: " << pChannelMonitoring->tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unChannelType;
            *pContentStr <<"\nChannel media type: ";
            ::GetChannelTypeName(pChannelMonitoring->tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unChannelType,*pContentStr);
            *pContentStr << "\nChannel direction (Numbers): " << pChannelMonitoring->tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unChannelDirection;
            *pContentStr <<"\nMC Channel Direction: ";
            ::GetChannelDirectionName(pChannelMonitoring->tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unChannelDirection,*pContentStr);
            *pContentStr <<"\nTicks Interval: "<<pChannelMonitoring->tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unTicksInterval<<"\n";
            *pContentStr <<"Media byte: "<<pChannelMonitoring->tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unMediaBytes<<"\n";
            *pContentStr <<"Protocol: "<<pChannelMonitoring->tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unProtocol<<"\n";
                if ((pChannelMonitoring->tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unChannelType == kIpVideoChnlType) ||
                                                                (pChannelMonitoring->tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unChannelType == kIpContentChnlType))
                {
                TRtpVideoChannelMonitoring * pVidChannelMonitoring = (TRtpVideoChannelMonitoring *)pChannelMonitoring;

                *pContentStr <<"MaxFrameRate: "<<pVidChannelMonitoring->unMaxFrameRate<<"\n";
                *pContentStr <<"MinFrameRate: "<<pVidChannelMonitoring->unMinFrameRate<<"\n";

                APIS32 videoMaxResolution = pVidChannelMonitoring->unVideoMaxResolution;
                if((videoMaxResolution >= kUnknownFormat) && (videoMaxResolution < kLastFormat))
                {
                    *pContentStr <<"Video Max Resolution: ";
                    CCapSetInfo capInfo;
                    *pContentStr << capInfo.GetFormatStr((EFormat)videoMaxResolution) << "\n";
                }

                APIS32 videoMinResolution = pVidChannelMonitoring->unVideoMinResolution;
                if((videoMinResolution >= kUnknownFormat) && (videoMinResolution < kLastFormat))
                {
                    *pContentStr <<"Video Min Resolution: ";
                    CCapSetInfo capInfo;
                    *pContentStr << capInfo.GetFormatStr((EFormat)videoMinResolution) << "\n";
                }

                *pContentStr <<"Video Max Width: "<<pVidChannelMonitoring->unVideoMaxWidth<<"\n";
                *pContentStr <<"Video Min Width: "<<pVidChannelMonitoring->unVideoMinWidth<<"\n";

                *pContentStr <<"Video Max Height: "<<pVidChannelMonitoring->unVideoMaxHeight<<"\n";
                *pContentStr <<"Video Min Height: "<<pVidChannelMonitoring->unVideoMinHeight<<"\n";

                TraceIpPartyMonitoringAdvance(*pContentStr,(void *)&(pVidChannelMonitoring->tAdvanceMonitoringResultsSt));
                }

        }
        else
            *pContentStr <<"Information is not valid for this specified channel: "<< i <<".\n";

        // Move the pointer to the next channel
        pChannelMonitoring = (TRtpChannelMonitoringInd *)((BYTE *)pChannelMonitoring + sizeof(TRtpVideoChannelMonitoring));
    }

    *pContentStr <<"-------------------------------------------\n";
}

////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceIpPartyMonitoringAdvance(CObjString& msg,void *pAdvanceMonitor)
{
	TAdvanceMonitoringResultsSt *pAdvance = (TAdvanceMonitoringResultsSt *)pAdvanceMonitor;

	TRtpStatisticSt rtpStatistic		= pAdvance->tRtpStatisticSt;
	TRtcpInfoSt rtcpInfo				= pAdvance->tRtcpInfoSt;

	msg <<"Rtcp Info: \n";
	msg <<"rtcpSrCnt: "<<rtcpInfo.unRtcpSrCnt<<"\n";
	msg <<"rtcpRrCnt: "<<rtcpInfo.unRtcpRrCnt<<"\n";
	msg <<"rtcpRrReceptionBlockCnt: "<<rtcpInfo.unRtcpRrReceptionBlockCnt<<"\n";
    msg <<"rtcpAccumulatedPacketLoss: "<<rtcpInfo.unAccumulatedPacketLoss <<"\n";
    msg <<"rtcpAccumulatedPacket: "<<rtcpInfo.unAccumulatedPacket <<"\n";
	msg <<"rtcpIntervalJitter: "<<rtcpInfo.unIntervalJitter <<"\n";
	msg <<"rtcpIntervalPeakJitter: "<<rtcpInfo.unIntervalPeakJitter <<"\n";
	msg <<"rtcpLatency: "<<rtcpInfo.unLatency <<"\n";
	msg <<"rtcpFractionLoss"<<rtcpInfo.unIntervalFractionLoss <<"\n";
	msg <<"rtcpPeakFractionLoss"<<rtcpInfo.unIntervalPeakFractionLoss <<"\n";

	msg <<"Accumulated: \n";
	msg <<"rtp Packet: "<<rtpStatistic.unAccumulatedPacket<<"\n";
	msg <<"rtp Packet Loss: "<<rtpStatistic.tPacketsActualLoss.unAccumulated <<"\n";
	msg <<"rtp Packet Out Of Order: "<<rtpStatistic.tPacketsOutOfOrder.unAccumulated <<"\n";
	msg <<"rtp Packet Fragmented: "<<rtpStatistic.tPacketsFragmented.unAccumulated <<"\n";
	msg <<"jitter Buffer Size: "<<rtpStatistic.tJitterBufferSize.unAccumulated <<"\n";
	msg <<"jitter Late Packet: "<<rtpStatistic.tJitterLatePackets.unAccumulated <<"\n";
	msg <<"jitter BitRate Over Flow: "<<rtpStatistic.tJitterOverFlows.unAccumulated <<"\n";
	msg <<"jitter Sample Packet Interval: "<<rtpStatistic.tJitterSamplePacketInterval.unAccumulated <<"\n";
	msg <<"error Repair: "<<rtpStatistic.tErrorResilienceRepairs.unAccumulated <<"\n";

	msg <<"Interval: \n";
	msg <<"rtp Packet: "<<rtpStatistic.unIntervalPacket <<"\n";
	msg <<"rtp Packet Loss: "<<rtpStatistic.tPacketsActualLoss.unInterval <<"\n";
	msg <<"rtp Out Of Order: "<<rtpStatistic.tPacketsOutOfOrder.unInterval <<"\n";
	msg <<"rtp Fragmented Packet: "<<rtpStatistic.tPacketsFragmented.unInterval <<"\n";
	msg <<"jitter Buffer size: "<<rtpStatistic.tJitterBufferSize.unInterval <<"\n";
	msg <<"jitter Late Packet: "<<rtpStatistic.tJitterLatePackets.unInterval <<"\n";
	msg <<"jitter Over Flow: "<<rtpStatistic.tJitterOverFlows.unInterval <<"\n";
	msg <<"jitter Sample Packet Interval: "<<rtpStatistic.tJitterSamplePacketInterval.unInterval <<"\n";
	msg <<"error Repair: "<<rtpStatistic.tErrorResilienceRepairs.unInterval <<"\n";

	msg <<"PEAKS:-----\n";
	msg <<"rtpPacketsIntervalPeakPacketLoss: "<<rtpStatistic.tPacketsActualLoss.unPeak <<"\n";
	msg <<"rtpPacketsIntervalPeakOutOfOrder: "<<rtpStatistic.tPacketsOutOfOrder.unPeak <<"\n";
	msg <<"rtpPacketsIntervalPeakFragmentedPacket: "<<rtpStatistic.tPacketsFragmented.unPeak <<"\n";
	msg <<"jitterMeIntervalPeakJitterBufferSize: "<<rtpStatistic.tJitterBufferSize.unPeak <<"\n";
	msg <<"jitterMeIntervalPeakLatePacket: "<<rtpStatistic.tJitterLatePackets.unPeak <<"\n";
	msg <<"jitterMeIntervalPeakOverFlow: "<<rtpStatistic.tJitterOverFlows.unPeak <<"\n";
	msg <<"jitterMeIntervalPeakSamplePacketInterval: "<<rtpStatistic.tJitterSamplePacketInterval.unPeak <<"\n";
	msg <<"errorReIntervalPeakRepair: "<<rtpStatistic.tErrorResilienceRepairs.unPeak <<"\n";

}

/////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceIpCmRtcpMsgInd(CObjString *pContentStr)
{
	*pContentStr << "\nCONTENT: IP_CM_RTCP_MSG_IND:";
	TraceIpCmRtcpMsg(pContentStr);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceIpCmRtcpMsgReq(CObjString *pContentStr)
{
	*pContentStr << "\nCONTENT: IP_CM_RTCP_MSG_REQ:";
	TraceIpCmRtcpMsg(pContentStr);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceIpCmRtcpMsg(CObjString *pContentStr)
{
	const TCmRtcpMsg *pSt = (const TCmRtcpMsg *)m_pMplMcmsProt->getpData();
	*pContentStr << "\nDspNumber    			: "<< (pSt->tCmRtcpHeader).uDspNumber << "\n";
	*pContentStr << "\nPortNumber   			: "<< (pSt->tCmRtcpHeader).uPortNumber<< "\n";

	*pContentStr << "\nChannel Type	      		:";
				::GetChannelTypeName((pSt->tCmRtcpMsgInfo).uMediaType,*pContentStr);
	*pContentStr << "\n Msg Type				:";
				::GetMsgTypeName((pSt->tCmRtcpMsgInfo).uMsgType,*pContentStr);
	*pContentStr << "\n Tip Position			:" << ::GetTipVideoPositionStr((ETipVideoPosition)((pSt->tCmRtcpMsgInfo).uTipPosition));
	*pContentStr << "\n SeqNum					:" << (pSt->tCmRtcpMsgInfo).uSeqNumber;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceUpdateMtPairReq(CObjString *pContentStr)
{
	const TUpdateMtPairReq *pSt = (const TUpdateMtPairReq *)m_pMplMcmsProt->getpData();

	*pContentStr <<"\nCONTENT: H323_RTP_UPDATE_MT_PAIR_REQ:"
				<< "\n Dest Mcu Id				 :" 	<< pSt->unDestMcuId
				<< "\n Dest Terminal Id			 :" 	<< pSt->unDestTerminalId;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceCmOpenUdpPortOrUpdateUdpAddrReq(CObjString *pContentStr, const char* OpcodeString)
{
  const TOpenUdpPortOrUpdateUdpAddrMessageStruct* pStructWithAll = (const TOpenUdpPortOrUpdateUdpAddrMessageStruct*)m_pMplMcmsProt->getpData();
  const mcReqCmOpenUdpPortOrUpdateUdpAddr& st = pStructWithAll->tCmOpenUdpPortOrUpdateUdpAddr;

  TraceMcmsMplPhysicalRsrcInfo(pContentStr, pStructWithAll->physicalPort);

  *pContentStr << "\nCONTENT: " << OpcodeString;
  *pContentStr << "\n  channelType                        :"; ::GetChannelTypeName(st.channelType, *pContentStr);
  *pContentStr << "\n  channelDirection                   :"; ::GetChannelDirectionName(st.channelDirection, *pContentStr);
  *pContentStr << "\n  CapTypeCode                        :" << CapEnumToString((CapEnum)st.capProtocolType);
  *pContentStr << "\n  CmLocalUdpAddressIp.transportType  :" << GetTransportTypeName(st.CmLocalUdpAddressIp.transportType);
  *pContentStr << "\n  CmLocalUdpAddressIp.ipVersion      :" << st.CmLocalUdpAddressIp.ipVersion;

  if (st.CmLocalUdpAddressIp.ipVersion == eIpVersion4)
  {
    *pContentStr << " (V4)";
    *pContentStr << "\n  CmLocalUdpAddressIp.addr.v4.ip     :";
    char str[30];
    sprintf(str, "%X", st.CmLocalUdpAddressIp.addr.v4.ip);
    *pContentStr << "0x" << str;
  }
  else
  {
    *pContentStr << " (V6)";
    *pContentStr << "\n  CmLocalUdpAddressIp.addr.v6.ip[16] :";
    char str[64];
    memset(str, '\0', 64);
    ::ipToString(st.CmLocalUdpAddressIp, str, 1);
    *pContentStr << str;
  }
  *pContentStr << "\n  CmLocalUdpAddressIp.port           :" << st.CmLocalUdpAddressIp.port;
  *pContentStr << "\n  Local RTCP port                    :" << st.LocalRtcpPort;
  *pContentStr << "\n  CmRemoteUdpAddressIp.transportType :" << GetTransportTypeName(st.CmRemoteUdpAddressIp.transportType);
  *pContentStr << "\n  CmRemoteUdpAddressIp.ipVersion     :" << st.CmRemoteUdpAddressIp.ipVersion;

  if (st.CmRemoteUdpAddressIp.ipVersion == eIpVersion4)
  {
    *pContentStr << " (V4)";
    *pContentStr << "\n  CmRemoteUdpAddressIp.addr.v4.ip    :";
    char str[30];
    sprintf(str, "%X", st.CmRemoteUdpAddressIp.addr.v4.ip);
    *pContentStr << "0x" << str;
  }
  else
  {
    *pContentStr << "V6";
    *pContentStr << "\n CmRemoteUdpAddressIp.addr.v6.ip[16] :";
    char str[64];
    memset(str, '\0', 64);
    ::ipToString(st.CmRemoteUdpAddressIp, str, 1);
    *pContentStr << str;
  }
  *pContentStr << "\n  CmRemoteUdpAddressIp.port          :" << st.CmRemoteUdpAddressIp.port;
  *pContentStr << "\n  Remote RTCP port                   :" << st.RemoteRtcpPort;
  *pContentStr << "\n  tosValue[MEDIA_TOS_VALUE_PLACE]    :" << st.tosValue[MEDIA_TOS_VALUE_PLACE];
  *pContentStr << "\n  tosValue[RTCP_TOS_VALUE_PLACE]     :" << st.tosValue[RTCP_TOS_VALUE_PLACE];
  *pContentStr << "\n  uRtpKeepAlivePeriod     			  :" << st.uRtpKeepAlivePeriod;
  *pContentStr << "\n  Ice channel RTP id                 :" << st.ice_channel_rtp_id;
  *pContentStr << "\n  Ice channel RTCP id                :" << st.ice_channel_rtcp_id;
  char strMask[30];
sprintf(strMask,"%x",st.RtcpCnameMask);
*pContentStr<< "\n CNAME RTCP mask: ";
  *pContentStr << " 0x" << strMask;
  *pContentStr <<" \n  connMode						  :" << GetConnectionTransportModeName(st.connMode);

  TraceSdesSpecificParams(pContentStr, st.sdesCap);
  *pContentStr << "\n  Media Detection Time          :" << st.ulDetectionTimerLen;
  *pContentStr << "\n  Msft EP Type          		 :" << st.uMsftType;

}

//////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceCmCloseUdpPortReq(CObjString *pContentStr,  const char* OpcodeString)
{
  const TCloseUdpPortMessageStruct* pStructWithAll = (const TCloseUdpPortMessageStruct*)m_pMplMcmsProt->getpData();

  mcReqCmCloseUdpPort st = pStructWithAll->tCmCloseUdpPort;

  TraceMcmsMplPhysicalRsrcInfo(pContentStr, pStructWithAll->physicalPort);

  *pContentStr << "\nCONTENT: " << OpcodeString;
  *pContentStr << "\n  channelType                        :"; ::GetChannelTypeName(st.channelType, *pContentStr);
  *pContentStr << "\n  channelDirection                   :"; ::GetChannelDirectionName(st.channelDirection, *pContentStr);
  *pContentStr << "\n  CmLocalUdpAddressIp.transportType  :" << GetTransportTypeName(st.CmLocalUdpAddressIp.transportType);
  *pContentStr << "\n  CmLocalUdpAddressIp.ipVersion      :" << st.CmLocalUdpAddressIp.ipVersion;

  if (st.CmLocalUdpAddressIp.ipVersion == eIpVersion4)
  {
    *pContentStr << " (V4)";
    *pContentStr << "\n  CmLocalUdpAddressIp.addr.v4.ip     :";
    char str[30];
    sprintf(str, "%X", st.CmLocalUdpAddressIp.addr.v4.ip);
    *pContentStr << "0x" << str;
  }
  else
  {
    *pContentStr << " (V6)";
    *pContentStr << "\n  CmLocalUdpAddressIp.addr.v6.ip[16] :";
    char str[64];
    memset(str, '\0', 64);
    ::ipToString(st.CmLocalUdpAddressIp, str, 1);
    *pContentStr << str;
  }
  *pContentStr << "\n  CmLocalUdpAddressIp.port           :" << st.CmLocalUdpAddressIp.port;
  *pContentStr << "\n  Local RTCP port                    :" << st.LocalRtcpPort;
  *pContentStr << "\n  CmRemoteUdpAddressIp.transportType :" << GetTransportTypeName(st.CmRemoteUdpAddressIp.transportType);
  *pContentStr << "\n  CmRemoteUdpAddressIp.ipVersion     :" << st.CmRemoteUdpAddressIp.ipVersion;

  if (st.CmRemoteUdpAddressIp.ipVersion == eIpVersion4)
  {
    *pContentStr << " (V4)";
    *pContentStr << "\n  CmRemoteUdpAddressIp.addr.v4.ip    :";
    char str[30];
    sprintf(str, "%X", st.CmRemoteUdpAddressIp.addr.v4.ip);
    *pContentStr << "0x" << str;
  }
  else
  {
    *pContentStr << "V6";
    *pContentStr << "\n CmRemoteUdpAddressIp.addr.v6.ip[16] :";
    char str[64];
    memset(str, '\0', 64);
    ::ipToString(st.CmRemoteUdpAddressIp, str, 1);
    *pContentStr << str;
  }
  *pContentStr << "\n  CmRemoteUdpAddressIp.port          :" << st.CmRemoteUdpAddressIp.port;
  *pContentStr << "\n  Remote RTCP port                   :" << st.RemoteRtcpPort;
  *pContentStr << "\n  Ice channel RTP id                 :" << st.ice_channel_rtp_id;
  *pContentStr << "\n  Ice channel RTCP id                :" << st.ice_channel_rtcp_id;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceRtpStreamOnReq(CObjString *pContentStr, const char* OpcodeString)
{
  const TStreamOnReq* pStruct = (const TStreamOnReq*)m_pMplMcmsProt->getpData();

  *pContentStr << "\nCONTENT: " << OpcodeString << ":";
  *pContentStr << "\n  ChannelType           :"; ::GetChannelTypeName(pStruct->unChannelType, *pContentStr);
  *pContentStr << "\n  ChannelDirection      :"; ::GetChannelDirectionName(pStruct->unChannelDirection, *pContentStr);
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceRtpStreamOffReq(CObjString *pContentStr, const char* OpcodeString)
{
  const TStreamOffReq* pStruct = (const TStreamOffReq*)m_pMplMcmsProt->getpData();

  *pContentStr << "\nCONTENT: " << OpcodeString << ":";
  *pContentStr << "\n  ChannelType           :"; ::GetChannelTypeName(pStruct->unChannelType, *pContentStr);
  *pContentStr << "\n  ChannelDirection      :"; ::GetChannelDirectionName(pStruct->unChannelDirection, *pContentStr);
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceRtpContentOnOffReq(CObjString *pContentStr, const char* OpcodeString)
{
  const TContentOnOffReq* pStruct = (const TContentOnOffReq*)m_pMplMcmsProt->getpData();

  *pContentStr << "\nCONTENT: " << OpcodeString << ":";
  *pContentStr << "\n  ChannelDirection      :"; ::GetChannelDirectionName(pStruct->unChannelDirection, *pContentStr);
  *pContentStr << "\n  ChannelType           :" << ((pStruct->bunIsOnOff == 1) ? "ON" : "OFF");
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceRtpEvacuateStreamReq(CObjString *pContentStr, const char* OpcodeString)
{
  const TEvacuateReq* pStruct = (const TEvacuateReq*)m_pMplMcmsProt->getpData();

  *pContentStr << "\nCONTENT: " << OpcodeString << ":";
  *pContentStr << "\n  ChannelType           :"; ::GetChannelTypeName(pStruct->unChannelType, *pContentStr);
  *pContentStr << "\n  ChannelDirection      :"; ::GetChannelDirectionName(pStruct->unChannelDirection, *pContentStr);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceRtpUpdatePortOpenChannelReq(CObjString* pContentStr, const char* OpcodeString)
{
  const TUpdatePortOpenRtpChannelReq *pStruct = (const TUpdatePortOpenRtpChannelReq *)m_pMplMcmsProt->getpData();

  *pContentStr << "\nCONTENT:";
  *pContentStr << "\n  ChannelType                               :"; ::GetChannelTypeName(pStruct->unChannelType, *pContentStr);
  *pContentStr << "\n  ChannelDirection                          :"; ::GetChannelDirectionName(pStruct->unChannelDirection, *pContentStr);
  *pContentStr << "\n  MediaMode                                 :"; ::GetMediaModeName(pStruct->mediaMode, *pContentStr);
  *pContentStr << "\n  CapTypeCode                               :" << CapEnumToString((CapEnum)pStruct->unCapTypeCode);
  *pContentStr << "\n  updateSsrcParams.bReplaceSSRC             :" <<  (pStruct->updateSsrcParams.bReplaceSSRC?  "TRUE" : "FALSE");
  *pContentStr << "\n  unEncryptionType                          :" << pStruct->unEncryptionType;
  *pContentStr << "\n  updateSsrcParams.bMuteStream              :" <<  (pStruct->bMuteStream?  "TRUE" : "FALSE");
  *pContentStr << "\n  updateSsrcParams.unMsFirstSyncSourceInRange   :" <<  pStruct->updateSsrcParams.unMsFirstSyncSourceInRange;
  *pContentStr << "\n  updateSsrcParams.unMsLastSyncSourceInRange   :" <<  pStruct->updateSsrcParams.unMsLastSyncSourceInRange;

  if (pStruct->updateSsrcParams.bReplaceSSRC)
  {
      *pContentStr << "\n  updateSsrcParams.unUpdatedSSRC  :" << pStruct->updateSsrcParams.unUpdatedSSRC;
  }
  *pContentStr << "\n  updateSsrcParams.bReplaceCSRC   :" <<  (pStruct->updateSsrcParams.bReplaceCSRC?  "TRUE" : "FALSE");
  if (pStruct->updateSsrcParams.bReplaceCSRC)
  {
      *pContentStr << "\n  updateSsrcParams.unUpdatedCSRC  :" << pStruct->updateSsrcParams.unUpdatedCSRC;
  }
  *pContentStr << "\n  Do Not Change SSRC     	 	   	  	     :" << ( (TRUE == pStruct->updateSsrcParams.bDoNotChangeSsrc) ? "TRUE" : "FALSE");
  TraceRtpUpdateRtpSpecificChannelParamsReq(pContentStr, pStruct->tUpdateRtpSpecificChannelParams);
  TraceLprSpecificParams(pContentStr, pStruct->tLprSpecificParams);
  TraceSdesSpecificParams(pContentStr, pStruct->sdesCap);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceRtpUpdateChannelReq(CObjString* pContentStr, const char* OpcodeString)
{
  const TUpdateRtpChannelReq* pStruct = (const TUpdateRtpChannelReq*)m_pMplMcmsProt->getpData();

  *pContentStr << "\nCONTENT:";
  *pContentStr << "\n  ChannelType           :"; ::GetChannelTypeName(pStruct->unChannelType, *pContentStr);
  *pContentStr << "\n  ChannelDirection      :"; ::GetChannelDirectionName(pStruct->unChannelDirection, *pContentStr);
  *pContentStr << "\n  MediaMode             :"; ::GetMediaModeName(pStruct->mediaMode, *pContentStr);
  *pContentStr << "\n  unEncryptionType   :" << pStruct->unEncryptionType;
  TraceRtpUpdateRtpSpecificChannelParamsReq(pContentStr, pStruct->tUpdateRtpSpecificChannelParams);
  TraceSdesSpecificParams(pContentStr, pStruct->sdesCap);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceRtpUpdateRtpSpecificChannelParamsReq(CObjString *pContentStr, const TUpdateRtpSpecificChannelParams& _struct)
{
  *pContentStr << "\n  BitRate               :" << _struct.unBitRate
               << "\n  PayloadType           :" << _struct.unPayloadType
               << "\n  FecRedPayloadType     :" << _struct.unFecRedPayloadType
               << "\n  DtmfPayloadType       :" << _struct.unDtmfPayloadType
               << "\n  IsH263Plus            :" << _struct.bunIsH263Plus
               << "\n  IsFlipIntraBit        :" << _struct.bunIsFlipIntraBit
               << "\n  AnnexesMask           :" << _struct.unAnnexesMask
               << "\n  HcMpiQCif             :" << _struct.nHcMpiQCif
               << "\n  HcMpiCif              :" << _struct.nHcMpiCif
               << "\n  HcMpi4Cif             :" << _struct.nHcMpi4Cif
               << "\n  HcMpi16Cif            :" << _struct.nHcMpi16Cif
               << "\n  HcMpiVga              :" << _struct.nHcMpiVga
               << "\n  HcMpiNtsc             :" << _struct.nHcMpiNtsc
               << "\n  HcMpiSvga             :" << _struct.nHcMpiSvga
               << "\n  HcMpiXga              :" << _struct.nHcMpiXga
               << "\n  HcMpiSif              :" << _struct.nHcMpiSif
               << "\n  HcMpiQvga             :" << _struct.nHcMpiQvga
               << "\n  CustomMaxMbpsValue    :" << _struct.unCustomMaxMbpsValue
               << "\n  MaxFramesPerPacket    :" << _struct.unMaxFramesPerPacket
               << "\n  DestMcuId             :" << _struct.unDestMcuId
               << "\n  DestTerminalId        :" << _struct.unDestTerminalId
               << "\n  ContentEnabled        :" << _struct.bunContentEnabled
               << "\n  PacketPayloadFormat   :" << _struct.unPacketPayloadFormat
               << "\n  ReqEndOfFrameParsing  :" << (_struct.b32StreamRequiresEndOfFrameParsing ? "TRUE" : "FALSE")
               << "\n  IsFecRedOn            :" << _struct.bIsFecRedOn
               << "\n  bnTipIsEnabled        		:" << _struct.openTipPortParams.bnTipIsEnabled
               << "\n  eTipPosition          		:" << _struct.openTipPortParams.eTipPosition
			   << "\n  unMsftClient		     		:" << _struct.unMsftClient
			   << "\n  b32TrafficShapingEnabled		:" << _struct.b32TrafficShapingEnabled
			   << "\n  unTrafficShapingWindowSize	:" << _struct.unTrafficShapingWindowSize;

}

//////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceRtpUpdateChannelRateReq(CObjString* pContentStr, const char* OpcodeString)
{
	const TUpdateRtpChannelRateReq *pStruct = (const TUpdateRtpChannelRateReq *)m_pMplMcmsProt->getpData();

	*pContentStr <<"\nCONTENT: " << OpcodeString << ":"

				<< "\n ChannelType       :";
				::GetChannelTypeName(pStruct->unChannelType,*pContentStr);

	*pContentStr<< "\n ChannelDirection  :";
				::GetChannelDirectionName(pStruct->unChannelDirection,*pContentStr);

	*pContentStr<< "\n New Rate			 :"
				<< pStruct->unNewChannelRate;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceRtpFeccTokenResponseReq(CObjString *pContentStr)
{
	const TFeccTokenResponseReq *pSt = (const TFeccTokenResponseReq *)m_pMplMcmsProt->getpData();

	static CLargeString physicalPortStr;
	physicalPortStr.Clear();

	TraceMcmsMplPhysicalRsrcInfo(&physicalPortStr, pSt->tDestTerminalPhysicalRtp);


	*pContentStr <<"\nCONTENT: H323_RTP_FECC_TOKEN_RESPONSE_REQ:";

	*pContentStr << "\n Response					:";	::GetFeccTokenResponseName(pSt->unResponse,*pContentStr);
	*pContentStr << "\n Dest Mcu Id					:" 	<< pSt->unDestMcuId
				<< "\n Dest Terminal Id				:" 	<< pSt->unDestTerminalId
				<< "\n Is camera control?			:" 	<< pSt->unIsCameraControl
				<< "\n Dest Party Id				:" 	<< pSt->tDestTerminalPhysicalRtp.party_id
				<< "\n--physical_port-----------------:" << physicalPortStr;


}

/////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceRtpLprModeChangeReq(CObjString* pContentStr, const char* OpcodeString)
{
	const TLprModeChangeReq *pStruct = (const TLprModeChangeReq *)m_pMplMcmsProt->getpData();

	*pContentStr <<"\nCONTENT: " << OpcodeString << ":"

				<< "\n ChannelType       :";
				::GetChannelTypeName(pStruct->unChannelType,*pContentStr);

	*pContentStr<< "\n ChannelDirection  :";
				::GetChannelDirectionName(pStruct->unChannelDirection,*pContentStr);

	*pContentStr<< "\n LossProtection	 :"
				<< pStruct->usLossProtection;
	*pContentStr<< "\n MTBF				 :"
				<< pStruct->unMTBF;
	*pContentStr<< "\n CongestionCeiling :"
				<< pStruct->unCongestionCeiling;
	*pContentStr<< "\n ModeTimeout	 :"
				<< pStruct->usModeTimeout;

}
/////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceIpRtpFeccTokenInd(CObjString *pContentStr)
{
	const TRtpFeccTokenRequestInd *pSt = (const TRtpFeccTokenRequestInd *)m_pMplMcmsProt->getpData();

	*pContentStr <<"\nCONTENT: IP_RTP_FECC_TOKEN_IND:"
				<< "\n Channel Type				 :";
				::GetChannelTypeName(pSt->unChannelType,*pContentStr);
	*pContentStr << "\n Channel Direction		 :";
	 			::GetChannelDirectionName(pSt->unChannelDirection,*pContentStr);
	*pContentStr<< "\n Token Opcode			     :" 	<< pSt->unTokenOpcode;
	*pContentStr<< "\n Is camera control?		 :" 	<< pSt->unIsCameraControl;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceIpRtpFeccKeyInd(CObjString *pContentStr)
{
	const TRtpFeccTokenRequestInd *pSt = (const TRtpFeccTokenRequestInd *)m_pMplMcmsProt->getpData();

	*pContentStr << "\nCONTENT: IP_RTP_FECC_KEY_IND:"
				 << "\nkey    : " 	<< ::feccKeyToString(feccKeyEnum(pSt->unTokenOpcode));
}
/////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceIpRtpDifferentPayloadTypeInd(CObjString *pContentStr)
{
	const TRtpDiffPayloadTypeInd *pSt = (const TRtpDiffPayloadTypeInd *)m_pMplMcmsProt->getpData();

	*pContentStr <<"\nCONTENT: IP_RTP_DIFF_PAYLOAD_TYPE_IND:"
				<< "\n Channel Type				 :";
				::GetChannelTypeName(pSt->unChannelType,*pContentStr);
	*pContentStr << "\n Channel Direction		 :";
	 			::GetChannelDirectionName(pSt->unChannelDirection,*pContentStr);
	*pContentStr<< "\n PayloadType			     :" 	<< pSt->unPayloadType;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceIpRtpFeccMuteInd(CObjString *pContentStr)
{
	const TRtpFeccMuteInd *pSt = (const TRtpFeccMuteInd *)m_pMplMcmsProt->getpData();

	*pContentStr <<"\nCONTENT: IP_RTP_FECC_MUTE_IND:"
				<< "\n IsMuteOn				 : " << (int)(pSt->unIsMuteOn) << (((int)(pSt->unIsMuteOn)==1)?" (MUTE) ":" (UNMUTE) ");
}
/////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceIpRtpVideoFastUpdateInd(CObjString *pContentStr)
{
	const TRtpVideoUpdatePictureInd *pSt = (const TRtpVideoUpdatePictureInd *)m_pMplMcmsProt->getpData();

	*pContentStr <<"\nCONTENT: IP_RTP_VIDEO_UPDATE_PIC_IND:"
				<< "\n Channel Type				 :";
				::GetChannelTypeName(pSt->unChannelType,*pContentStr);
	*pContentStr << "\n Channel Direction		 :";
	 			::GetChannelDirectionName(pSt->unChannelDirection,*pContentStr);
	*pContentStr << "\n Sequence number  		 :" << pSt->uSeqNumber;
	*pContentStr << "\n SSrcID					 :"	<< pSt->unSsrcID;
	*pContentStr << "\n PriorityID				 :"	<< pSt->unPrID;

}

/////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceIpRtpBadSpontaneousInd(CObjString *pContentStr)
{
	const TRtpBadSpontaneousInd *pSt = (const TRtpBadSpontaneousInd *)m_pMplMcmsProt->getpData();

	*pContentStr <<"\nCONTENT: IP_RTP_BAD_SPONTAN_IND:"
				<< "\n Channel Type				 :";
				::GetChannelTypeName(pSt->unChannelType,*pContentStr);
	*pContentStr<< "\n Channel Direction		 :";
		 		::GetChannelDirectionName(pSt->unChannelDirection,*pContentStr);
	if(pSt->unBadSpontReason < lastBadSpontanIndReason)
		*pContentStr <<	"\n Bad Spontanues Reason		 : " << g_badSpontanIndReasonStrings[pSt->unBadSpontReason];
	else
		*pContentStr <<	"\n Bad Spontanues Reason		 : UNKNOWN";
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceIpRtpStreamStatusInd(CObjString *pContentStr)
{
	const TRtpStreamStatusInd *pSt = (const TRtpStreamStatusInd *)m_pMplMcmsProt->getpData();

	*pContentStr <<"\nCONTENT: IP_RTP_STREAM_STATUS_IND:"
				<< "\n Channel Type				 :";
				::GetChannelTypeName(pSt->unChannelType,*pContentStr);
	*pContentStr << "\n Channel Direction		 :";
			 	::GetChannelDirectionName(pSt->unChannelDirection,*pContentStr);
	if(pSt->unStreamStatus)
	{
		*pContentStr << "\n Status					 :"		<< pSt->unStreamStatus << " (Strean Violation)";
		*pContentStr << "\n Violation				 :"		<< pSt->bunViolation;
		*pContentStr << "\n PayloadType				 :"		<< pSt->unPayloadType;
		*pContentStr << "\n AnnexesAndResolution	 :"		<< pSt->unAnnexesAndResolutionMask;
		*pContentStr << "\n Resolution				 :"		<< pSt->unResolution;
		*pContentStr << "\n BitRate				 	 :"		<< pSt->unBitRate;
		*pContentStr << "\n FramesPerSec			 :"		<< pSt->unFramesPerSec;
	}
	else
		*pContentStr << "\n Status					 : Status OK";
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceIpRtpLprModeChangeInd(CObjString *pContentStr)
{
	const TLprModeChangeInd *pSt = (const TLprModeChangeInd *)m_pMplMcmsProt->getpData();

	*pContentStr <<"\nCONTENT: IP_RTP_LPR_MODE_CHANGE_IND:"
				<< "\n Channel Type				 :";
				::GetChannelTypeName(pSt->unChannelType,*pContentStr);
	*pContentStr << "\n Channel Direction		 :";
			 	::GetChannelDirectionName(pSt->unChannelDirection,*pContentStr);
	*pContentStr<< "\n LossProtection	 :"
				<< pSt->usLossProtection;
	*pContentStr<< "\n MTBF				 :"
				<< pSt->unMTBF;
	*pContentStr<< "\n CongestionCeiling :"
				<< pSt->unCongestionCeiling;
	*pContentStr<< "\n ModeTimeout	 :"
				<< pSt->usModeTimeout;

}

/////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceIpRtpDbaInd(CObjString *pContentStr)
{
	const TDbaInd *pSt = (const TDbaInd *)m_pMplMcmsProt->getpData();

	*pContentStr <<"\nCONTENT: IP_RTP_DBA_IND:"
				<< "\n Channel Type				 :";
				::GetChannelTypeName(pSt->unChannelType,*pContentStr);
	*pContentStr << "\n Channel Direction		 :";
			 	::GetChannelDirectionName(pSt->unChannelDirection,*pContentStr);
	*pContentStr<< "\n ReductionExpansion :"
				<< pSt->usReductionExpansion;
	*pContentStr<< "\n RateFactor	 :"
				<< pSt->usRateFactor;

}

//////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceGraphicsShowTextBoxParams(CObjString* pContentStr)
{
	const char *pPresentationCharArray[] = {"dummy", "static", "down", "up"};
	const char *pPositionCharArray[] = {"up left", "up center", "up right", "middle left", "middle center", "middle right", "down left", "down center", "down right"};
	const char *pAlignmentCharArray[] = {"dummy", "right", "left", "center"};
	const char *pSpeedCharArray[] = {"dummy", "static", "slow", "fast"};
	const char *pFontSizeCharArray[] = {"dummy", "small", "medium", "large"};
	//const char *pAlignmentCharArray[] = {"dummy", "right", "left", "center"};

	const TEXT_BOX_DISPLAY_S *pTextBoxDisplayStruct = (const TEXT_BOX_DISPLAY_S *)m_pMplMcmsProt->getpData();

	if(pTextBoxDisplayStruct->tTextBoxParams.nNumberOfTextLines > MAX_TEXT_LINES_IN_MESSAGE)
	{
		*pContentStr <<"\nCONTENT: TEXT_BOX_DISPLAY_S: \n Number of text lines exceeded the possible values!!";
		return;
	}

	*pContentStr <<"\nCONTENT: TEXT_BOX_DISPLAY_S: \n Number of text lines     :"
                << pTextBoxDisplayStruct->tTextBoxParams.nNumberOfTextLines
				<<" \n PresentationMode         :"
				<< pPresentationCharArray[pTextBoxDisplayStruct->tTextBoxParams.nPresentationMode]
				<<" \n Position         :"
				<< pPositionCharArray[pTextBoxDisplayStruct->tTextBoxParams.nPosition]
	            <<" \n Speed: " << pSpeedCharArray[pTextBoxDisplayStruct->tTextBoxParams.nDisplaySpeed]
				<<"\n RepetitionCount: " << pTextBoxDisplayStruct->tTextBoxParams.nMessageRepetitionCount;

	if(pTextBoxDisplayStruct->tTextBoxParams.nPosition == -1)
	{
		*pContentStr <<" \n pTextBoxDisplayStruct.tTextBoxParams.tTextBoxSizeLocation         :"
		 <<" \n Upper Left X - " << pTextBoxDisplayStruct->tTextBoxParams.tTextBoxSizeLocation.nUpperLeftX
		 <<", Upper Left Y - " << pTextBoxDisplayStruct->tTextBoxParams.tTextBoxSizeLocation.nUpperLeftY
		 <<", Width - " << pTextBoxDisplayStruct->tTextBoxParams.tTextBoxSizeLocation.nWidth
		 <<", Height - " << pTextBoxDisplayStruct->tTextBoxParams.tTextBoxSizeLocation.nHeight;
	}

	for(int i=0; i < pTextBoxDisplayStruct->tTextBoxParams.nNumberOfTextLines; i++)
	{
		*pContentStr <<"\n Text Line Number["<<i<<"] Info: ";
		if(strlen(pTextBoxDisplayStruct->atTextLineParams[i].acTextLine) <= MAX_SITE_NAME_SIZE)
		{
			*pContentStr <<"\n\t Color: " << pTextBoxDisplayStruct->atTextLineParams[i].nTextColor
			<<", Background Color: " << pTextBoxDisplayStruct->atTextLineParams[i].nBackgroundColor
			<<", Transparency: " << pTextBoxDisplayStruct->atTextLineParams[i].nTransparency
			<<", ShadowWidth: " << pTextBoxDisplayStruct->atTextLineParams[i].nShadowWidth
			<<", Alignment: " << pAlignmentCharArray[pTextBoxDisplayStruct->atTextLineParams[i].nAlignment]
			<<", FontType: " << pTextBoxDisplayStruct->atTextLineParams[i].nFontType /* NULL, BOLD, ITALIC, UNDERLINE, STRIKEOUT	*/
			<<", Text: " << (pTextBoxDisplayStruct->atTextLineParams[i].acTextLine)   /*cant convert text since its in UCS2 format*/
			<<", FontSize: " << pFontSizeCharArray[pTextBoxDisplayStruct->atTextLineParams[i].nFontSize];

		}
		else
			*pContentStr <<"\n Text Line Number["<<i<<"] WRONG LINE INFO!!";
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
/*void CConfPartyMplMcmsProtocolTracer::TracePartyKeepAliveInd(CLargeString* pContentStr)
{
	mcIndPartyKeepAlive PartyKeepAliveInd;
	memcpy(&PartyKeepAliveInd, m_pMplMcmsProt->getpData(),sizeof(mcIndPartyKeepAlive));

	*pContentStr << "\nCONTENT: H323_CS_PARTY_KEEP_ALIVE_IND: \n"
				 << "Status : ";
	if (PartyKeepAliveInd.status == 0)
		*pContentStr << "STATUS_OK \n";
	else
		*pContentStr << "STATUS_FAIL \n";

}*/
////////////////////////////////////////////////////////////////////////////////////////////////

void CConfPartyMplMcmsProtocolTracer::TraceBondingInitReq(CObjString *pContentStr)
{
	BND_CONNECTION_INIT_REQUEST_S* sBondingInit = (BND_CONNECTION_INIT_REQUEST_S*)m_pMplMcmsProt->getpData();

    TraceBondingCallParams(pContentStr,sBondingInit->callParams);
    TraceBondingNegotiationParams(pContentStr,sBondingInit->negotiationParams);
    if(sBondingInit->callParams.direction==0) //dial in
    {
        TraceBondingPhoneNumber(pContentStr,sBondingInit->additional_dial_in_phone_num);
    }




// typedef struct
// {
//   BND_CALL_PARAMS_S callParams;
//   BND_NEGOTIATION_PARAMS_S negotiationParams;
//   BND_PHONE_NUM_S additional_dial_in_phone_num;
// } BND_CONNECTION_INIT_REQUEST_S;


    {//============= Ron - hex trace of messages ===================

        char* data_buff = (char*) m_pMplMcmsProt->getpData();
		const WORD  msgBufSize = 1024;//8192;
        char*       msgStr = new char[msgBufSize];
        memset(msgStr,'\0',1024);
        char        temp[16];
        memset(temp,'\0',16);
        int buflen = 8; // BND_CALL_PARAMS_S=4 + BND_NEGOTIATION_PARAMS_S=4 (for dial in + BND_PHONE_NUM_S=8)

		for (int byte_index = 0; byte_index < buflen; byte_index++) {
			if (byte_index==0)
				sprintf(temp,"{0x%02x",(char)(data_buff[byte_index]));
			else if (byte_index == (buflen-1))
				sprintf(temp,",0x%02x}",(char)(data_buff[byte_index]));
			else
				sprintf(temp,",0x%02x",(char)(data_buff[byte_index]));


            strcat(msgStr,temp);
		}
        FPTRACE(eLevelInfoNormal,msgStr);
        delete [] msgStr;
		} //============= olga ==================



}
////////////////////////////////////////////////////////////////////////////////////////////////

void CConfPartyMplMcmsProtocolTracer::TraceBondingAddChannelReq(CObjString *pContentStr)
{
	BND_ADD_CHANNEL_REQUEST_S* sBondingAddChannel = (BND_ADD_CHANNEL_REQUEST_S*)m_pMplMcmsProt->getpData();

    TraceBondingCallParams(pContentStr,sBondingAddChannel->callParams);

//     typedef struct
//     {
//         BND_CALL_PARAMS_S callParams;
//     } BND_ADD_CHANNEL_REQUEST_S;

    {//============= Ron - hex trace of messages ===================

        char* data_buff = (char*) m_pMplMcmsProt->getpData();
		const WORD  msgBufSize = 1024;//8192;
        char*       msgStr = new char[msgBufSize];
        memset(msgStr,'\0',1024);
        char        temp[16];
        memset(temp,'\0',16);
        int buflen = 4; // BND_CALL_PARAMS_S=4

		for (int byte_index = 0; byte_index < buflen; byte_index++) {
			if (byte_index==0)
				sprintf(temp,"{0x%02x",(char)(data_buff[byte_index]));
			else if (byte_index == (buflen-1))
				sprintf(temp,",0x%02x}",(char)(data_buff[byte_index]));
			else
				sprintf(temp,",0x%02x",(char)(data_buff[byte_index]));


            strcat(msgStr,temp);
		}
        FPTRACE(eLevelInfoNormal,msgStr);
        delete [] msgStr;
		} //============= olga ==================

}
////////////////////////////////////////////////////////////////////////////////////////////////

void CConfPartyMplMcmsProtocolTracer::TraceBondingEndNegotiationInd(CObjString *pContentStr)
{
    BND_END_NEGOTIATION_INDICATION_S* sBondingEndNegotiation = (BND_END_NEGOTIATION_INDICATION_S*)m_pMplMcmsProt->getpData();
    TraceBondingCallParams(pContentStr,sBondingEndNegotiation->callParams);
    if(sBondingEndNegotiation->callParams.direction==1) //dial out
    {
        TraceBondingPhoneNumberList(pContentStr,sBondingEndNegotiation->phoneList);
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////

void CConfPartyMplMcmsProtocolTracer::TraceBondingRequesParamsInd(CObjString *pContentStr)
{
    BND_REQ_PARAMS_INDICATION_S* sBondingRequestParams = (BND_REQ_PARAMS_INDICATION_S*)m_pMplMcmsProt->getpData();
    TraceBondingCallParams(pContentStr,sBondingRequestParams->callParams);
}
 ////////////////////////////////////////////////////////////////////////////////////////////////

void CConfPartyMplMcmsProtocolTracer::TraceResponseBondingAckParamsReq(CObjString *pContentStr)
{
    BND_END_NEGOTIATION_INDICATION_S* sBondingEndNegotiation = (BND_END_NEGOTIATION_INDICATION_S*)m_pMplMcmsProt->getpData();
    TraceBondingCallParams(pContentStr,sBondingEndNegotiation->callParams);
    TraceBondingPhoneNumberList(pContentStr,sBondingEndNegotiation->phoneList);
}
////////////////////////////////////////////////////////////////////////////////////////////////

void CConfPartyMplMcmsProtocolTracer::TraceBondingRemoteLocalAlignmentInd(CObjString *pContentStr)
{
      BND_REMOTE_LOCAL_ALIGNMENT_INDICATION_S* sBondingRemoteLocalAlignment = (BND_REMOTE_LOCAL_ALIGNMENT_INDICATION_S*)m_pMplMcmsProt->getpData();

    *pContentStr<< "indicator............." << (*sBondingRemoteLocalAlignment).indicator << "\n";
    *pContentStr<< "numOfChannels........." << (*sBondingRemoteLocalAlignment).numOfChannels << "\n";
//    *pContentStr<< "DownSpeedStatus......." << (*sBondingRemoteLocalAlignment).DownSpeedStatus << "\n";
//    *pContentStr<< "AlignedChannelsMask..." << (*sBondingRemoteLocalAlignment).AlignedChannelsMask << "\n";
}

////////////////////////////////////////////////////////////////////////////////////////////////

void CConfPartyMplMcmsProtocolTracer::TraceBondingCallParams(CObjString *pContentStr,BND_CALL_PARAMS_S& callParams )
{
    *pContentStr<< "direction.................";
    if(callParams.direction==1){
        *pContentStr << "dial-out";
    }else if (callParams.direction==0){
        *pContentStr << "dial-in";
    }else{
        *pContentStr << callParams.direction;
    }
    *pContentStr<< "\n";
    *pContentStr<< "NumOfBndChnls............." << (WORD)callParams.NumOfBndChnls << "\n";
    *pContentStr<< "restrictType..............";
    if(callParams.restrictType==0){
        *pContentStr << "restricted";
    }else if (callParams.restrictType==1){
        *pContentStr << "not restricted";
    }else{
        *pContentStr << callParams.restrictType;
    }
    *pContentStr<< "\n";
}
////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceBondingNegotiationParams(CObjString *pContentStr,BND_NEGOTIATION_PARAMS_S& negotiationParams )
{
    *pContentStr<< "channelWidthNegotiation...";
    if(negotiationParams.channelWidthNegotiation==1){
        *pContentStr << "negotiable";
    }else if (negotiationParams.channelWidthNegotiation==0){
        *pContentStr << "non negotiable";
    }else{
        *pContentStr << negotiationParams.channelWidthNegotiation;
    }
    *pContentStr<< "\n";
    *pContentStr<< "restrictTypeNegotiation...";
    if(negotiationParams.restrictTypeNegotiation==1){
        *pContentStr << "negotiable";
    }else if (negotiationParams.restrictTypeNegotiation==0){
        *pContentStr << "non negotiable";
    }else{
        *pContentStr << negotiationParams.restrictTypeNegotiation;
    }
    *pContentStr<< "\n";
    *pContentStr<< "IsDownspeedSupport........";
    if(negotiationParams.IsDownspeedSupport==1){
        *pContentStr << "supported";
    }else if (negotiationParams.IsDownspeedSupport==0){
        *pContentStr << "not supported";
    }else{
        *pContentStr << negotiationParams.IsDownspeedSupport;
    }
    *pContentStr<< "\n";
}
////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceBondingPhoneNumber(CObjString *pContentStr,BND_PHONE_NUM_S& additional_dial_in_phone_num)
{

    *pContentStr<< "AdditionalPhoneNumber.....";
    for(int digit_index=0;digit_index<BND_MAX_PHONE_LEN;digit_index++)
    {
    	if(additional_dial_in_phone_num.digits[digit_index]=='\f')
    		break;
        *pContentStr << (BYTE)additional_dial_in_phone_num.digits[digit_index];
    }
    *pContentStr<< "\n";

}
////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceBondingPhoneNumberList(CObjString *pContentStr,BND_PHONE_LIST_S& phone_list)
{
    WORD num_of_phones = (WORD)phone_list.numberOfPhoneNums;
    *pContentStr<< "num_of_phone_numbers:   " << num_of_phones << "\n";

    if(num_of_phones > MAX_ADDITIONAL_PHONE_NUM)
    {
    	*pContentStr<< " \n CConfPartyMplMcmsProtocolTracer::TraceBondingPhoneNumberList - Number of phones in list is illegal. Do not print event";
    	*pContentStr<< "\n";
    	return;
    }
    for(int phone_num_index=0;phone_num_index<num_of_phones;phone_num_index++) //MAX_ADDITIONAL_PHONE_NUM
    {

        *pContentStr << "AdditionalPhoneNumber " << phone_num_index+1 << " : ";
        BND_PHONE_NUM_S curr_num = phone_list.startOfPhoneList[phone_num_index];
        for(int digit_index=0;digit_index<BND_MAX_PHONE_LEN;digit_index++)
        {

            if(curr_num.digits[digit_index] ==  BND_STAR)
            {
                *pContentStr << "BND_STAR ";
            }
            else if(curr_num.digits[digit_index] ==  BND_DIEZ)
            {
                *pContentStr << "BND_DIEZ ";
            }
            else if(curr_num.digits[digit_index] ==  BND_EON)
            {
                *pContentStr << "BND_EON ";
            }
            else if(curr_num.digits[digit_index] ==  BND_PAD)
            {
                *pContentStr << "BND_PAD ";
            }else{
                *pContentStr << (WORD) curr_num.digits[digit_index] << " ";
            }

        }
        *pContentStr<< "\n";
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceStartPartyPreviewReq(CObjString* pContentStr)
{
	mcReqCmStartPreviewChannel *pStruct = (mcReqCmStartPreviewChannel *)m_pMplMcmsProt->getpData();

	*pContentStr <<"\nCONTENT: IP_CM_START_PREVIEW_CHANNEL:";

	*pContentStr << "\n CmRemoteUdpAddressIp.ipVersion     : V4";

	char szIP[20];
	::SystemDWORDToIpString(pStruct->remotePreviewAddress.addr.v4.ip, szIP);

	*pContentStr << "\n remotePreviewAddress.addr.v4.ip: ";
	*pContentStr << szIP;

	*pContentStr<< "\n remotePreviewAddress.port:"
	<< pStruct->remotePreviewAddress.port;

	*pContentStr << "\n Channel Type				 :";
					::GetChannelTypeName(pStruct->channelType,*pContentStr);
	*pContentStr << "\n Channel Direction		 :";
			 	::GetChannelDirectionName(pStruct->channelDirection,*pContentStr);
	*pContentStr << "\n PayloadType				 :"<<pStruct->payloadType;


}

////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceStopPartyPreviewReq(CObjString* pContentStr)
{
	mcReqCmCloseUdpPort *pStruct = (mcReqCmCloseUdpPort *)m_pMplMcmsProt->getpData();

	*pContentStr <<"\nCONTENT: IP_CM_STOP_PREVIEW_CHANNEL:"
				 << "\n Channel Type				 :";
					::GetChannelTypeName(pStruct->channelType,*pContentStr);
	*pContentStr << "\n Channel Direction		 :";
			 	::GetChannelDirectionName(pStruct->channelDirection,*pContentStr);


}
////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceIceGeneralIndication (CObjString *pContentStr)
{

    const ICE_GENERAL_IND_S *pStruct = (const ICE_GENERAL_IND_S *)m_pMplMcmsProt->getpData();
    *pContentStr << "status:             " << pStruct->status << "\n"
                 << "ice_session_index   " << pStruct->ice_session_index << "\n"
                 << "ice_audio_rtp_id    " << pStruct->ice_channels_id.ice_audio_rtp_id << "\n"
                 << "ice_audio_rtcp_id   " << pStruct->ice_channels_id.ice_audio_rtcp_id << "\n"
                 << "ice_video_rtp_id    " << pStruct->ice_channels_id.ice_video_rtp_id << "\n"
                 << "ice_video_rtcp_id   " << pStruct->ice_channels_id.ice_video_rtcp_id << "\n"
                 << "ice_data_rtp_id     " << pStruct->ice_channels_id.ice_data_rtp_id << "\n"
                 << "ice_data_rtcp_id    " << pStruct->ice_channels_id.ice_data_rtcp_id << "\n"
                 << "ice_content_rtp_id  " << pStruct->ice_channels_id.ice_content_rtp_id << "\n"
                 << "ice_content_rtcp_id " << pStruct->ice_channels_id.ice_content_rtcp_id << "\n"
                 << "allocatedBandwidth  " << pStruct->allocatedBandwidth << "\n"
                 << "sdp_size            " << pStruct->sdp_size << "\n";


    if (pStruct->sdp_size == 0 || (strcmp (pStruct->sdp, "") == 0))
         *pContentStr << "No SDP!" << "\n";
    else
        *pContentStr << "sdp:" << "\n" << pStruct->sdp << "\n";
}
////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceIceAckIndication(CObjString *pContentStr)
{
	 const ICE_GENERAL_IND_S *pStruct = (const ICE_GENERAL_IND_S *)m_pMplMcmsProt->getpData();
	 *pContentStr << "status:  " << pStruct->status << "\n"
				  << "ice_session_index   " << pStruct->ice_session_index << "\n";

}
////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceIceErrIndication(CObjString *pContentStr)
{
	 const ICE_ACK_IND_S *pStruct = (const ICE_ACK_IND_S *)m_pMplMcmsProt->getpData();
	 *pContentStr << "status:  " << pStruct->status << "\n";

}
////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceIceGeneralRequest (CObjString *pContentStr)
{
    const ICE_GENERAL_REQ_S *pStruct = (const ICE_GENERAL_REQ_S *)m_pMplMcmsProt->getpData();
    *pContentStr << "ice_session_index   " << pStruct->ice_session_index << "\n";
    *pContentStr << "isModifyChannels    " << pStruct->isModifyChannels << "\n";

    for(int i=0;i<NumOfMediaTypes-1;i++)
    {
        *pContentStr << "channelType  " << GetChannelTypeStr(pStruct->candidate_list[i].channelType) << "\n";
        if (pStruct->candidate_list[i].channelOperation == iceOpenChannel)
            *pContentStr << "channelOperation = iceOpenChannel \n";
        else if (pStruct->candidate_list[i].channelOperation == iceCloseChannel)
            *pContentStr << "channelOperation = iceCloseChannel \n";
        else
            *pContentStr << "channelOperation = iceInvalidChannel \n";

        for(int j=0;j<MaxIcePorts;j++)
        {
        	*pContentStr << "rtp_port  " << pStruct->candidate_list[i].rtp_port[j] << "\n";
        	*pContentStr << "rtcp_port  " << pStruct->candidate_list[i].rtcp_port[j] << "\n";
        }
    }

    *pContentStr << "sdp_size            " << pStruct->sdp_size << "\n";
    if (pStruct->sdp_size == 0 || (strcmp (pStruct->sdp, "") == 0))
         *pContentStr << "No SDP!" << "\n";
    else
        *pContentStr << "sdp:" << "\n" << pStruct->sdp << "\n";
}

////////////////////////////////////////////////////////////////////////////////////////////////
char * CConfPartyMplMcmsProtocolTracer::GetChannelTypeStr (APIU32 channelType)
{
    switch (channelType)
    {
        case kEmptyChnlType : return "kEmptyChnlType";
        case kIpAudioChnlType : return "kIpAudioChnlType";
        case kIpVideoChnlType : return "kIpVideoChnlType";
        case kIpFeccChnlType : return "kIpFeccChnlType";
        case kIpContentChnlType : return "kIpContentChnlType";
        case kPstnAudioChnlType : return "kPstnAudioChnlType";
        case kRtmChnlType : return "kRtmChnlType";
        case kIsdnMuxChnlType : return "kIsdnMuxChnlType";
        case kAvcVSWChnType   : return "kAvcVSWChnType"; /* kAvcVSWChnType need to change APICOM for a new channel type  */
        case kUnknownChnlType : return "kUnknownChnlType";
    }
    return "kUnknownChnlType";
}
////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceRtcpVideoPreferencesInd (CObjString *pContentStr)
{
	 const TCmRtcpVideoPreference *pStruct = (const TCmRtcpVideoPreference *)m_pMplMcmsProt->getpData();

	 *pContentStr <<"\nCONTENT: IP_CM_RTCP_VIDEO_PREFERENCE_IND:";

	 *pContentStr << "DspNumber     : "<< (pStruct->tCmRtcpHeader).uDspNumber << "\n";
	 *pContentStr << "PortNumber     : "<< (pStruct->tCmRtcpHeader).uPortNumber<< "\n";

	 *pContentStr << "FrameResWidth     : "<< (pStruct->tCmRtcpVideoPreferenceInfo).uFrameResWidth<< "\n";
	 *pContentStr << "FrameResHeight     : "<< (pStruct->tCmRtcpVideoPreferenceInfo).uFrameResHeight<< "\n";
	 *pContentStr << "BitRate     : "<< (pStruct->tCmRtcpVideoPreferenceInfo).uBitRate<< "\n";
	 *pContentStr << "FrameRate     : "<< (pStruct->tCmRtcpVideoPreferenceInfo).uFrameRate<< "\n";

}
////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceRtcpVideoPreferencesReq (CObjString *pContentStr)
{
	 const TCmRtcpVideoPreference *pStruct = (const TCmRtcpVideoPreference *)m_pMplMcmsProt->getpData();

	 *pContentStr <<"\nCONTENT: IP_CM_RTCP_VIDEO_PREFERENCE_REQ:"<<"\n";

	 *pContentStr << "DspNumber     : "<< (pStruct->tCmRtcpHeader).uDspNumber << "\n";
	 *pContentStr << "PortNumber     : "<< (pStruct->tCmRtcpHeader).uPortNumber<< "\n";

	 *pContentStr << "FrameResWidth     : "<< (pStruct->tCmRtcpVideoPreferenceInfo).uFrameResWidth<< "\n";
	 *pContentStr << "FrameResHeight     : "<< (pStruct->tCmRtcpVideoPreferenceInfo).uFrameResHeight<< "\n";
	 *pContentStr << "BitRate     : "<< (pStruct->tCmRtcpVideoPreferenceInfo).uBitRate<< "\n";
	 *pContentStr << "FrameRate     : "<< (pStruct->tCmRtcpVideoPreferenceInfo).uFrameRate<< "\n";

}
////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceRtcpRtpFeedbackReq (CObjString *pContentStr)
{
	 const TCmRtcpRTPFB *pStruct = (const TCmRtcpRTPFB *)m_pMplMcmsProt->getpData();

	 *pContentStr <<"\nCONTENT: IP_CM_RTCP_RTPFB_REQ:"<<"\n";

	 *pContentStr << "Exponential  : "<< (pStruct->tCmRtcpRTPFBInfo).MxTBRExp << "\n";
	 *pContentStr << "Mantissa     : "<< (pStruct->tCmRtcpRTPFBInfo).MxTBRMantissa<< "\n";
	 *pContentStr << "Overhead     : "<< (pStruct->tCmRtcpRTPFBInfo).MxTBRMeasuredOverhead<< "\n";
	 *pContentStr << "MediaType    : "<< (pStruct->tCmRtcpRTPFBInfo).uMediaType<< "\n";

}
////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceRtcpRtpFeedbackInd (CObjString *pContentStr)
{
	 const TCmRtcpRTPFB *pStruct = (const TCmRtcpRTPFB *)m_pMplMcmsProt->getpData();

	 *pContentStr <<"\nCONTENT: IP_CM_RTCP_RTPFB_IND:"<<"\n";

	 *pContentStr << "Exponential  : "<< (pStruct->tCmRtcpRTPFBInfo).MxTBRExp << "\n";
	 *pContentStr << "Mantissa     : "<< (pStruct->tCmRtcpRTPFBInfo).MxTBRMantissa<< "\n";
	 *pContentStr << "Overhead     : "<< (pStruct->tCmRtcpRTPFBInfo).MxTBRMeasuredOverhead<< "\n";
	 *pContentStr << "MediaType    : "<< (pStruct->tCmRtcpRTPFBInfo).uMediaType<< "\n";

}
////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceRtcpReceiverBandwidthReq (CObjString *pContentStr)
{
	 const TCmRtcpBandwidth *pStruct = (const TCmRtcpBandwidth *)m_pMplMcmsProt->getpData();

	 *pContentStr <<"\nCONTENT: IP_CM_RTCP_RECEIVER_BANDWIDTH_REQ:"<<"\n";

	 *pContentStr << "DspNumber     : "<< (pStruct->tCmRtcpHeader).uDspNumber << "\n";
	 *pContentStr << "PortNumber    : "<< (pStruct->tCmRtcpHeader).uPortNumber<< "\n";

	 *pContentStr << "Bandwidth    : "<< (pStruct->nBandwidth)<< "\n";

}
////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceRtcpReceiverBandwidthInd (CObjString *pContentStr)
{
	 const TCmRtcpBandwidth *pStruct = (const TCmRtcpBandwidth *)m_pMplMcmsProt->getpData();

	 *pContentStr <<"\nCONTENT: IP_CM_RTCP_RECEIVER_BANDWIDTH_IND:"<<"\n";

	 *pContentStr << "DspNumber     : "<< (pStruct->tCmRtcpHeader).uDspNumber << "\n";
	 *pContentStr << "PortNumber    : "<< (pStruct->tCmRtcpHeader).uPortNumber<< "\n";

	 *pContentStr << "Bandwidth    : "<< (pStruct->nBandwidth)<< "\n";

}

////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceBfcpMessageReq(CObjString *pContentStr)
{
	const mcReqBfcpMessage *pStruct = (const mcReqBfcpMessage *)m_pMplMcmsProt->getpData();

	*pContentStr <<"\nCONTENT: IP_CM_BFCP_MESSAGE_REQ:"<<"\n";

	 *pContentStr << "status	: "<< pStruct->status << "\n";
	 *pContentStr << "length	: "<< pStruct->length << "\n";
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceBfcpMessageInd(CObjString *pContentStr)
{
	const mcIndBfcpMessage *pStruct = (const mcIndBfcpMessage *)m_pMplMcmsProt->getpData();

	*pContentStr <<"\nCONTENT: IP_CM_BFCP_MESSAGE_IND:"<<"\n";

	 *pContentStr << "status	: "<< pStruct->status << "\n";
	 *pContentStr << "length	: "<< pStruct->length << "\n";
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceVsrMessageGen(CObjString *pContentStr)
{
	const TCmRtcpVsrMsg *pStruct = (const TCmRtcpVsrMsg *)m_pMplMcmsProt->getpData();

	if (pStruct)
	{
		 *pContentStr << "DspNumber     : "<< (pStruct->cmRtcpHeader).uDspNumber << "\n";
		 *pContentStr << "PortNumber    : "<< (pStruct->cmRtcpHeader).uPortNumber<< "\n";

		 const TCmRtcpVsrInfo& vsrInfo = pStruct->cmRtcpVsrInfo;
		 *pContentStr << "MSI           : " << vsrInfo.MSI << "\n";
		 *pContentStr << "SSRC          : " << vsrInfo.senderSSRC << "\n";
		 *pContentStr << "Key Frame     : " << vsrInfo.keyFrame << "\n";
		 *pContentStr << "Request ID    : " << vsrInfo.requestId << "\n";
		 *pContentStr << "Num of Entries: " << vsrInfo.numberOfEntries << "\n";

		 for (int i = 0; i < vsrInfo.numberOfEntries; ++i)
		 {
			 const TCmRtcpVsrEntry& vsrEntry = vsrInfo.VSREntry[i];

			 *pContentStr << "\nENTRY #" << i << "\n";
			 *pContentStr << "=================\n";
			 *pContentStr << "Payload type      : " << vsrEntry.payloadType << "\n";
			 *pContentStr << "Aspect ratio mask : " << vsrEntry.aspectRatioBitMask << "\n";
			 *pContentStr << "Flags             : " << vsrEntry.flags << "\n";
			 *pContentStr << "Frame rate mask   : " << vsrEntry.frameRateBitmask << "\n";
			 *pContentStr << "Bit rate per level: " << vsrEntry.bitRatePerLevel << "\n";
			 *pContentStr << "Max Height        : " << vsrEntry.maxHeight << "\n";
			 *pContentStr << "Max Width         : " << vsrEntry.maxWidth << "\n";
			 *pContentStr << "Max pixel number  : " << vsrEntry.maximumNumberOfPixels << "\n";
			 *pContentStr << "Minimum bitrate   : " << vsrEntry.minBitRate << "\n";
			 *pContentStr << "May instances num : " << vsrEntry.numberOfMayInstances << "\n";
			 *pContentStr << "Must instances num: " << vsrEntry.numberOfMustInstances << "\n";
			 *pContentStr << "Bitrate histogram : ";
			 for (int histNdx = 0; histNdx < VSR_NUM_OF_BITRATE_RANGES; ++histNdx) *pContentStr << vsrEntry.bitRateHistogram[histNdx] << " ";
			 *pContentStr << "\n";
			 *pContentStr << "Quality histogram : ";
			 for (int histNdx = 0; histNdx < VSR_NUM_OF_QUALITY_LEVELS; ++histNdx) *pContentStr << vsrEntry.qualityReportHistogram[histNdx] << " ";
			 *pContentStr << "\n";

		 }
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceVsrMessageReq(CObjString *pContentStr)
{
	*pContentStr <<"\nCONTENT: IP_CM_RTCP_VSR_REQ:"<<"\n";
	TraceVsrMessageGen(pContentStr);
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceVsrMessageInd(CObjString *pContentStr)
{
	*pContentStr <<"\nCONTENT: IP_CM_RTCP_VSR_IND:"<<"\n";
	TraceVsrMessageGen(pContentStr);
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceMsSvcSingleStreamDesc(CObjString *pContentStr, const SingleStreamDesc* pDesc, const BYTE noOfStreamDescs)
{
DBGPASSERT_AND_RETURN(!pContentStr || !pDesc);

	for(BYTE streamNdx = 0; streamNdx < noOfStreamDescs; ++streamNdx, ++pDesc)
	{
		//if (pDesc -> bIsActive)
		//{
			*pContentStr <<"\nStream #"<< streamNdx << "\n";
			*pContentStr <<"------------\n";
			*pContentStr <<"Is active   	: "<< pDesc -> bIsActive 			<< "\n";
			*pContentStr <<"Party ID    	: "<< pDesc -> unPartyId 			<< "\n";
			*pContentStr <<"Local MSI   	: "<< pDesc -> localMSI 			<< "\n";
			*pContentStr <<"Lync MSI    	: "<< pDesc -> lyncMSI 				<< "\n";
			*pContentStr <<"In SSRC     	: "<< pDesc -> remoteSSRCRange[0] 	<< "-" << pDesc -> remoteSSRCRange[1] << "\n";
			*pContentStr <<"Out SSRC    	: "<< pDesc -> localSSRCRange[0] 	<< "-" << pDesc -> localSSRCRange[1] << "\n";
			*pContentStr <<"IsUpdateArtInfo : "<< pDesc -> bUpdateArtInfo 		<< "\n";
		//}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceMsSvcMonitoringStreamDesc(CObjString *pContentStr, const DspInfoForPartyMonitoringReq* pDesc, const BYTE noOfStreamDescs)
{
DBGPASSERT_AND_RETURN(!pContentStr || !pDesc);

	for(BYTE streamNdx = 0; streamNdx < noOfStreamDescs; ++streamNdx, ++pDesc)
	{
		//if (pDesc -> bIsActive)
		//{
			*pContentStr <<"\nStream #"<< streamNdx << "\n";
			*pContentStr <<"------------\n";
			*pContentStr <<"Party ID    	: "<< pDesc -> unPartyId 			<< "\n";

		//}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////
//LYNC2013_FEC_RED:
void CConfPartyMplMcmsProtocolTracer::TracePacketLossStatusInd(CObjString *pContentStr)
{
	const RTCP_PACKET_LOSS_STATUS_IND_S *pStruct = (const RTCP_PACKET_LOSS_STATUS_IND_S *)m_pMplMcmsProt->getpData();

	*pContentStr <<"\nCONTENT: IP_CM_RTCP_PACKET_LOSS_STATUS_IND:"<<"\n";

	 *pContentStr << "mediaDirection        : "<< pStruct->mediaDirection << "\n";
	 *pContentStr << "fractionLossInPercent	: "<< pStruct->fractionLossInPercent << "\n";
	 *pContentStr << "PacketLossStatus  	: "<< pStruct->ePacketLossStatus << "\n";
	 *pContentStr << "MediaType	        	: "<< pStruct->eMediaType << "\n";
	 *pContentStr << "SSRC	          		: "<< pStruct->unSSRC << "\n";
}
////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceMsSvcP2PInitReq(CObjString *pContentStr)
{
	DBGPASSERT_AND_RETURN(!pContentStr || !m_pMplMcmsProt);
	const mcBasicLync2013InfoReq *pStruct = (const mcBasicLync2013InfoReq *)m_pMplMcmsProt->getpData();
	*pContentStr <<"\nCONTENT: CONFPARTY_CM_INIT_ON_LYNC_CALL_REQ:"<<"\n";
	if (pStruct)
	{
		*pContentStr << "\nIsAVMCU           : " << pStruct -> bIsAVMCU << (pStruct -> bIsAVMCU ? " (unexpected)" : "")  << "\n";
		*pContentStr << "IsIceParty        : " << pStruct -> bIsIceParty << "\n";
		*pContentStr <<"\n===============\n";
		*pContentStr <<"Active streams:\n";
		*pContentStr <<"===============\n";
		TraceMsSvcSingleStreamDesc(pContentStr, reinterpret_cast<const SingleStreamDesc*>(pStruct->connectedParties),MAX_STREAM_BASIC_LYNC_CONN);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceMsSvcAvMcuInitReq(CObjString *pContentStr)
{
DBGPASSERT_AND_RETURN(!pContentStr || !m_pMplMcmsProt);
	const mcAvMcuLync2013InfoReq *pStruct = (const mcAvMcuLync2013InfoReq *)m_pMplMcmsProt->getpData();
	*pContentStr <<"\nCONTENT: CONFPARTY_CM_INIT_ON_AVMCU_CALL_REQ:"<<"\n";
	if (pStruct)
	{
		*pContentStr << "\nIsAVMCU           : " << pStruct -> bIsAVMCU << (pStruct -> bIsAVMCU ? "" : " (unexpected)")  << "\n";
		*pContentStr << "IsIceParty        : " << pStruct -> bIsIceParty << "\n";
		*pContentStr << "IsEncrypted      : " << pStruct -> bIsEncrypted << "\n";
		*pContentStr <<"\n================\n";
		*pContentStr <<"Rx/DMux streams:\n";
		*pContentStr <<"================\n";
		TraceMsSvcSingleStreamDesc(pContentStr, reinterpret_cast<const SingleStreamDesc*>(pStruct->ConnectedParties),MAX_STREAM_LYNC_2013_CONN);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceMsSvcAvMcuMuxReq(CObjString *pContentStr)
{
DBGPASSERT_AND_RETURN(!pContentStr || !m_pMplMcmsProt);
	const mcMuxLync2013InfoReq *pStruct = (const mcMuxLync2013InfoReq *)m_pMplMcmsProt->getpData();
	*pContentStr <<"\nCONTENT: CONFPARTY_CM_MUX_ON_AVMCU_CALL_REQ:"<<"\n";
	if (pStruct)
	{
		*pContentStr << "\nIsAVMCU           : " << pStruct -> bIsAVMCU << (pStruct -> bIsAVMCU ? "" : " (unexpected)")  << "\n";
		*pContentStr << "IsIceParty        : " << pStruct -> bIsIceParty << "\n";
		*pContentStr <<"\n===============\n";
		*pContentStr <<"Tx/Mux streams:\n";
		*pContentStr <<"===============\n";
		TraceMsSvcSingleStreamDesc(pContentStr, reinterpret_cast<const SingleStreamDesc*>(pStruct->txConnectedParties),MAX_STREAM_MUX_LYNC_CONN);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceMsSvcAvMcuDmuxReq(CObjString *pContentStr)
{
DBGPASSERT_AND_RETURN(!pContentStr || !m_pMplMcmsProt);
	const mcDmuxLync2013InfoReq *pStruct = (const mcDmuxLync2013InfoReq *)m_pMplMcmsProt->getpData();
	*pContentStr <<"\nCONTENT: CONFPARTY_CM_DMUX_ON_AVMCU_CALL_REQ:"<<"\n";
	if (pStruct)
	{
		*pContentStr << "\nIsAVMCU           : " << pStruct -> bIsAVMCU << (pStruct -> bIsAVMCU ? "" : " (unexpected)")  << "\n";
		*pContentStr << "IsIceParty        : " << pStruct -> bIsIceParty << "\n";
		*pContentStr <<"\n================\n";
		*pContentStr <<"Rx/DMux streams:\n";
		*pContentStr <<"================\n";
		TraceMsSvcSingleStreamDesc(pContentStr, reinterpret_cast<const SingleStreamDesc*>(pStruct->rxConnectedParties),MAX_STREAM_DMUX_LYNC_CONN);
	}
}


void CConfPartyMplMcmsProtocolTracer::TraceMsSvcAvMcuMontioringReq(CObjString *pContentStr)
{
	DBGPASSERT_AND_RETURN(!pContentStr || !m_pMplMcmsProt);
	const cmPartyMonitoringAvMcuReq *pStruct = (const cmPartyMonitoringAvMcuReq *)m_pMplMcmsProt->getpData();
	*pContentStr <<"\nCONTENT: RTP_PARTY_MONITORING_AV_MCU_REQ:"<<"\n";
	if (pStruct)
	{
		*pContentStr << "IsIceParty        : " << pStruct -> bIsIceParty << "\n";
		*pContentStr <<"\n================\n";
		*pContentStr <<"monitoring  streams:\n";
		*pContentStr <<"================\n";
		TraceMsSvcMonitoringStreamDesc(pContentStr, reinterpret_cast<const DspInfoForPartyMonitoringReq*>(pStruct->DSPInfoList),MAX_STREAM_AVMCU_CONN);
	}

}

////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceMsSvcPliMessageGen(CObjString *pContentStr)
{
	const TCmRtcpMsSvcPLIMsg *pStruct = (const TCmRtcpMsSvcPLIMsg *)m_pMplMcmsProt->getpData();

	if (pStruct)
	{
		 *pContentStr << "DspNumber     : "<< (pStruct->cmRtcpHeader).uDspNumber << "\n";
		 *pContentStr << "PortNumber    : "<< (pStruct->cmRtcpHeader).uPortNumber<< "\n";

		 const TCmRtcpMsSvcPLIInfo& pliInfo = pStruct->cmRtcpMsSvcPLI;
		 *pContentStr << "MSI           : " << pliInfo.mediaSourceSSRC << "\n";
		 *pContentStr << "SSRC          : " << pliInfo.senderSSRC << "\n";
		 *pContentStr << "Request ID    : " << pliInfo.requestId << "\n";

		 *pContentStr << "SFRn          : " 	 << pliInfo.SFR0 << " " << pliInfo.SFR1 << " " << pliInfo.SFR2 << " " << pliInfo.SFR3 << " "
				 	 	 	 	 	 	 	 	 << pliInfo.SFR4 << " " << pliInfo.SFR5 << " " << pliInfo.SFR6 << " " << pliInfo.SFR7 << "\n";

	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceMsSvcPliMessageReq(CObjString *pContentStr)
{
	*pContentStr <<"\nCONTENT: IP_CM_RTCP_MS_SVC_PLI_REQ:"<<"\n";
	TraceMsSvcPliMessageGen(pContentStr);
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceMsSvcPliMessageInd(CObjString *pContentStr)
{
	*pContentStr <<"\nCONTENT: IP_CM_RTCP_MS_SVC_PLI_IND:"<<"\n";
	TraceMsSvcPliMessageGen(pContentStr);
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceBfcpTcpTransportMessageInd(CObjString *pContentStr)
{
	const mcIndBfcpTransport *pStruct = (const mcIndBfcpTransport *)m_pMplMcmsProt->getpData();

	*pContentStr <<"\nCONTENT: IP_CM_BFCP_TCP_TRANSPORT_IND:"<<"\n";

	 *pContentStr << "status	: "<< pStruct->status << "\n";
}
////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceRtpFecRedOnOffReq(CObjString *pContentStr)
{
	const TFEC_RED_OnOffReq *pStruct = (const TFEC_RED_OnOffReq *)m_pMplMcmsProt->getpData();

	*pContentStr <<"\nCONTENT: ART_FEC_RED_ON_OFF_REQ:"<<"\n";

	 *pContentStr << "IsOnOff		    : "<< pStruct->bunIsOnOff << "\n";
	 *pContentStr << "FecFrameRatio	    : "<< pStruct->FecFrameRatio << "\n";
	 *pContentStr << "eMediaType	    : "<< pStruct->eMediaType << "\n";          //LYNC2013_FEC_RED
	 *pContentStr << "unDspFractionLoss	: "<< pStruct->unDspFractionLoss << "\n";   //LYNC2013_FEC_RED
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceDtlsStartReq(CObjString *pContentStr, const char* OpcodeString)
{
	const TDtlsStartStruct *pStruct = (const TDtlsStartStruct *)m_pMplMcmsProt->getpData();

	const TDtlsStartStruct* pStructWithAll = (const TDtlsStartStruct*)m_pMplMcmsProt->getpData();
	const mcReqCmDtlsStart& st = pStructWithAll->tCmDtlsStart;

	TraceMcmsMplPhysicalRsrcInfo(pContentStr, pStructWithAll->physicalPort);

	*pContentStr << "\nCONTENT: " << OpcodeString;
	*pContentStr << "\n  channelType                        :"; ::GetChannelTypeName(st.channelType, *pContentStr);
	*pContentStr << "\n  channelDirection                   :"; ::GetChannelDirectionName(st.channelDirection, *pContentStr);
	*pContentStr << "\n  CmLocalUdpAddressIp.transportType  :" << GetTransportTypeName(st.CmLocalUdpAddressIp.transportType);
	*pContentStr << "\n  CmLocalUdpAddressIp.ipVersion      :" << st.CmLocalUdpAddressIp.ipVersion;

	if (st.CmLocalUdpAddressIp.ipVersion == eIpVersion4)
	{
		*pContentStr << " (V4)";
		*pContentStr << "\n  CmLocalUdpAddressIp.addr.v4.ip     :";
		char str[30];
		sprintf(str, "%X", st.CmLocalUdpAddressIp.addr.v4.ip);
		*pContentStr << "0x" << str;
	}
	else
	{
	    *pContentStr << " (V6)";
		*pContentStr << "\n  CmLocalUdpAddressIp.addr.v6.ip[16] :";
		char str[64];
		memset(str, '\0', 64);
		::ipToString(st.CmLocalUdpAddressIp, str, 1);
		*pContentStr << str;
	}

	*pContentStr << "\n  CmLocalUdpAddressIp.port           :" << st.CmLocalUdpAddressIp.port;
	*pContentStr << "\n  Local RTCP port                    :" << st.LocalRtcpPort;
	*pContentStr << "\n  CmRemoteUdpAddressIp.transportType :" << GetTransportTypeName(st.CmRemoteUdpAddressIp.transportType);
	*pContentStr << "\n  CmRemoteUdpAddressIp.ipVersion     :" << st.CmRemoteUdpAddressIp.ipVersion;

	if (st.CmRemoteUdpAddressIp.ipVersion == eIpVersion4)
	{
		*pContentStr << " (V4)";
		*pContentStr << "\n  CmRemoteUdpAddressIp.addr.v4.ip    :";
		char str[30];
		sprintf(str, "%X", st.CmRemoteUdpAddressIp.addr.v4.ip);
		*pContentStr << "0x" << str;
	}
	else
	{
		*pContentStr << "V6";
		*pContentStr << "\n CmRemoteUdpAddressIp.addr.v6.ip[16] :";
		char str[64];
		memset(str, '\0', 64);
		::ipToString(st.CmRemoteUdpAddressIp, str, 1);
		*pContentStr << str;
	}

	*pContentStr << "\n  CmRemoteUdpAddressIp.port          :" << st.CmRemoteUdpAddressIp.port;
	*pContentStr << "\n  Remote RTCP port                   :" << st.RemoteRtcpPort;

	if (!strncmp(OpcodeString, "IP_CM_DTLS_START_REQ", strlen("IP_CM_DTLS_START_REQ")))
		TraceSdesSpecificParams(pContentStr, st.sdesCap);
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceDtlsEndInd(CObjString *pContentStr)
{
	const mcIndCmDtlsEnd *pStruct = (const mcIndCmDtlsEnd *)m_pMplMcmsProt->getpData();

	*pContentStr <<"\nCONTENT: IP_CM_DTLS_END_IND:"<<"\n";

	*pContentStr << "status				: "<< pStruct->status << "\n";
	*pContentStr << "channelType		: "<< pStruct->channelType << "\n";
	*pContentStr << "channelDirection	: "<< pStruct->channelDirection << "\n";

	if (pStruct->status == STATUS_OK)
		TraceSdesSpecificParams(pContentStr, pStruct->sdesCap);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyMplMcmsProtocolTracer::TraceIpMediaDetectionInd(CObjString *pContentStr)
{
	const TMediaDisconnectedIndStruct *pSt = (const TMediaDisconnectedIndStruct *)m_pMplMcmsProt->getpData();

	*pContentStr <<"\nCONTENT: IP_CM_MEDIA_DISCONNECTED_IND:"
				<< "\n Channel Type				 :";
				::GetChannelTypeName(pSt->unChannelType,*pContentStr);

		*pContentStr << "\n IsRTP				 :"  << pSt->bIsRTP;
}


