#include "MplApiTxSocket.h"

#include <netinet/in.h>

#include "Segment.h"
#include "OpcodesMcmsInternal.h"
#include "Macros.h"
#include "MplMcmsStructs.h"
#include "ProcessBase.h"
#include "OsQueue.h"
#include "MplMcmsProtocol.h"
#include "StatusesGeneral.h"
#include "OpcodesMcmsCardMngrMaintenance.h"
#include "OsSocketConnected.h"
#include "TraceStream.h"

// message map
PBEGIN_MESSAGE_MAP(CMplApiTxSocket)
  ONEVENT(MPLAPI_MSG_TO_MPL, ANYCASE, CMplApiTxSocket::SendToSocket)
PEND_MESSAGE_MAP(CMplApiTxSocket, CStateMachine);

extern "C" void MplApiSocketTxEntryPoint(void* appParam)
{
  CMplApiTxSocket* pTaskApp = new CMplApiTxSocket;
  pTaskApp->Create(*(CSegment*)appParam);
  *(CSegment*)appParam << (void*)pTaskApp;
}

CMplApiTxSocket::CMplApiTxSocket()
{
	TRACEINTO << "Constructor";
}

CMplApiTxSocket::~CMplApiTxSocket()
{
	TRACEINTO_SOCKET << "Destructor";
}

void CMplApiTxSocket:: InitTask()
{
	TRACEINTO_SOCKET << "OK";
}

void CMplApiTxSocket:: AddFilterOpcodePoint()
{
  AddFilterOpcodeToQueue(MPLAPI_MSG_TO_MPL);
  AddFilterOpcodeToQueue(CM_KEEP_ALIVE_REQ);
}

void* CMplApiTxSocket::GetMessageMap()
{
  return (void*)m_msgEntries;
}

void CMplApiTxSocket::HandleDisconnect()
{
  // todo inform creator
  // 14.8.11 Rachel Cohen
  // mplApi bug VNGR-22185  only the creator ListenSocket.cpp
  // will Handle RX Disconnect . that will be when new connect on socket arrive from MFA
  // CSocketTask::HandleDisconnect();
   CloseSocket();
}

BOOL CMplApiTxSocket::TaskHandleEvent(CSegment* pMsg, DWORD msgLen,
                                      OPCODE opCode)
{
  return CSocketTxTask::TaskHandleEvent(pMsg, msgLen, opCode);
}

void CMplApiTxSocket::SendToSocket(CSegment& paramSegment)
{
	//Rachel Cohen - to avoid Sending on -1 File Descriptor - during disconect
	// in 7.7 we wil do a Fix that sends a message to Listen Socket that will kill TX and RX
  if (m_connection != NULL && m_connection->GetDescriptor() == -1)
	  return;

  TPKT_HEADER_S tTpktStruct;
  tTpktStruct.version_num = 3;
  tTpktStruct.reserved    = 0;

  CMplMcmsProtocol mplPrtcl;
  mplPrtcl.DeSerialize(paramSegment);
  OPCODE       opcode = mplPrtcl.getOpcode();
  DWORD        len = paramSegment.GetLen();
  eProcessType processType =
    (eProcessType)(mplPrtcl.getMsgDescriptionHeaderEntity_type());
  PushMessageToQueue(opcode, len, processType);

  static const WORD tpktHeaderSize = sizeof(TPKT_HEADER_S);
  WORD              payloadLen = paramSegment.GetWrtOffset() + tpktHeaderSize;
  tTpktStruct.payload_len   = (WORD)htons(payloadLen);

  CSegment seg;
  seg.Put((BYTE*)(&tTpktStruct), tpktHeaderSize);
  seg << paramSegment;

  STATUS status = Write((char*)(seg.GetPtr()), seg.GetWrtOffset());
  if (STATUS_OK != status)
  {
    std::string str = "FAILED to send to MFA(Tcp Socket)\n";
    str += "Status : ";
    str += CProcessBase::GetProcess()->GetStatusAsString(status);
    PASSERTMSGONCE(TRUE, str.c_str());
    TRACEINTO_SOCKET << "FAILED to send to MFA(Tcp Socket)";
  }
}
