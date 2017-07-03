// McuMngrManager.cpp

#include "McuMngrManager.h"

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "McuMngrManager.h"

#include <list>
#include <fcntl.h>
#include <vector>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <math.h>

#include "licensingServer.h"
#include "TraceStream.h"

#include "ConfigManagerOpcodes.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsShelfMngr.h"
#include "OpcodesMcmsInternal.h"
#include "MplMcmsProtocol.h"
#include "Segment.h"
#include "DummyEntry.h"
#include "StatusesGeneral.h"
#include "TaskApi.h"
#include "McuMngrInternalStructs.h"
#include "SysConfigSet.h"
#include "Request.h"
#include "IpService.h"
#include "McuMngrProcess.h"
#include "FaultsContainer.h"
#include "McuState.h"
#include "SystemFunctions.h"
#include "SnmpManagerApi.h"
#include "FaultsDefines.h"
#include "Licensing.h"
#include "Versions.h"
#include "SystemTime.h"
#include "ApacheDefines.h"
#include "MplMcmsProtocolTracer.h"
#include "McuMngrStatuses.h"
#include "HlogApi.h"
#include "SysConfigKeys.h"
#include "TerminalCommand.h"
#include "IPMC.h"
#include "IPMCInterfaceApi.h"
#include "OsFileIF.h"
#include "IfConfig.h"
#include "WrappersMcuMngr.h"
#include "IpServiceValidator.h"
#include "McmsDaemonApi.h"
#include "WrappersGK.h"
#include "SetTurnSsh.h"
#include "IpmiEntityReset.h"
#include "McuMngrSNMPTask.h"
#include "SNMPUtils.h"
#include "SNMPStructs.h"
#include "OsTask.h"
#include "IncludePaths.h"
#include "AlarmStrTable.h"
#include "QA_ApiStructs.h"
#include "AuditorApi.h"
#include "SslFunc.h"
#include "FipsMode.h"
#include "McuStateGetEx.h"
#include "McmsAuthentication.h"
#include "SlotsNumberingConversionTableWrapper.h"
#include "RtmIsdnMngrInternalDefines.h"
#include "GkTaskApi.h"
#include "SipProxyTaskApi.h"
#include "McuStateGetEx.h"
#include "CsCommonStructs.h"
#include "DefinesIpServiceStrings.h"
#include "MultipleServicesFunctions.h"
#include "CsCommonStructs.h"
#include "alarmActiveTable.h"
#include "SNMPDefines.h"
#include "GetSerialNumber.h"
#include "KeyCodeSaveLoader.h"
#include "ValidateKeyCode.h"
//for gesher
#include "ConfigHelper.h"
#include "AvcSvcCapStruct.h"

#include "Opcodes802_1x.h"

#include "802_1xApiInd.h"

#include "EncodeHelper.h"
#include "SharedDefines.h"
#include "LicensingServerStructs.h"
#include <netdb.h>




//end

#define RMX_STATUSES_FILE			((std::string)(MCU_TMP_DIR+"/RmxStatusesEnglish.xml"))
#define RMX_FAULTS_AND_AA_FILE		((std::string)(MCU_TMP_DIR+"/RmxFaultsAndActiveAlarmsEnglish.xml"))
#define RESTORE_CONFIG_SUCCESS		"States/restore_config_succeeded.flg"
#define ENTER_DIAGNOSTIC_FLG		((std::string)(MCU_MCMS_DIR+"/States/enter_diagnostic.flg"))
#define ERR_NO_MANAGEMENT_IPV6		"No Management ipV6 interface from MPL was configured"
#define ERR_NO_MANAGEMENT_IPV4		"No Management ipV4 interface from MPL was configured"
#define ERR_NO_MANAGEMENT_IP		"No Management ip interface from MPL was configured"
#define MAC_ADDRESSES_CONFIG_FILE_PATH  "Cfg/MacAddressesConfig.xml"
#define MAC_ADDRESSES_CONFIG_FILE_PATH          "Cfg/MacAddressesConfig.xml"
#define MEDIA_RECORDING_FILE_PATH           ((std::string)(MCU_MCMS_DIR+"/MediaRecording"))

#define CONF_802_1x_FILES_PATH           ((std::string)(MCU_TMP_DIR+"/802_1xCtrl"))

#define CONF_802_1x_EMB_FILES_PATH           ((std::string)(MCU_TMP_DIR+"/802_1xEmb/"))

#define CONF_802_1x_MEDIA1_FILES_PATH           ((std::string)(MCU_TMP_DIR+"/802_1xEmb/media1"))
#define CONF_802_1x_MEDIA2_FILES_PATH           ((std::string)(MCU_TMP_DIR+"/802_1xEmb/media2"))
#define CONF_802_1x_MEDIA3_FILES_PATH           ((std::string)(MCU_TMP_DIR+"/802_1xEmb/media3"))
#define CONF_802_1x_MEDIA4_FILES_PATH           ((std::string)(MCU_TMP_DIR+"/802_1xEmb/media4"))

#define CONF_802_1x_SWITCH_FILES_PATH           ((std::string)(MCU_TMP_DIR+"/802_1xEmb/switch"))


//for simulation
#define SIMULATION_CONF_802_1x_EMB_FILES_PATH           ((std::string)(MCU_MCMS_DIR+"/802_1xEmb/"))

#define SIMULATION_CONF_802_1x_MEDIA1_FILES_PATH           ((std::string)(MCU_MCMS_DIR+"/802_1xEmb/media1"))
#define SIMULATION_CONF_802_1x_MEDIA2_FILES_PATH           ((std::string)(MCU_MCMS_DIR+"/802_1xEmb/media2"))
#define SIMULATION_CONF_802_1x_MEDIA3_FILES_PATH           ((std::string)(MCU_MCMS_DIR+"/802_1xEmb/media3"))
#define SIMULATION_CONF_802_1x_MEDIA4_FILES_PATH           ((std::string)(MCU_MCMS_DIR+"/802_1xEmb/media4"))

#define SIMULATION_CONF_802_1x_SWITCH_FILES_PATH           ((std::string)(MCU_MCMS_DIR+"/802_1xEmb/switch"))

// 21.06.09: was asked by Switch to increase timeout of first sync
#define NTP_FIRST_SYNC_TIMEOUT         60 * SECOND * 3
#define CONFIG_IPV6_AUTO_INTERVAL      2 * SECOND
#define CONFIG_IPV6_AUTO_TIMEOUT       60 * SECOND * 10
#define NETWORK_CONFIG_FAILURE_TIMEOUT 30 * SECOND
#define DAILY_TIMER_TIMEOUT            SECOND * 60 * 60 * 24  // 24 hours
#define DELAY_SNMP_READY_EXTRA_STARTUP  9 * SECOND
#define DELAY_STARTUP_ON_NO_SYSTEM_IP_YET  2 * SECOND


#define KILL_SSHD_CYCLIC               5555
#define UPDATE_COLLECTING_STOP         5556
#define UPDATE_COLLECTING_STOP_INTERVAL 15 * SECOND

#define INVALID_SERVER_ADDR_STR "255.255.255.255"
#define NTP_INVALID_SERVER_ADDR_STR_IPV6 "::" //this is what was agreed with OriP & AnatG as the empty IPv6 string

#define MAX_NUMBER_HD_AVC_PARTICIPANTS_SOFT_MCU_EDGE 30

extern void			McuMngrMonitorEntryPoint(void* appParam);
extern void			McuMngrSNMPTaskEntryPoint(void* appParam);
extern char*		ProcessTypeToString(eProcessType processType);
extern char*		ProductTypeToString(APIU32 productType);
extern const char*	DnsConfigurationStatusToString(eDnsConfigurationStatus configStatus);
extern char*		IpTypeToString(APIU32 ipType, bool caps = false);
extern char*		IpV6ConfigurationTypeToString(APIU32 v6Type, bool caps = false);
extern const char*	GetSystemCardsModeStr(eSystemCardsMode theMode);
extern const char*	GetSystemRamSizeStr(eSystemRamSize theSize);
extern const char* PortSpeedTypeToString(ePortSpeedType type);
extern bool			IsJitcAndNetSeparation();
extern STATUS		ConfigDnsInOS(CIPService *pService, const string theCaller);
extern std::string	GetNicName(eIpType ipType);
extern const char* ShelfMngrComponentTypeToString(APIU32 compType);
extern char* AuthorizationGroupTypeToString(int authorizationGroup);
//extern char  const * GetBoardPartNumber(char * buf, size_t len);


//added for debug
void printIpAddressDebug(DWORD ipaddress);
void PrintIPServiceInfoDebug(CIPService *ipService);
//end

static const DWORD MAX_NUM_NTP_SERVERS_FAILURE = 3;
static const WORD READY = 1;
static WORD isNotFirstValidIPCheck=0;

//this is the context of the 802.1x manager
static s802_1xManagerCtx s802_1xManagerCtxObj;


PBEGIN_MESSAGE_MAP(CMcuMngrManager)
    ONEVENT(MPLAPI_MSG,                         ANYCASE,  CMcuMngrManager::OnMplApiMsg )
    ONEVENT(TO_MCUMNGR_STATE_CHANGED,           ANYCASE,  CMcuMngrManager::OnProcessStateChanged ) // received from each process (CStateFaultList object)
    ONEVENT(CS_MCU_NUM_OF_PORTS_REQ,            ANYCASE,  CMcuMngrManager::OnCsNumOfPortsReq )
    ONEVENT(DNSAGENT_MCUMNGR_CONFIGURATION_REQ, ANYCASE,  CMcuMngrManager::OnDnsAgentConfigurationReq )
    ONEVENT(SIPPROXY_MCUMNGR_CONFIGURATION_REQ, ANYCASE,  CMcuMngrManager::OnSipProxyConfigurationReq )
    ONEVENT(INSTALLER_TO_MCUMNGR_CFS_PARAMS_REQ,ANYCASE,  CMcuMngrManager::OnInstallerCfsParamsReq )
    ONEVENT(APACHE_MODULE_AUTHENTICATION_STRUCT_AND_LICENSING_REQ, ANYCASE, CMcuMngrManager::OnApacheModuleAuthenticationStructAndMultipleServicesReq )

    ONEVENT(MPL_AUTHENTICATION_IND,             ANYCASE,  CMcuMngrManager::OnAuthenticationInd )
    ONEVENT(MPL_MNGMNT_IP_CONFIG_IND,           ANYCASE,  CMcuMngrManager::OnMngmntIpConfigInd ) // 08.05.06: MPL_MNGMNT_IP_CONFIG_IND will be treated even if received before Authentication succeeded
    ONEVENT(MPL_MNGMNT_IP_PARAMS_UPDATE_IND,    ANYCASE,  CMcuMngrManager::OnMngmntIpParamsUpdateInd )
    ONEVENT(MPL_SW_LOCATION_IND,                IDLE,     CMcuMngrManager::OnMplMsgWhileNotREADY )
    ONEVENT(MPL_SW_LOCATION_IND,                READY,    CMcuMngrManager::OnSoftwareLocationInd )
    ONEVENT(FAULT_GENERAL_IND,                  ANYCASE,  CMcuMngrManager::OnFaultGeneralInd )
    ONEVENT(RESTORE_FACTORY_DEFAULT_IND,        ANYCASE,  CMcuMngrManager::OnRestoreFactoryInd )
    ONEVENT(OUT_OF_SECURE_MODE_IND,				ANYCASE,  CMcuMngrManager::OnOutOfSecureModeInd )
    ONEVENT(ENCRYPTION_KEY_SERVER_FIPS_140_TEST_RESULT_IND,ANYCASE,CMcuMngrManager::OnEncryptionKeyServerFips140TestResultInd )
    ONEVENT(ENC_TO_MCUMNGR_IS_NTP_SYNC_REQ, ANYCASE, CMcuMngrManager::OnEncryptionKeyServerIsNtpSyncLegalReq )


    ONEVENT(MCUMNGR_CERTIFICATES_DAILY_TIMER,	ANYCASE,  CMcuMngrManager::OnTimerCertificatesDailyTimeout )

    ONEVENT(MCUMNGR_802_1x_WAIT_FOR_CARDS_TIMER,	ANYCASE,  CMcuMngrManager::OnTimerWaitForCardsStartupTimeout )
    ONEVENT(MCUMNGR_802_1x_WAIT_FOR_CTRL_ETH_SET_TIMER,	ANYCASE,  CMcuMngrManager::Handle802_1xNewConfReq )
    ONEVENT(MCUMNGR_802_1x_WAIT_FOR_MEDIA1_ETH_SET_TIMER,	ANYCASE,  CMcuMngrManager::OnTimerWaitMedia1EthSetTimeout )
    ONEVENT(MCUMNGR_802_1x_WAIT_FOR_MEDIA2_ETH_SET_TIMER,	ANYCASE,  CMcuMngrManager::OnTimerWaitMedia2EthSetTimeout )
    ONEVENT(MCUMNGR_802_1x_WAIT_FOR_MEDIA3_ETH_SET_TIMER,	ANYCASE,  CMcuMngrManager::OnTimerWaitMedia3EthSetTimeout )
    ONEVENT(MCUMNGR_802_1x_WAIT_FOR_MEDIA4_ETH_SET_TIMER,	ANYCASE,  CMcuMngrManager::OnTimerWaitMedia4EthSetTimeout )
    ONEVENT(MCUMNGR_802_1x_WAIT_FOR_SWITCH_ETH_SET_TIMER,	ANYCASE,  CMcuMngrManager::OnTimerWaitSwitchEthSetTimeout )

    ONEVENT(NTP_SERVERS_STATUS_TIMER,			ANYCASE,  CMcuMngrManager::OnTimerNtpServersStatusTimeout )
    ONEVENT(NTP_SYNC_TIMER,						ANYCASE,  CMcuMngrManager::OnTimerNtpSyncTimeout )
    ONEVENT(TEST_DHCP_IP_TIMER,            	    ANYCASE,  CMcuMngrManager::OnTestDhcpIpTimeout )
    ONEVENT(MCUMNGR_STARTUP_TIMER,              ANYCASE,  CMcuMngrManager::OnTimerStartupTimeout )
    ONEVENT(SYSTEM_STARTUP_REMAINING_TIME_TIMER,ANYCASE,  CMcuMngrManager::OnTimerSystemStartupRemainingTime )
    ONEVENT(UPDATE_SYSTEM_STARTUP_DURATION_IND, ANYCASE,  CMcuMngrManager::OnUpdateSystemStartupDurationInd)
    //ONEVENT(MCUMNGR_IPV6_AUTO_CONFIG_TIMER,		ANYCASE,  CMcuMngrManager::OnTimerIpV6AutoConfig )
    ONEVENT(MCUMNGR_NETWORK_CONFIG_FAILURE_TIMER,ANYCASE, CMcuMngrManager::OnNetworkConfigFailureTimeout )
    ONEVENT(NEW_CORE_DUMP_IND,					ANYCASE,  CMcuMngrManager::OnNewCoreDumpInd )
    ONEVENT(RSRC_SYSTEM_MEDIA_RECORDING_REQ,	ANYCASE,  CMcuMngrManager::OnMediaRecordingReq )
    ONEVENT(COLLECTOR_COLLECTING_INFO_REQ,		ANYCASE,  CMcuMngrManager::OnCollectingInfoReq )
    ONEVENT(BACKUP_INPROGRESS_REQ,		ANYCASE,  CMcuMngrManager::OnBackupInProgressReq )
    ONEVENT(RESTORE_INPROGRESS_REQ,		ANYCASE,  CMcuMngrManager::OnRestoreInProgressReq )
    ONEVENT(BACKUP_TIMEOUT,                     ANYCASE, CMcuMngrManager::OnBackupTimeout)
    ONEVENT(RESTORE_TIMEOUT,                    ANYCASE, CMcuMngrManager::OnRestoreTimeout)
    ONEVENT(BACKUPRESTORE_TO_MCUMNGR_SYS_VERSION_REQ, ANYCASE, CMcuMngrManager::OnGetMcuVersionReq)
    ONEVENT(MPL_SHELF_ENTER_DIAGNOSTICS_IND,    ANYCASE,  CMcuMngrManager::OnEnterDiagnosticsInd )
    ONEVENT(MCUMNGR_GK_MNGMNT_REQ,              ANYCASE,  CMcuMngrManager::OnGKMngmntReq)

    ONEVENT(MCCF_MNGMNT_INTERFACE_IP_REQ,       ANYCASE,  CMcuMngrManager::OnMccfMngmntReq)

//    ONEVENT(SNMP_MNGMNT_INTERFACE_IP_REQ,		ANYCASE  , CMcuMngrManager::OnSNMP_ManagmentInterfaceIpRequest )
//    ONEVENT(CQAAPI_MNGMNT_INTERFACE_IP_REQ,		ANYCASE  , CMcuMngrManager::OnQA_Api_ManagmentInterfaceIpRequest )
    ONEVENT(CERTMNGR_TO_MCUMNGR_CERTIFICATE_UPDATE_IND, ANYCASE, CMcuMngrManager::OnUpdateCertificateInd )
    ONEVENT(MCMS_SYSTEM_CARDS_MODE_IND,			ANYCASE  , CMcuMngrManager::OnSystemCardsModeInd )
    ONEVENT(SM_COMP_SLOT_ID_IND,				ANYCASE  , CMcuMngrManager::OnShmCompSlotIdInd )
    ONEVENT(RTM_LANS_AND_ISDN_SLOT_IND,			ANYCASE	 , CMcuMngrManager::OnRtmLanAndIsdnSlotInd )
    ONEVENT(RTM_LANS_IND,		   	ANYCASE	 , CMcuMngrManager::OnRtmLanInd )
    ONEVENT(CSMNGR_TO_MCUMNGR_V35GW_UPDATE_IND,	ANYCASE	 , CMcuMngrManager::OnV35GwUpdateInd )
    ONEVENT(ETHERNET_SETTINGS_CONFIG_IND,		ANYCASE  , CMcuMngrManager::OnEthSettingConfigInd )
    ONEVENT(CARDS_SLOTS_NUMBERING_CONVERSION_TABLE_IND,ANYCASE,CMcuMngrManager::OnSlotsNumberingConversionTableInd)
    ONEVENT(RESOURCE_SYSTEM_RAM_SIZE_REQ,				ANYCASE,	CMcuMngrManager::OnResourceSystemRamSizeReq )
    ONEVENT(RESOURCE_SYSTEM_CPU_PROFILE_REQ,			ANYCASE,	CMcuMngrManager::OnResourceSystemCPUProfileReq )
    ONEVENT(CONFPARTY_SYSTEM_RAM_SIZE_REQ,				ANYCASE,	CMcuMngrManager::OnConfPartySystemRamSizeReq )
    ONEVENT(INSTALLER_KEYCODE_UPDATE_IND,				ANYCASE,	CMcuMngrManager::OnInstallerKeycodeUpdateInd )
    ONEVENT(MCMS_RESET,					   				ANYCASE, 	CMcuMngrManager::OnReset)	// for Call Generator
    ONEVENT(FAILOVER_MCUMNGR_CONFIG_REQ,				ANYCASE,	CMcuMngrManager::OnFailoverConfigReq)
    ONEVENT(FAILOVER_MCUMNGR_UPDATE_MNGMNT_IND,			ANYCASE,	CMcuMngrManager::OnFailoverUpdateMngmntInd)
    ONEVENT(FAILOVER_MCUMNGR_UPDATE_PAIR_IP_IND,		ANYCASE,	CMcuMngrManager::OnFailoverUpdatePairIPInd)
    ONEVENT(FAILOVER_MCUMNGR_EVENT_TRIGGER_IND,			ANYCASE,	CMcuMngrManager::OnFailoverEventTriggerInd)
    ONEVENT(MCUMNGR_RESET_TIMER,						ANYCASE, 	CMcuMngrManager::OnResetRequest)
    ONEVENT(CS_MCUMNGR_IP_SERVICE_PARAM_IND, 			ANYCASE, 	CMcuMngrManager::OnCsIpServiceParamsInd)
    ONEVENT(CS_MCUMNGR_DELETE_IP_SERVICE_IND, 			ANYCASE, 	CMcuMngrManager::OnCsDeleteIpeServiceParamsInd)
	ONEVENT(FAILOVER_SLAVE_BECOME_MASTER, 				ANYCASE,	CMcuMngrManager::OnFailoverSlaveBecomeMasterInd)
    ONEVENT(KILL_SSHD_CYCLIC,				 			ANYCASE, 	CMcuMngrManager::OnKillSshd)
    ONEVENT(GMT_OFFSET_SET_TIMER,						ANYCASE,  	CMcuMngrManager::SendGmtOffset )
    ONEVENT(RSRCMNGR_UPDATE_HD_TRUE_IN_NON_1500Q,         ANYCASE,    CMcuMngrManager::EnableHDLicenseInNon1500Q )
    ONEVENT(RSRCMNGR_UPDATE_MCUMNGR_ON_1500Q,           ANYCASE,    CMcuMngrManager::UpdateHDLicensePortsIn1500Q )
    ONEVENT(UPDATE_COLLECTING_STOP,				 		ANYCASE, 	CMcuMngrManager::OnUpdateCollectingStop)
    ONEVENT(SYSTEM_MONITORING_MCUMNGR_CPU_INFO,				 		ANYCASE, 	CMcuMngrManager::OnUpdateCpuInfo)
    ONEVENT(LOGGER_TO_MCUMNGR_CS_LOG_STATE,     ANYCASE, CMcuMngrManager::OnLoggerMngrCsLogsConfigInd)
    ONEVENT(MCUMNGR_NOTIFMNGR_MCU_STATE_REQ, 			ANYCASE, 	CMcuMngrManager::OnNotifMngrMcuStateReq)
    ONEVENT(MCUMNGR_NOTIFMNGR_AA_LIST_REQ, 				ANYCASE, 	CMcuMngrManager::OnNotifMngrAAListReq)
	ONEVENT(MCUMNGR_PRECEDENCE_SETTINGS_REQ,			ANYCASE, 	CMcuMngrManager::OnRequestPrecedenceSettings)
    ONEVENT(SNMP_CONFIG_TO_OTHER_PROGRESS, ANYCASE, CMcuMngrManager::OnSNMPConfigInd)
    //ONEVENT(MCUMNGR_GENERATE_CONF_FILE_802_1X     ,ANYCASE , CMcuMngrManager::ConfigGenerateConfFlie802_1x )
    ONEVENT(MCUMNGR_Op802_1x_NEW_KILL_WPA_REQ    ,ANYCASE , CMcuMngrManager::P802_1xKillAllWpaProcs )
    ONEVENT(OpInt802_1x_CONNECTION_STATUS_CHANGE_EVENT    ,ANYCASE , CMcuMngrManager::Handle802_1xConnectionStatusChangeEvent )
    ONEVENT(SECURITY_PKI_CFG_REQ, ANYCASE, CMcuMngrManager::SendSecurityPKIToDependedProcesses)
    ONEVENT(Op802_1x_NEW_CONFIG_IND , ANYCASE,CMcuMngrManager::Handle802_1xNewConfInd )
    ONEVENT(MCUMNGR_SNMP_READY_TIMER,	ANYCASE,  CMcuMngrManager::OnTimerSnmpReady )
    ONEVENT(FAILED_REMOVED_AA_TIMER,	ANYCASE,  CMcuMngrManager::OnTimerFailedRemoveAA )
    ONEVENT(LICENSE_SERVER_PARAMS_REQ,	ANYCASE,  CMcuMngrManager::OnLicenseServerParamsReq )
    ONEVENT(LICENSE_SERVER_FIRST_UPDATE_PARAMS_REQ,	ANYCASE, CMcuMngrManager::OnLicenseServerFirstUpdateParamsReq )
    ONEVENT(LICENSE_SERVER_UPDATE_PARAMS_REQ,	ANYCASE, CMcuMngrManager::OnLicenseServerUpdateParamsReq )
    ONEVENT(INSTALLER_AUTHENTICATION_STRUCT_REQ,	ANYCASE, CMcuMngrManager::OnInstallerAuthenticationStructReq )
  ONEVENT(LICENSE_SERVER_CONNECTION_STATUS_IND,	ANYCASE, CMcuMngrManager::OnLicenseServerConnectionStatusInd )
    ONEVENT(LICENSE_SERVER_CONNECTION_TIME_IND,	ANYCASE, CMcuMngrManager::OnLicenseServerTimeInd)
	
	
PEND_MESSAGE_MAP(CMcuMngrManager,CManagerTask);

BEGIN_TERMINAL_COMMANDS(CMcuMngrManager)
    ONCOMMAND("ver",			CMcuMngrManager::HandleTerminalMcuVer,			"ver - display system's versions' numbers")
    ONCOMMAND("mcu_state",		CMcuMngrManager::HandleTerminalMcuState,		"state - get the state of the process and state of the system")
    ONCOMMAND("all_aa",			CMcuMngrManager::HandleTerminalAllAA,			"all_aa - display all active alarms")
    ONCOMMAND("generate_k",		CMcuMngrManager::HandleTerminalGenerate_X_Kc,	"generate_x - generates X_Keycode")
    ONCOMMAND("generate_u",		CMcuMngrManager::HandleTerminalGenerate_U_Kc,	"generate_u - generates U_Keycode")
    ONCOMMAND("licensing_info",	CMcuMngrManager::HandleTerminalLicensingInfo,	"licensing_info - display licensing information")
    ONCOMMAND("start_dhcp",		CMcuMngrManager::HandleTerminalStartDHCP,	    "start dhcp client for mng interface")
    ONCOMMAND("stop_dhcp",		CMcuMngrManager::HandleTerminalStopDHCP,	    "stop dhcp client for mng interface")
    ONCOMMAND("get_dhcp",		CMcuMngrManager::HandleTerminalGetDHCP,	        "get_dhcp KEY - get returned dhcp value for key")
    ONCOMMAND("update_dns", 	CMcuMngrManager::HandleTerminalUpdateDns,       "update_dns DNS HOST ZONE IP")
  	ONCOMMAND("set_restore",	CMcuMngrManager::HandleTerminalSetRestore,		"set_restore - Removes all config, output in a next startup")
  	ONCOMMAND("reset_mngmnt",	CMcuMngrManager::HandleTerminalResetMngmntParams,"reset_mngmnt - Sends req to Switch to reset Management_Service parameters")
  	ONCOMMAND("reset_usr_list",	CMcuMngrManager::HandleTerminalResetUsrListParams,"reset_usr_list - Sends req to Switch to reset Users_List parameters")
  	ONCOMMAND("reset_all_params",CMcuMngrManager::HandleTerminalResetAllParams,	"reset_all_params - Sends req to Switch to reset all parameters")
    ONCOMMAND("turn_ssh_on",    CMcuMngrManager::HandleTerminalTurnSshOn,	"Turns on the ssh on MNGMNT IP")
	ONCOMMAND("create_status_file", CMcuMngrManager::HandleTerminalCreateStatusFile, "Create Xml status file")
//    ONCOMMAND("send_ip_ind_snmp", CMcuMngrManager::HandleTerminalSendIpIndSnmp, "Send IP indication to Snmp")
    ONCOMMAND("create_fault_and_aa_file", CMcuMngrManager::HandleTerminalCreateFaultAndAAFile, "Create Xml Faults and Active alarm file")
    ONCOMMAND("start_reset_timer", CMcuMngrManager::HandleTerminalStartResetTimer, "Starts reset timer")
    ONCOMMAND("mac_address_config", CMcuMngrManager::HandleTerminalMacAddressConfig, "Get MAC addresses from configuration file")
	// for Call Generator - net_config.sh
    ONCOMMAND("update_network_configuration", CMcuMngrManager::HandleTerminalUpdateNetworkConfiguration, "Update the network configuration - IP, MASK and Default GW")

	ONCOMMAND("config_apache",		CMcuMngrManager::HandleTerminalConfigApache,		"Configs Apache")
	ONCOMMAND("restart_apache", CMcuMngrManager::HandleTerminalRestartApache, "Restart Apache")
	ONCOMMAND("change_led",		CMcuMngrManager::HandleTerminalChangeLedState,		"Change LED state, usage: change_led [red|green|amber] [on|off|flickering]")
    ONCOMMAND("disable_whitelist",	CMcuMngrManager::HandleTerminalDisableWhiteList,		"Send configurator command to disable White list")

    ONCOMMAND("set_eth",	CMcuMngrManager::HandleTerminalSetEthSetting,		"Set ethernet setting parameters")

    ONCOMMAND("get_activealarms",	CMcuMngrManager::HandleTerminalGetActiveAlarms,		"Get System Active Alerts")

    ONCOMMAND("set_mcutype",	CMcuMngrManager::HandleTerminalSetMcuType,		"Set System McuType")

END_TERMINAL_COMMANDS

BEGIN_SET_TRANSACTION_FACTORY(CMcuMngrManager)
	ON_TRANS("TRANS_CFG",		"SET_CFG",					CSysConfigSet,	CMcuMngrManager::HandleSetCfg)
  ON_TRANS("TRANS_CFG", "SET_CFG_PARAM", CSysConfigSet, CMcuMngrManager::HandleSetCfgParam)
	ON_TRANS("TRANS_IP_SERVICE",		"UPDATE_MANAGEMENT_NETWORK",CIPService,				CMcuMngrManager::HandleSetMngmntNetwork)
	ON_TRANS("TRANS_ETHERNET_SETTINGS",	"UPDATE_ETHERNET_SETTINGS",	CEthernetSettingsConfig,CMcuMngrManager::HandleSetEthernetSettings)
    ON_TRANS("TRANS_MCU",		"SET_TIME",					CSystemTime,	CMcuMngrManager::HandleSetTime)
    ON_TRANS("TRANS_MCU",		"SET_PRECEDENCE_SETTINGS",	CPrecedenceSettings,	CMcuMngrManager::HandleSetPrecedenceSettings)
    ON_TRANS("TRANS_MCU",		"SET_RESTORE_TYPE",			CSetMcuRestore,	CMcuMngrManager::HandleSetMcuRestore)
    ON_TRANS("TRANS_MCU",		"TURN_SSH",			        CSetTurnSsh,    CMcuMngrManager::HandleTurnSsh)
    ON_TRANS("TRANS_MCU",		"RMX_GET_STATE_EX",		    CMcuStateGetEx, CMcuMngrManager::HandleGetMcuStateEx)
    ON_TRANS("TRANS_MCU",		"RESET",					CDummyEntry,	CMcuMngrManager::HandleReset)	// for Call Generator
    ON_TRANS("TRANS_IPMI_ENTITY", "RESET",					CIpmiEntityReset, CMcuMngrManager::HandleIpmiEntityReset)
    ON_TRANS("TRANS_MCU",		 "UPDATE_LICENSING_SERVER",  CLicensingServer,CMcuMngrManager::HandleUpdateLicensingServer)
END_TRANSACTION_FACTORY

void McuMngrManagerEntryPoint(void* appParam)
{
	CMcuMngrManager * pMcuMngrManager = new CMcuMngrManager;
	pMcuMngrManager->Create(*(CSegment*)appParam);
}

TaskEntryPoint CMcuMngrManager::GetMonitorEntryPoint()
{
	return McuMngrMonitorEntryPoint;
}

CMcuMngrManager::CMcuMngrManager()
{
	m_pProcess                                   = (CMcuMngrProcess*)CMcuMngrProcess::GetProcess();
	m_pMngmntIpParams_asIpService_fromProcess    = m_pProcess->GetMngmntIpParams_asIpService();
	m_pActiveAlarmList_fromProcess               = m_pProcess->GetActiveAlarmsList();
	m_pDefaultMngmntIpService					 = new CIPService;

	m_switchBoardId    = 0;
	m_switchSubBoardId = 0;
	m_cpuBoardId		= 0;
	m_cpuSubBoardId		= 0;

	m_isLegalU_keycode				= NO;
	m_isAuthenticationIndReceived	= NO;
	m_isIpConfigInOsSucceeded		= eIpTypeConfigSuccess_None;
	m_isDnsConfigInOsSucceeded		= NO; // only for a specific checking (at the end of OnMngmntIpConfigInd())
	m_IpV6AutoConfigState			= eIPv6AutoConfig_NotStarted;
	m_IpV6AutoConfigAccumulatedTime	= 0;
	m_isMngmntIpConfigIndReceived	= NO;
	m_isStartupCondition_Dns_added	= NO;
	m_isStartupTimoutReached		= NO;
	m_isUserChangedManagementIp		= NO;
	m_isRmxTimeChangedByUser		= NO;
	m_dnsConfigurationStatus					= eDnsNotConfigured;
	m_isDnsApplicabilityAlreadySentToSipProxy	= NO;

	m_worstProcessSate     = eProcessNormal;
	m_pProcessesStatesMap  = new CProcessStateFaultListMap;

	m_pSystemVersions = new CVersions;
	std::string versionFilePath = VERSIONS_FILE_PATH;
	m_pSystemVersions->ReadXmlFile(versionFilePath.c_str());

	m_pMacAddressConfig = new CMacAddressConfig;
	m_pLicensing = m_pProcess->GetLicensing();

	m_X_KeyCode			= "";
	m_isKeyCodeReset	= false;

	m_isNtpSyncLegal			= NO;
	m_NtpServersStatusPeriod	= 0;
	m_NtpSyncPeriod				= 0;
	m_numOfDnsEnable			= 0;

	for (int i=0; i< NTP_MAX_NUM_OF_SERVERS; i++)
	{
		m_isActiveAlarmNtpPeerAlreadyProduced[i] = NO;
	}

    m_NtpFailureCounter = 0;
    m_SnmpTaskId = 0;

    for(int i = 0; i < MNGMNT_NETWORK_NUM_OF_IPV6_ADDRESSES ; ++i)
    {
    	memset(m_cntrlIPv6AddressInAutoMode[i].address, '\0', IPV6_ADDRESS_LEN);
    	m_cntrlIPv6AddressInAutoMode[i].mask = DEFAULT_IPV6_SUBNET_MASK;
    }
    memset(m_defGwInAutoMode, '\0', IPV6_ADDRESS_LEN);
    m_defGwMaskInAutoMode = DEFAULT_IPV6_SUBNET_MASK;

    m_mcuMngrProductType = m_pProcess->GetProductType();
    m_mcuMngrRmxSystemCardsMode = m_pProcess->GetRmxSystemCardsModeDefault();

    m_isConfBlock = FALSE;
    m_cert_start_date = NULL;
    m_cert_expiration_date = NULL;
    m_isSNMPup = FALSE;
    m_isSNMPReady = FALSE;
    m_snmpExtraTimeForStartupLaunch = 0;
    m_minimumDelaySnmpTimer = 0;
    m_pSlotsNumConversionTable = new CSlotsNumberingConversionTableWrapper;
    m_pServiceForAccessOnNetConfigFailure = NULL;

    m_bRestoreConfigSuccess = FALSE;

    m_bRestoreNoDnsConfiguration_AA = TRUE;		//AA for AA_NO_DNS_CONFIGURATION after restore
    m_bRestoreDnsRegistrationFailed_AA = TRUE;	//AA for AA_DNS_REGISTRAION_FAILED after restore

    m_bIsV35GwEnabledInService	= FALSE;
	m_sInternalGwAddress			= "";

	for (int i=0; i<MAX_NUMBER_OF_SERVICES_IN_RMX_4000; i++)
	{
		m_StaticIpServiceList[i].ServId = 0;
		memset(m_StaticIpServiceList[i].DefaultGatewayIPv6, '\0',IPV6_ADDRESS_LEN);
		m_StaticIpServiceList[i].DefaultGatewayMaskIPv6 = 0;

		m_StaticIpServiceList[i].numPQSactual = 0;
		memset(m_StaticIpServiceList[i].IPServUDPperPQList, '\0',sizeof(IP_SERVICE_UDP_MCUMNGR_PER_PQ_S)*MAX_NUM_PQS);
	}

	m_firstTimeReqForOutOfSecured = FALSE;

	m_bCSLogStarted = FALSE;
	m_bTriggerFailover = FALSE;
	m_bNotificationMngrReady = FALSE;

	memset(m_mediaCards,0,4*sizeof(s802_1x_NEW_CONFIG_REQ));
	memset(&m_switchCard,0,sizeof(s802_1x_NEW_CONFIG_REQ));
	memset(&m_cntl,0,sizeof(s802_1x_NEW_CONFIG_REQ));


	m_isLicenseEnable = FALSE;
	m_isLicenseExpired = FALSE;
	m_isRestoreIndBlock = FALSE;

}

CMcuMngrManager::~CMcuMngrManager()
{
	POBJDELETE(m_pProcessesStatesMap);
	POBJDELETE(m_pSystemVersions);
	POBJDELETE(m_pMacAddressConfig);
	POBJDELETE(m_pDefaultMngmntIpService);
	POBJDELETE(m_pSlotsNumConversionTable);
	POBJDELETE(m_pServiceForAccessOnNetConfigFailure);
}

void CMcuMngrManager::ManagerInitActionsPoint()
{
	m_state = IDLE;
}

void CMcuMngrManager::SelfKill()
{
    if (m_mcuMngrProductType == eProductTypeGesher || m_mcuMngrProductType == eProductTypeNinja)
    {
        std::string cmd;
        std::string answer;

        cmd = "sudo /usr/bin/killall NTP_Bypass_SoftMCU_Server";

        TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::SelfKill: cmd " << cmd;

        STATUS stat = SystemPipedCommand(cmd.c_str(),answer);
        TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
            "CMcuMngrManager::SelfKill: " << cmd << std::endl << answer;
    }
    StopSoftNtpStatus();

     DWORD snmpTaskId = CProcessBase::GetProcess()->GetSNMPTaskId();
     COsTask::SendSignal(snmpTaskId, SIGALRM);

     CManagerTask::SelfKill();

}




void   CMcuMngrManager::InitNetworkInterfaceHelperFailRead(STATUS retStatus)
{
	if ( (STATUS_FILE_NOT_EXIST == retStatus) || (STATUS_OPEN_FILE_FAILED == retStatus) ) // no file exists (yet) - create a default, hard-coded MngmntNetworkInterface
			{
				// 30/01/06: it was decided that if file not exist, no hardCoded configuration will be done.
				//           When MPL will send MPL_MNGMNT_IP_CONFIG_IND the ip will be configured.

				TRACESTR(eLevelInfoNormal)
					<< "\n" << __FUNCTION__ << " - failed opening MngmntIpConfig file: " << MANAGEMENT_NETWORK_CONFIG_PATH;
			}
			else // retStatus is probably 'STATUS_PARSING_XML_FILE_FAILED'
			{
				CMedString errStr = "failed parsing MngmntIpConfig file: ";
				errStr << MANAGEMENT_NETWORK_CONFIG_PATH;

				TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::InitNetworkInterface - " << errStr.GetString();
				CHlogApi::XmlParseError( errStr.GetString() ); // produce Fault (only)
			}

			ConfigApache( eIpType_IpV4, "0.0.0.0", "");
}

STATUS CMcuMngrManager::InitNetworkInterfaceWithMcmsNetwork(CIPService& tmpService,STATUS readFileStatus)
{
	STATUS retStatus =readFileStatus;
	if (STATUS_OK != retStatus)
	{
			InitNetworkInterfaceHelperFailRead(retStatus);

	} // end (open_file != ok)
	else
	{
		//need to add case of failure ?
		// flow check configuration

		tmpService.SetNetMask(m_pProcess->m_NetSettings.m_ipv4_Mask);
		tmpService.SetDefaultGatewayIPv4(m_pProcess->m_NetSettings.m_ipv4_DefGw);
		CIPSpan* pCntrlSpan = tmpService.GetSpanByIdx(0);
		if(pCntrlSpan)
		{
			pCntrlSpan->SetInterface(m_pProcess->m_NetSettings.m_interface);
			pCntrlSpan->SetIPv4Address(m_pProcess->m_NetSettings.m_ipv4);
			if(eV6Configuration_Auto == m_pProcess->m_NetSettings.m_ipv6ConfigType)
			{

				   tmpService.SetDefaultGatewayBytesIpv6(m_pProcess->m_NetSettings.m_ipv6_DefGw.addr,m_pProcess->m_NetSettings.m_ipv6_DefGw.mask);
				   ipv6AddressWithMaskStruct ipv6Struct;
						for(int i = 0 ; i < MNGMNT_NETWORK_NUM_OF_IPV6_ADDRESSES ; ++i)
						{
							if(m_pProcess->m_NetSettings.GetIpv6Mngnt(i,ipv6Struct))
							{
								pCntrlSpan->m_IPv6AaddressArray[i] = ipv6Struct.addr;
								pCntrlSpan->SetIPv6SubnetMask(i,ipv6Struct.mask ); // set the mask
							}
						}
			}
			eIpType curIpType = m_pProcess->m_NetSettings.m_iptype;


			if (m_mcuMngrProductType==eProductTypeSoftMCU || m_mcuMngrProductType==eProductTypeSoftMCUMfw || m_mcuMngrProductType==eProductTypeEdgeAxis)
			{
				//cases where ipv6 configuration is manual change it to autoconfig
				tmpService.SetIpV6ConfigurationType( m_pProcess->m_NetSettings.m_ipv6ConfigType);


			}
			if(eNetConfigurationFailureActionChangeMngntIp4Type == m_pProcess->m_NetSettings.m_netConfigStatus)
			{ //error occur on configuring ipv6 set only ipv4
				tmpService.SetIpType(eIpType_IpV4);
			}

		}

		UpdateDnsForService(tmpService);
		UpdateNetworkInterface("caller InitNetworkInterfaceWithMcmsNetwork",tmpService,eMngmntIpInit);

		ConfigGeneral();
		CMcmsDaemonApi api;
		api.SendConfigApacheInd();
		if ( YES != IsTarget() && (eProductTypeGesher != m_mcuMngrProductType)
					&& (eProductTypeNinja != m_mcuMngrProductType))
		{
				m_isDnsConfigInOsSucceeded = YES;
				TreatDnsConfigurationNewStatus(eDnsNotConfigured, YES);
				TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__
				                       << " - System is not Target; a 'not configured' answer is sent to DnsAgent";
		}
		//config Quality of Service in firewall
		ConfigQosManagementNetwork();

		CManagerApi csmngr_api(eProcessCSMngr);
		csmngr_api.SendOpcodeMsg(MCUMNGR_TO_CSMNGR_END_IP_CONFIG_IND);
	}
	return retStatus;
}
//s.tanny this function should be removed not called any more
/*void CMcuMngrManager::SetIPv4SpanParams(CIPService& tmpService, IP_ADDR_S& ipAddrS)
{
    TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__ ;

    //Configure the management service to the first available IP address/Interface
    string ip_address="";
    SystemPipedCommand("echo -n `/sbin/ifconfig | grep -A1 'eth[0-9]' | grep 'inet addr' | cut -d':' -f2 | cut -d' ' -f1 | awk 'NR==1'`", ip_address);
//    if(ip_address == "")
//    	return;
    bool isIpValid = false;
    if (strstr(ip_address.c_str(),INVALID_SERVER_ADDR_STR) || ip_address == "")
	{
    	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::SetIPv4SpanParams -- VM system IP is not valid yet - need to wait for valid IP";
    	StartTimer(MCUMNGR_VM_IP_READY_TIMER, DELAY_STARTUP_ON_NO_SYSTEM_IP_YET);
    	isNotFirstValidIPCheck=0;
    	return;
	}
	else
	{
		TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::SetIPv4SpanParams --VM system IP is valid";
		DeleteTimer(MCUMNGR_VM_IP_READY_TIMER);

		isIpValid=true;
	}
    //get the interface according to the first ip address that was found
    std::string nic_name="";
	std::string cmd = "echo -n `/sbin/ifconfig | grep "+ip_address+" -B1 | grep 'eth[0-9]' | cut -d' ' -f1`";
	SystemPipedCommand(cmd.c_str(), nic_name);
    if(nic_name == "")
    	return;

    DWORD dword_ipAddress = SystemIpStringToDWORD(ip_address.c_str());

    //Read default gw
    string ipv4_defGw;
    SystemPipedCommand("echo -n `/sbin/ip route show | grep default | cut -d' ' -f3`", ipv4_defGw);
    DWORD dword_defaultGateway = SystemIpStringToDWORD(ipv4_defGw.c_str());
    tmpService.SetDefaultGatewayIPv4(dword_defaultGateway);

    //Read net mask from first interface
    string net_mask;
    cmd = "echo -n `/sbin/ifconfig | grep -A1 " + nic_name + " | grep Mask | cut -d':' -f4`";
    SystemPipedCommand(cmd.c_str(), net_mask);
    DWORD dword_netMask = SystemIpStringToDWORD(net_mask.c_str());
    tmpService.SetNetMask(dword_netMask);

    CIPSpan * pSpan = tmpService.GetFirstSpan();
    if(pSpan)
    {
    	pSpan->SetIPv4Address(dword_ipAddress);
    	pSpan->SetInterface(nic_name);
    }

    RetrieveIpAddresses(&tmpService, ipAddrS);

	if (isNotFirstValidIPCheck && isIpValid)
	{
		TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::SetIPv4SpanParams: Handle Recovery IP";
		ConfigGeneral(&tmpService);
		std::string cmd;
		cmd=(std::string)"sudo /bin/env "+"MCU_HOME_DIR="+((std::string)GET_MCU_HOME_DIR)+" LD_LIBRARY_PATH="+MCU_APACHE_DIR+"/lib\\:"+MCU_MCMS_DIR+"/Bin "+MCU_MCMS_DIR+"/Bin/httpd -f "+MCU_MCMS_DIR+"/StaticCfg/httpd.conf -k restart";
		std::string ans;
		STATUS stat = SystemPipedCommand(cmd.c_str(), ans);
		PASSERTSTREAM_AND_RETURN(stat != STATUS_OK,
		"SystemPipedCommand: " << cmd << ": " << ans);
		CManagerApi mngrApi(eProcessCSMngr);
		mngrApi.SendOpcodeMsg(MCUMNGR_TO_CSMNGR_IP_BECAME_VALID_IND);

	}

    TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::InitNetworkInterface ip address: SetIPv4SpanParams " << ipAddrS.iPv4_str
                                   << " default gw: " << ipAddrS.iPv4_defGW
                                   << " net mask: " << ipAddrS.iPv4_netMask
                                   << " interface: " << nic_name;

}*/
//s.tanny this function should be removed not called any more
/*void CMcuMngrManager::SetIPv6SpanParamsAuto(CIPService& tmpService, IP_ADDR_S& ipAddrS)
{
    TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__ ;

	eIpType curIpType	= tmpService.GetIpType();
	eV6ConfigurationType curIpV6configType	= tmpService.GetIpV6ConfigurationType();

    //Configure the management service to the first available IPv6 address/Interface
    string iPv6_str="";
    SystemPipedCommand("echo -n `/sbin/ifconfig | grep Scope:Global | grep 'inet6 addr:' | cut -d':' -f2- | cut -d' ' -f2 | awk 'NR==1'`", iPv6_str);
    if(iPv6_str == "")
    	return;

    if (eIpType_IpV6 == curIpType)
    {
		//get the interface according to the first ip address that was found
		std::string nic_name="";
		std::string cmd = "echo -n `/sbin/ifconfig | grep "+iPv6_str+" -B6 | grep 'eth[0-9]' | cut -d' ' -f1`";
		SystemPipedCommand(cmd.c_str(), nic_name);
		if(nic_name == "")
			return;

	    CIPSpan * pSpan = tmpService.GetFirstSpan();
	    if(pSpan)
	    	pSpan->SetInterface(nic_name);
    }

	bool success = RetrieveIpV6AutoAndConfig(tmpService);
	if(!success)
	{
		//m_IpV6AutoConfigState = eIPv6AutoConfig_InProgress;
		StartTimer(MCUMNGR_IPV6_AUTO_CONFIG_TIMER, CONFIG_IPV6_AUTO_INTERVAL);
		TRACEINTO << "\nCMcuMngrManager::OnTimerIpV6AutoConfig - currently failed to retrieve auto ipV6; reattempting...";
	}

	else // retrieving addresses succeeded
	{
		TRACEINTO << "\nCMcuMngrManager::OnTimerIpV6AutoConfig - auto ipV6 is retrieved successfully";

		m_IpV6AutoConfigState = eIPv6AutoConfig_Success;
		m_IpV6AutoConfigAccumulatedTime = 0; // for debugging (to see if it starts again)

		if (eIpTypeConfigSuccess_None == m_isIpConfigInOsSucceeded)
		{
			m_isIpConfigInOsSucceeded = eIpTypeConfigSuccess_IPv6;
		}
		else if(eIpTypeConfigSuccess_IPv4 == m_isIpConfigInOsSucceeded)
		{
			m_isIpConfigInOsSucceeded = eIpTypeConfigSuccess_Both;
		}

		ConfigGeneral();

		CMcmsDaemonApi daemonApi;
		daemonApi.SendConfigApacheInd();

    	CManagerApi mngrApi(eProcessCSMngr);
    	mngrApi.SendOpcodeMsg(MCUMNGR_TO_CSMNGR_END_IP_CONFIG_IND);

        //Retrieving IPV6 address succeeded, Remove ActiveAlarm

        TRACESTR(eLevelInfoNormal) << __FUNCTION__ << " IPV6 address success, Remove ActiveAlarm IPV6 AA_NO_SYSTEM_IP_YET";
	}

    RetrieveIpAddresses(&tmpService, ipAddrS);

    TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::InitNetworkInterface ip address: SetIPv6SpanParams " << ipAddrS.iPv6_str << " iPv6_NoBrackets: " << ipAddrS.iPv6_NoBrackets
                                   << " default gw: " << ipAddrS.iPv6_defGW
                                   << " net mask: " << ipAddrS.iPv6_netMask;

}*/

/*void CMcuMngrManager::SetIPv6SpanParamsManual(CIPService& tmpService, IP_ADDR_S& ipAddrS)
{
    TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__ ;

    SetIPv6SpanParamsAuto(tmpService, ipAddrS);

#if 0

    string iPv6_str; //eV6Configuration_Auto
    string iPv6_netMask; //eV6Configuration_Manual
    string iPv6_NoBrackets; //eV6Configuration_Manual
    string iPv6_defGW;

    SystemPipedCommand("echo -n `/sbin/ifconfig | grep Scope:Global | grep 'inet6 addr:' | cut -d':' -f2- | cut -d' ' -f2 | awk 'NR==1' | cut -d '/' -f1`", iPv6_NoBrackets);
    SystemPipedCommand("echo -n `/sbin/ifconfig | grep Scope:Global | grep 'inet6 addr:' | cut -d':' -f2- | cut -d' ' -f2 | awk 'NR==1' | cut -d '/' -f2`", iPv6_netMask);
    SystemPipedCommand("echo -n `/sbin/ifconfig | grep Scope:Global | grep 'inet6 addr:' | cut -d':' -f2- | cut -d' ' -f2 | awk 'NR==1'`", iPv6_str);
    SystemPipedCommand("echo -n `/sbin/route -A inet6 | grep G | grep U | awk '{ print $2 }' | awk 'NR==1'`", iPv6_defGW);

    tmpService.SetDefaultGatewayIPv6(iPv6_defGW.c_str());
    tmpService.SetDefaultGatewayMaskIPv6(iPv6_netMask.c_str());

    //tmpService.SetIpV6Params(eIpType_Both, eV6Configuration_Auto, iPv6_str, "", "", iPv6_defGW, 64, 0, 0, 64, false);

    CIPSpan *pSpan0 = tmpService.GetSpanByIdx(0);
    if(pSpan0)
    {
		pSpan0->SetIPv6Address(0,iPv6_str.c_str());
		pSpan0->SetIPv6SubnetMask(0,iPv6_netMask.c_str());
        string interface;
		SystemPipedCommand("echo -n `/sbin/ifconfig | grep -A1 'eth[0-9]' | grep 'Link encap' | cut -d' ' -f1 | awk 'NR==1'`", interface);
		pSpan0->SetInterface(interface);
    }
#endif
}*/

/*void CMcuMngrManager::SetIPv6SpanParams(CIPService& tmpService, IP_ADDR_S& ipAddrS)
{
    TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__ ;

	eV6ConfigurationType curCfgType	= tmpService.GetIpV6ConfigurationType();
	if(eV6Configuration_Manual == curCfgType)
	       SetIPv6SpanParamsManual(tmpService,ipAddrS);
	else
	       SetIPv6SpanParamsAuto(tmpService,ipAddrS);
}*/

/*void CMcuMngrManager::SetIPSpanParams(CIPService& tmpService, IP_ADDR_S& ipAddrS)
{
    TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__ ;

	eIpType curIpType	= tmpService.GetIpType();
	eV6ConfigurationType curIpV6configType	= tmpService.GetIpV6ConfigurationType();

	if (eIpType_IpV4 == curIpType || eIpType_Both == curIpType)
        SetIPv4SpanParams(tmpService,ipAddrS);

	if (eIpType_IpV6 == curIpType || eIpType_Both == curIpType)
        SetIPv6SpanParams(tmpService,ipAddrS);
}*/


/////////////////////////////////////////////////////////////////////////////


// Get router list from service
void CMcuMngrManager::RetrieveRouterList(CIPService *pService, std::list<CRouter> & routerList)
{
	const CH323Router *currentRouter = pService->GetFirstRouter();
	while(NULL != currentRouter)
	{
		if(false == currentRouter->IsDefault())
		{
			char targetIpBuffer		[128];
			char subnetMaskIPv4Buffer	[128];
			char gatewayBuffer		[128];

			SystemDWORDToIpString(currentRouter->GetRemoteIP()	, targetIpBuffer);
			SystemDWORDToIpString(currentRouter->GetSubnetMask(), subnetMaskIPv4Buffer);
			SystemDWORDToIpString(currentRouter->GetRouterIP()	, gatewayBuffer);

			CRouter param;
			param.m_type 		= (currentRouter->GetRemoteFlag() == H323_REMOTE_NETWORK ? router_net : router_host);
			param.m_targetIP 	= targetIpBuffer;
			param.m_subNetmask 	= subnetMaskIPv4Buffer;
			param.m_gateway 	= gatewayBuffer;

			routerList.push_back(param);
		}
		currentRouter = pService->GetNextRouter();
	}
}

////////////////////////////////////////////////////////////////////////////
eConfigInterfaceType CMcuMngrManager::GetMngrIfType()
{
	eConfigInterfaceType ifType = eManagmentNetwork;

	//for gesher/ninja lanredundancy
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	if (eProductTypeGesher == curProductType || eProductTypeNinja == curProductType)
	{
		//BOOL isLanRedundancy = GetSystemCfgFlagInt<BOOL>(CFG_KEY_LAN_REDUNDANCY);
		eIpType ipType = m_pMngmntIpParams_asIpService_fromProcess->GetIpType();
		BOOL isLanRedundancy = CProcessBase::GetProcess()->GetLanRedundancy(ipType);
		if (isLanRedundancy)
		{
			//for gesher/ninja lanredundancy
			ifType = eLanRedundancyManagementNetwork;
		}
	}

	if(IsJitcAndNetSeparation())
	{
		ifType = eSeparatedManagmentNetwork;
	}
	return ifType;
}





////////////////////////////////////////////////////////////////////////////

void CMcuMngrManager::ConfigBondingInterface(BOOL bLAN_REDUNDANCY)
{
	CConfigManagerApi api;
    std::string bondingMode;
	DWORD linkmonitoringfrequency;
    if (eProductTypeRMX1500 == m_pProcess->GetProductType() || eProductTypeRMX4000 == m_pProcess->GetProductType())
    {
		m_pProcess->GetSysConfig()->GetDataByKey("CPU_BONDING_MODE", bondingMode);
		m_pProcess->GetSysConfig()->GetDWORDDataByKey("CPU_BONDING_LINK_MONITORING_FREQUENCY",linkmonitoringfrequency);
		if (bLAN_REDUNDANCY==YES && (IsFederalOn() == FALSE)) //in Lan redundancy not multiple services o federal, turn on bond 0
		{
			api.ConfigBonding(bondingMode,linkmonitoringfrequency,TRUE);
		}
		else  //turn off bonding interface
		{
			api.ConfigBonding(bondingMode,linkmonitoringfrequency,FALSE);
		}
    }
}


STATUS CMcuMngrManager::ConfigEthernetInOS(CIPService *pService)
{
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::ConfigEthernetInOS";

	STATUS retStatus = STATUS_OK;

	// ===== 2. get data
	ePortSpeedType mngmntPortSpeed = ePortSpeed_Auto;
	//=================================================
	//=================================================
	// mngmntPortSpeed = theRealPortSpeed...  ---> to be implemented correctly on Amos (RMX4000)
	//=================================================
	//=================================================


	// ===== 3. config
    if ( YES == IsTarget() ) // otherwise no configuration should be done
    {
    	retStatus = ConfigureEthernetSettingsCpu(eManagmentNetwork, mngmntPortSpeed);
	}

	// ===== 4. print to log
	CLargeString retStr = "\nCMcuMngrManager::ConfigEthernetInOS\n";
	retStr << "Management port spped: "  << ::PortSpeedTypeToString(mngmntPortSpeed)
	       << "\nConfiguration status: " << m_pProcess->GetStatusAsString(retStatus).c_str();


	if ( NO == IsTarget() )
	{
		retStr << "\nNote: System is not a Target; no actual configuration was done!";
	}

    TRACESTR(eLevelInfoNormal) << retStr.GetString();


    if (STATUS_OK != retStatus)
    	retStatus = STATUS_FAIL_TO_CONFIG_MNGMNT_ETHERNET;

	return retStatus;
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendDnsConfigStatusToDnsAgent()
{
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendDnsConfigStatusToDnsAgent";

	// ===== 1. print data
	PrintDnsConfigurationStatusToTrace();

	// ===== 2. insert the appropriate value to a segment
	CSegment*  pRetParam = new CSegment();
	*pRetParam << (WORD)m_dnsConfigurationStatus;


	// ===== 3. send to DnsAgent
	CManagerApi api(eProcessDNSAgent);
	api.SendMsg(pRetParam, DNSAGENT_MCUMNGR_CONFIGURATION_IND);
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendDnsConfigStatusToSipProxy()
{
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendDnsConfigStatusToSipProxy";

	m_isDnsApplicabilityAlreadySentToSipProxy = YES;

	// ===== 1. print data
	PrintDnsConfigurationStatusToTrace();

	// ===== 2. insert the appropriate value to a segment
        for( int i = 1; i <= MAX_SERVICES_NUM; i++ )
        {
	    CSegment*  pRetParam = new CSegment();
	    *pRetParam << (WORD)m_dnsConfigurationStatus;

	  // ===== 3. send to DnsAgent
	  //CManagerApi api(eProcessSipProxy);
	    CSipProxyTaskApi api(i);
	    api.SendMsg(pRetParam, SIPPROXY_MCUMNGR_CONFIGURATION_IND);
        }
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::PrintDnsConfigurationStatusToTrace()
{
	CLargeString retStr = "\nCMcuMngrManager::PrintDnsConfigurationStatusToTrace";

	retStr << "\nDNS configuration ";
	if ( eDnsConfigurationSuccess == m_dnsConfigurationStatus )
	{
		retStr << "succeeded";
	}
	else if ( eDnsConfigurationFailure == m_dnsConfigurationStatus )
	{
		retStr << "failed";
	}
	else if ( eDnsNotConfigured == m_dnsConfigurationStatus )
	{
		retStr << "not performed";
	}
	else
	{
		retStr << "- illegal state: " << m_dnsConfigurationStatus;
	}

	TRACESTR(eLevelInfoNormal) << retStr.GetString();
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnEnterDiagnosticsInd(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::OnEnterDiagnosticsInd";
	std::string fname = MCU_MCMS_DIR+"/Cfg/diagnostics";
    int f = open(fname.c_str(), O_WRONLY|O_NONBLOCK|O_CREAT|O_NOCTTY|O_LARGEFILE, 0666);
    close(f);

}

void CMcuMngrManager::OnGKMngmntReq(CSegment* pSeg)
{
	SendMNGMNTServiceToGK();
}

void CMcuMngrManager::OnMccfMngmntReq(CSegment* pSeg)
{
	SendMNGMNTInfoToProcess(eProcessMCCFMngr, MCCF_MNGMNT_INTERFACE_IP_IND);
}

void CMcuMngrManager::OnMplApiMsg(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::OnMplApiMsg";

	CMplMcmsProtocol* pMplMcmsProtocol = new CMplMcmsProtocol;
	pMplMcmsProtocol->DeSerialize(*pSeg);
	CMplMcmsProtocolTracer(*pMplMcmsProtocol).TraceMplMcmsProtocol("MCUMNGR_RECEIVED_FROM_MPL");

	OPCODE opcd = pMplMcmsProtocol->getOpcode(); // extract the internal opcode...

	pSeg->ResetRead();
	DispatchEvent(opcd, pSeg);                     //  ... and send it to the stateMachine
	PushMessageToQueue(opcd, pSeg->GetLen(), eProcessMplApi);

	POBJDELETE(pMplMcmsProtocol);
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnMplMsgWhileNotREADY(CSegment* pSeg)
// 'READY' state means that Authentication indication was received by McuMngrManager.
//   In case that another Mpl message is received before an Authentication msg is,
//   it means that Mcms has fell and restarted --- and now needs an Authentication again!
//   Therefore the Mpl should be reset (in order to resend the Authentication msg).
{
	CMplMcmsProtocol* pMplMcmsProtocol = new CMplMcmsProtocol;
	pMplMcmsProtocol->DeSerialize(*pSeg);

	// print to trace
    TRACESTR(eLevelInfoNormal)
    	<< "CMcuMngrManager::OnMplMsgWhileNotREADY. Opcode: " << pMplMcmsProtocol->getOpcode();

	POBJDELETE(pMplMcmsProtocol);

	// ask for resetting all MFA boards
	SendResetAllMfaBoardsReqToMplApi();
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnAuthenticationInd(CSegment* pSeg)
{
	/************************************************************************************************/
	/* 27.6.10 VNGR-15657 changed by Rachel Cohen                                                   */
	/* when the first AuthenticationAck msg that we send to switch get lost,it cause switch to keep */
	/* sending AuthenticationInd FOREVER .                                                          */
	/* I removed the sending msg before the "if ((YES == m_isAuthenticationIndReceived)..."         */
	/* cause we return from that "if" and do not send the AuthenticationAck again                   */
	/************************************************************************************************/

	// ===== 1. set attributes with data from segment received
	CMplMcmsProtocol* pMplMcmsProtocol = new CMplMcmsProtocol;
	pMplMcmsProtocol->DeSerialize(*pSeg);

	// validate data size
	STATUS sizeStat = pMplMcmsProtocol->ValidateDataSize( sizeof(MPL_AUTHENTICATION_S) );
	if (STATUS_OK != sizeStat)
	  {
	    	POBJDELETE(pMplMcmsProtocol);
		return;
	  }
	m_authentication.SetData(pMplMcmsProtocol->GetData());

	m_switchBoardId		= pMplMcmsProtocol->getPhysicalInfoHeaderBoard_id();
	m_switchSubBoardId	= pMplMcmsProtocol->getPhysicalInfoHeaderSub_board_id();


	/****************************************************************************/
	/* 10.6.10 VNGR-15365 changed by Rachel Cohen                               */
	/* SendAuthenticationAckToMplApi use m_switchBoardId and m_switchSubBoardId */
	/* so it need to be after we initialize those parameters                    */
	/****************************************************************************/
	//let the switch know that we got this message
	SendAuthenticationAckToMplApi();

    //stop the configurator from checking eth0 link status
	if (YES == IsTarget())
	    StopCheckEth0Link();

	if ( (YES == m_isAuthenticationIndReceived) &&
	     (eLicensingValidationSucceeded == m_pProcess->GetMcuStateObject()->GetValidationState()) )
	{
		POBJDELETE(pMplMcmsProtocol);
		/*to fix bug BRIDGE-2259: replace the assert with normal trace */
		/*It will be the normal path since McuMngrManager will take long time to config the system, */
		/*it will receive multiple MPL_AUTHENTICATION_IND messages */
        TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnAuthenticationInd - MPL_AUTHENTICATION_IND already received (and authentication succeeded)" ;
		SendInfoToSwitchAfterSwitchReset();
		return;
	}

  HandleFirstNtpSync();

	STATUS retStatus = STATUS_OK;
	m_isAuthenticationIndReceived = YES;

	RemoveActiveAlarmByErrorCode(AA_MPL_STARTUP_FAILURE_AUTHENTICATION_NOT_RECEIVED);

	// ===== 2. update ShelfMngr with debug_mode flag (from sysCfg)
	SendDebugModeToShelfMngr(); //TODO For Federal -remove
	POBJDELETE(pMplMcmsProtocol);

	ValidateProductType();
	ePlatformType platformTypeFromSwitch = (ePlatformType)(m_authentication.GetPlatformType());

  // Tweaks authentication object in case of SoftMCU family.
  if (eProductTypeSoftMCU    == m_mcuMngrProductType ||
      eProductTypeSoftMCUMfw == m_mcuMngrProductType ||
      eProductTypeEdgeAxis   == m_mcuMngrProductType ||
      eProductTypeCallGeneratorSoftMCU   == m_mcuMngrProductType ||
      IsPlatformOfNewArch())
  {
    char xcode[128]  = {};
    char serial[128] = {};
    if (eProductTypeSoftMCU == m_mcuMngrProductType)
    {
      strcpy_safe(serial, "9251aBc471");
      strcpy_safe(xcode, "X2EB-E531-27B0-00FF-0024");
    }
    else if (eProductTypeCallGeneratorSoftMCU == m_mcuMngrProductType)
    {
        strcpy_safe(serial, "9251aBc471");
        strcpy_safe(xcode, "XC93-FC7E-EB70-03FF-0018");
    }
    else if (eProductTypeSoftMCUMfw == m_mcuMngrProductType)
    {
      strcpy_safe(serial, "9251aBc471");
      strcpy_safe(xcode, "X896-44F3-1680-0381-0043");  // D0.003810043
    }
    else if (eProductTypeEdgeAxis == m_mcuMngrProductType || IsPlatformOfNewArch())
    {
      if (!IsEdgeAxisMcuInSimulation())
      {
      	char err_code[128] = "";
        int res = GetSerialNumber(serial, ARRAYSIZE(serial),err_code,ARRAYSIZE(err_code));
        if (0 >= res)
        {
          TRACEWARN << "Unable to get serial number, defaut is used";
          strcpy_safe(serial, "9251aBc471");
        }

		if (eProductTypeNinja == m_mcuMngrProductType)
		{
			if (strlen(err_code))
			{
				AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT,
                                AA_CARDS_CONFIG_EVENT,
                                MAJOR_ERROR_LEVEL,
                                err_code,
                                true,
                                true);
			}
		}
		
        if (eProductTypeEdgeAxis == m_mcuMngrProductType)
		      std::transform(serial, serial+strlen(serial), serial, ::toupper);

        std::string key;
        KeyCodeSaveLoader().LoadKeyCode(key, m_pSystemVersions->GetMcuVersion());
        strcpy_safe(xcode, key.c_str());
      }
      else
      {
        strcpy_safe(serial, "925-SOFT-MCU-SIM-1471");
        strcpy_safe(xcode, "XDE9-29E4-D170-0381-0004"); // D0.003810004
      }
    }
    else
    {
      PASSERTSTREAM(true,
        "Unhandled platform type " << ProductTypeToString(m_mcuMngrProductType));
    }

    if (eProductTypeSoftMCU == m_mcuMngrProductType && !ValidRpmDate())
    {
      m_isConfBlock=TRUE;
      SendConfBlockToConfParty();
    }
    else
    {
      RemoveActiveAlarmByErrorCode(AA_LICENSE_EXPIRED);
      m_authentication.SetSerialNumber(serial);
      m_authentication.Set_X_KeyCode(xcode);
      m_authentication.Set_U_KeyCode("U6EE-6521-DB40-0000-2710");
    }
  }

	// ===== 3. print the data to trace (using Dump function)
  TRACEINTO << "Authentication for "
            << ProductTypeToString(m_mcuMngrProductType) << ":\n"
            << m_authentication;

	// ===== 4. set basic params from authentication struct
	m_pProcess->GetMcuStateObject()->SetMcuStateProductType(m_mcuMngrProductType);
	m_pProcess->GetMcuStateObject()->SetMplSerialNumber( m_authentication.GetSerialNumber() );
	m_pLicensing->SetMplSerialNum( m_authentication.GetSerialNumber() );

	SendAuthenticationStructToProcesses(); // for platform_type

//	SendParamsToCdrManager();
	// ===== 5. validate keycodes
	STATUS statusK = STATUS_OK;

	CSmallString serialNumStr;
	serialNumStr << m_authentication.GetSerialNumber();

	 TRACEINTO << "IsFlexeraLicenseInSysFlag " << m_pProcess->IsFlexeraLicenseInSysFlag() ;

	if (m_mcuMngrProductType != eProductTypeEdgeAxis || m_pProcess->IsFlexeraLicenseInSysFlag() == false)
	{

		statusK = ValidateKeycode_FromMpl(serialNumStr,m_authentication.GetkeyCodeVersionFromMpl());

		// failed to validate keycode(s)
		if (STATUS_OK != statusK)
		{
			SetMcuStateValidationFailure(eBothKeycodesValidationFailure);
			SendInitKeycodeFailureIndToInstaller();

			if (false == m_isKeyCodeReset)
			{
				SendAuthenticationFailureToMplApi();
			}
		}
		//	if ( (STATUS_OK != statusK) || (STATUS_OK != statusU) )
		//	{
		//		if ( (STATUS_OK != statusK) && (STATUS_OK != statusU) )
		//		{	// failed to validate both keycodes
		//			SetMcuStateValidationFailure(eBothKeycodesValidationFailure);
		//			SendAuthenticationFailureToMplApi();
		//		}
		//		else if (STATUS_OK != statusK)
		//		{	// failed to validate X_keycode
		//			SetMcuStateValidationFailure(eX_KeycodeValidationFailure);
		//			SendAuthenticationFailureToMplApi();
		//
		//			// U_Keycode can still be relevant (for versions upgrades etc.)
		//			if ( (NO == IsKeycodeExistsInMpl('U')) || (YES == IsKeycodeReset('U')) )
		//			{
		//				m_isLegalU_keycode = YES;
		//				UpdateLicensingWithParamsFrom_U_KeyCode();
		//			}
		//		}
		//		else if (STATUS_OK != statusU)
		//		{	// failed to validate U_keycode
		//			SetMcuStateValidationFailure(eU_KeycodeValidationFailure);
		//
		//			if (STATUS_KEYCODE_INVALID == statusU)
		//				SendAuthenticationFailureToMplApi();
		//		}
		//	} // end if failed to validate keycode(s)


		else // validate keycodes == OK
		{

			ContinueStartup_KeycodeValidated();
		}
	}

	// ===== 6. Now McuMngrManager is ready to receive other Mpl messages
	//          (If an Mpl message is received before MPL_AUTHENTICATION_IND is received,
	//          then MPL should be reset - func OnMplMsgWhileNotREADY)
	m_state = READY;

	// ===== 7. update other entities
	SendCfsParamsToInstaller();
	SendParamsToCdrManager();

	if (m_mcuMngrProductType == eProductTypeEdgeAxis && m_pProcess->IsFlexeraLicenseInSysFlag() == true)
		   SendLicensingServerParamsInd();
}



void CMcuMngrManager::SendParamsToCdrManager()
{
	CSegment* pSeg = new CSegment();
	MCMS_INFO_S mcmsInfoStruct;
	IP_ADDR_S ipAddrS;
	memset(&mcmsInfoStruct, 0, sizeof(MCMS_INFO_S));
	memset(&ipAddrS, 0, sizeof(IP_ADDR_S));

	// retrieve ip addresses for IpV6, IpV4
	RetrieveIpAddresses(m_pMngmntIpParams_asIpService_fromProcess, ipAddrS);
	strcpy(mcmsInfoStruct.ipv4, ipAddrS.iPv4_str);
	strcpy(mcmsInfoStruct.ipv6, ipAddrS.iPv6_str);
//	const char* hostName = GetHostNameFromService(m_pMngmntIpParams_asIpService_fromProcess).c_str();
	const char* serialNumber = m_authentication.GetSerialNumber();
	strcpy_safe_helper(mcmsInfoStruct.serialNumber,sizeof(mcmsInfoStruct.serialNumber),serialNumber);
	mcmsInfoStruct.chassisVersion = m_authentication.GetMcuChassisVersionFromMpl();

	pSeg->Put( (BYTE*)(&mcmsInfoStruct), sizeof(MCMS_INFO_S) );

	CManagerApi api(eProcessCDR);
	api.SendMsg(pSeg, MCUMNGR_TO_CDR_PARAMS_IND);

	std::ostringstream stChassis;
	stChassis << mcmsInfoStruct.chassisVersion.ver_major << "." << mcmsInfoStruct.chassisVersion.ver_minor << "." << mcmsInfoStruct.chassisVersion.ver_release << "." << mcmsInfoStruct.chassisVersion.ver_internal;
	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::SendParamsToCdrManager. serialNum=" << mcmsInfoStruct.serialNumber << " chassis=" << stChassis.str();

}
/////////////////////////////////////////////////////////////////////////////
BOOL CMcuMngrManager::ValidRpmDate()
{
	std::ostringstream cmd, cmd2, cmd3;

	if (m_mcuMngrProductType==eProductTypeSoftMCU || m_mcuMngrProductType==eProductTypeCallGeneratorSoftMCU)
		cmd << "rpm -q --queryformat '%{BUILDTIME}' `rpm -qa | grep Plcm-SoftMcuMain`";
	else if (m_mcuMngrProductType==eProductTypeSoftMCUMfw)
		cmd << "rpm -q --queryformat '%{BUILDTIME}' `rpm -qa | grep Plcm-SoftMcuMainMFW`";
    std::string ans;
    STATUS stat = SystemPipedCommand(cmd.str().c_str(), ans);
    TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::ValidRpmDate - command: " << cmd.str() << ":"<< ans;

    if (ans.length()==0)
    {
    	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::ValidRpmDate - no RPM installed - probably working in simulation. return TRUE";
  		return TRUE;
    }

    DWORD dword_ans = atoi(ans.c_str());	//getting the rpm build date in Epoch

	CStructTm now;
	stat = SystemGetTime(now);

	DWORD diff = ((DWORD)now > dword_ans ? now - dword_ans : 0);

	int days_diff = ((diff/60)/60)/24;	// calculate the seconds in days

	if (days_diff > 80)
	{
		TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::ValidRpmDate - more than 80 days!!!! : "<<days_diff;
		return FALSE;
	}
	else
		TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::ValidRpmDate - less than 80 days!!!! : "<<days_diff;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
/*void CMcuMngrManager::GetDateFromString(std::string date, CStructTm& rpm_date)
{
	int day, month, year;
	int string_end_pos = 0;
	day = get_int(date, string_end_pos);

	string_end_pos++;

	size_t pos;
	std::string  subString = date.substr(string_end_pos);

	//find the month
	pos = subString.find(" ");
	std::string str_month = subString.substr(0, pos);
	month=getMonth(str_month);

	year = get_int(subString, string_end_pos);

	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::GetDateFromString final -"<<day<<" "<<month<<" "<<year;

	tm tmpTime;
	tmpTime.tm_sec 	= 0;
	tmpTime.tm_min 	= 0;
	tmpTime.tm_hour = 0;
	tmpTime.tm_mday = day;
	tmpTime.tm_mon 	= month-1; //0..11 month
	tmpTime.tm_year = year - 1900;	//year start in 1900.
	tmpTime.tm_wday  = 0;
	tmpTime.tm_yday  = 0;
	tmpTime.tm_isdst = 0;

	time_t time_t_obj = mktime(&tmpTime);

	rpm_date.SetAbsTime(time_t_obj);

}*/

/////////////////////////////////////////////////////////////////////////////
/*int CMcuMngrManager::get_int( const string& s , int& string_end_pos)
{
	string result;
	string::size_type front = s.find_first_of( "0123456789" );
	if ( front != string::npos )
	{
		string::size_type back = s.find_first_not_of( "0123456789", front );
		result.assign( s, front, back - front );
		string_end_pos = int(back);
	}
	return atoi(result.c_str());
}

/////////////////////////////////////////////////////////////////////////////
int CMcuMngrManager::getMonth(const std::string& month)
{
	// loop through the array, and find the month that matches abbrev
	string months [] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
	for (int i=0; i<12; i++)
	{
		if (month == months[i])
			return i+1;
	}
	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::getMonth: failed to find the month "<<month;
	return 0;
}*/

/////////////////////////////////////////////////////////////////////////////

void CMcuMngrManager::SendInfoToSwitchAfterSwitchReset()
{
	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::SendInfoToSwitchAfterSwitchReset";

	//update ShelfMngr with debug_mode flag (from sysCfg)
	SendDebugModeToShelfMngr();

	// send isNtp to MplApi
	if ( false == IsActiveAlarmExistByErrorCode(AA_NO_TIME_CONFIGURATION) )
	{
		CSystemTime sysTime;
		m_pProcess->GetMcuTime(sysTime);

		if (NO == m_isRmxTimeChangedByUser)
			SendTimeConfigReqToMplApi(&sysTime, YES);
		else
			SendTimeConfigReqToMplApi(&sysTime, NO);
	}

	//send user list to MplApi
	SendAuthenticationSuccessToAuthenticationProcess();

}

/////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::ValidateKeycode_FromMpl(CSmallString serialNumStr, VERSION_S  keyCodeVersion)
{
	CMedString retStr = "\nCMcuMngrManager::ValidateKeycode_FromMpl -";
	TRACESTR(eLevelInfoNormal) << retStr.GetString();

	STATUS retStatus = STATUS_OK;

	if ( YES == IsKeycodeExistsInMpl() )
	{
		if ( YES == IsKeycodeReset() )  // keycode is reset ("X00000...") - an unacceptable situation
		{
			m_isKeyCodeReset = true;
			retStatus = STATUS_FAIL;

			TRACESTR(eLevelInfoNormal) << retStr.GetString() << "\nKeycode is reset";
		}

		else  // keycode exists, and not reset
		{
			TRACESTR(eLevelInfoNormal) << retStr.GetString() << "- start validation";

			CKeyCode keyCodeFromMpl( m_authentication.Get_X_KeyCode() );
			retStatus = keyCodeFromMpl.Validate(m_mcuMngrProductType, serialNumStr, -1, -1, &keyCodeVersion);
			if (STATUS_OK==retStatus)
			{
				int failReason = 0;
				retStatus = ValidateKeyCode(m_mcuMngrProductType, m_authentication.Get_X_KeyCode(), failReason, false, false);
				if (STATUS_OK != retStatus)
					TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::ValidateKeycode_FromMpl - invalid activation key, reason=[" << failReason << "]";
			}
		}
	}

	else  // keycode doesn't exist (an illegal situation)
	{
		retStr << "\nKeycode does not exist";
		retStatus = STATUS_FAIL;
	}

	retStr << "---- status: " << retStatus;
	TRACESTR(eLevelInfoNormal) << retStr.GetString();

	return retStatus;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CMcuMngrManager::IsKeycodeExistsInMpl()
{
	BOOL ret = YES;
	CMedString retStr = "CMcuMngrManager::IsKeycodeExistsInMpl -";

	// ===== 1. take keycode from structure received
	ALLOCBUFFER(pKeycode, KEYCODE_LENGTH);
	memset(pKeycode , 0, KEYCODE_LENGTH);

	memcpy(pKeycode, m_authentication.Get_X_KeyCode(), KEYCODE_LENGTH-1);


	// ===== 2. check if it begins with the expected character
	if ( 'X' != pKeycode[0] ) // keycode in structure (from MPL) doesn't begin with the expected character
	{
		retStr << " mismatch!\n---- expected keycode type: X,"
		       << " actual first char in keycode received: " << pKeycode[0];

		ret = NO;
	}
	else // ok
	{
		retStr << "---- success! keycode type: X";
	}


	TRACESTR(eLevelInfoNormal) << retStr.GetString();

    DEALLOCBUFFER(pKeycode);

	return ret;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CMcuMngrManager::IsKeycodeReset()
{
	BOOL ret = YES;
	CMedString retStr = "CMcuMngrManager::IsKeycodeReset -";

	// ===== 1. take keycode from structure received
	ALLOCBUFFER(pKeycode, KEYCODE_LENGTH);
	memset(pKeycode , 0, KEYCODE_LENGTH);

	memcpy(pKeycode, m_authentication.Get_X_KeyCode(), KEYCODE_LENGTH-1);


	// ===== 2. check if it is reset ("X000000.....")
	for (int i=1; i<KEYCODE_LENGTH; i++)
	{
		if ( 0 != pKeycode[i] )
		{
			ret = NO;
			break;
		}
	}

	if (YES == ret)
	{
		retStr << " keycode is reset. keycode: " << pKeycode;
	}
	else
	{
		retStr << " keycode is not reset. keycode: " << pKeycode;
	}

	TRACESTR(eLevelInfoNormal) << retStr.GetString();

    DEALLOCBUFFER(pKeycode);

	return ret;
}


/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::ContinueStartup_KeycodeValidated()
{
	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::ContinueStartup_KeycodeValidated";

	// ===== 1. update system's state
	RemoveActiveAlarmByErrorCode(AA_PRODUCT_ACTIVATION_FAILURE);
	m_pProcess->GetMcuStateObject()->SetValidationState(eLicensingValidationSucceeded);

	if (eProductTypeEdgeAxis != m_mcuMngrProductType || m_pProcess->IsFlexeraLicenseInSysFlag() == false)
	{
		RemoveActiveAlarmByErrorCode(AA_PRODUCT_ACTIVATION_FAILURE);

		// ===== 2. update attributes
		m_isLegalU_keycode = YES;
		m_X_KeyCode = m_authentication.Get_X_KeyCode();
	}

	// ===== 3. send isNtp to MplApi
	if ( false == IsActiveAlarmExistByErrorCode(AA_NO_TIME_CONFIGURATION) )
	{
		CSystemTime sysTime;
		m_pProcess->GetMcuTime(sysTime);

		if (eProductTypeGesher == m_mcuMngrProductType || eProductTypeNinja == m_mcuMngrProductType || eProductTypeEdgeAxis == m_mcuMngrProductType)
		{
			if (sysTime.GetIsNTP())
			{
				NtpSetServer(&sysTime);
			}
		}
		else
		{
			if (NO == m_isRmxTimeChangedByUser)
			{
				SendTimeConfigReqToMplApi(&sysTime, YES);
			}
			else
			{
				SendTimeConfigReqToMplApi(&sysTime, NO);
			}
		}
	}

        if (eProductTypeEdgeAxis != m_mcuMngrProductType || m_pProcess->IsFlexeraLicenseInSysFlag() == false)
	        UpdateLicensingWithParamsFromKeyCodes();
        else
        	UpdateLicensingWithParamsFromFile();

//	from V4.6
//	if (m_pLicensing->GetTotalNumOfCopParties()==0)
//		AddActiveAlarmSingleton( FAULT_GENERAL_SUBJECT,
//								 AA_EVENT_MODE_RESOURCE_DEFICIENCY_DUE_TO_WRONG_LICENSE,
//	                             MAJOR_ERROR_LEVEL,
//	                             "Event Mode Conferencing resources deficiency due to inappropriate license",
//	                             true,
//	                             true
//	                           );

	// ===== 4. some processes are dependant on Authentication success to complete their missions
	SendAuthenticationSuccessToProcesses();

	// ===== 5. spread params to processes
	SendLicensingParamsToProcesses();

	// ===== 6. Send Conference block to ConfParty
	if (m_isConfBlock == TRUE)
		SendConfBlockToConfParty();

	// ===== 7. update other entities
	SendCfsParamsToInstaller();

	// ===== 8. send multiple services to processes
	SendMultipleServicesToProcesses();
}

void CMcuMngrManager::SetMcuStateValidationFailure(eValidationFailureType failureType)
{
	// ===== 1. set statuses
	m_pProcess->GetMcuStateObject()->SetValidationState(eLicensingValidationFailed);

	// ===== 2. produce Fault:
	// a. prepare error description
	string errStr;

	if (eX_KeycodeValidationFailure == failureType )
		errStr = "Failed to validate Activation key";

	else if (eU_KeycodeValidationFailure == failureType )
		errStr = "Failed to validate Upgrading key";

	else if (eBothKeycodesValidationFailure == failureType )
		errStr = "Failed to validate Activation key";

	else if (eLicensingFileError == failureType )
		errStr = "Failed to read Licensing file";

	else if (eGeneratedKeyCodeMismatch == failureType )
		errStr = "Mismatch between original keyCode and generated keyCode";

	else if (eMplAuthenticationFailure == failureType )
		errStr = "Failed to authenticate MPL";

	else if (eVersionValidationFailure == failureType )
		errStr = "Keycodes versions mismatch";

	if (m_mcuMngrProductType != eProductTypeEdgeAxis)
	{
		// b. produce the Fault and Logger
		UpdateActiveAlarmDescriptionByErrorCode(AA_PRODUCT_ACTIVATION_FAILURE, errStr);
		UpdateStartupConditionByErrorCode(AA_PRODUCT_ACTIVATION_FAILURE, eStartupConditionFail);
	}
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SetMcuStateValidationFailure: " << errStr;
}

void CMcuMngrManager::SendAuthenticationAckToMplApi()
{
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendAuthenticationAckToMplApi";

	CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol();

	mplPrtcl->AddCommonHeader(MPL_AUTHENTICATION_REQ);
	mplPrtcl->AddMessageDescriptionHeader();
	mplPrtcl->AddPhysicalHeader(1, m_switchBoardId, m_switchSubBoardId);

	CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("MCUMNGR_SENDS_TO_MPL_API");
	mplPrtcl->SendMsgToMplApiCommandDispatcher();

	POBJDELETE(mplPrtcl);
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendAuthenticationFailureToMplApi()
{
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendAuthenticationFailureToMplApi";

	CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol();

	mplPrtcl->AddCommonHeader(MCUMNGR_AUTHENTICATION_FAILURE_REQ);
	mplPrtcl->AddMessageDescriptionHeader();
	mplPrtcl->AddPhysicalHeader(1, m_switchBoardId, m_switchSubBoardId);

	CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("MCUMNGR_SENDS_TO_MPL_API");
	mplPrtcl->SendMsgToMplApiCommandDispatcher();

	POBJDELETE(mplPrtcl);
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendInitKeycodeFailureIndToInstaller()
{
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendInitKeycodeFailureIndToInstaller";

	STATUS res = STATUS_OK;

    CSegment*  pRetParam = new CSegment;
	const COsQueue* pInstallerMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessInstaller, eManager);

	res = pInstallerMbx->Send(pRetParam,MCUMNGR_INIT_KEYCODE_FAILURE_IND);
}


/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendAuthenticationStructToProcesses()
{
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendAuthenticationStructToProcesses";

	SendAuthenticationStructToSpecificProcess(eProcessAuthentication);
	SendAuthenticationStructToSpecificProcess(eProcessApacheModule);
	SendAuthenticationStructToSpecificProcess(eProcessSystemMonitoring);
	SendAuthenticationStructToSpecificProcess(eProcessInstaller);
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendAuthenticationStructToSpecificProcess(eProcessType processType)
{
	STATUS res = STATUS_OK;

    CSegment*  pRetParam = new CSegment;

	// ===== 1. fill the segment with AuthenticationSuccess parameters
    MCMS_AUTHENTICATION_S *pAuth = new MCMS_AUTHENTICATION_S;
	memset(pAuth, 0, sizeof(MCMS_AUTHENTICATION_S));
    pAuth->productType			= (DWORD)m_mcuMngrProductType;
    pAuth->switchBoardId    	= m_switchBoardId;
    pAuth->switchSubBoardId 	= m_switchSubBoardId;
    pAuth->rmxSystemCardsMode   = (DWORD)m_mcuMngrRmxSystemCardsMode;
    pAuth->chassisVersion		= m_authentication.GetMcuChassisVersionFromMpl();
    pAuth->isCtrlNewGeneration  = m_authentication.GetIsNewCtrlGeneration();
	pRetParam->Put( (BYTE*)pAuth, sizeof(MCMS_AUTHENTICATION_S) );


	const char* productTypeStr = ::ProductTypeToString( (eProductType)(pAuth->productType) );

	CLargeString traceStr = "\nCMcuMngrManager::SendAuthenticationStructToSpecificProcess - ";
	traceStr << ProcessTypeToString(processType)
			 << "\nProduct Type:   " << productTypeStr
			 << "\nSwitch boardId: " << m_switchBoardId << ", switch subBoardId: " << m_switchSubBoardId;

	TRACESTR(eLevelInfoNormal) << traceStr.GetString();

	delete pAuth;

	// ===== 3. send
	CManagerApi api(processType);
	api.SendMsg(pRetParam,MCUMNGR_AUTHENTICATION_STRUCT_REQ);
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendAuthenticationSuccessToProcesses()
{
	// ----- 29.11.07: united with SendLicensingParamsToCards
	//inform success to Cards  - let it start treating CM_LOADED indications
	//SendAuthenticationSuccessToCardsProcess();

	// inform success to Authentication - let it send UsersList to ShelfMngr
	SendAuthenticationSuccessToAuthenticationProcess();
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendAuthenticationSuccessToAuthenticationProcess()
{
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendAuthenticationSuccessToAuthenticationProcess";

	STATUS res = STATUS_OK;

    CSegment*  pRetParam = new CSegment;
	const COsQueue* pAuthenticationMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessAuthentication, eManager);

	res = pAuthenticationMbx->Send(pRetParam,MCUMNGR_AUTHENTICATION_SUCCESS_REQ);
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendRMXTimeSetToAuthenticationProcess()
{
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendRMXTimeSetToAuthenticationProcess";

	STATUS res = STATUS_OK;

    CSegment*  pRetParam = new CSegment;
	const COsQueue* pAuthenticationMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessAuthentication, eManager);

	res = pAuthenticationMbx->Send(pRetParam,MCUMNGR_TIME_SET);
}
/////////////////////////////////////////////////////////////////////////////
BOOL CMcuMngrManager::OnMplIpParametersToService(CSegment* pSeg,CIPService& mplService)
{
	// ===== 2. get the data into IP_PARAMS_S object
	CMplMcmsProtocol mplMcmsProtocol;
	mplMcmsProtocol.DeSerialize(*pSeg);

	// ---- 3 validate data size
	STATUS sizeStat = mplMcmsProtocol.ValidateDataSize( sizeof(IP_PARAMS_S) );
	if (STATUS_OK != sizeStat)
	{
		PASSERTSTREAM(TRUE, "Invalid Size of struct IP_PARAMS_S from Mpl");
		return FALSE;
	 }
	CIpParameters ipParamsFromMpl;
	ipParamsFromMpl.SetData(mplMcmsProtocol.GetData());
	 // ---- 4 validate and replace invalid strings
	 ipParamsFromMpl.ValidateStrings();
	  // ===== 5  print the data to trace (using Dump function)
	 TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnMplIpParametersToService \n" << ipParamsFromMpl;
	 // ===== 5. convert to CIPService (for writing to file etc.)...

	 ipParamsFromMpl.ConvertToIpService(mplService);
	 mplService.SetName(MANAGEMENT_NETWORK_NAME);
	 mplService.SetIpServiceType(eIpServiceType_Management);
	 return TRUE;
}
void CMcuMngrManager::OnMplSecurityNotEqual(BOOL securityMode)
{
	// ===== Get 'old' ip service
	CIPService oldIpService = *m_pMngmntIpParams_asIpService_fromProcess;
	oldIpService.SetIsSecured(securityMode);
	if(TRUE == oldIpService.GetIsSecured())
		UpdateSecurityDependencies(oldIpService);
	else
		ConfigApache(oldIpService);

	 // Updates in memory
	 m_pProcess->SetMngmntIpParams_asIpService(oldIpService);
	 m_pMngmntIpParams_asIpService_fromProcess = m_pProcess->GetMngmntIpParams_asIpService();

	 m_pProcess->UpdateIpInterfaceInList(oldIpService);
	 // ===== 7. update the file
	 m_pMngmntIpParams_asIpService_fromProcess->WriteXmlFile(MANAGEMENT_NETWORK_CONFIG_PATH, "MngmntNetwork");
	 TRACESTR(eLevelInfoNormal) << "OnMplConfigIndChangeMcmsNetwork  update security  mode of managment and update file ";
	// update Authentication  processes with new security mode
	SendSecurityMode(eProcessAuthentication);
}
void CMcuMngrManager::OnMplConfigIndChangeMcmsNetwork(CSegment* pSeg)
{
	CIPService newService;

	// ===== Convert Mpl Message to CIPService
	BOOL res = OnMplIpParametersToService(pSeg,newService);
	if(!res)
		return;
	// ===== Get 'old' ip service
	CIPService oldIpService = *m_pMngmntIpParams_asIpService_fromProcess;

	//check If Services IP's are equal
	res = IsIpParamsEqual(oldIpService,newService) ;

    //if we got also from the SWITCH to set the secure mode to FALSE, ignore secure value we get from SWITCH
    if (newService.GetIsSecured() && m_firstTimeReqForOutOfSecured)
    	newService.SetIsSecured(FALSE);

	if(res)
	{
		// Ip parameters are equal check for secure mode.
		if(oldIpService.GetIsSecured() != newService.GetIsSecured())
		{ // security is not equal.
			OnMplSecurityNotEqual(newService.GetIsSecured());
		}
		if(eNetConfigurationFailureActionChangeMngntIp4Type == m_pProcess->m_NetSettings.m_netConfigStatus)
		{
			SendMngmntUpdateToMpl();
		}
	}
	else
	{
		STATUS status = STATUS_OK;
		//Ip parameters are not eqaul update mamangment services which changes and send restart to mcmsdaemon.
		newService.SetDns(*oldIpService.GetpDns());
		// merge security managment and white list ip since it is not  sent to switch when updated
		CManagementSecurity *pOld = oldIpService.GetManagementSecurity();
		newService.SetManagementSecurity(*pOld);
		newService.SetWhiteList(oldIpService.GetWhiteList());

		//check for duplicate Ip's
		IP_ADDR_S ipAddrS;
		RetrieveIpAddresses(&newService, ipAddrS);
		status = CheckDuplicateIpCntrlAndShelf(ipAddrS);
		if(STATUS_OK != status && IsTarget())
		{

				string err = ERR_NO_MANAGEMENT_IP;
				if(STATUS_DUPLICATE_IPV4_ADDRESS_CNTRL == status)
				{
					err = ERR_NO_MANAGEMENT_IPV4;
				}
				else if(STATUS_DUPLICATE_IPV6_ADDRESS_CNTRL == status)
				{
					err = ERR_NO_MANAGEMENT_IPV6;
				}



				// no point to continue and configure both duplicated addresses
				if(STATUS_DUPLICATE_ADDRESS_CNTRL == status)
				{
					return;
				}

		}
		bool isShelfChanged = IsShelfChanged(oldIpService, newService);
		SendUSBChangeAuditEvent(eIpPartToConfig_All, isShelfChanged, oldIpService, newService);

		 // Updates in memory
		 m_pProcess->SetMngmntIpParams_asIpService(newService);
		 m_pMngmntIpParams_asIpService_fromProcess = m_pProcess->GetMngmntIpParams_asIpService();
		 m_pProcess->UpdateIpInterfaceInList(newService);
			 // ===== 7. update the file
		m_pMngmntIpParams_asIpService_fromProcess->WriteXmlFile(MANAGEMENT_NETWORK_CONFIG_PATH, "MngmntNetwork");

		 string desc ="Mpl Config Ip parameters changed need to restart mcms";
		 if(IsTarget()|| (eProductTypeGesher == m_mcuMngrProductType) || (eProductTypeNinja == m_mcuMngrProductType))
		 {
			 //BRIDGE-12843
			 if( IsRestoreDefaultsStartup() )
			 {
				    m_isRestoreIndBlock = TRUE;
			        CreateFile(RESTORE_FACTORY_PATH);
             }
			 //
			 CMcmsDaemonApi api;
			 api.SendResetReq(desc);
		 }
		 else
			 TRACEINTO << "Rmx Simulation don't update McmsDaemon for restart";
	}


}
/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnMngmntIpConfigInd(CSegment* pSeg)
{
	//printf("[debug]: ManageIpInd \n");
	TRACEINTOFUNC << "Enter";
	m_isMngmntIpConfigIndReceived = YES;
	RemoveActiveAlarmByErrorCode(AA_MPL_STARTUP_FAILURE_MNGMNT_IP_CNFG_NOT_RECEIVED);

    //This indication sent from MPL and it is not relevant to SoftMCU products
	if (m_bRestoreConfigSuccess == TRUE ||
		m_mcuMngrProductType==eProductTypeSoftMCU || m_mcuMngrProductType==eProductTypeSoftMCUMfw || m_mcuMngrProductType==eProductTypeEdgeAxis || m_mcuMngrProductType==eProductTypeCallGeneratorSoftMCU )
	{
		TRACEINTO << "CMcuMngrManager::OnMngmntIpConfigInd - restore the management defaults and send them to the switch";
		//send restore management to mpl
		SendMngmntUpdateToMpl();
		m_bRestoreConfigSuccess = FALSE;

		CManagerApi api(eProcessCSMngr);
		api.SendOpcodeMsg(MCUMNGR_TO_CSMNGR_END_IP_CONFIG_IND);
		SendMNGMNTInfoToCertMngr();
		return;
	}

	// 5.11.06: if user changed the Management_ip (via EMA) before getting the indication from MPL,
	//          then MPL's indication is ignored (user's decision is 'stronger')
	if (YES == m_isUserChangedManagementIp)
	{
	    TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnMngmntIpConfigInd -"
	                                  << "\nUser has changed ManagementIp via EMA; MPL's indication is therefore irrelevant";
	    return;
	}

	if((eProductFamilyRMX == ::ProductTypeToProductFamily(m_mcuMngrProductType)) || (eProductTypeGesher == m_mcuMngrProductType) || (eProductTypeNinja == m_mcuMngrProductType))
		{
			TRACEINTO << "OnMngmntIpConfigInd - McmsNetwork is enabled, set Mpl indication configuration recieved and call OnMplConfigIndChangeMcmsNetwork.";
			OnMplConfigIndChangeMcmsNetwork(pSeg);
			SendSecurityMode(eProcessAuthentication);
		}
		else
			TRACEINTO << "This indication sent from MPL and it is not relevant to pure SoftMCU products";


}

void CMcuMngrManager::SendMNGMNTInfoToCertMngr()
{
  std::string host_name;

  host_name = GetHostNameFromService(m_pMngmntIpParams_asIpService_fromProcess);
  TRACEINTOFUNC << "New host name is " << host_name;

  // ===== 1. insert host name to the segment
  CSegment* pParam = new CSegment;
  *pParam << host_name;

  // ===== 2. send to CertMngr
  const COsQueue* pCertMngrMbx = CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCertMngr, eManager);
  STATUS status = pCertMngrMbx->Send(pParam, MCUMNGR_TO_CERTMNGR_HOST_NAME_UPDATE);

  if (status != STATUS_OK)
    TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendMNGMNTInfoToCertMngr - Failed to send mngmnt info to CertMngr.";
}
/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnMngmntIpParamsUpdateInd(CSegment* pSeg)
{
	STATUS retStatus = STATUS_OK;

	// ---------------------------------------------
	// -------------- a. get the data --------------
	// ---------------------------------------------

	// ===== 1. get the data into IP_PARAMS_S object
	CMplMcmsProtocol* pMplMcmsProtocol = new CMplMcmsProtocol;
	pMplMcmsProtocol->DeSerialize(*pSeg);

	// ===== 2. validate data size
	STATUS sizeStat = pMplMcmsProtocol->ValidateDataSize( sizeof(IP_PARAMS_S) );
	if (STATUS_OK != sizeStat)
	  {
	    POBJDELETE(pMplMcmsProtocol);
		return;
	  }
	// ===== 3. get the info
	CIpParameters ipParamsFromMpl;
	ipParamsFromMpl.SetData(pMplMcmsProtocol->GetData());
	POBJDELETE(pMplMcmsProtocol);

    // ===== 4 validate and replace invalid strings
    ipParamsFromMpl.ValidateStrings();

	// ===== 5. print the data to trace (using Dump function)
    TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnMngmntIpParamsUpdateInd\n" << ipParamsFromMpl;

	// ===== 6. convert to CIPService
	CIPService theService;
	ipParamsFromMpl.ConvertToIpService(theService);


	// ---------------------------------------------
	// -------------- b. use the data --------------
	// ---------------------------------------------
	IP_ADDR_S ipAddrS;
	RetrieveIpAddresses(&theService, ipAddrS);
	// ===== 7. set IPv6 Shelf's addresses
	UpdateShelfMngrInfoIfNeeded("OnMngmntIpParamsUpdateInd", &theService, ipAddrS, eMngmntIpFromMpl, false);
}

/////////////////////////////////////////////////////////////////////////////
//void CMcuMngrManager::SendMNGMNTInfoToSnmp(OPCODE opcode) const
//{
//    SNMP_MMGMNT_INFO_S mngmntInfo;
//
//    mngmntInfo.mngmntIp = m_pProcess->GetMngmntIp();
//    mngmntInfo.shelfIp = m_pProcess->GetShelfIp();
//
//    const APIU8*  	mngmtIpv6Address = m_pProcess->GetMngmtIpV6ByteArray();
//    if (mngmtIpv6Address != NULL)
//    {
//    	memcpy(mngmntInfo.mngmtIpv6Address, mngmtIpv6Address, (size_t)IPV6_ADDRESS_BYTES_LEN);
//    	mngmntInfo.isMngmtIpv6Address = TRUE;
//    }
//	const APIU8*	shelfIpv6Address = m_pProcess->GetSheIfIpV6ByteArray();
//	if (shelfIpv6Address != NULL)
//	{
//		memcpy(mngmntInfo.shelfIpv6Address, shelfIpv6Address, (size_t)IPV6_ADDRESS_BYTES_LEN);
//		mngmntInfo.isShelfIpv6Address = TRUE;
//	}
//
//
//    CSegment *pSeg = new CSegment;
//    pSeg->Put((BYTE*)&mngmntInfo, sizeof(SNMP_MMGMNT_INFO_S));
//
//    CManagerApi apiSnmp(eProcessSNMPProcess);
//    apiSnmp.SendMsg(pSeg, opcode);
//}
void CMcuMngrManager::SendMNGMNTInfoToProcess(eProcessType process, OPCODE opcode) const
{
	CSegment* pSeg = new CSegment;
	*pSeg << m_pProcess->GetMngmntIp();

    if (m_mcuMngrProductType==eProductTypeSoftMCU || m_mcuMngrProductType==eProductTypeSoftMCUMfw || m_mcuMngrProductType==eProductTypeEdgeAxis)
	{
    	std::string nic_name = "eth0";
    	CIPSpan * pSpan	= m_pMngmntIpParams_asIpService_fromProcess->GetFirstSpan();
	    if(pSpan)
	        nic_name = pSpan->GetInterface();
	    *pSeg << nic_name;
	}


	CManagerApi api(process);
	api.SendMsg(pSeg, opcode);
}

/////////////////////////////////////////////////////////////////////////////
bool CMcuMngrManager::IsShelfChanged(CIPService &service1, CIPService &service2)
{
    CIPSpan *pShelfSpan1 = service1.GetSpanByIdx(1);
    CIPSpan *pShelfSpan2 = service2.GetSpanByIdx(1);

    if( (NULL == pShelfSpan1 || NULL == pShelfSpan2) )
    {
        PTRACE(eLevelInfoNormal, "CMcuMngrManager::IsShelfChanged - One of shelf spans does not exist");

        if(pShelfSpan1 == pShelfSpan2)
        {
            return false;  // both of them are NULL
        }
        else
        {
            return true;   // only one of them is NULL
        }
    }

    bool areEqual = (*pShelfSpan1 == *pShelfSpan2);

    if (areEqual)
    {
    	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::IsShelfChanged - Shelf spans are equal";
    }
    else
    {
    	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::IsShelfChanged - Shelf spans are not equal";
    }

    return !areEqual;
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendUSBChangeAuditEvent(eIpPartToConfigType whatToConfig,
                                              bool isShelfChanged,
                                              CIPService &serviceBefore,
                                              CIPService &serviceAfter)
{
    string data = "Change details : ";
    switch(whatToConfig)
    {
        case eIpPartToConfig_Dns:
        {
            data += "DNS configuration";
            if(isShelfChanged)
            {
                data += ", Shelf IP address";
            }

            break;
        }

        case eIpPartToConfig_All:
        {
            data += "DHCP or IP  or Router configuration";
            if(isShelfChanged)
            {
                data += ", Shelf IP address";
            }
            break;
        }

        case eIpPartToConfig_WithoutCsReset:
        {
        	data = "IP configuration that doesn't effect the CS was changed";
        	break;
        }
        default:
        {
            if(isShelfChanged)
            {
                data += "Shelf IP address";
            }
            else
            {
                PASSERTMSG(whatToConfig + 100, "invalid type of configuration (code = type + 100)");
                data = "invalid type of configuration";
            }

            break;
        }
    }

    CXMLDOMElement *serviceBeforeXml = NULL;
    serviceBefore.SerializeXml(serviceBeforeXml);
    char *buffBefore = NULL;
    serviceBeforeXml->DumpDataAsLongStringEx(&buffBefore, TRUE);

    CXMLDOMElement *serviceAfterXml = NULL;
    serviceAfter.SerializeXml(serviceAfterXml);
    char *buffAfter = NULL;
    serviceAfterXml->DumpDataAsLongStringEx(&buffAfter, TRUE);

    AUDIT_EVENT_HEADER_S outAuditHdr;
    CAuditorApi::PrepareAuditHeader(outAuditHdr,
                                    "",
                                    eMcms,
                                    "",
                                    "",
                                    eAuditEventTypeInternal,
                                    eAuditEventStatusOk,
                                    "IP Management Service modified",
                                    "IP Management Service parameters were modified by USB key.",
                                    "",
                                    data);
    CFreeData freeData;
    CAuditorApi::PrepareFreeData(freeData,
                                 "Previous Parameters",
                                 eFreeDataTypeXml,
                                 buffBefore,
                                 "Updated Parameters",
                                 eFreeDataTypeXml,
                                 buffAfter);

    CAuditorApi api;
    api.SendEventMcms(outAuditHdr, freeData);

    delete [] buffBefore;
    delete [] buffAfter;
    PDELETE(serviceBeforeXml);
    PDELETE(serviceAfterXml);
}

/////////////////////////////////////////////////////////////////////////////
eIpPartToConfigType CMcuMngrManager::WhatIpPartShouldBeConfigured(CIPService &service1, CIPService &service2, BOOL& bResetApache)
{
	eIpPartToConfigType whatToConfig = eIpPartToConfig_Nothing;
	string tempStr = "nothing to config";

	// ===== check equality
	if ( NO == IsDnsParamsEqual(service1, service2) )
	{
		whatToConfig = eIpPartToConfig_Dns;
		tempStr = "config dns params only";
	}

	if (YES == IsTarget())
	{
		if (NO == IsPermenentNetworkOpenEqual(service1, service2))
		{
			whatToConfig = eIpPartToConfig_WithoutCsReset;
			tempStr = "config parameters that doesn't reset the CS";
			bResetApache = TRUE;
		}
	}

	if ( NO == IsSecuredEqual(service1, service2) )
	{
		whatToConfig = eIpPartToConfig_WithoutCsReset;
		tempStr = "config parameters that doesn't reset the CS";
		bResetApache = TRUE;
	}

	if ( (NO == IsDhcpEqual(service1, service2))		||
	     (NO == IsRoutersEqual(service1, service2)))
	{
		whatToConfig = eIpPartToConfig_All;
		tempStr = "config the whole interface";
	}

	if ( NO == IsIpParamsEqual(service1, service2))
	{
		whatToConfig = eIpPartToConfig_All;
		tempStr = "config the whole interface";
		bResetApache = TRUE;
	}

	string retStr = "\nCMcuMngrManager::WhatIpPartShouldBeConfigured - ";
	retStr += tempStr;

	// ===== print to log
	TRACESTR(eLevelInfoNormal) << retStr.c_str();

	return whatToConfig;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CMcuMngrManager::IsDhcpEqual(CIPService &service1, CIPService &service2)
{
	BOOL retVal = YES;

	// ===== 1. get info
	WORD isDhcpEnabled1 = service1.GetDHCPServer(),
	     isDhcpEnabled2 = service2.GetDHCPServer();

	// ===== 2. check equality and print to log
	if (isDhcpEnabled1 != isDhcpEnabled2)
	{
		retVal = NO;
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::IsDhcpEqual - Dhcp params differ";
	}
	else
	{
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::IsDhcpEqual - Dhcp params equal";
	}

	return retVal;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CMcuMngrManager::IsIpParamsEqual(CIPService &service1, CIPService &service2)
{
	BOOL retVal = YES;

	IP_ADDR_S ipAddrS1, ipAddrS2;
	RetrieveIpAddresses(&service1, ipAddrS1);
	RetrieveIpAddresses(&service2, ipAddrS2);

   	// ---- 1b. other params
	DWORD vLanId1	= service1.GetVlanId(),
	      vLanId2	= service2.GetVlanId();


	if((eNetConfigurationSuccess == m_pProcess->m_NetSettings.m_netConfigStatus) ||
		  (eNetConfigurationFailureNoAction == m_pProcess->m_NetSettings.m_netConfigStatus)	)
		{
			if(ipAddrS1.ipType != ipAddrS2.ipType)
					retVal = NO;
		}


	// ===== 2. check equality and print to log
	if (
		 (ipAddrS1.ipV6configType != ipAddrS2.ipV6configType)					||
		 (ipAddrS1.iPv4	!= ipAddrS2.iPv4)										||
//		 strncmp(ipAddrS1.iPv6_str, ipAddrS2.iPv6_str, IPV6_ADDRESS_LEN)		||	// will be checked separately
		 strcmp(ipAddrS1.iPv4_netMask, ipAddrS2.iPv4_netMask)					||
	     (ipAddrS1.iPv4_defGW != ipAddrS2.iPv4_defGW)							||
	     (vLanId1 != vLanId2)	) //											||
//	     strncmp(ipAddrS1.iPv6_defGW, ipAddrS2.iPv6_defGW, IPV6_ADDRESS_LEN)	||	// will be checked separately
//	     (defGwMask1	!= defGwMask2) )											// will be checked separately
	{
		retVal = NO;
	}

	// in Auto mode, the followings should not be compared
	//    (since they are automatically determined by the system)
	if (eV6Configuration_Auto != ipAddrS2.ipV6configType)
	{
		DWORD defGwMask1 = service1.GetDefaultGatewayMaskIPv6(),
		      defGwMask2 = service2.GetDefaultGatewayMaskIPv6();

		if ( strncmp(ipAddrS1.iPv6_str, ipAddrS2.iPv6_str, IPV6_ADDRESS_LEN)		||
			 strncmp(ipAddrS1.iPv6_defGW, ipAddrS2.iPv6_defGW, IPV6_ADDRESS_LEN)	||
			 (defGwMask1 != defGwMask2) )
		{
			retVal = NO;
		}
	}

	TRACEINTO << "CMcuMngrManager::IsIpParamsEqual - relevant Ip params are " << (retVal == NO ? "not " : "") << "equal";

	return retVal;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CMcuMngrManager::IsDnsParamsEqual(CIPService &service1, CIPService &service2)
{
	BOOL retVal = YES;

	// ===== 1. get info
	CIpDns *pDns1=NULL, *pDns2=NULL;

	eServerStatus serverStatus_dns1 = eServerStatusOff,
	              serverStatus_dns2 = eServerStatusOff;

	int     ipAdd0_dns1=0, ipAdd1_dns1=0,
	        ipAdd0_dns2=0, ipAdd1_dns2=0;

	char    domainName1[NAME_LEN],
	        domainName2[NAME_LEN];
	memset(domainName1, 0, NAME_LEN);
	memset(domainName2, 0, NAME_LEN);

	// ---- 1a. server status, server addresses, domain names
	pDns1 = service1.GetpDns();
	pDns2 = service2.GetpDns();

	if (pDns1)
	{
		serverStatus_dns1 = pDns1->GetStatus();
		ipAdd0_dns1 = pDns1->GetIPv4Address(0);
        ipAdd1_dns1 = pDns1->GetIPv4Address(1);
		memcpy(domainName1, pDns1->GetDomainName().GetString(), NAME_LEN);
	}

	if (pDns2)
	{
		serverStatus_dns2 = pDns2->GetStatus();
		ipAdd0_dns2 = pDns2->GetIPv4Address(0);
        ipAdd1_dns2 = pDns2->GetIPv4Address(1);
		memcpy(domainName2, pDns2->GetDomainName().GetString(), NAME_LEN);
	}

	// ---- 1b. host names
	string   hostName1,    hostName2;


   	hostName1 = GetHostNameFromService(&service1);
   	hostName2 = GetHostNameFromService(&service2);

	// ===== 2. check equality and print to log
	if ( (serverStatus_dns1	!= serverStatus_dns2)	||
	     (ipAdd0_dns1		!= ipAdd0_dns2)			||
	     (ipAdd1_dns1		!= ipAdd1_dns2)			||
	     (hostName1			!= hostName2)			||
	     strncmp(domainName1, domainName2, NAME_LEN) )
	{
		retVal = NO;
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::IsDnsParamsEqual - Dns params differ";
	}
	else
	{
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::IsDnsParamsEqual - Dns params equal";
	}

	return retVal;
}
/////////////////////////////////////////////////////////////////////////////
BOOL CMcuMngrManager::IsSecuredEqual(CIPService service1, CIPService service2)
{
	BOOL retVal = YES;

	// ===== 1. get info
	WORD isSecured1 = service1.GetIsSecured(),
	     isSecured2 = service2.GetIsSecured();

	// ===== 2. check equality and print to log
	if (isSecured1 != isSecured2)
	{
		retVal = NO;
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::IsSecuredEqual - Secured is differ - isSecured1 = " << (YES == isSecured1 ? "YES" : "NO") << " while isSecured2 = " << (YES == isSecured2 ? "YES" : "NO");
	}
	else
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::IsSecuredEqual - Secured is equal. both = "<<(YES == isSecured1 ? "YES" : "NO");

	return retVal;
}
/////////////////////////////////////////////////////////////////////////////
BOOL CMcuMngrManager::IsPermenentNetworkOpenEqual(CIPService service1, CIPService service2)
{
	BOOL retVal = YES;

	// ===== 1. get info
	BOOL isPermanent1 = service1.GetIsPermanentNetworkOpen(),
	     isPermanent2 = service2.GetIsPermanentNetworkOpen();

	// ===== 2. check equality and print to log
	if (isPermanent1 != isPermanent2)
	{
		retVal = NO;
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::IsPermenentNetworkOpenEqual - Permanent network is differ - isPermanent1="<<(YES == isPermanent1 ? "YES" : "NO") << " while isPermanent2 = " << (YES == isPermanent2 ? "YES" : "NO");
	}
	else
	{
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::IsPermenentNetworkOpenEqual - Permanent network is equal. both = "<<(YES == isPermanent1 ? "YES" : "NO");
	}

	return retVal;
}
/////////////////////////////////////////////////////////////////////////////
BOOL CMcuMngrManager::IsRoutersEqual(CIPService service1, CIPService service2)
{
	// ===== 1. check equality
	if ( service1.GetRoutersNumber() != service2.GetRoutersNumber() )
	{
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::IsRoutersEqual - unequal: "
		                       << "service1 has " << service1.GetRoutersNumber() << " routers, "
		                       << "service2 has " << service2.GetRoutersNumber() << " routers";
		return NO;
	}

	// if both services have same number of routers
	const CH323Router *pCurRouter1 = service1.GetFirstRouter(),
	                  *pCurRouter2 = service2.GetFirstRouter();

	BOOL retVal = YES;
	while ( (NULL != pCurRouter1) && (NULL != pCurRouter2) )
	{
		if ( (pCurRouter1->GetRemoteIP()	!= pCurRouter2->GetRemoteIP())		||
		     (pCurRouter1->GetSubnetMask()	!= pCurRouter2->GetSubnetMask())	||
		     (pCurRouter1->GetRouterIP()	!= pCurRouter2->GetRouterIP())		||
		     (pCurRouter1->GetRemoteFlag()	!= pCurRouter2->GetRemoteFlag()) )
		{
			retVal = NO;
			break;
		}

		pCurRouter1 = service1.GetNextRouter();
		pCurRouter2 = service2.GetNextRouter();
	}

	// ===== 2. print to log
	if (NO == retVal)
	{
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::IsRoutersEqual - Routers params differ";
	}
	else
	{
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::IsRoutersEqual - Routers params equal";
	}

	return retVal;
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnSoftwareLocationInd(CSegment* pSeg)
{
	// ===== 1. fill CMcuMngrManager's attribute with data from segment received
	CMplMcmsProtocol* pMplMcmsProtocol = new CMplMcmsProtocol;
	pMplMcmsProtocol->DeSerialize(*pSeg);

	// validate data size
	STATUS sizeStat = pMplMcmsProtocol->ValidateDataSize( sizeof(MPL_SW_LOCATION_S) );
	if (STATUS_OK != sizeStat)
	  {
	    POBJDELETE(pMplMcmsProtocol);
		return;
	  }
	m_softwareLocation.SetData(pMplMcmsProtocol->GetData());
	POBJDELETE(pMplMcmsProtocol);

    // ==== 1.5 validate and replace invalid strings
    m_softwareLocation.ValidateStrings();

	// ===== 2. print the data to trace (using Dump function)
    TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnSoftwareLocationInd\n" << m_softwareLocation;
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnFaultGeneralInd(CSegment* pSeg)
{
	// ===== 1. retrieve data from segment received
	CMplMcmsProtocol* pMplMcmsProtocol = new CMplMcmsProtocol;
	pMplMcmsProtocol->DeSerialize(*pSeg);

	FAULT_GENERAL_S faultStruct;
	memset( &faultStruct, 0, sizeof(FAULT_GENERAL_S) );
	memcpy( &faultStruct, pMplMcmsProtocol->GetData(), sizeof(FAULT_GENERAL_S) );
	faultStruct.description[GENERAL_MES_LEN-1] = 0;

	// ===== 2. test string validity
	m_pProcess->TestStringValidity( (char*)(faultStruct.description),
						            GENERAL_MES_LEN,
						            "CMcuMngrManager::OnFaultGeneralInd" );

	// ===== 3. add to log & fault
	string faultString = (char*)(faultStruct.description);

	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnFaultGeneralInd"
	                       << "\nDescription: " << faultString.c_str();

	BOOL isFullOnly = FALSE;
	CHlogApi::TaskFault( FAULT_GENERAL_SUBJECT,
	                     EXTERNAL_FAULT,
	                     SYSTEM_MESSAGE,
	                     faultString.c_str(),
	                     isFullOnly);

	POBJDELETE(pMplMcmsProtocol);
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnRestoreFactoryInd(CSegment* pSeg)
{

	// ===== 1. retrieve restore type and status from segment received


	MPL_RESTORE_DEFAULT_S dataStruct;
    memset(&dataStruct,0,sizeof(MPL_RESTORE_DEFAULT_S));
    pSeg->Get( (BYTE*)&dataStruct, sizeof(MPL_RESTORE_DEFAULT_S));


	// ===== 1. check if restore factory flag was set already
	//          if flag was set, just delete the flag
	if(IsRestoreDefaultsStartup())
	{
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnRestoreFactoryInd: "
							   << "System was already reset due to restore defaults";



		m_isRestoreDefaultsStartup = FALSE;
		if ( (CProcessBase::GetProcess()->GetProductFamily() == eProductFamilyRMX )  && !m_isRestoreIndBlock)
		{
			DeleteFile(RESTORE_FACTORY_PATH);
		}

		return;
	}

	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnRestoreFactoryInd - "
								  << "\nRestore type received: " << dataStruct.mcuRestoreType
								  << " (" << GetMcuRestoreName( (eMcuRestoreType)dataStruct.mcuRestoreType ) << ")";


	if (dataStruct.mcuRestoreStatus == eMcuRestoreStatusFailure)  //this can be 0-FAILURE or 1-SUCCESS
	{
			TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnRestoreFactoryInd: "
										   << "Last reset due to restore defaults has been failed";

			return;
	}

	// ===== 3. validate type, and act accordingly
	eMcuRestoreType restoreTypeFromSwitch = (eMcuRestoreType)dataStruct.mcuRestoreType;
	switch(restoreTypeFromSwitch)
	{
		case eMcuRestoreNone:
		{
			TRACESTR(eLevelInfoNormal) << "Restore type is \"None\", no action should be taken\n";
			break;
		}

		case eMcuRestoreStandard:
		case eMcuRestoreInhensive:
		default: // in V3.0 and V4.0 we perform 'Inhensive' restore anyway
		{
			// Restore defaults ( + reset )
			string answer = "";
			STATUS status = PerformRestoreFactoryDefaults(/*restoreTypeFromSwitch*/eMcuRestoreInhensive,
								      answer,
								      true); //  sagi: VNGR-14828
			break;
		}
	}
}

void CMcuMngrManager::OnEncryptionKeyServerFips140TestResultInd(CSegment* pSeg)
{
	ENCRYPTIONKEYSERVER_FIPS_140_TEST_INFO_S *pEncryptionFips140TestInfoStruct = (ENCRYPTIONKEYSERVER_FIPS_140_TEST_INFO_S*)pSeg->GetPtr();

	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnEncryptionKeyServerFips140TestResultInd - Fips 140 state: "
		                   << (pEncryptionFips140TestInfoStruct->result ? "FAIL" : "OK");

	RemoveActiveAlarmByErrorCode(AA_NO_FIPS_140_TEST_RESULT_FROM_ENCRYPTIONKEYSERVER);

	if(pEncryptionFips140TestInfoStruct->result != STATUS_OK)
	{
		AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT,
								AA_FIPS_140_FAILURE_WITHIN_ENCRYPTIONKEYSERVER,
								MAJOR_ERROR_LEVEL,
								"FIPS 140 failure within encryption module",
								true,
								true);
	 // ===== Send Conference block to ConfParty
		m_isConfBlock = TRUE;
		SendConfBlockToConfParty();
	}
}
/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendResetAllMfaBoardsReqToMplApi()
{
	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::SendResetAllMfaBoardsReqToMplApi";

	CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol();

	mplPrtcl->AddCommonHeader(MCUMNGR_RESET_ALL_MFA_BOARDS_REQ);
	mplPrtcl->AddMessageDescriptionHeader();
	mplPrtcl->AddPhysicalHeader(1, m_switchBoardId, m_switchSubBoardId);

	CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("MCUMNGR_SENDS_TO_MPL_API");
	mplPrtcl->SendMsgToMplApiCommandDispatcher();

	POBJDELETE(mplPrtcl);

}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendRestoreFactoryDefaultReqToMplApi(eMcuRestoreType restoreType)
{
	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::SendRestoreFactoryDefaultReqToMplApi";

	MPL_RESTORE_DEFAULT_S restoreDefault;

	restoreDefault.mcuRestoreType   = restoreType;


	CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol();

	mplPrtcl->AddCommonHeader(RESTORE_FACTORY_DEFAULT_REQ);
	mplPrtcl->AddMessageDescriptionHeader();
	mplPrtcl->AddPhysicalHeader(1, m_switchBoardId, m_switchSubBoardId);
	mplPrtcl->AddData(sizeof(MPL_RESTORE_DEFAULT_S), (char*)&restoreDefault);

	CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("MCUMNGR_SENDS_TO_MPL_API");
	mplPrtcl->SendMsgToMplApiCommandDispatcher();

	POBJDELETE(mplPrtcl);

}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnCsNumOfPortsReq(CSegment* pSeg)
{
	if (YES == m_isMngmntIpConfigIndReceived)
	{
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnCsNumOfPortsInd -"
		                              << "\nNum of cp ports sent:" << m_pLicensing->GetTotalNumOfCpParties();

		// ===== 1. insert numOfPorts to the segment
		CSegment*  pRetParam = new CSegment();
		*pRetParam << m_pLicensing->GetTotalNumOfCpParties();


		// ===== 2. send to CentralSignaling
		const COsQueue* pCsMngrMbx =
				CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCSMngr, eManager);

		STATUS res = pCsMngrMbx->Send(pRetParam, CS_MCU_NUM_OF_PORTS_IND);
	}

	else // MngmntIpConfigInd was not received
	{
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnCsNumOfPortsInd -"
		                              << "\nNothing is sent since MPL_MNGMNT_IP_CONFIG_IND was not received";
	}
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnEncryptionKeyServerIsNtpSyncLegalReq(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "McuMngrManager::OnEncryptionKeyServerIsNtpSyncLegalReq " <<  (int)m_isNtpSyncLegal  ;
	CSegment*  pRetParam = new CSegment();
	*pRetParam << m_isNtpSyncLegal;

	ResponedClientRequest(STATUS_OK, pRetParam);

}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnDnsAgentConfigurationReq(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnDnsAgentConfigurationReq";

	if ( NO == IsTarget() )
	{
		m_dnsConfigurationStatus = eDnsNotConfigured;
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnDnsAgentConfigurationReq - "
		                              << "System is not Target; a 'not configured' answer is sent to DnsAgent";
	}

	SendDnsConfigStatusToDnsAgent();
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnSipProxyConfigurationReq(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnSipProxyConfigurationReq";

	if ( NO == IsTarget() )
	{
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnDnsAgentConfigurationReq - "
		                              << "System is not Target; a 'not configured' answer is sent to DnsAgent";
	}

	SendDnsConfigStatusToSipProxy();
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnInstallerCfsParamsReq(CSegment* pSeg)
{
	CMedString retStr = "\nCMcuMngrManager::OnInstallerCfsParamsReq - ";
	if (YES == m_isAuthenticationIndReceived)
	{
		retStr << "Sending to Installer";
		SendCfsParamsToInstaller();
	}
	else
	{
		retStr << "AuthenticationInd was not received; nothing is being sent to Installer";
	}

	TRACESTR(eLevelInfoNormal) << retStr.GetString();
}




/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnApacheModuleAuthenticationStructAndMultipleServicesReq(CSegment* pSeg)
{
	CMedString retStr = "\nCMcuMngrManager::OnApacheModuleAuthenticationStructAndMultipleServicesReq - ";
	if (YES == m_isAuthenticationIndReceived)
	{
		retStr << "Sending to ApacheModule";
		SendAuthenticationStructToSpecificProcess(eProcessApacheModule);
		SendLicensingParamsToApacheModuleMngr();
                SendMultipleServicesToApache();
	}
	else
	{
		retStr << "AuthenticationInd was not received; nothing is being sent to ApacheModule";
	}

	TRACESTR(eLevelInfoNormal) << retStr.GetString();
}




//manage all cfg param changes which are not sent via ca command
void CMcuMngrManager::CfgParamChanged(CCfgData* cfgData)
{
	std::string key = cfgData->GetKey();
	std::string data = cfgData->GetData();
	if (key == "QOS_MANAGEMENT_NETWORK")
	{
		SendQosManagementDSCPToProcess(data, eProcessConfigurator);
		SendQosManagementDSCPToMplApi();
		TRACESTR(eLevelInfoNormal) <<"CMcuMngrManager::CfgParamChanged. key=" << key.c_str() << ", data=" << data.c_str();
	}

}
//Added by flora for LAN_REDUNDANCY conflict
STATUS CMcuMngrManager::isSystemFlagConflict(CSysConfigEma* sys_cfg, const std::string &key, CRequest* pRequest)
{
	// when V35_ULTRA_SECURED_SUPPORT = YES
	eProductType prodType = m_pProcess->GetProductType();
	if ( eProductTypeGesher == prodType || eProductTypeNinja == prodType )
	{
		//The date in file: SystemCfgUserTmp.xml
		std::string strSeparatedNetworkData_tmp = "";
		std::string strMultipleServicesData_tmp = "";
    	std::string strLanRedundancyData_tmp = "";
		
    	//get SEPARATE_NETWORK, MULTIPLE_SERVICES and LAN_REDUNDANCY value from EMA map
		if (sys_cfg->IsParamExist(CFG_KEY_SEPARATE_NETWORK))
		{
			CCfgData* cfgData = sys_cfg->GetCfgEntryByKey(CFG_KEY_SEPARATE_NETWORK);
			strSeparatedNetworkData_tmp = cfgData->GetData();
		}
		else
		{
			//Default value
			strSeparatedNetworkData_tmp = CFG_STR_NO;
		}

		if (sys_cfg->IsParamExist(CFG_KEY_MULTIPLE_SERVICES))
		{
			CCfgData* cfgData = sys_cfg->GetCfgEntryByKey(CFG_KEY_MULTIPLE_SERVICES);
			strMultipleServicesData_tmp = cfgData->GetData();
		}
		else
		{
			//Default value
			strMultipleServicesData_tmp = CFG_STR_NO;
		}

		if (sys_cfg->IsParamExist(CFG_KEY_LAN_REDUNDANCY))
		{
			CCfgData* cfgData = sys_cfg->GetCfgEntryByKey(CFG_KEY_LAN_REDUNDANCY);
			strLanRedundancyData_tmp = cfgData->GetData();
		}
		else
		{
			//Default value
			strLanRedundancyData_tmp = CFG_LANREDUNDANCY_DEFAULT;
		}

		//for gesher/ninja, seperate network and multiple service and Lan redundancy are conflict.
    	STATUS separatedNet_changed, multipleServices_changed, lanRedundancy_changed;
		int status = 0;
		if ( CFG_KEY_SEPARATE_NETWORK == key )
		{
			if (0 == strSeparatedNetworkData_tmp.compare(CFG_STR_YES))
			{
				if (("YES" == strMultipleServicesData_tmp) || ("YES" == strLanRedundancyData_tmp))
				{
					if ("YES" == strMultipleServicesData_tmp)
					{
						status = STATUS_CFG_PARAM_MISSMATCH_NETWORK_SEPARATION_AND_MULTIPLE_SERVICES;
					}
					else
					{
						status = STATUS_CFG_PARAM_MISSMATCH_LAN_REDUNDANCY_AND_NETWORK_SEPARATION;
					}
					TRACEINTO << "System Flag Confilct : Multiple Services & Lan Redundancy" << endl;
					pRequest->SetStatus(status);
    				return STATUS_OK;
				}
			}
		}

		if ( CFG_KEY_MULTIPLE_SERVICES == key )
		{
			if (0 == strMultipleServicesData_tmp.compare(CFG_STR_YES))
			{
				if (("YES" == strSeparatedNetworkData_tmp) || ("YES" == strLanRedundancyData_tmp))
				{
					if ("YES" == strSeparatedNetworkData_tmp)
					{
						status = STATUS_CFG_PARAM_MISSMATCH_NETWORK_SEPARATION_AND_MULTIPLE_SERVICES;
					}
					else
					{
						status = STATUS_CFG_PARAM_MISSMATCH_LAN_REDUNDANCY_AND_MULTIPLE_SERVICES;
					}
					TRACEINTO << "System Flag Confilct : Separated Network & Lan Redundancy" << endl;
					pRequest->SetStatus(status);
    				return STATUS_OK;
				}
			}
		}


		if ( CFG_KEY_LAN_REDUNDANCY == key )
		{
			if (0 == strLanRedundancyData_tmp.compare(CFG_STR_YES))
			{
				if (("YES" == strSeparatedNetworkData_tmp) || ("YES" == strMultipleServicesData_tmp))
				{
					if ("YES" == strSeparatedNetworkData_tmp)
					{
						status = STATUS_CFG_PARAM_MISSMATCH_LAN_REDUNDANCY_AND_NETWORK_SEPARATION;
					}
					else
					{
						status = STATUS_CFG_PARAM_MISSMATCH_LAN_REDUNDANCY_AND_MULTIPLE_SERVICES;
					}
					TRACEINTO << "System Flag Confilct : Separated Network & Multiple Services" << endl;
					pRequest->SetStatus(status);
    				return STATUS_OK;
				}
			}

		}
	}

	return STATUS_FAIL;
}

//update one flag and send ca command if no reset is needed
STATUS CMcuMngrManager::HandleSetCfgParam(CRequest *pRequest)
{
	TRACESTR(eLevelInfoNormal) <<"inside CMcuMngrManager::HandleSetCfgParam";
	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK_WARNING);

	//Authorization validation
	if (pRequest->GetAuthorization() != SUPER)
	{
		TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::HandleSetCfgParam: No permission to set system.cfg " << pRequest->GetAuthorization();
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_OK;
	}

	//Validate flag from EMA
	CLargeString strError;
	CSysConfigSet *pSysConfigSet = (CSysConfigSet*)pRequest->GetRequestObject();
	CSysConfigEma* cfgEMA = pSysConfigSet->GetSystemCfg();
	STATUS status = cfgEMA->IsValidOrChanged(strError);//check if new params are valid and different from memory params
	pRequest->SetStatus(status);
	if(STATUS_OK != status && STATUS_OK_WARNING != status)
	{
		pRequest->SetExDescription(strError.GetString());
		return STATUS_OK;
	}

	//Deserialize flag from EMA
	CSysMap* map = cfgEMA->GetMap();
	CSysMap::iterator iTer = map->begin();
	CCfgData* cfgDataEMA = iTer->second;


	//Get flag from memory
	CProcessBase* pProcess = CProcessBase::GetProcess();
	CSysConfig* memorySysConfig = pProcess->GetSysConfig();
	CSysMap* memoryMap =  memorySysConfig->GetMap();
	iTer = memoryMap->find(cfgDataEMA->GetKey());
	if (iTer == memoryMap->end())
	{
		//if trying to add/edit flag which is not in memory
		pRequest->SetStatus(STATUS_CFG_PARAM_NOT_EXIST);
        
        //added for BRIDGE-13424
        strError = "Flag does not exist: ";
        strError << cfgDataEMA->GetKey();
        pRequest->SetExDescription(strError.GetString());
        
		return STATUS_OK;
	}
	CCfgData* cfgDataMemory = (CCfgData*)iTer->second;




	//load from SystemCfgUserTmp.xml and write back with change
	CSysConfigEma* cfgFile = new CSysConfigEma();
	cfgFile->LoadFromFile(eCfgParamUser);
	//update flag
	if (cfgFile->IsParamExist(cfgDataEMA->GetKey()))
	{
		CCfgData* cfgData = cfgFile->GetCfgEntryByKey(cfgDataEMA->GetKey());
		if (cfgData->GetData() != cfgDataEMA->GetData()) // && (cfgDataMemory->GetIsReset() == true))
		{
			pRequest->SetStatus(STATUS_OK_WARNING);
		}
		cfgData->SetData(cfgDataEMA->GetData());
	}
	//insert new flag
	else
	{
		cfgFile->AddParamVisible(cfgDataEMA->GetSection(),cfgDataEMA->GetKey(),cfgDataEMA->GetData(),cfgDataMemory->GetCfgType(),cfgDataMemory->GetTypeValidator(),
				cfgDataMemory->GetCfgParamResponsibilityGroup(),cfgDataMemory->GetCfgDataType());
	}
	//Check MULTIPLE_SERVICES and exit if needed
	if (CheckMultipleServices(cfgEMA, pRequest) != STATUS_OK)
		return STATUS_OK;

	if (CheckLanRedundancy(cfgFile, pRequest) != STATUS_OK)
	{
		return STATUS_OK;
	}

	//Set SEPARATE_MANAGEMENT_NETWORK flag and exit if needed
	if (SetSeparateManagementNetworkFlag(cfgFile, cfgEMA, pRequest) != STATUS_OK)
		return STATUS_OK;
    //We must check on the Whole SystemCfgUserTmp.xml file, but not only the Cfg through EMA, 
	//There is only one item in  cfgEMA
	if (STATUS_OK == isSystemFlagConflict(cfgFile,cfgDataEMA->GetKey(), pRequest))
	{
		TRACEINTO << "System Flag: "  << cfgDataEMA->GetKey() << " Conflict detected, return." << endl;
		return STATUS_OK;
	}

    //MTU Size
    if (STATUS_FAIL == HandleMediaMtuSize(cfgEMA))
    {
        pRequest->SetStatus(STATUS_IPV6_MTU_SIZE_MUST_NO_LESS_THAN_1280);
        TRACEINTO << "CMcuMngrManager::HandleSetCfgParam System Flag MEDIA_NIC_MTU_SIZE must no less then 1280, return.";
        return STATUS_OK;
    }

	//write to file
	TRACEINTO << "save into SystemCfgUserTmp.xml: " << cfgDataEMA->GetKey() << endl;
	cfgFile->SaveToFile("Cfg/SystemCfgUserTmp.xml");
	TRACEINTO << "done saving into SystemCfgUserTmp.xml: " << cfgDataEMA->GetKey() << endl;
	//end load from SystemCfgUserTmp.xml and write back with change


	//Set visible in memory
	if (cfgDataMemory->GetCfgParamVisibilityType() == eCfgParamNotVisible)
	{
		cfgDataMemory->SetCfgParamVisibilityType(eCfgParamVisible);
	}

	//If no reset needed update flag value in memory and send ca cmd
	if (cfgDataMemory->GetIsReset() == false)
	{
		TRACESTR(eLevelInfoNormal) <<"cfgDataMemory->GetIsReset() == false";
		const char* processName;

		cfgDataMemory->SetData(cfgDataEMA->GetData());

		//send ca cmd to relevant process
		if (cfgDataMemory->GetIsAllProcesses() == true)
		{
			processName = "all";
		}
		else
		{
			processName = ProcessTypeToString(cfgDataMemory->GetProcessType());
		}
		//if eProcessMcuMngr then no need to send ca cmd
		if (cfgDataMemory->GetProcessType() == eProcessMcuMngr)
		{
			CfgParamChanged(cfgDataMemory);
			pRequest->SetStatus(STATUS_OK);
		}
		else
		{
			std::ostringstream cmd;
			cmd  << MCU_MCMS_DIR+"/Bin/McuCmd set" << " " << processName << " " << cfgDataEMA->GetKey() << " " << cfgDataEMA->GetData();
			std::string ans;
			STATUS stat;
			stat = SystemPipedCommand(cmd.str().c_str(), ans);
			//set status to OK (WARNING by default)
			if (stat == STATUS_OK)
			{
				pRequest->SetStatus(STATUS_OK);
			}
			//todo: set status "failed ca command"
			else
			{
				//pRequest->SetStatus(STATUS_FAILED_CA_CMD);
			}
		}
	}

	return STATUS_OK;
}

STATUS CMcuMngrManager::HandleMediaMtuSize(CSysConfigEma* cfgEMA)
{
    std::string mtuAnswer;
    eProductType prodType = m_pProcess->GetProductType();
	STATUS mtuChanged = cfgEMA->CheckIfKeyChanged(CFG_KEY_MEDIA_NIC_MTU_SIZE, mtuAnswer);
    TRACEINTO << "CMcuMngrManager::HandleMediaMtuSize mtuChanged:" << mtuChanged;
    
    if (STATUS_OK == mtuChanged)
    {   
        DWORD dMtuSize = 0;
        CCfgData *cfgData = cfgEMA->GetCfgEntryByKey(CFG_KEY_MEDIA_NIC_MTU_SIZE);
        dMtuSize = atoi(cfgData->GetData().c_str());
        
        //cfgEMA->GetDWORDDataByKey(CFG_KEY_MEDIA_NIC_MTU_SIZE, dMtuSize);
        TRACEINTO << "CMcuMngrManager::HandleMediaMtuSize: change MTU Size: " << dMtuSize;

        eIpType type = m_pMngmntIpParams_asIpService_fromProcess->GetIpType();
        if (dMtuSize < 1280 && (eIpType_IpV6 == type || eIpType_Both == type))
        {
            TRACEINTO << "CMcuMngrManager::HandleMediaMtuSize: IPV6 require MTU Size not less than 1280. MTUSize: " << dMtuSize;
		    return STATUS_FAIL;
        }
    }

    return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::HandleSetCfg(CRequest *pRequest)
{
	pRequest->SetConfirmObject(new CDummyEntry);


	if (pRequest->GetAuthorization() != SUPER)
	{
		TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::HandleSetCfg: No permission to set system.cfg " << pRequest->GetAuthorization();
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_OK;
	}

	CSysConfigSet *pSysConfigSet = (CSysConfigSet*)pRequest->GetRequestObject();

    CLargeString strError;
	//STATUS status = pRequest->GetStatus();
	STATUS status = pSysConfigSet->GetSystemCfg()->IsValidOrChanged(strError);//check if new params are valid and different from original params
	pRequest->SetStatus(status);

    // STATUS_OK_WARNING : CFG set is valid AND
    //                     it is different from the values that the system is working right now.

	if(STATUS_OK != status && STATUS_OK_WARNING != status)
	{
        pRequest->SetExDescription(strError.GetString());
        return STATUS_OK;
	}

	CSysConfigEma* sys_cfg = pSysConfigSet->GetSystemCfg();
	// VNGR-19442: PSO - RMX should prevent users from enabling
	// MULTIPLE_SERVICES when the RMX is configured for IPv6
	if (CheckMultipleServices(sys_cfg, pRequest) != STATUS_OK)
		return STATUS_OK;

	if ((CheckLanRedundancy(sys_cfg, pRequest) != STATUS_OK))
	{
		return STATUS_OK;
	}
	
	STATUS tmp = CheckUltraSecuredFlagAndSetOtherFlagsIfNeeded(pSysConfigSet->GetSystemCfg());
	if (tmp == STATUS_CFG_PARAM_CANT_REMOVE_ULTRA_SECURED_MODE)
	{
        pRequest->SetStatus(STATUS_CFG_PARAM_CANT_REMOVE_ULTRA_SECURED_MODE);
        return STATUS_OK;
	}

	//Set SEPARATE_MANAGEMENT_NETWORK flag and exit if needed
	if (SetSeparateManagementNetworkFlag(sys_cfg, sys_cfg, pRequest) != STATUS_OK)
		return STATUS_OK;

	std::string strPcmData = "";
	    STATUS pcm_changed = pSysConfigSet->GetSystemCfg()->CheckIfKeyChanged(CFG_KEY_NUM_OF_PCM_IN_MPMX, strPcmData);
	    TRACEINTO << "CMcuMngrManager::HandleSetCfg pcm_changed status=" << pcm_changed;
	    if(STATUS_OK == pcm_changed)
	    {
	    	TRACEINTO << "CMcuMngrManager::HandleSetCfg pcm_changed status ok";
	    	CLicensing* pLicense = m_pProcess->GetLicensing();
	    	bool isMpmx = pLicense->GetIsMPMXBitEnabled();
	    	if (isMpmx == false)
	    	{
	    		pRequest->SetStatus(STATUS_CFG_PARAM_NUM_OF_PCM_APPLICABLE_MPMX_ONLY);
	    		return STATUS_OK;
	    	}

	    	if(m_pLicensing->GetIsSvcEnabled())
	    	{
	    		if(strPcmData != "0" && strPcmData != "1" )
	    		{
	    			CCfgData * pcmData = pSysConfigSet->GetSystemCfg()->GetCfgEntryByKey(CFG_KEY_NUM_OF_PCM_IN_MPMX);
	    			pcmData->SetData("1");
	    			pRequest->SetStatus(STATUS_VALUE_OUT_OF_RANGE);
	    			//return STATUS_OK;
	    		}

	    	}


	    }

    //MTU Size
    if (STATUS_FAIL == HandleMediaMtuSize(sys_cfg))
    {
        pRequest->SetStatus(STATUS_IPV6_MTU_SIZE_MUST_NO_LESS_THAN_1280);
        TRACEINTO << "CMcuMngrManager::HandleSetCfg System Flag MEDIA_NIC_MTU_SIZE must no less then 1280, return.";
        return STATUS_OK;
    }
    
	eCfgParamType fileType = pSysConfigSet->GetFileType();
	CSysConfigEma* pSysConfigEMA = pSysConfigSet->GetSystemCfg();
	std::string level;
    if (m_bCSLogStarted==TRUE)
    	level = "5";
    else
    	level = "0";
	pSysConfigEMA->AddParamInFileNonVisible("CS_MODULE_PARAMETERS","trace1level","all.StackMsgs.logfileDebugLevel."+ level,eCfgParamUser,ONE_LINE_BUFFER_LENGTH,eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataString);
	pSysConfigEMA->AddParamInFileNonVisible("CS_MODULE_PARAMETERS","trace2level","all.XmlTrace.logfileDebugLevel."+level,eCfgParamUser,ONE_LINE_BUFFER_LENGTH,eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataString);
	pSysConfigEMA->AddParamInFileNonVisible("CS_MODULE_PARAMETERS","trace3level","siptask.SipMsgsTrace.logfileDebugLevel."+level,eCfgParamUser,ONE_LINE_BUFFER_LENGTH,eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataString);

	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	if (eProductTypeGesher != curProductType && eProductTypeNinja != curProductType)
	{
		pSysConfigEMA->SaveToFile(fileType);
	}

	//for debug purposes (write flags received from EMA to file)
//	std::ofstream outputFile;
//	outputFile.open("Cfg/DEBUG_SystemCfgVisible.txt");
//	outputFile << "HandleSetCfg" << endl;
//	CSysMap::iterator iT = pSysConfigSet->GetSystemCfg()->GetMap()->begin();
//	while (iT != pSysConfigSet->GetSystemCfg()->GetMap()->end())
//	{
//		CCfgData* cfgData = (CCfgData*)iT->second;
//		outputFile << cfgData->GetKey() << "|" << cfgData->GetData() << endl;
//		iT++;
//	}
//	outputFile.close();

    if(STATUS_OK == status)
    {
        RemoveActiveAlarmByErrorCode(CFG_CHANGED);
    }
    else if(STATUS_OK_WARNING == status)
	{
    	//check if JITC_MODE or SEPARATE_NETWORK keys changed
    	std::string strDebugModeData = "";
    	std::string strSeparatedNetworkData = "";
    	std::string strV35JITCSupportData = "";
    	std::string strMultipleServicesData = "";
    	std::string strJITCdata = "";
    	std::string strLanRedundancyData = CFG_LANREDUNDANCY_DEFAULT;

    	STATUS separatedNet_changed, v35_JITC_SupportChanged, multipleServices_changed, lanRedundancy_changed;

    	STATUS jitc_changed = pSysConfigSet->GetSystemCfg()->CheckIfKeyChanged(CFG_KEY_JITC_MODE, strJITCdata);
    	TRACEINTO << "CMcuMngrManager::HandleSetCfg jitc status=" << jitc_changed;

    	separatedNet_changed = pSysConfigSet->GetSystemCfg()->CheckIfKeyChanged(CFG_KEY_SEPARATE_NETWORK, strSeparatedNetworkData);
    	TRACEINTO << "CMcuMngrManager::HandleSetCfg network separtion status=" << separatedNet_changed;

    	v35_JITC_SupportChanged = pSysConfigSet->GetSystemCfg()->CheckIfKeyChanged(CFG_KEY_V35_ULTRA_SECURED_SUPPORT, strV35JITCSupportData);
    	TRACEINTO << "CMcuMngrManager::HandleSetCfg v35 jitc support status=" << v35_JITC_SupportChanged;

    	multipleServices_changed = pSysConfigSet->GetSystemCfg()->CheckIfKeyChanged(CFG_KEY_MULTIPLE_SERVICES, strMultipleServicesData);
    	TRACEINTO << "CMcuMngrManager::HandleSetCfg multiple service status=" << multipleServices_changed;

    	lanRedundancy_changed = pSysConfigSet->GetSystemCfg()->CheckIfKeyChanged(CFG_KEY_LAN_REDUNDANCY, strLanRedundancyData);
    	TRACEINTO << "CMcuMngrManager::HandleSetCfg LAN redundancy status=" << strLanRedundancyData;

    	//for gesher/ninja, seperate network and multiple service and Lan redundancy are conflict.
    	if (eProductTypeGesher == curProductType || eProductTypeNinja == curProductType)
    	{
    		if ("YES" == strSeparatedNetworkData && "YES" == strMultipleServicesData)
    		{
    			pRequest->SetStatus(STATUS_CFG_PARAM_MISSMATCH_NETWORK_SEPARATION_AND_MULTIPLE_SERVICES);
    			return STATUS_OK;
    		}
    		if ("YES" == strLanRedundancyData && "YES" == strMultipleServicesData)
    		{
    			pRequest->SetStatus(STATUS_CFG_PARAM_MISSMATCH_LAN_REDUNDANCY_AND_MULTIPLE_SERVICES);
    			return STATUS_OK;
    		}
    		if ("YES" == strLanRedundancyData && "YES" == strSeparatedNetworkData)
    		{
    			pRequest->SetStatus(STATUS_CFG_PARAM_MISSMATCH_LAN_REDUNDANCY_AND_NETWORK_SEPARATION);
    			return STATUS_OK;
    		}
    	}

    	if("YES" == strJITCdata && "YES" == strSeparatedNetworkData && "YES" == strMultipleServicesData)
    		pRequest->SetStatus(STATUS_CFG_PARAM_MISSMATCH_NETWORK_SEPARATION_AND_MULTIPLE_SERVICES);
    	else if("YES" == strMultipleServicesData && "YES" == strV35JITCSupportData)
    		pRequest->SetStatus(STATUS_CFG_PARAM_MISSMATCH_MULTIPLE_SERVICES_AND_V35_ULTRA_SECURED);
    	else
    	{

			//Send new values to Switch and MFAs
			if(STATUS_OK == jitc_changed || STATUS_OK == separatedNet_changed)
			{
				status = pSysConfigSet->GetSystemCfg()->CheckIfKeyChanged(CFG_KEY_DEBUG_MODE, strDebugModeData);
				if(STATUS_OK != status)
					strDebugModeData = CFG_STR_NO;

				SendNewSysCfgParamsToCards(strDebugModeData,
                                           strJITCdata,
                                           strSeparatedNetworkData,
                                           strMultipleServicesData);
			}

			AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT,
									CFG_CHANGED,
									MAJOR_ERROR_LEVEL,
									"System configuration flags were modified. Reset the MCU",
									true,
									true
									);
    	}
	}

	if (eProductTypeGesher == curProductType || eProductTypeNinja == curProductType)
	{
		pSysConfigEMA->SaveToFile(fileType);
	}
	return STATUS_OK;
}



//check SEPARATE_MANAGEMENT_NETWORK and 35_ULTRA_SECURED_SUPPORT and change value of SEPARATE_MANAGEMENT_NETWORK if needed
STATUS CMcuMngrManager::SetSeparateManagementNetworkFlag(CSysConfigEma* cfgToUpdate, CSysConfigEma* cfgToCheck, CRequest* pRequest)
{
	// VNGR-19806: V35>>SEPARATE_MANAGEMENT_NETWORK=NO should not be allowed when V35_ULTRA_SECURED_SUPPORT == YES
	eProductType prodType = m_pProcess->GetProductType();
	if (eProductTypeRMX2000 == prodType || eProductTypeSoftMCU == prodType || m_mcuMngrProductType==eProductTypeCallGeneratorSoftMCU
	|| eProductTypeGesher == prodType || eProductTypeNinja == prodType || eProductTypeEdgeAxis == prodType)
	{
		//if V35_ULTRA_SECURED_SUPPORT was set to YES then set SEPARATE_MANAGEMENT_NETWORK to YES
		std::string answer2;
		STATUS changed2 = cfgToCheck->CheckIfKeyChanged(CFG_KEY_V35_ULTRA_SECURED_SUPPORT, answer2);
		if (STATUS_OK == changed2 && 0 == answer2.compare(CFG_STR_YES))
		{
			CCfgData* sep = cfgToUpdate->GetCfgEntryByKey(CFG_KEY_SEPARATE_NETWORK);
			if (NULL != sep && 0 == sep->GetData().compare(CFG_STR_NO))
			{
				if (cfgToUpdate->IsParamExist(CFG_KEY_SEPARATE_NETWORK))
				{
					CCfgData* cfgDataToUpdate = cfgToUpdate->GetCfgEntryByKey(CFG_KEY_SEPARATE_NETWORK);
					cfgDataToUpdate->SetData(CFG_STR_YES);
				}
			}
		}
	    //if SEPARATE_MANAGEMENT_NETWORK and V35_ULTRA_SECURED_SUPPORT were YES and SEPARATE_MANAGEMENT_NETWORK was set to NO then return error
	    changed2 = cfgToCheck->CheckIfKeyChanged(CFG_KEY_SEPARATE_NETWORK, answer2);
	    if (STATUS_OK == changed2 && 0 == answer2.compare(CFG_STR_NO))
	    {
	        const CCfgData* v35 = cfgToCheck->GetCfgEntryByKey(CFG_KEY_V35_ULTRA_SECURED_SUPPORT);
	        if (NULL != v35 && 0 == v35->GetData().compare(CFG_STR_YES))
	        {
	        	pRequest->SetStatus(STATUS_CFG_PARAM_UNSUPPORTED_SEPARATE_MANAGEMENT_V35);
	        	return STATUS_CFG_PARAM_UNSUPPORTED_SEPARATE_MANAGEMENT_V35;
	        }
	    }
	}
	return STATUS_OK;

}


STATUS CMcuMngrManager::CheckMultipleServices(CSysConfigEma* cfgEMA, CRequest* pRequest)
{
	std::string answer;
	STATUS changed = cfgEMA->CheckIfKeyChanged(CFG_KEY_MULTIPLE_SERVICES, answer);
	if (STATUS_OK == changed && 0 == answer.compare(CFG_STR_YES))
	{
		eIpType type = m_pMngmntIpParams_asIpService_fromProcess->GetIpType();
		if (eIpType_IpV6 == type || eIpType_Both == type)
		{
			pRequest->SetStatus(STATUS_CFG_PARAM_UNSUPPORTED_MULTIPLE_SERVICE_IPV6);
			return STATUS_CFG_PARAM_UNSUPPORTED_MULTIPLE_SERVICE_IPV6;
		}
		if ( FALSE == m_pLicensing->GetIsMultipleServicesEnabled() )
		{
			pRequest->SetStatus(STATUS_CFG_PARAM_UNSUPPORTED_MULTIPLE_SERVICE_LICENSE);
			pRequest->SetExDescription("Since license, can't support Multiple Services");
			return STATUS_CFG_PARAM_UNSUPPORTED_MULTIPLE_SERVICE_LICENSE;
		}
	}
	return STATUS_OK;
}

STATUS CMcuMngrManager::CheckLanRedundancy(CSysConfigEma* sys_cfg, CRequest* pRequest)
{
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	if ((curProductType == eProductTypeGesher) || (curProductType == eProductTypeNinja))
	{
		std::string answer;
		STATUS changed = sys_cfg->CheckIfKeyChanged(CFG_KEY_LAN_REDUNDANCY, answer);
		if (STATUS_OK == changed && 0 == answer.compare(CFG_STR_YES))
		{
			eIpType type = m_pMngmntIpParams_asIpService_fromProcess->GetIpType();
			if (eIpType_IpV6 == type || eIpType_Both == type)
			{
				pRequest->SetStatus(STATUS_CFG_PARAM_UNSUPPORTED_LAN_REDUNDANCY_IPV6);
				return STATUS_CFG_PARAM_UNSUPPORTED_LAN_REDUNDANCY_IPV6;
			}
		}
	}
	return STATUS_OK;
}



//
// Get All V4, V6 ip addresses and their relevant information from service,
// and store them in the struct
//
/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::RetrieveIpAddresses(CIPService* pService, IP_ADDR_S & ipAddrS)
{
	if(!pService)
	{
		return;
	}
	CIPSpan* pTmpSpan0 = pService->GetSpanByIdx(0); // Ctrl
	CIPSpan* pTmpSpan1 = pService->GetSpanByIdx(1); // Shelf
	if(pTmpSpan0)
	{
		ipAddrS.iPv4 = pTmpSpan0->GetIPv4Address();
		pTmpSpan0->GetIPv6Address(0, ipAddrS.iPv6_str);
		pTmpSpan0->GetIPv6Address(0, ipAddrS.iPv6_NoBrackets, FALSE);
		pTmpSpan0->GetIPv6SubnetMaskStr(0, ipAddrS.iPv6_netMask);
	}
	SystemDWORDToIpString(ipAddrS.iPv4,	ipAddrS.iPv4_str);

	if(pTmpSpan1)
	{
		ipAddrS.iPv4_shelf = pTmpSpan1->GetIPv4Address();
		pTmpSpan1->GetIPv6Address(0, ipAddrS.iPv6_shelf);
	}

	ipAddrS.iPv4_defGW = pService->GetDefaultGatewayIPv4();
	SystemDWORDToIpString(pService->GetNetMask(), ipAddrS.iPv4_netMask);

	pService->GetDefaultGatewayIPv6(ipAddrS.iPv6_defGW, FALSE);
	ipAddrS.ipType = pService->GetIpType();

	// eV6Configuration_DhcpV6 is not supported!
	//   if it's retrieved, replace it with 'auto'
	if (eV6Configuration_DhcpV6 == pService->GetIpV6ConfigurationType())
	{
		PASSERTMSG(TRUE, "DhcpV6 is not supported as IPv6 configuration type");
		pService->SetIpV6ConfigurationType(eV6Configuration_Auto);
	}
	ipAddrS.ipV6configType = pService->GetIpV6ConfigurationType();
}

/////////////////////////////////////////////////////////////////////////////
bool CMcuMngrManager::WasSecuredTheOnlyChange(const CIPService* service)
{
	//Check if this is the only change in the management
	CIPService* tmpService = new CIPService(*service);
	tmpService->SetIsSecured(m_pMngmntIpParams_asIpService_fromProcess->GetIsSecured());
	//tmpService->SetManagementSecurity(*m_pMngmntIpParams_asIpService_fromProcess->GetManagementSecurity());
	COstrStream  answer;
	tmpService->Dump(answer);

	tmpService->RemoveZeroSpansFromService();

	CIPService* tmpServicePrevious = new CIPService(*m_pMngmntIpParams_asIpService_fromProcess);
	tmpServicePrevious->RemoveZeroSpansFromService();

	if (*tmpService != *tmpServicePrevious)
	{
		POBJDELETE(tmpService);
		POBJDELETE(tmpServicePrevious);
		return false;
	}

	TRACEINTO << "CMcuMngrManager::WasSecuredTheOnlyChange - only secured was changed";
	POBJDELETE(tmpService);
	POBJDELETE(tmpServicePrevious);
	return true;
}
/////////////////////////////////////////////////////////////////////////////
bool  CMcuMngrManager::WasWhiteListOnlyChange(const CIPService* service)
{
	//Check if this is the only change in the management
		CIPService* tmpService = new CIPService(*service);
		tmpService->SetWhiteList(m_pMngmntIpParams_asIpService_fromProcess->GetWhiteList());

		if (!m_pMngmntIpParams_asIpService_fromProcess->compareManagment(*tmpService))
		{
			TRACEINTO << "CMcuMngrManager::WasWhiteListOnlyChange - the change is more then just whitelist";
			POBJDELETE(tmpService);
			return false;
		}

		TRACEINTO << "CMcuMngrManager::WasWhiteListOnlyChange - only whitelist was changed";
		POBJDELETE(tmpService);
		return true;
}

/////////////////////////////////////////////////////////////////////////////
bool CMcuMngrManager::WasServiceChanged(const CIPService* service)
{
	bool wasChanged = false;

	//if (*service == *m_pMngmntIpParams_asIpService_fromProcess)
	if(m_pMngmntIpParams_asIpService_fromProcess->compareManagment(*service))
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::WasServiceChanged - ipservice are equal";
	else
	{
		wasChanged = true;
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::WasServiceChanged - ipservice are NOT equal";
	}

	return wasChanged;
}
/////////////////////////////////////////////////////////////////////////////

WhiteListStatus CMcuMngrManager::HandleWhiteList(CIPService *pService,STATUS& ref)
{

	WhiteListStatus res = WHITELIST_NO_CHANGE;
	if(*m_pMngmntIpParams_asIpService_fromProcess->GetWhiteList() == *pService->GetWhiteList())
		return res;
	CConfigManagerApi api;
	if(pService->GetWhiteList()->IsWhiteListEnable())
	{
		if(pService->GetWhiteList()->IsWhiteListValid())
		{
			CSegment  *pIpv4Seg = new CSegment;
			pService->GetWhiteList()->WriteWhiteListToSegment(pIpv4Seg);
			ref = api.EnableWhiteList(pIpv4Seg);
		}
		else
		{
			res = WHITELIST_FAILED;
			ref = STATUS_IP_ADDRESS_NOT_VALID;
		}
	}
	else
	{
		ref = api.DisableWhiteList();
	}
	bool whiteListOnlyChange = WasWhiteListOnlyChange(pService);

	if(ref == STATUS_OK)
	{
		if(whiteListOnlyChange)
			res = WHITELIST_CHANGE_NORESET;
		else
			res = WHITELIST_CHANGE;
	}
	else
		res = WHITELIST_FAILED;

	return res;
}
////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendWhiteListChangeAuditEvent(CIPService *pService)
{
	   string data = "Management service: WhiteList was changed";

	    CXMLDOMElement *serviceBeforeXml = NULL;
	    m_pMngmntIpParams_asIpService_fromProcess->SerializeXml(serviceBeforeXml);
	    char *buffBefore = NULL;
	    serviceBeforeXml->DumpDataAsLongStringEx(&buffBefore, TRUE);

	    CXMLDOMElement *serviceAfterXml = NULL;
	    pService->SerializeXml(serviceAfterXml);
	    char *buffAfter = NULL;
	    serviceAfterXml->DumpDataAsLongStringEx(&buffAfter, TRUE);

	    AUDIT_EVENT_HEADER_S outAuditHdr;
	    CAuditorApi::PrepareAuditHeader(outAuditHdr,
	                                    "",
	                                    eMcms,
	                                    "",
	                                    "",
	                                    eAuditEventTypeInternal,
	                                    eAuditEventStatusOk,
	                                    "IP Management Service modified",
	                                    "WhiteList was modified.",
	                                    "",
	                                    data);
	    CFreeData freeData;
	    CAuditorApi::PrepareFreeData(freeData,
	                                 "Previous Parameters",
	                                 eFreeDataTypeXml,
	                                 buffBefore,
	                                 "Updated Parameters",
	                                 eFreeDataTypeXml,
	                                 buffAfter);

	    CAuditorApi api;
	    api.SendEventMcms(outAuditHdr, freeData);

	    delete [] buffBefore;
	    delete [] buffAfter;
	    PDELETE(serviceBeforeXml);
	    PDELETE(serviceAfterXml);
}
///////////////////////////////////////////////////////////////////////////
bool CMcuMngrManager::IsWhiteListEnabled()
{
	BOOL isWhiteList = NO;

    CSysConfig *pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
    if (pSysConfig)
    {
	    pSysConfig->GetBOOLDataByKey(CFG_ENABLE_WHITE_LIST, isWhiteList);
    }

    return isWhiteList;
}

///////////////////////////////////////////////////////////////////////////
bool CMcuMngrManager::IsOneCertificate()
{


    CSysConfig *pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
    if (pSysConfig)
    {
    	string certificateString = "";
    	BOOL isOneCertificate = NO;

    	isOneCertificate = pSysConfig->GetDataByKey("802_1X_CERTIFICATE_MODE", certificateString);

    	 if (isOneCertificate)
    	 {
    		 if(certificateString.compare("ONE_CERTIFICATE")==0)
    			 return true;
    		 else
    			 return false;
    	 }
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////
bool CMcuMngrManager::IsSkipCertificateValidation()
{
	BOOL isSkipCertificateValidation = NO;

    CSysConfig *pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
    if (pSysConfig)
    {
	    pSysConfig->GetBOOLDataByKey("802_1X_SKIP_CERTIFICATE_VALIDATION", isSkipCertificateValidation);
    }

    return isSkipCertificateValidation;
}

//////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SetMFWMngNetworkExDiscription(STATUS nStatus, CRequest *pRequest, CIPService *pIpService)
{
	CIPSpan* pIpSpan = pIpService->GetFirstSpan();
	string strExtDiscription = "The configuration of the Management service is invalid. Interface " + pIpSpan->GetInterface();

	switch(nStatus)
	{
	case STATUS_IP_ADDRESS_MISMATCHES_SYSTEM:
		strExtDiscription += " with address IpV4:" + m_pProcess->GetMFWValidatedResult() + " does not exist or is invalid.";
		break;
	case STATUS_MUST_ASSIGN_INTERFACE_IN_MFW:
		strExtDiscription += " does not exist or is invalid.";
		break;
	case STATUS_IPV6_GLOBAL_ADDRESS_MISMATCHES_SYSTEM:
	case STATUS_IPV6_SITE_ADDRESS_MISMATCHES_SYSTEM:
	case STATUS_IPV6_LINK_ADDRESS_MISMATCHES_SYSTEM:
		strExtDiscription += "with address Ipv6: " + m_pProcess->GetMFWValidatedResult() + " does not exist or is invalid.";
		break;
	default:
		strExtDiscription = "";
		break;
	}
	pRequest->SetExDescription(strExtDiscription.c_str());
}

/////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::HandleSetMngmntNetwork(CRequest *pRequest)
{
    TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__;
    bool isMngmntIpChanged = false;

    pRequest->SetConfirmObject(new CDummyEntry);

	if(pRequest->GetAuthorization() != SUPER)
	{
		FPTRACE(eLevelInfoNormal,"CMcuMngrManager::HandleSetMngmntNetwork: No permission to update management service");
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_OK;
	}

	m_isUserChangedManagementIp = YES;
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

	STATUS retStatus = STATUS_OK;
	STATUS warningStatus = STATUS_OK;

	// ===== 1. get the service from the request
	CIPService* pUpdatedIpService = (CIPService*)(pRequest->GetRequestObject());
	if(!pUpdatedIpService)
	{
		retStatus = STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS;
		DBGPASSERT(1);
	}
	else
	{
		// check SoftMCU values

		if (eProductTypeSoftMCUMfw == curProductType || curProductType == eProductTypeSoftMCU || curProductType == eProductTypeEdgeAxis || m_mcuMngrProductType==eProductTypeCallGeneratorSoftMCU)
		{
			retStatus = m_pProcess->ValidateSwMcuFields(pUpdatedIpService);	//check that only fields that CAN be updated in SMCU were changed

			if (retStatus != STATUS_OK)
			{
				pRequest->SetStatus(retStatus);
				return STATUS_OK;
			}
		}
		if ( eProductTypeSoftMCUMfw == curProductType )
		{
			retStatus = m_pProcess->ValidateMFWFields(pUpdatedIpService);
			if(retStatus != STATUS_OK)
			{
				pRequest->SetStatus(retStatus);
				SetMFWMngNetworkExDiscription(retStatus, pRequest, pUpdatedIpService);
				return STATUS_OK;
			}
		}

		// Check if service was changed
		bool wasChanged = WasServiceChanged(pUpdatedIpService);
		if(!wasChanged)
		{
			TRACEINTO << "\n" << __FUNCTION__ << " - Service wasn't changed";
			pRequest->SetStatus(retStatus);
			return STATUS_OK;
		}
		if (IsFederalOn()==TRUE)
			pUpdatedIpService->SetIsPermanentNetworkOpen(FALSE);
		else
			pUpdatedIpService->SetIsPermanentNetworkOpen(TRUE);

		if(IsWhiteListEnabled())
		{
			WhiteListStatus statusWhiteList = HandleWhiteList(pUpdatedIpService,retStatus);
			switch(statusWhiteList)
			{
				case WHITELIST_NO_CHANGE:
					TRACEINTO << "\n" << __FUNCTION__ << " -WhiteList wasn't changed";
					break;
				case WHITELIST_CHANGE:
					SendWhiteListChangeAuditEvent(pUpdatedIpService);
					TRACEINTO << "\n" << __FUNCTION__ << " -WhiteList was changed reset is needed";
					break;
				case WHITELIST_CHANGE_NORESET:
					SendWhiteListChangeAuditEvent(pUpdatedIpService);
					retStatus = UpdateNetworkInterface("HandleSetMngmntNetwork", *pUpdatedIpService, eMngmntIpFromEma);
					TRACEINTO << "\n" << __FUNCTION__ << " -WhiteList was only changed reset is not needed";
					pRequest->SetStatus(retStatus);
					return STATUS_OK;
					break;
				case WHITELIST_FAILED:
					TRACEINTO << "\n" << __FUNCTION__ << " -Failed to update WhiteList status-"<<CProcessBase::GetProcess()->GetStatusAsString(retStatus);
					pRequest->SetStatus(retStatus);
					return STATUS_OK;
			}
		}

		// in JITC mode, check that security was not removed
		BOOL originalIsSecured	= m_pMngmntIpParams_asIpService_fromProcess->GetIsSecured(),
			 newIsSecured		= pUpdatedIpService->GetIsSecured();

		if ( (originalIsSecured) && (!newIsSecured) && IsFederalOn() )
		{
			TRACEINTO << "\nCMcuMngrManager::HandleSetMngmntNetwork - cannot disable 'Secured' in JITC mode";
			pRequest->SetStatus(STATUS_FAIL);
			pRequest->SetExDescription("WEB server must be secured in ULTRA SECURE mode");
			return STATUS_OK;
		}

		BOOL bSecuredChanged = FALSE;

		if ( originalIsSecured != newIsSecured )
		{
			TRACEINTO << "\nCMcuMngrManager::HandleSetMngmntNetwork - secured value was set";
			bSecuredChanged = TRUE;
		}

		eIpType newIpType = pUpdatedIpService->GetIpType();
		if ((eIpType_IpV6 == newIpType || eIpType_Both == newIpType) &&
			IsSystemFlagTrue(CFG_KEY_MULTIPLE_SERVICES))
		{
			TRACEINTO << "\nCMcuMngrManager::HandleSetMngmntNetwork - IPv6 cannot be enabled when MULTIPLE_SERVICE system flag is YES ";
			pRequest->SetStatus(STATUS_FAIL);
			pRequest->SetExDescription("IPv6 cannot be enabled when MULTIPLE_SERVICE system flag is YES");
			return STATUS_OK;
		}

		if ((eIpType_IpV6 == newIpType || eIpType_Both == newIpType) &&
			IsSystemFlagTrue(CFG_KEY_LAN_REDUNDANCY))
		{
			TRACEINTO << "\nCMcuMngrManager::HandleSetMngmntNetwork - IPv6 cannot be enabled when CFG_KEY_LAN_REDUNDANCY system flag is YES ";
			pRequest->SetStatus(STATUS_FAIL);
			pRequest->SetExDescription("IPv6 cannot be enabled when CFG_KEY_LAN_REDUNDANCY system flag is YES");
			return STATUS_OK;
		}

		/*BRIDGE-3645 - this block is not needed also mutilple service in ipv6 past issue was fixed
		 * if (CheckIfDefaultGWIPV6IsValid(pUpdatedIpService) == FALSE &&
		    eIpType_IpV4 != pUpdatedIpService->GetIpType())
		{
			pRequest->SetStatus(STATUS_IPV6_ALREADY_DEFINED_DIFFERENTLY_IN_ANOTHER_SERVICE);
			return STATUS_OK;
		}*/

		/***********************************************************************************
		 * 4.11.08:  Management service changes should NOT be online					   *
		 * 			- So no need to ask confParty for conferences status (Hillel).		   *
		 ***********************************************************************************/

//	    OPCODE respondeOpcode = 0;
//	    CSegment pRspMsg;
//
//	    CManagerApi confPartyMngrApi(eProcessConfParty);
//	    STATUS status = confPartyMngrApi.SendMessageSync(NULL, GET_CONF_NUM_REQ, 1 * SECOND,
//	                                                     respondeOpcode, pRspMsg);
//	    if(STATUS_OK == status && GET_CONF_NUM_IND == respondeOpcode)
//	    {
//	        DWORD confNum = 0;
//	        pRspMsg >> confNum;
//	        if(0 < confNum)
//	        {
//	            // reject this request
//	            FPTRACE(eLevelInfoNormal,"CMcuMngrManager::HandleSetMngmntNetwork: can't update management service since there are conferences");
//	            pRequest->SetStatus(OPERATION_BLOCKED);
//	            pRequest->SetExDescription("Can't change Management service while there are conferences");
//	            return STATUS_OK;
//	        }
//	    }

		IP_ADDR_S ipAddrS;
		// retrieve ip addresses for IpV6, IpV4
		RetrieveIpAddresses(pUpdatedIpService, ipAddrS);

		// ===== 2. check if there are valid pre-defined certificate
		// ===== 3. check if the dns has been changed in secured mode
		retStatus = CheckSecurity(pUpdatedIpService);

		if (STATUS_OK == retStatus)
		{
			retStatus = ValidateDnsValues(pUpdatedIpService, ipAddrS,warningStatus);
			if (warningStatus!=STATUS_OK)
			{
				retStatus = STATUS_OK_WARNING;
			}
			if ( (STATUS_OK == retStatus || STATUS_OK_WARNING == retStatus )&& YES == IsTarget())
				// ===== 4. check IP duplication (Cntrl and Shelf)
				retStatus = CheckDuplicateIpCntrlAndShelf(ipAddrS);

			if (STATUS_OK == retStatus || STATUS_OK_WARNING == retStatus)
			{
				// ===== 5. validate ip address (with netMask, and locality)
				CIpServiceValidator serviceValidator(*pUpdatedIpService);
				CLargeString errorMsg;
				retStatus = serviceValidator.ValidateFullMNGMNT(errorMsg);
				if (STATUS_OK != retStatus)
				    TRACEINTOFUNC << "IP address mismatches netMask";
			}
		}

		if (STATUS_OK == retStatus || STATUS_OK_WARNING == retStatus)
	    {

			bool wasSecuredTheOnlyChange = false;

			if (bSecuredChanged)
				wasSecuredTheOnlyChange = WasSecuredTheOnlyChange(pUpdatedIpService);

			eIpType oldIpType						= m_pMngmntIpParams_asIpService_fromProcess->GetIpType(); // for future comparing
			eV6ConfigurationType oldV6_configType	= m_pMngmntIpParams_asIpService_fromProcess->GetIpV6ConfigurationType();

			//Added by flora: Check the duplicate ip between Mngr Ip & IP Network service
			if ((eProductTypeNinja == curProductType) && ( eIpType_IpV4 == pUpdatedIpService->GetIpType()))
			{
				retStatus = CheckDuplicateIpCntrlAndOther(ipAddrS);
				if (STATUS_OK != retStatus)
				{
					TRACEINTO << "\nCMcuMngrManager::HandleSetMngmntNetwork - Check Duplicate Ip within IfConfig IP Lists Failed! ";
					pRequest->SetStatus(retStatus);
					pRequest->SetExDescription(m_pProcess->GetStatusAsString(retStatus).c_str());
					return STATUS_OK;
				}

			}
		
			// ===== 6. set updated MngmntNetwork params
			retStatus = UpdateNetworkInterface("HandleSetMngmntNetwork", *pUpdatedIpService, eMngmntIpFromEma);

			// ===== 7. in case IpType (IPv4/IPv6) was changed: update CS and produce an Alert
			if (STATUS_OK == retStatus || STATUS_OK_WARNING == retStatus)
			{
				eIpType newIpType						= m_pMngmntIpParams_asIpService_fromProcess->GetIpType();
				eV6ConfigurationType newV6_configType	= m_pMngmntIpParams_asIpService_fromProcess->GetIpV6ConfigurationType();



				if (wasSecuredTheOnlyChange == false)
				{

					AddActiveAlarmSingleton( FAULT_GENERAL_SUBJECT,
										SYSTEM_CONFIGURATION_CHANGED,
										 MAJOR_ERROR_LEVEL,
										 "System configuration changed. Please reset the MCU.",
										 true,
										 true
										);

					retStatus = STATUS_OK_WARNING; // for producing a 'Please Reset' popup by EMA
				}
			} // end ipConfig succeeded

			//VNGR-18955 - Judith: in case a reset is require, don't change the RMX to secure, until reset occurs.
			if (bSecuredChanged && (retStatus != STATUS_OK_WARNING))	//if secured was changed - reset apache
			{
				ConfigApache(*pUpdatedIpService);
				// update another processes with new security mode
				SendSecurityMode(eProcessAuthentication);
				if (eProductTypeSoftMCUMfw!=curProductType)
					SendSecurityMode(eProcessFailover);
			}

		} // STATUS_OK == retStatus
	} // (end service from request is ok)
	if (eProductTypeSoftMCUMfw!=curProductType)
		SendSecurityPKIToDependedProcesses();

	pRequest->SetExDescription(m_pProcess->GetStatusAsString(warningStatus).c_str());
	pRequest->SetStatus(retStatus);
    TRACEINTOFUNC << "Finished with "
                  << m_pProcess->GetStatusAsString(retStatus)
                  << "(" << retStatus << ")";

	return STATUS_OK;
}

STATUS CMcuMngrManager::ValidateDnsValues(CIPService* pService,
                                          const IP_ADDR_S& ipAddrS,STATUS& warningStatus)
{
    PASSERT_AND_RETURN_VALUE(NULL == pService, STATUS_FAIL);
    STATUS status = STATUS_OK;
	CIpDns* pDns = pService->GetpDns();
	PASSERT_AND_RETURN_VALUE(NULL == pDns, STATUS_FAIL);


	if (eServerStatusOff == pDns->GetStatus())
	{
	    // IPv6 requires configured DNS
	    if (eIpType_IpV6 == ipAddrS.ipType)
	    {
	    	warningStatus = STATUS_IPV6_FORCE_DNS;
	    	status =  STATUS_OK_WARNING;
	    }
	}


	DWORD ipv4_0 = pDns->GetIPv4Address(0);
	DWORD ipv4_1 = pDns->GetIPv4Address(1);
	DWORD ipv4_2 = pDns->GetIPv4Address(2);

	char ipv6_0[IPV6_ADDRESS_LEN];
	char ipv6_1[IPV6_ADDRESS_LEN];
	char ipv6_2[IPV6_ADDRESS_LEN];
	memset(ipv6_0, '\0', sizeof(ipv6_0));
	memset(ipv6_1, '\0', sizeof(ipv6_1));
	memset(ipv6_2, '\0', sizeof(ipv6_2));
	pDns->GetIPv6Address(0, ipv6_0, 0);
	pDns->GetIPv6Address(0, ipv6_1, 0);
	pDns->GetIPv6Address(0, ipv6_2, 0);

	if (eIpType_IpV6 == ipAddrS.ipType)//2
	{
		// if one of the ipv4 addresses were set (so it will be different from 0)
		if (ipv4_0 | ipv4_1 | ipv4_2)
		{
			status = STATUS_IP_SERVICE_DNS_TYPE_DNS;
		}
	}
	else if (eIpType_IpV4 == ipAddrS.ipType) //1
	{
		// if one of the ipv6 addresses were set
		if ((0 != strcmp("", ipv6_0) && 0 != strcmp(ipv6_0, "::")) ||
			(0 != strcmp("", ipv6_1) && 0 != strcmp(ipv6_1, "::")) ||
			(0 != strcmp("", ipv6_2) && 0 != strcmp(ipv6_2, "::")))
		{
			status = STATUS_IP_SERVICE_DNS_TYPE_DNS;
		}
	}

	if(eServerStatusOff != pDns->GetStatus() &&(ipv4_0 == 0) && (ipv4_1 == 0) && (ipv4_2 == 0) &&
	(0 == strcmp("", ipv6_0) || 0 == strcmp(ipv6_0, "::")) &&
	(0 == strcmp("", ipv6_1) || 0 == strcmp(ipv6_1, "::")) &&
	(0 == strcmp("", ipv6_2) || 0 == strcmp(ipv6_2, "::")))

	{

		status = STATUS_FAIL_TO_CONFIG_DNS;
	}

	char iPv4Address0_str[IP_ADDRESS_LEN],
		 iPv4Address1_str[IP_ADDRESS_LEN],
		 iPv4Address2_str[IP_ADDRESS_LEN];

	SystemDWORDToIpString(ipv4_0, iPv4Address0_str);
	SystemDWORDToIpString(ipv4_1, iPv4Address1_str);
	SystemDWORDToIpString(ipv4_2, iPv4Address2_str);

	TRACEINTOFUNC << "\nipType:  " << ::IpTypeToString(ipAddrS.ipType)
			      << "\nipv4(0): " << iPv4Address0_str << ", ipv6(0): " << ipv6_0
			      << "\nipv4(1): " << iPv4Address1_str << ", ipv6(1): " << ipv6_1
			      << "\nipv4(2): " << iPv4Address2_str << ", ipv6(2): " << ipv6_2;

	return status;
}

STATUS CMcuMngrManager::CheckSecurity(CIPService* pService)
{
  STATUS retStatus = STATUS_OK;
  BYTE oldIsSecured = m_pMngmntIpParams_asIpService_fromProcess->GetIsSecured();

  CManagementSecurity* sec = pService->GetManagementSecurity();
  PASSERT_AND_RETURN_VALUE(NULL == sec, STATUS_FAIL);
  bool ca_validation = sec->IsRequestPeerCertificate();
  std::string host_name = GetHostNameFromService(pService);

  BYTE revocation_method = sec->GetRevocationMethodType();

   if (pService->GetIsSecured() == TRUE)
   {
      retStatus = CheckForValidCertificate(host_name.c_str(), false, ca_validation,revocation_method);

      if (STATUS_CERTIFICATE_IS_GOING_TO_BE_EXPIRED == retStatus)
          retStatus = STATUS_OK;
   }
   else
   {

      RemoveTLSActiveAlarms();
   }

  return retStatus;
}

// Certificate validity tests, in case we start a secured connection
STATUS CMcuMngrManager::CheckForValidCertificate(const char* host_name,
                                                 bool add_aa,
                                                 bool ca_validation,
                                                 BYTE revocation_method)
{
    CSystemTime sysTime;
    m_pProcess->GetMcuTime(sysTime);

    const CStructTm* now = sysTime.GetMCUTime();
    PASSERTMSG_AND_RETURN_VALUE(NULL == now,
            "Unable to get MCU time", STATUS_FAIL);

    CSegment* seg = new CSegment;

    // Sends host name, system time and add active alarm flag
    *seg << host_name
         << (add_aa ? 1u : 0u)
         << (ca_validation ? 1u : 0u)
         << revocation_method;

    const_cast<CStructTm*>(now)->Serialize(NATIVE, *seg);

    OPCODE opcode;
    CSegment ret_seg;
    CManagerApi api(eProcessCertMngr);
    STATUS stat = api.SendMessageSync(seg,
                                      CERTMNGR_VERIFY_CERTIFICATE,
                                      5 * SECOND,
                                      opcode,
                                      ret_seg);

    PASSERTSTREAM_AND_RETURN_VALUE(stat != STATUS_OK,
        "Unable to send " << CERTMNGR_VERIFY_CERTIFICATE
            << " to " << ProcessNames[eProcessCertMngr]
            << ": " << m_pProcess->GetStatusAsString(stat),
            stat);

    if (opcode != STATUS_OK)
    {
        std::string buf;
        ret_seg >> buf;
        TRACEINTOFUNC << m_pProcess->GetStatusAsString(opcode) << ": " << buf;
    }

    return opcode;
}

void CMcuMngrManager::RemoveTLSActiveAlarms()
{
    CManagerApi api(eProcessCertMngr);
    STATUS stat = api.SendOpcodeMsg(CERTMNGR_REMOVE_ACTIVE_ALARM);

    PASSERTSTREAM(stat != STATUS_OK,
            "Unable to send " << CERTMNGR_REMOVE_ACTIVE_ALARM
            << " to " << ProcessNames[eProcessCertMngr]
            << ": " << m_pProcess->GetOpcodeAsString(stat));
}

BOOL CMcuMngrManager::IsSystemFlagTrue(const char* key) const
{
  PASSERT_AND_RETURN_VALUE(NULL == key, false);
  PASSERT_AND_RETURN_VALUE(NULL == m_pProcess, false);

  CSysConfig* cfg = m_pProcess->GetSysConfig();
  PASSERT_AND_RETURN_VALUE(NULL == cfg, false);

  BOOL val;
  BOOL res = cfg->GetBOOLDataByKey(key, val);
  PASSERTSTREAM_AND_RETURN_VALUE(!res,
      "CSysConfig::GetBOOLDataByKey:  " << key,
      false);

  return val;
}

STATUS CMcuMngrManager::CheckDuplicateIpCntrlAndShelf(const IP_ADDR_S& ipAddrS)
{
  STATUS stat;
  short failed = 0;
  CConfigManagerApi api;

  // Checks new Cntrl IpV4 address
  if (eIpType_IpV4 == ipAddrS.ipType || eIpType_Both == ipAddrS.ipType)
  {
    // Checks new Cntrl address vs. new Shelf address
    if (ipAddrS.iPv4 == ipAddrS.iPv4_shelf)
    {
      failed += 4;
      TRACEINTOFUNC << "New Cntrl IPv4 (" << ipAddrS.iPv4_str
                    << ") is similar to new Shelf address";
    }
    else
    {
      // Checks new Cntrl address duplication in general
      if (IsTarget() &&
          IsArpingNeeded(ipAddrS.iPv4_str) &&
          IsSystemFlagTrue(CFG_KEY_DUPLICATE_IP_DETECTION) )     {
        stat = api.ArpingRequest(ipAddrS.iPv4_str,ipAddrS.ipType );
        TRACEINTOFUNC << "New Cntrl IPv4 (" << ipAddrS.iPv4_str
                      << "): " << m_pProcess->GetStatusAsString(stat).c_str();
        if (stat)
          failed += 4;
      }
    }
  } // IPv4

  // Checks new Cntrl IpV6 address
  if (eIpType_IpV6 == ipAddrS.ipType || eIpType_Both == ipAddrS.ipType)
  {
    // Checks IpV6 duplication, only if it's manual configuration
    if (eV6Configuration_Manual == ipAddrS.ipV6configType)
    {
      // Checks new Cntrl address vs. new Shelf address
      if (0 == strncmp(ipAddrS.iPv6_str, ipAddrS.iPv6_shelf, IPV6_ADDRESS_LEN))
      {
        failed += 6;
        TRACEINTOFUNC << "New Cntrl iPv6 (" << ipAddrS.iPv6_str
                      << ") is similar to new Shelf address";
      }
      else
      {
        // Checks new Cntrl address duplication in general
        if (strcmp(ipAddrS.iPv6_str, m_cntrlIPv6AddressInAutoMode[0].address) == 0)
        {
          TRACEINTOFUNC << "The IpV6 address is the same address the control"
                        << " already has in auto mode";
        }
        else if (strcmp(ipAddrS.iPv6_str, m_pProcess->GetCntrlIPv6Address().c_str()) == 0)
        {
          TRACEINTOFUNC << "The IpV6 address is the same address the control"
                        << " already has in manual - no change";
        }
        else if (YES == IsTarget() && IsSystemFlagTrue(CFG_KEY_DUPLICATE_IP_DETECTION))        {
          stat = api.DADRequest(ipAddrS.iPv6_str);
          TRACEINTOFUNC << "New Cntrl IPv6 (" << ipAddrS.iPv6_str << ") duplicated? - "
                        << m_pProcess->GetStatusAsString(stat).c_str();
          if (stat)
            failed += 6;
        }
      }
    } // IPv6 Manual
  } // IPv6

  if (0 != failed)
  {
    stat = STATUS_DUPLICATE_ADDRESS_CNTRL;
    if (eIpType_Both == ipAddrS.ipType)
    {
      if (4 == failed)
        stat = STATUS_DUPLICATE_IPV4_ADDRESS_CNTRL;
      else if (6 == failed)
        stat = STATUS_DUPLICATE_IPV6_ADDRESS_CNTRL;
    }
  }
  else
    stat = STATUS_OK;

  return stat;
}

STATUS CMcuMngrManager::CheckDuplicateIpIfConfigList(std::string strIfName, std::string ipv4Addr)
{
	struct ifaddrs ifAddrs;
	struct ifaddrs *pIfAddrs = &ifAddrs;
	struct ifaddrs *pIfa = NULL;
	int status;
	struct sockaddr_in *sin;
	
	status = getifaddrs(&pIfAddrs);
	if (status)
	{
 		TRACEINTOFUNC << "CMcuMngrManager::CheckIpwithIfConfigList: getifaddrs failed, status =" << status << "IPV4Addr: " << ipv4Addr << "IfName: " << strIfName;
		return STATUS_OK;
 	}

	for (pIfa = pIfAddrs; pIfa; pIfa = pIfa->ifa_next) 
	{	
		sin = (struct sockaddr_in *) pIfa->ifa_addr;

		if (sin)
		{
			switch (sin->sin_family) 
			{

				case AF_INET:
					if (ipv4Addr == inet_ntoa(sin->sin_addr))
					{
						if (strcmp(strIfName.c_str(), pIfa->ifa_name)) 
						{
						   	TRACEINTOFUNC << "IP Confilict with IFCfg";
							return STATUS_FAIL;
						}
					}
		          	//TRACEINTOFUNC << "CMcuMngrManager::CheckIpwithIfConfigList: if name: " << pIfa->ifa_name << "; Port: " << ntohs(sin->sin_port) << "; IP Addr: " << inet_ntoa(sin->sin_addr) << "##" << sin->sin_addr.s_addr;
		            break;
				case AF_INET6: 
				default:
					break;
			}
		}
	}

	TRACEINTOFUNC << "CMcuMngrManager::CheckIpwithIfConfigList: check IP List OK!";
	return STATUS_OK;
}
STATUS CMcuMngrManager::CheckDuplicateIpCntrlAndOther(const IP_ADDR_S& ipAddrS)
{
  short failed = 0;

  // Checks new Cntrl IpV4 address
  if ( eIpType_IpV4 == ipAddrS.ipType )
  {
  	
    // Checks new Cntrl address vs. other interface address
	eConfigInterfaceType ifType = GetMngrIfType();
	std::string nic_name = GetLogicalInterfaceName(ifType, eIpType_IpV4);
	
	std::string tmpIPv4 = ipAddrS.iPv4_str;
	STATUS retStatus = CheckDuplicateIpIfConfigList(nic_name,tmpIPv4);
	if (STATUS_OK != retStatus)
	{
		TRACEINTOFUNC << "New Cntrl IPv4 (" << ipAddrS.iPv4_str
		            << ") is similar to other ifconfig IP ";
		return STATUS_DUPLICATE_IPV4_ADDRESS_CNTRL;
	}
    
   } // IPv4
  
	return STATUS_OK;
}

bool CMcuMngrManager::IsArpingNeeded(const char *newIpAddressStr)
{
	bool isNeeded = false;

	// ===== 1. if the previous configuration failed
	if (eIpTypeConfigSuccess_Both != m_isIpConfigInOsSucceeded &&
		  eIpTypeConfigSuccess_IPv4 != m_isIpConfigInOsSucceeded)
	{
		isNeeded = true;
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::IsArpingNeeded - Arping is needed"
							   << "\nm_isIpConfigInOsSucceeded: " << GetIpTypeConfigSuccessAsString(m_isIpConfigInOsSucceeded);
	}
	// ===== 2. previous configuration succeeded, but it's a different address now
	else
	{
		//    ---- 2a. the new address as DWORD
		DWORD dNewIpAddress = SystemIpStringToDWORD(newIpAddressStr);

		//    ---- 2b. mngmnt address as DWORD
		DWORD dMngmntIpAddress = 0;
		CIPSpan *pSpan = m_pMngmntIpParams_asIpService_fromProcess->GetFirstSpan(); // Mngmnt params are stored in the 1st span (idx==0)
		if (pSpan)
			dMngmntIpAddress = pSpan->GetIPv4Address();

		//    ---- 2c. compare addresses
		if (dNewIpAddress != dMngmntIpAddress)
		{
			isNeeded = true;

			char mngmntAddrStr[IP_ADDRESS_LEN];
			SystemDWORDToIpString(dMngmntIpAddress, mngmntAddrStr);
			TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::IsArpingNeeded - Arping is needed"
								   << "\nnewIpAddress:    " << newIpAddressStr
								   << "\nmngmntIpAddress: " << mngmntAddrStr;
		}
		// ===== 3. Arping is not needed
		else
		{
			isNeeded = false;

			TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::IsArpingNeeded - Arping is not needed";
		}
	}

	return isNeeded;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::HandleSetMcuRestore(CRequest *pRequest)
{
	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	if(pRequest->GetAuthorization() != SUPER || IsFederalOn())
	{
		if (IsFederalOn())
		{
			FPTRACE(eLevelInfoNormal,"CMcuMngrManager::HandleSetMcuRestore: No permission to set mcu restore type in federal mode");
		}
		else
		{
			FPTRACE(eLevelInfoNormal,"CMcuMngrManager::HandleSetMcuRestore: No permission to set mcu restore type");
		}

		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_OK;
	}

	CSetMcuRestore *pSetMcuRestore = dynamic_cast<CSetMcuRestore *>(pRequest->GetRequestObject());
	eMcuRestoreType restoreType = pSetMcuRestore->GetMcuRestoreType();

	/********************************************************************************/
	/* 11.03.10 added by Rachel cohen                                               */
	/* send restore factory defaults to switch on transaction                       */
	/********************************************************************************/
	if (restoreType == eMcuRestoreInhensive)
		SendRestoreFactoryDefaultReqToMplApi(restoreType);

	string answer = "";

	// during the restore factory defaults operation(see Start.sh), if this file exists, it means
	// the restore operation is performed through administration tools in the RMX menu
	// (RMX Support Tools Guide ver. 7.0.pdf), in that case we'll delete all the logs (see Start.sh)
	// VNGR-17102
	BOOL res = CreateFile(DEFAULT_RESTORE_USING_AdministrationTools_FILE);
	answer += "HandleSetMcuRestore 1 : Creating the file : " ;
	answer += DEFAULT_RESTORE_USING_AdministrationTools_FILE;
	answer += "; status = ";
	answer += (TRUE == res ? "True" : "False");
	answer += "\n";

	STATUS status = PerformRestoreFactoryDefaults(restoreType, answer);
    pRequest->SetStatus(status);

	return STATUS_OK;
}

STATUS CMcuMngrManager::HandleTurnSsh(CRequest *pRequest)
{
    const CSetTurnSsh *pSetTurnSsh = dynamic_cast<const CSetTurnSsh *>(pRequest->GetRequestObject());
    bool turnSshValue = pSetTurnSsh->GetTurnSsh();

	pRequest->SetConfirmObject(new CDummyEntry());

	//only admin can turn on SSH
	if(pRequest->GetAuthorization() != SUPER || IsFederalOn())
    {
    	if (IsFederalOn())
    	{
    		FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::HandleTurnSsh: No permission to turn SSH ON in federal mode";
    	}
    	else
    	{
    		FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::HandleTurnSsh: No permission to turn SSH ON";
    	}
		pRequest->SetStatus(STATUS_NO_PERMISSION);
        return STATUS_OK;
    }


    STATUS status = TurnSsh(turnSshValue);
    pRequest->SetStatus(status);

    return STATUS_OK;
}

STATUS CMcuMngrManager::HandleIpmiEntityReset(CRequest *pRequest)
{
    STATUS nStatus = STATUS_OK;
    if(pRequest->GetAuthorization() != SUPER)
    {
        FPTRACE(eLevelInfoNormal,"CMcuMngrManager::HandleIpmiEntityReset: No permission to reset system");

        pRequest->SetConfirmObject(new CDummyEntry);
        pRequest->SetStatus(STATUS_NO_PERMISSION);
        return nStatus;
    }

    pRequest->SetConfirmObject(new CDummyEntry);
    if (IsPlatformOfNewArch())
    {
        string desc = "Reset on demand from EMA.\n";

        TRACESTR(eLevelInfoNormal) << "\n" << desc.c_str();

        const CIpmiEntityReset *pResetReq = dynamic_cast<const CIpmiEntityReset *>(pRequest->GetRequestObject());
	  //check enter diagnostic flag.
        int const resetType = pResetReq->GetResetType();
	  if(RESET_TYPE_DIAGNOSTIC == resetType)
	  {
	  	string diagFlag = "";
		diagFlag = diagFlag + "sudo touch " + ENTER_DIAGNOSTIC_FLG;
	  	system(diagFlag.c_str());
	  }

        int const slotID = pResetReq->GetSlotId();
        if (IPMI_SLOT_ID_RESET==slotID)
        {
            StartResetTimer(desc, 5);
        }
        else
        {
            system("sudo /sbin/poweroff");
        }

        pRequest->SetStatus(nStatus);
        return STATUS_OK;
    }
    else
    {
        pRequest->SetStatus(nStatus);
        return STATUS_OK;
    }
}

STATUS CMcuMngrManager::HandleReset(CRequest *pRequest)
{
	if (IsPlatformOfNewArch())
	{
		string desc = "Reset on demand from EMA.\n";

		TRACESTR(eLevelInfoNormal) << "\n" << desc.c_str();
		if (pRequest)
		{
		    pRequest->SetConfirmObject(new CDummyEntry);
		    pRequest->SetStatus(STATUS_OK);
		}
	
		StartResetTimer(desc, 5);

		return STATUS_OK;
	}

	TRACEINTO << "CMcuMngrManager::HandleReset";
	if(NULL == pRequest)
	{
		PASSERTMSG(1, "CMcuMngrManager::HandleReset pRequest = NULL");
		return STATUS_FAIL;
	}

	pRequest->SetConfirmObject(new CDummyEntry);

	if(pRequest->GetAuthorization() != SUPER)
	{
		FPTRACE(eLevelInfoNormal,"CMcuMngrManager::HandleReset: No permission to reset system");
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_OK;
	}

	// restart service for SoftMcu and SoftMcuEdgeAxis
	if( eProductTypeSoftMCU == m_mcuMngrProductType || eProductTypeEdgeAxis == m_mcuMngrProductType || eProductTypeSoftMCUMfw == m_mcuMngrProductType )
	{
		if (pRequest)
		{
			STATUS st = RestartSoftMcuService(m_mcuMngrProductType);
			TRACEINTO << "restart soft mcu service status = " << st << ".";

			pRequest->SetConfirmObject(new CDummyEntry);
			pRequest->SetStatus(STATUS_OK);
		}
		return STATUS_OK;
	}
       

    	eProductFamily productFamily = CProcessBase::GetProcess()->GetProductFamily();
	switch (productFamily)
	{
		case eProductFamilyCallGenerator:
		    break;
		case eProductFamilyRMX:
		case eProductFamilySoftMcu:
		{
		    FPTRACE(eLevelInfoNormal,"CMcuMngrManager::HandleReset: This Reset transction is not supported by this product");
		    pRequest->SetStatus(STATUS_ILLEGAL_REQUEST);
		    return STATUS_OK;
		}
		default:
		    PASSERT(1);
	}

	const string &user = GetCurrentMsgHdr().GetUserName();
	const string &ip = GetCurrentMsgHdr().GetClientIp();
	const string &workStation = GetCurrentMsgHdr().GetWorkStation();

	CSegment *seg = new CSegment;
	*seg << user << ip << workStation;

	CManagerApi api(eProcessMcuMngr);
	STATUS status = api.SendMsg(seg, MCMS_RESET);

	TRACEINTO << "CMcuMngrManager::HandleReset user = " << user << ", ip = " << ip << ", workstation = " << workStation;

	pRequest->SetStatus(status);

    return STATUS_OK;
}

void CMcuMngrManager::OnReset(CSegment *pSeg)
{
    if (CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator)
    {
    	PTRACE(eLevelInfoNormal,"CMcuMngrManager::OnReset - ERROR - system is not CG!!");
    	return;
    }

    TRACEINTO << "CMcuMngrManager::OnReset";

	if(NULL == pSeg)
	{
		PASSERTMSG(1, "CMcuMngrManager::OnReset pRequest = NULL");
	}
	else
	{
		string ip, user, workStation;
		(*pSeg) >> user >> ip >> workStation;

		string reset = "user name: " + user + ", ip: " + ip + ", workstation: " + workStation;

		CMcmsDaemonApi api;
		api.SendResetExternalReq(reset);
	}
}

BOOL CMcuMngrManager::IsFederalOn() const
{
	return IsSystemFlagTrue(CFG_KEY_JITC_MODE);
}

void CMcuMngrManager::SendMNGMNTServiceToGK() const
{
	MngmntParamStruct data;
	memset(&data, 0, sizeof(MngmntParamStruct));

	const CIPSpan * pFirstSpan = m_pMngmntIpParams_asIpService_fromProcess->GetSpanByIdx(0); // Control params are stored in the 1st span (idx==0)
	if (NULL != pFirstSpan)
	{
		data.ipAddress = pFirstSpan->GetIPv4Address();
	}

	TRACEINTO << CMngmntParamStructWrapper(data);

	STATUS status = STATUS_OK;
	for (size_t i = 1; i <= MAX_SERVICES_NUM; ++i)
	{
		CSegment* pSeg = new CSegment;
		pSeg->Put((BYTE*)&data, sizeof(MngmntParamStruct));

		//CManagerApi api(eProcessGatekeeper);
		CGatekeeperTaskApi api(i);
		status = api.SendMsg(pSeg, MCUMNGR_GK_MNGMNT_IND);
	}

	TRACEINTO << "Status:" << m_pProcess->GetStatusAsString(status);
}

/////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::HandleGetMcuStateEx(CRequest* pRequest)
{
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::HandleGetMcuStateEx";

	CMcuStateGetEx* pMcuStateGetEx = new CMcuStateGetEx();
	*pMcuStateGetEx  = *(CMcuStateGetEx*)pRequest->GetRequestObject();

	//changed by huiyu
	CProcessBase *curProcess = CProcessBase::GetProcess();

	bool bFeatureEnabled = curProcess->GetIsFailoverFeatureEnabled();
	bool bSlaveMode     = curProcess->GetIsFailoverSlaveMode();
	string szIPFromSlave = pMcuStateGetEx->GetClientIp();

	if(bFeatureEnabled && !bSlaveMode &&(szIPFromSlave == m_szPairIPFailover))
	{
		//1- send to eProcessFailover
		char* ip = pMcuStateGetEx->GetClientIp();
		CSegment *pMsg = new CSegment;
		WORD len = strlen(ip);
		*pMsg << len;
		*pMsg << ip;

		CManagerApi api(eProcessFailover);
		STATUS status = api.SendMsg(pMsg, FAILOVER_MCUMNGR_GET_STATE_EX_IND);
		if (status != STATUS_OK)
			FPASSERT(FAILOVER_MCUMNGR_GET_STATE_EX_IND);

		CMcuStateGetExReply* pMcuStateGetExReply = new CMcuStateGetExReply();
		pMcuStateGetExReply->SetFailoverTrigger(m_bTriggerFailover);
		pRequest->SetStatus(STATUS_OK);
		pRequest->SetConfirmObject(pMcuStateGetExReply);
	}
	else
	{
		pRequest->SetStatus(STATUS_FAIL);
		pRequest->SetConfirmObject(new CDummyEntry);
	}

	POBJDELETE(pMcuStateGetEx);
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::PerformRestoreFactoryDefaults(eMcuRestoreType restoreType,
                                                      string &answer,
                                                      bool FromSwitch)
{
	std::string description = "Default system settings will be restored once Reset is completed";
	switch(restoreType)
	{
		case eMcuRestoreStandard:
		case eMcuRestoreInhensive:
		{
			// set to default user list in authentication
		    CManagerApi apiToAuthen(eProcessAuthentication);
		    apiToAuthen.SendOpcodeMsg(MCUMNGR_AUT_SET_DEFAULT_USER_LIST);
		    answer += "PerformRestoreFactoryDefaults 1: send set default user list request to authentication\n";

			// during the next startup the system will find this file and it will be trigger to
    		// perform the restore factory defaults operation(see Start.sh).
		    BOOL res = CreateFile(DEFAULT_RESTORE_FILE);
   			answer += "PerformRestoreFactoryDefaults 2: Creating the file: " ;
  			answer += DEFAULT_RESTORE_FILE;
        	answer += "; status = ";
  		    answer += (TRUE == res ? "True" : "False");
    		answer += "\n";
			break;
		}
		case eMcuRestoreBasic:
		{
			CConfigManagerApi configApi;
			configApi.DeleteFile(FIRST_REBOOT_FILE);

		    // format description for active alarm
		    description = "Overall logs deletion will be done once Reset is completed";

			break;
		}
		default:
			answer += "Bad restore type";
            return STATUS_BAD_RESTORE_TYPE;
	}

    if(eMcuRestoreInhensive == restoreType)
	{
    	// Sends CFS_keycode_reset request to Mpl
    	ResetCfsKeycodes();
    	answer += "PerformRestoreFactoryDefaults 3: ResetCfsKeycodes\n";

    	if (FromSwitch == false)
    	{

    		// Sends Default MNGMNT service to Mpl
    		SetDefaultMNGMNTService();
    		answer += "PerformRestoreFactoryDefaults 4: SetDefaultMNGMNTService\n";
    	}
    	else
    	{
    		// Removes the MngrmntNetwork configuration file from flash
    		DeleteFile(MANAGEMENT_NETWORK_CONFIG_PATH);
    	}

    	// Removes configuration files
    	DeleteConfigFiles();


    	answer += "PerformRestoreFactoryDefaults 5: DeleteConfigFiles\n";
	}

    AddActiveAlarmSingleton( FAULT_GENERAL_SUBJECT,
							 RESTORING_DEFAULT_FACTORY,
							 MAJOR_ERROR_LEVEL,
							 description,
							 true,
							 true
							 );

    string desc = "Restore Factory Defaults - ";
    desc += GetMcuRestoreName(restoreType);

    answer += "PerformRestoreFactoryDefaults 6: Reset : ";
    answer += desc;
    answer += "\n";
    answer += "---------------------------------------\n";
    answer += "Restore Factory Defaults complete, next step is reset request to daemon\n";

    TRACESTR(eLevelInfoNormal) << "\n" << answer.c_str();

    StartResetTimer(desc, 5);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::StartResetTimer(const string & resetDesc, int numOfSeconds)
{
    CSegment *pSeg = new CSegment;
    *pSeg << resetDesc;

    StartTimer(MCUMNGR_RESET_TIMER, numOfSeconds * SECOND, pSeg);
}

/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnResetRequest(CSegment * pSeg)
{
    string desc;
    *pSeg >> desc;
    // restart service for SoftMcu and SoftMcuEdgeAxis
    if( eProductTypeSoftMCU == m_mcuMngrProductType || eProductTypeEdgeAxis == m_mcuMngrProductType || eProductTypeSoftMCUMfw == m_mcuMngrProductType)
    {
    	TRACEINTO << "restart soft mcu service ";
    	RestartSoftMcuService(m_mcuMngrProductType);
    }
    else
    {
    	CMcmsDaemonApi api;
    	api.SendResetReq(desc);
    }

}

////////////////////////////////////////////////////////////////////////////////
// Static
void CMcuMngrManager::DeleteConfigFiles(void)
{
    std::string fnames[] = {
        CA_CERTIFICATES_FILE,       // certificate trusted list
        CA_CERTIFICATES_XML_FILE,
        TEMP_KEYF,                  // personal certificate
        TEMP_CERTF,
        TEMP_CERT_REQ_F,
        KEYF,
        KEYF_DES3,
        CERTF,
        CRL_CA_FILE,                // certificate revocation list
        CRL_DB_FILE,
        TEMP_KEYF_FOR_CS,           // CS personal
        TEMP_CERTF_FOR_CS,
        TEMP_CERT_REQ_F_FOR_CS,
        KEYF_FOR_CS,
        CERTF_FOR_CS,
        GetCfgPath(eCfgParamUser),
        GetEmaCfgPath(eCfgParamUser)
    };

    for (std::string* it = fnames; it < ARRAYEND(fnames); ++it)
    {
        size_t num = std::distance(fnames, it);

        if (!IsFileExists(*it))
        {
            FTRACEINTOFUNC << std::setw(2) << num
                           << " File " << *it << " does not exist";
            continue;
        }

        if (DeleteFile(*it))
        {
            FTRACEINTOFUNC << std::setw(2) << num
                           << " File " << *it << " is removed";
            continue;
        }

        FPASSERTSTREAM(true,
            std::setw(2) << num
                << " DeleteFile: " << *it << ": " << strerror(errno));
    }

    CConfigManagerApi configApi;
    configApi.DeleteDir(HOME_CONFIG_OCS);


    configApi.DeleteDir(HOME_CONFIG_KEYS);
}

/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SetDefaultMNGMNTService()
{
    CIPService* pDefaultMNGMNTService = new CIPService();
    pDefaultMNGMNTService->SetName(MANAGEMENT_NETWORK_NAME);
    pDefaultMNGMNTService->SetIpServiceType(eIpServiceType_Management);

    DWORD netMask = SystemIpStringToDWORD("255.255.255.0");
    pDefaultMNGMNTService->SetNetMask(netMask);

    DWORD defaultGateway = SystemIpStringToDWORD("192.168.1.1");
    pDefaultMNGMNTService->SetDefaultGatewayIPv4(defaultGateway);

    CIPSpan span;
    DWORD ipAddressCntl = SystemIpStringToDWORD("192.168.1.254");
    span.SetIPv4Address(ipAddressCntl);
    pDefaultMNGMNTService->AddSpan_NoCheck(span);

    DWORD ipAddressShelf = SystemIpStringToDWORD("192.168.1.252");
    span.SetIPv4Address(ipAddressShelf);
    pDefaultMNGMNTService->AddSpan_NoCheck(span);

    IP_ADDR_S ipAddrS;
	// retrieve IP addresses for IpV6, IpV4
	RetrieveIpAddresses(pDefaultMNGMNTService, ipAddrS);


	if (m_mcuMngrProductType==eProductTypeEdgeAxis)
	{
		TRACESTR(eLevelInfoNormal) << "Setting is secured mode to true";
		pDefaultMNGMNTService->SetIsSecured(TRUE);
	}


    UpdateNetworkInterface("SetDefaultMNGMNTService", *pDefaultMNGMNTService, eMngmntIpFromEma);
}

/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::ResetCfsKeycodes()
{
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::ResetCfsKeycodes";

	ResetCfsKeycodeSpecific('X');
	ResetCfsKeycodeSpecific('U');

    if (IsPlatformOfNewArch() || eProductTypeEdgeAxis == m_mcuMngrProductType)
	{
		VERSION_S mcuVersion = m_pSystemVersions->GetMcuVersion();
		KeyCodeSaveLoader keySaveLoader;
		keySaveLoader.ResetKeyCode(mcuVersion);
	}
}

/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::ResetCfsKeycodeSpecific(char keycodeType)
{
	TRACESTR(eLevelInfoNormal) <<
	     "\nCMcuMngrManager::ResetCfsKeycodeSpecific - send a " << keycodeType << " keycode full of zeros to MPL";

	CFS_KEYCODE_S keyCodeStruct;
	memset(&keyCodeStruct, 0, sizeof(CFS_KEYCODE_S));
	keyCodeStruct.keycode[0] = keycodeType;

	CMplMcmsProtocol *mplPrtcl = new CMplMcmsProtocol();

	mplPrtcl->AddCommonHeader(MCUMNGR_UPDATE_CFS_KEYCODE_REQ);
	mplPrtcl->AddMessageDescriptionHeader();
	mplPrtcl->AddPhysicalHeader(1, m_switchBoardId, m_switchSubBoardId);
	mplPrtcl->AddData(sizeof(CFS_KEYCODE_S), (char*)&keyCodeStruct);

	CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("MCUMNGR_SENDS_TO_MPL_API");
	mplPrtcl->SendMsgToMplApiCommandDispatcher();

	POBJDELETE(mplPrtcl);

}

/////////////////////////////////////////////////////////////////////////////
bool CMcuMngrManager::IsPortSpeedDifferent(const CPortSpeed *leftPortSpeed, const CPortSpeed *rightPortSpeed)
{
    for(int i = 0 ; i < MAX_NUM_OF_PORTS_SPEED ; i++)
    {
        if(leftPortSpeed[i].GetNum() != rightPortSpeed[i].GetNum())
        {
            return true;
        }
        if(leftPortSpeed[i].GetSpeed() != rightPortSpeed[i].GetSpeed())
        {
            return true;
        }
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendTimeConfigReqToMplApi(CSystemTime* pTime, BOOL isInit/*=NO*/)
{
	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::SendTimeConfigReqToMplApi";
	NTP_REQUEST_S ntpTime;

	ntpTime.unNtpYesNo       = pTime->GetIsNTP();
	ntpTime.unOffset         = pTime->GetGMTOffset();
	ntpTime.unOffsetSign     = pTime->GetGMTOffsetSign();
	ntpTime.unSampleInterval = 0;
	memset(ntpTime.aunServerIp, 0, sizeof(ntpTime.aunServerIp));
	memset(ntpTime.ucIpV6AddressesArr, 0, sizeof(ntpTime.ucIpV6AddressesArr));


	for (int i=0;i<NTP_MAX_NUM_OF_SERVERS;i++)
	{
		std::string ipv4ToSet = pTime->GetNTP_IPAddress(i);
		DWORD ipv4 = 0;
		std::string  ipv4Str;

		//if dns adress - resolve address
		if ((isIpV4Str(ipv4ToSet.c_str())) && (INVALID_SERVER_ADDR_STR != ipv4ToSet))
		{
			ipv4 = SystemIpStringToDWORD(ipv4ToSet.c_str());
			ntpTime.aunServerIp[i] = ipv4;
		}
		else if(ipv4ToSet != ""  && (INVALID_SERVER_ADDR_STR != ipv4ToSet))
		{
			ipv4Str =  GetIpFromDnsAddress(ipv4ToSet);
			ipv4 = SystemIpStringToDWORD(ipv4Str.c_str());
			ntpTime.aunServerIp[i] = ipv4;

		}

		TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::SendTimeConfigReqToMplApi ipv4ToSet = " << ipv4ToSet;
		TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::SendTimeConfigReqToMplApi ipv4= " << ipv4;
		TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::SendTimeConfigReqToMplApi ipv4Str= " << ipv4Str.c_str();

	}

	const CStructTm* pStructTm = pTime->GetMCUTime();
	if (NO == isInit)
	{
		ntpTime.unTimeSec   = pStructTm->m_sec;
		ntpTime.unTimeMin   = pStructTm->m_min;
		ntpTime.unTimeHour  = pStructTm->m_hour;
		ntpTime.unTimeDay   = pStructTm->m_day;
		ntpTime.unTimeMonth = pStructTm->m_mon;
		ntpTime.unTimeYear  = pStructTm->m_year;
	}
	else
	{
		ntpTime.unTimeSec   = 0;
		ntpTime.unTimeMin   = 0;
		ntpTime.unTimeHour  = 0;
		ntpTime.unTimeDay   = 0;
		ntpTime.unTimeMonth = 0;
		ntpTime.unTimeYear  = 0;
	}

	for (int i=0;i<NTP_MAX_NUM_OF_SERVERS;i++)
	{
		//char* pTemp = (char*)(ntpTime.ucIpV6AddressesArr[i]);
		std::string ipv6ToSet = pTime->GetNTP_IPv6_Address(i);

		//if dns adress - resolve address
		if (ipv6ToSet != "" && !isIpV6Str(ipv6ToSet.c_str()))
		{
			ipv6ToSet =  GetIpFromDnsAddress(ipv6ToSet);
		}



		strncpy((char*)(ntpTime.ucIpV6AddressesArr[i]), ipv6ToSet.c_str(), IPV6_ADDRESS_LEN);
		TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::SendTimeConfigReqToMplApi " << (char*)(ntpTime.ucIpV6AddressesArr[i]);
	}

	//set ntp dscp value according to QOS_MANAGEMENT_NETWORK flag
	ntpTime.ulNtpDscp = GetValidatedDscpValue();
	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::SendTimeConfigReqToMplApi, ulNtpDscp(decimal) = " << ntpTime.ulNtpDscp;

	// print to trace
	PrintTimeConfigSentToMplApi(ntpTime);

	CMplMcmsProtocol *mplPrtcl = new CMplMcmsProtocol();

	mplPrtcl->AddCommonHeader(MCUMNGR_SET_SYSTEM_TIME_REQ);
	mplPrtcl->AddMessageDescriptionHeader();
	mplPrtcl->AddPhysicalHeader(1, m_switchBoardId, m_switchSubBoardId);
	mplPrtcl->AddData(sizeof(NTP_REQUEST_S), (char*)&ntpTime);

	CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("MCUMNGR_SENDS_TO_MPL_API");
	mplPrtcl->SendMsgToMplApiCommandDispatcher();

	POBJDELETE(mplPrtcl);
}


//Get QOS_MANAGEMENT_NETWORK value and convert to decimal, if not validated return default
APIU32 CMcuMngrManager::GetValidatedDscpValue()
{
	APIU32 ret = 0x10;//0x10 is the default QOS_MANAGEMENT_NETWORK value
	int dscpValue = -1;
	std::string data;
	CSysConfig* cfg = m_pProcess->GetSysConfig();
	BOOL res = cfg->GetDataByKey(QOS_MANAGEMENT_NETWORK, data);
	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::GetValidatedDscpValue, data = " << data;
	if (ValidateHexString(data.c_str()))
	{
		sscanf(data.c_str(), "%x", &dscpValue);
		if (dscpValue > -1 && dscpValue < 64)
		{
			ret = dscpValue;
		}
	}
	return ret;
}





/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::PrintTimeConfigSentToMplApi(NTP_REQUEST_S ntpTime)
{
	CLargeString timeConfigStr =
	                 "\nCMcuMngrManager::PrintTimeConfigSentToMplApi";
	timeConfigStr << "\n============================================\n" ;

	timeConfigStr << "NTP Enabled: ";
	if (ntpTime.unNtpYesNo)
	{
		timeConfigStr << "Yes\n";
	}
	else
	{
		timeConfigStr << "No\n";
	}

	char ipAddressStr[IP_ADDRESS_LEN];
	for (int i=0; i< NTP_MAX_NUM_OF_SERVERS; i++)
	{
		SystemDWORDToIpString(ntpTime.aunServerIp[i], ipAddressStr);
		timeConfigStr << "Server_Ip " << i << ": " << ipAddressStr << ",   ";
	}

	timeConfigStr << "\nSampleInterval: " << ntpTime.unSampleInterval
	              << ", Offset: "         << ntpTime.unOffset
	              << ", OffsetSign: "     << ntpTime.unOffsetSign
	              << ", Date & Time: "    << ntpTime.unTimeDay << "/" << ntpTime.unTimeMonth << "/"  << ntpTime.unTimeYear
	                          << " "      <<  ntpTime.unTimeHour << ":" << ntpTime.unTimeMin << ":" << ntpTime.unTimeSec;

	for (int i=0;i<NTP_MAX_NUM_OF_SERVERS;i++)
	{
		timeConfigStr << "Server_Ipv6 " << i << ": " << (char*)(ntpTime.ucIpV6AddressesArr[i]) << ",   ";
	}

	TRACESTR(eLevelInfoNormal) << timeConfigStr.GetString();
}

/////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::HandleSetTime(CRequest *pRequest)
{
    // for Call Generator - prevent changing the RMX time
	if (CProcessBase::GetProcess()->GetProductFamily() == eProductFamilyCallGenerator || m_mcuMngrProductType == eProductTypeSoftMCUMfw)
    {
    	PTRACE(eLevelInfoNormal,"CMcuMngrManager::HandleSetTime - ERROR - It is forbidden to change the Call Generator time!!");
    	pRequest->SetConfirmObject(new CDummyEntry());
    	pRequest->SetStatus(STATUS_FAIL);
    	return STATUS_OK;
    }



	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::HandleSetTime";

    if (pRequest->GetAuthorization() == SUPER)
    {
    	// (NGR-11164) JITC: Time/Date must not be modified while conference is active
    	if (TRUE == IsFederalOn())
    	{
    		STATUS retStat = SendSyncNumOfConfsMsgToConfPartyProcess();
    		if (STATUS_OK != retStat)
    		{
    	    	pRequest->SetConfirmObject(new CDummyEntry());

    	    	if (OPERATION_BLOCKED == retStat)
    	    	{
		            pRequest->SetStatus(OPERATION_BLOCKED);
		            pRequest->SetExDescription("RMX Time updating is blocked while running conferences exist");
    	    	}
    	    	else
    	    	{
    				pRequest->SetStatus(STATUS_FAIL);
    	    	}
    		    return STATUS_OK;
    		}
    	} // end IsFederalOn

        BOOL isOriginalNtpEnabled = IsSysTimeNtpEnabled();

        CSystemTime* pTime = (CSystemTime*)(pRequest->GetRequestObject());
        CSystemTime oldTime;
        m_pProcess->GetMcuTime(oldTime);

        DWORD ifNoneMatch = pRequest->GetIfNoneMatch() ;

    	if( (ifNoneMatch < MAX_UNSIGN) && (ifNoneMatch != oldTime.GetUpdateCounter()))
        	{
    			pRequest->SetConfirmObject(new CDummyEntry());
        		pRequest->SetStatus(STATUS_PRECONDITION_FAILED);
        		pRequest->SetExDescription("Precondition failed");
        		return STATUS_OK;
        	}


        /*	pRequest->SetConfirmObject(new CDummyEntry());
        	pRequest->SetStatus(STATUS_FAIL);
        	return STATUS_OK;*/


        std::string ip_new, ip_old;
        BOOL isIpSettingChanged = FALSE;

        for (int i = 0; i < NTP_MAX_NUM_OF_SERVERS; i++)
        {
            ip_new = pTime->GetNTP_IPAddress(i);
            const char *ip6_new = pTime->GetNTP_IPv6_Address(i);

            if (ip_new != oldTime.GetNTP_IPAddress(i) || strcmp(oldTime.GetNTP_IPv6_Address(i), ip6_new) != 0)
            {
    	    	TRACESTR(eLevelDebug) << "isIpSettingChanged  i " << i ;
                isIpSettingChanged = TRUE;
            }

            BOOL new_ip_connfigured = (ip_new != "") ||  ((strcmp(ip6_new, "::") != 0) && (strcmp(ip6_new, "") != 0));

            BOOL foundOldConfiguration = FALSE;
            if (YES == pTime->GetIsNTP())
            {
				for (int j = 0; j < NTP_MAX_NUM_OF_SERVERS && !foundOldConfiguration; j++)
				{
					ip_old = oldTime.GetNTP_IPAddress(j);
					const char *ip6_old = oldTime.GetNTP_IPv6_Address(j);

					if ( (ip_old != "") || ( (strcmp(ip6_old, "::") != 0) && (strcmp(ip6_old, "") != 0) ))
					{
						if (ip_old == ip_new && strcmp(ip6_old, ip6_new) == 0 && oldTime.GetNtpServerStatus(j) != eNtpServerNotConfigured)
						{

							foundOldConfiguration = TRUE;
							pTime->SetNtpServerStatus(i, oldTime.GetNtpServerStatus(j));
							pTime->SetNumFailuresSinceConnecting(i, oldTime.GetNumFailuresSinceConnecting(j));


							TRACESTR(eLevelDebug) << "foundOldConfiguration. i " << i << " j " << j << " old status " << (int)oldTime.GetNtpServerStatus(j);

						}
					}
				}

				if (!foundOldConfiguration)
				{

					if (new_ip_connfigured)
					{
						TRACESTR(eLevelDebug) << "NOT foundOldConfiguration...eNtpServerConnecting i  " << i ;

						pTime->SetNtpServerStatus(i, eNtpServerConnecting);
					}
					else
					{
						TRACESTR(eLevelDebug) << "NOT foundOldConfiguration...eNtpServerNotConfigured i  " << i ;

						pTime->SetNtpServerStatus(i, eNtpServerNotConfigured);
					}
					pTime->SetNumFailuresSinceConnecting(i, 0);
				}

            }
            else  // ntp server is not configured
            {
            	TRACESTR(eLevelDebug) << "ntp server is not configured";
            	pTime->SetNtpServerStatus(i, eNtpServerNotConfigured);
				pTime->SetNumFailuresSinceConnecting(i, 0);

            }
        }

	    pTime->PrintParams("CMcuMngrManager::HandleSetTime");

	    eIpType ipType  = m_pMngmntIpParams_asIpService_fromProcess->GetIpType();
	    if ( (eIpType_IpV6 == ipType) && (YES == pTime->GetIsNTP()) )
	    {
	    	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::HandleSetTime - NTP is not supported in IPv6_only";

	    	pRequest->SetConfirmObject(new CDummyEntry());
			pRequest->SetStatus(STATUS_FAIL);
			pRequest->SetExDescription("NTP is not supported in IPv6 only mode");
	    }

	    else // not IPv6 only
	    {
		    bool isValidAddresses = IsValidNtpServersAddresses(pTime);
		    //isValidAddresses = IsValidNtpIPv6ServersAddresses(pTime);
		    if (true == isValidAddresses)
		    {
		    	m_isRmxTimeChangedByUser = YES;

			    // ===== 1. set new time (and write to file)
		    	if(oldTime != *pTime)
		    	{
		    		oldTime.IncreaseUpdateCounter();
		    		pTime->SetUpdateCounter(oldTime.GetUpdateCounter());
		    	}
		    	else
		    		pTime->SetUpdateCounter(oldTime.GetUpdateCounter());
			    m_pProcess->SetMcuTime(*pTime);
			    SendGmtOffset();
		    	RemoveActiveAlarmByErrorCode(AA_NO_TIME_CONFIGURATION);
		   		BOOL isNewNtpEnabled = IsSysTimeNtpEnabled();
				STATUS step2_result = STATUS_OK;

				if (eProductTypeGesher == m_mcuMngrProductType || eProductTypeNinja == m_mcuMngrProductType || eProductTypeEdgeAxis == m_mcuMngrProductType)
				{
                    string isNTP = ( pTime->GetIsNTP() ? "yes" : "no" );
                    string isIPChanged = ( isIpSettingChanged ? "yes" : "no" ); 
                    TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::HandleSetTime test: " << isNTP << " ---- " << isIPChanged;
					//if (!pTime->GetIsNTP() || isIpSettingChanged)
					//{
                        TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::HandleSetTime call NtpSetServer.";
						// ===== 2. configure NTP configuration file MCU_MCMS_DIR/Cfg/ntp.conf
						step2_result = NtpSetServer(pTime);
					//}
				}
				else
				{
					// ===== 2. update Switch
			   		if (YES == m_isAuthenticationIndReceived)
			   		{
						SendTimeConfigReqToMplApi(pTime);
			   		}
				}

				if (step2_result == STATUS_OK)
				{
					// ===== 3. NTP
					SampleNtpIfNeeded(isOriginalNtpEnabled, isNewNtpEnabled);
					// ===== 4. inform Authenticator
					SendRMXTimeSetToAuthenticationProcess();

					// ===== 5. request confirm
				    pRequest->SetConfirmObject(new CDummyEntry);
					pRequest->SetStatus(STATUS_OK);
				}
				else
				{
					pRequest->SetConfirmObject(new CDummyEntry);
					pRequest->SetStatus(STATUS_FAIL);
				}
		    }

		    else // invalid servers addresses
		    {
		    	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::HandleSetTime - Invalid server(s) address(es)";

				pRequest->SetConfirmObject(new CDummyEntry());
				pRequest->SetStatus(STATUS_IP_ADDRESS_NOT_VALID);
		    }
	    } // end if not IPv6 only
    }

    else // not SUPER user
    {
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::HandleSetTime: No permission to update mcu time";

		pRequest->SetConfirmObject(new CDummyEntry());
		pRequest->SetStatus(STATUS_NO_PERMISSION);
    }

    /* VNGR-12000
    AddActiveAlarmSingleton(	FAULT_GENERAL_SUBJECT,
							ALLOCATION_MODE_CHANGED,
							MAJOR_ERROR_LEVEL,
							"System time was modified. Please reset the MCU.",
							true,
							true
						);
    */
    return STATUS_OK;
}



STATUS CMcuMngrManager::HandleSetPrecedenceSettings(CRequest *pRequest)
{

	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::HandleSetPrecedenceSettings";


    if (pRequest->GetAuthorization() == SUPER)
    {
    	TRACESTR(eLevelInfoNormal) << "pRequest->GetStatus() " << pRequest->GetStatus();
    	CPrecedenceSettings* pPrecedenceNew = (CPrecedenceSettings*)(pRequest->GetRequestObject());

		CMcuMngrProcess* pProcess = (CMcuMngrProcess*)CMcuMngrProcess::GetProcess();
		pProcess->SetPrecedenceSettings(pPrecedenceNew);
		pProcess->GetPrecedenceSettings()->WriteXmlFile();

    	pRequest->SetConfirmObject(new CDummyEntry());
    	pRequest->SetStatus(STATUS_OK);
    	//send precedence settings to ConfParty
    	SendPrecedenceSettingsToProcess(pPrecedenceNew, eProcessConfParty);
    	//send precedence settings to CsMngr
    	SendPrecedenceSettingsToProcess(pPrecedenceNew, eProcessCSMngr);
    }

    else // not SUPER user
    {
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::HandleSetPrecedenceSettings: No permission to update precedence settings";

		pRequest->SetConfirmObject(new CDummyEntry());
		pRequest->SetStatus(STATUS_NO_PERMISSION);
    }

    return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
//bool CMcuMngrManager::IsValidNtpServersAddresses(CSystemTime* pTime)
//{
//    bool isValid = true;
//
//    if ( pTime->GetIsNTP() ) // otherwise servers' addresses are irrelevant
//    {
//        DWORD curAddress = 0;
//
//        for (int i=0; i<NTP_MAX_NUM_OF_SERVERS; i++)
//        {
//            curAddress = pTime->GetNTP_IPAddress(i);
//            if (0 == curAddress)
//            {
//                isValid = false;
//                break;
//            }
//        }
//    }
//
//    return isValid;
//}

/////////////////////////////////////////////////////////////////////////////
bool CMcuMngrManager::IsValidNtpServersAddresses(CSystemTime* pTime)
{
	bool isIpEmpty = true;
	bool isDup = false;
	bool isIpV6Empty = true;
	bool isDupIpV6 = false;

    PASSERTMSG_AND_RETURN_VALUE(NULL == pTime, "Illegal parameter", false);

    // otherwise servers' addresses are irrelevant
    if (!pTime->GetIsNTP())
        return true;

    //check IpV4
    std::vector<std::string> ips;
    for (int i = 0; i < NTP_MAX_NUM_OF_SERVERS; i++)
    {
        std::string ip = pTime->GetNTP_IPAddress(i);

        // do not take 255.255.255.255 addresses
        if ("" == ip || "255.255.255.255" == ip)
            continue;

        ips.push_back(ip);
    }
    // there is no legal address
    isIpEmpty = ips.empty();
    if (!isIpEmpty)
    {
    	 // one of the address is 0
    	isIpEmpty = ("" == ips[0]);
    }
    if (!isIpEmpty)
    {
		std::sort(ips.begin(), ips.end());
		//found duplicate
		isDup = std::unique(ips.begin(), ips.end()) != ips.end();
    }



    //check IpV6
    isIpV6Empty = pTime->IsNTPIpv6AddressesEmpty();
    if (!isIpV6Empty)
    {
		const char** ipsV6 = pTime->GetNTP_IPv6_Addresses();
		char ipsV6Tmp[NTP_MAX_NUM_OF_SERVERS][IPV6_ADDRESS_LEN];
		memcpy(ipsV6Tmp, ipsV6, sizeof(ipsV6Tmp));
		qsort (ipsV6Tmp, NTP_MAX_NUM_OF_SERVERS, IPV6_ADDRESS_LEN, (int(*)(const void*,const void*))strcmp);
		for (int i=0; i<NTP_MAX_NUM_OF_SERVERS-1 && !isDupIpV6; ++i)
		{
			const char* strCurr = (const char*)ipsV6Tmp[i];
			const char* strNext = (const char*)ipsV6Tmp[i + 1];
			//check if not empty, or starts with CSystemTime::NA_IPV6_ADDRESS (initialized value)
			if(strCurr && strNext && strcmp(strCurr,CSystemTime::NA_IPV6_ADDRESS.c_str()) != 0 && strcmp(strNext,CSystemTime::NA_IPV6_ADDRESS.c_str()) != 0)
			{
				//found duplicate
				if  ((strcmp(strCurr, strNext) == 0) && (strCurr != '\0')  && (strcmp(strCurr, "") != 0))
					isDupIpV6 = true;
			}
		}
    }
    TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::IsValidNtpServersAddresses. " << "isIpEmpty=" << isIpEmpty << ", isDup=" << isDup << ", isIpV6Empty=" << isIpV6Empty << ", isDupIpV6=" << isDupIpV6;
    //if ipv4 and ipv6 are not ok or one of them has duplicates
	if (((isIpEmpty || isDup) && (isIpV6Empty || isDupIpV6))
			|| ((!isIpEmpty && isDup) || (!isIpV6Empty && isDupIpV6)) )
	{
		PASSERTMSG_AND_RETURN_VALUE(isDupIpV6, "NTP IpV6 address is duplicated", false);
		PASSERTMSG_AND_RETURN_VALUE(isDup, "NTP address is duplicated", false);
		PASSERTMSG_AND_RETURN_VALUE(isIpEmpty, "NTP addresses are illegal", false);
		PASSERTMSG_AND_RETURN_VALUE(isIpV6Empty, "NTP Ipv6 addresses are illegal", false);
	}

    return true;
}




/////////////////////////////////////////////////////////////////////////////
BOOL CMcuMngrManager::IsSysTimeNtpEnabled()
{
	BOOL isNtp = NO;

	CSystemTime sysTime;
	m_pProcess->GetMcuTime(sysTime);

	if ( sysTime.GetIsNTP() )
		isNtp = YES;

	return isNtp;
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SampleNtpIfNeeded(BOOL isOriginalNtpEnabled, BOOL isNewNtpEnabled)
{
	if (YES == IsTarget() ||
		(eProductTypeGesher == m_mcuMngrProductType || eProductTypeNinja == m_mcuMngrProductType || eProductTypeEdgeAxis == m_mcuMngrProductType))
	{
		// ===== 1. Ntp servers status
		DeleteTimer(NTP_SERVERS_STATUS_TIMER);

		if (YES == isNewNtpEnabled)
		{
			if (eProductTypeGesher == m_mcuMngrProductType || eProductTypeNinja == m_mcuMngrProductType || eProductTypeEdgeAxis == m_mcuMngrProductType)
			{
				StartTimer(NTP_SERVERS_STATUS_TIMER, SECOND*30);
			}
			else
			{
				StartTimer(NTP_SERVERS_STATUS_TIMER, m_NtpServersStatusPeriod*SECOND*60);
			}
		}
		else
			RemoveActiveAlarmByErrorCode(AA_EXTERNAL_NTP_SERVERS_FAILURE);


		// ===== 2. Ntp sync
		DeleteTimer(NTP_SYNC_TIMER);

		if (eProductTypeGesher == m_mcuMngrProductType || eProductTypeNinja == m_mcuMngrProductType || eProductTypeEdgeAxis == m_mcuMngrProductType)
		{
			if (YES == isNewNtpEnabled)
				StartTimer(NTP_SYNC_TIMER, 30*SECOND);
		}
		else
		{
			if ( (NO == isOriginalNtpEnabled) && (NO == isNewNtpEnabled) ) // no need in to wait so long
				StartTimer(NTP_SYNC_TIMER, 10*SECOND);
			else
				StartTimer(NTP_SYNC_TIMER, m_NtpSyncPeriod*SECOND*60);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::HandleSetEthernetSettings(CRequest *pRequest)
{
	STATUS generateFile = STATUS_FAIL;
    if (pRequest->GetAuthorization() == SUPER)
    {
       //modified "eProductFamilySoftMcu" by Richer for 802.1x project om 2013.12.26 
    	if (/*m_mcuMngrProductType==eProductTypeSoftMCU || */m_mcuMngrProductType==eProductTypeSoftMCUMfw || m_mcuMngrProductType==eProductTypeEdgeAxis || m_mcuMngrProductType==eProductTypeCallGeneratorSoftMCU)	//hard coded license for softMcu
    	{
    		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::HandleSetEthernetSettings: Can't update ethernet setting in soft mcu";

    		pRequest->SetConfirmObject(new CDummyEntry());
    		pRequest->SetStatus(STATUS_ETHERNET_SETTING_CANT_BE_UPDATED_IN_SMCU);
    		return STATUS_OK;
    	}

    	CEthernetSettingsConfig* pEthernetSettingsConfig = (CEthernetSettingsConfig*)(pRequest->GetRequestObject());



       	eEthPortType portType = pEthernetSettingsConfig->GetPortType();
        ePortSpeedType portSpeed = pEthernetSettingsConfig->GetPortSpeed();

        eEth802_1xAuthenticationType authProtocol802_1x = pEthernetSettingsConfig->Get802_1xAuthenticationProtocol();

    	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::HandleSetEthernetSettings"
    						   << "\nSlot Id:   " << pEthernetSettingsConfig->GetSlotId()
    						   << "\nPort Id:   " << pEthernetSettingsConfig->GetPortId()
    						   << "\nPort Type: " << GetEthPortTypeStr(portType)
    						   << "\nSpeed:     " << ::PortSpeedTypeToString(portSpeed)
    	                       << "\n802.1x authentication protocol:     " << ::Get802_1xAuthenticationTypeStr(pEthernetSettingsConfig->Get802_1xAuthenticationProtocol());

    	/************************************************************************/
    	/* 23.2.10 added by Rachel Cohen                                        */
    	/* VNGFE 2587 and VNGFE 2582    on RMX4000 only                         */
    	/* managment and signalling port can not be configured to 1000MB        */
    	/************************************************************************/

    	//Only for RMX4000 (RMX1500 enable 1G)
    	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

      	if ((eProductTypeRMX4000 == curProductType)&&
      			 ((portType == eEthPortType_Management1) ||
        		 (portType == eEthPortType_Management2) ||
        		 (portType == eEthPortType_Signaling1)  ||
        		 (portType == eEthPortType_Signaling2))  &&

        		 ((portSpeed == ePortSpeed_1000_HalfDuplex) ||(portSpeed == ePortSpeed_1000_FullDuplex) ))
        		{
        		   TRACESTR(eLevelInfoNormal) <<
        		     "\nCMcuMngrManager::HandleSetEthernetSettings :can not config speed of 1G to managment/signalling ports" ;

        		    pRequest->SetConfirmObject(new CDummyEntry());
        			pRequest->SetStatus(STATUS_PORT_DOES_NOT_SUPPORT_SPEED_1G);
        			pRequest->SetExDescription("Port does not support speed of 1G");

        			return STATUS_PORT_DOES_NOT_SUPPORT_SPEED_1G;
        		}
      	// ===== 0. 802.1x check if authentication protocol has been changed

      	E_802_1xCertificateValidationStatus TLSCertStatus = E_CERTIFICATE_OK;


      	CEthernetSettingsConfig* pEthernetSettingsConfigOld = NULL;
        pEthernetSettingsConfigOld = GetEthernetSettings_specEntity( pEthernetSettingsConfig->GetSlotId(), pEthernetSettingsConfig->GetPortId() );

      	if (pEthernetSettingsConfigOld == NULL )
      	{
      		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::HandleSetEthernetSettings: unknown Entity";

      		pRequest->SetConfirmObject(new CDummyEntry);
      		pRequest->SetStatus(STATUS_OK);
      		std::string description = "pEthernetSettingsConfigOld is null";
      		PASSERTMSG(TRUE, description.c_str());
      		return STATUS_FAIL;
      	}

        //check if 802.1x has been enabled
      	if ((pEthernetSettingsConfigOld->Get802_1xAuthenticationProtocol() == eEth802_1xAuthenticationType_Off) &&
      			(pEthernetSettingsConfig->Get802_1xAuthenticationProtocol() != eEth802_1xAuthenticationType_Off))
      	{
      		generateFile = GenerateConfFlie802_1x(pEthernetSettingsConfig,TLSCertStatus);
      		 TRACESTR(eLevelInfoNormal) <<
      		        		     "\nCMcuMngrManager::HandleSetEthernetSettings :802_1x enabled flag changed from OFF to ON generateFile status " << generateFile;
      		// if we keep the new api we must prepare the files here  we use .
      		//sendSelfMsgToCreateConfFile(pEthernetSettingsConfig);

      		 if (generateFile == STATUS_OK)
      			    	//send 802 req msg
      			    	Start802_1xConfiguration_specificEntry(pEthernetSettingsConfig,true);
      	}
      	else if ((pEthernetSettingsConfigOld->Get802_1xAuthenticationProtocol() != eEth802_1xAuthenticationType_Off) &&
      			(pEthernetSettingsConfig->Get802_1xAuthenticationProtocol() == eEth802_1xAuthenticationType_Off))
      	{
      		//generateFile = STATUS_OK;
    		 TRACESTR(eLevelInfoNormal) <<
     		        		     "\nCMcuMngrManager::HandleSetEthernetSettings :802_1x enabled flag changed from ON to OFF  " ;

    		 End802_1xConfiguration_specificEntry(pEthernetSettingsConfig);

      	}

      	else if ((pEthernetSettingsConfigOld->Get802_1xAuthenticationProtocol() != eEth802_1xAuthenticationType_Off) &&
      			(pEthernetSettingsConfig->Get802_1xAuthenticationProtocol() != eEth802_1xAuthenticationType_Off) )
      		//(pEthernetSettingsConfigOld->Get802_1xAuthenticationProtocol() != pEthernetSettingsConfig->Get802_1xAuthenticationProtocol()) )
      	{
      		//generateFile = STATUS_OK;
      		TRACESTR(eLevelInfoNormal) <<
      				"\nCMcuMngrManager::HandleSetEthernetSettings :802_1x enabled flag changed from auth method " <<
      				::Get802_1xAuthenticationTypeStr(pEthernetSettingsConfigOld->Get802_1xAuthenticationProtocol()) <<
      				 " to auth method  "
      				 << ::Get802_1xAuthenticationTypeStr(pEthernetSettingsConfig->Get802_1xAuthenticationProtocol());

      		End802_1xConfiguration_specificEntry(pEthernetSettingsConfig);

      		generateFile = GenerateConfFlie802_1x(pEthernetSettingsConfig,TLSCertStatus);
      		TRACESTR(eLevelInfoNormal) <<
      				"\nCMcuMngrManager::HandleSetEthernetSettings :802_1x enabled flag changed from OFF to ON generateFile status " << generateFile;
      		// if we keep the new api we must prepare the files here  we use .
      		//sendSelfMsgToCreateConfFile(pEthernetSettingsConfig);

      		if (generateFile == STATUS_OK)
      			//send 802 req msg
      			Start802_1xConfiguration_specificEntry(pEthernetSettingsConfig,true);


      	}

      	//add event to audit file
      	SendEthSetChangeAuditEvent(pEthernetSettingsConfigOld,pEthernetSettingsConfig);

      	// ===== 1. set new settings (and write to file)
      	m_pProcess->SetEthernetSettingsConfig(*pEthernetSettingsConfig);




      	// ===== 2. ask for configuration
      	ConfigEthernet_specEntity( pEthernetSettingsConfig->GetSlotId(), pEthernetSettingsConfig->GetPortId() );

      	switch(TLSCertStatus)
      	{
      	case E_CERTIFICATE_OK:

      		pRequest->SetConfirmObject(new CDummyEntry);
      		pRequest->SetStatus(STATUS_OK);
      		return STATUS_OK;


      	case E_CERTIFICATE_DO_NOT_INCLUDE_SANS:

      		pRequest->SetConfirmObject(new CDummyEntry());
      		pRequest->SetStatus(STATUS_CERTIFICATE_SHOULD_INCLUDE_SANS);
      		pRequest->SetExDescription("Fot TLS authentication ,Certificate must include SANS extention");

      		return STATUS_CERTIFICATE_SHOULD_INCLUDE_SANS;

      	case E_CERTIFICATE_DO_NOT_INCLUDE_SANS_PRINCIPLE:

      		pRequest->SetConfirmObject(new CDummyEntry());
      		pRequest->SetStatus(STATUS_CERTIFICATE_SHOULD_INCLUDE_SANS_PRINCIPLE);
      		pRequest->SetExDescription("In TLS Certificate must include SANS Principle extention");

      		return STATUS_CERTIFICATE_SHOULD_INCLUDE_SANS_PRINCIPLE;


      	case E_CERTIFICATE_CN_NOT_EQUAL_TO_USERNAME:

      		pRequest->SetConfirmObject(new CDummyEntry());
      		pRequest->SetStatus(STATUS_CERTIFICATE_CN_SHOULD_BE_EQUAL_TO_USERNAME);
      		pRequest->SetExDescription("The user name in TLS should be same as Certificate Common Name");

      		return STATUS_CERTIFICATE_CN_SHOULD_BE_EQUAL_TO_USERNAME;

      	case E_CERTIFICATE_NO_SUCH_FILE:
      		pRequest->SetConfirmObject(new CDummyEntry());
      		pRequest->SetStatus(STATUS_CERTIFICATE_FILE_NOT_EXIST);
      		pRequest->SetExDescription("User must create a Certificate.");

      		return STATUS_CERTIFICATE_FILE_NOT_EXIST;
      	case E_CERTIFICATE_NO_COMMON_NAME:

      		pRequest->SetConfirmObject(new CDummyEntry());
      		pRequest->SetStatus(STATUS_CERTIFICATE_COMMON_NAME_NOT_EXIST);
      		pRequest->SetExDescription("The user name in TLS should be same as Certificate Common Name");

      		return STATUS_CERTIFICATE_COMMON_NAME_NOT_EXIST;

      	default:
      		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::HandleSetEthernetSettings: unknown TLS certificate status";

      		pRequest->SetConfirmObject(new CDummyEntry);
      		pRequest->SetStatus(STATUS_OK);

      	}

    }
    else
    {
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::HandleSetEthernetSettings: No permission to update ethernet settings";

		pRequest->SetConfirmObject(new CDummyEntry());
		pRequest->SetStatus(STATUS_NO_PERMISSION);
    }

    return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnProcessStateChanged(CSegment* pSeg)
{
	// ===== 1. extract the info received from each Process
	CProcessStateFaultList* pTmpFaultsList = new CProcessStateFaultList;
	pTmpFaultsList->DeSerialize(*pSeg);


	// ===== 2. to logger
	eProcessType  processType     = pTmpFaultsList->GetProcessType();
	eProcessStatus newProcessState = pTmpFaultsList->GetProcessStatus();
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnProcessStateChanged - "
                                  << "Process Type: " << CProcessBase::GetProcessName(processType)
                                  << ", New State: "  << GetProcessStatusName(newProcessState);


    // ===== 3. get lists of new/deleted alarms for SNMP
    vector<CLogFltElement> newAlarmVector;
    vector<CLogFltElement> deletedAlarmVector;
    FindChangesInAlarms(pTmpFaultsList, newAlarmVector, deletedAlarmVector);

    // ===== 5. add (pr update) this process's entry in m_pProcessesStatesMap
	AddFaultListToProcessStateMap(pTmpFaultsList);

    // ===== 6. update activeAlarms list (on process level)
	UpdateActiveAlarmsList_inProcess();

	// ===== 7. recalculate MCU state
	RecalculateMcuState();

    // ===== 8. if there are some changes update SNMP alarm table with the new alarm table
    if(!newAlarmVector.empty() || !deletedAlarmVector.empty())
    {

        string mngmntIp;
        m_pProcess->GetMngmntIpAsString(mngmntIp);

        UpdateSnmpAlarmTable(m_pActiveAlarmList_fromProcess, mngmntIp);

        eProcessWorkMode workMode = m_pProcess->GetWorkMode();

        if(eProcessWorkModeNormal == workMode)
        {

            CSnmpUtils::UpdateAlarmActiveLastChanged();
        }
        else
        {
            // eProcessWorkModeUnitTest == workMode
            // we don't need it in TDD mode
        }
    }

       // ===== 4. Send SNMP trap, only if SNMP is up





    //  if(m_isSNMPup)
  if (m_isSNMPReady)
  {

	    eMcuState rmxStatus = m_pProcess->GetSystemState();
	    CSnmpUtils::SendAlarmListSnmpTrap(newAlarmVector, false, (int)rmxStatus);
	    CSnmpUtils::SendAlarmListSnmpTrap(deletedAlarmVector, true, (int)rmxStatus);
  }
}


void CMcuMngrManager::RecalculateMcuState()
{
	// ===== 1. get worst process state&type
	m_worstProcessSate = m_pProcessesStatesMap->GetWorstProcessState(m_worstProcessType);

	// ===== 2. set MCU state
	eMcuState mcuCurState = m_pProcess->GetMcuStateObject()->GetMcuState(),
	          mcuNewState = SetMcuStateAccordingToWorstProcessState();


	// ===== 3. update LEDs status
    if ( (mcuCurState != mcuNewState) )
        {
        if (YES == IsTarget() || (eProductTypeNinja == CProcessBase::GetProcess()->GetProductType()))
        {
            ChangeLedsState(mcuCurState, mcuNewState);
        }        
    }


	// ===== 4. handle actions of startup-over
	if ( (eMcuState_Startup == mcuCurState) && (eMcuState_Startup != mcuNewState) )
	{
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::RecalculateMcuState"
							   << "\n----- The system got out of startup (to  " << GetMcuStateName(mcuNewState) << ") -----";

		CManagerApi api(eProcessExchangeModule);
		api.SendOpcodeMsg(MCUMNGR_TO_EXCHANGE_STARTUP_END);
		//DeleteTimer(MCUMNGR_VM_IP_READY_TIMER);
		// ===== 5. system is after version_installation
		//if ( STARTUP_TIME_LIMIT_AFTER_INSTALLATION == m_pProcess->GetMaxTimeForStartup() )
		BOOL isSystemAfterVersionInstall = IsFileExists(SYSTEM_AFTER_VERSION_INSTALLED_FILE);
		if (TRUE == isSystemAfterVersionInstall)
		{
			RemoveActiveAlarmByErrorCode(AA_INSTALLING_NEW_VERSION);
			DeleteFile(SYSTEM_AFTER_VERSION_INSTALLED_FILE);

			TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::RecalculateMcuState - delete file "
			                       << SYSTEM_AFTER_VERSION_INSTALLED_FILE;
		}

	} // end system got out of startup

	else
	{
		TRACESTR(eLevelInfoNormal) << "System no out of startup " << (int)mcuCurState << " mcuNewState " << (int)mcuNewState;

	}
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendSystemIsOutOfStartupToFailoverProcess()
{
    BOOL isJITCMode = NO;
    CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
    sysConfig->GetBOOLDataByKey(CFG_KEY_JITC_MODE, isJITCMode);
	if (isJITCMode)    // in JitcMode Failover proccess is not functional
		return;

	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendSystemIsOutOfStartupToFailoverProcess";

	STATUS res = STATUS_OK;

	// ===== 1. ipAddress
	DWORD ipAddress =  0;
	const CIPSpan *pFirstSpan = m_pMngmntIpParams_asIpService_fromProcess->GetSpanByIdx(0); // Control params are stored in the 1st span (idx==0)
	if (pFirstSpan)
	{
		ipAddress = pFirstSpan->GetIPv4Address();
	}

	// ===== 2. version number
	VERSION_S mcuVersion = m_pSystemVersions->GetMcuVersion();

	// ===== 3. defaultGwIp
	DWORD defaultGwIp = m_pMngmntIpParams_asIpService_fromProcess->GetDefaultGatewayIPv4();

	/*mcuVersion.ver_internal=2;
	mcuVersion.ver_major=3;
	mcuVersion.ver_minor=4;
	mcuVersion.ver_release=5;*/
	// ===== 4. prepare segment
	 eIpType ipType  = m_pMngmntIpParams_asIpService_fromProcess->GetIpType();

	MASTER_SLAVE_DATA_S mngmntInfo;
	mngmntInfo.ipType	  = (DWORD) ipType;
    mngmntInfo.mngmntIpAddress	  = ipAddress;
    mngmntInfo.mcuVer			  = mcuVersion;
    mngmntInfo.defaultGwIpAddress = defaultGwIp;

    CSegment *pSeg = new CSegment;
    pSeg->Put((BYTE*)&mngmntInfo, sizeof(MASTER_SLAVE_DATA_S));

    // ==== 5. send to manager task
    CManagerApi apiFailover(eProcessFailover);
    apiFailover.SendMsg(pSeg, MCUMNGR_SYSTEM_IS_OUT_OF_STARTUP_IND);
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::AddFaultListToProcessStateMap(CProcessStateFaultList* pFaultsList)
{
	eProcessType procType = pFaultsList->GetProcessType();

	if ( eProcessTypeInvalid == procType )
	{
		PASSERT(1);
		return;
	}

	string entryKey = ProcessNames[procType];
	if(m_pProcessesStatesMap->IsEntryExist(entryKey))
	{
		m_pProcessesStatesMap->UpdateEntry(entryKey, pFaultsList);
	}
	else
	{
		m_pProcessesStatesMap->AddNewEntry(entryKey, pFaultsList);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::FindChangesInAlarms(CProcessStateFaultList *pNewFaultsList,
                                         vector<CLogFltElement>& outNewAlarmVector,
                                         vector<CLogFltElement>& outDeletedAlarmVector)
{
    eProcessType procType = pNewFaultsList->GetProcessType();

	if ( eProcessTypeInvalid == procType )
	{
        PASSERTSTREAM(TRUE, procType);
		return;
	}

	string entryKey = ProcessNames[procType];
    CStateFaultList *pOldFaultList = m_pProcessesStatesMap->GetStateFaultList(entryKey);

    // looking for new alarms
    string description;
    CLogFltElement *pCurrentFault = pNewFaultsList->GetFirstFaultElement();
    while(NULL != pCurrentFault)
    {
        DWORD uniqueIndex = pCurrentFault->GetIndex();
        bool isFaultExist = (NULL != pOldFaultList
                             ?
                             pOldFaultList->GetFaultList()->IsFaultExistByUniqueIndex(uniqueIndex) : false);
        if(!isFaultExist)
        {
            outNewAlarmVector.push_back(*pCurrentFault);
        }

        pCurrentFault = pNewFaultsList->GetNextFaultElement();
    }

    if(NULL == pOldFaultList)
    {
        return;
    }

    // looking for deleted alarms
    pCurrentFault = pOldFaultList->GetFirstFaultElement();
    while(NULL != pCurrentFault)
    {
        DWORD uniqueIndex = pCurrentFault->GetIndex();
        bool isFaultExist = pNewFaultsList->GetFaultList()->IsFaultExistByUniqueIndex(uniqueIndex);
        if(!isFaultExist)
        {
            outDeletedAlarmVector.push_back(*pCurrentFault);
        }

        pCurrentFault = pOldFaultList->GetNextFaultElement();
    }
}


/////////////////////////////////////////////////////////////////////////////
eMcuState CMcuMngrManager::SetMcuStateAccordingToWorstProcessState()
{
	eMcuState newMcuState = eMcuState_Invalid;


	// ===== 1. convert worstProcessSate into mcuState
	switch (m_worstProcessSate)
	{
		case eProcessInvalid:
		{
			newMcuState = eMcuState_Invalid;
			break;
		}

		case eProcessNormal:
		{
			newMcuState = eMcuState_Normal;
			break;
		}

		case eProcessIdle:
		case eProcessStartup:
		{

            const eMcuState currentMcuState = m_pProcess->GetMcuStateObject()->GetMcuState();


            if(eMcuState_Startup == currentMcuState || eMcuState_Invalid == currentMcuState)
            {

                newMcuState = eMcuState_Startup;


            }
            else
            {
                // 10.01.08: we don't want to put McuState in Major on processes' recoveries
                newMcuState = currentMcuState; //eMcuState_Major;

            }
// 			if (NO == m_isStartupTimoutReached)		// if startupTimeout has reached, then the system cannot be in Startup state anymore
// 				newMcuState = eMcuState_Startup;    //   even if there is a process that has got stuck in startup. in such a case, the syatem state will become Major
//             else
// 				newMcuState = eMcuState_Major;
			break;
		}

		case eProcessMinor:
		{
			newMcuState = eMcuState_Minor;
			break;
		}

		case eProcessMajor:
		{

			newMcuState = eMcuState_Major;
			break;
		}

		default:
		{
			PASSERT(m_worstProcessSate);
			newMcuState = eMcuState_Invalid;
		}
	}

	// ===== 2. set McuState on process level with the new McuState
    SetSecureMcuState(newMcuState);

  	return newMcuState;
}

/////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::HandleTerminalChangeLedState(CTerminalCommand & command, std::ostream& answer)
{
	DWORD numOfParams = command.GetNumOfParams();
	if(2 != numOfParams)
	{
		answer << "error: Illegal number of parameters\n";
		answer << "usage: change_led [red|green|amber] [on|off|flickering]";
		return STATUS_FAIL;
	}

    eLedColor color = (eLedColor) -1;
    eLedState state = (eLedState) -1;


    if (command.GetToken(eCmdParam1)== "red")
        color = eRed;
    if (command.GetToken(eCmdParam1)== "green")
        color = eGreen;
    if (command.GetToken(eCmdParam1)== "amber")
        color = eAmber;


    if (command.GetToken(eCmdParam2)== "off")
        state = eTurnOff;
    if (command.GetToken(eCmdParam2)== "on")
        state = eTurnOn;
    if (command.GetToken(eCmdParam2)== "flickering")
        state = eFlickering;

    CIPMCInterfaceApi ipmcApi;
    ipmcApi.ChangeLedState(color,state);

    return STATUS_OK;


}


/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::ChangeLedsState(eMcuState oldMcuState, eMcuState newMcuState)
{
	CMedString retStr = "CMcuMngrManager::ChangeLedsState - ";
	retStr << "oldMcuStatus: " << GetMcuStateName(oldMcuState)
           << ", newMcuStatus: " << GetMcuStateName(newMcuState);

	CIPMCInterfaceApi ipmcApi;

    if ( (eProductTypeNinja == CProcessBase::GetProcess()->GetProductType())
        && (((eMcuState_Startup != oldMcuState) && (eMcuState_Startup == newMcuState))
            || ((eMcuState_Major != oldMcuState) && (eMcuState_Major == newMcuState))))
    {
        std::string cmd;
        std::string answer;
        cmd = "sudo "+MCU_MCMS_DIR+"/Bin/LightCli HotStandby single_mode";
        SystemPipedCommand(cmd.c_str(),answer);
        retStr << "\n LED changed for Ninja single_mode: M/S Green turn on.";
        TRACESTR(eLevelInfoNormal) << retStr.GetString();
        return;
    }
	// ===== 1. turn 'Active' LED off when startup is finished (and from now, ConfParty handles this LED)
	if (eMcuState_Startup == oldMcuState)
	{
		ipmcApi.ChangeLedState(eAmber, eTurnOff);
		retStr << "\nLED changed: Amber turned off";
	}

	// ===== 2.  treat 'Major' and 'Normal' LEDs
	if ( (eMcuState_Normal == newMcuState) || (eMcuState_Minor == newMcuState) )
	{
		ipmcApi.ChangeLedState(eRed,   eTurnOff);
		ipmcApi.ChangeLedState(eGreen, eTurnOn);

		retStr << "\nLED changed: Red   turned off";
		retStr << "\nLED changed: Green turned on";
	}

	else if ( (eMcuState_Major == newMcuState) || (eMcuState_Critical == newMcuState) )
	{
		ipmcApi.ChangeLedState(eRed,   eTurnOn);
		ipmcApi.ChangeLedState(eGreen, eTurnOff);

		retStr << "\nLED changed: Red   turned on";
		retStr << "\nLED changed: Green turned off";
	}

	else if (eMcuState_Startup == newMcuState)
	{
		ipmcApi.ChangeLedState(eRed,   eFlickering);
		ipmcApi.ChangeLedState(eAmber, eFlickering);
		ipmcApi.ChangeLedState(eGreen, eFlickering);

		retStr << "\nLED changed: Red   flicker";
		retStr << "\nLED changed: Amber flicker";
		retStr << "\nLED changed: Green flicker";
	}

	TRACESTR(eLevelInfoNormal) << retStr.GetString();
}

/////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::HandleTerminalDisableWhiteList(CTerminalCommand & command, std::ostream& answer)
{
	 CConfigManagerApi api;
	 return api.DisableWhiteList();
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::HandleTerminalGetActiveAlarms(CTerminalCommand & command, std::ostream& answer)
{

	if ((NULL != m_pProcess) && (NULL != m_pProcess->GetMcuStateObject()))
	{
		if (0 < m_pProcess->GetMcuStateObject()->GetNumOfActiveAlarms())
		{
			answer << "ACTIVEALARMS\n";
		}
		else
		{

			answer << "ACTIVENOALARMS\n";
		}

		return STATUS_OK;
	}

	answer << "ACTIVENOALARMS\n";
	return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::UpdateActiveAlarmsList_inProcess()
{
	STATUS retStatus = STATUS_OK;
	std::string cmd;

	// ===== 1. clean original activeAlarms list
	m_pActiveAlarmList_fromProcess->Clear();

	// ===== 2. loop over FaultLists in map, creating a new activeAlarms list
	CStateFaultList *pTmpStateFaultlist = m_pProcessesStatesMap->GetFirstStateFaultList();
	while( NULL != pTmpStateFaultlist )
	{
		// get current FaultsList
		CFaultList* pCurList = pTmpStateFaultlist->GetFaultList();

		// add to activeAlarms list
		if (pCurList)
		{
			CFaultElementList *pFaultElementList = pCurList->GetFaultElementList();
			m_pActiveAlarmList_fromProcess->ConcatFaultList(pFaultElementList, true);
		}

		pTmpStateFaultlist = m_pProcessesStatesMap->GetNextStateFaultList();
	}

	// ===== 3. update size of activeAlarms list in McuStateObject (on process level)
	WORD numOfActiveAlarms = m_pProcess->GetActiveAlarmsList()->GetSize();
	m_pProcess->GetMcuStateObject()->SetNumOfActiveAlarms(numOfActiveAlarms);

	/* Floar added for LED&LCD Feature */
	std::string answer;
	eProductType procType = CProcessBase::GetProcess()->GetProductType();
	if ((procType == eProductTypeNinja) || (procType == eProductTypeGesher))
	{
		if ( numOfActiveAlarms >= 1 )
		{
			cmd = "sudo "+MCU_MCMS_DIR+"/Bin/LightCli sysAlarm add_alarm";
			SystemPipedCommand(cmd.c_str(),answer);
		}

		if (numOfActiveAlarms == 0)
		{
			cmd = "sudo "+MCU_MCMS_DIR+"/Bin/LightCli sysAlarm del_alarm";
			SystemPipedCommand(cmd.c_str(),answer);
		}
	}

	m_pActiveAlarmList_fromProcess->IncreaseUpdateCounter();

	// AA list has changed -> notify NotificationMngr
	SendAAListToNotificationMngr();

	return retStatus;
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendLicensingParamsToProcesses()
{
	SendLicensingParamsToRsrcAlloc();
	SendLicensingParamsToConfParty();
    SendLicensingParamsToGK();
    SendLicensingParamsToRtmIsdnMngr();
    SendLicensingParamsToApacheModuleMngr();
    SendLicensingParamsToCards();
    SendLicensingParamsToLogger();
}
/////////////////////////////////////////////////////////////////////////////

void CMcuMngrManager::SendMultipleServicesToProcesses()
{
	SendMultipleServicesToAuthentication();
	SendMultipleServicesToCSManager();
	SendMultipleServicesToApache();
	SendMultipleServicesToRsrcAlloc();
	SendMultipleServicesToCertMngr();
	SendMultipleServicesToSipProxy();
}

/////////////////////////////////////////////////////////////////////////////

void CMcuMngrManager::SendMultipleServicesToApache()
{
	CSegment* pSeg = new CSegment();

	BYTE bMultipleServices = GetMultipleServices();
	BYTE bV35JITCSupport = GetV35JITCSupport();

	CSegment  v35Seg;
	WORD serviceV35Counter =  (WORD)m_apacheConfig.m_v35GwList.size();
	if(m_apacheConfig.m_v35GwList.size() > 0)
	{
		list<RvgwItem>::const_iterator it=m_apacheConfig.m_v35GwList.begin();
		for(;it != m_apacheConfig.m_v35GwList.end();it++)
		{
		  	RvgwItem item = (*it);
		  	v35Seg << item.m_sslport<<item.m_userName << item.m_password;
		}
	}

	*pSeg << bMultipleServices
    		  << bV35JITCSupport <<serviceV35Counter ;

    if(m_apacheConfig.m_v35GwList.size() > 0)
    {
    	*pSeg << v35Seg;
    }

    CManagerApi apiApache(eProcessApacheModule);
    apiApache.SendMsg(pSeg, MCUMNGR_TO_APACHE_MULTIPLE_SERVICES_IND);

    TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendMultipleServicesToApache"
	                       << "\nMultiple services: :               " << (YES == bMultipleServices ? "YES" : "NO");

}
/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendMultipleServicesToCertMngr()
{
	CSegment* pSeg = new CSegment();

	BYTE bMultipleServices = GetMultipleServices();

	*pSeg << bMultipleServices;

    CManagerApi apiRsrc(eProcessCertMngr);
    apiRsrc.SendMsg(pSeg, MCUMNGR_TO_CERTMNGR_MULTIPLE_SERVICES_IND);

    TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendMultipleServicesToCertMngr"
	                       << "\nMultiple services: :               " << (YES == bMultipleServices ? "YES" : "NO");

}
void CMcuMngrManager::SendMultipleServicesToSipProxy()
{
	for( int i = 1; i <= MAX_SERVICES_NUM; i++ )
	{
		CSegment* pSeg = new CSegment();
		BYTE bMultipleServices = GetMultipleServices();
		*pSeg << bMultipleServices;
		CSipProxyTaskApi api(i);
		api.SendMsg(pSeg, MCUMNGR_TO_SIPPROXY_MULTIPLE_SERVICES_IND);
		//CManagerApi apiSipProxy(eProcessSipProxy);
		//apiSipProxy.SendMsg(pSeg, MCUMNGR_TO_SIPPROXY_MULTIPLE_SERVICES_IND);
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendMultipleServicesToSipProxy"
	                       << "\nMultiple services: :               " << (YES == bMultipleServices ? "YES" : "NO");
	}


}

/////////////////////////////////////////////////////////////////////////////
BYTE CMcuMngrManager::GetMultipleServices()
{
	BOOL isMultipleServices = NO;
	std::string key_multiple_services = CFG_KEY_MULTIPLE_SERVICES;
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	sysConfig->GetBOOLDataByKey(key_multiple_services, isMultipleServices);

	if (YES == isMultipleServices && TRUE == m_pLicensing->GetIsMultipleServicesEnabled())
		return YES;

	return NO;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CMcuMngrManager::GetV35JITCSupport()
{
	BOOL isV35JITCSupport = NO;
	std::string key_multiple_services = CFG_KEY_V35_ULTRA_SECURED_SUPPORT;
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	sysConfig->GetBOOLDataByKey(key_multiple_services, isV35JITCSupport);

	BOOL isNetworkSeparation = NO;
	key_multiple_services = CFG_KEY_SEPARATE_NETWORK;
	sysConfig->GetBOOLDataByKey(key_multiple_services, isNetworkSeparation);

	return isNetworkSeparation && isV35JITCSupport;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CMcuMngrManager::GetSeparateNetworksSupport()
{
	BOOL isNetworkSeparation = NO;
	std::string key_services = CFG_KEY_SEPARATE_NETWORK;
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	sysConfig->GetBOOLDataByKey(key_services, isNetworkSeparation);



	return isNetworkSeparation ;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CMcuMngrManager::GetRtmLanSupport()
{
	BOOL isRtmLan = NO;
	std::string key_services = CFG_KEY_RMX2000_RTM_LAN;
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	sysConfig->GetBOOLDataByKey(key_services, isRtmLan);



	return isRtmLan ;
}

void CMcuMngrManager::SendMultipleServicesToAuthentication()
{
	CSegment* pSeg = new CSegment();

	BYTE bMultipleServices = GetMultipleServices();

	*pSeg << bMultipleServices;

    CManagerApi apiAuthentication(eProcessAuthentication);
    apiAuthentication.SendMsg(pSeg, MCUMNGR_TO_AUTHENTICATION_MULTIPLE_SERVICES_IND);

    TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendMultipleServicesToAuthentication"
	                       << "\nMultiple services: :               " << (YES == bMultipleServices ? "YES" : "NO");

}

void CMcuMngrManager::SendMultipleServicesToCSManager()
{
	CSegment* pSeg = new CSegment();

	BYTE bMultipleServices = GetMultipleServices();

	*pSeg << bMultipleServices;

    CManagerApi apiCSMngr(eProcessCSMngr);
    apiCSMngr.SendMsg(pSeg, MCUMNGR_TO_CSMNGR_MULTIPLE_SERVICES_IND);

    TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendMultipleServicesToCSManager"
	                       << "\nMultiple services: :               " << (YES == bMultipleServices ? "YES" : "NO");
}

void CMcuMngrManager::SendMultipleServicesToRsrcAlloc()
{
	CSegment* pSeg = new CSegment();

	BYTE bMultipleServices = GetMultipleServices();

	*pSeg << bMultipleServices;

    CManagerApi apiRsrc(eProcessResource);
    apiRsrc.SendMsg(pSeg, MCUMNGR_TO_RSRC_MULTIPLE_SERVICES_IND);

    TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendMultipleServicesToRsrcAlloc"
	                       << "\nMultiple services: :               " << (YES == bMultipleServices ? "YES" : "NO");

}


void CMcuMngrManager::SendLicensingParamsToRsrcAlloc()
{

    RSRCALLOC_LICENSING_S param;
    memset(&param, 0, sizeof(RSRCALLOC_LICENSING_S));

    //flexera license
    if ((m_mcuMngrProductType == eProductTypeEdgeAxis) &&  m_pProcess->IsFlexeraLicenseInSysFlag() == true)
    {
    	DWORD machineCapacity = CalculateMachineCapacity(); //need to calculate the  machin capacity;
    	if (m_pProcess->IsFlexeraCapabilityEnabled(RPP_PKG))
    		param.num_cp_parties  = machineCapacity;
    	else
    	{
    		if (m_pProcess->IsFlexeraSimulationMode())
    		{
    			param.num_cp_parties  = (machineCapacity < m_pProcess->GetFlexeraCapabilityCounted(RPCS_MAX_PORTS))? machineCapacity : m_pProcess->GetFlexeraCapabilityCounted(RPCS_MAX_PORTS);

    		}
    		else
    			param.num_cp_parties  = m_pProcess->GetFlexeraCapabilityCounted(RPCS_MAX_PORTS);
    	}
    	param.isHD            = m_pProcess->IsFlexeraCapabilityEnabled(AVC_CIF_PLUS);
    	param.isSvcEnabled    = m_pProcess->IsFlexeraCapabilityEnabled(RPCS_SVC);
    	param.isHdPortsUnit   = TRUE;
    	param.num_svc_parties = 3*param.num_cp_parties;


    	if (m_pProcess->IsFlexeraCapabilityEnabled(RPCS) == false)
        		param.isLicenseExpired = true;
    	else
    		    param.isLicenseExpired = false;

    	param.isRPPModeEnabled = m_pProcess->IsFlexeraCapabilityEnabled(RPP_PKG);
    }
    else
    {
    	param.num_cp_parties   = m_pLicensing->GetTotalNumOfCpParties();
    	param.num_cop_parties  = m_pLicensing->GetTotalNumOfCopParties();
    	param.isHD             = m_pLicensing->GetIsHDEnabled();
    	param.isSvcEnabled     = m_pLicensing->GetIsSvcEnabled();
    	param.isHdPortsUnit    = m_pLicensing->GetIsHdPortsUnit();
    	param.num_svc_parties  = m_pLicensing->GetTotalNumOfSvcParties();
    	param.isLicenseExpired = FALSE;
    	param.isRPPModeEnabled = FALSE;
    }

	param.productType     = (DWORD)m_mcuMngrProductType;

	CSegment*  pRetParam = new CSegment();
    pRetParam->Put((BYTE*)&param, sizeof(RSRCALLOC_LICENSING_S));

    CManagerApi api(eProcessResource);
    STATUS res = api.SendMsg(pRetParam, MCUMNGR_TO_RSRCALLOC_LICENSING_IND);

    TRACESTR(eLevelInfoNormal)	<< "\nCMcuMngrManager::SendLicensingParamsToRsrcAlloc"
    							<< "\nNum of CP ports:      " << param.num_cp_parties
    							<< "\nNum of COP ports:     " << param.num_cop_parties
    							<< "\nProduct type:         " << ::ProductTypeToString( (eProductType)(param.productType) )
    							<< "\nSVC enabled:          " << ((m_pLicensing->GetIsSvcEnabled()==YES) ? "yes" : "no")
    							<< "\nSend Status :         " << m_pProcess->GetStatusAsString(res).c_str()
    							<< "\nisRPPLicense:         " << (DWORD)param.isRPPModeEnabled;
}

void CMcuMngrManager::SendLicensingParamsToConfParty()
{
	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::SendLicensingParamsToConfParty";

	// ===== 1. insert numOfPorts to a segment
    CONFPARTY_LICENSING_S params;
    memset(&params, 0, sizeof(CONFPARTY_LICENSING_S));

    if ((m_mcuMngrProductType == eProductTypeEdgeAxis) &&  m_pProcess->IsFlexeraLicenseInSysFlag() == true)
    {
    	params.confPartyLicensing_pstn				= FALSE;
    	params.confPartyLicensing_encryption		= m_pProcess->IsFlexeraCapabilityEnabled(MEDIA_ENCRYPTION);
    	params.confPartyLicensing_telepresence		= m_pProcess->IsFlexeraCapabilityEnabled(RPCS_TELEPRESENCE);
    	params.confPartyLicensing_ms			    = FALSE;


    	/*The HD license should always be set to TRUE, except for the case that CIF_PLUS license is FALSE.
          In this case the HD license should also be false, since we are limiting the resolution to CIF
    	  */

    	params.confPartyLicensing_HD                = m_pProcess->IsFlexeraCapabilityEnabled(AVC_CIF_PLUS);
    	params.confPartyLicensing_partner_Avaya	    = m_pProcess->IsFlexeraCapabilityEnabled(RPCS_AVAYA);
    	params.confPartyLicensing_partner_IBM       = m_pProcess->IsFlexeraCapabilityEnabled(RPCS_IBM);

    	DWORD machineCapacity = CalculateMachineCapacity(); //need to calculate the  machin capacity;
    	if (m_pProcess->IsFlexeraCapabilityEnabled(RPP_PKG))
    		params.num_cp_parties  = machineCapacity;
    	else
    	{
    		if (m_pProcess->IsFlexeraSimulationMode())
    		{
    			params.num_cp_parties  = (machineCapacity < m_pProcess->GetFlexeraCapabilityCounted(RPCS_MAX_PORTS))? machineCapacity : m_pProcess->GetFlexeraCapabilityCounted(RPCS_MAX_PORTS);

    		}
    		else
    			params.num_cp_parties  = m_pProcess->GetFlexeraCapabilityCounted(RPCS_MAX_PORTS);
    	}



    	params.confPartyLicensing_svc               = m_pProcess->IsFlexeraCapabilityEnabled(RPCS_SVC);
    	params.confPartyLicensing_CIF_Plus          = m_pProcess->IsFlexeraCapabilityEnabled(AVC_CIF_PLUS);
    	params.confPartyLicensing_TipInterop        = m_pProcess->IsFlexeraCapabilityEnabled(TIP);


    	if (m_pProcess->IsFlexeraCapabilityEnabled(RPCS) == false)

    		params.isLicenseExpired = true;
    	else
    		params.isLicenseExpired = false;


    }
    else
    {


    	params.confPartyLicensing_encryption		= m_pLicensing->GetIsEncryptionEnabled();
    	params.confPartyLicensing_pstn				= m_pLicensing->GetIsPstnEnabled();
    	params.confPartyLicensing_telepresence		= m_pLicensing->GetIsTelepresenceEnabled();
    	params.confPartyLicensing_ms				= FALSE;
    	params.confPartyLicensing_HD	            = m_pLicensing->GetIsHDEnabled();
    	params.confPartyLicensing_partner_Avaya		= m_pLicensing->GetIsAvaya();
    	//params.confPartyLicensing_partner_Ericsson	= m_pLicensing->GetIsEricsson();
    	//params.confPartyLicensing_partner_Microsoft	= m_pLicensing->GetIsMicrosoft();
    	//params.confPartyLicensing_partner_Nortel	= m_pLicensing->GetIsNortel();
    	params.confPartyLicensing_partner_IBM		= m_pLicensing->GetIsIBM();
    	params.num_cp_parties					  	= m_pLicensing->GetTotalNumOfCpParties();
    	params.num_cop_parties		  				= m_pLicensing->GetTotalNumOfCopParties();
    	params.confPartyLicensing_svc    			= m_pLicensing->GetIsSvcEnabled();
    	params.confPartyLicensing_CIF_Plus          = TRUE;
    	params.confPartyLicensing_TipInterop        = TRUE;
    	params.isLicenseExpired                     = FALSE;
    }

	CSegment*  pRetParam = new CSegment();
    pRetParam->Put((BYTE*)&params,sizeof(CONFPARTY_LICENSING_S));

    CManagerApi api(eProcessConfParty);
    STATUS res = api.SendMsg(pRetParam, MCUMNGR_TO_CONFPARTY_LICENSING_IND);

    TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendLicensingParamsToConfParty\n"
                           << "Send Status : " << m_pProcess->GetStatusAsString(res).c_str();
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendLicensingParamsToGK()
{
    GK_LICENSING_S param;
    memset(&param, 0, sizeof(GK_LICENSING_S));

    if  ((m_mcuMngrProductType == eProductTypeEdgeAxis) &&  m_pProcess->IsFlexeraLicenseInSysFlag() == true)
    	param.partner_Avaya = m_pProcess->IsFlexeraCapabilityEnabled(RPCS_AVAYA);
    else
    	param.partner_Avaya = m_pLicensing->GetIsAvaya();

    STATUS status = STATUS_OK;
    for( int i = 1; i <= MAX_SERVICES_NUM; i++ )
    {
        CSegment *pSeg = new CSegment;
        pSeg->Put((BYTE*)&param,sizeof(GK_LICENSING_S));

        //CManagerApi api(eProcessGatekeeper);
        CGatekeeperTaskApi api(i);
        status = api.SendMsg(pSeg, MCUMNGR_TO_GK_LICENSING_IND);
    }
    TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendLicensingParamsToGK"
                           << CGKLicensingWrapper(param)
                           << "\nStatus : " << CProcessBase::GetProcess()->GetStatusAsString(status).c_str();
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendLicensingParamsToRtmIsdnMngr()
{
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendLicensingParamsToRtmIsdnMngr ";

	// ===== 1. insert PSTN flag to the segment
	CSegment*  pParam = new CSegment();

	if ((m_mcuMngrProductType == eProductTypeEdgeAxis) &&  m_pProcess->IsFlexeraLicenseInSysFlag() == true)
	{
		*pParam << (BYTE)FALSE;
	}
	else
	{
		TRACESTR(eLevelInfoNormal) << "\nPSTN flag:" << (YES == m_pLicensing->GetIsPstnEnabled() ? "YES" : "NO");

		*pParam << m_pLicensing->GetIsPstnEnabled();
	}

	// ===== 2. send to RtmIsdnMngr
	const COsQueue* pRtmIsdnMngrMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessRtmIsdnMngr, eManager);

	STATUS res = pRtmIsdnMngrMbx->Send(pParam, MCUMNGR_TO_RTMISDN_LICENSING_IND);
}

typedef void (*GetAvcSvcCapFunc)(CLicensing const &, VERSION_S const & ver, AvcSvcCap & cap);

void GetAvcSvcCap_HWRMX(CLicensing const & licensing, VERSION_S const & ver, AvcSvcCap & cap)
{
    bzero(&cap, sizeof(cap));
    cap.cpPortCount = licensing.GetTotalNumOfCpParties();
    cap.supportAvcCp  = (cap.cpPortCount>0) ? TRUE : FALSE;
    cap.supportAvcVsw = (cap.cpPortCount>0) ? TRUE : FALSE;

    if ( 0 == ver.ver_major && 0 == ver.ver_minor ) // simulation
    {
        cap.supportSvc = licensing.GetIsSvcEnabled();
        cap.supportMixedCp = licensing.GetIsSvcEnabled();
    }
    else if (ver.ver_major<7 || ((7==ver.ver_major) && (ver.ver_minor<=8)))
    {
        cap.supportSvc = TRUE;
        cap.supportMixedCp = FALSE;
    }
    else
    {
        cap.supportSvc = licensing.GetIsSvcEnabled();
        cap.supportMixedCp = licensing.GetIsSvcEnabled();
    }
    cap.supportMixedVsw = FALSE;

    cap.supportTip = TRUE;
    cap.supportAvcCifPlus = TRUE;


    if (licensing.GetSystemCardsMode() == eSystemCardsMode_mpmrx)
    {
    	cap.maxLineRate = 6144;
    	cap.supportHighProfileContent = TRUE;
    }
    else
    {
        cap.maxLineRate = 4096;
        cap.supportHighProfileContent = FALSE;
    }

    //cap.supportCascadeAvc = TRUE;
      cap.supportCascadeSvc = FALSE;
    //cap.supportSrtpAvc = TRUE;
    //cap.supportSrtpSvc = FALSE;

    cap.supportItp = licensing.GetIsTelepresenceEnabled();
    cap.supportAudioOnlyConf = FALSE;

    cap.licenseMode = 2; //cfs

}

void GetAvcSvcCap_SoftMcu(CLicensing const & licensing, VERSION_S const & ver, AvcSvcCap & cap)
{
	(void)ver;
	bzero(&cap, sizeof(cap));
	cap.cpPortCount = licensing.GetTotalNumOfCpParties();
	cap.supportAvcCp  = (cap.cpPortCount>0) ? TRUE : FALSE;
	cap.supportAvcVsw = FALSE;  //supported only in HW RMX
	cap.supportSvc = TRUE; //licensing.GetIsSvcEnabled();
	cap.supportMixedCp = licensing.GetIsSvcEnabled();
	cap.supportMixedVsw = FALSE;

	cap.supportTip = TRUE;
	cap.supportAvcCifPlus = TRUE;

	cap.supportHighProfileContent = FALSE;   //HP content
	cap.maxLineRate = 1920;


    //cap.supportCascadeAvc = TRUE;
    cap.supportCascadeSvc = FALSE;
    //cap.supportSrtpAvc = TRUE;
    //cap.supportSrtpSvc = TRUE;

    cap.supportItp = licensing.GetIsTelepresenceEnabled();
    cap.supportAudioOnlyConf = FALSE;
    cap.licenseMode =(int) eLicenseMode_cfs;
}

void GetAvcSvcCap_SoftMcuEdge(CLicensing const & licensing, VERSION_S const & ver, AvcSvcCap & cap)
{
	(void)ver;
	bzero(&cap, sizeof(cap));
	cap.cpPortCount = licensing.GetTotalNumOfCpParties();
	cap.supportAvcCp  =  (cap.cpPortCount>0) ? TRUE : FALSE ; //if there is no ports - no AVC
	cap.supportAvcVsw = FALSE; //supported only in HW RMX
	cap.supportSvc = licensing.GetIsSvcEnabled();
	cap.supportMixedCp = licensing.GetIsSvcEnabled(); //if no svc no mixed
	cap.supportMixedVsw = FALSE;


	cap.supportTip = licensing.GetIsTipEnabled();
	cap.supportAvcCifPlus = licensing.GetIsAvcCifPlusEnabled();


	cap.supportHighProfileContent = TRUE;   //HP content
	cap.maxLineRate = 1920;

	//cap.supportCascadeAvc = TRUE;
	cap.supportCascadeSvc = FALSE;
	//cap.supportSrtpAvc = TRUE;
	//cap.supportSrtpSvc = TRUE;

	cap.supportItp = licensing.GetIsTelepresenceEnabled();
	cap.supportAudioOnlyConf = FALSE;

	cap.licenseMode = (int)licensing.GetLicenseMode();



}

void GetAvcSvcCap_SoftMcuCG(CLicensing const & licensing, VERSION_S const & ver, AvcSvcCap & cap)
{
	(void)ver;
	bzero(&cap, sizeof(cap));
	cap.cpPortCount = licensing.GetTotalNumOfCpParties();
	cap.supportAvcCp  = (cap.cpPortCount>0) ? TRUE : FALSE;
	cap.supportAvcVsw = FALSE;
	cap.supportSvc = TRUE; //licensing.GetIsSvcEnabled();
	cap.supportMixedCp = licensing.GetIsSvcEnabled();
	cap.supportMixedVsw = FALSE;

	cap.supportTip = TRUE;
	cap.supportAvcCifPlus = TRUE;

	cap.supportHighProfileContent = FALSE;
	cap.maxLineRate = 1920;

	//cap.supportCascadeAvc = TRUE;
    cap.supportCascadeSvc = FALSE;
    //cap.supportSrtpAvc = TRUE;
    //cap.supportSrtpSvc = TRUE;

    cap.supportItp = licensing.GetIsTelepresenceEnabled();
    cap.supportAudioOnlyConf = FALSE;

    cap.licenseMode = 2;

}


void GetAvcSvcCap_SoftMcuMfw(CLicensing const & licensing, VERSION_S const & ver, AvcSvcCap & cap)
{
    (void)ver;
    bzero(&cap, sizeof(cap));
    cap.cpPortCount = 30;
    cap.supportAvcCp = FALSE;
    cap.supportAvcVsw = FALSE;
    cap.supportSvc = TRUE;
    cap.supportMixedCp = FALSE;
    cap.supportMixedVsw = TRUE;

    cap.supportTip = FALSE;
    cap.supportAvcCifPlus = FALSE;

    cap.supportHighProfileContent = FALSE;   
    cap.maxLineRate = 1920;

    //cap.supportCascadeAvc = FALSE;
    cap.supportCascadeSvc = TRUE;
    //cap.supportSrtpAvc = TRUE;
    //cap.supportSrtpSvc = TRUE;

    cap.supportItp = FALSE;
    cap.supportAudioOnlyConf = TRUE;

    cap.licenseMode = 0;

}

void GetAvcSvcCap_Gesher(CLicensing const & licensing, VERSION_S const & ver, AvcSvcCap & cap)
{
    (void)ver;
    bzero(&cap, sizeof(cap));
    cap.cpPortCount = licensing.GetTotalNumOfCpParties();
    cap.supportAvcCp  = (cap.cpPortCount>0) ? TRUE : FALSE;
    cap.supportAvcVsw = FALSE;
    cap.supportSvc = licensing.GetIsSvcEnabled();
    cap.supportMixedCp = licensing.GetIsSvcEnabled();
    cap.supportMixedVsw = FALSE;

    cap.supportTip = TRUE;
    cap.supportAvcCifPlus = TRUE;

    cap.supportHighProfileContent = FALSE;
    cap.maxLineRate = 1920;

    //cap.supportCascadeAvc = TRUE;
    cap.supportCascadeSvc = FALSE;
    //cap.supportSrtpAvc = TRUE;
    //cap.supportSrtpSvc = TRUE;

    cap.supportItp = licensing.GetIsTelepresenceEnabled();
    cap.supportAudioOnlyConf = FALSE;

    cap.licenseMode = 2;

}

void GetAvcSvcCap_Ninja(CLicensing const & licensing, VERSION_S const & ver, AvcSvcCap & cap)
{
    (void)ver;
    bzero(&cap, sizeof(cap));
    cap.cpPortCount = licensing.GetTotalNumOfCpParties();
    cap.supportAvcCp  = (cap.cpPortCount>0) ? TRUE : FALSE;
    cap.supportAvcVsw = FALSE;
    cap.supportSvc = licensing.GetIsSvcEnabled();
    cap.supportMixedCp = licensing.GetIsSvcEnabled();
    cap.supportMixedVsw = FALSE;

    cap.supportTip = TRUE;
    cap.supportAvcCifPlus = TRUE;

    cap.supportHighProfileContent = TRUE;
    cap.maxLineRate = 6144;

    //cap.supportCascadeAvc = TRUE;
    cap.supportCascadeSvc = FALSE;
    //cap.supportSrtpAvc = TRUE;
    //cap.supportSrtpSvc = TRUE;

    cap.supportItp = licensing.GetIsTelepresenceEnabled();
    cap.supportAudioOnlyConf = FALSE;

    cap.licenseMode = 2;

}

namespace
{
    struct Product2GetAvcSvcCap
    {
        eProductType type;
        GetAvcSvcCapFunc func;
    } const s_prod2GetCapTbl[] =
    {
          { eProductTypeSoftMCU, GetAvcSvcCap_SoftMcu }
        , { eProductTypeSoftMCUMfw, GetAvcSvcCap_SoftMcuMfw }
        , { eProductTypeGesher, GetAvcSvcCap_Gesher }
        , { eProductTypeNinja, GetAvcSvcCap_Ninja }
        , { eProductTypeEdgeAxis, GetAvcSvcCap_SoftMcuEdge }
        , { eProductTypeCallGeneratorSoftMCU, GetAvcSvcCap_SoftMcuCG }
    };
}

GetAvcSvcCapFunc FindGetAvcSvcCapFunc(eProductType type)
{
    for (unsigned i=0; i<sizeof(s_prod2GetCapTbl)/sizeof(0[s_prod2GetCapTbl]); ++i)
    {
        if (s_prod2GetCapTbl[i].type==type)
        {
            return s_prod2GetCapTbl[i].func;
        }
    }

    return GetAvcSvcCap_HWRMX;
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendLicensingParamsToLogger()
{
    LOGGER_LICENSING_S param;
    memset(&param, 0, sizeof(LOGGER_LICENSING_S));
    param.num_cop_parties  	= m_pLicensing->GetTotalNumOfCopParties();
    param.num_cp_parties	= m_pLicensing->GetTotalNumOfCpParties();
    param.sys_card_mode		= m_mcuMngrRmxSystemCardsMode;

    CSegment*  pSeg = new CSegment();
    pSeg->Put((BYTE*)&param, sizeof(LOGGER_LICENSING_S));

    CManagerApi api(eProcessLogger);
    STATUS res = api.SendMsg(pSeg, LOGGER_LICENSING_IND);

    TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendLicensingParamsToLoggerMngr"
	                       << "\nNum of COP ports: " << param.num_cop_parties
	                       << "\nNum of CP ports : " << param.num_cp_parties
                           << "\nMedis card mode : " << m_mcuMngrRmxSystemCardsMode;
}


/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendLicensingParamsToApacheModuleMngr()
{
    APACHEMODULE_LICENSING_S param;
    memset(&param, 0, sizeof(APACHEMODULE_LICENSING_S));
    param.num_cop_parties		  	= m_pLicensing->GetTotalNumOfCopParties();
    param.num_cp_parties		  	= m_pLicensing->GetTotalNumOfCpParties();
    {
        GetAvcSvcCapFunc GetAvcSvcCap = FindGetAvcSvcCapFunc(m_mcuMngrProductType);
        (*GetAvcSvcCap)(*m_pLicensing, m_pSystemVersions->GetMcuVersion(), param.avcSvcCap);
        //The only HW RMX supporting HighProfile Content is MPM-Rx
        if (m_mcuMngrRmxSystemCardsMode == eSystemCardsMode_mpmrx)
            param.avcSvcCap.supportHighProfileContent = TRUE;
    }

    CSegment*  pSeg = new CSegment();
    pSeg->Put((BYTE*)&param, sizeof(APACHEMODULE_LICENSING_S));

    CManagerApi api(eProcessApacheModule);
    STATUS res = api.SendMsg(pSeg, MCUMNGR_TO_APACHEMODULE_LICENSING_IND);

    TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendLicensingParamsToApacheModuleMngr"
	                       << "\nNum of COP ports: " << param.num_cop_parties
	                       << "\nNum of CP ports:  " << param.num_cp_parties
	                       << "\nSupportTip:  "      << param.avcSvcCap.supportTip
	                       << "\nSupportSvc:  "      << param.avcSvcCap.supportSvc
	                       << "\nLicenseMode:  "      << param.avcSvcCap.licenseMode
                           << "\nSend Status :     " << m_pProcess->GetStatusAsString(res).c_str();
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendLicensingParamsToCards()
{
	// ===== 1. fill the segment with AuthenticationSuccess parameters
	CARDS_LICENSING_S *pLicensing = new CARDS_LICENSING_S;
	memset(pLicensing, 0, sizeof(CARDS_LICENSING_S));
	eProductType theType	= ConvertPlatformTypeToProductType((ePlatformType)(m_authentication.GetPlatformType()));

	pLicensing->authenticationStruct.productType		= (DWORD)theType;
	pLicensing->authenticationStruct.switchBoardId		= m_switchBoardId;
	pLicensing->authenticationStruct.switchSubBoardId	= m_switchSubBoardId;
	pLicensing->federal									= YES;


	if ((m_mcuMngrProductType == eProductTypeEdgeAxis) &&  m_pProcess->IsFlexeraLicenseInSysFlag() == true)
		pLicensing->multipleServices = m_pProcess->IsFlexeraCapabilityEnabled(RPCS_MULTIPLE_SERVICES);

	else
		pLicensing->multipleServices = (BOOL)m_pLicensing->GetIsMultipleServicesEnabled();

	CSegment*  pRetParam = new CSegment();
	pRetParam->Put( (BYTE*)pLicensing, sizeof(CARDS_LICENSING_S) );
	delete pLicensing;

	// ===== 2. send
	CManagerApi api(eProcessCards);
	STATUS res = api.SendMsg(pRetParam, MCUMNGR_TO_CARDS_LICENSING_IND);

	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendLicensingParamsToCards\n";
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendCfsParamsToInstaller()
{
	STATUS res = STATUS_OK;

	// ===== 1. prepare Cfs parameters
    INSTALLER_CFS_S *pCfsParams = new INSTALLER_CFS_S;
    memset(pCfsParams, 0, sizeof(INSTALLER_CFS_S));

    memcpy(pCfsParams->mplSerialNumber, m_authentication.GetSerialNumber(), MPL_SERIAL_NUM_LEN );
    pCfsParams->mplSerialNumber[MPL_SERIAL_NUM_LEN - 1] = '\0';
    pCfsParams->switchBoardId		= m_switchBoardId;
    pCfsParams->switchSubBoardId	= m_switchSubBoardId;

	VERSION_S verFromKc  = m_pSystemVersions->GetMcuVersion();
//	VERSION_S verFromKc = GetKeycodeVersionFrom_U_KeycodeOrFromFile();
    pCfsParams->verFromKeycode.ver_major	= verFromKc.ver_major;
    pCfsParams->verFromKeycode.ver_minor	= verFromKc.ver_minor;
    pCfsParams->verFromKeycode.ver_release	= verFromKc.ver_release;
    pCfsParams->verFromKeycode.ver_internal	= verFromKc.ver_internal;


	// ===== 2. print the data received
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendCfsParamsToInstaller: "
	                       << "\nMpl Serial number: " << pCfsParams->mplSerialNumber
	                       << ", switchBoardId: "     << pCfsParams->switchBoardId
	                       << ", switchSubBoardId: "  << pCfsParams->switchSubBoardId
                           << "\nVersion from keycode: " << pCfsParams->verFromKeycode.ver_major   << "."
                                                         << pCfsParams->verFromKeycode.ver_minor   << "."
                                                         << pCfsParams->verFromKeycode.ver_release << "."
                                                         << pCfsParams->verFromKeycode.ver_internal;


	// ===== 3. fill a segment
    CSegment*  pRetParam = new CSegment;
	pRetParam->Put( (BYTE*)pCfsParams, sizeof(INSTALLER_CFS_S) );

	delete pCfsParams;

	// ===== 4. send
	const COsQueue* pInstallerMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessInstaller, eManager);

	res = pInstallerMbx->Send(pRetParam,MCUMNGR_TO_INSTALLER_CFS_PARAMS_IND);
}



void CMcuMngrManager::ManagerPostInitActionsPoint()
{

	//BRIDGE-13450
	BOOL isSystemAfterVersionInstall = IsFileExists(SYSTEM_AFTER_VERSION_INSTALLED_FILE);
	if (TRUE == isSystemAfterVersionInstall)
		m_pProcess->SetMaxTimeForStartup(STARTUP_TIME_LIMIT_AFTER_INSTALLATION);



	TestAndEnterFipsMode(false);
	//===== 3. flicker all LEDs
	if (YES == IsTarget())
	{
		CIPMCInterfaceApi ipmcApi;
		ipmcApi.ChangeLedState(eRed, eFlickering);
		ipmcApi.ChangeLedState(eAmber, eFlickering);
		ipmcApi.ChangeLedState(eGreen, eFlickering);
		TRACEINTOFUNC << "\nLED changed: Red   flicker"
				<< "\nLED changed: Amber flicker"
				<< "\nLED changed: Green flicker";
	}

  eProductType curProductType = CProcessBase::GetProcess()->GetProductType();


  bool bIsFederal = IsFederalOn() ? true : false;

	// ===== 5. ask state of all processes (in case McuMngr evokes late)
	SendStateRequestToAllProcesses();

	// ===== 6. check if there's a patch in the version
	if (TRUE == IsPatchedVersion())
  {
	  TRACEINTO << "Version contains patch(es)";
      AddActiveAlarm(FAULT_GENERAL_SUBJECT,
                   AA_PATCHED_VERSION,
                   MAJOR_ERROR_LEVEL,
                   "The software contains patch(es)",
                   true, true);
  }


	// ===== 7. on Target -
  if (IsTarget())
	{
    // Checks if the version that runs is the version that is linked to 'current'
		CheckRunningVersionVsCurrent();

		// Checks DMA
		CheckDMA();

		 if (eProductTypeRMX4000 == m_mcuMngrProductType ||
		      eProductTypeRMX1500 == m_mcuMngrProductType ||
		      eProductTypeRMX2000 == m_mcuMngrProductType )

			  // CheckEth0Link
			 //on startup start a link check until MCMS get first msg from Shelf - authenticationInd msg.
			 StartCheckEth0Link();
	}

  // ---- 8c. econfig MANAGEMENT_NETWORK
  if (IsFileExists(RESTORE_CONFIG_SUCCESS))
  {
    m_bRestoreConfigSuccess = TRUE;
    DeleteFile(RESTORE_CONFIG_SUCCESS);
  }

  //printf("[debug]: InitNetworkInterface\n");
	// ===== 1. read from file to mngmntIpService
  CIPService tmpService;
  STATUS readFileStatus = tmpService.ReadXmlFile(MANAGEMENT_NETWORK_CONFIG_PATH, eNoActiveAlarm, eRenameFile);



  STATUS initNetworkResStatus =STATUS_OK;

   m_pProcess->m_NetSettings.LoadFromFile();
    initNetworkResStatus  = InitNetworkInterfaceWithMcmsNetwork(tmpService,readFileStatus);

	// ===== 9. read and config system time
	STATUS retStatus = m_pProcess->LoadMcuTime();
  if (STATUS_OK != retStatus)
  {
    const std::string& strStatus = m_pProcess->GetStatusAsString(retStatus);
    std::string description = "Failed to read MCU time configuration file (";
    description += strStatus;
    description += ")";

    AddActiveAlarm(FAULT_GENERAL_SUBJECT,
                   AA_NO_TIME_CONFIGURATION,
                   MAJOR_ERROR_LEVEL,
                   description,
                   true, true);
  }

	SendGmtOffset();

	// ===== 10. update proxy in httpd file
	std::string external_content_dir, external_content_ip;
	DWORD external_content_port;
	if (GetDataForProxy(external_content_dir, external_content_ip, external_content_port)==TRUE)
	{
		//UpdateProxyInHttpdFile(external_content_dir, external_content_ip, external_content_port);
		// SAGI: TODO
	}

  // ===== 11. Check if the System is running with debug flags
  CheckDebugMode();

  // ===== 12. Create visible Cfg params.
  CreateCfgVisibleParams();

  if (IsTarget())
  {
    // ===== 13. Check the build validity.
    const VERSION_S version = m_pSystemVersions->GetMcuVersion();
    if (0 == version.ver_major)
    {
      CLargeString message;
      message << "MCU Version : " << version.ver_major << "."
              << version.ver_minor << "." << version.ver_release << "."
              << version.ver_internal;

      CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT,
				  AA_ILLEGAL_MCU_VERSION,
				  MAJOR_ERROR_LEVEL,
				  message.GetString(),  FALSE);
    }

    // ===== 14. Check for private version
    if (m_pSystemVersions->GetMcuPrivateDescription())
    {
      std::string sPrivateDesc = m_pSystemVersions->GetMcuPrivateDescription();
      if (!sPrivateDesc.empty())
      {
        std::string privateVersionStr = "A private version is loaded: ";
        privateVersionStr += sPrivateDesc;

		CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT,
				AA_PRIVATE_VERSION,
				MAJOR_ERROR_LEVEL,
				privateVersionStr.c_str(),
                            FALSE);

      }
    }


  } // end if Target
  else
  {
      // Creates MediaRecording folder if needed
      if (!IsFileExists(MEDIA_RECORDING_FILE_PATH))
      {
            BOOL res = CreateDirectory(MEDIA_RECORDING_FILE_PATH.c_str());
            std::string strSharePath = MEDIA_RECORDING_FILE_PATH;
            strSharePath += "/share";
            res = CreateDirectory(strSharePath.c_str());
            if(!res)
            {
                TRACEINTO << "\nFailed to create dir : " << MEDIA_RECORDING_FILE_PATH;
            }
      }
  }


    // --- was copied from InitTask z"l
    CTaskApi snmpTaskApi;
    COsQueue dummyMbx;
    CreateTask(&snmpTaskApi, McuMngrSNMPTaskEntryPoint, &dummyMbx);

  DWORD timeToEndStartup = m_pProcess->GetMaxTimeForStartup();
  StartTimer(MCUMNGR_STARTUP_TIMER, timeToEndStartup + SECOND * 30);

  // Check if this startup was after restore defaults
  if (IsFileExists(RESTORE_FACTORY_PATH))
  {
    TRACEINTOFUNC << RESTORE_FACTORY_PATH
                  << " exists, system starts after restoring defaults";

    m_isRestoreDefaultsStartup = TRUE;
    if ( CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyRMX )
    {
       		DeleteFile(RESTORE_FACTORY_PATH);
    }
  }

  // ===== 16. Set Kernel TCP stack params
  SetKernelTCPParams();

  if (eProductTypeRMX4000 == m_mcuMngrProductType ||
      eProductTypeRMX1500 == m_mcuMngrProductType ||
      eProductTypeRMX2000 == m_mcuMngrProductType ||
      eProductTypeGesher == m_mcuMngrProductType || eProductTypeNinja == m_mcuMngrProductType)
  {
    // ===== 17. read Ethernet Settings file
    retStatus = m_pProcess->LoadEthernetSettingsConfig();
    if (STATUS_OK == retStatus)
    {
      CEthernetSettingsConfigList* pEthList =
          m_pProcess->GetEthernetSettingsConfigList();
      TRACEINTOFUNC << "Ethernet Settings configuration from file:\n"
                    << *pEthList;

      if (eProductTypeGesher == m_mcuMngrProductType || eProductTypeNinja == m_mcuMngrProductType)
      {
        CEthernetSettingsConfig *pEthernetSettingsConfig = NULL;
        ePortSpeedType portSpeed = ePortSpeed_Auto;
        eEthPortType portType = eEthPortType_Illegal;

        for (int i = 0; i < MAX_NUM_OF_LAN_PORTS; i++)
        {
          pEthernetSettingsConfig = pEthList->GetSpecEthernetSettingsConfig(i);
          portSpeed = pEthernetSettingsConfig->GetPortSpeed();
          portType = pEthernetSettingsConfig->GetPortType();

          if (portSpeed != ePortSpeed_Auto)
          {
            TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::ManagerPostInitActionsPoint"
                       << "\nSlot Id:   " << pEthernetSettingsConfig->GetSlotId()
                       << "\nPort Id:   " << pEthernetSettingsConfig->GetPortId()
                       << "\nPort Type: " << GetEthPortTypeStr(portType)
                       << "\nSpeed:     " << ::PortSpeedTypeToString(portSpeed);

            ConfigureEthernetSettingsSpec(pEthernetSettingsConfig);
          }
        }
      }
    }
    else // failed to read EthernetSettings from file
    {
      if (STATUS_FILE_NOT_EXIST == retStatus)
      {
        TRACEINTOFUNC << "Ethernet Settings configuration file does not exist";
      }
      else
      {
        // status != ok, but the file exists
        std::string description =
            "Failed to read Ethernet Settings configuration file (";
        description += m_pProcess->GetStatusAsString(retStatus);
        description += ")";
        PASSERTMSG(TRUE, description.c_str());
      }
    }
  }

        // ===== 18. ask CsMngr for V35 Gateway params (for configuring Apache)
  SendV35GwParamsReqToCsMngr();
   //config Whitelist
  if(m_pMngmntIpParams_asIpService_fromProcess && m_pMngmntIpParams_asIpService_fromProcess->GetWhiteList()->IsWhiteListEnable())
  {
	  CConfigManagerApi api;
	  CSegment  *pIpvSeg = new CSegment;
	  m_pMngmntIpParams_asIpService_fromProcess->GetWhiteList()->WriteWhiteListToSegment(pIpvSeg);
	  api.EnableWhiteList(pIpvSeg);
  }
	// ===== 19. start startup_time timer
  int remainingStartupTime = (int)(m_pProcess->GetMaxTimeForStartup());
  m_pProcess->GetMcuStateObject()->SetRemainingTimeForStartup(remainingStartupTime);
  StartTimer(SYSTEM_STARTUP_REMAINING_TIME_TIMER, SECOND);

  if (m_pProcess->GetCSLogsStateFromSysCfg())
	  m_bCSLogStarted = TRUE;
  // Produces Startup Fault
  CHlogApi::SystemStartup(bIsFederal);

  //Send ready to SNMPProcess
  CSegment* seg = new CSegment;
  *seg << static_cast<unsigned int>(eProcessMcuMngr);
  CManagerApi(eProcessSNMPProcess).SendMsg(seg, SNMP_OTHER_PROCESS_READY);

  P802_1xManagerInit();
	//Load PrecedenceSettings
	CPrecedenceSettings pPrecedenceSettings;
	int status = pPrecedenceSettings.ReadXmlFile();
	if (status != STATUS_OK)
	{
		//if file doesn't exist then write default values
		pPrecedenceSettings.WriteXmlFile();
	}
	m_pProcess->SetPrecedenceSettings(&pPrecedenceSettings);

	//Send Precedence Settings to CsMngr (CSMngr is up before McuMngr)
	SendPrecedenceSettingsToProcess(&pPrecedenceSettings, eProcessCSMngr);

	if(eProductFamilySoftMcu == CProcessBase::GetProcess()->GetProductFamily())
	{
		//Get the system based mode from Cards
		SendSystemBasedModeReqToCardMngr();
	}

	if ((m_mcuMngrProductType == eProductTypeEdgeAxis) &&  m_pProcess->IsFlexeraLicenseInSysFlag() == true)
	    InitLicenseServerParamsFromFile();


}


////////////////////////////////////////////////////////////////////////
BYTE CMcuMngrManager::GetDataForProxy(std::string &external_content_dir,
                                      std::string &external_content_ip,
                                      DWORD &external_content_port)
{
  TRACECOND_AND_RETURN_VALUE(IsFederalOn(),
      "EXTERNAL_CONTENT_DIRECTORY is not supported in Ultra Secure Mode",
      FALSE);

	CSysConfig* pSysConfig = m_pProcess->GetSysConfig();

	BOOL bProxy = pSysConfig->GetDataByKey(CFG_KEY_EXTERNAL_CONTENT_DIRECTORY,
                                         external_content_dir);

	// We must have a value different then the root. the root is saved for EMA web site
	if (external_content_dir.length() == 0)
	{
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::GetDataForProxy - CFG_KEY_EXTERNAL_CONTENT_DIRECTORY is empty, and can't be saved in apache configuration file";
		return FALSE;
	}

	BOOL bProxyIp=FALSE, bProxyPort= FALSE;
	if (bProxy==TRUE)
	{
		bProxyIp	= pSysConfig->GetDataByKey(CFG_KEY_EXTERNAL_CONTENT_IP, external_content_ip);
		bProxyPort	= pSysConfig->GetDWORDDataByKey(CFG_KEY_EXTERNAL_CONTENT_PORT, external_content_port);

		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::GetDataForProxy"
							   << "\nIP:   " << external_content_ip
							   << "\nPort: " << external_content_port;
	}

	if ( (FALSE == bProxyIp) || (FALSE == bProxyPort) )	// no need to update proxy parameters
		return FALSE;

	if (external_content_ip.length()==0)		//can't update apache if ip is empty
	{
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::GetDataForProxy - CFG_KEY_EXTERNAL_CONTENT_IP is empty, and can't be saved in apache configuration file";
		return FALSE;
	}

	return TRUE;
}

void CMcuMngrManager::CheckDebugMode()
{
    CSysConfig *sysConfig = m_pProcess->GetSysConfig();
    BOOL isDebugMode = NO;
    sysConfig->GetBOOLDataByKey(CFG_KEY_DEBUG_MODE, isDebugMode);

    const char *systemCfgDebugPath = GetCfgPath(eCfgParamDebug);
	BOOL isFileExist = IsFileExists(systemCfgDebugPath);
    if(FALSE == isFileExist)
    {
        // the DEBUG mode is set by default
        if(YES == isDebugMode)
        {
            AddActiveAlarm(	FAULT_GENERAL_SUBJECT,
	                        DEBUG_MODE,
	                        MAJOR_ERROR_LEVEL,
	                        "System is running in DEBUG mode",
	                        true,
	                        true
	                        );
            // the system is in DEBUG mode, default is corrupted(Process's recovery does'n work).
        }
        return;
    }

    CSysConfigEma sysConfigEma;
    sysConfigEma.LoadFromFile(eCfgParamDebug);

    const CCfgData *cfgDebugEntry = sysConfigEma.GetCfgEntryByKey(CFG_KEY_DEBUG_MODE);
    if(NULL != cfgDebugEntry)
    {
        const std::string &debufModeData = cfgDebugEntry->GetData();
        if(debufModeData == CFG_STR_YES)
        {
            AddActiveAlarm(	FAULT_GENERAL_SUBJECT,
	                        DEBUG_MODE,
	                        MAJOR_ERROR_LEVEL,
	                        "System DEBUG mode initiated",
	                        true,
	                        true
	                        );
            // the system is in DEBUG mode, overwrited from file (Process's recovery does'n work).
            return;
        }
    }

    DWORD numOfParams = sysConfigEma.GetNumOfParams();
    if(1 < numOfParams) // when the file is empty, GetNumOfParams() returns 1
    {
        AddActiveAlarm(	FAULT_GENERAL_SUBJECT,
                        DEBUG_CFG_EXIST,
                        MAJOR_ERROR_LEVEL,
                        "System is using DEBUG CFG flags",
                        true,
                        true
                        );
        // the system is NOT in DEBUG mode, but it uses flags from Debug CFG.
        return;
    }
}

/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SetKernelTCPParams()
{
	CSysConfig *sysConfig = m_pProcess->GetSysConfig();
	DWORD keepalive_time=0, keepalive_intvl=0;
	sysConfig->GetDWORDDataByKey(CFG_KEY_CPU_TCP_KEEP_ALIVE_TIME_SECONDS, keepalive_time);
	sysConfig->GetDWORDDataByKey(CFG_KEY_CPU_TCP_KEEP_INTERVAL_SECONDS, keepalive_intvl);
	CConfigManagerApi api;
	api.SetTCPStackParams(keepalive_time, keepalive_intvl);
}

/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::CreateCfgVisibleParams()
{
    const char *systemCfgTmpPath = GetEmaCfgPath(eCfgParamUser);
	BOOL isFileExist = IsFileExists(systemCfgTmpPath);
    if(TRUE == isFileExist)
    {
        // the file exist already.
        return;
    }

    CSysConfig* pSysConfig = m_pProcess->GetSysConfig();

    pSysConfig->SetIsSerializeVisibleOnly(true);
    pSysConfig->SetCfgParamTypeState(eCfgParamUser);

    pSysConfig->SaveToFile(systemCfgTmpPath);

    pSysConfig->SetIsSerializeVisibleOnly(false);
    pSysConfig->SetCfgParamTypeState(NumOfCfgTypes);
}

/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::HandleFirstNtpSync()
{
	m_pProcess->GetSysConfig()->GetDWORDDataByKey("NTP_SERVERS_STATUS_PERIOD", m_NtpServersStatusPeriod);
	m_pProcess->GetSysConfig()->GetDWORDDataByKey("NTP_SYNC_PERIOD", m_NtpSyncPeriod);


	if (YES == IsTarget())
	{
//		UpdateIfNtpSyncLegal(); // sample time via http  - removed. vngr-13557
		SyncNtp();

		if (YES == m_isNtpSyncLegal)
 		{
			CManagerApi api(eProcessEncryptionKeyServer);
			STATUS res = api.SendMsg(NULL, START_CREATE_KEYS_REQ);

			// external NTP is not supported in IPv6_Only
			eIpType ipType  = m_pMngmntIpParams_asIpService_fromProcess->GetIpType();

			if ( (YES == IsSysTimeNtpEnabled()) && (eIpType_IpV6 != ipType) )
			{
				StartTimer(NTP_SERVERS_STATUS_TIMER, NTP_FIRST_SYNC_TIMEOUT); // 21.06.09: was asked by Switch to increase timeout of first sync
		        StartTimer(NTP_SYNC_TIMER,           NTP_FIRST_SYNC_TIMEOUT); // 21.06.09: was asked by Switch to increase timeout of first sync
//				StartTimer(NTP_SERVERS_STATUS_TIMER, m_NtpServersStatusPeriod*SECOND*60);
//		        StartTimer(NTP_SYNC_TIMER,           m_NtpSyncPeriod*SECOND*60);
			}
			else
			{
		        StartTimer(NTP_SYNC_TIMER,           NTP_FIRST_SYNC_TIMEOUT); // 21.06.09: was asked by Switch to increase timeout of first sync
//		        StartTimer(NTP_SYNC_TIMER,           m_NtpSyncPeriod*SECOND*60); // first NTP sync - wait for Switch to be prepared
//			    StartTimer(NTP_SYNC_TIMER, 10*SECOND); // no need in waiting so long
			}
 		} // end if year==ok from http

	} // end if Target
	else
	{
		CManagerApi api(eProcessEncryptionKeyServer);
		STATUS res = api.SendMsg(NULL, START_CREATE_KEYS_REQ);

		eIpType ipType  = m_pMngmntIpParams_asIpService_fromProcess->GetIpType();

		if ((eProductTypeGesher == m_mcuMngrProductType || eProductTypeNinja == m_mcuMngrProductType || eProductTypeEdgeAxis == m_mcuMngrProductType)
			&& (YES == IsSysTimeNtpEnabled())
			&& (eIpType_IpV6 != ipType))
		{
			StartTimer(NTP_SERVERS_STATUS_TIMER, 20*SECOND);
			StartTimer(NTP_SYNC_TIMER, 20*SECOND);
		}
	}
}

/////////////////////////////////////////////////////////////////////
/*void CMcuMngrManager::UpdateIfNtpSyncLegal()
{
    time_t secondsFromEpoch;
    CStructTm timeFromSwitch;
    STATUS status = GetTimeFromSwitch(timeFromSwitch, secondsFromEpoch);
    if(STATUS_OK != status || MIN_LEGAL_YEAR > timeFromSwitch.m_year)
    {
        m_isNtpSyncLegal = NO;

		CMedString yearStr;
		yearStr << "Illegal time from switch (year: " << timeFromSwitch.m_year << "); no sync with Switch will be done";

		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::UpdateIfNtpSyncLegal -"
		                       << "\n" << yearStr.GetString();

		UpdateStartupConditionByErrorCode(AA_ILLEGAL_TIME, eStartupConditionFail);
		UpdateActiveAlarmDescriptionByErrorCode( AA_ILLEGAL_TIME, yearStr.GetString() );
	}
	else
	{
		m_isNtpSyncLegal = YES;
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::UpdateIfNtpSyncLegal() - success; year via HTTP: " << timeFromSwitch.m_year;

        RemoveActiveAlarmByErrorCode(AA_ILLEGAL_TIME);
    }
}*/

/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::CheckRunningVersionVsCurrent()
{
	string runningVer, currentVer, fallbackVer, factoryVer;
	BOOL getVersionsVal = GetVersions(runningVer, currentVer, fallbackVer, factoryVer);

	CMedString infoStr = "\nCMcuMngrManager::CheckRunningVersionVsCurrent -";
	infoStr << "\nRunning:  " << runningVer.c_str()
	        << "\nCurrent:  " << currentVer.c_str()
	        << "\nFallback: " << fallbackVer.c_str()
	        << "\nFactory:  " << factoryVer.c_str();


	if (TRUE == getVersionsVal) // getting versions succeeded
	{
		if ( !strcmp(runningVer.c_str(), currentVer.c_str()) )
		{
			infoStr << "\n---- Versions ok: running version is linked to 'current'.";
		}

		else
		{
			infoStr << "\n---- Running version is not linked to 'current'!";

			CSmallString errStr;
			errStr << "Fallback version is being used. Restore current version. Version being used: " << runningVer.c_str() << "; Current version: " << currentVer.c_str();

			AddActiveAlarm(	FAULT_GENERAL_SUBJECT,
							AA_RUNNING_VERSION_MISMATCHES_CURRENT_VERSION,
							MAJOR_ERROR_LEVEL,
							errStr.GetString(),
							true,
							true
						);
		}
	}

	else // error getting versions
	{
		infoStr << "\n---- Error in getting versions!";
	}

	TRACESTR(eLevelInfoNormal) << infoStr.GetString();
}


/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::CheckEthernet()
{
    CConfigManagerApi api;
	std::string sMngmntNI = GetDeviceName(eManagmentNetwork);

	STATUS testRes = api.TestEthernetSettings(sMngmntNI.c_str());
	if (STATUS_OK != testRes)
    {

        PASSERTMSG(1,"bad ethernet settings");

//         AddActiveAlarm(	FAULT_GENERAL_SUBJECT,
//                         AA_BAD_ETHERNET_SETTINGS,
//                         MAJOR_ERROR_LEVEL,
//                         "Bad ethernet settings for eth0, system performance will be reduced",
//                         true,
//                         true);
    }

}

/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::CheckDMA()
{
    CConfigManagerApi api;

    if (api.TestFlashDMA() != STATUS_OK)
    {
        AddActiveAlarm(	FAULT_GENERAL_SUBJECT,
                        AA_IDE_NOT_IN_DMA_MODE,
                        MAJOR_ERROR_LEVEL,
                        "MCMS CF doesn't support DMA",
                        true,
                        true);
    }

    if (api.TestHardDiskDMA() != STATUS_OK)
    {
        AddActiveAlarm(	FAULT_GENERAL_SUBJECT,
                        AA_IDE_NOT_IN_DMA_MODE,
                        MAJOR_ERROR_LEVEL,
                        "MCMS HD doesn't support DMA",
                        true,
                        true);
    }
}

/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::DeclareStartupConditions()
{
	CActiveAlarm aa1(FAULT_GENERAL_SUBJECT,
			AA_MPL_STARTUP_FAILURE_AUTHENTICATION_NOT_RECEIVED,
			MAJOR_ERROR_LEVEL,
			"MPL failure - product activation indication was not received",
			true,
			true);
	AddStartupCondition(aa1);

	if (m_mcuMngrProductType != eProductTypeEdgeAxis)
	{
		CActiveAlarm aa2(FAULT_GENERAL_SUBJECT,

				AA_PRODUCT_ACTIVATION_FAILURE,
				MAJOR_ERROR_LEVEL,
				"Product activation failure",
				true,
				true);
		AddStartupCondition(aa2);
	}

/*
// 16.03.06: MNGMNT_IP_CONFIG_IND is not a condition anymore
	CActiveAlarm aa3(FAULT_GENERAL_SUBJECT,
					NO_MNGMNT_IP_CONFIG_IND_RECEIVED,
					MAJOR_ERROR_LEVEL,
					"No management ip config indication received",
					true,
					true);
	AddStartupCondition(aa3);
*/
// 08.04.06: MNGMNT_IP_CONFIG_IND is a condition again...
	CActiveAlarm aa3(FAULT_GENERAL_SUBJECT,
					AA_MPL_STARTUP_FAILURE_MNGMNT_IP_CNFG_NOT_RECEIVED,
					MAJOR_ERROR_LEVEL,
					"MPL failure - management_ip_config indication was not received",
					true,
					true);
	AddStartupCondition(aa3);



	CActiveAlarm aa5(FAULT_GENERAL_SUBJECT,
					AA_UNKNOWN_CPU_SLOT_ID,
					MAJOR_ERROR_LEVEL,
					"CPU board id was not received from ShelfManager",
					true,
					true);
	AddStartupCondition(aa5);

	if (eDnsConfigurationSuccess != m_dnsConfigurationStatus)
	{
		if ( YES == IsTarget() )
		{
			CActiveAlarm aa6(FAULT_GENERAL_SUBJECT,
							AA_ILLEGAL_TIME,
							MAJOR_ERROR_LEVEL,
							"Illegal MCU Year",
							true,
							true);
			AddStartupCondition(aa6);



    		if(CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator)
    		{
				CActiveAlarm aa7(FAULT_GENERAL_SUBJECT,
								AA_NO_DNS_CONFIGURATION,
								MAJOR_ERROR_LEVEL,
								"DNS was not configured",
								true,
								true);
				AddStartupCondition(aa7);

				m_isStartupCondition_Dns_added = YES;


				CActiveAlarm aa8(FAULT_GENERAL_SUBJECT,
								AA_DNS_REGISTRAION_FAILED,
								MAJOR_ERROR_LEVEL,
								"Failed to register as a DNS Client",
								true,
								true);
				AddStartupCondition(aa8);
			}
		} // end if IsTarget

	}

	CActiveAlarm aa9(FAULT_GENERAL_SUBJECT,
							AA_NO_FIPS_140_TEST_RESULT_FROM_ENCRYPTIONKEYSERVER,
							MAJOR_ERROR_LEVEL,
							"FIPS 140 test result was not received from Encryption module",
							true,
							true);
	AddStartupCondition(aa9);

	if ( eProductFamilySoftMcu == ::ProductTypeToProductFamily(m_mcuMngrProductType) )
	{
		CActiveAlarm aa10(FAULT_GENERAL_SUBJECT,
							AA_LICENSE_EXPIRED,
							MAJOR_ERROR_LEVEL,
							"Please install a sw update",
							true,
							true);
		AddStartupCondition(aa10);


    }



	DeclareStartupConditionsDependencies();
}

////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::DeclareStartupConditionsDependencies()
{
	if (m_mcuMngrProductType != eProductTypeEdgeAxis)
	{
	   AddStartupCondDependency( AA_MPL_STARTUP_FAILURE_AUTHENTICATION_NOT_RECEIVED,
	                          AA_PRODUCT_ACTIVATION_FAILURE );

	   AddStartupCondDependency( AA_PRODUCT_ACTIVATION_FAILURE,
						      AA_NO_FIPS_140_TEST_RESULT_FROM_ENCRYPTIONKEYSERVER );
	}

	AddStartupCondDependency( AA_MPL_STARTUP_FAILURE_AUTHENTICATION_NOT_RECEIVED,
	                          AA_MPL_STARTUP_FAILURE_MNGMNT_IP_CNFG_NOT_RECEIVED );


	AddStartupCondDependency( AA_MPL_STARTUP_FAILURE_AUTHENTICATION_NOT_RECEIVED,
							  AA_UNKNOWN_CPU_SLOT_ID );

	if (YES == m_isStartupCondition_Dns_added)
	{

		if ( YES == IsTarget() )
		{
			AddStartupCondDependency( AA_MPL_STARTUP_FAILURE_AUTHENTICATION_NOT_RECEIVED,
			                          AA_ILLEGAL_TIME );


			AddStartupCondDependency( AA_NO_DNS_CONFIGURATION,
			                          AA_DNS_REGISTRAION_FAILED );
		}
	}
/*
// 23.05.06:
// According to YuriR, when a dependency is declared, then all offsprings' ActiveAlarms will not be produced.
//   therefore there is no need to declare direct dependencies between parent and all his 'far' offsprings
*/
}

void CMcuMngrManager::ManagerStartupActionsPoint()
{
    CSysConfig *sysConfig = m_pProcess->GetSysConfig();
    BOOL isSshOn = FALSE, isJITCMode = NO;
    sysConfig->GetBOOLDataByKey("SSH", isSshOn);
    sysConfig->GetBOOLDataByKey(CFG_KEY_JITC_MODE, isJITCMode);

    bool bIsSshOn = (TRUE == isSshOn);

    if (CConfigManagerApi::IsSSHOnForDebug())
    {
        isJITCMode = FALSE;
        bIsSshOn = TRUE;
    }

    STATUS status = STATUS_OK;
    if(!isJITCMode)
    	status = TurnSsh(bIsSshOn, true);
//    else
//    	StartTimer(KILL_SSHD_CYCLIC, 10*SECOND);



	BOOL isSystemAfterVersionInstall = IsFileExists(SYSTEM_AFTER_VERSION_INSTALLED_FILE);
	if (TRUE == isSystemAfterVersionInstall)
	{
        AddActiveAlarm(	FAULT_GENERAL_SUBJECT,
        		   		AA_INSTALLING_NEW_VERSION,
                        MAJOR_ERROR_LEVEL,
                        "A new version installation in progress; please wait",
                        true,
                        true);
	}

	//in restore, startup condition are only AFTER we check the file, so we remove the AA here
	if (m_bRestoreConfigSuccess == TRUE)
	{


		if (m_bRestoreNoDnsConfiguration_AA == FALSE)
		{
			RemoveActiveAlarmByErrorCode(AA_NO_DNS_CONFIGURATION);
		}
		if (m_bRestoreDnsRegistrationFailed_AA == FALSE)
			RemoveActiveAlarmByErrorCode(AA_DNS_REGISTRAION_FAILED);
	}

	// ask CentralSignaling to send mediaIpParams
	SendIpServiceParamsReqToCS();

	// SoftMCU: because from lack of GideoSim, need remove some AA about missing indications
	if ( eProductFamilySoftMcu == ::ProductTypeToProductFamily(m_mcuMngrProductType) )	//hard coded license for softMcu
	{
		// 1. No more MPL_MNGMNT_IP_CONFIG_IND
		RemoveActiveAlarmByErrorCode(AA_MPL_STARTUP_FAILURE_MNGMNT_IP_CNFG_NOT_RECEIVED);
		// 2. No more SM_COMP_SLOT_ID_IND from switch card
		RemoveActiveAlarmByErrorCode(AA_UNKNOWN_CPU_SLOT_ID);
	}

}

STATUS CMcuMngrManager::HandleTerminalMcuVer(CTerminalCommand & command, std::ostream& answer)
{
	VERSION_S mcuVersion	= m_pSystemVersions->GetMcuVersion();
	VERSION_S mcmsVersion	= m_pSystemVersions->GetMcmsVersion();

	answer << "RMX version:  " << mcuVersion.ver_major  << "." << mcuVersion.ver_minor  << "." << mcuVersion.ver_release  <<  "." << mcuVersion.ver_internal  << "\n";
	answer << "MCMS version: " << mcmsVersion.ver_major << "." << mcmsVersion.ver_minor << "." << mcmsVersion.ver_release <<  "." << mcmsVersion.ver_internal << "\n";

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::HandleTerminalStartDHCP(CTerminalCommand & command,std::ostream& answer)
{
    CConfigManagerApi api;
    return api.RunDHCP();
}

/////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::HandleTerminalStopDHCP(CTerminalCommand & command,std::ostream& answer)
{

    CConfigManagerApi api;
    eIpType type = m_pMngmntIpParams_asIpService_fromProcess->GetIpType();
	//TRACESTR(NORMAL_TRACE) << "\n" << __FUNCTION__ << " type " << ::IpTypeToString(type);
    return api.KillDHCP(type);
}

/////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::HandleTerminalGetDHCP(CTerminalCommand & command,std::ostream& answer)
{
    DWORD numOfParams = command.GetNumOfParams();
	if(1 != numOfParams)
	{
		answer << "error: Illegal number of parameters\n";
		answer << "usage: McuCmd get_dhcp [IPADDR|NETMASK|NETWORK|BROADCAST|GATEWAY|DOMAIN|DNS]\n";
		return STATUS_FAIL;

    }

    CConfigManagerApi api;
    std::string value;
    STATUS stat = api.GetLastDHCPConfig(command.GetToken(eCmdParam1),value);
    answer << value;
    return stat;
}


/////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::HandleTerminalUpdateDns(CTerminalCommand & command,std::ostream& answer)
{
    DWORD numOfParams = command.GetNumOfParams();
	if(5 != numOfParams)
	{
		answer << "error: Illegal number of parameters\n";
		answer << "usage: McuCmd update_dns DNS HOST ZONE IP";

		return STATUS_FAIL;
    }

    CConfigManagerApi api;
    STATUS stat = api.RegisterHostNameInDNS(command.GetToken(eCmdParam1),
                                            command.GetToken(eCmdParam2),
                                            command.GetToken(eCmdParam3),
                                            command.GetToken(eCmdParam4),
                                            command.GetToken(eCmdParam5));

    return stat;
}

/////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::HandleTerminalMcuState(CTerminalCommand & command,std::ostream& answer)
{
	eMcuState systemState = CProcessBase::GetProcess()->GetSystemState();
    answer << "System State : " << GetMcuStateName(systemState);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::HandleTerminalAllAA(CTerminalCommand & command,std::ostream& answer)
{
	CFaultList *aaList = m_pProcess->GetActiveAlarmsList();
	CLogFltElement* iter = aaList->GetFirstFaultElement();
	while(NULL != iter)
	{
		iter->Dump(answer);

		iter = aaList->GetNextFaultElement();
	}
    return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::HandleTerminalGenerate_X_Kc(CTerminalCommand & command, std::ostream& answer)
{
	answer << "Licesing Key cannot be generated via RMX; use external utility";
	return STATUS_FAIL;

/*
	DWORD numOfParams = command.GetNumOfParams();
	if(4 != numOfParams)
	{
		answer << "error: Illegal number of parameters\n";
		answer << "usage: Bin/McuCmd generate_k_kc McuMngr [SerialNumber] [numOfPorts] [Encryption=YES/NO] [PSTN=YES/NO]\n";
		return STATUS_FAIL;
	}

	const string &serialNumberStr	= command.GetToken(eCmdParam1);
	const string &numOfPortsStr		= command.GetToken(eCmdParam2);
	const string &isEncEnabledStr	= command.GetToken(eCmdParam3);
	const string &isPstnEnabledStr	= command.GetToken(eCmdParam4);

	// check numOfPorts legality
	int numOfPorts = atoi(numOfPortsStr.c_str());
	if ( (MAX_NUM_OF_CP_PORTS_IN_KEYCODE * CP_PORTS_NUM_INTERVAL) < numOfPorts )
	{
		answer << "error: max legal number of ports is " << (MAX_NUM_OF_CP_PORTS_IN_KEYCODE*CP_PORTS_NUM_INTERVAL) << "\n";
		return STATUS_FAIL;
	}

	if ( numOfPorts % CP_PORTS_NUM_INTERVAL != 0)
	{
		answer << "error: number of ports should be a product of " << CP_PORTS_NUM_INTERVAL << "\n";
		return STATUS_FAIL;
	}

	// check isEncEnabled legality
	if 	( (isEncEnabledStr != "YES") && (isEncEnabledStr != "NO") )
	{
		answer << "error: legal value for Encryption is 'YES' or 'NO'\n";
		return STATUS_FAIL;
	}

	// check isPstnEnabled legality
	if 	( (isPstnEnabledStr != "YES") && (isPstnEnabledStr != "NO") )
	{
		answer << "error: legal value for PSTN is 'YES' or 'NO'\n";
		return STATUS_FAIL;
	}

	// ===== 1. prepare Options bitmask
	CSmallString optionBitmask;
	GenerateInputBitmask_K(optionBitmask, numOfPortsStr, isEncEnabledStr, isPstnEnabledStr);

	// ===== 2. generate keycode
	CKeyCode retKc;
	char szCreated_X_KeyCode[KEYCODE_LENGTH]={0};
	retKc.GenerateKeyCode( szCreated_X_KeyCode,
	                       (char*)(serialNumberStr.c_str()),
	                       (char*)(optionBitmask.GetString()),
	                       'X');

	answer << "Keycode generated successfully!\n" << szCreated_X_KeyCode << "\n";

	return STATUS_OK;
*/
}

////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::HandleTerminalGenerate_U_Kc(CTerminalCommand & command, std::ostream& answer)
{
	answer << "Licesing Key cannot be generated via RMX; use external utility";
	return STATUS_FAIL;


	DWORD numOfParams = command.GetNumOfParams();
	if(2 != numOfParams)
	{
		answer << "error: Illegal number of parameters\n";
		answer << "usage: Bin/McuCmd generate_u_kc McuMngr [SerialNumber] [versionNumber]\n";
		return STATUS_FAIL;
	}

	const string &serialNumberStr	= command.GetToken(eCmdParam1);
	const string &versionNumStr		= command.GetToken(eCmdParam2);


	// ===== 1. prepare Options bitmask
	CSmallString optionBitmask;
//	GenerateInputBitmask_U(optionBitmask, versionNumStr);
	optionBitmask << versionNumStr;

	// ===== 2. generate keycode
	CKeyCode retKc;
	char szCreated_U_KeyCode[KEYCODE_LENGTH]={0};
	retKc.GenerateKeyCode( szCreated_U_KeyCode,
	                       (char*)(serialNumberStr.c_str()),
	                       (char*)(optionBitmask.GetString()),
	                       'U');

	answer << "Keycode generated successfully!\n" << szCreated_U_KeyCode << "\n";

	return STATUS_OK;

}

////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::HandleTerminalLicensingInfo(CTerminalCommand & command, std::ostream& answer)
{
	DWORD numOfParams = command.GetNumOfParams();
	if(0 != numOfParams)
	{
		answer << "error: No parameters should be added\n";
		answer << "usage: Bin/McuCmd licensing_info McuMngr\n";
		return STATUS_FAIL;
	}

	CLicensing* pLicense = m_pProcess->GetLicensing();
	VERSION_S mcuVersion = pLicense->GetMcuVersion();

	string encStr  				= ( pLicense->GetIsEncryptionEnabled() 		 ? "True" : "False" );
	string pstnStr 				= ( pLicense->GetIsPstnEnabled()       		 ? "True" : "False" );
	string telepresenceStr 		= ( pLicense->GetIsTelepresenceEnabled()	 ? "True" : "False" );
	string multiple_servicesStr = ( pLicense->GetIsMultipleServicesEnabled() ? "True" : "False" );
	string  mpmxStr = ( pLicense->GetIsMPMXBitEnabled()    ? "is ON" : "is OFF" );

	answer << "Serial number:  "		    << pLicense->GetMplSerialNum()
	       << "\nTotal Num of COP Parties: "    << pLicense->GetTotalNumOfCopParties()
	       << "\nTotal Num of CP Parties:  "    << pLicense->GetTotalNumOfCpParties()
	       << "\nEncription: "		    << encStr.c_str()
	       << "\nPstn: "			    << pstnStr.c_str()
	       << "\nMPMX bit: "                    << mpmxStr.c_str()
               << "\nTelepresence: "		    << telepresenceStr.c_str()
	       << "\nMultipleServices: "            << multiple_servicesStr.c_str()
	       << "\nMCU Version:   "		    << mcuVersion.ver_major    << "."
	                                            << mcuVersion.ver_minor    << "."
	                                            << mcuVersion.ver_release  << "."
	                                            << mcuVersion.ver_internal << "\n";

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::HandleTerminalSetRestore(CTerminalCommand & command, std::ostream& answer)
{
	DWORD numOfParams = command.GetNumOfParams();
	if(1 != numOfParams)
	{
		answer << "error: Illegal number of parameters\n";
		answer << "usage: Bin/McuCmd set_restore McuMngr [standard/inhensive]\n";
		return STATUS_FAIL;
	}

	const string &type = command.GetToken(eCmdParam1);
	eMcuRestoreType restoreType = (type == "inhensive" ? eMcuRestoreInhensive : eMcuRestoreStandard);

	string description = "";
	STATUS status = PerformRestoreFactoryDefaults(restoreType, description);
	answer << description;

	return status;
}

/////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::HandleTerminalResetMngmntParams(CTerminalCommand & command, std::ostream& answer)
{
	SendResetMngmntParamsReqToMplApi();

	CSmallString retStr = "A request was sent to Switch to reset Management_Service parameters\n";
	answer << retStr.GetString();
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::HandleTerminalResetMngmnt - " << retStr.GetString();

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::HandleTerminalResetUsrListParams(CTerminalCommand & command, std::ostream& answer)
{
	SendResetUsrListParamsReqToMplApi();

	CSmallString retStr = "A request was sent to Switch to reset Users_List parameters\n";
	answer << retStr.GetString();
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::HandleTerminalResetUsrList - " << retStr.GetString();

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::HandleTerminalResetAllParams(CTerminalCommand & command, std::ostream& answer)
{
	SendResetAllParamsReqToMplApi();

	CSmallString retStr = "A request was sent to Switch to reset all parameters\n";
	answer << retStr.GetString();
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::HandleTerminalResetAllParams - " << retStr.GetString();

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::HandleTerminalTurnSshOn(CTerminalCommand & command, std::ostream& answer)
{

	DWORD numOfParams = command.GetNumOfParams();
	if(1 != numOfParams)
	{
		answer << "error: Illegal number of parameters\n";
		answer << "usage: Bin/McuCmd turn_ssh_on McuMngr [ON/*]\n";
		return STATUS_FAIL;
	}

	const string &strIsSshOn = command.GetToken(eCmdParam1);
    bool isSshOn = ("ON" == strIsSshOn);
    STATUS status = TurnSsh(isSshOn);

    return status;
}

/////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::TurnSsh(bool isOn,
                                bool isStartup/*=false*/)
{
    STATUS status = STATUS_OK;

    eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
    if(YES == IsTarget() || eProductTypeGesher == curProductType || eProductTypeNinja == curProductType || eProductTypeEdgeAxis == curProductType)
    {
        string strIpV4Address = "";  // empty string turns ssh off.
        string strIpV6Address = "";

        if(TRUE == isOn)
        {
            // Mngmnt params are stored in the 1st span (idx==0)
            CIPSpan* pTmpSpan = m_pMngmntIpParams_asIpService_fromProcess->GetSpanByIdx(0);
            if(NULL == pTmpSpan)
            {
                PASSERTMSG(TRUE, "No first span in the MNGMNT service");
                return STATUS_FAIL;
            }

            DWORD ipAddress = pTmpSpan->GetIPv4Address();
            char buffer [32];
            SystemDWORDToIpString(ipAddress, buffer);
            strIpV4Address = buffer;


			// ----- ipV6 (if needed) -----
            /*
            Note: here we say that ssh of IPv6 will be configured at startup only if it's Manual configuration.
                  Otherwise (startup + Auto) the address is not configured yet.
                  This should be re-checked when ssh of IPv6 is required.
            */
            bool isIPv6Needed = true;
			eIpType ipType  = m_pMngmntIpParams_asIpService_fromProcess->GetIpType();

			if (eIpType_IpV4 == ipType)  isIPv6Needed = false;


			if (true == isIPv6Needed)
			{
				string retStr_ipV6;

				IpV6AddressMaskS ipV6_S[MNGMNT_NETWORK_NUM_OF_IPV6_ADDRESSES];

				eConfigInterfaceType ifType = eManagmentNetwork;
				if(IsJitcAndNetSeparation())
					ifType = eSeparatedManagmentNetwork;
				std::string nic_name;
				if (m_mcuMngrProductType==eProductTypeEdgeAxis)
				{
					nic_name="eth0";
				    CIPSpan * pSpan =m_pMngmntIpParams_asIpService_fromProcess->GetFirstSpan();
				    if(pSpan)
				        nic_name = pSpan->GetInterface();
				}
				else
				    nic_name = GetLogicalInterfaceName(ifType, eIpType_IpV6);

				RetrieveIpAddressConfigured_IpV6(ipV6_S, retStr_ipV6, nic_name);
				TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::TurnSsh - return string IpV6 for " << nic_name << " :\n" << retStr_ipV6.c_str();

				strIpV6Address = ipV6_S[0].address;
			}
			// ----- end ipV6 -----

        } // end is ssh on

        CConfigManagerApi api;
        STATUS status = api.RestartSSHD(m_pMngmntIpParams_asIpService_fromProcess->GetIsPermanentNetworkOpen(), strIpV4Address, strIpV6Address, isOn);
    }


    if(STATUS_OK == status)
    {
	    //The alarm is also displayed in simulation
	    if(TRUE == isOn)
	    {
	    	AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT,
	                                    AA_SSH_ENABLED,
	                                    MAJOR_ERROR_LEVEL,
	                                    "SSH is enabled",
	                                    true,
	                                    true);

	        AUDIT_EVENT_HEADER_S outAuditHdr;
	        CAuditorApi::PrepareAuditHeader(outAuditHdr,
	                                        "",
	                                        eMcms,
	                                        "",
	                                        "",
	                                        eAuditEventTypeInternal,
	                                        eAuditEventStatusOk,
	                                        "SSH is enabled",
	                                        "SSH is enabled",
	                                        "",
	                                        "");
	        CFreeData freeData;
	        CAuditorApi::PrepareFreeData(freeData,
	                                     "",
	                                     eFreeDataTypeXml,
	                                     "",
	                                     "",
	                                     eFreeDataTypeXml,
	                                     "");

	        CAuditorApi api;
	        api.SendEventMcms(outAuditHdr, freeData);
	    }
	    else
	    	RemoveActiveAlarmByErrorCode(AA_SSH_ENABLED);

        CMcuState *pMcuState = m_pProcess->GetMcuStateObject();
        pMcuState->SetIsSshOn(isOn);
    }

    TRACEINTO << "SSH: " << (isOn ? "ON" : "OFF")
              << "\nStatus : " << CProcessBase::GetProcess()->GetStatusAsString(status).c_str();

    CSegment* seg = new CSegment(CTelemetryValue(eTT_IdentityConsoleAccess, (BOOL)isOn).GetSegment());
    CManagerApi(eProcessSNMPProcess).SendMsg(seg, SNMP_UPDATE_TELEMETRY_DATA_IND);

    return status;
}

/////////////////////////////////////////////////////////////////////
/*
void CMcuMngrManager::GenerateInputBitmask_K( CSmallString &optionBitmask, const string numOfPortsStr,
		                                      const string isEncEnabledStr, const string isPstnEnabledStr )
{
	int i=0;

	// ===== 1. fill with zeros until first significant digit
	for (i=0; i<VERSION_BITMASK_LEN-CP_PORTS_NUM_DIGIT; i++)
	{
		optionBitmask << "0";
	}

	// ===== 2. fill numOfPorts
	int numOfPorts = atoi(numOfPortsStr.c_str()) / CP_PORTS_NUM_INTERVAL;
	optionBitmask << numOfPorts;

	// ===== 3. fill with zeros until next significant digit
	CSmallString numOfPortsStrStr;
	numOfPortsStrStr << numOfPorts;
	int numOfDigitsInPorts = numOfPortsStrStr.GetStringLength();

	int numOfRemainingZeros = ( VERSION_BITMASK_LEN - CP_PORTS_NUM_DIGIT - numOfDigitsInPorts ) -
	                          ( VERSION_BITMASK_LEN - FEATURES_DIGIT );

	for (i=0; i<numOfRemainingZeros; i++)
	{
		optionBitmask << "0";
	}

	// ===== 4. fill features
	if ( (isEncEnabledStr == "YES") && (isPstnEnabledStr == "YES") )
	{
		optionBitmask << (ENCRYPTION_MASK | PSTN_MASK);
	}
	else if (isEncEnabledStr == "YES")
	{
		optionBitmask << ENCRYPTION_MASK;
	}
	else if (isPstnEnabledStr == "YES")
	{
		optionBitmask << PSTN_MASK;
	}
	else // (isEncEnabledStr == "NO") && (isPstnEnabledStr == "NO")
	{
		optionBitmask << "0";
	}
}

/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::GenerateInputBitmask_U(CSmallString &optionBitmask, const string versionNumStr)
{
	int numOfZeros = VERSION_BITMASK_LEN - versionNumStr.length();
	for (int i=0; i<numOfZeros; i++)
	{
		optionBitmask << "0";
	}

	optionBitmask << versionNumStr.c_str();
}
*/
/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendResetMngmntParamsReqToMplApi()
{
	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::SendResetMngmntParamsReqToMplApi";

	CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol();

	mplPrtcl->AddCommonHeader(MCUMNGR_RESET_MNGMNT_SERVICE_PARAMS_REQ);
	mplPrtcl->AddMessageDescriptionHeader();
	mplPrtcl->AddPhysicalHeader(1, m_switchBoardId, m_switchSubBoardId);

	CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("MCUMNGR_SENDS_TO_MPL_API");
	mplPrtcl->SendMsgToMplApiCommandDispatcher();

	POBJDELETE(mplPrtcl);
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendResetUsrListParamsReqToMplApi()
{
	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::SendResetUsrListParamsReqToMplApi";

	CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol();

	mplPrtcl->AddCommonHeader(MCUMNGR_RESET_USR_LIST_PARAMS_REQ);
	mplPrtcl->AddMessageDescriptionHeader();
	mplPrtcl->AddPhysicalHeader(1, m_switchBoardId, m_switchSubBoardId);

	CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("MCUMNGR_SENDS_TO_MPL_API");
	mplPrtcl->SendMsgToMplApiCommandDispatcher();

	POBJDELETE(mplPrtcl);
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendResetAllParamsReqToMplApi()
{
	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::SendResetAllParamsReqToMplApi";

	CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol();

	mplPrtcl->AddCommonHeader(MCUMNGR_RESET_ALL_PARAMS_REQ);
	mplPrtcl->AddMessageDescriptionHeader();
	mplPrtcl->AddPhysicalHeader(1, m_switchBoardId, m_switchSubBoardId);

	CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("MCUMNGR_SENDS_TO_MPL_API");
	mplPrtcl->SendMsgToMplApiCommandDispatcher();

	POBJDELETE(mplPrtcl);
}



/////////////////////////////////////////////////////////////////////
int CMcuMngrManager::GetNumberOfDefinedNtpServers()
{
	int num_of_defined_ntp_servers = 0;
	CSystemTime sysTime = m_pProcess->GetCurrentMcuTime();

	std::string server0_ip = sysTime.GetNTP_IPAddress(0);
	std::string server1_ip = sysTime.GetNTP_IPAddress(1);
	std::string server2_ip = sysTime.GetNTP_IPAddress(2);

	std::string stServer0_ipv6 = sysTime.GetNTP_IPv6_Address(0);
	std::string stServer1_ipv6 = sysTime.GetNTP_IPv6_Address(1);
	std::string stServer2_ipv6 = sysTime.GetNTP_IPv6_Address(2);

	if ("" != server0_ip)
	{
		TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::GetNumberOfDefinedNtpServers - server0_ip = "<<server0_ip;
		num_of_defined_ntp_servers++;
	}

	if ("" != server1_ip)
	{
		TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::GetNumberOfDefinedNtpServers - server1_ip = "<<server1_ip;
		num_of_defined_ntp_servers++;
	}

	if ("" != server2_ip)
	{
		TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::GetNumberOfDefinedNtpServers - server2_ip = "<<server2_ip;
		num_of_defined_ntp_servers++;
	}

	if ("::" != stServer0_ipv6 && "" == server0_ip && "" != stServer0_ipv6)
	{
		TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::GetNumberOfDefinedNtpServers - stServer0_ipv6 = "<<stServer0_ipv6;
		num_of_defined_ntp_servers++;
	}

	if ("::" != stServer1_ipv6 && "" == server1_ip && "" != stServer1_ipv6)
	{
		TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::GetNumberOfDefinedNtpServers - stServer1_ipv6 = "<<stServer1_ipv6;
		num_of_defined_ntp_servers++;
	}

	if ("::" != stServer2_ipv6 && "" == server2_ip && "" != stServer2_ipv6)
	{
		TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::GetNumberOfDefinedNtpServers - stServer2_ipv6 = "<<stServer2_ipv6;
		num_of_defined_ntp_servers++;
	}

	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::GetNumberOfDefinedNtpServers - num_of_defined_ntp_servers = "<<num_of_defined_ntp_servers;

	return num_of_defined_ntp_servers;

}
/////////////////////////////////////////////////////////////////////
std::string CMcuMngrManager::GetIpFromDnsAddress(std::string dnsAddress)
{
	struct hostent *he;
	struct in_addr **addr_list;

	if ((he = gethostbyname(dnsAddress.c_str())) == NULL) {  // get the host info

		return "";
	}

	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::HandleSetTime Official name is: he->h_name  " << he->h_name;
	printf("    IP addresses: ");
	addr_list = (struct in_addr **)he->h_addr_list;

	std::string address = inet_ntoa(*addr_list[0]);
	return address;
}
/////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::GetNtpPeerStatus(const char* ptr_peer_ip)
{
	    eProductType curProductType = m_mcuMngrProductType;



		if (!(eProductTypeGesher == curProductType || eProductTypeNinja == curProductType  || eProductTypeEdgeAxis == curProductType || eProductTypeEdgeAxis  == curProductType))
		{
			if (!IsTarget())
			{
				 TRACESTR(eLevelInfoNormal) << "\nThere is no switch in rmx simulation";
				return STATUS_NO_PERMISSION;
			}
		}
        std::string peer_ip(ptr_peer_ip);
	    std::string cmd;
	    std::string answer;

	    BOOL isIPV6 = isIpV6Str(peer_ip.c_str());

	    TRACESTR(eLevelInfoNormal) << "\nGetNtpPeerStatus ip.c_str() = "<<peer_ip.c_str();

	    if(!isIpV4Str(peer_ip.c_str()) && !isIPV6)

	    {
	    	TRACESTR(eLevelInfoNormal) << "\nGetNtpPeerStatus GetIpFromDnsAddress";
	    	peer_ip =  GetIpFromDnsAddress(peer_ip);
	    }


		    ///////////////////////

		if (eProductTypeGesher == curProductType || eProductTypeNinja == curProductType )
		{
			cmd += "sudo Bin/NTP_Bypass_SoftMCU_Query " + peer_ip + " | grep -E ^.Status";
		}
		else
		{
			if(eProductTypeEdgeAxis  != curProductType)
			{

				   cmd += " /usr/sbin/ntpq -p -n 169.254.128.16 | grep -E ^." + peer_ip+"[[:space:]]";

			}
			else
			{

				if (!isIPV6)
				     cmd += " /usr/sbin/ntpq -p -n 127.0.0.1 | grep -E ^." + peer_ip +"[[:space:]]";
				else
					 cmd += "ntpstat | grep 'synchronised to NTP server'";
			}

		}

	    STATUS stat = SystemPipedCommand(cmd.c_str(),answer);
	    TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
	        "CMcuMngrManager::GetNtpPeerStatus :" << cmd << std::endl << answer;

	    if((isIPV6 == true) && (answer.size() != 0))
	    {
	    	return STATUS_NTP_PEER_SYS_PEER;
	    }



	    if((stat == STATUS_FAIL)|| (answer.size() == 0))
	    {
	         return STATUS_FAIL;
	    }
		switch (answer.c_str()[0])
		{
			case ' ':
				stat = STATUS_NTP_PEER_REJECT;
				break;
			case 'x':
				stat = STATUS_NTP_PEER_FALSETICK;
				break;
			case '.':
				stat = STATUS_NTP_PEER_EXCESS;
				break;
			case '-':
				stat = STATUS_NTP_PEER_OUTLYER;
				break;
			case '+':
				stat = STATUS_NTP_PEER_CANDIDAT;
				break;
			case '#':
				stat = STATUS_NTP_PEER_SELECTED;
				break;
			case '*':
				stat = STATUS_NTP_PEER_SYS_PEER;
				break;
			case 'o':
				stat = STATUS_NTP_PEER_PPS_PEER;
				break;

			default:
				stat = STATUS_FAIL;
		}

		return stat;
}

// Assumption server_ipStr is char[IP_ADDRESS_LEN];
BOOL CMcuMngrManager::GetServerStatus(std::string *server_ipStr, int serverInd, int num_of_defined_ntp_servers, CLargeString& resStr)
{
	  TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::GetServerStatus";
	eProductType curProductType = m_mcuMngrProductType;
	const CSystemTime &sysTime = m_pProcess->GetCurrentMcuTime();

	std::string server_ip = sysTime.GetNTP_IPAddress(serverInd);
	std::string stServer_ipv6 = sysTime.GetNTP_IPv6_Address(serverInd);

	//SystemDWORDToIpString(server_ip, server_ipStr);

	//TODO: ofir: handle next line - should change server_ipStr to strings and its caller
	server_ipStr = &server_ip;
	STATUS server_stat = STATUS_FAIL;
	STATUS ipv6_server_stat = STATUS_FAIL;

	eNtpServerStatus server_status = eNtpServerNotConfigured;

	WORD numFailuresSinceConnecting = 0;


    TRACESTR(eLevelInfoNormal) << "GetServerStatus  " << (int)serverInd << " server_ip " << server_ip.c_str() << " stServer_ipv6 " << stServer_ipv6.c_str();

	if ("" != server_ip && INVALID_SERVER_ADDR_STR != server_ip)
	{
			server_stat = GetNtpPeerStatus(server_ipStr->c_str());

		if (IsPeerStatusFailed(server_stat) )
		{
			server_status = eNtpServerFail;
		}
		else
		{
			server_status = eNtpServerOk;
		}
		resStr << "\nStatus of " << server_ipStr->c_str() << ": "
			   << m_pProcess->GetStatusAsString(server_stat).c_str();
		TRACESTR(eLevelInfoNormal) << "GetServerStatus:  ipv4 of  serverInd " << serverInd << " server_stat: " << (int)server_stat << " server_status " << (int)server_status;


	}

	if ("::" != stServer_ipv6 && ("" == server_ip ||  INVALID_SERVER_ADDR_STR == server_ip) && "" != stServer_ipv6)
	{

			ipv6_server_stat = GetNtpPeerStatus(stServer_ipv6.c_str());


		if (IsPeerStatusFailed(ipv6_server_stat) )
		{
			server_status = eNtpServerFail;
		}
		else
		{
			server_status = eNtpServerOk;
		}

		resStr << "\nStatus of " << stServer_ipv6 << ": "
				<< m_pProcess->GetStatusAsString(server_stat).c_str();

		TRACESTR(eLevelInfoNormal) << "GetServerStatus:  ipv 6 of serverInd " << serverInd << " server_stat " << (int)server_stat << " server_status " << (int)server_status;

	}
	if (sysTime.GetNtpServerStatus(serverInd) == eNtpServerConnecting)
	{
		if (server_status == eNtpServerFail && sysTime.GetNumFailuresSinceConnecting(serverInd) == 0)
		{
			TRACESTR(eLevelInfoNormal) << "SetMcuTimeNtpServerStatus eNtpServerConnecting  serverInd " << serverInd ;

			server_status = eNtpServerConnecting;
			numFailuresSinceConnecting = 1;
		}

	}
	TRACESTR(eLevelInfoNormal) << "GetServerStatus  serverInd " << serverInd << " server_status " << (int)server_status <<  " numFailuresSinceConnecting " << numFailuresSinceConnecting;

	// set server status
	m_pProcess->SetMcuTimeNtpServerStatus(serverInd, server_status, numFailuresSinceConnecting);

	if ("::" != stServer_ipv6 && "" != stServer_ipv6 && ("" == server_ip || INVALID_SERVER_ADDR_STR == server_ip))
	{
		server_ipStr = &stServer_ipv6;
	}
	return (server_status == eNtpServerFail);

}



void CMcuMngrManager::SampleNtpPeerStatus()
{
	if ( NO == IsSysTimeNtpEnabled() )
	{
		RemoveActiveAlarmByErrorCode(AA_EXTERNAL_NTP_SERVERS_FAILURE);
		return;
	}

	CLargeString resStr = "\nCMcuMngrManager::SampleNtpPeerStatus -";


	std::string server0_ipStr;
	std::string server1_ipStr;
	std::string server2_ipStr;

	int num_of_defined_ntp_servers = GetNumberOfDefinedNtpServers();

    BOOL server0_fail =  GetServerStatus(&server0_ipStr, 0, num_of_defined_ntp_servers, resStr);
    BOOL server1_fail =  GetServerStatus(&server1_ipStr, 1, num_of_defined_ntp_servers, resStr);
    BOOL server2_fail =  GetServerStatus(&server2_ipStr, 2, num_of_defined_ntp_servers, resStr);

	TRACESTR(eLevelInfoNormal) << resStr.GetString();


	ProduceNtpPeerStatusFaultIfNeeded( server0_fail, server1_fail, server2_fail,
	                                     server0_ipStr,server1_ipStr, server2_ipStr );
}


/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::ProduceNtpPeerStatusFaultIfNeeded( BOOL isServer0_failed, BOOL isServer1_failed, BOOL isServer2_failed,
		std::string server0_ipStr, std::string server1_ipStr, std::string server2_ipStr )
{
	// ===== 1. if all three servers failed - add ActiveAlarm
	if ( (YES == isServer0_failed) && (YES == isServer1_failed) && (YES == isServer2_failed) )
	{
        if(MAX_NUM_NTP_SERVERS_FAILURE <= m_NtpFailureCounter)
        {
            CLargeString errStr = "NTP servers failure: ";
            errStr << server0_ipStr.c_str() << ", " << server1_ipStr.c_str() << ", " << server2_ipStr.c_str();

            if ( false == IsActiveAlarmExistByErrorCode(AA_EXTERNAL_NTP_SERVERS_FAILURE) )
            {
                AddActiveAlarm(	FAULT_GENERAL_SUBJECT,
                                AA_EXTERNAL_NTP_SERVERS_FAILURE,
                                SYSTEM_MESSAGE,
                                errStr.GetString(),
                                true,
                                true );
            }
            else // Alert exists, but maybe the user has changed the addresses of the servers
            {
            	UpdateActiveAlarmDescriptionByErrorCode( AA_EXTERNAL_NTP_SERVERS_FAILURE, errStr.GetString() );
            }
        }
        else
        {
            m_NtpFailureCounter++;
        }
	}
	else // no ActiveAlarm was produced
	{
        m_NtpFailureCounter = 0;

		// ===== 2. if at least one server is ok - remove ActiveAlarm
		if (NO == isServer0_failed)
		{
			m_isActiveAlarmNtpPeerAlreadyProduced[0] = NO;
			RemoveActiveAlarmByErrorCode(AA_EXTERNAL_NTP_SERVERS_FAILURE);
		}
		if (NO == isServer1_failed)
		{
			m_isActiveAlarmNtpPeerAlreadyProduced[1] = NO;
			RemoveActiveAlarmByErrorCode(AA_EXTERNAL_NTP_SERVERS_FAILURE);
		}
		if (NO == isServer2_failed)
		{
			m_isActiveAlarmNtpPeerAlreadyProduced[2] = NO;
			RemoveActiveAlarmByErrorCode(AA_EXTERNAL_NTP_SERVERS_FAILURE);
		}


		// ===== 3. if any of the servers failed (but not all three of them) - produce full_fault
		if ( (YES == isServer0_failed) && (NO == m_isActiveAlarmNtpPeerAlreadyProduced[0]) )
			ProduceNtpServerFailureFullFault(0, server0_ipStr);

		if ( (YES == isServer1_failed) && (NO == m_isActiveAlarmNtpPeerAlreadyProduced[1]) )
			ProduceNtpServerFailureFullFault(1, server1_ipStr);

		if ( (YES == isServer2_failed) && (NO == m_isActiveAlarmNtpPeerAlreadyProduced[2]) )
			ProduceNtpServerFailureFullFault(2, server2_ipStr);

	} // end else (no ActiveAlarm was produced)
}

/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::ProduceNtpServerFailureFullFault(int serverIdx, std::string ipStr)
{
	if ( (serverIdx >= 0) && (serverIdx < NTP_MAX_NUM_OF_SERVERS) )
		m_isActiveAlarmNtpPeerAlreadyProduced[serverIdx] = YES;

	CMedString errStr = "NTP server ";
	errStr << ipStr.c_str() << " failed";
	CHlogApi::TaskFault( FAULT_GENERAL_SUBJECT, EXTERNAL_NTP_SERVER_FAILURE, MAJOR_ERROR_LEVEL,
			             errStr.GetString(), TRUE/*=full_faults only*/ );

}

/////////////////////////////////////////////////////////////////////
BOOL CMcuMngrManager::IsPeerStatusFailed(STATUS serverStatus)
{
	BOOL isFailed = NO;

	if ( (STATUS_FAIL				== serverStatus) ||
	     (STATUS_NTP_PEER_REJECT	== serverStatus) ||
	     (STATUS_NTP_PEER_FALSETICK	== serverStatus) )
	{
		isFailed = YES;
	}

	return isFailed;
}

/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendNtpEventToAuditor(CStructTm timeBefore, CStructTm timeAfter)const
{
    CLargeString action;
    action << "System time changed";

    CLargeString description;
    description << "System time was changed";

    CLargeString descriptionEx;
    descriptionEx << "System time was changed from " << timeBefore << " to " << timeAfter;

    AUDIT_EVENT_HEADER_S outAuditHdr;
    CAuditorApi::PrepareAuditHeader(outAuditHdr,
                                    "",
                                    eMcms,
                                    "",
                                    "",
                                    eAuditEventTypeInternal,
                                    eAuditEventStatusOk,
                                    action.GetString(),
                                    description.GetString(),
                                    "",
                                    descriptionEx.GetString());
    CFreeData freeData;
    CAuditorApi::PrepareFreeData(freeData,
                                 "",
                                 eFreeDataTypeXml,
                                 "",
                                 "",
                                 eFreeDataTypeXml,
                                 "");
    CAuditorApi api;
    api.SendEventMcms(outAuditHdr, freeData);
}

void CMcuMngrManager::OnTimerNtpSyncTimeout(CSegment* pSeg)
{
	SyncNtp();
  StartTimer(NTP_SYNC_TIMER, m_NtpSyncPeriod*SECOND*60);
}

void CMcuMngrManager::OnTimerCertificatesDailyTimeout(CSegment* pSeg)
{
	StartTimer(MCUMNGR_CERTIFICATES_DAILY_TIMER, DAILY_TIMER_TIMEOUT);

	UpdateSecurityDependencies(*m_pMngmntIpParams_asIpService_fromProcess);
	CManagerApi api(eProcessCertMngr);
	CSegment *pSegEmpty = new CSegment;
	STATUS stat = api.SendMsg(pSegEmpty,CERTMNGR_VERIFY_CS_CERTIFICATE);
}

void CMcuMngrManager::OnTimerWaitForCardsStartupTimeout(CSegment* pSeg)
{
	int numberOfMediaCards = 4;
	//added "eProductFamilySoftMcu" by Richer for 802.1x project om 2013.12.26 
	if (eProductTypeRMX1500 == m_pProcess->GetProductType() || eProductFamilySoftMcu == CProcessBase::GetProcess()->GetProductFamily())
		numberOfMediaCards = 1;
	else if  (eProductTypeRMX2000 == m_pProcess->GetProductType())
			numberOfMediaCards = 2;

	Send802_1xConfigurationToMplApi(numberOfMediaCards);
}


/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnTimerNtpServersStatusTimeout(CSegment* pSeg)
{
	SampleNtpPeerStatus();
	StartTimer(NTP_SERVERS_STATUS_TIMER, m_NtpServersStatusPeriod*SECOND*60);
}

/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SyncNtp(BOOL isTimeAlreadyUpdated /*= FALSE*/,
		CStructTm *timeBeforeUpdate /*= NULL*/)
{
	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::SyncNtp";

	CStructTm curTimeBefore;
	SystemGetTime(curTimeBefore);

	// ===== 1. sync time
	CConfigManagerApi api;
	STATUS syncTimeStatus = STATUS_OK;

	if (isTimeAlreadyUpdated == FALSE)
	{
		syncTimeStatus = api.SyncTime();
	}
	else
	{
		if (timeBeforeUpdate)
		{
			curTimeBefore = *timeBeforeUpdate;
		}
	}

	if (STATUS_OK == syncTimeStatus)
	{
		RemoveActiveAlarmFaultOnlyByErrorCode(AA_NTP_SYNC_FAILURE);

		//check if time has changed
		CStructTm curTimeAfter;
		SystemGetTime(curTimeAfter);

		DWORD diffTime;
		if(curTimeAfter > curTimeBefore)
			diffTime = curTimeAfter - curTimeBefore;
		else
			diffTime = curTimeBefore - curTimeAfter;

		if(diffTime > 60) //more than 60 seconds
		{
			TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SyncNtp - Time has been changed! "
						<< " curTimeBefore: " << curTimeBefore
						<< " curTimeAfter: " << curTimeAfter;

			//send message to resource process
		    CManagerApi api(eProcessResource);
		    STATUS res = api.SendOpcodeMsg(MCUMNGR_TO_RSRCALLOC_TIME_CHANGED_IND);

		    // inform Auditor
		    SendNtpEventToAuditor(curTimeBefore, curTimeAfter);
		    // inform Authenticator
		    SendRMXTimeSetToAuthenticationProcess();

		    // VNGR-19897: PSO - Time related certificate issues
		    // Informs CertMngt to verify certificates with new the time
		    UpdateSecurityDependencies(*m_pMngmntIpParams_asIpService_fromProcess);
		    CManagerApi apiCert(eProcessCertMngr);
		    CSegment *pSegEmpty = new CSegment;
		    STATUS stat = apiCert.SendMsg(pSegEmpty,CERTMNGR_VERIFY_CS_CERTIFICATE);
		}
	}
	else // sync failed
	{
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SyncNtp - Sync NTP failed; status: " << m_pProcess->GetStatusAsString(syncTimeStatus);

		AddActiveAlarmFaultOnlySingleton( FAULT_GENERAL_SUBJECT,
		                         AA_NTP_SYNC_FAILURE,
		                         MAJOR_ERROR_LEVEL,
		                         "Failed to sync with NTP server");

	} // end else (sync failed)

	// ===== 2. Check year >= MIN_LEGAL_YEAR (2000)
	CheckMcuTimeYearValidity();
}

/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::CheckMcuTimeYearValidity()
{
	// ===== 1. get year
	CSystemTime sysTime;
	m_pProcess->GetMcuTime(sysTime);
	int year = sysTime.GetMCUTime()->m_year;

	// ===== 2. validate
	if ( MIN_LEGAL_YEAR <= year )
	{

		RemoveActiveAlarmByErrorCode(AA_ILLEGAL_TIME);
		m_isNtpSyncLegal = YES;
	}
	else
	{
		CSmallString yearStr;
		yearStr << "MCU year (" << year << ") must be  " << MIN_LEGAL_YEAR << " or later";

		AddActiveAlarmSingleton( FAULT_GENERAL_SUBJECT,
		                         AA_ILLEGAL_TIME,
		                         MAJOR_ERROR_LEVEL,
		                         yearStr.GetString(),
		                         true,
		                         true );


		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::CheckMcuTimeYearValidity -"
		                       << "\n" << yearStr.GetString();

		UpdateStartupConditionByErrorCode(AA_ILLEGAL_TIME, eStartupConditionFail);
		m_isNtpSyncLegal = NO;
	}
}

/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendStateRequestToAllProcesses()
{
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendStateRequestToAllProcesses";

	for (int i=0; i < NUM_OF_PROCESS_TYPES; i++)
	{
		CSegment* pParam = new CSegment;
		const COsQueue* pMbx = CProcessBase::GetProcess()->GetOtherProcessQueue( (eProcessType)i, eManager );
		if (pMbx)
		{
			pMbx->Send(pParam, MCUMNGR_TO_ALL_PROCESSES_STATE_REQ);
		}
	}
}

/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnNewCoreDumpInd(CSegment* pSeg)
{
	m_pProcess->GetMcuStateObject()->IncreaseNumOfCoreDumps();

	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::OnNewCoreDumpInd - updated number of core dumps: "
	                       << m_pProcess->GetMcuStateObject()->GetNumOfCoreDumps();
}


/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnMediaRecordingReq(CSegment* pSeg)
{
	RSRCALLOC_MEDIA_RECORDING_S *pMediaRecordingStruct = (RSRCALLOC_MEDIA_RECORDING_S*)pSeg->GetPtr();

	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::OnMediaRecordingReq - recording state: "
	                       << pMediaRecordingStruct->isRecording;

	m_pProcess->GetMcuStateObject()->SetMediaRecordingState( pMediaRecordingStruct->isRecording );
}

/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnCollectingInfoReq(CSegment* pSeg)
{
	COLLECTOR_COLLECTING_INFO_S *pCollectingInfoStruct = (COLLECTOR_COLLECTING_INFO_S*)pSeg->GetPtr();

	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::OnCollectingInfoReq - collectingInfo state: "
	                       << pCollectingInfoStruct->isCollecting;

	if (m_pProcess->GetMcuStateObject()->GetCollectingInfoState()==TRUE && pCollectingInfoStruct->isCollecting == FALSE)
	{ //Delay the update in 10 Seconds
		StartTimer(UPDATE_COLLECTING_STOP, UPDATE_COLLECTING_STOP_INTERVAL);
	}
	else
	{
		m_pProcess->GetMcuStateObject()->SetCollectingInfoState( pCollectingInfoStruct->isCollecting );
	}
}

/////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::OnUpdateCollectingStop(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::OnUpdateCollectingStop - collectingInfo state: FALSE";

	m_pProcess->GetMcuStateObject()->SetCollectingInfoState( FALSE );
	return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::AddFilterOpcodePoint()
{
	AddFilterOpcodeToQueue(TO_MCUMNGR_STATE_CHANGED);
}

////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendDebugModeToShelfMngr()
{
	BOOL isLegalValue = YES;
	CMedString logStr = "\nCMcuMngrManager::SendDebugModeToShelfMngr - ";
	logStr << "boardId: " << m_switchBoardId << ", subBoardId: " << m_switchSubBoardId;

	// ===== 1. read & validate SysConfig param
	CSysConfig *sysConfig = m_pProcess->GetSysConfig();
	const CCfgData *cfgData = sysConfig->GetCfgEntryByKey(CFG_KEY_DEBUG_MODE);
	if( false == CCfgData::TestValidity(cfgData) )
	{
		logStr << "\nFailed to validate sysConfig, key: " << CFG_KEY_DEBUG_MODE;
		TRACESTR(eLevelInfoNormal) << logStr.GetString();
		return;
	}

	std::string dataStr;
	sysConfig->GetDataByKey(CFG_KEY_DEBUG_MODE, dataStr);

	BOOL isDebugMode = FALSE;
	BOOL res = sysConfig->GetBOOLDataByKey(CFG_KEY_DEBUG_MODE, isDebugMode);

	if ( (FALSE == res) ||
	     ((CFG_STR_YES != dataStr) && (CFG_STR_NO  != dataStr)) ) 	// illegal data!
	{
		logStr << "\nIllegal DebugMode flag: "
		       << "key - " << CFG_KEY_DEBUG_MODE << ", value: " << dataStr.c_str()
		       << ". Nothing is sent to ShelfMngr!";
	}
	else
	{
		logStr << "\nDebugMode flag: " << dataStr.c_str();

		// ===== 2. send to Mfa
		CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol();

		if (CFG_STR_YES == dataStr)
		{
			mplPrtcl->AddCommonHeader(DEBUG_MODE_YES_REQ);
		}
		else
		{
			mplPrtcl->AddCommonHeader(DEBUG_MODE_NO_REQ);
		}

		mplPrtcl->AddMessageDescriptionHeader();
		mplPrtcl->AddPhysicalHeader(1, m_switchBoardId, m_switchSubBoardId);

		CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("MCUMNGR_SENDS_TO_MPL_API");
		mplPrtcl->SendMsgToMplApiCommandDispatcher();

		POBJDELETE(mplPrtcl);
	} // end (legal data)

	TRACESTR(eLevelInfoNormal) << logStr.GetString();
}

/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::UpdateLicensingWithParamsFromKeyCodes()
{
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::UpdateLicensingWithParamsFromKeyCodes";

//	UpdateLicensingWithParamsFrom_U_KeyCode();
	UpdateLicensingWithParamsFromFile();
	UpdateLicensingWithParamsFrom_X_KeyCode();
	// don't check BIOS in SoftwareMcu
	if( eProductFamilySoftMcu != CProcessBase::GetProcess()->GetProductFamily() )
		UpdateLicensingWithBIOSParams();
}

/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::UpdateLicensingWithParamsFromFile()//UpdateLicensingWithParamsFrom_U_KeyCode()
{
	VERSION_S mcuVer  = m_pSystemVersions->GetMcuVersion();
	char ptr [DESCRIPTION_LEN];
	strncpy(ptr, m_pSystemVersions->GetMcuDescription(),DESCRIPTION_LEN-1);
	ptr[DESCRIPTION_LEN - 1]='\0';
	m_pLicensing->SetMcuVersion(mcuVer,ptr);

//	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::UpdateLicensingWithParamsFrom_U_KeyCode";
//
//	VERSION_S kcVersion = GetKeycodeVersionFrom_U_KeycodeOrFromFile();
//	m_pLicensing->SetMcuVersion(kcVersion);
}

/////////////////////////////////////////////////////////////////////
bool CMcuMngrManager::IsSoftBasedMcu() const
{
	return 0
		|| this->IsPlatformOfNewArch()
		|| this->IsSoftMcu()
		;
}

bool CMcuMngrManager::IsPlatformOfNewArch() const
{
	return 0
		|| (eProductTypeGesher==m_mcuMngrProductType)
		|| (eProductTypeNinja==m_mcuMngrProductType)
		;
}

bool CMcuMngrManager::IsSoftMcu() const
{
	return 	(eProductFamilySoftMcu == ::ProductTypeToProductFamily(m_mcuMngrProductType) );

//	return 0
//		|| (eProductTypeSoftMCU==m_mcuMngrProductType)
//		|| (eProductTypeSoftMCUMfw==m_mcuMngrProductType)
//		;
}

void CMcuMngrManager::UpdateLicensingWithParamsFrom_X_KeyCode()
{
	if (this->IsPlatformOfNewArch())
	{
		this->UpdateLicensingWithParamsFrom_X_KeyCodeOnNewPlatforms();
	}
	else
	{
		this->UpdateLicensingWithParamsFrom_X_KeyCodeOnTraditionalPlatforms();
	}
}

namespace
{
	unsigned long GetOptionsMask(CKeyCode & keycode)
	{
		char szOptions[CFS_OPTIONS_BITMASK_LENGTH];
		memset(szOptions, 0, CFS_OPTIONS_BITMASK_LENGTH);
		keycode.GetOptionsFromKeyCode( szOptions, (char*)(keycode.GetKeyCode().GetString()) );
		assert(strlen(szOptions)<=8);
		return strtoul(szOptions, NULL, 16);
	}

	long const GESHER_MAX_NOMINAL_PORT_COUNT = GESHER_MAX_CP;
	long const NINJA_MAX_NOMINAL_PORT_COUNT = NINJA_MAX_CP;
	long BoundNominalAvcPartiesNew(eProductType productType, long nominalAvcParties)
	{
		if(eProductTypeGesher==productType)
        {
            return nominalAvcParties>GESHER_MAX_NOMINAL_PORT_COUNT ? GESHER_MAX_NOMINAL_PORT_COUNT : nominalAvcParties;
        }
        else if(eProductTypeNinja==productType)
        {
            return nominalAvcParties>NINJA_MAX_NOMINAL_PORT_COUNT ? NINJA_MAX_NOMINAL_PORT_COUNT : nominalAvcParties;
        }
        else
        {
            return nominalAvcParties;
        }
    }

	long BoundNominalSvcPartiesNew(int isSvcEnabled, eProductType productType, long nominalSvcParties)
	{
		if (!isSvcEnabled) return 0;
        return BoundNominalAvcPartiesNew(productType, nominalSvcParties);
	}

    int PARTY_MULTIPLIER = 5;
	long SvcNominalToActualParties(long nominal)
	{
		return 3*nominal*PARTY_MULTIPLIER;
	}

	long AvcNominalToActualParties(long nominal)
	{
		return 2*nominal*PARTY_MULTIPLIER; // count as CIF ports
	}
}
unsigned long GetFeatureMask(unsigned long optionsMask)
{
    return BitRangeToNumber(optionsMask, FEATURES_BIT_BEGIN, FEATURES_BIT_END);
}
int GetEncryptionFlag(unsigned long featuresMask)
{
    return (featuresMask & ENCRYPTION_MASK ? YES : NO);
}
int GetTelepresenceFlag(unsigned long featuresMask)
{
    return (featuresMask & TELEPRESENCE_MASK ? YES : NO);
}
int GetMultipleServiceFlag(unsigned long featuresMask)
{
    return (featuresMask & MULTIPLE_SERVICES_MASK ? YES : NO);
}

unsigned long GetPartnerMask(unsigned long optionsMask)
{
    return BitRangeToNumber(optionsMask, PARTNERS_BIT_BEGIN, PARTNERS_BIT_END);
}
int GetMpmxFlag(unsigned long partnersMask)
{
    return (partnersMask & MPMX_MASK) ? YES : NO;
}
int GetAvayaFlag(unsigned long partnersMask)
{
    return (partnersMask & AVAYA_MASK ? YES : NO);
}
int GetIBMFlag(unsigned long partnersMask)
{
    return (partnersMask & IBM_MASK ? YES : NO);
}

void CMcuMngrManager::UpdateLicensingWithParamsFrom_X_KeyCodeOnNewPlatforms()
{
	unsigned long const optionsMask = GetOptionsMask(m_X_KeyCode);
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

	{
		int const isSvcEnabled = IsBitOn(optionsMask, SVC_OPTION_BIT);

		long const nominalAvcParties = BoundNominalAvcPartiesNew(m_mcuMngrProductType, BitRangeToNumber(optionsMask, AVC_PORT_BIT_BEGIN, AVC_PORT_BIT_END));
		long const nominalSvcParties = isSvcEnabled ? nominalAvcParties : 0;//BoundNominalSvcPartiesNew(isSvcEnabled, m_mcuMngrProductType, BitRangeToNumber(optionsMask, SVC_PORT_BIT_BEGIN, SVC_PORT_BIT_END));

		m_pLicensing->SetTotalNumOfCopParties(0);
		m_pLicensing->SetIsSvcEnabled(isSvcEnabled);
		m_pLicensing->SetTotalNumOfSvcParties(SvcNominalToActualParties(nominalSvcParties));
		m_pLicensing->SetTotalNumOfCpParties(AvcNominalToActualParties(nominalAvcParties));
	}

	{
		unsigned long const featuresMask = GetFeatureMask(optionsMask);

		m_pLicensing->SetIsEncryptionEnabled((BYTE)GetEncryptionFlag(featuresMask));

		BYTE isPstn = NO; //(featuresMask & PSTN_MASK ? YES : NO);
		if(eProductTypeNinja == curProductType)
		{
		    isPstn = (featuresMask & PSTN_MASK ? YES : NO);
		}


		m_pLicensing->SetIsPstnEnabled(isPstn);

		m_pLicensing->SetIsTelepresenceEnabled((BYTE)GetTelepresenceFlag(featuresMask));

		// - - - 2.c.iv InternalScheduler - - -
		// 02.02.09: it's decided that InternalScheduler is not a licensed feature
		//    BYTE isInternalScheduler = (featuresMask & INTERNAL_SCHEDULER_MASK ? YES : NO);
		//    m_pLicensing->SetIsInternalSchedulerEnabled(isInternalScheduler);

		// - - - 2.c.iv MS - - -
		//    BYTE isMS = (featuresMask & MS_MASK ? YES : NO);
		//    m_pLicensing->SetIsMsEnabled(isMS);
		m_pLicensing->SetIsMultipleServicesEnabled((BYTE)GetMultipleServiceFlag(featuresMask));
	}

	{
		unsigned long const partnersMask = BitRangeToNumber(optionsMask, PARTNERS_BIT_BEGIN, PARTNERS_BIT_END);

		m_pLicensing->SetIsMPMXBitEnabled((BYTE)GetMpmxFlag(partnersMask));

		BYTE isHD = YES; // (partnersMask & HD_MASK) ? YES : NO;
		m_pLicensing->SetIsHDEnabled(isHD);

		if ( YES == isHD )
		{
			// count as HD720P ports
			int HD_port_CP = m_pLicensing->GetTotalNumOfCpParties() / 2;
			m_pLicensing->SetTotalNumOfCpParties(HD_port_CP);
		}

		m_pLicensing->SetIsAvaya((BYTE)GetAvayaFlag(partnersMask));

		m_pLicensing->SetIsIBM((BYTE)GetIBMFlag(partnersMask));

		//    BYTE isMicrosoft = (partnersMask & MICROSOFT_MASK ? YES : NO);
		//    m_pLicensing->SetIsMicrosoft(isMicrosoft);
	}
}

BYTE GetSvcBitFlag(char digit)
{
    if (!isxdigit(digit))
    {
        return NO;
    }

    char tmpStr[2] = { digit, 0 };
    long bitMask = strtol(tmpStr, NULL, 16);
    if (1 & bitMask)
    {
        return YES;
    }
    else
    {
        return NO;
    }
}

void CMcuMngrManager::UpdateLicensingWithParamsFrom_X_KeyCodeOnTraditionalPlatforms()
{
    char tmpStr[3];

	unsigned long const optionsMask = GetOptionsMask(m_X_KeyCode);
	int const isHdPortsUnit = IsBitOn(optionsMask, keycode::PORTS_UNIT_BIT);

	// ===== 1. extract Options bitmask
    char szOptions[CFS_OPTIONS_BITMASK_LENGTH];
    memset(szOptions, 0, CFS_OPTIONS_BITMASK_LENGTH);
    m_X_KeyCode.GetOptionsFromKeyCode( szOptions, (char*)(m_X_KeyCode.GetKeyCode().GetString()) );

    if (m_mcuMngrProductType==eProductTypeSoftMCU)
    {
    	m_pLicensing->SetIsSvcEnabled(YES);
    }
    else if (m_mcuMngrProductType==eProductTypeSoftMCUMfw)
    {
    	m_pLicensing->SetIsSvcEnabled(YES);
    	m_pLicensing->SetIsTipEnabled(NO);      //BRIDGE-11854 only MFW has NO value for tip & AvcCifPlus
    	m_pLicensing->SetIsAvcCifPlusEnabled(NO);
    }
    else
    {

    	int isSvcEnabled = IsBitOn(optionsMask, keycode::SVC_OPTION_BIT);

		/* In case PREFER_SVC_OVER_LYNC_CAPACITY = NO and SVC license bit = YES,
		 * We will turn off the SVC anyway for MPMX cards to allow admin scaling up the Lync connected users */
		BOOL isPreferSvcOverLyncCapacity = YES; // PREFER_SVC_OVER_LYNC_CAPACITY
		CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("PREFER_SVC_OVER_LYNC_CAPACITY", isPreferSvcOverLyncCapacity);

		if (isSvcEnabled && !isPreferSvcOverLyncCapacity)
		{
			unsigned long const partnersMask = BitRangeToNumber(optionsMask, PARTNERS_BIT_BEGIN, PARTNERS_BIT_END);

			if ((BYTE)GetMpmxFlag(partnersMask))
			{
			    TRACESTR(eLevelWarn) << "\nCMcuMngrManager::UpdateLicensingWithParamsFrom_X_KeyCodeOnNewPlatforms - isSvcEnabled in license = "
		    		<< (isSvcEnabled?" YES ":" NO ") <<  " PREFER_SVC_OVER_LYNC_CAPACITY system flag = "
		    		<<  (isPreferSvcOverLyncCapacity?"YES ":"NO ") << " Card = MPMx, SVC will be disabled in MCU";

			    isSvcEnabled = isPreferSvcOverLyncCapacity && isSvcEnabled;
			}
		}

    	m_pLicensing->SetIsSvcEnabled(isSvcEnabled);
    }
    m_pLicensing->SetIsHdPortsUnit(IsBitOn(optionsMask, keycode::PORTS_UNIT_BIT));

    TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::UpdateLicensingWithParamsFrom_X_KeyCodeOnTraditionalPlatforms - key=" << m_X_KeyCode.GetKeyCode().GetString() << "; optionsMask="
    		<< std::hex << optionsMask << "; Options=" << szOptions << "; SvcEnabled = " << (int)m_pLicensing->GetIsSvcEnabled() << "; HdPorts = " << (int)m_pLicensing->GetIsHdPortsUnit() << "." ;

    // ===== 2. parse Options bits
    // ------------ 2.a Num of COP participants ------------
    long numOfCopPortsFromKeyCode = 0;

    if ( isxdigit(szOptions[COP_PORTS_NUM_FIRST_DIGIT]) &&
            isxdigit(szOptions[COP_PORTS_NUM_SECOND_DIGIT]) )
    {
        tmpStr[0] = szOptions[COP_PORTS_NUM_FIRST_DIGIT];
        tmpStr[1] = szOptions[COP_PORTS_NUM_SECOND_DIGIT];
        tmpStr[2] = 0;

        numOfCopPortsFromKeyCode = strtol(tmpStr, NULL, 16);
    }
    // --------------- 2.b Partners -------------------
    long partnersDigitVal = 0;
    if ( isxdigit(szOptions[PARTNERS_DIGITS]) )
    {
        tmpStr[0] = szOptions[PARTNERS_DIGITS];
        tmpStr[1] = 0;
        tmpStr[2] = 0;
        //      partnersDigitVal = atoi(tmpStr);
        partnersDigitVal = strtol(tmpStr, NULL, 16);
    }

    m_pLicensing->SetTotalNumOfCopParties( (int)numOfCopPortsFromKeyCode * COP_PORTS_NUM_INTERVAL );



    // ------------ 2.b Num of CP participants ------------
    long numOfCpPortsFromKeyCode = 0;

    if ( isxdigit(szOptions[CP_PORTS_NUM_FIRST_DIGIT]) &&
            isxdigit(szOptions[CP_PORTS_NUM_SECOND_DIGIT]) )
    {
        tmpStr[0] = szOptions[CP_PORTS_NUM_FIRST_DIGIT];
        tmpStr[1] = szOptions[CP_PORTS_NUM_SECOND_DIGIT];
        tmpStr[2] = 0;

        //      numOfCpPortsFromKeyCode = atoi(tmpStr);
        numOfCpPortsFromKeyCode = strtol(tmpStr, NULL, 16);
		  TRACESTR(eLevelWarn) << "\nCMcuMngrManager::numOfCpPortsFromKeyCode " << numOfCpPortsFromKeyCode  ;


		if (eProductTypeRMX4000 == m_mcuMngrProductType)
		{
			if ( MAX_NUM_OF_CP_PORTS_IN_KEYCODE_RMX4000 < numOfCpPortsFromKeyCode )
				numOfCpPortsFromKeyCode = MAX_NUM_OF_CP_PORTS_IN_KEYCODE_RMX4000;
		}
		else if (eProductTypeSoftMCUMfw != m_mcuMngrProductType)
		{
			if ( MAX_NUM_OF_CP_PORTS_IN_KEYCODE_RMX2000 < numOfCpPortsFromKeyCode )
				numOfCpPortsFromKeyCode = MAX_NUM_OF_CP_PORTS_IN_KEYCODE_RMX2000;
		}
    }

    // --------------- 2.c Features -------------------
    long featuresDigitVal = 0;
    if ( isxdigit(szOptions[FEATURES_DIGIT]) )
    {
        tmpStr[0] = szOptions[FEATURES_DIGIT];
        tmpStr[1] = 0;
        tmpStr[2] = 0;
        featuresDigitVal = strtol(tmpStr, NULL, 16);
    }

    // - - - 2.c.i Encryption (AES) - - -
    BYTE isEnc = (featuresDigitVal & ENCRYPTION_MASK ? YES : NO);
    m_pLicensing->SetIsEncryptionEnabled(isEnc);

    // - - - 2.c.ii PSTN - - -
    BYTE isPstn = (featuresDigitVal & PSTN_MASK ? YES : NO);
    m_pLicensing->SetIsPstnEnabled(isPstn);

    // - - - 2.c.iii Telepresence - - -
    BYTE isTelepresence = (featuresDigitVal & TELEPRESENCE_MASK ? YES : NO);
    m_pLicensing->SetIsTelepresenceEnabled(isTelepresence);

    // - - - 2.c.iv InternalScheduler - - -
// 02.02.09: it's decided that InternalScheduler is not a licensed feature
//    BYTE isInternalScheduler = (featuresDigitVal & INTERNAL_SCHEDULER_MASK ? YES : NO);
//    m_pLicensing->SetIsInternalSchedulerEnabled(isInternalScheduler);

    // - - - 2.c.iv MS - - -
//    BYTE isMS = (featuresDigitVal & MS_MASK ? YES : NO);
//    m_pLicensing->SetIsMsEnabled(isMS);

    BYTE isMultipleServices = (featuresDigitVal & MULTIPLE_SERVICES_MASK ? YES : NO);
    m_pLicensing->SetIsMultipleServicesEnabled(isMultipleServices);

    if (eProductTypeSoftMCUMfw == m_mcuMngrProductType)
    {
    	//from SRS (Anat L.) : 67 cannot be divided by 5 (MPMx bit). The number in license should be 67, MPMx bit should NOT multiply by 5.
    	m_pLicensing->SetTotalNumOfCpParties( (int)numOfCpPortsFromKeyCode);
    }
    else
    {
    	m_pLicensing->SetTotalNumOfCpParties( (int)numOfCpPortsFromKeyCode * CP_PORTS_NUM_INTERVAL );
		  TRACESTR(eLevelWarn) << "\nCMcuMngrManager::UpdateLicensingWithParamsFrom_X_KeyCodeOnTraditionalPlatforms cp ports " << numOfCpPortsFromKeyCode * CP_PORTS_NUM_INTERVAL  ;

    }

    // Tsahi TODO: need to remove MPMX_MASK and m_isMPMXBitEnabled... (not in use since v8.0)
    BYTE isMpmx = (partnersDigitVal & MPMX_MASK) ? YES : NO;
    m_pLicensing->SetIsMPMXBitEnabled(isMpmx);

    // check HD bit only for RMX1500
    BYTE isHD;
    if (! (partnersDigitVal & HD_MASK))
    {
        if (eProductTypeRMX1500 != m_mcuMngrProductType)
        {
            TRACEINTO << "HD is always enable for "
                      << ProductTypeToString(m_mcuMngrProductType);
            isHD = YES;
        }
        else
        {
            TRACEINTO << "HD is disabled for "
                      << ProductTypeToString(m_mcuMngrProductType);
           isHD = NO;
        }
    }
    else
    {
        TRACEINTO << "HD_MASK is turned on for "
                  << ProductTypeToString(m_mcuMngrProductType)
                  << ", partnersDigitVal=" << partnersDigitVal
                  << ", HD_MASK=" << HD_MASK;
        isHD = YES;
    }
    m_pLicensing->SetIsHDEnabled(isHD);

    BYTE isAvaya = (partnersDigitVal & AVAYA_MASK ? YES : NO);
    m_pLicensing->SetIsAvaya(isAvaya);

    BYTE isIBM = (partnersDigitVal & IBM_MASK ? YES : NO);
    m_pLicensing->SetIsIBM(isIBM);

//    BYTE isMicrosoft = (partnersDigitVal & MICROSOFT_MASK ? YES : NO);
//    m_pLicensing->SetIsMicrosoft(isMicrosoft);
}

////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::EnableHDLicenseInNon1500Q()
{
    TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__ << " Update HD License to TRUE in Non 1500Q for EMA License Info";
    //fix VNGR-22501 , HD license by default will be YES, for 1500Q it will be updated from ConfParty to NO from internal Indication of resources
    PASSERT_AND_RETURN(NULL == m_pLicensing);
    m_pLicensing->SetIsHDEnabled(YES);
}
////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::UpdateHDLicensePortsIn1500Q()
{
	   TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__ << " Update HD License ports in 1500Q for EMA License Info";
	   PASSERT_AND_RETURN(NULL == m_pLicensing);
	   DWORD numOfCpPorts = m_pLicensing->GetTotalNumOfCpParties();
	   if (numOfCpPorts >= 10)
		   m_pLicensing->SetTotalNumOfCpParties(7);
}

////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::UpdateLicensingWithBIOSParams()
{
    char bufBiosDetails[256];
	std::string stBiosDate = "";
	GetBiosDate(stBiosDate);

	std::string stBiosVersion = "";
	GetBiosVersion(stBiosVersion);

	std::string stBiosVendor = "";
	GetBiosVendor(stBiosVendor);

    snprintf(bufBiosDetails, sizeof(bufBiosDetails), "%s %s %s", stBiosDate.c_str(), stBiosVendor.c_str(), stBiosVersion.c_str());
	//std::string stBiosDetails = stBiosDate + " " + stBiosVendor + " " + stBiosVersion;

	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::UpdateLicensingWithBIOSParams" << bufBiosDetails;

	m_pLicensing->SetBiosDetails(bufBiosDetails);
//    BYTE isMicrosoft = (partnersDigitVal & MICROSOFT_MASK ? YES : NO);
//    m_pLicensing->SetIsMicrosoft(isMicrosoft);

}



/////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnBackupInProgressReq(CSegment* pSeg)
{
	BYTE backUpProgress = (BYTE)eBackup_Success;
	*pSeg >> backUpProgress;

	if(eBackup_InProgress == backUpProgress)
	{
		StartTimer(BACKUP_TIMEOUT, BACKUP_RESTORE_TIMEOUT); // to unset in progress flag
	}
	else
	{
		DeleteTimer(BACKUP_TIMEOUT);
	}

	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnBackupInProgressReq() - backup state: "
	                       << ( (backUpProgress == eBackup_InProgress) ? "in progress" : "finish");

	m_pProcess->GetMcuStateObject()->SetBackupState( backUpProgress );
}

/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnRestoreInProgressReq(CSegment* pSeg)
{
	BYTE restoreProgress = (BYTE)eRestore_Success;
	*pSeg >> restoreProgress;

	if(eRestore_InProgress == restoreProgress)
	{
		StartTimer(RESTORE_TIMEOUT, BACKUP_RESTORE_TIMEOUT); // to unset in progress flag
	}
	else
	{
		DeleteTimer(RESTORE_TIMEOUT);
	}

	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnRestoreInProgressReq() - restore state: "
						   << ( (restoreProgress == eRestore_InProgress) ? "in progress" : "finish");

	m_pProcess->GetMcuStateObject()->SetRestoreState( restoreProgress );
}

/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnBackupTimeout(CSegment* pSeg)
{
	cout << "CMcuMngrManager::OnBackupTimeout()" << endl;
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnBackupTimeout()";
	m_pProcess->GetMcuStateObject()->SetBackupState( eBackup_FailureTimeout );
}

/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnRestoreTimeout(CSegment* pSeg)
{
	cout << "\nCMcuMngrManager::OnRestoreTimeout()" << endl;
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnRestoreTimeout()";
	m_pProcess->GetMcuStateObject()->SetRestoreState( eRestore_FailureTimeout );
}

/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnGetMcuVersionReq(CSegment* pSeg)
{
	// ===== 1. Get Mcu version
	VERSION_S mcuVer  = m_pSystemVersions->GetMcuVersion();

	// ===== 2. insert the appropriate value to a segment
	CSegment*  pRetParam = new CSegment();
	pRetParam->Put( (BYTE*)&mcuVer, sizeof(mcuVer));

	// ===== 3. send to BackupRestore
	const COsQueue* pBackupRestoreMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessBackupRestore, eManager);

	STATUS res = pBackupRestoreMbx->Send(pRetParam, MCUMNGR_TO_BACKUPRESTORE_SYS_VERSION_IND);
}


///////////////////////////////////////////////////////////////////////////



void CMcuMngrManager::ConfigQosManagementNetwork()
{
	std::string data;
	CSysConfig* cfg = m_pProcess->GetSysConfig();
	BOOL res = cfg->GetDataByKey(QOS_MANAGEMENT_NETWORK, data);
	PASSERTSTREAM_AND_RETURN(!res, "CSysConfig::GetDataByKey: " << QOS_MANAGEMENT_NETWORK);
	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::ConfigQosManagementNetwork, data=" << data;
	SendQosManagementDSCPToProcess(data, eProcessConfigurator);
}

void CMcuMngrManager::SendQosManagementDSCPToMplApi()
{
	NTP_DSCP_REQUEST_S ntpDscpRequest;

	ntpDscpRequest.ulNtpDscp = GetValidatedDscpValue();
	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::SendQosManagementDSCPToMplApi, ulNtpDscp(decimal) = " << ntpDscpRequest.ulNtpDscp;

	CMplMcmsProtocol* mplPrtcl = new CMplMcmsProtocol();

	mplPrtcl->AddCommonHeader(MPL_SW_SET_NTP_DSCP_REQ);
	mplPrtcl->AddMessageDescriptionHeader();
	mplPrtcl->AddPhysicalHeader(1, m_switchBoardId, m_switchSubBoardId);
	mplPrtcl->AddData(sizeof(NTP_DSCP_REQUEST_S), (char*)&ntpDscpRequest);

	CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("MPL_SW_SET_NTP_DSCP_REQ");
	mplPrtcl->SendMsgToMplApiCommandDispatcher();

	POBJDELETE(mplPrtcl);
}




/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::TreatDnsConfigurationNewStatus( const eDnsConfigurationStatus newStatus,
                                                      BOOL isToRemoveDnsAlarm)
{
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::TreatDnsConfigurationNewStatus; "
	                       << "Status: " << DnsConfigurationStatusToString(newStatus) 
	                       << ", m_dnsConfigurationStatus:" << m_dnsConfigurationStatus;

	// ===== 1. Dns configuration status
	if (m_dnsConfigurationStatus != newStatus)
	{
		// ----- 1a. update member
		m_dnsConfigurationStatus = newStatus;

        TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::TreatDnsConfigurationNewStatus "
            << " m_dnsConfigurationStatus: " << m_dnsConfigurationStatus;
            
		// ----- 1b. update ActiveAlarm
		if ( (eDnsConfigurationSuccess == newStatus) || (YES == isToRemoveDnsAlarm) )
		{
			bool isFoundAA = RemoveActiveAlarmByErrorCode(AA_NO_DNS_CONFIGURATION);
			m_bRestoreNoDnsConfiguration_AA = FALSE;
			if (isFoundAA == false)
			{
				StartTimer(FAILED_REMOVED_AA_TIMER, 5*SECOND);
			}
		}
		else if (eDnsConfigurationFailure == newStatus)
		{
			UpdateStartupConditionByErrorCode(AA_NO_DNS_CONFIGURATION, eStartupConditionFail);
			m_bRestoreNoDnsConfiguration_AA = TRUE;
		}

		// ----- 1c. update DnsAgent
		SendDnsConfigStatusToDnsAgent();

		// ----- 1d. update SipProxy
		SendDnsConfigStatusToSipProxy();
	}

	// ===== 2. Remove Alert
	if (YES == isToRemoveDnsAlarm)
	{
		bool isFoundAA = RemoveActiveAlarmByErrorCode(AA_NO_DNS_CONFIGURATION);
		m_bRestoreNoDnsConfiguration_AA = FALSE;
		if (isFoundAA == false)
		{
			StartTimer(FAILED_REMOVED_AA_TIMER, 5*SECOND);
		}
	}
}



void CMcuMngrManager::OnTimerFailedRemoveAA(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\n OnTimerFailedRemoveAA";
	DeleteTimer(FAILED_REMOVED_AA_TIMER);
	bool isFoundAA = RemoveActiveAlarmByErrorCode(AA_NO_DNS_CONFIGURATION);

	if(isFoundAA == true)
	{
		TRACESTR(eLevelInfoNormal) << "\n OnTimerFailedRemoveAA DeleteTimer AA_NO_DNS_CONFIGURATION";
	}

	isFoundAA = RemoveActiveAlarmByErrorCode(AA_DNS_REGISTRAION_FAILED);
	if(isFoundAA == true)
	{
		TRACESTR(eLevelInfoNormal) << "\n OnTimerFailedRemoveAA DeleteTime AA_DNS_REGISTRAION_FAILED";
	}
}
/////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::SampleDhcpIp()
{
	STATUS retStatus = STATUS_OK;

	// ===== 1. loop of 10 attempts to retrieve info from Dhcp
	CConfigManagerApi api;
	string ipFromDhcp;
	while (MAX_NUM_OF_DHCP_SAMPLING > m_dhcpIpTestCounter)
	{
		// test dhcp
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SampleDhcpIp - iteration no. " << m_dhcpIpTestCounter+1;

		retStatus = api.GetLastDHCPConfig("IPADDR", ipFromDhcp);
		if ( !(ipFromDhcp.empty()) ) // dhcp succeeded!
			break;

		sleep(SECOND);//StartTimer(TEST_DHCP_IP_TIMER, SECOND);

		 m_dhcpIpTestCounter++;
	} // end loop


	if ( ipFromDhcp.empty() ) // dhcp failed
	{
		retStatus = STATUS_FAIL;
	}
	else // dhcp succeeded
	{
		UpdateDhcpParamsMembers();
	}

	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SampleDhcpIp (iteration no. " << m_dhcpIpTestCounter+1 << ") - "
	                       << "value: "    << ipFromDhcp
	                       << ". Status: " << m_pProcess->GetStatusAsString(retStatus).c_str();

	return retStatus;
}

/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnTestDhcpIpTimeout()
{
	SampleDhcpIp();
}

/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnTimerStartupTimeout(CSegment* pSeg)
{
	m_isStartupTimoutReached = YES;
	RecalculateMcuState();

	RemoveActiveAlarmByErrorCode(AA_INSTALLING_NEW_VERSION);

	if (m_mcuMngrProductType == eProductTypeEdgeAxis &&  m_pProcess->IsFlexeraLicenseInSysFlag() == true && (m_pProcess->IsFlexeraCapabilityEnabled(RPCS) == false))
	{

		if ( false == IsActiveAlarmExistByErrorCode(AA_LICENSE_ACQUISITION_FAIL) )
			AddActiveAlarm(FAULT_GENERAL_SUBJECT,AA_LICENSE_ACQUISITION_FAIL,MAJOR_ERROR_LEVEL,"Valid license not found",true, true);
	}
}

/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnTimerSystemStartupRemainingTime()
{
	// ===== 1. get params
	int oldRemining = m_pProcess->GetMcuStateObject()->GetRemainingTimeForStartup();
	int newRemining = oldRemining - SECOND;
    eMcuState mcuCurState = m_pProcess->GetMcuStateObject()->GetMcuState();

	// ===== 2. print
    TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnTimerSystemStartupRemainingTime"
    					   << "\nPrevious remaining period: " << oldRemining << " (" << (oldRemining / SECOND) << " seconds)"
    					   << "\nCurrent  remaining period: " << newRemining << " (" << (newRemining / SECOND) << " seconds)"
    					   << "\nMcuState: " << GetMcuStateName(mcuCurState);

    // never go below zero
    if (0 > newRemining)
    	newRemining = 0;


    // ===== 3. set 'remaining'
    if (eMcuState_Startup == mcuCurState)
    {
    	//	----- 3a.
    	//	not all processes got out of startup (eMcuState_Startup == mcuCurState)
        //	and also startup_timeout has not been reached on McuMngr (0 < newRemining),
    	if (0 < newRemining)
    	{
    		m_pProcess->GetMcuStateObject()->SetRemainingTimeForStartup(newRemining);
    	}

    	//	----- 3b.
    	//	startup_timeout may have been reached on McuMngr (newRemining <= 0),
    	//	but not on all other processes [since some of them are loaded a bit after McuMngr] (eMcuState_Startup == mcuCurState)
    	else
    	{
    		// delay the 'remaining' a little (1 sec), until mcu gets out of startup
    		m_pProcess->GetMcuStateObject()->SetRemainingTimeForStartup(1*SECOND);
    	}

    	StartTimer(SYSTEM_STARTUP_REMAINING_TIME_TIMER, SECOND);
    }

	//	----- 3c.
	//	all processes got out of startup (eMcuState_Startup != mcuCurState)
    else
    {
    	m_pProcess->GetMcuStateObject()->SetRemainingTimeForStartup(0);
    }
}

/////////////////////////////////////////////////////////////////////
/*
void CMcuMngrManager::OnTestDhcpDnsTimeout()
{
	SampleDhcpDns();
}
*/
/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::UpdateDhcpParamsMembers()
{
    TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::UpdateDhcpParamsInMembers";

   	// ===== 1. retrieve params from dhcp
  	string dhcpIpStr,
  	       dhcpMaskStr,
  	       dhcpGwStr,
  	       dhcpDnsStr;

	CConfigManagerApi api;
	api.GetLastDHCPConfig("IPADDR", dhcpIpStr);
	api.GetLastDHCPConfig("NETMASK", dhcpMaskStr);
	api.GetLastDHCPConfig("GATEWAY", dhcpGwStr);
	api.GetFirstDNSFromDHCP(dhcpDnsStr);

	// ===== 2. in order to eliminate any '\n' character etc.
	char ipArr[IP_ADDRESS_LEN+1],
	     maskArr[IP_ADDRESS_LEN+1],
	     gwArr[IP_ADDRESS_LEN+1],
	     dnsArr[IP_ADDRESS_LEN+1];

	memset(ipArr,	0, IP_ADDRESS_LEN+1);
	memset(maskArr,	0, IP_ADDRESS_LEN+1);
	memset(gwArr,	0, IP_ADDRESS_LEN+1);
	memset(dnsArr,	0, IP_ADDRESS_LEN+1);

	memcpy(ipArr,	dhcpIpStr.c_str(),		IP_ADDRESS_LEN);
	memcpy(maskArr, dhcpMaskStr.c_str(),	IP_ADDRESS_LEN);
	memcpy(gwArr,   dhcpGwStr.c_str(),		IP_ADDRESS_LEN);
	memcpy(dnsArr,	dhcpDnsStr.c_str(),		IP_ADDRESS_LEN);

	int ipLen	= dhcpIpStr.length(),
	    maskLen	= dhcpMaskStr.length(),
	    gwLen	= dhcpGwStr.length(),
	    dnsLen	= dhcpDnsStr.length();

	if ( (IP_ADDRESS_LEN >= ipLen) && (ipLen >= 1) && ('\n' == ipArr[ipLen-1]) )
		ipArr[ipLen-1] = 0;
	if ( (IP_ADDRESS_LEN >= maskLen) && (maskLen >= 1) && ('\n' == maskArr[maskLen-1]) )
		maskArr[maskLen-1] = 0;
	if ( (IP_ADDRESS_LEN >= gwLen) && (gwLen >= 1) && ('\n' == gwArr[gwLen-1]) )
		gwArr[gwLen-1] = 0;
	if ( (IP_ADDRESS_LEN >= dnsLen) && (dnsLen >= 1) && ('\n' == dnsArr[dnsLen-1]) )
		dnsArr[dnsLen-1] = 0;

	// ===== 3. update members
	m_dhcp_ipStr			= ipArr;
	m_dhcp_MaskStr			= maskArr;
	m_dhcp_GwStr			= gwArr;
	m_dhcp_dnsServerIpStr	= dnsArr;


    TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::UpdateDhcpParamsInMembers"
                           << "\nIp: "		<< m_dhcp_ipStr
                           << ", Mask: "	<< m_dhcp_MaskStr
                           << ", GW: "		<< m_dhcp_GwStr
                           << ", DNS: "		<< m_dhcp_dnsServerIpStr;
}

/*
/////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::CompareDhcpIpWithInterfaceIp()
{
	STATUS retStatus = STATUS_OK;

	// ===== 1. get ipAddress from interface
	eConfigInterfaceNum mngmntIfNum = GetInterfaceNum(eManagmentNetwork);
	string sMngmntNI = GetConfigInterfaceNumName(mngmntIfNum);

	// the following is needed for adding the ":1"
	eProductType curProductType = m_pProcess->GetProductType();
	if (eProductTypeRMX2000 == curProductType)
	{
		::GetNicName(eIpType_IpV4);
	}

	DWORD ipFromInterface = 0xffffffff;
	int status = GetNiIpAddr(sMngmntNI.c_str(), &ipFromInterface);

	CLargeString errStr = "\nCMcuMngrManager::CompareDhcpIpWithInterfaceIp - ";
	if (IFCONFIG_SOCKET_FAIL == status)
	{
		errStr << "Failed to open socket during access to Network Interface";
		PASSERTMSG( TRUE, errStr.GetString() );
		TRACESTR(eLevelInfoNormal) << errStr.GetString();

		retStatus = STATUS_SYSCALL_FAILURE;
	}
	else if (IFCONFIG_IOCTL_FAIL == status)
	{
		errStr << "Failed to IOCTL during access to Network Interface";
		PASSERTMSG( TRUE, errStr.GetString() );
		TRACESTR(eLevelInfoNormal) << errStr.GetString();

		retStatus = STATUS_SYSCALL_FAILURE;
	}

	if (STATUS_OK == retStatus) // succeed to get ipAddress from interface
	{
		// ===== 2. get ipAddress from dhcp
		DWORD ipFromDhcp = ::SystemIpStringToDWORD( m_dhcp_ipStr.c_str() );

		// ===== 3. compare
		if (ipFromInterface != ipFromDhcp)
			retStatus = STATUS_FAIL;

		char interfaceIpStr[IP_ADDRESS_LEN];
		SystemDWORDToIpString(ipFromInterface, interfaceIpStr);
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::CompareDhcpIpWithInterfaceIp -"
		                       << "\nIp from dhcp: "      << m_dhcp_ipStr.c_str()
		                       << "; Ip from interface: " << interfaceIpStr;
	}

	return retStatus;
}
*/

/////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::RegisterDnsClient(CIPService *pService, BOOL isDnsAuto)
{
	STATUS retStatus = STATUS_OK;

	CIpDns* pTmpDns = pService->GetpDns();
	if (!pTmpDns)
	{
		PASSERTMSG(TRUE, "CMcuMngrManager::RegisterDnsClient - dns is NULL!");
		return STATUS_FAIL;
	}

	if ( FALSE == pTmpDns->GetRegisterDNSAutomatically() )
	{
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::RegisterDnsClient - "
		                       << "dns not AutoRegister; therefore nothing is actually done";
		return retStatus;
	}

	string myIpStr = "", myIpV6Str = "", myHostName, myZone = "", dnsServerIpStr = "";
	char myIpArr[IP_ADDRESS_LEN+1], dnsServerIpArr[IP_ADDRESS_LEN];
	memset(myIpArr, 0, IP_ADDRESS_LEN+1);

   	CIPSpan* pTmpSpan  = pService->GetSpanByIdx(0); // Mngmnt params are stored in the 1st span (idx==0)
   	eIpType ipType = pService->GetIpType();

	// ===== 1. retrieve myIp
	if (YES == isDnsAuto)
	{
		myIpStr = m_dhcp_ipStr;
	}
	else
	{
		if( pTmpSpan )
		{
			if(eIpType_IpV6 != ipType)
			{
				DWORD myIp = pTmpSpan->GetIPv4Address();
				SystemDWORDToIpString(myIp, myIpArr);
				myIpStr = myIpArr;
			}

			if(eIpType_IpV4 != ipType)
			{
				myIpV6Str = pTmpSpan->GetIPv6Address(0);
			}
		}
		else
			PTRACE(eLevelError,"CMcuMngrManager::RegisterDnsClient - pTmpSpan is NULL!!");
	}

	// ===== 2. retrieve hostName
	//CIpDns* pTmpDns = m_pMngmntIpParams_asIpService_fromProcess->GetpDns();
	//myHostName = pTmpDns->GetHostServiceName().GetString();
	myHostName =GetHostNameFromService(pService);

	// ===== 3. retrieve zone
	myZone = pTmpDns->GetDomainName().GetString();

	// ===== 4. retrieve dnsServerIp
	if (YES == isDnsAuto)
	{
		dnsServerIpStr = m_dhcp_dnsServerIpStr;
	}
	else
	{
		DWORD dnsIp = pTmpDns->GetIPv4Address(0);
		if(dnsIp != 0)
		{
		SystemDWORDToIpString(dnsIp, dnsServerIpArr);
		dnsServerIpStr = dnsServerIpArr;
		}
		else
		{
			char ipv6[IPV6_ADDRESS_LEN];
			ipv6[0] = '\0';
			pTmpDns->GetIPv6Address(0, ipv6);
			dnsServerIpStr = ipv6;
		}
	}

	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::RegisterDnsClient"
	                       << "\nDns Server: "	<< dnsServerIpStr
	                       << "; HostName: "	<< myHostName
	                       << "; Zone: "		<< myZone
	                       << "; Ip: "			<< myIpStr;

	// ===== 5. register as client
	CConfigManagerApi api;
	retStatus = api.RegisterHostNameInDNS(dnsServerIpStr, myHostName, myZone, myIpStr, myIpV6Str);

	return retStatus;
}

/////////////////////////////////////////////////////////////////////
/*void CMcuMngrManager::OnTimerIpV6AutoConfig(CSegment* pSeg)
{
	m_IpV6AutoConfigAccumulatedTime += CONFIG_IPV6_AUTO_INTERVAL;

	TRACEINTO << "\nCMcuMngrManager::OnTimerIpV6AutoConfig - accumulated time: "
			  << (m_IpV6AutoConfigAccumulatedTime/SECOND) << " seconds";

	// ===== 1. timeout was not reached yet - keep on sampling...
	if(m_IpV6AutoConfigAccumulatedTime < CONFIG_IPV6_AUTO_TIMEOUT)
	{
		CIPService tmpService;
		bool success = RetrieveIpV6AutoAndConfig(tmpService);
		if(!success)
		{
			//m_IpV6AutoConfigState = eIPv6AutoConfig_InProgress;
			StartTimer(MCUMNGR_IPV6_AUTO_CONFIG_TIMER, CONFIG_IPV6_AUTO_INTERVAL);
			TRACEINTO << "\nCMcuMngrManager::OnTimerIpV6AutoConfig - currently failed to retrieve auto ipV6; reattempting...";
		}

		else // retrieving addresses succeeded
		{
			TRACEINTO << "\nCMcuMngrManager::OnTimerIpV6AutoConfig - auto ipV6 is retrieved successfully";

			m_IpV6AutoConfigState = eIPv6AutoConfig_Success;
			m_IpV6AutoConfigAccumulatedTime = 0; // for debugging (to see if it starts again)

			if (eIpTypeConfigSuccess_None == m_isIpConfigInOsSucceeded)
			{
				m_isIpConfigInOsSucceeded = eIpTypeConfigSuccess_IPv6;
			}
			else if(eIpTypeConfigSuccess_IPv4 == m_isIpConfigInOsSucceeded)
			{
				m_isIpConfigInOsSucceeded = eIpTypeConfigSuccess_Both;
			}
			ConfigGeneral();

			CMcmsDaemonApi daemonApi;
			daemonApi.SendConfigApacheInd();

        	CManagerApi mngrApi(eProcessCSMngr);
        	mngrApi.SendOpcodeMsg(MCUMNGR_TO_CSMNGR_END_IP_CONFIG_IND);
		}
	}

	// ===== 2. timeout is reached!
	else
	{
		m_IpV6AutoConfigState = eIPv6AutoConfig_Failed;

		if( (eIpTypeConfigSuccess_IPv6 == m_isIpConfigInOsSucceeded)  )
		{
			m_isIpConfigInOsSucceeded = eIpTypeConfigSuccess_None;
		}
		else if(eIpTypeConfigSuccess_Both == m_isIpConfigInOsSucceeded)
		{
			m_isIpConfigInOsSucceeded = eIpTypeConfigSuccess_IPv4;
		}

		TRACEINTO << "\nCMcuMngrManager::OnTimerIpV6AutoConfig - failed to retrieve auto ipV6; giving up";
		PASSERTMSG(1, " Failed to configure auto ipV6");

		// send 'end of configuration' to the CSMngr,
		//       so it will 'check duplication' (and remove an unnecessary Alert)
    	CManagerApi mngrApi(eProcessCSMngr);
    	mngrApi.SendOpcodeMsg(MCUMNGR_TO_CSMNGR_END_IP_CONFIG_IND);
	}

	// Handle startup condition, if Switch indication was already received
	if(m_isMngmntIpConfigIndReceived)
	{
		bool successIPv4_Or_SuccessIpv4AndAuto =
			( (eIpTypeConfigSuccess_IPv4 == m_isIpConfigInOsSucceeded) || (eIpTypeConfigSuccess_Both == m_isIpConfigInOsSucceeded) ) ?
			true : false;


		if(eIPv6AutoConfig_Failed == m_IpV6AutoConfigState)
		{
			// v4 succeeded - update description for only v6
			if (true == successIPv4_Or_SuccessIpv4AndAuto)
			{

				m_bRestoreNoMngmntIpInterface_AA = TRUE;
			}
			else // active alarm will be with both
			{

				m_bRestoreNoMngmntIpInterface_AA = TRUE;
			}
		}

		else if(eIPv6AutoConfig_Success == m_IpV6AutoConfigState)
		{
			// v4 succeeded - no alarm
			if (true == successIPv4_Or_SuccessIpv4AndAuto)
			{

				m_bRestoreNoMngmntIpInterface_AA = FALSE;
			}
			else // v4 failed - update description for only v4
			{
				UpdateActiveAlarmDescriptionByErrorCode(AA_NO_MANAGEMENT_IP_INTERFACE, ERR_NO_MANAGEMENT_IPV4);
				m_bRestoreNoMngmntIpInterface_AA = TRUE;
			}
		}
	}
}*/

void CMcuMngrManager::ConfigGeneral(CIPService* pService/*=NULL*/)
{
	if (pService)
	{
		ConfigApache(*pService);
		ConfigSSHD(pService);
		StartDnsConfiguration(pService, eMngmntIpInit, false);
	}
	else
	{
		ConfigApache(*m_pMngmntIpParams_asIpService_fromProcess);
		ConfigSSHD(m_pMngmntIpParams_asIpService_fromProcess);

		CheckDNSStateMcmsNetwork();

	}
}
void   CMcuMngrManager::CheckDNSStateMcmsNetwork()
{
	eServerStatus serverStatus = m_pProcess->m_NetSettings.m_ServerDnsStatus;
	if(eServerStatusOff == serverStatus )
	{
		RemoveActiveAlarmByErrorCode(AA_DNS_REGISTRAION_FAILED);
		RemoveActiveAlarmByErrorCode(AA_NO_DNS_CONFIGURATION);
		m_bRestoreDnsRegistrationFailed_AA = FALSE;
		m_isDnsConfigInOsSucceeded = YES;
	}
	else
	{
		eDnsConfigurationStatus status =  m_pProcess->m_NetSettings.m_DnsConfigStatus;
		if( eDnsConfigurationSuccess == status)
		{
			RemoveActiveAlarmByErrorCode(AA_DNS_REGISTRAION_FAILED);
			RemoveActiveAlarmByErrorCode(AA_NO_DNS_CONFIGURATION);
			m_bRestoreDnsRegistrationFailed_AA = FALSE;
				m_isDnsConfigInOsSucceeded = YES;
		}
		else
		{
			UpdateStartupConditionByErrorCode(AA_DNS_REGISTRAION_FAILED, eStartupConditionFail);
			m_bRestoreDnsRegistrationFailed_AA = TRUE;
		}
	}
}
/////////////////////////////////////////////////////////////////////
bool CMcuMngrManager::RetrieveIpV6AutoAndConfig(CIPService& tmpService)
{
	TRACEINTO << "\nCMcuMngrManager::RetrieveIpV6AutoAndConfig";

	IpV6AddressMaskS autoIpV6[MNGMNT_NETWORK_NUM_OF_IPV6_ADDRESSES];
	for(int i = 0 ; i < MNGMNT_NETWORK_NUM_OF_IPV6_ADDRESSES ; ++i)
	{
		strcpy(autoIpV6[i].address, "");
		autoIpV6[i].mask = DEFAULT_IPV6_SUBNET_MASK;
	}

	// reset
	SetCntrlIPv6AddressInAutoMode(autoIpV6);
	SetDefGwAddressInAutoMode();

	//UpdateMemory_IPv6Auto(tmpService);

	RetrieveIpV6Addresses(autoIpV6, tmpService);
	if(autoIpV6[0].address[0] == '\0')
	{
		TRACEINTO << "\nCMcuMngrManager::RetrieveIpV6AutoAndConfig - Cannot retrieve IpV6 addresses";
		return false;
	}
	else
	{
		TRACEINTO << "\nCMcuMngrManager::RetrieveIpV6AutoAndConfig - IpV6 addresses are retrieved";
	}

	SetCntrlIPv6AddressInAutoMode(autoIpV6);
	SetDefGwAddressInAutoMode();

	UpdateMemory_IPv6Auto(tmpService);

	ConfigGeneral();




	return true;
}



void CMcuMngrManager::EnableAccessOnNetworkConfigFailure(CIPService* pService)
{
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::EnableAccessOnNetworkConfigFailure";

	POBJDELETE(m_pServiceForAccessOnNetConfigFailure);
	m_pServiceForAccessOnNetConfigFailure = new CIPService(*pService); // will be deleted when timeout is reached (and in destructor)

//	StartTimer(MCUMNGR_NETWORK_CONFIG_FAILURE_TIMER, NETWORK_CONFIG_FAILURE_TIMEOUT);
}

////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnNetworkConfigFailureTimeout(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnNetworkConfigFailureTimeout";

	ConfigGeneral(m_pServiceForAccessOnNetConfigFailure); // to at least enable an access
	POBJDELETE(m_pServiceForAccessOnNetConfigFailure);

	CMcmsDaemonApi api;
    api.SendConfigApacheInd();
}
// added for debug IP Settings
void printIpAddressDebug(DWORD ipaddress)
{
	unsigned char *ip4, *netmaskv4, *gwv4;
	char sIpAddress[128] = "";
	ip4 = (unsigned char*)&ipaddress;
	sprintf(sIpAddress, "%d.%d.%d.%d", (int)ip4[3],(int)ip4[2],(int)ip4[1],(int)ip4[0]);
	printf("[debug]: ipaddress= %s\n", sIpAddress);
}

void PrintIPServiceInfoDebug(CIPService *ipService)
{

	//DWORD ipv4Address = ipService->GetNetIPaddress();
	CIPSpan * ipSpan = ipService->GetFirstSpan();
	DWORD ipv4Address = ipSpan->GetIPv4Address();
	DWORD ipv4NetMask = ipService->GetNetMask();
	DWORD ipv4DefaultGateway = ipService->GetDefaultGatewayIPv4();
	printf("==========================\n");
	printf("[debug]: ip=%u, mask=%u, gw=%u, ipServiceType=%d\n",
			ipv4Address, ipv4NetMask, ipv4DefaultGateway, ipService->GetIpServiceType());
	printIpAddressDebug(ipv4Address);
	printIpAddressDebug(ipv4NetMask);
	printIpAddressDebug(ipv4DefaultGateway);
	printf("==========================\n");

}
//end
////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::UpdateShelfMngrInfoIfNeeded( const string theCaller,
												   CIPService *pNewService,
						  						   const IP_ADDR_S & ipAddrS,
						  						   eMngmntIpUpdatePhase updatePhase,
						  						   bool updateControl)
{
	// ===== 1. get info
	IP_ADDR_S ipOldAddrS;
	RetrieveIpAddresses(m_pMngmntIpParams_asIpService_fromProcess, ipOldAddrS);

	bool isAddressesDiffer = ( (ipOldAddrS.iPv4_shelf != ipAddrS.iPv4_shelf) ||								//	IPv4 Shelf address differ
							   (strncmp(ipOldAddrS.iPv6_shelf, ipAddrS.iPv6_shelf, IPV6_ADDRESS_LEN)) );	//	IPv6 Shelf address differ

	eIpType curIpType						= pNewService->GetIpType();
	eV6ConfigurationType curV6ConfigType	= pNewService->GetIpV6ConfigurationType();
	bool isAutoMode = ( ((eIpType_IpV6 == curIpType) || (eIpType_Both == curIpType)) &&
						(eV6Configuration_Auto == curV6ConfigType) );

	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::UpdateShelfMngrInfoIfNeeded (caller: " << theCaller << ")"
						   << "\nShelfMngr's Ip address differ: " << (isAddressesDiffer	? "yes" : "no")
						   << "\nIPv6 (or both) Auto mode:      " << (isAutoMode		? "yes" : "no");

	// ===== 2. check equality (and act accordingly)
	if ( (true == isAddressesDiffer) || (true == isAutoMode) )
	{
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::UpdateShelfMngrInfoIfNeeded (caller: " << theCaller << ") - params are updated";

		CIPService ipService;
		if ( (true == updateControl) && (false == isAutoMode) ) // cntrl params should also be updated
		{
			ipService = *pNewService;
		}

		else // only Shelf params should be updated
		{
			ipService = *m_pMngmntIpParams_asIpService_fromProcess;
			CIPSpan *pSpan = ipService.GetSpanByIdx(1);
			if(!pSpan)
			{
				TRACEINTO << "\nCMcuMngrManager::UpdateShelfMngrInfoIfNeeded (caller: " << theCaller << ") - Error: empty span";
				return;
			}

			// ===== 1. IPv4
			if ( (eIpType_IpV4 == curIpType) || (eIpType_Both == curIpType) )
			{
				pSpan->SetIPv4Address(ipAddrS.iPv4_shelf);
			}

			// ===== 1. IPv6
			if ( (eIpType_IpV6 == curIpType) || (eIpType_Both == curIpType) )
			{
				CIPSpan* pShelfSpan = pNewService->GetSpanByIdx(1);
				if (pShelfSpan)
				{
					string curIPv6Address = "";
					for(int i=0 ; i<NUM_OF_IPV6_ADDRESSES ; ++i)
					{
						curIPv6Address = pShelfSpan->GetIPv6Address(i);
						pSpan->SetIPv6Address(i, curIPv6Address.c_str());
					}
				}
			}
		} // end Shelf params should be updated
		UpdateNetworkInterface("UpdateShelfMngrInfoIfNeeded", ipService, updatePhase);

	} // end Shelf addresses differ
	else // Shelf addresses are equal
	{
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::UpdateShelfMngrInfoIfNeeded (caller: " << theCaller << ") - nothing is done";
	}
}

void CMcuMngrManager::UpdateSecurityDependencies(const CIPService& theService)
{
  if (theService.GetSpanByIdx(0) == NULL)
    return;

  CManagementSecurity* sec = theService.GetManagementSecurity();
  PASSERT_AND_RETURN(NULL == sec);
  bool ca_validation = sec->IsRequestPeerCertificate();
  std::string host_name = GetHostNameFromService(&theService);
  BYTE revocation_method = sec->GetRevocationMethodType();
  if (theService.GetIsSecured() == TRUE)
  {
    STATUS stat = CheckForValidCertificate(host_name.c_str(), true, ca_validation,revocation_method);
    if (stat != STATUS_OK)
    {
      // If there is a problem with the certificate,
      // other then the common name and expiration date
      if (CheckIfToGoBackToPort80(stat))
      {
        PASSERTSTREAM(true,
                      "Invalid TLS: " << m_pProcess->GetStatusAsString(stat)
                      << ". Go back to port 80.");

        // Disables the security
        const_cast<CIPService&>(theService).SetIsSecured(FALSE);

        //BRIDGE-13535 - do not block confference on failure of managment certificate
        ConfigApache(theService);
      }
    }
  }
  else
  {

    // Removes TLC active alarm in non-secure mode
    RemoveTLSActiveAlarms();

    // Updates certificate status, but doesn't fire active alarm
    CheckForValidCertificate(host_name.c_str(), false, ca_validation,revocation_method);
  }
}

STATUS CMcuMngrManager::UpdateDnsForService(CIPService &theService)
{
	theService.GetpDns()->SetStatus(m_pProcess->m_NetSettings.m_ServerDnsStatus); // m_pProcess->m_NetSettings.get
	theService.GetpDns()->SetIPv4Address(0, m_pProcess->m_NetSettings.m_ipv4DnsServer); // m_pProcess->m_NetSettings.get
	theService.GetpDns()->SetIPv4Address(1, m_pProcess->m_NetSettings.m_ipv4DnsServer_1);
	theService.GetpDns()->SetIPv4Address(2, m_pProcess->m_NetSettings.m_ipv4DnsServer_2);
	theService.GetpDns()->SetDomainName(m_pProcess->m_NetSettings.GetDomainName().c_str());

	char tmpAddr[IPV6_ADDRESS_LEN];
	char tmpMask[IPV6_ADDRESS_LEN];

	memset(tmpAddr,			0, IPV6_ADDRESS_LEN);
	memset(tmpMask,			0, IPV6_ADDRESS_LEN);
//	SplitIPv6AddressAndMask(&m_pProcess->m_NetSettings.m_ipv6_DnsServer.addr.ip, tmpAddr, tmpMask);
	FTRACESTR(eLevelInfoNormal) << "UpdateDnsForService m_pProcess->m_NetSettings.m_ipv6_DnsServer.addr.ip = " << m_pProcess->m_NetSettings.m_ipv6_DnsServer.addr.ip;
	FTRACESTR(eLevelInfoNormal) << "UpdateDnsForService tmpAddr = " << tmpAddr;

	 std::string ipv6,ipv6Mask;
	 m_pProcess->m_NetSettings.ConvertIpv6AddressToString(m_pProcess->m_NetSettings.m_ipv6_DnsServer, ipv6, ipv6Mask);
	theService.GetpDns()->SetIPv6Address(0, ipv6.c_str());
	theService.GetpDns()->SetIPv6SubnetMask(0, ipv6Mask.c_str());

	m_pProcess->m_NetSettings.ConvertIpv6AddressToString(m_pProcess->m_NetSettings.m_ipv6_DnsServer_1, ipv6, ipv6Mask);
	theService.GetpDns()->SetIPv6Address(1, ipv6.c_str());
	theService.GetpDns()->SetIPv6SubnetMask(1, ipv6Mask.c_str());

	m_pProcess->m_NetSettings.ConvertIpv6AddressToString(m_pProcess->m_NetSettings.m_ipv6_DnsServer_2, ipv6, ipv6Mask);
	theService.GetpDns()->SetIPv6Address(2, ipv6.c_str());
	theService.GetpDns()->SetIPv6SubnetMask(2, ipv6Mask.c_str());

	return STATUS_OK;
}
STATUS CMcuMngrManager::UpdateNetworkInterface(const string theCaller,
                                               const CIPService &theService,
		                                           eMngmntIpUpdatePhase updatePhase,
		                                           eIpTypeConfigSuccess ipTypeToConfig)
{
    TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::UpdateNetworkInterface (caller: " << theCaller << ")"
                           << "\nUpdate phase: " << GetMngmntIpUpdatePhaseAsString(updatePhase)
                           << "\nIP type(s) that succeeded to be configured: " << GetIpTypeConfigSuccessAsString(ipTypeToConfig);

    STATUS retStatus = STATUS_OK;
    eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

    // ===== 1. get ip parameters that were configured successfully
    CIPService *pUpdatedService = NULL;

    pUpdatedService = new CIPService(theService);
    if(pUpdatedService)
    {
        eV6ConfigurationType ipV6ConfigTypeFromService = pUpdatedService->GetIpV6ConfigurationType();

        if ( updatePhase == eMngmntIpFromMpl || (eMngmntIpInit == updatePhase && m_bRestoreConfigSuccess == TRUE))
        {
            if(eIpTypeConfigSuccess_Both != ipTypeToConfig)
            {
                eIpType ipType;
                switch(ipTypeToConfig)
                {
                case eIpTypeConfigSuccess_IPv4: // get original IpV6 params
                    ipType = eIpType_IpV6;
                    break;
                case eIpTypeConfigSuccess_IPv6: // get original IpV4 params
                    ipType = eIpType_IpV4;
                    break;
                case eIpTypeConfigSuccess_None: // get original Ip params (both types)
                    ipType = eIpType_Both;
                    break;
                default:
                    PASSERTMSG(1, "Invalid ipTypeToConfig");
                    ipType = eIpType_Both;
                }

                // update management ip
                pUpdatedService->SetIpParamsFromOtherService(m_pMngmntIpParams_asIpService_fromProcess, 0, ipType);
            }

            if (eV6Configuration_Auto == ipV6ConfigTypeFromService)
            {
                pUpdatedService->SetIpParamsFromOtherService(m_pMngmntIpParams_asIpService_fromProcess, 0, eIpType_IpV6);
            }
        }

        pUpdatedService->SetName(MANAGEMENT_NETWORK_NAME);

        pUpdatedService->SetIpServiceType(eIpServiceType_Management);

        CIPSpan* pCntrlSpan = pUpdatedService->GetSpanByIdx(0); // Control params are stored in the 1st span (idx==0)
        if (pCntrlSpan)
        {
            // set IPv6 address in Manual mode
            if (eV6Configuration_Manual == ipV6ConfigTypeFromService)
            {
                // in Manual mode, only one address (the 1st, idx==0) is referred to;
                //   therefore other addresses (starting from idx==1) should be removed
                for (int i = 1; i < MNGMNT_NETWORK_NUM_OF_IPV6_ADDRESSES; ++i)
                    pCntrlSpan->SetIPv6Address(i, "");
            }

            if (curProductType == eProductTypeEdgeAxis && eMngmntIpFromEma == updatePhase)
            {
                SetSystemHostName((pCntrlSpan->GetSpanHostName()).GetString());
            }

        	//host name for soft mcu can't be configured
            if (curProductType == eProductTypeSoftMCU || curProductType == eProductTypeSoftMCUMfw || curProductType == eProductTypeEdgeAxis || m_mcuMngrProductType == eProductTypeCallGeneratorSoftMCU)
            {
				std::string host_name = GetSystemHostName();

				pCntrlSpan->SetSpanHostName(host_name.c_str());
            }
        }

        //set Shelf IP same as Management IP when Gesher/Ninja
        if (curProductType == eProductTypeGesher || curProductType == eProductTypeNinja)
        {
            if(pUpdatedService->GetSpanByIdx(1))
            {
                (*pUpdatedService->GetSpanByIdx(1)) = *(pUpdatedService->GetSpanByIdx(0));
                pUpdatedService->GetSpanByIdx(1)->SetLineNumber(1);
            }
        }
            
        // Checks if there are valid pre-defined certificate
        UpdateSecurityDependencies(*pUpdatedService);

        // ...and starts polling for future certificates validation
        StartTimer(MCUMNGR_CERTIFICATES_DAILY_TIMER, DAILY_TIMER_TIMEOUT);

        // Updates in memory
        m_pProcess->SetMngmntIpParams_asIpService(*pUpdatedService);
        m_pMngmntIpParams_asIpService_fromProcess =
                m_pProcess->GetMngmntIpParams_asIpService();

        // ===== 5. add to list (stored at Process level)
        if ( NO == m_pProcess->IsIpInterfaceExistsInList(*pUpdatedService) )
        {
            m_pProcess->AddIpInterfaceToList(*pUpdatedService);
        }
        else // updating service (by Mpl/Ema)
        {
            m_pProcess->UpdateIpInterfaceInList(*pUpdatedService);

            // ===== 7. update the file
            pUpdatedService->WriteXmlFile(MANAGEMENT_NETWORK_CONFIG_PATH, "MngmntNetwork");

            TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::UpdateNetworkInterface (caller: " << theCaller
                    << ", phase: " << GetMngmntIpUpdatePhaseAsString(updatePhase) << ")"
                    << "\nFile " << MANAGEMENT_NETWORK_CONFIG_PATH << " was written";

        }

        // ===== 7. update MPL
        if (eMngmntIpFromEma == updatePhase)
        {
            SendMngmntUpdateToMpl();
        }

        POBJDELETE(pUpdatedService);
    }
    else // pUpdatedService == NULL
    {
    	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::UpdateNetworkInterface (caller: " << theCaller
    			<< ", phase: " << GetMngmntIpUpdatePhaseAsString(updatePhase) << ")"
    			<< "\nError: updatedService is NULL!";
    }

    return retStatus;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
std::string CMcuMngrManager::GetSystemHostName()
{
	//read the system host name
	string host_name;

	SystemPipedCommand("echo -n `/bin/hostname`", host_name);

	return host_name;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SetSystemHostName(const string & host_name)
{
	//set system host name
    std::string answer;
    STATUS stat = STATUS_FAIL;

    std::string cmd = "source "+MCU_UTILS_DIR+"/Scripts/DnsSetup.sh; set_hostname " + host_name ;
    stat = SystemPipedCommand(cmd.c_str() ,answer);
    TRACESTR(stat ? eLevelError : eLevelInfoNormal) <<
        "CConfiguratorManager::SetSystemHostName: " << host_name << " answer: "<< answer;
}

/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendConfBlockToConfParty(BYTE confBlockReason)
{
	if (m_isConfBlock == FALSE)
		RemoveActiveAlarmByErrorCode(AA_FAILED_TO_VALIDATE_CERTIFICATE);

    TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendIsConfBlockToConfParty -"
                                  << "\nTLS flag:" << (YES == m_isConfBlock ? "YES" : "NO");

    // ===== 1. insert TLS flag to the segment
    CSegment*  pParam = new CSegment();
    *pParam << (BYTE)m_isConfBlock;
    *pParam << (BYTE)confBlockReason;

    // ===== 2. send to ConfPartyMngr
    const COsQueue* pConfPartyMngrMbx = CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessConfParty, eManager);
    STATUS status = pConfPartyMngrMbx->Send(pParam, CONF_BLOCK_IND);

    if (status != STATUS_OK)
    	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendIsConfBlockToConfParty - Failed to send block request to ConfParty.";
}

/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendMngmntUpdateToMpl()
{
	CIpParameters ipParamsFromEma;
	m_pMngmntIpParams_asIpService_fromProcess->ConvertToIpParamsStruct( *(ipParamsFromEma.GetIpParamsStruct()) );
	SendMngmntIpParamsUpdateReqToMplApi(ipParamsFromEma.GetIpParamsStruct());

	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendMngmntUpdateToMpl\nMcuMngr -> EMB\n" << ipParamsFromEma;
}

////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendMngmntIpParamsUpdateReqToMplApi(IP_PARAMS_S* pIpParamsStruct)
{
	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::SendMngmntIpParamsUpdateReqToMplApi";

	CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol();

	mplPrtcl->AddCommonHeader(MPL_MNGMNT_IP_PARAMS_UPDATE_REQ);
	mplPrtcl->AddMessageDescriptionHeader();
	mplPrtcl->AddPhysicalHeader(1, m_switchBoardId, m_switchSubBoardId);
	mplPrtcl->AddData( sizeof(IP_PARAMS_S), (char*)pIpParamsStruct );

	CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("MCUMNGR_SENDS_TO_MPL_API");

	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::SendMngmntIpParamsUpdateReqToMplApi";
	mplPrtcl->SendMsgToMplApiCommandDispatcher();

	POBJDELETE(mplPrtcl);
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::UpdateMemory_IPv6Auto(CIPService& tmpService)
{
	TRACEINTO << "\nCMcuMngrManager::UpdateMemory_IPv6Auto";

	// ===== update params
	CIPService ipService;
    if (m_mcuMngrProductType==eProductTypeSoftMCU || m_mcuMngrProductType==eProductTypeSoftMCUMfw || m_mcuMngrProductType==eProductTypeEdgeAxis)
	    ipService = tmpService;
    else
    	ipService = *m_pMngmntIpParams_asIpService_fromProcess;

	ipService.SetDefaultGatewayIPv6(m_defGwInAutoMode);
	ipService.SetDefaultGatewayMaskIPv6(m_defGwMaskInAutoMode);

	CIPSpan* pCntrlSpan = ipService.GetSpanByIdx(0);
	if(pCntrlSpan)
	{
		for(int i = 0 ; i < MNGMNT_NETWORK_NUM_OF_IPV6_ADDRESSES ; ++i)
		{
			pCntrlSpan->SetIPv6Address(i, m_cntrlIPv6AddressInAutoMode[i].address); // set the address as the 1st IPv6 address of the Control
			pCntrlSpan->SetIPv6SubnetMask(i, m_cntrlIPv6AddressInAutoMode[i].mask); // set the mask
		}
	}

	m_pProcess->SetMngmntIpParams_asIpService(ipService);


	// ===== add to list (stored at Process level)
	if ( NO == m_pProcess->IsIpInterfaceExistsInList(ipService) )
	{
		m_pProcess->AddIpInterfaceToList(ipService);
	}
	else // updating service (by Mpl/Ema)
	{
		m_pProcess->UpdateIpInterfaceInList(ipService);
	}
	
	tmpService=*m_pProcess->GetMngmntIpParams_asIpService();
	m_pMngmntIpParams_asIpService_fromProcess = m_pProcess->GetMngmntIpParams_asIpService();
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::RetrieveIpV6Addresses(IpV6AddressMaskS pOutAddress[], CIPService& tmpService)
{
	TRACEINTO << "\nCMcuMngrManager::RetrieveIpV6Addresses";

	string retStr_ipV6;
	for( int i = 0 ; i < MNGMNT_NETWORK_NUM_OF_IPV6_ADDRESSES ; ++i)
	{
		memset(pOutAddress[i].address, '\0', IPV6_ADDRESS_LEN);
	}

	eConfigInterfaceType ifType = eManagmentNetwork;
	if(IsJitcAndNetSeparation())
		ifType = eSeparatedManagmentNetwork;

	std::string nic_name = GetDeviceName(ifType);

	if ((eProductTypeGesher==m_mcuMngrProductType) || (eProductTypeNinja==m_mcuMngrProductType))
	{		
	    nic_name = GetLogicalInterfaceName(ifType, eIpType_IpV6);
	}
	else if (m_mcuMngrProductType==eProductTypeSoftMCU || m_mcuMngrProductType==eProductTypeSoftMCUMfw || m_mcuMngrProductType==eProductTypeEdgeAxis)
	{
	    CIPSpan * pSpan = tmpService.GetFirstSpan();
	    if(pSpan)
	        nic_name = pSpan->GetInterface();
	}

	RetrieveIpAddressConfigured_IpV6(pOutAddress, retStr_ipV6, nic_name);

	TRACEINTO << "\nCMcuMngrManager::RetrieveIpV6Addresses\nAuto config return string IpV6 for " << nic_name << " : " << retStr_ipV6;
}

void CMcuMngrManager::SetCntrlIPv6AddressInAutoMode(IpV6AddressMaskS & addrS)
{
	strncpy(m_cntrlIPv6AddressInAutoMode[0].address, addrS.address, IPV6_ADDRESS_LEN-1);
	m_cntrlIPv6AddressInAutoMode[0].address[IPV6_ADDRESS_LEN-1] = '\0';
	m_cntrlIPv6AddressInAutoMode[0].mask = addrS.mask;
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SetCntrlIPv6AddressInAutoMode(IpV6AddressMaskS addrS[])
{
	for(int i = 0 ; i < MNGMNT_NETWORK_NUM_OF_IPV6_ADDRESSES ; ++i)
	{
		strncpy(m_cntrlIPv6AddressInAutoMode[i].address, addrS[i].address, IPV6_ADDRESS_LEN-1);
		m_cntrlIPv6AddressInAutoMode[i].address[IPV6_ADDRESS_LEN-1] = '\0';
		m_cntrlIPv6AddressInAutoMode[i].mask = addrS[i].mask;
	}
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SetDefGwAddressInAutoMode()
{
	SetDefGwInAutoMode();
	SetDefGwMaskInAutoMode();
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SetDefGwInAutoMode()
{
	string sDefGw;
	SystemPipedCommand("/sbin/route -A inet6 | grep G | grep U | awk '{ print $2 }'", sDefGw);

	int lenToCopy = sDefGw.length();

	int singleAddressLen = 0;
	const char *pDefGwStart	= sDefGw.c_str();
	const char *pDefGwEndl	= strchr(sDefGw.c_str(), '\n'); // search for '\n'
	if (pDefGwEndl)
	{
		singleAddressLen = pDefGwEndl - pDefGwStart; // num of characters before the '\n'

		if ( (0 <= singleAddressLen) && (singleAddressLen <= lenToCopy) )
	    {
		    lenToCopy = singleAddressLen;
	    }
	}

	if (lenToCopy < IPV6_ADDRESS_LEN)
	{

		memset(m_defGwInAutoMode, 0, IPV6_ADDRESS_LEN);
		strncpy(m_defGwInAutoMode, sDefGw.c_str(), lenToCopy); // copy only a single address
		m_defGwInAutoMode[lenToCopy] = '\0';
	}

	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SetDefGwInAutoMode"
						   << "\nDefGW: " << m_defGwInAutoMode;
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SetDefGwMaskInAutoMode()
{
	m_defGwMaskInAutoMode = DEFAULT_IPV6_SUBNET_MASK;
}

/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::PrintIpAddressesConfigured( string &retStr, eEntitytToConfigIp entityToConfig/*=eEntitytToConfigIp_Apache*/,
		                                          eIpType ipType/*=eIpType_IpV4*/,
		                                          char* ipAddrV4/*=""*/, char* ipAddrV6/*=""*/ )
{
	retStr = GetEntityToConfigIpAsString(entityToConfig);
	retStr += " is configured with the retrieved addresses (IP type: ";
	retStr += ::IpTypeToString(ipType);
	retStr += ") -\nIpV4: ";
	retStr += ipAddrV4;
	retStr += "\nIpV6: ";
	retStr += ipAddrV6;

	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::PrintIpAddressesConfigured\n" << retStr.c_str();
}

////////////////////////////////////////////////////////////////////////////
const char* CMcuMngrManager::GetMngmntIpUpdatePhaseAsString(eMngmntIpUpdatePhase updatePhase)
{
	const char *retStr = ( ( (0 <= updatePhase) && (MAX_NUM_OF_MNGMNT_IP_UPDATE_PHASE > updatePhase) )
						   ?
						   mngmntIpUpdatePhase[updatePhase] : "Invalid" );
	return retStr;
}

/////////////////////////////////////////////////////////////////////
const char* CMcuMngrManager::GetEntityToConfigIpAsString(eEntitytToConfigIp entityToConfig)
{
	const char *retStr = ( ( (0 <= entityToConfig) && (NUM_OF_ENTITIES_TO_CONFIG_IP > entityToConfig) )
	                       ?
	                       entityToConfigIpStr[entityToConfig] : "Invalid entity" );
	return retStr;
}

////////////////////////////////////////////////////////////////////////////
const char* CMcuMngrManager::GetIpTypeConfigSuccessAsString(eIpTypeConfigSuccess configSuccessType)
{
	const char *retStr = ( ( (0 <= configSuccessType) && (MAX_NUM_OF_IP_TYPE_CONFIG_SUCCESS > configSuccessType) )
						   ?
						   ipTypeConfigSuccess[configSuccessType] : "Invalid" );
	return retStr;
}

eIpTypeConfigSuccess CMcuMngrManager::GetTotalIpTypeConfigSuccess(STATUS ipV4Status,
                                                                  STATUS ipV6Status,
                                                                  eIpType ipType)
{
	STATUS overallIpV6Status = ipV6Status;
	if (STATUS_FAILED_CFG_IPV6_DEF_GW == ipV6Status)
	{
		// if the only failure in IPv6 is the defGw, then refer to it as ok
		overallIpV6Status = STATUS_OK;
	}

	eIpTypeConfigSuccess retType = eIpTypeConfigSuccess_None;
	if ( (STATUS_OK == ipV4Status) && (STATUS_OK == overallIpV6Status) )
	{
		retType = eIpTypeConfigSuccess_Both;
	}
	else if (eIpType_Both == ipType)
	{
		if (STATUS_OK == ipV4Status)
		{
			retType = eIpTypeConfigSuccess_IPv4;
		}
		else if (STATUS_OK == overallIpV6Status)
		{
			retType = eIpTypeConfigSuccess_IPv6;
		}
	}

	return retType;
}

void CMcuMngrManager::ConfigApache(const CIPService& service)
{
	CIPSpan* pSpan = service.GetSpanByIdx(0);

	// IPv4
	char iPv4Str[IP_ADDRESS_LEN] = {0};
	if (pSpan)
		SystemDWORDToIpString(pSpan->GetIPv4Address(), iPv4Str);

	// IPv6
	std::string ipV6Str;

	if (pSpan)
			ipV6Str = pSpan->GetIPv6Address();


  if (!service.GetIsSecured() && IsFederalOn())
  {
    AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT,
                            AA_HTTPS_DISABLED_IN_JITC,
                            MAJOR_ERROR_LEVEL,
                            "WEB server must be secured in ULTRA SECURE mode.",
                            true, true);
  }

  CManagementSecurity* sec = service.GetManagementSecurity();
  PASSERT_AND_RETURN(NULL == sec);

  std:string cHostName = GetHostNameFromService(&service);


  ConfigApache(service.GetIpType(),
               iPv4Str,
               ipV6Str,
               service.GetIsPermanentNetworkOpen(),
               service.GetIsSecured(),
               sec->IsRequestPeerCertificate(),
               sec->GetRevocationMethodType(),
               sec->IsUseResponderOcspUri(),
               sec->IsIncompleteRevocation(),
               sec->IsSkipValidationOcspCert(),
               sec->GetOCSPGlobalResponderURI(),
               cHostName);


}

void CMcuMngrManager::ConfigApache(eIpType ipType /* =eIpType_IpV4 */,
                                   const std::string& IpAddressToConfig_ipV4 /* ="" */,
                                   const std::string& IpAddressToConfig_ipV6 /* ="" */,
                                   BOOL isPermanentNetworkOpen /* =TRUE */,
                                   BOOL isSecured /* =FALSE */,
                                   BOOL isRequestPeerCertificate /* =FALSE */,
                                   BYTE revocationMethodType /* = eNoneMethod */,
                                   BOOL isUseResponderOcspURI /* = TRUE */,
                                   BOOL isIncompleteRevocation /* = TRUE */,
                                   BOOL isSkipValidateOcspCert /* = TRUE */,
                                   std::string	ocspGlobalResponderURI /*= ""*/,
                                   std::string hostname /*=""*/)

{
  CSysConfig* cfg = m_pProcess->GetSysConfig();
  PASSERT_AND_RETURN(NULL == cfg);

  m_apacheConfig.m_ipType = ipType;
  m_apacheConfig.m_IpAddressToConfig_ipV4 = IpAddressToConfig_ipV4;
  m_apacheConfig.m_IpAddressToConfig_ipV6 = IpAddressToConfig_ipV6;
  m_apacheConfig.m_isSecured = isSecured;

  m_apacheConfig.m_isRequestPeerCertificate = isRequestPeerCertificate;


  m_apacheConfig.m_revocationMethodType = revocationMethodType;
  m_apacheConfig.m_isUseResponderOcspURI = isUseResponderOcspURI;
  m_apacheConfig.m_isIncompleteRevocation = isIncompleteRevocation;
  m_apacheConfig.m_isSkipValidateOcspCert = isSkipValidateOcspCert;
  m_apacheConfig.m_ocspGlobalResponderURI = ocspGlobalResponderURI;

  // Doesn't listen on permanent address in Ultra Secure Mode and in simulation
  if (!IsFederalOn())
  {
    m_apacheConfig.m_isPermanentNetworkOpen =
        IsTarget() ? isPermanentNetworkOpen : FALSE;

  }
  else
  {
    m_apacheConfig.m_isPermanentNetworkOpen = FALSE;
    if (isPermanentNetworkOpen)
    	TRACEINTOFUNC << "PermanentNetworkOpen is switched off in ULTRA SECURE mode";
  }

	std::string sslProtocol;
	BOOL res = cfg->GetDataByKey(CFG_RMX_MANAGEMENT_SECURITY_PROTOCOL, sslProtocol);
	TRACEINTOFUNC << " ConfigApache CFG_RMX_MANAGEMENT_SECURITY_PROTOCOL "  << sslProtocol;
	PASSERTSTREAM_AND_RETURN(!res,
	    "CSysConfig::GetDataByKey: " << CFG_RMX_MANAGEMENT_SECURITY_PROTOCOL);
  int sslValue =0;

  CStringsMaps::GetValue(TLSV_ENUM,sslValue,(char*)sslProtocol.c_str());
  TRACEINTOFUNC << " ConfigApache CFG_RMX_MANAGEMENT_SECURITY_PROTOCOL value "  << sslValue;

  eMangmentSecurityProtocol eProtocol = (eMangmentSecurityProtocol)sslValue;
  //comment tls1.1 and tls1.2 need to be disabled untill EMA support dotnet 4.5 to support tls1.1 and tls1.2
 // if (IsFederalOn()&& (( eTLSV1_SSLV3 == eProtocol)|| (eTLS1_2_TLSV1_1_TLSV1_SSLV3 == eProtocol )  ))
  if (IsFederalOn()&& ( eTLSV1_SSLV3 == eProtocol))
  {
	  eProtocol = eTLSV1;
  }

   m_apacheConfig.m_SSLProtocol = eProtocol;


	// Updates proxy details
	std::string external_content_dir, external_content_ip;
	DWORD external_content_port;
	if (GetDataForProxy(external_content_dir,
                      external_content_ip,
                      external_content_port))
	{
	  TRACEINTOFUNC << "Port: " << external_content_port;

		m_apacheConfig.m_isExternalContentSupported = TRUE;
		m_apacheConfig.m_externalContentDir = external_content_dir;
		m_apacheConfig.m_externalContentIp = external_content_ip;
		m_apacheConfig.m_externalContentPort = external_content_port;
	}
	else
	{
		m_apacheConfig.m_isExternalContentSupported = FALSE;
	}

	res = cfg->GetDWORDDataByKey(CFG_KEY_APACHE_KEEP_ALIVE_TIMEOUT,
                               m_apacheConfig.m_KeepAliveTimeout);
	PASSERTSTREAM_AND_RETURN(!res,
	    "CSysConfig::GetDWORDDataByKey: " << CFG_KEY_APACHE_KEEP_ALIVE_TIMEOUT);

  res = cfg->GetDWORDDataByKey(CFG_KEY_MAX_KEEP_ALIVE_REQUESTS,
                               m_apacheConfig.m_MaxKeepAliveRequests);
  PASSERTSTREAM_AND_RETURN(!res,
      "CSysConfig::GetDWORDDataByKey: " << CFG_KEY_MAX_KEEP_ALIVE_REQUESTS);

	m_apacheConfig.m_isV35GWEnabled = m_bIsV35GwEnabledInService;
	m_apacheConfig.m_internalGWAddress = m_sInternalGwAddress;
	m_apacheConfig.m_strHostName="";
	if(hostname.empty())
	{

		m_apacheConfig.m_strHostName = GetHostNameFromService(m_pMngmntIpParams_asIpService_fromProcess);

	}
	else
		m_apacheConfig.m_strHostName = hostname;

	DWORD ocspTimeOut =1;
	res = cfg->GetDWORDDataByKey(CFG_OCSP_RESPONDER_TIMEOUT,ocspTimeOut);

	PASSERTSTREAM_AND_RETURN(!res,
		    "CSysConfig::GetDWORDDataByKey: " << CFG_OCSP_RESPONDER_TIMEOUT);
	if(res)
		m_apacheConfig.m_ocspResponderTimeout = ocspTimeOut;

	TRACEINTOFUNC << std::endl << m_apacheConfig;

	m_apacheConfig.GenerateConfigFile();

	CMcmsDaemonApi api;
	api.SendResetProcessReq(eProcessApacheModule);
}

void CMcuMngrManager::ConfigSSHD(CIPService* pService)
{
	string ipv4 = "";
	char ipv4_arr[IP_ADDRESS_LEN] = {0};
	string ipv6 = "";

	eIpType ipType = pService->GetIpType();
	CIPSpan *pSpan = pService->GetFirstSpan();
	if(pSpan)
	{
		if(eIpType_IpV6 != ipType)
		{
			::SystemDWORDToIpString(pSpan->GetIPv4Address(), ipv4_arr);
			ipv4 = ipv4_arr;
		}

		if(eIpType_IpV4 != ipType)
		{
			ipv6 = pSpan->GetIPv6Address(0);
		}
	}

	ConfigSSHD(pService->GetIsPermanentNetworkOpen(),
			   ipv4,
			   ipv6);
}

void CMcuMngrManager::ConfigSSHD(bool isPermanentNetwork,
								 std::string strIpV4Address/*=""*/,
                                 std::string strIpV6Address/*=""*/)
{
	TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__;

    CConfigManagerApi config_mgr_api;
	config_mgr_api.RestartSSHD(isPermanentNetwork, strIpV4Address, strIpV6Address);
}

STATUS CMcuMngrManager::StartDnsConfiguration(CIPService *pService,
                                              eMngmntIpUpdatePhase updatePhase,
                                              bool updateInterface/* = true*/)
{
	STATUS retStatus = STATUS_OK;
	static bool dnsMngntOnce=false;

	if((pService->GetIpServiceType() == eIpServiceType_Management) &&  (updatePhase != eMngmntIpFromEma))
			return STATUS_OK;

	CIpDns* pTmpDns = pService->GetpDns();
	eServerStatus dnsRegistrationMode = pTmpDns->GetStatus();



	if (!dnsMngntOnce && eServerStatusSpecify == dnsRegistrationMode)
	{
		m_numOfDnsEnable++;
		dnsMngntOnce =true;
	}


	TRACESTR(eLevelInfoNormal)	<< "CMcuMngrManager::StartDnsConfiguration - setting dns specify, m_numOfDnsEnable = " << m_numOfDnsEnable;

	///////////////////////////////////////////////////////////////
	//if m_numOfDnsEnable == 1  - Mngmt DNS and\or CS DNS is enable
	if(m_numOfDnsEnable == 1)
	{
		dnsRegistrationMode = eServerStatusSpecify;
	}

	// ===== a. RegistrationMode: Auto
	if (eServerStatusAuto == dnsRegistrationMode)
	{
		TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__ << " - dns mode is 'Auto'";

		BOOL isDhcp = NO;
		m_pProcess->GetSysConfig()->GetBOOLDataByKey("DHCP_ENABLED", isDhcp);
		if ( YES == isDhcp )	// dhcp enabled in sysCfg
		{
			retStatus = ConfigDnsAuto(pService);
		}
		else					// dhcp disabled in sysCfg
		{
			retStatus = STATUS_OK;
			TRACESTR(eLevelInfoNormal)	<< "\n" << __FUNCTION__ << " - "
			                        << "DHCP is disabled (in SysConfig); therefore nothing is actually done";
		}
	}

	// ===== b. RegistrationMode: Specify
	else if (eServerStatusSpecify == dnsRegistrationMode)
	{
		TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__ << " - dns mode is 'Specify'";
		retStatus = ConfigDnsSpecify(pService);
	}

	// ===== c. RegistrationMode: Off
	else if (eServerStatusOff == dnsRegistrationMode)
	{
		TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__ << " - dns mode is 'Off'";
		retStatus = ConfigDnsOff();
	}

	// ===== d. Illegal RegistrationMode
	else
	{
		TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__ << " - illegal dns mode: " << dnsRegistrationMode;
		retStatus = STATUS_FAIL;
	}

    if (STATUS_OK == retStatus)
    {
    	if(updateInterface)
    		UpdateNetworkInterface("StartDnsConfiguration", *pService, updatePhase);
    }
    else
    {
    	retStatus = STATUS_FAIL_TO_CONFIG_DNS;
    }

	return retStatus;
}

STATUS CMcuMngrManager::ConfigDnsAuto(CIPService *pService)
{
	TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__;
	STATUS retStatus = STATUS_OK;

	// ===== 1. Pizza
	if ( YES != IsTarget() ) // no configuration should be done on Pizzas
	{
		m_isDnsConfigInOsSucceeded = YES;
		TreatDnsConfigurationNewStatus(eDnsNotConfigured, YES);

		TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__
		                       << "\nSystem is not Target; a 'not configured' answer is sent to DnsAgent, and no actual configuration was done!";
	    return retStatus;
	}


	// ===== 1. Target
	if ( m_dhcp_dnsServerIpStr.empty() ) // dhcp failed
	{
		retStatus = STATUS_FAIL;

		m_isDnsConfigInOsSucceeded = NO;
		TreatDnsConfigurationNewStatus(eDnsConfigurationFailure, NO);

		TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__ << " - no dnsServerIp from dhcp; "
		                       << "status: " << m_pProcess->GetStatusAsString(retStatus).c_str();
	}

	else // dhcp_dns config succeeded
	{
		TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__ << " - Start RegisterDnsClient";

		m_isDnsConfigInOsSucceeded = YES;
		TreatDnsConfigurationNewStatus(eDnsConfigurationSuccess, YES);

		retStatus = RegisterDnsClient(pService, YES);
		TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__ << " - RegisterDnsClient failed; Status: "
	            	           << m_pProcess->GetStatusAsString(retStatus).c_str();

		if (STATUS_OK == retStatus) // dns registartion succeeded
		{
			RemoveActiveAlarmByErrorCode(AA_DNS_REGISTRAION_FAILED);
			m_bRestoreDnsRegistrationFailed_AA = FALSE;
		}
		else
		{
			UpdateStartupConditionByErrorCode(AA_DNS_REGISTRAION_FAILED, eStartupConditionFail);
			m_bRestoreDnsRegistrationFailed_AA = TRUE;
		}
	} // end dhcp_dns config succeeded

	return retStatus;
}

/////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::ConfigDnsSpecify(CIPService *pService,eMngmntIpUpdatePhase updatePhase)
{
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::ConfigDnsSpecify";
	STATUS retStatus = STATUS_OK;

	// ===== 1. config Dns (resolving)
	// in case of (JITCMode && ManagmentSeparation), the resolving is done by CSMngr (with IpService params)
	//    instead of by McuMngr (with MngmntNetworkService params)
  //  if ( false == ::IsJitcAndNetSeparation() )
  //  {
		if((pService->GetIpServiceType() == eIpServiceType_Management)&&  (updatePhase != eMngmntIpFromEma))
			return STATUS_OK;

		if(pService->GetIpServiceType() == eIpServiceType_Management)
			retStatus = ::ConfigDnsInOS(pService, "CMcuMngrManager::ConfigDnsSpecify");


  //  }

	if (STATUS_OK != retStatus) // config Dns failed
	{
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::ConfigDnsSpecify - "
		                       << "Config Dns failed; status: "
		                       << m_pProcess->GetStatusAsString(retStatus).c_str();

		m_isDnsConfigInOsSucceeded = NO;
		TreatDnsConfigurationNewStatus(eDnsConfigurationFailure, NO);
	}

	else // config Dns succeeded
	{
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::ConfigDnsSpecify - "
		                       << "Config Dns succeeded! Start RegisterDnsClient";

		m_isDnsConfigInOsSucceeded = YES;
		TreatDnsConfigurationNewStatus(eDnsConfigurationSuccess, YES);

		// ===== 2. register as client
		retStatus = RegisterDnsClient(pService, NO);
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::ConfigDnsSpecify - "
		                       << "RegisterDnsClient status: "
		                       << m_pProcess->GetStatusAsString(retStatus).c_str();

		if (STATUS_OK == retStatus) // dns registartion succeeded
		{
			RemoveActiveAlarmByErrorCode(AA_DNS_REGISTRAION_FAILED);
			m_bRestoreDnsRegistrationFailed_AA = FALSE;
		}
		else
		{
			UpdateStartupConditionByErrorCode(AA_DNS_REGISTRAION_FAILED, eStartupConditionFail);
			m_bRestoreDnsRegistrationFailed_AA = TRUE;
		}
	}

	return retStatus;
}

/////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::ConfigDnsOff()
{
	STATUS configOffStat = STATUS_OK;
	eProductType productType =	CProcessBase::GetProcess()->GetProductType();

	// ===== 1. call DnsConfig method with '0.0.0.0' (Configurator ignores it then) [checking IsTarget is done in ConfiguratorManager]
	if (productType != eProductTypeEdgeAxis)
	{
	    CConfigManagerApi api;
	    configOffStat = api.ConfigureDnsServers("", "0.0.0.0", "0.0.0.0", "0.0.0.0");

        // ===== 2. print to log
        TRACESTR(eLevelInfoNormal)
    	    << "\nCMcuMngrManager::ConfigDnsOff -"
    	    << "Dns server mode is Off; sent '0.0.0.0' to Configurator"
    	    << "\nConfiguration status: " << m_pProcess->GetStatusAsString(configOffStat);
	}

	// (ActiveAlarm AA_NO_DNS_CONFIGURATION will be removed in TreatDnsConfigurationNewStatus)
	RemoveActiveAlarmByErrorCode(AA_DNS_REGISTRAION_FAILED);
	m_bRestoreDnsRegistrationFailed_AA = FALSE;

	m_isDnsConfigInOsSucceeded = YES; // only for a specific checking (at the end of OnMngmntIpConfigInd())
	TreatDnsConfigurationNewStatus(eDnsNotConfigured, YES);

	return STATUS_OK;
}

eProductType CMcuMngrManager::ConvertPlatformTypeToProductType(ePlatformType platformType)
{
	eProductType productType = eProductTypeUnknown, ePT;

    eProductFamily curProductFamily = CProcessBase::GetProcess()->GetProductFamily();
    if (eProductFamilyCallGenerator == curProductFamily)
    {
    	productType = CProcessBase::GetProcess()->GetProductType();
    }
    else
    {
		switch(platformType)
		{
			case eGideonLite:
				if (NULL != getenv ("SOFT_MCU"))//OLGA temp
					productType = eProductTypeSoftMCU;
				else if (NULL != getenv ("SOFT_MCU_MFW"))
					productType = eProductTypeSoftMCUMfw;
				else if (NULL != getenv("GESHER"))
					productType = eProductTypeGesher;
				else if (NULL != getenv("NINJA"))
					productType = eProductTypeNinja;
				else if (NULL != getenv ("SOFT_MCU_EDGE"))
					productType = eProductTypeEdgeAxis;
				else if (NULL != getenv ("SOFT_MCU_CG"))
					productType = eProductTypeCallGeneratorSoftMCU;
				else
			        productType = eProductTypeRMX2000;	// 'eGideonLite' is sent on RMX2000, therefore 'RMX 2000' should be returned
				break;

			case eAmos:
				productType = eProductTypeRMX4000;
				break;

			case eYona:
			    productType = eProductTypeRMX1500;
			    break;

			default:
				productType = eProductTypeUnknown;
		}
    }

	return productType;
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::ValidateProductType()
{
	ePlatformType platformTypeFromSwitch = (ePlatformType)(m_authentication.GetPlatformType());
	eProductType  productTypeFromSwitch  = ConvertPlatformTypeToProductType(platformTypeFromSwitch);

	const char* productTypeFromSwitchStr = ::ProductTypeToString(productTypeFromSwitch);
	const char* productTypeFromFileStr   = ::ProductTypeToString(m_mcuMngrProductType);

    // ===== 1. print to log
    TRACESTR(eLevelInfoNormal)  << "\nCMcuMngrManager::ValidateProductType"
    						<< "\nProduct type from Switch: " << productTypeFromSwitchStr << "(" << (DWORD)productTypeFromSwitch << ")"
                            << "\nProduct type from file:   " << productTypeFromFileStr   << "(" << (DWORD)m_mcuMngrProductType  << ")";


////////////////////////////////////////////////////////////////
//    Temp: for the temp version that contains Switch from V4
////////////////////////////////////////////////////////////////
    // ===== 2. validate
    if (m_mcuMngrProductType != productTypeFromSwitch)
	{
	    //   ----- 2a. produce an Alert
        AddActiveAlarm(	FAULT_GENERAL_SUBJECT,
        				AA_PRODUCT_TYPE_MISMATCH,
                        MAJOR_ERROR_LEVEL,
                        "Different product types in file and from Switch",
                        true,
                        true);


        //   ----- 2b. set the correct type (for next startup)
    	CConfigManagerApi cfgApi;
    	cfgApi.SetProductType(productTypeFromSwitch);

    	/******************************************************************************************/
    	/* 5.8.10 VNGR-16063 fixed by Rachel & Haggai                                             */
    	/* when we start up on new RMX1500 the default product type is RMX2000 so we create       */
    	/* RtmIsdnSpanMapList.xml with slotid=1 instead of 13 (it is hard codded). we need to     */
    	/* delete the file.                                                                       */
    	/******************************************************************************************/
    	DeleteFile(RTM_ISDN_SPAN_MAP_LIST_PATH);


        //   ----- 2c. reset
    	if(IsTarget())
    	{
    		CIPMCInterfaceApi apiIPMC;
    		apiIPMC.SetWatchdog(5);
    		apiIPMC.TriggerWatchdog();




    		string desc = "Product Type mismatch";
			CMcmsDaemonApi dmnApi;
			dmnApi.SendResetReq(desc);

			SystemSleep(10);
    	}
	}

//    End Temp

    return;
}

void CMcuMngrManager::SetSecureMcuState(eMcuState mcuState)
{
    // Check if there is at least AA.
    DWORD numOfAA = m_pProcess->GetMcuStateObject()->GetNumOfActiveAlarms();
    if(0 == numOfAA && eMcuState_Major == mcuState)
    {
        eMcuState oldMcuState = m_pProcess->GetMcuStateObject()->GetMcuState();
        TreatBadStateManagement(oldMcuState, mcuState, numOfAA);
        return;
    }

    eMcuState oldState = m_pProcess->GetMcuStateObject()->GetMcuState();

    m_pProcess->GetMcuStateObject()->SetMcuState(__FUNCTION__, mcuState);

    // if state has changed -> notify NotificationMngr
    if( mcuState != oldState )
    	SendMcuStateToNotificationMngr();
}

void CMcuMngrManager::TreatBadStateManagement(eMcuState oldMcuState, eMcuState newMcuState, DWORD numOfAA)const
{
    CLargeString message = "OnBadStateManage - ";
    message << GetMcuStateName(oldMcuState) << " -> " << GetMcuStateName(newMcuState)
            << "; Num of AA : " << numOfAA;
    PASSERTMSG(TRUE, message.GetString());

    // send terminal command to mcms. all processes will send their AA to logger
    string output_string;
    SystemPipedCommand("Bin/McuCmd show_aa mcms", output_string);
}

STATUS CMcuMngrManager::HandleTerminalCreateStatusFile(CTerminalCommand & command, std::ostream& answer)
{
	CXMLDOMElement* pXMLRootElement =  new CXMLDOMElement;
	pXMLRootElement->set_nodeName("StringConfiguration");

	CXMLDOMElement* pNode = pXMLRootElement->AddChildNode("Translations");
	CXMLDOMElement* pLanguageNode = pNode->AddChildNode("Language");

	CXMLDOMAttribute* pLanguage_Attribute = new CXMLDOMAttribute();
	pLanguage_Attribute->set_nodeName("name");
	pLanguage_Attribute->SetValueForElement("English");
	pLanguageNode->AddAttribute(pLanguage_Attribute);

	m_pProcess->SerializeApiStatuses(pLanguageNode);

	char* pszStrConfigListXml;
	DWORD nStrConfigListXmlLen;

	FILE* pFileOperDB = fopen(RMX_STATUSES_FILE.c_str(),"w");

	if(!pFileOperDB)
	{
		answer << "Failed to create XML status file - fopen failed";
		return STATUS_OK;
	}

	pXMLRootElement->DumpDataAsStringWithAttribute(&pszStrConfigListXml,&nStrConfigListXmlLen,0,TRUE);

	if(pszStrConfigListXml)
	{
		fprintf(pFileOperDB,"%s",pszStrConfigListXml);
		DEALLOCBUFFER(pszStrConfigListXml);
	}

	int fcloseReturn = fclose(pFileOperDB);
	if (FCLOSE_SUCCESS != fcloseReturn)
		answer << "Failed to create XML status file - fclose failed";
	else
		answer << "The file has been created successfully";

	return STATUS_OK;
}

STATUS CMcuMngrManager::HandleTerminalCreateFaultAndAAFile(CTerminalCommand & command, std::ostream& answer)
{
	CXMLDOMElement* pXMLRootElement =  new CXMLDOMElement;
	pXMLRootElement->set_nodeName("StringConfiguration");

	CXMLDOMElement* pActiveAlarmsListNode = pXMLRootElement->AddChildNode("ActiveAlarmsList");

	CXMLDOMElement* pNode = pActiveAlarmsListNode->AddChildNode("Translations");
	CXMLDOMElement* pAALanguageNode = pNode->AddChildNode("Language");

	CXMLDOMAttribute* pAALanguage_Attribute = new CXMLDOMAttribute();
	pAALanguage_Attribute->set_nodeName("name");
	pAALanguage_Attribute->SetValueForElement("English");
	pAALanguageNode->AddAttribute(pAALanguage_Attribute);

	CXMLDOMElement* pFaultsListNode = pXMLRootElement->AddChildNode("FaultsList");

	pNode = pFaultsListNode->AddChildNode("Translations");
	CXMLDOMElement* pFaultsLanguageNode = pNode->AddChildNode("Language");

	CXMLDOMAttribute* pLanguage_Attribute = new CXMLDOMAttribute();
	pLanguage_Attribute->set_nodeName("name");
	pLanguage_Attribute->SetValueForElement("English");
	pFaultsLanguageNode->AddAttribute(pLanguage_Attribute);

	SerializeFaultsAndActiveAlarms(pAALanguageNode, pFaultsLanguageNode);

	char* pszStrConfigListXml;
	DWORD nStrConfigListXmlLen;

	FILE* pFileOperDB = fopen(RMX_FAULTS_AND_AA_FILE.c_str(),"w");

	if(!pFileOperDB)
	{
		answer << "Failed to create XML faules and active alarms file - fopen failed";
		return STATUS_OK;
	}

	pXMLRootElement->DumpDataAsStringWithAttribute(&pszStrConfigListXml,&nStrConfigListXmlLen,0,TRUE);

	if(pszStrConfigListXml)
	{
		fprintf(pFileOperDB,"%s",pszStrConfigListXml);
		DEALLOCBUFFER(pszStrConfigListXml);
	}

	int fcloseReturn = fclose(pFileOperDB);
	if (FCLOSE_SUCCESS != fcloseReturn)
		answer << "Failed to create XML status file - fclose failed";
	else
		answer << "The file has been created successfully";

	return STATUS_OK;
}

STATUS CMcuMngrManager::HandleTerminalConfigApache(CTerminalCommand & command, std::ostream& answer)
{
	DWORD numOfParams = command.GetNumOfParams();
	if(1 != numOfParams)
	{
		answer << "error: Illegal number of parameters\n";
		answer << "usage: Bin/McuCmd config_apache McuMngr [YES/NO]\n";
		return STATUS_FAIL;
	}

	const string &strIsNewService = command.GetToken(eCmdParam1);
	if ( ("YES" != strIsNewService) && ("NO" != strIsNewService) )
	{
		answer << "error: Illegal number of parameters\n";
		answer << "usage: Bin/McuCmd config_apache McuMngr [YES/NO]\n";
		return STATUS_FAIL;
	}

	if ("NO" == strIsNewService)
	{
		ConfigGeneral();
	}
	else
	{
		ConfigGeneral(m_pServiceForAccessOnNetConfigFailure);
	}

	CMcmsDaemonApi api;
    api.SendConfigApacheInd();

    return STATUS_OK;
}

STATUS CMcuMngrManager::HandleTerminalRestartApache(CTerminalCommand& cmd,
                                                    std::ostream& ans)
{
  // Sends asynchronous request to restart Apache as root
  eProcessType ptype = eProcessConfigurator;
  OPCODE opcode = CONFIGURATOR_RESTART_APACHE;

  STATUS stat = CManagerApi(ptype).SendOpcodeMsg(opcode);
  if (STATUS_OK == stat)
  {
    ans << "Request " << m_pProcess->GetOpcodeAsString(opcode)
        << " (" << opcode << ") to " << ProcessNames[ptype]
        << " was sent successfully";
  }
  else
  {
    ans  << "Unable to send " << m_pProcess->GetOpcodeAsString(opcode)
         << " (" << opcode << ") to " << ProcessNames[ptype]
         << ": " << m_pProcess->GetStatusAsString(stat);
  }

  return STATUS_OK;
}

//STATUS CMcuMngrManager::HandleTerminalSendIpIndSnmp(CTerminalCommand & command, std::ostream& answer)
//{
//    SendMNGMNTInfoToSnmp(SNMP_MNGMNT_IP_CONFIG_IND);
//    return STATUS_OK;
//}

STATUS CMcuMngrManager::HandleTerminalStartResetTimer(CTerminalCommand & command, std::ostream& answer)
{
    StartResetTimer("reset caused by terminal command", 5);
    return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::HandleTerminalMacAddressConfig(CTerminalCommand & command, std::ostream& answer)
{
	answer << "Eth1 MAC address:  " << m_pMacAddressConfig->GetEth1MacAddress() << "\n";
	answer << "Eth2 MAC address:  " << m_pMacAddressConfig->GetEth2MacAddress() << "\n";

	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::HandleTerminalUpdateNetworkConfiguration(CTerminalCommand & command, std::ostream& answer)
{
	   DWORD numOfParams = command.GetNumOfParams();
		if(3 != numOfParams)
		{
			answer << "error: Illegal number of parameters\n";
			answer << "usage: McuCmd IP MASK Default_GW";
			return STATUS_FAIL;
	    }

	    CIPService* pUpdateManagment = new CIPService;
	    const string &IP_address = command.GetToken(eCmdParam1);
	    const string &mask = command.GetToken(eCmdParam2);
	    const string &default_GW = command.GetToken(eCmdParam3);

	    pUpdateManagment->SetName(MANAGEMENT_NETWORK_NAME);
	    pUpdateManagment->SetIpServiceType(eIpServiceType_Management);

	    DWORD dwMask = SystemIpStringToDWORD(mask.c_str());
	    pUpdateManagment->SetNetMask(dwMask);

	    DWORD dwDefaultGW = SystemIpStringToDWORD(default_GW.c_str());
	    pUpdateManagment->SetDefaultGatewayIPv4(dwDefaultGW);

	    CIPSpan span;
		DWORD dwIpAddrr = SystemIpStringToDWORD(IP_address.c_str());
	    span.SetIPv4Address(dwIpAddrr);
	    pUpdateManagment->AddSpan_NoCheck(span);

	    STATUS retStatus = STATUS_OK;

	    if(STATUS_OK == retStatus)
	    {
	        // update memory.
	        m_pProcess->SetMngmntIpParams_asIpService(*pUpdateManagment);
	        m_pMngmntIpParams_asIpService_fromProcess = m_pProcess->GetMngmntIpParams_asIpService();

	        // update tmp file.
	        pUpdateManagment->WriteXmlFile(MANAGEMENT_NETWORK_CONFIG_PATH, "MngmntNetwork");

			AddActiveAlarmSingleton( FAULT_GENERAL_SUBJECT,
								SYSTEM_CONFIGURATION_CHANGED,
								 MAJOR_ERROR_LEVEL,
								 "System configuration changed. Please reset the MCU.",
								 true,
								 true
								);

	    }

	    return STATUS_OK;
}

//void CMcuMngrManager::OnSNMP_ManagmentInterfaceIpRequest(CSegment* pMsg)
//{
//     SendMNGMNTInfoToSnmp(SNMP_MNGMNT_INTERFACE_IP_IND);
//}

void CMcuMngrManager::OnUpdateCertificateInd(CSegment* pMsg)
{
  CIPService* pService = m_pMngmntIpParams_asIpService_fromProcess;
  if (pService && pService->GetIsSecured() == TRUE)
  {
    TRACEINTOFUNC << "Secured mode";

    CManagementSecurity* sec = m_pMngmntIpParams_asIpService_fromProcess->GetManagementSecurity();
    PASSERT_AND_RETURN(NULL == sec);
    bool ca_validation = sec->IsRequestPeerCertificate();
    std::string host_name =GetHostNameFromService(m_pMngmntIpParams_asIpService_fromProcess);

    BYTE revocation_method = sec->GetRevocationMethodType();
    STATUS statRes = CheckForValidCertificate(host_name.c_str(), true, ca_validation,revocation_method);

    if (STATUS_CERTIFICATE_IS_GOING_TO_BE_EXPIRED == statRes)
        	statRes = STATUS_OK;

    if (statRes == STATUS_OK) //if to update apache to go back to secure mode
    {

      char iPv4Str[IP_ADDRESS_LEN] = {0};
      CIPSpan *pSpan = pService->GetSpanByIdx(0);
      if(pSpan)
    	  SystemDWORDToIpString(pSpan->GetIPv4Address(), iPv4Str);

      ConfigApache(pService->GetIpType(),
                   iPv4Str,
                   pService->GetSpanByIdx(0)->GetIPv6Address(),
                   pService->GetIsPermanentNetworkOpen(),
                   pService->GetIsSecured(),
                   sec->IsRequestPeerCertificate(),
                   sec->GetRevocationMethodType(),
                   sec->IsUseResponderOcspUri(),
                   sec->IsIncompleteRevocation(),
                   sec->IsSkipValidationOcspCert(),
                   sec->GetOCSPGlobalResponderURI());
    }

    CMcmsDaemonApi api;
    api.SendResetProcessReq(eProcessApacheModule);
  }
}

/////////////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnV35GwUpdateInd(CSegment* pMsg)
{


	if (!m_pMngmntIpParams_asIpService_fromProcess)
	{
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnV35GwUpdateInd"
							   << "\nm_pMngmntIpParams_asIpService_fromProcess is NULL!!";

		return;
	}

	CIPService* pService = m_pMngmntIpParams_asIpService_fromProcess;


	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnV35GwUpdateInd";

	BOOL bIsV35GwEnabled	= FALSE;
	WORD boardId			= 0,
		 pqId				= 0,
		 count 				= 0;
	std::string ipServiceName,sslPort,username,password;
	*pMsg >> bIsV35GwEnabled;
	// set isEnbled member
	m_bIsV35GwEnabledInService = bIsV35GwEnabled;
	//if(bIsV35GwEnabled)
	std::list<RvgwItem>  listOfv35Services;
	std::string listOfPorts;
	char iPv4Str[IP_ADDRESS_LEN] = {0};
	if (pService->GetSpanByIdx(0) != NULL)
	    SystemDWORDToIpString(pService->GetSpanByIdx(0)->GetIPv4Address(), iPv4Str);
	if(m_bIsV35GwEnabledInService)
	{
		//num of service that V35 are enabled
		*pMsg >> count;
		for(int i=0;i<count;i++)
		{
			*pMsg >> boardId
					>> pqId
					>> ipServiceName
					>> sslPort
					>>username
					>>password;

			// set internalGwAddress member
			DWORD vlanId = CalcMSvlanId(boardId, pqId);
			DWORD internalGwAddress = GetVlanCardInternalIpv4Address(vlanId);

			char internalGwAddressStr[IP_ADDRESS_LEN] = {0};
			SystemDWORDToIpString(internalGwAddress, internalGwAddressStr);
			TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnV35GwUpdateInd\n"
									<< "boardId :" <<boardId
									<< "pqId :" << pqId
									<< "vlanId :" <<vlanId;
			RvgwItem item;

			item.m_aliasName = ipServiceName;
			item.m_internalIP = internalGwAddressStr;
			item.m_sslport = sslPort;
			item.m_userName = username;
			item.m_password =password;
			listOfv35Services.push_back(item);
			if((i+1) < count)
			{
				listOfPorts += sslPort + ",";
			}
			else
				listOfPorts += sslPort;
		}
		CConfigManagerApi configApi;
		configApi.OpenPortsForApache(iPv4Str,listOfPorts);
	}
	m_apacheConfig.m_v35GwList = listOfv35Services;


	// set internalGwAddress member
	/*DWORD vlanId = CalcMSvlanId(boardId, pqId);
	DWORD internalGwAddress = GetVlanCardInternalIpv4Address(vlanId);

	char internalGwAddressStr[IP_ADDRESS_LEN] = {0};
	SystemDWORDToIpString(internalGwAddress, internalGwAddressStr);

	m_sInternalGwAddress = internalGwAddressStr;*/

	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnV35GwUpdateInd"
						   << "\nV35 Gateway:       " << (m_bIsV35GwEnabledInService ? "enabled" : "disabled")
						   //<< "\nInternalGWAddress: " << m_sInternalGwAddress
						   << "\nCount list of V35 Gw:		" << count;

	//do not configure if no service enabled
	if(!m_bIsV35GwEnabledInService)
		return;
	//do not configure if no service enabled
	if(!m_bIsV35GwEnabledInService)
			return;
// TODO fix this. - bug that fixed in UC-APL 
	CManagementSecurity* sec = pService->GetManagementSecurity();
	PASSERT_AND_RETURN(NULL == sec);

	std:string cHostName = GetHostNameFromService(pService);

	ConfigApache( pService->GetIpType(),
				  iPv4Str,
				  pService->GetSpanByIdx(0)->GetIPv6Address(),
				  pService->GetIsPermanentNetworkOpen(),
				  pService->GetIsSecured(),
				  sec->IsRequestPeerCertificate(),
				  sec->GetRevocationMethodType(),
				  sec->IsUseResponderOcspUri(),
				  sec->IsIncompleteRevocation(),
				  sec->IsSkipValidationOcspCert(),
				  sec->GetOCSPGlobalResponderURI(),
				  cHostName);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnSystemCardsModeInd(CSegment* pMsg)
{
    DWORD tempMode = (DWORD) eSystemCardsMode_illegal;
    *pMsg >> tempMode;

    m_mcuMngrRmxSystemCardsMode = (eSystemCardsMode)tempMode;

    m_pProcess->GetMcuStateObject()->SetMcuStateSystemCardsMode(m_mcuMngrRmxSystemCardsMode );
    m_pLicensing->SetSystemCardsMode(m_mcuMngrRmxSystemCardsMode);
    SendAuthenticationStructToProcesses(); // for card mode
    TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnSystemCardsModeInd"
                           << "\nMode received from Cards process: " << ::GetSystemCardsModeStr(m_mcuMngrRmxSystemCardsMode);

     /************************************************************************************/
     /* added by Richer for 802.1x project om 2013.12.26                                                  */
     /* adding the 802.1x authentication protocol for each lan port for Ninja
       * need to delay it till cards are up                     */
     /************************************************************************************/
     if (eProductTypeGesher == m_mcuMngrProductType || eProductTypeNinja == m_mcuMngrProductType)
     {
         Start802_1xConfiguration();
         StartTimer(MCUMNGR_802_1x_WAIT_FOR_CARDS_TIMER, SECOND * 60*2);
     }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnShmCompSlotIdInd(CSegment* pMsg)
{
	RemoveActiveAlarmByErrorCode(AA_UNKNOWN_CPU_SLOT_ID);

    DWORD tmpCpuBoardId		= 0,
    	  tmpCpuSubBoardId	= 0,
          tmpCompType		= 0;

    eShmComponentType compType = eShmComp_Illegal;

    *pMsg >> tmpCpuBoardId
          >> tmpCpuSubBoardId
          >> tmpCompType;

    m_cpuBoardId	= (BYTE)tmpCpuBoardId;
    m_cpuSubBoardId	= (BYTE)tmpCpuSubBoardId;
    compType		= (eShmComponentType)tmpCompType;

    TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnShmCompSlotIdInd"
                           << "\nBoardId:    " << (int)m_cpuBoardId
                           << "\nSubBoardId: " << (int)m_cpuSubBoardId
                           << "\nCmpnntType: " << ::ShelfMngrComponentTypeToString(compType);

    	/***********************************************/
    	/* 21.4.10 VNGR-13801  changed by Rachel Cohen */
    	/***********************************************/
    	ConfigEthernetCpuAndSwitch();
}
/////////////////////////////////////////////////////////////////////////////////////

void  CMcuMngrManager::OnRtmLanAndIsdnSlotInd(CSegment* pMsg)
{


	CLargeString str = "\nCMcuMngrManager::OnRtmLanAndIsdnSlotInd";


	SLOTS_CONFIGURATION_S mfaBoardCnfArray[MAX_NUM_OF_SLOTS];
	memset(mfaBoardCnfArray,0,MAX_NUM_OF_SLOTS*sizeof(SLOTS_CONFIGURATION_S));

	pMsg->Get( (BYTE*)mfaBoardCnfArray, MAX_NUM_OF_SLOTS*sizeof(SLOTS_CONFIGURATION_S) );



    CEthernetSettingsConfigList   *pEthList = m_pProcess->GetEthernetSettingsConfigList();

    eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
    BYTE bMultipleServices = GetMultipleServices();

    if(pEthList)
    {
        /* In case we have RMX1500 with no RTM_ISDN  */
        if ((eProductTypeRMX1500 == curProductType) && (bMultipleServices == YES)  && (mfaBoardCnfArray[1].rtmCompType == eShmComp_Illegal) )
        {
            CEthernetSettingsConfig* pEthernetSettingsConfig = pEthList->GetSpecEthernetSettingsConfig(0);
            if (pEthernetSettingsConfig)
            {
                pEthernetSettingsConfig->SetSlotId(FIXED_BOARD_ID_RTM_1);
                pEthernetSettingsConfig->SetPortType(eEthPortType_Media_Signaling);
                pEthList->SetSpecEthernetSettingsConfig(*pEthernetSettingsConfig);
            }

            pEthernetSettingsConfig = pEthList->GetSpecEthernetSettingsConfig(1);
            if (pEthernetSettingsConfig)
            {
                pEthernetSettingsConfig->SetPortType(eEthPortType_Media_Signaling);
                pEthList->SetSpecEthernetSettingsConfig(*pEthernetSettingsConfig);
            }


            CEthernetSettingsConfig* pEthernetSettingsSigPort = pEthList->GetSpecEthernetSettingsConfig(ETH_SETTINGS_CPU_SIGNALLING_PORT_IDX_RMX1500);
            if (pEthernetSettingsSigPort)
            {
                pEthernetSettingsSigPort->SetSlotId(0);
                pEthList->SetSpecEthernetSettingsConfig(*pEthernetSettingsSigPort);
            }
            return;
        }
    }

    if(pEthList)
    {

    	for (int i=0;i<MAX_NUM_OF_SLOTS;i++)
    	{

    		/*TRACEINTO <<   "\nRTM LAN/ISDN  Params:\nMediaCompType" << mfaBoardCnfArray[i].mediaCompType
    				<< "\nRtmCompType "  << mfaBoardCnfArray[i].rtmCompType
    				<< "\nSubBoardId  "  << mfaBoardCnfArray[i].unSubBoardId
    				<< "\nBoardId      "  << i;*/


    		DWORD DisplayBoardId=0;
    		DisplayBoardId = m_pSlotsNumConversionTable->GetDisplayBoardId(i, 2);
    		CEthernetSettingsConfig* pEthernetSettingsConfig = pEthList->GetSpecEthernetSettingsConfig(GetEthernetSettingPos(i));

    		if(pEthernetSettingsConfig)
    		{

    			//update RtmLan Slots

    			if  (mfaBoardCnfArray[i].rtmCompType == eShmComp_RtmLan ||
    					      ((mfaBoardCnfArray[i].rtmCompType == eShmComp_RtmIsdn9 ||
    							mfaBoardCnfArray[i].rtmCompType == eShmComp_RtmIsdn9_10G ||
    							mfaBoardCnfArray[i].rtmCompType == eShmComp_RtmIsdn9_10G ||
    							mfaBoardCnfArray[i].rtmCompType == eShmComp_RtmLan4 ||
    							mfaBoardCnfArray[i].rtmCompType == eShmComp_RtmLan4_10G) &&
    							(mfaBoardCnfArray[i].mediaCompType == eShmComp_MfaMpmRx )))
    			{

    				if (mfaBoardCnfArray[i].rtmCompType    == eShmComp_RtmIsdn9 || mfaBoardCnfArray[i].rtmCompType    == eShmComp_RtmIsdn9_10G)
    					pEthernetSettingsConfig->SetMediaPortType(eEthMediaPortType_Rtm_Isdn);
    				else
    					pEthernetSettingsConfig->SetMediaPortType(eEthMediaPortType_Rtm_Lan);
    				pEthernetSettingsConfig->SetSlotId(DisplayBoardId);
    				pEthernetSettingsConfig->SetPortId(ETH_SETTINGS_MEDIA_2_PORT_ON_MEDIA_BOARD);


    				pEthList->SetSpecEthernetSettingsConfig(*pEthernetSettingsConfig);
    				SendConfigEthernetSettingsReqToMplApi(pEthernetSettingsConfig);

    			}

    			else if((mfaBoardCnfArray[i].rtmCompType    == eShmComp_RtmIsdn9      ||
    					 mfaBoardCnfArray[i].rtmCompType    == eShmComp_RtmIsdn9_10G  ||
    					 mfaBoardCnfArray[i].rtmCompType    == eShmComp_RtmLan4       ||
    					 mfaBoardCnfArray[i].rtmCompType    == eShmComp_RtmLan4_10G ) &&
    					 (mfaBoardCnfArray[i].mediaCompType == eShmComp_MfaMpmx))
    			{

    				if (mfaBoardCnfArray[i].rtmCompType    == eShmComp_RtmIsdn9 || mfaBoardCnfArray[i].rtmCompType    == eShmComp_RtmIsdn9_10G)
    					pEthernetSettingsConfig->SetMediaPortType(eEthMediaPortType_Rtm_Isdn);
    				else
    					pEthernetSettingsConfig->SetMediaPortType(eEthMediaPortType_Rtm_Lan);
    				pEthernetSettingsConfig->SetSlotId(DisplayBoardId);
    				pEthernetSettingsConfig->SetPortId(ETH_SETTINGS_MEDIA_4_PORT_ON_MEDIA_BOARD);

    				pEthList->SetSpecEthernetSettingsConfig(*pEthernetSettingsConfig);
    				SendConfigEthernetSettingsReqToMplApi(pEthernetSettingsConfig);


    			}

    			//update RtmIsdn Slots
    			else if  (mfaBoardCnfArray[i].rtmCompType == eShmComp_RtmIsdn)
    			{


    				if (eProductTypeRMX1500 != curProductType)
    				{


    					pEthernetSettingsConfig->SetMediaPortType(eEthMediaPortType_Rtm_Isdn);
    					pEthernetSettingsConfig->SetSlotId(DisplayBoardId);
    					pEthernetSettingsConfig->SetPortId(ETH_SETTINGS_MEDIA_1_PORT_ON_MEDIA_BOARD);


    					pEthList->SetSpecEthernetSettingsConfig(*pEthernetSettingsConfig);
    					SendConfigEthernetSettingsReqToMplApi(pEthernetSettingsConfig);


    				}
    			}



    		}

    	}
    	/************************************************************************************/
    	/* 28.4.10 changed by Rachel Cohen                                                  */
    	/* That function reach the configurator to config "ethtool -s eth1 autoneg on"      */
    	/* the following command get loss of the managment connection and the Ema get reset */
    	/* that happened  in version 5.1.                                                   */
    	/************************************************************************************/

    	//ConfigEthernetCpuAndSwitch();

    	/************************************************************************************/
    	/* 2.12.12 changed by Rachel Cohen                                                  */
    	/* adding the 802.1x authentication protocol for each lan port
    	 * need to delay it till cards are up                     */
    	/************************************************************************************/
    	Start802_1xConfiguration();
    	StartTimer(MCUMNGR_802_1x_WAIT_FOR_CARDS_TIMER, SECOND * 60*2);

    }
    else
    	PASSERTMSG(1, "CMcuMngrManager::OnRtmLanAndIsdnSlotInd, Invalid pEthList");
}

/////////////////////////////////////////////////////////////////////////////////////

/************************************************************************************/
/* 12.1.11 changed by Rachel Cohen                                                  */
/* we enter OnRtmLanInd function after media_ip_config msg arrive from MPL.         */
/* we need to check if Multiple Service is on and MS license bit is on and than     */
/* update ethernet setting list with the new ports.                                 */
/************************************************************************************/
void  CMcuMngrManager::OnRtmLanInd(CSegment* pMsg)
{
    WORD board_id = 0;
    WORD port_id  = 0;

    *pMsg >> board_id;
    *pMsg >> port_id;


    TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnRtmLanInd"<< "\nBoard id " << board_id << "port id "<<port_id;

    CEthernetSettingsConfigList	*pEthList = m_pProcess->GetEthernetSettingsConfigList();

    eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

    if(pEthList)
    {

        DWORD DisplayBoardId=0;
        DisplayBoardId = m_pSlotsNumConversionTable->GetDisplayBoardId(board_id, 2);


        /*********************************************************************************************/
        /* config eth setting for the recieved boardid if it is portid == 2 it is RTM_LAN and we need*/
        /* to set port id 2 and if Multiple Service also port 1                                      */
        /* if portid==1 it is RTM_ISDN board and we need to set entry of port 1                      */
        /*********************************************************************************************/


        BYTE bSeparateNetworks  = GetSeparateNetworksSupport();
        BYTE bMultipleServices = GetMultipleServices();
        BYTE bRtmLam = GetRtmLanSupport();
        if (bMultipleServices == YES)
        {
            if (port_id == 2 || port_id == 4) //on MPMX with new RTM_LAN4/RTM_LAN4_10G/RTM_ISDN9/RTM_ISDN9_10G the ports are 3,4
            {
                CEthernetSettingsConfig* pEthernetSettingsConfigPort2 = pEthList->GetSpecEthernetSettingsConfig(GetEthernetSettingPos(board_id));
                if (pEthernetSettingsConfigPort2)
                {
                    pEthernetSettingsConfigPort2->SetSlotId(DisplayBoardId);
                    pEthernetSettingsConfigPort2->SetPortId(port_id);

                    //change the port type of port 2 to media_signalling
                    pEthernetSettingsConfigPort2->SetPortType(eEthPortType_Media_Signaling);
                    pEthList->SetSpecEthernetSettingsConfig(*pEthernetSettingsConfigPort2);
                    SendConfigEthernetSettingsReqToMplApi(pEthernetSettingsConfigPort2);
                }
            }
            else  //port id 1

            {
                //prepare the entry for port 1 MS
                CEthernetSettingsConfig* pEthernetSettingsConfigPort1 = pEthList->GetSpecEthernetSettingsConfig(GetEthernetSettingMSLanPortsPos(board_id));
                if (pEthernetSettingsConfigPort1)
                {
                    pEthernetSettingsConfigPort1->SetSlotId(DisplayBoardId);
                    pEthernetSettingsConfigPort1->SetPortId(port_id);
                    pEthernetSettingsConfigPort1->SetPortType(eEthPortType_Media_Signaling);


                    pEthList->SetSpecEthernetSettingsConfig(*pEthernetSettingsConfigPort1);
                    SendConfigEthernetSettingsReqToMplApi(pEthernetSettingsConfigPort1);
                }
            }


            /*********************************************************************************************/
            /* VNGFE-4756 18.10.11 added by Rachel Cohen                                                 */
            /* in RMX1500 when multiple service is on port 1 and port 2 should be CS+M1 and CS+M2        */
            /* port 3 is not used. in RMX4000 ports 3 and 5 are not used.                                */
            /*********************************************************************************************/
            eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

            if (eProductTypeRMX4000 == curProductType)
            {
                CEthernetSettingsConfig* pEthernetSettingsSig1Port = pEthList->GetSpecEthernetSettingsConfig(ETH_SETTINGS_CPU_SIGNALLING_1_PORT_IDX_RMX4000);
                if (pEthernetSettingsSig1Port)
                {
                    pEthernetSettingsSig1Port->SetSlotId(0);
                    pEthList->SetSpecEthernetSettingsConfig(*pEthernetSettingsSig1Port);
                }
                else
                    PASSERT(1);

                CEthernetSettingsConfig* pEthernetSettingsSig2Port = pEthList->GetSpecEthernetSettingsConfig(ETH_SETTINGS_CPU_SIGNALLING_2_PORT_IDX_RMX4000);
                if (pEthernetSettingsSig2Port)
                {
                    pEthernetSettingsSig2Port->SetSlotId(0);
                    pEthList->SetSpecEthernetSettingsConfig(*pEthernetSettingsSig2Port);
                }
                else
                    PASSERT(2);
            }
            else if (eProductTypeRMX1500 == curProductType)
            {
                CEthernetSettingsConfig* pEthernetSettingsSigPort = pEthList->GetSpecEthernetSettingsConfig(ETH_SETTINGS_CPU_SIGNALLING_PORT_IDX_RMX1500);
                if (pEthernetSettingsSigPort)
                {
                    pEthernetSettingsSigPort->SetSlotId(0);
                    pEthList->SetSpecEthernetSettingsConfig(*pEthernetSettingsSigPort);
                }
                else
                    PASSERT(3);
            }


        }
        else //bMultipleServices == NO send req to port 2
        {
        	BYTE bRtmLam = GetRtmLanSupport();
        	if (!(eProductTypeRMX2000 == curProductType && bRtmLam == NO))
        	{

        		if (port_id == 2 || port_id == 4) //on MPMX with new RTM_LAN4/RTM_LAN4_10G/RTM_ISDN9/RTM_ISDN9_10G the ports are 3,4
        		{
        			CEthernetSettingsConfig* pEthernetSettingsConfigPort2 = pEthList->GetSpecEthernetSettingsConfig(GetEthernetSettingPos(board_id));
        			if (pEthernetSettingsConfigPort2)
        			{
        				pEthernetSettingsConfigPort2->SetSlotId(DisplayBoardId);
        				pEthernetSettingsConfigPort2->SetPortId(port_id);

        				//change the port type of port 2 to media_signalling
        				pEthernetSettingsConfigPort2->SetPortType(eEthPortType_Media);
        				pEthList->SetSpecEthernetSettingsConfig(*pEthernetSettingsConfigPort2);
        				SendConfigEthernetSettingsReqToMplApi(pEthernetSettingsConfigPort2);
        			}
        		}
        		else  //port id 1

        		{
        			//prepare the entry for port 1
        			CEthernetSettingsConfig* pEthernetSettingsConfigPort1 = pEthList->GetSpecEthernetSettingsConfig(GetEthernetSettingMSLanPortsPos(board_id));
        			if (pEthernetSettingsConfigPort1)
        			{
        				pEthernetSettingsConfigPort1->SetSlotId(DisplayBoardId);
        				pEthernetSettingsConfigPort1->SetPortId(port_id);
        				pEthernetSettingsConfigPort1->SetPortType(eEthPortType_Media);


        				pEthList->SetSpecEthernetSettingsConfig(*pEthernetSettingsConfigPort1);
        				SendConfigEthernetSettingsReqToMplApi(pEthernetSettingsConfigPort1);
        			}
        		}

        	}



            if (eProductTypeRMX4000 == curProductType)
            {
                CEthernetSettingsConfig* pEthernetSettingsSig1Port = pEthList->GetSpecEthernetSettingsConfig(ETH_SETTINGS_CPU_SIGNALLING_1_PORT_IDX_RMX4000);
                if (pEthernetSettingsSig1Port)
                {
                    pEthernetSettingsSig1Port->SetSlotId(FIXED_DISPLAY_BOARD_ID_SWITCH);
                    pEthList->SetSpecEthernetSettingsConfig(*pEthernetSettingsSig1Port);
                }
                else
                    PASSERT(5);


                //sig 2 not implemented yet
                //CEthernetSettingsConfig* pEthernetSettingsSig2Port = pEthList->GetSpecEthernetSettingsConfig(ETH_SETTINGS_CPU_SIGNALLING_2_PORT_IDX_RMX4000);
                //pEthernetSettingsSig2Port->SetSlotId(FIXED_DISPLAY_BOARD_ID_SWITCH);
                //pEthList->SetSpecEthernetSettingsConfig(*pEthernetSettingsSig2Port);
            }
            else if (eProductTypeRMX1500 == curProductType)
            {
                CEthernetSettingsConfig* pEthernetSettingsSigPort = pEthList->GetSpecEthernetSettingsConfig(ETH_SETTINGS_CPU_SIGNALLING_PORT_IDX_RMX1500);
                if (pEthernetSettingsSigPort)
                {
                    pEthernetSettingsSigPort->SetSlotId(FIXED_DISPLAY_BOARD_ID_SWITCH);
                    pEthList->SetSpecEthernetSettingsConfig(*pEthernetSettingsSigPort);
                }
                else
                    PASSERT(6);
            }

        }
        if (eProductTypeRMX2000 == curProductType)
        {
        	CEthernetSettingsConfig* pEthernetSettingsSigPort = pEthList->GetSpecEthernetSettingsConfig(ETH_SETTINGS_SHELF_PORT_IDX_RMX2000);
        	if (pEthernetSettingsSigPort)
        	{
        		if (bMultipleServices == NO)
        		{
        			if (bRtmLam == YES && bSeparateNetworks == NO)
        			{
        			pEthernetSettingsSigPort->SetPortType(eEthPortType_ManagementShelfMngr_Managment_Signaling);
        			pEthList->SetSpecEthernetSettingsConfig(*pEthernetSettingsSigPort);
        			}
        			if (bRtmLam == YES && bSeparateNetworks == YES)
        			{
        				pEthernetSettingsSigPort->SetPortType(eEthPortType_Signaling1);
        				pEthList->SetSpecEthernetSettingsConfig(*pEthernetSettingsSigPort);
        				CEthernetSettingsConfig* pEthernetSettingsMngPort = pEthList->GetSpecEthernetSettingsConfig(ETH_SETTINGS_MODEM_PORT_IDX_RMX2000);

        				if (pEthernetSettingsMngPort)

        				{
        					pEthernetSettingsMngPort->SetPortType(eEthPortType_ManagementShelfMngr_Managment);
        					pEthList->SetSpecEthernetSettingsConfig(*pEthernetSettingsMngPort);

        				}
        				else
        					PASSERT(7);

        			}
        			if (bRtmLam == NO && bSeparateNetworks == YES)
        			{
        				pEthernetSettingsSigPort->SetPortType(eEthPortType_Media_Signaling);
        				pEthList->SetSpecEthernetSettingsConfig(*pEthernetSettingsSigPort);
        				CEthernetSettingsConfig* pEthernetSettingsMngPort = pEthList->GetSpecEthernetSettingsConfig(ETH_SETTINGS_MODEM_PORT_IDX_RMX2000);

        				if (pEthernetSettingsMngPort)

        				{
        					pEthernetSettingsMngPort->SetPortType(eEthPortType_ManagementShelfMngr_Managment);
        					pEthList->SetSpecEthernetSettingsConfig(*pEthernetSettingsMngPort);

        				}
        				else
        					PASSERT(7);

        			}




        		}
        		else  //MS YES
        		{
        			pEthernetSettingsSigPort->SetPortType(eEthPortType_ManagementShelfMngr_Managment);
        			pEthList->SetSpecEthernetSettingsConfig(*pEthernetSettingsSigPort);

        		}



        	}
        	else
        		PASSERT(8);
        }
    }
    else
        PASSERTMSG(4, "CMcuMngrManager::OnRtmLanInd, Invalid pEthList");
}

int CMcuMngrManager::GetEthernetSettingPos(int slot_id)
{
    int retVal =slot_id*2-1;
    if (retVal<0 || retVal>=MAX_NUM_OF_LAN_PORTS)
    {
        TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::GetEthernetSettingPos invalid position";
        return 0;
    }

    return retVal;
}

int CMcuMngrManager::GetEthernetSettingMSLanPortsPos(int slot_id)
{
	int retVal = 2*(slot_id-1);
	if (retVal<0 || retVal>=MAX_NUM_OF_LAN_PORTS)
			{
				TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::GetEthernetSettingMSLanPortsPos invalid position";
				return 0;
			}

			return retVal;
}

//void  CMcuMngrManager::OnQA_Api_ManagmentInterfaceIpRequest(CSegment* pMsg)
//{
//  SendMNGMNTInfoToQA_Api(CQAAPI_MNGMNT_INTERFACE_IP_IND);
//}
//
//void CMcuMngrManager::SendMNGMNTInfoToQA_Api(OPCODE opcode)
//{
//    QA_API_MMGMNT_INFO_S mngmntInfo;
//    mngmntInfo.mngmntIp = m_pProcess->GetMngmntIp();
//
//    CSegment *pSeg = new CSegment;
//    pSeg->Put((BYTE*)&mngmntInfo, sizeof(QA_API_MMGMNT_INFO_S));
//
//    CManagerApi apiQA_Api(eProcessQAAPI);
//    apiQA_Api.SendMsg(pSeg, opcode);
//}

BYTE CMcuMngrManager::CheckIfToGoBackToPort80(STATUS status)
{
	if (status != STATUS_CERTIFICATE_ALREADY_EXPIRED &&
	    status != STATUS_CERTIFICATE_COMMON_NAME_DIFFER_THAN_RMX_HOST_NAME &&
	    status != STATUS_CERTIFICATE_NOT_VALID_YET &&
	    status != STATUS_CERTIFICATE_IS_GOING_TO_BE_EXPIRED)
		return TRUE;

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// the following two methods should be united to a single 'OnSystemRamSizeReq'
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnResourceSystemRamSizeReq(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnResourceSystemRamSizeReq";
	SendSystemRamSizeToSpecProcess(eProcessResource);
}

void CMcuMngrManager::OnResourceSystemCPUProfileReq(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnResourceSystemCPUProfileReq";
	SendSystemCPUProfileToResourceMngr();

}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnConfPartySystemRamSizeReq(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnConfPartySystemRamSizeReq";
	SendSystemRamSizeToSpecProcess(eProcessConfParty);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::HandleTerminalSetMcuType(CTerminalCommand & command, std::ostream& answer)
{
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::HandleTerminalSetMcuType";
	DWORD numOfParams = command.GetNumOfParams();

	if(3 != numOfParams)
	{
		answer << "error: missing arguments";
		answer << "usage: set_eth [bogomips value] [cpu size] [memory]";
		return STATUS_FAIL;
	}

	const string &bogoMipsVal = command.GetToken(eCmdParam1);
	const string &cpuSize    = command.GetToken(eCmdParam2);
	const string &memory	   = command.GetToken(eCmdParam3);

	SetBogmipsValue(atoi(bogoMipsVal.c_str()));
	SetCpuSize(atoi(cpuSize.c_str()));
	SetTotalMemory(atoi(memory.c_str()));
	SendSystemCPUProfileToResourceMngr();
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::HandleTerminalSetMcuType2";

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendSystemCPUProfileToResourceMngr()
{
	WORD grade = GetMachineProfileGrade();
    if (!grade)
    	return;
	// ===== 1. fill the Segment
	CSegment*  pRetParam = new CSegment;
    *pRetParam << (DWORD)grade;

	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendSystemCPUProfileToResourceMngr"
                           << "\nGrade sent to resource manager process: "
                           << grade;

	// ===== 2. send
	const COsQueue* pMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessResource, eManager);

	STATUS res = pMbx->Send(pRetParam, SYSTEM_CPU_PROFILE_IND, &GetRcvMbx());
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendSystemRamSizeToSpecProcess(eProcessType processType)
{
	eSystemRamSize ramSize = GetRamSizeAccordingToTotalMemory("CMcuMngrManager::SendSystemRamSizeToSpecProcess");

	// ===== 1. fill the Segment
	CSegment*  pRetParam = new CSegment;
    *pRetParam << (DWORD)ramSize;

	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendCurSystemRamSizeToSpecProcess"
                           << "\nSize sent to "  << ::ProcessTypeToString(processType) << " process: "
                           << ::GetSystemRamSizeStr(ramSize);

	// ===== 2. send
	const COsQueue* pMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(processType, eManager);

	STATUS res = pMbx->Send(pRetParam, SYSTEM_RAM_SIZE_IND, &GetRcvMbx());
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnInstallerKeycodeUpdateInd(CSegment* pMsg)
{
	if(!pMsg)
	{
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnInstallerKeycodeUpdateInd"
									  << "\nInput segment is NULL";
		return;
	}

	char keycode[KEYCODE_LENGTH];
	*pMsg >> keycode;
	keycode[EXACT_KEYCODE_LENGTH_IN_GL1] = '\0';

	FTRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnInstallerKeycodeUpdateInd"
								   << "\nThe updated keycode is "<< keycode;

	if (YES == m_isAuthenticationIndReceived)
	{
		 m_authentication.Set_X_KeyCode(keycode);
		 ContinueStartup_KeycodeValidated();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendNewSysCfgParamsToCards(const std::string& strDebugModedata,
												 const std::string& strJITCdata,
												 const std::string& strSeparatedNetworksdata,
												 const std::string& strMultipleServicesdata) const
{
    BOOL bIsJitcMode = FALSE;
    BOOL bIsDebugMode = FALSE;
    BOOL bIsSeparatedNetworks = FALSE;
    BOOL bIsMultipleServices = FALSE;

    if (CFG_STR_YES == strJITCdata)
        bIsJitcMode = TRUE;

    if (CFG_STR_YES == strDebugModedata)
        bIsDebugMode = TRUE;

    if (CFG_STR_YES == strSeparatedNetworksdata)
        bIsSeparatedNetworks = TRUE;

    if (CFG_STR_YES == strMultipleServicesdata)
        bIsMultipleServices = TRUE;

	CSegment* pMsg = new CSegment;
	*pMsg << bIsDebugMode
          << bIsJitcMode
          << bIsSeparatedNetworks
          << bIsMultipleServices;

	CManagerApi api(eProcessCards);
	api.SendMsg(pMsg, CARDS_NEW_SYS_CFG_PARAMS_IND);
}

/////////////////////////////////////////////////////////////////////////////
// to be removed!!
void CMcuMngrManager::OnTemporaryTimer()
{
	CManagerApi api(eProcessCSMngr);
    api.SendOpcodeMsg(MCUMNGR_TO_CSMNGR_END_IP_CONFIG_IND);
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::ConfigEthernetCpuAndSwitch()
{
	CEthernetSettingsConfigList	*pEthList		= m_pProcess->GetEthernetSettingsConfigList();
	CEthernetSettingsConfig		*pCurEthConfig = NULL;
	int curSlotId = 0,
	    curPortId = 0;


	for (int i=0; i<MAX_NUM_OF_LAN_PORTS; i++)
	{
		pCurEthConfig	= pEthList->GetSpecEthernetSettingsConfig(i);
		if(pCurEthConfig)
		{
			curSlotId		= pCurEthConfig->GetSlotId();
			curPortId		= pCurEthConfig->GetPortId();


			TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::ConfigEthernetCpuAndSwitch - added fo debug"
					<< "\ncurSlotId:    " << (int)curSlotId
					<< "\ncurPortId:     " <<(int) curPortId
					<< "\ni = :   " << (int)i;


			switch (pCurEthConfig->GetPortType())
			{
			  case eEthPortType_Management1:
			  case eEthPortType_Management2:
			  case eEthPortType_Signaling1:
			  case eEthPortType_Signaling2:
			  {
				  ConfigureEthernetSettingsCpu(pCurEthConfig);
				  break;
			  }

			  case eEthPortType_ManagementShelfMngr:
			  case eEthPortType_Modem:
			  case eEthPortType_ManagementShelfMngr_Managment_Signaling_media: //for RMX2000
			  case eEthPortType_ManagementShelfMngr_Managment_Signaling :      //for RMX2000
			  {
				  SendConfigEthernetSettingsReqToMplApi(pCurEthConfig);
				  break;
			  }
			  case eEthPortType_Illegal:
				  continue; // its the default
			  default:
				  break;

			}
		}
		else
			PASSERT(1);

	} // end loop

}


CEthernetSettingsConfig	* CMcuMngrManager::GetEthernetSettings_specEntity(const DWORD slotId, const DWORD portId)
{
	CEthernetSettingsConfigList	*pEthList		= m_pProcess->GetEthernetSettingsConfigList();
	CEthernetSettingsConfig		*pCurEthConfig	= NULL;
	DWORD curSlotId = 0,
    	  curPortId = 0;

	for (int i=0; i<MAX_NUM_OF_LAN_PORTS; i++)
	{
		pCurEthConfig	= pEthList->GetSpecEthernetSettingsConfig(i);
		if(pCurEthConfig)
		{
			curSlotId		= pCurEthConfig->GetSlotId();
			curPortId		= pCurEthConfig->GetPortId();





			if (slotId == curSlotId && portId == curPortId)
			{
				TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::GetEthernetSettings_specEntity - added fo 802.1x"
								<< "\ncurSlotId:    " << (int)curSlotId
								<< "\ncurPortId:     " <<(int) curPortId
								<< "\ni = :   " << (int)i;
				return pCurEthConfig;

			}
		}
	} // end loop
	return NULL;
}

void CMcuMngrManager::ConfigEthernet_specEntity(const DWORD slotId, const DWORD portId)
{
	CEthernetSettingsConfigList	*pEthList		= m_pProcess->GetEthernetSettingsConfigList();
	CEthernetSettingsConfig		*pCurEthConfig	= NULL;
	DWORD curSlotId = 0,
    	  curPortId = 0;

	for (int i=0; i<MAX_NUM_OF_LAN_PORTS; i++)
	{
		pCurEthConfig	= pEthList->GetSpecEthernetSettingsConfig(i);
		if(pCurEthConfig)
		{
			curSlotId		= pCurEthConfig->GetSlotId();
			curPortId		= pCurEthConfig->GetPortId();



			TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::ConfigEthernet_specEntity - added fo debug"
					<< "\ncurSlotId:    " << (int)curSlotId
					<< "\ncurPortId:     " <<(int) curPortId
					<< "\ni = :   " << (int)i;

			if (slotId == curSlotId && portId == curPortId)
			{
				ConfigureEthernetSettingsSpec(pCurEthConfig);
				break;
			}
		}
	} // end loop
}

bool CMcuMngrManager::IsCpuEthPort(const eEthPortType portType)
{
	bool isCpuPort = false;

	if ( (eEthPortType_Management1	== portType) ||
		 (eEthPortType_Management2	== portType) ||
		 (eEthPortType_Signaling1	== portType) ||
		 (eEthPortType_Signaling2	== portType))

	{
		isCpuPort = true;
	}

	if (eProductTypeGesher == m_mcuMngrProductType || eProductTypeNinja == m_mcuMngrProductType)
	{
		isCpuPort = true;
	}

	return isCpuPort;
}

STATUS CMcuMngrManager::ConfigureEthernetSettingsSpec(CEthernetSettingsConfig *pEthSettingsConfig)
{
	STATUS retStatus = STATUS_OK;

	// CPU port - configured by Mcms
	if ( true == IsCpuEthPort(pEthSettingsConfig->GetPortType()) )
	{
		retStatus = ConfigureEthernetSettingsCpu(pEthSettingsConfig);
	}

	// other ports - configured by CM
	else
	{
		SendConfigEthernetSettingsReqToMplApi(pEthSettingsConfig);

	}

	return retStatus;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::ConfigureEthernetSettingsCpu(CEthernetSettingsConfig *pEthSettingsConfig)
{
	STATUS retStatus = STATUS_OK;

	int curPortId			= pEthSettingsConfig->GetPortId();
	ePortSpeedType curSpeed	= pEthSettingsConfig->GetPortSpeed();


	eConfigInterfaceType ifType		= ConvertEthPortToConfigInterfaceType(curPortId,m_mcuMngrProductType);

	retStatus = ConfigureEthernetSettingsCpu(ifType, curSpeed);

    TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::ConfigureEthernetSettings - configuration request was sent to Configurator"
                           << "\nBoardId:    " << pEthSettingsConfig->GetSlotId()
                           << "\nPortId:     " << curPortId
                           << "\nPortType:   " << GetEthPortTypeStr(pEthSettingsConfig->GetPortType())
                           << "\nSpeed:      " << ::PortSpeedTypeToString(curSpeed)
                           << "\n------------"
                           << "\nIfType:     " << GetConfigInterfaceTypeName(ifType)
                           << "\nStatus::    " << m_pProcess->GetStatusAsString(retStatus);

    return retStatus;
}

/////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::ConfigureEthernetSettingsCpu(const eConfigInterfaceType ifType, const ePortSpeedType portSpeed)
{
    STATUS cfgStatus = STATUS_OK;
    eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

    if (IsTarget() || eProductTypeGesher == curProductType || eProductTypeNinja == m_mcuMngrProductType)
    {
        //CConfigManagerApi api;
        //cfgStatus = api.ConfigureEthernetSettings(ifType, portSpeed);


        //VNGR-24088   Sends asynchronous request to configurator
        CSegment* seg = new CSegment;
        *seg << (DWORD) ifType
        << (DWORD) portSpeed;

        CTaskApi::SendMsgWithTrace(eProcessConfigurator,
                eManager,
                seg,
                CONFIGURATOR_SET_ETH_SETTINGS);

    }

    TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::ConfigureEthernetSettings"
    << "\nInterface num: " << GetDeviceName(ifType)
    << "\nSpeed:         " << ::PortSpeedTypeToString(portSpeed);
    //<< "\nStatus:        " << m_pProcess->GetStatusAsString(cfgStatus).c_str();


    return cfgStatus;
}



/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendConfigEthernetSettingsReqToMplApi(CEthernetSettingsConfig *pEthSettingsConfig)
{
	DWORD subboardId ;
	DWORD displayBoardId	= pEthSettingsConfig->GetSlotId();

	if ((displayBoardId == FIXED_BOARD_ID_RTM_1) || (displayBoardId == FIXED_BOARD_ID_RTM_2 ) ||(displayBoardId == FIXED_BOARD_ID_RTM_3) ||
			(displayBoardId == FIXED_BOARD_ID_RTM_4))

		subboardId = RTM_ISDN_SUBBOARD_ID;
	else
		subboardId = FIXED_CM_SUBBOARD_ID;


	DWORD physicalBoardId	= m_pSlotsNumConversionTable->GetBoardId(displayBoardId, subboardId);

	ETH_SETTINGS_CONFIG_S *pConfigStruct = new ETH_SETTINGS_CONFIG_S;
	memset(pConfigStruct, 0, sizeof(ETH_SETTINGS_CONFIG_S));
	pConfigStruct->portParams.slotId	= (APIU32)( physicalBoardId );
	pConfigStruct->portParams.portNum	= (APIU32)( pEthSettingsConfig->GetPortId() );
	pConfigStruct->portSpeed			= (APIU32)( pEthSettingsConfig->GetPortSpeed() );

	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendConfigEthernetSettingsReqToMplApi - params sent to CM:"
	                       << "\nslotId:    " << (int)(pConfigStruct->portParams.slotId)
	                       << "\nDisplayBoardId: " << displayBoardId
	                       << "\nportNum:   " << (int)(pConfigStruct->portParams.portNum)
	                       << "\nportSpeed: " << (int)(pConfigStruct->portSpeed)
	                       			<< "(" << ::PortSpeedTypeToString((ePortSpeedType)(pConfigStruct->portSpeed)) << ")";

	CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol();

	mplPrtcl->AddCommonHeader(ETHERNET_SETTINGS_CONFIG_REQ);
	mplPrtcl->AddMessageDescriptionHeader();
	mplPrtcl->AddPhysicalHeader(1, physicalBoardId, FIXED_CM_SUBBOARD_ID);
	mplPrtcl->AddData(sizeof(ETH_SETTINGS_CONFIG_S), (char*)pConfigStruct);

	CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("MCUMNGR_SENDS_TO_MPL_API");
	mplPrtcl->SendMsgToMplApiCommandDispatcher();

	POBJDELETE(mplPrtcl);
	PDELETE(pConfigStruct);
}



/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnEthSettingConfigInd(CSegment* pSeg)
{
	// ===== 1. extract the mplMcmsProtocol from segment received
	CMplMcmsProtocol* pMplMcmsProtocol = new CMplMcmsProtocol;
	pMplMcmsProtocol->DeSerialize(*pSeg);

	// validate data size
	STATUS sizeStat = pMplMcmsProtocol->ValidateDataSize( sizeof(ETH_SETTINGS_STATE_S) );
	if (STATUS_OK != sizeStat)
    {

        POBJDELETE(pMplMcmsProtocol);
		return;
    }


	// ===== 2. get the ethConfigState data into a ETH_SETTINGS_STATE_S struct
	ETH_SETTINGS_STATE_S ethState;
	memset( &ethState, 0, sizeof(ETH_SETTINGS_STATE_S));
	memcpy( &ethState, pMplMcmsProtocol->GetData(), sizeof(ETH_SETTINGS_STATE_S) );


	// ===== 3. print content
	eEthSettingsState curState = (eEthSettingsState)(ethState.configState);

	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnEthSettingConfigInd - params received from CM:"
								  << "\nslotId:      " << (int)(ethState.portParams.slotId)
								  << "\nportNum:     " << (int)(ethState.portParams.portNum)
								  << "\nconfigState: " << (eEthSettingsState_ok == curState ? "ok" : "fail");
    POBJDELETE(pMplMcmsProtocol);


}


/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnSlotsNumberingConversionTableInd(CSegment* pMsg)
{
    SLOTS_NUMBERING_CONVERSION_TABLE_S* pSlotsNumTable = new SLOTS_NUMBERING_CONVERSION_TABLE_S;
	pMsg->Get((BYTE*)pSlotsNumTable, sizeof(SLOTS_NUMBERING_CONVERSION_TABLE_S));

	m_pSlotsNumConversionTable->SetStruct(pSlotsNumTable);
	m_pSlotsNumConversionTable->PrintData("CMcuMngrManager::OnSlotsNumberingConversionTableInd");
	delete pSlotsNumTable;
}

/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnUpdateSystemStartupDurationInd(CSegment* pMsg)
{
	DWORD newStartupTimeout;
	*pMsg >> newStartupTimeout;

	if ( eTaskStateStartup == GetTaskState() )
	{
		DeleteTimer(MCUMNGR_STARTUP_TIMER);
		StartTimer(MCUMNGR_STARTUP_TIMER, newStartupTimeout);

		m_pProcess->GetMcuStateObject()->SetRemainingTimeForStartup(newStartupTimeout);

		int newStartupTimeoutInSeconds = newStartupTimeout / SECOND;	// 'newStartupTimeout' is in ticks
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnUpdateSystemStartupDurationInd"
	    							  << "\nStartup duration was updated to "
	    							  << newStartupTimeout << " ticks (" << newStartupTimeoutInSeconds << " seconds)";
	}
}

//////////////////////////////////////////////////////////////////////////////
void	CMcuMngrManager::OnFailoverSlaveBecomeMasterInd(CSegment *pSeg)
{
	CIPSpan* pFirstSpanFromUpdatedService = m_pMngmntIpParams_asIpService_fromProcess->GetFirstSpan();
	HostName newHostName = GetHostNameFromService(m_pMngmntIpParams_asIpService_fromProcess).c_str();
	newHostName.RemoveChars("-bck");
	pFirstSpanFromUpdatedService->SetSpanHostName(newHostName);
	PerformUpdateSyncedMngmntProcedures(m_pMngmntIpParams_asIpService_fromProcess);
	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::OnFailoverSlaveBecomeMasterInd - newhostName = " << newHostName;

}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnFailoverConfigReq(CSegment* pSeg)
{
	BYTE bIsSecured = m_pMngmntIpParams_asIpService_fromProcess->GetIsSecured();

    CSegment *pMsg = new CSegment;
    *pMsg << bIsSecured;

    DWORD defaultGwIp = m_pMngmntIpParams_asIpService_fromProcess->GetDefaultGatewayIPv4();
	*pMsg << defaultGwIp;

	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::OnFailoverConfigReq - bIsSecured = " << (bIsSecured ? "yes" : "no");

	CManagerApi api(eProcessFailover);
	STATUS status = api.SendMsg(pMsg, FAILOVER_MCUMNGR_CONFIG_IND);
	if (status != STATUS_OK)
		FPASSERT(FAILOVER_MCUMNGR_CONFIG_IND);
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnFailoverUpdateMngmntInd(CSegment* pSeg)
{
    DWORD len;
    *pSeg >> len;

    char* pServiceElementStr = new char[len+1];
    pServiceElementStr[len] = '\0';
    *pSeg >> pServiceElementStr;

    FTRACESTR(eLevelInfoHigh) <<  "\nCMcuMngrManager::OnFailoverUpdateMngmntInd - the string:\n" << pServiceElementStr;

    CXMLDOMDocument *pDom = new CXMLDOMDocument;
    if (pDom->Parse((const char **)&pServiceElementStr) == SEC_OK)
	{
		CXMLDOMElement* pRoot = pDom->GetRootElement();
		if(pRoot)
		{
			char szErrorMsg[ERROR_MESSAGE_LEN];
			CIPService *pServiceFromFailoverTask = new CIPService;
			pServiceFromFailoverTask->DeSerializeXml(pRoot, szErrorMsg, "");

			// print the struct received (for debugging)
			CIpParameters ipParamsFromFailoverTask;
			pServiceFromFailoverTask->ConvertToIpParamsStruct( *(ipParamsFromFailoverTask.GetIpParamsStruct()) );
			TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnFailoverUpdateMngmntInd - the struct:\n" << ipParamsFromFailoverTask;

			CIPService *pUpdatedService = new CIPService(*m_pMngmntIpParams_asIpService_fromProcess);
			UpdateSyncedFields(pServiceFromFailoverTask, pUpdatedService);
			PerformUpdateSyncedMngmntProcedures(pUpdatedService);

			POBJDELETE(pServiceFromFailoverTask);
			POBJDELETE(pUpdatedService);
		}	// end if (pRoot)
	}	// end if (Parse==SEC_OK)

    PDELETEA(pServiceElementStr);
    POBJDELETE(pDom);
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnFailoverUpdatePairIPInd(CSegment* pSeg)
{
    std::string ip;
	*pSeg >> ip;

	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnFailoverUpdatePairIPInd - Pair IP = "<< ip.c_str();

	m_szPairIPFailover = ip;
}

/* Receive the Event Trigger Indication from Failover.  Some hotbackup trigger deactivated*/
void CMcuMngrManager::OnFailoverEventTriggerInd(CSegment * pSeg)
{
	m_bTriggerFailover = TRUE;
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::UpdateSyncedFields(CIPService *pServiceFromFailoverTask, CIPService *pUpdatedService)
{
	// ===== 1. DNS
	// --- 1a. MCU HostName

	CIPSpan* pFirstSpanFromUpdatedService = pUpdatedService->GetFirstSpan();

	//printf("protcol type %d\n",pServiceFromFailoverTask->GetIPProtocolType());


	HostName newHostName =GetHostNameFromService(pServiceFromFailoverTask).c_str();
	if (newHostName.Find("-bck") == 0)
		newHostName.ForceAppendStr("-bck");
	pFirstSpanFromUpdatedService->SetSpanHostName(newHostName);

	// --- 1b. other DNS params
	CIpDns* pDnsFromFailoverTask = pServiceFromFailoverTask->GetpDns();
	pUpdatedService->SetDns(*pDnsFromFailoverTask);

	//--- 1c Protocol Type
	pUpdatedService->SetIPProtocolType(pServiceFromFailoverTask->GetIPProtocolType());
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::PerformUpdateSyncedMngmntProcedures(CIPService *pUpdatedService)
{
	// since only DNS is synced, there is no need to perform all procedures
	//      that are done in <SetUpdatedMngmntNetworkParams> or in <ConfigNetworkInterfaceInOS>
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::PerformUpdateSyncedMngmntProcedures - Start Dns configuration";
	STATUS dnsStatus = StartDnsConfiguration(pUpdatedService, eMngmntIpFromEma);

	// notify other entities
    SendMNGMNTServiceToGK();
//    SendMNGMNTInfoToSnmp(SNMP_MNGMNT_IP_CONFIG_IND);
//    SendMNGMNTInfoToQA_Api(CQAAPI_MNGMNT_INTERFACE_IP_IND);
    SendMNGMNTInfoToCertMngr();
}
/////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::HandleNetworkSeparationConfigurations(CIPService Service)
{
	STATUS retStatus = STATUS_FAIL;

	if((eProductTypeRMX4000 == m_mcuMngrProductType) || (eProductTypeRMX1500 == m_mcuMngrProductType) ||
		(eProductTypeGesher == m_mcuMngrProductType) || (eProductTypeNinja == m_mcuMngrProductType) ||
		((eProductTypeRMX2000 == m_mcuMngrProductType) && IsJitcAndNetSeparation()))
	{
		eConfigInterfaceType ifType = GetMngrIfType();

		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::HandleNetworkSeparationConfigurations ifType = :" << ifType;
		CConfigManagerApi api;
		char ipStr[IP_ADDRESS_LEN]="";
		char defaultGWIPv4Str[IP_ADDRESS_LEN]="";
		char defaultGWIPv6Str[IPV6_ADDRESS_LEN] = "";

		CIPSpan* pTmpSpan = Service.GetSpanByIdx(0); // Mngmnt params are stored in the 1st span (idx==0)
		if(pTmpSpan)
		{
			DWORD ipAddress   = pTmpSpan->GetIPv4Address();
			SystemDWORDToIpString(ipAddress,ipStr);
			char ipv6SubNetMask[IPV6_ADDRESS_LEN];
			pTmpSpan->GetIPv6SubnetMaskStr(0, ipv6SubNetMask);

			DWORD dwDefaultGw = Service.GetDefaultGatewayIPv4();
			SystemDWORDToIpString(dwDefaultGw,defaultGWIPv4Str);

			DWORD mask = Service.GetNetMask();

			std::list<CRouter> routerList;
			RetrieveRouterList(&Service, routerList);

			eIpType ipType			= m_pMngmntIpParams_asIpService_fromProcess->GetIpType();
			//Cleanup
			retStatus = api.DelRouteTableRule(ifType,ipType, ipStr, mask, defaultGWIPv4Str);
			//Add

			retStatus = api.AddRouteTableRule(ifType,ipType,ipStr, mask, defaultGWIPv4Str,ipv6SubNetMask, routerList);
		}
		else
			PASSERTMSG(1, "CMcuMngrManager::HandleNetworkSeparationConfigurations, span not found.");

	}
	return retStatus;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::SendSyncNumOfConfsMsgToConfPartyProcess()
{
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendSyncNumOfConfsMsgToConfPartyProcess";

	OPCODE outOpcode = GET_CONF_NUM_REQ,
		   rspOpcode = 0;
    CSegment rspMsg;

    CManagerApi confPartyMngrApi(eProcessConfParty);
    STATUS respStatus = confPartyMngrApi.SendMessageSync(NULL, outOpcode, (int)(2*SECOND), rspOpcode, rspMsg);

    if(STATUS_OK == respStatus)
    {
		// verify response message
		if(0 < rspMsg.GetLen())
		{
	        DWORD confNum = 0;
	        rspMsg >> confNum;
	        if(0 < confNum)
	        {
	        	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendSyncNumOfConfsMsgToConfPartyProcess"
	        						   << "\nConfParty process returned that there are " << confNum << " running conferences;"
	        						   << "\nRMX Time updating is blocked while there are running conferences";

	        	respStatus = OPERATION_BLOCKED;
	        }
	        else
	        {
	        	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendSyncNumOfConfsMsgToConfPartyProcess"
	        						   << "\nConfParty process returned that there are " << confNum << " running conferences;"
	        						   << "\nRMX Time updating is accepted since there are no running conferences";

	        	respStatus = STATUS_OK;
	        }
		}

		else // rspMsg.GetLen() == 0
		{
			TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendSyncNumOfConfsMsgToConfPartyProcess - ConfParty returned error msg";
			respStatus = STATUS_ILLEGAL;
		}
    } // end respStatus==ok

	else  // timeout or dead process
	{
		PASSERTMSG(respStatus, "\nCMcuMngrManager::SendSyncNumOfConfsMsgToConfPartyProcess - CONF RESPONSE TIMED OUT !!! ");
	}

	return respStatus;
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendSecurityMode(eProcessType process) const
{
	CSegment* pSeg = new CSegment();

	BYTE bSequrity = m_pMngmntIpParams_asIpService_fromProcess->GetIsSecured();

	*pSeg << bSequrity;

	CManagementSecurity* sec = m_pMngmntIpParams_asIpService_fromProcess->GetManagementSecurity();
	PASSERT_AND_RETURN(NULL == sec);
	BYTE isRequestPeerCertificate = sec->IsRequestPeerCertificate();

	*pSeg << isRequestPeerCertificate;
    CManagerApi mngrApi(process);
    STATUS s = mngrApi.SendMsg(pSeg, MCUMNGR_SECURITY_MODE_IND);

    if( s != STATUS_OK)
    	PASSERT(MCUMNGR_SECURITY_MODE_IND);
    TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendSecurityMode"
	                       << "\nSecurity mode: " << (YES == bSequrity ? "YES" : "NO")
	                       << " RequestPeerCertificate " << (int)isRequestPeerCertificate;
}

/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendPrecedenceSettingsToProcess(CPrecedenceSettings* pPrecedence, eProcessType processType) {
	TRACESTR(eLevelInfoNormal)	<< "\nCMcuMngrManager::SendPrecedenceSettingsToProcess: " << processType;
	//	printf("CMcuMngrManager::SendPrecedenceSettingsToProcess\n");

	CSegment* pSeg = new CSegment();
	pPrecedence->Serialize(NATIVE, *pSeg);

	CManagerApi api(processType);
	api.SendMsg(pSeg, MCUMNGR_PRECEDENCE_SETTINGS);

}


//Send QOS_MANAGEMENT_NETWORK (string)value to process
void CMcuMngrManager::SendQosManagementDSCPToProcess(std::string signalingDCSP, eProcessType processType)
{
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendQosManagementDSCPToProcess";
//	printf("CMcuMngrManager::SendPrecedenceSettingsToProcess\n");

	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	if (YES == IsTarget() || eProductTypeGesher == curProductType || eProductTypeNinja == curProductType)
	{
		CSegment* pSeg = new CSegment();
		*pSeg << signalingDCSP;

		CManagerApi api(processType);
		api.SendMsg(pSeg, CONFIGURATOR_QOS_MANAGEMENT_DSCP);
	}

}
/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendV35GwParamsReqToCsMngr()
{
	TRACESTR(eLevelInfoNormal) << "\nCCMcuMngrManager::SendV35GwParamsReqToCsMngr";

    CManagerApi api(eProcessCSMngr);
    STATUS res = api.SendOpcodeMsg(MCUMNGR_CSMNGR_V35GW_PARAMS_REQ);
}
//////////////////////////////////////////////////////////////////////
void   CMcuMngrManager::SendSecurityPKIToDependedProcesses()
{
	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::SendSecurityPKIToDependedProcesses";
	CSegment* pSeg = new CSegment();
	CManagementSecurity* pPkiCfg = m_pMngmntIpParams_asIpService_fromProcess->GetManagementSecurity();
	pPkiCfg->Serialize(0,*pSeg);
	CManagerApi api(eProcessLdapModule);
	STATUS s = api.SendMsg(pSeg,SECURITY_PKI_CFG);

    if( s != STATUS_OK)
	   PASSERT(SECURITY_PKI_CFG);
}


/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendMcuStateToNotificationMngr()
{
	if( !m_bNotificationMngrReady )
		return;

	TRACESTR(eLevelInfoNormal) << "\nCCMcuMngrManager::SendMcuStateToNotificationMngr";

	// get MCU state
	CMcuState* pMcuState = m_pProcess->GetMcuStateObject();

	// serialize it to XML string
	CXMLDOMElement* pXMLRootElement =  new CXMLDOMElement;
	//pXMLRootElement->set_nodeName("GET_STATE");

	pMcuState->SerializeXml(pXMLRootElement);
	CXMLDOMNode* pMcuStateNode = NULL;
	pXMLRootElement->get_firstChild(&pMcuStateNode);

	std::stringstream sstream;
	pMcuStateNode->DumpDataAsStringImpl(sstream,0,FALSE);
	//std::cout << "MCU state:" << std::endl << sstream.str() << std::endl;
	PDELETE(pXMLRootElement);

	// put data to segment
	CSegment* pSegment = new CSegment;
	*pSegment << sstream.str();

	// send message with MCU state in XML
	CManagerApi api(eProcessNotificationMngr);
	STATUS res = api.SendMsg(pSegment,MCUMNGR_NOTIFMNGR_MCU_STATE_IND);
}

/////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendAAListToNotificationMngr()
{
	if( !m_bNotificationMngrReady )
		return;

	TRACESTR(eLevelInfoNormal) << "\nCCMcuMngrManager::SendAAListToNotificationMngr";

	CFaultList* pActiveAlarmsList = m_pProcess->GetActiveAlarmsList();
	// serialize it to XML string
	CXMLDOMElement* pXMLRootElement =  new CXMLDOMElement;
	pActiveAlarmsList->SerializeXml(pXMLRootElement, -1, 0);

	CXMLDOMNode* pAAListNode = NULL;
	pXMLRootElement->get_firstChild(&pAAListNode);

	std::stringstream sstream;
	pAAListNode->DumpDataAsStringImpl(sstream,0,FALSE);
	//std::cout << "AA list:" << std::endl << sstream.str() << std::endl;
	PDELETE(pXMLRootElement);

	// put data to segment
	CSegment* pSegment = new CSegment;
	*pSegment << sstream.str();

	// send message with MCU state in XML
	CManagerApi api(eProcessNotificationMngr);
	STATUS res = api.SendMsg(pSegment,MCUMNGR_NOTIFMNGR_AA_LIST_IND);
}

/////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::OnKillSshd(CSegment* pSeg)
{
	if (eProductFamilyRMX==CProcessBase::GetProcess()->GetProductFamily() &&   !IsTarget()) //in rmx Simulation do not kill ssshd as it is very stupied to loose connection to your vm
	{
		PTRACE(eLevelInfoNormal, "CMcuMngrManager::OnKillSshd , ssh will not be killed in simulation even if in Jitc Mode");
		return STATUS_OK;
	}
	PTRACE(eLevelInfoNormal, "CMcuMngrManager::OnKillSshd ");
	CConfigManagerApi configApi;
	configApi.KillSshd();
	StartTimer(KILL_SSHD_CYCLIC, 10*SECOND);
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////

STATUS CMcuMngrManager::CheckUltraSecuredFlagAndSetOtherFlagsIfNeeded(
		CSysConfigEma *pSysConfigSet) {
	CProcessBase *pProcess = CProcessBase::GetProcess();
	CSysConfigBase *realSysConfig = pProcess->GetSysConfig();

	CCfgData *realParam = realSysConfig->GetCfgEntryByKey(CFG_KEY_JITC_MODE);
	bool isParamExist = pSysConfigSet->IsParamExist(CFG_KEY_JITC_MODE);
	if (realParam->GetData() == "YES") {
		if (isParamExist == FALSE) {
			return STATUS_CFG_PARAM_CANT_REMOVE_ULTRA_SECURED_MODE;
		}
	} else { // Currently Jitec is off
		if (isParamExist) {
			CCfgData * emaJitecParam = pSysConfigSet->GetCfgEntryByKey(
					CFG_KEY_JITC_MODE);
			if (emaJitecParam->GetData() == "YES") // Jitec was changed from no to yes
			{
				// change LAST_LOGIN_ATTEMPTS parameter
				bool isLastLoginParamExist = pSysConfigSet->IsParamExist(
						CFG_KEY_LAST_LOGIN_ATTEMPTS);
				if (isLastLoginParamExist) {
					TRACESTR(eLevelInfoNormal)
							<< "\nChanging LAST_LOGIN_ATTEMPTS to YES because ULTRA_SECURED_MODE is on";

					CCfgData * emaLastLoginParam =
							pSysConfigSet->GetCfgEntryByKey(
									CFG_KEY_LAST_LOGIN_ATTEMPTS);
					emaLastLoginParam->SetData("YES");
				} else {
					TRACESTR(eLevelInfoNormal)
							<< "\nFailed to get LAST_LOGIN_ATTEMPTS paramter from Sys config";
				}

				if (pSysConfigSet->IsParamExist("SSH") == TRUE)
				{
				    CCfgData * emaSSHParam = pSysConfigSet->GetCfgEntryByKey("SSH");
				    emaSSHParam->SetData("NO");
				    TRACESTR(eLevelInfoNormal)<< "\nChanging SSH to NO because ULTRA_SECURED_MODE is on";
				}

				//change USER_LOCK_OUT
				bool isUserLockOutExistFromEma = pSysConfigSet->IsParamExist(CFG_KEY_USER_LOCKOUT);
				if (isUserLockOutExistFromEma){
					TRACESTR(eLevelInfoNormal)
							<< "\nChanging USER_LOCK_OUT to Yes because ULTRA_SECURED_MODE is on";

					CCfgData* userLockOutParamFromEma = pSysConfigSet->GetCfgEntryByKey(CFG_KEY_USER_LOCKOUT);
					userLockOutParamFromEma->SetData("YES");
				}else {
					TRACESTR(eLevelInfoNormal)<< "Failed to get CFG_KEY_USER_LOCKOUT from EMA SysConfig";
				}
			}
		}
	}
	return STATUS_OK;
}

STATUS CMcuMngrManager::StopCheckEth0Link()
{
	 CConfigManagerApi configuratorApi;
	 STATUS respStatus = configuratorApi.StopCheckEth0Link();

     return respStatus;
}

STATUS CMcuMngrManager::StartCheckEth0Link()
{
	 CConfigManagerApi configuratorApi;
	 STATUS respStatus = configuratorApi.StartCheckEth0Link();

     return respStatus;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::OnCsIpServiceParamsInd(CSegment* pMsg)
{
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnCsIpServiceParamsInd";

	const IP_SERVICE_MCUMNGR_S *pParam = (IP_SERVICE_MCUMNGR_S*)pMsg->GetPtr();

	//Fix to VNGR-21525 - case that Service ID is 0 (ID begins from 1)
	if (pParam->ServId < 1 ||
		pParam->ServId > MAX_NUMBER_OF_SERVICES_IN_RMX_4000)
	{
		TRACESTR(eLevelInfoNormal) << "Illegal service ID " << pParam->ServId;
		return STATUS_OK;
	}

	int pos = pParam->ServId - 1;




	strcpy(m_StaticIpServiceList[pos].DefaultGatewayIPv6, pParam->DefaultGatewayIPv6);
	m_StaticIpServiceList[pos].DefaultGatewayMaskIPv6 = pParam->DefaultGatewayMaskIPv6;

	m_StaticIpServiceList[pos].numPQSactual = pParam->numPQSactual;
	if (pParam->numPQSactual != 0)
	  memcpy(m_StaticIpServiceList[pos].IPServUDPperPQList, pParam->IPServUDPperPQList, MAX_NUM_PQS*sizeof(IP_SERVICE_UDP_MCUMNGR_PER_PQ_S));


	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnCsIpServiceParamsInd - pParam->dnsStatus=" << pParam->dnsStatus;
	if(pParam->dnsStatus == eServerStatusSpecify)
	{
		m_numOfDnsEnable ++;
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnCsIpServiceParamsInd - dnsStatus == eServerStatusSpecify, m_numOfDnsEnable=" << m_numOfDnsEnable;
	}

	//start dns only if it didn't started already
	if(m_numOfDnsEnable == 1 && eProductTypeEdgeAxis != m_mcuMngrProductType)
	{
		TRACESTR(eLevelInfoNormal) << "\nMcuMngrManager::OnCsIpServiceParamsInd m_numOfDnsEnable == 1";
		//creating dummy IPService for starting DNS
		CIPService otherService;
		CIpDns newDns ;
		otherService.SetDns(newDns);
		otherService.SetIpType(eIpType_IpV4);
		CIPSpan newSpan;
		newSpan.SetSpanHostName("dumyhost");
		otherService.AddSpan(newSpan);
		otherService.SetIpServiceType(eIpServiceType_Signaling);
		//Start DNS services
		ConfigDnsSpecify(&otherService);

	}


	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////
BOOL CMcuMngrManager::CheckIfDefaultGWIPV6IsValid(CIPService *pUpdatedIpService)
{
	char updatedDefGW[IPV6_ADDRESS_LEN];
	pUpdatedIpService->GetDefaultGatewayIPv6(updatedDefGW);

	// 1. check if the default ipv6 GW is empty
	if (pUpdatedIpService->GetDefaultGatewayMaskIPv6()==64 && strcmp(updatedDefGW, "::")==0)
		return TRUE;

	// 2. check if the ipv6 router is different in one of the other services
	for (int i=0; i<MAX_NUMBER_OF_SERVICES_IN_RMX_4000; i++)
	{
		//default value - move to the next service
		if (m_StaticIpServiceList[i].DefaultGatewayMaskIPv6==0 && strcmp(m_StaticIpServiceList[i].DefaultGatewayIPv6, "") ==0)
			continue;

		//check if the default ipv6 router is different than the one defined in other services
		if ( m_StaticIpServiceList[i].DefaultGatewayMaskIPv6!=0 && m_StaticIpServiceList[i].DefaultGatewayMaskIPv6!=64 && m_StaticIpServiceList[i].DefaultGatewayMaskIPv6 != pUpdatedIpService->GetDefaultGatewayMaskIPv6() )
		{
			TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::CheckIfDefaultGWIPV6IsValid - default gw mask in ipv6 can't be different between the services. From service ID = "<<m_StaticIpServiceList[i].ServId<<" it is "<<m_StaticIpServiceList[i].DefaultGatewayMaskIPv6;
			return FALSE;
		}

		if (strcmp(m_StaticIpServiceList[i].DefaultGatewayIPv6, "::") !=0  &&  (strcmp(m_StaticIpServiceList[i].DefaultGatewayIPv6, updatedDefGW) != 0))
		{
			TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::CheckIfDefaultGWIPV6IsValid - default gw ipv6 can't be different between the services. in this update service it is: "<<updatedDefGW<<" while in service Id: "<<m_StaticIpServiceList[i].ServId<<" it is "<<m_StaticIpServiceList[i].DefaultGatewayIPv6;
			return FALSE;
		}
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendIpServiceParamsReqToCS()
{
	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::SendIpServiceParamsReqToCS";

	CSegment*  pRetParam = new CSegment;

	const COsQueue* pCsMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCSMngr, eManager);

	STATUS res = pCsMbx->Send(pRetParam,CSMNGR_MCUMNGR_IP_SERVICE_PARAM_REQ);

	if (res != STATUS_OK)
	{
		TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::SendIpServiceParamsReqToCS - failed to send request to the CSMngr";

//		StartTimer(IPSERVICEFROMCSMNGRTOUT,30*SECOND);
	}
}

/////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::OnCsDeleteIpeServiceParamsInd(CSegment* pSeg)
{
	PTRACE(eLevelInfoNormal, "CMcuMngrManager::OnCsDeleteIpeServiceParamsInd ");

	Del_Ip_Service_S *param = (Del_Ip_Service_S*)pSeg->GetPtr();
	DWORD serviceID = param->service_id;

	if(serviceID > 0 && serviceID <= MAX_NUMBER_OF_SERVICES_IN_RMX_4000) {
		int pos = serviceID-1;
		m_StaticIpServiceList[pos].ServId = 0;
		memset(m_StaticIpServiceList[pos].DefaultGatewayIPv6, '\0',IPV6_ADDRESS_LEN);
		m_StaticIpServiceList[pos].DefaultGatewayMaskIPv6 = 0;

		m_StaticIpServiceList[pos].numPQSactual = 0;
		memset(m_StaticIpServiceList[pos].IPServUDPperPQList, '\0',sizeof(IP_SERVICE_UDP_MCUMNGR_PER_PQ_S)*MAX_NUM_PQS);

	} else {
		PTRACE(eLevelError, "CMcuMngrManager::OnConfDeleteIpeServiceParamsInd - Deleted IP service was not found in Ip service List ");
	}

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
//TODO - add support when EMB nerge their changes from 7.5

void CMcuMngrManager::OnOutOfSecureModeInd(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnOutOfSecureModeInd";

	// ===== check if this is not the first time we got this request, or the user changed the management before this request arrive
	if (m_firstTimeReqForOutOfSecured == TRUE)
	{
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnOutOfSecureModeInd - already removed secure mode after the reset. Nothing to do. ";
		return;
	}

	if (m_isUserChangedManagementIp)
	{
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnOutOfSecureModeInd - User updated management after the reset. Nothing to do. ";
		return;
	}

	if (m_pMngmntIpParams_asIpService_fromProcess->GetIsSecured() == FALSE)
	{
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnOutOfSecureModeInd - system NOT in secure mode. Nothing to do. ";
		return;
	}

	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnOutOfSecureModeInd - remove secure mode";

	m_firstTimeReqForOutOfSecured = TRUE;

	//m_pMngmntIpParams_asIpService_fromProcess->SetIsSecured(FALSE); // Fix BRIDGE-4842
	CIPService ipService(*m_pMngmntIpParams_asIpService_fromProcess);
	ipService.SetIsSecured(FALSE);
	
	m_pProcess->SetMngmntIpParams_asIpService(ipService);
	m_pMngmntIpParams_asIpService_fromProcess = m_pProcess->GetMngmntIpParams_asIpService();

	ipService.WriteXmlFile(MANAGEMENT_NETWORK_CONFIG_PATH, "MngmntNetwork");

	SendMngmntUpdateToMpl();

	ConfigApache(*m_pMngmntIpParams_asIpService_fromProcess);
	// update another processes with new security mode
	SendSecurityMode(eProcessAuthentication);
}

void CMcuMngrManager::SendGmtOffset()
{
	if (IsStartupFinished() == FALSE)
	{
		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendGmtOffset - IsStartupFinished == FALSE";
		StartTimer(GMT_OFFSET_SET_TIMER, 1 * SECOND);
		return;
	}
    else
    {
    	CSegment* pSeg = new CSegment;

    	CSystemTime sysTime;
    	m_pProcess->GetMcuTime(sysTime);
    	BYTE GMTOffset = sysTime.GetGMTOffset();;
    	BYTE GMTOffsetSign = sysTime.GetGMTOffsetSign();
    	*pSeg << GMTOffset;
    	*pSeg << GMTOffsetSign;

    	CManagerApi confPartyMngrApi(eProcessConfParty);
        STATUS stat = confPartyMngrApi.SendMsg(pSeg, MCUMNGR_CONF_GMT_UPDATE);
        TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendGmtOffset"
    	                       << "\nGMTOffset: " << (INT32)GMTOffset
    	                       << "\nGMTOffsetSign: " << (INT32)GMTOffsetSign
    	                       << "\nstat: " << stat;
    }
}

BOOL CMcuMngrManager::IsStartupFinished() const
{
	eMcuState systemState = CProcessBase::GetProcess()->GetSystemState();
	if( eMcuState_Invalid == systemState || eMcuState_Startup == systemState )
		return FALSE;
	return TRUE;
}

BOOL CMcuMngrManager::OnUpdateCpuInfo(CSegment* pMsg) const
{


	string cpu_info;
	*pMsg >> cpu_info;
	m_pLicensing->SetCpuInfo(cpu_info);

	return TRUE;
}

BOOL CMcuMngrManager::OnLoggerMngrCsLogsConfigInd(CSegment* pMsg)
{
    *pMsg >> m_bCSLogStarted;
    TRACESTR(eLevelInfoHigh) << "Value received from LoggerMngr process : " << ( m_bCSLogStarted ? "Enabled" : "Disabled" );
	return TRUE;
}

void CMcuMngrManager::OnNotifMngrMcuStateReq(CSegment* pMsg)
{
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnNotifMngrMcuStateReq";
	m_bNotificationMngrReady = TRUE;
	SendMcuStateToNotificationMngr();
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnNotifMngrAAListReq(CSegment* pMsg)
{
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnNotifMngrAAListReq";
	m_bNotificationMngrReady = TRUE;
	SendAAListToNotificationMngr();
}

void CMcuMngrManager::OnRequestPrecedenceSettings(CSegment* pMsg) {
	//	CPrecedenceSettings* pPrecedence = new CPrecedenceSettings();
	//	pPrecedence->LoadFromFile();

	CMcuMngrProcess* pProcess =	(CMcuMngrProcess*) CMcuMngrProcess::GetProcess();
	CPrecedenceSettings pPrecedenceSettings(*(pProcess->GetPrecedenceSettings()));

	unsigned int ptype;
	*pMsg >> ptype;
	TRACESTR(eLevelInfoNormal)	<< "\nCMcuMngrManager::OnRequestPrecedenceSettings, process: " << static_cast<eProcessType>(ptype);
	SendPrecedenceSettingsToProcess(&pPrecedenceSettings, static_cast<eProcessType>(ptype));
}
/////////////////////////////////////////////////////////////////////

void CMcuMngrManager::OnTimerSnmpReady(CSegment* pSeg)
{

	if (!m_isSNMPup)
	{
		TRACESTR(eLevelInfoNormal)	<< "SNMP is not up";
		m_snmpExtraTimeForStartupLaunch = 0;
		return;
	}
	if (m_isSNMPReady)
	{
		TRACESTR(eLevelWarn) << "SNMP is already ready";
		m_snmpExtraTimeForStartupLaunch = 0;
		return;	
	}
	if (!IsStartupFinished() /*&& m_snmpExtraTimeForStartupLaunch < 3*/)
	{
		TRACESTR(eLevelInfoNormal)	<< "Waiting extra time.  m_snmpExtraTimeForStartupLaunch " << (int)m_snmpExtraTimeForStartupLaunch;

		++m_snmpExtraTimeForStartupLaunch;
		//DeleteTimer(MCUMNGR_SNMP_READY_TIMER);
		StartTimer(MCUMNGR_SNMP_READY_TIMER, DELAY_SNMP_READY_EXTRA_STARTUP);
		return;
	}
	m_snmpExtraTimeForStartupLaunch = 0;
	TRACESTR(eLevelInfoNormal)	<< "Activate SNMP process ..";
	m_isSNMPReady = TRUE;

	std::vector<CLogFltElement> alarms;

	// 1) Gather all alarms into alarmVector


	CStateFaultList* pfaults = m_pProcessesStatesMap->GetFirstStateFaultList();
	while (pfaults)
	{
		CLogFltElement* fault = pfaults->GetFirstFaultElement();
		while (NULL != fault)
		{
			alarms.push_back(*fault);
			fault = pfaults->GetNextFaultElement();
		}

		pfaults = m_pProcessesStatesMap->GetNextStateFaultList();
	}

	// 2) Send all alarms as traps
	eMcuState state = m_pProcess->GetSystemState();
	if (!alarms.empty())
	{

		CSnmpUtils::SendAlarmListSnmpTrap(alarms, false, (int) state);
	}

	// Updates Telemetry Data.
    const char* name = GetHostNameFromService(m_pMngmntIpParams_asIpService_fromProcess).c_str();
	CSegment* seg = new CSegment(CTelemetryValue(eTT_MCUDisplayName, name).GetSegment());
	CManagerApi(eProcessSNMPProcess).SendMsg(seg, SNMP_UPDATE_TELEMETRY_DATA_IND);

}


/////////////////////////////////////////////////////////////////////

void CMcuMngrManager::OnSNMPConfigInd(CSegment* seg)
{
  BOOL enabled;
  *seg >> enabled;

  TRACESTR(eLevelInfoNormal)	<< "OnSNMPConfigInd: " << (int)enabled;

  if (enabled)
  {
	  m_isSNMPup = TRUE;
	  if (m_minimumDelaySnmpTimer == 0)
	  {
		  m_pProcess->GetSysConfig()->GetDWORDDataByKey(CFG_KEY_MINIMUM_DELAY_SNMP_READY_TIMER, m_minimumDelaySnmpTimer);
		  TRACESTR(eLevelInfoNormal) << "Minimum delay snmp ready timer " << m_minimumDelaySnmpTimer;
		  m_minimumDelaySnmpTimer = m_minimumDelaySnmpTimer * SECOND;

	  }

	  bool isStartupFinished = IsStartupFinished();

	  DWORD delayTime = !isStartupFinished ? m_minimumDelaySnmpTimer + DELAY_SNMP_READY_EXTRA_STARTUP : m_minimumDelaySnmpTimer;

	  if (m_snmpExtraTimeForStartupLaunch > 0)
	  {
		  //TRACESTR(eLevelInfoNormal)	<< "delete timer MCUMNGR_SNMP_READY_TIMER";
		  DeleteTimer(MCUMNGR_SNMP_READY_TIMER);
		  m_snmpExtraTimeForStartupLaunch = 0;
	  }
	  m_snmpExtraTimeForStartupLaunch = 1;
	  StartTimer(MCUMNGR_SNMP_READY_TIMER, delayTime);
  }
  else
  {
	  if (m_snmpExtraTimeForStartupLaunch > 0)
	  {
		  //TRACESTR(eLevelInfoNormal)	<< "delete timer MCUMNGR_SNMP_READY_TIMER -";
		  DeleteTimer(MCUMNGR_SNMP_READY_TIMER);
		  m_snmpExtraTimeForStartupLaunch = 0;
	  }
	  m_isSNMPReady = FALSE;
	  m_isSNMPup = FALSE;
  }
}
///////////////////////////////////////////////
STATUS CMcuMngrManager::StopSoftNtpStatus()
{
	STATUS stat = STATUS_OK;
	CConfigManagerApi configApi;
	if (eProductTypeEdgeAxis  == m_mcuMngrProductType || eProductTypeSoftMCUMfw == m_mcuMngrProductType  )
	{
		TRACESTR(eLevelInfoNormal) << " stop ntp service ";
		ServiceCommand command = eCmdStop;
		stat = configApi.HandleNtpService(command);
	}
	return stat;
}
//////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::StartSoftNtpServer()
{
	STATUS stat = STATUS_OK;
		CConfigManagerApi configApi;

		if (eProductTypeEdgeAxis  == m_mcuMngrProductType || eProductTypeSoftMCUMfw == m_mcuMngrProductType  )
		{
			TRACESTR(eLevelInfoNormal) << " start ntp service ";
			ServiceCommand command = eCmdStart;
			stat = configApi.HandleNtpService(command);
		}

		if(STATUS_OK != stat)
			TRACESTR(eLevelInfoNormal) << "Failed to start ntp service status  " << stat;

		return stat;
}
STATUS CMcuMngrManager::ConfigSoftNtp(std::vector< string > &ntp_servers)
{

	STATUS stat = STATUS_OK;
	CConfigManagerApi configApi;

	if (eProductTypeEdgeAxis  == m_mcuMngrProductType || eProductTypeSoftMCUMfw == m_mcuMngrProductType  )
	{
		std::string servers[NTP_MAX_NUM_OF_SERVERS];
		int len = ntp_servers.size();
		if(len <= NTP_MAX_NUM_OF_SERVERS )
		{
			for(int i =0;i <len;i++ )
				servers[i] = ntp_servers[i];

			stat = configApi.ConfigNtpServers(servers[0],servers[1],servers[2]);
		}
		else
			TRACESTR(eLevelInfoNormal) << " server vector is larger then NTP_MAX_NUM_OF_SERVERS";

		TRACESTR(eLevelInfoNormal) << " config ntp service  servers " << " " << servers[0]<< " " << servers[1]<< " " << servers[2];

	}
	return stat;
}
/////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::NtpSetServer(CSystemTime* pTime)
{
	std::string cmd;
	std::string answer;
	STATUS stat = STATUS_OK;
	CConfigManagerApi configApi;
	 eProductType curProductType = m_mcuMngrProductType;

	if (eProductTypeEdgeAxis  == curProductType || eProductTypeSoftMCUMfw == curProductType  )
	{

		stat = StopSoftNtpStatus();
	}
	else
	{
		cmd = "sudo /usr/bin/killall NTP_Bypass_SoftMCU_Server";

		TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::NtpSetServer: cmd " << cmd;

		stat = SystemPipedCommand(cmd.c_str(),answer);
		TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
			"CMcuMngrManager::NtpSetServer: " << cmd << std::endl << answer;
	}
	if(STATUS_OK != stat)
	{
		return STATUS_FAIL;
	}

	if (pTime->GetIsNTP())
	{
		char serverStr[256];
		UINT32 i = 0;
		std::string ipV4Addr = "";

		memset(serverStr, 0, sizeof(serverStr));

		strcpy(serverStr, " ");
		std::vector< string >  ntp_servers;

		//get the ntp servers of the CSystemTime struct and generate the server string
		//note that we can have BOTH IPv4 & IPv6 servers within the struct!!!
		for (i = 0 ;i < NTP_MAX_NUM_OF_SERVERS;i++)
		{
			char tmpAddrBuffer[IPV6_ADDRESS_LEN + 1]; //used locally within this for loop only
			memset(tmpAddrBuffer, 0, (IPV6_ADDRESS_LEN + 1)); //we clean before we use

			//retrieve IPv4 address into ascii form from its UINT32 form
	//		memset(ipV4Addr, 0, sizeof(ipV4Addr)); //we clean before we use
	//		::SystemDWORDToIpString(pTime->GetNTP_IPAddress(i), ipV4Addr);
			ipV4Addr = pTime->GetNTP_IPAddress(i);

			TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::NtpSetServer: num " << i
				<< " IPv4 " << ipV4Addr
				<< " IPv6 " << pTime->GetNTP_IPv6_Address(i);

			//we have BOTH IPv4 & IPv6 addresses in ascii format
			//now we wanna validate that NOT both IPv4 & IPv6 in index i contains valid addresses.
			if((ipV4Addr != INVALID_SERVER_ADDR_STR)  &&
				(memcmp(pTime->GetNTP_IPv6_Address(i), NTP_INVALID_SERVER_ADDR_STR_IPV6, strlen(NTP_INVALID_SERVER_ADDR_STR_IPV6)) != 0))
			{
				PASSERT(i+100);

				continue;
			}

			//now we wanna see that at least one of them (IPv4 OR IPv6) are valid -
			//else there is nothing to do in this loop entry
			if((ipV4Addr == INVALID_SERVER_ADDR_STR) &&
				(memcmp(pTime->GetNTP_IPv6_Address(i), NTP_INVALID_SERVER_ADDR_STR_IPV6, strlen(NTP_INVALID_SERVER_ADDR_STR_IPV6)) == 0))
			{
				continue;
			}

			//now we know for sure that either IPv4 OR IPv6 are valid
			if(ipV4Addr != INVALID_SERVER_ADDR_STR)
			{
				//memcpy(tmpAddrBuffer, ipV4Addr, ipV4Addr.c_str()); //we use IPv4
				strncat(serverStr, ipV4Addr.c_str(), sizeof(serverStr) - strlen(serverStr) -1);
				strncat(serverStr, " ", sizeof(serverStr) - strlen(serverStr) -1);
				ntp_servers.push_back(ipV4Addr);

			}
			else
			{
				//add the address - either IPv4 OR IPv6
				memcpy(tmpAddrBuffer, pTime->GetNTP_IPv6_Address(i), IPV6_ADDRESS_LEN); //we use IPv6
				strcat(serverStr, tmpAddrBuffer);
				strcat(serverStr," ");
				ntp_servers.push_back(tmpAddrBuffer);
			}

		}

		if (strlen(serverStr) == 1)
		{
			TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::NtpSetServer: no valid ip address given.";

			return STATUS_FAIL;
		}
		STATUS stat = STATUS_OK;
		if (eProductTypeEdgeAxis  == curProductType || eProductTypeSoftMCUMfw == curProductType  )
		{
			stat = ConfigSoftNtp(ntp_servers);
			if(STATUS_OK == stat)
			{
				stat =  StartSoftNtpServer();
			}
		}
		else
		{
			cmd = "sudo "+MCU_MCMS_DIR+"/Bin/NTP_Bypass_SoftMCU_Server \"";
			cmd += serverStr;
			cmd += "\"";

			 stat = SystemPipedCommand(cmd.c_str(), answer, TRUE, TRUE, FALSE);

			TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
				"CMcuMngrManager::NtpSetServer: " << cmd << std::endl << answer;
		}
		if(STATUS_OK != stat)
		{
			return STATUS_FAIL;
		}
	}
	else
	{
		//no ntp
		struct timeval tv;
		struct timezone tz;
		struct tm sTm;
		INT32 nTimeSuccess = 0;

		TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::NtpSetServer: Got Time... ";

		const CStructTm* pStructTm = pTime->GetMCUTime();

		PASSERTSTREAM_AND_RETURN_VALUE(pStructTm->m_year < 0 || pStructTm->m_year > 3000,
			"CMcuMngrManager::NtpSetServer: invalid year value " << pStructTm->m_year,
			STATUS_FAIL);

		PASSERTSTREAM_AND_RETURN_VALUE(pStructTm->m_mon < 1 || pStructTm->m_mon > 12,
			"CMcuMngrManager::NtpSetServer: invalid month value " << pStructTm->m_mon,
			STATUS_FAIL);

		PASSERTSTREAM_AND_RETURN_VALUE(pStructTm->m_day < 1 || pStructTm->m_day > 31,
			"CMcuMngrManager::NtpSetServer: invalid day value " << pStructTm->m_day,
			STATUS_FAIL);

		PASSERTSTREAM_AND_RETURN_VALUE(pStructTm->m_hour < 0 || pStructTm->m_hour > 23,
			"CMcuMngrManager::NtpSetServer: invalid hour value " << pStructTm->m_hour,
			STATUS_FAIL);

		PASSERTSTREAM_AND_RETURN_VALUE(pStructTm->m_min < 0 || pStructTm->m_min > 59,
			"CMcuMngrManager::NtpSetServer: invalid min value " << pStructTm->m_min,
			STATUS_FAIL);

		PASSERTSTREAM_AND_RETURN_VALUE(pStructTm->m_sec < 0 || pStructTm->m_sec > 59,
			"CMcuMngrManager::NtpSetServer: invalid second value " << pStructTm->m_sec,
			STATUS_FAIL);

		char dateStr[128];

		sprintf(dateStr, "\"%04u-%02u-%02u %02u:%02u:%02u\"",
			pStructTm->m_year, pStructTm->m_mon, pStructTm->m_day,
			pStructTm->m_hour, pStructTm->m_min, pStructTm->m_sec);

		TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::NtpSetServer: "
			<< "Time : " << dateStr;

		CStructTm timeBeforeUpdate;
		SystemGetTime(timeBeforeUpdate);

		cmd = "sudo /bin/date -u -s ";
		cmd += dateStr;

		stat = SystemPipedCommand(cmd.c_str(),answer);
		TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
			"CMcuMngrManager::NtpSetServer :" << cmd << std::endl << answer;

		if(STATUS_OK != stat)
		{
			return STATUS_FAIL;
		}

		cmd = "sudo /sbin/hwclock -w";

		stat = SystemPipedCommand(cmd.c_str(),answer);
		TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
			"CMcuMngrManager::NtpSetServer :" << cmd << std::endl << answer;

		if(STATUS_OK != stat)
		{
			return STATUS_FAIL;
		}
		else
		{
			SyncNtp(TRUE, &timeBeforeUpdate);
		}
	}

	return STATUS_OK;
}



///////////////////////////////////////////////////////
STATUS CMcuMngrManager::HandleTerminalSetEthSetting(CTerminalCommand & command, std::ostream& answer)
{
	PTRACE(eLevelInfoNormal, "CMcuMngrManager::SetEthSetting ");

	DWORD numOfParams = command.GetNumOfParams();

	if(4 != numOfParams)
	{
		answer << "error: missing arguments";
		answer << "usage: set_eth [eth name] [authentication protocol] [username] [password]";
		return STATUS_FAIL;
	}

	const string &sethname = command.GetToken(eCmdParam1);
	const string &sauthP    = command.GetToken(eCmdParam2);
	const string &sUserName	   = command.GetToken(eCmdParam3);
	const string &sPass	   = command.GetToken(eCmdParam4);

	DWORD authenticationP = atoi(sauthP.c_str());



	CSegment* pSeg = new CSegment;
	*pSeg <<   (DWORD) authenticationP
			<<   sethname
			<<   sUserName
			<<   sPass;



	CManagerApi api(eProcessMcuMngr);

	api.SendMsg(pSeg, MCUMNGR_GENERATE_CONF_FILE_802_1X);






	return STATUS_OK;
}


DWORD CMcuMngrManager::convertSlotIdPortIdToServiceId(DWORD slotid ,DWORD pqid)
{
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::convertSlotIdPortIdToServiceId get parameters"

	 						<< "\n802.1x checked slotId :     " << slotid
	 						<< "\n802.1x checked pqid:     " << pqid;


 	for (int i=0; i<MAX_NUMBER_OF_SERVICES_IN_RMX_4000; i++)
 		{
 		for (int j=0; j<MAX_NUM_PQS;j++)
 		{
 			/*TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::convertSlotIdPortIdToServiceId"

 						<< "\n802.1x slotId :     " << m_StaticIpServiceList[i].IPServUDPperPQList[j].boardId
 						<< "\n802.1x pqid :     " << m_StaticIpServiceList[i].IPServUDPperPQList[j].PQid
 						<< "\n802.1x IpV4Addr.ip :     " << m_StaticIpServiceList[i].IPServUDPperPQList[j].IpV4Addr.ip
 						<< "\n802.1x servId :     " << m_StaticIpServiceList[i].ServId;*/

 		    if (m_StaticIpServiceList[i].IPServUDPperPQList[j].boardId == slotid &&
 		    		m_StaticIpServiceList[i].IPServUDPperPQList[j].PQid == pqid &&
 		    		m_StaticIpServiceList[i].IPServUDPperPQList[j].IpV4Addr.ip != 0)

 		    	      //return m_StaticIpServiceList[i].ServId;
 		    	return i+1;
 		}

 		}
 	return 0;

}




DWORD CMcuMngrManager::findSigServiceIdNotMS()
{
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::findSigServiceId ";


 	for (int i=0; i<MAX_NUMBER_OF_SERVICES_IN_RMX_4000; i++)
 		{
 		for (int j=0; j<MAX_NUM_PQS;j++)
 		{
 			/*TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::findSigServiceId"

 						<< "\n802.1x slotId :     " << m_StaticIpServiceList[i].IPServUDPperPQList[j].boardId
 						<< "\n802.1x pqid :     " << m_StaticIpServiceList[i].IPServUDPperPQList[j].PQid
 						<< "\n802.1x IpV4Addr.ip :     " << m_StaticIpServiceList[i].IPServUDPperPQList[j].IpV4Addr.ip
 						<< "\n802.1x servId :     " << m_StaticIpServiceList[i].ServId;*/

 		    if (m_StaticIpServiceList[i].IPServUDPperPQList[j].IpV4Addr.ip != 0)
 		    {

 		    	      //return m_StaticIpServiceList[i].ServId;
 		    	return i+1;
 		    }
 		}

 		}
 	return 0;

}



/////////////////////////////////////////////////////////////////////////////
E_802_1xEth CMcuMngrManager::ConvertPortToEthName(CEthernetSettingsConfig *pCurEthConfig)
{

	E_802_1xEth eth = E_802_1x_INVALID;


	DWORD portId= pCurEthConfig->GetPortId();
	eEthPortType portType= pCurEthConfig->GetPortType();
	eEthMediaPortType mediaPortType= pCurEthConfig->GetMediaPortType();

	eProductType curProductType  = m_pProcess->GetProductType() ;

	switch (portType)
	{
	case eEthPortType_Media:
		//need conversion from slot&port to ethnum
		if (eEthMediaPortType_Rtm_Isdn ==  mediaPortType)
					eth = E_802_1x_ETH2;
		else //if (eEthMediaPortType_Rtm_Lan ==  mediaPortType)
		{
			if (portId == 1)
			{
				eth = E_802_1x_ETH3;
			}
			if (portId == 2)
			{
				eth = E_802_1x_ETH2;
			}
		}





		break;
	case eEthPortType_Modem:

		eth = E_802_1x_ETH0;



		break;

	case eEthPortType_ManagementShelfMngr:


		if (eProductTypeRMX1500 == curProductType || eProductTypeRMX4000 == curProductType)
			eth = E_802_1x_ETH1;
		else if (eProductTypeRMX2000 == curProductType)
			eth = E_802_1x_ETH0;



		break;

	case eEthPortType_Management1:
	case eEthPortType_ManagementShelfMngr_Managment_Signaling_media:
	case eEthPortType_ManagementShelfMngr_Managment_Signaling :
	case eEthPortType_ManagementShelfMngr_Managment:

		if (eProductTypeRMX2000 == curProductType)
			eth = E_802_1x_ETH0;
		//added by Richer for 802.1x project om 2013.12.26 
		else if(eProductTypeNinja == curProductType || eProductTypeGesher == curProductType)
		{
		    if (portId == 1)
		    {
			 eth = E_802_1x_ETH0;
		    }
		    if (portId == 2)
		    {
			 eth = E_802_1x_ETH1;
		    }
		}
		else
		    eth = E_802_1x_ETH1;
		break;

	case eEthPortType_Signaling1:
	case eEthPortType_Media_Signaling:

		if (eProductTypeRMX2000 == curProductType)
			eth = E_802_1x_ETH0;
		else
		    eth = E_802_1x_ETH2;

		break;



	default:

		break;
	}


	return eth;
}


/////////////////////////////////////////////////////////////////////////////
const char * CMcuMngrManager::convertPortTypeToConfPath(DWORD slotId , eEthPortType portType)
{
	const char * path="";
	switch (portType)
	{
	case eEthPortType_Media:
	{
		switch (slotId)
		{
		case FIXED_BOARD_ID_RTM_1:
		case FIXED_BOARD_ID_MEDIA_1:
			//return MCU_MCMS_DIR+"/802_1xEmb/media1";
			return CONF_802_1x_MEDIA1_FILES_PATH.c_str();

		case FIXED_BOARD_ID_RTM_2:
		case FIXED_BOARD_ID_MEDIA_2:
			//return MCU_MCMS_DIR+"/802_1xEmb/media2";
			return CONF_802_1x_MEDIA2_FILES_PATH.c_str();


		case FIXED_BOARD_ID_RTM_3:
		case FIXED_BOARD_ID_MEDIA_3:
			//return MCU_MCMS_DIR+"/802_1xEmb/media3";
			return CONF_802_1x_MEDIA3_FILES_PATH.c_str();


		case FIXED_BOARD_ID_RTM_4:
		case FIXED_BOARD_ID_MEDIA_4:
			//return MCU_MCMS_DIR+"/802_1xEmb/media4";
			return CONF_802_1x_MEDIA4_FILES_PATH.c_str();

		//for RMX2000
		case FIXED_BOARD_ID_SWITCH:
			return CONF_802_1x_FILES_PATH.c_str();

		default:

			break;


		}
		break;
	}
	case eEthPortType_Management1:
	case eEthPortType_Signaling1:
	case eEthPortType_Media_Signaling_Managment:
	case	eEthPortType_Media_Signaling:

		return CONF_802_1x_FILES_PATH.c_str();

	case eEthPortType_Modem:
	case eEthPortType_ManagementShelfMngr:
	case eEthPortType_ManagementShelfMngr_Managment_Signaling_media:
	case eEthPortType_ManagementShelfMngr_Managment_Signaling:
	case eEthPortType_ManagementShelfMngr_Managment:
		//return MCU_MCMS_DIR+"/802_1xEmb/switch";
		return CONF_802_1x_SWITCH_FILES_PATH.c_str();


	default:

				break;

	}

	return path;
}

/////////////////////////////////////////////////////////////////////////////
const char * CMcuMngrManager::convertPortTypeToConfPathForEmbSecurity(DWORD slotId , eEthPortType portType)
{
	const char * path="";
	switch (portType)
	{
	case eEthPortType_Media:
	{
		switch (slotId)
		{
		case FIXED_BOARD_ID_RTM_1:
		case FIXED_BOARD_ID_MEDIA_1:
			return (MCU_MCMS_DIR+"/802_1xEmb/media1").c_str();

		case FIXED_BOARD_ID_RTM_2:
		case FIXED_BOARD_ID_MEDIA_2:
			return (MCU_MCMS_DIR+"/802_1xEmb/media2").c_str();


		case FIXED_BOARD_ID_RTM_3:
		case FIXED_BOARD_ID_MEDIA_3:
			return (MCU_MCMS_DIR+"/802_1xEmb/media3").c_str();


		case FIXED_BOARD_ID_RTM_4:
		case FIXED_BOARD_ID_MEDIA_4:
			return (MCU_MCMS_DIR+"/802_1xEmb/media4").c_str();

		default:

			break;


		}
		break;
	}
	case eEthPortType_Management1:
	case eEthPortType_Signaling1:
	case eEthPortType_Media_Signaling_Managment:
	case	eEthPortType_Media_Signaling:

		return CONF_802_1x_FILES_PATH.c_str();

	case eEthPortType_Modem:
	case eEthPortType_ManagementShelfMngr:
	case eEthPortType_ManagementShelfMngr_Managment_Signaling_media:
	case eEthPortType_ManagementShelfMngr_Managment_Signaling:
	case eEthPortType_ManagementShelfMngr_Managment:
		return (MCU_MCMS_DIR+"/802_1xEmb/switch").c_str();


	default:

				break;

	}

	return path;
}




/////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::GenerateConfFlie802_1x(CEthernetSettingsConfig *pCurEthConfig,E_802_1xCertificateValidationStatus &certValidStat)
{

	certValidStat = E_CERTIFICATE_OK;
	STATUS generateFileStatus = STATUS_FAIL;


	eEth802_1xAuthenticationType authType_802_1x = pCurEthConfig->Get802_1xAuthenticationProtocol();
	const char * userName = pCurEthConfig->Get802_1xUserName();
	const char * passUserName = pCurEthConfig->Get802_1xUserPassword();
	DWORD slotId= pCurEthConfig->GetSlotId();
	DWORD portId= pCurEthConfig->GetPortId();
	eEthPortType portType = pCurEthConfig->GetPortType();
	eEthMediaPortType mediaPortType= pCurEthConfig->GetMediaPortType();

	eProductType curProductType  = m_pProcess->GetProductType() ;


	TRACESTR(eLevelInfoNormal) << "\nGenerateConfFlie802_1x::GenerateConfFlie802_1x"
			<< "\n802.1x authentication protocol:     " << Get802_1xAuthenticationTypeStr(authType_802_1x)
			<< "\n802.1x slotId :     " << slotId
			<< "\n802.1x portId :     " << portId
			<< "\n802.1x userName :     " << userName;
			//<< "\n802.1x passUserName :     " << passUserName;



	u802_1xCgNetworkBlockDescriptor sNetBlkDescMD5;
	u802_1xCgNetworkBlockDescriptor sNetBlkDescPEAP_MSCHAPV2;
	u802_1xCgNetworkBlockDescriptor sNetBlkDescEAP_TLS;


	memset(&sNetBlkDescMD5,0, sizeof(u802_1xCgNetworkBlockDescriptor));
	memset(&sNetBlkDescPEAP_MSCHAPV2, 0,sizeof(u802_1xCgNetworkBlockDescriptor));
	memset(&sNetBlkDescEAP_TLS, 0,sizeof(u802_1xCgNetworkBlockDescriptor));


	std::string path;
	std::string pathForSecurity;



	path = convertPortTypeToConfPath(slotId,portType);
	TRACESTR(eLevelInfoNormal) << "\nCConfiguratorManager::GenerateConfFlie802_1x the path is " << path;

	pathForSecurity = convertPortTypeToConfPathForEmbSecurity(slotId,portType);
	TRACESTR(eLevelInfoNormal) << "\nCConfiguratorManager::GenerateConfFlie802_1x the NFS path is " << pathForSecurity;


	E_802_1xEth ethNum = ConvertPortToEthName(pCurEthConfig);
	TRACESTR(eLevelInfoNormal) << "\nCConfiguratorManager::GenerateConfFlie802_1x the eth num is " << ethNum;



	switch(authType_802_1x) {

	    case eEth802_1xAuthenticationType_Off:
	    {
	    	generateFileStatus = STATUS_OK;
	    	break;
	    }

		case eEth802_1xAuthenticationType_PEAPv0_MSCHAPV2:
		{
			TRACESTR(eLevelInfoNormal) << "\nCConfiguratorManager::GenerateConfFlie802_1x E_802_1x_METHOD_PEAP_MSCHAPV2";

			sNetBlkDescPEAP_MSCHAPV2.eMethod = E_802_1x_METHOD_PEAP_MSCHAPV2;

			strncpy(sNetBlkDescPEAP_MSCHAPV2.uNetBlkParams.sPeapMschapV2.caIdentity , userName,sizeof(sNetBlkDescPEAP_MSCHAPV2.uNetBlkParams.sPeapMschapV2.caIdentity)-1);
			sNetBlkDescPEAP_MSCHAPV2.uNetBlkParams.sPeapMschapV2.caIdentity[P802_1x_USER_NAME_MAX_LENGTH-1]='\0';

			strncpy(sNetBlkDescPEAP_MSCHAPV2.uNetBlkParams.sPeapMschapV2.caPassword , passUserName,sizeof(sNetBlkDescPEAP_MSCHAPV2.uNetBlkParams.sPeapMschapV2.caPassword)-1);
			sNetBlkDescPEAP_MSCHAPV2.uNetBlkParams.sPeapMschapV2.caPassword[P802_1x_PASSWD_MAX_LENGTH-1]='\0';

			strncpy(sNetBlkDescPEAP_MSCHAPV2.uNetBlkParams.sPeapMschapV2.caSsid, "", strlen(""));


			if (IsSkipCertificateValidation())
			{
				TRACESTR(eLevelInfoNormal) << "\nCConfiguratorManager::GenerateConfFlie802_1x eEth802_1xAuthenticationType_EAP_TLS skip certificate validation";

				//strncpy(sNetBlkDescPEAP_MSCHAPV2.uNetBlkParams.sPeapMschapV2.caCaCert, "", strlen(""));
			}
			else
				CG802_1xSetCACertFile_CHAP(portType ,&sNetBlkDescPEAP_MSCHAPV2);



			generateFileStatus = CG802_1xGenerateWpaSuppConfFile(path.c_str(),pathForSecurity.c_str(), ethNum, &sNetBlkDescPEAP_MSCHAPV2);


			break;
		}

		case eEth802_1xAuthenticationType_EAP_TLS:
		{
			TRACESTR(eLevelInfoNormal) << "\nCConfiguratorManager::GenerateConfFlie802_1x eEth802_1xAuthenticationType_EAP_TLS";



			sNetBlkDescEAP_TLS.eMethod = E_802_1x_METHOD_EAP_TLS;
			strncpy(sNetBlkDescEAP_TLS.uNetBlkParams.sEapTls.caSsid, "", strlen(""));
			strncpy(sNetBlkDescEAP_TLS.uNetBlkParams.sEapTls.caIdentity, userName,sizeof(sNetBlkDescEAP_TLS.uNetBlkParams.sEapTls.caIdentity)-1);
			sNetBlkDescEAP_TLS.uNetBlkParams.sEapTls.caIdentity[P802_1x_USER_NAME_MAX_LENGTH-1]='\0';


			CG802_1xSetCACertFile_TLS(portType ,&sNetBlkDescEAP_TLS);


			bool isOneCert = IsOneCertificate();
			if (isOneCert == true) //one certificate
			{


			//	if (IsCpuEthPort(portType) && (eProductTypeRMX2000 != curProductType))
				//	strncpy(sNetBlkDescEAP_TLS.uNetBlkParams.sEapTls.caClientCert, (MCU_CONFIG_DIR+"/keys/cert_off.pem").c_str(), strlen(MCU_CONFIG_DIR+"/keys/cert_off.pem").c_str()));  //mngmnt certificate
				//else
				//	strncpy(sNetBlkDescEAP_TLS.uNetBlkParams.sEapTls.caClientCert, "/mnt/mfa_cm_fs/certficate_repository/cert_off.pem", strlen("/mnt/mfa_cm_fs/certficate_repository/cert_off.pem"));  //mngmnt certificate

				CG802_1xSetClientCertFile(portType,&sNetBlkDescEAP_TLS);

				//run certificate check
				certValidStat = ValidateCertificateFor802_1xTLS((char*)(MCU_CONFIG_DIR+"/keys/cert_off.pem").c_str(),sNetBlkDescEAP_TLS.uNetBlkParams.sEapTls.caIdentity);




				TRACESTR(eLevelInfoNormal) << "\nCConfiguratorManager::GenerateConfFlie802_1x eEth802_1xAuthenticationType_EAP_TLS ONE CERTIFICATE ";
				//if (IsCpuEthPort(portType) && (eProductTypeRMX2000 != curProductType))
					//strcpy(sNetBlkDescEAP_TLS.uNetBlkParams.sEapTls.caPrivateKey, (MCU_CONFIG_DIR+"/keys/private3.pem").c_str());
				//else
					//strcpy(sNetBlkDescEAP_TLS.uNetBlkParams.sEapTls.caPrivateKey, "/mnt/mfa_cm_fs/certficate_repository/private3.pem");

				CG802_1xSetPrivateKeyFile(portType,&sNetBlkDescEAP_TLS);

			}
			else
			{
				ALLOCBUFFER( keyPath, ONE_LINE_BUFFER_LEN);
				ALLOCBUFFER( certPath, ONE_LINE_BUFFER_LEN);

				DWORD serviceId;

				switch (portType)
				{


				case    eEthPortType_Signaling1:
				case    eEthPortType_Media_Signaling: //for RMX2000

					serviceId = findSigServiceIdNotMS();


					TRACESTR(eLevelInfoNormal) << "\nCConfiguratorManager::GenerateConfFlie802_1x eEth802_1xAuthenticationType_EAP_TLS slotid "<<slotId
							<<" portId " << portId << " cert Path " << certPath << " serviceId " << serviceId;



					if (serviceId != 0)
					{
						snprintf(certPath,ONE_LINE_BUFFER_LEN-1,(MCU_CONFIG_DIR+"/keys/cs/cs%d/cert.pem").c_str(),serviceId);


						strncpy(sNetBlkDescEAP_TLS.uNetBlkParams.sEapTls.caClientCert, certPath,sizeof(sNetBlkDescEAP_TLS.uNetBlkParams.sEapTls.caClientCert)-1);
						sNetBlkDescEAP_TLS.uNetBlkParams.sEapTls.caClientCert[P802_1x_FILES_PATH_MAX_LENGTH-1]='\0';


						//run certificate check
						certValidStat = ValidateCertificateFor802_1xTLS(certPath,sNetBlkDescEAP_TLS.uNetBlkParams.sEapTls.caIdentity);


						snprintf(keyPath,ONE_LINE_BUFFER_LEN-1,(MCU_CONFIG_DIR+"/keys/cs/cs%d/pkey.pem").c_str(),serviceId);

						TRACESTR(eLevelInfoNormal) << "\nCConfiguratorManager::GenerateConfFlie802_1x eEth802_1xAuthenticationType_EAP_TLS slotid "<<slotId
								<<" portId " << portId << " private keyPath " << keyPath << " serviceId " << serviceId;

						strncpy(sNetBlkDescEAP_TLS.uNetBlkParams.sEapTls.caPrivateKey, keyPath,sizeof(sNetBlkDescEAP_TLS.uNetBlkParams.sEapTls.caPrivateKey)-1);
						sNetBlkDescEAP_TLS.uNetBlkParams.sEapTls.caPrivateKey[P802_1x_FILES_PATH_MAX_LENGTH-1]='\0';
						break;
					}
					else
					{
						TRACESTR(eLevelInfoNormal) << "\nCConfiguratorManager::GenerateConfFlie802_1x we reach the default we return to one cert ";

						//if (IsCpuEthPort(portType) && (eProductTypeRMX2000 != curProductType))
						//	strncpy(sNetBlkDescEAP_TLS.uNetBlkParams.sEapTls.caClientCert, (MCU_CONFIG_DIR+"/keys/cert_off.pem").c_str(), strlen((MCU_CONFIG_DIR+"/keys/cert_off.pem").c_str()));  //mngmnt certificate
						//else
							//strncpy(sNetBlkDescEAP_TLS.uNetBlkParams.sEapTls.caClientCert, "/mnt/mfa_cm_fs/certficate_repository/cert_off.pem", strlen("/mnt/mfa_cm_fs/certficate_repository/cert_off.pem"));  //mngmnt certificate

						CG802_1xSetClientCertFile(portType,&sNetBlkDescEAP_TLS);

						//run certificate check
						certValidStat = ValidateCertificateFor802_1xTLS((char*)(MCU_CONFIG_DIR+"/keys/cert_off.pem").c_str(),sNetBlkDescEAP_TLS.uNetBlkParams.sEapTls.caIdentity);




						TRACESTR(eLevelInfoNormal) << "\nCConfiguratorManager::GenerateConfFlie802_1x eEth802_1xAuthenticationType_EAP_TLS ONE CERTIFICATE ";
						//if (IsCpuEthPort(portType) && (eProductTypeRMX2000 != curProductType))
						//	strcpy(sNetBlkDescEAP_TLS.uNetBlkParams.sEapTls.caPrivateKey, (MCU_CONFIG_DIR+"/keys/private3.pem").c_str());
						//else
							//strcpy(sNetBlkDescEAP_TLS.uNetBlkParams.sEapTls.caPrivateKey, "/mnt/mfa_cm_fs/certficate_repository/private3.pem");

						CG802_1xSetPrivateKeyFile(portType,&sNetBlkDescEAP_TLS);

						break;
					}

				case    eEthPortType_Media:
					{
					DWORD pqId = 0;
					if (portId == 2) pqId = 1;
					if (portId == 1) pqId = 2;

					//TRACESTR(eLevelInfoNormal) << "\nCConfiguratorManager::GenerateConfFlie802_1x pqid mod" << (portId % 2)+1;
					if (eProductTypeRMX2000 == curProductType)
						serviceId = convertSlotIdPortIdToServiceId(slotId,pqId);
					else
					{
						DWORD phySlotId = (slotId%13)+1;

						serviceId = convertSlotIdPortIdToServiceId(phySlotId,pqId);
					}




					TRACESTR(eLevelInfoNormal) << "\nCConfiguratorManager::GenerateConfFlie802_1x eEth802_1xAuthenticationType_EAP_TLS slotid "<<slotId
							<<" portId " << portId << " cert Path " << certPath << " serviceId " << serviceId;



					if (serviceId != 0)
					{
						sprintf(certPath,"/mnt/mfa_cm_fs/certficate_repository/cs/cs%d/cert.pem",serviceId);


						strncpy(sNetBlkDescEAP_TLS.uNetBlkParams.sEapTls.caClientCert, certPath,sizeof(sNetBlkDescEAP_TLS.uNetBlkParams.sEapTls.caClientCert)-1);
						sNetBlkDescEAP_TLS.uNetBlkParams.sEapTls.caClientCert[P802_1x_FILES_PATH_MAX_LENGTH-1]='\0';

						//run certificate check
						certValidStat = ValidateCertificateFor802_1xTLS(certPath,sNetBlkDescEAP_TLS.uNetBlkParams.sEapTls.caIdentity);


						sprintf(keyPath,"/mnt/mfa_cm_fs/certficate_repository/cs/cs%d/pkey.pem",serviceId);

						TRACESTR(eLevelInfoNormal) << "\nCConfiguratorManager::GenerateConfFlie802_1x eEth802_1xAuthenticationType_EAP_TLS slotid "<<slotId
								<<" portId " << portId << " private keyPath " << keyPath << " serviceId " << serviceId;

						strncpy(sNetBlkDescEAP_TLS.uNetBlkParams.sEapTls.caPrivateKey, keyPath,sizeof(sNetBlkDescEAP_TLS.uNetBlkParams.sEapTls.caPrivateKey)-1);
						sNetBlkDescEAP_TLS.uNetBlkParams.sEapTls.caPrivateKey[P802_1x_FILES_PATH_MAX_LENGTH-1]='\0';
					}
						break;
					}

				default:  // go to one cert
					TRACESTR(eLevelInfoNormal) << "\nCConfiguratorManager::GenerateConfFlie802_1x we reach the default we return to one cert ";

					//if (IsCpuEthPort(portType) && (eProductTypeRMX2000 != curProductType))
						//strncpy(sNetBlkDescEAP_TLS.uNetBlkParams.sEapTls.caClientCert, (MCU_CONFIG_DIR+"/keys/cert_off.pem").c_str(), strlen((MCU_CONFIG_DIR+"/keys/cert_off.pem").c_str()));  //mngmnt certificate
					//else
						//strncpy(sNetBlkDescEAP_TLS.uNetBlkParams.sEapTls.caClientCert, "/mnt/mfa_cm_fs/certficate_repository/cert_off.pem", strlen("/mnt/mfa_cm_fs/certficate_repository/cert_off.pem"));  //mngmnt certificate

					CG802_1xSetClientCertFile(portType,&sNetBlkDescEAP_TLS);

					//run certificate check
					certValidStat = ValidateCertificateFor802_1xTLS((char*)(MCU_CONFIG_DIR+"/keys/cert_off.pem").c_str(),sNetBlkDescEAP_TLS.uNetBlkParams.sEapTls.caIdentity);




					TRACESTR(eLevelInfoNormal) << "\nCConfiguratorManager::GenerateConfFlie802_1x eEth802_1xAuthenticationType_EAP_TLS ONE CERTIFICATE ";
					//if (IsCpuEthPort(portType) && (eProductTypeRMX2000 != curProductType))
						//strcpy(sNetBlkDescEAP_TLS.uNetBlkParams.sEapTls.caPrivateKey, (MCU_CONFIG_DIR+"/keys/private3.pem").c_str());
					//else
						//strcpy(sNetBlkDescEAP_TLS.uNetBlkParams.sEapTls.caPrivateKey, "/mnt/mfa_cm_fs/certficate_repository/private3.pem");
					CG802_1xSetPrivateKeyFile(portType,&sNetBlkDescEAP_TLS);

					break;

				}


				DEALLOCBUFFER(keyPath);
				DEALLOCBUFFER(certPath);

			}


			string  macAddress;
			string cmd;
			string nic_name = "eth0";
		    if (m_mcuMngrProductType==eProductTypeSoftMCU || m_mcuMngrProductType==eProductTypeSoftMCUMfw || m_mcuMngrProductType==eProductTypeEdgeAxis)
			{
		    	CIPSpan * pSpan = m_pMngmntIpParams_asIpService_fromProcess->GetFirstSpan();
			    if(pSpan)
			        nic_name = pSpan->GetInterface();
			}
		    cmd = "echo -n `/sbin/ifconfig " + nic_name + " | grep HWaddr | awk '{ print $5 }'`";
			SystemPipedCommand(cmd.c_str(), macAddress);
			strncpy(sNetBlkDescEAP_TLS.uNetBlkParams.sEapTls.caPrivateKeyPass, macAddress.c_str(),sizeof(sNetBlkDescEAP_TLS.uNetBlkParams.sEapTls.caPrivateKeyPass)-1);
			sNetBlkDescEAP_TLS.uNetBlkParams.sEapTls.caPrivateKeyPass[P802_1x_FILES_PATH_MAX_LENGTH-1]='\0';


			generateFileStatus = CG802_1xGenerateWpaSuppConfFile(path.c_str(),pathForSecurity.c_str(), ethNum, &sNetBlkDescEAP_TLS);

			break;
		}

		case eEth802_1xAuthenticationType_EAP_MD5:
		{

			TRACESTR(eLevelInfoNormal) << "\nCConfiguratorManager::GenerateConfFlie802_1x eEth802_1xAuthenticationType_EAP_MD5";


			sNetBlkDescMD5.eMethod = E_802_1x_METHOD_EAP_MD5;

			strncpy(sNetBlkDescMD5.uNetBlkParams.sMd5.caIdentity , userName,sizeof(sNetBlkDescMD5.uNetBlkParams.sMd5.caIdentity)-1);
			sNetBlkDescMD5.uNetBlkParams.sMd5.caIdentity[P802_1x_USER_NAME_MAX_LENGTH-1]='\0';

			strncpy(sNetBlkDescMD5.uNetBlkParams.sMd5.caPassword , passUserName,sizeof(sNetBlkDescMD5.uNetBlkParams.sMd5.caPassword)-1);
			sNetBlkDescMD5.uNetBlkParams.sMd5.caPassword[P802_1x_PASSWD_MAX_LENGTH-1]='\0';

			strncpy(sNetBlkDescMD5.uNetBlkParams.sMd5.caSsid, "", strlen(""));
			generateFileStatus = CG802_1xGenerateWpaSuppConfFile(path.c_str(),pathForSecurity.c_str(), ethNum, &sNetBlkDescMD5);



			break;
		}

		default:
		{
			TRACESTR(eLevelInfoNormal) << __FUNCTION__ << " INVALID authType_802_1x " << authType_802_1x;

			return generateFileStatus;
		}
	}



	return generateFileStatus;



}

void CMcuMngrManager::CG802_1xSetCACertFile_CHAP(eEthPortType portType , u802_1xCgNetworkBlockDescriptor * uNetBlkDesc)
{

	eProductType curProductType  = m_pProcess->GetProductType() ;

	if (eProductTypeRMX2000 == curProductType)
	{
		BYTE bSeparateNetworks  = GetSeparateNetworksSupport();
		BYTE bRtmLam = GetRtmLanSupport();

		if (bSeparateNetworks == YES && bRtmLam == YES)
		{
			switch (portType)
			{
			case eEthPortType_ManagementShelfMngr_Managment:
			case eEthPortType_Media:

				strncpy(uNetBlkDesc->uNetBlkParams.sPeapMschapV2.caCaCert, "/mnt/mfa_cm_fs/certficate_repository/ca_cert/ca-bundle-client.crt", strlen("/mnt/mfa_cm_fs/certficate_repository/ca_cert/ca-bundle-client.crt"));
				break;
			case eEthPortType_Signaling1:
				strncpy(uNetBlkDesc->uNetBlkParams.sPeapMschapV2.caCaCert, (MCU_CONFIG_DIR+"/keys/ca_cert/ca-bundle-client.crt").c_str(), sizeof(uNetBlkDesc->uNetBlkParams.sPeapMschapV2.caCaCert)-1);
			default :
				TRACESTR(eLevelInfoNormal) << "\nCConfiguratorManager::GenerateConfFlie802_1x invalid port type ";

			}
		}
		else if (bSeparateNetworks == YES && bRtmLam == NO)
		{
			switch (portType)
			{
			case eEthPortType_ManagementShelfMngr_Managment:


				strncpy(uNetBlkDesc->uNetBlkParams.sPeapMschapV2.caCaCert, "/mnt/mfa_cm_fs/certficate_repository/ca_cert/ca-bundle-client.crt", strlen("/mnt/mfa_cm_fs/certficate_repository/ca_cert/ca-bundle-client.crt"));
				break;
			case eEthPortType_Media_Signaling:
				strncpy(uNetBlkDesc->uNetBlkParams.sPeapMschapV2.caCaCert, (MCU_CONFIG_DIR+"/keys/ca_cert/ca-bundle-client.crt").c_str(), sizeof(uNetBlkDesc->uNetBlkParams.sPeapMschapV2.caCaCert)-1);
			default :
				TRACESTR(eLevelInfoNormal) << "\nCConfiguratorManager::GenerateConfFlie802_1x invalid port type ";

			}
		}
		else

			strncpy(uNetBlkDesc->uNetBlkParams.sPeapMschapV2.caCaCert, "/mnt/mfa_cm_fs/certficate_repository/ca_cert/ca-bundle-client.crt", strlen("/mnt/mfa_cm_fs/certficate_repository/ca_cert/ca-bundle-client.crt"));



	}
	else
	{
		if (IsCpuEthPort(portType))
			strncpy(uNetBlkDesc->uNetBlkParams.sPeapMschapV2.caCaCert, (MCU_CONFIG_DIR+"/keys/ca_cert/ca-bundle-client.crt").c_str(), sizeof(uNetBlkDesc->uNetBlkParams.sPeapMschapV2.caCaCert)-1);
		else
			strncpy(uNetBlkDesc->uNetBlkParams.sPeapMschapV2.caCaCert, "/mnt/mfa_cm_fs/certficate_repository/ca_cert/ca-bundle-client.crt", strlen("/mnt/mfa_cm_fs/certficate_repository/ca_cert/ca-bundle-client.crt"));
	}
}



void CMcuMngrManager::CG802_1xSetCACertFile_TLS(eEthPortType portType , u802_1xCgNetworkBlockDescriptor * uNetBlkDesc)
{

	eProductType curProductType  = m_pProcess->GetProductType() ;

	if (eProductTypeRMX2000 == curProductType)
	{
		BYTE bSeparateNetworks  = GetSeparateNetworksSupport();
		BYTE bRtmLam = GetRtmLanSupport();

		if (bSeparateNetworks == YES && bRtmLam == YES)
		{
			switch (portType)
			{
			case eEthPortType_ManagementShelfMngr_Managment:
			case eEthPortType_Media:

				strncpy(uNetBlkDesc->uNetBlkParams.sEapTls.caCaCert, "/mnt/mfa_cm_fs/certficate_repository/ca_cert/ca-bundle-client.crt", strlen("/mnt/mfa_cm_fs/certficate_repository/ca_cert/ca-bundle-client.crt"));
				break;
			case eEthPortType_Signaling1:
				strncpy(uNetBlkDesc->uNetBlkParams.sEapTls.caCaCert, (MCU_CONFIG_DIR+"/keys/ca_cert/ca-bundle-client.crt").c_str(), sizeof(uNetBlkDesc->uNetBlkParams.sEapTls.caCaCert)-1);
			default :
				TRACESTR(eLevelInfoNormal) << "\nCConfiguratorManager::GenerateConfFlie802_1x invalid port type ";

			}
		}
		else if (bSeparateNetworks == YES && bRtmLam == NO)
		{
			switch (portType)
			{
			case eEthPortType_ManagementShelfMngr_Managment:


				strncpy(uNetBlkDesc->uNetBlkParams.sEapTls.caCaCert, "/mnt/mfa_cm_fs/certficate_repository/ca_cert/ca-bundle-client.crt", strlen("/mnt/mfa_cm_fs/certficate_repository/ca_cert/ca-bundle-client.crt"));
				break;
			case eEthPortType_Media_Signaling:
				strncpy(uNetBlkDesc->uNetBlkParams.sEapTls.caCaCert, (MCU_CONFIG_DIR+"/keys/ca_cert/ca-bundle-client.crt").c_str(), sizeof(uNetBlkDesc->uNetBlkParams.sEapTls.caCaCert)-1);
			default :
				TRACESTR(eLevelInfoNormal) << "\nCConfiguratorManager::GenerateConfFlie802_1x invalid port type ";

			}
		}
		else

			strncpy(uNetBlkDesc->uNetBlkParams.sEapTls.caCaCert, "/mnt/mfa_cm_fs/certficate_repository/ca_cert/ca-bundle-client.crt", strlen("/mnt/mfa_cm_fs/certficate_repository/ca_cert/ca-bundle-client.crt"));



	}
	else
	{
		if (IsCpuEthPort(portType))
			strncpy(uNetBlkDesc->uNetBlkParams.sEapTls.caCaCert, (MCU_CONFIG_DIR+"/keys/ca_cert/ca-bundle-client.crt").c_str(), sizeof(uNetBlkDesc->uNetBlkParams.sEapTls.caCaCert)-1);
		else
			strncpy(uNetBlkDesc->uNetBlkParams.sEapTls.caCaCert, "/mnt/mfa_cm_fs/certficate_repository/ca_cert/ca-bundle-client.crt", strlen("/mnt/mfa_cm_fs/certficate_repository/ca_cert/ca-bundle-client.crt"));
	}
}

void CMcuMngrManager::CG802_1xSetClientCertFile(eEthPortType portType , u802_1xCgNetworkBlockDescriptor* uNetBlkDesc)
{

	eProductType curProductType  = m_pProcess->GetProductType() ;

	if (eProductTypeRMX2000 == curProductType)
	{
		BYTE bSeparateNetworks  = GetSeparateNetworksSupport();
		BYTE bRtmLam = GetRtmLanSupport();

		if (bSeparateNetworks == YES && bRtmLam == YES)
		{
			switch (portType)
			{
			case eEthPortType_ManagementShelfMngr_Managment:
			case eEthPortType_Media:

				strncpy(uNetBlkDesc->uNetBlkParams.sEapTls.caClientCert, "/mnt/mfa_cm_fs/certficate_repository/cert_off.pem", strlen("/mnt/mfa_cm_fs/certficate_repository/cert_off.pem"));  //mngmnt certificate
				break;
			case eEthPortType_Signaling1:
				strncpy(uNetBlkDesc->uNetBlkParams.sEapTls.caClientCert, (MCU_CONFIG_DIR+"/keys/cert_off.pem").c_str(), sizeof(uNetBlkDesc->uNetBlkParams.sEapTls.caClientCert)-1);  //mngmnt certificate
			default :
				TRACESTR(eLevelInfoNormal) << "\nCConfiguratorManager::GenerateConfFlie802_1x invalid port type ";

			}
		}
		else if (bSeparateNetworks == YES && bRtmLam == NO)
		{
			switch (portType)
			{
			case eEthPortType_ManagementShelfMngr_Managment:

				strncpy(uNetBlkDesc->uNetBlkParams.sEapTls.caClientCert, "/mnt/mfa_cm_fs/certficate_repository/cert_off.pem", strlen("/mnt/mfa_cm_fs/certficate_repository/cert_off.pem"));  //mngmnt certificate
				break;
			case eEthPortType_Media_Signaling:
				strncpy(uNetBlkDesc->uNetBlkParams.sEapTls.caClientCert, (MCU_CONFIG_DIR+"/keys/cert_off.pem").c_str(), sizeof(uNetBlkDesc->uNetBlkParams.sEapTls.caClientCert)-1);  //mngmnt certificate
			default :
				TRACESTR(eLevelInfoNormal) << "\nCConfiguratorManager::GenerateConfFlie802_1x invalid port type ";

			}
		}
		else

			strncpy(uNetBlkDesc->uNetBlkParams.sEapTls.caClientCert, "/mnt/mfa_cm_fs/certficate_repository/cert_off.pem", strlen("/mnt/mfa_cm_fs/certficate_repository/cert_off.pem"));  //mngmnt certificate



	}
	else
	{
		if (IsCpuEthPort(portType))
			strncpy(uNetBlkDesc->uNetBlkParams.sEapTls.caClientCert, (MCU_CONFIG_DIR+"/keys/cert_off.pem").c_str(), sizeof(uNetBlkDesc->uNetBlkParams.sEapTls.caClientCert)-1);  //mngmnt certificate
		else
			strncpy(uNetBlkDesc->uNetBlkParams.sEapTls.caClientCert, "/mnt/mfa_cm_fs/certficate_repository/cert_off.pem", strlen("/mnt/mfa_cm_fs/certficate_repository/cert_off.pem"));  //mngmnt certificate
	}
}


void CMcuMngrManager::CG802_1xSetPrivateKeyFile(eEthPortType portType , u802_1xCgNetworkBlockDescriptor* uNetBlkDesc)
{

	eProductType curProductType  = m_pProcess->GetProductType() ;

	if (eProductTypeRMX2000 == curProductType)
	{
		BYTE bSeparateNetworks  = GetSeparateNetworksSupport();
		BYTE bRtmLam = GetRtmLanSupport();

		if (bSeparateNetworks == YES && bRtmLam == YES)
		{
			switch (portType)
			{
			case eEthPortType_ManagementShelfMngr_Managment:
			case eEthPortType_Media:

				strcpy(uNetBlkDesc->uNetBlkParams.sEapTls.caPrivateKey, "/mnt/mfa_cm_fs/certficate_repository/private3.pem");
				break;
			case eEthPortType_Signaling1:
				strncpy(uNetBlkDesc->uNetBlkParams.sEapTls.caPrivateKey, (MCU_CONFIG_DIR+"/keys/private3.pem").c_str(), sizeof(uNetBlkDesc->uNetBlkParams.sEapTls.caPrivateKey)-1);
			default :
				TRACESTR(eLevelInfoNormal) << "\nCConfiguratorManager::GenerateConfFlie802_1x invalid port type ";

			}
		}
		else if (bSeparateNetworks == YES && bRtmLam == NO)
		{
			switch (portType)
			{
			case eEthPortType_ManagementShelfMngr_Managment:

				strcpy(uNetBlkDesc->uNetBlkParams.sEapTls.caPrivateKey, "/mnt/mfa_cm_fs/certficate_repository/private3.pem");
				break;
			case eEthPortType_Media_Signaling:
				strncpy(uNetBlkDesc->uNetBlkParams.sEapTls.caPrivateKey, (MCU_CONFIG_DIR+"/keys/private3.pem").c_str(),
						sizeof(uNetBlkDesc->uNetBlkParams.sEapTls.caPrivateKey)-1);
			default :
				TRACESTR(eLevelInfoNormal) << "\nCConfiguratorManager::GenerateConfFlie802_1x invalid port type ";

			}
		}
		else

			strcpy(uNetBlkDesc->uNetBlkParams.sEapTls.caPrivateKey, "/mnt/mfa_cm_fs/certficate_repository/private3.pem");



	}
	else
	{
		if (IsCpuEthPort(portType))
			strncpy(uNetBlkDesc->uNetBlkParams.sEapTls.caPrivateKey, (MCU_CONFIG_DIR+"/keys/private3.pem").c_str(), sizeof(uNetBlkDesc->uNetBlkParams.sEapTls.caPrivateKey)-1);
		else
			strcpy(uNetBlkDesc->uNetBlkParams.sEapTls.caPrivateKey, "/mnt/mfa_cm_fs/certficate_repository/private3.pem");
	}
}

/////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::CG802_1xGenerateWpaSuppConfFile(const char* caConfFilePath,const char* pathForSecurity, E_802_1xEth eEthNum, u802_1xCgNetworkBlockDescriptor* uNetBlkDesc)
{

	//this function generates a net block for an MD5 network
	//EXAMPLE:
	//  			network={
	// 					ssid="example"
	//  				key_mgmt=IEEE8021X
	//  				eap=MD5
	//  				identity="user"
	//  				password="password"
	//  				eapol_flags=0
	//  			}

	//this function generates a net block for an MSCHAPV2 network
	//EXAMPLE:
	//  			network={
	//  				ssid="example"
	//  				scan_ssid=0
	//  				key_mgmt=WPA-EAP
	//  				eap=PEAP
	//  				identity="user@example.com"
	//  				password="foobar"
	//  				ca_cert="/etc/cert/ca.pem"
	//  				phase2="auth=MSCHAPV2"
	//  				eapol_flags=0
	//  			}

	//this function generates a net block for an EAP-TLS network
	//EXAMPLE:
	//  		network={
	//  			 ssid="work"
	//  			 scan_ssid=0
	//  			 key_mgmt=WPA-EAP
	//  			 pairwise=CCMP TKIP
	//  			 group=CCMP TKIP
	//  			 eap=TLS
	//  			 identity="user@example.com"
	//  			 ca_cert="/etc/cert/ca.pem"
	//  			 client_cert="/etc/cert/user.pem"
	//  			 private_key="/etc/cert/user.prv"
	//  			 private_key_passwd="password"
	//  			 eapol_flags=0
	//  		}

	INT32 lRc = STATUS_FAIL;
	INT32 lCharsWritten = 0;
	INT8 caConfStr[WPA_CONF_BUFFER_MAX_LENGTH];
	INT8 caConfFileName[P802_1x_FILES_PATH_MAX_LENGTH] = {0};
	FILE* pFile = NULL;

	memset(caConfStr,0,WPA_CONF_BUFFER_MAX_LENGTH);

	//validate params
	if(!caConfFilePath) {
		  TRACESTR(eLevelInfoNormal) << __FUNCTION__ << " BAD PARAM caConfFilePath = NULL";

		return  STATUS_FAIL;
	}

	if((eEthNum < 0) || (eEthNum > E_802_1x_INVALID)) {

		  TRACESTR(eLevelInfoNormal) << __FUNCTION__ << " BAD PARAM eEthNum = " << eEthNum;

		return  STATUS_FAIL;
	}

	if(!uNetBlkDesc) {

		  TRACESTR(eLevelInfoNormal) << __FUNCTION__ << " BAD PARAM uNetBlkDesc = NULL " ;
		return  STATUS_FAIL;
	}
	//we have good params

	//now first thing we do is try to open the out file for writing - if we fail opening the out
	//file for writing there is no much sense in going on.
	//first we generate the full name of the file to open
	lCharsWritten = snprintf(caConfFileName, P802_1x_FILES_PATH_MAX_LENGTH, "%s/%s%d%s", caConfFilePath, P802_1x_WPA_CONF_FILE_NAME_PREFIX, eEthNum, P802_1x_WPA_CONF_FILE_NAME_SUFFIX);
	if(lCharsWritten < 0) {

		TRACESTR(eLevelInfoNormal) << __FUNCTION__ << " snprintf failed. errno = " << errno;
		return  STATUS_FAIL;
	}

	//we shall now attempt to open the file for writing
	if((pFile = fopen(caConfFileName, "w")) == NULL) {

		TRACESTR(eLevelInfoNormal) << __FUNCTION__ << " fopen failed. errno = " << errno;
		if(errno == 2) //no such directory try again
		{
			BOOL res = CreateDirectory(caConfFilePath);
			if (res == FALSE)
			{
				TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__ << " creation of conf file directoty failed" ;
				return  STATUS_FAIL;
			}
			if((pFile = fopen(caConfFileName, "w")) == NULL)
				return  STATUS_FAIL;
		}
		else
		  return  STATUS_FAIL;
	}
	//we now have the file for writing

	//now switch by the method and create the string accordignly
	switch(uNetBlkDesc->eMethod) {
		case E_802_1x_METHOD_EAP_MD5:
		{
			//now we build the block into the OUT param while keeping track of number
			//of bytes we write so we wont overrun the buffer and also to return in the OUT param
			lCharsWritten = snprintf(caConfStr, WPA_CONF_BUFFER_MAX_LENGTH,	 "network={\n"
																			 "\tssid=\"%s\"\n"
																			 "\tscan_ssid=0\n"
																			 "\tkey_mgmt=IEEE8021X\n"
																			 "\teap=MD5\n"
																			 "\tidentity=\"%s\"\n"
																			 "\tpassword=\"%s\"\n"
																			 "\teapol_flags=0\n"
																			 "}",
																				uNetBlkDesc->uNetBlkParams.sMd5.caSsid,
																				uNetBlkDesc->uNetBlkParams.sMd5.caIdentity,
																				uNetBlkDesc->uNetBlkParams.sMd5.caPassword);
			break;
		}

		case E_802_1x_METHOD_EAP_TLS:
		{
			//now we start building the block into the OUT param while keeping track of number
			//of bytes we write so we wont overrun the buffer and also to return in the OUT param
			lCharsWritten = snprintf(caConfStr, WPA_CONF_BUFFER_MAX_LENGTH, "network={\n"
															 "\tssid=\"%s\"\n"
															 "\tscan_ssid=0\n"
															 "\tkey_mgmt=IEEE8021X\n"
															 "\tpairwise=CCMP TKIP\n"
															 "\tgroup=CCMP TKIP\n"
															 "\teap=TLS\n"
															 "\tidentity=\"%s\"\n"
															 "\tca_cert=\"%s\"\n"
															 "\tclient_cert=\"%s\"\n"
															 "\tprivate_key=\"%s\"\n"
					                                         "\tprivate_key_passwd=\"%s\"\n"
															 "\teapol_flags=0\n"
					                                         "}",
																uNetBlkDesc->uNetBlkParams.sEapTls.caSsid,
																uNetBlkDesc->uNetBlkParams.sEapTls.caIdentity,
																uNetBlkDesc->uNetBlkParams.sEapTls.caCaCert,
																uNetBlkDesc->uNetBlkParams.sEapTls.caClientCert,
																uNetBlkDesc->uNetBlkParams.sEapTls.caPrivateKey,
																uNetBlkDesc->uNetBlkParams.sEapTls.caPrivateKeyPass);
			break;
		}

		case E_802_1x_METHOD_PEAP_MSCHAPV2:
		{
			if (IsSkipCertificateValidation())
			//now we start building the block into the OUT param while keeping track of number
			//of bytes we write so we wont overrun the buffer and also to return in the OUT param
			   lCharsWritten = snprintf(caConfStr, WPA_CONF_BUFFER_MAX_LENGTH, "network={\n"
															 "\tssid=\"%s\"\n"
															 "\tscan_ssid=0\n"
															 "\tkey_mgmt=IEEE8021X\n"
															 "\teap=PEAP\n"
															 "\tidentity=\"%s\"\n"
															 "\tpassword=\"%s\"\n"
															 //"\tca_cert=\"%s\"\n"
															 "\tphase2=\"auth=MSCHAPV2\"\n"
															 "\teapol_flags=0\n"
					                                         "}",
																uNetBlkDesc->uNetBlkParams.sPeapMschapV2.caSsid,
																uNetBlkDesc->uNetBlkParams.sPeapMschapV2.caIdentity,
																uNetBlkDesc->uNetBlkParams.sPeapMschapV2.caPassword);
																//uNetBlkDesc->uNetBlkParams.sPeapMschapV2.caCaCert);
			else

				   lCharsWritten = snprintf(caConfStr, WPA_CONF_BUFFER_MAX_LENGTH, "network={\n"
																	 "\tssid=\"%s\"\n"
																	 "\tscan_ssid=0\n"
																	 "\tkey_mgmt=IEEE8021X\n"
																	 "\teap=PEAP\n"
																	 "\tidentity=\"%s\"\n"
																	 "\tpassword=\"%s\"\n"
																	 "\tca_cert=\"%s\"\n"
																	 "\tphase2=\"auth=MSCHAPV2\"\n"
																	 "\teapol_flags=0\n"
							                                         "}",
																		uNetBlkDesc->uNetBlkParams.sPeapMschapV2.caSsid,
																		uNetBlkDesc->uNetBlkParams.sPeapMschapV2.caIdentity,
																		uNetBlkDesc->uNetBlkParams.sPeapMschapV2.caPassword,
																		uNetBlkDesc->uNetBlkParams.sPeapMschapV2.caCaCert);

			break;
		}

		default:
		{
			TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__ << " INVALID uNetBlkDesc->eMethod = " << uNetBlkDesc->eMethod;
			lCharsWritten = -1;
			break;
		}
	}

	//now test the snprintf operation
	//test the ret val of snprintf
	if(lCharsWritten < 0) {
		TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__ << " snprintf failed. errno = " << errno;

		if(pFile)
			fclose(pFile);
		//we need to rm the file from the file system
		remove(caConfFileName);

		return  STATUS_FAIL;


	}

	//we now have the string to write and the opened file for writing to
	//we start with the common header
	if(fputs(("ctrl_interface=DIR="+MCU_VAR_DIR+"/run/wpa_supplicant\n"+
			"ctrl_interface_group=wheel\n"
			"ap_scan=0\n"
			"fast_reauth=1\n").c_str(), pFile) == EOF) {

		TRACESTR(eLevelInfoNormal) << __FUNCTION__ << " fputs failed. ferror =" << ferror(pFile);

		if(pFile)
			fclose(pFile);
		//we need to rm the file from the file system
		remove(caConfFileName);

		return  STATUS_FAIL;

	}
	//for embedded interface we need to encrypt the caConfStr
	/*if ( strcmp(caConfFilePath,CONF_802_1x_FILES_PATH) != 0)
	{
		std::string srcConfFileStr = caConfStr;
		std::string dstConfFileStr,tmpStr;

		TRACESTR(eLevelInfoNormal) << __FUNCTION__ << " \ncaConfStr before " << caConfStr;

		EncodeHelper eH;
		eH.EncodeAes(srcConfFileStr, dstConfFileStr);
		eH.DecodeAes(dstConfFileStr,tmpStr);

		memset(caConfStr,0,WPA_CONF_BUFFER_MAX_LENGTH);
		//strcpy(caConfStr,dstConfFileStr.c_str());

		strcpy(caConfStr,tmpStr.c_str());

		TRACESTR(eLevelInfoNormal) << __FUNCTION__ << " \ncaConfStr after " << caConfStr;


		srcConfFileStr = caConfStr;
		dstConfFileStr="";


			eH.DecodeAes(srcConfFileStr, dstConfFileStr);

			memset(caConfStr,0,WPA_CONF_BUFFER_MAX_LENGTH);
				strcpy(caConfStr,dstConfFileStr.c_str());

		TRACESTR(eLevelInfoNormal) << __FUNCTION__ << " \nDecode caConfStr AFTER " << caConfStr;


	}*/

	if(fputs(caConfStr, pFile) == EOF) {

		TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__ << " fputs failed.. ferror =" << ferror(pFile);

		if(pFile)
			fclose(pFile);
		//we need to rm the file from the file system
		remove(caConfFileName);

		return  STATUS_FAIL;
	}

	if(pFile) {
		fclose(pFile);
	}

	//for embedded interface we need to encrypt the caConfStr
	if ( strcmp(caConfFilePath,CONF_802_1x_FILES_PATH.c_str()) != 0)
	{
		FILE* pEncFile = NULL;
		FILE* ptempFile = NULL;
		INT8 caConfEncFileName[P802_1x_FILES_PATH_MAX_LENGTH] = {0};
		//validate params
		if(!pathForSecurity)
		{
			TRACESTR(eLevelInfoNormal) << __FUNCTION__ << " BAD PARAM pathForSecurity = NULL";

			return  STATUS_FAIL;
		}


		//create an encrypted file in NFS MCU_MCMS_DIR+/802_1xEmb
		lCharsWritten = snprintf(caConfEncFileName, P802_1x_FILES_PATH_MAX_LENGTH, "%s/%s%d%s", pathForSecurity, P802_1x_WPA_CONF_FILE_NAME_PREFIX, eEthNum, P802_1x_WPA_CONF_FILE_NAME_SUFFIX);
		if(lCharsWritten < 0) {

			TRACESTR(eLevelInfoNormal) << __FUNCTION__ << " snprintf failed. errno = " << errno;
			return  STATUS_FAIL;
		}

		TRACESTR(eLevelInfoNormal) << __FUNCTION__ << " fopen caConfEncFileName for writing " << caConfEncFileName;



		//we shall now attempt to open the file for writing
		if((pFile = fopen(caConfFileName, "r")) == NULL) {

			TRACESTR(eLevelInfoNormal) << __FUNCTION__ << " fopen for reading failed. errno = " << errno;
			return  STATUS_FAIL;
		}



		//we shall now attempt to open the file for writing
		if((pEncFile = fopen(caConfEncFileName, "w")) == NULL) {

			TRACESTR(eLevelInfoNormal) << __FUNCTION__ << " fopen Encrypted failed. errno = " << errno;
			return  STATUS_FAIL;
		}

		EncodeHelper eH;
		STATUS encRes = eH.AesEncryptDecrypt(pFile,pEncFile,E_ENCRYPT);
		TRACESTR(eLevelInfoNormal) << __FUNCTION__ << "AesEncryptDecrypt result  " << encRes;


		if(pFile) {
			fclose(pFile);
		}

		if(pEncFile) {
			fclose(pEncFile);
		}

		// debugging


		/*TRACESTR(eLevelInfoNormal) << __FUNCTION__ << " fopen caConfEncFileName for reading " << caConfEncFileName;

		if((pEncFile = fopen(caConfEncFileName, "r")) == NULL) {

					TRACESTR(eLevelInfoNormal) << __FUNCTION__ << " fopen encrypted failed. errno = " << errno;
					return  STATUS_FAIL;
				}


		if((ptempFile = fopen("MCU_TMP_DIR/aaa.conf", "w")) == NULL) {

				TRACESTR(eLevelInfoNormal) << __FUNCTION__ << " fopen temp failed. errno = " << errno;
				return  STATUS_FAIL;
			}

		encRes = eH.AesEncryptDecrypt(pEncFile,ptempFile,E_DECRYPT);

		TRACESTR(eLevelInfoNormal) << __FUNCTION__ << "AesEncryptDecrypt result  " << encRes;


		if(pEncFile) {
						fclose(pEncFile);
					}

		if(ptempFile) {
						fclose(ptempFile);
					}*/



	}







	return STATUS_OK;
}



/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::Start802_1xConfiguration()
{

	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::Start802_1xConfiguration START";


	CEthernetSettingsConfigList	*pEthList		= m_pProcess->GetEthernetSettingsConfigList();
	CEthernetSettingsConfig		*pCurEthConfig	= NULL;
	DWORD curSlotId = 0,
    	  curPortId = 0;
	eEthPortType curPortType= eEthPortType_Illegal;

	eProductType curProductType  = m_pProcess->GetProductType() ;

	STATUS generateFile = STATUS_FAIL;


	BOOL res = CreateDirectory(CONF_802_1x_FILES_PATH.c_str());
	if (res == FALSE)
		TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__ << " creation of ctrl conf file directoty" << CONF_802_1x_FILES_PATH;



	//create 802.1x directories for embedded
	res = CreateDirectory(CONF_802_1x_EMB_FILES_PATH.c_str());
	if (res == FALSE)
		TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__ << " creation of Emb conf file directory" << CONF_802_1x_FILES_PATH;


	res = CreateDirectory(CONF_802_1x_MEDIA1_FILES_PATH.c_str());
	if (res == FALSE)
		TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__ << " creation of Emb media1 conf file directory" << CONF_802_1x_MEDIA1_FILES_PATH;

	res = CreateDirectory(CONF_802_1x_MEDIA2_FILES_PATH.c_str());
	if (res == FALSE)
		TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__ << " creation of Emb media2 conf file directory" << CONF_802_1x_MEDIA2_FILES_PATH;
	res = CreateDirectory(CONF_802_1x_MEDIA3_FILES_PATH.c_str());
	if (res == FALSE)
		TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__ << " creation of Emb media3 conf file directory" << CONF_802_1x_MEDIA3_FILES_PATH;
	res = CreateDirectory(CONF_802_1x_MEDIA4_FILES_PATH.c_str());
	if (res == FALSE)
		TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__ << " creation of Emb media4 conf file directory" << CONF_802_1x_MEDIA4_FILES_PATH;

	res = CreateDirectory(CONF_802_1x_SWITCH_FILES_PATH.c_str());
	if (res == FALSE)
		TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__ << " creation of Emb media4 conf file directory" << CONF_802_1x_SWITCH_FILES_PATH;


	if (NO == IsTarget())
	{

		//for simulation create 802.1x directories for embedded
		res = CreateDirectory(SIMULATION_CONF_802_1x_EMB_FILES_PATH.c_str());
		if (res == FALSE)
			TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__ << " creation of Emb conf file directory" << CONF_802_1x_FILES_PATH;


		res = CreateDirectory(SIMULATION_CONF_802_1x_MEDIA1_FILES_PATH.c_str());
		if (res == FALSE)
			TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__ << " creation of Emb media1 conf file directory" << CONF_802_1x_MEDIA1_FILES_PATH;

		res = CreateDirectory(SIMULATION_CONF_802_1x_MEDIA2_FILES_PATH.c_str());
		if (res == FALSE)
			TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__ << " creation of Emb media2 conf file directory" << CONF_802_1x_MEDIA2_FILES_PATH;
		res = CreateDirectory(SIMULATION_CONF_802_1x_MEDIA3_FILES_PATH.c_str());
		if (res == FALSE)
			TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__ << " creation of Emb media3 conf file directory" << CONF_802_1x_MEDIA3_FILES_PATH;
		res = CreateDirectory(SIMULATION_CONF_802_1x_MEDIA4_FILES_PATH.c_str());
		if (res == FALSE)
			TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__ << " creation of Emb media4 conf file directory" << CONF_802_1x_MEDIA4_FILES_PATH;

		res = CreateDirectory(SIMULATION_CONF_802_1x_SWITCH_FILES_PATH.c_str());
		if (res == FALSE)
			TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__ << " creation of Emb media4 conf file directory" << CONF_802_1x_SWITCH_FILES_PATH;
	}

	E_802_1xCertificateValidationStatus dummyStatus;
	for (int i=0; i<MAX_NUM_OF_LAN_PORTS; i++)
	{
		pCurEthConfig	= pEthList->GetSpecEthernetSettingsConfig(i);
		if(pCurEthConfig)
		{

			if (pCurEthConfig->Get802_1xAuthenticationProtocol() != eEth802_1xAuthenticationType_Off)
			{

				generateFile = GenerateConfFlie802_1x(pCurEthConfig,dummyStatus);
				if (generateFile != STATUS_OK)
				{
					TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__ << " generateFile failed for  card in slot " << pCurEthConfig->GetSlotId()
							<< " port id  "  << pCurEthConfig->GetPortId()
							<<" port type "  << pCurEthConfig->GetPortType();
					break;
				}
				Start802_1xConfiguration_specificEntry(pCurEthConfig);
			}

		}
	} // end loop





	/*send the req for all the media cards
	for (int i=0 ; i<4 ; i++)
	{
		TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::Start802_1xConfiguration eth0 "
				<< m_mediaCards[i].eEth0En
				<<" eth1 " << m_mediaCards[i].eEth1En
				<<" eth2 " << m_mediaCards[i].eEth2En
				<< " eth3 " << m_mediaCards[i].eEth3En;

	    SendConfig802_1xReqToMplApi(i+1,&(m_mediaCards[i]));
	}

	//send the req to the switch card
	SendConfig802_1xReqToMplApi(FIXED_BOARD_ID_SWITCH,&m_switchCard);*/

	//send the req to the ctrl (self msg)
	//Handle802_1xNewConfReq();

}


void CMcuMngrManager::Send802_1xConfigurationToMplApi(int numOfCards)
{
	//send the req for all the media cards
		for (int i=0 ; i<numOfCards ; i++)
		{
			TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::Start802_1xConfiguration eth0 "
					<< m_mediaCards[i].eEth0En
					<<" eth1 " << m_mediaCards[i].eEth1En
					<<" eth2 " << m_mediaCards[i].eEth2En
					<< " eth3 " << m_mediaCards[i].eEth3En;

		    SendConfig802_1xReqToMplApi(i+1,&(m_mediaCards[i]));
		}

		//send the req to the switch card
		if (YES == IsTarget())
		  SendConfig802_1xReqToMplApi(FIXED_BOARD_ID_SWITCH,&m_switchCard);
		else
			SendConfig802_1xReqToMplApi(FIXED_BOARD_ID_SWITCH_SIM,&m_switchCard);
}



/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::End802_1xConfiguration_specificEntry(CEthernetSettingsConfig	*pCurEthConfig)
{

	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::End802_1xConfiguration_specificEntry START";


	DWORD curSlotId = 0,
			curPortId = 0;
	eEthPortType curPortType = eEthPortType_Illegal;

	eProductType curProductType  = m_pProcess->GetProductType() ;




	DWORD physicalBoardId;
	UINT32 status=STATUS_OK;



	if(pCurEthConfig)
	{

		curSlotId		= pCurEthConfig->GetSlotId();
		curPortId		= pCurEthConfig->GetPortId();

		curPortType    = pCurEthConfig->GetPortType();

		TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::End802_1xConfiguration_specificEntry curSlotId "
						      <<curSlotId << " curPortId "
						      <<curPortId << " curPortType " << curPortType;


		switch (curPortType)
		{
		case eEthPortType_Media:
              {

			physicalBoardId	= m_pSlotsNumConversionTable->GetBoardId(curSlotId, 2);
	  		TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::End802_1xConfiguration_specificEntry physicalBoardId "<<physicalBoardId;

			/*Begin: modified by richer for BRIDGE-13957, 2014.7.25*/
			BOOL bRTM_LAN = NO;
			m_pProcess->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_RMX2000_RTM_LAN, bRTM_LAN);

			//NGB request
			if (m_mcuMngrRmxSystemCardsMode == eSystemCardsMode_mpmrx)  bRTM_LAN = YES;


			BOOL isSeparatedNetworks = NO;
			m_pProcess->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_SEPARATE_NETWORK, isSeparatedNetworks);

			if (eProductTypeRMX2000 == curProductType)
			{
			    //m_mediaCards[curSlotId-1].eEth0En = (UINT32 )FALSE;
				//NS=YES RTM_LAN=YES
				if ( bRTM_LAN == YES) //&& (isSeparatedNetworks == YES))
				{
					TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::End802_1xConfiguration_specificEntry RMX2000 with RTM_LAN YES "<<physicalBoardId;


					if (eEthMediaPortType_Rtm_Isdn ==  pCurEthConfig->GetMediaPortType() )

						m_mediaCards[physicalBoardId-1].eEth2En = (UINT32 )FALSE;
					else //if (eEthMediaPortType_Rtm_Lan ==  pCurEthConfig->GetMediaPortType() )
					{
						if (curPortId == 1)
						   m_mediaCards[physicalBoardId-1].eEth3En = (UINT32 )FALSE;

						if (curPortId == 2)
	        				   m_mediaCards[physicalBoardId-1].eEth2En = (UINT32 )FALSE;
					}





				}
				else
				{
					//toSend = false;
					TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::End802_1xConfiguration_specificEntry we should not be here "<<physicalBoardId;
					return;
				}

			}
			/*End: modified by richer for BRIDGE-13957, 2014.7.25*/
			else
			{

				if (eEthMediaPortType_Rtm_Isdn ==  pCurEthConfig->GetMediaPortType() )
					m_mediaCards[physicalBoardId-1].eEth2En = (UINT32 )FALSE;
				else //if (eEthMediaPortType_Rtm_Lan ==  pCurEthConfig->GetMediaPortType() )
				{
					if (curPortId == 1)
						m_mediaCards[physicalBoardId-1].eEth3En = (UINT32 )FALSE;

					if (curPortId == 2)
						m_mediaCards[physicalBoardId-1].eEth2En = (UINT32 )FALSE;

				}
			}

            switch (physicalBoardId)
            {
            case FIXED_BOARD_ID_MEDIA_1:

            	StartTimer(MCUMNGR_802_1x_WAIT_FOR_MEDIA1_ETH_SET_TIMER,2*SECOND);
            	break;
            case FIXED_BOARD_ID_MEDIA_2:

            	StartTimer(MCUMNGR_802_1x_WAIT_FOR_MEDIA2_ETH_SET_TIMER,2*SECOND);
            	break;
            case FIXED_BOARD_ID_MEDIA_3:

            	StartTimer(MCUMNGR_802_1x_WAIT_FOR_MEDIA3_ETH_SET_TIMER,2*SECOND);
            	break;
            case FIXED_BOARD_ID_MEDIA_4:

            	StartTimer(MCUMNGR_802_1x_WAIT_FOR_MEDIA4_ETH_SET_TIMER,2*SECOND);
            	break;
            default:
            	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::End802_1xConfiguration_specificEntry illegal physicalBoardId "<<physicalBoardId;
            	break;
            }

			break;
          }
		case eEthPortType_Modem:

			physicalBoardId	= m_pSlotsNumConversionTable->GetBoardId(curSlotId, 1);
	  		TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::End802_1xConfiguration_specificEntry physicalBoardId "<<physicalBoardId;


			m_switchCard.eEth0En = (UINT32 )FALSE;

			SendConfig802_1xReqToMplApi(physicalBoardId,&m_switchCard);

			break;


		case eEthPortType_ManagementShelfMngr:
            /*Begin: modified by richer for BRIDGE-13957, 2014.7.25*/
            case eEthPortType_ManagementShelfMngr_Managment:
            /*End: modified by richer for BRIDGE-13957, 2014.7.25*/
		case eEthPortType_ManagementShelfMngr_Managment_Signaling_media:
		case eEthPortType_ManagementShelfMngr_Managment_Signaling :

			physicalBoardId	= m_pSlotsNumConversionTable->GetBoardId(curSlotId, 1);


			if (eProductTypeRMX1500 == curProductType || eProductTypeRMX4000 == curProductType)
				m_switchCard.eEth1En = (UINT32 )FALSE;
			else if (eProductTypeRMX2000 == curProductType)
				m_switchCard.eEth0En = (UINT32 )FALSE;


			SendConfig802_1xReqToMplApi(physicalBoardId,&m_switchCard);
			break;

		case eEthPortType_Management1:
		case eEthPortType_Media_Signaling_Managment:
		case eEthPortType_Media_Signaling:
			if (eProductTypeRMX2000 == curProductType )
			{

				m_cntl.eEth0En = (UINT32 )FALSE;
			}
			//added by Richer for 802.1x project om 2013.12.26 
			else if(eProductTypeNinja == curProductType || eProductTypeGesher == curProductType)
			{
			    if(1 == curPortId)
			    {
			        m_cntl.eEth0En = (UINT32 )FALSE;
			    }
			    else
			    {
			        m_cntl.eEth1En = (UINT32 )FALSE;
			    }
			}
			else
				m_cntl.eEth1En = (UINT32 )FALSE;

			if (!IsValidTimer(MCUMNGR_802_1x_WAIT_FOR_CTRL_ETH_SET_TIMER))
			    KillWpaOnCTRL();
			TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::End802_1xConfiguration_specificEntry MNG start wait timer  ";

			StartTimer(MCUMNGR_802_1x_WAIT_FOR_CTRL_ETH_SET_TIMER,2*SECOND);
			break;

		case eEthPortType_Signaling1:
			if (eProductTypeRMX2000 == curProductType )
			{

				m_cntl.eEth0En = (UINT32 )FALSE;
			}
			else
				m_cntl.eEth2En = (UINT32 )FALSE;


			if (!IsValidTimer(MCUMNGR_802_1x_WAIT_FOR_CTRL_ETH_SET_TIMER))
			    KillWpaOnCTRL();
			TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::End802_1xConfiguration_specificEntry SIG start wait timer  ";

			StartTimer(MCUMNGR_802_1x_WAIT_FOR_CTRL_ETH_SET_TIMER,2*SECOND);

			break;
		default:
			break;

		}  //switch


	}


}



/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::Start802_1xConfiguration_specificEntry(CEthernetSettingsConfig	*pCurEthConfig,bool toSend)
{

	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::Start802_1xConfiguration_specificEntry START";



	DWORD curSlotId = 0,
    	  curPortId = 0;
	eEthPortType curPortType = eEthPortType_Illegal;

	eProductType curProductType  = m_pProcess->GetProductType() ;



	if(pCurEthConfig)
	{
		curSlotId		= pCurEthConfig->GetSlotId();
		curPortId		= pCurEthConfig->GetPortId();

		curPortType    = pCurEthConfig->GetPortType();

		TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::Start802_1xConfiguration_specificEntry curSlotId "
				      <<curSlotId << " curPortId "
				      <<curPortId << " curPortType " << curPortType;

		DWORD physicalBoardId	;


		switch (curPortType)
		{
		case eEthPortType_Media:
		{

			physicalBoardId	= m_pSlotsNumConversionTable->GetBoardId(curSlotId, 2);
			TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::Start802_1xConfiguration_specificEntry physicalBoardId "<<physicalBoardId;


			BOOL bRTM_LAN = NO;
			m_pProcess->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_RMX2000_RTM_LAN, bRTM_LAN);

			//NGB request
			if (m_mcuMngrRmxSystemCardsMode == eSystemCardsMode_mpmrx)  bRTM_LAN = YES;


			BOOL isSeparatedNetworks = NO;
			m_pProcess->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_SEPARATE_NETWORK, isSeparatedNetworks);


			if (eProductTypeRMX2000 == curProductType)
			{
				//NS=YES RTM_LAN=YES
				if ( bRTM_LAN == YES) //&& (isSeparatedNetworks == YES))
				{
					TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::Start802_1xConfiguration_specificEntry RMX2000 with RTM_LAN YES "<<physicalBoardId;


					if (eEthMediaPortType_Rtm_Isdn ==  pCurEthConfig->GetMediaPortType() )

						m_mediaCards[physicalBoardId-1].eEth2En = (UINT32 )TRUE;
					else //if (eEthMediaPortType_Rtm_Lan ==  pCurEthConfig->GetMediaPortType() )
					{
						if (curPortId == 1)
						   m_mediaCards[physicalBoardId-1].eEth3En = (UINT32 )TRUE;

						if (curPortId == 2)
	        				   m_mediaCards[physicalBoardId-1].eEth2En = (UINT32 )TRUE;
					}





				}
				else
				{
					//toSend = false;
					TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::Start802_1xConfiguration_specificEntry we should not be here "<<physicalBoardId;
					return;
				}





			}
			else
			{

				if (eEthMediaPortType_Rtm_Isdn ==  pCurEthConfig->GetMediaPortType() )

					m_mediaCards[physicalBoardId-1].eEth2En = (UINT32 )TRUE;
				else //if (eEthMediaPortType_Rtm_Lan ==  pCurEthConfig->GetMediaPortType() )
				{
					if (curPortId == 1)
					   m_mediaCards[physicalBoardId-1].eEth3En = (UINT32 )TRUE;

					if (curPortId == 2)
        				   m_mediaCards[physicalBoardId-1].eEth2En = (UINT32 )TRUE;

        		}
        	}


        	if ( toSend == true )
        	{
        		   switch (physicalBoardId)
        		            {
        		            case FIXED_BOARD_ID_MEDIA_1:

        		            	StartTimer(MCUMNGR_802_1x_WAIT_FOR_MEDIA1_ETH_SET_TIMER,2*SECOND);
        		            	break;
        		            case FIXED_BOARD_ID_MEDIA_2:

        		            	StartTimer(MCUMNGR_802_1x_WAIT_FOR_MEDIA2_ETH_SET_TIMER,2*SECOND);
        		            	break;
        		            case FIXED_BOARD_ID_MEDIA_3:

        		            	StartTimer(MCUMNGR_802_1x_WAIT_FOR_MEDIA3_ETH_SET_TIMER,2*SECOND);
        		            	break;
        		            case FIXED_BOARD_ID_MEDIA_4:

        		            	StartTimer(MCUMNGR_802_1x_WAIT_FOR_MEDIA4_ETH_SET_TIMER,2*SECOND);
        		            	break;
        		            default:
        		            	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::End802_1xConfiguration_specificEntry illegal physicalBoardId "<<physicalBoardId;
        		            	break;
        		            }
        	}
        	else
        	{
      		   switch (physicalBoardId)
             		            {
             		            case FIXED_BOARD_ID_MEDIA_1:

             		            	StartTimer(MCUMNGR_802_1x_WAIT_FOR_MEDIA1_ETH_SET_TIMER,60*SECOND);
             		            	break;
             		            case FIXED_BOARD_ID_MEDIA_2:

             		            	StartTimer(MCUMNGR_802_1x_WAIT_FOR_MEDIA2_ETH_SET_TIMER,60*SECOND);
             		            	break;
             		            case FIXED_BOARD_ID_MEDIA_3:

             		            	StartTimer(MCUMNGR_802_1x_WAIT_FOR_MEDIA3_ETH_SET_TIMER,60*SECOND);
             		            	break;
             		            case FIXED_BOARD_ID_MEDIA_4:

             		            	StartTimer(MCUMNGR_802_1x_WAIT_FOR_MEDIA4_ETH_SET_TIMER,60*SECOND);
             		            	break;
             		            default:
             		            	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::End802_1xConfiguration_specificEntry illegal physicalBoardId "<<physicalBoardId;
             		            	break;
             		            }
        	}


        	break;
		}
        case eEthPortType_Modem:

        {

        	m_switchCard.eEth0En = (UINT32 )TRUE;

        	if ( toSend == true )
        	{
        		physicalBoardId	= m_pSlotsNumConversionTable->GetBoardId(curSlotId, 1);
          		TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::Start802_1xConfiguration_specificEntry physicalBoardId "<<physicalBoardId;

        		SendConfig802_1xReqToMplApi(physicalBoardId,&m_switchCard);
        	}
        	else
        	 	StartTimer(MCUMNGR_802_1x_WAIT_FOR_SWITCH_ETH_SET_TIMER,60*SECOND);


        	break;
        }


        case eEthPortType_ManagementShelfMngr:
        case eEthPortType_ManagementShelfMngr_Managment:
        case eEthPortType_ManagementShelfMngr_Managment_Signaling_media:
        case eEthPortType_ManagementShelfMngr_Managment_Signaling:
        {



	       //added "eProductTypeNinja" by Richer for 802.1x project om 2013.12.26 
        	if (eProductTypeRMX1500 == curProductType || eProductTypeRMX4000 == curProductType || eProductTypeNinja == curProductType)
        		m_switchCard.eEth1En = (UINT32 )TRUE;
        	else if (eProductTypeRMX2000 == curProductType)
        		m_switchCard.eEth0En = (UINT32 )TRUE;


        	if ( toSend == true )
        	{
        	 	physicalBoardId	= m_pSlotsNumConversionTable->GetBoardId(curSlotId, 1);
          		TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::Start802_1xConfiguration_specificEntry physicalBoardId "<<physicalBoardId;

        		SendConfig802_1xReqToMplApi(physicalBoardId,&m_switchCard);
        	}
        	else
        	    StartTimer(MCUMNGR_802_1x_WAIT_FOR_SWITCH_ETH_SET_TIMER,60*SECOND);

        	break;
        }

        case eEthPortType_Management1:
        {
       // case eEthPortType_Media_Signaling_Managment:

        	//it is only a case of RMX different from RMX2000
        	
        	//added by Richer for 802.1x project om 2013.12.26 
		if(eProductTypeNinja == curProductType || eProductTypeGesher == curProductType)
		{
		    if(1 == curPortId)
		    {
		         m_cntl.eEth0En = (UINT32 )TRUE;
		    }
		    else
		    {
		         m_cntl.eEth1En = (UINT32 )TRUE;
		    }
		}
		else
        	    m_cntl.eEth1En = (UINT32 )TRUE;
			
        	if (!IsValidTimer(MCUMNGR_802_1x_WAIT_FOR_CTRL_ETH_SET_TIMER))
        		KillWpaOnCTRL();

        	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::Start802_1xConfiguration_specificEntry MNG start wait timer  ";

        	StartTimer(MCUMNGR_802_1x_WAIT_FOR_CTRL_ETH_SET_TIMER,2*SECOND);

        	break;
        }

        case eEthPortType_Signaling1:
        case eEthPortType_Media_Signaling:
        {
        	if (eProductTypeRMX2000 == curProductType )
        	{

        		m_cntl.eEth0En = (UINT32 )TRUE;
        	}
        	else
        		m_cntl.eEth2En = (UINT32 )TRUE;



        	if (!IsValidTimer(MCUMNGR_802_1x_WAIT_FOR_CTRL_ETH_SET_TIMER))
        		KillWpaOnCTRL();
        	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::Start802_1xConfiguration_specificEntry SIG start wait timer  ";

        	StartTimer(MCUMNGR_802_1x_WAIT_FOR_CTRL_ETH_SET_TIMER,2*SECOND);


        	break;
        }
        default:
        	break;

        }  //switch


	}


}


/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendConfig802_1xReqToMplApi(DWORD slotId,s802_1x_NEW_CONFIG_REQ *pEth802_1xSettingsConfig)
{


	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::SendConfig802_1xReqToMplApi " ;


	DWORD physicalBoardId	= slotId;



	CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol();

	mplPrtcl->AddCommonHeader(Op802_1x_NEW_CONFIG_REQ);
	mplPrtcl->AddMessageDescriptionHeader();
	mplPrtcl->AddPhysicalHeader(1, physicalBoardId, FIXED_CM_SUBBOARD_ID);
	mplPrtcl->AddData(sizeof(s802_1x_NEW_CONFIG_REQ), (char*)pEth802_1xSettingsConfig);

	CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("MCUMNGR_SENDS_TO_MPL_API");
	mplPrtcl->SendMsgToMplApiCommandDispatcher();

	POBJDELETE(mplPrtcl);

}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::KillWpaOnCTRL()
{

	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::KillWpaOnCTRL START";




	CManagerApi api(eProcessMcuMngr);

	api.SendMsg(NULL, MCUMNGR_Op802_1x_NEW_KILL_WPA_REQ);




}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::Handle802_1xConnectionStatusChangeEvent(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::Handle802_1xConnectionStatusChangeEvent START";

	// ===== 1. get the parameters from the structure received
	//s802_1xConnectionStatusChangeEvent  connection_status_event;
	s802_1x_CONNECTION_STATUS_UNSOLICITED_IND connection_status_event;

	pSeg->Get( (BYTE*)&connection_status_event, sizeof(s802_1x_CONNECTION_STATUS_UNSOLICITED_IND) );

	TRACESTR(eLevelInfoNormal) <<
			"CMcuMngrManager::Handle802_1xConnectionStatusChangeEvent: eEthNum is  "
			<< connection_status_event.ulNicId << "\nStatus is " << connection_status_event.eConnStatus;


}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::Handle802_1xNewConfReq()
{

	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::Handle802_1xNewConfReq START";


	UINT32 ulRc = STATUS_OK;
	INT32 lSysRet = 0;
	INT8 cWpaCommandBuffer[WPA_CMD_LINE_LEN] = {0};
	INT8 cTempLine[WPA_TEMP_BUF_LEN] = {0};
	UINT32 i = 0;


	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::Handle802_1xNewConfReq START"
				<< " eEth0En " << m_cntl.eEth0En
				<< " eEth1En " << m_cntl.eEth1En
				<< " eEth2En " << m_cntl.eEth2En
				<< " eEth3En " << m_cntl.eEth3En ;



	  DWORD eEth0En=m_cntl.eEth0En;
	  DWORD eEth1En=m_cntl.eEth1En;
	  DWORD eEth2En=m_cntl.eEth2En;
	  DWORD eEth3En=m_cntl.eEth3En;


	 /* ulRc = P802_1xKillAllWpaProcs();
	  if(ulRc != STATUS_OK) {
		  TRACESTR(eLevelInfoNormal) <<
				  "CMcuMngrManager::Handle802_1xNewConfReq: failed to P802_1xKillAllWpaProcs " ;
		  return ;
	  }
	  SystemSleep(20);*/

	  TRACESTR(eLevelInfoNormal) <<
				  "CMcuMngrManager::Handle802_1xNewConfReq: after P802_1xKillAllWpaProcs " ;


	  //first we need to see whether the new conf uses 802.1x at all.
	  if(eEth0En== (DWORD)FALSE && eEth1En == (DWORD)FALSE && eEth2En == (DWORD)FALSE && eEth3En == (DWORD)FALSE) {
		  s802_1xManagerCtxObj.ulIsEnabled = FALSE;

	  }
	  else

		  s802_1xManagerCtxObj.ulIsEnabled = TRUE;




	  //now copy the params from the request into the context object

	  strncpy(s802_1xManagerCtxObj.c802_1xConfFilesDirRootPath, CONF_802_1x_FILES_PATH.c_str(), sizeof(s802_1xManagerCtxObj.c802_1xConfFilesDirRootPath)-1);
	  s802_1xManagerCtxObj.sNicDb[E_802_1x_ETH0].ul802_1xEnabled = eEth0En;
	  s802_1xManagerCtxObj.sNicDb[E_802_1x_ETH1].ul802_1xEnabled = eEth1En;
	  s802_1xManagerCtxObj.sNicDb[E_802_1x_ETH2].ul802_1xEnabled = eEth2En;
	  s802_1xManagerCtxObj.sNicDb[E_802_1x_ETH3].ul802_1xEnabled = eEth3En;

	  //now we want to verify that all the configuration files exist
	  for(i = 0;i < P802_1x_MAX_ETH_NUM;i++) {
		  if(s802_1xManagerCtxObj.sNicDb[i].ul802_1xEnabled == TRUE) {
			  if(P802_1xVerifyWpaConfFilePerNic((E_802_1xEth)i, s802_1xManagerCtxObj.c802_1xConfFilesDirRootPath) != STATUS_OK) {
				  TRACESTR(eLevelInfoNormal) <<
						  "CMcuMngrManager::Handle802_1xNewConfReq: failed to P802_1xVerifyWpaConfFilePerNic FAILED for Eth " <<i;
				  //we need to update the internal DB
				  //override the ApiCommand param since it cant be enabled with INVALID conf file/s
				  s802_1xManagerCtxObj.sNicDb[i].ul802_1xEnabled = FALSE;
				  s802_1xManagerCtxObj.sNicDb[i].e802_1xConnState = E_802_1x_STATE_BAD_CONF; //paranoid ;)
				  continue;
			  }
		  }
		  else {
			  TRACESTR(eLevelInfoNormal) <<
					  "CMcuMngrManager::Handle802_1xNewConfReq: skipping conf verification for Eth " <<i << " since its NOT enabled in the ApiCommand ";

			  continue;
		  }
	  }

	  eProductType curProductType  = m_pProcess->GetProductType() ;

	  BOOL isSeparatedNetworks = NO;
	  m_pProcess->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_SEPARATE_NETWORK, isSeparatedNetworks);

	  //now its time to run wpa_supplicant - we need to build the cmd line first -
	  //and after that the wpa_cli instances according to the ApiCommand
	  //Modified "eProductTypeNinja" by Richer for 802.1x project om 2013.12.26 
	  if ( (eProductTypeGesher==curProductType) || (eProductTypeNinja==curProductType))
	  {
	      strncat(cWpaCommandBuffer,("sudo env LD_LIBRARY_PATH="+MCU_MCMS_DIR+"/Bin "+((std::string)GET_MCU_HOME_DIR)+"/usr/sbin/wpa_supplicant -B ").c_str(),
		  	  	  sizeof(cWpaCommandBuffer)-strlen(cWpaCommandBuffer)-1);
	  }
	  else
	  {
	      strncat(cWpaCommandBuffer, (((std::string)GET_MCU_HOME_DIR)+"/usr/sbin/wpa_supplicant -B ").c_str(),
		  	  	  sizeof(cWpaCommandBuffer)-strlen(cWpaCommandBuffer)-1);
	  }
	  
	  for(i = 0;i < P802_1x_MAX_ETH_NUM;i++) {
		  bzero(cTempLine, sizeof(cTempLine));
		  if(s802_1xManagerCtxObj.sNicDb[i].ul802_1xEnabled == TRUE) {
			  if ((i == E_802_1x_ETH0) && (curProductType == eProductTypeRMX2000) && (isSeparatedNetworks == YES))
			  {
				  snprintf(cTempLine, sizeof(cTempLine), "-ieth%d.2198 -c%s/%s%d%s -Dwired -d -N ", i,
				 						  s802_1xManagerCtxObj.c802_1xConfFilesDirRootPath,
				 						  P802_1x_WPA_CONF_FILE_NAME_PREFIX,
				 						  i,
				 						  P802_1x_WPA_CONF_FILE_NAME_SUFFIX);

				  strncat(cWpaCommandBuffer, cTempLine, sizeof(cWpaCommandBuffer)-strlen(cWpaCommandBuffer)-1);
			  }
			  else
			  {
				  snprintf(cTempLine, sizeof(cTempLine), "-ieth%d -c%s/%s%d%s -Dwired -d -N ", i,
						  s802_1xManagerCtxObj.c802_1xConfFilesDirRootPath,
						  P802_1x_WPA_CONF_FILE_NAME_PREFIX,
						  i,
						  P802_1x_WPA_CONF_FILE_NAME_SUFFIX);
				  strcat(cWpaCommandBuffer, cTempLine);
			  }
		  }
	  }
	  //cut off the last -N
	  cWpaCommandBuffer[strlen(cWpaCommandBuffer) - 3] = '\0';

	//we have the command string - lets print it
	TRACESTR(eLevelInfoNormal) <<
			"CMcuMngrManager::Handle802_1xNewConfReq: going to run wpa_supplicant: "<<cWpaCommandBuffer;


	CConfigManagerApi api;
	api.RunCmd(cWpaCommandBuffer);



	//now we need to run the wpa_cli for each NIC
	for(i = 0;i < P802_1x_MAX_ETH_NUM;i++) {
		bzero(cWpaCommandBuffer, sizeof(cWpaCommandBuffer));
		if(s802_1xManagerCtxObj.sNicDb[i].ul802_1xEnabled == TRUE) {
			if ((i == E_802_1x_ETH0) && (curProductType == eProductTypeRMX2000) && (isSeparatedNetworks == YES))
				snprintf(cWpaCommandBuffer, sizeof(cWpaCommandBuffer), "/usr/sbin/wpa_cli -ieth%d.2198 -a%s -B", i, P802_1x_STR_WPA_CLI_ACTION_FILE_PATH);


			else
				//Modified "eProductTypeNinja" by Richer for 802.1x project om 2013.12.26 
				snprintf(cWpaCommandBuffer, sizeof(cWpaCommandBuffer), "sudo /usr/sbin/wpa_cli -ieth%d -a%s -B", i, P802_1x_STR_WPA_CLI_ACTION_FILE_PATH);
			//lets print the line before running it
			TRACESTR(eLevelInfoNormal) <<
					"CMcuMngrManager::Handle802_1xNewConfReq:going to run wpa_cli: "<< cWpaCommandBuffer;



			CConfigManagerApi api;
			api.RunCmd(cWpaCommandBuffer);


		}
	}

	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::Handle802_1xNewConfReq END";


	return ;
}

/////////////////////////////////////////////////////////////////////////////
UINT32 CMcuMngrManager::P802_1xVerifyWpaConfFilePerNic(E_802_1xEth eEthNum, INT8* pcPath)
{
	UINT32 ulRc = STATUS_OK;
	FILE* pFile = NULL;
	INT8 cConfFileName[P802_1x_FILES_PATH_MAX_LENGTH] = {0};

	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::P802_1xVerifyWpaConfFilePerNic START";


	//verify param in range
	if((eEthNum < 0) || (eEthNum >= E_802_1x_INVALID)) {
		TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::P802_1xVerifyWpaConfFilePerNic got INVALID eth number eEthNum "<< eEthNum;
		return STATUS_FAIL;

	}

	//now verify that he wpa_ethX.conf file exists
	snprintf(cConfFileName, sizeof(cConfFileName), "%s/%s%d%s", pcPath, P802_1x_WPA_CONF_FILE_NAME_PREFIX, eEthNum, P802_1x_WPA_CONF_FILE_NAME_SUFFIX);

	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::P802_1xVerifyWpaConfFilePerNic verifying conf file "<<cConfFileName;


	//now verify the fucker
	pFile = fopen(cConfFileName, "r");
	if(pFile == NULL)
	{
		TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::P802_1xVerifyWpaConfFilePerNic verifying to open conf file "<< cConfFileName;

		return STATUS_FAIL;

	}

	if(pFile)
	{
		fclose(pFile);
	}
	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::P802_1xVerifyWpaConfFilePerNic END";
	return ulRc;
}

/////////////////////////////////////////////////////////////////////////////
UINT32 CMcuMngrManager::P802_1xManagerInit()
{
	UINT32 ulRc = STATUS_OK;
	UINT32 i = 0;

	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::P802_1xManagerInit START";

	//zeroize the context
	bzero(&s802_1xManagerCtxObj, sizeof(s802_1xManagerCtxObj));

	//now set the data we know for sure - regardless of whether we run
	//in enabled mode or not

	for(i = 0;i < P802_1x_MAX_ETH_NUM;i++) {
		s802_1xManagerCtxObj.sNicDb[i].ul802_1xEnabled = FALSE;
		s802_1xManagerCtxObj.sNicDb[i].e802_1xConnState = E_802_1x_STATE_DISABLED;
		snprintf(s802_1xManagerCtxObj.sNicDb[i].cEthName, P802_1x_MAX_ETH_NAME_LEN, "eth%d", i);
	}

	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::P802_1xManagerInit END status " << ulRc;
	return ulRc;
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::P802_1xKillAllWpaProcs()
{
	//UINT32 ulRc = STATUS_OK;
	//INT32 lSysRet = -1;

	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::P802_1xKillAllWpaProcs START" ;

       //modified by Richer for 802.1x project om 2013.12.26 
       std::string cmd;
       if ( eProductFamilySoftMcu == CProcessBase::GetProcess()->GetProductFamily())
       {
           cmd="sudo killall wpa_cli ; sudo killall wpa_supplicant";
       }
       else
       {
	    cmd="killall wpa_cli ; killall wpa_supplicant";
       }
	CConfigManagerApi api;
	api.RunCmd(cmd);




	//cmd="killall wpa_cli";
	//api.RunCmd(cmd);


	//TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::P802_1xKillAllWpaProcs END status " << ulRc;

	//return ulRc;
}

void CMcuMngrManager::Handle802_1xNewConfInd(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::Handle802_1xNewConfInd START" ;
	  DWORD eEth0En;
	  DWORD eEth1En;
	  DWORD eEth2En;
	  DWORD eEth3En;

	  *pSeg >> eEth0En
	        >> eEth1En
	        >> eEth2En
	        >> eEth3En;

}

void CMcuMngrManager::OnTimerWaitMedia1EthSetTimeout(CSegment* pSeg)
{
	SendConfig802_1xReqToMplApi(FIXED_BOARD_ID_MEDIA_1,&m_mediaCards[FIXED_BOARD_ID_MEDIA_1-1]);
}
void CMcuMngrManager::OnTimerWaitMedia2EthSetTimeout(CSegment* pSeg)
{
	SendConfig802_1xReqToMplApi(FIXED_BOARD_ID_MEDIA_2,&m_mediaCards[FIXED_BOARD_ID_MEDIA_2-1]);
}
void CMcuMngrManager::OnTimerWaitMedia3EthSetTimeout(CSegment* pSeg)
{
	SendConfig802_1xReqToMplApi(FIXED_BOARD_ID_MEDIA_3,&m_mediaCards[FIXED_BOARD_ID_MEDIA_3-1]);
}
void CMcuMngrManager::OnTimerWaitMedia4EthSetTimeout(CSegment* pSeg)
{
	SendConfig802_1xReqToMplApi(FIXED_BOARD_ID_MEDIA_4,&m_mediaCards[FIXED_BOARD_ID_MEDIA_4-1]);
}

void CMcuMngrManager::OnTimerWaitSwitchEthSetTimeout(CSegment* pSeg)
{
	SendConfig802_1xReqToMplApi(FIXED_BOARD_ID_SWITCH,&m_switchCard);
}

/////////////////////////////////////////////////////////////////////////////
BOOL CMcuMngrManager::IsEdgeAxisMcuInSimulation() const
{
	if (eProductTypeEdgeAxis != m_mcuMngrProductType && eProductTypeCallGeneratorSoftMCU != m_mcuMngrProductType)
		return FALSE;

	std::ostringstream cmd, cmd2, cmd3;

	//cmd << "rpm -q --queryformat '%{BUILDTIME}' `rpm -qa | grep Plcm-SoftMcuMainEdge`";
	cmd << "rpm -qa | grep Plcm-SoftMcuMain";
	std::string ans;
	STATUS stat = SystemPipedCommand(cmd.str().c_str(), ans);
	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::IsEdgeAxisMcuInSimulation - command:answer = <" << cmd.str() << ">:<"<< ans <<">.";

	if (ans.length()==0)
	{
		//TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::IsEdgeAxisMcuInSimulation - no RPM installed - probably working in simulation. return TRUE";
		return TRUE;
	}
	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::RemovePasswordFromString(const char* pszSearchString, char* pszNewString, const char* pszElementOpenTag, const char* pszElementCloseTag)
{


	char* pszStartPassword = (char*)strstr(pszSearchString,pszElementOpenTag);
	if (pszStartPassword)	//if password exist in the transaction
	{

		pszStartPassword += strlen(pszElementOpenTag);
		char *pszEndPassword = (char*)strstr(pszStartPassword,pszElementCloseTag);

		if(pszEndPassword)
		{
			strncpy(pszNewString, pszSearchString, pszStartPassword-pszSearchString);
			pszNewString[pszStartPassword-pszSearchString] = '\0';
			strcat(pszNewString, "*********");
			char* pszStartPassword2 = (char*)strstr(pszEndPassword,pszElementOpenTag);	//look if the password appear twice
			if (pszStartPassword2)
			{
				pszStartPassword2 += strlen(pszElementOpenTag);
				char *pszEndPassword2 = (char*)strstr(pszStartPassword2,pszElementCloseTag);

				if (pszEndPassword && pszEndPassword2)
				{
					strncat(pszNewString, pszEndPassword, pszStartPassword2-pszEndPassword);
					strcat(pszNewString, "*********");
					strcat(pszNewString, pszEndPassword2);
				}
				else
					PTRACE(eLevelError,"CFailoverCommunication::RemovePasswordFromString - either pszEndPassword or pszEndPassword2 is NULL");
			}
			else
				strcat(pszNewString, pszEndPassword);
		}
	}

}

////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendEthSetChangeAuditEvent(CEthernetSettingsConfig* pEthernetSettingsConfigOld,CEthernetSettingsConfig* pEthernetSettingsConfigNew)
{

	    CXMLDOMElement *ethSetBeforeXml = NULL;
	    pEthernetSettingsConfigOld->SerializeXml(ethSetBeforeXml);
	    char *buffBefore = NULL;
	    ethSetBeforeXml->DumpDataAsLongStringEx(&buffBefore, TRUE);

	    int paddingLenB = strlen(buffBefore)+10;

  		ALLOCBUFFER(pszBuffWithShadowPasswordBefore, paddingLenB);
	    memset(pszBuffWithShadowPasswordBefore, 0, paddingLenB);


	    if  (!(strstr(buffBefore, "<PASSWORD></PASSWORD>")) && (strstr(buffBefore, "<PASSWORD>")))  //check that there is password
	    	{

	    		RemovePasswordFromString(buffBefore, pszBuffWithShadowPasswordBefore, "<PASSWORD>", "</PASSWORD>");


	    	}
	    else
	    {
	    	memcpy(pszBuffWithShadowPasswordBefore,buffBefore,strlen(buffBefore));

	    }

	    //change due to CK issue
	    pszBuffWithShadowPasswordBefore[paddingLenB-1] = '\0';

	    CXMLDOMElement *ethSetAfterXml = NULL;
	    pEthernetSettingsConfigNew->SerializeXml(ethSetAfterXml);
	    char *buffAfter = NULL;
	    ethSetAfterXml->DumpDataAsLongStringEx(&buffAfter, TRUE);


	    int paddingLenA = strlen(buffAfter)+10;

        ALLOCBUFFER(pszBuffWithShadowPasswordAfter, paddingLenA);
		memset(pszBuffWithShadowPasswordAfter, 0, paddingLenA);


	    if  (!(strstr(buffAfter, "<PASSWORD></PASSWORD>")) && (strstr(buffAfter, "<PASSWORD>")))  //check that there is password
		    	{

		    		RemovePasswordFromString(buffAfter, pszBuffWithShadowPasswordAfter, "<PASSWORD>", "</PASSWORD>");



		    	}
	    else
	    {
    		memcpy(pszBuffWithShadowPasswordAfter,buffAfter,strlen(buffAfter));

	    }

	    pszBuffWithShadowPasswordAfter[paddingLenA-1] = '\0';

	    AUDIT_EVENT_HEADER_S outAuditHdr;
	    CAuditorApi::PrepareAuditHeader(outAuditHdr,
	                                    "",
	                                    eMcms,
	                                    "",
	                                    "",
	                                    eAuditEventTypeInternal,
	                                    eAuditEventStatusOk,
	                                    "Update Ethernet Settings table",
	                                    "An Ethernet settings record was updated and the Ethernet settings table was upd",
	                                    "",
	                                    "");
	    CFreeData freeData;
	    CAuditorApi::PrepareFreeData(freeData,
	                                 "Previous Parameters",
	                                 eFreeDataTypeXml,
	                                 pszBuffWithShadowPasswordBefore,
	                                 "Updated Parameters",
	                                 eFreeDataTypeXml,
	                                 pszBuffWithShadowPasswordAfter);

	    CAuditorApi api;
	    api.SendEventMcms(outAuditHdr, freeData);

	    DEALLOCBUFFER(pszBuffWithShadowPasswordBefore);
	    DEALLOCBUFFER(pszBuffWithShadowPasswordAfter);

	    delete [] buffBefore;
	    delete [] buffAfter;
	    PDELETE(ethSetBeforeXml);
	    PDELETE(ethSetAfterXml);
}
STATUS CMcuMngrManager::RestartSoftMcuService(eProductType productType) const
{
	COstrStream cmd;


	if (eProductTypeSoftMCUMfw == productType)
		cmd << "/usr/bin/nohup sudo /sbin/service soft_mcu restart";
	else
		cmd << "touch "+MCU_TMP_DIR+"/SERVICE_RESTART.cmd && /usr/bin/nohup "+MCU_UTILS_DIR+"/Scripts/ServiceCntl.sh --service soft_mcu --action restart";

	std::string answer;
	STATUS	stat = SystemPipedCommand(cmd.str().c_str(), answer);

	// temp temp temp - for debugging
//	TRACEINTO	<< "\nCmd:    " << cmd.str().c_str()
//				<< "\nstat:   " << stat << " (" << CProcessBase::GetProcess()->GetStatusAsString(stat) << ")"
//				<< "\nAnswer: " << answer;

	if(STATUS_OK != stat)
	{
		stat = STATUS_FAIL;
	}

	return stat;
}

////////////////////////////////////////////////////////////////////////////
E_802_1xCertificateValidationStatus CMcuMngrManager::ValidateCertificateFor802_1xTLS(char * fname ,char *userName )
{
	std::ostringstream cmd;


	cmd << " openssl x509 -in " << fname << " -text";



	std::string ans;
	STATUS stat = SystemPipedCommand(cmd.str().c_str(), ans);
	TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::ValidateCertificateFor802_1xTLS - command: " << cmd.str() << ":"<< ans.c_str();

	if(!ans.empty())
	{
		if (strstr(ans.c_str(), P802_1x_STR_CERTIFICATE_FILE_NOT_EXIST) != NULL)
			return E_CERTIFICATE_NO_SUCH_FILE;



		bool withBS = true;

		const char CN_PREFIX_WITH_BS[] = "/CN=";
		const char CN_PREFIX_WITHOUT_BS[] = "CN=";



		char* pStartCN = strstr((char *)ans.c_str(),CN_PREFIX_WITH_BS);
		if( NULL != pStartCN )
		{
			withBS = true;
		}
		else
		{

			pStartCN = strstr((char *)ans.c_str(),CN_PREFIX_WITHOUT_BS);
			if( NULL != pStartCN )
			{
				withBS = false;
			}
			else
				return E_CERTIFICATE_NO_COMMON_NAME;
		}



		char charEnd;
		if (withBS)
		{
			pStartCN += strlen(CN_PREFIX_WITH_BS);
			charEnd = '/';
		}
		else
		{
			pStartCN += strlen(CN_PREFIX_WITHOUT_BS);
			charEnd = ',';
		}
		char* pEndCN = pStartCN;

		while( '\0' != *pEndCN  &&  charEnd != *pEndCN )
		{
			pEndCN++;
		}

		int len = pEndCN - pStartCN;

		char* pszClientCertificateCN = new char[len+1];
		strncpy(pszClientCertificateCN,pStartCN,len);
		pszClientCertificateCN[len] = '\0';

		TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::ValidateCertificateFor802_1xTLS - userName: " << userName  << " len "<<strlen(userName);
		TRACESTR(eLevelInfoNormal) << "CMcuMngrManager::ValidateCertificateFor802_1xTLS - pszClientCertificateCN: " << pszClientCertificateCN  << " len "<<strlen(pszClientCertificateCN);


		//cnPtr += strlen(P802_1x_STR_CERTIFICATE_CN);
		if (strcmp(pszClientCertificateCN,userName) != 0)
		{
			PDELETEA(pszClientCertificateCN);
		    //return E_CERTIFICATE_CN_NOT_EQUAL_TO_USERNAME;
		}

		PDELETEA(pszClientCertificateCN);

		if(strstr(ans.c_str(), P802_1x_STR_CERTIFICATE_SANS) == NULL )
			return 	E_CERTIFICATE_DO_NOT_INCLUDE_SANS;

		if (strstr(ans.c_str(), P802_1x_STR_CERTIFICATE_PRINCIPLE) == NULL)
			return E_CERTIFICATE_DO_NOT_INCLUDE_SANS_PRINCIPLE;

	}

	return E_CERTIFICATE_OK;

}

////////////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendSystemBasedModeReqToCardMngr()
{
	PTRACE(eLevelInfoNormal, "CMcuMngrManager::SendSystemBasedModeReqToCardMngr");
	CSegment*  pRetParam = new CSegment;

	const COsQueue* pCardsMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCards, eManager);

	STATUS res = pCardsMbx->Send(pRetParam, MCUMNGR_SYSTEM_CARDS_MODE_REQ);

}

/////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrManager::HandleUpdateLicensingServer(CRequest* pSetRequest)
{
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::HandleUpdateLicensingServer - new licensingServer received";

	STATUS retStatus = STATUS_OK;
	std::string statusDesc = "";

	if (pSetRequest->GetAuthorization() == SUPER)
	{
		// ===== 1. extract keycode from segment received
		CLicensingServer* pLicensingServer = (CLicensingServer*)(pSetRequest->GetRequestObject());

		std::string licensingServer  = pLicensingServer->GetPrimaryLicenseServer();
		DWORD      licensingServerPort = pLicensingServer->GetPrimaryLicenseServerPort();



		std::string oldLicensingServer    =  m_pProcess->GetMcuStateObject()->GetPrimaryLicenseServer();
		DWORD      oldLicensingServerPort =  m_pProcess->GetMcuStateObject()->GetPrimaryLicenseServerPort();

		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::HandleUpdateLicensingServer"
				<< "\nNew Primary Licensing Server: " << licensingServer.c_str()
				<< "\nNew Primary Licensing Server port: " << licensingServerPort
		        << "\nOld Primary Licensing Server: " << oldLicensingServer.c_str()
				<< "\nOld Primary Licensing Server port: " << oldLicensingServerPort;

		//BRIDGE-12051
		if (licensingServer == "")
		{
			retStatus = STATUS_IP_ADDRESS_NOT_VALID;
		}

		 DWORD ifNoneMatch = pSetRequest->GetIfNoneMatch() ;
		if( (ifNoneMatch < MAX_UNSIGN) && (ifNoneMatch != m_pLicensing->GetLicenseServerParams()->GetUpdateCounter()))
		{
			pSetRequest->SetConfirmObject(new CDummyEntry());
			pSetRequest->SetStatus(STATUS_PRECONDITION_FAILED);
			pSetRequest->SetExDescription("Precondition failed");
			return STATUS_OK;

		}

		if ((oldLicensingServer != licensingServer) || (oldLicensingServerPort != licensingServerPort))
		{
			m_pLicensing->GetLicenseServerParams()->IncreaseUpdateCounter();
			pLicensingServer->SetUpdateCounter(m_pLicensing->GetLicenseServerParams()->GetUpdateCounter());
			m_pProcess->GetMcuStateObject()->SetPrimaryLicenseServer(licensingServer);
			m_pProcess->GetMcuStateObject()->SetPrimaryLicenseServerPort(licensingServerPort);
			m_pLicensing->SetLicenseServerParams(pLicensingServer );
			pLicensingServer->WriteXmlFile(LICENSING_CONFIGURATION.c_str());

			//need to send the new data to licenseServer process
			SendLicensingServerParamsInd();
		}

	}

	else // Authorization != SUPER
	{
		retStatus = STATUS_NO_PERMISSION;

		// print to log
		CSmallString traceStr = "\nCMcuMngrManager::HandleUpdateLicensingServer: No permission to update licensing server params;";
		char* authorizationGroupTypeStr = ::AuthorizationGroupTypeToString( pSetRequest->GetAuthorization() );
		if (authorizationGroupTypeStr)
		{
			traceStr << " Authorization Group: " << authorizationGroupTypeStr;
		}
		else
		{
			traceStr << " Authorization Group: (invalid: " << pSetRequest->GetAuthorization() << ")";
		}
		TRACESTR(eLevelInfoNormal) << traceStr.GetString();
	}

	pSetRequest->SetStatus(retStatus);
	pSetRequest->SetConfirmObject(new CDummyEntry());

	return retStatus;
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnLicenseServerParamsReq(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnLicenseServerParamsReq";

	//we got first msg from licenseServer and befor we send the port&ip of server - the connection status is NotAttempt.
	m_pLicensing->SetConnectionStatus(eLicensingConnectionNotAttempt);

	if (YES == m_isAuthenticationIndReceived)
	             SendLicensingServerParamsInd();
	// else I will send it from authenticationInd
}

///////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SendLicensingServerParamsInd()
{
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SendLicensingServerParamsInd";

	LICENSE_SERVER_DATA_S licenseInfo;
    memset(&licenseInfo, 0, sizeof(LICENSE_SERVER_DATA_S));
    CLicensingServer* licensingServerParams               = m_pLicensing->GetLicenseServerParams();


    std::string licensingServer               = licensingServerParams->GetPrimaryLicenseServer();
    memcpy(licenseInfo.primaryLicenseServerHost, licensingServer.c_str(), LICENSE_SERVER_HOST_MAX_LEN);
    licenseInfo.primaryLicenseServerHost[LICENSE_SERVER_HOST_MAX_LEN - 1] = '\0';

	licenseInfo.primaryLicenseServerPort	  = licensingServerParams->GetPrimaryLicenseServerPort();


	GetBoardPartNumber((char *)licenseInfo.mcuHostId , (size_t)(sizeof(licenseInfo.mcuHostId)));

	//set the UUID to be the serial number
	m_pProcess->GetMcuStateObject()->SetMplSerialNumber( licenseInfo.mcuHostId);
	m_pLicensing->SetMplSerialNum( licenseInfo.mcuHostId );

	licenseInfo.mcuVersion                    = m_pSystemVersions->GetMcuVersion();

	DWORD machineCapacity = CalculateMachineCapacity(); //need to calculate the  machin capacity;

	licenseInfo.maxMcuCapacity                = machineCapacity;

    CSegment *pSeg = new CSegment;
    pSeg->Put((BYTE*)&licenseInfo, sizeof(LICENSE_SERVER_DATA_S));

    //  send to manager task
    CManagerApi api(eProcessLicenseServer);
    api.SendMsg(pSeg, LICENSE_SERVER_PARAMS_IND);

}

////////////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnLicenseServerFirstUpdateParamsReq(CSegment* pSeg)
{
	DWORD  structLen = sizeof(FLEXERA_DATA_S);
	FLEXERA_DATA_S serverParams;
	memset(&serverParams, 0, structLen);
	pSeg->Get( (BYTE*)(&serverParams), structLen );


	m_pProcess->SetFlexeraData(&serverParams);


	SetCapabilitiesToLicense();

	CheckLicenseAddActiveAlarmIfNeeded(); //CheckLicenseAddActiveAlarmIfNeeded need to run before we change m_isLicenseEnable to true

	if ((m_isLicenseEnable == false) && (m_pProcess->IsFlexeraCapabilityEnabled(RPCS) == true))

	{

		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnLicenseServerFirstUpdateParamsReq License is ok";

		ContinueStartup_KeycodeValidated();
		m_isLicenseEnable = true;


	}

	//EE-560

    if (m_pProcess->GetFlexeraCapabilityStatus(RPCS) == E_FLEXERA_LICENSE_VALID)
    {
    	m_pLicensing->SetConnectionStatus(eLicensingConnectionSuccess);
    	m_pLicensing->SetLicenseStatus(eLicensingStatusValid);
    }
    else
    {
		m_pLicensing->SetConnectionStatus(eLicensingConnectionFail);
		m_pLicensing->SetLicenseStatus(eLicensingStatusInvalid);

    }



	//check if license expired
	if ((m_pProcess->IsFlexeraCapabilityEnabled(RPCS) == false) && (m_pProcess->GetFlexeraCapabilityStatus(RPCS) == E_FLEXERA_LICENSE_TIME_EXPIRED))

	{
		m_isLicenseExpired = true;
		SendLicensingParamsToRsrcAlloc();
		SendLicensingParamsToConfParty();

	}

	//PrintSpecificFeature(&serverParams);



}

void CMcuMngrManager::OnLicenseServerUpdateParamsReq(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnLicenseServerUpdateParamsReq Start";

	bool isLicenseChanged = FALSE;
	bool isRPCSEnabled    = FALSE;

	DWORD  structLen = sizeof(FLEXERA_DATA_S);
	FLEXERA_DATA_S serverParams;
	memset(&serverParams, 0, structLen);
	pSeg->Get( (BYTE*)(&serverParams), structLen );


	m_pProcess->SetFlexeraData(&serverParams);

	SetCapabilitiesToLicense();

	//CheckLicenseAddActiveAlarmIfNeeded need to run before we change m_isLicenseEnable to true
	isLicenseChanged = CheckLicenseAddActiveAlarmIfNeeded();
	isRPCSEnabled    = m_pProcess->IsFlexeraCapabilityEnabled(RPCS);

	if ((m_isLicenseEnable == false) && (isRPCSEnabled == true))
	{

		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::OnLicenseServerUpdateParamsReq License is ok";

		ContinueStartup_KeycodeValidated();
		m_isLicenseEnable = true;

		m_pLicensing->SetConnectionStatus(eLicensingConnectionSuccess);
	}



	//if expired send ti rsrc and conf
	if ( ((isRPCSEnabled == false) && (m_pProcess->GetFlexeraCapabilityStatus(RPCS) == E_FLEXERA_LICENSE_TIME_EXPIRED)) ||
            //on case we got RPCS false after we had a valid license (m_isLicenseEnable is true) - only when m_isLicenseExpired is false we send the rsrc & conf as if the license expired
			( (isRPCSEnabled == false)  && (m_isLicenseEnable == true) && (m_isLicenseExpired == false)	) ||
			// if m_isLicenseChanged == true we don't want to pass to Rsrc & Conf to remove there restriction cause user see a reboot active alarm
			((isRPCSEnabled == true) && (m_isLicenseExpired == true) && (isLicenseChanged == false)))
	{
		SendLicensingParamsToRsrcAlloc();
		SendLicensingParamsToConfParty();

		if (isRPCSEnabled == true)
		{
			m_isLicenseExpired = false;
			m_pLicensing->SetConnectionStatus(eLicensingConnectionSuccess);
		}
		else
		{
			m_isLicenseExpired = true;
			m_pLicensing->SetConnectionStatus(eLicensingConnectionFail);
		}
	}



	//PrintSpecificFeature(&serverParams);




}



////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::InitLicenseServerParamsFromFile()
{
	bool fileExist = IsFileExists(LICENSING_CONFIGURATION.c_str());

	if (fileExist)
	{
		CLicensingServer * pFlexeraParams = m_pLicensing->GetLicenseServerParams();
		pFlexeraParams->ReadXmlFile(LICENSING_CONFIGURATION.c_str());

		m_pLicensing->SetLicenseServerParams(pFlexeraParams );

		std::string licensingServer  = pFlexeraParams->GetPrimaryLicenseServer();
		DWORD      licensingServerPort = pFlexeraParams->GetPrimaryLicenseServerPort();


		TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::InitLicenseServerParamsFromFile"
				<< "\nPrimary Licensing Server: " << licensingServer.c_str()
				<< "\nPrimary Licensing Server port: " << licensingServerPort;

		m_pProcess->GetMcuStateObject()->SetPrimaryLicenseServer(licensingServer);
		m_pProcess->GetMcuStateObject()->SetPrimaryLicenseServerPort(licensingServerPort);
	}

}

bool CMcuMngrManager::CheckLicenseAddActiveAlarmIfNeeded()
{

	bool isLicenseChanged = false;
	//RPCS ENABLE
	if (m_pProcess->IsFlexeraCapabilityEnabled(RPCS) == true)
	{
		RemoveActiveAlarmByErrorCode(AA_LICENSE_ACQUISITION_FAIL);

		if (m_pProcess->IsFlexeraCapabilityEnabled(RPP_PKG) == false )
		{

			if (m_pProcess->GetFlexeraCapabilityStatus(RPCS) == E_FLEXERA_LICENSE_INSUFFICIENT_RESOURCES )
			{

				if ( false == IsActiveAlarmExistByErrorCode(INSUFFICIENT_RESOURCES) )
					AddActiveAlarm(FAULT_GENERAL_SUBJECT,INSUFFICIENT_RESOURCES,MAJOR_ERROR_LEVEL,"Insufficient resources",true, true);
			}
			else
				RemoveActiveAlarmByErrorCode(INSUFFICIENT_RESOURCES);




			if (m_pProcess->GetFlexeraCapabilityCounted(RPCS_MAX_PORTS) == 0 )
			{
				if ( false == IsActiveAlarmExistByErrorCode(NO_UTILIZABLE_UNIT_FOR_AUDIO_CONTROLLER) )
					AddActiveAlarm(FAULT_GENERAL_SUBJECT,NO_UTILIZABLE_UNIT_FOR_AUDIO_CONTROLLER,MAJOR_ERROR_LEVEL,"No usable unit",true, true);
			}
			else
				RemoveActiveAlarmByErrorCode(NO_UTILIZABLE_UNIT_FOR_AUDIO_CONTROLLER);
		}


		if (m_isLicenseEnable == true)  //BRIDGE-11988
		{
			//AA_LICENSE_WAS_MODIFIED
			for (int i=1 ; i< MAX_NUM_OF_FEATURES;i++) /*do not check IsChanged of RPCS start from the second capability */
			{
				if(m_pProcess->IsFlexeraCapabilityChanged(i) == true)
				{
					if ( false == IsActiveAlarmExistByErrorCode(AA_LICENSE_WAS_MODIFIED) )
					{
						AddActiveAlarm(FAULT_GENERAL_SUBJECT,AA_LICENSE_WAS_MODIFIED,MAJOR_ERROR_LEVEL,"License has been modified. Modified license will take effect after system restart.",true, true);
						isLicenseChanged = true;
						//EE-560
						m_pLicensing->SetLicenseStatus(eLicensingStatusRestartRequired);
					}
					break;
				}

			}
		}

	}
	//RPCS DISABLE
	else
	{

		RemoveActiveAlarmByErrorCode(INSUFFICIENT_RESOURCES);
		RemoveActiveAlarmByErrorCode(AA_LICENSE_WAS_MODIFIED);
		RemoveActiveAlarmByErrorCode(NO_UTILIZABLE_UNIT_FOR_AUDIO_CONTROLLER);


		if ( false == IsActiveAlarmExistByErrorCode(AA_LICENSE_ACQUISITION_FAIL) )
			AddActiveAlarm(FAULT_GENERAL_SUBJECT,AA_LICENSE_ACQUISITION_FAIL,MAJOR_ERROR_LEVEL,"Valid license not found",true, true);

	}


	return isLicenseChanged;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CMcuMngrManager::CalculateMachineCapacity()
{
	WORD machineCapacity = 0;
	WORD cpuCapacity = GetMachineProfileGrade();
    if (!cpuCapacity)
    	return 0;


    if (cpuCapacity < 1 || cpuCapacity > 200) // cpuCapacity must be percent from max capacity, i.e. 1 - 100%
    {
                    TRACESTRFUNC(eLevelWarn) << "WARNING: wrong cpuCapacity value = " << cpuCapacity;
                    return 0;
    }

    machineCapacity = (DWORD)ceil((MAX_NUMBER_HD_AVC_PARTICIPANTS_SOFT_MCU_EDGE*cpuCapacity)/100.);

    TRACEINTO << "INPUT: capacity=" << cpuCapacity << " ===> OUTPUT:  m_CpuCapacity=" << machineCapacity;

    return machineCapacity;



}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::SetCapabilitiesToLicense()
{
	DWORD numOfCPPorts = 0;
	//SVC
	m_pLicensing->SetIsSvcEnabled(m_pProcess->IsFlexeraCapabilityEnabled(RPCS_SVC) );

	DWORD machineCapacity = CalculateMachineCapacity(); //need to calculate the  machin capacity;
	if (m_pProcess->IsFlexeraCapabilityEnabled(RPP_PKG))
		numOfCPPorts  = machineCapacity;
	else
	{
		if (m_pProcess->IsFlexeraSimulationMode())
		{
			numOfCPPorts  = (machineCapacity < m_pProcess->GetFlexeraCapabilityCounted(RPCS_MAX_PORTS))? machineCapacity : m_pProcess->GetFlexeraCapabilityCounted(RPCS_MAX_PORTS);

		}
		else
			numOfCPPorts  = m_pProcess->GetFlexeraCapabilityCounted(RPCS_MAX_PORTS);
	}


	m_pLicensing->SetTotalNumOfCpParties(numOfCPPorts);

	//RPP_PKG
	m_pLicensing->SetIsRppEnabled(m_pProcess->IsFlexeraCapabilityEnabled(RPP_PKG) );

	//MEDIA_ENCRYPTION
	m_pLicensing->SetIsEncryptionEnabled(m_pProcess->IsFlexeraCapabilityEnabled(MEDIA_ENCRYPTION));

	//PSTN
	BYTE isPstn = NO; //(featuresMask & PSTN_MASK ? YES : NO);
	m_pLicensing->SetIsPstnEnabled(isPstn);

	//TELEPRESENCE
	m_pLicensing->SetIsTelepresenceEnabled(m_pProcess->IsFlexeraCapabilityEnabled(RPCS_TELEPRESENCE));



	//HD
	m_pLicensing->SetIsHDEnabled(m_pProcess->IsFlexeraCapabilityEnabled(AVC_CIF_PLUS));

	//MS
	m_pLicensing->SetIsMultipleServicesEnabled(m_pProcess->IsFlexeraCapabilityEnabled(RPCS_MULTIPLE_SERVICES));

	//AVC_CIF_PLUS
	m_pLicensing->SetIsAvcCifPlusEnabled(m_pProcess->IsFlexeraCapabilityEnabled(AVC_CIF_PLUS));
	//TIP
	m_pLicensing->SetIsTipEnabled(m_pProcess->IsFlexeraCapabilityEnabled(TIP));

	//AVAYA
	m_pLicensing->SetIsAvaya(m_pProcess->IsFlexeraCapabilityEnabled(RPCS_AVAYA));

	//IBM
	m_pLicensing->SetIsIBM(m_pProcess->IsFlexeraCapabilityEnabled(RPCS_IBM));

	if (m_pProcess->IsFlexeraCapabilityEnabled(RPCS))
	{
		m_pProcess->IncreaseFlexeraCapabilityExpDateMon(RPCS);

		CStructTm expDate = m_pProcess->GetFlexeraCapabilityExpDate(RPCS);
	    m_pLicensing->SetExpirationDate(expDate);
		m_pProcess->GetMcuStateObject()->SetExpirationDate(expDate);

	}
	else
	{
		CStructTm defaultValue;
		m_pLicensing->SetExpirationDate(defaultValue);
		m_pProcess->GetMcuStateObject()->SetExpirationDate(defaultValue);
	}



}

char const * CMcuMngrManager::GetBoardPartNumber(char * buf, size_t len)
   {
       return ReadFileContent("/sys/class/dmi/id/product_uuid", buf, len);
   }

char const * CMcuMngrManager::ReadFileContent(char const * fn, char * buf, size_t len)
    {
        char bufCmdLine[128];
        snprintf(bufCmdLine, sizeof(bufCmdLine), "sudo cat %s 2>/dev/null", fn);
        FILE * fp = popen(bufCmdLine, "r");

        if (fp)
        {
            size_t bytes = fread(buf, 1, len, fp);
            int offset = (bytes>=len) ? len-1 : bytes;
            buf[bytes] = 0;

            {
                int offset = strlen(buf)-1;
                while (offset>=0 && isspace(buf[offset]))
                {
                    buf[offset] = 0;
                    --offset;
                }
            }

            pclose(fp);
        }
        else
        {
            buf[0] = 0;
        }
        return buf;
    }

void CMcuMngrManager::OnLicenseServerConnectionStatusInd(CSegment* pSeg)
{
	DWORD  type= 0;
	*pSeg >> type;

	eLicensingConnectionStatus connectionStatus= (eLicensingConnectionStatus)type;
	m_pLicensing->SetConnectionStatus(connectionStatus);

}



void CMcuMngrManager::OnLicenseServerTimeInd(CSegment* pSeg)
{
	DWORD  type= 0;

	DWORD  year= 0;
	DWORD  mon = 0;
	DWORD  day = 0;

	DWORD  hour= 0;
	DWORD  min = 0;
	DWORD  sec = 0;

	*pSeg >> type
	      >> year
	      >> mon
	      >> day
	      >> hour
	      >> min
	      >> sec;

	CStructTm rcvTime;
	rcvTime.m_year = year;
	rcvTime.m_mon  = mon;
	rcvTime.m_day  = day;

	rcvTime.m_hour = hour;
	rcvTime.m_min  = min;
	rcvTime.m_sec  = sec;

	eLicensingConnectionTime connectionTime= (eLicensingConnectionTime)type;

	if (connectionTime == eLicensingConnectionLastAttemptTime)

	   m_pLicensing->SetLastAttemptDate(rcvTime);

	else if (connectionTime == eLicensingConnectionSuccessTime)

	   m_pLicensing->SetLastSuccesfulDate(rcvTime);

}
/////////////////////////////////////////////////////////////////////////////
void CMcuMngrManager::OnInstallerAuthenticationStructReq(CSegment* pSeg)
{
	CMedString retStr = "\nCMcuMngrManager::OnInstallerAuthenticationStructReq - ";
	if (YES == m_isAuthenticationIndReceived)
	{
		retStr << "Sending to Installer";
		SendAuthenticationStructToSpecificProcess(eProcessInstaller);

	}
	else
	{
		retStr << "AuthenticationInd was not received; nothing is being sent to Installer";
	}

	TRACESTR(eLevelInfoNormal) << retStr.GetString();
}

