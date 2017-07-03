//+========================================================================+
//                     ClientSocket.h                                      |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       ClientSocket.h                                              |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

#ifndef   __CLIENTSOCKET_H__
#define   __CLIENTSOCKET_H__


#include "StateMachine.h"
#include "SocketApi.h"

class CSegment;
class COsSocketClient;

class CClientSocket : public CStateMachine
{
CLASS_TYPE_1(CClientSocket,CStateMachine )
public:
				// Constructors
	CClientSocket(CTaskApp* pTask,
                  SOCKET_ENTRY_POINT rxEntryPoint,
	              SOCKET_ENTRY_POINT txEntryPoint,
                  COsSocketClient * pSocketDesc = NULL );
	virtual ~CClientSocket();
	virtual const char* NameOf() const { return "CClientSocket";}
				// base class overriding
	virtual void* GetMessageMap();
	virtual void  HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);

				// Operations
	void Connect();
	void Disconnect();
	void Send(CSegment* pMsg);
	void Destroy();
	//The socket is dropped by RX task. The creator will info this class.
	void Dropconnect();

	void Init(const char* pszIp,const WORD port);
	void Init(const mcTransportAddress ipAddr);
        void SetAddress(const mcTransportAddress ipAddr);
        void SetAddressPort(const char* pszIp,const WORD port);

	BOOL IsConnected() const;
	BOOL IsNotConnected() const;

	int  GetConnectionRetriesNumber() const { return m_connectionRetriesNumber; }
	void SetConnectionRetriesNumber(const int num) { if(num) m_connectionRetriesNumber = num; }
	int  GetConnectionRetried() const { return m_connectionRetried; }
	DWORD GetRetryTime() const { return m_connectionRetryTime; }
	void  SetRetryTime(const DWORD time) { if(time) m_connectionRetryTime = time; }
    void Reconnect();
protected:
				// Action functions
		// connect
	void OnTaskConnectIdle(CSegment* pMsg);
	void OnTaskConnectConnecting(CSegment* pMsg);
	void OnTaskConnectConnected(CSegment* pMsg);
		// disconnect
	void OnTaskDisConnectIdle(CSegment* pMsg);
	void OnTaskDisConnectConnecting(CSegment* pMsg);
	void OnTaskDisConnectConnected(CSegment* pMsg);
		// drop connect
	void OnTaskDropConnectIdle(CSegment* pMsg);
	void OnTaskDropConnectConnecting(CSegment* pMsg);
	void OnTaskDropConnectConnected(CSegment* pMsg);
	
	//  Send message with socket
	void OnTaskSendIdle(CSegment* pParam);
	void OnTaskSendConnecting(CSegment* pParam);
	void OnTaskSendConnected(CSegment* pParam);
	//  Retry connection Timer
	void OnTimerRetryConnectionConnecting(CSegment* pMsg);
	  // reconnect
	//void Reconnect();
    void OnTaskReconnectIdle(CSegment* pMsg);


protected:
				// Utilities
	void TryToConnect();
	void ConnectionEstablished();
	void ConnectionFailed();
	virtual void OnConnectionFailed();
	virtual void OnConnectionEstablished();

protected:
				// Attributes
	const SOCKET_ENTRY_POINT	m_socketRxEntryPoint;
	const SOCKET_ENTRY_POINT	m_socketTxEntryPoint;

	COsSocketClient * m_socketDesc;

	int			m_connectionRetriesNumber;
	int			m_connectionRetried;
	DWORD		m_connectionRetryTime;

	CSocketApi*	m_pRxTaskApi;
	CSocketApi*	m_pTxTaskApi;

	CTaskApi*	m_pTaskApi;

	PDECLAR_MESSAGE_MAP
};



#endif /* __CLIENTSOCKET_H__ */
