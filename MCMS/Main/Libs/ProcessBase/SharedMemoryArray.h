// SharedMemoryArray.h

#ifndef SHARED_MEMORY_ARRAY_H_
#define SHARED_MEMORY_ARRAY_H_

#include "SharedMemory.h"
#include "DataTypes.h"
#include "SharedHeader.h"

template <class ENTRY> class CSharedMemoryArray : public CShardMemory
{
	friend class CTestSharedMemory;
public:
	CSharedMemoryArray(const char* name,
					 BYTE permisssion,
					 DWORD maxEntries)
		:CShardMemory(name,
					  permisssion,
					  sizeof(ENTRY) * maxEntries + sizeof(CSharedHeader))
	{		
		if (m_status == STATUS_OK)
		{
			m_pHeader = (CSharedHeader*) m_pView;
			m_pEntries = (ENTRY*) (m_pView + sizeof(CSharedHeader));

			if (m_first)
				InitMemory(maxEntries);
		}
	}

	void Clean()
	{
		Lock();
        m_first = TRUE;
		m_pHeader->m_numEntries = 0;
		for (DWORD i = 0; i < m_pHeader->m_maxEntries; i++)
		{
			m_pEntries[i].m_id = EMPTY_ENTRY;
		}
		Unlock();
	}

	STATUS Add(const ENTRY& entry)
	{
		Lock();
		STATUS stat = STATUS_OK;
		if (entry.m_id == EMPTY_ENTRY)
			stat = STATUS_FAIL;

		if (stat == STATUS_OK && entry.m_id >= m_pHeader->m_maxEntries)
			stat = STATUS_FAIL;

		if (stat == STATUS_OK && m_pEntries[entry.m_id].m_id != EMPTY_ENTRY)
			stat = STATUS_FAIL;

		if (stat == STATUS_OK && m_pHeader->m_numEntries >= m_pHeader->m_maxEntries)
			stat = STATUS_FAIL;

		
		if (stat == STATUS_OK)
		{
			m_pHeader->m_numEntries++;
			m_pEntries[entry.m_id] = entry;
		}
		Unlock();
		return stat;
	}

	STATUS Get(DWORD id,ENTRY& entry) const
	{
		Lock();
		STATUS stat = STATUS_OK;

		if (id >= m_pHeader->m_maxEntries)
			stat = STATUS_FAIL;

		if (stat == STATUS_OK && id == EMPTY_ENTRY)
			stat = STATUS_FAIL;

		if (stat == STATUS_OK && id > m_pHeader->m_maxEntries)
			stat = STATUS_FAIL;

		if (stat == STATUS_OK && m_pEntries[id].m_id != id)
			stat = STATUS_FAIL;

		if (stat == STATUS_OK)
		{
			entry = m_pEntries[id];
		}
		Unlock();
		return stat;
	}

	STATUS Remove(DWORD id)
	{
		Lock();
		STATUS stat = STATUS_OK;
		if (id >= m_pHeader->m_maxEntries)
			stat =  STATUS_FAIL;

		if (stat == STATUS_OK && m_pEntries[id].m_id == EMPTY_ENTRY)
			stat =  STATUS_FAIL;

		if (stat == STATUS_OK)
		{
			m_pEntries[id].m_id = EMPTY_ENTRY;
		}
		Unlock();
		return stat;
	}

	STATUS Update(const ENTRY& entry)
	{
		Lock();
		STATUS stat = STATUS_OK;
		if (entry.m_id >= m_pHeader->m_maxEntries)
			stat =  STATUS_FAIL;
		if (stat == STATUS_OK && m_pEntries[entry.m_id].m_id == EMPTY_ENTRY)
			stat = STATUS_FAIL;
		if (stat == STATUS_OK)
		{
			m_pEntries[entry.m_id] = entry;
		}
		Unlock();
		return stat;
	}

private:
	void InitMemory(DWORD maxEntries)
	{
		m_pHeader->m_maxEntries = maxEntries;		
		Clean();		
	}

	ENTRY*          m_pEntries;
	CSharedHeader*  m_pHeader;
};

#endif  // SHARED_MEMORY_ARRAY_H_
