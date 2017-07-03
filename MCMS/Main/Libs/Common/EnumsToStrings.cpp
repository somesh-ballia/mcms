#include "ObjString.h"
#include "CommonStructs.h"
#include "McuMngrStructs.h"
#include "CardsStructs.h"
#include "CsStructs.h"
#include "ApacheDefines.h"
#include "McmsProcesses.h"
#include "SerializeObject.h"
#include "DefinesGeneral.h"
#include "RtmIsdnMngrInternalDefines.h"
#include "RtmIsdnMaintenanceStructs.h"
#include "ArtDefinitions.h"
#include "AllocateStructs.h"
#include "IpRtpFeccRoleToken.h"
#include "IpRtpReq.h"
#include "ConfPartyApiDefines.h"
#include "Trace.h"
#include "TraceStream.h"
#include "EnumsToStrings.h"

//--------------------------------------------------------------------------
char* IpV6ConfigurationTypeToString(APIU32 v6Type, bool caps = false)
{
  switch (v6Type)
  {
    case eV6Configuration_Auto  : return (char*)((caps) ? "AUTO" : "auto");
    case eV6Configuration_DhcpV6: return (char*)((caps) ? "DHCP" : "dhcp");
    case eV6Configuration_Manual: return (char*)((caps) ? "MANUAL" : "manual");
  }
  return NULL;
}

//--------------------------------------------------------------------------
char* IpTypeToString(APIU32 ipType, bool caps)
{
  switch (ipType)
  {
    case eIpType_None: return (char*)((caps) ? "NONE" : "none");
    case eIpType_IpV4: return (char*)((caps) ? "IPV4" : "ipv4");
    case eIpType_IpV6: return (char*)((caps) ? "IPV6" : "ipv6");
    case eIpType_Both: return (char*)((caps) ? "BOTH" : "both");
  }
  return NULL;
}

//--------------------------------------------------------------------------
char* DnsDhcpConfigurationTypeToString(APIU32 dnsDhcpConfigType)
{
  switch (dnsDhcpConfigType)
  {
    case eDnsDhcpV4: return "DnsDhcpV4";
    case eDnsDhcpV6: return "DnsDhcpV6";
  }
  return NULL;
}

//--------------------------------------------------------------------------
const char* PlatformTypeToString(APIU32 platformType)
{
  switch (platformType)
  {
    case eGideonLite: return "GideonLite";
    case eGideon5   : return "Gideon5";
    case eGideon14  : return "Gideon14";
    case eAmos      : return "Amos";
    case eYona      : return "Yona";
  }
  return NULL;
}

//--------------------------------------------------------------------------
char* UrlTypeToString(APIU32 urlType)
{
  switch (urlType)
  {
    case eFtp: return "Ftp";
    case eNfs: return "Nfs";
  }
  return NULL;
}

//--------------------------------------------------------------------------
const char* DnsConfigurationStatusToString(eDnsConfigurationStatus configStatus)
{
  // see enumerator eDnsConfigurationStatus in Libs/Common/DefinesGeneral.h
  static const char* dnsConfigurationStatusName [] =
  {
    "DnsConfiguration_Success",
    "DnsConfiguration_Failure",
    "DnsConfiguration_NotConfigured"
  };
  return (configStatus <= MAX_NUM_OF_DNS_CONFIG_STATUS
         ?
         dnsConfigurationStatusName[configStatus] : "Invalid");
}

//--------------------------------------------------------------------------
const char* GetStringValidityStatus(eStringValidityStatus status)
{
  return (0 <= status && status < NUM_OF_STRING_VALIDITY_STATUSES
         ?
         StringValidityStatuses[status] : "Invalid string validity type");
}

//--------------------------------------------------------------------------
const char* GetSystemCardsModeStr(eSystemCardsMode theMode)
{
  return (0 <= theMode && theMode < NUM_OF_SYSTEM_CARDS_MODES
         ?
         sSystemCardsModes[theMode] : "Invalid mode");
}

//--------------------------------------------------------------------------
const char* GetSystemRamSizeStr(eSystemRamSize theSize)
{
  return (0 <= theSize && theSize <  NUM_OF_SYSTEM_RAM_SIZES
         ?
         sSystemRamSizes[theSize] : "Invalid size");
}

//--------------------------------------------------------------------------
char* CardTypeToString(APIU32 cardType)
{
  switch (cardType)
  {
    case eEmpty             : return "empty";
    case eCpuBoard          : return "cpu_board";
    case eSwitch            : return "switch";
    case eMfa_26            : return "mpm-f";
    case eMfa_13            : return "mpm-h";
    case eRtmIsdn           : return "rtm_isdn";
    case eRtmIsdn_9PRI           : return "rtm_isdn_9pri";
    case eRtmIsdn_9PRI_10G           : return "rtm_isdn_9pri_10g";
    case eControl           : return "control";
    case eMpmPlus_20        : return "mpm_plus_20";
    case eMpmPlus_40        : return "mpm_plus_40";
    case eMpmPlus_80        : return "mpm_plus_80";
    case eMpmPlus_MezzanineA: return "mpm_plus_mezzanine_a";
    case eMpmPlus_MezzanineB: return "mpm_plus_mezzanine_b";
    case eMpmx_80           : return "mpm-x_full";
    case eMpmx_40           : return "mpm-x_partial";
    case eMpmx_20           : return "mpm-y_partial";
    case eMpmx_Soft_Half    : return "mpm-x_soft_half"; //OLGA - SoftMCU
    case eMpmx_Soft_Full    : return "mpm-x_soft_full"; 
    case eMpmRx_Half        : return "mpm-rx_partial";  //for EMA we need to send partial and not half. otherwise we will get the media card in MAJOR state in HW monitor
    case eMpmRx_Full        : return "mpm-rx_full";
    case eMpmRx_Ninja       : return "mpm-rx_ninja";
  }
  return NULL;
}

//--------------------------------------------------------------------------
char* CardUnitPhysicalTypeToString(APIU32 unitPhysicalType)
{
  switch (unitPhysicalType)
  {
    case eUndefined: return "Undefined";
    case ePQ       : return "PQ";
    case eDsp      : return "DSP";
  }
  return NULL;
}

//--------------------------------------------------------------------------
char* CardUnitConfiguredTypeToString(APIU32 unitConfigType)
{
  switch (unitConfigType)
  {
    case eNotConfigured: return "NotConfigured";
    case eArt          : return "Art";
    case eVideo        : return "Video";
    case eArtCntlr     : return "ArtController";
    case ePost         : return "Post"; // for Emb internal use; should not arrive to Mcms!
    case eRtm          : return "Rtm";  // for Emb internal use; should not arrive to Mcms!
  }
  return NULL;
}

//--------------------------------------------------------------------------
const char* UnitReconfigStatusToString(APIU32 unitReconfigStatus)
{
  switch (unitReconfigStatus)
  {
    case eUnitReconfigOk  : return "Ok";
    case eUnitReconfigFail: return "Fail";
    default               : return "Illegal status";
  }
  return NULL;
}
//--------------------------------------------------------------------------
char* CardUnitLoadedStatusToString(APIU32 unitLoadedStatus)
{
  switch (unitLoadedStatus)
  {
    case eOk         : return "Ok";
    case eUnitStartup: return "Startup";
    case eUnknown    : return "Unknown";
    case eFatal      : return "Fatal";
    case eReady      : return "Ready";
    case eNotExist   : return "NotExist";
    case eNotLoaded  : return "NotLoaded";
  }
  return NULL;
}

//--------------------------------------------------------------------------
char* CardMediaIpConfigStatusToString(APIU32 mediaIpConfigStatus)
{
  switch (mediaIpConfigStatus)
  {
    case eMediaIpConfig_Ok          : return "OK";
    case eMediaIpConfig_NotSupported: return "Not supported";
    case eMediaIpConfig_NotExist    : return "Does not exist";
    case eMediaIpConfig_IpFail      : return "IP failure";
    case eMediaIpConfig_IpDuplicate : return "Duplicate IP";
    case eMediaIpConfig_DhcpFail    : return "DHCP failure";
    case eMediaIpConfig_VLanFail    : return "VLAN failure";
  }
  return NULL;
}

//--------------------------------------------------------------------------
char* CardStateToString(APIU32 cardState)
{
  switch (cardState)
  {
    case eNormal      : return "normal";
    case eMajorError  : return "major_error";
    case eMinorError  : return "minor_error";
    case eSimulation  : return "simulation_mode";
    case eCardStartup : return "startup";
    case eNoConnection: return "no_connection";
    case eDisabled    : return "disabled";
  }
  return NULL;
}

//--------------------------------------------------------------------------
char* ShelfMngrComponentStatusToString(APIU32 compStatus)
{
  switch (compStatus)
  {
    case eSmComponentOk        : return "Ok";
    case eSmComponentMajor     : return "Major";
    case eSmComponentNotExist  : return "NotExist";
    case eSmComponentResetting : return "Resetting";
    case eSmComponentDiagnostic: return "Diagnostic";
  }
  return NULL;
}

//--------------------------------------------------------------------------
const char* ShelfMngrComponentTypeToString(APIU32 compType)
{
  switch (compType)
  {
    case eShmComp_Illegal    : return "Illegal";
    case eShmComp_SwitchBoard: return "Switch";
    case eShmComp_MfaBoard1  : return "Media_board";
    case eShmComp_MfaBoard2  : return "Media_board";
    case eShmComp_MfaBoard3  : return "Media_board";
    case eShmComp_MfaBoard4  : return "Media_board";
    case eShmComp_MfaBoard5  : return "Media_board";
    case eShmComp_MfaBoard6  : return "Media_board";
    case eShmComp_MfaBoard7  : return "Media_board";
    case eShmComp_MfaBoard8  : return "Media_board";
    case eShmComp_MfaMpmx    : return "Media_Mpmx";
    case eShmComp_MfaMpmRx   : return "Media_MpmRx";
    case eShmComp_RtmIsdn    : return "RtmIsdn";
    case eShmComp_RtmIsdn9   : return "RtmIsdn9";
    case eShmComp_RtmIsdn9_10G    : return "RtmIsdn9G";
    case eShmComp_RtmLan     : return "RtmLan";
    case eShmComp_RtmLan4     : return "RtmLan4";
    case eShmComp_RtmLan4_10G     : return "RtmLan4G";
    case eShmComp_McmsCpu    : return "MCMS_CPU";
    case eShmComp_Fan        : return "Fan";
    case eShmComp_PowerSupply: return "PowerSupply";
    case eShmComp_Lan        : return "LAN";
    case eShmComp_Backplane  : return "Backplane";
    case eShmComp_Iam        : return "IAM";
  }
  return NULL;
}

//--------------------------------------------------------------------------
char* CsCompTypeToString(compTypes compType)
{
  switch (compType)
  {
    case emNonComponent: return "NonComponent";
    case emCompCSMngnt : return "CSMngnt";
    case emCompCSH323  : return "CSH323";
    case emCompCSSIP   : return "CSSIP";
	case emCompCSSignalPort: return "CSSignalPort";
  }
  return NULL;
}

//--------------------------------------------------------------------------
char* CsCompStatusToString(compStatuses compStatus)
{
  switch (compStatus)
  {
    case emCompOk       : return "Ok";
    case emCompFailed   : return "Failed";
    case emCompRecovered: return "Recovered";
  }
  return NULL;
}

//--------------------------------------------------------------------------
char* CsRecoveryReasonToString(recoveryReasons recoveryReason)
{
  switch (recoveryReason)
  {
    case emRecoveryUnknown      : return "Unknown";
    case emRecoveryFromException: return "FromException";
    case emRecoveryFromStuck    : return "FromStuck";
    case emExitTooManyRecoveries: return "TooManyRecoveries";
    case emExternalTermination  : return "ExternalTermination";
  }
  return NULL;
}

//--------------------------------------------------------------------------
const char* MainEntityToString(APIU32 entityType)
{
  // see enumerator eMainEntities in MCMS/McmIncld/Common/MplMcmsStructs.h
  static const char* mainEntityName [] =
  {
    "Mcms",
    "Mpl",
    "CS",
    "Mpl_simulation",
    "Ema",
    "CM_Switch",
    "Shelf",
    "Art",
    "Video",
    "CardManager",
    "RTM",
    "Mux",
    "VideoVmp",
    "AudioAmp",
    "MPProxy"

  };
  return (entityType < NUM_OF_MAIN_ENTITIES && entityType < (int)(sizeof(mainEntityName)/sizeof(mainEntityName[0]))
         ?
         mainEntityName[entityType] : "Invalid");
}

//--------------------------------------------------------------------------
char* ResourceTypeToString(APIU32 resourceType)
{
  switch (resourceType)
  {
    case ePhysical_res_none         : return "Physical_res_none";
    case ePhysical_audio_controller : return "Physical_audio_controller";
    case ePhysical_art              : return "Physical_art";
    case ePhysical_art_light        : return "Physical_art_light";
    case ePhysical_video_encoder    : return "Physical_video_encoder";
    case ePhysical_video_decoder    : return "Physical_video_decoder";
    case ePhysical_rtm              : return "Physical_rtm";
    case ePhysical_ivr_controller   : return "Physical_ivr_controller";
    case ePhysical_mux              : return "Physical_mux";
	case ePhysical_mrmp             : return "Physical_mrmp";
  }
  return "Invalid";
}

//--------------------------------------------------------------------------
const char* CntrlTypeToString(APIU32 CntrlType)
{
  static const char* CntrlTypeNames [] =
  {
    "Control Normal",
    "Control Master",
    "Control Slave"
  };
  return (sizeof(CntrlTypeNames)/sizeof(CntrlTypeNames[0]) > CntrlType
         ?
         CntrlTypeNames[CntrlType] : "InvalidCntrlType");
}

//--------------------------------------------------------------------------
char* LogicalResourceTypeToString(APIU32 logicalResourceType)
{
	switch (logicalResourceType)
	{
		case eLogical_res_none                               : return "Logical_res_none";
		case eLogical_audio_encoder                          : return "Logical_audio_encoder";
		case eLogical_audio_decoder                          : return "Logical_audio_decoder";
		case eLogical_audio_controller                       : return "Logical_audio_controller";
		case eLogical_video_encoder_content                  :  return "Logical_video_encoder_content";
		case eLogical_video_encoder                          : return "Logical_video_encoder";
		case eLogical_video_decoder                          : return "Logical_video_decoder";
		case eLogical_rtp                                    : return "Logical_rtp";
		case eLogical_content_rtp                            : return "Logical_content_rtp";
		case eLogical_ip_signaling                           : return "Logical_ip_signaling";
		case eLogical_net                                    : return "Logical_net";
		case eLogical_ivr_controller                         : return "Logical_ivr_controller";
		case eLogical_mux                                    : return "Logical_mux";
		case eLogical_COP_Dynamic_decoder                    : return "Logical_COP_Dynamic_decoder";
		case eLogical_COP_CIF_encoder                        : return "Logical_COP_CIF_encoder";
		case eLogical_COP_4CIF_encoder                       : return "Logical_COP_4CIF_encoder";
		case eLogical_COP_VSW_encoder                        : return "Logical_COP_VSW_encoder";
		case eLogical_COP_VSW_decoder                        : return "Logical_COP_VSW_decoder";
		case eLogical_COP_PCM_encoder                        : return "Logical_COP_PCM_encoder";
		case eLogical_COP_LM_decoder                         : return "Logical_COP_LM_decoder";
		case eLogical_COP_HD720_encoder                      : return "Logical_COP_HD720_encoder";
		case eLogical_COP_HD1080_encoder                     : return "Logical_COP_HD1080_encoder";
		case eLogical_PCM_manager                            : return "Logical_PCM_manager";
		case eLogical_COP_dummy_encoder                      : return "Logical_COP_dummy_encoder";
		case eLogical_VSW_dummy_encoder                      : return "Logical_VSW_dummy_encoder";
		case eLogical_VSW_dummy_decoder                      : return "Logical_VSW_dummy_decoder";
		case eLogical_relay_rtp                              : return "Logical_relay_rtp";
		case eLogical_relay_audio_encoder                    : return "Logical_relay_audio_encoder";
		case eLogical_relay_audio_decoder                    : return "Logical_relay_audio_decoder";
		case eLogical_relay_video_encoder                    : return "Logical_relay_video_encoder";
		case eLogical_relay_svc_to_avc_rtp                   : return "Logical_relay_svc_to_avc_rtp";
		case eLogical_relay_avc_to_svc_video_encoder_1       : return "Logical_relay_avc_to_svc_video_encoder_1";
		case eLogical_relay_avc_to_svc_video_encoder_2       : return "Logical_relay_avc_to_svc_video_encoder_2";
		case eLogical_legacy_to_SAC_audio_encoder            : return "Logical_legacy_to_SAC_audio_encoder";
		case eLogical_relay_avc_to_svc_rtp                   : return "Logical_relay_avc_to_svc_rtp";
		case eLogical_relay_avc_to_svc_rtp_with_audio_encoder: return "Logical_relay_avc_to_svc_rtp_with_audio_encoder";
		case eLogical_ip_sigOrganizer                        : return "Logical_ip_sigOrganizer";
		case eLogical_ip_sigFocus                            : return "Logical_ip_sigFocus";
		case eLogical_ip_sigEventPackage                     : return "Logical_ip_sigEventPackage";
	}
	return "Invalid";
}

//--------------------------------------------------------------------------
const char* AllocationPolicyToString(APIU32 allocationPolicy)
{
  switch (allocationPolicy)
  {
    case eNoAllocationPolicy            : return "No Allocation Policy";
    case eAllocateAllRequestedResources : return "Allocation All Requested Resources";
    case eAllowDowngradingToAudioOnly   : return "Allow Downgrading To Audio Only";
  }
  return "Invalid";
}

//--------------------------------------------------------------------------
char* HeaderTypeToString(APIU32 headerType)
{
  switch (headerType)
  {
    case eHeaderNone    : return "HeaderNone";
    case eHeaderTpkt    : return "HeaderTpkt";
    case eHeaderCommon  : return "HeaderCommon";
    case eHeaderPhysical: return "HeaderPhysical";
    case eHeaderPortDesc: return "HeaderPortDesc";
    case eHeaderMsgDesc : return "HeaderMsgDesc";
    case eHeaderCs      : return "HeaderCs";
    case eHeaderUnknown : return "HeaderUnknown";
  }
  return "Invalid";
}

//--------------------------------------------------------------------------
char* RsrcCntlTypeToString(APIU32 rsrcCntlType)
{
  switch (rsrcCntlType)
  {
    case E_NORMAL                             : return "E_NORMAL";
    case E_AC_MASTER                          : return "E_AC_MASTER";
    case E_AC_SLAVE                           : return "E_AC_SLAVE";
    case E_AC_RESERVED                        : return "E_AC_RESERVED";
    case E_VIDEO_MASTER_LB_ONLY               : return "E_VIDEO_MASTER_LB_ONLY";
    case E_VIDEO_SLAVE_FULL_ENCODER           : return "E_VIDEO_SLAVE_FULL_ENCODER";
    case E_VIDEO_MASTER_SPLIT_ENCODER         : return "E_VIDEO_MASTER_SPLIT_ENCODER";
    case E_VIDEO_SLAVE_SPLIT_ENCODER          : return "E_VIDEO_SLAVE_SPLIT_ENCODER";
    case E_VIDEO_MASTER_SPLIT_ENCODER_HALF_DSP: return "E_VIDEO_MASTER_SPLIT_ENCODER_HALF_DSP";
    case E_VIDEO_SLAVE_SPLIT_ENCODER_HALF_DSP : return "E_VIDEO_SLAVE_SPLIT_ENCODER_HALF_DSP";
    case E_VIDEO_CONTENT_DECODER              : return "E_VIDEO_CONTENT_DECODER";
    case E_VIDEO_CONTENT_ENCODER              : return "E_VIDEO_CONTENT_ENCODER";
    case E_ENCODER_PCM                        : return "E_ENCODER_PCM";
  }
  return "Invalid";
}

//--------------------------------------------------------------------------
char* AuthorizationGroupTypeToString(int authorizationGroup)
{
  switch (authorizationGroup)
  {
    case SUPER          : return "Super";
    case ORDINARY       : return "Ordinary";
    case AUTH_OPERATOR  : return "Auth_Operator";
    case RECORDING_USER : return "Recording_User";
    case RECORDING_ADMIN: return "Recording_Admin";
    case GUEST          : return "Guest";
    case AUDITOR        : return "Auditor";
	case ADMINISTRATOR_READONLY:	return "Administrator_Readonly";
  }
  return NULL;
}

//--------------------------------------------------------------------------
char* UpdateOperatorTypeToString(int updateType)
{
  switch (updateType)
  {
    case NEW_OPERATOR   : return "Add_User";
    case UPDATE_OPERATOR: return "Update_User";
    case DEL_OPERATOR   : return "Delete_User";
  }
  return NULL;
}

//--------------------------------------------------------------------------
const char* ProcessTypeToString(eProcessType type)
{
  return ProcessTypeToStr(type); // Mcms Processes Names
}

//--------------------------------------------------------------------------
char* FailReadingFileActiveAlarmTypeToString(eFailReadingFileActiveAlarmType activeAlarmType)
{
  switch (activeAlarmType)
  {
    case eNoActiveAlarm      : return "NoActiveAlarm";
    case eActiveAlarmExternal: return "ActiveAlarmExternal";
    case eActiveAlarmInernal : return "ActiveAlarmInernal";
  }
  return "Unknown";
}

//--------------------------------------------------------------------------
const char* FailReadingFileOperationTypeToString(eFailReadingFileOperationType operationType)
{
  switch (operationType)
  {
    case eNoAction  : return "NoAction";
    case eRenameFile: return "RenameFile";
    case eRemoveFile: return "DeleteFile";
  }
  return "Unknown";
}

//--------------------------------------------------------------------------
const char* PortSpeedTypeToString(ePortSpeedType type)
{
  // see CommonStructs.h:ePortSpeedType
  static const char* names [] =
  {
    "speed_auto",
    "speed_10",
    "speed_10_duplex_full",
    "speed_100",
    "speed_100_duplex_full",
    "speed_1000",
    "speed_1000_duplex_full"
  };
  return ((DWORD)type < sizeof(names)/sizeof(names[0])
         ?
         names[type] : "Invalid");
}

//--------------------------------------------------------------------------
const char* IPv6ScopeIdToString(enScopeId theScopeId)
{
  switch (theScopeId)
  {
    case eScopeIdSite  : return "ScopeId_Site";
    case eScopeIdGlobal: return "ScopeId_Global";
    case eScopeIdLink  : return "ScopeId_Link";
    case eScopeIdOther : return "ScopeId_Other";
  }
  return "Invalid";
}

//--------------------------------------------------------------------------
const char* SpanEnableTypeStrToString(eRtmIsdnSpanEnableType theType)
{
  return (((0 <= theType) && (NUM_OF_SPAN_ENABLED_TYPES > theType))
         ?
         spanEnableTypeStr[theType] : "Invalid type");
}

//--------------------------------------------------------------------------
const char* SpanTypeToString(eSpanType theType)
{
  return (((0 <= theType) && (NUM_OF_SPAN_TYPES > theType))
         ?
         spanTypeStr[theType] : "Invalid type");
}

//--------------------------------------------------------------------------
const char* SystemSpanTypeToString(eSystemSpanType theType)
{
  return (((0 <= theType) && (NUM_OF_SYSTEM_SPAN_TYPES > theType))
         ?
         systemsSpanTypeStr[theType] : "Invalid type");
}

//--------------------------------------------------------------------------
const char* ServiceTypeToString(eServiceType theType)
{
  return (((0 <= theType) && (NUM_OF_SERVICE_TYPES > theType))
         ?
         serviceTypeStr[theType] : "Invalid type");
}

//--------------------------------------------------------------------------
const char* DfltNumToString(eDfltNumType theType)
{
  return (((0 <= theType) && (NUM_OF_DFLT_NUM_TYPES > theType))
         ?
         dfltNumTypeStr[theType] : "Invalid type");
}

//--------------------------------------------------------------------------
const char* NumPlanToString(eNumPlanType theType)
{
  return (((0 <= theType) && (NUM_OF_NUM_PLAN_TYPES > theType))
         ?
         numPlanTypeStr[theType] : "Invalid type");
}

//--------------------------------------------------------------------------
const char* VoiceTypeToString(eVoiceType theType)
{
  return (((0 <= theType) && (NUM_OF_VOICE_TYPES > theType))
         ?
         voiceTypeStr[theType] : "Invalid type");
}

//--------------------------------------------------------------------------
const char* NetSpecFacilityTypeToString(eNetSpecFacilityType theType)
{
  return (((0 <= theType) && (NUM_OF_NET_SPEC_FACILITY_TYPES > theType))
         ?
         facilityTypeStr[theType] : "Invalid type");
}

//--------------------------------------------------------------------------
const char* FramingTypeToString(eFramingType theType)
{
  return (((0 <= theType) && (NUM_OF_FRAMING_TYPES > theType))
         ?
         framingTypeStr[theType] : "Invalid type");
}

//--------------------------------------------------------------------------
const char* SideTypeToString(eSideType theType)
{
  return (((0 <= theType) && (NUM_OF_SIDE_TYPES > theType))
         ?
         sideTypeStr[theType] : "Invalid type");
}

//--------------------------------------------------------------------------
const char* LineCodingTypeToString(eLineCodingType theType)
{
  return (((0 <= theType) && (NUM_OF_LINE_CODING_TYPES > theType))
         ?
         lineCodingTypeStr[theType] : "Invalid type");
}

//--------------------------------------------------------------------------
const char* SwitchTypeToString(eSwitchType theType)
{
  return (((0 <= theType) && (NUM_OF_SWITCH_TYPES > theType)) ? switchTypeStr[theType] : "Invalid type");
}

//--------------------------------------------------------------------------
const char* SpanAlarmToString(eSpanAlarmType theType)
{
  return (((0 <= theType) && (NUM_OF_SPAN_ALARM_TYPES > theType))
         ?
         spanAlarmTypeStr[theType] : "Invalid type");
}

//--------------------------------------------------------------------------
const char* DChannelStateToString(eDChannelStateType theType)
{
  return (((0 <= theType) && (NUM_OF_D_CHANNEL_STATE_TYPES > theType))
         ?
         dChannelStateTypeStr[theType] : "Invalid type");
}

//--------------------------------------------------------------------------
const char* ClockingToString(eClockingType theType)
{
  return (((0 <= theType) && (NUM_OF_CLOCKING_TYPES > theType))
         ?
         clockingTypeStr[theType] : "Invalid type");
}

//--------------------------------------------------------------------------
const char* VLanEntityToString(eVLanEntityType theType)
{
  return (((0 <= theType) && (NUM_OF_VLAN_ENTITY_TYPES > theType))
         ?
         vLanEntityTypeStr[theType] : "Invalid type");
}

//--------------------------------------------------------------------------
const char* NetworkTypeToString(ENetworkType networkType)
{
  switch (networkType)
  {
    case E_NETWORK_TYPE_DUMMY  : return "E_NETWORK_TYPE_DUMMY";
    case E_NETWORK_TYPE_IP     : return "E_NETWORK_TYPE_IP";
    case E_NETWORK_TYPE_ISDN   : return "E_NETWORK_TYPE_ISDN";
    case E_NETWORK_TYPE_PSTN_E1: return "E_NETWORK_TYPE_PSTN_E1";
    case E_NETWORK_TYPE_PSTN_T1: return "E_NETWORK_TYPE_PSTN_T1";
    case E_NETWORK_TYPE_LAST   : return "E_NETWORK_TYPE_LAST";
  }
  return "Unknown";
}

//--------------------------------------------------------------------------
const char* feccKeyToString(feccKeyEnum key)
{
  static const char* feccKeyStrings[] =
  {
    "LEFT",
    "RIGHT",
    "UP",
    "DOWN",
    "ZOOMIN",
    "ZOOMOUT",
    "FOCUSIN",
    "FOCUSOUT"
  };

  return ((key >= 0 && key < eFeccKeyLast)
         ?
         feccKeyStrings[key] : "Invalid");
}

//--------------------------------------------------------------------------
const char* feccPartyTypeToString(feccPartyTypeEnum ePartyType)
{
  return ((ePartyType >= 0 && ePartyType < eFeccPartyTypeLast)
         ?
         feccPartyTypeStr[ePartyType] : "Invalid");
}

//--------------------------------------------------------------------------
char* LdapDirTypeToString(int ldapDirType)
{
  switch (ldapDirType)
  {
    case eMsActiveDirectory: return "ms_active_directory";
  }
  return NULL;
}

//--------------------------------------------------------------------------
char* LdapDirPortToString(int ldapDirPort)
{
  switch (ldapDirPort)
  {
    case e389: return "389";
    case e636: return "636";
  }
  return NULL;
}

//--------------------------------------------------------------------------
char* LdapAuthenticationTypeToString(int ldapAuthenticationType)
{
  switch (ldapAuthenticationType)
  {
    case ePlain   : return "plain";
    case eNTLM    : return "ntlm";
    case eKerberos: return "kerberos";
  }
  return NULL;
}

//--------------------------------------------------------------------------
char* AuthenticationProtocolToString(eAuthenticationProtocol authenticationProtocol)
{
  switch (authenticationProtocol)
  {
    case eNone: return "NONE";
    case eMD5 : return "MD5";
    case eSHA1: return "SHA1";
    case eAuto: return "AUTO";
    case eSHA256: return "SHA256";
    case eAES: return "AES";
	default:
		// Note: some enumeration value are not handled in switch. Add default to suppress warning.
		break;
  }
  return "INVALID_PROTOCOL";
}

//--------------------------------------------------------------------------
char* CascadeModeToString(BYTE cascadeMode)
{
  switch (cascadeMode)
  {
    case CASCADE_MODE_NONE       : return "CASCADE_MODE_NONE";
    case CASCADE_MODE_MASTER     : return "CASCADE_MODE_MASTER";
    case CASCADE_MODE_SLAVE      : return "CASCADE_MODE_SLAVE";
    case CASCADE_MODE_NEGOTIATED : return "CASCADE_MODE_NEGOTIATED";
    case CASCADE_MODE_CONFLICT   : return "CASCADE_MODE_CONFLICT";
    case CASCADE_MODE_MCU        : return "CASCADE_MODE_MCU";
    case CASCADE_MODE_AUTO       : return "CASCADE_MODE_AUTO";
  }
  return "Invalid";
}

//--------------------------------------------------------------------------
const char* MediaStateToString(eConfMediaState confMediaState)
{
	switch(confMediaState)
	{
		case eMediaStateEmpty: return "eMediaStateEmpty";
		case eMediaStateAvcOnly: return "eMediaStateAvcOnly";
		case eMediaStateSvcOnly: return "eMediaStateSvcOnly";
		case eMediaStateMixAvcSvc: return "eMediaStateMixAvcSvc";
	}
	return NULL;
}

//--------------------------------------------------------------------------
const char* TelePresencePartyTypeToString(eTelePresencePartyType telePresencePartyType)
{
	switch(telePresencePartyType)
	{
		case eTelePresencePartyNone: return "eTelePresencePartyNone";
		case eTelePresencePartyRPX: return "eTelePresencePartyRPX";
		case eTelePresencePartyFlex: return "eTelePresencePartyFlex";
		case eTelePresencePartyCTS: return "eTelePresencePartyCTS";
		case eTelePresencePartyInactive: return "eTelePresencePartyInactive";
		default: // Added default for warning: enumeration value 'eTelePresencePartyMaui' not handled in switch
			break;
	}
	return "Invalid";
};

//--------------------------------------------------------------------------
const char* ConfMediaTypeToString(eConfMediaType confMediaType)
{
	switch (confMediaType)
	{
		case eConfMediaType_dummy:            return "eConfMediaType_dummy";
		case eAvcOnly:                        return "eAvcOnly";
		case eSvcOnly:                        return "eSvcOnly";
		case eMixAvcSvc:                      return "eMixAvcSvc";
		case eMixAvcSvcVsw:                   return "eMixAvcSvcVsw";
		case eConfMediaType_last:             return "eConfMediaType_last";
		default:
			FTRACEINTO << "unknown eConfMediaType " << confMediaType;
			break;
	}
	
	return "Unknown_eConfMediaType";
}

//--------------------------------------------------------------------------
const char* RoleLabelToString(ERoleLabel roleLabel)
{
	switch (roleLabel)
	{
		case kRolePeople:                     return "kRolePeople";
		case kRoleContent:                    return "kRoleContent";
		case kRolePresentation:               return "kRolePresentation";
		case kRoleContentOrPresentation:      return "kRoleContentOrPresentation";
		case kRoleLive:                       return "kRoleLive";
		case kRoleLiveOrPresentation:         return "kRoleLiveOrPresentation";
		case kRoleUnknown:                    return "kRoleUnknown";
		default:
			FTRACEINTO << "unknown ERoleLabel " << roleLabel;
			break;
	}

	return "Unknown_ERoleLabel";
};

//--------------------------------------------------------------------------
const char* CapEnumToString(CapEnum capEnumNum)
{
	switch (capEnumNum)
	{
		case eG711Alaw64kCapCode:                     return "G711Alaw64k";
		case eG711Alaw56kCapCode:                     return "G711Alaw56k";
		case eG711Ulaw64kCapCode:                     return "G711Ulaw64k";
		case eG711Ulaw56kCapCode:                     return "G711Ulaw56k";
		case eG722_64kCapCode:                        return "G722_64k";
		case eG722_56kCapCode:                        return "G722_56k";
		case eG722_48kCapCode:                        return "G722_48k";
		case eG722Stereo_128kCapCode:                 return "G722Stereo_128k";
		case eG728CapCode:                            return "G728";
		case eG729CapCode:                            return "G729";
		case eG729AnnexACapCode:                      return "G729AnnexA";
		case eG729wAnnexBCapCode:                     return "G729wAnnexB";
		case eG729AnnexAwAnnexBCapCode:               return "G729AnnexAwAnnexB";
		case eG7231CapCode:                           return "G7231";
		case eIS11172AudioCapCode:                    return "IS11172Audio";
		case eIS13818CapCode:                         return "IS13818";
		case eG7231AnnexCapCode:                      return "G7231Annex";
		case eG7221_32kCapCode:                       return "G7221_32k";
		case eG7221_24kCapCode:                       return "G7221_24k";
		case eG7221_16kCapCode:                       return "G7221_16k";
		case eSiren7_16kCapCode:                      return "Siren7_16k";
		case eSiren14_48kCapCode:                     return "Siren14_48k";
		case eSiren14_32kCapCode:                     return "Siren14_32k";
		case eSiren14_24kCapCode:                     return "Siren14_24k";
		case eG7221C_48kCapCode:                      return "G7221C_48k";
		case eG7221C_32kCapCode:                      return "G7221C_32k";
		case eG7221C_24kCapCode:                      return "G7221C_24k";
		case eG7221C_CapCode:                         return "G7221C";
		case eSiren14Stereo_48kCapCode:               return "Siren14Stereo_48k";
		case eSiren14Stereo_56kCapCode:               return "Siren14Stereo_56k";
		case eSiren14Stereo_64kCapCode:               return "Siren14Stereo_64k";
		case eSiren14Stereo_96kCapCode:               return "Siren14Stereo_96k";
		case eG719_32kCapCode:                        return "G719_32k";
		case eG719_48kCapCode:                        return "G719_48k";
		case eG719_64kCapCode:                        return "G719_64k";
		case eG719_96kCapCode:                        return "G719_96k";
		case eG719_128kCapCode:                       return "G719_128k";
		case eSiren22_32kCapCode:                     return "Siren22_32k";
		case eSiren22_48kCapCode:                     return "Siren22_48k";
		case eSiren22_64kCapCode:                     return "Siren22_64k";
		case eG719Stereo_64kCapCode:                  return "G719Stereo_64k";
		case eG719Stereo_96kCapCode:                  return "G719Stereo_96k";
		case eG719Stereo_128kCapCode:                 return "G719Stereo_128k";
		case eSiren22Stereo_64kCapCode:               return "Siren22Stereo_64k";
		case eSiren22Stereo_96kCapCode:               return "Siren22Stereo_96k";
		case eSiren22Stereo_128kCapCode:              return "Siren22Stereo_128k";
		case eSirenLPR_32kCapCode:                    return "SirenLPR_32k";
		case eSirenLPR_48kCapCode:                    return "SirenLPR_48k";
		case eSirenLPR_64kCapCode:                    return "SirenLPR_64k";
		case eSirenLPRStereo_64kCapCode:              return "SirenLPRStereo_64k";
		case eSirenLPRStereo_96kCapCode:              return "SirenLPRStereo_96k";
		case eSirenLPRStereo_128kCapCode:             return "SirenLPRStereo_128k";
		case eAAC_LDCapCode:                          return "AAC_LD";
		case eSirenLPR_Scalable_32kCapCode:           return "SirenLPR_Scalable_32k";
		case eSirenLPR_Scalable_48kCapCode:           return "SirenLPR_Scalable_48k";
		case eSirenLPR_Scalable_64kCapCode:           return "SirenLPR_Scalable_64k";
		case eSirenLPRStereo_Scalable_64kCapCode:     return "SirenLPRStereo_Scalable_64k";
		case eSirenLPRStereo_Scalable_96kCapCode:     return "SirenLPRStereo_Scalable_96k";
		case eSirenLPRStereo_Scalable_128kCapCode:    return "SirenLPRStereo_Scalable_128k";
		case eiLBC_13kCapCode:                        return "iLBC_13k";
		case eiLBC_15kCapCode:                        return "iLBC_15k";
		case eOpus_CapCode:                        	  return "eOpus_64kCapCode";
		case eOpusStereo_CapCode:                 	  return "eOpusStereo_128kCapCode";
		case eRfc2833DtmfCapCode:                     return "Rfc2833Dtmf";
		case eH261CapCode:                            return "H261";
		case eH262CapCode:                            return "H262";
		case eH263CapCode:                            return "H263";
		case eH264CapCode:                            return "H264";
		case eVP8CapCode :                            return "eVP8CapCode"; //N.A. DEBUG VP8
		case eH26LCapCode:                            return "H26L";
		case eRtvCapCode:                             return "Rtv";
		case eIS11172VideoCapCode:                    return "IS11172Video";
		case eGenericVideoCapCode:                    return "GenericVideo";
		case eSvcCapCode:                             return "SVC";
		case eT120DataCapCode:                        return "T120Data";
		case eAnnexQCapCode:                          return "AnnexQ";
		case eRvFeccCapCode:                          return "RvFecc";
		case eNonStandardCapCode:                     return "NonStandard";
		case eGenericCapCode:                         return "Generic";
		case ePeopleContentCapCode:                   return "PeopleContent";
		case eRoleLabelCapCode:                       return "RoleLabel";
		case eDynamicPTRCapCode:                      return "DynamicPayloadTypeReplacement";
		case eIcePwdCapCode:                          return "IcePwd";
		case eIceUfragCapCode:                        return "IceUfrag";
		case eIceCandidateCapCode:                    return "IceCandidate";
		case eIceRemoteCandidateCapCode:              return "IceRemoteCandidate";
		case eRtcpCapCode:                            return "Rtcp";
		case eSdesCapCode:                            return "SDES";
		case eBFCPCapCode:                            return "BFCP";
		case eMCCFCapCode:                            return "MCCF";
		case eDtlsCapCode:                            return "DTLS";
		case eMsSvcCapCode:                           return "MS_SVC";
		case eH239ControlCapCode:                     return "H239Control";
		case eChairControlCapCode:                    return "ChairControl";
		case eEncryptionCapCode:                      return "Encryption";
		case eDBC2CapCode:                            return "DBC2";
		case eLPRCapCode:                             return "LPR";
		case eUnknownAlgorithemCapCode:               return "UnknownAlgorithem";
		case eFECCapCode:                             return "eFECCapCode";  //LYNC2013_FEC_RED
		case eREDCapCode:                             return "eREDCapCode";  //LYNC2013_FEC_RED
		
		default:
			FTRACEINTO << "CapCode:" << capEnumNum << " - Unknown CapEnum";
			break;
	}

	return "Unknown_CapEnum";
};

//--------------------------------------------------------------------------
const char* ChanneltypeToString(kChanneltype channelType)
{
	switch (channelType)
	{
		case kEmptyChnlType:                      return "kEmptyChnlType";
		case kIpAudioChnlType:                    return "kIpAudioChnlType";
		case kIpVideoChnlType:                    return "kIpVideoChnlType";
		case kIpFeccChnlType:                     return "kIpFeccChnlType";
		case kIpContentChnlType:                  return "kIpContentChnlType";
		case kPstnAudioChnlType:                  return "kPstnAudioChnlType";
		case kRtmChnlType:                        return "kRtmChnlType";
		case kIsdnMuxChnlType:                    return "kIsdnMuxChnlType";
		case kBfcpChnlType:                       return "kBfcpChnlType";
		case kSvcAvcChnlType:                     return "kSvcAvcChnlType";
		case kAvcVSWChnType:                      return "kAvcVSWChnType";
		case kAvcToSacChnlType:                   return "kAvcToSacChnlType";
		case kAvcToSvcChnlType:                   return "kAvcToSvcChnlType";
		case kUnknownChnlType:                    return "kUnknownChnlType";
		
		default:
			FTRACEINTO << "unknown kChanneltype " << channelType;
			break;
	}

	return "Unknown_kChanneltype";
};

//--------------------------------------------------------------------------
const char* CapDataTypeToString(cmCapDataType dataType)
{
	switch (dataType)
	{
	case cmCapEmpty:
		return "Empty";
	case cmCapAudio:
		return "Audio";
	case cmCapVideo:
		return "Video";
	case cmCapData:
		return "Data";
	case cmCapNonStandard:
		return "NonStandard";
	case cmCapUserInput:
		return "UserInput";
	case cmCapConference:
		return "Conference";
	case cmCapH235:
		return "H235";
	case cmCapMaxPendingReplacementFor:
		return "MaxPendingReplacementFor";
	case cmCapGeneric:
		return "Generic";
	case cmCapMultiplexedStream:
		return "MultiplexedStream";
	case cmCapAudioTelephonyEvent:
		return "AudioTelephonyEvent";
	case cmCapAudioTone:
		return "AudioTone";
	case cmCapBfcp:
		return "Bfcp";
	default:
		FTRACEINTO << "unknown cmCapDataType " << dataType;
	}

	return "Unknown";
}


//--------------------------------------------------------------------------
const char* CapDirectionToString(cmCapDirection direction)
{
	switch (direction)
	{
		case 0:                                 return "0";
		case cmCapReceive:                      return "cmCapReceive";
		case cmCapTransmit:                     return "cmCapTransmit";
		case cmCapReceiveAndTransmit:           return "cmCapReceiveAndTransmit";
		case cmCapDirectionMAX:                 return "cmCapDirectionMAX";
		
		default:
			FTRACEINTO << "unknown cmCapDirection " << direction;
			break;
	}

	return "Unknown_cmCapDirection";
};
//--------------------------------------------------------------------------
// TELEPRESENCE_LAYOUTS
const char* TelePresenceLayoutModeToString(ETelePresenceLayoutMode telePresenceLayoutMode)
{
	switch(telePresenceLayoutMode)
	{
	case eTelePresenceLayoutManual: return "eTelePresenceLayoutManual";
	case eTelePresenceLayoutContinuousPresence: return "eTelePresenceLayoutContinuousPresence";
	case eTelePresenceLayoutRoomSwitch: return "eTelePresenceLayoutRoomSwitch";
	case eTelePresenceLayoutCpSpeakerPriority: return "eTelePresenceLayoutCpSpeakerPriority";
	case eTelePresenceLayoutCpParticipantsPriority: return "eTelePresenceLayoutCpParticipantsPriority";
	default: return "Unknown ETelePresenceLayoutMode";
	}
	return "Unknown ETelePresenceLayoutMode";
}
//--------------------------------------------------------------------------
