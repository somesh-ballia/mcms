//+========================================================================+
//                            SIPPartyControlChangeMode.h                      |
//            Copyright 2005 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       SIPPartyControlChangeMode.CPP                                     |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+

#ifndef __SIPPARTYCONTROLCHANGEMODE__
#define __SIPPARTYCONTROLCHANGEMODE__

class CSipChangeModePartyCntl: public CSipPartyCntl
{
CLASS_TYPE_1(CSipChangeModePartyCntl, CSipPartyCntl)
public:
	// Constructors
	CSipChangeModePartyCntl();
	virtual ~CSipChangeModePartyCntl();
	
	// Initializations  
	
    // Operations
	virtual void* GetMessageMap() { return (void*)m_msgEntries; }
	CSipChangeModePartyCntl& operator= (const CSipChangeModePartyCntl& other);
	// messages from conf
	virtual void ChangeScm(CIpComMode* pScm,BYTE IsAsSipContentEnable=FALSE);
	virtual void EndChangeMode(); 
	virtual const char*  NameOf() const;
	virtual void DispatchChangeModeEvent();
	virtual void ChangeModeIdle(CSegment* pParam);
	void ChangeMode(CSegment* pParam);
	void ChangeModeChangeVideoAndDataBridgesState();
	void ChangeModeInformParty();
	void OnConfChangeModeIdle(CSegment* pParam);
	void OnConfChangeModePartyReCaps(CSegment* pParam);
	void OnConfChangeModeAnycase(CSegment* pParam);
	// messages from bridges
	void OnVideoBrdgConnectedIdle(CSegment* pParam);
	void OnAudioBrdgDisconnectedPartyReCaps(CSegment* pParam);
	void OnVideoBrdgDisconnectedUpdateLegacyStatus(CSegment* pParam);
	void OnAudConnectPartyReCaps(CSegment* pParam);
	virtual BYTE ChangeVideoBrdgRsrcIfNeeded();
	BYTE isNeedTochangeAllocation(eVideoPartyType eCurrentVideoPartyType);
	void OnVideoBrdgConnectedChangeBridges(CSegment* pParam);
	void OnVideoBrdgDisconnectedChangeBridges(CSegment* pParam);
	void OnVideoBrdgDisconnectedReallocRsrc(CSegment* pParam);
	virtual void HandleVideoBridgeUpdate(WORD status, EUpdateBridgeMediaAndDirection eUpdatedBridges);
	void OnCopVideoBridgeChangeIn(CSegment* pParam);
    void OnCopVideoBridgeChangeOut(CSegment* pParam);
    
	// messages from party
	void OnPartyReCapsIdle(CSegment* pParam);
	void OnPartyRemoteConnectedResponseReCaps(CSegment* pParam);
	void OnPartyRemoteConnectedIdle(CSegment* pParam);
	void OnPartyRemoteConnectedPartyChangeMode(CSegment* pParam);
	void OnPartyReCapsChangeBridges(CSegment* pParam);
	void OnPartyReCapsChannelHandle(CSegment* pParam);
	void OnPartyReCapsPartyChangeMode(CSegment* pParam);
	void HandlePartyReCapsParams(CSegment* pParam);
	void OnUpdateRemoteCapsFromParty(CSegment* pParam);
	void StartReCaps();
	void OnPartyDisconnectBridgesIdle(CSegment* pParam);
	void OnPartyConnectBridgesIdle(CSegment* pParam);

	//timers
	void OnChangeModeToutReCaps(CSegment* pParam);
	void OnChangeModeToutChangeBridges(CSegment* pParam);
	void ChangeModeTout(CSegment* pParam);
	void OnChangeModeToutResponseParty(CSegment* pParam);
	void OnChangeModeToutPartyChangeMode(CSegment* pParam);
	void PartyChangeModeTout(CSegment* pParam);
	// Internal functions
	void ChangeInitialAccordingToNewScm(CIpComMode* pScm);
	ePartyMediaState DecideOnPartyMediaStateForCOP(BYTE bCheckSymmetry);
	void SetInitialVideoRxInCurrentIfNeeded();
	// Party update bridges (diff payload)
	void OnPartyUpdateBridgesAnycase(CSegment* pParam);
	void OnRsrcReAllocatePartyRspReAllocate(CSegment* pParam);
	BYTE HandleReallocateResponseOnChangeMode(CSegment* pParam);
	BYTE DisconnectForUpdateLegacyStatusIfNeeded(BYTE bIsNeedToChangeVideoLegacy = FALSE);
	
	//content section (Eitan)
	int  OnContentBrdgConnectedAnycase(CSegment* pParam);
	int OnContentBrdgDisconnectedAnycase(CSegment* pParam);
	void OnPartyPresentationOutStreamUpdate(CSegment* pParam);
	void OnCAMUpdatePartyInConf(CSegment* pParam);
	void ConnectPartyToContentBridge();
	void ConnectToContentBridgeIfPossible();
	virtual BYTE IsChangeContentNeeded(BYTE bSetChangeModeStateIfNeeded);
	void ChangeContentBridgeStateAccordingToNewMode();
	BYTE IsContentProtocolNeedToBeChanged(/*BYTE isSecondaryCondition*/) const;
	BYTE IsContentHDResolutionOrMpiNeedToBeChanged(/*BYTE isSecondaryCondition*/) const;
	BYTE IsContentProfileNeedToBeChanged() const;  //HP content

	virtual void ChangeSipfromTipToNonTip(CIpComMode* pScm,CSipCaps* NewLocalCaps);
	void OnPartyRemoteConnectedReallocRsrc(CSegment* pParam);
	void OnPartyReCapsPartyReallocRsrc(CSegment* pParam);
	void HandlePartyReCapParamsInFallback(CSegment* pParam);
	void ChangeSipfromIceToNoneIce(CIpComMode* pScm,CSipCaps* pNewLocalCaps);

	void OnAudioBrdgDisconnected(CSegment* pParam);
	void OnVideoBrdgDisconnected(CSegment* pParam);
	int  OnContentBrdgDisconnected(CSegment* pParam);
	void OnFeccBrdgDisconnected(CSegment* pParam);
	void OnPartyRemoteConnectedDisconnectBridges(CSegment* pParam);
	void CheckAreAllBridgesDisconnected();

	void OnAudioBrdgConnectedConnectBridges(CSegment* pParam);
	void OnVideoBrdgConnectedConnectBridges(CSegment* pParam);
	void OnContentBrdgConnectedConnectBridges(CSegment* pParam);
	void OnFeccBrdgConnectedConnectBridges();
	void OnPartyRemoteConnectedConnectBridges(CSegment* pParam);
	void CheckAreAllBridgesConnected();
	void OnPartySendChannelHandle(CSegment* pParam);
    void OnPartySendChannelHandleAnycase(CSegment* pParam);
	virtual void HandleVideoBridgeUpdateForChannelHandle(WORD status, EUpdateBridgeMediaAndDirection eUpdatedBridges);
	void OnPartyPresentationOutStreamUpdateChannelHandle(CSegment* pParam);
	void OnPartyRemoteConnectedChannelHandle(CSegment* pParam);

	DWORD OnRsrcReAllocatePartyRspAdditionalReAllocate(CSegment* pParam);
	virtual void OnUpgradePartyToMixed(CSegment* pParam);
//	bool  checkBridgesUpgradeCompletion();
	void OnPartyUpgradeToMixTout();
//	void OnEndAudioUpgradeToMix();
//	void OnEndVideoUpgradeToMix();
        int checkShouldDisconnectCall(CSegment* pParam);
	void AddContentToScm();
	void ChangeSipfromTipToNonTip(CIpComMode* pScm,PartyControlInitParameters& partyControInitParam,PartyControlDataParameters &partyControlDataParams);
	void OnMSFocusEndDisConnection(CSegment* pParam);
//	virtual void  UpdateLegacyContentStatus(BYTE isBlockContent);
	void OnPartyLastTargetModeMsg(CSegment* pParam);
	virtual void HandlePendingScmIfNeeded();
	void ActiveMedia();

protected:
	eChangeModeState m_eChangeModeState;
	BYTE m_bPartyControlChangeScm;
	BYTE m_bStartRecapAfterChangeBridges;
	BYTE m_bStartRecapAfterUpdateChannelHandle;
	BYTE m_bRestartSipfromTipToNonTip;
	BYTE m_bShouldNotifyPartyOnSecondaryContent;
	BYTE m_bIsLegacyContent;

	bool m_bIsBridgeDisconnectedAudio;
	bool m_bIsBridgeDisconnectedVideo;
	bool m_bIsBridgeDisconnectedContent;
	bool m_bIsBridgeDisconnectedFecc;

	bool m_bIsBridgeConnectedAudio;
	bool m_bIsBridgeConnectedVideo;
	bool m_bIsBridgeConnectedContent;
	bool m_bIsBridgeConnectedFecc;


	/// ***CREATE OUT SLAVES INTEGRATION  - TEMP CODE - TO BE REMOVED***
	DWORD m_CreateOutSlavesintegrationCounter;
	/// *******

	PDECLAR_MESSAGE_MAP;	
};

class CSipChangeModeLyncPartyCntl: public CSipChangeModePartyCntl
{
	CLASS_TYPE_1(CSipChangeModeLyncPartyCntl, CSipChangeModePartyCntl)
public:
		// Constructors
		CSipChangeModeLyncPartyCntl();
		virtual ~CSipChangeModeLyncPartyCntl();
		virtual const char*  NameOf() const;
		virtual void* GetMessageMap() {return (void*)m_msgEntries;}
		void OnVideoBrdgConnectedUpdateLegacyStatus(CSegment* pParam);
		BYTE DisconnectForUpdateLegacyStatus();	
		void OnVideoBrdgDisconnectedUpdateLegacyStatus(CSegment* pParam);
		virtual void  UpdateLegacyContentStatus(BYTE isBlockContent);
		
		BYTE  DisconnectPartyFromVideoBridgeForLync();
protected:
		bool	m_bIsUpdateLegacyDueToPlugin;
		PDECLAR_MESSAGE_MAP;
};

#endif


