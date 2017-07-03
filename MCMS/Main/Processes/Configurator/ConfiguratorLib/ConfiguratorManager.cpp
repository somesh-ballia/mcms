// ConfiguratorManager.cpp

#include "ConfiguratorManager.h"
#include "ConfiguratorManagerSoftMcu.h"

#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sstream>
#include <shadow.h>
#include <streambuf>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>

#include "ConfigManagerOpcodes.h"
#include "Trace.h"
#include "StatusesGeneral.h"
#include "Segment.h"
#include "SystemFunctions.h"
#include "TraceStream.h"
#include "ConfigManagerApi.h"
#include "ApiStatuses.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "TraceStream.h"
#include "OsFileIF.h"
#include "IfConfig.h"
#include "InternalProcessStatuses.h"
#include "TerminalCommand.h"
#include "EthernetSettingsUtils.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "StringsLen.h"
#include "WhiteList.h"
#include "CommonStructs.h"
#include "McuMngrStructs.h"

#define CONFIG_SSHD_TIMER			314314314
#define CONFIG_CHECK_ZOMBIE_TOUT	1000

#define ZOMBIE_TOUT		60*SECOND
#define MNGMNT_ENTITY 0

#define inaddrr(x) (*(struct in_addr *) &ifr->x[sizeof sa.sin_port])
#define TEST_ROOT_AND_RETURN \
	if (TestRoot() == FALSE) \
	{ \
		ResponedClientRequest(STATUS_NO_PERMISSION); \
		return; \
	}

extern bool IsJitcAndNetSeparation();

extern void ConfiguratorMonitorEntryPoint(void* appParam);

//########## CConfiguratorManager Factory function ##############
void ConfiguratorManagerEntryPoint(void* appParam)
{
	CConfiguratorManager * pConfiguratorManager = NULL;
	eProductType curProductType  = CProcessBase::GetProcess()->GetProductType();

	if (eProductTypeEdgeAxis == curProductType)
	{
		pConfiguratorManager = new CConfiguratorManagerSoftMcu;
	}
	else
		pConfiguratorManager = new CConfiguratorManager;

	pConfiguratorManager->Create(*(CSegment*)appParam);
}

TaskEntryPoint CConfiguratorManager::GetMonitorEntryPoint()
{
	return ConfiguratorMonitorEntryPoint;
}

//---------------------------------------------------------------------

PBEGIN_MESSAGE_MAP(CConfiguratorManager)
    ONEVENT(XML_REQUEST                                ,IDLE , CConfiguratorManager::HandlePostRequest )
    ONEVENT(CONFIGURATOR_SYNC_TIME                     ,IDLE , CConfiguratorManager::SyncTimeWithSwitch )
    ONEVENT(CONFIGURATOR_ARPING_REQUEST                ,IDLE , CConfiguratorManager::McuMngrArpingRequest )
    ONEVENT(CONFIGURATOR_DAD_REQUEST                   ,IDLE , CConfiguratorManager::DadRequest )
    ONEVENT(CONFIGURATOR_FAILOVER_ARPING_REQUEST       ,IDLE , CConfiguratorManager::FailoverArpingRequest )
    ONEVENT(CONFIGURATOR_ADD_IP_INTERFACE              ,IDLE , CConfiguratorManager::AddIpInterface )
    ONEVENT(CONFIGURATOR_ADD_IPV6_INTERFACE            ,IDLE , CConfiguratorManager::AddIpV6Interface )
    ONEVENT(CONFIGURATOR_ADD_VLAN_INTERFACE            ,IDLE , CConfiguratorManager::AddVlan)
    ONEVENT(CONFIGURATOR_GET_NEW_VERSION_NUMBER        ,IDLE , CConfiguratorManager::GetNewVersionNumber )
    ONEVENT(CONFIGURATOR_REMOVE_IP_INTERFACE           ,IDLE , CConfiguratorManager::RemoveIpInterface )
    ONEVENT(CONFIGURATOR_DNS_SERVIER                   ,IDLE , CConfiguratorManager::ConfigureDnsServers )
    ONEVENT(CONFIGURATOR_MORE_DNS_SERVER               ,IDLE , CConfiguratorManager::ConfigureMoreDnsServers )
    ONEVENT(CONFIGURATOR_NS_UPDATE                     ,IDLE , CConfiguratorManager::NameServerUpdate )
    ONEVENT(CONFIGURATOR_RUN_DHCP                      ,IDLE , CConfiguratorManager::RunDHCP )
    ONEVENT(CONFIGURATOR_KILL_DHCP                     ,IDLE , CConfiguratorManager::KillDHCP )
    ONEVENT(CONFIGURATOR_GET_DHCP                      ,IDLE , CConfiguratorManager::GetLastDHCPConfiguration )
    ONEVENT(CONFIGURATOR_REMOUNT_VERSIONS              ,IDLE , CConfiguratorManager::RemountVersionPartition )
    ONEVENT(CONFIGURATOR_HANDLE_FALLBACK               ,IDLE , CConfiguratorManager::HandleFallbackVersion )
    ONEVENT(CONFIGURATOR_CYCLE_VERSION                 ,IDLE , CConfiguratorManager::CycleVersions )
    ONEVENT(CONFIGURATOR_ENABLE_DISABLE_PING           ,IDLE , CConfiguratorManager::EnableDisablePing )
    ONEVENT(CONFIGURATOR_ENABLE_DISABLE_PING_BROADCAST ,IDLE , CConfiguratorManager::EnableDisablePingBroadcast )
    //ONEVENT(CONFIGURATOR_ENABLE_DISABLE_PING_IPTABLES  ,IDLE , CConfiguratorManager::EnableDisablePingIptables)
    ONEVENT(CONFIGURATOR_ADD_USER                      ,IDLE , CConfiguratorManager::AddRootUserToSystem )
    ONEVENT(CONFIGURATOR_DEL_USER                      ,IDLE , CConfiguratorManager::DelUserFromSystem )
    ONEVENT(CONFIGURATOR_CHANGE_PASSWORD               ,IDLE , CConfiguratorManager::ChangePassword )
    ONEVENT(CONFIGURATOR_RESTART_SNMPD                 ,IDLE , CConfiguratorManager::RestartSnmpd )
    ONEVENT(CONFIGURATOR_START_SNMPD                   ,IDLE , CConfiguratorManager::StartSnmpd )
    ONEVENT(CONFIGURATOR_STOP_SNMPD                    ,IDLE , CConfiguratorManager::StopSnmpd )
    ONEVENT(CONFIGURATOR_RESTART_APACHE                ,IDLE , CConfiguratorManager::RestartApache )
    ONEVENT(CONFIGURATOR_TEST_DMA                      ,IDLE , CConfiguratorManager::GetDMAStatus )
    ONEVENT(CONFIGURATOR_TEST_ETH_SETTINGS             ,IDLE , CConfiguratorManager::TestEthSetting )
    ONEVENT(CONFIGURATOR_SET_ETH_SETTINGS              ,IDLE , CConfiguratorManager::SetEthSettings )
    ONEVENT(CONFIGURATOR_GET_SMART_ERRORS              ,IDLE , CConfiguratorManager::GetSmartErrors )
    ONEVENT(CONFIGURATOR_RUN_SMART_SELFTEST            ,IDLE , CConfiguratorManager::RunSmartSelfTest )
    ONEVENT(CONFIGURATOR_RESTART_SSHD                  ,IDLE , CConfiguratorManager::RestartSSH )
    ONEVENT(CONFIGURATOR_TAKE_CORE_OWNERSHIP           ,IDLE , CConfiguratorManager::TakeCoreOwnership )
    ONEVENT(CONFIGURATOR_SET_PRODUCT_TYPE              ,IDLE , CConfiguratorManager::SetProductType )
    ONEVENT(CONFIGURATOR_SET_SPECIAL_PRODUCT_TYPE      ,IDLE , CConfiguratorManager::SetSpecialProductType )
    ONEVENT(CONFIGURATOR_EVOKE_NETWORK_INTERFACES      ,IDLE , CConfiguratorManager::EvokeNetworkInterfaces )
    ONEVENT(CONFIGURATOR_SET_MAC_ADDRESS               ,IDLE , CConfiguratorManager::SetMacAddress )
    ONEVENT(CONFIGURATOR_DELETE_FILE                   ,IDLE , CConfiguratorManager::DeleteFile )
    ONEVENT(CONFIGURATOR_DELETE_DIR                   ,IDLE , CConfiguratorManager::DeleteDir )
    ONEVENT(CONFIGURATOR_SET_TCP_STACK_PARAMS          ,IDLE , CConfiguratorManager::SetTCPStackParams )
    ONEVENT(CONFIG_SSHD_TIMER   			      ,IDLE , CConfiguratorManager::OnConfigSSHTimeout)
    ONEVENT(CONFIG_INTERFACE_UP	   		      ,IDLE , CConfiguratorManager::InterfaceUp)
    ONEVENT(CONFIG_RUN_COMMAND	   		      ,IDLE , CConfiguratorManager::RunningCommand)
    ONEVENT(CONFIG_ADD_DEFAULT_GW   		      ,IDLE , CConfiguratorManager::AddDefaultGW)
    ONEVENT(CONFIG_ADD_STATIC_ROUTE   		      ,IDLE , CConfiguratorManager::AddStaticRoutes)
    ONEVENT(CONFIG_ADD_STATIC_IP_ROUTE   	      ,IDLE , CConfiguratorManager::AddStaticIpRoutes)
    ONEVENT(CONFIG_IPV6_AUTOCONFIG                     ,IDLE , CConfiguratorManager::SetIpv6AutoConfig)
    ONEVENT(CONFIGURATOR_ADD_DEFAULT_GW_ROUTE_RULE     ,IDLE , CConfiguratorManager::AddDefaultGWRoutingTableRule )
    ONEVENT(CONFIGURATOR_DEL_DEFAULT_GW_ROUTE_RULE     ,IDLE , CConfiguratorManager::DelDefaultGWRoutingTableRule )
    ONEVENT(CONFIGURATOR_DEL_TEMP_FILES                ,IDLE , CConfiguratorManager::DeleteTempFiles )
    ONEVENT(CONFIGURATOR_GET_HD_TEMPERATURE            ,IDLE , CConfiguratorManager::GetHDDTemperature )

    ONEVENT(CONFIGURATOR_GET_HD_SIZE                   ,IDLE , CConfiguratorManager::GetHDDSize )
    ONEVENT(CONFIGURATOR_GET_HD_MODEL	              ,IDLE , CConfiguratorManager::GetHDDModel )
    ONEVENT(CONFIGURATOR_GET_HD_FIRMWARE		      ,IDLE , CConfiguratorManager::GetHDDFirmware )

    ONEVENT(CONFIGURATOR_GET_FLASH_SIZE                ,IDLE , CConfiguratorManager::GetFlashSize )
    ONEVENT(CONFIGURATOR_GET_FLASH_MODEL               ,IDLE , CConfiguratorManager::GetFlashModel )
    ONEVENT(CONFIGURATOR_GET_CPU_TYPE                  ,IDLE , CConfiguratorManager::GetCPUDetails )
    ONEVENT(CONFIGURATOR_GET_RAM_SIZE		      ,IDLE , CConfiguratorManager::GetRAMsize )

    ONEVENT(CONFIGURATOR_ETH_SETTINGS_MONITORING       ,IDLE , CConfiguratorManager::EthSettingsMonitoring )
    ONEVENT(CONFIGURATOR_MOUNT_NEW_VERSION             ,IDLE , CConfiguratorManager::MountNewVersion )
    ONEVENT(CONFIGURATOR_UNMOUNT_NEW_VERSION           ,IDLE , CConfiguratorManager::UnmountNewVersion )
    ONEVENT(CONFIGURATOR_FIRMWARE_CHECK                ,IDLE , CConfiguratorManager::FirmwareCheck )
    ONEVENT(CONFIGURATOR_EXPOSE_EMBEDDED_NEW_VERSION   ,IDLE , CConfiguratorManager::ExposeEmbeddedNewVersion )
    ONEVENT(CONFIGURATOR_ADD_DROP_RULE_SHOREWALL	      ,IDLE , CConfiguratorManager::AddDropRuleToIptables )
    ONEVENT(CONFIGURATOR_ADD_ROUTE_RULE                ,IDLE , CConfiguratorManager::AddNICRoutingTableRule )
    ONEVENT(CONFIGURATOR_RESTORE_FALLBACK_VERSION      ,IDLE , CConfiguratorManager::RestoreFallbackVersion )
    ONEVENT(CONFIGURATOR_STOP_CHECK_ETH0_LINK 	      ,IDLE , CConfiguratorManager::StopCheckEth0Link )
    ONEVENT(CONFIGURATOR_START_CHECK_ETH0_LINK 	      ,IDLE , CConfiguratorManager::StartCheckEth0Link )
    ONEVENT(CONFIGURATOR_CHECK_ETH2_LINK 	      ,IDLE , CConfiguratorManager::OnCheckEth2 )
    ONEVENT(CHECK_ETH0_LINK_TIMER                      ,IDLE , CConfiguratorManager::OnCheckEth0LinkTimeout )
    ONEVENT(CLEAR_TCP_DUMP_STORAGE                     ,IDLE , CConfiguratorManager::ClearTcpDumpStorage )
    ONEVENT(START_TCP_DUMP      	                      ,IDLE , CConfiguratorManager::StartTcpDump )
    ONEVENT(STOP_TCP_DUMP      	                      ,IDLE , CConfiguratorManager::StopTcpDump )
    
    ONEVENT(CONFIG_CHECK_ZOMBIE_TOUT                   ,IDLE , CConfiguratorManager::OnCheckChildZombieProcessesTimer )
    ONEVENT(CONFIGURATOR_WRITE_FILE                    ,IDLE , CConfiguratorManager::OnWriteFile )
    ONEVENT(CONFIGURATOR_WRITE_FILE_ROOT_ONLY          ,IDLE , CConfiguratorManager::OnWriteFileRootOnly )
    
    ONEVENT(CONFIGURATOR_KILL_SSHD                     ,IDLE , CConfiguratorManager::KillSsh )
    ONEVENT(CONFIGURATOR_GET_VALUE_FROM_REGISTER       ,IDLE , CConfiguratorManager::GetValueFromRegister )
    ONEVENT(CONFIGURATOR_CHECK_CS_IP_CONFIG            ,IDLE , CConfiguratorManager::CheckCSIpConfig )
    ONEVENT(CONFIGURATOR_GET_TEMP_ADVANTECH_UTIL       ,IDLE , CConfiguratorManager::GetTempFromAdvantechUtil )
    ONEVENT(CONFIGURATOR_CONFIG_BONDING_INTERFACE      ,IDLE , CConfiguratorManager::ConfigBondingInterface )
    ONEVENT(CONFIGURATOR_SET_BONDING_INTERFACE_SLAVES  ,IDLE , CConfiguratorManager::SetBondingInterfaceSlaves )
    ONEVENT(CONFIGURATOR_ENABLE_WHITE_LIST	           ,IDLE , CConfiguratorManager::EnableWhiteList )
    ONEVENT(CONFIGURATOR_ADD_PORTS_IPTABLES		   		,IDLE , CConfiguratorManager::AddPortsIpTables )
    ONEVENT(CONFIGURATOR_DISABLE_WHITE_LIST            ,IDLE , CConfiguratorManager::DisableWhiteList )
    ONEVENT(CONFIGURATOR_QOS_MANAGEMENT_DSCP		   ,IDLE , CConfiguratorManager::AddQosManagementRuleToIptables )
    ONEVENT(CONFIG_NETWORK_SEPERATION_IPV6_VLAN_I	   ,IDLE , CConfiguratorManager::OnConfigNetworkSeperationIpv6Timeout)
    ONEVENT(CONFIG_NETWORK_SEPERATION_IPV6_VLAN_II	   ,IDLE , CConfiguratorManager::OnConfigNetworkSeperationIpv6Timeout)
    ONEVENT(ADD_IP_AND_NAT_RULE_FOR_DNS_PER_SERVICE            ,IDLE , CConfiguratorManager::AddIpAndNatRuleForDnsPerService )

    ONEVENT(CONFIGURATOR_FPGA_UPGRADE		   ,IDLE , CConfiguratorManager::FPGAUpgrade )

    ONEVENT(CONFIGURATOR_ADD_NAT_RULE		   ,IDLE , CConfiguratorManager::AddNatRule )
    ONEVENT(CONFIGURATOR_DELETE_NAT_RULE	   ,IDLE , CConfiguratorManager::DeleteNatRule )

    ONEVENT(CONFIGURATOR_ENABLE_DHCP_IPV6     ,IDLE , CConfiguratorManager::EnableDHCPIPv6 )
    ONEVENT(CONFIGURATOR_DISABLE_DHCP_IPV6    ,IDLE , CConfiguratorManager::DisableDHCPIPv6 )
    
    /*Begin:added by Richer for BRIDGE-15015, 11/13/2014*/
    ONEVENT(CONFIGURATOR_LED_SYS_ALARM                     ,IDLE , CConfiguratorManager:: OnLEDSysAlarmIndication)
    /*End:added by Richer for BRIDGE-15015, 11/13/2014*/
	ONEVENT(CONFIGURATOR_GET_UDP_OCCUPIED_PORTS    ,IDLE , CConfiguratorManagerSoftMcu::GetUdpOccupiedPorts)

PEND_MESSAGE_MAP(CConfiguratorManager,CManagerTask);

BEGIN_SET_TRANSACTION_FACTORY(CConfiguratorManager)
END_TRANSACTION_FACTORY

BEGIN_TERMINAL_COMMANDS(CConfiguratorManager)
  ONCOMMAND("arping", CConfiguratorManager::ArpingTerminalRequest,    "arping")
  ONCOMMAND("arping_f", CConfiguratorManager::ArpingFTerminalRequest, "arping_f")
END_TERMINAL_COMMANDS


CConfiguratorManager::CConfiguratorManager()
{
    eProductType const productType = CProcessBase::GetProcess()->GetProductType();
    m_productTypeDecider = new ProductTypeDecider(productType);
}

CConfiguratorManager::~CConfiguratorManager()
{
    delete m_productTypeDecider;
    m_productTypeDecider = NULL;
}

// Private, Static
STATUS CConfiguratorManager::EnableICMPv6()
{
  if ( FALSE == IsTarget())
	return STATUS_OK;

  std::string ans;
  const char* cmd = "ip6tables --list | grep icmp | grep DROP 2>&1";

  STATUS stat = SystemPipedCommand(cmd, ans);
  FPASSERTSTREAM_AND_RETURN_VALUE(STATUS_OK != stat,
      cmd << ": " << ans << ": " << stat,
      STATUS_FAIL);

  switch (ans[0])
  {
  case '\0':
    // It is enabled already
    break;

  case 'D':
    cmd = "ip6tables -D OUTPUT -p icmpv6 --icmpv6-type 135 -j DROP 2>&1";
    stat = SystemPipedCommand(cmd, ans);
    FPASSERTSTREAM_AND_RETURN_VALUE(stat != STATUS_OK,
        cmd << ": " << ans << ": " << stat,
        STATUS_FAIL);
    break;

  default:
    FPASSERTSTREAM_AND_RETURN_VALUE(true, "Illegal value " << ans, STATUS_FAIL);
  }

  return STATUS_OK;
}

void CConfiguratorManager::ManagerPostInitActionsPoint()
{
  StartTimer(CONFIG_CHECK_ZOMBIE_TOUT, ZOMBIE_TOUT);

  CProcessBase* proc = CProcessBase::GetProcess();
  PASSERT_AND_RETURN(NULL == proc);

  CSysConfig* cfg = proc->GetSysConfig();
  PASSERT_AND_RETURN(NULL == cfg);

  BOOL val;
  BOOL res = cfg->GetBOOLDataByKey(CFG_KEY_DUPLICATE_IP_DETECTION, val);
  PASSERTSTREAM_AND_RETURN(!res,
      "CSysConfig::GetBOOLDataByKey: " << CFG_KEY_DUPLICATE_IP_DETECTION);

  if (val)
    EnableICMPv6();

  // Configures TCP retransmission timeout
  if (IsTarget())
  {
    DWORD val2;
    BOOL res2 = cfg->GetDWORDDataByKey(CFG_KEY_TCP_RETRANSMISSION_TIMEOUT, val2);
    PASSERTSTREAM_AND_RETURN(!res2,
        "CSysConfig::GetDWORDDataByKey: " << CFG_KEY_TCP_RETRANSMISSION_TIMEOUT);

    std::string ans;
    std::ostringstream cmd;
    cmd << "echo " << val2 << " /proc/sys/net/ipv4/tcp_retries2 2>&1";

    STATUS status = SystemPipedCommand(cmd.str().c_str(), ans);
    PASSERTSTREAM_AND_RETURN(STATUS_OK != status,
        cmd.str() << ": " << ans << ": " << status);
  }
}

BOOL CConfiguratorManager::TestRoot()
{
    eProductType curProductType = m_productTypeDecider->GetProductType();

    if ((eProductTypeGesher == curProductType) || (eProductTypeNinja == curProductType)	)
    {
        return TRUE;
    }
	gid_t id = getgid();
	return (id == 0) ? TRUE : FALSE;
}

void CConfiguratorManager::McuMngrArpingRequest(CSegment* pSeg)
{
	ArpingRequest(pSeg,false);
}

eConfigInterfaceType CConfiguratorManager::GetMngrIfType(const eIpType ipType)
{
	
	eConfigInterfaceType ifType = eManagmentNetwork;
	
	//for gesher/ninja lanredundancy
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	if (eProductTypeGesher == curProductType || eProductTypeNinja == curProductType)
	{
		CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
		BOOL isLAN_REDUNDANCY = YES;
		//BOOL res = sysConfig->GetBOOLDataByKey(CFG_KEY_LAN_REDUNDANCY, isLAN_REDUNDANCY);
		isLAN_REDUNDANCY = CProcessBase::GetProcess()->GetLanRedundancy(ipType);
		
		if (isLAN_REDUNDANCY)
		{
			//for gesher/ninja lanredundancy
			ifType = eLanRedundancyManagementNetwork;
		}
	}

	BOOL isManagmentSep = IsSeparatedNetworks();
	if(isManagmentSep)
	{
		ifType = eSeparatedManagmentNetwork;
	}

	return ifType;
}

void CConfiguratorManager::ArpingRequest(CSegment* pSeg,bool isFailover)
{
	eProductType curProductType = m_productTypeDecider->GetProductType();
	
	/*BRIDGE-2802:  for Gesher and Ninja need to enable this command for failover*/
	
	if ( (FALSE == IsTarget()) && 
		 (curProductType != eProductTypeGesher) && 
		 (curProductType != eProductTypeNinja))
	{
		CSegment*  seg = new CSegment;
		*seg << "Done";

		ResponedClientRequest(STATUS_OK,seg);
		return;
	}

	TEST_ROOT_AND_RETURN
    std::string ip;
    std::string answer;
	std::string sourceIP;
	DWORD tmpIpType;
    eIpType ipType;

	if(isFailover)
	{
		*pSeg >> sourceIP;
	}

	*pSeg >> ip;
	*pSeg >> tmpIpType;
	ipType = (eIpType)tmpIpType;

    STATUS stat = STATUS_FAIL;
    std::string arpingCmd = GetCmdLine("/usr/bin/arping -c 2 -D ");
	
	eConfigInterfaceType ifType = GetMngrIfType(ipType);

	std::string stNicName;
	eConfigInterfaceType signalingIfType = eSignalingNetwork;
	std::string stSignalingNicName;

	eProductFamily curProductFamily = CProcessBase::GetProcess()->GetProductFamily();
	//For RMX4000 only, we need to disable eth2 in order to make sure arping is not answered via eth2.
	if ((eProductTypeRMX2000 == curProductType) || (eProductTypeCallGenerator == curProductType) ||
	    (eProductFamilySoftMcu == curProductFamily /* all SoftMcu products */) )
	{
		stNicName = GetLogicalInterfaceName(ifType, eIpType_IpV4);
		stSignalingNicName = GetLogicalInterfaceName(signalingIfType,eIpType_IpV4 );
		ifconfig_up(stNicName);
	}
	else  //RMX4000 or RMX1500
	{
		 stNicName = GetLogicalInterfaceName(ifType, ipType );
		 stSignalingNicName = GetLogicalInterfaceName(signalingIfType, ipType );
	}

	if(!isFailover)
	{
		arpingCmd += "-I " + stNicName + " ";
	}
	else
	{
		if(eProductTypeRMX2000 != curProductType)
		{
			arpingCmd += "-I " + stNicName + " ";
		}
		arpingCmd += "-s " + sourceIP + " ";
	}

    arpingCmd += ip;
    stat = SystemPipedCommand(arpingCmd.c_str() ,answer); //disable IP Management Configuration

    TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
        "CConfiguratorManager::ArpingRequest :" << arpingCmd
        << std::endl << "stat: " << stat
        << std::endl << "answer: " << std::endl << answer;


    TRACESTR(eLevelInfoNormal) << "\nCConfiguratorManager::ArpingRequest -  arpingCmd = " << arpingCmd << ",stat:"<< stat;
    if (STATUS_OK != stat && !isFailover)
    	stat = STATUS_DUPLICATE_ADDRESS;

    //For RMX4000 and RMX1500, enable eth2 back
	if ((eProductTypeRMX4000 == curProductType) || (eProductTypeRMX1500 == curProductType))
	{
	    // VNGR-20014: V4.7.1 - Hot Backup - when enabling Hot Backup dial in
	    // no more available

	    //ifconfig_up(stSignalingNicName);
	    ;
	}

	if(isFailover)
	{
		if(answer.find("Received 0 reply")!= string::npos)
		{
			stat = STATUS_FAIL;
		}
		else
		{
			stat = STATUS_OK;
		}
	}

  ResponedClientRequest(stat);
}

void CConfiguratorManager::DadRequest(CSegment* pSeg)
{
  TEST_ROOT_AND_RETURN

  std::string ip;
  *pSeg >> ip;

  std::string ans;
  std::ostringstream cmd;
  cmd << "/bin/ping6 -c 2 -w 4 " << ip << " 2>&1";

  STATUS stat = SystemPipedCommand(cmd.str().c_str(), ans);
  if (STATUS_OK == stat)
  {
    // Filters other possible errors
    if (std::string::npos != ans.find("packet loss"))
    {
      // Verifies the result
      if (std::string::npos == ans.find("100% packet loss"))
        stat = STATUS_DUPLICATE_ADDRESS;
    }
  }
  else
  {
    // In case of the command failure enable the address
    stat = STATUS_OK;
  }

  TRACEINTO << cmd.str() << std::endl << ans << ": stat: " << stat;

  ResponedClientRequest(stat);
}

void CConfiguratorManager::AddNICRoutingTableRule(CSegment* pSeg)
{
	TEST_ROOT_AND_RETURN

	//Only for RMX4000

    if (IfSkipAddNICRoutingTableRule())
    {
		return;
    }

	std::string answer;
	std::string ip = "";
	std::string rule_cmd = "";
	std::string route_cmd = "";
	std::string route_table = "";
	std::string cache_table_cmd = "";
	eConfigInterfaceType ifType;
	eIpType ipType;
	eConfigInterfaceNum  ifNum;
	DWORD tmpIfType;
	DWORD tmpIpType;

	*pSeg  >>  tmpIfType
	       >>  tmpIpType
		   >>  ip;

	ifType = (eConfigInterfaceType) tmpIfType;
	ipType = (eIpType) tmpIpType;

	ifNum = GetInterfaceNum(ifType,ipType);
	string stNicName = "";
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	if ((eProductTypeGesher == curProductType) || (eProductTypeNinja == curProductType))
	{
		stNicName = GetConfigInterfaceNumName(ifNum);
	}
	else
	{		
		stNicName = GetDeviceName(ifType);
	}

	route_table = GetRouteTableName(ifNum);

	rule_cmd = GetCmdLinePrefix() + "ip rule add from " + ip + " lookup " + route_table;
	route_cmd = GetCmdLinePrefix() + "ip route add table " + route_table + " default src " + ip + " dev " + stNicName;

	STATUS stat = SystemPipedCommand(rule_cmd.c_str() ,answer);
	TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "CConfiguratorManager::AddNICRoutingTableRule :" << rule_cmd << std::endl << answer;

	stat = SystemPipedCommand(route_cmd.c_str() ,answer);
	TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "CConfiguratorManager::AddNICRoutingTableRule :" << route_cmd << std::endl << answer;

	cache_table_cmd = GetCmdLinePrefix() + "ip route flush cache";
	stat = SystemPipedCommand(cache_table_cmd.c_str() ,answer);
	TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "CConfiguratorManager::AddNICRoutingTableRule :" << cache_table_cmd << std::endl << answer;

   	ResponedClientRequest(stat);
}


STATUS CConfiguratorManager::ArpingTerminalRequest(CTerminalCommand & command, std::ostream& answer)
{
	if (TestRoot() == FALSE)
	{
		answer << "error: Not root\n";
		return STATUS_FAIL;
	}

	DWORD numOfParams = command.GetNumOfParams();
	if(1 != numOfParams)
	{
		answer << "error: Illegal number of parameters\n";
		return STATUS_FAIL;
	}

	const string &ip = command.GetToken(eCmdParam1);
	STATUS stat = STATUS_FAIL;


	std::string res;
    std::string arpingCmd = GetCmdLinePrefix() + "/usr/bin/arping -c 2 -D " + ip;
    stat = SystemPipedCommand(arpingCmd.c_str() ,res); //disable IP Management Configuration

    TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
        "CConfiguratorManager::ArpingTerminalRequest :" << arpingCmd
        << std::endl << "stat: " << stat
        << std::endl << "answer: " << std::endl << res;

	answer <<  stat;
	return stat;
}

STATUS CConfiguratorManager::ArpingFTerminalRequest(CTerminalCommand & command, std::ostream& answer)
{
	if (TestRoot() == FALSE)
	{
		answer << "error: Not root\n";
		return STATUS_FAIL;
	}

	DWORD numOfParams = command.GetNumOfParams();
	if(1 != numOfParams)
	{
		answer << "error: Illegal number of parameters\n";
		return STATUS_FAIL;
	}

	const string &ip = command.GetToken(eCmdParam1);
	STATUS stat = STATUS_FAIL;

	std::string res;
	// -f = stop of first response. -w 1 = wait for 1 second, if no response, abort.
    std::string arpingCmd = GetCmdLinePrefix() + "/usr/bin/arping -f -w 1 " + ip;
    stat = SystemPipedCommand(arpingCmd.c_str() ,res); //disable IP Management Configuration

	TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
		"CConfiguratorManager::ArpingFTerminalRequest :" << arpingCmd
		<< std::endl << "stat: " << stat
		<< std::endl << "answer: " << std::endl << res;

	answer <<  stat;
	return stat;
}

void CConfiguratorManager::FailoverArpingRequest(CSegment* pSeg)
{
	ArpingRequest(pSeg,true);
}

void CConfiguratorManager::AddIpInterface(CSegment* pSeg)
{
	eProductType curProductType = m_productTypeDecider->GetProductType();
	if ((eProductTypeGesher == curProductType) || (eProductTypeNinja == curProductType)	)
    {
    	AddIpInterfaceGesherNinja(pSeg);
    }
    else
    {
    	AddIpInterfaceRMX(pSeg);
    }

}
void CConfiguratorManager::AddIpInterfaceRMX(CSegment* pSeg)
{
	TEST_ROOT_AND_RETURN
    std::string ipToCompare,ip,mask,gateway, broadcast;
    std::string answer;
    std::string ifconfig_cmd;
    std::ostringstream cmd;
    std::string route_add_cmd;
    eConfigInterfaceType ifType;
    eIpType ipType;
    DWORD tmpIfType;
    DWORD tmpIpType;
    DWORD vLanId;
	*pSeg  >>  tmpIfType
	       >>  tmpIpType
		   >>  ipToCompare
		   >>  ip
	       >>  mask
		   >>  gateway
		   >>  broadcast;

    DWORD num_of_routers;
    *pSeg >> num_of_routers;

	ifType 					= (eConfigInterfaceType) tmpIfType;
	ipType 					= (eIpType) tmpIpType;

	std::string stNicName;

	stNicName	= GetLogicalInterfaceName(ifType,ipType);


	TRACESTR(eLevelInfoNormal) << "CConfiguratorManager::AddIpInterfaceRMX() ifType:" << ifType <<" ipType:" << ipType <<" stNicName:"<<stNicName;

    STATUS stat = STATUS_FAIL;

    BOOL isManagmentSep = IsSeparatedNetworks();
    if(isManagmentSep)
   	{
    	//Remove permanent network interface
   		std::string stPermanentNetworkName = GetLogicalInterfaceName(ePermanentNetwork,eIpType_IpV4);
   		ifconfig_down(stPermanentNetworkName);
   	}


	ifconfig_up(stNicName);

	//Add default router
	std::string routeCmd = GetCmdLinePrefix() + "/sbin/route add default gw " + gateway + " " + stNicName;
	SystemPipedCommand(routeCmd.c_str() ,answer);
	m_PreviousDefaultGW=gateway;
	m_StNicName = stNicName;


	// Arping
	if(ip != ipToCompare) // check arping only if needed (VNGR-12185)
	{
	    std::string arpingCmd = GetCmdLinePrefix() + "/usr/bin/arping -c 2 -D ";
		arpingCmd += "-I " + stNicName + " ";
	   	arpingCmd += ip;
	    stat = SystemPipedCommand(arpingCmd.c_str() ,answer); //disable IP Management Configuration

    TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
        "CConfiguratorManager::AddIpInterfaceRMX :" << arpingCmd << std::endl << answer;

    if (stat)
    {
        ResponedClientRequest(STATUS_DUPLICATE_ADDRESS);
        return;
    }
	}

	ifconfig_cmd = GetCmdLinePrefix() + "/sbin/ifconfig " + stNicName + " down";
    stat = SystemPipedCommand(ifconfig_cmd.c_str() ,answer);
 	TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
        "CConfiguratorManager::AddIpInterfaceRMX(Remove Inter) :" << answer;


    //Config new Management IP configuration
   	ifconfig_cmd = GetCmdLinePrefix() + "/sbin/ifconfig " + stNicName + " " + ip + " netmask " + mask +  " broadcast " + broadcast;
    stat = SystemPipedCommand(ifconfig_cmd.c_str() ,answer);
    TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
        "CConfiguratorManager::AddIpInterfaceRMX(Add Inter) :" << answer;

    if(STATUS_OK != stat)
    {
        ResponedClientRequest(STATUS_FAILED_CFG_IP);
        return;
    }

    if (STATUS_OK == stat)
    {
        if (gateway != "0.0.0.0" &&
            gateway != "255.255.255.255")
        {
            //Config default gw for the route command
            route_add_cmd = /*"route delete default ;*/ GetCmdLinePrefix() + "/sbin/route add default gw " + gateway + " " + stNicName;
            stat = SystemPipedCommand(route_add_cmd.c_str() ,answer);
            TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
            "CConfiguratorManager::AddIpInterfaceRMX :" << answer;


            m_PreviousDefaultGW=gateway;
            m_StNicName = stNicName;

        }
    }


    if (stat == STATUS_OK)
    {
    	TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "CConfiguratorManager::AddIpInterfaceRMX :num_of_routers=" << num_of_routers;

        for (DWORD i=0; i< num_of_routers; i++)
        {
            CRouter router;
            DWORD temp;
            *pSeg >> temp
                  >> router.m_targetIP
                  >> router.m_subNetmask
                  >> router.m_gateway;
            router.m_type = (eRouteType) temp;

            route_add_cmd = GetCmdLinePrefix() + "/sbin/route add ";            
            const char* type = "";
            switch (router.m_type)
            {
                case router_net:
                {
                    type = "-net ";
                    break;
                }
                case router_host:
                {
                    type = "-host ";
                    break;
                }
                default:
                {
                    PASSERT(router.m_type);
                    break;
                }
            }

            if (router.m_targetIP != "255.255.255.255")
            {
                route_add_cmd += type + router.m_targetIP + " netmask "
                    + router.m_subNetmask + " gw " + router.m_gateway
                    + " dev " + stNicName;
                stat = SystemPipedCommand(route_add_cmd.c_str() ,answer);
                TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
                    "CConfiguratorManager::AddIpInterfaceRMX :" << answer;
            }

        }
    }

    if(STATUS_OK != stat)
    {
        ResponedClientRequest(STATUS_FAILED_CFG_ROUTERS);
        return;
    }

    ResponedClientRequest(STATUS_OK);
}

void CConfiguratorManager::AddIpInterfaceGesherNinja(CSegment* pSeg)
{
	TEST_ROOT_AND_RETURN
    std::string ipToCompare,ip,mask,gateway, broadcast;
    std::string answer;
    std::string ifconfig_cmd;
    std::ostringstream cmd;
    std::string route_add_cmd;
    eConfigInterfaceType ifType;
    DWORD tmpIfType;
    DWORD tmpIpType;
    DWORD vLanId;
	*pSeg  >>  tmpIfType
		   >>  tmpIpType
		   >>  ipToCompare
		   >>  ip
	       >>  mask
		   >>  gateway
		   >>  broadcast;

    DWORD num_of_routers;
    *pSeg >> num_of_routers;

	ifType 					= (eConfigInterfaceType) tmpIfType;
	eIpType ipType 					= (eIpType) tmpIpType;
	
	std::string stNicName = GetLogicalInterfaceName(ifType, ipType);

    STATUS stat = STATUS_FAIL;

    //printf("[debug]: Bringing up interface[%s], ifType: %d, ip %s\n", stNicName.c_str(), ifType, ip.c_str());
	//Config new Management IP configuration
    ifconfig_cmd = GetCmdLinePrefix() + "/sbin/ifconfig " + stNicName + " " + ip + " netmask " + mask +  " broadcast " + broadcast ;

    stat = SystemPipedCommand(ifconfig_cmd.c_str() ,answer);
    TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
        "CConfiguratorManager::AddIpInterface800S(Add Inter) :" << answer;
    //printf("[debug]: run command: %s, result=%s\n", ifconfig_cmd.c_str(), answer.c_str());
	ifconfig_up(stNicName);

    if(STATUS_OK != stat)
    {
        ResponedClientRequest(STATUS_FAILED_CFG_IP);
        return;
    }

	//Add default router
	std::string routeCmd;
	routeCmd = GetCmdLinePrefix() +  "/sbin/route add default gw " + gateway + " " + stNicName;
	stat = SystemPipedCommand(routeCmd.c_str() ,answer);
	m_PreviousDefaultGW=gateway;
	//printf("[debug]: run command: %s, result=%s\n", routeCmd.c_str(), answer.c_str());

	//m_PreviousDefaultGW=gateway;
	m_StNicName = stNicName;

	// Arping
	if(ip != ipToCompare) // check arping only if needed (VNGR-12185)
	{
	    std::string arpingCmd;
   	    arpingCmd = GetCmdLinePrefix() +  "/usr/bin/arping -c 2 -D ";

		arpingCmd += "-I " + stNicName + " ";
	   	arpingCmd += ip;
	   	arpingCmd += " &";
	    stat = SystemPipedCommand(arpingCmd.c_str() ,answer); //disable IP Management Configuration

		TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
			"CConfiguratorManager::AddIpInterface800S :" << arpingCmd << std::endl << answer;

		//printf("[debug]: run command: %s, result=%s\n", arpingCmd.c_str(), answer.c_str());
		if (stat)
		{
			//printf("[debug]:STATUS_DUPLICATE_ADDRESS\n");
			ResponedClientRequest(STATUS_DUPLICATE_ADDRESS);
			return;
		}
	}

    if (stat == STATUS_OK)
    {
    	TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "CConfiguratorManager::AddIpInterface800S :num_of_routers=" << num_of_routers;

        for (DWORD i=0; i< num_of_routers; i++)
        {
            CRouter router;
            DWORD temp;
            *pSeg >> temp
                  >> router.m_targetIP
                  >> router.m_subNetmask
                  >> router.m_gateway;
            router.m_type = (eRouteType) temp;

            route_add_cmd = GetCmdLinePrefix() +  "/sbin/route add ";

            const char* type = "";
            switch (router.m_type)
            {
                case router_net:
                {
                    type = "-net ";
                    break;
                }
                case router_host:
                {
                    type = "-host ";
                    break;
                }
                default:
                {
                    PASSERT(router.m_type);
                    break;
                }
            }

            if (router.m_targetIP != "255.255.255.255")
            {
                route_add_cmd += type + router.m_targetIP + " netmask "
                    + router.m_subNetmask + " gw " + router.m_gateway
                    + " dev " + stNicName;
                stat = SystemPipedCommand(route_add_cmd.c_str() ,answer);
                TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
                    "CConfiguratorManager::AddIpInterface800S :" << answer;
        		//printf("[debug]: run command: %s, result=%s\n", route_add_cmd.c_str(), answer.c_str());
            }

        }
    }

    //change MTU size
    DWORD rtmLanMtuSize;	
    BOOL res = CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey(CFG_KEY_MEDIA_NIC_MTU_SIZE, rtmLanMtuSize);
	TRACESTR(eLevelInfoNormal) << "CConfiguratorManager::AddIpInterface800S :" <<
            "key - " << CFG_KEY_MEDIA_NIC_MTU_SIZE<< ", value: " << rtmLanMtuSize;

    if ( (TRUE == res) && ((rtmLanMtuSize >= 500) && (rtmLanMtuSize <= 20000)) )
	{
        if (eSignalingNetwork == ifType || eLanRedundancySignalingNetwork == ifType 
            || eMultipleSignalingNetwork_1 == ifType || eMultipleSignalingNetwork_2 == ifType)
        {
            stringstream streamMtu;
            string strMtuSize;
            streamMtu << rtmLanMtuSize;
    	    streamMtu >> strMtuSize;
            TRACESTR(eLevelInfoNormal) << "CConfiguratorManager::AddIpInterface800S ifType: " << ifType << " strMtuSize:" << strMtuSize; 

            string mtuCmd;
        	mtuCmd = GetCmdLinePrefix() + "/sbin/ifconfig " + stNicName + " mtu " + strMtuSize;
        	stat = SystemPipedCommand(mtuCmd.c_str() ,answer);
        }
	}
        
    if(STATUS_OK != stat)
    {
        ResponedClientRequest(STATUS_FAILED_CFG_ROUTERS);
        return;
    }
    ResponedClientRequest(STATUS_OK);
}

void CConfiguratorManager::AddIpV6Interface(CSegment* pSeg)
{
	eProductType curProductType = m_productTypeDecider->GetProductType();
	if ((eProductTypeGesher == curProductType) || (eProductTypeNinja == curProductType)	)
    {
    	AddIpV6InterfaceGesherNinja(pSeg);
    }
    else
    {
    	AddIpV6InterfaceRMX(pSeg);
    }
}

void CConfiguratorManager::AddIpV6InterfaceGesherNinja(CSegment* pSeg)
{
	TEST_ROOT_AND_RETURN
    std::string ip,ipmask,gateway,gwmask;
    std::string answer;
    std::string ifconfig_cmd = "";

    std::ostringstream cmd;
    std::string route_add_cmd = "";

    DWORD tmp;
    DWORD vLanId;
    BYTE	bIsAutoConfig;
	
    *pSeg  >>  tmp
        >>  bIsAutoConfig
        >>  ip
        >>  ipmask
        >>  gateway
        >>  gwmask;

	TRACESTR(eLevelInfoNormal) << "\nCConfiguratorManager::AddIpV6Interface : Begin... \n bIsAutoConfig: " << bIsAutoConfig << "\nip = " << ip << "\nipmask = " << ipmask;
	TRACESTR(eLevelInfoNormal) << "\nIfType:" << tmp << " gateway: " << gateway << " gwmask = " << gwmask;
	
	eConfigInterfaceType interfaceType = (eConfigInterfaceType) tmp;
    STATUS stat = STATUS_OK;

    
    //eConfigInterfaceType interfaceType = eManagmentNetwork;

    std::string nic_name = GetLogicalInterfaceName(interfaceType, eIpType_IpV6);
	
	SetNICIpV6AutoConfig(nic_name.c_str(), bIsAutoConfig,interfaceType,eIpType_IpV6,gateway,gwmask);

    
    TRACESTR(eLevelInfoNormal) << "\nCConfiguratorManager::AddIpV6InterfaceGesherNinja : " << nic_name
    					   << "\nAutoConfig = " << (bIsAutoConfig ? "yes" : "no")
    					   << "\nAddress    = " << ip		<< "\\" << ipmask
    					   << "\nGateway    = " << gateway	<< "\\" << gwmask << "\n";

    //Only for manual configuration
    if(!bIsAutoConfig)
    {
		// ifconfig eth0 inet6 add 2001:da8:2004:1000:202:116:160:41/64
		ifconfig_cmd = GetCmdLinePrefix() + "/sbin/ifconfig " + nic_name + " add " + ip + "/" + ipmask;
		stat = SystemPipedCommand(ifconfig_cmd.c_str(), answer);

		//route -A inet6 add default gw fe80::b2c6:9aff:fed2:1f81 dev eth1
		route_add_cmd = GetCmdLinePrefix() + "/sbin/route -A inet6 add default gw " + gateway + " dev " + nic_name;
		stat = SystemPipedCommand(route_add_cmd.c_str(), answer);
    }

    if(STATUS_OK != stat)
    {
    	TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
        	"\nCConfiguratorManager::AddIpV6Interface : status failed "<< stat;

    	if (STATUS_FAILED_CFG_IPV6_DEF_GW != stat) // otherwise it's needed to respond with 'STATUS_FAILED_CFG_IPV6_DEF_GW' explicitly
    	{
    		stat = STATUS_FAIL;
    	}
    }

    ResponedClientRequest(stat);
}


void CConfiguratorManager::AddIpV6InterfaceRMX(CSegment* pSeg)
{
	TEST_ROOT_AND_RETURN
    std::string ip,ipmask,gateway,gwmask;
    std::string answer;
    std::string ifconfig_cmd;
    std::ostringstream cmd;
    std::string route_add_cmd;
    eConfigInterfaceType ifType;
    DWORD tmp;
    
    DWORD vLanId;
    BYTE	bIsAutoConfig;
    DWORD tmpIpCfgType;
	
	*pSeg  >>  tmp
		   >>  bIsAutoConfig
		   >>  ip
		   >>  ipmask
	       >>  gateway
	       >>  gwmask;

	ifType = (eConfigInterfaceType) tmp;
    STATUS stat = STATUS_OK;

    BOOL isManagmentSep = IsSeparatedNetworks();

    eConfigInterfaceType interfaceType = eManagmentNetwork;
	if(isManagmentSep)
	{
		interfaceType = eSeparatedManagmentNetwork;
		//For Ipv6, we don't support management default gw.
		gateway = "::0";

		std::string nic_name = GetLogicalInterfaceName(interfaceType, eIpType_IpV6);

		TRACESTR(eLevelInfoNormal) << "\nCConfiguratorManager::AddIpV6Interface : isManagmentSep = " << isManagmentSep << "NICName = " << nic_name << endl;

		SetNICIpV6AutoConfig(nic_name.c_str(), bIsAutoConfig,interfaceType,eIpType_IpV6,gateway,gwmask);
	}
	else
	{
		std::string dev_name = GetDeviceName(interfaceType);

		
		SetNICIpV6AutoConfig(dev_name.c_str(), bIsAutoConfig,interfaceType,eIpType_IpV6,gateway,gwmask);
	}
	std::string nic_name = GetLogicalInterfaceName(interfaceType, eIpType_IpV6);
    
    TRACESTR(eLevelInfoNormal) << "\nCConfiguratorManager::AddIpV6Interface : " << nic_name
    					   << "\nAutoConfig = " << (bIsAutoConfig ? "yes" : "no")
    					   << "\nAddress    = " << ip		<< "\\" << ipmask
    					   << "\nGateway    = " << gateway	<< "\\" << gwmask << "\n";

    //Only for manual configuration
    if(!bIsAutoConfig)
    {
    	stat = AcAddNi6((char*)nic_name.c_str(),
						(char*)ip.c_str(),
						(char*)ipmask.c_str(),
						(char*)gateway.c_str(),
						(char*)gwmask.c_str());

    }

    if(STATUS_OK != stat)
    {
    	TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
        	"\nCConfiguratorManager::AddIpV6Interface : status failed "<< stat;

    	if (STATUS_FAILED_CFG_IPV6_DEF_GW != stat) // otherwise it's needed to respond with 'STATUS_FAILED_CFG_IPV6_DEF_GW' explicitly
    	{
    		stat = STATUS_FAIL;
    	}
    }

    ResponedClientRequest(stat);
}

void CConfiguratorManager::InterfaceUp(CSegment* pSeg)
{
	STATUS stat = STATUS_OK;

	std::string stNicName = "";
	*pSeg >> stNicName;

	ifconfig_up(stNicName);

	ResponedClientRequest(stat);
}

void CConfiguratorManager::RunningCommand(CSegment* pSeg)
{
	STATUS stat = STATUS_OK;

	std::string cmdline = "";
	*pSeg >> cmdline;

	std::string answer;
	SystemPipedCommand(cmdline.c_str() ,answer);

	ResponedClientRequest(stat);
}

void CConfiguratorManager::ifconfig_up(std::string stNicName)
{
	if (stNicName != "")
	{
		std::string answer;
        std::string ifconfigCmd = GetCmdLinePrefix() + "/sbin/ifconfig " + stNicName + " up";
		SystemPipedCommand(ifconfigCmd.c_str() ,answer);
	}
}

void CConfiguratorManager::ifconfig_up(std::string stNicName,std::string ip4addr,std::string mask)
{
	if (stNicName != "")
	{
		std::string answer;
		std::string ifconfigCmd = GetCmdLinePrefix() + "/sbin/ifconfig " + stNicName + " " + ip4addr + " netmask " + mask + " up";
		SystemPipedCommand(ifconfigCmd.c_str() ,answer);
	}
}

void CConfiguratorManager::ifconfig_down(std::string stNicName)
{
	if (stNicName != "")
	{
		std::string answer;
        std::string ifconfigCmd = GetCmdLinePrefix() + "/sbin/ifconfig " + stNicName + " down";
		SystemPipedCommand(ifconfigCmd.c_str() ,answer);
	}
}

// Config default gw for the route command
void CConfiguratorManager::AddDefaultGW(CSegment* pSeg)
{
    TEST_ROOT_AND_RETURN

	STATUS stat = STATUS_OK;

	std::string gateway = "";
	std::string stNicName = "";
	std::string route_add_cmd = "";
	BOOL isIpv6 = FALSE;

	*pSeg >> stNicName
		  >> gateway
		  >> isIpv6;

	if (stNicName != "")
	{
		if (gateway != "0.0.0.0" &&
			gateway != "255.255.255.255")
		{
			std::string answer;
			route_add_cmd = GetCmdLinePrefix() + "route add default gw " + gateway + " " + stNicName;
			if(isIpv6)
				route_add_cmd += " -A inet6";

			stat = SystemPipedCommand(route_add_cmd.c_str() ,answer);

			m_PreviousDefaultGW=gateway;
			m_StNicName = stNicName;

		}
	}

	ResponedClientRequest(stat);
}

void CConfiguratorManager::AddStaticIpRoutes(CSegment* pSeg)
{
	TEST_ROOT_AND_RETURN
	std::string route_add_cmd, answer, cidrGWIpAddr;
	eConfigInterfaceType ifType;
	DWORD tmpIfType;
	DWORD num_of_routers;

	*pSeg  >> tmpIfType
		   >> num_of_routers;

	ifType 					= (eConfigInterfaceType) tmpIfType;
	std::string stNicName	= GetDeviceName(ifType);

	eProductType curProductType = m_productTypeDecider->GetProductType();
	TRACESTR(eLevelInfoNormal) << "CConfiguratorManager::AddStaticIpRoutes ifType:" << ifType <<" stNicName:"<<stNicName;

	STATUS stat = STATUS_OK;

	TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "CConfiguratorManager::AddStaticIpRoutes :num_of_routers=" << num_of_routers;

	for (DWORD i=0; i< num_of_routers; i++)
	{
		CRouter router;
		DWORD temp;
		*pSeg >> temp
			  >> router.m_targetIP
			  >> router.m_subNetmask
			  >> router.m_gateway;
		router.m_type = (eRouteType) temp;

		route_add_cmd = GetCmdLinePrefix() + "ip route add ";

		if (router.m_targetIP != "255.255.255.255")
		{
			cidrGWIpAddr = ConvertIpAddressToCIDRNotation(router.m_targetIP, router.m_subNetmask);
			route_add_cmd += cidrGWIpAddr + " via " + router.m_gateway + " dev " + stNicName;

			stat = SystemPipedCommand(route_add_cmd.c_str() ,answer);
			TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
				"CConfiguratorManager::AddStaticIpRoutes :" << answer;
		}

	}

	if(STATUS_OK != stat)
	{
		ResponedClientRequest(STATUS_FAILED_CFG_ROUTERS);
		return;
	}

	ResponedClientRequest(STATUS_OK);
}


void CConfiguratorManager::AddStaticRoutes(CSegment* pSeg)
{
	TEST_ROOT_AND_RETURN
	std::string route_add_cmd, answer, cidrGWIpAddr;
	eConfigInterfaceType ifType;
	DWORD tmpIfType;
	DWORD num_of_routers;

	*pSeg  >> tmpIfType
		   >> num_of_routers;

	ifType 					= (eConfigInterfaceType) tmpIfType;
	std::string stNicName	= GetDeviceName(ifType);

	eProductType curProductType = m_productTypeDecider->GetProductType();
	TRACESTR(eLevelInfoNormal) << "CConfiguratorManager::AddStaticRoutes ifType:" << ifType <<" stNicName:"<<stNicName;

	STATUS stat = STATUS_OK;

	TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "CConfiguratorManager::AddStaticRoutes :num_of_routers=" << num_of_routers;

	for (DWORD i=0; i< num_of_routers; i++)
	{
		CRouter router;
		DWORD temp;
		*pSeg >> temp
			  >> router.m_targetIP
			  >> router.m_subNetmask
			  >> router.m_gateway;
		router.m_type = (eRouteType) temp;


		TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "\nCConfiguratorManager::AddStaticRoutes  "
		<< " \n m_targetIP" << router.m_targetIP
		<< " \n m_subNetmask" << router.m_subNetmask
		<< " \n m_gateway" << router.m_gateway ;


        route_add_cmd = GetCmdLinePrefix() + "route add ";
        const char* type = "";
        switch (router.m_type)
        {
            case router_net:
            {
                type = "-net ";
                break;
            }
            case router_host:
            {
                type = "-host ";
                break;
            }
            default:
            {
                PASSERT(router.m_type);
                break;
            }
        }

        if (router.m_targetIP != "255.255.255.255")
        {
            route_add_cmd += type + router.m_targetIP + " netmask "
                + router.m_subNetmask + " gw " + router.m_gateway
                + " dev " + stNicName;

            stat = SystemPipedCommand(route_add_cmd.c_str() ,answer);
            TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
                "CConfiguratorManager::AddIpInterface :" << answer;
        }

	}

	if(STATUS_OK != stat)
	{
		ResponedClientRequest(STATUS_FAILED_CFG_ROUTERS);
		return;
	}

	ResponedClientRequest(STATUS_OK);
}

void CConfiguratorManager::SetIpv6AutoConfig(CSegment* pSeg)
{
	STATUS stat = STATUS_OK;

	std::string stNicName = "";
	std::string defaultGatewayIPv6Str = "";
	BOOL isIpv6Autoconf = FALSE;
	DWORD tmpifType , tmpipType;

	*pSeg >> stNicName
		  >> defaultGatewayIPv6Str
		  >> isIpv6Autoconf
		  >> tmpifType
		  >> tmpipType;

	SetNICIpV6AutoConfig(stNicName, isIpv6Autoconf,(const eConfigInterfaceType)tmpifType, (const eIpType)tmpipType,defaultGatewayIPv6Str,"");

	ResponedClientRequest(stat);
}

void CConfiguratorManager::SetNICIpV6AutoConfig(const string &NIC, BYTE bIsAutoConfig,const eConfigInterfaceType ifType , const eIpType ipType,const string ipv6gateway,const string ipv6gwmask)
{
	
	eConfigInterfaceNum  ifNum;
    BOOL isNinjaGesher = FALSE;
    eProductType curProductType = m_productTypeDecider->GetProductType();
    if ( (FALSE == IsTarget()) && (eProductTypeGesher != curProductType) && (eProductTypeNinja != curProductType)) // no configuration should be done on Pizzas
	{
		return;
	}

	if (((eProductTypeGesher == curProductType) || (eProductTypeNinja == curProductType)) && ( ipType == eIpType_IpV4 ))
	{
		return;
	}
	std::string answer, Ipv4_default_gw="", Ipv6_Manual_address="",Ipv4_address="",Mask="",Ipv6LocalAddr="",Ipv6LocalAddrNew="";
	STATUS stat = STATUS_FAIL;
	std::string ifconfig_cmd="", sysctl_cmd="", route_cmd="",ifconfig_cmd_for_eth0_2 = "";
	std::string value = bIsAutoConfig ? "1" : "0";

	//Read IPv4 default gw
	route_cmd = "route | grep default | grep "+ NIC+" | awk '{ print $2 }'";
	SystemPipedCommand(route_cmd.c_str(),Ipv4_default_gw);

	//Read IPv4 default gw interface
	//route_cmd = "route | grep default | awk '{ print $8 }'";
	//SystemPipedCommand(route_cmd.c_str(), Ipv4_default_gw_nic_name);

	//Read IPv6 Global address
	if(!bIsAutoConfig)
	{
		ifconfig_cmd = GetCmdLinePrefix() + "/sbin/ifconfig " + NIC + " | grep Scope:Global | awk '{ print $3 }'";
		SystemPipedCommand(ifconfig_cmd.c_str(), Ipv6_Manual_address);
	}

	if ((eProductTypeRMX2000 == curProductType) && (NIC == "eth0:2"))
	{
		ifconfig_cmd_for_eth0_2 = GetCmdLinePrefix() + "/sbin/ifconfig | grep -A 2 eth0:2 | grep  inet | awk -F ':' '{ print $2 }' | awk -F ' ' '{ print $1 }'";
		SystemPipedCommand(ifconfig_cmd_for_eth0_2.c_str(), Ipv4_address);

		ifconfig_cmd_for_eth0_2 = GetCmdLinePrefix() + "/sbin/ifconfig | grep -A 2 eth0:2 | grep  Mask | awk -F ':' '{ print $4 }'";
		SystemPipedCommand(ifconfig_cmd_for_eth0_2.c_str(), Mask);

	}

	//Deactivate eth0
	ifconfig_down(NIC);
	//Set Ipv6 Autoconfig flags

	
    if ((eProductTypeGesher == curProductType) || (eProductTypeNinja == curProductType))
    {
    	//'sudo sh -c "echo 0 > /proc/sys/net/ipv6/conf/eth0/autoconf"'
	    sysctl_cmd = "echo " + value + " | sudo tee /proc/sys/net/ipv6/conf/" + NIC + "/autoconf";
    }
    else
    {
	    sysctl_cmd = "echo " + value + " > /proc/sys/net/ipv6/conf/" + NIC + "/autoconf";
    }
	stat = SystemPipedCommand(sysctl_cmd.c_str() ,answer);

	if (Ipv4_address == "") //Activate eth0
	     ifconfig_up(NIC);
	else //vngr-26661
		 ifconfig_up(NIC,Ipv4_address,Mask);

	if ( Ipv4_default_gw !="")
	{
		Ipv4_default_gw.resize(Ipv4_default_gw.length()-1);//the address is with end of line in the end of it.
		//Add original Ipv4 default gw
		route_cmd =  GetCmdLinePrefix() +"route add default gw " + Ipv4_default_gw + " " + NIC;
		stat = SystemPipedCommand(route_cmd.c_str(), answer);


		m_PreviousDefaultGW=Ipv4_default_gw;
		m_StNicName = NIC;
	}


	//Add original IPv6 Global address
	//if ((eProductTypeGesher != curProductType) && (eProductTypeNinja != curProductType))
    {   
		if(!bIsAutoConfig)
		{
			if (NIC != "eth2" && Ipv6_Manual_address != "")
			{
				ifconfig_cmd = GetCmdLinePrefix() + "/sbin/ifconfig " + NIC + " add " + Ipv6_Manual_address;
				stat = SystemPipedCommand(ifconfig_cmd.c_str(), answer);
			}
			//for manual ipv6 set the default gw manually
			if (false==IsJitcAndNetSeparation())  // don't add default gw in netwrok seperation see above "::0"
			{
				route_cmd = GetCmdLinePrefix() + "route -A inet6 add default gw " + ipv6gateway + " " +   NIC;
				SystemPipedCommand(route_cmd.c_str(), answer);
    		}
		}
	}


	//Net Sep Jitc
	if ( ipType!=eIpType_IpV4 &&  true==IsJitcAndNetSeparation()  )
	{
		ifNum = GetInterfaceNum(ifType,ipType);

		CSegment* pSeg = new CSegment();
		std::string route_table = GetIPv6RouteTableName(ifNum);
		*pSeg << route_table << NIC;
		if ("eth0.2197"==NIC)
			StartTimer(CONFIG_NETWORK_SEPERATION_IPV6_VLAN_I, SECOND * 30, pSeg);
		else if ( "eth0.2198"==NIC)
			StartTimer(CONFIG_NETWORK_SEPERATION_IPV6_VLAN_II, SECOND * 30, pSeg);
	}
}

void CConfiguratorManager::AddVlan(CSegment* pSeg)
{
	TEST_ROOT_AND_RETURN
    std::string ip,mask,gateway,broadcast;
	eConfigInterfaceType ifType;
    DWORD tmp;
    DWORD vLanId;
	*pSeg  >>  tmp
		   >>  ip
	       >>  mask
		   >>  broadcast
		   >>  vLanId;

	ifType = (eConfigInterfaceType) tmp;
    std::string Interface;
    std::string answer;
    STATUS stat = STATUS_FAIL;
    COstrStream vconfig;
    COstrStream ifconfig_cmd, AddVlanCmd;

    ifconfig_cmd<< GetCmdLinePrefix() << "/sbin/ifconfig eth0 up";
    stat = SystemPipedCommand(ifconfig_cmd.str().c_str() ,answer); //Activate eth0
    TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
        "CConfiguratorManager::AddVlan :" << vconfig.str() << std::endl << answer;

    vconfig << GetCmdLinePrefix() << "/sbin/vconfig add eth0" << " " << vLanId;
    stat = SystemPipedCommand(vconfig.str().c_str() ,answer); //add new Vlan
    TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
        "CConfiguratorManager::AddVlan :" << vconfig.str() << std::endl << answer;

    //Configure new Vlan

    AddVlanCmd<< GetCmdLinePrefix() << "/sbin/ifconfig eth0."<< vLanId << " " << ip <<" netmask "<< mask << " up" ;
    stat = SystemPipedCommand(AddVlanCmd.str().c_str() ,answer);
    TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
        "CConfiguratorManager::AddVlan :" << AddVlanCmd << std::endl << answer;

    if(STATUS_OK != stat)
    {
        stat = STATUS_FAIL;
    }

    ResponedClientRequest(stat);
}

   void CConfiguratorManager::GetNewVersionNumber(CSegment* pSeg)
{
	TEST_ROOT_AND_RETURN;

    std::string version;
    STATUS stat =
        SystemPipedCommand(("Scripts/TestFirmware.sh "+MCU_DATA_DIR+"/new_version/new_version.bin").c_str(),
                           version);

    TRACESTR(stat ? eLevelError:eLevelInfoNormal)
        << "CConfiguratorManager::GetNewVersionNumber :" << version;

    CSegment*  seg = new CSegment;
    *seg << version;

    if (version.find("error") != string::npos)
    {
        PASSERTMSG(1,version.c_str());
        stat = STATUS_FAIL;
    }

    ResponedClientRequest(stat,seg);
}

/////////////////////////////////////////////////////////////////////////////
void CConfiguratorManager::RemoveIpInterface(CSegment* pSeg)
{
	TEST_ROOT_AND_RETURN

    std::string ip;

	*pSeg  >>  ip;

    // todo:...
//    std::string ifconfig_cmd;
//    ifconfig_cmd = "ifconfig etho add " + ip + " netmask " + mask + " broadcast " + gateway ;
//    SystemRunCommand(ifconfig_cmd.c_str());

    ResponedClientRequest(STATUS_OK);
}

void CConfiguratorManager::CycleVersions(CSegment* pSeg)
{
	TEST_ROOT_AND_RETURN
	std::string new_version_name;
	*pSeg >> new_version_name;

    std::string cmd = GetCycleVersionsScriptCmdLine(new_version_name);
    std::string answer;

    STATUS stat = SystemPipedCommand(cmd.c_str(),answer);
    TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
        "CConfiguratorManager::CycleVersions :" << cmd << std::endl << answer;

    eProductType curProductType = m_productTypeDecider->GetProductType();
    if(eProductTypeGesher == curProductType || eProductTypeNinja == curProductType)
    {
	    if (STATUS_OK == stat && answer.find( "already installed", 0 ) != string::npos)
	    {
	        stat = STATUS_VERSION_ALREADY_INSTALLED;
	    }
	    else if(STATUS_OK != stat)
	    {
	        stat = STATUS_FAIL;
	    }
    }
    else
    {
	    if (STATUS_OK == stat && answer.find( "already installed", 0 ) != string::npos)
	    {
	        stat = STATUS_VERSION_ALREADY_INSTALLED;
	    }
	    else if(STATUS_OK != stat)
	    {
	        stat = STATUS_FAIL;
	    }
    }

    ResponedClientRequest(stat);
}

void CConfiguratorManager::NameServerUpdate(CSegment* pSeg)
{
    eProductType curProductType = m_productTypeDecider->GetProductType();

    if(eProductTypeGesher != curProductType && eProductTypeNinja != curProductType)
    {
        if ( FALSE == IsTarget() ) // no configuration should be done on Pizzas
        {
            ResponedClientRequest(STATUS_OK);
            return;
        }
        TEST_ROOT_AND_RETURN
    }

    std::string dns, host, zone, ip, ipv6;

    *pSeg  >>  dns >> host >> zone >> ip >> ipv6;

    //IPv4
    ofstream nsupdate_file((MCU_TMP_DIR+"/nsupdate.txt").c_str());
    if (nsupdate_file.is_open() == FALSE)
    {
        PASSERT(1);
        ResponedClientRequest(STATUS_FAIL);
        return;
    }

    nsupdate_file << "server " << dns << std::endl;
    nsupdate_file << "zone " << zone << std::endl;

    if("" != ip)
    {
        nsupdate_file << "update add " << host << "." << zone << " 86400 A " << ip << std::endl;
    }
    if("" != ipv6)
    {
        nsupdate_file << "update add " << host << "." << zone << " 86400 AAAA " << ipv6 << std::endl;
    }

    nsupdate_file << "show" << std::endl;
    nsupdate_file << "send" << std::endl;


    nsupdate_file.close();

    std::string answer;
    STATUS stat;
    std::string command;

    command = GetCmdLinePrefix() + "nsupdate < "+MCU_TMP_DIR+"/nsupdate.txt";

    stat = SystemPipedCommand(command.c_str(), answer);

    TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
        "CConfiguratorManager::NameServerUpdate IPv4/6:"  << answer;
    ResponedClientRequest((stat == STATUS_OK)? STATUS_OK:STATUS_FAIL);
}

/////////////////////////////////////////////////////////////////////////////
void CConfiguratorManager::ConfigureDnsServers(CSegment* pSeg)
{
    TRACESTR(eLevelInfoNormal)<< "CConfiguratorManager::ConfigureDnsServers";

    eProductType curProductType = m_productTypeDecider->GetProductType();

    if(eProductTypeGesher != curProductType && eProductTypeNinja != curProductType && eProductTypeEdgeAxis != curProductType)
    {
        if ( FALSE == IsTarget() ) // no configuration should be done on Pizzas
        {
            ResponedClientRequest(STATUS_OK);
            return;
        }

        TEST_ROOT_AND_RETURN
    }

    std::string answer;

    if(eProductTypeGesher == curProductType || eProductTypeNinja == curProductType)
    {
        SystemPipedCommand(("sudo /bin/chmod 666 "+MCU_CONFIG_DIR+"/common/etc/resolv.conf").c_str(), answer);
    }
    else if (eProductTypeEdgeAxis == curProductType)
    {
        SystemPipedCommand("/usr/bin/sudo /bin/chmod -R 777 /etc/resolv.conf", answer);
    }

    std::string server,dns1,dns2,dns3;

    *pSeg  >>  server >> dns1 >> dns2 >> dns3;

    TRACESTR(eLevelInfoNormal)<< "CConfiguratorManager::ConfigureDnsServers" << " search: " << server
    		                                   << " nameserver: " << dns1 <<" nameserver: " << dns2 << " nameserver: " << dns3;

    ofstream resolv_conf("/etc/resolv.conf");
    if (resolv_conf.is_open() == FALSE)
    {
        TRACESTR(eLevelError) << "failed to open /etc/resolv.conf";
        ResponedClientRequest(STATUS_FAIL);
        return;
    }

    resolv_conf << "search " << server << std::endl;
    resolv_conf << "nameserver " << dns1 << std::endl;
    resolv_conf << "nameserver " << dns2 << std::endl;
    resolv_conf << "nameserver " << dns3 << std::endl;
    resolv_conf << "options timeout:3" << std::endl;
    resolv_conf << "options attempts:1" << std::endl;
    resolv_conf.close();

    if(eProductTypeGesher == curProductType || eProductTypeNinja == curProductType)
    {
        SystemPipedCommand(("sudo /bin/chmod 644 "+MCU_CONFIG_DIR+"/common/etc/resolv.conf").c_str(), answer);
    }
    else if (eProductTypeEdgeAxis == curProductType)
    {
        SystemPipedCommand("/usr/bin/sudo /bin/chmod -R 555 /etc/resolv.conf", answer);
    }

    ResponedClientRequest(STATUS_OK);
}

/////////////////////////////////////////////////////////////////////////////
void CConfiguratorManager::ConfigureMoreDnsServers(CSegment* pSeg)
{
    eProductType curProductType = m_productTypeDecider->GetProductType();

    if(eProductTypeGesher != curProductType && eProductTypeNinja != curProductType)
    {
        if ( FALSE == IsTarget() ) // no configuration should be done on Pizzas
        {
            ResponedClientRequest(STATUS_OK);
            return;
        }

        TEST_ROOT_AND_RETURN
    }

    std::string server,dns1;
    std::string answer;

    *pSeg  >>  server >> dns1;

    std::string dns1Line = "echo nameserver " + dns1 + " >> /etc/resolv.conf";

    if(eProductTypeGesher == curProductType || eProductTypeNinja == curProductType)
    {
        SystemPipedCommand(("sudo /bin/chmod 666 "+MCU_CONFIG_DIR+"/common/etc/resolv.conf").c_str(), answer);
    }
    
    std::string answer1, answer2;
    SystemPipedCommand(dns1Line.c_str(), answer1);

    if(eProductTypeGesher == curProductType || eProductTypeNinja == curProductType)
    {
        SystemPipedCommand(("sudo /bin/chmod 644 "+MCU_CONFIG_DIR+"/common/etc/resolv.conf").c_str(), answer);
    }
    
    std::string domainLine = GetCmdLinePrefix() + "Scripts/AddDomainToResolvConf.sh " + server;
    SystemPipedCommand(domainLine.c_str() , answer2);


    TRACESTR(eLevelInfoNormal) << "CConfiguratorManager::ConfigureMoreDnsServers"
                               << "\nserver: " << server
                               << "\n dns: " << dns1
                               << "\n answer1: " << answer1
                               << "\n answer2: " << answer2;

    ResponedClientRequest(STATUS_OK);
}

/////////////////////////////////////////////////////////////////////////////
void CConfiguratorManager::RunDHCP(CSegment* pSeg)
{
 	TEST_ROOT_AND_RETURN
    std::string answer;
    BOOL autoDNS;
    *pSeg >> autoDNS;
    STATUS stat;

    if (autoDNS)
        stat = SystemPipedCommand(("dhcpcd -N -L -Y "+MCU_TMP_DIR+" eth0:1 -I eth0:1").c_str(),answer);
    else
        stat = SystemPipedCommand(("dhcpcd -R -N -L -Y "+MCU_TMP_DIR+" eth0:1 -I eth0:1").c_str(),answer);

    if (stat != STATUS_OK)
    {
        PASSERTMSG(stat,answer.c_str());
        stat = STATUS_FAIL;
    }
    ResponedClientRequest(stat);
}

/////////////////////////////////////////////////////////////////////////////
void CConfiguratorManager::KillDHCP(CSegment* pSeg)
{
    TEST_ROOT_AND_RETURN
    std::string cmd;
    std::string answer;

    DWORD tmpIpType;
    eIpType ipType ;

    *pSeg >> tmpIpType;
    ipType 					= (eIpType) tmpIpType;
    string sMngmntNI;

    sMngmntNI = GetLogicalInterfaceName(eManagmentNetwork, ipType);


    cmd = "kill -q  `ps | grep dhcpcd | grep \"" + sMngmntNI + "\" | sed s/root.*//` ; echo killing DHCPDC";
    STATUS stat = SystemPipedCommand(cmd.c_str(),answer);

    if (stat != STATUS_OK)
    {
        PASSERTMSG(stat,answer.c_str());
        stat = STATUS_FAIL;
    }
    ResponedClientRequest(stat);
}

/////////////////////////////////////////////////////////////////////////////
void CConfiguratorManager::GetLastDHCPConfiguration(CSegment* pSeg)
{
    TEST_ROOT_AND_RETURN

    std::string key;
    std::string answer;
    STATUS stat;

    *pSeg >> key;


    std::string cmd = "cat "+MCU_TMP_DIR+"/dhcpcd-eth0\\:1.info | grep ";
    cmd += key;
    cmd += " | sed s/^";
    cmd += key;
    cmd += "=//";

    stat = SystemPipedCommand(cmd.c_str(),answer);
    CSegment*  seg = new CSegment;
    *seg << answer;

    if (stat == STATUS_OK)
    {
        if (answer.size() > 0 )
        {
            stat = STATUS_OK;
        }
        else
        {
            stat = STATUS_FAIL;
        }
    }
    else
    {
        stat = STATUS_FAIL;
    }

    ResponedClientRequest(stat,seg);
}

void CConfiguratorManager::RemountVersionPartition(CSegment* pSeg)
{
	TEST_ROOT_AND_RETURN

    BOOL withWritePermission	    = FALSE;
    BOOL isToDeleteFiles	    = FALSE;
    BOOL isToDeleteFallbackInNeeded = FALSE;

	*pSeg  >> withWritePermission
	       >> isToDeleteFiles
	       >> isToDeleteFallbackInNeeded;

    std::string command;

    {
	    if (withWritePermission)
	    {
	        command = GetCmdLinePrefix() + "mount "+MCU_DATA_DIR+" -o remount -o rw -o async -o noexec";
	    }
	    else
	    {
	        command =  GetCmdLinePrefix() + "mount "+MCU_DATA_DIR+" -o remount -o ro -o noexec";
	    }
    }


    std::string answer;
    STATUS stat = SystemPipedCommand(command.c_str(), answer);

    TRACESTRFUNC(stat ? eLevelError:eLevelInfoNormal) <<
        "Mount: " << command << std::endl << answer.c_str();

    // In case of write permission we also delete all files if exist
    if (withWritePermission && isToDeleteFiles)
    {
        STATUS statTmp;
        std::string answerTmp;
        std::string command = GetCmdLinePrefix() + "rm -f "+MCU_DATA_DIR+"/new_version/*";
	    statTmp = SystemPipedCommand(command.c_str(), answerTmp);
        
        TRACESTR(statTmp ? eLevelError:eLevelInfoNormal) <<
    		"CConfiguratorManager::RemountVersionPartition - clean new_version directory: " << answerTmp;
    }

    if (withWritePermission && isToDeleteFallbackInNeeded)
    {
    	std::string answerTmp;
    	STATUS statTmp = SystemPipedCommand("df | grep data | awk -F ' ' '{ print $4 }'",answerTmp);
    	int free_space_kb = atoi(answerTmp.c_str());
    	TRACESTR(eLevelInfoNormal) <<
    			"CConfiguratorManager::RemountVersionPartition - "<<MCU_DATA_DIR<<" free space is (KB):  " << free_space_kb;

	    int base_free_space_kb = 390000;
       // BRIDGE-12908
	    //eProductType curProductType = m_productTypeDecider->GetProductType();
    	//if(eProductTypeGesher == curProductType || eProductTypeNinja == curProductType)
	   // {
		//    base_free_space_kb = 150000;
	   // }
    	if (free_space_kb < base_free_space_kb)
    	{
    		std::string answer1, answer2;
    		stat = SystemPipedCommand(("readlink "+MCU_DATA_DIR+"/fallback").c_str(),answer1);
    		stat = SystemPipedCommand(("readlink "+MCU_DATA_DIR+"/current").c_str(),answer2);
    		//Verify current & fallback do not point to the same binary
    		if(answer1 != answer2)
    		{
    			std::string answerTmp2;
    			STATUS statTmp2;
                std::string command = GetCmdLinePrefix() + "rm -f "+MCU_DATA_DIR+"/"+"`readlink "+MCU_DATA_DIR+"/fallback`";
    			statTmp2 = SystemPipedCommand(command.c_str(), answerTmp2);
    			TRACESTR(statTmp2 ? eLevelError:eLevelInfoNormal) <<
    					"CConfiguratorManager::RemountVersionPartition - delete old fallback to free space: " << answerTmp2;
    		}
    	}
    }

    if(STATUS_OK != stat)
    {
        stat = STATUS_FAIL;
    }

    ResponedClientRequest(stat);
}

/////////////////////////////////////////////////////////////////////////////
void CConfiguratorManager::HandleFallbackVersion(CSegment* pSeg)
{
	TEST_ROOT_AND_RETURN

	std::string answerTmp="", linkfallback="", linkcurrent="";
	STATUS statCurrent = SystemPipedCommand(("readlink "+MCU_DATA_DIR+"/current").c_str(),linkcurrent);
	STATUS statTmp = SystemPipedCommand(("readlink "+MCU_DATA_DIR+"/fallback").c_str(),linkfallback);
	TRACESTR(statCurrent ? eLevelError:eLevelInfoNormal) << "CConfiguratorManager::HandleFallbackVersion - current link=" << linkcurrent;
	TRACESTR(statTmp ? eLevelError:eLevelInfoNormal) << "CConfiguratorManager::HandleFallbackVersion - fallback link=" << linkfallback;
	if(STATUS_OK == statTmp && STATUS_OK == statTmp && linkfallback != linkcurrent)
	{
		eProductType curProductType = m_productTypeDecider->GetProductType();

		std::string cmd = GetCmdLinePrefix() + "rm -f "+MCU_DATA_DIR + "/"  + linkfallback;
		STATUS statTmp = SystemPipedCommand(cmd.c_str(), answerTmp);
		TRACESTR(statTmp ? eLevelError:eLevelInfoNormal) << "CConfiguratorManager::HandleFallbackVersion - removed fallback version " << answerTmp;
	}

	ResponedClientRequest(statTmp);
}

// Static, Private
STATUS CConfiguratorManager::IsPingEnabled(BOOL& enable)
{
  std::string ans;
  const char* cmd = "cat /proc/sys/net/ipv4/icmp_echo_ignore_all";

  STATUS stat = SystemPipedCommand(cmd, ans);
  FPASSERTSTREAM_AND_RETURN_VALUE(STATUS_OK != stat,
      cmd << ": " << ans << ": " << stat,
      STATUS_FAIL);

  switch (ans[0])
  {
  case '0':
    enable = TRUE;
    break;

  case '1':
    enable = FALSE;
    break;

  default:
    FPASSERTSTREAM_AND_RETURN_VALUE(true, "Illegal value " << ans, STATUS_FAIL);
  }

  return STATUS_OK;
}

void CConfiguratorManager::EnableDisablePing(CSegment* seg)
{
  BOOL enable;
  *seg >> enable;

  // Reads current value
  BOOL old_value;
  STATUS stat = IsPingEnabled(old_value);
  if (STATUS_OK != stat)
  {
    ResponedClientRequest(STATUS_FAIL);
    return;
  }

  if (enable == old_value)
  {
    ResponedClientRequest(STATUS_OK);
    TRACEINTO << "Echo respose is "
              << (enable ? "enabled" : "disabled")
              << " already";
    return;
  }

  TEST_ROOT_AND_RETURN

  // Changes a value of the property
  std::string ans;
  const char* cmd = enable ?
      "echo 0 > /proc/sys/net/ipv4/icmp_echo_ignore_all" :
      "echo 1 > /proc/sys/net/ipv4/icmp_echo_ignore_all";

  stat = SystemPipedCommand(cmd, ans);
  PASSERTSTREAM(stat != STATUS_OK, cmd << ": " << ans << ": " << stat);

  // Verifies the new value.
  BOOL new_value;
  stat = IsPingEnabled(new_value);
  if (STATUS_OK != stat)
  {
    ResponedClientRequest(STATUS_FAIL);
    return;
  }

  TRACEINTO << "Echo respose status is changed from "
            << (old_value ? "enabled" : "disabled")
            << " to "
            << (new_value ? "enabled" : "disabled");

  if (STATUS_OK != stat)
    stat = STATUS_FAIL;

  ResponedClientRequest(stat);
}

void CConfiguratorManager::EnableDisablePingBroadcast(CSegment* seg)
{
  TEST_ROOT_AND_RETURN

  BOOL enable;
  *seg >> enable;

  const char* cmd = enable ?
      "echo 0 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcast" :
      "echo 1 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcast";

  std::string ans;
  STATUS stat = SystemPipedCommand(cmd, ans);

  TRACESTR(stat ? eLevelError:eLevelInfoNormal)
      << "CConfiguratorManager::EnableDisablePingBroadcast:" << cmd
      << ": " << ans;

  if (STATUS_OK != stat)
    stat = STATUS_FAIL;

  ResponedClientRequest(stat);
}

// Static, Private
STATUS CConfiguratorManager::IsPingEnabledIptables(BOOL& enable)
{
  std::string ans;
  const char* cmd = "iptables --list | grep icmp | grep DROP 2>&1";

  STATUS stat = SystemPipedCommand(cmd, ans);
  FPASSERTSTREAM_AND_RETURN_VALUE(STATUS_OK != stat,
      cmd << ": " << ans << ": " << stat,
      STATUS_FAIL);

  switch (ans[0])
  {
  case '\0':
    enable = TRUE;
    break;

  case 'D':
    enable = FALSE;
    break;

  default:
    FPASSERTSTREAM_AND_RETURN_VALUE(true, "Illegal value " << ans, STATUS_FAIL);
  }

  return STATUS_OK;
}

/*void CConfiguratorManager::EnableDisablePingIptables(CSegment* seg)
{
  TEST_ROOT_AND_RETURN

  BOOL enable;
  *seg >> enable;

  // Reads current value
  BOOL old_value;
  STATUS stat = IsPingEnabledIptables(old_value);
  if (STATUS_OK != stat)
  {
    ResponedClientRequest(STATUS_FAIL);
    return;
  }

  if (enable == old_value)
  {
    ResponedClientRequest(STATUS_OK);
    TRACEINTO << "Echo respose is "
              << (enable ? "enabled" : "disabled")
              << " already";
    return;
  }

  TRACEINTO << "CConfiguratorManager::EnableDisablePingIptables, enable: "<< enable;
  // Changes a value of the property.
  std::string ans;
  std::string cmd;
  if (enable)
  {
    cmd = "iptables -D INPUT -p icmp --icmp-type 8 -j ACCEPT -d 169.254.128.10 2>&1 ; "
          "iptables -D INPUT -p icmp --icmp-type 8 -j DROP 2>&1 ; "
          "ip6tables -D INPUT -p icmpv6 --icmpv6-type 128 -j DROP 2>&1";
  }
  else
  {
    cmd = "iptables -I INPUT 1 -p icmp --icmp-type 8 -j ACCEPT -d 169.254.128.10 2>&1 ; "
          "iptables -I INPUT 2 -p icmp --icmp-type 8 -j DROP 2>&1 ; "
          "ip6tables -I INPUT 1 -p icmpv6 --icmpv6-type 128 -j DROP 2>&1";
  }

  //disable unreachable msgs
  BOOL enableSendingIcmpDestinationUnreachable;
  CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
  BOOL res = sysConfig->GetBOOLDataByKey(CFG_KEY_ENABLE_SENDING_ICMP_DESTINATION_UNREACHABLE, enableSendingIcmpDestinationUnreachable);
  TRACEINTO << "CConfiguratorManager::EnableDisablePingIptables, enableSendingIcmpDestinationUnreachable: "<< enableSendingIcmpDestinationUnreachable;
  PASSERTSTREAM_AND_RETURN(!res, "CSysConfig::GetBOOLDataByKey: " << CFG_KEY_ENABLE_SENDING_ICMP_DESTINATION_UNREACHABLE);
  if(enableSendingIcmpDestinationUnreachable == false)
  {
	  cmd = cmd + " ; iptables -A OUTPUT -p icmp --icmp-type destination-unreachable -j DROP 2>&1";
	  //strcat(cmd," ; iptables -A OUTPUT -p icmp --icmp-type destination-unreachable -j DROP 2>&1");
  }
  TRACEINTO << "CConfiguratorManager::EnableDisablePingIptables, cmd: "<< cmd;

  stat = SystemPipedCommand(cmd.c_str(), ans);
  PASSERTSTREAM(stat != STATUS_OK, cmd.c_str() << ": " << ans << ": " << stat);
  // Verifies the new value.
  BOOL new_value;
  stat = IsPingEnabledIptables(new_value);
  if (STATUS_OK != stat)
  {
    ResponedClientRequest(STATUS_FAIL);
    return;
  }

  TRACEINTO << "Echo respose status is changed from "
            << (old_value ? "enabled" : "disabled")
            << " to "
            << (new_value ? "enabled" : "disabled");

  if (STATUS_OK != stat)
    stat = STATUS_FAIL;

  ResponedClientRequest(stat);
}*/
void CConfiguratorManager::AddPortsIpTables(CSegment* seg)
{
	TEST_ROOT_AND_RETURN;
	std::string hostIp,ports,ans;
	*seg >> hostIp >> ports;
	std::string cmd = "iptables -I mngmt2fw 3 ";
	cmd += "-p tcp -m multiport --dports ";
	cmd +=ports + " -j ACCEPT";
	TRACEINTOFUNC <<"CConfiguratorManager::AddPortsIpTables - command: " << cmd.c_str();
	STATUS stat =SystemPipedCommand(cmd.c_str(), ans);
	PASSERTSTREAM(stat != STATUS_OK, cmd << ": " << ans << ": " << stat);
	if (STATUS_OK != stat)
	    stat = STATUS_FAIL;

	ResponedClientRequest(stat);
//iptables -A INPUT -d 203.35.202.157 -p tcp -m multiport --dports 80 -j ACCEPT
}
void CConfiguratorManager::AddRootUserToSystem(CSegment *pSeg)
{
	TEST_ROOT_AND_RETURN;

    std::string usrname,userpassword, cmd, answer;
    *pSeg >> usrname >> userpassword;

    STATUS stat = AddUserToOS(usrname, userpassword, answer);

    TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
        "CConfiguratorManager::AddRootUserToSystem adduser :" << cmd << std::endl << answer;

    ResponedClientRequest(stat);
}

void CConfiguratorManager::DelUserFromSystem(CSegment *pSeg)
{
	TEST_ROOT_AND_RETURN

    std::string username,del_user_cmd, check_user_deleted_cmd;
    *pSeg >> username;
    if (username.empty())
    {
    	ResponedClientRequest(STATUS_EMPTY_USERNAME);
    	return;
    }

    del_user_cmd = "deluser " + username;

    std::string answer;
    STATUS stat = SystemPipedCommand(del_user_cmd.c_str() ,answer);
    TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
        "CConfiguratorManager::DelUserFromSystem:" << del_user_cmd << std::endl << answer;

    std::string script_answer;
    check_user_deleted_cmd = "Scripts/IsUserDeleted.sh " + username;
    stat = SystemPipedCommand(check_user_deleted_cmd.c_str(),script_answer);
    TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
        "CConfiguratorManager::DelUserFromSystem:" << check_user_deleted_cmd << std::endl << script_answer;

    if(STATUS_OK != stat)
    {
        stat = STATUS_FAIL;
    }
    ResponedClientRequest(stat);
}

void CConfiguratorManager::ChangePassword(CSegment *pSeg)
{
	TEST_ROOT_AND_RETURN;

    std::string username,new_password;
    *pSeg >> username >> new_password;

    std::string cmd = "deluser " + username;
    std::string answer;

    STATUS stat = SystemPipedCommand(cmd.c_str(),answer);

    TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
        "CConfiguratorManager::ChangePassword:" << cmd << std::endl << answer;

    if(STATUS_OK == stat || 256 == stat) // delete user returns 256 but it looks ok.
    {
        stat = AddUserToOS(username, new_password, answer);
        TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
            "CConfiguratorManager::ChangePassword:" << "adduser" << std::endl << answer;
    }

    if(STATUS_OK != stat)
    {
        stat = STATUS_FAIL;
    }
    ResponedClientRequest(stat);
}
void CConfiguratorManager::RestartSnmpd(CSegment *pSeg)
{
    eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

    std::string cmd;

    if(eProductTypeGesher == curProductType || eProductTypeNinja == curProductType )
    {
        cmd = "sudo killall -q -1 snmpd ; echo Restart snmpd;";
    }
    else if(eProductTypeSoftMCU == curProductType || eProductTypeEdgeAxis == curProductType ||
    		eProductTypeSoftMCUMfw == curProductType)
    {
        cmd = "sudo /usr/bin/killall -q -1 snmpd ; echo Restart snmpd;";
    }
    else
    {
        cmd = "killall -q -1 snmpd ; echo Restart snmpd;";
    }
    std::string answer;

    STATUS stat = SystemPipedCommand(cmd.c_str(),answer);
    TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
        "CConfiguratorManager::RestartSnmpd :" << cmd << std::endl << answer;

    if(STATUS_OK != stat)
    {
        stat = STATUS_FAIL;
    }
    ResponedClientRequest(stat);
}

void CConfiguratorManager::StartSnmpd(CSegment *pSeg)
{
    eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

        STATUS stat = STATUS_FAIL;

        CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
        BOOL isSNMPFipsModeMode = NO;
        sysConfig->GetBOOLDataByKey(CFG_SNMP_FIPS_MODE, isSNMPFipsModeMode);

	std::string strCmd = "Bin/snmpd";
	if (isSNMPFipsModeMode)
	{
		TRACEINTO << "running snmp in fips mode";
	    strCmd = strCmd + "j";
	}

	std::string answer;
    stat  = SystemPipedCommand(strCmd.c_str(),answer);

    ResponedClientRequest(stat);
}

void CConfiguratorManager::StopSnmpd(CSegment *pSeg)
{
    eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

    std::string cmd;
    if(eProductTypeGesher == curProductType || eProductTypeNinja == curProductType)
	{
    	cmd = "sudo killall -q snmpd ; echo Stop snmpd";
	}
    else if(eProductTypeSoftMCU == curProductType || eProductTypeEdgeAxis == curProductType ||
    		eProductTypeSoftMCUMfw == curProductType)
    {
        cmd = "sudo /usr/bin/killall -q snmpd ; echo Stop snmpd";
    }
    else
	{
    	cmd = "killall -q snmpd ; echo Stop snmpd";
	}
    std::string answer;
    STATUS stat = SystemPipedCommand(cmd.c_str(),answer);
    TRACESTR(stat ? eLevelError:eLevelInfoNormal) << cmd << ": " << answer;

    if (STATUS_OK != stat)
      stat = STATUS_FAIL;
    ResponedClientRequest(stat);

}

// The command doesn't respond to client. Built for asynchronous requests.
//
// Sending the HUP or restart signal to the parent causes it to kill off its
// children like in TERM, but the parent doesn't exit. It re-reads its
// configuration files, and re-opens any log files. Then it spawns a new set of
// children and continues serving hits.
//
// Original command:
//     apachectl -k restart
void CConfiguratorManager::RestartApache(CSegment*)
{
  std::string cmd;
  eProductType curProductType  = CProcessBase::GetProcess()->GetProductType();

  // Allows to run as non-root in simulation.
  if (IsRmxSimulation())
  {
	  cmd = "sudo env MCU_HOME_DIR="+(std::string)GET_MCU_HOME_DIR+" LD_LIBRARY_PATH="+MCU_APACHE_DIR+"/lib:"+MCU_MCMS_DIR+"/Bin "+MCU_MCMS_DIR+"/Bin/httpd -f "+MCU_MCMS_DIR+"/StaticCfg/httpd.conf.sim -k restart 2>&1";
  }
  else
  {
	  if ( eProductFamilyRMX == CProcessBase::GetProcess()->GetProductFamily())
  	  {
	      TEST_ROOT_AND_RETURN
	      cmd = "/usr/sbin/httpd -f "+MCU_MCMS_DIR+"/StaticCfg/httpd.conf -k restart 2>&1";
	  }
	  else
	  {
		  std::string result;
		  STATUS stat = SystemPipedCommand("echo -n `whoami`", result);
		  if (result != "mcms")
			  cmd = "sudo env MCU_HOME_DIR="+(std::string)GET_MCU_HOME_DIR+" LD_LIBRARY_PATH="+MCU_APACHE_DIR+"/lib:"+MCU_MCMS_DIR+"/Bin "+MCU_MCMS_DIR+"/Bin/httpd -f "+MCU_MCMS_DIR+"/StaticCfg/httpd.conf.sim -k restart 2>&1";
		  else
			  cmd = "sudo env MCU_HOME_DIR="+(std::string)GET_MCU_HOME_DIR+" LD_LIBRARY_PATH="+MCU_APACHE_DIR+"/lib:"+MCU_MCMS_DIR+"/Bin "+MCU_MCMS_DIR+"/Bin/httpd -f "+MCU_MCMS_DIR+"/StaticCfg/httpd.conf -k restart 2>&1";  // Fix BRIDGE-4989 //add new lib path for single httpd
	  }
  }

  std::string ans;
  STATUS stat = SystemPipedCommand(cmd.c_str(), ans);
  PASSERTSTREAM_AND_RETURN(stat != STATUS_OK,
    "SystemPipedCommand: " << cmd << ": " << ans);

  TRACEINTO << "Apache is soft restarted";
}



void CConfiguratorManager::RestartSSH(CSegment *pSeg)
{
	//TEST_ROOT_AND_RETURN

	STATUS stat = STATUS_FAIL;
	CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	BOOL isJITCMode = NO;
	BYTE isSSHOn = NO; //is ON No means SSH Still runs but only on internal IP Address 169.254.128.10 for debug from Switch

	sysConfig->GetBOOLDataByKey(CFG_KEY_JITC_MODE, isJITCMode);

	if(!isJITCMode)
	{
		BYTE isPermanentNetwork;
		std::string ipV4;
		std::string ipV6;

		*pSeg >> isPermanentNetwork;
		*pSeg >> ipV4;
		*pSeg >> ipV6;
		*pSeg >> isSSHOn;

		COstrStream ssh_cmd;
		
		ssh_cmd << "(cat "+MCU_MCMS_DIR+"/StaticCfg/sshd.conf ";

		if(isPermanentNetwork)
		{
			if (isSSHOn)
				ssh_cmd << "; echo ListenAddress " << "169.254.192.10";
		}

		if (ipV4 != "")
		{
			if (isSSHOn)
			{
				ssh_cmd << "; echo ListenAddress " << ipV4;
			}
		}
		ssh_cmd << "; echo ListenAddress " << "169.254.128.10";  //kobig always Listen On the internal Address for Debugging from the Switch

		TRACESTR(eLevelInfoNormal) << "\nCConfiguratorManager::RestartSSHD, ipV6=" << ipV6;

		if (ipV6 != "" && ipV6 != "::")
		{
			if (isSSHOn)
				ssh_cmd << "; echo ListenAddress " << ipV6;
		}

		if(m_productTypeDecider->IsGesherNinja())
		{
			ssh_cmd << ") > "+MCU_TMP_DIR+"/sshd.conf";
		}
		else
		{
			if (IsRmxSimulation())
				ssh_cmd << ") > "+MCU_TMP_DIR+"/sshd.conf";  // we do not want in RMX simulation to really close the ssh
			else
				ssh_cmd << ") > "+MCU_TMP_DIR+"/sshd.conf ; killall -1 sshd";
		}

		std::string answer;
			stat = SystemPipedCommand(ssh_cmd.str().c_str(), answer);

		if(STATUS_OK != stat)
		{
			stat = STATUS_FAIL;
		}

		// temp temp temp - for debugging
		TRACESTR(eLevelInfoNormal) << "\nCConfiguratorManager::RestartSSHD"
							   << "\nCmd:    " << ssh_cmd.str().c_str()
							   << "\nstat:   " << stat << " (" << CProcessBase::GetProcess()->GetStatusAsString(stat) << ")"
							   << "\nAnswer: " << answer;
		// temp temp temp - for debugging
		
		StartTimer(CONFIG_SSHD_TIMER, SECOND);

	}
	else
		TRACESTR(eLevelInfoNormal) << "\nCConfiguratorManager::RestartSSH : ssh is not allowed under JITC Mode!";

    ResponedClientRequest(stat);
}

void CConfiguratorManager::OnConfigSSHTimeout()
{
  COstrStream ssh_cmd;
  string answer;

  // Restarts sshd
  if (IsRmxSimulation())
	  return;

  if(m_productTypeDecider->IsGesherNinja())
  {
    ssh_cmd.str("sudo killall -1 sshd");
  }
  else
  {
    ssh_cmd.str("killall -1 sshd");
  }

  SystemPipedCommand(ssh_cmd.str().c_str(), answer);
}

void CConfiguratorManager::SyncTimeWithSwitch(CSegment *pSeg)
{
	eProductType curProductType = m_productTypeDecider->GetProductType();

	if (!(eProductTypeGesher == curProductType || eProductTypeNinja == curProductType || eProductTypeEdgeAxis == curProductType))
	{
		TEST_ROOT_AND_RETURN
	}

	STATUS stat = STATUS_OK;
	if ( CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator )
	{
		std::string cmd;
		std::string answer;

		if (eProductTypeGesher == curProductType || eProductTypeNinja == curProductType || eProductTypeEdgeAxis == curProductType)
		{
			cmd = "Bin/NTP_Bypass_SoftMCU_Client";

			stat = SystemPipedCommand(cmd.c_str(),answer);
			TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
				"CConfiguratorManager::SyncTimeWithSwitch :" << cmd << std::endl << answer;

			if(STATUS_OK != stat)
			{
				stat = STATUS_FAIL;
			}			
		}
		else
		{
			cmd = "Bin/NTP_Bypass_Client";

			stat = SystemPipedCommand(cmd.c_str(),answer);
			TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
				"CConfiguratorManager::SyncTimeWithSwitch :" << cmd << std::endl << answer;

			if(STATUS_OK != stat)
			{
				stat = STATUS_FAIL;
			}
		}
	}

    ResponedClientRequest(stat);
}

STATUS CConfiguratorManager::ChangePasswordInShadowFile(const std::string & username, const std::string & password)
{
    int Length=0;
    char * crypted_password = crypt(password.c_str() ,"$5$D78gVn$");
    std::string str_crypt_passwd = crypted_password;
    Length = str_crypt_passwd.length();
    int Len_To_search=0;
    std::string cmd;
    std::string answer;
    std::string SearchString;
    std::string ReplaceString;
    cmd = "Scripts/ChangePassword.sh " + username;
    cmd += " ";
    cmd += "'\\$5\\$D78gVn\\$";
    Len_To_search = cmd.length();
    cmd += str_crypt_passwd.substr(10 , (Length-10));
    cmd +="'";
    SearchString = "/";
    ReplaceString = "\\/";
    string::size_type pos = Len_To_search;
    while ( (pos = cmd.find(SearchString, pos)) != string::npos ) {
        cmd.replace( pos, SearchString.size(), ReplaceString );
        pos+= 2;
    }
    STATUS stat = SystemPipedCommand(cmd.c_str(), answer, TRUE, FALSE);
    if(STATUS_OK != stat)
        stat = STATUS_FAIL;

    return stat;
}

STATUS CConfiguratorManager::ChangeUserIdTo200(const std::string & username)
{
    std::string cmd,answer;
    cmd = "Scripts/ChangeUserIDTo200.sh " + username;
    STATUS stat = SystemPipedCommand(cmd.c_str(),answer);
    TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
       "CConfiguratorManager::ChangeUserIDTo200 :" << cmd << std::endl << answer;

    if(STATUS_OK != stat)
    {
        stat = STATUS_FAIL;
    }
    return stat;
}

///////////////////////////////////////////////////////////////////////////////////////////////
void CConfiguratorManager::EnableDHCPIPv6(CSegment *pSeg)
{
	std::string ans;
	std::string cmd = "/sbin/dhclient -6";

	STATUS stat = SystemPipedCommand(cmd.c_str(), ans);

	TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
	"CConfiguratorManagerSoftMcu::EnableIPv6 :" << cmd << std::endl << "answer:" << ans << ", stat:" << stat;

	ResponedClientRequest(stat);
}

///////////////////////////////////////////////////////////////////////////////////////////////
void CConfiguratorManager::DisableDHCPIPv6(CSegment *pSeg)
{
	std::string ans;
	std::string cmd = "kill -9 `ps | grep 'dhclient -6' | grep -v grep | head -1 | tr -d ' ' | cut -d 'r' -f1`";

	STATUS stat = SystemPipedCommand(cmd.c_str(), ans);

	TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
	"CConfiguratorManagerSoftMcu::DisableIPv6 :" << cmd << std::endl << "answer:" << ans << ", stat:" << stat;

	ResponedClientRequest(stat);
}

/////////////////////////////////////////////////////////////////////////////
void CConfiguratorManager::GetDMAStatus(CSegment *pSeg)
{
	TEST_ROOT_AND_RETURN
    std::string hd;
    *pSeg >> hd;

    std::string cmd;
    std::string answer;

    cmd += "cat /proc/ide/";
    cmd += hd;
    cmd += "/settings | grep using_dma | awk '{ print $2 }'";

    
    STATUS stat = SystemPipedCommand(cmd.c_str(),answer);
    TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
        "CConfiguratorManager::GetDMAStatus :" << cmd << std::endl << answer;

    if (STATUS_OK == stat)        
    {        
        if (IsFileExists("/proc/ide/" + hd)==false)//if the file does not exists we assume support of DMA
        {
            TRACEINTO << " file deos not exists to GetDMAStatus";
            stat = STATUS_OK;
        }
        else if (answer.c_str()[0] != '1')
        {
             TRACEINTO << "success to GetDMAStatus";
             stat = STATUS_FAIL;
        }
    }
    else //according to sagi if the /proc/hda does not exists it is probably new tech that supports DMA anyway - remove the AA in that case
    {
        TRACEINTO << "failed to GetDMAStatus";
        stat = STATUS_OK;
    }
    ResponedClientRequest(stat);
}

/////////////////////////////////////////////////////////////////////////////
void CConfiguratorManager::TestEthSetting(CSegment *pSeg)
{
	TEST_ROOT_AND_RETURN
    std::string eth;
    *pSeg >> eth;

    std::string cmd;
    std::string answer;

    cmd += "/sbin/ethtool ";
    cmd += eth;
    cmd += "| grep 'Speed: 1000Mb/s'";


    STATUS stat = SystemPipedCommand(cmd.c_str(),answer);
    TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
        "CConfiguratorManager::TestEthSetting :" << cmd << std::endl << answer;

    cmd = "/sbin/ethtool ";
    cmd += eth;
    cmd += "| grep 'Duplex: Full'";


    STATUS stat2 = SystemPipedCommand(cmd.c_str(),answer);
    TRACESTR(stat2 ? eLevelError:eLevelInfoNormal) <<
        "CConfiguratorManager::TestEthSetting :" << cmd << std::endl << answer;

    STATUS res = STATUS_OK;

    if (stat != STATUS_OK)
        res = STATUS_FAIL;

    if (stat2 != STATUS_OK)
        res = STATUS_FAIL;


    ResponedClientRequest(res);
}

void CConfiguratorManager::SetEthSettings(CSegment *pSeg)
{
  // TEST_ROOT_AND_RETURN

  eConfigInterfaceType	ifType;
  ePortSpeedType		portSpeed;
  DWORD tmpIfType;
  DWORD tmpPortSpeed;


  *pSeg >> tmpIfType >> tmpPortSpeed;

  ifType = (eConfigInterfaceType) tmpIfType;
  portSpeed = (ePortSpeedType) tmpPortSpeed;

  std::string stParams = ParsePortSpeed(portSpeed);

	/************************************************************************/
	/* 23.2.10 added by Rachel Cohen                                        */
	/* VNGFE 2587 and VNGFE 2582   on RMX4000 only                          */
	/* managment and signalling port can not be configured to 1000MB        */
	/* do not advertise 1000MB                                              */
	/************************************************************************/

	//Only for RMX4000 (RMX1500 enable 1G)
	eProductType curProductType = m_productTypeDecider->GetProductType();

	if ((eProductTypeRMX4000 == curProductType) && ((ifType == eManagmentNetwork) ||(ifType == eSignalingNetwork) ))
	{
		if  ((portSpeed == ePortSpeed_1000_HalfDuplex) ||(portSpeed == ePortSpeed_1000_FullDuplex) )
		{
		  TRACEINTO << "Can not config speed of 1G to managment/signalling intf";
			return;
		}

		if (portSpeed == ePortSpeed_Auto )  stParams = stParams+ " advertise 0x000F" ;//advertise only 10,100 full and half
	}

    std::string eth = GetDeviceName(ifType);
    std::string cmd;

    if (eProductTypeGesher == curProductType || eProductTypeNinja == curProductType)
    {
        cmd = "sudo /sbin/ethtool -s " + eth + " " + stParams + " 2>&1";
    }
    else
    {
        //only for inner NIC eth0, settings are hardcoded
        if("eth0" == eth)
            stParams = "speed 100 autoneg off duplex full";


        cmd = "/sbin/ethtool -s " + eth + " " + stParams;
    }
	
    std::string answer;

    STATUS stat = SystemPipedCommand(cmd.c_str(),answer);
    TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
        "CConfiguratorManager::SetEthSetting :" << cmd << std::endl << answer;

    if(STATUS_OK != stat)
    {
        stat = STATUS_FAIL;
    }

    static BOOL startupETH0LinkCheck = false;
    //on startup start a link check until MCMS get first msg from Shelf - authenticationInd msg.
    if ((eProductTypeRMX2000 == curProductType) || (eProductTypeRMX4000 == curProductType) ||
            (eProductTypeRMX1500 == curProductType))
    {
    	if ((TRUE == IsTarget()) && (startupETH0LinkCheck == false))
    	{
    		StartTimer(CHECK_ETH0_LINK_TIMER, 120 *SECOND);
    		startupETH0LinkCheck = true;
    	}
    }
    //ResponedClientRequest(stat);
}

void CConfiguratorManager::BackupIPParameters()
{
    int sockfd, size  = 1;
    struct sockaddr ifa;
    struct ifreq *ifr;
    struct ifreq ifrr;
    struct sockaddr_in sa;

    if (0 > (sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)))
    {
        fprintf(stderr, "Cannot open socket.\n");
        exit(EXIT_FAILURE);
    }

    ifr = &ifrr;
    ifrr.ifr_addr.sa_family = AF_INET;

    eConfigInterfaceType ifType = GetMngrIfType(eIpType_IpV4);
	
    std::string nic_name = GetLogicalInterfaceName(ifType, eIpType_IpV4);

    strncpy(ifrr.ifr_name, nic_name.c_str(), sizeof(ifrr.ifr_name) - 1);
    ifrr.ifr_name[sizeof(ifrr.ifr_name) - 1] = '\0';

    if (ioctl(sockfd, SIOCGIFADDR, ifr) < 0)
    {
        //ERROR !!!!! MUST HANDLE!!!!
    }

    ifa = ifrr.ifr_addr;

    m_PreviousIP = inet_ntoa(inaddrr(ifr_addr.sa_data));

    if (0 == ioctl(sockfd, SIOCGIFNETMASK, ifr) &&
        strcmp("255.255.255.255", inet_ntoa(inaddrr(ifr_addr.sa_data))))
    {
        m_PreviousNetmask = inet_ntoa(inaddrr(ifr_addr.sa_data));
    }

    if (ifr->ifr_flags & IFF_BROADCAST)
    {
        if (0 == ioctl(sockfd, SIOCGIFBRDADDR, ifr) &&
            strcmp("0.0.0.0", inet_ntoa(inaddrr(ifr_addr.sa_data))))
        {
            m_PreviousBroadcast = inet_ntoa(inaddrr(ifr_addr.sa_data));
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
void CConfiguratorManager::RollbackIP()
{

	TEST_ROOT_AND_RETURN
    std::string answer;
    std::string ifconfig_cmd;
    STATUS stat = STATUS_FAIL;

    //Config new Management IP configuration
    eProductType curProductType = m_productTypeDecider->GetProductType();

   	ifconfig_cmd = GetCmdLinePrefix() + "/sbin/ifconfig eth0:1 " + m_PreviousIP + " netmask " + m_PreviousNetmask +  " broadcast " + m_PreviousBroadcast;
    stat = SystemPipedCommand(ifconfig_cmd.c_str() ,answer);
    TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
        "CConfiguratorManager::RollbackIP :" << answer;

    if(STATUS_OK != stat)
    {
        stat = STATUS_FAIL;
    }
    ResponedClientRequest(stat);
    return;
}


/////////////////////////////////////////////////////////////////////////////
/*
 * this function was moved to McuMngr VNGFE-6776
void CConfiguratorManager::GetNtpPeerStatus(CSegment *pSeg)

*/
void CConfiguratorManager::GetSmartErrors(CSegment* pSeg)
{
    if (SkipOperation() 
        || // no configuration should be done on Pizzas
        (CProcessBase::GetProcess()->GetProductFamily() == eProductFamilyCallGenerator))
    {
        ResponedClientRequest(STATUS_OK);
        return;
    }

  TEST_ROOT_AND_RETURN;

  std::string errors;
  //Check if HD is mounted on /dev/hdb (PATA)
  STATUS stat = SystemPipedCommand(("mount | grep "+MCU_OUTPUT_DIR+" | grep /dev/hdb").c_str(), errors);

  std::string cmd;
  if (!errors.empty())
    cmd = GetCmdLinePrefix() + "smartctl --attributes --log=selftest --quietmode=errorsonly /dev/hdb";
  else
    cmd =  GetCmdLinePrefix() + "smartctl --attributes --log=selftest --quietmode=errorsonly -d ata /dev/sda";

  STATUS status = SystemPipedCommand(cmd.c_str(), errors);
  if (!errors.empty())
  {
    stat = STATUS_SMART_REPORT_ERRORS;
    errors = "The hard disk S.M.A.R.T status contain errors, "
             "please take actions to avoid unexpected outage and data loss";
  }
  else if (STATUS_OK != status)
  {
    stat = STATUS_FAIL;
  }

  CSegment* seg = new CSegment;
  *seg << errors;

  ResponedClientRequest(stat, seg);
}

//////////////////////////////////////////////////////////////////////
void CConfiguratorManager::RunSmartSelfTest(CSegment* pSeg)
{
	if (SkipOperation()) // no configuration should be done on Pizzas
	{
		ResponedClientRequest(STATUS_OK);
		return;
	}
	TEST_ROOT_AND_RETURN;

	BOOL bShortTest = FALSE;
    std::string result;
    std::string cmd = GetCmdLinePrefix() + "smartctl -t";

    *pSeg >> bShortTest;

    if(bShortTest)
    	cmd += " short";
    else
    	cmd += " long";

    //Check if HD is mounted on /dev/hdb (PATA)
    STATUS stat = SystemPipedCommand(("mount | grep "+MCU_OUTPUT_DIR+" | grep /dev/hdb").c_str(), result);
    if(result != "")
    	cmd += " /dev/hdb";
    else
    	cmd += " -d ata /dev/sda";

    UnlockRelevantSemaphore();
    stat = SystemPipedCommand(cmd.c_str(), result);
    LockRelevantSemaphore();

    TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "CConfiguratorManager::RunSmartSelfTest :" << result;

    ResponedClientRequest(stat);
}

//////////////////////////////////////////////////////////////////////
void CConfiguratorManager::GetHDDTemperature(CSegment* pSeg)
{
	TEST_ROOT_AND_RETURN;

	std::string temperature;
	CSegment*  seg = new CSegment;
	UnlockRelevantSemaphore();  //VNGR-16713 added By Ori & Rachel - we suspect configurator stuck on "smartctl"
	STATUS status = GetHDTemperature(temperature);
	LockRelevantSemaphore();
	*seg << temperature;
	ResponedClientRequest(status,seg);
}

void CConfiguratorManager::GetHDDSize(CSegment* pSeg)
{
    if (SkipOperation())
    {
        const char* size = "40.0";
        FTRACEINTO << "Simulate HDD size of " << size << " GB";

        CSegment* seg = new CSegment;
        *seg << size;
        ResponedClientRequest(STATUS_OK, seg);
        return;
    }

    TEST_ROOT_AND_RETURN;

	//Check if HD is mounted on /dev/hdb (PATA)
	std::string result;
	std::string hdd_dev_name = "";
	STATUS stat = SystemPipedCommand(("mount | grep "+MCU_OUTPUT_DIR+" | grep /dev/hdb").c_str(), result);
	if(result != "")
		hdd_dev_name = "/dev/hdb";
	else
		hdd_dev_name = "/dev/sda";

	std::string size;
	// cmd: /sbin/fdisk -l /dev/sda | sed -n '2,2p'  |awk '{print $3 $4 $5 $6}'
	std::string cmd = GetCmdLinePrefix() + "/sbin/fdisk -l " + hdd_dev_name + " | sed -n '2,2p' | awk '{print $3 $4 $5 $6}'";
	stat = SystemPipedCommand(cmd.c_str(), size);

	CSegment* seg = new CSegment;
	*seg << size;
	ResponedClientRequest(stat,seg);
}

void CConfiguratorManager::GetHDDModel(CSegment* pSeg)
{
    if (SkipOperation())
    {
        const char* model = "INTEL SSDSA2M040G2GC";
        FTRACEINTO << "Simulate HDD model " << model;

        CSegment* seg = new CSegment;
        *seg << model;
        ResponedClientRequest(STATUS_OK, seg);
        return;
    }

    TEST_ROOT_AND_RETURN;

	//Check if HD is mounted on /dev/hdb (PATA)
	std::string result;
	std::string hdd_dev_name = "";
	STATUS stat = SystemPipedCommand(("mount | grep "+MCU_OUTPUT_DIR+" | grep /dev/hdb").c_str(), result);
	if(result != "")
		hdd_dev_name = "/dev/hdb";
	else
		hdd_dev_name = "/dev/sda";

	std::string model_details;
	std::string cmd = GetCmdLinePrefix() + "/usr/sbin/smartctl --all " + hdd_dev_name + " | grep  Device | head -1| awk '{print $3, $4}'";
	stat = SystemPipedCommand(cmd.c_str(), model_details);

	CSegment* seg = new CSegment;
	*seg << model_details;
	ResponedClientRequest(stat, seg);
}

void CConfiguratorManager::GetHDDFirmware(CSegment* pSeg)
{
    if (SkipOperation())
    {
        const char* model = "2CV102HB";
        FTRACEINTO << "Firmware version " << model;

        CSegment* seg = new CSegment;
        *seg << model;
        ResponedClientRequest(STATUS_OK, seg);
        return;
    }

    TEST_ROOT_AND_RETURN;

	//Check if HD is mounted on /dev/hdb (PATA)
	std::string result;
	std::string hdd_dev_name = "";
	STATUS stat = SystemPipedCommand(("mount | grep "+MCU_OUTPUT_DIR+" | grep /dev/hdb").c_str(), result);
	if(result != "")
		hdd_dev_name = "/dev/hdb";
	else
		hdd_dev_name = "/dev/sda";

	std::string hd_firmware;
	std::string cmd = GetCmdLinePrefix() + "/usr/sbin/smartctl --all " + hdd_dev_name + " | grep Firmware | awk '{print $3}'";
	stat = SystemPipedCommand(cmd.c_str(), hd_firmware);

	CSegment* seg = new CSegment;
	*seg << hd_firmware;
	ResponedClientRequest(stat, seg);
}

void CConfiguratorManager::GetFlashSize(CSegment* pSeg)
{
    if (!IsTarget())
    {
        std::string const size = m_productTypeDecider->GetSimulatedFlashSize();
        FTRACEINTO << "Simulate flash size of " << size << " MB";

        CSegment* seg = new CSegment;
        *seg << size;
        ResponedClientRequest(STATUS_OK, seg);
        return;
    }

    TEST_ROOT_AND_RETURN;

	std::string result;
	std::string size="0";
	STATUS stat = SystemPipedCommand("mount | grep /dev/hda", result);
	if (result != "")
	{
		std::string cmd = "/sbin/fdisk -l /dev/hda 2> /dev/null | grep Disk | cut -d' ' -f3";
		SystemPipedCommand(cmd.c_str(), size);
	}
	else
	{
		// Supports sda at new CPU
		stat = SystemPipedCommand("mount | grep /dev/sdb", result);
		if (result != "")
		{
			// BRIDGE-10912 - in IVB cntl cards there no flash, /dev/sda is SSD, /dev/sdb is USB disk when inserted
			SystemPipedCommand("/sbin/udevinfo --query=all --name=sdb | grep ID_BUS | grep usb", result);

			if( result != "" )
			{
				TRACEINTO << " Device /dev/sdb is USB Disk. No flash found";
			}
			else
			{
				std::string cmd = "/sbin/fdisk -l /dev/sdb 2> /dev/null | grep Disk | cut -d' ' -f3";
				SystemPipedCommand(cmd.c_str(), size);
			}
		}
	}

	CSegment*  seg = new CSegment;
	*seg << size;
	ResponedClientRequest(stat,seg);
}

void CConfiguratorManager::GetFlashModel(CSegment* pSeg)
{
    if (!IsTarget())
    {
        const char* model = m_productTypeDecider->IsGesherNinja() ? "NA" : "SMART CF";
        FTRACEINTO << "Simulate flash model " << model;

        CSegment* seg = new CSegment;
        *seg << model;
        ResponedClientRequest(STATUS_OK, seg);
        return;
    }

    TEST_ROOT_AND_RETURN;

	//Check if HD is mounted on /dev/hdb (PATA)
	std::string result, model_details;

	// Not applicable for new CPU
	STATUS stat = SystemPipedCommand("mount | grep /dev/hda", result);
	if(result != "")
	{
		std::string cmd = "/usr/sbin/smartctl --all /dev/hda | sed -n '5,5p' | awk '{print $3, $4, $5}'";
		STATUS stat = SystemPipedCommand(cmd.c_str(), model_details);
	}
	else
	{
		stat = SystemPipedCommand("mount | grep /dev/sdb", result);
		if(result != "")
		{
			std::string cmd = "/usr/sbin/smartctl --all /dev/sdb | sed -n '5,5p' | awk '{print $3, $4, $5}'";
			STATUS stat = SystemPipedCommand(cmd.c_str(), model_details);
		}
	}
	CSegment* seg = new CSegment;
	*seg << model_details;
	ResponedClientRequest(stat,seg);
}

void CConfiguratorManager::GetCPUDetails(CSegment*)
{
  std::string ans;
  const char* cmd = "cat /proc/cpuinfo | grep \"model name\" | cut -d':' -f2 2>&1";

	STATUS stat = SystemPipedCommand(cmd, ans);

	CSegment* seg = new CSegment;
	*seg << ans;
	ResponedClientRequest(stat, seg);
}

void CConfiguratorManager::GetRAMsize(CSegment* pSeg)
{
	if (IsTarget())
	{
		TEST_ROOT_AND_RETURN;
	}

	std::string result;
	STATUS stat = SystemPipedCommand("free | grep Mem | awk '{ print $2 }'", result);

	CSegment*  seg = new CSegment;
	*seg << result;
	ResponedClientRequest(stat, seg);
}

/////////////////////////////////////////////////////////////////////////////
STATUS CConfiguratorManager::AddUserToOS(const string & userName, const string & userPassword, string & answer)
{
    std::string cmd = "adduser " + userName;
    cmd += " -D -G mcms -h "+MCU_TMP_DIR;
    cmd += userName;

    STATUS stat = SystemPipedCommand(cmd.c_str(),answer);
    if (STATUS_OK == stat)
    {
        stat = ChangePasswordInShadowFile(userName , userPassword);
        if (STATUS_OK == stat)
        {
            stat = ChangeUserIdTo200(userName);
        }

        cmd = "chmod o-rwx "+MCU_TMP_DIR + userName;
        stat = SystemPipedCommand(cmd.c_str(),answer);
    }
    return stat;
}

void CConfiguratorManager::TakeCoreOwnership(CSegment *pSeg)
{
	TEST_ROOT_AND_RETURN
    std::string core_file;
    *pSeg >> core_file;

    std::string full_name = MCU_MCMS_DIR+"/Cores/";
    full_name += core_file;
    int stat = chown(full_name.c_str(),
                     200,200);
    chmod(full_name.c_str(), 0766);

    if (stat == 0)
    {
        ResponedClientRequest(STATUS_OK);
    }
    else
    {
        int error = errno;
        PASSERTMSG(errno,full_name.c_str());
        ResponedClientRequest(STATUS_FAIL);
    }

}

/////////////////////////////////////////////////////////////////////////////
void CConfiguratorManager::DeleteFile(CSegment *pSeg)
{
        TEST_ROOT_AND_RETURN
        if(NULL == pSeg)
        {
                ResponedClientRequest(STATUS_FAIL);
                return;
        }

        std::string file_name;
        *pSeg >> file_name;
        std::string cmd = "rm -f ";
                    cmd += file_name;
        std::string answer;

        STATUS stat = SystemPipedCommand(cmd.c_str(), answer);
        TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
                "CConfiguratorManager::DeleteFile: " << cmd << std::endl << answer;

        ResponedClientRequest(stat);
}


/////////////////////////////////////////////////////////////////////////////
void CConfiguratorManager::DeleteDir(CSegment *pSeg)
{
        TEST_ROOT_AND_RETURN
        if(NULL == pSeg)
        {
                ResponedClientRequest(STATUS_FAIL);
                return;
        }

        std::string dir_name;
        *pSeg >> dir_name;
        std::string cmd = "rm -rf ";
                    cmd += dir_name;
        std::string answer;

        STATUS stat = SystemPipedCommand(cmd.c_str(), answer);
        TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
                "CConfiguratorManager::DeleteDir: " << cmd << std::endl << answer;

        ResponedClientRequest(stat);
}


////////////////////////////////////////////////////////////////////////////
void CConfiguratorManager::SetTCPStackParams(CSegment *pSeg)
{
        TEST_ROOT_AND_RETURN
        if(NULL == pSeg)
        {
                ResponedClientRequest(STATUS_FAIL);
                return;
        }

        DWORD keepalive_time=0, keepalive_intvl;
        *pSeg >> keepalive_time
			  >> keepalive_intvl;

        COstrStream cmd1, cmd2;
        cmd1 << "echo " << keepalive_time << " > /proc/sys/net/ipv4/tcp_keepalive_time";
        cmd2 << "echo " << keepalive_intvl << " > /proc/sys/net/ipv4/tcp_keepalive_intvl";

        std::string answer;

        STATUS stat = SystemPipedCommand(cmd1.str().c_str(), answer);
        TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
                "CConfiguratorManager::SetTCPKeepaliveParams: " << cmd1 << std::endl << answer;

        stat = SystemPipedCommand(cmd2.str().c_str(), answer);
		TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
				"CConfiguratorManager::SetTCPKeepaliveParams: " << cmd2 << std::endl << answer;

        ResponedClientRequest(stat);
}

void CConfiguratorManager::DeleteTempFiles(CSegment* pSeg)
{
	TEST_ROOT_AND_RETURN

    std::string command = GetCmdLinePrefix() + "rm -f "+MCU_DATA_DIR+"/new_version/*";
	
    std::string answer;
    STATUS stat = SystemPipedCommand(command.c_str(),answer);

    TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
        "CConfiguratorManager::DeleteTempFiles:" << command << std::endl << answer.c_str();

    if(STATUS_OK != stat)
    {
        stat = STATUS_FAIL;
    }

    ResponedClientRequest(stat);
}

BOOL CConfiguratorManager::IsSeparatedNetworks() const
{
	CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	BOOL isJITCMode = NO;
	BOOL isManagmentSep = NO;
	BOOL result = FALSE;

	sysConfig->GetBOOLDataByKey("ULTRA_SECURE_MODE", isJITCMode);
	sysConfig->GetBOOLDataByKey(CFG_KEY_SEPARATE_NETWORK, isManagmentSep);

	if(isJITCMode && isManagmentSep)
		result = TRUE;

	return result;
}

void CConfiguratorManager::AddDefaultGWRoutingTableRule(CSegment* pSeg)
{
	TEST_ROOT_AND_RETURN

	std::string answer;
	std::string ip = "";
	std::string defaultGW = "";
	std::string rule_cmd = "";
	std::string route_cmd1 = "";
	std::string route_cmd2 = "";
	std::string static_route_cmd = "";
	std::string route_table = "";
	std::string cache_table_cmd = "";
	std::string remove_gw_cmd = "";
	std::string mngmt_gw_cmd = "";
	std::string Ipv6_default_gw = "";
	std::string cidrIpAddr = "";
	std::string cidrGWIpAddr = "";
	std::string stNicName = "";
	std::string ipv6SubNetMask = "64";
	eConfigInterfaceType ifType;
	eIpType ipType;
	eConfigInterfaceNum  ifNum;
	DWORD mask, tmpIfType,tmpIpType, num_of_routers;
	BOOL bIPv6 = FALSE;
	STATUS stat = STATUS_OK;

	*pSeg  >>  tmpIfType
	       >>  tmpIpType
		   >>  ip
		   >>  mask
		   >>  defaultGW
		   >> ipv6SubNetMask
		   >>  bIPv6
		   >> num_of_routers;

	ifType = (eConfigInterfaceType) tmpIfType;
	ipType = (eIpType) tmpIpType;

	ifNum = GetInterfaceNum(ifType,ipType);


	if(!bIPv6)
	{
		stNicName = GetLogicalInterfaceName(ifType, ipType);

		cidrIpAddr = ConvertIpAddressToCIDRNotation(ip, mask);

		route_table = GetRouteTableName(ifNum);
		TRACESTR(eLevelInfoNormal) << "CConfiguratorManager::AddDefaultGWRoutingTableRule " << "\n" << " ifType=" << ifType << " ifNum=" << ifNum << " stNicName=" << stNicName << " route_table=" << route_table << "\n";

		rule_cmd = GetCmdLinePrefix() + "ip rule add from " + ip + " lookup " + route_table;
		//route_cmd = "ip route add table " + route_table + " default src " + ip + " dev " + stNicName;

		route_cmd1 = GetCmdLinePrefix() + "ip route add table " + route_table + " unicast " + cidrIpAddr + " dev " + stNicName + " src " + ip;

		route_cmd2 = GetCmdLinePrefix() + "ip route add table " + route_table + " default via " + defaultGW + " dev " + stNicName;

		stat = SystemPipedCommand(rule_cmd.c_str() ,answer);
		TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "CConfiguratorManager::AddDefaultGWRoutingTableRule :" << rule_cmd << std::endl << answer;

		stat = SystemPipedCommand(route_cmd1.c_str() ,answer);
		TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "CConfiguratorManager::AddDefaultGWRoutingTableRule :" << route_cmd1 << std::endl << answer;

		stat = SystemPipedCommand(route_cmd2.c_str() ,answer);
		TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "CConfiguratorManager::AddDefaultGWRoutingTableRule :" << route_cmd2 << std::endl << answer;
	}
	else
	{
		//Net Sep Jitc
		if ( true==IsJitcAndNetSeparation()  )
		{
			CSegment* pSeg = new CSegment();
			std::string route_table = GetIPv6RouteTableName(ifNum);
			std::string NIC = GetLogicalInterfaceName(ifType, ipType);
			*pSeg << route_table << NIC;
			if ("eth0.2197"==NIC)
				StartTimer(CONFIG_NETWORK_SEPERATION_IPV6_VLAN_I, SECOND * 30, pSeg);
			else if ( "eth0.2198"==NIC)
				StartTimer(CONFIG_NETWORK_SEPERATION_IPV6_VLAN_II, SECOND * 30, pSeg);
		}
		else
		{
			TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "CConfiguratorManager::AddDefaultGWRoutingTableRule ip:" << ip << " mask: " << mask << " default gw: " << defaultGW << " ipv6SubNetMask:" << ipv6SubNetMask << " bIPv6:" << (YES == bIPv6 ? "YES" : "NO") << std::endl;
			ConfigureNetworkSeperationIpv6Only(ifType, ipType,ip,mask,defaultGW,ipv6SubNetMask,bIPv6);
		}

	}

	if (stat == STATUS_OK)
	{
		TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "CConfiguratorManager::AddDefaultGWRoutingTableRule :num_of_routers=" << num_of_routers;

		for (DWORD i=0; i< num_of_routers; i++)
		{
			CRouter router;
			DWORD temp;
			*pSeg >> temp
				  >> router.m_targetIP
				  >> router.m_subNetmask
				  >> router.m_gateway;
			router.m_type = (eRouteType) temp;

			static_route_cmd = GetCmdLinePrefix() + "ip route add table " + route_table + " ";

			if (router.m_targetIP != "255.255.255.255")
			{

				cidrGWIpAddr = ConvertIpAddressToCIDRNotation(router.m_gateway, router.m_subNetmask);
				static_route_cmd += cidrGWIpAddr + " via " + router.m_targetIP + " dev " + stNicName;

				stat = SystemPipedCommand(static_route_cmd.c_str() ,answer);
				TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
					"CConfiguratorManager::AddDefaultGWRoutingTableRule :" << answer;
			}

		}
	}

	ResponedClientRequest(stat);
}

//This Function will Add Network routing table for ipv6 in Manaul Mode Only, for RMX 1500 & 4000 only
//When it is missing.
//Such a case will happen usually on Real network Separation when Configuration is with the same Default ipv6 router
//with 2 different ipv6 Subnets for signalling and Managment
//we will see if the rule appears , if not we will add it - according to the SubnetMask of ipv6
void CConfiguratorManager::ConfigureNetworkSeperationIpv6Only(eConfigInterfaceType ifType, eIpType ipType,std::string ip,DWORD mask,std::string defaultGW,std::string ipv6SubNetMask,BOOL bIPv6)
{
	std::string cmd = "";
	std::string answer = "";
	std::string strSearchstr = "";
	STATUS stat = STATUS_OK;
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	if (ip == "" || ip == "::" || ipv6SubNetMask=="" ||  YES != bIPv6 ||  (eProductTypeRMX4000 != curProductType && eProductTypeRMX1500 != curProductType) )
	{
		return;
	}

	std::string stNicName = GetLogicalInterfaceName(ifType, ipType);

//  Simulate the case of missing routing rule for ipv6 on subnet mask
//	cmd = GetCmdLinePrefix() + "ip -6 route del abcd:10:226:116::/64 dev eth2";
//	stat = SystemPipedCommand(cmd.c_str() ,answer);

	int ipV6Prefix = atoi(ipv6SubNetMask.c_str());
	int numberOfInstances = 0;
	string::size_type pos = 0;
	while((pos = ip.find(':', pos)) != string::npos)
	{
	        numberOfInstances++;
	        pos += sizeof(char);
	        if (numberOfInstances >= (ipV6Prefix/16) )
	        	break;
    }
	if (numberOfInstances <=0 )
		return;
	std::string subnetStr = ip.substr(0,pos) + ":/" + ipv6SubNetMask;
	cmd = GetCmdLinePrefix() + "ip -6 route | grep " + subnetStr + "| grep " + stNicName ;
	stat = SystemPipedCommand(cmd.c_str() ,answer);

	std::string prefix = subnetStr + " dev " + stNicName;
	if (answer.substr(0,prefix.size()) == prefix ){   // if the rule is already there do nothing
		TRACESTR(eLevelInfoNormal) << "CConfiguratorManager::ConfigureNetworkSeperationIpv6Only  the address does not exist:" << std::endl;
		return;
	}

	cmd = GetCmdLinePrefix() + "ip -6 route add " + subnetStr + " dev " + stNicName;
	stat = SystemPipedCommand(cmd.c_str() ,answer);

	//ip -6 route show | grep '^default via ' | grep eth0 | awk '{print $3}'
	cmd = GetCmdLinePrefix() + "ip -6 route show | grep '^default via ' | grep " + stNicName + " | awk '{print $3}'";
	stat = SystemPipedCommand(cmd.c_str() ,answer);
	if (answer=="") //no default gateway for ipv6 please add it
	{
		cmd = GetCmdLinePrefix() + "/sbin/route -A inet6 add default gw " + defaultGW + " dev " + stNicName;
		stat = SystemPipedCommand(cmd.c_str(), answer);
	}
}

void CConfiguratorManager::DelDefaultGWRoutingTableRule(CSegment* pSeg)
{
	TEST_ROOT_AND_RETURN

	std::string answer;
	std::string ip = "";
	std::string defaultGW = "";
	std::string rule_cmd = "";
	std::string route_cmd1 = "";
	std::string route_cmd2 = "";
	std::string route_table = "";
	std::string cache_table_cmd = "";
	std::string remove_gw_cmd = "";
	std::string mngmt_gw_cmd = "";
	std::string Ipv6_default_gw = "";
	std::string cidrIpAddr = "";
	std::string stNicName = "";
	eConfigInterfaceType ifType;
	eIpType ipType;
	eConfigInterfaceNum  ifNum;
	DWORD mask, tmpIfType,tmpIpType;
	BOOL bIPv6 = FALSE;
	STATUS stat = STATUS_OK;

	*pSeg  >>  tmpIfType
	       >>  tmpIpType
		   >>  ip
		   >>  mask
		   >>  defaultGW
		   >>  bIPv6;

	ifType = (eConfigInterfaceType) tmpIfType;
	ipType = (eIpType) tmpIpType;

	ifNum = GetInterfaceNum(ifType,ipType);


	cache_table_cmd = GetCmdLinePrefix() + "ip route flush cache";
	stat = SystemPipedCommand(cache_table_cmd.c_str() ,answer);
	TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "CConfiguratorManager::DelDefaultGWRoutingTableRule :" << cache_table_cmd << std::endl << answer;

	if(!bIPv6)
	{
		stNicName = GetLogicalInterfaceName(ifType, ipType);

		//cidrIpAddr = ConvertIpAddressToCIDRNotation(ip, mask);

		route_table = GetRouteTableName(ifNum);
		TRACESTR(eLevelInfoNormal) << "CConfiguratorManager::DelDefaultGWRoutingTableRule " << "\n" << " ifType=" << ifType << " ifNum=" << ifNum << " stNicName=" << stNicName << " route_table=" << route_table << "\n";

		rule_cmd = GetCmdLinePrefix() + "ip rule del from " + ip + " lookup " + route_table;
		//route_cmd = "ip route add table " + route_table + " default src " + ip + " dev " + stNicName;

		route_cmd1 = GetCmdLinePrefix() + "ip route flush table " + route_table ;

		stat = SystemPipedCommand(route_cmd1.c_str() ,answer);
		TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "CConfiguratorManager::DelDefaultGWRoutingTableRule :" << route_cmd1 << std::endl << answer;

		stat = SystemPipedCommand(rule_cmd.c_str() ,answer);
		TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "CConfiguratorManager::DelDefaultGWRoutingTableRule :" << rule_cmd << std::endl << answer;
	}
	else
	{
		route_table = GetIPv6RouteTableName(ifNum);
		stNicName = GetLogicalInterfaceName(ifType, ipType);

		//Get eth0.2197 default gw
		mngmt_gw_cmd = GetCmdLinePrefix() + "echo -n `ip -6 route | grep eth0.2197 | awk '/default/ { print $3 }'`";
		SystemPipedCommand(mngmt_gw_cmd.c_str(), Ipv6_default_gw);

		remove_gw_cmd = GetCmdLinePrefix() + "ip -6 route del via " + Ipv6_default_gw;

		/*rule_cmd = "ip -6 rule add from " + ip + " lookup " + route_table;
		//route_cmd = "ip route add table " + route_table + " default src " + ip + " dev " + stNicName;
		route_cmd = "ip -6 route add table " + route_table + " default via " + defaultGW;*/
		if (Ipv6_default_gw!="")
		{
		stat = SystemPipedCommand(remove_gw_cmd.c_str() ,answer);
		TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "CConfiguratorManager::DelDefaultGWRoutingTableRule 1:" << remove_gw_cmd << " ,answer: " << answer;
		}
	}

	ResponedClientRequest(stat);
}

//////////////////////////////////////////////////////////////////////
void CConfiguratorManager::SetProductType(CSegment* pSeg)
{
	TEST_ROOT_AND_RETURN

	eProductType productType;
    DWORD tmpProductType;

    *pSeg  >>  tmpProductType;
    productType	= (eProductType) tmpProductType;
    const char* sProductType = ProductTypeToString(productType);

    STATUS stat = STATUS_OK;

    FILE * pProductTypeFile = fopen((MCU_MCMS_DIR+"/ProductType").c_str(), "w" );
	if (pProductTypeFile==NULL)
	{
		TRACESTR(eLevelInfoNormal) << "\nCConfiguratorManager::SetProductType - failed to save product type file " << (MCU_MCMS_DIR+"/ProductType").c_str();
		stat = STATUS_FAIL;
	}

	else
	{
		// write to file
		rewind(pProductTypeFile);
		fputs(sProductType, pProductTypeFile);

		int fcloseReturn = 	fclose(pProductTypeFile);
		if (FCLOSE_SUCCESS != fcloseReturn)
		{
			perror("\nCConfiguratorManager::SetProductType - fclose failed. "); // for printing the errno
			TRACESTR(eLevelInfoNormal) << "\nCConfiguratorManager::SetProductType - failed to close file " << (MCU_MCMS_DIR+"/ProductType").c_str();
		}

        eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
        m_productTypeDecider->SetProductType(curProductType);
	}

	ResponedClientRequest(stat);
}

void CConfiguratorManager::SetSpecialProductType(CSegment* pSeg)
{
	std::string	specialProductType;


	*pSeg >>  specialProductType;


	std::string answerTmp;
	STATUS statTmp = SystemPipedCommand(("rm -f "+MCU_TMP_DIR+"/SpecialProductType").c_str(),answerTmp);
	        TRACESTR(statTmp ? eLevelError:eLevelInfoNormal) <<
	    		"CConfiguratorManager::SetSpecialProductType - remove file "+MCU_TMP_DIR+"/SpecialProductType : " << answerTmp;

	STATUS stat = STATUS_OK;
	FILE * pProductTypeFile = fopen((MCU_TMP_DIR+"/SpecialProductType").c_str(), "w" );
	if (pProductTypeFile==NULL)
	{
		TRACESTR(eLevelInfoNormal) << "\nCConfiguratorManager::SetSpecialProductType - failed to save product type file "+MCU_TMP_DIR+"/SpecialProductType";
		stat = STATUS_FAIL;
	}

	else
	{
		// write to file
		rewind(pProductTypeFile);
		fputs(specialProductType.c_str(), pProductTypeFile);

		int fcloseReturn = 	fclose(pProductTypeFile);
		if (FCLOSE_SUCCESS != fcloseReturn)
		{
			perror("\nCConfiguratorManager::SetProductType - fclose failed. "); // for printing the errno
			TRACESTR(eLevelInfoNormal) << "\nCConfiguratorManager::SetSpecialProductType - failed to close file "+MCU_TMP_DIR+"/SpecialProductType";
		}
	}
}

void CConfiguratorManager::EvokeNetworkInterfaces(CSegment *pSeg)
{
    TEST_ROOT_AND_RETURN
    DWORD tmpipConfigType;
    eV6ConfigurationType ipConfigType = eV6Configuration_Auto;
    *pSeg >> tmpipConfigType;
    ipConfigType = (eV6ConfigurationType)tmpipConfigType;
    STATUS stat = STATUS_OK;
    std::string answer;
    std:string accept_ra_cmd;
    //TODO: on ipv6only to remoe Router auto solicitation
    if (eV6Configuration_Manual ==  ipConfigType)
    {
		accept_ra_cmd = GetCmdLinePrefix() + "echo 0 > /proc/sys/net/ipv6/conf/eth1/accept_ra";
		stat = SystemPipedCommand(accept_ra_cmd.c_str(),answer);
    }

    // ----- 1. evoke eth1
    std::string cmd1 = "/sbin/ifconfig eth1 up";
    std::string answer1;
    stat = SystemPipedCommand(cmd1.c_str(), answer1);

    TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
        "CConfiguratorManager::EvokeNetworkInterfaces: " << cmd1 << std::endl << answer1;

    if(STATUS_OK != stat)
    {
        stat = STATUS_FAIL;
    }

    else // status == ok
    {

        //TODO: on ipv6only to remoe Router auto solicitation
	   if (eV6Configuration_Manual ==  ipConfigType)
	   {
			accept_ra_cmd = GetCmdLinePrefix() + "echo 0 > /proc/sys/net/ipv6/conf/eth2/accept_ra";
			stat = SystemPipedCommand(accept_ra_cmd.c_str(),answer);
	   }
        // ----- 2. evoke eth2
        std::string cmd2 = "/sbin/ifconfig eth2 up";
        std::string answer2;
        STATUS stat = SystemPipedCommand(cmd2.c_str(), answer2);

        TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
            "CConfiguratorManager::EvokeNetworkInterfaces: " << cmd2 << std::endl << answer2;

        if(STATUS_OK != stat)
        {
            stat = STATUS_FAIL;
        }
    }

    ResponedClientRequest(stat);
}

void CConfiguratorManager::SetMacAddress(CSegment *pSeg)
{
    TEST_ROOT_AND_RETURN
    STATUS stat = STATUS_OK;
	eConfigInterfaceType ifType;
	eIpType ipType;
    DWORD		tmpIfType , tmpIpType;
    std::string answer;
    std::string	macAddress;
    std::string cmdChangeSigMac = GetCmdLinePrefix() + "/usr/sbin/macchanger eth0.2198 | grep 'Faked MAC:' | awk '{ print $3 }'";

    *pSeg  >>  tmpIfType
    	   >> tmpIpType
		   >>  macAddress;

	ifType 					= (eConfigInterfaceType) tmpIfType;
	ipType 					= (eIpType) tmpIpType;

	if (eSeparatedSignalingNetwork==ifType) //for network seperation ipv6 purpose we will change the mac address of the Signalling/Media Vlan
	{
		stat = SystemPipedCommand(cmdChangeSigMac.c_str(),answer);
		macAddress = answer;
	}

    std::string	sEth	= GetLogicalInterfaceName(ifType,ipType);

    std::string cmd = GetCmdLinePrefix() + "/sbin/ifconfig ";
    eProductType curProductType = CProcessBase::GetProcess()->GetProductType();    

    if ((eProductTypeGesher == curProductType) || (eProductTypeNinja == curProductType)	)
    {
        cmd = "sudo /sbin/ifconfig ";
    }
    else
    {
    	cmd = "/sbin/ifconfig ";
    }
    cmd += sEth;
    cmd += " hw ether ";
    cmd += macAddress;

    stat = SystemPipedCommand(cmd.c_str(),answer);
    TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
        "CConfiguratorManager::SetMacAddress : " << cmd << std::endl << answer;


// ===============================================================
// ---------------------------------------------------------------
//	AMOS temp (as long as not implemented correctly)
// ---------------------------------------------------------------

    //   TODO 1:
    //		to be removed!!!!
			    /*std::string answer1;
			    STATUS stat1 = SystemPipedCommand("/sbin/shorewall clear" ,answer1);
			    TRACESTR(stat1 ? eLevelError:eLevelInfoNormal) <<
			        "CConfiguratorManager::SetMacAddress : /sbin/shorewall clear" << std::endl << answer1;*/
			//IptablesClear();
			//TRACESTR(eLevelInfoNormal) << "CConfiguratorManager::SetMacAddress : iptables cleared";

    //   TODO 2:
    //		remove the comment of 'TraceEnable off' in /StaticCfg/httpd.conf

// ---------------------------------------------------------------
// ===============================================================


    if(STATUS_OK != stat)
    {
        stat = STATUS_FAIL;
    }

    ResponedClientRequest(stat);
}

/////////////////////////////////////////////////////////////////////////////
void CConfiguratorManager::IptablesClear()
{
	std::string answer1;
	STATUS stat1 = SystemPipedCommand("iptables -P INPUT ACCEPT" ,answer1);
	TRACESTR(stat1 ? eLevelError:eLevelInfoNormal) << "CConfiguratorManager::IptablesClear : 1 " << std::endl << answer1;
	stat1 = SystemPipedCommand("iptables -P OUTPUT ACCEPT" ,answer1);
	TRACESTR(stat1 ? eLevelError:eLevelInfoNormal) << "CConfiguratorManager::IptablesClear : 2 " << std::endl << answer1;
	stat1 = SystemPipedCommand("iptables -P FORWARD ACCEPT" ,answer1);
	TRACESTR(stat1 ? eLevelError:eLevelInfoNormal) << "CConfiguratorManager::IptablesClear : 3 " << std::endl << answer1;
	stat1 = SystemPipedCommand("iptables -F" ,answer1);
	TRACESTR(stat1 ? eLevelError:eLevelInfoNormal) << "CConfiguratorManager::IptablesClear : 4 " << std::endl << answer1;
	stat1 = SystemPipedCommand("iptables -X" ,answer1);
	TRACESTR(stat1 ? eLevelError:eLevelInfoNormal) << "CConfiguratorManager::IptablesClear : 5 " << std::endl << answer1;
}

/////////////////////////////////////////////////////////////////////////////
void CConfiguratorManager::EthSettingsMonitoring(CSegment *pSeg)
{
    TEST_ROOT_AND_RETURN

    STATUS stat = STATUS_OK;

    DWORD portId;
    eConfigInterfaceNum	ifNum;
    DWORD tmpIfNum;

    *pSeg  >> tmpIfNum;

	ifNum = (eConfigInterfaceNum)tmpIfNum;

	ETH_SETTINGS_S *pCurEthStruct = new ETH_SETTINGS_S;
    FillEthSettingsStruct(pCurEthStruct, ifNum);

    CSegment *pRetSeg = new CSegment;
    pRetSeg->Put( (BYTE*)pCurEthStruct, sizeof(ETH_SETTINGS_S) );
    delete pCurEthStruct;

    if(STATUS_OK != stat)
    {
        stat = STATUS_FAIL;
    }

    ResponedClientRequest(stat, pRetSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CConfiguratorManager::FillEthSettingsStruct(ETH_SETTINGS_S *pCurEthStruct, eConfigInterfaceNum ifNum)
{
    const char *ifName = GetConfigInterfaceNumName(ifNum);

    std::string cmd;
    std::string answer;

    int retVal = EthGetEthInfo(ifName, pCurEthStruct);

	FTRACESTR(eLevelInfoNormal)  << "\nCConfiguratorManager::FillEthSettingsStruct: "
							 << "\nif name:  " << ifName
							 << "\nret val:  " << retVal
							 << "\nTxOctets: " << pCurEthStruct->ulTxOctets;
}

/////////////////////////////////////////////////////////////////////////////
void CConfiguratorManager::MountNewVersion(CSegment* pSeg)
{
	TEST_ROOT_AND_RETURN

    std::string cmd = GetMountNewVersionScriptCmdLine();
	
    std::string answer;

    STATUS stat = SystemPipedCommand(cmd.c_str(),answer);
    TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
        "CConfiguratorManager::MountNewVersion :" << cmd << std::endl << "answer:" << answer << ", stat:" << stat;

    if (STATUS_OK == stat && "STATUS_OK" == answer)
    {
        stat = STATUS_OK;
    }
    else
    {
        stat = STATUS_FAIL;
    }
    ResponedClientRequest(stat);
}

/////////////////////////////////////////////////////////////////////////////
void CConfiguratorManager::UnmountNewVersion(CSegment* pSeg)
{
	TEST_ROOT_AND_RETURN

    std::string cmd = GetUnmountNewVersionScriptCmdLine();
    std::string answer;

    STATUS stat = SystemPipedCommand(cmd.c_str(),answer);
    TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
        "CConfiguratorManager::UnmountNewVersion :" << cmd << std::endl << "answer:" << answer << ", stat:" << stat;

    if (STATUS_OK == stat && "STATUS_OK" == answer)
    {
        stat = STATUS_OK;
    }
    else
    {
        stat = STATUS_FAIL;
    }
    ResponedClientRequest(stat);
}

void CConfiguratorManager::FirmwareCheck(CSegment* pSeg)
{
	TEST_ROOT_AND_RETURN

    std::string cmd = GetFirmwareCheckScriptCmdLine();
    std::string answer;
	
    CSegment 	*seg = new CSegment;

    STATUS stat = SystemPipedCommand(cmd.c_str(),answer);
    TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
        "CConfiguratorManager::FirmwareCheck :" << cmd << std::endl << answer;

    *seg << answer;

    ResponedClientRequest(stat,seg);
}

void CConfiguratorManager::ExposeEmbeddedNewVersion(CSegment* pSeg)
{
	TEST_ROOT_AND_RETURN

	std::string cmd = MCU_MCMS_DIR+"/Scripts/ExposeNewEmbeddedVersion.sh ";
	std::string answer;
	CSegment 	*seg = new CSegment;

	STATUS stat = SystemPipedCommand(cmd.c_str(),answer);
	TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
		"CConfiguratorManager::ExposeEmbeddedNewVersion :" << cmd << std::endl << answer;

	*seg << answer;

	ResponedClientRequest(stat,seg);
}

void CConfiguratorManager::AddDropRuleToShorewall(CSegment* pSeg)
{
	TEST_ROOT_AND_RETURN

	std::string cmd = "shorewall drop ";
	std::string answer, drop_ip;
	CSegment 	*seg = new CSegment;

	*pSeg  >> drop_ip;

	cmd += drop_ip;

	STATUS stat = SystemPipedCommand(cmd.c_str(),answer);
	TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
		"CConfiguratorManager::AddDropRuleToShorewall :" << cmd << std::endl << answer;

	*seg << answer;

	ResponedClientRequest(stat,seg);
}

void CConfiguratorManager::AddDropRuleToIptables(CSegment* pSeg)
{
	TEST_ROOT_AND_RETURN

	std::string cmd = "iptables -I INPUT -s ";
	std::string answer, drop_ip;
	BOOL drop_ping_only = FALSE;
	CSegment 	*seg = new CSegment;

	*pSeg  >> drop_ip;
	*pSeg  >> drop_ping_only;

	if (drop_ping_only == FALSE)
	{
		cmd += drop_ip + " --dport 10005 -j DROP";

		STATUS stat = SystemPipedCommand(cmd.c_str(),answer);
		TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "CConfiguratorManager::AddDropRuleToIptables :" << cmd << std::endl << answer;
	}

	/********************************************************************************************************/
	/* 15.11.11 VNGR-23162 added by Rachel Cohen we need to drop also ping packet in case switch got reset  */
	/* in upgrade icmp. we do not want it to try connect mcms after it startuo again.                       */
	/********************************************************************************************************/
	cmd = "iptables -I INPUT -s ";
	cmd += drop_ip + " -p ICMP -j DROP";

	STATUS stat = SystemPipedCommand(cmd.c_str(),answer);
	TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "CConfiguratorManager::AddDropRuleToIptables :" << cmd << std::endl << answer;

	*seg << answer;

	ResponedClientRequest(stat,seg);
}
void CConfiguratorManager::AddQosManagementRuleToIptables(CSegment* pSeg)
{
	/*//add chain
	iptables -t mangle -N qos_management_network
	//add rule to chain (if repeated will add aditional line)
	iptables -t mangle -A qos_management_network -p tcp -m multiport --port 443,80,22 -j DSCP --set-dscp 17
	//add link to chain
	iptables -t mangle -A OUTPUT -j qos_management_network
	//clean rules inside chain
	iptables -t mangle -F qos_management_network
	//remove chain if not referenced (links)
	iptables -t mangle -X qos_management_network//
	//run tcpdump
	tcpdump -s 0 -i  eth1 -v -w MCU_OUTPUT_DIR+/tcp_dump/mcms/eran1.cap*/

	TRACESTR(eLevelInfoNormal) << "inside CConfiguratorManager::AddQosManagementRuleToIptables ";

	std::string cmd = "";
	std::string answer = "";
	std::string signalingDSCP = "";
	*pSeg  >> signalingDSCP;


	AddDSCPRuleToIpTables("iptables", signalingDSCP);
	AddDSCPRuleToIpTables("ip6tables", signalingDSCP);


	//if adding rule failed (faulty DSCP value) then add default rule (set dscp to 0x10) ip4
	cmd = GetCmdLinePrefix();
	cmd += "iptables -t mangle -L qos | grep 'DSCP'";
	SystemPipedCommand(cmd.c_str(),answer);
	if(answer == "")
	{
		AddDSCPRuleToIpTables("iptables", "0x10");
	}
	//if adding rule failed (faulty DSCP value) then add default rule (set dscp to 0x10) ip6
	cmd = GetCmdLinePrefix();
	cmd += "ip6tables -t mangle -L qos | grep 'DSCP'";
	SystemPipedCommand(cmd.c_str(),answer);
	if(answer == "")
	{
		AddDSCPRuleToIpTables("ip6tables", "0x10");
	}

}

void CConfiguratorManager::AddDSCPRuleToIpTables(std::string iptables, std::string signalingDSCP)
{
	std::string cmd = "";
	std::string answer = "";

	//add qos chain to iptables
	cmd = GetCmdLinePrefix();
	cmd += iptables;
	cmd += " -t mangle -N qos";
	SystemPipedCommand(cmd.c_str(),answer);


	//add qos link to OUTPUT
	cmd = GetCmdLinePrefix();
	cmd += iptables;
	cmd += " -t mangle -L OUTPUT | grep 'qos'";
	SystemPipedCommand(cmd.c_str(),answer);
	if(answer == "")
	{
		cmd = GetCmdLinePrefix();
		cmd += iptables;
		cmd += " -t mangle -A OUTPUT -j qos";
		SystemPipedCommand(cmd.c_str(),answer);
	}


	//add qos link to PREROUTING
	cmd = GetCmdLinePrefix();
	cmd += iptables;
	cmd += " -t mangle -L PREROUTING | grep 'qos'";
	SystemPipedCommand(cmd.c_str(),answer);
	if(answer == "")
	{
		cmd = GetCmdLinePrefix();
		cmd += iptables;
		cmd += " -t mangle -A PREROUTING -j qos";
		SystemPipedCommand(cmd.c_str(),answer);
	}


	//clean all rules in qos
	cmd = GetCmdLinePrefix();
	cmd += iptables;
	cmd += " -t mangle -F qos";
	SystemPipedCommand(cmd.c_str(),answer);

	//add DNS query rule to qos chain (destination port)
	cmd = GetCmdLinePrefix();
	cmd += iptables;
	cmd += " -t mangle -A qos -p udp --dport 53 -j DSCP --set-dscp ";
	cmd += signalingDSCP;
	SystemPipedCommand(cmd.c_str(),answer);



	//add SNMP rule to qos chain (source port)
	cmd = GetCmdLinePrefix();
	cmd += iptables;
	cmd += " -t mangle -A qos -p udp  --sport 161 -j DSCP --set-dscp ";
	cmd += signalingDSCP;
	SystemPipedCommand(cmd.c_str(),answer);


	//add rules to qos chain (source ports)
	cmd = GetCmdLinePrefix();
	cmd += iptables;
	cmd += " -t mangle -A qos -p tcp -m multiport --port 80,443,389,636,22 -j DSCP --set-dscp ";
	cmd += signalingDSCP;
	SystemPipedCommand(cmd.c_str(),answer);

}

/////////////////////////////////////////////////////////////////////////////
void CConfiguratorManager::RestoreFallbackVersion(CSegment* pSeg)
{
	TEST_ROOT_AND_RETURN

	std::string result;
    std::string command = GetCmdLinePrefix() + "mv -f "+MCU_DATA_DIR+"/"+"`readlink "+MCU_DATA_DIR+"/fallback` "+MCU_DATA_DIR+"/new_version/new_version.bin";
	STATUS stat = SystemPipedCommand(command.c_str(), result);
	
	TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "CConfiguratorManager::RestoreFallbackVersion :" << result;

	ResponedClientRequest(stat);
}

/***********************************************************************/
/* VNGR-18278 17.12.10 Rachel Cohen                                    */
/* at startup MCMS check eth0 link status and if it detect a problem   */
/* it restart the link                                                 */
/***********************************************************************/
void CConfiguratorManager::StopCheckEth0Link()
{
  DeleteTimer(CHECK_ETH0_LINK_TIMER );
  TRACEINTO << "Arrived it means AuthenticationInd arrived";
  ResponedClientRequest(STATUS_OK);
}

void CConfiguratorManager::StartCheckEth0Link()
{

	static BOOL startupETH0LinkCheck = false;
	//on startup start a link check until MCMS get first msg from Shelf - authenticationInd msg.


	if  (startupETH0LinkCheck == false)
	{
		StartTimer(CHECK_ETH0_LINK_TIMER, 120 *SECOND);
		startupETH0LinkCheck = true;
	}

	ResponedClientRequest(STATUS_OK);
}

void CConfiguratorManager::OnCheckEth2(CSegment* pSeg)
{	
	STATUS stat;
	std::string answer = "";
	stat = SystemPipedCommand("ethtool eth2 | grep 'Link detected' |grep 'yes' ",answer);
	if (stat != STATUS_OK)
	{
		TRACESTR(eLevelError) << "OnCheckEth2 - failed to check eth2";
	}
	BOOL link_detected = (answer != "");
		
	// TRACESTR(eLevelError) << "OnCheckEth2 - answer " << answer << " link_detected  " << (int)link_detected;
	
	CSegment*  ret_seg = new CSegment;
	*ret_seg << link_detected;
	
	ResponedClientRequest(stat,ret_seg);
}

void CConfiguratorManager::OnCheckEth0LinkTimeout(void)
{
	TRACEINTO << "Enter";

	TEST_ROOT_AND_RETURN

	DeleteTimer(CHECK_ETH0_LINK_TIMER);

	STATUS stat;
	std::string answer;
	stat = SystemPipedCommand("ethtool eth0 | grep 'Link detected' |grep 'yes' ",answer);

	if(answer != "")
	{
		TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "CConfiguratorManager::OnCheckEth0LinkTimeout :" << answer;
		return;
	}
	else
	{
        if (m_productTypeDecider->IsGesherNinja())
        {
            stat = SystemPipedCommand("sudo /sbin/ifconfig eth0 down ",answer);
		    stat = SystemPipedCommand("sudo /sbin/ifconfig eth0 up ",answer);
        }
        else
        {
			stat = SystemPipedCommand("ethtool eth0 ",answer);
			TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "CConfiguratorManager::OnCheckEth0LinkTimeout :Detected Link Problem" << answer;

			stat = SystemPipedCommand("ifconfig eth0 down ",answer);
			stat = SystemPipedCommand("ethtool -s eth0 speed 100 autoneg off duplex full",answer);  // we saw a case that the speed was 1G and the link was down
			stat = SystemPipedCommand("ifconfig eth0 up ",answer);
        }

        eProductType curProductType = m_productTypeDecider->GetProductType();
		if (eProductTypeRMX2000 == curProductType)
		{

		    // The command "ifconfig eth0 down" remove default gateway from route table we need to config it again.
		    std::string routeAdd_cmd;
		    routeAdd_cmd=  "/sbin/route add default gw " + m_PreviousDefaultGW + " " + m_StNicName;
		    stat = SystemPipedCommand(routeAdd_cmd.c_str(),answer);
		}
		else if ((eProductTypeRMX4000 == curProductType) || (eProductTypeRMX1500 == curProductType))
		{
		    BOOL isMultipleServices = NO;
		    CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
		    sysConfig->GetBOOLDataByKey(CFG_KEY_MULTIPLE_SERVICES, isMultipleServices);

		    if (NO == isMultipleServices /*&& TRUE == m_pLicensing->GetIsMultipleServicesEnabled()*/)
		    {
		        stat = SystemPipedCommand("ifconfig eth0.2012 down ",answer);
		        stat = SystemPipedCommand("ifconfig eth0.2013 down ",answer);
		        if (eProductTypeRMX4000 == curProductType)
		        {
		            stat = SystemPipedCommand("ifconfig eth0.2022 down ",answer);
		            stat = SystemPipedCommand("ifconfig eth0.2023 down ",answer);
		            stat = SystemPipedCommand("ifconfig eth0.2032 down ",answer);
		            stat = SystemPipedCommand("ifconfig eth0.2033 down ",answer);
		            stat = SystemPipedCommand("ifconfig eth0.2042 down ",answer);
		            stat = SystemPipedCommand("ifconfig eth0.2043 down ",answer);
		        }
		    }
		}

		StartTimer(CHECK_ETH0_LINK_TIMER, 60*SECOND);
	}
}

void CConfiguratorManager::ClearTcpDumpStorage()
{
	STATUS stat;
	std::string answer;
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	if ((eProductTypeGesher == curProductType) || (eProductTypeNinja == curProductType))
	{
		stat = SystemPipedCommand(("sudo rm "+MCU_OUTPUT_DIR+"/tcp_dump/emb/* ").c_str(),answer);
		stat = SystemPipedCommand(("sudo rm "+MCU_OUTPUT_DIR+"/tcp_dump/media-signaling/* ").c_str(),answer);
		stat = SystemPipedCommand(("sudo rm "+MCU_OUTPUT_DIR+"/tcp_dump/mcms/* ").c_str(),answer);
	}
	else if (eProductTypeSoftMCU == curProductType || eProductTypeSoftMCUMfw == curProductType || eProductTypeEdgeAxis == curProductType)
	{
		stat = SystemPipedCommand(("rm -f "+MCU_OUTPUT_DIR+"/tcp_dump/emb/* ").c_str(),answer);
		stat = SystemPipedCommand(("rm -f "+MCU_OUTPUT_DIR+"/tcp_dump/media-signaling/* ").c_str(),answer);
		stat = SystemPipedCommand(("rm -f "+MCU_OUTPUT_DIR+"/tcp_dump/mcms/* ").c_str(),answer);
	}
	else
	{
		stat = SystemPipedCommand(("rm "+MCU_OUTPUT_DIR+"/tcp_dump/emb/* ").c_str(),answer);
		stat = SystemPipedCommand(("rm "+MCU_OUTPUT_DIR+"/tcp_dump/mcms/* ").c_str(),answer);
	}


	 ResponedClientRequest(STATUS_OK);
}

void CConfiguratorManager::StartTcpDump(CSegment* pSeg)
{
	STATUS stat;
	std::string answer;
	std::string filter;
	std::string commandLine;
    std::string commandLine_lo = "";
    std::string startTimeStr;
    std::string ipStr;
    DWORD allocatedUnits;
    DWORD		ipTypeDword;
    eIpType		ipType;
    char allocatedUnitsStr[32];
    DWORD entitiyType;

    *pSeg  >>  entitiyType;
	*pSeg  >>  filter;
    *pSeg  >>  startTimeStr;
    *pSeg  >>  ipStr;
    *pSeg  >>  allocatedUnits;
    *pSeg  >>  ipTypeDword;

    ipType = (eIpType) ipTypeDword;

    sprintf(allocatedUnitsStr, "%d", allocatedUnits);

    TRACESTR(eLevelInfoNormal) << "CConfiguratorManager::StartTcpDump filter " << filter.c_str()
        << " StartTime " << startTimeStr << " Ip " << ipStr << " allocatedUnits " << allocatedUnits 
        << " ipType " << (int)ipType << " entitiyType:" << entitiyType;

	eProductType curProductType = m_productTypeDecider->GetProductType();
	eProductFamily curProductFamily = CProcessBase::GetProcess()->GetProductFamily();


	if (eProductTypeRMX2000 == curProductType)
	{
	    if (entitiyType == MNGMNT_ENTITY)
		{
			commandLine = GetCmdLinePrefix() + "tcpdump -s 0 -i  eth0 " + filter + " -v  -w "+MCU_OUTPUT_DIR+"/tcp_dump/mcms/"
        	    + startTimeStr + "_mgmt_" + ipStr + ".cap." + " -C 100 -W " + allocatedUnitsStr + " &";
		}
		else 
		{
			commandLine = GetCmdLinePrefix() + "tcpdump -s 0 -i  eth0 " + filter + " -v  -w "+MCU_OUTPUT_DIR+"/tcp_dump/mcms/"
        	    + startTimeStr + "_cs_" + ipStr + ".cap." + " -C 100 -W " + allocatedUnitsStr + " &";

		}
	}
	else if (eProductFamilySoftMcu == curProductFamily /* all SoftMcu products */)
	{
	    std::string eth = "";
	    GetNICFromIpAddress(ipStr.c_str(),  eth);
	    TRACESTR(eLevelInfoNormal) << "CConfiguratorManager::StartTcpDump GetNICFromIpAddress IpAddress: " << ipStr 
                << " NIC: " << eth << " entitiyType:" << entitiyType << " curProductType:" << curProductType;

        #if 0
        /*1. SoftMCU,MFW,EDGE: only one external interface, caputre both eth0(managment,signaling,media) and "lo" */
        if (eProductTypeSoftMCU == curProductType || eProductTypeSoftMCUMfw == curProductType || eProductTypeEdgeAxis == curProductType)
        {
            if (MNGMNT_ENTITY == entitiyType)
            {
                commandLine = "sudo /usr/sbin/tcpdump -s 0 -i " + eth + " " + filter + " -v  -w "+MCU_OUTPUT_DIR+"/tcp_dump/mcms/"
            	    + startTimeStr + "_mng_cs_media_" + ipStr + ".cap." + " -Z root -C 100 -W " + allocatedUnitsStr + " &";

                /*SoftMcu,EDGE,MFW: Capture "lo" automatically*/
                commandLine_lo = "sudo /usr/sbin/tcpdump -s 0 -i lo  -v  -w "+MCU_OUTPUT_DIR+"/tcp_dump/mcms/" + startTimeStr +
                                 "_cs_media_lo_127.0.0.1.cap. -Z root -C 100 -W 2 &";
            }
        }
        #endif
        //else
        //{
        //    /*2. Gesher,Ninja: (1).capture managment eth0; (2).caputre both eth1(signaling,media) and "lo" */
            if (entitiyType == MNGMNT_ENTITY)
            {
                commandLine = "sudo /usr/sbin/tcpdump -s 0 -i " + eth + " " + filter + " -v  -w "+MCU_OUTPUT_DIR+"/tcp_dump/mcms/"
                    + startTimeStr + "_mgmt_" + ipStr + ".cap." + " -Z root -C 100 -W " + allocatedUnitsStr + " &";
            }
            else 
            {
                //Create folder MCU_OUTPUT_DIR/tcp_dump/media-signaling if not exist.
                string folderName = MCU_OUTPUT_DIR+"/tcp_dump/media-signaling/";
                if (FALSE == IsFileExists(folderName))
                {
                    if (CreateDirectory(folderName.c_str(), 0777) == FALSE) 
                    {
                        TRACESTR(eLevelInfoNormal) << "CConfiguratorManager::StartTcpDump Create folder failed : " << folderName;
                    }
                }
                commandLine = "sudo /usr/sbin/tcpdump -s 0 -i  " + eth + " " + filter + " -v  -w "+MCU_OUTPUT_DIR+"/tcp_dump/media-signaling/"
                    + startTimeStr + "_media-signaling_" + ipStr + ".cap." + " -C 100 -W " + allocatedUnitsStr + " &";

                /*Gesher and Ninja: Capture "lo" automatically when capture signaling-media*/
                commandLine_lo = "sudo /usr/sbin/tcpdump -s 0 -i lo  -v  -w "+MCU_OUTPUT_DIR+"/tcp_dump/media-signaling/" + startTimeStr +
                                 "_media-signaling_lo_127.0.0.1.cap. -Z root -C 100 -W 2 &";   
            }
        //}

	}
	else
	{
		BOOL isMultipleServices = NO;
		CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
		sysConfig->GetBOOLDataByKey(CFG_KEY_MULTIPLE_SERVICES, isMultipleServices);

		BOOL isLAN_REDUNDANCY = YES;
		BOOL res = sysConfig->GetBOOLDataByKey(CFG_KEY_LAN_REDUNDANCY, isLAN_REDUNDANCY);

		if (isLAN_REDUNDANCY == YES)
		{
			if (ipType == eIpType_IpV6 || ipType == eIpType_Both)
			{
				isLAN_REDUNDANCY = NO;
			}
		}

		BOOL isV35JitcSupport = NO;
		sysConfig->GetBOOLDataByKey(CFG_KEY_V35_ULTRA_SECURED_SUPPORT, isV35JitcSupport);

		if (entitiyType == MNGMNT_ENTITY)
		{

			if (isLAN_REDUNDANCY == YES  &&   isMultipleServices == NO  && isV35JitcSupport == NO)
				commandLine = GetCmdLinePrefix() + "tcpdump -s 0 -i  bond0 " + filter + " -v  -w "+MCU_OUTPUT_DIR+"/tcp_dump/mcms/"
				+ startTimeStr + "_mgmt_" + ipStr + ".cap." + " -C 100 -W " + allocatedUnitsStr + " &";
			else

				commandLine = GetCmdLinePrefix() + "tcpdump -s 0 -i  eth1 " + filter + " -v  -w "+MCU_OUTPUT_DIR+"/tcp_dump/mcms/"
				+ startTimeStr + "_mgmt_" + ipStr + ".cap." + " -C 100 -W " + allocatedUnitsStr + " &";
		}
		else
		{
			if (isLAN_REDUNDANCY == YES &&   isMultipleServices == NO  && isV35JitcSupport == NO)
				commandLine = GetCmdLinePrefix() + "tcpdump -s 0 -i  bond0 " + filter + " -v  -w "+MCU_OUTPUT_DIR+"/tcp_dump/mcms/"
				+ startTimeStr + "_cs_" + ipStr + ".cap." + " -C 100 -W " + allocatedUnitsStr + " &";
			else
				commandLine = GetCmdLinePrefix() + "tcpdump -s 0 -i  eth2 " + filter + " -v  -w "+MCU_OUTPUT_DIR+"/tcp_dump/mcms/"
				+ startTimeStr + "_cs_" + ipStr + ".cap." + " -C 100 -W " + allocatedUnitsStr + " &";
		}

	}

    TRACESTR(eLevelInfoNormal) << "CConfiguratorManager::StartTcpDump commandLine:" << commandLine.c_str();
	std::string dummy;
    stat = SystemPipedCommand( commandLine.c_str() ,dummy, TRUE, TRUE,FALSE);

    /*softmcu,edge,mfw,gesher,ninja: capture "lo"*/
    if ("" != commandLine_lo)
    {
        std::string dummy_lo;
        TRACESTR(eLevelInfoNormal) << "CConfiguratorManager::StartTcpDump commandLine_lo:" << commandLine_lo.c_str();
        stat = SystemPipedCommand( commandLine_lo.c_str() ,dummy_lo, TRUE, TRUE,FALSE);
    }
    
	sleep(1);	//VNGR-21993

	stat = SystemPipedCommand("ps "  ,answer);
	TRACESTR(eLevelInfoNormal) << "CConfiguratorManager::StartTcpDump answer 1 " << answer.c_str();

	answer = "";

	TRACESTR(eLevelInfoNormal) << "CConfiguratorManager::StartTcpDump answer 2 " << answer.c_str();

	if (eProductFamilySoftMcu == curProductFamily /* all SoftMcu products */)
	{
		stat = SystemPipedCommand("ps -ef | awk '/tcpdump/ && !/awk/ {print $8}'"  ,answer);
	}
	else
	{
		stat = SystemPipedCommand("ps | awk '/tcpdump/ && !/awk/ {print $4}'"  ,answer);
	}

	TRACESTR(eLevelInfoNormal) << "CConfiguratorManager::StartTcpDump answer " << answer.c_str();

	if(answer == "")
		stat= STATUS_FAIL;
	else
		stat= STATUS_OK;


	TRACESTR(eLevelInfoNormal) << "CConfiguratorManager::StartTcpDump stat " << stat;

	CSegment 	*seg = new CSegment;
	*seg << answer;
	ResponedClientRequest(stat,seg);
}

void CConfiguratorManager::StopTcpDump()
{
	STATUS stat = STATUS_OK;
	std::string answer;
	std::string command;

	TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
			"CConfiguratorManager::StopTcpDump" ;

	eProductFamily curProductFamily = CProcessBase::GetProcess()->GetProductFamily();
	if (eProductFamilySoftMcu == curProductFamily /* all SoftMcu products */)
	{
	    command = "sudo killall tcpdump";
	}
	else
	{
	    command = "killall tcpdump";
	}

	stat = SystemPipedCommand(command.c_str(), answer);

	 ResponedClientRequest(stat);
}

void CConfiguratorManager::OnCheckChildZombieProcessesTimer(CSegment* pParam)
{
	TRACEINTO << "\nConfiguratorManager::OnCheckChildZombieProcessesTimer";

	pid_t childId = CollectZombieChildProcesses();
	while(childId > 0)
	{
		TRACEINTO << "\nConfiguratorManager::OnCheckChildZombieProcessesTimer: collected zombie process: " << childId;
		childId = CollectZombieChildProcesses();
	}
	StartTimer(CONFIG_CHECK_ZOMBIE_TOUT, ZOMBIE_TOUT);
}

void CConfiguratorManager::KillSsh(CSegment* seg)
{
    if (IsTarget())
    {
        TEST_ROOT_AND_RETURN
    }

    std::string ans;
    std::string cmd = GetCmdLinePrefix() + "killall -9 sshd 2>&1";
	
    STATUS stat = SystemPipedCommand(cmd.c_str(), ans);

    TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
        "CConfiguratorManager::KillSsh :" << cmd << std::endl << "answer:" << ans << ", stat:" << stat;

    ResponedClientRequest(stat);
}


void CConfiguratorManager::OnWriteFileRootOnly(CSegment* seg)
{
	WriteFile(seg, 0600);
	
}
void CConfiguratorManager::OnWriteFile(CSegment* seg)
{
	WriteFile(seg, 0);
}

void CConfiguratorManager::WriteFile(CSegment* seg, mode_t mode) 
{
  if (IsTarget())
  {
    TEST_ROOT_AND_RETURN
  }

  std::string fname, content;
  *seg >> fname >> content;

  // Removes old content
  STATUS stat;
  std::ofstream out(fname.c_str(), std::ios_base::trunc);
  if (out)
  {
    out << content;
    out.flush();
    

    if (out)
    {
    	if (mode != 0)
    	{
    		chmod(fname.c_str(), mode);
    	}
      stat = STATUS_OK;
    }
    else
    {
      stat = STATUS_FILE_WRITE_ERROR;
      PASSERTSTREAM(true,
          "Unable to write to file " << fname << ": " << strerror(errno));
    }
  }
  else
  {
    stat = STATUS_FILE_OPEN_ERROR;
    PASSERTSTREAM(true,
        "Unable to open file " << fname << ": " << strerror(errno));
  }

  CProcessBase* proc = CProcessBase::GetProcess();
  PASSERTMSG_AND_RETURN(NULL == proc, "Unable to continue");

  STATUS stat2 = ResponedClientRequest(stat);
  PASSERTSTREAM(STATUS_OK != stat2,
      "Unable to send a message: " << proc->GetStatusAsString(stat));
}

void CConfiguratorManager::GetValueFromRegister(CSegment* seg)
{
    if (IsTarget())
    {
        TEST_ROOT_AND_RETURN
    }
    std::string	sensor, register_address;

    *seg  >>  sensor
		  >>  register_address;

    std::string cmd =  "echo -n `/usr/bin/i2cget -y 0 ";
    cmd += sensor;
    cmd += " ";
    cmd += register_address;
    cmd += "`";

    std::string ans;
    STATUS stat = SystemPipedCommand(cmd.c_str(), ans);

    CSegment*  pSeg = new CSegment;
    *pSeg << ans;
	ResponedClientRequest(stat,pSeg);
}

/***********************************************************************/
/* VNGR-22579 14.9.11 Rachel Cohen                                     */
/* at startup MCMS check if CS ip has been config per service          */
/***********************************************************************/
void CConfiguratorManager::CheckCSIpConfig(CSegment* pSeg)
{
	STATUS stat=STATUS_OK;
	char ip_str[IP_ADDRESS_LEN];
	DWORD CSIpAddress;
	if (IsTarget())
	{
		TEST_ROOT_AND_RETURN
	}
	else
	{
	  return;
	}

	*pSeg >> CSIpAddress;

	SystemDWORDToIpString(CSIpAddress, ip_str);
	TRACEINTO << "CS IP " << ip_str;

	CSegment*  ret_seg = new CSegment;
	std::string ans;
	std::string cmd;

	SystemDWORDToIpString(CSIpAddress,	ip_str);

	ans="";
	cmd = GetCmdLinePrefix() + "/sbin/ifconfig | grep " ;	
	cmd = cmd + ip_str;

	stat = SystemPipedCommand(cmd.c_str(), ans);
	if(ans == "")
	{
		TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
			"CConfiguratorManager::CheckCSIpConfig ans in empty "  ;
		CSIpAddress = 0;
	}

	*ret_seg << (DWORD)(CSIpAddress);

	ResponedClientRequest(stat,ret_seg);
}
void CConfiguratorManager::ConfigBondingInterface(CSegment* pSeg)
{
	TEST_ROOT_AND_RETURN
    std::string answer;
    std::string bondingMode;
    DWORD linkmonitoringfrequency;
    BOOL  bIsBondingEnabled = TRUE;
    DWORD tmpValue;
	*pSeg  >>  linkmonitoringfrequency
		   >>  tmpValue
		   >>  bondingMode ;
	bIsBondingEnabled = (BOOL)tmpValue;
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	TRACESTR(eLevelInfoNormal) << "CConfiguratorManager::ConfigBondingInterfaceInterface() bonding mode=" << bondingMode <<" miimion:"<<linkmonitoringfrequency<< " IsBonding Enabled = " << (YES == bIsBondingEnabled ? "YES" : "NO") <<" Product:"<< ProductTypeToString(curProductType);
	//std::string bondingCmd = "/sbin/modprobe -r bonding";
	std::string bondingCmd;
	if (bIsBondingEnabled==FALSE)//remove bonding interface and return
	{
		//SystemPipedCommand(bondingCmd.c_str() ,answer);
		return;
	}
	SystemPipedCommand(bondingCmd.c_str() ,answer);
	char strlinkFreq[32];
	sprintf(strlinkFreq,"%d",linkmonitoringfrequency);
	bondingCmd = "/sbin/modprobe bonding mode=" + bondingMode + " miimon=" + strlinkFreq;
	SystemPipedCommand(bondingCmd.c_str() ,answer);

	bondingCmd = "/sbin/ifconfig bond0 up";
	SystemPipedCommand(bondingCmd.c_str() ,answer);
}


void CConfiguratorManager::SetBondingInterfaceSlaves(CSegment* pSeg)
{
	TEST_ROOT_AND_RETURN
    std::string answer;

	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	TRACESTR(eLevelInfoNormal) << "CConfiguratorManager::SetSlaveBondingInterfaces() Product:"<<ProductTypeToString(curProductType);

	std::string bondingCmd = "/sbin/ifenslave bond0 eth1";
	SystemPipedCommand(bondingCmd.c_str() ,answer);
	bondingCmd = "/sbin/ifenslave bond0 eth2";
	SystemPipedCommand(bondingCmd.c_str() ,answer);

}
void CConfiguratorManager::GetTempFromAdvantechUtil(CSegment* seg)
{
    if (IsTarget())
    {
        TEST_ROOT_AND_RETURN
    }

    std::string cmd =  MCU_MCMS_DIR+"/Bin/portwell-cpu-temperature";
    std::string ans;
    STATUS stat = SystemPipedCommand(cmd.c_str(), ans);

    CSegment*  pSeg = new CSegment;
    *pSeg << ans;
	ResponedClientRequest(stat,pSeg);
}

STATUS       CConfiguratorManager::InitalizeIpTablesWhiteList()
{
	STATUS stat=STATUS_OK;
	std::string ans,cmd,cmdWhiteListChain,cmdClearWhiteList;
	int iptypes[] = {SEG_IPV4,SEG_IPV6};
	const char* createCmds[] ={"iptables -N whiteList","ip6tables -N whiteList"};
	const char* cmdAddWhiteListChain[] ={"iptables -I INPUT -j whiteList",
										   "ip6tables -I INPUT -j whiteList"};
		
	const char* cmdClearWhiteListChain[] ={"iptables -F whiteList","ip6tables -F whiteList"};
	for(int i=0;i < (int)(ARRAYSIZE(iptypes));i++)
		{
			if(!CheckIfWhiteListExist(iptypes[i]))
			{
				//create whiteList chain
				cmd = GetCmdLinePrefix() + createCmds[i];
				stat = SystemPipedCommand(cmd.c_str(), ans);
				TRACESTR(eLevelInfoNormal) <<
									"CConfiguratorManager::EnableWhiteList :" << cmd << std::endl << "answer:" << ans << ", stat:" << stat;
				cmdWhiteListChain = GetCmdLinePrefix() + cmdAddWhiteListChain[i];
				stat = SystemPipedCommand(cmdWhiteListChain.c_str(), ans);
							TRACESTR(eLevelInfoNormal) <<
												"CConfiguratorManager::EnableWhiteList :" << cmdWhiteListChain << std::endl << "answer:" << ans << ", stat:" << stat;
			}
			else
			{
				//clean previous chain list
				cmdClearWhiteList = GetCmdLinePrefix() + cmdClearWhiteListChain[i];
				stat = SystemPipedCommand(cmdClearWhiteList.c_str(), ans);
				TRACESTR(eLevelInfoNormal) <<"CConfiguratorManager::EnableWhiteList :" << cmdClearWhiteList
	                                     << std::endl << "answer:" << ans << ", stat:" << stat;
				
			}
		}
	
	return stat;
}
void CConfiguratorManager::EnableWhiteList(CSegment* seg)
{
	eProductType curProductType = m_productTypeDecider->GetProductType();
	
	if (IsTarget() || curProductType == eProductTypeGesher || curProductType == eProductTypeNinja)
	{
	 TEST_ROOT_AND_RETURN
	}
	else
	{
	 ResponedClientRequest(STATUS_OK);
	 return;
	}

	STATUS stat=STATUS_OK;
	
	DWORD iptype,listSize=0;

	stat = InitalizeIpTablesWhiteList();
	if(STATUS_OK != stat)
	{
		ResponedClientRequest(stat);
	}
	int countFailure = 0;
	int countSuccess = 0;
	while(!seg->EndOfSegment())
	{
		*seg >>iptype ;
		*seg >>listSize ;
		for(DWORD i=0;i< listSize;i++)
		{
			std::string ip;
			*seg >> ip;
			if(!AddIpToWhiteList(ip,iptype))
				countFailure++;
			else
				countSuccess++;
		}
	}
		
	if((countFailure > 0) && (countSuccess ==0))
		stat = STATUS_FAIL;
	else
	{
		if(countSuccess >0)
		{
			stat =FinalizeIpTablesWhiteList();
		}
	}
	 ResponedClientRequest(stat);
}
STATUS CConfiguratorManager::FinalizeIpTablesWhiteList()
{
	STATUS stat=STATUS_OK;
	std::string ans,cmd;
	int iptypes[] = {SEG_IPV4,SEG_IPV6};
		
	const char* cmdBlocks[] ={"iptables -A whiteList -p tcp -m multiport --dports 80,443 -j DROP",
							   "ip6tables -A whiteList -p tcp -m multiport --dports 80,443 -j DROP"
							 };
	// block all other ip's				
	for(int i=0;i < (int)(ARRAYSIZE(iptypes));i++)
	{
		if(CheckIfWhiteListExist(iptypes[i]))
		{
			cmd = GetCmdLinePrefix() + cmdBlocks[i];
			stat = SystemPipedCommand(cmd.c_str(), ans);
			TRACESTR(eLevelInfoNormal) <<
					"CConfiguratorManager::EnableWhiteList :" << cmd << std::endl << "answer:" << ans << ", stat:" << stat;
		}
	}	
	return stat;
}
bool CConfiguratorManager::AddIpToWhiteList(std::string& sIp,DWORD& iptype)
{
	
	STATUS stat=STATUS_OK;
	std::string cmd,ans,cmdStart ;		
	std::string cmdEnd   = " -p tcp -m multiport --dports 80,443 -j ACCEPT";
	
	if(SEG_IPV4 == iptype)
	{
		cmdStart = GetCmdLinePrefix() +  "iptables -I whiteList -s ";
		switch(CheckIpv4Range(sIp))
		{
			case NO_Range:  cmd = 	cmdStart +sIp+cmdEnd;
							break;
			case IP24_Range: cmd = 	cmdStart +sIp+"/24" +cmdEnd;
							break;
			case IP16_Range: cmd =  cmdStart +sIp+"/16" +cmdEnd;		
							break;
			case Invalid_Range:  return false;
								
		}
	}
	else
	{
	 	cmdStart = "ip6tables -I whiteList -s ";
	 	cmd = GetCmdLinePrefix() + cmdStart +sIp+cmdEnd;
	}		
	stat = SystemPipedCommand(cmd.c_str(), ans);
	if(STATUS_OK != stat)
		return false;
	else
		return true;
}
IPRangeState CConfiguratorManager::CheckIpv4Range(std::string& sIp)
{
	int rangeCount = 0;
	size_t found;
	IPRangeState IpState =NO_Range;
	while(((found = sIp.find('*')) != string::npos))	
	{
		// a range was found replace it with 0;
		sIp.replace(found,1,"0");
		rangeCount++;
	}
	if(rangeCount > (int)IP16_Range)
		return Invalid_Range;
	IpState = (IPRangeState)rangeCount;
	return IpState;
}
void CConfiguratorManager::DisableWhiteList(CSegment* seg)
{
	STATUS stat=STATUS_OK;
	eProductType curProductType = m_productTypeDecider->GetProductType();

	if (IsTarget() || curProductType == eProductTypeGesher || curProductType == eProductTypeNinja)
	{
		 TEST_ROOT_AND_RETURN
	}
	else
	{
		ResponedClientRequest(STATUS_OK);
		return;
	}
	
	std::string ans,req;
	const char* cmds[] ={"iptables -F whiteList","ip6tables -F whiteList"};	
	int iptypes[] = {SEG_IPV4,SEG_IPV6};	
	for(int i=0;i < (int)(ARRAYSIZE(iptypes));i++)
	{
		if(CheckIfWhiteListExist(iptypes[i]))
		{
			req = GetCmdLinePrefix() + cmds[i];
			stat = SystemPipedCommand(req.c_str()  , ans);
			TRACESTR(eLevelInfoNormal) <<
						"CConfiguratorManager::DisableWhiteList :" << GetCmdLinePrefix() + cmds[i] << std::endl << "answer:" << ans << ", stat:" << stat;
	
		}
	}
	ResponedClientRequest(stat);
}

bool CConfiguratorManager::CheckIfWhiteListExist(int& iptype)
{
	bool res = false;
	std::string cmd;
	
	if( SEG_IPV4 == iptype )
	{
		cmd =GetCmdLinePrefix() + "iptables --list|grep whiteList";
	}
	else
	{
		cmd =GetCmdLinePrefix() + "ip6tables --list|grep whiteList";
	}
	std::string ans;
	STATUS stat = SystemPipedCommand(cmd.c_str(), ans);
	
	TRACESTR(eLevelInfoNormal) <<
	        "CConfiguratorManager::CheckIfWhiteListExist :" << cmd << std::endl << "answer:" << ans << ", stat:" << stat;
	
	res =  !ans.empty();			
	return res; 
}

void CConfiguratorManager::OnConfigNetworkSeperationIpv6Timeout(CSegment* seg)
{

	STATUS stat=STATUS_OK;
	if (IsTarget())
	{
		 TEST_ROOT_AND_RETURN
	}
	else
	{
		ResponedClientRequest(STATUS_OK);
		return;
	}
    std::string	stNicName;
    std::string route_table;

    *seg >>  route_table
    	>> stNicName;

	TRACESTR(eLevelInfoNormal) << "CConfiguratorManager::OnConfigNetworkSeperationIpv6Timeout : route table = " << route_table << " nicname = " << stNicName << std::endl;

	ConfigureIpv6NetSeperationRoutingTables(route_table,stNicName);

	ResponedClientRequest(stat);
}

STATUS CConfiguratorManager::ConfigureIpv6NetSeperationRoutingTables(std::string route_table,std::string stNicName)
{
	STATUS stat=STATUS_OK;
	std::string ans,tmpAnswer;
	std::string ifconfig_cmd,ipruleadd_cmd,iprourte_add_cmd,route_cmd;
	ifconfig_cmd = GetCmdLinePrefix() + "ifconfig " + stNicName + " | grep inet6 | awk '{ print $3 }'" ;
	stat = SystemPipedCommand(ifconfig_cmd.c_str(), ans);

   char split_char = '\n';
   std::istringstream split1(ans);
   std::vector<std::string> tokens1,tokens2;
   std::string ipaddress;
   std::string::size_type start_pos;
   for (std::string each; std::getline(split1, each, split_char); tokens1.push_back(each))
   {
	   start_pos = each.find("/64");
	   ipaddress = each.replace(start_pos, 3, "");
	   ipruleadd_cmd = GetCmdLinePrefix() + "ip -6 rule add from " + each + " lookup " + route_table;
	   stat = SystemPipedCommand(ipruleadd_cmd.c_str(), tmpAnswer);
   }


	ifconfig_cmd = GetCmdLinePrefix() + "ip -6 route | grep " + stNicName + " | awk '{print $1}'" ;
	stat = SystemPipedCommand(ifconfig_cmd.c_str(), ans);
	std::istringstream split2(ans);
	for (std::string each; std::getline(split2, each, split_char); tokens2.push_back(each))
	{
		if (each == "default")
			continue;
		iprourte_add_cmd = GetCmdLinePrefix() + "ip -6 route add table " + route_table + " unicast " + each + " dev " + stNicName;
		stat = SystemPipedCommand(iprourte_add_cmd.c_str(), tmpAnswer);
	}
	route_cmd = GetCmdLinePrefix() +  "echo -n `ip -6 route | grep " + stNicName + " | awk '/default/ { print $3 }'`";
	stat = SystemPipedCommand(route_cmd.c_str(), ans);
	route_cmd = GetCmdLinePrefix() +  "ip -6 route add table " + route_table + " via " + ans + " dev " + stNicName;
	stat = SystemPipedCommand(route_cmd.c_str(), ans);

	return stat;
}


void CConfiguratorManager::AddIpAndNatRuleForDnsPerService(CSegment* seg)
{
	 std::string	ipDnsAddress;
	 std::string	route_table;
	 std::string	ipAddress;
	 DWORD	priority;
	 DWORD	isIpv4;

	 *seg >> ipDnsAddress;
	 *seg >> route_table;
	 *seg >> priority;
	 *seg >> isIpv4;
	 *seg >> ipAddress;

	 std::ostringstream cmd;
	 //iptables -t nat -A POSTROUTING -d <DNS> -p udp --dport 53 -j SNAT --to <CS>";
	 //"iptables -t nat -A POSTROUTING -d 10.226.26.127 -p udp --dport 53 -j SNAT --to 172.22.185.160";
	 cmd  << "iptables -t nat -A POSTROUTING -d " <<  ipDnsAddress << " -p udp --dport 53 -j SNAT --to " << ipAddress;
	 std::string ans;
	 STATUS stat;
	 TRACEINTO << cmd;
	 stat = SystemPipedCommand(cmd.str().c_str(), ans);

	 std::ostringstream cmd2;
	 //example: ip rule add to 10.227.1.37 lookup 3 prio 10000


	 if(isIpV4Str(ipDnsAddress.c_str()))
	 {
		 cmd2  << "ip rule add to " <<  ipDnsAddress << " lookup "<< route_table << " prio " << priority;
	 }
	 else
	 {
		 //ip -6 rule add fwmark 0xfab lookup 0xf

		 cmd2 << "ip -6 rule add to " <<  ipDnsAddress << " lookup "<< route_table << " prio "<< priority;
	 }

	 std::string ans2;
	 TRACEINTO << cmd2;
	 STATUS stat2;
	 stat2 = SystemPipedCommand(cmd2.str().c_str(), ans2);

}

void  CConfiguratorManager::AddNatRule(CSegment* seg)
{
	std::string	ipDnsAddress;
	std::string	ipAddress;
	std::string	ports;
	std::string protocol;

	DWORD	isIpv4;
	DWORD	isMultiPorts;

	*seg >> ipDnsAddress;
	*seg >> ipAddress;
	*seg >> ports;
	*seg >> isIpv4;
	*seg >> isMultiPorts;
    *seg >> protocol;
	STATUS stat = STATUS_OK;

	std::ostringstream ipNatRule;
	std::ostringstream ipNatRuleDelete;
	ipNatRule  << GetCmdLinePrefix();
	ipNatRuleDelete << GetCmdLinePrefix();
	if(isIpv4)
	{
		ipNatRule  << "iptables -t nat -A POSTROUTING -d ";
		ipNatRuleDelete << "iptables -t nat -D POSTROUTING -d ";
	}
	else
	{
		ipNatRule  << "ip6tables -t nat -A POSTROUTING -d ";
		ipNatRuleDelete << "ip6tables -t nat -D POSTROUTING -d ";
	}
	ipNatRule  << ipDnsAddress;
	ipNatRuleDelete << ipDnsAddress;
	if(isMultiPorts)
	{

		ipNatRule  << " -p "<< protocol <<" -m multiport --dport " <<  ports;
		ipNatRuleDelete << " -p "<< protocol <<" -m multiport --dport " <<  ports;
	}
	else
	{
		ipNatRule << " -p "<< protocol <<" --dport " <<  ports;
		ipNatRuleDelete << " -p "<< protocol <<" --dport " <<  ports;
	}

	ipNatRule  <<" -j SNAT --to " << ipAddress;
	ipNatRuleDelete <<" -j SNAT --to " << ipAddress;
    std::string ans;
	//TRACEINTO << ipNatRule.str();
	TEST_ROOT_AND_RETURN
	stat = SystemPipedCommand(ipNatRule.str().c_str(), ans);

	CSegment*  pSeg = new CSegment;
	*pSeg << ipNatRuleDelete.str();
	ResponedClientRequest(stat,pSeg);
}
void  CConfiguratorManager::DeleteNatRule(CSegment* seg)
{
	std::string ipNatRule;
	std::string ans;
	*seg >> ipNatRule;
	//TRACEINTO << ipNatRule;

	TEST_ROOT_AND_RETURN

	SystemPipedCommand(ipNatRule.c_str(), ans);

	//TRACEINTO << ipNatRule << ", " << ans;
}

void CConfiguratorManager::FPGAUpgradeProgress(eFPGAUpgradeAction action, STATUS stat, unsigned int percent)
{
		CManagerApi api(eProcessInstaller);
		CSegment*  pMsg = new CSegment;
		*pMsg << (DWORD) action;
		*pMsg << (DWORD) stat;
		*pMsg << (DWORD) percent;
		api.SendMsg(pMsg, FPGA_UPGRADE_PROGRESS);
}

void CConfiguratorManager::FPGAUpgrade(CSegment* pSeg)
{
	STATUS ret = STATUS_FAIL;
	string szFPGAImgPath, szFPGAImgMD5, answerTmp, cmd, sum2;
	eFPGAUpgradeAction currentStep = eFPGAErase, lastStep = eFPGAErase;
	unsigned int percent = 0, lastPercent = 0;
	string readback = MCU_TMP_DIR+"/fpga_image_read_back_file.bin";

	*pSeg >> szFPGAImgPath;

	// remove '\n' if have.
	if((szFPGAImgPath.length() > 0) && (szFPGAImgPath.at(szFPGAImgPath.length() - 1) == '\n')) szFPGAImgPath.erase(szFPGAImgPath.length() - 1);

	if(IsFileExists(szFPGAImgPath))
	{
		//compare FPGA Image file's version with current version
		if(STATUS_OK != FPGAImageHeaderCompare(szFPGAImgPath))
		{
            // Allows to run other tasks during the operation
            CTaskApp::Unlocker unlocker(TRUE);

            //upgrade
            cmd = "sudo "+MCU_MRMX_DIR+"/bin/fpga_upgrade/fpga_upgrade " + szFPGAImgPath + " /dev/mtd0 " + readback;
            FILE* fpipe = popen(cmd.c_str(), "r");
            if(NULL == fpipe)
            {
                TRACESTR(eLevelInfoNormal) <<
                "CConfiguratorManager::FPGAUpgrade - popen failed: "<< cmd;
                ret = STATUS_FAIL;
                goto done;
            }


            char line[256] = {0};
            while (fgets(line, ARRAYSIZE(line), fpipe))
            {
                if (STATUS_OK != ParseFPGAUpgradeLine(line, currentStep, percent))
                {
                    continue;
                }

                TRACESTR(eLevelInfoNormal) << 
                "CConfiguratorManager::FPGAUpgrade - ParseFPGAUpgradeLine: currentStep:" << currentStep << "  percent:"<< percent;

                if(currentStep > lastStep || 100 == percent || percent > lastPercent + 10)
                {
                   FPGAUpgradeProgress(currentStep, STATUS_OK, percent);
                   lastPercent = percent;
                }

                lastStep = currentStep;
            }

            int rc = pclose(fpipe);
            TRACESTR(eLevelInfoNormal) << 
                "CConfiguratorManager::FPGAUpgrade - pclose: " << cmd << "  rc:"<< rc <<" errno:"<< strerror(errno) << " (" << errno << ")" << " last line:"<< line;

			//upgrade check
			cmd = "sudo /usr/bin/md5sum " + szFPGAImgPath + " | awk '{print $1}'";
			ret = SystemPipedCommand(cmd.c_str(), szFPGAImgMD5);
			TRACESTR(eLevelInfoNormal) <<
					"CConfiguratorManager::FPGAUpgrade - "<< cmd <<" : " << szFPGAImgMD5;
			if(STATUS_FAIL == ret) goto done;

			if((szFPGAImgMD5.length() > 0) && (szFPGAImgMD5.at(szFPGAImgMD5.length() - 1) == '\n')) szFPGAImgMD5.erase(szFPGAImgMD5.length() - 1);

			cmd = "sudo /usr/bin/md5sum " + readback + " | awk '{print $1}'";
			ret = SystemPipedCommand(cmd.c_str(), sum2);
			TRACESTR(eLevelInfoNormal) <<
					"CConfiguratorManager::FPGAUpgrade - "<< cmd <<" : " << sum2;
			if(STATUS_FAIL == ret) goto done;

			if((sum2.length() > 0) && (sum2.at(sum2.length() - 1) == '\n')) sum2.erase(sum2.length() - 1);
            
			if(0 != szFPGAImgMD5.compare(sum2))
			{
				TRACESTR(eLevelInfoNormal) <<
					"CConfiguratorManager::FPGAUpgrade - Upgrade Image MD5: "<< szFPGAImgMD5 <<"; is not equal to read back MD5: " << sum2 <<";";
				ret = STATUS_FAIL;
				goto done;
			}

			TRACESTR(eLevelInfoNormal) <<
				"CConfiguratorManager::FPGAUpgrade - upgrade OK.";

			// save read back MD5 to MCU_CONFIG_DIR/sysinfo/fpga_upgrade_md5
			cmd = "sudo echo -n '" + sum2 + "' > "+MCU_CONFIG_DIR+"/sysinfo/fpga_upgrade_md5";
			ret = SystemPipedCommand(cmd.c_str(), answerTmp);
			TRACESTR(eLevelInfoNormal) <<
					"CConfiguratorManager::FPGAUpgradeImgCompare - "<< cmd << " return :"<< ret <<" : " << answerTmp;
		}

		ret = STATUS_OK;
		currentStep = eFPGAActionMaxNum;
	}
	else
	{
		TRACESTR(eLevelInfoNormal) <<
			"CConfiguratorManager::FPGAUpgrade - FPGA Image file is not existed. Upgrade failed.";		
	}
	
done:
	FPGAUpgradeProgress(currentStep, ret, 100);
	return;
}

namespace
{ 
#define FPGA_UPGRADE_STEP_ERASE       "fpga erase:"
#define FPGA_UPGRADE_STEP_WRITE       "fpga write:"
#define FPGA_UPGRADE_STEP_READ        "fpga read:"
#define FPGA_FORCE_UPGRADE_FLG        (MCU_MCMS_DIR+"/States/forceFPGAUpgrade.flg")
}

STATUS CConfiguratorManager::ParseFPGAUpgradeLine(const char * line, eFPGAUpgradeAction & currentStep, unsigned int & percent)
{
    std::size_t found, foundPercent;
    std::string szLine(line);
    
    TRACESTR(eLevelInfoNormal) <<
        "CConfiguratorManager::ParseFPGAUpgradeLine :" << line;


    if((found = szLine.find(FPGA_UPGRADE_STEP_ERASE)) != std::string::npos)
    {
        currentStep = eFPGAErase;
    }
    else if((found = szLine.find(FPGA_UPGRADE_STEP_WRITE)) != std::string::npos)
    {
        currentStep = eFPGAUpgrade;
    }
    else if((found = szLine.find(FPGA_UPGRADE_STEP_READ)) != std::string::npos)
    {
        currentStep = eFPGAReadBack;
    }
    else
    {
        return STATUS_FAIL;
    }

    if((foundPercent = szLine.find_last_of("%")) == std::string::npos)
    {
        return STATUS_FAIL;
    }

    if((found = szLine.find_last_of(" ", foundPercent)) == std::string::npos)
    {
        return STATUS_FAIL;
    }

    int len = foundPercent - found - 1;
    if(len <= 0 || len > 5 )
    {
        TRACESTR(eLevelInfoNormal) <<
            "CConfiguratorManager::ParseFPGAUpgradeLine : error len = %d" << len;      
        return STATUS_FAIL;
    }

    std::string szPercent = szLine.substr(found+1, len);
    percent = ((int)atof(szPercent.c_str()));
    
    return STATUS_OK;
}

STATUS CConfiguratorManager::FPGAImageHeaderCompare(std::string szFPGAImgPath)
{
    std::string cmd = "sudo "+MCU_MCMS_DIR+"/Scripts/NinjaFPGACompare.sh " + szFPGAImgPath;
    std::string answer;
    std::string upgradeFlagPath = FPGA_FORCE_UPGRADE_FLG;

    if(IsFileExists(upgradeFlagPath.c_str()))
    {
        return STATUS_FAIL;
    }
    
    STATUS stat = SystemPipedCommand(cmd.c_str(),answer);
    TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
        "CConfiguratorManager::FPGAImageHeaderCompare :" << cmd << std::endl << "answer:" << answer << ", stat:" << stat;

    if (STATUS_OK == stat && "YES" == answer)
    {
        stat = STATUS_OK;
    }
    else
    {
        stat = STATUS_FAIL;
    }

    TRACESTR(eLevelInfoNormal) <<
		"CConfiguratorManager::FPGAImageHeaderCompare - compare result: "<< stat;
    
    return stat;
}

/*Begin:added by Richer for BRIDGE-15015, 11/13/2014*/
void CConfiguratorManager::OnLEDSysAlarmIndication(CSegment* pMsg)
{
    DWORD dLedIndi = 0;
    string led_cmd;
    std::string answer;
           
    memcpy(&dLedIndi, pMsg->GetPtr(), sizeof(DWORD));

    if (1 == dLedIndi)
    {
        led_cmd = GetCmdLinePrefix() + MCU_MCMS_DIR + "/Bin/LightCli sysAlarm del_ep";        
    }
    else
    {
        led_cmd= GetCmdLinePrefix() + MCU_MCMS_DIR + "/Bin/LightCli sysAlarm add_ep";    
    }

    SystemPipedCommand(led_cmd.c_str(),answer);
    
} 

/*End:added by Richer for BRIDGE-15015, 11/13/2014*/


void  CConfiguratorManager::GetUdpOccupiedPorts(CSegment* seg)
{
    std::string ans,if_name;
    std::stringstream cmd_buf;
    STATUS stat = STATUS_OK;

    *seg  >>  if_name;

    cmd_buf << GetCmdLinePrefix() << "/mcms/Scripts/GetUdpOccupiedPorts.sh " << if_name;
    stat = SystemPipedCommand(cmd_buf.str().c_str(), ans);
    TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
            "CConfiguratorManager::GetUdpOccupiedPorts :" << cmd_buf.str() << std::endl << "answer:" << ans << ", stat:" << stat;

    ResponedClientRequest(stat);
}
