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
// FILE:       SimPcmRxSocket.cpp                                          |
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
#include "SimPcmRxSocket.h"
#include "StatusesGeneral.h"


/////////////////////////////////////////////////////////////////////////////
//
//   CSimPcmRxSocket
//
/////////////////////////////////////////////////////////////////////////////


PBEGIN_MESSAGE_MAP(CSimPcmRxSocket)
   ONEVENT( SOCKET_WRITE, ANYCASE,  CStateMachine::NullActionFunction)
PEND_MESSAGE_MAP(CSimPcmRxSocket,CSocketRxTask);


/////////////////////////////////////////////////////////////////////////////
//  task creation function
void SimPcmRxEntryPoint(void* appParam)
{
	CSimPcmRxSocket*  pRxSocket = new CSimPcmRxSocket;
	pRxSocket->Create(*(CSegment*)appParam);
}

/////////////////////////////////////////////////////////////////////////////
CSimPcmRxSocket::CSimPcmRxSocket()      // constructor
{
}

/////////////////////////////////////////////////////////////////////////////
CSimPcmRxSocket::~CSimPcmRxSocket()     // destructor
{
}

/////////////////////////////////////////////////////////////////////////////
void CSimPcmRxSocket::InitTask()
{
}

/////////////////////////////////////////////////////////////////////////////
void*  CSimPcmRxSocket::GetMessageMap()
{
	return (void*)m_msgEntries;
}


/////////////////////////////////////////////////////////////////////////////
const char* CSimPcmRxSocket::GetTaskName() const
{
	return "SimGuiRxSocketTask";
}

//////////////////////////////////////////////////////////////////////
//void CSimPcmRxSocket::HandleDisconnect()
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
void CSimPcmRxSocket::ReceiveFromSocket()
{
    // read message from Socket
    TPKT_HEADER_S  rHeaderStruct;
    char* pHeaderBuf = (char*)(&rHeaderStruct);
    int sizeRead=0;
    STATUS result = Read(pHeaderBuf,sizeof(TPKT_HEADER_S),sizeRead);
    
    if( result == STATUS_OK ) {
        //DWORD bufLen = rHeaderStruct.payload_len;		
        WORD nSize = rHeaderStruct.payload_len;
        DWORD bufLen = (((nSize)&0xff00)>>8)+(((nSize)&0xff)<<8)-4;
        if( bufLen > MAX_SIM_GUI_MSG_LEN ) {
            PTRACE(eLevelError,"CSimPcmRxSocket::ReceiveFromSocket - too long msg");
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
                STATUS a = api.SendMsg(pMsgSeg,SIM_PCM_SOCKET_RCV_MSG);
                api.DestroyOnlyApi();
            }
            DEALLOCBUFFER(pMainBuf);
        }
    }
}











