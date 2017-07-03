#include  "Reception.h"
#include  "Lobby.h"
#include "OpcodesMcmsCommon.h"
#include "TraceStream.h"
#include "IsdnNetSetup.h"

PBEGIN_MESSAGE_MAP(CReception)
	// events from party layer
	ONEVENT(PARTY_DISCON,             IDENT,    CReception::OnPartyDisconIdent)
	ONEVENT(PARTY_DISCON,             TRANSFER, CReception::OnPartyDisconTransfer)
	ONEVENT(PARTY_IDENT,              IDENT,    CReception::OnPartyIdentIdent)
	ONEVENT(PARTY_TRANSFER,           TRANSFER, CReception::OnPartyTransferTransfer)
	// local events
	ONEVENT(TRANSFERTOUT,             TRANSFER, CReception::OnTimerPartyTransfer)
	ONEVENT(IDENTTOUT,                IDENT,    CReception::OnTimerPartyIdent)
	ONEVENT(PARTYSUSPENDTOUT,         SUSPEND,  CReception::OnTimerPartySuspend)
	// events from conference
	ONEVENT(RELEASE_UNRESERVED_PARTY, SUSPEND,  CReception::OnLobbyReleasePartySuspend)
	// events from conference manager
	ONEVENT(REJECT_UNRESERVED_PARTY,  SUSPEND,  CReception::OnConfMngrRejectPartySuspend)
PEND_MESSAGE_MAP(CReception, CStateMachine)


////////////////////////////////////////////////////////////////////////////
//                        CReception
////////////////////////////////////////////////////////////////////////////
CReception::CReception(CTaskApp* pOwnerTask) : CStateMachine(pOwnerTask)
{
	m_pPartyApi               = NULL;
	m_pParty                  = NULL;
	m_pLobby                  = NULL;
	m_state                   = IDENT;
	m_channel                 = 0;
	m_pSuspended_NetSetUp     = NULL;
	m_pSuspended_NetDesc      = NULL;
	m_isDisconnected          = 0;
	m_pBackUpPartyReservation = NULL;
	m_confOnAirFlag           = NO;
	m_targetConfName[0]       = '\0';
	m_targetConfType          = eDummyType;
}

//--------------------------------------------------------------------------
CReception::~CReception()
{
	if (IsValidPObjectPtr(m_pLobby))
	{
		BYTE inList = m_pLobby->IsReceptionStillInList(this);
		if (inList == YES)
		{
			PASSERTMSG(1, "Reception Destroy while still in Lobby List");
			m_pLobby->RemoveFromList(this);
		}
	}

	POBJDELETE(m_pSuspended_NetSetUp);
	POBJDELETE(m_pSuspended_NetDesc);
}

////--------------------------------------------------------------------------
//char* CReception::GetCalledNumber()
//{
//	WORD  numDigits        = 0;
//	char* pCalledTelNumber = new char[PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER];
//	strncpy(pCalledTelNumber, "NA", PRI_LIMIT_PHONE_DIGITS_LEN);
//	if (m_state == SUSPEND)
//		((CIsdnNetSetup*)m_pSuspended_NetSetUp)->GetCalledNumber(&numDigits, pCalledTelNumber);
//
//	return pCalledTelNumber;
//}
//
////--------------------------------------------------------------------------
//char* CReception::GetCallingNumber()
//{
//	WORD  numDigits         = 0;
//	char* pCallingTelNumber = new char[PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER];
//	strncpy(pCallingTelNumber, "NA", PRI_LIMIT_PHONE_DIGITS_LEN);
//	if (m_state == SUSPEND)
//		((CIsdnNetSetup*)m_pSuspended_NetSetUp)->GetCallingNumber(&numDigits, pCallingTelNumber);
//
//	return pCallingTelNumber;
//}

//--------------------------------------------------------------------------
void CReception::GetCallParams(CNetSetup** pNetSetup)
{
	PASSERT(1);// MUST BE OVERRIDEN IN DERIVED CLASS ACCORDING TO CALL TYPE
}

//--------------------------------------------------------------------------
void* CReception::GetMessageMap()
{
	return (void*)m_msgEntries;
}

//--------------------------------------------------------------------------
void CReception::Create(DWORD monitorConfId, DWORD monitorPartyId, COsQueue* pConfRcvMbx, COsQueue* pLobbyRcvMbx,
                        CNetSetup* pNetSetup, CLobby* pLobby, char* pPartyName, const char* confName,
                        WORD identMode, WORD partyType, CConfParty* pConfParty, WORD SubCPtype, WORD isChairEnabled,
                        CConfParty* pBackUpPartyReservation, WORD BackUpScenario)
{
	PASSERT(1);// MUST BE OVERRIDEN IN DERIVED CLASS ACCORDING TO CALL TYPE
}

//--------------------------------------------------------------------------
void CReception::Create(DWORD monitorConfId, DWORD monitorPartyId, COsQueue* pConfRcvMbx, COsQueue* pLobbyRcvMbx,
                        CIsdnNetSetup* pNetSetUp, CLobby* pLobby, char* pPartyName,
                        const char* confName, CConfParty* pConfParty, CCommConf* pComConf)
{
	PASSERT(1);// MUST BE OVERRIDEN IN DERIVED CLASS ACCORDING TO CALL TYPE
}

//--------------------------------------------------------------------------
// Create for party reject .
void CReception::Create(CNetSetup* pNetSetup, COsQueue* pLobbyRcvMbx, CLobby* pLobby)
{
	TRACEINTO << "Create for reject (Party is going to deal with the reject)";

	m_pPartyApi->RejectCall(pNetSetup);
	m_pPartyApi->Destroy();
	POBJDELETE(pNetSetup);
}

//--------------------------------------------------------------------------
// Create for party suspend in unattended call in.
void CReception::CreateSuspend(CNetSetup* pNetSetup, COsQueue* pLobbyRcvMbx, CLobby* pLobby, CTaskApp* ListId, DWORD ConfId, char* confName)
{
	m_state = SUSPEND;
	// each call type (ISDN,ATM,V35,323 ...) should override this function in a derived class
	PASSERT(1);// MUST BE OVERRIDEN IN DERIVED CLASS ACCORDING TO CALL TYPE

	// will remove this item from the list in 30 seconds if no response received from MNGR
	StartTimer(PARTYSUSPENDTOUT, 30*SECOND);
}

//--------------------------------------------------------------------------
// Create for party suspend in unattended call in.
void CReception::CreateSuspendConfExist(CNetSetup* pNetSetup, COsQueue* pLobbyRcvMbx, CLobby* pLobby, CTaskApp* ListId, DWORD ConfId, char* confName)
{
	m_state = SUSPEND;

	// each call type (ISDN,ATM,V35,323 ...) should override this function in a derived class
	PASSERT(1);// MUST BE OVERRIDEN IN DERIVED CLASS ACCORDING TO CALL TYPE

	// will remove this item from the list in 30 seconds if no response received from MNGR
	StartTimer(PARTYSUSPENDTOUT, 30*SECOND);
}

//--------------------------------------------------------------------------
// Create for party suspend in unattended call in.
void CReception::CreateSuspendConfExist(CIsdnNetSetup* pNetSetup, COsQueue* pLobbyRcvMbx, CLobby* pLobby, CTaskApp* ListId, DWORD ConfId, char* confName)
{
	CreateSuspendConfExist((CNetSetup*)pNetSetup, pLobbyRcvMbx, pLobby, ListId, ConfId, confName);
}

//--------------------------------------------------------------------------
// Create for new meeting room conference in unattended call in.
void CReception::CreateMeetingRoom(CNetSetup* pNetSetup, COsQueue* pLobbyRcvMbx, CLobby* pLobby, CTaskApp* ListId, DWORD confId, char* confName)
{
	m_state = SUSPEND;

	// each call type (ISDN,ATM,V35,323 ...) should override this function in a derived class
	PASSERT(1);// MUST BE OVERRIDEN IN DERIVED CLASS ACCORDING TO CALL TYPE

	// will remove this item from the list in 30 seconds if no response received from ConfPartyMngr
	StartTimer(PARTYSUSPENDTOUT, 30*SECOND);
}

//--------------------------------------------------------------------------
void CReception::Destroy(bool isInTransferList)
{
	DeleteAllTimers();
	if (isInTransferList)
		m_pLobby->OnEndPartyTransfer(this);

	if (m_pPartyApi)
	{
		m_pPartyApi->DestroyOnlyApi();
		POBJDELETE(m_pPartyApi);
	}
}

//--------------------------------------------------------------------------
void CReception::ReplayToNet(DWORD opcode, CIsdnNetSetup& netSetup) const
{
	PASSERT(1); // MUST BE OVERRIDEN IN DERIVED CLASS ACCORDING TO CALL TYPE
}

//--------------------------------------------------------------------------
void CReception::AcceptParty(CNetSetup* pNetSetUp, CCommConf* pCommConf, CConfParty* pConfParty)
{
	TRACEINTO << "Removing the reception, and release suspend reception";

	CSegment* pSeg = 0;
	OnLobbyReleasePartySuspend(pSeg);
}

//--------------------------------------------------------------------------
void CReception::HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode)
{
	switch (opCode)
	{
		case TIMER:
		{
			PASSERTMSG(1, "CReception::HandleEvent TIMER is not implemented yet");
			break;
		}

		default:
		{ // messages from party
			DispatchEvent(opCode, pMsg);
			break;
		}
	}
}

//--------------------------------------------------------------------------
bool operator==(const CReception& first, const CReception& second)
{
	bool rval = false;

	if (CPObject::IsValidPObjectPtr(const_cast<CReception*> (&first)) == 0)
		return rval;

	if (CPObject::IsValidPObjectPtr(const_cast<CReception*> (&second)) == 0)
		return rval;

	if (first.m_pParty == second.m_pParty)
		rval = true;

	return rval;
}

//--------------------------------------------------------------------------
bool operator<(const CReception& first, const CReception& second)
{
	bool rval = false;

	if (CPObject::IsValidPObjectPtr(const_cast<CReception*> (&first)) == 0)
		return rval;

	if (CPObject::IsValidPObjectPtr(const_cast<CReception*> (&second)) == 0)
		return rval;

	if (first.m_pParty != second.m_pParty)
		rval = true;

	return rval;
}

//--------------------------------------------------------------------------
void CReception::OnPartyDisconIdent(CSegment* pParam)
{
	TRACEINTO << "";

	OnException();
}

//--------------------------------------------------------------------------
void CReception::OnPartyDisconTransfer(CSegment* pParam)
{
	TRACEINTO << "";

	Destroy();
	delete this;
}

//--------------------------------------------------------------------------
void CReception::OnPartyIdentIdent(CSegment* pParam)
{
	*pParam >> m_channel >> m_status;  // m_channel for Type == 0

	TRACEINTO << "status:" << m_status << ", channel:" << m_channel << " - DeleteTimer(IDENTTOUT)";

	DeleteTimer(IDENTTOUT);

	if (m_status)
	{
		// if ident channel failed party "should entered unframed to conf - not implemented
		OnException();
	}
	else
	{
		m_state = TRANSFER;
		StartTimer(TRANSFERTOUT, TRANSFER_TOUT);

		if (m_channel == INITIAL)
		{
			if (m_pConfParty != NULL && m_pConfParty->GetPartyState() == PARTY_CONNECTING)
			{
				TRACESTRFUNC(eLevelError) << "Destroy party - IDENTIFICATION CONFLICT";
				m_pPartyApi->Destroy();
				Destroy();
			}
			else
				m_pPartyApi->Transfer(PARTYTRANSFER);
		}
		else
		{
			m_pPartyApi->Transfer(NETTRANSFER);
		}
	}
}

//--------------------------------------------------------------------------
void CReception::OnPartyTransferTransfer(CSegment* pParam)
{
	WORD status;
	*pParam >> status;

	TRACEINTO << "status:" << status << " - DeleteTimer(TRANSFERTOUT)";

	DeleteTimer(TRANSFERTOUT);
	DBGPASSERT(status);

	if (status) // party task delete itself
	{
		OnException();
	}
	else
	{
		Destroy();
		delete this;
	}
}

//--------------------------------------------------------------------------
// no need to send reject to the card the function is being called after the
// answer from the ConfMngr is analyzed by the Lobby and a new CReception was created.
// and this new object answer the card.
// Carmel changes NO more creation of Reject Reception when the ConfPartyManager invoked the Reject
void CReception::OnConfMngrRejectPartySuspend(CSegment* pParam)
{
	TRACEINTO << "DeleteTimer(PARTYSUSPENDTOUT)";

	DeleteTimer(PARTYSUSPENDTOUT);

	// kill the suspended object
	// a new reception object is created by lobby for the reject process
	DeleteAllTimers();

	delete this;
}

//--------------------------------------------------------------------------
void CReception::OnLobbyReleasePartySuspend(CSegment* pParam)
{
	TRACEINTO << "DeleteTimer(PARTYSUSPENDTOUT)";

	DeleteTimer(PARTYSUSPENDTOUT);
	DeleteAllTimers();
	m_pLobby->RemoveFromList(this);
	delete this;
}

//--------------------------------------------------------------------------
void CReception::OnTimerPartySuspend(CSegment* pParam)
{
	PASSERT(1); // MUST BE OVERRIDEN IN DERIVED CLASS ACCORDING TO CALL TYPE

	// there is no longer need for this reception object
	DeleteAllTimers();

	m_pLobby->RemoveReception(this);
	delete this;
}

//--------------------------------------------------------------------------
void CReception::OnTimerPartyIdent(CSegment* pParam)
{
	TRACEINTO << "";

	OnException();
}

//--------------------------------------------------------------------------
void CReception::OnTimerPartyTransfer(CSegment* pParam)
{
	TRACEINTO << "";

	OnException();
}

//--------------------------------------------------------------------------
void CReception::OnException()
{
	PASSERT(!m_pLobby);

	DeleteAllTimers();

	if (m_pPartyApi)
	{
		m_pPartyApi->Destroy();
		POBJDELETE(m_pPartyApi);
	}

	m_pLobby->OnEndPartyTransfer(this);
	delete this;
}

//--------------------------------------------------------------------------
BYTE CReception::IsPartyDefined(DWORD confMonitorId)
{
	TRACESTRFUNC(eLevelError) << "A function in my appropriate child should be defined";
	return FALSE;
	// Change it to pure virtual and set functions according for all reception types !!!!!
}

//--------------------------------------------------------------------------
const char* CReception::GetPartyName()
{
	if (GetConfParty())
		return GetConfParty()->GetName();
	else
		return ("Party Name NOT FOUND");
}
