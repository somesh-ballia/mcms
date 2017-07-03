// ErrorHandlerTask.h: interface for CErrorHandler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(ERRORHANDLERTASK_H)
#define ERRORHANDLERTASK_H

#include "AlarmableTask.h"

extern "C" void errorHandlerEntryPoint(void* appParam);

class CErrorHandlerTask : public CAlarmableTask 
{
public:
	CErrorHandlerTask();
	virtual ~CErrorHandlerTask();

    void InitTask();
	virtual const char* NameOf() const { return "CErrorHandlerTask";}
    
	BOOL         IsSingleton() const {return YES;}
	const char * GetTaskName() const {return "ErrorHandlerTask";}
    static void SignalHandler(int signal);
    static void SignalVoidHandler(int signal);    
    static void HandleDeadChild();
    


protected:
	void OnException(CSegment* pMsg);
    void SendResetReqToDaemon(CSegment* pMsg);
    
    void EmptyQueue(CTaskApp* TaskToFlushQueue);//Release all messages in a queue
    void ReviveTask(CTaskApp * pTask); //use same CTaskApp to revive a faulted thread
    void CloneTask(CTaskApp * pTask); //use new CTaskApp and new thread
    void KillWholeProcess();//Kill whole process
    TICKS GetFirstCrashTime(CTaskApp * pTask);
    int GetNumberOfCrashes(CTaskApp * pTask);
    void ZeroCrashCounter(CTaskApp * pTask);
    void ZeroTimeFromFirstCrash(CTaskApp * pTask);

    
    
	PDECLAR_MESSAGE_MAP
};

#endif // !defined(ERRORHANDLERTASK_H)
