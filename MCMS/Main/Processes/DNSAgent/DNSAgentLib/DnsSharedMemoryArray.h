/*
 * DnsSharedMemoryArray.h
 *
 *  Created on: Jun 29, 2014
 *      Author: vasily
 */

#ifndef __DNSSHAREDMEMORYARRAY_H__
#define __DNSSHAREDMEMORYARRAY_H__



#include "SharedMemory.h"
#include <list>
#include "DataTypes.h"
#include "SharedHeader.h"
#include "StatusesGeneral.h"

/*
 The _Entry has to be without any virtual function
 The _Entry type has to implement functions:
   >  void Clean()         - cleanup itself
   >  bool IsEmpty() const - check if is empty object
   > _Entry& operator= (const _Entry&) - assignment operator
   >
 */


template <class _Entry> class DnsSharedMemoryArray : public CShardMemory
{
	friend class TestDnsRecordsMngr;
	friend class CSemaphoreGuard<DnsSharedMemoryArray<_Entry> >;

public:
	DnsSharedMemoryArray(const char* name, BYTE permission, DWORD maxEntries)
		: CShardMemory(name, permission, sizeof(_Entry) * maxEntries + sizeof(CSharedHeaderDw))
	{
		if (m_status == STATUS_OK)
		{
			m_pHeader = (CSharedHeaderDw*) m_pView;
			m_pEntries = (_Entry*) (m_pView + sizeof(CSharedHeaderDw));

			if (m_first)
				InitMemory(maxEntries);
		}
	}

	ESharedMemStatus Clean()
	{
		CSemaphoreGuard<DnsSharedMemoryArray<_Entry> > guard(*this);

		if (STATUS_OK != m_status)
			return eSharedMem_MemoryNotCreated;

		if (READ_ONLY == m_permisssion)
			return eSharedMem_NoPermissions;

		m_first = TRUE;
		m_pHeader->m_numEntries = 0;
		for (DWORD i=0; i < m_pHeader->m_maxEntries; ++i)
			m_pEntries[i].Clean();

		return eSharedMem_StatusOk;
	}

	DWORD EntriesNumber()
	{
		CSemaphoreGuard<DnsSharedMemoryArray<_Entry> > guard(*this);
		return m_pHeader->m_numEntries;
	}

	DWORD HeaderData()
	{
		CSemaphoreGuard<DnsSharedMemoryArray<_Entry> > guard(*this);

		if (STATUS_OK != m_status)
			return 0;

		if (WRITE_ONLY == m_permisssion)
			return 0;

		return m_pHeader->m_dwData;
	}

	void HeaderData(const DWORD dw)
	{
		CSemaphoreGuard<DnsSharedMemoryArray<_Entry> > guard(*this);

		if (STATUS_OK != m_status)
			return ;

		if (READ_ONLY == m_permisssion)
			return ;

		m_pHeader->m_dwData = dw;
	}

	ESharedMemStatus Add(const _Entry& entry)
	{
		CSemaphoreGuard<DnsSharedMemoryArray<_Entry> > guard(*this);

		if (STATUS_OK != m_status)
			return eSharedMem_MemoryNotCreated;

		if (READ_ONLY == m_permisssion)
			return eSharedMem_NoPermissions;

		if (m_pHeader->m_numEntries >= m_pHeader->m_maxEntries)
			return eSharedMem_MemoryIsFull;

		if (entry.IsEmpty())
			return eSharedMem_BadParameter;

		for (DWORD i=0; i<m_pHeader->m_maxEntries; ++i )
		{
			if (m_pEntries[i].IsEmpty())
			{
				m_pEntries[i] = entry;
				m_pHeader->m_numEntries++;
				return eSharedMem_StatusOk;
			}
		}
		return eSharedMem_MemoryIsFull;
	}

	template<typename _Predicate>
	ESharedMemStatus Get(_Predicate predicate, std::list<_Entry>& entries)
	{
		CSemaphoreGuard<DnsSharedMemoryArray<_Entry> > guard(*this);

		if (STATUS_OK != m_status)
			return eSharedMem_MemoryNotCreated;

		if (WRITE_ONLY == m_permisssion)
			return eSharedMem_NoPermissions;

		for (DWORD i=0; i<m_pHeader->m_maxEntries; ++i)
		{
			if (m_pEntries[i].IsEmpty())
				continue;

			if (predicate(m_pEntries[i]))
				entries.push_back(m_pEntries[i]);
		}

		return eSharedMem_StatusOk;
	}

	template<typename _ConditionPredicate, typename _UpdatePredicate>
	ESharedMemStatus GetAndUpdate(_ConditionPredicate condition, _UpdatePredicate update, std::list<_Entry>& entries)
	{
		CSemaphoreGuard<DnsSharedMemoryArray<_Entry> > guard(*this);

		if (STATUS_OK != m_status)
			return eSharedMem_MemoryNotCreated;

		if (READ_ONLY == m_permisssion)
			return eSharedMem_NoPermissions;

		for (DWORD i=0; i<m_pHeader->m_maxEntries; ++i)
		{
			if (m_pEntries[i].IsEmpty())
				continue;

			if (condition(m_pEntries[i]))
			{
				update(m_pEntries[i]);
				entries.push_back(m_pEntries[i]);
			}
		}

		return eSharedMem_StatusOk;
	}

	template<typename _Predicate>
	ESharedMemStatus Get(_Predicate predicate, _Entry& entry)
	{
		CSemaphoreGuard<DnsSharedMemoryArray<_Entry> > guard(*this);

		if (STATUS_OK != m_status)
			return eSharedMem_MemoryNotCreated;

		if (WRITE_ONLY == m_permisssion)
			return eSharedMem_NoPermissions;

		for (DWORD i=0; i<m_pHeader->m_maxEntries; ++i)
		{
			if (m_pEntries[i].IsEmpty())
				continue;

			if (predicate(m_pEntries[i]))
			{
				entry = m_pEntries[i];
				return eSharedMem_StatusOk;
			}
		}
		return eSharedMem_EntryNotFound;
	}

	template<typename _Predicate>
	ESharedMemStatus Remove(_Predicate predicate, size_t& count)
	{
		CSemaphoreGuard<DnsSharedMemoryArray<_Entry> > guard(*this);

		if (STATUS_OK != m_status)
			return eSharedMem_MemoryNotCreated;

		if (READ_ONLY == m_permisssion)
			return eSharedMem_NoPermissions;

		count = 0;
		for (DWORD i=0; i<m_pHeader->m_maxEntries; ++i)
		{
			if (m_pEntries[i].IsEmpty())
				continue;

			if (predicate(m_pEntries[i]))
			{
				m_pEntries[i].Clean();
				--m_pHeader->m_numEntries;
				++count;
			}
		}
		return eSharedMem_StatusOk;
	}

	/*ESharedMemStatus Update(const _Entry& entry)
	{
		CSemaphoreGuard<DnsSharedMemoryArray<_Entry> > guard(*this);

		if (STATUS_OK != m_status)
			return eSharedMem_MemoryNotCreated;

		ESharedMemStatus status = eSharedMem_StatusOk;
		return status;
	}*/

	template<typename _Predicate>
	ESharedMemStatus Use(_Predicate predicate, bool once, size_t& count)
	{
		CSemaphoreGuard<DnsSharedMemoryArray<_Entry> > guard(*this);

		if (STATUS_OK != m_status)
			return eSharedMem_MemoryNotCreated;

		if (READ_ONLY == m_permisssion)
			return eSharedMem_NoPermissions;

		count = 0;
		for (DWORD i=0; i<m_pHeader->m_maxEntries; ++i)
		{
			if (m_pEntries[i].IsEmpty())
				continue;

			bool res = predicate(m_pEntries[i]);
			if (res)
			{
				++count;

				if (once)
					return eSharedMem_StatusOk;
			}
		}
		return eSharedMem_StatusOk;
	}


private:
	void InitMemory(DWORD maxEntries)
	{
		m_pHeader->m_maxEntries = maxEntries;
		m_pHeader->m_dwData     = 1;
		Clean();
	}

	_Entry*           m_pEntries;
	CSharedHeaderDw*  m_pHeader;
};



#endif /* __DNSSHAREDMEMORYARRAY_H__ */
