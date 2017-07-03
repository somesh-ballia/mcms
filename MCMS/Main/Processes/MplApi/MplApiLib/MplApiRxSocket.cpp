//+========================================================================+
//                         oprtr.cpp                                       |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       oprtr.cpp                                                   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//+========================================================================+
#include "MplApiRxSocket.h"
#include "ListenSocketApi.h"
#include "MplApiRxSocketMngr.h"
#include "Segment.h"
#include "TaskApi.h"
#include "Trace.h"
#include "Macros.h"
#include "MplApiProcess.h"
#include "StatusesGeneral.h"
#include "MplApiSpecialCommandHandler.h"
#include "MplMcmsProtocolTracer.h"
#include "OpcodesMcmsShelfMngr.h"
#include "OpcodesMcmsCardMngrMaintenance.h"
#include "TraceStream.h"
#include "WrappersCommon.h"
#include "OsSocketConnected.h"
#include "OpcodesMcmsVideo.h"


static CMplApiProcess *pMplApiProcess = NULL;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
extern "C" void MplApiSocketRxEntryPoint(void* appParam)
{  	
	CMplApiRxSocket*  pTaskApp = new CMplApiRxSocket;
	pTaskApp->Create(*(CSegment*)appParam);
	*(CSegment*)appParam << (void*)pTaskApp ;
}

//////////////////////////////////////////////////////////////////////
CMplApiRxSocket::CMplApiRxSocket()
{
	TRACEINTO << "Constructor";

	pMplApiProcess = dynamic_cast<CMplApiProcess *>(CProcessBase::GetProcess());
	m_IsKnownBoardID=NO;
	m_pMplApiRxSocketMngr = new CMplApiRxSocketMngr;
}

//////////////////////////////////////////////////////////////////////
CMplApiRxSocket::~CMplApiRxSocket()
{
	TRACEINTO_SOCKET << "Destructor";

	POBJDELETE(m_pMplApiRxSocketMngr);
}

//////////////////////////////////////////////////////////////////////
void CMplApiRxSocket::InitTask()
{
	TRACEINTO_SOCKET << "OK"; //"Starting Timer of 15 seconds to get data in the socket";
	//StartTimer(OPEN_SOCKET_TIMER,15*SECOND);
}

void CMplApiRxSocket::AddFilterOpcodePoint()
{
	AddFilterOpcodeToQueue(SM_KEEP_ALIVE_IND);
	AddFilterOpcodeToQueue(CM_KEEP_ALIVE_IND);
}

//////////////////////////////////////////////////////////////////////
void CMplApiRxSocket::HandleDisconnect()
{
	//14.8.11 Rachel Cohen
	//mplApi bug VNGR-22185  only the creator ListenSocket.cpp
	// will Handle RX Disconnect . that will be when new connect on socket arrive from MFA
	//CSocketTask::HandleDisconnect();
	TRACEINTO_ERR_SOCKET << "Call to Close Socket";
	CloseSocket();
}

//////////////////////////////////////////////////////////////////////
void CMplApiRxSocket::ReceiveFromSocket()
{
	CMplMcmsProtocol mplPrtcl;
  	char bufHdr[sizeof(TPKT_HEADER_S)];
  	int sizeRead=0;
  	STATUS stat = Read(bufHdr, sizeof(TPKT_HEADER_S),sizeRead,FALSE);
  	if(STATUS_OK != stat)
    {
		return;  
    }
  	if(sizeof(TPKT_HEADER_S) != sizeRead)
    {
    	OnFailReadFromSocket("TPKT_HEADER_S size not valid");
      	return;
    }

//     const TPKT_HEADER_S *temp_TPKT_Header = (const TPKT_HEADER_S *)bufHdr;
//     TRACEINTO << CTPKTHeaderWrapper(*temp_TPKT_Header);
    
  	DWORD mainBufLen = 0;
  	bool res = ReadValidate_TPKT_Header(bufHdr, mainBufLen);

  	if (mainBufLen == 0)
  	{
  		TRACEINTO_SOCKET << "Corrupted Message with Payload 4 message rejected";

  	    return;
  	}

  	if(false == res)
    {
   		OnCorruptedTPKT(bufHdr,"TPKT header are corrupted");
      	return;
    }
    
  	ALLOCBUFFER(mainbuf,mainBufLen);
  	stat = Read(mainbuf,mainBufLen,sizeRead,FALSE);
  	if(STATUS_OK != stat)
    {
    	PASSERTMSG(1, "CMplApiRxSocket::ReceiveFromSocket - Failed to read msg content, message canceled");
      	DEALLOCBUFFER(mainbuf);
    	return;
    }
  	if(mainBufLen != (DWORD)sizeRead)
    {
    	DEALLOCBUFFER(mainbuf);
      	OnFailReadFromSocket("Main buffer size not valid");
      	return;
    }
    
  	bool resDeserialize = mplPrtcl.DeSerialize((const BYTE*)mainbuf,mainBufLen);
	if(!resDeserialize)
    {
        DEALLOCBUFFER(mainbuf);
        
        const MplMcmsProtocolErrno_S &protErrno = mplPrtcl.Errno();
        TRACEINTO_SOCKET << "\nCorrupted Message : " << protErrno.m_Status << "\n"
                  << protErrno.m_Message;
        PASSERTMSG(1, "CMplApiRxSocket::ReceiveFromSocket - Failed to deserialize, message rejected");
      	return;
    }
    
  	OPCODE opcodeBefore = mplPrtcl.getOpcode();
#ifdef PRFFORMANCE_ANALYSIS
  	if (mainBufLen > 140)
  		TRACEINTO_SOCKET << "Performance analysis:\n\tMessage length: " << mainBufLen << ", opcode: " << opcodeBefore;
#endif
  	PushMessageToQueue(opcodeBefore, mainBufLen);

  	CMplApiSpecialCommandHandler handler(mplPrtcl);
  	STATUS status = handler.HandeSpecialCommand();
  	if(STATUS_OK != status)
    {
    	pMplApiProcess->OnSpecialCommanderFailure(mplPrtcl, opcodeBefore, status);
    	DEALLOCBUFFER(mainbuf);
      	return;
    }
	
  	UpdateBoardId(mplPrtcl);

  	DWORD len = m_pMplApiRxSocketMngr->SendMplEventToTheAppropriateProcess(&mplPrtcl);	
  	
  	//Since change layout is important
  	if (mplPrtcl.getOpcode() == VIDEO_ENCODER_CHANGE_LAYOUT_REQ )
  		CMplMcmsProtocolTracer(mplPrtcl).TraceMplMcmsProtocol("MPLAPI_RECEIVED_MESSAGE_FROM_CARDS");

  	//StartTimer(OPEN_SOCKET_TIMER,20*SECOND);

  	DEALLOCBUFFER(mainbuf);
}

////////////////////////////////////////////////////////////////////////////////////
BOOL  CMplApiRxSocket::IsKnownBoardID()
{
	return m_IsKnownBoardID;
}
////////////////////////////////////////////////////////////////////////////////////
void  CMplApiRxSocket::SetKnownBoardID(BOOL knownBoardID)
{
	m_IsKnownBoardID=knownBoardID;
}
////////////////////////////////////////////////////////////////////////////////////
void CMplApiRxSocket::UpdateBoardId(CMplMcmsProtocol &mplPrtcl)
{
	if(IsKnownBoardID() == NO)
	{
		CMplApiProcess* pProcess = (CMplApiProcess*) CMplApiProcess::GetProcess();
		pProcess->UpdateCardOrCs2ConnectionId( mplPrtcl.getPhysicalInfoHeaderBoard_id(),
		                                   m_socketConnectionId);
		SetKnownBoardID(YES);
		
	}
}

