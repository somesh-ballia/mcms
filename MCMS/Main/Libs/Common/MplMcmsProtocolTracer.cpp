// MplMcmsProtocolTracer.cpp

#include <stdio.h>
#include "MplMcmsProtocolTracer.h"
#include <iomanip>
#include <stdlib.h>
#include "ProcessBase.h"
#include "IpMfaOpcodes.h"
#include "IpMngrOpcodes.h"
#include "VideoApiDefinitions.h"
#include "IpRtpInd.h"
#include "IpCmReq.h"
#include "IpCmInd.h"
#include "IpWebRtcReq.h"
#include "IpWebRtcInd.h"
#include "IpAddressDefinitions.h"
#include "H323CapabiilityInfo.h"
#include "AudRequestStructs.h"
#include "AcRequestStructs.h"
#include "AcIndicationStructs.h"
#include "IpCsOpcodes.h"
#include "ConfStructs.h"
#include "CDRUtils.h"
#include "H264.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsCardMngrIvrCntl.h"
#include "OpcodesMcmsCardMngrTB.h"
#include "OpcodesMcmsAudio.h"
#include "OpcodesMcmsAudioCntl.h"
#include "OpcodesMcmsVideo.h"
#include "OpcodesRanges.h" // for NET_ opcodes (until implemented by other sides)
#include "OpcodesMcmsInternal.h"
#include "IpMfaOpcodes.h"
#include "IpCsOpcodes.h"
#include "IpRtpInd.h"
#include "H323CapabiilityInfo.h"
#include "VideoApiDefinitionsStrings.h"
#include "AudioApiDefinitionsStrings.h"
#include "IpCommonUtilTrace.h"
#include "SystemFunctions.h"
#include "NStream.h"
#include "SysConfig.h"
#include "Segment.h"
#include "OpcodesMcmsCardMngrMaintenance.h"
#include "OpcodesMcmsShelfMngr.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "RecordingRequestStructs.h"
#include "OpcodesMcmsCardMngrRecording.h"
#include "ObjString.h"
#include "OpcodesMcmsCardMngrIpMedia.h"
#include "OpcodesMcmsNetQ931.h"
#include "ArtRequestStructs.h"
#include "ArtDefinitions.h"
#include "MplMcmsProtocolSizeValidator.h"
#include "OpcodesMcmsMux.h"
#include "H221.h"
#include "EncryptionDefines.h"
#include "OpcodesMcmsPCM.h"
#include "OpcodesMcmsCardMngrTIP.h"
#include "TipStructs.h"
#include "TipUtils.h"
#include "TraceClass.h"
#include "BfcpStructs.h"
#include "OpcodesMcmsCardMngrBFCP.h"
#include "LibsCommonHelperFuncs.h"
#include "IceCmInd.h"

#include "MrcStructs.h"
#include "OpcodesMrmcCardMngrMrc.h"
#include "MrcApiDefinitionsStrings.h"
#include "IpCmInd.h"
#include "EnumsToStrings.h"

const DWORD TraceBufferSize = 1024 * 1024;

extern const char*  MainEntityToString(APIU32 entityType);
extern char*        ProcessTypeToString(eProcessType entityType);
extern char*        ResourceTypeToString(APIU32 resourceType);
extern char*        LogicalResourceTypeToString(APIU32 logicalResourceType);
extern const char*  NetworkTypeToString(ENetworkType networkType);
extern const char*  IPv6ScopeIdToString(enScopeId theScopeId);
extern const char*  feccPartyTypeToString(feccPartyTypeEnum ePartyType);

//////////////////////////////////////////////////////////////////////////////////////////////
CMplMcmsProtocolTracer::CMplMcmsProtocolTracer()
{
  m_pMplMcmsProt = NULL;
  m_ProtStr = new CManDefinedString(TraceBufferSize);
}

//////////////////////////////////////////////////////////////////////////////////////////////
CMplMcmsProtocolTracer::CMplMcmsProtocolTracer(CMplMcmsProtocol &mplMcmsProt)
{
  m_pMplMcmsProt = &mplMcmsProt;
  m_ProtStr = new CManDefinedString(TraceBufferSize);
}

//////////////////////////////////////////////////////////////////////////////////////////////
CMplMcmsProtocolTracer::~CMplMcmsProtocolTracer()
{
  POBJDELETE(m_ProtStr);
}

//////////////////////////////////////////////////////////////////////////////////////////////
bool CMplMcmsProtocolTracer::CheckPeriodTraceFilter(OPCODE opcode) const
{
  bool isToPrint = false;

  BOOL isPeriodTrace = FALSE;
  CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
  sysConfig->GetBOOLDataByKey(CFG_KEY_PERIOD_TRACE, isPeriodTrace);

  if (TRUE == isPeriodTrace)
    isToPrint = true;

  else
  {
    switch (opcode)
    {
	  case IP_CM_RTCP_MSG_IND: //VNGFE-4869
      case CS_KEEP_ALIVE_REQ:
      case CS_KEEP_ALIVE_IND:
      case CM_KEEP_ALIVE_REQ:
      case CM_KEEP_ALIVE_IND:
      case SM_KEEP_ALIVE_REQ:
      case SM_KEEP_ALIVE_IND:
      {
        isToPrint = false;
        break;
      }

      default:
      {
        isToPrint = true;
        break;
      }
    }
  }

  return isToPrint;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceMplMcmsProtocol(const char* message, BOOL Type, BOOL traceContent)
{
#ifdef NO_TRACE_MODE
  return;
#endif

  if (m_pMplMcmsProt == NULL || m_ProtStr == NULL)
	  return;
  if ((true == IsKeepAlive(m_pMplMcmsProt->getCommonHeaderOpcode())) || (false == CheckPeriodTraceFilter(m_pMplMcmsProt->getCommonHeaderOpcode())))
    return;

  if (m_pMplMcmsProt->getCommonHeaderOpcode() == H323_CS_PARTY_KEEP_ALIVE_REQ ||
      m_pMplMcmsProt->getCommonHeaderOpcode() == SIP_CS_PARTY_KEEP_ALIVE_REQ  ||
      m_pMplMcmsProt->getCommonHeaderOpcode() == H323_CS_PARTY_KEEP_ALIVE_IND ||
      m_pMplMcmsProt->getCommonHeaderOpcode() == SIP_CS_PARTY_KEEP_ALIVE_IND  ||
      m_pMplMcmsProt->getCommonHeaderOpcode() == REMOTE_BAS_CAPS)
  {
    //PTRACE(eLevelInfoNormal, "This is CS keep alive do not print to be removed! ");
    return;
  }

  CProcessBase *pProcess = CProcessBase::GetProcess();
  const eProcessType processType = pProcess->GetProcessType();
  BYTE srcId = m_pMplMcmsProt->getCommonHeaderSrc_id();
  BYTE EntityType = m_pMplMcmsProt->getMsgDescriptionHeaderEntity_type();
  BYTE destId = m_pMplMcmsProt->getCommonHeaderDest_id();
  const char * Source = ::MainEntityToString(srcId);

  m_ProtStr->Clear();
  char tempMsg[61];
  memset(tempMsg,'\0',sizeof(tempMsg));
  strncpy(tempMsg,message,sizeof(tempMsg)-1);
  *m_ProtStr << tempMsg << "\n";

  TraceCommonHeader(*m_ProtStr);
  TraceMessageHeader(*m_ProtStr);
  if ((CS_API_TYPE == Type) && (eProcessCSMngr != processType))
    TraceCSHeader(*m_ProtStr);

  TracePhysicalInfoHeader(*m_ProtStr);
  TracePortDescriptionHeader(*m_ProtStr);
  if (traceContent)
    TraceContent(m_ProtStr, processType);

  const DWORD traceLevel = (processType == eProcessMplApi || processType == eProcessCSApi ? eLevelInfoHigh : eLevelInfoNormal);
  OPCODE opcode = m_pMplMcmsProt->getCommonHeaderOpcode();
  const string &strOpcode = pProcess->GetOpcodeAsString(opcode);

  if ((m_ProtStr->GetString() != NULL ) && (strOpcode.c_str() != NULL ))
  PTRACEOPCODE(traceLevel, m_ProtStr->GetString(), opcode, strOpcode.c_str());
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceCommonHeader(CObjString &ostr) const
{
  static const CProcessBase* process = CProcessBase::GetProcess();
  const std::string &str = process->GetOpcodeAsString(m_pMplMcmsProt->getCommonHeaderOpcode());
  const char* OpcodeStr = str.c_str();

  BYTE destId = m_pMplMcmsProt->getCommonHeaderDest_id();
  const char* Destination = ::MainEntityToString(destId);

  BYTE srcId = m_pMplMcmsProt->getCommonHeaderSrc_id();
  const char * Source = ::MainEntityToString(srcId);

  static const eProcessType processType = CProcessBase::GetProcess()->GetProcessType();
  ostr << "Opcode:" << OpcodeStr << " (" << m_pMplMcmsProt->getCommonHeaderOpcode() << ")";

  if ((processType == eProcessMplApi) || (processType == eProcessCSApi))
  {
    ostr << "\nSnd_Time:" << m_pMplMcmsProt->getCommonHeaderTime_stamp()
         << " Dest:"      << Destination
         << " Src:"       << Source
         << " Seq:"       << m_pMplMcmsProt->getCommonHeaderSequence_num()
         << " PyldLn:"    << m_pMplMcmsProt->getCommonHeaderPayload_len();
  }
  ostr << "\n";
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceMessageHeader(CObjString &ostr) const
{
  eMainEntities srcId = (eMainEntities)m_pMplMcmsProt->getCommonHeaderSrc_id();
  const char* entity = (eMcms == srcId ? ::ProcessTypeToString((eProcessType)(m_pMplMcmsProt->getMsgDescriptionHeaderEntity_type())) : "NA");

  ostr << "Req:"        << m_pMplMcmsProt->getMsgDescriptionHeaderRequest_id()
	   << " Seq:"   << m_pMplMcmsProt->getCommonHeaderSequence_num()
       << " Entity:"    << entity
       << " MsgTime:"   << m_pMplMcmsProt->getMsgDescriptionHeaderTime_stamp()
       << " MsgAckInd:" << m_pMplMcmsProt->getPortDescriptionHeaderMsg_ack_ind() << "\n";
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TracePhysicalInfoHeader(CObjString &ostr) const
{
  BYTE rsrcType = m_pMplMcmsProt->getPhysicalInfoHeaderResource_type();
  const char* Resource_type = ::ResourceTypeToString(rsrcType);

  ostr << "Brd:"          << m_pMplMcmsProt->getPhysicalInfoHeaderBoard_id()
       << " SubBrd:"      << m_pMplMcmsProt->getPhysicalInfoHeaderSub_board_id()
       << " Unit:"        << m_pMplMcmsProt->getPhysicalInfoHeaderUnit_id()
       << " Accelerator:" << m_pMplMcmsProt->getPhysicalInfoHeaderAccelerator_id()
       << " Port:"        << m_pMplMcmsProt->getPhysicalInfoHeaderPort_id()
       << " Rsrc:"        << Resource_type
       << " Future1:"     << m_pMplMcmsProt->getPhysicalInfoHeaderFuture_use1()
       << " Future2:"     << m_pMplMcmsProt->getPhysicalInfoHeaderFuture_use2() << "\n";
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TracePortDescriptionHeader(CObjString &ostr) const
{
  DWORD partyId = m_pMplMcmsProt->getPortDescriptionHeaderParty_id();
  ostr << "Party:";
  if (0xffffffff == partyId)
    ostr << "0xffffffff";
  else
    ostr << partyId;

  DWORD confId = m_pMplMcmsProt->getPortDescriptionHeaderConf_id();
  ostr << " Conf:";
  if (0xffffffff == confId)
    ostr << "0xffffffff";
  else
    ostr << confId;

  DWORD conn = m_pMplMcmsProt->getPortDescriptionHeaderConnection_id();
  ostr << " Con:";
  if (0xffffffff == conn)
    ostr << "0xffffffff";
  else
    ostr << conn;

  BYTE logicRsrcType1 = m_pMplMcmsProt->getPortDescriptionHeaderLogical_resource_type_1();
  const char* logical_resource_type_1 = ::LogicalResourceTypeToString(logicRsrcType1);
  BYTE logicRsrcType2 = m_pMplMcmsProt->getPortDescriptionHeaderLogical_resource_type_2();
  const char* logical_resource_type_2 = ::LogicalResourceTypeToString(logicRsrcType2);

  ostr << " LRT1:" << logical_resource_type_1 << " LRT2:" << logical_resource_type_2;

  WORD roomId = m_pMplMcmsProt->getPortDescriptionHeaderRoom_Id();
  ostr << " RoomID:";
  if (0xffff == roomId)
    ostr << "0xffff";
  else
    ostr << roomId;

  ostr << " Op:" << m_pMplMcmsProt->getCommonHeaderOpcode();

  ostr << "\n";
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceCSHeader(CObjString &ostr) const
{
  ostr << "CShndl:"       << m_pMplMcmsProt->getCentralSignalingHeaderCsId()
       << " DestUnit:"    << m_pMplMcmsProt->getCentralSignalingHeaderDestUnitId()
       << " SrcUnit:"     << m_pMplMcmsProt->getCentralSignalingHeaderSrcUnitId()
       << " ChnlIndx:"    << m_pMplMcmsProt->getCentralSignalingHeaderChannelIndex()
       << " McChnlIndx:"  << m_pMplMcmsProt->getCentralSignalingHeaderMcChannelIndex()
       << " CalIndx:"     << m_pMplMcmsProt->getCentralSignalingHeaderCallIndex()
       << " Status:"      << m_pMplMcmsProt->getCentralSignalingHeaderStatus()
       << "\n";
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceTPKT_Header(const TPKT_HEADER_S &TPKT_Header)
{
	static CMedString str;
  str.Clear();

  str << "\nTPKT HEADER:"
      << "\n  Version_num                             :" << TPKT_Header.version_num
      << "\n  Reserved                                :" << TPKT_Header.reserved
      << "\n  Payload_len                             :" << TPKT_Header.payload_len << "\n";

  PTRACE(eLevelInfoNormal, str.GetString());
}

void CMplMcmsProtocolTracer::TraceContent(CObjString* pContentStr1, eProcessType processType)
{
  CObjString* pContentStr = (CObjString*)pContentStr1;
  if (m_pMplMcmsProt->getDataLen() == 0)
    return;
  if (IsContentSizeValid() == NO)
  {
    PASSERT(1);
    return;
  }
  switch (m_pMplMcmsProt->getCommonHeaderOpcode())
  {
    case TB_MSG_CONNECT_REQ:
      TraceTbMsgConnectDisconnectReq(pContentStr);
      break;

    case TB_MSG_DISCONNECT_REQ:
      TraceTbMsgConnectDisconnectReq(pContentStr);
      break;

    case AUDIO_OPEN_ENCODER_REQ:
      TraceAudioOpenEncoderReq(pContentStr);
      break;

    case AUDIO_OPEN_DECODER_REQ:
      TraceAudioOpenDecoderReq(pContentStr);
      break;

    case AUDIO_UPDATE_ALGORITHM_REQ:
      TraceUpdateAudioAlgorithmReq(pContentStr);
      break;

    case AUDIO_UPDATE_DECODER_REQ:
    	TraceUpdateAudioDecoderReq(pContentStr);
        break;

    case AUDIO_UPDATE_MUTE_REQ:
      TraceAudioMuteReq(pContentStr);
      break;

    case AUDIO_UPDATE_GAIN_REQ:
      TraceUpdateAudioGainReq(pContentStr);
      break;

    case AUDIO_UPDATE_USE_SPEAKER_SSRC_FOR_TX_REQ:
      TraceUpdateUseSpeakerSsrcForTxReq(pContentStr);
      break;

    case AUDIO_UPDATE_NOISE_DETECTION_REQ:
      TraceUpdateNoiseDetectionReq(pContentStr);
      break;

    case AUDIO_UPDATE_CONNECTION_STATUS_REQ:
      TraceUpdateAudioConnectionStatus(pContentStr);
      break;

    case AUDIO_UPDATE_AGC_REQ:
      TraceUpdateAGC(pContentStr);
      break;

    case AUDIO_UPDATE_RELAY_DEC_PARAMS_REQ:
      TraceAudioRelayRequests(pContentStr);
      break;

    case AUDIO_UPDATE_RELAY_ENC_PARAMS_REQ:
      TraceAudioRelayRequests(pContentStr);
      break;

    case AUDIO_UPDATE_BITRATE_REQ:
      TraceUpdateBitRate(pContentStr);
      break;

    case ART_UPDATE_RELAY_PARAMS_REQ:
	  TraceAudioRelayRequests(pContentStr);
	  break;

    case AC_UPDATE_CONF_PARAMS_REQ:
    case AC_OPEN_CONF_REQ:
      TraceOpenConfReq(pContentStr);
      break;

    case AC_ACTIVE_SPEAKER_IND:
    case AC_AUDIO_SPEAKER_IND:
      TraceSpeakerInd(pContentStr);
      break;

    case CONF_MPL_CREATE_PARTY_REQ:
      TraceConfMplCreateParty(pContentStr);
      break;
    case CONF_MPL_DELETE_PARTY_REQ:
		TraceConfMplDeleteParty(pContentStr);
		break;
    case TB_MSG_CLOSE_PORT_REQ:
		TraceTbMsgClosePortReq(pContentStr);
		break;

    case TB_MSG_OPEN_PORT_REQ:
      TraceTbMsgOpenPortReq(pContentStr);
      break;

    case VIDEO_ENCODER_CHANGE_LAYOUT_REQ:
      if (processType == eProcessMplApi)
        TraceVideoEncChangeLayoutReq(pContentStr);
      break;

    case VIDEO_ENCODER_DSP_SMART_SWITCH_CHANGE_LAYOUT_REQ:
      TraceVideoEncChangeLayoutReq(pContentStr);
      break;

    case VIDEO_ENCODER_CHANGE_LAYOUT_ATTRIBUTES_REQ:
      TraceVideoEncChangeLayoutAttributesReq(pContentStr);
      break;

    case VIDEO_ENCODER_UPDATE_PARAM_REQ:
      TraceUpdateEncoderParams(pContentStr);
      break;

    case VIDEO_DECODER_UPDATE_PARAM_REQ:
      TraceUpdateDecoderParams(pContentStr);
      break;

    case VIDEO_UPDATE_DECODER_RESOLUTION_REQ:
      TraceUpdateDecoderResolutionReq(pContentStr);
      break;

    case ALLOC_STATUS_PER_UNIT_REQ:
      TraceAllocateStatusPerUnitReq(pContentStr);
      break;

    case ADD_VIDEO_OPERATION_POINT_SET:
    case REMOVE_VIDEO_OPERATION_POINT_SET:
    {
        TraceOperationPointsSet(pContentStr);
        break;
     }

    case ACK_IND:
      TraceMplAckInd(pContentStr);
      break;

    case NET_SETUP_REQ:
      TraceNetSetupReq(pContentStr);
      break;

    case NET_SETUP_IND:
      TraceNetSetupInd(pContentStr);
      break;

    case NET_CONNECT_REQ:
      TraceNetConectReq(pContentStr);
      break;

    case NET_CONNECT_IND:
      TraceNetConnectInd(pContentStr);
      break;

    case NET_CLEAR_REQ:
      TraceNetClearReq(pContentStr);
      break;

    case NET_CLEAR_IND:
      TraceNetClearInd(pContentStr);
      break;

    case NET_PROGRESS_IND:
      TraceNetProgressInd(pContentStr);
      break;

    case NET_ALERT_IND:
      TraceNetAlertInd(pContentStr);
      break;

    case NET_ALERT_REQ:
      TraceNetAlertReq(pContentStr);
      break;

    case NET_PROCEED_IND:
      TraceNetProceedInd(pContentStr);
      break;

    case NET_DISCONNECT_IND:
      TraceNetDisconnectInd(pContentStr);
      break;

    case NET_DISCONNECT_ACK_REQ:
      TraceNetDisconnectReq(pContentStr);
      break;

    case NET_DISCONNECT_ACK_IND:
      TraceNetDisconnectAckInd(pContentStr);
      break;

    case VIDEO_DECODER_SYNC_IND:
      TraceDecoderSyncInd(pContentStr);
      break;

    case CONFPARTY_CM_OPEN_UDP_PORT_REQ:
      TraceCmOpenUdpPortOrUpdateUdpAddrReq(pContentStr, "CONFPARTY_CM_OPEN_UDP_PORT_REQ");
      break;

    case CONF_PARTY_MRMP_OPEN_CHANNEL_REQ:
      TraceCmMrmpOpenChannelReq(pContentStr, "CONF_PARTY_MRMP_OPEN_CHANNEL_REQ");
      break;

    case CONF_PARTY_MRMP_UPDATE_CHANNEL_REQ:
      TraceCmMrmpOpenChannelReq(pContentStr, "CONF_PARTY_MRMP_UPDATE_CHANNEL_REQ");
      break;

    case CONF_PARTY_MRMP_SCP_STREAM_REQ_IND:
    	TraceCmMrmpScpStreamReqInd(pContentStr, "CONF_PARTY_MRMP_SCP_STREAM_REQ_IND");
      break;
    case CONF_PARTY_MRMP_SCP_STREAM_ACK_REQ:
    	TraceCmMrmpScpStreamAckReq(pContentStr, "CONF_PARTY_MRMP_SCP_STREAM_ACK_REQ");
      break;
    case CONF_PARTY_MRMP_SCP_STREAM_NOTIFICATION_REQ:
    	TraceCmMrmpScpNotificationReq(pContentStr, "CONF_PARTY_MRMP_SCP_STREAM_NOTIFICATION_REQ");
      break;
    case CONF_PARTY_MRMP_SCP_IVR_STATE_NOTIFICATION_REQ:
    	TraceCmMrmpScpIvrStateNotificationReq(pContentStr, "CONF_PARTY_MRMP_SCP_IVR_STATE_NOTIFICATION_REQ");
        break;
    case CONF_PARTY_MRMP_SCP_STREAM_NOTIFICATION_ACK_IND:
    	TraceCmMrmpScpNotificationAckInd(pContentStr, "CONF_PARTY_MRMP_SCP_STREAM_NOTIFICATION_ACK_IND");
      break;
    case CONF_PARTY_MRMP_SCP_STREAM_NOTIFICATION_IND:
    	TraceCmMrmpScpNotificationInd(pContentStr, "CONF_PARTY_MRMP_SCP_STREAM_NOTIFICATION_IND");
      break;
    case CONF_PARTY_MRMP_SCP_STREAM_NOTIFICATION_ACK_REQ:
    	TraceCmMrmpScpNotificationAckReq(pContentStr, "CONF_PARTY_MRMP_SCP_STREAM_NOTIFICATION_ACK_REQ");
      break;

    case CONF_PARTY_MRMP_CLOSE_CHANNEL_REQ:
    	TraceCmMrmpCloseChannelReq(pContentStr, "CONF_PARTY_MRMP_CLOSE_CHANNEL_REQ");
      break;

    case CONF_PARTY_MRMP_RTCP_FIR_REQ:
      TraceCmMrmpRtcpFirReq(pContentStr, "CONF_PARTY_MRMP_RTCP_FIR_REQ");
      break;

    case CONF_PARTY_MRMP_RTCP_FIR_IND:
      TraceCmMrmpRtcpFirInd(pContentStr, "CONF_PARTY_MRMP_RTCP_FIR_IND");
      break;

    case CONF_PARTY_MRMP_SCP_PIPES_MAPPING_NOTIFICATION_REQ:
      TraceCmMrmpScpPipesMappingNotificationReq(pContentStr);
      break;

    case CONF_PARTY_MRMP_PARTY_MONITORING_REQ:
      TracePartyMrmpPartyMonitoringReq(pContentStr);
      break;

    case CONF_PARTY_MRMP_PARTY_MONITORING_IND:
    	TraceCmMrmpPartyMonitoringInd(pContentStr);
      break;

    case CONF_PARTY_MRMP_VIDEO_SOURCES_REQ:
        TraceVideoSourcesReq(pContentStr, "CONF_PARTY_MRMP_VIDEO_SOURCES_REQ");
        break;

    case CONFPARTY_CM_CLOSE_UDP_PORT_REQ:
      TraceCmCloseUdpPortReq(pContentStr, "CONFPARTY_CM_CLOSE_UDP_PORT_REQ");
      break;

    case KILL_UDP_PORT_REQ:
      TraceCmKillUdpPort(pContentStr, "KILL_UDP_PORT_REQ");
      break;

    case STARTUP_DEBUG_RECORDING_PARAM_REQ:
      TraceMediaRecordingPath(pContentStr, "STARTUP_DEBUG_RECORDING_PARAM_REQ");
      break;

    case IP_RTP_SET_FECC_PARTY_TYPE:
      TraceIpRtpSetFeccPartyType(pContentStr, "IP_RTP_SET_FECC_PARTY_TYPE");
      break;

    case IP_CM_RTCP_VIDEO_PREFERENCE_REQ:
      TraceCmRtcpVideoPreference(pContentStr, "IP_CM_RTCP_VIDEO_PREFERENCE_REQ");
      break;

    case IP_CM_RTCP_RECEIVER_BANDWIDTH_REQ:
      TraceCmRtcpReceiverBandwidth(pContentStr, "IP_CM_RTCP_RECEIVER_BANDWIDTH_REQ");
      break;

    case IVR_START_IVR_REQ:
      TraceIvrStartReq(pContentStr);
      break;

    case IVR_STOP_IVR_REQ:
      TraceIvrStopReq(pContentStr);
      break;

    case IVR_PLAY_MESSAGE_REQ:
      TraceIvrPlayMsgReq(pContentStr);
      break;

    case IVR_PLAY_MUSIC_REQ:
      TraceIvrPlayMusicReq(pContentStr);
      break;

    case IVR_RECORD_ROLL_CALL_REQ:
      TraceIvrRecordRollCallReq(pContentStr);
      break;

    case IVR_RECORD_ROLL_CALL_IND:
      TraceIvrRecordRollCallInd(pContentStr);
      break;

    case IVR_STOP_RECORD_ROLL_CALL_REQ:
      TraceIvrStopRecordRollCallReq(pContentStr);
      break;

    case IVR_SHOW_SLIDE_REQ:
      TraceIvrShowSlideReq(pContentStr);
      break;

    case IVR_STOP_SHOW_SLIDE_REQ:
      TraceIvrStopShowSlideReq(pContentStr);
      break;

    case IVR_UPDATE_STANDALONE_REQ:
      TraceUpdateAudioStandaloneReq(pContentStr);
      break;

    case IVR_FAST_UPDATE_REQ:
      TraceIvrFastUpdateReq(pContentStr);
      break;

    case AUDIO_PLAY_TONE_REQ:
      TraceAudioPlayToneReq(pContentStr);
      break;

    case AUD_DTMF_IND_VAL:
    	TraceAudioDtmfIndVal(pContentStr);
      break;

    case IP_RTP_DTMF_INPUT_IND:
    	TraceIpRtpDtmfInputInd(pContentStr);
      break;

    case MOVE_PARTY_RESOURCE_REQ:
      TraceMoveResourceMFAReq(pContentStr);
      break;

    case SM_FATAL_FAILURE_IND:
      TraceSMFatalFailureInd(pContentStr, "SM_FATAL_FAILURE_IND");
      break;

    case H221_INIT_COMM:
      TraceMuxInitComm(pContentStr);
      break;

    case END_INIT_COMM:
      TraceEndInitComm(pContentStr);
      break;

    case SET_XMIT_MODE:
      TraceSetXmitMode(pContentStr);
      break;

    case REMOTE_XMIT_MODE:
      TraceRmtXmitMode(pContentStr);
      break;

    case EXCHANGE_CAPS:
      TraceExchangeCap(pContentStr);
      break;

    case REMOTE_BAS_CAPS:
      TraceRemoteBasCap(pContentStr);
      break;

    case SEND_H_230:
    case REMOTE_CI:
      TraceH230MBE(pContentStr);
      break;

    case REPEATED_H230:
      TraceMuxRepeatedH230(pContentStr);
      break;

    case REMOTE_MFRAME_SYNC:
      TraceRemoteMframeSync(pContentStr);
      break;

    case LOCAL_MFRAME_SYNC:
      TraceLocalMframeSync(pContentStr);
      break;

    case SMART_RECOVERY_UPDATE:
      TraceSmartRecovery(pContentStr);
      break;

    case CM_UNIT_RECONFIG_REQ:
      TraceUnitReconfig(pContentStr);
      break;

    case CM_UNIT_RECONFIG_IND:
      TraceUnitReconfig(pContentStr);
      break;

    case START_DEBUG_RECORDING_REQ:
      TraceStartRecordingRequest(pContentStr);
      break;

    case PCM_INDICATION:
    case PCM_COMMAND:
    case PCM_CONFIRM:
      TracePCMMessage(pContentStr);
      break;

    case TB_MSG_CONNECT_PCM_REQ:
    case TB_MSG_DISCONNECT_PCM_REQ:
      TraceConnectDisconnetPCM(pContentStr);
      break;

    case VIDEO_GRAPHICS_SHOW_TEXT_BOX_REQ:
      TraceVideoGraphicsShowTextBoxRequest(pContentStr);
      break;

    case VIDEO_GRAPHICS_SHOW_ICON_REQ:
      TraceVideoGraphicsShowIconRequest(pContentStr);
      break;

    case SET_ECS:
      TraceSetEcs(pContentStr);
      break;

    case REMOTE_ECS:
      TraceRemoteEcs(pContentStr);
      break;

    case ENC_KEYS_INFO_REQ:
      TraceEncKeysInfoReq(pContentStr);
      break;

    case SLOTS_NUMBERING_CONVERSION_IND:
      TraceSlotsNumbering(pContentStr);
      break;

    case AUDIO_UPDATE_COMPRESSED_AUDIO_DELAY_REQ:
      TraceAudioUpdateCompressedDelay(pContentStr);
      break;

    case UNIT_RECOVERY_IND:
    case UNIT_RECOVERY_END_IND:
    case UNIT_FATAL_IND:
    case UNIT_UNFATAL_IND:
      TraceRecoveryUnit(pContentStr);
      break;

    case CM_HIGH_CPU_USAGE_IND:
      TraceHighUsageCPUInd(pContentStr);
      break;

    case RECOVERY_REPLACEMENT_UNIT_REQ:
      TraceRecoveryReplaceUnit(pContentStr);
      break;

    case PARTY_DEBUG_INFO_IND:
    case CONF_DEBUG_INFO_IND:
    case PARTY_CM_DEBUG_INFO_IND:
      TracePortDebugInfoInd(pContentStr);
      break;

    case CONF_DEBUG_INFO_REQ:
      TraceConfPortDebugInfoReq(pContentStr);
      break;

    case CARD_CONFIG_REQ:
      TraceCardConfigReq(pContentStr);
      break;

    case SET_LOG_LEVEL_REQ:
      TraceSetLogLevelReq(pContentStr);
      break;

    case IP_CM_TIP_START_NEGOTIATION_REQ:
      TraceTipStartNegotiationReq(pContentStr);
      break;

    case IP_CM_TIP_NEGOTIATION_RESULT_IND:
      TraceTipNegotiationResultInd(pContentStr);
      break;

    case IP_CM_TIP_END_NEGOTIATION_REQ:
      TraceTipEndNegotiationReq(pContentStr);
      break;

    case IP_CM_TIP_CONTENT_MSG_REQ:
      TraceTipContentMsgReq(pContentStr);
      break;

    case IP_CM_TIP_CONTENT_MSG_IND:
      TraceTipContentMsgInd(pContentStr);
      break;

    case IP_MSG_UPDATE_ON_TIP_CALL_REQ:
    case IP_MSG_CLOSE_TIP_CALL_REQ:
      TraceTipUpdateCloseCall(pContentStr);
      break;

    case H323_RTP_PARTY_MONITORING_REQ:
      TracePartyMonitoringReq(pContentStr);
      break;

    case RTP_PARTY_VIDEO_CHANNELS_STATISTICS_REQ:
        TracePartyVideoChannelsStatisticsReq(pContentStr); //CDR_MCCF
        break;

    case VIDEO_ENCODER_ICONS_DISPLAY_REQ:
      TraceVideoEncLayoutIndications(pContentStr);
      break;

    case ICE_INIT_REQ:
    	TraceIceInitReq(pContentStr);
    	break;

    case IP_CM_DTLS_START_REQ:
    	TraceDtlsStartReq(pContentStr, "IP_CM_DTLS_START_REQ");
    	break;

    case IP_CM_DTLS_CLOSE_REQ:
    	TraceDtlsStartReq(pContentStr, "IP_CM_DTLS_CLOSE_REQ");
		break;

    case IP_CM_DTLS_END_IND:
    	TraceDtlsEndInd(pContentStr);
    	break;

    case CG_PLAY_AUDIO_REQ:
    	TraceCGPlayAudioReq(pContentStr);
    	break;

    case CONF_PARTY_MRMP_STREAM_IS_MUST_REQ:
    	TraceMrmpStreamIsMust(pContentStr);
    	break;
    case AUDIO_ENCODER_UPDATE_SEEN_IMAGE_SSRC:
    	TraceAudioEncUpdateSeenImageReq(pContentStr);
    	break;

    case IP_CM_WEBRTC_CONNECT_REQ:
    	TraceWebRtcConnectReq(pContentStr);
    	break;
    case IP_CM_WEBRTC_CONNECT_IND:
    	TraceWebRtcConnectInd(pContentStr);
    	break;
    case ICE_INIT_IND:
    	TraceIceInitInd(pContentStr);
    	break;
    default:
      *pContentStr <<"\nCONTENT: Unrecognized Content ";
      break;
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceTbMsgConnectDisconnectReq(CObjString *pContentStr)
{
  const TB_MSG_CONNECT_S *pTbConnectStruct = (const TB_MSG_CONNECT_S *)m_pMplMcmsProt->getpData();

  static CLargeString physicalPort1Str;
  physicalPort1Str.Clear();
  static CLargeString physicalPort2Str;
  physicalPort2Str.Clear();

  TraceMcmsMplPhysicalRsrcInfo(&physicalPort1Str, pTbConnectStruct->physical_port1);
  TraceMcmsMplPhysicalRsrcInfo(&physicalPort2Str, pTbConnectStruct->physical_port2);

  *pContentStr << "\nCONTENT: TB_MSG_CONNECT_S:";
  *pContentStr << "\n--physical_port1-----------------:" << physicalPort1Str;
  *pContentStr << "\n--physical_port2-----------------:" << physicalPort2Str;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceCmOpenUdpPortOrUpdateUdpAddrReq(CObjString *pContentStr, const char* OpcodeString)
{
  const TOpenUdpPortOrUpdateUdpAddrMessageStruct* pStructWithAll = (const TOpenUdpPortOrUpdateUdpAddrMessageStruct*)m_pMplMcmsProt->getpData();
  const mcReqCmOpenUdpPortOrUpdateUdpAddr& st = pStructWithAll->tCmOpenUdpPortOrUpdateUdpAddr;

  TraceMcmsMplPhysicalRsrcInfo(pContentStr, pStructWithAll->physicalPort);

  *pContentStr << "\nCONTENT: " << OpcodeString;
  *pContentStr << "\n  channelType                    :"; ::GetChannelTypeName(st.channelType, *pContentStr);
  *pContentStr << "\n  channelDirection               :"; ::GetChannelDirectionName(st.channelDirection, *pContentStr);
  *pContentStr << "\n  CmLocalUdpAddressIp.transport  :" << GetTransportTypeName(st.CmLocalUdpAddressIp.transportType);
  *pContentStr << "\n  CmLocalUdpAddressIp.ipVersion  :" << st.CmLocalUdpAddressIp.ipVersion;

  if (st.CmLocalUdpAddressIp.ipVersion == eIpVersion4)
  {
    *pContentStr << " (V4)";
    *pContentStr << "\n  CmLocalUdpAddressIp.addr.v4.ip :";
    char str[30];
    sprintf(str, "%x", st.CmLocalUdpAddressIp.addr.v4.ip);
    *pContentStr << "0x" << str;
  }
  else
  {
    *pContentStr << " (V6)";
    *pContentStr << "\n  CmLocalUdpAddressIp.addr.v6.ip :";
    char str[64];
    memset(str, '\0', 64);
    ::ipToString(st.CmLocalUdpAddressIp, str, 1);
    *pContentStr << str;
  }

  *pContentStr << "\n  CmLocalUdpAddressIp.port       :" << st.CmLocalUdpAddressIp.port;
  *pContentStr << "\n  Local RTCP port                :" << st.LocalRtcpPort;
  *pContentStr << "\n  CmRemoteUdpAddressIp.transport :" << GetTransportTypeName(st.CmRemoteUdpAddressIp.transportType);
  *pContentStr << "\n  CmRemoteUdpAddressIp.ipVersion :" << st.CmRemoteUdpAddressIp.ipVersion;

  if (st.CmRemoteUdpAddressIp.ipVersion == eIpVersion4)
  {
    *pContentStr << " (V4)";
    *pContentStr << "\n  CmRemoteUdpAddressIp.addr.v4.ip:";
    char str[30];
    sprintf(str, "%x", st.CmRemoteUdpAddressIp.addr.v4.ip);
    *pContentStr << "0x" << str;
  }
  else
  {
    *pContentStr << "V6";
    *pContentStr << "\n CmRemoteUdpAddressIp.addr.v6.ip :";
    char str[64];
    memset(str, '\0', 64);
    ::ipToString(st.CmRemoteUdpAddressIp, str, 1);
    *pContentStr << str;
  }
  *pContentStr << "\n  CmRemoteUdpAddressIp.port      :" << st.CmRemoteUdpAddressIp.port;
  *pContentStr << "\n  Remote RTCP port               :" << st.RemoteRtcpPort;
  *pContentStr << "\n  tosValue[MEDIA_TOS_VALUE_PLACE]:" << st.tosValue[MEDIA_TOS_VALUE_PLACE];
  *pContentStr << "\n  tosValue[RTCP_TOS_VALUE_PLACE] :" << st.tosValue[RTCP_TOS_VALUE_PLACE];
  *pContentStr << "\n  Ice channel RTP id             :" << st.ice_channel_rtp_id;
  *pContentStr << "\n  Ice channel RTCP id            :" << st.ice_channel_rtcp_id;
  char strMask[30];
   sprintf(strMask, "%x", st.RtcpCnameMask);
  *pContentStr << "\n  RTCP cname mask                :" << "0x" << strMask;
  *pContentStr <<" \n  connMode				      :" << GetConnectionTransportModeName(st.connMode);
  *pContentStr << "\n   MediaDetection Time          :" << st.ulDetectionTimerLen;
  *pContentStr << "\n   Msft EP Type          		 :" << st.uMsftType;
}



void CMplMcmsProtocolTracer::TraceCmMrmpScpStreamReqInd(CObjString *pContentStr, const char* OpcodeString)
{

	PTRACE(eLevelInfoNormal,"eyaln CMplMcmsProtocolTracer::TraceCmMrmpScpStreamReqInd ");
	const MrmpScpStreamsRequestStruct* pStruct = (const MrmpScpStreamsRequestStruct*)m_pMplMcmsProt->getpData();
	int NumberOfMediaStream=pStruct->nNumberOfMediaStream;
	const MrmpStreamDesc*  pUdpSt= &(pStruct->mediaStreams[0]);
	int i;

	  *pContentStr << "\nChannelHandle "<<pStruct->unChannelHandle;
	  *pContentStr << "\nSequenseNumber "<<pStruct->unSequenseNumber;
	  *pContentStr << "\nNumber of media streams: " << NumberOfMediaStream;
	  if(NumberOfMediaStream>100)
	  {
		  *pContentStr << "\nNumber of media streams seems too high - truncating";
		  NumberOfMediaStream=100;
	  }

	  for(i=0;i<NumberOfMediaStream;++i)
	  {
		  *pContentStr << "\nStream no. " << i << ":";
		  *pContentStr << "\n\tChannelType: " << pUdpSt[i].unChannelType;
		  *pContentStr << "\n\tPayloadType: " << pUdpSt[i].unPayloadType;
		  *pContentStr << "\n\tSpecificSourceSsrc: " << pUdpSt[i].unSpecificSourceSsrc;
		  *pContentStr << "\n\tBitRate: " << pUdpSt[i].unBitRate;
		  *pContentStr << "\n\tFrameRate: " << pUdpSt[i].unFrameRate;
		  *pContentStr << "\n\tHeight: " << pUdpSt[i].unHeight;
		  *pContentStr << "\n\tWidth: "  << pUdpSt[i].unWidth;
		  *pContentStr << "\n\tPipeIdSsrc: "  << pUdpSt[i].unPipeIdSsrc;
		  *pContentStr << "\n\tSourceIdSsrc: " << pUdpSt[i].unSourceIdSsrc;
		  *pContentStr << "\n\tPriority: " << pUdpSt[i].unPriority;
	  }


}

void CMplMcmsProtocolTracer::TraceCmMrmpScpStreamAckReq(CObjString *pContentStr, const char* OpcodeString)
{
	PTRACE(eLevelInfoNormal,"eyaln CMplMcmsProtocolTracer::TraceCmMrmpScpStreamAckReq ");
	const MrmpScpAckStruct* pStruct = (const MrmpScpAckStruct*)m_pMplMcmsProt->getpData();
	*pContentStr << "\nChannelHandle: " << pStruct->unChannelHandle;
	*pContentStr << "\nRemoteSequenseNumber: " << pStruct->unRemoteSequenseNumber;
	*pContentStr << "\nbIsAck: " << pStruct->bIsAck;
}

void CMplMcmsProtocolTracer::TraceCmMrmpScpNotificationReq(CObjString *pContentStr, const char* opcodeString)
{
	PTRACE(eLevelInfoNormal,"CMplMcmsProtocolTracer::TraceCmMrmpScpNotificationReq");
	TraceCmMrmpScpNotification(pContentStr, opcodeString);
}

void CMplMcmsProtocolTracer::TraceCmMrmpScpIvrStateNotificationReq(CObjString *pContentStr,const char* OpcodeString)
{
	const MrmpScpIvrStateNotificationStruct* pStruct = (const MrmpScpIvrStateNotificationStruct*)m_pMplMcmsProt->getpData();
	*pContentStr << "\nChannelHandle: " << pStruct->unChannelHandle;
	*pContentStr << "\nSequenseNumber: " << pStruct->unSequenseNumber;
	*pContentStr << "\nIvrState: " << pStruct->unIvrState;
}

void CMplMcmsProtocolTracer::TraceCmMrmpScpNotificationInd(CObjString *pContentStr, const char* opcodeString)
{
	PTRACE(eLevelInfoNormal,"CMplMcmsProtocolTracer::TraceCmMrmpScpNotificationInd");
	TraceCmMrmpScpNotification(pContentStr, opcodeString);
}

void CMplMcmsProtocolTracer::TraceCmMrmpScpNotification(CObjString *pContentStr, const char* opcodeString)
{
	const MrmpScpStreamsNotificationStruct* pStruct = (const MrmpScpStreamsNotificationStruct*)m_pMplMcmsProt->getpData();

	int NumberOfPipes=pStruct->nNumberOfScpPipes;
	const MrmpScpPipe*  pPipeSt= &(pStruct->scpPipe[0]);
	int i;

	*pContentStr << "\n--- " << opcodeString << " ---";

	*pContentStr << "\nChannelHandle "<<pStruct->unChannelHandle;
	*pContentStr << "\nSequenseNumber "<<pStruct->unSequenseNumber;
	*pContentStr << "\nRemoteSequenseNumber "<<pStruct->unRemoteSequenseNumber;
	*pContentStr << "\nNumber of pipes: " << NumberOfPipes;
	if(NumberOfPipes>100)
	{
		*pContentStr << "\nNumber of media streams seems too high - truncating";
		NumberOfPipes=100;
	}

	for(i=0;i<NumberOfPipes;++i)
	{
		  *pContentStr << "\nStream no. " << i << ":";
		  *pContentStr << "\n\tPipeId:      " << pPipeSt[i].unPipeId;
		  *pContentStr << "\n\tNotifyType:  " << pPipeSt[i].unNotifyType;
		  *pContentStr << "\n\tReason:      " << pPipeSt[i].unReason;
		  *pContentStr << "\n\tIsPermanent: " << (BOOL)(pPipeSt[i].bIsPermanent);
	}
}

void CMplMcmsProtocolTracer::TraceCmMrmpScpNotificationAckInd(CObjString *pContentStr, const char* opcodeString)
{
	PTRACE(eLevelInfoNormal,"CMplMcmsProtocolTracer::TraceCmMrmpScpNotificationAckInd");
	TraceCmMrmpScpNotificationAck(pContentStr, opcodeString);
}

void CMplMcmsProtocolTracer::TraceCmMrmpScpNotificationAckReq(CObjString *pContentStr, const char* opcodeString)
{
	PTRACE(eLevelInfoNormal,"CMplMcmsProtocolTracer::TraceCmMrmpScpNotificationAckReq");
	TraceCmMrmpScpNotificationAck(pContentStr, opcodeString);
}

void CMplMcmsProtocolTracer::TraceCmMrmpScpNotificationAck(CObjString *pContentStr, const char* opcodeString)
{
	const MrmpScpAckStruct* pStruct = (const MrmpScpAckStruct*)m_pMplMcmsProt->getpData();

	*pContentStr << "\n--- " << opcodeString << " ---";

	*pContentStr << "\nChannelHandle:        " << pStruct->unChannelHandle;
	*pContentStr << "\nRemoteSequenseNumber: " << pStruct->unRemoteSequenseNumber;
	*pContentStr << "\nbIsAck:               " << pStruct->bIsAck;
}

void CMplMcmsProtocolTracer::TraceCmMrmpOpenChannelReq(CObjString *pContentStr, const char* OpcodeString)
{

  const MrmpOpenChannelRequestStruct* pStruct = (const MrmpOpenChannelRequestStruct*)m_pMplMcmsProt->getpData();;


  *pContentStr << "\nCONTENT: " << OpcodeString;

  *pContentStr << "\n  channelType                    :"; ::GetChannelTypeName(pStruct->m_channelType, *pContentStr);
  *pContentStr << "\n  channelDirection               :"; ::GetChannelDirectionName(pStruct->m_channelDirection, *pContentStr);
  // (Amir commented this line) *pContentStr << "\n  videoChannelType               :" << EMrmpVideoChannelTypeNames[pStruct->m_videoChannelType];

  *pContentStr << "\n  CmLocalUdpAddressIp.transport  :" << GetTransportTypeName(pStruct->m_localAddress.transportType);
  *pContentStr << "\n  CmLocalUdpAddressIp.ipVersion  :" << pStruct->m_localAddress.ipVersion;

    if (pStruct->m_localAddress.ipVersion == eIpVersion4)
    {
      *pContentStr << " (V4)";
      *pContentStr << "\n  CmLocalUdpAddressIp.addr.v4.ip :";
      char str[30];
      sprintf(str, "%x", pStruct->m_localAddress.addr.v4.ip);
      *pContentStr << "0x" << str;
    }
    else
    {
      *pContentStr << " (V6)";
      *pContentStr << "\n  CmLocalUdpAddressIp.addr.v6.ip :";
      char str[64];
      memset(str, '\0', 64);
      ::ipToString(pStruct->m_localAddress, str, 1);
      *pContentStr << str;
    }


    *pContentStr << "\n  CmLocalUdpAddressIp.port       :" << pStruct->m_localAddress.port;
    *pContentStr << "\n  Local RTCP port                :" << pStruct->m_localRtcpAddress.port;
    *pContentStr << "\n  CmRemoteUdpAddressIp.transport :" << GetTransportTypeName(pStruct->m_remoteAddress.transportType);
    *pContentStr << "\n  CmRemoteUdpAddressIp.ipVersion :" << pStruct->m_remoteAddress.ipVersion;

    if (pStruct->m_remoteAddress.ipVersion == eIpVersion4)
    {
      *pContentStr << " (V4)";
      *pContentStr << "\n  CmRemoteUdpAddressIp.addr.v4.ip:";
      char str[30];
      sprintf(str, "%x", pStruct->m_remoteAddress.addr.v4.ip);
      *pContentStr << "0x" << str;
    }
    else
    {
      *pContentStr << "V6";
      *pContentStr << "\n CmRemoteUdpAddressIp.addr.v6.ip :";
      char str[64];
      memset(str, '\0', 64);
      ::ipToString(pStruct->m_remoteAddress, str, 1);
      *pContentStr << str;
    }
    *pContentStr << "\n  CmRemoteUdpAddressIp.port      :" << pStruct->m_remoteAddress.port;
    *pContentStr << "\n  Remote RTCP port               :" << pStruct->m_remoteRtcpAddress.port;


    *pContentStr << "\n  CapTypeCode            :"<< pStruct->m_capTypeCode;

    *pContentStr << "\n  Payload Type           :" << pStruct->m_PayloadType;
    *pContentStr << "\n  Dtmf Payload Type      :" << pStruct->m_dtmfPayloadType;
    *pContentStr << "\n  Operation point set id :" << pStruct->m_operationPointsSetId;
    *pContentStr << "\n  Party id               :" << pStruct->m_partyId;

   for(int i=0;i<MAX_SSRC_PER_INCOMING_CHANNEL;++i)
    {
       	if(pStruct->m_ssrcInfo[i].m_ssrc)
       	{
       		*pContentStr << "\n  SSRC index             :" <<i<<" id: "<<pStruct->m_ssrcInfo[i].m_ssrc;
       	}

    }
   *pContentStr << "\n  Video Flag             :" << pStruct->m_videoFlag;
   *pContentStr << "\n  Legacy                 :" << pStruct->m_fLegacy;


   *pContentStr << "\nPhysical Resources Number :" << pStruct->m_allocatedPhysicalResources;
   static CLargeString physicalPortStr;
   for (unsigned int i = 0; i < pStruct->m_allocatedPhysicalResources; i++)
   {
       physicalPortStr.Clear();
       TraceMcmsMplPhysicalRsrcInfo(&physicalPortStr, pStruct->physicalId[i]);
       *pContentStr << "\n Physical port info:"<< physicalPortStr;
   }

   *pContentStr << "\n Max video BR (for video channel):"<< pStruct->m_maxVideoBR;

   TraceSdesSpecificParams(pContentStr, pStruct->m_sDesCapSt); // SRTP SVC


   *pContentStr << "\n Rtp KeepAlive Period:" << pStruct->uRtpKeepAlivePeriod;

   for (int i=0; i<NumberOfTosValues; i++)
   {
	   *pContentStr << "\n tosValue " << i << ":" << pStruct->tosValue[i];
   }

   *pContentStr << "\n ice channel rtp id: " << pStruct->ice_channel_rtp_id;
   *pContentStr << "\n ice channel rtcp id:" << pStruct->ice_channel_rtcp_id;
   *pContentStr << "\n Is Ice Enable:" << pStruct->bIsIceEnable;
   *pContentStr << "\n MediaDetection Time		  :" <<  pStruct->ulDetectionTimerLen;
   *pContentStr << "\n MRD Version: " <<  pStruct->mrdVersion.majorVer << "." << pStruct->mrdVersion.middleVer << "."  << pStruct->mrdVersion.minorVer;
   *pContentStr << "\n Is Link: " << pStruct->bIsLinkParty;
}
//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceCmMrmpRtcpFirReq(CObjString *pContentStr, const char* OpcodeString)
{

	PTRACE(eLevelInfoNormal,"CMplMcmsProtocolTracer::TraceCmMrmpRtcpFirReq ");

  const MrmpRtcpFirStruct* pStruct = (const MrmpRtcpFirStruct*)m_pMplMcmsProt->getpData();

  if (pStruct == NULL) return;

  *pContentStr << "\nCONTENT: " << OpcodeString;
  *pContentStr << "\n Sequence number: " << pStruct->unSequenseNumber;
  *pContentStr << "\n IsGdr: " << ( (TRUE==pStruct->bIsGdr) ? "true" : "false" );
  *pContentStr << "\n unChannelHandle :" << pStruct->unChannelHandle;
  *pContentStr << "\n nNumberOfSyncSources :" << pStruct->nNumberOfSyncSources;

  for (int i = 0; i < pStruct->nNumberOfSyncSources; i++)
	  *pContentStr << "\n syncSources: " << pStruct->syncSources[i];
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceCmMrmpRtcpFirInd(CObjString *pContentStr, const char* OpcodeString)
{

	PTRACE(eLevelInfoNormal,"CMplMcmsProtocolTracer::TraceCmMrmpRtcpFirInd ");


  const MrmpRtcpFirStruct* pStruct = (const MrmpRtcpFirStruct*)m_pMplMcmsProt->getpData();

  if (pStruct == NULL) return;

  *pContentStr << "\nCONTENT: " << OpcodeString;
  *pContentStr << "\n Sequence number: " << pStruct->unSequenseNumber;
  *pContentStr << "\n IsGdr: " << ( (TRUE==pStruct->bIsGdr) ? "true" : "false" );
  *pContentStr << "\n unChannelHandle :" << pStruct->unChannelHandle;
  *pContentStr << "\n nNumberOfSyncSources :" << pStruct->nNumberOfSyncSources;

  for (int i = 0; i < pStruct->nNumberOfSyncSources; i++)
	  *pContentStr << "\n syncSources: " << pStruct->syncSources[i];
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceCmMrmpScpPipesMappingNotificationReq(CObjString *pContentStr)
{
	const MrmpScpPipesMappingNotificationStruct* pStruct = (const MrmpScpPipesMappingNotificationStruct*)m_pMplMcmsProt->getpData();

	int NumberOfPipes=pStruct->nNumberOfScpPipes;
	const MrmpScpPipeMapping*  pPipeSt= &(pStruct->scpPipeMapping[0]);
	int i;

	*pContentStr << "\n--- CONF_PARTY_MRMP_SCP_PIPES_MAPPING_NOTIFICATION_REQ";

	*pContentStr << "\nChannelHandle "<<pStruct->unChannelHandle;
	*pContentStr << "\nSequenseNumber "<<pStruct->unSequenseNumber;
	*pContentStr << "\nRemoteSequenseNumber "<<pStruct->unRemoteSequenseNumber;
	*pContentStr << "\nNumber of pipes: " << NumberOfPipes;
	if(NumberOfPipes>100)
	{
		*pContentStr << "\nNumber of media streams seems too high - truncating";
		NumberOfPipes=100;
	}

	for(i=0;i<NumberOfPipes;++i)
	{
		  *pContentStr << "\nStream no. " << i << ":";
		  *pContentStr << "\n\tPipeId:  " << pPipeSt[i].unPipeId;
		  *pContentStr << "\n\tCSRC:    " << pPipeSt[i].unCsrc;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceCmMrmpPartyMonitoringInd(CObjString *pContentStr)
{
	*pContentStr << "\n--- CONF_PARTY_MRMP_PARTY_MONITORING_IND ---";

	// to fill the content



	/*
	const MrmpScpPipesMappingNotificationStruct* pStruct = (const MrmpScpPipesMappingNotificationStruct*)m_pMplMcmsProt->getpData();

	int NumberOfPipes=pStruct->nNumberOfScpPipes;
	const MrmpScpPipeMapping*  pPipeSt= &(pStruct->scpPipeMapping[0]);
	int i;

	*pContentStr << "\n--- " << opcodeString << " ---";

	*pContentStr << "\nChannelHandle "<<pStruct->unChannelHandle;
	*pContentStr << "\nSequenseNumber "<<pStruct->unSequenseNumber;
	*pContentStr << "\nRemoteSequenseNumber "<<pStruct->unRemoteSequenseNumber;
	*pContentStr << "\nNumber of pipes: " << NumberOfPipes;
	if(NumberOfPipes>100)
	{
		*pContentStr << "\nNumber of media streams seems too high - truncating";
		NumberOfPipes=100;
	}

	for(i=0;i<NumberOfPipes;++i)
	{
		  *pContentStr << "\nStream no. " << i << ":";
		  *pContentStr << "\n\tPipeId:  " << pPipeSt[i].unPipeId;
		  *pContentStr << "\n\tCSRC:    " << pPipeSt[i].unCsrc;
	}
	*/
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceCmKillUdpPort(CObjString *pContentStr, const char* OpcodeString)
{
  const TKillUdpPortMessageStruct *pStructWithAll = (const TKillUdpPortMessageStruct *)m_pMplMcmsProt->getpData();
  const mcKillUdpPortRequestStruct &st = pStructWithAll->tCmKillUdpPort;

  TraceMcmsMplPhysicalRsrcInfo(pContentStr, pStructWithAll->physicalPort);

  *pContentStr << "\nCONTENT: " << OpcodeString << ":";

  for (WORD i = 0; i < MAX_NUM_OF_ALLOCATED_PARTY_UDP_PORTS; i++)
  {
	*pContentStr << "\n  ulIsIceConn: " << st.ulIsIceConn;
    *pContentStr << "\n  udp_ports[" << i << "]       :" << st.udp_ports[i];
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceMediaRecordingPath(CObjString *pContentStr, const char* OpcodeString)
{
  const TStartupDebugRecordingParamReq *pStructWithAll = (const TStartupDebugRecordingParamReq *)m_pMplMcmsProt->getpData();

  *pContentStr << "\nCONTENT: " << OpcodeString << ":";
  *pContentStr << "\n  MediaRecordingPath             :\"" << pStructWithAll->ucPathName << "\"";
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceIpRtpSetFeccPartyType(CObjString *pContentStr, const char* OpcodeString)
{
  const FECC_PARTY_TYPE_S* pStruct = (const FECC_PARTY_TYPE_S*)m_pMplMcmsProt->getpData();

  *pContentStr << "\nCONTENT: " << OpcodeString;
  *pContentStr << "\n  FECC party type                :" << ::feccPartyTypeToString((feccPartyTypeEnum)pStruct->uFeccPartyType) << ", (" << pStruct->uFeccPartyType << ")";
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceCmRtcpVideoPreference(CObjString *pContentStr, const char* OpcodeString)
{
  const TCmRtcpVideoPreference* pStruct = (const TCmRtcpVideoPreference*)m_pMplMcmsProt->getpData();

  *pContentStr << "\nCONTENT:";
  *pContentStr << "\nTCmRtcpVideoPreference:";
  *pContentStr << "\n  uDspNumber                     :" << (pStruct->tCmRtcpHeader).uDspNumber;
  *pContentStr << "\n  uPortNumber                    :" << (pStruct->tCmRtcpHeader).uPortNumber;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceCmRtcpReceiverBandwidth(CObjString *pContentStr, const char* OpcodeString)
{
  const TCmRtcpBandwidth* pStruct = (const TCmRtcpBandwidth*)m_pMplMcmsProt->getpData();

  *pContentStr << "\nCONTENT:";
  *pContentStr << "\nTCmRtcpBandwidth:";
  *pContentStr << "\n  uDspNumber                     :" << (pStruct->tCmRtcpHeader).uDspNumber;
  *pContentStr << "\n  uPortNumber                    :" << (pStruct->tCmRtcpHeader).uPortNumber;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceIvrStartReq(CObjString *pContentStr)
{
  //Partial trace of MSG to be completed
  const SIVRStartIVRStruct *pStartIvrStruct = (const SIVRStartIVRStruct *)m_pMplMcmsProt->getpData();

  static CLargeString physicalPort1Str;
  physicalPort1Str.Clear();

  TracePhysicalRsrcInfo(&physicalPort1Str, pStartIvrStruct->physicalPortDescription);

  *pContentStr << "\nCONTENT:";
  *pContentStr << "\nPhysicalPortDescription:";
  *pContentStr << physicalPort1Str;

  *pContentStr << "\n\nSIVRStartIVRStruct:";
  *pContentStr << "\n  partyOrconfFlag                :" << pStartIvrStruct->partyOrconfFlag;
  *pContentStr << "\n  mediaType                      :" << pStartIvrStruct->mediaType;

  TraceStartIVRParams(pContentStr, pStartIvrStruct->params);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceStartIVRParams(CObjString *pContentStr, const SIVRStartIVRParams &startIVRParams)
{
  *pContentStr << "\nStartIVRParams:";
  *pContentStr << "\n  privateIVRMsgVolume            :" << startIVRParams.privateIVRMsgVolume;
  *pContentStr << "\n  privateIVRMusicVolume          :" << startIVRParams.privateIVRMusicVolume;
  *pContentStr << "\n  confIVRMsgVolume               :" << startIVRParams.confIVRMsgVolume;
  *pContentStr << "\n  confIVRMusicVolume             :" << startIVRParams.confIVRMusicVolume;
  *pContentStr << "\n  encoderConfMixVolume           :" << startIVRParams.encoderConfMixVolume;
  *pContentStr << "\n  decoderConfMixVolume           :" << startIVRParams.decoderConfMixVolume;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceIvrStopReq(CObjString *pContentStr)
{
  //Partial trace of MSG to be completed
  const SIVRStopIVRStruct *pStopIvrStruct = (const SIVRStopIVRStruct *)m_pMplMcmsProt->getpData();

  static CLargeString physicalPort1Str;
  physicalPort1Str.Clear();

  TracePhysicalRsrcInfo(&physicalPort1Str, pStopIvrStruct->physicalPortDescription);

  *pContentStr << "\nCONTENT:";
  *pContentStr << "\nPhysicalPortDescription:";
  *pContentStr << physicalPort1Str;

  *pContentStr << "\n\nSIVRStopIVRStruct:";
  *pContentStr << "\n  physicalPortDescription        :" << physicalPort1Str;
  *pContentStr << "\n  partyOrconfFlag                :" << pStopIvrStruct->partyOrconfFlag;
  *pContentStr << "\n  mediaType                      :" << pStopIvrStruct->mediaType;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceIvrPlayMsgReq(CObjString *pContentStr)
{
  //Partial trace of MSG to be completed
  SIVRPlayMessageStruct tPlayMessageStruct;
  int sizeOfPlayMsgStructWithoutPtr = sizeof(SIVRPlayMessageStruct) - sizeof(SIVRMediaFileParamsStruct*);
  memcpy(&tPlayMessageStruct, m_pMplMcmsProt->getpData(), sizeOfPlayMsgStructWithoutPtr);

  static CLargeString physicalPort1Str;
  physicalPort1Str.Clear();

  TracePhysicalRsrcInfo(&physicalPort1Str, tPlayMessageStruct.physicalPortDescription);

  *pContentStr << "\nCONTENT:";
  *pContentStr << "\nPhysicalPortDescription:";
  *pContentStr << physicalPort1Str;

  *pContentStr << "\n\nSIVRPlayMessageStruct:";
  *pContentStr << "\n  partyOrconfFlag                :";
  if (tPlayMessageStruct.partyOrconfFlag == IVR_PLAY_MSG_TO_PARTY)
    *pContentStr << "msg_to_party";
  else if (tPlayMessageStruct.partyOrconfFlag == IVR_PLAY_MSG_TO_CONF)
    *pContentStr << "msg_to_conf";
  else
    *pContentStr << "unknown=" << tPlayMessageStruct.partyOrconfFlag;

  *pContentStr << "\n  stopPrevOrAppend               :";
  if (tPlayMessageStruct.stopPrevOrAppend == IVR_STOP_PREV_MSG)
    *pContentStr << "stop_prev_msg";
  else if (tPlayMessageStruct.stopPrevOrAppend == IVR_APPEND_MSG)
    *pContentStr << "append_msg";
  else
    *pContentStr << "unknown=" << tPlayMessageStruct.stopPrevOrAppend;

  *pContentStr << "\n  mediaType                      :";
  if (tPlayMessageStruct.mediaType == IVR_MEDIA_TYPE_AUDIO)
    *pContentStr << "audio";
  else if (tPlayMessageStruct.mediaType == IVR_MEDIA_TYPE_VIDEO)
    *pContentStr << "video";
  else if (tPlayMessageStruct.mediaType == IVR_MEDIA_TYPE_AUDIO_AND_VIDEO)
    *pContentStr << "audio_and_video";
  else
    *pContentStr << "unknown=" << tPlayMessageStruct.mediaType;

  *pContentStr << "\n  numOfRepetition                :" << tPlayMessageStruct.numOfRepetition;
  *pContentStr << "\n  startIVRFlag                   :" << tPlayMessageStruct.startIVRFlag;

  if (1 == tPlayMessageStruct.startIVRFlag)
    TraceStartIVRParams(pContentStr, tPlayMessageStruct.startIVR);

  *pContentStr << "\n  videoBitRate                   :" << tPlayMessageStruct.videoBitRate;
  *pContentStr << "\n  isTipMode                      :" << tPlayMessageStruct.isTipMode;
  *pContentStr << "\n  numOfMediaFiles                :" << tPlayMessageStruct.numOfMediaFiles;

  DWORD numOfMediaFiles = tPlayMessageStruct.numOfMediaFiles;

  for (int i = 0; i < (int)numOfMediaFiles; i++)
  {
    const DWORD offset = sizeOfPlayMsgStructWithoutPtr + i * sizeof(SIVRMediaFileParamsStruct);
    const SIVRMediaFileParamsStruct *pMediaFileParamsStruct = (const SIVRMediaFileParamsStruct *)(m_pMplMcmsProt->getpData() + offset);

    *pContentStr << "\n\nMedia File #" << i + 1 << ":";
    TraceMediaFileParams(pContentStr, *pMediaFileParamsStruct);
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceMediaFileParams(CObjString *pContentStr, const SIVRMediaFileParamsStruct &mediaFileParams)
{
  *pContentStr << "\n  actionType                     :";
  if (mediaFileParams.actionType == IVR_ACTION_TYPE_PLAY)
    *pContentStr << "play";
  else if (mediaFileParams.actionType == IVR_ACTION_TYPE_RECORD)
    *pContentStr << "record";
  else if (mediaFileParams.actionType == IVR_ACTION_TYPE_SILENCE)
    *pContentStr << "silence";
  else
    *pContentStr << "unknown=" << mediaFileParams.actionType;

  *pContentStr << "\n  duration                       :" << mediaFileParams.duration;
  *pContentStr << "\n  playMode                       :" << mediaFileParams.playMode;
  *pContentStr << "\n  frequentness                   :" << mediaFileParams.frequentness;
  *pContentStr << "\n  checksum                       :" << mediaFileParams.checksum;
  *pContentStr << "\n  verNum                         :" << mediaFileParams.verNum;
  *pContentStr << "\n  fileNameLength                 :" << mediaFileParams.fileNameLength;
  *pContentStr << "\n  fileName                       :";
  if (0 == mediaFileParams.fileNameLength)
    *pContentStr << "FILE NAME IS EMPTY";
  else
    *pContentStr << mediaFileParams.fileName;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceIvrPlayMusicReq(CObjString *pContentStr)
{
  //Partial trace of MSG to be completed
  const SIVRPlayMusicStruct *pPlayMusicStruct = (const SIVRPlayMusicStruct *)m_pMplMcmsProt->getpData();

  static CLargeString physicalPort1Str;
  physicalPort1Str.Clear();

  TracePhysicalRsrcInfo(&physicalPort1Str, pPlayMusicStruct->physicalPortDescription);

  *pContentStr << "\nCONTENT:";
  *pContentStr << "\nPhysicalPortDescription:";
  *pContentStr << physicalPort1Str;

  *pContentStr << "\n\nSIVRStartIVRStruct:";
  *pContentStr << "\n  partyOrconfFlag                :" << pPlayMusicStruct->partyOrconfFlag;
  *pContentStr << "\n  musicSourceID                  :" << pPlayMusicStruct->musicSourceID;
  *pContentStr << "\n  music Full path name           :" << pPlayMusicStruct->fileName;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceIvrRecordRollCallReq(CObjString *pContentStr)
{
  // Record Roll Call request uses the same struct as Play Message request
  TraceIvrPlayMsgReq(pContentStr);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceIvrRecordRollCallInd(CObjString *pContentStr)
{
	//Partial trace of MSG to be completed
  const SIVRRecordMessageIndStruct *pRecordMsgIndStruct = (const SIVRRecordMessageIndStruct *)m_pMplMcmsProt->getpData();

  *pContentStr << "\nCONTENT:";
  *pContentStr << "\nSIVRRecordMessageIndStruct:";
  *pContentStr << "\n  status                         :" << pRecordMsgIndStruct->status;
  *pContentStr << "\n  recordingLength                :" << pRecordMsgIndStruct->recordingLength;
}
//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceIvrStopRecordRollCallReq(CObjString *pContentStr)
{
  const PHYSICAL_RESOURCE_INFO_S *pPhysicalRsrc = (const PHYSICAL_RESOURCE_INFO_S *)m_pMplMcmsProt->getpData();

  static CLargeString physicalPort1Str;
  physicalPort1Str.Clear();

  TracePhysicalRsrcInfo(&physicalPort1Str, *pPhysicalRsrc);

  *pContentStr << "\nCONTENT:";
  *pContentStr << "\nPhysicalPortDescription:";
  *pContentStr << physicalPort1Str;
}
//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceIvrShowSlideReq(CObjString *pContentStr)
{
  // Show Slide request uses the same struct as Play Message request
  TraceIvrPlayMsgReq(pContentStr);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceIvrStopShowSlideReq(CObjString *pContentStr)
{
  const PHYSICAL_RESOURCE_INFO_S *pPhysicalRsrc = (const PHYSICAL_RESOURCE_INFO_S *)m_pMplMcmsProt->getpData();

  static CLargeString physicalPort1Str;
  physicalPort1Str.Clear();

  TracePhysicalRsrcInfo(&physicalPort1Str, *pPhysicalRsrc);

  *pContentStr << "\nCONTENT:";
  *pContentStr << "\nPhysicalPortDescription:";
  *pContentStr << physicalPort1Str;
}
//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceIvrFastUpdateReq(CObjString *pContentStr)
{
	const PHYSICAL_RESOURCE_INFO_S *pPhysicalRsrc = (const PHYSICAL_RESOURCE_INFO_S *)m_pMplMcmsProt->getpData();

  static CLargeString physicalPort1Str;
  physicalPort1Str.Clear();

  TracePhysicalRsrcInfo(&physicalPort1Str, *pPhysicalRsrc);

  *pContentStr << "\nCONTENT:";
  *pContentStr << "\nPhysicalPortDescription:";
  *pContentStr << physicalPort1Str;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceTbMsgOpenPortReq(CObjString *pContentStr)
{
  BYTE logicRsrcType = m_pMplMcmsProt->getPortDescriptionHeaderLogical_resource_type_1();
  switch (logicRsrcType)
  {
    case eLogical_video_encoder:
    case eLogical_video_encoder_content:
    case eLogical_COP_CIF_encoder:
    case eLogical_COP_4CIF_encoder:
    case eLogical_COP_VSW_encoder:
    case eLogical_COP_PCM_encoder:
    case eLogical_COP_HD720_encoder:
    case eLogical_COP_HD1080_encoder:
    case eLogical_relay_avc_to_svc_video_encoder_1:
    case eLogical_relay_avc_to_svc_video_encoder_2:
    {
      TraceEncoderParams(pContentStr);
      break;
    }
    case eLogical_video_decoder:
    case eLogical_COP_Dynamic_decoder:
    case eLogical_COP_LM_decoder:
    case eLogical_COP_VSW_decoder:
    {
      TraceDecoderParams(pContentStr);
      break;
    }
    default:
    {
      TraceConfMplCreateParty(pContentStr);
      break;
    }
  }
}
//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceTbMsgClosePortReq(CObjString *pContentStr)
{
	TraceConfMplDeleteParty(pContentStr);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceAudioPlayToneReq(CObjString *pContentStr)
{
  BYTE logicRsrcType = m_pMplMcmsProt->getPortDescriptionHeaderLogical_resource_type_1();

  const SPlayToneStruct *tPlayToneStruct = (const SPlayToneStruct *)m_pMplMcmsProt->getpData();

  *pContentStr << "\nCONTENT:";
  *pContentStr << "\nPlayToneStruct:";
  *pContentStr << "\n  NumOfRepetition                :" << tPlayToneStruct->numOfRepetition;
  *pContentStr << "\n  NumOfTones                     :" << tPlayToneStruct->numOfTones;
  *pContentStr << "\n  Tone                           :";
  for (int index = 0; index < (int)tPlayToneStruct->numOfTones; index++)
    *pContentStr << EAudioToneNames[tPlayToneStruct->tone[index].tTone] << ",";

}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceAudioDtmfIndVal(CObjString *pContentStr)
{
  BYTE *dtmf;
  dtmf = (BYTE*)m_pMplMcmsProt->getpData();

  char temp[40];
  sprintf(temp, "%d val", (int)*dtmf);

  char tran[12] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '*', '#'};

  BOOL isHidePsw = NO;
  std::string key_hide = "HIDE_CONFERENCE_PASSWORD";
  CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(key_hide, isHidePsw);

  *pContentStr << "\nCONTENT: val: " << temp << ",   DTMF received: ";
  if (isHidePsw)
    *pContentStr << "*";
  else
    *pContentStr << tran[*dtmf];
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceIpRtpDtmfInputInd(CObjString *pContentStr)
{
  BYTE *dtmf;
  dtmf = (BYTE*)m_pMplMcmsProt->getpData();
  char chDtmf = (char)*dtmf;

  BOOL isHidePsw = NO;
  std::string key_hide = "HIDE_CONFERENCE_PASSWORD";
  CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(key_hide, isHidePsw);

  *pContentStr << "\nCONTENT: DTMF received: ";
  if (isHidePsw)
    *pContentStr << "*";
  else
    *pContentStr << chDtmf;
}
//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceConfMplCreateParty(CObjString *pContentStr)
{
  const TOpenArtReq* pOpenArtReq = (const TOpenArtReq *)m_pMplMcmsProt->getpData();

  *pContentStr << "\nCONTENT:";
  *pContentStr << "\nTOpenArtReq:";
  *pContentStr << "\n  enNetworkType                   :" << NetworkTypeToString(ENetworkType(pOpenArtReq->enNetworkType));
  *pContentStr << "\n  unVideoTxMaxNumberOfBitsPer10ms :" << pOpenArtReq->unVideoTxMaxNumberOfBitsPer10ms;
  *pContentStr << "\n  Media Mode :";
  ::GetMediaModeName(pOpenArtReq->nMediaMode, *pContentStr);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceConfMplDeleteParty(CObjString *pContentStr)
{
  const TCloseArtReq* pCloseArtReq = (const TCloseArtReq *)m_pMplMcmsProt->getpData();

  *pContentStr << "\nCONTENT:";
  *pContentStr << "\nTCloseArtReq:";
  *pContentStr << "\n  bIsNeedToCollectInfoFromArt     :" << pCloseArtReq->bIsNeedToCollectInfoFromArt;
}


//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceEncoderParams(CObjString* pStr, const ENCODER_PARAM_S* pEncoderParams)
{
	const ENCODER_PARAM_S* pEncoder = pEncoderParams ? pEncoderParams : (const ENCODER_PARAM_S*)m_pMplMcmsProt->getpData();

	*pStr << "\nENCODER_PARAM_S:";
	*pStr << "\n  nVideoConfType                 :" << EVideoConfTypeNames[pEncoder->nVideoConfType];
	*pStr << "\n  nVideoEncoderType              :" << (((DWORD)pEncoder->nVideoEncoderType == INVALID) ? "INVALID" : EVideoEncoderTypeNames[pEncoder->nVideoEncoderType]);
	*pStr << "\n  nBitRate                       :" << pEncoder->nBitRate;
	*pStr << "\n  nProtocol                      :" << (((DWORD)pEncoder->nProtocol == INVALID) ? "INVALID" : EVideoProtocolNames[pEncoder->nProtocol]);

	TraceH263_H261VideoParams(pStr, pEncoder->tH263_H261VideoParams);
	TraceH264VideoParams(pStr, pEncoder->tH264VideoParams);

	*pStr << "\n  nSampleAspectRatio             :" << pEncoder->nSampleAspectRatio;

	TraceDecoderDetectedModeParams(pStr, pEncoder->tDecoderDetectedMode);

	*pStr << "\n  nResolutionTableType           :" << pEncoder->nResolutionTableType;
	*pStr << "\n  nParsingMode                   :" << pEncoder->nParsingMode;
	*pStr << "\n  nMTUSize                       :" << pEncoder->nMTUSize;
	*pStr << "\n  nTelePresenceMode              :" << pEncoder->nTelePresenceMode;
	*pStr << "\n  nVideoQualityType              :" << EVideoQualityTypeNames[pEncoder->nVideoQualityType];
	*pStr << "\n  bIsVideoClarityEnabled         :" << pEncoder->bIsVideoClarityEnabled;
	*pStr << "\n  nFpsMode                       :" << pEncoder->nFpsMode;
	*pStr << "\n  nHorizontalCroppingPercentage  :" << pEncoder->tCroppingParams.nHorizontalCroppingPercentage;
	*pStr << "\n  nVerticalCroppingPercentage    :" << pEncoder->tCroppingParams.nVerticalCroppingPercentage;
	*pStr << "\n  bEnableMbRefresh               :" << pEncoder->bEnableMbRefresh;
	*pStr << "\n  nEncoderResolutionRatio        :" << GetResolutionRatioParamString(pEncoder->nEncoderResolutionRatio);
	*pStr << "\n  nMaxSingleTransferFrameSize    :" << pEncoder->nMaxSingleTransferFrameSize;
	*pStr << "\n  bIsTipMode                     :" << pEncoder->bIsTipMode;// TIP
	*pStr << "\n  bIsLinkEncoder                 :" << pEncoder->bIsLinkEncoder;
	*pStr << "\n  bUseIntermediateSDResolution   :" << pEncoder->bUseIntermediateSDResolution;// 448p
	*pStr << "\n  bRtvEnableBFrames              :" << pEncoder->bRtvEnableBFrames;
	*pStr << "\n  nFrThreshold		             :" << pEncoder->nFrThreshold;
	*pStr << "\n  nFontType                      :" << pEncoder->nFontType;
	*pStr << "\n  bIsCallGenerator               :" << pEncoder->tCallGeneratorParams.bIsCallGenerator;
	*pStr << "\n  bFollowSpeaker                 :" << pEncoder->bFollowSpeaker;

	*pStr << "\n\n  tH264SvcVideoParams:";
	TraceH264_SvcParams(pStr, &pEncoder->tH264SvcVideoParams);

	for (size_t i = 0; i < ARRAYSIZE(pEncoder->tMsSvcPacsiParams); ++i)
	{
		*pStr << "\n\n  tMsSvcPacsiParams[" << i << "]:";
		TraceH264_SvcParams(pStr, &pEncoder->tMsSvcPacsiParams[i]);
	}
}

void CMplMcmsProtocolTracer::TraceH264_SvcParams(CObjString* pStr, const H264_SVC_VIDEO_PARAM_S* pH264)
{
	*pStr << "\n    unSsrcID                       :" << pH264->unSsrcID;
	*pStr << "\n    unPrID                         :" << (long)pH264->unPrID;
	*pStr << "\n    nResolutionWidth               :" << pH264->nResolutionWidth;
	*pStr << "\n    nResolutionHeight              :" << pH264->nResolutionHeight;
	*pStr << "\n    unNumberOfTemporalLayers       :" << pH264->unNumberOfTemporalLayers;
	*pStr << "\n    nProfile                       :" << pH264->nProfile;
	*pStr << "\n    unPacketPayloadFormat          :" << pH264->unPacketPayloadFormat;

	*pStr << "\n\n    TemporalLayers Info:";

	for (size_t i = 0; i < pH264->unNumberOfTemporalLayers; ++i)
	{
		*pStr << "\n      nResolutionFrameRate        :";

		if (pH264->atSvcTemporalLayer[i].nResolutionFrameRate < E_VIDEO_FPS_LAST)
			*pStr << EVideoFrameRateNames[pH264->atSvcTemporalLayer[i].nResolutionFrameRate];
		else
			*pStr << pH264->atSvcTemporalLayer[i].nResolutionFrameRate;

		*pStr << "\n      nBitRate                    :" << pH264->atSvcTemporalLayer[i].nBitRate;
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceUpdateEncoderParams(CObjString* pContentStr)
{
	TraceEncoderParams(pContentStr, NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceDecoderParams(CObjString* pContentStr, const DECODER_PARAM_S* pDecoderParams)
{
	const DECODER_PARAM_S* pDecoder = pDecoderParams ? pDecoderParams : (const DECODER_PARAM_S *)m_pMplMcmsProt->getpData();

	*pContentStr << "\nCONTENT: DECODER_PARAM_S:";
	*pContentStr << "\n  nVideoConfType             :" << EVideoConfTypeNames[pDecoder->nVideoConfType];
	*pContentStr << "\n  nVideoDecoderType          :" << EVideoDecoderTypeNames[pDecoder->nVideoDecoderType];
	*pContentStr << "\n  nProtocol                  :" << (((DWORD)pDecoder->nProtocol == INVALID) ? "INVALID" : EVideoProtocolNames[pDecoder->nProtocol]);
	*pContentStr << "\n  nBitRate                   :" << pDecoder->nBitRate;

	TraceH263_H261VideoParams(pContentStr, pDecoder->tH263_H261VideoParams);
	TraceH264VideoParams(pContentStr, pDecoder->tH264VideoParams);

	*pContentStr << "\n  nSampleAspectRatio         :" << pDecoder->nSampleAspectRatio;
	*pContentStr << "\n  bInterSync                 :" << pDecoder->bInterSync;
	*pContentStr << "\n  nParsingMode               :" << pDecoder->nParsingMode;
	*pContentStr << "\n  nDecoderFrameRateLimit     :";

	if ((DWORD)pDecoder->nDecoderFrameRateLimit == INVALID)
		*pContentStr << "INVALID";
	else
		*pContentStr << (APIS32)(pDecoder->nDecoderFrameRateLimit);

	*pContentStr << "\n  nDecoderResolutionRatio    :" << GetResolutionRatioParamString(pDecoder->nDecoderResolutionRatio);
	*pContentStr << "\n  nBackgroundImageId         :" << pDecoder->nBackgroundImageId;
	*pContentStr << "\n  bIsVideoClarityEnabled     :" << pDecoder->bIsVideoClarityEnabled;
	*pContentStr << "\n  bIsAutoBrightnessEnabled   :" << pDecoder->bIsAutoBrightnessEnabled;
	*pContentStr <<" \n  bIsTipMode                 :" << pDecoder->bIsTipMode; //TIP
	*pContentStr <<" \n  bIsCallGenerator           :" << pDecoder->tCallGeneratorParams.bIsCallGenerator;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceUpdateDecoderParams(CObjString* pContentStr)
{
  TraceDecoderParams(pContentStr, NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceOperationPointsSet(CObjString* pContentStr)
{
    VIDEO_OPERATION_POINT_SET_S* pOperationPointsSet = (VIDEO_OPERATION_POINT_SET_S*)m_pMplMcmsProt->getpData();

    *pContentStr <<"\nCONTENT: VIDEO_OPERATION_POINT_SET_S:";
    *pContentStr <<"\n=====================================";
    *pContentStr <<"\n\n operationPointSetId:    " <<  pOperationPointsSet->operationPointSetId;
    *pContentStr <<"\n numberOfOperationPoints:    " <<  pOperationPointsSet->numberOfOperationPoints;

    for (int i = 0;i < pOperationPointsSet->numberOfOperationPoints;i++)
    {
        VIDEO_OPERATION_POINT_S operationPoint = pOperationPointsSet->tVideoOperationPoints[i];
        *pContentStr << "\n[" << i << "]:";
        TraceOperationPoint(pContentStr, operationPoint);
     }
}
//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceOperationPoint(CObjString* pContentStr, const VIDEO_OPERATION_POINT_S &videoOperationPointStruct)
{

	*pContentStr << " layerId:" << videoOperationPointStruct.layerId;
	*pContentStr << " Tid:" << videoOperationPointStruct.Tid;
	*pContentStr << " Did:" << videoOperationPointStruct.Did;
	*pContentStr << " Qid:" << videoOperationPointStruct.Qid;
	*pContentStr << " profile:" << videoOperationPointStruct.profile;
	*pContentStr << " level:" << videoOperationPointStruct.level;
	*pContentStr << " frameWidth:" << videoOperationPointStruct.frameWidth;
	*pContentStr << " frameHeight:" << videoOperationPointStruct.frameHeight;
	*pContentStr << " frameRate:" << videoOperationPointStruct.frameRate;
	*pContentStr << " maxBitRate:" << videoOperationPointStruct.maxBitRate;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void CMplMcmsProtocolTracer::TraceAllocateStatusPerUnitReq(CObjString* pContentStr)
{
//	CSegment seg;
//	seg << m_pMplMcmsProt->getpData();
//	seg.DumpHex();

	ALLOC_STATUS_PER_UNIT_S* pAllocStatusPerUnit = (ALLOC_STATUS_PER_UNIT_S*)m_pMplMcmsProt->getpData();
	*pContentStr <<"\nCONTENT: ALLOC_STATUS_PER_UNIT_S:    board:  " << m_pMplMcmsProt->getPhysicalInfoHeaderBoard_id() << "   unit:  " <<  m_pMplMcmsProt->getPhysicalInfoHeaderUnit_id();
	*pContentStr <<"\n=============================================================";
	for (int i = 0;i < MAX_VIDEO_PORTS_PER_UNIT;i++)
	{
		ALLOC_STATUS_PER_PORT_S allocStatusPerPort = pAllocStatusPerUnit->atPortsStatus[i];
		*pContentStr << "\natPortsStatus[" << i << "]:";
		if (allocStatusPerPort.tPortPhysicalId.connection_id != 0)
		{
			TraceMcmsMplPhysicalRsrcInfo(pContentStr,allocStatusPerPort.tPortPhysicalId);
			if (allocStatusPerPort.tPortPhysicalId.physical_id.resource_type == ePhysical_video_encoder)
				TraceEncoderParams(pContentStr,&allocStatusPerPort.tEncoderParam);
			else if (allocStatusPerPort.tPortPhysicalId.physical_id.resource_type == ePhysical_video_decoder)
				TraceDecoderParams(pContentStr,&allocStatusPerPort.tDecoderParam);
			else
				*pContentStr << "\n error - illegal resource type: " << ResourceTypeToString(allocStatusPerPort.tPortPhysicalId.physical_id.resource_type);

			*pContentStr << "\n portStatus:  " <<  (allocStatusPerPort.nPortStatus? "ePortOpen" : "ePortClose");
		}
		else
		{
			*pContentStr << " FREE (not allocated)";
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceVideoEncIndicationIcon(CObjString* pContentStr, const ICON_ATTR_S& tIcon, iconTypeEnum eIconType, size_t nCell/* = INVALID*/)
{
	if (tIcon.bActive)
	{
		static const char* aIconTypes[MAX_NUM_TYPES_OF_ICON] = {
			"eIconNetworkQuality",
//			"eIconEncryptedConf",
//			"eIconAudioPartiesCount",
//			"eIconVideoPartiesCount",
//			"eIconLocalPartyAppearsInLayout",
//			"eIconMutedParty",
//			"eIconRecording",
		};

		*pContentStr << "\n  ICON_ATTR_S: iconType=" << eIconType << " (" << aIconTypes[eIconType] << ")";

		if (nCell != INVALID)
			*pContentStr << ", cell=" << nCell;
		else
			*pContentStr << ", layout ";

		*pContentStr << ", location=" << tIcon.nLocation << ", index=" << tIcon.nIndexIntoStrip;

		if (tIcon.nIconData)
			*pContentStr << ", data=" << tIcon.nIconData;
	}
}

void CMplMcmsProtocolTracer::TraceVideoEncLayoutIndications(CObjString* pContentStr)
{
	const ICONS_DISPLAY_S* pIconsDisplay = reinterpret_cast<ICONS_DISPLAY_S*>(m_pMplMcmsProt->getpData());

	if (pIconsDisplay)
		TraceVideoEncLayoutIndications(pContentStr, *pIconsDisplay);
}

void CMplMcmsProtocolTracer::TraceVideoEncLayoutIndications(CObjString* pContentStr, const ICONS_DISPLAY_S& tIndications)
{
	*pContentStr << "\n ICONS_DISPLAY_S: [[";

	for (size_t i = 0; i < MAX_NUM_TYPES_OF_ICON; ++i)
	{
		TraceVideoEncIndicationIcon(pContentStr, tIndications.atLayoutIconParams[i], static_cast<iconTypeEnum>(i));

		for (size_t c = 0; c < MAX_NUMBER_OF_CELLS_IN_LAYOUT; ++c)
		{
			TraceVideoEncIndicationIcon(pContentStr, tIndications.atCellIconParams[c][i], static_cast<iconTypeEnum>(i), c);
		}
	}

	*pContentStr << "\n ]]";
}
//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceVideoEncChangeLayoutReq(CObjString* pContentStr)
{
//  const MCMS_CM_CHANGE_LAYOUT_S *pChangeLayoutStruct = (const MCMS_CM_CHANGE_LAYOUT_S *)m_pMplMcmsProt->getpData();

  MCMS_CM_CHANGE_LAYOUT_S changeLayoutStruct;
  int sizeOfChangeLayoutStructWithoutImages = sizeof(MCMS_CM_CHANGE_LAYOUT_S) - sizeof(MCMS_CM_IMAGE_PARAM_S*);
  memcpy(&changeLayoutStruct, m_pMplMcmsProt->getpData(), sizeOfChangeLayoutStructWithoutImages);


  std::ostringstream msg;

  char* separtor = "------+------+-------+-------+----------+------+-------------+------+---------+-----+-------------------+---------------------+---------+---------+----------------+----------------+---------+---------+----------+-----";

  msg << endl << "CONTENT:" << endl;
  msg << separtor << endl;
  msg << " cell | conn | party | board | subBoard | unit | accelerator | port | decoder | art | decoder           | decoder             | decoder | decoder | decoder aspect | decoder aspect | memory  | num of  | disable  | site" << endl;
  msg << " #    | id   | id    | id    | id       | id   | id          | id   | id      | id  | size in layout    | resolution ratio    | width   | height  | ratio width    | ratio height   | address | buffers | cropping | name" << endl;
  msg << separtor << endl;

  WORD numOfImages = CLibsCommonHelperFuncs::GetNumbSubImg(changeLayoutStruct.nLayoutType);
  DWORD offset = 0;
  MCMS_CM_IMAGE_PARAM_S *pImageParamsStruct = NULL;

  for (int i = 0; i < numOfImages; i++)
  {
	offset = sizeOfChangeLayoutStructWithoutImages + i * sizeof(MCMS_CM_IMAGE_PARAM_S);
	pImageParamsStruct = (MCMS_CM_IMAGE_PARAM_S *)(m_pMplMcmsProt->getpData() + offset);

//    const MCMS_CM_IMAGE_PARAM_S& Image = pImageParamsStruct;
    if (pImageParamsStruct->tDecoderPhysicalId.connection_id == INVALID)
      continue;

/*    int initialMemoryBufferAddress;
    if (Image.pucInitialMemoryBufferAddress >= 0xFFFFFFFF-1)
        initialMemoryBufferAddress = -1;
    else
        initialMemoryBufferAddress = Image.pucInitialMemoryBufferAddress;
*/
    char strSiteName[MAX_SITE_NAME_SIZE * 2 + 3];
    UCS2Dump(pImageParamsStruct->siteName, MAX_SITE_NAME_SIZE, strSiteName, MAX_SITE_NAME_SIZE * 2 + 2);

    msg << " " << setw( 4) << right << (int)i << " |"
        << " " << setw( 4) << right << (int)pImageParamsStruct->tDecoderPhysicalId.connection_id << " |"
        << " " << setw( 5) << right << (int)pImageParamsStruct->tDecoderPhysicalId.party_id << " |"
        << " " << setw( 5) << right << (int)pImageParamsStruct->tDecoderPhysicalId.physical_id.physical_unit_params.board_id << " |"
        << " " << setw( 8) << right << (int)pImageParamsStruct->tDecoderPhysicalId.physical_id.physical_unit_params.sub_board_id << " |"
        << " " << setw( 4) << right << (int)pImageParamsStruct->tDecoderPhysicalId.physical_id.physical_unit_params.unit_id << " |"
        << " " << setw(11) << right << (int)pImageParamsStruct->tDecoderPhysicalId.physical_id.accelerator_id << " |"
        << " " << setw( 4) << right << (int)pImageParamsStruct->tDecoderPhysicalId.physical_id.port_id << " |"
        << " " << setw( 7) << right << (int)pImageParamsStruct->tDecoderPhysicalId.party_id << " |"
        << " " << setw( 3) << right << (int)pImageParamsStruct->nArtPartyId << " |"
        << " " << setw(17) <<  left << ERelativeSizeOfImageInLayoutNames[(int) pImageParamsStruct->nDecoderSizeInLayout] << " |"
        << " " << setw(19) <<  left << GetResolutionRatioParamString(pImageParamsStruct->nDecoderResolutionRatio) << " |"
        << " " << setw( 7) << right << pImageParamsStruct->nDecoderDetectedMode.nDecoderDetectedModeWidth << " |"
        << " " << setw( 7) << right << pImageParamsStruct->nDecoderDetectedMode.nDecoderDetectedModeHeight << " |"
        << " " << setw(14) << right << pImageParamsStruct->nDecoderDetectedMode.nDecoderDetectedSampleAspectRatioWidth << " |"
        << " " << setw(14) << right << pImageParamsStruct->nDecoderDetectedMode.nDecoderDetectedSampleAspectRatioHeight << " |"
        << " " << setw( 7) << right << /*initialMemoryBufferAddress*/0xff << " |"
        << " " << setw( 7) << right << /*pImageParamsStruct->nNumberOfBuffers*/0xff << " |"
        << " " << setw( 8) << right << pImageParamsStruct->nThressoldCroppingOnImage << " |"
        << " " <<             left << strSiteName << endl;
//        << " " <<              left << (char*)pImageParamsStruct->siteName << endl;
  }
  msg << separtor;
  *pContentStr << msg.str();

  *pContentStr << "\nMCMS_CM_CHANGE_LAYOUT_S:";
  *pContentStr << "\n  nLayoutType                :" << ELayoutTypeNames[(int)changeLayoutStruct.nLayoutType];
  *pContentStr << "\n  nEncoderResolutionRatio    :" << GetResolutionRatioParamString(changeLayoutStruct.nEncoderResolutionRatio);

  *pContentStr << "\nDECODER_DETECTED_MODE_S:";
  TraceDecoderDetectedModeParams(pContentStr, changeLayoutStruct.nDecoderDetectedMode);

  TraceChangeLayoutAttributes(pContentStr, changeLayoutStruct.tChangeLayoutAttributes);

  TraceVideoEncLayoutIndications(pContentStr, changeLayoutStruct.tIconsDisplay);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceVideoEncChangeLayoutAttributesReq(CObjString* pContentStr)
{
	const CHANGE_LAYOUT_ATTRIBUTES_S *pChangeLayoutAttStruct = (const CHANGE_LAYOUT_ATTRIBUTES_S *)m_pMplMcmsProt->getpData();

	TraceChangeLayoutAttributes(pContentStr, *pChangeLayoutAttStruct);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void CMplMcmsProtocolTracer::TraceImageParams(CObjString* pContentStr, const IMAGE_PARAM_S &imageStruct)
{
	if (imageStruct.tDecoderPhysicalId.connection_id == INVALID)
  {
    *pContentStr << "NULL";
    return;
  }
  static CLargeString physicalIdStr;
  physicalIdStr.Clear();

  TraceMcmsMplPhysicalRsrcInfo(&physicalIdStr, imageStruct.tDecoderPhysicalId);

  *pContentStr << physicalIdStr;
  *pContentStr << "\n  nDecoderPartyId                :" << imageStruct.nDecoderPartyId;
  *pContentStr << "\n  nArtPartyId                    :" << imageStruct.nArtPartyId;
  *pContentStr << "\n  nDecoderSizeInLayout           :" << ERelativeSizeOfImageInLayoutNames[imageStruct.nDecoderSizeInLayout];
  *pContentStr << "\n  nDecoderResolutionRatio        :" << GetResolutionRatioParamString(imageStruct.nDecoderResolutionRatio);
  TraceDecoderDetectedModeParams(pContentStr, imageStruct.nDecoderDetectedMode);
  *pContentStr << "\n  nDecoderCoordinatorInLayout    :" << imageStruct.nDecoder_xX_CoordinatorInLayout;
  *pContentStr << "\n  pucInitialMemoryBufferAddress  :" << imageStruct.pucInitialMemoryBufferAddress;
  *pContentStr << "\n  nNumberOfBuffers               :" << imageStruct.nNumberOfBuffers;
  *pContentStr << "\n  siteName                       :";// << imageStruct.siteName;
  UCS2Dump(imageStruct.siteName, MAX_SITE_NAME_SIZE, pContentStr);
  *pContentStr << "\n  nThressoldCroppingOnImage        :" << imageStruct.nThressoldCroppingOnImage;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceChangeLayoutAttributes(CObjString* pContentStr, const CHANGE_LAYOUT_ATTRIBUTES_S &changeLayoutAttributes)
{
  TraceBorder(pContentStr, changeLayoutAttributes.tBorder);
  TraceSpeaker(pContentStr, changeLayoutAttributes.tSpeaker);
  TraceBackground(pContentStr, changeLayoutAttributes.tBackground);
  TraceSiteNames(pContentStr, changeLayoutAttributes.tSitenames);
  TraceFadeInFadeOut(pContentStr, changeLayoutAttributes.tFadeInFadeOut);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceChangeLayoutAttributes(CObjString* pContentStr, const MCMS_CM_CHANGE_LAYOUT_ATTRIBUTES_S &changeLayoutAttributes)
{
  TraceBorder(pContentStr, changeLayoutAttributes.tBorder);
  TraceSpeaker(pContentStr, changeLayoutAttributes.tSpeaker);
  TraceBackground(pContentStr, changeLayoutAttributes.tBackground);
  TraceSiteNames(pContentStr, changeLayoutAttributes.tSitenames);
  TraceFadeInFadeOut(pContentStr, changeLayoutAttributes.tFadeInFadeOut);
}
//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceBorderEdge(CObjString* pContentStr, int i,const MCMS_CM_BORDER_PARAM_S &borderEdges)
{

	*pContentStr << "\n	Cell "<<i<<" right border:";
		if (borderEdges.ucRight == 1)
		*pContentStr<< "on";
	else
		*pContentStr<< "off";

	*pContentStr << "\n	Cell "<<i<<" left border:";
		if (borderEdges.ucLeft == 1)
		*pContentStr<< "on";
	else
		*pContentStr<< "off";

	*pContentStr << "\n	Cell "<<i<<" up border:";
		if (borderEdges.ucUp == 1)
		*pContentStr<< "on";
	else
		*pContentStr<< "off";

	*pContentStr << "\n	Cell "<<i<<" down border:";
		if (borderEdges.ucDown == 1)
		*pContentStr<< "on";
	else
		*pContentStr<< "off";

}
//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceBorder(CObjString* pContentStr, const BORDERS_PARAM_S &border)
{
  *pContentStr << "\nBORDERS_PARAM_S:";
  TraceVisualAttributes(pContentStr, border.tVisualAttributes);

  //for (int i = 0; i < MAX_NUMBER_OF_CELLS_IN_LAYOUT; i++)
  	//TraceBorderEdge(pContentStr,i,border.tBorderEdges[i]);

}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceBorder(CObjString* pContentStr, const MCMS_CM_BORDERS_PARAM_S &border)
{
  *pContentStr << "\nBORDERS_PARAM_S:";
  TraceVisualAttributes(pContentStr, border.tVisualAttributes);
   //for (int i = 0; i < MAX_NUMBER_OF_CELLS_IN_LAYOUT; i++)
  	//TraceBorderEdge(pContentStr,i,border.tBorderEdges[i]);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceSpeaker(CObjString* pContentStr, const SPEAKER_PARAM_S &speaker)
{
  *pContentStr << "\nSPEAKER_PARAM_S:";
  *pContentStr << "\n  nSpeakerImageID            :" << speaker.nSpeakerImageID;
  TraceVisualAttributes(pContentStr, speaker.tVisualAttributes);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceSpeaker(CObjString* pContentStr, const MCMS_CM_SPEAKER_PARAM_S &speaker)
{
  *pContentStr << "\nSPEAKER_PARAM_S:";
  *pContentStr << "\n  nSpeakerImageID            :" << speaker.nSpeakerImageID;
  TraceVisualAttributes(pContentStr, speaker.tVisualAttributes);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceBackground(CObjString* pContentStr, const BACKGROUND_PARAM_S &background)
{
  *pContentStr << "\nBACKGROUND_PARAM_S:";
  *pContentStr << "\n  nColor                     :" << (DWORD)background.nColor;
  *pContentStr << "\n  nImageID                   :" << background.nImageId;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceSiteNames(CObjString* pContentStr, const SITENAMES_PARAM_S &siteNames)
{
  *pContentStr << "\nSITENAMES_PARAM_S:";
  *pContentStr << "\n  nTextColor                 :" << siteNames.nTextColor;
  *pContentStr << "\n  nBackgroundColor           :" << siteNames.nBackgroundColor;
  *pContentStr << "\n  nTransparency              :" << siteNames.nTransparency;
  *pContentStr << "\n  nShadowWidth               :" << siteNames.nShadowWidth;
  *pContentStr << "\n  nStripHeight               :" << siteNames.nStripHeight;
  *pContentStr << "\n  nSiteNamesVerPosition      :" << siteNames.nSiteNamesVerPosition;
  *pContentStr << "\n  nSiteNamesHorPosition      :" << siteNames.nSiteNamesHorPosition;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceSiteNames(CObjString* pContentStr, const MCMS_CM_SITENAMES_PARAM_S &siteNames)
{
	DWORD temp = 0;
  *pContentStr << "\nSITENAMES_PARAM_S:";
  temp = (DWORD)siteNames.nTextColor;
  *pContentStr << "\n  nTextColor                 :" << temp;//siteNames.nTextColor;
  temp = (DWORD)siteNames.nFontSize;
  *pContentStr << "\n  nFontSize                 :" << temp;//siteNames.nnFontSize;
  temp = (DWORD)siteNames.nBackgroundColor;
  *pContentStr << "\n  nBackgroundColor           :" << temp;//siteNames.nBackgroundColor;
  temp = (DWORD)siteNames.nTransparency;
  *pContentStr << "\n  nTransparency              :" << temp;//siteNames.nTransparency;
  temp = (DWORD)siteNames.nShadowWidth;
  *pContentStr << "\n  nShadowWidth               :" << temp;//siteNames.nShadowWidth;
  temp = (DWORD)siteNames.nStripHeight;
  *pContentStr << "\n  nStripHeight               :" << temp;//siteNames.nStripHeight;
  temp = (DWORD)siteNames.nSiteNamesVerPosition;
  *pContentStr << "\n  nSiteNamesVerPosition      :" << temp;//siteNames.nSiteNamesVerPosition;
  temp = (DWORD)siteNames.nSiteNamesHorPosition;
  *pContentStr << "\n  nSiteNamesHorPosition      :" << temp;//siteNames.nSiteNamesHorPosition;
  temp = (DWORD)siteNames.nSiteNamesLocation;
  *pContentStr << "\n  nSiteNamesLocation      :" << temp;//siteNames.nSiteNamesHorPosition;
}


//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceFadeInFadeOut(CObjString* pContentStr, const FADE_IN_FADE_OUT_PARAM_S &fadeInFadeOut)
{
  *pContentStr << "\nFADE_IN_FADE_OUT_PARAM_S:";
  *pContentStr << "\n  nEffectDurationMsec        :" << fadeInFadeOut.nEffectDurationMsec;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceVisualAttributes(CObjString* pContentStr, const VISUAL_ATTRIBUTES_S  &visualAttributes)
{
  *pContentStr << "\n  nColor                     :" << visualAttributes.nColor;
  *pContentStr << "\n  nThickness                 :" << visualAttributes.nThickness;
  *pContentStr << "\n  nTexture                   :" << visualAttributes.nTexture;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceVisualAttributes(CObjString* pContentStr, const MCMS_CM_VISUAL_ATTRIBUTES_S  &visualAttributes)
{
  *pContentStr << "\n  nColor                     :" << visualAttributes.nColor;
  *pContentStr << "\n  nThickness                 :" << visualAttributes.nThickness;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceMplAckInd(CObjString* pContentStr)
{
  const ACK_IND_S* pAckStruct = (const ACK_IND_S*)m_pMplMcmsProt->getpData();

  static const CProcessBase* process = CProcessBase::GetProcess();
  const std::string& strOpcode = process->GetOpcodeAsString(pAckStruct->ack_base.ack_opcode);
  const std::string& strStatus = process->GetStatusAsString(pAckStruct->ack_base.status);

  *pContentStr <<"\nCONTENT: ACK_IND_S:";
  *pContentStr << "\n  ack_opcode         :" << strOpcode.c_str() << " (" << pAckStruct->ack_base.ack_opcode << ")";
  *pContentStr << "\n  ack_seq_num        :" << pAckStruct->ack_base.ack_seq_num;
  *pContentStr << "\n  status             :" << strStatus.c_str();
  *pContentStr << "\n  reason             :" << pAckStruct->ack_base.reason;
  *pContentStr << "\n  media_type         :" << ChanneltypeToString((kChanneltype)pAckStruct->media_type);
  *pContentStr << "\n  media_direction    :" << CapDirectionToString((cmCapDirection)pAckStruct->media_direction);
  *pContentStr << "\n  channel_handle     :" << pAckStruct->channelHandle;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceDecoderSyncInd(CObjString* pContentStr)
{
  DECODER_SYNC_IND_S DecoderSyncStruct;
  memcpy(&DecoderSyncStruct, m_pMplMcmsProt->getpData(), sizeof(DECODER_SYNC_IND_S));

  static const CProcessBase* process   = CProcessBase::GetProcess();
  const std::string&         strStatus = process->GetStatusAsString(DecoderSyncStruct.nStatus);

  *pContentStr <<"\nCONTENT: DECODER_SYNC_IND_S:";
  TraceDecoderDetectedModeParams(pContentStr, DecoderSyncStruct.tDecoderDetectedMode);
  *pContentStr << "\n  unSsrcID                   :" << DecoderSyncStruct.unSsrcID;
  *pContentStr << "\n  unPrID                     :" << DecoderSyncStruct.unPrID;
  *pContentStr << "\n  status                         :" << strStatus.c_str();
}

//////////////////////////////////////////////////////////////////////////////////////////////
char* CMplMcmsProtocolTracer::GetResolutionRatioParamString(const DWORD& resolutionRatio)
{
  switch (resolutionRatio)
  {
    case RESOLUTION_RATIO_0 : return "RESOLUTION_RATIO_0";
    case RESOLUTION_RATIO_1 : return "RESOLUTION_RATIO_1";
    case RESOLUTION_RATIO_4 : return "RESOLUTION_RATIO_4";
    case RESOLUTION_RATIO_16: return "RESOLUTION_RATIO_16";
    case RESOLUTION_RATIO_64: return "RESOLUTION_RATIO_64";
  }
  return "INVALID";
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceDecoderDetectedModeParams(CObjString* pContentStr,const DECODER_DETECTED_MODE_PARAM_S &decoderDetectedModeParamsStruct)
{
  *pContentStr << "\n  nDecoderDetectedModeWidth  :" << decoderDetectedModeParamsStruct.nDecoderDetectedModeWidth;
  *pContentStr << "\n  nDecoderDetectedModeHeight :" << decoderDetectedModeParamsStruct.nDecoderDetectedModeHeight;
  *pContentStr << "\n  nDecoderAspectRatioWidth   :" << decoderDetectedModeParamsStruct.nDecoderDetectedSampleAspectRatioWidth;
  *pContentStr << "\n  nDecoderAspectRatioHeight  :" << decoderDetectedModeParamsStruct.nDecoderDetectedSampleAspectRatioHeight;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceH264VideoParams(CObjString* pContentStr,const H264_VIDEO_PARAM_S &h264VideoParams)
{
  *pContentStr << "\n  nMBPS                      :" << h264VideoParams.nMBPS << " (" << h264VideoParams.nMBPS*500 << " MB/s)"
               << "\n  nFS                        :" << h264VideoParams.nFS << " (" << h264VideoParams.nFS*256 << " MBs)"
               << "\n  nStaticMB                  :" << h264VideoParams.nStaticMB
               << "\n  nResolutionWidth           :" << h264VideoParams.nResolutionWidth
               << "\n  nResolutionHeight          :" << h264VideoParams.nResolutionHeight
               << "\n  nResolutionFrameRate       :" << EVideoFrameRateNames[h264VideoParams.nResolutionFrameRate]
               << "\n  nProfile                   :" << EVideoProfileTypeNames[h264VideoParams.nProfile]
               << "\n  unPacketPayloadFormat      :" << h264VideoParams.unPacketPayloadFormat
               << "\n  unMaxDPB                   :" << h264VideoParams.unMaxDPB;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceH263_H261VideoParams(CObjString* pStr, const H263_H261_VIDEO_PARAM_S& params)
{
	*pStr << "\n  nQcifFrameRate             :";
	if ((DWORD)params.nQcifFrameRate == INVALID)
		*pStr << INVALID;
	else
		*pStr << EVideoFrameRateNames[params.nQcifFrameRate];

	*pStr << "\n  nCifFrameRate              :";
	if ((DWORD)params.nCifFrameRate == INVALID)
		*pStr << INVALID;
	else
		*pStr << EVideoFrameRateNames[params.nCifFrameRate];

	*pStr << "\n  n4CifFrameRate             :";
	if ((DWORD)params.n4CifFrameRate == INVALID)
		*pStr << INVALID;
	else
		*pStr << EVideoFrameRateNames[params.n4CifFrameRate];

	*pStr << "\n  nVGAFrameRate              :";
	if ((DWORD)params.nVGAFrameRate == INVALID)
		*pStr << INVALID;
	else
		*pStr << EVideoFrameRateNames[params.nVGAFrameRate];

	*pStr << "\n  nSVGAFrameRate             :";
	if ((DWORD)params.nSVGAFrameRate == INVALID)
		*pStr << INVALID;
	else
		*pStr << EVideoFrameRateNames[params.nSVGAFrameRate];

	*pStr << "\n  nXGAFrameRate              :";
	if ((DWORD)params.nXGAFrameRate == INVALID)
		*pStr << INVALID;
	else
		*pStr << EVideoFrameRateNames[params.nXGAFrameRate];

	*pStr << "\n  b263HighBbIntra            :" << (params.b263HighBbIntra ? "Yes" : "No");
	*pStr << "\n  bIs263Plus                 :" << (params.bIs263Plus ? "Yes" : "No");
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceUpdateDecoderResolutionReq(CObjString* pContentStr)
{
	const UPDATE_DECODER_RESOLUTION_S *pUpdateDecoderResolutionReqStruct = (const UPDATE_DECODER_RESOLUTION_S *)m_pMplMcmsProt->getpData();

	*pContentStr << "DECODER_1_16_RESOLUTION (up to QCIF): ";
	if ((DWORD)pUpdateDecoderResolutionReqStruct->nOnOffResolution0 != 0)
		*pContentStr << "On \n";
	else
		*pContentStr << "Off \n";

	*pContentStr << "DECODER_1_4_RESOLUTION (up to CIF): ";
		if (pUpdateDecoderResolutionReqStruct->nOnOffResolution1)
			*pContentStr << "On \n";
		else
			*pContentStr << "Off \n";

	*pContentStr << "DECODER_FULL_RESOLUTION (up to SD): ";
		if (pUpdateDecoderResolutionReqStruct->nOnOffResolution4)
			*pContentStr << "On \n";
		else
			*pContentStr << "Off \n";

}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceNetSetupReq(CObjString* pContentStr)
{
	const NET_SETUP_REQ_S *pNetSetupReqStruct = (const NET_SETUP_REQ_S *)m_pMplMcmsProt->getpData();

	static CLargeString netCommonHeaderBuffer;
	netCommonHeaderBuffer.Clear();
	static CLargeString callingPartyBuffer;
	callingPartyBuffer.Clear();
	static CLargeString calledPartyBuffer;
	calledPartyBuffer.Clear();

	//TraceNetCommonParams(&netCommonHeaderBuffer, pNetSetupReqStruct->net_common_header);//olga
	TraceNetSetupHeader(&netCommonHeaderBuffer, pNetSetupReqStruct->net_setup_header);
	TraceCallingParty(&callingPartyBuffer, pNetSetupReqStruct->calling_party);
	TraceCalledParty(&calledPartyBuffer, pNetSetupReqStruct->called_party);

	*pContentStr << "\nCONTENT: NET_SETUP_REQ_S: \n"
		     << netCommonHeaderBuffer
		     << "\n"
		     << "\n net_spfc                     :"
		     << pNetSetupReqStruct->net_spfc
		     << "\n call_type                    :"
		     << pNetSetupReqStruct->call_type
		     << "\n"
		     << callingPartyBuffer
		     << "\n"
		     << calledPartyBuffer;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceNetSetupInd(CObjString* pContentStr)
{
	const NET_SETUP_IND_S *pNetSetupIndStruct = (const NET_SETUP_IND_S *)m_pMplMcmsProt->getpData();

	static CLargeString netCommonHeaderBuffer;
	netCommonHeaderBuffer.Clear();
	static CLargeString callingPartyBuffer;
	callingPartyBuffer.Clear();
	static CLargeString calledPartyBuffer;
	calledPartyBuffer.Clear();

	TraceNetCommonParams(&netCommonHeaderBuffer, pNetSetupIndStruct->net_common_header);
	TraceCallingParty(&callingPartyBuffer, pNetSetupIndStruct->calling_party);
	TraceCalledParty(&calledPartyBuffer, pNetSetupIndStruct->called_party);

	*pContentStr <<"\nCONTENT: NET_SETUP_IND_S: \n"
				 << netCommonHeaderBuffer
		                 << "\n net_spfc                     :"
				 << pNetSetupIndStruct->net_spfc
				 << "\n call_type                    :"
				 << pNetSetupIndStruct->call_type
				 << "\n"
				 << callingPartyBuffer
				 << "\n"
				 << calledPartyBuffer;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceNetConnectInd(CObjString* pContentStr)
{
	const NET_CONNECT_IND_S *pNetConnectIndStruct = (const NET_CONNECT_IND_S *)m_pMplMcmsProt->getpData();

	static CLargeString netCommonHeaderBuffer;
	netCommonHeaderBuffer.Clear();

	TraceNetCommonParams(&netCommonHeaderBuffer, pNetConnectIndStruct->net_common_header);

	*pContentStr << "\nCONTENT: NET_CONNECT_IND_S: \n"<< netCommonHeaderBuffer;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceNetClearReq(CObjString* pContentStr)
{
	const NET_CLEAR_REQ_S *pNetClearReqStruct = (const NET_CLEAR_REQ_S *)m_pMplMcmsProt->getpData();

	static CLargeString netCommonHeaderBuffer;
	netCommonHeaderBuffer.Clear();
	static CLargeString netCauseBuffer;
	netCauseBuffer.Clear();

	TraceNetCommonParams(&netCommonHeaderBuffer, pNetClearReqStruct->net_common_header);
	TraceNetCause(&netCauseBuffer, pNetClearReqStruct->cause);

	*pContentStr << "\nCONTENT: NET_CLEAR_REQ_S: \n"
				 << netCommonHeaderBuffer
				 << "\n cause                        :"
				 << netCauseBuffer;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceNetClearInd(CObjString* pContentStr)
{
	const NET_CLEAR_IND_S *pNetClearIndStruct = (const NET_CLEAR_IND_S *)m_pMplMcmsProt->getpData();

	static CLargeString netCommonHeaderBuffer;
	netCommonHeaderBuffer.Clear();
	static CLargeString netCauseBuffer;
	netCauseBuffer.Clear();

	TraceNetCommonParams(&netCommonHeaderBuffer, pNetClearIndStruct->net_common_header);
	TraceNetCause(&netCauseBuffer, pNetClearIndStruct->cause);

	*pContentStr << "\nCONTENT: NET_CLEAR_IND_S: \n"
				 << netCommonHeaderBuffer
				 << "\n cause                        :"
				 << netCauseBuffer;
}
//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceNetProgressInd(CObjString* pContentStr)
{
	const NET_PROGRESS_IND_S *pNetProgressIndStruct = (const NET_PROGRESS_IND_S *)m_pMplMcmsProt->getpData();

	static CLargeString netCommonHeaderBuffer;
	netCommonHeaderBuffer.Clear();

	TraceNetCommonParams(&netCommonHeaderBuffer, pNetProgressIndStruct->net_common_header);

	*pContentStr << "\nCONTENT: NET_PROGRESS_IND_S: \n"
				 << netCommonHeaderBuffer
				 << "\n progress_dscrd              :"
                	     << pNetProgressIndStruct->progress_dscr;

}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceNetAlertInd(CObjString* pContentStr)
{
	const NET_ALERT_IND_S *pNetAlertIndStruct = (const NET_ALERT_IND_S *)m_pMplMcmsProt->getpData();

	static CLargeString netCommonHeaderBuffer;
	netCommonHeaderBuffer.Clear();

	TraceNetCommonParams(&netCommonHeaderBuffer, pNetAlertIndStruct->net_common_header);

	*pContentStr << "\nCONTENT: NET_ALERT_IND_S: \n"
				 << netCommonHeaderBuffer;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceNetAlertReq(CObjString* pContentStr)
{
	const NET_ALERT_REQ_S *pNetAlertIndStruct = (const NET_ALERT_REQ_S*)m_pMplMcmsProt->getpData();

	static CLargeString netCommonHeaderBuffer;
	netCommonHeaderBuffer.Clear();

	TraceNetCommonParams(&netCommonHeaderBuffer, pNetAlertIndStruct->net_common_header);

	*pContentStr << "\nCONTENT: NET_ALERT_REQ_S: \n"
				 << netCommonHeaderBuffer;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceNetProceedInd(CObjString* pContentStr)
{
	const NET_PROCEED_IND_S *pNetProceedIndStruct = (const NET_PROCEED_IND_S *)m_pMplMcmsProt->getpData();

	static CLargeString netCommonHeaderBuffer;
	netCommonHeaderBuffer.Clear();

	TraceNetCommonParams(&netCommonHeaderBuffer, pNetProceedIndStruct->net_common_header);

	*pContentStr << "\nCONTENT: NET_PROCEED_IND_S: \n"
				 << netCommonHeaderBuffer;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceNetDisconnectInd(CObjString* pContentStr)
{
	const NET_DISCONNECT_IND_S *pNetDisconnectIndStruct = (const NET_DISCONNECT_IND_S *)m_pMplMcmsProt->getpData();

	static CLargeString netCommonHeaderBuffer;
	netCommonHeaderBuffer.Clear();
	static CLargeString netCauseBuffer;
	netCauseBuffer.Clear();

	TraceNetCommonParams(&netCommonHeaderBuffer, pNetDisconnectIndStruct->net_common_header);
	TraceNetCause(&netCauseBuffer, pNetDisconnectIndStruct->cause);

	*pContentStr << "\nCONTENT: NET_DISCONNECT_IND_S: \n"
				 << netCommonHeaderBuffer
				 << "\n cause                        :"
				 << netCauseBuffer;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceNetCommonParams(CObjString *pContentStr, const NET_COMMON_PARAM_S &netParamStruct)
{
  *pContentStr << "\nNET_COMMON_PARAM_S:";
  *pContentStr << "\n  span id                        :" << netParamStruct.span_id;
  *pContentStr << "\n  net_connection_id              :" << netParamStruct.net_connection_id;
  *pContentStr << "\n  virtual_port_number            :" << netParamStruct.virtual_port_number;
  *pContentStr << "\n  physical_port_number           :" << netParamStruct.physical_port_number;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceNetCause(CObjString *pContentStr, const CAUSE_DATA_S &causeDataStruct)
{
	*pContentStr << "\nCAUSE_DATA_S: \n cause_val              :"
		                       << causeDataStruct.cause_val;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceNetConectReq(CObjString* pContentStr)
{
  const NET_CONNECT_REQ_S * pNetConnectReq = (const NET_CONNECT_REQ_S *)m_pMplMcmsProt->getpData();
  static CLargeString netCommonHeaderBuffer;
  netCommonHeaderBuffer.Clear();

  TraceNetCommonParams(&netCommonHeaderBuffer,pNetConnectReq->net_common_header);

  *pContentStr << "\nCONTENT: NET_CONNECT_REQ_S: \n"
	       << netCommonHeaderBuffer;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceNetDisconnectReq(CObjString* pContentStr)
{
  const NET_DISCONNECT_ACK_REQ_S * pDisconnReq = (const NET_DISCONNECT_ACK_REQ_S *)m_pMplMcmsProt->getpData();
  static CLargeString netCommonHeaderBuffer;
  netCommonHeaderBuffer.Clear();
  static CLargeString netCauseBuffer;
  netCauseBuffer.Clear();

  TraceNetCommonParams(&netCommonHeaderBuffer,pDisconnReq->net_common_header);
  TraceNetCause(&netCauseBuffer, pDisconnReq->cause);

  *pContentStr << "\nCONTENT: NET_DISCONNECT_ACK_REQ_S: \n"
	       << netCommonHeaderBuffer
	       << "\n cause                        :"
	       << netCauseBuffer;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceNetDisconnectAckInd(CObjString* pContentStr)
{
  const NET_DISCONNECT_ACK_IND_S * pDisconnAckInd = (const NET_DISCONNECT_ACK_IND_S *)m_pMplMcmsProt->getpData();
  static CLargeString netCommonHeaderBuffer;
  netCommonHeaderBuffer.Clear();
  static CLargeString netCauseBuffer;
  netCauseBuffer.Clear();

  TraceNetCommonParams(&netCommonHeaderBuffer,pDisconnAckInd->net_common_header);
  TraceNetCause(&netCauseBuffer, pDisconnAckInd->cause);

  *pContentStr << "\nCONTENT: NET_DISCONNECT_ACK_IND_S: \n"
	       << netCommonHeaderBuffer
	       << "\n cause                        :"
	       << netCauseBuffer;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceCallingParty(CObjString *pContentStr, const NET_CALLING_PARTY_S &callingPartyStruct)
{
  *pContentStr << "\nNET_CALLING_PARTY_S:";
  *pContentStr << "\n  num_digits                     :" << callingPartyStruct.num_digits;
  *pContentStr << "\n  num_type                       :" << callingPartyStruct.num_type;
  *pContentStr << "\n  num_plan                       :" << callingPartyStruct.num_plan;
  *pContentStr << "\n  presentation_ind               :" << callingPartyStruct.presentation_ind;
  *pContentStr << "\n  screening_ind                  :" << callingPartyStruct.screening_ind;
  *pContentStr << "\n  digits                         :" << (char*)callingPartyStruct.digits;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceCalledParty(CObjString *pContentStr, const NET_CALLED_PARTY_S &calledPartyStruct)
{
  *pContentStr << "\nCALLED_PARTY_S:";
  *pContentStr << "\n  num_digits                     :" << calledPartyStruct.num_digits;
  *pContentStr << "\n  num_type                       :" << calledPartyStruct.num_type;
  *pContentStr << "\n  num_plan                       :" << calledPartyStruct.num_plan;
  *pContentStr << "\n  digits                         :" << (char*)calledPartyStruct.digits;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceXmlTransportAddress(CObjString* transportAddr, const xmlUnionPropertiesSt &unionProps)
{
	*transportAddr << "XML Address params; Type: " << unionProps.unionType << ", Size: " << unionProps.unionSize << "\n";
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceTransportAddrSt(CObjString* transportAddr, const mcXmlTransportAddress &ipAddress)
{
	TraceXmlTransportAddress(transportAddr, ipAddress.unionProps);

	const mcTransportAddress &IpTransportAddress = ipAddress.transAddr;
	const WORD port = IpTransportAddress.port;

	*transportAddr << "IP Address              : ";
	CLargeString trace;
	int i = 0;
	if (IpTransportAddress.ipVersion == eIpVersion4)
	{
		// Case IpV4
		char szIP[20];
		::SystemDWORDToIpString(IpTransportAddress.addr.v4.ip, szIP);
		*transportAddr << szIP;
		*transportAddr << "::" << port << " (";
		char str[30];
		sprintf(str,"%x",IpTransportAddress.addr.v4.ip);
		*transportAddr << "0x" << str;
	}
	else
	{
		char szIP[64];
		::ipToString(IpTransportAddress,szIP,1);
		*transportAddr << szIP;
		*transportAddr << "::" << port << " (";
	}

	if (IpTransportAddress.ipVersion == eIpVersion4)
		*transportAddr << ", V4, ";
	else
		*transportAddr << "V6, ";

	if (IpTransportAddress.transportType == (DWORD)eTransportTypeTcp)
		*transportAddr << " TCP,";
	else if (IpTransportAddress.transportType == (DWORD)eTransportTypeTls)
		*transportAddr << " TLS,";
	else//eTransportTypeUdp
		*transportAddr << " UDP,";

	if (IpTransportAddress.distribution == (DWORD)eDistributionMulticast)
		*transportAddr << " Multicast)\n";
	else
		*transportAddr << " Unicast)\n";

	// Case IpV6
	if (IpTransportAddress.ipVersion == eIpVersion6)
		*transportAddr << "Scope Id                : " << IPv6ScopeIdToString((enScopeId)IpTransportAddress.addr.v6.scopeId) << "\n";

}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceMoveResourceMFAReq(CObjString *pContentStr)
{
  const MOVE_RESOURCES_REQ_S *pMoveMfaRsrcReq = (const MOVE_RESOURCES_REQ_S *)m_pMplMcmsProt->getpData();

  *pContentStr << "\nCONTENT: MOVE_PARTY_RESOURCE_REQ:";
  *pContentStr << "\n  Conference type                :" << pMoveMfaRsrcReq->moveRsrcParams. confType;
  *pContentStr << "\n  Dest conf ID                   :" << pMoveMfaRsrcReq->moveRsrcParams.newConfId;
  *pContentStr << "\n  Dest conf Audio Sample Rate    :" << pMoveMfaRsrcReq->moveRsrcParams.confAudioSampleRate;
  *pContentStr << "\n  Dest Speaker Change Mode       :" << pMoveMfaRsrcReq->moveRsrcParams.enConfSpeakerChangeMode;
  *pContentStr << "\n  Active resources array: \n";

  static CLargeString trace;
  trace.Clear();
  for (int i = 0; i < NUM_OF_LOGICAL_RESOURCE_TYPES; i++)
  {
    trace << " ";
    trace << (BYTE)pMoveMfaRsrcReq->openLogicalResources[i];
  }
  *pContentStr << trace.GetString() << '\n';
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceSMFatalFailureInd(CObjString *pContentStr, const char* OpcodeString)
{
	const SWITCH_SM_KEEP_ALIVE_S *pSMFatalFailureInd = (const SWITCH_SM_KEEP_ALIVE_S *)m_pMplMcmsProt->getpData();

	*pContentStr << "\nCONTENT: " <<  OpcodeString << ":\n"
	 		<< "1)Name- " 		  <<  (char*)(pSMFatalFailureInd->unSmComp1.sSmCompName) << '\n'
	 		<< "1)SlotId- " 	  <<  pSMFatalFailureInd->unSmComp1.unSlotId    << '\n'
			<< "1)Status- " 	  <<  pSMFatalFailureInd->unSmComp1.unStatus	<< '\n'
			<< "1)Desc- " 		  <<  pSMFatalFailureInd->unSmComp1.unStatusDescriptionBitmask << "\n\n"
			<< "2)Name- " 		  <<  (char*)(pSMFatalFailureInd->unSmComp2.sSmCompName) << '\n'
	 		<< "2)SlotId- " 	  <<  pSMFatalFailureInd->unSmComp2.unSlotId    << '\n'
			<< "2)Status- " 	  <<  pSMFatalFailureInd->unSmComp2.unStatus	<< '\n'
			<< "2)Desc- " 		  <<  pSMFatalFailureInd->unSmComp2.unStatusDescriptionBitmask << "\n\n"
			<< "3)Name- " 		  <<  (char*)(pSMFatalFailureInd->unSmComp3.sSmCompName) << '\n'
	 		<< "3)SlotId- " 	  <<  pSMFatalFailureInd->unSmComp3.unSlotId    << '\n'
			<< "3)Status- " 	  <<  pSMFatalFailureInd->unSmComp3.unStatus	<< '\n'
			<< "3)Desc- " 		  <<  pSMFatalFailureInd->unSmComp3.unStatusDescriptionBitmask << "\n\n";
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::PrintHexNum(CObjString *pContentStr, const char* str, int size)
{
	int i;
	char buffer[10];
	unsigned char lPart, rPart;

	for (i = 0; i < size; i++ )
	{
		lPart = (str[i]>>4) & 0x0F;
		rPart = str[i] & 0x0F;
		if (sprintf(buffer, "0x%x%x ", lPart, rPart) != -1)
			*pContentStr << buffer;
	}
	*pContentStr << "\n";
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceMcmsMplPhysicalRsrcInfo(CObjString *pContentStr, const MCMS_MPL_PHYSICAL_RESOURCE_INFO_S &mcmsMplPhysicalRsrcInfoStruct)
{
  static CLargeString physicalId;
  physicalId.Clear();

  TracePhysicalRsrcInfo(&physicalId, mcmsMplPhysicalRsrcInfoStruct.physical_id);

  *pContentStr << "\n  connection_id                  :" << (int)mcmsMplPhysicalRsrcInfoStruct.connection_id;
  *pContentStr << "\n  party_id                       :" << (int)mcmsMplPhysicalRsrcInfoStruct.party_id;
  *pContentStr << physicalId;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TracePhysicalRsrcInfo(CObjString *pContentStr, const PHYSICAL_RESOURCE_INFO_S &physicalRsrcInfoStruct)
{
  static CLargeString physicalUnitParams;
  physicalUnitParams.Clear();

  TracePhysicalUnitParams(&physicalUnitParams, physicalRsrcInfoStruct.physical_unit_params);

  const char* Resource_type = ::ResourceTypeToString(physicalRsrcInfoStruct.resource_type);

  *pContentStr << physicalUnitParams;
  *pContentStr << "\n  accelerator_id                 :" << physicalRsrcInfoStruct.accelerator_id;
  *pContentStr << "\n  port_id                        :" << physicalRsrcInfoStruct.port_id;
  *pContentStr << "\n  resource_type                  :" << Resource_type;
  *pContentStr << "\n  future_use1                    :" << physicalRsrcInfoStruct.future_use1;
  *pContentStr << "\n  future_use2                    :" << physicalRsrcInfoStruct.future_use2;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TracePhysicalUnitParams(CObjString *pContentStr, const PHYSICAL_UNIT_PARAMS_S &physicalUnitParamsStruct)
{
  *pContentStr << "\n  box_id                         :" << physicalUnitParamsStruct.box_id;
  *pContentStr << "\n  board_id                       :" << physicalUnitParamsStruct.board_id;
  *pContentStr << "\n  sub_board_id                   :" << physicalUnitParamsStruct.sub_board_id;
  *pContentStr << "\n  unit_id                        :" << physicalUnitParamsStruct.unit_id;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceAudioOpenDecoderReq(CObjString* pContentStr)
{
  const TAudioOpenDecoderReq* pAudioOpenDecoderReq = (const TAudioOpenDecoderReq*)m_pMplMcmsProt->getpData();

  *pContentStr << "\nCONTENT: TAudioOpenDecoderReq:";
  *pContentStr << "\n  Network Type                   :" << ENetworkTypeNames[pAudioOpenDecoderReq->enNetworkType];
  *pContentStr << "\n  Audio Connection Status        :" << EAudioConnectionStatusNames[pAudioOpenDecoderReq->enAudioConnectionStatus];
  *pContentStr << "\n  Party Media Type               :" << EPartyMediaTypeNames[pAudioOpenDecoderReq->enPartyMediaType];
  *pContentStr << "\n  Call Direction                 :" << ECallDirectionNames[pAudioOpenDecoderReq->enCallDirection];
  *pContentStr << "\n  Vtx Support                    :" << pAudioOpenDecoderReq->bnVtxSupport;
  *pContentStr << "\n  Number Of Channels             :" << pAudioOpenDecoderReq->nNumberOfChannels;
  *pContentStr << "\n  Conf Sample Rate               :" << ESampleRateNames[pAudioOpenDecoderReq->enConfSampleRate];
  *pContentStr << "\n  Decoder Algorithm              :" << EAudioAlgorithmNames[pAudioOpenDecoderReq->enDecoderAlgorithm];
  *pContentStr << "\n  Decoder Mute                   :" << pAudioOpenDecoderReq->bnDecoderMute;
  *pContentStr << "\n  Decoder Gain                   :" << EAudioGainPresetNames[pAudioOpenDecoderReq->enDecoderGain];
  *pContentStr << "\n  Standalone                     :" << pAudioOpenDecoderReq->bnStandalone;
  *pContentStr << "\n  Error Concealment              :" << pAudioOpenDecoderReq->bnErrorConcealment;
  *pContentStr << "\n  AGC                            :" << pAudioOpenDecoderReq->bnAgc;
  *pContentStr << "\n  Tone Remove                    :" << pAudioOpenDecoderReq->bnToneRemove;
  *pContentStr << "\n  Noise Reduction                :" << pAudioOpenDecoderReq->bnNoiseReduction;
  *pContentStr << "\n  T1 Cpt Detection               :" << pAudioOpenDecoderReq->bnT1CptDetection;
  *pContentStr << "\n  DTMF Detection                 :" << pAudioOpenDecoderReq->bnDtmfDetection;
  *pContentStr << "\n  Noise Detection                :" << pAudioOpenDecoderReq->bnNoiseDetection;
  *pContentStr << "\n  Compress Delay                 :" << pAudioOpenDecoderReq->bnCompressedAudioDelay;
  *pContentStr << "\n  Compress Delay Value           :" << pAudioOpenDecoderReq->nCompressedAudioDelayValue;
  *pContentStr << "\n  Echo Suppression               :" << pAudioOpenDecoderReq->bnEchoSuppression;
  *pContentStr << "\n  Keyboard Suppression           :" << pAudioOpenDecoderReq->bnKeyboardSuppression;
  *pContentStr << "\n  Noise Detection Threshold      :" << ENoiseDetectionThresholdNames[pAudioOpenDecoderReq->enNoiseDetectionThreshold];
  *pContentStr << "\n  Audio PLC                      :" << pAudioOpenDecoderReq->bnAudioNewPLC;
  *pContentStr << "\n  Audio Clarity                  :" << pAudioOpenDecoderReq->bnAudioClarity;
  *pContentStr << "\n  Echo Delay                     :" << pAudioOpenDecoderReq->nEchoDelayMsec << " (msec)";
  *pContentStr << "\n  SSRC                           :" << pAudioOpenDecoderReq->nSSRC;
  *pContentStr << "\n  Speaker Change Mode            :" << pAudioOpenDecoderReq->enConfSpeakerChangeMode;
  *pContentStr << "\n  Selective Mixing               :" << pAudioOpenDecoderReq->enSelectiveMixing;
  *pContentStr << "\n  unDecoderGain                  :" << pAudioOpenDecoderReq->unDecoderGain;
  *pContentStr << "\n  Is Call Generator              :" << pAudioOpenDecoderReq->tCallGeneratorParams.bIsCallGenerator;
  *pContentStr << "\n  bnRelayToMix                   :" << pAudioOpenDecoderReq->bnRelayToMix;
  *pContentStr << "\n  unMsftClient                   :" << EMsftClientTypeNames[pAudioOpenDecoderReq->unMsftClient];
  *pContentStr << "\n  nBitRate                       :" << pAudioOpenDecoderReq->tOpusAudioParams.nBitRate;

}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceAudioOpenEncoderReq(CObjString* pContentStr)
{
  const TAudioOpenEncoderReq *pAudioOpenEncoderReq = (const TAudioOpenEncoderReq *)m_pMplMcmsProt->getpData();

  *pContentStr << "\nCONTENT: TAudioOpenEncoderReq:";
  *pContentStr << "\n  Network Type                   :" << ENetworkTypeNames[pAudioOpenEncoderReq->enNetworkType];
  *pContentStr << "\n  Number Of Channels             :" << pAudioOpenEncoderReq->nNumberOfChannels;
  *pContentStr << "\n  Conf Sample Rate               :" << ESampleRateNames[pAudioOpenEncoderReq->enConfSampleRate];
  *pContentStr << "\n  Encoder Algorithm              :" << EAudioAlgorithmNames[pAudioOpenEncoderReq->enEncoderAlgorithm];
  *pContentStr << "\n  Encoder Mute                   :" << pAudioOpenEncoderReq->bnEncoderMute;
  *pContentStr << "\n  Encoder Gain                   :" << EAudioGainPresetNames[pAudioOpenEncoderReq->enEncoderGain];
  *pContentStr << "\n  Standalone                     :" << pAudioOpenEncoderReq->bnStandalone;
  *pContentStr << "\n  Num of SSRC                    :" << pAudioOpenEncoderReq->stSSRC.numOfSSRC;
  *pContentStr << "\n  SSRC array                     :";
	for(DWORD i = 0; i < pAudioOpenEncoderReq->stSSRC.numOfSSRC; ++i)
      *pContentStr << pAudioOpenEncoderReq->stSSRC.ssrcList[i] << " ";
  *pContentStr << "\n  MixModeGet                     :" << EGetMixMode[pAudioOpenEncoderReq->enEMixModeGet];
  *pContentStr << "\n  IVR SSRC                       :" << pAudioOpenEncoderReq->unIvrSsrc;
  *pContentStr << "\n  IVR CSRC                       :" << pAudioOpenEncoderReq->unIvrCsrc;
	*pContentStr << "\n  Use Speaker SSRC For Tx        :" << (pAudioOpenEncoderReq->bUseSpeakerSsrcForTx ? "TRUE" : "FALSE");
	*pContentStr << "\n  unEncoderGain                  :" << pAudioOpenEncoderReq->unEncoderGain;
	*pContentStr << "\n  Is Call Generator              :" << pAudioOpenEncoderReq->tCallGeneratorParams.bIsCallGenerator;
  *pContentStr << "\n  bnRelayToMix                   :" << pAudioOpenEncoderReq->bnRelayToMix;
  *pContentStr << "\n  nBitRate                       :" << pAudioOpenEncoderReq->tOpusAudioParams.nBitRate;

}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceUpdateAudioAlgorithmReq(CObjString *pContentStr)
{
  const TAudioUpdateAlgorithmReq *pAudioUpdateAlgorithmReq = (const TAudioUpdateAlgorithmReq *)m_pMplMcmsProt->getpData();

	*pContentStr <<
		"\nCONTENT: TAudioUpdateAlgorithmReq:"
		"\n  Audio Algorithm :" << EAudioAlgorithmNames[pAudioUpdateAlgorithmReq->enAudioAlgorithm] <<
		"\n  Codec Gain      :" << pAudioUpdateAlgorithmReq->unCodecGain <<
		"\n  Audio BitRate   :" << pAudioUpdateAlgorithmReq->tOpusAudioParams.nBitRate;

}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceUpdateAudioDecoderReq(CObjString *pContentStr)
{
	const TAudioUpdateDecoderReq *pAudioUpdateDecoderReq = (const TAudioUpdateDecoderReq *)m_pMplMcmsProt->getpData();

	*pContentStr <<	"\nCONTENT: TAudioUpdateDecoderReq:"
				 << "\n  Media Type Update :" << EPartyMediaTypeNames[pAudioUpdateDecoderReq->enPartyMediaType];
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceAudioMuteReq(CObjString *pContentStr)
{
  const TAudioUpdateMuteReq *pAudioUpdateMuteReq = (const TAudioUpdateMuteReq *)m_pMplMcmsProt->getpData();

  *pContentStr << "\nCONTENT: TAudioUpdateMuteReq:"
               << "\n  Audio Channel Type :" << EAudioChannelTypeNames[pAudioUpdateMuteReq->enAudioChannelType]
               << "\n  Mute               :" << pAudioUpdateMuteReq->bnMute;
}

//////////////////////////////////////////////// :"///////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceUpdateAudioGainReq(CObjString *pContentStr)
{
  const TAudioUpdateGainReq *pAudioUpdateGainReq = (const TAudioUpdateGainReq *)m_pMplMcmsProt->getpData();

  *pContentStr << "\nCONTENT: TAudioUpdateGainReq:"
               << "\n  Audio Channel Type :" << EAudioChannelTypeNames[pAudioUpdateGainReq->enAudioChannelType]
               << "\n  Gain               :" << EAudioGainPresetNames[pAudioUpdateGainReq->enGain];
}


//////////////////////////////////////////////// :"///////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceUpdateBitRate(CObjString *pContentStr)
{
   const TAudioUpdateBitRateReq *AudioUpdateBitRate = (const TAudioUpdateBitRateReq *)m_pMplMcmsProt->getpData();

  *pContentStr << "\nCONTENT: TAudioUpdateBitRateReq:"
               << "\n  nBitRate : " << AudioUpdateBitRate->nBitRate;
}

//////////////////////////////////////////////// :"///////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceUpdateUseSpeakerSsrcForTxReq(CObjString *pContentStr)
{
  const TAudioUpdateUseSpeakerSsrcForTxReq *pUseSpeakerSsrcForTx = (const TAudioUpdateUseSpeakerSsrcForTxReq *)m_pMplMcmsProt->getpData();

  *pContentStr << "\nCONTENT: TAudioUpdateUseSpeakerSsrcForTxReq:"
               << "\n  Use Speaker SSRC For Tx :" << (pUseSpeakerSsrcForTx->bUseSpeakerSsrcForTx ? "true" : "false");
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceUpdateNoiseDetectionReq(CObjString *pContentStr)
{
  const TAudioUpdateNoiseDetectionReq *pAudioUpdateNoiseDetectionReq = (const TAudioUpdateNoiseDetectionReq *)m_pMplMcmsProt->getpData();

  *pContentStr << "\nCONTENT: TAudioUpdateNoiseDetectionReq:"
               << "\n  bnNoiseDetection          :" << pAudioUpdateNoiseDetectionReq->bnNoiseDetection
               << "\n  enNoiseDetectionThreshold :"<< ENoiseDetectionThresholdNames[pAudioUpdateNoiseDetectionReq->enNoiseDetectionThreshold];
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceUpdateAudioStandaloneReq(CObjString *pContentStr)
{
	const TAudioUpdateStandaloneReq *pAudioUpdateStandaloneReq = (const TAudioUpdateStandaloneReq *)m_pMplMcmsProt->getpData();

  *pContentStr << "\nCONTENT: TAudioUpdateStandaloneReq:"
               << "\n  Standalone :" << pAudioUpdateStandaloneReq->bnStandalone;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceUpdateAudioConnectionStatus(CObjString *pContentStr)
{
  const TAudioUpdateConnectionStatusReq *pAudioUpdateConnectionStatusReq = (const TAudioUpdateConnectionStatusReq *)m_pMplMcmsProt->getpData();

  *pContentStr << "\nCONTENT: TAudioUpdateConnectionStatusReq:"
               << "\n  Audio Connection Status :" << EAudioConnectionStatusNames[pAudioUpdateConnectionStatusReq->enAudioConnectionStatus];
}
/*void CMplMcmsProtocolTracer::TraceAudioRelayDecReq(CObjString *pContentStr)
{
	const TRtpUpdateRelayReq *pAudioUpdateDecoderReq = (const TRtpUpdateRelayReq *)m_pMplMcmsProt->getpData();
	*pContentStr << "\nCONTENT:TRtpUpdateRelayReq: ";
	*pContentStr << "\n  ChannelType           :"; ::GetChannelTypeName(pAudioUpdateDecoderReq->unChannelType, *pContentStr);
	*pContentStr << "\n  ChannelDirection      :"; ::GetChannelDirectionName(pAudioUpdateDecoderReq->unChannelDirection, *pContentStr);
	*pContentStr << "\n  SSRC array	:";
	for(DWORD i=0; i<pAudioUpdateDecoderReq->nSSRC.numOfSSRC; i++)
	{
		*pContentStr << pAudioUpdateDecoderReq->nSSRC.ssrcList[i] << " ";
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceAudioRelayEncReq(CObjString *pContentStr)
{
	const TRtpUpdateRelayReq *pAudioUpdateEncoderReq = (const TRtpUpdateRelayReq *)m_pMplMcmsProt->getpData();
	*pContentStr << "\nCONTENT:TRtpUpdateRelayReq : ";
	*pContentStr << "\n  ChannelType           :"; ::GetChannelTypeName(pAudioUpdateEncoderReq->unChannelType, *pContentStr);
	*pContentStr << "\n  ChannelDirection      :"; ::GetChannelDirectionName(pAudioUpdateEncoderReq->unChannelDirection, *pContentStr);
	*pContentStr << "\n  SSRC array	:";
	for(DWORD i=0; i<pAudioUpdateEncoderReq->nSSRC.numOfSSRC; i++)
	{
		*pContentStr << pAudioUpdateEncoderReq->nSSRC.ssrcList[i] << " ";
	}
}*/
//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceAudioRelayRequests(CObjString *pContentStr)
{
	const TRtpUpdateRelayReq *pAudioUpdateReq = (const TRtpUpdateRelayReq *)m_pMplMcmsProt->getpData();
	*pContentStr << "\nCONTENT:TRtpUpdateRelayReq : ";
	*pContentStr << "\n  ChannelType           :"; ::GetChannelTypeName(pAudioUpdateReq->unChannelType, *pContentStr);
	*pContentStr << "\n  ChannelDirection      :"; ::GetChannelDirectionName(pAudioUpdateReq->unChannelDirection, *pContentStr);
	*pContentStr << "\n  SSRC array	:";
	for(DWORD i=0; i<pAudioUpdateReq->nSSRC.numOfSSRC; i++)
	{
		*pContentStr << pAudioUpdateReq->nSSRC.ssrcList[i] << " ";
	}
	*pContentStr << "\n  IVR SSRC              :" << pAudioUpdateReq->unIvrSsrc;
	*pContentStr << "\n  IVR CSRC              :" << pAudioUpdateReq->unIvrCsrc;

}
//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceUpdateAGC(CObjString *pContentStr)
{
  const TAudioUpdateAgcReq *pAudioUpdateAgcReq = (const TAudioUpdateAgcReq *)m_pMplMcmsProt->getpData();

  *pContentStr << "\nCONTENT: TAudioUpdateAgcReq:"
               << "  \n bnAgc :" << pAudioUpdateAgcReq->bnAgc;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceOpenConfReq(CObjString *pContentStr)
{
  const TAcOpenConfReq* pAcOpenConfReq = (const TAcOpenConfReq*)m_pMplMcmsProt->getpData();

  *pContentStr << "\nCONTENT: TAcOpenConfReq:";
  *pContentStr << "\n  Video Speaker                  :" << pAcOpenConfReq->bunVideoSpeaker;
  *pContentStr << "\n  Talk Hold Time                 :" << pAcOpenConfReq->unTalkHoldTime;
  *pContentStr << "\n  Audio Speaker                  :" << pAcOpenConfReq->bunAudioSpeaker;
  *pContentStr << "\n  Audio Speaker Window           :" << pAcOpenConfReq->unAudioSpeakerWindow;
  *pContentStr << "\n  Req AudioMix Depth             :" << pAcOpenConfReq->unReqAudioMixDepth;
  *pContentStr << "\n  Speaker Change Mode            :" << pAcOpenConfReq->enConfSpeakerChangeMode;
  *pContentStr << "\n  Selective Mixing               :" << pAcOpenConfReq->bunSelectiveMixing;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceSpeakerInd(CObjString *pContentStr)
{
  const TAcActiveSpeakersInd *pAcActiveSpeakersInd = (const TAcActiveSpeakersInd *)m_pMplMcmsProt->getpData();

  *pContentStr << "\nCONTENT: TAcVideoSpeakerInd:";
  *pContentStr << "\n  Party ID                       :" << pAcActiveSpeakersInd->unVideoPartyID;
  *pContentStr << "\nCONTENT: TAcAudioSpeakerInd:";
  *pContentStr << "\n  Party ID                       :" << pAcActiveSpeakersInd->unAudioPartyID << '\n';
  // for speakers list (feature 237)
  *pContentStr << "\nSpeakers: ";
  for (int i = 0; i < MAX_ACTIVE_SPEAKER_LIST; i++)
	  *pContentStr << "  " << (DWORD)pAcActiveSpeakersInd->unActiveSpeakerList[i];

  *pContentStr << "\n\nCONTENT: TAcAudioSpeakerInd:";
    *pContentStr << "\n  Party Audio MSI              :" << pAcActiveSpeakersInd->unDominantSpeakerMsi << '\n';
}

//////////////////////////////////////////////////////////////////////////////////////////////
bool CMplMcmsProtocolTracer::IsKeepAlive(OPCODE opcode)const
{
  bool isKa = false;

  switch (opcode)
  {
    case CS_KEEP_ALIVE_REQ:
    case CS_KEEP_ALIVE_IND:
    case CM_KEEP_ALIVE_REQ:
    case CM_KEEP_ALIVE_IND:
    case SM_KEEP_ALIVE_REQ:
    case SM_KEEP_ALIVE_IND:
    {
      isKa = true;
      break;
    }

    default:
    {
      isKa = false;
      break;
    }
  }
  return isKa;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceCmCloseUdpPortReq(CObjString *pContentStr, const char* OpcodeString)
{
  const TCloseUdpPortMessageStruct* pStructWithAll = (const TCloseUdpPortMessageStruct*)m_pMplMcmsProt->getpData();

  mcReqCmCloseUdpPort st = pStructWithAll->tCmCloseUdpPort;

  TraceMcmsMplPhysicalRsrcInfo(pContentStr, pStructWithAll->physicalPort);

  *pContentStr << "\nCONTENT: " << OpcodeString;
  *pContentStr << "\n  channelType                    :"; ::GetChannelTypeName(st.channelType, *pContentStr);
  *pContentStr << "\n  channelDirection               :"; ::GetChannelDirectionName(st.channelDirection, *pContentStr);
  *pContentStr << "\n  CmLocalUdpAddressIp.transport  :" << GetTransportTypeName(st.CmLocalUdpAddressIp.transportType);
  *pContentStr << "\n  CmLocalUdpAddressIp.ipVersion  :" << st.CmLocalUdpAddressIp.ipVersion;

  if (st.CmLocalUdpAddressIp.ipVersion == eIpVersion4)
  {
    *pContentStr << " (V4)";
    *pContentStr << "\n  CmLocalUdpAddressIp.addr.v4.ip :";
    char str[30];
    sprintf(str, "%x", st.CmLocalUdpAddressIp.addr.v4.ip);
    *pContentStr << "0x" << str;
  }
  else
  {
    *pContentStr << " (V6)";
    *pContentStr << "\n  CmLocalUdpAddressIp.addr.v6.ip :";
    char str[64];
    memset(str, '\0', 64);
    ::ipToString(st.CmLocalUdpAddressIp, str, 1);
    *pContentStr << str;
  }
  *pContentStr << "\n  CmLocalUdpAddressIp.port       :" << st.CmLocalUdpAddressIp.port;
  *pContentStr << "\n  Local RTCP port                :" << st.LocalRtcpPort;
  *pContentStr << "\n  CmRemoteUdpAddressIp.transport :" << GetTransportTypeName(st.CmRemoteUdpAddressIp.transportType);
  *pContentStr << "\n  CmRemoteUdpAddressIp.ipVersion :" << st.CmRemoteUdpAddressIp.ipVersion;

  if (st.CmRemoteUdpAddressIp.ipVersion == eIpVersion4)
  {
    *pContentStr << " (V4)";
    *pContentStr << "\n  CmRemoteUdpAddressIp.addr.v4.ip :";
    char str[30];
    sprintf(str, "%x", st.CmRemoteUdpAddressIp.addr.v4.ip);
    *pContentStr << "0x" << str;
  }
  else
  {
    *pContentStr << "V6";
    *pContentStr << "\n CmRemoteUdpAddressIp.addr.v6.ip :";
    char str[64];
    memset(str, '\0', 64);
    ::ipToString(st.CmRemoteUdpAddressIp, str, 1);
    *pContentStr << str;
  }
  *pContentStr << "\n  CmRemoteUdpAddressIp.port      :" << st.CmRemoteUdpAddressIp.port;
  *pContentStr << "\n  Remote RTCP port               :" << st.RemoteRtcpPort;
  *pContentStr << "\n  Ice channel RTP id             :" << st.ice_channel_rtp_id;
  *pContentStr << "\n  Ice channel RTCP id            :" << st.ice_channel_rtcp_id;
}

void CMplMcmsProtocolTracer::TraceCmMrmpCloseChannelReq(CObjString *pContentStr, const char* OpcodeString)
{
	PTRACE(eLevelInfoNormal,"CMplMcmsProtocolTracer::TraceCmMrmpCloseChannelReq ");

	const MrmpCloseChannelRequestStruct* pStruct = (const MrmpCloseChannelRequestStruct*)m_pMplMcmsProt->getpData();

	MrmpCloseChannelRequestMessage pUdpSt = pStruct->tMrmpCloseChannelRequestMessage;

    *pContentStr << "\n  allocatedPhysicalResources                    :" << pStruct->m_allocatedPhysicalResources;
	for (unsigned int i = 0; i < pStruct->m_allocatedPhysicalResources; i++)
	    TraceMcmsMplPhysicalRsrcInfo(pContentStr, pStruct->physicalId[i]);

	*pContentStr << "\nCONTENT: " << OpcodeString;
	*pContentStr << "\n  channelHandle                    :" << pUdpSt.channelHandle;

	*pContentStr << "\n  channelType                    :"; ::GetChannelTypeName(pStruct->tMrmpCloseChannelRequestMessage.m_channelType, *pContentStr);
	  *pContentStr << "\n  channelDirection               :"; ::GetChannelDirectionName(pStruct->tMrmpCloseChannelRequestMessage.m_channelDirection, *pContentStr);

	  *pContentStr << "\n  CmLocalUdpAddressIp.transport  :" << GetTransportTypeName(pStruct->tMrmpCloseChannelRequestMessage.m_localAddress.transportType);
	  *pContentStr << "\n  CmLocalUdpAddressIp.ipVersion  :" << pStruct->tMrmpCloseChannelRequestMessage.m_localAddress.ipVersion;

	    if (pStruct->tMrmpCloseChannelRequestMessage.m_localAddress.ipVersion == eIpVersion4)
	    {
	      *pContentStr << " (V4)";
	      *pContentStr << "\n  CmLocalUdpAddressIp.addr.v4.ip :";
	      char str[30];
	      sprintf(str, "%x", pStruct->tMrmpCloseChannelRequestMessage.m_localAddress.addr.v4.ip);
	      *pContentStr << "0x" << str;
	    }
	    else
	    {
	      *pContentStr << " (V6)";
	      *pContentStr << "\n  CmLocalUdpAddressIp.addr.v6.ip :";
	      char str[64];
	      memset(str, '\0', 64);
	      ::ipToString(pStruct->tMrmpCloseChannelRequestMessage.m_localAddress, str, 1);
	      *pContentStr << str;
	    }


	    *pContentStr << "\n  CmLocalUdpAddressIp.port       :" << pStruct->tMrmpCloseChannelRequestMessage.m_localAddress.port;
	    *pContentStr << "\n  Local RTCP port                :" << pStruct->tMrmpCloseChannelRequestMessage.m_localRtcpAddress.port;
	    *pContentStr << "\n  CmRemoteUdpAddressIp.transport :" << GetTransportTypeName(pStruct->tMrmpCloseChannelRequestMessage.m_remoteAddress.transportType);
	    *pContentStr << "\n  CmRemoteUdpAddressIp.ipVersion :" << pStruct->tMrmpCloseChannelRequestMessage.m_remoteAddress.ipVersion;

	    if (pStruct->tMrmpCloseChannelRequestMessage.m_remoteAddress.ipVersion == eIpVersion4)
	    {
	      *pContentStr << " (V4)";
	      *pContentStr << "\n  CmRemoteUdpAddressIp.addr.v4.ip:";
	      char str[30];
	      sprintf(str, "%x", pStruct->tMrmpCloseChannelRequestMessage.m_remoteAddress.addr.v4.ip);
	      *pContentStr << "0x" << str;
	    }
	    else
	    {
	      *pContentStr << "V6";
	      *pContentStr << "\n CmRemoteUdpAddressIp.addr.v6.ip :";
	      char str[64];
	      memset(str, '\0', 64);
	      ::ipToString(pStruct->tMrmpCloseChannelRequestMessage.m_remoteAddress, str, 1);
	      *pContentStr << str;
	    }
	    *pContentStr << "\n  CmRemoteUdpAddressIp.port      :" << pStruct->tMrmpCloseChannelRequestMessage.m_remoteAddress.port;
	    *pContentStr << "\n  Remote RTCP port               :" << pStruct->tMrmpCloseChannelRequestMessage.m_remoteRtcpAddress.port;

	    *pContentStr << "\n  ice_channel_rtp_id				:" << pStruct->tMrmpCloseChannelRequestMessage.ice_channel_rtp_id;
	    *pContentStr << "\n  ice_channel_rtcp_id				:" << pStruct->tMrmpCloseChannelRequestMessage.ice_channel_rtcp_id;
}


void CMplMcmsProtocolTracer::TraceVideoSourcesReq(CObjString *pContentStr,const char* OpcodeString)
{
    const VideoRelaySourcesParams* pVideoRelaySourcesParams = (const VideoRelaySourcesParams*)m_pMplMcmsProt->getpData();
    *pContentStr << "\nCONTENT: VideoRelaySourcesParams:";
    *pContentStr << "\n  unChannelHandle                   :" << pVideoRelaySourcesParams->unChannelHandle;
    *pContentStr << "\n  unSequenseNumber                  :" << pVideoRelaySourcesParams->unSequenseNumber;
    *pContentStr << "\n  unSourceOperationPointSetId       :" << pVideoRelaySourcesParams->unSourceOperationPointSetId;
    *pContentStr << "\n  nNumberOfSourceParams             :" << pVideoRelaySourcesParams->nNumberOfSourceParams;
    int numOfStreams = pVideoRelaySourcesParams->nNumberOfSourceParams;
    for(int i=0;i<numOfStreams;i++)
    {
        *pContentStr << "\n  VideoRealySourceParams["<<i<<"]";
        *pContentStr << "\n\t unChannelHandle: "<< pVideoRelaySourcesParams->tVideoRelaySourceParamsArray[i].unChannelHandle;
        *pContentStr << "\n\t unSyncSource   : "<< pVideoRelaySourcesParams->tVideoRelaySourceParamsArray[i].unSyncSource;
        *pContentStr << "\n\t unLayerId      : "<< pVideoRelaySourcesParams->tVideoRelaySourceParamsArray[i].unLayerId;
        *pContentStr << "\n\t unPipeId       : "<< pVideoRelaySourcesParams->tVideoRelaySourceParamsArray[i].unPipeId;
        *pContentStr << "\n\t bIsSpeaker       : "<< pVideoRelaySourcesParams->tVideoRelaySourceParamsArray[i].bIsSpeaker;
        *pContentStr << "\n\t bTid             : "<< pVideoRelaySourcesParams->tVideoRelaySourceParamsArray[i].unTId;
      }
}


//////////////////////////////////////////////////////////////////////////////////////////////
BYTE CMplMcmsProtocolTracer::IsContentSizeValid()
{
  BYTE isValid = YES;
  OPCODE opcode = m_pMplMcmsProt->getCommonHeaderOpcode();
  CMplMcmsProtocolSizeValidator* pMplMcmsProtSizeValid = new CMplMcmsProtocolSizeValidator();
  DWORD dataSize = 0;
  if (opcode == TB_MSG_OPEN_PORT_REQ)
  {
    BYTE logicRsrcType = m_pMplMcmsProt->getPortDescriptionHeaderLogical_resource_type_1();
    dataSize = pMplMcmsProtSizeValid->GetContentSizeByOpcode(opcode, logicRsrcType);
  }
  else if (opcode == VIDEO_GRAPHICS_SHOW_TEXT_BOX_REQ || opcode == SET_ECS || opcode == REMOTE_ECS)
  {// no check for the VIDEO_GRAPHICS_SHOW_TEXT_BOX_REQ since its side is dynamic ; SET_ECS,REMOTE_ECS also dynamic
    isValid = YES;
    POBJDELETE(pMplMcmsProtSizeValid);
    return isValid;
  }
  else if(opcode == CONF_PARTY_MRMP_VIDEO_SOURCES_REQ)
  {
      // NEED TO CALCULATE
      VideoRelaySourcesParams* pVideoRelaySourcesParams = (VideoRelaySourcesParams*)m_pMplMcmsProt->GetData();
      DWORD numOfStreams = pVideoRelaySourcesParams->nNumberOfSourceParams;
      if( numOfStreams > 1 )
          dataSize = (sizeof(VideoRelaySourcesParams)) + (numOfStreams-1)*(sizeof(VideoRealySourceParams));
      else
          dataSize = sizeof(VideoRelaySourcesParams);
      //PTRACE2INT(eLevelInfoNormal, "CMplMcmsProtocolTracer::IsContentSizeValid : CONF_PARTY_MRMP_VIDEO_SOURCES_REQ  - dataSize = ", dataSize);
      isValid = YES;
  }
  else if (opcode == CONF_PARTY_MRMP_RTCP_FIR_REQ) {
	  MrmpRtcpFirStruct* pMrmpRtcpFirStruct = (MrmpRtcpFirStruct*)m_pMplMcmsProt->GetData();
      DWORD numOfStreams = pMrmpRtcpFirStruct->nNumberOfSyncSources;
      if (numOfStreams > 1)
    	  dataSize = (sizeof(pMrmpRtcpFirStruct->syncSources[0])) + (numOfStreams-1)*(sizeof(pMrmpRtcpFirStruct->syncSources[0]));
      else
    	  dataSize = (sizeof(pMrmpRtcpFirStruct->syncSources[0]));
      //PTRACE2INT(eLevelInfoNormal, "CMplMcmsProtocolTracer::IsContentSizeValid : CONF_PARTY_MRMP_RTCP_FIR_REQ  - dataSize = ", dataSize);
      isValid = YES;
  }
  else if(opcode == CONF_PARTY_MRMP_SCP_STREAM_REQ_IND)
  {
      // NEED TO CALCULATE
	  MrmpScpStreamsRequestStruct* pScpStreamsRequest = (MrmpScpStreamsRequestStruct*)m_pMplMcmsProt->GetData();
      DWORD numOfStreams = pScpStreamsRequest->nNumberOfMediaStream;
      if( numOfStreams > 1 )
//          dataSize = (sizeof(MrmpScpStreamsRequestStruct)) + (numOfStreams-1)*(sizeof(MrmpStreamDesc));
    	  dataSize = (sizeof(MrmpScpStreamsRequestStructBase)) + (numOfStreams)*(sizeof(MrmpStreamDesc));
      else
          dataSize = sizeof(MrmpScpStreamsRequestStruct);
      //PTRACE2INT(eLevelInfoNormal, "CMplMcmsProtocolTracer::IsContentSizeValid : CONF_PARTY_MRMP_SCP_STREAM_REQ_IND  - dataSize = ", dataSize);
      isValid = YES;
  }
  else if( (CONF_PARTY_MRMP_SCP_STREAM_NOTIFICATION_REQ == opcode) || (CONF_PARTY_MRMP_SCP_STREAM_NOTIFICATION_IND == opcode) )
  {
      // NEED TO CALCULATE
	  MrmpScpStreamsNotificationStruct* pScpStreamsNotificationStruct = (MrmpScpStreamsNotificationStruct*)m_pMplMcmsProt->GetData();
      DWORD numOfPipes = pScpStreamsNotificationStruct->nNumberOfScpPipes;
      if( numOfPipes > 1 )
    	  dataSize = (sizeof(MrmpScpStreamsNotificationStructBase)) + (numOfPipes)*(sizeof(MrmpScpPipe));
      else
          dataSize = sizeof(MrmpScpStreamsNotificationStruct);


      if (CONF_PARTY_MRMP_SCP_STREAM_NOTIFICATION_REQ == opcode)
      {
    	  PTRACE2INT(eLevelInfoNormal, "CMplMcmsProtocolTracer::IsContentSizeValid : CONF_PARTY_MRMP_SCP_STREAM_NOTIFICATION_REQ  - dataSize = ", dataSize);
      }
      else
      {
    	  PTRACE2INT(eLevelInfoNormal, "CMplMcmsProtocolTracer::IsContentSizeValid : CONF_PARTY_MRMP_SCP_STREAM_NOTIFICATION_IND  - dataSize = ", dataSize);
      }

      isValid = YES;
  }
  else if(CONF_PARTY_MRMP_SCP_PIPES_MAPPING_NOTIFICATION_REQ == opcode)
  {
      // NEED TO CALCULATE
      MrmpScpPipesMappingNotificationStruct* pScpPipesMappingNotificationStruct = (MrmpScpPipesMappingNotificationStruct*)m_pMplMcmsProt->GetData();
      DWORD numOfPipes = pScpPipesMappingNotificationStruct->nNumberOfScpPipes;
      if( numOfPipes > 1 )
    	  dataSize = (sizeof(MrmpScpStreamsNotificationStructBase)) + (numOfPipes)*(sizeof(MrmpScpPipeMapping));
      else
          dataSize = sizeof(MrmpScpPipesMappingNotificationStruct);

    //PTRACE2INT(eLevelInfoNormal, "CMplMcmsProtocolTracer::IsContentSizeValid : CONF_PARTY_MRMP_SCP_PIPES_MAPPING_NOTIFICATION_REQ  - dataSize = ", dataSize);

      isValid = YES;
  }

  else if( (CONF_PARTY_MRMP_SCP_STREAM_NOTIFICATION_ACK_IND == opcode) || (CONF_PARTY_MRMP_SCP_STREAM_NOTIFICATION_ACK_REQ == opcode) )
  {
	  MrmpScpAckStruct* pScpAckStruct = (MrmpScpAckStruct*)m_pMplMcmsProt->GetData();
	  dataSize = sizeof(MrmpScpAckStruct);

      if (CONF_PARTY_MRMP_SCP_STREAM_NOTIFICATION_ACK_IND == opcode)
      {
    	  //PTRACE2INT(eLevelInfoNormal, "CMplMcmsProtocolTracer::IsContentSizeValid : CONF_PARTY_MRMP_SCP_STREAM_NOTIFICATION_ACK_IND  - dataSize = ", dataSize);
      }
      else
      {
    	  //PTRACE2INT(eLevelInfoNormal, "CMplMcmsProtocolTracer::IsContentSizeValid : CONF_PARTY_MRMP_SCP_STREAM_NOTIFICATION_ACK_REQ  - dataSize = ", dataSize);
      }

      isValid = YES;
  }
  else if(CONF_PARTY_MRMP_PARTY_MONITORING_IND == opcode)
  {
      dataSize = sizeof(TCmPartyMonitoringInd);
      //PTRACE2INT(eLevelInfoNormal, "CMplMcmsProtocolTracer::IsContentSizeValid : CONF_PARTY_MRMP_PARTY_MONITORING_IND  - dataSize = ", dataSize);

      isValid = YES;
  }
  else if(CONF_PARTY_MRMP_PARTY_MONITORING_REQ == opcode)
  {
      dataSize = sizeof(MrmpPartyMonitoringReq);
      //PTRACE2INT(eLevelInfoNormal, "CMplMcmsProtocolTracer::IsContentSizeValid : CONF_PARTY_MRMP_PARTY_MONITORING_REQ  - dataSize = ", dataSize);

      isValid = YES;
  }
  else
  {
      dataSize = pMplMcmsProtSizeValid->GetContentSizeByOpcode(opcode);
  }
  if (dataSize == 0)
  {
    //PTRACE2INT(eLevelInfoNormal, "CMplMcmsProtocolTracer::IsContentSizeValid() not one of the opcodes ", opcode);
  }
  else
  {
    if (m_pMplMcmsProt->ValidateDataSize(dataSize, false) != STATUS_OK)
    {
      //PTRACE(eLevelError, "CMplMcmsProtocolTracer::IsContentSizeValid() ValidateDataSize Not OK " );

      isValid = NO;
    }
  }

  if (isValid == NO)
	  PTRACE2INT(eLevelError, "CMplMcmsProtocolTracer::IsContentSizeValid() ValidateDataSize Not OK, dataSize = ", dataSize);

  POBJDELETE(pMplMcmsProtSizeValid);
  return isValid;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceNetSetupHeader(CObjString* pContentStr, const NET_SETUP_REQ_HEADER_S& netParamStruct)
{
  *pContentStr << "\nNET_SETUP_REQ_HEADER_S:";
  *pContentStr << "\nspans order:";

  for (BYTE i = 0; i < MAX_NUM_SPANS_ORDER; i++)
    *pContentStr << netParamStruct.spans_order[i] << "   ";

  *pContentStr << "\n  net_connection_id              :" << netParamStruct.net_connection_id;
  *pContentStr << "\n  virtual_port_number            :" << netParamStruct.virtual_port_number;
  *pContentStr << "\n  physical_port_number           :" << netParamStruct.physical_port_number;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceMuxInitComm(CObjString* pContentStr)
{
  *pContentStr << "\nCMplMcmsProtocolTracer::TraceMuxInitComm: \n";
  char* pData = m_pMplMcmsProt->getpData();
  const H221_INIT_COMM_S *pInitCommStruct = (const H221_INIT_COMM_S*)pData;
  BYTE offset = sizeof (APIU32);

  std::ostringstream ostr_caps;
  APIU32 len_caps = pInitCommStruct->local_caps.caps_bas.number_of_bytes;
  char* ptr = pData + offset;
  *pContentStr << "\n===============================\n"
	"local capabilities: len = " << len_caps << "\n" << "===============================\n";
  CSegment capSeg;
  capSeg.Put((BYTE*)ptr, len_caps);
  CCDRUtils::DumpCap(capSeg.GetPtr(), capSeg.GetWrtOffset(), ostr_caps);
  *pContentStr << ostr_caps.str().c_str();

  std::ostringstream ostr1;
  BYTE len_commode = *(ptr + len_caps);
  *pContentStr << "\n===============================\n"
	"initial xmit mode : len = " << len_commode << "\n" << "===============================\n";
  ptr = ptr + len_caps + offset;
  CCDRUtils::DumpH221Stream(ostr1, len_commode, (BYTE*)ptr);
  *pContentStr << ostr1.str().c_str();

  BYTE len_h230 = *(ptr + len_commode);
  *pContentStr << "\n===============================\n"
	"initial h230 c&i : len = " << len_h230 << "\n" << "===============================\n";
  ptr = ptr + len_commode + offset;

  std::ostringstream ostr_h230;
  CSegment  h230Seg;
  h230Seg << (DWORD)0; // sim of seg descriptor
  h230Seg.Put((BYTE*)ptr, len_h230);
  CCDRUtils::Dump230Opcode(h230Seg, ostr_h230);
  *pContentStr << ostr_h230.str().c_str();

  *pContentStr << "\n===============================";
  ptr = ptr + len_h230;
  APIU32 channel_width = *ptr;
  *pContentStr << "\nchannel width = " << channel_width;
  ptr = ptr + offset;
  APIU32 restrict_type = *ptr;
  *pContentStr << "\nrestrict type = " << restrict_type;
  ptr = ptr + offset;
  APIU32 additional_flags = *ptr;
  *pContentStr << "\nadditional flags = " << additional_flags;
  *pContentStr << "\n===============================";

  DumpHex(len_caps+len_commode+len_h230+sizeof(APIU32)*6);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceEndInitComm(CObjString* pContentStr)
{
  *pContentStr << "\nCMplMcmsProtocolTracer::TraceEndInitComm: ";
  char* pData = m_pMplMcmsProt->getpData();
  const END_INIT_COMM_S *pEndCommStruct = (const END_INIT_COMM_S*)pData;

  std::ostringstream ostr;
  ostr.setf(ios::left, ios::adjustfield);
  ostr.setf(ios::showbase);
  BYTE offset = sizeof(APIU32);
  APIU32 len_caps = pEndCommStruct->remote_caps.caps_bas.number_of_bytes;
  char* ptr = pData + offset;
  *pContentStr << "\n===============================\n"
    "remote capabilities: len = " << len_caps << "\n" << "===============================\n";
  CSegment capSeg;
  capSeg.Put((BYTE*)ptr, len_caps);
  CCDRUtils::DumpCap(capSeg.GetPtr(), capSeg.GetWrtOffset(), ostr);
  *pContentStr << ostr.str().c_str();

  ptr = ptr + len_caps;
  *pContentStr << "\nrestrict type = " << (WORD)(*ptr) << "\n";

  DumpHex(len_caps + sizeof(APIU32) * 2);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceSetXmitMode(CObjString* pContentStr)
{
  *pContentStr << "\nCMplMcmsProtocolTracer::TraceSetXmitMode: ";
  char* pData = m_pMplMcmsProt->getpData();
  const SET_XMIT_MODE_S* pXmitModeStruct = (const SET_XMIT_MODE_S*)pData;

  std::ostringstream ostr;
  ostr.setf(ios::left, ios::adjustfield);
  ostr.setf(ios::showbase);
  BYTE offset = sizeof(APIU32);
  APIU32 len_commode = pXmitModeStruct->xmit_mode.comm_mode_bas.number_of_bytes;
  char* ptr = pData + offset;
  *pContentStr << "\n===============================\n"
    " xmit mode : len = " << len_commode << "\n" << "===============================\n";
  CCDRUtils::DumpH221Stream(ostr, len_commode, (BYTE*)ptr);
  *pContentStr << ostr.str().c_str();
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceRmtXmitMode(CObjString* pContentStr)
{
  *pContentStr << "\nCMplMcmsProtocolTracer::TraceRmtXmitMode: ";
  char* pData = m_pMplMcmsProt->getpData();
  const REMOTE_XMIT_MODE_S* pXmitModeStruct = (const REMOTE_XMIT_MODE_S*)pData;

  std::ostringstream ostr;
  BYTE offset = sizeof(APIU32);
  APIU32 len_commode = pXmitModeStruct->remote_xmit_mode.comm_mode_bas.number_of_bytes;
  char* ptr = pData + offset;
  *pContentStr << "\n===============================\n"
    " xmit mode : len = " << len_commode << "\n" << "===============================\n";
  CCDRUtils::DumpH221Stream(ostr, len_commode, (BYTE*)ptr);
  *pContentStr << ostr.str().c_str();

  DumpHex(len_commode + sizeof(APIU32));
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceExchangeCap(CObjString* pContentStr)
{
  *pContentStr << "\nCMplMcmsProtocolTracer::TraceExchangeCap: ";
  char* pData = m_pMplMcmsProt->getpData();
  const EXCHANGE_CAPS_S* pExchangeCapStruct = (const EXCHANGE_CAPS_S*)pData;
  std::ostringstream ostr;
  BYTE offset = sizeof(APIU32);
  APIU32 len_caps = pExchangeCapStruct->local_caps.caps_bas.number_of_bytes;
  char* ptr = pData + offset;
  *pContentStr << "\n===============================\n"
    "local capabilities: len = " << len_caps << "\n" << "===============================\n";
  CSegment capSeg;
  capSeg.Put((BYTE*)ptr, len_caps);
  CCDRUtils::DumpCap(capSeg.GetPtr(), capSeg.GetWrtOffset(), ostr);
  *pContentStr << ostr.str().c_str();
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceRemoteBasCap(CObjString* pContentStr)
{
  *pContentStr << "\nCMplMcmsProtocolTracer::TraceRemoteBasCap: ";
  char* pData = m_pMplMcmsProt->getpData();
  const REMOTE_BAS_CAPS_S* pRmtBasCapStruct = (const REMOTE_BAS_CAPS_S*)pData;

  std::ostringstream ostr;
  BYTE offset = sizeof (APIU32);
  APIU32 len_caps = pRmtBasCapStruct->remote_caps.caps_bas.number_of_bytes;
  char* ptr = pData + offset;
  *pContentStr << "\n===============================\n"
	"remote capabilities: len = " << len_caps << "\n" << "===============================\n";
  CSegment capSeg;
  capSeg.Put((BYTE*)ptr, len_caps);
  CCDRUtils::DumpCap(capSeg.GetPtr(), capSeg.GetWrtOffset(), ostr);
  *pContentStr << ostr.str().c_str();

  DumpHex(len_caps+sizeof(APIU32));
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceH230MBE(CObjString* pContentStr)
{
  *pContentStr << "\nCMplMcmsProtocolTracer::TraceH230MBE: ";
  char* pData = m_pMplMcmsProt->getpData();
  const SEND_H_230_S* pH230Struct = (const SEND_H_230_S*)pData;
  std::ostringstream ostr_h230;
  BYTE offset = sizeof(APIU32);

  APIU32 len_h230 = pH230Struct->h230_command.h230_bas.number_of_bytes;
  char* ptr = pData + offset;
  *pContentStr << "\n===============================\n"
    "Mbe Message: msg len = " << len_h230 << "\n===============================\n";

  CSegment h230Seg;
  h230Seg << (DWORD)0; // sim of seg descriptor
  h230Seg.Put((BYTE*)ptr, len_h230);
  CCDRUtils::Dump230Opcode(h230Seg, ostr_h230);
  *pContentStr << ostr_h230.str().c_str();

  DumpHex(len_h230 + sizeof(APIU32));
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceMuxRepeatedH230(CObjString* pContentStr)
{
  char* pData = m_pMplMcmsProt->getpData();
  BAS_CMD_DESC* p = (BAS_CMD_DESC*)pData;
  APIU32 len = p->number_of_bytes;
  BYTE offset = sizeof(APIU32);

  char* ptr = pData + offset;

  *pContentStr << "\nCMplMcmsProtocolTracer::TraceMuxRepeatedH230: " << "\n===============================\n"
    "repeated H230 opcodes: msg len = " << len << "\n===============================\n";
  std::ostringstream ostr;
  CSegment h230Seg;
  h230Seg << (DWORD)0; // sim of seg descriptor
  h230Seg.Put((BYTE*)ptr, len);
  CCDRUtils::Dump230Opcode(h230Seg, ostr);
  *pContentStr << ostr.str().c_str();

  DumpHex(len + sizeof(APIU32));
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceRemoteMframeSync(CObjString* pContentStr)
{
  *pContentStr << "\nCMplMcmsProtocolTracer:::TraceRemoteMframeSync: ";
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceLocalMframeSync(CObjString* pContentStr)
{
  *pContentStr << "\nCMplMcmsProtocolTracer:::TraceLocalMframeSync: ";
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceSmartRecovery(CObjString* pContentStr)
{
  *pContentStr << "\nSMART_RECOVERY_UPDATE_S: ";
  SMART_RECOVERY_UPDATE_S* smartRecStruct = (SMART_RECOVERY_UPDATE_S*)m_pMplMcmsProt->getpData();

  *pContentStr << "\n port id: " << smartRecStruct->unPortNum;
  *pContentStr << "\n dsp num: " << smartRecStruct->unDspNum;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::DumpHex(int data_len)
{
	std::string msg;
  char temp[16];
  memset(temp, '\0', 16);

  DWORD tickNow = SystemGetTickCount().GetIntegerPartForTrace();
  sprintf(temp, "%d", tickNow);
	msg  = "ISDN_SIM_XML (time, opcode, data) : ";
	msg += temp;
	msg += ", ";
  sprintf(temp, "%d", m_pMplMcmsProt->getCommonHeaderOpcode());
	msg += temp;
	msg += ", ";

  //============= Ron - hex trace of messages ===================

  char* data_buff = (char*)m_pMplMcmsProt->getpData();

  DWORD buflen = data_len;// m_pMplMcmsProt->getDataLen() received from mux maximum buffer - caused exception

  for (DWORD byte_index = 0; byte_index < buflen; byte_index++)
  {
    if (0 == byte_index)
      sprintf(temp, "{0x%02x", (unsigned char)(data_buff[byte_index]));
    else if (byte_index == (buflen - 1))
      sprintf(temp, ",0x%02x}", (unsigned char)(data_buff[byte_index]));
    else
      sprintf(temp, ",0x%02x", (unsigned char)(data_buff[byte_index]));

	msg += temp;
  }
	FPTRACE(eLevelInfoNormal, msg.c_str());
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceLprSpecificParams(CObjString* pContentStr, const TLprSpecificParams& _struct)
{
  *pContentStr << "\nLPR Specific params:";
  *pContentStr << "\n  Is LPR Enabled        :" << _struct.bunLprEnabled;
  *pContentStr << "\n  Version ID            :" << _struct.unVersionID;
  *pContentStr << "\n  Min protection period :" << _struct.unMinProtectionPeriod;
  *pContentStr << "\n  Max protection period :" << _struct.unMaxProtectionPeriod;
  *pContentStr << "\n  Max recovery set      :" << _struct.unMaxRecoverySet;
  *pContentStr << "\n  Max recovery packets  :" << _struct.unMaxRecoveryPackets;
  *pContentStr << "\n  Max packet size       :" << _struct.unMaxPacketSize;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceUnitReconfig(CObjString* pContentStr)
{
  *pContentStr << "\nCM_UNIT_RECONFIG_REQ: ";
  UNIT_RECONFIG_S* pReconfigStruct = (UNIT_RECONFIG_S*)m_pMplMcmsProt->getpData();

  *pContentStr << "\n  Board Id: " << pReconfigStruct->boardId;
  *pContentStr << "\n  Unit Id: " << pReconfigStruct->unitId;
  *pContentStr << "\n  Unit Type: " << pReconfigStruct->unitType;
  *pContentStr << "\n  Unit Status: " << pReconfigStruct->unitStatus;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceStartRecordingRequest(CObjString* pContentStr)
{
   *pContentStr << "\nSTART_DEBUG_RECORDING_REQ: ";
   TStartDebugRecordingReq* pStartRecordingData = (TStartDebugRecordingReq*)m_pMplMcmsProt->getpData();

   *pContentStr << "\nphysicalPort.physical_unit_params.box_id :" << pStartRecordingData->physicalPort.physical_unit_params.box_id
   << "\nphysicalPort.physical_unit_params.board_id :" << pStartRecordingData->physicalPort.physical_unit_params.board_id
   << "\nphysicalPort.physical_unit_params.sub_board_id :" << pStartRecordingData->physicalPort.physical_unit_params.sub_board_id
   << "\nphysicalPort.physical_unit_params.unit_id :" << pStartRecordingData->physicalPort.physical_unit_params.unit_id
   << "\nphysicalPort.accelerator_id :" << pStartRecordingData->physicalPort.accelerator_id
   << "\nphysicalPort.port_id :" << pStartRecordingData->physicalPort.port_id
   << "\nphysicalPort.resource_type :" << pStartRecordingData->physicalPort.resource_type
   << "\ntDebugRecordingJunction.unJunction :" << pStartRecordingData->tDebugRecordingJunction.unJunction
   << "\ntDebugRecordingJunction.unRate :" << pStartRecordingData->tDebugRecordingJunction.unRate
   << "\ntDebugRecordingJunction.unStreamId :" << pStartRecordingData->tDebugRecordingJunction.unStreamId
   << "\ntDebugRecordingJunction.unMaxRecFileLen :" << pStartRecordingData->tDebugRecordingJunction.unMaxRecFileLen
   << "\ntDebugRecordingJunction.ucFileName :" << pStartRecordingData->tDebugRecordingJunction.ucFileName
   << "\ntDebugRecordingJunction.unApplicationType :" << pStartRecordingData->tDebugRecordingJunction.unApplicationType;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TracePCMMessage(CObjString* pContentStr)
{
	char* pData = m_pMplMcmsProt->getpData();

	DWORD msg_len = m_pMplMcmsProt->getDataLen();
	char* msgContent = new char[msg_len];
	memset(msgContent,'\0',msg_len);
	strncpy(msgContent,pData,msg_len);
	msgContent[msg_len - 1] = '\0';
	*pContentStr << msgContent;

	delete [] msgContent;

}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceConnectDisconnetPCM(CObjString* pContentStr)
{
  const TB_MSG_CONNECT_PCM_S *pTbConnectStruct = (const TB_MSG_CONNECT_PCM_S *)m_pMplMcmsProt->getpData();

  static CLargeString physicalPort1Str;
  physicalPort1Str.Clear();

  TraceMcmsMplPhysicalRsrcInfo(&physicalPort1Str, pTbConnectStruct->physical_port1);

  *pContentStr << "\nCONTENT: TB_MSG_CONNECT_PCM_S:";
  *pContentStr << "\nphysical_port1:" << physicalPort1Str;
  *pContentStr << "\n  pcm_process_id                 :" << pTbConnectStruct->pcm_process_id;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceVideoGraphicsShowTextBoxRequest(CObjString* pContentStr)
{
  TEXT_BOX_DISPLAY_S* pTextBoxDisplayStruct = (TEXT_BOX_DISPLAY_S*)m_pMplMcmsProt->getpData();

  *pContentStr << "\nTEXT_BOX_DISPLAY_S:";
  *pContentStr << "\nTEXT_BOX_PARAMS_S:";
  *pContentStr << "\n  TextBoxType                    :" << pTextBoxDisplayStruct->tTextBoxParams.nTextBoxType;
  *pContentStr << "\n  NumberOfTextLines              :" << pTextBoxDisplayStruct->tTextBoxParams.nNumberOfTextLines;
  *pContentStr << "\n  PresentationMode               :" << pTextBoxDisplayStruct->tTextBoxParams.nPresentationMode;
  *pContentStr << "\n  Position                       :" << pTextBoxDisplayStruct->tTextBoxParams.nPosition;
  *pContentStr << "\n  DisplaySpeed                   :" << pTextBoxDisplayStruct->tTextBoxParams.nDisplaySpeed;
  *pContentStr << "\n  MessageRepetitionCount         :" << pTextBoxDisplayStruct->tTextBoxParams.nMessageRepetitionCount;

  for (int i = 0; i < pTextBoxDisplayStruct->tTextBoxParams.nNumberOfTextLines; i++)
  {
    *pContentStr << "\nLINE #" << i << ", TEXT_LINE_PARAMS_S:";
    *pContentStr << "\n  TextColor                    :" << pTextBoxDisplayStruct->atTextLineParams[i].nTextColor;
    *pContentStr << "\n  BackgroundColor              :" << pTextBoxDisplayStruct->atTextLineParams[i].nBackgroundColor;
    *pContentStr << "\n  Transparency                 :" << pTextBoxDisplayStruct->atTextLineParams[i].nTransparency;
    *pContentStr << "\n  ShadowWidth                  :" << pTextBoxDisplayStruct->atTextLineParams[i].nShadowWidth;
    *pContentStr << "\n  Alignment                    :" << pTextBoxDisplayStruct->atTextLineParams[i].nAlignment;
    *pContentStr << "\n  FontType                     :" << pTextBoxDisplayStruct->atTextLineParams[i].nFontType;
    *pContentStr << "\n  FontSize                     :" << pTextBoxDisplayStruct->atTextLineParams[i].nFontSize;
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceVideoGraphicsShowIconRequest(CObjString* pContentStr)
{
  ICON_PARAMS_S* pIconStruct = (ICON_PARAMS_S*)m_pMplMcmsProt->getpData();

  *pContentStr << "\nICON_PARAMS_S: \n"

  << "  ICON_PARAMS_S: \n"
  << "IconType  :   " << pIconStruct->nIconType << '\n';
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceSetEcs(CObjString* pContentStr)
{
  *pContentStr << "\nSET_ECS_S: ";
  SET_ECS_S* pSetEcsStruct = (SET_ECS_S*)m_pMplMcmsProt->getpData();

  *pContentStr << "\n p_opcode    : " << GetEcsOpcodeStr(pSetEcsStruct->p_opcode) << " (" << pSetEcsStruct->p_opcode << ")";
  *pContentStr << "\n len         : " << pSetEcsStruct->len;
  if (pSetEcsStruct->len > 0)
  {

    char* ecs_data = (char*)&(pSetEcsStruct->asn1_message);
    BYTE block_header = ecs_data[0];
    TraceEcsBlockType(pContentStr, block_header);

    *pContentStr << "\n asn1_message: ";
    if (PRINT_THE_KEYS)
    {
      if (pSetEcsStruct->len < 1024)
        DumpHex(pContentStr, pSetEcsStruct->len, (char*)&(pSetEcsStruct->asn1_message));
      else
        *pContentStr << "can't dump data len > 1024";
    }
    else
    {
      *pContentStr << DONT_PRINT_THE_KEYS;
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceRemoteEcs(CObjString* pContentStr)
{
  *pContentStr << "\nREMOTE_ECS_SE:";
  REMOTE_ECS_SE* pSetEcsStruct = (REMOTE_ECS_SE*)m_pMplMcmsProt->getpData();

  *pContentStr << "\n  len                            :" << pSetEcsStruct->len;

  if (pSetEcsStruct->len > 0)
  {
    char* ecs_data = (char*)&(pSetEcsStruct->asn1_message);
    BYTE block_header = ecs_data[0];
    TraceEcsBlockType(pContentStr, block_header);

    if (block_header == SE_HEADER_SINGLE_BLOCK || block_header == SE_HEADER_FIRST_BLOCK)
    {
      BYTE identifier = ecs_data[1];
      *pContentStr << "\n  identifier                     :" << GetEcsOpcodeStr(identifier);
    }

    *pContentStr << "\n asn1_message: ";
    if (PRINT_THE_KEYS)
    {
      if (pSetEcsStruct->len < 1024)
        DumpHex(pContentStr, pSetEcsStruct->len, (char*)&(pSetEcsStruct->asn1_message));
      else
        *pContentStr << "can't dump data len > 1024";
    }
    else
      *pContentStr << DONT_PRINT_THE_KEYS;
  }
  else
  {
    *pContentStr << "\n  content                        :no content";
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceEcsBlockType(CObjString* pContentStr,BYTE block_header)
{
  *pContentStr << "\n block_header: ";

  switch (block_header)
  {
    case SE_HEADER_SINGLE_BLOCK      : { *pContentStr << "SE_HEADER_SINGLE_BLOCK";        break; }
    case SE_HEADER_FIRST_BLOCK       : { *pContentStr << "SE_HEADER_FIRST_BLOCK";         break; }
    case SE_HEADER_INTERMEDIATE_BLOCK: { *pContentStr << "SE_HEADER_INTERMEDIATE_BLOCK";  break; }
    case SE_HEADER_LAST_BLOCK        : { *pContentStr << "SE_HEADER_LAST_BLOCK";          break; }
    case BAD_ECS_BLOCK               : { *pContentStr << "BAD_ECS_BLOCK";                 break; }
    default                          : { *pContentStr << "Unknown Block Type";            break; }
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceEncKeysInfoReq(CObjString* pContentStr)
{
  if(PRINT_THE_KEYS){
    *pContentStr << "\nENC_KEYS_INFO_S: ";
    ENC_KEYS_INFO_S* pKeysInfoReqStruct = (ENC_KEYS_INFO_S*)m_pMplMcmsProt->getpData();

    *pContentStr << "\n xmtKey    : ";
    DumpHex(pContentStr,16,(char*)&(pKeysInfoReqStruct->xmtKey));
    *pContentStr << "\n rcvKey    : ";
    DumpHex(pContentStr,16,(char*)&(pKeysInfoReqStruct->rcvKey));
  }else{
    *pContentStr << DONT_PRINT_THE_KEYS;
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////
const char* CMplMcmsProtocolTracer::GetEcsOpcodeStr(DWORD identifier)
{
  switch (identifier)
  {
    case P0_Identifier  : return "P0_Identifier";
    case P1_Identifier  : return "P1_Identifier";
    case P2_Identifier  : return "P2_Identifier";
    case P3_Identifier  : return "P3_Identifier";
    case P4_Identifier  : return "P4_Identifier";
    case P5_Identifier  : return "P5_Identifier";
    case P6_Identifier  : return "P6_Identifier";
    case P8_Identifier  : return "P8_Identifier";
    case P9_Identifier  : return "P9_Identifier";
    case P11_Identifier : return "P11_Identifier";
    case NULL_Identifier: return "NULL_Identifier";
    default             : return "unknown identifier";
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::DumpHex(CObjString* pContentStr,int data_len,char* data_buff)
{
  const WORD msgBufSize = 2048;
  char* msgStr = new char[msgBufSize];
  memset(msgStr, '\0', msgBufSize);
  char temp[16];
  memset(temp, '\0', 16);

  //============= Ron - hex trace of messages ===================

  DWORD buflen = data_len;// m_pMplMcmsProt->getDataLen() received from mux maximum buffer - caused exception

  for (DWORD byte_index = 0; byte_index < buflen; byte_index++)
  {
    if (0 == byte_index)
      sprintf(temp, "{0x%02x", (unsigned char)(data_buff[byte_index]));
    else if (byte_index == (buflen - 1))
      sprintf(temp, ",0x%02x}", (unsigned char)(data_buff[byte_index]));
    else
      sprintf(temp, ",0x%02x", (unsigned char)(data_buff[byte_index]));

    strcat(msgStr, temp);
  }

  *pContentStr << msgStr;
  delete[] msgStr;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceSlotsNumbering(CObjString* pContentStr)
{
  *pContentStr << "\nSLOTS_NUMBERING_CONVERSION_IND:";
  SLOTS_NUMBERING_CONVERSION_TABLE_S* pSlotsNumberingStruct = (SLOTS_NUMBERING_CONVERSION_TABLE_S*)m_pMplMcmsProt->getpData();

  int numOfBoardsInTable = (int)(pSlotsNumberingStruct->numOfBoardsInTable);
  *pContentStr << "\n\nNumber of boards in table: " << numOfBoardsInTable << "\n";

  if (MAX_NUM_OF_BOARDS >= numOfBoardsInTable)
  {
    *pContentStr << "\nBoard Id  Sub Board Id  Display Id";
    *pContentStr << "\n========  ============  ==========";

    for (int i = 0; i < numOfBoardsInTable; i++)
    {
      *pContentStr << "\n   "        << pSlotsNumberingStruct->conversionTable[i].boardId;
      *pContentStr << "            " << pSlotsNumberingStruct->conversionTable[i].subBoardId;
      *pContentStr << "            " << pSlotsNumberingStruct->conversionTable[i].displayBoardId;
    }
  }
  else // illegal numOfBoardsInTable
  {
    *pContentStr << "\nIllegal number of boards (must not be greater than " << MAX_NUM_OF_BOARDS << ")!";
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceAudioUpdateCompressedDelay(CObjString* pContentStr)
{
  TAudioUpdateCompressedAudioDelayReq* pAudioUpdateCompressedAudioDelayReq = (TAudioUpdateCompressedAudioDelayReq*)m_pMplMcmsProt->getpData();

  long ibCompressedAudioDelay = (long)pAudioUpdateCompressedAudioDelayReq->bnCompressedAudioDelay;
  long iCompressedAudioDelayValue = (long)pAudioUpdateCompressedAudioDelayReq->nCompressedAudioDelayValue;

  *pContentStr << "\nAUDIO_UPDATE_COMPRESSED_AUDIO_DELAY_REQ:";
  *pContentStr << "\n  bnCompressedAudioDelay    :" << ibCompressedAudioDelay;
  *pContentStr << "\n  CompressedAudioDelayValue :" << iCompressedAudioDelayValue;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceSdesSpecificParams(CObjString* pContentStr, const sdesCapSt& _struct)
{
  eLogLevel loggerlevel = CProcessBase::GetProcess()->GetMaxLogLevel();
  *pContentStr << "\nSRTP Specific params:";
  *pContentStr << "\n  Is SRTP Enabled       :" << _struct.bIsSrtpInUse;

  if (_struct.bIsSrtpInUse)
  {
	*pContentStr << "\n  cryptoSuite         :" << _struct.cryptoSuite;


		*pContentStr << "\nnumKeyParams   			:	" << _struct.numKeyParams << '\n';
		for(APIU32 i = 0; i < _struct.numKeyParams; i++)
		{
			if(eLevelDebug == loggerlevel && !CSysConfigBase::IsUnderJITCState() )
			*pContentStr <<  "	keySalt					:	" << _struct.keyParamsList[i].keyInfo.keySalt << '\n';

			*pContentStr <<  "	bIsMkiInUse				:	" << _struct.keyParamsList[i].keyInfo.bIsMkiInUse << '\n';
			*pContentStr <<  "	mkiValue				:	" << _struct.keyParamsList[i].keyInfo.mkiValue << '\n';
			*pContentStr <<  "	bIsMkiValueLenInUse		:	" << _struct.keyParamsList[i].keyInfo.bIsMkiValueLenInUse << '\n';
			*pContentStr <<  "	mkiValueLen				:	" << _struct.keyParamsList[i].keyInfo.mkiValueLen << '\n';
			*pContentStr <<  "	bIsLifeTimeInUse		:	" << _struct.keyParamsList[i].keyInfo.bIsLifeTimeInUse << '\n';
			*pContentStr <<  "	lifetime				:	" << _struct.keyParamsList[i].keyInfo.lifetime << '\n';
		}

  }
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceRecoveryUnit(CObjString* pContentStr)
{
  UNIT_RECOVERY_S* pUnitRecStruct = (UNIT_RECOVERY_S*)m_pMplMcmsProt->getpData();
  BYTE box_id = pUnitRecStruct->unit_recover.box_id;
  BYTE unit_id = pUnitRecStruct->unit_recover.unit_id;
  BYTE board_id = pUnitRecStruct->unit_recover.board_id;
  BYTE sub_board_id = pUnitRecStruct->unit_recover.sub_board_id;

  *pContentStr << "CONTENT: UNIT_RECOVERY_S:";
  *pContentStr << "\n  box_id                 :" << box_id;
  *pContentStr << "\n  board_id               :" << board_id;
  *pContentStr << "\n  sub_board_id           :" << sub_board_id;
  *pContentStr << "\n  unit_id                :" << unit_id;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceRecoveryReplaceUnit(CObjString* pContentStr)
{
  RECOVERY_REPLACEMENT_UNIT_S* pUnitRecStruct = (RECOVERY_REPLACEMENT_UNIT_S*)m_pMplMcmsProt->getpData();
  BYTE unit_recover_box_id = pUnitRecStruct->unit_recover.box_id;
  BYTE unit_recover_unit_id = pUnitRecStruct->unit_recover.unit_id;
  BYTE unit_recover_board_id = pUnitRecStruct->unit_recover.board_id;
  BYTE unit_recover_sub_board_id = pUnitRecStruct->unit_recover.sub_board_id;

  BYTE unit_replace_box_id = pUnitRecStruct->unit_replacement.box_id;
  BYTE unit_replace_unit_id = pUnitRecStruct->unit_replacement.unit_id;
  BYTE unit_replace_board_id = pUnitRecStruct->unit_replacement.board_id;
  BYTE unit_replace_sub_board_id = pUnitRecStruct->unit_replacement.sub_board_id;

  *pContentStr << "CONTENT: RECOVERY_REPLACEMENT_UNIT_S:";
  *pContentStr << "\n  status                 :" << pUnitRecStruct->status;
  *pContentStr << "\nunit_recover-------------:";
  *pContentStr << "\n  box_id                 :" << unit_recover_box_id;
  *pContentStr << "\n  board_id               :" << unit_recover_board_id;
  *pContentStr << "\n  sub_board_id           :" << unit_recover_sub_board_id;
  *pContentStr << "\n  unit_id                :" << unit_recover_unit_id;
  *pContentStr << "\nunit_replace-------------:";
  *pContentStr << "\n  box_id                 :" << unit_replace_box_id;
  *pContentStr << "\n  board_id               :" << unit_replace_board_id;
  *pContentStr << "\n  sub_board_id           :" << unit_replace_sub_board_id;
  *pContentStr << "\n  unit_id                :" << unit_replace_unit_id;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TracePortDebugInfoInd(CObjString* pContentStr)
{
  DEBUG_INFO_IND_S* pPortDebugInfoStruct = (DEBUG_INFO_IND_S*)m_pMplMcmsProt->getpData();
  *pContentStr << "CONTENT: DEBUG_INFO_IND_S:";
  *pContentStr << "\ndebug info: \n" << pPortDebugInfoStruct->debugInfo;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceConfPortDebugInfoReq(CObjString* pContentStr)
{
  CONF_DEBUG_INFO_REQ_S* pPortDebugInfoStruct = (CONF_DEBUG_INFO_REQ_S*)m_pMplMcmsProt->getpData();
  *pContentStr << "CONTENT: CONF_DEBUG_INFO_REQ_S:";
  *pContentStr << "\n  ConfId                 :" << pPortDebugInfoStruct->confId;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceCardConfigReq(CObjString* pContentStr)
{
  const CARDS_CONFIG_PARAMS_S *pCardsConfigParamsStruct = (const CARDS_CONFIG_PARAMS_S *)m_pMplMcmsProt->getpData();
  *pContentStr << "\nCONTENT: CARDS_CONFIG_PARAMS_S:";
  *pContentStr << "\n  unSystemConfMode       :" << sSystemConfModes[pCardsConfigParamsStruct->unSystemConfMode];
  *pContentStr << "\n  unFutureUse            :" << pCardsConfigParamsStruct->unFutureUse;
}

void CMplMcmsProtocolTracer::TraceSetLogLevelReq(CObjString* out) const
{
    const LOG_LEVEL_S* prm = (const LOG_LEVEL_S*)m_pMplMcmsProt->getpData();
    *out << "\nCONTENT: LOG_LEVEL_S: "
         << "\n New Log Level: "
         << CTrace::GetTraceLevelNameByValue(prm->log_level)
         << " (" << prm->log_level << ")";
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceTipSystemWideOptionsStruct(CObjString *pContentStr, TipSystemWideOptionsSt &systemWideOptions)
{
  *pContentStr << "\n\nMux parameters"
               << "\nRtp Profile            : " << systemWideOptions.rtpProfile
               << "\nDevice Option          : " << systemWideOptions.deviceOptions
               << "\nConfId                 : " << systemWideOptions.confId;
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceTipAudioMuxStruct(CObjString *pContentStr, TipAudioMuxCntlSt &audioMuxCntl)
{
  *pContentStr << "\nAudio positions - Transmit";
  int i = 0;
  for (i = 0; i < eTipAudioPosLast; i++)
  {
    if (audioMuxCntl.txPositions[i])
      *pContentStr << "\n" << ::GetTipAudioPositionStr((ETipAudioPosition)i);
  }
  *pContentStr << "\nAudio positions - Receive";
  for (i = 0; i < eTipAudioPosLast; i++)
  {
    if (audioMuxCntl.rxPositions[i])
      *pContentStr << "\n" << ::GetTipAudioPositionStr((ETipAudioPosition)i);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceTipVideoMuxStruct(CObjString *pContentStr, TipVideoMuxCntlSt &videoMuxCntl)
{
  *pContentStr << "\nVideo positions - Transmit";
  int i = 0;
  for (i = 0; i < eTipVideoPosLast; i++)
  {
    if (videoMuxCntl.txPositions[i])
      *pContentStr << "\n" << ::GetTipVideoPositionStr((ETipVideoPosition)i);
  }
  *pContentStr << "\nVideo positions - Receive";
  for (i = 0; i < eTipVideoPosLast; i++)
  {
    if (videoMuxCntl.rxPositions[i])
      *pContentStr << "\n" << ::GetTipVideoPositionStr((ETipVideoPosition)i);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceTipAudioOptionsStruct(CObjString *pContentStr, TipAudioOptionsSt &audioOptions, const char* strDirection)
{
  *pContentStr << "\n\nAudio options - " << strDirection
               << "\nDynamic Output         : " << audioOptions.IsAudioDynamicOutput
               << "\nDActivity Metric       : " << audioOptions.IsAudioActivityMetric
               << "\nG722 support           : " << audioOptions.G722Negotiation
               << "\nEKT                    : " << audioOptions.genericOptions.IsEKT;
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceTipVideoOptionsStruct(CObjString *pContentStr, TipVideoOptionsSt &videoOptions, const char* strDirection)
{
  *pContentStr << "\n\nVideo options - " << strDirection
               << "\nVideo Refresh Flag     : " << videoOptions.IsVideoRefreshFlag
               << "\nInband Parameter Sets  : " << videoOptions.IsInbandParameterSets
               << "\nCABAC                  : " << videoOptions.IsCABAC
               << "\nLong term reference pic: " << videoOptions.IsLTRP
               << "\nAux FPS                : " << ::GetTipAuxFPSStr((ETipAuxFPS)videoOptions.AuxVideoFPS) << " (" << videoOptions.AuxVideoFPS << ")"
               << "\nGradual decoder refresh: " << videoOptions.IsGDR
               << "\nHigh Profile           : " << videoOptions.IsHighProfile
               << "\nUnrestricted XGA 5FPS  : " << videoOptions.IsUnrestrictedMediaXGA5or1
               << "\nUnrestricted XGA 30FPS : " << videoOptions.IsUnrestrictedMediaXGA30
               << "\nUnrestricted 720p      : " << videoOptions.IsUnrestrictedMedia720p
               << "\nUnrestricted 1080p     : " << videoOptions.IsUnrestrictedMedia1080p
               << "\nEKT                    : " << videoOptions.genericOptions.IsEKT;
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceTipNegotiationStruct(CObjString *pContentStr, TipNegotiationSt &negotiationSt)
{
  TraceTipSystemWideOptionsStruct(pContentStr, negotiationSt.systemwideOptions);

  TraceTipAudioMuxStruct(pContentStr, negotiationSt.audioMuxCntl);
  TraceTipVideoMuxStruct(pContentStr, negotiationSt.videoMuxCntl);

  TraceTipAudioOptionsStruct(pContentStr, negotiationSt.audioTxOptions, "Transmit");
  TraceTipAudioOptionsStruct(pContentStr, negotiationSt.audioRxOptions, "Receive");

  TraceTipVideoOptionsStruct(pContentStr, negotiationSt.videoTxOptions, "Transmit");
  TraceTipVideoOptionsStruct(pContentStr, negotiationSt.videoRxOptions, "Receive");
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceTipStartNegotiationReq(CObjString *pContentStr)
{
  mcTipStartNegotiationReq *pStruct = (mcTipStartNegotiationReq*)m_pMplMcmsProt->getpData();

  *pContentStr << "\nUnit Id                : " << pStruct->unitId
               << "\nPort Id                : " << pStruct->portId
               << "\nIs Prefer TIP          : " << pStruct->isPreferTip; //TIP call from Polycom EPs feature
  TraceTipNegotiationStruct(pContentStr, pStruct->negotiationSt);
}
////////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceTipNegotiationResultInd(CObjString *pContentStr)
{
  mcTipNegotiationResultInd *pStruct = (mcTipNegotiationResultInd*)m_pMplMcmsProt->getpData();
  *pContentStr << "\nStatus                 : " << pStruct->status;
  *pContentStr << "\nDoReinvite              : " << pStruct->doVideoReInvite;
  TraceTipNegotiationStruct(pContentStr, pStruct->negotiationSt);
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceTipEndNegotiationReq(CObjString *pContentStr)
{
  mcTipEndNegotiationReq *pStruct = (mcTipEndNegotiationReq*)m_pMplMcmsProt->getpData();
  *pContentStr << "\nStatus                 : " << pStruct->status
               << "\nUnit Id                : " << pStruct->unitId
               << "\nPort Id                : " << pStruct->portId;
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceTipUpdateCloseCall(CObjString *pContentStr)
{
  mcTipMsgInfoReq* pStruct = (mcTipMsgInfoReq*)m_pMplMcmsProt->getpData();

  *pContentStr << "\nCenter TIP Channel Descr : "
               << "\n Is Active	: " << pStruct->centerTipChannelPartyId.bIsActive
               << "\n Party id		: " << pStruct->centerTipChannelPartyId.unPartyId
               << "\n Dsp Num		: " << pStruct->centerTipChannelPartyId.unDspNum
               << "\n Port Num		: " << pStruct->centerTipChannelPartyId.unPortNum;

  *pContentStr << "\nLeft TIP Channel Descr : "
               << "\n Is Active	: " << pStruct->leftTipChannelDescr.bIsActive
               << "\n Party id		: " << pStruct->leftTipChannelDescr.unPartyId
               << "\n Dsp Num		: " << pStruct->leftTipChannelDescr.unDspNum
               << "\n Port Num		: " << pStruct->leftTipChannelDescr.unPortNum;

  *pContentStr << "\nRight TIP Channel Descr : "
               << "\n Is Active	: " << pStruct->rightTipChannelDescr.bIsActive
               << "\n Party id		: " << pStruct->rightTipChannelDescr.unPartyId
               << "\n Dsp Num		: " << pStruct->rightTipChannelDescr.unDspNum
               << "\n Port Num		: " << pStruct->rightTipChannelDescr.unPortNum;

  *pContentStr << "\nAux TIP Channel Descr : "
               << "\n Is Active	: " << pStruct->auxTipChannelDescr.bIsActive
               << "\n Party id		: " << pStruct->auxTipChannelDescr.unPartyId
               << "\n Dsp Num		: " << pStruct->auxTipChannelDescr.unDspNum
               << "\n Port Num		: " << pStruct->auxTipChannelDescr.unPortNum;
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceTipContentMsgStruct(CObjString *pContentStr, TipContentMsgSt &ContentMsgSt)
{
  *pContentStr << "\nSub opcode             : " << ::GetAuxCntlSubopcodeStr((EAuxCntlSubOpcode)ContentMsgSt.subOpcode);
  *pContentStr << "\nPositions:";
  int i = 0;
  for (i = 0; i < eTipAuxPositionLast; i++)
  {
    if (ContentMsgSt.auxPositions[i])
      *pContentStr << "\n" << ::GetTipAuxPositionStr((ETipAuxPosition)i);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceTipContentMsgReq(CObjString *pContentStr)
{
  mcTipContentMsgReq *pStruct = (mcTipContentMsgReq*)m_pMplMcmsProt->getpData();
  *pContentStr << "\nUnit Id                : " << pStruct->unitId
               << "\nPort Id                : " << pStruct->portId;
  TraceTipContentMsgStruct(pContentStr, pStruct->ContentMsgSt);
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceTipContentMsgInd(CObjString *pContentStr)
{
  mcTipContentMsgInd *pStruct = (mcTipContentMsgInd*)m_pMplMcmsProt->getpData();
  TraceTipContentMsgStruct(pContentStr, pStruct->ContentMsgSt);
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TracePartyMonitoringReq(CObjString *pContentStr)
{
  TPartyMonitoringReq* pStruct = (TPartyMonitoringReq*)m_pMplMcmsProt->getpData();

  *pContentStr << "\nCONTENT: H323_RTP_PARTY_MONITORING_REQ";
  *pContentStr << "\nulTipChannelPartyID	: " << pStruct->ulTipChannelPartyID;
}

////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////
//for CDR_MCCF:
void CMplMcmsProtocolTracer::TracePartyVideoChannelsStatisticsReq(CObjString *pContentStr)
{
  TPartyMonitoringReq* pStruct = (TPartyMonitoringReq*)m_pMplMcmsProt->getpData();
  *pContentStr << "\nCONTENT: \nulTipChannelPartyID : " << pStruct->ulTipChannelPartyID;
}

void CMplMcmsProtocolTracer::TracePartyMrmpPartyMonitoringReq(CObjString *pContentStr)
{
	MrmpPartyMonitoringReq* pStruct = (MrmpPartyMonitoringReq*)m_pMplMcmsProt->getpData();

  *pContentStr << "\nCONTENT:   CONF_PARTY_MRMP_PARTY_MONITORING_REQ";
  *pContentStr << "\nunPartyID: " << pStruct->unPartyID;
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::UCS2Dump(const APIS8* pUCS2Buff, WORD iLen, CObjString* psResult)
{
	for (WORD i=0; i < iLen; ++i)
	{
		(*psResult) << (BYTE)(pUCS2Buff[i]) << "|";
		if (i > 0 && i%2 == 1 && pUCS2Buff[i] == 0 && pUCS2Buff[i-1] == 0)
			break;
	}
}

void CMplMcmsProtocolTracer::UCS2Dump(const APIS8* pUCS2Buff, WORD iLenSrc, char* psResult, WORD iLenDest)
{
	bool bAscii = true;
	for (WORD i=1; i < iLenSrc; i+=2)
	{
		if (pUCS2Buff[i] != 0)
		{
			bAscii = false;
			break;
		}
	}
	int j = 0;
	if (bAscii)
	{
		for (WORD i=0; i < iLenSrc && j < iLenDest; i+=2)
		{
			psResult[j++] = pUCS2Buff[i];
			if (pUCS2Buff[i] == 0)
				break;
		}
		psResult[j] = 0;
	}
	else
	{
		sprintf(psResult, "0x");
		j = 2;
		for (WORD i=1; i < iLenSrc && j < iLenDest; i+=2)
		{
			if (pUCS2Buff[i-1] == 0 && pUCS2Buff[i] == 0)
				break;
			sprintf(psResult + j, "%02x", (BYTE)(pUCS2Buff[i-1]));
			j += 2;
			sprintf(psResult + j, "%02x", (BYTE)(pUCS2Buff[i]));
			j += 2;
		}
		psResult[j] = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceIceInitReq(CObjString *pContentStr)
{
	ICE_SERVER_TYPES_S *pStruct = (ICE_SERVER_TYPES_S*)m_pMplMcmsProt->getpData();
	 *pContentStr << "\nice_env                : " << pStruct->ice_env
				  << "\nuser_name				:" << pStruct->authParams.user_name
				  << "\nPass    				:" << pStruct->authParams.password
				  << "\nRelayPort    			:" << pStruct->relay_udp_server_params.port
 				  << "\nRelayIpAddr    			:" << pStruct->relay_udp_server_params.sIpAddr
 				  << "\nRelayHostName			:" << pStruct->relay_udp_server_params.sHostName
				  << "\nSTUNPort				:" << pStruct->stun_pass_server_params.port
				  << "\nSTUNIpAddr				:" << pStruct->stun_pass_server_params.sIpAddr
				  << "\nSTUNHostName			:" << pStruct->stun_pass_server_params.sHostName
				  << "\nSTUNUDPIpAddr  			:" << pStruct->stun_udp_server_params.sIpAddr
				  << "\nSTUNUDPPort				:" << pStruct->stun_udp_server_params.port
				  << "\nSTUNUDPHost				:" << pStruct->stun_udp_server_params.sHostName
				  << "\nSTUNTCPIpAddr  			:" << pStruct->stun_tcp_server_params.sIpAddr
	 			  << "\nSTUNTCPPPort			:" << pStruct->stun_tcp_server_params.port
	 	 	 	  << "\nSTUNTCPPHost			:" << pStruct->stun_tcp_server_params.sHostName
	 			 << "\nipAddrsStr			    :" << pStruct->ipAddrsStr ;
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceDtlsStartReq(CObjString *pContentStr, const char* OpcodeString)
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
void CMplMcmsProtocolTracer::TraceDtlsEndInd(CObjString *pContentStr)
{
	const mcIndCmDtlsEnd *pStruct = (const mcIndCmDtlsEnd *)m_pMplMcmsProt->getpData();

	*pContentStr <<"\nCONTENT: IP_CM_DTLS_END_IND:"<<"\n";

	*pContentStr << "status				: "<< pStruct->status << "\n";
	*pContentStr << "channelType			: "<< pStruct->channelType << "\n";
	*pContentStr << "channelDirection	: "<< pStruct->channelDirection << "\n";

	if (pStruct->status == STATUS_OK)
		TraceSdesSpecificParams(pContentStr, pStruct->sdesCap);
}


////////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceMrmpStreamIsMust(CObjString *pContentStr)
{
	const MrmpStreamIsMustStruct *pStruct = (const MrmpStreamIsMustStruct *)m_pMplMcmsProt->getpData();

	*pContentStr <<"\nCONTENT:\n";

	*pContentStr << "unChannelHandle  : "<< pStruct->unChannelHandle << "\n";
	*pContentStr << "unSyncSource     : "<< pStruct->unSyncSource << "\n";
	*pContentStr << "bIsMust          : "<< (DWORD)pStruct->bIsMust << "\n";

}


////////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceHighUsageCPUInd(CObjString *pContentStr)
{
	CM_HIGH_CPU_USAGE_S *pStruct = (CM_HIGH_CPU_USAGE_S*)m_pMplMcmsProt->getpData();
	*pContentStr << "CONTENT:\nboard_id              : " << pStruct->board_id
				 << "\nis_exceed  			  : " << pStruct->is_exceed;
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceCGPlayAudioReq(CObjString *pContentStr)
{
	const TCGPlayAudioReqStruct *pStruct = (TCGPlayAudioReqStruct*)m_pMplMcmsProt->getpData();

	*pContentStr << "\nCONTENT: CG_PLAY_AUDIO_REQ:"
				 << "\n enAudioGain      : " << EAudioGainPresetNames[pStruct->enAudioGain]
				 << "\n unNumOfRepetition: " << pStruct->unNumOfRepetition
				 << "\n fileNameLength   : " << pStruct->fileNameLength
				 << "\n fileName         : " << pStruct->fileName;
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceAudioEncUpdateSeenImageReq(CObjString *pContentStr)
{
	TAudioEncoderUpdateSeenImageSsrcReq * pStruct = (TAudioEncoderUpdateSeenImageSsrcReq *)m_pMplMcmsProt->getpData();
	*pContentStr << "CONTENT:\ntUpdateSeenImageSsrcReq: ";
	if((DWORD)pStruct->unSSRC == INVALID)
		*pContentStr <<"INVALID";
	else
		*pContentStr <<pStruct->unSSRC ;
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceIceInitInd(CObjString *pContentStr)
{
	ICE_INIT_IND_S *pStruct = (ICE_INIT_IND_S*)m_pMplMcmsProt->getpData();

	*pContentStr 	<< "\nSTUN TCP Status       : ";  ::getIceStatusErrorText((iceServersStatus)pStruct->STUN_tcp_status, *pContentStr);
	*pContentStr    << "\nSTUN UDP Status       : ";  ::getIceStatusErrorText((iceServersStatus)pStruct->STUN_udp_status, *pContentStr);
	*pContentStr    << "\nRelay TCP Status      : ";  ::getIceStatusErrorText((iceServersStatus)pStruct->Relay_tcp_status, *pContentStr);
	*pContentStr 	<< "\nRelay UDP Status      : ";  ::getIceStatusErrorText((iceServersStatus)pStruct->Relay_udp_status, *pContentStr);
    *pContentStr    << "\nFW Type               : ";  ::getIceFwType((firewallTypes)pStruct->fw_type, *pContentStr);
    *pContentStr	<< "\nICE Environment       : ";  ::getIceEnv((eIceEnvTypes)pStruct->ice_env, *pContentStr);
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceWebRtcConnectReq(CObjString *pContentStr)
{
  mcReqCmWebRtcConnect *pStruct = (mcReqCmWebRtcConnect*)m_pMplMcmsProt->getpData();

  *pContentStr << "\nSdpSize                : " << pStruct->sdpSize
               << "\nSdp                    : " << pStruct->sdp;

  TraceWebRtcExternalUdpAddr(pContentStr, &pStruct->mcExternalUdpAddressIp);

  for (int i = 0; i < MAX_MEDIA_TYPES; i++)
	  TraceWebRtcInternalUdpAddr(pContentStr, &pStruct->openUdpPorts[i]);
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceWebRtcConnectInd(CObjString *pContentStr)
{
  mcIndCmWebRtcConnectInd *pStruct = (mcIndCmWebRtcConnectInd*)m_pMplMcmsProt->getpData();

  *pContentStr << "\nStatus                	: " << pStruct->status
               << "\nSdpSize                : " << pStruct->sdpSize
               << "\nSdp		          	: " << pStruct->sdp;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceWebRtcExternalUdpAddr(CObjString *pContentStr, const mcTransportAddress *pAddr)
{
  *pContentStr << "\nExternalAddress:";
  *pContentStr << "\n  transport  :" << GetTransportTypeName(pAddr->transportType);
  *pContentStr << "\n  ipVersion  :" << pAddr->ipVersion;

  if (pAddr->ipVersion == eIpVersion4)
  {
    *pContentStr << " (V4)";
    *pContentStr << "\n  addr.v4.ip :";
    char str[30];
    sprintf(str, "%x", pAddr->addr.v4.ip);
    *pContentStr << "0x" << str;
  }
  else
  {
    *pContentStr << " (V6)";
    *pContentStr << "\n  addr.v6.ip :";
    char str[64];
    memset(str, '\0', 64);
    ::ipToString(*pAddr, str, 1);
    *pContentStr << str;
  }

  *pContentStr << "\n  port       :" << pAddr->port;

}

//////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocolTracer::TraceWebRtcInternalUdpAddr(CObjString *pContentStr, const mcReqCmOpenUdpPortOrUpdateUdpAddr* pUdpAddr)
{
  *pContentStr << "\nInternalAddress:";
  *pContentStr << "\n  channelType                    :"; ::GetChannelTypeName(pUdpAddr->channelType, *pContentStr);
  *pContentStr << "\n  channelDirection               :"; ::GetChannelDirectionName(pUdpAddr->channelDirection, *pContentStr);
  *pContentStr << "\n  CmLocalUdpAddressIp.transport  :" << GetTransportTypeName(pUdpAddr->CmLocalUdpAddressIp.transportType);
  *pContentStr << "\n  CmLocalUdpAddressIp.ipVersion  :" << pUdpAddr->CmLocalUdpAddressIp.ipVersion;

  if (pUdpAddr->CmLocalUdpAddressIp.ipVersion == eIpVersion4)
  {
    *pContentStr << " (V4)";
    *pContentStr << "\n  CmLocalUdpAddressIp.addr.v4.ip :";
    char str[30];
    sprintf(str, "%x", pUdpAddr->CmLocalUdpAddressIp.addr.v4.ip);
    *pContentStr << "0x" << str;
  }
  else
  {
    *pContentStr << " (V6)";
    *pContentStr << "\n  CmLocalUdpAddressIp.addr.v6.ip :";
    char str[64];
    memset(str, '\0', 64);
    ::ipToString(pUdpAddr->CmLocalUdpAddressIp, str, 1);
    *pContentStr << str;
  }

  *pContentStr << "\n  CmLocalUdpAddressIp.port       :" << pUdpAddr->CmLocalUdpAddressIp.port;
  *pContentStr << "\n  Local RTCP port                :" << pUdpAddr->LocalRtcpPort;
  *pContentStr << "\n  CmRemoteUdpAddressIp.transport :" << GetTransportTypeName(pUdpAddr->CmRemoteUdpAddressIp.transportType);
  *pContentStr << "\n  CmRemoteUdpAddressIp.ipVersion :" << pUdpAddr->CmRemoteUdpAddressIp.ipVersion;

  if (pUdpAddr->CmRemoteUdpAddressIp.ipVersion == eIpVersion4)
  {
    *pContentStr << " (V4)";
    *pContentStr << "\n  CmRemoteUdpAddressIp.addr.v4.ip:";
    char str[30];
    sprintf(str, "%x", pUdpAddr->CmRemoteUdpAddressIp.addr.v4.ip);
    *pContentStr << "0x" << str;
  }
  else
  {
    *pContentStr << "V6";
    *pContentStr << "\n CmRemoteUdpAddressIp.addr.v6.ip :";
    char str[64];
    memset(str, '\0', 64);
    ::ipToString(pUdpAddr->CmRemoteUdpAddressIp, str, 1);
    *pContentStr << str;
  }
  *pContentStr << "\n  CmRemoteUdpAddressIp.port      :" << pUdpAddr->CmRemoteUdpAddressIp.port;
  *pContentStr << "\n  Remote RTCP port               :" << pUdpAddr->RemoteRtcpPort;
  *pContentStr << "\n  tosValue[MEDIA_TOS_VALUE_PLACE]:" << pUdpAddr->tosValue[MEDIA_TOS_VALUE_PLACE];
  *pContentStr << "\n  tosValue[RTCP_TOS_VALUE_PLACE] :" << pUdpAddr->tosValue[RTCP_TOS_VALUE_PLACE];
  *pContentStr << "\n  Ice channel RTP id             :" << pUdpAddr->ice_channel_rtp_id;
  *pContentStr << "\n  Ice channel RTCP id            :" << pUdpAddr->ice_channel_rtcp_id;
  char strMask[30];
   sprintf(strMask, "%x", pUdpAddr->RtcpCnameMask);
  *pContentStr << "\n  RTCP cname mask                :" << "0x" << strMask;
  *pContentStr <<" \n  connMode				      :" << GetConnectionTransportModeName(pUdpAddr->connMode);
  *pContentStr << "\n   MediaDetection Time          :" << pUdpAddr->ulDetectionTimerLen;
  *pContentStr << "\n   Msft EP Type          		 :" << pUdpAddr->uMsftType;
}
