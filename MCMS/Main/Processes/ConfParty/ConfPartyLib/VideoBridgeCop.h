// +========================================================================+
// VideoBridgeCop.h                                                         |
// Copyright 1995 Pictel Technologies Ltd.                                  |
// All Rights Reserved.                                                     |
// -------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary      |
// information of Pictel Technologies Ltd. and is protected by law.         |
// It may not be copied or distributed in any form or medium, disclosed     |
// to third parties, reverse engineered or used in any manner without       |
// prior written authorization from Pictel Technologies Ltd.                |
// -------------------------------------------------------------------------|
// FILE:       VideoBridgeCop.h                                             |
// SUBSYSTEM:  MCMS                                                         |
// PROGRAMMER:                                                              |
// -------------------------------------------------------------------------|
// Who  | Date    | Description                                             |
// +========================================================================+

#ifndef VIDEOBRIDGECOP_H_
#define VIDEOBRIDGECOP_H_

#include "VideoBridge.h"
#include "VideoHardwareInterface.h"
#include "VideoBridgeCopEncoder.h"
#include "VideoBridgeCopDecoder.h"
#include "COP_ConfParty_Defs.h"
#include "ChangeLayoutActions.h"
#include "COP_Layout_definitions.h"
#include "IntraSuppression.h"

class CChangeLayoutActions;

typedef struct
{
  DWORD artPartyId;
  EMediaDirection disconnectDirection;
} sDisconnectingParty;

typedef std::vector<sDisconnectingParty> DISCONNECTING_VECTOR;

#define CONNECT_COP_VB_TOUT                               210
#define SMART_SWITCH_TOUT                                 211
#define SWITCH_TIME_OUT_COP_VALUE                         3*SECOND
#define CONNECT_COP_VB_TIME_OUT_VALUE                     10*SECOND
#define COP_BRIDGE_DISCONNECT_TIME_OUT_VALUE              5*SECOND

#define COP_LECTURE_MODE_ACTION_TIME_OUT                  213
#define COP_LECTURE_MODE_ACTION_TIME_VALUE                5*SECOND

#define COP_DSP_SMART_SWITCH_TIME_OUT                     214
#define COP_DSP_SMART_SWITCH_TIME_VALUE                   15

#define DELAY_DECODER_SYNC_TIMER                          215
#define DELAY_DECODER_SYNC_TOUT                           20 // 200 ms

#define DISABLE_INTRA_SUPPRESS_AFTER_STOP_CONTENT_TIMER   216
#define DISABLE_INTRA_SUPPRESS_AFTER_START_CONTENT_TIMER  217
#define DISABLE_INTRA_SUPPRESS_AFTER_STOP_CONTENT_TOUT    12*SECOND

/*#define LAYOUT_NOT_CHANGED                                0
#define LAYOUT_CHANGED_ATTRIBUTES                         1
#define LAYOUT_CHANGED                                    2
#define LAYOUT_CHANGED_AUTOSCAN                           3
*/
#define PCM_RESOURCE_INDEX                                NUM_OF_LEVEL_ENCODERS

#define CONNECT_PCM_ENCODER_TIMER                         205
#define LECTURER_DECODER_TIMER                            207


////////////////////////////////////////////////////////////////////////////
//                        CVideoBridgeCOP
////////////////////////////////////////////////////////////////////////////
class CVideoBridgeCOP : public CVideoBridge
{
  CLASS_TYPE_1(CVideoBridgeCOP, CVideoBridge)

public:
                           CVideoBridgeCOP();
  virtual                 ~CVideoBridgeCOP();

  virtual void             Create(const CVideoBridgeInitParams* pVideoBridgeInitParams);
  virtual void             InitBridgeParams(const CBridgePartyInitParams* pBridgePartyInitParams);

  void                     OpenConstEncodersAndDecoders();
  void                     ConnectPCMEncoderToPCMMenu();
  void                     DisconnectPCMEncoderFromPCMMenu();
  void                     EndSetup();
  void                     EndDisconnect();

  // object & state machine virtual functions
  virtual const char*      NameOf() const;
  virtual void*            GetMessageMap();

  virtual CLayout*         GetReservationLayout(void) const;

  virtual WORD             IsValidEvent(OPCODE event) const;
  void                     OnTempTimer(CSegment* pParam);
  virtual void             OnConfTerminateDISCONNECTING(CSegment* pParam);
  void                     OnTimerDisconnetDISCONNECTING(CSegment* pParam);
  virtual void             OnConfDisConnectConfINSWITCH(CSegment* pParam);
  virtual void             OnConfDisConnectConfIDLE(CSegment* pParam);

  virtual void             OnTimerEndSwitchINSWITCH(CSegment* pParam);

  // Same as in CP
  virtual void             StartAutoLayout();
  void                     OnConfUpdateAutoLayoutIDLE(CSegment* pParam);
  void                     OnConfUpdateAutoLayoutCONNECTED(CSegment* pParam);
  void                     OnConfUpdateAutoLayoutINSWITCH(CSegment* pParam);
  void                     OnConfUpdateAutoLayoutDISCONNECTING(CSegment* pParam);
  void                     OnConfUpdateAutoLayout(CSegment* pParam);

  void                     ChangeConfLayoutType(const LayoutType newLayoutType);
  virtual void             UpdateVideoInParams(CTaskApp* pParty, CBridgePartyVideoParams* pBridgePartyVideoParams);
  virtual void             UpdateVideoOutParams(CTaskApp* pParty, CBridgePartyVideoParams* pBridgePartyVideoParams);
  void                     OnEndCOPEncoderConnectAnycase(CSegment* pParam);

  // Api from PCM
  virtual void             ConnectPartyToPCMEncoder(PartyRsrcID partyId);
  virtual void             DisconnectPartyFromPCMEncoder(PartyRsrcID partyId);
  virtual DWORD            GetPartyResolutionInPCMTerms(PartyRsrcID partyId);

  void                     OnPartyConnectedToPCMEncoder(CSegment* pParam);
  void                     OnPartyDisconnectedFromPCMEncoder(CSegment* pParam);
  void                     OnPCMEncoderParamsUpdated(CSegment* pParam);

  void                     UpdatePCMEncoderWithNewLevelParamsIfNeeded(CVideoBridgePartyCntlCOP* pPartyCntl);
  virtual CLayout*         GetPartyLayout(const char* partyName);
  virtual CLayout*         GetConfLayout() { return m_ConfLayout; }
  virtual void             ChangeSpeakerNotation(PartyRsrcID partyId, DWORD imageId);

  virtual void             GetCroppingValues(DWORD& croppingHorr, DWORD& croppingVer);
  virtual void             EventModeIntraPreviewReq(PartyRsrcID partyId);

  virtual void             UpdateEMPartyStartCascadeLinkAsLecturerPendingMode(CTaskApp* pParty, WORD cascadeLecturerCopEncoderIndex);

  virtual void             SendMessageOverlayToNewConnectedPartyIfNeeded(CSegment* pParam);
  virtual void             OnDisplayIndicationIconCONNECTED(CSegment* pParam);

protected:

  // Implementing Create
  DWORD                    ValidateInitParams(const CVideoBridgeInitParams* pVideoBridgeInitParams);
  void                     InitApplicationParams(const CVideoBridgeInitParams* pVideoBridgeInitParams);
  void                     InitLayoutsReservation(const CVideoBridgeInitParams* pVideoBridgeInitParams);
  void                     CreateLevelEncoders(const CVideoBridgeInitParams* pVideoBridgeCOPInitParams);
  void                     CreatePcmEncoder(const CVideoBridgeInitParams* pVideoBridgeCOPInitParams);
  void                     ConnectLevelEncodes();
  WORD                     UpdateEncoderVideoParams(CBridgePartyVideoOutParams* pOutVideoParams, CCommConf* pCommConf /*const CVideoBridgeInitParams* pVideoBridgeCOPInitParams*/, WORD encoder_index) const;
  void                     UpdateEncoderResourceParams(CBridgePartyVideoOutParams& rOutVideoParams, PartyRsrcID& partyRsrcID, DWORD encoder_index, eVideoConfType videoConfType, CopRsrcDesc encoderCopRsrcDesc) const;
  WORD                     CheckAllLevelEncodersConnected() const;
  void                     PrintVideoBridgeInitParams(const CVideoBridgeInitParams* pVideoBridgeInitParams);
  void                     SetVideoConfTypeFromFirstEncoder(const CVideoBridgeInitParams* pVideoBridgeCOPInitParams);
///  ECopEncoderMaxResolution GetCopEncoderMaxResolution() const;

  void                     UpdatePartyCopEncoderParams(CBridgePartyInitParams& partyInitParams);
  WORD                     GetCopEncoderIndexFromEntityId(DWORD levelEncoderEntityId) const;

  // buildlayout
  void                     ChangeLayout(LayoutType newLayoutType, BOOL isChangeLayoutBecauseOfRemoveParty = FALSE, BYTE always_send = NO);
  DWORD                    GetAudioSpeakerPlaceInLayout();
  BOOL                     IsSmartSwitchNeeded(CLayout* pNewLayout, CLayout* pOldLayout, WORD& closeDisconnectDecoderIndex);

  // events from cop encoders
  void                     OnLevelEncoderEndConnectIDLE(CSegment* pParam);
  void                     OnLevelEncoderEndConnectCONNECTED(CSegment* pParam);
  void                     OnLevelEncoderEndConnectINSWITCH(CSegment* pParam);
  void                     OnLevelEncoderEndConnectDISCONNECTING(CSegment* pParam);

  void                     OnLevelEncoderVideoOutUpdatedCONNECTED(CSegment* pParam);
  void                     OnLevelEncoderVideoOutUpdatedINSWITCH(CSegment* pParam);
  void                     OnLevelEncoderVideoOutUpdated(CSegment* pParam);

  // events from cop decoders
  void                     OnCopDecoderEndCloseCONNECTED(CSegment* pParam);
  void                     OnCopDecoderEndCloseINSWITCH(CSegment* pParam);
  void                     OnCopDecoderEndCloseDISCONNECTING(CSegment* pParam);

  void                     OnCopDecoderEndDisconnectCONNECTED(CSegment* pParam);
  void                     OnCopDecoderEndDisconnectINSWITCH(CSegment* pParam);
  void                     OnCopDecoderEndDisconnectDISCONNECTING(CSegment* pParam);

  void                     OnCopDecoderEndConnectCONNECTED(CSegment* pParam);
  void                     OnCopDecoderEndConnectINSWITCH(CSegment* pParam);
  void                     OnCopDecoderEndConnectDISCONNECTING(CSegment* pParam);

  void                     OnCopDecoderVideoRefreshCONNECTED(CSegment* pParam);
  void                     OnCopDecoderVideoRefreshINSWITCH(CSegment* pParam);
  void                     OnCopDecoderVideoRefreshDISCONNECTING(CSegment* pParam);
  void                     OnCopDecoderVideoRefresh(CSegment* pParam);

  void                     OnCopDecoderVideoInSyncCONNECTED(CSegment* pParam);
  void                     OnCopDecoderVideoInSyncINSWITCH(CSegment* pParam);
  void                     OnCopDecoderVideoVideoInSyncDISCONNECTING(CSegment* pParam);

  void                     OnConfConnectPartyCONNECTED(CSegment* pParam);
  void                     OnConfConnectPartyINSWITCH(CSegment* pParam);

  void                     OnEndPartyConnectCONNECTED(CSegment* pParam);
  void                     OnEndPartyConnectINSWITCH(CSegment* pParam);
  void                     OnEndPartyConnect(CSegment* pParam, BYTE updatePartyDB);

  void                     OnEndPartyConnectIVRModeINSWITCH(CSegment* pParam);

  void                     OnEndPartyUpdateVideoInINSWITCH(CSegment* pParam);
  void                     OnEndPartyUpdateVideoInCONNECTED(CSegment* pParam);

  virtual void             OnConfDisConnectPartyCONNECTED(CSegment* pParam);
  virtual void             OnConfDisConnectPartyINSWITCH(CSegment* pParam);

  virtual void             OnConfDeletePartyFromConfCONNECTED(CSegment* pParam);
  void                     OnConfDeletePartyFromConfINSWITCH(CSegment* pParam);
  void                     OnConfDeletePartyFromConf(CSegment* pParam);

  void                     OnConfSetConfVideoLayoutSeeMeAllCONNECTED(CSegment* pParam);
  void                     OnConfSetConfVideoLayoutSeeMeAllINSWITCH(CSegment* pParam);
  void                     OnConfSetConfVideoLayoutSeeMeAll(CSegment* pParam);

  virtual void             OnConfUpdateVideoMuteCONNECTED(CSegment* pParam);
  void                     OnConfUpdateVideoMuteINSWITCH(CSegment* pParam);
  void                     OnConfUpdateVideoMute(CSegment* pParam);

  void                     OnConfSpeakersChangedCONNECTED(CSegment* pParam);
  void                     OnConfSpeakersChangedINSWITCH(CSegment* pParam);
  void                     OnConfSpeakersChanged(CSegment* pParam);
  void                     AudioSpeakerChanged(CTaskApp* pNewAudioSpeaker);

  virtual void             OnConfVideoRefreshCONNECTED(CSegment* pParam);
  virtual void             OnConfVideoRefreshINSWITCH(CSegment* pParam);
  virtual void             OnConfVideoRefresh(CSegment* pParam);

  virtual void             OnVideoPreviewRefreshCONNECTED(CSegment* pParam);
  virtual void             OnVideoPreviewRefreshINSWITCH(CSegment* pParam);
  virtual void             OnVideoPreviewRefresh(CSegment* pParam);

  virtual void             OnVideoInSynced(CSegment* pParam);

  void                     OnEndPartyUpdateVideoOutCONNECTED(CSegment* pParam);
  void                     OnEndPartyUpdateVideoOutINSWITCH(CSegment* pParam);
  void                     OnEndPartyUpdateVideoOutDISCONNECTING(CSegment* pParam);
  void                     OnEndPartyUpdateVideoOut(CSegment* pParam);

  void                     OnConfSetLectureModeCONNECTED(CSegment* pParam);
  void                     OnConfSetLectureModeINSWITCH(CSegment* pParam);

  void                     OnTimerConnectCOPVideoBridgeIDLE(CSegment* pParam);

  // auto scan
  void                     OnAutoScanTimerConnected(CSegment* pParam);
  void                     OnConfSetAutoScanOrderCONNECTED(CSegment* pParam);
  void                     OnConfSetAutoScanOrderINSWITCH(CSegment* pParam);
  void                     OnConfSetAutoScanOrder(CSegment* pParam);

  void                     SendFastUpdateToPartyLevelEncoder(CVideoBridgePartyCntlCOP* pPartyCntl);

  void                     CloseLevelEncoders();
  void                     CloseDynamicDecoders();
  void                     ClosePcmEncoder();
  void                     DestroyDynamicDecoders();
  void                     StopSwitchActionsOnDisconnecting();

  void                     DestroyCopEncoder(WORD copEncoderIndex);
  void                     DestroyCopDecoder(WORD copDecoderIndex);

  CopRsrcDesc              GetCopRsrcDesc(ECopRsrcType copRsrcType, WORD index = 0) const;

  BOOL                     AreAllCopRsrcsDestroyed();

  virtual void             NewPartyCntl(CVideoBridgePartyCntl*& pVideoBrdgPartyCntl);
  virtual void             CreateAndNewPartyCntl(CVideoBridgePartyCntl*& pVideoBrdgPartyCntl, CVideoBridgePartyInitParams* pVideoBridgePartyInitParams);

  virtual void             RemovePartyFromConfMixBeforeDisconnecting(const CTaskApp* pParty);
  virtual void             AddPartyToConfMix(const CTaskApp* pParty);
  virtual void             RemovePartyFromAnyConfSettingsWhenDeletedFromConf(const char* pDeletedPartyName);

  void                     AddPartyToConfMixChange(const CTaskApp* pParty);
  void                     OnMplConnectAck(BYTE status);

  virtual void             OnLevelEncoderEndDisconnectDISCONNECTING(CSegment* pParam);
  virtual void             OnLevelEncoderEndDisconnectCONNECTED(CSegment* pParam);
  virtual void             OnLevelEncoderEndDisconnectINSWITCH(CSegment* pParam);
  virtual void             GetForcesFromReservation(CCommConf* pCommConf);
  virtual void             UpdateDB_ConfLayout();

  // calculate required actions for change layout
  void                     CalculateChangeLayoutActions(CLayout& newLayout, CLayout& oldLayout);
  void                     CalculateChangeLayoutActionsForDecoder(WORD decoder_index, DWORD oldArtId, DWORD newArtId, ECopDecoderResolution newRequiredResolution, CObjString& cstr, BYTE isNeedToConsiderOutRes);
  void                     CalculateChangeLayoutActionsForDecoderWhenOldArtIsNotValid(WORD decoder_index, DWORD newArtId, ECopDecoderResolution newRequiredResolution, CObjString& cstr, BYTE isNeedToConsiderOutRes);
  WORD                     GetDecoderIndexInLayout(CLayout& layout, WORD decoder_index, ECopDecoderResolution& requiredResolution);
  void                     AdjustRequiredResolotion(ECopDecoderResolution& newRequiredResolution, ECopDecoderResolution newPartyMaxDecoderResolution, DWORD newPartyAlgorithm, CObjString& cstr, DWORD newArtId, ECopDecoderResolution copDecoderResolutionFromEncoder, BYTE isNeedToConsiderOutRes);
  void                     AdjustRequiredResolotionToMaxH263Resolution(ECopDecoderResolution& newRequiredResolution, DWORD& newPartyAlgorithm, DWORD newArtId);
  void                     AdjustRequiredResolotionToConfMaxResolution(ECopDecoderResolution& newRequiredResolution);
  ECopDecoderResolution    GetDecoderResolutionFromEncoder(CVideoBridgePartyCntlCOP* partyCntl);

  // execute required actions for changelayout
  void                     StartSwitch();
  void                     StartSwitchTimer();
  void                     StartSmartSwitch(DWORD smartSwitchTout, WORD closeDisconnectDecoderIndex);
  void                     StartSwitchOpenAndConnectDecodersActions();
  void                     EndChangeLayoutActions();
  void                     SendChangeLayoutToAllLevelEncoders();
  void                     SendLayoutAttributesToEncoders();
  void                     SendLayoutAttributesToLecturerEncoder();
  void                     SendLayoutAttributesToListenersEncoders();
  void                     SendChangeModeToParty(WORD decoderIndex, DWORD artPartyId, ECopDecoderResolution copDecRes);
  void                     SendClosePortToDecoder(WORD decoderIndex);
  void                     SendDisconnectPortToDecoder(WORD decoderIndex);
  void                     SendConnectPortToDecoder(WORD decoderIndex, DWORD artPartyId);
  void                     SendOpenPortToDecoder(WORD decoderIndex, DWORD artPartyId);
  void                     CreateCopDecoderInitParams(WORD decoderIndex, DWORD artPartyId);
  BOOL                     UpdateDecoderVideoParams(CBridgePartyVideoInParams* pInVideoParams, WORD decoderIndex, DWORD artPartyId);
  BOOL                     AreAllCloseDisconnnectChangeModeDecActionsCompleted();
  BOOL                     AreAllCloseDisconnnectChangeModeDecActionsCompletedInSmartSwitch();
  BOOL                     AreAllOpenConnectSyncDecActionsCompleted();
  DWORD                    GetSmartSwitchTout();
  void                     OnTimerEndSmartSwitchINSWITCH(CSegment* pParam);
  BOOL                     IsWaitingToDecoderSyncInSmartSwitch();
  BOOL                     IsSwitchWithDisconnectedParty(DWORD disconnectingPartyArtId);
  void                     SendDisconnectToAllPartiesInSwitchVector();
  void                     AddPartyToDisconnectVectorAfterSwitchEnds(DWORD disconnectingPartyArtId, EMediaDirection disconnectDirection);
  void                     UpdateCopEncoderPartiesLayoutInDB(CLayout* layout, DWORD levelEncoderEntityId);
  void                     UpdateCopPartyLayoutInDB(CLayout* layout, CVideoBridgePartyCntlCOP* pPartyCntl);

  void                     AddNewArtPartyIdToDiscVector(DWORD art, EMediaDirection disconnectDirection);
  WORD                     GetNumberOfArtPartyIdsInDiscVector() {return m_pDisconnectingPartiesInSwitchVector->size();}
  void                     ClearDiscVector(void)                {m_pDisconnectingPartiesInSwitchVector->clear();}
  void                     SimulateUpdateVideoInFromParty(DWORD artPartyId, ECopDecoderResolution copDecRes);
  void                     HandleUnfinishedChangeLayoutActions();
  BOOL                     AreThereUnfinishedChangeLayoutActionsWeNeedToHandle();
  void                     RemovePartyFromDisconnectingVectorIfExists(DWORD artPartyId);

  virtual void             OnVideoBridgeUpdateMessageOverlayCONNECTED(CSegment* pParam);
  virtual void             OnVideoBridgeUpdateMessageOverlayINSWITCH(CSegment* pParam);
  virtual void             UpdateMessageOverlay(CSegment* pParam);
  void                     AddDecoderIndexToCalcStr(WORD decoder_index, CObjString& cstr);
  void                     AddPartyIdToCalcStr(DWORD party_id, CObjString& cstr);
  void                     AddVideoAlgToCalcStr(DWORD algo, CObjString& cstr);
  void                     AddResolutionToCalcStr(ECopDecoderResolution res, CObjString& cstr);

  // LPR and FlowControl support
  void                     OnConfUpdateFlowControlRateCONNECTED(CSegment* pParam);
  void                     OnConfUpdateFlowControlRateINSWITCH(CSegment* pParam);
  virtual void             OnConfUpdateFlowControlRate(CSegment* pParam);
  DWORD                    FindLowestFlowControlRateForLevelEncoder(DWORD levelEncoderEntityId);
  void                     UpdateFlowControlRateWhenPartyConnectOrDisconnectsFromPCMEncoder(CVideoBridgePartyCntlCOP* pPartyCntl, BOOL isConnectToPCM);
  void                     UpdateFlowControlRateWhenPartyDisconnectsFromEncoder(CVideoBridgePartyCntlCOP* pPartyCntl);
  void                     HandleFlowControlRequestForVSWEncoder(CVideoBridgePartyCntlCOP* pFlowCntlInitiatorPartyCntl, DWORD newBitRate, CLPRParams* pLprParams);
  void                     UpdateFlowControlRateWhenMovingParticipantsToAndFromVSWEncoder(BYTE toVsw);
  void                     ResetLecturerFlowControlRate();

  // lecture mode actions
  void                     StartLectureMode();
  void                     EndLectureMode(BYTE removeLecturer);
  void                     ChangeLecturer(CVideoBridgeLectureModeParams& pNewVideoBridgeLectureMode, BOOL isUpdateStateNeeded = TRUE);
  void                     StartCascadeLecturerMode();
  void                     EndCascadeLecturerMode(BYTE removeLecturer, CVideoBridgePartyCntl* pLecturer);
  void                     ChangeLecturerToCascadeLecturer(CVideoBridgeLectureModeParams& pNewVideoBridgeLectureMode);
  void                     ChangeLecturerFromCascadeLecturer(CVideoBridgeLectureModeParams& pNewVideoBridgeLectureMode);
  void                     ChangeLecturerToAndFromCascadeLecturer(CVideoBridgeLectureModeParams& pNewVideoBridgeLectureMode);

  void                     EndStartLectureMode();
  void                     EndEndLectureMode(BYTE removeLecturer);
  void                     EndChangeLecturer(BYTE removeLecturer);
  void                     EndStartCascadeLecturerMode();
  void                     EndEndCascadeLecturerMode();
  void                     EndChangeLecturerToCascadeLecturer();
  void                     EndChangeLecturerFromCascadeLecturer();
  void                     EndChangeLecturerToAndFromCascadeLecturer();
  void                     EndCurrentLectureModeActionForCascadeLecturer(LectureModeAction lecture_mode_action);

  // lecture mode video bridge actions
  void                     OpenLecturerCodecs(CVideoBridgePartyCntlCOP* pLecturer);
  void                     CloseLecturerCodecs();
  void                     SwitchListenersEncoder(BYTE toVsw);
  void                     UpdateCascadeLinkAsLecturer(CVideoBridgePartyCntlCOP* pLecturer);
  void                     UpdateCascadeLinkAsNotLecturer(CVideoBridgePartyCntlCOP* pLecturer);
  void                     UpdateCascadeLinkAsNotLecturerForDisconnectingParty();
  void                     UpdatePartyCascadeLecturerMode(CVideoBridgePartyCntlCOP* pLecturer, BYTE isSetCascadeAsLecturer);

  void                     EndOpenLecturerCodecs();
  void                     EndCloseLecturerCodecs();
  void                     EndSwitchListenersEncoder();
  void                     EndUpdateCascadeLinkAsLecturer();
  void                     EndUpdateCascadeLinkAsNotLecturer();

  // lecture mode decoder / encoder management
  void                     CreateLecturerDecoder(CVideoBridgePartyCntlCOP* pLecturerBridgePartyCntl);
  void                     CreateLecturerVSWDecoder(CVideoBridgePartyCntlCOP* pLecturerBridgePartyCntl);
  void                     CreateVswEncoder(const CBridgePartyVideoParams& rBridgePartyVideoParams);

  // lecture mode layout management
  void                     BuildLayout1x1(CLayout& rResultLayout);
  const char*              ForceLecturer(CLayout& rResultLayout) const;
  void                     UpdateLectureDecoderParamsInImage(CLayout& rResultLayout) const;
///  void                     RemoveLecturerImageFromImagesVector(CPartyImageVector& rImagesReadyForSetting) const;
///  void                     RemoveLecturerForce(CLayout& rResultLayout) const;
  BYTE                     IsLecturerForced(CLayout& rResultLayout) const;
  void                     SetLecturerNameInDB(const char* lecturer_name);

  BYTE                     IsActiveLectureMode() const;
  BYTE                     IsLecturerInDB(const char* party_name) const;

  CVideoBridgePartyCntl*   GetVideoBridgePartyCntl(DWORD artPartyId);
  const char*              GetPartyName(DWORD decoderArtPartyId) const;

  CVideoBridgeCopDecoder*  GetCopDecoder(WORD decoder_index);
  BYTE                     IsOnlyLecturerConnected() const;
  BYTE                     IsLecturerDecodersConnected() const;
  BYTE                     IsLecturerCodecsConnected() const;
  BYTE                     IsPartyUsesLecturerEncoder(CBridgePartyInitParams& partyInitParams) const;
  void                     SetVswEncoderParams(CBridgePartyInitParams& partyInitParams);
  void                     FastUpdateFromLecturer(WORD ignore_intra_filtering = FALSE, WORD isRequestFromRemoteMGC = FALSE) const;
  void                     SwitchlecturerDecoderIdInImage(CLayout& rResultLayout, ECopConstRsrcs decoderType) const;
  void                     OnCopEncoderSyncIndCONNECTED(CSegment* pParam);
  void                     OnCopEncoderSyncIndINSWITCH(CSegment* pParam);
  void                     OnCopEncoderSyncInd(CSegment* pParam);
  void                     DumpConnnections(const char* pDumpHeader) const;
  void                     DumpEncoderParties(DWORD encoderEntityId, std::ostringstream& msg) const;
  void                     DumpLayouts(const char* pDumpHeader);
  WORD                     GetCopEncoderIndex(CVideoBridgePartyCntlCOP* pPartyCntl) const;
  BYTE                     IsUpdateEncodersEnd() const;
  void                     SetVideoParams(CBridgePartyVideoOutParams& rOutVideoParams, const CBridgePartyVideoParams& rBridgePartyVideoParams) const;
  BYTE                     IsLectureCodecsClosed() const;
  void                     SetCurrentSpeakerLecturerParams(CVideoBridgeLectureModeParams& rNewVideoBridgeLectureMode);
  void                     DoLectureModeBaseAction(LectureModeAction lecture_mode_action, CVideoBridgeLectureModeParams& rNewVideoBridgeLectureMode);
  void                     EndLectureModeBaseAction();
  void                     OnTimerLectureModeActionFailed(CSegment* pParam);
  void                     ChangeModeLecturerByLevelEncoder(CVideoBridgePartyCntlCOP* pLecturer);
  void                     EndChangeModeLecturerByLevelEncoder();
  BYTE                     IsLecturerConnectedToPCM();
  void                     ApplicationActionsOnDeletePartyFromConf(const char* pDeletedPartyName);
  void                     ApplicationActionsOnRemovePartyFromMix(CVideoBridgePartyCntl* pPartyCntl);
  void                     ApplicationActionsOnAddPartyToMix(CVideoBridgePartyCntl* pPartyCntl);
  WORD                     UpdateLecturerCodecsVideoParamsFromLevel(CBridgePartyVideoOutParams* pOutVideoParams, CCommConf* pCommConf, WORD encoder_index) const;
  void                     EndCurrentLectureModeAction();
  BYTE                     IsLecturerReady();
  void                     SetSmartLecturerState(CVideoBridgePartyCntlCOP* pOldLecturer, CVideoBridgePartyCntlCOP* pNewLecturer);
  void                     DoPendingLectureModeActionOnEndOfChangeLayout();
  void                     InformACLayoutChangeCompleted(BYTE just_send_indication = FALSE);
  void                     SendChangeLayoutOnLecturerDecoderSync();
  void                     SendChangeLayoutToLevelEncoder(DWORD encoder_index, BYTE sendChangeLayoutAnyWay);
  void                     SendChangeLayoutToVSWEncoder(DWORD encoder_index, BYTE sendChangeLayoutAnyWay);
  void                     OnConfLecturerDecoderTimerCONNECTED(CSegment* pParam);
  void                     OnConfLecturerDecoderTimerINSWITCH(CSegment* pParam);
  BYTE                     IsUpdatedPartyToFromVSW(CVideoBridgePartyCntlCOP* pPartyCntl, BYTE toVsw);

  // delay decoder sync timer
  void                     OnTimerDelayDecoderSyncINSWITCH(CSegment* pParam);

  // DSP smart switch
  void                     StartDSPSmartSwitch();
  void                     SendDSPSmartSwitchChangeLayoutToAllLevelEncoders();
  void                     UpdateDSPSmartSwitchImagesParam(CLayout& newLayout, CLayout& oldLayout);
  BYTE                     IsDSPSmartSwitchNeeded(CLayout oldLayout);
  void                     OnTimerEndDSPSmartSwitchINSWITCH(CSegment* pParam);
  void                     SetIsRemoteNeedSmartSwitchAccordingToVendor(CSegment* pParam);
  BYTE                     IsRemoteNeedSmartSwitchAccordingToVendor(DWORD artPartyId);

  // enable/disable intra filtering
  // action functions
  void                     OnConfContentBridgeStopPresentationCONNECTED(CSegment* pParam);
  void                     OnConfContentBridgeStopPresentationINSWITCH(CSegment* pParam);
  void                     OnConfContentBridgeStartPresentationCONNECTED(CSegment* pParam);
  void                     OnConfContentBridgeStartPresentationINSWITCH(CSegment* pParam);
  void                     OnTimerDisableIntraAfterStopContent(CSegment* pParam);
  void                     OnTimerDisableIntraAfterStartContent(CSegment* pParam);

  // enable/disable intra filtering cases
  void                     IgnoreIntraFilteringAfterStopContent();
  void                     IgnoreIntraFilteringAfterStartContent();

  // enable intra supression per type
  bool                     IsIntraSuppressEnabled(WORD intra_suppression_type) const;
  void                     EnableIntraSuppress(WORD intra_suppression_type);
  void                     DisableIntraSuppress(WORD intra_suppression_type);

  void                     StartCascadeLinkAsLecturerPendingMode(CVideoBridgePartyCntlCOP* pLecturer);
  void                     EndStartCascadeLinkAsLecturerPendingMode();

  WORD                     UpdateEncoderVideoParamsForAsymmetricCascade(CBridgePartyVideoOutParams& pOutVideoParams, WORD encoder_index) const;
  void                     UpdateCascadeLinkAsLecturerOnRecevingUpdateVideoOutParams(CBridgePartyVideoOutParams* pBridgePartyVideoParams, CVideoBridgePartyCntlCOP* pLecturer);
  void                     UpdateCascadeLinkAsNotLecturerOnRecevingUpdateVideoOutParams(CBridgePartyVideoOutParams* pBridgePartyVideoParams, CVideoBridgePartyCntlCOP* pPrevCascadeLecturer);
  void                     UpdateCascadeLinkAsNotLecturerOnDisconnectingCascadeLink();

  BYTE                     IsEndUpdateCascadeLinkAsLecturer();
  BYTE                     IsPrevCascadeLinkLecturer(CVideoBridgePartyCntl* pVideoBridgePartyCntl) const;
  CVideoBridgePartyCntl*   GetPrevCascadeLinkLecturer();
  LectureModeAction        GetLectureModeChangeLecturerBaseAction(CVideoBridgePartyCntlCOP* oldLecturer, CVideoBridgePartyCntlCOP* newLecturer);
  void                     OnConfUpdateSiteNameInfoCONNECTED(CSegment* pParam);
  void                     OnConfUpdateRefreshLayoutCONNECTED(CSegment* pParam);

  CVideoBridgeCopEncoder* m_pCopLevelEncoders[NUM_OF_LEVEL_ENCODERS];
  CVideoBridgeCopDecoder* m_pCopDecoders[NUM_OF_COP_DECODERS];
///  eVideoConfType          m_VideoConfType;  // initiate to  eVideoConfTypeCopHD108025fps / eVideoConfTypeCopHD72050fps by first level encoder
///  CopRsrcsParams*         m_pCopRsrcs;      // we keep the CopRsrcsParams to update the decoders partyRsrc Id on create

  // Lecture mode
  CVideoBridgeCopDecoder* m_pCopDecoderLecturer;
  CVideoBridgeCopDecoder* m_pCopDecoderVsw;
  CVideoBridgeCopEncoder* m_pCopEncoderVsw;
  WORD                    m_lecturerlevelEncoderIndex;
///  CCopLectureModeCntl*    m_pCopLectureModeCntl;
  BYTE                    m_pCopCascadeLinkLecturerEncoderWasUpdated;
  BYTE                    m_pCopCascadeLinkLecturerLinkOutParamsWereUpdated;
  WORD                    m_copEncoderIndexOfCascadeLinkLecturer;

  // 2) layout handling
///  CLayout*                m_ConfLayout; // active layout
  BYTE                    m_SwitchInSwitch;
  LayoutType              m_SwitchInSwitchLayoutType; // current layout
  BOOL                    m_IsSmartSwitch;
  BOOL                    m_IsSmartSwitchTimerPoped;
  DISCONNECTING_VECTOR*   m_pDisconnectingPartiesInSwitchVector;

  // In MPMX the DSP smart switch needs special treatment in the MCMS side
  BOOL                    m_IsDSPSmartSwitch;
  BOOL                    m_IsDSPSmartSwitchTimerSet;

  // ChangeLayoutActionsManager m_changeLayoutActionsManager;
  CLayout*                m_ConfLayout1x1;              // lecture mode 1x1 layout
///  CLayout*                m_pReservation[CP_NO_LAYOUT]; // reservation layout
///  LayoutType              m_layoutType;                 // current layout

  // visual effects
  CVisualEffectsParams*   m_pVisualEffects; // borders etc
///  CTaskApp*               m_pLastSpeakerNotationParty; // Save the last speakerNotation party to avoid redundant msgs to VideoCard
  BYTE                    m_isVideoClarityEnabled;
  CChangeLayoutActions*   m_changeLayoutActions;
  WORD                    m_resourcesStatus;
  int                     m_openRequests;
  int                     m_closeRequests;
  CVideoBridgeCopEncoder* m_pPCMEncoderCntl;
  DWORD                   m_pcmMenuId;
  DWORD                   m_pcmLevelIndex;
  BYTE                    m_pcmConnected;
  CVisualEffectsParams*   m_pPcmFeccVisualEffects; // speaker notation is changed in order to set which party to move in fecc
  CSmartLecturerState     m_SmartLecturerState;

  BYTE                    m_needToRequestIntraFromLevelEncoder[NUM_OF_LEVEL_ENCODERS];

  // AC hand shake - inform AC when layout change that started because of speaker change is done
  BYTE                    m_isChangeLayoutBecauseOfSpeakerChange;
  BYTE                    m_isPendingChangeLayoutBecauseOfSpeakerChange;
  BYTE                    m_isChangeLayoutBecauseOfLectureModeFlows;
  BYTE                    m_SendChangeLayoutAnyWay;
  bool                    m_enableIntraSupress[NUM_OF_INTRA_SUPPRESS_TYPES];

  PDECLAR_MESSAGE_MAP
};

#endif /*VIDEOBRIDGECOP_H_*/
