// SharedMemory.h: interface for the CSharedMemoryMap class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_SHAREDMEMORY_H__)
#define _SHAREDMEMORY_H__

#include "DataTypes.h"

#define READ_ONLY  0
#define READ_WRITE 1
#define WRITE_ONLY 2


// Make sure that unlocking will be called when going out of this class definition scope
template <class T>
class CSemaphoreGuard
{
public:
	
	CSemaphoreGuard(T & t) : m_t(t)
	{
		m_t.Lock();
	}
	~CSemaphoreGuard()
	{
		m_t.Unlock();
	}
private:
	T& m_t;
	
};

class CShardMemory
{
public:
	 
	CShardMemory(const char* name,
				 BYTE permisssion,
				 DWORD size);


	virtual ~CShardMemory();

protected:
	void Lock() const;
	void Unlock() const;

	const char* m_name;
	BYTE        m_permisssion;
	STATUS      m_status;
	SM_HANDLE   m_fileMap;
	BOOL        m_first;
	BYTE*       m_pView;
	DWORD       m_size;
    int         m_semaphore;
};


#endif // !defined(_SHAREDMEMORY_H__)
