//+========================================================================+
//                    SimGuiRxSocket.cpp                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       SimGuiRxSocket.cpp                                          |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

#include "Macros.h"
#include "Trace.h"
#include "ProcessBase.h"
#include "SocketApi.h"

#include "SimApi.h"
#include "GideonGuiApi.h"
#include "GideonSim.h"
#include "EpGuiPcmRxSocket.h"
#include "StatusesGeneral.h"


/////////////////////////////////////////////////////////////////////////////
//
//   CEpGuiPcmRxSocket
//
/////////////////////////////////////////////////////////////////////////////


PBEGIN_MESSAGE_MAP(CEpGuiPcmRxSocket)
   ONEVENT( SOCKET_WRITE, ANYCASE,  CStateMachine::NullActionFunction)
PEND_MESSAGE_MAP(CEpGuiPcmRxSocket,CSocketRxTask);


/////////////////////////////////////////////////////////////////////////////
//  task creation function
void EpGuiPcmRxEntryPoint(void* appParam)
{
	CEpGuiPcmRxSocket*  pRxSocket = new CEpGuiPcmRxSocket;
	pRxSocket->Create(*(CSegment*)appParam);
}

/////////////////////////////////////////////////////////////////////////////
CEpGuiPcmRxSocket::CEpGuiPcmRxSocket()      // constructor
{
}

/////////////////////////////////////////////////////////////////////////////
CEpGuiPcmRxSocket::~CEpGuiPcmRxSocket()     // destructor
{
}

/////////////////////////////////////////////////////////////////////////////
void CEpGuiPcmRxSocket::InitTask()
{
}

/////////////////////////////////////////////////////////////////////////////
void*  CEpGuiPcmRxSocket::GetMessageMap()
{
	return (void*)m_msgEntries;
}


/////////////////////////////////////////////////////////////////////////////
const char* CEpGuiPcmRxSocket::GetTaskName() const
{
	return "SimGuiRxSocketTask";
}

//////////////////////////////////////////////////////////////////////
//void CEpGuiPcmRxSocket::HandleDisconnect()
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
void CEpGuiPcmRxSocket::ReceiveFromSocket()
{
    // read message from Socket
    TPKT_HEADER_S  rHeaderStruct;
    char* pHeaderBuf = (char*)(&rHeaderStruct);
    int sizeRead=0;
    STATUS result = Read(pHeaderBuf,sizeof(TPKT_HEADER_S),sizeRead);
    
    if( result == STATUS_OK ) {
        
        DWORD bufLen = rHeaderStruct.payload_len;
        if( bufLen > MAX_SIM_GUI_MSG_LEN ) {
            PTRACE(eLevelError,"CEpGuiPcmRxSocket::ReceiveFromSocket - too long msg");
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
                    CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessGideonSim,eManager);
                CTaskApi api;
                api.CreateOnlyApi(*pManagerQueue);
                STATUS a = api.SendMsg(pMsgSeg,EPGUI_PCM_SOCKET_RCV_MSG);
                api.DestroyOnlyApi();
            }
            DEALLOCBUFFER(pMainBuf);
        }
    }
}











