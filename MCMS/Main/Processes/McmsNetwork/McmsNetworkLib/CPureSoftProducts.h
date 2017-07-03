/*
 * CPureSoftProducts.h
 *
 *  Created on: Aug 14, 2013
 *      Author: stanny
 */

#ifndef CPURESOFTPRODUCTS_H_
#define CPURESOFTPRODUCTS_H_
#include "SystemInterface.h"
#include "CCommonSoftMcuMfwEdgeAxis.h"

namespace McmsNetworkPackage {

class CMngntSoftMcu: public McmsNetworkPackage::CCommonSoftMcuMfwEdgeAxis
{
	CLASS_TYPE_1(CMngntSoftMcu,CCommonSoftMcuMfwEdgeAxis)
public:
	virtual const char* NameOf() const { return "CMngntSoftMcu";}
	CMngntSoftMcu();
	virtual ~CMngntSoftMcu();
};

class CMngntMfw: public McmsNetworkPackage::CCommonSoftMcuMfwEdgeAxis
{
	CLASS_TYPE_1(CMngntMfw,CCommonSoftMcuMfwEdgeAxis)
protected:
	STATUS    OnInitInterfaces() { return STATUS_OK;}
	STATUS    OnPreConfigNetworkMngmnt();  // action which is call just before starting to config the management network.
	STATUS	  OnPostConfigNetworkMngmnt(){return STATUS_OK;}  // action which  is called after config management network.
	STATUS 	  OnConfigNetworkMngmnt() { return STATUS_OK;}
	STATUS    SetIPv4SpanParams();
	STATUS    SetIPv6SpanParams();
	STATUS    ValidateInterfaceIpv4(IP_ADDR_S& ipAddrS);
	STATUS    ValidateInterfaceIpv6(IP_ADDR_S& ipAddrS);

    STATUS	  ValidateInitInterfacesStatus(STATUS& stat) {return STATUS_OK;}


  	std::string GetLogicalInterfaceName(const eConfigInterfaceType ifType,const eIpType ipType);

public:
	virtual const char* NameOf() const { return "CMngntMfw";}
	CMngntMfw();
	virtual ~CMngntMfw();

	virtual DWORD GetTimeoutofVM() { return 60 * SECOND;}
	STATUS TryToRecover(STATUS eSelfLastStatus, STATUS eOtherLastStatus, CBaseNetworkPlatform *pOtherNetwork);

private:
	STATUS CheckPerIPSpan(CIPSpan *pIpSpan, eIpType ipType);
	STATUS CheckStatusWithSystmSetting(CIPSpan* pIpSpan, STATUS status);

private:
    CSystemInterfaceList m_systemInterfaceList;
};

class CMngntEdgeAxis: public McmsNetworkPackage::CCommonSoftMcuMfwEdgeAxis
{
	CLASS_TYPE_1(CMngntEdgeAxis,CCommonSoftMcuMfwEdgeAxis)
protected:
	STATUS	  OnPostConfigNetworkMngmnt(){return STATUS_OK;}  // action which  is called after config management network.
	STATUS 	  OnConfigNetworkMngmnt() { return STATUS_OK;}
	STATUS    SetIPv4SpanParams();
	STATUS    ValidateInterfaceIpv4(IP_ADDR_S& ipAddrS);
    STATUS	  ValidateInitInterfacesStatus(STATUS& stat) {return stat;}
  	std::string GetLogicalInterfaceName(const eConfigInterfaceType ifType,const eIpType ipType);
	STATUS    ValidateInterfaceIpv6(IP_ADDR_S& ipAddrS);

public:
	virtual const char* NameOf() const { return "CMngntEdgeAxis";}
	CMngntEdgeAxis();
	virtual ~CMngntEdgeAxis();
};

class CMngntSoftCallGenerator: public McmsNetworkPackage::CCommonSoftMcuMfwEdgeAxis
{
	CLASS_TYPE_1(CMngntSoftCallGenerator,CCommonSoftMcuMfwEdgeAxis)
public:
	virtual const char* NameOf() const { return "CMngntSoftCallGenerator";}
	CMngntSoftCallGenerator();
	virtual ~CMngntSoftCallGenerator();
};

} /* namespace McmsNetworkPackage */
#endif /* CPURESOFTPRODUCTS_H_ */
