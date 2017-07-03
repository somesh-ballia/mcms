// LayoutSharedMemoryMap.cpp: implementation of the CLayoutSharedMemoryMap class.
//
//Change Layout Improvement - Layout Shared Memory (CL-SM)
//////////////////////////////////////////////////////////////////////

#include "LayoutSharedMemoryMap.h"
#include "LibsCommonHelperFuncs.h"
#include "TraceStream.h"

/////////////////////////////////////////////////////////////////////////
// Construction/Destruction
/////////////////////////////////////////////////////////////////////////
CLayoutSharedMemoryMap::CLayoutSharedMemoryMap()
{
	m_pLayoutSharedMemoryTable = new SharedMemoryLayoutMap(SHARED_MEMORY_LAYOUT_NAME, 1, SHARED_MEMORY_LAYOUT_SIZE);
}
/////////////////////////////////////////////////////////////////////////
CLayoutSharedMemoryMap::~CLayoutSharedMemoryMap()
{
 // m_pLayoutSharedMemoryTable->Clean();
//  delete m_pLayoutSharedMemoryTable;
  POBJDELETE(m_pLayoutSharedMemoryTable);
}
//////////////////////////////////////////////////////////////////////////
int CLayoutSharedMemoryMap::Add(CLayoutEntry& Entry)
{

 int status = m_pLayoutSharedMemoryTable->Add(Entry);
 if(status != STATUS_OK)
 {
  PASSERT(status);
  //trace???
 }
 return status;
}

//////////////////////////////////////////////////////////////////////////
int CLayoutSharedMemoryMap::Get(DWORD id, CLayoutEntry& Entry)
{

 int status = m_pLayoutSharedMemoryTable->Get(id,Entry);
 if(status != STATUS_OK)
 {
  PASSERT(status);
  //trace???
 }
 return status;
}

//////////////////////////////////////////////////////////////////////////
int CLayoutSharedMemoryMap::Remove(DWORD id)
{
 if (NULL == m_pLayoutSharedMemoryTable)
 {
	PASSERT(101);
	return STATUS_FAIL;
 }

 int status = m_pLayoutSharedMemoryTable->Remove(id);

 if(status != STATUS_OK)
 {
  //PASSERT(1);
  	TRACEINTO << "CLayoutSharedMemoryMap::Remove - layout was not found for this party, hence could not be removed (probably was never added to shared memory.. (OK)";
 }

 return status;
}

//////////////////////////////////////////////////////////////////////////
int CLayoutSharedMemoryMap::Update(CLayoutEntry& Entry)
{
 int status = m_pLayoutSharedMemoryTable->Update(Entry);

 if(status != STATUS_OK)
 {
  PASSERT(status);
  //trace???
 }
 return status;

}

//////////////////////////////////////////////////////////////////////////
int CLayoutSharedMemoryMap::AddOrUpdate(CLayoutEntry& Entry)
{
 int status = m_pLayoutSharedMemoryTable->AddOrUpdate(Entry);

 if(status != STATUS_OK)
 {
  PASSERT(status);
  //trace???
 }
 return status;
}


/////////////////////////////////////////////////////////////////////
DWORD CLayoutSharedMemoryMap::GetNumOfEntries() const
{
	return  m_pLayoutSharedMemoryTable->GetNumOfEntries();
}


/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
///	CLayoutEntry
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
CLayoutEntry::CLayoutEntry()
{
    m_id = 0;
    m_confRsrcId = 0;
    m_partyRsrcId = 0;
    m_connectionId = 0;
    m_isChanged = FALSE;


   	int sizeOfChangeLayoutWithoutImages = sizeof(MCMS_CM_CHANGE_LAYOUT_S)-sizeof(MCMS_CM_IMAGE_PARAM_S*);
    memset(&m_changeLayoutParams, 0xff, sizeOfChangeLayoutWithoutImages);

   	int sizeOfImageParamArray = MAX_NUMBER_OF_CELLS_IN_LAYOUT*(sizeof(MCMS_CM_IMAGE_PARAM_S));
    memset(&(m_atImageParam[0]), 0xff, sizeOfImageParamArray);

}


/////////////////////////////////////////////////////////////////////////////
CLayoutEntry::CLayoutEntry(const CLayoutEntry &rHnd )
{
    m_id                = rHnd.m_id;
    m_confRsrcId 		= rHnd.m_confRsrcId;
    m_partyRsrcId 		= rHnd.m_partyRsrcId;
    m_connectionId		= rHnd.m_connectionId;
    m_isChanged			= rHnd.m_isChanged;

    SetChangeLayoutParams(rHnd);

}

/////////////////////////////////////////////////////////////////////////////
CLayoutEntry::~CLayoutEntry( )
{
}

/////////////////////////////////////////////////////////////////////////////
CLayoutEntry& CLayoutEntry::operator= (const CLayoutEntry &rHnd )
{
	if ( &rHnd == this )
		return *this;

    m_id                = rHnd.m_id;
    m_confRsrcId 		= rHnd.m_confRsrcId;
    m_partyRsrcId 		= rHnd.m_partyRsrcId;
    m_connectionId		= rHnd.m_connectionId;
    m_isChanged			= rHnd.m_isChanged;

    SetChangeLayoutParams(rHnd);

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
void CLayoutEntry::SetChangeLayoutParams(const CLayoutEntry& layoutEntry)
{
	int sizeOfChangeLayoutWithoutImages = sizeof(MCMS_CM_CHANGE_LAYOUT_S)-sizeof(MCMS_CM_IMAGE_PARAM_S*);
	memcpy(&m_changeLayoutParams, &layoutEntry.m_changeLayoutParams, sizeOfChangeLayoutWithoutImages);

	WORD numOfImages = CLibsCommonHelperFuncs::GetNumbSubImg(layoutEntry.m_changeLayoutParams.nLayoutType);
   	int sizeOfImageParamArray = numOfImages*(sizeof(MCMS_CM_IMAGE_PARAM_S));
	memcpy(&(m_atImageParam[0]), &(layoutEntry.m_atImageParam[0]), sizeOfImageParamArray);
}

/////////////////////////////////////////////////////////////////////////////
void CLayoutEntry::SetChangeLayoutParams(MCMS_CM_CHANGE_LAYOUT_S& tChangeLayoutStruct)
{
	int sizeOfChangeLayoutWithoutImages = sizeof(MCMS_CM_CHANGE_LAYOUT_S)-sizeof(MCMS_CM_IMAGE_PARAM_S*);
	memcpy(&m_changeLayoutParams, &tChangeLayoutStruct, sizeOfChangeLayoutWithoutImages);

	WORD numOfImages = CLibsCommonHelperFuncs::GetNumbSubImg(tChangeLayoutStruct.nLayoutType);
   	int sizeOfImageParamArray = numOfImages*(sizeof(MCMS_CM_IMAGE_PARAM_S));
	memcpy(&(m_atImageParam[0]), &(tChangeLayoutStruct.atImageParam[0]), sizeOfImageParamArray);
}





