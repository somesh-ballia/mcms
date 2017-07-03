/*
 * CMngntRmx2000.h
 *
 *  Created on: Jul 29, 2013
 *      Author: stanny
 */

#ifndef CMNGNTRMX2000_H_
#define CMNGNTRMX2000_H_

#include "CManagmentNetwork.h"

namespace McmsNetworkPackage {

class CMngntRmx2000: public McmsNetworkPackage::CManagmentNetwork {
protected:
	eConfigInterfaceNum GetInterfaceNum(const eConfigInterfaceType ifType,eIpType ipType);
	eConfigDeviceName   GetInterfaceDeviceNum(const eConfigInterfaceType ifType);
	//STATUS	  			ConfigureEthernetSettingsCpu(eConfigInterfaceType ifType, ePortSpeedType portSpeed);
	STATUS     OnPreHandleNetworkSeparationConfigurations();
public:
	CMngntRmx2000();
	virtual ~CMngntRmx2000();
};

} /* namespace McmsNetworkPackage */
#endif /* CMNGNTRMX2000_H_ */
