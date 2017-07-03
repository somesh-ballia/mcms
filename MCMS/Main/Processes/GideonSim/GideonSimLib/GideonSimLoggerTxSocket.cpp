//+========================================================================+
//                GideonSimLoggerTxSocket.cpp                              |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       GideonSimLoggerTxSocket.cpp                                 |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

#include <netinet/in.h>

#include "Macros.h"
//#include "Trace.h"
#include "MplMcmsProtocol.h"
#include "SocketApi.h"
#include "GideonSimLoggerTxSocket.h"


/////////////////////////////////////////////////////////////////////////////
//
//   CSimCardTxSocket
//
/////////////////////////////////////////////////////////////////////////////

// message map
PBEGIN_MESSAGE_MAP(CGideonSimLoggerTxSocket)
   ONEVENT( SOCKET_WRITE, ANYCASE,  CGideonSimLoggerTxSocket::OnWriteSocketAnycase)
PEND_MESSAGE_MAP(CGideonSimLoggerTxSocket,CSocketTxTask);


/////////////////////////////////////////////////////////////////////////////
//  task creation function
void GideonSimLoggerTxEntryPoint(void* appParam)
{
	CGideonSimLoggerTxSocket*  pTxSocket = new CGideonSimLoggerTxSocket;
	pTxSocket->Create(*(CSegment*)appParam);
}

/////////////////////////////////////////////////////////////////////////////
CGideonSimLoggerTxSocket::CGideonSimLoggerTxSocket()      // constructor
        :CSocketTxTask(TRUE) // drop blocked messages
{
}

/////////////////////////////////////////////////////////////////////////////
CGideonSimLoggerTxSocket::~CGideonSimLoggerTxSocket()     // destructor
{
}

/////////////////////////////////////////////////////////////////////////////
void*  CGideonSimLoggerTxSocket::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
const char* CGideonSimLoggerTxSocket::GetTaskName() const
{
	return "GideonSimLoggerTxSocket";
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimLoggerTxSocket::InitTask()
{
}

//////////////////////////////////////////////////////////////////////
//void CGideonSimLoggerTxSocket::HandleDisconnect()
//{
//	CSocketTask::HandleDisconnect();
//
//	CTaskApi*  pTaskApi = new CTaskApi;
//	pTaskApi->CreateOnlyApi(*m_pCreatorRcvMbx);
//	pTaskApi->SendMsg(NULL,SOCKET_DROPPED);
// 	pTaskApi->DestroyOnlyApi();
//	POBJDELETE(pTaskApi);
//}

//////////////////////////////////////////////////////////////////////
void CGideonSimLoggerTxSocket::OnWriteSocketAnycase(CSegment* pParam)
{
	// fill LEVEL1 header information
	TPKT_HEADER_S   tTpktHead;
	tTpktHead.version_num	= TPKT_VERSION_NUM;
	tTpktHead.reserved		= 0;
	
	WORD payloadLen = sizeof(TPKT_HEADER_S)	+ pParam->GetWrtOffset();
	tTpktHead.payload_len = (WORD)htons(payloadLen);

	CSegment* pMsg = new CSegment; // pMsg will delete inside

	// put LEVEL1 header to message
	pMsg->Put((BYTE*)(&tTpktHead), sizeof(TPKT_HEADER_S));

	*pMsg << *pParam;

	Write((char*)(pMsg->GetPtr()),pMsg->GetWrtOffset());
	POBJDELETE(pMsg);
}










