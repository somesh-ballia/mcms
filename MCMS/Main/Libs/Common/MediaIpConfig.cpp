// CMediaIpConfig.cpp: implementation of the CMediaIpConfig class.
//
//////////////////////////////////////////////////////////////////////


#include <iomanip>
#include "MediaIpConfig.h"
#include "StringsLen.h"
#include "SystemFunctions.h"

extern char* CardMediaIpConfigStatusToString(APIU32 mediaIpConfigStatus);
extern char* IpTypeToString(APIU32 ipType, bool caps = false);


// ------------------------------------------------------------
CMediaIpConfig::CMediaIpConfig ()
{
}


// ------------------------------------------------------------
CMediaIpConfig::CMediaIpConfig (const CMediaIpConfig& rOther)
:CPObject(rOther)
{
	memcpy( &m_mediaIpConfigStruct,
		    &(rOther.m_mediaIpConfigStruct),
			sizeof(MEDIA_IP_CONFIG_S) );

	*this = rOther;
}

// ------------------------------------------------------------
CMediaIpConfig::~CMediaIpConfig ()
{
}


// ------------------------------------------------------------
CMediaIpConfig& CMediaIpConfig::operator = (const CMediaIpConfig &other)
{
	memcpy( &m_mediaIpConfigStruct,
		    &(other.m_mediaIpConfigStruct),
			sizeof(MEDIA_IP_CONFIG_S) );

	return *this;
}


// ------------------------------------------------------------
WORD operator==(const CMediaIpConfig& lhs,const CMediaIpConfig& rhs)
{
	return ( (lhs.m_mediaIpConfigStruct.serviceId == rhs.m_mediaIpConfigStruct.serviceId) &&
		     (lhs.m_mediaIpConfigStruct.pqNumber  == rhs.m_mediaIpConfigStruct.pqNumber) );
}


// ------------------------------------------------------------
bool operator<(const CMediaIpConfig& lhs,const CMediaIpConfig& rhs)
{
	if (lhs.m_mediaIpConfigStruct.serviceId != rhs.m_mediaIpConfigStruct.serviceId)
		return (lhs.m_mediaIpConfigStruct.serviceId < rhs.m_mediaIpConfigStruct.serviceId);

	return (lhs.m_mediaIpConfigStruct.pqNumber < rhs.m_mediaIpConfigStruct.pqNumber);
}


// ------------------------------------------------------------
MEDIA_IP_CONFIG_S* CMediaIpConfig::GetMediaIpConfigStruct()
{
	return &m_mediaIpConfigStruct;
}


// ------------------------------------------------------------
void  CMediaIpConfig::Dump(std::ostream& msg) const
{
	msg.setf(std::ios::left,std::ios::adjustfield);
	msg.setf(std::ios::showbase);

	msg << "\n\n"
		<< "MediaIpConfig::Dump\n"
		<< "-------------------\n";

	char ipAddressStr[IP_ADDRESS_LEN],
	     defaultGatewayIPv6Str[IPV6_ADDRESS_LEN];

	SystemDWORDToIpString(m_mediaIpConfigStruct.iPv4.iPv4Address, ipAddressStr);
	strncpy(defaultGatewayIPv6Str, (char*)(m_mediaIpConfigStruct.defaultGatewayIPv6), sizeof(defaultGatewayIPv6Str) - 1);
	defaultGatewayIPv6Str[sizeof(defaultGatewayIPv6Str) - 1] ='\0';

	char* mediaIpConfigStatusStr =
	                  ::CardMediaIpConfigStatusToString( (eMediaIpConfigStatus)(m_mediaIpConfigStruct.status) );

	if (mediaIpConfigStatusStr)
	{
		msg	<< std::setw(17) << "Status: " << mediaIpConfigStatusStr			<< "\n";
	}
	else
	{
		msg	<< std::setw(17) << "Status: (invalid: "<< mediaIpConfigStatusStr	<< ")\n";
	}

	msg	<< std::setw(17) << "Service Id : " << m_mediaIpConfigStruct.serviceId	<< "\n";

	msg	<< std::setw(17) << "IPv4 Address: "	<< ipAddressStr					<< "\n";
	for (int i=0; i<NUM_OF_IPV6_ADDRESSES; i++)
	{
		msg	<< "IPv6 Address " << i << std::setw(3) << ": " << m_mediaIpConfigStruct.iPv6[i].iPv6Address << "\n";
	}
	msg	<< std::setw(17) << "IP Type: " << IpTypeToString(m_mediaIpConfigStruct.ipType)	<< "\n";

	msg	<< std::setw(17) << "PQ Number: " << m_mediaIpConfigStruct.pqNumber	<< "\n";
	
	msg	<< std::setw(17) << "IPv6 Default GW: " << defaultGatewayIPv6Str << "\n";

	msg<< "\n\n";
}


// ------------------------------------------------------------
eMediaIpConfigStatus CMediaIpConfig::GetStatus ()
{
	return (eMediaIpConfigStatus)(m_mediaIpConfigStruct.status);
}


// ------------------------------------------------------------
void CMediaIpConfig::SetStatus (const eMediaIpConfigStatus status)
{
	m_mediaIpConfigStruct.status = (DWORD)status;
}


// ------------------------------------------------------------
DWORD CMediaIpConfig::GetServiceId ()
{
	return m_mediaIpConfigStruct.serviceId;
}


// ------------------------------------------------------------
void CMediaIpConfig::SetServiceId (const DWORD id)
{
	m_mediaIpConfigStruct.serviceId = id;
}


// ------------------------------------------------------------
eIpType CMediaIpConfig::GetIpType ()
{
	return (eIpType)(m_mediaIpConfigStruct.ipType);
}


// ------------------------------------------------------------
void CMediaIpConfig::SetIpType (const eIpType ipType)
{
	m_mediaIpConfigStruct.ipType = (APIU32)ipType;
}


// ------------------------------------------------------------
DWORD CMediaIpConfig::GetIpV4Address ()
{
	return m_mediaIpConfigStruct.iPv4.iPv4Address;
}


// ------------------------------------------------------------
void CMediaIpConfig::SetIpV4Address (const DWORD address)
{
	m_mediaIpConfigStruct.iPv4.iPv4Address = address;
}


// ------------------------------------------------------------
void CMediaIpConfig::GetIpV6Address(int idx, char* retStr)
{
	if ( (0 <= idx) && (NUM_OF_IPV6_ADDRESSES > idx) )
	{
		strncpy( retStr,
				 (char*)(m_mediaIpConfigStruct.iPv6[idx].iPv6Address),
				 IPV6_ADDRESS_LEN );
	}
}


// ------------------------------------------------------------
void CMediaIpConfig::SetIpV6Address(const char* ipV6Address, int idx)
{
	if ( (0 <= idx) && (NUM_OF_IPV6_ADDRESSES > idx) )
	{
		memset( &m_mediaIpConfigStruct.iPv6[idx], 0, sizeof(IPV6_S) );

		strncpy( (char*)(m_mediaIpConfigStruct.iPv6[idx].iPv6Address),
				 ipV6Address,
				 IPV6_ADDRESS_LEN );
	}
}


// ------------------------------------------------------------
BYTE CMediaIpConfig::GetPqNumber ()
{
	return m_mediaIpConfigStruct.pqNumber;
}


// ------------------------------------------------------------
void CMediaIpConfig::SetPqNumber (const BYTE num)
{
	m_mediaIpConfigStruct.pqNumber = num;
}


// ------------------------------------------------------------
void CMediaIpConfig::SetDefaultGatewayIPv6(const char* defaultGateway)
{
	strncpy( (char*)(m_mediaIpConfigStruct.defaultGatewayIPv6),
			 defaultGateway,
			 IPV6_ADDRESS_LEN );
}


// ------------------------------------------------------------
char* CMediaIpConfig::GetDefaultGatewayIPv6() const
{
	return (char*)(m_mediaIpConfigStruct.defaultGatewayIPv6);
}


// ------------------------------------------------------------
void CMediaIpConfig::SetData(const char *data)
{
	memcpy( &m_mediaIpConfigStruct, data, sizeof(MEDIA_IP_CONFIG_S) );
}


// ------------------------------------------------------------
void CMediaIpConfig::SetData(MEDIA_IP_CONFIG_S* mediaIpConfig)
{
	memcpy(&m_mediaIpConfigStruct, mediaIpConfig, sizeof(MEDIA_IP_CONFIG_S));
}

// ------------------------------------------------------------
