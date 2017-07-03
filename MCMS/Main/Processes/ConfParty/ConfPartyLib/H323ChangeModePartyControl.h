//+========================================================================+
//                            CH323ChangeModeCntl.H                        |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       CH323ChangeModeCntl.H                                          |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Sami                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 7/4/95     |                                                      |
//+========================================================================+
#ifndef _H323ChangePartyControl
#define _H323ChangePartyControl



#include "H323PartyControl.h"

class CH323ChangeModeCntl : public CH323PartyCntl
{
CLASS_TYPE_1(CH323ChangeModeCntl, CH323PartyCntl)
public:
	// Constructors
    CH323ChangeModeCntl();
    virtual ~CH323ChangeModeCntl();

    // Initializations
    CH323ChangeModeCntl& operator= (const CH323PartyCntl& other);
	CH323ChangeModeCntl& operator= (const CH323ChangeModeCntl& other);

    // Operations
	const virtual char*  NameOf() const;
	virtual void*  GetMessageMap();

	virtual void ChangeScm(CIpComMode* pH323Scm, EChangeMediaType eChangeMediaType, BYTE bOnlyUdateBridge = FALSE);


    // action functions
	void  OnConfChangeModeIdle(CSegment* pParam);
	void  OnConfChangeModeVideo(CSegment* pParam);
	void  OnConfChangeModeOther(CSegment* pParam);
    void  OnConfChangeModeRealloc(CSegment* pParam);
    void  OnConfChangeModeUpdateLegacyStatus(CSegment* pParam);
    void  OnAudConnectPartyIdleOrAnycase(CSegment* pParam);
	void  OnVidBrdgConVideo(CSegment* pParam);
	void  OnCamVidBrdgConVideo(CSegment* pParam);
    void  OnVidBrdgConContent(CSegment* pParam);
    void  OnVidBrdgConChangeAll(CSegment* pParam);
    void  OnVidBrdgConUpdateLegacyState(CSegment* pParam);
	void  OnVidBrdgConVideoIdle(CSegment* pParam);
    void  OnPartyEndChangeReAlloc(CSegment* pParam);
	void  OnVidBrdgDisConVideo(CSegment* pParam);
	void  OnPartyEndChangeOther(CSegment* pParam);
	void  OnPartyEndChangeIdle(CSegment* pParam);
	void  OnPartyEndChangeVideo(CSegment* pParam);
	void  OnPartyMoveToSecondary(CSegment* pParam);
	void  OnPartyMoveToSecondaryChangeMode(CSegment* pParam);
	void  OnTimerPartyVideo(CSegment* pParam);
	void  OnTimerPartyOther(CSegment* pParam);
    void  OnTimerPartyReAlloc(CSegment* pParam);
/*	void  OnDataBrdgConVideo(CSegment* pParam);
	void  OnDataBrdgConOther(CSegment* pParam);
	void  OnDataBrdgConIdle(CSegment* pParam);
    void  OnDataBrdgDisConOther(CSegment* pParam);
    void  OnDataBrdgDisChangeMode(CSegment* pParam);
	void  OnPartyH323DisconnectMmlp(CSegment* pParam);*/

	//Content:
    void  OnConfChangeModeContent(CSegment* pParam);
    virtual void  OnPartyEndChangeContent(CSegment* pParam);
    void  OnTimerPartyContent(CSegment* pParam);
 /*   virtual void OnPartyEndContentReconnectIdle(CSegment* pParam);
	virtual void OnPartyEndContentReconnectAnycaseButIdle(CSegment* pParam);
*/
	virtual int  OnContentBrdgConnected(CSegment* pParam);
	void  OnContentBrdgConnectedAnycase(CSegment* pParam);
	virtual int OnContentBrdgDisconnected(CSegment* pParam);
	virtual void OnXCodeBrdgDisconnected(CSegment* pParam);
	void OnVideoBrdgDisconnectedUpdateLegacyStatus(CSegment* pParam);


	BYTE IsInitialContentModeEqualToCurContentMode();
	void UpdateInitialWithNewContentMode(CComModeH323* pNewModeH323);

	void OnPartyMuteVideoIdle(CSegment* pParam);
	void OnPartyMuteVideoAnycase(CSegment* pParam);

	virtual WORD IsSupportErrorCompensation();
	virtual void OpenDataChannel(WORD bitRate,WORD type);
	virtual void CloseDataChannel(WORD type = 0);
	void SendFlowControlFromConf(DWORD newVidRate);

	virtual WORD IsPartyInChangeMode() const {return (m_state != IDLE);}
	virtual WORD IsFeccChanOpened() const;

	void   OnPartyH323SetSecondaryCause(CSegment* pParam);
	void   OnH323DBC2Command(CSegment* pParam);

	void   OnVideoInBrdgUpdatedChangeVideo(CSegment* pParam);
	void   OnVideoInBrdgUpdatedAnycase(CSegment* pParam);
	void   OnVideoOutBrdgUpdatedChangeVideo(CSegment* pParam);
	void   OnVideoOutBrdgUpdatedAnycase(CSegment* pParam);

	void   OnPartyH323ConnectAllIdle(CSegment* pParam);
	void   OnPartyH323ConnectAll_other(CSegment* pParam);
	void   OnPartyH323ConnectAllChangeAll(CSegment* pParam);
		
	void   UpdateVideoBrigdeStateAfterBridgeConnected(CSegment* pParam);

    virtual void   PartyH323ConnectAllPartyConnectAudioOrChangeAll(CSegment* pParam);

	virtual BYTE IsRemoteAndLocalCapSetHasContent(eToPrint toPrint = eToPrintNo)const;
	virtual BYTE IsRemoteAndLocalHasEPCContentOnly();
	//call generator
	void OnCGSendStartContent();
	void OnCGSendStopContent();

	void HandleLprUpdatedIndications(CSegment* pParam);
	virtual DWORD OnRsrcReAllocatePartyRspAdditionalReAllocate(CSegment* pParam);
	void OnPartyUpgradeToMixTout(CSegment* pParam);
	virtual void FinishUpgradeToMix();
	virtual void HandlePendingScmIfNeeded();

protected:
	WORD  HandleCpRate();
    // Operations
	virtual void  ChangeOther();
//	virtual void  OpenDaynamicLsd();
//	virtual void  CloseDaynamicLsd();

	//content:
	virtual void  ChangeContent();
	void  ContinueToChangeContent();
//    virtual BYTE  IsContentRateNeedToBeChanged();
    BYTE IsSecondaryConditionForContent(DWORD & details);
    void SetPartyToContentSecondaryMode(DWORD &details);
    BYTE IsContentOutProtocolNeedToBeChanged(BYTE isSecondaryCondition) const;
    BYTE IsContentInProtocolNeedToBeChanged(BYTE isSecondaryCondition) const;
    BYTE IsContentInHDResOrMpiNeedToBeChanged(BYTE isSecondaryCondition) const;
    BYTE IsContentOutHDResOrMpiNeedToBeChanged(BYTE isSecondaryCondition) const;
    //HP content:
    BYTE IsContentOutProfileNeedToBeChanged(BYTE isSecondaryCondition) const;
    BYTE IsContentInProfileNeedToBeChanged(BYTE isSecondaryCondition) const;
//    virtual void PartyEndContentReconnect(CSegment* pParam);

	virtual void  EndChangeMode();
    void HandleDataInMove();
	void PartyMuteVideo(CSegment* pParam);

	virtual void SetPartyToSecondaryAndStopChangeMode(BYTE reason,DWORD details = 0,BYTE direction = cmCapTransmit,CSecondaryParams *pSecParams=NULL,BYTE bDisconnectChannels = TRUE);

	//ECS:
	virtual void OnPartyReceivedReCapsChangeAll(CSegment* pParam);
	virtual void OnPartyReceivedReCapsChangeVideo(CSegment* pParam);
	virtual void OnPartyReceivedECS(CSegment* pParam);

	//Highest Common:
	virtual ePartyMediaState DecideOnPartyMediaStateForCOP(eChangeModeState& changeModeState, DWORD& details, BYTE& reason, BYTE& direction);
    virtual ePartyMediaState DecideOnPartyMediaStateForCP (eChangeModeState& changeModeState, DWORD& details, BYTE& reason, BYTE& direction);
	virtual ePartyMediaState DecideOnPartyMediaStateForVSW(eChangeModeState& changeModeState, DWORD &details, BYTE &reason, BYTE &direction);
    eChangeModeState DecideOnPartyChangeModeState(DWORD details, cmCapDirection direction);
	virtual ePartyMediaState DecideOnUpgradingFromSecondary(eChangeModeState& changeModeState);
	ePartyMediaState CheckSecondaryChanges(eChangeModeState& rChangeModeState, BYTE &reason);
	virtual BYTE ChangeVideoModeIfNeeded();
	void ChangeModeFromScm(eChangeModeState changeModeState, ePartyMediaState mediaState = eChangeMode_Must, cmCapDataType dataType = cmCapVideo, ERoleLabel role = kRolePeople,
			CRsrcParams** avcToSvcTranslatorRsrcParams = NULL,CRsrcParams* pMrmpRsrcParams = NULL);
	void OnTimerVideoUpdate(CSegment* pParam);

    BYTE IsRemoteCapsEnableChangeMode(eChangeModeState& changeModeState);
	BYTE AreAllMustChangesInModeSucceeded(BYTE& reason, DWORD& details, BYTE& direction);
    BYTE IsMustToChangeModeInCp(BYTE& reason, DWORD& details);
    BYTE IsResolutionFeetsToRateInCp(BYTE& reason, DWORD& details);
	eChangeModeState CombineChangeModeStates(eChangeModeState outgoingState, eChangeModeState incomingState);
//	BYTE ChangeScmAndCapsInCpConf(BYTE reason, eChangeModeState& changeModeState, WORD& details, BYTE& direction, WORD compareNotEqual);
	//void UpdateVideoStreamIfNeeded();
	BYTE ChangeVideoBridgeState(BOOL bForceBridgeIncomingUpdate = NO);
//    void SetMaxModeInCp(CapEnum mediaType, CBaseVideoCap* pVidCap);
    virtual void CheckDetails(DWORD* pDetails, BYTE* direction) const;

    void HandleVideoBridgeUpdate(WORD status, EUpdateBridgeMediaAndDirection eUpdatedBridges);

	virtual void  EndConnectionProcess(WORD status);

	//Conect to content bridge
    void ConnectToContentBridgeIfPossible();
    virtual void OnPartyPresentationOutStreamUpdateIdle(CSegment* pParam);
    virtual void OnPartyPresentationOutStreamUpdate(CSegment* pParam);
    void OnCAMUpdatePartyInConf(CSegment* pParam);

    //Party Msg on screen functions
    void OnVideoBridgePartyChangeLayout(CSegment* pParam);
    void StartPartyMsgOnScreen();
    BYTE SetNoContentMsg(char** displayStringArr, DWORD &displayStringArrNo);
    
    // Cop change decoder
    void OnCopVideoBridgeChangeIn(CSegment* pParam);
    void OnCopVideoBridgeChangeOut(CSegment* pParam);
    void OnCopCascadeLectureMode(CSegment* pParam);
    void OnCopCascadeStartCopLinkLecturePendingMode(CSegment* pParam);
    void OnCopCascadeCopLinkLecturePendingModeTimerExpire(CSegment* pParam);
    void OnCopNoVideoUpdatesTout(CSegment* pParam);
    virtual BYTE IsNeedToChangeDueToSwitchFromLecturerToNonLecturer(BYTE copLevel,BYTE takeInitial = TRUE);
    BYTE DisconnectForUpdateLegacyStatusIfNeeded();

	//WORD GetOldLsdRate();

	WORD			 m_lsdChangeModeRequestRate; //in H320 values
	WORD			 m_lsdOldChangeModeRequestRate;
	CComModeH323*	 m_pScmSentToParty;
	BYTE		     m_bSecondaryLimitation;
//	BYTE			 m_bChannelsUpdatedWithoutChangeMode;
	BYTE             m_bBridgeChangeToAsymmetricProtocol;

	EChangeMediaType m_eChangeMediaType;

	BYTE             m_bChangeFlag;


	// Attributes
	PDECLAR_MESSAGE_MAP
};

#endif //_H323ChangePartyControl
