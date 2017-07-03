// WatchDogTask.h: interface for the CWatchDogTask class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(WATCHDOGTASK__H)
#define WATCHDOGTASK__H

#include "TaskApp.h"

extern "C" void watchDogEntryPoint(void* appParam);

class CWatchDogTask : public CTaskApp
{
public:
	CWatchDogTask();
	virtual ~CWatchDogTask();


	virtual const char* NameOf() const { return "CWatchDogTask";}
	void InitTask();

	BOOL         IsSingleton() const {return YES;}
	const char * GetTaskName() const {return "WatchDogTask";}
    //   virtual int GetTaskPrioirty()const {return -10;}
protected:
	void OnTimerWatchDog(CSegment* pMsg);
	void*  GetMessageMap();
	
	PDECLAR_MESSAGE_MAP

};

#endif // !defined(WATCHDOGTASK__H)
