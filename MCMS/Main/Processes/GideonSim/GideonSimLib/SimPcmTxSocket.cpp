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
#include "SimPcmTxSocket.h"


/////////////////////////////////////////////////////////////////////////////
//
//   CSimCardTxSocket
//
/////////////////////////////////////////////////////////////////////////////

// message map
PBEGIN_MESSAGE_MAP(CSimPcmTxSocket)
   ONEVENT( SOCKET_WRITE, ANYCASE,  CSimPcmTxSocket::OnWriteSocketAnycase)
PEND_MESSAGE_MAP(CSimPcmTxSocket,CSocketTxTask);


/////////////////////////////////////////////////////////////////////////////
//  task creation function
void SimPcmTxEntryPoint(void* appParam)
{
	CSimPcmTxSocket*  pTxSocket = new CSimPcmTxSocket;
	pTxSocket->Create(*(CSegment*)appParam);
}

/////////////////////////////////////////////////////////////////////////////
CSimPcmTxSocket::CSimPcmTxSocket()      // constructor
{
}

/////////////////////////////////////////////////////////////////////////////
CSimPcmTxSocket::~CSimPcmTxSocket()     // destructor
{
}

/////////////////////////////////////////////////////////////////////////////
void*  CSimPcmTxSocket::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
const char* CSimPcmTxSocket::GetTaskName() const
{
	return "SimGuiTxSocketTask";
}

/////////////////////////////////////////////////////////////////////////////
void CSimPcmTxSocket::InitTask()
{
}

//////////////////////////////////////////////////////////////////////
//void CSimPcmTxSocket::HandleDisconnect()
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
void CSimPcmTxSocket::OnWriteSocketAnycase(CSegment* pParam)
{
	// fill LEVEL1 header information
	TPKT_HEADER_S   tTpktStruct;
	tTpktStruct.version_num      = TPKT_VERSION_NUM;
	tTpktStruct.reserved         = 0;	
	//tTpktStruct.payload_len      = (WORD)(pParam->GetWrtOffset());
	WORD nSize = (WORD)(pParam->GetWrtOffset()) + 4;
	tTpktStruct.payload_len      = (((nSize)&0xff00)>>8)+(((nSize)&0xff)<<8);

	CSegment* pMsg = new CSegment; // pMsg will delete inside

	// put LEVEL1 header to message
	pMsg->Put((BYTE*)(&tTpktStruct),sizeof(TPKT_HEADER_S));

	*pMsg << *pParam;

	Write((char*)(pMsg->GetPtr()),pMsg->GetWrtOffset());
	POBJDELETE(pMsg);
}










