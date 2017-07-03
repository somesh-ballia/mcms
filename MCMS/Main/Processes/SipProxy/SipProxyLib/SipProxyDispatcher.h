#if !defined(_SIPPROXYDISPATCHER_H_)
#define _SIPPROXYDISPATCHER_H_

#include <set>

#include "DispatcherTask.h"
#include "SipProxyServiceManager.h"
#include "SipProxyMplMcmsProtocolTracer.h"
#include "TaskApi.h"

class CSipProxyDispatcherTask : public CDispatcherTask
{
	CLASS_TYPE_1(CSipProxyDispatcherTask, CDispatcherTask)
public:
	
	CSipProxyDispatcherTask(BOOL isSync);
	virtual ~CSipProxyDispatcherTask();
	virtual const char* NameOf() const { return "CSipProxyDispatcherTask";}


	void InitTask(){;}
	//BOOL TaskHandleEvent(CSegment *pMsg,DWORD  msgLen,OPCODE opCode);
        virtual void HandleOtherIdTypes(CMessageHeader & header);
	BOOL TaskHandleEvent(CSegment *pMsg,DWORD  msgLen,OPCODE opCode);
    
};


#endif // !defined(_SIPPROXYDISPATCHER_H_)
