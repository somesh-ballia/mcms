//+========================================================================+
//                    EpGuiTxSocket.cpp                                    |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       EpGuiTxSocket.cpp                                           |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+


#include "Macros.h"
#include "MplMcmsProtocol.h"
#include "SocketApi.h"
#include "EndpointsGuiApi.h"
#include "EpGuiTxSocket.h"


/////////////////////////////////////////////////////////////////////////////
//
//   CSimCardTxSocket
//
/////////////////////////////////////////////////////////////////////////////

// message map
PBEGIN_MESSAGE_MAP(CEpGuiTxSocket)
   ONEVENT( SOCKET_WRITE, ANYCASE,  CEpGuiTxSocket::OnWriteSocketAnycase)
PEND_MESSAGE_MAP(CEpGuiTxSocket,CSocketTxTask);


/////////////////////////////////////////////////////////////////////////////
//  task creation function
void EpGuiTxEntryPoint(void* appParam)
{
	CEpGuiTxSocket*  pTxSocket = new CEpGuiTxSocket;
	pTxSocket->Create(*(CSegment*)appParam);
}

/////////////////////////////////////////////////////////////////////////////
CEpGuiTxSocket::CEpGuiTxSocket()      // constructor
{
}

/////////////////////////////////////////////////////////////////////////////
CEpGuiTxSocket::~CEpGuiTxSocket()     // destructor
{
}

/////////////////////////////////////////////////////////////////////////////
void*  CEpGuiTxSocket::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
const char* CEpGuiTxSocket::GetTaskName() const
{
	return "EpGuiTxSocketTask";
}

/////////////////////////////////////////////////////////////////////////////
void CEpGuiTxSocket::InitTask()
{
}

//////////////////////////////////////////////////////////////////////
//void CEpGuiTxSocket::HandleDisconnect()
//{
//	CSocketTask::HandleDisconnect();
//
//	CTaskApi*  pTaskApi = new CTaskApi;
//	pTaskApi->CreateOnlyApi(*m_pCreatorRcvMbx);
//	pTaskApi->SendMsg(NULL,SOCKET_DROPPED);
//	pTaskApi->DestroyOnlyApi();
//	POBJDELETE(pTaskApi);
//}

//////////////////////////////////////////////////////////////////////
void CEpGuiTxSocket::OnWriteSocketAnycase(CSegment* pParam)
{
	// fill LEVEL1 header information
/*
 * 
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
 *
 * 
 */
//	DWORD  nMsgLen = pParam->GetWrtOffset();
	int nMsgLen = pParam->GetWrtOffset();
	do
	{
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










