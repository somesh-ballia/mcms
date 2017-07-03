// CSMngrManager.h

#ifndef CS_MNGR_MANAGER_H_
#define CS_MNGR_MANAGER_H_

#include <map>
#include <string>
#include "ManagerTask.h"
#include "CsStructs.h"
#include "ConfigManagerApi.h"
#include "IpService.h"
#include "SNMPDefines.h"
#include "PrecedenceSettings.h"

class CIPService;
class CCSMngrManager;
class CCommCardService;
class CCommConfService;
class CCommMcuService;
class CCommStartupService;
class CCommRsrcService;
class CCommProxyService;
class CCommIceService;
class CCommDnsAgentService;
class CCommGKService;
class CCommMcuMngrService;
class CCSMngrMplMcmsProtocolTracer;
class CCSMngrMessageValidator;
class CTerminalCommand;
class CCommSnmpService;
class CCfgData;
class CCsPinger;
class CIPServiceList;
class CCommCertMngrService;
class CCommTCPDumpService;

enum eProcessesToSendIpList
{
  eToSendIpList_Cards = 0,
  eToSendIpList_ConfParty,
  eToSendIpList_Resource,
  eToSendIpList_Proxy,
  eToSendIpList_Gk,
  eToSendIpList_CertMngr,
  eToSendIpList_McuMngr,
  eToSendIpList_TCPDump,
  eToSendIpList_Ice,

  NUM_OF_PROCESSES_TO_SEND_IP_LIST
};

enum eOperationOnService
{
  eOperationOnService_Add = 0,
  eOperationOnService_Delete,
  eOperationOnService_Update,

  NUM_OF_OPERATIONS_ON_SERVICE
};

typedef struct
{
  DWORD board_id;
  DWORD sub_board_id;
  DWORD service_id;
  DWORD link_status;
} MEDIA_BOARD_STATUS;

void CSMngrManagerEntryPoint(void* appParam);


class CCSMngrManager : public CManagerTask
{
  CLASS_TYPE_1(CCSMngrManager, CManagerTask)
  friend class CTestCSMngrManager;

public:
  CCSMngrManager();
  virtual ~CCSMngrManager();

  const char*     NameOf(void) const           {return "CCSMngrManager";}
  void            SelfKill();
  virtual bool    IsResetInStartupFail() const {return true;}

  TaskEntryPoint  GetMonitorEntryPoint();
  void            CreateDispatcher();

  // isLock is a boolean if it's a lock or release
  STATUS          SendSyncLockMsgToConfPartyProcess();

protected:
  void            InitkeepAliveStruct();
  void            OnCsCompStatusInd(CSegment* pSeg);
  void            OnTimerKeepAliveReceCIPServiceListiveTimeout();
  void            OnCsActiveAlarmInd(CSegment* pSeg);
  void            OnFailoverUpdateServiceInd(CSegment* pSeg);
  void            OnEPProcessStarted(void);
  void            GenerateCompStatusStStr(ostream& ostr);
  void            PrintLastKeepAliveTimes();
  BOOL            IsNewFailure(int id, compStatuses unitStatus);
  void            OnSnmpInterfaceReq(CSegment* pMsg);
  void            OnConfBlockInd(CSegment* pMsg);
  // Check if the differnce between the 2 services are in Gatekeeper part, and update consequently
  bool            UpdateGKIfNeeded(CIPService& service1, CIPService& service2);
  // Check if the differnce between the 2 services are in Sip Proxy part, and update consequently
  bool            UpdateSipProxyIfNeeded(CIPService& service1,
                                         CIPService& service2UpdateSipProxyIfNeeded,
                                         bool& isIceTheOnlyChange);
  bool            UpdateV35GwIfNeeded(CIPService& service1,
                                      CIPService& service2,
                                      bool& isResetApacheRequired);
  bool            AreServicesEqual(CIPService& service1, CIPService& service2,
                                   std::string sPurpose, bool isFromEMA = false,
                                   bool fixPorts = true);

  bool            UpdateSecurityIfNeeded(CIPService& service1,
                                         CIPService& service2);
  void            EncryptV35GwPassword(CIPService& theService);
  void            DecryptV35GwPassword(CIPService& theService);
  void            EncryptH323Password(CIPService& theService);
  void            DecryptH323Password(CIPService& theService);

  void            DecryptV35GwPasswordsFromFile(CIPServiceList& theList);
  void            DecryptGKAuthenticationPasswordsFromFile(CIPServiceList& theList);
  STATUS          DecryptV35asswordsFromFileToService(CIPService& theService);
  void            TreatV35GwPasswordFromEMA(CIPService& theService,
                                            eOperationOnService theOperation);
  void            TreatH323PasswordFromEMA(CIPService& theService,
                                           eOperationOnService theOperation);
  STATUS          UpdateV35GwPasswordsFile(CIPService& theService,
                                           eOperationOnService theOperation);
  STATUS          AddEntryToV35GwPasswordsFile(CIPService& theService);
  STATUS          UpdateEntryInV35GwPasswordsFile(CIPService& theService);
  STATUS          RemoveEntryFromV35GwPasswordsFile(CIPService& theService);
  STATUS          PrepareUpdatedV35GwPasswordsFileString_UpdateEntry(CIPService& theService, string& sUpdatedFileString);
  STATUS          PrepareUpdatedV35GwPasswordsFileString_DeleteEntry(CIPService& theService, string& sUpdatedFileString);
  STATUS          WriteV35GwPasswordsFile(std::string sToWrite);
  void            OnCardsToCsIceDetails(CSegment* pSeg);
  void 			  OnCardsToCsIceDetailsUpdateStatus(CSegment* pSeg);
  void            OnInterfaceConfigurationReq(CSegment* pSeg);
  void	          OnSNMPConfigInd(CSegment* pSeg);
  void	          OnSignalingServiceUpInd(CSegment* pSeg);
  void	          OnTimerGetServiceInfoInd(CSegment* pSeg);
  void            SetIsSNMPEnabled(BOOL bEnabled)		   { m_bSNMPEnabled = bEnabled; }
  BOOL            GetIsSNMPEnabled() const				   { return m_bSNMPEnabled; }
  STATUS 		  CheckCsMediaDuplicateIp(char* sIp);
  STATUS 		  PerformUpdate_UpdateProcess(CIPService* pUpdatedService,CIPService& dynamicLocal,bool isGKchanged,bool isSipProxychanged,bool isSecuritychanged,bool isIceTheOnlyChange);
private:
   std::string GetCloudIp() const;
  virtual void    DeclareStartupConditions();
  virtual void    ManagerPostInitActionsPoint();
  virtual void    ManagerStartupActionsPoint();
  virtual void    AddFilterOpcodePoint();
  virtual void    OnSysConfigTableChanged(const string& key, const string& data);

  void            SendDebugModeChangedToAllCS(const string& data);
  void            SendDebugModeChangedToSpecificSignaling(const int csId,
                                                          const CCfgData* cfgData);

  STATUS          UpdateIpService(CRequest*);
  STATUS          DeleteIpService(CRequest*);
  STATUS          AddIpService(CRequest*);
  STATUS          SetDefaultIPService(CRequest*);
  STATUS          SetDefaultSIPService(CRequest*);
  STATUS          PerformUpdateServiceProcedures(CIPService* pUpdatedService,
                                                 bool isFromEMA = false);
  STATUS          PerformUpdateServiceProceduresV2(CIPService* pUpdatedService,
                                                   bool isFromEMA = false);
  void 			  CreateNewSelfSignedCertificate(CIPService* pUpdatedIpService, WORD service_id);
  void 			  DeleteSelfSignedCertificate(CIPService* pUpdatedIpService);
  void 			  SyncCustomCfgFileWithIceParams(std::string key,DWORD data);
  void            SendV35GwUpdateIndToMcuMngr(CIPService& theService);
  void            RetrieveV35GwSpanParams(CIPService& theService, WORD& boardId,
                                          WORD& pqId);
  void            CalculateBoardIdPqNum(int spanIdx, WORD& boardId, WORD& pqId);
  void            UpdateSyncedFields(CIPService* pServiceFromFailoverTask,
                                     CIPService* pUpdatedService);
  std::string    ForceAppendToAliasName(const char* originalAliasName,
                                        const char* strToAppend);
  STATUS          SetPing(CRequest*);
  void            OnCsPingInd(CSegment* pMsg);
  STATUS          HandleTerminalPing4(CTerminalCommand& command,
                                      std::ostream& answer);
  STATUS          HandleTerminalPing6(CTerminalCommand& command,
                                      std::ostream& answer);
  STATUS          HandleTerminalPing(ePingIpType ipType,
                                     CTerminalCommand& command,
                                     std::ostream& answer);
  STATUS          HandleTerminalActiveAlarm(CTerminalCommand& command,
                                            std::ostream& answer);
  STATUS          HandleTerminalEnc(CTerminalCommand& command,
                                    std::ostream& answer);
  BYTE            IsGoodHostName(const char* cName);
  STATUS          HandleTerminalBombi(CTerminalCommand& command,
                                      std::ostream& answer);
  STATUS          HandleTerminalBombiFor1(CTerminalCommand& command,
                                          std::ostream& answer);
  STATUS          HandleTerminalUpdateIPServiceConfiguration(CTerminalCommand& command, std::ostream& answer);
  void            HandleNetworkSeparationConfigurations();
  void            ConfigureNetwork(CIPService* pService,
                                   const eConfigInterfaceType ifType);
  void            SetNICIpV6AutoConfig();
  void            ClearIPv6Addresses();
  //deleted for IP setting
  //void            RetrieveRouterList(CIPService* pService,
  //                                   std::list<CRouter>& routerList);
  //end
  BOOL            IsStartupFinished() const;

  STATUS          OnMcuMngrDnsHostRegistration(CSegment* pSeg);

  STATUS          OnMcuMngrEndIpConfigInd(CSegment* pSeg);
  void            OnMultipleServicesInd(CSegment* pParam);
  void            OnMcuMngrV35GwParamsReq(CSegment* pParam);
  void            OnCSApi_Msg(CSegment* pSeg);
  STATUS          OnCsNewInd(CSegment* seg);
  STATUS          OnCsEndStartUpInd(CSegment* seg);
  STATUS          OnCsNewServiceInitInd(CSegment* pSeg);
  STATUS          OnCsCommonParamInd(CSegment* pSeg);
  STATUS          OnCsCommonParamReadyInd(CSegment* pSeg);
  STATUS          OnCsDelServiceInd(CSegment* pSeg);
  STATUS          OnCsReconnectInd(CSegment* pSeg);

  STATUS          OnCsCardsMediaIpParamReq(CSegment* pSeg);
  STATUS          OnCsCardsMediaIpConfigInd(CSegment* pSeg);
  STATUS          OnCsCardsEthernetSettingInd(CSegment* pSeg);

  STATUS          OnCsConfIpServiceParamReq(CSegment* pSeg);
  STATUS          OnCsRcrsIpServiceParamReq(CSegment* pSeg);

  STATUS          OnCsRcrsUdpPortRangeInd(CSegment* pSeg);
  STATUS          OnCFSInd(CSegment* pSeg);

  STATUS          OnCsProxyIpServiceParamReq(CSegment* pSeg,
                                             const string theCaller = "");
  STATUS          OnIceIpServiceParamReq(CSegment* pSeg,
                                               const string theCaller = "");
  STATUS          OnCsProxyIpServiceParamReqNotReady(CSegment* pSeg);
  STATUS          OnCsProxyIpServiceParamReqReady(CSegment* pSeg);
  STATUS          OnIceIpServiceParamReqReady(CSegment* pSeg);

  STATUS          OnCsGKIpServiceParamReq(CSegment* pSeg);
  STATUS          OnCsGKIpServiceUpdateParamReq(CSegment* pSeg);
  STATUS          OnCsGKIpServiceUpdatePropertiesReq(CSegment* pSeg);
  STATUS          OnCsGKClearAltPropertiesReq(CSegment* pSeg);
  STATUS          OnCsGKIpInPropertiesReq(CSegment* pSeg);
  STATUS          OnCsGKIdInPropertiesReq(CSegment* pSeg);
  STATUS          OnCsGKNameInPropertiesReq(CSegment* pSeg);

  STATUS          OnDnsResolveInd(CSegment* pSeg);
  STATUS          OnCsProxyRegistrarStatus(CSegment* pSeg);

  STATUS          OnCsCertMngrIpServiceParamReq(CSegment* pSeg);
  STATUS          OnCsMcuMngrIpServiceParamReq(CSegment* pSeg);
  STATUS          OnCsTCPDumpIpServiceParamReq(CSegment* pSeg);
  STATUS          OnDnsAgentGetIpconfig(CSegment* pSeg);

  BOOL SendManagementTCPDumpParams();
  BOOL SendIpServicesTCDumpParams();

	void TakeInfoMsgFromCS();
	void TraceMessage(const char *location, const char *message, STATUS status);

	void OnCSIPConfigEndMSPerService(CSegment* pParam);

  void   SendSnmpCsInterface(DWORD serviceId);
  STATUS AddRoutingTableRule(CIPService* pService);
  void   SendIpServiceListIfNeeded(const int serviceId,
                                   BOOL bSendIpServiceListAfterGettingIpType);
  void   SendIpServiceListToResources();
  void   SetMngmntIpParamsInProcess(eIpType ipType,
                                    eV6ConfigurationType ipv6ConfigType,
                                    DWORD ipV4Add,
                                    string ipv6Add_0,
                                    string ipv6Add_1,
                                    string ipv6Add_2,
                                    string defGwIpv6,
                                    DWORD defGwIpv6Mask,
                                    char ipAddressStr[IP_ADDRESS_LEN],
                                    BOOL dnsStatus,
                                    char ipv6_address[IPV6_ADDRESS_LEN]);
  void   SetMngntIpParamsMcmsNetwork();

  char ipAddressStr[IP_ADDRESS_LEN];
  BOOL dnsStatus;

  BYTE    IsMultipleServices();
  void    StartMultipleCS();
  void    CopyOCSFilesToRightLocation();
  BOOL    CheckIfDefaultGWIPV6IsValid(CIPService* pUpdatedIpService);
  BOOL    CheckIfDefaultGWIPV4IsValid(CIPService* pUpdatedIpService);
  BOOL    IsToCopyMSFileToOneServiceFile();
  void    CreateSignalingTask(char* data, WORD csId);
  BOOL    IsTaskAlreadyExists(WORD csId);
  void    CopyOCSFilesToRightLocation(CIPServiceList* pIpServListStatic);
  void    AllocHeapMemory();
  void    FreeHeapMemory();
  void    OnTimerSendIpServiceToUtilityProcess(CSegment* pParam);
  void  OnMcuMngrPrecedenceSettings(CSegment* pSeg);
  eIpType GetSysIpTypeForSim() const;
  //added for IP setting
  void 	  SetCsMediaIpConfig(CIPService *ipService,  const eConfigInterfaceType ifType);

  
  void    AddCSMediaIpInterfacesGesherNinja(CIPService *pService, BYTE bSystemMode, eProductType curProductType);

  void   CSMngrRestartStartupProcedure(CSegment *pSeg);
  //end
  STATUS CSMngrCalculateSetProcessState(CSegment* pSeg);
  STATUS OnCheckEth2CS(CSegment* pSeg);
  STATUS OnFireEth2CS(CSegment* pSeg);
  STATUS OnGetDefaultRouter(CSegment* pSeg);
  
  void   SendPrecedentSetting();
  bool m_flagIsMediaIpConfigReceived;
  bool m_flagIsIpTypeReceived;
  bool m_flagIsIpListSentToCardsAfterMediaIpConfigReceived;
  bool m_flagIsRsrcAskedForIpServiceList;
  bool m_flagIsTCPDumpAskedForIpServiceList;
  bool m_sentMngmtTCPDump;

  CCommDnsAgentService* m_CommDnsAgentService;
  CCommMcuService*      m_CommMcuService;
  CCommCardService*     m_CommCardService;
  CCommConfService*     m_CommConfService;
  CCommRsrcService*     m_CommRcrsService;
  CCommProxyService*    m_CommProxyService;
  CCommIceService*    	m_CommIceService;
  CCommGKService*       m_CommGKService;
  CCommSnmpService*     m_CommSnmpService;
  CCommCertMngrService* m_CommCertMngrService;
  CCommMcuMngrService*  m_CommMcuMngrService;
  CCommStartupService*  m_CommStartupService;
  CCommTCPDumpService*  m_CommTCPDumpService;
  CPrecedenceSettings*  m_precedentSetting;
  STATUS m_StatusReadIpServiceList;
  CCSMngrMplMcmsProtocolTracer* m_CSMngrMplMcmsProtocolTracer;
  CCSMngrMessageValidator*      m_CSMngrMessageValidator;

  eIpType              m_sysIpType;
  eV6ConfigurationType m_sysV6ConfigurationType;
  bool                 m_ipListAlreadySent[NUM_OF_PROCESSES_TO_SEND_IP_LIST];
  bool                 m_mcuMngrIpTypeIndAlreadyReceived;
  BYTE                 m_isMultipleServices;
  bool                 m_flagAreIpOk [MAX_NUMBER_OF_SERVICES_IN_RMX_4000];
  CCsPinger*           m_pCsPinger;
  MEDIA_BOARD_STATUS   m_media_board_status[MAX_NUMBER_OF_SERVICES_IN_RMX_4000];

  BOOL				   m_bSNMPEnabled;
  std::vector<WORD>    m_vecServiceID;
  BOOL					m_isEth2Up;
  BOOL					m_firedEth2CS;

  PDECLAR_MESSAGE_MAP;
  PDECLAR_TRANSACTION_FACTORY;
  PDECLAR_TERMINAL_COMMANDS;

  DISALLOW_COPY_AND_ASSIGN(CCSMngrManager);
};

#endif
