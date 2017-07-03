//+========================================================================+
//                         CSApiTxSocket.cpp                               |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       CSApiTxSocket.cpp                                           |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Shlomit                                                     |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//+========================================================================+
#ifndef LINUX
#include <Winsock.h>
#else
#include <netinet/in.h>
#endif
#include "CSApiTxSocket.h"
#include "Segment.h"
#include "OpcodesMcmsInternal.h"
#include "Macros.h"
#include "MplMcmsStructs.h"
#include "ProcessBase.h"
#include "OsQueue.h"
#include "MplMcmsProtocol.h"
#include "XmlEngineApi.h"

#include "CSApiDefines.h"
#include "CSApiProcess.h"

// message map
PBEGIN_MESSAGE_MAP(CCSApiTxSocket)
ONEVENT(CS_API_MSG_TO_CS      ,ANYCASE    , CCSApiTxSocket::SendToSocket)
PEND_MESSAGE_MAP(CCSApiTxSocket,CStateMachine); 

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
extern "C" void CSApiSocketTxEntryPoint(void* appParam)
{  	
	CCSApiTxSocket*  pTaskApp = new CCSApiTxSocket;
	pTaskApp->Create(*(CSegment*)appParam);
	*(CSegment*)appParam << (void*)pTaskApp ;
}


//////////////////////////////////////////////////////////////////////
CCSApiTxSocket::CCSApiTxSocket()
{

}

//////////////////////////////////////////////////////////////////////
CCSApiTxSocket::~CCSApiTxSocket()
{

}

//////////////////////////////////////////////////////////////////////
void CCSApiTxSocket::InitTask()
{
	
}

//////////////////////////////////////////////////////////////////////
void*  CCSApiTxSocket::GetMessageMap()
{
	return (void*)m_msgEntries;    
}

//////////////////////////////////////////////////////////////////////
void CCSApiTxSocket::HandleDisconnect()
{
	// todo inform creator
}


/////////////////////////////////////////////////////////////////////////////
BOOL CCSApiTxSocket::TaskHandleEvent(CSegment *pMsg,DWORD  msgLen,OPCODE opCode)
{
//	switch (opCode)
//	{
////		case 9999:
////		{
////			char buffer[] = {"*"};
////			Write(buffer,1);			
////			return TRUE;
////		}
//	}

	return CSocketTxTask::TaskHandleEvent(pMsg,msgLen,opCode);
}

///////////////////////////////////////////////////////////////////////////////////////
void CCSApiTxSocket::SendToSocket(CSegment &paramSegment) 
{
	TPKT_HEADER_S tpkt;
	tpkt.version_num   = 3;
	tpkt.reserved 		= 0;
	tpkt.payload_len    = 0;
	
	CSegment seg;
	STATUS status = STATUS_OK;

	CCSApiProcess *process = (CCSApiProcess*)(CProcessBase::GetProcess());
	eCommProtocol currentProtocol = process->GetCurrentProtocol();	
	
	if(eBinary == currentProtocol)
	{
		status = FillBinarySegment(seg, tpkt, paramSegment);
		if(STATUS_FAIL == status)
		{
			PASSERTMSG(1, "FAILED to put BINARY message into segment, message canceled");
			return;
		}
	}
	else if(eXML == currentProtocol)
	{
		status = FillXMLSegment(seg, tpkt, paramSegment);
		if(STATUS_FAIL == status)
		{
			PASSERTMSG(1, "FAILED to put XML message into segment, message canceled");
			return;
		}
	}
	else
	{
		PASSERTMSG(currentProtocol, "Illegal Protocol type, It must be eXML or eBinary");
		return;
	}
	
	status = Write((char*)(seg.GetPtr()), seg.GetWrtOffset());
    if(STATUS_OK != status)
    {
        std::string str = "FAILED to send to CS(Tcp Socket)\n";
        str += "Status : ";
        str += process->GetStatusAsString(status);
        PASSERTMSGONCE(TRUE, str.c_str());
    }
}
/////////////////////////////////////////////////////////////////////////////

STATUS CCSApiTxSocket::FillBinarySegment(CSegment &outSeg, TPKT_HEADER_S &outTPKT, CSegment &inSeg)
{
  APIU16 len = (APIU16)(inSeg.GetWrtOffset() + sizeof(TPKT_HEADER_S));
  outTPKT.payload_len = (htons(len));

	outSeg.Put((BYTE*)(&outTPKT),sizeof(TPKT_HEADER_S));
	outSeg << inSeg;
	
	return STATUS_OK;
}

STATUS CCSApiTxSocket::FillXMLSegment(CSegment &outSeg, TPKT_HEADER_S &outTPKT, CSegment &inSeg)
{
	DWORD nMsgLen =  inSeg.GetWrtOffset() - inSeg.GetRdOffset();
	if(0 == nMsgLen)
	{
		PASSERTMSG(1, "Empty Message, nothing sent");
		return STATUS_FAIL;
	}
	
	ALLOCINITBUFFER(pMessage, nMsgLen, 0);
	inSeg.Get((BYTE*)pMessage,nMsgLen);

    int XmlMessageLen = 0;
	int newStatus = GetXmlMessageLen((char*)pMessage, &XmlMessageLen);
	DEALLOCBUFFER(pMessage);
	if(0 != newStatus || 0 == XmlMessageLen)
	{
		PASSERTMSG(newStatus, "GetXmlMessageLen FAILED. nothing sent");
		return STATUS_FAIL;
	}
	
    ALLOCINITBUFFER(XmlMessageBuff, XmlMessageLen, 0);

	newStatus = GetXmlMessage(XmlMessageLen, XmlMessageBuff);
	if(0 != newStatus)
	{
		PASSERTMSG(newStatus, "GetXmlMessage FAILED. nothing sent");
		DEALLOCBUFFER(XmlMessageBuff);
		return STATUS_FAIL;
	}
	
	APIU16 len = (APIU16)(XmlMessageLen + sizeof(TPKT_HEADER_S));
	outTPKT.payload_len = (htons(len));
	
	outSeg.Put((BYTE*)(&outTPKT),sizeof(TPKT_HEADER_S));
	outSeg.Put((BYTE*)XmlMessageBuff, XmlMessageLen);
	
	DEALLOCBUFFER(XmlMessageBuff);
		
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
void CCSApiTxSocket::AddFilterOpcodePoint()
{
	AddFilterOpcodeToQueue(CS_API_MSG_TO_CS);
}

