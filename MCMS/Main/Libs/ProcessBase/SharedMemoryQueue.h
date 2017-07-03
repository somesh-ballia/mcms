#ifndef __SHARED_MEMORY_QUEUE_H__
#define __SHARED_MEMORY_QUEUE_H__

#include "SharedMemory.h"
#include "DataTypes.h"
#include "SharedHeader.h"
#include "StatusesGeneral.h"
#include "SharedDefines.h"
#include "Trace.h"
#include <sstream>


template <typename ENTRY> class CSharedMemoryQueue : public CShardMemory
{
public:
	friend class CSemaphoreGuard<CSharedMemoryQueue<ENTRY> >;
	CSharedMemoryQueue(const char* name,
					 BYTE permisssion,
					 DWORD maxEntries)
		:CShardMemory(name,
					  permisssion,
					  sizeof(ENTRY)*maxEntries + sizeof(CSharedHeaderQueue))
	{
		if (m_status == STATUS_OK)
		{
			m_pHeader = (CSharedHeaderQueue*) m_pView;
			m_pEntries = (ENTRY*) (m_pView + sizeof(CSharedHeaderQueue));

			if (m_first)
			{
				InitMemory(maxEntries);
				m_first = TRUE;
			}
		}
	}
	
	STATUS Dequeue(ENTRY& entry)
	{
		CSemaphoreGuard<CSharedMemoryQueue<ENTRY> > guard(*this);
		//Lock();
		STATUS stat = STATUS_OK;

		if (m_pHeader->m_numEntries>0)
		{
			// PASSERTMSG((m_pHeader->m_startQueue ==0 || m_pHeader->m_startQueue >=m_pHeader->m_maxEntries), "Dequeue: Unreasonable m_startQueue value. ");
			 // PASSERTMSG(( m_pHeader->m_startQueue >=m_pHeader->m_maxEntries), "Dequeue: Unreasonable m_startQueue value. ");
			 entry = m_pEntries[m_pHeader->m_startQueue];			 
			 
			//entry =*(m_pEntries + m_pHeader->m_startQueue);
			m_pHeader->m_startQueue = (m_pHeader->m_startQueue + 1) % m_pHeader->m_maxEntries;
			--m_pHeader->m_numEntries;
		}
		else
		{
			// TODO supply reason for failure
			stat = STATUS_FAIL;
		}

		//Unlock();
		return stat;

	}
	STATUS Queue(const ENTRY& entry)
	{
		CSemaphoreGuard<CSharedMemoryQueue<ENTRY> > guard(*this);
		
		//Lock();
		STATUS stat = STATUS_OK;
		if (m_pHeader->m_numEntries < m_pHeader->m_maxEntries)
		{
			//PASSERTMSG((m_pHeader->m_startQueue == 0 || m_pHeader->m_startQueue >=m_pHeader->m_maxEntries), "Queue: Unreasonable m_startQueue value. ");
 			//PASSERTMSG((m_pHeader->m_startQueue >=m_pHeader->m_maxEntries), "Queue: Unreasonable m_startQueue value. ");
			
			DWORD endQueue = (m_pHeader->m_startQueue + m_pHeader->m_numEntries ) % m_pHeader->m_maxEntries;			
			m_pEntries[endQueue] = entry;
			++m_pHeader->m_numEntries;
			//*(m_pEntries +endQueue) = entry;
		}
		else
		{
			stat = STATUS_FAIL;			
		}
		//Unlock();

		return stat;
	}

	// Note: no lock on this method
	DWORD GetNumOfEntries() const {return m_pHeader->m_numEntries;}
	
	BOOL IsCreator() const
	{
		return m_first;
	}
	
	// for debugging
	
	DWORD  StartQueuePlace() const
	{
		if (m_pHeader->m_numEntries == 0)
		{
			return -1;
		}
		return m_pHeader->m_startQueue;
	}
	
	DWORD  LastQueuePlace() const
	{
		if (m_pHeader->m_numEntries == 0)
		{
			return -1;
		}
		
		return (m_pHeader->m_startQueue + m_pHeader->m_numEntries ) % m_pHeader->m_maxEntries;	
	}		
	
	
private:
	void InitMemory(DWORD maxEntries)
	{
			Lock();
			m_pHeader->m_numEntries = 0;
			m_pHeader->m_startQueue = 0;
			m_pHeader->m_maxEntries = maxEntries;
			Unlock();
	}
	void DebugPrint(std::ostream &out);

	ENTRY*          	m_pEntries;
	CSharedHeaderQueue* m_pHeader;
	
//	template <class _ENTRY>
	//friend  std::ostream &operator<<( std::ostream &out, const CSharedMemoryQueue<_ENTRY>& sharedMemoryQueue );
	
};

//template <class _ENTRY> 
//std::ostream &operator<<( std::ostream &out, const CSharedMemoryQueue<_ENTRY>& sharedMemoryQueue );


#endif //__SHARED_MEMORY_QUEUE_H__
