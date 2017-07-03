
#include "ImportIsdnPartyCntl.h"
#include "BridgeMoveParams.h"
#include "Conf.h"
#include "PartyApi.h"

PBEGIN_MESSAGE_MAP(CImportIsdnPartyCntl)

	ONEVENT(PARTY_AUDIO_CONNECTED,                      IDLE, CImportIsdnPartyCntl::OnAudConnect)
	ONEVENT(IMPORT_PARTY_CONNECT_TO_AUDIO_BRIDGE_TIMER, IDLE, CImportIsdnPartyCntl::OnTimerConnectToAudioBrdg)

PEND_MESSAGE_MAP(CImportIsdnPartyCntl, CIsdnPartyCntl);

////////////////////////////////////////////////////////////////////////////
//                        CImportIsdnPartyCntl
////////////////////////////////////////////////////////////////////////////
CImportIsdnPartyCntl::CImportIsdnPartyCntl()
{
}

//--------------------------------------------------------------------------
CImportIsdnPartyCntl::~CImportIsdnPartyCntl()
{
}

//--------------------------------------------------------------------------
CImportIsdnPartyCntl& CImportIsdnPartyCntl::operator=(const CImportIsdnPartyCntl& other)
{
	CIsdnPartyCntl::operator=(other);
	return *this;
}

//--------------------------------------------------------------------------
void* CImportIsdnPartyCntl::GetMessageMap()
{
	return (void*)m_msgEntries;
}

//--------------------------------------------------------------------------
const char* CImportIsdnPartyCntl::NameOf()  const
{
	return "CImportIsdnPartyCntl";
}

//--------------------------------------------------------------------------
// This function is the most important function in the move party scenario.
// we create the CIsdnPartyCntl in the target conference for the new party as in CAddIsdnPartyCntl::Create
// but using existing already opened channels, and resources (audio).
void CImportIsdnPartyCntl::Create(CMoveIPImportParams* pMoveImportParams)
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

	TRACEINTO << m_partyConfName;

	m_pTaskApi = new CConfApi;
	m_pTaskApi->CreateOnlyApi((pMoveImportParams->m_pConf)->GetRcvMbx(), this);
	m_pTaskApi->SetLocalMbx((pMoveImportParams->m_pConf)->GetLocalQueue());

	UpdatePartyEntryInGlobalRsrcRoutingTblAfterMove();

	if (pMoveImportParams->m_pAudBridgeInterface != NULL)
		m_pAudioInterface = (pMoveImportParams->m_pAudBridgeInterface);

	if (pMoveImportParams->m_pVidBridgeInterface != NULL)
		m_pVideoBridgeInterface = (pMoveImportParams->m_pVidBridgeInterface);

	if (pMoveImportParams->m_pConfAppMngrInterface != NULL)
		m_pConfAppMngrInterface = (pMoveImportParams->m_pConfAppMngrInterface);

/*  at the moment, fecc not supported
  if (pMoveImportParams->m_pFECCBridge != NULL)
    m_pFECCBridge = (pMoveImportParams->m_pFECCBridge);
*/

	if (pMoveImportParams->m_pContentBridge != NULL)
		m_pContentBridge = (pMoveImportParams->m_pContentBridge);

	if (pMoveImportParams->m_pTerminalNumberingManager != NULL)
		m_pTerminalNumberingManager = (pMoveImportParams->m_pTerminalNumberingManager);

	// 3) set communication mode and local capabilities
	m_IsMovedParty = TRUE;
	m_pPartyApi->SetRsrcConfIdForInterface(m_destResourceConfId);

	m_pPartyHWInterface = new CPartyInterface(*((pMoveImportParams->m_pImpConfPartyCntl)->GetPartyCntlResource()));
	m_pPartyHWInterface->SetConfRsrcId(m_destResourceConfId);
	// b) set resourses descriptors that allocated/deallocated at CConf level

	// set to new conf id only after using old conf id for deallocating resources
	m_monitorConfId = pMoveImportParams->m_sysConfId;

	CBridgePartyInitParams* pBrdgPartyInitParams = new CBridgePartyInitParams(m_name, m_pParty,
	                                                                          GetPartyRsrcId(), m_RoomId,
	                                                                          GetInterfaceType(),
	                                                                          NULL, NULL,
	                                                                          m_pBridgeMoveParams->GetAndResetAudioBridgePartyCntlOnImport());
	bool isIvrAfterMove = false;

	m_pConfAppMngrInterface->ConnectPartyAudio(pBrdgPartyInitParams, isIvrAfterMove);

	m_eAudBridgeConnState = eSendOpenInAndOut;
	ON(m_bIsMemberInAudBridge);
	StartTimer(IMPORT_PARTY_CONNECT_TO_AUDIO_BRIDGE_TIMER, 8*SECOND);

	// FECC: -- currently not supported
	// ConnectPartyToFECCBridge();

	WORD refMcuNumber      = 1;
	WORD refTerminalNumber = 1;
	if (m_pTerminalNumberingManager)
	{
		m_pTerminalNumberingManager->allocatePartyNumber(m_pParty, refMcuNumber, refTerminalNumber);
		ON(m_isTerminalNumberingConn);
	}
	else
	{
		PASSERTSTREAM(1, "PartyId:" << GetPartyRsrcId() << " - Invalid pointer");
	}

	if (m_moveType == eMoveIntoIvr)
		ON(isIvrAfterMove);

	POBJDELETE(pBrdgPartyInitParams);
}

//--------------------------------------------------------------------------
// This function ends the import side of the move party scenario successful.
// Audio connected
void CImportIsdnPartyCntl::OnAudConnect(CSegment* pParam)
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
void CImportIsdnPartyCntl::OnTimerConnectToAudioBrdg(CSegment* pParam)
{
	PASSERTSTREAM(1, m_partyConfName);

	// Disconnect the party
	m_pTaskApi->EndImportParty(m_pParty->GetPartyId(), statIllegal);
}
