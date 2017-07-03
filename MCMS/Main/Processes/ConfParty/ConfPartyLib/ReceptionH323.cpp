// ReceptionH323.cpp: implementation of the CReceptionH323 class.
//
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//Revisions and Updates:
//
//Date         Updated By         Description
//
//17/7/05		Yoella				Porting to Carmel
//========   ==============   =====================================================================

#include "ReceptionH323.h"
#include "Lobby.h"
#include "ConfPartyManagerLocalApi.h"
#include "H323PartyIn.h"
#include "H323PartyInSimulation.h"
#include "ProcessBase.h"
#include "IpServiceListManager.h"
#include "SysConfig.h"
#include "ManagerApi.h"
#include "RsrvParty.h"

//~~~~~~~~~~~~~~ Global functions ~~~~~~~~~~~~~~~~~~~~~~~~~~
extern CIpServiceListManager* GetIpServiceListMngr();


/////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//                            CReceptionH323
////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CReceptionH323)

  //events from conference
  ONEVENT(RELEASE_UNRESERVED_PARTY,SUSPEND  ,CReceptionH323::OnLobbyReleasePartySuspend)

  // events from conferance manager
  //ONEVENT(REJECT_UNRESERVED_PARTY, SUSPEND  ,CReceptionH323::OnConfMngrRejectPartySuspend)

  //local events
  ONEVENT(PARTYSUSPENDTOUT,		SUSPEND  ,CReceptionH323::OnTimerPartySuspend)

PEND_MESSAGE_MAP(CReceptionH323,CReception);

/////////////////////////////////////////////////////////////////////////////
CReceptionH323::CReceptionH323(CTaskApp *pOwnerTask)
        :CReception(pOwnerTask)
{
	m_pConfParty = NULL;

	VALIDATEMESSAGEMAP;
}
/////////////////////////////////////////////////////////////////////////////
CReceptionH323::~CReceptionH323()
{

}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void  CReceptionH323::CreateH323(DWORD monitorConfId,DWORD monitorPartyId,COsQueue* pConfRcvMbx, //shiraITP - 52
                          COsQueue* pLobbyRcvMbx,CH323NetSetup* pNetSetup,
						  CLobby* pLobby, char* pPartyName,const char* confName, CConfParty* pConfParty,
						  CCommConf* pComConf/*, WORD isChairEnabled ,BYTE IsGateWay, */)
{

	m_pLobby	 	= pLobby;
	m_pConfParty 	= pConfParty;
	m_confMonitorId = monitorConfId;
	SetConfOnAirFlag(YES);

	//ATTENTION: Here, we use conference name in order to find conference id.
	//This should be corrected and we should base on the parameter DWORD confId.
	//FAILING IN THE PROCESS OF ALLOCATION OF ONE OR MORE RESOURCE HANDLE EXCEPTION

	DWORD confRate = 0;

	BYTE networkType = pComConf->GetNetwork();
	DWORD setupRate = pNetSetup->GetRemoteSetupRate();

	WORD voice_type = pConfParty->GetVoice();
    if 	(voice_type == YES) voice_type = U_LAW;

	void (*entryPoint)(void*);
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	BOOL bIsH323PartyLevelSimulation = 0;
	std::string key = "H323_PARTY_LEVEL_SIMULATION";
	pSysConfig->GetBOOLDataByKey(key, bIsH323PartyLevelSimulation);

	if(bIsH323PartyLevelSimulation)
		entryPoint =  H323PartyInSimulationEntryPoint;// for party control level simulation
	else
		entryPoint = H323PartyInEntryPoint;

	/*const char* serviceName  = pConfParty->GetServiceProviderName();//"sip and 323";//
	CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
	CConfIpParameters* pServiceParams = pIpServiceListManager->GetRelevantService(serviceName, pConfParty->GetNetInterfaceType());
	WORD serviceId;
	if (pServiceParams != NULL)
	{
		//PASSERTMSG(monitorPartyId,"CReceptionH323::CreateH323 - No IP Service exsists!!!");
		//return;
		PTRACE(eLevelInfoNormal,"CReceptionH323::CreateH323 : ServiceId from Service name");
		pConfParty->SetServiceProviderName((const char*)(pServiceParams->GetServiceName ()));
		serviceId  = (WORD)pServiceParams->GetServiceId();
	}
	else
	{
		PTRACE(eLevelInfoNormal,"CReceptionH323::CreateH323 : ServiceId from CS");
		serviceId  = (WORD)pNetSetup->GetCsServiceid();
	        pServiceParams = pIpServiceListManager->FindIpService(serviceId);
	        if (pServiceParams != NULL)
	        {
		   PASSERTMSG(monitorPartyId,"CReceptionH323::CreateH323 - No IP Service exsists!!!");
		   return;
		}
		pConfParty->SetServiceProviderName((const char*)(pServiceParams->GetServiceName ()));
	}*/
	WORD serviceId = (WORD)pNetSetup->GetCsServiceid();
	PTRACE2INT(eLevelInfoNormal,"CReceptionH323::CreateH323 : ServiceId from CS ", serviceId);
	CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
	CConfIpParameters* pServiceParams = pIpServiceListManager->FindIpService(serviceId);
	if (pServiceParams == NULL)
	{
	   const char* serviceName  = pConfParty->GetServiceProviderName();//"sip and 323";//
	   PTRACE2(eLevelInfoNormal,"CReceptionH323::CreateH323 : ServiceId from Service name ", serviceName);
	   pServiceParams = pIpServiceListManager->GetRelevantService(serviceName, pConfParty->GetNetInterfaceType());
	   if (pServiceParams == NULL)
	   {
	      PASSERTMSG(monitorPartyId,"CReceptionH323::CreateH323 - No IP Service exists!!!");
	      return;
	   }
	   serviceId  = (WORD)pServiceParams->GetServiceId();
	}
	
	PTRACE2(eLevelInfoNormal,"CReceptionH323::CreateH323 : Service name ", (const char*)(pServiceParams->GetServiceName ()));
	pConfParty->SetServiceProviderName((const char*)(pServiceParams->GetServiceName ()));
	

	mcTransportAddress *pSrcTrAddr = const_cast<mcTransportAddress *>(pNetSetup->GetTaSrcPartyAddr());
	if( !::isApiTaNull(pSrcTrAddr))
	{
		BYTE  isMatch = ::isIpVersionMatchBetweenPartyAndService(pSrcTrAddr, pServiceParams);
		if (isMatch == FALSE)
		{
			PASSERTMSG(3,"CReceptionH323::CreateH323 - No match between service and EP ip types!!!");
		   	return;
		}
	}

	if (!pNetSetup->IsItPrimaryNetwork())
	{
		 pConfParty->SetSubServiceName(SUB_SERVICE_NAME_SECONDARY);
	}
	
	BYTE isAutoBitRate = 0;
	if (pConfParty->GetVideoRate() == 0XFFFFFFFF)
		ON(isAutoBitRate);

	m_pPartyApi = new CPartyApi;
	m_pPartyApi->Create(entryPoint,
	                    *pLobbyRcvMbx,
	                    *pConfRcvMbx,
	                    DEFAULT_PARTY_ID,
	                    monitorPartyId,
	                    monitorConfId,
	                    "",
	                    pPartyName,
	                    confName,
	                    serviceId,
	                    1,
	                    1,
	                    "ggg",
	                    voice_type,
	                    1,
	                    pComConf->GetIsGateway(),
	                    0,
	                    isAutoBitRate);

	m_pParty = m_pPartyApi->GetTaskAppPtr();    // set self identification

#ifdef LOOKUP_TABLE_DEBUG_TRACE
	TRACEINTO << "D.K."
			<< "  PartyName:"        << pPartyName
			<< ", PartyId:"          << m_pParty->GetPartyId() << "(0)"
			<< ", IsDialOut:"        << 0
			<< ", Pointer:"          << std::hex << m_pParty;
#endif

	PTRACE(eLevelInfoNormal,"CReceptionH323::CreateH323 : StartTimer(IDENTTOUT,IDENT_TOUT)");
	StartTimer(IDENTTOUT,IDENT_TOUT);

	BYTE *pAuthKey	= NULL;
	BYTE *pEncKey	= NULL;
	BYTE sid[2];
	int	 indexTblAuth	= 0;

	memset(sid,0,2);
		/*
		CReceptionLRQH323* pReception = m_pLobby->FindMatchingCreception(pNetSetup->GetCallId());
		if (pReception && (::isValidPObjectPtr(pReception)))
		{
			if(pReception->m_pDHKeyMngr && pReception->m_bIsAuthenticated)
			{
				pAuthKey= pReception->m_pDHKeyMngr->GetAuthCallKey()->GetArray();
				pEncKey	= pReception->m_pDHKeyMngr->GetEncrCallKey()->GetArray();
			}
			else
			{
				CSmallString trace_str;
				if(!pReception->m_pDHKeyMngr)
					trace_str << "m_pDHKeyMngr = NULL" ;
				trace_str << " m_bIsAuthenticated =  " << "pReception->m_bIsAuthenticated.\n";

				PTRACE2(eLevelInfoNormal,"CReceptionH323::CreateH323 - ",trace_str.GetString());
			}
			memcpy(sid,pReception->m_sid,2);
			indexTblAuth = pReception->m_pDHKeyMngr->GetIndexTblAuth();
		}*/

	DWORD authLen	= 0;
	DWORD encLen	= 0;
	if(pAuthKey)
		authLen = /*LengthEncAuthKey*/0;
	if(pEncKey)
		encLen = /*LengthEncAuthKey*/0;

	DWORD bIsEncrypted = 0;
	if(pConfParty->GetIsEncrypted()==YES || (pConfParty->GetIsEncrypted()==AUTO && pComConf->GetIsEncryption()))
		bIsEncrypted = 1;

	m_pPartyApi->IdentifyCall(pNetSetup, bIsEncrypted,indexTblAuth,sid,pAuthKey,pEncKey,authLen,encLen); //shiraITP - 53
}

/////////////////////////////////////////////////////////////////////////////
// Create for party reject in unattended call in.
void CReceptionH323::CreateH323(CH323NetSetup* pNetSetup, COsQueue* pLobbyRcvMbx, CLobby* pLobby, int reason, char* alternativeAlias)
{
	PTRACE(eLevelInfoNormal, "CReceptionH323::CreateH323 for Reject : ");

	m_pLobby = pLobby;

	CConfIpParameters* pServiceParams = ::GetIpServiceListMngr()->GetRelevantService(NULL, H323_INTERFACE_TYPE);
	PASSERT_AND_RETURN(!pServiceParams);

	WORD serviceId = (WORD)pServiceParams->GetServiceId();
	void (*entryPoint)(void*) = H323PartyInEntryPoint;

	m_pPartyApi = new CPartyApi;
	m_pPartyApi->Create(entryPoint,
	                    *pLobbyRcvMbx,
	                    *pLobbyRcvMbx,
	                    DEFAULT_PARTY_ID,
	                    0xFFFFFFFF,
	                    (DWORD)this,
	                    "",
	                    "UNATTENTED",
	                    "UNATTENTED",
	                    serviceId);

#ifdef LOOKUP_TABLE_DEBUG_TRACE
	m_pParty = m_pPartyApi->GetTaskAppPtr();
	TRACEINTO << "D.K."
	          << "  PartyName:"        << "UNATTENTED"
	          << ", PartyId:"          << m_pParty->GetPartyId() << "(0)"
	          << ", IsDialOut:"        << 0
	          << ", Pointer:"          << std::hex << m_pParty;
#endif

	m_pPartyApi->RejectH323Call(pNetSetup, reason);
	m_pPartyApi->Destroy();
}


/////////////////////////////////////////////////////////////////////////////
// Create for party suspend in unattended call in.
void  CReceptionH323::CreateSuspend(CH323NetSetup* pNetSetup,COsQueue* pLobbyRcvMbx,
                         CLobby* pLobby,CTaskApp* ListId,DWORD confID, char* targetConfName)
{
	PTRACE(eLevelInfoNormal,"CReceptionH323::CreateSuspend for suspend");

    WORD isVoice = 0;
	m_state  = SUSPEND;
	m_pLobby = pLobby;

	//this value (ListId) is not a valid memory pointer,
	//it is used only to identify the suspended CReception object in the waiting list
	SetParty(ListId);
    SetMonitorMRId(confID);
    SetConfOnAirFlag(NO);
    SetTargetConfName(targetConfName);

   	//copy H323 setup call parameters to local storage for suspend
	m_pSuspended_NetSetUp = new CH323NetSetup;
	m_pSuspended_NetSetUp->copy(pNetSetup);


	PTRACE(eLevelInfoNormal,"CReceptionH323::CreateSuspend : StartTimer(PARTYSUSPENDTOUT,30*SECOND)");
	//will remove this item from the list in 30 seconds if no response received from MNGR
	StartTimer(PARTYSUSPENDTOUT,30*SECOND);

}

/////////////////////////////////////////////////////////////////////////////
// Create suspend for party when conf is already Exist
void  CReceptionH323::CreateSuspendConfExist(CH323NetSetup* pNetSetup,COsQueue* pLobbyRcvMbx, //shiraITP - 42
                         CLobby* pLobby,CTaskApp* ListId,DWORD confID,char* confName)
{
	PTRACE(eLevelInfoNormal,"CReceptionH323::CreateSuspendConfExist CreateSuspend for Undefined party in Existing Conf");

	WORD isVoice = 0;
	CreateSuspend(pNetSetup,pLobbyRcvMbx,pLobby,ListId,confID,confName);
	SetConfOnAirFlag(YES);
	const mcTransportAddress* pDestIP = ((CH323NetSetup*)m_pSuspended_NetSetUp)->GetTaDestPartyAddr();

	CConfPartyManagerLocalApi confPartyMngrApi;
	confPartyMngrApi.AcceptUnreservedParty(pDestIP, confID, (DWORD)ListId, isVoice, NO, eNotSipFactory,"",
                                            AUTO,pNetSetup->GetH323userUser()); //shiraITP - 43 - CReceptionH323::CreateSuspendConfExist
}


/////////////////////////////////////////////////////////////////////////////
// Awake meeting room conference in unattented call in.
void  CReceptionH323::CreateMeetingRoom(CH323NetSetup* pNetSetup,COsQueue* pLobbyRcvMbx,
                         CLobby* pLobby, CTaskApp* ListId, DWORD mrID,char* mrName)
{
	PTRACE(eLevelInfoNormal,"CReceptionH323::AwakMeetingRoom : ");

	m_state  = SUSPEND;
	m_pLobby = pLobby;

	SetTargetConfName(mrName);

	//this value (ListId) is not a valid memory pointer,
	//it is used only to identify the suspended CReception object in the waiting list
	SetParty(ListId);
	SetMonitorMRId(mrID);
	SetConfOnAirFlag(NO);

	//copy H323 setup call parameters to local storage for suspend
	m_pSuspended_NetSetUp = new CH323NetSetup;
	m_pSuspended_NetSetUp->copy(pNetSetup);

	PTRACE(eLevelInfoNormal,"CReceptionH323::CreateMeetingRoom : StartTimer(PARTYSUSPENDTOUT,30*SECOND)");
	//will remove this item from the list in 30 seconds if no response recived from ConfParyMngr
	StartTimer(PARTYSUSPENDTOUT,30*SECOND);

	//send CREATE_MEETING_ROOM msg to ConfPartyManager
	CConfPartyManagerLocalApi confPartyMngrApi;
	confPartyMngrApi.StartMeetingRoom(mrID);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CReceptionH323::IsPartyDefined(DWORD confMonitorId)
{
	BOOL isDefined = NO;
	const mcTransportAddress* pPhoneLenOrIP = ((CH323NetSetup*)m_pSuspended_NetSetUp)->GetTaSrcPartyAddr();
	const char* aliasString  = ((CH323NetSetup*)m_pSuspended_NetSetUp)->GetH323PartyAlias();

	CCommRes* pMR = ::GetpMeetingRoomDB()->GetCurrentRsrv(m_confMonitorId);
	if(NULL != pMR)
	{
		int numOfSrcAlias;
		CH323Alias* PartySrcAliasList = ((CH323NetSetup*)m_pSuspended_NetSetUp)->GetSrcPartyAliasList(&numOfSrcAlias);
		
		isDefined = pMR->IsPartyDefined(pPhoneLenOrIP,aliasString, (DWORD&)m_pParty, H323_INTERFACE_TYPE,PartySrcAliasList,numOfSrcAlias);
		POBJDELETE(pMR);
		//delete [] PartySrcAliasList;
		PDELETEA(PartySrcAliasList);
	}
	else
	{
	    PTRACE(eLevelInfoNormal,"CReceptionH323::IsPartyDefined : MR NOT FOUND!!!");
	}

	return isDefined;
}
/////////////////////////////////////////////////////////////////////////////
void*  CReceptionH323::GetMessageMap()
{
  return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void  CReceptionH323::OnException()
{
	PTRACE(eLevelInfoNormal/*|LOBBY_TRACE*/,"CReceptionH323::OnException : ");
	PASSERT(!m_pLobby);

    DeleteAllTimers();

	if(IsValidPObjectPtr(m_pPartyApi))
	{
		STATUS r_status;
		r_status = m_pPartyApi->SendOpcodeMsg(LOBBYDESTROY);
//		m_pPartyApi->LobbyDestroy();
		PTRACE2INT(eLevelInfoNormal/*|LOBBY_TRACE*/,"CReceptionH323::OnException : return status - ",r_status);
		POBJDELETE(m_pPartyApi);
	}

	m_pLobby->OnEndPartyTransfer(this);
	delete this;
}

/////////////////////////////////////////////////////////////////////////////
void  CReceptionH323::GetH323CallParams(CH323NetSetup** pNetSetup/*, CIpRsrcDesc** pIpDesc*/)
{
	//create a copy of the suspended H323 parameters and return them to caller
	if (m_pSuspended_NetSetUp)
	{
		*pNetSetup = new CH323NetSetup;
		(*pNetSetup)->copy(m_pSuspended_NetSetUp);
	}
	else
		*pNetSetup = NULL;
}



/////////////////////////////////////////////////////////////////////////////
// no need to send Accept to the card the function is being called after the
// answer from the Conf is analized by the Lobby and a new Creception was created.
// and this new object answer the card nad make the call.
void  CReceptionH323::OnLobbyReleasePartySuspend(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CReceptionH323::OnLobbyReleasePartySuspend : ");
	CReception::OnLobbyReleasePartySuspend(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void  CReceptionH323::OnTimerPartySuspend(CSegment* pParam)
{
  PTRACE(eLevelError,"CReceptionH323::OnTimerPartySuspend Jump!");
  PASSERT(1);
   // In case the timer expired we need to send a reject to the Party
  COsQueue lobbyRcvMbx = m_pLobby->GetRcvMbx();
  CH323NetSetup *pNetSetup = NULL;

  //get call information from the suspended object
  GetH323CallParams(&pNetSetup);

  //meeting room or Undefined Dial In: can't make "forwarding" in this case
  //Create new reception object for Reject
  if (pNetSetup)
	CreateH323( pNetSetup, &lobbyRcvMbx, m_pLobby, TimerForUnreservedCall );
  else
	PASSERTMSG(1,"CReceptionH323::OnTimerPartySuspend - pNetSetup not valid");

  POBJDELETE(pNetSetup);
  DeleteAllTimers();

  m_pLobby->RemoveReception(this);
  delete this;
}

///////////////////////////////////////////////////////////////////////////////
void CReceptionH323::AcceptParty(CNetSetup* pNetSetUp,CCommConf* pCommConf,CConfParty* pConfParty)
{
  m_pLobby->AcceptH323CallIn((CH323NetSetup*)pNetSetUp, pCommConf, pConfParty);
  CReception::AcceptParty(pNetSetUp,pCommConf,pConfParty);
}

/////////////////////////////////////////////////////////////////////////////
void CReceptionH323::GetCallParams(CNetSetup** pNetSetUp/*,CNetConnRsrcDesc** pNetDesc*/)
{
	CMedString trace_str;
	trace_str << "Call Id  = " << (DWORD)m_pParty;
	trace_str << " Doing nothing in 323 participant. " << "\n";

	PTRACE2(eLevelInfoNormal,"CReceptionH323::GetCallParams - ",trace_str.GetString());
}
/*/////////////////////////////////////////////////////////////////////////////
WORD  CReceptionH323::GetVideoSession(CComMode* pComMode, CConfDesc* pComConf)
{
	WORD  rc = 0;

	PASSERT(!pComMode);
	switch ( pComConf->GetVideoSession() ) {
	case VIDEO_SWITCH :			rc = 1; break;
	case CONTINUOUS_PRESENCE :  rc = 3; break;
	case VIDEO_TRANSCODING :	rc = 3; break;
	}

	return rc;
}
*/
/////////////////////////////////////////////////////////////////////////////
// Create for new gateway conference in unattented call in.
void  CReceptionH323::CreateGateWayConf(CH323NetSetup* pNetSetup, COsQueue* pLobbyRcvMbx,
										CLobby* pLobby, CTaskApp* ListId, DWORD mrId,char* mrName, char* targetNumber)
										
{
	PTRACE(eLevelInfoNormal,"CReceptionH323::CreateGateWayConf : ");

	m_state  = SUSPEND;  
	m_pLobby = pLobby; 
		
	SetTargetConfName(mrName);

	//this value (ListId) is not a valid memory pointer, 
	//it is used only to identify the suspended CReception object in the waiting list
	SetParty(ListId); 
	SetMonitorMRId(mrId);
	SetConfOnAirFlag(NO);
		
	//copy H323 setup call parameters to local storage for suspend
	m_pSuspended_NetSetUp = new CH323NetSetup;
	m_pSuspended_NetSetUp->copy(pNetSetup);

	PTRACE(eLevelInfoNormal,"CReceptionH323::CreateGateWayConf : StartTimer(PARTYSUSPENDTOUT,30*SECOND)");
	//will remove this item from the list in 30 seconds if no response recived from ConfParyMngr
	StartTimer(PARTYSUSPENDTOUT,30*SECOND); 

	//send START_GW_CONF msg to ConfPartyManager 
	CConfPartyManagerLocalApi confPartyMngrApi;
	confPartyMngrApi.StartGateWayConf(mrId,mrName,targetNumber);
	
	
}
/*CH323NetSetup* pNetSetup, 
										CIpRsrcDesc *pIpDesc,
										COsQueue* pLobbyRcvMbx, CLobby* pLobby,
										CTaskApp* ListId, char* ISDNtargetNumber,char* PrefixNumber,
										DWORD GateWayConfrenceType,
										const char* numberWithPrefix)
{
	if (GateWayConfrenceType == 323320)
		PTRACE(eLevelInfoNormal,"CReceptionH323::CreateGateWayConf 323_320 : ");
	else
		PTRACE(eLevelInfoNormal,"CReceptionH323::CreateGateWayConf 323_323 : ");


	m_state  = SUSPEND;
	m_pLobby = pLobby;

	//this value (ListId) is not a valid memory pointer,
	//it is used only to identify the suspended CReception object in the waiting list
	SetParty(ListId);

	InitTimer(*pLobbyRcvMbx);

	//copy H323 setup call parameters to local storage for suspend
	m_pSuspendedSetup = new CH323NetSetup;
	m_pSuspendedSetup->copy(pNetSetup);
	m_pSuspendedDesc  = new CIpRsrcDesc(*pIpDesc);

	PTRACE(eLevelInfoNormal,"CReceptionH323::CreateGateWayConf : StartTimer(PARTYSUSPENDTOUT,30*SECOND)");
	//will remove this item from the list in 30 seconds if no response recived from MNGR
	StartTimer(PARTYSUSPENDTOUT,30*SECOND);

	//send CREATE_GATEWAY msg to MNGR
	COsQueue McuRcvMbx;
	PASSERT_AND_RETURN(McuRcvMbx.Ident("MMGR",0,NOWAIT));
	CMcuApi* pMcuApi = new CMcuApi;
	pMcuApi->CreateOnlyApi(McuRcvMbx);

    // net setup fields we can pass
	DWORD ip_Address = m_pSuspendedSetup->GetH245Ip();
	const char* tmp = m_pSuspendedSetup->GetH323PartyAlias();
	WORD size = strlen(tmp) + 1;
	char* AliasName = new char[size];
	strncpy(AliasName, tmp, size);
	DWORD AliasType  = m_pSuspendedSetup->GetH323PartyAliasType();

	//list id is converted to WORD because we use a pointer as the identifyer
	//and the MMGR uses a WORD (ASSUMPTION: no more then 0xFFFF calls are handled at the same time)
	WORD boardId = 0;
	WORD unitId  = 0;
	boardId		 = m_pSuspendedDesc->GetBoardId();
	unitId		 = m_pSuspendedDesc->GetUnitId();

	pMcuApi->StartGateWayConf((WORD) ListId, boardId, unitId, "0", "0", ISDNtargetNumber,PrefixNumber,
							  ip_Address, size, AliasName, AliasType, GateWayConfrenceType,
							  numberWithPrefix,0);

	pMcuApi->DestroyOnlyApi();
    POBJDELETE(pMcuApi);
    DEALLOCBUFFER(AliasName);
}

*/
