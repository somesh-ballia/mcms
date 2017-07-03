//+========================================================================+
//                   SimCardTxSocket.cpp                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       SimCardTxSocket.cpp                                         |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

#include <netinet/in.h>

#include "Macros.h"
#include "MplMcmsProtocol.h"
#include "SocketApi.h"
#include "SimCardTxSocket.h"


/////////////////////////////////////////////////////////////////////////////
//
//   CSimCardTxSocket
//
/////////////////////////////////////////////////////////////////////////////

// message map
PBEGIN_MESSAGE_MAP(CSimCardTxSocket)
   ONEVENT( SOCKET_WRITE, ANYCASE,  CSimCardTxSocket::OnWriteSocketAnycase)
PEND_MESSAGE_MAP(CSimCardTxSocket,CSocketTxTask);


/////////////////////////////////////////////////////////////////////////////
//  task creation function
void SimCardTxEntryPoint(void* appParam)
{
	CSimCardTxSocket*  pTxSocket = new CSimCardTxSocket;
	pTxSocket->Create(*(CSegment*)appParam);
}

/////////////////////////////////////////////////////////////////////////////
CSimCardTxSocket::CSimCardTxSocket()      // constructor
{
}

/////////////////////////////////////////////////////////////////////////////
CSimCardTxSocket::~CSimCardTxSocket()     // destructor
{
}

/////////////////////////////////////////////////////////////////////////////
void*  CSimCardTxSocket::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
const char* CSimCardTxSocket::GetTaskName() const
{
	return "SimCardTxSocketTask";
}

/////////////////////////////////////////////////////////////////////////////
void CSimCardTxSocket::InitTask()
{
}

//////////////////////////////////////////////////////////////////////
//void CSimCardTxSocket::HandleDisconnect()
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
void CSimCardTxSocket::OnWriteSocketAnycase(CSegment* pParam)
{
	// fill LEVEL1 header information
	TPKT_HEADER_S   tTpktStruct;
	tTpktStruct.version_num		= TPKT_VERSION_NUM;
	tTpktStruct.reserved		= 0;
	
	static const DWORD tpktHeaderLen = sizeof(TPKT_HEADER_S);
	WORD payloadLen = pParam->GetWrtOffset() + tpktHeaderLen;
	tTpktStruct.payload_len		= (WORD)htons(payloadLen);

	CSegment* pMsg = new CSegment; // pMsg will delete inside

	// put LEVEL1 header to message
	pMsg->Put((BYTE*)(&tTpktStruct), tpktHeaderLen);

	*pMsg << *pParam;

	Write((char*)(pMsg->GetPtr()),pMsg->GetWrtOffset());
	POBJDELETE(pMsg);
}










