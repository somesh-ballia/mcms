/*
 * Note: this file was manually written, although it should be originated using mib2c similarly to ipAddrTable
 */

#include <iostream>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <map>

#include "DataTypes.h"
#include "ipAddressTable.h"
#include "SystemFunctions.h"
#include "SnmpTableUtils.h"
#include "TraceStream.h"


#define IPADDRESSTABLE_OID              1,3,6,1,2,1,4,34


#define COLUMN_IPADDRESSADDRTYPE         1
#define COLUMN_IPADDRESSADDR        	 2
#define COLUMN_IPADDRESSIFINDEX          3


#define INETADDRESSTYPE_IPV4  			1
#define INETADDRESSTYPE_IPV6  			2


/** Initialize the ipA ddrTable table by defining its contents and how it's structured */
void CIpTableAddress::IpTableAddress_Init(void)
{
    //static oid      ipAddressTable_oid[] = { 1, 3, 6, 1, 2, 1, 4, 34 };
    static oid      ipAddressTable_oid[] = { IPADDRESSTABLE_OID };
   
    
    size_t          ipAddressTable_oid_len = OID_LENGTH(ipAddressTable_oid);
    /*
     * create the table structure itself 
     */    
    ipTableAddress_table_set = netsnmp_create_table_data_set("ipAddressTable");

    /*
     * comment this out or delete if you don't support creation of new rows 
     */
    ipTableAddress_table_set->allow_creation = 1;    
    /***************************************************
     * Adding indexes
     */
    DEBUGMSGTL(("initialize_table_ipAddressTable",
                "adding indexes to table ipAddressTable\n"));

    netsnmp_table_set_add_indexes(ipTableAddress_table_set,ASN_OCTET_STR ,     /* index: ipAdEntAddr  ASN_IPADDRESS  ASN_OCTET_STR*/ 
    								ASN_INTEGER,  
    								0);
    DEBUGMSGTL(("initialize_table_ipAddressTable",
                "adding column types to table ipAddressTable\n"));
        
                                            
	netsnmp_table_set_multi_add_default_row(ipTableAddress_table_set,
											(int)COLUMN_IPADDRESSIFINDEX,  /* INDEX */
											ASN_INTEGER, 0, NULL, 0,
											0	
											);
                                                                                   
    /*
     * registering the table with the master agent 
     */
    /*
     * note: if you don't need a subhandler to deal with any aspects
     * of the request, change ipAddrTable_handler to "NULL" 
     */

    netsnmp_register_table_data_set(netsnmp_create_handler_registration
                                    ("ipAddressTable", NULL, 
                                     ipAddressTable_oid, ipAddressTable_oid_len,
                                     HANDLER_CAN_RWRITE), ipTableAddress_table_set, NULL);
      
    netsnmp_register_auto_data_table(ipTableAddress_table_set, NULL);
    
}




void CIpTableAddress::IpTableAddress_AddIp4Address(const std::string &strIpAddress, eInterfaceIndex eIfFace)
{
	
	mcTransportAddress tmpIPv4Addr;
	::stringToIpV4(&tmpIPv4Addr,(char *)strIpAddress.c_str(), eNetwork);	
	APIU32 networkIp4Address = tmpIPv4Addr.addr.v4.ip;
	
	// FTRACEINTO << "IpTableAddress_AddIp4Address  " << strIpAddress << " strIpAddress " << (int)eIfFace << " networkIp4Address " << networkIp4Address ;
		
	netsnmp_table_row  *newRow = CreateIpV4RowToIpTableAddress(eIfFace, networkIp4Address);	
	// Remove  row  	
	netsnmp_table_row  *row =  GetIpV4AddressRowByIndex(ipTableAddress_table_set, networkIp4Address);	
    if(NULL != row)
    {
    	// FTRACEINTO << "IpTableAddress_AddIp4Address removing previous row " << strIpAddress << " strIpAddress " << (int)eIfFace << " networkIp4Address " << networkIp4Address;
    	
        netsnmp_table_dataset_remove_and_delete_row(ipTableAddress_table_set, row);
    }

    APIU32 oldIpAddress = 0xFFFFFFFF;
     
    netsnmp_table_row  *origRow = NULL; 
    
    const InterfaceIpAddress* interfaceIpAddress = m_interfaceIndexIpAddressMapping.GetIpAddressEntryByInterface(eIfFace);
    
    if (interfaceIpAddress)
    {
    	if (interfaceIpAddress->IsIpV4NetworkAddressSet())
    	{
    		// FTRACEINTO << "IpTableAddress_AddIp4Addressfound old entry  with ipv4 row will be replaced "<<(int)eIfFace  << strIpAddress ;
			oldIpAddress = interfaceIpAddress->GetIpV4NetworkAddress();
			origRow =  GetIpV4AddressRowByIndex(ipTableAddress_table_set, oldIpAddress);
    	}

    }
        
    NetSnmpTableAddReplaceRow(ipTableAddress_table_set, origRow, newRow);
    
    m_interfaceIndexIpAddressMapping.SetIpv4NetworkAddress(eIfFace, networkIp4Address);
    
}



void CIpTableAddress::IpTableAddress_AddIp6Address(const std::string &strIpAddress, eInterfaceIndex eIfFace)
{
    mcTransportAddress tmpIPv6Addr;
    ::stringToIpV6(&tmpIPv6Addr,strIpAddress.c_str());
    const byte *ipv6Address = (const byte *)tmpIPv6Addr.addr.v6.ip;

    
    IpTableAddress_AddIp6Address(ipv6Address, eIfFace);
    
}

void CIpTableAddress::IpTableAddress_AddIp6Address(const byte *ipv6Address , eInterfaceIndex eIfFace)

{
    FTRACEINTO << "IpTableAddress_AddIp6Address " <<  " eIfFace " << (int)eIfFace ;

	netsnmp_table_row  *newRow = CreateIpV6RowToIpTableAddress(eIfFace, ipv6Address);	
	// Remove  row  	
	netsnmp_table_row  *row =  GetIpV6AddressRowByIndex(ipTableAddress_table_set, ipv6Address);	
    if(NULL != row)
    {
    	// FTRACEINTO << "IpTableAddress_AddIp6Address removing previous row " << (int)eIfFace ;
    	
        netsnmp_table_dataset_remove_and_delete_row(ipTableAddress_table_set, row);
    }

    
    APIU8 oldIpAddress[IPV6_ADDRESS_BYTES_LEN];
    
    netsnmp_table_row  *origRow = NULL; 
    
    const InterfaceIpAddress* interfaceIpAddress = m_interfaceIndexIpAddressMapping.GetIpAddressEntryByInterface(eIfFace);
    
    if (interfaceIpAddress)
    {    	
    	if (interfaceIpAddress->IsIpV6AddressSet())
    	{
    		// FTRACEINTO << "IpTableAddress_AddIp6Address found old entry will be replaced "<<(int)eIfFace ;
    		memcpy(oldIpAddress, interfaceIpAddress->GetIpV6Address(), (size_t)IPV6_ADDRESS_BYTES_LEN);
			
			origRow =  GetIpV6AddressRowByIndex(ipTableAddress_table_set, oldIpAddress);
    	}
    }
        
    NetSnmpTableAddReplaceRow(ipTableAddress_table_set, origRow, newRow);
    m_interfaceIndexIpAddressMapping.SetIpV6Address(eIfFace, ipv6Address);    
}



netsnmp_table_row* CIpTableAddress::GetIpV4AddressRowByIndex(netsnmp_table_data_set *dataTable,
															APIU32	ipv4Address) 

{
    netsnmp_table_row *row = NULL;
    netsnmp_variable_list *idxs = NULL;
    
    int ipAddressType =  (int)INETADDRESSTYPE_IPV4;
    
    void *res = snmp_varlist_add_variable(&idxs,
                                          NULL,
                                          0,
                                          ASN_INTEGER,
                                          (u_char*)&ipAddressType,
                                          sizeof(ipAddressType));
    
    if (!res)
    {
    	FTRACEINTO << "Failed add ipAddressType " << (int)ipAddressType;

    	return NULL;
    }
    res = snmp_varlist_add_variable(&idxs,
                                          NULL,
                                          0,
                                          ASN_OCTET_STR,
                                          (u_char*)&ipv4Address,
                                          sizeof(ipv4Address));

    
    if (!res)
    {
    	FTRACEINTO << "Failed add " << (int)ipv4Address;
    	return NULL;
    }
    
    row = netsnmp_table_data_get(dataTable->table, idxs);
    
    snmp_free_var(idxs);    
    return row;
	
}

netsnmp_table_row* CIpTableAddress::GetIpV6AddressRowByIndex(netsnmp_table_data_set *dataTable,
                                             const byte	*ipv6Address) 

{
    netsnmp_table_row *row = NULL;
    netsnmp_variable_list *idxs = NULL;
    int ipAddressType =  (int)INETADDRESSTYPE_IPV6;
    
    void *res = snmp_varlist_add_variable(&idxs,
                                          NULL,
                                          0,
                                          ASN_INTEGER,
                                          (u_char*)&ipAddressType,
                                          sizeof(ipAddressType));
    
    if (!res)
    { 
    	FTRACEINTO << "Failed add ipAddressType " << (int)ipAddressType;
    	return NULL;
    }
    res = snmp_varlist_add_variable(&idxs,
                                          NULL,
                                          0,
                                          ASN_OCTET_STR,
                                          (const byte *)ipv6Address,
                                          (int)IPV6_ADDRESS_BYTES_LEN);

    
    if (!res)
    {
    	FTRACEINTO << "Failed add the ipv6 address ";
    	return NULL;
    }
    
    row = netsnmp_table_data_get(dataTable->table, idxs);
    
    snmp_free_var(idxs);    
    return row;
	
}



netsnmp_table_row* CIpTableAddress::CreateIpV4RowToIpTableAddress(eInterfaceIndex eIfFace, APIU32 networkIp4Address)
{
	netsnmp_table_row *row = netsnmp_create_table_data_row();
	int ipAddressType =  (int)INETADDRESSTYPE_IPV4;
	netsnmp_table_row_add_index(row, ASN_INTEGER,  (const char *)&ipAddressType, sizeof(ipAddressType));

	// TODO add ipv4 address
	netsnmp_table_row_add_index(row, ASN_OCTET_STR,  &networkIp4Address, sizeof(APIU32));
    
    netsnmp_set_row_column(row, COLUMN_IPADDRESSIFINDEX, ASN_INTEGER,
                          (const char *)&eIfFace ,sizeof(eInterfaceIndex) );
    
    return row;
	
}

netsnmp_table_row* CIpTableAddress::CreateIpV6RowToIpTableAddress(eInterfaceIndex eIfFace, const byte* ipv6Adrress)
{	
	netsnmp_table_row *row = netsnmp_create_table_data_row();
	int ipAddressType =  (int)INETADDRESSTYPE_IPV6;
	netsnmp_table_row_add_index(row, ASN_INTEGER,  (const char *)&ipAddressType, sizeof(ipAddressType));

	netsnmp_table_row_add_index(row, ASN_OCTET_STR,  (const byte *)ipv6Adrress, (int)IPV6_ADDRESS_BYTES_LEN);
    
    netsnmp_set_row_column(row, COLUMN_IPADDRESSIFINDEX, ASN_INTEGER,
                          (const char *)&eIfFace ,sizeof(eInterfaceIndex) );

    return row;
}

