/*
 * NetworkPlatform.cpp
 *
 *  Created on: Jul 22, 2013
 *      Author: stanny
 */

#include "NetworkPlatform.h"
#include "SysConfigKeys.h"
#include "SystemFunctions.h"
#include "Trace.h"
#include "TraceStream.h"
#include "IfConfig.h"
#include "McmsNetworkProcess.h"
#include <grp.h>
namespace McmsNetworkPackage {



BYTE CBaseNetworkPlatform::m_SystemState =0;

CBaseNetworkPlatform::CBaseNetworkPlatform() {
	m_strAlarm ="";
	init();
}

CBaseNetworkPlatform::~CBaseNetworkPlatform() {

}
BOOL CBaseNetworkPlatform::IsRedundancySystemFlag()
{
	return IsBondingEnabled();
}

BOOL		CBaseNetworkPlatform::TestRoot()
{
	gid_t id = getgid();
	return (id == 0) ? TRUE : FALSE;
}

void CBaseNetworkPlatform::ReadCommonSystemFlags()
{
	CSysConfig *pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	if (pSysConfig)
	{
		BOOL isJITCMode = NO;
		BOOL isManagmentSep = NO;
		BOOL isMultiSerives = NO;
		pSysConfig->GetBOOLDataByKey(CFG_KEY_ULTRA_SECURE_MODE, isJITCMode);
		pSysConfig->GetBOOLDataByKey(CFG_KEY_SEPARATE_NETWORK, isManagmentSep);
		pSysConfig->GetBOOLDataByKey(CFG_KEY_MULTIPLE_SERVICES, isMultiSerives);

		if(isJITCMode)
			SetSystemFlagState(flagJitcMode);
		if(isManagmentSep)
			SetSystemFlagState(flagSeperatedNetwork);
		if(isMultiSerives)
			SetSystemFlagState(flagMultipleServices);
		if(IsRedundancySystemFlag())
			SetSystemFlagState(flagLanRedunancy);

	}
}

void CBaseNetworkPlatform::init()
{
	if(YES == IsTarget())
	{
		SetSystemFlagState(flagIsTarget);
	}
	ReadCommonSystemFlags();
}

STATUS CBaseNetworkPlatform::KillDHCP(eIpType ipType)
{
	TEST_ROOT_AND_RETURN

	 std::string cmd,sMngmntNI,answer;
	 STATUS status = STATUS_OK;

	 sMngmntNI = GetLogicalInterfaceName(eManagmentNetwork, ipType);
	 cmd = "kill -q  `ps | grep dhcpcd | grep \"" + sMngmntNI + "\" | sed s/root.*//` ; echo killing DHCPDC";
	 status = SystemPipedCommand(cmd.c_str(),answer);

	 if (status != STATUS_OK)
	 {
		 PASSERTMSG(status,answer.c_str());
		 status = STATUS_FAIL;
	 }
	 return status;
}

STATUS    CBaseNetworkPlatform::OnConfigDHCP(eIpType ipType)
{
	TEST_ROOT_AND_RETURN
	STATUS status = STATUS_OK;
	if ((eIpType_IpV6 == ipType) || (eIpType_Both == ipType))
		{
			CSysConfig *pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
			if (pSysConfig)
			{
					BOOL isDHCP_IPV6 = FALSE;
					pSysConfig->GetBOOLDataByKey(CFG_KEY_ENABLE_DHCPV6,isDHCP_IPV6 );
					if(isDHCP_IPV6)
					{
						std::string ans;
						std::string cmd = GetCmdLinePrefix() + "/sbin/dhclient -6&";

						STATUS stat = SystemPipedCommand(cmd.c_str(), ans,TRUE,TRUE,FALSE);

						TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<"CBaseNetworkPlatform::OnConfigDHCP  - EnableIPv6 :" << cmd << std::endl << "answer:" << ans << ", stat:" << stat;

					}
					else
					{
						std::string ans;
						std::string cmd =GetCmdLinePrefix() +  "kill -9 `ps | grep 'dhclient -6' | grep -v grep | head -1 | tr -d ' ' | cut -d 'r' -f1`";

						STATUS stat = SystemPipedCommand(cmd.c_str(), ans);

						TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "CBaseNetworkPlatform::OnConfigDHCP - DisableIPv6 :" << cmd << std::endl << "answer:" << ans << ", stat:" << stat;

					}
			}
		}
	return status;
}


eConfigInterfaceType CBaseNetworkPlatform::GetMngrIfType()
{
	eConfigInterfaceType ifType = eManagmentNetwork;

	if((m_SystemState & flagSeperatedNetwork)&&(m_SystemState & flagJitcMode))
		ifType = eSeparatedManagmentNetwork;

	return ifType;
}

void   CBaseNetworkPlatform::RetrieveRouterList(CIPService *pService, std::list<CRouter> & routerList)
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



//Returns the 'logical' interface name
std::string CBaseNetworkPlatform::GetLogicalInterfaceName(const eConfigInterfaceType ifType,const eIpType ipType)
{
	const eConfigInterfaceNum ifNum = GetInterfaceNum(ifType, ipType);
	return GetConfigInterfaceNumName(ifNum);
}
//Returns the 'hardware' device name
const char* CBaseNetworkPlatform::GetDeviceName(const eConfigInterfaceType ifType)
{
	const eConfigDeviceName ifName = GetInterfaceDeviceNum(ifType);
	return GetConfigDeviceName(ifName);
}

STATUS CBaseNetworkPlatform::ConfigBondingInterface( std::string& bondingMode,DWORD linkmonitoringfrequency){
	TEST_ROOT_AND_RETURN
	STATUS status = STATUS_OK;
	std::string bondingCmd,answer;

	char strlinkFreq[32];
	sprintf(strlinkFreq,"%d",linkmonitoringfrequency);
	bondingCmd = "/sbin/modprobe bonding mode=" + bondingMode + " miimon=" + strlinkFreq;
	status = SystemPipedCommand(bondingCmd.c_str() ,answer);

	bondingCmd = "/sbin/ifconfig bond0 up";
	status = SystemPipedCommand(bondingCmd.c_str() ,answer);
	return status;
}

STATUS CBaseNetworkPlatform::ConfigBondingInterfaceSlaves()
{
	TEST_ROOT_AND_RETURN
	STATUS status = STATUS_OK;
    std::string answer;

	std::string bondingCmd = "/sbin/ifenslave bond0 eth1";
	SystemPipedCommand(bondingCmd.c_str() ,answer);
	bondingCmd = "/sbin/ifenslave bond0 eth2";
	SystemPipedCommand(bondingCmd.c_str() ,answer);
	return status;
}

STATUS CBaseNetworkPlatform::AddRouteTableRule(eConfigInterfaceType networkType,eIpType ipType,const std::string & ip,
															 const DWORD mask,
															 const std::string & defaultGW,
															 const std::string ipv6SubNetMask,
															 std::list<CRouter>& routerList,
															 const BOOL bIPv6)
{

	TEST_ROOT_AND_RETURN
	STATUS stat = STATUS_OK;
	eConfigInterfaceNum ifNum = GetInterfaceNum(networkType,ipType);
	std::string answer,rule_cmd = "";
	std::string route_cmd1 = "";
	std::string route_cmd2 = "";
	std::string static_route_cmd = "";
	std::string route_table = "";
	std::string cache_table_cmd = "";
	std::string remove_gw_cmd = "";
	std::string mngmt_gw_cmd = "";
	std::string cidrIpAddr = "";
	std::string cidrGWIpAddr = "";
	std::string stNicName = "";

	if(!bIPv6)
		{
			stNicName = GetLogicalInterfaceName(networkType, ipType);

			cidrIpAddr = ConvertIpAddressToCIDRNotation(ip, mask);

			route_table = GetRouteTableName(ifNum);
			TRACESTR(eLevelInfoNormal) << "CBaseNetworkPlatform::AddDefaultGWRoutingTableRule " << "\n" << " ifType=" << networkType << " ifNum=" << ifNum << " stNicName=" << stNicName << " route_table=" << route_table << "\n";

			rule_cmd = GetCmdLinePrefix() + "ip rule add from " + ip + " lookup " + route_table;
			//route_cmd = "ip route add table " + route_table + " default src " + ip + " dev " + stNicName;

			route_cmd1 = GetCmdLinePrefix() + "ip route add table " + route_table + " unicast " + cidrIpAddr + " dev " + stNicName + " src " + ip;

			route_cmd2 = GetCmdLinePrefix() + "ip route add table " + route_table + " default via " + defaultGW + " dev " + stNicName;

			stat = SystemPipedCommand(rule_cmd.c_str() ,answer);
			TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "CBaseNetworkPlatform::AddDefaultGWRoutingTableRule :" << rule_cmd << std::endl << answer;

			stat = SystemPipedCommand(route_cmd1.c_str() ,answer);
			TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "CBaseNetworkPlatform::AddDefaultGWRoutingTableRule :" << route_cmd1 << std::endl << answer;

			stat = SystemPipedCommand(route_cmd2.c_str() ,answer);
			TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "CBaseNetworkPlatform::AddDefaultGWRoutingTableRule :" << route_cmd2 << std::endl << answer;
		}
		else
		{
			//Net Sep Jitc
			if((m_SystemState & flagSeperatedNetwork)&&(m_SystemState & flagJitcMode))
			{
				std::string route_table = GetIPv6RouteTableName(ifNum);
				std::string NIC = GetLogicalInterfaceName(networkType, ipType);

				if ("eth0.2197"==NIC)
					ConfigNetworkSeperationIpv6Vlan(route_table,NIC);
				else if ( "eth0.2198"==NIC)
					ConfigNetworkSeperationIpv6Vlan(route_table,NIC);
			}
			else
			{
				TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "CBaseNetworkPlatform::AddDefaultGWRoutingTableRule ip:" << ip << " mask: " << mask << " default gw: " << defaultGW << " ipv6SubNetMask:" << ipv6SubNetMask << " bIPv6:" << (YES == bIPv6 ? "YES" : "NO") << std::endl;
				ConfigureNetworkSeperationIpv6Only(networkType, ipType,ip,mask,defaultGW,ipv6SubNetMask,bIPv6);
			}


		}

		if (stat == STATUS_OK)
		{
			TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "CBaseNetworkPlatform::AddDefaultGWRoutingTableRule :num_of_routers=";

			for (std::list<CRouter>::const_iterator itr = routerList.begin();itr != routerList.end();itr++)
			{
				CRouter router = *itr;
				static_route_cmd = GetCmdLinePrefix() + "ip route add table " + route_table + " ";

				if (router.m_targetIP != "255.255.255.255")
				{
					cidrGWIpAddr = ConvertIpAddressToCIDRNotation(router.m_gateway, router.m_subNetmask);
					static_route_cmd += cidrGWIpAddr + " via " + router.m_targetIP + " dev " + stNicName;

					stat = SystemPipedCommand(static_route_cmd.c_str() ,answer);
					TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
										"CBaseNetworkPlatform::AddDefaultGWRoutingTableRule :" << answer;
				}


			}
		}
	return stat;
}

void   CBaseNetworkPlatform::ConfigNetworkSeperationIpv6Vlan(std::string& route_table,std::string& NIC)
{
	STATUS stat=STATUS_OK;
	std::string ans,tmpAnswer;
	std::string ifconfig_cmd,ipruleadd_cmd,iprourte_add_cmd,route_cmd;
	ifconfig_cmd = GetCmdLinePrefix() + "ifconfig " + NIC + " | grep inet6 | awk '{ print $3 }'" ;
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


	ifconfig_cmd = GetCmdLinePrefix() + "ip -6 route | grep " + NIC + " | awk '{print $1}'" ;
	stat = SystemPipedCommand(ifconfig_cmd.c_str(), ans);
	std::istringstream split2(ans);
	for (std::string each; std::getline(split2, each, split_char); tokens2.push_back(each))
	{
		if (each == "default")
			continue;
		iprourte_add_cmd = GetCmdLinePrefix() + "ip -6 route add table " + route_table + " unicast " + each + " dev " + NIC;
		stat = SystemPipedCommand(iprourte_add_cmd.c_str(), tmpAnswer);
	}
	route_cmd = GetCmdLinePrefix() +  "echo -n `ip -6 route | grep " + NIC + " | awk '/default/ { print $3 }'`";
	stat = SystemPipedCommand(route_cmd.c_str(), ans);
	route_cmd = GetCmdLinePrefix() +  "ip -6 route add table " + route_table + " via " + ans + " dev " + NIC;
	stat = SystemPipedCommand(route_cmd.c_str(), ans);


}




STATUS CBaseNetworkPlatform::DelRouteTableRule(eConfigInterfaceType networkType,eIpType ipType,const std::string ip,
															 const DWORD mask,
															 const std::string defaultGW,
															 const BOOL bIPv6)
{
	TEST_ROOT_AND_RETURN

	eConfigInterfaceNum ifNum = GetInterfaceNum(networkType,ipType);

	std::string route_table,stNicName,mngmt_gw_cmd,remove_gw_cmd,answer,rule_cmd,Ipv6_default_gw,route_cmd1;
	std:string	cache_table_cmd = GetCmdLinePrefix() + "ip route flush cache";
	STATUS	stat = SystemPipedCommand(cache_table_cmd.c_str() ,answer);
		TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "CBaseNetworkPlatform::DelDefaultGWRoutingTableRule :" << cache_table_cmd << std::endl << answer;

	if(!bIPv6)
	{
			stNicName = GetLogicalInterfaceName(networkType, ipType);

			//cidrIpAddr = ConvertIpAddressToCIDRNotation(ip, mask);

			route_table = GetRouteTableName(ifNum);
			TRACESTR(eLevelInfoNormal) << "CBaseNetworkPlatform::DelDefaultGWRoutingTableRule " << "\n" << " ifType=" << networkType << " ifNum=" << ifNum << " stNicName=" << stNicName << " route_table=" << route_table << "\n";

			rule_cmd = GetCmdLinePrefix() + "ip rule del from " + ip + " lookup " + route_table;
			//route_cmd = "ip route add table " + route_table + " default src " + ip + " dev " + stNicName;

			route_cmd1 = GetCmdLinePrefix() + "ip route flush table " + route_table ;

			stat = SystemPipedCommand(route_cmd1.c_str() ,answer);
			TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "CBaseNetworkPlatform::DelDefaultGWRoutingTableRule :" << route_cmd1 << std::endl << answer;

			stat = SystemPipedCommand(rule_cmd.c_str() ,answer);
			TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "CBaseNetworkPlatform::DelDefaultGWRoutingTableRule :" << rule_cmd << std::endl << answer;
	}
	else
	{
			route_table = GetIPv6RouteTableName(ifNum);
			stNicName = GetLogicalInterfaceName(networkType, eIpType_IpV6);

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
				TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "CBaseNetworkPlatform::DelDefaultGWRoutingTableRule 1:" << remove_gw_cmd << " ,answer: " << answer;
			}
	}
	return stat;
}

std::string CBaseNetworkPlatform::GetMacAddress(std::string interface)
{
	std::string macAddress ="";

	std::string macAddressCmd = "echo -n `" +GetCmdLinePrefix()  +  "/sbin/ifconfig " + interface + "| head -n1|awk '{print $5}'`";
	SystemPipedCommand(macAddressCmd.c_str(), macAddress);

	PASSERTMSG(macAddress.size() == 0 || macAddress.size() > 18, "Wrong MAC Address");

	return macAddress;
}

STATUS CBaseNetworkPlatform::AddIpInterface(eConfigInterfaceType eNetworkType, eIpType ipType,const std::string & ip,const std::string & mask,const std::string & gateway,
														  	  	  	  	  	  	  	  	  	  	  	  	  	  	  const std::string & broadcast,
														  	  	  	  	  	  	  	  	  	  	  	  	  	  	  const std::list<CRouter> &routerList)
{
	TEST_ROOT_AND_RETURN

    std::string answer,ifconfig_cmd;
	std::string route_add_cmd,stNicName;
    std::ostringstream cmd;

	stNicName	= GetLogicalInterfaceName(eNetworkType,ipType);

	TRACESTR(eLevelInfoNormal) << "CBaseNetworkPlatform::AddIpInterface() ifType:" << eNetworkType <<" ipType:" << ipType <<" stNicName:"<<stNicName;

    STATUS stat = STATUS_FAIL;

    if((m_SystemState & flagSeperatedNetwork)&&(m_SystemState & flagJitcMode))
   	{
    	//Remove permanent network interface
   		std::string stPermanentNetworkName = GetLogicalInterfaceName(ePermanentNetwork,eIpType_IpV4);
   		ifconfig_down(stPermanentNetworkName);
   	}

	ifconfig_up(stNicName);

	//Add default router
	std::string routeCmd = GetCmdLinePrefix() + "/sbin/route add default gw " + gateway + " " + stNicName;
	SystemPipedCommand(routeCmd.c_str() ,answer);
	string ipToCompare = "0.0.0.0"; // for testing if Arping check is needed
	// Arping
	if(ip != ipToCompare) // check arping only if needed (VNGR-12185)
	{
	    std::string arpingCmd = GetCmdLinePrefix() + "/usr/bin/arping -c 2 -D ";
		arpingCmd += "-I " + stNicName + " ";
	   	arpingCmd += ip;
	    stat = SystemPipedCommand(arpingCmd.c_str() ,answer); //disable IP Management Configuration

	    TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "CBaseNetworkPlatform::AddIpInterface :" << arpingCmd << std::endl << answer;
		if (stat)
		{
			return STATUS_DUPLICATE_ADDRESS;
		}
	}
    ifconfig_down(stNicName);

    //Config new Management IP configuration
   	ifconfig_cmd = GetCmdLinePrefix() + "/sbin/ifconfig " + stNicName + " " + ip + " netmask " + mask +  " broadcast " + broadcast;
    stat = SystemPipedCommand(ifconfig_cmd.c_str() ,answer);
    TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
        "CBaseNetworkPlatform::AddIpInterface(Add Inter) :" << answer;

    if(STATUS_OK != stat)
    {
        return STATUS_FAILED_CFG_IP;
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
            "CBaseNetworkPlatform::AddIpInterface :" << answer;
        }
    }


    if (stat == STATUS_OK)
    {
    	TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "CBaseNetworkPlatform::AddIpInterface :num_of_routers=" << routerList.size();

    	for (std::list<CRouter>::const_iterator itr = routerList.begin();itr != routerList.end();itr++)
    	{
    		CRouter router = *itr;
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
		       route_add_cmd += type + router.m_targetIP + " netmask " + router.m_subNetmask + " gw " + router.m_gateway + " dev " + stNicName;
		       stat = SystemPipedCommand(route_add_cmd.c_str() ,answer);
		       TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "CBaseNetworkPlatform::AddIpInterfaceRMX :" << answer;

		    }
    	}

    }

    if(STATUS_OK != stat)
    {
        return STATUS_FAILED_CFG_ROUTERS;
    }

    return STATUS_OK;

}
STATUS CBaseNetworkPlatform::AddIpV6Interface(eConfigInterfaceType ifType,BYTE bIsAutoConfig,std::string ip,std::string ipmask,
																												std::string gateway,
																												std::string gwmask)
{
	TEST_ROOT_AND_RETURN
	std::string answer , ifconfig_cmd , route_add_cmd;
	std::ostringstream cmd;
	STATUS stat = STATUS_OK;


	eConfigInterfaceType interfaceType = eManagmentNetwork;
	if((m_SystemState & flagSeperatedNetwork)&&(m_SystemState & flagJitcMode))
	{
		interfaceType = eSeparatedManagmentNetwork;
		//For Ipv6, we don't support management default gw.
		gateway = "::0";

		std::string nic_name = GetLogicalInterfaceName(interfaceType, eIpType_IpV6);

		TRACESTR(eLevelInfoNormal) << "\nCBaseNetworkPlatform::AddIpV6Interface : isManagmentSep = yes " <<  "NICName = " << nic_name << endl;

		SetNICIpV6AutoConfig(nic_name.c_str(), bIsAutoConfig,interfaceType,eIpType_IpV6,gateway,gwmask);
	}
	else
	{
		std::string dev_name = GetDeviceName(interfaceType);
		SetNICIpV6AutoConfig(dev_name.c_str(), bIsAutoConfig,interfaceType,eIpType_IpV6,gateway,gwmask);
	}
	std::string nic_name = GetLogicalInterfaceName(interfaceType, eIpType_IpV6);

	TRACESTR(eLevelInfoNormal) << "\nCBaseNetworkPlatform::AddIpV6Interface : " << nic_name
						   << "\nAutoConfig = " << (bIsAutoConfig ? "yes" : "no")
						   << "\nAddress    = " << ip		<< "\\" << ipmask
						   << "\nGateway    = " << gateway	<< "\\" << gwmask << "\n";

	//Only for manual configuration
	if(!bIsAutoConfig)
	{

		stat =AddIpv6ToInterfaceManaul(nic_name,ip,ipmask,gateway,gwmask);

	}

	if(STATUS_OK != stat)
	{
		TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
			"\nCBaseNetworkPlatform::AddIpV6Interface : status failed "<< stat;

		if (STATUS_FAILED_CFG_IPV6_DEF_GW != stat) // otherwise it's needed to respond with 'STATUS_FAILED_CFG_IPV6_DEF_GW' explicitly
		{
			stat = STATUS_FAIL;
		}
	}

	return stat;
}

STATUS      CBaseNetworkPlatform::AddIpv6ToInterfaceManaul(std::string nic_name,std::string ip,std::string ipmask,std::string gateway,std::string gwmask)
{
	return  AcAddNi6((char*)nic_name.c_str(),(char*)ip.c_str(),(char*)ipmask.c_str(),(char*)gateway.c_str(),(char*)gwmask.c_str());
}

void CBaseNetworkPlatform::ifconfig_up(std::string stNicName)
{
	STATUS stat = STATUS_OK;
	if (stNicName != "")
	{
		std::string answer;
        std::string ifconfigCmd = GetCmdLinePrefix() + "/sbin/ifconfig " + stNicName + " up";
        stat =  SystemPipedCommand(ifconfigCmd.c_str() ,answer);
		TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
		        "CBaseNetworkPlatform::ifconfig_up(Add Inter) :" << answer;
	}
}


void CBaseNetworkPlatform::ifconfig_down(std::string stNicName)
{
	STATUS stat = STATUS_OK;
	if (stNicName != "")
	{
		std::string answer;
        std::string ifconfigCmd = GetCmdLinePrefix() + "/sbin/ifconfig " + stNicName + " down";
        stat = SystemPipedCommand(ifconfigCmd.c_str() ,answer);
		TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
		        "CBaseNetworkPlatform::ifconfig_down(Remove Inter) :" << answer;
	}
}
std::string CBaseNetworkPlatform::GetCmdAutoConfigIpv6Flag(std::string& value,const std::string &NIC)
{
	std::string cmd;
	cmd ="echo " + value + " > /proc/sys/net/ipv6/conf/" + NIC + "/autoconf";
	return cmd;
}
void CBaseNetworkPlatform::SetNICIpV6AutoConfig(const string &NIC, BYTE bIsAutoConfig,const eConfigInterfaceType ifType , const eIpType ipType,const string ipv6gateway,const string ipv6gwmask)
{
	TEST_ROOT_AND_RETURN_NO_STATUS // no configuration should be done on Pizzas

	eConfigInterfaceNum  ifNum;

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
	/*
	if ((eProductTypeRMX2000 == curProductType) && (NIC == "eth0:2"))
	{
		ifconfig_cmd_for_eth0_2 = GetCmdLinePrefix() + "/sbin/ifconfig | grep -A 2 eth0:2 | grep  inet | awk -F ':' '{ print $2 }' | awk -F ' ' '{ print $1 }'";
		SystemPipedCommand(ifconfig_cmd_for_eth0_2.c_str(), Ipv4_address);

		ifconfig_cmd_for_eth0_2 = GetCmdLinePrefix() + "/sbin/ifconfig | grep -A 2 eth0:2 | grep  Mask | awk -F ':' '{ print $4 }'";
		SystemPipedCommand(ifconfig_cmd_for_eth0_2.c_str(), Mask);

	}*/

	//Deactivate eth0
	ifconfig_down(NIC);
	//Set Ipv6 Autoconfig flags
	sysctl_cmd = GetCmdAutoConfigIpv6Flag(value,NIC);
	stat = SystemPipedCommand(sysctl_cmd.c_str() ,answer);

	if (Ipv4_address == "") //Activate eth0
	     ifconfig_up(NIC);
	/*else //vngr-26661
		 ifconfig_up(NIC,Ipv4_address,Mask);
	*/
	if ( Ipv4_default_gw !="")
	{
		Ipv4_default_gw.resize(Ipv4_default_gw.length()-1);//the address is with end of line in the end of it.
		//Add original Ipv4 default gw
		route_cmd =  GetCmdLinePrefix() +"/sbin/route add default gw " + Ipv4_default_gw + " " + NIC;
		stat = SystemPipedCommand(route_cmd.c_str(), answer);
	}


	//Add original IPv6 Global address
	if(!bIsAutoConfig)
	{
		if (NIC != "eth2" && Ipv6_Manual_address != "")
		{
			ifconfig_cmd = GetCmdLinePrefix() + "/sbin/ifconfig " + NIC + " add " + Ipv6_Manual_address;
			stat = SystemPipedCommand(ifconfig_cmd.c_str(), answer);
		}

		//for manual ipv6 set the default gw manually
		if(!((m_SystemState & flagSeperatedNetwork)&&(m_SystemState & flagJitcMode)))  // don't add default gw in netwrok seperation see above "::0"
		{
			route_cmd = GetCmdLinePrefix() + "/sbin/route -A inet6 add default gw " + ipv6gateway + " " +   NIC;
			SystemPipedCommand(route_cmd.c_str(), answer);
		}
	}
	//Net Sep Jitc
	if ( ipType!=eIpType_IpV4 &&  (m_SystemState & flagSeperatedNetwork)&&(m_SystemState & flagJitcMode)  )
	{
		    ifNum = GetInterfaceNum(ifType,ipType);
		    std::string route_table = GetIPv6RouteTableName(ifNum);
		std::string tmpNic= NIC;
		if (("eth0.2197"==NIC) || ( "eth0.2198"==NIC))
			ConfigNetworkSeperationIpv6Vlan(route_table,tmpNic);
	}
}

std::string CBaseNetworkPlatform::GetStatusAsString(STATUS status)
{
	return CMcmsNetworkProcess::GetProcess()->GetStatusAsString(status);
}

STATUS CBaseNetworkPlatform::DADRequest(const std::string& ip)
{
	  TEST_ROOT_AND_RETURN

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
	  return stat;

}

BOOL CBaseNetworkPlatform::TestMcmsUser()
{
	gid_t id = getgid();
	struct group  *gr  = getgrgid(id);
	if(gr && gr->gr_name)
	{
		std::string grp_mcms="mcms";
		if(strcmp("mcms",gr->gr_name)==0)
			return true;
		else
		{
			TRACESTR(eLevelInfoNormal)<< "not a mcms user  group user : " << gr->gr_name;
		}

	}

	return false;
}

} /* namespace McmsNetworkPackage */
