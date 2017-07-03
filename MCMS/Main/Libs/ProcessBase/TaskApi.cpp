#include "TaskApi.h"

#include "TaskApp.h"
#include "OsQueue.h"
#include "MessageHeader.h"

#include "OpcodesMcmsCommon.h"

#include "StatusesGeneral.h"
#include "InternalProcessStatuses.h"
#include "ApiStatuses.h"

#include "ProcessBase.h"
#include "OsTask.h"

#include "NStream.h"
#include "ObjString.h"

#include "Trace.h"
#include "TraceStream.h"
#include "PrettyTable.h"

#include "Macros.h"

#include "SysConfig.h"
#include "SysConfigKeys.h"

#include <map>
#include <iomanip>
#include <signal.h>
#include <fstream>


static BOOL g_isPrintMessageQueueStatistics = 0xFF;

// Static.
STATUS CTaskApi::SendMsgWithTrace(eProcessType            ptype,
                                  eOtherProcessQueueEntry qtype,
                                  CSegment*               msg,
                                  OPCODE                  opcode)
{
  CProcessBase* proc = CProcessBase::GetProcess();
  FPASSERT_AND_RETURN_VALUE(NULL == proc, STATUS_FAIL);

  STATUS stat = CTaskApi(ptype, qtype).SendMsg(msg, opcode);
  FPASSERTSTREAM_AND_RETURN_VALUE(STATUS_OK != stat,
    "Unable to send " << proc->GetOpcodeAsString(opcode)
    << " (" << opcode << ") to " << ProcessNames[ptype]
    << "-" << InfrastructuresTaskNames[qtype]
    << ": " << proc->GetStatusAsString(stat),
    stat);

  FTRACESTRFUNC(eLevelDebug)
    << "Message " << proc->GetOpcodeAsString(opcode)
    << " (" << opcode << ") to "
    << ProcessNames[ptype] << "-" << InfrastructuresTaskNames[qtype]
    << " was sent successfully";

  return STATUS_OK;
}

CTaskApi::CTaskApi()
{
	m_pRcvMbx            = NULL;
	m_pRspMbx            = NULL;
	m_pLocalRcvMbx       = NULL;
	m_pTaskApp           = NULL;
	m_lastSendError      = STATUS_OK;
	m_ClientRspMsgSeqNum = 0;
	m_process            = eProcessTypeInvalid;
	m_entry              = (eOtherProcessQueueEntry)-1;
}

CTaskApi::CTaskApi(eProcessType process, eOtherProcessQueueEntry queueType)
{
	m_pRcvMbx            = NULL;
	m_pRspMbx            = NULL;
	m_pLocalRcvMbx       = NULL;
	m_pTaskApp           = NULL;
	m_lastSendError      = STATUS_OK;
	m_ClientRspMsgSeqNum = 0;
	m_process            = eProcessTypeInvalid;
	m_entry              = (eOtherProcessQueueEntry)-1;

	const COsQueue* queue = CProcessBase::GetProcess()->GetOtherProcessQueue(process, queueType);
	if (NULL != queue)
		CreateOnlyApi(*queue);
}

CTaskApi::CTaskApi(const CTaskApi& other)
	: CPObject(other)
{
	if (other.m_pRcvMbx)
	{
		m_pRcvMbx  = new COsQueue;
		*m_pRcvMbx = *(other.m_pRcvMbx);
	}
	else
		m_pRcvMbx = NULL;

	if (other.m_pRspMbx)
	{
		m_pRspMbx  = new COsQueue;
		*m_pRspMbx = *(other.m_pRspMbx);
	}
	else
		m_pRspMbx = NULL;

	m_pLocalRcvMbx       = other.m_pLocalRcvMbx;
	m_pTaskApp           = other.m_pTaskApp;
	m_StateMachineDesc   = other.m_StateMachineDesc;
	m_lastSendError      = STATUS_OK;
	m_ClientRspMsgSeqNum = other.m_ClientRspMsgSeqNum;
	m_process            = other.m_process;
	m_entry              = other.m_entry;
}

// Virtual
CTaskApi::~CTaskApi()
{
	if (m_pRspMbx)
	{
		STATUS rc = m_pRspMbx->Delete();
		delete m_pRspMbx;
		m_pRspMbx = NULL;
	}

	delete m_pRcvMbx;
	m_pRcvMbx = NULL;
}

// Creates api instance but not a task app.
// <syncCall> enable/disable sync call to taskapp.
void CTaskApi::CreateOnlyApi(const COsQueue& rcvMbx,
                             const StateMachineDescriptor& stateMachineDesc,
                             LocalQueue* pLocalRcvMbx,
                             WORD syncCall,
                             DWORD ClientRspMsgSeqNum)
{
	m_StateMachineDesc   = stateMachineDesc;
	m_ClientRspMsgSeqNum = ClientRspMsgSeqNum;

	if (!m_pRcvMbx)
		m_pRcvMbx = new COsQueue(rcvMbx);
	else
		*m_pRcvMbx = rcvMbx;

	if (pLocalRcvMbx)
		m_pLocalRcvMbx = pLocalRcvMbx;
}

// Creates api instance but not a task app.
// <syncCall> enable/disable sync call to taskapp.
void CTaskApi::CreateOnlyApi(const COsQueue& rcvMbx,
                             CStateMachine* pStateMachine,
                             LocalQueue* pLocalRcvMbx,
                             WORD syncCall,
                             DWORD ClientRspMsgSeqNum)
{
	StateMachineDescriptor stateMachineDesc;

	if (pStateMachine)
		stateMachineDesc = pStateMachine->GetStateMachineDescriptor();

	CreateOnlyApi(rcvMbx, stateMachineDesc, pLocalRcvMbx, syncCall, ClientRspMsgSeqNum);
}

// Destroys api instance but not a task app.
void CTaskApi::DestroyOnlyApi()
{
	if (m_pRspMbx)
	{
		PASSERT(m_pRspMbx->Delete());
		POBJDELETE(m_pRspMbx);
	}
}

void CTaskApi::Create(TASK_ENTRY_POINT entryPoint, const COsQueue& creatorRcvMbx)
{
	// General taskApi creation
	CTaskApi::Create(creatorRcvMbx);
	// Class params are irrelevant thus entryPoint is immediately called.
	LoadApp(entryPoint);
}

// Creats api + task app
// <syncCall> enable/disable sync call to taskapp.
void CTaskApi::Create(const COsQueue& creatorRcvMbx, WORD syncCall)
{
	creatorRcvMbx.Serialize(m_appParam);
}

// Destroys api instance + task app.
void CTaskApi::Destroy(const char* reason)
{
	SendOpcodeMsg(DESTROY);
	DestroyOnlyApi();
}

// Destroys son task (wait until the son is terminated
void CTaskApi::SyncDestroy(const char* reason, BOOL free_sem)
{
	PASSERT_AND_RETURN(NULL == m_pTaskApp);

	SendOpcodeMsg(DESTROY);

	{
		// Allows to run other tasks during the operation
		CTaskApp::Unlocker unlocker(free_sem);
		if ( CPObject::IsValidPObjectPtr(m_pTaskApp) && NULL != m_pTaskApp->GetOsTask())
		   COsTask::Join(m_pTaskApp->GetOsTask()->m_id);
	}

	DestroyOnlyApi();
}

// Destroys son task (wait until the son is terminated)
void CTaskApi::SyncDestroyTaskID(DWORD m_id, BOOL free_semaphre)
{
	CTaskApp* currentTask = NULL;

	// 28.11.11 Rachel Cohen after a bug from corporate that show that RX task has been closed after 9 minutes
	// we decided to send a signal to the task to enter self kill
	// instead of sending it in command.
	COsTask::SendSignal(m_id, SIGALRM);

	/*if (free_semaphre)
		currentTask = CProcessBase::GetProcess()->GetCurrentTask();

	DWORD thread_handle = m_id;
	SendOpcodeMsg(DESTROY);

	if (currentTask)
		currentTask->UnlockRelevantSemaphore();

	COsTask::Join(thread_handle);

	if (currentTask)
		currentTask->LockRelevantSemaphore();*/



	DestroyOnlyApi();
}

void CTaskApi::LoadApp(TASK_ENTRY_POINT entryPoint)
{
	(*entryPoint)((void*)&m_appParam);

	POBJDELETE(m_pRcvMbx);

	// set return values
	m_pRcvMbx = new COsQueue;
	m_pRcvMbx->DeSerialize(m_appParam); // get application recieve mbx

	void* ptr;
	m_appParam >> ptr;                  // get application object pointer
	m_pTaskApp = (CTaskApp*)ptr;
}

#if 0
STATUS CTaskApi::SendMsg(
	CSegment* msg,
	OPCODE opcode,
	const COsQueue* myQueue,
	const StateMachineDescriptor* myStateMachineDesc,
	WORD numOfRetries,
	OPCODE sub_opcode) const
{
	// Romem's patch - disabled Ron's mechanism - vngr-26631
	numOfRetries = 0;

	if (0 == numOfRetries)
		// no retry - keep it simple
		return Send(msg, opcode, NULL, myQueue, myStateMachineDesc, true);
	else // send msg with retry
	{
		STATUS retVal = STATUS_OK;
		for (WORD i = 0; i < numOfRetries+1; i++)
		{
			if (i == numOfRetries) // last trial - send and assert on failure
			{
				retVal = Send(msg, opcode, NULL, myQueue, myStateMachineDesc, true);
				if (STATUS_SOCKET_WOULD_BLOCKED == retVal)
					PTRACE2INT(
						eLevelError,
						"CTaskApi::SendMsg got STATUS_SOCKET_WOULD_BLOCKED - on last trial = ",
						i+1);
				else if (STATUS_OK == retVal)
					PTRACE2INT(eLevelError,
						"CTaskApi::SendMsg got STATUS_OK - on last trial = ",
						i+1);
				else
					PTRACE2INT(eLevelError, "CTaskApi::SendMsg failed - on last trial = ",
						i+1);
			}
			else   // not last trial
			{
				// create copy of segment (segment delete on Send function)
				CSegment* msgCopy = NULL;
				if (NULL != msg)
					msgCopy = new CSegment(*msg);

				retVal = Send(msgCopy, opcode, NULL, myQueue, myStateMachineDesc, false);

				if (STATUS_SOCKET_WOULD_BLOCKED != retVal) // no block - continue
				{
					if (i > 0 && STATUS_OK == retVal)
						PTRACE2INT(eLevelError,
							"CTaskApi::SendMsg got STATUS_OK - on try = ",
							i+1);

					POBJDELETE(msg);
					break;
				}
				else // STATUS_SOCKET_WOULD_BLOCKED == retVal
				{
					// prevent more trace when trace message failed:
					// dont use SystemSleep because of sleep
					// set to STATUS_OK - atherwise more prints will sent because of status
					if (TRACE_MESSAGE == opcode)
					{
						retVal = STATUS_OK;
						POBJDELETE(msg);
						break;
					}

					SystemSleep(1);
				} // STATUS_SOCKET_WOULD_BLOCKED == retVal

			}
		}

		return retVal;
	}

	// never get here
	DBGPASSERT(1);
	return STATUS_OK;
}
#endif

STATUS CTaskApi::SendMsgWithStateMachine(CSegment* msg, OPCODE opcode) const
{
	return m_StateMachineDesc.IsValid() ? Send(msg, opcode, &m_StateMachineDesc) : STATUS_FAIL;
}

STATUS CTaskApi::Send(CSegment* msg,
                      OPCODE opcode,
                      const StateMachineDescriptor* pStateMachineDescriptor,
                      const COsQueue* myQueue,
                      const StateMachineDescriptor* myStateMachineDescriptor,
                      bool assert_on_would_block) const
{
  unsigned int msg_len  = msg ? msg->GetLen() : 0;
  STATUS stat     = STATUS_FAIL;
  CProcessBase* pProcess = CProcessBase::GetProcess();
  eProcessType proType  = eProcessTypeInvalid;

  if (m_pRcvMbx == NULL)
  {
    if (m_entry != (eOtherProcessQueueEntry) -1 && m_process !=
        eProcessTypeInvalid)
    {
      const COsQueue* queue = pProcess->GetOtherProcessQueue(m_process, m_entry);
      if (queue == NULL)
      {
        POBJDELETE(msg);
        return STATUS_FAIL;
      }

      if (queue->IsValid())
      {
        m_pRcvMbx  = new COsQueue;
        *m_pRcvMbx = *(queue);
      }
    }
    else
      POBJDELETE(msg);
  }

  if (m_pRcvMbx != NULL)
  {
    proType = m_pRcvMbx->m_process;
    stat    = m_pRcvMbx->Send(msg,                      // the segment
                              opcode,
                              myQueue,                  // response mailbox
                              pStateMachineDescriptor,  // address state machine
                              myStateMachineDescriptor, // response state machine
                              eAsyncMessage,
                              FALSE,
                              m_ClientRspMsgSeqNum);    // don't delete segment in failure

    m_lastSendError = stat;
    if (stat != STATUS_OK)
    {
      if (stat == STATUS_BROKEN_PIPE || stat == STATUS_TRANSPORT_NOTCONNECTED)
      {
        if (m_pRcvMbx->m_process != eProcessTypeInvalid)
        {
          eProcessType process = m_pRcvMbx->m_process;
          eOtherProcessQueueEntry entry = pProcess->FindQueueEntry(process,
                                                                   *m_pRcvMbx);
          pProcess->RemoveBrokenQueue(process);

          if (entry != (eOtherProcessQueueEntry) - 1)
          {
            m_process = process;
            m_entry   = entry;
            const COsQueue* queue = pProcess->GetOtherProcessQueue(process,
                                                                   entry);
            if (queue->IsValid())
            {
              *m_pRcvMbx = *(queue);
              stat       = m_pRcvMbx->Send(msg,                      // the segment
                                           opcode,
                                           myQueue,                  // response mailbox
                                           pStateMachineDescriptor,  // address state machine
                                           myStateMachineDescriptor, // response state machine
                                           eAsyncMessage,
                                           FALSE,
                                           m_ClientRspMsgSeqNum);

           	  if (stat == STATUS_BROKEN_PIPE || stat == STATUS_TRANSPORT_NOTCONNECTED)
           	  {
           		  POBJDELETE(m_pRcvMbx);
           		  m_pRcvMbx = NULL;
           		  return STATUS_FAIL;		//Judith - return STATUS_FAIL in case we failed again to use the socket
              }
            }
            else	//queue is not valid.
            {

              POBJDELETE(m_pRcvMbx);
              m_pRcvMbx = NULL;
              POBJDELETE(msg);
              return STATUS_FAIL;
            }
          }
          else
          {
            // maybe we need to resend (Dispatcher queue will be recreate now)
            stat = m_pRcvMbx->Send(msg,                       // the segment
                                   opcode,
                                   myQueue,                   // response mailbox
                                   pStateMachineDescriptor,   // address state machine
                                   myStateMachineDescriptor,  // response state machine
                                   eAsyncMessage,
                                   FALSE,
                                   m_ClientRspMsgSeqNum);

			if (stat == STATUS_BROKEN_PIPE || stat == STATUS_TRANSPORT_NOTCONNECTED)		//in case of failure, the Send delete the segment.
			{
			  POBJDELETE(m_pRcvMbx);
			  m_pRcvMbx = NULL;
			  POBJDELETE(msg);
			  return STATUS_FAIL;	//Judith - return STATUS_FAIL in case we failed again to use the socket
            }
          }
        }
        else
          // the target is not MCMS process, ClientLogger
          POBJDELETE(msg);
      }
      else
        POBJDELETE(msg);
    }
  }


  if (stat == STATUS_OK)
  {
    pProcess->m_numMessageSent++;
    if(pProcess->GetProcessType() != eProcessApacheModule) // BRIDGE-11710 in apache threads are not sync two threads can access the member cause coredump
	{
		pProcess->IncreaseMsgsSentConuter(opcode);
	}
  }

  if (proType != eProcessLogger &&
      proType != eProcessFaults &&
      proType != eProcessClientLogger)
  {
    if (proType == eProcessConfParty)
    {
      if (g_isPrintMessageQueueStatistics == 0xFF)
      {
        CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
        if (sysConfig)
          sysConfig->GetBOOLDataByKey(CFG_PRINT_MESSAGE_QUEUE_STATISTICS,
                                      g_isPrintMessageQueueStatistics);
      }

      if (g_isPrintMessageQueueStatistics)
      {
        static unsigned int counter;

        typedef std::map<std::pair<OPCODE, OPCODE>, unsigned int> OpcodeCounter;
        typedef std::map<std::pair<OPCODE,
                                   OPCODE>,
                                   unsigned int>::iterator OpcodeCounterItr;

        static OpcodeCounter opcodeCounter;

        OPCODE subOpcode        = 0;
        const WORD PARTY_MSG        = 5050;
        const WORD CONFAPP_MNGR_MSG = 5068;

        switch (opcode)
        {
          case ACK_IND:
          {
            if(NULL == msg){
        	   return stat;
            }
            *msg >> subOpcode;
            msg->ResetRead();
            break;
          }

          case PARTY_MSG:
          {
            if(NULL == msg){
            	return stat;
            }
            CTaskApp* pParty;
            WORD msgSubOpcode;
            *msg >> (void*&)pParty;
            *msg >> msgSubOpcode;
            subOpcode = msgSubOpcode;
            msg->ResetRead();
            break;
          }

          case CONFAPP_MNGR_MSG:
          {
            FPASSERT_AND_RETURN_VALUE(NULL == msg,stat);
            DWORD event;
            DWORD notifyOpcode;
            *msg >> event;
            *msg >> notifyOpcode;
            subOpcode = notifyOpcode;
            msg->ResetRead();
            break;
          }
        } // switch

        std::pair<OpcodeCounterItr, bool> res =
          opcodeCounter.insert(std::make_pair(std::make_pair(opcode, subOpcode), 0));
        res.first->second++;

        // Clears counters if one of them reach 10000.
        if (res.first->second > 10000)
          opcodeCounter.clear();

        if (counter != 0 && 0 == counter%1000)
        {

          COstrStream ostrTasksQueues;
          pProcess->DumpTasksQueues(ostrTasksQueues);
          TRACESTRFUNC(eLevelError)  << " Queues Statistic: \n" << ostrTasksQueues.str() << endl;

          CPrettyTable<unsigned int, const char*, OPCODE>
          tbl2("count", "opcode", "sub-opcode");

          for (OpcodeCounterItr ii = opcodeCounter.begin();
               ii != opcodeCounter.end(); ++ii)
            tbl2.Add(ii->second, pProcess->GetOpcodeAsString(ii->first.first).c_str(),
                                                             ii->first.second);

          TRACEWARN << "QueueID:"       << m_pRcvMbx->m_id
                    << ", QueueIDType:" << m_pRcvMbx->m_idType
                    << ", DstProcess:"  << pProcess->GetProcessName(m_pRcvMbx->m_process)
                    << ", SrcProcess:"  << pProcess->GetProcessName(m_pRcvMbx->m_scope)
                    << "\n" << tbl2.Get();
        }

        counter++;
      }
    }	//if (proType == eProcessConfParty)

    if (stat == STATUS_SOCKET_WOULD_BLOCKED)
    {
      static unsigned int counter;
      const unsigned int size = 40;

      // Asserts once per size times.
      PASSERTSTREAM(0 == counter%size,
        "QueueID:"          << m_pRcvMbx->m_id
        << ", QueueIDType:" << m_pRcvMbx->m_idType
        << ", DstProcess:"  << pProcess->GetProcessName(m_pRcvMbx->m_process)
        << ", SrcProcess:"  << pProcess->GetProcessName(m_pRcvMbx->m_scope)
        << ", Opcode:"      << pProcess->GetOpcodeAsString(opcode).c_str());

      // Collects and prints dropped messages statistics only in eLevelInfoHigh
      // and lower.

      /*static CPrettyTable<unsigned int,
                           OPCODE,
                           const char*,
                           unsigned int,
                           STATUS,
                           const char*,
                           const char*>
       tbl("num", "opcode", "opcode desc", "length", "status", "status desc", "destination");

      if (tbl.Size() < size)
      {
		  tbl.Add(counter,
				  opcode,
				  pProcess->GetOpcodeAsString(opcode).c_str(),
				  msg_len,
				  stat,
				  pProcess->GetStatusAsString(stat).c_str(),
				  pProcess->GetProcessName(proType));
      }


		  if (pProcess->GetMaxLogLevel() >= eLevelInfoHigh)
		  {
			// Prints 'size' lines table.
			if (counter > 0 && 0 == counter%size)
			{
			  std::ostringstream cap;
			  cap << counter-size+1 << " - " << counter;
			  tbl.SetCaption(cap.str().c_str());

			  FTRACESTRFUNC(eLevelError) << "\n" << tbl.Get();
			  tbl.Clear();
			  //internalCounter = 0;
			}
		  }*/

		  ++counter;
		} // end of if would block
  	}
    return stat;
  	}





void CTaskApi::SendLocalMessage(CSegment* msg, OPCODE opcode) const
{
	CMessageHeader header;
	header.m_opcode    = opcode;
	header.m_segment   = msg;
	header.m_bufferLen = msg ? msg->GetLen() : 0;
	if (m_pLocalRcvMbx)
		m_pLocalRcvMbx->push_back(header);
}

void CTaskApi::StaticSendLocalMessage(CSegment* msg, OPCODE opcode)
{
	CTaskApp* currentTask = CProcessBase::GetProcess()->GetCurrentTask();
	FPASSERT_AND_RETURN(!currentTask);

	CMessageHeader header;
	header.m_opcode    = opcode;
	header.m_segment   = msg;
	header.m_bufferLen = msg ? msg->GetLen() : 0;
	currentTask->QueueLocalMessage(header);
}

STATUS CTaskApi::SendMessageSync(CSegment* msg,
                                 OPCODE opcode,
                                 TICKS timeOut) const
{
	OPCODE   dummy;
	CSegment rspMsg;
	return SendMessageSync(msg, opcode, timeOut, dummy, rspMsg);
}

STATUS CTaskApi::SendMessageSync(CSegment* msg,
                                 OPCODE opcode,
                                 TICKS timeOut,
                                 OPCODE& resOpcode) const
{
	CSegment rspMsg;
	return SendMessageSync(msg, opcode, timeOut, resOpcode, rspMsg);
}

STATUS CTaskApi::SendMessageSync(CSegment* msg,
                                 OPCODE opcode,
                                 TICKS timeOut,
                                 OPCODE& respondeOpcode,
                                 CSegment& rspMsg) const
{
	STATUS res = STATUS_FAIL;

	CTaskApp* currentTask = CProcessBase::GetProcess()->GetCurrentTask();

	if (currentTask == NULL)
	{
		POBJDELETE(msg);
		PASSERTSTREAM(1, "Opcode:" << opcode << " - Failed to send sync message without being in a task scoop");
		return STATUS_FAIL;
	}

	const COsQueue* rspMbx     = currentTask->GetRspMbx();
	const COsQueue* rspMbxRead = currentTask->GetRspMbxRead();

	if (m_pRcvMbx && rspMbx && rspMbxRead)
	{
		DWORD MsgSeqNum = currentTask->GetNextRspMsgSeqNum();

		char  buf[300];
		snprintf(buf, sizeof(buf),
		         "Task %s sends sync message:  send seq num %d opcode: %d ",
		         currentTask->GetTaskName(), MsgSeqNum, opcode);
		PTRACE(eLevelInfoNormal, buf);

		STATUS err = m_pRcvMbx->Send(msg,
		                             opcode,
		                             currentTask->GetRspMbx(),
		                             &m_StateMachineDesc,
		                             NULL,
		                             eSyncMessage,
		                             FALSE,
		                             MsgSeqNum);

		m_lastSendError = err;
		if (err != STATUS_OK)
		{
			if (err == STATUS_BROKEN_PIPE || err == STATUS_TRANSPORT_NOTCONNECTED)
			{
				if (m_pRcvMbx->m_process != eProcessTypeInvalid)
				{
					CProcessBase* pProcess = CProcessBase::GetProcess();

					eProcessType process = m_pRcvMbx->m_process;
					eOtherProcessQueueEntry entry = pProcess->FindQueueEntry(process, *m_pRcvMbx);
					pProcess->RemoveBrokenQueue(process);

					if (entry != (eOtherProcessQueueEntry) -1)
					{
						m_process = process;
						m_entry   = entry;
						const COsQueue* queue = pProcess->GetOtherProcessQueue(process, entry);
						if (queue->IsValid())
						{
							*m_pRcvMbx = *(queue);
							err = m_pRcvMbx->Send(msg,
							                      opcode,
							                      currentTask->GetRspMbx(),
							                      &m_StateMachineDesc,
							                      NULL,
							                      eSyncMessage,
							                      FALSE,
							                      MsgSeqNum);
							if (err == STATUS_BROKEN_PIPE || err == STATUS_TRANSPORT_NOTCONNECTED)
							{
								POBJDELETE(m_pRcvMbx);
								m_pRcvMbx = NULL;
							}
						}
						else
						{
							POBJDELETE(m_pRcvMbx);
							m_pRcvMbx = NULL;
							POBJDELETE(msg);
						}
					}
					else
					{
						// maybe we need to resend (Dispatcher queuer will be recreate now)
						err = m_pRcvMbx->Send(msg,
						                      opcode,
						                      currentTask->GetRspMbx(),
						                      &m_StateMachineDesc,
						                      NULL,
						                      eSyncMessage,
						                      FALSE,
						                      MsgSeqNum);

						if (err == STATUS_BROKEN_PIPE || err == STATUS_TRANSPORT_NOTCONNECTED)
						{
							POBJDELETE(m_pRcvMbx);
							m_pRcvMbx = NULL;
						}

					}
				}
				else
					// the target is not MCMS process, ClientLogger
					POBJDELETE(msg);
			}
			else
				POBJDELETE(msg);
		}
		else
		{
			CProcessBase::GetProcess()->m_numSyncMsgSent++;
			CMessageHeader header;
			TICKS AbsTimeOut = SystemGetTickCount() + timeOut;
			TICKS CurTime    = 0;

			do
			{
				currentTask->UnlockRelevantSemaphore();
				res = rspMbxRead->Receive(header, timeOut);
				currentTask->LockRelevantSemaphore();

				if (res == STATUS_OK && header.IsValid())
				{
					char buf2[400];
					snprintf(buf2,
					         sizeof(buf2),
					         "Task %s receives rsp message:  send seq num %d  rsp seq num %d opcode %d Process %s",
					         currentTask->GetTaskName(),
					         MsgSeqNum,
					         header.m_RspMsgSeqNum,
					         header.m_opcode,
					         ProcessNames[header.m_process]);

					PTRACE(eLevelInfoNormal, buf2);
					if (header.m_RspMsgSeqNum == MsgSeqNum)
					{
						respondeOpcode = header.m_opcode;
						CProcessBase::GetProcess()->m_numToutSyncMsg++;
						if (header.m_segment)
						{
							rspMsg = *(header.m_segment);
							POBJDELETE(header.m_segment);
						}
						break;
					}
					else
					{
						char buf1[200];
						snprintf(buf1,
						         sizeof(buf1),
						         "CTaskApi::SendMessageSync : Process %s discards the received response message with seq num of %d",
						         ProcessNames[header.m_process],
						         header.m_RspMsgSeqNum);
						PTRACE(eLevelInfoNormal, buf1);
					}
				}
				else if (res == STATUS_QUEUE_TIMEOUT)
				{
					PTRACE(eLevelInfoNormal, "CTaskApi::SendMessageSync : STATUS_QUEUE_TIMEOUT");
					break;
				}
				else
					break;

				CurTime = SystemGetTickCount();
				timeOut = AbsTimeOut - CurTime;

				char buf3[300];
				snprintf(buf3,
				         sizeof(buf3),
				         "Process %s Task %s waits for rsp message:  send seq num %d opcode %d Timeout: %d",
				         ProcessNames[header.m_process],
				         currentTask->GetTaskName(),
				         MsgSeqNum, opcode,
				         timeOut.GetIntegerPartForTrace());
				PTRACE(eLevelInfoNormal, buf3);
			}
			while (AbsTimeOut > CurTime);

			POBJDELETE(header.m_segment);
		}
	}
	else
		PASSERT(1);

	return res;
}

STATUS CTaskApi::SendSyncOpcodeMsg(OPCODE opcode, TICKS ticks, OPCODE& outRespOpcode) const
{
	CSegment seg;
	STATUS stat = SendMessageSync(NULL, opcode, ticks, outRespOpcode, seg);

	PASSERT(stat); // create assert if timeout exceeded
	return stat;
}

void CTaskApi::StaticSendLocalOpcodeMsg(OPCODE opcode)
{
	CProcessBase::GetProcess()->m_numLocalMsgSent++;
	StaticSendLocalMessage(NULL, opcode);
}

bool CTaskApi::TaskExists() const
{
	return CPObject::IsValidPObjectPtr(((CTaskApi*)this)->GetTaskAppPtr());
}

void CTaskApi::ObserverUpdate(WORD event, WORD type, DWORD observerInfo1, DWORD val, void* pPointer)
{
	CSegment* seg = new CSegment;

	*seg
		<< type
		<< observerInfo1
		<< event
		<< val;

	SendMsg(seg, OBSERVER_UPDATE);
}

void CTaskApi::Dump(WORD i) const
{
}

