//+========================================================================+
//                         CSApiRxSocketMngr.cpp                           |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       CSApiRxSocketMngr.cpp                                       |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Shlomit                                                     |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//+========================================================================+
#include "CSApiRxSocketMngr.h"
#include "Segment.h"
#include "ProcessBase.h"
#include "TaskApi.h"
#include "OpcodesRanges.h"
#include "OpcodesMcmsCommon.h"
#include "IpCsOpcodes.h"
#include "IpMngrOpcodes.h"
#include "CSApiMplMcmsProtocolTracer.h"
#include "CSApiProcess.h"
#include "TraceStream.h"
#include "IpCsOpcodes.h"


static CCSApiProcess *pCSApiProcess = NULL;

/////////////////////////////////////////////////////////////////////////////
CCSApiRxSocketMngr::CCSApiRxSocketMngr() // constructor
{
	pCSApiProcess = (CCSApiProcess*)CCSApiProcess::GetProcess();
	m_CSApiMplMcmsProtocolTracer = new CCSApiMplMcmsProtocolTracer;
}
/////////////////////////////////////////////////////////////////////////////
CCSApiRxSocketMngr::~CCSApiRxSocketMngr() // constructor
{
	POBJDELETE(m_CSApiMplMcmsProtocolTracer);
}
/////////////////////////////////////////////////////////////////////////////
const char * CCSApiRxSocketMngr::GetTaskName() const 
{
	return "CCSApiRxSocketMngr";
}
/////////////////////////////////////////////////////////////////////////////
DWORD CCSApiRxSocketMngr::SendCSEventToTheAppropriateProcess(CMplMcmsProtocol& CSPrtcl)
{
    OPCODE opcode = CSPrtcl.getOpcode();
	const COsQueue * DestinationProcessQueue = GetDestinationProcessQueueByOpcode(opcode);
	
	if(NULL == DestinationProcessQueue)
	{
        CMplMcmsProtocolTracer(CSPrtcl).TraceMplMcmsProtocol("this message from CS was dropped");
        string buff = "Destination process was not found by opcode; the opcode : ";
        buff += pCSApiProcess->GetOpcodeAsString(opcode);
        PASSERTMSG(TRUE, buff.c_str());
        
		return 0xFFFFFFFF;
	}
	
	CSPrtcl.UpdateSendTimes();
	
	m_CSApiMplMcmsProtocolTracer->SetData(&CSPrtcl);
	m_CSApiMplMcmsProtocolTracer->TraceMplMcmsProtocol("CS -> MCMS", CS_API_TYPE);
	m_CSApiMplMcmsProtocolTracer->SetData(NULL);
	
	CSegment * pSeg = new CSegment;
	CSPrtcl.Serialize(*pSeg,CS_API_TYPE);
	DWORD len = pSeg->GetLen();	
	
	CTaskApi api;
	api.CreateOnlyApi(*DestinationProcessQueue);
	STATUS status = api.SendMsg(pSeg,CSAPI_MSG);
	api.DestroyOnlyApi();
	
	if(STATUS_OK == status)
		pCSApiProcess->IncrementCntCsMfaToMcms();
	else
		FTRACESTR(eLevelInfoNormal) << "CCSApiRxSocketMngr::SendCSEventToTheAppropriateProcess - Failed send CS message. Destination opcode = "<<opcode<<" status = "<< status;
	
	return len;
}
////////////////////////////////////////////////////////////////////////////////////
const COsQueue * CCSApiRxSocketMngr::GetDestinationProcessQueueByOpcode (DWORD opcode)
{
	static struct{
		DWORD start;
		DWORD end;
		eProcessType process;
		eOtherProcessQueueEntry task;
	} RangeTable[] = {
	{CS_TO_PROXY_FIRST_OPCODE_IN_RANGE, CS_TO_PROXY_LAST_OPCODE_IN_RANGE, eProcessSipProxy, eDispatcher},
	{CS_TO_PARTY_FIRST_OPCODE_IN_RANGE, CS_TO_PARTY_LAST_OPCODE_IN_RANGE, eProcessConfParty, eDispatcher},
	{CS_MNGR_DISPATCHER_FIRST_OPCODE_IN_RANGE, CS_MNGR_DISPATCHER_LAST_OPCODE_IN_RANGE, eProcessCSMngr, eDispatcher},
	{CS_TO_GATE_KEEPER_MNGR_FIRST_OPCODE_IN_RANGE, CS_TO_GATE_KEEPER_MNGR_LAST_OPCODE_IN_RANGE, eProcessGatekeeper, eDispatcher},
	{CS_TO_DNSAGENT_FIRST_OPCODE_IN_RANGE, CS_TO_DNSAGENT_LAST_OPCODE_IN_RANGE, eProcessDNSAgent, eManager},
	{CS_TO_MCCFMNG_FIRST_OPCODE_IN_RANGE, CS_TO_MCCFMNG_LAST_OPCODE_IN_RANGE, eProcessMCCFMngr, eManager}, //_mccf_

/// TBD cs to Signaling agents
/// TBD cs to Signaling logger?
//  ...
    {0,0,eProcessTypeInvalid,eDispatcher}}; // leave this line for termination detection
	
	eProcessType process = eProcessTypeInvalid;
    eOtherProcessQueueEntry task = eDispatcher;
    
	int i = 0;
	while (RangeTable[i].process != eProcessTypeInvalid)
	{
		if (RangeTable[i].start < opcode && opcode < RangeTable[i].end)
		{
			process = RangeTable[i].process;
			
			if (opcode == CS_NEW_IND)		//CS_NEW_IND is the only message that go to the manager from the CS. All the other messages go to specific CS Task. 
				task = eManager;
			else
				task = RangeTable[i].task;
				
		}
		i++;
	}
	const COsQueue * res = NULL;
	if (process != eProcessTypeInvalid)
	{
		res = pCSApiProcess->GetOtherProcessQueue(process,task);
	}
	return res;
}
////////////////////////////////////////////////////////////////////////////////////
