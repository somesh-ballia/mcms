#ifndef TASK_APP_H__
#define TASK_APP_H__

//////////////////////////////////////////////////////////////////////
#include "SharedDefines.h"

#include "StateMachine.h"

#include "MessageHeader.h"
#include "TraceHeader.h"

#include "SignalHandling.h"

#include "SystemTick.h"
#include "SystemFunctions.h"

#include "Macros.h"

#include <list>
#include <string>
#include <map>

//////////////////////////////////////////////////////////////////////
typedef void (*TaskEntryPoint)(void*);
typedef std::list<CMessageHeader> LocalQueue;
typedef std::map<DWORD, const CStateMachine*> StateMachinesMap;

//////////////////////////////////////////////////////////////////////
class CSegment;
class CTaskApi;
class COsQueue;
class COsTask;
class CTimer;
class CFaultList;
class CMessageQueue;
class CFaultBlockQueue;

//////////////////////////////////////////////////////////////////////
enum eThread_Group
{
	eTaskGroupNone    = -1,
	eTaskGroupLow     = 0,
	eTaskGroupRegular = 1,
	TASK_GROUPS_NUM   = 2
};

//////////////////////////////////////////////////////////////////////
enum eTaskRecoveryPolicy
{
	eRecreateTaskAndThread,   // Option A in SRS
	eRecreateThread           // Option B in SRS
};

//////////////////////////////////////////////////////////////////////
enum eTaskRecoveryPolicyAfterSeveralRetries
{
	eTerminateProcess,        // Kill the all process
	eDoNothing,               // Don't recover the task any more
	eCreateNewTask
};

//////////////////////////////////////////////////////////////////////
enum eActiveAlarmPolicyAfterSeveralRetries
{
	eGenerateActiveAlarm,
	eDontGenerateActiveAlarm
};

//////////////////////////////////////////////////////////////////////
enum eTaskRecoveryPolicyDuringSystemStartup
{
	eTerminateProcessDuringStartup,   // Kill the all process
	eRegularTaskRecoveryPolicy        // use the eTaskRecoveryPolicy
};

//////////////////////////////////////////////////////////////////////
enum eResetSource
{
	eResetSourceInternalUnknown = 0,
	eResetSourceInternalMcmsDaemon,
	eResetSourceInternalMcmsOther,
	eResetSourceExternalEma,
	eResetSourceExternalShelf,
	eResetSourceDiagnostics,
	eResetSourceExternalTerminal
};

//////////////////////////////////////////////////////////////////////
bool operator ==(const CTaskApp& first, const CTaskApp& second);
bool operator <(const CTaskApp& first, const CTaskApp& second);

//////////////////////////////////////////////////////////////////////
class CTaskApp : public CStateMachine
{
	friend class CStateMachine;
	friend class CErrorHandlerTask;

	friend bool operator==(const CTaskApp& first, const CTaskApp& second);
	friend bool operator<(const CTaskApp& first, const CTaskApp& second);

	CLASS_TYPE_1(CTaskApp, CStateMachine)

	virtual const char* NameOf() const
	{ return GetCompileType(); }

public:

	class Unlocker
	{
	public:

		Unlocker(bool enable = true);
		~Unlocker();

	private:

		bool      m_enable;
		CTaskApp* m_task;

		DISALLOW_COPY_AND_ASSIGN(Unlocker);
	};

	class MutexLocker
	{
	public:
		MutexLocker(pthread_mutex_t& mutex);
		~MutexLocker();
	private:
		pthread_mutex_t& m_mutex;
		DISALLOW_COPY_AND_ASSIGN(MutexLocker);
	};
	                             CTaskApp();
	virtual                     ~CTaskApp();

	virtual void                 Create(CSegment& appParam, WORD limited = FALSE);

	void                         CreateReceiveMailbox();
	void                         CreateResponseMailbox();
	void                         Dump(std::ostream& msg) const;
	void                         DumpQueues(std::ostream& msg) const;
	void                         DumpTimers(std::ostream& msg) const;
	void                         DumpOpcodeTail(std::ostream& msg, DWORD numOfMessages) const;
	void                         DumpStateMachines(std::ostream& msg) const;
	COsQueue&                    GetRcvMbx();

	const COsQueue*              GetRspMbx() const               { return m_pRspMbx; }
	const COsQueue*              GetRspMbxRead() const           { return m_pRspMbxRead; }
	const COsQueue*              GetClientRspMbx() const         { return m_pClientRspMbx; }

	const StateMachineDescriptor GetClientStateMachine() const   { return m_ClientStateMachineDesc; }
	const COsTask*               GetOsTask()                     { return m_pTask; }

	void                         FlushQueue();
	LocalQueue*                  GetLocalQueue()                 { return &m_localQueue;}
	DWORD                        GetTaskId() const;
	void                         SetPriority(DWORD priority);
	void                         SetOsTask(COsTask& pOsTask)     { m_pTask = &pOsTask; }
	void                         SetMaxCrashAllowed(DWORD MaxCrashAllowed) { m_MaxCrashAllowed = MaxCrashAllowed; }
	DWORD                        GetNextRspMsgSeqNum();
	DWORD                        GetCurrentRspMsgSeqNum();
	DWORD                        GetClientRspMsgSeqNum();

	DWORD                        GetCrashCounter() const         { return m_CrashCounter; }
	DWORD                        GetAbsCrashCounter() const      { return m_AbsCrashCounter; }

	void                         QueueLocalMessage(const CMessageHeader& header);
	BOOL                         DequeueLocalMessage(CMessageHeader& header);
	eThread_Group                GetThreadGroup()                { return m_Thread_Group; }

	static void                  DummyStartTask1(CTaskApp* pCurrentTask);
	static void                  DummyStartTask2(CTaskApp* pCurrentTask);
	static void                  DummyStartTask3(CTaskApp* pCurrentTask);
	static void                  DummyStartTask4(CTaskApp* pCurrentTask);
	static void                  DummyStartTask(CTaskApp* pCurrentTask);

	virtual int                  GetTaskPrioirty() const         { return 0; }
	virtual BOOL                 Dispatcher(CMessageHeader&)     { return false; }
	virtual const char*          GetTaskName() const             { return NameOf(); };
	virtual BOOL                 IsSingleton() const = 0;
	virtual BOOL                 IsManager() const               { return false; }
	virtual int                  GetTaskMbxBufferSize() const    { return 128 * 1024 - 1; }
	virtual int                  GetTaskMbxThreshold() const     { return -1; }
	virtual int                  GetTaskMbxSndBufferSize() const { return -1; }

	void                         SetCpuLimit(const TICKS& samplePriod, const TICKS& maxAllowed);

	virtual eTaskRecoveryPolicy  GetTaskRecoveryPolicy() const
	{ return eRecreateThread; }

	virtual eTaskRecoveryPolicyAfterSeveralRetries GetTaskRecoveryPolicyAfterSeveralRetries() const
	{ return eTerminateProcess; }

	virtual eTaskRecoveryPolicyDuringSystemStartup GetTaskRecoveryPolicyDuringSystemStartup() const
	{ return eRegularTaskRecoveryPolicy; }

	virtual bool                 IsResetInStartupFail() const    { return false; }
	virtual BOOL                 CloneTask()                     { return false; }
	DWORD                        GetMaximumTaskRecoveryRetries() { return m_MaxCrashAllowed;}
	DWORD                        GetNumOfCrashes()               { return m_CrashCounter;}
	const TICKS                  GetFirstCrashTime()             { return m_FirstCrashTime;}
	DWORD                        GetOpcodeTailLen() const;
	void                         IncrementCrashCounter();
	CFaultBlockQueue*            GetFaultBlockQueue() const      { return m_FaultBlockQueue; }

	// only Conf and Party tasks will override those functions
	virtual DWORD                GetConfId() const               { return DEFAULT_CONF_ID; }
	virtual DWORD                GetPartyId() const              { return DEFAULT_PARTY_ID; }
	virtual DWORD                GetMonitorConfId() const        { return DEFAULT_CONF_ID; }
	virtual DWORD                GetMonitorPartyId() const       { return DEFAULT_PARTY_ID; }

	void                         LockRelevantSemaphore();
	void                         UnlockRelevantSemaphore();
	STATUS                       UndoRelevantSemaphore();
	DWORD                        AddStateMachine(const CStateMachine*);
	void                         DeleteStateMachine(DWORD sequenceNumber, const CStateMachine*);
	virtual void                 SetSelfKill()                   { m_selfKill = true; }
	BOOL                         GetSelfKill() const             { return m_selfKill; }

protected:

	COsTask*                     m_pTask;
	CTimer*                      m_pTimer;
	COsQueue*                    m_pRcvMbx;     // other tasks in this process should use this
	COsQueue*                    m_pRcvMbxRead; // for WaitForEvent function
	COsQueue*                    m_pRspMbx;     //
	COsQueue*                    m_pRspMbxRead; //

	COsQueue*                    m_pCreatorRcvMbx;
	COsQueue*                    m_pClientRspMbx;
	StateMachineDescriptor       m_ClientStateMachineDesc;
	DWORD                        m_ClientRspMsgSeqNum;
	DWORD                        m_ClientRspMsgType;

	WORD                         m_selfKill;
	eResetSource                 m_resetSource;
	std::string                  m_ResetDescription;
	CSignaldHandling             m_SignalHandling;
	LocalQueue                   m_localQueue;
	eThread_Group                m_Thread_Group;

	bool                         m_wasInitialized;
	bool                         m_initializing;
	bool                         m_isTaskCpuLimited;

	TICKS                        m_lastRealTickSample;
	TICKS                        m_lastCpuTickSample;
	TICKS                        m_samplePriod;
	TICKS                        m_maxTickAllowInPriod;

	pid_t                        m_pid;

	DWORD                        m_SemaphoreLastLockedTicks;
	DWORD                        m_SemaphoreTaskThatDidLastOp;

protected:

	void InitTaskMessagePool();

	void SetPID(pid_t pid)
	{ m_pid = pid; }

	void SetInit()
	{ m_wasInitialized = true; }

	BOOL GetInit()
	{ return m_wasInitialized; }

	bool CreateOsTask();

	STATUS ResponedClientRequest(OPCODE opcode = 0, CSegment* pMsg = NULL);

	STATUS ResponedClientRequest(
		const COsQueue& pClientRspMbx,
		DWORD ClientRspMsgSeqNum,
		DWORD ClientRspMsgType,
		StateMachineDescriptor*
		pClientStateMachineDesc = NULL,
		OPCODE opcode = 0,
		CSegment* pMsg = NULL);

	void SetDestroyReason(const std::string&);
	const std::string& GetDestroyReason();

	void PushMessageToQueue(OPCODE opcode, DWORD len, eProcessType processType = eProcessTypeInvalid);
	void AddFilterOpcodeToQueue(OPCODE opcode) const;

protected:

	static std::string GetUniquePerExeTaskName(const char* taskName = "TaskName");

protected:

	virtual void  WaitForEvent();

	virtual TICKS CalcExpiredTimeout(bool& isTimerTimeout);

	virtual void  HandleEventWithStateMachine(const StateMachineDescriptor StateMachineDesc, OPCODE opcode, CSegment* pMsg);
	virtual void  HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode);

	virtual BOOL  TaskHandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode)
	{ return false; }

	virtual void  SelfKill();

	virtual void  InitTask() = 0;

	virtual int   GetPrivateFileDescriptor()
	{ return -1; }

	virtual void  HandlePrivateFileDescriptor()
	{}

	virtual void  AddFilterOpcodePoint()
	{}

private:

	void IncreaseCrashCounter()
	{ ++m_CrashCounter; ++m_AbsCrashCounter; }

	void ZeroCrashCounter()
	{ m_CrashCounter = 0; }

	void ZeroTimeFromFirstCrash()
	{ m_FirstCrashTime = SystemGetTickCount(); }

private:

	DWORD                        m_LastCrashTime;
	DWORD                        m_CrashCounter;
	DWORD                        m_AbsCrashCounter;
	DWORD                        m_MaxCrashAllowed;
	TICKS                        m_FirstCrashTime;
	std::string                  m_destroyReason;
	CTaskApi*                    m_pSelfApi;

	CMessageQueue*               m_pMessageQueue;
	CFaultBlockQueue*            m_FaultBlockQueue;

	DWORD                        m_NextStateMachineHandle;
	StateMachinesMap             m_StateMachinesMap;

	DWORD                        m_RspMsgSeqNum;
	DWORD                        m_TicksSemaphoreAlarm;
};

#endif // ifndef TASK_APP_H__
