/*
 * CMngntRmx2000.cpp
 *
 *  Created on: Jul 29, 2013
 *      Author: stanny
 */

#include "CMngntRmx2000.h"
#include "Trace.h"
#include "TraceStream.h"
#include "NetCommonDefines.h"

namespace McmsNetworkPackage {

CMngntRmx2000::CMngntRmx2000() {


}

CMngntRmx2000::~CMngntRmx2000() {

}
/*
STATUS	  CMngntRmx2000::ConfigureEthernetSettingsCpu(eConfigInterfaceType ifType, ePortSpeedType portSpeed)
{
	if (!IsSystemFlagExist(flagIsTarget) )
	{
			TRACESTR(eLevelDebug) << "CMngntRmx2000 - no configuration should be done on Pizzas";
			return STATUS_OK;
	}
	std::string stParams = ParsePortSpeed(portSpeed);

    std::string eth = GetDeviceName(ifType);
    std::string cmd;

    //only for inner NIC eth0, settings are hardcoded
     if("eth0" == eth)
            stParams = "speed 100 autoneg off duplex full";

       cmd = "/sbin/ethtool -s " + eth + " " + stParams;


	   std::string answer;

	    STATUS stat = SystemPipedCommand(cmd.c_str(),answer);
	    TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
	        "CMngntRmx2000::SetEthSetting :" << cmd << std::endl << answer;

	    if(STATUS_OK != stat)
	    {
	        stat = STATUS_FAIL;
	    }
	    return stat;
}
*/
eConfigInterfaceNum CMngntRmx2000::GetInterfaceNum(const eConfigInterfaceType ifType,eIpType ipType)
{
	eConfigInterfaceNum retIfNum = eEth0;
	//check if simulation
	if(!IsSystemFlagExist(flagIsTarget))
		 return eEth0;

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
		case eManagmentNetwork:
				if(eIpType_IpV6 == ipType )
					retIfNum =eEth0;
				else
					retIfNum = eEth0_alias_1;
				break;
		default:
			retIfNum = eEth0_alias_1;
			break;
	}
	return retIfNum;
}

eConfigDeviceName   CMngntRmx2000::GetInterfaceDeviceNum(const eConfigInterfaceType ifType)
{
	return eEth_0;
}

STATUS      CMngntRmx2000::OnPreHandleNetworkSeparationConfigurations()
{
	if(IsSystemFlagExist(flagSeperatedNetwork)&& IsSystemFlagExist(flagJitcMode))
	{
		TRACESTR(eLevelDebug) << "CMngntRmx2000 - is in Jitc and Network separation do network separation";
		return STATUS_OK;
	}
	else
		TRACESTR(eLevelDebug) << "CMngntRmx2000 - is NOT in Jitc and Network separation do not do  network separation";
	return MANGMENT_RMX200_STATUS_NO_NET_SEPERATION;
}

} /* namespace McmsNetworkPackage */
