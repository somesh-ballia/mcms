#include "IsdnVideoPartyIn.h"
#include "ConfApi.h"
#include "TraceStream.h"
#include "OpcodesMcmsBonding.h"

const WORD TEST_TIMER = 101;

void PartyIsdnVideoInEntryPoint(void* appParam)
{
	CIsdnVideoPartyIn* pPartyTaskApp = new CIsdnVideoPartyIn;
	pPartyTaskApp->Create(*(CSegment*)appParam);
}

PBEGIN_MESSAGE_MAP(CIsdnVideoPartyIn)
	ONEVENT(LOBBYNETIDENT,       IDLE,            CIsdnVideoPartyIn::OnLobbyNetIdentIdle)
	ONEVENT(LOBBYTRANS,          PARTYSETUP,      CIsdnVideoPartyIn::OnLobbyTransferSetup)
	ONEVENT(ESTABLISHCALL,       PARTYSETUP,      CIsdnVideoPartyIn::OnConfEstablishCallSetup)
	ONEVENT(NETCONNECT,          PARTYSETUP,      CIsdnVideoPartyIn::OnNetConnectSetUp)
	ONEVENT(UPDATECHANNELACK,    PARTYSETUP,      CIsdnVideoPartyIn::OnConfAddNetDescSetup)
	ONEVENT(BONDREQCHNL,         PARTYSETUP,      CIsdnVideoPartyIn::OnBondReqChnlSetUp)
	ONEVENT(BONDDISCONNECT,      PARTYSETUP,      CIsdnVideoPartyIn::OnBondDisConnect)
	ONEVENT(BND_END_NEGOTIATION, PARTYSETUP,      CIsdnVideoPartyIn::OnBondEndnegotiationSetup)
	ONEVENT(EXPORT,              PARTYCHANGEMODE, CIsdnVideoPartyIn::OnConfExportChangeMode)
	ONEVENT(REALLOCATERTM_ACK,   PARTYSETUP,      CIsdnVideoPartyIn::OnConfReallocateRtmAck)
PEND_MESSAGE_MAP(CIsdnVideoPartyIn, CIsdnVideoParty);

////////////////////////////////////////////////////////////////////////////
//                        CIsdnVideoPartyIn
////////////////////////////////////////////////////////////////////////////
CIsdnVideoPartyIn::CIsdnVideoPartyIn()
{
	m_pLobbyApi            = NULL;
	m_numOfChannels        = 0;
	m_numChannelsConnected = 0;
	m_DownSpeedStatus      = 0;

	VALIDATEMESSAGEMAP;
}

//--------------------------------------------------------------------------
CIsdnVideoPartyIn::~CIsdnVideoPartyIn()
{
	if (m_pLobbyApi)
		POBJDELETE(m_pLobbyApi);
}

//--------------------------------------------------------------------------
void CIsdnVideoPartyIn::Create(CSegment& appParam)
{
	CParty::Create(appParam);
	m_pLobbyApi = new CLobbyApi;
	m_pLobbyApi->CreateOnlyApi(*m_pCreatorRcvMbx);
}

//--------------------------------------------------------------------------
void CIsdnVideoPartyIn::OnLobbyNetIdentIdle(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
	m_pNetSetUp->DeSerialize(NATIVE, *pParam);
	m_pLobbyApi->PartyIdent(this, 0, statOK);  // Always initial channel
	m_state = PARTYSETUP;
}

//--------------------------------------------------------------------------
void CIsdnVideoPartyIn::OnConfEstablishCallSetup(CSegment* pParam)
{
	*pParam >> m_PartyRsrcID;
	*pParam >> m_mcuNum;
	*pParam >> m_termNum;

	TRACEINTO << PARTYNAME << ", PartyId:" << m_PartyRsrcID << ", TerminalNum:" << m_termNum << ", mcuNum:" << m_mcuNum;

	m_pNetSetUp[0].DeSerialize(NATIVE, *pParam);

	*pParam >> m_numOfChannels;

	PASSERTSTREAM_AND_RETURN(m_numOfChannels > MAX_CHNLS_IN_PARTY, "NumOfChannels:" << m_numOfChannels << " - Invalid channels number");

	TRACEINTO << "NumOfChannels:" << m_numOfChannels;

	for (WORD j = 0; j < m_numOfChannels; j++)
		m_pNetRsrcParams[j].DeSerialize(NATIVE, *pParam);

	// deserialize the CIsdnPartyRsrcDesc
	CIsdnPartyRsrcDesc pPartyRsrcDesc;
	pPartyRsrcDesc.DeSerialize(NATIVE, *pParam);
	m_ConfRsrcId = pPartyRsrcDesc.GetConfRsrcId();

	m_pTargetComMode->DeSerialize(NATIVE, *pParam);

	CCapH320 localCap;
	localCap.DeSerialize(NATIVE, *pParam);

	*pParam >> m_RoomId;

	// fill m_pNetSetUp array from the CIsdnPartyRsrcDesc
	m_pNetChnlCntl[0] = new CNetChnlCntl;
	m_pNetChnlCntl[0]->Create(m_pNetRsrcParams[0], m_pNetSetUp[0], this, 0, DIALIN);
	m_pNetChnlCntl[0]->StartNetCntl(); // open UDP ports

	CRsrcParams muxRsrcParams;
	WORD mux_desc_found = pPartyRsrcDesc.GetRsrcParams(muxRsrcParams, eLogical_mux);
	PASSERTSTREAM_AND_RETURN(!mux_desc_found, "MUX descriptor is not valid");

	m_state = PARTYSETUP;  // TODO: check states

	const char* phoneNumber = (const char*)pPartyRsrcDesc.GetTmpPhoneNumber();
	if (strlen(phoneNumber) > BND_MAX_PHONE_LEN)
	{
		TRACEINTO << "PhoneNumber:" << phoneNumber << " - Phone number length is too large, send to bonding just last 7 digits";
		phoneNumber = &phoneNumber[strlen(phoneNumber) - BND_MAX_PHONE_LEN];
	}

	m_pBndCntl = new CBondingCntl(DIALIN, m_numOfChannels, (char*)phoneNumber, muxRsrcParams, this);

	m_pMuxCntl = new CMuxCntl(muxRsrcParams, this);
	m_pMuxCntl->SetLocalCaps(localCap);

	m_pBndMuxCntl = new CBondingMuxCntl(m_pBndCntl, m_pMuxCntl, muxRsrcParams, this);

	CCommConf* pCommConf = NULL;
	if (m_pParty)
		  pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
	else
		  PASSERT_AND_RETURN(1);

	if (pCommConf && m_pParty)
	{
		CConfParty* pConfParty = pCommConf->GetCurrentParty(m_pParty->GetMonitorPartyId());
		if (pConfParty)
		{
			//Send Info to CDR for ISDN dial in
			WORD dummy;
			char initial_number[PHONE_NUMBER_DIGITS_LEN+1];
			m_pNetSetUp[0].GetCalledNumber(&dummy, initial_number);

			pConfParty->SetCorrelationId(GenerateCorrelationId(initial_number));
			pCommConf->PartyCorrelationDataToCDR(pConfParty->GetName(), m_pParty->GetMonitorPartyId(), pConfParty->GetCorrelationId());
		}
	}
}

//--------------------------------------------------------------------------
void CIsdnVideoPartyIn::OnNetConnectSetUp(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;

	WORD seqNum, status;
	BYTE cause = 0;
	*pParam >> seqNum >> status >> cause;

	if (status)
	{
		TRACEINTO << PARTYNAME << ", SeqNum:" << seqNum << ", Status:" << status << ", Cause:" << cause << " - Bad status, disconnect party";

		if (m_pConfApi)
			m_pConfApi->UpdateDB(this, DISCAUSE, NO_NET_CONNECTION, 1);

		UpdateQ931DisCause(cause);
		if (m_conf_has_been_notifyed == FALSE)
		{
			if (m_pConfApi)
				m_pConfApi->PartyConnect(GetPartyId(), NO_NET_CONNECTION);
			m_conf_has_been_notifyed = TRUE;
		}
		return;
	}

	/* update net channel status in EMA */
	DWORD chnlToggle = 0xC0000000;
	chnlToggle |= (seqNum + 1);

	if (m_pConfApi)
		m_pConfApi->UpdateDB(this, NETCHNL, chnlToggle, 1);

	m_numChannelsConnected++;
	if (m_numChannelsConnected == 1)      /* if this is an initial channel */
	{
		TRACEINTO << PARTYNAME << " - Connect initial channel";
		m_pBndCntl->Connect();
	}
	else     /* this is an additional bonding channel */
	{
		m_pBndCntl->AddChannel();
		if (m_numChannelsConnected < m_numOfChannels) // not the last additional channel
		{
			TRACEINTO << PARTYNAME << " - Waiting for more additional channels";
		}
		else
		{
			TRACEINTO << PARTYNAME << " - All channels connected";
			if (m_pConfApi)
				m_pConfApi->FreeTmpPhoneNumber(GetPartyId(), m_name);
		}
	}
}

//--------------------------------------------------------------------------
void CIsdnVideoPartyIn::OnBondEndnegotiationSetup(CSegment* pParam)
{
	BYTE  need_reallocate = 0;
	DWORD number_of_channels;

	*pParam >> need_reallocate >> number_of_channels;

	TRACEINTO << PARTYNAME << ", NewNumOfChannels:" << number_of_channels << " , OldNumOfChannels:" << m_numOfChannels << " , NeedReallocate:" << (int)need_reallocate;

	if (m_numOfChannels != number_of_channels)
	{
		m_numOfChannels = number_of_channels;
		if (CPObject::IsValidPObjectPtr(m_pTargetComMode))
			m_pTargetComMode->SetXferMode(number_of_channels, 1);
	}

	if (need_reallocate)  // if we reallocate RTM we can dial inly after Ack (means shared memory updated)
		m_pConfApi->ReallocateBondingChannels(GetPartyId(), number_of_channels);
}

//--------------------------------------------------------------------------
void CIsdnVideoPartyIn::OnBondReqChnlSetUp(CSegment* pParam)
{
	WORD  remote_restType;
	DWORD dummy;
	WORD  remote_additional_channels;
	BYTE* phone_ptr;

	TRACEINTO << PARTYNAME;

	*pParam >> remote_additional_channels >> dummy >> remote_restType;
	phone_ptr = (BYTE*)dummy;     // points to phone record of bndcntl

	char msg[128];
	if (remote_additional_channels != m_numOfChannels-1)
	{
		TRACESTRFUNC(eLevelError) << "remote_additional_channels:" << remote_additional_channels << ", additional_channels:" << m_numOfChannels-1;
		DBGPASSERT(1);
	}

	if (m_numOfChannels > MAX_CHNLS_IN_PARTY)
	{
		TRACESTRFUNC(eLevelError) << "additional_channels:" << m_numOfChannels-1;
		DBGPASSERT(1);
	}

	if (m_numOfChannels != m_pTargetComMode->GetNumB0Chnl())
	{
		TRACESTRFUNC(eLevelError) << "additional_channels:" << m_numOfChannels-1;
		DBGPASSERT(1);
	}
	// If at least one additional channel is already connected, then
	// for each connected channel we connect the TS of the Net to the BONDING,
	// and then send an 'AddChnl' request to the EMBEDDED.
	// (at a call width of 1 initial ch + the number of additional channels)
}

//--------------------------------------------------------------------------
void CIsdnVideoPartyIn::OnConfAddNetDescSetup(CSegment* pParam)
{
	CIsdnNetSetup netSetUp;
	WORD OldSeqNum;

	DWORD monitor_conf_id, monitor_party_id, connection_id, status, seqNum;

	*pParam >> monitor_conf_id >> monitor_party_id >> connection_id >> status >> seqNum;
	TRACEINTO << PARTYNAME
	          << ", MonitorConfId:"  << monitor_conf_id
	          << ", MonitorPartyId:" << monitor_party_id
	          << ", ConnectionId:"   << connection_id
	          << ", Status:"         << status
	          << ", SeqNum:"         << seqNum;

	if (0 != status)
	{
		if (m_conf_has_been_notifyed == FALSE)
		{
			m_pConfApi->PartyConnect(GetPartyId(), status);
			m_conf_has_been_notifyed = TRUE;
		}
		return;
	}

	netSetUp.DeSerialize(NATIVE, *pParam);
	// test if legal additional channel
	if (m_numChannelsConnected >= m_numOfChannels) // party already full connected
	{
		TRACESTRFUNC(eLevelError) << "NumOfChannels:" << m_numOfChannels << ", NumChannelsConnected:" << m_numChannelsConnected << " - Additional channel received after all channels connected";
		DBGPASSERT(1);
	}

	if (seqNum > m_numOfChannels) // seq number is index from 0 to m_numOfChannels
	{
		TRACESTRFUNC(eLevelError) << "NumOfChannels:" << m_numOfChannels << ", SeqNum:" << seqNum << " - Additional channel has illegal Seq number";
		DBGPASSERT(1);
	}

	CRsrcParams tmp(connection_id, monitor_party_id, monitor_conf_id, eLogical_net);
	m_pNetRsrcParams[seqNum] = tmp;
	// create net channel control and connect the channel
	m_pNetSetUp[seqNum] = netSetUp;

	m_pNetChnlCntl[seqNum] = new CNetChnlCntl;
	m_pNetChnlCntl[seqNum]->Create(m_pNetRsrcParams[seqNum], m_pNetSetUp[seqNum], this, seqNum, DIALIN);

	m_pNetChnlCntl[seqNum]->StartNetCntl(); // rons
	MonitorPartyPhoneNumbers((BYTE)seqNum, &m_pNetSetUp[seqNum], (BYTE)0);
}

//--------------------------------------------------------------------------
void CIsdnVideoPartyIn::OnLobbyTransferSetup(CSegment* pParam)
{
	CSegment rspMsg;
	WORD mode = 0xFFFF;
	WORD returnSyncStatus = 0;

	*pParam >> mode;

	TRACEINTO << PARTYNAME << ", Mode:" << mode;

	m_pConfApi = new CConfApi();
	m_pConfApi->CreateOnlyApi(*m_pConfRcvMbx, NULL, NULL, 1);

	switch (mode)
	{
		case PARTYTRANSFER:
		{
			// sync call to conf
			WORD status = m_pConfApi->AddInParty(m_pNetSetUp, this, *m_pRcvMbx, m_name, PARTY_TRANSFER_TOUT, rspMsg); // TODO: check in conf the AddInParty Func
			if (status == 0)
				rspMsg >> returnSyncStatus;

			if (status)
			{
				PASSERTSTREAM(1, "Status:" << status);
				m_pLobbyApi->PartyTransfer(this, statTout);
				m_state = PARTYDISCONNECTING;

				if (m_pConfApi->TaskExists() != NO)
					DBGPASSERT(1);
			}
			else if (returnSyncStatus)
			{
				PASSERTSTREAM(1, "Status:" << status << " - Export party failed");
				m_pLobbyApi->PartyTransfer(this, returnSyncStatus);
				m_state = PARTYDISCONNECTING;
			}
			else
			{
				TRACEINTO << PARTYNAME << " - Export party succeeded";
				m_pLobbyApi->PartyTransfer(this, statOK);
			}
			break;
		}

		default:
		{
			PASSERT(1);
			break;
		}
	}
}

//--------------------------------------------------------------------------
void CIsdnVideoPartyIn::OnBondDisConnect(CSegment* pParam)
{
	DWORD bonding_status;

	*pParam >> bonding_status;

	TRACEINTO << PARTYNAME << ", Status:" << bonding_status;

// m_pConfApi->PartyConnect(this,*m_pRmtCap,G728,bonding_status); //ask conf to disconnect
	if (m_conf_has_been_notifyed == FALSE)
	{
		m_pConfApi->PartyConnect(GetPartyId(), bonding_status);
		m_conf_has_been_notifyed = TRUE;
	}

	if (m_pConfApi)
		m_pConfApi->FreeTmpPhoneNumber(GetPartyId(), m_name);
}

//--------------------------------------------------------------------------
void CIsdnVideoPartyIn::OnTestTimer(CSegment* pParam)
{
	CIsdnVideoParty::OnBondAlignedSetup(pParam);
}

//--------------------------------------------------------------------------
void CIsdnVideoPartyIn::OnConfExportChangeMode(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
	m_state = PARTYCONNECTED;
	OnConfExport(pParam);
}

//--------------------------------------------------------------------------
void CIsdnVideoPartyIn::OnConfReallocateRtmAck(CSegment* pParam)
{
	CIsdnPartyRsrcDesc pPartyRsrcDesc;
	pPartyRsrcDesc.DeSerialize(NATIVE, *pParam);
	BYTE allocationFailed;
	*pParam >> allocationFailed;

	TRACEINTO << PARTYNAME << ", Status:" << (int)allocationFailed;

	if (allocationFailed)
	{
		/* it can happen only in the case when a rate increment was requested and we don't support this */
		DBGPASSERT(1);
		m_pConfApi->UpdateDB(this, DISCAUSE, NET_PORT_DEFICIENCY, 1);
		if (m_conf_has_been_notifyed == FALSE)
		{
			m_pConfApi->PartyConnect(GetPartyId(), NET_PORT_DEFICIENCY);
			m_conf_has_been_notifyed = TRUE;
		}
		return;
	}
}

