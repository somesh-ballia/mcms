// CsDispatcherTask.h: interface for the CCsDispatcherTask class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CS_DISPATCHERTASK_H__)
#define _CS_DISPATCHERTASK_H__


#include "DispatcherTask.h"
#include "CSMngrProcess.h"
#include "IpService.h"


class CCsDispatcherTask : public CDispatcherTask
{
	CLASS_TYPE_1(CCsDispatcherTask, CDispatcherTask)
public:
	
	CCsDispatcherTask();
	virtual ~CCsDispatcherTask();
	virtual const char* NameOf() const { return "CCsDispatcherTask";}


	void InitTask(){;}
	BOOL TaskHandleEvent(CSegment *pMsg,DWORD  msgLen,OPCODE opCode);
	void HandleSignalingEvent(CSegment *pSeg);
	
	void SendSignalingNotCreatedToCsApi(BYTE csId);
	
	void SetIsAssertForThisSignalingTask(BOOL isAssert, WORD csId);
	BOOL GetIsAssertForThisSignalingTask(WORD csId);

protected:
	CCSMngrProcess*  m_pProcess;
	BOOL  m_isAssertForThisSignalingTask[MAX_NUMBER_OF_SERVICES_IN_RMX_4000];
};

#endif // _CS_DISPATCHERTASK_H__
