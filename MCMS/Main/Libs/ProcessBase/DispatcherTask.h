// DispatcherTask.h: interface for the CDispatcherTask class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_DISPATCHERTASK_H__)
#define _DISPATCHERTASK_H__

#include "TaskApp.h"

extern "C" void dispatcherEntryPoint(void* appParam);
extern "C" void syncDispatcherEntryPoint(void* appParam);

class CDispatcherTask : public CTaskApp
{
CLASS_TYPE_1(CDispatcherTask,CTaskApp )
public:
//	CDispatcherTask();
	CDispatcherTask(BOOL isSync);
	virtual ~CDispatcherTask();
	virtual const char* NameOf() const { return "CDispatcherTask";}

    virtual void HandleOtherIdTypes(CMessageHeader & header){PASSERT(1);}

//	void   WaitForEvent();
	void InitTask(){;}

	BOOL         IsSingleton() const {return YES;}
	const char * GetTaskName() const;
    virtual BOOL Dispatcher(CMessageHeader&);


	BOOL m_isSync;

};

#endif // !defined(_DISPATCHERTASK_H__)
