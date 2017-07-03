// McuMngrManager.h

#ifndef MCU_MNGR_MANAGER_H_
#define MCU_MNGR_MANAGER_H_

#include <openssl/ssl.h>


#include "ManagerTask.h"
#include "ProcessBase.h"
#include "Macros.h"
#include "SharedMcmsCardsStructs.h"
#include "Authentication.h"
#include "IpParameters.h"
#include "SoftwareLocation.h"
#include "DefinesGeneral.h"
#include "McuMngrDefines.h"
#include "KeyCode.h"
#include "SetMcuRestore.h"
#include "ConfigManagerApi.h"
#include "MacAddressConfig.h"
#include "EthernetSettingsConfig.h"
#include "ApacheConfig.h"
#include "StringsLen.h"
#include "AllocateStructs.h"
#include "PrecedenceSettings.h"
#include <fstream>
#include "DefinesGeneral.h"

#include "802_1xApiReq.h"
#include "IpService.h"
#include "LicensingServerStructs.h"

class CMcuMngrProcess;
class CMplMcmsProtocol;
class CIPSpan;
class CProcessStateFaultListMap;
class CProcessStateFaultList;
class CLicensing;
class CVersions;
class CSystemTime;
class CTerminalCommand;
class CPortSpeed;
class CLogFltElement;
class CSlotsNumberingConversionTableWrapper;
class CSysConfigEma;





/*
typedef struct {
	APIU32 ulEnabled;
	UINT32 eEth0En;
	UINT32 eEth1En;
	UINT32 eEth2En;
	UINT32 eEth3En;
} s802_1x_NEW_CONFIG_REQ;

typedef enum {
	E_802_1x_STATE_DISCONNECTED,
	E_802_1x_STATE_CONNECTING,
	E_802_1x_STATE_CONNECTED,
	E_802_1x_STATE_BAD_CONF,
	E_802_1x_STATE_DISABLED,
	E_802_1x_STATE_INVALID
} E_802_1xConnState;


typedef enum {
	E_802_1x_METHOD_PEAP_MSCHAPV2,
	E_802_1x_METHOD_EAP_TLS,
	E_802_1x_METHOD_EAP_MD5,
	E_802_1x_METHOD_MAX
} E_802_1xMethod;


typedef enum {
	E_802_1x_PORT_STATUS_AUTHORIZED,
	E_802_1x_PORT_STATUS_UNAUTHORIZED,
	E_802_1x_PORT_STATUS_INVALID
} E_802_1xSuppPortStatus;


typedef enum {
	E_802_1x_EAP_STATE_IDLE,
	E_802_1x_EAP_STATE_FAILURE,
	E_802_1x_EAP_STATE_SUCCESS,
	E_802_1x_EAP_STATE_INVALID
} E_802_1x_EapState;



//request from MCMS to manager to query the status of a spesific NIC
//when requesting info about 802.1x, there is a seperate request per NIC -
//mimicing the model of EthHandling
typedef struct {
	E_802_1xEth eNicId;
} s802_1x_CONNECTION_STATUS_REQ;




//the username can be up to 256 chars
#define P802_1x_USER_NAME_MAX_LENGTH (256)

//the passwd field can be up to 256 UNICODE (16bit) chars
#define P802_1x_PASSWD_MAX_LENGTH (256 * 2)

#define P802_1x_SSID_MAX_LENGTH (128)

//4 is the number of eth's in the NGB
#define P802_1x_MAX_ETH_NUM (4)

//this is max length for the path of the root dir of the 802.1x conf files
//its used by the managers and by the conf generator
#define P802_1x_FILES_PATH_MAX_LENGTH (512)

#define P802_1x_WPA_CONF_FILE_NAME_PREFIX "wpa_eth"
#define P802_1x_WPA_CONF_FILE_NAME_SUFFIX ".conf"
*/
/************ WPA defines ******************/
#define WPA_CONF_BUFFER_MAX_LENGTH (4096)

#define WPA_CMD_LINE_LEN (2048)
#define WPA_TEMP_BUF_LEN (128)

#define P802_1x_STR_CERTIFICATE_CN         "CN="
#define P802_1x_STR_CERTIFICATE_SANS       "Subject Alternative Name"
#define P802_1x_STR_CERTIFICATE_PRINCIPLE  "othername:<unsupported>"

#define P802_1x_STR_CERTIFICATE_FILE_NOT_EXIST  "No such file"

#define P802_1x_STR_WPA_CLI_ACTION_FILE_PATH "./wpa_action.sh"

#define MAX_UNSIGN 4294967295u


typedef enum {
	E_CERTIFICATE_OK,
	E_CERTIFICATE_DO_NOT_INCLUDE_SANS,
	E_CERTIFICATE_DO_NOT_INCLUDE_SANS_PRINCIPLE,
	E_CERTIFICATE_CN_NOT_EQUAL_TO_USERNAME,
	E_CERTIFICATE_NO_SUCH_FILE,
	E_CERTIFICATE_NO_COMMON_NAME,
	E_CERTIFICATE_INVALID
} E_802_1xCertificateValidationStatus;


//these are the possible events generated by wpa_supplicant and sent
//to wpa_cli (and then passed to the application)
/*typedef enum {
	E_802_1x_STATE_DISCONNECTED,
	E_802_1x_STATE_CONNECTED,
	E_802_1x_STATE_BAD_CONF,
	E_802_1x_STATE_DISABLED,
	E_802_1x_STATE_INVALID
} E_802_1xConnState;


typedef enum {
	E_802_1x_METHOD_PEAP_MSCHAPV2,
	E_802_1x_METHOD_EAP_TLS,
	E_802_1x_METHOD_EAP_MD5,
	E_802_1x_METHOD_MAX
} E_802_1xMethod;


typedef enum {
	E_802_1x_PORT_STATUS_AUTHORIZED,
	E_802_1x_PORT_STATUS_UNAUTHORIZED,
	E_802_1x_PORT_STATUS_INVALID
} E_802_1xSuppPortStatus;


typedef enum {
	E_802_1x_EAP_STATE_IDLE,
	E_802_1x_EAP_STATE_FAILURE,
	E_802_1x_EAP_STATE_SUCCESS,
	E_802_1x_EAP_STATE_INVALID
} E_802_1x_EapState;

*/


typedef struct {
	INT8 caSsid[P802_1x_SSID_MAX_LENGTH];
	INT8 caIdentity[P802_1x_USER_NAME_MAX_LENGTH];
	INT8 caPassword[P802_1x_PASSWD_MAX_LENGTH];
} s802_1xCg_PEAP_EAP_MD5_Params;


typedef struct {
	INT8 caSsid[P802_1x_SSID_MAX_LENGTH];
	INT8 caIdentity[P802_1x_USER_NAME_MAX_LENGTH];
	INT8 caPassword[P802_1x_PASSWD_MAX_LENGTH];
	INT8 caCaCert[P802_1x_FILES_PATH_MAX_LENGTH];
} s802_1xCg_PEAP_MSCHAPV2_Params;


typedef struct {
	INT8 caSsid[P802_1x_SSID_MAX_LENGTH];
	INT8 caIdentity[P802_1x_USER_NAME_MAX_LENGTH];
	INT8 caCaCert[P802_1x_FILES_PATH_MAX_LENGTH];
	INT8 caClientCert[P802_1x_FILES_PATH_MAX_LENGTH];
	INT8 caPrivateKey[P802_1x_FILES_PATH_MAX_LENGTH];
	INT8 caPrivateKeyPass[P802_1x_FILES_PATH_MAX_LENGTH];
} s802_1xCg_EAP_TLS_Params;


typedef union {
	s802_1xCg_PEAP_EAP_MD5_Params sMd5;
	s802_1xCg_PEAP_MSCHAPV2_Params sPeapMschapV2;
	s802_1xCg_EAP_TLS_Params sEapTls;
} u802_1xCg_NetworkBlockParams;


typedef struct {
	E_802_1xMethod eMethod;
	u802_1xCg_NetworkBlockParams uNetBlkParams;
} u802_1xCgNetworkBlockDescriptor;






#define P802_1x_MAX_ETH_NAME_LEN (16)
#define P802_1x_MAX_CONN_STATE_LEN (16)



typedef struct {
	UINT32 ul802_1xEnabled; //is the port 802.1x managed at all
	E_802_1xConnState e802_1xConnState;
	INT8 cEthName[P802_1x_MAX_ETH_NAME_LEN];
} s802_1xNicDbEntry;

typedef struct {
	UINT32 ulIsEnabled;
	s802_1xNicDbEntry sNicDb[P802_1x_MAX_ETH_NUM];
	INT8 c802_1xConfFilesDirRootPath[P802_1x_FILES_PATH_MAX_LENGTH];
} s802_1xManagerCtx;















enum eValidationFailureType
{
	eX_KeycodeValidationFailure = 0,
	eU_KeycodeValidationFailure,
	eBothKeycodesValidationFailure,
	eLicensingFileError,
	eGeneratedKeyCodeMismatch,
	eMplAuthenticationFailure,
	eVersionValidationFailure
};

enum eIpPartToConfigType
{
	eIpPartToConfig_Nothing		= 0,
	eIpPartToConfig_All,
	eIpPartToConfig_Dns,
	eIpPartToConfig_WithoutCsReset
};

enum eIsDhcpEnabled
{
	eDhcpEnaled = 0,
	eDhcpDisabled
};

enum eEntitytToConfigIp
{
	eEntitytToConfigIp_None = 0,
	eEntitytToConfigIp_Apache,
	eEntitytToConfigIp_SSHD,
	eEntitytToConfigIp_Both,

	NUM_OF_ENTITIES_TO_CONFIG_IP
};

static const char *entityToConfigIpStr[] =
{
    "None",				// eNone
    "Apache",  			// eApache
    "SSHD",				// eSSHD
    "Apache and SSHD",	// eBoth
};

enum eIPv6AutoConfigState
{
	eIPv6AutoConfig_NotStarted,
	eIPv6AutoConfig_InProgress,
	eIPv6AutoConfig_Success,
	eIPv6AutoConfig_Failed,

	NUM_OF_IPV6_AUTO_CONFIG_STATES
};

static const char *ipV6autoConfigStateToStr[] =
{
	"not started",
	"in progress",
	"success",
	"failed"
};

typedef struct
{
	DWORD   iPv4;
	char	iPv4_str[IP_ADDRESS_LEN];
	DWORD	iPv4_shelf;
	DWORD	iPv4_defGW;
	char 	iPv4_netMask[IPV6_ADDRESS_LEN];

	char	iPv6_str[IPV6_ADDRESS_LEN];
	char	iPv6_NoBrackets[IPV6_ADDRESS_LEN];
	char	iPv6_shelf[IPV6_ADDRESS_LEN];
	char	iPv6_defGW[IPV6_ADDRESS_LEN]; // no brackets
	char 	iPv6_netMask[IPV6_ADDRESS_LEN];
	eV6ConfigurationType	ipV6configType;

	eIpType ipType;

} IP_ADDR_S;




enum WhiteListStatus{WHITELIST_NO_CHANGE,WHITELIST_CHANGE,WHITELIST_CHANGE_NORESET,WHITELIST_FAILED};

class CMcuMngrManager : public CManagerTask
{
CLASS_TYPE_1(CMcuMngrManager, CManagerTask)
public:
	CMcuMngrManager();
	virtual ~CMcuMngrManager();

	virtual const char* NameOf() const { return "CMcuMngrManager";}
	virtual bool IsResetInStartupFail() const {return true;}
	virtual void  SelfKill();

    STATUS TurnSsh(bool isOn,
                   bool isStartup=false);

	void ManagerInitActionsPoint();
	void ManagerPostInitActionsPoint();

	TaskEntryPoint GetMonitorEntryPoint();

    virtual int GetTaskMbxBufferSize() const {return 4096*1024-1;}
    virtual int GetTaskMbxThreshold() const {return 3500*1024-1;}

    void RetrieveIpAddresses(CIPService* pService, IP_ADDR_S & ipAddrS);
    void   InitNetworkInterfaceHelperFailRead(STATUS retStatus);
	STATUS InitNetworkInterfaceWithMcmsNetwork(CIPService& tmpService,STATUS readFileStatus);
	void UpdateSecurityDependencies(const CIPService &theService);
	STATUS UpdateNetworkInterface( const string theCaller,
								   const CIPService &theService,
			                       eMngmntIpUpdatePhase updatePhase,
			                       eIpTypeConfigSuccess ipTypeToConfig=eIpTypeConfigSuccess_Both );

	STATUS UpdateDnsForService(CIPService &theService);


	void   SendMngmntUpdateToMpl();
	void   SendConfBlockToConfParty(BYTE confBlockReason = eConfBlockReason_McuMngr_Position_1);

	void   SendSystemIsOutOfStartupToSpecProcess(eProcessType processType);

	void   SendMngmntIpParamsUpdateReqToMplApi(IP_PARAMS_S* pIpParamsStruct);

	void ConfigBondingInterface(BOOL bLAN_REDUNDANCY);
	BOOL IsEnableBonding();

	STATUS ConfigEthernetInOS(CIPService *pService);
	STATUS ConfigDnsInOS(CIPService *pService);
	
	eConfigInterfaceType GetMngrIfType();

	// Task action table
	void   OnMplApiMsg(CSegment* pSeg);
	void   OnMplMsgWhileNotREADY(CSegment* pSeg);
	void   OnAuthenticationInd(CSegment* pSeg);
	void   OnEnterDiagnosticsInd(CSegment* pSeg);
	void   OnMngmntIpConfigInd(CSegment* pSeg);
	void   OnMngmntIpParamsUpdateInd(CSegment* pSeg);
	void   OnSoftwareLocationInd(CSegment* pSeg);
	void   OnFaultGeneralInd(CSegment* pSeg);
    void   OnRestoreFactoryInd(CSegment* pSeg);
    void   OnOutOfSecureModeInd(CSegment* pSeg);

	void   OnProcessStateChanged(CSegment* pSeg);
	void   OnCsNumOfPortsReq(CSegment* pSeg);
	void   OnDnsAgentConfigurationReq(CSegment* pSeg);
	void   OnSipProxyConfigurationReq(CSegment* pSeg);
	void   OnInstallerCfsParamsReq(CSegment* pSeg);
	void   OnApacheModuleAuthenticationStructAndMultipleServicesReq(CSegment* pSeg);


	void OnNewCoreDumpInd(CSegment* pSeg);
	void OnMediaRecordingReq(CSegment* pSeg);
	void OnCollectingInfoReq(CSegment* pSeg);
	void OnBackupInProgressReq(CSegment* pSeg);
	void OnRestoreInProgressReq(CSegment* pSeg);
	void OnBackupTimeout(CSegment* pSeg);
	void OnRestoreTimeout(CSegment* pSeg);
	void OnGetMcuVersionReq(CSegment* pSeg);
	void OnGKMngmntReq(CSegment* pSeg);
	void OnMccfMngmntReq(CSegment* pSeg);
//	void OnSNMP_ManagmentInterfaceIpRequest(CSegment* pMsg);
	void OnSNMP_upInd(CSegment* pMsg);
	void OnSNMP_downInd(CSegment* pMsg);
	void OnQA_Api_ManagmentInterfaceIpRequest(CSegment* pMsg);
	void SendMNGMNTInfoToQA_Api(OPCODE opcode);
	void OnUpdateCertificateInd(CSegment* pMsg);
    void OnResetRequest(CSegment * pSeg);
	void OnSystemCardsModeInd(CSegment* pMsg);
	void OnShmCompSlotIdInd(CSegment* pMsg);
	void OnResourceSystemRamSizeReq(CSegment* pSeg);		// should be united to a single
	void OnResourceSystemCPUProfileReq(CSegment* pSeg);
	void OnConfPartySystemRamSizeReq(CSegment* pSeg);	//    method: OnSystemRamSizeReq
	void SendSystemCPUProfileToResourceMngr();
	void OnInstallerKeycodeUpdateInd(CSegment* pSeg);
	void OnUpdateSystemStartupDurationInd(CSegment* pSeg);
	void OnFailoverConfigReq(CSegment* pSeg);
	void OnFailoverUpdateMngmntInd(CSegment* pSeg);
	void OnFailoverUpdatePairIPInd(CSegment* pSeg);
	void OnFailoverEventTriggerInd(CSegment* pSeg);


	void UpdateSyncedFields(CIPService *pServiceFromFailoverTask, CIPService *pUpdatedService);
	void PerformUpdateSyncedMngmntProcedures(CIPService *pUpdatedService);

	void OnEthSettingConfigInd(CSegment* pSeg);
	void OnSlotsNumberingConversionTableInd(CSegment* pSeg);
	void SendMNGMNTInfoToCertMngr();
  void OnEncryptionKeyServerFips140TestResultInd(CSegment* pSeg);
  void OnRtmLanAndIsdnSlotInd(CSegment* pMsg);
  void OnRtmLanInd(CSegment* pMsg);
  void OnV35GwUpdateInd(CSegment* pMsg);
  void OnReset(CSegment *pSeg);	// for Call Generator
	void OnFailoverSlaveBecomeMasterInd(CSegment *pSeg);
	void OnNotifMngrMcuStateReq(CSegment* pSeg);
	void OnNotifMngrAAListReq(CSegment* pMsg);
	void OnRequestPrecedenceSettings(CSegment* pMsg);

	STATUS HandleSetCfg(CRequest *pRequest);
	STATUS HandleSetCfgParam(CRequest *pRequest);
	STATUS HandleSetMngmntNetwork(CRequest* pSetRequest);
  STATUS HandleSetTime(CRequest *pRequest);
  STATUS HandleSetPrecedenceSettings(CRequest* pRequest);
  STATUS HandleSetEthernetSettings(CRequest *pRequest);
	STATUS HandleSetMcuRestore(CRequest *pRequest);
  STATUS HandleTurnSsh(CRequest *pRequest);
  STATUS HandleReset(CRequest *pRequest);
  STATUS HandleIpmiEntityReset(CRequest *pRequest);

  bool   WasServiceChanged(const CIPService* service);
  bool   WasSecuredTheOnlyChange(const CIPService* service);
  bool   WasWhiteListOnlyChange(const CIPService* service);
  void   ConfigGeneral(CIPService* pService=NULL);
  bool   RetrieveIpV6AutoAndConfig(CIPService& tmpService);
  void   OnTimerIpV6AutoConfig(CSegment* pSeg);
  void   UpdateMemory_IPv6Auto(CIPService& tmpService);
  void   RetrieveIpV6Addresses(IpV6AddressMaskS pOutAddress[], CIPService& tmpService);
  STATUS HandleGetMcuStateEx(CRequest *pRequest);

	void   HandleFirstNtpSync();
	bool   IsValidNtpServersAddresses(CSystemTime* pTime);
	BOOL   IsSysTimeNtpEnabled();
	void   SampleNtpIfNeeded(BOOL isOriginalNtpEnabled, BOOL isNewNtpEnabled);
	STATUS CheckDuplicateIpCntrlAndShelf(const IP_ADDR_S & ipAddrS);
	

	STATUS CheckDuplicateIpIfConfigList(std::string strIfName, std::string ipv4Addr);
	STATUS CheckDuplicateIpCntrlAndOther(const IP_ADDR_S& ipAddrS);
	bool   IsArpingNeeded(const char *newIpAddressStr);

	void	SetCntrlIPv6AddressInAutoMode(IpV6AddressMaskS & IpV6);
	void	SetCntrlIPv6AddressInAutoMode(IpV6AddressMaskS IpV6_S[]);
	void	SetDefGwAddressInAutoMode();
	void	SetDefGwInAutoMode();
	void	SetDefGwMaskInAutoMode();

	void   SendResetAllMfaBoardsReqToMplApi();
	void   SendResetMngmntParamsReqToMplApi();
	void   SendResetUsrListParamsReqToMplApi();
	void   SendResetAllParamsReqToMplApi();
	void   SendRestoreFactoryDefaultReqToMplApi(eMcuRestoreType restoreType);

	STATUS StopCheckEth0Link();
	STATUS StartCheckEth0Link();

	eProductType	ConvertPlatformTypeToProductType(ePlatformType platformType);
	void			ValidateProductType();

	void OnTemporaryTimer();
	const char*  GetMngmntIpUpdatePhaseAsString(eMngmntIpUpdatePhase updatePhase);
	const char*  GetEntityToConfigIpAsString(eEntitytToConfigIp entityToConfig);
	const char*  GetIpTypeConfigSuccessAsString(eIpTypeConfigSuccess configSuccessType);
	eIpTypeConfigSuccess GetTotalIpTypeConfigSuccess(STATUS ipV4Status, STATUS ipV6Status, eIpType ipType);

	STATUS OnCsIpServiceParamsInd(CSegment* pMsg);
	void   SendGmtOffset();
	void   EnableHDLicenseInNon1500Q();
	void   UpdateHDLicensePortsIn1500Q();

	STATUS SetSeparateManagementNetworkFlag(CSysConfigEma* cfgToUpdate, CSysConfigEma* cfgToCheck, CRequest* pRequest);
	
	STATUS CheckLanRedundancy(CSysConfigEma* sys_cfg, CRequest* pRequest);
	
	STATUS CheckMultipleServices(CSysConfigEma* sys_cfg, CRequest* pRequest);
    STATUS isSystemFlagConflict(CSysConfigEma* sys_cfg, const std::string &key, CRequest* pRequest);
void   CheckDNSStateMcmsNetwork();
    STATUS HandleUpdateLicensingServer(CRequest* pSetRequest);
    void   OnLicenseServerParamsReq(CSegment* pSeg);
    void   SendLicensingServerParamsInd();
    void   OnLicenseServerFirstUpdateParamsReq(CSegment* pSeg);
    void   OnLicenseServerUpdateParamsReq(CSegment* pSeg);
    void   OnLicenseServerConnectionStatusInd(CSegment* pSeg);
    void   OnLicenseServerTimeInd(CSegment* pSeg);

    STATUS HandleMediaMtuSize(CSysConfigEma* cfgEMA);
    
protected:
	static BYTE CheckIfToGoBackToPort80(STATUS status);
    //static void SendRestartMCMSMsg();
	STATUS ValidateKeycode_FromMpl(CSmallString serialNumStr, VERSION_S  keyCodeVersion);
	BOOL   IsKeycodeExistsInMpl(/*char keycodeType*/);
	BOOL   IsKeycodeReset(/*char keycodeType*/);
	void   ContinueStartup_KeycodeValidated();

	void   SetMcuStateValidationFailure(eValidationFailureType failureType);
	void   SendAuthenticationFailureToMplApi();
	void   SendAuthenticationAckToMplApi();
	void   SendInitKeycodeFailureIndToInstaller();
	void   SendAuthenticationStructToProcesses();
	void   SendAuthenticationStructToSpecificProcess(eProcessType processType);
	void   SendAuthenticationSuccessToProcesses();
	void   SendAuthenticationSuccessToCardsProcess();
	void   SendAuthenticationSuccessToAuthenticationProcess();
	void   SendRMXTimeSetToAuthenticationProcess();
	void   SendDnsConfigStatusToDnsAgent();
	void   SendDnsConfigStatusToSipProxy();
	void   SendLicensingParamsToProcesses();
	void   SendLicensingParamsToRsrcAlloc();
	void   SendLicensingParamsToConfParty();
	void   SendMultipleServicesToProcesses();
	void   SendMultipleServicesToAuthentication();
	void   SendMultipleServicesToApache();
	void   SendMultipleServicesToCSManager();
	void   SendMultipleServicesToRsrcAlloc();
	void   SendMultipleServicesToCertMngr();
	void   SendMultipleServicesToSipProxy();
	void   SendCfsParamsToInstaller();
    void   SendLicensingParamsToGK();
    void   SendLicensingParamsToLogger();
    void   SendLicensingParamsToApacheModuleMngr();
    void   SendSystemRamSizeToSpecProcess(eProcessType processType);
    void   SendLicensingParamsToRtmIsdnMngr();
    void   SendConfigEthernetSettingsReqToMplApi(CEthernetSettingsConfig *pEthSettingsConfig);
    void   sendEthernetSettingDataToConfigurator(CEthernetSettingsConfig *pEthSettingsConfig);

    void   SendV35GwParamsReqToCsMngr();
    void   SendMcuStateToNotificationMngr();
    void   SendAAListToNotificationMngr();
    void   SendSecurityPKIToDependedProcesses();
    BYTE   GetMultipleServices();
    BYTE   GetV35JITCSupport();
    BYTE   GetSeparateNetworksSupport();
    BYTE   GetRtmLanSupport();

    void   ConfigEthernetCpuAndSwitch();
    void   ConfigEthernet_allEntities();
    void   ConfigEthernet_specEntity(const DWORD slotId, const DWORD portId);
    CEthernetSettingsConfig	* GetEthernetSettings_specEntity(const DWORD slotId, const DWORD portId);
    bool   IsCpuEthPort(const eEthPortType portType);
    STATUS ConfigureEthernetSettingsSpec(CEthernetSettingsConfig *pEthSettingsConfig);
    STATUS ConfigureEthernetSettingsCpu(CEthernetSettingsConfig *pEthSettingsConfig);
    STATUS ConfigureEthernetSettingsCpu(const eConfigInterfaceType ifType, const ePortSpeedType portSpeed);

    void   SendLicensingParamsToCards();
    int    GetEthernetSettingPos(int slot_id);
    int    GetEthernetSettingMSLanPortsPos(int slot_id);

    void   FindChangesInAlarms(CProcessStateFaultList *pNewFaultsList,
                               vector<CLogFltElement>& outNewAlarmVector,
                               vector<CLogFltElement>& outDeletedAlarmVector);

	eIpPartToConfigType WhatIpPartShouldBeConfigured(CIPService &service1, CIPService &service2, BOOL &bResetApache);
	BOOL   IsDhcpEqual(CIPService &service1, CIPService &service2);
	BOOL   IsIpParamsEqual(CIPService &service1, CIPService &service2);
	BOOL   IsDnsParamsEqual(CIPService &service1, CIPService &service2);
	BOOL   IsRoutersEqual(CIPService service1, CIPService service2);
	BOOL   IsSecuredEqual(CIPService service1, CIPService service2);
	BOOL   IsPermenentNetworkOpenEqual(CIPService service1, CIPService service2);

	void		RecalculateMcuState();
	eMcuState	SetMcuStateAccordingToWorstProcessState();
	void		AddFaultListToProcessStateMap(CProcessStateFaultList* pfaultsList);
	void		ChangeLedsState(eMcuState oldMcuState, eMcuState newMcuState);
    STATUS  	HandleTerminalGetActiveAlarms(CTerminalCommand & command, std::ostream& answer);
    STATUS 		HandleTerminalSetMcuType(CTerminalCommand & command, std::ostream& answer);
	STATUS		UpdateActiveAlarmsList_inProcess();

	void PrintDnsConfigurationStatusToTrace();

	void SendTimeConfigReqToMplApi(CSystemTime* pTime, BOOL isInit=NO);
	void PrintTimeConfigSentToMplApi(NTP_REQUEST_S ntpTime);
	void SendDebugModeToShelfMngr();
	void SendNewSysCfgParamsToCards(const std::string& strDebugModedata,
									const std::string& strJITCdata,
									const std::string& strSeparatedNetworksdata,
									const std::string& strMultipleServicesdata) const;
	void SendInfoToSwitchAfterSwitchReset();

	BYTE GetDataForProxy(string &external_content_dir, string &external_content_ip, DWORD &external_content_port);


	STATUS ValidateDnsValues(CIPService* pService, const IP_ADDR_S & ipAddrS,STATUS& warningStatus);
	STATUS CheckSecurity(CIPService* pService);
	void UpdateSslProtocol(const string file_content, std::string& new_file_content, int &pos_end);

	void   RemoveTLSActiveAlarms();
	void   SampleNtpPeerStatus();
	STATUS GetNtpPeerStatus(const char* ptr_peer_ip);
	std::string GetIpFromDnsAddress(std::string dnsAddress);
	STATUS StopSoftNtpStatus();
	STATUS StartSoftNtpServer();
	STATUS ConfigSoftNtp(std::vector< string > &ntp_servers);
	void   SyncNtp(BOOL isTimeAlreadyUpdated = FALSE, CStructTm *timeBeforeUpdate = NULL);
	void   CheckMcuTimeYearValidity();
	BOOL   IsPeerStatusFailed(STATUS serverStatus);
	void   ProduceNtpPeerStatusFaultIfNeeded( BOOL isServer0_failed, BOOL isServer1_failed, BOOL isServer2_failed,
			std::string server0_ipStr, std::string server1_ipStr, std::string server2_ipStr );
	void   ProduceNtpServerFailureFullFault(int serverIdx, std::string ipStr);
	void   SendNtpEventToAuditor(CStructTm timeBefore, CStructTm timeAfter) const;

	void   OnTimerCertificatesDailyTimeout(CSegment* pSeg);
	void   OnTimerWaitForCardsStartupTimeout(CSegment* pSeg);
	void   OnTimerWaitMedia1EthSetTimeout(CSegment* pSeg);
	void   OnTimerWaitMedia2EthSetTimeout(CSegment* pSeg);
	void   OnTimerWaitMedia3EthSetTimeout(CSegment* pSeg);
	void   OnTimerWaitMedia4EthSetTimeout(CSegment* pSeg);
	void   OnTimerWaitSwitchEthSetTimeout(CSegment* pSeg);

	void   OnTimerNtpServersStatusTimeout(CSegment* pSeg);
	void   OnTimerNtpSyncTimeout(CSegment* pSeg);
	void   OnTimerStartupTimeout(CSegment* pSeg);
  void   OnRestoreFactoryTimeout(CSegment* pSeg);
  void   OnNetworkConfigFailureTimeout(CSegment* pSeg);
	void   OnTimerSystemStartupRemainingTime();
	void   OnSNMPConfigInd(CSegment* seg);
	void OnTimerSnmpReady(CSegment* pSeg);
	void OnTimerFailedRemoveAA(CSegment* pSeg);

	STATUS OnIpServiceParamInd(CSegment* pMsg);
	STATUS OnCsDeleteIpeServiceParamsInd(CSegment* pSeg);
	STATUS OnKillSshd(CSegment* pSeg);
	BOOL   CheckIfDefaultGWIPV6IsValid(CIPService *pUpdatedIpService);

	BOOL IsSystemFlagTrue(const char* key) const;
	BOOL IsFederalOn(void) const;

	// DHCP + DNS

	void   EnableAccessOnNetworkConfigFailure(CIPService* pService);
	void   UpdateShelfMngrInfoIfNeeded( const string theCaller,
										CIPService *pNewService,
										const IP_ADDR_S & ipAddrS,
										eMngmntIpUpdatePhase updatePhase,
										bool updateControl );
	void   TreatDnsConfigurationNewStatus( const eDnsConfigurationStatus newStatus, BOOL isToRemoveDnsAlarm=NO);
	STATUS SampleDhcpIp();
	void   OnTestDhcpIpTimeout();
	void   UpdateDhcpParamsMembers();
	STATUS RegisterDnsClient(CIPService *pService, BOOL isDnsAuto=NO);

	void ConfigApache(const CIPService& service);
  void ConfigApache(eIpType ipType = eIpType_IpV4,
                    const std::string& IpAddressToConfig_ipV4 = "",
                    const std::string& IpAddressToConfig_ipV6 = "",
                    BOOL isPermanentNetworkOpen = TRUE,
                    BOOL isSecured = FALSE,
                    BOOL isRequestPeerCertificate = FALSE,
                    BYTE revocationMethodType = eNoneMethod,
                    BOOL isUseResponderOcspURI = TRUE,
                    BOOL isIncompleteRevocation = TRUE,
                    BOOL isSkipValidateOcspCert = TRUE ,
                    std::string	ocspGlobalResponderURI = "",
                    std::string hostname ="");

  void RetrieveRouterList(CIPService *pService, std::list<CRouter> & routerList);

  void PrintIpAddressesConfigured( string &retStr,
	                                 eEntitytToConfigIp entityToConfig=eEntitytToConfigIp_Apache,
	                                 eIpType ipType=eIpType_IpV4,
	                                 char* ipAddrV4="", char* ipAddrV6="" );

	STATUS StartDnsConfiguration(CIPService *pService, eMngmntIpUpdatePhase updatePhase, bool updateInterface = true);
	STATUS ConfigDnsAuto(CIPService *pService);
	STATUS ConfigDnsSpecify(CIPService *pService,eMngmntIpUpdatePhase updatePhase =eMngmntIpInit);
	void   ConfigSSHD(bool isPermanentNetwork, std::string strIpV4Address="", std::string strIpV6Address="");
	void   ConfigSSHD(CIPService* pService);
	STATUS ConfigDnsOff();
	void   SendStateRequestToAllProcesses();

	void UpdateLicensingWithParamsFromKeyCodes();
	void UpdateLicensingWithParamsFromFile();//UpdateLicensingWithParamsFrom_U_KeyCode();
	void UpdateLicensingWithParamsFrom_X_KeyCode();
private:
	void RemovePasswordFromString(const char* pszSearchString, char* pszNewString, const char* pszElementOpenTag, const char* pszElementCloseTag);

	bool IsSoftBasedMcu() const;
	bool IsSoftMcu() const;
	bool IsPlatformOfNewArch() const;
	void UpdateLicensingWithParamsFrom_X_KeyCodeOnTraditionalPlatforms();
	void UpdateLicensingWithParamsFrom_X_KeyCodeOnNewPlatforms();
	void CfgParamChanged(CCfgData* cfgData);
	APIU32 GetValidatedDscpValue();
	void SendParamsToCdrManager();

	void InitLicenseServerParamsFromFile();
	bool CheckLicenseAddActiveAlarmIfNeeded();
	void SetCapabilitiesToLicense();

	void  OnInstallerAuthenticationStructReq(CSegment* pSeg);
	char const * GetBoardPartNumber(char * buf, size_t len);
	char const * ReadFileContent(char const * fn, char * buf, size_t len);

	void SetMFWMngNetworkExDiscription(STATUS nStatus, CRequest *pRequest, CIPService *pIpService);

protected:
	void UpdateLicensingWithBIOSParams();
	// terminal commands
	STATUS HandleTerminalMcuVer(CTerminalCommand & command,std::ostream& answer);
	STATUS HandleTerminalMcuState(CTerminalCommand & command,std::ostream& answer);
	STATUS HandleTerminalAllAA(CTerminalCommand & command,std::ostream& answer);

	STATUS HandleTerminalGenerate_X_Kc(CTerminalCommand & command,std::ostream& answer);
	STATUS HandleTerminalGenerate_U_Kc(CTerminalCommand & command,std::ostream& answer);
	STATUS HandleTerminalLicensingInfo(CTerminalCommand & command,std::ostream& answer);
  STATUS HandleTerminalStartDHCP(CTerminalCommand & command, std::ostream& answer);
  STATUS HandleTerminalStopDHCP(CTerminalCommand & command, std::ostream& answer);
  STATUS HandleTerminalGetDHCP(CTerminalCommand & command, std::ostream& answer);
  STATUS HandleTerminalUpdateDns(CTerminalCommand & command, std::ostream& answer);
  STATUS HandleTerminalSetRestore(CTerminalCommand & command, std::ostream& answer);
  STATUS HandleTerminalResetMngmntParams(CTerminalCommand & command, std::ostream& answer);
  STATUS HandleTerminalResetUsrListParams(CTerminalCommand & command, std::ostream& answer);
  STATUS HandleTerminalResetAllParams(CTerminalCommand & command, std::ostream& answer);
  STATUS HandleTerminalTurnSshOn(CTerminalCommand & command, std::ostream& answer);
 	STATUS HandleTerminalCreateStatusFile(CTerminalCommand & command, std::ostream& answer);
  STATUS HandleTerminalSendIpIndSnmp(CTerminalCommand & command, std::ostream& answer);
 	STATUS HandleTerminalConfigApache(CTerminalCommand & command, std::ostream& answer);
 	STATUS HandleTerminalRestartApache(CTerminalCommand& cmd, std::ostream& ans);
  STATUS HandleTerminalCreateFaultAndAAFile(CTerminalCommand & command, std::ostream& answer);
  STATUS HandleTerminalStartResetTimer(CTerminalCommand & command, std::ostream& answer);
  STATUS HandleTerminalChangeLedState(CTerminalCommand & command, std::ostream& answer);
  STATUS HandleTerminalDisableWhiteList(CTerminalCommand & command, std::ostream& answer);
  STATUS HandleTerminalSetEthSetting(CTerminalCommand & command, std::ostream& answer);
  //void sendSelfMsgToCreateConfFile(CEthernetSettingsConfig *pEthSettingsConfig);
  // for Call Generator - net_config.sh
	STATUS HandleTerminalUpdateNetworkConfiguration(CTerminalCommand & command, std::ostream& answer);
  BOOL   IsRestoreDefaultsStartup() const { return m_isRestoreDefaultsStartup; }
  STATUS HandleTerminalMacAddressConfig(CTerminalCommand & command, std::ostream& answer);
	STATUS SendSyncNumOfConfsMsgToConfPartyProcess();
	STATUS OnUpdateCollectingStop(CSegment* pSeg);

	DWORD convertSlotIdPortIdToServiceId(DWORD slotid ,DWORD pqid);
	DWORD  findSigServiceIdNotMS();
	E_802_1xEth ConvertPortToEthName(CEthernetSettingsConfig *pCurEthConfig);
	STATUS       GenerateConfFlie802_1x(CEthernetSettingsConfig *pCurEthConfig,E_802_1xCertificateValidationStatus &certValidStat);
	const char *  convertPortTypeToConfPath(DWORD slotId , eEthPortType portType);
	const char *  convertPortTypeToConfPathForEmbSecurity(DWORD slotId , eEthPortType portType);
	STATUS       CG802_1xGenerateWpaSuppConfFile(const char* caConfFilePath, E_802_1xEth eEthNum, u802_1xCgNetworkBlockDescriptor* uNetBlkDesc);

	STATUS       CG802_1xGenerateWpaSuppConfFile(const char* caConfFilePath,const char* pathForSecurity, E_802_1xEth eEthNum, u802_1xCgNetworkBlockDescriptor* uNetBlkDesc);
	void         CG802_1xSetCACertFile_CHAP(eEthPortType portType , u802_1xCgNetworkBlockDescriptor* uNetBlkDesc);
	void         CG802_1xSetCACertFile_TLS(eEthPortType portType , u802_1xCgNetworkBlockDescriptor* uNetBlkDesc);
	void         CG802_1xSetClientCertFile(eEthPortType portType , u802_1xCgNetworkBlockDescriptor* uNetBlkDesc);
	void         CG802_1xSetPrivateKeyFile(eEthPortType portType , u802_1xCgNetworkBlockDescriptor* uNetBlkDesc);


	E_802_1xCertificateValidationStatus         ValidateCertificateFor802_1xTLS(char * fname ,char *userName );

	void         Start802_1xConfiguration();
	void         Send802_1xConfigurationToMplApi(int numOfCards);
	//void         StartOREnd802_1xConfiguration_specificEntry(CEthernetSettingsConfig	*pCurEthConfig);
	void         Start802_1xConfiguration_specificEntry(CEthernetSettingsConfig	*pCurEthConfig,bool toSend=false);
	void         End802_1xConfiguration_specificEntry(CEthernetSettingsConfig	*pCurEthConfig);
	void         SendConfig802_1xReqToMplApi(DWORD slotId,s802_1x_NEW_CONFIG_REQ *pEth802_1xSettingsConfig);
	void         KillWpaOnCTRL();
	void         Handle802_1xNewConfReq();  //req from CTRL
	void         Handle802_1xNewConfInd(CSegment* pSeg);
	void         Handle802_1xConnectionStatusChangeEvent(CSegment* pSeg);
	UINT32 P802_1xVerifyWpaConfFilePerNic(E_802_1xEth eEthNum, INT8* pcPath);
	UINT32 P802_1xManagerInit();
	void P802_1xKillAllWpaProcs();
	STATUS RestartSoftMcuService(eProductType productType) const;

	//int m_bogoMipsValue;
	//void setBogmipsValue(int val){m_bogoMipsValue = val;};
    //std::string GetHostNameFromMngmnService(const CIPService* pMngmtService);

protected:
	CAuthentication m_authentication;

	BYTE m_switchBoardId;
	BYTE m_switchSubBoardId;
	BYTE m_cpuBoardId;
	BYTE m_cpuSubBoardId;

	CMcuMngrProcess*	m_pProcess;
	CIPService*			m_pMngmntIpParams_asIpService_fromProcess;     // stored at Process level,
	CFaultList*			m_pActiveAlarmList_fromProcess;
	CSoftwareLocation   m_softwareLocation;
	CVersions*			m_pSystemVersions;
	CMacAddressConfig*	m_pMacAddressConfig;

	// Processes and MCU states
	CProcessStateFaultListMap* m_pProcessesStatesMap;

	// Licensing
	CLicensing*  m_pLicensing;

	// DHCP
	string		m_dhcp_ipStr;
	string		m_dhcp_dnsServerIpStr;
	string		m_dhcp_MaskStr;
	string		m_dhcp_GwStr;
	int			m_dhcpIpTestCounter;
	int			m_dhcpDnsTestCounter;
	CIPService*	m_pDefaultMngmntIpService;

	CKeyCode     m_X_KeyCode;
	bool		 m_isKeyCodeReset;

	BOOL   m_isSystemTarget;	// Target or Pizza
	BOOL   m_isLegalU_keycode;
	BOOL   m_isAuthenticationIndReceived;
	eIpTypeConfigSuccess   m_isIpConfigInOsSucceeded;
	BOOL   m_isDnsConfigInOsSucceeded;
	eIPv6AutoConfigState	m_IpV6AutoConfigState; // the status of auto configuration ipv6
	int	   m_IpV6AutoConfigAccumulatedTime;
	BOOL   m_isMngmntIpConfigIndReceived;
	BOOL   m_isStartupCondition_Dns_added;
	BOOL   m_isStartupTimoutReached;
	BOOL   m_isNtpSyncLegal;
	DWORD  m_NtpServersStatusPeriod;
	DWORD  m_NtpSyncPeriod;
	BOOL   m_isActiveAlarmNtpPeerAlreadyProduced[NTP_MAX_NUM_OF_SERVERS];
	BOOL   m_isUserChangedManagementIp;
    BOOL   m_isRestoreDefaultsStartup;
    BOOL   m_isRestoreIndBlock;
	BOOL   m_isConfBlock;
	BOOL   m_isRmxTimeChangedByUser;
	int	   m_numOfDnsEnable;

	ASN1_TIME* m_cert_start_date;
	ASN1_TIME* m_cert_expiration_date;

	IpV6AddressMaskS	m_cntrlIPv6AddressInAutoMode[MNGMNT_NETWORK_NUM_OF_IPV6_ADDRESSES];
	char				m_defGwInAutoMode[IPV6_ADDRESS_LEN];
	DWORD				m_defGwMaskInAutoMode;

	CIPService* m_pServiceForAccessOnNetConfigFailure;

	BOOL 	m_bRestoreConfigSuccess;
	//BOOL	m_bRestoreNoMngmntIpInterface_AA;
	BOOL	m_bRestoreNoDnsConfiguration_AA;
	BOOL	m_bRestoreDnsRegistrationFailed_AA;

	BOOL	m_bIsV35GwEnabledInService;
	std::string	m_sInternalGwAddress;

	BOOL	m_firstTimeReqForOutOfSecured;
	//added by huiyu 2011.07.13
	string	m_szPairIPFailover;
	BOOL	m_bTriggerFailover;

	BOOL	m_bNotificationMngrReady;
	BOOL IsStartupFinished() const;
	BOOL OnUpdateCpuInfo(CSegment* pMsg) const;
	BOOL OnLoggerMngrCsLogsConfigInd(CSegment* pMsg);

private:

/*	void GetDateFromString(std::string date, CStructTm& rpm_date);
	int getMonth(const std::string& month);
	int get_int( const string& s , int& string_end_pos);*/

    WhiteListStatus HandleWhiteList(CIPService *pService,STATUS& ref);
    void SendWhiteListChangeAuditEvent(CIPService *pService);
    void SendEthSetChangeAuditEvent(CEthernetSettingsConfig* pEthernetSettingsConfigOld,CEthernetSettingsConfig* pEthernetSettingsConfigNew);
	static void DeleteConfigFiles(void);

	virtual void ManagerStartupActionsPoint();
	virtual void DeclareStartupConditions();
	void DeclareStartupConditionsDependencies();
	virtual void AddFilterOpcodePoint();

  STATUS CheckForValidCertificate(const char* host_name,
                                    bool add_aa,
                                    bool ca_validation,
                                    BYTE revocation_method);

	void CheckRunningVersionVsCurrent();
	void CheckDMA();
	void CheckEthernet();

	STATUS PerformRestoreFactoryDefaults(eMcuRestoreType restoreType,
                                         string &answer,
                                         bool FromSwitch = false);
	void ResetCfsKeycodes();
	void ResetCfsKeycodeSpecific(char keycodeType);

	void SetDefaultMNGMNTService();

	void SendMNGMNTServiceToGK() const;
//	void SendMNGMNTInfoToSnmp(OPCODE opcode) const;
	void SendMNGMNTInfoToProcess(eProcessType process, OPCODE opcode) const;

	void CreateCfgVisibleParams();
	void CheckDebugMode();
    void SetKernelTCPParams();

	void SetSecureMcuState(eMcuState state);
	void TreatBadStateManagement(eMcuState oldMcuState, eMcuState newMcuState, DWORD numOfAA)const;
	bool IsPortSpeedDifferent(const CPortSpeed *leftPortSpeed, const CPortSpeed *rightPortSpeed);

    bool IsShelfChanged(CIPService &service1, CIPService &service2);
    void SendUSBChangeAuditEvent(eIpPartToConfigType whatToConfig, bool isShelfChanged, CIPService &service1, CIPService &service2);

    void StartResetTimer(const string & resetDesc, int numOfSeconds);
    void SendSystemIsOutOfStartupToFailoverProcess();
    STATUS HandleNetworkSeparationConfigurations(CIPService Service);

    void SendSecurityMode(eProcessType process) const;

    void SendIpServiceParamsReqToCS();

    void SendQosManagementDSCPToProcess(std::string signalingDCSP, eProcessType processType);
    void ConfigQosManagementNetwork();
    void SendQosManagementDSCPToMplApi();
    void SendPrecedenceSettingsToProcess(CPrecedenceSettings* pPrecedence, eProcessType processType);

    void OnMplSecurityNotEqual(BOOL securityMode);
    BOOL OnMplIpParametersToService(CSegment* pSeg,CIPService& mplService);
    void OnMplConfigIndChangeMcmsNetwork(CSegment* pSeg);

    STATUS CheckUltraSecuredFlagAndSetOtherFlagsIfNeeded(CSysConfigEma *pSysConfigSet);

    void PerformOutOfSecureMode();

    int  GetNumberOfDefinedNtpServers();

    void OnEncryptionKeyServerIsNtpSyncLegalReq(CSegment* pSeg);

    BOOL ValidRpmDate();
    bool IsWhiteListEnabled();
    bool IsOneCertificate();
    bool IsSkipCertificateValidation();
    STATUS NtpSetServer(CSystemTime* pTime);

    std::string GetSystemHostName();
    void SetSystemHostName(const string & host_name);
    BOOL IsEdgeAxisMcuInSimulation() const;

    void SendSystemBasedModeReqToCardMngr();

    DWORD CalculateMachineCapacity();
	
   // void SetIPSpanParams(CIPService& tmpService, IP_ADDR_S& ipAddrS);
   // void SetIPv4SpanParams(CIPService& tmpService, IP_ADDR_S& ipAddrS);
    //void SetIPv6SpanParams(CIPService& tmpService, IP_ADDR_S& ipAddrS);
    //void SetIPv6SpanParamsManual(CIPService& tmpService, IP_ADDR_S& ipAddrS);
   // void SetIPv6SpanParamsAuto(CIPService& tmpService, IP_ADDR_S& ipAddrS);

    BOOL GetServerStatus(std::string *server_ipStr, int serverInd, int num_of_defined_ntp_servers, CLargeString& resStr);

	eDnsConfigurationStatus m_dnsConfigurationStatus;
	BOOL                    m_isDnsApplicabilityAlreadySentToSipProxy;

	// worst state of process (for inner caculation of McuState)
	eProcessStatus m_worstProcessSate;
	eProcessType  m_worstProcessType;

	DWORD m_NtpFailureCounter;

	DWORD m_SnmpTaskId;
	BYTE  m_isSNMPup;
	BYTE  m_isSNMPReady;
	BYTE   m_snmpExtraTimeForStartupLaunch;
	DWORD	m_minimumDelaySnmpTimer;

    eProductType		m_mcuMngrProductType;
    eSystemCardsMode	m_mcuMngrRmxSystemCardsMode;
    CApacheConfig       m_apacheConfig;
	CSlotsNumberingConversionTableWrapper* m_pSlotsNumConversionTable;

	IP_SERVICE_MCUMNGR_S m_StaticIpServiceList[MAX_NUMBER_OF_SERVICES_IN_RMX_4000];

	BOOL 	m_bCSLogStarted;

	s802_1x_NEW_CONFIG_REQ m_mediaCards[4];
	s802_1x_NEW_CONFIG_REQ m_switchCard;
	s802_1x_NEW_CONFIG_REQ m_cntl;

	//flexera license
	BOOL                   m_isLicenseEnable;
	BOOL                   m_isLicenseExpired;

protected:
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY
	PDECLAR_TERMINAL_COMMANDS
};

#endif  // MCU_MNGR_MANAGER_H_
