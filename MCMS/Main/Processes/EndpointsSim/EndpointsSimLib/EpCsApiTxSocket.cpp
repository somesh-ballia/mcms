//+========================================================================+
//                   EpCsApiTxSocket.cpp                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       EpCsApiTxSocket.cpp                                         |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+


#include "Macros.h"
#include "MplMcmsProtocol.h"
#include "SocketApi.h"
#include "EpCsApiTxSocket.h"
#include "XmlEngineApi.h"
#include <netinet/in.h>

#include "SysConfig.h"
#include "ProcessBase.h"


/////////////////////////////////////////////////////////////////////////////
//
//   CEpCsApiTxSocket
//
/////////////////////////////////////////////////////////////////////////////

// message map
PBEGIN_MESSAGE_MAP(CEpCsApiTxSocket)
   ONEVENT( SOCKET_WRITE, ANYCASE,  CEpCsApiTxSocket::OnWriteSocketAnycase)
PEND_MESSAGE_MAP(CEpCsApiTxSocket,CSocketTxTask);


/////////////////////////////////////////////////////////////////////////////
//  task creation function
void EpCsApiTxEntryPoint(void* appParam)
{
	CEpCsApiTxSocket*  pTxSocket = new CEpCsApiTxSocket;
	pTxSocket->Create(*(CSegment*)appParam);
}

/////////////////////////////////////////////////////////////////////////////
CEpCsApiTxSocket::CEpCsApiTxSocket()      // constructor
{
}

/////////////////////////////////////////////////////////////////////////////
CEpCsApiTxSocket::~CEpCsApiTxSocket()     // destructor
{
}

/////////////////////////////////////////////////////////////////////////////
void*  CEpCsApiTxSocket::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
const char* CEpCsApiTxSocket::GetTaskName() const
{
	return "EpCsApiTxSocketTask";
}

/////////////////////////////////////////////////////////////////////////////
void CEpCsApiTxSocket::InitTask()
{
}

//////////////////////////////////////////////////////////////////////
void CEpCsApiTxSocket::OnWriteSocketAnycase(CSegment* paramSegment)
{
	// check SYSTEM.CFG for XmlConverter use
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	BOOL bIsXmlConverter = TRUE;
	std::string key = "CS_XML_CONVERTER";
	pSysConfig->GetBOOLDataByKey(key, bIsXmlConverter);
					
	if ( TRUE != bIsXmlConverter) {	// binary API
		// fill LEVEL1 header information
		TPKT_HEADER_S   tTpktStruct;
		tTpktStruct.version_num      = TPKT_VERSION_NUM;
		tTpktStruct.reserved		 = 0;
//		tTpktStruct.payload_len      = (WORD)(paramSegment->GetWrtOffset());
		APIU16 len = (APIU16)(paramSegment->GetWrtOffset()) + sizeof(TPKT_HEADER_S);
		tTpktStruct.payload_len = (htons(len));

		CSegment* pMsg = new CSegment; // pMsg will delete inside
		
		// put LEVEL1 header to message
		pMsg->Put((BYTE*)(&tTpktStruct),sizeof(TPKT_HEADER_S));
		
		*pMsg << *paramSegment;
		
		Write((char*)(pMsg->GetPtr()),pMsg->GetWrtOffset());
		POBJDELETE(pMsg);
		
	}
	else	// XML API
	{
		
		DWORD	nMsgLen =  paramSegment->GetWrtOffset() - paramSegment->GetRdOffset();
		if( !nMsgLen ) {
			PTRACE(eLevelError,"CEpCsApiTxSocket::OnWriteSocketAnycase - empty params, message can't be sent.");
			return;
		}
		BYTE*	pMessage = new BYTE[nMsgLen];
		paramSegment->Get(pMessage,nMsgLen);
		
		int XmlMessageLen = 0;
		int newStatus = ::GetXmlMessageLen( (char*)pMessage, &XmlMessageLen );
		PDELETEA(pMessage);

		if( newStatus != 0 ) // is not OK
		{
			char* pTempStr = new char[128];
			sprintf(pTempStr,"XML converter failed. Status <%d>",newStatus);
			PTRACE2(eLevelError,"CEpCsApiTxSocket::OnWriteSocketAnycase - ",pTempStr);
			
//			DBGPASSERT(newStatus);
			
			PDELETEA(pTempStr);
			return;
		}
		ALLOCBUFFER(XmlMessageBuff,XmlMessageLen+1);
		newStatus = ::GetXmlMessage(XmlMessageLen,XmlMessageBuff);
		XmlMessageBuff[XmlMessageLen] = '\0';
		//char strBuf[256];
		//sprintf(strBuf,"Binary len: %d, XML len: %d", (int)nMsgLen, XmlMessageLen);
//		PTRACE2( eLevelInfoNormal, "CEpCsApiTxSocket::OnWriteSocketAnycase +++++++TX-EpSim ", strBuf );
		//PTRACE2( eLevelInfoNormal, "CEpCsApiTxSocket::OnWriteSocketAnycase +++++++TX-EpSim ", XmlMessageBuff );

		// fill LEVEL1 header information
		TPKT_HEADER_S   tStructLevel1;
		tStructLevel1.version_num	= TPKT_VERSION_NUM;
		tStructLevel1.reserved		= 0;
		tStructLevel1.payload_len	= (htons((WORD)(XmlMessageLen)+sizeof(TPKT_HEADER_S)));
		
		Write((char*)(&tStructLevel1),sizeof(TPKT_HEADER_S));

		Write((char*)XmlMessageBuff,XmlMessageLen);

		DEALLOCBUFFER(XmlMessageBuff);
	}
	
}






