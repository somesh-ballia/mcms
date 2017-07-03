// SwitchTask.cpp

#include "SwitchTask.h"

#include "Segment.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsShelfMngr.h"
#include "OpcodesMcmsCardMngrMaintenance.h"
#include "MplMcmsProtocol.h"
#include "TraceStream.h"
#include "Trace.h"
#include "ProcessBase.h"
#include "TaskApi.h"
#include "CardsProcess.h"
#include "StatusesGeneral.h"
#include "FaultsDefines.h"
#include "HwMonitoring.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "MplMcmsProtocolTracer.h"
#include "MfaApi.h"
#include "HlogApi.h"
#include "OpcodesMcmsRtmIsdnMaintenance.h"
#include "ManagerApi.h"
#include "EthernetSettingsMonitoring.h"
#include "SlotsNumberingConversionTableWrapper.h"
#include "ConfigManagerApi.h"
#include "ConfigManagerApi.h"
#include "Trace.h"
#include "ConfigManagerOpcodes.h"
#include "SNMPDefines.h"
#include "PrettyTable.h"
#include "AlarmStrTable.h"
#include "McuMngrInternalStructs.h"

extern char* ShelfMngrComponentStatusToString(APIU32 compStatus);
extern const char* VLanEntityToString(eVLanEntityType theType);
extern char* CardTypeToString(APIU32 cardType);
extern char* ProcessTypeToString(eProcessType processType);
extern const char* ShelfMngrComponentTypeToString(APIU32 compType);

const    WORD  STARTUP        = 1;
const    WORD  CONFIGURATION  = 2;
const    WORD  READY          = 3;

#define SWAPL(X) (((X)&0xff)<<24)+(((X)&0xff00)<<8)+(((X)&0xff0000)>>8)+(((X)&0xff000000)>>24)

PBEGIN_MESSAGE_MAP(CSwitchTask)
  ONEVENT(ACK_IND,                   ANYCASE,    CSwitchTask::IgnoreMessage )
  ONEVENT(SM_CARD_MNGR_RECONNECT_IND,	ANYCASE,	CSwitchTask::OnCmReconnectInd )
	ONEVENT(SM_FATAL_FAILURE_IND,		ANYCASE,	CSwitchTask::OnShmFatalFailureInd)
	ONEVENT(SM_LAN_FAILURE_IND,			ANYCASE,	CSwitchTask::OnShmLanFailureInd)
	ONEVENT(SM_KEEP_ALIVE_IND,			ANYCASE,	CSwitchTask::OnShmKeepAliveInd)
	ONEVENT(KEEP_ALIVE_TIMER_RECEIVE,	ANYCASE,	CSwitchTask::OnTimerKeepAliveReceiveTimeout)
	ONEVENT(KEEP_ALIVE_TIMER_SEND,		ANYCASE,	CSwitchTask::OnTimerKeepAliveSendTimeout)
	ONEVENT(SPEC_MFA_ACTIVE_ALARMS_REQ, ANYCASE,    CSwitchTask::OnSpecMfaActiveAlarmReq )
	ONEVENT(ETHERNET_SETTINGS_IND,						ANYCASE,	CSwitchTask::OnEthSettingInd )
	ONEVENT(ETHERNET_SETTINGS_CLEAR_MAX_COUNTERS_IND,	ANYCASE,	CSwitchTask::OnEthSettingClearMaxCountersInd )
	ONEVENT(SLOTS_NUMBERING_CONVERSION_IND, ANYCASE,	CSwitchTask::OnSlotsNumberingConversionTableInd )
	ONEVENT(OLD_SYSCFG_PARAMS_IND, 			ANYCASE,    CSwitchTask::OnOldSysCfgParamsInd )
	ONEVENT(CARDS_NEW_SYS_CFG_PARAMS_IND, 	ANYCASE,    CSwitchTask::OnNewSysCfgParamsInd )
	ONEVENT( IVR_MUSIC_ADD_SOURCE_RECEIVED_IND, ANYCASE,CSwitchTask::IgnoreMessage )
	ONEVENT( CM_UPGRADE_PROGRESS_IND, 		ANYCASE, 	CSwitchTask::OnCmUpgradeProcessInd )
	ONEVENT( CM_UPGRADE_IPMC_IND, 			ANYCASE, 	CSwitchTask::OnCmUpgradeIpmcInd )
	ONEVENT( CM_UPGRADE_IPMC_PROGRESS_IND,	ANYCASE, CSwitchTask::OnCmUpgradeIpmcProcessInd )
	ONEVENT( IPMC_UPGRADE_TIMER,       		ANYCASE, CSwitchTask::OnCmUpgradeIpmcTimer )

	// messgaes that should not arrive to Switch task (they actually arrive due to unknown cardType before sending etc)
	ONEVENT(CARDS_NEW_MEDIA_IP_IND,     	ANYCASE,    CSwitchTask::IgnoreMessage ) // nothing to be done
	ONEVENT( RTM_ISDN_ATTACH_SPAN_MAP_IND,	ANYCASE,	CSwitchTask::IgnoreMessage )
	ONEVENT( RTM_ISDN_DETACH_SPAN_MAP_IND,	ANYCASE,	CSwitchTask::IgnoreMessage )
	ONEVENT( CM_UPGRADE_NEW_VERSION_READY_ACK_IND,	ANYCASE,	CSwitchTask::OnCmUpgradeNewVersionReadyAckInd)
	ONEVENT( HOT_SWAP_RTM_REMOVED,			ANYCASE,	CSwitchTask::IgnoreMessage )
	ONEVENT(CARDS_NEW_MEDIA_IP_IND,     	ANYCASE,    CSwitchTask::IgnoreMessage )
	ONEVENT(BAD_SPONTANEOUS_IND, ANYCASE, CSwitchTask::OnBadSpontaneousInd)
	ONEVENT(IS_USER_EXIST_ON_ACTIVE_DIRECTORY_IND, ANYCASE, CSwitchTask::OnLdapLoginRequest)
PEND_MESSAGE_MAP(CSwitchTask, CAlarmableTask);

void switchEntryPoint(void* appParam)
{
	CSwitchTask * pSwitchTask = new CSwitchTask;
	pSwitchTask->Create(*(CSegment*)appParam);
}

CSwitchTask::CSwitchTask()
{
	TRACESTR(eLevelInfoNormal) << "CSwitchTask - constructor";
	m_pProcess = (CCardsProcess*)CCardsProcess::GetProcess();

	CSysConfig *sysConfig = m_pProcess->GetSysConfig();
	sysConfig->GetDWORDDataByKey("KEEP_ALIVE_RECEIVE_PERIOD", m_keepAliveReceivePeriod);
	m_keepAliveSendPeriod = DEFAULT_KEEP_ALIVE_SEND_PERIOD;

	m_isFirstKeepAliveAlreayTreated		= NO;
	m_isFirstCNRLalreadyTreated         = NO;
	m_isModeDetectionAlreadyTreated     = NO;
	m_numOfkeepAliveTimeoutReached		= 0;
	m_isKeepAliveFailureAlreadyTreated	= NO;
	m_isMpmAlreadyReported				= NO;
	m_lastKeepAliveReqTime.InitDefaults();
	m_lastKeepAliveIndTime.InitDefaults();
	m_isJitcMode		= NO;
	m_resetOnOldJitcInd = NO;
	m_isSeparatedNetworks = NO;
	m_isMultipleServices = NO;

	m_boardId    = (DWORD)this;
	m_subBoardId = (DWORD)this;
	m_isInstalling = NO;
	m_require_ipmc_upgrade = NO;
	m_isInstallingIpmc = NO;
	CreateTaskName();
}

CSwitchTask::~CSwitchTask()
{}

void CSwitchTask::AlarmableTaskCreateEndPoint()
{
	// base: set status to normal.

	// was added here since task_name is set only later (and task_status should be set to Normal
	//   only after task_name is set, in order to get an appropriate task_name in ActiveAlarms)
	// task_state is therefore set in method <CCardsManager::CreateSwitchTask>,
	//   only after <SetBoardId> and <SetSubBoardId> are called (and task_name is set accordingly)
}

//////////////////////////////////////////////////////////////////////
void CSwitchTask::InitTask()
{
    m_state = READY;

	eCardType cardType = m_cardMngr.GetType();
	if ( eSwitch != cardType )
	{
		TRACESTR(eLevelInfoNormal) << "\nCSwitchTask::InitTask - illegal card type on Switch task. Card type: " << cardType;
		PASSERT(1);
		return;
	}

	m_pProcess->GetCardsMonitoringDB()->SetCardState(m_boardId, m_subBoardId, eNormal);
	TRACESTR(eLevelInfoNormal) << "\nCSwitchTask::InitTask"
						   << "\nCard on boardId " << m_boardId << " subBoardId " << m_subBoardId
						   << " (type: " << ::CardTypeToString(cardType) << ")"
						   << " updated its state to Normal";

	// Switch's LAN ports become mounted once the Switch is added
	SetSwitchLanPortsMounted();

	SendSlotsNumberingConversionReqToMplApi();
	SendVLanIdReqToMplApi();
	SendSysCfgParamsToShelfMngr();

	// ===== KeepAlive polling starts
//	InitTimer(*m_pRcvMbx);
//	StartTimer(KEEP_ALIVE_TIMER_SEND, m_keepAliveSendPeriod*SECOND); // no need to wait until sending the 1st KA_req
	OnTimerKeepAliveSendTimeout();


	// remove an unnecessary Alert
	DWORD displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(m_boardId, m_subBoardId);
	if ( true == IsActiveAlarmExistByErrorCodeUserId(AA_POWER_OFF_PROBLEM, displayBoardId) )
	{
		RemoveActiveAlarmSwitchByErrorCodeUserId("CSwitchTask::InitTask", YES, AA_POWER_OFF_PROBLEM, FAULT_CARD_SUBJECT, displayBoardId);
		TRACESTR(eLevelInfoNormal) << "\nCSwitchTask::InitTask - 'old' PowerOff Alert should be removed (the card exists, after all)";
	}
}

//////////////////////////////////////////////////////////////////////
void CSwitchTask::SetSwitchLanPortsMounted()
{
	eProductType  curProductType  = CProcessBase::GetProcess()->GetProductType();
	if (curProductType ==eProductTypeRMX1500)
	{
		m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(FIXED_DISPLAY_BOARD_ID_SWITCH, ETH_SETTINGS_MODEM_PORT_ON_SWITCH_BOARD_RMX1500, true);
		m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(FIXED_DISPLAY_BOARD_ID_SWITCH, ETH_SETTINGS_SHELF_MNGR_PORT_ON_SWITCH_BOARD_RMX1500, true);

	}
	else if (curProductType ==eProductTypeRMX4000)
	{
		m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(FIXED_DISPLAY_BOARD_ID_SWITCH, ETH_SETTINGS_SHELF_MNGR_PORT_ON_SWITCH_BOARD, true);
		m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(FIXED_DISPLAY_BOARD_ID_SWITCH, ETH_SETTINGS_MODEM_PORT_ON_SWITCH_BOARD, true);
	}
	else if (curProductType ==eProductTypeRMX2000)
	{
		m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(FIXED_BOARD_ID_SWITCH,ETH_SETTINGS_NOT_USED_PORT_ON_SWITCH_BOARD_RMX2000, true);
		m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(FIXED_BOARD_ID_SWITCH, ETH_SETTINGS_SHELF_MNGR_PORT_ON_SWITCH_BOARD_RMX2000, true);
		m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(FIXED_BOARD_ID_SWITCH, ETH_SETTINGS_MODEM_PORT_ON_SWITCH_BOARD_RMX2000, true);
	}
}
//////////////////////////////////////////////////////////////////////
void CSwitchTask::SendSlotsNumberingConversionReqToMplApi()
{
	TRACESTR(eLevelInfoNormal) << "\nCSwitchTask::SendSlotsNumberingConversionReqToMplApi";

	CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol;

	mplPrtcl->AddCommonHeader(SLOTS_NUMBERING_CONVERSION_REQ);
	mplPrtcl->AddMessageDescriptionHeader();
	mplPrtcl->AddPhysicalHeader(1, m_boardId, m_subBoardId);

	CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("CARDS_SENDS_TO_MPLAPI");
	mplPrtcl->SendMsgToMplApiCommandDispatcher();

	POBJDELETE(mplPrtcl);
}

//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CSwitchTask::SendVLanIdReqToMplApi()
{
	string retStr = "\nCSwitchTask::SendVLanIdReqToMplApi\n";

	// ===== 1. prepare the struct
	RTM_ISDN_VLAN_ID_S theStruct;
	/*init the memory to avoid the valgrind error*/
	memset(&theStruct,0,sizeof(RTM_ISDN_VLAN_ID_S));
	theStruct.vLan_entityType	= eVLanEntity_Rtm;
	theStruct.vLan_id			= RTM_ISDN_VLAN_ID;

	// ===== 2. print
	PrintRtmIsdnVLanIdStruct(theStruct);

	// ===== 3. send to Switch
	BYTE switchBoardId		= m_pProcess->GetAuthenticationStruct()->switchBoardId,
	     switchSubBoardId	= m_pProcess->GetAuthenticationStruct()->switchSubBoardId;

	SendMessageToSwitch(RTM_ISDN_VLAN_ID_REQ, sizeof(RTM_ISDN_VLAN_ID_S), (char*)&theStruct);
}

void CSwitchTask::SendSysCfgParamsToShelfMngr()
{
	CMedString logStr = "\nCSwitchTask::SendSysCfgParamsToShelfMngr - ";
	logStr << "boardId: " << m_boardId << ", subBoardId: " << m_subBoardId;

	// ===== 1. read & validate SysConfig param
	// 1.1. Read & Validate DEBUG_MODE
	BOOL isDebugMode = FALSE;
    string dataDebugModeStr;
	CSysConfig *sysConfig = m_pProcess->GetSysConfig();
	const CCfgData *cfgData = sysConfig->GetCfgEntryByKey(CFG_KEY_DEBUG_MODE);
	if( false == CCfgData::TestValidity(cfgData) )
	{
	    logStr << "\nFailed to validate sysConfig, key: " << CFG_KEY_DEBUG_MODE;
	    TRACESTR(eLevelError) << logStr.GetString();

	}
	else
	{

	    sysConfig->GetDataByKey(CFG_KEY_DEBUG_MODE, dataDebugModeStr);


	    BOOL res = sysConfig->GetBOOLDataByKey(CFG_KEY_DEBUG_MODE, isDebugMode);

	    if ( (FALSE == res) ||
	            ((CFG_STR_YES != dataDebugModeStr) && (CFG_STR_NO  != dataDebugModeStr)) ) 	// illegal data!
	    {
	        logStr << "\nIllegal DebugMode flag: "
	        << "key - " << CFG_KEY_DEBUG_MODE << ", value: " << dataDebugModeStr.c_str()
	        << ". default value is sent to ShelfMngr!";
	        TRACESTR(eLevelError) << logStr.GetString();

	        isDebugMode = FALSE;
	    }
	}

	// 1.2. Read & Validate JITC_MODE
	BOOL isJITCMode = FALSE;
    string dataJITCModeStr;
	cfgData = sysConfig->GetCfgEntryByKey(CFG_KEY_JITC_MODE);
	if( false == CCfgData::TestValidity(cfgData) )
	{
	    logStr << "\nFailed to validate sysConfig, key: " << CFG_KEY_JITC_MODE;
	    TRACESTR(eLevelError) << logStr.GetString();

	}
	else
	{
	    sysConfig->GetDataByKey(CFG_KEY_JITC_MODE, dataJITCModeStr);


	    BOOL res = sysConfig->GetBOOLDataByKey(CFG_KEY_JITC_MODE, isJITCMode);

	    if ( (FALSE == res) ||
	            ((CFG_STR_YES != dataJITCModeStr) && (CFG_STR_NO  != dataJITCModeStr)) ) 	// illegal data!
	    {
	        logStr << "\nIllegal JITCMode flag: "
	        << "key - " << CFG_KEY_JITC_MODE << ", value: " << dataJITCModeStr.c_str()
	        << ". default value is sent to ShelfMngr!";
	        TRACESTR(eLevelError) << logStr.GetString();

	        isJITCMode = FALSE;
	    }
	}

	m_isJitcMode = isJITCMode;
	m_resetOnOldJitcInd = YES;

	string dataSeparatedNetworksStr;
	sysConfig->GetDataByKey(CFG_KEY_SEPARATE_NETWORK, dataSeparatedNetworksStr);

	BOOL isSeparatedNetworks = NO;
	BOOL res = sysConfig->GetBOOLDataByKey(CFG_KEY_SEPARATE_NETWORK, isSeparatedNetworks);

	if ( (FALSE == res) ||
	        ((CFG_STR_YES != dataSeparatedNetworksStr) && (CFG_STR_NO  != dataSeparatedNetworksStr)) ) 	// illegal data!
	{
	    logStr << "\nIllegal SeparatedNetworks flag: "
	    << "key - " << CFG_KEY_SEPARATE_NETWORK << ", value: " << dataSeparatedNetworksStr.c_str()
	    << ". default value is sent to ShelfMngr!";
	    TRACESTR(eLevelError) << logStr.GetString();

	    isSeparatedNetworks = NO;
	}

	BOOL isMultipleServicesCfgFlag = NO, isMultipleServicesMode = NO;
	sysConfig->GetBOOLDataByKey(CFG_KEY_MULTIPLE_SERVICES, isMultipleServicesCfgFlag);
	if (m_pProcess->GetLicensingStruct()->multipleServices && isMultipleServicesCfgFlag)
	{
	    isMultipleServicesMode = YES;
	    isSeparatedNetworks = NO;
	}

	m_isSeparatedNetworks = isSeparatedNetworks;
	m_isMultipleServices = isMultipleServicesMode;

	string dataReportDSPCrashStr;
	sysConfig->GetDataByKey(CFG_KEY_REPORT_DSP_CRASH, dataReportDSPCrashStr);

	BOOL bReportDSPCrash = NO;
	res = sysConfig->GetBOOLDataByKey(CFG_KEY_REPORT_DSP_CRASH, bReportDSPCrash);

	if ( (FALSE == res) ||
	        ((CFG_STR_YES != dataReportDSPCrashStr) && (CFG_STR_NO  != dataReportDSPCrashStr)) ) 	// illegal data!
	{
	    logStr << "\nIllegal Report DSP Crash flag: "
	    << "key - " << CFG_KEY_REPORT_DSP_CRASH << ", value: " << dataReportDSPCrashStr.c_str()
	    << ". default value is sent to ShelfMngr!";
	    TRACESTR(eLevelError) << logStr.GetString();

	    bReportDSPCrash = NO;
	}
	string dataV35JitcSupportStr;
	sysConfig->GetDataByKey(CFG_KEY_V35_ULTRA_SECURED_SUPPORT, dataV35JitcSupportStr);

	BOOL isV35JitcSupport = NO;
	res = sysConfig->GetBOOLDataByKey(CFG_KEY_V35_ULTRA_SECURED_SUPPORT, isV35JitcSupport);
	if ( (FALSE == res) ||
        ((CFG_STR_YES != dataV35JitcSupportStr) && (CFG_STR_NO  != dataV35JitcSupportStr)) ) 	// illegal data!
	{
	    logStr << "\nIllegal V35_ULTRA_SECURED_SUPPORT flag: "
	    << "key - " << CFG_KEY_V35_ULTRA_SECURED_SUPPORT << ", value: " << dataV35JitcSupportStr.c_str()
	    << ". default value is sent to MPL!";
	    TRACESTR(eLevelError) << logStr.GetString();
		    isV35JitcSupport = NO;
	}


	string dataRTMLanStr;
	BOOL bRTM_LAN = NO;	

	//NGB request
	eSystemCardsMode curMode = m_pProcess->GetSystemCardsModeCur();
	if (curMode == eSystemCardsMode_mpmrx)
		{
		    dataRTMLanStr = CFG_STR_YES;
			bRTM_LAN = YES;
		}
        else
                {
                  sysConfig->GetDataByKey(CFG_KEY_RMX2000_RTM_LAN, dataRTMLanStr);
	res = sysConfig->GetBOOLDataByKey(CFG_KEY_RMX2000_RTM_LAN, bRTM_LAN);

	if ( (FALSE == res) ||
	        ((CFG_STR_YES != dataRTMLanStr) && (CFG_STR_NO  != dataRTMLanStr)) ) 	// illegal data!
	{
	    logStr << "\nIllegal RMX2000_RTM_LAN flag: "
	    << "key - " << CFG_KEY_RMX2000_RTM_LAN << ", value: " << dataRTMLanStr.c_str()
	    << ". Nothing is sent to ShelfMngr!";
	    TRACESTR(eLevelError) << logStr.GetString();

	    bRTM_LAN = NO;

	          }
                }


	std::string dataLanRedundancyStr;
	sysConfig->GetDataByKey(CFG_KEY_LAN_REDUNDANCY, dataLanRedundancyStr);

	BOOL bLAN_REDUNDANCY = NO;
	res = sysConfig->GetBOOLDataByKey(CFG_KEY_LAN_REDUNDANCY, bLAN_REDUNDANCY);

	if ((FALSE == res) ||
	        ((CFG_STR_YES != dataLanRedundancyStr) && (CFG_STR_NO  != dataLanRedundancyStr)))
	{
	    TRACEINTO << "\nIllegal CFG_KEY_LAN_REDUNDANCY flag: "
	    << "key - " << CFG_KEY_LAN_REDUNDANCY
	    << ", value: " << dataLanRedundancyStr.c_str()
	    << ". Default value is sent to ShelfMngr!";

	    bLAN_REDUNDANCY = NO;
	}

	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	if ((eProductTypeGesher == curProductType) || (eProductTypeNinja == curProductType))
	{
		bLAN_REDUNDANCY = CProcessBase::GetProcess()->GetLanRedundancy();
		if (bLAN_REDUNDANCY == YES)
		{
			dataLanRedundancyStr = "YES";
		}
		else
		{
			dataLanRedundancyStr = "NO";
		}
	}

	// Enable TC (Traffic Control) package
	string dataEnableTcPackageStr;
	sysConfig->GetDataByKey(CFG_KEY_ENABLE_TC_PACKAGE, dataEnableTcPackageStr);

	BOOL isEnableTcPackage = NO;
	res = sysConfig->GetBOOLDataByKey(CFG_KEY_ENABLE_TC_PACKAGE, isEnableTcPackage);
	if ( (FALSE == res) ||
	        ((CFG_STR_YES != dataEnableTcPackageStr) && (CFG_STR_NO  != dataEnableTcPackageStr)) ) 	// illegal data!
	{
	    logStr << "\nIllegal EnableTcPackage flag: "
	    << "key - " << CFG_KEY_ENABLE_TC_PACKAGE << ", value: " << dataEnableTcPackageStr.c_str()
	    << ". Default value is sent to ShelfMngr!";
	    TRACESTR(eLevelError) << logStr.GetString();

	    isEnableTcPackage = NO;
	}

	//TC (Traffic Control) latency size
	DWORD dataTcLatencySize;
	res = sysConfig->GetDWORDDataByKey(CFG_KEY_TC_LATENCY_SIZE, dataTcLatencySize);

	if ( (FALSE == res) ||
			(1 > dataTcLatencySize) || (1000  <  dataTcLatencySize) ) 	// illegal data!
	{
	    logStr << "\nIllegal dataTcLatencySize flag: "
	    << "key - " << CFG_KEY_TC_LATENCY_SIZE << ", value: " << dataTcLatencySize
	    << ". Default value is sent to ShelfMngr!";
	    TRACESTR(eLevelError) << logStr.GetString();

	    dataTcLatencySize = 500; //set the default value

	}

	//Tc Burst Size
	DWORD datTcBurstSize;
	res = sysConfig->GetDWORDDataByKey(CFG_KEY_TC_BURST_SIZE, datTcBurstSize);

	if ((FALSE == res) || (30  <  datTcBurstSize)) 	// illegal data!
	{
	    logStr << "\nIllegal datTcBurstSize flag: "
	    << "key - " << CFG_KEY_TC_BURST_SIZE << ", value: " << datTcBurstSize
	    << ". Default value is sent to ShelfMngr!";
	    TRACESTR(eLevelError) << logStr.GetString();

	    datTcBurstSize = 10; //set the default value
	}

	// For Ntp synchronization  VNGR-22433
	DWORD dataNtpLocalServerStratum;

	res = sysConfig->GetDWORDDataByKey(CFG_KEY_NTP_LOCAL_SERVER_STRATUM, dataNtpLocalServerStratum);

	if ( (FALSE == res) ||
	        (1 > dataNtpLocalServerStratum) || (15  <  dataNtpLocalServerStratum) ) 	// illegal data!
	{
	    logStr << "\nIllegal NtpLocalServerStratum flag: "
	    << "key - " << CFG_KEY_NTP_LOCAL_SERVER_STRATUM << ", value: " << dataNtpLocalServerStratum
	    << ". Default value is sent to ShelfMngr!";
	    TRACESTR(eLevelError) << logStr.GetString();

	    dataNtpLocalServerStratum = 14; //set the default value
	}



	string dataMonitoringPacketStr;
	sysConfig->GetDataByKey(CFG_KEY_MONITORING_PACKET, dataMonitoringPacketStr);

	BOOL isMonitoringPacket = YES;
	res = sysConfig->GetBOOLDataByKey(CFG_KEY_MONITORING_PACKET, isMonitoringPacket);
	if ( (FALSE == res) ||
	        ((CFG_STR_YES != dataMonitoringPacketStr) && (CFG_STR_NO  != dataMonitoringPacketStr)) )  // illegal data!
	{
	    logStr << "\nIllegal MonitoringPacket flag: "
	    << "key - " << CFG_KEY_MONITORING_PACKET << ", value: " << dataMonitoringPacketStr.c_str()
	    << ". default value is sent to ShelfMngr!";
	    TRACESTR(eLevelError) << logStr.GetString();

	    isMonitoringPacket = YES;
	}


	string dataIpResponseEchoStr;
	sysConfig->GetDataByKey(CFG_KEY_IP_RESPONSE_ECHO, dataIpResponseEchoStr);

	BOOL response_echo = YES;
	res = sysConfig->GetBOOLDataByKey(CFG_KEY_IP_RESPONSE_ECHO, response_echo);
	if ( (FALSE == res) ||
	        ((CFG_STR_YES != dataIpResponseEchoStr) && (CFG_STR_NO  != dataIpResponseEchoStr)) )  // illegal data!
	{
	    logStr << "\nIllegal IpResponseEcho flag: "
	    << "key - " << CFG_KEY_IP_RESPONSE_ECHO << ", value: " << dataIpResponseEchoStr.c_str()
	    << ". default value is sent to ShelfMngr!";
	    TRACESTR(eLevelError) << logStr.GetString();
	    response_echo = YES;
	}

	string dataCheckArpingStr;
	sysConfig->GetDataByKey(CFG_KEY_DUPLICATE_IP_DETECTION, dataCheckArpingStr);

	BOOL check_arping = YES;
	res = sysConfig->GetBOOLDataByKey(CFG_KEY_DUPLICATE_IP_DETECTION, check_arping);
	if ( (FALSE == res) ||
	        ((CFG_STR_YES != dataCheckArpingStr) && (CFG_STR_NO  != dataCheckArpingStr)) )  // illegal data!
	{
	    logStr << "\nIllegal CheckArping flag: "
	    << "key - " << CFG_KEY_DUPLICATE_IP_DETECTION << ", value: " << dataCheckArpingStr.c_str()
	    << ". default value is sent to ShelfMngr!";
	    TRACESTR(eLevelError) << logStr.GetString();
	    check_arping = YES;
	}


	BOOL disableCellNQ;
	res = sysConfig->GetBOOLDataByKey(CFG_KEY_DISABLE_CELLS_NETWORK_IND, disableCellNQ);
	PASSERTSTREAM_AND_RETURN(!res, "CSysConfig::GetBOOLDataByKey: " << CFG_KEY_DISABLE_CELLS_NETWORK_IND);

	DWORD percentMajorNQ;
	res = sysConfig->GetDWORDDataByKey(CFG_KEY_NETWORK_IND_MAJOR_PERCENTAGE, percentMajorNQ);
	PASSERTSTREAM_AND_RETURN(!res, "CSysConfig::GetDWORDDataByKey: " << CFG_KEY_NETWORK_IND_MAJOR_PERCENTAGE);
	PASSERTSTREAM_AND_RETURN((percentMajorNQ < 1 || percentMajorNQ > 100), "Illegal value for " CFG_KEY_NETWORK_IND_MAJOR_PERCENTAGE);

	DWORD percentCriticalNQ;
	res = sysConfig->GetDWORDDataByKey(CFG_KEY_NETWORK_IND_CRITICAL_PERCENTAGE, percentCriticalNQ);
	PASSERTSTREAM_AND_RETURN(!res, "CSysConfig::GetDWORDDataByKey: " << CFG_KEY_NETWORK_IND_CRITICAL_PERCENTAGE);
	PASSERTSTREAM_AND_RETURN((percentCriticalNQ <= percentMajorNQ || percentCriticalNQ > 100), "Illegal value for " CFG_KEY_NETWORK_IND_CRITICAL_PERCENTAGE);

	BOOL enableSendingIcmpDestinationUnreachable;
	res = sysConfig->GetBOOLDataByKey(CFG_KEY_ENABLE_SENDING_ICMP_DESTINATION_UNREACHABLE, enableSendingIcmpDestinationUnreachable);
	PASSERTSTREAM_AND_RETURN(!res, "CSysConfig::GetBOOLDataByKey: " << CFG_KEY_ENABLE_SENDING_ICMP_DESTINATION_UNREACHABLE);

	BOOL enableAcceptingIcmpRedirect;
	res = sysConfig->GetBOOLDataByKey(CFG_KEY_ENABLE_ACCEPTING_ICMP_REDIRECT, enableAcceptingIcmpRedirect);
	PASSERTSTREAM_AND_RETURN(!res, "CSysConfig::GetBOOLDataByKey: " << CFG_KEY_ENABLE_ACCEPTING_ICMP_REDIRECT);
	// ===== 2. send to Switch
	if(isJITCMode && isSeparatedNetworks && isV35JitcSupport && isMultipleServicesMode)
		isMultipleServicesMode = FALSE;

	//BRIDGE-5887
	if(isJITCMode && isV35JitcSupport)
	{
		dataLanRedundancyStr = "NO";
		bLAN_REDUNDANCY = NO;
	}

	logStr << "\nDebugMode flag: " << dataDebugModeStr.c_str();
	logStr << "\nJITCMode flag: " << dataJITCModeStr.c_str();
	logStr << "\nSeparated networks flag: " << dataSeparatedNetworksStr.c_str();
	logStr << "\nReport DSP crash flag: " << dataReportDSPCrashStr.c_str();
	logStr << "\nMultiple services mode: " << (YES ==  isMultipleServicesMode? "YES" : "NO");
	logStr << "\nV35 JITC support mode: " << (YES ==  isV35JitcSupport? "YES" : "NO");
	logStr << "\nRMX200 RTM LAN flag: " << dataRTMLanStr.c_str();
	logStr << "\nEnable Traffic Control package " << dataEnableTcPackageStr.c_str();
	logStr << "\nTraffic Control latency size " << dataTcLatencySize;
	logStr << "\nTraffic Control Burst Size " << datTcBurstSize;
	logStr << "\nRMX200 LAN REDUNDANCY flag: " << dataLanRedundancyStr.c_str();
	logStr << "\nMonitoring Packet: " << (isMonitoringPacket ? "YES" : "NO");
	logStr << "\n" CFG_KEY_IP_RESPONSE_ECHO ": " << (response_echo ? "YES" : "NO");
	logStr << "\n" CFG_KEY_NTP_LOCAL_SERVER_STRATUM ": " << dataNtpLocalServerStratum;
	logStr << "\n" CFG_KEY_DISABLE_CELLS_NETWORK_IND ": " << (disableCellNQ ? "YES" : "NO");
	logStr << "\n" CFG_KEY_NETWORK_IND_MAJOR_PERCENTAGE ": " << percentMajorNQ;
	logStr << "\n" CFG_KEY_NETWORK_IND_CRITICAL_PERCENTAGE ": " << percentCriticalNQ;
	logStr << "\n" << CFG_KEY_DUPLICATE_IP_DETECTION << ": " << (check_arping ? "YES" : "NO");
	logStr << "\n" << CFG_KEY_ENABLE_SENDING_ICMP_DESTINATION_UNREACHABLE << ": " << (enableSendingIcmpDestinationUnreachable ? "YES" : "NO");
	logStr << "\n" << CFG_KEY_ENABLE_ACCEPTING_ICMP_REDIRECT << ": " << (enableAcceptingIcmpRedirect ? "YES" : "NO");

	SYSCFG_PARAMS_S SysCfgParamsStruct;
	memset(&SysCfgParamsStruct, 0, sizeof SysCfgParamsStruct);

	SysCfgParamsStruct.unDebugMode = isDebugMode;
	SysCfgParamsStruct.unJITCMode = isJITCMode;
	SysCfgParamsStruct.unSeparatedNetworks = isSeparatedNetworks;
	SysCfgParamsStruct.unReportDspCrash = bReportDSPCrash;
	SysCfgParamsStruct.unMultipleServices = isMultipleServicesMode;
	SysCfgParamsStruct.unV35Mode = isV35JitcSupport;
	SysCfgParamsStruct.unRtmLan = bRTM_LAN;
	SysCfgParamsStruct.unLanRedundancy = bLAN_REDUNDANCY;
	SysCfgParamsStruct.unEnableTcPackage = isEnableTcPackage;
	SysCfgParamsStruct.unTcLatencySize = dataTcLatencySize;
	SysCfgParamsStruct.unTcBurstSize = datTcBurstSize;
	SysCfgParamsStruct.unMonitoringPacket = isMonitoringPacket;
	SysCfgParamsStruct.unNtpLocalServerStratum = dataNtpLocalServerStratum;
	SysCfgParamsStruct.unPacketLossMajorValue = percentMajorNQ;
	SysCfgParamsStruct.unPacketLossCriticalValue = percentCriticalNQ;
	//SysCfgParamsStruct.fEnablePacketLossIndication = !(disableSelfNQ && disableCellNQ);
	SysCfgParamsStruct.fEnablePacketLossIndication = YES;
	SysCfgParamsStruct.unIsIpV4ResposeEcho = response_echo;
	SysCfgParamsStruct.unCheckArping = check_arping;
	SysCfgParamsStruct.unEnableAcceptingIcmpRedirect = enableAcceptingIcmpRedirect;
	SysCfgParamsStruct.unEnableSendingIcmpDestinationUnreachable = enableSendingIcmpDestinationUnreachable;

	SendMessageToSwitch(SYSCFG_PARAMS_REQ,
                      sizeof SysCfgParamsStruct,
                      (char*)&SysCfgParamsStruct);

	TRACEINTOFUNC << logStr.GetString();
}

void CSwitchTask::SendMessageToSwitch(const DWORD opCode, const size_t size, const char* pStruct)
{
	CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol();
	mplPrtcl->AddCommonHeader(opCode);
	mplPrtcl->AddMessageDescriptionHeader();
	mplPrtcl->AddPhysicalHeader(1, m_boardId, m_subBoardId);

	if(size > 0 && pStruct)
		mplPrtcl->AddData( size, pStruct );

	CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("CARDS_SENDS_TO_MPLAPI");
	mplPrtcl->SendMsgToMplApiCommandDispatcher();

	POBJDELETE(mplPrtcl);
}
//////////////////////////////////////////////////////////////////////
void CSwitchTask::PrintRtmIsdnVLanIdStruct(const RTM_ISDN_VLAN_ID_S &theStruct)
{
	eVLanEntityType	curEntity = (eVLanEntityType)theStruct.vLan_entityType;

	CLargeString retStr =  "\nCSwitchTask::PrintRtmIsdnVLanIdStruct";
	retStr << "\nBoardId:     "	<< (int)m_boardId
	       << "\nSubBoardId:  " << (int)m_subBoardId
	       << "\nvLan entity: "	<< ::VLanEntityToString(curEntity)
	       << "\nvLan Id:     "	<< theStruct.vLan_id;

    TRACESTR(eLevelInfoNormal) << retStr.GetString();

	return;
}

void CSwitchTask::CreateTaskName()
{
  std::ostringstream buf;
  buf << "SwitchTask (BoardId " << m_boardId << ")";
  m_TaskName = buf.str();
}

void CSwitchTask::OnTimerKeepAliveSendTimeout()
{
	TRACESTR(eLevelInfoNormal) << "\nCSwitchTask::OnTimerKeepAliveSendTimeout";
	SendSmKeepAliveRequestToMplApi();
	StartTimer(KEEP_ALIVE_TIMER_RECEIVE, m_keepAliveReceivePeriod*SECOND);
}

//////////////////////////////////////////////////////////////////////
void CSwitchTask::SendSmKeepAliveRequestToMplApi()
{
	SystemGetTime(m_lastKeepAliveReqTime);

	SendMessageToSwitch(SM_KEEP_ALIVE_REQ, 0, NULL);
}

void CSwitchTask::OnCmReconnectInd(CSegment* pSeg)
{
	// ===== 1. logger
	TRACESTR(eLevelInfoNormal)
		<< "\nCSwitchTask::OnCmReconnectInd -"
		<< "\nSwitch Card Manger Reconnected. boardId: " << m_boardId << ", SubBoardId: " << m_subBoardId;

	// ===== 2. fault
	CHlogApi::CmReconnect(m_boardId, m_subBoardId, FAULT_TYPE_SWITCH);
}

//////////////////////////////////////////////////////////////////////
void CSwitchTask::OnCmUpgradeNewVersionReadyAckInd(CSegment* pSeg)
{

	TRACESTR(eLevelInfoNormal)
	<< "\nCSwitchTask::OnCmUpgradeNewVersionReadyAckInd -pSeg-m_size " << pSeg->GetLen();

	if (pSeg->GetLen() != 0)
	{
	    DWORD  structLen = sizeof(UPGRADE_NEW_VERSION_READY_IND_S);
	    UPGRADE_NEW_VERSION_READY_IND_S rStruct;
	    memset(&rStruct,0,structLen);
	    pSeg->Get( (BYTE*)(&rStruct), structLen );

	    TRACESTR(eLevelInfoHigh)
	    << "\nCSwitchTask::OnCmUpgradeNewVersionReadyAckInd -status " << rStruct.status ;

	    if (rStruct.status == MOUNT_NOT_CONNECTED)
	    {
	        TRACESTR(eLevelInfoHigh)
	        << "\nCSwitchTask::OnCmUpgradeNewVersionReadyAckInd -"
	        << "\nSWITCH Card failed in mount. boardId: " << m_boardId << ", SubBoardId: " << m_subBoardId;

	        m_pProcess->DecBoardIpmcIndCounter(m_boardId);
	        m_pProcess->DecearseBoardInstallingIpmc(m_boardId);

	        //Block further communication with this card
	        CConfigManagerApi apiConfigurator;
	        apiConfigurator.AddDropRuleToShorewall(SWITCH_IP_ADDRESS);

	        return;
	    }

	}

	if (m_isInstalling)
	{
		// CM_UPGRADE_NEW_VERSION_READY_ACK_IND received twice
		PASSERT(1);
	}
	else
	{
		char message[100];
		eProductType  curProductType  = CProcessBase::GetProcess()->GetProductType();
		if (curProductType !=eProductTypeRMX1500)
			sprintf(message,"RTM IP software upgrade 0%% board Id:%d",m_boardId );
		else
			sprintf(message,"RTM IP software upgrade 0%%" );

		m_require_ipmc_upgrade = NO;
		AddActiveAlarm(FAULT_GENERAL_SUBJECT,
				AA_VERSION_INSTALL_PROGRESS,
				SYSTEM_MESSAGE,
				message,
				true,
				true,
				0xFFFFFFFF,
				m_boardId);


		m_isInstalling = YES;

		m_pProcess->AddBoardInstalling();
		m_pProcess->UpdateSoftwareProgress(m_boardId,0);
	}
}

//////////////////////////////////////////////////////////////////////
void CSwitchTask::OnCmUpgradeProcessInd(CSegment* pSeg)
{

	DWORD  structLen = sizeof(UPGRADE_PROGRESS_IND_S);
	UPGRADE_PROGRESS_IND_S rStruct;
	memset(&rStruct,0,structLen);
	pSeg->Get( (BYTE*)(&rStruct), structLen );
	int p = rStruct.progress_precents;
	PASSERT(p > 100);
	char message[100];
	sprintf(message,"RTM IP software upgrade %d%% board Id:%d",p,m_boardId );

	TRACESTR(eLevelInfoNormal) << "CSwitchTask::OnCmUpgradeProcessInd boardId: " << m_boardId << " %" << p;
	if (m_isInstalling)
	{
		UpdateActiveAlarmDescriptionByErrorCode(AA_VERSION_INSTALL_PROGRESS,
				message);
		m_pProcess->UpdateSoftwareProgress(m_boardId,p);
	}
	else
	{
	  // CM_UPGRADE_NEW_VERSION_READY_ACK_IND was never received
	  PASSERT(1);
	}

}

//////////////////////////////////////////////////////////////////////
void CSwitchTask::OnCmUpgradeIpmcInd(CSegment* pSeg)
{

	DWORD  structLen = sizeof(UPGRADE_IPMC_IND_S);
	UPGRADE_IPMC_IND_S rStruct;
	memset(&rStruct,0,structLen);
	pSeg->Get( (BYTE*)(&rStruct), structLen );



	TRACESTR(eLevelInfoNormal) << "CSwitchTask::OnCmUpgradeIpmcInd boardId: " << m_boardId
	<< "require IPMC upgrade: "  << rStruct.require_ipmc_upgrade;
	if (m_isInstalling)
	{
		m_isInstalling = NO;
		RemoveActiveAlarmByErrorCode(AA_VERSION_INSTALL_PROGRESS);
		m_require_ipmc_upgrade = rStruct.require_ipmc_upgrade;
		m_pProcess->DecearseBoardInstalling();
		m_pProcess->DecBoardIpmcIndCounter(m_boardId);

		if (m_require_ipmc_upgrade)
		{
			m_pProcess->AddBoardInstallingIpmc(m_boardId);
			//VNGR-23162
			CConfigManagerApi apiConfigurator;
			apiConfigurator.AddDropRuleToShorewall(SWITCH_IP_ADDRESS,TRUE); // we drop only ping
		}
		else
		{
			m_pProcess->DecearseBoardInstallingIpmc(m_boardId);
			//Block further communication with this card
			CConfigManagerApi apiConfigurator;
			apiConfigurator.AddDropRuleToShorewall(SWITCH_IP_ADDRESS);
		}
	}
	else
	{
		PASSERT(1);
	}

}

void CSwitchTask::OnCmUpgradeIpmcProcessInd(CSegment* pSeg)
{

    DWORD  structLen = sizeof(UPGRADE_PROGRESS_IND_S);
    UPGRADE_PROGRESS_IND_S rStruct;
    memset(&rStruct,0,structLen);
    pSeg->Get( (BYTE*)(&rStruct), structLen );

    int p = rStruct.progress_precents;
    PASSERT(p > 100);
    char message[100];
    eProductType  curProductType  = CProcessBase::GetProcess()->GetProductType();
    if (curProductType !=eProductTypeRMX1500)
       sprintf(message,"RTM IP IPMC upgrade %d%% board Id:%d",p,m_boardId);
    else
    	 sprintf(message,"RTM IP IPMC upgrade %d%%",p);

    int seconds = 10;
    TRACESTR(eLevelInfoNormal) << "CSwitchTask::OnCmUpgradeIpmcProcessInd boardId: " << m_boardId << " %" << p;

    if(m_require_ipmc_upgrade)
    {
    	if (m_isInstallingIpmc)
    	{
    		UpdateActiveAlarmDescriptionByErrorCode(AA_VERSION_IPMC_INSTALL_PROGRESS,
    				message);

    		m_pProcess->UpdateIpmcProgress(m_boardId,p);

    		StartTimer( IPMC_UPGRADE_TIMER, seconds*SECOND);
    	}
    	else
    	{

    		AddActiveAlarm(FAULT_GENERAL_SUBJECT,
    				AA_VERSION_IPMC_INSTALL_PROGRESS,
    				SYSTEM_MESSAGE,
    				message,
    				true,
    				true,
    				0xFFFFFFFF,
    				m_boardId);

    		StartTimer( IPMC_UPGRADE_TIMER, seconds*SECOND);
    		m_pProcess->UpdateIpmcProgress(m_boardId,p);
    		m_isInstallingIpmc = YES;
    	}
    }
    else
    {
    	PASSERT(1);
    }
}

////////////////////////////////////////////////////////////////////
void CSwitchTask::OnCmUpgradeIpmcTimer(CSegment* pSeg)
{

	TRACESTR(eLevelInfoNormal) << "CSwitchTask::OnCmUpgradeIpmcTimer  boardId: " << m_boardId;

	if (m_isInstallingIpmc)
	{
		m_isInstallingIpmc = NO;
		RemoveActiveAlarmByErrorCode(AA_VERSION_IPMC_INSTALL_PROGRESS);
		m_pProcess->DecearseBoardInstallingIpmc(m_boardId);
		m_require_ipmc_upgrade = NO;

		//Block further communication with this card
		CConfigManagerApi apiConfigurator;
		apiConfigurator.AddDropRuleToShorewall(SWITCH_IP_ADDRESS);

		//PASSERT(2);

		// update installer here
	}
	else
	{
		PASSERT(1);
	}

}

void CSwitchTask::OnShmFatalFailureInd(CSegment* pSeg)
{
	 SLOTS_CONFIGURATION_S mfaBoardCnfArray[MAX_NUM_OF_SLOTS];
	 memset(mfaBoardCnfArray, 0, MAX_NUM_OF_SLOTS*sizeof(SLOTS_CONFIGURATION_S));

	// ===== 1. fill object with data from structure received
	DWORD  structLen = sizeof(SWITCH_SM_KEEP_ALIVE_S);
	SWITCH_SM_KEEP_ALIVE_S fatalFailureStruct;
	memset(&fatalFailureStruct,0,structLen);
	pSeg->Get( (BYTE*)(&fatalFailureStruct), structLen );

	// ===== 2. print failure's data
	PrintSmFatalFailureToTrace(fatalFailureStruct);


	// ----- 3. treat RtmLan and RtmIsdn slots


	if (	 m_isModeDetectionAlreadyTreated == NO)
	            RetrieveBasicInfoFromKA(fatalFailureStruct);

	// ===== 4. treat the failures on each unit
	TreatAllComponentsOnSmFatalFailure(fatalFailureStruct,mfaBoardCnfArray);

	eProductFamily curProductFamily = CProcessBase::GetProcess()->GetProductFamily();

	if (eProductFamilySoftMcu != curProductFamily )
	{

		MountRtmShmComponent(mfaBoardCnfArray);

		//HandleRtmIsdnOrLanSlots(fatalFailureStruct,FALSE);

		SendRtmLanIsdnParamsToProcesses(mfaBoardCnfArray);   //added by Rachel Cohen 8.1.14
	}
}

//////////////////////////////////////////////////////////////////////
void CSwitchTask::OnShmLanFailureInd(CSegment* pSeg)
{
	// ===== 1. fill object with data from structure received
	DWORD  structLen = sizeof(SM_COMPONENT_STATUS_S);
	SM_COMPONENT_STATUS_S lanFailureStruct;
	memset(&lanFailureStruct,0,structLen);
	pSeg->Get( (BYTE*)(&lanFailureStruct), structLen );

	// ===== 2. print lan failure's data
	CLargeString dataStr = "\nCSwitchTask::OnShmLanFailureInd";
	dataStr             << "\n==============================\n"
	                    << "slot id: "        << lanFailureStruct.unSlotId
	                    << "sub board id: "   << lanFailureStruct.unSubBoardId
	                    << ", status: "       << lanFailureStruct.unStatus
	                    << ", bitmask: "      << lanFailureStruct.unStatusDescriptionBitmask;
	if (eSmComponentNotExist != lanFailureStruct.unStatus)
	{
		dataStr << ", component name: " << (char*)(lanFailureStruct.sSmCompName);
	}
	TRACESTR(eLevelInfoNormal) << dataStr.GetString();

	// ===== 3. treat the failures
	TreatFailureInSmUnit_nonMfa(eShmComp_Lan, &lanFailureStruct);
}
//////////////////////////////////////////////////////////////////////
void CSwitchTask::TreatAllComponentsOnSmFatalFailure(SWITCH_SM_KEEP_ALIVE_S & fatalFailureStruct ,SLOTS_CONFIGURATION_S mfaBoardCnfArray[])
{
    bool needCheckingMissingRtm = false;

	SM_COMPONENT_STATUS_S *fatalFailureStructPtr = (SM_COMPONENT_STATUS_S *)&fatalFailureStruct;

	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();



	//initiate mfaBoardCnfArray  array


	for (int i=0 ; i< MAX_NUM_OF_SLOTS ;i++)
	{

		mfaBoardCnfArray[i].unStatus  = eSmComponentNotExist;
		mfaBoardCnfArray[i].mediaCompType = eShmComp_Illegal;
		mfaBoardCnfArray[i].rtmCompType = eShmComp_Illegal;

	}



	// we need to pass over all components to check if there is ac case of MPM without a Lan card.
	//to do that we use mfaBoardCnfArray[] that keep data per slotid.
	for (int i=0 ; i< MAX_NUM_OF_SLOTS ;i++,fatalFailureStructPtr++)
	{

		eShmComponentType compType = ConvertShelfMngrStringToSpecificCompType( (char*)(fatalFailureStructPtr->sSmCompName) );



			needCheckingMissingRtm = true;
			//START print for debug can be removed - Rachel Cohen
			if ( eShmComp_Illegal == compType)
			{
				CLargeString dataStr =   "\nCSwitchTask::TreatAllComponentsOnSmFatalFailure - illegal component";
				dataStr	<< "\nslot id: "        << fatalFailureStructPtr->unSlotId
				        << ", sub board id: "   << fatalFailureStructPtr->unSubBoardId
				        << ", status: "         << fatalFailureStructPtr->unStatus
				        << ", bitmask: "        << fatalFailureStructPtr->unStatusDescriptionBitmask
				        << ", component name: " << (char*)(fatalFailureStructPtr->sSmCompName);
				TRACESTR(eLevelInfoNormal) << dataStr.GetString();

				continue;
				//PASSERT(1);
			}
			else{

				CLargeString dataStr =   "\nCSwitchTask::TreatAllComponentsOnSmFatalFailure - GOOD component";
								dataStr	<< "\nslot id: "        << fatalFailureStructPtr->unSlotId
								        << ", sub board id: "   << fatalFailureStructPtr->unSubBoardId
								        << ", status: "         << fatalFailureStructPtr->unStatus
								        << ", bitmask: "        << fatalFailureStructPtr->unStatusDescriptionBitmask
								        << ", component name: " << (char*)(fatalFailureStructPtr->sSmCompName);
								TRACESTR(eLevelInfoNormal) << dataStr.GetString();
			}
			// END print for debug


			switch (compType){

			 case eShmComp_MfaBoard1:
			 case eShmComp_MfaMpmx:
			 case eShmComp_MfaMpmRx:

				  mfaBoardCnfArray[fatalFailureStructPtr->unSlotId].unSubBoardId = fatalFailureStructPtr ->unSubBoardId;
				  mfaBoardCnfArray[fatalFailureStructPtr->unSlotId].unStatus = fatalFailureStructPtr ->unStatus;
				  mfaBoardCnfArray[fatalFailureStructPtr->unSlotId].unStatusDescriptionBitmask = fatalFailureStructPtr ->unStatusDescriptionBitmask;
				  mfaBoardCnfArray[fatalFailureStructPtr->unSlotId].mediaCompType = compType;

				  if ((fatalFailureStructPtr ->unStatus == eSmComponentNotExist) && (fatalFailureStructPtr ->unStatusDescriptionBitmask & SM_COMP_POWER_OFF_BITMASK))
					 {

						//case of hot swap When card is pushed in ( RTM power OFF -> power ON )

					  fatalFailureStructPtr ->unStatus = eSmComponentOk;
					  fatalFailureStructPtr ->unStatusDescriptionBitmask = SM_COMP_CLEAR_BITMASK;
					  mfaBoardCnfArray[fatalFailureStructPtr->unSlotId].unStatus= eSmComponentOk;
					  mfaBoardCnfArray[fatalFailureStructPtr->unSlotId].unStatusDescriptionBitmask = SM_COMP_CLEAR_BITMASK;
					 }

				  break;
			 case eShmComp_RtmLan:
			 case eShmComp_RtmLan4:
			 case eShmComp_RtmLan4_10G:
				 // Dotan embedded
				 // When getting a HotSwap event, the following will be send in one shot:
				 // When card is pushed in ( RTM power OFF -> power ON ) :  status field will set to eSmComponentNotExist (= 2 ) with bitmask 0x20
				 // When card is pushed out (ON -> OFF ) : status field will set to eSmComponentOk (= 0 ) with bitmask 0x20


				  if ((fatalFailureStructPtr ->unStatus == eSmComponentOk) && (!(fatalFailureStructPtr ->unStatusDescriptionBitmask & SM_COMP_POWER_OFF_BITMASK)))
				     {    // it means card is in and status ok
                          //no need for active alarm.TreatSpecificComponent() will treat it.
					      mfaBoardCnfArray[fatalFailureStructPtr->unSlotId].rtmCompType = compType;
						  break;

				     }


				  if ((fatalFailureStructPtr ->unStatus == eSmComponentNotExist) && (fatalFailureStructPtr ->unStatusDescriptionBitmask & SM_COMP_POWER_OFF_BITMASK))
				     {
					  //TreatHotSwapFailureInSmUnit_mfa(i,mfaBoardCnfArray[i].unSubBoardId);
					  //case of hot swap When card is pushed in ( RTM power OFF -> power ON )
					  //no need for active alarm.TreatSpecificComponent() will treat it.
					  fatalFailureStructPtr ->unStatus = eSmComponentOk;
					  fatalFailureStructPtr ->unStatusDescriptionBitmask = SM_COMP_CLEAR_BITMASK;
					  mfaBoardCnfArray[fatalFailureStructPtr->unSlotId].rtmCompType = compType;
				     }



			      break;
			 case eShmComp_RtmIsdn:
			 case eShmComp_RtmIsdn9:
			 case eShmComp_RtmIsdn9_10G:
				 //  Dotan embedded
					 // When getting a HotSwap event, the following will be send in one shot:
					 // When card is pushed in ( RTM power OFF -> power ON ) :  status field will set to eSmComponentNotExist (= 2 ) with bitmask 0x20
					 // When card is pushed out (ON -> OFF ) : status field will set to eSmComponentOk (= 0 ) with bitmask 0x20
				 mfaBoardCnfArray[fatalFailureStructPtr->unSlotId].rtmCompType = compType;

				 if ((fatalFailureStructPtr ->unStatus == eSmComponentOk) && !(fatalFailureStructPtr ->unStatusDescriptionBitmask & SM_COMP_POWER_OFF_BITMASK))
				 {

					 mfaBoardCnfArray[fatalFailureStructPtr->unSlotId].rtmCompType = compType;
					 break;
				 }


				  if ((fatalFailureStructPtr ->unStatus == eSmComponentNotExist) && (fatalFailureStructPtr ->unStatusDescriptionBitmask & SM_COMP_POWER_OFF_BITMASK))
				     {
					  //TreatHotSwapFailureInSmUnit_mfa(i,mfaBoardCnfArray[i].unSubBoardId);
					  //case of hot swap When card is pushed in ( RTM power OFF -> power ON )
					  //fatalFailureStructPtr ->unStatus = eSmComponentOk;
					  //fatalFailureStructPtr ->unStatusDescriptionBitmask = SM_COMP_CLEAR_BITMASK;
					  mfaBoardCnfArray[fatalFailureStructPtr->unSlotId].rtmCompType = compType;
				     }


				  break;
			 default:
				  break;
			}



		TreatSpecificComponent(fatalFailureStructPtr);

	}

	//check the deviceCnfArray to detect RTM_LAN or RTM_ISDN missing problem
	if (needCheckingMissingRtm){

		for (int i=0 ; i< MAX_NUM_OF_SLOTS ;i++ ){

			if  ( ((mfaBoardCnfArray[i].unStatus  == eSmComponentOk ) || (mfaBoardCnfArray[i].unStatus  == eSmComponentResetting )) && ( mfaBoardCnfArray[i].unStatusDescriptionBitmask == SM_COMP_CLEAR_BITMASK))
			{

				if (mfaBoardCnfArray[i].rtmCompType == eShmComp_Illegal )
				{
					if (eProductTypeRMX4000 == curProductType  || ((eProductTypeRMX2000 == curProductType) && (mfaBoardCnfArray[i].mediaCompType == eShmComp_MfaMpmRx)))
				        TreatRtmOrIsdnFailureInSmUnit_mfa(i,mfaBoardCnfArray[i].unSubBoardId,true);
				}
				else
					 TreatRtmOrIsdnFailureInSmUnit_mfa(i,mfaBoardCnfArray[i].unSubBoardId,false);
			}


		}


	}
}

void CSwitchTask::TreatSpecificComponent(SM_COMPONENT_STATUS_S * statusStruct )
{
        eShmComponentType compType = ConvertShelfMngrStringToCompType( (char*)(statusStruct->sSmCompName) );


		/*****************************************************************************************************/
	/* 23.7.10 VNGR - 16358 fixed by Rachel Cohen                                                        */
	/* in case it is a rtm_isdn component with compStatus->unStatus = eSmComponentNotExist &&            */
	/* unStatusDescriptionBitmask & SM_COMP_POWER_OFF_BITMASK that mean card was pushed in               */
	/* and we do not want to return.                                                                     */
	/*****************************************************************************************************/
	if (( eSmComponentNotExist == statusStruct->unStatus) && (compType !=eShmComp_RtmIsdn))
		return;

	if ( eShmComp_Illegal == compType)
	{
		CLargeString dataStr =   "\nCSwitchTask::TreatSpecificComponent - illegal component";
		dataStr	<< "\nslot id: "        << statusStruct->unSlotId
		        << ", sub board id: "   << statusStruct->unSubBoardId
		        << ", status: "         << statusStruct->unStatus
		        << ", bitmask: "        << statusStruct->unStatusDescriptionBitmask
		        << ", component name: " << (char*)(statusStruct->sSmCompName);
		TRACESTR(eLevelInfoNormal) << dataStr.GetString();

		PASSERT(1);
	}
	else
	{
		if ( TRUE == IsMpmOrMpmPlusSmComponent(compType) )
			TreatFailureInSmUnit_mfa(compType, statusStruct);
	    else
	    	TreatFailureInSmUnit_nonMfa(compType, statusStruct);
	}
}

void CSwitchTask::TreatRtmOrIsdnFailureInSmUnit_mfa(int slotid,int subboardid,bool IsRTMorISDNmissing)
{

    COsQueue* queue = m_pProcess->GetMfaMbx(slotid, subboardid);
    DWORD displayMfaBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(slotid, RTM_ISDN_SUBBOARD_ID);

	if (!queue) // queue not exist - Switch treats the messege
		{
  	   	 TRACESTR(eLevelInfoNormal) << "\nCSwitchTask::TreatRtmOrIsdnFailureInSmUnit_mfa - No Mbx for BoardId: " << slotid
   	   			                        <<"displayBoardId"<<displayMfaBoardId;


          if (IsRTMorISDNmissing==true)  //we recognized a missing rtm problem
          {

        	  if(false == IsActiveAlarmExistByErrorCodeUserId(AA_RTM_LAN_OR_ISDN_MISSING, displayMfaBoardId))
	          {

          		 TRACESTR(eLevelInfoNormal) << "\nCSwitchTask::TreatRtmOrIsdnFailureInSmUnit_mfa - add active alarm on slot id: " << slotid
          		        	   			                        <<"displayBoardId"<<displayMfaBoardId;


               AddActiveAlarmSwitch("TreatRtmOrIsdnFailureInSmUnit_mfa",
                                    NO,
                                    FAULT_CARD_SUBJECT,
		                            AA_RTM_LAN_OR_ISDN_MISSING,
                                    MAJOR_ERROR_LEVEL,
                                    GetAlarmDescription(AA_RTM_LAN_OR_ISDN_MISSING),
                                    true,
                                    true,
                                    displayMfaBoardId,
                                    displayMfaBoardId,
                                    FAULT_TYPE_MPM);//change the string
	           }
          }
          else
          {
     		 TRACESTR(eLevelInfoNormal) << "\nCSwitchTask::TreatRtmOrIsdnFailureInSmUnit_mfa - remove active alarm on slot id: " << slotid
     		        	   			                        <<"displayBoardId"<<displayMfaBoardId;
     		 RemoveActiveAlarmSwitchByErrorCodeUserId("TreatRtmOrIsdnFailureInSmUnit_mfa",
     						                                  NO,
     						                                  FAULT_CARD_SUBJECT,
     						                                  AA_RTM_LAN_OR_ISDN_MISSING,
     						                                  displayMfaBoardId);

          }
		}
	else// queue ok - let Mfa treat the messege
		{

     	  if (IsRTMorISDNmissing==true)  //we recognized a missing rtm problem
    	  {
     		 TRACESTR(eLevelInfoNormal) << "\nCSwitchTask::TreatRtmOrIsdnFailureInSmUnit_mfa - send to MFA add active alarm on slot id: " << slotid
      		        	   			                        <<"displayBoardId"<<displayMfaBoardId;

		    //prepare the data to be sent
		    CSegment* pSegToBoard = new CSegment;
		    *pSegToBoard << (DWORD)eSmComponentMajor;
		    *pSegToBoard << (DWORD)SM_COMP_RTM_OR_ISDN_MISSING_BITMASK;
		    *pSegToBoard << (DWORD)slotid;

		     // send the message to MFA
		     CMfaApi mfaApi;
		     mfaApi.CreateOnlyApi(*queue);
		     mfaApi.SendMsg(pSegToBoard, SM_MFA_FAILURE_IND);
    	  }
    	  else
    	  {
    	   		 TRACESTR(eLevelInfoNormal) << "\nCSwitchTask::TreatRtmOrIsdnFailureInSmUnit_mfa - send to MFA remove active alarm on slot id: " << slotid
    	      		        	   			                        <<"displayBoardId"<<displayMfaBoardId;

  		    //prepare the data to be sent
  		    CSegment* pSegToBoard = new CSegment;
  		    *pSegToBoard << (DWORD)eSmComponentOk;
  		    *pSegToBoard << (DWORD)SM_COMP_RTM_OR_ISDN_MISSING_BITMASK;
  		    *pSegToBoard << (DWORD)RTM_ISDN_SUBBOARD_ID;

  		     // send the message to MFA
  		     CMfaApi mfaApi;
  		     mfaApi.CreateOnlyApi(*queue);
  		     mfaApi.SendMsg(pSegToBoard, SM_MFA_FAILURE_IND);

    	  }
	    }
}

void CSwitchTask::PrintSmFatalFailureToTrace(SWITCH_SM_KEEP_ALIVE_S& failure)
{
  SM_COMPONENT_STATUS_S* ptr = (SM_COMPONENT_STATUS_S*)&failure;

  CPrettyTable<int, const char*> tbl("id", "state");
  tbl.SetCaption("boards status");

  for (int i = 0; i < MAX_NUM_OF_SLOTS; i++)
  {
    char buf[ONE_LINE_BUFFER_LEN];
    PrintSmSpecificCompFailure(ptr + i, buf);
    tbl.Add(i, buf);
  }

  TRACEINTO << tbl.Get();

  // Updates data regardless SNMP enableness.
  // Possible values are disabled(1), ok(2), failed(3).
  unsigned int fan         = 2,
               power       = 2,
               board       = 2,
               overall     = 2,
               temperature = 2;

  for (int j = 0; j < MAX_NUM_OF_SLOTS; j++)
  {
    // Omits all statuses besides Major.
    if (eSmComponentMajor != ptr[j].unStatus)
      continue;

    if (SM_COMP_POWER_OFF_BITMASK & ptr[j].unStatusDescriptionBitmask)
      continue;

    overall = 3;

    if (0 == strcmp(reinterpret_cast<const char*>(ptr[j].sSmCompName), "MPM"))
      board = 3;
    else if (0 == strcmp(reinterpret_cast<const char*>(ptr[j].sSmCompName), "FANS"))
      fan = 3;
    else if (0 == strcmp(reinterpret_cast<const char*>(ptr[j].sSmCompName), "PWR"))
      power = 3;
    else if (SM_COMP_TEMPERATURE_CRITICAL_BITMASK & ptr[j].unStatusDescriptionBitmask ||
             SM_COMP_TEMPERATURE_MAJOR_BITMASK    & ptr[j].unStatusDescriptionBitmask)
      temperature = 3;
  }

  CSegment* seg = new CSegment;
  *seg << 5u  // Number of parameters.
       << static_cast<unsigned int>(eTT_HardwareFanStatus)             << fan
       << static_cast<unsigned int>(eTT_HardwarePowerSupplyStatus)     << power
       << static_cast<unsigned int>(eTT_HardwareOverallStatus)         << overall
       << static_cast<unsigned int>(eTT_HardwareIntegratedBoardStatus) << board
       << static_cast<unsigned int>(eTT_HardwareChassisTempStatus)     << temperature;

  CManagerApi(eProcessSNMPProcess).SendMsg(seg, SNMP_UPDATE_MULTIPLE_TELEMETRY_DATA_IND);
}

void CSwitchTask::PrintSmSpecificCompFailure(SM_COMPONENT_STATUS_S* failure,
                                             char* out)
{
  CSmallString statusStr;
  char* tmpStr = ::ShelfMngrComponentStatusToString(failure->unStatus);
  if (tmpStr)
      statusStr << tmpStr;
  else
      statusStr << "(invalid: " << failure->unStatus << ")";
 
  // Checks the bit mask - has been added for hot swap.
  if (eSmComponentNotExist != failure->unStatus ||
      failure->unStatusDescriptionBitmask & SM_COMP_POWER_OFF_BITMASK)
  {
      sprintf(out,
              "slot id:%d, sub board id:%d, status:%s, bitmask:0x%x, name:%s",
              failure->unSlotId,
              failure->unSubBoardId,
              statusStr.GetString(),
              failure->unStatusDescriptionBitmask,
              (char*)failure->sSmCompName);
  }
  else
  {
      sprintf(out,
              "slot id:%d, sub board id:%d, status:%s, bitmask:0x%x",
              failure->unSlotId,
              failure->unSubBoardId,
              statusStr.GetString(),
              failure->unStatusDescriptionBitmask);
  }
}

void CSwitchTask::PrintSmLanFailureToTrace(SM_COMPONENT_STATUS_S lanFailureStruct)
{
	CMedString dataStr = "\nCSwitchTask::PrintSmLanFailureToTrace";
	dataStr           << "\n=====================================\n";
	dataStr << "Lan status: " << lanFailureStruct.unStatus << ", bitmask: " << lanFailureStruct.unStatusDescriptionBitmask << "\n";
	TRACESTR(eLevelInfoNormal) << dataStr.GetString();
}

void CSwitchTask::OnTimerKeepAliveReceiveTimeout()
{

	CMedString dataStr = "CSwitchTask::OnTimerKeepAliveReceiveTimeout, m_numOfkeepAliveTimeoutReached = ";

	if ( SM_KEEP_ALIVE_RETRIES > m_numOfkeepAliveTimeoutReached )
	{
		m_numOfkeepAliveTimeoutReached++;
		dataStr           << m_numOfkeepAliveTimeoutReached;
		TRACESTR(eLevelInfoNormal) << dataStr.GetString();
	}
	else
	{
		dataStr           << m_numOfkeepAliveTimeoutReached;
		TRACESTR(eLevelInfoNormal) << dataStr.GetString();
		if ( NO == m_isKeepAliveFailureAlreadyTreated )
		{
			m_isKeepAliveFailureAlreadyTreated = YES;
			if (!m_isInstalling && !m_isInstallingIpmc) // skip this flow during switch ipmc upgrade
			{
			  TreatSmKeepAliveFailure();
			  return;
			}
		}
	}

	// and anyway, keep sending KeepAlive requests
	OnTimerKeepAliveSendTimeout();
}
//////////////////////////////////////////////////////////////////////
void CSwitchTask::OnShmKeepAliveInd(CSegment* pSeg)
{
//	TRACESTR(eLevelInfoNormal) << "\nCSwitchTask::OnShmKeepAliveInd";

	SystemGetTime(m_lastKeepAliveIndTime);

	// ===== 1. print (if periodTrace==YES)
	BOOL isPeriodTrace = FALSE;
	m_pProcess->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_PERIOD_TRACE, isPeriodTrace);
	if (YES == isPeriodTrace)
	{
		TRACESTR(eLevelInfoNormal) << "\nCSwitchTask::OnShmKeepAliveInd - KeepAlive indication received "
		                              << "(boardId: " << m_boardId << ")";
	}

	// ===== 2. stop previous timer
	DeleteTimer(KEEP_ALIVE_TIMER_RECEIVE);

	RemoveActiveAlarmSwitchByErrorCodeUserId("OnShmKeepAliveInd",
                                             YES,
                                             AA_NO_CONNECTION_WITH_CARD,
                                             FAULT_CARD_SUBJECT,
                                             m_boardId);

	m_numOfkeepAliveTimeoutReached = 0;


	// ===== 3. fill object with data from structure received
	DWORD  structLen = sizeof(SWITCH_SM_KEEP_ALIVE_S);
	SWITCH_SM_KEEP_ALIVE_S keepAliveStruct;
	memset(&keepAliveStruct, 0, structLen);
	pSeg->Get( (BYTE*)(&keepAliveStruct), structLen );

	// VNGR-21504: save last switch shelf manager keep alive struct
	m_pProcess->SaveLastSsmKeepAliveStruct(keepAliveStruct);

	// ===== 4. report MPM as disabled
	ReportMpmExistsIfNeeded(keepAliveStruct);

	/******************************************************************************/
	/* 18.7.10 VNGR-15566 fixed by Rachel Cohen                                   */
	/* when CNRL component does not arrive in first keep alive . we do not get it */
	/* I get the function HandleCpuBoardId out of the first keep alive. and add   */
	/* m_isFirstCNRLalreadyTreated flag that indicate if we got that component    */
	/******************************************************************************/

	if (NO ==m_isFirstCNRLalreadyTreated)
		// -----  treat CPU's slotId issues
		HandleCpuBoardId(keepAliveStruct);

	/***************************************************************************/
	/*  30.07.09  VNGR-16588 fixed by Rachel Cohen.                            */
	/*  media card did not arrive on first keep alive.we need to keep entering */
	/*  the keep alive loop .                                                  */
	/***************************************************************************/

	 SLOTS_CONFIGURATION_S mfaBoardCnfArray[MAX_NUM_OF_SLOTS];
	 memset(mfaBoardCnfArray, 0, MAX_NUM_OF_SLOTS*sizeof(SLOTS_CONFIGURATION_S));

	// ===== 5. analyze the structure if needed
	if ( NO == m_isFirstKeepAliveAlreayTreated || m_isModeDetectionAlreadyTreated == NO)
	{
		TRACESTR(eLevelInfoNormal) << "\nCSwitchTask::OnShmKeepAliveInd"
		                              << "\nKeepAlive is received (and printed below)";

		// ----- 5a. print data
		PrintSmFatalFailureToTrace(keepAliveStruct);



		//**************************************************************************
		//  29.12.09  RTM_LAN detection
		//  added by Rachel Cohen
		//**************************************************************************

		// ----- 5b. treat failures on each unit (if exist)
		TreatAllComponentsOnSmFatalFailure(keepAliveStruct,mfaBoardCnfArray);

		/*for (int i=0;i<MAX_NUM_OF_SLOTS;i++)
		    	{

		    		TRACEINTO <<   "\nAfter TreatAllComponentsOnSmFatalFailure RTM LAN/ISDN  Params:\nMediaCompType" << mfaBoardCnfArray[i].mediaCompType
		    				<< "\nRtmCompType "  << mfaBoardCnfArray[i].rtmCompType
		    				<< "\nSubBoardId  "  << mfaBoardCnfArray[i].unSubBoardId
		    				<< "\nBoardId      "  << i;
		    	}*/

		eProductFamily curProductFamily = CProcessBase::GetProcess()->GetProductFamily();
		if (eProductFamilySoftMcu != curProductFamily )
		{

			MountRtmShmComponent(mfaBoardCnfArray);
			// ----- 5d. treat RtmLan and RtmIsdn slots
			//HandleRtmIsdnOrLanSlots(keepAliveStruct);

			SendRtmLanIsdnParamsToProcesses(mfaBoardCnfArray);

		}


		// ----- 5c. retrieve basic info
		RetrieveBasicInfoFromKA(keepAliveStruct,true);


		m_isFirstKeepAliveAlreayTreated = YES;
	}

	//added by huiyu. 2011.9.20 the trigger event for hot backup
	bool bTrigger = FALSE;
	bool bIsMediaCard = FALSE;
	bool bIsISDNCard = FALSE;
	static SWITCH_SM_KEEP_ALIVE_S keepAliveStructOld;
	SM_COMPONENT_STATUS_S * pKeepAlive = (SM_COMPONENT_STATUS_S *)&keepAliveStruct;
	SM_COMPONENT_STATUS_S *pKeepAliveOld = (SM_COMPONENT_STATUS_S *)&keepAliveStructOld;

	for (int i=0 ; i< MAX_NUM_OF_SLOTS ;i++,pKeepAlive++,pKeepAliveOld++)
	{	
		if(eSmComponentNotExist == pKeepAliveOld->unStatus)
		{
			continue;
		}

		if(NULL != pKeepAliveOld->sSmCompName)
		{
			eCardType curTypeOld = ConvertShelfMngrStringToCardType( (char*)(pKeepAliveOld->sSmCompName));
			//new current card name maybe is NULL.
			//eCardType curType = ConvertShelfMngrStringToCardType( (char*)(pKeepAlive->sSmCompName));
			bIsMediaCard = m_pProcess->IsMediaCard(curTypeOld);
			bIsISDNCard = m_pProcess->IsRtmIsdnCard(curTypeOld); 
			if( bIsMediaCard || bIsISDNCard)
			{
				//TRACESTR(eLevelInfoNormal) << "\nCSwitchTask::OnShmKeepAliveInd - old card name = " << (char *)pKeepAliveOld->sSmCompName 
				// 	<<" old state = " << pKeepAliveOld->unStatus;

				//TRACESTR(eLevelInfoNormal) << "\nCSwitchTask::OnShmKeepAliveInd - new card name = " << (char *)pKeepAlive->sSmCompName 
				// 	<< " new state = " <<pKeepAlive->unStatus;

				if((eSmComponentOk == pKeepAliveOld->unStatus)|| (eSmComponentMajor == pKeepAliveOld->unStatus)) 
				{		
					if ((eSmComponentResetting == pKeepAlive->unStatus)||
						(eSmComponentNotExist == pKeepAlive->unStatus)||
						(eSmComponentPowerOff == pKeepAlive->unStatus))
				  	{
						bTrigger = true;
					}
				}
			}	

			if(bTrigger)
			{
				break;
			}
		}
	}

	memcpy(&keepAliveStructOld,&keepAliveStruct,sizeof(SWITCH_SM_KEEP_ALIVE_S));

	if(bTrigger)
	{
		DWORD eventType = 0;
		CSegment *pMsg = new CSegment;

		if ( bIsMediaCard )	eventType = eFailoverMpmCardFailure;
		if ( bIsISDNCard ) eventType = eFailoverIsdnCardFailure;
		
		*pMsg << eventType;

		TRACESTR(eLevelInfoNormal) << "\nCSwitchTask::OnShmKeepAliveInd, trigger the Hot backup event:" << eventType; 

		CManagerApi api(eProcessFailover);
		STATUS status = api.SendMsg(pMsg, FAILOVER_EVENT_TRIGGER_IND);
		if (status != STATUS_OK)
		{	
			FPASSERT(FAILOVER_EVENT_TRIGGER_IND);
		}
	}
	///////////////////////////////////////////////////////////////	


	StartTimer(KEEP_ALIVE_TIMER_SEND, m_keepAliveSendPeriod*SECOND);
}

void CSwitchTask::ReportMpmExistsIfNeeded(SWITCH_SM_KEEP_ALIVE_S keepAliveStruct)
{
    // MPM is not enabled in RMX4000 or in JITC mode

	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

	if (eProductTypeRMX1500 == curProductType) return;

    BOOL bJitcMode = FALSE;
    CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
    sysConfig->GetBOOLDataByKey(CFG_KEY_JITC_MODE, bJitcMode);

    if ( (eProductTypeRMX4000 != curProductType) && (FALSE == bJitcMode) )
    {
    	return;
    }


    bool doesMpmExist = false;

	// ===== 1. search for MPM(s)
	// Bad, bad code (but it's not Mcms to blame...)!!
	if ( (true == IsMpmSmComponent(keepAliveStruct.unSmComp1))	||
		 (true == IsMpmSmComponent(keepAliveStruct.unSmComp2))	||
		 (true == IsMpmSmComponent(keepAliveStruct.unSmComp3))	||
		 (true == IsMpmSmComponent(keepAliveStruct.unSmComp4))	||
		 (true == IsMpmSmComponent(keepAliveStruct.unSmComp5))	||
		 (true == IsMpmSmComponent(keepAliveStruct.unSmComp6))	||
		 (true == IsMpmSmComponent(keepAliveStruct.unSmComp7))	||
		 (true == IsMpmSmComponent(keepAliveStruct.unSmComp8))	||
		 (true == IsMpmSmComponent(keepAliveStruct.unSmComp9))	||
		 (true == IsMpmSmComponent(keepAliveStruct.unSmComp10))	||
		 (true == IsMpmSmComponent(keepAliveStruct.unSmComp11))	||
		 (true == IsMpmSmComponent(keepAliveStruct.unSmComp12))	||
		 (true == IsMpmSmComponent(keepAliveStruct.unSmComp13))	||
		 (true == IsMpmSmComponent(keepAliveStruct.unSmComp14))	||
		 (true == IsMpmSmComponent(keepAliveStruct.unSmComp15))	||
		 (true == IsMpmSmComponent(keepAliveStruct.unSmComp16))	||
		 (true == IsMpmSmComponent(keepAliveStruct.unSmComp17))	||
		 (true == IsMpmSmComponent(keepAliveStruct.unSmComp18))	||
		 (true == IsMpmSmComponent(keepAliveStruct.unSmComp19))	||
		 (true == IsMpmSmComponent(keepAliveStruct.unSmComp20)) )
	{
		doesMpmExist = true;
	}


	// ===== 2. report (once)
	if (true == doesMpmExist)
	{
		if (NO == m_isMpmAlreadyReported)
		{
			TRACESTR(eLevelInfoNormal) << "\nCSwitchTask::ReportMpmExistsIfNeeded"
								   << "\nMPM exists (KeepAlive struct is printed below)"
								   << "\nProductType: " << ::ProductTypeToString(curProductType)
								   << "\nJITC mode:   " << ( (TRUE == bJitcMode) ? "yes" : "no" );

			PrintSmFatalFailureToTrace(keepAliveStruct);

			AddActiveAlarmSingleton( FAULT_GENERAL_SUBJECT,
									 AA_CARDS_CONFIG_EVENT,
									 MAJOR_ERROR_LEVEL,
									 MPM_BLOCKING_ALERT_DESCRIPTION,
									 true,
									 true,
									 MPM_BLOCKING_ALERT_USER_ID);

			m_isMpmAlreadyReported = YES;
		}
	}

	else // no MPM exists
	{
		RemoveActiveAlarmByErrorCodeUserId(AA_CARDS_CONFIG_EVENT, MPM_BLOCKING_ALERT_USER_ID);
		m_isMpmAlreadyReported = NO;
	}
}

//////////////////////////////////////////////////////////////////////
void CSwitchTask::RetrieveBasicInfoFromKA(SWITCH_SM_KEEP_ALIVE_S & fatalFailureStruct,bool isKeepAliveMsg)
{
	TRACESTR(eLevelInfoNormal) << "\nCSwitchTask::RetrieveBasicInfoFromKA";

	eCardType tmpCardType = eEmpty;
	eCardType curCardType= eEmpty;

	int numOfBreezeCards = 0;
	int numOfMPMRXCards  = 0;

	SM_COMPONENT_STATUS_S *fatalFailureStructPtr = (SM_COMPONENT_STATUS_S *)&fatalFailureStruct;

	for (int i=0 ; i< MAX_NUM_OF_BOARDS ;i++,fatalFailureStructPtr++){

      // we want the biggest media card type - .breeze is greater than MPM+
	  UpdateNumOfBoardsExpectedInProcess(fatalFailureStructPtr, tmpCardType);
	  if (isKeepAliveMsg == true )
	  {
	      eCardType curType = ConvertShelfMngrStringToCardType( (char*)(fatalFailureStructPtr->sSmCompName) );
	      if (m_pProcess->IsBreezeCard(curType) == true) numOfBreezeCards++;
	      if (m_pProcess->IsMpmRXCard(curType) == true) numOfMPMRXCards++;
	  }
	  if (tmpCardType > curCardType) curCardType = tmpCardType;

	}

	CManagerApi api(eProcessCards);

    /********************************************************************************/
	/* 21.6.10 VNGR-15478,VNGR-15464 added by Rachel Cohen                          */
	/* after upgrade the shelf send first keep alive without MFA components         */
	/* when we notice that the card type is empty we will not send Resources the    */
	/* card type. we will check it again th the FatalFailureInd                     */
	/********************************************************************************/

	if (curCardType != eEmpty){
	 CSegment* pMsg = new CSegment; //olga
	 *pMsg << (DWORD)curCardType;
	 *pMsg << (DWORD)numOfBreezeCards;
	 *pMsg << (DWORD)numOfMPMRXCards;
	 api.SendMsg(pMsg, MODE_DETECTION_IND);
	 m_isModeDetectionAlreadyTreated = YES;
	}

	/****************************************************************************************************/
	/* 3.8.10 VNGR-16718 fixed by Rachel Cohen                                                          */
	/* When target starting up with no media cards we send to CardsManager task BOARDS_EXPECTED_IND msg */
	/* and there we restartStartupTimers for all the processes. Resource process do not exit startup.   */
	/* so the mcu state stuck on startup on 1 sec.                                                      */
	/****************************************************************************************************/
	CSegment* pMsgCM = new CSegment;
	*pMsgCM << (BOOL)m_isFirstKeepAliveAlreayTreated;
	api.SendMsg(pMsgCM, BOARDS_EXPECTED_IND);
}

//////////////////////////////////////////////////////////////////////
void CSwitchTask::UpdateNumOfBoardsExpectedInProcess(SM_COMPONENT_STATUS_S * statusStruct, eCardType& curCardType)
{
	eCardType curType = ConvertShelfMngrStringToCardType( (char*)(statusStruct->sSmCompName) );

	static bool firstCheck=true;  //we want to send to configurator just one time at startup
	if ((firstCheck == true) && (curType == eMpmx_20))
	{
		std::string	specialProductType="RMX1500Q";

		TRACESTR(eLevelInfoNormal) << " CSwitchTask::UpdateNumOfBoardsExpectedInProcess : curType = "
							   << (DWORD)curType << ", curCardType = " << curCardType << " specialProductType= " << specialProductType;

		//   ----- 2b. set the correct type (for next startup)
		//CConfigManagerApi cfgApi;
		//cfgApi.SetSpecialProductType(specialProductType);
		// use async msg
		CSegment*  seg = new CSegment;

		*seg << specialProductType;

		CTaskApi::SendMsgWithTrace(eProcessConfigurator,
	                               			   eManager,
	                               			   seg,
	                               			   CONFIGURATOR_SET_SPECIAL_PRODUCT_TYPE);
		firstCheck = false;

	}
	else
	    TRACESTR(eLevelInfoNormal) << " CSwitchTask::UpdateNumOfBoardsExpectedInProcess : curType = "
						   << (DWORD)curType << ", curCardType = " << curCardType;


	bool isRelevant = m_pProcess->IsMediaCard(curType);
	if (true == isRelevant)
	{
		curCardType = curType;
		m_pProcess->IncreaseNumOfBoardsExpected();

		TRACESTR(eLevelInfoNormal) << "\nCSwitchTask::UpdateNumOfBoardsExpectedInProcess"
							   << "\nBoardId "		<< statusStruct->unSlotId
							   << ", subBoardId "	<< statusStruct->unSubBoardId
							   << ", type: "		<< (char*)(statusStruct->sSmCompName)
							   << "\nNumOfBoardsExpected is now " << m_pProcess->GetNumOfBoardsExpected();


		//decide on the NGB card mode  mpmx only/mpmrx only/mixed mode
		eNGBSystemCardsMode NGBSystemCardsMode_old= m_pProcess->GetNGBSystemCardsMode();
		eNGBSystemCardsMode NGBSystemCardsMode_new = NGBSystemCardsMode_old;

		if ((eNGBSystemCardsMode_illegal == NGBSystemCardsMode_old) && (m_pProcess->IsBreezeCard(curType)))
		    NGBSystemCardsMode_new = eNGBSystemCardsMode_breeze_only;

		if ((eNGBSystemCardsMode_illegal == NGBSystemCardsMode_old) && ((m_pProcess->IsMpmRXCard(curType)  || m_pProcess->IsSoftCard(curType))))
		    NGBSystemCardsMode_new = eNGBSystemCardsMode_mpmrx_only;

		/*if (((m_pProcess->IsMpmRXCard(curType)) && (NGBSystemCardsMode_old == eNGBSystemCardsMode_breeze_only)) ||
		        ((m_pProcess->IsBreezeCard(curType)) && (NGBSystemCardsMode_old == eNGBSystemCardsMode_mpmrx_only)))
		    NGBSystemCardsMode_new = eNGBSystemCardsMode_mixed_mode;*/

		m_pProcess->SetNGBSystemCardsMode(NGBSystemCardsMode_new);

		  TRACESTR(eLevelInfoNormal) << "\nCSwitchTask::UpdateNumOfBoardsExpectedInProcess"
		                               << "\nNGBSystemCardsMode_old "      << NGBSystemCardsMode_old
		                               << "\nNGBSystemCardsMode_new "   << NGBSystemCardsMode_new;

	}
}

//////////////////////////////////////////////////////////////////////
void CSwitchTask::HandleCpuBoardId(SWITCH_SM_KEEP_ALIVE_S &pStrcut)
{
	int cpuBoardId		= NOT_FIND,
	    cpuSubBoardId	= NOT_FIND;

	GetCpuBoardIdSubBoardId(pStrcut, cpuBoardId, cpuSubBoardId);

	if ( (NOT_FIND != cpuBoardId) && (NOT_FIND != cpuSubBoardId) )
	{
		// ===== 1. update process's attributes
		m_pProcess->SetCpuBoardIdSubBoardId(cpuBoardId, cpuSubBoardId);

	    // ===== 2. update CPU's record in EthSettings (CPU's LAN ports are 'always' mounted)
	    eProductType  curProductType  = CProcessBase::GetProcess()->GetProductType();
	    if (curProductType ==eProductTypeRMX4000)
	    {
	        m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(FIXED_DISPLAY_BOARD_ID_SWITCH, ETH_SETTINGS_CPU_MNGMNT_1_PORT_ON_SWITCH_BOARD, true);
	        if (m_isMultipleServices == NO)
	            m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(FIXED_DISPLAY_BOARD_ID_SWITCH, ETH_SETTINGS_CPU_SGNLNG_1_PORT_ON_SWITCH_BOARD, true);

	    }
	    else if (curProductType ==eProductTypeRMX1500)
	    {
	        if (m_isMultipleServices == NO)
	            m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(FIXED_DISPLAY_BOARD_ID_SWITCH, ETH_SETTINGS_CPU_MNGMNT_PORT_ON_SWITCH_BOARD_RMX1500, true);

	        m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(FIXED_DISPLAY_BOARD_ID_SWITCH, ETH_SETTINGS_CPU_MNGMNTB_PORT_ON_SWITCH_BOARD_RMX1500, true);
	    }

		// ===== 3. update other processes
		SendSpecSlotIdToSpecProcess(cpuBoardId, cpuSubBoardId, eShmComp_McmsCpu, eProcessMcuMngr);
		SendSpecSlotIdToSpecProcess(cpuBoardId, cpuSubBoardId, eShmComp_McmsCpu, eProcessSystemMonitoring);

		m_isFirstCNRLalreadyTreated = YES;
	}
}
//////////////////////////////////////////////////////////////////////
/*void CSwitchTask::HandleRtmIsdnOrLanSlots(SWITCH_SM_KEEP_ALIVE_S keepAliveStruct,BOOL sendMsg)
{

	//need to send the msg to utility process and to mcumngr
	string str = "\nCSwitchTask::HandleRtmIsdnOrLanSlots";

	TRACESTR(eLevelInfoNormal) << str;

	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	eProductFamily curProductFamily = CProcessBase::GetProcess()->GetProductFamily();

    if (eProductFamilySoftMcu == curProductFamily )
    	return;


	SM_COMPONENT_STATUS_S *keepAliveStructPtr = (SM_COMPONENT_STATUS_S *)&keepAliveStruct;

    DWORD RtmLan[MAX_RTM_LAN_OR_ISDN_CARDS], RtmIsdn[MAX_RTM_LAN_OR_ISDN_CARDS];
    WORD RtmLanIndex=0, RtmIsdnIndex=0;
    BOOL bExist = FALSE;
    for (int i=0; i<MAX_RTM_LAN_OR_ISDN_CARDS; i++)
    {
    	RtmLan[i] = 0;
    	RtmIsdn[i] = 0;
    }

	for (int i=0 ; i< MAX_NUM_OF_SLOTS ;i++,keepAliveStructPtr++)
       IsRtmShmComponent(*keepAliveStructPtr, RtmLan, RtmIsdn, RtmLanIndex, RtmIsdnIndex, bExist);


	// ====== 2.if one of the cards exist - send the info to McuMngr
    if ((eProductTypeRMX2000 == curProductType) || ((bExist == TRUE ) && (sendMsg == TRUE )) )
    	SendRtmIsdnOrLanToMcuMngr(RtmLanIndex, RtmIsdnIndex, RtmLan, RtmIsdn);

}*/

/*/////////////////////////////////////////////////////////////////////
void CSwitchTask::HandleRtmIsdnOrLanSlots(SWITCH_SM_KEEP_ALIVE_S keepAliveStruct,BOOL sendMsg)
{
	string str = "\nCSwitchTask::HandleRtmIsdnOrLanSlots";

	TRACESTR(eLevelInfoNormal) << str;

	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	eProductFamily curProductFamily = CProcessBase::GetProcess()->GetProductFamily();

    if (eProductFamilySoftMcu == curProductFamily )
    	return;


    DWORD RtmLan[MAX_RTM_LAN_OR_ISDN_CARDS], RtmIsdn[MAX_RTM_LAN_OR_ISDN_CARDS];
    WORD RtmLanIndex=0, RtmIsdnIndex=0;
    BOOL bExist = FALSE;
    for (int i=0; i<MAX_RTM_LAN_OR_ISDN_CARDS; i++)
    {
    	RtmLan[i] = 0;
    	RtmIsdn[i] = 0;
    }



	// ===== 1. search for RtmLan and RtmIsdn
	// Bad, bad code (but it's not Mcms to blame...)!!
    IsRtmShmComponent(keepAliveStruct.unSmComp1, RtmLan, RtmIsdn, RtmLanIndex, RtmIsdnIndex, bExist);
    IsRtmShmComponent(keepAliveStruct.unSmComp2, RtmLan, RtmIsdn, RtmLanIndex, RtmIsdnIndex, bExist);
    IsRtmShmComponent(keepAliveStruct.unSmComp3, RtmLan, RtmIsdn, RtmLanIndex, RtmIsdnIndex, bExist);
    IsRtmShmComponent(keepAliveStruct.unSmComp4, RtmLan, RtmIsdn, RtmLanIndex, RtmIsdnIndex, bExist);
    IsRtmShmComponent(keepAliveStruct.unSmComp5, RtmLan, RtmIsdn, RtmLanIndex, RtmIsdnIndex, bExist);
    IsRtmShmComponent(keepAliveStruct.unSmComp6, RtmLan, RtmIsdn, RtmLanIndex, RtmIsdnIndex, bExist);
    IsRtmShmComponent(keepAliveStruct.unSmComp7, RtmLan, RtmIsdn, RtmLanIndex, RtmIsdnIndex, bExist);
    IsRtmShmComponent(keepAliveStruct.unSmComp8, RtmLan, RtmIsdn, RtmLanIndex, RtmIsdnIndex, bExist);
    IsRtmShmComponent(keepAliveStruct.unSmComp9, RtmLan, RtmIsdn, RtmLanIndex, RtmIsdnIndex, bExist);
    IsRtmShmComponent(keepAliveStruct.unSmComp10, RtmLan, RtmIsdn, RtmLanIndex, RtmIsdnIndex, bExist);
    IsRtmShmComponent(keepAliveStruct.unSmComp11, RtmLan, RtmIsdn, RtmLanIndex, RtmIsdnIndex, bExist);
    IsRtmShmComponent(keepAliveStruct.unSmComp12, RtmLan, RtmIsdn, RtmLanIndex, RtmIsdnIndex, bExist);
    IsRtmShmComponent(keepAliveStruct.unSmComp13, RtmLan, RtmIsdn, RtmLanIndex, RtmIsdnIndex, bExist);
    IsRtmShmComponent(keepAliveStruct.unSmComp14, RtmLan, RtmIsdn, RtmLanIndex, RtmIsdnIndex, bExist);
    IsRtmShmComponent(keepAliveStruct.unSmComp15, RtmLan, RtmIsdn, RtmLanIndex, RtmIsdnIndex, bExist);
    IsRtmShmComponent(keepAliveStruct.unSmComp16, RtmLan, RtmIsdn, RtmLanIndex, RtmIsdnIndex, bExist);
    IsRtmShmComponent(keepAliveStruct.unSmComp17, RtmLan, RtmIsdn, RtmLanIndex, RtmIsdnIndex, bExist);
    IsRtmShmComponent(keepAliveStruct.unSmComp18, RtmLan, RtmIsdn, RtmLanIndex, RtmIsdnIndex, bExist);
    IsRtmShmComponent(keepAliveStruct.unSmComp19, RtmLan, RtmIsdn, RtmLanIndex, RtmIsdnIndex, bExist);
    IsRtmShmComponent(keepAliveStruct.unSmComp20, RtmLan, RtmIsdn, RtmLanIndex, RtmIsdnIndex, bExist);

	// ====== 2.if one of the cards exist - send the info to McuMngr
    if ((eProductTypeRMX2000 == curProductType) || ((bExist == TRUE ) && (sendMsg == TRUE )) )
    	SendRtmIsdnOrLanToMcuMngr(RtmLanIndex, RtmIsdnIndex, RtmLan, RtmIsdn);
    else  //check if it is RMX1500 with no RTM_ISDN
    {
        if (eProductTypeRMX1500 == curProductType )
        {

            //first lan port become active
             m_pProcess->GetEthernetSettingsStructsList()->GetSpecEthernetSettingsStructWrapper(0)->SetSlotId(FIXED_BOARD_ID_RTM_1);

             m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(FIXED_BOARD_ID_RTM_1, ETH_SETTINGS_MEDIA_1_PORT_ON_MEDIA_BOARD, true);
             m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(FIXED_BOARD_ID_RTM_1, ETH_SETTINGS_MEDIA_2_PORT_ON_MEDIA_BOARD, true);


            if (m_isMultipleServices == YES)
            {


                m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(FIXED_DISPLAY_BOARD_ID_SWITCH,ETH_SETTINGS_CPU_SIGNALLING_PORT_IDX_RMX1500,false);

            }

            else
            {

                m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(FIXED_DISPLAY_BOARD_ID_SWITCH,ETH_SETTINGS_CPU_SIGNALLING_PORT_IDX_RMX1500,true);

            }

            // In RMX1500 with no RTM_ISDN we want that ethernetSetting will be set in case of MS
            SendRtmIsdnOrLanToMcuMngr(RtmLanIndex, RtmIsdnIndex, RtmLan, RtmIsdn);

        }

    }
}*/

void CSwitchTask::SendRtmIsdnOrLanToMcuMngr(WORD RtmLanIndex, WORD RtmIsdnIndex, DWORD RtmLan[], DWORD RtmIsdn[])
{
	// === prepare segment
	CSegment*  pSeg = new CSegment;

	*pSeg << (WORD)RtmLanIndex
	      << (WORD)RtmIsdnIndex;

	// === print to log

	CLargeString str = "\nCSwitchTask::SendRtmIsdnOrLanToMcuMngr";

	if (RtmLanIndex==0)
		str << "\nNo RtmLan Slots";
	else
	{
    	for (int i=0; i<RtmLanIndex; i++)
    	{
    		str << "\nRtmLan exist in slot ";
    		str << RtmLan[i];

    		*pSeg << (DWORD)(RtmLan[i]);
    	}
	}

	if (RtmIsdnIndex==0)
		str << "\nNo RtmIsdn Slots";
	else
	{
    	for (int i=0; i<RtmIsdnIndex; i++)
    	{
    		str << "\nRtmIsdn exist in slot ";
    		str << RtmIsdn[i];

    		*pSeg << (DWORD)(RtmIsdn[i]);
    	}
	}

	TRACESTR(eLevelInfoNormal) << str.GetString();

	// === send
	CManagerApi api(eProcessMcuMngr);
	api.SendMsg(pSeg, RTM_LANS_AND_ISDN_SLOT_IND);
}
//////////////////////////////////////////////////////////////////////
/*void CSwitchTask::IsRtmShmComponent(SM_COMPONENT_STATUS_S statusStruct, DWORD RtmLan[], DWORD RtmIsdn[], WORD &RtmLanIndex, WORD &RtmIsdnIndex, BOOL &bExist)
{
	eShmComponentType compType = ConvertShelfMngrStringToCompType( (char*)(statusStruct.sSmCompName) );
	DWORD DisplayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(statusStruct.unSlotId, statusStruct.unSubBoardId);

    eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	if (compType == eShmComp_RtmIsdn)
	{
		RtmIsdn[RtmIsdnIndex] = statusStruct.unSlotId;
		RtmIsdnIndex++;
		bExist = TRUE;

	    if (eProductTypeRMX1500 != curProductType)
	        m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(DisplayBoardId, ETH_SETTINGS_MEDIA_1_PORT_ON_MEDIA_BOARD, true);

	    else
	    {

	        if (m_isMultipleServices == YES)
	        {
	            //first lan port become active
	            m_pProcess->GetEthernetSettingsStructsList()->GetSpecEthernetSettingsStructWrapper(0)->SetSlotId(FIXED_BOARD_ID_RTM_1);
	            //remove signalling port
	           // m_pProcess->GetEthernetSettingsStructsList()->GetSpecEthernetSettingsStructWrapper(ETH_SETTINGS_CPU_SIGNALLING_PORT_IDX_RMX1500)->SetSlotId(0);
	            m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(FIXED_DISPLAY_BOARD_ID_SWITCH,ETH_SETTINGS_CPU_SIGNALLING_PORT_IDX_RMX1500,false);

	            m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(DisplayBoardId, ETH_SETTINGS_MEDIA_1_PORT_ON_MEDIA_BOARD, true);
	            m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(DisplayBoardId, ETH_SETTINGS_MEDIA_2_PORT_ON_MEDIA_BOARD, true);
	        }

	        else
	        {
	            m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(FIXED_DISPLAY_BOARD_ID_SWITCH,ETH_SETTINGS_CPU_SIGNALLING_PORT_IDX_RMX1500,true);

	           // m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(DisplayBoardId, ETH_SETTINGS_MEDIA_1_PORT_ON_MEDIA_BOARD, false);

	            m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(DisplayBoardId, ETH_SETTINGS_MEDIA_2_PORT_ON_MEDIA_BOARD, true);
	        }
	    }

		return;
	}

	if (compType == eShmComp_RtmLan)
	{
		RtmLan[RtmLanIndex] = statusStruct.unSlotId;
		RtmLanIndex++;
		bExist = TRUE;

		if (eProductTypeRMX2000 == curProductType)
		{
			m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(DisplayBoardId, ETH_SETTINGS_MEDIA_1_PORT_ON_MEDIA_BOARD, true);
			m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(DisplayBoardId, ETH_SETTINGS_MEDIA_2_PORT_ON_MEDIA_BOARD, true);
		}
		else
		{
	    //for RMX4000
	    if (m_isMultipleServices == YES)
	    {
	        m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(FIXED_DISPLAY_BOARD_ID_SWITCH,ETH_SETTINGS_CPU_SIGNALLING_1_PORT_IDX_RMX4000,false);
	        m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(FIXED_DISPLAY_BOARD_ID_SWITCH,ETH_SETTINGS_CPU_SIGNALLING_2_PORT_IDX_RMX4000,false);
	        m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(DisplayBoardId, ETH_SETTINGS_MEDIA_1_PORT_ON_MEDIA_BOARD, true);
	        m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(DisplayBoardId, ETH_SETTINGS_MEDIA_2_PORT_ON_MEDIA_BOARD, true);
	    }
	    else
	    {
	        m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(FIXED_DISPLAY_BOARD_ID_SWITCH,ETH_SETTINGS_CPU_SIGNALLING_1_PORT_IDX_RMX4000,true);
	        m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(FIXED_DISPLAY_BOARD_ID_SWITCH,ETH_SETTINGS_CPU_SIGNALLING_2_PORT_IDX_RMX4000,true);
	        m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(DisplayBoardId, ETH_SETTINGS_MEDIA_1_PORT_ON_MEDIA_BOARD, true);
	        m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(DisplayBoardId, ETH_SETTINGS_MEDIA_2_PORT_ON_MEDIA_BOARD, true);
			}
	    }
	    return;
	}

	if (DisplayBoardId>=13 && DisplayBoardId<=16)	//for slots 13-16
	{
		m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(DisplayBoardId, ETH_SETTINGS_MEDIA_2_PORT_ON_MEDIA_BOARD, false);
	}
}
*/



//////////////////////////////////////////////////////////////////////
void CSwitchTask::MountRtmShmComponent(SLOTS_CONFIGURATION_S mfaBoardCnfArray[])
{



	/*                          MPMX                |             MPMRX
---------------------------------------------------------------------------------
	Old RTM ISDN               Port 1	                          Port 1
---------------------------------------------------------------------------------
	Old RTM LAN                Port 2                              Port 2
	                           Redundancy : Port 1           Redundancy : Port 1
---------------------------------------------------------------------------------


	New RTM ISDN               Port 4                           Port 2
                               Redundancy : Port 3              Redundancy : Port 1
	-----------------------------------------------------------------------------


	New RTM LAN               Port 4                           Port 2
	                          Redundancy : Port 3              Redundancy : Port 1
	----------------------------------------------------------------------------*/


	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
    bool isRtmIsdnExistInRmx1500 = false;

	for (int i=0;i<MAX_NUM_OF_SLOTS;i++)
	{
		eShmComponentType mediaCompType = mfaBoardCnfArray[i].mediaCompType;
		eShmComponentType rtmCompType = mfaBoardCnfArray[i].rtmCompType;
		DWORD DisplayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(i, RTM_ISDN_SUBBOARD_ID);


		if (rtmCompType == eShmComp_RtmIsdn )
		{
			m_pProcess->GetEthernetSettingsStructsList()->GetSpecEthernetSettingsStructWrapper(i*2-2)->SetPortId(ETH_SETTINGS_MEDIA_1_PORT_ON_MEDIA_BOARD);
			m_pProcess->GetEthernetSettingsStructsList()->GetSpecEthernetSettingsStructWrapper(i*2-1)->SetPortId(ETH_SETTINGS_MEDIA_2_PORT_ON_MEDIA_BOARD);


	    if (eProductTypeRMX1500 != curProductType)
	        m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(DisplayBoardId, ETH_SETTINGS_MEDIA_1_PORT_ON_MEDIA_BOARD, true);

			else
			{
				isRtmIsdnExistInRmx1500 = true;
				if (m_isMultipleServices == YES)
				{
					//first lan port become active
					m_pProcess->GetEthernetSettingsStructsList()->GetSpecEthernetSettingsStructWrapper(0)->SetSlotId(FIXED_BOARD_ID_RTM_1);

					//remove signalling port
					// m_pProcess->GetEthernetSettingsStructsList()->GetSpecEthernetSettingsStructWrapper(ETH_SETTINGS_CPU_SIGNALLING_PORT_IDX_RMX1500)->SetSlotId(0);
					m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(FIXED_DISPLAY_BOARD_ID_SWITCH,ETH_SETTINGS_CPU_SIGNALLING_PORT_IDX_RMX1500,false);

	            m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(DisplayBoardId, ETH_SETTINGS_MEDIA_1_PORT_ON_MEDIA_BOARD, true);
	            m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(DisplayBoardId, ETH_SETTINGS_MEDIA_2_PORT_ON_MEDIA_BOARD, true);
	        }

	        else
	        {
	            m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(FIXED_DISPLAY_BOARD_ID_SWITCH,ETH_SETTINGS_CPU_SIGNALLING_PORT_IDX_RMX1500,true);

	           // m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(DisplayBoardId, ETH_SETTINGS_MEDIA_1_PORT_ON_MEDIA_BOARD, false);

	            m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(DisplayBoardId, ETH_SETTINGS_MEDIA_2_PORT_ON_MEDIA_BOARD, true);
	        }
			}


		}
		else if ((rtmCompType == eShmComp_RtmIsdn9 || rtmCompType == eShmComp_RtmIsdn9_10G ||rtmCompType == eShmComp_RtmLan4 ||rtmCompType == eShmComp_RtmLan4_10G ) && (mediaCompType == eShmComp_MfaMpmx))
		{
			m_pProcess->GetEthernetSettingsStructsList()->GetSpecEthernetSettingsStructWrapper(i*2-2)->SetPortId(ETH_SETTINGS_MEDIA_3_PORT_ON_MEDIA_BOARD);
			m_pProcess->GetEthernetSettingsStructsList()->GetSpecEthernetSettingsStructWrapper(i*2-1)->SetPortId(ETH_SETTINGS_MEDIA_4_PORT_ON_MEDIA_BOARD);



			if (m_isMultipleServices == YES)
			{
				//first lan port become active
				m_pProcess->GetEthernetSettingsStructsList()->GetSpecEthernetSettingsStructWrapper(0)->SetSlotId(FIXED_BOARD_ID_RTM_1);

				//remove signalling port
				// m_pProcess->GetEthernetSettingsStructsList()->GetSpecEthernetSettingsStructWrapper(ETH_SETTINGS_CPU_SIGNALLING_PORT_IDX_RMX1500)->SetSlotId(0);
				m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(FIXED_DISPLAY_BOARD_ID_SWITCH,ETH_SETTINGS_CPU_SIGNALLING_PORT_IDX_RMX1500,false);

				m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(DisplayBoardId, ETH_SETTINGS_MEDIA_3_PORT_ON_MEDIA_BOARD, true);
				m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(DisplayBoardId, ETH_SETTINGS_MEDIA_4_PORT_ON_MEDIA_BOARD, true);
			}

			else
			{
				m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(FIXED_DISPLAY_BOARD_ID_SWITCH,ETH_SETTINGS_CPU_SIGNALLING_PORT_IDX_RMX1500,true);

				m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(DisplayBoardId, ETH_SETTINGS_MEDIA_4_PORT_ON_MEDIA_BOARD, true);
			}



		}
		else if (rtmCompType == eShmComp_RtmLan ||
				((rtmCompType == eShmComp_RtmIsdn9 || rtmCompType == eShmComp_RtmIsdn9_10G
						|| rtmCompType == eShmComp_RtmIsdn9_10G ||rtmCompType == eShmComp_RtmLan4 ||rtmCompType == eShmComp_RtmLan4_10G) && (mediaCompType == eShmComp_MfaMpmRx )))
		{
			m_pProcess->GetEthernetSettingsStructsList()->GetSpecEthernetSettingsStructWrapper(i*2-2)->SetPortId(ETH_SETTINGS_MEDIA_1_PORT_ON_MEDIA_BOARD);
			m_pProcess->GetEthernetSettingsStructsList()->GetSpecEthernetSettingsStructWrapper(i*2-1)->SetPortId(ETH_SETTINGS_MEDIA_2_PORT_ON_MEDIA_BOARD);



			if (eProductTypeRMX2000 == curProductType)
			{
				m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(DisplayBoardId, ETH_SETTINGS_MEDIA_1_PORT_ON_MEDIA_BOARD, true);
				m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(DisplayBoardId, ETH_SETTINGS_MEDIA_2_PORT_ON_MEDIA_BOARD, true);
			}
			else
			{
				//for RMX4000
				if (m_isMultipleServices == YES)
				{
					m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(FIXED_DISPLAY_BOARD_ID_SWITCH,ETH_SETTINGS_CPU_SIGNALLING_1_PORT_IDX_RMX4000,false);
					m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(FIXED_DISPLAY_BOARD_ID_SWITCH,ETH_SETTINGS_CPU_SIGNALLING_2_PORT_IDX_RMX4000,false);
					m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(DisplayBoardId, ETH_SETTINGS_MEDIA_1_PORT_ON_MEDIA_BOARD, true);
					m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(DisplayBoardId, ETH_SETTINGS_MEDIA_2_PORT_ON_MEDIA_BOARD, true);
				}
				else
				{
					m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(FIXED_DISPLAY_BOARD_ID_SWITCH,ETH_SETTINGS_CPU_SIGNALLING_1_PORT_IDX_RMX4000,true);
					m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(FIXED_DISPLAY_BOARD_ID_SWITCH,ETH_SETTINGS_CPU_SIGNALLING_2_PORT_IDX_RMX4000,true);
					m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(DisplayBoardId, ETH_SETTINGS_MEDIA_1_PORT_ON_MEDIA_BOARD, true);
					m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(DisplayBoardId, ETH_SETTINGS_MEDIA_2_PORT_ON_MEDIA_BOARD, true);
				}
			}

		}
		else if ((mediaCompType != eShmComp_Illegal) && (DisplayBoardId>=13 && DisplayBoardId<=16))	//for slots 13-16

			m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(DisplayBoardId, ETH_SETTINGS_MEDIA_2_PORT_ON_MEDIA_BOARD, false);

	}

	//check if it is RMX1500 with no RTM_ISDN
	if (eProductTypeRMX1500 == curProductType && isRtmIsdnExistInRmx1500 == false )
	{

		//first lan port become active
		m_pProcess->GetEthernetSettingsStructsList()->GetSpecEthernetSettingsStructWrapper(0)->SetSlotId(FIXED_BOARD_ID_RTM_1);

		m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(FIXED_BOARD_ID_RTM_1, ETH_SETTINGS_MEDIA_1_PORT_ON_MEDIA_BOARD, true);
		m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(FIXED_BOARD_ID_RTM_1, ETH_SETTINGS_MEDIA_2_PORT_ON_MEDIA_BOARD, true);


		if (m_isMultipleServices == YES)
		{


			m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(FIXED_DISPLAY_BOARD_ID_SWITCH,ETH_SETTINGS_CPU_SIGNALLING_PORT_IDX_RMX1500,false);

		}

		else
		{

			m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(FIXED_DISPLAY_BOARD_ID_SWITCH,ETH_SETTINGS_CPU_SIGNALLING_PORT_IDX_RMX1500,true);

		}

		/* In RMX1500 with no RTM_ISDN we want that ethernetSetting will be set in case of MS */
		//SendRtmIsdnOrLanToMcuMngr(RtmLanIndex, RtmIsdnIndex, RtmLan, RtmIsdn);

	}


}
//////////////////////////////////////////////////////////////////////
void CSwitchTask::GetCpuBoardIdSubBoardId(SWITCH_SM_KEEP_ALIVE_S &pStrcut, int &cpuBoardId, int &cpuSubBoardId)
{
	// Mcms should be prepared to all variations of 'CPU' that are sent from ShelfMngr...
	//     They can be either "CNTL" or "CNTL_PLUS or (the illegal name) "CPU"
	SM_COMPONENT_STATUS_S *pStrcutPtr = (SM_COMPONENT_STATUS_S *)&pStrcut;


	for (int i=0 ; i< MAX_NUM_OF_COMPONENTS ;i++,pStrcutPtr++){

		if ( ( !strncmp((char*)pStrcutPtr->sSmCompName, "CNTL", strlen("CNTL"))) 	            ||
		     ( !strncmp((char*)pStrcutPtr->sSmCompName, "CNTL+", strlen("CNTL+")))			    ||
		     ( !strncmp((char*)pStrcutPtr->sSmCompName, "CNTL_PLUS", strlen("CNTL_PLUS")))      ||
		     ( !strncmp((char*)pStrcutPtr->sSmCompName, "CNTL4000", strlen("CNTL4000")))        ||
		     ( !strncmp((char*)pStrcutPtr->sSmCompName, "CNTL1500", strlen("CNTL1500")))        ||
		     ( !strncmp((char*)pStrcutPtr->sSmCompName, "CPU", strlen("CPU"))) )
		{
			cpuBoardId		= pStrcutPtr->unSlotId;
			cpuSubBoardId	= pStrcutPtr->unSubBoardId;

	TRACESTR(eLevelInfoNormal) << "\nCSwitchTask::GetSpecSlotId"
									   << "\nComponent name: " << (char *)pStrcutPtr->sSmCompName
									   << "\nSlotId: " << cpuBoardId << ", subBoardId: " << cpuSubBoardId;

			return;
		}
	}
}

void CSwitchTask::SendSpecSlotIdToSpecProcess(int boardId, int subBoardId, eShmComponentType compType, eProcessType processType)
{
	STATUS res = STATUS_OK;

    CSegment*  pRetParam = new CSegment;
    *pRetParam << (DWORD)boardId
    		   << (DWORD)subBoardId
    		   << (DWORD)compType;

	// ===== 2. print to log
	TRACESTR(eLevelInfoNormal) << "\nCSwitchTask::SendSpecSlotIdToSpecProcess"
								  << "\nBoardId:     " << boardId
								  << "\nSubBoardId:  " << subBoardId
								  << "\nCmpnntName:  " << ::ShelfMngrComponentTypeToString(compType)
								  << "\nProcessType: " << ::ProcessTypeToString(processType);

	// ===== 3. send
	CManagerApi api(processType);
	api.SendMsg(pRetParam, SM_COMP_SLOT_ID_IND);
}

//////////////////////////////////////////////////////////////////////
void CSwitchTask::TreatFailureInSmUnit_mfa(eShmComponentType compType, SM_COMPONENT_STATUS_S * compStatus)
{
	DWORD slotId = compStatus->unSlotId;
	DWORD sub_BoardId = compStatus->unSubBoardId;

	// ===== 1. print to log
	CLargeString compStr = "\nCSwitchTask::TreatFailureInSmUnit_mfa;";
	compStr << "\nCompType: " << ::ShelfMngrComponentTypeToString(compType)
	        << ", boardId: " << slotId
	        << ", sub board id: " << sub_BoardId;
	char* statusStr = ::ShelfMngrComponentStatusToString(compStatus->unStatus);
	if (statusStr)
	{
		compStr <<  ", status: " << statusStr;
	}
	else
	{
		compStr <<  ", status: (invalid: " << compStatus->unStatus << ")";
	}
	compStr << ", bitmask: "        << compStatus->unStatusDescriptionBitmask
	        << ", component name: " << (char*)(compStatus->sSmCompName);
	TRACESTR(eLevelInfoNormal) << compStr.GetString();


		/*****************************************************************************************************/
	/* 23.7.10 VNGR - 16358 fixed by Rachel Cohen                                                        */
	/* in case it is a rtm_isdn component with compStatus->unStatus = eSmComponentNotExist &&            */
	/* unStatusDescriptionBitmask & SM_COMP_POWER_OFF_BITMASK that mean card was pushed in               */
	/* and we do not want to return.                                                                     */
	/*****************************************************************************************************/

	// ===== 2. send the segment to the relevant MFA
	if ((eSmComponentNotExist == compStatus->unStatus) && (compType !=eShmComp_RtmIsdn))
		return;

	//   === 2a. prepare the data to be sent
	CSegment* pSegToBoard = new CSegment;
	*pSegToBoard << (DWORD)compStatus->unStatus;
	*pSegToBoard << (DWORD)compStatus->unStatusDescriptionBitmask;
	*pSegToBoard << (DWORD)sub_BoardId;
	//   === 2b. send the msg
	COsQueue* queue = m_pProcess->GetMfaMbx(slotId, sub_BoardId);

	if (!queue) // queue not exist - Switch treats the messege
	{
//		if ( eSmComponentOk == compStatus.unStatus) // cannot count on unStatus (since it comes as 'Resetting' even if the component is ok)
		/*****************************************************************************************************/
		/* 23.7.10 VNGR - 16358 fixed by Rachel Cohen                                                        */
		/* There can be a case that RtmIsdnTask is not created yet and the AA_POWER_OFF_PROBLEM of rtm_isdn  */
	    /* component is added in hot swap when card pushed in ,we want to remove the active alarm            */
	    /* ( AA_POWER_OFF_PROBLEM).The indication on that one is eSmComponentNotExist == compStatus.unStatus */
	    /*  && 0x20  == compStatus->unStatusDescriptionBitmask.                                              */
		/*****************************************************************************************************/
		if (( 0/* OK!! */ == compStatus->unStatusDescriptionBitmask ) || ((compStatus->unStatusDescriptionBitmask & SM_COMP_POWER_OFF_BITMASK) && (eSmComponentNotExist == compStatus->unStatus)) )
		{
			RemoveActiveAlarmsByCompTypeUserId(compType, slotId, FAULT_CARD_SUBJECT);
		}

		else // status != ok
		{
			BOOL isResetting = FALSE;

			DWORD displayMfaBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(slotId, sub_BoardId);
			BOOL isFailureProblem = AddMfaActiveAlarms(compStatus->unStatusDescriptionBitmask, displayMfaBoardId, isResetting);
			TRACESTR(eLevelInfoNormal) << "\nCSwitchTask::TreatFailureInSmUnit_mfa - No Mbx for BoardId " << slotId << " subBoardId " << sub_BoardId;

			if (TRUE == isFailureProblem)
			{
				CSmallString errStr = "MPM failure, ";
				errStr << "boardId: " << slotId;

				// ask for reset
				m_pProcess->SendResetReqToDaemon(errStr);
			}
		} // end (status != ok)

        PDELETE(pSegToBoard);
	}

	else // queue ok - let Mfa treat the messege
	{
		// send the message to MFA
		CMfaApi mfaApi;
		mfaApi.CreateOnlyApi(*queue);
		mfaApi.SendMsg(pSegToBoard, SM_MFA_FAILURE_IND);
	}
}

//////////////////////////////////////////////////////////////////////
void CSwitchTask::TreatFailureInSmUnit_nonMfa( eShmComponentType compType,
                                               SM_COMPONENT_STATUS_S * compStatus )
{
	DWORD slotId = compStatus->unSlotId;
	//DWORD sub_BoardId = compStatus.unSubBoardId;
	DWORD sub_BoardId = 2;//compStatus.unSubBoardId;

	if (eSmComponentNotExist == compStatus->unStatus)
	{
		CMedString compStr = "\nCSwitchTask::TreatFailureInSmUnit_nonMfa;";
		compStr << " slot id: "         << slotId
		        << " sub board id: "    << sub_BoardId
		        << ", status: ComponentNotExist"
		        << ", bitmask: "        << compStatus->unStatusDescriptionBitmask
		        << ", component name: " << (char*)(compStatus->sSmCompName);
		TRACESTR(eLevelInfoNormal) << compStr.GetString();

		return;
	}


	WORD subBoardId = FIRST_SUBBOARD_ID;

//	if ( eSmComponentOk == compStatus.unStatus) // cannot count on unStatus (since it comes as 'Resetting' even if the component is ok)
	if ( 0/* OK!! */ == compStatus->unStatusDescriptionBitmask )
	{
		RemoveActiveAlarmsByCompTypeUserId(compType, slotId, FAULT_MPL_SUBJECT);
	}

	else // compStatus != ok
	{
		BOOL isResetting = FALSE;


// Temp: until ShelfMngr will send the right status of the MFA!!
//		if ( eShmComponentResetting == compStatus.unStatus)
//			isResetting = TRUE;

		BOOL isFailureProblem = UpdateActiveAlarmsNonMfaIfNeeded(compType, compStatus->unStatusDescriptionBitmask, slotId, isResetting, (char*)(compStatus->sSmCompName));

		// ===== in Failure problem of Switch: ask reset from Daemon
		if ( (eShmComp_SwitchBoard == compType) &&
		     ((TRUE == isFailureProblem) || (TRUE == isResetting)) )
		{
			COsQueue& rcvMbx = GetRcvMbx();
			m_pProcess->TurnMfaTaskToZombie(&rcvMbx); // no messages will be received by this task

			// ask for reset
			m_pProcess->SendResetReqToDaemon("Switch failure");
		}
	} // end  compStatus != ok
}

//////////////////////////////////////////////////////////////////////
BOOL CSwitchTask::AddMfaActiveAlarms(APIU32 problemBitmask, DWORD mfaBoardId, BOOL isResetting)
{
	BOOL isFailureProblem = FALSE;


	if ( (SM_COMP_VOLTAGE_BITMASK & problemBitmask) &&
	     (false == IsActiveAlarmExistByErrorCodeUserId(AA_VOLTAGE_PROBLEM, mfaBoardId)) )
	{
		AddActiveAlarmSwitch("AddMfaActiveAlarms", NO, FAULT_CARD_SUBJECT, AA_VOLTAGE_PROBLEM, MAJOR_ERROR_LEVEL, "Shelf voltage problem", true, true, mfaBoardId, mfaBoardId, FAULT_TYPE_MPM);
	}

	DWORD displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(m_boardId, RTM_ISDN_SUBBOARD_ID);
	if ( (SM_COMP_RTM_OR_ISDN_MISSING_BITMASK & problemBitmask) &&
		     (false == IsActiveAlarmExistByErrorCodeUserId(AA_RTM_LAN_OR_ISDN_MISSING, displayBoardId)) )
		{
      AddActiveAlarmSwitch("AddMfaActiveAlarms",
                           NO,
                           FAULT_CARD_SUBJECT,
                           AA_RTM_LAN_OR_ISDN_MISSING,
                           MAJOR_ERROR_LEVEL,
                           GetAlarmDescription(AA_RTM_LAN_OR_ISDN_MISSING),
                           true,
                           true,
                           displayBoardId,
                           displayBoardId,
                           FAULT_TYPE_MPM);
		}

	if ( (SM_COMP_TEMPERATURE_MAJOR_BITMASK & problemBitmask) &&
	     (false == IsActiveAlarmExistByErrorCodeUserId(AA_TEMPERATURE_MAJOR_PROBLEM, mfaBoardId)) )
	{
		AddActiveAlarmSwitch("AddMfaActiveAlarms", NO, FAULT_CARD_SUBJECT, AA_TEMPERATURE_MAJOR_PROBLEM, MAJOR_ERROR_LEVEL, "Temperature has reached a problematic level and requires attention", true, true, mfaBoardId, mfaBoardId, FAULT_TYPE_MPM);
	}

	if ( (SM_COMP_TEMPERATURE_CRITICAL_BITMASK & problemBitmask) &&
	     (false == IsActiveAlarmExistByErrorCodeUserId(AA_TEMPERATURE_CRITICAL_PROBLEM, mfaBoardId)) )
	{
		AddActiveAlarmSwitch("AddMfaActiveAlarms", NO, FAULT_CARD_SUBJECT, AA_TEMPERATURE_CRITICAL_PROBLEM, MAJOR_ERROR_LEVEL, "Temperature has reached a critical level. MCU will shut down", true, true, mfaBoardId, mfaBoardId, FAULT_TYPE_MPM);
	}

	if ( (SM_COMP_POWER_OFF_BITMASK & problemBitmask) &&
	     (false == IsActiveAlarmExistByErrorCodeUserId(AA_POWER_OFF_PROBLEM, mfaBoardId)) )
	{
		AddActiveAlarmSwitch("AddMfaActiveAlarms", NO, FAULT_CARD_SUBJECT, AA_POWER_OFF_PROBLEM, MAJOR_ERROR_LEVEL, "power off error", true, true, mfaBoardId, mfaBoardId, FAULT_TYPE_MPM);
	}

	if ( ((SM_COMP_FAILURE_BITMASK & problemBitmask) || (TRUE == isResetting)) &&
	     (false == IsActiveAlarmExistByErrorCodeUserId(AA_FAILURE_PROBLEM, mfaBoardId)) )
	{
		AddActiveAlarmSwitch("AddMfaActiveAlarms", NO, FAULT_CARD_SUBJECT, AA_FAILURE_PROBLEM, MAJOR_ERROR_LEVEL, "Unknown shelf error", true, true, mfaBoardId, mfaBoardId, FAULT_TYPE_MPM);
		isFailureProblem = TRUE;
	}
	// Fault only
	if ( (SM_COMP_OTHER_BITMASK & problemBitmask) &&
	     (false == IsActiveAlarmFaultOnlyExistByErrorCodeUserId(AA_OTHER_PROBLEM, mfaBoardId)) )
	{
		AddActiveAlarmFaultOnlySwitch("AddMfaActiveAlarms", FAULT_CARD_SUBJECT, AA_OTHER_PROBLEM, MAJOR_ERROR_LEVEL, "Unspecified shelf error", mfaBoardId, mfaBoardId, FAULT_TYPE_MPM);

	}

	return isFailureProblem;
}

//////////////////////////////////////////////////////////////////////
void CSwitchTask::RemoveActiveAlarmsByCompTypeUserId(eShmComponentType compType, DWORD slotId, BYTE subject, BOOL isTransferredToMfa)
{
	BOOL wasAlarmExist = NO;
	DWORD displayMfaBoardId,displaySwitchBoardId=0;

	BOOL isToChangeState = NO;
	if (eShmComp_SwitchBoard == compType)
	{
		isToChangeState = YES;
		//VNGFE-4848 added by rachel Cohen
		TRACESTR(eLevelInfoNormal)<< "\nCSwitchTask::RemoveActiveAlarmsByCompTypeUserId slotId "<< slotId;
		displaySwitchBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(slotId, SWITCH_SUBBOARD_ID);
		TRACESTR(eLevelInfoNormal)<< "\nCSwitchTask::RemoveActiveAlarmsByCompTypeUserId displaySwitchBoardId "<< displaySwitchBoardId;
		slotId = displaySwitchBoardId;

	}


	/****************************************************************************************************/
	/* 23.7.10 VNGR - 16358 fixed by Rachel Cohen                                                       */
	/* There can be a case that RtmIsdnTask is not created yet and the AA_POWER_OFF_PROBLEM of rtm_isdn */
	/* component is added from the switch and we want to remove it from switch.                         */
	/****************************************************************************************************/
	 displayMfaBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(slotId, RTM_ISDN_SUBBOARD_ID);
	if (( true == IsActiveAlarmExistByErrorCodeUserId(AA_POWER_OFF_PROBLEM, displayMfaBoardId) ) &&((eShmComp_RtmLan == compType) || (eShmComp_RtmIsdn == compType )))
	{

		wasAlarmExist = YES;
		RemoveActiveAlarmSwitchByErrorCodeUserId("RemoveActiveAlarmsByCompTypeUserId", isToChangeState, subject, AA_POWER_OFF_PROBLEM, displayMfaBoardId);
	}


	if ( true == IsActiveAlarmExistByErrorCodeUserId(AA_POWER_OFF_PROBLEM, slotId) )
	{
		wasAlarmExist = YES;
		RemoveActiveAlarmSwitchByErrorCodeUserId("RemoveActiveAlarmsByCompTypeUserId", isToChangeState, subject, AA_POWER_OFF_PROBLEM, slotId);

	}

	if ( true == IsActiveAlarmExistByErrorCodeUserId(AA_VOLTAGE_PROBLEM, slotId))
	{
		wasAlarmExist = YES;
		RemoveActiveAlarmSwitchByErrorCodeUserId("RemoveActiveAlarmsByCompTypeUserId", isToChangeState, subject, AA_VOLTAGE_PROBLEM, slotId);
	}
	else
	{
		RemoveActiveAlarmSwitchByErrorCodeUserId("RemoveActiveAlarmsByCompTypeUserId", isToChangeState, subject, AA_VOLTAGE_PROBLEM, slotId);
	}

	if ( true == IsActiveAlarmExistByErrorCodeUserId(AA_TEMPERATURE_MAJOR_PROBLEM, slotId) )
	{
		wasAlarmExist = YES;
		RemoveActiveAlarmSwitchByErrorCodeUserId("RemoveActiveAlarmsByCompTypeUserId", isToChangeState, subject, AA_TEMPERATURE_MAJOR_PROBLEM, slotId);
	}

	if ( true == IsActiveAlarmExistByErrorCodeUserId(AA_TEMPERATURE_CRITICAL_PROBLEM, slotId) )
	{
		wasAlarmExist = YES;
		RemoveActiveAlarmSwitchByErrorCodeUserId("RemoveActiveAlarmsByCompTypeUserId", isToChangeState, subject, AA_TEMPERATURE_CRITICAL_PROBLEM, slotId);
	}

	if ( true == IsActiveAlarmExistByErrorCodeUserId(AA_FAILURE_PROBLEM, slotId) )
	{
		wasAlarmExist = YES;
		RemoveActiveAlarmSwitchByErrorCodeUserId("RemoveActiveAlarmsByCompTypeUserId", isToChangeState, subject, AA_FAILURE_PROBLEM, slotId);
	}
	// Fault only
	if ( true == IsActiveAlarmFaultOnlyExistByErrorCodeUserId(AA_OTHER_PROBLEM, slotId) )
	{
		wasAlarmExist = YES;
		RemoveActiveAlarmSwitchByErrorCodeUserId("RemoveActiveAlarmsByCompTypeUserId", isToChangeState, subject, AA_OTHER_PROBLEM, slotId, TRUE);
	}

	 displayMfaBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(slotId, RTM_ISDN_SUBBOARD_ID);

	if ( true == IsActiveAlarmExistByErrorCodeUserId(AA_RTM_LAN_OR_ISDN_MISSING, displayMfaBoardId) )
		{
			wasAlarmExist = YES;
			RemoveActiveAlarmSwitchByErrorCodeUserId("RemoveActiveAlarmsByCompTypeUserId", isToChangeState, subject, AA_RTM_LAN_OR_ISDN_MISSING, displayMfaBoardId);
		}

	// print to logger (if needed)
	if (YES == isTransferredToMfa)
	{
		TRACESTR(eLevelInfoNormal)
			<< "\nCSwitchTask::RemoveActiveAlarmsByCompTypeUserId -"
			<< "\nAlarms are transferred to Mfa (boardId " << slotId << ")";
	}

	else if (YES == wasAlarmExist)
	{
		TRACESTR(eLevelInfoNormal)
			<< "\nCSwitchTask::RemoveActiveAlarmsByCompTypeUserId -"
			<< "\n" << ::ShelfMngrComponentTypeToString(compType) << " (boardId " << slotId << ") is now ok";
	}
}

//////////////////////////////////////////////////////////////////////
BOOL CSwitchTask::UpdateActiveAlarmsNonMfaIfNeeded(eShmComponentType compType, APIU32 problemBitmask, DWORD slotId,
												   BOOL isResetting, char *compName)
{
	BOOL isFailureProblem     = FALSE,
	     isNewSpecificProblem = FALSE,
	     isNewProblem         = FALSE;

	BOOL isToChangeState = NO;
	string description;
	if (eShmComp_SwitchBoard == compType)
		isToChangeState = YES;

	// prepare string to log file
	CLargeString errStr = "\nCSwitchTask::UpdateActiveAlarmsNonMfaIfNeeded\n";
	errStr << ::ShelfMngrComponentTypeToString(compType) << " is not ok; problem in ";

	// ===== Voltage problem
	description = "Voltage problem in ";
	description+= compName;


	isNewSpecificProblem = UpdateSpecificActiveAlarmNonMfa(SM_COMP_VOLTAGE_BITMASK, problemBitmask, AA_VOLTAGE_PROBLEM, isToChangeState, slotId, compType, description);
	if (TRUE == isNewSpecificProblem)
	{
		isNewProblem = TRUE;
		errStr << "- Voltage ";
	}

	// ===== Rtm missing problem
	isNewSpecificProblem = UpdateSpecificActiveAlarmNonMfa(SM_COMP_RTM_OR_ISDN_MISSING_BITMASK,
                                                         problemBitmask,
                                                         AA_RTM_LAN_OR_ISDN_MISSING,
                                                         isToChangeState,
                                                         slotId,
                                                         compType,
                                                         GetAlarmDescription(AA_RTM_LAN_OR_ISDN_MISSING));
	if (TRUE == isNewSpecificProblem)
	{
		isNewProblem = TRUE;
		errStr << "- No Lan Card ";
	}

	description = "Temperature problem in ";
	description+= compName;
	description+= " - major";
	// ===== Temperature major problem
	isNewSpecificProblem = UpdateSpecificActiveAlarmNonMfa(SM_COMP_TEMPERATURE_MAJOR_BITMASK, problemBitmask, AA_TEMPERATURE_MAJOR_PROBLEM, isToChangeState, slotId, compType, description);
	if (TRUE == isNewSpecificProblem)
	{
		isNewProblem = TRUE;
		errStr << "- Temperature major ";
	}

	// ===== Temperature critical problem
	description = "Temperature problem in ";
	description+= compName;
	description+= " - Critical";
	isNewSpecificProblem = UpdateSpecificActiveAlarmNonMfa(SM_COMP_TEMPERATURE_CRITICAL_BITMASK, problemBitmask, AA_TEMPERATURE_CRITICAL_PROBLEM, isToChangeState, slotId, compType, description);
	if (TRUE == isNewSpecificProblem)
	{
		isNewProblem = TRUE;
		errStr << "- Temperature critical ";
	}

	// ===== Power off critical problem
	description = "Power off problem in ";
	description+= compName;
	isNewSpecificProblem = UpdateSpecificActiveAlarmNonMfa(SM_COMP_POWER_OFF_BITMASK, problemBitmask, AA_POWER_OFF_PROBLEM, isToChangeState, slotId, compType, "Power off problem");
	if (TRUE == isNewSpecificProblem)
	{
		isNewProblem = TRUE;
		errStr << "- Power off ";
	}

	// ===== Reset problem
	if (TRUE == isResetting)
	{
		isNewSpecificProblem = AddResettingActiveAlarmNonMfa(isToChangeState, slotId, compType);
		if (TRUE == isNewSpecificProblem)
		{
			isNewProblem     = TRUE;
			isFailureProblem = TRUE;
			errStr << "- Resetting ";
		}
	}
	else
	{
		description = "Failure problem in ";
		description+= compName;
		isNewSpecificProblem = UpdateSpecificActiveAlarmNonMfa(SM_COMP_FAILURE_BITMASK, problemBitmask, AA_FAILURE_PROBLEM, isToChangeState, slotId, compType, description);
		if (TRUE == isNewSpecificProblem)
		{
			isNewProblem     = TRUE;
			isFailureProblem = TRUE;
			errStr << "- Failure ";
		}
	}


	// ===== not any known problem
	description = "Unspecified problem in ";
	description+= compName;
	isNewSpecificProblem = UpdateSpecificActiveAlarmNonMfa(SM_COMP_OTHER_BITMASK, problemBitmask, AA_OTHER_PROBLEM, isToChangeState, slotId, compType, description, TRUE);
	if (TRUE == isNewSpecificProblem)
	{
		isNewProblem = TRUE;
		errStr << "- Other ";
	}


	// print to logger (if needed)
	if (TRUE == isNewProblem)
	{
		TRACESTR(eLevelInfoNormal) << errStr.GetString();
	}

	return isFailureProblem;
}

//////////////////////////////////////////////////////////////////////
BOOL CSwitchTask::UpdateSpecificActiveAlarmNonMfa( APIU32 specificBitMask, APIU32 problemBitmask, WORD errCode, BOOL isToChangeState,
                                                   DWORD slotId, eShmComponentType compType, string description, BOOL isFaultOnly )
{
	DWORD displayBoardId;
	WORD faultType;
	BOOL isNewSpecificProblem = FALSE;

	if ( specificBitMask & problemBitmask )
	{

		TRACESTR(eLevelInfoNormal) << "\nCSwitchTask::UpdateSpecificActiveAlarmNonMfa - add active alarm on slot id: " << slotId
		<<"specificBitMask "<<specificBitMask;
		/****************************************************************************************************/
		/* 15.11.11  VNGFE-4848 added by Rachel cohen - For Error voltage problem for switch AA should be   */
		/* displayboard 17 and not on slotid 5.                                                             */
		/****************************************************************************************************/

		if (compType == eShmComp_SwitchBoard)
		{
			 displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(slotId, SWITCH_SUBBOARD_ID);
			 TRACESTR(eLevelInfoNormal) << "\nCSwitchTask::UpdateSpecificActiveAlarmNonMfa - comp type switch slot id: " << slotId
										          		        	   			                       <<"displayBoardId"<<displayBoardId <<"specificBitMask "<<specificBitMask;


			 WORD faultType = ConvertShelfMngrComponentTypeToFaultType(compType);
			 if (!isFaultOnly) {
				if (false == IsActiveAlarmExistByErrorCodeUserId(errCode,
						displayBoardId)) {
					AddActiveAlarmSwitch("UpdateSpecificActiveAlarmNonMfa",
							isToChangeState, FAULT_MPL_SUBJECT, errCode,
							MAJOR_ERROR_LEVEL, description, true, true,
							displayBoardId, displayBoardId, faultType);
				}

			 } else {
				if (false == IsActiveAlarmFaultOnlyExistByErrorCodeUserId(
						errCode, displayBoardId)) {
					AddActiveAlarmFaultOnlySwitch(
							"UpdateSpecificActiveAlarmNonMfa",
							FAULT_MPL_SUBJECT, errCode, MAJOR_ERROR_LEVEL,
							description, displayBoardId, displayBoardId,
							faultType);
				}

			 }
		}
		/****************************************************************************************************/
		/* 8.3.11  added by Ofir Nissel - For Error voltage problem for different Power supplies (PWR1\PWR2)*/
		/* If there was two pwr supplies problems (PWR1 and PWR2) there only one active alarm was shown		*/
		/****************************************************************************************************/
		else if ( false == IsActiveAlarmOrFaultOnlyExistByErrorCodeUserId(errCode, slotId, isFaultOnly) )
		{
			isNewSpecificProblem = TRUE;

			if ((SM_COMP_RTM_OR_ISDN_MISSING_BITMASK == specificBitMask) || ((SM_COMP_POWER_OFF_BITMASK == specificBitMask) && (compType == eShmComp_RtmLan)))
			{

				 displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(slotId, RTM_ISDN_SUBBOARD_ID);

				 TRACESTR(eLevelInfoNormal) << "\nCSwitchTask::UpdateSpecificActiveAlarmNonMfa - add active alarm on slot id: " << slotId
						          		        	   			                       <<"displayBoardId"<<displayBoardId <<"specificBitMask "<<specificBitMask;

				 faultType = ConvertShelfMngrComponentTypeToFaultType(compType);
				 if (!isFaultOnly) {
					AddActiveAlarmSwitch("UpdateSpecificActiveAlarmNonMfa",
							isToChangeState, FAULT_CARD_SUBJECT, errCode,
							MAJOR_ERROR_LEVEL, description, true, true,
							displayBoardId, displayBoardId, faultType);
				 } else {
					AddActiveAlarmFaultOnlySwitch(
							"UpdateSpecificActiveAlarmNonMfa",
							FAULT_CARD_SUBJECT, errCode, MAJOR_ERROR_LEVEL,
							description, displayBoardId, displayBoardId,
							faultType);
				 }
			}
			else
			{
				 displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(m_boardId, m_subBoardId);


			    WORD faultType = ConvertShelfMngrComponentTypeToFaultType(compType);

				if (!isFaultOnly) {
					AddActiveAlarmSwitch("UpdateSpecificActiveAlarmNonMfa",
							isToChangeState, FAULT_MPL_SUBJECT, errCode,
							MAJOR_ERROR_LEVEL, description, true, true, slotId,
							displayBoardId, faultType);
				} else {
					AddActiveAlarmFaultOnlySwitch(
							"UpdateSpecificActiveAlarmNonMfa",
							FAULT_MPL_SUBJECT, errCode, MAJOR_ERROR_LEVEL,
							description, slotId, displayBoardId, faultType);
				}
			}
		}

	}
	else
	{
		if ( (SM_COMP_RTM_OR_ISDN_MISSING_BITMASK == specificBitMask)|| ((SM_COMP_POWER_OFF_BITMASK == specificBitMask) && (compType == eShmComp_RtmLan)))
		{
			 displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(m_boardId, RTM_ISDN_SUBBOARD_ID);
			 RemoveActiveAlarmSwitchByErrorCodeUserId("UpdateSpecificActiveAlarmNonMfa", isToChangeState, FAULT_MPL_SUBJECT, errCode, displayBoardId, isFaultOnly);
		}
		else
		    RemoveActiveAlarmSwitchByErrorCodeUserId("UpdateSpecificActiveAlarmNonMfa", isToChangeState, FAULT_MPL_SUBJECT, errCode, slotId, isFaultOnly);
	}

	return isNewSpecificProblem;
}

//////////////////////////////////////////////////////////////////////
BOOL CSwitchTask::AddResettingActiveAlarmNonMfa(BOOL isToChangeState, DWORD slotId, eShmComponentType compType)
{
	BOOL isNewResettingProblem = FALSE;

	if ( false == IsActiveAlarmExistByErrorCode(AA_FAILURE_PROBLEM) )
	{
		isNewResettingProblem = TRUE;

		WORD faultType = ConvertShelfMngrComponentTypeToFaultType(compType);
		DWORD displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(m_boardId, m_subBoardId);
		AddActiveAlarmSwitch( "UpdateResettingActiveAlarmNonMfa", isToChangeState, FAULT_MPL_SUBJECT, AA_FAILURE_PROBLEM,
							  MAJOR_ERROR_LEVEL, "Resetting component", true, true, slotId, displayBoardId, faultType );
	}

	return isNewResettingProblem;
}

void CSwitchTask::TreatSmKeepAliveFailure()
{
	// ===== 1. produce ActiveAlarm/Faults
	DWORD displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(m_boardId, m_subBoardId);
	AddActiveAlarmSwitch( "TreatSmKeepAliveFailure", YES, FAULT_CARD_SUBJECT, AA_NO_CONNECTION_WITH_CARD,
	                       MAJOR_ERROR_LEVEL,
	                       "No connection with Switch",
	                       true,
	                       true,
	                       displayBoardId, //as 'userId' (token)
	                       displayBoardId,
	                       FAULT_TYPE_SWITCH );

	// ===== 2. to Logger
	TRACESTR(eLevelInfoNormal) << "\nCSwitchTask::TreatSmKeepAliveFailure - No connection with Switch";
	m_pProcess->PrintLastKeepAliveTimes(m_boardId, m_lastKeepAliveReqTime, m_lastKeepAliveIndTime);

	// ===== 3. ask reset from Daemon
	COsQueue& rcvMbx = GetRcvMbx();
	m_pProcess->TurnMfaTaskToZombie(&rcvMbx); // no messages will be received by this task

	m_pProcess->SendResetReqToDaemon("No connection with Switch");
}

//////////////////////////////////////////////////////////////////////
void CSwitchTask::SelfKill()
{
//	TRACESTR(eLevelInfoNormal) << "CSwitchTask::SelfKill";

	// ===== 1.  clear entry in array
	COsQueue& rcvMbx = GetRcvMbx();
	m_pProcess->TurnMfaTaskToZombie(&rcvMbx);

	// ===== 2.  destroy timer
	//DestroyTimer();

	// ===== 3. call father's SelfKill
	CTaskApp::SelfKill();
}

void CSwitchTask::IgnoreMessage()
{
	// (nothing to be done)
}

//////////////////////////////////////////////////////////////////////
void CSwitchTask::SetCardMngrStructData(const char *data)
{
	m_cardMngr.SetData(data);
}

//////////////////////////////////////////////////////////////////////
void CSwitchTask::SetCardMngrStructData(CCardMngrLoaded* other)
{
	m_cardMngr.SetData(other);
}

//////////////////////////////////////////////////////////////////////
CCardMngrLoaded CSwitchTask::GetCardMngr()
{
	return m_cardMngr;
}

//////////////////////////////////////////////////////////////////////
void CSwitchTask::SetBoardId(DWORD id)
{
	m_boardId = id;
	CreateTaskName();
}

//////////////////////////////////////////////////////////////////////
DWORD CSwitchTask::GetBoardId()
{
	return m_boardId;
}

//////////////////////////////////////////////////////////////////////
void CSwitchTask::SetSubBoardId(DWORD id)
{
	m_subBoardId = id;
	CreateTaskName();
}

//////////////////////////////////////////////////////////////////////
DWORD CSwitchTask::GetSubBoardId()
{
	return m_subBoardId;
}

//////////////////////////////////////////////////////////////////////
WORD CSwitchTask::ConvertShelfMngrComponentTypeToFaultType(eShmComponentType compType)
{
	switch (compType)
	{
		case eShmComp_SwitchBoard:
			return FAULT_TYPE_SWITCH;

		case eShmComp_MfaBoard1:
		case eShmComp_MfaBoard2:
		case eShmComp_MfaBoard3:
		case eShmComp_MfaBoard4:
		case eShmComp_MfaBoard5:
		case eShmComp_MfaBoard6:
		case eShmComp_MfaBoard7:
		case eShmComp_MfaBoard8:

		case eShmComp_RtmIsdn:
		case eShmComp_RtmLan:
			return FAULT_TYPE_MPM;

		case eShmComp_McmsCpu:
			return FAULT_TYPE_MCMS_CPU;

		case eShmComp_Fan:
			return FAULT_TYPE_FAN;

		case eShmComp_PowerSupply:
			return FAULT_TYPE_PWR_SPLY;

		case eShmComp_Lan:
			return FAULT_TYPE_LAN;

		case eShmComp_Backplane:
			return FAULT_TYPE_BACKPLANE;

		case eShmComp_Iam:
			return FAULT_TYPE_IAM;

		case eShmComp_Fsm4000:
			return FAULT_TYPE_FSM4000;

		default:
			return FAULT_TYPE_ILLEGAL;
	} // end switch
}

//////////////////////////////////////////////////////////////////////
char* CSwitchTask::ConvertShelfMngrComponentProblemTypeToString(APIU32 problemType)
{
	switch (problemType)
	{
		case eShmCompVoltage:
		{
			return "Voltage";
		}

		case eShmCompTemperature:
		{
			return "Temperature";
		}

		case eShmCompReset:
		{
			return "Reset";
		}

		case eShmCompOtherProblem:
		{
			return "Other";
		}

		default:
		{
			return NULL;
		}
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////
eShmComponentType CSwitchTask::ConvertShelfMngrStringToCompType(char *compType)
{
	if ( ( !strncmp(compType, "SWITCH", strlen("SWITCH")) ) ||
	     ( !strncmp(compType, "RTM IP", strlen("RTM IP")) ) ||
	     ( !strncmp(compType, "RTM-IP4000", strlen("RTM-IP4000")) )  ||
	     ( !strncmp(compType, "RTM-IP1500", strlen("RTM-IP1500")) )  ||
	     ( !strncmp(compType, "RTM IP+", strlen("RTM IP+")) ))
	{
		return eShmComp_SwitchBoard;
	}
	else if ( ( !strncmp(compType, "MPMY", strlen("MPMY")))  ||
			( !strncmp(compType, "MPMX", strlen("MPMX"))) ||
			( !strncmp(compType, "MPMRX-H", strlen("MPMRX-H"))) ||
			( !strncmp(compType, "MPMRX-F", strlen("MPMRX-F"))) ||
	        ( !strncmp(compType, "MPMRX", strlen("MPMRX"))) ||
			( !strncmp(compType, "MPMYPARTIAL", strlen("MPMYPARTIAL"))) ||
			( !strncmp(compType, "MFA",   strlen("MFA"))   )			||
	          ( !strncmp(compType, "MPM-H", strlen("MPM-H")) )			||
	          ( !strncmp(compType, "MPM-F", strlen("MPM-F")) )			||
	          ( !strncmp(compType, "MPM+20", strlen("MPM+20")) )		||
	          ( !strncmp(compType, "MPM+40", strlen("MPM+40")) )		||
	          ( !strncmp(compType, "MPM+80", strlen("MPM+80")) )		||
	          ( !strncmp(compType, "MPM_PLUS", strlen("MPM_PLUS")) )	||
	          ( !strncmp(compType, "MPM+MEZZANINE_A", strlen("MPM+MEZZANINE_A")) )	||
	          ( !strncmp(compType, "MPM+MEZZANINE_B", strlen("MPM+MEZZANINE_B")) )  ||
	          ( !strncmp(compType, "MPMX_CARRIER", strlen("MPMX_CARRIER")) )        ||
	          ( !strncmp(compType, "MPMX_80", strlen("MPMX_80"))) ||
	          ( !strncmp(compType, "MPMX_40", strlen("MPMX_40"))) )
	{
		return eShmComp_MfaBoard1; // just to identify it's an Mfa/Barak board (the exact type doesn't matter)
	}
	else if (!strncmp(compType, "RTM_ISDN", strlen("RTM_ISDN")))
	{
		return eShmComp_RtmIsdn;
	}
	else if (!strncmp(compType, "RTM_LAN", strlen("RTM_LAN")) )
	{
		return eShmComp_RtmLan;
	}
	else if ( !strncmp(compType, "FANS", strlen("FANS")) )
	{
		return eShmComp_Fan;
	}
	else if ( !strncmp(compType, "PWR", strlen("PWR")) )
	{
		return eShmComp_PowerSupply;
	}
	else if ( ( !strncmp(compType, "Backplane", strlen("Backplane")) )	||
              ( !strncmp(compType, "BACKPLANE", strlen("BACKPLANE")) )	||
              ( !strncmp(compType, "BackplaneY", strlen("BackplaneY")) )	||
              ( !strncmp(compType, "BACKPLANEY", strlen("BACKPLANEY")) )	||
              ( !strncmp(compType, "Chassis",   strlen("Chassis"))   )  )
	{
		return eShmComp_Backplane;
	}
	else if ( ( !strncmp(compType, "CNTL", strlen("CNTL"))			) 	||
			  ( !strncmp(compType, "CNTL+", strlen("CNTL+")))			||
			  ( !strncmp(compType, "CNTL_PLUS", strlen("CNTL_PLUS")))   ||
			  ( !strncmp(compType, "CNTL4000", strlen("CNTL4000")))     ||
			  ( !strncmp(compType, "CNTL1500", strlen("CNTL1500"))) )
	{
		return eShmComp_McmsCpu;
	}
	else if ( !strncmp(compType, "LAN", strlen("LAN"))  			||
		    ( !strncmp(compType, "RTM LAN", strlen("RTM LAN")))		||
		    ( !strncmp(compType, "RTM_LAN", strlen("RTM_LAN"))))
	{
		return eShmComp_Lan;
	}
	else if ( !strncmp(compType, "IAM", strlen("IAM")) )
	{
		return eShmComp_Iam;
	}
	else if ( !strncmp(compType, "FSM4000", strlen("FSM4000")) 		||
			  !strncmp(compType, "SHOVAL", strlen("SHOVAL")) )
	{
		return eShmComp_Fsm4000;
	}
	else
	{
		return eShmComp_Illegal;
	}
}


//////////////////////////////////////////////////////////////////////
eShmComponentType CSwitchTask::ConvertShelfMngrStringToSpecificCompType(char *compType)
{
	if ( ( !strncmp(compType, "SWITCH", strlen("SWITCH")) ) ||
	     ( !strncmp(compType, "RTM IP", strlen("RTM IP")) ) ||
	     ( !strncmp(compType, "RTM-IP4000", strlen("RTM-IP4000")) )  ||
	     ( !strncmp(compType, "RTM-IP1500", strlen("RTM-IP1500")) )  ||
	     ( !strncmp(compType, "RTM IP+", strlen("RTM IP+")) ))
	{
		return eShmComp_SwitchBoard;
	}

	else if ( ( !strncmp(compType, "MPMY", strlen("MPMY")))  ||
				( !strncmp(compType, "MPMX", strlen("MPMX"))) ||
				( !strncmp(compType, "MPMYPARTIAL", strlen("MPMYPARTIAL"))) ||
		          ( !strncmp(compType, "MPMX_CARRIER", strlen("MPMX_CARRIER")) )        ||
		          ( !strncmp(compType, "MPMX_80", strlen("MPMX_80"))) ||
		          ( !strncmp(compType, "MPMX_40", strlen("MPMX_40"))) )
		{
			return eShmComp_MfaMpmx; // just to identify it's an Mfa/Barak board (the exact type doesn't matter)
		}


	else if (
				( !strncmp(compType, "MPMRX-H", strlen("MPMRX-H"))) ||
				( !strncmp(compType, "MPMRX-F", strlen("MPMRX-F"))) ||
		        ( !strncmp(compType, "MPMRX", strlen("MPMRX")))
				 )
		{
			return eShmComp_MfaMpmRx; // just to identify it's an Mfa/Barak board (the exact type doesn't matter)
		}


	else if ( ( !strncmp(compType, "MPMY", strlen("MPMY")))  ||
			( !strncmp(compType, "MPMX", strlen("MPMX"))) ||
			( !strncmp(compType, "MPMRX-H", strlen("MPMRX-H"))) ||
			( !strncmp(compType, "MPMRX-F", strlen("MPMRX-F"))) ||
	        ( !strncmp(compType, "MPMRX", strlen("MPMRX"))) ||
			( !strncmp(compType, "MPMYPARTIAL", strlen("MPMYPARTIAL"))) ||
			( !strncmp(compType, "MFA",   strlen("MFA"))   )			||
	          ( !strncmp(compType, "MPM-H", strlen("MPM-H")) )			||
	          ( !strncmp(compType, "MPM-F", strlen("MPM-F")) )			||
	          ( !strncmp(compType, "MPM+20", strlen("MPM+20")) )		||
	          ( !strncmp(compType, "MPM+40", strlen("MPM+40")) )		||
	          ( !strncmp(compType, "MPM+80", strlen("MPM+80")) )		||
	          ( !strncmp(compType, "MPM_PLUS", strlen("MPM_PLUS")) )	||
	          ( !strncmp(compType, "MPM+MEZZANINE_A", strlen("MPM+MEZZANINE_A")) )	||
	          ( !strncmp(compType, "MPM+MEZZANINE_B", strlen("MPM+MEZZANINE_B")) )  ||
	          ( !strncmp(compType, "MPMX_CARRIER", strlen("MPMX_CARRIER")) )        ||
	          ( !strncmp(compType, "MPMX_80", strlen("MPMX_80"))) ||
	          ( !strncmp(compType, "MPMX_40", strlen("MPMX_40"))) )
	{
		return eShmComp_MfaBoard1; // just to identify it's an Mfa/Barak board (the exact type doesn't matter)


	}

	else if (!strncmp(compType, "RTM_ISDN9_10G", strlen("RTM_ISDN9_10G")))
			{
				return eShmComp_RtmIsdn9_10G;
			}

	else if (!strncmp(compType, "RTM_ISDN9", strlen("RTM_ISDN9")))
			{
				return eShmComp_RtmIsdn9;
			}

	else if (!strncmp(compType, "RTM_ISDN", strlen("RTM_ISDN")))
	{
		return eShmComp_RtmIsdn;
	}

	else if (!strncmp(compType, "RTM_LAN4_10G", strlen("RTM_LAN4_10G")) )
			{
				return eShmComp_RtmLan4_10G;
			}

	else if (!strncmp(compType, "RTM_LAN4", strlen("RTM_LAN4")) )
			{
				return eShmComp_RtmLan4;
			}

	else if (!strncmp(compType, "RTM_LAN", strlen("RTM_LAN")) )
	{
		return eShmComp_RtmLan;
	}


	else if ( !strncmp(compType, "FANS", strlen("FANS")) )
	{
		return eShmComp_Fan;
	}
	else if ( !strncmp(compType, "PWR", strlen("PWR")) )
	{
		return eShmComp_PowerSupply;
	}
	else if ( ( !strncmp(compType, "Backplane", strlen("Backplane")) )	||
              ( !strncmp(compType, "BACKPLANE", strlen("BACKPLANE")) )	||
              ( !strncmp(compType, "BackplaneY", strlen("BackplaneY")) )	||
              ( !strncmp(compType, "BACKPLANEY", strlen("BACKPLANEY")) )	||
              ( !strncmp(compType, "Chassis",   strlen("Chassis"))   )  )
	{
		return eShmComp_Backplane;
	}
	else if ( ( !strncmp(compType, "CNTL", strlen("CNTL"))			) 	||
			  ( !strncmp(compType, "CNTL+", strlen("CNTL+")))			||
			  ( !strncmp(compType, "CNTL_PLUS", strlen("CNTL_PLUS")))   ||
			  ( !strncmp(compType, "CNTL4000", strlen("CNTL4000")))     ||
			  ( !strncmp(compType, "CNTL1500", strlen("CNTL1500"))) )
	{
		return eShmComp_McmsCpu;
	}
	else if ( !strncmp(compType, "LAN", strlen("LAN"))  			||
		    ( !strncmp(compType, "RTM LAN", strlen("RTM LAN")))		||
		    ( !strncmp(compType, "RTM_LAN", strlen("RTM_LAN"))))
	{
		return eShmComp_Lan;
	}
	else if ( !strncmp(compType, "IAM", strlen("IAM")) )
	{
		return eShmComp_Iam;
	}
	else if ( !strncmp(compType, "FSM4000", strlen("FSM4000")) 		||
			  !strncmp(compType, "SHOVAL", strlen("SHOVAL")) )
	{
		return eShmComp_Fsm4000;
	}
	else
	{
		return eShmComp_Illegal;
	}
}

//////////////////////////////////////////////////////////////////////
eCardType CSwitchTask::ConvertShelfMngrStringToCardType(char *compType)
{
	if ( ( !strncmp(compType, "SWITCH", strlen("SWITCH")) ) ||
	     ( !strncmp(compType, "RTM IP", strlen("RTM IP")) ) ||
	     ( !strncmp(compType, "RTM-IP4000", strlen("RTM-IP4000")) )  ||
	     ( !strncmp(compType, "RTM-IP1500", strlen("RTM-IP1500")) ))
	{
		return eSwitch;
	}
	else if ( !strncmp(compType, "MPMRX-H",   strlen("MPMRX-H"))   )
	{
	    return eMpmRx_Half;
	}
	else if ( !strncmp(compType, "MPMRX-F",   strlen("MPMRX-F"))   )
	{
	    return eMpmRx_Full;
	}
	else if ( !strncmp(compType, "MPMRX",   strlen("MPMRX"))   )
	{
	    return eMpmRx_Half;
	}
	else if ( ( !strncmp(compType, "MFA",   strlen("MFA"))   )	||
	          ( !strncmp(compType, "MPM-H", strlen("MPM-H")) )	)
	{
		return eMfa_13;
	}
	else if ( !strncmp(compType, "MPM-F", strlen("MPM-F")) )
	{
		return eMfa_26;
	}
	else if ( ( !strncmp(compType, "MPM_PLUS", strlen("MPM_PLUS")) )	||
	          ( !strncmp(compType, "MPM+20",   strlen("MPM+20"))   ) )
	{
		return eMpmPlus_20;
	}
	else if ( !strncmp(compType, "MPM+40", strlen("MPM+40")) )
	{
		return eMpmPlus_40;
	}
	else if ( !strncmp(compType, "MPM+80", strlen("MPM+80")) )
	{
		return eMpmPlus_80;
	}
	else if ( !strncmp(compType, "MPM+MEZZANINE_A", strlen("MPM+MEZZANINE_A")) )
	{
		return eMpmPlus_MezzanineA;
	}
	else if ( !strncmp(compType, "MPM+MEZZANINE_B", strlen("MPM+MEZZANINE_B")) )
	{
		return eMpmPlus_MezzanineB;
	}
	else if ( !strncmp(compType, "RTM_ISDN", strlen("RTM_ISDN")) )
	{
		return eRtmIsdn;
	}
	else if ( !strncmp(compType, "RTM_ISDN9", strlen("RTM_ISDN9")) )
	{
		return eRtmIsdn_9PRI;
	}
	else if ( !strncmp(compType, "RTM_ISDN9_10G", strlen("RTM_ISDN9_10G")) )
	{
		return eRtmIsdn_9PRI_10G;
	}
	else if ( !strncmp(compType, "MPMYPARTIAL", strlen("MPMYPARTIAL")) )
	{
		return eMpmx_20;
	}
	else if ( (!strncmp(compType, "MPMX", strlen("MPMX")) )  ||
			(!strncmp(compType, "MPMY", strlen("MPMY")) ) )
	{
		return eMpmx_40;
	}
	else if ( !strncmp(compType, "MPMX_80", strlen("MPMX_80")) )
	{
		return eMpmx_80;
	}
	else if ( !strncmp(compType, "MPMX_40", strlen("MPMX_40")) )
	{
		return eMpmx_40;
	}
	else
	{
		return eEmpty;
	}
}

//////////////////////////////////////////////////////////////////////
BOOL  CSwitchTask::IsMpmOrMpmPlusSmComponent(eShmComponentType compType)
{
	BOOL isMpm = NO;

	if ( (compType == eShmComp_MfaBoard1)	||
	     (compType == eShmComp_MfaBoard2)	||
	     (compType == eShmComp_MfaBoard3)	||
	     (compType == eShmComp_MfaBoard4)	||
	     (compType == eShmComp_MfaBoard5)	||
	     (compType == eShmComp_MfaBoard6)	||
	     (compType == eShmComp_MfaBoard7)	||
	     (compType == eShmComp_MfaBoard8)	||
	     (compType == eShmComp_RtmIsdn) )

	{
		isMpm = YES;
	}

	return isMpm;
}

//////////////////////////////////////////////////////////////////////
bool CSwitchTask::IsMpmSmComponent(SM_COMPONENT_STATUS_S statusStruct)
{
	eCardType curType = ConvertShelfMngrStringToCardType( (char*)(statusStruct.sSmCompName) );

	bool isMpm = m_pProcess->IsMpmCard(curType);

	return isMpm;
}

//////////////////////////////////////////////////////////////////////
DWORD CSwitchTask::AddActiveAlarmSwitch( CSmallString callerStr, BOOL isToChangeState, BYTE subject,
                                       DWORD errorCode, BYTE errorLevel, string description, bool isForEma,
                                       bool isForFaults, DWORD userId, DWORD boardId, WORD theType)
{
	// ===== 1. add ActiveAlarm
	AddActiveAlarm( subject, errorCode, errorLevel, description,
	                isForEma, isForFaults, userId, boardId, 0/*as unitId*/, theType);

	// ===== 2. add status (in Cards' DB)
	//m_pProcess->GetCardsMonitoringDB()->AddStatus(subject, errorCode, errorLevel, boardId, unitId);
	//NEW_STATUS_LIST
	m_pProcess->GetCardsMonitoringDB()->AddStatusNew( subject, errorCode, errorLevel, userId,
	                                                  boardId, m_subBoardId, 0/*as unitId*/, description, theType);

	// ===== 3. update card's status
	if (YES == isToChangeState)
	{
		UpdateCardStateAccordingToTaskState(callerStr);
	}

    return 0;
}

//////////////////////////////////////////////////////////////////////
DWORD CSwitchTask::AddActiveAlarmFaultOnlySwitch(CSmallString callerStr,
		BYTE subject, DWORD errorCode, BYTE errorLevel,
		string description, DWORD userId, DWORD boardId, WORD theType) {
	// ===== 1. add ActiveAlarm fault only
	AddActiveAlarmFaultOnly(subject, errorCode, errorLevel, description,
			userId, boardId, 0/*as unitId*/, theType);

	// ===== 2. add status (in Cards' DB)
	//m_pProcess->GetCardsMonitoringDB()->AddStatus(subject, errorCode, errorLevel, boardId, unitId);
	//NEW_STATUS_LIST
	m_pProcess->GetCardsMonitoringDB()->AddStatusNew(subject, errorCode,
			errorLevel, userId, boardId, m_subBoardId, 0/*as unitId*/,
			description, theType);

	// Unlike AddActiveAlarmSwitch No call to UpdateCardStateAccordingToTaskState is made because fault only alarm shoudnt change card state
	return 0;
}

//////////////////////////////////////////////////////////////////////
void CSwitchTask::RemoveActiveAlarmSwitchByErrorCode(CSmallString callerStr,
						     BOOL isToChangeState,
						     BYTE subject,
						     DWORD errorCode,
						     BOOL isFaultOnly)
{
	bool isActiveAlarmOrFaultExist;

	if (!isFaultOnly) {
		isActiveAlarmOrFaultExist = IsActiveAlarmExistByErrorCode(errorCode);
	}
	else {
		isActiveAlarmOrFaultExist = IsActiveAlarmFaultOnlyExistByErrorCode(errorCode);
	}

	if ( true == isActiveAlarmOrFaultExist )
	{
		if (!isFaultOnly) {
			// ===== 1. remove ActiveAlarm
			RemoveActiveAlarmByErrorCode(errorCode);
		} else {
			RemoveActiveAlarmFaultOnlyByErrorCode(errorCode);
		}
		// ===== 2. del status (from Cards' DB)
		//m_pProcess->GetCardsMonitoringDB()->DelStatus(subject, errorCode, MAJOR_ERROR_LEVEL, m_boardId);
		//NEW_STATUS_LIST
		m_pProcess->GetCardsMonitoringDB()->DelStatusNewByErrorCode(errorCode, m_boardId, m_subBoardId);

		// ===== 3. update card's status
		if (YES == isToChangeState && !isFaultOnly)
		{
			UpdateCardStateAccordingToTaskState(callerStr);
		}
	}
}

//////////////////////////////////////////////////////////////////////
void CSwitchTask::RemoveActiveAlarmSwitchByErrorCodeUserId(CSmallString callerStr,
                                                           BOOL isToChangeState,
                                                           WORD subject,
                                                           WORD errorCode,
                                                           DWORD userId,
                                                           BOOL isFaultOnly)
{
	bool isActiveAlarmOrFaultExist;
	if (!isFaultOnly) {
		isActiveAlarmOrFaultExist = IsActiveAlarmExistByErrorCodeUserId(errorCode, userId);

	} else {
		isActiveAlarmOrFaultExist = IsActiveAlarmFaultOnlyExistByErrorCodeUserId(errorCode, userId);
	}

	if ( true == isActiveAlarmOrFaultExist )
	{
		// ===== 1. remove ActiveAlarm
		if (!isFaultOnly) {
			// ===== 1. remove ActiveAlarm
			RemoveActiveAlarmByErrorCodeUserId(errorCode, userId);
		} else {
			RemoveActiveAlarmFaultOnlyByErrorCodeUserId(errorCode, userId);
		}

		// ===== 2. del status (from Cards' DB)
		//m_pProcess->GetCardsMonitoringDB()->DelStatus(subject, errorCode, MAJOR_ERROR_LEVEL, m_boardId);
		//NEW_STATUS_LIST
		m_pProcess->GetCardsMonitoringDB()->DelStatusNewByErrorCodeUserId(errorCode, userId, m_boardId, m_subBoardId);

		// ===== 3. update card's status
		if (YES == isToChangeState && !isFaultOnly)
		{
			UpdateCardStateAccordingToTaskState(callerStr);
		}
	}
}
//////////////////////////////////////////////////////////////////////

BOOL CSwitchTask::IsActiveAlarmOrFaultOnlyExistByErrorCodeUserId(
		BYTE errorLevel, DWORD userId, BOOL isFaultOnly) {
	if (!isFaultOnly) {
		return IsActiveAlarmExistByErrorCodeUserId(errorLevel, userId);
	} else {
		return IsActiveAlarmFaultOnlyExistByErrorCodeUserId(errorLevel, userId);
	}
}

//////////////////////////////////////////////////////////////////////
void CSwitchTask::UpdateCardStateAccordingToTaskState(CSmallString callerStr)
{
	eTaskStatus curTaskStatus = GetTaskStatus();
	eTaskState curTaskState = GetTaskState();
	// Rachel: Asks if there are any alarms on the Rtm Ip.
	bool faultExist = IsActiveAlarmExistByUserId(FIXED_DISPLAY_BOARD_ID_SWITCH);

	TRACESTR(eLevelInfoNormal) << "\nCSwitchTask::UpdateCardStateAccordingToTaskState \ncurTaskStatus " << curTaskStatus
	<<"\n curTaskState " << curTaskState
	<<"\nfaultExist " << faultExist;

	eCardState  curCardState  = m_pProcess->ConvertTaskStatusToCardState(curTaskStatus, curTaskState,faultExist);
	m_pProcess->GetCardsMonitoringDB()->SetCardState(m_boardId, m_subBoardId, curCardState);

	TRACESTR(eLevelInfoNormal) << "\nCSwitchTask::" << callerStr.GetString()
	                       << "\nCard on boardId " << m_boardId << " updated its state to " << curCardState;
}

//////////////////////////////////////////////////////////////////////
void CSwitchTask::AddFilterOpcodePoint()
{
	AddFilterOpcodeToQueue(SM_KEEP_ALIVE_IND);
}

//////////////////////////////////////////////////////////////////////
void CSwitchTask::OnSpecMfaActiveAlarmReq(CSegment* pSeg)
{
	// ===== 1. get parameters from structure received
	DWORD mfaBoardId=0, mfaSubBoardId=0;
	*pSeg >> mfaBoardId
	      >> mfaSubBoardId;

	// ===== 2. transfer ActiveAlarms from Switch task to Mfa task
	COsQueue* mfaQueue = m_pProcess->GetMfaMbx(mfaBoardId, mfaSubBoardId);
	if (mfaQueue)
	{
		// ===== 1. prepare the data to be sent
		CSegment* pSegToBoard = new CSegment;

		ACTIVE_ALARMS_SPECIFIC_CARD_S* pAaStruct = new ACTIVE_ALARMS_SPECIFIC_CARD_S;
		FillActiveAlarmsSpecificCardStruct(pAaStruct, mfaBoardId);
		pSegToBoard->Put( (BYTE*)(pAaStruct), sizeof(ACTIVE_ALARMS_SPECIFIC_CARD_S) );

		// ==== 2. print to log
		CLargeString AaStructStr;
		m_pProcess->ActiveAlarmsStructAsString(pAaStruct, AaStructStr, mfaBoardId);
		TRACESTR(eLevelInfoNormal) << "\nCSwitchTask::TransferActiveAlarmsToMfa -"
		                              << AaStructStr.GetString();

		delete pAaStruct;

		// ==== 3. remove Alarms from Switch task (Mfa will produce them instead)
		//         'eShmComp_MfaBoard1' is sent as compType, just to identify it's an Mfa type
		RemoveActiveAlarmsByCompTypeUserId(eShmComp_MfaBoard1, mfaBoardId, FAULT_CARD_SUBJECT, YES);

		// ==== 4. send the msg
		CMfaApi mfaApi;
		mfaApi.CreateOnlyApi(*mfaQueue);
		mfaApi.SendMsg(pSegToBoard, SPEC_MFA_ACTIVE_ALARMS_IND);
	}
	else
	{
		TRACESTR(eLevelInfoNormal) << "\nCSwitchTask::TransferActiveAlarmsToMfa - "
		                       << "\nNo Mbx for Mfa  boardId: " << mfaBoardId << ", subBoardId: " << mfaSubBoardId;
	}
}

//////////////////////////////////////////////////////////////////////
void CSwitchTask::FillActiveAlarmsSpecificCardStruct(ACTIVE_ALARMS_SPECIFIC_CARD_S* pAaStruct, DWORD mfaBoardId)
{
	pAaStruct->isVoltageProblem				= NO;
	pAaStruct->isTemperatureMajorProblem	= NO;
	pAaStruct->isTemperatureCriticalProblem	= NO;
	pAaStruct->isFailureProblem				= NO;
	pAaStruct->isOtherProblem				= NO;
	pAaStruct->isPowerOffProblem            = NO;
	pAaStruct->isRtmIsdnMissingProblem      = NO;


	if ( true == IsActiveAlarmExistByErrorCodeUserId(AA_VOLTAGE_PROBLEM, mfaBoardId) )
		pAaStruct->isVoltageProblem = YES;

	DWORD displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(mfaBoardId, RTM_ISDN_SUBBOARD_ID);
	if ( true == IsActiveAlarmExistByErrorCodeUserId(AA_RTM_LAN_OR_ISDN_MISSING, displayBoardId) )
			pAaStruct->isRtmIsdnMissingProblem = YES;


	if ( true == IsActiveAlarmExistByErrorCodeUserId(AA_TEMPERATURE_MAJOR_PROBLEM, mfaBoardId) )
		pAaStruct->isTemperatureMajorProblem = YES;

	if ( true == IsActiveAlarmExistByErrorCodeUserId(AA_TEMPERATURE_CRITICAL_PROBLEM, mfaBoardId) )
		pAaStruct->isTemperatureCriticalProblem = YES;

	if ( true == IsActiveAlarmExistByErrorCodeUserId(AA_POWER_OFF_PROBLEM, mfaBoardId) )
		pAaStruct->isPowerOffProblem = YES;

	if ( true == IsActiveAlarmExistByErrorCodeUserId(AA_FAILURE_PROBLEM, mfaBoardId) )
		pAaStruct->isFailureProblem = YES;

	if ( true == IsActiveAlarmFaultOnlyExistByErrorCodeUserId(AA_OTHER_PROBLEM, mfaBoardId) )
		pAaStruct->isOtherProblem = YES;

}

/////////////////////////////////////////////////////////////////////////////
void CSwitchTask::SwapEndian(ETH_SETTINGS_SPEC_S *pSpecSettingsStruct)
{
	pSpecSettingsStruct->monitoringParams.ulRxPackets = SWAPL(pSpecSettingsStruct->monitoringParams.ulRxPackets);
	pSpecSettingsStruct->monitoringParams.ulRxBadPackets = SWAPL(pSpecSettingsStruct->monitoringParams.ulRxBadPackets);
	pSpecSettingsStruct->monitoringParams.ulRxCRC = SWAPL(pSpecSettingsStruct->monitoringParams.ulRxCRC);
	pSpecSettingsStruct->monitoringParams.ulRxOctets = SWAPL(pSpecSettingsStruct->monitoringParams.ulRxOctets);
	pSpecSettingsStruct->monitoringParams.ulMaxRxPackets = SWAPL(pSpecSettingsStruct->monitoringParams.ulMaxRxPackets);
	pSpecSettingsStruct->monitoringParams.ulMaxRxBadPackets = SWAPL(pSpecSettingsStruct->monitoringParams.ulMaxRxBadPackets);
	pSpecSettingsStruct->monitoringParams.ulMaxRxCRC = SWAPL(pSpecSettingsStruct->monitoringParams.ulMaxRxCRC);
	pSpecSettingsStruct->monitoringParams.ulMaxRxOctets = SWAPL(pSpecSettingsStruct->monitoringParams.ulMaxRxOctets);
	pSpecSettingsStruct->monitoringParams.ulRxPackets = SWAPL(pSpecSettingsStruct->monitoringParams.ulRxPackets);
	pSpecSettingsStruct->monitoringParams.ulRxPackets = SWAPL(pSpecSettingsStruct->monitoringParams.ulRxPackets);
	pSpecSettingsStruct->monitoringParams.ulTxPackets = SWAPL(pSpecSettingsStruct->monitoringParams.ulTxPackets);
	pSpecSettingsStruct->monitoringParams.ulTxBadPackets = SWAPL(pSpecSettingsStruct->monitoringParams.ulTxBadPackets);
	pSpecSettingsStruct->monitoringParams.ulTxFifoDrops = SWAPL(pSpecSettingsStruct->monitoringParams.ulTxFifoDrops);
	pSpecSettingsStruct->monitoringParams.ulTxOctets = SWAPL(pSpecSettingsStruct->monitoringParams.ulTxOctets);
	pSpecSettingsStruct->monitoringParams.ulMaxTxPackets = SWAPL(pSpecSettingsStruct->monitoringParams.ulMaxTxPackets);
	pSpecSettingsStruct->monitoringParams.ulMaxTxBadPackets = SWAPL(pSpecSettingsStruct->monitoringParams.ulMaxTxBadPackets);
	pSpecSettingsStruct->monitoringParams.ulMaxTxFifoDrops = SWAPL(pSpecSettingsStruct->monitoringParams.ulMaxTxFifoDrops);
	pSpecSettingsStruct->monitoringParams.ulMaxTxOctets = SWAPL(pSpecSettingsStruct->monitoringParams.ulMaxTxOctets);
	pSpecSettingsStruct->monitoringParams.ulRxPackets = SWAPL(pSpecSettingsStruct->monitoringParams.ulRxPackets);
	pSpecSettingsStruct->monitoringParams.ulRxPackets = SWAPL(pSpecSettingsStruct->monitoringParams.ulRxPackets);
	pSpecSettingsStruct->monitoringParams.ulActLinkStatus = SWAPL(pSpecSettingsStruct->monitoringParams.ulActLinkStatus);
	pSpecSettingsStruct->monitoringParams.ulActLinkMode = SWAPL(pSpecSettingsStruct->monitoringParams.ulActLinkMode);
	pSpecSettingsStruct->monitoringParams.ulActLinkAutoNeg = SWAPL(pSpecSettingsStruct->monitoringParams.ulActLinkAutoNeg);
	pSpecSettingsStruct->monitoringParams.ulAdvLinkMode = SWAPL(pSpecSettingsStruct->monitoringParams.ulAdvLinkMode);
	pSpecSettingsStruct->monitoringParams.ulAdvLinkAutoNeg = SWAPL(pSpecSettingsStruct->monitoringParams.ulAdvLinkAutoNeg);

	//802.1x
	pSpecSettingsStruct->monitoringParams.e802_1xSuppPortStatus = SWAPL(pSpecSettingsStruct->monitoringParams.e802_1xSuppPortStatus);
	pSpecSettingsStruct->monitoringParams.e802_1xMethod = SWAPL(pSpecSettingsStruct->monitoringParams.e802_1xMethod);
	pSpecSettingsStruct->monitoringParams.e802_1xFailReason = SWAPL(pSpecSettingsStruct->monitoringParams.e802_1xFailReason);

}


/////////////////////////////////////////////////////////////////////////////
void CSwitchTask::OnEthSettingInd(CSegment* pSeg)
{
	DWORD displayBoardId ;

	// ===== 1. get the parameters from the structure received
	ETH_SETTINGS_SPEC_S* pEthSettings = (ETH_SETTINGS_SPEC_S*)pSeg->GetPtr();

	// ===== 2. replace boardId with 'displayBoardId' (since it is used for monitoring)
	//when slot id is 1,2,3,4 we use the conversion table but when slotid is 5 we do not use the conversion table

	eProductType  curProductType  = CProcessBase::GetProcess()->GetProductType();

	if (curProductType != eProductTypeRMX2000)
	{
		if (pEthSettings->portParams.slotId==FIXED_BOARD_ID_SWITCH)
			pEthSettings->portParams.slotId = FIXED_DISPLAY_BOARD_ID_SWITCH;
		else
		{
		/*************************************************************************************************************************************/
			/* VNGR 14077 Rachel Cohen 8/3/10                                                                                                    */
			/* pEthSettings->portParams.portNum is 1 when isdn_lan and 2 if rtm_lan so it is an error to go to conversion table with that value  */
			/* meaning GetDisplayBoardId(pEthSettings->portParams.slotId, pEthSettings->portParams.portNum)                                      */
			/*************************************************************************************************************************************/

		 displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(pEthSettings->portParams.slotId, RTM_ISDN_SUBBOARD_ID);
	pEthSettings->portParams.slotId = displayBoardId;
		}
	}


	/*temptemptemp - for debugging*/
		//CEthernetSettingsStructWrapper tempStruct(true);
		//tempStruct.SetEthSettingsStruct(pEthSettings);
		//TRACESTR(eLevelInfoNormal) << "\nCSwitchTask::OnEthSettingInd - struct received from CM:\n" << tempStruct
						//		<< "\nulActLinkStatus: " << pEthSettings->monitoringParams.ulActLinkStatus
						//		<< "\nslot id: " << pEthSettings->portParams.slotId;

		/*temptemptemp - for debugging*/


	SwapEndian(pEthSettings);

	// ===== 3. update the process
	CEthernetSettingsStructWrappersList *pTheList = m_pProcess->GetEthernetSettingsStructsList();
	pTheList->SetSpecEthernetSettingsStructWrapper(pEthSettings);
}

void CSwitchTask::OnEthSettingClearMaxCountersInd(CSegment* pSeg)
{
	ETH_SETTINGS_STATE_S* pEthState = (ETH_SETTINGS_STATE_S*)pSeg->GetPtr();

	eEthSettingsState curState = (eEthSettingsState)(pEthState->configState);

	TRACESTR(eLevelInfoNormal) << "\nCSwitchTask::OnEthSettingClearMaxCountersInd - params received from CM:"
								  << "\nslotId:      " << (int)(pEthState->portParams.slotId)
								  << "\nportNum:     " << (int)(pEthState->portParams.portNum)
								  << "\nconfigState: " << (eEthSettingsState_ok == curState ? "ok" : "fail");

	m_pProcess->SendClearMaxCountersIndToSwitch(pEthState);
}

/////////////////////////////////////////////////////////////////////////////
void CSwitchTask::OnSlotsNumberingConversionTableInd(CSegment* pSeg)
{
	SLOTS_NUMBERING_CONVERSION_TABLE_S* pSlotsNumTable = (SLOTS_NUMBERING_CONVERSION_TABLE_S*)pSeg->GetPtr();
	m_pProcess->SetSlotsNumberingConversionTable(pSlotsNumTable);
	m_pProcess->GetSlotsNumberingConversionTable()->PrintData("CSwitchTask::OnSlotsNumberingConversionTableInd");

	SendSlotsNumberingConversionTableToProcesses();

}

/////////////////////////////////////////////////////////////////////////////
void CSwitchTask::SendSlotsNumberingConversionTableToProcesses()
{
	SendSlotsNumberingConversionTableToSpecProcess(eProcessMcuMngr);
	SendSlotsNumberingConversionTableToSpecProcess(eProcessResource);
	SendSlotsNumberingConversionTableToSpecProcess(eProcessRtmIsdnMngr);
}

/////////////////////////////////////////////////////////////////////////////
void CSwitchTask::SendSlotsNumberingConversionTableToSpecProcess(const eProcessType processType)
{
	TRACESTR(eLevelInfoNormal) << "\nCSwitchTask::SendSlotsNumberingConversionTableToSpecProcess"
						   << " (to " << ::ProcessTypeToString(processType) << ")";

	// ===== 1. fill the Segment with items from the list
	CSegment*  pRetParam = new CSegment;
	SLOTS_NUMBERING_CONVERSION_TABLE_S *pTable = m_pProcess->GetSlotsNumberingConversionTable()->GetStruct();
	pRetParam->Put( (BYTE*)pTable, sizeof(SLOTS_NUMBERING_CONVERSION_TABLE_S) );

	// ===== 2. send
	CManagerApi api(processType);
	api.SendMsg(pRetParam, CARDS_SLOTS_NUMBERING_CONVERSION_TABLE_IND);
}

//////////////////////////////////////////////////////////////////////
void CSwitchTask::OnNewSysCfgParamsInd(CSegment* pSeg)
{
	SYSCFG_PARAMS_S newSysCfgParamsStruct;
	memset(&newSysCfgParamsStruct, 0, sizeof newSysCfgParamsStruct);

	TRACESTR(eLevelInfoNormal) << "\nCSwitchTask::OnNewSysCfgParamsInd ";

	WORD boardId=0, subBoardId=0;
	*pSeg >> boardId
		  >> subBoardId
		  >> newSysCfgParamsStruct.unDebugMode
		  >> newSysCfgParamsStruct.unJITCMode
		  >> newSysCfgParamsStruct.unSeparatedNetworks
		  >> newSysCfgParamsStruct.unMultipleServices;

	// ===== 2. if it's a service from the suitable boardID and subBoardId...
	if ( (boardId == m_boardId) && (subBoardId == m_subBoardId) )
	{
		m_resetOnOldJitcInd = NO;
		m_isJitcMode = newSysCfgParamsStruct.unJITCMode;
		m_isSeparatedNetworks = newSysCfgParamsStruct.unSeparatedNetworks;
		m_isMultipleServices = newSysCfgParamsStruct.unMultipleServices;

		SendMessageToSwitch(SYSCFG_PARAMS_REQ, sizeof(SYSCFG_PARAMS_S), (char*)&newSysCfgParamsStruct);
	}
}

void CSwitchTask::OnOldSysCfgParamsInd(CSegment* pSeg)
{
	// ===== 1. fill object with data from structure received
	DWORD  structLen = sizeof(SYSCFG_PARAMS_S);
	SYSCFG_PARAMS_S oldSysCfgParamsStruct;
	memset(&oldSysCfgParamsStruct,0,structLen);
	pSeg->Get( (BYTE*)(&oldSysCfgParamsStruct), structLen );

	char jitc_data[100] = "";
	char separatedNet_data[100] = "";
	char multiple_data[100] = "";

	sprintf(jitc_data, "oldJITCMode=%d, m_isJitcMode=%d", oldSysCfgParamsStruct.unJITCMode, m_isJitcMode);
	sprintf(separatedNet_data, "oldSeparatedNetworks=%d, m_isSeparatedNetworks=%d", oldSysCfgParamsStruct.unSeparatedNetworks, m_isSeparatedNetworks);
	sprintf(multiple_data, "oldMultipleServices=%d, m_isMultipleServices=%d", oldSysCfgParamsStruct.unMultipleServices, m_isMultipleServices);

	TRACESTR(eLevelError) << "CSwitchTask::OnOldSysCfgParamsInd, " << jitc_data;
	TRACESTR(eLevelError) << "CSwitchTask::OnOldSysCfgParamsInd, " << separatedNet_data;
	TRACESTR(eLevelError) << "CSwitchTask::OnOldSysCfgParamsInd, " << multiple_data;

	// ===== 2. If value is different from the one sent to it - reset the system.
	// Reset system - only if indication is response on SysCfgParams during startup.
	// Do not reset system - if indication is response on SysCfgParams from updated System.cfg
	if(m_resetOnOldJitcInd)
	{
		if(oldSysCfgParamsStruct.unJITCMode != m_isJitcMode)
		{
			if(SYSCFG_PARAM_VAL_ERROR_READ == oldSysCfgParamsStruct.unJITCMode || SYSCFG_PARAM_VAL_ERROR_WRITE == oldSysCfgParamsStruct.unJITCMode)
			{
				TRACESTR(eLevelError) << "CMfaTask::OnOldSysCfgParamsInd - Mfa encountered an error while accessing flash!!!";
				std::string desc = (const char*)oldSysCfgParamsStruct.sDescription;
				if(m_isJitcMode)
					desc += " - Card is not in JITC mode !!!";
				AddActiveAlarmSwitch( "OnOldSysCfgParamsInd", YES, FAULT_CARD_SUBJECT, AA_CARDS_FLASH_ACCESS_PROBLEM, MAJOR_ERROR_LEVEL,
									desc, true, true,
									m_boardId, // boardId as 'userId' (token)
									m_boardId,
									FAULT_TYPE_SWITCH );
			}
			else
			{
				TRACESTR(eLevelError) << "CSwitchTask::OnOldSysCfgParamsInd - Switch held different JITC_MODE value, resetting system!!!";
				//reset system
				m_pProcess->SendResetReqToDaemon("JITC Mode changed.");
			}
		}
		else
		{
			if(oldSysCfgParamsStruct.unSeparatedNetworks != m_isSeparatedNetworks)
			{
				TRACESTR(eLevelError) << "CSwitchTask::OnOldSysCfgParamsInd - Switch held different SEPARATED_MANAGEMENT_NETWORK value, resetting system!!!";
				//reset system
				m_pProcess->SendResetReqToDaemon("Separated Management Networks configuration changed.");
			}

			else{

				if(oldSysCfgParamsStruct.unMultipleServices != m_isMultipleServices)
				{
					TRACESTR(eLevelError) << "CSwitchTask::OnOldSysCfgParamsInd - Switch held different Multiple Services value, resetting system!!!";
					//reset system
					//TODO - Ori - remove remark
					//m_pProcess->SendResetReqToDaemon("Multiple Services Networks configuration changed.");
				}
				else
					TRACESTR(eLevelError) << "CSwitchTask::OnOldSysCfgParamsInd - Switch held same SysCfg values.";
			}
		}
	}
}

void CSwitchTask::OnBadSpontaneousInd(CSegment* msg)
{
    // ===== 1. retrieve data from segment received
	MFA_BAD_SPONTANEOUS_IND_S dataStruct;
    memset(&dataStruct, 0, sizeof dataStruct);

    msg->Get((BYTE*)&dataStruct, sizeof dataStruct);


    // ===== 2. test string validity
    unsigned int len = ARRAYSIZE(dataStruct.badSpontaneousStruct.description) * sizeof(APIU32);

    unsigned char* ptrByteDesc = (unsigned char*)(dataStruct.badSpontaneousStruct.description);
    ptrByteDesc[len - 1] = '\0';

    CObjString::ReplaceByte(ptrByteDesc, len, '\n', ';');
    m_pProcess->TestStringValidity((char*)ptrByteDesc, len, __FUNCTION__);

    // ===== 3. add to log & fault
    const char* desc = (const char*)(dataStruct.badSpontaneousStruct.description);

    TRACEINTOFUNC << "reason: " << dataStruct.badSpontaneousStruct.reason
                  << ", description: " << desc << ", MfaBoardId: " << dataStruct.MfaBoardId;

    // Adds the string alarm to the Fault list in the EMA
    CHlogApi::TaskFault(FAULT_CARD_SUBJECT,
                        BAD_SPONTANEOUS_INDICATION,
                        MAJOR_ERROR_LEVEL,
                        desc,
                        TRUE,
                        dataStruct.MfaBoardId,
                        0,
                        FAULT_TYPE_MPM);
}


void CSwitchTask::OnLdapLoginRequest(CSegment* pSeg)
{
	// TRACESTR(eLogLevelDEBUG) << "CSwitchTask::OnLdapLoginRequest: " ;
	CSegment* pCopySeg = new CSegment(*pSeg);

	CManagerApi api(eProcessAuthentication);
	if (STATUS_OK == api.SendMsg(pCopySeg,SWITCH_LDAP_LOGIN_REQ))
	{
		TRACEINTOFUNC << "\nSent switch login req to authentication";
	}
	else {
		TRACEINTOFUNC << "\nFail to send switch login req to authentication";
	}
}

/////////////////////////////////////////////////////////////////////////////
void CSwitchTask::SendRtmLanIsdnParamsToProcesses(SLOTS_CONFIGURATION_S mfaBoardCnfArray[])
{
    SendRtmLanIsdnParamsToMcuMngr(mfaBoardCnfArray);
	SendRtmLanIsdnParamsToUtility(mfaBoardCnfArray);
}

///////////////////////////////////////////////////////////////////////////////
void CSwitchTask::SendRtmLanIsdnParamsToMcuMngr(SLOTS_CONFIGURATION_S mfaBoardCnfArray[])
{



    CSegment *pSeg = new CSegment;
    pSeg->Put((BYTE*)mfaBoardCnfArray, MAX_NUM_OF_SLOTS*sizeof(SLOTS_CONFIGURATION_S));

    //  send to manager task
    CManagerApi api(eProcessMcuMngr);
    api.SendMsg(pSeg, RTM_LANS_AND_ISDN_SLOT_IND);

}
///////////////////////////////////////////////////////////////////////////////
void CSwitchTask::SendRtmLanIsdnParamsToUtility(SLOTS_CONFIGURATION_S mfaBoardCnfArray[])
{



    CSegment *pSeg = new CSegment;
    pSeg->Put((BYTE*)mfaBoardCnfArray, MAX_NUM_OF_SLOTS*sizeof(SLOTS_CONFIGURATION_S));

    //  send to manager task
    CManagerApi api(eProcessUtility);
    api.SendMsg(pSeg, RTM_LANS_AND_ISDN_SLOT_IND);

}
