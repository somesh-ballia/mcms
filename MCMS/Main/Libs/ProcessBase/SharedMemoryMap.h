// SharedMemoryMap.h: interface for the CSharedMemoryMap class template.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_SHAREDMEMORYMAP_H__)
#define _SHAREDMEMORYMAP_H__

#include "SharedMemory.h"
#include "DataTypes.h"
#include "SharedHeader.h"
#include "StatusesGeneral.h"
#include "MplMcmsStructs.h"
#include "SharedDefines.h"

// status definitions
#define STATUS_ILLEGAL_ENTRY_ID    1001
#define STATUS_ENTRY_NOT_FOUND     1002
#define STATUS_ENTRY_ALREDY_EXISTS 1003
#define STATUS_MEMORY_MAP_IS_FULL  1004
#define STATUS_MEMORY_MAP_IS_EMPTY 1005

template <class ENTRY> class CSharedMemoryMap : public CShardMemory
{
	friend class CTestSharedMemory;
public:
	class CSharedMemEntryCondition
	{
	public:
	  virtual ~CSharedMemEntryCondition(){;}
	  virtual BOOL IsMatch(const ENTRY& candidate) = 0;
	};

	CSharedMemoryMap(const char* name,
					 BYTE permisssion,
					 DWORD maxEntries)
		:CShardMemory(name,
					  permisssion,
					  sizeof(ENTRY)*maxEntries + sizeof(CSharedHeader))
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
			stat = STATUS_ILLEGAL_ENTRY_ID;

		if (stat == STATUS_OK && Find(entry.m_id))
			stat = STATUS_ENTRY_ALREDY_EXISTS;

		if (stat == STATUS_OK && m_pHeader->m_numEntries >= m_pHeader->m_maxEntries)
			stat = STATUS_MEMORY_MAP_IS_FULL;

		
		if (stat == STATUS_OK)
		{
			for (DWORD i = 0;i < m_pHeader->m_maxEntries; i++)
			{
				if (m_pEntries[i].m_id == EMPTY_ENTRY)
				{
					m_pHeader->m_numEntries++;
					m_pEntries[i] = entry;
					break;
				}
			}
		}
		Unlock();
		return stat;
	}

	STATUS Get(DWORD id,ENTRY& entry) const
	{
		Lock();
		STATUS stat = STATUS_OK;
		if (id == EMPTY_ENTRY)
			stat = STATUS_ILLEGAL_ENTRY_ID;

		if (stat == STATUS_OK)
		{
			ENTRY * foundEntry = Find(id);
			if (foundEntry)
				entry = *foundEntry;
			else
				stat = STATUS_ENTRY_NOT_FOUND;
		}
		Unlock();
		return stat;
	}

	STATUS Update(const ENTRY& entry)
	{
		Lock();
		STATUS stat = STATUS_OK;
		if (entry.m_id == EMPTY_ENTRY)
			stat = STATUS_ILLEGAL_ENTRY_ID;
		if (stat == STATUS_OK)
		{
			ENTRY * foundEntry = Find(entry.m_id);
			if (foundEntry)
				*foundEntry = entry;
			else
				stat = STATUS_ENTRY_NOT_FOUND;
		}

		Unlock();
		return stat;
	}

	STATUS AddOrUpdate(const ENTRY& entry)
	{
		Lock();

		STATUS stat = STATUS_OK;
		if (entry.m_id == EMPTY_ENTRY)
			stat = STATUS_ILLEGAL_ENTRY_ID;

		if (stat == STATUS_OK)
		{
			ENTRY * foundEntry = Find(entry.m_id);
			if (foundEntry)
				*foundEntry = entry;
			else
			{
				if (stat == STATUS_OK && m_pHeader->m_numEntries >= m_pHeader->m_maxEntries)
					stat = STATUS_MEMORY_MAP_IS_FULL;

				if (stat == STATUS_OK)
				{
					for (DWORD i = 0;i < m_pHeader->m_maxEntries; i++)
					{
						if (m_pEntries[i].m_id == EMPTY_ENTRY)
						{
							m_pHeader->m_numEntries++;
							m_pEntries[i] = entry;
							break;
						}
					}
				}
			}
		}

		Unlock();
		return stat;
	}

	
	STATUS Remove(DWORD id)
	{
		Lock();
		STATUS stat = STATUS_OK;
		if (id == EMPTY_ENTRY)
			stat =  STATUS_ILLEGAL_ENTRY_ID;

		if (stat == STATUS_OK)
		{
			ENTRY * foundEntry = Find(id);
			if (foundEntry)
			{
				foundEntry->m_id = EMPTY_ENTRY;
				m_pHeader->m_numEntries--;
			}
			else
				stat = STATUS_ENTRY_NOT_FOUND;
		}
		Unlock();
		return stat;
	}

	DWORD GetNumOfEntries() const {return m_pHeader->m_numEntries;}

	void GetEntriesSet(CSharedMemEntryCondition& entryChecker, ENTRY* matchingEntries, WORD nMatchingEntriesMaxSize, WORD& nNumMatched)
	{
		Lock();
		DWORD passedEntries = 0;
		DWORD i = 0;
		DWORD count = 0;
		while (i < m_pHeader->m_maxEntries &&
			   passedEntries < m_pHeader->m_numEntries &&
			   count < nMatchingEntriesMaxSize)
		{
			if (entryChecker.IsMatch(m_pEntries[i]))
			{
				matchingEntries[count++] = m_pEntries[i];
			}
			i++;
		}
		nNumMatched = count;
		Unlock();
	}
//private:
	ENTRY* Find(DWORD id) const // linear search
	{ // assuming already locked
		if (id == EMPTY_ENTRY)
			return NULL;
		DWORD passedEntries = 0;
		DWORD i = 0;
		while (i < m_pHeader->m_maxEntries &&
			   passedEntries < m_pHeader->m_numEntries)
		{
			if (m_pEntries[i].m_id != EMPTY_ENTRY)
			{
				passedEntries++;
				if (m_pEntries[i].m_id == id)
					return &(m_pEntries[i]);
			}
			i++;
		}
		return NULL;
	}

	void InitMemory(DWORD maxEntries)
	{
		m_pHeader->m_maxEntries = maxEntries;		
		Clean();		
	}

	ENTRY*          m_pEntries;
	CSharedHeader*  m_pHeader;
};


#endif // !defined(_SHAREDMEMORYMAP_H__)
                 
