#include "IsdnVoicePartyIn.h"
#include "ConfApi.h"
#include "ConfPartyOpcodes.h"
#include "Trace.h"
#include "PartyRsrcDesc.h"

void PartyVoiceInEntryPoint(void* appParam)
{
  CIsdnVoicePartyIn* pPartyTaskApp = new CIsdnVoicePartyIn;
  pPartyTaskApp->Create(*(CSegment*)appParam);
}


PBEGIN_MESSAGE_MAP(CIsdnVoicePartyIn)
  // lobby events
  ONEVENT(LOBBYTRANS,    PARTYSETUP, CIsdnVoicePartyIn::OnLobbyTransferSetup)
  ONEVENT(LOBBYNETIDENT, IDLE,       CIsdnVoicePartyIn::OnLobbyNetIdentIdle)
  // conf events
  ONEVENT(ESTABLISHCALL, PARTYSETUP, CIsdnVoicePartyIn::OnConfEstablishCallSetup)
  ONEVENT(NETCONNECT,    PARTYSETUP, CIsdnVoicePartyIn::OnNetConnectSetUp)
  ONEVENT(NETCONNECT,    ANYCASE,    CIsdnVoicePartyIn::NullActionFunction)

PEND_MESSAGE_MAP(CIsdnVoicePartyIn, CIsdnVoiceParty);

////////////////////////////////////////////////////////////////////////////
//                        CIsdnVoicePartyIn
////////////////////////////////////////////////////////////////////////////
CIsdnVoicePartyIn::CIsdnVoicePartyIn()
{
  m_pLobbyApi = NULL;

  VALIDATEMESSAGEMAP;
}

//--------------------------------------------------------------------------
CIsdnVoicePartyIn::~CIsdnVoicePartyIn()
{
  if (m_pLobbyApi)
    POBJDELETE(m_pLobbyApi);
}

//--------------------------------------------------------------------------
void CIsdnVoicePartyIn::Create(CSegment& appParam)
{
  CParty::Create(appParam);
  m_pLobbyApi = new CLobbyApi;
  m_pLobbyApi->CreateOnlyApi(*m_pCreatorRcvMbx);
}

//--------------------------------------------------------------------------
void CIsdnVoicePartyIn::OnLobbyTransferSetup(CSegment* pParam)
{
  PTRACE(eLevelInfoNormal, "CIsdnVoicePartyIn::OnLobbyTransferSetup ");
  CSegment rspMsg;
  WORD     mode             = 0xFFFF;
  WORD     returnSyncStatus = 0;

  *pParam >> mode;

  m_pConfApi = new CConfApi();
  m_pConfApi->CreateOnlyApi(*m_pConfRcvMbx, NULL, NULL, 1);

  switch (mode)
  {
    case PARTYTRANSFER:
    {
      // sync call to conf
      WORD res = m_pConfApi->AddInParty(m_pNetSetUp, this, *m_pRcvMbx, m_name, PARTY_TRANSFER_TOUT, rspMsg);

      if (res == 0)
      {
        rspMsg >> returnSyncStatus;
      }

      if (res)
      {
        PTRACE2(eLevelError, "CIsdnVoicePartyIn::OnLobbyTransferIdle: Timer expired on transfer party to conf", m_partyConfName);
        PASSERT(1);
        m_pLobbyApi->PartyTransfer(this, statTout);
        m_state = PARTYDISCONNECTING;

        if (m_pConfApi->TaskExists() != NO)
          DBGPASSERT(1);
      }
      else if (returnSyncStatus)
      {
        PTRACE(eLevelError, "CIsdnVoicePartyIn::OnLobbyTransferIdle : \'EXPORT PARTY FAILED (Conf Reject) !!!\'");
        PASSERT(1);
        // Udi - PSTN specified Handling ??? TBD
        m_pLobbyApi->PartyTransfer(this, returnSyncStatus);
        m_state = PARTYDISCONNECTING;
      }
      else
      {
        PTRACE(eLevelInfoNormal, "CIsdnVoicePartyIn::OnLobbyTransferSetup : \'EXPORT PARTY O.K. !!!\'");
        m_pLobbyApi->PartyTransfer(this, statOK);
      }

      break;
    }

    default:
    {
      PASSERT(1);
      break;
    }
  } // switch
}

//--------------------------------------------------------------------------
void CIsdnVoicePartyIn::OnConfEstablishCallSetup(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CIsdnVoicePartyIn::OnConfEstablishCallSetup : Name - ", PARTYNAME);
  OnConfEstablishCall(pParam);

  /* deserialize the CIsdnPartyRsrcDesc */
  CIsdnPartyRsrcDesc pPartyRsrcDesc;
  pPartyRsrcDesc.DeSerialize(NATIVE, *pParam);
  pPartyRsrcDesc.DumpToTrace();

  *pParam >> m_RoomId;

  m_pNetChnlCntl[0]->StartNetCntl(); // OnSetup();
  MonitorPartyPhoneNumbers(0, &m_pNetSetUp[0], 0);
}

//--------------------------------------------------------------------------
void CIsdnVoicePartyIn::OnLobbyNetIdentIdle(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CIsdnVoicePartyIn::OnLobbyNetIdentIdle : Name - ", PARTYNAME);
  WORD chnl = 0;

  m_pNetSetUp->DeSerialize(NATIVE, *pParam);
  m_numNetChnlCntl = 1;                       // connect initial channel
  m_pLobbyApi->PartyIdent(this, 0, statOK);   // allways initial channel
  m_state          = PARTYSETUP;
}

//--------------------------------------------------------------------------
void CIsdnVoicePartyIn::OnConfEstablishCall(CSegment* pParam)
{
  // Read the params
  CIsdnVoiceParty::OnConfEstablishCall(pParam);

  // Create the Cntl
  m_pNetChnlCntl[0] = new CNetChnlCntl;
  m_pNetChnlCntl[0]->Create(*m_pNetRsrcParams, m_pNetSetUp[0], this, 0, DIALIN);
}

//--------------------------------------------------------------------------
void CIsdnVoicePartyIn::OnNetConnectSetUp(CSegment* pParam)
{
  WORD seqNum, status;
  BYTE cause = 0;
  *pParam >> seqNum >> status >> cause;

  PASSERT_AND_RETURN(seqNum != 0);  // MAX_CHNLS_IN_voice parties == 1, seqNum MUST be 0

  if (m_isCleanup)
  {
    PTRACE2(eLevelError, "CIsdnVoicePartyIn::OnNetConnectSetUp : net connect while Cleanup", PARTYNAME);
    return;                         // do not handle net connection while in party cleanup
  }

  if (status)
  {
    CSmallString cstr;
    cstr <<  "CIsdnVoicePartyIn::OnNetConnectSetUp : got NETCONNECT with status: " << status << " Name: " << PARTYNAME;
    PTRACE(eLevelInfoNormal, cstr.GetString());

    m_pConfApi->UpdateDB(this, DISCAUSE, NO_NET_CONNECTION, 1);
    UpdateQ931DisCause(cause);
    m_pConfApi->PartyConnect(GetPartyId(), /**m_pRmtCap,G728,*/ NO_NET_CONNECTION);
  }
  else
  {
    // net connection is o.k.
    m_pConfApi->PartyConnect(GetPartyId(), /*m_pCurrentComMode->GetAudMode(),*/ statOK);

    PTRACE2(eLevelInfoNormal, "CIsdnVoicePartyIn::OnNetConnectSetUp : \'PSTN PARTY CONNECTED.\' - ", PARTYNAME);
    m_state = PARTYCONNECTED;

    ALLOCBUFFER(tmp, 2*H243_NAME_LEN+100);
    // max size of PARTYNAME is 2*H243_NAME_LEN + 50
    sprintf(tmp, "[%s] - Connection Established.", PARTYNAME);
    PTRACE2(eLevelInfoNormal, " ---> ", tmp);
    DEALLOCBUFFER(tmp);
    StartIvr();
  }
}
