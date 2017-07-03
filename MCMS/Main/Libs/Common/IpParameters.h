// IpParameters.h: interface for the CIpParameters class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _IpParameters_H_
#define _IpParameters_H_


#include "IpInterface.h"
#include "NetworkParameters.h"
#include "DnsConfiguration.h"

class CIPService;


//------------------------------------------------------------------
//              CIpParameters  
//------------------------------------------------------------------
class CIpParameters : public CPObject
{

CLASS_TYPE_1(CIpParameters, CPObject)

public:
	CIpParameters ();
	CIpParameters (const IP_PARAMS_S ipParams);
	virtual ~CIpParameters ();
	virtual void Dump(std::ostream& msg) const;
	virtual const char* NameOf() const { return "CIpParameters";}
	CIpParameters& operator = (const CIpParameters &rOther);

	IP_PARAMS_S*        GetIpParamsStruct ();
	
	IP_INTERFACE_S*     GetIpInterfacesList ();
	IP_INTERFACE_S		GetIpInterfaceByIdx (const int idx);
	void                InsertToInterfacesList (IP_INTERFACE_S &ipInterface);
	void                ClearInterfacesList ();

	NETWORK_PARAMS_S    GetNetworkParams ();
	void                SetNetworkParams (const NETWORK_PARAMS_S netParams);

	DNS_CONFIGURATION_S GetDnsConfiguration ();
	void                SetDnsConfiguration (const DNS_CONFIGURATION_S dnsConfig);

    void                ClearPortSpeedList();
    
	void                SetData(const char *data);
	void                SetData(const IP_PARAMS_S ipParams);

	STATUS				ConvertToIpService(CIPService &ipService);

    void                ValidateStrings();
    
protected:
	IP_PARAMS_S         m_ipParamsStruct;
};



//------------------------------------------------------------------
//              CCsIpParameters  
//------------------------------------------------------------------
class CCsIpParameters : public CPObject
{

CLASS_TYPE_1(CCsIpParameters, CPObject)

public:
	CCsIpParameters ();
	CCsIpParameters (const CS_IP_PARAMS_S ipParams);
	virtual ~CCsIpParameters ();
	virtual void Dump(std::ostream& msg) const;
	virtual const char* NameOf() const { return "CCsIpParameters";}

	CCsIpParameters& operator = (const CCsIpParameters &rOther);

	CS_IP_PARAMS_S*     GetIpParamsStruct ();
	
	IP_INTERFACE_S*     GetIpInterfacesList ();
	void                ClearInterfacesList ();
	void                InsertToInterfacesList (IP_INTERFACE_S &ipInterface);

	NETWORK_PARAMS_S    GetNetworkParams ();
	void                SetNetworkParams (const NETWORK_PARAMS_S netParams);

	DNS_CONFIGURATION_S GetDnsConfiguration ();
	void                SetDnsConfiguration (const DNS_CONFIGURATION_S dnsConfig);

	void                SetData(const char *data);


protected:
	CS_IP_PARAMS_S      m_ipParamsStruct;
};


#endif // _IpParameters_H_
