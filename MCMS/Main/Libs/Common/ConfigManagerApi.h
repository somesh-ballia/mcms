// ConfigManagerApi.h

#ifndef CONFIG_MANAGER_API_H_
#define CONFIG_MANAGER_API_H_

#include <list>
#include "ManagerApi.h"
#include "CommonStructs.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "DefinesGeneral.h"
#include <sstream>


static BOOL IsBondingEnabled()
{
	static BOOL isFileAlreadyRead = FALSE;
	static BOOL isBondingEnabled = FALSE;
	if (isFileAlreadyRead==TRUE)
		return isBondingEnabled;


	std::string file = MCU_MCMS_DIR+"/ENABLE_BONDING.txt";

	char szFileLine[5] = "";

	FILE* pFile = fopen(file.c_str(), "r");
	if(pFile)
	{
		fgets(szFileLine,4,pFile);
        fclose(pFile);

	}

	if(!strncmp(szFileLine, "YES", 3))
	{
		isBondingEnabled = TRUE;
	}

	isFileAlreadyRead = TRUE;

	return isBondingEnabled;

}

static  eConfigInterfaceNum GetInterfaceNum(const eConfigInterfaceType ifType,
												  eIpType ipType)
{
	eConfigInterfaceNum retIfNum = eEth0_alias_1;	//Call Generator has only this one interface

	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();


	if ( FALSE == IsTarget() && eProductTypeGesher != curProductType && eProductTypeNinja != curProductType)
	   return eEth0;
	
	BOOL bMULTIPLE_SERVICES = NO;
	CSysConfig* cfg = CProcessBase::GetProcess()->GetSysConfig();
	cfg->GetBOOLDataByKey(CFG_KEY_MULTIPLE_SERVICES, bMULTIPLE_SERVICES);
		
	BOOL bLAN_REDUNDANCY = CFG_VALUE_LANREDUNDANCY_DEFAULT;

	bLAN_REDUNDANCY = IsBondingEnabled();

	

	if (eProductTypeRMX2000 == curProductType &&(( eIpType_IpV4 == ipType) ||( eIpType_Both == ipType)))
	{
		switch(ifType)
		{
			case(eSeparatedManagmentNetwork):
				retIfNum = eEth0_2197;
				break;
			case(eInternalNetwork):
				retIfNum = eEth0_2093;
				break;
			case(eSignalingNetwork):
				retIfNum = eEth0_alias_2;
				break;
			case(eSeparatedSignalingNetwork):
				retIfNum = eEth0_2198;
				break;
			case(eSeparatedSignalingNetwork_1_1):
				retIfNum = eEth0_2012;
				break;
			case(eSeparatedSignalingNetwork_1_2):
				retIfNum = eEth0_2013;
				break;
			case(eSeparatedSignalingNetwork_2_1):
				retIfNum = eEth0_2022;
				break;
			case(eSeparatedSignalingNetwork_2_2):
				retIfNum = eEth0_2023;
				break;
			case(ePermanentNetwork):
				retIfNum = eEth0_2097;
				break;
			case(eManagmentNetwork):
			default:
				retIfNum = eEth0_alias_1;
		}
	}
	else if (eProductTypeRMX2000 == curProductType && eIpType_IpV6 == ipType)
	{
		switch(ifType)
		{
			case(eSeparatedManagmentNetwork):
				retIfNum = eEth0_2197;
				break;
			case(eInternalNetwork):
				retIfNum = eEth0_2093;
				break;
			case(eSignalingNetwork):
				retIfNum = eEth0;
				break;
			case(eSeparatedSignalingNetwork):
				retIfNum = eEth0_2198;
				break;
			case(ePermanentNetwork):
				retIfNum = eEth0_2097;
				break;
			case(eManagmentNetwork):
			default:
				retIfNum = eEth0;
		}
	}
	else if (eProductTypeRMX4000 == curProductType)
	{
		switch(ifType)
		{
			case(eManagmentNetwork):
				if (bLAN_REDUNDANCY==YES)
					retIfNum = eBond0_alias_1;
				else
					retIfNum = eEth1;
				break;
			case(eInternalNetwork):
				retIfNum = eEth0_2093;
				break;
			case(eSignalingNetwork):
				if (bLAN_REDUNDANCY==YES)
					retIfNum = eBond0_alias_2;
				else
				retIfNum = eEth2;
				break;
			case(eSeparatedSignalingNetwork_1_1):
				retIfNum = eEth0_2012;
				break;
			case(eSeparatedSignalingNetwork_1_2):
				retIfNum = eEth0_2013;
				break;
			case(eSeparatedSignalingNetwork_2_1):
				retIfNum = eEth0_2022;
				break;
			case(eSeparatedSignalingNetwork_2_2):
				retIfNum = eEth0_2023;
				break;
			case(eSeparatedSignalingNetwork_3_1):
				retIfNum = eEth0_2032;
				break;
			case(eSeparatedSignalingNetwork_3_2):
				retIfNum = eEth0_2033;
				break;
			case(eSeparatedSignalingNetwork_4_1):
				retIfNum = eEth0_2042;
				break;
			case(eSeparatedSignalingNetwork_4_2):
				retIfNum = eEth0_2043;
				break;
			case(ePermanentNetwork):
				retIfNum = eEth0_2097;
				break;
			default:
				retIfNum = eEth1;
		}
	}
	else if(eProductTypeRMX1500 == curProductType)
	{
		switch(ifType)
		{
			case(eManagmentNetwork):
				if (bLAN_REDUNDANCY==YES)
				{
					retIfNum = eBond0_alias_1;
				}
				else
					retIfNum = eEth1;
				break;
			case(eInternalNetwork):
				retIfNum = eEth0_2093;
				break;
			case(eSignalingNetwork):
				if (bLAN_REDUNDANCY==YES)
					retIfNum = eBond0_alias_2;
				else
				retIfNum = eEth2;
				break;
			case(eSeparatedSignalingNetwork_1_1):
				retIfNum = eEth0_2012;
				break;
			case(eSeparatedSignalingNetwork_1_2):
				retIfNum = eEth0_2013;
				break;
			case(ePermanentNetwork):
				retIfNum = eEth0_2097;
				break;
			default:
				retIfNum = eEth1;
		}
	}
	//added for ip setting
	else if (eProductTypeGesher == curProductType)
	{
		switch(ifType)
		{
			case(eManagmentNetwork):
				retIfNum = eEth0;
				break;
			case(eSignalingNetwork):
				retIfNum = eEth1;
				break;
			case(eLanRedundancyManagementNetwork):
				retIfNum = eEth0;
				break;
			case(eLanRedundancySignalingNetwork):
				if (bLAN_REDUNDANCY == NO)
				{
					retIfNum = eEth1;
				}
				else
				{
					retIfNum = eBond0_alias_1;
				}
				break;
			case(eMultipleSignalingNetwork_1):
				retIfNum = eEth1;
				break;
			case(eMultipleSignalingNetwork_2):
				retIfNum = eEth2;
				break;
			case(eMultipleSignalingNetwork_3):
				retIfNum = eEth3;
				break;
			default:
				retIfNum = eEth0;
		}
	}
	else if (eProductTypeNinja == curProductType)
	{
		switch(ifType)
		{
			case(eManagmentNetwork):
				if (bMULTIPLE_SERVICES == YES)
				{
					retIfNum = eEth0_alias_1;
				}
				else if (bLAN_REDUNDANCY == YES)
				{				
					retIfNum = eBond0_alias_1;
				}
				else
				{
					retIfNum = eEth0;
				}
				break;
			case(eSignalingNetwork):
				retIfNum = eEth1;
				break;
			case(eLanRedundancyManagementNetwork):
				if (bLAN_REDUNDANCY == NO)
				{
					retIfNum = eEth0;
				}
				else
				{				
					retIfNum = eBond0_alias_1;
				}
				break;
			case(eLanRedundancySignalingNetwork):
				if (bLAN_REDUNDANCY == NO)
				{
					retIfNum = eEth1;
				}
				else
				{
					retIfNum = eBond0_alias_2;
				}
				break;
			case(eMultipleSignalingNetwork_1):
				retIfNum = eEth0_alias_2;
				break;
			case(eMultipleSignalingNetwork_2):
				retIfNum = eEth1;
				break;

			//added by Richer for 802.1x project om 2013.12.26 
			case(eSoftMcuEth0Network):
				retIfNum = eEth0;
				break;
			case(eSoftMcuEth1Network):
				retIfNum = eEth1;
				break;
				
			default:
				retIfNum = eEth0;
		}
	}
	//end
	return retIfNum;
}

static eConfigDeviceName GetInterfaceDeviceNum(const eConfigInterfaceType ifType)
{
	eConfigDeviceName retIfName = eEth_0;	//Call Generator has only this one interface

	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	if (eProductTypeRMX2000 == curProductType)
	{
		retIfName = eEth_0;
	}
	else if ((eProductTypeRMX4000 == curProductType) || (eProductTypeRMX1500 == curProductType) )
	{
		switch(ifType)
		{
			case(eManagmentNetwork):
				retIfName = eEth_1;
				break;
			case(eSignalingNetwork):
				retIfName = eEth_2;
				break;
			case(eInternalNetwork):
			case(ePermanentNetwork):
			default:
				retIfName = eEth_0;
		}
	}
	else if (eProductTypeGesher == curProductType)
	{
		switch (ifType)
		{
			case (eSoftMcuEth0Network):
				retIfName = eEth_0;
				break;
				
			case (eSoftMcuEth1Network):
				retIfName = eEth_1;
				break;

			case (eSoftMcuEth2Network):
				retIfName = eEth_2;
				break;

			case (eSoftMcuEth3Network):
				retIfName = eEth_3;
				break;
				
			default:
				retIfName = eEth_0;
		}
	}
	else if (eProductTypeNinja == curProductType)
	{
		switch (ifType)
		{
			case (eSoftMcuEth0Network):
				retIfName = eEth_0;
				break;
				
			case (eSoftMcuEth1Network):
				retIfName = eEth_1;
				break;
	
			default:
				retIfName = eEth_0;
		}
	}

	return retIfName;
}


//Returns the 'logical' interface name
static const char* GetLogicalInterfaceName(const eConfigInterfaceType ifType,
										   const eIpType ipType)
{
	const eConfigInterfaceNum ifNum = GetInterfaceNum(ifType, ipType);
	return GetConfigInterfaceNumName(ifNum);
}

//Returns the 'hardware' device name
static const char* GetDeviceName(const eConfigInterfaceType ifType)
{
	const eConfigDeviceName ifName = GetInterfaceDeviceNum(ifType);
	return GetConfigDeviceName(ifName);
}



static char* sPortSpeedType[] =
{
	"autoneg on",
	"speed 10 autoneg off duplex half",
	"speed 10 autoneg off duplex full",
	"speed 100 autoneg off duplex half",
	"speed 100 autoneg off duplex full",
	"speed 1000 autoneg off duplex half",
	"speed 1000 autoneg off duplex full"
};

static const char* ParsePortSpeed(ePortSpeedType portSpeed)
{
	const char *name = (0 <= portSpeed && portSpeed < NUM_OF_PORT_SPEED_TYPES
						?
						sPortSpeedType[portSpeed] : "Invalid");
	return name;
}

enum eRouteType {router_net,router_host};

struct CRouter
{
    eRouteType m_type;
    std::string m_targetIP;
    std::string m_subNetmask;
    std::string m_gateway;
};

static char* sRoutingTableName[] =
{
	"1",	//eEth0
	"2",	//eEth1,
	"3",	//eEth2,
	"1",	//eEth0_alias_1,
	"1",	//eEth0_alias_2,
	"1",	//eEth0_2093,
	"1",	//eEth0_2097,
	"2",	//eEth0_2197,
	"3",	//eEth0_2198,
	"12",	//eEth0_2012,
	"13",   //eEth0_2013,
	"22",	//eEth0_2022,
	"23",	//eEth0_2023,
	"32",	//eEth0_2032,
	"33",	//eEth0_2033,
	"42",	//eEth0_2042,
	"43",	//eEth0_2043,
	"4",    //eEth3
	"2" ,   //eBond0_alias_1
	"3"    //eBond0_alias_2
};

static const char* GetRouteTableName(eConfigInterfaceNum ifNum)
{
	const char *name = (0 <= ifNum && ifNum < NumOfConfigInterfaceNums
						?
						sRoutingTableName[ifNum] : "Invalid");
	return name;
}

static char* sIPv6RoutingTableName[] =
{
	"61",	//eEth0
	"62",	//eEth1,
	"63",	//eEth2,
	"61",	//eEth0_alias_1,
	"61",	//eEth0_alias_2,
	"61",	//eEth0_2093,
	"61",	//eEth0_2097,
	"62",	//eEth0_2197,
	"63",	//eEth0_2198,
	"72",	//eEth0_2012,
	"73",   //eEth0_2013,
	"82",	//eEth0_2022,
	"83",	//eEth0_2023,
	"92",	//eEth0_2032,
	"93",	//eEth0_2033,
	"102",	//eEth0_2042,
	"103",	//eEth0_2043,
	"64",    //eEth3
	"62" ,   //eBond0_alias_1
	"63"    //eBond0_alias_2
};

static const char* GetIPv6RouteTableName(eConfigInterfaceNum ifNum)
{
	int arraySize = sizeof(sIPv6RoutingTableName) / sizeof(sIPv6RoutingTableName [0]);
	const char *name = (0 <= ifNum && ifNum < arraySize && ifNum < NumOfConfigInterfaceNums
						?
								sIPv6RoutingTableName[ifNum] : "Invalid");
	return name;
}

class CConfigManagerApi : public CManagerApi
{
CLASS_TYPE_1(CConfigManagerApi, CManagerApi)
public:
	virtual const char* NameOf() const { return "CConfigManagerApi";}
	CConfigManagerApi();

	// Always returns false in release versions
	static bool IsSSHOnForDebug(void);

    STATUS ArpingRequest(const std::string & ip,eIpType ipType);	// IPv4 duplication checking
    STATUS DADRequest(const std::string & ip);		// IPv6 duplication checking
    STATUS FailoverArpingRequest(const std::string & ip,const std::string & defaultGwIp,DWORD ipType,std::string& answer);
    STATUS ConfigBonding(const std::string& bondingMode,DWORD linkmonitoringfrequency,BOOL bIsBondingEnabled);
    STATUS ConfigBondingInterfaceSlaves();
    STATUS AddIpInterface(eConfigInterfaceType eNetworkType,
    					  eIpType ipType,
						  const std::string & ipToCompare,
						  const std::string & ip,
                          const std::string & mask,
                          const std::string & gateway,
                          const std::string & broadcast,
                          const std::list<CRouter> &routerList);

    STATUS AddIpV6Interface(eConfigInterfaceType eNetworkType,
							BYTE  bIsAutoConfig,
							const std::string & ip = "",
							const std::string & mask = "",
							const std::string & gateway = "",
							const std::string & gwmask = "");


    STATUS AddVlan(eConfigInterfaceType eNetworkType,
                   const std::string & ip,
                   const std::string & mask,
                   const std::string & broadcast,
                   const DWORD vLanId);


    STATUS RemoveIpInterface(const std::string & ip);
    STATUS CycleVersions(const std::string & new_version_name);
    STATUS ConfigureDnsServers(const std::string & search,
                               const std::string & dns1,
                               const std::string & dns2,
                               const std::string & dns3);
    STATUS ConfigureMoreDnsServers(const char* search, const std::string & dns1);

    STATUS RunDHCP(BOOL autoDNS = TRUE);
    STATUS KillDHCP(eIpType type);
    STATUS GetLastDHCPConfig(const std::string& key,
                             std::string& value);

    STATUS GetFirstDNSFromDHCP(std::string& dnsServerIP);

    STATUS RegisterHostNameInDNS(const std::string& dnsIpAddress,
                                 const std::string& myHostName,
                                 const std::string& myzone,
                                 const std::string& myIpAddress,
                                 const std::string& myIpv6Address);

    STATUS GetNewVersionNumber(std::string & version);

    STATUS RemountVersionPartition(BOOL withWritePermission, 
				   BOOL isToDeleteFiles=FALSE,
				   BOOL isToDeleteFallbackInNeeded=FALSE,
				   WORD timeOutInSec=15);

    STATUS EnableDisablePing(BOOL enable);
    STATUS EnableDisablePingBroadcast(BOOL enable);
   // STATUS EnableDisablePingIptables(BOOL enable);
    STATUS AddAdminUser(std::string username , std::string password);
    STATUS DelAdminUser(std::string username);
    STATUS ChangePassword(std::string username , std::string new_password);

    STATUS RestartSnmpd();
    STATUS StartSnmpd();
    STATUS StopSnmpd();

    STATUS RestartSSHD(bool isPermanentNetwork, const std::string & ipV4 = "*", const std::string & ipV6 = "", bool isOn = TRUE);
    STATUS SyncTime();

    STATUS HandleNtpService(const ServiceCommand& cmd);
    STATUS ConfigNtpServers(const std::string& s1, const std::string& s2 = string(), const std::string& s3 = string());

    STATUS TestDMA(const std::string &ide_name);
    STATUS TestFlashDMA();
    STATUS TestHardDiskDMA();
    STATUS TestEthernetSettings(const std::string &eth_name);
    STATUS GetNtpPeerStatus(const std::string &ip_address);
    STATUS ConfigureEthernetSettings(const eConfigInterfaceType ifType, const ePortSpeedType portSpeed);
    STATUS EthernetSettingsMonitoring(ETH_SETTINGS_S &ethStruct, const eConfigInterfaceNum ifNum);
    STATUS GetSMARTErrors(std::string & errorReport);
    STATUS RunSMARTSelftest(BOOL short_test);
    STATUS TakeCoreFileOwnership(const std::string & core_name);
    STATUS ChangeMyNiceLevel(int new_nice_level /*-19 to 20*/, int myTaskPid);	// for Call Generator - change MM process priority
	STATUS DeleteFile(const std::string &file_name);
	STATUS DeleteDir(const std::string& dir_name);
	STATUS SetTCPStackParams(const DWORD keepalive_tout, const DWORD keepalive_intvl);
	STATUS InterfaceUp(const std::string nicName);
	STATUS RunCmd(const std::string cmd);
	STATUS AddDefaultGW(const std::string nicName, const std::string gateway, const BOOL isIpv6=FALSE);
	STATUS AddStaticIpRoutes(eConfigInterfaceType networkType, const std::list<CRouter> routerList);
	STATUS AddStaticRoutes(eConfigInterfaceType networkType, const std::list<CRouter> routerList);
	STATUS SetNICIpV6AutoConfig(const std::string nicName, const BOOL isIpv6Autoconfig,const eConfigInterfaceType ifType , const eIpType ipType,const std::string defaultGatewayIPv6Str);
	STATUS AddRouteTableRule(eConfigInterfaceType networkType,
			                 eIpType ipType,
							 const std::string ip,
							 const DWORD mask,
							 const std::string defaultGW,
							 const std::string ipv6SubNetMask,
							 const std::list<CRouter> routerList,
							 const BOOL bIPv6=FALSE);
	STATUS DelRouteTableRule(eConfigInterfaceType networkType,
			                 eIpType ipType,
							 const std::string ip,
							 const DWORD mask,
							 const std::string defaultGW,
							 const BOOL bIPv6=FALSE);
	STATUS DeleteTempFiles();
    STATUS SetProductType(const eProductType productType);
    STATUS EvokeNetworkInterfaces(eV6ConfigurationType ipconfigType);
    STATUS SetMacAddress(const eConfigInterfaceType ifNum,eIpType ipType, const std::string & macAddress);
    STATUS GetHDTemperature(std::string &temperature);
    STATUS GetHDSize(std::string &size);
    STATUS GetHDModel(std::string &details);
    STATUS GetHDFirmware(std::string &hd_firmware);
    STATUS GetFlashSize(std::string &size);
    STATUS GetFlashModel(std::string &details);
    STATUS GetCPUType(std::string &type);
    STATUS GetRAMSize(std::string &temperature);
    STATUS MountNewVersion();
    STATUS UnmountNewVersion();
    STATUS FirmwareCheck(std::string &result);
    STATUS HandleFallbackVersion(WORD timeOutInSec=15);
    STATUS ExposeEmbeddedNewVersion();
    STATUS AddDropRuleToShorewall(std::string drop_ip,BOOL drop_ping_only=FALSE);
    STATUS RestoreFallbackVersion();
    STATUS StopCheckEth0Link();
    STATUS StartCheckEth0Link();
    STATUS ClearTcpDumpStorage();
    STATUS StartTcpDump(const DWORD entitiyType,const std::string filterStr, char *startTimeStr, char *ipStr,
        DWORD allocatedUnits, eIpType	ipType, std::string &result);
    STATUS StopTcpDump();
    STATUS KillSshd();
    STATUS ReadValueFromRegisterBySensor(std::string &ans, const std::string sensor, const std::string register_address);
    STATUS OpenPortsForApache(std::string host,std::string ports);
    STATUS ReadTempFromAdvantechUtil(std::string& ans);
    STATUS CheckCSIpConfig(DWORD CSIpAddress,DWORD &retVal);
    STATUS AddIpandNatRuleForDNSPerService(char * ipDnsAddress, std::string route_table, int priority, bool isIpv4, char* ipAddressStr, DWORD& retVal);
    STATUS EnableWhiteList(CPObject* pWhiteList);
    STATUS DisableWhiteList();
    STATUS CheckCSEth2(BOOL& retVal);

    STATUS AddNatRule(std::string& ansdeleteCmd,std::string&	TargetIpDnsAddress,std::string&	SourceIpAddress,
    										std::string&	ports,DWORD	isIpv4,DWORD	isMultiPorts,string protocol ="tcp");
    STATUS DeleteAddNatRule(std::string& deleteNateCmd);
    STATUS GetUdpOccupiedPorts(std::string& if_name);
};

#endif  // CONFIG_MANAGER_API_H_
