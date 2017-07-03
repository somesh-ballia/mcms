//+========================================================================+
//                    ClientSocket.cpp                                     |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       ClientSocket.cpp                                            |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

#include "TaskApp.h"
#include "TaskApi.h"
#include "Trace.h"
#include "SystemFunctions.h"
#include "ClientSocket.h"
#include "OsSocketClient.h"
#include "SocketTask.h"
#include "OsQueue.h"
#include "Macros.h"
#include "StatusesGeneral.h"

/////////////////////////////////////////////////////////////////////////////
//
//   STATES:
//
//const WORD  IDLE        = 0;        // default state -  defined in base class
const WORD  CONNECTING    = 1;
const WORD  CONNECTED     = 2;
//const WORD  DISCONNECTING = 3;
//const WORD  ANYCASE     = 0xFFFF;   // any other state -  defined in base class

/////////////////////////////////////////////////////////////////////////////
//
//   CONSTANTS:
//
const DWORD  RETRY_TIME_DEFAULT = 1 * SECOND;

/////////////////////////////////////////////////////////////////////////////
//
//   EVENTS:
//
const WORD CONNECT            = 2001;
const WORD DISCONNECT         = 2002;
const WORD SEND_MSG           = 2003;
const WORD RECONNECT          = 2004;
const WORD DROPCONNECT		  = 2005;
const WORD RETRY_CONNECT_TOUT = 2021;



/////////////////////////////////////////////////////////////////////////////
//
//   CClientSocket
//
/////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CClientSocket)

	//  Connect socket
	ONEVENT(CONNECT, IDLE,          CClientSocket::OnTaskConnectIdle)
	ONEVENT(RECONNECT, IDLE,        CClientSocket::OnTaskReconnectIdle)
	ONEVENT(CONNECT, CONNECTING,    CClientSocket::OnTaskConnectConnecting)
	ONEVENT(CONNECT, CONNECTED,     CClientSocket::OnTaskConnectConnected)
	//  Disconnect socket
	ONEVENT(DISCONNECT, IDLE,       CClientSocket::OnTaskDisConnectIdle)
	ONEVENT(DISCONNECT, CONNECTING, CClientSocket::OnTaskDisConnectConnecting)
	ONEVENT(DISCONNECT, CONNECTED,  CClientSocket::OnTaskDisConnectConnected)
	//  Dropconnect socket
	ONEVENT(DROPCONNECT, IDLE,       CClientSocket::OnTaskDropConnectIdle)
	ONEVENT(DROPCONNECT, CONNECTING, CClientSocket::OnTaskDropConnectConnecting)
	ONEVENT(DROPCONNECT, CONNECTED,  CClientSocket::OnTaskDropConnectConnected)
	//  Send message with socket
	ONEVENT(SEND_MSG, IDLE,         CClientSocket::OnTaskSendIdle)
	ONEVENT(SEND_MSG, CONNECTING,   CClientSocket::OnTaskSendConnecting)
	ONEVENT(SEND_MSG, CONNECTED,    CClientSocket::OnTaskSendConnected)
	//  Retry connection Timer
	ONEVENT(RETRY_CONNECT_TOUT, CONNECTING,  CClientSocket::OnTimerRetryConnectionConnecting)

PEND_MESSAGE_MAP(CClientSocket,CStateMachine);

/////////////////////////////////////////////////////////////////////////////
CClientSocket::CClientSocket (  CTaskApp* pTask,  // creator task
                                SOCKET_ENTRY_POINT rxEntryPoint,
                                SOCKET_ENTRY_POINT txEntryPoint,
                                COsSocketClient * pSocketDesc )

: CStateMachine(pTask),
  m_socketRxEntryPoint(rxEntryPoint),m_socketTxEntryPoint(txEntryPoint) // constructor

{
    if (pSocketDesc == NULL)
        m_socketDesc = new COsSocketClient;
    else
        m_socketDesc = pSocketDesc;

	m_connectionRetriesNumber	= 1000;
	m_connectionRetried		= 0;
	m_connectionRetryTime		= RETRY_TIME_DEFAULT;

	m_pRxTaskApi = NULL;
	m_pTxTaskApi = NULL;
	
	m_pTaskApi = new CTaskApi;

	m_pTaskApi->CreateOnlyApi(pTask->GetRcvMbx(),
							  this,
							  pTask->GetLocalQueue());

}

/////////////////////////////////////////////////////////////////////////////
CClientSocket::~CClientSocket()     // destructor
{
	if( CPObject::IsValidPObjectPtr(m_pTaskApi) )  {
		m_pTaskApi->DestroyOnlyApi();
		POBJDELETE(m_pTaskApi);
	}

	if (m_state != IDLE)
	{
		m_socketDesc->Close();
	}

	if ( CPObject::IsValidPObjectPtr(m_pRxTaskApi) ) {
		m_pRxTaskApi->SyncDestroy();
		POBJDELETE(m_pRxTaskApi);
	}
	if ( CPObject::IsValidPObjectPtr(m_pTxTaskApi) ) {
		m_pTxTaskApi->SyncDestroy();
		POBJDELETE(m_pTxTaskApi);
	}
	//BRIDGE-13310 - Seems that the RX and Tx are not closed Syncroniously
	SystemSleep(5);

	POBJDELETE(m_socketDesc);
}

/////////////////////////////////////////////////////////////////////////////
void* CClientSocket::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void CClientSocket::HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode)
{
//	DispatchEvent(opCode,(void*)m_msgEntries,pMsg);
	DispatchEvent(opCode,pMsg);
}



/////////////////////////////////////////////////////////////////////////////
void CClientSocket::Init(const mcTransportAddress ipAddr)
{
	SetAddress(ipAddr);

}

/////////////////////////////////////////////////////////////////////////////
void CClientSocket::Init(const char* pszIp,const WORD port)
{
	mcTransportAddress tempAddr;
	stringToIp(&tempAddr, (char*)pszIp, eNetwork);
	tempAddr.port = port;
	Init(tempAddr);
}

/////////////////////////////////////////////////////////////////////////////
BOOL CClientSocket::IsConnected() const
{
	return( m_state == CONNECTED ) ? TRUE : FALSE;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CClientSocket::IsNotConnected() const
{
	return( m_state == IDLE ) ? TRUE : FALSE;
}

/////////////////////////////////////////////////////////////////////////////
void CClientSocket::TryToConnect()
{
	BOOL bConnected = m_socketDesc->Select(50); //0.5 sec -> In failover - different networks it took 300 ms

	if( bConnected == FALSE )
	{
		m_connectionRetried++;
		if( m_connectionRetried < m_connectionRetriesNumber )
		{
			StartTimer(RETRY_CONNECT_TOUT,m_connectionRetryTime);
		} else
			ConnectionFailed();
	} else
		ConnectionEstablished();
}

/////////////////////////////////////////////////////////////////////////////
void CClientSocket::ConnectionEstablished()
{
	//  after socket connection was established, we shall create
	// two tasks for sending and receiving data - RX, TX
	// Create Socket RX
	PTRACE(eLevelError,"CClientSocket::ConnectionEstablished");
	
	COsSocketConnected* pConnected = NULL;//(128*1024-1,-1);

	m_socketDesc->CreateSocketConnected(&pConnected);

	COsQueue dummy;

	//  destroy TX and RX tasks
	if( CPObject::IsValidPObjectPtr(m_pRxTaskApi) )
	{
		PTRACE(eLevelError,"CClientSocket::ConnectionEstablished - deleting m_pRxTaskApi");
		m_pRxTaskApi->Destroy();
		POBJDELETE(m_pRxTaskApi);
	}

	if( CPObject::IsValidPObjectPtr(m_pTxTaskApi) )
	{
		PTRACE(eLevelError,"CClientSocket::ConnectionEstablished - deleting m_pTxTaskApi");
		m_pTxTaskApi->Destroy();
		POBJDELETE(m_pTxTaskApi);
	}

	m_pRxTaskApi = new CSocketApi;

    m_pRxTaskApi->Create(m_socketRxEntryPoint,
						 m_pTaskApi->GetRcvMbx(),
						 *pConnected,
						 dummy,
						 pConnected->IsSecured());

	m_pTxTaskApi = new CSocketApi;

    m_pTxTaskApi->Create(m_socketTxEntryPoint,
						 m_pTaskApi->GetRcvMbx(),
						 *pConnected,
						 m_pRxTaskApi->GetRcvMbx(),
						 pConnected->IsSecured());

	CSocketTask* rxTask = (CSocketTask*)m_pRxTaskApi->GetTaskAppPtr();
	rxTask->SetTwinMbx(m_pTxTaskApi->GetRcvMbx());

	m_connectionRetried = 0;
	m_state = CONNECTED;

	OnConnectionEstablished();
	
	delete pConnected;
}

/////////////////////////////////////////////////////////////////////////////
void CClientSocket::ConnectionFailed()
{
	//  if socket connection was not established after
	// all retries, we shall back to IDLE state
	PTRACE(eLevelError,"CClientSocket::ConnectionFailed - Connection failed. Go to IDLE mode.");
	//close(m_socketDesc);
	m_socketDesc->Close();
	m_state = IDLE;
	m_connectionRetried = 0;

	OnConnectionFailed();
}

/////////////////////////////////////////////////////////////////////////////
void CClientSocket::OnConnectionEstablished()
{
	// send notification to owner task
	if( CPObject::IsValidPObjectPtr(m_pTaskApi) )
		m_pTaskApi->SendLocalMessage(NULL,SOCKET_CONNECTED);
}

/////////////////////////////////////////////////////////////////////////////
void CClientSocket::OnConnectionFailed()
{
	// send notification to owner task
	if( CPObject::IsValidPObjectPtr(m_pTaskApi) )
		m_pTaskApi->SendLocalMessage(NULL,SOCKET_FAILED);
}

/////////////////////////////////////////////////////////////////////////////
void CClientSocket::Connect()
{
//	DispatchEvent(CONNECT,(void*)m_msgEntries,NULL);
    DispatchEvent(CONNECT);

}
/////////////////////////////////////////////////////////////////////////////
void CClientSocket::Reconnect()
{
//	DispatchEvent(CONNECT,(void*)m_msgEntries,NULL);
    DispatchEvent(RECONNECT);

}

/////////////////////////////////////////////////////////////////////////////
void CClientSocket::OnTaskReconnectIdle(CSegment* pMsg)
{
	int res = 0;

    PTRACE(eLevelInfoNormal,"CClientSocket::OnTaskRonnectIdle !!!!!");

	if (STATUS_OK != m_socketDesc->ConfigureClientSocket())
	{
		DBGPASSERT(res);
		return;
	}

	m_socketDesc->Connect();
	m_state = CONNECTING;

	TryToConnect();
}


/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
void CClientSocket::OnTaskConnectIdle(CSegment* pMsg)
{
	int res = 0;
	//PTRACE(eLevelInfoNormal,"CClientSocket::OnTaskConnectIdle");
    PTRACE(eLevelInfoNormal,"CClientSocket::OnTaskConnectIdle !!!!!");
	if (STATUS_OK != m_socketDesc->ConfigureClientSocket())
	{
		DBGPASSERT(res);
		return;
	}

	m_socketDesc->Connect();
	m_state = CONNECTING;

	TryToConnect();
}


/////////////////////////////////////////////////////////////////////////////
void CClientSocket::OnTaskConnectConnecting(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CClientSocket::OnTaskConnectConnecting - Already connecting");
	DBGPASSERT(1);
}

/////////////////////////////////////////////////////////////////////////////
void CClientSocket::OnTaskConnectConnected(CSegment* pMsgConnect)
{
	PTRACE(eLevelInfoNormal,"CClientSocket::OnTaskConnectConnected - Already connected");
	DBGPASSERT(1);
}


/////////////////////////////////////////////////////////////////////////////
void CClientSocket::Disconnect()
{
//	DispatchEvent(DISCONNECT,(void*)m_msgEntries,NULL);
	DispatchEvent(DISCONNECT);
}

/////////////////////////////////////////////////////////////////////////////
void CClientSocket::OnTaskDisConnectIdle(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CClientSocket::OnTaskDisConnectIdle - not connected");
	DBGPASSERT(1);
}

/////////////////////////////////////////////////////////////////////////////
void CClientSocket::OnTaskDisConnectConnecting(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CClientSocket::OnTaskDisConnectConnecting");
	// close connection and go to IDLE
	m_socketDesc->Close();
	m_state = IDLE;
	m_connectionRetried = 0;
	DeleteTimer(RETRY_CONNECT_TOUT);
}

/////////////////////////////////////////////////////////////////////////////
void CClientSocket::Destroy()
{
	// close socket
	m_socketDesc->Close();

	//  destroy TX and RX tasks
	if( CPObject::IsValidPObjectPtr(m_pRxTaskApi) )
		m_pRxTaskApi->SyncDestroy();
	POBJDELETE(m_pRxTaskApi);

	if( CPObject::IsValidPObjectPtr(m_pTxTaskApi) )
		m_pTxTaskApi->SyncDestroy();
	POBJDELETE(m_pTxTaskApi);
}

/////////////////////////////////////////////////////////////////////////////
void CClientSocket::OnTaskDisConnectConnected(CSegment* pMsg)
{
	//PTRACE(eLevelInfoNormal,"CClientSocket::OnTaskDisConnectConnected");

	Destroy();

	m_state = IDLE;
	m_connectionRetried = 0;
	DeleteTimer(RETRY_CONNECT_TOUT);
}

/////////////////////////////////////////////////////////////////////////////
// In SocketTask-->Read --> HandleDisconnect(), the socket RX/TX task is gone when reading error
// We need to have special handle here.
void CClientSocket::Dropconnect()
{
	DispatchEvent(DROPCONNECT);
}

/////////////////////////////////////////////////////////////////////////////
void CClientSocket::OnTaskDropConnectIdle(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CClientSocket::OnTaskDropConnectIdle - not connected");
	DBGPASSERT(1);
}

/////////////////////////////////////////////////////////////////////////////
void CClientSocket::OnTaskDropConnectConnecting(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CClientSocket::OnTaskDropConnectConnecting - not connected");
	DBGPASSERT(1);
}

/////////////////////////////////////////////////////////////////////////////
// When Dropconnect message is received, it means that the socket is already closed RX/TX task is also destroyed too.
// Only free RxTaskApi and TxTaskApi, drop the socket.
void CClientSocket::OnTaskDropConnectConnected(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CClientSocket::OnTaskDropConnectConnected");

	// Drop the socket
	m_socketDesc->Drop();

	//  destroy TX and RX tasks Api Only
	if( CPObject::IsValidPObjectPtr(m_pRxTaskApi) )
		m_pRxTaskApi->DestroyOnlyApi();
	POBJDELETE(m_pRxTaskApi);

	if( CPObject::IsValidPObjectPtr(m_pTxTaskApi) )
		m_pTxTaskApi->DestroyOnlyApi();
	POBJDELETE(m_pTxTaskApi);

	m_state = IDLE;
	m_connectionRetried = 0;
	DeleteTimer(RETRY_CONNECT_TOUT);

}


/////////////////////////////////////////////////////////////////////////////
void CClientSocket::Send(CSegment* pMsg)
{
//	DispatchEvent(SEND_MSG,(void*)m_msgEntries,pMsg);
	DispatchEvent(SEND_MSG,pMsg);
}

/////////////////////////////////////////////////////////////////////////////
void CClientSocket::OnTaskSendIdle(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CClientSocket::OnTaskSendIdle - IDLE state");
	DBGPASSERT(1);
}

/////////////////////////////////////////////////////////////////////////////
void CClientSocket::OnTaskSendConnecting(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CClientSocket::OnTaskSendConnecting - not connected yet");
	DBGPASSERT(1);
}

/////////////////////////////////////////////////////////////////////////////
void CClientSocket::OnTaskSendConnected(CSegment* pParam)
{
	//PTRACE(eLevelInfoNormal,"CClientSocket::OnTaskSendConnected");
	CSegment*  pMsg = new CSegment(*pParam);
	m_pTxTaskApi->WriteSocket(pMsg);
}

/////////////////////////////////////////////////////////////////////////////
void CClientSocket::SetAddress(const mcTransportAddress ipAddr)
{
	m_socketDesc->SetAddress(ipAddr);
}

/////////////////////////////////////////////////////////////////////////////
void CClientSocket::SetAddressPort(const char* pszIp,const WORD port)
{
	mcTransportAddress tempAddr;
	stringToIp(&tempAddr, (char*)pszIp, eNetwork);
	tempAddr.port = port;
 	SetAddress(tempAddr);
}

/////////////////////////////////////////////////////////////////////////////
void CClientSocket::OnTimerRetryConnectionConnecting(CSegment* pMsg)
{
	//PTRACE(eLevelInfoNormal,"CClientSocket::OnTimerRetryConnectionConnecting");

	m_socketDesc->Connect();

	TryToConnect();
}
















