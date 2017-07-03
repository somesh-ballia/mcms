/*
 * Note: this file originally auto-generated by mib2c using
 *        : mib2c.create-dataset.conf,v 5.4 2004/02/02 19:06:53 rstory Exp $
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
#include "ipAddrTable.h"
#include "SystemFunctions.h"
#include "SnmpTableUtils.h"
using namespace std;



netsnmp_table_row* CreateRowToIpTable(eInterfaceIndex eIfFace, DWORD ipAddress);




Netsnmp_Node_Handler ipAddrTable_handler;
netsnmp_table_data_set *ipTable_table_set;
/** Initialize the ipAddrTable table by defining its contents and how it's structured */
void IpTable_Init(void)
{
    static oid      ipAddrTable_oid[] = { 1, 3, 6, 1, 2, 1, 4, 20 };
    size_t          ipAddrTable_oid_len = OID_LENGTH(ipAddrTable_oid);
   

    /*
     * create the table structure itself 
     */
    ipTable_table_set = netsnmp_create_table_data_set("ipAddrTable");

    /*
     * comment this out or delete if you don't support creation of new rows 
     */
    ipTable_table_set->allow_creation = 1;

    /***************************************************
     * Adding indexes
     */
    DEBUGMSGTL(("initialize_table_ipAddrTable",
                "adding indexes to table ipAddrTable\n"));
    netsnmp_table_set_add_indexes(ipTable_table_set, ASN_IPADDRESS,     /* index: ipAdEntAddr */
                                  0);

    DEBUGMSGTL(("initialize_table_ipAddrTable",
                "adding column types to table ipAddrTable\n"));
    netsnmp_table_set_multi_add_default_row(ipTable_table_set,
                                            //   COLUMN_IPADENTADDR,
                                            //ASN_IPADDRESS, 0, NULL, 0,
                                            COLUMN_IPADENTIFINDEX,
                                            ASN_INTEGER, 0, NULL, 0,
                                            COLUMN_IPADENTNETMASK,
                                            ASN_IPADDRESS, 0, NULL, 0,
                                            COLUMN_IPADENTBCASTADDR,
                                            ASN_INTEGER, 0, NULL, 0,
                                            COLUMN_IPADENTREASMMAXSIZE,
                                            ASN_INTEGER, 0, NULL, 0, 0);

    /*
     * registering the table with the master agent 
     */
    /*
     * note: if you don't need a subhandler to deal with any aspects
     * of the request, change ipAddrTable_handler to "NULL" 
     */
    netsnmp_register_table_data_set(netsnmp_create_handler_registration
                                    ("ipAddrTable", ipAddrTable_handler,
                                     ipAddrTable_oid, ipAddrTable_oid_len,
                                     HANDLER_CAN_RWRITE), ipTable_table_set, NULL);
    
    netsnmp_register_auto_data_table(ipTable_table_set, NULL);
}

/** handles requests for the ipAddrTable table, if anything else needs to be done */
int
ipAddrTable_handler(netsnmp_mib_handler *handler,
                    netsnmp_handler_registration *reginfo,
                    netsnmp_agent_request_info *reqinfo,
                    netsnmp_request_info *requests)
{
    /*
     * perform anything here that you need to do.  The requests have
     * already been processed by the master table_dataset handler, but
     * this gives you chance to act on the request in some other way
     * if need be. 
     */
    return SNMP_ERR_NOERROR;
}





/*-----------------------------------------------------------------------
   Interface -> IpAddress map.
-----------------------------------------------------------------------*/
typedef map<eInterfaceIndex, DWORD> CInterfaceIpAddressMap;

CInterfaceIpAddressMap& GetInterfaceIpAddressMap()
{
    static map<eInterfaceIndex, DWORD> interfaceIpAddressMap;
    return interfaceIpAddressMap;
}
bool GetIpAddressByInterface(eInterfaceIndex eIfFace, DWORD &outIp)
{
    bool isFound = false;
    CInterfaceIpAddressMap &map = GetInterfaceIpAddressMap();
    if(map.find(eIfFace) != map.end())
    {
        outIp = map[eIfFace];
        isFound = true;
    }
    return isFound;
}
void SetIpAddressByInterface(eInterfaceIndex eIfFace, DWORD ipAddress)
{
    CInterfaceIpAddressMap &map = GetInterfaceIpAddressMap();
    map[eIfFace] = ipAddress;
}
/*-----------------------------------------------------------------------
   Interface -> IpAddress map.
-----------------------------------------------------------------------*/



void IpTable_AddRow(const std::string & strIpAddress ,  eInterfaceIndex eIfFace)
{
    DWORD newIpAddress = SystemIpStringToDWORD(strIpAddress.c_str());
    
    netsnmp_table_row *newRow = CreateRowToIpTable(eIfFace, newIpAddress);

    DWORD oldIpAddress = 0xFFFFFFFF;
    GetIpAddressByInterface(eIfFace, oldIpAddress);

    NetSnmpTableRemoveRowByIndex(ipTable_table_set, newIpAddress, ASN_IPADDRESS);
    
    NetSnmpTableReplaceRowByIndex(ipTable_table_set, oldIpAddress, newRow, ASN_IPADDRESS);

    SetIpAddressByInterface(eIfFace, newIpAddress);
}

netsnmp_table_row* CreateRowToIpTable(eInterfaceIndex eIfFace, DWORD address)
{
    netsnmp_table_row *row = netsnmp_create_table_data_row();
    netsnmp_table_row_add_index(row, ASN_IPADDRESS, &address, sizeof(DWORD));
    
    netsnmp_set_row_column(row,COLUMN_IPADENTIFINDEX, ASN_INTEGER,
                          (const char *)&eIfFace ,sizeof(eInterfaceIndex) );
    return row;
}
