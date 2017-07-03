#include "NStream.h"
#include "ConfApi.h"
#include  "IsdnVoicePartyOut.h"
#include "ConfPartyOpcodes.h"
#include "Trace.h"
#include "IpCommon.h"

// party states
const WORD DISCONNECTING = 4;
const WORD VTX_SETUP     = 5;

void PartyVoiceOutEntryPoint(void* appParam)
{
	CIsdnVoicePartyOut* pPartyTaskApp = new CIsdnVoicePartyOut;
	pPartyTaskApp->Create(*(CSegment*)appParam);
}

PBEGIN_MESSAGE_MAP(CIsdnVoicePartyOut)
	ONEVENT(ESTABLISHCALL, IDLE,       CIsdnVoicePartyOut::OnConfEstablishCallIdle)
	ONEVENT(ESTABLISHCALL, PARTYSETUP, CIsdnVoicePartyOut::OnConfEstablishCallSetup)
	ONEVENT(NETCONNECT,    PARTYSETUP, CIsdnVoicePartyOut::OnNetConnectSetUp)
PEND_MESSAGE_MAP(CIsdnVoicePartyOut, CIsdnVoiceParty);

////////////////////////////////////////////////////////////////////////////
//                        CIsdnVoicePartyOut
////////////////////////////////////////////////////////////////////////////
CIsdnVoicePartyOut::CIsdnVoicePartyOut()
{
	VALIDATEMESSAGEMAP;
}

//--------------------------------------------------------------------------
CIsdnVoicePartyOut::~CIsdnVoicePartyOut()
{
}

//--------------------------------------------------------------------------
void CIsdnVoicePartyOut::Create(CSegment& appParam)
{
	CIsdnVoiceParty::Create(appParam);

#ifdef MESSAGE_QUEUE_BLOCKING_RESERCH
	m_pConfApi = new CConfApi(GetMonitorConfId());
#else
	m_pConfApi = new CConfApi;
#endif
	m_pConfApi->CreateOnlyApi(*m_pCreatorRcvMbx, NULL, NULL, 1);
}

//--------------------------------------------------------------------------
void CIsdnVoicePartyOut::OnConfEstablishCallIdle(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CIsdnVoicePartyOut::OnConfEstablishCallIdle : Name - ", PARTYNAME);

	OnConfEstablishCall(pParam);

	m_numNetChnlCntl = 1;     // connect initial channel

	// Deserialize the CIsdnPartyRsrcDesc
	CIsdnPartyRsrcDesc pPartyRsrcDesc;
	pPartyRsrcDesc.DeSerialize(NATIVE, *pParam);
	pPartyRsrcDesc.DumpToTrace();

	// fill m_pNetSetUp array from the CIsdnPartyRsrcDesc
	SetNetSetup(pPartyRsrcDesc);

	*pParam >> m_RoomId;

	EstablishOutCall();

	m_state = PARTYSETUP;
}

//--------------------------------------------------------------------------
void CIsdnVoicePartyOut::SetNetSetup(CIsdnPartyRsrcDesc& pPartyRsrcDesc)
{
	// fill m_pNetSetUp array from the CIsdnPartyRsrcDesc
	std::vector<CIsdnSpanOrderPerConnection*>* ConnectionVector = pPartyRsrcDesc.GetSpanOrderPerConnectionVector();
	WORD connectionSize   = ConnectionVector ? ConnectionVector->size() : 0;

	CLargeString str;

	if (ConnectionVector)
	{
		WORD i = 0;
		CIsdnSpanOrderPerConnection* connection_i = ConnectionVector->at(i);
		DWORD connection_id = connection_i->GetConnectionId();
		WORD board_id = connection_i->GetBoardId();

		if (connection_id == 0)
			return;

		std::vector<WORD>* spansOrderVector = connection_i->GetSpansListVector();
		WORD spansOrderSize = spansOrderVector ? spansOrderVector->size() : 0;

		/* initialize the current CIsdnNetSetup from the first element */
		m_pNetSetUp[i] = m_pNetSetUp[0];
		/* fill boardID, connectionID and spans order from the CIsdnSpanOrderPerBoard object */
		m_pNetSetUp[i].m_boardId = board_id;
		m_pNetSetUp[i].m_net_connection_id = connection_id;

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

	PTRACE2(eLevelInfoNormal, "CIsdnVoicePartyOut::SetNetSetup:\n", str.GetString());
}

// This action function may be used in Auto Detect only
//--------------------------------------------------------------------------
void CIsdnVoicePartyOut::OnConfEstablishCallSetup(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CIsdnVoicePartyOut::OnConfEstablishCallSetup : Name - ", PARTYNAME);

	OnConfEstablishCallIdle(pParam);
}

//--------------------------------------------------------------------------
void CIsdnVoicePartyOut::OnNetConnectSetUp(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CIsdnVoicePartyOut::OnNetConnectSetUp : Name - ", PARTYNAME);

	WORD seqNum, status;
	BYTE cause = 0;
	*pParam >> seqNum >> status >> cause;

	PASSERT_AND_RETURN(seqNum != 0);  // MAX_CHNLS_IN_voice parties == 1, seqNum MUST be 0

	if (m_isCleanup)
	{
		PTRACE2(eLevelError, "CIsdnVoicePartyOut::OnNetConnectSetUp : net connect while Cleanup", PARTYNAME);
		return; // do not handle net connection while in party cleanup
	}

	if (status)
	{
		m_pConfApi->UpdateDB(this, DISCAUSE, NO_NET_CONNECTION, 1);
		UpdateQ931DisCause(cause);

		m_pConfApi->PartyConnect(GetPartyId(), NO_NET_CONNECTION);
	}
	else     // net connection is o.k.
	{
		UpdateNetChnlStatus(seqNum, TRUE);

		WORD dummy;
		char initial_number[PHONE_NUMBER_DIGITS_LEN+1];

		m_pNetSetUp[seqNum].GetCalledNumber(&dummy, initial_number);

		/* send update to CAddIsdnPartyCntl */
		m_pConfApi->UpdateNetChannel(GetPartyId(), m_pNetSetUp[seqNum].m_boardId,
		                             m_pNetSetUp[seqNum].m_spanId[0],
		                             m_pNetSetUp[seqNum].m_net_connection_id,
		                             seqNum);

		m_pConfApi->PartyConnect(GetPartyId(), statOK);

		PTRACE2(eLevelInfoNormal, "CIsdnVoicePartyOut::OnNetConnectSetUp : \'PSTN PARTY CONNECTED.\' - ", PARTYNAME);
		m_state = PARTYCONNECTED;

		ALLOCBUFFER(tmp, 2*H243_NAME_LEN+100);
		// max size of PARTYNAME is 2*H243_NAME_LEN + 50
		sprintf(tmp, "[%s] - Connection Established.", PARTYNAME);
		PTRACE2(eLevelInfoNormal, " ---> ", tmp);
		DEALLOCBUFFER(tmp);
		StartIvr();
	}
}

//--------------------------------------------------------------------------
void CIsdnVoicePartyOut::DestroyNetConnection(WORD chnl)
{
	PASSERT_AND_RETURN(chnl != 0);  // MAX_CHNLS_IN_voice parties == 1, chnl MUST be 0

	m_pNetChnlCntl[chnl]->Destroy();

	POBJDELETE(m_pNetChnlCntl[chnl]);

	m_retryNetCall[chnl] = 0;
}

//--------------------------------------------------------------------------
void CIsdnVoicePartyOut::EstablishOutCall()
{
	PTRACE2(eLevelInfoNormal, "CIsdnVoicePartyOut::EstablishOutCall : Name - ", PARTYNAME);

	WORD chnl = 0;
	WORD dummy;
	char initial_number[PHONE_NUMBER_DIGITS_LEN+1];

	ALLOCBUFFER(tmp, 2*H243_NAME_LEN+ 150);

	m_pNetSetUp[chnl].GetCalledNumber(&dummy, initial_number);
	sprintf(tmp, "[%s] - Dialing OUT Channel Number %d, Phone Number %s", PARTYNAME, chnl, initial_number);
	PTRACE2(eLevelInfoNormal, " ---> ", tmp);
	DEALLOCBUFFER(tmp)

	m_pNetChnlCntl[chnl] = new CNetChnlCntl;
	m_pNetChnlCntl[chnl]->Create(*m_pNetRsrcParams, m_pNetSetUp[chnl], this, chnl, DIALOUT);
	m_pNetChnlCntl[chnl]->StartNetCntl();
	MonitorPartyPhoneNumbers((BYTE)chnl, &m_pNetSetUp[chnl], (BYTE)1);
}

