// McuMngrProcess.h

#ifndef MCU_MNGR_PROCESS_H_
#define MCU_MNGR_PROCESS_H_

#include "ProcessBase.h"
#include "McuMngrDefines.h"
#include "InstallPhaseCommon.h"
#include "PrecedenceSettings.h"
#include "LicensingServerStructs.h"
#include "LicenseStatusApiEnums.h"

class CIPServiceFullList;
class CIPServiceList;
class CIPService;
class CIPSpan;
class CMcuState;
class CSystemTime;
class CLicensing;
class CEthernetSettingsConfig;
class CEthernetSettingsConfigList;
class CPortSpeed;
class CSystemInterfaceList;

class CMcuMngrProcess : public CProcessBase
{
  CLASS_TYPE_1(CMcuMngrProcess, CProcessBase)
public:
  friend class CTestMcuMngrProcess;

  CMcuMngrProcess();
  virtual const char*          NameOf() const   { return "CMcuMngrProcess";}
  virtual ~CMcuMngrProcess();
  virtual eProcessType         GetProcessType() {return eProcessMcuMngr;}
  virtual BOOL                 UsingSockets()   {return NO;}
  virtual TaskEntryPoint       GetManagerEntryPoint();
  virtual void                 AddExtraStringsToMap();
  virtual void                 AddExtraStatusesStrings();
  void                         SetMngmntIpParams_asIpService(CIPService& theService);
  CIPService*                  GetMngmntIpParams_asIpService();
  CIPServiceList*              GetIpInterfaces_asIpServicesList();
  CIPSpan*                     GetShelfMngrIpInterface_asIpSpan();
  void                         SetShelfMngrIpInterface_asIpSpan(CIPSpan& other);
  CIPServiceFullList*          GetXMLWraper() {return m_XMLWraper;  }
  BOOL                         IsIpInterfaceExistsInList(const CIPService& other);
  STATUS                       AddIpInterfaceToList(const CIPService& other);
  STATUS                       UpdateIpInterfaceInList(CIPService& other);
  CFaultList*                  GetActiveAlarmsList();
  void                         SetActiveAlarmsList(CFaultList* pActiveAlarmsList);
  CMcuState*                   GetMcuStateObject();
  void                         CreateDefaultMcuTimeFile();

  //void GetFlexeraCapability(int i,char* name);
  bool IsFlexeraCapabilityEnabled(int i);
  DWORD GetFlexeraCapabilityCounted(int i);
  bool IsFlexeraCapabilityChanged(int i);
  CStructTm GetFlexeraCapabilityExpDate(int i);
  void IncreaseFlexeraCapabilityExpDateMon(int i);

  void SetFlexeraData(FLEXERA_DATA_S * serverParams);

  E_FLEXERA_LICENSE_VALIDATION_STATUS GetFlexeraCapabilityStatus(int i);

  const CSystemTime& 		   GetCurrentMcuTime() const;
  BOOL                         GetMcuTime(CSystemTime& Time);
  void                         SetMcuTime(CSystemTime& Time);
  void                         SetMcuTimeNtpServerStatus(int index,
                                                         eNtpServerStatus serverStatus, WORD numFailuresSinceConnecting = 0);
  STATUS                       LoadMcuTime();
  CLicensing*                  GetLicensing();
  void                         SetLicensing(CLicensing* pLicensing);

  // Increases the time to 120 seconds
  virtual DWORD                GetMaxTimeForIdle(void) const
  {
    return 12000;
  }

  DWORD                        GetMngmntIp() const;
  void                         GetMngmntIpV6(char* retIp) const;
  void                         GetMngmntIpAsString(std::string& outStrMngmntIp) const;
  DWORD                        GetShelfIp() const;
  virtual bool                 IsFailoverBlockTransaction_SlaveMode(std::string sAction);
  
  void                         GetSheIfIpV6(char* retIp) const;

  CEthernetSettingsConfigList* GetEthernetSettingsConfigList();
  void                         SetEthernetSettingsConfig(CEthernetSettingsConfig& ethernetSettingsConfig);
  STATUS                       LoadEthernetSettingsConfig();
  std::string                  GetCntrlIPv6Address();
  
  const APIU8* 					GetMngmtIpV6ByteArray() const;
  const APIU8*					GetSheIfIpV6ByteArray() const;
  
  CPrecedenceSettings*		   GetPrecedenceSettings();
  void						   SetPrecedenceSettings(CPrecedenceSettings* pPrecedenceSettings);
  virtual int 				   GetProcessAddressSpace() {return 96 * 1024 * 1024;};
  STATUS ValidateSwMcuFields(CIPService* pUpdatedIpService);
  STATUS ValidateMFWFields(CIPService* pUpdateIpService);
  std::string GetMFWValidatedResult();

  CInstallPhaseList m_installPhaseList;

  BOOL IsFlexeraSimulationMode();

  LicenseFeatureStatus ConvertFeatureStatus(E_FLEXERA_LICENSE_VALIDATION_STATUS type);

  void PrintFlexeraData();

  FLEXERA_DATA_S         m_flexeraCapabilitiesList;

private:
  STATUS ValidateDnsDetails(CIPService *pService, CIPService *pUpdatedService);
  STATUS ValidateSubnetMaskDetails(DWORD netMask, DWORD updatedNetmask);
  STATUS ValidateRouterDetails(CIPService *pService, CIPService *pUpdatedService);
  STATUS ValidateIpTypeDetails(eIpType ip_type, eIpType updated_ip_type);
  STATUS ValidateLanPortsDetails(CPortSpeed* port_speed_vector, CPortSpeed* updated_port_speed_vector);
  STATUS ValidateIpAddressAndInterface(DWORD ipv4Address, DWORD UpdatedIpv4Address, std::string interface, std::string UpdatedInterface);
  STATUS ValidateSHMIpAddress(DWORD ipv4Address, DWORD UpdatedIpv4Address);
  STATUS ValidateHostname(const CSmallString& host_name, const CSmallString& updated_host_name);


private:
  CIPServiceFullList*          m_XMLWraper;        // contains m_update_cnt for dynamic part of services
  CIPServiceList*              m_pIpInterfaces_asIpServicesList;
  CIPService*                  m_pMngmntIpParams_asIpService;
  CIPSpan*            		   m_pShelfMngrIpInterface_asIpSpan;
  CSystemInterfaceList*        m_pSysInterfaceList;

  CFaultList*                  m_pActiveAlarmsList;
  CMcuState*                   m_pMcuStateObject;
  CSystemTime*                 m_pMcuTime;
  CLicensing*                  m_pLicense;
  CEthernetSettingsConfigList* m_pEthernetSettingsConfigList;
  CPrecedenceSettings*		   m_pPrecedenceSettings;

  BOOL m_isFlexeraSimulationMode;



};

#endif
