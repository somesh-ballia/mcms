#include "ListenSocket.h"

#include "OsSocketListener.h"

#include "OsSocketConnected.h"
#include "PairOfSockets.h"
#include "SocketTask.h"

#include "SystemFunctions.h"
#include "ProcessBase.h"

#include "Segment.h"
#include "SocketApi.h"

#include "OpcodesMcmsCommon.h"
#include "StatusesGeneral.h"

#include "Macros.h"
#include "StringsLen.h"

#include "SysConfigKeys.h"
#include "SysConfig.h"
#include "ConfigHelper.h"

#include "Trace.h"
#include "TraceStream.h"
#include "PrettyTable.h"

#include <netinet/in.h>

///////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CListenSocket)
	ONEVENT(CLOSE_SOCKET_CONNECTION, ANYCASE, CListenSocket::RemoveFromVector)
	ONEVENT(SOCKET_DROPPED         , ANYCASE, CListenSocket::OnSocketDroppedAnycase)
	ONEVENT(SOCKET_CORRUPTED_MSG   , ANYCASE, CListenSocket::OnSocketCorruptedMsg)
PEND_MESSAGE_MAP(CListenSocket, CStateMachine);

///////////////////////////////////////////////////////////////////////////////
void listenSocketEntryPoint(void* appParam)
{
	CListenSocket *pListenSocketTask = new CListenSocket;
	pListenSocketTask->Create(*reinterpret_cast<CSegment*>(appParam));

	// processBase stores the pointer to ListenSocket
	CProcessBase* pProcess = CProcessBase::GetProcess();
	pProcess->SetListenSocketTask(pListenSocketTask);
}

/////////////////////////////////////////////////////////////////////////////
CListenSocket::CListenSocket()
	: socketListener_(NULL)
    , connectionID_(0)
	, bufferSize_(128*1024-1)
	, bufferThreshold_(-1)
	, maxConnections_(DEFAULT_MAX_NUM_CONNECTIONS)
    , connectionMode_(eTxRxConnection)
{
	memset(&address_, 0, sizeof(address_));
	TRACEINTO << "Memory usage:" << GetUsedMemory(false);

}

/////////////////////////////////////////////////////////////////////////////
CListenSocket::~CListenSocket()
{
	PDELETE(socketListener_);
	CProcessBase* pProcess = CProcessBase::GetProcess();
	if (pProcess)
		pProcess->SetListenSocketTask(NULL);

	TRACEINTO << "Memory usage:" << GetUsedMemory(false);
}

/////////////////////////////////////////////////////////////////////////////
void CListenSocket::SetRestrictions(size_t size, int threshold)
{
	bufferSize_ = size;
	bufferThreshold_ = threshold;
}

/////////////////////////////////////////////////////////////////////////////
void CListenSocket::InitTask()
{
	m_state = LSTN_SOCKET_IDLE;
	socketListener_ = new COsSocketListener(address_.port);
	const char* interface = m_interface.empty() ? NULL : m_interface.c_str();

	STATUS status = socketListener_->ConfigureListenSocket(address_.addr.v4.ip, interface);

	if (status == STATUS_OK)
		m_state = LSTN_SOCKET_ACTIVE;
	else
	{
		char buff[128];
		snprintf(buff, sizeof(buff),"ConfigureListenSocket failed on port:  %d , aborting",address_.port);
		PASSERTMSG(status, buff);
		m_selfKill = true;
	}
}

/////////////////////////////////////////////////////////////////////////////
void CListenSocket::Create(CSegment& appParam)
{
	m_selfKill = false;

	DWORD connectionMode = 0;

	m_pCreatorRcvMbx->DeSerialize(appParam);

	appParam
		>> (void*&)rxEntryPoint_
		>> (void*&)txEntryPoint_
		>> address_.port
		>> address_.addr.v4.ip
		>> m_interface
		>> connectionMode
		>> maxConnections_;

	connectionMode_ = (ServerConnectionModeEnum)connectionMode;

	CreateReceiveMailbox();
	CreateResponseMailbox();

	CTaskApp::CreateOsTask();

	m_pRcvMbx->Serialize(appParam);
	appParam << (void*)this;
}

/////////////////////////////////////////////////////////////////////////////
void CListenSocket::HandlePrivateFileDescriptor()
{
	PASSERT_AND_RETURN(!socketListener_);

	COsSocketConnected connected(bufferSize_, bufferThreshold_);

	const size_t numOfConnections = socketsVector_.size();

	if (maxConnections_ <= numOfConnections)
	{
		STATUS status = socketListener_->Reject(connected);
		TRACEINTO
			<< "\nCannot accept socket connection. \n"
			<< "Maximum Number of Connections : " << maxConnections_ << "\n"
			<< "Current Number of Connections : " << numOfConnections << "\n"
			<< "Status : " << CProcessBase::GetProcess()->GetStatusAsString(status);

		return;
	}

	STATUS res = socketListener_->Accept(connected);

	int newFileDescriptor = connected.GetDescriptor();

	DWORD remoteIp = connected.GetRemoteIp();

	char strRemoteIp[32];
	SystemDWORDToIpString(remoteIp, strRemoteIp);

	if (IsIpBlocked(remoteIp))
	{
		TRACEINTO << "Remote peer [" << strRemoteIp << "] tried to connect, but was blocked";

		socketListener_->Close(connected);
		return;
	}

	switch (res)
	{
	case STATUS_OK:
	{
		/***************************************************************************/
		/* VNGR-17411,VNGR-17588 4.11.10 added by Rachel Cohen                     */
		/* When creator is MplApi there should be only one connection per board    */
		/* the maxNumOfConnections is 8 and on startup we had a case that all 4    */
		/* media cards got reset and connected again we reach our 8 connections    */
		/* while the switch could not get connected. that cause MPL Failure bug    */
		/***************************************************************************/
		IsKnownIp(remoteIp, newFileDescriptor);

		TRACEINTO
			<< "Accept socket connection, Connected socket fd:" << connected.GetDescriptor()
			<< ", Remote IP:" << strRemoteIp << ", port:" << socketListener_->m_portNum;

		switch (connectionMode_)
		{
		case eTxRxConnection:
			CreateRxTxTasks(connected);
			break;

		case eRxConnection:
			CreateRxTasksOnly(connected);
			break;

		default:
			PASSERTMSG(connectionMode_, "Bad Choice");
			break;
		}

		SendMessageToCreator(NULL, OPEN_SOCKET_CONNECTION);
		break;
	}

	default:
		m_state = LSTN_SOCKET_IDLE;
		PASSERTMSG(res, "CListenSocket::ListenSocket : socket error");
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////
bool CListenSocket::IsIpBlocked(DWORD remoteIp)
{
	for (CIpVector::iterator it = blockedIpVector_.begin(); it != blockedIpVector_.end(); ++it)
	{
		const DWORD currentIp = *it;

		if (currentIp == remoteIp)
			return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////
void CListenSocket::IsKnownIp(DWORD remoteIp, int newFD)
{
	if (MPL_API_LISTEN_SOCKET_PORT_NUM == socketListener_->m_portNum &&
		(IsTarget() || (eProductTypeNinja == CProcessBase::GetProcess()->GetProductType())) &&//modified by Richer for BRIDGE-12875, 2014.4.22
		eProductFamilyCallGenerator != CProcessBase::GetProcess()->GetProductFamily())
	{
		TRACEINTO << "it is MPL_API_LISTEN_SOCKET_PORT_NUM";

		char strRemoteIp[32];
		SystemDWORDToIpString(remoteIp, strRemoteIp);

		// If it is MplApi the remove from the vector will be after killing RX and TX tasks - kill both
		bool flag = GetSystemCfgFlagInt<bool>(CFG_KEY_SYNC_DESTROY_SOCKET);

		for (SOCKETSVECTOR::iterator it = socketsVector_.begin(); it != socketsVector_.end(); ++it)
		{
			DWORD connectedRemoteIp = it->m_connected.GetRemoteIp();

			char strRemoteIp2[32];
			SystemDWORDToIpString(connectedRemoteIp, strRemoteIp2);

			TRACEINTO << "Remote ip1: " << strRemoteIp << ", Remote ip2:" << strRemoteIp2;

			if (connectedRemoteIp == remoteIp)
			{
				TRACEINTO
					<< "Remove OLD Connection (connected socket fd:" << it->m_connected.GetDescriptor()
					<< ", Remote IP:" << strRemoteIp << ", port: " << socketListener_->m_portNum;

				it->KillBoth();

				CSegment* pSeg = new CSegment;
				*pSeg << (WORD) it->m_conId;

				// close socket
				if (flag)
				{
					int oldFD = it->m_connected.GetDescriptor();

					RemoveFromVectorAfterKillBoth(pSeg); // remove from vector is done only after destroying the rx and tx tasks

					if (newFD != oldFD)
						socketListener_->CloseSocket(oldFD);
				}
				else
				{
					socketListener_->Close(it->m_connected);
					RemoveFromVector(pSeg);
				}
				if (pSeg != NULL)
				{
					delete pSeg;
				}
				SystemSleep(100);
				return;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CListenSocket::CreateRxTxTasks(const COsSocketConnected& connected)
{
	COsQueue dummy;

	CSocketApi socketTxApi;
	socketTxApi.Create(
		txEntryPoint_,
		*m_pRcvMbx,
		connected,
		dummy,
		FALSE,
		connectionID_);

	CSocketApi socketRxApi;
	socketRxApi.Create(
		rxEntryPoint_,
		*m_pRcvMbx,
		connected,
		socketTxApi.GetRcvMbx(),
		FALSE,
		connectionID_);

	CSocketTask* rxTask = (CSocketTask*)socketRxApi.GetTaskAppPtr();
	CSocketTask* txTask = (CSocketTask*)socketTxApi.GetTaskAppPtr();

	//when creating the tx we still do not have the Mbx of rx so after we create the rx we set its Mbx to TX
	txTask->SetTwinMbx(socketRxApi.GetRcvMbx());

	if (CProcessBase::GetProcess()->GetProcessType() == eProcessMCCFMngr)
	{
		rxTask->SetTwinMbx(socketTxApi.GetRcvMbx());

		rxTask->SetTwinSocketTask(txTask);
		txTask->SetTwinSocketTask(rxTask);
	}

	bool flag = GetSystemCfgFlagInt<bool>(CFG_KEY_SYNC_DESTROY_SOCKET);

	const DWORD rx_pid = rxTask->GetTaskId();
	const DWORD tx_pid = txTask->GetTaskId();

	TRACEINTO << "SYNC_DESTROY_SOCKET flag:" << flag << ", rx_pid:" << rx_pid << ", tx_pid:" << tx_pid;

	bool updatePIDs =
		flag &&
		CProcessBase::GetProcess()->GetProcessType() == eProcessMplApi &&
		socketListener_->m_portNum == MPL_API_LISTEN_SOCKET_PORT_NUM;

	AddtoVectorEx(socketRxApi.GetRcvMbx(), socketTxApi.GetRcvMbx(), connected, rx_pid, tx_pid, updatePIDs);

	SystemSleep(200);
}

/////////////////////////////////////////////////////////////////////////////
void  CListenSocket::CreateRxTasksOnly(const COsSocketConnected& connected)
{
	COsQueue dummy;

	CSocketApi socketRxApi;
	socketRxApi.Create(
		rxEntryPoint_,
		*m_pRcvMbx,
		connected,
		dummy,
		FALSE,
		connectionID_);

	CSocketTask* rxTask = (CSocketTask*)socketRxApi.GetTaskAppPtr();
	AddtoVectorEx(socketRxApi.GetRcvMbx(), dummy, connected);

	SystemSleep(200);
}

/////////////////////////////////////////////////////////////////////////////
void CListenSocket::SelfKill()
{
	if (m_state == LSTN_SOCKET_ACTIVE)
	{
		for (SOCKETSVECTOR::iterator it = socketsVector_.begin(); it != socketsVector_.end(); ++it)
		{
			it->KillBoth();
			//socketsVector_.erase(theIterator);
		}

		socketListener_->Close();
		m_state = LSTN_SOCKET_IDLE;
	}

	CTaskApp::SelfKill();
}

/////////////////////////////////////////////////////////////////////////////
void CListenSocket::OnSocketDroppedAnycase(CSegment* pParam)
{
	WORD conId = 0xFF;
	*pParam >> conId;

	// ************VNGR-20510 do not remove from vector for MplApi*********************************
	// If it is MplApi the remove from the vector will be after killing RX and TX tasks - kill both
	CProcessBase* proc = CProcessBase::GetProcess();

	bool flag = GetSystemCfgFlagInt<bool>(CFG_KEY_SYNC_DESTROY_SOCKET);

	if (flag && proc->GetProcessType() == eProcessMplApi)
	{
		PTRACE(eLevelInfoNormal, "OnSocketDroppedAnycase returns for MplApi \n");
		return;
	}

	// *************VNGR-20510 do not remove from vector for MplApi*********************************
	if (conId == 0xFF)
		return;

	CSegment* pSeg = new CSegment;
	*pSeg << (WORD) conId;
	SendMessageToCreator(pSeg, CLOSE_SOCKET_CONNECTION);

	for (SOCKETSVECTOR::iterator it = socketsVector_.begin(); it != socketsVector_.end(); ++it)
	{
		if (it->m_conId == conId)
		{
			TRACEINTO << "conId:" << conId << ", rx_pid:" << it->rx_pid;

			socketsVector_.erase(it);
			break;
		}
	}

	ShowVector();
}

/////////////////////////////////////////////////////////////////////////////
void CListenSocket::OnSocketCorruptedMsg(CSegment* pParam)
{
	DWORD remoteIp = 0;
	*pParam >> remoteIp;

	blockedIpVector_.push_back(remoteIp);
}

//////////////////////////////////////////////////////////////
SOCKETSVECTOR& CListenSocket::GetSockectsVector()
{
	return socketsVector_;
}

//////////////////////////////////////////////////////////////
void CListenSocket::ShowVector()
{
	TRACECOND_AND_RETURN(socketsVector_.empty(), "No connections.");

	typedef char IP_string[32];
	CPrettyTable<WORD, WORD, const char*> t("Conn ID", "Socket FD", "Remote IP");
	t.SetCaption("Open Sockets");

	for (SOCKETSVECTOR::iterator it = socketsVector_.begin(); it != socketsVector_.end(); ++it)
	{
		IP_string strRemoteIp;
		SystemDWORDToIpString(it->m_connected.GetRemoteIp(), strRemoteIp);

		t.Add(it->m_conId, it->m_connected.GetDescriptor(), strRemoteIp);
	}

	TRACEINTO << '\n' << t.Get();
}

/***************************************************************************/
/* The function adds pair of sockets to the vector - setting the sockets   */
/* Rx and Tx pids in order to know afterwards for which tasks the father   */
/* should wait after destroying the tasks (for example in a reconnection   */
/***************************************************************************/
void CListenSocket::AddtoVectorEx(
	const COsQueue& rcv_task_mailslot,
	const COsQueue& trx_task_mailslot,
	const COsSocketConnected& connected,
	DWORD rx_pid, DWORD tx_pid, bool updatePIDs)
{
	COsSocketConnected con(connected);

	CPairOfSockets sockets(rcv_task_mailslot, trx_task_mailslot, connectionID_, connected);

	// set Task's id of the socket
	if (updatePIDs)
	{
		sockets.rx_pid = rx_pid;
		sockets.tx_pid = tx_pid;
	}

	socketsVector_.push_back(sockets);

	char strRemoteIp[32];
	SystemDWORDToIpString(connected.GetRemoteIp(), strRemoteIp);

	TRACEINTO
		<< " conId:" << connectionID_ << ", GetDescriptor:" << con.GetDescriptor()
		<< ", GetRemoteIp:" << strRemoteIp
		<< ", rx_pid:" << rx_pid << ", tx_pid:" << tx_pid;

	++connectionID_;

	ShowVector();
}

///////////////////////////////////////////////////////////////
void CListenSocket::RemoveFromVector(CSegment* pMsg)
{
	WORD conId = 0xFF;
	*pMsg >> conId;

	// ************VNGR-20510 do not remove from vector for MplApi*********************************
	// If it is MplApi the remove from the vector will be after killing RX and TX tasks - kill both

	CProcessBase* proc = CProcessBase::GetProcess();

	bool flag = GetSystemCfgFlagInt<bool>(CFG_KEY_SYNC_DESTROY_SOCKET);

	if (flag && proc->GetProcessType() == eProcessMplApi)
	{
		PTRACE(eLevelInfoNormal, "RemoveFromVector returns for MplApi \n");
		return;
	}

	// **************VNGR-20510 do not remove from vector for MplApi*********************************
	if (conId != 0xFF)
	{
		CSegment* pSeg = new CSegment;
		*pSeg << (WORD) conId;

		CTaskApi api;
		api.CreateOnlyApi(*m_pCreatorRcvMbx);
		STATUS c = api.SendMsg(pSeg, CLOSE_SOCKET_CONNECTION);

		TRACEINTO << "connection ID:" << conId;

		for (SOCKETSVECTOR::iterator it = socketsVector_.begin(); it != socketsVector_.end(); ++it)
		{
			if (it->m_conId == conId)
			{
				socketsVector_.erase(it);
				break;
			}
		}

		ShowVector();
	}
	else
		TRACEINTO << "Illegal connection id:" << conId;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// VNGR-20510 after destroying tx and rx tasks - closing connection and remove from vector
void CListenSocket::RemoveFromVectorAfterKillBoth(CSegment* pMsg)
{
	// That function is running only for MplApi process

	WORD conId = 0xFF;
	*pMsg >> conId;

	if (conId != 0xFF)
	{

		//CSegment* pSeg = new CSegment;
		//*pSeg << (WORD) conId;

		//CTaskApi api;
		//api.CreateOnlyApi(*m_pCreatorRcvMbx);
		//STATUS c = api.SendMsg(pSeg, CLOSE_SOCKET_CONNECTION);

		CProcessBase* pProcess = CProcessBase::GetProcess();
		pProcess->CloseConnection(conId);

		TRACEINTO << "connection id:" << conId;

		for (SOCKETSVECTOR::iterator it = socketsVector_.begin(); it != socketsVector_.end(); ++it)
		{
			if (it->m_conId==conId)
			{
				socketsVector_.erase(it);
				break;
			}
		}


		ShowVector();
	}
	else
		TRACEINTO << "Illegal connection id:" << conId;
}

/////////////////////////////////////////////////////////////////////////////
COsQueue* CListenSocket::GetTxMbx(WORD conId)
{
	for (SOCKETSVECTOR::iterator it = socketsVector_.begin(); it != socketsVector_.end(); ++it)
	{
		if (it->m_conId == conId)
			return it->m_TransmitSocketMailSlot;
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CListenSocket::SendMessageToCreator(CSegment* pSeg, OPCODE opcode)
{
	CTaskApi api;
	api.CreateOnlyApi(*m_pCreatorRcvMbx);
	return api.SendMsg(pSeg, opcode);
}

/////////////////////////////////////////////////////////////////////////////
