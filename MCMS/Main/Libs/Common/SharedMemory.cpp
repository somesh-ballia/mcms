// SharedMemory.cpp: 
//
//////////////////////////////////////////////////////////////////////

#include "SystemFunctions.h"
#include "SharedMemory.h"
#include "Semaphore.h"

//////////////////////////////////////////////////////////////////////
CShardMemory::CShardMemory(const char* name,
						   BYTE permisssion,
						   DWORD size)
:m_name(name),
 m_permisssion(permisssion),
 m_fileMap(0),
 m_first(false),
 m_pView(NULL),
 m_size(size)
{
	std::string semName(name);
	m_status = CreateSemaphore(&m_semaphore,semName,TRUE);
	if (STATUS_OK == m_status)
	{
		Lock();
		m_status = SystemAllocateSharedMemory(m_fileMap,
										&m_pView,
										m_size,
										m_name,
										m_first);
		Unlock();
	}
}

//////////////////////////////////////////////////////////////////////
CShardMemory::~CShardMemory()
{
	Lock();
	if (STATUS_OK == m_status && (m_fileMap>0) )
		SystemFreeSharedMemory((void *)m_pView,m_fileMap);
	Unlock();

	//RemoveSemaphore(m_semaphore);
}

//////////////////////////////////////////////////////////////////////
void CShardMemory::Lock() const
{
	FPASSERT(LockSemaphore(m_semaphore));
}

//////////////////////////////////////////////////////////////////////
void CShardMemory::Unlock() const
{
	FPASSERT(UnlockSemaphore(m_semaphore));
}
