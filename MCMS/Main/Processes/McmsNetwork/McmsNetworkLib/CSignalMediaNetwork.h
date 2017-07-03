/*
 * CSignalMediaNetwork.h
 *
 *  Created on: May 16, 2014
 *      Author: penrod
 */

#ifndef CSIGNALMEDIANETWORK_H_
#define CSIGNALMEDIANETWORK_H_

#include "NetworkPlatform.h"


class CIPServiceList;

namespace McmsNetworkPackage
{

class CSignalMediaNetwork: public McmsNetworkPackage::CBaseNetworkPlatform
{
	CLASS_TYPE_1(CSignalMediaNetwork,CBaseNetworkPlatform)


public:
	CSignalMediaNetwork();
	virtual ~CSignalMediaNetwork();
	virtual const char* NameOf() const { return "CSignalMediaNetwork";}
/*
 * ConfigNetworkSgnlMd function is the interface of Signal/Media network configuration
 * There is no parameter in the function, unlike CManagementNetwork::ConfigNetworkMngmnt() function which need management IP service as parameter.
 * And the management IP service is read by  CNetworkFactory.
 * In my opinion, It's the responsibility of OnInitInterfaces.
 */
	virtual STATUS ConfigNetworkSgnlMd();

protected:
 // OnInitInterfaces is pure virtual function, since every MCU products are the difference in Interfaces
	virtual STATUS OnInitInterfaces() = 0;
	virtual STATUS OnPreConfigNetworkSignalMedia();
	virtual STATUS OnConfigNetworkSignalMedia();
	virtual STATUS OnPostConfigNetworkSignalMedia();


	virtual STATUS	  ValidateInitInterfacesStatus(STATUS& stat) {return stat;}

	// TO-DO: when implement RMX and Gesher/Ninja, the below 2 functions need to be re-implement.
	//        Currently for Multiple Network interfaces in MFW, ignore the 2 functions
	eConfigInterfaceNum GetInterfaceNum(const eConfigInterfaceType ifType,eIpType ipType)
	{
		return eEth0;
	}
	eConfigDeviceName   GetInterfaceDeviceNum(const eConfigInterfaceType ifType)
	{
		return eEth_0;
	}


protected:
	CIPServiceList *m_pSignalMediaServiceList;


};

} /* namespace McmsNetworkPackage */

#endif /* CSIGNALMEDIANETWORK_H_ */
