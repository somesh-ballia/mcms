
//	FastIndexTable.h
//	Written by Mikhail Karasik
#ifndef __FAST_INDEX_TABLE__
#define __FAST_INDEX_TABLE__

//	Include files:
//----------------
//	Macros:
//---------
#define FastIndexTableAddIndex(a, b, c) FastIndexTableAddIndexExt(a, b, #b, c)
// Structures:
//------------
typedef struct {
	unsigned long		index;
	unsigned long		value;
	char				indexName[64];
}fastIndexTableEntryStruct;

typedef struct {
	unsigned long						firstIndex;
	unsigned long						numOfIndexes;
	unsigned long						filler1;
	unsigned long						filler2;

	fastIndexTableEntryStruct	array[1];

}fastIndexTableStruct;

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
	unsigned long						numberOfIndexes);

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
	fastIndexTableStruct		*pFastTable);

//---------------------------------------------------------------------------
//
//	Function name:	FastIndexTableAddIndexExt
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
		char   						*pIndexName,
		unsigned long				value);

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
		unsigned long				index,
		unsigned long				value);

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
		char						*pIndexName);

#endif //__FAST_INDEX_TABLE__
