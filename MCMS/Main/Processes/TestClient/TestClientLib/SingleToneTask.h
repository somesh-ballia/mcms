// SingleToneTask.h: interface for the CSingleToneTask class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_SINGLETONETASK_H__)
#define _SINGLETONETASK_H__

#include "TaskApp.h"

extern "C" void singleToneEntryPoint(void* appParam);

class CSingleToneTask : public CTaskApp  
{
public:
	CSingleToneTask();
	virtual ~CSingleToneTask();

	BOOL         IsSingleton() const {return YES;}
	const char * GetTaskName() const {return "SingleToneTask";}
	void InitTask() {;}


};

#endif // !defined(_SINGLETONETASK_H__)
