#ifndef UNIQUE_INDEX_H
#define UNIQUE_INDEX_H

#include "SharedMemoryMap.h"
#include "DefinesGeneral.h"



class CUniqueIndex : private CShardMemory
{
public:
	CUniqueIndex(const char *sharedMemoryName = "SystemUniqueIndex");
	virtual ~CUniqueIndex();
	
	DWORD GetInc();
	
private:
    DWORD *m_ptrUniqueIndex;
};


#endif /*UNIQUE_INDEX_H*/

