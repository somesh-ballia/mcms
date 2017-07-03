#include "ContentToken.h"
#include "Party.h"

extern char* CascadeModeToString(BYTE cascadeMode);

//China-withdraw Slave
#include "Conf.h"
#include "ContentBridge.h"

////////////////////////////////////////////////////////////////////////////
//                        CContentToken
////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CContentToken)

	ONEVENT(ACQUIRE_CONTENT_TOKEN ,             NOTHELD ,   CContentToken::OnContentBrdgAcquireTokenNOTHELD)
	ONEVENT(ACQUIRE_CONTENT_TOKEN ,             HELD ,      CContentToken::OnContentBrdgAcquireTokenHELD)
	ONEVENT(ACQUIRE_CONTENT_TOKEN ,             WITHDRAW ,  CContentToken::OnContentBrdgAcquireTokenWithdraw)

	ONEVENT(RELEASE_CONTENT_TOKEN ,             NOTHELD ,   CContentToken::OnContentBrdgReleaseTokenNOTHELD)
	ONEVENT(RELEASE_CONTENT_TOKEN ,             HELD ,      CContentToken::OnContentBrdgReleaseTokenHELD)
	ONEVENT(RELEASE_CONTENT_TOKEN ,             WITHDRAW ,  CContentToken::OnContentBrdgReleaseTokenWithdraw)

	ONEVENT(WITHDRAW_CONTENT_TOKEN ,            NOTHELD ,   CContentToken::NullActionFunction)
	ONEVENT(WITHDRAW_CONTENT_TOKEN ,            HELD ,      CContentToken::OnContentBrdgWithdrawTokenHELD)
	ONEVENT(WITHDRAW_CONTENT_TOKEN ,            WITHDRAW ,  CContentToken::OnContentBrdgWithdrawTokenWithdraw)

	ONEVENT(WITHDRAW_ACK ,                      NOTHELD ,   CContentToken::NullActionFunction)
	ONEVENT(WITHDRAW_ACK ,                      HELD ,      CContentToken::NullActionFunction)
	ONEVENT(WITHDRAW_ACK ,                      WITHDRAW ,  CContentToken::OnContentBrdgWithdrawAckWithdraw)

	ONEVENT(DROP_CONTENT_TOKEN_HOLDER ,         NOTHELD ,   CContentToken::NullActionFunction)
	ONEVENT(DROP_CONTENT_TOKEN_HOLDER ,         HELD ,      CContentToken::OnContentBrdgDropTokenHolderHELD)
	ONEVENT(DROP_CONTENT_TOKEN_HOLDER ,         WITHDRAW ,  CContentToken::OnContentBrdgDropTokenHolderWithdraw)

	ONEVENT(DROP_LAST_CONTENT_TOKEN_REQUESTER , NOTHELD ,   CContentToken::NullActionFunction)
	ONEVENT(DROP_LAST_CONTENT_TOKEN_REQUESTER , HELD ,      CContentToken::OnContentBrdgDropLastContentTokenRequesterHELD)
	ONEVENT(DROP_LAST_CONTENT_TOKEN_REQUESTER , WITHDRAW ,  CContentToken::OnContentBrdgDropLastContentTokenRequesterWithdraw)

	ONEVENT(PARTY_DISCONNECTS ,                 NOTHELD ,   CContentToken::NullActionFunction)
	ONEVENT(PARTY_DISCONNECTS ,                 HELD ,      CContentToken::OnContentBrdgPartyDisconnectHELD)
	ONEVENT(PARTY_DISCONNECTS ,                 WITHDRAW ,  CContentToken::OnContentBrdgPartyDisconnectWithdraw)

	ONEVENT(WITHDRAW_CONTENT_TOKEN_TOUT ,       NOTHELD ,   CContentToken::OnTimerWithdrawReTransmitNOTHELD)
	ONEVENT(WITHDRAW_CONTENT_TOKEN_TOUT ,       HELD ,      CContentToken::OnTimerWithdrawReTransmitHELD)
	ONEVENT(WITHDRAW_CONTENT_TOKEN_TOUT ,       WITHDRAW ,  CContentToken::OnTimerWithdrawReTransmitWithdraw)

	ONEVENT(NO_ROLE_PROVIDER_TOUT ,             HELD ,      CContentToken::NullActionFunction)
	ONEVENT(NO_ROLE_PROVIDER_TOUT ,             NOTHELD ,   CContentToken::OnTimerNoRoleProviderNOTHELD)
	ONEVENT(NO_ROLE_PROVIDER_TOUT ,             WITHDRAW ,  CContentToken::NullActionFunction)

PEND_MESSAGE_MAP(CContentToken,CStateMachine);

//--------------------------------------------------------------------------
CContentToken::CContentToken()
{
	m_pConfApi  = NULL;
	m_roleLabel = LABEL_CONTENT;
	m_currentTokenHolder.SetToNone();
	m_lastTokenRequester.SetToNone();

	VALIDATEMESSAGEMAP
}

//--------------------------------------------------------------------------
void* CContentToken::GetMessageMap()
{
	return (void*)m_msgEntries;
}

//--------------------------------------------------------------------------
CContentToken::~CContentToken(void)
{
	DeleteAllTimers();
	if (m_pConfApi)
	{
		m_pConfApi->DestroyOnlyApi();
		POBJDELETE(m_pConfApi);
	}
}

//--------------------------------------------------------------------------
void CContentToken::HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode)
{
	DispatchEvent(opCode, pMsg);
}

//--------------------------------------------------------------------------
void CContentToken::InitToken(COsQueue& rcvMbx, const CTaskApi* pConfApi)
{
	if (!CPObject::IsValidPObjectPtr((CPObject*)pConfApi)) {
		DBGPASSERT_AND_RETURN(1);
	}

	m_pConfApi = new CConfApi(*(CConfApi*)pConfApi);
	m_state    = NOTHELD;
	StartTimer(NO_ROLE_PROVIDER_TOUT, NO_ROLE_PROVIDER_TIME);
}

//--------------------------------------------------------------------------
CContentToken& CContentToken::operator=(const CContentToken& rContentToken)
{
	if (&rContentToken == this)
		return *this;

	m_roleLabel          = rContentToken.m_roleLabel;
	m_currentTokenHolder = rContentToken.m_currentTokenHolder;
	m_lastTokenRequester = rContentToken.m_lastTokenRequester;
	m_state              = rContentToken.m_state;

	if (m_pConfApi)
		POBJDELETE(m_pConfApi);

	m_pConfApi = new CConfApi(*(CConfApi*)rContentToken.m_pConfApi);

	return *this;
}

//--------------------------------------------------------------------------
const CTaskApp* CContentToken::GetTokenHolderParty(void)
{
	PartyRsrcID partyId = m_currentTokenHolder.GetPartyId();
	if (partyId)
	{
		CTaskApp* pParty = (CTaskApp*)m_currentTokenHolder.GetParty();
		if (pParty)
		{
			if (pParty->GetRcvMbx().IsValid())
				return (const CTaskApp*)pParty;
		}
	}
	return NULL;
}

//--------------------------------------------------------------------------
const CTaskApp* CContentToken::GetLastTokenRequesterParty(void)
{
	PartyRsrcID partyId = m_lastTokenRequester.GetPartyId();
	if (partyId)
		return m_lastTokenRequester.GetParty();
	return NULL;
}

//--------------------------------------------------------------------------
BYTE CContentToken::IsLinkToSlave(PartyRsrcID partyId)
{
	CParty* pParty = (CParty*)GetLookupTableParty()->Get(partyId);
	TRACECOND_AND_RETURN_VALUE(!pParty, "PartyId:" << partyId << " - Failed, Party does not exist", NO);

	BYTE IsLinkToSlave = (pParty->IsCascadeToPolycomBridge() && pParty->GetCascadeMode() == CASCADE_MODE_MASTER) ? YES : NO;

	std::ostringstream msg;
	msg << "CContentToken::IsLinkToSlave "
	    << "- PartyId:"                  << pParty->GetPartyRsrcID()
	    << ", IsCascadeToPolycomBridge:" << (int)pParty->IsCascadeToPolycomBridge()
	    << ", CascadeMode:"              << CascadeModeToString(pParty->GetCascadeMode())
	    << ", RC:"                       << (int)IsLinkToSlave;

	PTRACE(eLevelInfoNormal, msg.str().c_str());
	return IsLinkToSlave;
}

//--------------------------------------------------------------------------
BYTE CContentToken::IsLinkToMaster(PartyRsrcID partyId)
{
	CParty* pParty = (CParty*)GetLookupTableParty()->Get(partyId);
	TRACECOND_AND_RETURN_VALUE(!pParty, "PartyId:" << partyId << " - Failed, Party does not exist", NO);

	BYTE IsLinkToMaster = (pParty->IsCascadeToPolycomBridge() && pParty->GetCascadeMode() == CASCADE_MODE_SLAVE) ? YES : NO;

	std::ostringstream msg;
	msg << "CContentToken::IsLinkToMaster "
			<< "- PartyId:"                  << pParty->GetPartyRsrcID()
	    << ", IsCascadeToPolycomBridge:" << (int)pParty->IsCascadeToPolycomBridge()
	    << ", CascadeMode:"              << CascadeModeToString(pParty->GetCascadeMode())
	    << ", RC:"                       << (int)IsLinkToMaster;

	PTRACE(eLevelInfoNormal, msg.str().c_str());
	return IsLinkToMaster;
}

//--------------------------------------------------------------------------
void CContentToken::Acquire(PartyRsrcID partyId, BYTE mcuNum, BYTE terminalNum, BYTE randomNum)
{
	CSegment* pSeg = new CSegment;
	*pSeg << partyId << mcuNum << terminalNum << randomNum;

	DispatchEvent(ACQUIRE_CONTENT_TOKEN, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CContentToken::Release(PartyRsrcID partyId, BYTE mcuNum, BYTE terminalNum)
{
#ifdef CONTENT_LOAD
	TRACESTRFUNC(eLevelError) << "CONTENT_LOAD McuNum:" << (int)mcuNum << ", TerminalNum:" << (int)terminalNum;
#else
	TRACEINTO << "PartyId:" << partyId << ", McuNum:" << (int)mcuNum << ", TerminalNum:" << (int)terminalNum;
#endif

	CSegment* pSeg = new CSegment;
	*pSeg << partyId << mcuNum << terminalNum;

	DispatchEvent(RELEASE_CONTENT_TOKEN, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CContentToken::Withdraw(PartyRsrcID partyId)
{
	TRACEINTO << "PartyId:" << partyId;

	CSegment* pSeg = new CSegment;
	*pSeg << partyId;

	DispatchEvent(WITHDRAW_CONTENT_TOKEN, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CContentToken::WithdrawAck(PartyRsrcID partyId)
{
	TRACEINTO << "PartyId:" << partyId;

	CSegment* pSeg = new CSegment;
	*pSeg << partyId;

	DispatchEvent(WITHDRAW_ACK, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CContentToken::BadAcquire(PartyRsrcID partyId, BYTE mcuNum, BYTE terminalNum)
{
	TRACEINTO << "PartyId:" << partyId << ", McuNum:" << (int)mcuNum << ", TerminalNum:" << (int)terminalNum;

	// Bad Acquire can be received only for out-side party (which is not current token holder or token requester)
	PASSERT_AND_RETURN(partyId == m_currentTokenHolder.GetPartyId());
	PASSERT_AND_RETURN(partyId == m_lastTokenRequester.GetPartyId());

	// Bad Acquire should only send AcquireNak to the party (there must be answer for each party token request)
	AcquireNak(partyId, mcuNum, terminalNum);
}

//--------------------------------------------------------------------------
void CContentToken::BadAcquireBridgeDisconnecting(PartyRsrcID partyId, BYTE mcuNum, BYTE terminalNum)
{
	TRACEINTO << "PartyId:" << partyId << ", McuNum:" << (int)mcuNum << ", TerminalNum:" << (int)terminalNum;

	AcquireNak(partyId, mcuNum, terminalNum);
}

//--------------------------------------------------------------------------
void CContentToken::DropTokenHolder(PartyRsrcID partyId, BYTE isImmediate)
{
	TRACEINTO << "PartyId:" << partyId << ", IsImmediate:" << (int)isImmediate;

	CSegment* pSeg = new CSegment;
	*pSeg << partyId << isImmediate;

	DispatchEvent(DROP_CONTENT_TOKEN_HOLDER, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CContentToken::DropLastContentTokenRequester()
{
	DispatchEvent(DROP_LAST_CONTENT_TOKEN_REQUESTER, NULL);
}

//--------------------------------------------------------------------------
void CContentToken::PartyDisconnects(PartyRsrcID partyId)
{
	TRACEINTO << "PartyId:" << partyId;

	CSegment* pSeg = new CSegment;
	*pSeg << partyId;

	DispatchEvent(PARTY_DISCONNECTS, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CContentToken::OnContentBrdgAcquireTokenNOTHELD(CSegment* pPartyParams)
{
	PartyRsrcID partyId;
	BYTE mcuNum;
	BYTE terminalNum;
	BYTE randomNum;

	*pPartyParams >> partyId >> mcuNum >> terminalNum >> randomNum;

	TRACEINTO << "PartyId:" << partyId << ", McuNum:" << (int)mcuNum << ", TerminalNum:" << (int)terminalNum << ", RandomNum:" << (int)randomNum;

	m_currentTokenHolder.Set(partyId, mcuNum, terminalNum, randomNum);
	m_lastTokenRequester.SetToNone(); // no additional requester for token
	DumpToken(__FUNCTION__);
	AcquireAck(partyId, mcuNum, terminalNum);
	NewTokenHolder(mcuNum, terminalNum);
}

//--------------------------------------------------------------------------
void CContentToken::OnContentBrdgReleaseTokenNOTHELD(CSegment* pParam)
{
	PartyRsrcID partyId;
	BYTE mcuNum;
	BYTE terminalNum;

	*pParam >> partyId >> mcuNum >> terminalNum;

	ReleaseAck(partyId, mcuNum, terminalNum);
}

//--------------------------------------------------------------------------
void CContentToken::OnTimerNoRoleProviderNOTHELD(CSegment* pParam)
{
	// ask ContentBridge for broadcast NS-IND/NoRoleProvider to all parties - content members
	BroadcastNoRoleProvider();
	StartTimer(NO_ROLE_PROVIDER_TOUT, NO_ROLE_PROVIDER_TIME);
}

//--------------------------------------------------------------------------
void CContentToken::OnContentBrdgAcquireTokenHELD(CSegment* pParam)
{
	PartyRsrcID partyId;
	BYTE mcuNum;
	BYTE terminalNum;
	BYTE randomNum;

	*pParam >> partyId >> mcuNum >> terminalNum >> randomNum;

	TRACEINTO << "PartyId:" << partyId << ", McuNum:" << (int)mcuNum << ", TerminalNum:" << (int)terminalNum << ", RandomNum:" << (int)randomNum;

	BYTE speakerMcuNumber             = m_currentTokenHolder.GetMcuNumber();
	BYTE speakerTerminalNumber        = m_currentTokenHolder.GetTerminalNumber();
	BYTE isRemoteIsSlavePolycomBridge = IsLinkToSlave(partyId);

	// Acquire request is received from the party which is currently holds the token
	if (partyId == m_currentTokenHolder.GetPartyId() && (mcuNum == speakerMcuNumber) && (terminalNum == speakerTerminalNumber) && !isRemoteIsSlavePolycomBridge) // Just to be sure, update m_currentTokenHolder with values from Acquire message
	{
		m_currentTokenHolder.Set(partyId, mcuNum, terminalNum, randomNum);
		AcquireAck(partyId, mcuNum, terminalNum);
	}
	else // Acquire request is received from the new party, set pending requester params
	{
		m_lastTokenRequester.Set(partyId, mcuNum, terminalNum, randomNum);

		// we need to send Freeze content to all content parties, before we send Stram OFF and Stream ON to content channel.
		SendFreezeContent();
		WithdrawRequest(m_currentTokenHolder.GetPartyId());
	}

	DumpToken(__FUNCTION__);
}

//--------------------------------------------------------------------------
void CContentToken::OnContentBrdgReleaseTokenHELD(CSegment* pParam)
{
	PartyRsrcID partyId;
	BYTE mcuNum;
	BYTE terminalNum;

	*pParam >> partyId >> mcuNum >> terminalNum;

	TRACEINTO << "PartyId:" << partyId << ", McuNum:" << (int)mcuNum << ", TerminalNum:" << (int)terminalNum;

	ReleaseAck(partyId, mcuNum, terminalNum);

	// If Release request was received from current token holder
	if (partyId == m_currentTokenHolder.GetPartyId())
	{
		m_currentTokenHolder.SetToNone();
		m_state = NOTHELD;
		StartTimer(NO_ROLE_PROVIDER_TOUT, NO_ROLE_PROVIDER_TIME_AFTER_WITHDRAW);
		NoTokenHolder();
	}
	else
	{
		// Do nothing if Release request was received from party which is not current token holder
	}

	DumpToken(__FUNCTION__);
}

//--------------------------------------------------------------------------
void CContentToken::OnContentBrdgWithdrawTokenHELD(CSegment* pParam)
{
	PartyRsrcID partyId;
	*pParam >> partyId;

	WithdrawRequest(partyId);
}

//--------------------------------------------------------------------------
void CContentToken::OnContentBrdgRoleProviderInfoHELD(CSegment* pParam)
{
}

//--------------------------------------------------------------------------
// this event shell be send when the Abort Presentation in action
void CContentToken::OnContentBrdgDropTokenHolderHELD(CSegment* pParam)
{
	PartyRsrcID partyId;
	BYTE isImmediate;

	*pParam >> partyId >> isImmediate;

	// Withdraw_Ack can be received only from current token holder
	if (partyId != m_currentTokenHolder.GetPartyId())
		return;

	WithdrawRequest(partyId);
	WithdrawAckHolderAction(isImmediate);
}

//--------------------------------------------------------------------------
void CContentToken::OnContentBrdgDropTokenHolderWithdraw(CSegment* pParam)
{
	PartyRsrcID partyId;
	BYTE isImmediate;

	*pParam >> partyId >> isImmediate;

	// Withdraw_Ack can be received only from current token holder
	if (partyId != m_currentTokenHolder.GetPartyId())
		return;

	WithdrawAckHolderAction(isImmediate);
}

//--------------------------------------------------------------------------
void CContentToken::OnContentBrdgDropLastContentTokenRequesterHELD(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_lastTokenRequester.GetPartyId() << " - Reset to NONE";

	m_lastTokenRequester.SetToNone();
}

//--------------------------------------------------------------------------
void CContentToken::OnContentBrdgDropLastContentTokenRequesterWithdraw(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_lastTokenRequester.GetPartyId() << " - Reset to NONE";

	m_lastTokenRequester.SetToNone();
}

//--------------------------------------------------------------------------
// this function called when the tokenHolder party is disconnecting (since the state is HELD)
void CContentToken::OnContentBrdgPartyDisconnectHELD(CSegment* pParam)
{
	PartyRsrcID partyId;
	*pParam >> partyId;

	TRACEINTO << "PartyId:" << partyId << ", CurrentTokenHolderPartyId:" << m_currentTokenHolder.GetPartyId();

	// if disconnecting party is not TokenHolder - return
	if (partyId != m_currentTokenHolder.GetPartyId())
		return;

	// disconnecting party is TokenHolder, so send withdraw and don't wait for ACK
	WithdrawRequest(partyId, FALSE);
	WithdrawAckHolderAction();
}

//--------------------------------------------------------------------------
void CContentToken::OnContentBrdgAcquireTokenWithdraw(CSegment* pParam)
{
	PartyRsrcID partyId;
	BYTE mcuNum;
	BYTE terminalNum;
	BYTE randomNum;

	*pParam >> partyId >> mcuNum >> terminalNum >> randomNum;

	TRACEINTO << "PartyId:" << partyId << ", McuNum:" << (int)mcuNum << ", TerminalNum:" << (int)terminalNum << ", RandomNum:" << (int)randomNum;

	// Do nothing if Acquire request is received from the party which is already
	// waiting for token to be acquired. It means that we are waiting for Wihdraw_Ack
	// from the current token holder.
	TRACECOND_AND_RETURN(partyId == m_lastTokenRequester.GetPartyId(), "Acquire request is received from the party which is already waiting for token to be acquired");

	// Do nothing if Acquire request is received from the party which is currently
	// holds the token. In this case we should continue to wait for Wihdraw_Ack
	// from the current token holder (m_lastTokenRequester and m_currentTokenHolder cannot
	// be the same party!!!).
	TRACECOND_AND_RETURN(partyId == m_currentTokenHolder.GetPartyId(), "Acquire request is received from the party which is currently holds the token");

	BYTE lastRequesterMcuNumber      = m_lastTokenRequester.GetMcuNumber();
	BYTE lastRequesterTerminalNumber = m_lastTokenRequester.GetTerminalNumber();

	// If Acquire request is received from the new party (which is not m_currentTokenHolder and
	// is not m_lastTokenRequester) we should do following steps:
	// 1. send Acquire_Nak to m_lastTokenRequester;
	// 2. change m_lastTokenRequester for the new value
	// 3. continue to wait for Wihdraw_Ack from the current token holder (stay in state WITHDRAW)
	if (!(!m_lastTokenRequester)) // if exists - see desc below
		AcquireNak(m_lastTokenRequester.GetPartyId(), lastRequesterMcuNumber, lastRequesterTerminalNumber);

	// Vasily - 10.07.2003
	// Scenario: P&C + 3 parties, A - speaker
	// If B asks for Token and disconnects, A didn't answer with WITHDRAW_ACK some time,
	// till C ask for Token  =>  m_lastTokenRequester.GetParty()==NULL
	m_lastTokenRequester.Set(partyId, mcuNum, terminalNum, randomNum);
}

//--------------------------------------------------------------------------
void CContentToken::OnContentBrdgReleaseTokenWithdraw(CSegment* pParam)
{
	PartyRsrcID partyId;
	BYTE mcuNumber;
	BYTE terminalnumber;

	*pParam >> partyId >> mcuNumber >> terminalnumber;

	// If Release request is received from the party which is waiting for
	// token to be acquired, set m_lastTokenRequester to None.
	// It means that we continue to wait for Wihdraw_Ack from the current token holder
	// and when it will be received we will set token state to NOTHELD.
	if (partyId == m_lastTokenRequester.GetPartyId())
		m_lastTokenRequester.SetToNone();

	// ReleaseAck relevant only for PPC and blocked by the partyCntl
	ReleaseAck(partyId, mcuNumber, terminalnumber);
}

//--------------------------------------------------------------------------
void CContentToken::OnContentBrdgWithdrawTokenWithdraw(CSegment* pParam)
{
	// If we in the middle of switch process and we wait for withdraw ACK and
	// meantime we want to change the content protocol in the bridge - we need to:
	// 1) Send NAK to the last token requester
	// 2) clear the lastTokenRequester.
	if (!m_lastTokenRequester) // if there is pending Acquire request
	{
		PTRACE(eLevelInfoNormal, "CContentToken::OnContentBrdgWithdrawTokenWithdraw - Do Nothing...");
	}
	else
	{
		BYTE lastRequesterMcuNumber      = m_lastTokenRequester.GetMcuNumber();
		BYTE lastRequesterTerminalNumber = m_lastTokenRequester.GetTerminalNumber();

		AcquireNak(m_lastTokenRequester.GetPartyId(), lastRequesterMcuNumber, lastRequesterTerminalNumber);
		m_lastTokenRequester.SetToNone();
	}
}

//--------------------------------------------------------------------------
void CContentToken::OnContentBrdgWithdrawAckWithdraw(CSegment* pParam)
{
	PartyRsrcID partyId;
	*pParam >> partyId;

	// Withdraw_Ack can be received only from current token holder
	if (partyId != m_currentTokenHolder.GetPartyId())
		return;

	WithdrawAckHolderAction();
}

//--------------------------------------------------------------------------
void CContentToken::OnContentBrdgRoleProviderInfoWithdraw(CSegment* pParam)
{
}

//--------------------------------------------------------------------------
void CContentToken::OnContentBrdgPartyDisconnectWithdraw(CSegment* pParam)
{
	PartyRsrcID partyId;
	*pParam >> partyId;

	if (partyId == m_lastTokenRequester.GetPartyId())      // if disconnecting party is last requester - clean last requester
		m_lastTokenRequester.SetToNone();
	else if (partyId == m_currentTokenHolder.GetPartyId()) // if disconnecting party is TokenHolder - drop it
		WithdrawAckHolderAction();
}

//--------------------------------------------------------------------------
void CContentToken::OnTimerWithdrawReTransmitNOTHELD(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CContentToken::OnTimerWithdrawReTransmitNOTHELD - Ignored");
}

//--------------------------------------------------------------------------
void CContentToken::OnTimerWithdrawReTransmitHELD(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CContentToken::OnTimerWithdrawReTransmitHELD - Ignored");
}

//--------------------------------------------------------------------------
void CContentToken::OnTimerWithdrawReTransmitWithdraw(CSegment* pParam)
{
	CTaskApp* pParty = (CTaskApp*)m_currentTokenHolder.GetParty();
	TRACECOND_AND_RETURN(!pParty, "Failed, Party does not exist");

	TRACEINTO << "PartyId:" << pParty->GetPartyId();

	CPartyApi* pPartyApi = new CPartyApi;
	pPartyApi->CreateOnlyApi(pParty->GetRcvMbx());

	// For use of H323 parties only (EPC and Duo):
	// send to party the flag that action is ChangeSpeaker
	BYTE isSpeakerChange = (!(!m_lastTokenRequester));
	BYTE mcuNumber       = m_currentTokenHolder.GetMcuNumber();
	BYTE terminalNumber  = m_currentTokenHolder.GetTerminalNumber();

	pPartyApi->SendContentMessage(CONTENT_ROLE_TOKEN_WITHDRAW, mcuNumber, terminalNumber, 0, isSpeakerChange);

	StartTimer(WITHDRAW_CONTENT_TOKEN_TOUT, WITHDRAW_RETRANSMIT_TIME);

	POBJDELETE(pPartyApi);
}

//--------------------------------------------------------------------------
void CContentToken::AcquireAck(PartyRsrcID partyId, BYTE mcuNum, BYTE terminalNum)
{
	// Note, that this function must be called only after m_currentTokenHolder is updated to the new value
	if (GetTokenHolderPartyId())
	{
		PASSERT_AND_RETURN(partyId != m_currentTokenHolder.GetPartyId());

		CTaskApp* pParty = (CTaskApp*)GetLookupTableParty()->Get(partyId);
		TRACECOND_AND_RETURN(!pParty, "PartyId:" << partyId << " - Failed, Party does not exist");

		m_state = HELD;
		DeleteTimer(NO_ROLE_PROVIDER_TOUT);

		CPartyApi* pPartyApi = new CPartyApi;
		pPartyApi->CreateOnlyApi(pParty->GetRcvMbx());
		pPartyApi->SendContentMessage(CONTENT_ROLE_TOKEN_ACQUIRE_ACK, mcuNum, terminalNum);
		POBJDELETE(pPartyApi);
	}
}

//--------------------------------------------------------------------------
void CContentToken::AcquireNak(PartyRsrcID partyId, BYTE mcuNum, BYTE terminalNum)
{
	CTaskApp* pParty = (CTaskApp*)GetLookupTableParty()->Get(partyId);
	TRACECOND_AND_RETURN(!pParty, "PartyId:" << partyId << " - Failed, Party does not exist");

	TRACEINTO << "PartyId:" << partyId << ", McuNum:" << (int)mcuNum << ", TerminalNum:" << (int)terminalNum;

	CPartyApi* pPartyApi = new CPartyApi;
	pPartyApi->CreateOnlyApi(pParty->GetRcvMbx());
	pPartyApi->SendContentMessage(CONTENT_ROLE_TOKEN_ACQUIRE_NAK, mcuNum, terminalNum);
	POBJDELETE(pPartyApi);
}

//--------------------------------------------------------------------------
void CContentToken::ReleaseAck(PartyRsrcID partyId, BYTE mcuNum, BYTE terminalNum)
{
	CTaskApp* pParty = (CTaskApp*)GetLookupTableParty()->Get(partyId);
	TRACECOND_AND_RETURN(!pParty, "PartyId:" << partyId << " - Failed, Party does not exist");

	TRACEINTO << "PartyId:" << partyId << ", McuNum:" << (int)mcuNum << ", TerminalNum:" << (int)terminalNum;

	CPartyApi* pPartyApi = new CPartyApi;
	pPartyApi->CreateOnlyApi(pParty->GetRcvMbx());
	pPartyApi->SendContentMessage(CONTENT_ROLE_TOKEN_RELEASE_ACK, mcuNum, terminalNum);
	POBJDELETE(pPartyApi);
}

//--------------------------------------------------------------------------
void CContentToken::WithdrawRequest(PartyRsrcID partyId, WORD isSendToParty)
{
	// Withdraw request can be sent only to current token holder
	PASSERT_AND_RETURN(partyId != m_currentTokenHolder.GetPartyId());

	m_state = WITHDRAW;
	if (isSendToParty)
	{
		CTaskApp* pParty = (CTaskApp*)GetLookupTableParty()->Get(partyId);
		TRACECOND_AND_RETURN(!pParty, "PartyId:" << partyId << " - Failed, Party does not exist");

		CPartyApi* pPartyApi = new CPartyApi;
		pPartyApi->CreateOnlyApi(pParty->GetRcvMbx());

		// For use of H323 parties only (EPC and Duo):
		// send to party the flag that action is ChangeSpeaker
		BYTE isSpeakerChange = (!(!m_lastTokenRequester));
		BYTE mcuNumber       = m_currentTokenHolder.GetMcuNumber();
		BYTE terminalNumber  = m_currentTokenHolder.GetTerminalNumber();

		pPartyApi->SendContentMessage(CONTENT_ROLE_TOKEN_WITHDRAW, mcuNumber, terminalNumber, 0, isSpeakerChange);
		StartTimer(WITHDRAW_CONTENT_TOKEN_TOUT, WITHDRAW_RETRANSMIT_TIME);
		POBJDELETE(pPartyApi);
	}
	else
	{
		TRACEINTO << "PartyId:" << partyId << " - Party disconnect, don't send withdraw to party";
	}
}

//--------------------------------------------------------------------------
void CContentToken::WithdrawAckHolderAction(BYTE immediate)
{
	DeleteTimer(WITHDRAW_CONTENT_TOKEN_TOUT);
	if (!m_lastTokenRequester) // if there is NO pending Acquire request
	{
		m_currentTokenHolder.SetToNone();
		m_state = NOTHELD;
		StartTimer(NO_ROLE_PROVIDER_TOUT, NO_ROLE_PROVIDER_TIME_AFTER_WITHDRAW);
		DumpToken("WithdrawAckHolderAction 1");
		if (!immediate)
			NoTokenHolder();
		else
			NoTokenHolderImmediate();
	}
	else // if there is pending Acquire request
	{
		m_currentTokenHolder = m_lastTokenRequester;
		m_lastTokenRequester.SetToNone();
		BYTE mcuNumber      = m_currentTokenHolder.GetMcuNumber();
		BYTE terminalNumber = m_currentTokenHolder.GetTerminalNumber();
		DumpToken("WithdrawAckHolderAction 2");
		AcquireAck(m_currentTokenHolder.GetPartyId(), mcuNumber, terminalNumber);
		NewTokenHolder(mcuNumber, terminalNumber);
	}
}

//--------------------------------------------------------------------------/
void CContentToken::NoTokenHolderImmediate(void)
{
	// Inform Content Bridge that there is no Content token holder
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(m_pConfApi));
	PASSERT_AND_RETURN(GetTokenHolderPartyId());

	CConf* pConf = (CConf*)CProcessBase::GetProcess()->GetCurrentTask();
	PASSERT_AND_RETURN(!pConf);
	PASSERT_AND_RETURN(!pConf->m_pContentBridge);

	CSegment* pMsg  = new CSegment();
	pConf->m_pContentBridge->HandleEvent(pMsg, 0, NOTOKENHOLDER);
	pConf->OnContentBrdgStopContentCONNECT(NULL);
}

//--------------------------------------------------------------------------
void CContentToken::NoTokenHolder(void)
{
	// Inform Content Bridge that there is no Content token holder
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(m_pConfApi));
	PASSERT_AND_RETURN(GetTokenHolderPartyId());

	((CConfApi*)m_pConfApi)->NoContentTokenHolder();
}

//--------------------------------------------------------------------------
void CContentToken::NewTokenHolder(BYTE mcuNumber, BYTE terminalNumber)
{
	// Inform Content Bridge that there is new Content Presenter
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(m_pConfApi));
	PASSERT_AND_RETURN(!GetTokenHolderPartyId());

	((CConfApi*)m_pConfApi)->NewContentTokenHolder(mcuNumber, terminalNumber);
}

//--------------------------------------------------------------------------
void CContentToken::SendFreezeContent()
{
	// Send freeze content to all content parties
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(m_pConfApi));

	((CConfApi*)m_pConfApi)->ContentFreezePic();
}

//--------------------------------------------------------------------------
void CContentToken::BroadcastNoRoleProvider()
{
	// Inform Content Bridge that there is NO Content token holder !
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(m_pConfApi));
	PASSERT_AND_RETURN(GetTokenHolderPartyId());

	((CConfApi*)m_pConfApi)->ContentNoRoleProvider();
}

//--------------------------------------------------------------------------
void CContentToken::DumpToken(const char* str)
{
	TRACEINTO << "CurrentTokenHolderPartyId:" << m_currentTokenHolder.GetPartyId() << ", LastTokenRequesterPartyId:" << m_lastTokenRequester.GetPartyId() << " - Called from " << str;
}


////////////////////////////////////////////////////////////////////////////
//                        CContentCascadeSlaveToken
////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CContentCascadeSlaveToken)

	ONEVENT(SLAVE_ACQUIRE_CONTENT_TOKEN ,NOTHELD  ,CContentCascadeSlaveToken::OnSlaveAcquireTokenNOTHELD)
	ONEVENT(SLAVE_ACQUIRE_CONTENT_TOKEN ,HELD     ,CContentCascadeSlaveToken::OnSlaveAcquireTokenHELD)
	ONEVENT(SLAVE_ACQUIRE_CONTENT_TOKEN ,WITHDRAW ,CContentCascadeSlaveToken::OnSlaveAcquireTokenWITHDRAW)

	ONEVENT(RELEASE_CONTENT_TOKEN       ,NOTHELD  ,CContentCascadeSlaveToken::OnContentBrdgReleaseTokenNOTHELD)
	ONEVENT(RELEASE_CONTENT_TOKEN       ,HELD     ,CContentCascadeSlaveToken::OnContentBrdgReleaseTokenHELD)
	ONEVENT(RELEASE_CONTENT_TOKEN       ,WITHDRAW ,CContentCascadeSlaveToken::OnContentBrdgReleaseTokenWITHDRAW)

	ONEVENT(ACQUIRE_ACK_FROM_MASTER     ,NOTHELD  ,CContentCascadeSlaveToken::OnContentBrdgAcquireAckFromMasterNOTHELD)

	ONEVENT(RATE_CHANGE_FROM_MASTER     ,NOTHELD  ,CContentCascadeSlaveToken::OnContentBrdgRateChangeFromMasterNOTHELD)
	ONEVENT(RATE_CHANGE_FROM_MASTER     ,HELD     ,CContentCascadeSlaveToken::OnContentBrdgRateChangeFromMasterHELD)

	ONEVENT(PARTY_DISCONNECTS           ,NOTHELD  ,CContentCascadeSlaveToken::OnContentBrdgPartyDisconnectNOTHELD)
	ONEVENT(PARTY_DISCONNECTS           ,HELD     ,CContentCascadeSlaveToken::OnContentBrdgPartyDisconnectHELD)
	ONEVENT(PARTY_DISCONNECTS           ,WITHDRAW ,CContentCascadeSlaveToken::OnContentBrdgPartyDisconnectWithdraw)

PEND_MESSAGE_MAP(CContentCascadeSlaveToken,CContentToken);

//--------------------------------------------------------------------------
CContentCascadeSlaveToken::CContentCascadeSlaveToken()
{
	VALIDATEMESSAGEMAP
	m_newRateArrivedBeforeAcquireAck = 0xFFFFFFFF;
	m_rateChangeArrivedBeforeAcquireAck = NO;
}

//--------------------------------------------------------------------------
CContentCascadeSlaveToken::~CContentCascadeSlaveToken(void)
{
}

//--------------------------------------------------------------------------
void CContentCascadeSlaveToken::SlaveAcquire(CSegment* pPartyParams)
{
	PTRACE(eLevelInfoNormal, "CContentCascadeSlaveToken::SlaveAcquire");
	DispatchEvent(SLAVE_ACQUIRE_CONTENT_TOKEN, pPartyParams);
}

//--------------------------------------------------------------------------
void CContentCascadeSlaveToken::OnSlaveAcquireTokenNOTHELD(CSegment* pPartyParams)
{
	PTRACE(eLevelInfoNormal, "CContentCascadeSlaveToken::OnSlaveAcquireTokenNOTHELD");
	SlaveAcquireToken(pPartyParams);
}

//--------------------------------------------------------------------------
void CContentCascadeSlaveToken::OnSlaveAcquireTokenHELD(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CContentCascadeSlaveToken::OnSlaveAcquireTokenHELD");
	SlaveAcquireToken(pParam);
}

//--------------------------------------------------------------------------
void CContentCascadeSlaveToken::OnSlaveAcquireTokenWITHDRAW(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CContentCascadeSlaveToken::OnSlaveAcquireTokenWITHDRAW");
	SlaveAcquireToken(pParam);
}

//--------------------------------------------------------------------------
void CContentCascadeSlaveToken::SlaveAcquireToken(CSegment* pPartyParams)
{
	PartyRsrcID partyId;
	BYTE mcuNum;
	BYTE terminalNum;
	BYTE randomNum;
	BYTE isRequestSentAlreadyToMaster = NO;

	DWORD oldSegRead = pPartyParams->GetRdOffset();
	*pPartyParams >> partyId >> mcuNum >> terminalNum >> randomNum;
	DWORD newSegRead = pPartyParams->GetRdOffset();
	pPartyParams->DecRead(newSegRead - oldSegRead);

	TRACEINTO << "PartyId:" << partyId << ", McuNum:" << (int)mcuNum << ", TerminalNum:" << (int)terminalNum << ", RandomNum:" << (int)randomNum;

	if ((m_currentTokenHolder.GetPartyId() != 0) && (NULL != m_currentTokenHolder.GetParty()))
		isRequestSentAlreadyToMaster = YES;

	m_currentTokenHolder.Set(partyId, mcuNum, terminalNum, randomNum);
	m_lastTokenRequester.SetToNone(); // no additional requester for token
	DumpToken(__FUNCTION__);
	if (NO == isRequestSentAlreadyToMaster)
		ForwardContentTokenMsgToMaster(pPartyParams, CONTENT_ROLE_TOKEN_ACQUIRE);
}

//--------------------------------------------------------------------------
void CContentCascadeSlaveToken::ForwardContentTokenMsgToMaster(CSegment* pPartyParams, OPCODE opcode)
{
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(m_pConfApi));

	PartyRsrcID partyId;
	BYTE mcuNum;
	BYTE terminalNum;
	BYTE randomNum;

	*pPartyParams >> partyId >> mcuNum >> terminalNum >> randomNum;

	TRACEINTO << "PartyId:" << partyId << ", McuNum:" << (int)mcuNum << ", TerminalNum:" << (int)terminalNum << ", RandomNum:" << (int)randomNum;

	((CConfApi*)m_pConfApi)->ForwardContentTokenMsgToMaster(mcuNum, terminalNum, randomNum, opcode);
}

//--------------------------------------------------------------------------
void CContentCascadeSlaveToken::OnContentBrdgReleaseTokenNOTHELD(CSegment* pPartyParams)
{
	PTRACE(eLevelInfoNormal, "CContentCascadeSlaveToken::OnContentBrdgReleaseTokenNOTHELD");
	ContentBrdgReleaseToken(pPartyParams);
}

//--------------------------------------------------------------------------
void CContentCascadeSlaveToken::OnContentBrdgReleaseTokenHELD(CSegment* pPartyParams)
{
	PTRACE(eLevelInfoNormal, "CContentCascadeSlaveToken::OnContentBrdgReleaseTokenHELD");
	ContentBrdgReleaseToken(pPartyParams);
}

//--------------------------------------------------------------------------
void CContentCascadeSlaveToken::OnContentBrdgReleaseTokenWITHDRAW(CSegment* pPartyParams)
{
	PTRACE(eLevelInfoNormal, "CContentCascadeSlaveToken::OnContentBrdgReleaseTokenWITHDRAW");
	ContentBrdgReleaseToken(pPartyParams);
}

//--------------------------------------------------------------------------
void CContentCascadeSlaveToken::ContentBrdgReleaseToken(CSegment* pPartyParams)
{
	PartyRsrcID partyId;
	BYTE mcuNum;
	BYTE terminalNum;
	BYTE DummyRandomNum = 1; // The randomeNum is NOT in use Release i add it to be align with Acquire
	                         // to be able to common use of the function ForwardContentTokenMsgToMaster

	DWORD oldSegRead = pPartyParams->GetRdOffset();
	*pPartyParams >> partyId >> mcuNum >> terminalNum;
	DWORD newSegRead = pPartyParams->GetRdOffset();
	pPartyParams->DecRead(newSegRead - oldSegRead);

	*pPartyParams << DummyRandomNum;

	TRACEINTO << "PartyId:" << partyId << ", McuNum:" << (int)mcuNum << ", TerminalNum:" << (int)terminalNum;

	ForwardContentTokenMsgToMaster(pPartyParams, CONTENT_ROLE_TOKEN_RELEASE);
}

//--------------------------------------------------------------------------
void CContentCascadeSlaveToken::OnContentBrdgAcquireAckFromMasterNOTHELD(CSegment* pPartyParams)
{
	PartyRsrcID partyId = m_currentTokenHolder.GetPartyId();
	BYTE mcuNum         = m_currentTokenHolder.GetMcuNumber();
	BYTE terminalNum    = m_currentTokenHolder.GetTerminalNumber();

	TRACEINTO << "PartyId:" << partyId << ", McuNum:" << (int)mcuNum << ", TerminalNum:" << (int)terminalNum;

	AcquireAck(partyId, mcuNum, terminalNum);

	if (YES == RateChangeArrivedBeforeAcquireAck())
	{
		BYTE      randomNumber      = 0xFF;
		CSegment* pChangeRateParams = new CSegment;

		*pChangeRateParams << partyId << mcuNum << terminalNum << randomNumber << (DWORD)m_newRateArrivedBeforeAcquireAck;

		DispatchEvent(RATE_CHANGE_FROM_MASTER, pChangeRateParams);
		SetNewRateArrivedBeforeAcquireAck(0xFFFFFFFF);
		SetRateChangeArrivedBeforeAcquireAck(NO);
	}
}

//--------------------------------------------------------------------------
void CContentCascadeSlaveToken::OnContentBrdgRateChangeFromMasterNOTHELD(CSegment* pPartyParams)
{
	PartyRsrcID partyId;
	BYTE  mcuNum;
	BYTE  terminalNum;
	BYTE  randomNum;
	DWORD newContentRate;

	*pPartyParams >> partyId >> mcuNum >> terminalNum >> randomNum >> newContentRate;

	TRACEINTO << "PartyId:" << partyId << ", NewContentRate:" << newContentRate;

	if (newContentRate > 0)
	{
		SetNewRateArrivedBeforeAcquireAck(newContentRate);
		SetRateChangeArrivedBeforeAcquireAck(YES);
	}

// //The delay is because PresentationTokenResponse and RateChange from the master,in case that the Slave
// // is trying to start content,can arrive from the Master side in any order between them,the ACK on Acquire
// // may delay on tokenMsgManager since the party is waiting for RTP to open the resources before sending the
// //ACK to the slave.(In a basic bridge the ack is sent immediatly without waiting the RTP and the rate change
// //sent from the bridge just after,so the token is already in HELD state when RATE_CHANGE arrived.)
// //==> we will start timer to wait to this ack, to keep the original order.
// //The OnContentBrdgRateChangeFromMaster will activate only for HELD state which meens the ACK arrived !!!
// // see functions when timer jump.//if we want to open it again we can set counter and decreas it each time
// //For this reson we have to deal with STREAM_ON that arrive in CONNECT state. in basic bridge arrive on
// //RATE_CHANGE or CONTENT state
// CSegment* pSeg = new CSegment(*pPartyParams);
// StartTimer(START_CONTENT_DELAY,20*2*2,pSeg); //RTPCONTENTACKTOUT=20, + NETWORK TIME AND GK TIME *2=SAVE FACTOR
}

//--------------------------------------------------------------------------
void CContentCascadeSlaveToken::OnContentBrdgRateChangeFromMasterHELD(CSegment* pPartyParams)
{
	PartyRsrcID partyId;
	BYTE  mcuNum;
	BYTE  terminalNum;
	BYTE  randomNum;
	DWORD newContentRate;

	*pPartyParams >> partyId >> mcuNum >> terminalNum >> randomNum >> newContentRate;

	TRACEINTO << "PartyId:" << partyId << ", NewContentRate:" << newContentRate;

	DumpToken(__FUNCTION__);
	if (newContentRate > AMC_0k) // start content
	{
		NewTokenHolder(m_currentTokenHolder.GetMcuNumber(), m_currentTokenHolder.GetTerminalNumber());
	}
	else // stop content
	{
		m_currentTokenHolder.SetToNone();
		m_state = NOTHELD;
		NoTokenHolder();
	}
}

//--------------------------------------------------------------------------
void CContentCascadeSlaveToken::WithdrawAckHolderAction(BYTE immediate)
{
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(m_pConfApi));

	// Dummy params to use this function
	((CConfApi*)m_pConfApi)->ForwardContentTokenMsgToMaster(0xFF, 0xFF, 0xFF, CONTENT_ROLE_TOKEN_WITHDRAW_ACK);
	CContentToken::WithdrawAckHolderAction(immediate);
}

//--------------------------------------------------------------------------
void CContentCascadeSlaveToken::AcquireAckFromMaster(CSegment* pPartyParams)
{
	DispatchEvent(ACQUIRE_ACK_FROM_MASTER, pPartyParams);
}

//--------------------------------------------------------------------------
void CContentCascadeSlaveToken::RateChangeFromMaster(CSegment* pPartyParams)
{
	PTRACE(eLevelInfoNormal, "CContentCascadeSlaveToken::RateChangeFromMaster");
	DispatchEvent(RATE_CHANGE_FROM_MASTER, pPartyParams);
}

//--------------------------------------------------------------------------
void CContentCascadeSlaveToken::OnContentBrdgPartyDisconnectNOTHELD(CSegment* pParam)
{
	PartyRsrcID partyId;
	*pParam >> partyId;

	TRACEINTO << "PartyId:" << partyId << ", CurrentTokenHolderPartyId:" << m_currentTokenHolder.GetPartyId();

	// if disconnecting party is not TokenHolder - return
	if (partyId != m_currentTokenHolder.GetPartyId())
		return;

	ResetCurrentTokenHolder();
}

//--------------------------------------------------------------------------
void CContentCascadeSlaveToken::OnContentBrdgPartyDisconnectHELD(CSegment* pParam)
{
	PartyRsrcID partyId;
	*pParam >> partyId;

	TRACEINTO << "PartyId:" << partyId << ", CurrentTokenHolderPartyId:" << m_currentTokenHolder.GetPartyId();

	// if disconnecting party is not TokenHolder - return
	if (partyId != m_currentTokenHolder.GetPartyId())
		return;

	// here: disconnecting party is TokenHolder
	// send withdraw and don't wait for Ack
	BYTE mcuNumber      = m_currentTokenHolder.GetMcuNumber();
	BYTE terminalNumber = m_currentTokenHolder.GetTerminalNumber();
	Release(partyId, mcuNumber, terminalNumber);
}

//--------------------------------------------------------------------------
void CContentCascadeSlaveToken::OnContentBrdgPartyDisconnectWithdraw(CSegment* pParam)
{
	PartyRsrcID partyId;
	*pParam >> partyId;

	TRACEINTO << "PartyId:" << partyId << ", LastTokenRequesterPartyId:" << m_lastTokenRequester.GetPartyId();

	// if disconnecting party is last requester - clean last requester
	if (partyId == m_lastTokenRequester.GetPartyId())
		m_lastTokenRequester.SetToNone();

	PASSERT(1); // Yoella please take care!!!!!
}

//--------------------------------------------------------------------------
BYTE CContentCascadeSlaveToken:: RateChangeArrivedBeforeAcquireAck()
{
	return m_rateChangeArrivedBeforeAcquireAck;
}

//--------------------------------------------------------------------------
void CContentCascadeSlaveToken:: SetRateChangeArrivedBeforeAcquireAck(BYTE YesNo)
{
	m_rateChangeArrivedBeforeAcquireAck = YesNo;
}

//--------------------------------------------------------------------------
DWORD CContentCascadeSlaveToken::GetNewRateArrivedBeforeAcquireAck()
{
	return m_newRateArrivedBeforeAcquireAck;
}

//--------------------------------------------------------------------------
void CContentCascadeSlaveToken:: SetNewRateArrivedBeforeAcquireAck(DWORD rate)
{
	m_newRateArrivedBeforeAcquireAck = rate;
}

//--------------------------------------------------------------------------
void CContentCascadeSlaveToken:: ResetCurrentTokenHolder()
{
	PTRACE(eLevelInfoNormal, "CContentCascadeSlaveToken::ResetCurrentTokenHolder (Set to None) ");
	m_currentTokenHolder.SetToNone();
}

//--------------------------------------------------------------------------
// CContentCascadeMasterToken
//--------------------------------------------------------------------------
PBEGIN_MESSAGE_MAP(CContentCascadeMasterToken)

PEND_MESSAGE_MAP(CContentCascadeMasterToken, CContentToken);

//--------------------------------------------------------------------------
CContentCascadeMasterToken::CContentCascadeMasterToken()
{
	VALIDATEMESSAGEMAP
}

//--------------------------------------------------------------------------
CContentCascadeMasterToken::~CContentCascadeMasterToken(void)
{
}
