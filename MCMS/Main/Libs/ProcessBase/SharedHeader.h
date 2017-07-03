// SharedHeader.h: interface for the CSharedHeader struct.
//
//////////////////////////////////////////////////////////////////////


#if !defined(_SHAREDHEADER_H__)
#define _SHAREDHEADER_H__

#include "DataTypes.h"


#define EMPTY_ENTRY 0xffffffff

struct CSharedHeader
{
	DWORD m_maxEntries;
	DWORD m_numEntries;
};

struct CSharedHeaderQueue
{
	DWORD 	m_maxEntries;
	DWORD 	m_numEntries;
	DWORD	m_startQueue;
};

struct CSharedHeaderDw
{
	DWORD m_maxEntries;
	DWORD m_numEntries;
	DWORD m_dwData;
};

typedef enum
{
	eSharedMem_StatusOk = 0,
	eSharedMem_MemoryNotCreated,
	eSharedMem_MemoryNotFound,
	eSharedMem_EntryNotFound,
	eSharedMem_BadParameter,
	eSharedMem_NoPermissions,
	eSharedMem_MemoryIsFull,
	eSharedMem_MemoryIsEmpty,
} ESharedMemStatus;

#endif // _SHAREDHEADER_H__


