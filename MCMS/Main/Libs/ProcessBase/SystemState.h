#ifndef MCMSSTATES_H_
#define MCMSSTATES_H_

#include "SharedMemoryMap.h"
#include "DefinesGeneral.h"


#define SHARED_MEMORY_NAME 			"SystemStates"
#define SHARED_MEMORY_PERMISITIONS 	1
#define SHARED_MEMORY_MAX_NUM 		1



class CSystemState : private CShardMemory
{
public:
	CSystemState();
	virtual ~CSystemState();
	
	eMcuState Get() const;
	void Set(eMcuState state);
	
private:
	eMcuState *m_pState;
};


#endif /*MCMSSTATES_H_*/


