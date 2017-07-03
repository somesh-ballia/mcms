
#include "ReceptionSip.h"
#include "Lobby.h"
#include "ConfApi.h"
#include "CommConfDB.h"
#include "SIPPartyInSimulation.h"
#include "SIPPartyInCreate.h"
#include "SysConfig.h"
#include "ProcessBase.h"
#include "ConfPartyManagerLocalApi.h"
#include "SystemFunctions.h"
#include "IpServiceListManager.h"
#include "SipUtils.h"

extern CIpServiceListManager* GetIpServiceListMngr();


////////////////////////////////////////////////////////////////////////////
//                            CReceptionSip          
////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CReceptionSip)

    //events from conference 
  ONEVENT(RELEASE_UNRESERVED_PARTY,SUSPEND  ,CReceptionSip::OnLobbyReleasePartySuspend)
  // events from conferance manager
  //ONEVENT(REJECT_UNRESERVED_PARTY, SUSPEND  ,CReceptionSip::OnConfMngrRejectPartySuspend)
  //local events
  ONEVENT(PARTYSUSPENDTOUT,		SUSPEND  ,CReceptionSip::OnTimerPartySuspend)
  ONEVENT(IDENTTOUT,			IDENT	 ,CReceptionSip::OnTimerPartyIdent)
  ONEVENT(TRANSFERTOUT,			TRANSFER ,CReceptionSip::OnTimerPartyTransfer) 

PEND_MESSAGE_MAP(CReceptionSip,CReception); 
  
/////////////////////////////////////////////////////////////////////////////
CReceptionSip::CReceptionSip(CTaskApp *pOwnerTask)
        :CReception(pOwnerTask)
{
	m_pConfParty = NULL;
//	m_pRsrcAlloc = NULL;
//	m_pMuxDesc	 = NULL;
	m_pNetDesc	 = NULL;
//	m_pIpDesc    = NULL;
    m_pSdpAndHeaders	= NULL;
	m_SdpAndHeadersLen	= 0;
	m_endPoints  = NULL;
	m_multiInviter = NULL;
	m_factoryCreatedConfName = NULL;
	m_factoryType = eNotSipFactory;
	m_ClickToConfId[0] = '\0';

	VALIDATEMESSAGEMAP;
}
/////////////////////////////////////////////////////////////////////////////
CReceptionSip::~CReceptionSip() 
{
	PDELETEA(m_pSdpAndHeaders);
	DEALLOCBUFFER(m_endPoints);
	DEALLOCBUFFER(m_multiInviter);
	DEALLOCBUFFER(m_factoryCreatedConfName);
}

/////////////////////////////////////////////////////////////////////////////
const char*   CReceptionSip::NameOf()  const
{
	return "CReceptionSip";
}
/////////////////////////////////////////////////////////////////////////////
void  CReceptionSip::CreateSip(DWORD confMonitorId, DWORD partyMonitorId, COsQueue * pConfRcvMbx,
							   COsQueue* pLobbyRcvMbx, /*CRsrcTbl* pRsrcTbl,*/CSipNetSetup* pNetSetup,
							   /*CIpRsrcDesc * pIpDesc,*/ CLobby * pLobby, char * pPartyName,
							   const char* confName,/*CComMode * pComMode,*/ CConfParty* pConfParty,
							   CCommConf* pComConf, WORD isChairEnabled/*, WORD advancedAudio*/,BYTE bIsSoftCp/*,BYTE IsGateWay*/,
							   const sipSdpAndHeadersSt* pSdpAndHeaders/*, WORD videoPlus*/)      
{               
	WORD rsrcDeficncy	= 0;
	WORD cardID			= 0;
	WORD origionBoardID = 0;
	WORD line			= 0;
	int index;
	OFF(rsrcDeficncy);
	STATUS status = STATUS_OK;

	m_pLobby	 	= pLobby; 
	m_pConfParty 	= pConfParty;
	m_confMonitorId  = confMonitorId;
	SetSdpAndHeaders(*pSdpAndHeaders);

	//Check if this is MSFT AVMCU -> put it to cascade mode (VNGR-25321) 
	char cUserAgent[MaxUserAgentSize] = {0};
	BYTE* pStart = (BYTE *)(m_pSdpAndHeaders->capsAndHeaders) + m_pSdpAndHeaders->sipHeadersOffset;
	sipMessageHeaders* pHeaders = (sipMessageHeaders *)pStart;
	::SipGetHeaderValue(pHeaders, kUserAgent, cUserAgent, MaxUserAgentSize);

	if ( strstr(cUserAgent, "AV-MCU") )
	{
		PTRACE(eLevelInfoNormal,"CReceptionSip::CreateSip : user agent is MS AV-MCU setting to cascade mode");
		m_pConfParty->SetCascadeMode(CASCADE_MODE_MASTER);
	}

	// ------------------------------------------------------------------------------
	// SVC Cascade treatment is done in <CSipPartyInCreate::OnConfEstablishCallIdle>
	// ------------------------------------------------------------------------------


	DWORD confRate = 0;
	DWORD vid_bitrate;
	DWORD rtp_vid_bitrate; 


	//select A_law / U_law for voice calls
	WORD voice_type = pConfParty->GetVoice();
	if 	(voice_type == YES) voice_type = U_LAW;
	
	void (*entryPoint)(void*);
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	BOOL bIsSipPartyLevelSimulation = 0;
	std::string key = "SIP_PARTY_LEVEL_SIMULATION";
	pSysConfig->GetBOOLDataByKey(key, bIsSipPartyLevelSimulation);
	if(bIsSipPartyLevelSimulation)
		entryPoint =  SipPartyInSimulationEntryPoint;// for party control level simulation
	else if (::IsWebRtcCall((sipSdpAndHeaders*)pSdpAndHeaders))
		entryPoint =  SipPartyInWebRtcCreateEntryPoint;
	else
		entryPoint =  SipPartyInCreateEntryPoint;
	
	// in EPC mcuNumber / terminalNumber always 0
	//WORD  mcuNumber      = ( pComMode->GetContentMode() != 0 ) ? 0 : 1;
	//WORD  terminalNumber = ( pComMode->GetContentMode() != 0 ) ? 0 : 1;
	WORD  mcuNumber = 1;
	WORD  terminalNumber = 1;
	//WORD  serviceId  = pConfParty->GetServiceId();

	WORD serviceId = (WORD)pNetSetup->GetCsServiceid();
	PTRACE2INT(eLevelInfoNormal,"CReceptionSip::CreateSip : ServiceId from CS ", serviceId);
	CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
	CConfIpParameters* pServiceParams = pIpServiceListManager->FindIpService(serviceId);
	if (pServiceParams == NULL)
	{
	   const char* serviceName  = pConfParty->GetServiceProviderName();//"sip and 323";//
	   PTRACE2(eLevelInfoNormal,"CReceptionSip::CreateSip : ServiceId from Service name ", serviceName);
	   pServiceParams = pIpServiceListManager->GetRelevantService(serviceName, pConfParty->GetNetInterfaceType());
	   if (pServiceParams == NULL)
	   {
	      PASSERTMSG(partyMonitorId,"CReceptionSip::CreateSip - No IP Service exists!!!");
	      return;
	   }
	   serviceId  = (WORD)pServiceParams->GetServiceId();
	}
	
	PTRACE2(eLevelInfoNormal,"CReceptionSip::CreateSip : Service name ", (const char*)(pServiceParams->GetServiceName ()));
	pConfParty->SetServiceProviderName((const char*)(pServiceParams->GetServiceName ())); 
	
	if (!pNetSetup->IsItPrimaryNetwork())
	{
		pConfParty->SetSubServiceName(SUB_SERVICE_NAME_SECONDARY);
	}
	
	m_pPartyApi = new CPartyApi;
	m_pPartyApi->Create(entryPoint,
	                    *pLobbyRcvMbx,
	                    *pConfRcvMbx,
	                    DEFAULT_PARTY_ID,
	                    partyMonitorId,
	                    confMonitorId,
	                    "",
	                    pPartyName,
	                    confName,
	                    serviceId,
	                    terminalNumber,
	                    mcuNumber,
	                    "ggg",
	                    voice_type,
	                    1,
	                    pComConf->GetIsGateway(),
	                    pConfParty->GetRecordingLinkParty());

	m_pParty = m_pPartyApi->GetTaskAppPtr();      // set self identification

#ifdef LOOKUP_TABLE_DEBUG_TRACE
	TRACEINTO << "D.K."
			<< "  PartyName:"        << pPartyName
			<< ", PartyRsrcId:"          << m_pParty->GetPartyId() << "(0)"
			<< ", IsDialOut:"        << 0
			<< ", Pointer:"          << std::hex << m_pParty;
#endif

	PTRACE(eLevelInfoNormal,"CReceptionSip::CreateSip : StartTimer(IDENTTOUT,IDENT_TOUT)");
	StartTimer(IDENTTOUT,IDENT_TOUT);
	
	BYTE bIsQuad = FALSE; //In case sip will support quad will sell check if it is quad conference.
TRACEINTO << "IP VERSION: " << (DWORD)pNetSetup->GetIpVersion();
	m_pPartyApi->IdentifySip(pNetSetup,/*pIpDesc,pTargetMode,*/m_pSdpAndHeaders,m_SdpAndHeadersLen);

}

/////////////////////////////////////////////////////////////////////////////
// Create for party reject in unattended call in.
void CReceptionSip::CreateRejectSip(CSipNetSetup* pNetSetup, COsQueue* pLobbyRcvMbx, CLobby* pLobby, int reason, const sipSdpAndHeadersSt* pSdpAndHeaders, char* pAltAddress, STATUS status)
{
	PTRACE(eLevelInfoNormal, "CReceptionSip::Create for Reject : ");

	m_pLobby = pLobby;

	SetSdpAndHeaders(*pSdpAndHeaders);

	mcTransportAddress* pDestIP = FindDestIp(pSdpAndHeaders);
	CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
	CConfIpParameters* pServiceParams = (pIpServiceListManager && pDestIP) ? pIpServiceListManager->FindServiceByIPAddress(*pDestIP) : NULL;
	if (pServiceParams == NULL)
	{
		PASSERTMSG(1, "CReceptionSip::CreateRejectSip - IP Service does not exist!!!");
		PDELETE(pDestIP);
		return;
	}

	WORD serviceId = pServiceParams->GetServiceId();

	void (*entryPoint)(void*) = SipPartyInCreateEntryPoint;

	m_pPartyApi = new CPartyApi;
	m_pPartyApi->Create(entryPoint,
	                    *pLobbyRcvMbx,
	                    *pLobbyRcvMbx,
	                    DEFAULT_PARTY_ID,
	                    0xFFFFFFFF,
	                    (DWORD) this,
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

	m_pPartyApi->RejectSipCall(pNetSetup, reason, m_SdpAndHeadersLen, m_pSdpAndHeaders, pAltAddress, status);
	m_pPartyApi->Destroy();
	POBJDELETE(m_pPartyApi);
	PDELETE(pDestIP);
}

/////////////////////////////////////////////////////////////////////////////
// Create for party suspend in unattended call in.
void  CReceptionSip::CreateSuspend(CSipNetSetup *pNetSetup, const sipSdpAndHeadersSt* pSdpAndHeaders, COsQueue *pLobbyRcvMbx, CLobby *pLobby,
									 CTaskApp *pListId, DWORD confID,char* confName)
{ 
	WORD isVoice = 0;

	PTRACE(eLevelInfoNormal,"CReceptionSip::CreateSuspend for suspend");

	m_state  = SUSPEND;  
	m_pLobby = pLobby; 
	
	SetTargetConfName(confName);
	//this value (pListId) is not a valid memory pointer, 
	//it is used only to identify the suspended CReception object in the waiting list
	SetParty(pListId); 
	SetMonitorMRId(confID);
	SetConfOnAirFlag(NO);
	//SetMsConversationId(pSdpAndHeaders);
	SetClickToConfId(pSdpAndHeaders);
	//copy H323 setup call parameters to local storage for suspend
	m_pSuspended_NetSetUp = new CSipNetSetup;
	m_pSuspended_NetSetUp->copy(pNetSetup);
	SetSdpAndHeaders(*pSdpAndHeaders);


	PTRACE(eLevelInfoNormal,"CReceptionSip::CreateSuspend : StartTimer(PARTYSUSPENDTOUT,30*SECOND)");
	//will remove this item from the list in 30 seconds if no response received from MNGR
	StartTimer(PARTYSUSPENDTOUT,30*SECOND); 
}

/////////////////////////////////////////////////////////////////////////////
// Create suspend for party when conf is already Exist
void CReceptionSip::CreateSuspendConfExist(CSipNetSetup* pNetSetup, const sipSdpAndHeadersSt* pSdpAndHeaders, COsQueue* pLobbyRcvMbx, CLobby* pLobby, CTaskApp* ListId, DWORD confID, char* confName)
{
  PTRACE(eLevelInfoNormal,"CReceptionSip::CreateSuspendConfExist - Create suspend for undefined party in existing conference");
  WORD bIsMrcCall = ::IsMrcHeader(pSdpAndHeaders);
  WORD bIsWebRtcCall = ::IsWebRtcCall((const sipSdpAndHeaders*) pSdpAndHeaders);
  //eFeatureRssDialin
  WORD srsSessionType	=  ::getSrsSessionType(pSdpAndHeaders);

  //BYTE tmpCascadeMode = ::GetCascadeModeFromHeader(pSdpAndHeaders);	//--- patch for ignoring Encryption in Cascade
  BYTE bIsMSConfInvite = pNetSetup->GetIsMsConfInvite();
  BYTE bIsNonCCCPAvMcu = IsAvMcuNonCCCPCascade(pSdpAndHeaders,bIsMSConfInvite);

  PTRACE2INT(eLevelInfoNormal,"CReceptionSip::CreateSuspendConfExist - bIsMSConfInvite ",bIsMSConfInvite);

  CreateSuspend(pNetSetup, pSdpAndHeaders, pLobbyRcvMbx, pLobby, ListId, confID, confName);

  SetConfOnAirFlag(YES);
  const char* pSrcAddr = ((CSipNetSetup*)m_pSuspended_NetSetUp)->GetRemoteSipAddress();
  CConfPartyManagerLocalApi* pConfPartyMngrApi = (CConfPartyManagerLocalApi*)CProcessBase::GetProcess()->GetManagerApi();

  mcTransportAddress* pDestIP = (mcTransportAddress*)((CSipNetSetup*)m_pSuspended_NetSetUp)->GetTaDestPartyAddr();
   //BRIDGE-15864: workaround for SAV only call
   WORD	bIsAudioOnly = NO;
   CCommConf* pCommConf = NULL;
   pCommConf = ::GetpConfDB()->GetCurrentConf(confID);
   if(!pCommConf)
   {
	TRACEINTO << "ConfId:" << confID << " - Failed to find conference in ConfDB";
   }
   if(bIsMrcCall && pCommConf && (pCommConf->GetConfMediaType() != eAvcOnly))
   {
	bIsAudioOnly =  ::IsMediaAudioOnly(m_pSdpAndHeaders);
	PTRACE(eLevelInfoNormal,"CReceptionSip::CreateSuspendConfExist - Mrc audio only call, set to SAC only");
   }
  if (!isApiTaNull(pDestIP))
  {
    pConfPartyMngrApi->AcceptUnreservedParty(pDestIP, confID, (DWORD)ListId, bIsAudioOnly, YES, eNotSipFactory, pSrcAddr, ((CSipNetSetup*)m_pSuspended_NetSetUp)->GetInitialEncryptionValue(), "", bIsMrcCall, bIsWebRtcCall, /*tmpCascadeMode,*/bIsMSConfInvite, srsSessionType,bIsNonCCCPAvMcu);	//--- patch for ignoring Encryption in Cascade
    return;
  }
  pDestIP = FindDestIp(pNetSetup);
  pConfPartyMngrApi->AcceptUnreservedParty(pDestIP, confID, (DWORD)ListId, bIsAudioOnly, YES, eNotSipFactory, pSrcAddr, ((CSipNetSetup*)m_pSuspended_NetSetUp)->GetInitialEncryptionValue(), "", bIsMrcCall, bIsWebRtcCall, /*tmpCascadeMode,*/bIsMSConfInvite, srsSessionType,bIsNonCCCPAvMcu);	//--- patch for ignoring Encryption in Cascade
  PDELETE(pDestIP);
}

/////////////////////////////////////////////////////////////////////////////
// Create for new meeting room conference in unattented call in.
void  CReceptionSip::CreateMeetingRoom(CSipNetSetup *pNetSetup, const sipSdpAndHeadersSt* pSdpAndHeaders, COsQueue *pLobbyRcvMbx,
								       CLobby *pLobby,CTaskApp *pListId,DWORD mrID,char* mrName)
//									   char* pAdHocConfName, char* pAdHocNID, char* pMultiInviter, char* pEndPoints)
{ 
	PTRACE(eLevelInfoNormal,"CReceptionSip::CreateMeetingRoom : ");
	int status = STATUS_OK; 
	WORD isVoice = 0;
	BYTE bIsMultiInvite = FALSE;

	CreateSuspend(pNetSetup, pSdpAndHeaders, pLobbyRcvMbx, pLobby, pListId, mrID, mrName);

	CConfPartyManagerLocalApi confPartyMngrApi;
	confPartyMngrApi.StartMeetingRoom(mrID);

	/*if(pEndPoints && pMultiInviter)
	{
		if(*pMultiInviter != '\0')
		{
			char* pTemp = NULL;
			bIsMultiInvite = TRUE;
			m_endPoints = new char[IP_LIMIT_ADDRESS_CHAR_LEN * 10];
			strncpy(m_endPoints, pEndPoints, (IP_LIMIT_ADDRESS_CHAR_LEN * 10)-1);
			m_multiInviter = new char[IP_LIMIT_ADDRESS_CHAR_LEN];
			pTemp = (char*)strstr(pMultiInviter, "sip:");
			if(pTemp)
				strncpy(m_multiInviter, pTemp + 4, IP_LIMIT_ADDRESS_CHAR_LEN -1);
			else
				strncpy(m_multiInviter, pMultiInviter, IP_LIMIT_ADDRESS_CHAR_LEN -1);
		}
	}
	
	//this value (ListId) is not a valid memory pointer, 
	//it is used only to identify the suspended CReception object in the waiting list
	SetParty(pListId); 
	
//	InitTimer(*pLobbyRcvMbx);
	
	//copy H323 setup call parameters to local storage for suspend
	m_pSuspendedSetup = new CSipNetSetup;
	m_pSuspendedSetup->copy(pNetSetup);
//	m_pSuspendedDesc  = new CIpRsrcDesc(*pIpDesc);
	SetSdpAndHeaders(*pSdpAndHeaders);

	PTRACE(eLevelInfoNormal,"CReceptionSip::CreateMeetingRoom : StartTimer(PARTYSUSPENDTOUT,30*SECOND)");
	//will remove this item from the list in 30 seconds if no response received from MNGR
	StartTimer(PARTYSUSPENDTOUT,30*SECOND); 

	//send CREATE_MEETING_ROOM msg to MNGR 
	COsQueue McuRcvMbx;  
	PASSERT_AND_RETURN(McuRcvMbx.Ident("MMGR",0,NOWAIT));
	CMcuApi* pMcuApi = new CMcuApi;
	pMcuApi->CreateOnlyApi(McuRcvMbx);
	
	//list id is converted to WORD because we use a pointer as the identifyer
	//and the MMGR uses a WORD (ASSUMPTION: no more then 0xFFFF calls are handled at the same time)
	WORD boardId = 0;
	WORD unitId  = 0;
	boardId		 = m_pSuspendedDesc->GetBoardId();
	unitId		 = m_pSuspendedDesc->GetUnitId();
	
	//if Ad-hoc
	if(pAdHocConfName)
	{
		DWORD wConf_id = 0xFFFFFFFF;
		CCommRes* pEqCommRes  = ::GetpMeetingRoomDB()->GetCurrentRsrv(confId);
		CNumericConferenceId* pResultConferenceId = NULL;
		
		if(pEqCommRes && pEqCommRes->GetAdHoc())
		{
			// case of Ad hoc Entry-Queue
			DWORD dwAdHocProfileId = pEqCommRes->GetAdHocProfileId();
			// If the ad hoc is checked and no profile is configured return error
			if(dwAdHocProfileId != 0xFFFFFFFF)
			{
				CCommRes* pConfProfile = ::GetpTemplatesDB()->GetCurrentRsrv(dwAdHocProfileId);	
				if(pConfProfile)
				{
					if(pAdHocNID)
					{
						CCommRes *pCommRes = new CCommRes;
						// Check if the numeric id is already exists in database
						if(strcmp(pAdHocNID, ""))
						{
							pCommRes->SetEntryPassword((char*)pAdHocNID);
							status =::PasswordAndNID_LengtheCheckAccordingConfiguration(pCommRes);
							if (status==STATUS_OK)
								status=::PasswordCheckAndAllocation(pCommRes,NO);
						}
						//if No NID is given, allocate one
						else
						{
							//status = ::GetpNumericConferenceIdDB //CIpRsrcDesc * pIpDesc, ()->AllocateNewNumericId(&pResultConferenceId);
							status=::PasswordCheckAndAllocation(pCommRes,NO);
							if(STATUS_OK == status)
							{
//								strncpy(pAdHocNID, pCommRes->seth, NUMERIC_CONFERENCE_ID_LEN);
								//::GetpNumericConferenceIdDB()->ReleaseNumericId(&pResultConferenceId);
							}
						}

						if(STATUS_OK == status)
						{
							//if no conf name is given, allocate one
							if(!strcmp(pAdHocConfName, ""))
							{
								
								//if caller display name exist, ad-hoc conf name is taken from it
								if(strcmp(pNetSetup->GetRemoteDisplayName(), ""))
								{
									ALLOCBUFFER(pTemp, MaxDisplaySize);
									strncpy(pTemp, pNetSetup->GetRemoteDisplayName(), MaxDisplaySize-1);
									int len = strlen(pTemp);
									//remove  "
									//replace whitespaces , @ ; : with _
									for(int i=0; i<len; i++) {
										if (*(pTemp+i)==' ' || *(pTemp+i)=='@' || *(pTemp+i)==',' || *(pTemp+i)==';' || *(pTemp+i)==':') 
											*(pTemp+i)='_';
										else   
										{
											if(*(pTemp+i)=='"' || *(pTemp+i)=='\'')
											{
												strncpy((pTemp+i), (pTemp+i+1), len-i);
												i--;
											}
										}
									}
									sprintf(pAdHocConfName,"%s_%s", pTemp, (const char *)pAdHocNID);
									DEALLOCBUFFER(pTemp);
								}
								else
									sprintf(pAdHocConfName,"%s",(const char *)pAdHocNID);
							}
							
							if( NULL == pResultConferenceId)  // Create new ad hoc conference if the numeric id is not in use
							{
								if (::GetpRsrvDB()->FindName(pAdHocConfName) == NOT_FIND)  
								{
									if(::GetpConfDB()->FindName(pAdHocConfName) == NOT_FIND)
									{
										if(::GetpMeetingRoomDB()->FindName(pAdHocConfName) == NOT_FIND)
										{
											pConfProfile->SetMonitorConfId(0xFFFFFFFF);
											pConfProfile->SetEntryQ(NO);
											pConfProfile->SetName(pAdHocConfName);
											pConfProfile->SetMeetingRoom(NO);

											CStructTm curTime;
											curTime.m_year = 0;
											pConfProfile->SetStartTime(curTime);
											
											// Numeric conference id provided by user
											if(strcmp(pAdHocNID, ""))
												pConfProfile->SetNumericConfId((const char *)pAdHocNID);
											pConfProfile->SetMeetMePerEntryQ(YES);
											pConfProfile->SetMeetMePerConf(YES);
											pConfProfile->SetUnlimitedReservFlag(YES);
											pConfProfile->SetConfContactInfo(pEqCommRes->GetName(), 2);
											pMcuApi->CreateAdHocConference(confId,(WORD)pListId,boardId,unitId,(char*)pNetSetup->GetRemoteSipAddress(),pNetSetup->GetSrcPartyIp(),isVoice, YES, bIsMultiInvite, eNotSipFactory, pConfProfile);
										}
										else
											status = STATUS_CONF_NAME_EXISTS;
									}
									else
										status = STATUS_CONF_NAME_EXISTS;
								}
								else
									status = STATUS_CONF_NAME_EXISTS;
							}	
							else
								status = STATUS_NUMERIC_CONFERENCE_ID_OCCUPIED;
						}
					}
				}
				if(STATUS_OK != status)
					PTRACE2(eLevelError,"CReceptionSip::CreateMeetingRoom, failed to create ad-hoc conf : ", GetStatusAsString(status));
				POBJDELETE(pConfProfile);
			}
		}
		POBJDELETE(pEqCommRes);
	}
	//else - regular meeting room
	else
		pMcuApi->StartMeetingRoom(confId,(WORD)pListId,boardId,unitId,"",pNetSetup->GetSrcPartyIp(),isVoice,YES, strlen(pNetSetup->GetRemoteSipAddress()), (char*)pNetSetup->GetRemoteSipAddress());
	pMcuApi->DestroyOnlyApi();
    POBJDELETE(pMcuApi);*/
}
/////////////////////////////////////////////////////////////////////////////

void  CReceptionSip::CreateGateWayConf(CSipNetSetup *pNetSetup, const sipSdpAndHeadersSt* pSdpAndHeaders, COsQueue *pLobbyRcvMbx,
								       CLobby *pLobby,CTaskApp *pListId,DWORD gwID,char* gwName,char* dialString)
{
	PTRACE(eLevelInfoNormal,"CReceptionSip::CreateGateWayConf : ");
		
	CreateSuspend(pNetSetup, pSdpAndHeaders, pLobbyRcvMbx, pLobby, pListId, gwID, gwName);
		
	CConfPartyManagerLocalApi confPartyMngrApi;
	confPartyMngrApi.StartGateWayConf(gwID,gwName,dialString);

}
/////////////////////////////////////////////////////////////////////////////
// Create for new conference in unattented call in to factory.
void  CReceptionSip::CreateConfFromFactory(CSipNetSetup * pNetSetup, COsQueue * pLobbyRcvMbx,
								       CLobby * pLobby, CTaskApp * pListId,const sipSdpAndHeadersSt * pSdpAndHeaders,
									   CCommRes* pFactory)
{ 
	PTRACE(eLevelInfoNormal,"CReceptionSip::CreateConfFromFactory : ");
	DWORD profileID = pFactory->GetAdHocProfileId();
	DWORD mrID = pFactory->GetMonitorConfId();
	
	char AdHocConfName[H243_NAME_LIST_ID_LEN] = "";
	snprintf(AdHocConfName, sizeof(AdHocConfName), "%s_%d", pFactory->GetName(), (DWORD)pListId);
	SetTargetConfName(AdHocConfName);

	char AdHocConfDisplayName[H243_NAME_LIST_ID_LEN] = "";
	snprintf(AdHocConfDisplayName, sizeof(AdHocConfDisplayName), "%s_%d", pFactory->GetDisplayName(), (DWORD)pListId);
	
	CreateSuspend(pNetSetup, pSdpAndHeaders, pLobbyRcvMbx, pLobby, pListId, mrID, AdHocConfName);
	
	m_factoryCreatedConfName = new char[2*IP_LIMIT_ADDRESS_CHAR_LEN];
	*m_factoryCreatedConfName = '\0';
	strncpy(m_factoryCreatedConfName, AdHocConfName, IP_LIMIT_ADDRESS_CHAR_LEN);
	strcat(m_factoryCreatedConfName, "@");
	strncat(m_factoryCreatedConfName, pNetSetup->GetDestPartyAddress(), MaxAddressListSize);
	
	char szType[16]="";
	if(pFactory->IsAutoConnectFactory())
	{
		m_factoryType = e200SipFactory;
		strncpy(szType, "auto", 4);
	}
	else
	{
		m_factoryType = e302SipFactory;
		strncpy(szType, "redirect", 8);
	}
	
	PTRACE2(eLevelInfoNormal,"CReceptionSip::CreateConfFromFactory : factory type: ", szType);
	CCommRes* pProfile = NULL;
	pProfile = ::GetpProfilesDB()->GetCurrentRsrv(profileID);
	
	CConfPartyManagerLocalApi confPartyMngrApi;
	// Romem klocwork
	if(pProfile)
	{
		pProfile->SetAdHocProfileId(profileID);
		confPartyMngrApi.LobbyStartAdHocConf(profileID, pProfile, AdHocConfName, '\0', TRUE,AdHocConfDisplayName);
		POBJDELETE(pProfile);
	}
}

/////////////////////////////////////////////////////////////////////////////
void*  CReceptionSip::GetMessageMap()                                        
{
  return (void*)m_msgEntries;    
}

/////////////////////////////////////////////////////////////////////////////
void  CReceptionSip::OnException()
{
	PTRACE(eLevelInfoNormal,"CReceptionSip::OnException : ");
	PASSERT(!m_pLobby);	 

    DeleteAllTimers();
	if(IsValidPObjectPtr(m_pPartyApi))
	{
		STATUS r_status;
		r_status = m_pPartyApi->SendOpcodeMsg(LOBBYDESTROY);
//		m_pPartyApi->LobbyDestroy();
		PTRACE2INT(eLevelInfoNormal/*|LOBBY_TRACE*/,"CReceptionSip::OnException : return status - ",r_status);
		POBJDELETE(m_pPartyApi);
	}
		
	m_pLobby->OnEndPartyTransfer(this);
	delete this;
} 

/////////////////////////////////////////////////////////////////////////////
void CReceptionSip::SetSdpAndHeaders(const sipSdpAndHeadersSt & SdpAndHeaders)
{
	PDELETEA(m_pSdpAndHeaders);
	m_SdpAndHeadersLen = sizeof(sipSdpAndHeadersBaseSt) + SdpAndHeaders.lenOfDynamicSection;
	m_pSdpAndHeaders = (sipSdpAndHeadersSt *)new BYTE[m_SdpAndHeadersLen];
	memset(m_pSdpAndHeaders, 0, m_SdpAndHeadersLen);
	memcpy(m_pSdpAndHeaders,&SdpAndHeaders,m_SdpAndHeadersLen);
}

/////////////////////////////////////////////////////////////////////////////
const sipSdpAndHeadersSt*	CReceptionSip::GetSdpAndHeaders()
{
	return m_pSdpAndHeaders;
}

/////////////////////////////////////////////////////////////////////////////
void  CReceptionSip::GetSipCallParams(CSipNetSetup** ppNetSetup/*,CIpRsrcDesc** ppIpDesc*/,sipSdpAndHeadersSt** ppSdpAndHeaders)
{
	//create a copy of the suspended Sip parameters and return them to caller
	if (m_pSuspended_NetSetUp)
	{
		*ppNetSetup = new CSipNetSetup;
		(*ppNetSetup)->copy(m_pSuspended_NetSetUp);
	}
	else
		*ppNetSetup = NULL;

	if (m_pSdpAndHeaders)
	{
		*ppSdpAndHeaders = (sipSdpAndHeadersSt *)new BYTE[m_SdpAndHeadersLen];
		memcpy(*ppSdpAndHeaders,m_pSdpAndHeaders,m_SdpAndHeadersLen);
	}
	else
		*ppSdpAndHeaders = NULL;
}



/////////////////////////////////////////////////////////////////////////////
// no need to send Accept to the card the function is being called after the
// answer from the Conf is analized by the Lobby and a new Creception was created.
// and this new object answer the card nad make the call.
void  CReceptionSip::OnLobbyReleasePartySuspend(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CReceptionSip::OnLobbyReleasePartySuspend : ");
	DeleteTimer(PARTYSUSPENDTOUT); 

	if(IsMultiInvite())
	{
		pParam->ResetRead();
		DWORD	msgLen, confId;
		WORD	opCode;
		
		*pParam >> msgLen;
		*pParam >> opCode;
		*pParam >> confId;

		CallMultiInviteParties(confId);
	}
	
	//lobby continue proccessing of the suspended call 
	//there is no longer need for this reception object
    DeleteAllTimers();
    m_pLobby->RemoveFromList(this);
	delete this;
}

/////////////////////////////////////////////////////////////////////////////
void  CReceptionSip::OnTimerPartySuspend(CSegment* pParam)
{
	PTRACE(eLevelError,"CReceptionSip::OnTimerPartySuspend : msg");

	// In case the timer expired we need to send a reject to the Card
	// so the card will release it's incoming call port
	WORD advancedAudio		= FALSE;
	COsQueue lobbyRcvMbx = m_pLobby->GetRcvMbx();

	CSipNetSetup * pNetSetup = NULL;
	sipSdpAndHeadersSt * pSdpAndHeaders = NULL;
	
	//get call information from the suspended object
	GetSipCallParams(&pNetSetup/*,&pIpDesc*/,&pSdpAndHeaders); 
	
	//create new reception object for the accept process
	if (pNetSetup/* && pIpDesc*/)
	{ 
		//meeting room or Undefined Dial In: can't make "forwarding" in this case
		CreateRejectSip(pNetSetup,/*pIpDesc,*/
						&lobbyRcvMbx,m_pLobby,/*advancedAudio,*/ int(cmReasonTypeNoBandwidth),pSdpAndHeaders);  
	
		POBJDELETE(pNetSetup);
		PDELETEA(pSdpAndHeaders);
	}
	else 
	{
		//there is no longer need for this reception object
		POBJDELETE(pNetSetup);
		PDELETEA(pSdpAndHeaders);
		PASSERT(1);
        DeleteAllTimers();
        
		delete this;
	}
}

/////////////////////////////////////////////////////////////////////////////
void  CReceptionSip::OnTimerPartyIdent(CSegment* pParam)
{
	PTRACE(eLevelError,"CReceptionSip::OnTimerPartyIdent : msg");
	OnException();
}

/////////////////////////////////////////////////////////////////////////////
void  CReceptionSip::OnTimerPartyTransfer(CSegment* pParam)
{
	PTRACE(eLevelError,"CReceptionSip::OnTimerPartyTransfer : msg");
	OnException();
}

////////////////////////////////////////////////////////////////////////////		
void  CReceptionSip::AcceptParty(CNetSetup* pNetSetUp,CCommConf* pCommConf,CConfParty* pConfParty)
{
	BYTE bIsSoftCp = (pCommConf->GetVideoSession() == SOFTWARE_CONTINUOUS_PRESENCE)? YES: NO;
	m_pLobby->AcceptSipCallIn((CSipNetSetup*)pNetSetUp, pCommConf, pConfParty, bIsSoftCp, GetSdpAndHeaders());
	CReception::AcceptParty(pNetSetUp,pCommConf,pConfParty);
}
			  

/////////////////////////////////////////////////////////////////////////////
void  CReceptionSip::AddMultiInvite(DWORD confId, WORD listId, WORD boardId)
{
	PTRACE(eLevelError,"CReceptionSip::AddMultiInvite - Currently does nothing");

	/* The following 4 lines remained uncommented for some reason, they of course make no sense without the rest of the function, commenting them
	char* temp;
	BYTE  bAddInviter = FALSE;
	ALLOCBUFFER(pPartyData, IP_LIMIT_ADDRESS_CHAR_LEN);
	ALLOCBUFFER(pUserName, IP_LIMIT_ADDRESS_CHAR_LEN);   */

/*	COsQueue McuRcvMbx;  
	PASSERT_AND_RETURN(McuRcvMbx.Ident("MMGR",0,NOWAIT));   
	CMcuApi* pMcuApi = new CMcuApi;
	pMcuApi->CreateOnlyApi(McuRcvMbx);
	
	CCommConf* pCommconf = ::GetpConfDB()->GetCurrentConf(confId);
	if(pCommconf)
	{
		while(m_endPoints)
		{
			//search for opening
			temp = (char*)strstr(m_endPoints, "<sip:");
			if(temp)
			{
				strcpy(m_endPoints, temp+5 );
				//search for closing
				temp = (char*)strstr(m_endPoints, ">");
				if(temp)
				{
					strncpy(pPartyData, m_endPoints, temp - m_endPoints);
					strcpy(m_endPoints, temp +1);
					pPartyData[temp-m_endPoints]='\0';
					temp=strstr(pPartyData, "@");
					if(temp)
					{
						strncpy(pUserName, pPartyData, temp - pPartyData);
						pUserName[temp - pPartyData]='\0';
						//if name is equal to source EQ name
						if(!strncmp(pCommconf->GetConfContactInfo(2), pUserName, IP_LIMIT_ADDRESS_CHAR_LEN))
							continue;
						PTRACE2(eLevelInfoNormal, "CReceptionSip::MultiInvite, Adding a party to the conf. party = ", pPartyData);
						//if party is the inviter, use the listId, otherwise - listId = 0 (-> defined party)
						if(!strncmp(m_multiInviter, pPartyData, IP_LIMIT_ADDRESS_CHAR_LEN))
							bAddInviter = TRUE;
						else 
							pMcuApi->CreateSipDialOutParty(confId, 0, pPartyData, boardId);
					}
				}
				else
					break;
			}
			else
				break;
		}
	}
	if(bAddInviter)
		pMcuApi->CreateSipDialOutParty(confId, listId, m_multiInviter, boardId);

	pMcuApi->DestroyOnlyApi();
	POBJDELETE(pMcuApi);
	DEALLOCBUFFER(pPartyData);
	DEALLOCBUFFER(pUserName);*/
}

/////////////////////////////////////////////////////////////////////////////
BYTE CReceptionSip::IsMultiInvite()
{
	BYTE result = FALSE;
	if(m_endPoints)
		result = TRUE;
	return result;
}

/////////////////////////////////////////////////////////////////////////////
char* CReceptionSip::GetFactoryCreatedConfName()
{
	return m_factoryCreatedConfName;
}

/////////////////////////////////////////////////////////////////////////////
eSipFactoryType	CReceptionSip::GetFactoryType()
{
	return m_factoryType;
}

/////////////////////////////////////////////////////////////////////////////
CConfApi* GetConfApi(DWORD confId)
{
	CConfApi*  pConfApi		= NULL;
	CCommConf* pCommConf	= ::GetpConfDB()->GetCurrentConf(confId);
	if(pCommConf)
	{
		COsQueue* pConfRcvMbx = pCommConf->GetRcvMbx();
		
		if(pConfRcvMbx)
		{
			pConfApi = new CConfApi; 
			pConfApi->CreateOnlyApi(*pConfRcvMbx); 
		}
	}
	return pConfApi;
}

/////////////////////////////////////////////////////////////////////////////
void  CReceptionSip::CallMultiInviteParties(DWORD confId)
{
	
	CConfApi* pConfApi = GetConfApi(confId); 
	if(pConfApi)
		pConfApi->ConnectDialOutPartyByNumb(0xFF);
	else
		PTRACE(eLevelError,"CReceptionSip::CallMultiInviteParties - pConfApi is NULL!!!");
	POBJDELETE(pConfApi);
}

/////////////////////////////////////////////////////////////////////////////
//if x@domain -> return the IP of the service that the domain belongs to
//if x@ip	  -> if ip=domain -> return the IP of the service that the domain belongs to
//				 if ip 		  -> return the IP as is.
mcTransportAddress*  CReceptionSip::FindDestIp(CSipNetSetup* pNetSetup)
{
	mcTransportAddress* pDestIP = NULL;
	const char *pDestAddr = pNetSetup->GetDestPartyAddress();
	if(pDestAddr && *pDestAddr && !strstr(pDestAddr, "@"))
		pDestIP = FindDestIp(pDestAddr);

	return pDestIP;
}

/////////////////////////////////////////////////////////////////////////////
//if x@domain -> return the IP of the service that the domain belongs to
//if x@ip	  -> if ip=domain -> return the IP of the service that the domain belongs to
//				 if ip 		  -> return the IP as is.
mcTransportAddress*  CReceptionSip::FindDestIp(const sipSdpAndHeadersSt* pSdpAndHeaders)
{
	mcTransportAddress* pDestIP = NULL;
	BYTE * pSipCaps = (BYTE *)pSdpAndHeaders->capsAndHeaders;
	sipMessageHeaders * pSipHeaders = (sipMessageHeaders *)(pSipCaps + pSdpAndHeaders->sipHeadersOffset);
	CSipHeaderList* pTemp = new CSipHeaderList(*pSipHeaders);
	const CSipHeader* pReq = pTemp->GetNextHeader(kReqLine);
	const char *pReqLine = pReq ? pReq->GetHeaderStr() : NULL;
	
	if(pReqLine && *pReqLine)
	{
		char* pDestAddr = NULL;
		
		if(strstr(pReqLine, "@"))
		{
			pDestAddr = (char*)strstr(pReqLine, "@");
			pDestAddr++;
		}
		else
			pDestAddr = (char*)pReqLine;
			
		pDestIP = FindDestIp(pDestAddr);
	}		
	POBJDELETE(pTemp);	
	return pDestIP;
}

/////////////////////////////////////////////////////////////////////////////
//if x@domain -> return the IP of the service that the domain belongs to
//if x@ip	  -> if ip=domain -> return the IP of the service that the domain belongs to
//				 if ip 		  -> return the IP as is.
mcTransportAddress*  CReceptionSip::FindDestIp(const char* pDestAddr)
{
  mcTransportAddress* pDestIP = new mcTransportAddress;
  memset(pDestIP, 0, sizeof(mcTransportAddress));
  CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
  CConfIpParameters* pServiceParams = pIpServiceListManager->FindServiceBySIPDomain(pDestAddr);
  if (pServiceParams == NULL)
    stringToIp(pDestIP, (char*)pDestAddr);
  else
  {
    CONF_IP_PARAMS_S* confParams = pServiceParams->GetConfIpParamsStruct();
    eIpType ipTypeFromStruct = confParams->service_ip_protocol_types;
    if ((eIpType_IpV4 == ipTypeFromStruct) || (eIpType_Both == ipTypeFromStruct))
    {
      pDestIP->addr.v4.ip = confParams->cs_ipV4.v4.ip;
      pDestIP->ipVersion = eIpVersion4;
    }
    else if (eIpType_IpV6 == ipTypeFromStruct)
    {
      memcpy(pDestIP->addr.v6.ip, confParams->cs_ipV6Array[0].v6.ip, sizeof(pDestIP->addr.v6.ip));
      pDestIP->ipVersion = eIpVersion6;
    }
  }
  return pDestIP;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CReceptionSip::IsPartyDefined(DWORD confMonitorId)
{
  PTRACE(eLevelInfoNormal, "ReceptionSip::IsPartyDefined Searching for sip defined party");
  
  BOOL isDefined = NO;
  const mcTransportAddress* pPhoneLenOrIP = ((CSipNetSetup *)m_pSuspended_NetSetUp)->GetTaSrcPartyAddr();
  const char* aliasString  = ((CSipNetSetup *)m_pSuspended_NetSetUp)->GetRemoteSipAddress();
  
  CCommRes* pMR = ::GetpMeetingRoomDB()->GetCurrentRsrv(m_confMonitorId);
  if(NULL != pMR)
    {
      isDefined = pMR->IsPartyDefined(pPhoneLenOrIP,aliasString, (DWORD&)m_pParty, SIP_INTERFACE_TYPE);
      POBJDELETE(pMR);
    }
  else
    {
      PTRACE(eLevelInfoNormal, "CReceptionH323::IsPartyDefined : MR NOT FOUND!!!");
    }
  
  return isDefined;
}
/////////////////////////////////////////////////////////////////////////////////
//void CReceptionSip::SetMsConversationId(const sipSdpAndHeadersSt* pSdpAndHeaders)
void CReceptionSip::SetClickToConfId(const sipSdpAndHeadersSt* pSdpAndHeaders)
{
	if (pSdpAndHeaders->sipHeadersLength)
	{
		BYTE* pStart = (BYTE*) &(pSdpAndHeaders->capsAndHeaders[pSdpAndHeaders->sipHeadersOffset]);
		sipMessageHeaders* pHeaders = (sipMessageHeaders*) pStart;
		//char msConvId[MS_CONVERSATION_ID_LEN];
		//msConvId[0] = '\0';
		//::SipGetHeaderValue(pHeaders,kMsConversationId, msConvId);
	//	SetMsConversationId(ConfId);
		char ConfId[MS_CONVERSATION_ID_LEN];
		ConfId[0] = '\0';
		::SipGetHeaderValue(pHeaders,kClick2Conf, ConfId, MS_CONVERSATION_ID_LEN);
		SetClickToConfId(ConfId);
	}
}
/////////////////////////////////////////////////////////////////////////////
//void CReceptionSip::SetMsConversationId(char* msConvId)
void CReceptionSip::SetClickToConfId(char* ConfId)
{
	//strncpy(m_msConversationId, msConvId, MS_CONVERSATION_ID_LEN - 1);  // For KlockWork
	//m_msConversationId[MS_CONVERSATION_ID_LEN-1] = '\0';
	strncpy(m_ClickToConfId, ConfId, MS_CONVERSATION_ID_LEN - 1);  // For KlockWork
	m_ClickToConfId[MS_CONVERSATION_ID_LEN-1] = '\0';

}
/////////////////////////////////////////////////////////////////////////////////
/*const char* CReceptionSip::GetMsConversationId() const
{
	return m_msConversationId;
}
*/
/////////////////////////////////////////////////////////////////////////////////
const char* CReceptionSip::GetClickToConfId() const
{
	return m_ClickToConfId;
}
