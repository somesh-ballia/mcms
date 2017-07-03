//+========================================================================+
//                  GKCall.cpp									  		   |
//					Copyright Polycom							           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       GKCall.cpp                                                  |
// SUBSYSTEM:  Processes/Gatekeeper/GatekeeperLib		                   |                       |
// PROGRAMMER: Yael A.	                                                   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |  Attributes and basix functionality of calls through |
//	   |			|													   |
//						 												   |
//						                                                   |
//+========================================================================+                        



#include "GKCall.h"
#include "GKManagerOpcodes.h"
#include "TraceStream.h"

const DWORD PARTY_KEEP_ALIVE_TIMER = 100001;
#define PARTY_KEEP_ALIVE_PERIOD 120

PBEGIN_MESSAGE_MAP(CGkCall)
	ONEVENT(PARTY_KEEP_ALIVE_TIMER,     IDLE,    CGkCall::OnTimerPartyKeepAlive)
PEND_MESSAGE_MAP(CGkCall, CStateMachine);


//////////////////////////////////////////////////////////////////////////////
void  CGkCall::HandleEvent(CSegment *pMsg, DWORD msgLen, OPCODE opCode)
{
    DispatchEvent(opCode, pMsg);
}

/////////////////////////////////////////////////////////////////////////////
void* CGkCall::GetMessageMap()                                        
{
	return m_msgEntries;    
}

/////////////////////////////////////////////////////////////////////////////
/*bool operator<(const CGkCall& lhs,const CGkCall& rhs) 
{
	if(lhs.GetConnId() < rhs.GetConnId())
    	return TRUE;
    else
    	return FALSE;
}*/


////////////////////////////////////////////////////////////////////////////
CGkCall::CGkCall(const COsQueue& gkManagerRcvMbx, DWORD mcmsConnId, DWORD partyId, DWORD serviceId, DWORD confId, char conferenceId[Size16], char callId[Size16],
				int	crv, BYTE bIsDialIn, eCallStatus callState)
{
	m_mcmsConnId    = mcmsConnId;
	m_partyId		= partyId;
	m_serviceId     = serviceId;
	m_ConfRsrcId 	= confId;
	
	memset(m_conferenceId, 0, Size16);
	if (conferenceId)
		memcpy(m_conferenceId, conferenceId, Size16);
	
	memset(m_callId, 0, Size16);
	if (callId)
		memcpy(m_callId, callId, Size16);
		
	m_crv		    = crv;
	m_bIsDialIn     = bIsDialIn;
	m_callState		= callState;
	
	InitErrorHandlingStatus();
	
	m_pGkManagerApi = new CGKManagerApi;
	m_pGkManagerApi->CreateOnlyApi(gkManagerRcvMbx);
	
	VALIDATEMESSAGEMAP
}


////////////////////////////////////////////////////////////////////////////
//This function should not be called (because it doestn't coppy the timer list)!!!
CGkCall::CGkCall(const CGkCall &other):CStateMachine(other)
{
	PTRACE2INT(eLevelInfoNormal,"CGkCall::CGkCall - Init from other - THIS IS WRONG!! - Conn id = ", other.m_mcmsConnId);
	m_mcmsConnId    = other.m_mcmsConnId;
	m_partyId		= other.m_partyId;
	m_serviceId     = other.m_serviceId;
	memcpy(m_conferenceId, other.m_conferenceId, Size16);
	memcpy(m_callId, other.m_callId, Size16);
	m_crv		    = other.m_crv;
	m_bIsDialIn     = other.m_bIsDialIn;
	m_callState		= other.m_callState;
	m_eErrorHandlingStatus = other.m_eErrorHandlingStatus;	
	
	m_pGkManagerApi = new CGKManagerApi;
	m_pGkManagerApi->CreateOnlyApi(other.m_pGkManagerApi->GetRcvMbx());
	
	VALIDATEMESSAGEMAP
}


///////////////////////////////////////////////////////////////////////////
CGkCall::~CGkCall()
{
	if (m_pGkManagerApi)
	{
		m_pGkManagerApi->DestroyOnlyApi();
    	POBJDELETE(m_pGkManagerApi);
  	}  
}

////////////////////////////////////////////////////////////////////////////
//Update after ARQ Req
void CGkCall::UpdateParams(char conferenceId[Size16], char callId[Size16], BYTE bIsDialIn, eCallStatus callState)
{
	memcpy(m_conferenceId, conferenceId, Size16);
	memcpy(m_callId, callId, Size16);
	m_bIsDialIn   = bIsDialIn;
	m_callState	  = callState;
}

///////////////////////////////////////////////////////////////////////////
void CGkCall::UpdateCallParamsAccordingToARQInd(int crv, char conferenceId[Size16], char callId[Size16])
{
	memcpy(m_conferenceId, conferenceId, Size16);
	memcpy(m_callId, callId, Size16);
	m_crv = crv;
}

///////////////////////////////////////////////////////////////////////////
void CGkCall::SetCallState(eCallStatus callState)
{
	m_callState	= callState;
}

///////////////////////////////////////////////////////////////////////////
eCallStatus CGkCall::GetCallState() const
{
	return m_callState;
}

///////////////////////////////////////////////////////////////////////////
DWORD CGkCall::GetServiceId() const
{
	return m_serviceId;
}

///////////////////////////////////////////////////////////////////////////
const char* CGkCall::GetCallId() const
{
	return m_callId;
}

///////////////////////////////////////////////////////////////////////////
const char* CGkCall::GetConferenceId() const
{
	return m_conferenceId;
}

///////////////////////////////////////////////////////////////////////////
int CGkCall::GetCrv() const
{
	return m_crv;
}

////////////////////////////////////////////////////////////////////////////
DWORD CGkCall::GetConnId() const
{
	return m_mcmsConnId;
}

////////////////////////////////////////////////////////////////////////////
DWORD CGkCall::GetPartyId() const
{
	return m_partyId;
}
////////////////////////////////////////////////////////////////////////////
DWORD CGkCall::GetConfRsrcId() const
{
	return m_ConfRsrcId;
}

///////////////////////////////////////////////////////////////////////////
BYTE CGkCall::GetIsDialIn() const
{
	return m_bIsDialIn;
}
///////////////////////////////////////////////////////////////////////////
void CGkCall::SetSeviceId(DWORD seviceId)
{
	m_serviceId = seviceId;
}
///////////////////////////////////////////////////////////////////////////
void CGkCall::SetConnId(DWORD connId)
{
	m_mcmsConnId = connId;
}    
///////////////////////////////////////////////////////////////////////////
void CGkCall::SetConfRsrcId(DWORD confId)
{
	m_ConfRsrcId = confId;
}

////////////////////////////////////////////////////////////////////////////
BYTE CGkCall::IsCallInHoldState() const
{
	BYTE bRes = (m_callState == eArqHold) || (m_callState == eDrqHold) || 
		        (m_callState == eDrqAfterArqHold) || (m_callState == eBrqHold);
	return bRes;
}

////////////////////////////////////////////////////////////////////////////
void CGkCall::Dump(std::ostream& msg) const
{
	msg << "conn id = "     << m_mcmsConnId;
	msg << "party id = "    << m_partyId;
	msg << "service id = "  << m_serviceId;
	msg << "conf id = "     << m_conferenceId;
	msg << "call id = " 	<< m_callId;
	msg << "crv = " 		<< m_crv;
	msg << "is dial in  = " << m_bIsDialIn;
//	msg << "call state  = " << GetRoleStr(m_callState);
	msg << "call state  = " << m_callState;
	msg << "\n";
}

///////////////////////////////////////////////////////////////////////////
///////////  			Error Handling Section 				///////////////
///////////////////////////////////////////////////////////////////////////

void CGkCall::InitErrorHandlingStatus()
{
	m_eErrorHandlingStatus = eNoSend;
}

////////////////////////////////////////////////////////////////////////////
//Send keep alive req to pary and start timer 
void CGkCall::StartPartyKeepAliveFlow()
{
	PTRACE2INT(eLevelInfoNormal,"CGkCall::StartPartyKeepAliveFlow - ConnId = ", m_mcmsConnId);
	SendReqToConfParty(GK_MANAGER_PARTY_KEEP_ALIVE_REQ, m_mcmsConnId, m_partyId);
	StartPartyKeepAliveTimer();
}

////////////////////////////////////////////////////////////////////////////
void CGkCall::StartPartyKeepAliveTimer()
{
	m_eErrorHandlingStatus++;
	StartTimer(PARTY_KEEP_ALIVE_TIMER, PARTY_KEEP_ALIVE_PERIOD * SECOND);
}

////////////////////////////////////////////////////////////////////////////
//When party doesn't answer: 
void CGkCall::OnTimerPartyKeepAlive(CSegment *pMsg)
{
	if (m_eErrorHandlingStatus == eWaitToSendFirstTime)
	{
		PTRACE2INT(eLevelInfoNormal,"CGkCall::OnTimerPartyKeepAlive - Party status is correct - ConnId = ", m_mcmsConnId);	
		StartPartyKeepAliveFlow();
	}
	else if (m_eErrorHandlingStatus == eSendFirstTime)
	{
		PTRACE2INT(eLevelInfoNormal,"CGkCall::OnTimerPartyKeepAlive - Party hasn't responsed once - ConnId = ", m_mcmsConnId);	
		StartPartyKeepAliveFlow();
	}
	else if (m_eErrorHandlingStatus == eSendSecondTime)
	{
		PTRACE2INT(eLevelError,"CGkCall::OnTimerPartyKeepAlive - Party hasn't responsed twice - ConnId = ", m_mcmsConnId);	
		if (m_callState == eArqInd)
			m_pGkManagerApi->SendNoResponseFromPartyToGkManager(m_mcmsConnId);
		else
			TRACEINTO << "CGkCall::OnTimerPartyKeepAlive - Party hasn't responsed twice, but state isn't ArqInd - ConnId = " << m_mcmsConnId << " , State = " << m_callState;
	}
	else
		PTRACE2INT(eLevelError,"CGkCall::OnTimerPartyKeepAlive - Unknown status - ConnId = ", m_mcmsConnId);	
}

////////////////////////////////////////////////////////////////////////////
//When party answer: 
// 1- Set counter of no answer to 0
// 2 - Restart the timer.
void CGkCall::OnPartyAnswerToKeepAlive()
{
	PTRACE2INT(eLevelInfoNormal,"CGkCall::OnPartyAnswerToKeepAlive - ConnId = ", m_mcmsConnId);
	InitErrorHandlingStatus();
	DeleteTimer(PARTY_KEEP_ALIVE_TIMER);
	StartPartyKeepAliveTimer();
}

////////////////////////////////////////////////////////////////////////////



