#if !defined(_GKDISPATCHERTASK_H_)
#define _GKDISPATCHERTASK_H_

#include <set>

#include "DispatcherTask.h"
#include "GKServiceManager.h"
#include "GkMplMcmsProtocolTracer.h"
#include "TaskApi.h"

class CGKDispatcherTask : public CDispatcherTask
{
	CLASS_TYPE_1(CGKDispatcherTask, CDispatcherTask)
public:
	
	CGKDispatcherTask(BOOL isSync);
	virtual ~CGKDispatcherTask();
	virtual const char* NameOf() const { return "CGKDispatcherTask";}


	void InitTask(){;}
	//BOOL TaskHandleEvent(CSegment *pMsg,DWORD  msgLen,OPCODE opCode);
        virtual void HandleOtherIdTypes(CMessageHeader & header);
	BOOL TaskHandleEvent(CSegment *pMsg,DWORD  msgLen,OPCODE opCode);
    
};


#endif // !defined(_GKDISPATCHERTASK_H_)
