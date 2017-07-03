// ConfiguratorManager.h

#ifndef CONFIGURATOR_MANAGER_H_
#define CONFIGURATOR_MANAGER_H_

#include "ManagerTask.h"
#include "Macros.h"
#include "ConfigManagerApi.h"
#include "ProductTypeDecider.h"

void ConfiguratorManagerEntryPoint(void* appParam);

typedef enum IPRangeState{NO_Range,IP24_Range,IP16_Range,Invalid_Range} IPRangeState;
class CConfiguratorManager : public CManagerTask
{
CLASS_TYPE_1(CConfiguratorManager, CManagerTask)
public:
	CConfiguratorManager(void);
	~CConfiguratorManager(void);
	void ManagerPostInitActionsPoint();

	void ArpingRequest(CSegment* pSeg,bool isFailover);	// IPv4 duplication checking
	
	eConfigInterfaceType GetMngrIfType(const eIpType ipType);
	
	void McuMngrArpingRequest(CSegment* pSeg);
	void DadRequest(CSegment* pSeg);	// IPv6 duplication checking
	void FailoverArpingRequest(CSegment* pSeg);
    void AddIpInterface(CSegment* pSeg);
    //for gesher/ninja
    void AddIpInterfaceGesherNinja(CSegment* pSeg);
    void AddIpInterfaceRMX(CSegment* pSeg);
    //end
    void AddIpV6Interface(CSegment* pSeg);
	void AddIpV6InterfaceGesherNinja(CSegment* pSeg);
	void AddIpV6InterfaceRMX(CSegment* pSeg);
	
    void AddVlan(CSegment *pSeg);
    void GetNewVersionNumber(CSegment* pSeg);
    void RemoveIpInterface(CSegment* pSeg);
    void CycleVersions(CSegment* pSeg);
    void ConfigureDnsServers(CSegment* pSeg);
    void ConfigureMoreDnsServers(CSegment* pSeg);
    void NameServerUpdate(CSegment* pSeg);
    void RunDHCP(CSegment* pSeg);
    void KillDHCP(CSegment* pSeg);
    void GetLastDHCPConfiguration(CSegment* pSeg);
    void RemountVersionPartition(CSegment* pSeg);
    void AddRootUserToSystem(CSegment *pSeg);
    void DelUserFromSystem(CSegment *pSeg);
    void ChangePassword(CSegment *pSeg);
    void EnableDisablePing(CSegment* pSeg);
    void EnableDisablePingBroadcast(CSegment* pSeg);
    //void EnableDisablePingIptables(CSegment* pSeg);
    void SyncTimeWithSwitch(CSegment* pSeg);
    void RestartSnmpd(CSegment* pSeg);
    void StartSnmpd(CSegment* pSeg);
    void StopSnmpd(CSegment* pSeg);
    virtual void RestartSSH(CSegment* pSeg);
    void OnConfigSSHTimeout();
    void GetDMAStatus(CSegment *pSeg);
    void TestEthSetting(CSegment *pSeg);
    void SetEthSettings(CSegment *pSef);
    void GetSmartErrors(CSegment* pSeg);
    void RunSmartSelfTest(CSegment* pSeg);
    void TakeCoreOwnership(CSegment* pSeg);
    void DeleteFile(CSegment* pSeg);
    void DeleteDir(CSegment *pSeg);
    void SetTCPStackParams(CSegment *pSeg);
    void InterfaceUp(CSegment* pSeg);
    void RunningCommand(CSegment* pSeg);
    void DeleteTempFiles(CSegment* pSeg);
    void AddDefaultGW(CSegment* pSeg);
    void AddStaticIpRoutes(CSegment* pSeg);
    void AddStaticRoutes(CSegment* pSeg);
    void SetIpv6AutoConfig(CSegment* pSeg);
    void AddDefaultGWRoutingTableRule(CSegment* pSeg);
    void ConfigureNetworkSeperationIpv6Only(eConfigInterfaceType ifType,eIpType ipType,std::string ip,DWORD mask,std::string defaultGW,std::string ipv6SubNetMask,BOOL bIPv6);
    void DelDefaultGWRoutingTableRule(CSegment* pSeg);
    void SetProductType(CSegment* pSeg);
    void SetSpecialProductType(CSegment* pSeg);
    void EvokeNetworkInterfaces(CSegment *pSeg);
    void SetMacAddress(CSegment *pSeg);
    void GetHDDTemperature(CSegment* pSeg);
    void GetHDDSize(CSegment* pSeg);
    void GetHDDModel(CSegment* pSeg);
    void GetHDDFirmware(CSegment* pSeg);
    void GetFlashSize(CSegment* pSeg);
    void GetFlashModel(CSegment* pSeg);
    void GetCPUDetails(CSegment* pSeg);
    void GetRAMsize(CSegment* pSeg);
    void EthSettingsMonitoring(CSegment *pSeg);
    void FillEthSettingsStruct(ETH_SETTINGS_S *pCurEthStruct, eConfigInterfaceNum ifNum);
    void MountNewVersion(CSegment* pSeg);
    void UnmountNewVersion(CSegment* pSeg);
    void FirmwareCheck(CSegment* pSeg);
    void ExposeEmbeddedNewVersion(CSegment* pSeg);
    void AddNICRoutingTableRule(CSegment* pSeg);
    void HandleFallbackVersion(CSegment* pSeg);
    void AddDropRuleToShorewall(CSegment* pSeg);
    void AddDropRuleToIptables(CSegment* pSeg);
    void AddQosManagementRuleToIptables(CSegment* pSeg);
    void RestoreFallbackVersion(CSegment* pSeg);
    void AddDSCPRuleToIpTables(std::string iptables, std::string signalingDSCP);
    void StopCheckEth0Link();
    void StartCheckEth0Link();
    void CheckCSIpConfig(CSegment* pSeg);
    void OnCheckEth0LinkTimeout(void);
    void ClearTcpDumpStorage();
    void StartTcpDump(CSegment* pSeg);
    void StopTcpDump();
    // void RenameTcpDumpOutput(CSegment* pSeg);
    void FPGAUpgrade(CSegment* pSeg);
    STATUS ParseFPGAUpgradeLine(const char * line, eFPGAUpgradeAction & currentStep, unsigned int & percent);
    STATUS FPGAImageHeaderCompare(std::string szFPGAImgPath);

    void OnCheckChildZombieProcessesTimer(CSegment* pParam);
    void OnWriteFile(CSegment* seg);
    void OnWriteFileRootOnly(CSegment* seg);
    
    void GetValueFromRegister(CSegment* seg);
    void GetTempFromAdvantechUtil(CSegment* seg);
    void EnableWhiteList(CSegment* seg);
    void DisableWhiteList(CSegment* seg);    
    void OnConfigNetworkSeperationIpv6Timeout(CSegment* seg);

    virtual void EnableDHCPIPv6(CSegment* pSeg);
    virtual void DisableDHCPIPv6(CSegment* pSeg);

    virtual void KillSsh(CSegment* seg);
    void RestartApache(CSegment* seg);
    void ConfigBondingInterface(CSegment* pSeg);
    void AddPortsIpTables(CSegment* seg);
    void SetBondingInterfaceSlaves(CSegment* pSeg);

    void GetUdpOccupiedPorts(CSegment* seg);
    void ConfigGenerateConfFlie802_1x(CSegment* pSeg);

    BOOL TestRoot();
	TaskEntryPoint GetMonitorEntryPoint();

	STATUS ArpingTerminalRequest(CTerminalCommand & command, std::ostream& answer);
	STATUS ArpingFTerminalRequest(CTerminalCommand & command, std::ostream& answer);
	void AddIpAndNatRuleForDnsPerService(CSegment* seg);
	void  AddNatRule(CSegment* seg);
	void  DeleteNatRule(CSegment* seg);

    /*Begin:added by Richer for BRIDGE-15015, 11/13/2014*/
    void OnLEDSysAlarmIndication(CSegment* pMsg);
    /*End:added by Richer for BRIDGE-15015, 11/13/2014*/

	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY
	PDECLAR_TERMINAL_COMMANDS


private:
	static STATUS IsPingEnabled(BOOL& enable);
	static STATUS IsPingEnabledIptables(BOOL& enable);
	static STATUS EnableICMPv6(void);

  void RollbackIP();
  void BackupIPParameters();
  STATUS AddUserToOS(const std::string& userName,
                     const std::string& userPassword,
                     std::string& answer);

  void SetNICIpV6AutoConfig(const string &NIC, BYTE bIsAutoConfig,const eConfigInterfaceType ifType , const eIpType ipType,const string ipv6gateway,const string ipv6gwmask);
  BOOL IsSeparatedNetworks() const;
  void ifconfig_up(std::string stNicName);
  void ifconfig_up(std::string stNicName,std::string ip4addr,std::string mask);
  void ifconfig_down(std::string stNicName);
  std::string m_PreviousIP, m_PreviousNetmask, m_PreviousBroadcast;
  std::string m_PreviousDefaultGW ;
  std::string m_StNicName;
  STATUS ChangeUserIdTo200(const std::string& username);
  STATUS ChangePasswordInShadowFile(const std::string& password,
                                    const std::string& username);
  void IptablesClear();
  bool CheckIfWhiteListExist(int& iptype);
  bool AddIpToWhiteList(std::string& sIp,DWORD& iptype);
  void FPGAUpgradeProgress(eFPGAUpgradeAction action , STATUS stat, unsigned int percent);
  
  IPRangeState CheckIpv4Range(std::string& sIp);
  STATUS       InitalizeIpTablesWhiteList();
  STATUS       FinalizeIpTablesWhiteList();
  void OnCheckEth2(CSegment* pSeg);
  
  
    std::string GetCmdLinePrefix() const
    {
        return m_productTypeDecider->GetCmdLinePrefix();
    }
    std::string GetCmdLine(char const * base) const
    {
        return m_productTypeDecider->GetCmdLine(base);
    }
    BOOL SkipOperation() const
    {
        return m_productTypeDecider->SkipOperation();
    }
    //char const * GetHddDevName() const;
    std::string GetCycleVersionsScriptCmdLine(std::string const & new_version_name) const
    {
        return m_productTypeDecider->GetCycleVersionsScriptCmdLine(new_version_name);
    }
    std::string GetMountNewVersionScriptCmdLine() const
    {
        return m_productTypeDecider->GetMountNewVersionScriptCmdLine();
    }
    std::string GetUnmountNewVersionScriptCmdLine() const
    {
        return m_productTypeDecider->GetUnmountNewVersionScriptCmdLine();
    }
    std::string GetFirmwareCheckScriptCmdLine() const
    {
        return m_productTypeDecider->GetFirmwareCheckScriptCmdLine();
    }
    BOOL IfSkipAddNICRoutingTableRule() const
    {
        return m_productTypeDecider->IfSkipAddNICRoutingTableRule();
    }

    STATUS ConfigureIpv6NetSeperationRoutingTables(std::string route_table,std::string	stNicName);
    void WriteFile(CSegment* seg, mode_t mode) ;
    
  const char* m_managment_interface_name;
  ProductTypeDecider * m_productTypeDecider;
  
  DISALLOW_COPY_AND_ASSIGN(CConfiguratorManager);
};

#endif // !defined(_DEMOMANAGER_H__)
