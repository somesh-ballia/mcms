/*
 * CCommonGesherNinja.h
 *
 *  Created on: Aug 24, 2013
 *      Author: stanny
 */

#ifndef CCOMMONGESHERNINJA_H_
#define CCOMMONGESHERNINJA_H_

#include "CManagmentNetwork.h"

namespace McmsNetworkPackage {

class CCommonGesherNinja: public McmsNetworkPackage::CManagmentNetwork {
	CLASS_TYPE_1(CCommonGesherNinja,CManagmentNetwork)
protected:
	STATUS    OnInitInterfaces();
	STATUS	  OnPostConfigNetworkMngmnt();
	std::string GetCmdLinePrefix(){return "sudo ";};
	BOOL		TestRoot() {return TRUE;};
	std::string GetCmdAutoConfigIpv6Flag(std::string& value,const std::string &NIC);
	STATUS      AddIpv6ToInterfaceManaul(std::string nic_name,std::string ip,std::string ipmask,std::string gateway,std::string gwmask);

public:
	CCommonGesherNinja();
	virtual ~CCommonGesherNinja();
	virtual const char* NameOf() const { return "CCommonGesherNinja";}
};

class CMngntGesher: public McmsNetworkPackage::CCommonGesherNinja {
	CLASS_TYPE_1(CMngntGesher,CManagmentNetwork)
protected:
	eConfigInterfaceNum GetInterfaceNum(const eConfigInterfaceType ifType,eIpType ipType);
	eConfigDeviceName   GetInterfaceDeviceNum(const eConfigInterfaceType ifType);
	STATUS     OnConfigDHCP(eIpType ipType);
public:
		CMngntGesher();
	virtual ~CMngntGesher();
	virtual const char* NameOf() const { return "CMngntGesher";}
};

class CMngntNinja: public McmsNetworkPackage::CCommonGesherNinja {
	CLASS_TYPE_1(CMngntNinja,CManagmentNetwork)
protected:
	eConfigInterfaceNum GetInterfaceNum(const eConfigInterfaceType ifType,eIpType ipType);
	eConfigDeviceName   GetInterfaceDeviceNum(const eConfigInterfaceType ifType);
	STATUS ConfigBondingInterfaceSlaves();
	STATUS ConfigBondingInterface(std::string& bondingMode,DWORD linkmonitoringfrequency);

public:
	CMngntNinja();
	virtual ~CMngntNinja();
	virtual const char* NameOf() const { return "CMngntNinja";}
};


} /* namespace McmsNetworkPackage */
#endif /* CCOMMONGESHERNINJA_H_ */
