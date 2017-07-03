// SoftwareLocation.h: interface for the CSoftwareLocation class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _DnsConfiguration_H_
#define _DnsConfiguration_H_


#include "PObject.h"
#include "SharedMcmsCardsStructs.h"
#include "DefinesIpService.h"



class CDnsConfiguration : public CPObject
{

CLASS_TYPE_1(CDnsConfiguration, CPObject)

public:
	CDnsConfiguration ();
	CDnsConfiguration (const DNS_CONFIGURATION_S dnsConfig);
	virtual ~CDnsConfiguration ();
	virtual void Dump(std::ostream& msg) const;

	CDnsConfiguration& operator = (const CDnsConfiguration &rOther);
	virtual const char* NameOf() const { return "CNetworkParameters";}

	eServerStatus  GetDnsServerStatus ();
	void           SetDnsServerStatus (const eServerStatus serverStatus);

	BYTE*   GetHostName ();
	void    SetHostName (const BYTE* name);
	
	BYTE*   GetDomainName ();
	void    SetDomainName (const BYTE* name);
	
	DWORD*  GetIpV4AddressList ();
	void    SetIpV4AddressList (const DWORD* list);
    void    SetV4IpAddressByIdx(int idx ,const DWORD ipAddress);
	
	BYTE*   GetIpV6AddressList ();
	void    SetIpV6AddressList (const BYTE* list);
	
	BOOL    GetIsRegfister ();
	void    SetIsRegfister (const BOOL isRegister);

	eDnsDhcpConfigurationType   GetDnsConfiguredFromDHCPv4_or_DHCPv6 ();
	void                        SetDnsConfiguredFromDHCPv4_or_DHCPv6
		                               (const eDnsDhcpConfigurationType ipType);

	void    SetData(DNS_CONFIGURATION_S dnsConfig);


protected:
	DNS_CONFIGURATION_S m_dnsConfigurationStruct;
};

#endif // _DnsConfiguration_H_
