
#ifndef IPADDESSRTABLE_H_
#define IPADDESSRTABLE_H_

#include <iostream>
#include "Singleton.h"
#include "ifTable.h"
#include "InterfaceIpAddressMapping.h"

class CIpTableAddress : public SingletonHolder<CIpTableAddress>
{
public:
	void IpTableAddress_Init();
	
	void IpTableAddress_AddIp4Address(const std::string &strIpAddress, eInterfaceIndex eIfFace);
	void IpTableAddress_AddIp6Address(const std::string &strIpAddress, eInterfaceIndex eIfFace);

	void IpTableAddress_AddIp6Address(const byte *ipv6Address , eInterfaceIndex eIfFace);
	
private:

	netsnmp_table_row* GetIpV4AddressRowByIndex(netsnmp_table_data_set *dataTable,
												APIU32	ipv4Address);
	netsnmp_table_row* GetIpV6AddressRowByIndex(netsnmp_table_data_set *dataTable,
	                                             const byte	*ipv6Address); 

	
	netsnmp_table_row* CreateIpV4RowToIpTableAddress(eInterfaceIndex eIfFace, APIU32 networkIp4Address);
	netsnmp_table_row* CreateIpV6RowToIpTableAddress(eInterfaceIndex eIfFace, const byte* ipv6Adrress);

	
	CInterfaceIndexIpAddressMapping m_interfaceIndexIpAddressMapping;
	netsnmp_table_data_set *ipTableAddress_table_set;
	netsnmp_table_data_set *ipTable_table_set;


};

#endif                          /* IPADDRESSTABLE_H_ */
