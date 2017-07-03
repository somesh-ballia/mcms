//+========================================================================+
//                            VideoBridge.CPP                              |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       VideoBridge.CPP                                             |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Talya                                                       |
//-------------------------------------------------------------------------|
// Who  | Date  July-2005  | Description                                   |
//-------------------------------------------------------------------------|
//
//+========================================================================+


#include "VideoBridgeVsw.h"
#include "VideoBridgeInitParams.h"
#include "TaskApi.h"
#include "BridgePartyInitParams.h"
#include "VideoBridgePartyCntl.h"
#include "BridgePartyDisconnectParams.h"
#include "BridgePartyExportParams.h"
#include "BridgePartyVideoParams.h"
#include "TraceStream.h"
#include "ConfPartyGlobals.h"
#include "VideoBridgePartyInitParams.h"
#include "VideoApiDefinitions.h"
#include "Party.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~ global function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
LayoutType GetNewLayoutType(const BYTE oldLayoutType);

// Timers opcodes
//#define		SWITCH_TOUT							((WORD)202)
//#define		SWITCH_TIME_OUT_VALUE				1*SECOND

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//~~~~~~~~~~~~~~~~~~~~~~~~~~ state machine ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CVideoBridgeVSW)

	ONEVENT(SPEAKERS_CHANGED,		CONNECTED,		CVideoBridgeVSW::OnConfSpeakersChangedCONNECTED)
	ONEVENT(SPEAKERS_CHANGED,		INSWITCH,		CVideoBridgeVSW::OnConfSpeakersChangedINSWITCH)

	ONEVENT(SETCONFVIDLAYOUT_SEEMEALL,	CONNECTED,		CVideoBridgeVSW::OnConfSetConfVideoLayoutSeeMeAllCONNECTED)
	ONEVENT(SETCONFVIDLAYOUT_SEEMEALL,	INSWITCH,		CVideoBridgeVSW::OnConfSetConfVideoLayoutSeeMeAllINSWITCH)

    ONEVENT(SETCONFVIDLAYOUT_SEEMEPARTY,CONNECTED,		CVideoBridgeVSW::OnConfSetConfVideoLayoutSeeMePartyCONNECTED)
	ONEVENT(SETCONFVIDLAYOUT_SEEMEPARTY,INSWITCH,	    CVideoBridgeVSW::OnConfSetConfVideoLayoutSeeMePartyINSWITCH)

	ONEVENT(END_CHANGE_LAYOUT,			CONNECTED,		CVideoBridgeVSW::OnPartyEndChangeLayoutCONNECTED)
	ONEVENT(END_CHANGE_LAYOUT,			INSWITCH,		CVideoBridgeVSW::OnPartyEndChangeLayoutINSWITCH)
	ONEVENT(END_CHANGE_LAYOUT,			DISCONNECTING,	CVideoBridgeVSW::NullActionFunction)

	ONEVENT(VIDREFRESH,					CONNECTED,		CVideoBridgeVSW::OnConfVideoRefreshCONNECTED)
	ONEVENT(VIDREFRESH,					INSWITCH,		CVideoBridgeVSW::OnConfVideoRefreshINSWITCH)
	ONEVENT(VIDREFRESH,					DISCONNECTING,	CVideoBridgeVSW::NullActionFunction)

	ONEVENT(VIDREFRESH_VSW_ENC,			CONNECTED,		CVideoBridgeVSW::OnConfVideoRefreshEncoderCONNECTED)
	ONEVENT(VIDREFRESH_VSW_ENC,			INSWITCH,		CVideoBridgeVSW::OnConfVideoRefreshINSWITCH)
	ONEVENT(VIDREFRESH_VSW_ENC,			DISCONNECTING,	CVideoBridgeVSW::NullActionFunction)

	ONEVENT(VIDEOMUTE,					CONNECTED,		CVideoBridgeVSW::OnConfUpdateVideoMuteCONNECTED)
	ONEVENT(VIDEOMUTE,					INSWITCH,		CVideoBridgeVSW::OnConfUpdateVideoMuteINSWITCH)

	ONEVENT(ENDCONNECTPARTY,			CONNECTED,		CVideoBridgeVSW::OnEndPartyConnectCONNECTED)
	ONEVENT(ENDCONNECTPARTY,			INSWITCH,		CVideoBridgeVSW::OnEndPartyConnectINSWITCH)

	ONEVENT(ENDCONNECTPARTY_IVR_MODE,	INSWITCH,		CVideoBridgeVSW::OnEndPartyConnectIVRModeINSWITCH)


	ONEVENT(LECTURE_MODE_TOUT,			CONNECTED,		CVideoBridgeVSW::OnTimerLectureModeCONNECTED)
	ONEVENT(LECTURE_MODE_TOUT,			INSWITCH,		CVideoBridgeVSW::OnTimerLectureModeINSWITCH)

	ONEVENT(ENDDISCONNECTPARTY,			CONNECTED,		CVideoBridgeVSW::OnEndPartyDisConnectCONNECTED)
	ONEVENT(ENDDISCONNECTPARTY,			INSWITCH,		CVideoBridgeVSW::OnEndPartyDisConnectINSWITCH)
	ONEVENT(ENDDISCONNECTPARTY,			DISCONNECTING,		CVideoBridgeVSW::OnEndPartyDisConnectDISCONNECTING)



	ONEVENT(UPDATE_VIDEO_FLOW_CNTL_RATE,CONNECTED,		CVideoBridgeVSW::OnConfUpdateFlowControlRateCONNECTED)
	ONEVENT(UPDATE_VIDEO_FLOW_CNTL_RATE,INSWITCH,		CVideoBridgeVSW::OnConfUpdateFlowControlRateINSWITCH)

	//ONEVENT(AUDIO_SPEAKER_CHANGED,		CONNECTED,		CVideoBridgeVSW::OnConfAudioSpeakerChangedCONNECTED)
	//ONEVENT(AUDIO_SPEAKER_CHANGED,		INSWITCH,		CVideoBridgeVSW::OnConfAudioSpeakerChangedINSWITCH)

	ONEVENT(DELETED_PARTY_FROM_CONF,	INSWITCH,		CVideoBridgeVSW::OnConfDeletePartyFromConfINSWITCH)
	ONEVENT(SET_SITE_AND_VISUAL_NAME,	INSWITCH,		CVideoBridgeVSW::OnSetSiteNameINSWITCH)
	ONEVENT(DISCONNECTCONF,				INSWITCH,		CVideoBridgeVSW::OnConfDisConnectConfINSWITCH)
	ONEVENT(TERMINATE,					INSWITCH,		CVideoBridgeVSW::OnConfTerminateINSWITCH)
	ONEVENT(CONNECTPARTY,				INSWITCH,		CVideoBridgeVSW::OnConfConnectPartyINSWITCH)

	ONEVENT(SWITCH_TOUT,				INSWITCH,		CVideoBridgeVSW::OnTimerEndSwitchINSWITCH)
	ONEVENT(SWITCH_TOUT,                           DISCONNECTING,               CVideoBridgeVSW::NullActionFunction)

	ONEVENT(DISCONNECTPARTY,			INSWITCH,		CVideoBridgeVSW::OnConfDisConnectPartyINSWITCH)
	ONEVENT(UPDATELECTUREMODE,			INSWITCH,		CVideoBridgeVSW::OnConfSetLectureModeINSWITCH)

	ONEVENT(CONTENT_BRIDGE_START_PRESENTATION,	ANYCASE,	CVideoBridgeVSW::NullActionFunction)
	ONEVENT(CONTENT_BRIDGE_STOP_PRESENTATION,	ANYCASE,	CVideoBridgeVSW::NullActionFunction)
PEND_MESSAGE_MAP(CVideoBridgeVSW, CVideoBridge);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//~~~~~~~~~~~~~~~~~~~~~~~~~~ constructors & destructors ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVideoBridgeVSW::CVideoBridgeVSW ()
{
	m_pConfLayout = NULL;
	m_pConfSourceBridgeParty = NULL;
	m_pConfSourceLayout = NULL;
	m_pReservationLayout = NULL;
	m_SwitchInSwitch = 0;
	m_flowControlRate = 0; // No flow control rate is received
}
// ------------------------------------------------------------------------------------------------------------------------
CVideoBridgeVSW::~CVideoBridgeVSW ()
{
	POBJDELETE(m_pConfLayout);
	POBJDELETE(m_pConfSourceLayout);
	POBJDELETE(m_pReservationLayout);
}
// ------------------------------------------------------------------------------------------------------------------------
void CVideoBridgeVSW::Create (const CVideoBridgeInitParams* pVideoBridgeInitParams)
{
	PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::Create , ", m_pConfName);
	// validation test
	if ( ! CPObject::IsValidPObjectPtr(pVideoBridgeInitParams) ) {
		PASSERTMSG(1,"CVideoBridgeCP::Create - invalid VideoBridgeInitParams");
		m_pConfApi->EndVidBrdgConnect(statInconsistent);
		return;
	}
	// create base
	CVideoBridge::Create(pVideoBridgeInitParams);
	// get reservation layout
	m_pConfLayout = new CLayout(CP_LAYOUT_1X1,GetConfName());
	m_pConfSourceBridgeParty = NULL;
	m_pConfSourceLayout = new CLayout(CP_LAYOUT_1X1,GetConfName());
	m_pReservationLayout = new CLayout(CP_LAYOUT_1X1,GetConfName());
	GetForcesFromReservation(pVideoBridgeInitParams->GetCommConf());
	// block for same layout - currently no implemented for vsw
	m_IsSameLayout = NO;
	// update DB
	UpdateDB_ConfLayout();
	// change state and confirm conf
	m_state = CONNECTED;
	m_pConfApi->EndVidBrdgConnect(statOK);
}
// ------------------------------------------------------------------------------------------------------------------------
void  CVideoBridgeVSW::InitBridgeParams(const CBridgePartyInitParams* pBridgePartyInitParams)
{
	CBridge::InitBridgeParams(pBridgePartyInitParams);

	if(pBridgePartyInitParams->GetMediaOutParams())
	{
//		no visual effects in vsw, we do not initiate in order not to send them later
//		((CBridgePartyVideoOutParams*)pBridgePartyInitParams->GetMediaOutParams())->SetVisualEffects(m_pVisualEffects);
		((CBridgePartyVideoOutParams*)pBridgePartyInitParams->GetMediaOutParams())->SetLayoutType(CP_LAYOUT_1X1);

		ePartyLectureModeRole partyLectureModeRole = GetLectureModeRoleForParty(pBridgePartyInitParams->GetPartyName());
		((CBridgePartyVideoOutParams*)pBridgePartyInitParams->GetMediaOutParams())->SetPartyLectureModeRole(partyLectureModeRole);
		((CBridgePartyVideoOutParams*)pBridgePartyInitParams->GetMediaOutParams())->SetVideoQualityType(m_videoQuality);

	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//~~~~~~~~~~~~~~~~~~~~~~~~~~ object & state machine virtual functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------------------------------------------------------
const char* CVideoBridgeVSW::NameOf () const
{
	return "CVideoBridgeVSW";
}
// ------------------------------------------------------------------------------------------------------------------------
void*  CVideoBridgeVSW::GetMessageMap()
{
	return (void*)m_msgEntries;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Change Layout Scenario -  scenario api
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ---------------------------------------------------------------------------------------------------------------------------------
// entry point to change layout scenario, this  function:
// 1) check if the layout changed (or new party added)
// 2) update layout data members, and mark new speaker for resync
// 3) start the switch
void CVideoBridgeVSW::ChangeLayout(CVideoBridgePartyCntl* pAddParty)
{
	PTRACE2(eLevelInfoNormal, "CVideoBridgeVSW::ChangeLayout , ConfName - ", m_pConfName);

	// check switch in switch
	if (m_state == INSWITCH)
	{
		PTRACE2(eLevelInfoNormal, "CVideoBridgeVSW::ChangeLayout ChangeLayout received in INSWITCH state - set switch in switch, ConfName - ", m_pConfName);
		SetSwitchInSwitch(pAddParty);
		return;
	}
	// initiate temporary layout
	CLayout confLayout(CP_LAYOUT_1X1, m_pConfName);
	CVideoBridgePartyCntl* confSource = NULL;
	CLayout confSourceLayout(CP_LAYOUT_1X1, m_pConfName);
	// build layout and check if changed
	WORD build_layout_succeeded = BuildLayout(confLayout, confSource, confSourceLayout);
	if (build_layout_succeeded == FALSE)
	{
		PTRACE2(eLevelInfoNormal, "CVideoBridgeVSW::ChangeLayout : build layout failed , ConfName - ", m_pConfName);
		// we update the data members in order to set the layout to null after last party disconnection
		UpdateLayoutData(confLayout, confSource, confSourceLayout, pAddParty); // update data members
		return;
	}

	BYTE is_conf_layout_changed = IsConfLayoutChanged(confLayout);
	BYTE is_conf_source_layout_changed = IsConfSourceLayoutChanged(confSource, confSourceLayout);

	//VNGFE-2028
	//In case there are only 2 parties in the conf
	// there is no need to change the layout for every speaker switch.
	BYTE isLayoutChangedFor2PartiesSwitch = IsLayoutChangeAfterSpeakerChangeForConfWith2Parties(confSource, confSourceLayout, confLayout);
	if (!isLayoutChangedFor2PartiesSwitch)
	{
		PTRACE2(eLevelInfoNormal, "CVideoBridgeVSW::ChangeLayout : 2 parties in conf - No need to change layout , ConfName - ", m_pConfName);
		is_conf_layout_changed = FALSE;
		is_conf_source_layout_changed = FALSE;
		UpdateLayoutData(confLayout, confSource, confSourceLayout, pAddParty); // update data members
	}

	CSmallString sstr;
	sstr << "is_conf_layout_changed = " << is_conf_layout_changed << ", is_conf_source_layout_changed = " << is_conf_source_layout_changed;
	PTRACE2(eLevelInfoNormal, "CVideoBridgeVSW::ChangeLayout: ", sstr.GetString());

	// update and send change layout
	PTRACE(eLevelInfoNormal, "CVideoBridgeVSW::ChangeLayout , old layout:");
	BYTE isOneOfPartiesLayoutChanged = FALSE;
	CVideoBridgePartyCntl* pCurrParty = NULL;
	CLayout* pPartyCurrLayout = NULL;
	CLayout* pPartyNewLayout = NULL;
	BYTE isPartyLayoutChanged;

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
		{
			isPartyLayoutChanged = FALSE;
			pPartyNewLayout = FindBestLayoutForParty(&confLayout, pPartyCntl, confSource, &confSourceLayout, isPartyLayoutChanged);
			if (isPartyLayoutChanged)
			{
				ON(isOneOfPartiesLayoutChanged);
				break;
			}
		}
	}

	if (is_conf_layout_changed || is_conf_source_layout_changed || pAddParty || isOneOfPartiesLayoutChanged)
	{
		UpdateLayoutData(confLayout, confSource, confSourceLayout, pAddParty); // update data members
		PTRACE(eLevelInfoNormal, "CVideoBridgeVSW::ChangeLayout , new layout:");

		StartSwitch(m_pConfLayout, m_pConfSourceBridgeParty, m_pConfSourceLayout, is_conf_layout_changed, is_conf_source_layout_changed, isOneOfPartiesLayoutChanged, pAddParty);
	}
	else
	{
		PTRACE2(eLevelInfoNormal, "CVideoBridgeVSW::ChangeLayout : layout did not change , ConfName - ", m_pConfName);
		// end switch will ensure that state will be change to connected and intra req will be send (case switch in switch and no change in layout)
		EndSwitch();
	}
}
// ---------------------------------------------------------------------------------------------------------------------------------
void CVideoBridgeVSW::StartSwitch(CLayout* confLayout,CVideoBridgePartyCntl* confSource,CLayout* confSourceLayout,BYTE is_conf_layout_changed, BYTE is_conf_source_layout_changed,BYTE is_one_of_parties_layout_changed,CVideoBridgePartyCntl* pAddParty)
{
	PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::StartSwitch , ConfName - ", m_pConfName);
	m_state = INSWITCH;
	StartTimer(SWITCH_TOUT,SWITCH_TIME_OUT_VALUE);
	// send change layout
	SendChangeLayout(confLayout,confSource,confSourceLayout,is_conf_layout_changed,is_conf_source_layout_changed,is_one_of_parties_layout_changed,pAddParty);
}

// ---------------------------------------------------------------------------------------------------------------------------------
void CVideoBridgeVSW::OnPartyEndChangeLayoutINSWITCH(CSegment* pParam)
{
	CParty* pParty = NULL;
	WORD status = statIllegal;

	*pParam >> (void*&)pParty >> status;

	PASSERT_AND_RETURN(!pParty);

	CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(pParty->GetPartyRsrcID());
	const CTaskApp* pPartyVideoSource = (pPartyCntl) ? GetVideoSource(pPartyCntl) : NULL;

	SendToPartyVIN(pParty, pPartyVideoSource);

	// num of participants waiting for ack is tested by inswitch state in video out
	WORD num_of_waiting_for_ack = GetNumOfChangeLayoutWaitingForAck();
	if (num_of_waiting_for_ack == 0)
	{
		EndSwitch();
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------
// this function should not be called on normal scenario (it may be called after if ack received after timer)
void CVideoBridgeVSW::OnPartyEndChangeLayoutCONNECTED(CSegment* pParam)
{
	CParty* pParty = NULL;
	WORD status = statIllegal;

	*pParam >> (void*&)pParty >> status;

	CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(pParty->GetPartyRsrcID());
	const CTaskApp* pPartyVideoSource = (pPartyCntl) ? GetVideoSource(pPartyCntl) : NULL;

	SendToPartyVIN(pParty, pPartyVideoSource);
}

// ---------------------------------------------------------------------------------------------------------------------------------
// end of swtich, this function:
// 1) ask inra from new video source (marked previousely)
// 2) update DB
// 3) continue to new switch in switch
WORD CVideoBridgeVSW::EndSwitch()
{
	PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::EndSwitch , ConfName - ", m_pConfName);
	DeleteTimer(SWITCH_TOUT);
	if(DISCONNECTING == m_state){
	  PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::EndSwitch , ConfName - EndSwitch received in disconnecting state ", m_pConfName);
	  return TRUE;
	}
	m_state = CONNECTED;
	if(m_SwitchInSwitch){
		PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::EndSwitch -  Switch In Switch starting new change layout - ", m_pConfName);
		m_SwitchInSwitch = NO;
		ChangeLayout();
	}else{
		// start of intra request
		AskIntraFromNewVideoSource();
		// update DB
		UpdateDB_ConfLayout();
	}

	return TRUE;
}
// ---------------------------------------------------------------------------------------------------------------------------------
// if not all change layout ack received
// 1) print those whom not received (for debug)
// 2) end the switch
void CVideoBridgeVSW::OnTimerEndSwitchINSWITCH(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"OnTimerEndSwitchINSWITCH , ConfName - ", m_pConfName);
	// dump parties waiting for ack
	GetNumOfChangeLayoutWaitingForAck(1);
	// continue to end switch
	EndSwitch();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//~~~~~~~~~~~~~~~~~~~~~~~~~~ State machine action functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------------------------------------------------------
void CVideoBridgeVSW::OnConfSpeakersChangedCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnConfVideoSpeakerChangedCONNECTED : Name - ",m_pConfName);
	OnConfSpeakersChanged(pParam);
}
// ------------------------------------------------------------------------------------------------------------------------
void CVideoBridgeVSW::OnConfSpeakersChangedINSWITCH(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnConfVideoSpeakerChangedINSWITCH : Name - ",m_pConfName);
	OnConfSpeakersChanged(pParam);
}
// ------------------------------------------------------------------------------------------------------------------------
void CVideoBridgeVSW::OnConfSpeakersChanged(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnConfSpeakersChanged : Name - ",m_pConfName);
	CTaskApp* pNewVideoSpeaker = NULL;
	CTaskApp* pNewAudioSpeaker = NULL;

	BYTE rShouldResend = NO;
	*pParam >> (void*&)pNewVideoSpeaker
	        >> (void*&)pNewAudioSpeaker
	        >> rShouldResend;
	if(rShouldResend == YES)
	  {
	    m_pLastActiveVideoSpeakerRequest = NULL;
	    m_pLastActiveAudioSpeakerRequest= NULL;
	  }
	if(m_pLastActiveVideoSpeakerRequest == pNewVideoSpeaker && m_pLastActiveAudioSpeakerRequest == pNewAudioSpeaker )
	{
		PTRACE(eLevelInfoNormal,"CVideoBridgeVSW::OnConfSpeakersChanged - Neither speaker changed !!!");
		return;
	}
	// video speaker change
	if(m_pLastActiveVideoSpeakerRequest != pNewVideoSpeaker)
	{
		PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnConfSpeakersChanged::Video Speaker changed - : Name - ",m_pConfName);
		m_pLastActiveVideoSpeakerRequest = pNewVideoSpeaker;
		BYTE  is_change_layout = UpdateNewVideoSpeaker();
		if(is_change_layout)
			ChangeLayout();
	}
	// audio speake changed
	if(m_pLastActiveAudioSpeakerRequest != pNewAudioSpeaker)
	{
		PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnConfSpeakersChanged::Audio Speaker changed - : Name - ",m_pConfName);
		m_pLastActiveAudioSpeakerRequest = pNewAudioSpeaker;
		AudioSpeakerChanged();

	}
}
// ------------------------------------------------------------------------------------------------------------------------
void  CVideoBridgeVSW::OnConfSetConfVideoLayoutSeeMeAllCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnConfSetConfVideoLayoutSeeMeAllCONNECTED : Name - ",m_pConfName);
	OnConfSetConfVideoLayoutSeeMeAll(pParam);
}
// ------------------------------------------------------------------------------------------------------------------------
void  CVideoBridgeVSW::OnConfSetConfVideoLayoutSeeMeAllINSWITCH(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnConfSetConfVideoLayoutSeeMeAllINSWITCH : Name - ",m_pConfName);
	OnConfSetConfVideoLayoutSeeMeAll(pParam);
}
// ------------------------------------------------------------------------------------------------------------------------
void CVideoBridgeVSW::OnConfSetConfVideoLayoutSeeMePartyCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnConfSetConfVideoLayoutSeeMePartyCONNECTED: Name - ",m_pConfName);
	OnConfSetConfVideoLayoutSeeMeParty(pParam);
}
// ------------------------------------------------------------------------------------------------------------------------
void CVideoBridgeVSW::OnConfSetConfVideoLayoutSeeMePartyINSWITCH(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnConfSetConfVideoLayoutSeeMePartyINSWITCH: Name - ",m_pConfName);
	OnConfSetConfVideoLayoutSeeMeParty(pParam);
}
// ------------------------------------------------------------
void  CVideoBridgeVSW::OnConfSetConfVideoLayoutSeeMeParty(CSegment* pParam)
{
	//set conf video layout requested by operator in Party Level
	PTRACE2(eLevelInfoNormal,"CVideoBridgVSW::OnConfSetConfVideoLayoutSeeMeParty : Name - ",m_pConfName);

	char            targetPartyName[H243_NAME_LEN];
	CVideoLayout   layout;

	*pParam >> targetPartyName;
	layout.DeSerialize(NATIVE,*pParam );
	targetPartyName[H243_NAME_LEN-1]='\0';

	CVideoBridgePartyCntl* pCurrParty;
	pCurrParty=(CVideoBridgePartyCntl*)GetPartyCntl(targetPartyName);
	DBGPASSERT_AND_RETURN(pCurrParty==NIL(CVideoBridgePartyCntl));
	((CVideoBridgePartyCntlVSW*)pCurrParty)->SetPartyForce(layout);
	ChangeLayout(pCurrParty);
}
// ------------------------------------------------------------------------------------------------------------------------
void  CVideoBridgeVSW::OnConfSetConfVideoLayoutSeeMeAll(CSegment* pParam)
{
	BYTE is_resrevation_Layout_changed = UpdateReservationLayout(pParam);
	if(is_resrevation_Layout_changed){
		ChangeLayout();
	}
}
// ------------------------------------------------------------------------------------------------------------------------

void CVideoBridgeVSW::OnConfVideoRefreshEncoderCONNECTED(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CVideoBridgeVSW::OnConfVideoRefreshEncoderCONNECTED");
	OnConfVideoRefresh(pParam, false, VIDREFRESH);
}

void  CVideoBridgeVSW::OnConfVideoRefreshCONNECTED(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CVideoBridgeVSW::OnConfVideoRefreshCONNECTED");
	OnConfVideoRefresh(pParam, true, VIDREFRESH);
}

void CVideoBridgeVSW::OnConfVideoRefresh(CSegment* pParam, bool bPartyIntraSuppress, WORD wEvent)
{
	if (CProcessBase::GetProcess()->GetProductFamily() == eProductFamilyCallGenerator)
	{
		//CallG_Keren
		PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnConfVideoRefresh in Call Generator  ",m_pConfName);
		OnConfVideoRefreshForCallGen(pParam);
		//CallG_Keren
		return;
	}

	PartyRsrcID partyRsrcID = INVALID;
	*pParam >> partyRsrcID;

	CVideoBridgePartyCntl*  pTargePartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyRsrcID);
	if ( !pTargePartyCntl )
	{
		PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnConfVideoRefresh: the party is not connected to brdg, Name - ",m_pConfName);
		return;
	}
	else
	{
		PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnConfVideoRefresh: request from party ",pTargePartyCntl->GetName());
	}

	// Romem - 6.1.09 - vngr-9460
	if(pTargePartyCntl->IsConnectedStandalone())
	{
		PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnConfVideoRefresh: Party is in slide state, ask MFA for intra. request from party ",pTargePartyCntl->GetName());
		pTargePartyCntl->FastUpdate();
		return;
	}
	if (bPartyIntraSuppress && pTargePartyCntl->IsPartyIntraSuppressed())
	{
		PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnConfVideoRefresh: Party is intra suppressed ignore this intra request ",pTargePartyCntl->GetName());
		return;
	}
	CVideoBridgePartyCntl*  pVideoSourcePartyCntl = GetVideoSourceCntl(pTargePartyCntl);
	if ( !pVideoSourcePartyCntl )
	{
		PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnConfVideoRefresh: the video source is NULL, Name - ",m_pConfName);
		return;
	}
	else
	{
		PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnConfVideoRefreshCONNECTED : video source is ",pVideoSourcePartyCntl->GetName());
	}
	// ask intra from video source
	pVideoSourcePartyCntl->HandleEvent(NULL, 0, wEvent);
}

// ------------------------------------------------------------------------------------------------------------------------
void  CVideoBridgeVSW::OnConfVideoRefreshINSWITCH(CSegment* pParam)
{
	if (CProcessBase::GetProcess()->GetProductFamily() == eProductFamilyCallGenerator)
	{
		//CallG_Keren
		PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnConfVideoRefreshINSWITCH in Call Generator  ",m_pConfName);
		OnConfVideoRefreshForCallGen(pParam);
		//CallG_Keren
		return;
	}

	PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnConfVideoRefreshINSWITCH , video source will refresh at the end of switch ",m_pConfName);

	PartyRsrcID partyRsrcID = INVALID;
	*pParam >> partyRsrcID;

	CVideoBridgePartyCntl*  pTargePartyCntl = ( CVideoBridgePartyCntl*)GetPartyCntl(partyRsrcID);
	if ( !pTargePartyCntl )
	{
		PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnConfVideoRefreshINSWITCH :the party is not connected to brdg, Name - ",m_pConfName);
		return;
	}else{
		PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnConfVideoRefreshINSWITCH : request from party ",pTargePartyCntl->GetName());

	}
	CVideoBridgePartyCntl*  pVideoSourcePartyCntl = GetVideoSourceCntl(pTargePartyCntl);
	if ( !pVideoSourcePartyCntl )
	{
		PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnConfVideoRefreshINSWITCH :the video source is NULL, Name - ",m_pConfName);
		return;
	}else{
		PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnConfVideoRefreshINSWITCH : video source is ",pVideoSourcePartyCntl->GetName());
	}
	// mark party as video source - intra will be requested at the end of switch
	pVideoSourcePartyCntl->MarkAsNewVideoSource(1);
}
//-------------------------------------------------------------------------------------------------------------------------
void  CVideoBridgeVSW::OnConfVideoRefreshForCallGen(CSegment* pParam)
{
	if (CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator)
	{
		PTRACE(eLevelInfoNormal,"CVideoBridgeVSW::OnConfVideoRefreshForCallGen - ERROR - system is not CG!!");
		return;
	}

	//CallG_Keren
	PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnConfVideoRefreshForCallGen : Name - ",m_pConfName);
	/*In case of video refresh request in VSW when running in Call Generator mode we will send
	 *the fast update to the video encoder like in CP case and not to the video source.
	 * The media manger in this case will start the movie and send intra
	*/
	PartyRsrcID partyRsrcID = INVALID;
	*pParam >> partyRsrcID;


	CVideoBridgePartyCntl*  pPartyCntl = ( CVideoBridgePartyCntl*)GetPartyCntl(partyRsrcID);
	if ( !pPartyCntl )
	{
		PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnConfVideoRefreshForCallGen :the party is not connected to brdg, Name - ",m_pConfName);
		return;
	}

	pPartyCntl->FastUpdate();
}

//--------------------------------------------------------------------------
void CVideoBridgeVSW::OnEndPartyConnectCONNECTED(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnEndPartyConnect(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgeVSW::OnEndPartyConnectINSWITCH(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnEndPartyConnect(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgeVSW::OnEndPartyConnectIVRModeINSWITCH(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	CBridge::EndPartyConnect(pParam, PARTY_VIDEO_IVR_MODE_CONNECTED);
}

//--------------------------------------------------------------------------
void CVideoBridgeVSW::OnEndPartyConnect(CSegment* pParam)
{
	PartyRsrcID PartyId;
	WORD status;
	EMediaDirection eMediaDirection = eNoDirection;
	CSegment* pTempParam = new CSegment(*pParam);
	*pTempParam >> PartyId >> status << (WORD&)eMediaDirection;
	POBJDELETE(pTempParam)

	// this CBridge function is called twice when video connected with IVR
	// the CBridge take care of failure
	CBridge::EndPartyConnect(pParam, PARTY_VIDEO_CONNECTED);

	// return in case of failure
	TRACECOND_AND_RETURN(status != statOK, "PartyId:" << PartyId << ", status:" << status << " - Failed, invalid status");

	const CParty* pParty = GetLookupTableParty()->Get(PartyId);

	// we add the party to image list to enable build layout
	// sync will receive when party will become speaker
	AddPartyToConfMixAfterVideoInSynced(pParty);

	// start of change layout
	CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(PartyId);
	PASSERT_AND_RETURN(!pPartyCntl);

	ChangeLayout(pPartyCntl);

	// if the party that connects is the last video speaker
	// we will send video speaker again
	if (pPartyCntl->GetPartyTaskApp() == m_pLastActiveVideoSpeakerRequest)
		ResendLastActiveVideoSpeakerRequest();

	// Send rate limitation to the party
	if (eMediaDirection == eMediaIn || eMediaDirection == eMediaInAndOut)
	{
		if (m_flowControlRate)
			pPartyCntl->ForwardFlowControlCommand(m_flowControlRate);
	}
}

// ------------------------------------------------------------------------------------------------------------------------
void  CVideoBridgeVSW::OnConfDisConnectPartyCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnConfDisConnectPartyCONNECTED : ConfName - ",m_pConfName);
	CVideoBridge::OnConfDisConnectPartyCONNECTED(pParam);
	ChangeLayout();
}
// ------------------------------------------------------------------------------------------------------------------------
void  CVideoBridgeVSW::OnConfDisConnectPartyINSWITCH(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnConfDisConnectPartyINSWITCH : ConfName - ",m_pConfName);
	CVideoBridge::OnConfDisConnectPartyCONNECTED(pParam);
	ChangeLayout();
}
// ------------------------------------------------------------------------------------------------------------------------
void CVideoBridgeVSW::OnConfSetLectureModeINSWITCH(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnConfSetLectureModeINSWITCH : ConfName - ",m_pConfName);
	CVideoBridge::OnConfSetLectureModeCONNECTED(pParam);
}
//-------------------------------------------------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//~~~~~~~~~~~~~~~~~~~~~~~~~~ State machine action functions all events for inswitch~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------------------------------
void CVideoBridgeVSW::OnConfConnectPartyINSWITCH(CSegment* pParam)
{
	 PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnConfConnectPartyINSWITCH", m_pConfName);
	 CVideoBridge::OnConfConnectPartyCONNECTED(pParam);
}
// ------------------------------------------------------------------------------------------------------------------------
void CVideoBridgeVSW::OnConfDeletePartyFromConfINSWITCH(CSegment * pParam)
{
	 PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnConfDeletePartyFromConfINSWITCH", m_pConfName);
	 CVideoBridge::OnConfDeletePartyFromConfCONNECTED(pParam);
}
// ------------------------------------------------------------------------------------------------------------------------
void  CVideoBridgeVSW::OnEndPartyDisConnectCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnEndPartyDisConnectCONNECTED : ConfName - ", m_pConfName);
	OnEndPartyDisConnect(pParam);
}
// ------------------------------------------------------------------------------------------------------------------------
void  CVideoBridgeVSW::OnEndPartyDisConnectINSWITCH(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnEndPartyDisConnectINSWITCH : ConfName - ", m_pConfName);
	OnEndPartyDisConnect(pParam);
}
// ------------------------------------------------------------------------------------------------------------------------
void  CVideoBridgeVSW::OnEndPartyDisConnectDISCONNECTING(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnEndPartyDisConnectDISCONNECTING : ConfName - ", m_pConfName);
	CVideoBridge::OnEndPartyDisConnect(pParam);
}
// ------------------------------------------------------------------------------------------------------------------------
void  CVideoBridgeVSW::OnEndPartyDisConnect(CSegment* pParam)
{
	HandleFlowControlRatePartyDisconnected(pParam);
	CVideoBridge::OnEndPartyDisConnect(pParam);
	ChangeLayout();
}
// ------------------------------------------------------------------------------------------------------------------------
void CVideoBridgeVSW::OnConfUpdateVideoMuteCONNECTED(CSegment * pParam)
{
	PTRACE2(eLevelInfoNormal,"CVideoBridgeCP::OnConfUpdateVideoMuteCONNECTED : Name - ",m_pConfName);
	OnConfUpdateVideoMute(pParam);
}
// ------------------------------------------------------------------------------------------------------------------------
void CVideoBridgeVSW::OnConfUpdateVideoMuteINSWITCH(CSegment * pParam)
{
	PTRACE2(eLevelInfoNormal,"CVideoBridgeCP::OnConfUpdateVideoMuteINSWITCH : Name - ",m_pConfName);
	OnConfUpdateVideoMute(pParam);
}
// ------------------------------------------------------------------------------------------------------------------------
void CVideoBridgeVSW::OnConfUpdateVideoMute(CSegment * pParam)
{
	WORD  srcReq = 0;
	EOnOff eOnOff = eOff;
	EMediaDirection eMediaDirection = eNoDirection;
	CVideoBridgePartyCntl* pPartyCntl = NULL;

	*pParam >> srcReq;

	if(srcReq==OPERATOR)
	{
		char name[H243_NAME_LEN];
		*pParam >> name >> (BYTE&)eOnOff >> (BYTE&)eMediaDirection;
		name[H243_NAME_LEN-1]='\0';
		pPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(name);
	}
	else
	{
		PartyRsrcID partyRsrcID = INVALID;
		*pParam >> partyRsrcID >> (BYTE&)eOnOff >> (BYTE&)eMediaDirection;
		pPartyCntl = ( CVideoBridgePartyCntl*)GetPartyCntl(partyRsrcID);
	}

	if(eMediaDirection != eMediaIn)
	{
		PTRACE2(eLevelInfoNormal,"CVideoBridgeCP::OnConfUpdateVideoMute Mute Video Out Not supported: Name - ",m_pConfName);
		DBGPASSERT_AND_RETURN(101);
	}

	if(NIL(CVideoBridgePartyCntl) == pPartyCntl)
	{
		PTRACE2(eLevelInfoNormal,"CVideoBridgeCP::OnConfUpdateVideoMute Party not connected to bridge: Name - ",m_pConfName);
		return;
	}

	RequestPriority reqPrio=AUTO_Prior;
	reqPrio = GetRequestPriority(srcReq);

	const CImage* pImage=pPartyCntl->GetPartyImage();
	DBGPASSERT_AND_RETURN(pImage==NIL(CImage));

	BYTE previouselyMuted=pImage->isMuted(); //before updating we get previouse
	pPartyCntl->UpdateSelfMute(reqPrio, eOnOff);
	BYTE currentlyMuted=pImage->isMuted(); //after updating we get current

	if(previouselyMuted==currentlyMuted)
		return;

	if(currentlyMuted)
		ApplicationActionsOnRemovePartyFromMix(pPartyCntl);
	else
		ApplicationActionsOnAddPartyToMix(pPartyCntl);

	ChangeLayout();
}

// ------------------------------------------------------------------------------------------------------------------------
void CVideoBridgeVSW::OnConfUpdateFlowControlRateCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnConfUpdateFlowControlRateCONNECTED : Name - ",m_pConfName);
	OnConfUpdateFlowControlRate(pParam);
}

// ------------------------------------------------------------------------------------------------------------------------
void CVideoBridgeVSW::OnConfUpdateFlowControlRateINSWITCH(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnConfUpdateFlowControlRateINSWITCH : Name - ",m_pConfName);
	OnConfUpdateFlowControlRate(pParam);
}

// ------------------------------------------------------------------------------------------------------------------------
void CVideoBridgeVSW::OnConfUpdateFlowControlRate(CSegment* pParam)
{
	PartyRsrcID partyId;
	DWORD newBitRate;
	WORD channelDirection, roleLabel;
	CLPRParams* pLprParams = NULL;
	BYTE bIsCascade;
	BOOL isLpr = 0;

	*pParam >> partyId >> newBitRate >> channelDirection >> roleLabel >> bIsCascade >> isLpr;
	if (isLpr == TRUE)
		*pParam >> (DWORD&)pLprParams;

	TRACEINTO << "PartyId:" << partyId << ", NewBitRate:" << newBitRate << ", ChannelDirection:" << channelDirection << ", IsCascade:" << (int)bIsCascade << ", IsLpr:" << (int)isLpr;

	if (!newBitRate)
		return;

	// 1. Extract the LPR data (if there is one)
	// 2. In case there is LPR data - Save/Pass the party control that sent the request (The TaskApp) to the SendFlowControlRateToAllParties
	// 3. In the spreading loop (Spreading the flow control) when you reach the party that sent the request and the LPR data is valid (!=NULL)
	// Send a new API with the LPR data back towards the PartyControl (I've already prepared the API)
	CVideoBridgePartyCntlVSW* pInitiatorPartyCntl = (CVideoBridgePartyCntlVSW*)GetPartyCntl(partyId);
	if (pInitiatorPartyCntl)
	{
		pInitiatorPartyCntl->SetPartyFlowControlRate(newBitRate);
		pInitiatorPartyCntl->SetIsCascadeParty(bIsCascade);
		if (FindLowestFlowControlRate())
		{
			if (pLprParams != NULL)
				SendFlowControlRateToAllParties(partyId, pLprParams);
			else
				SendFlowControlRateToAllParties();
		}
		else    //need to return ack to lpr request sender
		{
			if (pLprParams)
				pInitiatorPartyCntl->ForwardFlowControlCommand(m_flowControlRate, pLprParams);
		}
	}
}

//--------------------------------------------------------------------------
void CVideoBridgeVSW::HandleFlowControlRatePartyDisconnected(CSegment* pParam)
{
	TRACECOND_AND_RETURN(!m_flowControlRate, "ConfName:" << m_pConfName << " - Flow control limitation does not exists");

	PartyRsrcID PartyId;
	WORD status;
	EMediaDirection eMediaDirection = eNoDirection;
	CSegment* pTempParam = new CSegment(*pParam);
	*pTempParam >> PartyId >> status << (WORD&)eMediaDirection;
	POBJDELETE(pTempParam);

	TRACEINTO << "PartyId:" << PartyId << ", MediaDirection:" << eMediaDirection << ", status:" << status;

	if ((eMediaDirection != eMediaOut) && (eMediaDirection != eMediaInAndOut))
	{
		CVideoBridgePartyCntlVSW* pPartyCntl = (CVideoBridgePartyCntlVSW*)GetPartyCntl(PartyId);
		PASSERT_AND_RETURN(!pPartyCntl);

		DWORD disconnectedPartyFcRate = pPartyCntl->GetPartyFlowControlRate();
		pPartyCntl->SetPartyFlowControlRate(0);
		if (disconnectedPartyFcRate == m_flowControlRate)
		{
			if (FindLowestFlowControlRate())
				SendFlowControlRateToAllParties();
		}
	}
}

// ------------------------------------------------------------------------------------------------------------------------
BYTE CVideoBridgeVSW::FindLowestFlowControlRate()
{
	DWORD newFlowControlRate = 0, curPartyRate;

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntlVSW* pPartyCntl = (CVideoBridgePartyCntlVSW*)_itr->second;
		if (pPartyCntl)
		{
			curPartyRate = pPartyCntl->GetPartyFlowControlRate();
			if ((curPartyRate) && ((curPartyRate < newFlowControlRate) || (!newFlowControlRate)))
				newFlowControlRate = curPartyRate;
		}
	}

	if (m_flowControlRate != newFlowControlRate)
	{
		m_flowControlRate = newFlowControlRate;
		TRACEINTO << "ConfName:" << m_pConfName << ", FlowControlRate:" << m_flowControlRate << " - Rate updated";
		return TRUE;
	}

	TRACEINTO << "ConfName:" << m_pConfName << ", FlowControlRate:" << m_flowControlRate << " - Rate not updated";
	return FALSE;
}

// ------------------------------------------------------------------------------------------------------------------------
void CVideoBridgeVSW::SendFlowControlRateToAllParties(PartyRsrcID partyId, CLPRParams* LprParams)
{
	BYTE bIsCascadeLimitation = FALSE; // No rate limitation

	if (m_flowControlRate) // There is rate limitation. Check whether the  rate limitation is only from cascade link.
	{
		bIsCascadeLimitation = TRUE;

		CBridgePartyList::iterator _end = m_pPartyList->end();
		for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
		{
			// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
			CVideoBridgePartyCntlVSW* pPartyCntl = (CVideoBridgePartyCntlVSW*)_itr->second;
			if (pPartyCntl)
			{
				if ((pPartyCntl->GetPartyFlowControlRate() == m_flowControlRate) && (pPartyCntl->GetIsCascadeParty() == FALSE))
				{
					bIsCascadeLimitation = FALSE;
					break;
				}
			}
		}
	}

	// Send rate limitation to all parties:
	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntlVSW* pPartyCntl = (CVideoBridgePartyCntlVSW*)_itr->second;
		if (pPartyCntl)
		{
			 // If limitation is only from cascade party (parties) then don't send them back the limitation because it will cause that the limitation will remain as long as the link exists.
			if (bIsCascadeLimitation && pPartyCntl->GetIsCascadeParty() && (pPartyCntl->GetPartyFlowControlRate() == m_flowControlRate))
				continue;

			//If it's LPR req that caused the Flow control we need to forward the initiator party the LPR params
			if (partyId != INVALID && pPartyCntl->GetPartyRsrcID() == partyId)
				pPartyCntl->ForwardFlowControlCommand(m_flowControlRate, LprParams);
			else
				pPartyCntl->ForwardFlowControlCommand(m_flowControlRate);
		}
	}
}

// ------------------------------------------------------------------------------------------------------------------------
void CVideoBridgeVSW::OnSetSiteNameINSWITCH(CSegment* pParams)
{
	// site names updated in vsw at image, but timer for stop display is blocked
	OnSetSiteNameCONNECTED(pParams);
}
// ------------------------------------------------------------------------------------------------------------------------
void  CVideoBridgeVSW::OnConfDisConnectConfINSWITCH(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnConfDisConnectConfINSWITCH Changing State to DISCONNECTING: Name - ",m_pConfName);

	// Change to disconnecting state - to prevent connecting new parties
	m_state = DISCONNECTING;
}
// ------------------------------------------------------------------------------------------------------------------------
void  CVideoBridgeVSW::OnConfTerminateINSWITCH(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnConfTerminateINSWITCH : Name - ",m_pConfName);
	PASSERTMSG(1, "ILLEGAL STATE TO RECEIVE TERMINATE EVENT - the video bridge should be in disconnecting state.");
}

//-------------------------------------------------------------------------------------------------------------------------
void  CVideoBridgeVSW::AudioSpeakerChanged()
{
	PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::AudioSpeakerChanged : Name - ",m_pConfName);

	if (NULL == m_pLastActiveAudioSpeakerRequest) //Last speaker in conf in ivr
		m_pConfApi->UpdateDB( m_pLastActiveAudioSpeakerRequest, NOAUDIOSRC, 0);
	else
		m_pConfApi->UpdateDB( m_pLastActiveAudioSpeakerRequest, AUDIOSRC, 0);

	ApplicationActionsOnAudioSpeakerChange();
}
/*
// ------------------------------------------------------------------------------------------------------------------------
void  CVideoBridgeVSW::OnConfAudioSpeakerChangedINSWITCH(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnConfAudioSpeakerChangedINSWITCH : Name - ",m_pConfName);
	OnConfAudioSpeakerChangedCONNECTED(pParam);

}
*/
// ------------------------------------------------------------------------------------------------------------------------
void  CVideoBridgeVSW::StartLectureMode()
{
	if(NULL == m_pLectureModeParams)
	{
		PTRACE2(eLevelError,"CVideoBridgeVSW::StartLectureMode : invalid lecture mode params, Name - ",m_pConfName);
		DBGPASSERT_AND_RETURN(1);
	}
	// this function begins lecture mode
	if(m_pLectureModeParams->GetLecturerName() == NIL(const char))
	{
		PTRACE2(eLevelError,"CVideoBridgeVSW::StartLectureMode : invalid lecturer`s name, Name - ",m_pConfName);
		DBGPASSERT_AND_RETURN(1);
	}

	PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::StartLectureMode : LecturerName - ", m_pLectureModeParams->GetLecturerName());

    DBGPASSERT_AND_RETURN(!m_pLectureModeParams->IsLectureModeOn());// the lecture mode signal must be ON

	if(m_pLectureModeParams->GetLectureModeType() == eLectureModeRegular && m_pLectureModeParams->GetIsTimerOn())
		StartTimer(LECTURE_MODE_TOUT, m_pLectureModeParams->GetTimerInterval()*SECOND);

	// set lecturer in conf level force
//	(*m_pReservationLayout)[0]->SetPartyForceName(m_pLectureModeParams->GetLecturerName());
//	(*m_pReservationLayout)[0]->SetForceAttributes(OPERATOR_Prior,FORCE_CONF_Activ);

	ChangeLayout();

	UpdateConfDBLectureMode();
}
// ------------------------------------------------------------------------------------------------------------------------
void  CVideoBridgeVSW::EndLectureMode(BYTE removeLecturer)
{
	PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::EndLectureMode, Name - ",m_pConfName);
    // if removeLecturer == 1 this mean that the conference goes to mode regular and LM is over
	// if removeLecturer == 0 this mean that the conference still in LM but temporarly goes to mode transcoding
	if(NULL == m_pLectureModeParams)
	{
		PTRACE2(eLevelError,"CVideoBridgeVSW::EndLectureMode : invalid lecture mode params, Name - ",m_pConfName);
		DBGPASSERT_AND_RETURN(1);
	}

	DeleteTimer(LECTURE_MODE_TOUT);

	if(removeLecturer == YES)
		m_pLectureModeParams->SetLecturerName("");

//	m_pReservationLayout->RemovePartyForce(m_pLectureModeParams->GetLecturerName(),OPERATOR_Prior);
	ChangeLayout();

	UpdateConfDBLectureMode();
}
// ------------------------------------------------------------------------------------------------------------------------
void CVideoBridgeVSW::OnTimerLectureModeCONNECTED(CSegment * pParam)
{
	PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnTimerLectureModeCONNECTED , LecturerName - ", m_pLectureModeParams->GetLecturerName());
	OnTimerLectureMode(pParam);
}
// ------------------------------------------------------------------------------------------------------------------------
void CVideoBridgeVSW::OnTimerLectureModeINSWITCH(CSegment * pParam)
{
	PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::OnTimerLectureModeINSWITCH , LecturerName - ", m_pLectureModeParams->GetLecturerName());
	OnTimerLectureMode(pParam);
}
//--------------------------------------------------------------------------
void CVideoBridgeVSW::OnTimerLectureMode(CSegment* pParam)
{
	CVideoBridgePartyCntl* pLecturer = (CVideoBridgePartyCntl*)GetPartyCntl(m_pLectureModeParams->GetLecturerName());
	DWORD partyImageSpeakerId = GetPartyImageSpeakerId();

	if (pLecturer && partyImageSpeakerId && m_pLectureModeParams->GetLectureModeType() == eLectureModeRegular)
	{
		if (pLecturer->GetPartyRsrcID() == partyImageSpeakerId) // if the lecturer is the current speaker, change his Image
		{
			DWORD partyImageChangedId = GetPartyImageIdByPosition(GetPartyImageVectorSize()-1);
			PASSERT_AND_RETURN(!partyImageChangedId);

			// Cause to set partyImageSpeakerId to the second place in the vector
			SetPartyImageSpeakerId(partyImageChangedId);
			SetPartyImageSpeakerId(partyImageSpeakerId);

			ChangeLayout();
		}

		if (m_pLectureModeParams->GetIsTimerOn())
			StartTimer(LECTURE_MODE_TOUT, m_pLectureModeParams->GetTimerInterval()*SECOND);
	}
}

//--------------------------------------------------------------------------
CLayout* CVideoBridgeVSW::GetReservationLayout(void)const
{
	 return m_pReservationLayout;
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeVSW::AddPartyToConfMixAfterVideoInSynced(const CTaskApp *pParty)
{
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pParty));

	PartyRsrcID partyId = ((CParty*)pParty)->GetPartyRsrcID();

	CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pPartyCntl));

	const CImage* pImage = pPartyCntl->GetPartyImage();
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pImage));

	AddPartyImage(partyId);

	if (!pImage->isMuted())
	{
		ApplicationActionsOnAddPartyToMix(pPartyCntl);
	}
}

//--------------------------------------------------------------------------
void CVideoBridgeVSW::RemovePartyFromConfMixBeforeDisconnecting(const CTaskApp* pParty)
{
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pParty));

	PartyRsrcID partyId = ((CParty*)pParty)->GetPartyRsrcID();

	CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
	PASSERT_AND_RETURN(!pPartyCntl);

	// AUTO_Prior - remove all forces
	m_pReservationLayout->RemovePartyForce(pPartyCntl->GetName(), AUTO_Prior);

	// remove image from vsw layout data members
	RemoveImageFromVswLayoutData(partyId);

	// remove from image list
	BYTE result = DelPartyImage(partyId);
	if (result == YES)
	{
		ApplicationActionsOnRemovePartyFromMix(pPartyCntl);

		// remove pointers to image from Reservation bridge party video out layout
		CBridgePartyList::iterator _end = m_pPartyList->end();
		for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
		{
			// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
			CVideoBridgePartyCntlVSW* pPartyCntl = (CVideoBridgePartyCntlVSW*)_itr->second;
			if (pPartyCntl)
				pPartyCntl->ResetRsrvImage0(partyId);
		}
	}
}

// ------------------------------------------------------------
void CVideoBridgeVSW::RemoveImageFromVswLayoutData(DWORD partyRscId)
{
	CVidSubImage* pSubImage = NULL;

	for (int i = 0; i < MAX_SUB_IMAGES_IN_LAYOUT; ++i)
	{
		pSubImage = m_pConfLayout->GetSubImageNum(i);
		if (CPObject::IsValidPObjectPtr(pSubImage) && pSubImage->GetImageId() == partyRscId)
			pSubImage->SetImageId(0);

		pSubImage = m_pConfSourceLayout->GetSubImageNum(i);
		if (CPObject::IsValidPObjectPtr(pSubImage) && pSubImage->GetImageId() == partyRscId)
			pSubImage->SetImageId(0);
	}
}

// ------------------------------------------------------------
void CVideoBridgeVSW::RemovePartyFromAnyConfSettingsWhenDeletedFromConf(const char* pDeletedPartyName)
{
	m_pReservationLayout->RemovePartyForce(pDeletedPartyName);

	UpdateDB_ConfLayout();

	ApplicationActionsOnDeletePartyFromConf(pDeletedPartyName);
}

// ------------------------------------------------------------------------------------------------------------------------
void CVideoBridgeVSW::UpdateDB_ConfLayout()
{
	CSegment vidLayoutSeg;
	if(!m_pReservationLayout->Serialize(CONF_lev,&vidLayoutSeg))
		m_pConfApi->UpdateDB((CTaskApp*) 0xffff,CPCONFLAYOUT,(DWORD) 0,0,&vidLayoutSeg);
	else
		DBGPASSERT(1);
}
// ------------------------------------------------------------
void CVideoBridgeVSW::GetForcesFromReservation(CCommConf* pCommConf)
{
	GetForcesFromReservation(pCommConf,*m_pReservationLayout);
}
// ------------------------------------------------------------
void CVideoBridgeVSW::GetForcesFromReservation(CCommConf* pCommConf,CLayout& vswlayout)
{
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr((CPObject*)pCommConf));

	// num of layouts validation test
	WORD  m_wNumVideoLayouts=pCommConf->GetNumRsrvVidLayout();
	if( m_wNumVideoLayouts > (WORD)CP_NO_LAYOUT ) {
		DBGPASSERT_AND_RETURN(m_wNumVideoLayouts);
	}

	//get layouts and write it
	if(m_wNumVideoLayouts == 0){
		PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::GetForcesFromReservation num of layouts = 0 , ConfName - ", m_pConfName);
		return;
	}

	CVideoLayout* pCurrentLayout = pCommConf->GetFirstRsrvVidLayout();
	DBGPASSERT_AND_RETURN(pCurrentLayout == NIL(CVideoLayout));

	LayoutType newLayoutType = GetNewLayoutType(pCurrentLayout->GetScreenLayout());
	if( newLayoutType != CP_LAYOUT_1X1 ){
		PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::GetForcesFromReservation reservation layout type is not 1x1 , ConfName - ", m_pConfName);
		DBGPASSERT_AND_RETURN(newLayoutType);
	}

	if( vswlayout.GetLayoutType() != CP_LAYOUT_1X1 ){
		PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::GetForcesFromReservation target layout type is not 1x1 , ConfName - ", m_pConfName);
		DBGPASSERT_AND_RETURN(vswlayout.GetLayoutType());
	}

	// sometimes first layout in reservation is not active one
	vswlayout.SetLayoutFromRes(*pCurrentLayout,CONF_lev);
}
// ------------------------------------------------------------
CLayout* CVideoBridgeVSW::FindBestLayoutForParty(CLayout* confLayout,CVideoBridgePartyCntl* pParty,CVideoBridgePartyCntl* confSource,CLayout* confSourceLayout,BYTE& isPartyLayoutChanged)
{
	CLayout* pPartyNewLayout = NULL;
	isPartyLayoutChanged = FALSE;
	CLayout* pPartyResrvationLayout = NULL;
	if(!CPObject::IsValidPObjectPtr(pParty)){
	   DBGPASSERT(1);
	   return confLayout;
	}

	BOOL rIsInPortOpened, rIsOutPortOpened;

	pParty->ArePortsOpened (rIsInPortOpened,rIsOutPortOpened);
	if(!rIsOutPortOpened)
		return confLayout;

	//If party in IVR state
	if(pParty->IsConnectedStandalone())
	{
		PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::FindBestLayoutForParty, party in IVR mode ,PartyName - ", pParty->GetFullName());
		return confLayout;
	}

	CLayout* pPartyCurrentLayout = pParty->GetCurrentLayout();
	if(pParty->IsUniDirectionConnection(eMediaIn))
		return confLayout;
	 pPartyResrvationLayout = pParty->GetReservationLayout();

	if(pParty!=confSource)
	{
		CVidSubImage* pSubImage = m_pReservationLayout->GetSubImageNum(0);
		if(pSubImage){
		// if force exist
			  if(pSubImage->isForcedInConfLevel())
			  {
		         PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::FindBestLayoutForParty, Conference  layout (force), PartyName - ", pParty->GetFullName());
		         pPartyNewLayout = confLayout;
		         if(*pPartyNewLayout!=*pPartyCurrentLayout)
		        	 isPartyLayoutChanged = TRUE;
		         return pPartyNewLayout;
			  }
	   }
	}
	// Romem - klocwork
	if((*pPartyResrvationLayout)[0] == NULL || (*pPartyCurrentLayout)[0] == NULL)
	{
		PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::FindBestLayoutForParty, 1st cell in Reservation/Current party layout is NULL , take conference layout, PartyName - ", pParty->GetFullName());
		pPartyNewLayout = confLayout;
		if(*pPartyNewLayout!=*pPartyCurrentLayout)
			isPartyLayoutChanged = TRUE;
		return pPartyNewLayout;
	}
	if((*pPartyResrvationLayout)[0]->isForcedInPartLevel())
	{
		const char* ForcedPartyName = (*pPartyResrvationLayout)[0]->GetPartyForce();
		CVideoBridgePartyCntl *pForcedParty=(CVideoBridgePartyCntl*)GetPartyCntl((*pPartyResrvationLayout)[0]->GetPartyForce());

		// VNGR-7604 force layout to disconnected party
		if (!CPObject::IsValidPObjectPtr(pForcedParty))
			return confLayout;

		const CImage* pForceImage = pForcedParty->GetPartyImage();
		// VNGR-15992 pForceImage is null!, add protection!
		if (!CPObject::IsValidPObjectPtr(pForceImage))
		{
			PTRACE(eLevelInfoNormal,"CVideoBridgeVSW::FindBestLayoutForParty, Warning! party is forced but forced image is not valid!!! (return conf layout)");
			return confLayout;
		}

		(*pPartyResrvationLayout)[0]->SetImageId(pForcedParty->GetPartyRsrcID());
		CSmallString sstr;
		sstr << "Party Name: " << pParty->GetFullName() << ", Forced Party Name = " << pForcedParty->GetFullName();
		if(pForceImage->isMuted())
		{
			if(pParty==confSource)
			{
			   PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::FindBestLayoutForParty, Party is Source Conf.  Party image is video muted, Use SourceConf layout- ", sstr.GetString());
			   pPartyNewLayout = confSourceLayout;
			   if(*pPartyNewLayout!=*pPartyCurrentLayout)
			      isPartyLayoutChanged = TRUE;
			   return pPartyNewLayout;
			}
			else
			{
				 PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::FindBestLayoutForParty,Party image is video muted, Use SourceConf layout- ", sstr.GetString());

				 pPartyNewLayout = confLayout;
				 if(*pPartyNewLayout!=*pPartyCurrentLayout)
				    isPartyLayoutChanged = TRUE;
				 return pPartyNewLayout;
			}
		}
		TRACESTR(eLevelInfoNormal)  << "CVideoBridgeVSW::FindBestLayoutForParty, party layout , PartyName - " <<  pParty->GetFullName() <<
		                        " Forced Party - " << ForcedPartyName;
		pPartyNewLayout=pPartyResrvationLayout;
		if(*pPartyNewLayout!=*pPartyCurrentLayout)
		{

			 isPartyLayoutChanged = TRUE;
			 TRACESTR(eLevelInfoNormal)  << "CVideoBridgeVSW::FindBestLayoutForParty, party layout - layout is changed , PartyName - " <<  pParty->GetFullName() <<
		                        " Forced Party - " << ForcedPartyName;
		}
		else // 2 layouts are equal
		{
			if(((*pPartyResrvationLayout)[0]->isForcedInPartLevel()) && !((*pPartyCurrentLayout)[0]->isForcedInPartLevel()))
				  isPartyLayoutChanged = TRUE;
		}
		return pPartyNewLayout;
	}
	else
	{
	   if(pParty==confSource)
	   {
		   PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::FindBestLayoutForParty, party is conf source, PartyName - ", pParty->GetFullName());
		   pPartyNewLayout=confSourceLayout;
		   	if(*pPartyNewLayout!=*pPartyCurrentLayout)
		   		isPartyLayoutChanged = TRUE;
		   	else
		   	{
		   		if(!((*pPartyResrvationLayout)[0]->isForcedInPartLevel()) && ((*pPartyCurrentLayout)[0]->isForcedInPartLevel()))
		   			isPartyLayoutChanged = TRUE;
		   	}
		   return pPartyNewLayout;
	   }
	   else
	   {
		   PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::FindBestLayoutForParty, regular conference layout , PartyName - ", pParty->GetFullName());
		   pPartyNewLayout=confLayout;
		   if(*pPartyNewLayout!=*pPartyCurrentLayout)
		   	  isPartyLayoutChanged = TRUE;
		   else
			  if(!((*pPartyResrvationLayout)[0]->isForcedInPartLevel()) && ((*pPartyCurrentLayout)[0]->isForcedInPartLevel()))
			   		isPartyLayoutChanged = TRUE;
		   return pPartyNewLayout;
	   }
	}
	pPartyNewLayout=confLayout;
	if(*pPartyNewLayout!=*pPartyCurrentLayout)
	   isPartyLayoutChanged = TRUE;
	return pPartyNewLayout;

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Change layout main internal function
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------
WORD CVideoBridgeVSW::BuildLayout(CLayout& confLayout, CVideoBridgePartyCntl*& confSource, CLayout& confSourceLayout)
{
	WORD retVal = TRUE;

	// get force / lecture mode
	DWORD forcedPartyId = 0;
	DWORD lowPriorityForcedPartyId = 0;
	GetForcedParty(forcedPartyId, lowPriorityForcedPartyId);

	// get speakers
	DWORD currSpeakerId = 0;
	DWORD prevSpeakerId = 0;
	GetUnmutedSpeaker(currSpeakerId, prevSpeakerId);

	TRACEINTO << "CVideoBridgeVSW::BuildLayout "
	          << "- ConfName:"                 << m_pConfName
	          << ", currSpeakerId:"            << currSpeakerId
	          << ", prevSpeakerId:"            << prevSpeakerId
	          << ", forcedPartyId:"            << forcedPartyId
	          << ", lowPriorityForcedPartyId:" << lowPriorityForcedPartyId;

	// build layout
	if (forcedPartyId != 0)
	{
		confLayout.SetVswLayout(forcedPartyId, m_pConfName);
		confSource = GetVideoSourceCntl(confLayout);
		if (lowPriorityForcedPartyId != 0 && lowPriorityForcedPartyId != forcedPartyId)
		{
			confSourceLayout.SetVswLayout(lowPriorityForcedPartyId, m_pConfName);
		}
		else if (currSpeakerId != 0 && currSpeakerId != forcedPartyId)
		{
			confSourceLayout.SetVswLayout(currSpeakerId, m_pConfName);
		}
		else if (prevSpeakerId != 0 && prevSpeakerId != forcedPartyId)
		{
			confSourceLayout.SetVswLayout(prevSpeakerId, m_pConfName);
		}
		else
		{
			confSourceLayout.SetVswLayout(forcedPartyId, m_pConfName);
			PTRACE2(eLevelInfoNormal, "CVideoBridgeVSW::BuildLayout - no un-muted speaker, ConfName:", m_pConfName);
		}
	}
	else
	{
		if (currSpeakerId != 0)
		{
			confLayout.SetVswLayout(currSpeakerId, m_pConfName);
			confSource = GetVideoSourceCntl(confLayout);
			if (prevSpeakerId != 0)
			{
				confSourceLayout.SetVswLayout(prevSpeakerId, m_pConfName);
			}
			else
			{
				confSourceLayout.SetVswLayout(currSpeakerId, m_pConfName);
				PTRACE2(eLevelInfoNormal, "CVideoBridgeVSW::BuildLayout - no un-muted previous speaker, ConfName:", m_pConfName);
			}
		}
		else
		{
			PTRACE2(eLevelInfoNormal, "CVideoBridgeVSW::BuildLayout failed - no un-muted valid images, ConfName:", m_pConfName);
			retVal = FALSE;
		}
	}

	return retVal;
}

// ---------------------------------------------------------------------------------------------------------------------------------
void CVideoBridgeVSW::UpdateLayoutData(CLayout& confLayout,CVideoBridgePartyCntl* confSource,CLayout& confSourceLayout,CVideoBridgePartyCntl* pAddParty)
{
	// mark new video sources for intra
	if(confSource!=NULL){// if conf source is NULL (last party disconnection) - no mark for intra
		if(IsConfSourceLayoutChanged(confSource,confSourceLayout)){
			CVideoBridgePartyCntl* pConfSourceVideoSourceCntl = GetVideoSourceCntl(confSourceLayout);
			if(pConfSourceVideoSourceCntl!=NULL){
				pConfSourceVideoSourceCntl->MarkAsNewVideoSource(1);
			}
		}
		if(IsConfLayoutChanged(confLayout) || pAddParty){
			confSource->MarkAsNewVideoSource(1);
		}
	}
	// update data members
	*m_pConfLayout = confLayout;
	m_pConfSourceBridgeParty = confSource;
	*m_pConfSourceLayout = confSourceLayout;
}
// ---------------------------------------------------------------------------------------------------------------------------------
void CVideoBridgeVSW::AskIntraFromNewVideoSource()
{
	CVideoBridgePartyCntl* pConfSourceVideoSourceCntl = GetVideoSourceCntl(*m_pConfSourceLayout);
	if (CPObject::IsValidPObjectPtr(pConfSourceVideoSourceCntl))
	{
		if (pConfSourceVideoSourceCntl->IsMarkedAsNewVideoSource())
		{
			pConfSourceVideoSourceCntl->ResyncVideoSource();
			PTRACE2(eLevelInfoNormal, "CVideoBridgeVSW::AskIntraFromNewVideoSource, seen party source name - ", pConfSourceVideoSourceCntl->GetName());
		}
	}
	if (CPObject::IsValidPObjectPtr(m_pConfSourceBridgeParty))
	{
		if (m_pConfSourceBridgeParty->IsMarkedAsNewVideoSource())
		{
			m_pConfSourceBridgeParty->ResyncVideoSource();
			PTRACE2(eLevelInfoNormal, "CVideoBridgeVSW::AskIntraFromNewVideoSource, conf source name - ", m_pConfSourceBridgeParty->GetName());
		}
	}

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
		{
			if ((pPartyCntl != pConfSourceVideoSourceCntl) && (pPartyCntl != m_pConfSourceBridgeParty))
			{
				if (pPartyCntl->IsMarkedAsNewVideoSource())
					pPartyCntl->ResyncVideoSource();
			}
		}
	}
}

//--------------------------------------------------------------------------
void CVideoBridgeVSW::SendChangeLayout(CLayout* confLayout, CVideoBridgePartyCntl* confSource, CLayout* confSourceLayout, BYTE is_conf_layout_changed, BYTE is_conf_source_layout_changed, BYTE is_one_of_parties_layout_changed, CVideoBridgePartyCntl* pAddParty)
{
	CLayout* pPartyLayout = NULL;
	BYTE     isPartyLayoutChanged;

	// send change layout to conf source
	if (is_conf_source_layout_changed || is_one_of_parties_layout_changed)
	{
		TRACEINTO << "ConfName:" << m_pConfName;
		// send party layout
		if (CPObject::IsValidPObjectPtr(confSource))
		{
			isPartyLayoutChanged = FALSE;
			pPartyLayout =  FindBestLayoutForParty(confLayout, confSource, confSource, confSourceLayout, isPartyLayoutChanged);
			if ((*pPartyLayout)[0] == NULL)
			{
				TRACEINTO << "PartyId:" << confSource->GetPartyRsrcID() << " - Party Layout of Conf Source is empty, avoid change layout";
			}
			else
			{
				if (isPartyLayoutChanged && confSource != pAddParty)
				{
					if (pPartyLayout != confSourceLayout)
					{
						TRACEINTO << "PartyId:" << confSource->GetPartyRsrcID() << " - Send change layout to conf source with party layout";
						PartyRsrcID partyRscId = (*pPartyLayout)[0] ? (*pPartyLayout)[0]->GetImageId() : 0;
						if (partyRscId)
						{
							CVideoBridgePartyCntl* pPartyVideoSourceCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyRscId);
							if (pPartyVideoSourceCntl)
								pPartyVideoSourceCntl->MarkAsNewVideoSource(1);
						}
					}
					else
					{
						TRACEINTO << "PartyId:" << confSource->GetPartyRsrcID() << " - Send change layout to conf source with conf source layout";
					}
					confSource->ChangeConfLayout(pPartyLayout);
				}
			}
		}

		// send change layout to all other participants
		if (is_conf_layout_changed || is_one_of_parties_layout_changed)
		{
			/*VNGR-9033 :
			 * When switching in conf with more then 80 parties last parties does not change layout
			 * */
			TRACEINTO << "ConfName:" << m_pConfName;
			CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
			DWORD SleepTime  = 0;
			if (pSysConfig)
				pSysConfig->GetDWORDDataByKey("SLEEP_VALUE", SleepTime);

			// send conf layout
			int i = 0;
			CBridgePartyList::iterator _end = m_pPartyList->end();
			for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr, ++i)
			{
				if (i % 20 == 0 && i != 0)
				{
					PTRACE2INT(eLevelInfoHigh, "CVideoBridgeVSW:SendChangeLayout go to sleep for ", SleepTime);
					SystemSleep(SleepTime);
				}

				// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
				CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
				if (pPartyCntl)
				{
					if (pPartyCntl != confSource && pPartyCntl != pAddParty)
					{
						if (pPartyCntl->IsConnectedStandalone() == FALSE) // dont send change layout in slide
						{
							isPartyLayoutChanged = FALSE;
							pPartyLayout =  FindBestLayoutForParty(confLayout, pPartyCntl, confSource, confSourceLayout, isPartyLayoutChanged);
							if ((*pPartyLayout)[0] == NULL)
							{
								TRACEINTO << "PartyId:" << pPartyCntl->GetPartyRsrcID() << " - Party Layout is empty, avoid change layout";
								continue;
							}

							if (isPartyLayoutChanged)
							{
								if (pPartyLayout != confLayout)
								{
									TRACEINTO << "PartyId:" << pPartyCntl->GetPartyRsrcID() << " - Send change layout with party layout to Party";
									PartyRsrcID partyRscId = (*pPartyLayout)[0] ? (*pPartyLayout)[0]->GetImageId() : 0;
									if (partyRscId)
									{
										CVideoBridgePartyCntl* pPartyVideoSourceCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyRscId);
										if (pPartyVideoSourceCntl)
											pPartyVideoSourceCntl->MarkAsNewVideoSource(1);
									}
								}
								else
								{
									TRACEINTO << "PartyId:" << pPartyCntl->GetPartyRsrcID() << " - Send change conference layout to Party";
								}
								pPartyCntl->ChangeConfLayout(pPartyLayout);
							}
						}
					}
				}
			}
		}

		// send layout to added party
		if (pAddParty != NULL)
		{
			isPartyLayoutChanged = FALSE;
			pPartyLayout =  FindBestLayoutForParty(confLayout, pAddParty, confSource, confSourceLayout, isPartyLayoutChanged);
			// Romem klocwork
			if ((*pPartyLayout)[0] == NULL)
			{
				PTRACE2(eLevelInfoNormal, "CVideoBridgeVSW::SendChangeLayout, Party Layout of New Added party is empty, avoid change layout.  Party Name ", pAddParty->GetFullName());
			}
			else
			{
				if (pPartyLayout == confLayout)
				{
					if (!is_conf_layout_changed && !(is_conf_source_layout_changed && confSource == pAddParty))
					{
						PTRACE2(eLevelInfoNormal, "CVideoBridgeVSW::SendChangeLayout send change layout with conference layout to new added party ", pAddParty->GetFullName());
						if (CPObject::IsValidPObjectPtr(pAddParty))
						{
							if (isPartyLayoutChanged)
								pAddParty->ChangeConfLayout(confLayout);
						}
					}
				}
				else
				{
					if (CPObject::IsValidPObjectPtr(pAddParty))
					{
						DWORD partyRscId = (*pPartyLayout)[0] ? (*pPartyLayout)[0]->GetImageId() : 0;
						if (partyRscId)
						{
							CVideoBridgePartyCntl* pPartyVideoSourceCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyRscId);
							if (CPObject::IsValidPObjectPtr(pPartyVideoSourceCntl))
								pPartyVideoSourceCntl->MarkAsNewVideoSource(1);
						}

						if (isPartyLayoutChanged)
							pAddParty->ChangeConfLayout(pPartyLayout);
					}
				}
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Change layout help internal function
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ---------------------------------------------------------------------------------------------------------------------------------
BYTE CVideoBridgeVSW::IsConfLayoutChanged(CLayout& confNewLayout)
{
	BYTE is_changed = YES;
	if(m_pConfLayout!=NULL){
	 	if(confNewLayout == *m_pConfLayout){
	 		is_changed = NO;
	 	}
	}

	return is_changed;
}
// ---------------------------------------------------------------------------------------------------------------------------------
BYTE CVideoBridgeVSW::IsConfSourceLayoutChanged(CVideoBridgePartyCntl* newConfSource,CLayout& confSourceNewLayout)
{
	BYTE is_changed = YES;

	if(newConfSource == NULL)
	{
		PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::IsConfSourceLayoutChanged, new conf source is NULL ", m_pConfName);
		// this is error, we retutn NO in order to not change the layout
		is_changed = NO;
	}else{
		if(newConfSource==m_pConfSourceBridgeParty){ // same speaker
				if(confSourceNewLayout==*m_pConfSourceLayout){
					is_changed = NO;
					PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::IsConfSourceLayoutChanged, same conf source, and layout not changed ", m_pConfName);
				}else{
					PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::IsConfSourceLayoutChanged, same conf source, layout changed ", m_pConfName);
				}
		}else{
			if(m_pConfSourceBridgeParty==NULL)// first party connection
			{
				PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::IsConfSourceLayoutChanged, conf source changed, old conf source is NULL ", m_pConfName);
			}else{
				// speaker changed
				CLayout*  pConfSourceCurrentLayout = newConfSource->GetCurrentLayout();
				if( pConfSourceCurrentLayout!= NULL && (confSourceNewLayout == *pConfSourceCurrentLayout) ){
					is_changed = NO;
					PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::IsConfSourceLayoutChanged, conf source changed, but conf source layout did not changed ", m_pConfName);
				}else{
					PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::IsConfSourceLayoutChanged, conf source changed, and conf source layout changed ", m_pConfName);
				}

//				const CTaskApp* oldVideoSource = newConfSource->GetPartySouceInCellZero();
//				CVidSubImage* pSubImage = confSourceNewLayout.GetSubImageNum(0);
//				const CTaskApp* newVideoSource =  pSubImage->GetVidSrc();

//				if(oldVideoSource == newVideoSource){
//					is_changed = NO;
//					PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::IsConfSourceLayoutChanged, conf source changed, but conf source layout did not changed ", m_pConfName);
//				}else{
//					PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::IsConfSourceLayoutChanged, conf source changed, and conf source layout changed ", m_pConfName);
//				}
			}
		}
	}
	return is_changed;
}
// ---------------------------------------------------------------------------------------------------------------------------------

BYTE CVideoBridgeVSW::IsLayoutChangeAfterSpeakerChangeForConfWith2Parties(CVideoBridgePartyCntl* newConfSource,CLayout& confSourceNewLayout,CLayout& confNewLayout)
{
	BYTE is_changed = YES;

	WORD numOfParties = GetNumConntectedParties();

	if(numOfParties != 2 || m_pConfSourceLayout == NULL || m_pConfLayout == NULL || newConfSource == NULL)
	{
		return is_changed;
	}

	if(newConfSource!=m_pConfSourceBridgeParty) // speaker changed
	{
		if(confNewLayout == *m_pConfSourceLayout && confSourceNewLayout == *m_pConfLayout)
		{
			is_changed = NO;
		}

	}

	return is_changed;
}
// ---------------------------------------------------------------------------------------------------------------------------------
WORD CVideoBridgeVSW::GetNumConntectedParties()
{
	WORD count = 0;

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
		{
			if (!pPartyCntl->IsConnectedStandalone())
				count++;
		}
	}
	return count;
}
// ---------------------------------------------------------------------------------------------------------------------------------


/*BYTE CVideoBridgeVSW::IsOldConfSourceLayoutChanged(CVideoBridgePartyCntl* newConfSource, CLayout& confNewLayout)
{
	BYTE is_changed = YES;

	if(newConfSource != NULL && m_pConfSourceBridgeParty!=NULL && newConfSource!=m_pConfSourceBridgeParty)
	{
		// previous speaker changed
		if(confNewLayout == *m_pConfSourceLayout){
			is_changed = NO;
			PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::IsOldConfSourceLayoutChanged, previous speaker layout don't change ", m_pConfName);
		}else{
			PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::IsConfSourceLayoutChanged, conf source changed, and conf source layout changed ", m_pConfName);
		}
	}
	return is_changed;
}*/
//--------------------------------------------------------------------------
CVideoBridgePartyCntl* CVideoBridgeVSW::GetVideoSourceCntl(CLayout& layout)
{
	CVidSubImage* pSubImage = layout.GetSubImageNum(0);
	if (CPObject::IsValidPObjectPtr(pSubImage))
	{
		DWORD partyRscId = pSubImage->GetImageId();
		if (partyRscId)
			return (CVideoBridgePartyCntl*)GetPartyCntl(partyRscId);
	}
	return NULL;
}

//--------------------------------------------------------------------------
CVideoBridgePartyCntl* CVideoBridgeVSW::GetVideoSourceCntl(CVideoBridgePartyCntl* pTargetParty)
{
	TRACECOND_AND_RETURN_VALUE(!pTargetParty, "CVideoBridgeVSW::GetVideoSourceCntl - Failed, The target party is NULL, ConfName:" << m_pConfName, NULL);

	CLayout* pPartyCurrentLayout = pTargetParty->GetCurrentLayout();
	TRACECOND_AND_RETURN_VALUE(!pPartyCurrentLayout, "CVideoBridgeVSW::GetVideoSourceCntl - Failed, The target party current layout is NULL, ConfName:" << m_pConfName, NULL);

	CVideoBridgePartyCntl* video_source = GetVideoSourceCntl(*pPartyCurrentLayout);
	TRACECOND_AND_RETURN_VALUE(!video_source, "CVideoBridgeVSW::GetVideoSourceCntl - Failed, The video source is NULL, self-view is returned, ConfName:" << m_pConfName, pTargetParty);

	return video_source;
}

// ---------------------------------------------------------------------------------------------------------------------------------
WORD CVideoBridgeVSW::GetNumOfChangeLayoutWaitingForAck(BYTE rChangeLayoutAgain)
{
	WORD num_of_not_completed = 0;

	std::ostringstream msg;
	msg << "{";

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
		{
			if (pPartyCntl->IsWaitingForChangeLayoutAck())
			{
				num_of_not_completed++;
				if (rChangeLayoutAgain)
				{
					msg << pPartyCntl->GetPartyRsrcID() << ",";
					SendChangLAyoutAgain(pPartyCntl);
				}
			}
		}
	}
	msg << "}";

	if (rChangeLayoutAgain)
	{
		TRACEINTO << "Waiting for ACK " << msg.str().c_str();
		if (num_of_not_completed)
			AskIntraFromNewVideoSource();
	}
	return num_of_not_completed;
}

// ---------------------------------------------------------------------------------------------------------------------------------
const CImage*	CVideoBridgeVSW::GetForcedParty()
{
	const CImage* forcedVideoSource = NULL;
	CVidSubImage* pSubImage = m_pReservationLayout->GetSubImageNum(0);
	if(pSubImage){
		// if force exist
		if(pSubImage->isForcedInConfLevel())
		{
			// get party cntl from bridge by name
			const char* pForcedPartyName = pSubImage->GetPartyForce();
			CVideoBridgePartyCntl* 	pForcedPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl (pForcedPartyName);
			if(pForcedPartyCntl){
				// forced party found in bridge
				forcedVideoSource =  pForcedPartyCntl->GetPartyImage();
				PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::GetForcedParty forced party found, name - ", pForcedPartyName);

			}else{
				// forced party not connected to bridge
				PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::GetForcedParty forced party not connected to bridge, force party name - ", pForcedPartyName);
			}
		}
	}
	return forcedVideoSource;
}

// ---------------------------------------------------------------------------------------------------------------------------------
void CVideoBridgeVSW::GetForcedParty(DWORD& forcedPartyId, DWORD& secondPriorForcedPartyId)
{
	forcedPartyId            = 0;
	secondPriorForcedPartyId = 0;

	// Find lecturer - if lecturer exist he will be forced
	if (m_pLectureModeParams->IsLectureModeOn())
	{
		const char* pLecturerName = m_pLectureModeParams->GetLecturerName();
		CVideoBridgePartyCntl* pLecturerPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(pLecturerName);
		if (pLecturerPartyCntl)
		{
			// forced party found in bridge
			if (IsPartyImageExistInImageVector(pLecturerPartyCntl->GetPartyRsrcID()))
			{
				forcedPartyId = pLecturerPartyCntl->GetPartyRsrcID();
				TRACEINTO << "CVideoBridgeVSW::GetForcedParty - Lecturer found, PartyName:" << pLecturerName;
			}
			else
			{
				TRACEINTO << "CVideoBridgeVSW::GetForcedParty - Failed, Lecturer image not in image vector, PartyName:" << pLecturerName;
			}
		}
		else
		{
			// forced party not connected to bridge
			TRACEINTO << "CVideoBridgeVSW::GetForcedParty - Failed, Lecturer not connected to bridge, PartyName:" << pLecturerName;
		}
	}

	// find forced party
	CVidSubImage* pSubImage = m_pReservationLayout->GetSubImageNum(0);
	if (pSubImage)
	{
		// if force exist
		if (pSubImage->isForcedInConfLevel())
		{
			// get party cntl from bridge by name
			const char*            pForcedPartyName = pSubImage->GetPartyForce();
			CVideoBridgePartyCntl* pForcedPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(pForcedPartyName);
			if (pForcedPartyCntl)
			{
				if (IsPartyImageExistInImageVector(pForcedPartyCntl->GetPartyRsrcID()))
				{
					// forced party found in bridge
					if (forcedPartyId == 0)
					{
						forcedPartyId =  pForcedPartyCntl->GetPartyRsrcID();
					}
					else
					{
						// if force party is lecturer, the lecturer will see the conf force
						secondPriorForcedPartyId = pForcedPartyCntl->GetPartyRsrcID();
					}

					TRACEINTO << "CVideoBridgeVSW::GetForcedParty - Forced party found, PartyName:" << pForcedPartyName;
				}
				else
				{
					TRACEINTO << "CVideoBridgeVSW::GetForcedParty - Failed, Forced image not in image vector, PartyName:" << pForcedPartyName;
				}
			}
			else
			{
				// forced party not connected to bridge
				TRACEINTO << "CVideoBridgeVSW::GetForcedParty - Failed, Forced party not connected to bridge, PartyName:" << pForcedPartyName;
			}
		}
	}
}

//--------------------------------------------------------------------------
const CImage* CVideoBridgeVSW::GetVideoSourceImage(CVideoBridgePartyCntl* pTargetParty)
{
	CVidSubImage* pSubImage = NULL;
	if (pTargetParty == m_pConfSourceBridgeParty)
		pSubImage = m_pConfSourceLayout->GetSubImageNum(0);
	else
		pSubImage = m_pConfLayout->GetSubImageNum(0);

	if (pSubImage)
	{
		DWORD partyRscId = pSubImage->GetImageId();
		if (partyRscId)
		{
			CImage* pImage = GetPartyImageLookupTable()->GetPartyImage(partyRscId);
			PASSERTSTREAM_AND_RETURN_VALUE(!pImage, "CVideoBridgeVSW::GetVideoSourceImage - Failed, The lookup table doesn't have an element, PartyId:" << partyRscId, NULL);
			return pImage;
		}
	}
	return NULL;
}

//--------------------------------------------------------------------------
const CTaskApp* CVideoBridgeVSW::GetVideoSource(CVideoBridgePartyCntl* pTargetParty)
{
	const CImage* pImage = GetVideoSourceImage(pTargetParty);
	return (pImage) ? pImage->GetVideoSource() : NULL;
}

//--------------------------------------------------------------------------
void CVideoBridgeVSW::GetUnmutedSpeaker(DWORD& currSpeakerId, DWORD& prevSpeakerId) const
{
	currSpeakerId = 0;
	prevSpeakerId = 0;

	bool curr_speaker_found = NO;
	bool prev_speaker_found = NO;

	// Get speaker and previous speaker
	WORD partyImageVectorSize = GetPartyImageVectorSize();
	for (WORD i = 0; i < partyImageVectorSize; ++i)
	{
		DWORD partyId = GetPartyImageIdByPosition(i);
		const CImage* pImage = GetPartyImageLookupTable()->GetPartyImage(partyId);
		PASSERTSTREAM(!pImage, "CVideoBridgeVSW::GetUnmutedSpeaker - Failed, The lookup table doesn't have an element, PartyId:" << partyId);

		if (pImage)
		{
			const CTaskApp* pVideoSource = pImage->GetVideoSource();
			TRACECOND(!pVideoSource, "CVideoBridgeVSW::GetUnmutedSpeaker - Image found, but video source is NULL, PartyId:" << partyId);

			if (!pImage->isMuted())
			{
				if (curr_speaker_found == NO)
				{
					currSpeakerId = pImage->GetArtPartyId();
					curr_speaker_found = YES;
				}
				else if (prev_speaker_found == NO)
				{
					prevSpeakerId = pImage->GetArtPartyId();
					prev_speaker_found = YES;
					break;
				}
			}
		}
	}
}

//--------------------------------------------------------------------------
BYTE CVideoBridgeVSW::UpdateNewVideoSpeaker()
{
	TRACECOND_AND_RETURN_VALUE(!m_pLastActiveVideoSpeakerRequest, "Failed, New video speaker is NULL", NO);

	PartyRsrcID speakerPartyId = ((CParty*)m_pLastActiveVideoSpeakerRequest)->GetPartyRsrcID();

	CVideoBridgePartyCntl* pNewSpeakerPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(speakerPartyId);
	TRACECOND_AND_RETURN_VALUE(!pNewSpeakerPartyCntl, "PartyId:" << speakerPartyId << " - Failed, party not connected to bridge", NO);

	if (!IsPartyImageExistInImageVector(speakerPartyId))
	{
		// in case the participant is not connected + synced yet
		// we save last active speaker request
		// and resend it after the party connects
		// in function AddPartyToConfMixAfterVideoInSynced
		TRACEINTO << "PartyId:" << speakerPartyId << " - Failed, party is not connected to bridge";
		return NO;
	}

	const CImage* pImage = GetPartyImageLookupTable()->GetPartyImage(speakerPartyId);
	PASSERTSTREAM_AND_RETURN_VALUE(!pImage, "PartyId:" << speakerPartyId, NO);

	// only when video decoder is fully connected
	TRACEINTO << "SpeakerPartyId:" << speakerPartyId;

	// update image vector
	SetPartyImageSpeakerId(speakerPartyId);

	if (pImage->isMuted())
	{
		TRACEINTO << "SpeakerPartyId:" << speakerPartyId << " - Failed, speaker image is muted";
		return NO;
	}

	return YES; // continue to change layout
}

// ---------------------------------------------------------------------------------------------------------------------------------
BYTE  CVideoBridgeVSW::UpdateReservationLayout(CSegment* pParam)
{
	//set conf video layout requested by operator in Conf Level
	PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::UpdateReservationLayout : Name - ",m_pConfName);

	CVideoLayout   layout;
	layout.DeSerialize(NATIVE,*pParam );

	LayoutType newLayoutType=GetNewLayoutType(layout.GetScreenLayout ());
	if(newLayoutType!=CP_LAYOUT_1X1){
		PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::UpdateReservationLayout : can`t change layout other then CP_LAYOUT_1X1 in vsw conf, ",m_pConfName);
		return NO;
	}

	if(m_pReservationLayout)
	{
		if((*m_pReservationLayout)==layout)
		{
			PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::UpdateReservationLayout : received same layout, Name - ",m_pConfName);
			return NO;
		}

		m_pReservationLayout->SetLayout(layout,CONF_lev);
	}

    return YES;
}

// ---------------------------------------------------------------------------------------------------------------------------------
void CVideoBridgeVSW::CreateAndNewPartyCntl(CVideoBridgePartyCntl*& pVideoBrdgPartyCntl,
										 CVideoBridgePartyInitParams* pVideoBridgePartyInitParams)
{
	NewPartyCntl(pVideoBrdgPartyCntl);
	pVideoBrdgPartyCntl->Create(pVideoBridgePartyInitParams);
}

// ---------------------------------------------------------------------------------------------------------------------------------
void CVideoBridgeVSW::NewPartyCntl(CVideoBridgePartyCntl*& pVideoBrdgPartyCntl)
{
	pVideoBrdgPartyCntl = new CVideoBridgePartyCntlVSW();
}
//--------------------------------------------------------------------------
void CVideoBridgeVSW::SetSwitchInSwitch(CVideoBridgePartyCntl* pAddParty)
{
	PTRACE2(eLevelInfoNormal, "CVideoBridgeVSW::SetSwitchInSwitch , ConfName - ", m_pConfName);

	// if new party connected during switch we do not wait to end of switch
	// we send change layout with the conf layout to the new party
	if (CPObject::IsValidPObjectPtr(pAddParty))
	{
		PTRACE2(eLevelInfoNormal, "CVideoBridgeVSW::SetSwitchInSwitch party added during switch send conf layout to party - ", pAddParty->GetName());
		if (pAddParty->IsConnectedStandalone() == FALSE) // dont send change layout in slide
		{
			BYTE isPartyLayoutChanged = FALSE;
			CLayout* pPartyLayout =  FindBestLayoutForParty(m_pConfLayout, pAddParty, m_pConfSourceBridgeParty, m_pConfSourceLayout, isPartyLayoutChanged);
			if (isPartyLayoutChanged)
			{
				// Romem klocwork
				if ((*pPartyLayout)[0] == NULL)
				{
					PTRACE2(eLevelInfoNormal, "CVideoBridgeVSW::SetSwitchInSwitch, Party Layout of New Added party is empty, avoid change layout.  Party Name ", pAddParty->GetName());
				}
				else
				{
					pAddParty->ChangeConfLayout(pPartyLayout); // send change layout (during switch)
					DWORD partyRscId = (*pPartyLayout)[0]->GetImageId();
					if (partyRscId)
					{
						CVideoBridgePartyCntl* pPartyVideoSourceCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyRscId);
						if (CPObject::IsValidPObjectPtr(pPartyVideoSourceCntl))
							pPartyVideoSourceCntl->MarkAsNewVideoSource(1); // mark party source for intra
					}
				}
			}
		}
	}
	else
	{
		m_SwitchInSwitch = YES;
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------
// VSW patch because receinig wrong value (illegal) of mpi from ip party controll
WORD CVideoBridgeVSW::CorrectPartyInitParams(CBridgePartyInitParams& partyInitParams)
{
	WORD rval = FALSE;

	// protection because of receiving wrong (althogh ligal) resolution value from ip party controll
	// receiving of 16CIF instead of HD caused wron values to pass to video card.

	PASSERTSTREAM_AND_RETURN_VALUE(!(CBridgePartyVideoInParams*)partyInitParams.GetMediaOutParams() && !(CBridgePartyVideoInParams*)partyInitParams.GetMediaInParams(), "MediaInOutParams is NULL", rval);

	eVideoResolution in_resolution = eVideoResolutionDummy;
	eVideoFrameRate in_frame_rate = eVideoFrameRateDUMMY;
	if ((CBridgePartyVideoInParams*)partyInitParams.GetMediaInParams())
	{
		in_resolution = ((CBridgePartyVideoInParams*)partyInitParams.GetMediaInParams())->GetVideoResolution();
		in_frame_rate = ((CBridgePartyVideoInParams*)partyInitParams.GetMediaInParams())->GetVideoFrameRate(in_resolution);
	}

	eVideoResolution out_resolution = eVideoResolutionDummy;
	eVideoFrameRate out_frame_rate = eVideoFrameRateDUMMY;
	if ((CBridgePartyVideoInParams*)partyInitParams.GetMediaOutParams())
	{
		out_resolution = ((CBridgePartyVideoOutParams*)partyInitParams.GetMediaOutParams())->GetVideoResolution();
		out_frame_rate = ((CBridgePartyVideoOutParams*)partyInitParams.GetMediaOutParams())->GetVideoFrameRate(out_resolution);
	}

	// protection because of receiving illigal MPI value from ip party controll
	// receiving of 0 instead of 1 caused IsValidParams() to fail, and the party to connect secondary.
	if(in_frame_rate != eVideoFrameRate30FPS && partyInitParams.GetMediaInParams()){
		CSmallString sstr;
		((CBridgePartyVideoInParams*)partyInitParams.GetMediaInParams())->SetVideoFrameRate(in_resolution, eVideoFrameRate30FPS);
		sstr << "video in frame rate is " << in_frame_rate << ", fixed to eVideoFrameRate30FPS (=1)";
		PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::CorrectPartyInitParams: ",sstr.GetString());
	}
	if(out_frame_rate != eVideoFrameRate30FPS && partyInitParams.GetMediaOutParams()){
		CSmallString sstr;
		((CBridgePartyVideoOutParams*)partyInitParams.GetMediaOutParams())->SetVideoFrameRate(out_resolution, eVideoFrameRate30FPS);
		sstr << "video out frame rate is " << out_frame_rate << ", fixed to eVideoFrameRate30FPS (=1)";
		PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::CorrectPartyInitParams: ",sstr.GetString());
	}

	if(in_resolution != eVideoResolutionHD720 && partyInitParams.GetMediaInParams()){
		CSmallString sstr;
		((CBridgePartyVideoInParams*)partyInitParams.GetMediaInParams())->SetVideoResolution(eVideoResolutionHD720);
		sstr << "video in resolution is " << in_resolution << ", fixed to eVideoResolutionHD720 (=8)";
		PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::CorrectPartyInitParams: ",sstr.GetString());
	}
	if(out_resolution != eVideoResolutionHD720  && partyInitParams.GetMediaOutParams()){
		CSmallString sstr;
		((CBridgePartyVideoOutParams*)partyInitParams.GetMediaOutParams())->SetVideoResolution(eVideoResolutionHD720);
		sstr << "video out resolution is " << out_resolution << ", fixed to eVideoResolutionHD720 (=8)";
		PTRACE2(eLevelInfoNormal,"CVideoBridgeVSW::CorrectPartyInitParams: ",sstr.GetString());
	}
	if(partyInitParams.IsValidParams()){
		rval = TRUE;
	}

	return rval;
}
// ---------------------------------------------------------------------------------------------------------------------------------

// VSW patch because receinig wrong value (illegal) of mpi from ip party controll
void CVideoBridgeVSW::SendChangLAyoutAgain(CVideoBridgePartyCntl* pCurrParty)
{
  BYTE isPartyLayoutChanged = FALSE;
  CLayout* pPartyLayout = NULL;
  PTRACE(eLevelInfoHigh,"CVideoBridgeVSW::SendChangLAyoutAgain: Send ChangeLayout Again ");
              if(CPObject::IsValidPObjectPtr(pCurrParty) )
                        {
                                if(pCurrParty!=m_pConfSourceBridgeParty){
                                        if(pCurrParty->IsConnectedStandalone()==FALSE){// dont send change layout in slide

                                                pPartyLayout =  FindBestLayoutForParty(m_pConfLayout,pCurrParty,m_pConfSourceBridgeParty,m_pConfSourceLayout,isPartyLayoutChanged);
                                                pCurrParty->ChangeConfLayout(pPartyLayout);
                                                    }
                                                }
                                        }
}
