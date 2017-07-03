//+========================================================================+
//                       CSApiDispatcherTask.cpp                           |
//                     Copyright 2005 Polycom                              |
//                      All Rights Reserved.                               |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       CSApiDispatcherTask.cpp                                     |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Shlomit                                                     |
//-------------------------------------------------------------------------|
// Who    | Date       | Description                                       |
//-------------------------------------------------------------------------|
// Shlomit| 07.6.05    | Interprocess tasks                                |
//+========================================================================+


#include "CSApiDispatcherTask.h"
#include "DispatcherTask.h"
#include "Segment.h"
#include "ProcessBase.h"
#include "Trace.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "Macros.h"
#include "MplMcmsProtocol.h"
#include "TaskApi.h"
#include "MplMcmsStructs.h"
#include "CSApiProcess.h"
#include "ListenSocket.h"
#include "CSApiMplMcmsProtocolTracer.h"
#include "TraceStream.h"

PBEGIN_MESSAGE_MAP(CCSApiDispatcherTask)
  ONEVENT(BASIC_MSG_TO_CS_API     ,ANYCASE , CCSApiDispatcherTask::OnBasicMsgToCSApi)
PEND_MESSAGE_MAP(CCSApiDispatcherTask,CStateMachine);



////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////

extern "C" void CSApiDispatcherEntryPoint(void* appParam)
{
	CCSApiDispatcherTask * CSApidispatcherTask = new CCSApiDispatcherTask;
	CSApidispatcherTask->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CCSApiDispatcherTask::CCSApiDispatcherTask()
        :CDispatcherTask(FALSE)
{
	m_pProcess = (CCSApiProcess*)CCSApiProcess::GetProcess();
	m_CSApiMplMcmsProtocolTracer = new CCSApiMplMcmsProtocolTracer;
    m_Thread_Group = eTaskGroupRegular;
}


/////////////////////////////////////////////////////////////////////
CCSApiDispatcherTask::~CCSApiDispatcherTask()
{
	POBJDELETE(m_CSApiMplMcmsProtocolTracer);
}

///////////////////////////////////////////////////////////////////////////////
void CCSApiDispatcherTask::InitTask()
{

}

///////////////////////////////////////////////////////////////////////////////
void*  CCSApiDispatcherTask::GetMessageMap()
{
  return (void*)m_msgEntries;
}
///////////////////////////////////////////////////////////////////////////////
void  CCSApiDispatcherTask::OnBasicMsgToCSApi(CSegment* pMsg)
{
	CMplMcmsProtocol CSPrtcl;
	CSPrtcl.DeSerialize(*pMsg,CS_API_TYPE);
	CSPrtcl.UpdateCommonHeaderToMPL();
	CSPrtcl.AddPayload_len(CS_API_TYPE);

	COsQueue *TxtaskMailSlot = m_pProcess->GetTxQueue(CSPrtcl.getCentralSignalingHeaderCsId());
	if(NULL == TxtaskMailSlot)
	{
		string buff = "failed to get TxQueue, message canceled, opcode: ";
		buff += m_pProcess->GetOpcodeAsString(CSPrtcl.getCommonHeaderOpcode());
		TRACEINTO << ">>>>+++++<<<<CCSApiDispatcherTask::SendToCard -\n"
		<< buff<<"\n";
		//PASSERTMSG(1, "FAILED to find a TxQueue, message canceled, Is CS Module(EndPointsSim) running ?");
		return;
	}

	CSPrtcl.UpdateSendTimes();

	m_CSApiMplMcmsProtocolTracer->SetData(&CSPrtcl);
	m_CSApiMplMcmsProtocolTracer->TraceMplMcmsProtocol("MCMS -> CS",CS_API_TYPE);
	m_CSApiMplMcmsProtocolTracer->SetData(NULL);

	CSegment * pSeg = new CSegment;
	CSPrtcl.Serialize(*pSeg,CS_API_TYPE);

	CTaskApi api;
	api.CreateOnlyApi(*TxtaskMailSlot);
	STATUS status = api.SendMsg(pSeg,CS_API_MSG_TO_CS);
	api.DestroyOnlyApi();
	if(STATUS_OK != status)
	{
		PASSERTMSG(1, "FAILED to send message to TxQueue");
		return;
	}

	m_pProcess->IncrementCntMcmsToCsMfa();
}

///////////////////////////////////////////////////////////////////////////////
void CCSApiDispatcherTask::AddFilterOpcodePoint()
{
	AddFilterOpcodeToQueue(BASIC_MSG_TO_CS_API);
}


///////////////////////////////////////////////////////////////////////////////
