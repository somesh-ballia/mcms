// LayoutSharedMemoryMap.h: interface for the CLayoutSharedMemoryMap class.
//
//Change Layout Improvement - Layout Shared Memory (CL-SM)
//////////////////////////////////////////////////////////////////////

#ifndef __LAYOUT_SHARED_MEMORY_MAP_H__
#define __LAYOUT_SHARED_MEMORY_MAP_H__


#include "SharedHeader.h"
#include "SharedMemory.h"
#include "SharedMemoryMap.h"
#include "Trace.h"
#include "AllocateStructs.h"
#include "PObject.h"
#include "VideoStructs.h"


#define MAX_VIDEO_PARTIES_ON_RMX 800		// for layout shared memory allocation - maximum 200 parties per MPMRx card X 4 cards in RMX4000

//////////////////////////////////////////////////////////////

class CLayoutEntry
{
public:
	CLayoutEntry();
	~CLayoutEntry();
	CLayoutEntry(const CLayoutEntry &rHnd );
	CLayoutEntry& operator= (const CLayoutEntry &rHnd );

	DWORD 	GetConfRsrcId() 	{ return m_confRsrcId; }
	DWORD 	GetPartyRsrcId() 	{ return m_partyRsrcId; }
	DWORD 	GetConnectionId() 	{ return m_connectionId; }
	BOOL 	IsChanged() 		{ return m_isChanged; }

	void 	SetConfRsrcId(DWORD confRsrcId) 	{ m_confRsrcId = confRsrcId; }
	void 	SetPartyRsrcId(DWORD partyRsrcId) 	{ m_partyRsrcId = partyRsrcId; }
	void 	SetConnectionId(DWORD connectionId) { m_connectionId = connectionId; }
	void 	SetIsChanged(BOOL isChanged) 		{ m_isChanged = isChanged; }

	DWORD	GetLayoutId()	{ return m_id; }
	void	SetLayoutId(DWORD id)	{ m_id = id; }

	MCMS_CM_CHANGE_LAYOUT_S 	GetChangeLayoutParams() 	{ return m_changeLayoutParams; }
	MCMS_CM_IMAGE_PARAM_S*		GetImageParam()				{ return m_atImageParam;}
	void SetChangeLayoutParams(const CLayoutEntry& layoutEntry);
	void SetChangeLayoutParams(MCMS_CM_CHANGE_LAYOUT_S& tChangeLayoutStruct);


	DWORD                   m_id;	// Layout ID, currently party resource ID

private:
	DWORD					m_confRsrcId;
    DWORD					m_partyRsrcId;
    DWORD					m_connectionId;
    BOOL					m_isChanged;

    MCMS_CM_CHANGE_LAYOUT_S		m_changeLayoutParams;
    MCMS_CM_IMAGE_PARAM_S		m_atImageParam[MAX_NUMBER_OF_CELLS_IN_LAYOUT];

};


/////////////////////////// typedef CSharedMemoryMap///////////////////////////
typedef CSharedMemoryMap<CLayoutEntry> SharedMemoryLayoutMap;

///////////////////////////////////////////////////////////////////////////////////////
#define SHARED_MEMORY_LAYOUT_NAME "SharedMemoryLayoutTable"
#define SHARED_MEMORY_LAYOUT_SIZE MAX_VIDEO_PARTIES_ON_RMX



///////////////////////////////////////////////////////////////////////////////////////
class CLayoutSharedMemoryMap : public CPObject
{
CLASS_TYPE_1(CLayoutSharedMemoryMap,CPObject)
public:
	CLayoutSharedMemoryMap();
	virtual const char* NameOf() const { return "CLayoutSharedMemoryMap";}
	virtual ~CLayoutSharedMemoryMap();

	int Add(CLayoutEntry& pEntry);
    int Remove(DWORD id);
    int Update(CLayoutEntry& pEntry);
    int AddOrUpdate(CLayoutEntry& pEntry);
	int Get(DWORD id, CLayoutEntry& Entry);


	DWORD GetNumOfEntries() const;

private:

	SharedMemoryLayoutMap* m_pLayoutSharedMemoryTable;

};




#endif 	//__LAYOUT_SHARED_MEMORY_MAP_H__
