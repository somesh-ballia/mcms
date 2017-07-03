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
#include "EpGuiPcmTxSocket.h"


/////////////////////////////////////////////////////////////////////////////
//
//   CSimCardTxSocket
//
/////////////////////////////////////////////////////////////////////////////

// message map
PBEGIN_MESSAGE_MAP(CEpGuiPcmTxSocket)
   ONEVENT( SOCKET_WRITE, ANYCASE,  CEpGuiPcmTxSocket::OnWriteSocketAnycase)
PEND_MESSAGE_MAP(CEpGuiPcmTxSocket,CSocketTxTask);


/////////////////////////////////////////////////////////////////////////////
//  task creation function
void EpGuiPcmTxEntryPoint(void* appParam)
{
	CEpGuiPcmTxSocket*  pTxSocket = new CEpGuiPcmTxSocket;
	pTxSocket->Create(*(CSegment*)appParam);
}

/////////////////////////////////////////////////////////////////////////////
CEpGuiPcmTxSocket::CEpGuiPcmTxSocket()      // constructor
{
}

/////////////////////////////////////////////////////////////////////////////
CEpGuiPcmTxSocket::~CEpGuiPcmTxSocket()     // destructor
{
}

/////////////////////////////////////////////////////////////////////////////
void*  CEpGuiPcmTxSocket::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
const char* CEpGuiPcmTxSocket::GetTaskName() const
{
	return "SimGuiTxSocketTask";
}

/////////////////////////////////////////////////////////////////////////////
void CEpGuiPcmTxSocket::InitTask()
{
}

//////////////////////////////////////////////////////////////////////
//void CEpGuiPcmTxSocket::HandleDisconnect()
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
void CEpGuiPcmTxSocket::OnWriteSocketAnycase(CSegment* pParam)
{
/*
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
*/
	int nMsgLen = pParam->GetWrtOffset();
	do
	{
		DWORD MAX_SIM_GUI_MSG_LEN = 65535;
		TPKT_HEADER_S   tTpktStruct;
		tTpktStruct.version_num   = TPKT_VERSION_NUM;
		tTpktStruct.reserved      = 0; // not continued message

		DWORD  nBytesToSend = (DWORD)nMsgLen;
		if( nMsgLen > (int)MAX_SIM_GUI_MSG_LEN )
		{
			nBytesToSend = MAX_SIM_GUI_MSG_LEN;
			tTpktStruct.reserved  = 1; // continued message
		}
		tTpktStruct.payload_len   = nBytesToSend;

		BYTE*  pBytes = new BYTE[nBytesToSend];
		pParam->Get(pBytes,nBytesToSend);

		CSegment* pMsg = new CSegment; // pMsg will delete inside

		// put LEVEL1 header to message
		pMsg->Put((BYTE*)(&tTpktStruct),sizeof(TPKT_HEADER_S));

		// put data bytes from pParam segment
		pMsg->Put(pBytes,nBytesToSend);
		PDELETEA(pBytes);

		Write((char*)(pMsg->GetPtr()),pMsg->GetWrtOffset());
		POBJDELETE(pMsg);

		nMsgLen -=  nBytesToSend;

	} while( nMsgLen > 0 );

}










