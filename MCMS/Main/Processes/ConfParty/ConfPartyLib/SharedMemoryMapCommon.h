//SharedMemoryMapCommon
//
// Created on: Mar 21, 2011
//     Author: gshefer
#ifndef __SHARED_MEMORY_MAP_COMMON_H__
#define __SHARED_MEMORY_MAP_COMMON_H__

#include "AllocateStructs.h"
#include "SharedMemoryMap.h"



typedef CSharedMemoryMap<ConnToCardTableEntry> CSharedMemMap;

class CMatchResourceByUnit : public CSharedMemoryMap<ConnToCardTableEntry>::CSharedMemEntryCondition
{
public:
	CMatchResourceByUnit(WORD boardId, WORD subBoardId, WORD unitId)
	  :m_nBoardId(boardId), m_nSubBoardId(subBoardId), m_nUnitId(unitId) 
	  {;}
	
	virtual ~CMatchResourceByUnit(){;}
	
	virtual BOOL IsMatch(const ConnToCardTableEntry& candidate)
	{
		return (((m_nBoardId == (DWORD)-1) || (candidate.boardId == m_nBoardId)) &&
				((m_nSubBoardId == (DWORD)-1) ||(candidate.subBoardId == m_nSubBoardId)) &&
				((m_nUnitId == (DWORD)-1) ||(candidate.unitId == m_nUnitId)) );
	}
private:
	DWORD m_nBoardId;
	DWORD m_nSubBoardId;
	DWORD m_nUnitId;
};

#endif //__SHARED_MEMORY_MAP_COMMON_H__
