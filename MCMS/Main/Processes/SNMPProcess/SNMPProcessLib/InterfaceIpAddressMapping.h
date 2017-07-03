#ifndef INTERFACE_IPADDRESS_MAPPING_H_
#define INTERFACE_IPADDRESS_MAPPING_H_

#include <string.h>
#include "ifTable.h"
#include "IpAddressDefinitions.h"

#include <map>
#include "SharedDefines.h"

using namespace std;
class CInterfaceIndexIpAddressMapping;

enum IPAddress_Type{IPAddressIPV4, IPAddressIPV6 };
class InterfaceIpAddress
{	
	
public:	
	
	bool IsIpV4NetworkAddressSet() const
	{
		return m_bNetworkIp4AddressSet;
	}
	APIU32 GetIpV4NetworkAddress() const 
	{ 
		return 	m_networkIp4Address; 
	}
	void SetNetworkIp4Address(APIU32 networkIp4Address) 
	{ 
		m_networkIp4Address = networkIp4Address; 
		m_bNetworkIp4AddressSet = TRUE;
	}

	
	bool IsIpV6AddressSet() const
	{
		return m_bIpv6AddressSet;
	}
	const byte* GetIpV6Address() const 
	{
		return 	m_ipv6Address; 
	}
	
	// Assumption ipv6Address length is IPV6_ADDRESS_BYTES_LEN!
	void SetIpv6Address(const byte*  ipv6Address) 
	{ 
		memcpy(m_ipv6Address, ipv6Address, (size_t)IPV6_ADDRESS_BYTES_LEN); 
		m_bIpv6AddressSet = TRUE;
	}
	
private:
	
	APIU32	m_networkIp4Address;	
	bool	m_bNetworkIp4AddressSet;
	
	byte	m_ipv6Address[IPV6_ADDRESS_BYTES_LEN]; /* IPv6 address */
	bool	m_bIpv6AddressSet;
	
	friend class CInterfaceIndexIpAddressMapping;
};


typedef std::map<eInterfaceIndex, InterfaceIpAddress> CInterfaceIndexIpAddressMap;


class CInterfaceIndexIpAddressMapping
{
	public:
		CInterfaceIndexIpAddressMap& GetInterfaceIpAddressMap();
		const InterfaceIpAddress* GetIpAddressEntryByInterface(eInterfaceIndex eIfFace) const;
		
		
		void SetIpv4NetworkAddress(eInterfaceIndex eIfFace, DWORD networkIp4Address);
		
		void SetIpV6Address(eInterfaceIndex eIfFace, const byte*  ipv6Address);
		
	private:
	
		InterfaceIpAddress* GetOrCreateIpAddressEntryByInterface(eInterfaceIndex eIfFace) ;

		
		CInterfaceIndexIpAddressMap	m_interfaceIndexIpAddressMap;	
		
};

#endif
