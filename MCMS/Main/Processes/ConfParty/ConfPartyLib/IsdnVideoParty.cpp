#include "ConfApi.h"
#include "ConfDef.h"
#include "DataTypes.h"
#include "H221.h"
#include "IsdnVideoParty.h"
#include "NonStandardCaps.h"
#include "TraceStream.h"
#include "OpcodesMcmsBonding.h"
#include "ConfPartyGlobals.h"
#include "IVRCntl.h"
#include "UnifiedComMode.h"
#include "PrettyTable.h"

// Mux ack for COntent On/Off tout
const WORD MUXCONTENTACKTOUT      = 20;
const WORD MUXEVACUATEACKTOUT     = 21;
const WORD TIMER_BETWEEN_VCU_REC  = 22;

PBEGIN_MESSAGE_MAP(CIsdnVideoParty)

	ONEVENT(CONFDISCONNECT,                 ANYCASE,            CIsdnVideoParty::OnConfDisconnectAnyState)
	ONEVENT(DELNETCHNL,                     PARTYCONNECTED,     CIsdnVideoParty::OnConfDelNetCntl)
	ONEVENT(DELNETCHNL,                     PARTYSETUP,         CIsdnVideoParty::OnConfDelNetCntl)
	ONEVENT(DELNETCHNL,                     PARTYCHANGEMODE,    CIsdnVideoParty::OnConfDelNetCntl)

	ONEVENT(RMTH230,                        PARTYSETUP,         CIsdnVideoParty::OnMuxH230SetUp)
	ONEVENT(RMTH230,                        PARTYCHANGEMODE,    CIsdnVideoParty::OnMuxH230ChangeMode)
	ONEVENT(RMTH230,                        PARTYCONNECTED,     CIsdnVideoParty::OnMuxH230Connect)
	ONEVENT(RMTH230,                        PARTYDISCONNECTING, CIsdnVideoParty::NullActionFunction)


	ONEVENT(ENDH221CON,                     PARTYSETUP,         CIsdnVideoParty::OnMuxEndH320ConSetUp)
	ONEVENT(MSYNCFOUND,                     PARTYSETUP,         CIsdnVideoParty::OnMuxSyncInitChnl)

	ONEVENT(CONFCHANGEMODE,                 PARTYSETUP,         CIsdnVideoParty::OnConfChangeMode)
	ONEVENT(CONFCHANGEMODE,                 PARTYCONNECTED,     CIsdnVideoParty::OnConfChangeMode)
	ONEVENT(CONFCHANGEMODE,                 PARTYCHANGEMODE,    CIsdnVideoParty::OnConfChangeMode)

	ONEVENT(CONFEXNGCAP,                    PARTYCHANGEMODE,    CIsdnVideoParty::OnConfExchangeCap)
	ONEVENT(CONFEXNGCAP,                    PARTYCONNECTED,     CIsdnVideoParty::OnConfExchangeCap)
	ONEVENT(CONFUPDATELOCALCAPS,            ANYCASE,            CIsdnVideoParty::OnConfUpdateLocalCaps)

	ONEVENT(DISCONNECTENC,                  PARTYSETUP,         CIsdnVideoParty::OnEncDisconnect)
	ONEVENT(DISCONNECTENC,                  PARTYCHANGEMODE,    CIsdnVideoParty::OnEncDisconnect)
	ONEVENT(DISCONNECTENC,                  PARTYCONNECTED,     CIsdnVideoParty::OnEncDisconnect)
	ONEVENT(DISCONNECTENC,                  PARTYDISCONNECTING, CIsdnVideoParty::NullActionFunction)

	ONEVENT(RMTXFRMODE,                     PARTYSETUP,         CIsdnVideoParty::OnMuxRmtXfrMode)
	ONEVENT(RMTXFRMODE,                     PARTYCHANGEMODE,    CIsdnVideoParty::OnMuxRmtXfrMode)
	ONEVENT(RMTXFRMODE,                     PARTYCONNECTED,     CIsdnVideoParty::OnMuxRmtXfrMode)
	ONEVENT(RMTXFRMODE,                     PARTYDISCONNECTING, CIsdnVideoParty::NullActionFunction)

	ONEVENT(STARTCOMMANDH230SEQ,            PARTYSETUP,         CIsdnVideoParty::OnMuxStartCommH230Seq)
	ONEVENT(STARTCOMMANDH230SEQ,            PARTYCHANGEMODE,    CIsdnVideoParty::OnMuxStartCommH230Seq)
	ONEVENT(STARTCOMMANDH230SEQ,            PARTYCONNECTED,     CIsdnVideoParty::OnMuxStartCommH230Seq)

	ONEVENT(AUDIOSILANCE,                   PARTYSETUP,         CIsdnVideoParty::OnMuxSendSilence)
	ONEVENT(AUDIOSILANCE,                   PARTYCHANGEMODE,    CIsdnVideoParty::OnMuxSendSilence)
	ONEVENT(AUDIOSILANCE,                   PARTYCONNECTED,     CIsdnVideoParty::OnMuxSendSilence)

	ONEVENT(AUDVALID,                       PARTYSETUP,         CIsdnVideoParty::OnAudBrdgValidation)
	ONEVENT(AUDVALID,                       PARTYCONNECTED,     CIsdnVideoParty::OnAudBrdgValidation)
	ONEVENT(AUDVALID,                       PARTYCHANGEMODE,    CIsdnVideoParty::OnAudBrdgValidation)

	ONEVENT(RMTCAP,                         PARTYSETUP,         CIsdnVideoParty::OnMuxRmtCapSetUp)
	ONEVENT(RMTCAP,                         PARTYCHANGEMODE,    CIsdnVideoParty::OnMuxRmtCapConnect)
	ONEVENT(RMTCAP,                         PARTYCONNECTED,     CIsdnVideoParty::OnMuxRmtCapConnect)
	ONEVENT(RMTCAP,                         PARTYDISCONNECTING, CIsdnVideoParty::NullActionFunction)

	ONEVENT(SYNCLOSS,                       PARTYSETUP,         CIsdnVideoParty::NullActionFunction)
	ONEVENT(SYNCLOSS,                       PARTYCHANGEMODE,    CIsdnVideoParty::OnMuxSyncChangeMode)
	ONEVENT(SYNCLOSS,                       PARTYCONNECTED,     CIsdnVideoParty::OnMuxSyncConnect)
	ONEVENT(SYNCLOSS,                       PARTYDISCONNECTING, CIsdnVideoParty::NullActionFunction)

	ONEVENT(MEDIAERR,                       PARTYCONNECTED,     CIsdnVideoParty::OnMcuMediaErrConnect)
	ONEVENT(ENDCHAGEMODE,                   ANYCASE,            CIsdnVideoParty::NullActionFunction)

	ONEVENT(VIDREFRESH,                     PARTYSETUP,         CIsdnVideoParty::NullActionFunction)
	ONEVENT(VIDREFRESH,                     PARTYCHANGEMODE,    CIsdnVideoParty::OnVidBrdgRefresh)
	ONEVENT(VIDREFRESH,                     PARTYCONNECTED,     CIsdnVideoParty::OnVidBrdgRefresh)
	ONEVENT(VIDREFRESH,                     PARTYDISCONNECTING, CIsdnVideoParty::NullActionFunction)

	ONEVENT(SEND_H239_VIDEO_CAPS,           PARTYCONNECTED,     CIsdnVideoParty::OnVidBrdgH239ReCap)
	ONEVENT(SEND_H239_VIDEO_CAPS,           PARTYCHANGEMODE,    CIsdnVideoParty::OnVidBrdgH239ReCap)
	ONEVENT(SEND_H239_VIDEO_CAPS,           PARTYDISCONNECTING, CIsdnVideoParty::NullActionFunction)

	ONEVENT(NUMBERINGMESSAGE,               ANYCASE,            CIsdnVideoParty::OnBridgeNumberingMessage)

	ONEVENT(ALLRMTCAPSRECEIVED,             ANYCASE,            CIsdnVideoParty::OnMuxReceivedPartyFullCapSet)

	ONEVENT(AMC_CONTENT_VIDEO_MODE,         PARTYCONNECTED,     CIsdnVideoParty::OnContentBrdgAmcVideoMode)
	ONEVENT(AMC_CONTENT_VIDEO_MODE,         PARTYCHANGEMODE,    CIsdnVideoParty::OnContentBrdgAmcVideoMode)
	ONEVENT(AMC_CONTENT_VIDEO_MODE,			PARTYSETUP,			CIsdnVideoParty::OnContentBrdgAmcVideoPartySetup)

	ONEVENT(AMC_LOGICAL_CHANNEL_INACTIVE,   PARTYCONNECTED,     CIsdnVideoParty::OnContentBrdgAmcLogicalChannelInactive)
	ONEVENT(AMC_LOGICAL_CHANNEL_INACTIVE,   PARTYCHANGEMODE,    CIsdnVideoParty::OnContentBrdgAmcLogicalChannelInactive)

	ONEVENT(AMC_MCS,                        PARTYCONNECTED,     CIsdnVideoParty::OnContentBrdgAmcMCS)
	ONEVENT(AMC_MCS,                        PARTYCHANGEMODE,    CIsdnVideoParty::OnContentBrdgAmcMCS)

	ONEVENT(UPDATE_PRESENTATION_OUT_STREAM  ,ANYCASE            ,CIsdnVideoParty::OnPartyCntlUpdatePresentationOutStream)
	ONEVENT(PRESENTATION_OUT_STREAM_UPDATED ,ANYCASE            ,CIsdnVideoParty::OnPartyUpdatedPresentationOutStream)

	ONEVENT(BND_REMOTE_LOCAL_ALIGNMENT,     PARTYSETUP,         CIsdnVideoParty::OnBondAlignedSetup)
	ONEVENT(UPDATERSRCCONFID,               ANYCASE,            CIsdnVideoParty::OnPartyUpdateConfRsrcIdForInterfaceAnycase)

	ONEVENT(CONTENTRATECHANGEDONE           ,PARTYCHANGEMODE    ,CIsdnVideoParty::OnContentRateChangeDone)
	ONEVENT(CONTENTRATECHANGEDONE           ,PARTYCONNECTED     ,CIsdnVideoParty::OnContentRateChangeDone)

	ONEVENT(CONFCONTENTTOKENMSG,            PARTYCONNECTED,     CIsdnVideoParty::OnContentBrdgTokenMsg)
	ONEVENT(CONFCONTENTTOKENMSG,            PARTYCHANGEMODE,    CIsdnVideoParty::OnContentBrdgTokenMsg)

	ONEVENT(HW_CONTENT_ON_OFF_ACK,          PARTYCONNECTED,     CIsdnVideoParty::OnMuxAckForContentOnOff)
	ONEVENT(HW_CONTENT_ON_OFF_ACK,          PARTYCHANGEMODE,    CIsdnVideoParty::OnMuxAckForContentOnOff)
	ONEVENT(HW_CONTENT_ON_OFF_ACK,          PARTYDISCONNECTING, CIsdnVideoParty::OnMuxAckForContentOnOff)

	ONEVENT(RTP_EVACUATE_ON_OFF_ACK,        PARTYCONNECTED,     CIsdnVideoParty::OnMuxAckForEvacuateContentStream)
	ONEVENT(RTP_EVACUATE_ON_OFF_ACK,        PARTYCHANGEMODE,    CIsdnVideoParty::OnMuxAckForEvacuateContentStream)
	ONEVENT(RTP_EVACUATE_ON_OFF_ACK,        PARTYDISCONNECTING, CIsdnVideoParty::NullActionFunction)

	ONEVENT(CONTENTFREEZEPIC,               PARTYCONNECTED,     CIsdnVideoParty::OnContentBrdgFreezePic)
	ONEVENT(CONTENTFREEZEPIC,               PARTYCHANGEMODE,    CIsdnVideoParty::OnContentBrdgFreezePic)

	ONEVENT(CONTENTVIDREFRESH,              PARTYCONNECTED,     CIsdnVideoParty::OnContentBrdgRefreshVideo)
	ONEVENT(CONTENTVIDREFRESH,              PARTYCHANGEMODE,    CIsdnVideoParty::OnContentBrdgRefreshVideo)
	ONEVENT(CONTENTVIDREFRESH,				PARTYSETUP,			CIsdnVideoParty::NullActionFunction)

	ONEVENT(TIMER_BETWEEN_VCU_REC,          ANYCASE,            CIsdnVideoParty::OnVidBrdgRefresh)

	ONEVENT(SMART_RECOVERY,                 ANYCASE,            CIsdnVideoParty::OnSmartRecovery)
	ONEVENT(VIDEO_READY_FOR_IVR,            ANYCASE,            CIsdnVideoParty::OnVideoReadyForIvr)

	ONEVENT(SYNCTIMER,                      PARTYCHANGEMODE,    CIsdnVideoParty::OnMuxSyncTimer)
	ONEVENT(SYNCTIMER,                      PARTYCONNECTED,     CIsdnVideoParty::OnMuxSyncTimer)
	ONEVENT(SYNCTIMER,                      PARTYDISCONNECTING, CIsdnVideoParty::NullActionFunction)

	ONEVENT(FREEZPIC,                       PARTYSETUP,         CIsdnVideoParty::NullActionFunction)
	ONEVENT(FREEZPIC,                       PARTYCHANGEMODE,    CIsdnVideoParty::OnPartyFreezePic)
	ONEVENT(FREEZPIC,                       PARTYCONNECTED,     CIsdnVideoParty::OnPartyFreezePic)
	ONEVENT(FREEZPIC,                       PARTYDISCONNECTING, CIsdnVideoParty::NullActionFunction)

	ONEVENT(SEND_MMS,                       PARTYSETUP,         CIsdnVideoParty::NullActionFunction)
	ONEVENT(SEND_MMS,                       PARTYCHANGEMODE,    CIsdnVideoParty::OnPartyCntlSendMMSOnOff)
	ONEVENT(SEND_MMS,                       PARTYCONNECTED,     CIsdnVideoParty::OnPartyCntlSendMMSOnOff)
	ONEVENT(SEND_MMS,                       PARTYDISCONNECTING, CIsdnVideoParty::NullActionFunction)

	ONEVENT(SEND_MCC,                       PARTYSETUP,         CIsdnVideoParty::NullActionFunction)
	ONEVENT(SEND_MCC,                       PARTYCHANGEMODE,    CIsdnVideoParty::OnPartyCntlSendMCCOnOff)
	ONEVENT(SEND_MCC,                       PARTYCONNECTED,     CIsdnVideoParty::OnPartyCntlSendMCCOnOff)
	ONEVENT(SEND_MCC,                       PARTYDISCONNECTING, CIsdnVideoParty::NullActionFunction)

PEND_MESSAGE_MAP(CIsdnVideoParty,CIsdnParty);


////////////////////////////////////////////////////////////////////////////
//                        CIsdnVideoParty
////////////////////////////////////////////////////////////////////////////
CIsdnVideoParty::CIsdnVideoParty() : CIsdnParty(),
	m_pBndCntl(NULL),
	m_pMuxCntl(NULL),
	m_pBndMuxCntl(NULL)
{
	m_pTargetComMode                        = new CComMode;
	m_pCurrentComMode                       = new CComMode;
	m_pH239TokenMsgMngr                     = new CTokenMsgMngr;
	m_delayIVR                              = FALSE;
	m_lastContentRateFromMasterForThisToken = 0;     // for the content coming from our side (slave)
	m_isContentSpeakerOnSlave               = NO;
	m_isSlaveFinishCM                       = NO;
	m_nodeType                              = CASCADE_MODE_NONE;

	m_pH239TokenMsgMngr->EnableTokenMsgMngr();
}

//--------------------------------------------------------------------------
CIsdnVideoParty::~CIsdnVideoParty()
{
	POBJDELETE(m_pTargetComMode);
	POBJDELETE(m_pCurrentComMode);
	POBJDELETE(m_pBndCntl);
	POBJDELETE(m_pMuxCntl);
	POBJDELETE(m_pBndMuxCntl);
	POBJDELETE(m_pH239TokenMsgMngr);
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnConfDisconnectAnyState(CSegment* pParam)
{
	// rons - JUST FOR PARTY-IN, for party out freeTmpPhoneNumber will do nothing
	if (m_pConfApi)
		m_pConfApi->FreeTmpPhoneNumber(GetPartyId(), m_name);

	TRACEINTO << PARTYNAME;

	if (m_pBndCntl)
		m_pBndCntl->Disconnect();

	if (m_pMuxCntl)
		m_pMuxCntl->Disconnect();

	CleanUp();
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnConfDelNetCntl(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;

	m_state = PARTYDISCONNECTING;
	for (WORD i = 0; i < MAX_CHNLS_IN_PARTY; i++)
		DisconnectNetChnlCntl(i);
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnConfChangeMode(CSegment* pParam)
{
	if (PARTYSETUP == GetState())
	{
		FTRACESTRFUNC(eLevelError) << PARTYNAME << ", state:PARTYSETUP - Failed, illegal party state";
		return;
	}

	TRACEINTO << PARTYNAME;

	CComMode newScm;
	newScm.DeSerialize(NATIVE, *pParam);
	newScm.Dump(1);

	*m_pTargetComMode = newScm;

	if (IsPartyLinkToMaster()) // Link Party connected to the Slave conference and serve as LinkToMaster
	{
		if (m_pTargetComMode->GetContentModeContentRate() != 0)
		{
			if (NO == m_isSlaveFinishCM)
			{
				return;
			}
		}
	}

	if (m_pMuxCntl)
		m_pMuxCntl->SetXmitComMode(*m_pTargetComMode);
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnConfExchangeCap(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;

	CCapH320 localCap;
	localCap.DeSerialize(NATIVE, *pParam);

	if (m_pMuxCntl)
		m_pMuxCntl->ExchangeCap(localCap); // , cap->OnDataVidCap(H263_2000));

}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnConfUpdateLocalCaps(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;

	CCapH320 localCaps;
	localCaps.DeSerialize(NATIVE, *pParam);

	if (m_pMuxCntl)
		m_pMuxCntl->SetLocalCaps(localCaps);
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnBondAlignedSetup(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;

	if (!m_voice)
	{
		// MMS Control
		WORD result = 1; // CheckIfMMSIsEnabled(pCommConf);//olga
		if (result)
			m_pMuxCntl->SetIsMmsEnabled(1);

		// start H320 connection on ALL bonding channels
		m_pMuxCntl->Connect(*m_pTargetComMode, m_mcuNum, m_termNum);
	}
}

//--------------------------------------------------------------------------/
void CIsdnVideoParty::SetNetSetup(CIsdnPartyRsrcDesc& pPartyRsrcDesc)
{
	/* fill m_pNetSetUp array from the CIsdnPartyRsrcDesc */
	std::vector<CIsdnSpanOrderPerConnection*>* ConnectionVector = pPartyRsrcDesc.GetSpanOrderPerConnectionVector();
	WORD connectionSize   = ConnectionVector ? ConnectionVector->size() : 0;

	TRACEINTO << PARTYNAME;

	CPrettyTable<int, WORD, DWORD, const char*> tbl("Index", "BoardId", "ConnectionId", "Span order");

	for (WORD i = 0; i < connectionSize; i++)
	{
		CIsdnSpanOrderPerConnection* connection_i  = ConnectionVector->at(i);
		DWORD connection_id = connection_i->GetConnectionId();
		WORD  board_id      = connection_i->GetBoardId();

		if (connection_id == 0)
			break;

		std::vector<WORD>* spansOrderVector = connection_i->GetSpansListVector();
		WORD spansOrderSize = spansOrderVector ? spansOrderVector->size() : 0;

		/* initialize the current CIsdnNetSetup from the first element */
		m_pNetSetUp[i] = m_pNetSetUp[0];
		/* fill boardID, connectionID and spans order from the CIsdnSpanOrderPerBoard object */
		m_pNetSetUp[i].m_boardId           = board_id;
		m_pNetSetUp[i].m_net_connection_id = connection_id;

		std::ostringstream msg;

		for (WORD s = 0; s < spansOrderSize; s++)
		{
			WORD span = spansOrderVector->at(s);
			m_pNetSetUp[i].m_spanId[s] = span;

			msg << span;
			if (s != spansOrderSize-1)
				msg << ",";
		}
		tbl.Add(i, board_id, connection_id, msg.str().c_str());
	}

	TRACEINTO << PARTYNAME << tbl.Get();
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnMuxEndH320ConSetUp(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;

	CCapH320 rmtCap;
	rmtCap.DeSerialize(NATIVE, *pParam);

	CComMode newScm;
	newScm.DeSerialize(NATIVE, *pParam);

	WORD status;
	*pParam >> status;

	BYTE isEncryptionSetupDone;
	*pParam >> isEncryptionSetupDone;
	StartTimer(VCUTOUT, 10*SECOND);

	// send indication to CAddPartyCntl
	m_pConfApi->IsdnPartyConnect(GetPartyId(), rmtCap, newScm, status, isEncryptionSetupDone);

	if (isEncryptionSetupDone)
		StartIvr();
	else
		m_delayIVR = TRUE;
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnMuxSyncInitChnl(CSegment* pParam)
{
	/* this event is sent from the mux control after END_INIT_COMM */
	TRACEINTO << PARTYNAME;

	if (!m_voice)
	{
		// send indication to CAddPartyCntl
		m_pConfApi->PartyEndInitCom(GetPartyId());

		CCapH320 rmtCap;
		rmtCap.DeSerialize(NATIVE, *pParam);

		CSegment rmtCapSeg;
		rmtCap.Serialize(NATIVE, rmtCapSeg);

		m_pConfApi->UpdateDB(this, RMOTCAP, (DWORD)0, 1, &rmtCapSeg);

		// assign terminal number to party (send TIA to REMOTE EP)
		CSegment* pSeg   = new CSegment;
		BYTE      opcode = TIA|Attr001;
		*pSeg << opcode << (BYTE)m_mcuNum << (BYTE)m_termNum;
		m_pMuxCntl->SendH230Sbe(pSeg);
		POBJDELETE(pSeg);

		// send TCS_2 to party (in order to get IIS with ident name)
		m_pMuxCntl->SendH230(Attr011, TCS_2);
	}
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnContentRateChangeDone(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;

	m_isSlaveFinishCM = YES;   // Relevant only for Open Content from SLAVE

	if (m_pMuxCntl)
	{
		m_pMuxCntl->OnMuxRmtXmitModeSpecial(cmCapReceive, m_pTargetComMode);
		m_pMuxCntl->SetXmitComMode(*m_pTargetComMode);
	}
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnMuxRmtXfrMode(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;

	CComMode newScm;
	newScm.DeSerialize(NATIVE, *pParam);

	if (m_pMuxCntl->IsPartyH239() && IsPartyLinkToMaster())  // Link Party connected to the Slave conference and serve as LinkToMaster
	{
		// to set it to H323 values in conference
		DWORD contentRate = CUnifiedComMode::TranslateAmscRateToIPRate(newScm.GetContentModeContentRate());

		if (YES == m_isContentSpeakerOnSlave) // Start or Stop Content from EP on SLAVE m_isContentSpeakerOnSlave updated only on end change mode of all parties
		{
			if (contentRate != m_lastContentRateFromMasterForThisToken)
			{
				m_pConfApi->MasterContentMessage(MASTER_RATE_CHANGE, (CTaskApp*)this, m_mcuNum, m_termNum, contentRate);
				m_lastContentRateFromMasterForThisToken = contentRate;

				if (0 == contentRate)
					m_isContentSpeakerOnSlave = NO;
			}
		}
		else // Start/Stop Content from EP on Master
		{
			if (0 != contentRate)
				m_pConfApi->MasterContentMessage(MASTER_START_CONTENT, (CTaskApp*)this, m_mcuNum, m_termNum, contentRate);

			else if ((m_pMuxCntl->GetContentInStreamState() != eNoChannel &&
			          m_pMuxCntl->GetContentInStreamState() != eStreamOff) ||
			         (m_pMuxCntl->GetContentOutStreamState() != eNoChannel &&
			          m_pMuxCntl->GetContentOutStreamState() != eStreamOff)) // Stop content
			{
				CSegment* seg = new CSegment;
				BYTE lable = 0;
				*seg << (OPCODE)PARTY_TOKEN_RELEASE;
				*seg << m_mcuNum << m_termNum << lable;

				OnContentBrdgTokenMsg(seg); // should be replaced by dispatch event - Or send message to party!!!!!!!!
				m_lastContentRateFromMasterForThisToken = contentRate;
			}
		}
	}

	m_pConfApi->PartyRemoteXferMode(GetPartyId(), newScm);
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnMuxStartCommH230Seq(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnMuxSendSilence(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnMuxRmtCapSetUp(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;

	WORD status;

	if (!m_voice)
	{
		*pParam >> status;
		if (status == statOK)
		{
			CCapH320 rmtCap;
			rmtCap.DeSerialize(NATIVE, *pParam);

			CSegment rmtCapSeg;
			rmtCap.Serialize(NATIVE, rmtCapSeg);

			m_pConfApi->UpdateDB(this, RMOTCAP, (DWORD)0, 1, &rmtCapSeg);

			rmtCap.Dump();
		}
		else
			PASSERT(status);
	}
}
//--------------------------------------------------------------------------
void CIsdnVideoParty::OnMuxRmtCapConnect(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;

	WORD status;
	if (!m_voice)
	{
		*pParam >> status;
		if (status == statOK)
		{
			CCapH320 rmtCap;
			rmtCap.DeSerialize(NATIVE, *pParam);

			CSegment rmtCapSeg;
			rmtCap.Serialize(NATIVE, rmtCapSeg);

			m_pConfApi->UpdateDB(this, RMOTCAP, (DWORD)0, 1, &rmtCapSeg);

			rmtCap.Dump();
			m_pConfApi->PartyRemoteCap(GetPartyId(), rmtCap);
		}
		else
			PASSERT(status);
	}
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnMuxSyncChangeMode(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
	OnMuxSync(pParam);
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnMuxSyncConnect(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
	OnMuxSync(pParam);
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnMuxSync(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
	WORD localRemote, lostRegain, chnlNum;
	*pParam >> localRemote >> lostRegain >> chnlNum;
	if (chnlNum == 0 && localRemote == LOCAL)       // initial channel sync loss
	{
		if (lostRegain)
			TRACEINTO << PARTYNAME << " - Sync regain, so unmute audio";
		else
			TRACEINTO << PARTYNAME << " - Sync loss, so mute audio";

		if (lostRegain)
			m_pConfApi->AudioActive(GetPartyRsrcID(), 0, MCMS);
		else
			m_pConfApi->AudioActive(GetPartyRsrcID(), 1, MCMS);
	}

	DWORD par = 0;
	par   = localRemote;
	par <<= 16;
	par  |= lostRegain;
	m_pConfApi->UpdateDB(this, H221, par, 1);
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnMcuMediaErrConnect(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;

	WORD onOff;
	WORD srcRequest;
	*pParam >> srcRequest >> onOff;
	m_pConfApi->AudioActive(GetPartyRsrcID(), onOff, srcRequest);
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnMuxH230SetUp(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
	OnMuxH230(pParam);
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnMuxH230ChangeMode(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
	OnMuxH230(pParam);
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnMuxH230Connect(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
	OnMuxH230(pParam);
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnMuxH230(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
	pParam->DumpHex();

	BYTE opcode = 0;
	*pParam >> opcode;
	switch (opcode)
	{
		case (Start_Mbe | ESCAPECAPATTR):
		{
			OnMuxH230Mbe(pParam);
			break;
		}

		case (Ns_Com | ESCAPECAPATTR):
		{
			ALLOCBUFFER(msg, ONE_LINE_BUFFER_LEN);
			strncpy(msg, m_name, (sizeof(msg)-1));
			strcat(msg, " H230 - Ns_Comm");
			PTRACE2(eLevelInfoNormal, "CIsdnVideoParty::OnMuxH230 : Name - ", msg);
			DEALLOCBUFFER(msg);
			OnMuxH230NS_Com(pParam);
			break;
		}

		case (H230_Esc | ESCAPECAPATTR):
		{
			ALLOCBUFFER(msg, ONE_LINE_BUFFER_LEN*2);
			strncpy(msg, m_name, (sizeof(msg)-1));
			strcat(msg, " H230 - ");
			WORD        i      = 0;
			H230_ENTRY* pEntry = m_H230Entries;
			BYTE        opcode;
			*pParam >> opcode;
			while (pEntry->actFunc != 0)
			{
				if (pEntry->opcode == opcode)
				{
					strcat(msg, pEntry->opcodeStr);
					strcat(msg, " ,  ");
					strcat(msg, pEntry->descStr);
					PTRACE2(eLevelInfoNormal, "CIsdnVideoParty::OnMuxH230 : Name - ", msg);
					(this->*(pEntry->actFunc))(pParam);
					break;
				}

				pEntry++;
			}

			if (pEntry->actFunc == 0)
				TRACESTRFUNC(eLevelError) << PARTYNAME << ", Opcode:" << (DWORD)opcode << " - Failed, opcode not found";

			DEALLOCBUFFER(msg);
			break;
		}

		default:
		{               // bas h230 opcodes
			OnMuxH230Bas(opcode);
			break;
		}
	} // switch
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnMuxH230Bas(BYTE opcode)
{
	ALLOCBUFFER(msg, ONE_LINE_BUFFER_LEN);
	strncpy(msg, m_name, (sizeof(msg)-1));
	switch (opcode)
	{
		case (Freeze_Pic  | OTHRCMDATTR):
		{
			strcat(msg, " H230 - VCF");
			if (!m_pCurrentComMode->IsFreeVideoRate())
				m_pConfApi->VideoFreeze(this);

			if (m_videoPlusType != eNONE && m_pCurrentComMode->IsFreeVideoRate())
				m_pConfApi->VideoFreeze(this);

			break;
		}

		case (Fast_Update | OTHRCMDATTR):
		{
			// currently no vcu filtering
			m_rcvVcuCounter++;
			strcat(msg, " H230 - VCU");
			if (m_pTargetComMode->GetVidMode())
			{
				m_pConfApi->VideoRefresh(GetPartyId());
			}
			else       // party connected to itself
			{
				m_pMuxCntl->SendH230(H230_Code_000, VIS);
			}

			break;
		}

		case (Au_Loop     | OTHRCMDATTR):
		{
			strcat(msg, " H230 - Au_Loop");
			break;
		}

		case (Vid_Loop    | OTHRCMDATTR):
		{
			strcat(msg, " H230 - Vid_Loop");
			break;
		}

		case (Dig_Loop    | OTHRCMDATTR):
		{
			strcat(msg, " H230 - Dig_Loop");
			break;
		}

		case (Loop_Off    | OTHRCMDATTR):
		{
			strcat(msg, " H230 - Loop_Off");
			break;
		}

		default:
		{
			TRACESTRFUNC(eLevelError) << PARTYNAME << ", Opcode:" << (DWORD)opcode << " - Failed, unknown H230 BAS opcode";
			break;
		}
	} // switch

	TRACEINTO << PARTYNAME << " - " << msg;
	DEALLOCBUFFER(msg);
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnMuxH230Mbe(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;

	ALLOCBUFFER(msg, ONE_LINE_BUFFER_LEN);
	strncpy(msg, m_name, (sizeof(msg)-1));
	BYTE msgLen = 0;
	BYTE opcode = 0;
	*pParam >> msgLen >> opcode;
	switch (opcode)
	{
		case TIL:
		{
			strcat(msg, " H230 - TIL");
			break;
		}

		case IIS:
		{
			strcat(msg, " H230 - IIS");
			BYTE tcs_n = 0;
			*pParam >> tcs_n;
			switch (tcs_n)
			{
				case 1:
				{
					char password[30];

					if (msgLen > 30)
					{
						TRACEINTO << "wrong message length";
						break;
					}

					WORD i = 0;
					for (i = 0;  i < msgLen - 2; i++)
						*pParam >> (BYTE &)password[i];

					password[i] = '\0';
					// do nothing currently TCS_i not supported
					TRACEINTO << "password:" << password << " - Do nothing, currently TCS_i not supported";
					break;
				}

				case 2:
				{
					eTelePresencePartyType eActualTelePresencePartyType = eTelePresencePartyNone;
					char* siteName = new char[H243_TERMINAL_NAME_LEN];
					memset(siteName, '\0', H243_TERMINAL_NAME_LEN);

					WORD ActualTypedLength = msgLen - 2;
					if (ActualTypedLength > 0)
					{
						if (ActualTypedLength > (H243_TERMINAL_NAME_LEN - 1))   // To avoid memory corruption.
						{
							PTRACE2(eLevelInfoNormal, "CIsdnVideoParty::OnMuxH230Bas : Party name was cut : Name - ", PARTYNAME);
							ActualTypedLength = (H243_TERMINAL_NAME_LEN - 1);
						}

						pParam->Get((unsigned char*)siteName, ActualTypedLength);

						siteName[ActualTypedLength] = '\0';
						DWORD len = ActualTypedLength+1;
						m_pConfApi->SendSiteAndVisualNamePlusProductIdToPartyControl(this, TRUE /*bIsVisualName*/, len, siteName, FALSE, 0, NULL, FALSE, 0, NULL, eTelePresencePartyNone);
					}

					PDELETEA(siteName);
					PTRACE2(eLevelInfoNormal, "CIsdnVideoParty::OnMuxH230Mbe received TCS_2, ", PARTYNAME);
					break;
				}

				case 4:
				{
					ALLOCBUFFER(String_TCS4, H243_NAME_LEN);
					WORD i = 0;
					for (i = 0;  i < msgLen - 2 && i < H243_NAME_LEN - 2; i++)
						*pParam >> (BYTE &)String_TCS4[i];

					String_TCS4[i] = '\0';
					// do nothing currently TCS_i not supported
					TRACESTR(eLevelInfoNormal) << " CIsdnVideoParty::OnMuxH230Mbe received TCS_4 : extension_address = " << String_TCS4 << ", " << PARTYNAME;
					DEALLOCBUFFER(String_TCS4);

					break;
				}

				default:
				{
					break;
				}
			} // switch

			break;
		}

		case TIR:
		{
			strcat(msg, " H230 - TIR");
			// do nothing chair control not supported
			break;
		}

		case TIP:
		{
			strcat(msg, " H230 - TIP");
			break;
		}

		case NIA:
		{
			strcat(msg, " H230 - NIA");
			break;
		}

		case NIAP:
		{
			strcat(msg, " H230 - NIAP");
			break;
		}

		case Au_MAP:
		{
			strcat(msg, " H230 - Au_MAP");
			break;
		}

		case MRQ:
		{
			strcat(msg, " H230 - MRQ");
			break;
		}

		case VideoNotDecodedMBs:
		{
			strcat(msg, " H230 - VideoNotDecodedMBs");
			break;
		}

		case H239_messsage:
		{
			strcat(msg, " H230 - H239_messsage");
			OnMuxH239Message(msgLen, pParam);
			break;
		}

		case AMC_CI:
		{
			strcat(msg, " H230 - AMC-CI_messsage");
			OnMuxAMC_CI(msgLen, pParam);
			break;
		}

		case h239ExtendedVideoCapability:
		{
			strcat(msg, " H320 - h239ExtendedVideoCapability");
			if (m_pMuxCntl->IsMuxOnSyncRecover() == TRUE)
				strcat(msg, " mux is in sync loss recover, ignore remote caps, might be corrupted!");
			else
				m_pMuxCntl->SetRmtH239ExtendedVidCaps(*pParam, msgLen-1);

			break;
		}

		case AMC_cap:
		{
			strcat(msg, " H230 - AMC_cap ERROR - should be sent in caps set");
			m_pMuxCntl->SetRmtAMCCaps(*pParam, msgLen);
			break;
		}

		default:
		{
			strcat(msg, " H230 - ???");
			break;
		}
	} // switch

	TRACEINTO << PARTYNAME << " - " << msg;
	DEALLOCBUFFER(msg);
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnMuxH239Message(BYTE msgLen, CSegment* pParam)
{
	BYTE  H239subMsgIdentifier;
	DWORD ContentOpcode;
	BYTE  ack_nak = 0;
	EMsgDirection direction = eMsgIn;

	TRACEINTO << PARTYNAME;

	*pParam >> H239subMsgIdentifier;
	if (H239subMsgIdentifier == PresentationTokenResponse)
		*pParam >> ack_nak;

	ContentOpcode = TranslateH239OpcodeToPPCOpcode(H239subMsgIdentifier, ack_nak);

	SpreadAllH239Msgs(pParam, direction, 0, ContentOpcode);
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::SpreadAllH239Msgs(CSegment* pParam, EMsgDirection direction, WORD isOldTokenMsg, DWORD Opcode, EMsgStatus msgStat)
{
	BYTE           H239subMsgIdentifier, ContentOpcode;
	CTokenMsg*     pTokenMsg = NULL;
	EMsgStatus     eMsgStat  = eMsgInvalid;
	EHwStreamState eStreamState;
	int            channelId = 0, terminalLabel = 0, symmetryBreaking = 0;
	BYTE           mcuNum    = 0, terminalNum = 0, lable = 0;
	BYTE           msgLength;
	BYTE           IsMsgToParty = false;

	if (!(m_pMuxCntl->IsPartyH239()))
	{
		TRACESTRFUNC(eLevelError) << PARTYNAME << " - Party hasn't H239 Cap Or Conf not supports";
	}
	else
	{
		CSegment* segTemp = new CSegment;
		CSegment* seg     = new CSegment;
		*seg << (BYTE)(Start_Mbe | ESCAPECAPATTR);

		// If this is first msg after update we don't need to go through TMM
		// Need to send it to its destination.
		if (isOldTokenMsg)
		{
			TRACEINTO << PARTYNAME << " - OldOpcode:" << msgStat;
			eMsgStat = msgStat;
		}
		else
		{
			TRACEINTO << PARTYNAME << " - Opcode:" << Opcode << ", StreamState:" << (DWORD)m_pMuxCntl->GetContentInStreamState();

			// 1. Set correct stream state for TMM
			eStreamState = SetCorrectStreamStateForTMM();
			// Send new Token Msg to TMM
			pTokenMsg = new CTokenMsg(Opcode, direction, pParam);
			eMsgStat  = m_pH239TokenMsgMngr->NewTokenMsg(pTokenMsg, eStreamState);
		}

		CMedString str;
		str << "CIsdnVideoParty::SpreadAllH239Msgs : ";

		switch (Opcode)
		{
			// From Party to CB
			case PARTY_TOKEN_ACQUIRE: // terminal label, channel id, symb
			{
				if (eMsgStat == eMsgFree)
				{
					terminalLabel    = MbeBytesToInt(pParam);
					channelId        = MbeBytesToInt(pParam);
					symmetryBreaking = MbeBytesToInt(pParam);
					terminalNum      = terminalLabel%TERMINAL_LABEL_MULTIPLIER;
					mcuNum           = (terminalLabel-terminalNum)/TERMINAL_LABEL_MULTIPLIER;
					str << "PARTY_TOKEN_ACQUIRE: Channel Id: "<< channelId<< " symmetryBreaking: "<<symmetryBreaking<<" MCUNumber: "<<mcuNum<<" terminalNumber: "<<terminalNum;
					m_pConfApi->SendContentTokenMessage(PARTY_TOKEN_ACQUIRE, this, mcuNum, terminalNum, lable, (BYTE)symmetryBreaking);
				}

				break;
			}

			// "From CB" to Party
			// This Code run only  the slave (case CONTENT_ROLE_TOKEN_ACQUIRE)
			case CONTENT_ROLE_TOKEN_ACQUIRE: // In cascade mode the slave send the acquire to the master (link),From Party(Slave) to Party(Master)
			{
				if (eMsgStat == eMsgFree)
				{
					*pParam >> mcuNum	>> terminalNum;
					str << "CONTENT_ROLE_TOKEN_ACQUIRE(changed to PARTY_TOKEN_ACQUIRE and send to Master): MCUNumber: "<<mcuNum<<" terminalNumber: "<<terminalNum;
					*segTemp << (BYTE)H239_messsage;
					*segTemp << (BYTE)PresentationTokenRequest;
					SerializeGenericParameter(RejectParamIdent, 0, segTemp);
					SerializeGenericParameter(TerminalLabelParamIdent, ((mcuNum*TERMINAL_LABEL_MULTIPLIER)+terminalNum), segTemp);
					SerializeGenericParameter(ChannelIdParamIdent, SecondVideoChannel, segTemp);
					IsMsgToParty              = true;
					m_isContentSpeakerOnSlave = YES;
					m_isSlaveFinishCM         = NO;
				}

				break;
			}

			// "From CB" to Party
			case CONTENT_ROLE_TOKEN_RELEASE: // In cascade mode the slave send the release to the master (link),From Party(Slave) to Party(Master)
			{
				if (eMsgStat == eMsgFree)
				{
					*pParam >> mcuNum	>> terminalNum;
					str << "CONTENT_ROLE_TOKEN_RELEASE(changed to PARTY_TOKEN_RELEASE and send to Master): MCUNumber: "<<mcuNum<<" terminalNumber: "<<terminalNum;
					*segTemp << (BYTE)H239_messsage;
					*segTemp << (BYTE)PresentationTokenRelease;
					SerializeGenericParameter(RejectParamIdent, 0, segTemp);
					SerializeGenericParameter(TerminalLabelParamIdent, ((mcuNum*TERMINAL_LABEL_MULTIPLIER)+terminalNum), segTemp);
					SerializeGenericParameter(ChannelIdParamIdent, SecondVideoChannel, segTemp);
					IsMsgToParty      = true;
					m_isSlaveFinishCM = NO;
				}

				break;
			}

			// From CB to Party
			case CONTENT_ROLE_TOKEN_ACQUIRE_NAK:
			{
				if (eMsgStat == eMsgFree)
				{
					*pParam >> mcuNum	>> terminalNum;

					str << "CONTENT_ROLE_TOKEN_ACQUIRE_NAK:  MCUNumber: "<<mcuNum<<" terminalNumber: "<<terminalNum;
					*segTemp << (BYTE)H239_messsage;
					*segTemp << (BYTE)PresentationTokenResponse;
					SerializeGenericParameter(RejectParamIdent, 0, segTemp);
					SerializeGenericParameter(TerminalLabelParamIdent, ((mcuNum*TERMINAL_LABEL_MULTIPLIER)+terminalNum), segTemp);
					SerializeGenericParameter(ChannelIdParamIdent, SecondVideoChannel, segTemp);
					IsMsgToParty              = true;
					m_isContentSpeakerOnSlave = NO;
				}

				break;
			}

			// From CB to Party
			case CONTENT_ROLE_TOKEN_ACQUIRE_ACK:
			{
				if (eMsgStat == eMsgFree)
				{
					*pParam >> mcuNum	>> terminalNum;

					str << "CONTENT_ROLE_TOKEN_ACQUIRE_ACK:  MCUNumber: "<<mcuNum<<" terminalNumber: "<<terminalNum;
					*segTemp << (BYTE)H239_messsage;
					*segTemp << (BYTE)PresentationTokenResponse;  // Changed to PARTY_TOKEN_ACQUIRE_ACK
					SerializeGenericParameter(AcknowledgeParamIdent, 0, segTemp);
					SerializeGenericParameter(TerminalLabelParamIdent, ((mcuNum*TERMINAL_LABEL_MULTIPLIER)+terminalNum), segTemp);
					SerializeGenericParameter(ChannelIdParamIdent, SecondVideoChannel, segTemp);
					SerializeGenericParameter(SymmetryBreakingParamIdent, SYMMETRY_BREAKING_FOR_AQCUIRE_TOKEN, segTemp);
					IsMsgToParty = true;
				}

				break;
			}

			case PARTY_TOKEN_ACQUIRE_ACK: // Slave got it from M and inform the B
			{
				if (eMsgStat == eMsgFree)
				{
					terminalLabel    = MbeBytesToInt(pParam);
					channelId        = MbeBytesToInt(pParam);
					symmetryBreaking = MbeBytesToInt(pParam);
					terminalNum      = terminalLabel%TERMINAL_LABEL_MULTIPLIER;
					mcuNum           = (terminalLabel-terminalNum)/TERMINAL_LABEL_MULTIPLIER;
					str << "PARTY_TOKEN_ACQUIRE_ACK: Channel Id: "<< channelId<< " symmetryBreaking: "<<symmetryBreaking<<" MCUNumber: "<<mcuNum<<" terminalNumber: "<<terminalNum;
					m_pConfApi->SendContentTokenMessage(PARTY_TOKEN_ACQUIRE_ACK, this, mcuNum, terminalNum, lable, (BYTE)symmetryBreaking);
				}

				break;
			}

			// From CP to Party
			case CONTENT_ROLE_TOKEN_WITHDRAW:
			{
				if (eMsgStat == eMsgFree)
				{
					BYTE dummy;

					*pParam >> dummy // Only for compatibility with 323
					>> mcuNum
					>> terminalNum;

					str << "CONTENT_ROLE_TOKEN_WITHDRAW:  MCUNumber: "<<mcuNum<<" terminalNumber: "<<terminalNum;
					*segTemp << (BYTE)H239_messsage;
					*segTemp << (BYTE)PresentationTokenRequest;
					SerializeGenericParameter(TerminalLabelParamIdent, ((mcuNum*TERMINAL_LABEL_MULTIPLIER)+terminalNum), segTemp);
					SerializeGenericParameter(ChannelIdParamIdent, SecondVideoChannel, segTemp);
					SerializeGenericParameter(SymmetryBreakingParamIdent, SYMMETRY_BREAKING_FOR_WITHDRAW_TOKEN, segTemp);
					IsMsgToParty = true;
				}

				break;
			}

			// From Party to CB
			case PARTY_TOKEN_WITHDRAW_ACK:
			{
				if (eMsgStat == eMsgFree)
				{
					terminalLabel = MbeBytesToInt(pParam);
					channelId     = MbeBytesToInt(pParam);
					terminalNum   = terminalLabel%TERMINAL_LABEL_MULTIPLIER;
					mcuNum        = (terminalLabel-terminalNum)/TERMINAL_LABEL_MULTIPLIER;
					str << "PARTY_TOKEN_WITHDRAW_ACK: Channel Id: "<< channelId<< " MCUNumber: "<<mcuNum<<" terminalNumber: "<<terminalNum;
					m_pConfApi->SendContentTokenMessage(PARTY_TOKEN_WITHDRAW_ACK, this, mcuNum, terminalNum, lable);
				}

				break;
			}

			// From Party to CB
			case PARTY_TOKEN_RELEASE:
			{
				if (eMsgStat == eMsgFree)
				{
					terminalLabel = MbeBytesToInt(pParam);
					channelId     = MbeBytesToInt(pParam);
					terminalNum   = terminalLabel%TERMINAL_LABEL_MULTIPLIER;
					mcuNum        = (terminalLabel-terminalNum)/TERMINAL_LABEL_MULTIPLIER;

					if (IsPartyLinkToMaster())
					{
						str << "PARTY_TOKEN_RELEASE changed to MASTER_RATE_CHANGE : Channel Id: "<< channelId<<" MCUNumber: "<<mcuNum<<" terminalNumber: "<<terminalNum;
						m_pConfApi->MasterContentMessage(MASTER_RATE_CHANGE, (CTaskApp*)this, mcuNum, terminalNum, 0);
					}

					else
					{
						str << "PARTY_TOKEN_RELEASE: Channel Id: "<< channelId<<" MCUNumber: "<<mcuNum<<" terminalNumber: "<<terminalNum;
						m_pConfApi->SendContentTokenMessage(PARTY_TOKEN_RELEASE, this, mcuNum, terminalNum, lable);
					}
				}

				break;
			}

			// From CB to Party
			case CONTENT_ROLE_TOKEN_RELEASE_ACK:
			{
				str <<" CONTENT_ROLE_TOKEN_RELEASE_ACK Ignored in H239 Party: " <<PARTYNAME;
				break;
			}

			// From Party to CB
			case ROLE_PROVIDER_IDENTITY:
			{
				BYTE  tempSize = 0;
				BYTE* pData    = NULL;

				if (eMsgStat == eMsgFree)
				{
					terminalLabel = MbeBytesToInt(pParam);
					channelId     = MbeBytesToInt(pParam);
					terminalNum   = terminalLabel%TERMINAL_LABEL_MULTIPLIER;
					mcuNum        = (terminalLabel-terminalNum)/TERMINAL_LABEL_MULTIPLIER;
					str << "ROLE_PROVIDER_IDENTITY: Channel Id: "<< channelId<<" MCUNumber: "<<mcuNum<<" terminalNumber: "<<terminalNum;
					// for loop only
					m_pConfApi->ContentTokenRoleProviderMessage(this, mcuNum, terminalNum, lable, tempSize, pData);
				}

				break;
			}

			// From CB to Party
			case CONTENT_ROLE_PROVIDER_IDENTITY:
			{
				if (eMsgStat == eMsgFree)
				{
					*pParam >> mcuNum
					>> terminalNum;

					str << "CONTENT_ROLE_PROVIDER_IDENTITY:  MCUNumber: "<<mcuNum<<" terminalNumber: "<<terminalNum;
					*segTemp << (BYTE)H239_messsage;
					*segTemp << (BYTE)PresentationTokenIndicateOwner;
					SerializeGenericParameter(TerminalLabelParamIdent, ((mcuNum*TERMINAL_LABEL_MULTIPLIER)+terminalNum), segTemp);
					SerializeGenericParameter(ChannelIdParamIdent, SecondVideoChannel, segTemp);
					IsMsgToParty = true;
				}

				break;
			}

			case CONTENT_MEDIA_PRODUCER_STATUS:
			{
				if (eMsgStat == eMsgFree)
				{
					BYTE status, ChannelId;
					*pParam >> ChannelId
					>> status;

					if (status)
					{
						*segTemp << (BYTE)AMC_CI;
						channelId = m_pMuxCntl->GetContentModeControlID();
						*segTemp << (BYTE)channelId;
						*segTemp << (BYTE)(H230_Esc | ESCAPECAPATTR);
						*segTemp << (BYTE)(VIA | Attr000);  // send Logical Channel Active

						str << " CONTENT_MEDIA_PRODUCER_STATUS with Status Active :Channel Id: "<< channelId;
					}
					else
					{
						*segTemp << (BYTE)AMC_CI;
						channelId = m_pMuxCntl->GetContentModeControlID();
						*segTemp << (BYTE)channelId;
						*segTemp << (BYTE)(H230_Esc | ESCAPECAPATTR);
						*segTemp << (BYTE)(VIS | Attr000);  // send Logical Channel inActive

						str << " CONTENT_MEDIA_PRODUCER_STATUS with Status inActive :Channel Id: "<< channelId;
					}

					IsMsgToParty = true;
				}

				break;
			}

			default:
			{
				str << " Unknown Sub Message Identifier " << PARTYNAME;
			}
		} // switch

		if (eMsgStat != eMsgFree)
		{
			// In case of Delay- we need to check if we need to send
			if (eMsgStat == eMsgDelayed)
			{
				SendContentOnOff();
				PTRACE2(eLevelInfoNormal, "CIsdnVideoParty::SpreadAllH239Msgs : Delay msg case , Name - ", PARTYNAME);
			}
			else // eMsgInvalid case
			{
				PASSERTMSG(Opcode, "CIsdnVideoParty::SpreadAllH239Msgs: Msg invalid");
			}
		}

		if (IsMsgToParty)
		{
			msgLength = (BYTE)segTemp->GetWrtOffset();
			*seg << msgLength;
			*seg << *segTemp;

			m_pMuxCntl->SendH230(*seg);
		}

		str << ", Name - ";
		PTRACE2(eLevelInfoNormal, str.GetString(), PARTYNAME);
		POBJDELETE(seg);
		POBJDELETE(segTemp);
		POBJDELETE(pTokenMsg);
	}
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::SendContentOnOff()
{
	if (m_pMuxCntl->GetContentInStreamState() == eSendStreamOn || m_pMuxCntl->GetContentInStreamState() == eSendStreamOff)
	{
		// In this case we do nothing
		// Since we are in the middle of another Content On/Off req from the RTP
		TRACEINTO << PARTYNAME << " - Do nothing, in the middle of other Content ON/OFF";
		return;
	}

	// Updating the stream status
	if (m_pMuxCntl->GetContentInStreamState() == eStreamOn)
		m_pMuxCntl->SetContentInStreamState(eSendStreamOff);
	else if (m_pMuxCntl->GetContentInStreamState() == eStreamOff)
		m_pMuxCntl->SetContentInStreamState(eSendStreamOn);

	// Sending Content On/Off to the RTP.
	m_pMuxCntl->SendContentOnOffReqToMux();
	TRACEINTO << PARTYNAME << " - Send StreamON/OFF to Mux";
	StartTimer(MUXCONTENTACKTOUT, SECOND*2);
}

//--------------------------------------------------------------------------
DWORD CIsdnVideoParty::TranslateH239OpcodeToPPCOpcode(BYTE H239Opcode, BYTE Ack_Nak)
{
	DWORD Opcode = 0;

	switch (H239Opcode)
	{
		case PresentationTokenRequest:
		{
			Opcode = PARTY_TOKEN_ACQUIRE;
			break;
		}

		case PresentationTokenResponse:
		{
			if (Ack_Nak == AcknowledgeParamIdent)
			{
				if (IsPartyLinkToMaster())
					Opcode = PARTY_TOKEN_ACQUIRE_ACK;
				else
					Opcode = PARTY_TOKEN_WITHDRAW_ACK;
			}
			else if (Ack_Nak == RejectParamIdent)
				Opcode = PARTY_TOKEN_ACQUIRE_NAK;
			else if (Ack_Nak)
			{
				PASSERT(Ack_Nak);
			}
			else
			{
				PASSERT(101);
			}
			break;
		}

		case PresentationTokenRelease:
		{
			Opcode = PARTY_TOKEN_RELEASE;
			break;
		}

		case PresentationTokenIndicateOwner:
		{
			Opcode = ROLE_PROVIDER_IDENTITY;
			break;
		}

		default:
		{
			TRACESTRFUNC(eLevelError) << PARTYNAME << " - Unknown Sub Message identifier";
		}
	} // switch

	return Opcode;
}

//--------------------------------------------------------------------------
EHwStreamState CIsdnVideoParty::SetCorrectStreamStateForTMM()
{
	EHwStreamState eStreamStateForTmm = eHwStreamStateNone;

	switch (m_pMuxCntl->GetContentInStreamState())
	{
		case eStreamOn:
		case eSendStreamOn:
		case eSendStreamOff:
		{
			eStreamStateForTmm = eHwStreamStateOn;
			break;
		}

		case eStreamOff:
		{
			eStreamStateForTmm = eHwStreamStateOff;
			break;
		}

		case eNoChannel:
		case eWaitToSendStreamOn:
		{
			eStreamStateForTmm = eHwStreamStateNone;
			break;
		}

		default:
		{
			PASSERTMSG((DWORD)m_pMuxCntl->GetContentInStreamState(), "Stream state not valid ");
		}
	} // switch

	return eStreamStateForTmm;
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnMuxAMC_CI(BYTE msgLen, CSegment* pParam)
{
	BYTE channelID, opcode, length, mediaMode;

	*pParam >> channelID >> opcode;

	std::ostringstream str;
	str << PARTYNAME << " - ";

	switch (opcode)
	{
		case (Freeze_Pic  | OTHRCMDATTR):
		{
			str << "Freeze_Pic";
			m_pConfApi->ContentFreezePic(channelID);
			break;
		}

		case (Fast_Update | OTHRCMDATTR):
		{
			str << "Fast_Update";
			m_pConfApi->ContentVideoRefresh(channelID, YES, this);
			break;
		}

		case (H230_Esc | ESCAPECAPATTR):
		{
			*pParam >> opcode;
			if (opcode == (VIA | Attr000))
			{
				str << "VIA";
				m_pConfApi->ContentMediaProducerStatus(this, channelID, 1);
			}
			else if (opcode == (VIS | Attr000))
			{
				// In H239 we initiate the sending of Logical Channel Inactive before the change mode and do not wait for the EPs to send
				str << "VIS - Do nothing, ignored in H239 Party";
			}
			else if (opcode == (MCS | Attr001))
			{
				str << "MCS - Do nothing";
			}
			else
			{
				str << "Unknown AMC C&I, H230_Esc";
			}

			break;
		}

		case (Video_Off  | OTHRCMDATTR):
		case (H261  | OTHRCMDATTR):
		case (H263  | OTHRCMDATTR):
		case (H264  | OTHRCMDATTR):
		{
			length = msgLen - 2; // 1 - opcode (AMC_CI), 2 - ChannelID
			if (length)
			{
				BYTE* pMediaMode = new BYTE[length];
				for (int i = 0; i < length; i++)
				{
					mediaMode = opcode ^ OTHRCMDATTR;
					pMediaMode[i] = mediaMode;

					if ((i+1) < length) // last loop should not remove byte from segment because segment is empty.
						*pParam >> opcode;
				}

				m_pCurrentComMode->SetContentModeMediaMode(length, pMediaMode);
				PDELETEA(pMediaMode);
			}

			break;
		}

		default:
		{
			str << "Unknown AMC C&I";
		}
	} // switch
	TRACEINTO << str.str().c_str();
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnMuxH230NS_Com(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnMuxSyncTimer(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
	m_pConfApi->PartyDisConnect(NO_ESTABL_H243_CONNECT, this);
}

//--------------------------------------------------------------------------
BYTE CIsdnVideoParty::isChangeModeAndForContentOnly() const
{
	if (m_state != PARTYCHANGEMODE)
		return NO;

	if (m_pCurrentComMode->m_audMode == m_pTargetComMode->m_audMode &&
	    m_pCurrentComMode->m_vidMode == m_pTargetComMode->m_vidMode &&
	    m_pCurrentComMode->m_xferMode == m_pTargetComMode->m_xferMode &&
	    m_pCurrentComMode->m_hsdHmlpMode == m_pTargetComMode->m_hsdHmlpMode &&
	    m_pCurrentComMode->m_lsdMlpMode == m_pTargetComMode->m_lsdMlpMode &&
	    m_pCurrentComMode->m_otherMode == m_pTargetComMode->m_otherMode &&
	    m_pCurrentComMode->m_nsMode == m_pTargetComMode->m_nsMode &&
	    !(m_pCurrentComMode->m_contentMode == m_pTargetComMode->m_contentMode))
	{
		return YES;
	}

	return NO;
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnPartyUpdateConfRsrcIdForInterfaceAnycase(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;

	DWORD confRsrcId;

	*pParam >> confRsrcId;
	m_ConfRsrcId = confRsrcId;
	WORD num_of_channels = GetNumConnectedChannels();
	for (WORD channel_index = 0; channel_index < num_of_channels; channel_index++)
	{
		CSmallString str;
		str << "m_pNetChnlCntl " << channel_index << " ConfRsrcId = " << confRsrcId;
		PTRACE(eLevelInfoNormal, str.GetString());
		m_pNetChnlCntl[channel_index]->SetNewConfRsrcId(confRsrcId);
	}

	m_pBndCntl->SetNewConfRsrcId(confRsrcId);
	m_pMuxCntl->SetNewConfRsrcId(confRsrcId);
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnContentBrdgTokenMsg(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;

	if (!(m_pMuxCntl->IsPartyH239()))
	{
		PASSERTMSG(1, "Party hasn't H239 Cap Or Conf not supports");
	}
	else
	{
		DWORD opcode;
		*pParam >> opcode;

		EMsgDirection direction = eMsgOut;

		if (IsPartyLinkToMaster() && (PARTY_TOKEN_RELEASE == opcode))
			direction = eMsgIn;

		SpreadAllH239Msgs(pParam, direction, 0, opcode);
	}
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnMuxAckForContentOnOffWhileDisconnecting(CSegment* pParam)
{
}

//--------------------------------------------------------------------------
// After ack for Content ON/OFF we need to send Update to TMM
// Update give us the tokens list
// After update we need to send the first msg to it's destination - party or CB
void CIsdnVideoParty::OnMuxAckForContentOnOff(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;

	DWORD          status;
	EHwStreamState eStreamStateForTmm;
	EMsgStatus     eMsgStat;
	BYTE           isFirstMsgDelay    = 0;
	DWORD          tempContentInState = (DWORD)eNoChannel;
	CTokenMsg*     pTokenDelayMsg     = NULL;

	*pParam >> status >> tempContentInState;

	DeleteTimer(MUXCONTENTACKTOUT);

	if (status != STATUS_OK)
	{
		// Error handling -
		// There are 2 different paths:
		// 1. In case that we wait for stream ON ack and received NACK
		// 2. In case we wait for Stream OFF ack and receive NACK
		if (m_pMuxCntl->GetContentInStreamState() == eSendStreamOn)
		{
			// Case 1 - Stream ON
			DisconnectPartyDueToProblemsWithH239Stream();
		}
		else if (m_pMuxCntl->GetContentInStreamState() == eSendStreamOff)
		{
			// Case 2 - Stream OFF
			HandleProblemsDuringClosingContentStream();
		}
		else
			PASSERTMSG((DWORD)m_pMuxCntl->GetContentInStreamState(), "Wrong stream state");

		return;
	}

	// Forword Msg to ContentBrdg
	eContentState ePState = eStreamOff;
	if (m_pMuxCntl->GetContentInStreamState() == eStreamOn)
	{
		ePState = eStreamOn;
		m_pConfApi->HWConetntOnOffAck(this, ePState);
	}

	// Send Update stream to TMM - Handling the list...
	CTokenMsgMngr* tokenMsgList = new CTokenMsgMngr;
	tokenMsgList->EnableTokenMsgMngr();

	m_pH239TokenMsgMngr->StreamUpdate(tokenMsgList);

	if (tokenMsgList->Size() == 0)
	{
		tokenMsgList->Clear();
		POBJDELETE(tokenMsgList);
		return;
	}

	// Handling first message
	TOKEN_MSG_LIST::iterator itr = tokenMsgList->Begin();
	CTokenMsg* pTokenMsg = (CTokenMsg*)(*itr);

	SpreadAllH239Msgs(pTokenMsg->GetMsgSegment(), pTokenMsg->GetMsgDirection(), 1, pTokenMsg->GetMsgOpcode(), eMsgFree);

	HandleTMMList(tokenMsgList);

	tokenMsgList->ClearAndDestroy();
	POBJDELETE(tokenMsgList);
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::HandleTMMList(CTokenMsgMngr* tokenMsgList)
{
	TRACEINTO << PARTYNAME;

	CTokenMsg* pTokenMsg = NULL;
	CTokenMsg* pDelayedTokenMsg = NULL;
	BYTE isDelayed = 0;
	EHwStreamState eStreamStateForTmm;
	EMsgStatus eMsgStat;
	TOKEN_MSG_LIST::iterator itr = tokenMsgList->Begin();
	itr++; // Start from the second message

	while (itr != tokenMsgList->End())
	{
		pTokenMsg = (CTokenMsg*)(*itr);
		if (IsValidPObjectPtr(pTokenMsg))
		{
			eStreamStateForTmm = SetCorrectStreamStateForTMM();
			eMsgStat = m_pH239TokenMsgMngr->NewTokenMsg(pTokenMsg, eStreamStateForTmm);

			// In this case we will send all messages till we come across a delay one and then we will
			// 1. Insert all messages to the list
			// 2. Handle the Delay message.
			if ((eMsgStat == eMsgDelayed) && isDelayed == 0)
			{
				pDelayedTokenMsg = new CTokenMsg(*pTokenMsg);
				isDelayed        = 1;
			}

			if (isDelayed)
			{
				if (eMsgStat != eMsgDelayed)
				{
					PASSERTMSG((DWORD)eMsgStat, "Status was suppose to be DELAY (Regular case)");
				}
			}
			else
				SpreadAllH239Msgs(pTokenMsg->GetMsgSegment(), pTokenMsg->GetMsgDirection(), 1, pTokenMsg->GetMsgOpcode(), eMsgFree);
		}
		else
			PASSERTMSG(tokenMsgList->Size(), "TMM list is corrupted - Token msg not valid");

		itr++;
	}

	if (isDelayed)
		SpreadAllH239Msgs(pDelayedTokenMsg->GetMsgSegment(), pDelayedTokenMsg->GetMsgDirection(), 1, pDelayedTokenMsg->GetMsgOpcode(), eMsgDelayed);

	POBJDELETE(pDelayedTokenMsg);
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::DisconnectPartyDueToProblemsWithH239Stream()
{
	// In this case - we will send evacuate towards the RTP and disconnect the party
	TRACEINTO << PARTYNAME;

	m_pMuxCntl->SendEvacuateReqForMuxOnH239Stream();
	m_pConfApi->PartyDisConnect(ISDN_CALL_CLOSE_H239_CONTENT_PROCESSING_ERROR, this);
	m_pConfApi->UpdateDB(this, DISCAUSE, ISDN_CALL_CLOSE_H239_CONTENT_PROCESSING_ERROR, 1); // Disconnect cause
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::HandleProblemsDuringClosingContentStream()
{
	TRACEINTO << PARTYNAME;
	// In this case we will:
	// 1. Send Evacuate and wait for the Ack
	// 1.1 In case we don't receive an Ack(Tout)/Receive Nack - Disconnect the party.
	// 1.2 If we receive an Ack from the Rtp - Change stream state to eStreamOff and release all queued token messages (Update).
	m_pMuxCntl->SendEvacuateReqForMuxOnH239Stream();
	StartTimer(MUXEVACUATEACKTOUT, 2*SECOND);
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnMuxAckForEvacuateContentStream(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;

	DeleteTimer(MUXEVACUATEACKTOUT);
	DWORD status;
	*pParam >> status;

	if (status != STATUS_OK) // Disconnect the party
	{
		OnMuxAckForEvacuateTout(pParam);
		return;
	}

	m_pMuxCntl->SetContentInStreamState(eStreamOff);

	// Update stream to TMM - Handling the list...
	CTokenMsgMngr* tokenMsgList = new CTokenMsgMngr;
	tokenMsgList->EnableTokenMsgMngr();

	m_pH239TokenMsgMngr->StreamUpdate(tokenMsgList);

	if (tokenMsgList->Size() == 0)
	{
		PASSERTMSG((DWORD)m_pMuxCntl->GetContentInStreamState(), "List is empty");
		tokenMsgList->Clear();
		POBJDELETE(tokenMsgList);
		return;
	}

	// Handling first message
	TOKEN_MSG_LIST::iterator itr = tokenMsgList->Begin();
	CTokenMsg* pTokenMsg = (CTokenMsg*)(*itr);
	// If the first message id Delay - We need to issue an Assert and send all other messages to
	// the TMM (Ignore all replays till list is empty

	SpreadAllH239Msgs(pTokenMsg->GetMsgSegment(), pTokenMsg->GetMsgDirection(), 1, pTokenMsg->GetMsgOpcode(), eMsgFree);

	HandleTMMList(tokenMsgList);

	tokenMsgList->ClearAndDestroy();
	POBJDELETE(tokenMsgList);
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnMuxAckForEvacuateTout(CSegment* pParam) // Inga - TO DO
{
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnContentBrdgFreezePic(CSegment* pParam)
{
	BYTE controlID = 0, msgLength = 0;

	CSegment* seg = new CSegment; AUTO_DELETE(seg);

	if (m_pMuxCntl->IsPartyH239())
	{
		TRACEINTO << PARTYNAME;

		*seg << (BYTE)(Start_Mbe | ESCAPECAPATTR);
		CSegment* segTemp = new CSegment;

		*segTemp << (BYTE)AMC_CI;
		controlID = m_pTargetComMode->GetContentModeControlID();
		*segTemp << (BYTE)controlID;
		*segTemp << (BYTE)(Freeze_Pic  | OTHRCMDATTR);

		msgLength = (BYTE)segTemp->GetWrtOffset();
		*seg << (BYTE)msgLength;
		*seg << *segTemp;

		POBJDELETE(segTemp);

		m_pMuxCntl->SendH230(*seg);
		POBJDELETE(seg);
	}
	else
	{
		TRACEINTO << PARTYNAME << " - Not H239 Party";
	}
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnContentBrdgRefreshVideo(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;

	TICKS curTimer;
	BYTE  controlID, msgLength = 0;

	// Fast Update requests will be sent to a party
	// in a minimum interval of 1 seconds
	curTimer = SystemGetTickCount();

	if (curTimer < m_lastContentRefreshTime)
		m_lastContentRefreshTime = 0; // rollover fixup

	if ((m_lastContentRefreshTime == 0) || (curTimer - m_lastContentRefreshTime > SECOND/2))
	{
		CSegment* seg = new CSegment; AUTO_DELETE(seg);
		if (m_pTargetComMode->IsContentOn())
		{
			if (m_pMuxCntl->IsPartyH239())
			{
				*seg << (BYTE)(Start_Mbe | ESCAPECAPATTR);

				CSegment* segTemp = new CSegment;
				*segTemp << (BYTE)AMC_CI;
				controlID = m_pTargetComMode->GetContentModeControlID();
				*segTemp << (BYTE)controlID;
				*segTemp << (BYTE)(Fast_Update | OTHRCMDATTR);

				msgLength = (BYTE)segTemp->GetWrtOffset();
				*seg << (BYTE)msgLength;
				*seg << *segTemp;

				POBJDELETE(segTemp);

				m_lastContentRefreshTime = curTimer;
				m_pMuxCntl->SendH230(*seg);
				POBJDELETE(seg);
			}
		}
		else
		{
			TRACEINTO << PARTYNAME << " - AMSC ControlID is closed";
		}
	}
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnH230AudioMute(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
	DWORD paramMask = 0xF0000000;

	// should send mute to audio codec when AIM received according to SYSTEM.CFG flag
	BOOL isIgnoreMuteOnAim = NO;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("IGNORE_AIM", isIgnoreMuteOnAim);

	if (!isIgnoreMuteOnAim)
		m_pConfApi->AudioMute(GetPartyId(), eOn);

	// update DB
	m_pConfApi->UpdateDB(this, MUTE_STATE, paramMask | 1L, 1);
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnH230AudioActive(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
	DWORD paramMask = 0xF0000000;
	m_pConfApi->AudioMute(GetPartyId(), eOff);
	// update DB
	m_pConfApi->UpdateDB(this, MUTE_STATE, paramMask, 1);
}
//--------------------------------------------------------------------------
// audio command equalize (ACE): Sent by a terminal to request that the delay of the audio signal
// be equalized to that of the video signal ("lip synchronization"), in both directions.
// A terminal sending this request shall itself equalize the delays in the same way.
// *** Not supported in both RMX and MGC
void CIsdnVideoParty::OnH230AudioEqualize(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
// audio command zero-delay (ACZ): Sent by a terminal to request
// that the audio signal not be delayed to match that of the video signal.
// *** Not supported in both RMX and MGC
void CIsdnVideoParty::OnH230AudioZeroDelay(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
// video indicate suppressed (VIS): This symbol is used to indicate that the content of the video channel
// does not represent a normal camera image. The video encoder may be without video input
// or an electronically generated pattern may have been substituted.
void CIsdnVideoParty::OnH230VideoIndSuppressed(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
	// update DB
	m_pConfApi->UpdateDB(this, MUTE_STATE, 0xF0000010, 1);
}

//--------------------------------------------------------------------------
// video indicate active (VIA): Complementary to VIS. The video source is the only one,
// or, in the case that more video sources are to be distinguished, it is that designated "video No. 1".
void CIsdnVideoParty::OnH230VideoActive(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
	// update DB
	m_pConfApi->UpdateDB(this, MUTE_STATE, 0xF000000E, 1);
}

//--------------------------------------------------------------------------
// Chair-control Indicate Capability (CIC) Included in the cap-set of an MCU to show that it can
// properly process the codes (CCA, CIT, CCR, CIS, CCD, CIR, CCK), (TIA, TIN, TID, TIL, TCU, TIF), (VCB, VIN, VCR, VCE).
// *** chair control not supported at RMX - do nothing
void CIsdnVideoParty::OnH230ChairCntlCap(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
	// will be given in the cap set of master mcu
}

//--------------------------------------------------------------------------
// Chair Command Acquire (CCA) Transmitted by a terminal or MCU to claim a chair-control token.
// *** chair control not supported at RMX - do nothing
void CIsdnVideoParty::OnH230ChairCmdAcquire(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
// Chair Indicate Stopped-using-token (CIS) Transmitted by a terminal holding the chair token to release it.
// *** chair control not supported at RMX - do nothing
void CIsdnVideoParty::OnH230ChairStopToken(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
// Chair Indicate Release/refuse (CIR) Transmitted by an MCU when it cannot comply with the command CCD.
// *** chair control not supported at RMX - do nothing
void CIsdnVideoParty::OnH230ChairIndRefuse(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
// Chair Indicate Token (CIT) Used by an MCU to pass the chair-control token.
// *** chair control not supported at RMX - do nothing
void CIsdnVideoParty::OnH230ChairIndToken(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
// Chair Command Release/refuse (CCR) Used by an MCU to withdraw/refuse assignment of chair-control token.
// *** chair control not supported at RMX - do nothing
void CIsdnVideoParty::OnH230ChairReleaseToken(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
// Chair Command Disconnect (CCD) Transmitted by a chair-control terminal to an MCU
// to cause dropping of the terminal whose identity number follows.
// *** chair control not supported at RMX - do nothing
void CIsdnVideoParty::OnH230ChairDropTerm(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}
//--------------------------------------------------------------------------
// Chair Command Kill (CCK) Transmitted by a chair-control terminal to drop all terminals from the conference.
// *** chair control not supported at RMX - do nothing
void CIsdnVideoParty::OnH230ChairDropConf(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
// Terminal Indicate Floor-request (TIF) Transmitted by a terminal to its MCU; shall be followed by <M> <T>
// An MCU receiving TIF forwards it to the chair-control terminal if TM is locally connected,
// otherwise it is forwarded to the master MCU to forward it to the chair-control terminal.
// *** chair control not supported at RMX - do nothing
void CIsdnVideoParty::OnH230FloorReq(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
// Terminal Indicate End_of_Listing (TIE) Sent by an MCU when it has completed the transmission
// of a series of complementary TIL messages.
// *** H320 cascade currently not supported at RMX - do nothing
void CIsdnVideoParty::OnH230IndicateEndOfString(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
// Terminal Command Update (TCU) Transmitted by a terminal or MCU to an MCU
// to request an updated list of terminals connected.
void CIsdnVideoParty::OnH230UpdateTerminalList(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
// Terminal Indicate Assignment (TIA) Used by an MCU to transmit the assigned terminal number
// to another MCU or to a terminal; shall be followed by <M> <T>.
void CIsdnVideoParty::OnH230AssignTerminaNum(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
	BYTE mcu, terminal;
	GetMcuTerminalNum(pParam, mcu, terminal);
	m_pConfApi->AssignTerminalNum(this, CHAIRCNTL_MSG, mcu, terminal);
}

//--------------------------------------------------------------------------
// Terminal Indicate Number (TIN) Used to pass information
// concerning terminal number assignments made; shall be followed by <M> <T>.
void CIsdnVideoParty::OnH230IndTerminaNum(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
	BYTE mcu, terminal;
	GetMcuTerminalNum(pParam, mcu, terminal);
	m_pConfApi->IndicateTerminalNum(this, mcu, terminal);
}

//--------------------------------------------------------------------------
// Terminal Indicate Dropped (TID) Used to pass information concerning
// any terminal number no longer effective; shall be followed by <M><T>.
void CIsdnVideoParty::OnH230DropTerminaNum(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
	BYTE mcu, terminal;
	GetMcuTerminalNum(pParam, mcu, terminal);
	m_pConfApi->IndicateDroppedTerminalNum(this, mcu, terminal);
}

//--------------------------------------------------------------------------
// Video Command Broadcast (VCB) Transmitted by a chair-control terminal or an MCU to an MCU
// to cause broadcasting of the video from the terminal whose identity number follows VCB.
void CIsdnVideoParty::OnH230ChairVideoBroadcast(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
	BYTE mcu, terminal;
	GetMcuTerminalNum(pParam, mcu, terminal);
	m_pConfApi->ChairVideoBroadcast(this, mcu, terminal);
}

//--------------------------------------------------------------------------
// Cancel Video Command Broadcasting (VCE Cancel-VCB)  Returns the conference to voice-activated video switching.
void CIsdnVideoParty::OnH230ChairCancelVideoBroadcast(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
	m_pConfApi->ChairCancelVideoBroadcast(this);
}

//--------------------------------------------------------------------------
// Token Command Association (TCA) Sent by a terminal requesting the MCU to provide the terminal numbers associated with each token.
// The MCU responds with an MBE TIR.
void CIsdnVideoParty::OnH230TokenCommandAssociation(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
	m_pConfApi->TokenCommandAssociation(this);
}

//--------------------------------------------------------------------------
// LSD/HSD Command Acquire-token (DCA_L) Transmitted by a terminal or MCU to claim an
// LSD/HSD token; shall be followed by an SBE number indicating the data rate requested
// *** H320 LSD currently not supported at RMX - do nothing
void CIsdnVideoParty::OnH230LsdAcquireToken(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
// LSD/HSD Command Acquire-token (DCA_H) Transmitted by a terminal or MCU to claim an
// LSD/HSD token; shall be followed by an SBE number indicating the data rate requested
// *** H320 LSD currently not supported at RMX - do nothing
void CIsdnVideoParty::OnH230HsdAcquireToken(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
// LSD Indicate Token (DIT_L) Used by an MCU to pass the LSD token.
// *** H320 LSD currently not supported at RMX - do nothing
void CIsdnVideoParty::OnH230LsdIndicateToken(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
// HSD Indicate Token (DIT_H) Used by an MCU to pass the HSD token.
// *** H320 LSD currently not supported at RMX - do nothing
void CIsdnVideoParty::OnH230HsdIndicateToken(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
// LSD Indicate Stopped-using-token (DIS_L) Transmitted by a terminal holding the LSD token to release it.
// *** H320 LSD currently not supported at RMX - do nothing
void CIsdnVideoParty::OnH230LsdStopToken(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
// HSD Indicate Stopped-using-token (DIS_H) Transmitted by a terminal holding the HSD token to release it.
// *** H320 LSD currently not supported at RMX - do nothing
void CIsdnVideoParty::OnH230HsdStopToken(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
// LSD/HSD Command Release/refuse (DCR_L) Used by an MCU to withdraw/refuse
// assignment of LSD token, or by the chair-control terminal to cause this withdrawal.
// *** H320 LSD currently not supported at RMX - do nothing
void CIsdnVideoParty::OnH230LsdReleaseToken(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
// LSD/HSD Command Release/refuse (DCR_H) Used by an MCU to withdraw/refuse
// assignment of LSD token, or by the chair-control terminal to cause this withdrawal.
// *** H320 LSD currently not supported at RMX - do nothing
void CIsdnVideoParty::OnH230HsdReleaseToken(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
// LSD/HSD Command Close (DCC_L) Transmitted by a terminal holding the LSD/HSD token
// to release it and close the LSD/HSD channel.
// *** H320 LSD currently not supported at RMX - do nothing
void CIsdnVideoParty::OnH230LsdCloseChannel(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
// Terminal Command Identify (TCI) Sent by an MCU to a directly-connected terminal or vice versa
// to exact identification by means of a symbol TII*.
// *** H320 LSD currently not supported at RMX - do nothing
void CIsdnVideoParty::OnH230TerminalIdentify(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
// Terminal Indicate Identity (TII) Sent in response to TCI; shall be followed by an SBE alphanumeric character according to 3.4,
// the content being prescribed by the MCU service provider.
// *** H320 LSD currently not supported at RMX - do nothing
void CIsdnVideoParty::OnH230TerminalIndIdentity(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
// Terminal Indicate identity-Stop (TIS) End-marker to indicate the end of a sequence of TII symbols.
void CIsdnVideoParty::OnH230TerminalIndIdentityStop(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnH230TerminalPassword(CSegment* pParam)
{
	// only relevant in cascade mode. not clear if password ACK to be handle
	// in party layer or in the conf layer.
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnH230TerminalIdentity(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnH230TerminalConfIdentify(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnH230TerminalPersonaldentify(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnH230TerminalExtensionAddress(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnH230MultiCmdVisualize(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;

	BYTE forceType = 0;    // default - usual MCV force
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnH230MultiCancelCmdVisualize(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnH230MultiIndVisualize(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnH230MultiCancelIndVisualize(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnH230VideoCmdSelect(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnH230CancelVideoCmdSelect(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnH230MultiIndLopp(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnH230MultiIndMaster(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnH230CancelMultiIndMaster(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnH230RandomNum(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnH230IndVideoSrcNum(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnH230ZeroCom(CSegment* pParam)
{
	// only cascaded
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnH230SecondaryStatus(CSegment* pParam)
{
	// only cascaded
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnH230CancelZeroCom(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnH230CancelSecondaryStatus(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnH230MlpData(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnH230MultiCmdSymetric(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnH230MultiCmdConf(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
	m_nodeType = CASCADE_MODE_MCU;
	m_pConfApi->SetNodeType(GetPartyId(), m_nodeType);

	m_ivrCtrl->SetCascadeLinkInConf();
	m_ivrCtrl->SetIsLeader(0, 0, 1); // Set as ChairPerson only for DTMF collector

	CCommConf* pTmpCommConf = ::GetpConfDB()->GetCurrentConf(m_monitorConfId);
	if (pTmpCommConf)
	{
		CConfParty* pConfParty = pTmpCommConf->GetCurrentParty(m_monitorPartyId);
		if (pConfParty) {
			if (DIAL_OUT == pConfParty->GetConnectionTypeOper())
				pConfParty->SetCascadeMode(CASCADE_MODE_MASTER);  // VNGR-16290 temp as Master to set the Icon - NO Cascade UI for ISDN yet
			else
			{
				if (DIAL_IN == pConfParty->GetConnectionTypeOper())
					pConfParty->SetCascadeMode(CASCADE_MODE_SLAVE);
			}
		}
	}

	m_pMuxCntl->AddH261ToLocalCapsAndSenfReCapIfNeeded();
}

//--------------------------------------------------------------------------
BYTE CIsdnVideoParty::IsPartyLinkToMaster()
{
	BYTE isLinkToM = NO;
	if (CASCADE_MODE_MCU == m_nodeType)
	{
		CCommConf* pTmpCommConf = ::GetpConfDB()->GetCurrentConf(m_monitorConfId);
		if (pTmpCommConf)
		{
			CConfParty* pConfParty = pTmpCommConf->GetCurrentParty(m_monitorPartyId);
			if (pConfParty && (CASCADE_MODE_SLAVE == pConfParty->GetCascadeMode()))
			{
				isLinkToM = YES;
			}
		}
	}
	return isLinkToM;
}

//--------------------------------------------------------------------------
BYTE CIsdnVideoParty::IsPartyLinkToSlave()
{
	BYTE isLinkToS = NO;
	if (CASCADE_MODE_MCU == m_nodeType)
	{
		CCommConf* pTmpCommConf = ::GetpConfDB()->GetCurrentConf(m_monitorConfId);
		if (pTmpCommConf)
		{
			CConfParty* pConfParty = pTmpCommConf->GetCurrentParty(m_monitorPartyId);
			if (pConfParty && (CASCADE_MODE_MASTER == pConfParty->GetCascadeMode()))
			{
				isLinkToS = YES;
			}
		}
	}
	return isLinkToS;
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnH230MultiIndicateHierarchy(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::GetMcuTerminalNum(CSegment* pParam, BYTE& mcu, BYTE& terminal)
{
	BYTE sbe;
	if (!pParam->EndOfSegment())
	{
		*pParam >> sbe;
		if (sbe != (H230_Sbe_Esc | ESCAPECAPATTR))
		{
			TRACESTRFUNC(eLevelError) << PARTYNAME << " - invalid mcu sbe";
			return;
		}
		else
			*pParam >> mcu;

		*pParam >> sbe;
		if (sbe != (H230_Sbe_Esc | ESCAPECAPATTR))
		{
			TRACESTRFUNC(eLevelError) << PARTYNAME << " - invalid terminal sbe";
			return;
		}
		else
			*pParam >> terminal;
	}
	else
		TRACESTRFUNC(eLevelError) << PARTYNAME << " - Terminal did not send SBE information (mcu  & terminal numbers)";
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnVidBrdgRefresh(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
	DWORD ask_for_intra_every_5_seconds = 0;   // temporary for debug

	if (ask_for_intra_every_5_seconds)
	{
		StartTimer(TIMER_BETWEEN_VCU_REC, 5*SECOND);
		m_pMuxCntl->AskEpForIntra();
	}
	else
	{
		DWORD is_intra_sent = FilterAndSendVideoRefresh();
	}
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnVidBrdgH239ReCap(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
	m_pMuxCntl->SendH239VideoCaps();
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnAudBrdgValidation(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
	WORD onOff = 0;
	*pParam >> onOff;
	WORD value = 0;
	onOff == 1 ? value = AIA : value = AIM;
	m_pMuxCntl->SendH230(H230_Code_000, value);
	m_pMuxCntl->SendAudioValid(onOff); // temp because it causes a noise in audio
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnPartyCntlSendMMSOnOff(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
	WORD onOff = 0;
	*pParam >> onOff;
	WORD value = 0;
	onOff == 1 ? value = MMS : value = Cancel_MMS;
	m_pMuxCntl->SendH230(Attr001, value);
	m_pMuxCntl->SendMMS(onOff);
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnPartyCntlSendMCCOnOff(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
	WORD onOff = 0;
	*pParam >> onOff;
	WORD value = 0;
	onOff == 1 ? value = MCC : value = Cancel_MCC;
	m_pMuxCntl->SendH230(Attr001, value);
	m_pMuxCntl->SendMCC(onOff);
}

//--------------------------------------------------------------------------
WORD CIsdnVideoParty::FilterAndSendVideoRefresh()
{
	TRACEINTO << PARTYNAME;

	WORD ret_val = 0;

	// Fast Update requests will be sent to a party
	// in a minimum interval of 1 seconds
	TICKS curTimer = SystemGetTickCount();

	if (curTimer < m_lastVcuTime)
		m_lastVcuTime = 0; // rollover fixup

	if ((m_lastVcuTime == 0) || (curTimer - m_lastVcuTime > SECOND*1))
	{
		m_pMuxCntl->AskEpForIntra();
		m_lastVcuTime = curTimer;
		ret_val = 1;
	}
	else
	{
		TRACEINTO << PARTYNAME << " - block intra";
		ret_val = 0;
	}
	return ret_val;
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnBridgeNumberingMessage(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnMuxReceivedPartyFullCapSet(CSegment* pParam)
{
	CCapH320 partyCapsTest;
	CComMode currentRmtScm;

	partyCapsTest.DeSerialize(NATIVE, *pParam);
	currentRmtScm.DeSerialize(NATIVE, *pParam);

	TRACEINTO << PARTYNAME;

	m_state = PARTYCONNECTED;
	m_pConfApi->PartyReceivedFullCapSet(GetPartyId(), partyCapsTest, currentRmtScm);

	if (m_delayIVR)
		StartIvr();
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnContentBrdgAmcVideoMode(CSegment* pParam)
{
	if (m_pMuxCntl->IsPartyH239())
	{
		TRACEINTO << PARTYNAME;

		CContentMode* pContentMode = new CContentMode;
		CSegment*     seg          = new CSegment;
		CSegment*     segTemp      = new CSegment;
		BYTE          msgLength    = 0;


		pContentMode->DeSerialize(NATIVE, *pParam);

		BYTE mediaMode     = pContentMode->GetContentMediaMode();
		BYTE mediaModeSize = pContentMode->GetMediaModeSize();
		BYTE channelID     = pContentMode->GetControlID();

		// send Video Mode
		if (mediaModeSize)
		{
			*seg << (BYTE)(Start_Mbe | ESCAPECAPATTR);
			*segTemp << (BYTE)AMC_CI;
			*segTemp << (BYTE)channelID;
			*segTemp << (BYTE)(mediaMode | OTHRCMDATTR);
			msgLength = (BYTE)segTemp->GetWrtOffset();
			*seg << (BYTE)msgLength;
			*seg << *segTemp;

			m_pMuxCntl->SendH230(*seg);
		}

		POBJDELETE(seg);
		POBJDELETE(segTemp);
		POBJDELETE(pContentMode);

		/****************If Works need to add to TMM all function*******/ //
		// send VIA
		seg     = new CSegment;
		segTemp = new CSegment;

		// send Logical Channel Active
		*seg << (BYTE)(Start_Mbe | ESCAPECAPATTR);
		*segTemp << (BYTE)AMC_CI;
		*segTemp << (BYTE)channelID;
		*segTemp << (BYTE)(H230_Esc | ESCAPECAPATTR) << (BYTE)(VIA | Attr000);
		msgLength = (BYTE)segTemp->GetWrtOffset();
		*seg << (BYTE)msgLength;
		*seg << *segTemp;

		m_pMuxCntl->SendH230(*seg);

		POBJDELETE(seg);
		POBJDELETE(segTemp);
	}
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnContentBrdgAmcLogicalChannelInactive(CSegment* pParam)
{
	if (m_pMuxCntl->IsPartyH239())
	{
		TRACEINTO << PARTYNAME;

		BYTE msgLength = 0;
		BYTE channelID = 0;
		CSegment* seg       = new CSegment;
		CSegment* segTemp   = new CSegment;

		*pParam >> channelID;

		*seg << (BYTE)(Start_Mbe | ESCAPECAPATTR);
		*segTemp << (BYTE)AMC_CI;
		*segTemp << (BYTE)channelID;
		*segTemp << (BYTE)(H230_Esc | ESCAPECAPATTR) << (BYTE)(VIS | Attr000);
		msgLength = (BYTE)segTemp->GetWrtOffset();
		*seg << (BYTE)msgLength;
		*seg << *segTemp;

		m_pMuxCntl->SendH230(*seg);

		POBJDELETE(seg);
		POBJDELETE(segTemp);
	}
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnContentBrdgAmcMCS(CSegment* pParam)
{
	if (m_pMuxCntl->IsPartyH239())
	{
		TRACEINTO << PARTYNAME;

		BYTE msgLength = 0;
		BYTE channelID = 0;
		CSegment* seg       = new CSegment;
		CSegment* segTemp   = new CSegment;
		// inga - do we receive channel ID
		*pParam >> channelID;

		*seg << (BYTE)(Start_Mbe | ESCAPECAPATTR);
		*segTemp << (BYTE)AMC_CI;
		*segTemp << (BYTE)channelID;
		*segTemp << (BYTE)(H230_Esc | ESCAPECAPATTR) << (BYTE)(MCS | Attr001);
		msgLength = (BYTE)segTemp->GetWrtOffset();
		*seg << (BYTE)msgLength;
		*seg << *segTemp;

		m_pMuxCntl->SendH230(*seg);

		POBJDELETE(seg);
		POBJDELETE(segTemp);
	}
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnPartyCntlUpdatePresentationOutStream(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
	m_pMuxCntl->UpdatePresentationOutStream();
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnPartyUpdatedPresentationOutStream(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
	m_pConfApi->UpdatePresentationRes(GetPartyId());
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::PartySpecfiedIvrDelay()
{
	TRACEINTO << PARTYNAME << " - Start timer";
	StartTimer(TIMER_START_IVR, 10*SECOND);
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnVideoReadyForIvr(CSegment* pParam)
{
	// sdded in order to sync ivr message + slide in order to prevent "cut message" and short slide.
	TRACEINTO << PARTYNAME;

	if (IsValidTimer(TIMER_START_IVR))
	{
		DeleteTimer(TIMER_START_IVR);
		CParty::OnTimerStartIvr(NULL);
	}
}

//--------------------------------------------------------------------------
WORD CIsdnVideoParty::GetNumConnectedChannels()
{
	WORD numChnl = 0;
	for (WORD i = 0; i < MAX_CHNLS_IN_PARTY; i++)
	{
		if (m_pNetChnlCntl[i] != NULL && m_pNetChnlCntl[i]->IsConnected())
			numChnl++;
		else
			break;
	}

	return numChnl;
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnSmartRecovery()
{
	m_pConfApi->OnSmartRecovery(GetPartyId());
}

//--------------------------------------------------------------------------
void CIsdnVideoParty::OnPartyFreezePic(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;
	m_pMuxCntl->FreezePic();
}
//--------------------------------------------------------------------------
void CIsdnVideoParty::OnContentBrdgAmcVideoPartySetup(CSegment* pParam)
{
	TRACEINTO << "The event AMC_CONTENT_VIDEO_MODE with state PARTYSETUP is ignored ";
}
//--------------------------------------------------------------------------
std::string CIsdnVideoParty::GenerateCorrelationId(const char* initial_number)
{
	// *** Get the MAC address by its IP address ***
	CProcessBase* proc = CProcessBase::GetProcess();


	std::string MAC = proc->m_NetSettings.m_MacAddress;


	PASSERTMSG(MAC.size() == 0 || MAC.size() > 18, "Wrong MAC Address");
	// ***  retrieve the current local time's components ***

	timeval tv;
	gettimeofday(&tv, NULL);

	tm* timeinfo = localtime(&tv.tv_sec);

	// *** combine the Correlation ID out of components ***

	std::ostringstream os;

	if (NULL == timeinfo)
	{
		os << "";
		TRACEINTO << "localtime returned NULL (probably error)";
	}
	else
	{
		os
			<< initial_number << '-'
			<< (timeinfo->tm_year + 1900) << '-'
			<< std::setfill('0') << std::setw(2) << (timeinfo->tm_mon + 1) << '-'
			<< std::setfill('0') << std::setw(2) << timeinfo->tm_mday
			<< 'T'
			<< std::setfill('0') << std::setw(2) << timeinfo->tm_hour << ':'
			<< std::setfill('0') << std::setw(2) << timeinfo->tm_min << ':'
			<< std::setfill('0') << std::setw(2) << timeinfo->tm_sec << ".0Z"
			<< '-' << MAC;
	}

	const std::string& id(os.str());

	TRACEINTO << id;
	return id;
}

//--------------------------------------------------------------------------
#define _MCC Cancel_MCC
#define _MIZ Cancel_MIZ
#define _MIS Cancel_MIS
#define _MCV Cancel_MCV
#define _MIV Cancel_MIV
#define _VCS Cancel_VCS
#define _MIM Cancel_MIM

BEGIN_H230_TBL
	// opcode related to audio
	ONH230(AIM | Attr000,   CIsdnVideoParty::OnH230AudioMute                   ,"AIM",        "Audio indicate muted")
	ONH230(AIA | Attr000,   CIsdnVideoParty::OnH230AudioActive                 ,"AIA",        "Audio indicate active")
	ONH230(ACE | Attr000,   CIsdnVideoParty::OnH230AudioEqualize               ,"ACE",        "Audio command equalize")
	ONH230(ACZ | Attr000,   CIsdnVideoParty::OnH230AudioZeroDelay              ,"ACZ",        "Audio zero delay")
	// opcode related to video
	ONH230(VIS | Attr000,   CIsdnVideoParty::OnH230VideoIndSuppressed          ,"VIS",        "Video indicate suppressed")
	ONH230(VIA | Attr000,   CIsdnVideoParty::OnH230VideoActive                 ,"VIA",        "Video indicate active")
	// opcode related to chair control
	ONH230(CIC | Attr010,   CIsdnVideoParty::OnH230ChairCntlCap                ,"CIC",        "Chair control capbility")
	ONH230(CCA | Attr010,   CIsdnVideoParty::OnH230ChairCmdAcquire             ,"CCA",        "Chair token acquire")
	ONH230(CIS | Attr010,   CIsdnVideoParty::OnH230ChairStopToken              ,"CIS",        "Chair token release")
	ONH230(CIR | Attr010,   CIsdnVideoParty::OnH230ChairIndRefuse              ,"CIR",        "Chair indicate releas/refuse terminal drop")
	ONH230(CIT | Attr010,   CIsdnVideoParty::OnH230ChairIndToken               ,"CIT",        "Chair indicate token")
	ONH230(CCR | Attr010,   CIsdnVideoParty::OnH230ChairReleaseToken           ,"CCR",        "Chair token releas/refuse")
	ONH230(CCD | Attr010,   CIsdnVideoParty::OnH230ChairDropTerm               ,"CCD",        "Chair drop terminal")
	ONH230(CCK | Attr010,   CIsdnVideoParty::OnH230ChairDropConf               ,"CCK",        "Chair drop conference")
	ONH230(TIF | Attr010,   CIsdnVideoParty::OnH230FloorReq                    ,"TIF",        "OnH230FloorReq")
	ONH230(TIE | Attr010,   CIsdnVideoParty::OnH230IndicateEndOfString         ,"TIE",        "OnH230IndicateEndOfString")
	//ONH230(TCU | Attr001,   CIsdnVideoParty::NullActorReq                      ,"TIF",        "Request for floor")
	ONH230(TCU | Attr001,   CIsdnVideoParty::OnH230UpdateTerminalList          ,"TCU",        "Update terminal list")
	ONH230(TIA | Attr001,   CIsdnVideoParty::OnH230AssignTerminaNum            ,"TIA",        "Assign terminal number")
	ONH230(TIN | Attr001,   CIsdnVideoParty::OnH230IndTerminaNum               ,"TID",        "Indicate terminal number")
	ONH230(TID | Attr001,   CIsdnVideoParty::OnH230DropTerminaNum              ,"TID",        "Dropped  terminal number")

	ONH230(VCB | Attr001,   CIsdnVideoParty::OnH230ChairVideoBroadcast         ,"VCB",        "Chair video command broadcast")
	ONH230(VCE | Attr001,   CIsdnVideoParty::OnH230ChairCancelVideoBroadcast   ,"VCE",        "Chair video cancel command broadcast")
	ONH230(TCA | Attr001,   CIsdnVideoParty::OnH230TokenCommandAssociation     ,"TCA",        "OnH230CommandAssociation")

	// opcode related to data control
	ONH230(DCA_L | Attr010, CIsdnVideoParty::OnH230LsdAcquireToken             ,"DCA_L",      "Lsd acquire token")
	ONH230(DCA_H | Attr010, CIsdnVideoParty::OnH230HsdAcquireToken             ,"DCA_H",      "Hsd acquire token")
	ONH230(DIT_L | Attr010, CIsdnVideoParty::OnH230LsdIndicateToken            ,"DIT_L",      "Lsd indicate token")
	ONH230(DIT_H | Attr010, CIsdnVideoParty::OnH230HsdIndicateToken            ,"DIT_H",      "Hsd indicate token")
	ONH230(DIS_L | Attr010, CIsdnVideoParty::OnH230LsdStopToken                ,"DIS_L",      "Lsd stop using token")
	ONH230(DIS_H | Attr010, CIsdnVideoParty::OnH230HsdStopToken                ,"DIS_H",      "Hsd stop using token")
	ONH230(DCR_L | Attr010, CIsdnVideoParty::OnH230LsdReleaseToken             ,"DCR_L",      "Lsd releas/refuse token")
	ONH230(DCR_H | Attr010, CIsdnVideoParty::OnH230HsdReleaseToken             ,"DCR_H",      "Hsd releas/refuse token")
	ONH230(DCC_L | Attr010, CIsdnVideoParty::OnH230LsdCloseChannel             ,"DCC_L",      "Lsd close channel")

	// opcode related to terminal identification
	ONH230(TCI   | Attr000, CIsdnVideoParty::OnH230TerminalIdentify            ,"TCI",        "Terminal command identify")
	ONH230(TII   | Attr000, CIsdnVideoParty::OnH230TerminalIndIdentity         ,"TII",        "Terminal indicate identity")
	ONH230(TIS   | Attr000, CIsdnVideoParty::OnH230TerminalIndIdentityStop     ,"TIS",        "Terminal indicate identity stop")
	ONH230(TCS_0 | Attr011, CIsdnVideoParty::NullActionFunction                ,"TCS_0",      "Terminal string reservrd")
	ONH230(TCS_1 | Attr011, CIsdnVideoParty::OnH230TerminalPassword            ,"TCS_1",      "Terminal string password")
	ONH230(TCS_2 | Attr011, CIsdnVideoParty::OnH230TerminalIdentity            ,"TCS_2",      "Terminal string identity")
	ONH230(TCS_3 | Attr011, CIsdnVideoParty::OnH230TerminalConfIdentify        ,"TCS_3",      "Terminal string conf identify")
	ONH230(TCP_  | Attr011, CIsdnVideoParty::OnH230TerminalPersonaldentify     ,"TCP_",       "Terminal personal identify")
	ONH230(TCS_4 | Attr011, CIsdnVideoParty::OnH230TerminalExtensionAddress    ,"TCS_4",      "Terminal extension address")

	// opcode related to broadcast control - CURRENTLY DISABLED
	ONH230(MCV  | Attr001,  CIsdnVideoParty::OnH230MultiCmdVisualize           ,"MCV" ,       "Multipoint Visualize Force")
	ONH230(_MCV | Attr001,  CIsdnVideoParty::OnH230MultiCancelCmdVisualize     ,"_MCV",       "Cancel Multipoint Visualize Force")
	ONH230(MIV  | Attr001,  CIsdnVideoParty::OnH230MultiIndVisualize           ,"MIV" ,       "Multipoint Visualize Indication")
	ONH230(_MIV | Attr001,  CIsdnVideoParty::OnH230MultiCancelIndVisualize     ,"_MIV",       "Cancel Multipoint Visualize Indication")

	ONH230(MIV  | Attr001,  CIsdnVideoParty::NullActionFunction                ,"MIV" ,       "Multipoint Visualize Indication")
	ONH230(_MIV | Attr001,  CIsdnVideoParty::NullActionFunction                ,"_MIV",       "Cancel Multipoint Visualize Indication")

	ONH230(MVA  | Attr010,  CIsdnVideoParty::NullActionFunction                ,"MVA" ,       "Multipoint Visualization Achieved")
	ONH230(MVR  | Attr010,  CIsdnVideoParty::NullActionFunction                ,"MVR" ,       "Multipoint Visualization Refused/Revoked")

	// opcode related to select control - CURRENTLY DISABLED
	ONH230(VCS  | Attr001,  CIsdnVideoParty::OnH230VideoCmdSelect              ,"VCS" ,       "Multipoint Visualize Force")
	ONH230(_VCS | Attr001,  CIsdnVideoParty::OnH230CancelVideoCmdSelect        ,"_VCS",       "Cancel Multipoint Visualize Force")
	ONH230(VCR  | Attr001,  CIsdnVideoParty::NullActionFunction                ,"VCR",        "Reject video enforce")

	// opcode related to cascading
	ONH230(MIL  | Attr001,  CIsdnVideoParty::OnH230MultiIndLopp                ,"MIL" ,       "Multipoint indicate loop")
	ONH230(MIM  | Attr001,  CIsdnVideoParty::OnH230MultiIndMaster              ,"MIM" ,       "Multipoint indicate master")
	ONH230(RAN  | Attr001,  CIsdnVideoParty::OnH230RandomNum                   ,"RAN" ,       "Random number")

	// opcode related to other
	ONH230(VIN | Attr001,   CIsdnVideoParty::OnH230IndVideoSrcNum              ,"VIN",        "Indicate video src number")
	ONH230(MIZ | Attr001,   CIsdnVideoParty::OnH230ZeroCom                     ,"MIZ",        "Multipoint zero communication")
	ONH230(MIS | Attr001,   CIsdnVideoParty::OnH230SecondaryStatus             ,"MIS",        "Multipoint secondery status")
	ONH230(_MCC | Attr001,  CIsdnVideoParty::NullActionFunction                ,"Cancel_MCC", "Cancel multipoint command conference")
	ONH230(_MIZ | Attr001,  CIsdnVideoParty::OnH230CancelZeroCom               ,"Cancel_MIZ", "Cancel multipoint command symmetrical")
	ONH230(_MIS | Attr001,  CIsdnVideoParty::OnH230CancelSecondaryStatus       ,"Cancel_MIS", "Cancel multipoint secondery status")
	ONH230(_MIM | Attr001,  CIsdnVideoParty::OnH230CancelMultiIndMaster        ,"Cancel_MIM", "Cancel Multipoint indicate master")
	ONH230(DCM  | Attr010,  CIsdnVideoParty::OnH230MlpData                     ,"DCM",        "Data command MLP")
	// probably will be send in exchange cap sequence
	ONH230(MCS | Attr001,   CIsdnVideoParty::OnH230MultiCmdSymetric            ,"MCS",        "Multipoint command symmetrical")
	ONH230(MCC | Attr001,   CIsdnVideoParty::OnH230MultiCmdConf                ,"MCC",        "Multipoint command conference")
	ONH230(MIH | Attr001,   CIsdnVideoParty::OnH230MultiIndicateHierarchy	     ,"MIH",        "Multipoint Indicate Hierarchy")

END__H230_TBL
