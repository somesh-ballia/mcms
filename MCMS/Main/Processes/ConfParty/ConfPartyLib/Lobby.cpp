#include "Lobby.h"
#include "IpCsOpcodes.h"
#include "Segment.h"
#include "RsrcParams.h"
#include "ConfDef.h"
#include "CommConfDB.h"
#include "SipUtils.h"
#include "ReceptionSip.h"
#include "CommResDB.h"
#include "GKManagerOpcodes.h"
#include "ConfPartyManagerLocalApi.h"
#include "SipCaps.h"
#include "SystemFunctions.h"
#include "MplMcmsProtocol.h"
#include "SipScm.h"
#include "ConfPartyMplMcmsProtocolTracer.h"
#include "ConfPartyDefines.h"
#include "OpcodesMcmsCommon.h"
#include "HostCommonDefinitions.h"
#include "IpServiceListManager.h"
#include "CommResShort.h"
#include "TraceStream.h"
#include "IsdnNetSetup.h"
#include "ReceptionVoice.h"
#include "ReceptionIsdn.h"
#include "OpcodesMcmsNetQ931.h"
#include "DwordBitMask.h"
#include "ConfApi.h"
#include "ConfPartyProcess.h"
#include "IpCommon.h"
#include "IpServiceListManager.h"
#include "IpCommon.h"
#include "IpServiceListManager.h"
#include "GkTaskApi.h"
#include "PrecedenceSettings.h"

extern CCommResDB*            GetpMeetingRoomDB();
extern CIpServiceListManager* GetIpServiceListMngr();
extern CPrecedenceSettings*   GetpPrecedenceSettingsDB();

const WORD                    TEST_TIMER_2 = 102;

//#define DUMPSTR(str) (((str != 0) && (str[0] != '\0')) ? str : "NA")

PBEGIN_MESSAGE_MAP(CLobby)
  ONEVENT(RELEASE_UNRESERVED_PARTY  ,CONNECT ,CLobby::OnConfReleaseParty)
  ONEVENT(CONF_ON_AIR               ,CONNECT ,CLobby::OnConfConfOnAir)
  ONEVENT(REJECT_UNRESERVED_PARTY   ,CONNECT ,CLobby::OnRejectParty)      // From conferance and conf too
  ONEVENT(REJECT_START_MEETING_ROOM ,CONNECT ,CLobby::OnConfMngrRejectParty)
  ONEVENT(SUSPEND_IVR_PARTY         ,CONNECT ,CLobby::OnConfMngrSuspendIVRParty)
  ONEVENT(GK_MANAGER_SEARCH_CALL    ,CONNECT ,CLobby::OnGkManagerSearchForCall)
PEND_MESSAGE_MAP(CLobby, CStateMachine);

//--------------------------------------------------------------------------
void LobbyEntryPoint(void* appParam)
{
  CLobby* pLobby = new CLobby;
  pLobby->Create(*(CSegment*)appParam);
}

////////////////////////////////////////////////////////////////////////////
//                        CLobby
////////////////////////////////////////////////////////////////////////////
CLobby::CLobby()
{
  m_pTransferList = NULL;
  m_pLobbyTrans   = NULL;
  m_pParyList_Id  = (CTaskApp*)0x0001; // zero is not valid as id
  m_state         = CONNECT;
  m_chnlCnt       = 0;
  gw_index        = 0;
  m_pDelayedCalls = new VEC_DELAYED_CALLS;

  memset(&m_dummyMediaIp, 0, sizeof(m_dummyMediaIp));

}

//--------------------------------------------------------------------------
CLobby::~CLobby()
{
  PDELETE(m_pDelayedCalls);
}

//--------------------------------------------------------------------------
void CLobby::Destroy()
{
  if (m_pTransferList)
    m_pTransferList->ClearAndDestroy();

  POBJDELETE(m_pTransferList);
  POBJDELETE(m_pLobbyTrans);
}

//--------------------------------------------------------------------------
void CLobby::SelfKill()
{
  Destroy();
  CTaskApp::SelfKill();
}

//--------------------------------------------------------------------------
void CLobby::Create(CSegment& appParam)
{
  AllocTransferList();
  m_pLobbyTrans = new CReception(this);
  CTaskApp::Create(appParam); // Create is running the task
}

//--------------------------------------------------------------------------
void CLobby::AllocTransferList()
{
  if (m_pTransferList)
  {
    m_pTransferList->ClearAndDestroy();
    POBJDELETE(m_pTransferList);
  }
  m_pTransferList = new CReceptionList;
}

//--------------------------------------------------------------------------
BOOL CLobby::TaskHandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode)
{
  switch (opCode)
  {
    case H323_CS_SIG_CALL_OFFERING_IND:
    case SIP_CS_SIG_INVITE_IND:
    case SIP_CS_SIG_OPTIONS_IND:
    case NET_SETUP_IND:
    case NET_CLEAR_IND:
    case NET_DISCONNECT_IND:
    case NET_DISCONNECT_ACK_IND:
    case SIP_CS_CCCP_SIG_INVITE_IND:
    {
      HandleCentralSignalingEvent(pMsg, opCode);
      return TRUE;
    }

    case  DESTROY:
    {
      TRACEINTO << "CLobby::TaskHandleEvent - Opcode:DESTROY";
      return FALSE;                     // new carmel CTaskApp::HandleEvent will take care for DESTROY EVENT
    }

    case  RELEASE_UNRESERVED_PARTY:     // suspended call messages
    case  REJECT_UNRESERVED_PARTY:
    case  REJECT_START_MEETING_ROOM:
    case  CONF_ON_AIR:
    case  GK_MANAGER_SEARCH_CALL:
    case  SUSPEND_IVR_PARTY:
    {
      DispatchEvent(opCode, pMsg);
      return TRUE;
    }

    case  SIP_CS_SIG_TRACE_INFO_IND:
    {
      return TRUE;
    }

    default: // messages to objects in transfer list from party
    {
      CTaskApp* pParty = NULL;
      *pMsg >> (void*&)pParty;
      m_pLobbyTrans->SetParty(pParty);
      CStateMachine* pStateMachine = m_pTransferList->Find(m_pLobbyTrans);
      PASSERTSTREAM_AND_RETURN_VALUE(!pStateMachine, "CLobby::TaskHandleEvent - Failed, Invalid opcode:" << opCode, TRUE);

      pStateMachine->HandleEvent(pMsg, msgLen, opCode);
      return TRUE;
    }
  } // switch
}

//--------------------------------------------------------------------------
void CLobby::HandleCentralSignalingEvent(CSegment* pMsg, OPCODE opcode)
{
  switch (opcode)
  {
    case H323_CS_SIG_CALL_OFFERING_IND:
    {
      OnH323CallIn(pMsg);
      break;
    }


    case SIP_CS_SIG_INVITE_IND:
    {
      OnSipCallIn(pMsg);
      break;
    }

    case SIP_CS_SIG_OPTIONS_IND:
    {
      OnSipOptions(pMsg);
      break;
    }

    case NET_SETUP_IND:
    {
      OnNetCallIn(pMsg);
      break;
    }

    case NET_DISCONNECT_IND:
    case NET_CLEAR_IND:
    case NET_DISCONNECT_ACK_IND:
    {
      TRACEINTO << "CLobby::HandleCentralSignalingEvent - Failed, the event should be handled at network layer, Opcode:" << opcode;
      break;
    }

    case SIP_CS_CCCP_SIG_INVITE_IND:
    {
      OnSipMsConfInvite(pMsg);
      break;
    }

    default:
    {
      TRACEINTO << "CLobby::HandleCentralSignalingEvent - Failed, invalid opcode, Opcode:" << opcode;
      break;
    }
  }
}

//--------------------------------------------------------------------------
void CLobby::OnH323CallIn(CSegment* pCentralSigParam) //shiraITP - 40
{
  CSegment* pCopyOfParams = new CSegment(*pCentralSigParam);

  APIU32 csServiceId    = 0;
  APIU32 callIndex      = 0;
  APIU32 channelIndex   = 0;
  APIU32 mcChannelIndex = 0;
  APIU32 stat1          = 0;
  APIS32 status         = 0;
  APIU16 srcUnitId      = 0;

  *pCentralSigParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId >> csServiceId;
  status = (APIS32)stat1;

  DWORD nMsgLen = pCentralSigParam->GetWrtOffset()-pCentralSigParam->GetRdOffset();
  if (!nMsgLen)
  {
    PDELETE(pCopyOfParams);
    return;
  }

  BYTE* pMessage = new BYTE[nMsgLen];
  pCentralSigParam->Get(pMessage, nMsgLen);
  pCentralSigParam->DecRead(nMsgLen);

  mcIndCallOffering* pCallOfferingInd = (mcIndCallOffering*)new BYTE[sizeof(mcIndCallOffering)];
  memcpy(pCallOfferingInd, pMessage, min(sizeof(mcIndCallOffering), nMsgLen));
  PDELETEA(pMessage);

  if (pCallOfferingInd->encryTokens.numberOfTokens > 0)
  {
    int lenWithEncry = sizeof(mcIndCallOffering) + pCallOfferingInd->encryTokens.dynamicTokensLen - (sizeof(encTokensHeaderStruct) - sizeof(encTokensHeaderBasicStruct));
    PDELETEA(pCallOfferingInd);
    pCallOfferingInd = (mcIndCallOffering*)new BYTE[lenWithEncry];

    BYTE* pMessageEncry = new BYTE[lenWithEncry];
    pCentralSigParam->Get(pMessageEncry, lenWithEncry);
    memcpy(pCallOfferingInd, pMessageEncry, lenWithEncry);
    PDELETEA(pMessageEncry);
  }

  std::auto_ptr<CH323NetSetup> pNetSetup(new CH323NetSetup);

  pNetSetup->SetVariabels(*pCallOfferingInd);
  pNetSetup->SetRemoteSetupRate(pCallOfferingInd->rate);
  pNetSetup->SetCallIndex(callIndex);
  pNetSetup->SetConnectionId(LOBBY_CONNECTION_ID);
  pNetSetup->SetSrcUnitId(srcUnitId);
  pNetSetup->SetCsServiceid(csServiceId);

  // For HotBackup validity
  CProcessBase* pConfPartyProcess = CProcessBase::GetProcess();
  if (TRUE == pConfPartyProcess->GetIsFailoverSlaveMode())
  {
    TRACEINTO << "CLobby::OnH323CallIn - RMX is in Slave MODE (call rejected)";
    RejectH323Call(pNetSetup.get(), int(cmReasonTypeNoPermision));
    PDELETE(pCallOfferingInd);
    POBJDELETE(pCopyOfParams);
    return;
  }

  if (status == -2)
  {
    TRACEINTO << "CLobby::OnH323CallIn - Lack of control ports (call rejected)";
    RejectH323Call(pNetSetup.get(), int(cmReasonTypeDestinationRejection));
    PDELETE(pCallOfferingInd);
    POBJDELETE(pCopyOfParams);
    return;
  }

  PDELETEA(pCallOfferingInd);

  // IpV6 - Add correct print
  char strSrcPartyAddress[MaxAddressListSize] = { 0 };
  char strDstPartyAddress[MaxAddressListSize] = { 0 };
  ::ipToString(*(pNetSetup->GetTaDestPartyAddr()), strDstPartyAddress, 1);
  ::ipToString(*(pNetSetup->GetTaSrcPartyAddr()), strSrcPartyAddress, 1);

  TRACEINTO << "CLobby::OnH323CallIn "
            << "- CalleeIP:" << DUMPSTR(strDstPartyAddress)
            << ", CallerIP:" << DUMPSTR(strSrcPartyAddress);

  // Find party in Database
  CCommConf*                pComConf           = NULL;
  CConfParty*               pConfParty         = NULL;
  int                       numOfSrcAlias      = 0;
  int                       numOfDestAlias     = 0;
  const mcTransportAddress* pPartyIp           = pNetSetup->GetTaSrcPartyAddr();
  CH323Alias*               PartySrcAliasList  = pNetSetup->GetSrcPartyAliasList(&numOfSrcAlias);
  CH323Alias*               PartyDestAliasList = pNetSetup->GetDestPartyAliasList(&numOfDestAlias);

  // set best best party party alias.
  // will be used later for SiteName/Visual Name
  if (PartySrcAliasList)
    pNetSetup->SetH323HighestPriorityPartyAlias();

  if (strcmp(pNetSetup->NameOf(), "CH323NetSetup") == 0)
  {
    // clear the alias fields
    pNetSetup->SetH323PartyAlias("");
    pNetSetup->SetH323PartyAliasType(PARTY_H323_ALIAS_H323_ID_TYPE);
  }

  DWORD meetingRoomId = 0xFFFFFFFF;
  int i = GetIpConfPartyType(pPartyIp, PartySrcAliasList, numOfSrcAlias, (CCommConf**)&pComConf, &pConfParty, PartyDestAliasList, numOfDestAlias, meetingRoomId);
  if (pConfParty && pConfParty->GetNetInterfaceType() != H323_INTERFACE_TYPE)
  {
    i = -6;
    pConfParty->SetDefinedPartyAssigned(FALSE); // H.323 call into defined sip party
  }

  if (i == -1 && pConfParty && pConfParty->GetPartyState() == PARTY_DISCONNECTING)
  {
    TRACEINTO << "CLobby::OnH323CallIn - H323 call delayed, CallIndex:" << callIndex;
    CDelayedH323Call* pDelayedCall = new CDelayedH323Call(this);
    pDelayedCall->Create(this, callIndex, pCopyOfParams);
    m_pDelayedCalls->push_back(pDelayedCall);
    return;
  }
  else
  {
    POBJDELETE(pCopyOfParams);
  }

  // Transit EQ - In case a destination conference wasn't found we will try to connect to Transit EQ
  WORD useTransitEQ = FALSE;
  char transitEQName[H243_NAME_LEN];
  SAFE_COPY(transitEQName, ::GetpMeetingRoomDB()->GetTransitEQName());

  if (i == -1 && strcmp(transitEQName, ""))
  {
    char AliasName[ALIAS_NAME_LEN] = { 0 };
    if (PartyDestAliasList && numOfDestAlias)
      SAFE_COPY(AliasName, PartyDestAliasList->GetAliasName());

    TRACEINTO << "CLobby::OnH323CallIn - Destination conference wasn't found"
              << ", CalleeIP:" << DUMPSTR(strDstPartyAddress)
              << ", CallerIP:" << DUMPSTR(strSrcPartyAddress)
              << ", We will try to connect to default EQ name:" << transitEQName;

    useTransitEQ = SetNewDestinationConfName(pNetSetup.get(), transitEQName);
    if (useTransitEQ == TRUE)
    {
      PDELETEA(PartyDestAliasList);
      PartyDestAliasList = pNetSetup->GetDestPartyAliasList(&numOfDestAlias);
      i = GetIpConfPartyType(pPartyIp, PartySrcAliasList, numOfSrcAlias, (CCommConf**)&pComConf, &pConfParty, PartyDestAliasList, numOfDestAlias, meetingRoomId, TRUE, NULL, TRUE);
      SetPreDefinedIvrForTransitAdhocEqIfNeeded(transitEQName, pNetSetup.get(), AliasName);
    }
    else
    {
      TRACEINTO << "CLobby::OnH323CallIn - Failed to change the destination address to transit EQ";
    }
  }

  CConfIpParameters* pService = ::GetIpServiceListMngr()->FindIpService(csServiceId);
  if (!pService)
  {
    PDELETEA(PartySrcAliasList);
    PDELETEA(PartyDestAliasList);
    PASSERT_AND_RETURN(1);
  }
  char* serviceName = (char*)pService->GetServiceName();
  int idx = 0;

  if (pComConf)
  {
    for (; idx < NUM_OF_IP_SERVICES; idx++)
    {
      if (!strcmp(serviceName, pComConf->GetServiceRegistrationContentServiceName(idx)))
        break;
    }

    if (idx == NUM_OF_IP_SERVICES)
    {
      TRACEINTO << "CLobby::OnH323CallIn - Failed to find appropriate service, ServiceName:" << serviceName;
    }
    else
    {
      if (pComConf->GetServiceRegistrationContentAcceptCall(idx) == FALSE)
      {
        TRACEINTO << "CLobby::OnH323CallIn - Failed, Dial-In from this service is not allowed (call rejected), ServiceName:" << serviceName;
        RejectH323Call(pNetSetup.get(), int(cmReasonTypeNoPermision));
        PDELETEA(PartySrcAliasList);
        PDELETEA(PartyDestAliasList);
        return;
      }
    }
  }
  else
  {
    CCommRes* pConfRes = ::GetpMeetingRoomDB()->GetCurrentRsrv(meetingRoomId);
    //PASSERT(!pConfRes);

    if (pConfRes)
    {
      for (; idx < NUM_OF_IP_SERVICES; idx++)
      {
        if (!strcmp(serviceName, pConfRes->GetServiceRegistrationContentServiceName(idx)))
          break;
      }

      if (idx == NUM_OF_IP_SERVICES)
      {
        TRACEINTO << "CLobby::OnH323CallIn - Failed to find appropriate service, ServiceName:" << serviceName;
      }
      else
      {
        if (pConfRes->GetServiceRegistrationContentAcceptCall(idx) == FALSE)
        {
          TRACEINTO << "CLobby::OnH323CallIn - Failed, Dial-In from this service is not allowed (call rejected), ServiceName:" << serviceName;
          RejectH323Call(pNetSetup.get(), int(cmReasonTypeNoPermision));
          PDELETEA(PartySrcAliasList);
          PDELETEA(PartyDestAliasList);
		  POBJDELETE(pConfRes);
          return;
        }
      }
      POBJDELETE(pConfRes);
    }
  }

  //Multiple links for ITP in cascaded conference feature: CLobby::OnH323CallIn
  eTypeOfLinkParty linkType                   = eRegularParty;
  BYTE             cascadedLinksNumber        = 0;
  BYTE             index                      = 0;
  DWORD            mainLinkDialInNumber       = 0;
  BOOL             bIsITPcall                 = FALSE;

  bIsITPcall = ::GetITPparams(pNetSetup->GetH323userUser(),cascadedLinksNumber,index,linkType,mainLinkDialInNumber,H323_INTERFACE_TYPE);

  //for dial in - defined and undefined! - disconnect the second conf that was dial in with different cascadedLinksNumber:
  if (bIsITPcall && linkType == eMainLinkParty)
  {
      PTRACE2INT(eLevelError,"ITP_CASCADE: CLobby::OnH323CallIn cascadedLinksNumber:",cascadedLinksNumber);
      if ( pComConf && (cascadedLinksNumber !=  pComConf->GetCascadedLinksNumber()) && (pComConf->GetCascadedLinksNumber() > 0) )
      {
          //disconnect mainLink:
          PTRACE2INT(eLevelError,"ITP_CASCADE: CLobby::OnH323CallIn pComConf->GetCascadedLinksNumber:",pComConf->GetCascadedLinksNumber());
          //PASSERTMSG(1,"ITP_CASCADE: CLobby::OnH323CallIn - NOT BUG - JUST FOR NOTICE! disconnect the second conf that was dial in with different cascadedLinksNumber!");
          PTRACE(eLevelError,"ITP_CASCADE: CLobby::OnH323CallIn - NOT BUG - JUST FOR NOTICE! disconnect the second conf that was dial in with different cascadedLinksNumber!");
          RejectH323Call(pNetSetup.get(), int(cmReasonTypeNoPermision));
          PDELETEA(PartySrcAliasList);
          PDELETEA(PartyDestAliasList);
          return;
      }
  }
  else
      PTRACE(eLevelInfoNormal, "ITP_CASCADE: CLobby::OnH323CallIn DialIn GetITPparams == FALSE");


  switch (i)
  {
    case 0:
    {
      // Conf And Party were found !!!
      // if the parameters are configured at the party we reset the netsetup
      // parameters at the AcceptH323CallIn
      if (PartySrcAliasList)
      {
        pNetSetup->SetH323PartyAliasType(PartySrcAliasList->GetAliasType());
        pNetSetup->SetH323PartyAlias(PartySrcAliasList->GetAliasName());
      }

      if (pComConf && ( (pComConf->isIvrProviderEQ()) || (pComConf->isExternalIvrControl()) ) )  //AT&T
      {
        TRACEINTO << "CLobby::OnH323CallIn - Failed, IvrProviderEQ/ExternalIvrControl is not allowed in H323 calls";
        RejectH323Call(pNetSetup.get(), int(cmReasonTypeNoPermision));
        break;
      }
      //Multiple links for ITP in cascaded conference feature: CLobby::OnH323CallIn definedParty dialIn- rename the party name:
      if (pConfParty && (TRUE == bIsITPcall) )
      {
          char mainPartyName[H243_NAME_LEN];
          ::CreateMainLinkName(pConfParty->GetName(),(char *)mainPartyName);

          if (linkType == eMainLinkParty)
          {
              PTRACE2(eLevelError, "ITP_CASCADE: CLobby::OnH323CallIn DialIn partyName:",pConfParty->GetName());
              PTRACE2INT(eLevelError, "ITP_CASCADE: CLobby::OnH323CallIn DialIn index:",index);

              pConfParty->SetName(mainPartyName);
              pConfParty->SetPartyType(linkType);
              ((CRsrvParty*)pConfParty)->SetCascadedLinksNumber(cascadedLinksNumber);
          }

          if (linkType == eSubLinkParty)
          {
              PTRACE2INT(eLevelError, "ITP_CASCADE: CLobby::OnH323CallIn DialIn eSubLinkParty mainLinkDialInNumber:",mainLinkDialInNumber);
              pConfParty->SetPartyType(linkType);
              ((CRsrvParty*)pConfParty)->SetMainPartyNumber(mainLinkDialInNumber);

              //check if there is main link for this sub..
              if (pComConf)
              {
                  CConfParty* pConfPartyOfMain = pComConf->GetCurrentParty(mainPartyName);
                  if (pConfPartyOfMain == NULL )
                  {
                      PTRACE(eLevelError,"ITP_CASCADE: CLobby::OnH323CallIn -ERROR! there is no main link for this sub link - reject the call!");
                      RejectH323Call(pNetSetup.get(), int(cmReasonTypeNoPermision));
                      PDELETEA(PartySrcAliasList);
                      PDELETEA(PartyDestAliasList);
                      return;
                  }
              }
          }
      }
      else
          PTRACE(eLevelInfoNormal, "ITP_CASCADE: CLobby::OnH323CallIn DialIn defined GetITPparams == FALSE");

      AcceptH323CallIn(pNetSetup.get(), pComConf, pConfParty); //defined dialin..

      break;
    }

    case -1:
    {
      // REJECT THE CALL UNABLE TO FIND THE CONF AND/OR THE PARTY
      TRACEINTO << "CLobby::OnH323CallIn - Incoming call rejected"
                << ", CalleeIP:" << DUMPSTR(strDstPartyAddress)
                << ", CallerIP:" << DUMPSTR(strSrcPartyAddress);

      if (useTransitEQ == TRUE)
        TRACEINTO << "CLobby::OnH323CallIn - Incoming call rejected (failed to connect to Transit EQ)";

      // unattended call handle ...
      RejectH323Call(pNetSetup.get(), int(cmReasonTypeNoPermision));
      break;
    }

    case -2:
    // no break!
    case -7:
	{

        //PTRACE(eLevelInfoNormal,"CLobby::OnH323CallIn -DialIn -7");

        //case -7: Found the conf and party, but the party is in connected status
		if( (-7 == i)&&
			(cmEndpointTypeMCU== pNetSetup->GetEndpointType()) &&
			(CASCADE_MODE_SLAVE==pConfParty->GetCascadeMode()) &&
			(linkType == eSubLinkParty) )                          //Multiple links for ITP in cascaded conference feature
			//(pComConf->GetMainLinkDefinedOriginName() == NULL) )
		{
		    PTRACE(eLevelInfoNormal,"CLobby::OnH323CallIn - DialIn i = -7 &&");
		    RejectH323Call(pNetSetup.get(), int(cmReasonTypeInConf));
			break;
		}
      // unreserved call (returned value at Undefined and Advance Dial In)
      // CONFERENCE FOUND BUT PARTY NOT FOUND (UNRESERVED PARTY)
      TRACEINTO << "CLobby::OnH323CallIn - Incoming UnReserved Call"  //dial in - undefined
                << ", CalleeIP:" << DUMPSTR(strDstPartyAddress)
                << ", CallerIP:" << DUMPSTR(strSrcPartyAddress);

      // unattended call handle ...
      PASSERT_AND_RETURN(!pComConf);

      if (PartySrcAliasList)
      {
        pNetSetup->SetH323PartyAliasType(PartySrcAliasList->GetAliasType());
        pNetSetup->SetH323PartyAlias(PartySrcAliasList->GetAliasName());
      }

      if (pComConf->isIvrProviderEQ() || pComConf->isExternalIvrControl()) //AT&T
      {
        TRACEINTO << "CLobby::OnH323CallIn - Incoming call rejected (IvrProviderEQ/ExternalIvrControl is not allowed in H323 calls)";
        RejectH323Call(pNetSetup.get(), int(cmReasonTypeNoPermision));
        break;
      }

      SuspendH323CallConfExist(pNetSetup.get(), pComConf->GetMonitorConfId(), strSrcPartyAddress, strDstPartyAddress, (char*)pComConf->GetName()); //shiraITP - 41
      break;
    }

    case -3:  // conference is locked
    {
      TRACEINTO << "CLobby::OnH323CallIn - Incoming call rejected (conference is locked at Dial-In)";
      RejectH323Call(pNetSetup.get(), int(cmReasonTypeNoPermision));
      break;
    }

    case -4:  // max parties number in conference has been reached
    {
      TRACEINTO << "CLobby::OnH323CallIn - Incoming call rejected (max parties number in conference has been reached at Dial-In)";
      RejectH323Call(pNetSetup.get(), int(cmReasonTypeNoPermision));
      break;
    }

    case -5:  // conference is secured
    {
      TRACEINTO << "CLobby::OnH323CallIn - Incoming call rejected (conference is secured at Dial-In)";
      RejectH323Call(pNetSetup.get(), int(cmReasonTypeNoPermision));
      break;
    }

    case -6:  // This is not a 323 call
    {
      TRACEINTO << "CLobby::OnH323CallIn - Incoming call rejected (this is not a 323 call, check participant interface type in the properties)";
      RejectH323Call(pNetSetup.get(), int(cmReasonTypeNoPermision));
      break;
    }

    // In this case there are 2 options:
    // 1. This is the first party to awake the MR
    // 2. This is the 2nd 3rd ...party to the MR but the MR is still NOT up
    case 1:
    {
      // meeting room or GW
      CCommRes* pMrRes = ::GetpMeetingRoomDB()->GetCurrentRsrv(meetingRoomId);
      PASSERT(!pMrRes);

      if (pMrRes)
      {
        if (pMrRes->isIvrProviderEQ() || pMrRes->isExternalIvrControl()) //AT&T
        {
          TRACEINTO << "CLobby::OnH323CallIn - Incoming call rejected (IvrProviderEQ/ExternalIvrControl is not allowed in H323 calls)";
          RejectH323Call(pNetSetup.get(), int(cmReasonTypeNoPermision));
          POBJDELETE(pMrRes);
          break;
        }

        char* mrName = (char*)pMrRes->GetName();

        if (PartySrcAliasList)
        {
          pNetSetup->SetH323PartyAliasType(PartySrcAliasList->GetAliasType());
          pNetSetup->SetH323PartyAlias(PartySrcAliasList->GetAliasName());
        }

        if (YES == TargetReceptionAlreadyExist(mrName))
          SuspendH323Call(pNetSetup.get(), meetingRoomId, strSrcPartyAddress, strDstPartyAddress, mrName);
        // When conf is UP those Receptions will removed from list
        else   // This is the party to awake the MR
        {
          if (pMrRes->GetIsGateway() || strcmp(mrName, "GateWayTest") == 0)
          {
            char dialString[MaxAddressListSize] = { 0 };
            for (int k = 0; k < numOfDestAlias; k++)
            {
              const char* strDest = PartyDestAliasList[k].GetAliasName();
              char* pReadPtr1 = (char*)strchr(strDest, '*');
              if (pReadPtr1)
              {
                char* pReadPtr2 = strchr(pReadPtr1, ',');
                int   len = pReadPtr2 ? pReadPtr2-pReadPtr1-1 : strlen(pReadPtr1);
                len = min(len, MaxAddressListSize-1);
                strncpy(dialString, pReadPtr1+1, len);
                dialString[len] = '\0';
                break;
              }
            }
            SuspendH323CallAndCreateGateWayConf(pNetSetup.get(), meetingRoomId, strSrcPartyAddress, strDstPartyAddress, mrName, dialString);
          }
          else
          {
            SuspendH323CallAndCreateMeetingRoom(pNetSetup.get(), meetingRoomId, strSrcPartyAddress, strDstPartyAddress, mrName);
          }
        }
        POBJDELETE(pMrRes);
      }
      break;
    }

    default:
    {
      // REJECT THE CALL UNABLE TO FIND THE CONF AND/OR THE PARTY
      TRACEINTO << "CLobby::OnH323CallIn - Incoming call rejected"
                << ", CalleeIP:" << DUMPSTR(strDstPartyAddress)
                << ", CallerIP:" << DUMPSTR(strSrcPartyAddress);
      // unattended call handle ...
      RejectH323Call(pNetSetup.get(), int(cmReasonTypeNoPermision));
      break;
    }
  } // switch

  PDELETEA(PartySrcAliasList);
  PDELETEA(PartyDestAliasList);
}

//--------------------------------------------------------------------------
void CLobby::AcceptH323CallIn(CH323NetSetup* pNetSetup, CCommConf* pComConf, CConfParty* pConfParty) //shiraITP - 50
{
  PASSERT_AND_RETURN(!pNetSetup);
  PASSERT_AND_RETURN(!pComConf);
  PASSERT_AND_RETURN(!pConfParty);

  COsQueue*   pConfRcvMbx   = pComConf->GetRcvMbx();
  const char* confName      = pComConf->GetName();
  DWORD       monitorConfId = pComConf->GetMonitorConfId();

  TRACEINTO << "CLobby::AcceptH323CallIn "
            << "- PartyId:"   << pConfParty->GetPartyId()
            << ", PartyName:" << pConfParty->GetName()
            << ", EnableICE:" << (int)pConfParty->GetEnableICE();

  // set netsetup parameters
  pNetSetup->SetConfId(monitorConfId);

  // reset the parameters if they exist at the party (operator)
  if (strncmp(pConfParty->GetH323PartyAlias(), "", IP_STRING_LEN) != 0)
  {
    pNetSetup->SetH323PartyAliasType(pConfParty->GetH323PartyAliasType());
    pNetSetup->SetH323PartyAlias(pConfParty->GetH323PartyAlias());
  }

  // To open IVR
  int         numOfDestAlias;
  CH323Alias* PartyDestAliasList     = pNetSetup->GetDestPartyAliasList(&numOfDestAlias);
  BYTE        bPreDefinedIvrStrExist = FALSE;
  if (PartyDestAliasList)
  {
    const char* AliasName = PartyDestAliasList->GetAliasName();
    if (AliasName)
    {
      TRACEINTO << "CLobby::AcceptH323CallIn - AliasName:" << AliasName;
      const char* PreDefinedString = strstr(AliasName, "#");
      if (PreDefinedString)
      {
        pConfParty->SetPreDefinedIvrString(PreDefinedString);
        bPreDefinedIvrStrExist = TRUE;
      }
    }
  }

  if (!bPreDefinedIvrStrExist)
  {
    const char* netSetupPredefinedIvr = pNetSetup->GetPredefiendIvrStr();   // for the ENABLE_DEFAULT_ADHOC_CALLS
    if (strlen(netSetupPredefinedIvr))
      pConfParty->SetPreDefinedIvrString(netSetupPredefinedIvr);
  }

  PDELETEA(PartyDestAliasList);

  WORD isChairEnabled = 1;
  BYTE Meet_me_method = MEET_ME_PER_USER;

  switch (Meet_me_method)
  {
    case MEET_ME_PER_MCU:
      PASSERTMSG(1, "CLobby::AcceptH323CallIn - MEET_ME_PER_MCU not supported yet");
      break;

    case MEET_ME_PER_CONFERENCE:
      PASSERTMSG(1, "CLobby::AcceptH323CallIn - MEET_ME_PER_CONFERENCE not supported yet");
      break;

    case MEET_ME_PER_USER:
      IdentifyH323Party(pNetSetup, pComConf, pConfParty, pConfRcvMbx, m_pRcvMbx, confName, monitorConfId, isChairEnabled); //shiraITP - 51
      break;

    default:    // REJECT THE CALL UNKOWN MEET ME METHOD
      // unattented call handle ...
      RejectH323Call(pNetSetup, int(cmReasonTypeNoPermision));
      break;
  }
}

//--------------------------------------------------------------------------
void CLobby::SuspendH323Call(CH323NetSetup* pNetSetup, DWORD ConfId, char* strSrcPartyAddress, char* strDstPartyAddress, char* confName)
{
  PASSERT_AND_RETURN(!pNetSetup);

  TRACEINTO << "CLobby::SuspendH323Call "
            << "- ConfId:"   << ConfId
            << ", CalleeIP:" << DUMPSTR(strDstPartyAddress)
            << ", CallerIP:" << DUMPSTR(strSrcPartyAddress);

  SetPartyListIDForSuspendCall(strSrcPartyAddress, strDstPartyAddress);

  // allocate a Suspended Reception
  CReceptionH323* pLobbyTrans = new CReceptionH323(this);
  pLobbyTrans->CreateSuspend(pNetSetup, m_pRcvMbx, this, m_pParyList_Id, ConfId, confName);// suspend H323 call
  m_pTransferList->Insert(pLobbyTrans);
}

//--------------------------------------------------------------------------
void CLobby::SuspendH323CallConfExist(CH323NetSetup* pNetSetup, DWORD ConfId, char* strSrcPartyAddress, char* strDstPartyAddress, char* confName) //shiraITP - 41
{
  PASSERT_AND_RETURN(!pNetSetup);

  TRACEINTO << "CLobby::SuspendH323CallConfExist "
            << "- ConfId:"   << ConfId
            << ", CalleeIP:" << DUMPSTR(strDstPartyAddress)
            << ", CallerIP:" << DUMPSTR(strSrcPartyAddress);

  SetPartyListIDForSuspendCall(strSrcPartyAddress, strDstPartyAddress);

  // allocate a Suspended Reception
  CReceptionH323* pLobbyTrans =  new CReceptionH323(this);
  pLobbyTrans->CreateSuspendConfExist(pNetSetup, m_pRcvMbx, this, m_pParyList_Id, ConfId, confName);  // suspend H323 call  //shiraITP - 42
  m_pTransferList->Insert(pLobbyTrans);
}

//--------------------------------------------------------------------------
void CLobby::SuspendH323CallAndCreateMeetingRoom(CH323NetSetup* pNetSetup, DWORD ConfId, char* strSrcPartyAddress, char* strDstPartyAddress, char* mrName)
{
  // Save incomming call information in a holding queue Reception List
  // the call will be susupened until a release message is recived
  // from an internal lobby timer or an external message
  PASSERT_AND_RETURN(!pNetSetup);

  TRACEINTO << "CLobby::SuspendH323CallAndCreateMeetingRoom "
            << "- ConfId:"   << ConfId
            << ", CalleeIP:" << DUMPSTR(strDstPartyAddress)
            << ", CallerIP:" << DUMPSTR(strSrcPartyAddress);

  SetPartyListIDForSuspendCall(strSrcPartyAddress, strDstPartyAddress);

  // allocate a Suspended Reception
  CReceptionH323* pLobbyTrans = new CReceptionH323(this);
  pLobbyTrans->CreateMeetingRoom(pNetSetup, m_pRcvMbx, this, m_pParyList_Id, ConfId, mrName);
  m_pTransferList->Insert(pLobbyTrans);
}

//--------------------------------------------------------------------------
void CLobby::OnConfConfOnAir(CSegment* pParam)
{
  DWORD confID = 0xFFFFFFFF;
  char  targeConfName[H243_NAME_LEN] = { 0 };

  *pParam  >> confID  >> targeConfName;

  SetMsConversationIdInConfDB(confID, targeConfName);
  ReleaseAllConfOnAirSuspendParties(confID, targeConfName);
}

//--------------------------------------------------------------------------
void CLobby::ReleaseAllConfOnAirSuspendParties(DWORD confMonitorId, char* targetConfName)
{
  // To change this function to support all receptions type
  TRACEINTO << "CLobby::ReleaseAllConfOnAirSuspendParties - confMonitorId:" << confMonitorId << ", targetConfName:" << targetConfName;

  eSipFactoryType factoryType      = eNotSipFactory;
  CReception*     pSameMRReception = m_pTransferList->FindSameMRReceptionConfOnAirNO(targetConfName);

  while (NULL != pSameMRReception)
  {
    bool       isDefinedParty = pSameMRReception->IsPartyDefined(confMonitorId);
    CNetSetup* pNetSetUp      = pSameMRReception->GetNetSetUp();

    if (isDefinedParty || !(strncmp(pSameMRReception->NameOf(), "CReceptionIVR", 13)))
    {
      CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(confMonitorId);
      PASSERT(!pCommConf);

      if (pCommConf)
      {
        DWORD       partyID    = pSameMRReception->GetPartyId();
        CConfParty* pConfParty = pCommConf->GetCurrentParty(partyID);

        TRACEINTO << "CLobby::ReleaseAllConfOnAirSuspendParties - Defined party found in list, PartyId:" << partyID;

        pSameMRReception->AcceptParty(pNetSetUp, pCommConf, pConfParty);     // in case of IVR invoke MOVE!!!
      }
    }
    else if (!isDefinedParty)
    {
      WORD                isVoice = 0;
      mcTransportAddress* pDestIP = NULL;
      DWORD               ListId  = pSameMRReception->GetPartyId();

      TRACEINTO << "CLobby::ReleaseAllConfOnAirSuspendParties - Undefined party found in list, PartyId:" << ListId;

      if (pNetSetUp)
      {
        BYTE bIsSip = FALSE;

        CConfPartyManagerLocalApi* pConfPartyMngrApi = (CConfPartyManagerLocalApi*)CProcessBase::GetProcess()->GetManagerApi();
        if (!strncmp(pSameMRReception->NameOf(), "CReceptionVoice", 13))
        {
          TRACEINTO << "CLobby::ReleaseAllConfOnAirSuspendParties - Undefined Voice party found in list";
          isVoice = TRUE;
        }
        else if (!strncmp(pSameMRReception->NameOf(), "CReceptionIsdn", 13))
        {
          TRACEINTO << "CLobby::ReleaseAllConfOnAirSuspendParties - Undefined ISDN party found in list";
        }
        else
        {
          pDestIP = (mcTransportAddress*)(((CIpNetSetup*)pNetSetUp)->GetTaDestPartyAddr());
        }

        if (!strncmp(pSameMRReception->NameOf(), "CReceptionSip", 13))
        {
          bIsSip = TRUE;

          BYTE bIsMSConfInvite = ((CSipNetSetup*)pNetSetUp)->GetIsMsConfInvite();

           PTRACE2INT(eLevelInfoNormal,"CLobby::ReleaseAllConfOnAirSuspendParties - bIsMSConfInvite ",bIsMSConfInvite);

          factoryType = ((CReceptionSip*)pSameMRReception)->GetFactoryType();

          bool IsDeallocateDestIP = false;
          if (pDestIP==NULL || (::isApiTaNull(pDestIP) || ::isIpTaNonValid(pDestIP)) )
          {
            IsDeallocateDestIP = true;
            pDestIP = ((CReceptionSip*)pSameMRReception)->FindDestIp((CSipNetSetup*)pNetSetUp);
          }

          const char* pSrcAddr = ((CSipNetSetup*)pNetSetUp)->GetRemoteSipAddress();
          TRACEINTO << "CLobby::ReleaseAllConfOnAirSuspendParties - SIP party dialed to factory " << pSrcAddr;

          sipSdpAndHeadersSt* pSdpAndHeaders = NULL;
          CSipNetSetup*       pSipNetSetup      = NULL;

          // get call information from the suspended object
          ((CReceptionSip*)pSameMRReception)->GetSipCallParams(&pSipNetSetup, &pSdpAndHeaders);
          BYTE initial_encryption_setting = (pSipNetSetup != NULL)?  pSipNetSetup->GetInitialEncryptionValue() : AUTO;
          WORD bIsMrcCall = ::IsMrcHeader(pSdpAndHeaders);
          WORD bIsWebRtcCall = ::IsWebRtcCall(pSdpAndHeaders);
	//eFeatureRssDialin
	WORD srsSessionType =(WORD)  ::getSrsSessionType(pSdpAndHeaders);

          if (eNotSipFactory != factoryType)
          {
            if (e302SipFactory == factoryType)
            {
              ListId = 0;


              // get call information from the suspended object
              TRACEINTO << "CLobby::ReleaseAllConfOnAirSuspendParties - SIP party dialed to 'redirect' factory";

              // get call information from the suspended object
              RejectSipCall(pSipNetSetup, int(cmReasonTypeCallForwarded), pSdpAndHeaders, ((CReceptionSip*)pSameMRReception)->GetFactoryCreatedConfName());

            }
            else
              pConfPartyMngrApi->AcceptUnreservedParty(pDestIP, confMonitorId, ListId, isVoice, bIsSip, factoryType, pSrcAddr, initial_encryption_setting, "", bIsMrcCall, bIsWebRtcCall,/* CASCADE_MODE_NONE,*/bIsMSConfInvite, srsSessionType);
          }
          else
            pConfPartyMngrApi->AcceptUnreservedParty(pDestIP, confMonitorId, ListId, isVoice, bIsSip, factoryType, pSrcAddr, initial_encryption_setting, "", bIsMrcCall, bIsWebRtcCall, /*CASCADE_MODE_NONE,*/bIsMSConfInvite, srsSessionType);

          if (IsDeallocateDestIP)
            PDELETE(pDestIP);
          POBJDELETE(pSipNetSetup);
          PDELETEA(pSdpAndHeaders);
        }
        else
          // NOT SIP
          pConfPartyMngrApi->AcceptUnreservedParty(pDestIP, confMonitorId, ListId, isVoice, bIsSip, factoryType);
      }

      // Only undefined parties should stay in the reception list
      pSameMRReception->SetConfOnAirFlag(YES); // to set Move timer and delete all moved parties!!!!
      m_pTransferList->Update(pSameMRReception);

      if (e302SipFactory == factoryType)
      {
        RemoveFromList(pSameMRReception);
        PDELETE(pSameMRReception);
      }
    }

    pSameMRReception = m_pTransferList->FindSameMRReceptionConfOnAirNO(targetConfName);
  } // while

  PTRACE(eLevelInfoNormal, "CLobby::ReleaseAllConfOnAirSuspendParties - All suspend parties for this conference are removed from list");
}

//--------------------------------------------------------------------------
void CLobby::SetPartyListIDForSuspendCall(const char* strSrcPartyAddress, const char* strDstPartyAddress)
{
  // put incoming call information into a holding queue
  // the call will be suspend until a release message is received
  // from an internal lobby timer or an external message
  DWORD counter = (DWORD) m_pParyList_Id;
  if (counter == 0xffff)
    counter = 0x0001;
  else
    counter++;

  m_pParyList_Id = (CTaskApp*)counter;  // advance party list id number

  TRACEINTO << "CLobby::SetPartyListIDForSuspendCall - Incoming call suspended"
            << ", CallId:"   << (DWORD)m_pParyList_Id
            << ", CalleeIP:" << DUMPSTR(strDstPartyAddress)
            << ", CallerIP:" << DUMPSTR(strSrcPartyAddress);
}

//--------------------------------------------------------------------------
void CLobby::IdentifyH323Party(CH323NetSetup* pNetSetup, CCommConf* pComConf, CConfParty* pConfParty, COsQueue* pConfRcvMbx, COsQueue* pLobbyRcvMbx, const char* confName, DWORD monitorConfId, WORD isChairEnabled)
{
  PASSERT_AND_RETURN(!pNetSetup);
  PASSERT_AND_RETURN(!pComConf);
  PASSERT_AND_RETURN(!m_pTransferList);

  char name[H243_NAME_LEN];
  SAFE_COPY(name, pConfParty->GetName());

  // pass allocated H323 descriptor to the reception dial in party
  CReceptionH323* pLobbyTrans = new CReceptionH323(this);
  pLobbyTrans->CreateH323(monitorConfId, pConfParty->GetPartyId(), pConfRcvMbx, pLobbyRcvMbx, pNetSetup, this, name, confName, pConfParty, pComConf); //shiraITP - 52
  m_pTransferList->Insert(pLobbyTrans);
}


/*
  THE FUNCTION CHANGES
  1. the allocation of the ATM/ISDN Reception -> doesn't exist in H323
  2. the resource type -> doesn't exist in H323
  3. Replacement:
    in the function variables:
      CNetSetup* pNetSetup              with CH323NetSetup* pNetSetup
      CNetConnRsrcDesc* pNetDesc  with CIpRsrcDesc *pIpDesc
      CReception*  pLobbyTrans        with CReceptionH323*  pLobbyTrans
      None means no existence.
*/
//--------------------------------------------------------------------------
void CLobby::RejectH323Call(CH323NetSetup* pNetSetup, int reason)
{
  TRACEINTO << "CLobby::RejectH323Call - Call rejected, CallerIP:" << DUMPSTR(pNetSetup->GetSrcPartyAddress());
  CReceptionH323* pLobbyTrans = new CReceptionH323(this);
  pLobbyTrans->CreateH323(pNetSetup, m_pRcvMbx, this, reason); // Create for reject.
  pLobbyTrans->Destroy(false);
  POBJDELETE(pLobbyTrans);
}

/*
 Function name: GetH323ConfPartyType(Updated by Yoella 21/2/06)
 Variables:     partyIPaddress: The IP address of the dial in setup message.
 pSrcH323AliasArray: The source aliases.
 wSrcNumAlias: Source number of aliases.
 pConf: Out value. The information on the selected conference at the DB.
 pParty: Out value. The information on the selected party at the DB.
 pDestH323AliasArray: The destination 'aliases' as they appear at the setup string.
 wDestNumAlias: destination number of 'aliases'.
 meetingRoomId
 isH323
 pAdHocConfName

 Description:   Find the party that match the dial in setup string and returned the party and conference that was found at DB.
 1.Search the H323 party in the ConfDB (for AdHoc
 Return value:  0  - for found party and conference (defined party).
 -1 - not found
 -2 - for found undefined party.
 -3 - for found locked conference.
 -4 - for found party and conference but max parties number in conference has been reached
-7 - for found both conf and party, but party is in connected status.
+1 - for found meeting room or GW call
*/
//--------------------------------------------------------------------------
int CLobby::GetIpConfPartyType(const mcTransportAddress* pPartyIPaddress, CH323Alias* pSrcH323AliasArray, WORD wSrcNumAlias, CCommConf** pConf, CConfParty** pConfParty, CH323Alias* pDestH323AliasArray, WORD wDestNumAlias, DWORD& meetingRoomId, BYTE isH323, char* pAdHocConfName, WORD useTransitEQ)
{
  int val = 0;

  *pConf      = NULL;
  *pConfParty = NULL;

  // if Ad-hoc, look only in meeting rooms
  // calling this function for H323 pAdHocConfName=NULL always!!this "if" is relevant only for SIP
  if (!pAdHocConfName)
  {
    // (1) Meet me per conference (this search looking for the party too)
    // IpV6
    val = ::GetpConfDB()->SearchForH323ConferenceMatch(pPartyIPaddress, pSrcH323AliasArray, wSrcNumAlias, pConf, pConfParty, pDestH323AliasArray, wDestNumAlias, useTransitEQ, isH323);
    if (val != -1)              // there was a match in the search for meet_me_per_conference
    {
      TRACEINTO << "CLobby::GetIpConfPartyType - IpConfPartyType:" << val;
      return val;
    }
  }

  // val = -1  or 0 ConfDb does not got this Conf
  // (2) Meeting room */

  char pTempMeetingRoomConfName[H243_NAME_LEN] = { 0 };
  int meeting_room_found = -1;
  if (pDestH323AliasArray)
    meeting_room_found = ::GetpMeetingRoomDB()->GetMeetingRoomForH323Call(pDestH323AliasArray, wDestNumAlias, pTempMeetingRoomConfName, meetingRoomId, useTransitEQ, isH323);

  if (meeting_room_found == 0)  // CONFERENCE ROOM WAS FOUND
  {
    TRACEINTO << "CLobby::GetIpConfPartyType - Meeting room found, ConfName:" << pTempMeetingRoomConfName;
    val = 1;
  }

  /* (4) defined parties */
  /*CARMEL dont care yet for System.cfg*/
  if (val != 1 && !(::GetpConfDB()->IsMeetingRoomSpecified(pDestH323AliasArray, wDestNumAlias, isH323)))  // VNGR-24962
  {
	  val = ::GetpConfDB()->SearchForH323DefinedMatch(*pPartyIPaddress, pSrcH323AliasArray, wSrcNumAlias, pConf, pConfParty, pDestH323AliasArray, wDestNumAlias);
  }

  if (val == 0)
  {
    // direct Dial In found a reservation to this party in the MCU data base
    // checking if the received party is fully connected. if yes - reject the party
    PASSERT_AND_RETURN_VALUE(!(*pConfParty), val);

    const char* tempName = (*pConfParty)->GetName();
    if (tempName)
      TRACEINTO << "CLobby::GetIpConfPartyType - Party found, PartyName:" << tempName;

    DWORD party_status = (*pConfParty)->GetPartyState();

    switch (party_status)
    {
      case PARTY_CONNECTED:
      case PARTY_SECONDARY:
      case PARTY_CONNECTING:
      case PARTY_CONNECTED_WITH_PROBLEM:
      case PARTY_DISCONNECTING:
      {
        DWORD confStat = (*pConf)->GetStatus();
        if (confStat & CONFERENCE_RESOURCES_DEFICIENCY)
          TRACEINTO << "CLobby::GetIpConfPartyType - Conference resource deficiency (call rejected)";
        else
          TRACEINTO << "CLobby::GetIpConfPartyType - Destination party is full connected (call rejected)";
        val = -1;
        break;
      }

      default:
      {
        if ((*pConfParty)->IsDefinedPartyAssigned())
        {
          TRACEINTO << "CLobby::GetIpConfPartyType - Destination party is connecting (call rejected)";
          val = -1; // unattended call handle ...
        }
        else
          (*pConfParty)->SetDefinedPartyAssigned(TRUE);
        break;
      }
    } // switch
  }

  if (val != 1 && !isH323)
  {
    CCommRes* pFactory = ::GetpMeetingRoomDB()->GetCurrentRsrv(pDestH323AliasArray->GetAliasName());
    if (pFactory && pFactory->IsSIPFactory())
      val = 2;

    POBJDELETE(pFactory);
  }
  TRACEINTO << "CLobby::GetIpConfPartyType - IpConfPartyType:" << val;
  return val;
}

//--------------------------------------------------------------------------
void CLobby::OnEndPartyTransfer(CReception* pLobbyTransfer)
{
  PASSERT_AND_RETURN(!pLobbyTransfer);
  TRACEINTO << "PartyName:" << pLobbyTransfer->GetPartyName();
	//TRACEINTO << "CLobby::OnEndPartyTransfer - RemoveFromList reception = " << (DWORD)pLobbyTransfer;
  RemoveFromList(pLobbyTransfer);
}

//--------------------------------------------------------------------------
void CLobby::OnRejectParty(CSegment* pParam)
{
  ConfMngrRejectParty(pParam, TRUE);
}

//--------------------------------------------------------------------------
void CLobby::OnConfMngrRejectParty(CSegment* pParam)
{
  ConfMngrRejectParty(pParam, FALSE);
}

//--------------------------------------------------------------------------
void CLobby::ConfMngrRejectParty(CSegment* pParam, BYTE rejectOnlyParty)
{
  CReception* pReception = NULL;
  CTaskApp*   pParty     = NULL;
  DWORD       confId     = 0;
  DWORD       listId     = 0;
  WORD        status     = 0;
  BYTE        isAdHoc    = FALSE;
  char        adHocConfName[H243_NAME_LEN];

  // ConfPartyManager has decided to reject the un-reserved call
  *pParam >> confId >> listId >> status;
  if (FALSE == rejectOnlyParty)
    *pParam >> isAdHoc;

  TRACEINTO << "CLobby::ConfMngrRejectParty - Reject a suspended Incoming call "
            << "- ConfId:"  << confId
            << ", CallId:"  << listId
            << ", Status:"  << status
            << ", isAdHoc:" << (int)isAdHoc;

  // get suspended parameters from list
  // convert from list id arrived from ConfPartyManager to a party pointer
  // the pointer is not a valid pointer to a party object
  // it is only used as an identifier in the waiting list
  char MRName[H243_NAME_LEN];
  memset(MRName, '\0', H243_NAME_LEN);

  if (listId != 0xFFFFFFFF)
  {
    CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(confId);
    pParty = (CTaskApp*)listId;

    m_pLobbyTrans->SetParty(pParty);
    pReception = m_pTransferList->Find(m_pLobbyTrans);

    if (CPObject::IsValidPObjectPtr(pCommConf))
      SAFE_COPY(MRName, pCommConf->GetName());
  }
  else
  {
    if (isAdHoc)
    {
      *pParam >> adHocConfName;
      pReception = m_pTransferList->FindSameMRReceptionConfOnAirNO(adHocConfName);
      SAFE_COPY(MRName, adHocConfName);
      TRACEINTO << "CLobby::OnConfMngrRejectParty - Reject AdHoc conference, ConfName:" << MRName;
    }
    else
    {
      CCommRes* pMR = ::GetpMeetingRoomDB()->GetCurrentRsrv(confId);
      PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pMR));

      pReception = m_pTransferList->FindSameMRReceptionConfOnAirNO((char*)pMR->GetName());
      SAFE_COPY(MRName, pMR->GetName());
      TRACEINTO << "CLobby::OnConfMngrRejectParty - Reject Meeting Room, ConfName:" << MRName;
    }
  }

  PASSERT_AND_RETURN(!pReception);

  // VNGR-11187 - reject all suspended parties for this MR/Conf
  while (NULL != pReception)
  {
    if (!strcmp(pReception->NameOf(), "CReceptionH323")) // case of H323 Call In
    {
      CH323NetSetup* pNetSetup = NULL;

      // get call information from the suspended object
      ((CReceptionH323*)pReception)->GetH323CallParams(&pNetSetup);

      // create new reception object for the accept proccess
      if (pNetSetup)
        // meeting room or Undefined Dial In: can't make "forwarding" in this case
        RejectH323Call(pNetSetup, int(cmReasonTypeNoPermision));
      else
        PASSERT(1);

      POBJDELETE(pNetSetup);
    }

    else
    {
      if (!strcmp(pReception->NameOf(), "CReceptionSip")) // case of Sip Call In
      {
        CSipNetSetup*       pNetSetup      = NULL;
        sipSdpAndHeadersSt* pSdpAndHeaders = NULL;

        // get call information from the suspended object
        ((CReceptionSip*)pReception)->GetSipCallParams(&pNetSetup, &pSdpAndHeaders);

        // create new reception object for the accept process
        if (pNetSetup)
          RejectSipCall(pNetSetup, int(cmReasonTypeNoPermision), pSdpAndHeaders, NULL, (STATUS)status);
        else
          PASSERT(1);

        POBJDELETE(pNetSetup);
        PDELETEA(pSdpAndHeaders);
      }
      else
      {
        if (!strcmp(pReception->NameOf(), "CReceptionIVR")) // case of reject AdHocConference // TBD
        { }
      }
    }

    // remove suspended reception item
		//TRACEINTO << "CLobby::ConfMngrRejectParty - RemoveFromList reception = " << (DWORD)pReception;
    RemoveFromList(pReception);
    pReception->HandleEvent(pParam, 0, REJECT_UNRESERVED_PARTY);

    pReception = m_pTransferList->FindSameMRReceptionConfOnAirNO(MRName);
  }
}

//--------------------------------------------------------------------------
WORD CLobby::IsVoiceCall(const CNetSetup* pNetSetup) const
{
  if ((pNetSetup->m_callType == ACU_VOICE_SERVICE) || (pNetSetup->m_callType == ACU_MODEM_SERVICE))
  {
    PTRACE2INT(eLevelInfoNormal, "CLobby::IsVoiceCall - IsVoiceCall:YES, CallType:", pNetSetup->m_callType);
    return YES;
  }

  PTRACE2INT(eLevelInfoNormal, "CLobby::IsVoiceCall - IsVoiceCall:NO, CallType:", pNetSetup->m_callType);
  return NO;
}


//--------------------------------------------------------------------------
void CLobby::RemoveReception(CReception* pReception)
{
	//TRACEINTO << "CLobby::RemoveReception - RemoveFromList reception = " << (DWORD)pReception;
  RemoveFromList(pReception);
}

//--------------------------------------------------------------------------
//BRIDGE-5753
void CLobby::SetTipForPartyIfNeeded(CConfParty* pConfParty, CCommConf* pCommConf, CSipNetSetup* pNetSetup)
{
	PASSERTMSG_AND_RETURN(!pConfParty || !pCommConf || !pNetSetup, "!pConfParty || !pCommConf || !pNetSetup");

	if (FALSE == pCommConf->GetIsTipCompatibleVideo())
		return;

	BYTE        interfaceType = pConfParty->GetNetInterfaceType();
	const char* strContact    = pNetSetup->GetRemoteSipContact();
	BOOL        bIsTipHeader  = FALSE;
	BOOL        bIsTipCall    = FALSE;
	BOOL        bIsSip        = FALSE;

	bIsSip = (interfaceType == SIP_INTERFACE_TYPE);

	if (bIsSip && (strcasestr(strContact, "x-cisco-tip") || strcasestr(strContact, "x-cisco-multiple-screen")))
		bIsTipHeader = TRUE;
	else
		bIsTipHeader = FALSE;

	pConfParty->SetIsTipHeader(bIsTipHeader);

	bIsTipCall = bIsSip && bIsTipHeader;

	pConfParty->SetIsTipCall(bIsTipCall);

	TRACEINTO << "IsSip:" << (int)bIsSip << ", IsTipHeader:"<< (int)bIsTipHeader << ", IsTipCall:" << (int)bIsTipCall;
}
//--------------------------------------------------------------------------
void CLobby::SetDBMSConversationId(CConfParty* pConfParty,  CSipNetSetup* pNetSetup)
{
	if(0 != strlen(pNetSetup->GetMsConversationId()))
	{
		pConfParty->SetMsConversationId(pNetSetup->GetMsConversationId());
	}
}
//--------------------------------------------------------------------------
void CLobby::SetAliasDisplayName(findConfAndPartyKeysSt &keys,const char* aliasName)
{
	int len = strlen(aliasName);
	if(len > 0)
	{
		if (len < ALIAS_NAME_LEN)
		{
			keys.pPartySrcAliasList = new CH323Alias;
			keys.pPartySrcAliasList->SetAliasName(aliasName);
			keys.pPartySrcAliasList->SetAliasType(PARTY_H323_ALIAS_DISPLAY_ID_TYPE);
			keys.numOfSrcAlias++;
		}
		else
		{
			TRACEINTO << "Invalid  alias name length: " << aliasName;
			PASSERT(1);
		}
	}
}

//--------------------------------------------------------------------------
void CLobby::OnConfReleaseParty(CSegment* pParam)
{
	ConfMonitorID confMonitorId  = 0;
	PartyMonitorID partyMonitorId = 0;
	DWORD lobbyListId = 0;

	// conference has decided to accept the un-reserved call
	*pParam >> confMonitorId >> partyMonitorId >> lobbyListId;

	TRACEINTO << "MonitorConfId:"    << confMonitorId
	          << ", MonitorPartyId:" << partyMonitorId
	          << ", LobbyListId:"    << lobbyListId << " - Accept a suspended Incoming call";

	// get suspended parameters from list convert from list id to a party pointer
	// the pointer is not a valid pointer to a party object it is only used as an identifier in the waiting list
	CTaskApp* pParty = (CTaskApp*)lobbyListId;

	m_pLobbyTrans->SetParty(pParty);
	CReception* pReception = m_pTransferList->Find(m_pLobbyTrans);
	PASSERT_AND_RETURN(!pReception);

	// FIND CALL IN DATA BASE according to conference id and party id
	CConfParty* pConfParty = (CConfParty*)::GetpConfDB()->GetCurrentParty(confMonitorId, partyMonitorId);
	CCommConf*  pCommConf  = ::GetpConfDB()->GetCurrentConf(confMonitorId);

	if (!strcmp(pReception->NameOf(), "CReceptionVoice")) // case of VoiceParty
	{
		CIsdnNetSetup* pNetSetup = NULL;
		((CReceptionVoice*)pReception)->GetCallParams(&pNetSetup);

		if (pNetSetup)
		{
			if (pCommConf && pConfParty)
			{
				TRACEINTO <<"MonitorPartyId:" << partyMonitorId << " - Accept the Voice party";
				AcceptCallIn(pNetSetup, pCommConf, pConfParty);
			}
			else
			{
				// the ComConf or the ConfParty are corrupted but we can still reject the call
				TRACEINTO <<"MonitorPartyId:" << partyMonitorId << " - Reject the Voice party";
				RejectIsdnCall(pNetSetup, 1);
			}
			POBJDELETE(pNetSetup);
		}
		else
		{
			PASSERT(1);
		}
	}

	if (!strcmp(pReception->NameOf(), "CReceptionIsdn")) // case of ISDN video Party
	{
		CIsdnNetSetup* pNetSetup = NULL;
		((CReceptionIsdn*)pReception)->GetCallParams(&pNetSetup);

		if (pNetSetup)
		{
			if (pCommConf && pConfParty)
			{
				TRACEINTO <<"MonitorPartyId:" << partyMonitorId << " - Accept the ISDN party";
				AcceptCallIn(pNetSetup, pCommConf, pConfParty);
			}
			else
			{
				// the ComConf or the ConfParty are corrupted but we can still reject the call
				TRACEINTO <<"MonitorPartyId:" << partyMonitorId << " - Reject the ISDN party";
				RejectIsdnCall(pNetSetup, 1);
			}
			POBJDELETE(pNetSetup);
		}
		else
		{
			PASSERT(1);
		}
	}
	else if (!strcmp(pReception->NameOf(), "CReceptionH323")) // case of H323 Call In
	{
		CH323NetSetup* pNetSetup = NULL;
		((CReceptionH323*)pReception)->GetH323CallParams(&pNetSetup);

		if (pNetSetup)
		{
			if (pCommConf && pConfParty)
			{
				TRACEINTO <<"MonitorPartyId:" << partyMonitorId << " - Accept the 323 party";
				AcceptH323CallIn(pNetSetup, pCommConf, pConfParty); // shiraITP - 50
			}
			else
			{
				// the ComConf or the ConfParty are corrupted but we can still reject the call
				TRACEINTO <<"MonitorPartyId:" << partyMonitorId << " - Reject the 323 party";
				RejectH323Call(pNetSetup, int(cmReasonTypeNoPermision));
			}
			POBJDELETE(pNetSetup);
		}
		else
		{
			PASSERT(1);
		}
	}
	else if (!strcmp(pReception->NameOf(), "CReceptionSip")) // case of Sip Call In
	{
		CSipNetSetup* pNetSetup = NULL;
		sipSdpAndHeadersSt* pSdpAndHeaders = NULL;
		BYTE bIsSoftCp = (pCommConf && pCommConf->GetVideoSession() == SOFTWARE_CONTINUOUS_PRESENCE) ? YES : NO;

		// get call information from the suspended object
		((CReceptionSip*)pReception)->GetSipCallParams(&pNetSetup, &pSdpAndHeaders);
		eSipFactoryType factoryType = ((CReceptionSip*)pReception)->GetFactoryType();
		if (e302SipFactory == factoryType)
		{
			// get call information from the suspended object
			TRACEINTO <<"MonitorPartyId:" << partyMonitorId << " - Reject the SIP party, party dialed to 'redirect' factory";
			RejectSipCall(pNetSetup, int(cmReasonTypeCallForwarded), pSdpAndHeaders, ((CReceptionSip*)pReception)->GetFactoryCreatedConfName());
		}
		// create new reception object for the accept process
		else if (pNetSetup && pSdpAndHeaders && pCommConf && pConfParty && !((CReceptionSip*)pReception)->IsMultiInvite())
		{
			TRACEINTO <<"MonitorPartyId:" << partyMonitorId << " - Accept the SIP party";

			SetTipForPartyIfNeeded(pConfParty, pCommConf, pNetSetup);

			SetDBMSConversationId(pConfParty,pNetSetup);


			// In case this is MS-conf-invite call we don't want to create dial in party
			// we want to start dial out flow.
			if (pConfParty->GetMsftAvmcuState() != eMsftAvmcuNone)
			{
				TRACEINTO <<"MonitorPartyId:" << partyMonitorId << ", AVMCUstate:" << enMsftAvmcuStateNames[pConfParty->GetMsftAvmcuState()] << " - AV MCU party, will start dial out flow";

				pConfParty->SetCascadeMode(CASCADE_MODE_MASTER);
				COsQueue* pConfRcvMbx      = pCommConf->GetRcvMbx();
				DWORD     SdpAndHeadersLen = sizeof(sipSdpAndHeadersBaseSt) + pSdpAndHeaders->lenOfDynamicSection;
				CConfApi* pConfApi         = new CConfApi;
				pConfApi->CreateOnlyApi(*pConfRcvMbx);
				pConfApi->ReleaseAVMCUParty(partyMonitorId, pNetSetup, pSdpAndHeaders, SdpAndHeadersLen);
				POBJDELETE(pConfApi);
			}
			else
			{
				TRACEINTO << "IP VERSION: " << (DWORD)pNetSetup->GetIpVersion();
				AcceptSipCallIn(pNetSetup, pCommConf, pConfParty, bIsSoftCp, pSdpAndHeaders);
			}
		}
		else if (pNetSetup != NULL)
		{
			// the ComConf or the ConfParty are corrupted but we can still reject the call
			// also reject in case of multiInvite
			TRACEINTO <<"MonitorPartyId:" << partyMonitorId << " - Reject the SIP party";
			RejectSipCall(pNetSetup, int(cmReasonTypeNoPermision), pSdpAndHeaders);
		}
		else
			PASSERT(1);

		POBJDELETE(pNetSetup);
		PDELETEA(pSdpAndHeaders);
	}

	pParam->ResetRead();
	pReception->HandleEvent(pParam, 0, RELEASE_UNRESERVED_PARTY);
}

//--------------------------------------------------------------------------
void CLobby::OnConfMngrSuspendIVRParty(CSegment* pParam)
{
  DWORD           dwSourceConfId       = 0xFFFFFFFF;
  DWORD           dwPartyId            = 0xFFFFFFFF;
  DWORD           dwTargetConfId       = 0xFFFFFFFF;
  CCommRes*       pAdHocRsrv           = NULL;
  const char*     confName             = NULL;
  CCommResShort*  pMeetingRoomShortRes = NULL;
  DWORD           tempTargetConfType;

  *pParam >> dwSourceConfId >> dwPartyId >> tempTargetConfType;
  ETargetConfType targetConfType = (ETargetConfType)tempTargetConfType;

  TRACEINTO << "CLobby::OnConfMngrSuspendIVRParty - Accept a suspended incoming call, ConfId:" << dwSourceConfId << ", PartyId:" << dwPartyId << ", TargetConfType:" << targetConfType;

  switch (targetConfType)
  {
    case eMeetingRoom:
    {
      *pParam >> dwTargetConfId;
      pMeetingRoomShortRes = ::GetpMeetingRoomDB()->GetCurrentRsrvShort(dwTargetConfId);
      PASSERT_AND_RETURN(!pMeetingRoomShortRes);
      confName = pMeetingRoomShortRes->GetName();
      break;
    }

    case eAdHoc:
    {
      pAdHocRsrv = new CCommRes;
      pAdHocRsrv->DeSerialize(NATIVE, *pParam);
      confName   = pAdHocRsrv->GetName();
      break;
    }

    default:
    {
      PASSERT_AND_RETURN(1);
    }
  } // switch

  SetPartyListIDForSuspendCall();

  CReceptionIVR* pLobbyTrans = new CReceptionIVR(this);

  if (YES == TargetReceptionAlreadyExist(const_cast<char*>(confName)))
    pLobbyTrans->CreateSuspendIVR(m_pRcvMbx, this, m_pParyList_Id, dwSourceConfId, const_cast<char*>(confName), dwPartyId, targetConfType);
  else // This is the party to awake the MR - create the AdHoc conference
    pLobbyTrans->CreateConfAndSuspendIVRMove(m_pRcvMbx, this, m_pParyList_Id, dwSourceConfId, const_cast<char*>(confName), dwPartyId, targetConfType, pAdHocRsrv);

  m_pTransferList->Insert(pLobbyTrans); // When conference is UP those Receptions will removed from list

  POBJDELETE(pAdHocRsrv);
  POBJDELETE(pMeetingRoomShortRes);
}

//--------------------------------------------------------------------------
WORD CLobby::SetNewDestinationConfName(CIpNetSetup* pNetSetup, char* newDestConfName, WORD IsSip)
{
  PASSERT_AND_RETURN_VALUE(!newDestConfName, FALSE);
  PASSERT_AND_RETURN_VALUE(!newDestConfName[0], FALSE);

  const char* oldFullAddress = pNetSetup->GetDestPartyAddress();

  PASSERT_AND_RETURN_VALUE(!oldFullAddress, FALSE);
  PASSERT_AND_RETURN_VALUE(!oldFullAddress[0], FALSE);

  int   oldFullAddressLength = strlen(oldFullAddress);
  char* ret = (char*)strchr(oldFullAddress, ',');
  int   IPAddressLength = 0;
  if (ret != NULL) // the destination address is with extension, we need just the IP address for the new destination address
  {
    int retLength = strlen(ret);
    IPAddressLength = oldFullAddressLength - retLength;
  }
  else // The old destination address holds just IP address of the card
  {
    IPAddressLength = oldFullAddressLength;
  }

  IPAddressLength = min(IPAddressLength, IP_LIMIT_ADDRESS_CHAR_LEN-1);
  char newFullAddress[IP_LIMIT_ADDRESS_CHAR_LEN] = { 0 };
  if (!IsSip)
  {
    strncpy(newFullAddress, oldFullAddress, IPAddressLength);
	newFullAddress[IPAddressLength] = '\0';
    strncat(newFullAddress, ",", 1);
    int newDestConfNameLength = strlen(newDestConfName);
    strncat(newFullAddress, newDestConfName, newDestConfNameLength);
  }
  else
  {
    int newDestConfNameLength = strlen(newDestConfName);
    newDestConfNameLength = min(newDestConfNameLength, IP_LIMIT_ADDRESS_CHAR_LEN-1);
    strncpy(newFullAddress, newDestConfName, newDestConfNameLength);
	newFullAddress[newDestConfNameLength] = '\0';
    strncat(newFullAddress, "@", 1);
    strncat(newFullAddress, oldFullAddress, IPAddressLength);
  }
  TRACEINTO << "CLobby::SetNewDestinationConfName - newFullAddress: " << newFullAddress << ", IsSIP:" << IsSip;

  pNetSetup->SetDestPartyAddress(newFullAddress);
  pNetSetup->SetDestPartyAliases(newFullAddress);
  return TRUE;
}

//--------------------------------------------------------------------------
BYTE CLobby::TargetReceptionAlreadyExist(char* targetConfName)
{
  return m_pTransferList->FindSameMRReceptionConfOnAirNO(targetConfName) ? YES : NO;
}

//--------------------------------------------------------------------------
BYTE CLobby::IsReceptionStillInList(CReception* pReception)
{
  CReception* pReceptionFound = m_pTransferList->Find(pReception);
  return (pReceptionFound) ? YES : NO;
}

//--------------------------------------------------------------------------
void CLobby::RemoveFromList(CReception* pLobbyTransfer)
{
  CReception* pErasedReception = m_pTransferList->Remove(pLobbyTransfer);
//	TRACEINTO << "CLobby::RemoveFromList: pLobbyTransfer->GetParty (reception passed as parameter): " << (DWORD)pLobbyTransfer->GetParty()
//										<< ", pErasedReception->GetParty (erazed reception): " << (DWORD)pErasedReception->GetParty();
  PASSERT(!pErasedReception);
}

//--------------------------------------------------------------------------
void CLobby::AcceptCallIn(CIsdnNetSetup* pNetSetUp, CCommConf* pComConf, CConfParty* pConfParty)
{
  char pCalledTelNumber [PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER];
  char pCallingTelNumber[PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER];
  memset(pCalledTelNumber, 0, sizeof(pCalledTelNumber));
  memset(pCallingTelNumber, 0, sizeof(pCallingTelNumber));

  COsQueue*   pConfRcvMbx   = pComConf->GetRcvMbx();
  const char* confName      = pComConf->GetName();
  DWORD       monitorConfId = pComConf->GetMonitorConfId();
  WORD        numDigits     = 0;

  pNetSetUp->GetCalledNumber(&numDigits, pCalledTelNumber);
  pNetSetUp->GetCallingNumber(&numDigits, pCallingTelNumber);

  TRACEINTO << "CLobby::AcceptCallIn "
            << "- CalleeIP:" << DUMPSTR(pCalledTelNumber)
            << ", CallerIP:" << DUMPSTR(pCallingTelNumber);

  WORD seqNum = 0;
  CDwordBitMask ActiveChannelsMask = pConfParty->GetNet_channels();
  if (ActiveChannelsMask.GetDWORD())
  {
    int NumActiveChannels = ActiveChannelsMask.GetNumberOfSetBits();
    if (NumActiveChannels >= pConfParty->GetNetChannelNumber())
    {
      TRACEINTO << "CLobby::AcceptCallIn - Reject call (all party channels are connected)";
      RejectIsdnCall(pNetSetUp, 1);
      return;
    }
  }

  // if not Voice and Bonding and ActiveChannelsMask - Add channel
  if (!pConfParty->GetVoice() && pConfParty->GetBondingMode1() && ActiveChannelsMask.GetDWORD())
  {
    seqNum = ActiveChannelsMask.GetFirstFalseBitIndex()-1;

    TRACEINTO << "CLobby::AcceptCallIn - PER USER + BONDING "
              << "- PartyName:"          << (char*)pConfParty->GetName()
              << ", Additional channel:" << seqNum;

    pConfParty->SetNet_channels(TRUE, seqNum+1, 1, pNetSetUp->m_callType);
    CConfApi* pConfApi = new CConfApi;
    pConfApi->CreateOnlyApi(*pConfRcvMbx);
    pConfApi->AddPartyChannelDesc(pNetSetUp, (char*)pConfParty->GetName(), seqNum);
    POBJDELETE(pConfApi);
  }
  else            // bonding initial channel
  {
    pConfParty->SetNet_channels(TRUE, 1, 1, pNetSetUp->m_callType);
    IdentifyParty(pNetSetUp, pComConf, pConfParty, pConfRcvMbx, m_pRcvMbx, confName, monitorConfId);
  }
}

//--------------------------------------------------------------------------
void CLobby::OnNetCallIn(CSegment* pMplParam)
{
  NET_SETUP_IND_S setupIndStruct;
  memset(&setupIndStruct, 0, sizeof(NET_SETUP_IND_S));

  pMplParam->Get((BYTE*)(&setupIndStruct), sizeof(NET_SETUP_IND_S));

  BYTE box_id = 0, boardId = 0, sub_board_id = 0;
  (*pMplParam) >> box_id;
  (*pMplParam) >> boardId; // Getting the boardId
  (*pMplParam) >> sub_board_id;

  std::auto_ptr<CIsdnNetSetup> pNetSetup(new CIsdnNetSetup);

  pNetSetup->m_callType = setupIndStruct.call_type;
  pNetSetup->m_netSpcf  = setupIndStruct.net_spfc;
  pNetSetup->SetCalledNumber(setupIndStruct.called_party.num_digits, (const char*)setupIndStruct.called_party.digits);
  pNetSetup->ResetCallingNumber();
  pNetSetup->SetCallingNumber(setupIndStruct.calling_party.num_digits, (const char*)setupIndStruct.calling_party.digits);

  pNetSetup->m_boardId      = boardId; // Set the boardId
  pNetSetup->m_box_id       = box_id;
  pNetSetup->m_sub_board_id = sub_board_id;

  // Set the header params
  pNetSetup->SetNetCommnHeaderParams(setupIndStruct.net_common_header);

  pNetSetup->LegalizeCalledNumber();

  WORD numDigits = 0;

  char pCalledTelNumber [PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER];
  char pCallingTelNumber[PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER];
  memset(pCalledTelNumber, 0, sizeof(pCalledTelNumber));
  memset(pCallingTelNumber, 0, sizeof(pCallingTelNumber));

  pNetSetup->GetCalledNumber(&numDigits, pCalledTelNumber);
  pNetSetup->GetCallingNumber(&numDigits, pCallingTelNumber);

  TRACEINTO << "CLobby::OnNetCallIn - Incoming Call "
            << "- CalleeIP:" << DUMPSTR(pCalledTelNumber)
            << ", CallerIP:" << DUMPSTR(pCallingTelNumber);

  // For HotBackup validity
  CProcessBase* pConfPartyProcess = CProcessBase::GetProcess();
  if (TRUE == pConfPartyProcess->GetIsFailoverSlaveMode())
  {
    TRACEINTO << "CLobby::OnNetCallIn - RMX is in Slave MODE (call rejected)";
    RejectIsdnCall(pNetSetup.get(), 1);
    return;
  }

  // LOOKUP inclooming call in data base
  CCommConf*  pComConf = NULL;
  DWORD       confId;
  char        meetingRoomConfName[H243_NAME_LEN] = { 0 };
  CConfParty* pConfParty    = NULL;
  int lookup_result = FindDestConf(pCalledTelNumber, pCallingTelNumber, meetingRoomConfName, confId, (CCommConf**)&pComConf);

  if (lookup_result == 1 && pComConf->FindPartyByBondingTelNum(pCalledTelNumber, (CConfParty**)&pConfParty))
    lookup_result = 3;

  // lookup results from this function can be :
  // 0 - Did not found any conference
  // 1 - Found Ongoing conf
  // 2 - Found MR
  // 3 - Party is found according to bonding temp number.

  switch (lookup_result)
  {
    case 3:
    {           // found a reservation to this party in the MCU data base
      if (!IsValidPObjectPtr(pConfParty))
      {
        PASSERTMSG(1, "CLobby::OnNetCallIn - Failed, Called number was found but CONF/PARTY objects in database are invalid (call rejected)");
        RejectIsdnCall(pNetSetup.get(), 1);
      }
      else
      {
        // Checking if the received party is fully connected. If YES - reject the party
        DWORD party_status = pConfParty->GetPartyState();

        switch (party_status)
        {
          case PARTY_CONNECTED:
          case PARTY_SECONDARY:
          case PARTY_CONNECTED_WITH_PROBLEM:
          case PARTY_DISCONNECTING:
          {
            DWORD confStat = (pComConf) ? pComConf->GetStatus() : 0;
            if (confStat & CONFERENCE_RESOURCES_DEFICIENCY)
            {
              TRACEINTO << "CLobby::OnNetCallIn - Failed, Conference resource deficiency (call rejected)";
              RejectIsdnCall(pNetSetup.get(), 1);
              return;
            }

            pNetSetup->GetCalledNumber(&numDigits, pCalledTelNumber);
            pNetSetup->GetCallingNumber(&numDigits, pCallingTelNumber);

            TRACEINTO << "CLobby::OnNetCallIn - Failed, Destination party is full connected (call rejected), lookup_result:" << lookup_result << ", party_status:" << party_status;

            // unattented call handle ...
            RejectIsdnCall(pNetSetup.get(), 1);
            return;
          }
        }
        AcceptCallIn(pNetSetup.get(), pComConf, pConfParty);
      }
      break;
    }

    case 2:
    {
      // meeting room or GW
      PASSERTMSG((0xFFFFFFFF == confId), "CLobby::OnNetCallIn - ConfId is 0xFFFFFFFF, core protection in case of TransitEQ mismatch");

      CCommRes* pMrRes = ::GetpMeetingRoomDB()->GetCurrentRsrv(confId);

	  if(pMrRes && (pMrRes->isIvrProviderEQ() || pMrRes->isExternalIvrControl())) //AT&T
	  {
		  POBJDELETE(pMrRes);
		  TRACEINTO << "CLobby::OnNetCallIn - Failed, No IvrProviderEQ/ExternalIvrControl is allowed in ISDN/PSTN calls (call rejected)";
		  RejectIsdnCall(pNetSetup.get(), 1);
		  break;
	  }

      ALLOCBUFFER(phoneNumberForward, PHONE_NUMBER_DIGITS_LEN);
      if (pMrRes && pMrRes->GetIsGateway() && pMrRes->IsEnableIsdnPstnAccess()) // Daimler
      {
        CServicePhoneStr* pServicePhone = pMrRes->GetFirstServicePhone();
        if (pServicePhone && pServicePhone->IsUseServicePhonesAsRange())
        {
          const char* prefixForward    = pServicePhone->GetPhonePrefixForward();
          WORD        numDigitsForward = pServicePhone->GetPhoneNumDigitsForward();

          strncpy(phoneNumberForward, prefixForward, SERVICE_PHONE_PREFIX_LEN);
          WORD        prefixForwardLen = strlen(phoneNumberForward);
          WORD        nTelNumberLen    = setupIndStruct.called_party.num_digits;

          ALLOCBUFFER(pTelNumber, PHONE_NUMBER_DIGITS_LEN);
          strncpy(pTelNumber, (const char*)setupIndStruct.called_party.digits, nTelNumberLen);
          BYTE        cutNumDigits = (numDigitsForward > nTelNumberLen) ? 0 : (nTelNumberLen - numDigitsForward);
          strcat(phoneNumberForward, pTelNumber+cutNumDigits);

          WORD        numDigits = setupIndStruct.called_party.num_digits - cutNumDigits + prefixForwardLen;

          TRACEINTO << "CLobby::OnNetCallIn - MR or GW, use service phones as range, phoneNumberForward:" << phoneNumberForward << ", numDigits:" << numDigits;
          DEALLOCBUFFER(pTelNumber);
        }
      }

      if (IsVoiceCall(pNetSetup.get())) // PSTN
      {
        if (YES == TargetReceptionAlreadyExist(meetingRoomConfName))
          SuspendVoiceCall(pNetSetup.get(), meetingRoomConfName);
        else if (pMrRes && (pMrRes->GetIsGateway() || strcmp(meetingRoomConfName, "GateWayTest") == 0))
          SuspendVoiceCallAndCreateGateWayConf(pNetSetup.get(), confId, meetingRoomConfName, phoneNumberForward);
        else
          SuspendVoiceCallAndCreateMeetingRoom(pNetSetup.get(), confId, meetingRoomConfName);
      }
      else                        // ISDN
      {
        if (YES == TargetReceptionAlreadyExist(meetingRoomConfName))
          SuspendIsdnCall(pNetSetup.get(), meetingRoomConfName);
        else
        {
          if (pMrRes && (pMrRes->GetIsGateway() || strcmp(meetingRoomConfName, "GateWayTest") == 0))
            SuspendIsdnCallAndCreateGateWayConf(pNetSetup.get(), confId, meetingRoomConfName, phoneNumberForward);
          else
            SuspendIsdnCallAndCreateMeetingRoom(pNetSetup.get(), confId, meetingRoomConfName);
        }
      }
      DEALLOCBUFFER(phoneNumberForward);
      POBJDELETE(pMrRes);
      break;
    }

    case 1:
    { // CONFERENCE FOUND BUT PARTY NOT FOUND (UNRESERVED PARTY)
      SuspendIsdnCallConfExist(pNetSetup.get(), pComConf->GetMonitorConfId(), (char*)pComConf->GetName(), pCalledTelNumber, pCallingTelNumber);
      break;
    }

    default:
    {
      TRACEINTO << "CLobby::OnNetCallIn - Failed, Unknow lookup value (call rejected)";
      RejectIsdnCall(pNetSetup.get(), 1);
      break;
    }
  } // switch
}

//--------------------------------------------------------------------------
void CLobby::OnNetDisconnect(CSegment* pMplParam)
{
  NET_DISCONNECT_IND_S netDisInd;
  memset(&netDisInd, 0, sizeof(NET_DISCONNECT_IND_S));

  pMplParam->Get((BYTE*)(&netDisInd), sizeof(NET_DISCONNECT_IND_S));

  BYTE box_id = 0, boardId = 0, sub_board_id = 0;
  (*pMplParam) >> box_id;
  (*pMplParam) >> boardId;              // Getting the boardId
  (*pMplParam) >> sub_board_id;

  BYTE cause = netDisInd.cause.cause_val;
  TRACEINTO << "CLobby::OnNetDisconnect - Reason:" << int(cause);

  CIsdnNetSetup* pNetSetup = new CIsdnNetSetup;

  // Set the header params
  pNetSetup->SetNetCommnHeaderParams(netDisInd.net_common_header);

  pNetSetup->m_boardId      = boardId;  // Set the boardId
  pNetSetup->m_box_id       = box_id;
  pNetSetup->m_sub_board_id = sub_board_id;

  // Return Answer to the NET
  CReceptionVoice* pReception = new CReceptionVoice(this);
  pReception->ReplayToNet(NET_DISCONNECT_ACK_REQ, *pNetSetup);
  pReception->Destroy(false);

  POBJDELETE(pReception);
  POBJDELETE(pNetSetup);
}

//--------------------------------------------------------------------------
void CLobby::SuspendVoiceCall(CIsdnNetSetup* pNetSetup, char* confName)
{
  PASSERT_AND_RETURN(!pNetSetup);

  TRACEINTO << "CLobby::SuspendVoiceCall - ConfName:" << DUMPSTR(confName);

  DWORD counter = (DWORD)m_pParyList_Id;
  if (counter == 0xffff)
    counter = 0x0001;
  else
    counter++;

  m_pParyList_Id = (CTaskApp*)counter;  // advance party list id number

  // allocate a Suspended Reception
  CReceptionVoice* pLobbyTrans = new CReceptionVoice(this);
  pLobbyTrans->CreateSuspend(pNetSetup, m_pRcvMbx, this, m_pParyList_Id, 0, confName);
  m_pTransferList->Insert(pLobbyTrans);
}

//--------------------------------------------------------------------------
void CLobby::SuspendIsdnCall(CIsdnNetSetup* pNetSetup, char* confName)
{
  PASSERT_AND_RETURN(!pNetSetup);

  TRACEINTO << "CLobby::SuspendIsdnCall - ConfName:" << DUMPSTR(confName);

  DWORD counter = (DWORD)m_pParyList_Id;
  if (counter == 0xffff)
    counter = 0x0001;
  else
    counter++;

  m_pParyList_Id = (CTaskApp*)counter;  // advance party list id number

  // allocate a Suspended Reception
  CReceptionIsdn* pLobbyTrans = new CReceptionIsdn(this);
  pLobbyTrans->CreateSuspend(pNetSetup, m_pRcvMbx, this, m_pParyList_Id, 0, confName);
  m_pTransferList->Insert(pLobbyTrans);
}

//--------------------------------------------------------------------------
void CLobby::SuspendVoiceCallAndCreateMeetingRoom(CNetSetup* pNetSetup, DWORD confId, char* mrName)
{
	PASSERT_AND_RETURN(!pNetSetup);

	TRACEINTO << "CLobby::SuspendVoiceCallAndCreateMeetingRoom - ConfId:" << confId << ", ConfName:" << DUMPSTR(mrName);

	// Fix for VNGR-24738 - OLEG
	SetPartyListIDForSuspendCall();
	// allocate a Suspended Reception
	CReceptionVoice* pLobbyTrans = new CReceptionVoice(this);
	pLobbyTrans->CreateMeetingRoom(pNetSetup, m_pRcvMbx, this, m_pParyList_Id, confId, mrName);
	m_pTransferList->Insert(pLobbyTrans);
}

//--------------------------------------------------------------------------
void CLobby::SuspendIsdnCallAndCreateMeetingRoom(CNetSetup* pNetSetup, DWORD confId, char* mrName)
{
	PASSERT_AND_RETURN(!pNetSetup);

	TRACEINTO << "CLobby::SuspendIsdnCallAndCreateMeetingRoom - ConfId:" << confId << ", ConfName:" << DUMPSTR(mrName);

	// Fix for VNGR-24738 - OLEG
	SetPartyListIDForSuspendCall();
	// allocate a Suspended Reception
	CReceptionIsdn* pLobbyTrans = new CReceptionIsdn(this);
	pLobbyTrans->CreateMeetingRoom(pNetSetup, m_pRcvMbx, this, m_pParyList_Id, confId, mrName);
	m_pTransferList->Insert(pLobbyTrans);
}

//--------------------------------------------------------------------------
void CLobby::SuspendVoiceCallAndCreateGateWayConf(CNetSetup* pNetSetup, DWORD confId, char* mrName, char* targetNumber)
{
	PASSERT_AND_RETURN(!pNetSetup);

	TRACEINTO << "GATEWAY_LOG: ConfId:" << confId << ", ConfName:" << DUMPSTR(mrName) << ", targetNumber:" << targetNumber;

	char targetGWConfName[H243_NAME_LEN+8];
	snprintf(targetGWConfName, ARRAYSIZE(targetGWConfName), "GW_%s(%03d)", mrName, gw_index++);

	// Fix for VNGR-24738 - OLEG
	SetPartyListIDForSuspendCall();
	// allocate a Suspended Reception
	CReceptionVoice* pLobbyTrans = new CReceptionVoice(this);
	pLobbyTrans->CreateGateWayConf(pNetSetup, m_pRcvMbx, this, m_pParyList_Id, confId, targetGWConfName, targetNumber);
	//TRACEINTO << "CLobby::SuspendVoiceCallAndCreateGateWayConf: pLobbyTrans->GetPartyId() = " << pLobbyTrans->GetPartyId();
	m_pTransferList->Insert(pLobbyTrans);
}

//--------------------------------------------------------------------------
void CLobby::SuspendIsdnCallAndCreateGateWayConf(CNetSetup* pNetSetup, DWORD confId, char* mrName, char* targetNumber)
{
	PASSERT_AND_RETURN(!pNetSetup);

	TRACEINTO << "GATEWAY_LOG: ConfId:" << confId << ", ConfName:" << DUMPSTR(mrName) << ", targetNumber:" << targetNumber;

	char targetGWConfName[H243_NAME_LEN+8];
	snprintf(targetGWConfName, ARRAYSIZE(targetGWConfName), "GW_%s(%03d)", mrName, gw_index++);

	// Fix for VNGR-24738 - OLEG
	SetPartyListIDForSuspendCall();
	// allocate a Suspended Reception
	CReceptionIsdn* pLobbyTrans = new CReceptionIsdn(this);
	pLobbyTrans->CreateGateWayConf(pNetSetup, m_pRcvMbx, this, m_pParyList_Id, confId, targetGWConfName, targetNumber);
	m_pTransferList->Insert(pLobbyTrans);
}

//--------------------------------------------------------------------------
IdentificationStatus CLobby::IdentifyParty(CIsdnNetSetup* pNetSetup, CCommConf* pComConf, CConfParty* pConfParty, COsQueue* pConfRcvMbx, COsQueue* pLobbyRcvMbx, const char* confName, DWORD monitorConfId)
{
  PASSERT_AND_RETURN_VALUE(!m_pTransferList, Status_InvalidArguments);
  PASSERT_AND_RETURN_VALUE(!pConfParty, Status_InvalidArguments);

  TRACEINTO << "CLobby::IdentifyParty - PartyId:" << pConfParty->GetPartyId() << ", PartyName:" << pConfParty->GetName();

  CReception* pLobbyTrans = NULL;
  if (pConfParty->GetVoice())
    pLobbyTrans = new CReceptionVoice(this);
  else
    pLobbyTrans = new CReceptionIsdn(this);

  char partyName[H243_NAME_LEN];
  SAFE_COPY(partyName, pConfParty->GetName());

  pLobbyTrans->Create(monitorConfId, pConfParty->GetPartyId(), pConfRcvMbx, pLobbyRcvMbx, pNetSetup, this, partyName, confName, pConfParty, pComConf);
  m_pTransferList->Insert(pLobbyTrans);
  return Status_OK;
}

//--------------------------------------------------------------------------
void CLobby::OnGkManagerSearchForCall(CSegment* pParam)
{
  char destAlias[MaxAddressListSize] = { 0 };
  char srcAlias [MaxAddressListSize] = { 0 };

  DWORD hsRas = 0, serviceId = 0;
  *pParam >> destAlias >> srcAlias >> hsRas >> serviceId;

  TRACEINTO << "CLobby::OnGkManagerSearchForCall - destAlias:" << destAlias << ", srcAlias:" << srcAlias;

  CH323Alias* pPartyDestAliasList = new CH323Alias;
  pPartyDestAliasList->SetAliasName(destAlias);

  CH323Alias* pPartySrcAliasList  = new CH323Alias;
  pPartySrcAliasList->SetAliasName(srcAlias);

  mcTransportAddress sourcePartyIp;
  memset(&sourcePartyIp, 0, sizeof(mcTransportAddress));
  sourcePartyIp.ipVersion = eIpVersion4;

  DWORD       meetingRoomId  = 0xFFFFFFFF;
  CCommConf*  pComConf       = NULL;
  CConfParty* pConfParty     = NULL;
  int         numOfSrcAlias  = 1;
  int         numOfDestAlias = 1;

  int i = GetIpConfPartyType(((const mcTransportAddress*)&sourcePartyIp), pPartySrcAliasList, numOfSrcAlias, (CCommConf**)&pComConf, &pConfParty, pPartyDestAliasList, numOfDestAlias, meetingRoomId);
  BYTE resultToGkManager = (i != -1) ? 1 : 0;

  PDELETE(pPartySrcAliasList);
  PDELETE(pPartyDestAliasList);

  CProcessBase* pProcess = CProcessBase::GetProcess();
  PASSERT_AND_RETURN(!pProcess);

  CSegment* pSeg = new CSegment;
  *pSeg << resultToGkManager << serviceId << hsRas;

  CGatekeeperTaskApi api(serviceId);
  STATUS res = api.SendMsg(pSeg, LOBBY_LRQ_RESPONSE);
  PASSERT(res != STATUS_OK);
}

//--------------------------------------------------------------------------
void CLobby::SuspendH323CallAndCreateGateWayConf(CH323NetSetup* pNetSetup, DWORD confId, char* strSrcPartyAddress, char* strDstPartyAddress, char* mrName, char* TargetNumber)
{
	SetPartyListIDForSuspendCall(strSrcPartyAddress, strDstPartyAddress);

	TRACEINTO << "GATEWAY_LOG: CLobby::SuspendH323CallAndCreateGateWayConf - Activating GateWay conference "
			<< "- CalleeIP:"           << DUMPSTR(strDstPartyAddress)
			<< ", CallerIP:"           << DUMPSTR(strSrcPartyAddress)
			<< ", ISDN target number:" << TargetNumber;

	char targetGWConfName[H243_NAME_LEN+8];
	snprintf(targetGWConfName, ARRAYSIZE(targetGWConfName), "GW_%s(%03d)", mrName, gw_index++);

	// allocate a Suspended Reception
	CReceptionH323* pLobbyTrans = new CReceptionH323(this);
	pLobbyTrans->CreateGateWayConf(pNetSetup, m_pRcvMbx, this, m_pParyList_Id, confId, targetGWConfName, TargetNumber); // , PrefixNumber,GateWayConfrenceType, numberWithPrefix);
	m_pTransferList->Insert(pLobbyTrans);
}
//--------------------------------------------------------------------------
void CLobby::OnSipMsConfInvite(CSegment* pCSParam)
{
	 PTRACE(eLevelInfoNormal, "CLobby::OnSipMsConfInvite");
	 // 1. build sip invite C++ instance, which has functions to help retrieve fields such as SIP headers.
	  CSipInviteStruct *pSipInvite = new CSipInviteStruct; AUTO_DELETE(pSipInvite);
	  STATUS result;

	  result = BuildSipInviteStruct(pCSParam, pSipInvite);

	  if (result != STATUS_OK)
	  {
	    POBJDELETE(pSipInvite);
	    return;
	  }

	  // 2. build sip netsetup from sip invite
	   CSipNetSetup *pNetSetup = new CSipNetSetup;

	   BuildSipNetSetupForMSConfInvite(pSipInvite, pNetSetup);

	   // 3. decide on call flow, whether to accept or reject or suspend
	     CallFlowDecision(pSipInvite, pNetSetup);
}
//--------------------------------------------------------------------------
void CLobby::OnSipCallIn(CSegment* pCSParam)
{
  // 1. build sip invite C++ instance, which has functions to help retrieve fields such as SIP headers.
  CSipInviteStruct *pSipInvite = new CSipInviteStruct;
  STATUS result;

  result = BuildSipInviteStruct(pCSParam, pSipInvite);

  if (result != STATUS_OK)
  {
	TRACEINTO << "result != STATUS_OK"; // BRIDGE-18254
    POBJDELETE(pSipInvite);
    return;
  }

  // 2. build sip netsetup from sip invite
  CSipNetSetup *pNetSetup = new CSipNetSetup;

  BuildSipNetSetup(pSipInvite, pNetSetup);

  // 3. decide on call flow, whether to accept or reject or suspend
  CallFlowDecision(pSipInvite, pNetSetup);

  POBJDELETE(pNetSetup);
  POBJDELETE(pSipInvite);
}

//--------------------------------------------------------------------------
void CLobby::ParseSipHost(const char* strAddr, CH323Alias* PartyAliasList, CSipNetSetup* pNetSetup)
{
  if (strAddr && PartyAliasList && pNetSetup)
  {
    char strUser[H243_NAME_LEN] = { 0 };
    char strHost[H243_NAME_LEN] = { 0 };

    int readed = sscanf(strAddr, "%80[^@]@%80[^;]", strUser, strHost);

    if (readed == 2)                 // uri address
    {
      pNetSetup->SetSrcPartyAddress(strHost);
      PartyAliasList->SetAliasName(strAddr);
      PartyAliasList->SetAliasType(PARTY_H323_ALIAS_URL_ID_TYPE);
    }
    else // not uri
    {
      PartyAliasList->SetAliasName(strUser);
    }
  }
}

//--------------------------------------------------------------------------
void CLobby::ParseEqFormat(char *strUser, CH323Alias* PartyAliasList, CSipNetSetup* pNetSetup, char** ppAdHocConfName, char** ppAdHocNID)
{
  // VNGFE-4657
  if (ppAdHocConfName != NULL && ppAdHocNID != NULL)
  {
    // use EQ(confName)(NID)
    int nClose = 0, nOpen = 0, len = 0;
    len = strlen(strUser)-1;
    if (len >= 0 && strUser[len] == ')')
    {
      // VNGFE-4657
      PDELETEA(*ppAdHocConfName);
      PDELETEA(*ppAdHocNID);

      *ppAdHocConfName = new char[H243_NAME_LEN];
      *ppAdHocNID      = new char[H243_NAME_LEN];

      int i = len;
      for (; i >= 0; i--)
      {
        if (strUser[i] == ')')
          nClose++;

        if (strUser[i] == '(')
        {
          nOpen++;
          if (nClose == 1 && nOpen == 1)
          {
            // copy NID
            strncpy(*ppAdHocNID, &strUser[i+1], len - i-1);
            (*ppAdHocNID)[len-i-1] = '\0'; // VNGFE-4657
            if (strcmp(*ppAdHocNID, ""))
            {
              // check that NID is digits only
              int n = 0;
              while (TRUE)
              {
                if ((*ppAdHocNID)[n] == '\0')
                  break;

                if (!isdigit((*ppAdHocNID)[n]))
                  break;

                n++;
              }

              // if found char that is not a digit
              if ((*ppAdHocNID)[n] != '\0')
                break;
            }

            // if the next char is not ')' this is not (confName)(NID)
            if (strUser[i-1] != ')')
              break;

            len = i-1;
          }
          else
          {
            // copy ConfName
            if (nClose == nOpen)
            {
              strncpy(*ppAdHocConfName, &strUser[i+1], len - i-1);
              strUser[i] = '\0';
              PartyAliasList->SetAliasName(strUser);
              i       = 0; // end the for loop
            }
          }
        }

        if ((nClose - nOpen >= 2 && nOpen == 0) || (nOpen > nClose))
          break;
      }

      // if failed to parse
      if (i >= 0)
      {
        pNetSetup->SetDestUserName(strUser);
        PDELETEA(*ppAdHocConfName);//VNGFE-4657
        PDELETEA(*ppAdHocNID);     //VNGFE-4657
      }
    } // if(user[len]==')')
  } // if(ppAdHocConfName != NULL && ppAdHocNID != NULL)
}

//--------------------------------------------------------------------------
void CLobby::ParseSipDest(const char* strAddr, CH323Alias* PartyAliasList, CSipNetSetup* pNetSetup, char** ppAdHocConfName, char** ppAdHocNID)
{
  if (strAddr && PartyAliasList && pNetSetup)
  {
    char strUser[H243_NAME_LEN] = { 0 };
    char strHost[H243_NAME_LEN] = { 0 };

    int readed = sscanf(strAddr, "%80[^@]@%80[^;]", strUser, strHost);

    if (readed == 2)                 // uri address
    {
      pNetSetup->SetDestPartyAddress(strHost);
      PartyAliasList->SetAliasName(strUser);
      PartyAliasList->SetAliasType(PARTY_H323_ALIAS_H323_ID_TYPE);
      // VNGFE-4657
      ParseEqFormat(strUser, PartyAliasList, pNetSetup, ppAdHocConfName, ppAdHocNID);
    }
    else // not uri
    {
      PartyAliasList->SetAliasName(strUser);
    }
  }
}



//--------------------------------------------------------------------------
void CLobby::ParseSipUri(eAddressType eType, const char* strAddr, findConfAndPartyKeysSt &keys, CSipNetSetup* pNetSetup)
{
  // Parse Source data
  if (eType == kSrc)
  {
    ParseSipHost(strAddr, keys.pPartySrcAliasList, pNetSetup);
	SetAliasDisplayName(keys, pNetSetup->GetRemoteDisplayName());
  }
  // Parse Destination data
  else if (eType == kDst)
  {
    ParseSipDest(strAddr, keys.pPartyDstAliasList, pNetSetup, &keys.pAdHocConfName, &keys.pAdHocNID);
  } // Dest Uri
  else if (eType == kDst2)
  {
    ParseSipDest(strAddr, keys.pPartyDstAliasList + 1, pNetSetup, NULL, NULL);
  } // Dest Uri
  else if (eType == kDstReqUri)
  {
    ParseSipDest(strAddr, keys.pPartyDstAliasList + keys.numOfDestAlias, pNetSetup, NULL, NULL);
  } // Dest Uri
  else if (eType == kDstReqUri2)
  {
    ParseSipDest(strAddr, keys.pPartyDstAliasList + keys.numOfDestAlias + 1, pNetSetup, NULL, NULL);
  } // Dest Uri
}

/*
 Parsing the following phone-context format for local numbering, OCS with PSTN configuration support it.
 See: rfc4504.html, rfc3966.txt
 Examples:
 sip:5551234;phone-context=1212@example.net;user=phone ==> (DestPartyAddress1 = "1212@example.net", DestPartyAddress2= "5551234")
 sips:5551234;phone-context=+1212@example.net;user=phone  ==> (DestPartyAddress1 = "+1212@example.net", DestPartyAddress2= "5551234")
 sips:77777777;phone-context=10.1.1.1@example.net ==> (DestPartyAddress1 = "10.1.1.1@example.net", DestPartyAddress2= "77777777")
 tel:6767676;phone-context=+1212 ==> (DestPartyAddress1 = "+1212", DestPartyAddress2= "6767676") , Need to enable all TelUris in the stack.
*/
//--------------------------------------------------------------------------
int CLobby::ParseSipToPhContextUri(const char* strTo, int* pNumOfDestAliases, char* pDestPartyAddress1, int sizeDestPartyAddress1, char* pDestPartyAddress2, int sizeDestPartyAddress2)
{
  char sipAlias1[IP_LIMIT_ADDRESS_CHAR_LEN] = { 0 };
  char sipAlias2[IP_LIMIT_ADDRESS_CHAR_LEN] = { 0 };

  if (!strTo || !pNumOfDestAliases || !pDestPartyAddress1 || !pDestPartyAddress2)
  {
    strcpy_safe(pDestPartyAddress1, sizeDestPartyAddress1, strTo);

    return STATUS_FAIL;
  }

  int num = sscanf(strTo, "%255[^;] ; phone-context=%255[^;]", sipAlias1, sipAlias2);

  if (num == 2)
  {
    // fix bug 19.08.10 due to fact that now the To: field is being parsed and not the  Req Line: (part of ice)
    strncpy(pDestPartyAddress1, sipAlias1, sizeDestPartyAddress1-1), pDestPartyAddress1[sizeDestPartyAddress1-1] = '\0';
    strncpy(pDestPartyAddress2, sipAlias2, sizeDestPartyAddress2-1), pDestPartyAddress2[sizeDestPartyAddress2-1] = '\0';
    *pNumOfDestAliases = num;
    return STATUS_OK;
  }

  strcpy_safe(pDestPartyAddress1, sizeDestPartyAddress1, strTo);

  return STATUS_FAIL;
}

//--------------------------------------------------------------------------
void CLobby::AcceptSipCallIn(CSipNetSetup* pNetSetup, CCommConf* pComConf, CConfParty* pConfParty, BYTE bIsSoftCp, const sipSdpAndHeadersSt* pSdpAndHeaders)
{
  PASSERT_AND_RETURN(!pNetSetup);
  PASSERT_AND_RETURN(!pComConf);
  PASSERT_AND_RETURN(!pConfParty);

  COsQueue*   pConfRcvMbx   = pComConf->GetRcvMbx();
  const char* confName      = pComConf->GetName();
  DWORD       confMonitorId = pComConf->GetMonitorConfId();

  TRACEINTO << "confMonitorId:"      << confMonitorId
  	        << ", partyMonitorId:"   << pConfParty->GetPartyId()
            << ", PartyName:" << pConfParty->GetName()
            << ", EnableICE:" << (int)pConfParty->GetEnableICE();

  // set net setup parameters
  pNetSetup->SetConfId(confMonitorId);

  BYTE        bPreDefinedIvrStrExist = FALSE;
  const char* AliasName = pNetSetup->GetLocalSipAddress();
  if (AliasName)
  {
	  char PreDefinedString[H243_NAME_LEN] = { 0 };
	  char strUri[H243_NAME_LEN] = { 0 };
	  SAFE_COPY(strUri, AliasName);

	  char* temp = strstr(strUri, ";");
	  if (temp)
		  strUri[temp-strUri] = '\0';
	  temp = strstr(strUri, "@");
	  if (temp)// uri address
		strUri[temp-strUri] = '\0';
	  temp = strstr(strUri, "**");
	  if (temp)
	  {
		  SAFE_COPY(PreDefinedString, temp);
		  PreDefinedString[1] = '#'; //compatible with H323 passwords
		  pConfParty->SetPreDefinedIvrString(PreDefinedString + 1);
		  bPreDefinedIvrStrExist = TRUE;
		  TRACEINTO << "Received SIP URI conf password: " << (PreDefinedString + 2);
	  }
  }

  if (!bPreDefinedIvrStrExist)
  {
	  const char* netSetupPredefinedIvr = pNetSetup->GetPredefiendIvrStr();   // for the ENABLE_DEFAULT_ADHOC_CALLS
	  if (strlen(netSetupPredefinedIvr))
		  pConfParty->SetPreDefinedIvrString(netSetupPredefinedIvr);
  }

  pNetSetup->SetEnableSipICE(pConfParty->GetEnableICE());

  // IT'S NOT USED YET IN THE SYSTEM
  WORD isChairEnabled = 1;
  BYTE Meet_me_method = MEET_ME_PER_USER;

  switch (Meet_me_method)
  {
    case MEET_ME_PER_MCU:
      PASSERTMSG(1, "MEET_ME_PER_MCU not supported yet");
      break;

    case MEET_ME_PER_CONFERENCE:
      PASSERTMSG(1, "MEET_ME_PER_CONFERENCE not supported yet");
      break;

    case MEET_ME_PER_USER:
    	TRACEINTO << "IP VERSION: " << (DWORD)pNetSetup->GetIpVersion();
      IdentifySipParty(pNetSetup, pComConf, pConfParty, pConfRcvMbx, m_pRcvMbx, confName, confMonitorId, isChairEnabled, bIsSoftCp, pSdpAndHeaders);
      break;

    default:    // REJECT THE CALL UNKOWN MEET ME METHOD
      // unattented call handle ...
      RejectSipCall(pNetSetup, int(cmReasonTypeNoPermision), pSdpAndHeaders);
      break;
  }
}

//--------------------------------------------------------------------------
void CLobby::RejectSipCall(CSipNetSetup* pNetSetup, int reason, const sipSdpAndHeadersSt* pSdpAndHeaders, char* pAltAddress, STATUS status)
{
  PASSERT_AND_RETURN(!pNetSetup);

  if (pAltAddress)
    TRACEINTO << "CLobby::RejectSipCall - Call rejected, CalleeIP:" << DUMPSTR(pNetSetup->GetSrcPartyAddress()) << ", Redirected to:" << pAltAddress;
  else
    TRACEINTO << "CLobby::RejectSipCall - Call rejected, CalleeIP:" << DUMPSTR(pNetSetup->GetSrcPartyAddress());

  CReceptionSip* pLobbyTrans = new CReceptionSip(this);
  pLobbyTrans->CreateRejectSip(pNetSetup, m_pRcvMbx, this, reason, pSdpAndHeaders, pAltAddress, status);
  pLobbyTrans->Destroy(false);
  POBJDELETE(pLobbyTrans);
}

//--------------------------------------------------------------------------
void CLobby::IdentifySipParty(CSipNetSetup* pNetSetup, CCommConf* pComConf, CConfParty* pConfParty, COsQueue* pConfRcvMbx, COsQueue* pLobbyRcvMbx, const char* confName, DWORD confMonitorId, WORD isChairEnabled, BYTE bIsSoftCp, const sipSdpAndHeadersSt* pSdpAndHeaders)
{
  PASSERT_AND_RETURN(!pNetSetup);
  PASSERT_AND_RETURN(!pComConf);
  PASSERT_AND_RETURN(!m_pTransferList);

  // Checking conditions of channel width not include in our process

  CReceptionSip* pLobbyTrans = new CReceptionSip(this);
  PASSERT_AND_RETURN(!pLobbyTrans);

  char name[H243_NAME_LEN];
  SAFE_COPY(name, pConfParty->GetName());

  m_pTransferList->Insert(pLobbyTrans);

  TRACEINTO << "IP VERSION: " << (DWORD)pNetSetup->GetIpVersion();

  // Pass allocated sip descriptor to the reception dial in party
  pLobbyTrans->CreateSip(confMonitorId, pConfParty->GetPartyId(), pConfRcvMbx, pLobbyRcvMbx, pNetSetup, this, name, confName, pConfParty, pComConf, isChairEnabled, bIsSoftCp, pSdpAndHeaders);
}

//--------------------------------------------------------------------------
void CLobby::SuspendSipCall(CSipNetSetup* pNetSetup, const sipSdpAndHeadersSt* pSdpAndHeaders, DWORD confId, const char* strSrcPartyAddress, const char* strDstPartyAddress, char* confName)
{
  PASSERT_AND_RETURN(!pNetSetup);

  // put incoming call information into a holding queue
  // the call will be suspend until a release message is received
  // from an internal lobby timer or an external message

  DWORD counter = (DWORD) m_pParyList_Id;
  if (counter == 0xffff)
    counter = 0x0001;
  else
    counter++;

  m_pParyList_Id = (CTaskApp*)counter;  // advance party list id number

  TRACEINTO << "CLobby::SuspendSipCall - Incoming call suspended"
          << ", CallId:"   << (DWORD)m_pParyList_Id
          << ", CalleeIP:" << DUMPSTR(strDstPartyAddress)
          << ", CallerIP:" << DUMPSTR(strSrcPartyAddress)
          << ", ConfId:"   << (DWORD)confId;

  // allocate a Suspended Reception
  CReceptionSip* pLobbyTrans = new CReceptionSip(this);
  pLobbyTrans->CreateSuspend(pNetSetup, pSdpAndHeaders, m_pRcvMbx, this, m_pParyList_Id, confId, confName);
  m_pTransferList->Insert(pLobbyTrans);
}

//--------------------------------------------------------------------------
void CLobby::SuspendSipCallConfExist(CSipNetSetup* pNetSetup, const sipSdpAndHeadersSt* pSdpAndHeaders, DWORD ConfId, const char* strSrcPartyAddress, const char* strDstPartyAddress, char* confName)
{
  PASSERT_AND_RETURN(!pNetSetup);

  SetPartyListIDForSuspendCall(strSrcPartyAddress, strDstPartyAddress);

  // allocate a Suspended Reception
  CReceptionSip* pLobbyTrans =  new CReceptionSip(this);
  pLobbyTrans->CreateSuspendConfExist(pNetSetup, pSdpAndHeaders, m_pRcvMbx, this, m_pParyList_Id, ConfId, confName);
  m_pTransferList->Insert(pLobbyTrans);
}

//--------------------------------------------------------------------------
void CLobby::SuspendSipCallAndCreateGateWayConf(CSipNetSetup* pNetSetup, const sipSdpAndHeadersSt* pSdpAndHeaders, DWORD confId, const char* strSrcPartyAddress, const char* strDstPartyAddress, char* mrName, char* dialString)
{
	PASSERT_AND_RETURN(!pNetSetup);

	SetPartyListIDForSuspendCall(strSrcPartyAddress, strDstPartyAddress);

	char targetGWConfName[H243_NAME_LEN+8];
	snprintf(targetGWConfName, ARRAYSIZE(targetGWConfName), "GW_%s(%03d)", mrName, gw_index++);


	TRACEINTO << "GATEWAY_LOG: CLobby::SuspendSipCallAndCreateGateWayConf - Activating GateWay conference"
			<< ", CalleeIP:"           << DUMPSTR(strDstPartyAddress)
			<< ", CallerIP:"           << DUMPSTR(strSrcPartyAddress)
			<< ", ISDN target number:" << dialString
			<< ", Target ConfName:"    << targetGWConfName;

	// allocate a Suspended Reception
	CReceptionSip* pLobbyTrans = new CReceptionSip(this);
	pLobbyTrans->CreateGateWayConf(pNetSetup, pSdpAndHeaders, m_pRcvMbx, this, m_pParyList_Id, confId, targetGWConfName, dialString);
	m_pTransferList->Insert(pLobbyTrans);
}

//--------------------------------------------------------------------------
void CLobby::SuspendSipCallAndCreateMeetingRoom(CSipNetSetup* pNetSetup, const sipSdpAndHeadersSt* pSdpAndHeaders, DWORD confId, const char* strSrcPartyAddress, const char* strDstPartyAddress, char* mrName)
{
  // Save incomming call information in a holding queue Reception List
  // the call will be susupened until a release message is recived
  // from an internal lobby timer or an external message
  PASSERT_AND_RETURN(!pNetSetup);

  DWORD counter = (DWORD) m_pParyList_Id;
  if (counter == 0xffff)
    counter = 0x0001;
  else
    counter++;

  m_pParyList_Id = (CTaskApp*)counter; // advance party list id number

  TRACEINTO << "CLobby::SuspendSipCallAndCreateMeetingRoom - Incoming call suspended"
            << ", CallId:"   << (DWORD)m_pParyList_Id
            << ", CalleeIP:" << DUMPSTR(strDstPartyAddress)
            << ", CallerIP:" << DUMPSTR(strSrcPartyAddress)
            << ", ConfId:"   << (DWORD)confId;

  // allocate a Suspended Reception
  CReceptionSip* pLobbyTrans = new CReceptionSip(this);
  pLobbyTrans->CreateMeetingRoom(pNetSetup, pSdpAndHeaders, m_pRcvMbx, this, m_pParyList_Id, confId, mrName);
  m_pTransferList->Insert(pLobbyTrans);
}

//--------------------------------------------------------------------------
void CLobby::SuspendSipCallAndCreateConfFromFactory(CSipNetSetup* pNetSetup, const char* strSrcPartyAddress, const char* strDstPartyAddress, const sipSdpAndHeadersSt* pSdpAndHeaders, CCommRes* pFactory)
{
	// Save incomming call information in a holding queue Reception List
	// the call will be susupened until a release message is recived
	// from an internal lobby timer or an external message
	PASSERT_AND_RETURN(!pNetSetup);

	DWORD counter = (DWORD) m_pParyList_Id;
	if (counter == 0xffff)
	counter = 0x0001;
	else
	counter++;

	m_pParyList_Id = (CTaskApp*)counter; // advance party list id number

	TRACEINTO << "CLobby::SuspendSipCallAndCreateConfFromFactory - Incoming call suspended"
			<< ", CallId:"   << (DWORD)m_pParyList_Id
			<< ", CalleeIP:" << DUMPSTR(strDstPartyAddress)
			<< ", CallerIP:" << DUMPSTR(strSrcPartyAddress);

	// Fix for VNGR-24738 - OLEG
	SetPartyListIDForSuspendCall();
	// allocate a Suspended Reception
	CReceptionSip* pLobbyTrans = new CReceptionSip(this);
	pLobbyTrans->CreateConfFromFactory(pNetSetup, m_pRcvMbx, this, m_pParyList_Id, pSdpAndHeaders, pFactory);
	m_pTransferList->Insert(pLobbyTrans);
}

//--------------------------------------------------------------------------
void CLobby::OnSipOptions(CSegment* pCSParam)
{
  const char* strFrom        = "";
  const char* strTo          = "";
  APIU32      callIndex      = 0;
  APIU32      channelIndex   = 0;
  APIU32      mcChannelIndex = 0;
  APIU32      status         = 0;
  APIU16      srcUnitId      = 0;
  APIU32      csServiceId    = 0;

  *pCSParam >> callIndex >> channelIndex >> mcChannelIndex >> status >> srcUnitId>> csServiceId;

  DWORD nMsgLen =  pCSParam->GetWrtOffset() - pCSParam->GetRdOffset();
  if (!nMsgLen)
    return;

  BYTE* pMessage = new BYTE[nMsgLen];
  pCSParam->Get(pMessage, nMsgLen);

  mcIndOptions* pOptionsMsg = (mcIndOptions*)new char[nMsgLen];
  memcpy(pOptionsMsg, pMessage, nMsgLen);
  PDELETEA(pMessage);

  std::auto_ptr<CSipHeaderList> pSipHeaderList(new CSipHeaderList(pOptionsMsg->sipHeaders));

  char strSrcPartyAddress[IP_LIMIT_ADDRESS_CHAR_LEN] = { 0 };
  char strDstPartyAddress[IP_LIMIT_ADDRESS_CHAR_LEN] = { 0 };

  const CSipHeader* pFrom = pSipHeaderList->GetNextHeader(kFrom);
  if (pFrom)
    strFrom = pFrom->GetHeaderStr();

  const CSipHeader* pTo = pSipHeaderList->GetNextHeader(kTo);
  if (pTo)
    strTo = pTo->GetHeaderStr();

  SAFE_COPY(strSrcPartyAddress, strFrom);
  SAFE_COPY(strDstPartyAddress, strTo);

  std::auto_ptr<CSipNetSetup> pNetSetup(new CSipNetSetup);

  pNetSetup->SetRemoteSipAddress(strSrcPartyAddress);
  if (strncmp(strSrcPartyAddress, "tel:", 4) == 0)
    pNetSetup->SetRemoteSipAddressType(PARTY_SIP_TELURL_ID_TYPE);
  else
    pNetSetup->SetRemoteSipAddressType(PARTY_SIP_SIPURI_ID_TYPE);

  pNetSetup->SetSrcPartyAddress(strSrcPartyAddress);
  pNetSetup->SetDestPartyAddress(strDstPartyAddress);
  pNetSetup->SetCallIndex(callIndex);
  pNetSetup->SetCsServiceid(csServiceId);

  TRACEINTO << "CLobby::OnSipOptions - Incoming Options "
            << "- CalleeIP:" << DUMPSTR(strDstPartyAddress)
            << ", CallerIP:" << DUMPSTR(strSrcPartyAddress);

  // Find party in Database
  CCommConf*  pComConf       = NULL;
  CConfParty* pConfParty     = NULL;
  DWORD       meetingRoomId  = 0xFFFFFFFF;

  std::auto_ptr<CH323Alias> pPartySrcAliasList(new CH323Alias);
  std::auto_ptr<CH323Alias> pPartyDstAliasList(new CH323Alias);

  findConfAndPartyKeysSt keys;
  keys.numOfSrcAlias = 1;
  keys.numOfDestAlias = 1;
  keys.pAdHocConfName = NULL;
  keys.pAdHocNID = NULL;
  keys.pPartySrcAliasList = pPartySrcAliasList.get();
  keys.pPartyDstAliasList = pPartyDstAliasList.get();

  ParseSipUri(kSrc, strFrom, keys, pNetSetup.get());
  ParseSipUri(kDst, strTo, keys, pNetSetup.get());

  mcTransportAddress dummyAddr;
  memset(&dummyAddr, 0, sizeof(mcTransportAddress));

  int i = GetIpConfPartyType((const mcTransportAddress*)&dummyAddr, keys.pPartySrcAliasList, keys.numOfSrcAlias, (CCommConf**)&pComConf, &pConfParty, keys.pPartyDstAliasList, keys.numOfDestAlias, meetingRoomId, FALSE, keys.pAdHocConfName);

  if (pConfParty && pConfParty->GetNetInterfaceType() != SIP_INTERFACE_TYPE)
  {
    i = -6;
    pConfParty->SetDefinedPartyAssigned(FALSE); // sip call into defined H.323 party
  }

  // Transit EQ - In case a destination conference wasn't found we will try to connect to Transit EQ
  char transitEQName[H243_NAME_LEN] = { 0 };
  SAFE_COPY(transitEQName, ::GetpMeetingRoomDB()->GetTransitEQName());

  if (i == -1 && strcmp(transitEQName, ""))
  {
    TRACEINTO << "CLobby::OnSipOptions - Destination conference wasn't found"
              << ", CalleeIP:" << DUMPSTR(strDstPartyAddress)
              << ", CallerIP:" << DUMPSTR(strSrcPartyAddress)
              << ", We will try to connect to default EQ name:" << transitEQName;

    WORD useTransitEQ = SetNewDestinationConfName(pNetSetup.get(), transitEQName, TRUE);
    if (useTransitEQ == TRUE)
    {
      SAFE_COPY(strDstPartyAddress, pNetSetup->GetDestPartyAddress());
      ParseSipUri(kDst, strDstPartyAddress, keys, pNetSetup.get());
      i = GetIpConfPartyType((const mcTransportAddress*)&dummyAddr, keys.pPartySrcAliasList, keys.numOfSrcAlias, (CCommConf**)&pComConf, &pConfParty, keys.pPartyDstAliasList, keys.numOfDestAlias, meetingRoomId, TRUE, NULL, TRUE);
    }
    else
    {
      TRACEINTO << "CLobby::OnSipOptions - Failed to change the destination address to transit EQ";
    }
  }

  switch (i)
  {
    case 0:
    { // direct Dial In found a reservation to this party in the MCU data base
      // checking if the received party is fully connected.
      // if yes - reject the party
      if (pConfParty)
      {
        const char* tempName = pConfParty->GetName();
        if (tempName)
          TRACEINTO << "CLobby::OnSipOptions - Party found, PartyName:" << tempName;

        DWORD party_status = pConfParty->GetPartyState();
        switch (party_status)
        {
          case PARTY_CONNECTED:
          case PARTY_SECONDARY:
          case PARTY_CONNECTING:
          case PARTY_CONNECTED_WITH_PROBLEM:
          case PARTY_DISCONNECTING:
          {
            DWORD confStat = (pComConf) ? pComConf->GetStatus() : 0;
            if (confStat & CONFERENCE_RESOURCES_DEFICIENCY)
              TRACEINTO << "CLobby::OnSipOptions - Failed, Conference resource deficiency (call rejected) "
                        << "- CalleeIP:" << DUMPSTR(strDstPartyAddress)
                        << ", CallerIP:" << DUMPSTR(strSrcPartyAddress);
            else
              TRACEINTO << "CLobby::OnSipOptions - Failed, Destination party is full connected (call rejected) "
                        << "- CalleeIP:" << DUMPSTR(strDstPartyAddress)
                        << ", CallerIP:" << DUMPSTR(strSrcPartyAddress);

            // unattended call handle ...Mess
            ResponseSipOptions(pOptionsMsg, callIndex, srcUnitId, int(cmReasonTypeNoPermision), NULL, NULL, csServiceId);
            break;
          }

          default:
          {
            if (pConfParty->GetNetInterfaceType() != SIP_INTERFACE_TYPE)
            {
              TRACEINTO << "CLobby::OnSipOptions - Failed, The defined party is not a SIP party (call rejected)";
              ResponseSipOptions(pOptionsMsg, callIndex, srcUnitId, int(cmReasonTypeNoPermision), NULL, NULL, csServiceId);
            }
            else
            {
              ResponseSipOptions(pOptionsMsg, callIndex, srcUnitId, -1, pComConf, pConfParty, csServiceId);
              break;
            }
          }
        } // switch
      }
    }

    case -1:    // REJECT THE CALL UNABLE TO FIND THE CONF AND/OR THE PARTY
    {
      TRACEINTO << "CLobby::OnSipOptions - Incoming call rejected "
                << "- CalleeIP:" << DUMPSTR(strDstPartyAddress)
                << ", CallerIP:" << DUMPSTR(strSrcPartyAddress);

      // unattented call handle ...
      ResponseSipOptions(pOptionsMsg, callIndex, srcUnitId, int(cmReasonTypeNoPermision), NULL, NULL, csServiceId);
      break;
    }


    case -2:    // CONFERENCE FOUND BUT PARTY NOT FOUND (UNRESERVED PARTY)
    {           // unreserved call (returned value at Undefined and Advance Dial InMess)
      TRACEINTO << "CLobby::OnSipOptions - Incoming UnReserved Call "
                << "- CalleeIP:" << DUMPSTR(strDstPartyAddress)
                << ", CallerIP:" << DUMPSTR(strSrcPartyAddress);

      ResponseSipOptions(pOptionsMsg, callIndex, srcUnitId, -1, pComConf, NULL, csServiceId);
      break;
    }

    case -3:  // conference is locked
    {
      TRACEINTO << "CLobby::OnSipOptions - Failed, Conference is locked at Dial-In (call rejected)";
      ResponseSipOptions(pOptionsMsg, callIndex, srcUnitId, int(cmReasonTypeNoPermision), NULL, NULL, csServiceId);
      break;
    }

    case -4:  // max parties number in conference has been reached
    {
      TRACEINTO << "CLobby::OnSipOptions - Failed, Max parties number in conference has been reached at Dial In (call rejected)";
      ResponseSipOptions(pOptionsMsg, callIndex, srcUnitId, int(cmReasonTypeNoBandwidth), NULL, NULL, csServiceId);
      break;
    }

    case -5:  // conference is secured
    {
      TRACEINTO << "CLobby::OnSipOptions - Failed, Conference is secured at Dial-In (call rejected)";
      ResponseSipOptions(pOptionsMsg, callIndex, srcUnitId, int(cmReasonTypeNoPermision), NULL, NULL, csServiceId);
      break;
    }

    case 1:   // found meeting room or GW call
    {
      TRACEINTO << "CLobby::OnSipOptions - Destination MR found (unreserved party)";

      char pMeetingRoomConfName[H243_NAME_LEN] = { 0 };
      if (pPartyDstAliasList.get())
        ::GetpMeetingRoomDB()->GetMeetingRoomForH323Call(keys.pPartyDstAliasList, keys.numOfDestAlias, pMeetingRoomConfName, meetingRoomId, FALSE, FALSE);

      CCommRes*  pCommRes  = ::GetpMeetingRoomDB()->GetCurrentRsrv(meetingRoomId);
      CCommConf* pCommConf = NULL;
      if (pCommRes)
        pCommConf = new CCommConf(*pCommRes);

      ResponseSipOptions(pOptionsMsg, callIndex, srcUnitId, -1, pCommConf, NULL, csServiceId);
      POBJDELETE(pCommConf);
      POBJDELETE(pCommRes);
      break;
    }

    case 2: // if call is to SIP factory
    {
      TRACEINTO << "CLobby::OnSipOptions - SIP Factory call";
      const CCommRes* pFactory = ::GetpMeetingRoomDB()->GetCurrentRsrv(keys.pPartyDstAliasList->GetAliasName());
      if (!IsValidPObjectPtr(pFactory))
        break;

      CCommConf* pCommConf = new CCommConf(*pFactory);

      ResponseSipOptions(pOptionsMsg, callIndex, srcUnitId, -1, pCommConf, NULL, csServiceId);
      POBJDELETE(pFactory);
      POBJDELETE(pCommConf);
    }

    case 20:    // Not a SIP service error
    {
      break;
    }

    default:    // REJECT THE CALL UNABLE TO FIND THE CONF AND/OR THE PARTY
    {
      TRACEINTO << "CLobby::OnSipOptions - Failed, Unspecified error "
                << "- CalleeIP:" << DUMPSTR(strDstPartyAddress)
                << ", CallerIP:" << DUMPSTR(strSrcPartyAddress)
                << ", Error:"    << i;

      // unattended call handle ...
      ResponseSipOptions(pOptionsMsg, callIndex, srcUnitId, int(cmReasonTypeNoPermision), NULL, NULL, csServiceId);
      break;
    }
  } // end switch

  PDELETEA(pOptionsMsg);
  PDELETE(keys.pAdHocConfName);
  PDELETE(keys.pAdHocNID);
}

//--------------------------------------------------------------------------
void CLobby::ResponseSipOptions(mcIndOptions* pOptionsMsg, DWORD callIndex, WORD srcUnitId, int reason, CCommConf* pComConf, CConfParty* pConfParty, DWORD serviceId)
{
  enSipCodes        sipCode       = TranslateLobbyRejectReasonToSip(reason);
  const char*       strAccept     = NULL;
  BYTE              sdpVerified   = FALSE;
  CSipCaps*         pSipLocalCaps = NULL;
  CSipHeaderList*   pTemp         = new CSipHeaderList(pOptionsMsg->sipHeaders);
  const CSipHeader* pAccept       = pTemp->GetNextHeader(kAccept);

  if (pAccept)
    strAccept = pAccept->GetHeaderStr();

  if (strAccept)
  {
    // check if the the accept field holds "application/sdp"
    if (strstr(strAccept, "application/sdp"))
      sdpVerified = TRUE;
  }

  if (-1 == reason && IsValidPObjectPtr(pComConf) && sdpVerified)
  {
    pSipLocalCaps = new CSipCaps;
    CalculateSipOptionsCaps(pComConf, pConfParty, pSipLocalCaps, serviceId);
  }

  SendSipOptionsResponse(pOptionsMsg, sipCode, callIndex, srcUnitId, pSipLocalCaps, serviceId);
  PDELETE(pSipLocalCaps);
  POBJDELETE(pTemp);
}

//--------------------------------------------------------------------------
void CLobby::CalculateSipOptionsCaps(CCommConf* pComConf, CConfParty* pConfParty, CSipCaps* pSipLocalCaps, DWORD serviceId)
{
  PASSERT_AND_RETURN(!pComConf);
  PASSERT_AND_RETURN(!pConfParty);
  PASSERT_AND_RETURN(!pSipLocalCaps);

  PTRACE(eLevelInfoNormal, "CLobby::CalculateSipOptionsCaps");

  // create sip capabilities
  CSipComMode* pComMode = new CSipComMode;
  pComMode->CreateSipOptions(pComConf, AUTO, pConfParty->GetIsEncrypted());

  // calculate video bit rate
  DWORD vidBitrate = 0; DWORD confRate = 0;
  CalculateRateForSipOptions(pConfParty, pComMode, confRate, vidBitrate, pComConf);
  vidBitrate = vidBitrate / 100;

  // create with default caps and enable H263 4cif according to video parameters
  eVideoQuality vidQuality = pComConf->GetVideoQuality();

  pSipLocalCaps->Create(pComMode, vidBitrate, pConfParty->GetName(), vidQuality, serviceId,eUseBothEncryptionKeys, eCopVideoFrameRate_None, pConfParty->GetMaxResolution());

  POBJDELETE(pComMode);
}

//--------------------------------------------------------------------------
void CLobby::SendSipOptionsResponse(mcIndOptions* pOptionsInd, enSipCodes sipCode, DWORD callIndex, WORD srcUnitId, CSipCaps* pSipCaps, DWORD serviceId)
{
  int localCapSize = 0, capabilitiesSize = 0, bodyAndHeadersSize = 0, exactCapsSize = 0;

  if (pSipCaps)
  {
    localCapSize       = pSipCaps->CalcCapBuffersSize(cmCapReceiveAndTransmit, NO);
    capabilitiesSize   = sizeof(sipMediaLinesEntryBaseSt) + localCapSize;
    bodyAndHeadersSize = 0 + capabilitiesSize;
  }

  mcReqOptionsResp* pOptionsResp = (mcReqOptionsResp*) new BYTE[sizeof(mcReqOptionsResp) + bodyAndHeadersSize];
  memset(pOptionsResp, 0, sizeof(mcReqOptionsResp) + bodyAndHeadersSize);
  pOptionsResp->pCallObj   = pOptionsInd->pCallObj;
  pOptionsResp->status     = sipCode;
  pOptionsResp->remoteCseq = pOptionsInd->remoteCseq;

  if (pSipCaps)
  {
    sipSdpAndHeadersSt* pSdpAndHeaders = (sipSdpAndHeadersSt*)&pOptionsResp->sipSdpAndHeaders;

    // start with Bfcp over UDP
    exactCapsSize = pSipCaps->AddCapsToCapStruct(cmCapReceiveAndTransmit, NO, (sipMediaLinesEntrySt*)pSdpAndHeaders->capsAndHeaders, capabilitiesSize, eMediaLineSubTypeUdpBfcp);
  }

  pOptionsResp->sipSdpAndHeaders.callRate            = 0;
  pOptionsResp->sipSdpAndHeaders.sipMediaLinesOffset = 0;
  pOptionsResp->sipSdpAndHeaders.sipMediaLinesLength = exactCapsSize;
  pOptionsResp->sipSdpAndHeaders.lenOfDynamicSection = exactCapsSize;
  for (int i = kMediaLineInternalTypeNone+1; i < kMediaLineInternalTypeLast; i++)
  {
    mcXmlTransportAddress& mediaIp = ExtractMLineMediaIp((eMediaLineInternalType)i, &pOptionsResp->sipSdpAndHeaders, m_dummyMediaIp);
    mediaIp.unionProps.unionType = eIpVersion4;
    mediaIp.unionProps.unionSize = sizeof(ipAddressIf);
    mediaIp.transAddr.ipVersion  = eIpVersion4;
  }

  CSegment* pSeg = new CSegment;
  *pSeg << callIndex << srcUnitId;
  pSeg->Put((BYTE*)pOptionsResp, sizeof(mcReqOptionsResp)+bodyAndHeadersSize);

  SendMsgToCS(SIP_CS_SIG_OPTIONS_RESP_REQ, pSeg, serviceId);
  POBJDELETE(pSeg);
  PDELETEA(pOptionsResp);
}

//--------------------------------------------------------------------------
void CLobby::SendMsgToCS(OPCODE opcode, CSegment* pseg1, DWORD serviceId)
{
  CMplMcmsProtocol* pMplMcmsProtocol = new CMplMcmsProtocol;
  PASSERT_AND_RETURN(!pMplMcmsProtocol);

  pMplMcmsProtocol->AddCommonHeader(opcode);
  pMplMcmsProtocol->AddMessageDescriptionHeader();
  pMplMcmsProtocol->AddPortDescriptionHeader(0, 0);

  if (pseg1)
  {
    DWORD callIndex = 0;
    WORD  srcUnitId = 0;
    *pseg1 >> callIndex >> srcUnitId;
    pMplMcmsProtocol->AddCSHeader(serviceId, 0, srcUnitId, callIndex);

    DWORD nMsgLen  = pseg1->GetWrtOffset() - pseg1->GetRdOffset();
    BYTE* pMessage = new BYTE[nMsgLen];
    pseg1->Get(pMessage, nMsgLen);
    pMplMcmsProtocol->AddData(nMsgLen, (const char*)pMessage);
    PDELETEA(pMessage);
  }

  pMplMcmsProtocol->AddPayload_len(CS_API_TYPE);
  CMplMcmsProtocolTracer(*pMplMcmsProtocol).TraceMplMcmsProtocol("CLobby::SendMsgToCS ", CS_API_TYPE);
  pMplMcmsProtocol->SendMsgToCSApiCommandDispatcher();

  PDELETE(pMplMcmsProtocol);
}

//--------------------------------------------------------------------------
void CLobby::SuspendIsdnCallConfExist(CIsdnNetSetup* pNetSetup, DWORD ConfId, char* confName, const char* pCalledTelNumber, const char* pCallingTelNumber)
{
  PASSERT_AND_RETURN(!pNetSetup);
  PASSERT_AND_RETURN(!confName);

  TRACEINTO << "CLobby::SuspendIsdnCallConfExist - ConfName:" << confName;
  SetPartyListIDForSuspendCall(pCalledTelNumber, pCallingTelNumber);

  CReception* pLobbyTrans;
  // allocate a Suspended Reception
  if ((pNetSetup->m_callType == ACU_VOICE_SERVICE) || (pNetSetup->m_callType == ACU_MODEM_SERVICE))   // PSTN
    pLobbyTrans =  new CReceptionVoice(this);
  else                                                                                                // ISDN
    pLobbyTrans =  new CReceptionIsdn(this);

  pLobbyTrans->CreateSuspendConfExist(pNetSetup, m_pRcvMbx, this, m_pParyList_Id, ConfId, confName);
  m_pTransferList->Insert(pLobbyTrans);
}

//--------------------------------------------------------------------------
void CLobby::RejectIsdnCall(CIsdnNetSetup* pNetSetup, int reason)
{
  PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pNetSetup));

  WORD numDigits = 0;
  ALLOCBUFFER(pCallingTelNumber, PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER);
  pNetSetup->GetCallingNumber(&numDigits, pCallingTelNumber);

  TRACEINTO << "CLobby::RejectIsdnCall - Call rejected, CLI:" << pCallingTelNumber;

  CReception* pReception;
  if ((pNetSetup->m_callType == ACU_VOICE_SERVICE) || (pNetSetup->m_callType == ACU_MODEM_SERVICE)) // PSTN
    pReception =  new CReceptionVoice(this);
  else                                                                                              // ISDN
    pReception =  new CReceptionIsdn(this);

  pReception->ReplayToNet(NET_CLEAR_REQ, *pNetSetup);
  pReception->Destroy(false);

  DEALLOCBUFFER(pCallingTelNumber);
  POBJDELETE(pReception);
}

//--------------------------------------------------------------------------
int CLobby::FindDestConf(const char* calledPhoneNumber, const char* pCallingTelNumber, char* confName, DWORD& confId, CCommConf** ppComConf)
{
  int lookupRes = ::GetpConfDB()->GetConfParty(calledPhoneNumber, (CCommConf**)ppComConf);

  // Found ongoing confernece
  if (-2 == lookupRes)
  {
    TRACEINTO << "CLobby::FindDestConf - Ongoing conference is found, ConfName:" << (char*)(*ppComConf)->GetName();
    return 1;
  }

  // Search for a Meeting room
  lookupRes = ::GetpMeetingRoomDB()->GetMeetingRoom(calledPhoneNumber, pCallingTelNumber, confName, confId);

  // Found a meeting room
  if (lookupRes == 0)
  {
    TRACEINTO << "CLobby::FindDestConf - Meeting room is found, ConfName:" << confName;
    return 2;
  }

  // Transit EQ - In case a destination conference wasn't found we will try to connect to Transit EQ
  std::string transitEQName = ::GetpMeetingRoomDB()->GetTransitEQName();
  if (transitEQName == "")
  {
    TRACEINTO << "CLobby::FindDestConf - Can not find Conf or MR and No Transit EQ in System";
    return 0;
  }

  // Found a transit EQ - Find out if its alive
  *ppComConf = ::GetpConfDB()->GetCurrentOnGoingEQ(transitEQName.c_str());
  if (*ppComConf)
  {
    TRACEINTO << "CLobby::FindDestConf - Ongoing Transit EQ is found, ConfName:" << transitEQName;
    return 1;
  }

  // The Transit EQ is not Awake
  confId = ::GetpMeetingRoomDB()->GetTransitEQID();
  strncpy(confName, transitEQName.c_str(), H243_NAME_LEN-1);
  confName[H243_NAME_LEN-1] = '\0';

  TRACEINTO << "CLobby::FindDestConf - Transit EQ reservation is found, awaking it, ConfName:" << transitEQName;
  return 2;
}

//--------------------------------------------------------------------------
void CLobby::SetPreDefinedIvrForTransitAdhocEqIfNeeded(char* transitEQName, CIpNetSetup* pNetSetup, const char* strToSet)
{
  if (!strToSet || !strcmp(strToSet, ""))
    return;

  const char* strWithoutPrefix = ::GetIpServiceListMngr()->FindServiceAndGetStringWithoutPrefix(strToSet, PARTY_H323_ALIAS_H323_ID_TYPE);
  if (!strWithoutPrefix)
    strWithoutPrefix = strToSet;

  CCommRes* pTransitEq = ::GetpMeetingRoomDB()->GetCurrentRsrv(transitEQName);
  BYTE      bEnableDefaultAdHocCalls = GetSystemCfgFlagInt<BOOL>(CFG_KEY_ENABLE_DEFAULT_ADHOC_CALLS);
  if (pTransitEq && pTransitEq->GetAdHoc() && bEnableDefaultAdHocCalls)
  {
    TRACEINTO << "CLobby::SetPreDefinedIvrForTransitAdhocEqIfNeeded - strWithoutPrefix:" << strWithoutPrefix;
    pNetSetup->SetPredefiendIvrStr(strWithoutPrefix);
  }

  POBJDELETE(pTransitEq);
}

//--------------------------------------------------------------------------
void CLobby::OnTestTimer(CSegment* pParam)
{
  DeleteTimer(TEST_TIMER_2);
  CSegment* pMsg = new CSegment;

  pMsg->Put((BYTE*)m_pBndNetSetup, sizeof(NET_SETUP_IND_S));
  (*pMsg) << m_pBndNetSetup->m_box_id;
  (*pMsg) << m_pBndNetSetup->m_boardId;
  (*pMsg) << m_pBndNetSetup->m_sub_board_id;

  OnNetCallIn((CSegment*)pMsg);
}

//--------------------------------------------------------------------------
void CLobby::OnDelayedH323Call(CDelayedH323Call* pCall)
{
  CSegment* pMsg = pCall->GetMessage();
  OnH323CallIn(pMsg);
  VEC_DELAYED_CALLS::iterator itr  = FindPosition(pCall);
  if (itr != m_pDelayedCalls->end())
    m_pDelayedCalls->erase(itr);

  POBJDELETE(pCall);
}

//--------------------------------------------------------------------------
VEC_DELAYED_CALLS::iterator CLobby::FindPosition(const CDelayedH323Call* pCall)
{
  VEC_DELAYED_CALLS::iterator itr =  m_pDelayedCalls->begin();

  while (itr != m_pDelayedCalls->end())
  {
    if (CPObject::IsValidPObjectPtr(pCall) && *(*itr) == *pCall)
      return itr;
    itr++;
  }
  return itr;
}

//--------------------------------------------------------------------------
void CLobby::SetMsConversationIdInConfDB(DWORD confId, char* targetConfName)
{
  CReception* pSameMRReception = m_pTransferList->FindSameMRReceptionConfOnAirNO(targetConfName);
  if (pSameMRReception && (strcmp(pSameMRReception->NameOf(), "CReceptionSip") == 0))
  {
    CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(confId);
    if (pCommConf)
    {
      pCommConf->SetMsConversationId((char*)((CReceptionSip*)pSameMRReception)->GetClickToConfId());
      TRACEINTO << "CLobby::SetMsConversationIdInConfDB - ConversationId:" << pCommConf->GetMsConversationId();
    }
  }
}

//--------------------------------------------------------------------------
MsAdhocConfType CLobby::FindAdhocConfForMsConversationId(char* pMsConversationId, char* pAdhocName)
{
  PASSERT_AND_RETURN_VALUE(!pMsConversationId, MsAdhoc_ConfNotFound);
  PASSERT_AND_RETURN_VALUE(!strcmp(pMsConversationId, ""), MsAdhoc_ConfNotFound);
  PASSERT_AND_RETURN_VALUE(!pAdhocName, MsAdhoc_ConfNotFound);

  TRACEINTO << "CLobby::FindAdhocConfForMsConversationId - ConversationId:" << pMsConversationId;

  MsAdhocConfType retAdhocType = MsAdhoc_ConfNotFound;

  const char* pTempAdhocName = ::GetpConfDB()->GetConfByMsConversationId(pMsConversationId);
  if (pTempAdhocName)
    retAdhocType = MsAdhoc_ExistingConf;
  else
  {
    CReception* pReceptionSameMsConvId = m_pTransferList->FindReceptionWithSameMsConversationIdConfOnAirNO(pMsConversationId);
    if (pReceptionSameMsConvId)
    {
      pTempAdhocName = pReceptionSameMsConvId->GetTargetConfName();
      if (pTempAdhocName)
        retAdhocType = MsAdhoc_ExistingSuspend;
    }
    else
    {
      pTempAdhocName = ::GetpMeetingRoomDB()->GetFirstSIPFactoryName(TRUE);
      if (pTempAdhocName)
        retAdhocType = MsAdhoc_CreateFromFactory;
    }
  }

  if ((retAdhocType != MsAdhoc_ConfNotFound) && pTempAdhocName)
  {
    strncpy(pAdhocName, pTempAdhocName, H243_NAME_LEN-1);
    pAdhocName[H243_NAME_LEN-1] = '\0';
    TRACEINTO << "CLobby::FindAdhocConfForMsConversationId - Found target adhoc, ConfName:" << pAdhocName << ", Type:" << retAdhocType;
  }
  else
    TRACEINTO << "CLobby::FindAdhocConfForMsConversationId - No match adhoc conference was found and no auto connect factory is defined";

  return retAdhocType;
}

//--------------------------------------------------------------------------
void CLobby::UpdateInitialEncryptionForParty(CSipNetSetup* pNetSetup, CConfIpParameters* pService, BOOL partyHasSdp, BOOL partyHasSDES, CCommRes* pCommRes)
{
  PASSERT_AND_RETURN(!pNetSetup);
  PASSERT_AND_RETURN(!pService);
  PASSERT_AND_RETURN(!pCommRes);

  //BRIDGE-4061
  BYTE isTipCompatible = pCommRes->GetIsTipCompatible();
  CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(pCommRes->GetName());
  bool isOngoing = false;
  if( pCommConf )
	  isOngoing = true;
  if( (pCommRes->IsMeetingRoom() || pCommRes->GetEntryQ()) &&  (!isOngoing) )
  {
	 DWORD templateId = 0xFFFFFFFF;
	 templateId = pCommRes->GetAdHocProfileId();
	 STATUS status = STATUS_OK;
	 if (templateId != (DWORD)-1)
	 {
		STATUS status = pCommRes->IsProfileExists(templateId);
		if( status == STATUS_OK )
		{
			CCommResApi* pProfile = (CCommResApi*)::GetpProfilesDB()->GetCurrentRsrv(templateId);
			if (pProfile)
			{
				isTipCompatible = pProfile->GetIsTipCompatible();
				POBJDELETE(pProfile);
			}
		}

	 }
	 TRACEINTO << "templateId - " << templateId << " isTipCompatible - " << (int)isTipCompatible ;
  }

  // Fix for undefined dial-in, when the EP encryption setting hasn't resolved yet
  if (pService->GetSipTransportType() != eTransportTypeTls && !isTipCompatible && (CProcessBase::GetProcess()->GetProductType()!=eProductTypeSoftMCUMfw))
  {
	  TRACEINTO << "CLobby::UpdateInitialEncryptionForParty - IP Service without TLS- Setting encryption to NO";
	  pNetSetup->SetInitialEncryptionValue(NO);
  }
  else		// call into TLS IP service
  {
	  if (partyHasSDES)
	  {
		  /* BRIDGE 7107  */
		  TRACEINTO << "CLobby::UpdateInitialEncryptionForParty - SDES declared - Setting encryption to unknown";
		  pNetSetup->SetInitialEncryptionValue(AUTO);
	  }
	  else
	  {
		  if (!partyHasSdp || isTipCompatible )
		  {
			  TRACEINTO << "CLobby::UpdateInitialEncryptionForParty - No SDP found -  encryption setting unknown";
			  pNetSetup->SetInitialEncryptionValue(AUTO);
		  }
		  else
		  {
			  TRACEINTO << "CLobby::UpdateInitialEncryptionForParty - No SDES declared - Setting encryption to NO";
			  pNetSetup->SetInitialEncryptionValue(NO);
		  }
	  }
  }
}

//--------------------------------------------------------------------------
// vngr-25287
BYTE CLobby::McCheckIfNoSdp(const sipSdpAndHeadersSt *pSdp)
{
	sipMediaLinesEntrySt 	*pMediaLinesEntry 	= NULL;
	sipMediaLineSt 			*pMediaLine 		= NULL;
	unsigned int 			i 					= 0;
	int 					mediaLinePos 		= 0;

	// It mean no dynamic part or capLength <= 0
	//------------------------------------------
	if (pSdp->sipMediaLinesLength == 0)
	{
		TRACEINTO << "CLobby::McCheckIfNoSdp - sipMediaLinesLength is 0";
		return TRUE;
	}

	// it means caps header exist but caps is zero.
	//---------------------------------------------
	pMediaLinesEntry = (sipMediaLinesEntrySt *) &pSdp->capsAndHeaders[pSdp->sipMediaLinesOffset];

	if (!pMediaLinesEntry)
	{
		TRACEINTO << "CLobby::McCheckIfNoSdp - pMediaLinesEntry is NULL";
		return TRUE;
	}

	if (pMediaLinesEntry->numberOfMediaLines == 0)
	{
		TRACEINTO << "CLobby::McCheckIfNoSdp - numberOfMediaLines is 0";
		return TRUE;
	}

	for (i = 0; i < pMediaLinesEntry->numberOfMediaLines; i++) {

		pMediaLine = (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];

		if (!pMediaLine)
		{
			TRACEINTO << "CLobby::McCheckIfNoSdp - pMediaLine is NULL";
			return TRUE;
		}

		if (pMediaLine->numberOfCaps)
			return FALSE;

		mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;
	}

	return TRUE;
}

//Amihay: the same routine in SipUtils.cpp - IsSdesDeclaredInSdp()
////--------------------------------------------------------------------------
//BYTE CLobby::CheckIsSdpHasSdesCaps(sipSdpAndHeadersSt *pSdp)
//{
//	sipMediaLinesEntrySt* pMediaLinesEntry 	= (sipMediaLinesEntrySt*)pSdp->capsAndHeaders;
//	int mediaLinePos 						= 0;
//
//	const sipMediaLineSt *pMediaLine 		= (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];
//	const capBuffer* pCapBuffer 			= (capBuffer*) &pMediaLine->caps[0];
//	const BYTE* pTemp 						= (const BYTE*)pCapBuffer;
//
//	mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;
//
//	for (unsigned int i = 0 ; i < pMediaLine->numberOfCaps; i++)
//	{
//		if ((CapEnum)pCapBuffer->capTypeCode == eSdesCapCode)
//		{
//			PTRACE(eLevelInfoNormal,"CLobby::CheckIsSdpHasSdesCaps -FOUND SDES");
//			return TRUE;
//		}
//
//		pTemp 		+= sizeof(capBufferBase) + pCapBuffer->capLength;
//		pCapBuffer 	= (capBuffer*)pTemp;
//	}
//
//	return FALSE;
//}

//--------------------------------------------------------------------------
void CLobby::ExtractAndSetPerferedIpV6ScopeAddr(sipSdpAndHeadersSt *pSdpAndHeaders,
	CSipNetSetup *pNetSetup)
{
    // In this case we need to find out what is the perfered scopeId we want to open the media towards the EP
    // We will take thee scope Id from the Media Ip!
    char* mediaAddr = new char[IPV6_ADDRESS_LEN];
    memset(mediaAddr, '\0', IPV6_ADDRESS_LEN);

    enScopeId ePerferedIpV6ScopeAddr;
	mcXmlTransportAddress mediaIp = ExtractMLineMediaIpAccordingToMediaType(eMediaLineTypeAudio, pSdpAndHeaders, m_dummyMediaIp);
    ::ipToString(mediaIp.transAddr, mediaAddr, 1);
    ePerferedIpV6ScopeAddr = ::getScopeId(mediaAddr);
    TRACEINTO << "IPv6 addr:" << mediaAddr << ", scope:" << ePerferedIpV6ScopeAddr;
    PDELETEA(mediaAddr);
    pNetSetup->SetPerferedIpV6ScopeAddr(ePerferedIpV6ScopeAddr);
}

/*
 Function name: BuildSipNetSetup
 Variables:     pSipInvite: SIP invite struct C++ instance
                     pNetSetup: SIP net setup instance used by call flow decision

 Description:   Build SIP net setup from SIP invite
*/
//--------------------------------------------------------------------------
void CLobby::BuildSipNetSetup(CSipInviteStruct *pSipInvite,
	CSipNetSetup *pNetSetup)
{
  sipSdpAndHeadersSt *pSdpAndHeaders = pSipInvite->GetSdpAndHeaders();
  int    numOfDestAlias   = 1;
  int    numOfReqAliases  = 1;

  char strDstPartyAddress       [IP_LIMIT_ADDRESS_CHAR_LEN] = { 0 };
  char strDstPartyAddress2      [IP_LIMIT_ADDRESS_CHAR_LEN] = { 0 };

  // step 1: retrieve info such as IP address from SIP invite struct
  const char* strFrom           = pSipInvite->GetHeaderField(kFrom);
  const char* strTo             = pSipInvite->GetHeaderField(kTo);
  const char* strCallId         = pSipInvite->GetHeaderField(kCallId);
  const char* strReqUri         = pSipInvite->GetHeaderField(kReqLine);
  const char* strContact        = pSipInvite->GetHeaderField(kContact);
//  BOOL 	fIsMSConfInvite 		= pSipInvite->FindHeaderField(kContentType, "application/ms-conf-invite+xml");
//  BOOL 	fIsCCCPInvite   		= pSipInvite->FindHeaderField(kContentType, "application/cccp+xml");
  BOOL  fSupportedPlcmIvr       = pSipInvite->FindHeaderField(kSupported,   "plcm-ivr-service-provider");


  ParseSipToPhContextUri(strTo, &numOfDestAlias, strDstPartyAddress, IP_LIMIT_ADDRESS_CHAR_LEN, strDstPartyAddress2, IP_LIMIT_ADDRESS_CHAR_LEN);

  TRACEINTO << "- strFrom:"   << DUMPSTR(strFrom)
            << ", strTo:"     << DUMPSTR(strTo)
            << ", strCallId:" << DUMPSTR(strCallId)
            << ", strReqUri:" << DUMPSTR(strReqUri)
            << ", strContact:" << DUMPSTR(strContact)
            << ", CalleeIP:"  << DUMPSTR(strDstPartyAddress);

  // step 2: set those info to SIP net setup
  pNetSetup->SetRemoteSipAddress(strFrom);
  if (strncmp(strFrom, "tel:", 4) == 0)
    pNetSetup->SetRemoteSipAddressType(PARTY_SIP_TELURL_ID_TYPE);
  else
    pNetSetup->SetRemoteSipAddressType(PARTY_SIP_SIPURI_ID_TYPE);

  pNetSetup->SetLocalSipAddress(strDstPartyAddress);
  pNetSetup->SetSrcPartyAddress(strFrom);
  pNetSetup->SetDestPartyAddress(strDstPartyAddress);
  pNetSetup->SetToPartyAddress(strDstPartyAddress);
  pNetSetup->SetCallIndex(pSipInvite->GetCallIndex());
  pNetSetup->SetRemoteDisplayName(pSipInvite->GetHeaderField(kFromDisplay));
  pNetSetup->SetCallId(strCallId); // call_id - to NetSetup
  pNetSetup->SetRemoteSetupRate(pSdpAndHeaders->callRate * 100);
  pNetSetup->SetRemoteSipContact(strContact);
  pNetSetup->SetMsConversationId(pSdpAndHeaders);  //Inga



  mcXmlTransportAddress mediaIp = ExtractMLineMediaIpAccordingToMediaType(eMediaLineTypeAudio, pSdpAndHeaders, m_dummyMediaIp);
  //pNetSetup->SetIpVersion((enIpVersion)ExtractMLineMediaIpAccordingToMediaType(eMediaLineTypeAudio, pSdpAndHeaders, m_dummyMediaIp).transAddr.ipVersion);
  //pNetSetup->SetTaSrcPartyAddr((const mcTransportAddress*)(&m_dummyMediaIp.transAddr));
  pNetSetup->SetIpVersion((enIpVersion)mediaIp.transAddr.ipVersion);
  pNetSetup->SetTaSrcPartyAddr((const mcTransportAddress*)(&mediaIp.transAddr));


  int scopeNum = 0;
  BYTE isNoSdp = ! IsSdpPresent(pSdpAndHeaders);
  if(isNoSdp) // set ip version according to URI
	scopeNum = pNetSetup->SetIpVersionAccordingToUri();

  pNetSetup->SetCsServiceid(pSipInvite->GetCsServiceId());

  TRACEINTO << "IP VERSION: " << pNetSetup->GetIpVersion();
  // step 3: IPv6 special handle
  if (pNetSetup->GetIpVersion() == eIpVersion6)
  {
		if(isNoSdp)
			pNetSetup->SetPerferedIpV6ScopeAddr((enScopeId)scopeNum);
		else
			ExtractAndSetPerferedIpV6ScopeAddr(pSdpAndHeaders, pNetSetup);
  }

}
//--------------------------------------------------------------------------
void CLobby::BuildSipNetSetupForMSConfInvite(CSipInviteStruct *pSipInvite,CSipNetSetup *pNetSetup)
{
  sipSdpAndHeadersSt *pSdpAndHeaders = pSipInvite->GetSdpAndHeaders();
  int    numOfDestAlias   = 1;
  int    numOfReqAliases  = 1;

  char strDstPartyAddress       [IP_LIMIT_ADDRESS_CHAR_LEN] = { 0 };
  char strDstPartyAddress2      [IP_LIMIT_ADDRESS_CHAR_LEN] = { 0 };

  // step 1: retrieve info such as IP address from SIP invite struct
  const char* strFrom           = pSipInvite->GetHeaderField(kFrom);
  const char* strTo             = pSipInvite->GetHeaderField(kTo);
  const char* strCallId         = pSipInvite->GetHeaderField(kCallId);
  const char* strReqUri         = pSipInvite->GetHeaderField(kReqLine);
  const char* strContact        = pSipInvite->GetHeaderField(kContact);
//  BOOL 	fIsMSConfInvite 		= pSipInvite->FindHeaderField(kContentType, "application/ms-conf-invite+xml");
//  BOOL 	fIsCCCPInvite   		= pSipInvite->FindHeaderField(kContentType, "application/cccp+xml");
  BOOL  fSupportedPlcmIvr       = pSipInvite->FindHeaderField(kSupported,   "plcm-ivr-service-provider");


  ParseSipToPhContextUri(strTo, &numOfDestAlias, strDstPartyAddress, IP_LIMIT_ADDRESS_CHAR_LEN, strDstPartyAddress2, IP_LIMIT_ADDRESS_CHAR_LEN);

  TRACEINTO << "- strFrom:"   << DUMPSTR(strFrom)
            << ", strTo:"     << DUMPSTR(strTo)
            << ", strCallId:" << DUMPSTR(strCallId)
            << ", strReqUri:" << DUMPSTR(strReqUri)
            << ", strContact:" << DUMPSTR(strContact)
            << ", CalleeIP:"  << DUMPSTR(strDstPartyAddress);

  // step 2: set those info to SIP net setup
  pNetSetup->SetRemoteSipAddress(strFrom);
  if (strncmp(strFrom, "tel:", 4) == 0)
    pNetSetup->SetRemoteSipAddressType(PARTY_SIP_TELURL_ID_TYPE);
  else
    pNetSetup->SetRemoteSipAddressType(PARTY_SIP_SIPURI_ID_TYPE);

  pNetSetup->SetLocalSipAddress(strDstPartyAddress);
  pNetSetup->SetSrcPartyAddress(strFrom);
  pNetSetup->SetDestPartyAddress(strDstPartyAddress);
  pNetSetup->SetToPartyAddress(strDstPartyAddress);
  pNetSetup->SetCallIndex(pSipInvite->GetCallIndex());
  pNetSetup->SetRemoteDisplayName(pSipInvite->GetHeaderField(kFromDisplay));
  pNetSetup->SetCallId(strCallId); // call_id - to NetSetup
  pNetSetup->SetRemoteSetupRate(pSdpAndHeaders->callRate * 100);
  pNetSetup->SetRemoteSipContact(strContact);

  pNetSetup->SetCsServiceid(pSipInvite->GetCsServiceId()); //Is needed??

  //Lync 2013
    pNetSetup->SetMsConversationId(pSdpAndHeaders);  //Inga
    pNetSetup->SetIsMsConfInvite(TRUE);
    pNetSetup->SetOriginalToFromDmaStr(pSdpAndHeaders);
    //take to


}

//--------------------------------------------------------------------------
void CLobby::AllocateAndExtractFindConfAndPartyKeys(CSipInviteStruct *pSipInvite,
	CSipNetSetup *pNetSetup,
	findConfAndPartyKeysSt &keys)
{
  memset(&keys, 0, sizeof(keys));
  keys.numOfSrcAlias = 1;
  keys.numOfDestAlias   = 1;

  int    numOfReqAliases  = 1;

  char strDstPartyAddress       [IP_LIMIT_ADDRESS_CHAR_LEN] = { 0 };
  char strDstPartyAddress2      [IP_LIMIT_ADDRESS_CHAR_LEN] = { 0 };
  char strDstPartyAddressReqUri [IP_LIMIT_ADDRESS_CHAR_LEN] = { 0 };
  char strDstPartyAddressReqUri2[IP_LIMIT_ADDRESS_CHAR_LEN] = { 0 };

  // step 1: get INVITE header value
  const char* strFrom           = pSipInvite->GetHeaderField(kFrom);
  const char* strTo             = pSipInvite->GetHeaderField(kTo); // Should be kTo instead, need to investigate more.
  const char* strReqUri         = pSipInvite->GetHeaderField(kReqLine);

  // step 2: parse INVITE header value
  int parsePcStatus = ParseSipToPhContextUri(strTo, &keys.numOfDestAlias,
  	strDstPartyAddress, IP_LIMIT_ADDRESS_CHAR_LEN,
  	strDstPartyAddress2, IP_LIMIT_ADDRESS_CHAR_LEN);

  int parseReqPcStatus = ParseSipToPhContextUri(strReqUri, &numOfReqAliases,
  	strDstPartyAddressReqUri, IP_LIMIT_ADDRESS_CHAR_LEN,
  	strDstPartyAddressReqUri2, IP_LIMIT_ADDRESS_CHAR_LEN);

  keys.pPartySrcAliasList = new CH323Alias;
  keys.pPartyDstAliasList = new CH323Alias[keys.numOfDestAlias + numOfReqAliases]; // [numOfDestAlias]; //VNGFE-4657

  ParseSipUri(kSrc, strFrom, keys, pNetSetup);

  // VNGFE-4657
  ParseSipUri(kDst, strDstPartyAddress, keys, pNetSetup);

  if (parsePcStatus == STATUS_OK && strDstPartyAddress2[0] != '\0' && keys.numOfDestAlias == 2)
  {
    keys.pPartyDstAliasList[0].SetAliasType(PARTY_H323_ALIAS_H323_ID_TYPE);
    ParseSipUri(kDst2, strDstPartyAddress2, keys, pNetSetup);
  }

  // VNGFE-4657
  // Add the ReqURI to the check list
  ParseSipUri(kDstReqUri, strDstPartyAddressReqUri, keys, pNetSetup);

  if (parseReqPcStatus == STATUS_OK && strDstPartyAddressReqUri2[0] != '\0' && numOfReqAliases == 2)
  {
    keys.pPartyDstAliasList[keys.numOfDestAlias].SetAliasType(PARTY_H323_ALIAS_H323_ID_TYPE);
    ParseSipUri(kDstReqUri2, strDstPartyAddressReqUri2, keys, pNetSetup);
  }

  keys.numOfDestAlias += numOfReqAliases;

  // step 3: Save INVITE header kMsConversationId
 // SAFE_COPY(keys.strMsConversationID, pSipInvite->GetHeaderField(kMsConversationId));
  SAFE_COPY(keys.strClickToConfID, pSipInvite->GetHeaderField(kClick2Conf));
}

//--------------------------------------------------------------------------
void CLobby::FindMatchingConfAndParty(findConfAndPartyKeysSt &keys,
	CSipNetSetup *pNetSetup,
	findConfAndPartyResultSt &findResult)
{
  findResult.pComConf = NULL;
  findResult.pConfParty = NULL;
  findResult.meetingRoomId = 0xFFFFFFFF;
  findResult.useTransitEQOnly = FALSE;
  memset(findResult.strMsAdhocName, 0, H243_NAME_LEN);
  findResult.eResult = eFindConfAndPartyResultLast;

  mcTransportAddress dummyAddr;
  memset(&dummyAddr, 0, sizeof(mcTransportAddress));

  char strDstPartyAddress       [IP_LIMIT_ADDRESS_CHAR_LEN] = { 0 };
  MsAdhocConfType eAdhocType = MsAdhoc_Unknown;

/*  // step 1: Find Ms Conversation if needed
  if (strcmp(keys.strClickToConfID, ""))  // Call with ms conversation id
  {
    eAdhocType = FindAdhocConfForMsConversationId(keys.strClickToConfID,
		findResult.strMsAdhocName);

    if ((eAdhocType != MsAdhoc_Unknown)
		&& (eAdhocType != MsAdhoc_ConfNotFound) && strcmp(findResult.strMsAdhocName, ""))
    {
      if (SetNewDestinationConfName(pNetSetup, (char*)findResult.strMsAdhocName, TRUE))
      {
        strncpy(strDstPartyAddress, pNetSetup->GetDestPartyAddress(), IP_LIMIT_ADDRESS_CHAR_LEN - 1);
        strDstPartyAddress[IP_LIMIT_ADDRESS_CHAR_LEN - 1] = '\0';
        ParseSipUri(kDst, strDstPartyAddress, keys, pNetSetup);
      }
    }
  }*/

  // step 2: Find matching conf and party according to input alias
  findResult.eResult = (eFindConfAndPartyResult)GetIpConfPartyType(((const mcTransportAddress*)&dummyAddr),
  	keys.pPartySrcAliasList, keys.numOfSrcAlias,
  	(CCommConf**)&findResult.pComConf, &findResult.pConfParty,
  	keys.pPartyDstAliasList, keys.numOfDestAlias,
  	findResult.meetingRoomId, FALSE, keys.pAdHocConfName);

  if (eAdhocType == MsAdhoc_ExistingSuspend)
    findResult.eResult = eFindConfAndParty_ON_AIR_MS_ADHOC_CONF_FOUND;                                     // wait for adhoc to be on air.
  else if (eAdhocType == MsAdhoc_ConfNotFound)
    findResult.eResult = eFindConfAndParty_MS_ADHOC_CONF_NOT_FOUND;                                     // no conference was found for ms adhoc call

  if (findResult.pConfParty && findResult.pConfParty->GetNetInterfaceType() != SIP_INTERFACE_TYPE)
  {
    findResult.eResult = eFindConfAndParty_H323_PARTY_FOUND_FOR_SIP;
    findResult.pConfParty->SetDefinedPartyAssigned(FALSE); // sip call into defined H.323 party
  }

  char transitEQName[H243_NAME_LEN] = { 0 };
  SAFE_COPY(transitEQName, ::GetpMeetingRoomDB()->GetTransitEQName());

  // step 3: Find through transit entry queue
  if (findResult.eResult == eFindConfAndParty_CONF_NOT_FOUND && strcmp(transitEQName, ""))
  {
    char user[ALIAS_NAME_LEN] = { 0 };
    SAFE_COPY(user, keys.pPartyDstAliasList->GetAliasName());

    TRACEINTO << "Failed, Destination conference wasn't found "
              << ", TransitEQName:" << DUMPSTR(transitEQName);

    findResult.useTransitEQOnly = SetNewDestinationConfName(pNetSetup, transitEQName, TRUE);
    if (findResult.useTransitEQOnly == TRUE)
    {
      SAFE_COPY(strDstPartyAddress, pNetSetup->GetDestPartyAddress());

      ParseSipUri(kDst, strDstPartyAddress, keys, pNetSetup);
      findResult.eResult = (eFindConfAndPartyResult)GetIpConfPartyType(((const mcTransportAddress*)&dummyAddr),
	  	    keys.pPartySrcAliasList, keys.numOfSrcAlias,
	  	    (CCommConf**)&findResult.pComConf, &findResult.pConfParty,
	  	    keys.pPartyDstAliasList, keys.numOfDestAlias,
	  	    findResult.meetingRoomId, TRUE, NULL, TRUE);

      if (::isIpV6Str(user) || ::IsValidIPAddress(user))
        memset(user, 0, ALIAS_NAME_LEN);

      SetPreDefinedIvrForTransitAdhocEqIfNeeded(transitEQName, pNetSetup, user);
    }
    else
    {
      TRACEINTO << "Failed to change the destination address to transit EQ";
    }
  }

  return;
}

//--------------------------------------------------------------------------
BOOL CLobby::IsServiceConfiguredAcceptCall(APIU32 csServiceId,
	CCommConf *pComConf, DWORD meetingRoomId, eFindConfAndPartyResult result, CH323Alias *pPartyDstAliasList)
{
  CConfIpParameters* pService = ::GetIpServiceListMngr()->FindIpService(csServiceId);
  char* serviceName = (char*)pService->GetServiceName();
  int   idx = 0;
  CCommRes* pConfRes = NULL;
  CCommRes* pTargetConfRes = NULL;

  if (pComConf)
  {
    pTargetConfRes = pComConf;
  }
  else
  {
    if (result != eFindConfAndParty_SIP_FACTORY_FOUND)
      pConfRes = ::GetpMeetingRoomDB()->GetCurrentRsrv(meetingRoomId);
    else
      pConfRes = ::GetpMeetingRoomDB()->GetCurrentRsrv(pPartyDstAliasList->GetAliasName());

	pTargetConfRes = pConfRes;
  }

  if (pTargetConfRes)
  {
    for (; idx < NUM_OF_IP_SERVICES; idx++)
    {
      if (!strcmp(serviceName, pTargetConfRes->GetServiceRegistrationContentServiceName(idx)))
        break;
    }

    if (idx == NUM_OF_IP_SERVICES)
    {
      TRACEINTO << "Failed to find appropriate service, ServiceName:" << serviceName;
    }
    else
    {
      if (pTargetConfRes->GetServiceRegistrationContentAcceptCall(idx) == FALSE)
      {
        TRACEINTO << "Failed, Dial-In from this service is not allowed (call rejected), ServiceName:" << serviceName;

        POBJDELETE(pConfRes);
        return FALSE;
      }
    }
    POBJDELETE(pConfRes);
  }

  return TRUE;
}

//--------------------------------------------------------------------------
BOOL CLobby::IsIvrNotSupported(CSipInviteStruct *pSipInvite,
	CCommConf *pComConf, DWORD meetingRoomId, eFindConfAndPartyResult result)
{
  BOOL ivrNotSupported = FALSE;
  BOOL fSupportedPlcmIvr = pSipInvite->FindHeaderField(kSupported, "plcm-ivr-service-provider");

  // Overide GetIpConfPartyType() returned value for SupportedPlcmIvr
  if (pComConf && !pComConf->GetEntryQ() && (pComConf->isIvrProviderEQ() || pComConf->isExternalIvrControl()) && !fSupportedPlcmIvr) //AT&T
  {
    TRACEINTO << "Overide value for SupportedPlcmIvr";
	ivrNotSupported = TRUE;
  }

  CCommRes* pMrRes = ::GetpMeetingRoomDB()->GetCurrentRsrv(meetingRoomId); // carmit3
  if (eFindConfAndParty_MEETING_ROOM_FOUND == result && pMrRes) // for unreserved party
  {
    if ((pMrRes->isIvrProviderEQ() || pMrRes->isExternalIvrControl()) && !pMrRes->GetEntryQ()  /*!fSupportedPlcmIvr*/) //AT&T
    {
      TRACEINTO << "Error, ivrProvider/externalIvrControl UNRESERVED PARTY with no plcmIvr support";
	  ivrNotSupported = TRUE;
    }
    POBJDELETE(pMrRes);
  }

  return ivrNotSupported;
}

//--------------------------------------------------------------------------
void CLobby::SubProcessConfAndPartyFound(CCommConf *pComConf,
	CConfParty *pConfParty, CSipNetSetup *pNetSetup, sipSdpAndHeadersSt *pSdpAndHeaders)
{
  // direct Dial In found a reservation to this party in the MCU data base
  // checking if the received party is fully connected.
  // if yes - reject the party
  if (pConfParty)
  {
    const char* tempName = pConfParty->GetName();
    if (tempName)
      TRACEINTO << "Party found, PartyName:" << tempName;

    DWORD party_status = pConfParty->GetPartyState();
    switch (party_status)
    {
      case PARTY_CONNECTED:
      case PARTY_SECONDARY:
      case PARTY_CONNECTING:
      case PARTY_CONNECTED_WITH_PROBLEM:
      case PARTY_DISCONNECTING:
      {
        DWORD confStat = (pComConf) ? pComConf->GetStatus() : 0;
        if (confStat & CONFERENCE_RESOURCES_DEFICIENCY)
          TRACEINTO << "Failed, Conference resource deficiency (call rejected) ";
        else
          TRACEINTO << "Failed, Destination party is full connected (call rejected) ";

        // unattended call handle ...
        RejectSipCall(pNetSetup, int(cmReasonTypeNoPermision), pSdpAndHeaders);
        break;
      }

      default:
      {
        if (pConfParty->GetNetInterfaceType() != SIP_INTERFACE_TYPE)
        {
          TRACEINTO << "Failed, The defined party is not a SIP party (call rejected)";
          RejectSipCall(pNetSetup, int(cmReasonTypeNoPermision), pSdpAndHeaders);
        }
        else
        {
         //eFeatureRssDialin
         //If there's Rss in the conference already, reject the incoming one
          if( IsRssDialinRejected(pNetSetup, pComConf, pSdpAndHeaders))
          {
          	TRACEINTO << "Failed, There's already recording link in the conf! (call rejected)";
	}
         else
  	{
		  //eFeatureRssDialin
		  pConfParty->SetRecordingLinkParty(NO);
		  pConfParty->SetPlaybackLinkParty(NO);
		  if(eSrsSessionTypeRecording ==  ::getSrsSessionType(pSdpAndHeaders))
		  {
			pConfParty->SetRecordingLinkParty(YES);
			//mute video for recording link
			pConfParty->SetVideoMute(YES);
		  }
		  else if(eSrsSessionTypePlayback==  ::getSrsSessionType(pSdpAndHeaders))
		  {
			pConfParty->SetPlaybackLinkParty(YES);
		  }

		  BYTE bIsSoftCp = (pComConf && pComConf->GetVideoSession() == SOFTWARE_CONTINUOUS_PRESENCE) ? YES : NO;
		  AcceptSipCallIn(pNetSetup, pComConf, pConfParty, bIsSoftCp, pSdpAndHeaders);
  	}
	break;
        }
      }
    } // switch
  }
}

//--------------------------------------------------------------------------
void CLobby::SubProcessConfNotFound(CSipNetSetup *pNetSetup,
	sipSdpAndHeadersSt *pSdpAndHeaders, BOOL useTransitEQOnly)
{
      TRACEINTO << "Incoming call rejected ";

      if (useTransitEQOnly == TRUE)
        TRACEINTO << "Incoming call rejected, failed to connect to Transit EQ";
      RejectSipCall(pNetSetup, int(cmReasonTypeNoPermision), pSdpAndHeaders);
}

//--------------------------------------------------------------------------
void CLobby::SubProcessConfFoundPartyNotFound(CCommConf *pComConf,
	CSipNetSetup *pNetSetup, sipSdpAndHeadersSt *pSdpAndHeaders, APIU32 csServiceId)
{
  DWORD confMonitorId   = pComConf ? pComConf->GetMonitorConfId() : 0;
  char* confName = pComConf ? (char*)pComConf->GetName() : NULL;

  BOOL partyHasSdp = IsSdpPresent(pSdpAndHeaders);
  // vngr-25287
  BOOL partyHasSDES = IsSdesDeclaredInSdp(pSdpAndHeaders);

  CConfIpParameters* pService = ::GetIpServiceListMngr()->FindIpService(csServiceId);

  TRACEINTO << "Incoming UnReserved Call "
            << "- confMonitorId:"   << confMonitorId
            << ", ConfName:" << confName;
  //BRIDGE-14921: Reject the incoming RDP Gw if content protocol is incompatible
  if(IsRdpGwRejected(partyHasSdp, pNetSetup, pComConf, pSdpAndHeaders))
  {
	  TRACEINTO << "Incompatible remote SDP! (call rejected)";
	  return;
  }

  //eFeatureRssDialin
  //If there's Rss in the conference already, reject the incoming one
  if( IsRssDialinRejected(pNetSetup, pComConf, pSdpAndHeaders))
  {
 	TRACEINTO << "Failed, There's already recording/playback link in the conf! (call rejected)";
	return;
  }

  UpdateInitialEncryptionForParty(pNetSetup, pService, partyHasSdp, partyHasSDES, pComConf);
  SuspendSipCallConfExist(pNetSetup, pSdpAndHeaders, confMonitorId,
  	pNetSetup->GetSrcPartyAddress(), pNetSetup->GetDestPartyAddress(), confName);
}
//--------------------------------------------------------------------------
void CLobby::SubProcessConfFoundMSPartyNotFound(CCommConf *pComConf,
		CSipNetSetup *pNetSetup, sipSdpAndHeadersSt *pSdpAndHeaders, APIU32 csServiceId)
{

	DWORD confId   = pComConf ? pComConf->GetMonitorConfId() : 0;
	  char* confName = pComConf ? (char*)pComConf->GetName() : NULL;

	  CConfIpParameters* pService = ::GetIpServiceListMngr()->FindIpService(csServiceId);

	 TRACEINTO << "Incoming UnReserved Call "
	           << "- ConfId:"   << confId
	           << ", ConfName:" << confName;


	 SuspendSipCallConfExist(pNetSetup, pSdpAndHeaders, confId,
			  	pNetSetup->GetSrcPartyAddress(), pNetSetup->GetDestPartyAddress(), confName);

	// CConfPartyManagerLocalApi* pConfPartyMngrApi = (CConfPartyManagerLocalApi*)CProcessBase::GetProcess()->GetManagerApi();
	// int SdpAndHeadersLen = sizeof(sipSdpAndHeadersBaseSt) + pSdpAndHeaders.lenOfDynamicSection;
	// mcTransportAddress* pDestIP = FindDestIp(pNetSetup);
	// pConfPartyMngrApi->AcceptAVMCUInvitation(pDestIP, confID,pSrcAddr,pSdpAndHeaders,SdpAndHeadersLen);
	// PDELETE(pDestIP);


}
//--------------------------------------------------------------------------
void CLobby::SubProcessConfLocked(CSipNetSetup *pNetSetup,
	sipSdpAndHeadersSt *pSdpAndHeaders)
{
      TRACEINTO << "Failed, Conference is locked at Dial-In (call rejected)";
      RejectSipCall(pNetSetup, int(cmReasonTypeNoPermision), pSdpAndHeaders);
}

//--------------------------------------------------------------------------
void CLobby::SubProcessMaxPartiesReached(CSipNetSetup *pNetSetup,
	sipSdpAndHeadersSt *pSdpAndHeaders)
{
      TRACEINTO << "Failed, Max parties number in conference has been reached at Dial-In (call rejected)";
      RejectSipCall(pNetSetup, int(cmReasonTypeNoBandwidth), pSdpAndHeaders);
}

//--------------------------------------------------------------------------
void CLobby::SubProcessConfSecured(CSipNetSetup *pNetSetup,
	sipSdpAndHeadersSt *pSdpAndHeaders)
{
      TRACEINTO << "Failed, Conference is secured at Dial-In (call rejected)";
      RejectSipCall(pNetSetup, int(cmReasonTypeNoPermision), pSdpAndHeaders);
}

//--------------------------------------------------------------------------
void CLobby::SubProcessPlcmIVRReject(CSipNetSetup *pNetSetup,
	sipSdpAndHeadersSt *pSdpAndHeaders)
{
      TRACEINTO << "Failed, PLCM IVR reject at Dial-In (call rejected)";
      RejectSipCall(pNetSetup, int(cmReasonTypeNoPermision), pSdpAndHeaders);
}

//--------------------------------------------------------------------------
void CLobby::SubProcessMeetingRoomFound(sipSdpAndHeadersSt *pSdpAndHeaders,
	int numOfDestAlias, CH323Alias *pPartyDstAliasList,
	DWORD meetingRoomId, CSipNetSetup *pNetSetup, APIU32 csServiceId)
{
  BOOL partyHasSdp = IsSdpPresent(pSdpAndHeaders);
  // vngr-25287
  BOOL partyHasSDES = IsSdesDeclaredInSdp(pSdpAndHeaders);

  CConfIpParameters* pService = ::GetIpServiceListMngr()->FindIpService(csServiceId);

  const char *strSrcPartyAddress = pNetSetup->GetSrcPartyAddress();
//  const char *strDstPartyAddress = pNetSetup->GetDestPartyAddress();
  const char *strDstPartyAddress = pNetSetup->GetToPartyAddress();

      TRACEINTO << "Destination MR found (unreserved party)";
      // meeting room
      char pMeetingRoomConfName[H243_NAME_LEN] = { 0 };
      if (pPartyDstAliasList)
        ::GetpMeetingRoomDB()->GetMeetingRoomForH323Call(pPartyDstAliasList, numOfDestAlias, pMeetingRoomConfName, meetingRoomId, FALSE, FALSE);

      if (YES == TargetReceptionAlreadyExist((char*)pMeetingRoomConfName))
        SuspendSipCall(pNetSetup, pSdpAndHeaders, meetingRoomId,
              strSrcPartyAddress, strDstPartyAddress,
              pMeetingRoomConfName);
      // When conf is UP those Receptions will be removed from list
      else // This is the party to awake the MR
      {
        // meeting room or GW
        CCommRes* pMrRes = ::GetpMeetingRoomDB()->GetCurrentRsrv(meetingRoomId);
        UpdateInitialEncryptionForParty(pNetSetup, pService, partyHasSdp, partyHasSDES, pMrRes);
        if ((pMrRes && pMrRes->GetIsGateway()) || strcmp(pMeetingRoomConfName, "GateWayTest") == 0)
        {
          char dialString[MaxAddressListSize];
          memset(dialString, 0, sizeof(dialString));

          const char* pReadPtr1 = strchr(strDstPartyAddress, '*'); // ::GetpGW_Delimiters()->GetDialUpNumbersDelimiter());
          if (pReadPtr1)
          {
            const char* pReadPtr2 = strchr(pReadPtr1, '@');
            int   len = pReadPtr2 ? pReadPtr2-pReadPtr1-1 : strlen(pReadPtr1);
            if (len > (MaxAddressListSize - 1))
            {
              len = MaxAddressListSize - 1;
            }
            strncpy(dialString, pReadPtr1+1, len);
            dialString[len] = '\0';
          }
          SuspendSipCallAndCreateGateWayConf(pNetSetup, pSdpAndHeaders, meetingRoomId, strSrcPartyAddress, strDstPartyAddress, pMeetingRoomConfName, dialString);
        }
        else
        {
          SuspendSipCallAndCreateMeetingRoom(pNetSetup, pSdpAndHeaders, meetingRoomId, strSrcPartyAddress, strDstPartyAddress, pMeetingRoomConfName);
        }
        POBJDELETE(pMrRes);
      }
}
//--------------------------------------------------------------------------
void CLobby::SubProcessMeetingRoomFoundForMsParty(sipSdpAndHeadersSt *pSdpAndHeaders,
	int numOfDestAlias, CH323Alias *pPartyDstAliasList,
	DWORD meetingRoomId, CSipNetSetup *pNetSetup, APIU32 csServiceId)
{

 // CConfIpParameters* pService = ::GetIpServiceListMngr()->FindIpService(csServiceId);

  const char *strSrcPartyAddress = pNetSetup->GetSrcPartyAddress();
//  const char *strDstPartyAddress = pNetSetup->GetDestPartyAddress();
  const char *strDstPartyAddress = pNetSetup->GetToPartyAddress();

      TRACEINTO << "Destination MR found (unreserved party)";

      // meeting room
      char pMeetingRoomConfName[H243_NAME_LEN] = { 0 };
      if (pPartyDstAliasList)
        ::GetpMeetingRoomDB()->GetMeetingRoomForH323Call(pPartyDstAliasList, numOfDestAlias, pMeetingRoomConfName, meetingRoomId, FALSE, FALSE);

      if (YES == TargetReceptionAlreadyExist((char*)pMeetingRoomConfName))
        SuspendSipCall(pNetSetup, pSdpAndHeaders, meetingRoomId,
              strSrcPartyAddress, strDstPartyAddress,
              pMeetingRoomConfName);
      // When conf is UP those Receptions will be removed from list
      else // This is the party to awake the MR
      {
        // meeting room or GW
        CCommRes* pMrRes = ::GetpMeetingRoomDB()->GetCurrentRsrv(meetingRoomId);
      //  UpdateInitialEncryptionForParty(pNetSetup, pService, partyHasSdp, partyHasSDES, pMrRes);
   /*     if ((pMrRes && pMrRes->GetIsGateway()) || strcmp(pMeetingRoomConfName, "GateWayTest") == 0)
        {
          char dialString[MaxAddressListSize];
          memset(dialString, 0, sizeof(dialString));

          const char* pReadPtr1 = strchr(strDstPartyAddress, '*'); // ::GetpGW_Delimiters()->GetDialUpNumbersDelimiter());
          if (pReadPtr1)
          {
            const char* pReadPtr2 = strchr(pReadPtr1, '@');
            int   len = pReadPtr2 ? pReadPtr2-pReadPtr1-1 : strlen(pReadPtr1);
            if (len > (MaxAddressListSize - 1))
            {
              len = MaxAddressListSize - 1;
            }
            strncpy(dialString, pReadPtr1+1, len);
            dialString[len] = '\0';
          }
          SuspendSipCallAndCreateGateWayConf(pNetSetup, pSdpAndHeaders, meetingRoomId, strSrcPartyAddress, strDstPartyAddress, pMeetingRoomConfName, dialString);
        }
        else
   */ //    {
          SuspendSipCallAndCreateMeetingRoom(pNetSetup, pSdpAndHeaders, meetingRoomId, strSrcPartyAddress, strDstPartyAddress, pMeetingRoomConfName);
     //   }
        POBJDELETE(pMrRes);
      }
}
//--------------------------------------------------------------------------
void CLobby::SubProcessSipFactory(CH323Alias *pPartyDstAliasList,
	CSipNetSetup *pNetSetup, sipSdpAndHeadersSt *pSdpAndHeaders)
{
  const char *strSrcPartyAddress = pNetSetup->GetSrcPartyAddress();
  const char *strDstPartyAddress = pNetSetup->GetDestPartyAddress();

  TRACEINTO << "SIP Factory call";
  CCommRes* pFactory = ::GetpMeetingRoomDB()->GetCurrentRsrv(pPartyDstAliasList->GetAliasName());
  if (!IsValidPObjectPtr(pFactory))
    return;

  //eFeatureRssDialin
  if(true == RejectRssDialin(pNetSetup,pSdpAndHeaders))
	return;

  SuspendSipCallAndCreateConfFromFactory(pNetSetup, strSrcPartyAddress, strDstPartyAddress, pSdpAndHeaders, pFactory);
  POBJDELETE(pFactory);
}

//--------------------------------------------------------------------------
void CLobby::SubProcessSuspendMsAdhoc(char *strMsAdhocName,
	CSipNetSetup *pNetSetup, sipSdpAndHeadersSt *pSdpAndHeaders)
{
      if (strcmp(strMsAdhocName, ""))
      {
        TRACEINTO << "Suspend until adhoc will be on air, Name:" << strMsAdhocName;
        SuspendSipCall(pNetSetup, pSdpAndHeaders, -1,
			pNetSetup->GetSrcPartyAddress(),
			pNetSetup->GetDestPartyAddress(),
			(char*)strMsAdhocName);
      }
      else
        DBGPASSERT(YES);
}

//--------------------------------------------------------------------------
void CLobby::SubProcessConfNotFoundForMsAdhoc(CSipNetSetup *pNetSetup,
	sipSdpAndHeadersSt *pSdpAndHeaders)
{
      TRACEINTO << "Failed, No conference was found for ms adhoc call (call rejected)";
      RejectSipCall(pNetSetup, int(cmReasonTypeNoPermision), pSdpAndHeaders);
}

//--------------------------------------------------------------------------
void CLobby::SubProcessDefault(CSipNetSetup *pNetSetup,
	sipSdpAndHeadersSt *pSdpAndHeaders, eFindConfAndPartyResult result)
{
      TRACEINTO << "Failed, Unspecified error (call rejected), Error:" << result;
      RejectSipCall(pNetSetup, int(cmReasonTypeNoPermision), pSdpAndHeaders);
}

//--------------------------------------------------------------------------
void CLobby::ProcessConfAndPartyFindResult(CSipInviteStruct *pSipInvite,
	CSipNetSetup *pNetSetup,
	findConfAndPartyKeysSt keys,
	findConfAndPartyResultSt &result)
{
  sipSdpAndHeadersSt *pSdpAndHeaders = pSipInvite->GetSdpAndHeaders();

  BOOL bAccept = IsServiceConfiguredAcceptCall(pSipInvite->GetCsServiceId(),
	result.pComConf, result.meetingRoomId, result.eResult, keys.pPartyDstAliasList);

  if (bAccept == FALSE)
  {
    RejectSipCall(pNetSetup, int(cmReasonTypeNoPermision), pSdpAndHeaders);

	return;
  }
  if(true == IsRssDialinRejected( pNetSetup, pSdpAndHeaders))
  {
  	//RejectSipCall() already in last function
	return ;
  }

  BOOL ivrNotSupported = IsIvrNotSupported(pSipInvite, result.pComConf,
  	result.meetingRoomId, result.eResult);

  if (ivrNotSupported == TRUE)
  {
  	result.eResult = eFindConfAndParty_PLCM_IVR_REJECT;
  }

  switch (result.eResult)
  {
    case eFindConfAndParty_OK:
    {
      SubProcessConfAndPartyFound(result.pComConf, result.pConfParty, pNetSetup, pSdpAndHeaders);
      break;
    }

    case eFindConfAndParty_CONF_NOT_FOUND:  // REJECT THE CALL UNABLE TO FIND THE CONF AND/OR THE PARTY
    {
	  SubProcessConfNotFound(pNetSetup, pSdpAndHeaders, result.useTransitEQOnly);
      break;
    }

    case eFindConfAndParty_ONLY_CONF_FOUND:  // CONFERENCE FOUND BUT PARTY NOT FOUND (UNRESERVED PARTY)
    {         // unreserved call (returned value at Undefined and Advance Dial In)
    	if(!pNetSetup->GetIsMsConfInvite())
       	{
    		SubProcessConfFoundPartyNotFound(result.pComConf, pNetSetup, pSdpAndHeaders,
    		pSipInvite->GetCsServiceId());
    	}
    	else
    	{
    		SubProcessConfFoundMSPartyNotFound(result.pComConf, pNetSetup, pSdpAndHeaders,
    		 pSipInvite->GetCsServiceId());
    	}
      break;
    }

    case eFindConfAndParty_CONF_IS_LOCKED:    // conference is locked
    {
	  SubProcessConfLocked(pNetSetup, pSdpAndHeaders);
      break;
    }

    case eFindConfAndParty_MAX_PARTIES_REACHED:    // max parties number in conference has been reached
    {
	  SubProcessMaxPartiesReached(pNetSetup, pSdpAndHeaders);
      break;
    }

    case eFindConfAndParty_CONF_IS_SECURED:    // conference is secured
    {
	  SubProcessConfSecured(pNetSetup, pSdpAndHeaders);
      break;
    }

    case eFindConfAndParty_PLCM_IVR_REJECT: // plcm ivr reject
    {
	  SubProcessPlcmIVRReject(pNetSetup, pSdpAndHeaders);
      break;
    }

    case eFindConfAndParty_MEETING_ROOM_FOUND:
    {
    	if(!pNetSetup->GetIsMsConfInvite())
    	{
		  SubProcessMeetingRoomFound(pSdpAndHeaders, keys.numOfDestAlias,
			keys.pPartyDstAliasList, result.meetingRoomId, pNetSetup,
			pSipInvite->GetCsServiceId());
    	}
    	else
    	{
    		SubProcessMeetingRoomFoundForMsParty(pSdpAndHeaders, keys.numOfDestAlias,
    				keys.pPartyDstAliasList, result.meetingRoomId, pNetSetup,
    				pSipInvite->GetCsServiceId());
    	}

      break;
    }

    case eFindConfAndParty_SIP_FACTORY_FOUND: // if call is to SIP factory
    {
	  SubProcessSipFactory(keys.pPartyDstAliasList, pNetSetup, pSdpAndHeaders);
      break;
    }

    case eFindConfAndParty_NOT_A_SIP_SERVICE: // Not a SIP service error
    {
      break;
    }

    case eFindConfAndParty_ON_AIR_MS_ADHOC_CONF_FOUND:  // suspend for ms adhoc conference to be on air
    {
	  SubProcessSuspendMsAdhoc(result.strMsAdhocName, pNetSetup, pSdpAndHeaders);
      break;
    }

    case eFindConfAndParty_MS_ADHOC_CONF_NOT_FOUND:  // No conference was found for ms adhoc call.
    {
	  SubProcessConfNotFoundForMsAdhoc(pNetSetup, pSdpAndHeaders);
      break;
    }

    default:// REJECT THE CALL UNABLE TO FIND THE CONF AND/OR THE PARTY
    {
	  SubProcessDefault(pNetSetup, pSdpAndHeaders, result.eResult);
      break;
    }
  } // end switch
}

/*
 Function name: BuildSipInviteStruct
 Variables:     pCSParam: Message from CS
                     pSipInvite: SIP invite struct C++ instance used by MCMS

 Description:   Build SIP invite struct C++ instance from CS message
*/
//--------------------------------------------------------------------------
STATUS CLobby::BuildSipInviteStruct(CSegment* pCSParam,	CSipInviteStruct *pSipInvite)
{
  APIU32 csServiceId      = 0;
  APIU32 callIndex        = 0;
  APIU32 paramNotUsed     = 0;

  *pCSParam >> callIndex >> paramNotUsed >> paramNotUsed >> paramNotUsed
    >> paramNotUsed >> csServiceId;

  // BRIDGE-18254
  TRACEINTO << "callIndex: " << callIndex << ", csServiceId: " << csServiceId;

  pSipInvite->SetCallIndex(callIndex);
  pSipInvite->SetCsServiceId(csServiceId);

  DWORD nMsgLen =  pCSParam->GetWrtOffset() - pCSParam->GetRdOffset();
  if (!nMsgLen)
    return STATUS_FAIL;

  mcIndInvite* pInviteMsg = (mcIndInvite*)pCSParam->GetPtr(TRUE);

  // MFW Core FSN-671, BRIDGE-18254
  if ( ((sipMediaLinesEntrySt*)(pInviteMsg->sipSdpAndHeaders.capsAndHeaders))->numberOfMediaLines > 1000 )
  {
	  PASSERT(((sipMediaLinesEntrySt*)(pInviteMsg->sipSdpAndHeaders.capsAndHeaders))->numberOfMediaLines > 1000);
	  return STATUS_FAIL;
  }


  pSipInvite->ReadInviteInd(pInviteMsg);
  if (!pSipInvite->GetSdpAndHeaders())
  {
	  TRACEINTO << "CLobby::BuildSipInviteStruct - Failed to allocate SdpAndHeaders !!! return error";
	  return STATUS_FAIL;
  }

  return STATUS_OK;
}

/*
 Function name: CallFlowDecision
 Variables:     pSipInvite: SIP invite struct C++ instance
                     pNetSetup: SIP net setup instance used by call flow decision

 Description:   process according to different cases
*/
//--------------------------------------------------------------------------
void CLobby::CallFlowDecision(CSipInviteStruct *pSipInvite,CSipNetSetup *pNetSetup)
{
  sipSdpAndHeadersSt *pSdpAndHeaders = pSipInvite->GetSdpAndHeaders();

  // For HotBackup validity
  CProcessBase* pConfPartyProcess = CProcessBase::GetProcess();
  if (TRUE == pConfPartyProcess->GetIsFailoverSlaveMode())
  {
    TRACEINTO << "RMX is in Slave MODE (call rejected)";
    RejectSipCall(pNetSetup, int(cmReasonTypeNoPermision), pSdpAndHeaders);
    return;
  }

  // For WebRtc
  if (::IsWebRtcCall((sipSdpAndHeaders *)pSdpAndHeaders) && !::IsWebRtcCallSupported())
  {
    TRACEINTO << "WebRtc call not supported (call rejected)";
    RejectSipCall(pNetSetup, int(cmReasonTypeNoPermision), pSdpAndHeaders);
    return;
  }

  // MFW: Reject none DMA VMR calls
  if (CProcessBase::GetProcess()->GetProductType() == eProductTypeSoftMCUMfw)
  {
	  BOOL bDMACall = GetSystemCfgFlagInt<BOOL>("CALLS_ONLY_FROM_DMA");
	  if (bDMACall == YES)
	  {
		  const char *pRemoteSipContact = pNetSetup->GetRemoteSipContact();
		  if (pRemoteSipContact && !strstr(pRemoteSipContact, ".DMA_VMR."))
		  {
			TRACEINTO << "MFW: None DMA VMR call (call rejected)";
			RejectSipCall(pNetSetup, int(cmReasonTypeNoPermision), pSdpAndHeaders);
			return;
		  }
	  }
  }

  // extract keys to find the target conference and party
  findConfAndPartyKeysSt keys;

  AllocateAndExtractFindConfAndPartyKeys(pSipInvite, pNetSetup, keys);

  findConfAndPartyResultSt result;

  // find the target conference and party
  FindMatchingConfAndParty(keys, pNetSetup, result);

  // process according to the find result
  ProcessConfAndPartyFindResult(pSipInvite, pNetSetup, keys, result);

  PDELETE(keys.pPartySrcAliasList);
  PDELETEA(keys.pPartyDstAliasList);
  PDELETEA(keys.pAdHocConfName);
  PDELETEA(keys.pAdHocNID);
}

//////////////////////////////////////////////////////////////////////////////
//eFeatureRssDialin
BOOL    CLobby::IsRssDialinRejected(CSipNetSetup* pNetSetup, CCommConf* pComConf, const sipSdpAndHeadersSt* pSdpAndHeaders)
{
	if(!pComConf || !pSdpAndHeaders || !pNetSetup)
	{
		TRACEINTO << "error - Invalid parameters!";
		return false;
	}


	enSrsSessionType  srsSessionType = ::getSrsSessionType(pSdpAndHeaders) ;
	switch(srsSessionType)
	{
		case eSrsSessionTypeRecording:
		{
			//If the recording is not enabled, reject the RSS dialin
			if(false == pComConf->GetEnableRecording())
			{
				TRACEINTO << "Failed, Recording is not enabled (call rejected)";
				RejectSipCall(pNetSetup, int(cmReasonTypeNoPermision), pSdpAndHeaders);
				return true;
			}

			//If there's Recording seesion already, reject this new one
			if(pComConf->IncludeRecordingParty())
			{
				//This is a incoming RSS, and there's already a RSS in the conf, reject the new one!

				TRACEINTO << "Failed, There's already a recording link in Conf (call rejected)";
					//NoBandwidth == 486 Busy here
				RejectSipCall(pNetSetup, int(cmReasonTypeNoBandwidth), pSdpAndHeaders);
				return true;
			}
			break;
		}
		case eSrsSessionTypePlayback:
		{
			//If there's Playback seesion already, reject this new one
			if(pComConf->IncludePlaybackParty())
			{
				TRACEINTO << "Failed, There's already a playback link in Conf (call rejected)";
					//NoBandwidth == 486 Busy here
				RejectSipCall(pNetSetup, int(cmReasonTypeNoBandwidth), pSdpAndHeaders);
				return true;
			}
			break;
		}
		case eSrsSessionTypeRegular:
		// no break
		default:
		{
			return false;
			//break;
		}

	}

	return false;

}
//////////////////////////////////////////////////////////////////////////////
//eFeatureRssDialin  -- RSS/SRS is not allowed to dial into a MCU without this feature
BOOL    CLobby::IsRssDialinRejected(CSipNetSetup* pNetSetup, const sipSdpAndHeadersSt* pSdpAndHeaders)
{
	if(!pNetSetup || !pSdpAndHeaders)
	{
		TRACEINTO << "error - Invalid parameters!";
		return false;
	}

	if( (eSrsSessionTypeRegular !=  ::getSrsSessionType(pSdpAndHeaders))&&( !IsFeatureSupportedBySystem(eFeatureRssDialin)))
	{
		TRACEINTO << "Failed, The Recording dialin is not supported (call rejected)";
		RejectSipCall(pNetSetup, int(cmReasonTypeNoPermision), pSdpAndHeaders);
		return true;
	}

	return false;

}

//////////////////////////////////////////////////////////////////////////////
//eFeatureRssDialin
BOOL    CLobby::RejectRssDialin(CSipNetSetup* pNetSetup, sipSdpAndHeadersSt* pSdpAndHeaders)
{
	if(!pSdpAndHeaders || !pNetSetup)
	{
		TRACEINTO << "error - Invalid parameters!";
		return false;
	}

	if((eSrsSessionTypeRecording ==  ::getSrsSessionType(pSdpAndHeaders))
		||(eSrsSessionTypePlayback==  ::getSrsSessionType(pSdpAndHeaders)))
	{
		//This is a incoming RSS,  reject it!
          	TRACEINTO << "Rss cannot dial into a EQ nor SipFactory (as the first Party)! (call rejected)";
		RejectSipCall(pNetSetup, int(cmReasonTypeNoPermision), pSdpAndHeaders);
		return true;
	}
	return false;

}
BOOL    CLobby::IsRdpGwRejected(BOOL hasSdp, CSipNetSetup* pNetSetup, CCommConf* pComConf, const sipSdpAndHeadersSt* pSdpAndHeaders)
{
	BOOL 	isRDPGw = FALSE;
	
	if(!pComConf || !pSdpAndHeaders || !pNetSetup)
	{
		TRACEINTO << "error - Invalid parameters!";
		return FALSE;
	}
	
	sipMessageHeaders *pHeaders = (sipMessageHeaders *) &pSdpAndHeaders->capsAndHeaders[pSdpAndHeaders->sipHeadersOffset];	
	if (!pHeaders)
	{	
		TRACEINTO << "  invalid SDP headers" ;
		return FALSE;
	}
	
	char cUAName[MaxUserAgentSize] = {0};
	::SipGetHeaderValue(pHeaders, kUserAgent, cUAName,MaxUserAgentSize);	
	if(strlen(cUAName) > 0)
	{
		if (NULL != strstr(cUAName, "Polycom CSS Lync Gateway") ) 
		{
			TRACEINTO << "  RDP Gateway dialing in!" ;			
			isRDPGw = TRUE;
		}
	}
	if (!isRDPGw)
	{
		return false;
	}
	
	if(!hasSdp)
	{
		//If there's no SDP, reject it with 488 Not Acceptable Here
		TRACEINTO << "No SDP present in RDP Gw's INVITE, reject it!";
		RejectSipCall(pNetSetup, int(SipCodesNotAcceptedInHere), pSdpAndHeaders);
		return true;
	}
	else
	{
		BOOL	isCommonCapFound = FALSE;
		ePresentationProtocol contentProtocolMode = (ePresentationProtocol)pComConf->GetPresentationProtocol();
		if(eH264Fix == contentProtocolMode || eH263Fix == contentProtocolMode ||eH264Dynamic ==contentProtocolMode)
		{
			CapEnum contentProtocol = (eH263Fix == contentProtocolMode) ? eH263CapCode : eH264CapCode;
			sipMediaLinesEntrySt* pMediaLinesEntry = (sipMediaLinesEntrySt*)pSdpAndHeaders->capsAndHeaders;
			int mediaLinePos = 0;
			
			for (unsigned int j = 0; j < pMediaLinesEntry->numberOfMediaLines; j++) 
			{			
				const sipMediaLineSt *pMediaLine = (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];
				const capBuffer* pCapBuffer = (capBuffer*) &pMediaLine->caps[0];
				const BYTE* pTemp = (const BYTE*)pCapBuffer;
				mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;

				if((eMediaLineType) pMediaLine->type == eMediaLineTypeVideo 
					&& (eMediaLineContent)pMediaLine->content == eMediaLineContentSlides)
				{
					for (unsigned int i = 0 ; i < pMediaLine->numberOfCaps; i++)
					{
						if ((CapEnum)pCapBuffer->capTypeCode == contentProtocol)
						{
							//FPTRACE(eLevelInfoNormal," CLobby::IsRdpGwRejected - Found common content protocol ");
							isCommonCapFound = TRUE;
							break;
						}
					
						pTemp += sizeof(capBufferBase) + pCapBuffer->capLength;
						pCapBuffer = (capBuffer*)pTemp;
					}
				}
				if(isCommonCapFound == TRUE)
				{
					break;
				}
			}
			if(!isCommonCapFound)
			{
				//TRACEINTO << "No common content protocol present in RDP Gw's INVITE, reject it!";
				RejectSipCall(pNetSetup, int(SipCodesNotAcceptedInHere), pSdpAndHeaders);
				return true;
			}
		}
	}
	
	return false;

}



