#include "TaskApp.h"

#include "OsTask.h"
#include "TaskApi.h"

#include "Timer.h"

#include "OsQueue.h"

#include "MessageHeader.h"
#include "Segment.h"

#include "OpcodesRanges.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"

#include "InternalProcessStatuses.h"
#include "ApiStatuses.h"

#include "SystemFunctions.h"
#include "ObjString.h"

#include "SysConfig.h"

#include "MessagePool.h"
#include "FaultBlockQueue.h"

#include "TraceStream.h"
#include "TraceAccumulator.h"

#include <sys/types.h>
#include <stdio.h>
#include <linux/unistd.h>
#include <ostream>
#include <iomanip>
#include <string>

////////////////////////////////////////////////////////////////////////////
// After 3 times the level 2 starts
static const DWORD MAXIMUM_CRASH_ALLOWED = 3;

////////////////////////////////////////////////////////////////////////////
// in minutes
static CProcessBase* pProcess = NULL;

////////////////////////////////////////////////////////////////////////////
// Parameter enable is true by default
CTaskApp::Unlocker::Unlocker(bool enable)
	: m_enable(enable)
	, m_task(COsTask::GetTaskApp())
{
	if (!m_enable)
		return;

	if (!m_task)
		return;

	m_task->UnlockRelevantSemaphore();
}

CTaskApp::Unlocker::~Unlocker()
{
	if (!m_enable)
		return;

	if (!m_task)
		return;

	m_task->LockRelevantSemaphore();
}


////////////////////////////////////////////////////////////////////////////
CTaskApp::MutexLocker::MutexLocker(pthread_mutex_t& mutex) : m_mutex(mutex)
{
	pthread_mutex_lock(&m_mutex);
}
CTaskApp::MutexLocker::~MutexLocker()
{
	pthread_mutex_unlock(&m_mutex);
}

////////////////////////////////////////////////////////////////////////////
CTaskApp::CTaskApp()
	: m_pTask(NULL)
	, m_pTimer(new CTimer)
	, m_pRcvMbx(NULL)
	, m_pRcvMbxRead(NULL)
	, m_pRspMbx(NULL)
	, m_pRspMbxRead(NULL)
	, m_pCreatorRcvMbx(new COsQueue)
	, m_pClientRspMbx(new COsQueue)
	, m_ClientRspMsgSeqNum(0)
	, m_ClientRspMsgType(0)
	, m_selfKill(false)
	, m_resetSource(eResetSourceInternalUnknown)
	, m_Thread_Group(eTaskGroupRegular)
	, m_wasInitialized(false)
	, m_initializing(false)
	, m_isTaskCpuLimited(false)
	, m_pid(0)
	, m_SemaphoreLastLockedTicks(0)
	, m_SemaphoreTaskThatDidLastOp(0)
	, m_LastCrashTime(~0)
	, m_CrashCounter(0)
	, m_AbsCrashCounter(0)
	, m_MaxCrashAllowed(0)
	, m_pSelfApi(NULL)
	, m_pMessageQueue(new CMessageQueue)
	, m_FaultBlockQueue(new CFaultBlockQueue)
	, m_NextStateMachineHandle(1)
	, m_RspMsgSeqNum(0)
	, m_TicksSemaphoreAlarm(10 * SECOND)
{
	m_type = "CTaskApp";

	// The state machine of this task was registered in other task's table
	// therefore - we delete it from "other" task and registered it in itself.
	if (m_OwnerTaskApp)
		m_OwnerTaskApp->DeleteStateMachine(m_StateMachineHandle, this);

	m_StateMachineHandle = AddStateMachine(this);
	m_OwnerTaskApp = this;
}

//--------------------------------------------------------------------------
CTaskApp::~CTaskApp()
{
	POBJDELETE(m_pTimer);
	POBJDELETE(m_pRcvMbx);
	POBJDELETE(m_pRcvMbxRead);
	POBJDELETE(m_pRspMbx);
	POBJDELETE(m_pRspMbxRead);
	POBJDELETE(m_pCreatorRcvMbx);
	POBJDELETE(m_pClientRspMbx);
	POBJDELETE(m_pTask);
	POBJDELETE(m_pMessageQueue);
	POBJDELETE(m_FaultBlockQueue);

	m_suspended = true;
}

//--------------------------------------------------------------------------
void CTaskApp::CreateReceiveMailbox()
{
	eProcessType processType = CProcessBase::GetProcess()->GetProcessType();

	char uniqueName[MAX_QUEUE_NAME_LEN];
	const char* taskName = NULL;
	if (IsSingleton())
		taskName = GetTaskName();

	if (!taskName)
	{
		COsQueue::CreateUniqueQueueName(uniqueName);
		taskName = uniqueName;
	}

	m_pRcvMbxRead = new COsQueue;
	m_pRcvMbx     = new COsQueue;

	m_pRcvMbxRead->CreateRead(processType, taskName, GetTaskMbxBufferSize(), GetTaskMbxThreshold());
	PASSERTMSG((m_pRcvMbxRead->m_idType == eInvalidId), "Invalid m_pRcvMbxRead");

	m_pRcvMbx->CreateWrite(processType, taskName, true, GetTaskMbxSndBufferSize());
	PASSERTMSG((m_pRcvMbx->m_idType == eInvalidId), "Invalid m_pRcvMbx");
}

//--------------------------------------------------------------------------
void CTaskApp::CreateResponseMailbox()
{
	const eProcessType processType = CProcessBase::GetProcess()->GetProcessType();

	char uniqueName[MAX_QUEUE_NAME_LEN] = {};

	if (IsSingleton())
		strncpy(uniqueName, GetTaskName(), MAX_QUEUE_NAME_LEN - 10);
	else
		COsQueue::CreateUniqueQueueName(uniqueName);

	strncat(uniqueName, "_Response", MAX_QUEUE_NAME_LEN-1);

	m_pRspMbxRead = new COsQueue;
	m_pRspMbx     = new COsQueue;

	m_pRspMbxRead->CreateRead(processType, uniqueName);
	m_pRspMbx->CreateWrite(processType, uniqueName, true);
}

//--------------------------------------------------------------------------
void CTaskApp::Create(CSegment& appParam, WORD limited)
{
	pProcess = CProcessBase::GetProcess();

	InitTaskMessagePool();

	m_selfKill = false;
	m_pCreatorRcvMbx->DeSerialize(appParam);
	CreateReceiveMailbox();
	CreateResponseMailbox();

	bool res = false;

	if (m_pRcvMbxRead->m_idType == eInvalidId || m_pRspMbxRead->m_idType == eInvalidId)
		m_selfKill = true;
	else
		res = CreateOsTask();

	if (!res)
	{
		PASSERTMSG(true, "Task creation failed");

		if (GetTaskRecoveryPolicyDuringSystemStartup() == eTerminateProcessDuringStartup)
			SystemCoreDump(false);
		else
		{
			bool continueRunning = (GetTaskRecoveryPolicyAfterSeveralRetries() != eTerminateProcess);

			SystemCoreDump(continueRunning);
		}
	}


	m_pRcvMbx->Serialize(appParam);
	appParam << (void*)this;
}

//--------------------------------------------------------------------------
bool CTaskApp::CreateOsTask()
{
	PASSERTSTREAM_AND_RETURN_VALUE(m_pTask != NULL,"CTaskApp::CreateOsTask is called twice",FALSE);

	m_pTask = new COsTask;

	bool res = m_pTask->Create(this);

	if (!res)
	{
		delete m_pTask;
		m_pTask = NULL;
	}

	return res;
}

//--------------------------------------------------------------------------
void CTaskApp::DummyStartTask1(CTaskApp* pCurrentTask)
{
	CTaskApp::DummyStartTask2(pCurrentTask);
}

//--------------------------------------------------------------------------
void CTaskApp::DummyStartTask2(CTaskApp* pCurrentTask)
{
	CTaskApp::DummyStartTask3(pCurrentTask);
}

//--------------------------------------------------------------------------
void CTaskApp::DummyStartTask3(CTaskApp* pCurrentTask)
{
	CTaskApp::DummyStartTask4(pCurrentTask);
}

//--------------------------------------------------------------------------
void CTaskApp::DummyStartTask4(CTaskApp* pCurrentTask)
{
	// we use 5 dummy functions in order to pad the stack with 5 function
	// so every passert will be able to get at least 5 return addresses
	// mark the ostask as running.
	if (!pCurrentTask->GetInit())
	{
		if (pCurrentTask->m_initializing)
		{
			FPASSERTMSG(999, "Last exception was during task init.");

			if (pCurrentTask->GetTaskRecoveryPolicyDuringSystemStartup() == eTerminateProcessDuringStartup ||
				pCurrentTask->GetTaskRecoveryPolicyAfterSeveralRetries() == eTerminateProcess)
			{
				SystemCoreDump(false);
			}

			// this task had exception during last init.
			// we should abort this task.
			pCurrentTask->CTaskApp::SelfKill();

			// the next line should never be executed !!!
			// the task failed killing itself
			FPASSERTMSG(1000, "Task self kill failed");
			COsTask::Suspend();
		}

		pCurrentTask->LockRelevantSemaphore();


		//kobig: removed this print Boris to find another way to do it
//		FTRACESTR(eLevelInfoHigh)<< pCurrentTask->NameOf() << " -> Task created successfully. Queues [Scope:FD] : RcvMbxRead-[" << (int)pCurrentTask->m_pRcvMbxRead->m_scope << ":" << pCurrentTask->m_pRcvMbxRead->m_id
//						<< "]; RcvMbx-[" << (int)pCurrentTask->m_pRcvMbx->m_scope << ":" << pCurrentTask->m_pRcvMbx->m_id
//						<< "]; RspMbxRead-[" << (int)pCurrentTask->m_pRspMbxRead->m_scope << ":" << pCurrentTask->m_pRspMbxRead->m_id
//						<< "]; RspMbx-[" << (int)pCurrentTask->m_pRspMbx->m_scope << ":" << pCurrentTask->m_pRspMbx->m_id << "].";
		 CProcessBase* proc = CProcessBase::GetProcess();

		// if process runs under Valgrind set more time for semaphore alarm
		if (proc->IsUnderValgrid())
			pCurrentTask->m_TicksSemaphoreAlarm = 60 * SECOND;

		pCurrentTask->m_initializing = true;
		// TimeInterval is given in minutes and we want it in Ticks.
		pCurrentTask->SetMaxCrashAllowed(MAXIMUM_CRASH_ALLOWED);

		CProcessBase::GetProcess()->Add(pCurrentTask);
		pCurrentTask->m_pTask->SetStatus(TS_INIT);
		pCurrentTask->InitTask();
		pCurrentTask->SetInit();
		pCurrentTask->m_initializing = false;
		pCurrentTask->m_pTask->SetStatus(TS_WAIT);

		pCurrentTask->UnlockRelevantSemaphore();
	}
	else
	{
		FTRACEINTO << "A new revived task:" << *pCurrentTask;
	}

	pCurrentTask->WaitForEvent();
}

// Task starts at this point. Needed due to compiler
// bugs in assignment of virtual function pointer.
// --------------------------------------------------------------------------
void CTaskApp::DummyStartTask(CTaskApp* pCurrentTask)
{
	return DummyStartTask1(pCurrentTask);
}

bool IsNeedTrace(timeval& tvStart, timeval& tvEnd)
{
	return (tvEnd.tv_sec - tvStart.tv_sec) * 1000000L + tvEnd.tv_usec - tvStart.tv_usec > 500000L;
}

// --------------------------------------------------------------------------
void CTaskApp::WaitForEvent()
{
	m_pid = 0;
	bool   maxTraceLevelRead = false;
	string pMaxTraceLevelStr;

	CProcessBase* pProc = CProcessBase::GetProcess();
	bool bTrace = (pProc && pProc->GetProcessType() == eProcessConfParty);
	CTraceAccumulator* pTraceAcc = NULL;
	timeval tvLastStatistic;
	timerclear(&tvLastStatistic);
	timeval tv500000;
	timerclear(&tv500000);
	tv500000.tv_usec = 500000;
	if (bTrace)
		pTraceAcc = new CTraceAccumulator(IsNeedTrace, trim_pretty_function(__PRETTY_FUNCTION__, ARRAYEND(__PRETTY_FUNCTION__)), this);
	while (!m_selfKill && m_pRcvMbxRead)
	{
		size_t nLocalQueueSize = m_localQueue.size();
		bool bRcvMbxReadTrace = false;
		int nMsgsTreatCount = 0;

		CMessageHeader header;
		if (m_localQueue.empty()) // for internal messages that were sent during InitTask
		{
			bool isTimerTimeout = true;
			TICKS timeout = CalcExpiredTimeout(isTimerTimeout);
			STATUS res = m_pRcvMbxRead->Receive(header, timeout, GetPrivateFileDescriptor());

			LockRelevantSemaphore();
			m_pTask->SetStatus(TS_RUN);

			if (pTraceAcc)
				pTraceAcc->AppendWithTime() << ", m_pRcvMbxRead:\n" << *m_pRcvMbxRead
					   << "\n\tLocal Queue Size before Lock: " << nLocalQueueSize;
			bRcvMbxReadTrace = true;

			if (STATUS_OK == res)
			{
				if (header.IsValid())
				{
					++nMsgsTreatCount;
					if (pTraceAcc)
						pTraceAcc->MSG << ", Receive from socket, OpcodeValue: " << header.m_opcode
							   << ", Buffer size: " << header.m_bufferLen + sizeof header;

					*m_pClientRspMbx         = header.m_sender;
					m_ClientStateMachineDesc = header.m_senderStateMachine;
					m_ClientRspMsgSeqNum     = header.m_RspMsgSeqNum;
					m_ClientRspMsgType       = header.m_msgType;

					DWORD len = (NULL != header.m_segment ? header.m_segment->GetLen() : 0);
					PushMessageToQueue(header.m_opcode, len, header.m_process);

					switch (header.m_msgType)
					{
						case eDirectMessage:
						case eDirectSyncMessage:
							if (header.m_stateMachine.IsValid())
								HandleEventWithStateMachine(header.m_stateMachine, header.m_opcode, header.m_segment);
							else
								HandleEvent(header.m_segment, header.m_bufferLen, header.m_opcode);
							POBJDELETE(header.m_segment);
							break;

						case eAsyncMessage:
						case eSyncDispatcher:
						case eSyncMessageRsp:
							if (Dispatcher(header))
								break;

						default:
							PASSERT(1);
					}
				}
				else
					PASSERT(1);
			}

			else if (STATUS_NEW_CONNECTION == res)
			{
				TRACEINTO << "STATUS_NEW_CONNECTION and continue";
				m_pTask->SetStatus(TS_WAIT);
				UnlockRelevantSemaphore();
				if (pTraceAcc)
					pTraceAcc->Reset();
				continue;
			}

			else if (STATUS_QUEUE_TIMEOUT == res && isTimerTimeout)
			{
				CTimerDesc desc;
				m_pTimer->TimerExpired(desc);
				HandleEventWithStateMachine(desc.m_client, desc.m_type, desc.m_pSegment);
				POBJDELETE(desc.m_pSegment);
			}

			else if (STATUS_PRIVATE_FD == res)
			{
				HandlePrivateFileDescriptor();
			}

			// BRIDGE-15452
			//else if (STATUS_FAIL == res)
			//{
			//	m_pTask->SetStatus(TS_WAIT);
			//	UnlockRelevantSemaphore();
			//}

		}
		else
		{
			LockRelevantSemaphore();
			m_pTask->SetStatus(TS_RUN);
		}

		// Handle all local messages before continue handling other messages
		nLocalQueueSize = m_localQueue.size();
		int nLocalMsgsCount = 0;
		if (pTraceAcc && ! bRcvMbxReadTrace)
			pTraceAcc->MSG << "\n\tm_pRcvMbxRead:" << *m_pRcvMbxRead
				   << "\n\tLocal Queue Size before Lock: " << nLocalQueueSize;

		while (DequeueLocalMessage(header))
		{
			CProcessBase::GetProcess()->m_numLocalMsgRvc++;

			++nMsgsTreatCount;
#ifdef PRFFORMANCE_ANALYSIS
			if (pTraceAcc)
				pTraceAcc->AppendWithTime() << ", RcvLQ: " << header.m_opcode
                                            << "," << header.m_bufferLen + sizeof header
                                            << "," << m_localQueue.size();
#endif
			HandleEvent(header.m_segment, header.m_bufferLen, header.m_opcode);
			++nLocalMsgsCount;

			POBJDELETE(header.m_segment);
		}

		if (pTraceAcc && nMsgsTreatCount > 0)
		{
			pTraceAcc->AppendWithTime() << ", Local queue counters - Size before Dequeue: " << nLocalQueueSize
				   << ", Messages Count: " << nLocalMsgsCount;
			long diff = pTraceAcc->GetTimesDiff();
			tvLastStatistic = pTraceAcc->Flush("Performance analysis: ", __FILE__, __LINE__);
			if (diff > 500000)
			{
				timeradd(&tvLastStatistic, &tv500000, &tvLastStatistic);
				pTraceAcc->SetExplicitTime(tvLastStatistic);
			}
		}
		else if (pTraceAcc)
			pTraceAcc->Reset();

		m_pTask->SetStatus(TS_WAIT);
		UnlockRelevantSemaphore();
	}
	if (pTraceAcc)
		delete pTraceAcc;
	m_pTask->SetStatus(TS_DESTROY);
	SelfKill();

	// the next line should never be executed !!!
	// the task failed killing itself
	FPASSERTMSG(1000, "Task self kill failed");

	COsTask::Suspend();
}

//--------------------------------------------------------------------------
TICKS CTaskApp::CalcExpiredTimeout(bool& isTimerTimeout)
{
	isTimerTimeout = true;

	TICKS timeout = m_pTimer->CalcExpiredTimeout();

	return timeout;
}

//--------------------------------------------------------------------------
void CTaskApp::HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode)
{
	PASSERTMSG_AND_RETURN(opCode == INVALID_OPCODE, "Invalid opcode");

	// 1. try to handle opcodes that are specific to the current task
	if (TaskHandleEvent(pMsg, msgLen, opCode))
		return;

	// 2. try to handle opcodes that are specific to the current task
	// using the state machine
	if (DispatchEvent(opCode, pMsg))
		return;

	if (opCode == DESTROY)   // and not handled by the event dispatcher
	{
		m_resetSource = eResetSourceInternalUnknown;
		m_selfKill    = true;
	}
}

//--------------------------------------------------------------------------
void CTaskApp::SelfKill()
{
	//TRACEINTO << "TaskName:" << GetTaskName() << ", TaskId:" << GetTaskId();
	std::ostringstream sstr;
	sstr  << "TaskName:" << GetTaskName() << ", TaskId:" << GetTaskId() << "; Queues to remove [Scope:FD] : ";

	STATUS rc = STATUS_OK;

	if (m_pRcvMbxRead)
	{
		sstr << "RcvMbxRead-[" << (int)m_pRcvMbxRead->m_scope << ":" << m_pRcvMbxRead->m_id << "]; ";
		m_pRcvMbxRead->Flush();
		rc = m_pRcvMbxRead->Delete();
	}

	if (m_pRcvMbx)
	{
		sstr << "RcvMbx-[" << (int)m_pRcvMbx->m_scope << ":" << m_pRcvMbx->m_id << "]; ";
		rc = m_pRcvMbx->Delete();
	}

	if (m_pRspMbxRead)
	{
		sstr << "RspMbxRead-[" << (int)m_pRspMbxRead->m_scope << ":" << m_pRspMbxRead->m_id << "]; ";
		rc = m_pRspMbxRead->Delete();
	}

	if (m_pRspMbx)
	{
		sstr << "RspMbx-[" << (int)m_pRspMbx->m_scope << ":" << m_pRspMbx->m_id << "]; ";
		rc = m_pRspMbx->Delete();
	}

	TRACESTRFUNC(eLevelInfoHigh) << sstr.str();

	pProcess->Cancel(this);

	// kills the CTaskApp object from the heap
	// in this point in time all heap memory related to this task is free
	delete this;

	COsTask::Exit(); // kills the thread itself

	FPASSERTMSG(1000, "Task self kill failed");

	COsTask::Suspend();
}

//--------------------------------------------------------------------------
void CTaskApp::SetPriority(DWORD priority)
{
}

//--------------------------------------------------------------------------
COsQueue& CTaskApp::GetRcvMbx()
{
	return *m_pRcvMbx;
}

//--------------------------------------------------------------------------
void CTaskApp::Dump(std::ostream& msg) const
{
	msg.setf(std::ios_base::left);

	msg << setw(7)  << m_pid << " "
		<< setw(20) << GetTaskName() << " "
		<< setw(10) << "Max Crush" << setw(4) << m_MaxCrashAllowed << " "
		<< setw(10) << "Cnt Crush" << setw(4) << m_CrashCounter << " ";

	if (m_pTask)
	{
		msg.setf(std::ios_base::hex);
		msg << (void*)(m_pTask->m_id) << " " << (void*)this;
	}

	msg << "\n";
}

//--------------------------------------------------------------------------
void CTaskApp::DumpQueues(std::ostream& msg) const
{
	Dump(msg);
	msg << *m_pRcvMbxRead;
}

//--------------------------------------------------------------------------
void CTaskApp::DumpTimers(std::ostream& msg) const
{
	msg << setw(20) << GetTaskName()  << "\n";
	m_pTimer->Dump(msg);
}

//--------------------------------------------------------------------------
void CTaskApp::DumpStateMachines(std::ostream& msg) const
{
	msg
		<< "State Machines of " << GetTaskName() << ", " << NameOf() << '\n'
		<< "number  |                 name\n"
		<< "--------+---------------------\n";

	for (StateMachinesMap::const_iterator it = m_StateMachinesMap.begin(); it != m_StateMachinesMap.end(); ++it)
	{
		msg << setw(7) << it->first << " | "
			<< setw(20) << it->second->NameOf() << '\n';
	}
}

//--------------------------------------------------------------------------
bool operator ==(const CTaskApp& first, const CTaskApp& second)
{
	return (first.m_pTask->GetTaskId() == second.m_pTask->GetTaskId());
}

//--------------------------------------------------------------------------
bool operator <(const CTaskApp& first, const CTaskApp& second)
{
	return (first.m_pTask->GetTaskId() < second.m_pTask->GetTaskId());
}

//--------------------------------------------------------------------------
STATUS CTaskApp::ResponedClientRequest(OPCODE opcode, CSegment* pMsg)
{
	return ResponedClientRequest(
		*m_pClientRspMbx,
		m_ClientRspMsgSeqNum,
		m_ClientRspMsgType,
		&m_ClientStateMachineDesc,
		opcode,
		pMsg);
}

//--------------------------------------------------------------------------
STATUS CTaskApp::ResponedClientRequest(
	const COsQueue& pClientRspMbx,
	DWORD ClientRspMsgSeqNum,
	DWORD ClientRspMsgType,
	StateMachineDescriptor* clientStateMachineDesc,
	OPCODE opcode,
	CSegment* pMsg)
{
	// Client response mail box must be valid
	PASSERT_AND_RETURN_VALUE(!pClientRspMbx.IsValid(), STATUS_INVALID_MAILBOX);

	CTaskApi api;
	if (ClientRspMsgType == eSyncMessage || ClientRspMsgType == eDirectSyncMessage)
	{
		PASSERT_AND_RETURN_VALUE(!ClientRspMsgSeqNum, STATUS_RESPONSE_SEQ_NUMBER_IS_EMPTY);

		char buf[300];
		snprintf(
			buf, sizeof(buf),
			"Task %s sends rsp message: seq num %d opcode %d",
			GetTaskName(), ClientRspMsgSeqNum, opcode);
		PTRACE(eLevelInfoNormal, buf);
	}

	api.CreateOnlyApi(
		pClientRspMbx,
		*clientStateMachineDesc,
		NULL,
		0,
		ClientRspMsgSeqNum);

	if (clientStateMachineDesc && clientStateMachineDesc->IsValid())
		return api.SendMsgWithStateMachine(pMsg, opcode);

	if (pMsg)
		return api.SendMsg(pMsg, opcode);

	return api.SendOpcodeMsg(opcode);
}

//--------------------------------------------------------------------------
DWORD CTaskApp::GetTaskId() const
{
	if (m_pTask)
		return (m_pTask->m_id);

	return -1;
}

//--------------------------------------------------------------------------
DWORD CTaskApp::GetNextRspMsgSeqNum()
{
	if (m_RspMsgSeqNum == static_cast<DWORD>(-1))
		m_RspMsgSeqNum = 0;

	return ++m_RspMsgSeqNum;
}

//--------------------------------------------------------------------------
DWORD CTaskApp::GetCurrentRspMsgSeqNum()
{
	return m_RspMsgSeqNum;
}

//--------------------------------------------------------------------------
DWORD CTaskApp::GetClientRspMsgSeqNum()
{
	return m_ClientRspMsgSeqNum;
}

//--------------------------------------------------------------------------
void CTaskApp::QueueLocalMessage(const CMessageHeader& header)
{
	m_localQueue.push_back(header);
}

//--------------------------------------------------------------------------
BOOL CTaskApp::DequeueLocalMessage(CMessageHeader& header)
{
	if (m_localQueue.empty())
		return false;

	header = *m_localQueue.begin();
	m_localQueue.pop_front();

	return true;
}

//--------------------------------------------------------------------------
void CTaskApp::HandleEventWithStateMachine(const StateMachineDescriptor stateMachineDesc, OPCODE opcode, CSegment* pMsg)
{
	CStateMachine* pStateMachine  = (CStateMachine*)stateMachineDesc.m_pStateMachine;
	DWORD sequenceNumber = stateMachineDesc.m_StateMachineHandle;

	std::map<DWORD, const CStateMachine*>::iterator itr = m_StateMachinesMap.find(sequenceNumber);

	if (itr != m_StateMachinesMap.end())
	{
		if (pStateMachine == itr->second)
		{
			if (CPObject::IsValidPObjectPtr(pStateMachine)) // regular flow
			{
				pStateMachine->DispatchEvent(opcode, pMsg);
			}
			else                                            // should not happen
			{
				PASSERTSTREAM(true, "Invalid task state machines, opcode " << opcode);
				SystemCoreDump(true, true);
			}
		}
		else // should not happen
		{
			const CStateMachine* temp = itr->second;

			PASSERTSTREAM(true, "Message and task state machines are inconsistent, opcode " << opcode);
			SystemCoreDump(true, true);
		}
	}
	else // could be - state machine was deleted, just before getting this message
	{
		TRACEINTOFUNC
			 << "ptr:" << pStateMachine
			<< ", seq:" << sequenceNumber
			<< ", opcode:" << opcode;
	}
}

//--------------------------------------------------------------------------
void CTaskApp::LockRelevantSemaphore()
{
	if (m_Thread_Group == eTaskGroupNone)
		return;

	// Stores current time (for checking when UnlockRelevantSemaphore)
	m_SemaphoreLastLockedTicks = SystemGetTickCount().GetIntegerPartForTrace();

	CProcessBase* pProc = CProcessBase::GetProcess();
	if (!pProc)
		return;

	int sid = pProc->GetGroupSID(m_Thread_Group);

	STATUS stat = LockSemaphore(sid);
	if (stat != STATUS_OK && pProc->GetProcessStatus() != eProcessTearDown)
		SystemCoreDump(false);
	else
		m_SemaphoreTaskThatDidLastOp = GetTaskId();
}

//--------------------------------------------------------------------------
void CTaskApp::UnlockRelevantSemaphore()
{
	if (m_Thread_Group == eTaskGroupNone)
		return;

	// Checks time elapsed since semaphore was locked
	DWORD curTicks  = SystemGetTickCount().GetIntegerPartForTrace();
	DWORD diffTicks = curTicks - m_SemaphoreLastLockedTicks;

	// more than 10 (60 under valgrind)
	// seconds elapsed since the semaphore was locked
	if (m_TicksSemaphoreAlarm <= diffTicks)
	{
		std::ostringstream buf;
		DumpOpcodeTail(buf, 20);

		TRACEWARN << diffTicks << " ticks " << "(" << diffTicks / SECOND
			<< " seconds) passed between locking and unlocking, start tick "
			<< m_SemaphoreLastLockedTicks << ", end tick " << curTicks
			<< "\n" << buf.str();
	}

	CProcessBase* pProc = CProcessBase::GetProcess();
	STATUS stat = STATUS_OK;

	int sid = 0;
	if (pProc)
	{
		sid = pProc->GetGroupSID(m_Thread_Group);
		stat = UnlockSemaphore(sid);
	}

	if (stat != STATUS_OK && pProc->GetProcessStatus() != eProcessTearDown)
		SystemCoreDump(false);
}

//--------------------------------------------------------------------------
STATUS CTaskApp::UndoRelevantSemaphore()
{
	if (m_Thread_Group == eTaskGroupNone)
		return STATUS_FAIL;

	DWORD taskId = GetTaskId();
	if (taskId != m_SemaphoreTaskThatDidLastOp)
		return STATUS_OK;

	// Current task ID did the last sem lock
	int sid = CProcessBase::GetProcess()->GetGroupSID(m_Thread_Group);

	return UndoSemaphore(sid);
}

//--------------------------------------------------------------------------
void CTaskApp::FlushQueue()
{
	m_pRcvMbxRead->Flush();
}

//--------------------------------------------------------------------------
void CTaskApp::SetDestroyReason(const std::string& reason)
{
	m_destroyReason = reason;
}

//--------------------------------------------------------------------------
const std::string& CTaskApp::GetDestroyReason()
{
	return m_destroyReason;
}

//--------------------------------------------------------------------------
void CTaskApp::PushMessageToQueue(OPCODE opcode, DWORD len, eProcessType processType)
{
	m_pMessageQueue->PushMessage(opcode, len, processType);
}

//--------------------------------------------------------------------------
void CTaskApp::AddFilterOpcodeToQueue(OPCODE opcode) const
{
	m_pMessageQueue->AddFilter(opcode);
}

void CTaskApp::InitTaskMessagePool()
{
	AddFilterOpcodeToQueue(TERMINAL_COMMAND);
	AddFilterOpcodeToQueue(MPLAPI_MSG);
	AddFilterOpcodeToQueue(TASK_CHANGE_STATE_FAULT_IND);

	AddFilterOpcodePoint();
}

//--------------------------------------------------------------------------
void CTaskApp::DumpOpcodeTail(std::ostream& ostr, DWORD numOfMessages) const
{
	if (m_pMessageQueue)
		m_pMessageQueue->DumpMessagePool(ostr, numOfMessages);
	else
		ostr << "m_pMessageQueue == NULL";
}

//--------------------------------------------------------------------------
DWORD CTaskApp::GetOpcodeTailLen() const
{
	return m_pMessageQueue->GetLength();
}

//--------------------------------------------------------------------------
DWORD CTaskApp::AddStateMachine(const CStateMachine* pStateMachine)
{
	m_StateMachinesMap[m_NextStateMachineHandle] = pStateMachine;
	return m_NextStateMachineHandle++;
}

//--------------------------------------------------------------------------
void CTaskApp::DeleteStateMachine(DWORD sequenceNumber, const CStateMachine* pStateMachine)
{
	if (m_StateMachinesMap[sequenceNumber] != pStateMachine)
	{
		PASSERT(sequenceNumber);
	}
	else
	{
		StateMachinesMap::iterator it = m_StateMachinesMap.find(sequenceNumber);

		if (it != m_StateMachinesMap.end())
			m_StateMachinesMap.erase(it);
		else
			PASSERT(sequenceNumber);
	}
}

//--------------------------------------------------------------------------
void CTaskApp::IncrementCrashCounter()
{
	DWORD now = SystemGetTickCount().GetSeconds();

	// in a first crash don't zero the counter
	static bool isFirstTimeHere = true;

	DWORD maxTimeBetweenCrush = 30 * 60;
	CSysConfig* sysCfg = CProcessBase::GetProcess()->GetSysConfig();
	sysCfg->GetDWORDDataByKey("MAX_TIME_BETWEEN_TASK_CRUSH", maxTimeBetweenCrush);

	if (true != isFirstTimeHere &&
		maxTimeBetweenCrush < now - m_LastCrashTime)
		ZeroCrashCounter();

	isFirstTimeHere = false;
	m_LastCrashTime = now;
	IncreaseCrashCounter();
}
