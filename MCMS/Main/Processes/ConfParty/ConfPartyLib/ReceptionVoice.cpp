#include "ReceptionVoice.h"
#include "PObject.h"
#include "Lobby.h"
#include "Trace.h"
#include "TraceStream.h"
#include "IsdnNetSetup.h"
#include "ConfPartyManagerLocalApi.h"
#include "ConfPartyProcess.h"
#include "NetHardwareInterface.h"
#include "ConfPartyGlobals.h"
#include "ConfPartyRoutingTable.h"

// ~~~~~~~~~~~~~~ Global functions ~~~~~~~~~~~~~~~~~~~~~~~~~~
extern CConfPartyRoutingTable* GetpConfPartyRoutingTable(void);
extern "C" void                PartyVoiceInEntryPoint(void* appParam);

PBEGIN_MESSAGE_MAP(CReceptionVoice)
	ONEVENT(RELEASE_UNRESERVED_PARTY, SUSPEND, CReceptionVoice::OnLobbyReleasePartySuspend)
	ONEVENT(REJECT_UNRESERVED_PARTY,  SUSPEND, CReceptionVoice::OnConfMngrRejectPartySuspend)
PEND_MESSAGE_MAP(CReceptionVoice, CReception);

////////////////////////////////////////////////////////////////////////////
//                        CReceptionVoice
////////////////////////////////////////////////////////////////////////////
CReceptionVoice::CReceptionVoice(CTaskApp* pOwnerTask) : CReception(pOwnerTask)
{
	VALIDATEMESSAGEMAP;
}

//--------------------------------------------------------------------------
CReceptionVoice::~CReceptionVoice()
{
}

//--------------------------------------------------------------------------
const char* CReceptionVoice::NameOf()  const
{
	return "CReceptionVoice";
}

//--------------------------------------------------------------------------
void* CReceptionVoice::GetMessageMap()
{
	return (void*)m_msgEntries;
}

//--------------------------------------------------------------------------
void CReceptionVoice::Create(DWORD monitorConfId,
                             DWORD monitorPartyId,
                             COsQueue* pConfRcvMbx,
                             COsQueue* pLobbyRcvMbx,
                             CIsdnNetSetup* pNetSetUp,
                             CLobby* pLobby,
                             char* pPartyName,
                             const char* confName,
                             CConfParty* pConfParty,
                             CCommConf* pComConf)
{
	TRACEINTO << "monitorConfId:" << monitorConfId << ", monitorPartyId:" << monitorPartyId;

	m_pLobby        = pLobby;
	m_pConfParty    = pConfParty;
	m_confMonitorId = monitorConfId;

	WORD mcuNumber      = 1;
	WORD terminalNumber = 1;
	WORD serviceId      = 0;

	SetConfOnAirFlag(YES);

	// copy ISDN setup call parameters to local storage for suspend
	m_pSuspended_NetSetUp = new CIsdnNetSetup;
	m_pSuspended_NetSetUp->copy(pNetSetUp);

	// select A_law / U_law for voice calls
	WORD voice_type = pConfParty->GetVoice();
	PASSERT(voice_type != YES);

	voice_type = GetVoiceType(pConfParty);

	// select party
	void (*entryPoint)(void*) = PartyVoiceInEntryPoint;

	// start party
	m_pPartyApi = new CPartyApi;
	m_pPartyApi->Create(entryPoint,
	                    *pLobbyRcvMbx,
	                    *pConfRcvMbx,
	                    DEFAULT_PARTY_ID,
	                    m_pConfParty->GetPartyId(),
	                    m_confMonitorId,
	                    "",
	                    pPartyName,
	                    confName,
	                    serviceId,
	                    1,
	                    1,
	                    pConfParty->GetPassword(),
	                    voice_type,
	                    1,
	                    pConfParty->GetRecordingPort());

	// Starts identify party
	m_pParty = m_pPartyApi->GetTaskAppPtr();  // set self identification

#ifdef LOOKUP_TABLE_DEBUG_TRACE
	TRACEINTO << "D.K."
			<< "  PartyName:"        << pPartyName
			<< ", PartyId:"          << m_pParty->GetPartyId() << "(0)"
			<< ", IsDialOut:"        << 0
			<< ", Pointer:"          << std::hex << m_pParty;
#endif

	StartTimer(IDENTTOUT, IDENT_TOUT);
	m_pPartyApi->IdentifyNetChannel(pNetSetUp);
}

//--------------------------------------------------------------------------
void CReceptionVoice::CreateSuspend(CIsdnNetSetup* pNetSetup, COsQueue* pLobbyRcvMbx, CLobby* pLobby, CTaskApp* ListId, DWORD ConfId, char* confName)
{
	TRACEINTO << "ConfId:" << ConfId;

	m_state  = SUSPEND;
	m_pLobby = pLobby;

	SetTargetConfName(confName);
	SetParty(ListId); // this value (pListId) is not a valid memory pointer, it is used only to identify the suspended CReception object in the waiting list
	SetMonitorMRId(ConfId);
	SetConfOnAirFlag(NO);

	// copy ISDN setup call parameters to local storage for suspend
	m_pSuspended_NetSetUp = new CIsdnNetSetup;
	m_pSuspended_NetSetUp->copy(pNetSetup);

	// will remove this item from the list in 30 seconds if no response received from MNGR
	StartTimer(PARTYSUSPENDTOUT, 30*SECOND);
}

//--------------------------------------------------------------------------
void CReceptionVoice::CreateMeetingRoom(CNetSetup* pNetSetup, COsQueue* pLobbyRcvMbx, CLobby* pLobby, CTaskApp* ListId, DWORD ConfId, char* mrName)
{
	TRACEINTO << "ConfId:" << ConfId;

	m_state  = SUSPEND;
	m_pLobby = pLobby;

	SetTargetConfName(mrName);
	SetParty(ListId);
	SetMonitorMRId(ConfId);
	SetConfOnAirFlag(NO);

	// copy ISDN setup call parameters to local storage for suspend
	m_pSuspended_NetSetUp = new CIsdnNetSetup;
	m_pSuspended_NetSetUp->copy(pNetSetup);

	StartTimer(PARTYSUSPENDTOUT, 30*SECOND);

	CConfPartyManagerLocalApi confPartyMngrApi;
	confPartyMngrApi.StartMeetingRoom(ConfId);
}

//--------------------------------------------------------------------------
void CReceptionVoice::CreateGateWayConf(CNetSetup* pNetSetup, COsQueue* pLobbyRcvMbx, CLobby* pLobby, CTaskApp* ListId, DWORD ConfId, char* gwName, char* targetNumber)
{
	TRACEINTO << "ConfId:" << ConfId;

	m_state  = SUSPEND;
	m_pLobby = pLobby;

	SetTargetConfName(gwName);
	SetParty(ListId);
	SetMonitorMRId(ConfId);
	SetConfOnAirFlag(NO);

	// copy ISDN setup call parameters to local storage for suspend
	m_pSuspended_NetSetUp = new CIsdnNetSetup;
	m_pSuspended_NetSetUp->copy(pNetSetup);

	// will remove this item from the list in 30 seconds if no response received from ConfParyMngr
	StartTimer(PARTYSUSPENDTOUT, 30*SECOND);

	// send START_GW_CONF msg to ConfPartyManager
	CConfPartyManagerLocalApi confPartyMngrApi;
	confPartyMngrApi.StartGateWayConf(ConfId, gwName, targetNumber);
}

//--------------------------------------------------------------------------
void CReceptionVoice::GetCallParams(CIsdnNetSetup** pNetSetUp)
{
	// create a copy of the suspended ISDN parameters and return them to caller
	if (IsValidPObjectPtr(m_pSuspended_NetSetUp))
	{
		*pNetSetUp = new CIsdnNetSetup;
		(*pNetSetUp)->copy(m_pSuspended_NetSetUp);
	}
	else
		*pNetSetUp = NULL;
}

//--------------------------------------------------------------------------
void CReceptionVoice::OnLobbyReleasePartySuspend(CSegment* pParam)
{
	DeleteTimer(PARTYSUSPENDTOUT);

	// lobby continue processing of the suspended call
	// there is no longer need for this reception object
	DeleteAllTimers();
	m_pLobby->RemoveFromList(this);
	delete this;
}

//--------------------------------------------------------------------------
void CReceptionVoice::CreateSuspendConfExist(CIsdnNetSetup* pNetSetUp, COsQueue* pLobbyRcvMbx, CLobby* pLobby, CTaskApp* ListId, DWORD ConfId, char* confName)
{
	TRACEINTO << "ConfId:" << ConfId;

	WORD isVoice = 1;
	CreateSuspend(pNetSetUp, pLobbyRcvMbx, pLobby, ListId, ConfId, confName);
	SetConfOnAirFlag(YES);
	mcTransportAddress destIP;
	memset(&destIP, 0, sizeof(mcTransportAddress));
	destIP.ipVersion = eIpVersion4;
	CConfPartyManagerLocalApi* pConfPartyMngrApi = (CConfPartyManagerLocalApi*)CProcessBase::GetProcess()->GetManagerApi();
	pConfPartyMngrApi->AcceptUnreservedParty((const mcTransportAddress*)&destIP, ConfId, (DWORD)ListId, isVoice, NO, eNotSipFactory);
}

//--------------------------------------------------------------------------
WORD CReceptionVoice::GetVoiceType(CConfParty* pConfParty)
{
	// get E1/T1 span type for voice A_law / U_law calls
	WORD voice_type = U_LAW;
	CConfPartyProcess* pConfPartyProcess = (CConfPartyProcess*)CConfPartyProcess::GetProcess();
	const RTM_ISDN_PARAMS_MCMS_S* pIsdnServiceStruct = pConfPartyProcess->GetIsdnService("");
	PASSERT_AND_RETURN_VALUE(!pIsdnServiceStruct, voice_type);

	if (eSpanTypeE1 == pIsdnServiceStruct->spanDef.spanType)
		voice_type = 1;     // A_law;
	else
		voice_type = U_LAW; // U_law

	return (voice_type);
}

//--------------------------------------------------------------------------
void CReceptionVoice::ReplayToNet(DWORD opcode, CIsdnNetSetup& netSetup) const
{
	DWORD rejectId = 0;

	::AllocateRejectID(rejectId);
	CRsrcParams rRsrcParams(rejectId, rejectId, STANDALONE_CONF_ID, eLogical_net);

	CNetHardwareInterface netHardwareInterface;
	netHardwareInterface.SendMsgWithPhysicalInfo(opcode, rRsrcParams, netSetup);
}

//--------------------------------------------------------------------------
void CReceptionVoice::OnException()
{
	CIsdnNetSetup* pNetSetup = dynamic_cast<CIsdnNetSetup*>(m_pSuspended_NetSetUp);
	PASSERT_AND_RETURN(!pNetSetup);

	// disconnect the party
	ReplayToNet(NET_CLEAR_REQ, *pNetSetup);

	// Remove the Reception from the Lobby
	CReception::OnException();
}

//--------------------------------------------------------------------------
void CReceptionVoice::OnConfMngrRejectPartySuspend(CSegment* pParam)
{
	DeleteTimer(PARTYSUSPENDTOUT);

	CIsdnNetSetup* pNetSetup = dynamic_cast<CIsdnNetSetup*>(m_pSuspended_NetSetUp);
	PASSERT_AND_RETURN(!pNetSetup);

	if (pNetSetup)
		ReplayToNet(NET_CLEAR_REQ, *pNetSetup);
	else
		PASSERT(1);

	DeleteAllTimers();

	delete this;
}
