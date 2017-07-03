//+========================================================================+
//                            SIPPartyControl.h                      	   |
//            Copyright 2005 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPPartyControl.h                                     	   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+

#ifndef __SIPPARTYCNTL__
#define __SIPPARTYCNTL__

#include "IpPartyControl.h"
#include "SipConfPartyDefinitions.h"
#include "CsInterface.h"
#include "MSOrganizerMngr.h"
#include "MSFocusMngr.h"
#include "SipStructures.h"
#include "MSSubscriberMngr.h"
#include "MsSlavesController.h"

class CScpHandler;


class CSipPartyCntl: public CIpPartyCntl
{
CLASS_TYPE_1(CSipPartyCntl, CIpPartyCntl)
public:
	CSipPartyCntl();
	virtual ~CSipPartyCntl();
	virtual void Destroy();
	virtual void AllocMemory();

	CSipPartyCntl& operator=(const CSipPartyCntl& other);

	virtual const char* NameOf() const {return "CSipPartyCntl";}
	virtual void* GetMessageMap() {return (void*)m_msgEntries;}
	virtual DWORD GetRemoteCapsVideoRate(CapEnum protocol) const;
	//virtual BOOL IsPluginPartyCall() const {return FALSE;}
//	virtual DWORD GetLocalCapsVideoRate(CapEnum protocol) const;
//	CSipCaps* GetSipRemoteCap() const;
//	virtual void DisconnectVideoChannels();
	CSipNetSetup* GetSipNetSetup() {return m_pSIPNetSetup;};
	virtual BYTE IsRemoteCapNotHaveVideo() const;
	virtual WORD IsSupportErrorCompensation();
	virtual BOOL IsDisconnectionBecauseOfNetworkProblems() const;
	virtual	BOOL IsRedialImmediately();

	//IMHO should be in IPPartyControl level
	virtual void  ImplementUpdateSecondaryInPartyControlLevel();
	virtual void  ImplementSecondaryInPartyLevel();
	//IMHO end here

	// general update after RTP indication (not part of the signaling flow)
	void OnVideoInBrdgUpdatedPartyReCaps(CSegment* pParam); //VSGNINJA-589
	void OnVideoInBrdgUpdatedAnycase(CSegment* pParam);
	void OnVideoOutBrdgUpdatedPartyReCaps(CSegment* pParam); //VSGNINJA-589
	void OnVideoOutBrdgUpdatedAnycase(CSegment* pParam);
	// code for signaling flow that include update of bridges
	void OnVideoInBrdgUpdated(CSegment* pParam);
	void OnVideoOutBrdgUpdated(CSegment* pParam);
	virtual void HandleVideoBridgeUpdate(WORD status, EUpdateBridgeMediaAndDirection eUpdatedBridges){}
	virtual CIpComMode*	GetScmForVideoBridgeConnection(cmCapDirection direction);
	void OnPartyCntlScpRequestByEP(CSegment* pParam);
	void OnPartyCntlScpNotificationByEP(CSegment* pParam);
	void OnPartyCntlFirRequestByEP(CSegment* pParam);

	// SCP Streams notification
	void OnPartyCntlScpNotificationFromBridgeReq(CSegment* pParam);
	void OnConfApiScpNotificationAckFromEP(CSegment* pParam);
	void OnPartyCntlScpNotificationReqTout(CSegment* pParam);
	void SendScpNotificationToParty(unsigned int channelHandle);
	void StartScpNotificationTimer(DWORD iterationNum, DWORD channelHandle, DWORD notification_seq_num);
	// SCP for IVR state notifications
	void OnPartyCntlScpIvrShowSlideFromBridgeReqANYCASE(CSegment* pParam);
	void OnPartyCntlScpIvrStopShowSlideFromBridgeReqANYCASE(CSegment* pParam);
	void OnPartyCntlScpIvrMessageFromBridge(eIvrState newState);
	void SendScpIvrStateNotificationToParty(const ScpIvrStateNotification& rScpIvrStateNotification);
	void StartScpIvrStateNotificationTimer(DWORD iterationNum, DWORD notification_seq_num);
	void OnPartyCntlScpIvrStateNotificationReqToutANYCASE(CSegment* pParam);
	//CCS
	virtual void  OnPartyAuthCompleate(CSegment* pParam);
	// SCP for Pipes mapping notifications
	void OnPartyCntlScpPipesMappingNotificationFromBridgeReq(CSegment* pParam);
	void SendScpPipesMappingNotificationToParty(CScpPipeMappingNotification* pPipesMapStruct);
	void StartScpPipesMappingNotificationTimer(DWORD iterationNum, CScpPipeMappingNotification* pPipesMapStruct);
	void OnPartyCntlScpPipesMappingNotificationReqTout(CSegment* pParam);

//	void OnPartyActiveMedia(CSegment* pParam);
	void OnPartyMuteMedia(CSegment* pParam);
	void OnPartyMoveToSecondary(CSegment* pParam);
	void OnPartyUpdateBridges(CSegment* pParam);
	virtual void OnVideoBrdgConnectedIdleOrAnycase(CSegment* pParam);
	virtual void OnVideoBrdgDisconnected(CSegment* pParam);
	virtual void OnAudBrdgDisconnect(CSegment* pParam);

	virtual void SetDataForImportPartyCntl(CPartyCntl *apOtherPartyCntl);
//void UpdateVideoRatesOnAddingVideo(CComMode* pScm, CCapH263* pCapH263);
	virtual BYTE IsRemoteAndLocalCapSetHasContent(eToPrint toPrint = eToPrintNo)const;
	virtual BYTE IsLegacyContentParty();
	virtual DWORD  GetPossibleContentRate() const;
	virtual DWORD  GetMinContentPartyRate(DWORD currContentRate = 0);
	virtual BYTE IsRemoteAndLocalHasEPCContentOnly() {return NO;};
	virtual CSipCaps*  GetLocalCap()		{ return m_pSipLocalCaps;}
	virtual BYTE IsCapableOfVideo();
	virtual BYTE IsCapableOfHD720();
	virtual BYTE IsCapableOfHD1080();
	virtual BYTE  IsRemoteAndLocalHasHDContent1080() const;
	virtual BYTE  IsRemoteAndLocalHasHDContent720() const;
	virtual BYTE IsRemoteAndLocalHasHighProfileContent() const;
	virtual BYTE IsCapableOf4CIF();
	virtual eVideoPartyType GetCPVideoPartyTypeAccordingToCapabilities();
	virtual void GetRemoteCapsParams( WORD& maxMBPS, WORD& maxFS, WORD& maxDPB, WORD& maxBRandCPB, WORD& maxSAR, WORD& maxStaticMB, ERoleLabel eRole,DWORD profile = 0);

	virtual BYTE IsRemoteAndLocalCapSetHasBfcpUdp()const;
	virtual BYTE IsFirstContentNegotiation()const {return m_IsFirstContentNegotiation;}


	virtual DWORD GetConfRate() const;
	virtual DWORD GetSetupRate();
	virtual void UpdateH264ModeInLocalCaps(H264VideoModeDetails h264VidModeDetails,ERoleLabel eRole = kRolePeople);
	virtual APIS32 GetLocalCapsMbps(ERoleLabel eRole);
    virtual void Disable4CifInLocalCaps();
    virtual DWORD GetMaxFsAccordingtoProfile(APIU16 profile);
    virtual void RemoveH263H261FromLocalCaps();
	virtual void SetPartyToAudioOnly();
  	virtual BYTE AreLocalCapsSupportMedia(cmCapDataType eDataType);

  	// Bridge connection
	virtual void  InitVideoOutParams(CBridgePartyVideoOutParams *pMediaOutParams, CIpComMode* pScm, BYTE isLprInitiate = FALSE);
  	//flow control
  	virtual	void OnUpdatePartyVideoBitRate(CSegment* pParam);
  	virtual void UpdateBridgeFlowControlRateIfNeeded(CLPRParams* lprParams = NULL);

	virtual BOOL IsNeedToChangeResAccordingToRemoteRevision();
  	void OnUpdatePartyLprVideoBitRate(CSegment* pParam);
  	void HandleLprUpdatedIndications(CSegment* pParam);
  	void OnLprVideoOutBrdgBitRateUpdated(CSegment* pParam);
	//ICE
	void OnPartyUpdateICEParams(CSegment* pParam);
	void DumpUdpAddresses();
	void DumpNewUdpAddresses(UdpAddresses& newudp);
	UdpAddresses GetUdpAddress(){return m_udpAddresses; };
	void SetUdpAddress(UdpAddresses UdpAdd);
	virtual BYTE IsPartyCapsContainsH264SCM(const CMediaModeH323* H323ContentMode,ERoleLabel role);

	virtual BYTE CheckIfNeedToSendIntra();
	//RTV
//	void OnUpdatePartyVideoReolution(CSegment* pParam);
	void UpdateRtvModeInLocalCaps(RTVVideoModeDetails rtvVidModeDetails,ERoleLabel eRole,DWORD videoRate);
	void SendVideoPreferenceToParty();
	void OnSendVideoPreferenceToPartyTout(CSegment* pParam);

	virtual	void OnPartyUpdateVideoResolution(CSegment* pParam);
	virtual BYTE IsNeedToChangeVideoResourceAllocation(eVideoPartyType eCurrentVideoPartyType);
	virtual eVideoPartyType GetLocalVideoPartyType(BYTE partyTypeWithH263 = TRUE);
  	virtual BOOL IsNeedToUpdateVisualName();
	virtual	void OnPartyUpdateVideoAfterVsrMsg(CSegment* pParam);
	virtual void OnPartyAvmcu2013Detected(CSegment* pParam);
	virtual void OnMsftOutSlavesCreated(CSegment* pParam);
	void OnMsftInSlavesCreated(CSegment* pParam);

	// TIP
	void SetIsTipCall(BYTE);
	virtual BOOL GetIsTipCall() const;
	void AdjustPartynameToTip(DWORD roomId);
	void OnSlaveToMasterAckMessage(CSegment* pParam);
	void OnSlaveToMasterMessage(CSegment* pParam);
	void OnAddSlaveParty(CSegment* pParam);
	void OnFallBackFromTip(CSegment* pParam);
	void PartyToPartyCntlMessageMasterToSlave(CSegment* pParam);
	void PartyCntlToPartyMessageMasterToSlave(CSegment* pParam);
	void PartyToPartyCntlMessageSlaveToMaster(CSegment* pParam);
	void PartyCntlToPartyMessageSlaveToMaster(CSegment* pParam);
	virtual void ChangeSipfromTipToNonTip(CIpComMode* pScm,PartyControlInitParameters& partyControInitParam,PartyControlDataParameters &partyControlDataParams);
	virtual ECascadePartyType GetPartyCascadeTypeAndVendor();
	virtual BYTE GetPartyCascadeType() const;
	void OnChangeVideoOutTipPolycom(CSegment* pParam); //_t_p_
	void OnPartyCntlUpdateVidBrdgTelepresenseEPInfo(CSegment* pParam); //_e_m_

//	void PartyCntlToPartyCntlMessageMasterToSlave(CSegment* pParam);
//	void PartyCntlToPartyCntlMessageSlaveToMaster(CSegment* pParam);
	void SendMessageFromMasterToSlave(WORD destTipPartyType, DWORD opcode, CSegment *pMsg);
	void SendMessageFromSlaveToMaster(WORD srcTipPartyType, DWORD opcode, CSegment *pMsg);
	void SetTipMasterName(char* name);
	BYTE IsTipCompatibleContent();
	BYTE IsTipVideoPartyType()const;
	//BYTE IsTipSlavePartyType() const;
	// ICE
	void OnPartyFallbackFromIceToSip(CSegment* pParam);
	virtual void ChangeSipfromIceToNoneIce(PartyControlInitParameters& partyControInitParam,PartyControlDataParameters &partyControlDataParams);
    virtual void UpdateVideoOutBridgeH239Case(BYTE bTakeInitial);

	//LYNC2013_FEC_RED:
	void OnPartyCntlUpdatePartyFecOrRedVideoBitRate(CSegment* pParam);
	void OnPartyCntlSendPartySingleFecOrRedMsg(CSegment* pParam);
	void HandleFECUpdatedInd(WORD Status);
	void HandleREDUpdatedInd(WORD status);
	virtual BOOL			IsLync()													{return m_bIsLync;}

	void UpdateSvcSipPartyConnectedInCdr(CIpComMode* pIpCurrentMode);
	ECodecSubType CapEnum2ECodecSubType(CapEnum algorithm);
	void OnPartyCntlUpdateArtWithSsrc(CSegment* pParam);
    virtual BOOL GetEnableICE();

    virtual DWORD GetFirs2013Ssrc(BYTE index);


	// Channel handle update
	virtual BYTE UpdateBridgesForChannelHandle();
	virtual void OnVideoOutBrdgUpdatedChannelHandle(CSegment* pParam);
	virtual void OnVideoInBrdgUpdatedChannelHandle(CSegment* pParam);
	virtual void HandleVideoBridgeUpdateForChannelHandle(WORD status, EUpdateBridgeMediaAndDirection eUpdatedBridges) {};
	virtual void UpdateLocalCapsForHdVswInMixMode(const VideoOperationPoint *pVideoOperationPoint);

//	void SetTimerForAsSipAddContentIfNeeded();

	void SetIsBlockContentForlegacy(BYTE isBlock = FALSE) { m_bIsBlockContentForLegacy = isBlock;}
	BYTE  GetIsBlockContentForLegacy(){return m_bIsBlockContentForLegacy;}
	void OnMrmpStreamIsMustReq(CSegment* pSeg);

	void SetCSConnectionsId();
	void OnMsSlaveToMainAckMessage(CSegment* pMsg);
    void OnPartyVsrMsgInd(CSegment* pMsg);
    void OnSlavesControllerSingleVsrMassageInd(CSegment* pMsg);
    void OnSlavesControllerFullPacsiInfoInd(CSegment* pMsg);
    void OnSlavesControllerMsg(CSegment* pMsg);
    void OnPartyReceivedSingleUpdatePacsiInfoAnycase(CSegment* pParam);
   
    void OnPartyReceviedSingleUpdatePacsiInfoAnycase(CSegment* pParam);
    void BuildSingleUpdatePacsiInfo(MsSvcParamsStruct& pacsiInfo, CIpComMode* bestMode, CapEnum algorithm, BYTE isMute);
    void OnPartyReceivedFullPacsiFromMSSlavesController(CSegment* pParam);
    virtual void SendSingleUpdatePacsiInfoToSlavesController(const MsSvcParamsStruct& pacsiInfo, BYTE isReasonFecOrRed=FALSE);

    CMsSlavesController* GetMsSlavesController() { return m_pMsSlavesController; }
	RemoteIdent   GetRemoteIdentity() { return m_remoteIdentity;}
	void OnMsSlaveToMainMsgMessage(CSegment* pMsg);	
	void PartyToPartyCntlMessageMSSlaveToMain(CSegment* pParam);
	void SendMessageFromMSSlaveToMain(DWORD opcode, CSegment *pMsg);
	void OnMsSlaveToMainVideoInSyncMessage(CSegment* pMsg);

	void OnPartyRecordingControlAck(CSegment* pParam);
	void  OnPartyLayoutControl(CSegment* pParam);
	

	virtual void OnPartyChangeBridgesMuteState(CSegment* pParam);

	//FSN-613: Dynamic Content for SVC/Mix Conf
	DWORD GetPartyContentRate() {return m_partyContentRate;}
	void SetPartyContentRate(DWORD contentRate) { m_partyContentRate = contentRate;}

	void SetIsAsSipContentEnable(BOOL isAsSipContentEnable)  	{m_bIsAsSipContentEnable = isAsSipContentEnable;}
	BOOL GetIsAsSipContentEnable()  {return m_bIsAsSipContentEnable;}

protected:
	virtual void UpdateCurrentModeInDB();
	virtual eVideoPartyType GetVideoPartyTypeForRtvBframe();
	virtual DWORD GetScpNotificationRemoteSeqNumber();
	virtual void SendDisconnectMessageFromMasterToSlaves(CSegment* pParam);
	void SetSIPPartyCapsAndVideoParam(CIpComMode* pPartyScm, CSipCaps* pCap, CConfParty* pConfParty, DWORD& vidBitrate, BOOL bEnableIce,  CSipNetSetup* pNetSetup , DWORD setupRate,BYTE IsOfferer, DWORD serviceId,PartyControlInitParameters&	 partyControInitParam,PartyControlDataParameters &partyControlDataParams);
	CIpComMode* NewAndGetPartyCntlScmForFallback(PartyControlInitParameters&	 partyControInitParam,PartyControlDataParameters &partyControlDataParams );
	virtual CSipNetSetup* NewAndSetupSipNetSetup(CConfParty* pConfParty,PartyControlInitParameters& partyControInitParam,PartyControlDataParameters &partyControlDataParams);
	virtual void SetMaxBitRateInNetSetup(CSipNetSetup* pIpNetSetup, CConfParty* pConfParty, CIpComMode*pPartyScm, PartyControlInitParameters& partyControInitParam,PartyControlDataParameters &partyControlDataParams);
	virtual CSipCaps* NewAndGetLocalSipCaps(CIpComMode* pPartyScm,CConfParty* pConfParty,CSipNetSetup* pIpNetSetup,DWORD& vidBitrate,
			PartyControlInitParameters& partyControInitParam, PartyControlDataParameters& partyControlDataParams);
	void CopyNoneTipEncryptionParams(CIpComMode* pPartyScm, CIpComMode* pTempScm);
	DWORD GetUseMkiEncrytion(CConfParty* pConfParty, BYTE IsOfferer); //BRIDGE-10123
	BOOL  GetUseNonMkiEncryptOrderFirst(CConfParty* pConfParty, BYTE IsOfferer);

	PDECLAR_MESSAGE_MAP;

	WORD m_connectingState;
	EAdvancedVideoConferenceType m_eConfTypeForAdvancedVideoFeatures;

	CSipCaps*	m_pSipLocalCaps;
	CSipCaps*	m_pSipRemoteCaps;
	CMedString m_strAlternativeAddr;

	//IMHO should be in IPPartyLevelControl levelr
	CSipNetSetup* m_pSIPNetSetup;
//	UdpAddresses m_udpAddresses;
	//IMHO end here

	BYTE m_bIsOfferer;  // Is RMX side is the offerer in the invite (create) transaction
	CIceParams* m_IceParams;
	BYTE		m_IsIceParty;

	// TIP
	PartyRsrcID m_SlaveRightRsrcId;
	PartyRsrcID m_SlaveLeftRsrcId;
	PartyRsrcID m_SlaveAuxRsrcId;
  	char* m_TipMasterName;
  	BOOL  m_needToUpdateVisualName;
  	BOOL  m_IsWaitForAckForFecUpdate;
  	BOOL  m_IsWaitForAckForRedUpdate;
  	CScpHandler *m_pScpHandler;
	BOOL 	m_bIsLync;
	WORD 	m_TipNumOfScreens;

	BYTE m_IsAsSipContentEnable;
	BYTE m_IsFirstContentNegotiation;
	BOOL	m_bIsBlockContentForLegacy;
	
	BYTE 	m_TipSlaveRightAddSent;
	BYTE 	m_TipSlaveLeftAddSent;
	BYTE 	m_TipSlaveAuxAddSent;
	eVideoPartyType                m_eMaxVideoPartyTypeForTipFailure; //Bridge-10151
	int 	m_lyncRedialOutAttempt;
	DWORD 	m_partyContentRate;

	/// Lync 2013 - AVMCU
	sipSdpAndHeadersSt* 	m_MsConfReq;

//	CCsInterface*		m_pCsOrganizerInterface;
//	CRsrcParams*		m_pCsOrganizerRsrcDesc;

//	CCsInterface*		m_pCsFocusInterface;
//	CRsrcParams*		m_pCsFocusRsrcDesc;

//	CCsInterface*		m_pCsEventPackageInterface;
//	CRsrcParams*		m_pCsEventPackageRsrcDesc;

	CMSOrganizerMngr*    m_pMsOrganizerMngr;
	CMSFocusMngr*        m_pMsFocusMngr;
	CMSSubscriberMngr* m_pMsEventPackageMngr;

	char*				 m_FocusUri;
	BOOL 				 m_isDMAAVMCU;
	BOOL 				 m_EndSubscriber;
	BOOL 				 m_EndFocus;

	// Slaves Controller
	CMsSlavesController* 	m_pMsSlavesController;
	BYTE 					m_isWaitingForFullPacsi;
	MsFullPacsiInfoStruct	m_pFullPacsi;
	EVideoResolutionType    m_maxResForAvMcu;
	RemoteIdent		m_remoteIdentity;

	BOOL			m_bIsAsSipContentEnable;

};

#endif
