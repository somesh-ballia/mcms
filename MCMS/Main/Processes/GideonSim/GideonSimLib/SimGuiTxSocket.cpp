//+========================================================================+
//                    SimGuiTxSocket.cpp                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       SimGuiTxSocket.cpp                                          |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+


#include "Macros.h"
#include "MplMcmsProtocol.h"
#include "SocketApi.h"
#include "SimGuiTxSocket.h"


/////////////////////////////////////////////////////////////////////////////
//
//   CSimCardTxSocket
//
/////////////////////////////////////////////////////////////////////////////

// message map
PBEGIN_MESSAGE_MAP(CSimGuiTxSocket)
   ONEVENT( SOCKET_WRITE, ANYCASE,  CSimGuiTxSocket::OnWriteSocketAnycase)
PEND_MESSAGE_MAP(CSimGuiTxSocket,CSocketTxTask);


/////////////////////////////////////////////////////////////////////////////
//  task creation function
void SimGuiTxEntryPoint(void* appParam)
{
	CSimGuiTxSocket*  pTxSocket = new CSimGuiTxSocket;
	pTxSocket->Create(*(CSegment*)appParam);
}

/////////////////////////////////////////////////////////////////////////////
CSimGuiTxSocket::CSimGuiTxSocket()      // constructor
{
}

/////////////////////////////////////////////////////////////////////////////
CSimGuiTxSocket::~CSimGuiTxSocket()     // destructor
{
}

/////////////////////////////////////////////////////////////////////////////
void*  CSimGuiTxSocket::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
const char* CSimGuiTxSocket::GetTaskName() const
{
	return "SimGuiTxSocketTask";
}

/////////////////////////////////////////////////////////////////////////////
void CSimGuiTxSocket::InitTask()
{
}

//////////////////////////////////////////////////////////////////////
//void CSimGuiTxSocket::HandleDisconnect()
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
void CSimGuiTxSocket::OnWriteSocketAnycase(CSegment* pParam)
{
	// fill LEVEL1 header information
	TPKT_HEADER_S   tTpktStruct;
	tTpktStruct.version_num      = TPKT_VERSION_NUM;
	tTpktStruct.reserved         = 0;
	tTpktStruct.payload_len      = (WORD)(pParam->GetWrtOffset());

	CSegment* pMsg = new CSegment; // pMsg will delete inside

	// put LEVEL1 header to message
	pMsg->Put((BYTE*)(&tTpktStruct),sizeof(TPKT_HEADER_S));

	*pMsg << *pParam;

	Write((char*)(pMsg->GetPtr()),pMsg->GetWrtOffset());
	POBJDELETE(pMsg);
}










