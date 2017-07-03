#include "IsdnParty.h"
#include "PartyApi.h"
#include "ConfPartyOpcodes.h"
#include "IsdnNetSetup.h"
#include "ConfApi.h"
#include "HardwareInterface.h"
#include "TraceStream.h"
#include "ConfPartyGlobals.h"
#include "AvcToSvcArtTranslator.h"

//--------------------------------------------------------------------------
void PartyIsdnEntryPoint(void* appParam)
{
	CIsdnParty* pPartyTaskApp = new CIsdnParty;
	pPartyTaskApp->Create(*(CSegment*)appParam);
}

PBEGIN_MESSAGE_MAP(CIsdnParty)

	ONEVENT(PARTY_FAULTY_RSRC, ANYCASE,            CIsdnParty::OnPartySendFaultyMfaToPartyCntlAnycase)

	ONEVENT(DISCONNECT,        PARTYSETUP,         CIsdnParty::OnNetDisconnectSetUp)
	ONEVENT(DISCONNECT,        PARTYCONNECTED,     CIsdnParty::OnNetDisconnectConnect)
	ONEVENT(DISCONNECT,        PARTYDISCONNECTING, CIsdnParty::NullActionFunction)      // VNGR-8596

	ONEVENT(NETCONNECT,        PARTYDISCONNECTING, CIsdnParty::NullActionFunction)      // VNGR-8596

	ONEVENT(ENDDISCONNECT,     ANYCASE,            CIsdnParty::OnEndNetDisconnect)

	ONEVENT(LOGICALDELNETCHNL, ANYCASE,            CIsdnParty::OnConfDelLogicalNetCntl)

	ONEVENT(START_AVC_TO_SVC_ART_TRANSLATOR,  IDLE,                    CIsdnParty::OnStartAvcToSvcArtTranslator)
	ONEVENT(START_AVC_TO_SVC_ART_TRANSLATOR,  PARTYSETUP,              CIsdnParty::OnStartAvcToSvcArtTranslator)
	ONEVENT(START_AVC_TO_SVC_ART_TRANSLATOR,  PARTYCONNECTED,          CIsdnParty::OnStartAvcToSvcArtTranslator)



	ONEVENT(REMOVE_AVC_TO_SVC_ART_TRANSLATOR,  PARTYDISCONNECTING,     CIsdnParty::OnRemoveAvcToSvcArtTranslatorAnycase)

PEND_MESSAGE_MAP(CIsdnParty, CParty);

////////////////////////////////////////////////////////////////////////////
//                        CIsdnParty
////////////////////////////////////////////////////////////////////////////
CIsdnParty::CIsdnParty()
{
	m_pNetSetUp      = new CIsdnNetSetup[MAX_CHNLS_IN_PARTY];
	m_pNetRsrcParams = new CRsrcParams[MAX_CHNLS_IN_PARTY];
	for (WORD i = 0; i < MAX_CHNLS_IN_PARTY; i++)
	{
		m_pNetChnlCntl[i] = NULL;
		m_retryNetCall[i] = 0;
	}

	m_byQ931Cause = causDEFAULT_VAL;
	m_conf_has_been_notifyed = FALSE;
	m_pAvcToSvcArtTranslator = NULL;
}

//--------------------------------------------------------------------------
CIsdnParty::~CIsdnParty()
{
	if (m_pNetSetUp)
	{
		delete [] m_pNetSetUp;
		m_pNetSetUp = NULL;
	}

	if (m_pNetRsrcParams)
	{
		delete [] m_pNetRsrcParams;
		m_pNetRsrcParams = NULL;
	}
}

//--------------------------------------------------------------------------
void CIsdnParty::SetPartyResource(CRsrcParams* NetRsrcParams)
{
}

//--------------------------------------------------------------------------
void CIsdnParty::OnNetDisconnect(CSegment* pParam)
{
	WORD seqNum;
	BYTE cause;
	*pParam >> seqNum >> cause;
	m_pConfApi->PartyDisConnect(PARTY_HANG_UP, this);
	UpdateQ931DisCause(cause);
}

//--------------------------------------------------------------------------
// Filters Q.931 Disconnection Cause for party net channels and sends
// UpdateDB to conf
void CIsdnParty::UpdateQ931DisCause(const BYTE cause)
{
	// If the first channel connected well and we have problem with
	// additional channel, we are going to disconnect all channels
	if (m_byQ931Cause == causDEFAULT_VAL /*default cause or not available*/ ||
	    m_byQ931Cause == causNORMAL_CLR_VAL /*normal call clearing procedures*/)
	{
		m_byQ931Cause = cause;
	}

	m_pConfApi->UpdateDB(this, DISQ931, m_byQ931Cause, 1);
}

//--------------------------------------------------------------------------
void CIsdnParty::DisconnectNetChnlCntl(WORD chnl)
{
	if (CPObject::IsValidPObjectPtr(m_pNetChnlCntl[chnl]))
	{
		TRACEINTO << PARTYNAME << ", channel:" << chnl << " - Channel local disconnecting";
		m_pNetChnlCntl[chnl]->Clear();
	}
	else    // net control is already disconnected
	{
		TRACEINTO << PARTYNAME << ", channel:" << chnl << " - Channel already disconnected";
		CPartyApi* pTaskApi = new CPartyApi;
		pTaskApi->CreateOnlyApi(this->GetRcvMbx());
		pTaskApi->SetLocalMbx(this->GetLocalQueue());
		pTaskApi->EndNetDisConnect(chnl, statOK);
		pTaskApi->DestroyOnlyApi();
		POBJDELETE(pTaskApi);
	}
}

//--------------------------------------------------------------------------
WORD CIsdnParty::NetDisconnectCompleted()
{
	for (WORD i = 0; i < MAX_CHNLS_IN_PARTY; i++)
		if (m_pNetChnlCntl[i] != NULL && !m_pNetChnlCntl[i]->IsDisconnected())
			return 0;
	return 1;
}

//--------------------------------------------------------------------------
void CIsdnParty::UpdateNetChnlStatus(WORD chnl, WORD status)
{
	DWORD chnlToggel = 0L;

	if (status) chnlToggel = 0xC0000000;

	chnlToggel |= (chnl + 1);

	m_pConfApi->UpdateDB(this, NETCHNL, chnlToggel, 1);
}

//--------------------------------------------------------------------------
void CIsdnParty::MonitorPartyPhoneNumbers(BYTE channel, CNetSetup* pNetSetUp, BYTE isDailOut)
{
	if (!pNetSetUp)
	{
		TRACESTRFUNC(eLevelError) << PARTYNAME << " - Failed, NULL pointer to CNetSetUp object";
		return;
	}

	WORD numDigits = 0;
	char calledNumber[PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER];
	char callingNumber[PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER];
	CIsdnNetSetup* pIsdnNetSetup = static_cast<CIsdnNetSetup*>(pNetSetUp);

	pIsdnNetSetup->GetCalledNumber(&numDigits, calledNumber);
	pIsdnNetSetup->GetCallingNumber(&numDigits, callingNumber);
	if (isDailOut)
		UpdatePartyPhoneNumbers(channel, callingNumber, calledNumber);
	else
		UpdatePartyPhoneNumbers(channel, calledNumber, callingNumber);
}

//--------------------------------------------------------------------------
void CIsdnParty::UpdatePartyPhoneNumbers(BYTE channel, const char* mcuNumber, const char* partyNumber)
{
	CSegment* pParam = new CSegment;
	*pParam << channel << mcuNumber << partyNumber;

	m_pConfApi->UpdateDB(this, MONITORPHONENUM, TRUE, (WORD)1, pParam);
	POBJDELETE(pParam);
}

//--------------------------------------------------------------------------
void CIsdnParty::OnNetDisconnectConnect(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
	OnNetDisconnect(pParam);
}

//--------------------------------------------------------------------------
void CIsdnParty::OnNetDisconnectSetUp(CSegment* pParam)
{
	WORD seqNum;
	BYTE cause;
	*pParam >> seqNum >> cause;

	TRACEINTO << PARTYNAME << ", channel:" << seqNum << ", cause:" << (WORD)cause;

	if (causNO_CHAN_AVL_VAL == cause)  /* board full case */
	{
		DWORD conn_id = m_pNetSetUp[seqNum].m_net_connection_id;

		if (m_pNetRsrcParams && m_pNetRsrcParams[seqNum].GetRsrcDesc())
			conn_id = m_pNetRsrcParams[seqNum].GetRsrcDesc()->GetConnectionId();

		TRACEINTO << PARTYNAME << ", conn_id:" << conn_id << " - Board is full";

		/* turn on related a 'retry' flag. It'll be used when BOARDFULL_ACK is arrived */
		m_retryNetCall[seqNum] = 1;
		/* send request to AddIsdnPartyCntl */
		m_pConfApi->ReallocateOnBoardFull(GetPartyId(), conn_id);
	}
	else if (m_conf_has_been_notifyed == FALSE)
	{
		m_pConfApi->PartyConnect(GetPartyId(), statIllegal);
		m_conf_has_been_notifyed = TRUE;
	}
}

//--------------------------------------------------------------------------
void CIsdnParty::OnPartySendFaultyMfaToPartyCntlAnycase(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
	m_pConfApi->SendFaultyMfaNoticeToPartyCntl(GetPartyId(), STATUS_FAIL);
}

//--------------------------------------------------------------------------
void CIsdnParty::PartySpecfiedIvrDelay()
{
	// Isdn parties do not require IVR timer of 2 seconds
	DispatchEvent(TIMER_START_IVR);
}

//--------------------------------------------------------------------------
void CIsdnParty::OnConfEstablishCall(CSegment* pParam)
{
	*pParam >> m_PartyRsrcID;

	for (WORD i = 0; i < 1; i++)
		m_pNetSetUp[i].DeSerialize(NATIVE, *pParam);

	m_pNetRsrcParams->DeSerialize(NATIVE, *pParam);
	m_ConfRsrcId = m_pNetRsrcParams->GetConfRsrcId();
}

//--------------------------------------------------------------------------
void CIsdnParty::CleanUp(WORD mode)
{
	TRACEINTO << PARTYNAME << ", PartyId:" << GetPartyId();

	m_state = PARTYDISCONNECTING;
	ON(m_isCleanup);

	if (!mode)
	{
		for (WORD i = 0; i < MAX_CHNLS_IN_PARTY; i++)
			DisconnectNetChnlCntl(i);
	}
	else     // disconnect without net disconnection
	{
		if (CPObject::IsValidPObjectPtr(m_pConfApi))
		{
			STATUS stat = STATUS_OK;
			if (m_byQ931Cause != causDEFAULT_VAL && m_byQ931Cause != causNORMAL_CLR_VAL)
				stat = m_byQ931Cause;

			m_pConfApi->PartyEndDisConnect(GetPartyId(), stat);  // notify conference
		}
		PartySelfKill();
	}
}

//--------------------------------------------------------------------------
void CIsdnParty::OnEndNetDisconnect(CSegment* pParam)
{
	WORD seqNum, status;
	*pParam >> seqNum >> status;

	TRACEINTO << PARTYNAME << ", channel:" << seqNum << ", status:" << status;

	PASSERT(!CPObject::IsValidPObjectPtr(m_pConfApi));

	if (CPObject::IsValidPObjectPtr(m_pConfApi))
	{
		if (m_pNetChnlCntl[seqNum] != NULL)
			m_pConfApi->UpdateDB(this, NETCHNL, seqNum+1, 1);
	}

	if (m_pNetChnlCntl[seqNum] != NULL)
	{
		m_pNetChnlCntl[seqNum]->Destroy();
		POBJDELETE(m_pNetChnlCntl[seqNum]);
	}

	if (NetDisconnectCompleted() && m_isCleanup)
	{
		for (WORD i = 0; i < MAX_CHNLS_IN_PARTY; i++)
		{
			if (m_pNetChnlCntl[i] != NULL)
			{
				m_pNetChnlCntl[i]->Destroy();
				POBJDELETE(m_pNetChnlCntl[i]);
			}
		}

		PartySelfKill();
		if (CPObject::IsValidPObjectPtr(m_pConfApi))
			m_pConfApi->PartyEndDisConnect(GetPartyId(), status); // notify conference
	}
	else
	{
		// notify conference  about all net channels disconnection
		if (NetDisconnectCompleted())
		{
			if (CPObject::IsValidPObjectPtr(m_pConfApi))
			{
				TRACEINTO << PARTYNAME << " - All net channels are disconnected before CLEANUP starts";
				m_pConfApi->PartyEndNetChnlDisConnect(GetPartyId(), status);
			}
		}
	}
}

//--------------------------------------------------------------------------
void CIsdnParty::RemoveNetChannel(WORD remove_index)
{
	// check legal index
	if (remove_index >= MAX_CHNLS_IN_PARTY)
	{
		DBGPASSERT(remove_index);
		return;
	}

	// delete net channel control
	POBJDELETE(m_pNetChnlCntl[remove_index]);

	// copy other channel to previous index
	for (int next_channl = remove_index+1; next_channl < (MAX_CHNLS_IN_PARTY); next_channl++)
	{
		if (m_pNetSetUp[next_channl].m_net_connection_id != 0)
		{
			m_pNetSetUp[next_channl-1] = m_pNetSetUp[next_channl];
			m_pNetSetUp[next_channl].m_net_connection_id = 0;

			m_pNetRsrcParams[next_channl-1] = m_pNetRsrcParams[next_channl];
			// copy pointers
			m_pNetChnlCntl[next_channl-1] = m_pNetChnlCntl[next_channl];
			m_pNetChnlCntl[next_channl]   = NULL;

			m_retryNetCall[next_channl-1] = m_retryNetCall[next_channl];
			m_retryNetCall[next_channl]   = 0;
		}
	}
}

//--------------------------------------------------------------------------
// VNGR-6166
void CIsdnParty::OnConfDelLogicalNetCntl(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
	WORD discause = 0;
	*pParam >> discause;
	ReplayToNetWhenNoResource(NET_CLEAR_REQ, *m_pNetSetUp, discause);
}

//--------------------------------------------------------------------------
void CIsdnParty::ReplayToNetWhenNoResource(DWORD opcode, CIsdnNetSetup& netSetup, WORD discasue)
{
	CNetHardwareInterface netHardwareInterface;
	DWORD rejectId = 0;

	::AllocateRejectID(rejectId);
	CRsrcParams rRsrcParams(rejectId, rejectId, STANDALONE_CONF_ID, eLogical_net);

	netHardwareInterface.SendMsgWithPhysicalInfo(opcode, rRsrcParams, netSetup, discasue);

	// kill the party task
	PartySelfKill();
}
//--------------------------------------------------------------------------
void CIsdnParty::AvcToSvcArtTranslatorConnected(STATUS status)
{
	TRACEINTO << "PartyId: " << GetPartyRsrcID() << ", ConfId:" << GetConfId() << ", status: " << ((status == STATUS_OK) ? "OK" : "FAILED") << " (" << status << ")";
	m_pConfApi->SendAvcToSvcArtTranslatorConnectedToPartyControl(this, status);
}
//--------------------------------------------------------------------------
void CIsdnParty::OnStartAvcToSvcArtTranslator(CSegment* pParam)
{

	POBJDELETE(m_pAvcToSvcArtTranslator);
	m_pAvcToSvcArtTranslator = new CAvcToSvcArtTranslator;

	CRsrcParams artTranslatorRsrcParams;
	CRsrcParams mrmpArtTranslatorRsrcParams;
	DWORD ssrc = 0;
	WORD roomId;

	*pParam >> ssrc;
	*pParam >> roomId;
	artTranslatorRsrcParams.DeSerialize(NATIVE, *pParam);
	mrmpArtTranslatorRsrcParams.DeSerialize(NATIVE, *pParam);

	ConfRsrcID confRsrcId = artTranslatorRsrcParams.GetConfRsrcId();
	PartyRsrcID partyRsrcId = artTranslatorRsrcParams.GetPartyRsrcId();

	CREATE_AVC_TO_SVC_ART_TRANSLATOR_S stCreateAvcToSvcArtTranslator;
	memset(&stCreateAvcToSvcArtTranslator, 0, sizeof(CREATE_AVC_TO_SVC_ART_TRANSLATOR_S));

	stCreateAvcToSvcArtTranslator.confRsrcId = confRsrcId;
	stCreateAvcToSvcArtTranslator.partyRsrcId = partyRsrcId;
	stCreateAvcToSvcArtTranslator.ssrc = ssrc;
	stCreateAvcToSvcArtTranslator.pMfaRsrcParams = &artTranslatorRsrcParams;
	stCreateAvcToSvcArtTranslator.pMrmpRsrcParams = &mrmpArtTranslatorRsrcParams;
	stCreateAvcToSvcArtTranslator.roomId = roomId;

	TRACEINTO << "PartyId: " << partyRsrcId << ", ConfId:" << confRsrcId << ", ssrc: " << ssrc << " ,roomId:" << roomId;

	m_pAvcToSvcArtTranslator->Create(stCreateAvcToSvcArtTranslator);
	m_pAvcToSvcArtTranslator->Connect();
}
//--------------------------------------------------------------------------
void CIsdnParty::AvcToSvcArtTranslatorDisconnected(STATUS statusOnCloseMrmpChannel, STATUS statusOnCloseArt)
{
	TRACEINTO << "PartyId: " << GetPartyRsrcID() << ", ConfId:" << GetConfId()
			  << ", statusOnCloseMrmpChannel: " << ((statusOnCloseMrmpChannel == STATUS_OK) ? "OK" : "FAILED") << " (" << statusOnCloseMrmpChannel << ")"
			  << ", statusOnCloseArt:" << ((statusOnCloseArt == STATUS_OK) ? "OK" : "FAILED") << " (" << statusOnCloseArt << ")";

	POBJDELETE(m_pAvcToSvcArtTranslator);

	STATUS status = STATUS_OK;

	if(statusOnCloseMrmpChannel == STATUS_FAIL || statusOnCloseArt == STATUS_FAIL)
	{
		status = STATUS_FAIL;
	}
	m_pConfApi->SendAvcToSvcArtTranslatorDisconnectedToPartyControl(this, status);
}
//--------------------------------------------------------------------------
void CIsdnParty::OnRemoveAvcToSvcArtTranslatorAnycase(CSegment* pParam)
{
	TRACEINTO << "PartyId: " << GetPartyRsrcID() << ", ConfId:" << GetConfId();
	if(m_pAvcToSvcArtTranslator)
	{
		m_pAvcToSvcArtTranslator->Disconnect();
	}
	else
	{
		m_pConfApi->SendAvcToSvcArtTranslatorDisconnectedToPartyControl(this, STATUS_OK);
	}
}
