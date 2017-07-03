//=================================================================================================
//
//Copyright (C) 2000 ACCORD Networks Ltd.
//This file contains confidential information proprietary to ACCORD Networks Ltd. The use or 
//disclosure of any information contained in this file without the written consent of an officer of
//ACCORD Networks Ltd. is expressly forbidden.
//
//=================================================================================================

//=================================================================================================
//
//Module Name:  PartyVoice
//
//General Description:  
//
//    1. 
//    2.
//
//Generated By:                                  Date: 
//
//Revisions and Updates: 
//
//Date         Updated By         Description
//========   ==============   =====================================================================
//18.5.05		Talya			Inserted to Carmel
//=================================================================================================

//===== Include Files =====
#include "NStream.h"
#include "ConfApi.h"
#include "IsdnVoiceParty.h"
#include "PartyApi.h"
#include "OpcodesMcmsCommon.h"
#include "ConfPartyOpcodes.h"
#include "IsdnNetSetup.h"
#include "Trace.h"
#include "StatusesGeneral.h"
#include "IpMfaOpcodes.h"
#include "TraceStream.h"
#include "ConfPartyGlobals.h"
#include "ConfPartyRoutingTable.h"



//const WORD   VTX_SETUP		 = 5;

//~~~~~~~~~~~~~~ Global functions ~~~~~~~~~~~~~~~~~~~~~~~~~~
extern CConfPartyRoutingTable* GetpConfPartyRoutingTable( void );

//----------------------------------------------------------------------------
void PartyVoiceEntryPoint(void* appParam)
{  
  CIsdnVoiceParty*  pPartyTaskApp = new CIsdnVoiceParty;
  pPartyTaskApp->Create(*(CSegment*)appParam);
}

//----------------------------------------------------------------------------
PBEGIN_MESSAGE_MAP(CIsdnVoiceParty)
//  ONEVENT(AUDVALID     ,PARTYCONNECTED     ,CIsdnVoiceParty::NullActionFunction)
//  ONEVENT(VCUTOUT      ,PARTYCONNECTED     ,CIsdnVoiceParty::NullActionFunction)                                                          

//  //carmel instead of sending destroy=CONFDISCONNECT overwrite function self kill
  ONEVENT(CONFDISCONNECT ,ANYCASE    ,CIsdnVoiceParty::OnConfDisconnectANYSTATE)   

  ONEVENT(DELNETCHNL     ,IDLE       	    ,CIsdnVoiceParty::OnConfDelNetCntlIdle)
  ONEVENT(DELNETCHNL     ,PARTYCONNECTED	,CIsdnVoiceParty::OnConfDelNetCntl)
  ONEVENT(DELNETCHNL     ,PARTYSETUP        ,CIsdnVoiceParty::OnConfDelNetCntl)
  
  ONEVENT(UPDATERSRCCONFID,ANYCASE, CIsdnVoiceParty::OnPartyUpdateConfRsrcIdForInterfaceAnycase)
//  ONEVENT(DELNETCHNL     ,VTX_SETUP  ,CIsdnVoiceParty::OnConfDelNetCntl)
//  ONEVENT(DISCONNECT     ,VTX_SETUP  ,CIsdnParty::OnNetDisconnectSetUp)
//  ONEVENT(VTX_STATUS_IND ,CONNECT	   ,CIsdnVoiceParty::OnAudioPlusVTXStatusIndConnect)  
PEND_MESSAGE_MAP(CIsdnVoiceParty,CIsdnParty);   

//----------------------------------------------------------------------------
CIsdnVoiceParty::CIsdnVoiceParty() 
{

}

//----------------------------------------------------------------------------
CIsdnVoiceParty::~CIsdnVoiceParty() 
{
}

//----------------------------------------------------------------------------
void  CIsdnVoiceParty::Create(CSegment& appParam)     
{      
  CParty::Create(appParam);   
}

//----------------------------------------------------------------------------
void  CIsdnVoiceParty::OnConfDelNetCntl(CSegment* pParam)
{
  TRACESTR (eLevelInfoNormal) <<"CIsdnVoiceParty::OnConfDelNetCntl - " << PARTYNAME;
	m_state = PARTYDISCONNECTING;
//	if (m_recordingManager && m_ivrCtrl) {
//		m_ivrCtrl->PartyNetDisconnected(STOP_INITIATOR_CONF);	// block activities
//		ConfStopRecording();	// initiator is Conf
//	} else 
		DisconnectNetChnlCntl(0);
}

//----------------------------------------------------------------------------
void  CIsdnVoiceParty::OnConfDelNetCntlIdle(CSegment* pParam)
{
	m_state = PARTYDISCONNECTING;
 // net control is already disconnected
    CPartyApi*  pTaskApi = new CPartyApi;
    pTaskApi->CreateOnlyApi(this->GetRcvMbx());
    pTaskApi->SetLocalMbx(this->GetLocalQueue());
    pTaskApi->EndNetDisConnect(0,statOK);
    pTaskApi->DestroyOnlyApi();
    POBJDELETE(pTaskApi);
}

//----------------------------------------------------------------------------
void  CIsdnVoiceParty::OnConfDisconnectANYSTATE(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CIsdnVoiceParty::OnConfDisconnectANYSTATE : Name - ",PARTYNAME);
  CleanUp(1);
}

/*   
/////////////////////////////////////////////////////////////////////////////
//					OnAudioPlusVTXStatusIndConnect
//					We Rcv update on current VTX algorithm from Audio
//					Only sends new ComMode to MMI
/////////////////////////////////////////////////////////////////////////////
void  CIsdnVoiceParty::OnAudioPlusVTXStatusIndConnect(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CIsdnVoiceParty::OnAudioPlusVTXStatusIndConnect : Name - ",PARTYNAME);
	
	BYTE EndPointVtxSupport;
	WORD VtxVoiceAlg;
	*pParam >> EndPointVtxSupport >> VtxVoiceAlg;

	CAudMode   aud_mode;
	aud_mode.SetAudMode(VtxVoiceAlg);
	CComMode* pNewComMode = new CComMode;
	*pNewComMode = *m_pCurrentComMode;
	pNewComMode->SetAudMode(aud_mode);
	pNewComMode->UpdateH221string();
	
	CSegment ComMode;
	pNewComMode->Serialize(NATIVE,ComMode);
	m_pConfApi->UpdateDB(this,REMOTECOMMODE,(DWORD) 0 ,1,&ComMode);   
	m_pConfApi->UpdateDB(this,CURCOMMODE,(DWORD) 0,1,&ComMode); 
	
	POBJDELETE(pNewComMode);
}
//----------------------------------------------------------------------------
void  CIsdnVoiceParty::StartVTXSetup() 
{
	PTRACE2(eLevelInfoNormal,"CIsdnVoiceParty::StartVTXSetup : Party Name - ",PARTYNAME);
	m_pConfApi->InformAudioNetConnected(m_pParty); //send to Audio Card NetConnectionStatus (Connected) - after Net connected.
	m_state = VTX_SETUP;
	StartTimer(VTX_SETUP_TIMEOUT, 30*SECOND);
}
*/

/////////////////////////////////////////////////////////////////////////////
void CIsdnVoiceParty::OnPartyUpdateConfRsrcIdForInterfaceAnycase(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CIsdnVoiceParty::OnPartyUpdateConfRsrcIdForInterfaceAnycase : Name - ",PARTYNAME);
  DWORD confRsrcId;
  
  *pParam >> confRsrcId;
  m_ConfRsrcId = confRsrcId;
  m_pNetChnlCntl[0]->SetNewConfRsrcId(confRsrcId);
}

/////////////////////////////////////////////////////////////////////////////
void CIsdnVoiceParty::PartySpecfiedIvrDelay()
{
	TRACEINTO << "ConfId:" << m_ConfRsrcId << ", PartyId:" << m_PartyRsrcID << " ,TIMER_START_IVR with 2 seconds delay";
	StartTimer(TIMER_START_IVR, 2*SECOND);
}


