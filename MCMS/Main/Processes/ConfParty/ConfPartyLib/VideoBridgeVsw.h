//+========================================================================+
//                       VideoBridge.H                                     |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       VideoBridge.H                                               |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Talya                                                       |
//-------------------------------------------------------------------------|
// Who  | Date  July-2005  | Description                                   |
//-------------------------------------------------------------------------|
//
//+========================================================================+

#ifndef _CVideoBridgeVsw_H_
#define _CVideoBridgeVsw_H_

#include "VideoBridge.h"

class CVideoBridgeVSW : public CVideoBridge
{
CLASS_TYPE_1(CVideoBridgeVSW,CVideoBridge)
public:
	// constructors & destructors
	CVideoBridgeVSW ();
	virtual ~CVideoBridgeVSW ();

	virtual void	Create (const CVideoBridgeInitParams* pVideoBridgeInitParams);
	virtual void  	InitBridgeParams(const CBridgePartyInitParams* pBridgePartyInitParams);
	const char*	NameOf () const;

	virtual void CreateAndNewPartyCntl(CVideoBridgePartyCntl*& pVideoBrdgPartyCntl,
										 CVideoBridgePartyInitParams* pVideoBridgePartyInitParams);
	// 	object & state machine virtual functions
	virtual void*  GetMessageMap();

	// API to control
	virtual CLayout *GetReservationLayout(void)const;
	// API to layout handler vsw
	CLayout* GetConfLayout(void)const{ return m_pConfLayout;};
	CVideoBridgePartyCntl* GetConfSource(void)const{ return m_pConfSourceBridgeParty;};
	CLayout* GetConfSourceLayout(void)const{ return m_pConfSourceLayout;};

protected:
	// aplications management functions
		// lecture mode
	virtual void StartLectureMode();
	virtual void EndLectureMode(BYTE removeLecturer);

	// main scenarios functions
		// connect party
	virtual void RemovePartyFromConfMixBeforeDisconnecting(const CTaskApp *pParty);
	virtual void NewPartyCntl(CVideoBridgePartyCntl*& pVideoBrdgPartyCntl);
	virtual WORD CorrectPartyInitParams(CBridgePartyInitParams& partyInitParams);
		// disconnect / delete party
	virtual void RemovePartyFromAnyConfSettingsWhenDeletedFromConf(const char* pDeletedPartyName);
	virtual void AddPartyToConfMixAfterVideoInSynced(const CTaskApp *pParty);
	void RemoveImageFromVswLayoutData(DWORD partyRscId);
		// switch  - scenario interface
	void ChangeLayout(CVideoBridgePartyCntl* pAddParty = NULL);
	void StartSwitch(CLayout* confLayout,CVideoBridgePartyCntl* confSource,CLayout* confSourceLayout, BYTE is_conf_layout_changed, BYTE is_conf_source_layout_changed,BYTE is_one_of_parties_layout_changed,CVideoBridgePartyCntl* pAddParty = NULL);
	WORD EndSwitch();
	void AskIntraFromNewVideoSource();
	void SendChangLAyoutAgain(CVideoBridgePartyCntl* pCurrParty);
		// switch scenario - main internal functions
	WORD BuildLayout(CLayout& confLayout,CVideoBridgePartyCntl*& confSource,CLayout& confSourceLayout);
	void UpdateLayoutData(CLayout& confLayout,CVideoBridgePartyCntl* confSource,CLayout& confSourceLayout,CVideoBridgePartyCntl* pAddParty = NULL);
	void SendChangeLayout(CLayout* confLayout,CVideoBridgePartyCntl* confSource,CLayout* confSourceLayout, BYTE is_conf_layout_changed, BYTE is_conf_source_layout_changed, BYTE is_one_of_parties_layout_changed, CVideoBridgePartyCntl* pAddParty = NULL);
	WORD GetNumOfChangeLayoutWaitingForAck(BYTE rChangeLayoutAgain=0);

	//State machine action functions
	void OnConfSpeakersChangedCONNECTED(CSegment* pParam);
	void OnConfSpeakersChangedINSWITCH(CSegment* pParam);

    void OnConfSetConfVideoLayoutSeeMeAllCONNECTED(CSegment* pParam);
    void OnConfSetConfVideoLayoutSeeMeAllINSWITCH(CSegment* pParam);

    void OnConfSetConfVideoLayoutSeeMePartyCONNECTED(CSegment* pParam);
    void OnConfSetConfVideoLayoutSeeMePartyINSWITCH(CSegment* pParam);

    void OnPartyEndChangeLayoutCONNECTED(CSegment* pParam);
    void OnPartyEndChangeLayoutINSWITCH(CSegment* pParam);

	void OnConfVideoRefresh(CSegment* pParam, bool bPartyIntraSuppress, WORD wEvent);
	void OnConfVideoRefreshEncoderCONNECTED(CSegment* pParam);
	void OnConfVideoRefreshCONNECTED(CSegment* pParam);
	void  OnConfVideoRefreshINSWITCH(CSegment* pParam);
	// for Call Generator - CallG_Keren
	void OnConfVideoRefreshForCallGen(CSegment* pParam);

	void OnConfUpdateVideoMuteCONNECTED(CSegment* pParam);
	void OnConfUpdateVideoMuteINSWITCH(CSegment* pParam);

	void OnTimerEndSwitchINSWITCH(CSegment* pParam);

	void OnEndPartyConnectCONNECTED(CSegment* pParam);
	void OnEndPartyConnectINSWITCH(CSegment* pParam);
        void OnEndPartyDisConnectDISCONNECTING(CSegment* pParam);

	void OnEndPartyConnectIVRModeINSWITCH(CSegment* pParam);


	void  OnConfDisConnectPartyCONNECTED(CSegment* pParam);
	void  OnConfDisConnectPartyINSWITCH(CSegment* pParam);

	//void  OnConfAudioSpeakerChangedCONNECTED(CSegment* pParam);
	//void  OnConfAudioSpeakerChangedINSWITCH(CSegment* pParam);

	void OnTimerLectureModeCONNECTED(CSegment * pParam);
	void OnTimerLectureModeINSWITCH(CSegment * pParam);

	void OnEndPartyDisConnectCONNECTED(CSegment* pParam);
	void OnEndPartyDisConnectINSWITCH(CSegment* pParam);

		// Update rate (flow control)
	void OnConfUpdateFlowControlRateCONNECTED(CSegment* pParam);
	void OnConfUpdateFlowControlRateINSWITCH(CSegment* pParam);

		// inswitch events
	void OnConfDeletePartyFromConfINSWITCH(CSegment * pParam);
	void OnSetSiteNameINSWITCH(CSegment* pParams);
	void OnConfDisConnectConfINSWITCH(CSegment* pParam);
	void OnConfTerminateINSWITCH(CSegment* pParam);
	void OnConfConnectPartyINSWITCH(CSegment* pParam);
	void OnConfSetLectureModeINSWITCH(CSegment* pParam);

	// common implementation of action functions
	void OnConfSpeakersChanged(CSegment* pParam);
	void AudioSpeakerChanged();
	void OnEndPartyDisConnect(CSegment* pParam);

	//virtual void AudioSpeakerChanged(CTaskApp* pNewAudioSpeaker);
	void OnConfSetConfVideoLayoutSeeMeAll(CSegment* pParam);
	void OnConfSetConfVideoLayoutSeeMeParty(CSegment* pParam);
	void OnEndPartyConnect(CSegment* pParam);
	void OnConfUpdateVideoMute(CSegment * pParam);
	void OnTimerLectureMode(CSegment * pParam);

	// help internal functions
		// vsw layout management
//	BYTE IsLayoutChanged(CLayout& confNewLayout,CVideoBridgePartyCntl* newConfSource,CLayout& confSourceNewLayout);
	BYTE IsConfLayoutChanged(CLayout& confNewLayout);
	BYTE IsConfSourceLayoutChanged(CVideoBridgePartyCntl* newConfSource,CLayout& confSourceNewLayout);
	BYTE IsLayoutChangeAfterSpeakerChangeForConfWith2Parties(CVideoBridgePartyCntl* newConfSource,CLayout& confSourceNewLayout,CLayout& confNewLayout);
	void SetSwitchInSwitch(CVideoBridgePartyCntl* pAddParty);
	WORD GetNumConntectedParties();

		// get video source
	CVideoBridgePartyCntl* GetVideoSourceCntl(CLayout& layout);
	CVideoBridgePartyCntl* GetVideoSourceCntl(CVideoBridgePartyCntl* pTargetParty);
	const CImage* GetVideoSourceImage(CVideoBridgePartyCntl* pTargetParty);
	const CTaskApp* GetVideoSource(CVideoBridgePartyCntl* pTargetParty);
		// help for build layout
	const CImage*	GetForcedParty();
	void  GetForcedParty(DWORD& forcedPartyId, DWORD& secondPriorForcedPartyId);
	void  GetUnmutedSpeaker(DWORD& currSpeakerId, DWORD& prevSpeakerId) const;
		// get forces
	virtual void GetForcesFromReservation(CCommConf* pCommConf);
	void GetForcesFromReservation(CCommConf* pCommConf,CLayout& vswlayout);
	 // Choose best layout for a participant (Conference layout or forces in party level)
	CLayout* FindBestLayoutForParty(CLayout* confLayout,CVideoBridgePartyCntl* pParty,CVideoBridgePartyCntl* confSource,CLayout* confSourceLayout,BYTE& isPartyLayoutChanged);
		// data update
	BYTE UpdateNewVideoSpeaker();
	BYTE  UpdateReservationLayout(CSegment* pParam);
	virtual void UpdateDB_ConfLayout();
		// general functions
		// Flow control handling
	void OnConfUpdateFlowControlRate(CSegment* pParam);
	void HandleFlowControlRatePartyDisconnected(CSegment* pParam);
	BYTE FindLowestFlowControlRate();
	void SendFlowControlRateToAllParties(PartyRsrcID partyId = INVALID, CLPRParams* pLprParams = NULL);

	// attributes
		// active layout
	CLayout* m_pConfLayout;
	CVideoBridgePartyCntl* m_pConfSourceBridgeParty;
	CLayout* m_pConfSourceLayout;
		// conf forces
	CLayout* m_pReservationLayout;
		// switch in switchvoid  CVideoBridgeVSW::OnConfDisConnectPartyCONNECTED(CSegment* pParam)
	BYTE m_SwitchInSwitch;

	DWORD m_flowControlRate;
	PDECLAR_MESSAGE_MAP
};



#endif //_CVideoBridge_H_

