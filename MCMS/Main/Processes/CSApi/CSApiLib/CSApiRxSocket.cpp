//+========================================================================+
//                         CSApiRxSocket.cpp                               |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       CSApiRxSocket.cpp                                           |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Shlomit                                                     |
//-------------------------------------------------------------------------|
// Who | Date 07.06.05    | Description                                    |
//-------------------------------------------------------------------------|
//+========================================================================+
#include "CSApiRxSocket.h"
#include "ListenSocketApi.h"
#include "MplMcmsProtocol.h"
#include "CSApiRxSocketMngr.h"
#include "Segment.h"
#include "TaskApi.h"
#include "Trace.h"
#include "TraceStream.h"
#include "Macros.h"
#include "CSApiProcess.h"
#include "StatusesGeneral.h"
#include "XmlEngineApi.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "ProcessBase.h"
#include "IpCsOpcodes.h"

static CCSApiProcess *pCSApiProcess = NULL;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
extern "C" void CSApiSocketRxEntryPoint(void* appParam)
{  	
	CCSApiRxSocket*  pTaskApp = new CCSApiRxSocket;
	pTaskApp->Create(*(CSegment*)appParam);
	*(CSegment*)appParam << (void*)pTaskApp ;
}

//////////////////////////////////////////////////////////////////////
CCSApiRxSocket::CCSApiRxSocket()
{
	m_IsKnownCsID=NO;
}

//////////////////////////////////////////////////////////////////////
CCSApiRxSocket::~CCSApiRxSocket()
{
}

//////////////////////////////////////////////////////////////////////
void CCSApiRxSocket::InitTask()
{
	pCSApiProcess = static_cast<CCSApiProcess*>(CProcessBase::GetProcess());
	bool bMultipleServices = false;
	bool bXMLMode = false;
	char* pSim = getenv ("ENDPOINTSSIM");
	pCSApiProcess->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_MULTIPLE_SERVICES, bMultipleServices);
	eProductType prodType = pCSApiProcess->GetProductType();
	eProductFamily prodFamily = pCSApiProcess->GetProductFamily();
	eCommProtocol currentProtocol = eBinary;
	if (pSim && strncmp(pSim, "YES", 3) == 0)
		currentProtocol = eXML;
	else if (bMultipleServices && eProductFamilySoftMcu == prodFamily && prodType != eProductTypeGesher && prodType != eProductTypeNinja)
	{
		currentProtocol = eBinary;
		TRACEINTO << "Current Protocol: " << (int)currentProtocol
				<< ", MultipleServices: " << bMultipleServices
				<< ", prodFamily: " << (int)prodFamily
				<< ", prodType: " << (int)prodType;
	}
	else
	{
		pCSApiProcess->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_CS_API_XML_MODE, bXMLMode);
		char* pSimTest = getenv ("CS_SIMULATION_TEST");
		if (pSimTest && strncmp(pSimTest, "YES", 3) == 0)
				currentProtocol = eXML;
		else
			currentProtocol = bXMLMode ? eXML : eBinary;
		TRACEINTO << "Current Protocol: " << (int)currentProtocol
				<< ", XMLMode: " << bXMLMode;
	}
	pCSApiProcess->SetCurrentProtocol(currentProtocol);
}

//////////////////////////////////////////////////////////////////////
void CCSApiRxSocket::HandleDisconnect()
{
	CSocketTask::HandleDisconnect();
}

//////////////////////////////////////////////////////////////////////
void CCSApiRxSocket::ReceiveFromSocket()
{
	int sizeRead=0;
	char bufHdr[sizeof(TPKT_HEADER_S)];
	STATUS stat = Read(bufHdr, sizeof(TPKT_HEADER_S),sizeRead);
	if (STATUS_OK != stat)
	{
//		PASSERTMSG(stat, "FAILED to Read TPKT header");
		return;
	}
	if(sizeof(TPKT_HEADER_S) != sizeRead)
    {
        OnFailReadFromSocket("TPKT_HEADER_S size not valid");
        return;
    }
    
	CMplMcmsProtocol csPrtcl;
	DWORD mainBufLen = 0;
	bool res = ReadValidate_TPKT_Header(bufHdr, mainBufLen);
	if(false == res)
	{
		OnCorruptedTPKT(bufHdr);
		return;
	}

	stat = ReadMainBuff(csPrtcl, mainBufLen);
	if(STATUS_OK != stat)
	{
		PASSERTMSG(stat, "FAILED to Read Main Buffer");		
		return;
	}
    
	OPCODE opcode = csPrtcl.getOpcode();
	PushMessageToQueue(opcode, mainBufLen);

	UpdateCsId(csPrtcl);
	
	CCSApiRxSocketMngr csApiRxSocketMngr;
	DWORD len = csApiRxSocketMngr.SendCSEventToTheAppropriateProcess(csPrtcl);
}

////////////////////////////////////////////////////////////////////////////////////
BOOL  CCSApiRxSocket::IsKnownCsID()
{
	return m_IsKnownCsID;
}
////////////////////////////////////////////////////////////////////////////////////
void  CCSApiRxSocket::SetKnownCsID(BOOL knownCsID)
{
	m_IsKnownCsID=knownCsID;
}

////////////////////////////////////////////////////////////////////////////////////
STATUS CCSApiRxSocket::ReadMainBuff(CMplMcmsProtocol &CSPrtcl, DWORD mainBufLen)
{
	ALLOCBUFFER(mainBuf,mainBufLen);
	int sizeRead=0;
	STATUS stat = Read(mainBuf, mainBufLen,sizeRead);
	if (STATUS_OK != stat)
	{
		PTRACE(eLevelError,"CCSApiRxSocket::ReadMainBuff - Failed to read message content");
		DEALLOCBUFFER(mainBuf);
		
		return STATUS_FAIL;	
	}
    if(mainBufLen != (DWORD)sizeRead)
    {
        DEALLOCBUFFER(mainBuf);
        OnFailReadFromSocket("Main buffer size not valid");
        DEALLOCBUFFER(mainBuf);
        
        return STATUS_FAIL;
    }
    
	eCommProtocol currentProtocol = GetCurrentProtocol(mainBuf[0]);
	if(eXML == currentProtocol)
	{
		char *binaryMainbuf = NULL;
		int binaryMsgSize = 0;
		stat = XmlToBinary(mainBuf, binaryMainbuf, sizeRead, binaryMsgSize);
		if(STATUS_OK != stat)
		{
			PTRACE(eLevelError,"CCSApiRxSocket::ReadMainBuff - Failed to convert from XML to Binary");
			DEALLOCBUFFER(mainBuf);
			
			return STATUS_FAIL;
		}
		if(NULL == binaryMainbuf)
		{
			PTRACE(eLevelError,"CCSApiRxSocket::ReadMainBuff - binaryMainbuf is NULL!!!");
			DEALLOCBUFFER(mainBuf);

			return STATUS_FAIL;
		}
		
		bool resDeserialize = CSPrtcl.DeSerialize((const BYTE*)binaryMainbuf,binaryMsgSize,CS_API_TYPE);
        if(!resDeserialize)
        {
            DEALLOCBUFFER(binaryMainbuf);
    		DEALLOCBUFFER(mainBuf);
            PASSERTMSG(1, "CCSApiRxSocket::ReadMainBuff - Failed to deserialize, message rejected");
            return STATUS_FAIL;
        }
        
		DEALLOCBUFFER(binaryMainbuf);
	}
	else if(eBinary == currentProtocol)
	{
		CSPrtcl.DeSerialize((const BYTE*)mainBuf,mainBufLen,CS_API_TYPE);
	}
	else
	{
		PASSERTMSG(currentProtocol, "Illegal Protocol type, It must be eXML or eBinary");
		DEALLOCBUFFER(mainBuf);
		
		return STATUS_FAIL;
	}
	
	DEALLOCBUFFER(mainBuf);

	return STATUS_OK;
}
////////////////////////////////////////////////////////////////////////////////////
STATUS CCSApiRxSocket::XmlToBinary(char *Mainbuf, char *& BinaryMainbuf, int xmlMsgLen, int &binaryMsgSize)const
{
	int len = 0;
	int opcode = 0;
	int res = GetBinMessageSize(Mainbuf,&len,&opcode);
	binaryMsgSize = len;
	if(0 != res)
	{
		PTRACE(eLevelError,"CCSApiRxSocket::ReadMainBuff - Failed to get binary message size");
		return STATUS_FAIL;
	}
	
//	PTRACE2(eLevelInfoNormal, "XML TRACE \nXML --> Binary\n", Mainbuf);
	BinaryMainbuf = new char[binaryMsgSize];
	memset(BinaryMainbuf,'\0',binaryMsgSize);
	res = GetBinMessage(Mainbuf, xmlMsgLen, BinaryMainbuf);
	if(0 != res)
	{
		printf("res = %d\n", res);
		PTRACE(eLevelError,"CCSApiRxSocket::ReadMainBuff - Failed to get binary message");
		PASSERT(res);		
		PDELETEA(BinaryMainbuf);

		return STATUS_FAIL;
	}

	return STATUS_OK;
}
////////////////////////////////////////////////////////////////////////////////////
void CCSApiRxSocket::UpdateCsId(CMplMcmsProtocol &csPrtcl)
{
	if(IsKnownCsID() == NO)
	{
		CCSApiProcess* pProcess = (CCSApiProcess*)CCSApiProcess::GetProcess();
		pProcess->UpdateCardOrCs2ConnectionId( csPrtcl.getCentralSignalingHeaderCsId(), m_socketConnectionId );
		SetKnownCsID(YES);
	}
}

////////////////////////////////////////////////////////////////////////////////////
eCommProtocol CCSApiRxSocket::GetCurrentProtocol(char testChar)
{
	CCSApiProcess* pProcess = static_cast<CCSApiProcess*>(CProcessBase::GetProcess());
	return pProcess->GetCurrentProtocol();
}

////////////////////////////////////////////////////////////////////////////////////
bool CCSApiRxSocket::IsXTimeHere(WORD time)
{
	static WORD val = 0;
	val++;
	
	return time == val;
}

//////////////////////////////////////////////////////////////////////
void CCSApiRxSocket::AddFilterOpcodePoint()
{
	AddFilterOpcodeToQueue(CS_KEEP_ALIVE_IND);
}
