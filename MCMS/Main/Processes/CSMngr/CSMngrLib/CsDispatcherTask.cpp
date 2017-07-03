// CsDispatcherTask.cpp: implementation of the CCsDispatcherTask class.
//
//////////////////////////////////////////////////////////////////////


#include "CsDispatcherTask.h"
#include "TraceStream.h"
#include "MplMcmsProtocol.h"
#include "MplMcmsProtocolTracer.h"
#include "CSMngrProcess.h"
#include "Segment.h"
#include "SignalingApi.h"
#include "OpcodesRanges.h"

////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////

extern "C" void CsDispatcherEntryPoint(void* appParam)
{  
	CCsDispatcherTask * csDispatcherTask = new CCsDispatcherTask;
	csDispatcherTask->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CCsDispatcherTask::CCsDispatcherTask()
        :CDispatcherTask(FALSE)
{
	m_pProcess = (CCSMngrProcess*)CCSMngrProcess::GetProcess();
	
	for (int i=0; i<MAX_NUMBER_OF_SERVICES_IN_RMX_4000; i++)
		m_isAssertForThisSignalingTask[i] = NO;
	
	m_Thread_Group = eTaskGroupRegular;

}
/////////////////////////////////////////////////////////////////////
CCsDispatcherTask::~CCsDispatcherTask()
{

}

/////////////////////////////////////////////////////////////////////////////
//ALL SPECIFIC HANDLING OF EVENTS IN CSMNGR PROCESS SHOULD BE ADDED IN THIS FUNCTION
BOOL CCsDispatcherTask::TaskHandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode)
{               
	void* ptr = NULL;        
	switch ( opCode ) 
	{						   
	case CSAPI_MSG:  
		{
			HandleSignalingEvent(pMsg);
			break;
		}
	default:
		{
			// was put in comment due to leak problems
			// TRACESTR(eLevelInfoNormal) << "CCsDispatcherTask::HandleEvent : invalid opcode";
			return FALSE;
			break;
		}
	}
	return TRUE;
  
}              

/////////////////////////////////////////////////////////////////////////////
void CCsDispatcherTask::HandleSignalingEvent(CSegment *pSeg)
{
	STATUS res = STATUS_OK;
	
	CMplMcmsProtocol* pProtocol = new CMplMcmsProtocol;
	pProtocol->DeSerialize(*pSeg, CS_API_TYPE);
	CMplMcmsProtocolTracer(*pProtocol).TraceMplMcmsProtocol("CS_DISPATCHER_RECEIVED_FROM_CS");

	//extract info from segment
	OPCODE opcode  = pProtocol->getOpcode();
	WORD   msgCsId = pProtocol->getCentralSignalingHeaderCsId();
	msgCsId        = CCSMngrProcess::CheckAndFixCSID(msgCsId, opcode);

	// send the segment to the relevant Signaling
	// ===== 1. get Signaling's Mbx
	CCSMngrProcess* pProcess = (CCSMngrProcess*)CCSMngrProcess::GetProcess();
	COsQueue* queue = pProcess->GetSignalingMbx(msgCsId - 1);

	if (!queue)
	{
		// ASSERT + log (only once for each absent queue)
		if ( NO == GetIsAssertForThisSignalingTask(msgCsId - 1) )
		{
			PASSERT(1);
			
			string opcdStr = m_pProcess->GetOpcodeAsString(opcode);
    		TRACESTR(eLevelInfoNormal)
    			<< "\nCCsDispatcherTask::HandleSignalingEvent. Opcode: " << opcdStr.c_str()
				<< "\nNo Mbx for csId: " << (WORD)msgCsId;
				
			SetIsAssertForThisSignalingTask(YES, msgCsId - 1);
		}

		// send a Reject to MplAPi
		SendSignalingNotCreatedToCsApi(msgCsId);
		
	}
	
	else // queue ok
	{
		SetIsAssertForThisSignalingTask(NO, msgCsId - 1); // if connection will fail next time, an ASSERT will be produced
		
		if ( eSignalingTaskZombie != pProcess->GetSignalingTaskState(msgCsId - 1) )
		{
			// ===== 2. prepare the data to be sent
			CSegment* pSegToCs = new CSegment;
			if( pProtocol->getDataLen() )
			{
				pSegToCs->Put( (unsigned char*)pProtocol->GetData(), pProtocol->getDataLen() );
			}
		
			// ===== 3. send the message
			CSignalingApi signalingApi;
			signalingApi.CreateOnlyApi(*queue);
			res = signalingApi.SendMsg(pSegToCs, opcode);
		} // end if (task != zombie)
		
		else
		{
	    	TRACESTR(eLevelInfoNormal) << "\nCCsDispatcherTask::HandleSignalingEvent. Task is zombie!";
		}
	} // end else (queue ok)

	delete pProtocol;
}

/////////////////////////////////////////////////////////////////////////////
void CCsDispatcherTask::SendSignalingNotCreatedToCsApi(BYTE csId)
{
	TRACESTR(eLevelInfoNormal) << "CCsDispatcherTask::SendSignalingNotCreatedToCsApi";

	CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol();

	mplPrtcl->AddCommonHeader(SIGNALING_TASK_NOT_CREATED_REQ, 0, 0, 0, eCentral_signaling);
	mplPrtcl->AddMessageDescriptionHeader();
	mplPrtcl->AddCSHeader(csId, 0, eServiceMngr);

	CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("CS_SENDS_TO_MPL");
	mplPrtcl->SendMsgToCSApiCommandDispatcher();
	
	POBJDELETE(mplPrtcl);
}

//////////////////////////////////////////////////////////////////////////
void CCsDispatcherTask::SetIsAssertForThisSignalingTask(BOOL isAssert, WORD index)
{
	if ( m_pProcess->GetMaxNumOfCSTasks() <= index )
	{
		char buff[TEN_LINE_BUFFER_LEN];
		sprintf(buff, "CCsDispatcherTask::SetIsAssertForThisSignalingTask - Illegal: csId: %d", index + 1);
		PASSERTMSG(1, buff);
	}

	m_isAssertForThisSignalingTask[index] = isAssert;
}

//////////////////////////////////////////////////////////////////////////
BOOL CCsDispatcherTask::GetIsAssertForThisSignalingTask(WORD index)
{
	if ( m_pProcess->GetMaxNumOfCSTasks() <= index )
	{
		char buff[TEN_LINE_BUFFER_LEN];
		sprintf(buff, "CCsDispatcherTask::SetIsAssertForThisSignalingTask - Illegal: csId: %d", index + 1);
		PASSERTMSG(1, buff);
	}

	return m_isAssertForThisSignalingTask[index];
}

