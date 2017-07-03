// ReceptionIVR.cpp: implementation of the CReceptionIVR class.
//
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//Revisions and Updates: 
//
//Date         Updated By         Description
//
//17/7/05		Yoella			Handle move invoked by IVR
//========   ==============   =====================================================================

#include "ReceptionIVR.h"
#include "Lobby.h"

#include "ConfPartyManagerLocalApi.h"
#include "CommResShort.h"
#include "CommConfDB.h"
#include "ConfApi.h"
/////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//                            CReceptionIVR          
////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CReceptionIVR)

  //events from conference 
  //ONEVENT(RELEASE_UNRESERVED_PARTY,SUSPEND  ,CReceptionIVR::OnLobbyReleasePartySuspend)
  // events from conferance manager
  //ONEVENT(REJECT_UNRESERVED_PARTY, SUSPEND  ,CReceptionIVR::OnMcuRejectPartySuspend)
  ONEVENT(REJECT_UNRESERVED_PARTY, SUSPEND  ,CReceptionIVR::OnConfMngrRejectPartySuspend)
  //local events
  ONEVENT(PARTYSUSPENDTOUT,		SUSPEND  ,CReceptionIVR::OnTimerPartySuspend)

PEND_MESSAGE_MAP(CReceptionIVR,CReception); 
  
/////////////////////////////////////////////////////////////////////////////
CReceptionIVR::CReceptionIVR(CTaskApp *pOwnerTask)
        :CReception(pOwnerTask)
{
	m_pConfParty = NULL;
	//m_adHocConfName[0]    = '\0';
	m_adHocNumericID[0]   = '\0';

	VALIDATEMESSAGEMAP;
}
/////////////////////////////////////////////////////////////////////////////
CReceptionIVR::~CReceptionIVR() 
{

}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
void CReceptionIVR::SetRsrvPartyID(DWORD dwRsrvPartyId)
{
	m_rsrvPartyID = dwRsrvPartyId;
}
/////////////////////////////////////////////////////////////////////////////
DWORD CReceptionIVR::GetRsrvPartyID()
{
	return m_rsrvPartyID;
}

/////////////////////////////////////////////////////////////////////////////
// Create for party suspend in unattended call in.
void  CReceptionIVR::CreateSuspendIVR(COsQueue* pLobbyRcvMbx,CLobby* pLobby, CTaskApp* ListId,DWORD dwSourceConfId,
									 char* confName,DWORD dwPartyID,ETargetConfType targetConfType)
{ 
	m_state  = SUSPEND;  
	m_pLobby = pLobby;
	
	SetTargetConfName(confName);
	SetTargetConfType(targetConfType);
	
	//this value (ListId) is not a valid memory pointer, 
	//it is used only to identify the suspended CReception object in the waiting list
	SetParty(ListId); 
	SetMonitorMRId(dwSourceConfId);
	SetRsrvPartyID(dwPartyID);
	
	//SetAdHocNumericId(adHocNumericId);
	SetConfOnAirFlag(NO);
	
	PTRACE(eLevelInfoNormal,"CReceptionIVR::CreateSuspendIVR : StartTimer(PARTYSUSPENDTOUT,30*SECOND)");
	//will remove this item from the list in 30 seconds if no response recived from ConfParyMngr
	StartTimer(PARTYSUSPENDTOUT,30*SECOND); 
	
}

/////////////////////////////////////////////////////////////////////////////
// Create Ad Hoc conference / Activate MR
void  CReceptionIVR::CreateConfAndSuspendIVRMove(COsQueue* pLobbyRcvMbx,CLobby* pLobby, CTaskApp* ListId,
						DWORD dwSourceConfId,char* confName, DWORD dwPartyID,ETargetConfType targetConfType,CCommRes* pAdHocRsrv)
{ 	
	if(targetConfType == eAdHoc)
	{
		if (NULL == pAdHocRsrv)
			PASSERT(1);
		else
		 confName = (char*)pAdHocRsrv->GetName();
	}
	
	CreateSuspendIVR(pLobbyRcvMbx,pLobby,ListId,dwSourceConfId,confName,dwPartyID,targetConfType);
    
	CConfPartyManagerLocalApi confPartyMngrApi;
	if(eAdHoc == targetConfType)
		confPartyMngrApi.LobbyStartAdHocConf(dwSourceConfId,pAdHocRsrv );
		
	else if (eMeetingRoom == targetConfType)
	{
	   CCommResShort * pTmpShort = ::GetpMeetingRoomDB()->GetCurrentRsrvShort(confName);
	   if( pTmpShort)
	   {
		  DWORD mrID=pTmpShort->GetConferenceId ();//::GetpMeetingRoomDB()->GetCurrentRsrv(confName)->GetMonitorConfId();
		  confPartyMngrApi.StartMeetingRoom(mrID);
		  POBJDELETE(pTmpShort);
	   }
	}
}

/////////////////////////////////////////////////////////////////////////////
void*  CReceptionIVR::GetMessageMap()                                        
{
  return (void*)m_msgEntries;    
}

/////////////////////////////////////////////////////////////////////////////
void  CReceptionIVR::OnException()
{
	PTRACE(eLevelInfoNormal/*|LOBBY_TRACE*/,"CReceptionIVR::OnException : ");
	PASSERT(!m_pLobby);	 
	
    DeleteAllTimers();
    
	if(m_pPartyApi)
	{
		m_pPartyApi->Destroy();
		POBJDELETE(m_pPartyApi);
	}
	m_pLobby->OnEndPartyTransfer(this);
	delete this;
} 
 


/////////////////////////////////////////////////////////////////////////////
void  CReceptionIVR::OnTimerPartySuspend(CSegment* pParam)
{
	//	 In case the timer expired we need to deal with the move rejection!!!
	STATUS status = STATUS_OK;
	PTRACE(eLevelError,"CReceptionIVR::OnTimerPartySuspend : msg");
		
	//there is no longer need for this reception object

    DeleteAllTimers();
    
    status = ::GetpConfDB()->SearchPartyName(GetConfId(),GetRsrvPartyID());

   if (status == STATUS_OK)
   {
	   const CCommConf*  pCurCommConf  = ::GetpConfDB()->GetCurrentConf(GetConfId());
	   if(pCurCommConf)
	   {
		   const CConfParty* pCurConfParty = pCurCommConf->GetCurrentParty(GetRsrvPartyID());

		   char* curPartyName = NULL;
		   if (pCurConfParty)
		   {
			   curPartyName = (char* )pCurConfParty->GetName();

			   CConfApi confApi;
			   confApi.CreateOnlyApi(*(pCurCommConf->GetRcvMbx()));
			   confApi.DropParty(curPartyName);
			   confApi.DestroyOnlyApi();

		   }//party exist
		   else
		   {
			   PASSERT(GetRsrvPartyID());
			   PTRACE(eLevelError,"CReceptionIVR::OnTimerPartySuspend : Party is not found in DB");
		   }
	   }
	   else
	   {
		   PASSERT(GetConfId());
		   PTRACE(eLevelError,"CReceptionIVR::OnTimerPartySuspend : Source conference is not found in DB");
	   }
  }//STATUS_OK
   
    
	m_pLobby->RemoveReception(this);
	delete this;
}
///////////////////////////////////////////////////////////////////////////////
void CReceptionIVR::OnConfMngrRejectPartySuspend(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CReceptionIVR::OnConfMngrRejectPartySuspend : ");
	CCommConf*  pCommConf = ::GetpConfDB()->GetCurrentConf(m_confMonitorId);
	if(IsValidPObjectPtr( pCommConf))
	{
	  COsQueue* pConfRcvMbx =pCommConf->GetRcvMbx();
	  CConfApi confApi;
	  confApi.CreateOnlyApi(*pConfRcvMbx);
	  const char* party_name = NULL;
	  party_name = ::GetpConfDB()->GetPartyName(m_confMonitorId,m_rsrvPartyID);
	  if(party_name)
	          confApi.DropParty(party_name,0);        // 0 == delete   
	 }


	CReception::OnConfMngrRejectPartySuspend(pParam);
	
}
///////////////////////////////////////////////////////////////////////////////
void  CReceptionIVR::AcceptParty(CNetSetup* pNetSetUp,CCommConf* pCommConf,CConfParty* pConfParty)
{
  
  DWORD dwSourceConfId = GetConfId();
  //DWORD dwPartyId = pConfParty->GetPartyId();
  DWORD dwPartyId = GetRsrvPartyID();
  DWORD dwTargetConfId = pCommConf->GetMonitorConfId();
   
  CConfPartyManagerLocalApi confPartyLocalApi;
  confPartyLocalApi.MovePartyToConfOrMeetingRoom(dwSourceConfId, dwPartyId, eOnGoingConf,dwTargetConfId,NULL);

  CReception::AcceptParty(pNetSetUp,pCommConf,pConfParty);
}
/////////////////////////////////////////////////////////////////////////////////
//void  CReceptionIVR::SetAdHocConfName(char* adHocConfName)
//{
//   strncpy(m_adHocConfName,adHocConfName,H243_NAME_LEN);
//}

///////////////////////////////////////////////////////////////////////////////
void  CReceptionIVR::SetAdHocNumericId(char* adHocNumericId)
{
   strncpy(m_adHocNumericID,adHocNumericId,sizeof(m_adHocNumericID) - 1);
   m_adHocNumericID[sizeof(m_adHocNumericID) - 1] = 0;
}

/////////////////////////////////////////////////////////////////////////////////
//char*  CReceptionIVR::GetAdHocConfName()
//{
//   return m_adHocConfName;
//}

///////////////////////////////////////////////////////////////////////////////
char*  CReceptionIVR::GetAdHocNumericId()
{
   return m_adHocNumericID ;
}





