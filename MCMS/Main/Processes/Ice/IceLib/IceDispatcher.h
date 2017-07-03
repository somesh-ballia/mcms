#if !defined(_ICEDISPATCHER_H_)
#define _ICEDISPATCHER_H_

#include <set>

#include "DispatcherTask.h"
#include "IceServiceManager.h"
#include "TaskApi.h"

class CIceDispatcherTask : public CDispatcherTask
{
	CLASS_TYPE_1(CIceDispatcherTask, CDispatcherTask)
public:
	
	CIceDispatcherTask(BOOL isSync);
	virtual ~CIceDispatcherTask();
	virtual const char* NameOf() const { return "CIceDispatcherTask";}


	void InitTask(){;}
    virtual void HandleOtherIdTypes(CMessageHeader & header);
	BOOL TaskHandleEvent(CSegment *pMsg,DWORD  msgLen,OPCODE opCode);
    
};


#endif // !defined(_ICEDISPATCHER_H_)
