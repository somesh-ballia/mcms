// TestTask.h: interface for the CTestTask class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(TESTTASK__H)
#define TESTTASK__H

#include "TaskApp.h"

extern "C" void testTaskEntryPoint(void* appParam);

class CTestTask : public CTaskApp
{
public:
	CTestTask();
	virtual ~CTestTask();

	void InitTask();

	BOOL         IsSingleton() const {return NO;}
	const char * GetTaskName() const {return "TestTask";}
    virtual int GetTaskPrioirty() {return -10;}
protected:
	void*  GetMessageMap();
	
	PDECLAR_MESSAGE_MAP

};

#endif // !defined(TESTTASK__H)
