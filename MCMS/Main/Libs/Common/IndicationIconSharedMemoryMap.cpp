// LayoutSharedMemoryMap.cpp: implementation of the CIndicationIconSharedMemoryMap class.
//
//Change Layout Improvement - Layout Shared Memory (CL-SM)
//////////////////////////////////////////////////////////////////////

#include "IndicationIconSharedMemoryMap.h"
#include "LibsCommonHelperFuncs.h"
#include "TraceStream.h"

/////////////////////////////////////////////////////////////////////////
// Construction/Destruction
/////////////////////////////////////////////////////////////////////////
CIndicationIconSharedMemoryMap::CIndicationIconSharedMemoryMap()
{
	m_pIndicationIconSharedMemoryTable = new SharedMemoryIndicationIconMap(SHARED_MEMORY_INDICATION_ICON_NAME, 1, SHARED_MEMORY_INDICATION_ICON_SIZE);
}
/////////////////////////////////////////////////////////////////////////
CIndicationIconSharedMemoryMap::~CIndicationIconSharedMemoryMap()
{

  POBJDELETE(m_pIndicationIconSharedMemoryTable);
}
//////////////////////////////////////////////////////////////////////////
int CIndicationIconSharedMemoryMap::Add(CIndicationIconEntry& Entry)
{

 int status = m_pIndicationIconSharedMemoryTable->Add(Entry);
 if(status != STATUS_OK)
 {
  PASSERT(status);
  //trace???
 }
 return status;
}

//////////////////////////////////////////////////////////////////////////
int CIndicationIconSharedMemoryMap::Get(DWORD id, CIndicationIconEntry& Entry)
{

 int status = m_pIndicationIconSharedMemoryTable->Get(id,Entry);
 if(status != STATUS_OK)
 {
  PASSERT(status);
  //trace???
 }
 return status;
}

//////////////////////////////////////////////////////////////////////////
int CIndicationIconSharedMemoryMap::Remove(DWORD id)
{
 if (NULL == m_pIndicationIconSharedMemoryTable)
 {
	PASSERT(101);
	return STATUS_FAIL;
 }

 int status = m_pIndicationIconSharedMemoryTable->Remove(id);

 if(status != STATUS_OK)
 {
  //PASSERT(1);
  	TRACEINTO << "CIndicationIconSharedMemoryMap::Remove - layout was not found for this party, hence could not be removed (probably was never added to shared memory.. (OK)";
 }

 return status;
}

//////////////////////////////////////////////////////////////////////////
int CIndicationIconSharedMemoryMap::Update(CIndicationIconEntry& Entry)
{
 int status = m_pIndicationIconSharedMemoryTable->Update(Entry);

 if(status != STATUS_OK)
 {
  PASSERT(status);
  //trace???
 }
 return status;

}

//////////////////////////////////////////////////////////////////////////
int CIndicationIconSharedMemoryMap::AddOrUpdate(CIndicationIconEntry& Entry)
{
 int status = m_pIndicationIconSharedMemoryTable->AddOrUpdate(Entry);

 if(status != STATUS_OK)
 {
  PASSERT(status);
  //trace???
 }
 return status;
}


/////////////////////////////////////////////////////////////////////
DWORD CIndicationIconSharedMemoryMap::GetNumOfEntries() const
{
	return  m_pIndicationIconSharedMemoryTable->GetNumOfEntries();
}


/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
///	CIndicationIconEntry
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
CIndicationIconEntry::CIndicationIconEntry()
{
    m_id = 0;
    m_confRsrcId = 0;
    m_partyRsrcId = 0;
    m_connectionId = 0;
    m_isChanged = FALSE;

    memset(&m_indicationIconParams, 0xff, sizeof(ICONS_DISPLAY_S));
}


/////////////////////////////////////////////////////////////////////////////
CIndicationIconEntry::CIndicationIconEntry(const CIndicationIconEntry &rHnd )
{
    m_id                = rHnd.m_id;
    m_confRsrcId 		= rHnd.m_confRsrcId;
    m_partyRsrcId 		= rHnd.m_partyRsrcId;
    m_connectionId		= rHnd.m_connectionId;
    m_isChanged			= rHnd.m_isChanged;

    SetIndicationIconParams(rHnd);

}

/////////////////////////////////////////////////////////////////////////////
CIndicationIconEntry::~CIndicationIconEntry( )
{
}

/////////////////////////////////////////////////////////////////////////////
CIndicationIconEntry& CIndicationIconEntry::operator= (const CIndicationIconEntry &rHnd )
{
	if ( &rHnd == this )
		return *this;

    m_id                = rHnd.m_id;
    m_confRsrcId 		= rHnd.m_confRsrcId;
    m_partyRsrcId 		= rHnd.m_partyRsrcId;
    m_connectionId		= rHnd.m_connectionId;
    m_isChanged			= rHnd.m_isChanged;

    SetIndicationIconParams(rHnd);

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
void CIndicationIconEntry::SetIndicationIconParams(const CIndicationIconEntry& layoutEntry)
{
	memcpy(&m_indicationIconParams, &(layoutEntry.m_indicationIconParams), sizeof(ICONS_DISPLAY_S));
}

/////////////////////////////////////////////////////////////////////////////
void CIndicationIconEntry::SetIndicationIconParams(const ICONS_DISPLAY_S& tindicationIconStruct)
{
	memcpy(&m_indicationIconParams, &tindicationIconStruct, sizeof(ICONS_DISPLAY_S));

}





