/*
 * CCommonRmx1500And4000.cpp
 *
 *  Created on: Aug 1, 2013
 *      Author: stanny
 */

#include "CCommonRmx1500And4000.h"
#include "Trace.h"
#include "TraceStream.h"

extern const char* PortSpeedTypeToString(ePortSpeedType type);
namespace McmsNetworkPackage {

CCommonRmx1500And4000::CCommonRmx1500And4000() {
	// TODO Auto-generated constructor stub

}

CCommonRmx1500And4000::~CCommonRmx1500And4000() {
	// TODO Auto-generated destructor stub
}


STATUS    CCommonRmx1500And4000::OnInitInterfaces()
{
	STATUS stat = STATUS_OK;
	//call base function for initalization
	CManagmentNetwork::OnInitInterfaces();

	if (IsSystemFlagExist(flagIsTarget) ) // no configuration should be done on Pizzas
	{
		stat = EvokeNetworkInterfaces(m_pMngmntService->GetIpV6ConfigurationType());
	}

	TRACESTR(eLevelInfoNormal) << "\n CCommonRmx1500And4000::EvokeNetworkInterfaces" << "- Status: " << GetStatusAsString(stat) ;

	return stat;
}

STATUS   CCommonRmx1500And4000::EvokeNetworkInterfaces(eV6ConfigurationType ipConfigType)
{
	TEST_ROOT_AND_RETURN
	STATUS stat = STATUS_OK;
	std::string answer;
	std:string accept_ra_cmd;

	// on ipv6only to remoe Router auto solicitation
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
		"CCommonRmx1500And4000::EvokeNetworkInterfaces: " << cmd1 << std::endl << answer1;

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
			"CCommonRmx1500And4000::EvokeNetworkInterfaces: " << cmd2 << std::endl << answer2;

		if(STATUS_OK != stat)
		{
			stat = STATUS_FAIL;
		}
	}
	return stat;
}

//This Function will Add Network routing table for ipv6 in Manaul Mode Only, for RMX 1500 & 4000 only
//When it is missing.
//Such a case will happen usually on Real network Separation when Configuration is with the same Default ipv6 router
//with 2 different ipv6 Subnets for signalling and Managment
//we will see if the rule appears , if not we will add it - according to the SubnetMask of ipv6
void CCommonRmx1500And4000::ConfigureNetworkSeperationIpv6Only(eConfigInterfaceType ifType, eIpType ipType,std::string ip,DWORD mask,std::string defaultGW,std::string ipv6SubNetMask,BOOL bIPv6)
{
	std::string cmd = "";
	std::string answer = "";
	std::string strSearchstr = "";
	STATUS stat = STATUS_OK;

	if (ip == "" || ip == "::" || ipv6SubNetMask=="" ||  YES != bIPv6 )
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


eConfigDeviceName   CCommonRmx1500And4000::GetInterfaceDeviceNum(const eConfigInterfaceType ifType)
{
	eConfigDeviceName retIfName = eEth_0;
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
	return retIfName;
}

STATUS	   CCommonRmx1500And4000::OnPostConfigNetworkMngmnt()
{
		STATUS retStatus = STATUS_OK;
		ePortSpeedType mngmntPortSpeed = ePortSpeed_Auto;

		// ===== 4. print to log
		CLargeString retStr = " Config Ethernet \n";

		// ===== 3. config
	    if (IsSystemFlagExist(flagIsTarget)) // otherwise no configuration should be done
	    {
	    	retStatus = ConfigureEthernetSettingsCpu(eManagmentNetwork, mngmntPortSpeed);
		}
	    else
	    	retStr << "\nNote: System is not a Target; no actual configuration was done!";


		retStr << "\n Management port speed: "  << ::PortSpeedTypeToString(mngmntPortSpeed)
		       << "\n Configuration status: " << GetStatusAsString(retStatus).c_str();

	    TRACESTR(eLevelInfoNormal) << retStr.GetString();

	    return retStatus;

}
//////////////// RMX 1500 //////////////////////////////////////////////

CMngntRmx1500::CMngntRmx1500()
{

}

CMngntRmx1500::~CMngntRmx1500()
{

}

eConfigInterfaceNum CMngntRmx1500::GetInterfaceNum(const eConfigInterfaceType ifType,eIpType ipType)
{
	eConfigInterfaceNum retIfNum = eEth0;
		//check if simulation
	if(!IsSystemFlagExist(flagIsTarget))
			 return eEth0;

	switch(ifType)
		{
			case(eManagmentNetwork):
				if(IsSystemFlagExist(flagLanRedunancy))
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
				if(IsSystemFlagExist(flagLanRedunancy))
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
	return retIfNum;
}






//////////////// RMX 4000 /////////////////////////////////////////////


CMngntRmx4000::CMngntRmx4000()
{

}
CMngntRmx4000::~CMngntRmx4000()
{

}

eConfigInterfaceNum CMngntRmx4000::GetInterfaceNum(const eConfigInterfaceType ifType,eIpType ipType)
{
	eConfigInterfaceNum retIfNum = eEth0;
		//check if simulation
	if(!IsSystemFlagExist(flagIsTarget))
			 return eEth0;

	switch(ifType)
			{
				case(eManagmentNetwork):
					if(IsSystemFlagExist(flagLanRedunancy))
						retIfNum = eBond0_alias_1;
					else
						retIfNum = eEth1;
					break;
				case(eInternalNetwork):
					retIfNum = eEth0_2093;
					break;
				case(eSignalingNetwork):
					if(IsSystemFlagExist(flagLanRedunancy))
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
	return retIfNum;
}



} /* namespace McmsNetworkPackage */














