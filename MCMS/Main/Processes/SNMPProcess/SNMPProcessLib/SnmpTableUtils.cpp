#include <vector>
using namespace std;


#include "SnmpTableUtils.h"




/*-----------------------------------------------------------------------
  Private Functions declarations
-----------------------------------------------------------------------*/

void NetSnmpTableAddReplaceRow(netsnmp_table_data_set *dataTable,
                               netsnmp_table_row *origRow,
                               netsnmp_table_row * newRow);
netsnmp_table_row* GetNetSnmpTableRowByIndex(netsnmp_table_data_set *dataTable,
                                             int index,
                                             int indexType);

void CleanNetSnmpTable(netsnmp_table_data_set *dataTable);









/*-----------------------------------------------------------------------
  Public Functions
-----------------------------------------------------------------------*/

///////////////////////////////////////////////////////////////////////////////
void NetSnmpTableReplaceRowByIndex(netsnmp_table_data_set *dataTable,
                                   int index,
                                   netsnmp_table_row *newRow,
                                   int indexType)
{
    netsnmp_table_row *origRow = GetNetSnmpTableRowByIndex(dataTable, index, indexType);
    NetSnmpTableAddReplaceRow(dataTable, origRow, newRow);
}









/*-----------------------------------------------------------------------
  Private Functions
-----------------------------------------------------------------------*/

///////////////////////////////////////////////////////////////////////////////
void NetSnmpTableRemoveRowByIndex(netsnmp_table_data_set *dataTable, int index, int indexType)
{
    netsnmp_table_row *row = GetNetSnmpTableRowByIndex(dataTable, index, indexType);
    if(NULL != row)
    {
        netsnmp_table_dataset_remove_and_delete_row(dataTable, row);
    }
}

///////////////////////////////////////////////////////////////////////////////
void CleanNetSnmpTable(netsnmp_table_data_set *dataTable)
{
//     netsnmp_table_data_set_storage *data;
//     netsnmp_table_dataset_delete_all_data (dataTable);
    
    vector<netsnmp_table_row*> rowsToClean;
    netsnmp_table_row *currentRow = netsnmp_table_data_set_get_first_row(dataTable);
    while(NULL != currentRow)
    {
        rowsToClean.push_back(currentRow);
        currentRow = netsnmp_table_data_set_get_next_row(dataTable, currentRow);
    }
    
    for(vector<netsnmp_table_row*>::iterator iTer = rowsToClean.begin();
        iTer != rowsToClean.end();
        iTer++)
    {
        currentRow = *iTer;
        netsnmp_table_dataset_remove_and_delete_row (dataTable, currentRow);
    }
}

///////////////////////////////////////////////////////////////////////////////
int GetNetSnmpTableRowNumber(netsnmp_table_data_set *dataTable)
{
    int rowNumber = 0;
    
    netsnmp_table_row *currentRow = netsnmp_table_data_set_get_first_row(dataTable);
    while(NULL != currentRow)
    {
        rowNumber++;
        currentRow = netsnmp_table_data_set_get_next_row(dataTable, currentRow);
    }

    return rowNumber;
}

///////////////////////////////////////////////////////////////////////////////
netsnmp_table_row* GetNetSnmpTableRowByIndex(netsnmp_table_data_set *dataTable,
                                             int index,
                                             int indexType)
{
    netsnmp_table_row *row = NULL;
    netsnmp_variable_list *idxs = NULL;
    
    void *res = snmp_varlist_add_variable(&idxs,
                                          NULL,
                                          0,
                                          indexType,
                                          (u_char*)&index,
                                          sizeof(index));
    if(NULL != res)
    {
		row = netsnmp_table_data_get(dataTable->table, idxs);
	}

    snmp_free_var(idxs);
    
    return row;
}

///////////////////////////////////////////////////////////////////////////////
void NetSnmpTableAddReplaceRow(netsnmp_table_data_set *dataTable,
                               netsnmp_table_row *origRow,
                               netsnmp_table_row * newRow)
{
    if(NULL != origRow)
    {
        netsnmp_table_dataset_replace_row(dataTable, origRow, newRow);
        netsnmp_table_dataset_delete_row(origRow);
    }
    else
    {
        netsnmp_table_dataset_add_row(dataTable, newRow);
    }
}

