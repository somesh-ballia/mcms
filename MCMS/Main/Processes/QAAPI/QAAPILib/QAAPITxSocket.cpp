//+========================================================================+
//                   SimCardTxSocket.cpp                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       SimCardTxSocket.cpp                                         |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+


#include "SocketApi.h"
#include "QAAPITxSocket.h"
#include "HTTPDefi.h"
#include "TraceStream.h"
#include "SecuredSocketConnected.h"

/////////////////////////////////////////////////////////////////////////////
//
//   CSimCardTxSocket
//
/////////////////////////////////////////////////////////////////////////////

// message map
PBEGIN_MESSAGE_MAP(CQAAPITxSocket)
   ONEVENT( SOCKET_WRITE, ANYCASE,  CQAAPITxSocket::OnWriteSocketAnycase)
PEND_MESSAGE_MAP(CQAAPITxSocket,CSocketTxTask);


/////////////////////////////////////////////////////////////////////////////
//  task creation function
void QAAPITxEntryPoint(void* appParam)
{
	BYTE bSecured;
	CSegment* pSeg = (CSegment*)appParam;
	*pSeg >> (BYTE&)bSecured;
	pSeg->ResetRead();

	if (bSecured == TRUE)
	{
		FTRACEINTO << "\nCQAAPITxSocket::QAAPITxEntryPoint - secured connection";
		CQAAPITxSocket*  pTxSocket = new CQAAPITxSocket(new CSecuredSocketConnected);
		pTxSocket->Create(*(CSegment*)appParam);
	}
	else
	{
		FTRACEINTO << "\nCQAAPITxSocket::QAAPITxEntryPoint - not-secured connection";
		CQAAPITxSocket*  pTxSocket = new CQAAPITxSocket(new COsSocketConnected);
		pTxSocket->Create(*(CSegment*)appParam);
	}
	
	
}

/////////////////////////////////////////////////////////////////////////////
CQAAPITxSocket::CQAAPITxSocket(COsSocketConnected * pSocketDesc): CSocketTxTask(FALSE, pSocketDesc)     // constructor
{
}

/////////////////////////////////////////////////////////////////////////////
CQAAPITxSocket::~CQAAPITxSocket()     // destructor
{
}

/////////////////////////////////////////////////////////////////////////////
void*  CQAAPITxSocket::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
const char* CQAAPITxSocket::GetTaskName() const
{
	return "CQAAPITxSocket";
}

/////////////////////////////////////////////////////////////////////////////
void CQAAPITxSocket::InitTask()
{
}

//////////////////////////////////////////////////////////////////////
//void CSimCardTxSocket::HandleDisconnect()
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
void CQAAPITxSocket::OnWriteSocketAnycase(CSegment* pMsg)
{
	DWORD len,ContentLen;
	DWORD port;
	char *pRequest;
	*pMsg >> ContentLen;
	
	pRequest=new char[ContentLen+1];
	pRequest[ContentLen]='\0';
	//pMsg->Get((BYTE*)pRequest,len); 
	//pMsg->ResetRead();
	*pMsg >> pRequest;
	*pMsg >> port;
	*pMsg >> len;
	
	
	// fill LEVEL1 header information
	char *dir=new char [len+2];
	char *dir1=new char [len+1];
	dir[len+1]='\0';
	dir1[len]='\0';
	*pMsg >> dir1;
	*pMsg >> len;
	char *pIP=new char[len+1];
	pIP[len]='\0';
	*pMsg >> pIP;
		
	sprintf(dir,"/%s",dir1);
	AddPostHeader(dir);
	m_nContentType=CONTENT_TYPE_XML;
	AddContentTypeHeader();
	m_nContentLenght=ContentLen;
	AddContentLengthHeader();
	//CLanCfg lanCfg;
	//char *pIP=lanCfg.TranslDwordToString(m_pAppServerData->GetIp());
	AddHostHeader(pIP,port);
	//PDELETEA(pIP);
	AddLastCommonHeaders();

	//add error handle later 
	//PTRACE2(eLevelInfoHigh,"CQAAPITxSocket::OnWriteSocketAnycase - header ",m_pHTTPHeader);

    STATUS status = Write((char*)m_pHTTPHeader,m_HTTPHeaderLen);
    if(STATUS_OK != status)
    {
        AssertSendFailure("FAILED to send a header length", status);
    }
    else
    {
        status = Write(pRequest,ContentLen);
        if(STATUS_OK != status)
        {
            AssertSendFailure("FAILED to send a content", status);
        }
    }
    
    


//PTRACE2(eLevelInfoHigh,"CQAAPITxSocket::OnWriteSocketAnycase - content ",pRequest);
	PDELETEA(pRequest);
	PDELETEA(dir);
	PDELETEA(dir1);
	PDELETEA(pIP);
}

//////////////////////////////////////////////////////////////////////
void CQAAPITxSocket::AssertSendFailure(const char *str, STATUS status)const
{
    string  message = str;
    message += "\n";
    message += "Status : ";
    message += CProcessBase::GetProcess()->GetStatusAsString(status);
    PASSERTMSGONCE(TRUE, message.c_str());
}


