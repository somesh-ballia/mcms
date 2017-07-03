#include "IsdnVideoPartyOut.h"
#include "ConfPartyOpcodes.h"
#include "Trace.h"
#include "TraceStream.h"
#include "OpcodesMcmsBonding.h"
#include "H263.h"
#include "CdrPersistHelper.h"

const WORD UPDATE_CHANNEL_ACK = 1;   // waiting for channel acknowledge
const WORD TEST_TIMER         = 100;

void PartyVideoOutEntryPoint(void* appParam)
{
	CIsdnVideoPartyOut* pPartyTaskApp = new CIsdnVideoPartyOut;
	pPartyTaskApp->Create(*(CSegment*)appParam);
}

PBEGIN_MESSAGE_MAP(CIsdnVideoPartyOut)

	ONEVENT(ESTABLISHCALL,         IDLE,       CIsdnVideoPartyOut::OnConfEstablishCallIdle)
	ONEVENT(NETCONNECT,            PARTYSETUP, CIsdnVideoPartyOut::OnNetConnectSetUp)
	ONEVENT(UPDATECHANNELACK,      ANYCASE,    CIsdnVideoPartyOut::OnUpdateChannelAck)

	ONEVENT(UPDATECHANNELACK,      PARTYSETUP, CIsdnVideoPartyOut::OnUpdateChannelAck)
	ONEVENT(REALLOCATERTM_ACK,     PARTYSETUP, CIsdnVideoPartyOut::OnConfReallocateRtmAck)

	ONEVENT(CHANNEL_DIALING_TIMER, PARTYSETUP, CIsdnVideoPartyOut::OnTimerDialAdditionalChannel)
	ONEVENT(CHANNEL_DIALING_TIMER, ANYCASE,    CIsdnVideoPartyOut::NullActionFunction)

	ONEVENT(BND_END_NEGOTIATION,   PARTYSETUP, CIsdnVideoPartyOut::OnBondEndnegotiationSetup)

	ONEVENT(BOARDFULL_ACK,         PARTYSETUP, CIsdnVideoPartyOut::OnBoardFullAck)

	ONEVENT(BONDDISCONNECT,        PARTYSETUP, CIsdnVideoPartyOut::OnBondDisConnect)
	ONEVENT(TEST_TIMER,            ANYCASE,    CIsdnVideoPartyOut::OnTestTimer)

PEND_MESSAGE_MAP(CIsdnVideoPartyOut, CIsdnVideoParty);

////////////////////////////////////////////////////////////////////////////
//                        CIsdnVideoPartyOut
////////////////////////////////////////////////////////////////////////////
CIsdnVideoPartyOut::CIsdnVideoPartyOut() : CIsdnVideoParty()
{
	m_numOfChannels        = 0;
	m_numChannelsConnected = 0;
	m_lastChannelDial      = 0;

	VALIDATEMESSAGEMAP;
}

//--------------------------------------------------------------------------
CIsdnVideoPartyOut::~CIsdnVideoPartyOut()
{
	for (WORD i = 0; i < MAX_CHNLS_IN_PARTY; i++)
		POBJDELETE(m_pNetChnlCntl[i]);
}

//--------------------------------------------------------------------------
void CIsdnVideoPartyOut::Create(CSegment& appParam)
{
	CIsdnVideoParty::Create(appParam);

#ifdef MESSAGE_QUEUE_BLOCKING_RESERCH
	m_pConfApi = new CConfApi(GetMonitorConfId());
#else
	m_pConfApi = new CConfApi;
#endif
	m_pConfApi->CreateOnlyApi(*m_pCreatorRcvMbx, NULL, NULL, 1);
}

//--------------------------------------------------------------------------
void CIsdnVideoPartyOut::OnConfEstablishCallIdle(CSegment* pParam)
{
// OnConfEstablishCall(pParam);
	*pParam >> m_PartyRsrcID;
	*pParam >> m_mcuNum;
	*pParam >> m_termNum;

	TRACESTR(eLevelInfoNormal) << " CIsdnVideoPartyOut::OnConfEstablishCallIdle  :  Name - "<< PARTYNAME
	                           << "\nterminalNum="<< m_termNum<<"\nmcuNumber="<< m_mcuNum<< "\npartyRsrcId="<< m_PartyRsrcID;

	for (WORD i = 0; i < 1; i++)
		m_pNetSetUp[i].DeSerialize(NATIVE, *pParam);

	*pParam >> m_numOfChannels;

	for (WORD j = 0; j < m_numOfChannels; j++)
		m_pNetRsrcParams[j].DeSerialize(NATIVE, *pParam);

	/* deserialize the CIsdnPartyRsrcDesc */
	CIsdnPartyRsrcDesc pPartyRsrcDesc;
	pPartyRsrcDesc.DeSerialize(NATIVE, *pParam);
	m_ConfRsrcId = pPartyRsrcDesc.GetConfRsrcId();

	m_pTargetComMode->DeSerialize(NATIVE, *pParam);
	CCapH320 localCap;
	localCap.DeSerialize(NATIVE, *pParam);

	*pParam >> m_RoomId;

	/* fill m_pNetSetUp array from the CIsdnPartyRsrcDesc */
	SetNetSetup(pPartyRsrcDesc);

	CRsrcParams muxRsrcParams;
	WORD mux_desc_found = pPartyRsrcDesc.GetRsrcParams(muxRsrcParams, eLogical_mux);
	if (!mux_desc_found)
	{
		PTRACE2(eLevelError, "CIsdnVideoPartyOut::OnConfEstablishCallIdle : Mux descriptor not valid, ", PARTYNAME);
		PASSERT_AND_RETURN(1);
	}

	char tmpPhoneNumber[ISDN_PHONE_NUMBER_DIGITS_LEN];
	tmpPhoneNumber[0] = '\0';
	m_pBndCntl = new CBondingCntl(DIALOUT, m_numOfChannels, tmpPhoneNumber, muxRsrcParams, this);

	m_pMuxCntl = new CMuxCntl(muxRsrcParams, this);
	m_pMuxCntl->SetLocalCaps(localCap);

	m_pBndMuxCntl = new CBondingMuxCntl(m_pBndCntl, m_pMuxCntl, muxRsrcParams, this);

	/* set parameters to repeated230 */
	DialChannel();

	m_state = PARTYSETUP;

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
			//Send Info to CDR for ISDN dial out
			WORD dummy;
			char initial_number[PHONE_NUMBER_DIGITS_LEN+1];
			m_pNetSetUp[0].GetCalledNumber(&dummy, initial_number);

			pConfParty->SetCorrelationId(GenerateCorrelationId(initial_number));
			pCommConf->PartyCorrelationDataToCDR(pConfParty->GetName(), m_pParty->GetMonitorPartyId(), pConfParty->GetCorrelationId());
			PlcmCdrEventCallStartExtendedHelper cdrEventCallStartExtendedHelper;
			cdrEventCallStartExtendedHelper.SetNewIsdnUndefinedParty_BasicAndContinue(*pConfParty, ISDN_INTERFACE_TYPE, *pCommConf);
			pCommConf->SendCdrEvendToCdrManager((ApiBaseObjectPtr)&cdrEventCallStartExtendedHelper.GetCdrObject(), false, cdrEventCallStartExtendedHelper.GetCdrObject().m_partyDetails.m_id);
		}
	}
}

//--------------------------------------------------------------------------
void CIsdnVideoPartyOut::OnNetConnectSetUp(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CIsdnVideoPartyOut::OnNetConnectSetUp : Name - ", PARTYNAME);

	WORD seqNum, status;
	BYTE cause = 0;
	*pParam >> seqNum >> status >> cause;

	if (status)
	{
		ostringstream str;
		str << "seqNum = " << seqNum << " , status = " << status << " , cause = " << (WORD)cause << " , Name - " << PARTYNAME;
		PTRACE2(eLevelInfoNormal, "CIsdnVideoPartyOut::OnNetConnectSetUp -  party disconnected: ", str.str().c_str());

		m_pConfApi->UpdateDB(this, DISCAUSE, NO_NET_CONNECTION, 1);
		UpdateQ931DisCause(cause);
		if (m_conf_has_been_notifyed == FALSE)
		{
			m_pConfApi->PartyConnect(GetPartyId(), NO_NET_CONNECTION);
			m_conf_has_been_notifyed = TRUE;
		}
		return;
	}

	/* update net channel status in EMA and CDR */
	DWORD chnlToggle = 0xC0000000;
	chnlToggle |= (seqNum + 1);
	m_pConfApi->UpdateDB(this, NETCHNL, chnlToggle, 1);

	/* send update to CAddIsdnPartyCntl */
	m_pConfApi->UpdateNetChannel(GetPartyId(), m_pNetSetUp[seqNum].m_boardId,
	                             m_pNetSetUp[seqNum].m_spanId[0], // olga?
	                             m_pNetSetUp[seqNum].m_net_connection_id,
	                             seqNum);

	/* bonding control */
	m_numChannelsConnected++;
	if (m_numChannelsConnected == 1)      /* if this is an initial channel */
	{
		PTRACE2(eLevelInfoNormal, "CIsdnVideoPartyOut::OnNetConnectSetUp : \' INITIAL CHANNEL CONNECTED \' ", PARTYNAME);
		m_pBndCntl->Connect();
	}
	else     /* this is an additional bonding channel */
	{
		PTRACE2(eLevelInfoNormal, "CIsdnVideoPartyOut::OnNetConnectSetUp : \' ADDITIONAL CHANNEL CONNECTED  \' ", PARTYNAME);
		m_pBndCntl->AddChannel();
	}

	// dial next channel if needed
	if (m_numChannelsConnected > 1 && m_numChannelsConnected < m_numOfChannels && GetDialingMethod() == DIALING_METHOD_SEQUENTIAL)
	{
		PTRACE2(eLevelInfoNormal, "CIsdnVideoPartyOut::OnNetConnectSetUp : \' DIALING NEXT ADDITIONAL CHANNEL (SEQUENTIAL)  \' ", PARTYNAME);
		DialNextAdditionalChannel();
	}
	else if (m_numChannelsConnected == m_numOfChannels)   /* all additional channels have been connected */
	{
		PTRACE2(eLevelInfoNormal, "CIsdnVideoPartyOut::OnNetConnectSetUp : \' ALL ADDITIONAL CHANNELS CONNECTED \' ", PARTYNAME);
	}
}

//--------------------------------------------------------------------------
void CIsdnVideoPartyOut::OnUpdateChannelAck(CSegment* pParam)
{
	DWORD monitor_conf_id, monitor_party_id, connection_id, status, channelNum;
	*pParam >> monitor_conf_id >> monitor_party_id >> connection_id >> status >> channelNum;
	TRACESTR(eLevelInfoNormal) << " CIsdnVideoPartyOut::OnUpdateChannelAck   Name - " << PARTYNAME << " : "
	                           << monitor_conf_id << ", " << monitor_party_id << ", "
	                           << connection_id << ", " << status << ", " << channelNum;

	CIsdnNetSetup netSetup;
	netSetup.DeSerialize(NATIVE, *pParam);

	for (WORD i = 0; i < MAX_CHNLS_IN_PARTY; i++)
	{
		if (m_pNetSetUp[i].m_net_connection_id == connection_id)
			if (m_pNetChnlCntl[i])
			{
				m_pNetChnlCntl[i]->SetUpdated(TRUE);
				break;
			}
	}
}

//--------------------------------------------------------------------------
void CIsdnVideoPartyOut::OnBondEndnegotiationSetup(CSegment* pParam)
{
	BYTE  need_reallocate = 0;
	DWORD number_of_channels;

	*pParam >> need_reallocate >> number_of_channels;

	ostringstream str;
	str << "number_of_channels = " << number_of_channels << " , m_numOfChannels = " << m_numOfChannels << " , need_reallocate = " << (WORD)need_reallocate << ", Name - " << PARTYNAME;
	PTRACE2(eLevelInfoNormal, "CIsdnVideoPartyOut::OnBondEndnegotiationSetup: ", str.str().c_str());

	if (m_numOfChannels != number_of_channels)
	{
		// update number of channel for EP end negotiation
		m_numOfChannels = number_of_channels;
		if (CPObject::IsValidPObjectPtr(m_pTargetComMode))
		{
			WORD newXferMode = CXferMode::GetXferModeByNumChannels(m_numOfChannels);
			m_pTargetComMode->SetXferMode(newXferMode);
		}
	}

	if (need_reallocate)  // if we reallocate RTM we can dial inly after Ack (means shared memory updated)
	{
		PTRACE2(eLevelInfoNormal, "CIsdnVideoPartyOut::OnBondEndnegotiationSetup : Reallocate RTM channels, Name - ", PARTYNAME);

		m_pConfApi->ReallocateBondingChannels(GetPartyId(), number_of_channels);
	}
	else
	{
		// set bonding additional numbers in net channel control
		SetBondingAdditionalPhoneNumbers();

		DialNextAdditionalChannel();
	}
}

//--------------------------------------------------------------------------
void CIsdnVideoPartyOut::OnTimerDialAdditionalChannel()
{
	PTRACE2(eLevelInfoNormal, "CIsdnVideoPartyOut::OnTimerDialAdditionalChannel : Name - ", PARTYNAME);
	if (m_lastChannelDial < m_numOfChannels)
	{
		// dial current channel
		DialChannel();
		// if we dialing by timer, or first group of sequential dialing - dial next channel
		WORD  dialin_method      = GetDialingMethod();
		DWORD numChannelsInGroup = GetNumChannelsInGroup();
		if (m_lastChannelDial < numChannelsInGroup+1 || dialin_method == DIALING_METHOD_BY_TIMER)
		{
			DialNextAdditionalChannel();
		}
	}
	else
	{
		PTRACE2(eLevelError, "CIsdnVideoPartyOut::OnTimerDialAdditionalChanne ALL ADDITIONAL CHANNELS DIALED, Name - ", PARTYNAME);
	}
}

//--------------------------------------------------------------------------
void CIsdnVideoPartyOut::OnConfReallocateRtmAck(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CIsdnVideoPartyOut::OnConfReallocateRtmAck : Name - ", PARTYNAME);

	CIsdnPartyRsrcDesc pPartyRsrcDesc;
	pPartyRsrcDesc.DeSerialize(NATIVE, *pParam);
	BYTE               allocationFailed;
	*pParam>> allocationFailed;
	if (allocationFailed)
	{
		PTRACE2(eLevelInfoNormal, "CIsdnVideoPartyOut::OnConfReallocateRtmAck ALLOCATION FAILED!!!  Name - ", PARTYNAME);
		m_pConfApi->UpdateDB(this, DISCAUSE, NET_PORT_DEFICIENCY, 1);
		if (m_conf_has_been_notifyed == FALSE)
		{
			m_pConfApi->PartyConnect(GetPartyId(), NET_PORT_DEFICIENCY);
			m_conf_has_been_notifyed = TRUE;
		}
		return;
	}

	ostringstream str;
	int items_removed = 0;
	int net_index     = 0;
	while ((net_index+items_removed) < MAX_CHNLS_IN_PARTY)
	{
		str << "m_pNetSetUp[" << net_index << "].m_net_connection_id = " << m_pNetSetUp[net_index].m_net_connection_id << "\n";
		CRsrcDesc tempRsrcDesc;
		if (pPartyRsrcDesc.GetRsrcDesc(tempRsrcDesc, m_pNetSetUp[net_index].m_net_connection_id))
		{
			net_index++;
			str << "found \n";
		}
		else
		{
			str << "not found - removing\n";
			RemoveNetChannel(net_index);
			items_removed++;
		}
	}

	PTRACE2(eLevelInfoNormal, "CIsdnVideoPartyOut::OnConfReallocateRtmAck:\n", str.str().c_str());
	ostringstream str1;
	for (int net_index = 0; net_index < MAX_CHNLS_IN_PARTY; net_index++)
	{
		str1 << "m_pNetSetUp[" << net_index << "].m_net_connection_id = " << m_pNetSetUp[net_index].m_net_connection_id << "\n";
	}

	PTRACE2(eLevelInfoNormal, "CIsdnVideoPartyOut::OnConfReallocateRtmAck  testing update:\n", str1.str().c_str());

	// set bonding additional numbers in net channel control
	SetBondingAdditionalPhoneNumbers();

	DialNextAdditionalChannel();
}

//--------------------------------------------------------------------------
void CIsdnVideoPartyOut::OnBoardFullAck(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CIsdnVideoPartyOut::OnBoardFullAck : Name - ", PARTYNAME);

	CIsdnPartyRsrcDesc pPartyRsrcDesc;
	pPartyRsrcDesc.DeSerialize(NATIVE, *pParam);
	BYTE allocationFailed;
	*pParam>> allocationFailed;
	if (allocationFailed)
	{
		PTRACE2(eLevelInfoNormal, "CIsdnVideoPartyOut::OnBoardFullAck ALLOCATION FAILED!!!  Name - ", PARTYNAME);
		return;
	}

	WORD chnl = 0;
	/* find a non zero 'retry' flag. It means this channel was last disconnected due BOARD_FULL */
	for (WORD i = 0; i < MAX_CHNLS_IN_PARTY; i++)
	{
		if (m_retryNetCall[i] != 0)
		{
			chnl = i;
			TRACESTR(eLevelInfoNormal) << " CIsdnVideoPartyOut::OnBoardFullAck : get ACK for channel = " << chnl << ", " << PARTYNAME;
			break;
		}
	}

	/* no need to update RsrcParams related to this channel since nothing was changed */
	// m_pNetRsrcParams[chnl] = ...

	/* update IsdnNetSetup related to this channel */
	std::vector<CIsdnSpanOrderPerConnection*>* connectVector = pPartyRsrcDesc.GetSpanOrderPerConnectionVector();
	WORD connectSize = connectVector ? connectVector->size() : 0;
	for (WORD i = 0; i < connectSize; i++)
	{
		CIsdnSpanOrderPerConnection* connection_i = connectVector->at(i);
		DWORD connection_id = connection_i->GetConnectionId();

		if (m_pNetSetUp[chnl].m_net_connection_id == connection_id)
		{
			/* update board_id */
			WORD               board_id = connection_i->GetBoardId();
			m_pNetSetUp[chnl].m_boardId = board_id;

			/* update spans order */
			std::vector<WORD>* spansOrderVector = connection_i->GetSpansListVector();
			WORD spansOrderSize = spansOrderVector ? spansOrderVector->size() : 0;
			for (WORD s = 0; s < spansOrderSize; s++)
			{
				WORD span = spansOrderVector->at(s);
				m_pNetSetUp[chnl].m_spanId[s] = span;
			}
			break;
		}
	}

	/* zero the 'retry' flag */
	m_retryNetCall[chnl] = 0;
	/* update related CNetChnlCntl */
	m_pNetChnlCntl[chnl]->Reconnect(m_pNetSetUp[chnl]);
}

//--------------------------------------------------------------------------
void CIsdnVideoPartyOut::DumpIsdnPartyRsrcDesc(CIsdnPartyRsrcDesc& pPartyRsrcDesc)
{
	/* fill m_pNetSetUp array from the CIsdnPartyRsrcDesc */
	std::vector<CIsdnSpanOrderPerConnection*>* ConnectionVector = pPartyRsrcDesc.GetSpanOrderPerConnectionVector();
	WORD connectionSize = ConnectionVector ? ConnectionVector->size() : 0;

	CLargeString str;

	for (WORD i = 0; i < connectionSize; i++)
	{
		CIsdnSpanOrderPerConnection* connection_i = ConnectionVector->at(i);
		DWORD connection_id = connection_i->GetConnectionId();
		WORD board_id = connection_i->GetBoardId();

		if (connection_id == 0)
			break;

		std::vector<WORD>* spansOrderVector = connection_i->GetSpansListVector();
		WORD spansOrderSize = spansOrderVector ? spansOrderVector->size() : 0;

		/* initialize the current CIsdnNetSetup from the first element */
// m_pNetSetUp[i] = m_pNetSetUp[0];
		/* fill boardID, connectionID and spans order from the CIsdnSpanOrderPerBoard object */
// m_pNetSetUp[i].m_boardId = board_id;
// m_pNetSetUp[i].m_net_connection_id = connection_id;

		str << "pNetSetUp["<< i <<"] - boardId:" << board_id << ", net_connection_id:" << connection_id <<", span order:{";

		for (WORD s = 0; s < spansOrderSize; s++)
		{
			WORD span = spansOrderVector->at(s);
			m_pNetSetUp[i].m_spanId[s] = span;

			str << span;
			if (s != spansOrderSize-1)
				str << ",";
		}
		str << "}\n";
	}

	str << "temp phone number = " << pPartyRsrcDesc.GetTmpPhoneNumber() << ", Name - " << PARTYNAME;

	PTRACE2(eLevelInfoNormal, "CIsdnVideoPartyOut::SetNetSetup:\n", str.GetString());
}

//--------------------------------------------------------------------------
void CIsdnVideoPartyOut::SetBondingAdditionalPhoneNumbers()
{
	// get initial phone number
	WORD initialPhoneNumDigits = 0;
	char initialPhoneNumber[PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER];
	memset(initialPhoneNumber, '\0', PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER);
	m_pNetSetUp[0].GetCalledNumber(&initialPhoneNumDigits, initialPhoneNumber);

	// get '#' suffix from initial channel
	// we currently support only # suffix
	char suffix = '\0';
	if (initialPhoneNumber[strlen(initialPhoneNumber)-1] == '#')
	{
		suffix = '#';
		initialPhoneNumber[strlen(initialPhoneNumber)-1] = '\0';  // delete the suffix for further calculation of prfix by phone length
	}

	// initiate longestPhoneNumber with initialPhoneNumber
	WORD longestPhoneNumberLen = 0;
	char longestPhoneNumber[PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER];
	memset(longestPhoneNumber, '\0', PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER);
	strncpy(longestPhoneNumber, initialPhoneNumber, PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER);
	longestPhoneNumberLen = strlen(longestPhoneNumber);

	// rules for adding phone numbers :
	// 1) a zero length number means to copy the previous recived number
	// 2) a number shorter the the longest_phone_number recived so far will change
	// only the least segnificant digits of the longestPhoneNumber
	// 3) a new phone number becomes the longest_phone_number if its legth is equal or greater
	// than the current longestPhoneNumber
	char channelPhoneNumber[PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER];
	memset(channelPhoneNumber, '\0', PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER);
	strncpy(channelPhoneNumber, initialPhoneNumber, PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER);

	for (int channel_index = 1; channel_index < m_numOfChannels; channel_index++)
	{
		char bonding_number[BND_MAX_PHONE_LEN+1];
		memset(bonding_number, '\0', BND_MAX_PHONE_LEN+1);
		m_pBndCntl->GetBondingPhoneNumber(channel_index, bonding_number);
		bonding_number[BND_MAX_PHONE_LEN] = '\0';

		WORD bondingPhoneNumberLen = strlen(bonding_number);

		if (bondingPhoneNumberLen == 0)
		{
			// do nothing channelPhoneNumber not changed it set to the previos phone num
			// add trace
		}
		else if (bondingPhoneNumberLen < longestPhoneNumberLen)
		{
			WORD prefixLen = longestPhoneNumberLen - bondingPhoneNumberLen;
			for (int digit_index = prefixLen; digit_index < longestPhoneNumberLen; digit_index++)
			{
				// copy least segnificant bytes
				channelPhoneNumber[digit_index] = bonding_number[digit_index- prefixLen];
			}
		}
		else
		{
			strncpy(channelPhoneNumber, bonding_number, BND_MAX_PHONE_LEN);
			strncpy(longestPhoneNumber, channelPhoneNumber, PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER);
			longestPhoneNumberLen = strlen(longestPhoneNumber);
		}

		// add '\0' or '#' suffix from initial channel
		// channelPhoneNumber is always longestPhoneNumberLen
		channelPhoneNumber[longestPhoneNumberLen] = suffix;

		ostringstream str;
		str << "channel_index = " << channel_index << " , bonding_number  = " <<  bonding_number << " , channel_number  = " << channelPhoneNumber << " , Name - " << PARTYNAME;
		PTRACE2(eLevelInfoNormal, "CIsdnVideoPartyOut:: SetBondingAdditionalPhoneNumbers: ", str.str().c_str());

		m_pNetSetUp[channel_index].SetCalledNumber(strlen(channelPhoneNumber), channelPhoneNumber);
	}
}

//--------------------------------------------------------------------------
void CIsdnVideoPartyOut::EstablishOutCall(WORD chnl_number)
{
	PTRACE2(eLevelInfoNormal, "CIsdnVideoPartyOut::EstablishOutCall : Name - ", PARTYNAME);

	WORD chnl = chnl_number-1;

	WORD dummy;
	char initial_number[PHONE_NUMBER_DIGITS_LEN+1];
	ALLOCBUFFER(tmp, 2*H243_NAME_LEN + 150);  // max size of PARTYNAME is 2*H243_NAME_LEN+50 and initial_number is PHONE_NUMBER_DIGITS_LEN+1

	m_pNetSetUp[chnl].GetCalledNumber(&dummy, initial_number);
	sprintf(tmp, "[%s] - Dialing OUT Channel Number %d, Phone Number %s", PARTYNAME, chnl_number, initial_number);
	PTRACE2(eLevelInfoNormal, " ---> ", tmp);
	DEALLOCBUFFER(tmp)

	m_pNetChnlCntl[chnl] = new CNetChnlCntl;
	m_pNetChnlCntl[chnl]->Create(m_pNetRsrcParams[chnl], m_pNetSetUp[chnl], this, chnl, DIALOUT);
	m_pNetChnlCntl[chnl]->StartNetCntl();
	MonitorPartyPhoneNumbers((BYTE)chnl, &m_pNetSetUp[chnl], (BYTE)1);
}

//--------------------------------------------------------------------------
void CIsdnVideoPartyOut::DestroyNetConnection(WORD chnl)
{
	PASSERT_AND_RETURN(/*chnl < 0 ||*/ chnl >= MAX_CHNLS_IN_PARTY);

	m_pNetChnlCntl[chnl]->Destroy();

	POBJDELETE(m_pNetChnlCntl[chnl]);

	m_retryNetCall[chnl] = 0;
}

//--------------------------------------------------------------------------
void CIsdnVideoPartyOut::DialChannel()
{
	ostringstream str;
	str << "dialling channel number = " << (m_lastChannelDial+1) << " , party  = " <<  PARTYNAME;
	PTRACE2(eLevelInfoNormal, "CIsdnVideoPartyOut::DialChannel: ", str.str().c_str());

	if (m_lastChannelDial < m_numOfChannels)
	{
		EstablishOutCall(m_lastChannelDial+1);
		m_lastChannelDial++;
	}
	else
		PTRACE2(eLevelError, "CIsdnVideoPartyOut::DialChannel ALL CHANNELS DIALED, ", PARTYNAME);
}

//--------------------------------------------------------------------------
void CIsdnVideoPartyOut::DialNextAdditionalChannel()
{
	if (m_lastChannelDial < m_numOfChannels)
	{
		DWORD next_channel_timer = TimerToWaitBeforeDialAdditinalChannl(m_lastChannelDial+1);
		ostringstream str;
		str << "next channel number = " << (m_lastChannelDial+1) <<" , timer to wait = " << next_channel_timer << " , party  = " << PARTYNAME;
		PTRACE2(eLevelInfoNormal, "CIsdnVideoPartyOut::DialNextAdditionalChannel: ", str.str().c_str());
		if (next_channel_timer > 0)
		{
			StartTimer(CHANNEL_DIALING_TIMER, next_channel_timer);
		}
		else      // next_channel_timer==0
		{
			DialChannel();
			// if we dialing by timer, or first group of sequential dialing - dial next channel
			WORD  dialin_method      = GetDialingMethod();
			DWORD numChannelsInGroup = GetNumChannelsInGroup();
			if (m_lastChannelDial < numChannelsInGroup+1 || dialin_method == DIALING_METHOD_BY_TIMER)
			{
				ostringstream str1;
				str1 << "m_lastChannelDial = " << m_lastChannelDial <<" , numChannelsInGroup = " << numChannelsInGroup <<  " , party  = " <<  PARTYNAME;
				PTRACE2(eLevelInfoNormal, "CIsdnVideoPartyOut::DialNextAdditionalChannel: ", str1.str().c_str());
				DialNextAdditionalChannel();
			}
		}
	}
	else
	{
		PTRACE2(eLevelError, "CIsdnVideoPartyOut::DialAdditionalChannel ALL ADDITIONAL CHANNELS DIALED, ", PARTYNAME);
	}
}

//--------------------------------------------------------------------------
DWORD CIsdnVideoPartyOut::TimerToWaitBeforeDialAdditinalChannl(WORD channel_number)
{
	// channel_number is next channel to dial; initial channel=1 , first additional channel=2
	DWORD timer_value        = 0;
	WORD  dialin_method      = GetDialingMethod();
	DWORD channel_delay      = GetChannelDelay();
	DWORD numChannelsInGroup = GetNumChannelsInGroup();
	DWORD group_delay        = GetGroupDelay();

	// do not wait before dialing initial channel
	if (channel_number == 1)
	{
		return 0;
	}

	// dialing of first group in both methods
	if (channel_number < numChannelsInGroup+1)
	{
		timer_value = channel_delay;
	}
	else if (dialin_method == DIALING_METHOD_BY_TIMER)
	{
		WORD group = (channel_number-1)/numChannelsInGroup;     // casting to WORD gives the group
		if (group*numChannelsInGroup+1 == channel_number)
		{
			timer_value = group_delay;
		}
		else
		{
			timer_value = channel_delay;
		}
	}

	ostringstream str;
	str << "channel_number = " << channel_number << " , timer_value = " << timer_value << ", Name - " << PARTYNAME;
	PTRACE2(eLevelInfoNormal, "CIsdnVideoPartyOut::TimerToWaitBeforeDialAdditinalChannl: ", str.str().c_str());
	return timer_value;
}

//--------------------------------------------------------------------------
WORD CIsdnVideoPartyOut::GetDialingMethod()
{
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	std::string dialing_method;
	sysConfig->GetDataByKey("BONDING_DIALING_METHOD", dialing_method);
	TRACESTR(eLevelInfoNormal) << " CIsdnVideoPartyOut::GetDialingMethod: " << dialing_method.c_str() << ",  Name - " << PARTYNAME;
	if (strncmp(dialing_method.c_str(), "BY_TIMERS", sizeof("BY_TIMERS")) == 0)
	{
		return DIALING_METHOD_BY_TIMER;
	}

	if (strncmp(dialing_method.c_str(), "SEQUENTIAL", sizeof("SEQUENTIAL")) == 0)
	{
		return DIALING_METHOD_SEQUENTIAL;
	}

	PTRACE2(eLevelInfoNormal, "CIsdnVideoPartyOut::GetDialingMethod: failed to get dialing method, dial by timers, Name - ", PARTYNAME);
	return DIALING_METHOD_BY_TIMER;
}

//--------------------------------------------------------------------------
DWORD CIsdnVideoPartyOut::GetChannelDelay()
{
	PTRACE2(eLevelInfoNormal, "CIsdnVideoPartyOut::GetChannelDelay temporary for debug - fixed delay between channels, Name - ", PARTYNAME);

	CSysConfig* sysConfig     = CProcessBase::GetProcess()->GetSysConfig();
	DWORD       channel_delay = 0;
	if (FALSE == sysConfig->GetDWORDDataByKey("BONDING_CHANNEL_DELAY", channel_delay))
		channel_delay = 4;

	return channel_delay;
}

//--------------------------------------------------------------------------
DWORD CIsdnVideoPartyOut::GetGroupDelay()
{
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	DWORD group_delay = 0;
	sysConfig->GetDWORDDataByKey("BONDING_GROUP_DELAY", group_delay);
	return group_delay;
}

//--------------------------------------------------------------------------
DWORD CIsdnVideoPartyOut::GetNumChannelsInGroup()
{
	CSysConfig* sysConfig          = CProcessBase::GetProcess()->GetSysConfig();
	DWORD       numChannelsInGroup = 0;
	sysConfig->GetDWORDDataByKey("BONDING_NUM_CHANNELS_IN_GROUP", numChannelsInGroup);
	return numChannelsInGroup;
}

//--------------------------------------------------------------------------
// this function will stop the dialling of additional channels
void CIsdnVideoPartyOut::StopDial()
{
	m_lastChannelDial = 0xff;
}

//--------------------------------------------------------------------------
void CIsdnVideoPartyOut::OnBondDisConnect(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CIsdnVideoPartyOut::OnBondDisConnect : Name - ", PARTYNAME);

	DWORD bonding_status;

	*pParam >> bonding_status;

	StopDial(); // stop dialin the additional channel

	m_pConfApi->PartyConnect(GetPartyId(), bonding_status); // ask conf to disconnect
}

//--------------------------------------------------------------------------
void CIsdnVideoPartyOut::OnTestTimer(CSegment* pParam)
{
	CCapH320 partyCapsTest;
	CCapH320 partyCapsTest2;
	partyCapsTest.CreateDefault();
	partyCapsTest.SetAudioCap(e_A_Law);
	partyCapsTest.GetCapH263()->SetOneH263Cap(H263_CIF, MPI_1);

	PTRACE2(eLevelInfoNormal, "** Eitan Debug: CIsdnVideoPartyOut::OnTestTimer local caps before serialize, ", PARTYNAME);
	partyCapsTest.Dump();

	CComMode partyComModeTest;
	CComMode partyComModeTest2;
	CAudMode partyAudioModeTest;
	CVidMode partyVidModeTest;

	partyAudioModeTest.SetBitRate(A_Law_OF);
	partyVidModeTest.SetFreeBitRate(TRUE);

	partyComModeTest.SetXferMode(Xfer_384);
	partyComModeTest.SetAudMode(partyAudioModeTest);
	partyComModeTest.SetVidMode(partyVidModeTest);

	PTRACE2(eLevelInfoNormal, "** Eitan Debug: CIsdnVideoPartyOut::OnTestTimer local commMode before serialize, ", PARTYNAME);
	partyComModeTest.Dump(1);

	m_state = PARTYCONNECTED;
	m_pConfApi->IsdnPartyConnect(GetPartyId(), partyCapsTest, partyComModeTest, statOK);
}
