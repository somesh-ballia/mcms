//	FastIndexTable.c
//	Written by Mikhail Karasik

//	Include files:
//----------------
#include <XmlErrors.h>
#include <FastIndexTable.h>

#include <XmlPrintLog.h>
#include <stdlib.h>
#include <string.h>

//	Macros:
//---------
#ifdef SRC
#undef SRC
#endif

#define SRC	"FastIndexTable"

// Global variables:
//------------------
int    bIgnoreInvalidIndexPrint    = 0;

// Routines:
//---------

//---------------------------------------------------------------------------
//
//	Function name:	CreateFastIndexTable
//
//	Description:	Allocate memory for table
//					Initialize all relevant data
//
//	Return code:
//			0				- success
//			ESOFTWARE		- error
//
//---------------------------------------------------------------------------

int	CreateFastIndexTable(
	fastIndexTableStruct		**ppFastTable,
	unsigned long						firstIndex,
	unsigned long						numberOfIndexes)
{
	fastIndexTableStruct		*pFastTable;
	unsigned long						tableMemorySize;

	if (!ppFastTable) {
		XmlPrintLog("!ppFastTable", PERR);
		return E_XML_INVALID_PARAMETER;
	}

	*ppFastTable	= NULL;

	tableMemorySize	= numberOfIndexes * sizeof(fastIndexTableEntryStruct) + sizeof(fastIndexTableStruct);

	pFastTable		= (fastIndexTableStruct*)calloc(
		1, 
		tableMemorySize);

	if (!pFastTable) {
		XmlPrintLog("calloc failed", 0);
		return E_XML_INVALID_PARAMETER;
	}

	memset(
		pFastTable, 
		0, 
		tableMemorySize);

	pFastTable->firstIndex		= firstIndex;
	pFastTable->numOfIndexes	= numberOfIndexes;

	*ppFastTable	= pFastTable;

	return 0;
}//CreateFastIndexTable

//---------------------------------------------------------------------------
//
//	Function name:	ReleaseFastIndexTable
//
//	Description:	free table
//
//	Return code:
//			0				- success
//			ESOFTWARE		- received NULL pointer
//
//---------------------------------------------------------------------------

int	ReleaseFastIndexTable(
	fastIndexTableStruct		*pFastTable)
{
	if (!pFastTable)
		return E_XML_INVALID_PARAMETER;

	free(pFastTable);

	return 0;
}//ReleaseFastIndexTable

//---------------------------------------------------------------------------
//
//	Function name:	FastIndexTableAddIndex
//
//	Description:	Add new index and value to fast table
//
//	Return code:
//			0				- success
//			ESOFTWARE		- invalid index or pointer to table is NULL
//			DUPLICATEID		- same index already in table index wasn't added
//
//---------------------------------------------------------------------------

int	FastIndexTableAddIndexExt(
		fastIndexTableStruct		*pFastTable,
		unsigned long				index,
		char						*pIndexName,
		unsigned long				value)
{

	unsigned long						entry;
	fastIndexTableEntryStruct	*pEntry;

	if (!pFastTable) {
		XmlPrintLog("!pFastTable index = 0x%X", PERR, index);
		return E_XML_INVALID_PARAMETER;
	}

	entry	= index - pFastTable->firstIndex;

	if(entry >= pFastTable->numOfIndexes) {
		XmlPrintLog("invalid index = 0x%X", PERR, index);
		return E_XML_INVALID_PARAMETER;
	}

	pEntry = &pFastTable->array[entry];

	if (pEntry->index)
		return E_XML_INVALID_PARAMETER;

	pEntry->index	= index;
	pEntry->value	= value;
	
	strncpy(pEntry->indexName, pIndexName, sizeof(pEntry->indexName)-1);
	pEntry->indexName[sizeof(pEntry->indexName)-1] = '\0';

	return 0;

}//FastIndexTableAddIndex

//---------------------------------------------------------------------------
//
//	Function name:	FastIndexTableDeleteIndex
//
//	Description:	delete index from fast table
//
//	Return code:
//			0				- success
//			ESOFTWARE		- invalid index or pointer to table is NULL
//
//---------------------------------------------------------------------------

int	FastIndexTableDeleteIndex(
		fastIndexTableStruct		*pFastTable,
		unsigned long						index,
		unsigned long						value)
{

	unsigned long						entry;
	fastIndexTableEntryStruct	*pEntry;

	if (!pFastTable) {
		XmlPrintLog("!pFastTable index = 0x%X", PERR, index);
		return E_XML_INVALID_PARAMETER;
	}

	entry	= index - pFastTable->firstIndex;

	if(entry >= pFastTable->numOfIndexes) {
		XmlPrintLog("invalid index = 0x%X", PERR, index);
		return E_XML_INVALID_PARAMETER;
	}

	pEntry = &pFastTable->array[entry];

	pEntry->index	= 0;
	pEntry->value	= 0;

	memset(
		&pEntry->indexName[0],
		0,
		64);
		
	return 0;
}//FastIndexTableDeleteIndex

//---------------------------------------------------------------------------
//
//	Function name:	FastIndexTableGetByIndex
//
//	Description:	get index from fast table
//
//	Return code:
//			0				- success
//			ESOFTWARE		- invalid index or one of pointers is NULL
//								in this case *pValue = 0
//
//---------------------------------------------------------------------------

int	FastIndexTableGetByIndex(
		fastIndexTableStruct		*pFastTable,
		unsigned long				index,
		unsigned long				*pValue,
		char						*pIndexName)
{

	unsigned long						entry;

	if (!pFastTable) {
		XmlPrintLog("!pFastTable index = 0x%X", PERR, index);
		return E_XML_INVALID_PARAMETER;
	}

	if (!pValue) {
		XmlPrintLog("!pValue index = 0x%X", index);
		return E_XML_INVALID_PARAMETER;
	}

	*pValue = 0;	

	entry	= index - pFastTable->firstIndex;

	if(entry >= pFastTable->numOfIndexes) {
        if (!bIgnoreInvalidIndexPrint)
		  XmlPrintLog("invalid index = 0x%X index name 0x%s", PERR, index, pIndexName);
		return E_XML_INVALID_PARAMETER;
	}

	if (pFastTable->array[entry].index == index) {
		
		*pValue	= pFastTable->array[entry].value;
		strcpy(pIndexName, &pFastTable->array[entry].indexName[0]);
	}
	else {
		XmlPrintLog("not init index = 0x%X pTable = 0x%X", PERR, index, pFastTable);
		return E_XML_INVALID_PARAMETER;
	}
 
	return 0;
}//FastIndexTableGetByIndex
