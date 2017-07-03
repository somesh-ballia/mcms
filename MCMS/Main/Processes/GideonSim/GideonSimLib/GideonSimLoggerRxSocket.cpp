//+========================================================================+
//                GideonSimLoggerRxSocket.cpp                              |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       GideonSimLoggerRxSocket.cpp                                 |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

#include "Macros.h"
#include "Trace.h"
#include "StatusesGeneral.h"
#include "SocketApi.h"
#include "MplMcmsStructs.h"
//#include "MplMcmsProtocol.h"
//#include "MplMcmsProtocolTracer.h"
#include "GideonSimLoggerRxSocket.h"

/////////////////////////////////////////////////////////////////////////////
//
//   CGideonSimLoggerRxSocket
//
/////////////////////////////////////////////////////////////////////////////


PBEGIN_MESSAGE_MAP(CGideonSimLoggerRxSocket)
   ONEVENT( SOCKET_WRITE, ANYCASE,  CStateMachine::NullActionFunction)
PEND_MESSAGE_MAP(CGideonSimLoggerRxSocket,CSocketRxTask);


/////////////////////////////////////////////////////////////////////////////
//  task creation function
void GideonSimLoggerRxEntryPoint(void* appParam)
{
	CGideonSimLoggerRxSocket*  pRxSocket = new CGideonSimLoggerRxSocket;
	pRxSocket->Create(*(CSegment*)appParam);
}

/////////////////////////////////////////////////////////////////////////////
CGideonSimLoggerRxSocket::CGideonSimLoggerRxSocket()      // constructor
{
}

/////////////////////////////////////////////////////////////////////////////
CGideonSimLoggerRxSocket::~CGideonSimLoggerRxSocket()     // destructor
{
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimLoggerRxSocket::InitTask()
{
}

/////////////////////////////////////////////////////////////////////////////
void*  CGideonSimLoggerRxSocket::GetMessageMap()
{
	return (void*)m_msgEntries;
}


/////////////////////////////////////////////////////////////////////////////
const char* CGideonSimLoggerRxSocket::GetTaskName() const
{
	return "GideonSimLoggerRxSocket";
}

//////////////////////////////////////////////////////////////////////
//void CGideonSimLoggerRxSocket::HandleDisconnect()
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
void CGideonSimLoggerRxSocket::ReceiveFromSocket()
{
/*    // read message from Socket
    char bufHdr[sizeof(TPKT_HEADER_S)];
    int sizeRead=0;
    STATUS result = Read(bufHdr,sizeof(TPKT_HEADER_S),sizeRead);
    
    if( result == STATUS_OK ) {
        CMplMcmsProtocol*  pProtocol = new CMplMcmsProtocol;
        
        DWORD bufLen = pProtocol->Read_TPKT_Header(bufHdr);
        if( bufLen > MAX_MPL_API_MSG_LEN ) {
            PTRACE(eLevelError,"CSimCardRxSocket::ReceiveFromSocket - too long msg");
        } else {
            ALLOCBUFFER(pMainBuf,bufLen);
            
            result = Read(pMainBuf,bufLen,sizeRead);
            if ( result == STATUS_OK ) {
                pProtocol->DeSerialize((const BYTE*)pMainBuf,bufLen);
                
                CMplMcmsProtocolTracer(*pProtocol).TraceMplMcmsProtocol("GIDEON_SIM_RECEIVE_FROM_MPL_API");
                
                // send message to creator task
                CSegment* pMessage = new CSegment;
                pProtocol->Serialize(*pMessage);
                
                CTaskApi*  pTaskApi = new CTaskApi;
                pTaskApi->CreateOnlyApi(*m_pCreatorRcvMbx);
                pTaskApi->SendMsg(pMessage,SOCKET_RCV_MSG);
                pTaskApi->DestroyOnlyApi();
                POBJDELETE(pTaskApi);
            }
            DEALLOCBUFFER(pMainBuf);
        }
        POBJDELETE(pProtocol);
    }*/
}

BOOL CGideonSimLoggerRxSocket::IsValidTimer(OPCODE opcode)
{
	CProcessBase* proc = CProcessBase::GetProcess();
	if (NULL == proc)
		return FALSE;
	//PASSERT_AND_RETURN_VALUE(NULL == proc, FALSE);

	CTaskApp* task = proc->GetCurrentTask();

	if (task == NULL)
	{
		return FALSE;
	}
	return CSocketRxTask::IsValidTimer(opcode);
}











