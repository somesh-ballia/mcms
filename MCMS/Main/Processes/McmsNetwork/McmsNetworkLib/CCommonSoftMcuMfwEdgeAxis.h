/*
 * CCommonSoftMcuMfwEdgeAxis.h
 *
 *  Created on: Aug 1, 2013
 *      Author: stanny
 *      Cases of SMCU family, not including Gesher/Ninja
 */

#ifndef CCOMMONSOFTMCUMFWEDGEAXIS_H_
#define CCOMMONSOFTMCUMFWEDGEAXIS_H_

#include "CManagmentNetwork.h"

namespace McmsNetworkPackage {

class CCommonSoftMcuMfwEdgeAxis: public McmsNetworkPackage::CManagmentNetwork {
	CLASS_TYPE_1(CCommonSoftMcuMfwEdgeAxis,CManagmentNetwork)
protected:
	STATUS    OnInitInterfaces();
	STATUS    OnPreConfigNetworkMngmnt(){return STATUS_OK;} ;
	eConfigInterfaceNum GetInterfaceNum(const eConfigInterfaceType ifType,eIpType ipType);
	eConfigDeviceName   GetInterfaceDeviceNum(const eConfigInterfaceType ifType);
	std::string GetLogicalInterfaceName(const eConfigInterfaceType ifType,const eIpType ipType);
	STATUS	  ConfigureEthernetSettingsCpu(eConfigInterfaceType ifType, ePortSpeedType portSpeed){return  STATUS_OK;}; //don't configure for pure soft products
	virtual STATUS SetDhcpClientV6();
	STATUS SetIPv4SpanParams();
	STATUS SetIPv6SpanParams();
	STATUS	  ValidateInitInterfacesStatus(STATUS& stat) {return stat; };
	STATUS    ValidateInterfaceIpv4(IP_ADDR_S& ipAddrS); //override default behavior
public:
	virtual const char* NameOf() const { return "CCommonSoftMcuMfwEdgeAxis";}
	CCommonSoftMcuMfwEdgeAxis();
	virtual ~CCommonSoftMcuMfwEdgeAxis();
protected:
	STATUS m_Ipv6_status;
};

} /* namespace McmsNetworkPackage */
#endif /* CCOMMONSOFTMCUMFWEDGEAXIS_H_ */
