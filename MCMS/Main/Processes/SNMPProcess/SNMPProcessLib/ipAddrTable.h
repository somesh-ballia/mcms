/*
 * Note: this file originally auto-generated by mib2c using
 *  : mib2c.create-dataset.conf,v 5.4 2004/02/02 19:06:53 rstory Exp $
 */
#ifndef IPADDRTABLE_H
#define IPADDRTABLE_H
#include <iostream>
#include "ifTable.h"
/*
 * function declarations 
 */

void IpTable_Init();
void IpTable_AddRow(const std::string & IpAddress ,  eInterfaceIndex eIfFace);


// void            init_ipAddrTable(void);
// void            initialize_table_ipAddrTable(void);

// void AddRowToIpTable(const std::string & IpAddress ,  eInterfaceIndex eIfFace);

/*
 * column number definitions for table ipAddrTable 
 */
#define COLUMN_IPADENTADDR		1
#define COLUMN_IPADENTIFINDEX		2
#define COLUMN_IPADENTNETMASK		3
#define COLUMN_IPADENTBCASTADDR		4
#define COLUMN_IPADENTREASMMAXSIZE		5
#endif                          /* IPADDRTABLE_H */
