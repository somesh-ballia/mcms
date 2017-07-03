/*
 * CCommonGesherNinja.cpp
 *
 *  Created on: Aug 24, 2013
 *      Author: stanny
 */

#include "CCommonGesherNinja.h"
#include "Trace.h"
#include "TraceStream.h"

namespace McmsNetworkPackage {

CCommonGesherNinja::CCommonGesherNinja() {
	// TODO Auto-generated constructor stub

}

CCommonGesherNinja::~CCommonGesherNinja() {
	// TODO Auto-generated destructor stub
}

STATUS    CCommonGesherNinja::OnInitInterfaces()
{
//gesher ninja set eth setting in the end post .
	return STATUS_OK;
}



STATUS	  CCommonGesherNinja::OnPostConfigNetworkMngmnt()
{
    //TBD - set ethernet setting for ninja and gesher from mcumngr
	return STATUS_OK;
}

std::string CCommonGesherNinja::GetCmdAutoConfigIpv6Flag(std::string& value,const std::string &NIC)
{
	std::string cmd;
	cmd ="echo " + value + " | sudo tee /proc/sys/net/ipv6/conf/" + NIC + "/autoconf";
	return cmd;
}

STATUS      CCommonGesherNinja::AddIpv6ToInterfaceManaul(std::string nic_name,std::string ip,std::string ipmask,std::string gateway,std::string gwmask)
{
	STATUS stat = STATUS_OK;
	std::string answer,ifconfig_cmd,route_add_cmd;
	ifconfig_cmd = "sudo /sbin/ifconfig " + nic_name + " add " + ip + "/" + ipmask;
	stat = SystemPipedCommand(ifconfig_cmd.c_str(), answer);
	if(stat != STATUS_OK)
		return stat;
			//route -A inet6 add default gw fe80::b2c6:9aff:fed2:1f81 dev eth1
	route_add_cmd = "sudo /sbin/route -A inet6 add default gw " + gateway + " dev " + nic_name;
	stat = SystemPipedCommand(route_add_cmd.c_str(), answer);

	return stat;
}
//////////////////////////////////////// GESHER //////////////////////////
CMngntGesher::CMngntGesher() {
	// TODO Auto-generated constructor stub

}

CMngntGesher::~CMngntGesher() {
	// TODO Auto-generated destructor stub
}


eConfigInterfaceNum CMngntGesher::GetInterfaceNum(const eConfigInterfaceType ifType,eIpType ipType)
{
	eConfigInterfaceNum retIfNum = eEth0;
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
					if (!IsSystemFlagExist(flagLanRedunancy))
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
	return retIfNum;
}

eConfigDeviceName   CMngntGesher::GetInterfaceDeviceNum(const eConfigInterfaceType ifType)
{
	eConfigDeviceName retIfName = eEth_0;
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
	return retIfName;
}

STATUS     CMngntGesher::OnConfigDHCP(eIpType ipType)
{
	TRACESTR(eLevelInfoNormal) << " dhclient is still not in the system of gesher/ninja when it will be inside need to remove this function ";
	return STATUS_OK;
}

///////////////////////////////////////// NINJA  ////////////////////////////////
CMngntNinja::CMngntNinja() {
	// TODO Auto-generated constructor stub

}

CMngntNinja::~CMngntNinja() {
	// TODO Auto-generated destructor stub
}

eConfigInterfaceNum CMngntNinja::GetInterfaceNum(const eConfigInterfaceType ifType,eIpType ipType)
{
	eConfigInterfaceNum retIfNum = eEth0;
	switch(ifType)
	{
		case(eManagmentNetwork):
			if (IsSystemFlagExist(flagMultipleServices))
			{
				retIfNum = eEth0_alias_1;
			}
			else if (IsSystemFlagExist(flagLanRedunancy))
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
			if (!IsSystemFlagExist(flagLanRedunancy))
			{
				retIfNum = eEth0;
			}
			else
			{
				retIfNum = eBond0_alias_1;
			}
			break;
		case(eLanRedundancySignalingNetwork):
			if (!IsSystemFlagExist(flagLanRedunancy))
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
		default:
			retIfNum = eEth0;
	}
	return retIfNum;
}

eConfigDeviceName   CMngntNinja::GetInterfaceDeviceNum(const eConfigInterfaceType ifType)
{
	eConfigDeviceName retIfName = eEth_0;
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
	return retIfName;
}

STATUS CMngntNinja::ConfigBondingInterfaceSlaves()
{
	TEST_ROOT_AND_RETURN
	STATUS status = STATUS_OK;

	std::string answer;
	std::string bondingCmd;
	do
	{
		bondingCmd = GetCmdLinePrefix() + "/sbin/ifenslave -d bond0 eth0 eth1";
		status = SystemPipedCommand(bondingCmd.c_str(), answer);
		if(STATUS_OK != status)
			break;
		bondingCmd = GetCmdLinePrefix() + "/sbin/ifenslave bond0 eth0 eth1";
		status = SystemPipedCommand(bondingCmd.c_str() ,answer);
		if(STATUS_OK != status)
			break;
	}while(0);

	if(STATUS_OK != status)
	{
		TRACESTRFUNC(eLevelError) << "execute command "<< bondingCmd << " error. and return "<< answer;
	}

	return status;
}

STATUS CMngntNinja::ConfigBondingInterface(std::string& bondingMode,DWORD linkmonitoringfrequency)
{
	TEST_ROOT_AND_RETURN
	STATUS status = STATUS_OK;
	std::string bondingCmd,answer;
	do
	{
		bondingCmd = GetCmdLinePrefix() + "/sbin/modprobe bonding mode=active-backup miimon=100";
		status = SystemPipedCommand(bondingCmd.c_str(), answer);
		if(STATUS_OK != status)
			break;
		bondingCmd = GetCmdLinePrefix() + "/sbin/ifconfig bond0 up";
		status = SystemPipedCommand(bondingCmd.c_str() ,answer);
		if(STATUS_OK != status)
			break;
	}while(0);

	if(STATUS_OK != status)
	{
		TRACESTRFUNC(eLevelError) << "execute command "<< bondingCmd << " error. and return "<< answer;
	}

	return status;
}



} /* namespace McmsNetworkPackage */


