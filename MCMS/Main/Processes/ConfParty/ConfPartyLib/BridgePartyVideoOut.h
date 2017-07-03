#ifndef BRIDGE_PARTY_VIDEO_OUT_H__
#define BRIDGE_PARTY_VIDEO_OUT_H__

///////////////////////////////////////////////////////////////////////////
#include "BridgePartyVideoUniDirection.h"

#include "VisualEffectsParams.h"
#include "VideoBridgePartyCntl.h"
#include "VideoDefines.h"
#include "VideoLayout.h"
#include "VideoApiDefinitions.h"
#include "TextOnScreenMngr.h"
#include "BridgePartyVideoParams.h"

///////////////////////////////////////////////////////////////////////////
#define DEFAULT_CIF_SIF             eVideoResolutionSIF
#define ENCODER_UPDATE_PARAM_TOUT   ((WORD)302)

///////////////////////////////////////////////////////////////////////////
class CBridgePartyCntl;
class CRsrcParams;
class CBridgePartyVideoOutParams;
class CTaskApi;
class CLayoutHandler;
class CLayout;
class CConf;
class CGathering;
class CSiteNameInfo;
class CVideoBridge;

////////////////////////////////////////////////////////////////////////////
class CBridgePartyVideoOut : public CBridgePartyVideoUniDirection
{
	CLASS_TYPE_1(CBridgePartyVideoOut, CBridgePartyVideoUniDirection)

	virtual ~CBridgePartyVideoOut();

	virtual const char* NameOf() const
	{ return GetCompileType(); }

public:

	enum STATE
	{
		SETUP = IDLE + 1, SLIDE, CONNECTED, CHANGE_LAYOUT, DISCONNECTING
	};

	CBridgePartyVideoOut();
	CBridgePartyVideoOut(const CBridgePartyVideoOut&);

	virtual void Connect();
	virtual void DisConnect();

	virtual void Create(const CBridgePartyCntl* pBridgePartyCntl, const CRsrcParams* pRsrcParams, const CBridgePartyMediaParams* pBridgePartyMediaParams);

	virtual CLayout* GetReservationLayout() const
	{ return m_pReservation[GetConfLayoutType()]; }

	virtual CLayout* GetCurrentLayout()
	{ return m_pCurrentView; }

	virtual CLayout* GetPrivateReservationLayout() const;
	virtual LayoutType GetPartyCurrentLayoutType() const;

	virtual CVisualEffectsParams* GetPartyVisualEffects()
	{ return m_pPartyVisualEffects; }

	virtual CSiteNameInfo* GetSiteNameInfo()
	{ return m_pSiteNameInfo; }

	virtual void SetSiteNameInfo(const CSiteNameInfo* pSiteNameInfo);

	virtual EIconType             GetRecordingType()               {return m_recordingType;}
	virtual void                  SetRecordingType(EIconType recordingType)   {m_recordingType = recordingType;}
  
	virtual WORD                  GetIsAudioParticipantsIconActive()               {return m_isAudioParticipantsIconActived;}
	virtual void                  SetAudioParticipantsIconActive(WORD isIconActive)   {m_isAudioParticipantsIconActived = isIconActive;}

	virtual WORD 			        GetNumAudioParticipantsInConf()				 {return m_numAudioParticipantsInConf;}
	virtual void				    SetNumAudioParticipantsInConf(WORD numAudio)   {m_numAudioParticipantsInConf = numAudio;}

	virtual BOOL 			        GetIsAudioIconToSentAfterOpenPort()				 {return m_isAudioIconToSentAfterOpenPort;}

	virtual void SetIsPrivateLayout(bool isPrivate);

	virtual bool GetIsPrivateLayout() const
	{ return m_isPrivate; }

	virtual WORD GetIsForce1x1() const
	{ return m_isForce1x1; }

	virtual ePartyLectureModeRole GetPartyLectureModeRole() const
	{ return m_partyLectureModeRole; }

	virtual BOOL IsConnected();
	virtual bool IsStateIsConnected();

	virtual BOOL IsConnecting()
	{ return (m_state == SETUP); }

	virtual BOOL IsDisConnected()
	{ return (m_state == IDLE || m_state == SLIDE); }

	virtual BOOL IsDisconnecting()
	{ return (m_state == DISCONNECTING); }
	const char*                   GetStateAsString(WORD state);
	virtual void RemoveConfParams();
	
	virtual void UpdateNewConfParams(ConfRsrcID confRsrcId, const CBridgePartyMediaParams* pBridgePartyVideoOutParams);
	void UpdateNewConfParamsForOpenedPortAfterMove(ConfRsrcID confRsrcId, const CBridgePartyMediaParams* pBridgePartyVideoOutParams);
	void UpdateNewConfVideoRelatedParamsAfterMove(const CBridgePartyMediaParams* pBridgePartyVideoOutParams);

	virtual void AddImage(const CTaskApp* pParty);
	virtual void VideoRefresh();
	virtual void MuteImage();
	virtual void UnMuteImage();
	virtual void UpdateImage();
	virtual void ChangeAudioSpeaker(const CTaskApp* pParty);
	virtual void ChangeSpeakers(const CTaskApp* pPartyVideo, const CTaskApp* pPartyAudio);
	virtual void DelImage(const CTaskApp* pParty);
	virtual void ChangeConfLayout(CSegment* pParam);
	virtual void ChangePartyLayout(CVideoLayout& newVideoLayout);
	virtual void ChangePartyPrivateLayout(CVideoLayout& newVideoLayout);
	virtual void ChangeLayoutPrivatePartyButtonOnly(WORD isPrivate);
	virtual void ChangeLayoutOfTPRoomSublink(CSegment* pParam);
	virtual BYTE BuildLayout(BOOL bUseSharedMemForChangeLayoutReq = false, BYTE alwaysSendToHardware = false);
	virtual void Export();
	virtual void CleanAllLayouts();
	virtual void CleanAllLayoutsAndPrivateSettings();
	virtual void UpdatePartyTelePresenceMode(eTelePresencePartyType partyNewTelePresenceMode);
	virtual void UpdateOnImageSvcToAvcTranslate();

	virtual void PLC_SetPartyPrivateLayoutType(LayoutType newPrivateLayoutType);
	virtual void PLC_ReturnToConfLayout();
	virtual void PLC_ForceToCell(char* partyImageToSee, BYTE cellToForce);
	virtual void PLC_CancelAllPrivateLayoutForces();

	virtual void UpdateVisualEffects(CVisualEffectsParams* pVisualEffects, BYTE bInternalUpdateOnly = false);
	virtual void UpdateVideoParams(CBridgePartyVideoParams* pBridgePartyVideoParams);
	virtual void ShowSlide(CSegment* pDataSeg);
	virtual void ShowSlideForIvrResume(CSegment* pDataSeg);
	virtual void StopShowSlide(CSegment* pDataSeg);
	virtual void StopShowSlideForIvrResume(CSegment* pDataSeg);
	virtual void StartPLC(CSegment* pDataSeg);
	virtual void StopPLC(CSegment* pDataSeg);
	virtual void DisplayRecordingIcon(EIconType eRecordingIcon);

	virtual void UpdateLectureModeRole(ePartyLectureModeRole partyLectureModeRole);

	virtual void DeletePartyFromConf(const char* pDeletedPartyName);
	virtual void DisplayPartyTextOnScreen(CTextOnScreenMngr& TextMsgList, DWORD timeout);
	virtual void DisplayGathering(CGathering* pGathering);
	CGathering* GetGathering();
	virtual void StopDisplayPartyTextOnScreen();
	virtual void DumpAllInfoOnConnectionState(CMedString* pMedString, bool isShortPrint = false);

	void GetBridgePartyVideoParams(CBridgePartyVideoParams& bridgePartyVideoParams) const;
	void GetBridgePartyVideoOutParams(CBridgePartyVideoOutParams& bridgePartyVideoOutParams) const;

	DWORD GetDecoderDetectedModeWidth() const
	{ return m_decoderDetectedModeWidth; }

	DWORD GetDecoderDetectedModeHeight() const
	{ return m_decoderDetectedModeHeight; }

	DWORD GetDecoderDetectedSampleAspectRatioWidth() const
	{ return m_decoderDetectedSampleAspectRatioWidth; }

	DWORD GetDecoderDetectedSampleAspectRatioHeight() const
	{ return m_decoderDetectedSampleAspectRatioHeight; }

	void UpdateDecoderDetectedMode(
		DWORD decoderDetectedModeWidth, DWORD decoderDetectedModeHeight,
		DWORD decoderDetectedSampleAspectRatioWidth, DWORD decoderDetectedSampleAspectRatioHeight);

	BYTE IsInswitch();

	void UpdatePartyOutParams(CUpdatePartyVideoInitParams* pUpdatePartyVideoInitParams);
	void UpdateVideoClarity(WORD isVideoClarityEnabled);
	void SetVisualEffectsParams(CVisualEffectsParams* pVisualEffects);
	virtual void UpdateMessageOverlay(CMessageOverlayInfo* pMessageOverlayInfo);
	virtual void UpdateMessageOverlayStop();
	void ChangeSpeakerNotationForPcmFecc(DWORD imageId);

	virtual void FillAllocStatus(ALLOC_STATUS_PER_PORT_S& allocStatusPerPort);

	virtual BYTE IsVsw()
	{ return false; }

	void UpdateAutoScanOrder(CAutoScanOrder* pAutoScanOrder);
	void UpdateSiteNameInfo(CSiteNameInfo* pSiteNameInfo);
	void RefreshLayout();
	
	void                          UpdateIndicationIcons(BOOL bUseSharedMemForIndicationIcon = FALSE);

	FontTypesEnum GetFontType() const
	{ return m_fontType; }

	virtual void GetPortsOpened(std::map<eLogicalResourceTypes, bool>& isOpenedRsrcMap);

	void UpdateLayoutHandlerType();

	void SetTelepresenceModeChanged(BYTE mode)
	{ m_telepresenceModeChanged = mode; }

	virtual BOOL                  isAllowConfToHiddenAudioIcon();
	virtual BOOL                  isInGatheringMode();
	virtual void                  InitIsGatheringModeEnabled();

protected:

	PDECLAR_MESSAGE_MAP

protected:

	FontTypesEnum GetFontTypeFromConf() const;

protected:

	virtual void SendConnectToRtp();
	virtual void SendOpenEncoder();
	virtual void SendCloseEncoder();
	virtual void SendChangeLayoutToHardware(BOOL bUseSharedMemForChangeLayoutReq = false, BYTE isVSW = false, BOOL* bSent = NULL);

	void RemoveFromLayoutSharedMemory(); //Change Layout Improvement - Layout Shared Memory (CL-SM)
	void RemoveFromIndicationIconSharedMemory(); // Indication Icon change Improvement - Indication Icon Shared Memory (CL-SM)

	virtual void SendChangeLayoutAttributesToHardware(BYTE isFromAudioSpeakerChange = false);
	virtual void SendUpdateCroppingToHardware();
	virtual void SaveAndSendUpdatedVideoParams(CBridgePartyVideoParams* pBridgePartyVideoParams);
	virtual void BuildLayoutAndSendOnEncoderResolutionChange();

	virtual DWORD GetAudioSpeakerPlaceInLayout();

	LayoutType GetConfLayoutType() const;

	virtual BOOL RejectChangeLayoutRequestBecauseOfApplications();
	virtual BOOL RejectPLCRequestBecauseOfApplications(OPCODE requestOpcode);
	virtual void DeleteAllPartyForces(const char* partyName);
	virtual void SetPrivateLayoutForParty(LayoutType layoutType);
	virtual void SetGatheringLayoutForParty(bool bNullImage = true);

	CGatheringManager* GetGatheringManager();
	bool IsNeedPersonalLayoutForGathering();

	virtual void StartSiteNamesOffTimer();
	virtual void SetLayoutHandler();

	// action functions
	virtual void OnVideoBridgePartyConnectIDLE(CSegment* pParam);
	virtual void OnVideoBridgePartyConnectSLIDE(CSegment* pParam);
	virtual void OnVideoBridgePartyConnectSETUP(CSegment* pParam);
	virtual void OnVideoBridgePartyConnectCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgePartyConnectCHANGELAYOUT(CSegment* pParam);
	virtual void OnVideoBridgePartyConnectDISCONNECTING(CSegment* pParam);

	virtual void OnVideoBridgePartyDisConnectIDLE(CSegment* pParam);
	virtual void OnVideoBridgePartyDisConnectSLIDE(CSegment* pParam);
	virtual void OnVideoBridgePartyDisConnectSETUP(CSegment* pParam);
	virtual void OnVideoBridgePartyDisConnectCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgePartyDisConnectCHANGELAYOUT(CSegment* pParam);
	virtual void OnVideoBridgePartyDisConnectDISCONNECTING(CSegment* pParam);
	virtual void OnVideoBridgePartyDisConnect(CSegment* pSeg);

	virtual void OnMplAckIDLE(CSegment* pSeg);
	virtual void OnMplAckSLIDE(CSegment* pParam);
	virtual void OnMplAckSETUP(CSegment* pParam);
	virtual void OnMplAckDISCONNECTING(CSegment* pParam);
	virtual void OnMplAckCHANGELAYOUT(CSegment* pParam);
	virtual void OnMplAckCONNECTED(CSegment* pParam);

	virtual void OnVideoBridgePartyUpdateVideoParamsIDLE(CSegment* pParam);
	virtual void OnVideoBridgePartyUpdateVideoParamsSLIDE(CSegment* pParam);
	virtual void OnVideoBridgePartyUpdateVideoParamsSETUP(CSegment* pParam);
	virtual void OnVideoBridgePartyUpdateVideoParamsCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgePartyUpdateVideoParamsCHANGELAYOUT(CSegment* pParam);
	virtual void OnVideoBridgePartyUpdateVideoParamsDISCONNECTING(CSegment* pParam);

	virtual void OnVideoBridgePartyFastUpdateSLIDE(CSegment* pParam);
	virtual void OnVideoBridgePartyFastUpdateSETUP(CSegment* pParam);
	virtual void OnVideoBridgePartyFastUpdateCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgePartyFastUpdateCHANGELAYOUT(CSegment* pParam);
	virtual void OnVideoBridgePartyFastUpdate(CSegment* pParam);

	virtual void OnVideoBridgePartyAddImageCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgePartyDelImageCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgePartyMuteImageCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgePartyUnMuteImageCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgePartyUpdateImageCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgePartyUpdateImageCHANGELAYOUT(CSegment* pParam);

	virtual void OnVideoBridgePartyNotifyCHANGELAYOUT(CSegment* pParam);

	virtual void OnVideoBridgePartyChangeAudioSpeakerCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgePartyChangeAudioSpeakerCHANGELAYOUT(CSegment* pParam);

	virtual void OnVideoBridgePartyChangeSpeakersCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgePartyChangeSpeakersCHANGELAYOUT(CSegment* pParam);

	virtual void OnVideoBridgePartyEncoderResolutionChangedCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgePartyEncoderResolutionChangedCHANGELAYOUT(CSegment* pParam);

	virtual void OnVideoBridgePartyChangeConfLayoutCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgePartyChangeConfLayoutCHANGELAYOUT(CSegment* pParam);

	virtual void OnVideoBridgeChangeLayoutOfTPRoomSublinkCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgeChangeLayoutOfTPRoomSublinkCHANGELAYOUT(CSegment* pParam);

	virtual void OnVideoBridgePartyChangePartyLayoutCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgePartyChangePartyLayoutCHANGELAYOUT(CSegment* pParam);

	virtual void OnVideoBridgePartyChangePrivateLayoutSLIDE(CSegment* pParam);
	virtual void OnVideoBridgePartyChangePrivateLayoutSETUP(CSegment* pParam);
	virtual void OnVideoBridgePartyChangePrivateLayoutCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgePartyChangePrivateLayoutCHANGELAYOUT(CSegment* pParam);

	virtual void OnVideoBridgePartySetPrivateLayoutOnOffCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgePartySetPrivateLayoutOnOffCHANGELAYOUT(CSegment* pParam);

	virtual void OnVideoBridgePartyPLC_SetPrivateLayoutTypeCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgePartyPLC_SetPrivateLayoutTypeCHANGELAYOUT(CSegment* pParam);
	virtual void OnVideoBridgePartyPLC_SetPrivateLayoutType(CSegment* pParam);

	virtual void OnVideoBridgePartyVENUS_SetPrivateLayoutTypeCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgePartyVENUS_SetPrivateLayoutTypeCHANGELAYOUT(CSegment* pParam);
	virtual void OnVideoBridgePartyVENUS_SetPrivateLayoutType(CSegment* pParam);

	virtual void OnVideoBridgePartyPLC_ReturnToConfLayoutCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgePartyPLC_ReturnToConfLayoutCHANGELAYOUT(CSegment* pParam);
	virtual void OnVideoBridgePartyPLC_ReturnToConfLayout(CSegment* pParam);

	virtual void OnVideoBridgePartyPLC_ForceCellCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgePartyPLC_ForceCellCHANGELAYOUT(CSegment* pParam);
	virtual void OnVideoBridgePartyPLC_ForceCell(CSegment* pParam);

	virtual void OnVideoBridgePartyPLC_CancelAllPrivateLayoutForcesCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgePartyPLC_CancelAllPrivateLayoutForcesCHANGELAYOUT(CSegment* pParam);
	virtual void OnVideoBridgePartyPLC_CancelAllPrivateLayoutForces(CSegment* pParam);

	virtual void OnVideoBridgePartyChangeVisualEffectsIDLE(CSegment* pParam);
	virtual void OnVideoBridgePartyChangeVisualEffectsSLIDE(CSegment* pParam);
	virtual void OnVideoBridgePartyChangeVisualEffectsSETUP(CSegment* pParam);
	virtual void OnVideoBridgePartyChangeVisualEffectsCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgePartyChangeVisualEffectsCHANGELAYOUT(CSegment* pParam);

	virtual void OnVideoBridgePartyChangeSiteNameIDLE(CSegment* pParam);
	virtual void OnVideoBridgePartyChangeSiteNameSLIDE(CSegment* pParam);
	virtual void OnVideoBridgePartyChangeSiteNameSETUP(CSegment* pParam);
	virtual void OnVideoBridgePartyChangeSiteNameCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgePartyChangeSiteNameCHANGELAYOUT(CSegment* pParam);

	virtual void OnVideoBridgePartyRefreshLayoutIDLE(CSegment* pParam);
	virtual void OnVideoBridgePartyRefreshLayoutSLIDE(CSegment* pParam);
	virtual void OnVideoBridgePartyRefreshLayoutSETUP(CSegment* pParam);
	virtual void OnVideoBridgePartyRefreshLayoutCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgePartyRefreshLayoutCHANGELAYOUT(CSegment* pParam);

	virtual void OnVideoBridgePartyShowSlideIDLE(CSegment* pParam);
	virtual void OnVideoBridgePartyShowSlideSLIDE(CSegment* pParam);
	void  OnVideoBridgePartyShowSlide(CSegment* pParam);

	virtual void OnVideoBridgePartyStopShowSlideIDLE(CSegment* pParam);
	virtual void OnVideoBridgePartyStopShowSlideSLIDE(CSegment* pParam);
	void OnVideoBridgePartyStopShowSlide(CSegment* pParam);

	virtual void OnVideoBridgePartyStartPLC(CSegment* pParam);
	virtual void OnVideoBridgePartyStopPLC(CSegment* pParam);

	virtual void OnVideoBridgePartyUpdateLectureModeRoleCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgePartyUpdateLectureModeRoleIDLE(CSegment* pParam);
	virtual void OnVideoBridgePartyUpdateLectureModeRoleSETUP(CSegment* pParam);
	virtual void OnVideoBridgePartyUpdateLectureModeRoleSLIDE(CSegment* pParam);
	virtual void OnVideoBridgePartyUpdateLectureModeRoleCHANGELAYOUT(CSegment* pParam);
	virtual void OnVideoBridgePartyUpdateLectureModeRole(CSegment* pParam);

	virtual void OnVideoBridgePartyDeletePartyFromConfCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgePartyDeletePartyFromConfCHANGELAYOUT(CSegment* pParam);
	virtual void OnVideoBridgePartyDeletePartyFromConf(CSegment* pParam);

	virtual void OnVideoBridgeUpdateDecoderDetectedModeSETUP(CSegment* pParam);
	virtual void OnVideoBridgeUpdateDecoderDetectedModeCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgeUpdateDecoderDetectedModeCHANGELAYOUT(CSegment* pParam);

	virtual void OnVideoBridgeDisplayTextOnScreenCONNECT(CSegment* pParam);
	virtual void OnVideoBridgeDisplayTextOnScreenCHANGE_LAYOUT(CSegment* pParam);
	virtual void OnVideoBridgeDisplayTextOnScreen(CSegment* pParam);
	virtual void StartTextOnScreenTimer(DWORD timeout);
	virtual void OnTextDisplayToutCONNECTED(CSegment* pParam);
	virtual void OnTextDisplayToutCHANGE_LAYOUT(CSegment* pParam);
	virtual void OnTextDisplayTout(CSegment* pParam);
	virtual void SendStopTextDisplay(CSegment* pParam);

	virtual void OnMplOpenPortAck(STATUS status);
	virtual void OnMplConnectAck(STATUS status);
	virtual void OnMplChangeLayoutAck(STATUS status);
	virtual void OnMplClosePortAck(STATUS status);

	virtual void OnTimerVideoOutSetupSETUP(CSegment* pParam);
	virtual void OnTimerVideoOutDisconnectionDISCONNECTING(CSegment* pParam);
	virtual void OnTimerChangeLayoutCHANGELAYOUT(CSegment* pParam);

	virtual void OnSiteNameToutCONNECTED(CSegment* pParam);
	virtual void OnSiteNameToutCHANGE_LAYOUT(CSegment* pParam);
	virtual void OnSiteNameTout(CSegment* pParam);
	CTaskApp* GetLastActiveAudioSpeakerRequestFromBridge();

	virtual void OnTimerSlideIntraSLIDE(CSegment* pParam);

	virtual void OnVideoBridgePartyUpdateVideoClarityIDLE(CSegment* pParam);
	virtual void OnVideoBridgePartyUpdateVideoClaritySLIDE(CSegment* pParam);
	virtual void OnVideoBridgePartyUpdateVideoClaritySETUP(CSegment* pParam);
	virtual void OnVideoBridgePartyUpdateVideoClarityCHANGE_LAYOUT(CSegment* pParam);
	virtual void OnVideoBridgePartyUpdateVideoClarityCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgePartyUpdateVideoClarityDISCONNECTING(CSegment* pParam);
	void SaveAndSendUpdateVideoClarity(CSegment* pParam);

	virtual void OnVideoBridgeChangeSpeakerNotationForPcmFeccIDLE(CSegment* pParam);
	virtual void OnVideoBridgeChangeSpeakerNotationForPcmFeccSETUP(CSegment* pParam);
	virtual void OnVideoBridgeChangeSpeakerNotationForPcmFeccSLIDE(CSegment* pParam);
	virtual void OnVideoBridgeChangeSpeakerNotationForPcmFeccCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgeChangeSpeakerNotationForPcmFeccCHANGE_LAYOUT(CSegment* pParam);
	virtual void OnVideoBridgeChangeSpeakerNotationForPcmFeccDISCONNECTING(CSegment* pParam);

	virtual void OnVideoBridgeUpdatePartyTelePresenceModeIDLE(CSegment* pParam);
	virtual void OnVideoBridgeUpdatePartyTelePresenceModeSLIDE(CSegment* pParam);
	virtual void OnVideoBridgeUpdatePartyTelePresenceModeSETUP(CSegment* pParam);
	virtual void OnVideoBridgeUpdatePartyTelePresenceModeCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgeUpdatePartyTelePresenceModeCHANGE_LAYOUT(CSegment* pParam);
	virtual void OnVideoBridgeUpdatePartyTelePresenceModeDISCONNECTING(CSegment* pParam);
	virtual void OnVideoBridgeUpdatePartyTelePresenceMode(CSegment* pParam);
	void SaveAndSendUpdatePartyTelePresenceMode(CSegment* pParam);

	virtual void OnVideoBridgeUpdateMessageOverlayCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgeUpdateMessageOverlayCHANGE_LAYOUT(CSegment* pParam);

	virtual void OnTimerEncoderUpdateCONNECTED(CSegment* pParam);
	virtual void OnTimerEncoderUpdateCHANGE_LAYOUT(CSegment* pParam);
	virtual void OnTimerRecurrentIntraRequest(CSegment* pParam);


	void CreatePartyPcmFeccVisualEffects();

	virtual void OnAutoScanTimerCONNECTED(CSegment* pParam);
	virtual void OnAutoScanTimerCHANGE_LAYOUT(CSegment* pParam);

	virtual void OnConfSetAutoScanOrderSETUP(CSegment* pParam);
	virtual void OnConfSetAutoScanOrderSLIDE(CSegment* pParam);
	virtual void OnConfSetAutoScanOrderCONNECTED(CSegment* pParam);
	virtual void OnConfSetAutoScanOrderCHANGE_LAYOUT(CSegment* pParam);
	virtual void OnConfSetAutoScanOrder(CSegment* pParam);

	virtual void OnVideoBridgePartyIndicationIconsChangeCONNECTED(CSegment* pParam);
	virtual void                  OnTimerAudioNumberHiden(CSegment* pParam);
	 

	virtual void OnVideoBridgePartyUpdateOnImageSvcToAvcCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgePartyUpdateOnImageSvcToAvcCHANGELAYOUT(CSegment* pParam);

	virtual void OnVideoBridgePartyUpdateLayoutHandlerTypeSETUP(CSegment* pParam);
	virtual void OnVideoBridgePartyUpdateLayoutHandlerTypeIDLE(CSegment* pParam);
	virtual void OnVideoBridgePartyUpdateLayoutHandlerTypeSLIDE(CSegment* pParam);
	virtual void OnVideoBridgePartyUpdateLayoutHandlerTypeCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgePartyUpdateLayoutHandlerTypeCHANGELAYOUT(CSegment* pParam);
	virtual void OnVideoBridgePartyUpdateLayoutHandlerType(CSegment* pParam);

private:

	bool InitH264SvcVideoParams(H264_SVC_VIDEO_PARAM_S& h264, const MsSvcParamsStruct& svc) const;
	void FillEncoderParamsForMsSvc(ENCODER_PARAM_S& encoder) const;
	void SiteNameSerialization(CSegment* pParam);
	void ModifyVisuallEffectsForAVMCUParty(CVisualEffectsParams* pPartyVisualEffects);

	void SendIndicationIconsChangeToHardware(BOOL bUseSharedMemForIndicationIcon = FALSE);

	DWORD SendUpdateEncoder(bool open = false)/* const */;

protected:

	CLayoutHandler* m_pCurrHandler;
	CLayout* m_pCurrentView;

	DWORD m_decoderDetectedModeWidth;
	DWORD m_decoderDetectedModeHeight;
	DWORD m_decoderDetectedSampleAspectRatioWidth;
	DWORD m_decoderDetectedSampleAspectRatioHeight;

	CLayout* m_pReservation[CP_NO_LAYOUT]; // For forces in party level made on the conference layout
	CLayout* m_pPrivateReservation[CP_NO_LAYOUT]; // For forces in Private Layout
	LayoutType m_PrivatelayoutType;

	bool m_isPrivate;
	bool m_isPrivateChanged;

	CVisualEffectsParams* m_pPartyVisualEffects;
	CVisualEffectsParams* m_pPartyPcmFeccVisualEffects;

	ePartyLectureModeRole m_partyLectureModeRole; // lect/listen/cold listen

	BYTE m_layoutChangedWhileWaitingForAck; // change layout received while old change layout has not received ack yet
	BYTE m_visualEffectsChangedWhileWaitingForAck; // change visual effects/ Speaker received while old change layout has not received ack yet
	BYTE m_resolutionChangedWhileWaitingForAck; // change resolution received while old change layout has not received ack yet
	BYTE m_imageResolutionChangedWhileWaitingForAck; // image resolution has been changed while old change layout has not received ack yet
	CTaskApp* m_pLastSpeakerNotationParty; // Save the last speakerNotation party to avoid redundant msgs to VideoCard
	WORD m_isForce1x1; // Yes/No Default value for cascade link party is YES means layout 1X1 for the link.
	eTelePresencePartyType m_eTelePresenceMode;
	eVideoQuality m_videoQuality;
	BYTE m_isSiteNamesEnabled;
	BYTE m_sitenameChangedWhileWaitingForAck;

	bool m_bEnableReStopGathering;
	DWORD m_tmpSpeakerNotationForPcmFeccImageId;
	BYTE m_waitForUpdateEncoderAck;
	bool m_isLprActive;
	BYTE m_bUseIntermediateSDResolution;
	CSiteNameInfo* m_pSiteNameInfo;
	DWORD m_dwFrThreshold;

	FontTypesEnum m_fontType;
	DWORD m_nTimerFastUpdateReq;

	BYTE m_telepresenceModeChanged; // telepresence is turned off
	BYTE m_layoutTPRoomSublinkChangedWhileWaitingForAck;
	
	EIconType                     m_recordingType;
	WORD                          m_isAudioParticipantsIconActived;
	WORD                          m_numAudioParticipantsInConf;

	//after video out is opened, we need to display the audio icon when build layout. 
	//Meanwhile, Conf may request each party to hide the audio icon when conf duration timeout, 
	//Use this flag to indcation whether the conf could hide the new party's audio icon
	BOOL                          m_isAudioIconToSentAfterOpenPort;   
	BOOL                          m_isInGatheringMode;

	BYTE                          m_bIsFollowSpeakerOn1X1;
};

////////////////////////////////////////////////////////////////////////////
class CBridgePartyVideoOutVSW : public CBridgePartyVideoOut
{
	CLASS_TYPE_1(CBridgePartyVideoOutVSW, CBridgePartyVideoOut)

	virtual ~CBridgePartyVideoOutVSW();

	virtual const char* NameOf() const
	{ return GetCompileType(); }

public:

	CBridgePartyVideoOutVSW();

	void ResetImage0(DWORD partyRscId);
	void ResetRsrvImage0(DWORD partyRscId);
	void SetPartyForce(CVideoLayout& pPartyLayout);

	virtual BYTE IsVsw()
	{ return true; }

protected:

	virtual BYTE BuildLayout(BYTE alwaysSendToHardware = FALSE);
	virtual void SendOpenEncoder();

	virtual void OnMplOpenPortAck(STATUS status);
	virtual void OnMplChangeLayoutAck(STATUS status);

	void OnVideoBridgePartyDisConnectCHANGELAYOUT(CSegment* pParam);
	void OnVideoBridgePartyChangeConfLayoutCONNECTED(CSegment* pParam);
	void OnVideoBridgePartyChangeConfLayoutCHANGELAYOUT(CSegment* pParam);
	void OnMplEncoderSyncIndCONNECTED(CSegment* pParam);
	void OnMplEncoderSyncIndCHANGELAYOUT(CSegment* pParam);
	virtual void OnTimerSlideIntraSLIDE(CSegment* pParam);

	// site names timer block
	virtual void StartSiteNamesOffTimer();
	virtual void OnSiteNameTout(CSegment* pParam);

	// PLC block
	virtual BOOL RejectPLCRequestBecauseOfApplications(OPCODE requestOpcode);

	// move
	virtual void UpdateNewConfParams(ConfRsrcID confRsrcId, const CBridgePartyMediaParams* pBridgePartyVideoOutParams);
	virtual void SetLayoutHandler();

	PDECLAR_MESSAGE_MAP
};

////////////////////////////////////////////////////////////////////////////
class CBridgePartyVideoOutLegacy : public CBridgePartyVideoOut
{
	CLASS_TYPE_1(CBridgePartyVideoOutLegacy, CBridgePartyVideoOut)

	virtual ~CBridgePartyVideoOutLegacy();

	virtual const char* NameOf() const
	{ return GetCompileType(); }

public:

	CBridgePartyVideoOutLegacy();

	virtual void AddContentImage();
	virtual void DelContentImage();

	virtual void DisplayGathering(CGathering* pGathering);
	virtual void OnVideoBridgePartyDisConnect(CSegment* pSeg);

	void SaveLastVideoOnlyLayoutAndSetDefaultLayout();
	void RestoreLastVideoOnlyLayout();

	virtual LayoutType GetPartyCurrentLayoutType() const;

	BOOL IsContentImageNeedToBeAdded() const;BOOL IsOriginalLayoutSaved() const;
	void SetIsOriginalLayoutSaved(BOOL yesNo);
	virtual BOOL RejectChangeLayoutRequestBecauseOfApplications();

protected:

	virtual void OnVideoBridgePartyAddContentImageCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgePartyAddContentImageCHANGELAYOUT(CSegment* pParam);
	virtual void OnVideoBridgePartyDelContentImageCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgePartyDelContentImageCHANGELAYOUT(CSegment* pParam);
	virtual void OnMplOpenPortAck(STATUS status);
	virtual void SetLayoutHandler();
	virtual void HideGatheringText();
	virtual void ShowGatheringText();

	void SetLastVideoOnlyLayout(const LayoutType lastVideoOnlyLayout)
	{ m_LastVideoOnlyLayout = lastVideoOnlyLayout; }

	void SetIsLastVideoLayoutPrivate(const BYTE isLastVideoLayoutPrivate)
	{ m_IsLastVideoLayoutPrivate = isLastVideoLayoutPrivate; }

	LayoutType GetLastVideoOnlyLayout()
	{ return m_LastVideoOnlyLayout; }

	BYTE IsLastVideoLayoutPrivate()
	{ return m_IsLastVideoLayoutPrivate; }

protected:

	LayoutType m_LastVideoOnlyLayout;
	BYTE m_IsLastVideoLayoutPrivate;
	BOOL m_IsOriginalLayoutSaved;

	PDECLAR_MESSAGE_MAP
};

////////////////////////////////////////////////////////////////////////////
class CBridgePartyVideoOutCOP : public CBridgePartyVideoOut
{
	CLASS_TYPE_1(CBridgePartyVideoOutCOP, CBridgePartyVideoOut)

	virtual ~CBridgePartyVideoOutCOP();

	virtual const char* NameOf() const
	{ return GetCompileType(); }

public:

	enum STATE
	{
		SETUP = (IDLE + 1), SLIDE, CONNECTED, /*not relevant*/CHANGE_LAYOUT, DISCONNECTING, UPDATE_ENCODER, CONNECTING_PCM, PCM, DISCONNECTING_PCM
	};

	CBridgePartyVideoOutCOP();
	virtual void Create(
		const CBridgePartyCntl* pBridgePartyCntl,
		const CRsrcParams* pRsrcParams,
		const CBridgePartyMediaParams* pBridgePartyMediaParams);

	virtual BOOL IsConnected();

	virtual WORD IsValidState(WORD state) const;

	DWORD GetCopEncoderId()
	{ return m_copEncoderPartyId; }

	DWORD GetCopEncoderConnectionId()
	{ return m_copEncoderConnectionId; }

	WORD GetCopEncoderIndex()
	{ return m_copEncoderIndex; }

	WORD GetCopEncoderIndexOfCascadeLinkLecturer()
	{ return m_copResourceIndexOfCascadeLinkLecturer; }

	BYTE GetIsCopCascadeLecturer()
	{ return m_bCascadeIsLecturer; }

	void ConnectToPCMEncoder(DWORD pcmEncoderConnectionId, DWORD pcmEncoderPartyId);
	void DisconnectFromPCMEncoder();

	void OnMplAckCONNECTING_PCM(CSegment* pParam);
	void OnMplAckDISCONNECTING_PCM(CSegment* pParam);
	void OnPCMConnectAck(STATUS status);
	void OnPCMDisconnectAck(STATUS status);
	void SendAckIndToPCM(OPCODE AckOpcode, DWORD ack_seq_num, STATUS status);BOOL IsConnectedOrConnectingPCM();

	BYTE IsInUpdateEncoder() const;

protected:

	virtual void SendConnectToRtp();
	virtual void SendDisconnectFromRtp();

	virtual void SaveAndSendUpdatedVideoParams(CBridgePartyVideoParams* pBridgePartyVideoParams);

	virtual void OnVideoBridgePartyDisConnectSETUP(CSegment* pParam);
	virtual void OnVideoBridgePartyDisConnectUPDATE_ENCODER(CSegment* pParam);
	virtual void OnVideoBridgePartyDisConnectCONNECTING_PCM(CSegment* pParam);
	virtual void OnVideoBridgePartyDisConnectPCM(CSegment* pParam);
	virtual void OnVideoBridgePartyDisConnectDISCONNECTING_PCM(CSegment* pParam);
	virtual void OnVideoBridgePartyDisConnect(CSegment* pSeg);
	virtual void OnMplAckSETUP(CSegment* pParam);
	virtual void OnMplAckDISCONNECTING(CSegment* pParam);
	virtual void OnMplAckUPDATE_ENCODER(CSegment* pParam);

	virtual void OnVideoBridgePartyUpdateVideoParamsIDLE(CSegment* pParam);
	virtual void OnVideoBridgePartyUpdateVideoParamsSLIDE(CSegment* pParam);
	virtual void OnVideoBridgePartyUpdateVideoParamsSETUP(CSegment* pParam);
	virtual void OnVideoBridgePartyUpdateVideoParamsCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgePartyUpdateVideoParamsUPDATE_ENCODER(CSegment* pParam);
	virtual void OnVideoBridgePartyUpdateVideoParamsCONNECTING_PCM(CSegment* pParam);
	virtual void OnVideoBridgePartyUpdateVideoParamsPCM(CSegment* pParam);
	virtual void OnVideoBridgePartyUpdateVideoParamsDISCONNECTING_PCM(CSegment* pParam);
	virtual void OnVideoBridgePartyUpdateVideoParams(CSegment* pParam);

	virtual void OnMplOpenPortAck(STATUS status);
	virtual void OnMplConnectAckSETUP(STATUS status);
	virtual void OnMplConnectAckUPDATE_ENCODER(STATUS status);

	virtual void OnMplDisconnectAckDISCONNECTING(STATUS status);
	virtual void OnMplDisconnectAckUPDATE_ENCODER(STATUS status);

	virtual void OnVideoBridgeConnectPartyToPcmIDLE(CSegment* pParam);
	virtual void OnVideoBridgeConnectPartyToPcmSLIDE(CSegment* pParam);
	virtual void OnVideoBridgeConnectPartyToPcmSETUP(CSegment* pParam);
	virtual void OnVideoBridgeConnectPartyToPcmCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgeConnectPartyToPcmDISCONNECTING(CSegment* pParam);
	virtual void OnVideoBridgeConnectPartyToPcmUPDATE_ENCODER(CSegment* pParam);
	virtual void OnVideoBridgeConnectPartyToPcmCONNECTING_PCM(CSegment* pParam);
	virtual void OnVideoBridgeConnectPartyToPcmPCM(CSegment* pParam);
	virtual void OnVideoBridgeConnectPartyToPcmDISCONNECTING_PCM(CSegment* pParam);

	void OnVideoBridgeConnectPartyToPcm(CSegment* pParam);

	virtual void OnVideoBridgeDisconnectPartyFromPcmCONNECTING_PCM(CSegment* pParam);
	void OnVideoBridgeDisconnectPartyFromPcm(CSegment* pParam);

	void ConnectRtpToPCMEncoder();
	void DisconnectRtpFromPCMEncoder();

protected:

	WORD m_copEncoderIndex;
	DWORD m_copEncoderConnectionId;
	DWORD m_copEncoderPartyId;
	DWORD m_pcmEncoderConnectionId;
	DWORD m_pcmEncoderEntityId;
	BYTE m_needToConnectToPcmEncoder;
	BYTE m_needToDisconnectFromPcmEncoder;
	BYTE m_disconnectAckReceived;
	WORD m_copResourceIndexOfCascadeLinkLecturer;
	BYTE m_bCascadeIsLecturer;

	PDECLAR_MESSAGE_MAP
};

///////////////////////////////////////////////////////////////////////////
class CBridgePartyVideoOutXcode : public CBridgePartyVideoOutCOP
{
	CLASS_TYPE_1(CBridgePartyVideoOutXcode, CBridgePartyVideoOutCOP)

	virtual ~CBridgePartyVideoOutXcode();

	virtual const char* NameOf() const
	{ return GetCompileType(); }

public:

	CBridgePartyVideoOutXcode();

	virtual void Create(
		const CBridgePartyCntl* pBridgePartyCntl,
		const CRsrcParams* pRsrcParams,
		const CBridgePartyMediaParams* pBridgePartyMediaParams);

	virtual BOOL IsConnected();

	virtual WORD IsValidState(WORD state) const;

	void ConnectToPCMEncoder(DWORD pcmEncoderConnectionId, DWORD pcmEncoderPartyId);
	void DisconnectFromPCMEncoder();

	void OnPCMConnectAck(STATUS status);
	void OnPCMDisconnectAck(STATUS status);
	BYTE IsInUpdateEncoder() const;

	DWORD GetXCodeEncoderId()
	{ return m_xcodeEncoderPartyId; }

	DWORD GetXCodeEncoderConnectionId()
	{ return m_xcodeEncoderConnectionId; }

	eXcodeRsrcType GetXCodeEncoderIndex()
	{ return m_xcodeEncoderIndex; }

protected:

	virtual void SendConnectToRtp();
	virtual void SendDisconnectFromRtp();

	virtual void OnVideoBridgePartyDisConnectSETUP(CSegment* pParam);
	virtual void OnVideoBridgePartyDisConnect(CSegment* pSeg);

	virtual void OnMplAckSETUP(CSegment* pParam);
	virtual void OnMplAckDISCONNECTING(CSegment* pParam);

	virtual void OnMplOpenPortAck(STATUS status);
	virtual void OnMplConnectAckSETUP(STATUS status);

	virtual void OnMplDisconnectAckDISCONNECTING(STATUS status);

protected:

	eXcodeRsrcType m_xcodeEncoderIndex;
	DWORD m_xcodeEncoderConnectionId;
	DWORD m_xcodeEncoderPartyId;

	PDECLAR_MESSAGE_MAP
};

///////////////////////////////////////////////////////////////////////////
#endif // BRIDGE_PARTY_VIDEO_OUT_H__
