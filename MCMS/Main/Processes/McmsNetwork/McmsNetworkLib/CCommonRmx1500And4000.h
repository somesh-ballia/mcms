/*
 * CCommonRmx1500And4000.h
 *
 *  Created on: Aug 1, 2013
 *      Author: stanny
 */

#ifndef CCOMMONRMX1500AND4000_H_
#define CCOMMONRMX1500AND4000_H_

#include "CManagmentNetwork.h"

namespace McmsNetworkPackage {

class CCommonRmx1500And4000: public McmsNetworkPackage::CManagmentNetwork {
	CLASS_TYPE_1(CCommonRmx1500And4000,CManagmentNetwork)
protected:
	STATUS    OnInitInterfaces();
	STATUS   EvokeNetworkInterfaces(eV6ConfigurationType ipConfigType);
	void ConfigureNetworkSeperationIpv6Only(eConfigInterfaceType ifType,eIpType ipType,std::string ip,DWORD mask,
	    														std::string defaultGW,std::string ipv6SubNetMask,BOOL bIPv6);
	eConfigDeviceName   GetInterfaceDeviceNum(const eConfigInterfaceType ifType);
	STATUS	   OnPostConfigNetworkMngmnt();
public:
	virtual const char* NameOf() const { return "CCommonRmx1500And4000";}
	CCommonRmx1500And4000();
	virtual ~CCommonRmx1500And4000();
};

class CMngntRmx1500: public McmsNetworkPackage::CCommonRmx1500And4000
{
	CLASS_TYPE_1(CMngntRmx1500,CCommonRmx1500And4000)
protected:
	eConfigInterfaceNum GetInterfaceNum(const eConfigInterfaceType ifType,eIpType ipType);

public:
	virtual const char* NameOf() const { return "CMngntRmx1500";}
	CMngntRmx1500();
	virtual ~CMngntRmx1500();
};

class CMngntRmx4000: public McmsNetworkPackage::CCommonRmx1500And4000
{
	CLASS_TYPE_1(CMngntRmx4000,CCommonRmx1500And4000)
protected:
	eConfigInterfaceNum GetInterfaceNum(const eConfigInterfaceType ifType,eIpType ipType);
public:
	virtual const char* NameOf() const { return "CMngntRmx4000";}
	CMngntRmx4000();
	virtual ~CMngntRmx4000();
};

} /* namespace McmsNetworkPackage */
#endif /* CCOMMONRMX1500AND4000_H_ */
