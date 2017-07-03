// ConfigManagerApi.cpp

#include "ConfigManagerApi.h"
#include "ConfigManagerOpcodes.h"
#include "ProcessBase.h"
#include "ApiStatuses.h"
#include "TraceStream.h"
#include "Trace.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"

// Static.
bool CConfigManagerApi::IsSSHOnForDebug(void)
{
#ifdef SSH_ON_FOR_DEBUG
	
  bool ret = true;

#else

  // Always returns false in release versions.
  // On release this is the only line that should be changed from true to false.
//  bool ret = false;
  bool ret = true;

  // TODO: put comment before release.
  CProcessBase* proc = CProcessBase::GetProcess();
  FPASSERT_AND_RETURN_VALUE(NULL == proc, ret);
  switch (proc->GetProductType())
  {
    case eProductTypeRMX1500:
    case eProductTypeRMX2000:
    case eProductTypeRMX4000:
      if (!IsTarget())
        ret = false;
    	break;  // Does not run in simulation.

    case eProductTypeGesher:
    case eProductTypeNinja:
    	break;

    default:   //for MFW / Soft MCU / Edge - alwys close SSH during and at last version
    	ret = false;
  }

#endif

  FTRACECOND(ret, "SSH is on for debug: remove before release");

  return ret;
}

CConfigManagerApi::CConfigManagerApi() :
  CManagerApi(eProcessConfigurator)
{ }

STATUS CConfigManagerApi::ArpingRequest(const std::string& ip,eIpType ipType)
{
  CSegment* seg = new CSegment;
  *seg << ip
	   <<  (DWORD) ipType ;

  OPCODE opcode;
  STATUS res =  SendMessageSync(seg,
                                CONFIGURATOR_ARPING_REQUEST,
                                30*SECOND,
                                opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::FailoverArpingRequest(const std::string& ip,
                                                const std::string& defaultGwIp,
                                                DWORD ipType,
                                                std::string& answer)
{
  CSegment* seg = new CSegment;
  *seg << ip;
  *seg << defaultGwIp;
  *seg << ipType;

  OPCODE opcode;
  STATUS res =  SendMessageSync(seg,
                                CONFIGURATOR_FAILOVER_ARPING_REQUEST,
                                30*SECOND,
                                opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::DADRequest(const std::string& ip)
{
  CSegment* seg = new CSegment;
  *seg << ip;

  OPCODE opcode;
  STATUS res =  SendMessageSync(seg,
                                CONFIGURATOR_DAD_REQUEST,
                                30*SECOND,
                                opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}
STATUS CConfigManagerApi::ConfigBonding(const std::string& bondingMode,DWORD linkmonitoringfrequency,BOOL bIsBondingEnabled)
{
	CSegment* seg = new CSegment;

	*seg << (DWORD) linkmonitoringfrequency
		<< (DWORD)bIsBondingEnabled
    	<< bondingMode ;


	OPCODE opcode;
	STATUS res =  SendMessageSync(seg,
			CONFIGURATOR_CONFIG_BONDING_INTERFACE,
								30*SECOND,
								opcode);

	if (res == STATUS_OK)
		return opcode;

	return res;
}

STATUS CConfigManagerApi::ConfigBondingInterfaceSlaves()
{
	CSegment* seg = new CSegment;

	OPCODE opcode;
	STATUS res =  SendMessageSync(seg,
			CONFIGURATOR_SET_BONDING_INTERFACE_SLAVES,
								30*SECOND,
								opcode);

	if (res == STATUS_OK)
    	return opcode;

	return res;
}

STATUS CConfigManagerApi::AddIpInterface(eConfigInterfaceType networkType,
										 eIpType ipType,
                                         const std::string& ipToCompare,
                                         const std::string& ip,
                                         const std::string& mask,
                                         const std::string& gateway,
                                         const std::string& broadcast,
                                         const std::list<CRouter>& routerList)
{
  CSegment* seg = new CSegment;

  *seg << (DWORD) networkType
	   << (DWORD)ipType
       << ipToCompare
       << ip
       << mask
       << gateway
       << broadcast;

  DWORD size = routerList.size();

  *seg << routerList.size();
  for (std::list<CRouter>::const_iterator itr = routerList.begin();
       itr != routerList.end();
       itr++)
  {
    DWORD temp = (DWORD)itr->m_type;
    *seg << temp
         << itr->m_targetIP
         << itr->m_subNetmask
         << itr->m_gateway;
  }

  OPCODE opcode;
  STATUS res =  SendMessageSync(seg,
                                CONFIGURATOR_ADD_IP_INTERFACE,
                                30*SECOND,
                                opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::AddIpV6Interface(eConfigInterfaceType networkType,
                                           BYTE bIsAutoConfig,
                                           const std::string& ip /*= ""*/,
                                           const std::string& mask /*= ""*/,
                                           const std::string& gateway /*= ""*/,
                                           const std::string& gwmask /*= ""*/)
{
  CSegment* seg = new CSegment;

  *seg << (DWORD) networkType
       << bIsAutoConfig
       << ip
       << mask
       << gateway
       << gwmask;

  OPCODE opcode;
  STATUS res =  SendMessageSync(seg,
                                CONFIGURATOR_ADD_IPV6_INTERFACE,
                                30*SECOND,
                                opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::InterfaceUp(const std::string nicName)
{
  CSegment* seg = new CSegment;

  *seg << nicName;

  OPCODE opcode;
  STATUS res =  SendMessageSync(seg,
                                CONFIG_INTERFACE_UP,
                                30*SECOND,
                                opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}



STATUS CConfigManagerApi::RunCmd(const std::string cmd)
{
  CSegment* seg = new CSegment;

  *seg << cmd;

  OPCODE opcode;
  STATUS res =  SendMessageSync(seg,
		                        CONFIG_RUN_COMMAND,
                                10*SECOND,
                                opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::AddDefaultGW(const std::string nicName,
                                       const std::string gateway,
                                       const BOOL isIpv6)
{
  CSegment* seg = new CSegment;

  *seg << nicName
       << gateway
       << isIpv6;

  OPCODE opcode;
  STATUS res =  SendMessageSync(seg,
                                CONFIG_ADD_DEFAULT_GW,
                                30*SECOND,
                                opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::AddStaticIpRoutes(eConfigInterfaceType networkType,
                                            const std::list<CRouter> routerList)
{
  CSegment* seg = new CSegment;

  *seg << (DWORD) networkType
       << routerList.size();

  for (std::list<CRouter>::const_iterator itr = routerList.begin();
       itr != routerList.end();
       itr++)
  {
    DWORD temp = (DWORD)itr->m_type;
    *seg << temp
         << itr->m_targetIP
         << itr->m_subNetmask
         << itr->m_gateway;
  }

  OPCODE opcode;
  STATUS res =  SendMessageSync(seg,
                                CONFIG_ADD_STATIC_IP_ROUTE,
                                30*SECOND,
                                opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::AddStaticRoutes(eConfigInterfaceType networkType,
                                          const std::list<CRouter> routerList)
{
  CSegment* seg = new CSegment;

  *seg << (DWORD) networkType
       << routerList.size();

  for (std::list<CRouter>::const_iterator itr = routerList.begin();
       itr != routerList.end();
       itr++)
  {
    DWORD temp = (DWORD)itr->m_type;
    *seg << temp
         << itr->m_targetIP
         << itr->m_subNetmask
         << itr->m_gateway;
  }

  OPCODE opcode;
  STATUS res =  SendMessageSync(seg,
                                CONFIG_ADD_STATIC_ROUTE,
                                30*SECOND,
                                opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;

}

STATUS CConfigManagerApi::SetNICIpV6AutoConfig(const std::string nicName,
                                               const BOOL isIpv6Autoconfig,const eConfigInterfaceType ifType
                                               , const eIpType ipType,const std::string defaultGatewayIPv6Str)
{
  CSegment* seg = new CSegment;

  *seg << nicName
	   << defaultGatewayIPv6Str
       << isIpv6Autoconfig
       << (DWORD)ifType
       << (DWORD)ipType;

  OPCODE opcode;
  STATUS res =  SendMessageSync(seg,
                                CONFIG_IPV6_AUTOCONFIG,
                                30*SECOND,
                                opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::AddVlan(eConfigInterfaceType networkType,
                                  const std::string& ip,
                                  const std::string& mask,
                                  const std::string& broadcast,
                                  const DWORD vLanId)
{
  CSegment* seg = new CSegment;

  *seg << (DWORD) networkType
       << ip
       << mask
       << broadcast
       << vLanId;

  OPCODE opcode;
  STATUS res =  SendMessageSync(seg,
                                CONFIGURATOR_ADD_VLAN_INTERFACE,
                                30*SECOND,
                                opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::GetNewVersionNumber(std::string& version)
{
  CSegment seg;
  OPCODE   opcode;
  STATUS   stat =  SendMessageSync(NULL,
                                   CONFIGURATOR_GET_NEW_VERSION_NUMBER,
                                   30*SECOND,
                                   opcode,
                                   seg);

  if (stat != STATUS_OK)
    return stat;

  if (opcode != STATUS_OK)
    return opcode;

  seg >> version;
  return STATUS_OK;
}

STATUS CConfigManagerApi::RemoveIpInterface(const std::string& ip)
{
  CSegment* seg = new CSegment;

  *seg << ip;

  OPCODE opcode;
  STATUS res = SendMessageSync(seg,
                               CONFIGURATOR_REMOVE_IP_INTERFACE,
                               15*SECOND,
                               opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::CycleVersions(const std::string& new_version_name)
{
  OPCODE    opcode;
  CSegment* seg = new CSegment;
  *seg << new_version_name;
  STATUS res = SendMessageSync(seg,
                               CONFIGURATOR_CYCLE_VERSION,
                               120*SECOND,
                               opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::MountNewVersion()
{
  CSegment seg;
  OPCODE   opcode;
  STATUS   stat =  SendMessageSync(NULL,
                                   CONFIGURATOR_MOUNT_NEW_VERSION,
                                   60*SECOND,
                                   opcode,
                                   seg);

  if (stat != STATUS_OK)
    return stat;

  if (opcode != STATUS_OK)
    return opcode;

  return STATUS_OK;
}

STATUS CConfigManagerApi::UnmountNewVersion()
{
  CSegment seg;
  OPCODE   opcode;
  STATUS   stat =  SendMessageSync(NULL,
                                   CONFIGURATOR_UNMOUNT_NEW_VERSION,
                                   60*SECOND,
                                   opcode,
                                   seg);

  if (stat != STATUS_OK)
    return stat;

  if (opcode != STATUS_OK)
    return opcode;

  return STATUS_OK;
}

STATUS CConfigManagerApi::FirmwareCheck(std::string& result)
{
  CSegment seg;
  OPCODE   opcode;
  STATUS   stat =  SendMessageSync(NULL,
                                   CONFIGURATOR_FIRMWARE_CHECK,
                                   90*SECOND,
                                   opcode,
                                   seg);

  if (stat != STATUS_OK)
    return stat;

  if (opcode != STATUS_OK)
    return opcode;

  seg >> result;

  return STATUS_OK;
}

STATUS CConfigManagerApi::ConfigureDnsServers(const std::string& search,
                                              const std::string& dns1,
                                              const std::string& dns2,
                                              const std::string& dns3)
{
  CSegment* seg = new CSegment;
  *seg << search << dns1 << dns2 << dns3;

  OPCODE opcode;
  STATUS res = SendMessageSync(seg,
                               CONFIGURATOR_DNS_SERVIER,
                               15*SECOND,
                               opcode);
  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::ConfigureMoreDnsServers(const char* search,
                                                  const std::string& dns1)
{
  CSegment* seg = new CSegment;
  *seg << search << dns1;

  OPCODE opcode;
  STATUS res = SendMessageSync(seg,
                               CONFIGURATOR_MORE_DNS_SERVER,
                               15*SECOND,
                               opcode);
  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::RunDHCP(BOOL autoDNS)
{
  CSegment* seg = new CSegment;
  *seg << autoDNS;

  OPCODE opcode;
  STATUS res = SendMessageSync(seg,
                               CONFIGURATOR_RUN_DHCP,
                               5*SECOND,
                               opcode);
  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::KillDHCP(eIpType type)
{
  OPCODE opcode;
  CSegment* seg = new CSegment;
   *seg << (DWORD) type;

  STATUS res = SendMessageSync(seg,
                               CONFIGURATOR_KILL_DHCP,
                               5*SECOND,
                               opcode);
  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::RegisterHostNameInDNS(const std::string& dnsIpAddress,
                                                const std::string& myHostName,
                                                const std::string& myzone,
                                                const std::string& myIpAddress,
                                                const std::string&
                                                myIpv6Address)
{
  CSegment* seg = new CSegment;
  *seg << dnsIpAddress
       << myHostName
       << myzone
       << myIpAddress
       << myIpv6Address;

  OPCODE opcode;
  STATUS res = SendMessageSync(seg,
                               CONFIGURATOR_NS_UPDATE,
                               5*SECOND,
                               opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}

// ///////////////////////////////////////////////////////////////////////////
// keys : "IPADDR","NETMASK","NETWORK","BROADCAST","GATEWAY","DOMAIN","DNS"
// value : a string
STATUS CConfigManagerApi::GetLastDHCPConfig(const std::string& key,
                                            std::string& value)
{
  CSegment* seg = new CSegment;
  CSegment  ret_seg;
  *seg << key;

  OPCODE opcode;
  STATUS res = SendMessageSync(seg,
                               CONFIGURATOR_GET_DHCP,
                               5*SECOND,
                               opcode,
                               ret_seg);

  if (res == STATUS_OK)
  {
    if (opcode != STATUS_NO_PERMISSION)
      ret_seg >> value;

    return opcode;
  }

  return res;
}

STATUS CConfigManagerApi::GetFirstDNSFromDHCP(std::string& dnsServerIP)
{
  std::string value;
  STATUS      ret = GetLastDHCPConfig("DNS", value);

  if (ret == STATUS_OK)
  {
    string::size_type loc = value.find(",", 0);
    if (loc == string::npos)
      dnsServerIP = value;
    else
      dnsServerIP = value.substr(0, loc);
  }

  return ret;
}

STATUS CConfigManagerApi::RemountVersionPartition(BOOL withWritePermission,
                                                  BOOL isToDeleteFiles /*=FALSE*/,
                                                  BOOL isToDeleteFallbackInNeeded /*=FALSE*/,
                                                  WORD timeOutInSec /*=15*/)
{
  CSegment* seg = new CSegment;

  *seg << withWritePermission
       << isToDeleteFiles
       <<  isToDeleteFallbackInNeeded;

  TRACESTR(eLevelInfoNormal) << "CConfigManagerApi::RemountVersionPartition ";

  OPCODE opcode;
  STATUS res = SendMessageSync(seg,
                               CONFIGURATOR_REMOUNT_VERSIONS,
                               timeOutInSec*SECOND,
                               opcode);
  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::HandleFallbackVersion(WORD timeOutInSec /*=15*/)
{
  OPCODE opcode;
  STATUS res = SendMessageSync(NULL,
                               CONFIGURATOR_HANDLE_FALLBACK,
                               timeOutInSec*SECOND,
                               opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::EnableDisablePing(BOOL enable)
{
  CSegment* seg = new CSegment;
  *seg << enable;

  OPCODE opcode;
  STATUS res = SendMessageSync(seg,
                               CONFIGURATOR_ENABLE_DISABLE_PING,
                               5 * SECOND,
                               opcode);
  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::EnableDisablePingBroadcast(BOOL enable)
{
  CSegment* seg = new CSegment;
  *seg << enable;

  OPCODE opcode;
  STATUS res = SendMessageSync(seg,
                               CONFIGURATOR_ENABLE_DISABLE_PING_BROADCAST,
                               5 * SECOND,
                               opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}

/*STATUS CConfigManagerApi::EnableDisablePingIptables(BOOL enable)
{
  CSegment* seg = new CSegment;
  *seg << (BYTE)enable;

  OPCODE opcode;
  STATUS res = SendMessageSync(seg,
                               CONFIGURATOR_ENABLE_DISABLE_PING_IPTABLES,
                               5 * SECOND,
                               opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}*/
STATUS CConfigManagerApi::OpenPortsForApache(std::string host,std::string ports)
{
	 CSegment* seg = new CSegment;
	 *seg << host << ports;
	 OPCODE opcode;
	 STATUS res = SendMessageSync(seg,
								    CONFIGURATOR_ADD_PORTS_IPTABLES,
	                                5 * SECOND,
	                                opcode);

	   if (res == STATUS_OK)
	     return opcode;
	   return res;
}
STATUS CConfigManagerApi::AddAdminUser(std::string username,
                                       std::string password)
{
  CSegment* seg = new CSegment;
  *seg <<username<<password;

  OPCODE opcode;
  STATUS res = SendMessageSync(seg,
                               CONFIGURATOR_ADD_USER,
                               5*SECOND,
                               opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::DelAdminUser(std::string username)
{
  CSegment* seg = new CSegment;
  *seg <<username;

  OPCODE opcode;
  STATUS res =  SendMessageSync(seg,
                                CONFIGURATOR_DEL_USER,
                                5*SECOND,
                                opcode);
  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::ChangePassword(std::string username,
                                         std::string new_password)
{
  CSegment* seg = new CSegment;
  *seg <<username<<new_password;

  OPCODE opcode;
  STATUS res = SendMessageSync(seg,
                               CONFIGURATOR_CHANGE_PASSWORD,
                               5*SECOND,
                               opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::RestartSnmpd()
{
  OPCODE opcode;
  STATUS res = SendMessageSync(NULL,
                               CONFIGURATOR_RESTART_SNMPD,
                               3*SECOND,
                               opcode);
  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::StartSnmpd()
{
  OPCODE opcode;
  STATUS res = SendMessageSync(NULL,
                               CONFIGURATOR_START_SNMPD,
                               3*SECOND,
                               opcode);
  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::StopSnmpd()
{
  OPCODE opcode;
  STATUS res = SendMessageSync(NULL,
                               CONFIGURATOR_STOP_SNMPD,
                               3*SECOND,
                               opcode);
  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::RestartSSHD(bool isPermanentNetwork,
                                      const std::string& ipV4 /*="*"*/,
                                      const std::string& ipV6 /*= ""*/,
                                      bool isOn)
{
  CSegment* seg = new CSegment;
  *seg <<  (BYTE)isPermanentNetwork;
  *seg <<  ipV4;
  *seg <<  ipV6;
  *seg <<  (BYTE)isOn;

  OPCODE opcode;
  STATUS res = SendMessageSync(seg,
                               CONFIGURATOR_RESTART_SSHD,
                               15*SECOND,
                               opcode);
  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::KillSshd()
{
  OPCODE opcode;
  STATUS res = SendMessageSync(NULL,
                               CONFIGURATOR_KILL_SSHD,
                               5*SECOND,
                               opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::SyncTime()
{
  OPCODE opcode;
  STATUS res = SendMessageSync(NULL,
                               CONFIGURATOR_SYNC_TIME,
                               5*SECOND,
                               opcode);
  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::TestDMA(const std::string& ide_name)
{
  CSegment* seg = new CSegment;
  *seg << ide_name;

  OPCODE opcode;
  STATUS res = SendMessageSync(seg,
                               CONFIGURATOR_TEST_DMA,
                               3*SECOND,
                               opcode);
  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::TestFlashDMA()
{
	//Check if system uses CF
        std::string answer;
        std::string cmd = "mount | grep "+MCU_CONFIG_DIR+" | grep /dev/hda2";
        STATUS stat = SystemPipedCommand(cmd.c_str(), answer);
        if(answer != "")
                return TestDMA("hda");
        //Otherwise, if SSD only, no need to test.
        else
                return STATUS_OK;
}

STATUS CConfigManagerApi::TestHardDiskDMA()
{
  std::string answer;
  // Check if HD is mounted on /dev/hdb (PATA)
  std::string cmd = "mount | grep "+MCU_OUTPUT_DIR+" | grep /dev/hdb";
  STATUS stat = SystemPipedCommand(cmd.c_str(),
                                   answer);
  if (answer != "")
  {
          return TestDMA("hdb");
  }
 
  return STATUS_OK;
}

STATUS CConfigManagerApi::TestEthernetSettings(const std::string& eth_name)
{
  CSegment* seg = new CSegment;
  *seg << eth_name;

  OPCODE opcode;
  STATUS res = SendMessageSync(seg,
                               CONFIGURATOR_TEST_ETH_SETTINGS,
                               3*SECOND,
                               opcode);
  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::ConfigureEthernetSettings(const eConfigInterfaceType ifType,
                                                    const ePortSpeedType portSpeed)
{
  CSegment* seg = new CSegment;
  *seg << (DWORD) ifType
       << (DWORD) portSpeed;

  OPCODE opcode;
  STATUS res = SendMessageSync(seg,
                               CONFIGURATOR_SET_ETH_SETTINGS,
                               3*SECOND,
                               opcode);
  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::EthernetSettingsMonitoring(ETH_SETTINGS_S& ethStruct,
                                                     const eConfigInterfaceNum ifNum)
{
  CSegment* seg = new CSegment;
  CSegment  ret_seg;

  *seg << (DWORD)ifNum;

  OPCODE opcode;
  STATUS stat = SendMessageSync(seg,
                                CONFIGURATOR_ETH_SETTINGS_MONITORING,
                                3*SECOND,
                                opcode,
                                ret_seg);

  if (stat != STATUS_OK)
    return stat;

  if (opcode != STATUS_OK)
    return opcode;

  ret_seg.Get((BYTE*)&ethStruct, sizeof(ETH_SETTINGS_S));

  return STATUS_OK;
}

// function CConfigManagerApi::GetNtpPeerStatus
// return status
// STATUS_FAIL - ntp query failed
// STATUS_NTP_PEER_REJECT - The peer is discarded as unreachable,
// synchronized to this server (synch loop) or outrageous synchronization distance.
// STATUS_NTP_PEER_FALSETICK - The peer is discarded by the intersection algorithm as a falseticker.
// STATUS_NTP_PEER_EXCESS - The peer is discarded as not among the first ten peers sorted by synchronization
// distance and so is probably a poor candidate for further consideration.
// STATUS_NTP_PEER_OUTLYER - The peer is discarded by the clustering algorithm as an outlyer.
// STATUS_NTP_PEER_CANDIDAT - The peer is a survivor and a candidate for the combining algorithm.
// STATUS_NTP_PEER_SELECTED - The peer is a survivor, but not among the first six peers sorted by synchronization distance.
// If the association is ephemeral, it may be demobilized to conserve resources.
// STATUS_NTP_PEER_SYS_PEER - The peer has been declared the system peer and lends its variables to the system variables.
// STATUS_NTP_PEER_PPS_PEER - The peer has been declared the system peer and lends its variables to the system variables.
// However, the actual system synchronization is derived from a pulse-per-second (PPS) signal,
// either indirectly via the PPS reference clock driver or directly via kernel interface.
STATUS CConfigManagerApi::GetNtpPeerStatus(const std::string& ip_address)
{
  CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();

  DWORD NtpPeerStatusPeriod = 0;

  sysConfig->GetDWORDDataByKey("NTP_PEER_STATUS_PERIOD", NtpPeerStatusPeriod);

  CSegment* seg = new CSegment;
  *seg << ip_address;

  OPCODE opcode;
  STATUS res = SendMessageSync(seg,
                               CONFIGURATOR_NTP_PEER_STATUS,
                               NtpPeerStatusPeriod*SECOND,
                               opcode);
  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::GetSMARTErrors(std::string& errorReport)
{
  OPCODE   opcode;
  CSegment seg;
  STATUS   res = SendMessageSync(NULL,
                                 CONFIGURATOR_GET_SMART_ERRORS,
                                 10 * SECOND,
                                 opcode,
                                 seg);
  if (res == STATUS_OK)
  {
    if (opcode != STATUS_OK &&
        opcode != STATUS_NO_PERMISSION &&
        opcode != STATUS_QUEUE_TIMEOUT)
      seg >> errorReport;

    return opcode;
  }

  return res;
}

STATUS CConfigManagerApi::RunSMARTSelftest(BOOL short_test)
{
  OPCODE    opcode;
  CSegment* seg = new CSegment;
  *seg << short_test;

  STATUS res = SendMessageSync(seg,
                               CONFIGURATOR_RUN_SMART_SELFTEST,
                               10*SECOND,
                               opcode);
  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::GetHDTemperature(std::string& temperature)
{
  OPCODE   opcode;
  CSegment seg;
  STATUS   res = SendMessageSync(NULL,
                                 CONFIGURATOR_GET_HD_TEMPERATURE,
                                 10*SECOND,
                                 opcode,
                                 seg);
  if (res == STATUS_OK)
  {
    if (STATUS_OK == opcode)
      seg >> temperature;

    return opcode;
  }

  return res;
}

STATUS CConfigManagerApi::GetHDSize(std::string& size)
{
  OPCODE   opcode;
  CSegment seg;
  STATUS   res = SendMessageSync(NULL,
                                 CONFIGURATOR_GET_HD_SIZE,
                                 10*SECOND,
                                 opcode,
                                 seg);
  if (res == STATUS_OK)
  {
    if (STATUS_OK == opcode)
      seg >> size;

    return opcode;
  }

  return res;
}

STATUS CConfigManagerApi::GetHDModel(std::string& details)
{
  OPCODE   opcode;
  CSegment seg;
  STATUS   res = SendMessageSync(NULL,
                                 CONFIGURATOR_GET_HD_MODEL,
                                 10*SECOND,
                                 opcode,
                                 seg);
  if (res == STATUS_OK)
  {
    if (STATUS_OK == opcode)
      seg >> details;

    return opcode;
  }

  return res;
}

STATUS CConfigManagerApi::GetHDFirmware(std::string& hd_firmware)
{
  OPCODE   opcode;
  CSegment seg;

  STATUS res = SendMessageSync(NULL,
                               CONFIGURATOR_GET_HD_FIRMWARE,
                               10*SECOND,
                               opcode,
                               seg);
  if (res == STATUS_OK)
  {
    if (STATUS_OK == opcode)
      seg >> hd_firmware;

    return opcode;
  }

  return res;
}

STATUS CConfigManagerApi::GetFlashSize(std::string& size)
{
  OPCODE   opcode;
  CSegment seg;

  STATUS res = SendMessageSync(NULL,
                               CONFIGURATOR_GET_FLASH_SIZE,
                               10*SECOND,
                               opcode,
                               seg);
  if (res == STATUS_OK)
  {
    if (STATUS_OK == opcode)
      seg >> size;

    return opcode;
  }

  return res;
}

STATUS CConfigManagerApi::GetFlashModel(std::string& details)
{
  OPCODE   opcode;
  CSegment seg;
  STATUS   res = SendMessageSync(NULL,
                                 CONFIGURATOR_GET_FLASH_MODEL,
                                 10*SECOND,
                                 opcode,
                                 seg);
  if (res == STATUS_OK)
  {
    if (STATUS_OK == opcode)
      seg >> details;

    return opcode;
  }

  return res;
}

STATUS CConfigManagerApi::GetCPUType(std::string& type)
{
  OPCODE   opcode;
  CSegment seg;

  STATUS res = SendMessageSync(NULL,
                               CONFIGURATOR_GET_CPU_TYPE,
                               10 * SECOND,
                               opcode, seg);
  if (res == STATUS_OK)
  {
    if (STATUS_OK == opcode)
      seg >> type;

    return opcode;
  }

  return res;
}

STATUS CConfigManagerApi::GetRAMSize(std::string& temperature)
{
  OPCODE   opcode;
  CSegment seg;
  STATUS   res = SendMessageSync(NULL,
                                 CONFIGURATOR_GET_RAM_SIZE,
                                 10*SECOND,
                                 opcode,
                                 seg);
  if (res == STATUS_OK)
  {
    if (STATUS_OK == opcode)
      seg >> temperature;

    return opcode;
  }

  return res;
}

STATUS CConfigManagerApi::TakeCoreFileOwnership(const std::string& core_name)
{
  CSegment* seg = new CSegment;
  *seg << core_name;
  OPCODE opcode;
  STATUS res = SendMessageSync(seg,
                               CONFIGURATOR_TAKE_CORE_OWNERSHIP,
                               5*SECOND,
                               opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}

// for Call Generator - change MM process priority
STATUS CConfigManagerApi::ChangeMyNiceLevel(int new_nice_level /*-19 to 20*/,
                                            int myTaskPid)
{
  if (CProcessBase::GetProcess()->GetProductFamily() !=
      eProductFamilyCallGenerator)
  {
    PTRACE(eLevelInfoNormal,
           "CConfigManagerApi::ChangeMyNiceLevel - ERROR - system is not CG!!");
    return STATUS_FAIL;
  }

  OPCODE    opcode;
  CSegment* seg = new CSegment;

  *seg << (DWORD)  new_nice_level;
  *seg << (DWORD)  myTaskPid;

  STATUS res = SendMessageSync(seg,
                               CONFIGURATOR_CHANGE_PID_NICE_LEVEL,
                               2*SECOND,
                               opcode);
  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::DeleteFile(const std::string& file_name)
{
  CSegment* seg = new CSegment;
  *seg << file_name;
  OPCODE opcode;
  STATUS res = SendMessageSync(seg,
                               CONFIGURATOR_DELETE_FILE,
                               5*SECOND,
                               opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}


STATUS CConfigManagerApi::DeleteDir(const std::string& dir_name)
{
  CSegment* seg = new CSegment;
  *seg << dir_name;
  OPCODE opcode;
  STATUS res = SendMessageSync(seg,
                               CONFIGURATOR_DELETE_DIR,
                               5*SECOND,
                               opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::SetTCPStackParams(const DWORD keepalive_tout,
                                            const DWORD keepalive_intvl)
{
  CSegment* seg = new CSegment;
  *seg << keepalive_tout
       << keepalive_intvl;
  OPCODE opcode;
  STATUS res = SendMessageSync(seg,
                               CONFIGURATOR_SET_TCP_STACK_PARAMS,
                               5*SECOND,
                               opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::DeleteTempFiles()
{
  CSegment* seg = new CSegment;

  OPCODE opcode;
  STATUS res = SendMessageSync(seg,
                               CONFIGURATOR_DEL_TEMP_FILES,
                               15*SECOND,
                               opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::SetProductType(const eProductType productType)
{
  CSegment* seg = new CSegment;
  *seg << (DWORD) productType;

  OPCODE opcode;
  STATUS res =  SendMessageSync(seg,
                                CONFIGURATOR_SET_PRODUCT_TYPE,
                                30*SECOND,
                                opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::EvokeNetworkInterfaces(eV6ConfigurationType ipconfigType)
{
  CSegment* seg = new CSegment;

  *seg << (DWORD)ipconfigType;
  OPCODE opcode;

  STATUS res =  SendMessageSync(seg,
                                CONFIGURATOR_EVOKE_NETWORK_INTERFACES,
                                30*SECOND,
                                opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::SetMacAddress(const eConfigInterfaceType ifNum,eIpType ipType,
                                        const std::string& macAddress)
{
  CSegment* seg = new CSegment;

  *seg << (DWORD) ifNum
	   << (DWORD)ipType
       << macAddress;

  OPCODE opcode;
  STATUS res =  SendMessageSync(seg,
                                CONFIGURATOR_SET_MAC_ADDRESS,
                                30*SECOND,
                                opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::AddRouteTableRule(eConfigInterfaceType networkType,
		                                    eIpType ipType,
                                            const std::string ip,
                                            const DWORD mask,
                                            const std::string defaultGW,
                                            const std::string ipv6SubNetMask,
                                            const std::list<CRouter> routerList,
                                            const BOOL bIPv6)
{
  CSegment* seg = new CSegment;

  *seg << (DWORD) networkType
		  << (DWORD) ipType
       << ip
       << mask
       << defaultGW
       << ipv6SubNetMask
       << bIPv6;

  *seg << (DWORD) routerList.size();
  for (std::list<CRouter>::const_iterator itr = routerList.begin();
       itr != routerList.end();
       itr++)
  {
    DWORD temp = (DWORD)itr->m_type;
    *seg << (DWORD)temp
         << itr->m_targetIP
         << itr->m_subNetmask
         << itr->m_gateway;
  }

  OPCODE opcode;
  STATUS res =  SendMessageSync(seg,
                                CONFIGURATOR_ADD_DEFAULT_GW_ROUTE_RULE,
                                30*SECOND,
                                opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::DelRouteTableRule(eConfigInterfaceType networkType,
		                                    eIpType ipType,
                                            const std::string ip,
                                            const DWORD mask,
                                            const std::string defaultGW,
                                            const BOOL bIPv6)
{
  CSegment* seg = new CSegment;

  *seg << (DWORD) networkType
		  << (DWORD) ipType
       << ip
       << mask
       << defaultGW
       << bIPv6;

  OPCODE opcode;
  STATUS res =  SendMessageSync(seg,
                                CONFIGURATOR_DEL_DEFAULT_GW_ROUTE_RULE,
                                30*SECOND,
                                opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::ExposeEmbeddedNewVersion()
{
  CSegment* seg = new CSegment;

  OPCODE opcode;
  STATUS res =  SendMessageSync(seg,
                                CONFIGURATOR_EXPOSE_EMBEDDED_NEW_VERSION,
                                30*SECOND,
                                opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::AddDropRuleToShorewall(std::string drop_ip,
                                                 BOOL drop_ping_only)
{
  CSegment* seg = new CSegment;
  OPCODE    opcode;

  *seg << drop_ip;
  *seg << drop_ping_only;

  STATUS res =  SendMessageSync(seg,
                                CONFIGURATOR_ADD_DROP_RULE_SHOREWALL,
                                30*SECOND,
                                opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::RestoreFallbackVersion()
{
  CSegment* seg = new CSegment;
  OPCODE    opcode;

  STATUS res =  SendMessageSync(seg,
                                CONFIGURATOR_RESTORE_FALLBACK_VERSION,
                                30*SECOND,
                                opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::StopCheckEth0Link()
{
  CSegment* seg = new CSegment;
  OPCODE    opcode;
  STATUS    res = STATUS_FAIL;

  res =  SendMessageSync(seg,
                         CONFIGURATOR_STOP_CHECK_ETH0_LINK,
                         30*SECOND,
                         opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::StartCheckEth0Link()
{
  CSegment* seg = new CSegment;
  OPCODE    opcode;
  STATUS    res = STATUS_FAIL;

  res =  SendMessageSync(seg,
                         CONFIGURATOR_START_CHECK_ETH0_LINK,
                         30*SECOND,
                         opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::EnableWhiteList(CPObject* pWhiteList)
{	 
	  OPCODE    opcode;
	  STATUS    res = STATUS_FAIL;
	  CSegment* seg = ( CSegment*)pWhiteList;
	  res =  SendMessageSync(seg,
							  CONFIGURATOR_ENABLE_WHITE_LIST,
	                         30*SECOND,
	                         opcode);

	  if (res == STATUS_OK)
	    return opcode;

	  return res;
}

STATUS CConfigManagerApi::DisableWhiteList()
{
	 OPCODE    opcode;
	 STATUS    res = STATUS_FAIL;
	 CSegment* seg = new CSegment;
	 res =  SendMessageSync(seg,
							 CONFIGURATOR_DISABLE_WHITE_LIST,
	                       30*SECOND,
	                        opcode);

	if (res == STATUS_OK)
	    return opcode;

	return res;
}
STATUS CConfigManagerApi::ClearTcpDumpStorage()
{
  CSegment* seg = new CSegment;
  OPCODE    opcode;

  STATUS res =  SendMessageSync(seg,
                                CLEAR_TCP_DUMP_STORAGE,
                                30*SECOND,
                                opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}


STATUS CConfigManagerApi::StartTcpDump(const DWORD entitiyType,const std::string filterStr,
                                       char* startTimeStr,
                                       char* ipStr, DWORD allocatedUnits,
                                       eIpType	ipType,
                                       std::string& result)
{
  OPCODE    opcode;
  CSegment* seg = new CSegment;

  *seg << entitiyType;
  *seg << filterStr;
  *seg << startTimeStr;
  *seg << ipStr;
  *seg << allocatedUnits;
  *seg << (DWORD)ipType;

  CSegment ret_seg;

  TRACESTR(eLevelInfoNormal) << "CConfigManagerApi::StartTcpDump filterStr " <<
  filterStr.c_str();

  STATUS stat =  SendMessageSync(seg,
                                 START_TCP_DUMP,
                                 30*SECOND,
                                 opcode,
                                 ret_seg);

  TRACESTR(eLevelInfoNormal) << "CConfigManagerApi::StartTcpDump opcode = " <<
  opcode;
  TRACESTR(eLevelInfoNormal) << "CConfigManagerApi::StartTcpDump stat = " << stat;


  if (stat != STATUS_OK)
    return stat;

  if (opcode != STATUS_OK)
    return opcode;

  ret_seg >> result;
  TRACESTR(eLevelInfoNormal) << "CConfigManagerApi::StartTcpDump ret_seg result= "
                         << result.c_str();

  return STATUS_OK;
}

STATUS CConfigManagerApi::StopTcpDump()
{
  CSegment* seg = new CSegment;
  OPCODE    opcode;

  STATUS res =  SendMessageSync(seg,
                                STOP_TCP_DUMP,
                                30*SECOND,
                                opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}


STATUS CConfigManagerApi::ReadValueFromRegisterBySensor(std::string& ans,
                                                        const std::string sensor,
                                                        const std::string register_address)
{
  CSegment* params = new CSegment;

  *params << sensor;
  *params << register_address;

  CSegment seg;
  OPCODE   opcode;
  STATUS   res =  SendMessageSync(params,
                                  CONFIGURATOR_GET_VALUE_FROM_REGISTER,
                                  30*SECOND,
                                  opcode,
                                  seg);

  if (res == STATUS_OK)
  {
    if (STATUS_OK == opcode)
      seg >> ans;

    return opcode;
  }

  return res;
}

STATUS CConfigManagerApi::AddNatRule(std::string& ansdeleteCmd,std::string&	TargetIpDnsAddress,std::string&	SourceIpAddress,
										std::string&	ports,DWORD	isIpv4,DWORD	isMultiPorts,string protocol)
{

   CSegment* params = new CSegment;
  *params << TargetIpDnsAddress << SourceIpAddress << ports <<  isIpv4 << isMultiPorts << protocol;

  CSegment seg;
  OPCODE   opcode;
  STATUS   res =  SendMessageSync(params,
		  	  	  	  	  	  	  CONFIGURATOR_ADD_NAT_RULE,
                                  30*SECOND,
                                  opcode,
                                  seg);

  if (res == STATUS_OK)
  {
    if (STATUS_OK == opcode)
      seg >> ansdeleteCmd;

    return opcode;
  }



  return res;
}

STATUS CConfigManagerApi::DeleteAddNatRule(std::string& deleteNateCmd)
{

   CSegment* params = new CSegment;
  *params << deleteNateCmd;

  CSegment seg;
  OPCODE   opcode;
  STATUS   res =  SendMessageSync(params,
		  	  	  	  	  	  	  CONFIGURATOR_DELETE_NAT_RULE,
                                  30*SECOND,
                                  opcode,
                                  seg);

  if (res == STATUS_OK)
  {
    if (STATUS_OK != opcode)
    	return opcode;
  }

  return res;
}
STATUS CConfigManagerApi::ReadTempFromAdvantechUtil(std::string& ans)
{
  CSegment* params = new CSegment;


  CSegment seg;
  OPCODE   opcode;
  STATUS   res =  SendMessageSync(params,
		  CONFIGURATOR_GET_TEMP_ADVANTECH_UTIL,
		  30*SECOND,
		  opcode,
		  seg);

  if (res == STATUS_OK)
  {
    if (STATUS_OK == opcode)
      seg >> ans;

    return opcode;
  }

  return res;
}
STATUS CConfigManagerApi::CheckCSIpConfig(DWORD CSIpAddress, DWORD& retVal)
{
  OPCODE    opcode;
  CSegment* pSeg = new CSegment;

  CSegment seg;

  *pSeg << (DWORD)CSIpAddress;

  STATUS res = SendMessageSync(pSeg,
                               CONFIGURATOR_CHECK_CS_IP_CONFIG,
                               5*SECOND,
                               opcode,
                               seg);




  TRACESTR(eLevelInfoNormal) << "CConfigManagerApi::CheckCSIpConfig  res = " << res;
  if (res == STATUS_OK)
  {
    TRACESTR(eLevelInfoNormal) << "CConfigManagerApi::CheckCSIpConfig  STATUS_OK";
    if (STATUS_OK == opcode)
    {
      seg >> retVal;
      TRACESTR(eLevelInfoNormal) << "CConfigManagerApi::CheckCSIpConfig  opcode == STATUS_OK "
                             << retVal;
    }

    TRACESTR(eLevelInfoNormal) << "CConfigManagerApi::CheckCSIpConfig  opcode = "
                           << opcode;
    return opcode;
  }

  return res;
}

STATUS CConfigManagerApi::CheckCSEth2(BOOL& retVal)
{
  OPCODE    opcode;
  CSegment seg;

  STATUS res = SendMessageSync(NULL,
		  	  	  	  	  CONFIGURATOR_CHECK_ETH2_LINK,
                               5*SECOND,
                               opcode,
                               seg);


  if (res != STATUS_OK)
  {
	  TRACESTR(eLevelInfoNormal) << "Failed sending CONFIGURATOR_CHECK_ETH2_LINK";
	  return res;
  }
  if (opcode != STATUS_OK)
  {
	  TRACESTR(eLevelInfoNormal) << "Failed executing CONFIGURATOR_CHECK_ETH2_LINK  opcode " << (int)opcode;
	  return res;
  }
  seg >> retVal;

  // TRACESTR(eLevelInfoNormal) << "CheckCSEth2 retVal  " << (int)retVal;
  
  return STATUS_OK;

}
STATUS CConfigManagerApi::AddIpandNatRuleForDNSPerService(char * ipDnsAddress, std::string route_table, int priority, bool isIpv4, char* ipAddressStr, DWORD& retVal)
{
  OPCODE    opcode;
  CSegment* pSeg = new CSegment;

  CSegment seg;

  *pSeg << ipDnsAddress;
  *pSeg << route_table;
  *pSeg << (DWORD)priority;
  *pSeg << (DWORD)isIpv4;
  *pSeg << (DWORD)ipAddressStr;


  STATUS res = SendMessageSync(pSeg,
		  ADD_IP_AND_NAT_RULE_FOR_DNS_PER_SERVICE,
                               5*SECOND,
                               opcode,
                               seg);

  return res;
}

STATUS CConfigManagerApi::HandleNtpService(const ServiceCommand& cmd)
{
  CSegment* pSeg = new CSegment;
  *pSeg << (DWORD)cmd;

  OPCODE opcode;
  STATUS res = SendMessageSync(pSeg,
                  CONFIGURATOR_NTP_SERVICE,
                               5*SECOND,
                               opcode);

  if (res == STATUS_OK)
    return opcode;

  return res;
}

STATUS CConfigManagerApi::ConfigNtpServers(const std::string& s1, const std::string& s2, const std::string& s3)
{
  CSegment* pSeg = new CSegment;
  *pSeg << s1 << s2 << s3;

  OPCODE opcode;
  STATUS res = SendMessageSync(pSeg,
                               CONFIGURATOR_CONFIG_NTP_SERVERS,
                               15*SECOND,
                               opcode);
  if (res == STATUS_OK)
    return opcode;

  return res;
}
STATUS CConfigManagerApi::GetUdpOccupiedPorts(std::string& if_name)
{
  CSegment* pSeg = new CSegment;
  *pSeg << if_name;

  STATUS res = SendMessageSync(pSeg, CONFIGURATOR_GET_UDP_OCCUPIED_PORTS, 5*SECOND);

  return res;
}
