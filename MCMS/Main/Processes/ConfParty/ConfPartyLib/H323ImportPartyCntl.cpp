//+========================================================================+
//                            H323ImportPartyCntl.cpp                      |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H323ImportPartyCntl.cpp                                     |
// SUBSYSTEM:  ConfParty                                                   |
// PROGRAMMER: GuyD                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
// GuyD| 13/11/05   |  Create + Stage 1 - Move between same conferences    |
//+========================================================================+

#include "H323ImportPartyCntl.h"
#include "ConfPartyOpcodes.h"
#include "StatusesGeneral.h"
#include "ConfApi.h"
#include "PartyApi.h"
#include "AudioBridgeInterface.h"
#include "ConfAppMngrInterface.h"
#include "VideoBridgeInterface.h"
#include "BridgeMoveParams.h"
#include "H323Caps.h"
 
PBEGIN_MESSAGE_MAP(CH323ImportPartyCntl)

	ONEVENT(PARTY_AUDIO_CONNECTED,					IDLE,	CH323ImportPartyCntl::OnAudConnect)
    ONEVENT(IMPORT_PARTY_CONNECT_TO_AUDIO_BRIDGE,	IDLE,	CH323ImportPartyCntl::OnTimerConnectToAudioBrdg)
    // Party connect event
    ONEVENT(H323PARTYCONNECTALL,					ANYCASE,CH323ImportPartyCntl::OnPartyH323ConnectAll) 	 
PEND_MESSAGE_MAP(CH323ImportPartyCntl,CH323PartyCntl);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

CH323ImportPartyCntl::CH323ImportPartyCntl() //Ctor
{
	
}

/////////////////////////////////////////////////////////////////////////////
CH323ImportPartyCntl::~CH323ImportPartyCntl() // Dtor
{
	
}

/////////////////////////////////////////////////////////////////////////////
CH323ImportPartyCntl& CH323ImportPartyCntl::operator =(const CH323ImportPartyCntl& other)
{
	(CH323PartyCntl&)*this = (CH323PartyCntl&)other;
	
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
void*  CH323ImportPartyCntl::GetMessageMap()                                        
{
  return (void*)m_msgEntries;    
}

/////////////////////////////////////////////////////////////////////////////
const char*   CH323ImportPartyCntl::NameOf()  const
{
  return "CH323ImportPartyCntl";
}

/////////////////////////////////////////////////////////////////////////////
// This function is the most importent function in the move party scenario.
// we create the CPartyCntl in the target coference for the new party as in CAddH323PartyCntl::Create
// but using existing allready opened channels, and resources (H323,audio).

void  CH323ImportPartyCntl::Create(CMoveIPImportParams* pMoveImportParams)
{
	PASSERT_AND_RETURN(! CPObject::IsValidPObjectPtr(pMoveImportParams->m_pImpConfPartyCntl));
	PASSERT_AND_RETURN(! CPObject::IsValidPObjectPtr(pMoveImportParams->m_pConf));

		
	m_state = IDLE;
	WORD IsVoiceOnlyParty = (pMoveImportParams->m_pImpConfPartyCntl)->GetVoice();
	
	// 1) copy data from source-party CPartyCntl
	//	  here we copy all the relevant party data that doesn't depend on the new conference. 
	SetDataForImportPartyCntl(pMoveImportParams->m_pImpConfPartyCntl);

    // 1.a  register all party control state machines in the current task
    if (m_pBridgeMoveParams)
    {
        m_pBridgeMoveParams->RegisterBridgePartyCntlsInTask();
    }
  
	// 2) set data from new conference
	//	  here we set the new conference relevant data in the CPartyCntl
	m_pConf        = pMoveImportParams->m_pConf;
	SetFullName(pMoveImportParams->m_pImpConfPartyCntl->GetName(),pMoveImportParams->m_name);
	PTRACE2PARTYID(eLevelInfoNormal, "CH323ImportPartyCntl::Create: Name - ", m_partyConfName, GetPartyRsrcId());

	m_pTaskApi = new CConfApi;
	m_pTaskApi->CreateOnlyApi((pMoveImportParams->m_pConf)->GetRcvMbx(),this);
	m_pTaskApi->SetLocalMbx((pMoveImportParams->m_pConf)->GetLocalQueue());

	UpdatePartyEntryInGlobalRsrcRoutingTblAfterMove();
	// reset flow control parameters
	m_bIsFlowControlParty = 0;
	m_flowControlRate 	= 0;
	m_DbaContentRate	= 0;
	
	
	if (pMoveImportParams->m_pAudBridgeInterface != NULL)
		m_pAudioInterface = (pMoveImportParams->m_pAudBridgeInterface);
		
	if (pMoveImportParams->m_pVidBridgeInterface != NULL)
		m_pVideoBridgeInterface = (pMoveImportParams->m_pVidBridgeInterface);
		
	if (pMoveImportParams->m_pConfAppMngrInterface != NULL)
		m_pConfAppMngrInterface = (pMoveImportParams->m_pConfAppMngrInterface);
				
	if (pMoveImportParams->m_pFECCBridge != NULL)
		m_pFECCBridge = (pMoveImportParams->m_pFECCBridge);
		
	if (pMoveImportParams->m_pContentBridge != NULL)
		m_pContentBridge = (pMoveImportParams->m_pContentBridge);
				
	if (pMoveImportParams->m_pTerminalNumberingManager != NULL)
		m_pTerminalNumberingManager = (pMoveImportParams->m_pTerminalNumberingManager);
				
	if (pMoveImportParams->m_pCopVideoTxModes != NULL)
	{
		POBJDELETE(m_pCopVideoTxModes);
		m_pCopVideoTxModes	= new CCopVideoTxModes(*(pMoveImportParams->m_pCopVideoTxModes));
	}
	
	// 3) set communication mode and local capabilities
	m_IsMovedParty = TRUE;

//	m_pIpInitialMode->SetConfType(kCp);//update according to new conf

/*	BYTE bSameLineRate = TRUE;
	if (! m_pScm->IsFreeVideoRate() ) //VSW
	{
		bSameLineRate = IsConfHasSameLineRateAsSourceConf();
		if (!bSameLineRate)
		{
			m_incomingFlowControlAction = RATE_STATUS_SECONDERY;
			m_outgoingFlowControlAction = RATE_STATUS_SECONDERY;
		}
	}

	BYTE bChangeRate = FALSE;
	if (bSameLineRate)
		bChangeRate = SetVideoFlowControlParameters(vid_bitrate,tdm_vid_bitrate);

	if (bChangeRate)
	{// The rates in party control adn in h323cntl must be equal:
	// Only if flow control is sent, the tdm buffer bytes is changed in h323cntl,
	// because we can't change tdm rate without change the channel.
		m_videoRate    = vid_bitrate;
		m_tdmVideoRate = tdm_vid_bitrate;
	}*/

	if (m_pIpInitialMode->GetConfType() == kCop)
	{
		m_pIpCurrentMode->SetCopTxLevel(0xFF);
		m_pIpInitialMode->SetCopTxLevel(0xFF);
	}
	else
		m_pIpInitialMode->SetVideoBitRate(m_videoRate, cmCapReceiveAndTransmit);

	// BRIDGE-15567: update SCM if Conference Profile is changed
	UpdateScmAccordingtoRes(pMoveImportParams->m_pImpConfPartyCntl);

	// 4) set mode for audio/video message
//	WORD newStatus;
//	SetNewPartyConnectionMode(newStatus,WelcomMode,pImpConfPartyCntl);

	// 5) set audio/video-connection
	


	//m_pPartyApi->SetRsrcConfIdForInterface(m_destResourceConfId);
	// 6) move resourses:
	// participant keeps its h323 recources (updete resource-allocator DB in function Move323Rsrc(...)
	// and keeps is audio resources ( by dealocate from old conf and capture in new conf)
	// video and data resource are dealocated from old conferenc and allocated in new conference
	// at CConf level (all conference video resources must be on the same card)
	// massage resources allocated/dealocaeted in depend of the attend state at source/target conf

		// a) move party resourses (H323,audio,message)
//	MovePatyResources(sysConfId,pImpConfPartyCntl,IsAudioPlus,IsVoiceOnlyParty);
	
	m_pPartyHWInterface	= new CPartyInterface(*((pMoveImportParams->m_pImpConfPartyCntl)->GetPartyCntlResource()));
	m_pPartyHWInterface->SetConfRsrcId(m_destResourceConfId);
//	*m_pPartyHWInterface = *((pMoveImportParams->m_pImpConfPartyCntl)->GetPartyCntlResource());
		// b) set resourses descriptors that allocated/dealocaeted at CConf level

	// set to new conf id only after using old conf id for dealocating resources
    m_monitorConfId    = pMoveImportParams->m_sysConfId;
//	RsrcDump();

	// 9) connect party audio
	// in H323 we connect with the audio alg from source conf
	// because change mode is not active
//	BYTE audAlgortim = m_pCurrentScm->GetAudMode();
	
/*	if((m_moveType== MOVE_INTO_IVR||m_moveType== MOVE_BACK_INTO_IVR)  && !IsIvr()){
		m_moveType = MOVE_DEFAULT;
	}*/
	
//    m_isChairEnabled = isChairEnabled;

	// Connecting to Audio bridge via CAM
	// m_pConfAppMngrInterface->ConnectAudio(
	

	CBridgePartyInitParams* pBrdgPartyInitParams = new CBridgePartyInitParams(m_name, m_pParty, 
																			  GetPartyRsrcId(), m_RoomId,
																			  GetInterfaceType(),
																			  NULL, NULL,
																			  m_pBridgeMoveParams->GetAndResetAudioBridgePartyCntlOnImport()); 
	bool isIvrAfterMove = false;
	if(m_moveType == eMoveIntoIvr)
		ON(isIvrAfterMove);
	m_pConfAppMngrInterface->ConnectPartyAudio(pBrdgPartyInitParams,isIvrAfterMove);	
	

	m_eAudBridgeConnState = eSendOpenInAndOut;	
	ON(m_bIsMemberInAudBridge);
	
	StartTimer(IMPORT_PARTY_CONNECT_TO_AUDIO_BRIDGE, 8*SECOND);
	
	
	//FECC:
	ConnectPartyToFECCBridge(m_pIpCurrentMode);	
	
	WORD refMcuNumber = 1;
	WORD refTerminalNumber = 1;
	if (m_pTerminalNumberingManager)
   	{
   		m_pTerminalNumberingManager->allocatePartyNumber(m_pParty, refMcuNumber, refTerminalNumber);
   		ON(m_isTerminalNumberingConn);
   	}
	else
		PASSERTMSG(GetPartyRsrcId(),"CH323ImportPartyCntl::Create - m_pTerminalNumberingManager not valid ");
		
	EConfType confType = m_pIpInitialMode->GetConfType();
	

    if (m_pConf->GetVideoOperationPointsSet())
    {
        m_pIpInitialMode->SetOperationPoints(m_pConf->GetVideoOperationPointsSet());
        m_pIpCurrentMode->SetOperationPoints(m_pConf->GetVideoOperationPointsSet());
    }
    else
    {
        TRACEINTO << " mix_mode: no operation points at conference level.";
    }
	m_eLastAllocatedVideoPartyType = pMoveImportParams->m_eLastAllocatedVideoPartyType;
	m_pPartyApi->SetMoveDestConfParams( (WORD)confType, refMcuNumber, refTerminalNumber, m_pCopVideoTxModes, m_pConf->GetVideoOperationPointsSet());

	POBJDELETE(pBrdgPartyInitParams);	
	//connect party audio in onhold message or normal mode :
/*	if  ( m_pAudConnection ) {
		
		m_pAudConnection->ImportParty(audAlgortim);
		
		if (m_moveType == MOVE_ONHOLD) 
		{ //build onhold mode parameters
			
			PTRACEPARTYID(eLevelInfoNormal,"CImportIpPartyCntl::Create: ------ start ONHOLD MSG mode -------");
			ChangeAudioConnectionModeToMessage(PARTY_MESSAGE_MODE,audAlgortim,AUDIO_ONHOLD,DUMMY_MUSIC_RATIO,DUMMY_MSG_VOLUME,DUMMY_MUSIC_VOLUME,m_pMsgDesc,m_pMusicDesc,m_avServiceName);
			if (GetAdvancedAudio()) {
				if (!IsIvr())
					SendMsgFromAvService( AUDIO_ONHOLD, 2 );
				else
					SendMsgFromIvr( AUDIO_ONHOLD, 2 );
			}
		}
		else {
			PTRACEPARTYID(eLevelInfoNormal,"CImportIpPartyCntl::Create: ------ start INCONF mode -------");
			if (GetAdvancedAudio())
				((CPartyAudPlusConnection *)m_pAudConnection)->BeepIfNeeded();

			WORD partyMode = PARTY_INCONF_MODE;
			if((m_moveType== MOVE_INTO_IVR || m_moveType==MOVE_BACK_INTO_IVR) && IsIvr())
				partyMode = PARTY_MESSAGE_MODE;

			ChangeAudioConnectionMode(partyMode, audAlgortim);
		}
	}
	else    {
		PTRACEPARTYID(eLevelError,"CImportIpPartyCntl::Create: No Audio Connection !!");
	}
*/
//	m_pChairCntl->Connect(m_name,m_pParty,m_pPartyApi->GetRcvMbx(),m_pMuxDesc,m_nodeType);

//    m_pTaskApi->SendPartyH230CommSeq(m_pParty);

	// in case someone ask for operator to join the conf: reset the request
/*	if (m_pConf->IsOperParty(m_name))
		m_pConf->ResetOperToConf();*/
}



/////////////////////////////////////////////////////////////////////////////
//              CImportIpPartyCntl Action Functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// This function ends the import side of the move party scenario successfuly.
// Audio connected end we continue to "SetCapCommonDenominator" and "ChangeMode" as any
// other new party in the conference.
void  CH323ImportPartyCntl::OnAudConnect(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323ImportPartyCntl::OnAudConnect", GetPartyRsrcId());
    DeleteTimer(IMPORT_PARTY_CONNECT_TO_AUDIO_BRIDGE);
    
    if (m_eAudBridgeConnState == eBridgeDisconnected) 
	{
		DBGPASSERT(GetPartyRsrcId());
		PTRACEPARTYID(eLevelInfoNormal,"CH323ImportPartyCntl::OnAudConnect : Connect has received after disconnect.", GetPartyRsrcId());
	}    
	
    HandleAudioBridgeConnectedInd(pParam);
  
 
	m_pTaskApi->EndImportParty(m_pParty->GetPartyId(), statOK);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323ImportPartyCntl::OnTimerConnectToAudioBrdg(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323ImportPartyCntl::OnTimerConnectToAudioBrdg", GetPartyRsrcId());
	PASSERT(GetPartyRsrcId());
	// Disconnect the party
	// TBD - Is there any thing to do before ? Matvey.
	m_pTaskApi->EndImportParty(m_pParty->GetPartyId(), statIllegal);
} 
////////////////////////////////////////////////////////////////////////////////////////
BYTE CH323ImportPartyCntl::IsRemoteAndLocalCapSetHasContent(eToPrint toPrint) const
{
	BYTE res = NO;
	if (m_pRmtCapH323 && m_pLocalCapH323)
	{
		if (m_pRmtCapH323->IsH239() && m_pLocalCapH323->IsH239())
			res = YES;
		else if (m_pRmtCapH323->IsEPC() && m_pLocalCapH323->IsEPC())
			res = YES;		
	}
	return res;
}
/////////////////////////////////////////////////////////////////////////////////////////
BYTE CH323ImportPartyCntl::IsRemoteAndLocalHasEPCContentOnly()
{
	BYTE res = NO;
	if (m_pRmtCapH323 && m_pLocalCapH323)
	{
		if (!m_pRmtCapH323->IsH239() || !m_pLocalCapH323->IsH239())
			if (m_pRmtCapH323->IsEPC() && m_pLocalCapH323->IsEPC())
				res = YES;

	}
	return res;
}
//////////////////////////////////////////////////////////////////////////////
void CH323ImportPartyCntl::OnPartyH323ConnectAll(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323ImportPartyCntl::OnPartyH323ConnectAll", GetPartyRsrcId());
	PartyH323ConnectAllPartyConnectAudioOrChangeAll(pParam);
	m_bIsPartyConnectAllWhileMove = TRUE;
}
//////////////////////////////////////////////////////////////////////////////
void CH323ImportPartyCntl::UpdateScmAccordingtoRes(CPartyCntl* pImpPartyCntl)
{
	const CCommConf* pCommConf = m_pConf->GetCommConf();
	if (pCommConf)
	{
		BYTE impTipContentCompatibility = eTipCompatibleNone;
		if (pImpPartyCntl->GetConf())
		{
			const CCommConf* pImpCommConf = pImpPartyCntl->GetConf()->GetCommConf();
			if (pImpCommConf)
				impTipContentCompatibility = pImpCommConf->GetIsTipCompatible();
			else
				impTipContentCompatibility = ((CIpPartyCntl*)pImpPartyCntl)->GetInitialMode()->GetTipContentMode();
		}

		if (pCommConf->GetIsTipCompatible() != impTipContentCompatibility)
		{
			m_pIpInitialMode->SetContentProtocolMode((ePresentationProtocol)(pCommConf->GetPresentationProtocol()));
			m_pIpInitialMode->SetTipContentMode(pCommConf->GetIsTipCompatible());			
		}
	}
}

//////////////////////////////////////////////////////////////////////////////

