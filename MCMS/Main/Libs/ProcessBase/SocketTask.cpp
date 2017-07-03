#include "SocketTask.h"

#include "OpcodesMcmsCommon.h"

#include "SocketApi.h"

#include "Trace.h"
#include "TraceStream.h"

//////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CSocketTask)
	ONEVENT(OPEN_SOCKET_TIMER      ,ANYCASE, CSocketTask::OnOpenSocketTimer )
PEND_MESSAGE_MAP(CSocketTask,CStateMachine); 

//////////////////////////////////////////////////////////////////////

CSocketTask::CSocketTask(COsSocketConnected* pSocketDesc/* = NULL*/)
	: m_connection(pSocketDesc ? pSocketDesc : new COsSocketConnected())
	, m_twinTask(new COsQueue)
	, m_socketConnectionId(-1)
    , isSecured_(false)
    , twinSocketTask_(NULL)
{
}

//////////////////////////////////////////////////////////////////////
CSocketTask::~CSocketTask()
{
	PDELETE(m_connection);
	PDELETE(m_twinTask);
	twinSocketTask_ = NULL;
}

/////////////////////////////////////////////////////////////////////////////
void CSocketTask::Create(CSegment& appParam)
{
	DWORD connectionID = 0;

	appParam >> isSecured_ >> connectionID;

	m_socketConnectionId = connectionID;

	m_connection->DeSerialize(appParam);
	m_twinTask->DeSerialize(appParam);

	CTaskApp::Create(appParam);	
}

/////////////////////////////////////////////////////////////////////////////
BOOL CSocketTask::TaskHandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opcode)
{
	switch (opcode)
	{
		case KILLED_BY_TWIN:
			TRACEINTO_ERR_SOCKET << "Got KILLED_BY_TWIN";
			PDELETE(m_connection);
			m_selfKill = true;
			break;

		case CLOSE_SOCKET_BY_TWIN:
			if (m_connection)
			{
				//m_connection->Close(); the socket already been closed on the other twin side.
				PDELETE(m_connection);
				PDELETE(m_twinTask);
				//we do not change m_selfKill = true cause we kill task by signal
			}
			break;

		default:
			return false;
	}

	TRACESTRFUNC(eLevelError) << "handled opcode:" << opcode;
	return true;
}

/////////////////////////////////////////////////////////////////////////////
void CSocketTask::HandleDisconnect()
{
	TRACEINTO_ERR_SOCKET << "connection:" << m_connection << ", twinTask:" << m_twinTask << ", twinSocketTask:" << twinSocketTask_;

	if (m_twinTask->IsValid())
	{
		SetSelfKillForTwinTask();

		CSocketApi api;
		api.CreateOnlyApi(*m_twinTask);
		api.KillByTwin();

		const COsQueue& t = *m_twinTask;
		TRACEINTO_ERR_SOCKET << "Send KILLED_BY_TWIN via OsQueue { id:" << t.m_id << ", type:" << t.m_idType << ", process:" << t.m_process << " }";
	}

	if (!m_connection)
		PASSERT(1);
	else
	{
		m_connection->Close();
		PDELETE(m_connection);
	}

	// inform task-owner
	CSegment* pMsg = new CSegment;
	*pMsg << (WORD)m_socketConnectionId;

	SendMsgToCreator(SOCKET_DROPPED, pMsg);

	m_selfKill = true;
}

/////////////////////////////////////////////////////////////////////////////
void CSocketTask::CloseSocket()
{
	TRACEINTO_ERR_SOCKET << " ";

	if (m_twinTask && m_twinTask->IsValid())
	{
		CSocketApi api;
		api.CreateOnlyApi(*m_twinTask);
		api.CloseSocketByTwin();
	}

	if (!m_connection)
		PASSERT(1);
	else
	{
		m_connection->Close();
		PDELETE(m_connection);
		PDELETE(m_twinTask);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CSocketTask::SendMsgToCreator(OPCODE opcode, CSegment* pSeg) const
{
	CTaskApi taskApi;
	taskApi.CreateOnlyApi(*m_pCreatorRcvMbx);
	taskApi.SendMsg(pSeg, opcode);
}

/////////////////////////////////////////////////////////////////////////////
void CSocketTask::SendMessageToTwin(CSegment* pSeg, OPCODE opcode) const
{
	CTaskApi api;
	api.CreateOnlyApi(*m_twinTask);
	api.SendMsg(pSeg, opcode);
}

/////////////////////////////////////////////////////////////////////////////
void CSocketTask::SetSelfKillForTwinTask()
{
	if (twinSocketTask_)
	{
		TRACEINTO_SOCKET << "mark the twin SocketTask for self kill";
		twinSocketTask_->SetTwinSocketTask(NULL);
		twinSocketTask_->m_selfKill = true;
	}
}

/////////////////////////////////////////////////////////////////////////////
void CSocketTask::OnOpenSocketTimer(CSegment* pParam)
{
	if (!m_connection)
		return;

	char strRemoteIp[32];
	SystemDWORDToIpString(m_connection->GetRemoteIp(), strRemoteIp);

	//test 
	const char* cmd = "netstat -t";
	std::string answer;
	STATUS stat = SystemPipedCommand(cmd, answer);

	TRACEINTO_SOCKET
		<< NameOf() << " DID NOT GET ANY DATA FOR 15 SECONDS FROM SOCKET (fd:" << m_connection->GetDescriptor() << ")\n"
		<< "socket remote ip:" << strRemoteIp << "\n"
		<< cmd << "\n" << answer;

	/************************************************************************/
	/* 7.10.10 VNGR- 17411 fixed by Rachel Cohen .                          */
	/* We enter a timer on the MPLApiRxSocket . that cause on upgrade and   */
	/* downgrade a lo of RECONNECT asserts.                                 */
	/* before closing socket I am checking if it is still alive.            */
	/************************************************************************/

/*	if( (m_connection->IsPeerSocketDisconnected() == 0))
			{
				PTRACE(eLevelError,"CSocketTask::OnOpenSocketTimer - socket is still connected.");
				StartTimer(OPEN_SOCKET_TIMER,20*SECOND);

			}
	else
	{*/

	TRACEINTO_ERR_SOCKET << "ENTER SOCKET DISCONNECT";

	HandleDisconnect();
	stat = SystemPipedCommand(cmd, answer);

	TRACEINTO_SOCKET << "check netstat after socket disconnection: " << cmd << "\n" << answer;
	//}
}

/////////////////////////////////////////////////////////////////////////////
void CSocketTask::SetSelfKill()
{
	if (CProcessBase::GetProcess()->GetTestSignalFlag())
	{
		TRACEINTO_SOCKET << "Task Name:" << GetTaskName() << ", Task ID:" << GetTaskId();

		STATUS stat = UndoRelevantSemaphore();

		if (stat == STATUS_OK)
			CTaskApp::SelfKill();
		else //take the long way
			m_selfKill = true;
	}
	else
		TRACEINTO_HIGH_SOCKET << "signal flag is off";
}

/////////////////////////////////////////////////////////////////////////////
