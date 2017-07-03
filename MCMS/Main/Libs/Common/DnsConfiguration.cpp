// SoftwareLocation.cpp: implementation of the CSoftwareLocation class.
//
//////////////////////////////////////////////////////////////////////

#include <iomanip>
#include "NStream.h"
#include "DnsConfiguration.h"
#include "SystemFunctions.h"
#include "ObjString.h"
#include "StringsLen.h"

extern char* DnsDhcpConfigurationTypeToString(APIU32 dnsDhcpConfigType);


// ------------------------------------------------------------
CDnsConfiguration::CDnsConfiguration ()
{
}


// ------------------------------------------------------------
CDnsConfiguration::CDnsConfiguration (const DNS_CONFIGURATION_S dnsConfig)
{
	memcpy( &m_dnsConfigurationStruct, &dnsConfig, sizeof(DNS_CONFIGURATION_S) );
}

	
// ------------------------------------------------------------
CDnsConfiguration::~CDnsConfiguration ()
{
}


// ------------------------------------------------------------
void  CDnsConfiguration::Dump(std::ostream& msg) const
{
	char ipV4IpAddressStr[IP_ADDRESS_LEN];

	msg.setf(std::ios::left,std::ios::adjustfield);
	msg.setf(std::ios::showbase);
	
	msg << "\n ===== DnsConfiguration::Dump ===== \n"
	    << "DNS Status: "  << m_dnsConfigurationStruct.dnsServerStatus << ", "
		<< "Host Name: "   << m_dnsConfigurationStruct.hostName        << ", "
		<< "Domain Name: " << m_dnsConfigurationStruct.domainName      << "\n";
	
	int i=0;
	for (i=0; i<NUM_OF_DNS_SERVERS; i++)
	{
		SystemDWORDToIpString(m_dnsConfigurationStruct.ipV4AddressList[i], ipV4IpAddressStr);
		msg	<< "IpV4 Address " << i << ": " << ipV4IpAddressStr << ", ";
	}
	
	msg	<< "\n";
	for (i=0; i<NUM_OF_DNS_SERVERS; i++)
	{
		msg	<< "IpV6 Address " << i << ": " << m_dnsConfigurationStruct.ipV6AddressList[i]  << ", ";
	}
	
	msg << "\n"
	    << "Is Register: "     << m_dnsConfigurationStruct.isRegister << ", "
		<< "DNS Config From: ";
		
		
	char* dnsDhcpConfigurationTypeStr = ::DnsDhcpConfigurationTypeToString(m_dnsConfigurationStruct.dnsConfiguredFromDHCPv4_or_DHCPv6);
	if (dnsDhcpConfigurationTypeStr)
	{
		msg << dnsDhcpConfigurationTypeStr << "\n";
	}
	else
	{
		msg << "(invalid: " << m_dnsConfigurationStruct.dnsConfiguredFromDHCPv4_or_DHCPv6 << ")\n";
	}
}


// ------------------------------------------------------------
CDnsConfiguration& CDnsConfiguration::operator = (const CDnsConfiguration &rOther)
{
	memcpy( &m_dnsConfigurationStruct,
		    &rOther.m_dnsConfigurationStruct,
			sizeof(DNS_CONFIGURATION_S) );

	return *this;
}


// ------------------------------------------------------------
eServerStatus CDnsConfiguration::GetDnsServerStatus ()
{
	return (eServerStatus)(m_dnsConfigurationStruct.dnsServerStatus);
}


// ------------------------------------------------------------
void CDnsConfiguration::SetDnsServerStatus (const eServerStatus serverStatus)
{
	m_dnsConfigurationStruct.dnsServerStatus = serverStatus;
}


// ------------------------------------------------------------
BYTE* CDnsConfiguration::GetHostName ()
{
	return m_dnsConfigurationStruct.hostName;
}


// ------------------------------------------------------------
void CDnsConfiguration::SetHostName (const BYTE* name)
{
	memcpy(&m_dnsConfigurationStruct.hostName, name, NAME_LEN);
}


// ------------------------------------------------------------
BYTE* CDnsConfiguration::GetDomainName ()
{
	return m_dnsConfigurationStruct.domainName;
}


// ------------------------------------------------------------
void CDnsConfiguration::SetDomainName (const BYTE* name)
{
	memcpy(&m_dnsConfigurationStruct.domainName, name, NAME_LEN);
}


// ------------------------------------------------------------
DWORD* CDnsConfiguration::GetIpV4AddressList ()
{
	return m_dnsConfigurationStruct.ipV4AddressList;
}


// ------------------------------------------------------------
void CDnsConfiguration::SetIpV4AddressList (const DWORD* addresslist)
{
	memcpy(&m_dnsConfigurationStruct.ipV4AddressList,
		   addresslist,
		   sizeof(DWORD)*NUM_OF_DNS_SERVERS);
}


// ------------------------------------------------------------
void CDnsConfiguration::SetV4IpAddressByIdx(int idx ,const DWORD ipAddress)
{
	if ( (0 <= idx) && (NUM_OF_DNS_SERVERS > idx) )
		m_dnsConfigurationStruct.ipV4AddressList[idx] = ipAddress;
}


// ------------------------------------------------------------
BYTE* CDnsConfiguration::GetIpV6AddressList ()
{
	return *(m_dnsConfigurationStruct.ipV6AddressList);
}


// ------------------------------------------------------------
void CDnsConfiguration::SetIpV6AddressList (const BYTE* addresslist)
{
	memcpy(&m_dnsConfigurationStruct.ipV6AddressList,
		   addresslist,
		   IPV6_ADDRESS_LEN*NUM_OF_DNS_SERVERS);
}


// ------------------------------------------------------------
BOOL CDnsConfiguration::GetIsRegfister ()
{
	return m_dnsConfigurationStruct.isRegister;
}


// ------------------------------------------------------------
void CDnsConfiguration::SetIsRegfister (const BOOL isRegister)
{
	m_dnsConfigurationStruct.isRegister = isRegister;
}


// ------------------------------------------------------------
eDnsDhcpConfigurationType CDnsConfiguration::GetDnsConfiguredFromDHCPv4_or_DHCPv6 ()
{
	return (eDnsDhcpConfigurationType)  // (casting)
		   (m_dnsConfigurationStruct.dnsConfiguredFromDHCPv4_or_DHCPv6);
}


// ------------------------------------------------------------
void CDnsConfiguration::SetDnsConfiguredFromDHCPv4_or_DHCPv6
                                   (const eDnsDhcpConfigurationType ipType)
{
	m_dnsConfigurationStruct.dnsConfiguredFromDHCPv4_or_DHCPv6 = ipType;
}


// ------------------------------------------------------------
void CDnsConfiguration::SetData(DNS_CONFIGURATION_S dnsConfig)
{
	memcpy(&m_dnsConfigurationStruct, &dnsConfig, sizeof(DNS_CONFIGURATION_S));
}


// ------------------------------------------------------------

