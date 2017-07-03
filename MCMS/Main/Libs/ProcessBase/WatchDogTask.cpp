//+========================================================================+
//                       WatchDogTask.cpp                                  |
//                     Copyright 2005 Polycom                              |
//                      All Rights Reserved.                               |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       WatchDogTask.cpp                                            |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Sagi                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
// Sagi| 14.2.05    | Interprocess tasks                                   |
//+========================================================================+


#include "WatchDogTask.h"
#include "MessageHeader.h"
#include "Segment.h"
#include "Trace.h"
#include "Macros.h"
#include "ProcessBase.h"
#include "ManagerApi.h"
#include "OpcodesMcmsCommon.h"
#include "SysConfig.h"

// CTestClientManager timers
#define WATCH_DOG_TIMER 1008
#define WATCHDOG_TIMEOUT (5 * SECOND)   //kobi g : Quality Task, reduce number ofWatdog messages in the Queues by changing interval form 2 to 5

// message map
PBEGIN_MESSAGE_MAP(CWatchDogTask)
  ONEVENT(WATCH_DOG_TIMER  ,ANYCASE , CWatchDogTask::OnTimerWatchDog)
PEND_MESSAGE_MAP(CWatchDogTask,CStateMachine);



////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void watchDogEntryPoint(void* appParam)
{
	CWatchDogTask * watchDogTask = new CWatchDogTask;
	watchDogTask->Create(*(CSegment*)appParam);
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CWatchDogTask::CWatchDogTask()
{
//    m_Thread_Group = eTaskGroupCritical;
}


//////////////////////////////////////////////////////////////////////
CWatchDogTask::~CWatchDogTask()
{

}

//////////////////////////////////////////////////////////////////////
void CWatchDogTask::InitTask()
{
	StartTimer(WATCH_DOG_TIMER, WATCHDOG_TIMEOUT);
}

//////////////////////////////////////////////////////////////////////
void CWatchDogTask::OnTimerWatchDog(CSegment* pMsg)
{
    CProcessBase *pProcess = CProcessBase::GetProcess();
    DWORD processType = (DWORD)(pProcess->GetProcessType());

    CSegment *pSeg = new CSegment;
    *pSeg << processType;

   	CManagerApi api(eProcessMcmsDaemon);
	STATUS status = api.SendMsg(pSeg, WD_KEEP_ALIVE);
    if(STATUS_OK == status)
    {
		pProcess->IncrementWatchDogCnt();
    }

	StartTimer(WATCH_DOG_TIMER, WATCHDOG_TIMEOUT);
}

//////////////////////////////////////////////////////////////////////
void*  CWatchDogTask::GetMessageMap()
{
  return (void*)m_msgEntries;
}
