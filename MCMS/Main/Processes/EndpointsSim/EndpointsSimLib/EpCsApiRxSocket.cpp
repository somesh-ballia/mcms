//+========================================================================+
//                    EpCsApiRxSocket.cpp                                  |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       EpCsApiRxSocket.cpp                                         |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

#include "Macros.h"
#include "Trace.h"
#include "SocketApi.h"
#include "MplMcmsStructs.h"
#include "MplMcmsProtocol.h"

#include "SysConfig.h"
#include "ProcessBase.h"
#include "EpCsApiRxSocket.h"
#include "StatusesGeneral.h"
#include "XmlEngineApi.h"
#include "SystemFunctions.h"
#include "MplMcmsProtocolTracer.h"
#include "WrappersCommon.h"
#include "TraceStream.h"

/////////////////////////////////////////////////////////////////////////////
//
//   CEpCsApiRxSocket
//
/////////////////////////////////////////////////////////////////////////////


PBEGIN_MESSAGE_MAP(CEpCsApiRxSocket)
   ONEVENT( SOCKET_WRITE, ANYCASE,  CStateMachine::NullActionFunction)
PEND_MESSAGE_MAP(CEpCsApiRxSocket,CSocketRxTask);


/////////////////////////////////////////////////////////////////////////////
//  task creation function
void EpCsApiRxEntryPoint(void* appParam)
{
	CEpCsApiRxSocket*  pRxSocket = new CEpCsApiRxSocket;
	pRxSocket->Create(*(CSegment*)appParam);
}

/////////////////////////////////////////////////////////////////////////////
CEpCsApiRxSocket::CEpCsApiRxSocket()      // constructor
{
}

/////////////////////////////////////////////////////////////////////////////
CEpCsApiRxSocket::~CEpCsApiRxSocket()     // destructor
{
}

/////////////////////////////////////////////////////////////////////////////
void CEpCsApiRxSocket::InitTask()
{
}

/////////////////////////////////////////////////////////////////////////////
void*  CEpCsApiRxSocket::GetMessageMap()
{
	return (void*)m_msgEntries;
}


/////////////////////////////////////////////////////////////////////////////
const char* CEpCsApiRxSocket::GetTaskName() const
{
	return "EpCsApiRxSocketTask";
}

//////////////////////////////////////////////////////////////////////
void CEpCsApiRxSocket::ReceiveFromSocket()
{
		int sizeRead=0;
		// read message from Socket
		char bufHdr[sizeof(TPKT_HEADER_S)];
		STATUS result = Read(bufHdr,sizeof(TPKT_HEADER_S),sizeRead);

		if( result == STATUS_OK ) {
			CMplMcmsProtocol*  pProtocol = new CMplMcmsProtocol;

			DWORD bufLen = 0;
			bool res = ReadValidate_TPKT_Header(bufHdr, bufLen);
			if( false == res ) {
				const TPKT_HEADER_S *temp_TPKT_Header = (const TPKT_HEADER_S *)bufHdr;
				TRACEINTO << CTPKTHeaderWrapper(*temp_TPKT_Header);
//				PTRACE(eLevelError,"CEpCsApiRxSocket::ReceiveFromSocket - TPKT header corrupted");
			} else {
				ALLOCBUFFER(pMainBuf,bufLen+1);

				//SystemSleep(10);
				result = Read(pMainBuf,bufLen,sizeRead);
				pMainBuf[bufLen] = '\0';
				if ( result == STATUS_OK ) 
				{
					// check SYSTEM.CFG for XmlConverter use
					CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
					BOOL bIsXmlConverter = TRUE;
					std::string key = "CS_XML_CONVERTER";
					pSysConfig->GetBOOLDataByKey(key, bIsXmlConverter);
					
					if ( TRUE == bIsXmlConverter ) { // XML 
						int binaryMsgSize = 0;
						int opcode = 0;
						int newStatus = ::GetBinMessageSize(pMainBuf, &binaryMsgSize, &opcode);

						if( newStatus != 0 ) // is not OK
						{
							char* pTempStr = new char[128];
							sprintf(pTempStr,"XML converter failed. Status <%d>",newStatus);
							PTRACE2(eLevelError,"CEpCsApiRxSocket::ReceiveFromSocket - ",pTempStr);
							
							DBGPASSERT(newStatus);
							
							PDELETEA(pTempStr);
							POBJDELETE(pProtocol);
							DEALLOCBUFFER(pMainBuf);
							return;
						}

						//char strBuf[256];
						//sprintf(strBuf,"Binary len: %d, XML len: %d", binaryMsgSize, (int)bufLen);
						//PTRACE2( eLevelInfoNormal, "CEpCsApiRxSocket::ReceiveFromSocket +++++++RX-EpSim ", strBuf );
						//PTRACE2( eLevelInfoNormal, "CEpCsApiRxSocket::ReceiveFromSocket +++++++RX-EpSim ", pMainBuf );

				//		ALLOCBUFFER(BinaryMainbuf,binaryMsgSize);
						BYTE*  BinaryMainbuf = new BYTE [binaryMsgSize+100];
						memset(BinaryMainbuf, 0, binaryMsgSize+100);
						int statusBinMsg = ::GetBinMessage(pMainBuf, sizeRead,(char*)BinaryMainbuf);
						pProtocol->DeSerialize((const BYTE*)BinaryMainbuf,binaryMsgSize,CS_API_TYPE);

						CMplMcmsProtocolTracer(*pProtocol).TraceMplMcmsProtocol("CS_SIM_RECIVE_FROM_CS_API",CS_API_TYPE);

				//		DEALLOCBUFFER(BinaryMainbuf);
				PDELETEA(BinaryMainbuf);
					}
					else	// binary
					{
						pProtocol->DeSerialize((const BYTE*)pMainBuf,bufLen,CS_API_TYPE);
					}

					// send message to creator task
					CSegment* pMessage = new CSegment;
					pProtocol->Serialize(*pMessage,CS_API_TYPE);

					CTaskApi*  pTaskApi = new CTaskApi;
					pTaskApi->CreateOnlyApi(*m_pCreatorRcvMbx);
					STATUS status = pTaskApi->SendMsg(pMessage,SOCKET_RCV_MSG);
					pTaskApi->DestroyOnlyApi();
					POBJDELETE(pTaskApi);
				}
				else
				{
					PASSERT(result);
            }
            DEALLOCBUFFER(pMainBuf);
        }
        POBJDELETE(pProtocol);
    }
}











