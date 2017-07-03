//+========================================================================+
//                     EpGuiRxSocket.cpp                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       EpGuiRxSocket.cpp                                           |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

#include "Macros.h"
#include "Trace.h"
#include "ProcessBase.h"
#include "SocketApi.h"

#include "SimApi.h"
#include "EndpointsGuiApi.h"
#include "EndpointsSim.h"
#include "EpGuiRxSocket.h"
#include "StatusesGeneral.h"
#include "OpcodesMcmsInternal.h"


/////////////////////////////////////////////////////////////////////////////
//
//   CSimGuiRxSocket
//
/////////////////////////////////////////////////////////////////////////////


PBEGIN_MESSAGE_MAP(CEpGuiRxSocket)
   ONEVENT( SOCKET_WRITE, ANYCASE,  CStateMachine::NullActionFunction)
PEND_MESSAGE_MAP(CEpGuiRxSocket,CSocketRxTask);


/////////////////////////////////////////////////////////////////////////////
//  task creation function
void EpGuiRxEntryPoint(void* appParam)
{
	CEpGuiRxSocket*  pRxSocket = new CEpGuiRxSocket;
	pRxSocket->Create(*(CSegment*)appParam);
}

/////////////////////////////////////////////////////////////////////////////
CEpGuiRxSocket::CEpGuiRxSocket()      // constructor
{
}

/////////////////////////////////////////////////////////////////////////////
CEpGuiRxSocket::~CEpGuiRxSocket()     // destructor
{
}

/////////////////////////////////////////////////////////////////////////////
void CEpGuiRxSocket::InitTask()
{
}

/////////////////////////////////////////////////////////////////////////////
void*  CEpGuiRxSocket::GetMessageMap()
{
	return (void*)m_msgEntries;
}


/////////////////////////////////////////////////////////////////////////////
const char* CEpGuiRxSocket::GetTaskName() const
{
	return "EpGuiRxSocketTask";
}

//////////////////////////////////////////////////////////////////////
//void CEpGuiRxSocket::HandleDisconnect()
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
void CEpGuiRxSocket::ReceiveFromSocket()
{
    // read message from Socket
    TPKT_HEADER_S  rHeaderStruct;
    int sizeRead=0;
    char* pHeaderBuf = (char*)(&rHeaderStruct);
    STATUS result = Read(pHeaderBuf,sizeof(TPKT_HEADER_S),sizeRead);

    if( result == STATUS_OK ) {

        DWORD bufLen = rHeaderStruct.payload_len;
        if( bufLen > MAX_SIM_GUI_MSG_LEN ) {
            PTRACE(eLevelError,"CEpGuiRxSocket::ReceiveFromSocket - too long msg");
        } else {
            ALLOCBUFFER(pMainBuf,bufLen);

            result = Read(pMainBuf,bufLen,sizeRead);
            if ( result == STATUS_OK ) {
                // put to segment Mbx of twin socket for response
                CSegment*  pMsgSeg = new CSegment;
                m_twinTask->Serialize(*pMsgSeg);
                // put message
//					*pMsgSeg << bufLen;
                pMsgSeg->Put((BYTE*)pMainBuf,bufLen);

                // send message to Manager Task
                const COsQueue* pManagerQueue =
                    CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessEndpointsSim,eManager);
                CTaskApi api;
                api.CreateOnlyApi(*pManagerQueue);
                STATUS a = api.SendMsg(pMsgSeg,GUI_SOCKET_RCV_MSG);
                api.DestroyOnlyApi();
            }
            DEALLOCBUFFER(pMainBuf);
        }
    }
}











