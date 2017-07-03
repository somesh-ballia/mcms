//IndicationIconSharedMemoryMap.h: interface for the CIndicationIconSharedMemoryMap class.
//
//Indication Icon Change Improvement - Indication Icon Shared Memory (CL-SM)
//////////////////////////////////////////////////////////////////////

#ifndef __INDICATION_ICON_SHARED_MEMORY_MAP_H__
#define __INDICATION_ICON_SHARED_MEMORY_MAP_H__


#include "SharedHeader.h"
#include "SharedMemory.h"
#include "SharedMemoryMap.h"
#include "Trace.h"
#include "AllocateStructs.h"
#include "PObject.h"
#include "VideoStructs.h"


#define MAX_VIDEO_PARTIES_ON_RMX 800		// for Indication Icon Change shared memory allocation - maximum 200 parties per MPMRx card X 4 cards in RMX4000

//////////////////////////////////////////////////////////////

class CIndicationIconEntry
{
public:
	CIndicationIconEntry();
	~CIndicationIconEntry();
	CIndicationIconEntry(const CIndicationIconEntry &rHnd );
	CIndicationIconEntry& operator= (const CIndicationIconEntry &rHnd );

	DWORD 	GetConfRsrcId() 	{ return m_confRsrcId; }
	DWORD 	GetPartyRsrcId() 	{ return m_partyRsrcId; }
	DWORD 	GetConnectionId() 	{ return m_connectionId; }
	BOOL 	IsChanged() 		{ return m_isChanged; }

	void 	SetConfRsrcId(DWORD confRsrcId) 	{ m_confRsrcId = confRsrcId; }
	void 	SetPartyRsrcId(DWORD partyRsrcId) 	{ m_partyRsrcId = partyRsrcId; }
	void 	SetConnectionId(DWORD connectionId) { m_connectionId = connectionId; }
	void 	SetIsChanged(BOOL isChanged) 		{ m_isChanged = isChanged; }

	DWORD	GetIndicationIconId()	{ return m_id; }
	void	SetIndicationIconId(DWORD id)	{ m_id = id; }

	ICONS_DISPLAY_S 	GetIndicationIconParams() 	{ return m_indicationIconParams; }
	void SetIndicationIconParams(const CIndicationIconEntry& indicationIconEntry);
	void SetIndicationIconParams(const ICONS_DISPLAY_S& tindicationIconStruct);


	DWORD                   m_id;	// Indication Icon id, currently party resource ID

private:
	DWORD					m_confRsrcId;
    DWORD					m_partyRsrcId;
    DWORD					m_connectionId;
    BOOL					m_isChanged;

    ICONS_DISPLAY_S		m_indicationIconParams;
};


/////////////////////////// typedef CSharedMemoryMap///////////////////////////
typedef CSharedMemoryMap<CIndicationIconEntry> SharedMemoryIndicationIconMap;

///////////////////////////////////////////////////////////////////////////////////////
#define SHARED_MEMORY_INDICATION_ICON_NAME "SharedMemoryIndicationIconTable"
#define SHARED_MEMORY_INDICATION_ICON_SIZE MAX_VIDEO_PARTIES_ON_RMX



///////////////////////////////////////////////////////////////////////////////////////
class CIndicationIconSharedMemoryMap : public CPObject
{
CLASS_TYPE_1(CIndicationIconSharedMemoryMap,CPObject)
public:
	CIndicationIconSharedMemoryMap();
	virtual const char* NameOf() const { return "CIndicationIconSharedMemoryMap";}
	virtual ~CIndicationIconSharedMemoryMap();

	int Add(CIndicationIconEntry& pEntry);
    int Remove(DWORD id);
    int Update(CIndicationIconEntry& pEntry);
    int AddOrUpdate(CIndicationIconEntry& pEntry);
	int Get(DWORD id, CIndicationIconEntry& Entry);


	DWORD GetNumOfEntries() const;

private:

	SharedMemoryIndicationIconMap* m_pIndicationIconSharedMemoryTable;

};




#endif 	//__LAYOUT_SHARED_MEMORY_MAP_H__
