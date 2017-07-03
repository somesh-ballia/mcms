#ifndef __SNMP_TABLE_UTILS_H__
#define __SNMP_TABLE_UTILS_H__

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>




void NetSnmpTableReplaceRowByIndex(netsnmp_table_data_set *dataTable,
                                   int index,
                                   netsnmp_table_row *newRow,
                                   int indexType);  // ASN_INTEGER, ASN_IPADDRESS

void NetSnmpTableRemoveRowByIndex(netsnmp_table_data_set *dataTable,
                                  int index,
                                  int indexType);  // ASN_INTEGER, ASN_IPADDRESS

void NetSnmpTableAddReplaceRow(netsnmp_table_data_set *dataTable,
                               netsnmp_table_row *origRow,
                               netsnmp_table_row * newRow);

int GetNetSnmpTableRowNumber(netsnmp_table_data_set *dataTable);


#endif // __SNMP_TABLE_UTILS_H__
