#include "UniqueIndex.h"
#include "Trace.h"


CUniqueIndex::CUniqueIndex(const char *sharedMemoryName)
        :CShardMemory(sharedMemoryName, 1, sizeof(DWORD))
{
    if (m_status == STATUS_OK)
	{	
		m_ptrUniqueIndex = (DWORD*)m_pView;
		if (m_first)
		{
			*m_ptrUniqueIndex = 1;
		}
	}
    else
    {
        m_ptrUniqueIndex = NULL;
        FPASSERTMSG(m_status,"Shared memory allocation failed");
    }
}

CUniqueIndex::~CUniqueIndex()
{
}

DWORD CUniqueIndex::GetInc()
{
    if(NULL == m_ptrUniqueIndex)
    {
        return 0xFFFFFFFF;
    }

    Lock();
    
    DWORD currentIndex = *m_ptrUniqueIndex;
    (*m_ptrUniqueIndex)++;
    
    Unlock();
    
    return currentIndex;
}
