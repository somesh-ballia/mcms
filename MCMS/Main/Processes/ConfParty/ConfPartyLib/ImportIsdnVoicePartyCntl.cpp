#include "ImportIsdnVoicePartyCntl.h"
#include "BridgeMoveParams.h"
#include "Conf.h"
#include "PartyApi.h"

PBEGIN_MESSAGE_MAP(CImportIsdnVoicePartyCntl)

	ONEVENT(PARTY_AUDIO_CONNECTED,                      IDLE, CImportIsdnVoicePartyCntl::OnAudConnect)
	ONEVENT(IMPORT_PARTY_CONNECT_TO_AUDIO_BRIDGE_TIMER, IDLE, CImportIsdnVoicePartyCntl::OnTimerConnectToAudioBrdg)

PEND_MESSAGE_MAP(CImportIsdnVoicePartyCntl, CIsdnPartyCntl);

////////////////////////////////////////////////////////////////////////////
//                        CImportIsdnVoicePartyCntl
////////////////////////////////////////////////////////////////////////////
CImportIsdnVoicePartyCntl::CImportIsdnVoicePartyCntl()
{
}

//--------------------------------------------------------------------------
CImportIsdnVoicePartyCntl::~CImportIsdnVoicePartyCntl()
{
}

//--------------------------------------------------------------------------
CImportIsdnVoicePartyCntl& CImportIsdnVoicePartyCntl::operator=(const CImportIsdnVoicePartyCntl& other)
{
	CIsdnPartyCntl::operator=(other);
	return *this;
}

//--------------------------------------------------------------------------
void* CImportIsdnVoicePartyCntl::GetMessageMap()
{
	return (void*)m_msgEntries;
}

//--------------------------------------------------------------------------
const char* CImportIsdnVoicePartyCntl::NameOf()  const
{
	return "CImportIsdnVoicePartyCntl";
}

//--------------------------------------------------------------------------
// This function is the most important function in the move party scenario.
// we create the CIsdnPartyCntl in the target conference for the new party as in CAddIsdnVoicePartyCntl::Create
// but using existing already opened channels, and resources (audio).
void CImportIsdnVoicePartyCntl::Create(CMoveIPImportParams* pMoveImportParams)
{
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pMoveImportParams->m_pImpConfPartyCntl));
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pMoveImportParams->m_pConf));

	m_state = IDLE;

	// 1) copy data from source-party CIsdnPartyCntl
	// here we copy all the relevant party data that doesn't depend on the new conference.
	SetDataForImportVoicePartyCntl(*(CIsdnPartyCntl*)(pMoveImportParams->m_pImpConfPartyCntl));

	// 1.a  register all party control state machines in the current task
	if (m_pBridgeMoveParams)
		m_pBridgeMoveParams->RegisterBridgePartyCntlsInTask();

	// 2) set data from new conference
	// here we set the new conference relevant data in the CIsdnPartyCntl
	m_pConf = pMoveImportParams->m_pConf;
	SetFullName(pMoveImportParams->m_pImpConfPartyCntl->GetName(), pMoveImportParams->m_name);

	m_pTaskApi = new CConfApi;
	m_pTaskApi->CreateOnlyApi((pMoveImportParams->m_pConf)->GetRcvMbx(), this);
	m_pTaskApi->SetLocalMbx((pMoveImportParams->m_pConf)->GetLocalQueue());

	UpdatePartyEntryInGlobalRsrcRoutingTblAfterMove();
	if (pMoveImportParams->m_pAudBridgeInterface != NULL)
		m_pAudioInterface = (pMoveImportParams->m_pAudBridgeInterface);

	if (pMoveImportParams->m_pConfAppMngrInterface != NULL)
		m_pConfAppMngrInterface = (pMoveImportParams->m_pConfAppMngrInterface);

	// 3) set communication mode and local capabilities
	m_IsMovedParty = TRUE;
	m_pPartyApi->SetRsrcConfIdForInterface(m_destResourceConfId);

	m_pPartyHWInterface = new CPartyInterface(*((pMoveImportParams->m_pImpConfPartyCntl)->GetPartyCntlResource()));
	m_pPartyHWInterface->SetConfRsrcId(m_destResourceConfId);
	// b) set resourses descriptors that allocated/deallocated at CConf level

	// set to new conf id only after using old conf id for deallocating resources
	m_monitorConfId = pMoveImportParams->m_sysConfId;

	CBridgePartyInitParams* pBrdgPartyInitParams = new CBridgePartyInitParams(m_name, m_pParty,
	                                                                          GetPartyRsrcId(), m_RoomId, GetInterfaceType(),
	                                                                          NULL, NULL,
	                                                                          m_pBridgeMoveParams->GetAndResetAudioBridgePartyCntlOnImport());
	bool isIvrAfterMove = false;
	if (m_moveType == eMoveIntoIvr)
		ON(isIvrAfterMove);

	m_pConfAppMngrInterface->ConnectPartyAudio(pBrdgPartyInitParams, isIvrAfterMove);

	m_eAudBridgeConnState = eSendOpenInAndOut;
	ON(m_bIsMemberInAudBridge);
	StartTimer(IMPORT_PARTY_CONNECT_TO_AUDIO_BRIDGE_TIMER, 8*SECOND);
	POBJDELETE(pBrdgPartyInitParams);
}

//--------------------------------------------------------------------------
// This function ends the import side of the move party scenario successful.
// Audio connected
void CImportIsdnVoicePartyCntl::OnAudConnect(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;

	DeleteTimer(IMPORT_PARTY_CONNECT_TO_AUDIO_BRIDGE_TIMER);

	if (m_eAudBridgeConnState == eBridgeDisconnected)
	{
		DBGPASSERT(1);
		TRACEINTO << "PartyId:" << GetPartyRsrcId() << "-  Connect has received after disconnect";
	}

	HandleAudioBridgeConnectedInd(pParam);

	m_pTaskApi->EndImportParty(m_pParty->GetPartyId(), statOK);
}

//--------------------------------------------------------------------------
void CImportIsdnVoicePartyCntl::OnTimerConnectToAudioBrdg(CSegment* pParam)
{
	PASSERTSTREAM(1, m_partyConfName);

	// Disconnect the party
	m_pTaskApi->EndImportParty(m_pParty->GetPartyId(), statIllegal);
}
