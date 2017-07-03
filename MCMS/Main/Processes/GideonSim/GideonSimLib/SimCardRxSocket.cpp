//+========================================================================+
//                    SimCardRxSocket.cpp                                  |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       SimCardRxSocket.cpp                                         |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

#include "SocketApi.h"
#include "MplMcmsStructs.h"
#include "MplMcmsProtocol.h"
#include "SimCardRxSocket.h"
#include "Macros.h"
#include "Trace.h"
#include "StatusesGeneral.h"
#include "MplMcmsProtocolTracer.h"
#include "GideonSim.h"
#include "WrappersCommon.h"
#include "TraceStream.h"

/////////////////////////////////////////////////////////////////////////////
//
//   CSimCardRxSocket
//
/////////////////////////////////////////////////////////////////////////////


PBEGIN_MESSAGE_MAP(CSimCardRxSocket)
   ONEVENT( SOCKET_WRITE, ANYCASE,  CStateMachine::NullActionFunction)
PEND_MESSAGE_MAP(CSimCardRxSocket,CSocketRxTask);


/////////////////////////////////////////////////////////////////////////////
//  task creation function
void SimCardRxEntryPoint(void* appParam)
{
	CSimCardRxSocket*  pRxSocket = new CSimCardRxSocket;
	pRxSocket->Create(*(CSegment*)appParam);
}

/////////////////////////////////////////////////////////////////////////////
CSimCardRxSocket::CSimCardRxSocket()      // constructor
{
}

/////////////////////////////////////////////////////////////////////////////
CSimCardRxSocket::~CSimCardRxSocket()     // destructor
{
}

/////////////////////////////////////////////////////////////////////////////
void CSimCardRxSocket::InitTask()
{
}

/////////////////////////////////////////////////////////////////////////////
void*  CSimCardRxSocket::GetMessageMap()
{
	return (void*)m_msgEntries;
}


/////////////////////////////////////////////////////////////////////////////
const char* CSimCardRxSocket::GetTaskName() const
{
	return "SimCardRxSocketTask";
}

//////////////////////////////////////////////////////////////////////
//void CSimCardRxSocket::HandleDisconnect()
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
void CSimCardRxSocket::ReceiveFromSocket()
{
    // read message from Socket
    char bufHdr[sizeof(TPKT_HEADER_S)];
    int sizeRead=0;
    STATUS result = Read(bufHdr,sizeof(TPKT_HEADER_S),sizeRead);
    
    if( result == STATUS_OK ) {
        CMplMcmsProtocol*  pProtocol = new CMplMcmsProtocol;
        
        DWORD bufLen = 0;
        bool res = ReadValidate_TPKT_Header(bufHdr, bufLen);
        if( false == res ) {
        	const TPKT_HEADER_S *temp_TPKT_Header = (const TPKT_HEADER_S *)bufHdr;
			TRACEINTO << CTPKTHeaderWrapper(*temp_TPKT_Header);
            PTRACE(eLevelError,"CSimCardRxSocket::ReceiveFromSocket - TPKT header corrupted");
        } else {
            ALLOCBUFFER(pMainBuf,bufLen);
            
            result = Read(pMainBuf,bufLen,sizeRead);
            if ( result == STATUS_OK ) {
                pProtocol->DeSerialize((const BYTE*)pMainBuf,bufLen);
                
                CMplMcmsProtocolTracer(*pProtocol).TraceMplMcmsProtocol("GIDEON_SIM_RECEIVE_FROM_MPL_API");
				TraceMplMcms(pProtocol);
                
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
    }
}

//////////////////////////////////////////////////////////////////////
void CSimCardRxSocket::TraceMplMcms(const CMplMcmsProtocol* pMplProt) const
{
	static const CProcessBase * process = CProcessBase::GetProcess();
	const std::string &str = process->GetOpcodeAsString(pMplProt->getCommonHeaderOpcode());
	const char* pszOpcodeStr = str.c_str();

	ALLOCBUFFER(pszMessage,512);
	sprintf(pszMessage,"Req: <%d>; HEADER: Board<%d>, SubBoard<%d>, Unit<%d>; OPCODE: <%s>",
		(int)pMplProt->getMsgDescriptionHeaderRequest_id(),
		(int)pMplProt->getPhysicalInfoHeaderBoard_id(),
		(int)pMplProt->getPhysicalInfoHeaderSub_board_id(),
		(int)pMplProt->getPhysicalInfoHeaderUnit_id(),
		pszOpcodeStr);
    LOGGER_TRACE2(eLevelInfoHigh,"GIDEON_SIM_RECEIVE_FROM_MPL_API - ",pszMessage);
    DEALLOCBUFFER(pszMessage);
}












