//+========================================================================+
//                     VideoBridgeCop.cpp                                  |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       VideoBridgeCop.cpp                                          |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who  | Date    | Description                                            |
//+========================================================================+
#include <memory>
#include "VideoBridgeCop.h"
#include "VideoBridgeInitParams.h"
#include "OpcodesMcmsCardMngrTB.h"
#include "H264Util.h"
#include "VideoBridgePartyCntl.h"
#include "BridgePartyVideoIn.h"
#include "BridgeVideoOutCopEncoder.h"
#include "BridgePartyVideoParams.h"
#include "BridgePartyVideoUniDirection.h"
#include "VideoHardwareInterface.h"
#include "TaskApi.h"
#include "StatusesGeneral.h"
#include "TraceStream.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsVideo.h"
#include "OpcodesMcmsCardMngrTB.h"
#include "HostCommonDefinitions.h"
#include "BridgePartyDisconnectParams.h"
#include "BridgePartyInitParams.h"
#include "VideoBridgePartyInitParams.h"
#include "CommResDB.h"
#include "Conf.h"
#include "COP_video_mode.h"
#include "Party.h"
#include "COP_Layout_definitions.h"
#include "ChangeLayoutActions.h"
#include "VideoBridgeAutoScanParams.h"
#include "ConfPartyGlobals.h"
#include "VideoApiDefinitionsStrings.h"

extern DWORD GetPcmYUVColor();
extern BYTE GetDebugFlagForDisconnectLecturerPrevDecoder();
extern CConfPartyRoutingTable* GetpConfPartyRoutingTable(void);
LayoutType GetNewLayoutType(const BYTE oldLayoutType);

PBEGIN_MESSAGE_MAP(CVideoBridgeCOP)
  //COP ENCODER ACTIONS
  ONEVENT(END_CONNECT_COP_ENCODER,                          IDLE,           CVideoBridgeCOP::OnLevelEncoderEndConnectIDLE)
  ONEVENT(END_CONNECT_COP_ENCODER,                          CONNECTED,      CVideoBridgeCOP::OnLevelEncoderEndConnectCONNECTED)
  ONEVENT(END_CONNECT_COP_ENCODER,                          INSWITCH,       CVideoBridgeCOP::OnLevelEncoderEndConnectINSWITCH)
  ONEVENT(END_CONNECT_COP_ENCODER,                          DISCONNECTING,  CVideoBridgeCOP::OnLevelEncoderEndConnectDISCONNECTING)

  ONEVENT(END_DISCONNECT_COP_ENCODER,                       CONNECTED,      CVideoBridgeCOP::OnLevelEncoderEndDisconnectCONNECTED)
  ONEVENT(END_DISCONNECT_COP_ENCODER,                       INSWITCH,       CVideoBridgeCOP::OnLevelEncoderEndDisconnectINSWITCH)
  ONEVENT(END_DISCONNECT_COP_ENCODER,                       DISCONNECTING,  CVideoBridgeCOP::OnLevelEncoderEndDisconnectDISCONNECTING)

  ONEVENT(COP_ENCODER_VIDEO_OUT_UPDATED,                    CONNECTED,      CVideoBridgeCOP::OnLevelEncoderVideoOutUpdatedCONNECTED)
  ONEVENT(COP_ENCODER_VIDEO_OUT_UPDATED,                    INSWITCH,       CVideoBridgeCOP::OnLevelEncoderVideoOutUpdatedINSWITCH)
  ONEVENT(COP_ENCODER_VIDEO_OUT_UPDATED,                    DISCONNECTING,  CVideoBridgeCOP::NullActionFunction)

  //COP DECODER ACTIONS
  ONEVENT(END_CLOSE_COP_DECODER,                            CONNECTED,      CVideoBridgeCOP::OnCopDecoderEndCloseCONNECTED)
  ONEVENT(END_CLOSE_COP_DECODER,                            INSWITCH,       CVideoBridgeCOP::OnCopDecoderEndCloseINSWITCH)
  ONEVENT(END_CLOSE_COP_DECODER,                            DISCONNECTING,  CVideoBridgeCOP::OnCopDecoderEndCloseDISCONNECTING)

  ONEVENT(END_DISCONNECT_COP_DECODER,                       CONNECTED,      CVideoBridgeCOP::OnCopDecoderEndDisconnectCONNECTED)
  ONEVENT(END_DISCONNECT_COP_DECODER,                       INSWITCH,       CVideoBridgeCOP::OnCopDecoderEndDisconnectINSWITCH)
  ONEVENT(END_DISCONNECT_COP_DECODER,                       DISCONNECTING,  CVideoBridgeCOP::OnCopDecoderEndDisconnectDISCONNECTING)

  ONEVENT(END_CONNECT_COP_DECODER,                          CONNECTED,      CVideoBridgeCOP::OnCopDecoderEndConnectCONNECTED)
  ONEVENT(END_CONNECT_COP_DECODER,                          INSWITCH,       CVideoBridgeCOP::OnCopDecoderEndConnectINSWITCH)
  ONEVENT(END_CONNECT_COP_DECODER,                          DISCONNECTING,  CVideoBridgeCOP::OnCopDecoderEndConnectDISCONNECTING)

  ONEVENT(VIDEO_REFRESH_COP_DECODER,                        CONNECTED,      CVideoBridgeCOP::OnCopDecoderVideoRefreshCONNECTED)
  ONEVENT(VIDEO_REFRESH_COP_DECODER,                        INSWITCH,       CVideoBridgeCOP::OnCopDecoderVideoRefreshINSWITCH)
  ONEVENT(VIDEO_REFRESH_COP_DECODER,                        DISCONNECTING,  CVideoBridgeCOP::OnCopDecoderVideoRefreshDISCONNECTING)

  ONEVENT(VIDEO_IN_SYNCED_COP_DECODER,                      CONNECTED,      CVideoBridgeCOP::OnCopDecoderVideoInSyncCONNECTED)
  ONEVENT(VIDEO_IN_SYNCED_COP_DECODER,                      INSWITCH,       CVideoBridgeCOP::OnCopDecoderVideoInSyncINSWITCH)
  ONEVENT(VIDEO_IN_SYNCED_COP_DECODER,                      DISCONNECTING,  CVideoBridgeCOP::OnCopDecoderVideoVideoInSyncDISCONNECTING)

  ONEVENT(PARTY_VIDEO_IN_UPDATED,                           INSWITCH,       CVideoBridgeCOP::OnEndPartyUpdateVideoInINSWITCH)
  ONEVENT(PARTY_VIDEO_IN_UPDATED,                           CONNECTED,      CVideoBridgeCOP::OnEndPartyUpdateVideoInCONNECTED)

  ONEVENT(CONNECTPARTY,                                     CONNECTED,      CVideoBridgeCOP::OnConfConnectPartyCONNECTED)
  ONEVENT(CONNECTPARTY,                                     INSWITCH,       CVideoBridgeCOP::OnConfConnectPartyINSWITCH)

  ONEVENT(ENDCONNECTPARTY,                                  CONNECTED,      CVideoBridgeCOP::OnEndPartyConnectCONNECTED)
  ONEVENT(ENDCONNECTPARTY,                                  INSWITCH,       CVideoBridgeCOP::OnEndPartyConnectINSWITCH)
  ONEVENT(ENDCONNECTPARTY,                                  DISCONNECTING,  CVideoBridgeCOP::OnEndPartyConnectDISCONNECTING)

  ONEVENT(DISCONNECTPARTY,                                  INSWITCH,       CVideoBridgeCOP::OnConfDisConnectPartyINSWITCH)

  ONEVENT(ENDDISCONNECTPARTY,                               INSWITCH,       CVideoBridgeCOP::OnEndPartyDisConnect) // same as base class

  ONEVENT(DELETED_PARTY_FROM_CONF,                          CONNECTED,      CVideoBridgeCOP::OnConfDeletePartyFromConfCONNECTED)
  ONEVENT(DELETED_PARTY_FROM_CONF,                          INSWITCH,       CVideoBridgeCOP::OnConfDeletePartyFromConfINSWITCH)
  ONEVENT(DELETED_PARTY_FROM_CONF,                          DISCONNECTING,  CVideoBridgeCOP::NullActionFunction)

  ONEVENT(VIDEOMUTE,                                        CONNECTED,      CVideoBridgeCOP::OnConfUpdateVideoMuteCONNECTED) // needs COP implementation
  ONEVENT(VIDEOMUTE,                                        INSWITCH,       CVideoBridgeCOP::OnConfUpdateVideoMuteINSWITCH) // needs COP implementation
  ONEVENT(VIDEOMUTE,                                        DISCONNECTING,  CVideoBridgeCOP::NullActionFunction)

  ONEVENT(SPEAKERS_CHANGED,                                 CONNECTED,      CVideoBridgeCOP::OnConfSpeakersChangedCONNECTED)
  ONEVENT(SPEAKERS_CHANGED,                                 INSWITCH,       CVideoBridgeCOP::OnConfSpeakersChangedINSWITCH)
  ONEVENT(SPEAKERS_CHANGED,                                 DISCONNECTING,  CVideoBridgeCOP::NullActionFunction)

  ONEVENT(SETCONFVIDLAYOUT_SEEMEALL,                        CONNECTED,      CVideoBridgeCOP::OnConfSetConfVideoLayoutSeeMeAllCONNECTED)
  ONEVENT(SETCONFVIDLAYOUT_SEEMEALL,                        INSWITCH,       CVideoBridgeCOP::OnConfSetConfVideoLayoutSeeMeAllINSWITCH)
  ONEVENT(SETCONFVIDLAYOUT_SEEMEALL,                        DISCONNECTING,  CVideoBridgeCOP::NullActionFunction)

  ONEVENT(DISCONNECTCONF,                                   IDLE,           CVideoBridgeCOP::OnConfDisConnectConfIDLE)
  ONEVENT(DISCONNECTCONF,                                   INSWITCH,       CVideoBridgeCOP::OnConfDisConnectConfINSWITCH)

  ONEVENT(VIDEO_IN_SYNCED,                                  CONNECTED,      CVideoBridgeCOP::OnVideoInSynced)
  ONEVENT(VIDEO_IN_SYNCED,                                  INSWITCH,       CVideoBridgeCOP::OnVideoInSynced)
  ONEVENT(VIDEO_IN_SYNCED,                                  DISCONNECTING,  CVideoBridgeCOP::OnVideoInSynced)

  ONEVENT(ENDCONNECTPARTY_IVR_MODE,                         INSWITCH,       CVideoBridgeCOP::OnEndPartyConnectIVRModeINSWITCH)

  ONEVENT(PARTY_VIDEO_OUT_UPDATED,                          CONNECTED,      CVideoBridgeCOP::OnEndPartyUpdateVideoOutCONNECTED)
  ONEVENT(PARTY_VIDEO_OUT_UPDATED,                          INSWITCH,       CVideoBridgeCOP::OnEndPartyUpdateVideoOutINSWITCH)
  ONEVENT(PARTY_VIDEO_OUT_UPDATED,                          DISCONNECTING,  CVideoBridgeCOP::OnEndPartyUpdateVideoOutDISCONNECTING)

  ONEVENT(VIDREFRESH,                                       CONNECTED,      CVideoBridgeCOP::OnConfVideoRefreshCONNECTED)
  ONEVENT(VIDREFRESH,                                       INSWITCH,       CVideoBridgeCOP::OnConfVideoRefreshINSWITCH)

  ONEVENT(UPDATELECTUREMODE,                                CONNECTED,      CVideoBridgeCOP::OnConfSetLectureModeCONNECTED)
  ONEVENT(UPDATELECTUREMODE,                                INSWITCH,       CVideoBridgeCOP::OnConfSetLectureModeINSWITCH)
  ONEVENT(UPDATELECTUREMODE,                                DISCONNECTING,  CVideoBridgeCOP::NullActionFunction)

  ONEVENT(UPDATEAUTOLAYOUT,                                 IDLE,           CVideoBridgeCOP::OnConfUpdateAutoLayoutIDLE)
  ONEVENT(UPDATEAUTOLAYOUT,                                 CONNECTED,      CVideoBridgeCOP::OnConfUpdateAutoLayoutCONNECTED)
  ONEVENT(UPDATEAUTOLAYOUT,                                 INSWITCH,       CVideoBridgeCOP::OnConfUpdateAutoLayoutINSWITCH)
  ONEVENT(UPDATEAUTOLAYOUT,                                 DISCONNECTING,  CVideoBridgeCOP::NullActionFunction)

  ONEVENT(VIDEO_ENCODER_SYNC_IND,                           CONNECTED,      CVideoBridgeCOP::OnCopEncoderSyncIndCONNECTED)
  ONEVENT(VIDEO_ENCODER_SYNC_IND,                           INSWITCH,       CVideoBridgeCOP::OnCopEncoderSyncIndINSWITCH)
  ONEVENT(VIDEO_ENCODER_SYNC_IND,                           DISCONNECTING,  CVideoBridgeCOP::NullActionFunction)
  ONEVENT(BRIDGE_DISCONNECT_TOUT,                           DISCONNECTING,  CVideoBridgeCOP::OnTimerDisconnetDISCONNECTING)

  ONEVENT(PCM_CONNECTED,                                    INSWITCH,       CVideoBridgeCOP::OnPartyConnectedToPCMEncoder)
  ONEVENT(PCM_CONNECTED,                                    CONNECTED,      CVideoBridgeCOP::OnPartyConnectedToPCMEncoder)
  ONEVENT(PCM_CONNECTED,                                    DISCONNECTING,  CVideoBridgeCOP::NullActionFunction)

  ONEVENT(PCM_DISCONNECTED,                                 INSWITCH,       CVideoBridgeCOP::OnPartyDisconnectedFromPCMEncoder)
  ONEVENT(PCM_DISCONNECTED,                                 CONNECTED,      CVideoBridgeCOP::OnPartyDisconnectedFromPCMEncoder)
  ONEVENT(PCM_DISCONNECTED,                                 DISCONNECTING,  CVideoBridgeCOP::NullActionFunction)

  ONEVENT(PCM_UPDATED,                                      INSWITCH,       CVideoBridgeCOP::OnPCMEncoderParamsUpdated)
  ONEVENT(PCM_UPDATED,                                      CONNECTED,      CVideoBridgeCOP::OnPCMEncoderParamsUpdated)
  ONEVENT(PCM_UPDATED,                                      DISCONNECTING,  CVideoBridgeCOP::NullActionFunction)

  ONEVENT(SWITCH_TOUT,                                      INSWITCH,       CVideoBridgeCOP::OnTimerEndSwitchINSWITCH)
  ONEVENT(SWITCH_TOUT,                                      DISCONNECTING,  CVideoBridgeCOP::NullActionFunction)

  ONEVENT(SMART_SWITCH_TOUT,                                INSWITCH,       CVideoBridgeCOP::OnTimerEndSmartSwitchINSWITCH)
  ONEVENT(SMART_SWITCH_TOUT,                                DISCONNECTING,  CVideoBridgeCOP::NullActionFunction)

  ONEVENT(CONNECT_COP_VB_TOUT,                              IDLE,           CVideoBridgeCOP::OnTimerConnectCOPVideoBridgeIDLE)
  ONEVENT(CONNECT_COP_VB_TOUT,                              CONNECTED,      CVideoBridgeCOP::NullActionFunction)
  ONEVENT(CONNECT_COP_VB_TOUT,                              INSWITCH,       CVideoBridgeCOP::NullActionFunction)
  ONEVENT(CONNECT_COP_VB_TOUT,                              DISCONNECTING,  CVideoBridgeCOP::NullActionFunction)

  ONEVENT(SET_MESSAGE_OVERLAY,                              IDLE,           CVideoBridgeCOP::NullActionFunction)
  ONEVENT(SET_MESSAGE_OVERLAY,                              CONNECTED,      CVideoBridgeCOP::OnVideoBridgeUpdateMessageOverlayCONNECTED)
  ONEVENT(SET_MESSAGE_OVERLAY,                              INSWITCH,       CVideoBridgeCOP::OnVideoBridgeUpdateMessageOverlayINSWITCH)
  ONEVENT(SET_MESSAGE_OVERLAY,                              DISCONNECTING,  CVideoBridgeCOP::NullActionFunction)

  ONEVENT(AUTO_SCAN_TIMER,                                  CONNECTED,      CVideoBridgeCOP::OnAutoScanTimerConnected)
  ONEVENT(AUTO_SCAN_TIMER,                                  ANYCASE,        CVideoBridgeCOP::NullActionFunction)

  ONEVENT(SET_AUTOSCAN_ORDER,                               IDLE,           CVideoBridgeCOP::NullActionFunction)
  ONEVENT(SET_AUTOSCAN_ORDER,                               CONNECTED,      CVideoBridgeCOP::OnConfSetAutoScanOrderCONNECTED)
  ONEVENT(SET_AUTOSCAN_ORDER,                               INSWITCH,       CVideoBridgeCOP::OnConfSetAutoScanOrderINSWITCH)
  ONEVENT(SET_AUTOSCAN_ORDER,                               DISCONNECTING,  CVideoBridgeCOP::NullActionFunction)

  ONEVENT(UPDATELECTUREMODE,                                ANYCASE,        CVideoBridgeCOP::NullActionFunction)

  ONEVENT(UPDATE_VIDEO_FLOW_CNTL_RATE,                      IDLE,           CVideoBridgeCOP::NullActionFunction)
  ONEVENT(UPDATE_VIDEO_FLOW_CNTL_RATE,                      CONNECTED,      CVideoBridgeCOP::OnConfUpdateFlowControlRateCONNECTED)
  ONEVENT(UPDATE_VIDEO_FLOW_CNTL_RATE,                      INSWITCH,       CVideoBridgeCOP::OnConfUpdateFlowControlRateINSWITCH)
  ONEVENT(UPDATE_VIDEO_FLOW_CNTL_RATE,                      DISCONNECTING,  CVideoBridgeCOP::NullActionFunction)

  ONEVENT(COP_LECTURE_MODE_ACTION_TIME_OUT,                 ANYCASE,        CVideoBridgeCOP::OnTimerLectureModeActionFailed)
  ONEVENT(LECTURE_MODE_TOUT,                                ANYCASE,        CVideoBridgeCOP::NullActionFunction)

  ONEVENT(LECTURER_DECODER_TIMER,                           IDLE,           CVideoBridgeCOP::NullActionFunction)
  ONEVENT(LECTURER_DECODER_TIMER,                           CONNECTED,      CVideoBridgeCOP::OnConfLecturerDecoderTimerCONNECTED)
  ONEVENT(LECTURER_DECODER_TIMER,                           INSWITCH,       CVideoBridgeCOP::OnConfLecturerDecoderTimerINSWITCH)
  ONEVENT(LECTURER_DECODER_TIMER,                           DISCONNECTING,  CVideoBridgeCOP::NullActionFunction)

  ONEVENT(COP_DSP_SMART_SWITCH_TIME_OUT,                    INSWITCH,       CVideoBridgeCOP::OnTimerEndDSPSmartSwitchINSWITCH)
  ONEVENT(COP_DSP_SMART_SWITCH_TIME_OUT,                    DISCONNECTING,  CVideoBridgeCOP::NullActionFunction)

  ONEVENT(COP_USE_SMART_SWITCH_ACCORDING_TO_VENDOR,         IDLE,           CVideoBridgeCOP::NullActionFunction)
  ONEVENT(COP_USE_SMART_SWITCH_ACCORDING_TO_VENDOR,         CONNECTED,      CVideoBridgeCOP::SetIsRemoteNeedSmartSwitchAccordingToVendor)
  ONEVENT(COP_USE_SMART_SWITCH_ACCORDING_TO_VENDOR,         INSWITCH,       CVideoBridgeCOP::SetIsRemoteNeedSmartSwitchAccordingToVendor)
  ONEVENT(COP_USE_SMART_SWITCH_ACCORDING_TO_VENDOR,         DISCONNECTING,  CVideoBridgeCOP::NullActionFunction)

  ONEVENT(DELAY_DECODER_SYNC_TIMER,                         IDLE,           CVideoBridgeCOP::NullActionFunction)
  ONEVENT(DELAY_DECODER_SYNC_TIMER,                         CONNECTED,      CVideoBridgeCOP::NullActionFunction)
  ONEVENT(DELAY_DECODER_SYNC_TIMER,                         INSWITCH,       CVideoBridgeCOP::OnTimerDelayDecoderSyncINSWITCH)
  ONEVENT(DELAY_DECODER_SYNC_TIMER,                         DISCONNECTING,  CVideoBridgeCOP::NullActionFunction)

  ONEVENT(REQUEST_PREVIEW_INTRA,                            IDLE,           CVideoBridgeCOP::NullActionFunction)
  ONEVENT(REQUEST_PREVIEW_INTRA,                            CONNECTED,      CVideoBridgeCOP::OnVideoPreviewRefreshCONNECTED)
  ONEVENT(REQUEST_PREVIEW_INTRA,                            INSWITCH,       CVideoBridgeCOP::OnVideoPreviewRefreshINSWITCH)
  ONEVENT(REQUEST_PREVIEW_INTRA,                            DISCONNECTING,  CVideoBridgeCOP::NullActionFunction)

  ONEVENT(CONTENT_BRIDGE_STOP_PRESENTATION,                 CONNECTED,      CVideoBridgeCOP::OnConfContentBridgeStopPresentationCONNECTED)
  ONEVENT(CONTENT_BRIDGE_STOP_PRESENTATION,                 INSWITCH,       CVideoBridgeCOP::OnConfContentBridgeStopPresentationINSWITCH)
  ONEVENT(CONTENT_BRIDGE_STOP_PRESENTATION,                 DISCONNECTING,  CVideoBridgeCOP::NullActionFunction)

  ONEVENT(CONTENT_BRIDGE_START_PRESENTATION,                CONNECTED,      CVideoBridgeCOP::OnConfContentBridgeStartPresentationCONNECTED)
  ONEVENT(CONTENT_BRIDGE_START_PRESENTATION,                INSWITCH,       CVideoBridgeCOP::OnConfContentBridgeStartPresentationINSWITCH)
  ONEVENT(CONTENT_BRIDGE_START_PRESENTATION,                DISCONNECTING,  CVideoBridgeCOP::NullActionFunction)

  ONEVENT(DISABLE_INTRA_SUPPRESS_AFTER_STOP_CONTENT_TIMER,  CONNECTED,      CVideoBridgeCOP::OnTimerDisableIntraAfterStopContent)
  ONEVENT(DISABLE_INTRA_SUPPRESS_AFTER_STOP_CONTENT_TIMER,  INSWITCH,       CVideoBridgeCOP::OnTimerDisableIntraAfterStopContent)
  ONEVENT(DISABLE_INTRA_SUPPRESS_AFTER_STOP_CONTENT_TIMER,  DISCONNECTING,  CVideoBridgeCOP::NullActionFunction)

  ONEVENT(DISABLE_INTRA_SUPPRESS_AFTER_START_CONTENT_TIMER, CONNECTED,      CVideoBridgeCOP::OnTimerDisableIntraAfterStartContent)
  ONEVENT(DISABLE_INTRA_SUPPRESS_AFTER_START_CONTENT_TIMER, INSWITCH,       CVideoBridgeCOP::OnTimerDisableIntraAfterStartContent)
  ONEVENT(DISABLE_INTRA_SUPPRESS_AFTER_START_CONTENT_TIMER, DISCONNECTING,  CVideoBridgeCOP::NullActionFunction)
  ONEVENT(SET_SITE_NAME,                                    CONNECTED,      CVideoBridgeCOP::OnConfUpdateSiteNameInfoCONNECTED)
  ONEVENT(REFRESHLAYOUT,                                    CONNECTED,      CVideoBridgeCOP::OnConfUpdateRefreshLayoutCONNECTED)
PEND_MESSAGE_MAP(CVideoBridgeCOP, CVideoBridge);

////////////////////////////////////////////////////////////////////////////
//                        CVideoBridgeCOP
////////////////////////////////////////////////////////////////////////////
CVideoBridgeCOP::CVideoBridgeCOP ()
{
  for (WORD decoder_index = 0; decoder_index < NUM_OF_COP_DECODERS; decoder_index++)
    m_pCopDecoders[decoder_index] = new CVideoBridgeCopDecoder();

  for (WORD encoder_index = 0; encoder_index < NUM_OF_LEVEL_ENCODERS; encoder_index++)
  {
    m_pCopLevelEncoders[encoder_index]                  = new CVideoBridgeCopEncoder();
    m_needToRequestIntraFromLevelEncoder[encoder_index] = FALSE;
  }

  for (int i = (int)CP_LAYOUT_1X1; i < (int)CP_NO_LAYOUT; i++)
    m_pReservation[i] = NULL;

  for (int j = 0; j < NUM_OF_INTRA_SUPPRESS_TYPES; j++)
    m_enableIntraSupress[j] = true;

  m_pPCMEncoderCntl                                 = new CVideoBridgeCopEncoder();
  m_pCopDecoderLecturer                             = new CVideoBridgeCopDecoder();
  m_pCopDecoderVsw                                  = new CVideoBridgeCopDecoder();
  m_pCopEncoderVsw                                  = new CVideoBridgeCopEncoder();
  m_ConfLayout1x1                                   = new CLayout(CP_LAYOUT_1X1, ""); // lecture mode layout
  m_SwitchInSwitch                                  = 0;
  m_SwitchInSwitchLayoutType                        = CP_LAYOUT_1X1;
  m_IsSmartSwitch                                   = FALSE;
  m_IsSmartSwitchTimerPoped                         = FALSE;
  m_IsDSPSmartSwitch                                = FALSE;
  m_IsDSPSmartSwitchTimerSet                        = FALSE;
  m_layoutType                                      = CP_LAYOUT_1X1;
  m_pVisualEffects                                  = new CVisualEffectsParams;
  m_pPcmFeccVisualEffects                           = new CVisualEffectsParams;
  m_pLastSpeakerNotationParty                       = NULL;
  m_isVideoClarityEnabled                           = NO;
  m_VideoConfType                                   = eVideoConfTypeCopHD108025fps;
  m_pCopRsrcs                                       = new CopRsrcsParams;
  m_changeLayoutActions                             = new CChangeLayoutActions();
  m_resourcesStatus                                 = STATUS_OK;
  m_pcmConnected                                    = FALSE;
  m_pDisconnectingPartiesInSwitchVector             = new DISCONNECTING_VECTOR;
  m_lecturerlevelEncoderIndex                       = (WORD)-1;
  m_pCopLectureModeCntl                             = new CCopLectureModeCntl();
  m_isChangeLayoutBecauseOfSpeakerChange            = FALSE;
  m_isPendingChangeLayoutBecauseOfSpeakerChange     = FALSE;
  m_isChangeLayoutBecauseOfLectureModeFlows         = FALSE;
  m_SendChangeLayoutAnyWay                          = NO;
  m_pCopCascadeLinkLecturerEncoderWasUpdated        = NO;
  m_pCopCascadeLinkLecturerLinkOutParamsWereUpdated = NO;
  m_copEncoderIndexOfCascadeLinkLecturer            = (WORD)-1;

  memset(m_pCopRsrcs, 0, sizeof(CopRsrcsParams));
}

////////////////////////////////////////////////////////////////////////////
CVideoBridgeCOP::~CVideoBridgeCOP ()
{
  for (WORD decoder_index = 0; decoder_index < NUM_OF_COP_DECODERS; decoder_index++)
    POBJDELETE(m_pCopDecoders[decoder_index]);

  for (WORD encoder_index = 0; encoder_index < NUM_OF_LEVEL_ENCODERS; encoder_index++)
    POBJDELETE(m_pCopLevelEncoders[encoder_index]);

  for (int i = 0; i < (int)CP_NO_LAYOUT; i++)
    if (IsValidPObjectPtr(m_pReservation[i]))
      POBJDELETE(m_pReservation[i]);

  POBJDELETE(m_ConfLayout1x1);
  POBJDELETE(m_pPCMEncoderCntl);
  POBJDELETE(m_pCopDecoderLecturer);
  POBJDELETE(m_pCopDecoderVsw);
  POBJDELETE(m_pCopEncoderVsw);
  POBJDELETE(m_pVisualEffects);
  POBJDELETE(m_pPcmFeccVisualEffects);
  POBJDELETE(m_pCopRsrcs);
  POBJDELETE(m_pDisconnectingPartiesInSwitchVector);
  POBJDELETE(m_pCopLectureModeCntl);
  POBJDELETE(m_changeLayoutActions);
}

////////////////////////////////////////////////////////////////////////////
WORD CVideoBridgeCOP::IsValidEvent(OPCODE event) const
{
  WORD valid_event = TRUE;
  switch (event)
  {
    case EXPORTPARTY:
    case ENDEXPORTPARTY:
    {
      valid_event = FALSE;
      break;
    }
  }
  return valid_event;
}

////////////////////////////////////////////////////////////////////////////
const char* CVideoBridgeCOP::NameOf() const
{
  return "CVideoBridgeCOP";
}

////////////////////////////////////////////////////////////////////////////
void* CVideoBridgeCOP::GetMessageMap()
{
  return (void*)m_msgEntries;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::Create(const CVideoBridgeInitParams* pVideoBridgeInitParams)
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::Create");

  // validate params
  DWORD validation_status = ValidateInitParams(pVideoBridgeInitParams);
  if (STATUS_OK != validation_status)
  {
    PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::Create illegal params, status = ", validation_status);
    PASSERT(validation_status);
    return;
  }

  // temp Eitan
  m_openRequests = 0;
  PrintVideoBridgeInitParams(pVideoBridgeInitParams);

  // Create CBridge params - m_pConfApi and allocates parties list
  CBridge::Create((CBridgeInitParams*)pVideoBridgeInitParams);
  // keep COP resources for create of decoders
  *m_pCopRsrcs = *(pVideoBridgeInitParams->GetCopRsrcsParams());

  // Initiate aplication params (m_IsSameLayout,m_pLectureModeParams,m_videoQuality) - override behaviour of CVideoBridge::Create
  InitApplicationParams(pVideoBridgeInitParams);

  // Initiate conf level forces and auto layout
  InitLayoutsReservation(pVideoBridgeInitParams);

  // Create COP level encoders and COP decoders
  SetVideoConfTypeFromFirstEncoder(pVideoBridgeInitParams); // must be before CreateLevelEncoders to initiate m_VideoConfType

  CreateLevelEncoders(pVideoBridgeInitParams);

  m_pcmLevelIndex = 0;
  CreatePcmEncoder(pVideoBridgeInitParams);
  m_pcmMenuId     = m_pCopRsrcs->pcmMenuId;

  ConnectLevelEncodes();
  StartTimer(CONNECT_COP_VB_TOUT, CONNECT_COP_VB_TIME_OUT_VALUE);
  m_pPCMEncoderCntl->Connect(NO /*isIvr*/);

  *m_pSiteNameInfo =*(pVideoBridgeInitParams->GetCommConf()->GetSiteNameInfo());
  *m_pMessageOverlayInfo = *(pVideoBridgeInitParams->GetCommConf()->GetMessageOverlay());
}

////////////////////////////////////////////////////////////////////////////
DWORD CVideoBridgeCOP::ValidateInitParams(const CVideoBridgeInitParams* pVideoBridgeInitParams)
{
  PASSERT_AND_RETURN_VALUE(!CPObject::IsValidPObjectPtr(pVideoBridgeInitParams), statInconsistent);

  // can add here validation of encoders information in pVideoBridgeInitParams
  return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::InitApplicationParams(const CVideoBridgeInitParams* pVideoBridgeInitParams)
{
  // same as in CP
  // need to check lacture mode and same layout
  m_IsSameLayout = (pVideoBridgeInitParams->GetIsSameLayout()) ? YES : NO;

  *m_pLectureModeParams = *(pVideoBridgeInitParams->GetLectureModeParams());
  if (m_pLectureModeParams->GetLectureModeType() == eLectureModePresentation)
  {
    TRACEINTO << "CVideoBridgeCOP::InitApplicationParams - illegal LectureModeType, 'eLectureModePresentation' is not supported in COP, set LectureModeType to none";
    m_pLectureModeParams->SetLectureModeType(eLectureModeNone);
  }

  TRACEINTO << "CVideoBridgeCOP::InitApplicationParams lecture Mode params:\n"<<  *m_pLectureModeParams;
  if (eLmAction_LecturerStartAudioActivated == m_pCopLectureModeCntl->GetLectureModeActionNew(*m_pLectureModeParams))
  {
    m_pLectureModeParams->SetAudioActivatedLectureMode(eAudioActivatedRegular);
    m_pCopLectureModeCntl->UpdateCurrentParams(*m_pLectureModeParams);
  }

  UpdateConfDBLectureMode();
  m_videoQuality       = pVideoBridgeInitParams->GetVideoQuality();
  m_isSiteNamesEnabled = pVideoBridgeInitParams->GetIsSiteNamesEnabled();

  CCommConf* pCommConf = pVideoBridgeInitParams->GetCommConf();
  if (IsValidPObjectPtr(pCommConf))
    m_pAutoScanParams->SetTimerInterval(pCommConf->GetAutoScanInterval());
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::InitLayoutsReservation(const CVideoBridgeInitParams* pVideoBridgeInitParams)
{
  // same as in CP
  for (int i = (int)CP_LAYOUT_1X1; i < (int)CP_NO_LAYOUT; i++)
    m_pReservation[i] = new CLayout((LayoutType)i, GetConfName());

  GetForcesFromReservation(pVideoBridgeInitParams->GetCommConf());

  if (pVideoBridgeInitParams->GetIsAutoLayout())
  {
    m_pAutoLayoutSet = new CAutoLayoutSet;
    LayoutType tempLayoutType = m_pAutoLayoutSet->GetLayoutType(GetPartyImageVectorSizeUnmuted(), IsSameLayout());
    if (tempLayoutType != m_layoutType)
    {
      m_pReservation[m_layoutType]->SetCurrActiveLayout(NO);
      m_pReservation[tempLayoutType]->SetCurrActiveLayout(YES);
      m_layoutType = tempLayoutType;
    }
  }

  UpdateDB_ConfLayout();

  *m_pVisualEffects        = *(pVideoBridgeInitParams->GetVisualEffectsParams());
  *m_pPcmFeccVisualEffects = *m_pVisualEffects;
  m_pPcmFeccVisualEffects->SetUseYUVcolor(TRUE);
  m_pPcmFeccVisualEffects->SetSpeakerNotationEnable(TRUE);
  m_pPcmFeccVisualEffects->SetSpeakerNotationColorYUV(GetPcmYUVColor());
  m_isVideoClarityEnabled = pVideoBridgeInitParams->GetIsVideoClarityEnabled();
  *m_ConfLayout           = *m_pReservation[m_layoutType];
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::CreateLevelEncoders(const CVideoBridgeInitParams* pVideoBridgeCOPInitParams)
{
  for (WORD encoder_index = 0; encoder_index < NUM_OF_LEVEL_ENCODERS; encoder_index++)
  {
    // 1) init parameters that not relevant for Cop Level Encoders
    char partyName[128];                                      // init encoder name
    memset(partyName, '\0', 128);
    sprintf(partyName, "%s%d", "COP_Level_Encoder_#", encoder_index);

    const CTaskApp*                pParty            = NULL;  // no party task for COP encoder
    const WORD                     wNetworkInterface = AUTO_INTERFACE_TYPE;
    const CBridgePartyMediaParams* pMediaInParams    = NULL;
    const CBridgePartyCntl*        pBridgePartyCntl  = NULL;
    const char*                    pSiteName         = "COP_Level_Encoder";
    const BYTE                     bCascadeLinkMode  = NO;

    // cretae CBridgePartyVideoOutParams for encoder
    CBridgePartyVideoOutParams* pOutVideoParams      = new CBridgePartyVideoOutParams();

    // add video params
    WORD update_status = UpdateEncoderVideoParams(pOutVideoParams, pVideoBridgeCOPInitParams->GetCommConf(), encoder_index);
    if (update_status == FALSE)
    {
      PASSERT(111);
      continue;
    }

    PartyRsrcID partyRsrcID = (PartyRsrcID)(-1);
    // party resource id allocate for COP
    UpdateEncoderResourceParams(*pOutVideoParams, partyRsrcID, encoder_index, m_VideoConfType, /*pVideoBridgeCOPInitParams->*/ GetCopRsrcDesc(eLevelEncoderType, encoder_index));
    pOutVideoParams->SetIsSiteNamesEnabled(m_isSiteNamesEnabled);
    PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::CreateLevelEncoders m_isSiteNamesEnabled = ", m_isSiteNamesEnabled);

    CVideoBridgePartyInitParams* pVideoBridgePartyInitParams = new CVideoBridgePartyInitParams(partyName, pParty, partyRsrcID, wNetworkInterface, pMediaInParams, pOutVideoParams, pBridgePartyCntl, pSiteName, bCascadeLinkMode);

    // set bridge parameters
    pVideoBridgePartyInitParams->SetBridge(this);
    pVideoBridgePartyInitParams->SetConf(m_pConf);
    pVideoBridgePartyInitParams->SetConfRsrcID(m_confRsrcID);

    m_pCopLevelEncoders[encoder_index]->Create(pVideoBridgePartyInitParams);

    POBJDELETE(pOutVideoParams);
    POBJDELETE(pVideoBridgePartyInitParams);
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::CreateVswEncoder(const CBridgePartyVideoParams& rBridgePartyVideoParams)
{
  PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::CreateVswEncoder - m_lecturerlevelEncoderIndex=", m_lecturerlevelEncoderIndex);

  // 1) init parameters that not relevant for Cop Level Encoders
  char partyName[128];                                        // init encoder name
  memset(partyName, '\0', 128);
  sprintf(partyName, "%s", "COP_VSW_Encoder");

  const CTaskApp*                pParty            = NULL;    // no party task for COP encoder
  const WORD                     wNetworkInterface = AUTO_INTERFACE_TYPE;
  const CBridgePartyMediaParams* pMediaInParams    = NULL;
  const CBridgePartyCntl*        pBridgePartyCntl  = NULL;
  const char*                    pSiteName         = "COP_VSW_Encoder";
  const BYTE                     bCascadeLinkMode  = NO;

  // cretae CBridgePartyVideoOutParams for encoder
  CBridgePartyVideoOutParams* pOutVideoParams      = new CBridgePartyVideoOutParams();

  // add video params
  CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();
  UpdateLecturerCodecsVideoParamsFromLevel(pOutVideoParams, pCommConf, m_lecturerlevelEncoderIndex);

  PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::CreateVswEncoder - Dump of VideoOut params:");
  pOutVideoParams->DumpVideoParams();


  PartyRsrcID partyRsrcID = (PartyRsrcID)(-1);
  // party resource id allocate for COP
  UpdateEncoderResourceParams(*pOutVideoParams, partyRsrcID, VSW_ENCODER_INDEX, m_VideoConfType, /*pVideoBridgeCOPInitParams->*/ GetCopRsrcDesc(eVswEncoderType));

  CVideoBridgePartyInitParams* pVideoBridgePartyInitParams = new CVideoBridgePartyInitParams(partyName, pParty, partyRsrcID, wNetworkInterface, pMediaInParams, pOutVideoParams, pBridgePartyCntl, pSiteName, bCascadeLinkMode);

  // set bridge paramameters
  pVideoBridgePartyInitParams->SetBridge(this);
  pVideoBridgePartyInitParams->SetConf(m_pConf);
  pVideoBridgePartyInitParams->SetConfRsrcID(m_confRsrcID);

  m_pCopEncoderVsw->Create(pVideoBridgePartyInitParams);

  POBJDELETE(pOutVideoParams);
  POBJDELETE(pVideoBridgePartyInitParams);
}

////////////////////////////////////////////////////////////////////////////
WORD CVideoBridgeCOP::UpdateEncoderVideoParams(CBridgePartyVideoOutParams* pOutVideoParams, CCommConf* pCommConf, WORD encoder_index) const
{
  // get encoder configuration params
  if (!CPObject::IsValidPObjectPtr(pCommConf))
  {
    PASSERT(111);
    return FALSE;
  }

  CCOPConfigurationList* pCOPConfigurationList = pCommConf->GetCopConfigurationList();
  if (!CPObject::IsValidPObjectPtr(pCOPConfigurationList))
  {
    PASSERT(111);
    return FALSE;
  }

  CCopVideoParams* pCopEncoderLevelParams = pCOPConfigurationList->GetVideoMode(encoder_index);
  if (!CPObject::IsValidPObjectPtr(pCopEncoderLevelParams))
  {
    PASSERT(encoder_index);
    return FALSE;
  }

  // Translate of GUI params to CBridgePartyVideoOutParams in table CCopVideoModeTable
  CCopVideoModeTable videoModeTable;
  videoModeTable.GetCOPEncoderBridgePartyVideoOutParams(pCopEncoderLevelParams, *pOutVideoParams);

  return TRUE;
}

////////////////////////////////////////////////////////////////////////////
WORD CVideoBridgeCOP::UpdateLecturerCodecsVideoParamsFromLevel(CBridgePartyVideoOutParams* pOutVideoParams, CCommConf* pCommConf, WORD encoder_index) const
{
  // get encoder configuration params
  if (!CPObject::IsValidPObjectPtr(pCommConf))
  {
    PASSERT(111);
    return FALSE;
  }

  CCOPConfigurationList* pCOPConfigurationList = pCommConf->GetCopConfigurationList();
  if (!CPObject::IsValidPObjectPtr(pCOPConfigurationList))
  {
    PASSERT(111);
    return FALSE;
  }

  CCopVideoParams* pCopEncoderLevelParams = pCOPConfigurationList->GetVideoMode(encoder_index);
  if (!CPObject::IsValidPObjectPtr(pCopEncoderLevelParams))
  {
    PASSERT(111);
    return FALSE;
  }

  // Translate of GUI params to CBridgePartyVideoOutParams in table CCopVideoModeTable
  CCopVideoModeTable videoModeTable;
  videoModeTable.GetCOPLecturerCodecsVideoParamsByEncoder(pCopEncoderLevelParams, *pOutVideoParams);

  return TRUE;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::UpdateEncoderResourceParams(CBridgePartyVideoOutParams& rOutVideoParams, PartyRsrcID& partyRsrcID, DWORD encoder_index, eVideoConfType videoConfType, CopRsrcDesc encoderCopRsrcDesc) const
{
  partyRsrcID = encoderCopRsrcDesc.rsrcEntityId;
  rOutVideoParams.SetCopResourceIndex(encoder_index);
  rOutVideoParams.SetVideConfType(videoConfType);
  rOutVideoParams.SetCopLrt(encoderCopRsrcDesc.logicalRsrcType);

  TRACEINTO << "CVideoBridgeCOP::UpdateEncoderResourceParams "
            << "- PartyRsrcID:" << partyRsrcID
            << ", EncoderIndex:" << encoder_index
            << ", VideoConfType:" <<VideoConfTypeAsString[videoConfType]
            << ", LogicalResourceType:" << LogicalResourceTypeToString(encoderCopRsrcDesc.logicalRsrcType);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::CreatePcmEncoder(const CVideoBridgeInitParams* pVideoBridgeCOPInitParams)
{
  // 1) init parameters that not relevant for Cop Level Encoders
  char partyName[128];                                      // init encoder name
  memset(partyName, '\0', 128);
  sprintf(partyName, "%s", "PCM_Encoder");

  const CTaskApp*                pParty            = NULL;  // no party task for COP encoder
  const WORD                     wNetworkInterface = AUTO_INTERFACE_TYPE;
  const CBridgePartyMediaParams* pMediaInParams    = NULL;
  const CBridgePartyCntl*        pBridgePartyCntl  = NULL;
  const char*                    pSiteName         = "PCM_Encoder-site_name";
  const BYTE                     bCascadeLinkMode  = NO;

  CBridgePartyVideoOutParams* pOutVideoParams      = new CBridgePartyVideoOutParams();

  // add video params
  // m_pcmLevelIndex is 0 by default
  WORD update_status = UpdateEncoderVideoParams(pOutVideoParams, pVideoBridgeCOPInitParams->GetCommConf(), m_pcmLevelIndex);

  if (update_status == FALSE) {
    PASSERT_AND_RETURN(111);
  }

  // Pcm encoder is always 5fps
  pOutVideoParams->SetVidFrameRate(eVideoFrameRate5FPS);

  PartyRsrcID partyRsrcID = (PartyRsrcID)(-1);
  // party resource id allocate for COP
  UpdateEncoderResourceParams(*pOutVideoParams, partyRsrcID, PCM_RESOURCE_INDEX, m_VideoConfType, /*pVideoBridgeCOPInitParams->*/ GetCopRsrcDesc(ePcmEncoderType));

  CVideoBridgePartyInitParams* pVideoBridgePartyInitParams = new CVideoBridgePartyInitParams(partyName, pParty, partyRsrcID, wNetworkInterface, pMediaInParams, pOutVideoParams, pBridgePartyCntl, pSiteName, bCascadeLinkMode);

  // set bridge paramameters
  pVideoBridgePartyInitParams->SetBridge(this);
  pVideoBridgePartyInitParams->SetConf(m_pConf);
  pVideoBridgePartyInitParams->SetConfRsrcID(m_confRsrcID);

  m_pPCMEncoderCntl->Create(pVideoBridgePartyInitParams);

  POBJDELETE(pOutVideoParams);
  POBJDELETE(pVideoBridgePartyInitParams);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::ConnectLevelEncodes()
{
  BYTE isIVR = NO;
  for (DWORD encoder_index = 0; encoder_index < NUM_OF_LEVEL_ENCODERS; encoder_index++)
    m_pCopLevelEncoders[encoder_index]->Connect(isIVR);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnLevelEncoderEndConnectIDLE(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnLevelEncoderEndConnectIDLE, ConfName - ", m_pConfName);

  WORD            receivedStatus = statOK;
  EMediaDirection eMediaDirection;
  DWORD           encoderIndex;
  CTaskApp*       pParty         = NULL;
  *pParam >> (void*&)pParty;
  *pParam >> (WORD&)receivedStatus;
  *pParam >> (WORD&)eMediaDirection;
  *pParam >> (DWORD&)encoderIndex;

  if (receivedStatus != statOK)
  {
    PASSERT(receivedStatus);
    DeleteTimer(CONNECT_COP_VB_TOUT);
    m_pConfApi->EndVidBrdgConnect(receivedStatus);
  }

  PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::OnLevelEncoderEndConnectIDLE, event from encoder index: ", encoderIndex);
  if (CheckAllLevelEncodersConnected()) // maybe it should be a function that check if all the relevant resources connected
  {
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnLevelEncoderEndConnectIDLE all encoders connected, ConfName - ", m_pConfName);
    m_state = CONNECTED;
    DeleteTimer(CONNECT_COP_VB_TOUT);
    m_pConfApi->EndVidBrdgConnect(statOK);
  }

  if (encoderIndex == PCM_RESOURCE_INDEX)
    ConnectPCMEncoderToPCMMenu();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnLevelEncoderEndConnectCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnLevelEcoderEndConnectCONNECTED, ConfName - ", m_pConfName);

  WORD            receivedStatus = statOK;
  EMediaDirection eMediaDirection;
  DWORD           encoderIndex;
  CTaskApp*       pParty         = NULL;
  *pParam >> (void*&)pParty;
  *pParam >> (WORD&)receivedStatus;
  *pParam >> (WORD&)eMediaDirection;
  *pParam >> (DWORD&)encoderIndex;

  if (receivedStatus != statOK)
    PASSERT(receivedStatus);

  PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::OnLevelEcoderEndConnectCONNECTED, event from encoder index: ", encoderIndex);

  if (encoderIndex == VSW_ENCODER_INDEX)
  {
    if (IsLecturerCodecsConnected())
    {
      // move listeners from lecture level encoder to vsw encoder
      PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::OnLevelEncoderEndConnectCONNECTED lecturer codecs connected, SwitchListenersEncoder: Name - ", m_pConfName);
      EndOpenLecturerCodecs();
      // LayoutType newLayoutType  = m_ConfLayout->GetLayoutType(); // the layout type didn't change
      // ChangeLayout(newLayoutType);
    }
  }
  else if (encoderIndex == PCM_RESOURCE_INDEX)
  {
    ConnectPCMEncoderToPCMMenu();
  }
  else if (encoderIndex < PCM_RESOURCE_INDEX)
  {
    PTRACE2(eLevelInfoNormal, "Warning!!! - CVideoBridgeCOP::OnLevelEncoderEndConnectCONNECTED Ignored, we shouldn't be in the encoder connect stage in connected state, ConfName - ", m_pConfName);
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnLevelEncoderEndConnectINSWITCH(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnLevelEcoderEndConnectINSWITCH, ConfName - ", m_pConfName);

  WORD            receivedStatus = statOK;
  EMediaDirection eMediaDirection;
  DWORD           encoderIndex;
  CTaskApp*       pParty         = NULL;
  *pParam >> (void*&)pParty;
  *pParam >> (WORD&)receivedStatus;
  *pParam >> (WORD&)eMediaDirection;
  *pParam >> (DWORD&)encoderIndex;

  if (receivedStatus != statOK)
    PASSERT(receivedStatus);

  PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::OnLevelEcoderEndConnectINSWITCH, event from encoder index: ", encoderIndex);

  if (encoderIndex == VSW_ENCODER_INDEX)
  {
    if (IsLecturerCodecsConnected())
    {
      PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::OnLevelEncoderEndConnectINSWITCH lecturer codecs connected, SwitchListenersEncoder: Name - ", m_pConfName);
      EndOpenLecturerCodecs();
    }
  }

  if (encoderIndex < PCM_RESOURCE_INDEX)
    PTRACE2(eLevelInfoNormal, "Warning!!! - CVideoBridgeCOP::OnLevelEncoderEndConnectINSWITCH Ignored, we shouldn't be in the encoder connect stage in switch state, ConfName - ", m_pConfName);

  if (encoderIndex == PCM_RESOURCE_INDEX)
    ConnectPCMEncoderToPCMMenu();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnLevelEncoderEndConnectDISCONNECTING(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnLevelEncoderEndConnectDISCONNECTING, ConfName - ", m_pConfName);

  CTaskApp*       pParty         = NULL;
  WORD            receivedStatus = statOK;
  EMediaDirection eMediaDirection;
  DWORD           encoderIndex;
  *pParam >> (void*&)pParty;
  *pParam >> receivedStatus;
  *pParam >> (WORD&)eMediaDirection;
  *pParam >> (DWORD&)encoderIndex;

  if (receivedStatus != statOK)
  {
    PASSERT(receivedStatus);
    m_resourcesStatus = receivedStatus;
  }
}

////////////////////////////////////////////////////////////////////////////
WORD CVideoBridgeCOP::CheckAllLevelEncodersConnected() const
{
  WORD ret_value              = FALSE;
  WORD num_encoders_connected = 0;

  for (WORD encoder_index = 0; encoder_index < NUM_OF_LEVEL_ENCODERS; encoder_index++)
  {
    if (m_pCopLevelEncoders[encoder_index]->IsCopEncoderInConnectedState())
    {
      num_encoders_connected++;
    }
  }

  if (num_encoders_connected == NUM_OF_LEVEL_ENCODERS)
  {
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::CheckAllLevelEncodersConnected All level encoders connected, ConfName - ", m_pConfName);
    ret_value = TRUE;
  }

  return ret_value;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::PrintVideoBridgeInitParams(const CVideoBridgeInitParams* pVideoBridgeInitParams)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::PrintVideoBridgeInitParams, ConfName - ", m_pConfName);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::SetVideoConfTypeFromFirstEncoder(const CVideoBridgeInitParams* pVideoBridgeCOPInitParams)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::SetVideoConfTypeFromFirstEncoder, ConfName - ", m_pConfName);

  m_VideoConfType = eVideoConfTypeCopHD108025fps;
  CCommConf* pCommConf = pVideoBridgeCOPInitParams->GetCommConf();
  if (!CPObject::IsValidPObjectPtr(pCommConf))
  {
    PASSERT(111);
    return;
  }

  CCOPConfigurationList* pCOPConfigurationList = pCommConf->GetCopConfigurationList();
  if (!CPObject::IsValidPObjectPtr(pCOPConfigurationList))
  {
    PASSERT(112);
    return;
  }

  CCopVideoParams* pCopEncoderFirstLevelParams = pCOPConfigurationList->GetVideoMode(0);
  if (!CPObject::IsValidPObjectPtr(pCopEncoderFirstLevelParams))
  {
    PASSERT(113);
    return;
  }

  if ((pCopEncoderFirstLevelParams->GetFormat() == eCopLevelEncoderVideoFormat_HD720p) && ((pCopEncoderFirstLevelParams->GetFrameRate() == eCopVideoFrameRate_50) || (pCopEncoderFirstLevelParams->GetFrameRate() == eCopVideoFrameRate_60)))
    m_VideoConfType = eVideoConfTypeCopHD72050fps;
}


////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::InitBridgeParams(const CBridgePartyInitParams* pBridgePartyInitParams)
{
  CBridge::InitBridgeParams(pBridgePartyInitParams);
  // IF WE WILL INHERIT FROM VBCP we can call it instead of the code below!!!
  if (pBridgePartyInitParams->GetMediaOutParams())
  {
    ((CBridgePartyVideoOutParams*)pBridgePartyInitParams->GetMediaOutParams())->SetVisualEffects(m_pVisualEffects);
    ((CBridgePartyVideoOutParams*)pBridgePartyInitParams->GetMediaOutParams())->SetLayoutType(m_layoutType);

    ePartyLectureModeRole partyLectureModeRole = GetLectureModeRoleForParty(pBridgePartyInitParams->GetPartyName());
    ((CBridgePartyVideoOutParams*)pBridgePartyInitParams->GetMediaOutParams())->SetPartyLectureModeRole(partyLectureModeRole);
    ((CBridgePartyVideoOutParams*)pBridgePartyInitParams->GetMediaOutParams())->SetVideoQualityType((eVideoQuality)m_videoQuality);
    ((CBridgePartyVideoOutParams*)pBridgePartyInitParams->GetMediaOutParams())->SetIsVideoClarityEnabled(m_isVideoClarityEnabled);
    ((CBridgePartyVideoOutParams*)pBridgePartyInitParams->GetMediaOutParams())->SetIsSiteNamesEnabled(m_isSiteNamesEnabled);
  }

  if (pBridgePartyInitParams->GetMediaInParams())
  {
    ((CBridgePartyVideoInParams*)pBridgePartyInitParams->GetMediaInParams())->SetBackgroundImageID(m_pVisualEffects->GetBackgroundImageID());
    ((CBridgePartyVideoInParams*)pBridgePartyInitParams->GetMediaInParams())->SetIsVideoClarityEnabled(m_isVideoClarityEnabled);
  }

  // take encoder index from party cntl decission and update specific encoder params
  CBridgePartyInitParams partyInitParams = CBridgePartyInitParams(*pBridgePartyInitParams);

  // take encoder index from party cntl decission and update specific encoder params
  UpdatePartyCopEncoderParams(partyInitParams);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfConnectPartyCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfConnectPartyCONNECTED, ConfName - ", m_pConfName);

	CBridgePartyInitParams partyInitParams;
	partyInitParams.DeSerialize(NATIVE, *pParam);

	// check init params validity - from CVideoBridge
	if (!partyInitParams.IsValidParams())
	{
		if (CorrectPartyInitParams(partyInitParams) == FALSE)
		{
			PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfConnectPartyCONNECTED : Video Params - Params Invalid!!! ConfName - ", m_pConfName);
			CBridge::RejectPartyConnect(partyInitParams.GetParty(), PARTY_VIDEO_DISCONNECTED, statInvalidPartyInitParams);
			return;
		}
	}

	if (!partyInitParams.IsValidCopParams())
	{
		PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfConnectPartyCONNECTED : Video Params - COP Params Invalid!!! ConfName - ", m_pConfName);
		CBridge::RejectPartyConnect(partyInitParams.GetParty(), PARTY_VIDEO_DISCONNECTED, statInvalidPartyInitParams);
		return;
	}

	if (IsActiveLectureMode() && partyInitParams.GetMediaOutParams())
	{
		if (IsPartyUsesLecturerEncoder(partyInitParams))
		{
			SetVswEncoderParams(partyInitParams);
		}
	}

	partyInitParams.SetSiteNameInfo(m_pSiteNameInfo);

	const CParty* pParty = (CParty*)partyInitParams.GetParty();
	PartyRsrcID partyId = pParty->GetPartyRsrcID();

	CVideoBridgePartyCntl* pVideoBrdgPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);

	// If this party exist in this conf...
	// We don't need to create new VideoBridgePartyCntl - only add or update one of the directions -VideoIn or VideoOut
	if (pVideoBrdgPartyCntl)
	{
		PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfConnectPartyCONNECT: Connect existing party!!!: ConfName - ", m_pConfName);
		pVideoBrdgPartyCntl->Update((CVideoBridgePartyInitParams*)(&partyInitParams));
		pVideoBrdgPartyCntl->Connect(partyInitParams.IsIvrInConf());
	}
	else
	{
		// If move from other conf....
		if (NULL != partyInitParams.GetPartyCntl())
		{
			PASSERT(111);
			PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfConnectPartyCONNECTED : COP video bridge does not support move, ConfName - ", m_pConfName);
			CBridge::RejectPartyConnect(partyInitParams.GetParty(), PARTY_VIDEO_DISCONNECTED, statInvalidPartyInitParams);
			return;
		}
		else // NOT In MOVE
		{
			PTRACE2(eLevelInfoNormal, "CVideoBridge::OnConfConnectPartyCONNECTED: Creating New VideoBridgePartyCntl!!!: ConfName - ", m_pConfName);

			// To be able to create a list of LegacyPartyCntl within RegularPartyCntl for the LegacyContentBridge
			pVideoBrdgPartyCntl = new CVideoBridgePartyCntlCOP();
			pVideoBrdgPartyCntl->Create((CVideoBridgePartyInitParams*)&partyInitParams);

			// Insert the party to the PartyCtl List and activate Connect on it
			ConnectParty(pVideoBrdgPartyCntl, partyInitParams.IsIvrInConf());
		}
	}
}

////////////////////////////////////////////////////////////////////////////
BYTE CVideoBridgeCOP::IsPartyUsesLecturerEncoder(CBridgePartyInitParams& partyInitParams) const
{
  BYTE IsPartyUsesLecturerEncoder                     = NO;
  CBridgePartyVideoParams* pBridgePartyVideoOutParams = (CBridgePartyVideoParams*)(partyInitParams.GetMediaOutParams());
  DWORD partyCopEncoderConnectionId                   = pBridgePartyVideoOutParams->GetCopConnectionId();
  DWORD partyCopEncoderPartyId                        = pBridgePartyVideoOutParams->GetCopPartyId();
  DWORD lecturerCopEncoderPartyId                     = m_pCopRsrcs->constRsrcs[m_lecturerlevelEncoderIndex].rsrcEntityId;
  DWORD lecturerCopEncoderConnectionId                = m_pCopRsrcs->constRsrcs[m_lecturerlevelEncoderIndex].connectionId;

  TRACEINTO << "CVideoBridgeCOP::IsPartyUsesLecturerToEncoder "
            << "- party_encoder_party_id:" << partyCopEncoderPartyId
            << ", party_encoder_connection_id:" << partyCopEncoderConnectionId
            << ", lecturer_encoder_party_id:" << lecturerCopEncoderPartyId
            << ", lecturer_encoder_connection_id:" << lecturerCopEncoderConnectionId;

  if (partyCopEncoderPartyId == lecturerCopEncoderPartyId && partyCopEncoderConnectionId == lecturerCopEncoderConnectionId)
  {
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::IsPartyUsesLecturerToEncoder : level encoder used be lecturer - party will connect to VSW encoder, ConfName - ", m_pConfName);
    IsPartyUsesLecturerEncoder = YES;
  }

  return IsPartyUsesLecturerEncoder;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::SetVswEncoderParams(CBridgePartyInitParams& partyInitParams)
{
  // get vsw encoder id
  DWORD vswEncoderPartyId      = m_pCopRsrcs->constRsrcs[eVswEncoder].rsrcEntityId;
  DWORD vswEncoderConnectionId = m_pCopRsrcs->constRsrcs[eVswEncoder].connectionId;
  TRACEINTO << "CVideoBridgeCOP::SetVswEncoderParams "
            << "- vsw_encoder_party_id:" << vswEncoderPartyId
            << ", vsw_encoder_connection_id:" << vswEncoderConnectionId;

  // set it to party
  CBridgePartyVideoParams* pBridgePartyVideoOutParams = (CBridgePartyVideoParams*)(partyInitParams.GetMediaOutParams());
  pBridgePartyVideoOutParams->SetCopConnectionId(vswEncoderConnectionId);
  pBridgePartyVideoOutParams->SetCopPartyId(vswEncoderPartyId);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfConnectPartyINSWITCH(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridge::OnConfConnectPartyINSWITCH, ConfName - ", m_pConfName);
  OnConfConnectPartyCONNECTED(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgeCOP::OnEndPartyConnectCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CVideoBridgeCOP::OnEndPartyConnectCONNECTED : ConfName - ",m_pConfName);

  CSegment seg(*pParam);
  SendMessageOverlayToNewConnectedPartyIfNeeded(&seg);

  BYTE updatePartyDB = YES;
  OnEndPartyConnect(pParam, updatePartyDB);
}

//--------------------------------------------------------------------------
void CVideoBridgeCOP::OnEndPartyConnect(CSegment* pParam, BYTE updatePartyDB)
{
	PartyRsrcID partyId;
	WORD status;
	EMediaDirection eMediaDirection = eNoDirection;
	CSegment* pTempParam = new CSegment(*pParam);
	*pTempParam >> partyId >> status >> (WORD&)eMediaDirection;
	POBJDELETE(pTempParam)

	TRACEINTO << "PartyId:" << partyId << ", MediaDirection:" << eMediaDirection << ", status:" << status;

	CVideoBridgePartyCntlCOP* pPartyCntl = (CVideoBridgePartyCntlCOP*)GetPartyCntl(partyId);
	PASSERT_AND_RETURN(!pPartyCntl);

	if (eMediaDirection != eMediaIn) // only in connected
	{
		DWORD levelEncoderEntityId = pPartyCntl->GetCopEncoderEntityId();
		WORD levelEncoderIndex = GetCopEncoderIndexFromEntityId(levelEncoderEntityId);

		if (IsLecturer(pPartyCntl))
		{
			PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::OnEndPartyConnect Lecturer connected, levelEncoderIndex = ", levelEncoderIndex);
			// TODO maybe we need different behavior for cascade link
			if (levelEncoderIndex <= NUM_OF_LEVEL_ENCODERS - 1)
			{
				m_lecturerlevelEncoderIndex = levelEncoderIndex;
			}
			else
			{
				PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::OnEndPartyConnect Lecturer connected, illegal levelEncoderIndex = ", levelEncoderIndex);
			}
		}

		if (levelEncoderIndex <= NUM_OF_LEVEL_ENCODERS - 1)
		{
			m_pCopLevelEncoders[levelEncoderIndex]->FastUpdate();
			WORD dbLevelEncoderIndex = levelEncoderIndex + 1;
			pPartyCntl->UpdateLevelEncoderInDB(dbLevelEncoderIndex);

			// update the party layout in DB according to the encoder it's connected to
			CLayout* layout = m_pCopLevelEncoders[levelEncoderIndex]->GetCopEncoderLayout();
			if (updatePartyDB)
				UpdateCopPartyLayoutInDB(layout, pPartyCntl);
			else
			{
				PTRACE2(eLevelError, "CVideoBridgeCOP::OnEndPartyConnect we wont update the party layout in DB now, at the end of the switch, Name - ", m_pConfName);
			}
		}
		else if (levelEncoderIndex == VSW_ENCODER_INDEX)
		{
			PTRACE2(eLevelError, "CVideoBridgeCOP::OnEndPartyConnect Ask fast update from lecturer, Name - ", m_pConfName);
			FastUpdateFromLecturer();
			WORD dbLevelEncoderIndex = m_lecturerlevelEncoderIndex + 1;
			pPartyCntl->UpdateLevelEncoderInDB(dbLevelEncoderIndex);
			// update the party layout in DB according to the encoder it's connected to
			CLayout* layout = m_pCopEncoderVsw->GetCopEncoderLayout();
			if (updatePartyDB)
				UpdateCopPartyLayoutInDB(layout, pPartyCntl);
			else
			{
				PTRACE2(eLevelError, "CVideoBridgeCOP::OnEndPartyConnect we wont update the party layout in DB now, at the end of the switch, Name - ", m_pConfName);
			}
		}
		else
		{
			PTRACE2(eLevelError, "CVideoBridgeCOP::OnEndPartyConnect invalid levelEncoderIndex - not sending fast update, Name - ", m_pConfName);
			PASSERT(levelEncoderIndex);
		}
	}

	// this function is called twice when video connected with IVR
	CBridge::EndPartyConnect(pParam, PARTY_VIDEO_CONNECTED);

	// will initiate change layout
	if (status == STATUS_OK)
	{
		if ((eMediaDirection == eMediaIn) || (eMediaDirection == eMediaInAndOut))
		{
			// we add party to conf mix after in is connected
			bool bPartyImageExistInImageVector = IsPartyImageExistInImageVector(pPartyCntl->GetPartyRsrcID());
			if (!bPartyImageExistInImageVector)
			{
				AddPartyToConfMix(pPartyCntl->GetPartyTaskApp());
			}
			else
			{
				// In case the party is already in the Image vector don't add it.
				// VNGR-18115 In case of mute video and release video in VVX the video in is connected and we reconnect the out.
				// In this case we receive that both the in and out are connected but we dont need to add the party to conf mix cause it is already in the vecor.
				DWORD partyArtId = pPartyCntl->GetPartyRsrcID();
				PTRACE2INT(eLevelError, "CVideoBridgeCOP::OnEndPartyConnect party already in vector of seen images  we wont add to mix, party ART Id: ", partyArtId);
			}
		}
	}
	else
		PTRACE2(eLevelError, "CVideoBridgeCOP::OnEndPartyConnect status not OK  we wont add to mix, Name - ", m_pConfName);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnEndPartyConnectINSWITCH(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnEndPartyConnectINSWITCH : ConfName - ", m_pConfName);
  BYTE updatePartyDB = NO; // VNGR-18251 only at the end of the switch we will update the party layout in the DB
  OnEndPartyConnectCONNECTED(pParam);
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCOP::AddPartyToConfMix(const CTaskApp* pParty)
{
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pParty));

	PartyRsrcID partyId = ((CParty*)pParty)->GetPartyRsrcID();

	CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
	PASSERT_AND_RETURN(!pPartyCntl);

	const CImage* pImage = pPartyCntl->GetPartyImage();
	PASSERT_AND_RETURN(!pImage);

	AddPartyImage(partyId);

	// bug fix, in case the same party in disconnect and connects during the same switch we will remove it from the disconnecting vector
	RemovePartyFromDisconnectingVectorIfExists(partyId);

	if (!pImage->isMuted())
	{
		ApplicationActionsOnAddPartyToMix(pPartyCntl);

		// change conf COP layout
		if (!IsLecturer(pPartyCntl))
		{
			ChangeLayout(m_layoutType);
		}
		else
			PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::AddPartyToConfMix : lecturer connected, change layout only after open decoders, ConfName - ", m_pConfName);
	}

	// from CP
	// if the party that connects is the last video speaker
	// we will send video speaker again
	if (pPartyCntl->GetPartyTaskApp() == m_pLastActiveVideoSpeakerRequest && !IsLecturer(pPartyCntl))
	{
		ResendLastActiveVideoSpeakerRequest();
	}
}


////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::UpdatePartyCopEncoderParams(CBridgePartyInitParams& partyInitParams)
{
  CBridgePartyVideoOutParams* pOutVideoParams = (CBridgePartyVideoOutParams*)partyInitParams.GetMediaOutParams();
  if (pOutVideoParams != NULL)
  {
    WORD        encoder_index     = pOutVideoParams->GetCopResourceIndex();
    CopRsrcDesc encoderCopRsrcDes = GetCopRsrcDesc(eLevelEncoderType, encoder_index);

    pOutVideoParams->SetCopConnectionId(encoderCopRsrcDes.connectionId);
    pOutVideoParams->SetCopPartyId(encoderCopRsrcDes.rsrcEntityId);
  }
  partyInitParams.SetConfRsrcID(m_confRsrcID);
}

////////////////////////////////////////////////////////////////////////////
CopRsrcDesc CVideoBridgeCOP::GetCopRsrcDesc(ECopRsrcType copRsrcType, WORD index) const
{
  DWORD                 connectionId    = DUMMY_CONNECTION_ID; // DWORD(-1);
  eLogicalResourceTypes logicalRsrcType = eLogical_res_none;
  DWORD                 rsrcEntityId    = DUMMY_PARTY_ID; // DWORD(-1);
  CopRsrcDesc           copRsrcDesc     = {connectionId, logicalRsrcType, rsrcEntityId};

  if (m_pCopRsrcs == NULL)
  {
    PASSERT(111);
    return copRsrcDesc;
  }

  switch (copRsrcType)
  {
    case eLevelEncoderType:
    {
      if (index > NUM_OF_LEVEL_ENCODERS -1)
      {
        PASSERT(index);
        break;
      }

      WORD level_encoder_index = index +  eEncoderLevel1;
      copRsrcDesc =  m_pCopRsrcs->constRsrcs[level_encoder_index];
      break;
    }

    case ePcmEncoderType:
    {
      copRsrcDesc =  m_pCopRsrcs->constRsrcs[ePcmEncoder];
      break;
    }

    case eVswEncoderType:
    {
      copRsrcDesc =  m_pCopRsrcs->constRsrcs[eVswEncoder];
      break;
    }

    case eVswDecoderType:
    {
      copRsrcDesc =  m_pCopRsrcs->constRsrcs[eVswDecoder];
      break;
    }

    case eLectureDecoderType:
    {
      copRsrcDesc =  m_pCopRsrcs->constRsrcs[eLectureDecoder];
      break;
    }

    case eDynamicDecoderType:
    {
      if (index > NUM_OF_COP_DECODERS -1)
      {
        PASSERT(index);
        break;
      }

      copRsrcDesc =  m_pCopRsrcs->dynamicDecoders[index];
      break;
    }

    default:
    {
      if (copRsrcType == 0)
      {
        PASSERT(111);
      }
      else
      {
        PASSERT(copRsrcType);
      }

      break;
    }
  } // switch

  return copRsrcDesc;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::ChangeLayout(LayoutType newLayoutType, BOOL isChangeLayoutBecauseOfRemoveParty, BYTE always_send)
{
  WORD isLectureModeActive = m_pCopLectureModeCntl->IsLectureModeActive();
  BOOL isChangeLecturerNeeded = FALSE;

  std::ostringstream msg;
  msg << "CVideoBridgeCOP::ChangeLayout "
      << "- NewLayoutType="     << LayoutTypeAsString[newLayoutType]
      << ", IsLectureMode="     << (isLectureModeActive ? "YES" : "NO")
      << ", VideoBridgeState="  << ((m_state == DISCONNECTING) ? "DISCONNECTING" : (m_state == CONNECTED) ? "CONNECTED" : (m_state == INSWITCH) ? "INSWITCH" : "UNDEFINED")
      << ", ConfName="          << m_pConfName;

  if (m_state == DISCONNECTING)
  {
    msg << ", Action=Ignored because received during DISCONNECTING state";
    PTRACE(eLevelInfoNormal, msg.str().c_str());
    return;
  }

  if (m_state == INSWITCH)
  {
    msg << ", Action=Ignored because received during INSWITCH state - set switch in switch";
    PTRACE(eLevelInfoNormal, msg.str().c_str());
    m_SwitchInSwitchLayoutType = newLayoutType;
    m_SwitchInSwitch           = YES;
    return;
  }

  if (m_pCopLectureModeCntl->IsInAction())
  {
    msg << ", Action=Ignored because received during lecture mode action - set switch in switch";
    PTRACE(eLevelInfoNormal, msg.str().c_str());
    // Although we are not in switch we set the SwitchInSwitch and the new layout type will be updated after the end of the lecture mode action change layoyt
    m_SwitchInSwitchLayoutType = newLayoutType;
    m_SwitchInSwitch           = YES;
    return;
  }

  BYTE waitingToLecturerDecoderSync = m_pCopLectureModeCntl->GetWaitingToLecturerDecoderSync();
  if (waitingToLecturerDecoderSync)
  {
    msg << ", Action=Ignored because received during wait for lecturer decoder sync - we will send the 1x1 layout to listeners before we start a new change layout";
    SendChangeLayoutOnLecturerDecoderSync();
  }

  // Tsahi - VNGFE-4916 - If smart lecturer mechanism is active, and the new layout type is not 1X1 or AutoScan is enabled,
  // then we need to clean the smart lecturer state (set it to eLmSmartState_None) and call ChangeLecturer, in order to
  // reconnect decoders/encoders with eLmSmartState_None which will not use the smart lecturer mechanism.
  if (newLayoutType != CP_LAYOUT_1X1 || m_pReservation[newLayoutType]->IsAutoScanSet())
  {
    if (newLayoutType != CP_LAYOUT_1X1)
    {
      msg << ", \nAction='Smart Lecturer' switching mechanism disabled because layout not 1x1";
    }
    else if (m_pReservation[newLayoutType]->IsAutoScanSet())
    {
      msg << ", \nAction='Smart Lecturer' switching mechanism disabled because conference layout has autoscan images";
    }

    if (m_SmartLecturerState.m_eState != eLmSmartState_None)
    {
      isChangeLecturerNeeded = TRUE;
    }

    m_SmartLecturerState.Clean();

    if (isChangeLecturerNeeded)
    {
      PTRACE(eLevelInfoNormal, msg.str().c_str());

      // Reconnect decoders/encoders with eLmSmartState_None which will not use the smart lecturer mechanism.
      ChangeLecturer(*m_pLectureModeParams, FALSE);

      return;
    }
  }

  if (m_SmartLecturerState.IsSmartLevelState())
  {
    // SMART_SWITCH_LECTURER_1x1: In this mode we never rebuild layouts (never change images in ConfLayout and Layout1x1),
    // since in this mode ConfLayout never detached from "dynamic decoder" and Layout1x1 never detached from "lecturer decoder".
    // So, in this mode we just perform "EndChangeLayoutActions".
    msg << ", \nAction=Ignored because received during the SmartLecturerState=" << m_SmartLecturerState.GetStateName(eLmSmartState_Current);
    PTRACE(eLevelInfoNormal, msg.str().c_str());
    EndChangeLayoutActions();
    *m_pLectureModeParams = *(m_pCopLectureModeCntl->GetCurrentParams());

    return;
  }

  PTRACE(eLevelInfoNormal, msg.str().c_str());

  DeleteTimer(AUTO_SCAN_TIMER);
  CLayout* pNewLayout  = new CLayout(newLayoutType, GetConfName());
  BuildLayout(*pNewLayout, newLayoutType);
  DWORD isLayoutChange = IsLayoutChange(pNewLayout, m_ConfLayout);

  CLayout oldConfLayout1x1(*m_ConfLayout1x1);

  // lecture mode
  DWORD isLayout1x1Change = LAYOUT_NOT_CHANGED;
  if (isLectureModeActive)
  {
	CLayout* pNewLayout1x1 = new CLayout(CP_LAYOUT_1X1, GetConfName());
	BuildLayout1x1(*pNewLayout1x1);
	isLayout1x1Change = IsLayoutChange(pNewLayout1x1, m_ConfLayout1x1);
	if (isLayout1x1Change == LAYOUT_CHANGED)
	{
	  std::ostringstream msg;
	  msg << "\nThe new layout 1x1 is:";
	  pNewLayout1x1->Dump(msg);
	  msg << "\nThe previous layout 1x1 is:";
	  m_ConfLayout1x1->Dump(msg);

	  PTRACE2(eLevelInfoNormal, "CCVideoBridgeCOP::ChangeLayout, Layout=E_VIDEO_LAYOUT_1X1, isLectureModeActive=YES", msg.str().c_str());

	  *m_ConfLayout1x1 = *pNewLayout1x1;
	  m_ConfLayout1x1->SetCurrActiveLayout(YES);
	}

	POBJDELETE(pNewLayout1x1);
  }
  else
  {
    PTRACE(eLevelInfoNormal, "CCVideoBridgeCOP::ChangeLayout, Layout=E_VIDEO_LAYOUT_1X1, isLectureModeActive=NO");
    if (m_ConfLayout1x1->isActive())
    {
      m_ConfLayout1x1->SetCurrActiveLayout(NO);
      isLayout1x1Change = LAYOUT_CHANGED;
      PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::ChangeLayout Layout1x1 - end lecture mode, LAYOUT_CHANGED, ConfName=", m_pConfName);
    }
  }

  if (always_send)
  {
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::ChangeLayout - always_send is YES => setting LAYOUT_CHANGED, ConfName=", m_pConfName);
    isLayoutChange           = LAYOUT_CHANGED;
    m_SendChangeLayoutAnyWay = YES;
  }

  // VNGR-13667 If lecture mode has started / ended and the new layout is 1x1,
  // there are cases that the same image is in the layout but with different decoder id --> we will send the change layout anyway
  if (m_ConfLayout1x1->isActive() && (*m_ConfLayout1x1 == *m_ConfLayout || oldConfLayout1x1 == *pNewLayout))
  {
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::ChangeLayout - m_ConfLayout1x1 is active (lecture mode) and it is the same as old conf layout (same parties, different decoders)--> set m_SendChangeLayoutAnyWay to YES, ConfName=", m_pConfName);
    m_SendChangeLayoutAnyWay = YES;
  }

  std::ostringstream stm;
  stm << "CVideoBridgeCOP::ChangeLayout ";
  if (isLayoutChange    == LAYOUT_NOT_CHANGED        ) stm << "- isLayoutChange=LAYOUT_NOT_CHANGED";
  if (isLayoutChange    == LAYOUT_CHANGED            ) stm << "- isLayoutChange=LAYOUT_CHANGED";
  if (isLayoutChange    == LAYOUT_CHANGED_ATTRIBUTES ) stm << "- isLayoutChange=LAYOUT_CHANGED_ATTRIBUTES";
  if (isLayoutChange    == LAYOUT_CHANGED_AUTOSCAN   ) stm << "- isLayoutChange=LAYOUT_CHANGED_AUTOSCAN";
  if (isLayout1x1Change == LAYOUT_NOT_CHANGED        ) stm << ", isLayout1x1Change=LAYOUT_NOT_CHANGED";
  if (isLayout1x1Change == LAYOUT_CHANGED            ) stm << ", isLayout1x1Change=LAYOUT_CHANGED";
  if (isLayout1x1Change == LAYOUT_CHANGED_ATTRIBUTES ) stm << ", isLayout1x1Change=LAYOUT_CHANGED_ATTRIBUTES";
  if (isLayout1x1Change == LAYOUT_CHANGED_AUTOSCAN   ) stm << ", isLayout1x1Change=LAYOUT_CHANGED_AUTOSCAN";
  stm << ", ConfName=" << m_pConfName;
  PTRACE(eLevelInfoNormal, stm.str().c_str());

  if (isLayoutChange == LAYOUT_NOT_CHANGED)
  {
    if (isLayout1x1Change == LAYOUT_CHANGED)
    {
      EndChangeLayoutActions();
    }
    else
    {
      InformACLayoutChangeCompleted();
      // VNGR-12946
      UpdateDB_ConfLayout();
      DoPendingLectureModeActionOnEndOfChangeLayout();
      m_isChangeLayoutBecauseOfLectureModeFlows = FALSE;
    }
  }
  else if (isLayoutChange == LAYOUT_CHANGED_AUTOSCAN)
  {
    UpdateConfLayout(pNewLayout);
    InformACLayoutChangeCompleted();
    UpdateDB_ConfLayout();
    DoPendingLectureModeActionOnEndOfChangeLayout();
    m_isChangeLayoutBecauseOfLectureModeFlows = FALSE;
  }
  else if (isLayoutChange == LAYOUT_CHANGED_ATTRIBUTES)
  {
    if (isLayout1x1Change == LAYOUT_CHANGED)
    {
      EndChangeLayoutActions();
    }

    UpdateConfLayout(pNewLayout);
    if (!isLectureModeActive)
      SendLayoutAttributesToEncoders();
    else
    {
      SendLayoutAttributesToLecturerEncoder();
      if (isLayout1x1Change == LAYOUT_CHANGED_ATTRIBUTES)
        SendLayoutAttributesToListenersEncoders();
    }

    InformACLayoutChangeCompleted();
    // VNGR-12946
    UpdateDB_ConfLayout();
    DoPendingLectureModeActionOnEndOfChangeLayout();
    m_isChangeLayoutBecauseOfLectureModeFlows = FALSE;
  }
  else // layout changed
  {
    m_state = INSWITCH;

    // calculate change layout actions
    CalculateChangeLayoutActions(*pNewLayout, *m_ConfLayout); // set into ChangeLayoutActionsManager m_changeLayoutActionsManager

    DWORD smartSwitchTout             = 0;
    WORD  closeDisconnectDecoderIndex = NUM_OF_COP_DECODERS;
    if (IsSmartSwitchNeeded(pNewLayout, m_ConfLayout, closeDisconnectDecoderIndex))
    {
      PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::ChangeLayout -  IsSmartSwitchNeeded TRUE, ConfName=", m_pConfName);
      m_IsSmartSwitch = TRUE;
      smartSwitchTout = GetSmartSwitchTout();
    }

    StartSwitchTimer();

    UpdateConfLayout(pNewLayout);

    // execute change mode for party and decoders
    if (m_IsSmartSwitch)
    {
      StartSmartSwitch(smartSwitchTout, closeDisconnectDecoderIndex);
    }
    else
    {
      if (m_IsDSPSmartSwitch)
      {
        PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::ChangeLayout - StartDSPSmartSwitch, ConfName=", m_pConfName);
        StartDSPSmartSwitch();
        m_IsSmartSwitch = FALSE;
      }
      else
      {
        StartSwitch();
      }
    }
  }

  POBJDELETE(pNewLayout);
}


////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::BuildLayout1x1(CLayout& rResultLayout)
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::BuildLayout1x1");

  WORD isLectureMode = m_pCopLectureModeCntl->IsLectureModeActive();

  // set layout type
  rResultLayout.SetLayoutType(CP_LAYOUT_1X1);

  // Remove old forces and blanks
  RemoveOldConfForcesAndBlankes(rResultLayout);
  // forces and blanks

  if (isLectureMode)
  {
    PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::BuildLayout1x1 lecture mode ON");
    ForceLecturer(rResultLayout);
  }
  else
  {
    PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::BuildLayout1x1 lecture mode OFF");
    SetConfForces(rResultLayout, CP_LAYOUT_1X1);
  }

  // Fill Layout
  FillLayout(rResultLayout, NO);

  // update decoders connection id and entity id in images
  if (isLectureMode)
  {
    UpdateLectureDecoderParamsInImage(rResultLayout);
  }
  else
  {
    UpdateDecodersParamsInImages(rResultLayout);
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::CalculateChangeLayoutActions(CLayout& newLayout, CLayout& oldLayout)
{
  std::ostringstream msg;
  msg.setf(ios::left, ios::adjustfield);
  msg.setf(ios::showbase);

  msg << "\nnewLayout: \n";
  newLayout.Dump(msg);

  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::CalculateChangeLayoutAction ", msg.str().c_str());

  // Reset the actions table before starting to calculate the new ones:
  m_changeLayoutActions->ResetActionTable();

  CManDefinedString cstr(ONE_LINE_BUFFER_LEN*5*NUM_OF_COP_DECODERS);
  cstr << "+-----------+---------+-----------+-----------+-----------------------------------+-----------------------------------------------------------------------------+--------------------------------+" << "\n"
       << "| decoder   | decoder | decoder   | decoder   |        old party params           |                            new party params                                 |                                |" << "\n"
       << "| index     | PartyId | index in  | index in  | (the party that was connected     |           (the party to be connected to this decoder in new layout)         |                                |" << "\n"
       << "|           |         |old layout | new layout|  to this decoder in prev layout)  |         |           | current res |required res | initial res | selected    |          Actions               |" << "\n"
       << "|           |         |           |           | PartyId | algorithm |  resolution | PartyId | algorithm |             |(from table) | (party caps)| resolution  |                                |" << "\n"
       << "+-----------+---------+-----------+-----------+---------+-----------+-------------+---------+-----------+-------------+-------------+-------------+-------------+--------------------------------+" << "\n";

  BYTE isNeedToConsiderOutRes = NO;
  if (newLayout.GetLayoutType() == CP_LAYOUT_1X1 && IsActiveLectureMode())
  {
    PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::CalculateChangeLayoutActions The layout is 1x1 and lecture mode");
    isNeedToConsiderOutRes = YES;
  }

  // going over all COP decoders
  for (WORD decoder_index = 0; decoder_index < NUM_OF_COP_DECODERS; decoder_index++)
  {
    cstr << "|";

    ECopDecoderResolution oldRequiredResolution, newRequiredResolution;
    // find if (and in which cell) decoder in use in new layout
    WORD decoder_index_in_new_layout = (WORD)-1;
    decoder_index_in_new_layout = GetDecoderIndexInLayout(newLayout, decoder_index, newRequiredResolution);

    // find if (and in which cell) decoder in use in old layout
    WORD decoder_index_in_old_layout = (WORD)-1;
    decoder_index_in_old_layout = GetDecoderIndexInLayout(oldLayout, decoder_index, oldRequiredResolution);

    AddDecoderIndexToCalcStr(decoder_index, cstr);
    DWORD decoderPartyId = m_pCopRsrcs->dynamicDecoders[decoder_index].rsrcEntityId;
    AddPartyIdToCalcStr(decoderPartyId, cstr);
    AddDecoderIndexToCalcStr(decoder_index_in_old_layout, cstr);
    AddDecoderIndexToCalcStr(decoder_index_in_new_layout, cstr);
    if (decoder_index_in_new_layout != (WORD)-1)
    {
      CImage* pImage = GetPartyImage(newLayout, decoder_index_in_new_layout);
      DWORD newArtPartyId = (pImage) ? pImage->GetArtPartyId() : INVALID;
      if (newArtPartyId == INVALID)
      {
        // cstr << " PartyId | algorithm |  resolution | PartyId | algorithm |          |(from table) | (party caps)| resolution  |                  |" << "\n"
        cstr << "         |           |             | INVALID |           |             |             |             |             |                                |" << "\n";

        PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::CalculateChangeLayoutAction invalid art party id connected to decoder in new layout(decoder index: %d)", decoder_index);
        PASSERT(decoder_index);
        continue; // ??
      }

      // decoder was in use in old layout and also in use in new layout
      if (decoder_index_in_old_layout != (WORD)-1)
      {
        CImage* pImage = GetPartyImage(oldLayout, decoder_index_in_old_layout);
        DWORD oldArtPartyId = (pImage) ? pImage->GetArtPartyId() : INVALID;
        if (oldArtPartyId == INVALID)
        {
          // cstr << " PartyId | algorithm |  resolution |
          cstr << " INVALID |           |             |";

          // This case can happen when we disconnected a party during switch.
          PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::CalculateChangeLayoutAction invalid art party id connected to decoder in old layout(decoder index: %d)", decoder_index);
          CalculateChangeLayoutActionsForDecoderWhenOldArtIsNotValid(decoder_index, newArtPartyId, newRequiredResolution, cstr, isNeedToConsiderOutRes);
        }
        else
        {
          // When the party in the old layout is disconnecting we don't wont to send disconnect request to the decoder
          // but to close it so there wont be cases that the party's resources are no longer in shared memory
          // and we fail to send the disconnect request to the MPL
          CVideoBridgePartyCntlCOP* pOldPartyCntl                    = (CVideoBridgePartyCntlCOP*)GetPartyCntl(oldArtPartyId);
          BOOL                      canDisconnectDecoderWithOldParty = TRUE;
          if (IsValidPObjectPtr(pOldPartyCntl))
          {
            if (pOldPartyCntl->IsVideoInDisconnected())
              canDisconnectDecoderWithOldParty = FALSE;
          }
          else
          {
            PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::CalculateChangeLayoutAction party in old layout is not valid decoder index:", decoder_index);
            canDisconnectDecoderWithOldParty = FALSE;
          }

          if (canDisconnectDecoderWithOldParty)
            CalculateChangeLayoutActionsForDecoder(decoder_index, oldArtPartyId, newArtPartyId, newRequiredResolution, cstr, isNeedToConsiderOutRes);
          else
          {
            PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::CalculateChangeLayoutAction party in old layout video in is disconnected decoder index:", decoder_index);
            AddPartyIdToCalcStr(oldArtPartyId, cstr);
            // algorithm   |  resolution |
            cstr << "         |             |";
            CalculateChangeLayoutActionsForDecoderWhenOldArtIsNotValid(decoder_index, newArtPartyId, newRequiredResolution, cstr, isNeedToConsiderOutRes);
          }
        }
      }
      else // decoder was not in use in old layout and in use in new layout --> create decoder

      {
        // partyCntl of new Image in decoder (in order to get video params from it)
        CVideoBridgePartyCntlCOP* pNewPartyCntl = (CVideoBridgePartyCntlCOP*)GetPartyCntl(newArtPartyId);
        if (!CPObject::IsValidPObjectPtr(pNewPartyCntl))
        {
          // cstr << " PartyId | algorithm |  resolution | PartyId | algorithm |          |(from table) | (party caps)| resolution  |                  |" << "\n"
          cstr << "         |           |             |         |           |               |             |             |             |                                |" << "\n";
          PASSERT(1);
          continue;
        }

        // cstr << " PartyId | algorithm |  resolution | PartyId | algorithm |          |(from table) | (party caps)| resolution  |                  |" << "\n"
        cstr << "         |           |             |";

        ECopDecoderResolution newPartyMaxDecoderResolution, newPartyCurrentDecoderResolution;
        DWORD                 newPartyAlgorithm, newPartyInitialAlgorithm;
        // required res = min (newRequiredResolution, newPartyInitialDecoderResolution)
        pNewPartyCntl->GetInitialCopDecoderResolution(newPartyInitialAlgorithm, newPartyMaxDecoderResolution);

        // current video Parameters of next connected party
        pNewPartyCntl->GetCurrentCopDecoderResolution(newPartyAlgorithm, newPartyCurrentDecoderResolution);

        // cstr << "  PartyId  |   algorithm     |  resolution |  PartyId  | algorithm | current res |required res | initial res | selected    |       Actions          |"
        AddPartyIdToCalcStr(newArtPartyId, cstr);
        AddVideoAlgToCalcStr(newPartyAlgorithm, cstr);
        AddResolutionToCalcStr(newPartyCurrentDecoderResolution, cstr);
        AddResolutionToCalcStr(newRequiredResolution, cstr);
        AddResolutionToCalcStr(newPartyMaxDecoderResolution, cstr);

        ECopDecoderResolution copDecoderResolutionFromEncoder = GetDecoderResolutionFromEncoder(pNewPartyCntl);
        AdjustRequiredResolotion(newRequiredResolution, newPartyMaxDecoderResolution, newPartyAlgorithm, cstr, newArtPartyId, copDecoderResolutionFromEncoder, isNeedToConsiderOutRes);

        AddResolutionToCalcStr(newRequiredResolution, cstr);

        if (newRequiredResolution != newPartyCurrentDecoderResolution)
        {
          cstr << "            ReCap Open          |\n";
          m_changeLayoutActions->AddDetailedAction(decoder_index, eCopDecoderAction_ReCapToParty, newArtPartyId, newRequiredResolution);
        }
        else
        {
          cstr << "             Open               |\n";
        }

        m_changeLayoutActions->AddDetailedAction(decoder_index, eCopDecoderAction_Open, newArtPartyId);
      }
    }
    else
    {
      // decoder was in use in old layout and not in use in new layout --> close decoder
      if (decoder_index_in_old_layout != (WORD)-1)
      {
        cstr << "         |       |         |         |       |             |             |             |             |Close (dec removed from layout) |"<< "\n";

        // case close decoder (removed from layout)
        m_changeLayoutActions->UpdateActionStatus(decoder_index, eCopDecoderAction_Close);
      }
      else  // decoder was not use in old layout and not in use in new layout --> do nothing
      {
        // cstr << "---------+-----------+-------------+---------+-----------+-------------+-------------+-------------+-------------+--------------------------------+" << "\n";
        cstr << "         |           |             |         |           |             |             |             |             |                                |" << "\n";
      }
    }
  } // for decoders

  cstr << "+-----------+---------+-----------+-----------+---------+-----------+-------------+---------+-----------+-------------+-------------+-------------+-------------+--------------------------------+" << "\n";

  if(cstr.GetString())
    PTRACE(eLevelInfoNormal, cstr.GetString());
  else
    PASSERTMSG(1, "cstr.GetString() return NULL");

  // DSP MPMX smartSwitch
  if (IsDSPSmartSwitchNeeded(oldLayout))
  {
    PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::CalculateChangeLayoutAction IsDSPSmartSwitchNeeded = YES");
    m_IsDSPSmartSwitch = TRUE;
    UpdateDSPSmartSwitchImagesParam(newLayout, oldLayout);
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::CalculateChangeLayoutActionsForDecoder(WORD decoder_index, DWORD oldArtId, DWORD newArtId, ECopDecoderResolution newRequiredResolution, CObjString& cstr, BYTE isNeedToConsiderOutRes)
{
  CCopLayoutResolutionTable table;

  // partyCntl of Old Image in decoder (in order to get video params from it)
  CVideoBridgePartyCntlCOP* pOldPartyCntl = (CVideoBridgePartyCntlCOP*)GetPartyCntl(oldArtId);

  // partyCntl of Old Image in decoder (in order to get video params from it)
  CVideoBridgePartyCntlCOP* pNewPartyCntl = (CVideoBridgePartyCntlCOP*)GetPartyCntl(newArtId);

  if (!CPObject::IsValidPObjectPtr(pOldPartyCntl)) {
    PASSERT_AND_RETURN(oldArtId);
  }

  if (!CPObject::IsValidPObjectPtr(pNewPartyCntl)) {
    PASSERT_AND_RETURN(newArtId);
  }

  DWORD newPartyAlgorithm, newPartyInitialAlgorithm, oldPartyAlgorithm;

  // newPartyMaxDecoderResolution - max resolution from caps of new party (the parameters that the first create from party cntl received)
  // newPartyCurrentDecoderResolution - the paramaters that new party is opened now with
  // oldPartyCurrentDecoderResolution - the paramaters that old party is opened now with (the parameters of the decoder)

  ECopDecoderResolution newPartyMaxDecoderResolution, newPartyCurrentDecoderResolution, oldPartyCurrentDecoderResolution;

  // required res = min (newRequiredResolution, newPartyInitialDecoderResolution)
  pNewPartyCntl->GetInitialCopDecoderResolution(newPartyInitialAlgorithm, newPartyMaxDecoderResolution);

  // current video Parameters of next connected party
  pNewPartyCntl->GetCurrentCopDecoderResolution(newPartyAlgorithm, newPartyCurrentDecoderResolution);

  if (newPartyAlgorithm != newPartyInitialAlgorithm)
    PTRACE2INT(eLevelInfoNormal, "Notice! party has changed its video protocol party id: ", newArtId);

  // current video Parameters of that this decoder is open with (the params of old party)
  pOldPartyCntl->GetCurrentCopDecoderResolution(oldPartyAlgorithm, oldPartyCurrentDecoderResolution);

  AddPartyIdToCalcStr(oldArtId, cstr);
  AddVideoAlgToCalcStr(oldPartyAlgorithm, cstr);
  AddResolutionToCalcStr(oldPartyCurrentDecoderResolution, cstr);
  AddPartyIdToCalcStr(newArtId, cstr);
  AddVideoAlgToCalcStr(newPartyAlgorithm, cstr);
  AddResolutionToCalcStr(newPartyCurrentDecoderResolution, cstr);
  AddResolutionToCalcStr(newRequiredResolution, cstr);
  AddResolutionToCalcStr(newPartyMaxDecoderResolution, cstr);

  ECopDecoderResolution copDecoderResolutionFromEncoder = GetDecoderResolutionFromEncoder(pNewPartyCntl);
  AdjustRequiredResolotion(newRequiredResolution, newPartyMaxDecoderResolution, newPartyAlgorithm, cstr, newArtId, copDecoderResolutionFromEncoder, isNeedToConsiderOutRes);

  AddResolutionToCalcStr(newRequiredResolution, cstr);

  BYTE needRecap = FALSE;
  if (newRequiredResolution != newPartyCurrentDecoderResolution)
  {
    needRecap = TRUE;
    m_changeLayoutActions->AddDetailedAction(decoder_index, eCopDecoderAction_ReCapToParty, newArtId, newRequiredResolution);
  }

  // same party different resolution
  if (oldArtId == newArtId)
  {
    if (newRequiredResolution != oldPartyCurrentDecoderResolution)
    {
      // cstr << "ReCap Open                      |\n";
      if (needRecap)
        cstr << "ReCap Close Open                |\n";
      else
        cstr << "Close Open                      |\n";

      m_changeLayoutActions->AddDetailedAction(decoder_index, eCopDecoderAction_Open, newArtId, newRequiredResolution);
      m_changeLayoutActions->AddDetailedAction(decoder_index, eCopDecoderAction_Close, newArtId, newRequiredResolution);
    }
    else
    {
      if (needRecap)
        cstr << "ReCap                           |\n";
      else
        cstr << "                            |\n";
    }
  }
  else
  {
    if (newRequiredResolution != oldPartyCurrentDecoderResolution)
    {
      if (needRecap)
        cstr << "ReCap Close Open                |\n";
      else
        cstr << "Close Open              |\n";

      m_changeLayoutActions->AddDetailedAction(decoder_index, eCopDecoderAction_Open, newArtId, newRequiredResolution);
      m_changeLayoutActions->AddDetailedAction(decoder_index, eCopDecoderAction_Close, newArtId, newRequiredResolution);
    }
    else
    {
      PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::CalculateChangeLayoutActionsForDecoder close-open instead of disconnect-connect");

      if (needRecap)
        cstr << "ReCap Close Open                |\n";
      else
        cstr << "Close Open              |\n";

      m_changeLayoutActions->AddDetailedAction(decoder_index, eCopDecoderAction_Open, newArtId, newRequiredResolution);
      m_changeLayoutActions->AddDetailedAction(decoder_index, eCopDecoderAction_Close, newArtId, newRequiredResolution);
    }
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::CalculateChangeLayoutActionsForDecoderWhenOldArtIsNotValid(WORD decoder_index, DWORD newArtId, ECopDecoderResolution newRequiredResolution, CObjString& cstr, BYTE isNeedToConsiderOutRes)
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::CalculateChangeLayoutActionsForDecoderWhenOldArtIsNotValid ");

  CCopLayoutResolutionTable table;

  // partyCntl of Old Image in decoder (in order to get video params from it)
  CVideoBridgePartyCntlCOP* pNewPartyCntl = (CVideoBridgePartyCntlCOP*)GetPartyCntl(newArtId);

  // add protection - fix bug
  if (!CPObject::IsValidPObjectPtr(pNewPartyCntl)) {
    PASSERT_AND_RETURN(newArtId);
  }

  DWORD newPartyAlgorithm, newPartyInitialAlgorithm;

  // newPartyMaxDecoderResolution - max resolution from caps of new party (the parameters that the first create from party cntl received)
  // newPartyCurrentDecoderResolution - the paramaters that new party is opened now with

  ECopDecoderResolution newPartyMaxDecoderResolution, newPartyCurrentDecoderResolution;

  // required res = min (newRequiredResolution, newPartyInitialDecoderResolution)
  pNewPartyCntl->GetInitialCopDecoderResolution(newPartyInitialAlgorithm, newPartyMaxDecoderResolution);

  // current video Parameters of next connected party
  pNewPartyCntl->GetCurrentCopDecoderResolution(newPartyAlgorithm, newPartyCurrentDecoderResolution);

  if (newPartyAlgorithm != newPartyInitialAlgorithm)
    PTRACE2INT(eLevelInfoNormal, "Notice! party has changed its video protocol party id: ", newArtId);

  AddPartyIdToCalcStr(newArtId, cstr);
  AddVideoAlgToCalcStr(newPartyAlgorithm, cstr);
  AddResolutionToCalcStr(newPartyCurrentDecoderResolution, cstr);
  AddResolutionToCalcStr(newRequiredResolution, cstr);
  AddResolutionToCalcStr(newPartyMaxDecoderResolution, cstr);

  ECopDecoderResolution copDecoderResolutionFromEncoder = GetDecoderResolutionFromEncoder(pNewPartyCntl);
  AdjustRequiredResolotion(newRequiredResolution, newPartyMaxDecoderResolution, newPartyAlgorithm, cstr, newArtId, copDecoderResolutionFromEncoder, isNeedToConsiderOutRes);

  AddResolutionToCalcStr(newRequiredResolution, cstr);

  // cstr << "Actions: ";
  if (newRequiredResolution != newPartyCurrentDecoderResolution)
  {
    cstr << "ReCap Close Open                |\n";
    m_changeLayoutActions->AddDetailedAction(decoder_index, eCopDecoderAction_ReCapToParty, newArtId, newRequiredResolution);
  }
  else
    cstr << "Close Open              |\n";

  // Any case when the old art id wasn't valid we will close and open the decoder
  m_changeLayoutActions->AddDetailedAction(decoder_index, eCopDecoderAction_Open, newArtId, newRequiredResolution);
  m_changeLayoutActions->AddDetailedAction(decoder_index, eCopDecoderAction_Close, newArtId, newRequiredResolution);
}

////////////////////////////////////////////////////////////////////////////
DWORD CVideoBridgeCOP::GetAudioSpeakerPlaceInLayout()
{
	if (!m_pVisualEffects)
		return INVALID;

	// Step 1: check speaker Notation enabled
	if (m_pVisualEffects->IsSpeakerNotationEnable() == 0)
		return INVALID;

	// Step 2: Speaker Notation should not be activated in layout 1x1
	WORD numSubImage = m_ConfLayout->GetNumberOfSubImages();
	if (numSubImage == 1)
		return INVALID;

	// Step 3: Find the Audio Speaker
	CTaskApp* pLastActiveAudioSpeaker = GetLastActiveAudioSpeakerRequest();
	if (pLastActiveAudioSpeaker == NULL)
		return INVALID;

	// Step 4: Look for Audio Speaker in layout
	CPartyImageLookupTable* pPartyImageLookupTable = GetPartyImageLookupTable();
	for (WORD imageNumber = 0; imageNumber < numSubImage; imageNumber++)
	{
		CVidSubImage* pSubImage = (*m_ConfLayout)[imageNumber];
		if (pSubImage)
		{
			DWORD partyRscId = pSubImage->GetImageId();
			CImage* pImage = pPartyImageLookupTable->GetPartyImage(partyRscId);
			PASSERTSTREAM(!pImage, "CVideoBridgeCOP::GetAudioSpeakerPlaceInLayout - Failed, The lookup table doesn't have an element, PartyId:" << partyRscId);

			if (pImage && pImage->GetVideoSource() == pLastActiveAudioSpeaker)
			{
				// Step 4:Speaker found in layout -> return place in array
				return imageNumber;
			}
		}
	}

	// Step 5: Audio Speaker NOT found in layout -> return INVALID
	return INVALID;
}


////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::StartSwitch()
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::StartSwitch");
  BOOL areThereActionsToPerform = FALSE;
  // Stage I in performing the changelayout actions:
  // Go over all the decoders and to send: close,disconnect for the relevant decoders,and change mode for the relevant party that will be connected to the decoder
  for (int i = 0; i < NUM_OF_COP_DECODERS; i++)
  {
    WORD                       decoderIndex              = i;
    eChangeLayoutActionsStatus actionStatusForClose      = m_changeLayoutActions->GetChangeLayoutActionStatusForDecoder(i, eCopDecoderAction_Close);
    eChangeLayoutActionsStatus actionStatusForDisconnect = m_changeLayoutActions->GetChangeLayoutActionStatusForDecoder(i, eCopDecoderAction_Disconnect);
    eChangeLayoutActionsStatus actionStatusForChangeMode = m_changeLayoutActions->GetChangeLayoutActionStatusForDecoder(i, eCopDecoderAction_ReCapToParty);
    DWORD                      artPartyId                = DUMMY_PARTY_ID;
    ECopDecoderResolution      copDecRes                 = COP_decoder_resolution_Last;
    if (actionStatusForChangeMode == eActionNeeded)
    {
      areThereActionsToPerform = TRUE;
      artPartyId               = m_changeLayoutActions->GetChangeLayoutActionArtIdForDecoder(i, eCopDecoderAction_ReCapToParty);
      copDecRes                = m_changeLayoutActions->GetChangeLayoutActionDecoderResForDecoder(i, eCopDecoderAction_ReCapToParty);
      SendChangeModeToParty(decoderIndex, artPartyId, copDecRes);
    }

    if (actionStatusForClose == eActionNeeded)
    {
      areThereActionsToPerform = TRUE;
      SendClosePortToDecoder(decoderIndex);
    }
    else if (actionStatusForDisconnect == eActionNeeded)
    {
      areThereActionsToPerform = TRUE;
      SendDisconnectPortToDecoder(decoderIndex);
    }
  }

  BOOL isStageFinished = AreAllCloseDisconnnectChangeModeDecActionsCompleted();

  if (!isStageFinished && !areThereActionsToPerform)
    PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::StartSwitch no new actions but stage didn't finish");

  if (!areThereActionsToPerform && isStageFinished)
  {
    PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::StartSwitch End Stage I no close/disconnect/changemode actions needed we can continue");
    StartSwitchOpenAndConnectDecodersActions();
  }
  else
    PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::StartSwitch End Stage I waiting to all ACKs to continue with the ChangeLayout actions");
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::StartSwitchOpenAndConnectDecodersActions()
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::StartSwitchOpenAndConnectDecodersActions");
  // Stage II in performing the changelayout actions :
  // We will go over all the decoders and to send: open(create and connect) ,and connect(update and connect) to the relevant decoders
  BOOL areThereActionsToPerform = FALSE;
  for (int i = 0; i < NUM_OF_COP_DECODERS; i++)
  {
    WORD                       decoderIndex           = i;
    eChangeLayoutActionsStatus actionStatusForOpen    = m_changeLayoutActions->GetChangeLayoutActionStatusForDecoder(i, eCopDecoderAction_Open);
    eChangeLayoutActionsStatus actionStatusForConnect = m_changeLayoutActions->GetChangeLayoutActionStatusForDecoder(i, eCopDecoderAction_Connect);
    DWORD artPartyId                                  = DUMMY_PARTY_ID;
    if (actionStatusForOpen == eActionNeeded)
    {
      areThereActionsToPerform = TRUE;
      artPartyId               = m_changeLayoutActions->GetChangeLayoutActionArtIdForDecoder(i, eCopDecoderAction_Open);
      SendOpenPortToDecoder(decoderIndex, artPartyId);
    }

    else if (actionStatusForConnect == eActionNeeded)
    {
      areThereActionsToPerform = TRUE;
      artPartyId               = m_changeLayoutActions->GetChangeLayoutActionArtIdForDecoder(i, eCopDecoderAction_Connect);
      SendConnectPortToDecoder(decoderIndex, artPartyId);
    }
  }

  BOOL isStageFinished = AreAllOpenConnectSyncDecActionsCompleted();
  if (!isStageFinished && !areThereActionsToPerform)
    PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::StartSwitchOpenAndConnectDecodersActions no new actions but stage didn't finish");

  if (!areThereActionsToPerform && isStageFinished)
  {
    PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::StartSwitchOpenAndConnectDecodersActions End Stage II no Open/Connect actions needed we can continue");
    EndChangeLayoutActions();
  }
  else
    PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::StartSwitchOpenAndConnectDecodersActions End Stage II waiting to all ACKs to continue with the ChangeLayout actions");
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::SendChangeLayoutToAllLevelEncoders()
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::SendChangeLayoutToAllLevelEncoders");

  if (!IsActiveLectureMode())
  {
    // send the new layout to all encoders
    for (WORD encoder_index = 0; encoder_index < NUM_OF_LEVEL_ENCODERS; encoder_index++)
    {
      // VNGR-13667 If lecture mode has started / ended and the new layout is 1x1,
      // there are cases that the same image is in the layout but with different decoder id --> we will send the change layout anyway
      m_pCopLevelEncoders[encoder_index]->ChangeLayout(m_ConfLayout, m_pVisualEffects, m_pSiteNameInfo, GetAudioSpeakerPlaceInLayout(), m_SendChangeLayoutAnyWay);
      DWORD levelEncoderEntityId = m_pCopLevelEncoders[encoder_index]->GetPartyRsrcID();
      UpdateCopEncoderPartiesLayoutInDB(m_ConfLayout, levelEncoderEntityId);
      // the case when changing from lecture mode to same layout
      if (m_needToRequestIntraFromLevelEncoder[encoder_index] == TRUE)
      {
        PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::SendChangeLayoutToAllLevelEncoders send fast update for postponed intra request ", encoder_index);
        m_pCopLevelEncoders[encoder_index]->FastUpdate();
        m_needToRequestIntraFromLevelEncoder[encoder_index] = FALSE;
      }
    }

    if (m_pcmConnected)
      m_pPCMEncoderCntl->ChangeLayout(m_ConfLayout, m_pVisualEffects, NULL, GetAudioSpeakerPlaceInLayout(), m_SendChangeLayoutAnyWay);
  }
  else // active lecture mode
  {
    PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::SendChangeLayoutToAllLevelEncoders lecture mode active");

    DumpLayouts(NULL);

    DWORD last_lm_decoder_sync_status                 = m_pCopDecoderLecturer->GetLastVideoInSyncStatus();
    BYTE  isLecturerDetectedVideoMatchCurrentResRatio = m_pCopDecoderLecturer->IsDetectedVideoMatchCurrentResRatio();
    PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::SendChangeLayoutToAllLevelEncoders last_lm_decoder_sync_status = ", last_lm_decoder_sync_status);
    if (last_lm_decoder_sync_status != statOK)
    {
      m_pCopLectureModeCntl->SetWaitingToLecturerDecoderSync(YES);
      m_pCopLectureModeCntl->SetSendChangeLayoutToEveryOneAfterLecturerDecoderSync(m_SendChangeLayoutAnyWay);
      StartTimer(LECTURER_DECODER_TIMER, 3*SECOND);
    }

    for (WORD encoder_index = 0; encoder_index < NUM_OF_LEVEL_ENCODERS; encoder_index++)
    {
      DWORD levelEncoderEntityId = m_pCopLevelEncoders[encoder_index]->GetPartyRsrcID();
      if (encoder_index == m_lecturerlevelEncoderIndex)
      {
        PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::SendChangeLayoutToAllLevelEncoders send change layout to lecturer, encoder_index = ", encoder_index);
        // send layout to lecturer encoder
        if (IsOnlyLecturerConnected())
        {
          PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::SendChangeLayoutToAllLevelEncoders only lecturer connected");
          // if only lecturer connected send 1x1 layout to lecturer encoder
          // VNGR-13665 In this case there are cases that just the decoder entity id changed, and we will like to send the change layout in this case.
          // Eitan (01/10) - use the flag of VNGR-13667 for that case also
          m_pCopLevelEncoders[encoder_index]->ChangeLayout(m_ConfLayout1x1, m_pVisualEffects, m_pSiteNameInfo, INVALID, m_SendChangeLayoutAnyWay);
          UpdateCopEncoderPartiesLayoutInDB(m_ConfLayout1x1, levelEncoderEntityId);
        }
        else
        {
          // send conf layout to lecturer encoder
          if (m_SmartLecturerState.IsSmartLevelState())
          {
            // SMART_SWITCH_LECTURER_1x1: LECTURE1 and LECTURE2 are in the same level or in the different encoder level.
            // In any direction of switching LECTURE1->LECTURE2 or LECTURE2->LECTURE1 lecturer encoder stay connected to its layout, i.e. NOT CHANGE layout.
            PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::SendChangeLayoutToAllLevelEncoders - eLmSmartState_XXX, Layout=Not_Changed, lecturer_encoder_index=", encoder_index);
          }
          else
          {
            PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::SendChangeLayoutToAllLevelEncoders - eLmSmartState_None, Layout=ConfLayout, lecturer_encoder_index=", encoder_index);
            m_pCopLevelEncoders[encoder_index]->ChangeLayout(m_ConfLayout, m_pVisualEffects, m_pSiteNameInfo, GetAudioSpeakerPlaceInLayout(), m_SendChangeLayoutAnyWay);
            UpdateCopEncoderPartiesLayoutInDB(m_ConfLayout, m_pCopLevelEncoders[encoder_index]->GetPartyRsrcID());
            if (m_needToRequestIntraFromLevelEncoder[encoder_index] == TRUE)
            {
              m_pCopLevelEncoders[encoder_index]->FastUpdate();
              m_needToRequestIntraFromLevelEncoder[encoder_index] = FALSE;
            }
          }

          if (last_lm_decoder_sync_status != statOK)
            PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::SendChangeLayoutToAllLevelEncoders - waiting to lecturer decoder sync, encoder_index=", encoder_index);
          else
            SendChangeLayoutToVSWEncoder(encoder_index, m_SendChangeLayoutAnyWay);
        }
      }
      else // LISTENERS
      {
        if (last_lm_decoder_sync_status != statOK)
        {
          PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::SendChangeLayoutToAllLevelEncoders waiting to lecturer decoder sync, encoder_index = ", encoder_index);
        }
        else
        {
          SendChangeLayoutToLevelEncoder(encoder_index, m_SendChangeLayoutAnyWay);
          if (m_needToRequestIntraFromLevelEncoder[encoder_index] == TRUE)
          {
            m_pCopLevelEncoders[encoder_index]->FastUpdate();
            m_needToRequestIntraFromLevelEncoder[encoder_index] = FALSE;
          }
        }
      }
    }

    if (m_pcmConnected)
    {
      if (!IsLecturerConnectedToPCM() || IsOnlyLecturerConnected())
        m_pPCMEncoderCntl->ChangeLayout(m_ConfLayout1x1, m_pVisualEffects, NULL, INVALID, m_SendChangeLayoutAnyWay);
      else
        m_pPCMEncoderCntl->ChangeLayout(m_ConfLayout, m_pVisualEffects, NULL, GetAudioSpeakerPlaceInLayout(), m_SendChangeLayoutAnyWay);
    }

    DumpConnnections("SendChangeLayoutToAllLevelEncoders");
  }

  m_SendChangeLayoutAnyWay                  = NO;
  m_isChangeLayoutBecauseOfLectureModeFlows = FALSE;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::SendChangeLayoutOnLecturerDecoderSync()
{
  BYTE bIsActiveLectureMode = IsActiveLectureMode();
  PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::SendChangeLayoutOnLecturerDecoderSync - IsActiveLectureMode=", (int)bIsActiveLectureMode);

  m_pCopLectureModeCntl->SetWaitingToLecturerDecoderSync(NO);
  BYTE sendChangeLayoutAnyway = m_pCopLectureModeCntl->GetSendChangeLayoutToEveryOneAfterLecturerDecoderSync();
  m_pCopLectureModeCntl->SetSendChangeLayoutToEveryOneAfterLecturerDecoderSync(NO);
  DeleteTimer(LECTURER_DECODER_TIMER);
  if (bIsActiveLectureMode) // active lecture mode
  {
    for (WORD encoder_index = 0; encoder_index < NUM_OF_LEVEL_ENCODERS; encoder_index++)
    {
      if (encoder_index == m_lecturerlevelEncoderIndex)
        SendChangeLayoutToVSWEncoder(encoder_index, sendChangeLayoutAnyway);
      else
        SendChangeLayoutToLevelEncoder(encoder_index, sendChangeLayoutAnyway);

      if (m_needToRequestIntraFromLevelEncoder[encoder_index] == TRUE)
      {
        m_pCopLevelEncoders[encoder_index]->FastUpdate();
        m_needToRequestIntraFromLevelEncoder[encoder_index] = FALSE;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::SendChangeLayoutToLevelEncoder(DWORD encoder_index, BYTE sendChangeLayoutAnyWay)
{
  switch (m_SmartLecturerState.m_eState)
  {
    // SMART_SWITCH_LECTURER_1x1: LECTURE1 and LECTURE2 are in the same level (LECTURE1->LECTURE2)
    // Connect CONF_LAYOUT to all encoders.
    case eLmSmartState_SameLevel2Lecturer:
      PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::SendChangeLayoutToLevelEncoder - eLmSmartState_SameLevel2Lecturer, Layout=ConfLayout, listener_encoder_index=", encoder_index);
      m_pCopLevelEncoders[encoder_index]->ChangeLayout(m_ConfLayout, m_pVisualEffects, m_pSiteNameInfo, INVALID, sendChangeLayoutAnyWay);
      UpdateCopEncoderPartiesLayoutInDB(m_ConfLayout, m_pCopLevelEncoders[encoder_index]->GetPartyRsrcID());
      m_needToRequestIntraFromLevelEncoder[encoder_index] = TRUE;
      break;

    // SMART_SWITCH_LECTURER_1x1: LECTURE1 and LECTURE2 are in the same level (LECTURE2->LECTURE1)
    // Connect LECT_LAYOUT to all encoders.
    case eLmSmartState_SameLevel1Lecturer:
      PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::SendChangeLayoutToLevelEncoder - eLmSmartState_SameLevel1Lecturer, Layout=Layout1x1, listener_encoder_index=", encoder_index);
      m_pCopLevelEncoders[encoder_index]->ChangeLayout(m_ConfLayout1x1, m_pVisualEffects, m_pSiteNameInfo, INVALID, sendChangeLayoutAnyWay);
      UpdateCopEncoderPartiesLayoutInDB(m_ConfLayout1x1, m_pCopLevelEncoders[encoder_index]->GetPartyRsrcID());
      m_needToRequestIntraFromLevelEncoder[encoder_index] = TRUE;
      break;

    // SMART_SWITCH_LECTURER_1x1: LECTURE1 and LECTURE2 are in the different encoder level (LECTURE1->LECTURE2)
    // Connect CONF_LAYOUT to all encoders except LECTURER2 (new lecturer) level encoder
    case eLmSmartState_DiffLevel2Lecturer:
      if (encoder_index == m_SmartLecturerState.m_newLecturer.m_LevelEncoderIndex)
      {
        PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::SendChangeLayoutToLevelEncoder - eLmSmartState_DiffLevel2Lecturer, Layout=Not_Changed, listener_encoder_index=", encoder_index);
        break;
      }

      PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::SendChangeLayoutToLevelEncoder - eLmSmartState_DiffLevel2Lecturer, Layout=ConfLayout, listener_encoder_index=", encoder_index);
      m_pCopLevelEncoders[encoder_index]->ChangeLayout(m_ConfLayout, m_pVisualEffects, m_pSiteNameInfo, INVALID, sendChangeLayoutAnyWay);
      UpdateCopEncoderPartiesLayoutInDB(m_ConfLayout, m_pCopLevelEncoders[encoder_index]->GetPartyRsrcID());
      m_needToRequestIntraFromLevelEncoder[encoder_index] = TRUE;
      break;

    // SMART_SWITCH_LECTURER_1x1: LECTURE1 and LECTURE2 are in the different encoder level (LECTURE2->LECTURE1)
    // Connect LECT_LAYOUT to all encoders except LECTURER1 (new lecturer) level encoder
    case eLmSmartState_DiffLevel1Lecturer:
      if (encoder_index == m_SmartLecturerState.m_newLecturer.m_LevelEncoderIndex)
      {
        PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::SendChangeLayoutToLevelEncoder - eLmSmartState_DiffLevel1Lecturer, Layout=Not_Changed, listener_encoder_index=", encoder_index);
        break;
      }

      PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::SendChangeLayoutToLevelEncoder - eLmSmartState_DiffLevel1Lecturer, Layout=Layout1x1, listener_encoder_index=", encoder_index);
      m_pCopLevelEncoders[encoder_index]->ChangeLayout(m_ConfLayout1x1, m_pVisualEffects, m_pSiteNameInfo, INVALID, sendChangeLayoutAnyWay);
      UpdateCopEncoderPartiesLayoutInDB(m_ConfLayout1x1, m_pCopLevelEncoders[encoder_index]->GetPartyRsrcID());
      m_needToRequestIntraFromLevelEncoder[encoder_index] = TRUE;
      break;

    case eLmSmartState_None:
    default:
      PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::SendChangeLayoutToLevelEncoder - eLmSmartState_None, Layout1x1, listener_encoder_index=", encoder_index);
      m_pCopLevelEncoders[encoder_index]->ChangeLayout(m_ConfLayout1x1, m_pVisualEffects, m_pSiteNameInfo, INVALID, sendChangeLayoutAnyWay);
      UpdateCopEncoderPartiesLayoutInDB(m_ConfLayout1x1, m_pCopLevelEncoders[encoder_index]->GetPartyRsrcID());
      break;
  } // switch
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::SendChangeLayoutToVSWEncoder(DWORD encoder_index, BYTE sendChangeLayoutAnyWay)
{
	PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::SendChangeLayoutToVSWEncoder - send change layout to VSW encoder, encoder_index=", encoder_index);

	switch (m_SmartLecturerState.m_eState)
	{
		case eLmSmartState_DiffLevel2Lecturer:
			SwitchlecturerDecoderIdInImage(*m_ConfLayout, eVswDecoder);
			m_pCopEncoderVsw->ChangeLayout(m_ConfLayout, m_pVisualEffects, NULL, INVALID, sendChangeLayoutAnyWay);
			UpdateCopEncoderPartiesLayoutInDB(m_ConfLayout, m_pCopEncoderVsw->GetPartyRsrcID());
			SwitchlecturerDecoderIdInImage(*m_ConfLayout, eLectureDecoder);
			break;

		case eLmSmartState_DiffLevel1Lecturer:
		case eLmSmartState_SameLevel1Lecturer:
		case eLmSmartState_SameLevel2Lecturer:
		case eLmSmartState_None:
		default:
			SwitchlecturerDecoderIdInImage(*m_ConfLayout1x1, eVswDecoder);
			m_pCopEncoderVsw->ChangeLayout(m_ConfLayout1x1, m_pVisualEffects, NULL, INVALID, sendChangeLayoutAnyWay);
			UpdateCopEncoderPartiesLayoutInDB(m_ConfLayout1x1, m_pCopEncoderVsw->GetPartyRsrcID());
			SwitchlecturerDecoderIdInImage(*m_ConfLayout1x1, eLectureDecoder);
			break;
	}
}
////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::SendLayoutAttributesToEncoders()
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::SendLayoutAttributesToEncoders");
  // send the new layout to all encoders
  for (WORD encoder_index = 0; encoder_index < NUM_OF_LEVEL_ENCODERS; encoder_index++)
  {
    m_pCopLevelEncoders[encoder_index]->ChangeLayoutAttributes(m_pVisualEffects, GetAudioSpeakerPlaceInLayout());
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::SendLayoutAttributesToLecturerEncoder()
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::SendLayoutAttributesToLecturerEncoder");
  m_pCopLevelEncoders[m_lecturerlevelEncoderIndex]->ChangeLayoutAttributes(m_pVisualEffects, GetAudioSpeakerPlaceInLayout());
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::SendLayoutAttributesToListenersEncoders()
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::SendLayoutAttributesToListenersEncoders");
  for (WORD encoder_index = 0; encoder_index < NUM_OF_LEVEL_ENCODERS; encoder_index++)
    if (encoder_index != m_lecturerlevelEncoderIndex)
      m_pCopLevelEncoders[encoder_index]->ChangeLayoutAttributes(m_pVisualEffects, INVALID);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::SendChangeModeToParty(WORD decoderIndex, DWORD artPartyId, ECopDecoderResolution copDecRes)
{
	CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)m_pPartyList->Find(artPartyId);
	if (pPartyCntl)
	{
		TRACEINTO << "PartyId:" << artPartyId << ", DecoderIndex:" << decoderIndex << ", CopDecoderResolution:" << ECopDecoderResolutionString[copDecRes];

		eChangeLayoutActionsStatus actionStatusForChangeMode = m_changeLayoutActions->GetChangeLayoutActionStatusForDecoder(decoderIndex, eCopDecoderAction_ReCapToParty);
		if (actionStatusForChangeMode != eActionNeededInFirstPrior)
			m_changeLayoutActions->UpdateActionStatus(decoderIndex, eCopDecoderAction_ReCapToParty, eActionInProgress);

		m_pConfApi->CopVideoInChangeMode(artPartyId, (BYTE)copDecRes);
	}
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::SendClosePortToDecoder(WORD decoderIndex)
{
  if (CPObject::IsValidPObjectPtr(m_pCopDecoders[decoderIndex]))
  {
    PTRACE2INT(eLevelInfoNormal, "SendClosePortToDecoder Decoder Index:", decoderIndex);
    if (m_pCopDecoders[decoderIndex]->GetState() == IDLE)
    {
      PTRACE2INT(eLevelInfoNormal, "SendClosePortToDecoder decoder is already IDLE Decoder Index:", decoderIndex);
      CSegment* pSeg = new CSegment;
      *pSeg << (DWORD)0xFFFFFFFF
            << (WORD)statOK
            << (WORD)eNoDirection
            << (DWORD)decoderIndex;
      DispatchEvent(END_CLOSE_COP_DECODER, pSeg);
      POBJDELETE(pSeg);
    }
    else
    {
      eChangeLayoutActionsStatus actionStatusForClose = m_changeLayoutActions->GetChangeLayoutActionStatusForDecoder(decoderIndex, eCopDecoderAction_Close);
      if (actionStatusForClose != eActionNeededInFirstPrior)
      {
        m_changeLayoutActions->UpdateActionStatus(decoderIndex, eCopDecoderAction_Close, eActionInProgress);
      }

      m_pCopDecoders[decoderIndex]->Close();
    }
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::SendDisconnectPortToDecoder(WORD decoderIndex)
{
  if (CPObject::IsValidPObjectPtr(m_pCopDecoders[decoderIndex]))
  {
    PTRACE2INT(eLevelInfoNormal, "SendDisconnectPortToDecoder Decoder Index: ", decoderIndex);
    if (m_pCopDecoders[decoderIndex]->GetState() == IDLE)
    {
      PTRACE2INT(eLevelInfoNormal, "SendDisconnectPortToDecoder decoder is already IDLE Decoder Index:\n", decoderIndex);
      // In case the decoder is in IDLE state we will change the action from disconnect-connect
      // to close-open (no need to wait for close cause it's already closed)
      DWORD                 newArtId              = m_changeLayoutActions->GetChangeLayoutActionArtIdForDecoder(decoderIndex, eCopDecoderAction_Disconnect);
      ECopDecoderResolution newRequiredResolution = m_changeLayoutActions->GetChangeLayoutActionDecoderResForDecoder(decoderIndex, eCopDecoderAction_Disconnect);

      m_changeLayoutActions->UpdateActionStatus(decoderIndex, eCopDecoderAction_Connect, eActionComplete);
      m_changeLayoutActions->AddDetailedAction(decoderIndex, eCopDecoderAction_Open, newArtId, newRequiredResolution);

      CSegment* pSeg = new CSegment;
      *pSeg << (DWORD)0xFFFFFFFF
            << (WORD)statOK
            << (WORD)eNoDirection
            << (DWORD)decoderIndex;
      DispatchEvent(END_DISCONNECT_COP_DECODER, pSeg);
      POBJDELETE(pSeg);
    }
    else
    {
      m_pCopDecoders[decoderIndex]->SetDisConnectingDirectionsReq(eMediaIn);
      eChangeLayoutActionsStatus actionStatusForDisconnect = m_changeLayoutActions->GetChangeLayoutActionStatusForDecoder(decoderIndex, eCopDecoderAction_Disconnect);
      if (actionStatusForDisconnect != eActionNeededInFirstPrior)
        m_changeLayoutActions->UpdateActionStatus(decoderIndex, eCopDecoderAction_Disconnect, eActionInProgress);
      m_pCopDecoders[decoderIndex]->DisConnect();
    }
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::SendOpenPortToDecoder(WORD decoderIndex, DWORD artPartyId)
{
  // We need to create the init params for the cop decoder from the party connected to it
  CreateCopDecoderInitParams(decoderIndex, artPartyId);

  // Connect the decoder
  m_changeLayoutActions->UpdateActionStatus(decoderIndex, eCopDecoderAction_Open, eActionInProgress);
  m_pCopDecoders[decoderIndex]->Connect(NO);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::SendConnectPortToDecoder(WORD decoderIndex, DWORD artPartyId)
{
  // In the connect we send update decoder first
  // We need to create the in params for the cop decoder
  // create CBridgePartyVideoInParams for decoder
  CBridgePartyVideoInParams* pInVideoParams = new CBridgePartyVideoInParams();
  BOOL                       isUpdateOK     = UpdateDecoderVideoParams(pInVideoParams, decoderIndex, artPartyId); // TODO
  if (isUpdateOK == FALSE)
  {
    CSmallString cstr;
    cstr << "UpdateDecoderVideoParams failed decoderIndex: " << decoderIndex << " artPartyId: " << artPartyId;
    PTRACE(eLevelInfoNormal, cstr.GetString());
    PASSERT(111);
    // TODO to define error handeling flows
  }
  else
    m_pCopDecoders[decoderIndex]->UpdateVideoInParams(pInVideoParams);

  // Connect the decoder
  m_changeLayoutActions->UpdateActionStatus(decoderIndex, eCopDecoderAction_Connect, eActionInProgress);
  m_pCopDecoders[decoderIndex]->Connect(NO);
  POBJDELETE(pInVideoParams);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::CreateCopDecoderInitParams(WORD decoderIndex, DWORD artPartyId)
{
  // 1) init parameters that not relevant for Cop Decoder
  char partyName[128];                                      // init encoder name
  memset(partyName, '\0', 128);
  sprintf(partyName, "%s%d", "COP_Level_Decoder_#", decoderIndex);

  const CTaskApp*                pParty            = NULL;  // no party task for COP decoder
  const WORD                     wNetworkInterface = AUTO_INTERFACE_TYPE;
  const CBridgePartyMediaParams* pMediaOutParams   = NULL;
  const CBridgePartyCntl*        pBridgePartyCntl  = NULL;
  const char*                    pSiteName         = "COP_Level_Decoder";
  const BYTE                     bCascadeLinkMode  = NO;
  PartyRsrcID                    partyRsrcID       = m_pCopRsrcs->dynamicDecoders[decoderIndex].rsrcEntityId;

  // cretae CBridgePartyVideoInParams for decoder
  CBridgePartyVideoInParams* pInVideoParams        = new CBridgePartyVideoInParams();

  // add video params
  BOOL isUpdateOK                                  = UpdateDecoderVideoParams(pInVideoParams, decoderIndex, artPartyId); // TODO
  if (isUpdateOK == FALSE)
  {
    PASSERT(111);
    // TODO define error handeling flows
  }

  CVideoBridgePartyInitParams* pVideoBridgePartyInitParams = new CVideoBridgePartyInitParams(partyName, pParty, partyRsrcID, wNetworkInterface, pInVideoParams, pMediaOutParams, pBridgePartyCntl, pSiteName, bCascadeLinkMode);

  // set bridge paramameters
  pVideoBridgePartyInitParams->SetBridge(this);
  pVideoBridgePartyInitParams->SetConf(m_pConf);
  pVideoBridgePartyInitParams->SetConfRsrcID(m_confRsrcID);

  m_pCopDecoders[decoderIndex]->Create(pVideoBridgePartyInitParams);

  POBJDELETE(pInVideoParams);
  POBJDELETE(pVideoBridgePartyInitParams);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::CreateLecturerDecoder(CVideoBridgePartyCntlCOP* pLecturerBridgePartyCntl)
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::CreateLecturerDecoder");

  if (pLecturerBridgePartyCntl == NULL)
  {
    PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::CreateLecturerDecoder pLecturerBridgePartyCntl == NULL");
    PASSERT(111);
    return;
  }

  // 1) init parameters that not relevant for Cop Decoder
  char partyName[128];                                      // init encoder name
  memset(partyName, '\0', 128);
  sprintf(partyName, "%s", "COP_Lecturer_Decoder");

  const CTaskApp*                pParty            = NULL;  // no party task for COP decoder
  const WORD                     wNetworkInterface = AUTO_INTERFACE_TYPE;
  const CBridgePartyMediaParams* pMediaOutParams   = NULL;
  const CBridgePartyCntl*        pBridgePartyCntl  = NULL;
  const char*                    pSiteName         = "COP_Lecturer_Decoder";
  const BYTE                     bCascadeLinkMode  = NO;
  PartyRsrcID partyRsrcID                          = m_pCopRsrcs->constRsrcs[eLectureDecoder].rsrcEntityId;

  // cretae CBridgePartyVideoInParams for decoder
  CBridgePartyVideoInParams* pInVideoParams = new CBridgePartyVideoInParams();

  ((CVideoBridgePartyCntlCOP*)pLecturerBridgePartyCntl)->GetInParamFromPartyCntl(pInVideoParams);

  PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::CreateLecturerDecoder video in params:");
  pInVideoParams->DumpVideoParams();

  // cretae CBridgePartyVideoOutParams for encoder
  CBridgePartyVideoOutParams* pOutVideoParams = new CBridgePartyVideoOutParams();
  // add video params
  CCommConf* pCommConf                        = (CCommConf*)m_pConf->GetCommConf();
  // UpdateEncoderVideoParams(pOutVideoParams,pCommConf,m_lecturerlevelEncoderIndex);
  UpdateLecturerCodecsVideoParamsFromLevel(pOutVideoParams, pCommConf, m_lecturerlevelEncoderIndex);

  PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::CreateLecturerDecoder: m_lecturerlevelEncoderIndex = ", m_lecturerlevelEncoderIndex);
  PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::CreateLecturerDecoder video out params:");
  pOutVideoParams->DumpVideoParams();

  // create from level encoder
  pInVideoParams->SetOnlyVideoParams(pOutVideoParams);
  POBJDELETE(pOutVideoParams);

  pInVideoParams->SetCopResourceIndex(LM_DECODER_INDEX);
  pInVideoParams->SetVideConfType(m_VideoConfType);

  CVideoBridgePartyInitParams* pVideoBridgePartyInitParams = new CVideoBridgePartyInitParams(partyName, pParty, partyRsrcID, wNetworkInterface, pInVideoParams, pMediaOutParams, pBridgePartyCntl, pSiteName, bCascadeLinkMode);

  // set bridge paramameters
  pVideoBridgePartyInitParams->SetBridge(this);
  pVideoBridgePartyInitParams->SetConf(m_pConf);
  pVideoBridgePartyInitParams->SetConfRsrcID(m_confRsrcID);

  // create the lecturer decoder
  m_pCopDecoderLecturer->Create(pVideoBridgePartyInitParams);

  POBJDELETE(pInVideoParams);
  POBJDELETE(pVideoBridgePartyInitParams);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::CreateLecturerVSWDecoder(CVideoBridgePartyCntlCOP* pLecturerBridgePartyCntl)
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::CreateLecturerVSWDecoder");

  if (pLecturerBridgePartyCntl == NULL)
  {
    PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::CreateLecturerVSWDecoder pLecturerBridgePartyCntl == NULL");
    PASSERT(111);
    return;
  }

  PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::CreateLecturerVSWDecoder: m_lecturerlevelEncoderIndex=", m_lecturerlevelEncoderIndex);

  // 1) init parameters that not relevant for Cop Decoder
  char partyName[128];                                      // init encoder name
  memset(partyName, '\0', 128);
  sprintf(partyName, "%s", "COP_VSW_Decoder");

  const CTaskApp*                pParty            = NULL;  // no party task for COP decoder
  const WORD                     wNetworkInterface = AUTO_INTERFACE_TYPE;
  const CBridgePartyMediaParams* pMediaOutParams   = NULL;
  const CBridgePartyCntl*        pBridgePartyCntl  = NULL;
  const char*                    pSiteName         = "COP_VSW_Decoder";
  const BYTE                     bCascadeLinkMode  = NO;
  PartyRsrcID partyRsrcID                          = m_pCopRsrcs->constRsrcs[eVswDecoder].rsrcEntityId;

  // cretae CBridgePartyVideoInParams for decoder
  CBridgePartyVideoInParams* pInVideoParams = new CBridgePartyVideoInParams();

  ((CVideoBridgePartyCntlCOP*)pLecturerBridgePartyCntl)->GetInParamFromPartyCntl(pInVideoParams);

  PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::CreateLecturerVSWDecoder - Dump of VideoIn params:");
  pInVideoParams->DumpVideoParams();

  // cretae CBridgePartyVideoOutParams for encoder
  CBridgePartyVideoOutParams* pOutVideoParams = new CBridgePartyVideoOutParams();
  CCommConf* pCommConf                        = (CCommConf*)m_pConf->GetCommConf();
  UpdateLecturerCodecsVideoParamsFromLevel(pOutVideoParams, pCommConf, m_lecturerlevelEncoderIndex);

  PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::CreateLecturerVSWDecoder: m_lecturerlevelEncoderIndex = ", m_lecturerlevelEncoderIndex);
  PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::CreateLecturerVSWDecoder - Dump of VideoOut params:");
  pOutVideoParams->DumpVideoParams();

  // create from level encoder
  pInVideoParams->SetOnlyVideoParams(pOutVideoParams);
  POBJDELETE(pOutVideoParams);

  pInVideoParams->SetCopResourceIndex(VSW_DECODER_INDEX);
  pInVideoParams->SetVideConfType(m_VideoConfType);

  CVideoBridgePartyInitParams* pVideoBridgePartyInitParams = new CVideoBridgePartyInitParams(partyName, pParty, partyRsrcID, wNetworkInterface, pInVideoParams, pMediaOutParams, pBridgePartyCntl, pSiteName, bCascadeLinkMode);

  // set bridge paramameters
  pVideoBridgePartyInitParams->SetBridge(this);
  pVideoBridgePartyInitParams->SetConf(m_pConf);
  pVideoBridgePartyInitParams->SetConfRsrcID(m_confRsrcID);

  // create the lecturer decoder
  m_pCopDecoderVsw->Create(pVideoBridgePartyInitParams);

  POBJDELETE(pInVideoParams);
  POBJDELETE(pVideoBridgePartyInitParams);
}

////////////////////////////////////////////////////////////////////////////
CVideoBridgePartyCntl* CVideoBridgeCOP::GetVideoBridgePartyCntl(DWORD artPartyId)
{
	return (CVideoBridgePartyCntl*)m_pPartyList->Find(artPartyId);
}

////////////////////////////////////////////////////////////////////////////
BOOL CVideoBridgeCOP::UpdateDecoderVideoParams(CBridgePartyVideoInParams* pInVideoParams, WORD decoderIndex, DWORD artPartyId)
{
	// Update the decoder in params according to the party that will be connected to it
	CVideoBridgePartyCntlCOP* pPartyCntl = (CVideoBridgePartyCntlCOP*)m_pPartyList->Find(artPartyId);
	if (pPartyCntl)
	{
		pPartyCntl->GetInParamFromPartyCntl(pInVideoParams);
		pInVideoParams->SetCopResourceIndex(decoderIndex);
		pInVideoParams->SetVideConfType(m_VideoConfType);
		return TRUE;
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////
WORD CVideoBridgeCOP::GetDecoderIndexInLayout(CLayout& layout, WORD decoder_index, ECopDecoderResolution& requiredResolution)
{
	CCopLayoutResolutionTable table;
	WORD index =  table.GetDecoderIndexInLayout(m_VideoConfType, layout.GetLayoutType(), decoder_index, requiredResolution);

	if (index != (WORD)-1 && layout[index] && layout[index]->GetImageId() != 0)
		return index;

	return (WORD)-1;
}


////////////////////////////////////////////////////////////////////////////
const char* CVideoBridgeCOP::ForceLecturer(CLayout& rResultLayout) const
{
  const char* partyName = NULL;
  partyName = GetLecturerName();
  if (partyName == NULL)
    return NULL;

  // Romem - klocwork
  if((rResultLayout)[0])
  {
	  (rResultLayout)[0]->SetPartyForceName(partyName);
	  (rResultLayout)[0]->SetForceAttributes(OPERATOR_Prior, FORCE_CONF_Activ);
  }

  return partyName;
}


////////////////////////////////////////////////////////////////////////////
BYTE CVideoBridgeCOP::IsLecturerForced(CLayout& rResultLayout) const
{
  BYTE        is_lecturer_forced = NO;
  const char* partyName          = NULL;
  partyName = GetLecturerName();

  if (partyName == NULL)
    return NO;

  WORD numbSubImg = rResultLayout.GetNumberOfSubImages();
  for (WORD i = 0; i < numbSubImg; i++)
  {
	if((rResultLayout)[i] == NULL)
		  continue;
    if ((rResultLayout)[i]->isForcedInConfLevel() && !strncmp(partyName, ((rResultLayout)[i]->GetPartyForce()), strlen(partyName)))
    {
      is_lecturer_forced = YES;
      break;
    }
  }
  return is_lecturer_forced;
}



////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::UpdateLectureDecoderParamsInImage(CLayout& rResultLayout) const
{
	// to go over all the sub images and if there is an image
	// to set the m_source_connection_id and m_source_partyRsrc_id of the decoder according to the global topology table
	LayoutType layoutType = rResultLayout.GetLayoutType();
	WORD numSubImages = GetNumbSubImg(layoutType);

	// set layout type
	TRACECOND_AND_RETURN(CP_LAYOUT_1X1 != layoutType, "Failed, The layout is not 1x1");
	TRACECOND_AND_RETURN(numSubImages != 1, "Failed, The number of sub-images is not 1");

	CVidSubImage* pVidSubImage = rResultLayout[0];
	TRACECOND_AND_RETURN(!pVidSubImage, "Failed, Invalid pVidSubImage");

	DWORD partyRscId = pVidSubImage->GetImageId();
	CImage* pImage = GetPartyImageLookupTable()->GetPartyImage(partyRscId);
	TRACECOND_AND_RETURN(!pImage, "PartyId:" << partyRscId);

	ECopEncoderMaxResolution encoder_max_resolution = GetCopEncoderMaxResolution();

	CCopLayoutResolutionTable copLayoutResolutionTable;
	ECopDecoderResolution decoder_resolution;
	WORD decoderConnectionIdIndex;
	WORD ret_value = copLayoutResolutionTable.GetCellResolutionDef(encoder_max_resolution, layoutType, 0, decoder_resolution, decoderConnectionIdIndex);
	PASSERTSTREAM_AND_RETURN(ret_value == (WORD)-1, "PartyId:" << partyRscId << ", Index:" << 0);

	DWORD decoderConnectionId = m_pCopRsrcs->constRsrcs[eLectureDecoder].connectionId;
	DWORD decoderPartyId      = m_pCopRsrcs->constRsrcs[eLectureDecoder].rsrcEntityId;

	PartyRsrcID artPartyId = pImage->GetArtPartyId();

	CVideoBridgePartyCntlCOP* pPartyCntl = (CVideoBridgePartyCntlCOP*)m_pPartyList->Find(artPartyId);
	if (pPartyCntl)
	{
		pPartyCntl->SetDecoderConnectionIdInImage(decoderConnectionId);
		pPartyCntl->SetDecoderPartyIdInImage(decoderPartyId);
	}
	else
	{
		PASSERTSTREAM_AND_RETURN(1, "PartyId:" << artPartyId << " - Party not found");
	}

	DWORD oldDecoderConnectionId = pImage->GetConnectionId();
	DWORD oldDecoderPartyId      = pImage->GetPartyRsrcId();

	TRACEINTO
	          << "Sub-image index:0"
	          << ", decoderConnectionId:"    << decoderConnectionId
	          << ", oldDecoderConnectionId:" << oldDecoderConnectionId
	          << ", decoderPartyId:"         << decoderPartyId
	          << ", oldDecoderPartyId:"      << oldDecoderPartyId
	          << ", artPartyId:"             << artPartyId;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::SwitchlecturerDecoderIdInImage(CLayout& rResultLayout, ECopConstRsrcs decoderType) const
{
	// to go over all the sub images and if there is an image
	// to set the m_source_connection_id and m_source_partyRsrc_id of the decoder according to the global topology table
	LayoutType layoutType   = rResultLayout.GetLayoutType();
	WORD       numSubImages = GetNumbSubImg(layoutType);

	TRACECOND_AND_RETURN(CP_LAYOUT_1X1 != layoutType, "CVideoBridgeCOP::SwitchlecturerDecoderIdInImage - Failed, The layout is not 1x1");
	TRACECOND_AND_RETURN(numSubImages != 1, "CVideoBridgeCOP::SwitchlecturerDecoderIdInImage - Failed, The number of sub-images is not 1");
	if (decoderType != eVswDecoder && decoderType != eLectureDecoder) {
		TRACECOND_AND_RETURN(1, "CVideoBridgeCOP::SwitchlecturerDecoderIdInImage - Failed, Illegal decoder type, decoderType:" << decoderType);
	}

	CVidSubImage* pVidSubImage = rResultLayout[0];
	TRACECOND_AND_RETURN(!pVidSubImage, "CVideoBridgeCOP::SwitchlecturerDecoderIdInImage - Failed, Invalid pVidSubImage");

	DWORD partyRscId = pVidSubImage->GetImageId();
	CImage* pImage = GetPartyImageLookupTable()->GetPartyImage(partyRscId);
	TRACECOND_AND_RETURN(!pImage, "CBridgePartyVideoOut::SwitchlecturerDecoderIdInImage - Failed, The lookup table doesn't have an element, PartyId:" << partyRscId);

	DWORD oldDecoderConnectionId = pImage->GetConnectionId();
	DWORD oldDecoderPartyId      = pImage->GetPartyRsrcId();
	DWORD decoderConnectionId    = m_pCopRsrcs->constRsrcs[decoderType].connectionId;
	DWORD decoderPartyId         = m_pCopRsrcs->constRsrcs[decoderType].rsrcEntityId;

	pImage->SetCopDecoderEntityID(decoderPartyId);
	pImage->SetCopDecoderConnectionID(decoderConnectionId);

	TRACEINTO << "CVideoBridgeCOP::SwitchlecturerDecoderIdInImage "
	          << ", decoderConnectionId:"    << decoderConnectionId
	          << ", oldDecoderConnectionId:" << oldDecoderConnectionId
	          << ", decoderPartyId:"         << decoderPartyId
	          << ", oldDecoderPartyId:"      << oldDecoderPartyId;
}


////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::ConnectPCMEncoderToPCMMenu()
{
  m_pPCMEncoderCntl->ConnectPCMEncoderToPCMMenu(m_pcmMenuId);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::DisconnectPCMEncoderFromPCMMenu()
{
  m_pPCMEncoderCntl->DisConnectPCMEncoderFromPCMMenu(m_pcmMenuId);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfDisConnectConfINSWITCH(CSegment* pParam)
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::OnConfDisConnectConfINSWITCH");
  StopSwitchActionsOnDisconnecting();
  // Change to disconnecting state - to prevent connecting new parties
  m_state = DISCONNECTING;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfTerminateDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << "State:" << m_state;

	StartTimer(BRIDGE_DISCONNECT_TOUT, COP_BRIDGE_DISCONNECT_TIME_OUT_VALUE);
	CloseDynamicDecoders();
	CloseLevelEncoders(); //
	ClosePcmEncoder();

	if (m_pCopLectureModeCntl->IsLectureModeActive())
	{
		CloseLecturerCodecs();
	}
	else
	{
		// in case the LM is not active we just need to free the memory //VNGR-13391
		POBJDELETE(m_pCopDecoderLecturer);
		POBJDELETE(m_pCopDecoderVsw);
		POBJDELETE(m_pCopEncoderVsw);
	}

	if (m_pPartyList->size())
	{
		// Upon receiving TERMINATE event, Bridge should be empty
		TRACEINTO << *m_pPartyList << " - Bridge not empty";
		PASSERT(101);
	}
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnTimerDisconnetDISCONNECTING(CSegment* pParam)
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::OnTimerDisconnetDISCONNECTING");
  DBGPASSERT(BRIDGE_DISCONNECT_TOUT);
  m_resourcesStatus = STATUS_FAIL;
  EndDisconnect();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::EndDisconnect()
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::EndDisconnect");

  if (IsValidTimer(BRIDGE_DISCONNECT_TOUT))
    DeleteTimer(BRIDGE_DISCONNECT_TOUT);

  WORD status = m_resourcesStatus;
  m_state = IDLE;
  m_pConfApi->EndVidBrdgDisConnect(status);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::CreateAndNewPartyCntl(CVideoBridgePartyCntl*& pVideoBrdgPartyCntl, CVideoBridgePartyInitParams* pVideoBridgePartyInitParams)
{
  NewPartyCntl(pVideoBrdgPartyCntl);
  pVideoBrdgPartyCntl->Create(pVideoBridgePartyInitParams);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::NewPartyCntl(CVideoBridgePartyCntl*& pVideoBrdgPartyCntl)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::NewPartyCntl  ConfName - ", m_pConfName);
  pVideoBrdgPartyCntl = new CVideoBridgePartyCntlCOP();
}

////////////////////////////////////////////////////////////////////////////
CLayout* CVideoBridgeCOP::GetReservationLayout(void) const
{
  return m_pReservation[m_layoutType];
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::ConnectPartyToPCMEncoder(PartyRsrcID partyId)
{
  if (m_state == DISCONNECTING)
  {
    PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::ConnectPartyToPCMEncoder, state is DISCONNECTING - do nothing");
    return;
  }

  CVideoBridgePartyCntlCOP* pPartyCntl = (CVideoBridgePartyCntlCOP*)GetPartyCntl(partyId);
  PASSERT_AND_RETURN(pPartyCntl == NIL(CVideoBridgePartyCntlCOP));

  // FlowControl support when connecting party to PCM encoder
  if (pPartyCntl->GetPartyFlowControlRate() != 0)
  {
    BOOL isConnectToPCM = TRUE;
    UpdateFlowControlRateWhenPartyConnectOrDisconnectsFromPCMEncoder(pPartyCntl, isConnectToPCM);
  }

  UpdatePCMEncoderWithNewLevelParamsIfNeeded(pPartyCntl);

  CopRsrcDesc pcmRsrcDesc = GetCopRsrcDesc(ePcmEncoderType);
  // send the party the PCM rsrc params
  pPartyCntl->ConnectToPCMEncoder(pcmRsrcDesc.connectionId, pcmRsrcDesc.rsrcEntityId);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::DisconnectPartyFromPCMEncoder(PartyRsrcID partyId)
{
  if (m_state == DISCONNECTING)
  {
    PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::DisconnectPartyFromPCMEncoder, state is DISCONNECTING - do nothing");
    return;
  }

  CVideoBridgePartyCntlCOP* pPartyCntl = (CVideoBridgePartyCntlCOP*)GetPartyCntl(partyId);
  PASSERT_AND_RETURN(pPartyCntl == NIL(CVideoBridgePartyCntlCOP));

  pPartyCntl->DisconnectFromPCMEncoder();

  // FlowControl support when disconnecting party to PCM encoder
  if (pPartyCntl->GetPartyFlowControlRate() != 0)
  {
    BOOL isConnectToPCM = FALSE;
    UpdateFlowControlRateWhenPartyConnectOrDisconnectsFromPCMEncoder(pPartyCntl, isConnectToPCM);
  }
}

////////////////////////////////////////////////////////////////////////////
DWORD CVideoBridgeCOP::GetPartyResolutionInPCMTerms(PartyRsrcID partyId)
{
  if (m_state == DISCONNECTING)
  {
    PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::GetPartyResolutionInPCMTerms, state is DISCONNECTING - do nothing");
    return ePcmIllegalImageParams;
  }

  CVideoBridgePartyCntlCOP* pPartyCntl = (CVideoBridgePartyCntlCOP*)GetPartyCntl(partyId);
  PASSERT_AND_RETURN_VALUE(pPartyCntl == NIL(CVideoBridgePartyCntlCOP), ePcmIllegalImageParams);

  DWORD levelEncoderEntityId           = pPartyCntl->GetCopEncoderEntityId();
  WORD  levelEncoderIndex              = GetCopEncoderIndexFromEntityId(levelEncoderEntityId);
  if (levelEncoderIndex == VSW_ENCODER_INDEX)
    levelEncoderIndex = m_lecturerlevelEncoderIndex;

  CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();

  if (!CPObject::IsValidPObjectPtr(pCommConf))
  {
    PASSERT_AND_RETURN_VALUE(111, ePcmIllegalImageParams);
  }

  CCOPConfigurationList* pCOPConfigurationList = pCommConf->GetCopConfigurationList();
  if (!CPObject::IsValidPObjectPtr(pCOPConfigurationList))
  {
    PASSERT_AND_RETURN_VALUE(111, ePcmIllegalImageParams);
  }

  CCopVideoParams* pCopEncoderLevelParams = pCOPConfigurationList->GetVideoMode(levelEncoderIndex);
  if (!CPObject::IsValidPObjectPtr(pCopEncoderLevelParams))
  {
    PASSERT_AND_RETURN_VALUE(111, ePcmIllegalImageParams);
  }

  // Translate of GUI params to pcmImageParams
  pcmImageParams     imageParams;
  CCopVideoModeTable videoModeTable;
  videoModeTable.GetPartyResolutionInPCMTerms(pCopEncoderLevelParams, imageParams);

  return imageParams;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnPartyConnectedToPCMEncoder(CSegment* pParam)
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::OnPartyConnectedToPCMEncoder");

  m_pcmConnected = TRUE;

  CTaskApp* pParty         = NULL;
  BYTE      responseStatus = statIllegal;

  *pParam >> (void*&)pParty >> responseStatus;

  // lecture mode has started / ended and the new layout is 1x1,
  // there are cases that the last layout that is saved in m_pPCMEncoderCntl is with
  // the same image as in the new layout but with different decoder id --> we will send the change layout anyway
  BYTE sendChangeLayoutAnyWay = YES;
  if (IsActiveLectureMode() && (!IsLecturerConnectedToPCM() || IsOnlyLecturerConnected()))
    m_pPCMEncoderCntl->ChangeLayout(m_ConfLayout1x1, m_pVisualEffects, NULL, INVALID, sendChangeLayoutAnyWay);
  else
    m_pPCMEncoderCntl->ChangeLayout(m_ConfLayout, m_pVisualEffects, NULL, GetAudioSpeakerPlaceInLayout(), sendChangeLayoutAnyWay);

  m_pPCMEncoderCntl->FastUpdate();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnPartyDisconnectedFromPCMEncoder(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::OnPartyDisconnectedFromPCMEncoder");
	m_pcmConnected = FALSE;

	CParty* pParty = NULL;
	BYTE responseStatus = statIllegal;

	*pParam >> (void*&)pParty >> responseStatus;

	if (!CPObject::IsValidPObjectPtr(pParty))
	{
		PTRACE2(eLevelError, "CVideoBridgeCOP::OnPartyDisconnectedFromPCMEncoder received pParty==NULL, Name - ", m_pConfName);
		PASSERT(111);
		return;
	}

	CVideoBridgePartyCntlCOP* pPartyCntl = (CVideoBridgePartyCntlCOP*)GetPartyCntl(pParty->GetPartyRsrcID());
	if (!CPObject::IsValidPObjectPtr(pPartyCntl))
	{
		PTRACE(eLevelError, "CVideoBridgeCOP::OnPartyDisconnectedFromPCMEncoder, cant get CVideoBridgePartyCntl using pParty");
		PASSERT(111);
		return;
	}

	DWORD levelEncoderEntityId = pPartyCntl->GetCopEncoderEntityId();
	WORD levelEncoderIndex = GetCopEncoderIndexFromEntityId(levelEncoderEntityId);

	PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::OnPartyDisconnectedFromPCMEncoder - send intra req from encoder index: ", levelEncoderIndex);
	if (levelEncoderIndex <= NUM_OF_LEVEL_ENCODERS - 1)
	{
		m_pCopLevelEncoders[levelEncoderIndex]->FastUpdate();
	}
	else if (levelEncoderIndex == VSW_ENCODER_INDEX)
	{
		PTRACE2(eLevelError, "CVideoBridgeCOP::OnPartyDisconnectedFromPCMEncoder Ask fast update from lecturer, Name - ", m_pConfName);
		FastUpdateFromLecturer();
	}
	else
	{
		PTRACE2(eLevelError, "CVideoBridgeCOP::OnPartyDisconnectedFromPCMEncoder invalid levelEncoderIndex - not sending fast update, Name - ", m_pConfName);
		PASSERT(levelEncoderIndex);
	}
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnPCMEncoderParamsUpdated(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::OnPCMEncoderParamsUpdated");

	m_pcmConnected = TRUE;

	CParty* pParty = NULL;
	BYTE responseStatus = statIllegal;

	*pParam >> (void*&)pParty >> responseStatus;

	if (!CPObject::IsValidPObjectPtr(pParty))
	{
		PTRACE2(eLevelError, "CVideoBridgeCOP::OnPCMEncoderParamsUpdated received pParty==NULL, Name - ", m_pConfName);
		PASSERT(111);
		return;
	}

	CVideoBridgePartyCntlCOP* pPartyCntl = (CVideoBridgePartyCntlCOP*)GetPartyCntl(pParty->GetPartyRsrcID());
	if (!CPObject::IsValidPObjectPtr(pPartyCntl))
	{
		PTRACE(eLevelError, "CVideoBridgeCOP::OnPCMEncoderParamsUpdated, cant get CVideoBridgePartyCntl using pParty");
		PASSERT(111);
		return;
	}

	UpdatePCMEncoderWithNewLevelParamsIfNeeded(pPartyCntl);
}


////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnAutoScanTimerConnected(CSegment* pParam)
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::OnAutoScanTimerConnected");

  CLayout* pNewLayout = new CLayout(*m_ConfLayout);
  if (FillAutoScanImageInLayout(*pNewLayout) == FALSE)
  {
    return;
  }

  // update decoders connection id and entity id in images
  UpdateDecodersParamsInImages(*pNewLayout);

  if (!IsLayoutChange(pNewLayout, m_ConfLayout)) {
    PASSERT_AND_RETURN(1);
  }

  m_state = INSWITCH;
  // calculate change layout actions
  CalculateChangeLayoutActions(*pNewLayout, *m_ConfLayout);  // set into ChangeLayoutActionsManager m_changeLayoutActionsManager

  DWORD smartSwitchTout             = 0;
  WORD  closeDisconnectDecoderIndex = NUM_OF_COP_DECODERS;
  if (IsSmartSwitchNeeded(pNewLayout, m_ConfLayout, closeDisconnectDecoderIndex))
  {
    PTRACE2(eLevelInfoNormal, "***CVideoBridgeCOP::OnAutoScanTimerConnected -  IsSmartSwitchNeeded TRUE!!!!!!!!!!!, ConfName - ", m_pConfName);
    m_IsSmartSwitch = TRUE;
    smartSwitchTout = GetSmartSwitchTout();
  }

  StartSwitchTimer();

  UpdateConfLayout(pNewLayout);

  // execute change mode for party and decoders
  if (m_IsSmartSwitch)
  {
    StartSmartSwitch(smartSwitchTout, closeDisconnectDecoderIndex);
  }
  else
  {
    if (m_IsDSPSmartSwitch)
    {
      PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnAutoScanTimerConnected , StartDSPSmartSwitch ConfName - ", m_pConfName);
      StartDSPSmartSwitch();
      m_IsSmartSwitch = FALSE;
    }
    else
    {
      StartSwitch();
    }
  }

  POBJDELETE(pNewLayout);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::EndChangeLayoutActions()
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::EndChangeLayoutActions ConfName - ", m_pConfName);

  DeleteTimer(SWITCH_TOUT);
  SendChangeLayoutToAllLevelEncoders();
  m_state = CONNECTED;
  UpdateDB_ConfLayout();


  m_IsSmartSwitch    = FALSE;
  m_IsDSPSmartSwitch = FALSE;
  InformACLayoutChangeCompleted();

  // 1. Send disconnect to all the parties that we didn't disconnect because we were inswitch
  SendDisconnectToAllPartiesInSwitchVector();

  // Switch In Switch
  if (m_SwitchInSwitch)
  {
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::EndChangeLayoutActions SWITCH IN SWITCH, ConfName - ", m_pConfName);
    if (m_pCopLectureModeCntl->GetWaitingToEndChangeLayout())
    {
      // only trace so we can know we are waiting
      PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::EndChangeLayoutActions do pending lecture mode action waiting for end of SWITCH IN SWITCH, ConfName - ", m_pConfName);
    }

    // 2. Start the new changeLayout
    LayoutType newLayoutType = m_SwitchInSwitchLayoutType;
    m_SwitchInSwitch           = NO;
    m_SwitchInSwitchLayoutType = CP_LAYOUT_1X1;
    ChangeLayout(newLayoutType, FALSE, NO);
  }
  else
  {
    DoPendingLectureModeActionOnEndOfChangeLayout();
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::InformACLayoutChangeCompleted(BYTE just_send)
{
  if (just_send)
  {
    // inform AC
    m_pConfApi->StartLookForActiveSpeaker();
    return;
  }

  if (m_isChangeLayoutBecauseOfSpeakerChange)
  {
    // inform AC layout has changed --> start look for a new active speaker
    m_pConfApi->StartLookForActiveSpeaker();
  }

  m_isChangeLayoutBecauseOfSpeakerChange        = m_isPendingChangeLayoutBecauseOfSpeakerChange;
  m_isPendingChangeLayoutBecauseOfSpeakerChange = FALSE;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::CloseLevelEncoders()
{
  for (DWORD encoder_index = 0; encoder_index < NUM_OF_LEVEL_ENCODERS; encoder_index++)
  {
    if (CPObject::IsValidPObjectPtr(m_pCopLevelEncoders[encoder_index]))
      m_pCopLevelEncoders[encoder_index]->DisConnect();
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::CloseDynamicDecoders()
{
  for (DWORD decoder_index = 0; decoder_index < NUM_OF_COP_DECODERS; decoder_index++)
  {
    if (CPObject::IsValidPObjectPtr(m_pCopDecoders[decoder_index]))
    {
      if (m_pCopDecoders[decoder_index]->GetState() == IDLE)
        DestroyCopDecoder(decoder_index);
      else
        m_pCopDecoders[decoder_index]->Close();
    }
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::ClosePcmEncoder()
{
  DisconnectPCMEncoderFromPCMMenu();

  m_pPCMEncoderCntl->DisConnect();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::DestroyDynamicDecoders()
{
  for (WORD decoderIndex = 0; decoderIndex < NUM_OF_COP_DECODERS; decoderIndex++)
  {
    if (CPObject::IsValidPObjectPtr(m_pCopDecoders[decoderIndex]))
    {
      POBJDELETE(m_pCopDecoders[decoderIndex]);
    }
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnLevelEncoderEndDisconnectDISCONNECTING(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnLevelEncoderEndDisconnectDISCONNECTING, ConfName - ", m_pConfName);

  WORD            receivedStatus = statOK;
  EMediaDirection eMediaDirection;
  DWORD           encoderIndex;
  CTaskApp*       pParty         = NULL;
  *pParam >> (void*&)pParty;
  *pParam >> (WORD&)receivedStatus;
  *pParam >> (WORD&)eMediaDirection;
  *pParam >> (DWORD&)encoderIndex;

  if (receivedStatus != statOK)
  {
    PASSERT(receivedStatus);
    m_resourcesStatus = receivedStatus;
  }

  if (encoderIndex == PCM_RESOURCE_INDEX)
  {
    POBJDELETE(m_pPCMEncoderCntl);
  }
  // VNGR-13391
  else if (encoderIndex == VSW_ENCODER_INDEX)
  {
    POBJDELETE(m_pCopEncoderVsw);
  }
  else
  {
    DestroyCopEncoder(encoderIndex);
  }

  if (AreAllCopRsrcsDestroyed())
  {
    EndDisconnect();
  }
  else
  {
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnLevelEncoderEndDisconnectDISCONNECTING not all COP the resources are destroyed, ConfName - ", m_pConfName);
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnLevelEncoderEndDisconnectCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnLevelEncoderEndDisconnectCONNECTED, ConfName - ", m_pConfName);

  WORD            receivedStatus = statOK;
  EMediaDirection eMediaDirection;
  DWORD           encoderIndex;
  CTaskApp*       pParty         = NULL;
  *pParam >> (void*&)pParty;
  *pParam >> (WORD&)receivedStatus;
  *pParam >> (WORD&)eMediaDirection;
  *pParam >> (DWORD&)encoderIndex;

  if (receivedStatus != statOK)
  {
    PASSERT(receivedStatus);
    m_resourcesStatus = receivedStatus;
  }

  if (encoderIndex == VSW_ENCODER_INDEX)
  {
    if (IsLectureCodecsClosed())
    {
      EndCloseLecturerCodecs();
    }
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnLevelEncoderEndDisconnectINSWITCH(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnLevelEncoderEndDisconnectINSWITCH, ConfName - ", m_pConfName);

  WORD            receivedStatus = statOK;
  EMediaDirection eMediaDirection;
  DWORD           encoderIndex;
  CTaskApp*       pParty         = NULL;
  *pParam >> (void*&)pParty;
  *pParam >> (WORD&)receivedStatus;
  *pParam >> (WORD&)eMediaDirection;
  *pParam >> (DWORD&)encoderIndex;

  if (receivedStatus != statOK)
  {
    PASSERT(receivedStatus);
    m_resourcesStatus = receivedStatus;
  }

  if (encoderIndex == VSW_ENCODER_INDEX)
  {
    if (IsLectureCodecsClosed())
    {
      EndCloseLecturerCodecs();
    }
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::DestroyCopEncoder(WORD copEncoderIndex)
{
  PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::DestroyCopEncoder, copEncoderIndex = ", copEncoderIndex);
  if (copEncoderIndex >= NUM_OF_LEVEL_ENCODERS) {
    PASSERT_AND_RETURN(copEncoderIndex);
  }

  POBJDELETE(m_pCopLevelEncoders[copEncoderIndex]);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::DestroyCopDecoder(WORD copDecoderIndex)
{
  PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::DestroyCopDecoder, copDecoderIndex = ", copDecoderIndex);
  if (copDecoderIndex >= NUM_OF_COP_DECODERS) {
    PASSERT_AND_RETURN(copDecoderIndex);
  }

  POBJDELETE(m_pCopDecoders[copDecoderIndex]);
}

////////////////////////////////////////////////////////////////////////////
BOOL CVideoBridgeCOP::AreAllCopRsrcsDestroyed()
{
  BOOL areAllCopRsrcsDestroyed = TRUE;
  // 1. check AreAllLevelEncodersDestroyed();//
  for (DWORD encoder_index = 0; encoder_index < NUM_OF_LEVEL_ENCODERS; encoder_index++)
  {
    if (CPObject::IsValidPObjectPtr(m_pCopLevelEncoders[encoder_index]))
    {
      areAllCopRsrcsDestroyed = FALSE;
      return areAllCopRsrcsDestroyed;
    }
  }

  // VNGR-13391
  if (CPObject::IsValidPObjectPtr(m_pCopDecoderLecturer))
    return FALSE;

  if (CPObject::IsValidPObjectPtr(m_pCopDecoderVsw))
    return FALSE;

  if (CPObject::IsValidPObjectPtr(m_pCopEncoderVsw))
    return FALSE;

  for (int i = 0; i < NUM_OF_COP_DECODERS; i++)
  {
    if (CPObject::IsValidPObjectPtr(m_pCopDecoders[i]))
    {
      areAllCopRsrcsDestroyed = FALSE;
      return areAllCopRsrcsDestroyed;
    }
  }
  return areAllCopRsrcsDestroyed;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::StopSwitchActionsOnDisconnecting()
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::StopSwitchActionsOnDisconnecting");

  SendDisconnectToAllPartiesInSwitchVector();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::RemovePartyFromConfMixBeforeDisconnecting(const CTaskApp* pParty)
{
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pParty));

	CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(((CParty*)pParty)->GetPartyRsrcID());
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pPartyCntl));

	for (DWORD j = (int)CP_LAYOUT_1X1; j < (int)CP_NO_LAYOUT; j++)
		m_pReservation[j]->RemovePartyForce(pPartyCntl->GetName(), PARTY_Prior);

	// make that all other parties can`t see the deleted source
	if (m_pAutoScanParams)
		m_pAutoScanParams->RemoveImageFromAutoScanVector(pPartyCntl->GetPartyRsrcID());

	BYTE result = DelPartyImage(pPartyCntl->GetPartyRsrcID());
	if (YES == result)
	{
		ApplicationActionsOnRemovePartyFromMix(pPartyCntl);
		// lecturer will change layout at the end of ApplicationActionsOnRemovePartyFromMix
		if (!IsLecturer(pPartyCntl))
		{
			// change conf COP layout
			ChangeLayout(m_layoutType);
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfDisConnectPartyCONNECTED(CSegment* pParam)
{
	CBridgePartyDisconnectParams partyDisconnectParams;
	partyDisconnectParams.DeSerialize(NATIVE, *pParam);

	PartyRsrcID partyId = partyDisconnectParams.GetPartyId();
	EMediaDirection eMediaDirection = partyDisconnectParams.GetMediaDirection();

	TRACEINTO << "PartyId:" << partyId << ", MediaDirection:" << eMediaDirection;

	const CTaskApp* pParty = (CTaskApp*)GetLookupTableParty()->Get(partyId);
	PASSERT_AND_RETURN(!pParty);

	CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
	if (!pPartyCntl)
	{
		TRACEINTO << "PartyId:" << partyId << " - Party already disconnected";
		m_pConfApi->PartyBridgeResponseMsg(pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_DISCONNECTED, statOK, 1, eNoDirection);
		return;
	}

	// if party video out is disconnecting and it has FlowControl rate
	if ((eMediaDirection == eMediaOut || eMediaDirection == eMediaInAndOut) && ((CVideoBridgePartyCntlCOP*)pPartyCntl)->GetPartyFlowControlRate() != 0)
	{
		UpdateFlowControlRateWhenPartyDisconnectsFromEncoder((CVideoBridgePartyCntlCOP*)pPartyCntl);
	}

	// if video out is disconnecting we won't remove party from Mix
	BOOL canDisconnectParty = TRUE;
	if (eMediaDirection == eMediaIn || eMediaDirection == eMediaInAndOut)
	{
		// in case the disconnecting party is part of the current change layout we will disconnect the party after the switch finishes
		RemovePartyFromConfMixBeforeDisconnecting(pParty);
		if (IsSwitchWithDisconnectedParty(partyId))
			canDisconnectParty = FALSE;
	}

	if (IsActiveLectureMode() && m_pCopLectureModeCntl->GetUpdateCascadeLinkAsNotLecturer())
	{
		if (IsPrevCascadeLinkLecturer(pPartyCntl))
		{
			BYTE isPartyCascadeLinkSupportAsymmetricEMCascade = ((CVideoBridgePartyCntlCOP*)pPartyCntl)->IsPartyCascadeLinkSupportAsymmetricEMCascade();
			TRACEINTO << "PartyId:" << partyId << ", isPartyCascadeLinkSupportAsymmetricEMCascade:" << (int)isPartyCascadeLinkSupportAsymmetricEMCascade;
			PASSERT(!isPartyCascadeLinkSupportAsymmetricEMCascade);
			UpdateCascadeLinkAsNotLecturerOnDisconnectingCascadeLink();
		}
	}

	if (canDisconnectParty)
	{
		pPartyCntl->SetDisConnectingDirectionsReq(eMediaDirection);
		CBridge::DisconnectParty(partyId);
	}
	else
	{
		TRACEINTO << "PartyId:" << partyId << " - Can't disconnect party now";
		AddPartyToDisconnectVectorAfterSwitchEnds(partyId, eMediaDirection);
	}
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfDisConnectPartyINSWITCH(CSegment* pParam)
{
	CBridgePartyDisconnectParams partyDisconnectParams;
	partyDisconnectParams.DeSerialize(NATIVE, *pParam);

	PartyRsrcID partyId = partyDisconnectParams.GetPartyId();
	EMediaDirection eMediaDirection = partyDisconnectParams.GetMediaDirection();

	TRACEINTO << "PartyId:" << partyId << ", MediaDirection:" << eMediaDirection;

	const CTaskApp* pParty = (CTaskApp*)GetLookupTableParty()->Get(partyId);
	PASSERT_AND_RETURN(!pParty);

	CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
	if (!pPartyCntl)
	{
		TRACEINTO << "PartyId:" << partyId << " - Party already disconnected";
		m_pConfApi->PartyBridgeResponseMsg(pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_DISCONNECTED, statOK, 1, eNoDirection);
		return;
	}

	// if party video out is disconnecting and it has FlowControl rate
	if ((eMediaDirection == eMediaOut || eMediaDirection == eMediaInAndOut) && ((CVideoBridgePartyCntlCOP*)pPartyCntl)->GetPartyFlowControlRate() != 0)
	{
		UpdateFlowControlRateWhenPartyDisconnectsFromEncoder((CVideoBridgePartyCntlCOP*)pPartyCntl);
	}

	// if video out is disconnecting we won't remove party from Mix
	BOOL canDisconnectParty = TRUE;
	if (eMediaDirection == eMediaIn || eMediaDirection == eMediaInAndOut)
	{
		// in case the disconnecting party is part of the current change layout we will disconnect the party after the switch finishes
		if (IsSwitchWithDisconnectedParty(partyId))
			canDisconnectParty = FALSE;
		RemovePartyFromConfMixBeforeDisconnecting(pParty);
	}

	if (canDisconnectParty)
	{
		pPartyCntl->SetDisConnectingDirectionsReq(eMediaDirection);
		CBridge::DisconnectParty(partyId);
	}
	else
	{
		TRACEINTO << "PartyId:" << partyId << " - Can't disconnect party now";
		AddPartyToDisconnectVectorAfterSwitchEnds(partyId, eMediaDirection);
	}
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfDeletePartyFromConfCONNECTED(CSegment* pParam)
{
  OnConfDeletePartyFromConf(pParam);

  // change conf COP layout
  // vngr-18616
  ChangeLayout(m_layoutType);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfDeletePartyFromConfINSWITCH(CSegment* pParam)
{
  OnConfDeletePartyFromConf(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfDeletePartyFromConf(CSegment* pParam)
{
  char deletedPartyName[H243_NAME_LEN];
  *pParam >> deletedPartyName;
  deletedPartyName[H243_NAME_LEN-1] = '\0';

  RemovePartyFromAnyConfSettingsWhenDeletedFromConf(deletedPartyName);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfUpdateVideoMuteCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfUpdateVideoMuteCONNECTED : Name - ", m_pConfName);
  OnConfUpdateVideoMute(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfUpdateVideoMuteINSWITCH(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfUpdateVideoMuteINSWITCH : Name - ", m_pConfName);
  OnConfUpdateVideoMute(pParam);                // need to see if this implementation is enought on inswitch
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfUpdateVideoMute(CSegment* pParam)
{
  WORD                   srcReq          = 0;
  EOnOff                 eOnOff          = eOff;
  EMediaDirection        eMediaDirection = eNoDirection;
  CVideoBridgePartyCntl* pPartyCntl      = NULL;

  *pParam >> srcReq;

  if (srcReq == OPERATOR)
  {
    char name[H243_NAME_LEN];
    *pParam >> name >> (BYTE&)eOnOff >> (BYTE&)eMediaDirection;
    name[H243_NAME_LEN-1] = '\0';
    pPartyCntl            = (CVideoBridgePartyCntl*)GetPartyCntl(name);
  }
  else
  {
    PartyRsrcID partyRsrcID = INVALID;
    *pParam >> partyRsrcID >> (BYTE&)eOnOff >> (BYTE&)eMediaDirection;
    pPartyCntl = ( CVideoBridgePartyCntl*)GetPartyCntl(partyRsrcID);
  }

  if (eMediaDirection != eMediaIn)
  {
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfUpdateVideoMute Mute Video Out Not supported: Name - ", m_pConfName);
    DBGPASSERT_AND_RETURN(101);
  }

  if (NIL(CVideoBridgePartyCntl) == pPartyCntl)
  {
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfUpdateVideoMute Party not connected to bridge: Name - ", m_pConfName);
    return;
  }

  RequestPriority reqPrio = AUTO_Prior;
  reqPrio = GetRequestPriority(srcReq);

  const CImage* pImage    = pPartyCntl->GetPartyImage();
  DBGPASSERT_AND_RETURN(pImage == NIL(CImage));

  BYTE previouselyMuted   = pImage->isMuted();  // before updating we get previouse
  pPartyCntl->UpdateSelfMute(reqPrio, eOnOff);
  BYTE currentlyMuted     = pImage->isMuted();  // after updating we get current

  if (previouselyMuted == currentlyMuted)
    return;

  if (currentlyMuted)
    ApplicationActionsOnRemovePartyFromMix(pPartyCntl);
  else
    ApplicationActionsOnAddPartyToMix(pPartyCntl);

  // change conf COP layout
  // vngr-18616
  ChangeLayout(m_layoutType);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::RemovePartyFromAnyConfSettingsWhenDeletedFromConf(const char* pDeletedPartyName)
{
  for (int i = (int)CP_LAYOUT_1X1; i < (int)CP_NO_LAYOUT; i++)
    m_pReservation[i]->RemovePartyForce(pDeletedPartyName);

  ApplicationActionsOnDeletePartyFromConf(pDeletedPartyName);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfSetConfVideoLayoutSeeMeAllCONNECTED(CSegment* pParam)
{
  // set conf video layout requested by operator in Conf Level
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfSetConfVideoLayoutSeeMeAllCONNECTED : Name - ", m_pConfName);
  OnConfSetConfVideoLayoutSeeMeAll(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfSetConfVideoLayoutSeeMeAllINSWITCH(CSegment* pParam)
{
  // set conf video layout requested by operator in Conf Level
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfSetConfVideoLayoutSeeMeAllINSWITCH : Name - ", m_pConfName);
  OnConfSetConfVideoLayoutSeeMeAll(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfSetConfVideoLayoutSeeMeAll(CSegment* pParam)
{
  CVideoLayout layout;

  layout.DeSerialize(NATIVE, *pParam);

  LayoutType newLayoutType = GetNewLayoutType(layout.GetScreenLayout());
  if ((newLayoutType != m_layoutType) && GetIsAutoLayout())
  {
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfSetConfVideoLayoutSeeMeAll : can`t change layout manually is Auto Layout conf", m_pConfName);
    return;
  }

  if ((*m_pReservation[m_layoutType]) == layout)
  {
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfSetConfVideoLayoutSeeMeAll : received same layout, Name - ", m_pConfName);
    return;
  }

  CLayout* pBrdgResLayout = GetReservationLayout();

  // case when send layout just for setting reservations
  if (!layout.IsActive())
  {
    m_pReservation[newLayoutType]->SetLayout(layout, CONF_lev);
    UpdateDB_ConfLayout();
    return;
  }

  m_pReservation[newLayoutType]->SetLayout(layout, CONF_lev);

  ChangeConfLayoutType(newLayoutType);

  // change conf COP layout
  // vngr-18616
  ChangeLayout(m_layoutType);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::ChangeConfLayoutType(const LayoutType newLayoutType)
{
  if (newLayoutType == m_layoutType)
  {
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::ChangeConfLayoutType : same layout as before - Continue there might be forces change - Name - ", m_pConfName);
  }

  if (newLayoutType != CP_NO_LAYOUT)
    m_pReservation[newLayoutType]->SetCurrActiveLayout(YES);

  if (m_layoutType != CP_NO_LAYOUT && m_layoutType != newLayoutType)
    m_pReservation[m_layoutType]->SetCurrActiveLayout(NO);

  m_layoutType = newLayoutType;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfSpeakersChangedCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfVideoSpeakerChangedCONNECTED : Name - ", m_pConfName);
  OnConfSpeakersChanged(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfSpeakersChangedINSWITCH(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfVideoSpeakerChangedINSWITCH : Name - ", m_pConfName);
  OnConfSpeakersChanged(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfSpeakersChanged(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfSpeakersChanged - ConfName=", m_pConfName);

	CTaskApp* pNewVideoSpeaker = NULL;
	CTaskApp* pNewAudioSpeaker = NULL;
	BYTE rShouldResend = NO;

	*pParam >> (void*&)pNewVideoSpeaker >> (void*&)pNewAudioSpeaker >> rShouldResend;

	if (rShouldResend == YES)
	{
		m_pLastActiveVideoSpeakerRequest = NULL;
		m_pLastActiveAudioSpeakerRequest = NULL;
	}

	if ((m_pLastActiveAudioSpeakerRequest == pNewAudioSpeaker) && (m_pLastActiveVideoSpeakerRequest == pNewVideoSpeaker))
	{
		PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfSpeakersChanged - Neither speaker changed, ConfName=", m_pConfName);
		InformACLayoutChangeCompleted(TRUE);
		return;
	}

	// The order of check is important
	// 1.Last party in conference and in IVR
	if ((NULL == pNewAudioSpeaker) && (NULL == pNewVideoSpeaker))
	{
		PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfSpeakersChanged - No speaker in conference (Last party in IVR), ConfName=", m_pConfName);
		AudioSpeakerChanged(pNewAudioSpeaker);                    // Remove Notation and UpdateDB (remove EMA icon from last speaker)
		InformACLayoutChangeCompleted(TRUE);
	}

	else if ((m_pLastActiveVideoSpeakerRequest != pNewVideoSpeaker) || ((m_pLastActiveAudioSpeakerRequest != pNewAudioSpeaker) && (pNewVideoSpeaker == pNewAudioSpeaker)))
	{
		// SMART_SWITCH_LECTURER_1x1: Calculate next optimization state on basis of old/new speakers and the previous optimizations state
		// SetSmartLecturerState(m_pLastActiveVideoSpeakerRequest, pNewVideoSpeaker);
		m_pLastActiveVideoSpeakerRequest = pNewVideoSpeaker;

		// Audio Speaker also changed
		if (m_pLastActiveAudioSpeakerRequest != pNewAudioSpeaker)
		{
			m_pLastActiveAudioSpeakerRequest = pNewAudioSpeaker;
			m_pConfApi->UpdateDB(pNewAudioSpeaker, AUDIOSRC, 0);
			ApplicationActionsOnAudioSpeakerChange();
		}

		CVideoBridgePartyCntl* pNewSpkrPartyCntl = NULL;
		if (m_pLastActiveVideoSpeakerRequest)
		CVideoBridgePartyCntl* pNewSpkrPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(((CParty*)m_pLastActiveVideoSpeakerRequest)->GetPartyRsrcID());

		bool bPartyImageExistInImageVector = false;
		if (pNewSpkrPartyCntl)
			bPartyImageExistInImageVector = IsPartyImageExistInImageVector(pNewSpkrPartyCntl->GetPartyRsrcID());

		if (bPartyImageExistInImageVector)  // only when video decoder is fully connected
		{
			DWORD partyRscId = pNewSpkrPartyCntl->GetPartyRsrcID();

			CImage* pImage = GetPartyImageLookupTable()->GetPartyImage(partyRscId);
			PASSERTSTREAM_AND_RETURN(!pImage, "CVideoBridgeCOP::OnConfSpeakersChanged - Failed, The lookup table doesn't have an element, PartyId:" << partyRscId);

			TRACEINTO << "CVideoBridgeCOP::OnConfSpeakersChanged - New video speaker, PartyName:" << pNewSpkrPartyCntl->GetName();
			SetPartyImageSpeakerId(partyRscId);
			if (pImage->isMuted())
			{
				TRACEINTO << "CVideoBridgeCOP::OnConfSpeakersChanged - New video speaker is muted --> Do Nothing";
				InformACLayoutChangeCompleted(TRUE);
				return;
			}

			if (m_state != INSWITCH)                                // m_SwitchInSwitch???
			{
				m_isChangeLayoutBecauseOfSpeakerChange = TRUE;
			}
			else
			{
				m_isPendingChangeLayoutBecauseOfSpeakerChange = TRUE;
			}

			if (m_pCopLectureModeCntl->IsAudioActivatedLectureMode())
			{
				PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::OnConfSpeakersChanged : Audio Activated Lecture Mode, New lecturer: ", pNewSpkrPartyCntl->GetName());
				// change lecturer name in current lecture mode params
				CVideoBridgeLectureModeParams newLectureModeParams = *(m_pCopLectureModeCntl->GetCurrentParams());
				const char* new_lecturer_name = pNewSpkrPartyCntl->GetName();
				newLectureModeParams.SetLecturerName(new_lecturer_name);

				if (m_pCopLectureModeCntl->IsLectureModeActive())
				{
					CVideoBridgePartyCntl* oldLecturer = GetLecturer();
					LectureModeAction changeLecturerBaseAction = GetLectureModeChangeLecturerBaseAction(((CVideoBridgePartyCntlCOP*)oldLecturer), ((CVideoBridgePartyCntlCOP*)pNewSpkrPartyCntl));
					PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::OnConfSpeakersChanged - change lecturer base action type:", changeLecturerBaseAction);
					DoLectureModeBaseAction(changeLecturerBaseAction, newLectureModeParams);
				}
				else
				{
					// first auto lecturer - start lecture mode
					if (((CVideoBridgePartyCntlCOP*)pNewSpkrPartyCntl)->IsPartyCascadeLinkSupportAsymmetricEMCascade())
					{
						PTRACE2(eLevelInfoNormal, "LECTURE_MODE:ASYMMETRIC_CASCADE: CVideoBridgeCOP::CVideoBridgeCOP::OnConfSpeakersChanged - the new lecturer is Cascade Link that supports asymmetric cascade, ", m_pConfName);
						DoLectureModeBaseAction(eLmAction_LecturerConnect_CascadeLecturer, newLectureModeParams);
					}
					else
					{
						DoLectureModeBaseAction(eLmAction_LecturerConnect, newLectureModeParams);
					}
				}
			}
			else
			{
				// vngr-18616
				ChangeLayout(m_layoutType);
			}
		}
		else
		{
			// in case the participant is not connected + synced yet,we save last active speaker request
			// and resend it after the party connects in function AddPartyToConfMixAfterVideoInSynced
			PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfSpeakersChanged - New Video speaker is not connected, ConfName=", m_pConfName);
			InformACLayoutChangeCompleted(TRUE);
		}
	}

	// 3.Only Audio speaker changed
	else
	{
		if (m_pLastActiveAudioSpeakerRequest != pNewAudioSpeaker)
		{
			PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfSpeakersChanged::Video Speaker didn`t changed only new Audio Speaker - : Name - ", m_pConfName);
			AudioSpeakerChanged(pNewAudioSpeaker);
		}
		else
			PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::OnConfSpeakersChanged- Neither speaker changed !!!");
	}
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::AudioSpeakerChanged(CTaskApp* pNewAudioSpeaker)
{
  // Set the parameter m_pLastActiveAudioSpeakerRequest to indicate the Speaker Notation before Layout change
  m_pLastActiveAudioSpeakerRequest = pNewAudioSpeaker;

  if (NULL == pNewAudioSpeaker) // Last speaker in conf
    m_pConfApi->UpdateDB(pNewAudioSpeaker, NOAUDIOSRC, 0);
  else
    m_pConfApi->UpdateDB(pNewAudioSpeaker, AUDIOSRC, 0);

  SendLayoutAttributesToEncoders();

  ApplicationActionsOnAudioSpeakerChange();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::GetForcesFromReservation(CCommConf* pCommConf)
{
  PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr((CPObject*)pCommConf));

  WORD m_wNumVideoLayouts = pCommConf->GetNumRsrvVidLayout();
  if (m_wNumVideoLayouts > (WORD)CP_NO_LAYOUT) {
    DBGPASSERT_AND_RETURN(m_wNumVideoLayouts);
  }

  // get layouts and write it
  if (m_wNumVideoLayouts == 0)
    return;

  CVideoLayout* pCurrentLayout = pCommConf->GetFirstRsrvVidLayout();
  DBGPASSERT_AND_RETURN(pCurrentLayout == NIL(CVideoLayout));

  LayoutType newLayoutType     = GetNewLayoutType(pCurrentLayout->GetScreenLayout());
  if (newLayoutType >= CP_NO_LAYOUT) {
    DBGPASSERT_AND_RETURN(newLayoutType);
  }

  // sometimes first layout in reservation is not active one
  m_pReservation[newLayoutType]->SetLayoutFromRes(*pCurrentLayout, CONF_lev);
  if (pCurrentLayout->IsActive())
  {
    m_layoutType = newLayoutType;
    m_pReservation[m_layoutType]->SetCurrActiveLayout(YES);
  }

  for (WORD i = 1; i < m_wNumVideoLayouts; i++)
  {
    pCurrentLayout = pCommConf->GetNextRsrvVidLayout();
    if (pCurrentLayout == NIL(CVideoLayout))
    {
      PASSERT(1);
      break;
    }

    newLayoutType = GetNewLayoutType(pCurrentLayout->GetScreenLayout());
    if (newLayoutType >= CP_NO_LAYOUT)
    {
      DBGPASSERT(newLayoutType);
      break;
    }

    m_pReservation[newLayoutType]->SetLayoutFromRes(*pCurrentLayout, CONF_lev);
    if (pCurrentLayout->IsActive())
    {
      m_layoutType = newLayoutType;
      m_pReservation[m_layoutType]->SetCurrActiveLayout(YES);
    }
  }

  CVideoLayout* pVideoLayout = pCommConf->GetNextRsrvVidLayout();
  WORD testValue = (pVideoLayout != NIL(CVideoLayout)) ? pVideoLayout->GetScreenLayout() : 0;
  DBGPASSERT(testValue);
}

////////////////////////////////////////////////////////////////////////////
WORD CVideoBridgeCOP::GetCopEncoderIndexFromEntityId(DWORD levelEncoderEntityId) const
{
  WORD  resultEncoderIndex = (WORD)(-1);
  DWORD encoderPartyId     = (DWORD)(-1);
  for (WORD encoder_index = 0; encoder_index < NUM_OF_LEVEL_ENCODERS; encoder_index++)
  {
    encoderPartyId = m_pCopLevelEncoders[encoder_index]->GetPartyRsrcID();
    if (encoderPartyId == levelEncoderEntityId)
    {
      resultEncoderIndex = encoder_index;
      break;
    }
  }

  if (resultEncoderIndex == (WORD)(-1))   // not found level encoder - check if VSW encoder
  {
    encoderPartyId = m_pCopEncoderVsw->GetPartyRsrcID();
    if (encoderPartyId == levelEncoderEntityId)
    {
      resultEncoderIndex = VSW_ENCODER_INDEX;
    }
  }

  if (resultEncoderIndex == (WORD)(-1))   // not found
  {
    if (levelEncoderEntityId != 0)
      PASSERT(levelEncoderEntityId);
    else
      PASSERT(101);

    PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::GetCopEncoderIndexFromEntityId didn't find index for encoder party id:", levelEncoderEntityId);
  }

  return resultEncoderIndex;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnCopDecoderEndCloseCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnCopDecoderEndCloseCONNECTED, ConfName - ", m_pConfName);

  WORD            receivedStatus = statOK;
  EMediaDirection eMediaDirection;
  DWORD           decoderIndex;
  CTaskApp*       pParty         = NULL;
  *pParam >> (void*&)pParty;
  *pParam >> (WORD&)receivedStatus;
  *pParam >> (WORD&)eMediaDirection;
  *pParam >> (DWORD&)decoderIndex;

  if (receivedStatus != statOK)
  {
    m_resourcesStatus = receivedStatus;
    PASSERT(receivedStatus);
  }

  if (decoderIndex == LM_DECODER_INDEX || decoderIndex == VSW_DECODER_INDEX)
    if (IsLectureCodecsClosed())
      EndCloseLecturerCodecs();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnCopDecoderEndCloseINSWITCH(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnCopDecoderEndCloseINSWITCH, ConfName - ", m_pConfName);

  WORD            receivedStatus = statOK;
  EMediaDirection eMediaDirection;
  DWORD           decoderIndex;
  CTaskApp*       pParty         = NULL;
  *pParam >> (void*&)pParty;
  *pParam >> (WORD&)receivedStatus;
  *pParam >> (WORD&)eMediaDirection;
  *pParam >> (DWORD&)decoderIndex;

  if (receivedStatus != statOK)
  {
    m_resourcesStatus = receivedStatus;
    PASSERT(receivedStatus);
  }

  // lecture mode
  if (decoderIndex == LM_DECODER_INDEX || decoderIndex == VSW_DECODER_INDEX)
  {
    if (IsLectureCodecsClosed())
      EndCloseLecturerCodecs();
    return;
  }

  // if we are in Smart Switch we need to see if there still actions for the first stage
  if (m_IsSmartSwitch)
  {
    eChangeLayoutActionsStatus actionStatusForClose = m_changeLayoutActions->GetChangeLayoutActionStatusForDecoder(decoderIndex, eCopDecoderAction_Close);
    if (actionStatusForClose == eActionNeededInFirstPrior)
    {
      // a. Update the m_changeLayoutActions that the close decoder ack received
      m_changeLayoutActions->UpdateActionStatus(decoderIndex, eCopDecoderAction_Close, eActionComplete);
      // b. check if the first stage of smart switch actions of CLOSE,DISCONNECT AND CHNAGE MODE finished
      BOOL isStageFinished = AreAllCloseDisconnnectChangeModeDecActionsCompletedInSmartSwitch();
      if (isStageFinished)
      {
        m_IsSmartSwitch = FALSE;
        if (m_IsDSPSmartSwitch)
        {
          PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnCopDecoderEndCloseINSWITCH , StartDSPSmartSwitch ConfName - ", m_pConfName);
          StartDSPSmartSwitch();
        }
        else
        {
          StartSwitch();
        }
      }
    }
  }
  else // regular switch
  {
    // a. Update the m_changeLayoutActions that the close decoder ack received
    m_changeLayoutActions->UpdateActionStatus(decoderIndex, eCopDecoderAction_Close, eActionComplete);

    // b. check if the first stage actions of CLOSE,DISCONNECT AND CHNAGE MODE finished
    BOOL isStageFinished = AreAllCloseDisconnnectChangeModeDecActionsCompleted();

    // c. if it finished we can continue to the second stage
    if (isStageFinished)
    {
      StartSwitchOpenAndConnectDecodersActions();
    }
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnCopDecoderEndCloseDISCONNECTING(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnCopDecoderEndCloseDISCONNECTING, ConfName - ", m_pConfName);

  WORD            receivedStatus = statOK;
  EMediaDirection eMediaDirection;
  DWORD           decoderIndex;
  CTaskApp*       pParty         = NULL;
  *pParam >> (void*&)pParty;
  *pParam >> (WORD&)receivedStatus;
  *pParam >> (WORD&)eMediaDirection;
  *pParam >> (DWORD&)decoderIndex;

  if (receivedStatus != statOK)
  {
    m_resourcesStatus = receivedStatus;
    PASSERT(receivedStatus);
  }

  // VNGR-13391
  if (decoderIndex == VSW_DECODER_INDEX)
  {
    POBJDELETE(m_pCopDecoderVsw);
  }
  else if (decoderIndex == LM_DECODER_INDEX)
  {
    POBJDELETE(m_pCopDecoderLecturer);
  }
  else
    DestroyCopDecoder(decoderIndex);

  if (AreAllCopRsrcsDestroyed() == TRUE)
  {
    EndDisconnect();
  }
  else
  {
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnCopDecoderEndCloseDISCONNECTING not all COP resources are destroyed, ConfName - ", m_pConfName);
  }
}

////////////////////////////////////////////////////////////////////////////
BOOL CVideoBridgeCOP::AreAllCloseDisconnnectChangeModeDecActionsCompleted()
{
  BOOL res = TRUE;
  for (int i = 0; i < NUM_OF_COP_DECODERS; i++)
  {
    WORD                       decoderIndex              = i;
    eChangeLayoutActionsStatus actionStatusForClose      = m_changeLayoutActions->GetChangeLayoutActionStatusForDecoder(i, eCopDecoderAction_Close);
    eChangeLayoutActionsStatus actionStatusForDisconnect = m_changeLayoutActions->GetChangeLayoutActionStatusForDecoder(i, eCopDecoderAction_Disconnect);
    eChangeLayoutActionsStatus actionStatusForChangeMode = m_changeLayoutActions->GetChangeLayoutActionStatusForDecoder(i, eCopDecoderAction_ReCapToParty);
    if ((actionStatusForClose != eActionNotNeeded) && (actionStatusForClose != eActionComplete))
    {
      res = FALSE;
      PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::AreAllCloseDisconnnectChangeModeDecActionsCompleted not all acks on close decoder received", m_pConfName);
      return res;
    }

    if ((actionStatusForDisconnect != eActionNotNeeded) && (actionStatusForDisconnect != eActionComplete))
    {
      res = FALSE;
      PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::AreAllCloseDisconnnectChangeModeDecActionsCompleted not all acks on disconnect decoder received", m_pConfName);
      // tmp debug
      PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::AreAllCloseDisconnnectChangeModeDecActionsCompleted not all acks on disconnect decoder received actionStatusForDisconnect = ", actionStatusForDisconnect);
      PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::AreAllCloseDisconnnectChangeModeDecActionsCompleted not all acks on disconnect decoder received DECODER  = ", i);

      return res;
    }

    if ((actionStatusForChangeMode != eActionNotNeeded) && (actionStatusForChangeMode != eActionComplete))
    {
      res = FALSE;
      PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::AreAllCloseDisconnnectChangeModeDecActionsCompleted not all acks on change mode received", m_pConfName);
      // tmp debug
      PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::AreAllCloseDisconnnectChangeModeDecActionsCompleted not all acks on change mode received actionStatusForChangeMode = ", actionStatusForDisconnect);
      PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::AreAllCloseDisconnnectChangeModeDecActionsCompleted not all acks on change mode received DECODER  = ", i);

      return res;
    }
  }

  return res;
}

////////////////////////////////////////////////////////////////////////////
BOOL CVideoBridgeCOP::AreAllCloseDisconnnectChangeModeDecActionsCompletedInSmartSwitch()
{
  BOOL res = TRUE;
  for (int i = 0; i < NUM_OF_COP_DECODERS; i++)
  {
    WORD                       decoderIndex              = i;
    eChangeLayoutActionsStatus actionStatusForClose      = m_changeLayoutActions->GetChangeLayoutActionStatusForDecoder(i, eCopDecoderAction_Close);
    eChangeLayoutActionsStatus actionStatusForDisconnect = m_changeLayoutActions->GetChangeLayoutActionStatusForDecoder(i, eCopDecoderAction_Disconnect);
    eChangeLayoutActionsStatus actionStatusForChangeMode = m_changeLayoutActions->GetChangeLayoutActionStatusForDecoder(i, eCopDecoderAction_ReCapToParty);
    if (actionStatusForClose == eActionNeededInFirstPrior)
    {
      res = FALSE;
      PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::AreAllCloseDisconnnectChangeModeDecActionsCompletedInSmartSwitch not all acks on close decoder received ", m_pConfName);
      return res;
    }

    if (actionStatusForDisconnect == eActionNeededInFirstPrior)
    {
      res = FALSE;
      PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::AreAllCloseDisconnnectChangeModeDecActionsCompletedInSmartSwitch not all acks on disconnect decoder received ", m_pConfName);
      return res;
    }

    if (actionStatusForChangeMode == eActionNeededInFirstPrior)
    {
      res = FALSE;
      PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::AreAllCloseDisconnnectChangeModeDecActionsCompletedInSmartSwitch not all acks on change mode received ", m_pConfName);
      return res;
    }
  }

  if (m_IsSmartSwitchTimerPoped == FALSE)
  {
    res = FALSE;
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::AreAllCloseDisconnnectChangeModeDecActionsCompletedInSmartSwitch WAITING FOR SMART SWITCH TIMER ", m_pConfName);
  }

  return res;
}

////////////////////////////////////////////////////////////////////////////
BOOL CVideoBridgeCOP::AreAllOpenConnectSyncDecActionsCompleted()
{
  BOOL res = TRUE;
  for (int i = 0; i < NUM_OF_COP_DECODERS; i++)
  {
    WORD                       decoderIndex               = i;
    eChangeLayoutActionsStatus actionStatusForOpen        = m_changeLayoutActions->GetChangeLayoutActionStatusForDecoder(i, eCopDecoderAction_Open);
    eChangeLayoutActionsStatus actionStatusForConnect     = m_changeLayoutActions->GetChangeLayoutActionStatusForDecoder(i, eCopDecoderAction_Connect);
    eChangeLayoutActionsStatus actionStatusForWaitForSync = m_changeLayoutActions->GetChangeLayoutActionStatusForDecoder(i, eCopDecoderAction_WaitForSync);

    if ((actionStatusForOpen != eActionNotNeeded) && (actionStatusForOpen != eActionComplete))
    {
      res = FALSE;
      PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::AreAllOpenConnectSyncDecActionsCompleted not all acks on open decoder received", m_pConfName);
      return res;
    }

    if ((actionStatusForConnect != eActionNotNeeded) && (actionStatusForConnect != eActionComplete))
    {
      res = FALSE;
      PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::AreAllOpenConnectSyncDecActionsCompleted not all acks on connect decoder received", m_pConfName);
      return res;
    }

    if ((actionStatusForWaitForSync != eActionNotNeeded) && (actionStatusForWaitForSync != eActionComplete))
    {
      res = FALSE;
      PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::AreAllOpenConnectSyncDecActionsCompleted not all indications on decoder sync received", m_pConfName);
      return res;
    }
  }

  return res;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnCopDecoderEndDisconnectCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnCopDecoderEndDisconnectCONNECTED, ConfName - ", m_pConfName);
  WORD            receivedStatus = statOK;
  EMediaDirection eMediaDirection;
  DWORD           decoderIndex;
  CTaskApp*       pParty         = NULL;
  *pParam >> (void*&)pParty;
  *pParam >> (WORD&)receivedStatus;
  *pParam >> (WORD&)eMediaDirection;
  *pParam >> (DWORD&)decoderIndex;

  if (receivedStatus != statOK)
  {
    m_resourcesStatus = receivedStatus;
    PASSERT(receivedStatus);
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnCopDecoderEndDisconnectINSWITCH(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnCopDecoderEndDisconnectINSWITCH, ConfName - ", m_pConfName);

  WORD            receivedStatus = statOK;
  EMediaDirection eMediaDirection;
  DWORD           decoderIndex;
  CTaskApp*       pParty         = NULL;
  *pParam >> (void*&)pParty;
  *pParam >> (WORD&)receivedStatus;
  *pParam >> (WORD&)eMediaDirection;
  *pParam >> (DWORD&)decoderIndex;

  if (receivedStatus != statOK)
  {
    m_resourcesStatus = receivedStatus;
    PASSERT(receivedStatus);
  }

  // if we are in Smart Switch we need to see if there still actions for the first stage
  if (m_IsSmartSwitch)
  {
    eChangeLayoutActionsStatus actionStatusForDisconnect = m_changeLayoutActions->GetChangeLayoutActionStatusForDecoder(decoderIndex, eCopDecoderAction_Disconnect);
    if (actionStatusForDisconnect == eActionNeededInFirstPrior)
    {
      // a. Update the m_changeLayoutActions that the disconnect decoder ack received
      m_changeLayoutActions->UpdateActionStatus(decoderIndex, eCopDecoderAction_Disconnect, eActionComplete);
      // b. check if the first stage of smart switch actions of CLOSE,DISCONNECT AND CHNAGE MODE finished
      BOOL isStageFinished = AreAllCloseDisconnnectChangeModeDecActionsCompletedInSmartSwitch();
      if (isStageFinished)
      {
        m_IsSmartSwitch = FALSE;
        if (m_IsDSPSmartSwitch)
        {
          PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnCopDecoderEndDisconnectINSWITCH , StartDSPSmartSwitch ConfName - ", m_pConfName);
          StartDSPSmartSwitch();
        }
        else
        {
          StartSwitch();
        }
      }
    }
  }
  else
  {
    // a. Update the m_changeLayoutActions that the close decoder ack received
    m_changeLayoutActions->UpdateActionStatus(decoderIndex, eCopDecoderAction_Disconnect, eActionComplete);
    // b. check if the first stage actions of CLOSE,DISCONNECT AND CHNAGE MODE finished
    BOOL isStageFinished = AreAllCloseDisconnnectChangeModeDecActionsCompleted();

    // c. if it finished we can continue to the second stage
    if (isStageFinished)
    {
      StartSwitchOpenAndConnectDecodersActions();
    }
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnCopDecoderEndDisconnectDISCONNECTING(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnCopDecoderEndDisconnectDISCONNECTING, ConfName - ", m_pConfName);

  WORD            receivedStatus = statOK;
  EMediaDirection eMediaDirection;
  DWORD           decoderIndex;
  CTaskApp*       pParty         = NULL;
  *pParam >> (void*&)pParty;
  *pParam >> (WORD&)receivedStatus;
  *pParam >> (WORD&)eMediaDirection;
  *pParam >> (DWORD&)decoderIndex;

  if (receivedStatus != statOK)
  {
    m_resourcesStatus = receivedStatus;
    PASSERT(receivedStatus);
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnCopDecoderEndConnectCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnCopDecoderEndConnectCONNECTED, ConfName - ", m_pConfName);

  WORD            receivedStatus = statOK;
  EMediaDirection eMediaDirection;
  DWORD           decoderIndex;
  CTaskApp*       pParty         = NULL;
  *pParam >> (void*&)pParty;
  *pParam >> (WORD&)receivedStatus;
  *pParam >> (WORD&)eMediaDirection;
  *pParam >> (DWORD&)decoderIndex;
  if (receivedStatus != statOK)
  {
    // TODO handel error handleing flow
    PASSERT(receivedStatus);
  }

  if (decoderIndex == LM_DECODER_INDEX || decoderIndex == VSW_DECODER_INDEX)
  {
    if (IsLecturerCodecsConnected())
    {
      EndOpenLecturerCodecs();
    }
    else
    {
      PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::OnCopDecoderEndConnectCONNECTED, decoderIndex = ", decoderIndex);
    }
  }
  else
  {
    PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::OnCopDecoderEndConnectCONNECTED, decoderIndex = ", decoderIndex);
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnCopDecoderEndConnectINSWITCH(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnCopDecoderEndConnectINSWITCH, ConfName - ", m_pConfName);

  WORD            receivedStatus = statOK;
  EMediaDirection eMediaDirection;
  DWORD           decoderIndex;
  CTaskApp*       pParty         = NULL;
  *pParam >> (void*&)pParty;
  *pParam >> (WORD&)receivedStatus;
  *pParam >> (WORD&)eMediaDirection;
  *pParam >> (DWORD&)decoderIndex;

  if (receivedStatus != statOK)
  {
    m_resourcesStatus = receivedStatus;
    PASSERT(receivedStatus);
  }

  if (decoderIndex == LM_DECODER_INDEX || decoderIndex == VSW_DECODER_INDEX)
  {
    if (IsLecturerCodecsConnected())
    {
      EndOpenLecturerCodecs();
    }
    else
    {
      PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::OnCopDecoderEndConnectINSWITCH, decoderIndex = ", decoderIndex);
    }
  }
  else
  {
    // a. Update the m_changeLayoutActions that the decoder opened/connected ack received
    m_changeLayoutActions->UpdateActionStatus(decoderIndex, eCopDecoderAction_Open, eActionComplete);
    m_changeLayoutActions->UpdateActionStatus(decoderIndex, eCopDecoderAction_Connect, eActionComplete);

    // b. check if the first stage actions of CLOSE,DISCONNECT AND CHNAGE MODE finished
    BOOL isStageFinished = AreAllOpenConnectSyncDecActionsCompleted();

    // c. if it finished we can continue to the second stage
    if (isStageFinished)
    {
      EndChangeLayoutActions();
    }
  }
}
////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnCopDecoderEndConnectDISCONNECTING(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnCopDecoderEndConnectDISCONNECTING, ConfName - ", m_pConfName);

  WORD            receivedStatus = statOK;
  EMediaDirection eMediaDirection;
  DWORD           decoderIndex;
  CTaskApp*       pParty         = NULL;
  *pParam >> (void*&)pParty;
  *pParam >> (WORD&)receivedStatus;
  *pParam >> (WORD&)eMediaDirection;
  *pParam >> (DWORD&)decoderIndex;
  if (receivedStatus != statOK)
  {
    m_resourcesStatus = receivedStatus;
    PASSERT(receivedStatus);
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::UpdateVideoInParams(CTaskApp* pParty, CBridgePartyVideoParams* pBridgePartyVideoParams)
{
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pParty));
	CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl*)(GetPartyCntl(((CParty*)pParty)->GetPartyRsrcID()));
	if (!CPObject::IsValidPObjectPtr(pVideoPartyCntl))
	{
		PTRACE2(eLevelInfoNormal, "CVideoBridgeCP::UpdateVideoInParams : Party Not Connected to Video Bridge!!! ConfName - ", m_pConfName);
		m_pConfApi->SendResponseMsg(pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_IN_UPDATED, statIllegal, 1);
		return;
	}

	if (!(CPObject::IsValidPObjectPtr(pBridgePartyVideoParams)))
	{
		PTRACE2(eLevelInfoNormal, "CVideoBridgeCP::UpdateVideoInParams : Video Params Invalid Object!!! ConfName - ", m_pConfName);
		m_pConfApi->SendResponseMsg(pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_IN_UPDATED, statIllegal, 1);
		return;
	}

	if (!(pBridgePartyVideoParams->IsValidParams()))
	{
		PTRACE2(eLevelInfoNormal, "CVideoBridgeCP::UpdateVideoInParams : Video Params - Params Invalid!!! ConfName - ", m_pConfName);
		m_pConfApi->SendResponseMsg(pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_IN_UPDATED, statIllegal, 1);
		return;
	}

	pVideoPartyCntl->UpdateVideoInParams(pBridgePartyVideoParams);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::UpdateVideoOutParams(CTaskApp* pParty, CBridgePartyVideoParams* pBridgePartyVideoParams)
{
	PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::UpdateVideoOutParams, ConfName - ", m_pConfName);

	// this call is not through state machine, so I print the state for further debugging
	switch (m_state)
	{
		case DISCONNECTING:
		{
			PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::UpdateVideoOutParams received in DISCONNECTING state - ignored , ConfName - ", m_pConfName);
			return; // dont execute the function
		}

		case INSWITCH:
		{
			PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::UpdateVideoOutParams received in INSWITCH state, ConfName - ", m_pConfName);
			// just for information, not returning
			break;
		}

		case CONNECTED:
		default:
			break;
	} // switch

	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pParty));
	CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(((CParty*)pParty)->GetPartyRsrcID());

	if (!CPObject::IsValidPObjectPtr(pVideoPartyCntl))
	{
		PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::UpdateVideoOutParams : Party Not Connected to Video Bridge!!! ConfName - ", m_pConfName);
		m_pConfApi->SendResponseMsg(pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_OUT_UPDATED, statIllegal, 1);
		return;
	}

	if (!(CPObject::IsValidPObjectPtr(pBridgePartyVideoParams)))
	{
		PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::UpdateVideoOutParams : Video Params Invalid Object!!! ConfName - ", m_pConfName);
		m_pConfApi->SendResponseMsg(pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_OUT_UPDATED, statIllegal, 1);
		return;
	}

	if (!(pBridgePartyVideoParams->IsValidParams()))
	{
		PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::UpdateVideoOutParams : Video Params - Params Invalid!!! ConfName - ", m_pConfName);
		m_pConfApi->SendResponseMsg(pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_OUT_UPDATED, statIllegal, 1);
		return;
	}

	// update encode resource params from resource index
	WORD newCopEncoderIndex = ((CBridgePartyVideoOutParams*)pBridgePartyVideoParams)->GetCopResourceIndex();
	CopRsrcDesc encoderCopRsrcDesc;
	BYTE wasUpdateSent = NO;

	memset(&encoderCopRsrcDesc, 0xFF, sizeof(encoderCopRsrcDesc));

	PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::UpdateVideoOutParams:  party_name = ", pVideoPartyCntl->GetName());
	PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::UpdateVideoOutParams:  lecturerName = ", m_pLectureModeParams->GetLecturerName());

	PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::UpdateVideoOutParams:  newCopEncoderIndex = ", newCopEncoderIndex);
	PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::UpdateVideoOutParams:  m_lecturerlevelEncoderIndex = ", m_lecturerlevelEncoderIndex);

	if (IsLecturer(pVideoPartyCntl))
	{
		if (m_pCopLectureModeCntl->GetUpdateCascadeLinkAsLecturer())
		{
			PTRACE(eLevelInfoNormal, "LECTURE_MODE:ASYMMETRIC_CASCADE CVideoBridgeCOP::UpdateVideoOutParams:  UpdateCascadeLinkAsLecturerOnRecevingUpdateVideoOutParams");
			UpdateCascadeLinkAsLecturerOnRecevingUpdateVideoOutParams(((CBridgePartyVideoOutParams*)pBridgePartyVideoParams), ((CVideoBridgePartyCntlCOP*)pVideoPartyCntl));
			wasUpdateSent = YES;
		}
		else if (m_pCopLectureModeCntl->GetUpdateCascadeLinkAsNotLecturer())
		{
			if (IsPrevCascadeLinkLecturer(pVideoPartyCntl))
			{
				PTRACE(eLevelInfoNormal, "LECTURE_MODE:ASYMMETRIC_CASCADE CVideoBridgeCOP::UpdateVideoOutParams:  UpdateCascadeLinkAsNotLecturerOnRecevingUpdateVideoOutParams for lecturer (change from cascade lecturer)");
				UpdateCascadeLinkAsNotLecturerOnRecevingUpdateVideoOutParams(((CBridgePartyVideoOutParams*)pBridgePartyVideoParams), ((CVideoBridgePartyCntlCOP*)pVideoPartyCntl));
				wasUpdateSent = YES;
			}
		}
		else if (((CVideoBridgePartyCntlCOP*)pVideoPartyCntl)->IsPartyCascadeLinkSupportAsymmetricEMCascade())
		{
			PTRACE(eLevelInfoNormal, "LECTURE_MODE:ASYMMETRIC_CASCADE CVideoBridgeCOP::UpdateVideoOutParams:  UpdateCascadeLinkAsLecturerOnRecevingUpdateVideoOutParams");
			UpdateCascadeLinkAsLecturerOnRecevingUpdateVideoOutParams(((CBridgePartyVideoOutParams*)pBridgePartyVideoParams), ((CVideoBridgePartyCntlCOP*)pVideoPartyCntl));
			wasUpdateSent = YES;
		}
		else
		{
			if (newCopEncoderIndex != m_lecturerlevelEncoderIndex)
			{
				PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::UpdateVideoOutParams:  update lecturer to new encoder, ConfName - ", m_pConfName);
				PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::UpdateVideoOutParams:  newCopEncoderIndex = ", newCopEncoderIndex);
				PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::UpdateVideoOutParams:  m_lecturerlevelEncoderIndex = ", m_lecturerlevelEncoderIndex);

				CMedString mstr;
				mstr << "LectureModeParams:\n";
				m_pLectureModeParams->Dump(mstr);
				PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::UpdateVideoOutParams:\n", mstr.GetString());

				encoderCopRsrcDesc = GetCopRsrcDesc(eLevelEncoderType, newCopEncoderIndex);
				((CBridgePartyVideoOutParams*)pBridgePartyVideoParams)->SetCopConnectionId(encoderCopRsrcDesc.connectionId);
				((CBridgePartyVideoOutParams*)pBridgePartyVideoParams)->SetCopPartyId(encoderCopRsrcDesc.rsrcEntityId);
				pVideoPartyCntl->UpdateVideoOutParams(pBridgePartyVideoParams);
				wasUpdateSent = YES;
				DoLectureModeBaseAction(eLmAction_ChangeLecturer, *m_pLectureModeParams);
			}
			else
			{
				PTRACE2(eLevelInfoNormal, "CVideoBridgeCP::UpdateVideoOutParams : sent to lecturer with the same level - do nothing, ConfName - ", m_pConfName);
				encoderCopRsrcDesc = GetCopRsrcDesc(eLevelEncoderType, newCopEncoderIndex);
				((CBridgePartyVideoOutParams*)pBridgePartyVideoParams)->SetCopConnectionId(encoderCopRsrcDesc.connectionId);
				((CBridgePartyVideoOutParams*)pBridgePartyVideoParams)->SetCopPartyId(encoderCopRsrcDesc.rsrcEntityId);

				// ChangeLecturer();
			}
		}
	}
	else if (m_pCopLectureModeCntl->GetUpdateCascadeLinkAsNotLecturer())
	{
		if (IsPrevCascadeLinkLecturer(pVideoPartyCntl))
		{
			PTRACE(eLevelInfoNormal, "LECTURE_MODE:ASYMMETRIC_CASCADE CVideoBridgeCOP::UpdateVideoOutParams:  UpdateCascadeLinkAsNotLecturerOnRecevingUpdateVideoOutParams");
			UpdateCascadeLinkAsNotLecturerOnRecevingUpdateVideoOutParams(((CBridgePartyVideoOutParams*)pBridgePartyVideoParams), ((CVideoBridgePartyCntlCOP*)pVideoPartyCntl));
			wasUpdateSent = YES;
		}
	}
	else if (newCopEncoderIndex == m_lecturerlevelEncoderIndex && IsActiveLectureMode())
	{
		PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::UpdateVideoOutParams : party uses lecturer index - updating to vsw encoder, ConfName - ", m_pConfName);
		((CBridgePartyVideoOutParams*)pBridgePartyVideoParams)->SetCopResourceIndex(m_lecturerlevelEncoderIndex);
		newCopEncoderIndex = m_lecturerlevelEncoderIndex;
		encoderCopRsrcDesc = GetCopRsrcDesc(eVswEncoderType);
		((CBridgePartyVideoOutParams*)pBridgePartyVideoParams)->SetCopConnectionId(encoderCopRsrcDesc.connectionId);
		((CBridgePartyVideoOutParams*)pBridgePartyVideoParams)->SetCopPartyId(encoderCopRsrcDesc.rsrcEntityId);
	}
	else
	{
		encoderCopRsrcDesc = GetCopRsrcDesc(eLevelEncoderType, newCopEncoderIndex);
		((CBridgePartyVideoOutParams*)pBridgePartyVideoParams)->SetCopConnectionId(encoderCopRsrcDesc.connectionId);
		((CBridgePartyVideoOutParams*)pBridgePartyVideoParams)->SetCopPartyId(encoderCopRsrcDesc.rsrcEntityId);
	}

	TRACEINTO << "CVideoBridgeCOP::UpdateVideoOutParams: newCopEncoderIndex = " << newCopEncoderIndex << " , connectionId = " << encoderCopRsrcDesc.connectionId << " , rsrcEntityId = " << encoderCopRsrcDesc.rsrcEntityId;

	// In case the party has FlowControl we reset it's prev flow control request has in the disconnection flow
	if (((CVideoBridgePartyCntlCOP*)pVideoPartyCntl)->GetPartyFlowControlRate() != 0)
	{
		UpdateFlowControlRateWhenPartyDisconnectsFromEncoder((CVideoBridgePartyCntlCOP*)pVideoPartyCntl);
	}

	if (!wasUpdateSent)
	{
		pVideoPartyCntl->UpdateVideoOutParams(pBridgePartyVideoParams);
	}
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnEndPartyUpdateVideoInINSWITCH(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnEndPartyUpdateVideoInINSWITCH, ConfName - ", m_pConfName);

	CParty* pParty = NULL;
	WORD status = statIllegal;
	BOOL isPartyCntlMsg = TRUE;

	*pParam >> (void*&)pParty >> status;

	if (CPObject::IsValidPObjectPtr(pParty))
	{
		m_pConfApi->SendResponseMsg(pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_IN_UPDATED, status, isPartyCntlMsg);

		CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(pParty->GetPartyRsrcID());
		if (pVideoPartyCntl)
		{
			DWORD artId = pVideoPartyCntl->GetPartyRsrcID();
			// if we are in Smart Switch we need to see if there still actions for the first stage
			if (m_IsSmartSwitch)
			{
				WORD decoderIndex = m_changeLayoutActions->GetDecoderIndexForReCapParty(artId);
				if (decoderIndex < NUM_OF_COP_DECODERS)
				{
					eChangeLayoutActionsStatus actionStatusForChangeMode = m_changeLayoutActions->GetChangeLayoutActionStatusForDecoder(decoderIndex, eCopDecoderAction_ReCapToParty);
					if (actionStatusForChangeMode == eActionNeededInFirstPrior)
					{
						// a. Update the m_changeLayoutActions that the disconnect decoder ack received
						m_changeLayoutActions->UpdateReCapToPartyActionStatus(artId, eActionComplete);

						// b. check if the first stage of smart switch actions of CLOSE,DISCONNECT AND CHNAGE MODE finished
						BOOL isStageFinished = AreAllCloseDisconnnectChangeModeDecActionsCompletedInSmartSwitch();
						if (isStageFinished)
						{
							m_IsSmartSwitch = FALSE;
							if (m_IsDSPSmartSwitch)
							{
								PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnEndPartyUpdateVideoInINSWITCH , StartDSPSmartSwitch ConfName - ", m_pConfName);
								StartDSPSmartSwitch();
							}
							else
							{
								StartSwitch();
							}
						}
					}
				}
			}
			else
			{
				// a. Update the m_changeLayoutActions that the changemode finished ack received
				m_changeLayoutActions->UpdateReCapToPartyActionStatus(artId, eActionComplete);
				// b. check if the first stage actions of CLOSE,DISCONNECT AND CHNAGE MODE finished
				BOOL isStageFinished = AreAllCloseDisconnnectChangeModeDecActionsCompleted();

				// c. if it finished we can continue to the second stage
				if (isStageFinished)
				{
					StartSwitchOpenAndConnectDecodersActions();
				}
			}

			if (IsActiveLectureMode())
			{
				if (IsLecturer(pVideoPartyCntl))
				{
					if (m_pCopLectureModeCntl->GetChangeModeToLecturer())
					{
						EndChangeModeLecturerByLevelEncoder();
					}
				}
			}
		}
	}
	else
	{
		PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnEndPartyUpdateVideoInINSWITCH NOT VALID PARTY, ConfName - ", m_pConfName);
		PASSERT(101);
	}
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnEndPartyUpdateVideoInCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CVideoBridge::OnEndPartyUpdateVideoInCONNECTED : ConfName - ", m_pConfName);

	CTaskApp* pParty = NULL;
	WORD status = statIllegal;
	BOOL isPartyCntlMsg = TRUE;

	*pParam >> (void*&)pParty >> status;

	if (CPObject::IsValidPObjectPtr(pParty))
	{
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(((CParty*)pParty)->GetPartyRsrcID());
		if (pPartyCntl)
		{
			if (IsActiveLectureMode())
			{
				if (IsLecturer(pPartyCntl))
				{
					if (m_pCopLectureModeCntl->GetChangeModeToLecturer())
					{
						EndChangeModeLecturerByLevelEncoder();
					}
				}
			}
		}
		m_pConfApi->SendResponseMsg(pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_IN_UPDATED, status, isPartyCntlMsg);
	}
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnCopDecoderVideoRefreshCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnCopDecoderVideoRefreshCONNECTED, ConfName - ", m_pConfName);
  OnCopDecoderVideoRefresh(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnCopDecoderVideoRefresh(CSegment* pParam)
{
	WORD receivedStatus = statOK;
	EMediaDirection eMediaDirection;
	DWORD decoderIndex;
	CTaskApp* pParty = NULL;
	*pParam >> (void*&)pParty;
	*pParam >> (WORD&)receivedStatus;
	*pParam >> (WORD&)eMediaDirection;
	*pParam >> (DWORD&)decoderIndex;

	TRACEINTO << "ConfName:" << m_pConfName << ", DecoderIndex:" << decoderIndex;

	CVideoBridgeCopDecoder* pVideoCopDecoder = GetCopDecoder(decoderIndex);
	PASSERTSTREAM_AND_RETURN(!pVideoCopDecoder, "DecoderIndex:" << decoderIndex);

	PartyRsrcID artPartyId = pVideoCopDecoder->GetCopDecoderArtId();
	CVideoBridgePartyCntlCOP* pPartyCntl = (CVideoBridgePartyCntlCOP*)m_pPartyList->Find(artPartyId);
	if (pPartyCntl)
		pPartyCntl->VideoRefresh();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnCopDecoderVideoRefreshINSWITCH(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnCopDecoderVideoRefreshINSWITCH, ConfName - ", m_pConfName);
  OnCopDecoderVideoRefresh(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnCopDecoderVideoRefreshDISCONNECTING(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnCopDecoderVideoRefreshDISCONNECTING Ignored, ConfName - ", m_pConfName);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnVideoInSynced(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnVideoInSynced : Name - ", m_pConfName);
  CTaskApp* pParty = NULL;
  WORD      status = statIllegal;

  *pParam >> (void*&)pParty >> status;
  if (status != statOK)
  {
    PTRACE2(eLevelError, "CVideoBridge::OnVideoInSynced received bad status: Name - ", m_pConfName);
    return;
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::UpdateDB_ConfLayout()
{
  CSegment vidLayoutSeg;
  if (!m_pReservation[m_layoutType]->Serialize(CONF_lev, &vidLayoutSeg))
    m_pConfApi->UpdateDB((CTaskApp*) 0xffff, CPCONFLAYOUT, (DWORD) 0, 0, &vidLayoutSeg);
  else
    DBGPASSERT(1);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnTimerEndSwitchINSWITCH(CSegment* pParam)
{
  // The change layout actions didn't complete at time...

  // If the change layout actions didn't complete at time is only because the decoder didn't sync yet (there is a known limitation in the HDX8000 Eps
  // that every re-cap takes more than 3 seconds) do not popup ASSERT
  bool popupAssert = false;

  for (int decoderNumber = 0; decoderNumber < NUM_OF_COP_DECODERS; ++decoderNumber)
    for (int decoderAction = 0; decoderAction < eCopDecoderAction_Last; ++decoderAction)
      switch (m_changeLayoutActions->GetActionStatus(decoderNumber, (eCopDecoderActions)decoderAction))
      {
        case eActionNeeded:
        case eActionNeededInFirstPrior:
        case eActionInProgress:
        	if (decoderAction != eCopDecoderAction_WaitForSync)
        		popupAssert = true;

        	break;
		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
      }

  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnTimerEndSwitchINSWITCH , ConfName - ", m_pConfName);
  if (popupAssert)
    DBGPASSERT(101);

  // dump Change Layout action table.
  m_changeLayoutActions->Dump();

  // if there are actions to perform that we can handele we will perform then and set the timer to 3 more seconds and continue.
  if (AreThereUnfinishedChangeLayoutActionsWeNeedToHandle())
  {
    HandleUnfinishedChangeLayoutActions();
  }
  else
  {
    // continue to end switch
    EndChangeLayoutActions();
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfVideoRefresh(CSegment* pParam)
{
	PartyRsrcID PartyId = INVALID;
	*pParam >> PartyId;

	TRACEINTO << "PartyId:" << PartyId;

	CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(PartyId);
	if (!pPartyCntl)
	{
		TRACEINTO << "PartyId:" << PartyId << " - Failed, party is not connected to Video Bridge";
		return;
	}

	if (pPartyCntl->IsConnectedStandalone())
	{
		TRACEINTO << "PartyId:" << PartyId << " - Failed, party is in SLIDE STATE, ask MFA for intra";
		pPartyCntl->FastUpdate();
		return;
	}

	BYTE downgradeNoisyPartyFromLevel1 = FALSE;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("EVENT_MODE_HANDLE_NOISY_PARTICIPANT", downgradeNoisyPartyFromLevel1);

	DWORD levelEncoderEntityId = ((CVideoBridgePartyCntlCOP*)pPartyCntl)->GetCopEncoderEntityId();
	WORD  levelEncoderIndex    = GetCopEncoderIndexFromEntityId(levelEncoderEntityId);
	CTaskApp*   pParty         = pPartyCntl->GetPartyTaskApp();
	BYTE        isRemoteIsMGC  = FALSE;
	if (pParty->IsTypeOf("CParty") && ((CParty*)pParty)->IsRemoteIsMGC())
		isRemoteIsMGC = TRUE;

	if (levelEncoderIndex <= NUM_OF_LEVEL_ENCODERS-1)
	{
		// VNGR-18424
		// In case we are in the middle of lecture mode actions inorder to avoid intermediate  layout we will suppress the intra until the lecture mode actions finish
		if (m_pCopLectureModeCntl->IsInAction())
		{
			PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfVideoRefresh, In the middle of lecture mode actions we will suppress this request Name - ", m_pConfName);
			m_needToRequestIntraFromLevelEncoder[levelEncoderIndex] = TRUE;
			return;
		}

		BYTE waitingToLecturerDecoderSync = m_pCopLectureModeCntl->GetWaitingToLecturerDecoderSync();
		// VNGR-18901 in case we finished the lecture mode actions but we still didnt send change layout to the listeners encoders we will postpone the request
		if (IsActiveLectureMode() && m_lecturerlevelEncoderIndex != levelEncoderIndex && waitingToLecturerDecoderSync)
		{
			PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfVideoRefresh, at the end of lecture mode and we didnt send the updated changelayout yet, we will postpone the intra request Name - ", m_pConfName);
			m_needToRequestIntraFromLevelEncoder[levelEncoderIndex] = TRUE;
			return;
		}

		if ((m_state == INSWITCH) && m_isChangeLayoutBecauseOfLectureModeFlows)
		{
			PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfVideoRefresh, at the change layout stage that caused because of lecture mode flows we will postpone the intra request Name - ", m_pConfName);
			m_needToRequestIntraFromLevelEncoder[levelEncoderIndex] = TRUE;
			return;
		}

		if (!pPartyCntl->IsPartyIntraSuppressed() || IsIntraSuppressEnabled(SUPPRESS_TYPE_ALL) == false)
		{
			m_pCopLevelEncoders[levelEncoderIndex]->FastUpdate(isRemoteIsMGC);
		}
		else
		{
			// 1. if the party is the lecturer we wont suppress it's request cause he is the only one one the encoder
			if (IsActiveLectureMode() && m_lecturerlevelEncoderIndex == levelEncoderIndex)
			{
				PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfVideoRefresh : , Party is the lecturer and it's noisy  --> we don't suppress intra's of lecturer! ", pPartyCntl->GetName());
				m_pCopLevelEncoders[levelEncoderIndex]->FastUpdate();
			}
			else // not lecturer
			{
				// 2. if the party in in the level encoder 0 and downgradeNoisyPartyFromLevel1
				if (levelEncoderIndex == 0 && downgradeNoisyPartyFromLevel1)
				{
					PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfVideoRefresh : EVENT_MODE_HANDLE_NOISY_PARTICIPANT is ON, Party is noisy and connected to Level1 --> DOWNGRADE PARTY TO LEVEL 2 , ", pPartyCntl->GetName());
					m_pConfApi->CopVideoOutChangeMode(PartyId, 1);
				}
				else
				{
					PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfVideoRefresh : Party is intra suppressed we wont send intra to the encoder, Party =", pPartyCntl->GetName());
				}
			}
		}
	}
	// party is listener and connected to VSW encoder
	else if (levelEncoderIndex == VSW_ENCODER_INDEX)
	{
		PTRACE2(eLevelError, "CVideoBridgeCOP::OnConfVideoRefresh Ask fast update from lecturer, Name - ", m_pConfName);
		if (!pPartyCntl->IsPartyIntraSuppressed() || IsIntraSuppressEnabled(SUPPRESS_TYPE_ALL) == false)
		{
			FastUpdateFromLecturer(FALSE, isRemoteIsMGC);
		}
		else if (m_lecturerlevelEncoderIndex == 0 && downgradeNoisyPartyFromLevel1)
		{
			PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfVideoRefresh : Party is intra suppressed and connected to VSW encoder(with Level 1 params), system flag is on - downgrade to level 2, Party = ", pPartyCntl->GetName());
			m_pConfApi->CopVideoOutChangeMode(PartyId, 1);
		}
		else
		{
			PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfVideoRefresh : Party is intra suppressed we won't ask intra from lecturer, Party =", pPartyCntl->GetName());
		}
	}
	else
	{
		PTRACE2(eLevelError, "CVideoBridgeCOP::OnConfVideoRefresh invalid levelEncoderIndex - not sending fast update, Name - ", m_pConfName);
		PASSERT(levelEncoderIndex);
	}

	if (m_pcmConnected && m_pcmLevelIndex == levelEncoderIndex)
		m_pPCMEncoderCntl->FastUpdate();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnVideoPreviewRefreshCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnVideoPreviewRefreshCONNECTED, Name - ", m_pConfName);
  OnVideoPreviewRefresh(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnVideoPreviewRefreshINSWITCH(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnVideoPreviewRefreshINSWITCH, Name - ", m_pConfName);
  OnVideoPreviewRefresh(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnVideoPreviewRefresh(CSegment* pParam)
{
  // VNGR-18541 In case of video preview we don't take into acount the suppression mechanism
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnVideoPreviewRefresh, Name - ", m_pConfName);

  PartyRsrcID partyRsrcID           = INVALID;
  *pParam >> partyRsrcID;
  CVideoBridgePartyCntl* pPartyCntl = ( CVideoBridgePartyCntl*)GetPartyCntl(partyRsrcID);
  if (!pPartyCntl)
  {
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnVideoPreviewRefresh :the party is not connected to BRIDGE, Name - ", m_pConfName);
    return;
  }

  if (pPartyCntl->IsConnectedStandalone())
  {
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnVideoPreviewRefresh : Party is in slide state, ask MFA for intra. request from party ", pPartyCntl->GetName());
    pPartyCntl->FastUpdate();
    return;
  }

  DWORD levelEncoderEntityId = ((CVideoBridgePartyCntlCOP*)pPartyCntl)->GetCopEncoderEntityId();
  WORD  levelEncoderIndex    = GetCopEncoderIndexFromEntityId(levelEncoderEntityId);
  if (levelEncoderIndex <= NUM_OF_LEVEL_ENCODERS-1)
  {
    // VNGR-18424
    // In case we are in the middle of lecture mode actions inorder to avoid intermediate  layout we will suppress the intra until the lecture mode actions finish
    if (m_pCopLectureModeCntl->IsInAction())
    {
      PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnVideoPreviewRefresh, In the middle of lecture mode actions we will suppress this request Name - ", m_pConfName);
      m_needToRequestIntraFromLevelEncoder[levelEncoderIndex] = TRUE;
      return;
    }

    BYTE waitingToLecturerDecoderSync = m_pCopLectureModeCntl->GetWaitingToLecturerDecoderSync();
    // listener ib lecture mode - in case we finished the lecture mode actions but we still didn't send change layout to the listeners encoders we will postpone the request
    if (IsActiveLectureMode() && m_lecturerlevelEncoderIndex != levelEncoderIndex && waitingToLecturerDecoderSync)
    {
      PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnVideoPreviewRefresh, at the end of lecture mode and we didnt send the updated changelayout yet, we will postpone the intra request Name - ", m_pConfName);
      m_needToRequestIntraFromLevelEncoder[levelEncoderIndex] = TRUE;
      return;
    }

    m_pCopLevelEncoders[levelEncoderIndex]->FastUpdate();
  }
  // party is listener and connected to VSW encoder
  else if (levelEncoderIndex == VSW_ENCODER_INDEX)
  {
    PTRACE2(eLevelError, "CVideoBridgeCOP::OnVideoPreviewRefresh Ask fast update from lecturer, Name - ", m_pConfName);
    FastUpdateFromLecturer();
  }
  else
  {
    PTRACE2(eLevelError, "CVideoBridgeCOP::OnVideoPreviewRefresh invalid levelEncoderIndex - not sending fast update, Name - ", m_pConfName);
    PASSERT(levelEncoderIndex);
  }

  if (m_pcmConnected && m_pcmLevelIndex == levelEncoderIndex)
    m_pPCMEncoderCntl->FastUpdate();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfVideoRefreshCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfVideoRefreshCONNECTED, Name - ", m_pConfName);
  OnConfVideoRefresh(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfVideoRefreshINSWITCH(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfVideoRefreshINSWITCH, Name - ", m_pConfName);
  OnConfVideoRefresh(pParam);
}

////////////////////////////////////////////////////////////////////////////
BYTE CVideoBridgeCOP::IsLecturerInDB(const char* party_name) const
{
  CCommConf*          pCommConf             = (CCommConf*)m_pConf->GetCommConf();
  CLectureModeParams* pApiLectureModeParams = pCommConf->GetLectureMode();
  const char*         DBLecturerName        = pApiLectureModeParams->GetLecturerName();
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::IsLecturerInDB name in DB: ", DBLecturerName);

  if (!strncmp(DBLecturerName, party_name, H243_NAME_LEN))
    return TRUE;
  else
    return FALSE;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::SetLecturerNameInDB(const char* lecturer_name)
{
  if (!lecturer_name)
  {
    PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::SetLecturerNameInDB Invalid name!!!");
    return;
  }

  CSegment* pSeg = new CSegment;
  *pSeg << lecturer_name;

  m_pConfApi->UpdateDB((CTaskApp*) 0xffff, UPDATELECTURERNAME, (DWORD) 0, 0, pSeg);
}

////////////////////////////////////////////////////////////////////////////
BYTE CVideoBridgeCOP::IsActiveLectureMode() const
{
  BYTE isActiveLectureMode = NO;

  if (NULL != m_pLectureModeParams)
  {
    if (m_pCopLectureModeCntl->IsLectureModeActive())
    {
      isActiveLectureMode = YES;
    }
  }

  return isActiveLectureMode;
}

////////////////////////////////////////////////////////////////////////////
CVideoBridgeCopDecoder* CVideoBridgeCOP::GetCopDecoder(WORD decoder_index)
{
  CVideoBridgeCopDecoder* pVideoCopDecoder = NULL;
  // index 0 - 15: level decoders
  if (decoder_index < NUM_OF_COP_DECODERS)
  {
    pVideoCopDecoder = m_pCopDecoders[decoder_index];
  }
  else if (decoder_index == LM_DECODER_INDEX)
  {
    pVideoCopDecoder = m_pCopDecoderLecturer;
  }
  else if (decoder_index == VSW_DECODER_INDEX)
  {
    pVideoCopDecoder = m_pCopDecoderVsw;
  }
  else
  {
    PASSERT(decoder_index);
  }

  return pVideoCopDecoder;
}

////////////////////////////////////////////////////////////////////////////
BYTE CVideoBridgeCOP::IsOnlyLecturerConnected() const
{
	const char* lecturerName = m_pLectureModeParams->GetLecturerName();
	BYTE isOnlyLecturerConnected  = NO;
	DWORD num_of_connected_parties = 0;

	std::ostringstream msg;
	msg.precision(0);
	msg << "CVideoBridgeCOP::IsOnlyLecturerConnected:" << endl;
	msg << " ---------+-----------+----------+-------------------------------------------------+" << endl;
	msg << " lecturer | connected | party id | party name                                      |" << endl;
	msg << " ---------+-----------+----------+-------------------------------------------------+" << endl;

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
		{
			const char* partyName   = pPartyCntl->GetName();
			bool        IsLecturer  = !strcmp(lecturerName, partyName) ? true : false;
			bool        IsConnected = (bool)pPartyCntl->IsConnectedState();
			PartyRsrcID partyRscId  = pPartyCntl->GetPartyRsrcID();

			msg << " " << setw(8) << right << (WORD)IsLecturer << " |"
					<< " " << setw(9) << right << (WORD)IsConnected << " |"
					<< " " << setw(8) << right << partyRscId << " |"
					<< " " << setw(47) << left  << partyName << " " << endl;

			if (IsConnected)
			{
				num_of_connected_parties++;
				if (num_of_connected_parties == 1 && IsLecturer)
					isOnlyLecturerConnected = YES;

				if (num_of_connected_parties > 1)
					isOnlyLecturerConnected = NO;
			}
		}
	}

	msg << " ---------+-----------+----------+-------------------------------------------------+" << endl;
	msg << "IsOnlyLecturerConnected:" << (isOnlyLecturerConnected ? "true" : "false");
	PTRACE(eLevelInfoNormal, msg.str().c_str());
	return isOnlyLecturerConnected;
}

////////////////////////////////////////////////////////////////////////////
BYTE CVideoBridgeCOP::IsLecturerConnectedToPCM()
{
	const char* lecturerName   = m_pLectureModeParams->GetLecturerName();
	if (!lecturerName || !m_pcmConnected)
		return FALSE;

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntlCOP* pPartyCntl = (CVideoBridgePartyCntlCOP*)_itr->second;
		if (pPartyCntl)
		{
			if (pPartyCntl->IsConnectedOrConnectingPCM())
			{
				if (!strncmp(lecturerName, pPartyCntl->GetName(), H243_NAME_LEN))
					return TRUE;
				else
					return FALSE; // there is connected party to PCM but it is not the lecturer
			}
		}
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////
BYTE CVideoBridgeCOP::IsLecturerDecodersConnected() const
{
  BYTE isLecturerDecodersConnected = NO;
  if (m_pCopDecoderLecturer->IsConnectedState() && m_pCopDecoderVsw->IsConnectedState())
  {
    isLecturerDecodersConnected = YES;
  }

  PTRACE2COND((IsActiveLectureMode() && isLecturerDecodersConnected), eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::IsLecturerDecodersConnected = YES, ", m_pConfName);
  return isLecturerDecodersConnected;
}

////////////////////////////////////////////////////////////////////////////
BYTE CVideoBridgeCOP::IsLecturerCodecsConnected() const
{
  BYTE isLecturerDecodersConnected = NO;

  if (m_pCopDecoderLecturer->IsConnectedState() && m_pCopDecoderVsw->IsConnectedState() && m_pCopEncoderVsw->IsCopEncoderInConnectedState())
  {
    isLecturerDecodersConnected = YES;
  }

  PTRACE2INT(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::IsLecturerCodecsConnected = ", (DWORD)isLecturerDecodersConnected);
  return isLecturerDecodersConnected;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::FastUpdateFromLecturer(WORD ignore_intra_filtering, WORD isRequestFromRemoteMGC) const
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::FastUpdateFromLecturer, ConfName=", m_pConfName);

  CVideoBridgePartyCntl* pLecturer = GetLecturer();

  if (!pLecturer)
  {
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::FastUpdateFromLecturer - The lecturer is not connected to bridge, Name=", m_pConfName);
    return;
  }

  CSegment* pSeg = new CSegment;
  *pSeg << ignore_intra_filtering << isRequestFromRemoteMGC;

  pLecturer->FastUpdate();
  pLecturer->HandleEvent(pSeg, 0, VIDREFRESH);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnEndPartyUpdateVideoOutCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnEndPartyUpdateVideoOutCONNECTED, Name - ", m_pConfName);
  OnEndPartyUpdateVideoOut(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnEndPartyUpdateVideoOutINSWITCH(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnEndPartyUpdateVideoOutINSWITCH, Name - ", m_pConfName);
  OnEndPartyUpdateVideoOut(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnEndPartyUpdateVideoOutDISCONNECTING(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnEndPartyUpdateVideoOutDISCONNECTING --> ignored, Name - ", m_pConfName);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnEndPartyUpdateVideoOut(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnEndPartyUpdateVideoOut, Name - ", m_pConfName);

	CParty* pParty = NULL;
	WORD status = statIllegal;
	BOOL isPartyCntlMsg = TRUE;
	CSegment* pSeg = NULL;
	CAckParams* pAckParams = NULL;
	EMediaDirection eMediaDirection;
	BOOL IsExtParams;

	*pParam >> (void*&)pParty >> status >> (WORD&)eMediaDirection >> (BYTE&)IsExtParams;

	if (!CPObject::IsValidPObjectPtr(pParty))
	{
		PTRACE2(eLevelError, "CVideoBridgeCOP::OnEndPartyUpdateVideoOut received pParty==NULL, Name - ", m_pConfName);
		PASSERT(111);
		return;
	}

	CVideoBridgePartyCntlCOP* pPartyCntl = (CVideoBridgePartyCntlCOP*)GetPartyCntl(pParty->GetPartyRsrcID());
	BYTE isMovePartiesAction = m_pCopLectureModeCntl->GetMoveVswPartiesAction();
	BYTE isMoveToVswAction = m_pCopLectureModeCntl->GetMovePartiesToVswAction();

	if ((m_pCopLectureModeCntl->GetMoveVswPartiesAction()) && (IsUpdatedPartyToFromVSW(pPartyCntl, isMoveToVswAction)))
	{
		// PTRACE2(eLevelInfoNormal,"LECTURE_MODE: CVideoBridgeCOP::OnEndPartyUpdateVideoOut MoveVswPartiesAction active, Name - ",m_pConfName);
		if (IsUpdateEncodersEnd())
		{
			EndSwitchListenersEncoder();
		}
	}
	else
	{
		if (status == statOK && IsExtParams)
		{
			// If there is valid ack params we need to send with the response to party the Ack params.
			pAckParams = new CAckParams;
			pAckParams->DeSerialize(NATIVE, *pParam);

			// PTRACE(eLevelInfoNormal,"CVideoBridge::OnEndPartyUpdateVideoOutCONNECTED : Add Ack Params to msg ");
			pSeg = new CSegment;
			pAckParams->Serialize(NATIVE, *pSeg);
		}

		if (CPObject::IsValidPObjectPtr(pPartyCntl))
		{
			if (IsActiveLectureMode() && m_pCopLectureModeCntl->GetUpdateCascadeLinkAsLecturer())
			{
				if (IsLecturer(pPartyCntl))
				{
					PTRACE2(eLevelError, "LECTURE_MODE:ASYMMETRIC_CASCADE CVideoBridgeCOP::OnEndPartyUpdateVideoOut update the lecturer params lecturer Name - ", pPartyCntl->GetName());
					m_pCopCascadeLinkLecturerLinkOutParamsWereUpdated = YES;
					if (IsEndUpdateCascadeLinkAsLecturer())
						EndUpdateCascadeLinkAsLecturer();
				}
			}
			else if (m_pCopLectureModeCntl->GetUpdateCascadeLinkAsNotLecturer())
			{
				if (IsPrevCascadeLinkLecturer(pPartyCntl))
				{
					PTRACE(eLevelInfoNormal, "LECTURE_MODE:ASYMMETRIC_CASCADE CVideoBridgeCOP::OnEndPartyUpdateVideoOut:  UpdateCascadeLinkAsNotLecturerOnRecevingUpdateVideoOutParams");
					m_pCopCascadeLinkLecturerLinkOutParamsWereUpdated = YES;
					if (IsEndUpdateCascadeLinkAsLecturer())
						EndUpdateCascadeLinkAsNotLecturer();
				}
			}
		}
		else
		{
			PTRACE2(eLevelError, "CVideoBridgeCOP::OnEndPartyUpdateVideoOut received can't get CVideoBridgePartyCntl using pParty, Name - ", m_pConfName);
			PASSERT(111);
		}

		if (status == statOK)
		{
			if (CPObject::IsValidPObjectPtr(pPartyCntl))
			{
				SendFastUpdateToPartyLevelEncoder(pPartyCntl);
			}
		}

		m_pConfApi->SendResponseMsg(pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_OUT_UPDATED, status, isPartyCntlMsg, pSeg);

		POBJDELETE(pSeg);
		POBJDELETE(pAckParams);
	}
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::SendFastUpdateToPartyLevelEncoder(CVideoBridgePartyCntlCOP* pPartyCntl)
{
  if (!CPObject::IsValidPObjectPtr(pPartyCntl))
  {
    PTRACE2(eLevelError, "CVideoBridgeCOP::SendFastUpdateToPartyLevelEncoder, invalid pPartyCntl Name - ", m_pConfName);
    PASSERT(111);
    return;
  }
  else
  {
    PTRACE2(eLevelError, "CVideoBridgeCOP::SendFastUpdateToPartyLevelEncoder to encoder of party ", pPartyCntl->GetName());
  }

  DWORD levelEncoderEntityId = pPartyCntl->GetCopEncoderEntityId();
  WORD  levelEncoderIndex    = GetCopEncoderIndexFromEntityId(levelEncoderEntityId);

  if (levelEncoderIndex <= NUM_OF_LEVEL_ENCODERS-1)
  {
    m_pCopLevelEncoders[levelEncoderIndex]->FastUpdate();
    WORD dbLevelEncoderIndex = levelEncoderIndex+1;
    pPartyCntl->UpdateLevelEncoderInDB(dbLevelEncoderIndex);
  }
  else if (levelEncoderIndex == VSW_ENCODER_INDEX)
  {
    PTRACE2(eLevelError, "CVideoBridgeCOP::SendFastUpdateToPartyLevelEncoder Ask fast update from lecturer, Name - ", m_pConfName);
    FastUpdateFromLecturer();
    WORD dbLevelEncoderIndex = m_lecturerlevelEncoderIndex+1;
    pPartyCntl->UpdateLevelEncoderInDB(dbLevelEncoderIndex);
  }
  else
  {
    PTRACE2(eLevelError, "CVideoBridgeCOP::SendFastUpdateToPartyLevelEncoder invalid levelEncoderIndex - not sending fast update, Name - ", m_pConfName);
    PASSERT(levelEncoderIndex);
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfDisConnectConfIDLE(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridge::OnConfDisConnectConfCONNECTED Changing State to DISCONNECTING: Name - ", m_pConfName);

  // Change to disconnecting state - to prevent connecting new parties
  m_state = DISCONNECTING;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnTimerConnectCOPVideoBridgeIDLE(CSegment* pParam)
{
  PTRACE2(eLevelError, "CVideoBridgeCOP::OnTimerConnectCOPVideoBridgeIDLE, Name - ", m_pConfName);
  PASSERT(101);
  m_pConfApi->EndVidBrdgConnect(statVideoInOutResourceProblem);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnEndPartyConnectIVRModeINSWITCH(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;

  // this function is called twice when video connected with IVR
  CBridge::EndPartyConnect(pParam, PARTY_VIDEO_IVR_MODE_CONNECTED);
}

////////////////////////////////////////////////////////////////////////////
BOOL CVideoBridgeCOP::IsSmartSwitchNeeded(CLayout* pNewLayout, CLayout* pOldLayout, WORD& closeDisconnectDecoderIndex)
{
  TRACECOND_AND_RETURN_VALUE(!pNewLayout, "CVideoBridgeCOP::IsSmartSwitchNeeded - Failed, Invalid pNewLayout, ConfName:" << m_pConfName, FALSE);
  TRACECOND_AND_RETURN_VALUE(!pOldLayout, "CVideoBridgeCOP::IsSmartSwitchNeeded - Failed, Invalid pOldLayout, ConfName:" << m_pConfName, FALSE);

  BOOL       isSmartSwitchNeeded = FALSE;
  LayoutType oldLayoutType       = pOldLayout->GetLayoutType();
  LayoutType newLayoutType       = pNewLayout->GetLayoutType();
  closeDisconnectDecoderIndex    = NUM_OF_COP_DECODERS;

  if (oldLayoutType != newLayoutType)
    return isSmartSwitchNeeded;

  const WORD numCellsInLayout = pNewLayout->GetNumberOfSubImages(); // same layout type both have same num cells
  if (IsLayoutWithSpeakerPicture(newLayoutType)) // asymmetric layout
  {
    // /set an option to disable the MCMS smart switch in asymmetric layout
    BOOL bSmartSwitchForAsymSysFlagOn = YES;
    CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("IS_COP_SMART_SWITCH_FOR_ASYM_ON", bSmartSwitchForAsymSysFlagOn);

    if (!bSmartSwitchForAsymSysFlagOn)
    {
      TRACEINTO << "CVideoBridgeCOP::IsSmartSwitchNeeded - FALSE, The system flag is OFF, ConfName:" << m_pConfName;
      return isSmartSwitchNeeded;
    }

    DWORD partyRscIdInBigCellOfNewLayout = pNewLayout->GetSubImageNum(0)->GetImageId();
    DWORD partyRscIdInBigCellOfOldLayout = pOldLayout->GetSubImageNum(0)->GetImageId();
    if (partyRscIdInBigCellOfNewLayout && partyRscIdInBigCellOfOldLayout && partyRscIdInBigCellOfNewLayout != partyRscIdInBigCellOfOldLayout)
    {
      // find the decoder index of the party that is going to be in big cell in the old layout
      BYTE  isPartyUseSmartSwitchAccordingToVendor = IsRemoteNeedSmartSwitchAccordingToVendor(partyRscIdInBigCellOfNewLayout);
      if (!isPartyUseSmartSwitchAccordingToVendor)
      {
        PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::IsSmartSwitchNeeded FALSE the party isn't HDX8000 in asymmetric layout: ConfName - ", m_pConfName);
      }
      else
      {
        PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::IsSmartSwitchNeeded TRUE in asymmetric layout: ConfName - ", m_pConfName);
        isSmartSwitchNeeded = TRUE;
        for (int oldImageNumber = 0; oldImageNumber < numCellsInLayout; oldImageNumber++)
        {
          CImage* pImage = GetPartyImage(*pOldLayout, oldImageNumber);
          DWORD imageArtPartyId = (pImage) ? pImage->GetArtPartyId() : INVALID;
          if (partyRscIdInBigCellOfNewLayout == imageArtPartyId)
          {
            CCopLayoutResolutionTable copLayoutResolutionTable;
            ECopDecoderResolution     copDecoderResolution;
            WORD                      retValue               = (WORD)-1;
            ECopEncoderMaxResolution  encoder_max_resolution = GetCopEncoderMaxResolution(); // COP_encoder_max_resolution_HD1080p25;
            retValue = copLayoutResolutionTable.GetCellResolutionDef(encoder_max_resolution, oldLayoutType, oldImageNumber, copDecoderResolution, closeDisconnectDecoderIndex);
            if (retValue != (WORD)-1)
            {
              PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::IsSmartSwitchNeeded decoder index of prev place of new speaker: ", closeDisconnectDecoderIndex);
            }
          }
        }
      }
    }
  }
  else // symmetric layout
  {
    // check both are new and old layouts are full
    BOOL areBothLayoutsFull = TRUE;
    for (int imageNumber = 0; imageNumber < numCellsInLayout; imageNumber++)
    {
      DWORD partyRscIdInNewLayout = pNewLayout->GetSubImageNum(imageNumber)->GetImageId();
      DWORD partyRscIdInOldLayout = pOldLayout->GetSubImageNum(imageNumber)->GetImageId();
      if (!partyRscIdInNewLayout || !partyRscIdInOldLayout)
        areBothLayoutsFull = FALSE;
    }

    if (areBothLayoutsFull)
    {
      // there is only one different participant in one of the cells
      WORD numNewPartys = 0;
      for (int i = 0; i < numCellsInLayout; ++i)
      {
        DWORD partyRscIdInNewLayout = pNewLayout->GetSubImageNum(i)->GetImageId();
        if (partyRscIdInNewLayout != 0)
        {
          BOOL isPartyInOldLayout = FALSE;
          for (int j = 0; j < numCellsInLayout; ++j)
          {
            DWORD partyRscIdInOldLayout = pOldLayout->GetSubImageNum(j)->GetImageId();
            if (partyRscIdInNewLayout == partyRscIdInOldLayout)
            {
              isPartyInOldLayout = TRUE;
              break;
            }
          }
          if (!isPartyInOldLayout)
            numNewPartys++;
      	}
      }

      if (numNewPartys == 1)
      {
        PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::IsSmartSwitchNeeded TRUE in symmetric layout: ConfName - ", m_pConfName);
        isSmartSwitchNeeded = TRUE;
      }
    }
  }

  return isSmartSwitchNeeded;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::StartSmartSwitch(DWORD smartSwitchTout, WORD closeDisconnectDecoderIndex)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::StartSmartSwitch : ConfName - ", m_pConfName);

  BOOL areThereActionsToPerform = FALSE;
  WORD reCapDecoderIndex        = NUM_OF_COP_DECODERS;

  if (IsLayoutWithSpeakerPicture(m_ConfLayout->GetLayoutType())) // asymmetric layout
  {
    // we are in smart switch because we are in asymmetric layout and only the party in the big cell changed.
    DWORD partyRscId = m_ConfLayout->GetSubImageNum(0)->GetImageId();
    if (partyRscId != 0)
    {
      CImage* pImage = GetPartyImageLookupTable()->GetPartyImage(partyRscId);
      PASSERTSTREAM(!pImage, "CVideoBridgeCOP::StartSmartSwitch - Failed, The lookup table doesn't have an element, PartyId:" << partyRscId);

      if (pImage)
      {
        DWORD copDecoderEntityId = pImage->GetCopDecoderEntityID();
        for (WORD i = 0; i < NUM_OF_COP_DECODERS; i++)
        {
          CVideoBridgeCopDecoder* pCopDecoder = m_pCopDecoders[i];
          if (CPObject::IsValidPObjectPtr((CPObject*)pCopDecoder))
            if (pCopDecoder->GetPartyRsrcID() == copDecoderEntityId)
              reCapDecoderIndex = i;
        }
      }
    }
  }
  else
  {
    // in case of symmetric layout there is only change in one party and we it wasn't connected to any decoder before
    // we will set just the change mode section as first prior
    for (WORD i = 0; i < NUM_OF_COP_DECODERS; i++)
    {
      eChangeLayoutActionsStatus actionStatusForChangeMode = m_changeLayoutActions->GetChangeLayoutActionStatusForDecoder(i, eCopDecoderAction_ReCapToParty);
      if (actionStatusForChangeMode == eActionNeeded)
        reCapDecoderIndex = i;
    }
  }

  // We will update the actions of close/disconnect/recap of the decoder
  // with the speaker state to be eActionNeededInFirstPrior
  if (reCapDecoderIndex < NUM_OF_COP_DECODERS)
  {
    PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::StartSmartSwitch  reCapDecoderIndex  ", reCapDecoderIndex);

    eChangeLayoutActionsStatus actionStatusForChangeMode = m_changeLayoutActions->GetChangeLayoutActionStatusForDecoder(reCapDecoderIndex, eCopDecoderAction_ReCapToParty);
    DWORD                      artPartyId                = DUMMY_PARTY_ID;
    ECopDecoderResolution      copDecRes                 = COP_decoder_resolution_Last;
    if (actionStatusForChangeMode == eActionNeeded)
    {
      areThereActionsToPerform = TRUE;
      artPartyId               = m_changeLayoutActions->GetChangeLayoutActionArtIdForDecoder(reCapDecoderIndex, eCopDecoderAction_ReCapToParty);
      copDecRes                = m_changeLayoutActions->GetChangeLayoutActionDecoderResForDecoder(reCapDecoderIndex, eCopDecoderAction_ReCapToParty);
      m_changeLayoutActions->UpdateActionStatus(reCapDecoderIndex, eCopDecoderAction_ReCapToParty, eActionNeededInFirstPrior);
      SendChangeModeToParty(reCapDecoderIndex, artPartyId, copDecRes);
    }
  }

  if (closeDisconnectDecoderIndex < NUM_OF_COP_DECODERS)
  {
    PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::StartSmartSwitch  closeDisconnectDecoderIndex  ", closeDisconnectDecoderIndex);
    eChangeLayoutActionsStatus actionStatusForClose      = m_changeLayoutActions->GetChangeLayoutActionStatusForDecoder(closeDisconnectDecoderIndex, eCopDecoderAction_Close);
    eChangeLayoutActionsStatus actionStatusForDisconnect = m_changeLayoutActions->GetChangeLayoutActionStatusForDecoder(closeDisconnectDecoderIndex, eCopDecoderAction_Disconnect);
    if (actionStatusForClose == eActionNeeded)
    {
      areThereActionsToPerform = TRUE;
      m_changeLayoutActions->UpdateActionStatus(closeDisconnectDecoderIndex, eCopDecoderAction_Close, eActionNeededInFirstPrior);
      SendClosePortToDecoder(closeDisconnectDecoderIndex);
    }
    else if (actionStatusForDisconnect == eActionNeeded)
    {
      areThereActionsToPerform = TRUE;
      m_changeLayoutActions->UpdateActionStatus(closeDisconnectDecoderIndex, eCopDecoderAction_Disconnect, eActionNeededInFirstPrior);
      SendDisconnectPortToDecoder(closeDisconnectDecoderIndex);
    }
  }

  if (!areThereActionsToPerform)
  {
    // In case of layout 1x1 and the IS_COP_SMART_SWITCH_WAIT_DECODER_SYNC is on we will wait for the decoder to sync
    // even if we didn't asked party to change mode
    const LayoutType layoutType = m_ConfLayout->GetLayoutType();
    if (IsWaitingToDecoderSyncInSmartSwitch() && layoutType == CP_LAYOUT_1X1)
    {
      WORD  layout1x1DecoderIndex = NUM_OF_COP_DECODERS;
      DWORD partyRscId = m_ConfLayout->GetSubImageNum(0)->GetImageId();
      if (partyRscId != 0)
      {
        CImage* pImage = GetPartyImageLookupTable()->GetPartyImage(partyRscId);
        PASSERTSTREAM(!pImage, "CVideoBridgeCOP::StartSmartSwitch - Failed, The lookup table doesn't have an element, PartyId:" << partyRscId);

        if (pImage)
        {
          DWORD copDecoderEntityId = pImage->GetCopDecoderEntityID();
          for (WORD i = 0; i < NUM_OF_COP_DECODERS; i++)
          {
            CVideoBridgeCopDecoder* pCopDecoder = m_pCopDecoders[i];
            if (CPObject::IsValidPObjectPtr((CPObject*)pCopDecoder))
              if (pCopDecoder->GetPartyRsrcID() == copDecoderEntityId)
                layout1x1DecoderIndex = i;
          }
        }
      }

      if (layout1x1DecoderIndex < NUM_OF_COP_DECODERS)
        m_changeLayoutActions->UpdateActionStatus(layout1x1DecoderIndex, eCopDecoderAction_WaitForSync);
    }

    m_IsSmartSwitch = FALSE;

    if (m_IsDSPSmartSwitch)
    {
      PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::StartSmartSwitch , StartDSPSmartSwitch ConfName - ", m_pConfName);
      StartDSPSmartSwitch();
    }
    else
      StartSwitch();
  }
  else
  {
    if (smartSwitchTout != 0)
    {
      StartTimer(SMART_SWITCH_TOUT, smartSwitchTout);
      m_IsSmartSwitchTimerPoped = FALSE;
      PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::StartSmartSwitch smart switch timer is = ", smartSwitchTout);
    }
    else
    {
      m_IsSmartSwitchTimerPoped = TRUE;
      PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::StartSmartSwitch smart switch timer is 0 no need to wait for it : ConfName - ", m_pConfName);
    }

    // In case we are waiting to the new party in big cell/new party in assymmetric to sync
    if (IsWaitingToDecoderSyncInSmartSwitch() && reCapDecoderIndex < NUM_OF_COP_DECODERS)
    {
      m_changeLayoutActions->UpdateActionStatus(reCapDecoderIndex, eCopDecoderAction_WaitForSync);
    }
  }
}

////////////////////////////////////////////////////////////////////////////
DWORD CVideoBridgeCOP::GetSmartSwitchTout()
{
  CSysConfig* pSysConfig      = CProcessBase::GetProcess()->GetSysConfig();
  DWORD       smartSwitchTout = 0;
  if (pSysConfig)
    pSysConfig->GetDWORDDataByKey("COP_SMART_SWITCH_TOUT", smartSwitchTout);

  if (smartSwitchTout >= 320) // tmp
    smartSwitchTout = 320;

  return smartSwitchTout;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnTimerEndSmartSwitchINSWITCH(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnTimerEndSmartSwitchINSWITCH : ConfName - ", m_pConfName);
  m_IsSmartSwitchTimerPoped = TRUE;
  if (AreAllCloseDisconnnectChangeModeDecActionsCompletedInSmartSwitch())
  {
    m_IsSmartSwitch = FALSE;
    if (m_IsDSPSmartSwitch)
    {
      PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnTimerEndSmartSwitchINSWITCH, StartDSPSmartSwitch ConfName - ", m_pConfName);
      StartDSPSmartSwitch();
    }
    else
    {
      StartSwitch();
    }
  }
}

////////////////////////////////////////////////////////////////////////////
BOOL CVideoBridgeCOP::IsWaitingToDecoderSyncInSmartSwitch()
{
  CSysConfig* pSysConfig                           = CProcessBase::GetProcess()->GetSysConfig();
  BOOL        bIsWaitingToDecoderSyncInSmartSwitch = NO;
  std::string key                                  = "IS_COP_SMART_SWITCH_WAIT_DECODER_SYNC";
  if (pSysConfig)
    pSysConfig->GetBOOLDataByKey(key, bIsWaitingToDecoderSyncInSmartSwitch);

  if (m_IsDSPSmartSwitch)
  {
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::IsWaitingToDecoderSyncInSmartSwitch in case of MCMS+DSP smart switch we wont wait to the decoder to sync, ConfName - ", m_pConfName);

    bIsWaitingToDecoderSyncInSmartSwitch = NO;
  }

  return bIsWaitingToDecoderSyncInSmartSwitch;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnCopDecoderVideoInSyncCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnCopDecoderVideoInSyncCONNECTED, ConfName - ", m_pConfName);

  WORD            receivedStatus = statOK;
  EMediaDirection eMediaDirection;
  DWORD           decoderIndex;
  CTaskApp*       pParty         = NULL;
  *pParam >> (void*&)pParty;
  *pParam >> (WORD&)receivedStatus;
  *pParam >> (WORD&)eMediaDirection;
  *pParam >> (DWORD&)decoderIndex;
  if (receivedStatus != statOK)
  {
    m_resourcesStatus = receivedStatus;
    PASSERT(receivedStatus);
    return;
  }

  PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::OnCopDecoderVideoInSyncCONNECTED decoder index: ", decoderIndex);
  if (decoderIndex == LM_DECODER_INDEX)
  {
    BYTE waitingToLecturerDecoderSync = m_pCopLectureModeCntl->GetWaitingToLecturerDecoderSync();
    // send ChangeLayout to level encoders that waiting for lecturer decoder sync
    if (waitingToLecturerDecoderSync)
    {
      if (m_pCopDecoderLecturer)
      {
        SendChangeLayoutOnLecturerDecoderSync();
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnCopDecoderVideoInSyncINSWITCH(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnCopDecoderVideoInSyncINSWITCH, ConfName - ", m_pConfName);

  WORD            receivedStatus = statOK;
  EMediaDirection eMediaDirection;
  DWORD           decoderIndex;
  CTaskApp*       pParty         = NULL;
  *pParam >> (void*&)pParty;
  *pParam >> (WORD&)receivedStatus;
  *pParam >> (WORD&)eMediaDirection;
  *pParam >> (DWORD&)decoderIndex;
  if (receivedStatus != statOK)
  {
    m_resourcesStatus = receivedStatus;
    PASSERT(receivedStatus);
  }

  PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::OnCopDecoderVideoInSyncINSWITCH decoder index: ", decoderIndex);
  if (decoderIndex < NUM_OF_COP_DECODERS)
  {
    // a. Update the m_changeLayoutActions that the close decoder ack received
    m_changeLayoutActions->UpdateActionStatus(decoderIndex, eCopDecoderAction_WaitForSync, eActionComplete);
  }

  // b. check if the first stage actions of OPEN CONNECT and SYNC finished
  BOOL isOpenStageFinished  = AreAllOpenConnectSyncDecActionsCompleted();
  // Bug VNGR-18504 fix we received that a decoder was synced and we didnt verify that there are still close actions
  BOOL isCloseStageFinished = AreAllCloseDisconnnectChangeModeDecActionsCompleted();

  // c. if it finished we can continue to the second stage
  if (isOpenStageFinished && isCloseStageFinished)
  {
    EndChangeLayoutActions();
  }
  else
  {
    if (isOpenStageFinished)
      PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::OnCopDecoderVideoInSyncINSWITCH there are still open connect actions");
    else
      PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::OnCopDecoderVideoInSyncINSWITCH there are still close disconnect actions ");
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnCopDecoderVideoVideoInSyncDISCONNECTING(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnCopDecoderVideoVideoInSyncDISCONNECTING Ignored, ConfName - ", m_pConfName);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::UpdatePCMEncoderWithNewLevelParamsIfNeeded(CVideoBridgePartyCntlCOP* pPartyCntl)
{
  if (!CPObject::IsValidPObjectPtr(pPartyCntl))
  {
    PASSERT(111);
    return;
  }

  DWORD levelEncoderEntityId = pPartyCntl->GetCopEncoderEntityId();
  WORD  levelEncoderIndex    = GetCopEncoderIndexFromEntityId(levelEncoderEntityId);
  if (levelEncoderIndex == VSW_ENCODER_INDEX)
    levelEncoderIndex = m_lecturerlevelEncoderIndex;

  if (levelEncoderIndex != m_pcmLevelIndex)
  {
    m_pcmLevelIndex = levelEncoderIndex;
    CBridgePartyVideoOutParams* pOutVideoParams = new CBridgePartyVideoOutParams();
    WORD update_status = UpdateEncoderVideoParams(pOutVideoParams, ((CCommConf*)m_pConf->GetCommConf()), m_pcmLevelIndex);

    if (update_status == FALSE)
      PASSERT(111);

    // Pcm encoder is always 5fps
    pOutVideoParams->SetVidFrameRate(eVideoFrameRate5FPS);

    m_pPCMEncoderCntl->UpdateVideoOutParams(pOutVideoParams);

    DWORD     pcmNewResolution = GetPartyResolutionInPCMTerms(pPartyCntl->GetPartyRsrcID());
    CSegment* pSeg             = new CSegment;
    *pSeg << pcmNewResolution;
    pPartyCntl->PCMNotification(PCM_RESOLUTION_CHANGED, pSeg);
    POBJDELETE(pSeg)
    POBJDELETE(pOutVideoParams);
  }
  else
  {
    PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::UpdatePCMEncoderWithNewLevelParamsIfNeeded - update is not needed!");
  }
}

////////////////////////////////////////////////////////////////////////////
CLayout* CVideoBridgeCOP::GetPartyLayout(const char* partyName)
{
  if (!partyName || !IsActiveLectureMode())
    return m_ConfLayout;

  if (!strncmp(m_pLectureModeParams->GetLecturerName(), partyName, H243_NAME_LEN))
    return m_ConfLayout;
  else
    return m_ConfLayout1x1;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::ChangeSpeakerNotation(PartyRsrcID partyId, DWORD imageId)
{
  if (imageId > 15)
    m_pPCMEncoderCntl->ChangeLayoutAttributes(m_pVisualEffects, GetAudioSpeakerPlaceInLayout());
  else
  {
    // tmp for finding the best color
    m_pPcmFeccVisualEffects->SetSpeakerNotationColorYUV(GetPcmYUVColor());
    m_pPCMEncoderCntl->ChangeLayoutAttributes(m_pPcmFeccVisualEffects, imageId);
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::AddDecoderIndexToCalcStr(WORD decoder_index, CObjString& cstr)
{
  if (decoder_index == (WORD)-1)
    // cstr << "12345678901|";
    cstr << "     NA    |";
  else if (decoder_index < 10)
  {
    // cstr << "12345" << 6          << "78901|";
    cstr << "     " << decoder_index << "     |";
  }
  else
  {
    // cstr << "12345" << 67       << "8901|";
    cstr << "     " << decoder_index << "    |";
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::AddPartyIdToCalcStr(DWORD party_id, CObjString& cstr)
{
  // " PartyId |"
  // "123456789|"
  if (party_id == (DWORD)-1)
  {
    cstr << "    NA   |";
  }
  else if (party_id < 10)
  {
    // "123456789|"
    // cstr << "1234" << 5     << "6789|";
    cstr << "    " << party_id << "    |";
  }
  else if (party_id < 100)
  {
    // cstr << "1234" << 56      << "789|";
    cstr << "    " << party_id << "   |";
  }
  else if (party_id < 1000)
  {
    // cstr << "123" << 456     << "789|";
    cstr << "   " << party_id << "   |";
  }
  else if (party_id < 10000)
  {
    cstr << "   " << party_id << "  |";
  }
  else if (party_id < 100000)
  {
    cstr << "  "  << party_id << "  |";
  }
  else if (party_id < 1000000)
  {
    cstr << "  "  << party_id << " |";
  }
  else if (party_id < 10000000)
  {
    cstr << " "   << party_id << " |";
  }
  else if (party_id < 100000000)
  {
    cstr << " "   << party_id << "|";
  }
  else
  {
    cstr << party_id << "|";
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::AddVideoAlgToCalcStr(DWORD algo, CObjString& cstr)
{
  // " algorithm |"
  // "   H264    |"
  switch (algo)
  {
    case (H264):
    {
      cstr << "   H264    |";
      break;
    }

    case (H263):
    {
      cstr << "   H263    |";
      break;
    }

    case (H261):
    {
      cstr << "   H261    |";
      break;
    }

    default:
    {
      cstr << " unknown   |";
      break;
    }
  } // switch
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::AddResolutionToCalcStr(ECopDecoderResolution res, CObjString& cstr)
{
  if (res == COP_decoder_resolution_HD108030)
  {
    cstr << "   HD108030  |";
  }
  else if (res == COP_decoder_resolution_HD720p50)
  {
    cstr << "   HD720p50  |";
  }
  else if (res == COP_decoder_resolution_HD720p25)
  {
    cstr << "   HD720p25  |";
  }
  else if (res == COP_decoder_resolution_W4CIF25)
  {
    cstr << "   W4CIF25   |";
  }
  else if (res == COP_decoder_resolution_4CIF50)
  {
    cstr << "    4CIF50   |";
  }
  else if (res == COP_decoder_resolution_4CIF25)
  {
    cstr << "    4CIF25   |";
  }
  else if (res == COP_decoder_resolution_CIF25)
  {
    cstr << "    CIF25    |";
  }
  else
  {
    cstr << "   INVALID!  |";
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnVideoBridgeUpdateMessageOverlayCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnVideoBridgeUpdateMessageOverlayCONNECTED : ConfName - ", m_pConfName);

  UpdateMessageOverlay(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnVideoBridgeUpdateMessageOverlayINSWITCH(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnVideoBridgeUpdateMessageOverlayINSWITCH : ConfName - ", m_pConfName);

  UpdateMessageOverlay(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::UpdateMessageOverlay(CSegment* pParam)
{
  //CMessageOverlayInfo* pMessageOverlayInfo = new CMessageOverlayInfo;
  m_pMessageOverlayInfo->DeSerialize(NATIVE, *pParam);

  for (WORD encoder_index = 0; encoder_index < NUM_OF_LEVEL_ENCODERS; encoder_index++)
  {
    if (!CPObject::IsValidPObjectPtr(m_pCopLevelEncoders[encoder_index]))
    {
      PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::UpdateMessageOverlay invalid COP encoder - m_pCopLevelEncoders index = ", encoder_index);
      continue;
    }

    ((CVideoBridgePartyCntl*)m_pCopLevelEncoders[encoder_index])->UpdateMessageOverlay(m_pMessageOverlayInfo);
  }

  //POBJDELETE(pMessageOverlayInfo);
}

////////////////////////////////////////////////////////////////////////////
BOOL CVideoBridgeCOP::IsSwitchWithDisconnectedParty(DWORD disconnectingPartyArtId)
{
  BOOL isSwitchWithDisconnectedParty = FALSE;
  if (m_state == INSWITCH)
  {
    // VNGR-19532 - verify if we are in lecture mode and the disconnecting party is in m_ConfLayout1x1(listeners layout)
    if (IsActiveLectureMode())
    {
      if (!CPObject::IsValidPObjectPtr(m_ConfLayout1x1))
      {
        PASSERTMSG(1, "CVideoBridgeCOP::IsSwitchWithDisconnectedPartye - invalid m_ConfLayout1x1");
      }
      else
      {
        CVidSubImage* pVidSubImage = (*m_ConfLayout1x1)[0];
        if (pVidSubImage)
        {
          DWORD partyRscId = pVidSubImage->GetImageId();
          if (partyRscId)
          {
            CImage* pImage = GetPartyImageLookupTable()->GetPartyImage(partyRscId);
            PASSERTSTREAM(!pImage, "CVideoBridgeCOP::IsSwitchWithDisconnectedParty - Failed, The lookup table doesn't have an element, PartyId:" << partyRscId);
            if (pImage)
            {
              DWORD imageArtId = pImage->GetArtPartyId();
              if (disconnectingPartyArtId == imageArtId)
                isSwitchWithDisconnectedParty = TRUE;
            }
          }
        }
      }
    }

    // /we will check if the party is in one of the cells in the new layout
    if (!isSwitchWithDisconnectedParty)
    {
      WORD numSubImage = m_ConfLayout->GetNumberOfSubImages();
      CPartyImageLookupTable* pPartyImageLookupTable = GetPartyImageLookupTable();
      for (WORD i = 0; i < numSubImage; i++)
      {
        CVidSubImage* pVidSubImage = (*m_ConfLayout)[i];
        if (pVidSubImage)
        {
          DWORD partyRscId = pVidSubImage->GetImageId();
          if (partyRscId)
          {
            CImage* pImage = pPartyImageLookupTable->GetPartyImage(partyRscId);
            PASSERTSTREAM(!pImage, "CVideoBridgeCOP::IsSwitchWithDisconnectedParty - Failed, The lookup table doesn't have an element, PartyId:" << partyRscId);
            if (pImage)
            {
              const DWORD imageArtId = pImage->GetArtPartyId();
              if (disconnectingPartyArtId == imageArtId)
                isSwitchWithDisconnectedParty = TRUE;
            }
          }
        }
      }
    }

    // maybe the party was part of the old layout, and that party id is part of the disconnect decoder request    //we will go over
    if (!isSwitchWithDisconnectedParty)
    {
      for (WORD decoder_index = 0; decoder_index < NUM_OF_COP_DECODERS; decoder_index++)
      {
        eChangeLayoutActionsStatus actionStatusForDisconnect = m_changeLayoutActions->GetChangeLayoutActionStatusForDecoder(decoder_index, eCopDecoderAction_Disconnect);
        if ((actionStatusForDisconnect != eActionNotNeeded) && (actionStatusForDisconnect != eActionComplete))
        {
          DWORD artPartyId = m_changeLayoutActions->GetChangeLayoutActionArtIdForDecoder(decoder_index, eCopDecoderAction_Disconnect);
          if (artPartyId == disconnectingPartyArtId)
          {
            isSwitchWithDisconnectedParty = TRUE;
          }
        }
      }
    }
  }

  return isSwitchWithDisconnectedParty;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::AddNewArtPartyIdToDiscVector(DWORD artPartyId, EMediaDirection disconnectDirection)
{
  DISCONNECTING_VECTOR::iterator itr                         =  m_pDisconnectingPartiesInSwitchVector->begin();
  BOOL                           isPartyArtIdAlreadyInVector = FALSE;
  while (itr != m_pDisconnectingPartiesInSwitchVector->end())
  {
    sDisconnectingParty discParty = *itr;
    if (discParty.artPartyId == artPartyId)
    {
      isPartyArtIdAlreadyInVector = TRUE;
    }

    itr++;
  }

  if (!isPartyArtIdAlreadyInVector)
  {
    sDisconnectingParty disconnectingPartyInfo;
    disconnectingPartyInfo.artPartyId          = artPartyId;
    disconnectingPartyInfo.disconnectDirection = disconnectDirection;
    m_pDisconnectingPartiesInSwitchVector->push_back(disconnectingPartyInfo);
  }

  // if we are waiting to recap from party we will simulate that we received it VNGR-12486
  // the partycntl when it is in delepartycntl doesn't do anything when we ask for recap
  WORD decoderIndex = m_changeLayoutActions->GetDecoderIndexForReCapParty(artPartyId);
  if (decoderIndex < NUM_OF_COP_DECODERS)
  {
    eChangeLayoutActionsStatus actionStatusForChangeMode = m_changeLayoutActions->GetChangeLayoutActionStatusForDecoder(decoderIndex, eCopDecoderAction_ReCapToParty);
    if ((actionStatusForChangeMode != eActionNotNeeded) && (actionStatusForChangeMode != eActionComplete))
    {
      PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::AddNewArtPartyIdToDiscVector changelayout waiting for recap from party we will simulate that we received : ConfName - ", m_pConfName);
      ECopDecoderResolution copDecRes = m_changeLayoutActions->GetChangeLayoutActionDecoderResForDecoder(decoderIndex, eCopDecoderAction_ReCapToParty);
      SimulateUpdateVideoInFromParty(artPartyId, copDecRes);
    }
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::AddPartyToDisconnectVectorAfterSwitchEnds(DWORD disconnectingPartyArtId, EMediaDirection disconnectDirection)
{
  PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::AddPartyToDisconnectVectorAfterSwitchEnds: disconnectingPartyArtId = ", disconnectingPartyArtId);
  AddNewArtPartyIdToDiscVector(disconnectingPartyArtId, disconnectDirection);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::SendDisconnectToAllPartiesInSwitchVector()
{
	PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::SendDisconnectToAllPartiesInSwitchVector");
	DISCONNECTING_VECTOR::iterator itr = m_pDisconnectingPartiesInSwitchVector->begin();
	if (itr != m_pDisconnectingPartiesInSwitchVector->end())
	{
		// VNGR-18594 In case of delayed disconnection of party its sub image could be deleted and as a result because it wasnt updated in the m_pCopConfLayout of CBridgeVideoOutCopEncoder when we compare the layouts we could have reach deleted pointer.
		// in this case we will send in the next change layout sendChangeLayoutAnyWay
		// We will update the encoders that in the nexr changelayout request we will send changelayout anyway.
		PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::SendDisconnectToAllPartiesInSwitchVector the list is not empty we will change layout anyway");
		m_SendChangeLayoutAnyWay = YES;
	}

	while (itr != m_pDisconnectingPartiesInSwitchVector->end())
	{
		sDisconnectingParty discParty = *itr;
		PartyRsrcID artPartyId = discParty.artPartyId;
		EMediaDirection disconnectDirection = discParty.disconnectDirection;

		CVideoBridgePartyCntlCOP* pPartyCntl = (CVideoBridgePartyCntlCOP*)GetPartyCntl(artPartyId);
		if (pPartyCntl)
		{
			pPartyCntl->SetDisConnectingDirectionsReq(disconnectDirection);
			CBridge::DisconnectParty(artPartyId);
		}

		m_pDisconnectingPartiesInSwitchVector->erase(itr);
	}
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnCopEncoderSyncIndCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnCopEncoderSyncIndCONNECTED : ConfName - ", m_pConfName);
  OnCopEncoderSyncInd(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnCopEncoderSyncIndINSWITCH(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnCopEncoderSyncIndINSWITCH : ConfName - ", m_pConfName);
  OnCopEncoderSyncInd(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnCopEncoderSyncInd(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnCopEncoderSyncInd : ConfName - ", m_pConfName);

  DWORD levelEncoderIndex = (DWORD)-1;
  *pParam >> levelEncoderIndex;

  if (levelEncoderIndex == VSW_ENCODER_INDEX)
  {
    PTRACE2(eLevelError, "CVideoBridgeCOP::OnCopEncoderSyncInd VSW_ENCODER_INDEX - Ask fast update from lecturer, Name - ", m_pConfName);
    FastUpdateFromLecturer(TRUE);
  }
  else
  {
    PTRACE2(eLevelError, "CVideoBridgeCOP::OnCopEncoderSyncInd not VSW_ENCODER_INDEX - do nothing, Name - ", m_pConfName);
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::DumpConnnections(const char* pDumpHeader) const
{
	std::ostringstream msg;
	msg.precision(0);

	msg << "CVideoBridgeCOP::DumpConnnections(" << pDumpHeader << ") - ConfName:" << m_pConfName << endl;

	msg << " ------------+-------------------------------------------------------------------------------------------------------------------" << endl;
	msg << " party id    | party name " << endl;
	msg << " ------------+-------------------------------------------------------------------------------------------------------------------" << endl;

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
		{
			msg << " " << setw(11) << right << pPartyCntl->GetPartyRsrcID() << " |" << " " << left  << pPartyCntl->GetName();

			if (IsLecturer(pPartyCntl))
				msg << " (Lecturer)";
			msg << endl;
		}
	}

	msg << " ------------+--------+---------+------------------------------------------------------------------------------------------------" << endl;
	msg << " entity type | entity | connect | connected participants " << endl;
	msg << "             | id     | id      |                        " << endl;
	msg << " ------------+--------+---------+------------------------------------------------------------------------------------------------" << endl;

	DWORD partyId;
	for (WORD decoder_index = 0; decoder_index < NUM_OF_COP_DECODERS; ++decoder_index)
	{
		partyId = m_pCopDecoders[decoder_index]->GetCopDecoderArtId();
		msg << " " << setw( 8) << left << "decoder[" << setw(2) << right << decoder_index << "] |"
				<< " " << setw( 6) << right << (int)m_pCopRsrcs->dynamicDecoders[decoder_index].rsrcEntityId << " |"
				<< " " << setw( 7) << right << (int)m_pCopRsrcs->dynamicDecoders[decoder_index].connectionId << " |";
		if (partyId != (DWORD)-1)
			msg << " " << left << partyId;
		msg << endl;
	}
	partyId = m_pCopDecoderVsw->GetCopDecoderArtId();
	msg << " " << setw(11) << left << "vsw decoder" << " |"
			<< " " << setw( 6) << right << (int)m_pCopDecoderVsw->GetPartyRsrcID() << " |"
			<< " " << setw( 7) << right << "x" << " |";
	if (partyId != (DWORD)-1)
		msg << " " << left << partyId;
	msg << endl;

	partyId = m_pCopDecoderLecturer->GetCopDecoderArtId();
	msg << " " << setw(11) << left << "lec decoder" << " |"
			<< " " << setw( 6) << right << (int)m_pCopDecoderLecturer->GetPartyRsrcID() << " |"
			<< " " << setw( 7) << right << "x" << " |";
	if (partyId != (DWORD)-1)
		msg << " " << left << partyId;
	msg << endl;

	msg << " ------------+--------+---------+------------------------------------------------------------------------------------------------" << endl;

	for (WORD encoder_index = 0; encoder_index < NUM_OF_LEVEL_ENCODERS; ++encoder_index)
	{
		msg << " " << setw( 8) << left << "encoder[" << setw(2) << right << encoder_index << "] |"
				<< " " << setw( 6) << right << (int)m_pCopRsrcs->constRsrcs[encoder_index].rsrcEntityId << " |"
				<< " " << setw( 7) << right << (int)m_pCopRsrcs->constRsrcs[encoder_index].connectionId << " |";
		DumpEncoderParties(m_pCopRsrcs->constRsrcs[encoder_index].rsrcEntityId, msg);
	}
	msg << " " << setw( 8) << left << "vsw encoder |"
			<< " " << setw( 6) << right << (int)m_pCopRsrcs->constRsrcs[eVswEncoder].rsrcEntityId << " |"
			<< " " << setw( 7) << right << (int)m_pCopRsrcs->constRsrcs[eVswEncoder].connectionId << " |";
	DumpEncoderParties(m_pCopRsrcs->constRsrcs[eVswEncoder].rsrcEntityId, msg);

	msg << " " << setw( 8) << left << "pcm encoder |"
			<< " " << setw( 6) << right << (int)m_pCopRsrcs->constRsrcs[ePcmEncoder].rsrcEntityId << " |"
			<< " " << setw( 7) << right << (int)m_pCopRsrcs->constRsrcs[ePcmEncoder].connectionId << " |";
	DumpEncoderParties(m_pCopRsrcs->constRsrcs[ePcmEncoder].rsrcEntityId, msg);

	msg << " ------------+--------+---------+------------------------------------------------------------------------------------------------";

	PTRACE(eLevelInfoNormal, msg.str().c_str());
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::DumpEncoderParties(DWORD encoderEntityId, std::ostringstream& msg) const
{
	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntlCOP* pPartyCntl = (CVideoBridgePartyCntlCOP*)_itr->second;
		if (pPartyCntl)
		{
			if (pPartyCntl->GetCopEncoderEntityId() == encoderEntityId)
				msg << " " << left << pPartyCntl->GetPartyRsrcID() << ";";
		}
	}

	msg << endl;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::DumpLayouts(const char* pDumpHeader)
{
  std::ostringstream msg;
  if (m_SmartLecturerState.IsSmartLevelState())
  {
  	CPartyImageLookupTable* pPartyImageLookupTable = GetPartyImageLookupTable();
    for (int i = 0; i < 2; i++)
    {
      CVidSubImage* pVidSubImage = (i == 0) ? (*m_ConfLayout)[0] : (*m_ConfLayout1x1)[0];
      if (pVidSubImage)
      {
        DWORD partyRscId = pVidSubImage->GetImageId();
        if (partyRscId)
        {
          CImage* pImage = pPartyImageLookupTable->GetPartyImage(partyRscId);
          PASSERTSTREAM(!pImage, "CVideoBridgeCOP::DumpLayouts - Failed, The lookup table doesn't have an element, PartyId:" << partyRscId);

          if (pImage)
          {
            DWORD artPartyId = pImage->GetArtPartyId();
            msg << "\n  " << ((i == 0) ? "ConfLayout[0]=" : "Layout1x1 [0]=") << artPartyId;
            CVideoBridgePartyCntl* pVideoBridgePartyCntl = GetVideoBridgePartyCntl(artPartyId);
            if (pVideoBridgePartyCntl)
              msg << " (" << pVideoBridgePartyCntl->GetName() << ")";
          }
        }
      }
    }
  }

  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::DumpLayouts:", msg.str().c_str());
}

////////////////////////////////////////////////////////////////////////////
const char* CVideoBridgeCOP::GetPartyName(DWORD partyId) const
{
	CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)m_pPartyList->Find(partyId);
	return pPartyCntl ? pPartyCntl->GetName() : "";
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::SetVideoParams(CBridgePartyVideoOutParams& rOutVideoParams, const CBridgePartyVideoParams& rBridgePartyVideoParams) const
{
  // add video params
  rOutVideoParams.SetVideoAlgorithm(rBridgePartyVideoParams.GetVideoAlgorithm());
  rOutVideoParams.SetVideoBitRate(rBridgePartyVideoParams.GetVideoBitRate());
  rOutVideoParams.SetVideoResolution(rBridgePartyVideoParams.GetVideoResolution());

  rOutVideoParams.SetVideoFrameRate(eVideoResolutionQCIF, rBridgePartyVideoParams.GetVideoFrameRate(eVideoResolutionQCIF));
  rOutVideoParams.SetVideoFrameRate(eVideoResolutionCIF, rBridgePartyVideoParams.GetVideoFrameRate(eVideoResolutionCIF));
  rOutVideoParams.SetVideoFrameRate(eVideoResolution4CIF, rBridgePartyVideoParams.GetVideoFrameRate(eVideoResolution4CIF));
  rOutVideoParams.SetVideoFrameRate(eVideoResolutionVGA, rBridgePartyVideoParams.GetVideoFrameRate(eVideoResolutionVGA));
  rOutVideoParams.SetVideoFrameRate(eVideoResolutionSVGA, rBridgePartyVideoParams.GetVideoFrameRate(eVideoResolutionSVGA));
  rOutVideoParams.SetVideoFrameRate(eVideoResolutionXGA, rBridgePartyVideoParams.GetVideoFrameRate(eVideoResolutionXGA));

  rOutVideoParams.SetMBPS(rBridgePartyVideoParams.GetMBPS());
  rOutVideoParams.SetFS(rBridgePartyVideoParams.GetFS());
  rOutVideoParams.SetSampleAspectRatio(rBridgePartyVideoParams.GetSampleAspectRatio());
  rOutVideoParams.SetStaticMB(rBridgePartyVideoParams.GetStaticMB());
  rOutVideoParams.SetIsVideoClarityEnabled(rBridgePartyVideoParams.GetIsVideoClarityEnabled());
  rOutVideoParams.SetMaxDPB(rBridgePartyVideoParams.GetMaxDPB());
}

////////////////////////////////////////////////////////////////////////////
WORD CVideoBridgeCOP::GetCopEncoderIndex(CVideoBridgePartyCntlCOP* pPartyCntl) const
{
  WORD levelEncoderIndex = (WORD)-1;
  if (!CPObject::IsValidPObjectPtr(pPartyCntl))
  {
    PTRACE(eLevelError, "CVideoBridgeCOP::CVideoBridgeCOP::GetCopEncoderIndex, not valid pPartyCntl");
    PASSERT(111);
    return levelEncoderIndex;
  }

  DWORD levelEncoderEntityId = pPartyCntl->GetCopEncoderEntityId();
  levelEncoderIndex = GetCopEncoderIndexFromEntityId(levelEncoderEntityId);
  return levelEncoderIndex;
}

////////////////////////////////////////////////////////////////////////////
BYTE CVideoBridgeCOP::IsUpdateEncodersEnd() const
{
	BYTE isUpdateEncodersEnd = YES;
	const char* party_name = "empty";

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntlCOP* pPartyCntl = (CVideoBridgePartyCntlCOP*)_itr->second;
		if (pPartyCntl)
		{
			if (pPartyCntl->IsUpdateEncoderActive())
			{
				isUpdateEncodersEnd = NO;
				party_name = pPartyCntl->GetName();
				PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::IsUpdateEncodersEnd - active update encoder for party_name = ", party_name);
				break;
			}
		}
	}
	return isUpdateEncodersEnd;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfSetLectureModeCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::OnConfSetLectureModeCONNECTED , ", m_pConfName);

  CLectureModeParams* pNewResrvationLectureMode = new CLectureModeParams; AUTO_DELETE(pNewResrvationLectureMode);
  pNewResrvationLectureMode->DeSerialize(NATIVE, *pParam);

  TRACEINTO << "CVideoBridgeCOP::OnConfSetLectureModeCONNECTED : "<< m_pConfName << *pNewResrvationLectureMode;

  CVideoBridgeLectureModeParams* pNewVideoBridgeLectureMode = new CVideoBridgeLectureModeParams; AUTO_DELETE(pNewVideoBridgeLectureMode);
  *pNewVideoBridgeLectureMode = *pNewResrvationLectureMode;

  if (*pNewVideoBridgeLectureMode == *m_pLectureModeParams)
  {
    CLargeString lstr;
    lstr << "m_pLectureModeParams:\n";
    m_pLectureModeParams->Dump(lstr);
    lstr << "\n";
    lstr << "pNewVideoBridgeLectureMode:\n";
    pNewVideoBridgeLectureMode->Dump(lstr);
    lstr << "\n";

    PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfSetLectureModeCONNECTED No change in lecture mode params, :\n", lstr.GetString());
    return;
  }

  if (pNewVideoBridgeLectureMode->GetLectureModeType() == eLectureModePresentation)
  {
    TRACEINTO << "!!!!CCVideoBridgeCOP::OnConfSetLectureModeCONNECTED illegal LectureModeType - eLectureModePresentation is not supported in COP!, set LectureModeType to none!!!!";
    pNewVideoBridgeLectureMode->SetLectureModeType(eLectureModeNone);
  }

  BYTE waitingToLecturerDecoderSync = m_pCopLectureModeCntl->GetWaitingToLecturerDecoderSync();
  if (waitingToLecturerDecoderSync)
  {
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfSetLectureModeCONNECTED received during wait for lecturer decoder sync. we will send the 1x1 layout to listeners before we start a new lecture mode change, ConfName - ", m_pConfName);
    SendChangeLayoutOnLecturerDecoderSync();
  }

  CMedString mstr;
  mstr << "Old LectureModeParams:\n";
  m_pLectureModeParams->Dump(mstr);
  mstr << "\n\n";
  mstr << "New LectureModeParams:\n";
  pNewVideoBridgeLectureMode->Dump(mstr);
  PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::OnConfSetLectureModeCONNECTED:\n", mstr.GetString());

  LectureModeAction lecture_mode_action = m_pCopLectureModeCntl->GetLectureModeActionNew(*pNewVideoBridgeLectureMode);

  switch (lecture_mode_action)
  {
    case eLmAction_LecturerConnect:
    {
      PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::OnConfSetLectureModeCONNECTED: StartLectureMode, ", m_pConfName);

      // ASYMMETRICCASCADE
      const char*       newLecturerName = pNewVideoBridgeLectureMode->GetLecturerName();
      CBridgePartyCntl* pLecturer       =  GetPartyCntl(newLecturerName);
      if (IsValidPObjectPtr(pLecturer) && ((CVideoBridgePartyCntlCOP*) pLecturer)->IsPartyCascadeLinkSupportAsymmetricEMCascade())
      {
        PTRACE2(eLevelInfoNormal, "LECTURE_MODE:ASYMMETRIC_CASCADE: CVideoBridgeCOP::OnConfSetLectureModeCONNECTED: StartLectureMode - the new lecturer is Cascade Link that supports asymmetric cascade, ", m_pConfName);
        DoLectureModeBaseAction(eLmAction_LecturerConnect_CascadeLecturer, *pNewVideoBridgeLectureMode);
      }
      else
      {
        DoLectureModeBaseAction(eLmAction_LecturerConnect, *pNewVideoBridgeLectureMode);
      }
      break;
    }

    case eLmAction_LecturerDisonnect:
    {
      PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::OnConfSetLectureModeCONNECTED: EndLectureMode, ", m_pConfName);


      CVideoBridgePartyCntl* pLecturer = GetLecturer();

      if (IsValidPObjectPtr(pLecturer) && ((CVideoBridgePartyCntlCOP*) pLecturer)->IsPartyCascadeLinkSupportAsymmetricEMCascade())
      {
        PTRACE2(eLevelInfoNormal, "LECTURE_MODE:ASYMMETRIC_CASCADE: CVideoBridgeCOP::OnConfSetLectureModeCONNECTED: EndLectureMode - the old lecturer is Cascade Link that supports asymmetric cascade, ", m_pConfName);
        DoLectureModeBaseAction(eLmAction_LecturerDisonnect_CascadeLecturer, *pNewVideoBridgeLectureMode);
      }
      else
      {
        DoLectureModeBaseAction(eLmAction_LecturerDisonnect, *pNewVideoBridgeLectureMode);
      }
      break;
    }

    case eLmAction_ChangeLecturer:
    {
      PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::OnConfSetLectureModeCONNECTED: ChangeLecturer, ", m_pConfName);
      CBridgePartyCntl* pLecturer = GetPartyCntl(pNewVideoBridgeLectureMode->GetLecturerName());
      // new lecturer (fixed) is not connected to the bridge, just update DB
      if (pLecturer == NIL(CBridgePartyCntl))
      {
        PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::OnConfSetLectureModeCONNECTED: ChangeLecturer: new lecturer is not connected to bridge,start conf with Audio Activated and set the new name in GUI, Name - ", m_pConfName);
        m_pLectureModeParams->SetAudioActivatedLectureMode(eAudioActivated_LecturerDisonnect);
        m_pCopLectureModeCntl->SetAudioActivatedLectureMode(eAudioActivated_LecturerDisonnect);
        SetLecturerNameInDB(pNewVideoBridgeLectureMode->GetLecturerName());
        SetCurrentSpeakerLecturerParams(*pNewVideoBridgeLectureMode);
        pNewVideoBridgeLectureMode->SetAudioActivatedLectureMode(eAudioActivatedRegular);

        if (pNewVideoBridgeLectureMode->GetLecturerName()[0] == '\0')
        {
          PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::OnConfSetLectureModeCONNECTED: ChangeLecturer: new lecturer is not connected to bridge,start conf with Audio Activated, there is no audio activate party we will end the prev lecturemode, Name - ", m_pConfName);
          DoLectureModeBaseAction(eLmAction_LecturerDisonnect, *pNewVideoBridgeLectureMode);
        }
        else
          DoLectureModeBaseAction(eLmAction_ChangeLecturer, *pNewVideoBridgeLectureMode);
      }
      else
      {
        CVideoBridgePartyCntl* oldLecturer              = GetLecturer();
        LectureModeAction      changeLecturerBaseAction = GetLectureModeChangeLecturerBaseAction(((CVideoBridgePartyCntlCOP*) oldLecturer), ((CVideoBridgePartyCntlCOP*) pLecturer));
        PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::changeLecturerBaseAction - change lecturer base action type:", changeLecturerBaseAction);
        DoLectureModeBaseAction(changeLecturerBaseAction, *pNewVideoBridgeLectureMode);
      }
      break;
    }

    case eLmAction_LecturerStartAudioActivated:
    {
      PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::OnConfSetLectureModeCONNECTED: StartAudioActivated, ", m_pConfName);

      SetCurrentSpeakerLecturerParams(*pNewVideoBridgeLectureMode);
      pNewVideoBridgeLectureMode->SetAudioActivatedLectureMode(eAudioActivatedRegular);
      CBridgePartyCntl* pLecturer = GetPartyCntl(pNewVideoBridgeLectureMode->GetLecturerName());

      if (IsValidPObjectPtr(pLecturer) && pLecturer && ((CVideoBridgePartyCntlCOP*) pLecturer)->IsPartyCascadeLinkSupportAsymmetricEMCascade())
      {
        PTRACE2(eLevelInfoNormal, "LECTURE_MODE:ASYMMETRIC_CASCADE: CVideoBridgeCOP::OnConfSetLectureModeCONNECTED: StartAudioActivated- the new lecturer is Cascade Link that supports asymmetric cascade, ", m_pConfName);
        DoLectureModeBaseAction(eLmAction_LecturerConnect_CascadeLecturer, *pNewVideoBridgeLectureMode);
      }
      else
        DoLectureModeBaseAction(eLmAction_LecturerConnect, *pNewVideoBridgeLectureMode);

      break;
    }

    case eLmAction_ChangeLecturerToAudioActivated:
    {
      PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::OnConfSetLectureModeCONNECTED: ChangeLecturerToAudioActivated, ", m_pConfName);

      SetCurrentSpeakerLecturerParams(*pNewVideoBridgeLectureMode);
      pNewVideoBridgeLectureMode->SetAudioActivatedLectureMode(eAudioActivatedRegular);

      CBridgePartyCntl* pLecturer                     = GetPartyCntl(pNewVideoBridgeLectureMode->GetLecturerName());

      CVideoBridgePartyCntl* oldLecturer              = GetLecturer();
      LectureModeAction      changeLecturerBaseAction = GetLectureModeChangeLecturerBaseAction(((CVideoBridgePartyCntlCOP*) oldLecturer), ((CVideoBridgePartyCntlCOP*) pLecturer));
      PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::OnConfSetLectureModeCONNECTED - change lecturer base action type:", changeLecturerBaseAction);
      DoLectureModeBaseAction(changeLecturerBaseAction, *pNewVideoBridgeLectureMode);
      break;
    }

    case eLmAction_ChangeLecturerFromAudioActivated:
    {
      PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::OnConfSetLectureModeCONNECTED: ChangeLecturerFromAudioActivated, ", m_pConfName);

      CBridgePartyCntl* pLecturer = GetPartyCntl(pNewVideoBridgeLectureMode->GetLecturerName());
      // new lecturer (fixed) is not connected to the bridge, just update DB
      if (pLecturer == NIL(CBridgePartyCntl))
      {
        PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::OnConfSetLectureModeCONNECTED: ChangeLecturerFromAudioActivated: new lecturer is not connected to bridge,keep conf with Audio Activated and set the new name in GUI, Name - ", m_pConfName);
        m_pLectureModeParams->SetAudioActivatedLectureMode(eAudioActivated_LecturerDisonnect);
        m_pCopLectureModeCntl->SetAudioActivatedLectureMode(eAudioActivated_LecturerDisonnect);
        SetLecturerNameInDB(pNewVideoBridgeLectureMode->GetLecturerName());
      }
      else
      {
        pNewVideoBridgeLectureMode->SetAudioActivatedLectureMode(eAudioActivatedFalse);
        CVideoBridgePartyCntl* oldLecturer              = GetLecturer();
        LectureModeAction      changeLecturerBaseAction = GetLectureModeChangeLecturerBaseAction(((CVideoBridgePartyCntlCOP*) oldLecturer), ((CVideoBridgePartyCntlCOP*) pLecturer));
        PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::OnConfSetLectureModeCONNECTED - ChangeLecturerFromAudioActivated change lecturer base action type:", changeLecturerBaseAction);
        DoLectureModeBaseAction(changeLecturerBaseAction, *pNewVideoBridgeLectureMode);
      }
      break;
    }

    case eLmAction_LecturerStopAudioActivated:
    {
      PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::OnConfSetLectureModeCONNECTED: LecturerStopAudioActivated, ", m_pConfName);

      pNewVideoBridgeLectureMode->SetAudioActivatedLectureMode(eAudioActivatedFalse);
      CVideoBridgePartyCntl* pLecturer = GetLecturer();
      if (IsValidPObjectPtr(pLecturer) && ((CVideoBridgePartyCntlCOP*) pLecturer)->IsPartyCascadeLinkSupportAsymmetricEMCascade())
      {
        PTRACE2(eLevelInfoNormal, "LECTURE_MODE:ASYMMETRIC_CASCADE: CVideoBridgeCOP::OnConfSetLectureModeCONNECTED: LecturerStopAudioActivated - the old lecturer is Cascade Link that supports asymmetric cascade, ", m_pConfName);
        DoLectureModeBaseAction(eLmAction_LecturerDisonnect_CascadeLecturer, *pNewVideoBridgeLectureMode);
      }
      else
        DoLectureModeBaseAction(eLmAction_LecturerDisonnect, *pNewVideoBridgeLectureMode);
      break;
    }

    case eLmAction_SameLecturerForcedFromAudioActivated:
    {
      PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::OnConfSetLectureModeCONNECTED: eLmAction_SameLecturerForcedFromAudioActivated, only update DB", m_pConfName);

      m_pLectureModeParams->SetAudioActivatedLectureMode(eAudioActivatedFalse);
      m_pCopLectureModeCntl->SetAudioActivatedLectureMode(eAudioActivatedFalse);
      SetLecturerNameInDB(pNewVideoBridgeLectureMode->GetLecturerName());
      break;
    }

    case eLmAction_None:
    {
      PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::OnConfSetLectureModeCONNECTED: eLmAction_None - do nothing, ", m_pConfName);
      break;
    }

    default:
    {
      PASSERT((DWORD)lecture_mode_action);
      break;
    }
  } // switch

  POBJDELETE(pNewResrvationLectureMode);
  POBJDELETE(pNewVideoBridgeLectureMode);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfSetLectureModeINSWITCH(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::OnConfSetLectureModeINSWITCH , wating for switch to end ", m_pConfName);

  CLectureModeParams* pNewResrvationLectureMode = new CLectureModeParams; AUTO_DELETE(pNewResrvationLectureMode);
  pNewResrvationLectureMode->DeSerialize(NATIVE, *pParam);

  TRACEINTO << "OnConfSetLectureModeINSWITCH : "<< m_pConfName << *pNewResrvationLectureMode;
  m_pCopLectureModeCntl->SetWaitingToEndChangeLayout(WAIT_API);
  m_pCopLectureModeCntl->SetWaitingToEndChangeLayoutParams(*pNewResrvationLectureMode);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::StartLectureMode()
{
  // check params
  PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::StartLectureMode: Name - ", m_pConfName);
  if (NULL == m_pLectureModeParams)
  {
    PTRACE2(eLevelError, "CVideoBridge::StartLectureMode : invalid lecture mode params, Name - ", m_pConfName);
    DBGPASSERT_AND_RETURN(1);
  }

  if (m_pLectureModeParams->GetLecturerName()[0] == '\0')
  {
    PTRACE2(eLevelError, "CVideoBridge::StartLectureMode : invalid lecturer`s name (audio activated and no active speaker(?)-->only update DB), Name - ", m_pConfName);
    m_pCopLectureModeCntl->UpdateCurrentParams(*m_pLectureModeParams);
    UpdateConfDBLectureMode();
    return;
  }

  // this function begins lecture mode
  if (m_pLectureModeParams->GetLecturerName() == NIL(const char))
  {
    PTRACE2(eLevelError, "CVideoBridge::StartLectureMode : invalid lecturer`s name, Name - ", m_pConfName);
    DBGPASSERT_AND_RETURN(1);
  }

  DBGPASSERT_AND_RETURN(!m_pLectureModeParams->IsLectureModeOn()); // the lecture mode signal must be ON

  CVideoBridgePartyCntlCOP* pLecturer = (CVideoBridgePartyCntlCOP*)(GetLecturer());
  if (pLecturer == NULL)
  {
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::StartLectureMode: lecturer not found (not connected to bridge-->only update DB), Name - ", m_pConfName);
    m_pCopLectureModeCntl->UpdateCurrentParams(*m_pLectureModeParams);
    UpdateConfDBLectureMode();
    return;
  }

  if (IsLecturerReady() == NO)
  {
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::StartLectureMode: lecturer not ready only update DB, Name - ", m_pConfName);
    m_pCopLectureModeCntl->UpdateCurrentParams(*m_pLectureModeParams);
    UpdateConfDBLectureMode();
    return;
  }

  // update m_lecturerlevelEncoderIndex
  DWORD levelEncoderEntityId = pLecturer->GetCopEncoderEntityId();
  WORD  levelEncoderIndex    = GetCopEncoderIndexFromEntityId(levelEncoderEntityId);

  if (levelEncoderIndex <= NUM_OF_LEVEL_ENCODERS-1)
  {
    m_lecturerlevelEncoderIndex = levelEncoderIndex;
  }
  else
  {
    PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::StartLectureMode: Lecturer connected, illegal levelEncoderIndex = ", levelEncoderIndex);
  }

  // print connection before start the lecture mode
  DumpConnnections("Start of StartLectureMode()");

  // set active
  m_pCopLectureModeCntl->SetLectureModeActive(YES);
  m_pCopLectureModeCntl->StartLectureMode(*m_pLectureModeParams);

  // start lecturer layout timer
  if (m_pLectureModeParams->GetLectureModeType() == eLectureModeRegular && m_pLectureModeParams->GetIsTimerOn())
  {
//    StartTimer(LECTURE_MODE_TOUT, m_pLectureModeParams->GetTimerInterval()*SECOND);
  //  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::StartLectureMode: start LECTURE_MODE_TOUT - ", m_pConfName);
  }

  // open decoders/encoder
  ChangeModeLecturerByLevelEncoder(pLecturer);
  // OpenLecturerCodecs(pLecturer);

  UpdateConfDBLectureMode();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::EndLectureMode(BYTE removeLecturer)
{
  PTRACE2(eLevelError, "LECTURE_MODE: CVideoBridgeCOP::EndLectureMode: Name - ", m_pConfName);
  // if removeLecturer == 1 this mean that the conference goes to mode regular and LM is over
  // if removeLecturer == 0 this mean that the conference still in LM but temporarly goes to mode transcoding
  if (NULL == m_pLectureModeParams)
  {
    PTRACE2(eLevelError, "CVideoBridgeCOP::EndLectureMode : invalid lecture mode params, Name - ", m_pConfName);
    DBGPASSERT_AND_RETURN(1);
  }

  m_pCopLectureModeCntl->EndLectureMode(*m_pLectureModeParams);

  DumpConnnections("Start of EndLectureMode()");

  // move listeners from vsw encoder to  lecture level encoder
  SwitchListenersEncoder(NO);


  m_pCopLectureModeCntl->SetLectureModeActive(NO);

//  DeleteTimer(LECTURE_MODE_TOUT);

  if (removeLecturer == YES)
    m_pLectureModeParams->SetLecturerName("");
  UpdateConfDBLectureMode();
}

////////////////////////////////////////////////////////////////////////////
void  CVideoBridgeCOP::ChangeLecturer(CVideoBridgeLectureModeParams& pNewVideoBridgeLectureMode, BOOL isUpdateStateNeeded)
{
  PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::ChangeLecturer: Name - ", m_pConfName);

  if (!m_pCopLectureModeCntl->IsLectureModeActive())
  {
    PTRACE(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::ChangeLecturer: IsLectureModeActive() = FALSE (current lecturer is not connected to bridge) --> go to StartLectureMode");
    *m_pLectureModeParams = pNewVideoBridgeLectureMode;
    StartLectureMode();
    return;
  }

  // validity
  if (NULL == m_pLectureModeParams)
  {
    PTRACE2(eLevelError, "CVideoBridgeCOP::ChangeLecturer - Invalid lecture mode params, ConfName=", m_pConfName);
    DBGPASSERT_AND_RETURN(1);
  }

  // this function begins lecture mode
  if (m_pLectureModeParams->GetLecturerName() == NIL(const char))
  {
    PTRACE2(eLevelError, "CVideoBridgeCOP::ChangeLecturer - Invalid lecturer`s name, ConfName=", m_pConfName);
    DBGPASSERT_AND_RETURN(1);
  }

  DBGPASSERT_AND_RETURN(!m_pLectureModeParams->IsLectureModeOn()); // the lecture mode signal must be ON

  CVideoBridgePartyCntlCOP* pNewLecturer = (CVideoBridgePartyCntlCOP*)GetPartyCntl(pNewVideoBridgeLectureMode.GetLecturerName());
  if (pNewLecturer == NULL)
  {
    PTRACE2(eLevelError, "CVideoBridgeCOP::ChangeLecturer - New lecturer not found in bridge --> go to EndLectureMode and update DB, ConfName=", m_pConfName);
    EndLectureMode(NO);
    return;
  }

  if (pNewLecturer->IsReadyToStartLecturer() == NO)
  {
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::ChangeLecturer  Lecturer not ready only update DB, ConfName=", m_pConfName);
    m_pCopLectureModeCntl->UpdateCurrentParams(*m_pLectureModeParams);
    UpdateConfDBLectureMode();
    return;
  }

  // update m_lecturerlevelEncoderIndex
  DWORD levelEncoderEntityId         = pNewLecturer->GetCopEncoderEntityId();
  WORD  newLecturerlevelEncoderIndex = GetCopEncoderIndexFromEntityId(levelEncoderEntityId);

  CVideoBridgePartyCntlCOP* pOldLecturer = (CVideoBridgePartyCntlCOP*)(GetLecturer());

  if (isUpdateStateNeeded)
  {
	  // SMART_SWITCH_LECTURER_1x1: Calculate next optimization state on basis of old/new speakers and the previous optimizations state
	  SetSmartLecturerState(pOldLecturer, pNewLecturer);
  }


  m_pCopLectureModeCntl->ChangeLecturer(pNewVideoBridgeLectureMode);

  DumpConnnections("Start of ChangeLecturer()");

  if (m_SmartLecturerState.m_eState == eLmSmartState_SameLevel1Lecturer)
  {
    // SMART_SWITCH_LECTURER_1x1: Switching from LECTURER2 to LECTURER1 - connect all listeners (except LECTURER1)
    // from LECTURER1 level encoder to VSW encoder. Thats why IsSwitchToVSW=YES.
    SwitchListenersEncoder(YES);
  }
  else
    SwitchListenersEncoder(NO);

  // start lecturer layout timer
  if (m_pLectureModeParams->GetLectureModeType() == eLectureModeRegular && m_pLectureModeParams->GetIsTimerOn())
  {
  //  StartTimer(LECTURE_MODE_TOUT, m_pLectureModeParams->GetTimerInterval()*SECOND);
   // PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::ChangeLecturer: start 'LECTURE_MODE_TOUT' - ConfName=", m_pConfName);
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::EndStartLectureMode()
{
  PTRACE2(eLevelError, "LECTURE_MODE: CVideoBridgeCOP::EndStartLectureMode: Name - ", m_pConfName);

  m_pCopLectureModeCntl->EndStartLectureMode();
  EndLectureModeBaseAction();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::EndEndLectureMode(BYTE removeLecturer)
{
  PTRACE2(eLevelError, "LECTURE_MODE: CVideoBridgeCOP::EndEndLectureMode: Name - ", m_pConfName);
  m_pCopLectureModeCntl->EndEndLectureMode();

  // when we end lecture mode we need to clean the m_ConfLayout1x1
  POBJDELETE(m_ConfLayout1x1);
  m_ConfLayout1x1 = new CLayout(CP_LAYOUT_1X1, ""); // lecture mode layout

  EndLectureModeBaseAction();

  m_isChangeLayoutBecauseOfLectureModeFlows = TRUE;
  // change layout
  // vngr-18616
  ChangeLayout(m_layoutType, FALSE, YES); // always send layout whn moving back to conf layout
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::EndChangeLecturer(BYTE removeLecturer)
{
  PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::EndChangeLecturer - ConfName=", m_pConfName);
  m_pCopLectureModeCntl->EndChangeLecturer();
  UpdateConfDBLectureMode();
  EndLectureModeBaseAction();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::ChangeModeLecturerByLevelEncoder(CVideoBridgePartyCntlCOP* pLecturer)
{
	TRACEINTO << "LecturerName:" << pLecturer->GetName() << ", LecturerLevelEncoderIndex:" << m_lecturerlevelEncoderIndex;

	StartTimer(COP_LECTURE_MODE_ACTION_TIME_OUT, 2*SECOND);

	m_pConfApi->CopVideoInChangeMode(pLecturer->GetPartyRsrcID(), (BYTE)m_lecturerlevelEncoderIndex, eCop_EncoderIndex);
	m_pCopLectureModeCntl->StartChangeModeToLecturer();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::EndChangeModeLecturerByLevelEncoder()
{
  PTRACE2COND(IsActiveLectureMode(), eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::EndChangeModeLecturerByLevelEncoder - ConfName=", m_pConfName);

  DeleteTimer(COP_LECTURE_MODE_ACTION_TIME_OUT);
  m_pCopLectureModeCntl->EndChangeModeToLecturer();
  CVideoBridgePartyCntl* pLecturer = GetLecturer();
  if (pLecturer == NULL)
  {
    PTRACE2COND(IsActiveLectureMode(), eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::EndChangeModeLecturerByLevelEncoder - Failed, lecturer not found, ConfName=", m_pConfName);
    EndCurrentLectureModeAction();
    return;
  }
  OpenLecturerCodecs((CVideoBridgePartyCntlCOP*)pLecturer);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OpenLecturerCodecs(CVideoBridgePartyCntlCOP* pLecturer)
{
  std::ostringstream msg;
  msg << "LECTURE_MODE: CVideoBridgeCOP::OpenLecturerCodecs:";
  msg << "\n  ConfName           = " << m_pConfName;
  msg << "\n  SmartLecturerState = " << m_SmartLecturerState.GetStateName(eLmSmartState_Current);

  StartTimer(COP_LECTURE_MODE_ACTION_TIME_OUT, 5*SECOND);

  m_pCopLectureModeCntl->StartOpenCodecs();

  if (m_SmartLecturerState.IsDiffLevelState())
  {
    msg << "\n  Lecturer Decoder   = Opening suppressed";
    msg << "\n  VSW Decoder        = Opening allowed";
    msg << "\n  VSW Encoder        = Opening allowed";
    PTRACE2COND(IsActiveLectureMode(), eLevelInfoNormal, "", msg.str().c_str());
  }
  else
  {
    msg << "\n  Lecturer Decoder   = Opening allowed";
    msg << "\n  VSW Decoder        = Opening allowed";
    msg << "\n  VSW Encoder        = Opening allowed";
    DWORD lecturerPartyId        = pLecturer->GetPartyRsrcID();
    WORD  lecturer_decoder_index = (WORD)-1;

    msg << "\n  lecturerPartyId    = " << lecturerPartyId;
    for (WORD decoder_index = 0; decoder_index < NUM_OF_COP_DECODERS; decoder_index++)
    {
      msg << "\n  decoder_index      = " << decoder_index << ", decoder_art_id = " << (int)m_pCopDecoders[decoder_index]->GetCopDecoderArtId();
      if (m_pCopDecoders[decoder_index]->GetCopDecoderArtId() == lecturerPartyId)
      {
        msg << " (this is the lecturer decoder, so disconnect it)";
        lecturer_decoder_index = decoder_index;
      }
    }

    PTRACE2COND(IsActiveLectureMode(), eLevelInfoNormal, "", msg.str().c_str());

    if (lecturer_decoder_index != (WORD)-1)
    {
      m_pCopDecoders[lecturer_decoder_index]->SetDisConnectingDirectionsReq(eMediaIn);
      m_pCopDecoders[lecturer_decoder_index]->DisConnect();
    }

    // open lecturer decoder and connect lecturer
    CreateLecturerDecoder(pLecturer);
    m_pCopDecoderLecturer->Connect(NO);
  }

  CreateLecturerVSWDecoder(pLecturer);
  m_pCopDecoderVsw->Connect(NO);

  // open vsw encoder
  // cretae vsw encoder with the same parameters as vsw decoder
  CBridgePartyVideoInParams* pInVideoParams = new CBridgePartyVideoInParams();
  pLecturer->GetInParamFromPartyCntl(pInVideoParams);
  CreateVswEncoder(*pInVideoParams);
  m_pCopEncoderVsw->Connect(NO);
  POBJDELETE(pInVideoParams);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::EndOpenLecturerCodecs()
{
  PTRACE2COND(IsActiveLectureMode(), eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::EndOpenLecturerCodecs: Name - ", m_pConfName);

  DeleteTimer(COP_LECTURE_MODE_ACTION_TIME_OUT);

  m_pCopLectureModeCntl->EndOpenCodecs();

  LectureModeAction lectureModeAction = m_pCopLectureModeCntl->GetCurrentLectureModeAction();
  switch (lectureModeAction)
  {
    case eLmAction_LecturerConnect:
    {
      SwitchListenersEncoder(YES);
      break;
    }

    case eLmAction_LecturerDisonnect: {
      PTRACE2COND(IsActiveLectureMode(), eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::EndOpenLecturerCodecs received during eLmAction_LecturerDisonnect: Name - ", m_pConfName);
      break;
    }

    case eLmAction_ChangeLecturer:
    case eLmAction_ChangeLecturerToCascadeLecturer:
    case eLmAction_ChangeLecturerFromCascadeLecturer:
    case eLmAction_ChangeLecturerToAndFromCascadeLecturer:
    {
      SwitchListenersEncoder(YES);
      break;
    }

    case eLmAction_LecturerConnect_CascadeLecturer:
    {
      PTRACE2COND(IsActiveLectureMode(), eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::EndOpenLecturerCodecs for cascade lecturer: Name - ", m_pConfName);
      SwitchListenersEncoder(YES);
      break;
    }

    case eLmAction_LecturerDisonnect_CascadeLecturer:
    {
      PTRACE2COND(IsActiveLectureMode(), eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::EndOpenLecturerCodecs received during eLmAction_LecturerDisonnect_CascadeLecturer: Name - ", m_pConfName);
      break;
    }

    case eLmAction_None:
    default: {
      break;
    }
  } // switch
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::CloseLecturerCodecs()
{
  if (m_pCopLectureModeCntl->GetCloseCodecsAction())
  {
    PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::CloseLecturerCodecs - Already closing, ConfName=", m_pConfName);
  }
  else
  {
    StartTimer(COP_LECTURE_MODE_ACTION_TIME_OUT, 5 * SECOND);
    m_pCopLectureModeCntl->StartCloseCodecs();

    std::ostringstream msg;
    msg << "LECTURE_MODE: CVideoBridgeCOP::CloseLecturerCodecs:";
    msg << "\n  ConfName           = " << m_pConfName;
    msg << "\n  SmartLecturerState = " << m_SmartLecturerState.GetStateName(eLmSmartState_Current);

    // SMART_SWITCH_LECTURER_1x1: LECTURE1 and LECTURE2 are in the different encoder level.
    // In any direction of switching LECTURE1->LECTURE2 or LECTURE2->LECTURE1 we should only disconnect VSW decoder from old lecturer and connect it to new lecturer.
    // That why we suppress here closing of Lecturer Decoder and allow only closing of VSW decoder/encoder
    if (m_SmartLecturerState.IsDiffLevelState())
    {
      msg << "\n  Lecturer Decoder   = Closing suppressed";
      msg << "\n  VSW Decoder        = Closing allowed";
      msg << "\n  VSW Encoder        = Closing allowed";
      PTRACE2COND(IsActiveLectureMode(), eLevelInfoNormal, "", msg.str().c_str());
      if (IsValidPObjectPtr(m_pCopDecoderVsw))
        m_pCopDecoderVsw->Close();

      if (IsValidPObjectPtr(m_pCopEncoderVsw))
        m_pCopEncoderVsw->DisConnect();
    }
    else
    {
      msg << "\n  Lecturer Decoder   = Closing allowed";
      msg << "\n  VSW Decoder        = Closing allowed";
      msg << "\n  VSW Encoder        = Closing allowed";
      PTRACE2COND(IsActiveLectureMode(), eLevelInfoNormal, "", msg.str().c_str());
      if (IsValidPObjectPtr(m_pCopDecoderLecturer))
        m_pCopDecoderLecturer->Close();

      if (IsValidPObjectPtr(m_pCopDecoderVsw))
        m_pCopDecoderVsw->Close();

      if (IsValidPObjectPtr(m_pCopEncoderVsw))
        m_pCopEncoderVsw->DisConnect();
    }
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::SwitchListenersEncoder(BYTE toVsw)
{
	if (toVsw)
		TRACEINTO << "LECTURE_MODE: CVideoBridgeCOP::SwitchListenersEncoder - switch participants TO vsw encoder, FROM level encoder index " << m_lecturerlevelEncoderIndex;
	else
		TRACEINTO << "LECTURE_MODE: CVideoBridgeCOP::SwitchListenersEncoder - switch participants FROM vsw encoder, To level encoder index " << m_lecturerlevelEncoderIndex;

	StartTimer(COP_LECTURE_MODE_ACTION_TIME_OUT, 3*SECOND);
	// create video out params
	// same as CreateVswEncoder (if) and CreateLevelEncoders (else)
	CBridgePartyVideoOutParams* pOutVideoParams = NULL;
	if (toVsw)
	{
		// SMART_SWITCH_LECTURER_1x1: LECTURE1 and LECTURE2 are in the different or in same encoder level.
		// In this mode we always want use NEW LECTURER video params
		CVideoBridgePartyCntl* pLecturer = GetLecturer(m_SmartLecturerState.IsSmartLevelState() ? TRUE : FALSE);
		PASSERTMSG_AND_RETURN(pLecturer == NULL, "LECTURE_MODE: CVideoBridgeCOP::SwitchListenersEncoder - Failed, lecturer not found");

		pOutVideoParams = new CBridgePartyVideoOutParams();
		m_pCopLectureModeCntl->StartMovePartiesToVsw();

		// cretae vsw out params with the same parameters as lecturer in params
		CBridgePartyVideoInParams* pLecturerInVideoParams = new CBridgePartyVideoInParams();
		((CVideoBridgePartyCntlCOP*)pLecturer)->GetInParamFromPartyCntl(pLecturerInVideoParams);
		SetVideoParams(*pOutVideoParams, *pLecturerInVideoParams);

		// update encoder party and connection id
		CopRsrcDesc encoderCopRsrcDes = GetCopRsrcDesc(eVswEncoderType);
		pOutVideoParams->SetCopConnectionId(encoderCopRsrcDes.connectionId);
		pOutVideoParams->SetCopPartyId(encoderCopRsrcDes.rsrcEntityId);
		POBJDELETE(pLecturerInVideoParams);
	}
	else   // from vsw to level encoder
	{
		pOutVideoParams = new CBridgePartyVideoOutParams();
		m_pCopLectureModeCntl->StartMovePartiesFromVsw();

		// add video params
		WORD update_status = UpdateEncoderVideoParams(pOutVideoParams, ((CCommConf*)m_pConf->GetCommConf()), m_lecturerlevelEncoderIndex);
		if (update_status == FALSE)
			PASSERT(111);

		CopRsrcDesc encoderCopRsrcDes = GetCopRsrcDesc(eLevelEncoderType, m_lecturerlevelEncoderIndex);
		pOutVideoParams->SetCopConnectionId(encoderCopRsrcDes.connectionId);
		pOutVideoParams->SetCopPartyId(encoderCopRsrcDes.rsrcEntityId);
	}

	// send update encoder to all relevant parties
	WORD num_of_moved_parties = 0;

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntlCOP* pPartyCntl = (CVideoBridgePartyCntlCOP*)_itr->second;
		if (pPartyCntl)
		{
			// SMART_SWITCH_LECTURER_1x1: LECTURE1 and LECTURE2 are in the different or in same encoder level.
			// In any direction of switching LECTURE1->LECTURE2 or LECTURE2->LECTURE1 the old and new lecturer always stay connected to their encoders.
			if (m_SmartLecturerState.IsSmartLevelState()) // Old and new lecturer
			{
				if (IsLecturer(pPartyCntl, FALSE) || IsLecturer(pPartyCntl, TRUE))
					continue;
			}
			else // Old lecturer
			{
				// SMART_SWITCH_LECTURER_1x1: In smart switching scenario lecturer can be connected to VSW encoder.
				// We should move old lecturer from VSW encoder to level encoder during switch from LECTURE2->NONE on same level
				if (IsLecturer(pPartyCntl, FALSE) && toVsw == YES)
					continue;
			}

			WORD levelEncoderIndex = GetCopEncoderIndex(pPartyCntl);
			if (((levelEncoderIndex == m_lecturerlevelEncoderIndex && toVsw == YES) || (levelEncoderIndex == VSW_ENCODER_INDEX && toVsw == NO)))
			{
				pPartyCntl->UpdateVideoOutParams(pOutVideoParams);
				num_of_moved_parties++;
				// when moving from the VSW encoder to the level encoder or the opposite we will remove the flow control party constraint
				// reset the FlowControl rate of the party
				DWORD partyFCRate = pPartyCntl->GetPartyFlowControlRate();
				if (partyFCRate != 0)
				{
					pPartyCntl->SetPartyFlowControlRate(0);
					pPartyCntl->UpdatePartyOnStopFlowControlConstraint();
				}
			}
		}
	}

	POBJDELETE(pOutVideoParams);

	if (!toVsw && num_of_moved_parties > 0)
	{
		m_needToRequestIntraFromLevelEncoder[m_lecturerlevelEncoderIndex] = TRUE;
	}

	// Handle the flow control constraint when moving participants from and to VSW encoder
	UpdateFlowControlRateWhenMovingParticipantsToAndFromVSWEncoder(toVsw);

	if (num_of_moved_parties == 0)
	{
		PTRACE(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::SwitchListenersEncoder, no relevant listeners");
		EndSwitchListenersEncoder();
	}
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::EndSwitchListenersEncoder()
{
  PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::EndSwitchListenersEncoder , ", m_pConfName);

  DeleteTimer(COP_LECTURE_MODE_ACTION_TIME_OUT);

  BYTE moveToVsw   = m_pCopLectureModeCntl->GetMovePartiesToVswAction();
  BYTE moveFromVsw = m_pCopLectureModeCntl->GetMovePartiesFromVswAction();

  // if we here it means that no party is in active update encoder action
  m_pCopLectureModeCntl->EndMovePartiesToVsw();
  m_pCopLectureModeCntl->EndMovePartiesFromVsw();


  LectureModeAction lectureModeAction = m_pCopLectureModeCntl->GetCurrentLectureModeAction();
  switch (lectureModeAction)
  {
    case eLmAction_LecturerConnect:
    {
      m_isChangeLayoutBecauseOfLectureModeFlows = TRUE;
      // vngr-18616
      ChangeLayout(m_layoutType);
      EndStartLectureMode();
      break;
    }

    case eLmAction_LecturerDisonnect:
    {
      PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::EndSwitchListenersEncoder -  CloseLecturerCodecs, ", m_pConfName);
      CloseLecturerCodecs();
      break;
    }

    case eLmAction_ChangeLecturer:
    {
      std::ostringstream msg;
      msg << "LECTURE_MODE: CVideoBridgeCOP::EndSwitchListenersEncoder - eLmAction_ChangeLecturer:"
          << "\n  ConfName           = " << m_pConfName
          << "\n  SmartLecturerState = " << m_SmartLecturerState.GetStateName(eLmSmartState_Current)
          << "\n  IsMoveToVsw        = " << (int)moveToVsw
          << "\n  IsMoveFromVsw      = " << (int)moveFromVsw;

      PTRACE2(eLevelInfoNormal, "", msg.str().c_str());
      if (m_SmartLecturerState.IsSameLevelState())
      {
        // SMART_SWITCH_LECTURER_1x1: LECTURE1 and LECTURE2 are in the same encoder level.
        // In any direction of switching LECTURE1->LECTURE2 or LECTURE2->LECTURE1 we do not close lecturer codecs.
        ChangeLayout(m_ConfLayout->GetLayoutType(), FALSE, YES);
        EndChangeLecturer(NO);
      }
      else
      {
        // SMART_SWITCH_LECTURER_1x1: LECTURE1 and LECTURE2 are in the different encoder level or this is not a SMART_SWITCH case.
        // In any direction of switching LECTURE1->LECTURE2 or LECTURE2->LECTURE1 we reconnect VSW decoder from old lecturer to new lecturer like in not a SMART_SWITCH case.
        if (moveFromVsw)
        {
          CloseLecturerCodecs();                  // closing old lecturer
          break;
        }

        if (moveToVsw)
        {
          m_isChangeLayoutBecauseOfLectureModeFlows = TRUE;
          // vngr-18616
          ChangeLayout(m_layoutType, FALSE, YES); // change lecturer to itself with different level requires force to change layout
          EndChangeLecturer(NO);
        }
      }
      DumpConnnections("End of EndSwitchListenersEncoder()");
      break;
    }

    case eLmAction_LecturerConnect_CascadeLecturer:
    {
      PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::EndSwitchListenersEncoder -  send Updatecascadelink as lecturer, ", m_pConfName);
      CVideoBridgePartyCntlCOP* pLecturer = (CVideoBridgePartyCntlCOP*)(GetLecturer());
      // Romem = klocwork
      if(pLecturer)
    	  UpdateCascadeLinkAsLecturer(pLecturer);
      break;
    }

    case eLmAction_LecturerDisonnect_CascadeLecturer:
    {
      PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::EndSwitchListenersEncoder - end cascade lecturer  CloseLecturerCodecs, ", m_pConfName);
      CloseLecturerCodecs();
      break;
    }

    case eLmAction_ChangeLecturerToCascadeLecturer:
    {
      if (moveFromVsw)
      {
        // closing old lecturer
        CloseLecturerCodecs();
      }

      if (moveToVsw)
      {
        PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::EndSwitchListenersEncoder -  send Updatecascadelink as lecturer,in case of eLmAction_ChangeLecturerToCascadeLecturer ", m_pConfName);
        CVideoBridgePartyCntlCOP* pLecturer = (CVideoBridgePartyCntlCOP*)(GetLecturer());
        // Romem = klocwork
        if(pLecturer)
        	UpdateCascadeLinkAsLecturer(pLecturer);
      }
    }
    break;

    case eLmAction_ChangeLecturerFromCascadeLecturer:
    {
      if (moveFromVsw)
      {
        // closing old lecturer
        CloseLecturerCodecs();
      }

      if (moveToVsw)
      {
        m_isChangeLayoutBecauseOfLectureModeFlows = TRUE;
        // vngr-18616
        ChangeLayout(m_layoutType, FALSE, YES); // change lecturer to itself with different level requires force to change layout
        EndChangeLecturerFromCascadeLecturer();
      }
      break;
    }

    case eLmAction_ChangeLecturerToAndFromCascadeLecturer:
    {
      if (moveFromVsw)
      {
        // closing old lecturer
        CloseLecturerCodecs();
      }

      if (moveToVsw)
      {
        PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::EndSwitchListenersEncoder -  send Updatecascadelink as lecturer,in case of eLmAction_ChangeLecturerToAndFromCascadeLecturer ", m_pConfName);
        CVideoBridgePartyCntlCOP* pLecturer = (CVideoBridgePartyCntlCOP*)(GetLecturer());
        // Romem = klocwork
        if(pLecturer)
        	UpdateCascadeLinkAsLecturer(pLecturer);
      }

      break;
    }

    case eLmAction_None:
    default: {
      break;
    }
  } // switch
}

////////////////////////////////////////////////////////////////////////////
BYTE CVideoBridgeCOP::IsLectureCodecsClosed() const
{
  BYTE isLectureCodecsClosed = NO;
  if (m_SmartLecturerState.IsDiffLevelState())
  {
    // SMART_SWITCH_LECTURER_1x1: LECTURE1 and LECTURE2 are in the different encoder level.
    // In any direction of switching LECTURE1->LECTURE2 or LECTURE2->LECTURE1 we reconnect VSW decoder from old lecturer to new lecturer.
    // So that enough for positive answer if VSW decoder and VSW encoder are closed
    if (m_pCopDecoderVsw->IsClosed() && m_pCopEncoderVsw->IsClosed())
      isLectureCodecsClosed = YES;
  }
  else
  {
    if (m_pCopDecoderLecturer->IsClosed() && m_pCopDecoderVsw->IsClosed() && m_pCopEncoderVsw->IsClosed())
      isLectureCodecsClosed = YES;
  }
  PTRACE2INT(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::IsLectureCodecsClosed -  isLectureCodecsClosed=", (DWORD)isLectureCodecsClosed);
  return isLectureCodecsClosed;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::EndCloseLecturerCodecs()
{
  PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::EndCloseLecturerCodecs, ", m_pConfName);
  // if we here it means that no party is in active update encoder action
  DeleteTimer(COP_LECTURE_MODE_ACTION_TIME_OUT);
  m_pCopLectureModeCntl->EndCloseCodecs();

  LectureModeAction lectureModeAction = m_pCopLectureModeCntl->GetCurrentLectureModeAction();
  switch (lectureModeAction)
  {
    case eLmAction_LecturerConnect:
    {
      PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::EndCloseLecturerCodecs received during eLmAction_LecturerConnect, ", m_pConfName);
      break;
    }

    case eLmAction_LecturerDisonnect:
    {
      PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::EndCloseLecturerCodecs received during eLmAction_LecturerDisonnect, ", m_pConfName);
      EndEndLectureMode(NO);
      break;
    }

    case eLmAction_ChangeLecturer:
    case eLmAction_ChangeLecturerFromCascadeLecturer:
    {
      PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::EndCloseLecturerCodecs received during eLmAction_ChangeLecturer or eLmAction_ChangeLecturerFromCascadeLecturer open codecs with new lecturer, ", m_pConfName);

      CMedString mstr;
      mstr << "Old m_pLectureModeParams:\n";
      m_pLectureModeParams->Dump(mstr);
      mstr << "\n\n";

      *m_pLectureModeParams = *(m_pCopLectureModeCntl->GetCurrentParams());
      mstr << "New m_pLectureModeParams:\n";
      m_pLectureModeParams->Dump(mstr);
      PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::EndCloseLecturerCodecs:\n", mstr.GetString());

      CVideoBridgePartyCntlCOP* pLecturer = (CVideoBridgePartyCntlCOP*)(GetLecturer());
      if (pLecturer == NULL)
      {
        PTRACE2(eLevelError, "CVideoBridgeCOP::EndCloseLecturerCodecs: lecturer not found, Name - ", m_pConfName);
        DBGPASSERT_AND_RETURN(1);
      }

      DWORD levelEncoderEntityId         = pLecturer->GetCopEncoderEntityId();
      WORD  newLecturerlevelEncoderIndex = GetCopEncoderIndexFromEntityId(levelEncoderEntityId);

      CSmallString sstr;
      sstr << "m_lecturerlevelEncoderIndex = " << newLecturerlevelEncoderIndex << " , old value = " << m_lecturerlevelEncoderIndex;
      PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::EndCloseLecturerCodecs: case change lecturer update:", sstr.GetString());

      if (m_lecturerlevelEncoderIndex != newLecturerlevelEncoderIndex)
      {
        // when moving lecturer to another level (update encoder) --> need to send at the end of the process update encoder to the level encoder
        m_needToRequestIntraFromLevelEncoder[m_lecturerlevelEncoderIndex] = TRUE;
      }

      if (newLecturerlevelEncoderIndex <= NUM_OF_LEVEL_ENCODERS-1)
      {
        m_lecturerlevelEncoderIndex = newLecturerlevelEncoderIndex;
      }
      else
      {
        PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::EndCloseLecturerCodecs: Lecturer , illegal newLecturerlevelEncoderIndex = ", newLecturerlevelEncoderIndex);
      }

      ChangeModeLecturerByLevelEncoder(pLecturer);
      break;
    }

    case eLmAction_LecturerDisonnect_CascadeLecturer:
    {
      EndEndCascadeLecturerMode();
      break;
    }

    case eLmAction_ChangeLecturerToCascadeLecturer:
    case eLmAction_ChangeLecturerToAndFromCascadeLecturer:
    {
      CMedString mstr;
      mstr << "Old m_pLectureModeParams:\n";
      m_pLectureModeParams->Dump(mstr);
      mstr << "\n\n";

      *m_pLectureModeParams = *(m_pCopLectureModeCntl->GetCurrentParams());
      mstr << "New m_pLectureModeParams:\n";
      m_pLectureModeParams->Dump(mstr);
      PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::EndCloseLecturerCodecs:\n", mstr.GetString());

      CVideoBridgePartyCntlCOP* pLecturer = (CVideoBridgePartyCntlCOP*)(GetLecturer());
      if (pLecturer == NULL)
      {
        PTRACE2(eLevelError, "CVideoBridgeCOP::EndCloseLecturerCodecs: lecturer not found, Name - ", m_pConfName);
        DBGPASSERT_AND_RETURN(1);
      }

      WORD newLecturerlevelEncoderIndex = pLecturer->GetCopEncoderIndexOfCascadeLinkLecturer();

      CSmallString sstr;
      sstr << "m_lecturerlevelEncoderIndex = " << newLecturerlevelEncoderIndex << " , old value = " << m_lecturerlevelEncoderIndex;
      PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::EndCloseLecturerCodecs: case change lecturer to cascade lecturer update:", sstr.GetString());

      if (m_lecturerlevelEncoderIndex != newLecturerlevelEncoderIndex)
      {
        // when moving lecturer to another level (update encoder) --> need to send at the end of the process update encoder to the level encoder
        m_needToRequestIntraFromLevelEncoder[m_lecturerlevelEncoderIndex] = TRUE;
      }
      StartCascadeLinkAsLecturerPendingMode(pLecturer);
    }

    case eLmAction_None:
    default: {
      break;
    }
  } // switch
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfUpdateAutoLayoutCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfUpdateAutoLayoutCONNECTED : ConfName - ", m_pConfName);
  OnConfUpdateAutoLayout(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfUpdateAutoLayoutINSWITCH(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfUpdateAutoLayoutINSWITCH : ConfName - ", m_pConfName);
  OnConfUpdateAutoLayout(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfUpdateAutoLayout(CSegment* pParam)
{
  WORD newIsAutoLayout = NO;

  *pParam >> newIsAutoLayout;

  // check no change ->return
  if ((GetIsAutoLayout() && newIsAutoLayout) || (!GetIsAutoLayout() && !newIsAutoLayout))
    return;

  // auto layout turned on
  if (!GetIsAutoLayout() && newIsAutoLayout)
  {
    SetIsAutoLayout(newIsAutoLayout);
    StartAutoLayout();
    ChangeLayout(m_layoutType); // NEW for cop (in CP the StartAutoLayout send changeLayout to all parties)
  }

  // auto layout turned off
  else if (GetIsAutoLayout() && !newIsAutoLayout)
  {
  	SetIsAutoLayout(newIsAutoLayout);
  }

  m_pConfApi->UpdateDB((CTaskApp*)0xFFFF, UPDATEAUTOLAYOUT, newIsAutoLayout);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfUpdateAutoLayoutIDLE(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfUpdateAutoLayoutIDLE : ConfName - ", m_pConfName);
  WORD newIsAutoLayout = NO;
  *pParam >> newIsAutoLayout;
  // check no change ->return

  if ((GetIsAutoLayout() && newIsAutoLayout) || (!GetIsAutoLayout() && !newIsAutoLayout))
    return;

  // auto layout turned on
  if (!GetIsAutoLayout() && newIsAutoLayout)
  {
      SetIsAutoLayout(newIsAutoLayout);
      StartAutoLayout();
  }

  // auto layout turned off
  else if (GetIsAutoLayout() && !newIsAutoLayout)
  {
      SetIsAutoLayout(newIsAutoLayout);
  }

  m_pConfApi->UpdateDB((CTaskApp*)0xFFFF, UPDATEAUTOLAYOUT, newIsAutoLayout);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::StartAutoLayout()
{
  if (GetIsAutoLayout() && m_pAutoLayoutSet != NULL) // Conf set with Auto Layout ON
  {
    LayoutType tempLayoutType = m_pAutoLayoutSet->GetLayoutType(GetPartyImageVectorSizeUnmuted(), IsSameLayout());
    if (tempLayoutType != m_layoutType)
    {
      TRACEINTO << "CVideoBridgeCOP::StartAutoLayout: New Layout Type: "<< LayoutTypeAsString[tempLayoutType] << " : Name - " << m_pConfName;
      ChangeConfLayoutType(tempLayoutType);
    }
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfSetAutoScanOrderCONNECTED(CSegment* pParam)
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::OnConfSetAutoScanOrderCONNECTED");
  OnConfSetAutoScanOrder(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfSetAutoScanOrderINSWITCH(CSegment* pParam)
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::OnConfSetAutoScanOrderINSWITCH");
  OnConfSetAutoScanOrder(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfSetAutoScanOrder(CSegment* pParam)
{
  CAutoScanOrder* pAutoScanOrder = new CAutoScanOrder;
  pAutoScanOrder->DeSerialize(NATIVE, *pParam);

  pAutoScanOrder->Dump();

  if (m_pAutoScanParams)
    m_pAutoScanParams->InitScanOrder(pAutoScanOrder);

  // if scan order has been changed while auto scan is active
  // start change layout again in order to set new images in auto scan cell
  if (IsValidTimer(AUTO_SCAN_TIMER))
  {
    // vngr-18616
    ChangeLayout(m_layoutType);
  }

  POBJDELETE(pAutoScanOrder);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfUpdateFlowControlRateCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfUpdateFlowControlRateCONNECTED : Name - ", m_pConfName);
  OnConfUpdateFlowControlRate(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfUpdateFlowControlRateINSWITCH(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfUpdateFlowControlRateINSWITCH : Name - ", m_pConfName);
  OnConfUpdateFlowControlRate(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfUpdateFlowControlRate(CSegment* pParam)
{
	PartyRsrcID partyId;
	DWORD newBitRate;
	WORD channelDirection, roleLabel;
	CLPRParams* pLpr = NULL;
	CLPRParams* pLprParams = NULL;
	BYTE bIsCascade;
	BOOL isLpr = 0;

	*pParam >> partyId >> newBitRate >> channelDirection >> roleLabel >> bIsCascade >> isLpr;
	if (isLpr == TRUE)
		*pParam >> (DWORD&)pLpr;

	if (pLpr != NULL)
	{
		PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::OnConfUpdateFlowControlRate : LPR Request for flow control!!");
		pLprParams = new CLPRParams;
		*pLprParams = *pLpr;
	}

	PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::OnConfUpdateFlowControlRate : received rate -  ", newBitRate);

	if (!newBitRate)
	{
		PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::OnConfUpdateFlowControlRate : Not handled. newBitRate = 0");
		POBJDELETE(pLprParams);
		return;
	}

	// 1. Extract the LPR data (if there is one)
	// 2. Only if the request is valid (the rate < encoder's original rate) we will continue.
	// 3. Save the updated party's FC request rate in the VBPartyCntl
	// 4. If the party is connected to PCM update the PCM encoder
	// 5. else If after this request the min FCRate of all the participants connected to this encoder has changed update the VBCOPEncoder and send update out params to the video encoder
	// 6. In case there is LPR data - Send a new API with the LPR data back towards the PartyControl that sent the request (The TaskApp)

	CVideoBridgePartyCntlCOP* pFlowCntlInitiatorPartyCntl = (CVideoBridgePartyCntlCOP*)GetPartyCntl(partyId);
	if (CPObject::IsValidPObjectPtr(pFlowCntlInitiatorPartyCntl))
	{
		DWORD encoderFlowControlRate = 0;
		DWORD encoderOriginalVideoRate = 0;
		DWORD levelEncoderEntityId = pFlowCntlInitiatorPartyCntl->GetCopEncoderEntityId();
		WORD levelEncoderIndex = GetCopEncoderIndexFromEntityId(levelEncoderEntityId);
		if ((levelEncoderIndex > NUM_OF_LEVEL_ENCODERS - 1) && (levelEncoderIndex != VSW_ENCODER_INDEX))
		{
			PTRACE2(eLevelError, "CVideoBridgeCOP::OnConfUpdateFlowControlRate invalid levelEncoderIndex - not updating the FC, Name - ", m_pConfName);
			PASSERT(levelEncoderIndex);
			POBJDELETE(pLprParams);
			return;
		}

		if (levelEncoderIndex == VSW_ENCODER_INDEX)
		{
			HandleFlowControlRequestForVSWEncoder(pFlowCntlInitiatorPartyCntl, newBitRate, pLprParams);
			return;
		}

		encoderFlowControlRate = m_pCopLevelEncoders[levelEncoderIndex]->GetEncoderFlowControlRate();
		encoderOriginalVideoRate = m_pCopLevelEncoders[levelEncoderIndex]->GetOutVideoRate();
		if (encoderOriginalVideoRate < newBitRate)
		{
			CMedString mstr;
			mstr << "\nEncoder video rate = " << encoderOriginalVideoRate;
			mstr << ", Party's flow control request rate = " << newBitRate;
			FPTRACE2(eLevelError, "CVideoBridgeCOP::OnConfUpdateFlowControlRate invalid - the requested flow control rate is bigger than the encoder video rate we will ignore request:", mstr.GetString());
			return;
		}

		((CVideoBridgePartyCntlCOP*)pFlowCntlInitiatorPartyCntl)->SetPartyFlowControlRate(newBitRate);

		// in case the party is connected to the PCM encoder
		if (pFlowCntlInitiatorPartyCntl->IsConnectedOrConnectingPCM())
		{
			PTRACE2(eLevelError, "CVideoBridgeCOP::OnConfUpdateFlowControlRate Party is connected to PCM, Name - ", m_pConfName);
			// In case of PCM we will update the PCM encoder with the party's request
			DWORD pcmEncoderFlowControlRate = m_pPCMEncoderCntl->GetEncoderFlowControlRate();
			if (pcmEncoderFlowControlRate != newBitRate) // ONLY ONE PARTY CONNECTED TO THE PCM
			{
				pcmEncoderFlowControlRate = newBitRate;
				PTRACE2INT(eLevelError, "CVideoBridgeCOP::OnConfUpdateFlowControlRate Party is connected to PCM. PCM encoder new FlowControl Rate =", newBitRate);
				m_pPCMEncoderCntl->UpdateEncoderFlowControlRate(pcmEncoderFlowControlRate);
			}

			if (pLprParams != NULL)
			{
				pFlowCntlInitiatorPartyCntl->ForwardFlowControlCommand(pcmEncoderFlowControlRate, pLprParams);
			}

			return;
		}

		DWORD newFlowControlRate = FindLowestFlowControlRateForLevelEncoder(levelEncoderEntityId);
		if (encoderFlowControlRate != newFlowControlRate)
		{
			CMedString mstr;
			mstr << "Encoder Index = " << levelEncoderIndex;
			mstr << ", New Flow Control Rate = " << newFlowControlRate;
			FPTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfUpdateFlowControlRate: ", mstr.GetString());
			m_pCopLevelEncoders[levelEncoderIndex]->UpdateEncoderFlowControlRate(newFlowControlRate);
			if (pLprParams != NULL)
			{
				pFlowCntlInitiatorPartyCntl->ForwardFlowControlCommand(newFlowControlRate, pLprParams);
			}
		}
		else
		{
			if (pLprParams != NULL)
			{
				PTRACE2(eLevelError, "CVideoBridgeCOP::OnConfUpdateFlowControlRate Do not change encoder rate but do update party with LPR params - ", m_pConfName);
				pFlowCntlInitiatorPartyCntl->ForwardFlowControlCommand(newFlowControlRate, pLprParams);
			}
		}
	}

	POBJDELETE(pLprParams);
}

////////////////////////////////////////////////////////////////////////////
DWORD CVideoBridgeCOP::FindLowestFlowControlRateForLevelEncoder(DWORD levelEncoderEntityId)
{
	DWORD newFlowControlRate = 0, curPartyRate;

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntlCOP* pPartyCntl = (CVideoBridgePartyCntlCOP*)_itr->second;
		if (pPartyCntl)
		{
			DWORD partyLevelEncoderEntityId = pPartyCntl->GetCopEncoderEntityId();
			if (partyLevelEncoderEntityId == levelEncoderEntityId)
			{
				if (pPartyCntl->IsConnectedOrConnectingPCM())
				{
					PTRACE2(eLevelError, "CVideoBridgeCOP::FindLowestFlowControlRateForLevelEncoder Party is connected to PCM encoder we wont take it in the calculation ", pPartyCntl->GetName());
				}
				else
				{
					curPartyRate = pPartyCntl->GetPartyFlowControlRate();
					if ((curPartyRate) && ((curPartyRate < newFlowControlRate) || (!newFlowControlRate)))
						newFlowControlRate = curPartyRate;
				}
			}
		}
	}
	return newFlowControlRate;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::UpdateFlowControlRateWhenPartyConnectOrDisconnectsFromPCMEncoder(CVideoBridgePartyCntlCOP* pPartyCntl, BOOL isConnectToPCM)
{
	// FlowControl support when connecting party to PCM encoder
	// In case the party has asked for flow control request:
	// 1. We will update the PCM encoder with that rate
	// 2. Calculate the new flow control rate of the original encoder and if it changed update it
	DWORD partyFlowControlRate      = pPartyCntl->GetPartyFlowControlRate();
	DWORD levelEncoderEntityId      = pPartyCntl->GetCopEncoderEntityId();
	WORD  levelEncoderIndex         = GetCopEncoderIndexFromEntityId(levelEncoderEntityId);
	DWORD pcmEncoderFlowControlRate = m_pPCMEncoderCntl->GetEncoderFlowControlRate();
	DWORD encoderFlowControlRate    = 0;
	DWORD encoderOriginalVideoRate  = 0;
	if ((levelEncoderIndex > NUM_OF_LEVEL_ENCODERS-1) && (levelEncoderIndex != VSW_ENCODER_INDEX))
	{
		PTRACE2(eLevelError, "CVideoBridgeCOP::UpdateFlowControlRateWhenPartyConnectOrDisconnectsFromPCMEncoder invalid levelEncoderIndex - not updating the FC, Name - ", m_pConfName);
		PASSERT(levelEncoderIndex);
		return;
	}

	if (levelEncoderIndex != VSW_ENCODER_INDEX)
	{
		encoderFlowControlRate   = m_pCopLevelEncoders[levelEncoderIndex]->GetEncoderFlowControlRate();
		encoderOriginalVideoRate = m_pCopLevelEncoders[levelEncoderIndex]->GetOutVideoRate();
	}
	else
	{
		encoderFlowControlRate   = m_pCopEncoderVsw->GetEncoderFlowControlRate();
		encoderOriginalVideoRate = m_pCopEncoderVsw->GetOutVideoRate();
	}

	if (isConnectToPCM)
	{
		// 1. set the PCM encoder with the new FC rate
		if ((levelEncoderIndex == m_pcmLevelIndex) || ((levelEncoderIndex == VSW_ENCODER_INDEX) && (m_pcmLevelIndex == m_lecturerlevelEncoderIndex)))
		{
			m_pPCMEncoderCntl->UpdateEncoderFlowControlRate(partyFlowControlRate);
		}
		else
		{
			// we will update the encoder any way so we will just set now the FC rate
			m_pPCMEncoderCntl->SetEncoderFlowControlRate(partyFlowControlRate);
		}

		// 2. find the new FC rate without this party for the encoder
		DWORD curPartyFCRate     = 0;
		DWORD newFlowControlRate = 0;

		CBridgePartyList::iterator _end = m_pPartyList->end();
		for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
		{
			// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
			CVideoBridgePartyCntlCOP* pCurrParty = (CVideoBridgePartyCntlCOP*)_itr->second;
			if (pCurrParty)
			{
				DWORD partyLevelEncoderEntityId = pCurrParty->GetCopEncoderEntityId();
				CTaskApp* pCurrPartyTaskApp = pCurrParty->GetPartyTaskApp();
				if ((partyLevelEncoderEntityId == levelEncoderEntityId) && (pCurrPartyTaskApp != pPartyCntl->GetPartyTaskApp()))
				{
					curPartyFCRate = pCurrParty->GetPartyFlowControlRate();
					if ((curPartyFCRate) && ((curPartyFCRate < newFlowControlRate) || (!newFlowControlRate)))
						newFlowControlRate = curPartyFCRate;
				}
			}
		}

		if (encoderFlowControlRate != newFlowControlRate)
		{
			CMedString mstr;
			mstr << "Encoder Index = " << levelEncoderIndex;
			mstr << ", New Flow Control Rate = "<< newFlowControlRate;
			FPTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::UpdateFlowControlRateWhenPartyConnectOrDisconnectsFromPCMEncoder: ", mstr.GetString());
			if (levelEncoderIndex == VSW_ENCODER_INDEX)
			{
				m_pCopEncoderVsw->UpdateEncoderFlowControlRate(newFlowControlRate);

				// Send Request to the Lecturer to transmit rate according to the new flow control request
				CVideoBridgePartyCntl* pLecturer = GetLecturer();
				if (IsValidPObjectPtr(pLecturer))
				{
					PTRACE2(eLevelError, "CVideoBridgeCOP::UpdateFlowControlRateWhenPartyConnectOrDisconnectsFromPCMEncoder send request to the lecturer with new flow control rate, Name - ", m_pConfName);
					((CVideoBridgePartyCntlCOP*)pLecturer)->ForwardFlowControlCommand(newFlowControlRate, NULL, FALSE);
				}
			}
			else
				m_pCopLevelEncoders[levelEncoderIndex]->UpdateEncoderFlowControlRate(newFlowControlRate);
		}
	}
	else // disconnect from PCM
	{
		m_pPCMEncoderCntl->UpdateEncoderFlowControlRate(0);
		DWORD newFlowControlRate = FindLowestFlowControlRateForLevelEncoder(levelEncoderEntityId);
		if (encoderFlowControlRate != newFlowControlRate)
		{
			CMedString mstr;
			mstr << "Encoder Index = " << levelEncoderIndex;
			mstr << ", New Flow Control Rate = "<< newFlowControlRate;
			FPTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::UpdateFlowControlRateWhenPartyConnectOrDisconnectsFromPCMEncoder: ", mstr.GetString());
			if (levelEncoderIndex == VSW_ENCODER_INDEX)
			{
				m_pCopEncoderVsw->UpdateEncoderFlowControlRate(newFlowControlRate);
				// Send Request to the Lecturer to transmit rate according to the new flow control request
				CVideoBridgePartyCntl* pLecturer = GetLecturer();
				if (IsValidPObjectPtr(pLecturer))
				{
					PTRACE2(eLevelError, "CVideoBridgeCOP::UpdateFlowControlRateWhenPartyConnectOrDisconnectsFromPCMEncoder send request to the lecturer with new flow control rate, Name - ", m_pConfName);
					((CVideoBridgePartyCntlCOP*)pLecturer)->ForwardFlowControlCommand(newFlowControlRate, NULL, FALSE);
				}
			}
			else
			{
				m_pCopLevelEncoders[levelEncoderIndex]->UpdateEncoderFlowControlRate(newFlowControlRate);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::UpdateFlowControlRateWhenPartyDisconnectsFromEncoder(CVideoBridgePartyCntlCOP* pPartyCntl)
{
  PTRACE2(eLevelError, "CVideoBridgeCOP::UpdateFlowControlRateWhenPartyDisconnectsFromEncoder, Name - ", m_pConfName);

  // party is connected to PCM
  if (pPartyCntl->IsConnectedOrConnectingPCM())
  {
    PTRACE2(eLevelError, "CVideoBridgeCOP::UpdateFlowControlRateWhenPartyDisconnectsFromEncoder Party is connected to PCM, Name - ", m_pConfName);
    m_pPCMEncoderCntl->UpdateEncoderFlowControlRate(0);
    return;
  }

  pPartyCntl->SetPartyFlowControlRate(0);
  pPartyCntl->UpdatePartyOnStopFlowControlConstraint();
  DWORD levelEncoderEntityId = pPartyCntl->GetCopEncoderEntityId();
  WORD  levelEncoderIndex    = GetCopEncoderIndexFromEntityId(levelEncoderEntityId);
  if ((levelEncoderIndex > NUM_OF_LEVEL_ENCODERS-1) && (levelEncoderIndex != VSW_ENCODER_INDEX))
  {
    PTRACE2(eLevelError, "CVideoBridgeCOP::UpdateFlowControlRateWhenPartyDisconnectsFromEncoder invalid levelEncoderIndex - not updating the FC, Name - ", m_pConfName);
    PASSERT(levelEncoderIndex);
    return;
  }

  DWORD encoderFlowControlRate   = 0;
  DWORD encoderOriginalVideoRate = 0;
  if (levelEncoderIndex != VSW_ENCODER_INDEX)
  {
    encoderFlowControlRate   = m_pCopLevelEncoders[levelEncoderIndex]->GetEncoderFlowControlRate();
    encoderOriginalVideoRate = m_pCopLevelEncoders[levelEncoderIndex]->GetOutVideoRate();
  }
  else
  {
    encoderFlowControlRate   = m_pCopEncoderVsw->GetEncoderFlowControlRate();
    encoderOriginalVideoRate = m_pCopEncoderVsw->GetOutVideoRate();
  }

  DWORD newFlowControlRate = FindLowestFlowControlRateForLevelEncoder(levelEncoderEntityId);
  if (encoderFlowControlRate != newFlowControlRate)
  {
    CMedString mstr;
    mstr << "Encoder Index = " << levelEncoderIndex;
    mstr << ", New Flow Control Rate = "<< newFlowControlRate;
    FPTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::UpdateFlowControlRateWhenPartyDisconnectsFromEncoder: ", mstr.GetString());
    if (levelEncoderIndex == VSW_ENCODER_INDEX)
    {
      m_pCopEncoderVsw->UpdateEncoderFlowControlRate(newFlowControlRate);

      // Send Request to the Lecturer to transmit rate according to the new flow control request
      CVideoBridgePartyCntl* pLecturer = GetLecturer();
      if (IsValidPObjectPtr(pLecturer))
      {
        PTRACE2(eLevelError, "CVideoBridgeCOP::UpdateFlowControlRateWhenPartyDisconnectsFromEncoder send request to the lecturer with new flow control rate, Name - ", m_pConfName);

        ((CVideoBridgePartyCntlCOP*)pLecturer)->ForwardFlowControlCommand(newFlowControlRate, NULL /*pLprParams*/, FALSE);
      }
    }
    else
      m_pCopLevelEncoders[levelEncoderIndex]->UpdateEncoderFlowControlRate(newFlowControlRate);
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::SimulateUpdateVideoInFromParty(DWORD artPartyId, ECopDecoderResolution decoderResolution)
{
	PTRACE2(eLevelError, "CVideoBridgeCOP::SimulateUpdateVideoInFromParty Name - ", m_pConfName);

	CVideoBridgePartyCntlCOP* pCurrParty = (CVideoBridgePartyCntlCOP*)m_pPartyList->Find(artPartyId);
	if (pCurrParty)
	{
		DWORD               MBPS            = INVALID;
		DWORD               FS              = INVALID;
		eVideoResolution    videoResolution = eVideoResolutionDummy;
		CCopVideoModeTable* pCopTable       = new CCopVideoModeTable;
		sCopH264VideoMode   copH264VideoMode;
		long                mbps, fs, h264Level;
		WORD                res             = pCopTable->GetDecoderH264Mode(decoderResolution, copH264VideoMode);
		h264Level = copH264VideoMode.levelValue;
		CH264Details h264Details            = h264Level;
		mbps      = copH264VideoMode.maxMBPS;
		fs        = copH264VideoMode.maxFS;
		if (fs == -1)
			fs = h264Details.GetDefaultFsAsDevision();

		if (mbps == -1)
			mbps = h264Details.GetDefaultMbpsAsDevision();

		MBPS = mbps;
		FS   = fs;

		POBJDELETE(pCopTable);
		// for the H263 case
		switch (decoderResolution)
		{
			case COP_decoder_resolution_4CIF25:
			{
				videoResolution = eVideoResolution4CIF;
				break;
			}

			case COP_decoder_resolution_CIF25:
			{
				videoResolution = eVideoResolutionCIF;
				break;
			}

			default:
			{
				PTRACE2(eLevelError, "CVideoBridgeCOP::SimulateUpdateVideoInFromParty not relevant for H263, Name - ", ECopDecoderResolutionString[decoderResolution]);
			}
		} // switch

		CBridgePartyVideoInParams* pInVideoParams = new CBridgePartyVideoInParams();
		pCurrParty->GetInParamFromPartyCntl(pInVideoParams);
		pInVideoParams->SetMBPS(MBPS);
		pInVideoParams->SetFS(FS);
		pInVideoParams->SetVideoResolution(videoResolution);
		if (videoResolution == eVideoResolution4CIF)
			pInVideoParams->SetVideoFrameRate(eVideoResolution4CIF, eVideoFrameRate15FPS);
		else
			pInVideoParams->SetVideoFrameRate(eVideoResolution4CIF, eVideoFrameRateDUMMY);

		pCurrParty->UpdateVideoInParams(pInVideoParams);
		return;
	}


	PTRACE2(eLevelError, "CVideoBridgeCOP::SimulateUpdateVideoInFromParty Didn't find party, Name - ", m_pConfName);
	// we will remove from the change layout actions
	DBGPASSERT(101);
	m_changeLayoutActions->UpdateReCapToPartyActionStatus(artPartyId, eActionComplete);
}

////////////////////////////////////////////////////////////////////////////
BOOL CVideoBridgeCOP::AreThereUnfinishedChangeLayoutActionsWeNeedToHandle()
{
  BOOL ret = FALSE;
  for (int i = 0; i < NUM_OF_COP_DECODERS; i++)
  {
    WORD                       decoderIndex              = i;
    eChangeLayoutActionsStatus actionStatusForChangeMode = m_changeLayoutActions->GetChangeLayoutActionStatusForDecoder(i, eCopDecoderAction_ReCapToParty);
    eChangeLayoutActionsStatus actionStatusForClose      = m_changeLayoutActions->GetChangeLayoutActionStatusForDecoder(i, eCopDecoderAction_Close);
    eChangeLayoutActionsStatus actionStatusForDisconnect = m_changeLayoutActions->GetChangeLayoutActionStatusForDecoder(i, eCopDecoderAction_Disconnect);
    if (m_IsSmartSwitch) // if we are smart switch there might be actions that we didn't even start
    {
      if ((actionStatusForChangeMode == eActionNeededInFirstPrior) || (actionStatusForClose == eActionNeededInFirstPrior) || (actionStatusForDisconnect == eActionNeededInFirstPrior))
      {
        ret = TRUE;
      }
    }

    if ((actionStatusForChangeMode != eActionNotNeeded) && (actionStatusForChangeMode != eActionComplete))
    {
      ret = TRUE;
    }
  }

  return ret;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::HandleUnfinishedChangeLayoutActions()
{
  PTRACE2(eLevelError, "CVideoBridgeCOP::HandleUnfinishedChangeLayoutActions, Name - ", m_pConfName);
  for (int i = 0; i < NUM_OF_COP_DECODERS; i++)
  {
    eChangeLayoutActionsStatus actionStatusForChangeMode = m_changeLayoutActions->GetChangeLayoutActionStatusForDecoder(i, eCopDecoderAction_ReCapToParty);
    eChangeLayoutActionsStatus actionStatusForClose      = m_changeLayoutActions->GetChangeLayoutActionStatusForDecoder(i, eCopDecoderAction_Close);
    eChangeLayoutActionsStatus actionStatusForDisconnect = m_changeLayoutActions->GetChangeLayoutActionStatusForDecoder(i, eCopDecoderAction_Disconnect);

    if (m_IsSmartSwitch) // if we are smart switch there might be actions that we didn't even start
    {
      // simulate the recap succeded for the party we tried to change in first prior and it didn't finish
      if (actionStatusForChangeMode == eActionNeededInFirstPrior)
      {
        PTRACE2(eLevelError, "CVideoBridgeCOP::HandleUnfinishedChangeLayoutActions case smart switch change mode, Name - ", m_pConfName);
        DWORD                 artPartyId        = m_changeLayoutActions->GetChangeLayoutActionArtIdForDecoder(i, eCopDecoderAction_ReCapToParty);
        ECopDecoderResolution decoderResolution = m_changeLayoutActions->GetChangeLayoutActionDecoderResForDecoder(i, eCopDecoderAction_ReCapToParty);
        m_changeLayoutActions->UpdateActionStatus(i, eCopDecoderAction_ReCapToParty, eActionInProgress);
        SimulateUpdateVideoInFromParty(artPartyId, decoderResolution);
      }

      if (actionStatusForClose == eActionNeededInFirstPrior)
      {
        PTRACE2(eLevelError, "CVideoBridgeCOP::HandleUnfinishedChangeLayoutActions disable smart switch change close action to in progress, Name - ", m_pConfName);
        m_changeLayoutActions->UpdateActionStatus(i, eCopDecoderAction_Close, eActionInProgress);
      }

      if (actionStatusForDisconnect == eActionNeededInFirstPrior)
      {
        PTRACE2(eLevelError, "CVideoBridgeCOP::HandleUnfinishedChangeLayoutActions disable smart switch change disconnect action to in progress, Name - ", m_pConfName);
        m_changeLayoutActions->UpdateActionStatus(i, eCopDecoderAction_Disconnect, eActionInProgress);
      }
    }

    else // not smart switch
    {
      if ((actionStatusForChangeMode != eActionNotNeeded) && (actionStatusForChangeMode != eActionComplete))
      {
        DWORD                 artPartyId        = m_changeLayoutActions->GetChangeLayoutActionArtIdForDecoder(i, eCopDecoderAction_ReCapToParty);
        ECopDecoderResolution decoderResolution = m_changeLayoutActions->GetChangeLayoutActionDecoderResForDecoder(i, eCopDecoderAction_ReCapToParty);
        SimulateUpdateVideoInFromParty(artPartyId, decoderResolution);
      }
    }
  }

  if (m_IsSmartSwitch)
  {
    PTRACE2(eLevelError, "CVideoBridgeCOP::HandleUnfinishedChangeLayoutActions finish smart switch, Name - ", m_pConfName);
    m_IsSmartSwitch           = FALSE;
    m_IsSmartSwitchTimerPoped = FALSE;
    StartSwitch(); // to start the other actions that were delayed due to smart switch
  }

  PTRACE2(eLevelError, "CVideoBridgeCOP::HandleUnfinishedChangeLayoutActions restart timer, Name - ", m_pConfName);
  StartTimer(SWITCH_TOUT, SWITCH_TIME_OUT_COP_VALUE);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::RemovePartyFromDisconnectingVectorIfExists(DWORD artPartyId)
{
  DISCONNECTING_VECTOR::iterator itr =  m_pDisconnectingPartiesInSwitchVector->begin();
  while (itr != m_pDisconnectingPartiesInSwitchVector->end())
  {
    sDisconnectingParty discParty = *itr;
    if (discParty.artPartyId == artPartyId)
    {
      m_pDisconnectingPartiesInSwitchVector->erase(itr);
      PTRACE2INT(eLevelError, "CVideoBridgeCOP::RemovePartyFromDisconnectinVector erase party- ", artPartyId);
      return;
    }

    itr++;
  }

  PTRACE2INT(eLevelError, "CVideoBridgeCOP::RemovePartyFromDisconnectinVector party  isn't in disconnecting vector- ", artPartyId);
}

//--------------------------------------------------------------------------
void CVideoBridgeCOP::SetCurrentSpeakerLecturerParams(CVideoBridgeLectureModeParams& rNewVideoBridgeLectureMode)
{
	rNewVideoBridgeLectureMode.SetLecturerName("");

	DWORD partyImageSpeakerId = GetPartyImageSpeakerId();
	TRACECOND_AND_RETURN(!partyImageSpeakerId, "CVideoBridgeCOP::SetCurrentSpeakerLecturerParams - Video speaker is not valid");

	CPartyImageLookupTable* pPartyImageLookupTable = GetPartyImageLookupTable();

	CImage* pImage = pPartyImageLookupTable->GetPartyImage(partyImageSpeakerId);
	PASSERTSTREAM_AND_RETURN(!pImage, "CVideoBridgeCP::SetCurrentSpeakerLecturerParams - Failed, The lookup table doesn't have an element, PartyId:" << partyImageSpeakerId);

	if (pImage->isMuted())
	{
		TRACEINTO << "CVideoBridgeCOP::SetCurrentSpeakerLecturerParams - Video speaker image is muted we will try to find the next party, PartyId:" << partyImageSpeakerId;

		int partyImageVectorSize = GetPartyImageVectorSize();
		for (int i = 1; i < partyImageVectorSize; ++i)
		{
			partyImageSpeakerId = (*m_pPartyImageVector)[i];
			pImage = pPartyImageLookupTable->GetPartyImage(partyImageSpeakerId);
			PASSERTSTREAM_AND_RETURN(!pImage, "CVideoBridgeCP::SetCurrentSpeakerLecturerParams - Failed, The lookup table doesn't have an element, PartyId:" << partyImageSpeakerId);

			if (!pImage->isMuted())
			{
				const CTaskApp* pVideoSource = pImage->GetVideoSource();
				PASSERTSTREAM_AND_RETURN(!IsValidPObjectPtr(pVideoSource), "CVideoBridgeCP::SetCurrentSpeakerLecturerParams - Failed, Video Source is not valid, PartyId:" << partyImageSpeakerId);

				const char* pNewLecturerName = pImage->GetVideoSourceName();
				rNewVideoBridgeLectureMode.SetLecturerName(pNewLecturerName);
				TRACEINTO << "CVideoBridgeCOP::SetCurrentSpeakerLecturerParams - PartyName:" << pNewLecturerName;
				return;
			}
		}
		TRACEINTO << "CVideoBridgeCOP::SetCurrentSpeakerLecturerParams - Video speaker image is muted and there is no other party in the speaker list we can set";
	}
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::DoLectureModeBaseAction(LectureModeAction lecture_mode_action, CVideoBridgeLectureModeParams& rNewVideoBridgeLectureMode)
{
  if (lecture_mode_action != eLmAction_LecturerConnect && lecture_mode_action != eLmAction_LecturerDisonnect && lecture_mode_action != eLmAction_ChangeLecturer &&
      lecture_mode_action != eLmAction_LecturerConnect_CascadeLecturer && lecture_mode_action != eLmAction_LecturerDisonnect_CascadeLecturer &&
      lecture_mode_action != eLmAction_ChangeLecturerToCascadeLecturer && lecture_mode_action != eLmAction_ChangeLecturerFromCascadeLecturer && lecture_mode_action != eLmAction_ChangeLecturerToAndFromCascadeLecturer)
  {
    PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::DoLectureModeBaseAction, illegal base action = ", (DWORD)lecture_mode_action);
    return;
  }

  if (m_pCopLectureModeCntl->IsInAction())
  {
    PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::DoLectureModeBaseAction: setting pending action, ", m_pConfName);
    m_pCopLectureModeCntl->SetPendingAction(lecture_mode_action, rNewVideoBridgeLectureMode);
    m_pCopLectureModeCntl->Dump();
  }
  else if (m_state == INSWITCH)
  {
    PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::DoLectureModeBaseAction: setting wating to end change layout action, ", m_pConfName);
    m_pCopLectureModeCntl->SetWaitingToEndChangeLayout(WAIT_BRIDGE);
    m_pCopLectureModeCntl->SetPendingAction(lecture_mode_action, rNewVideoBridgeLectureMode);
    m_pCopLectureModeCntl->Dump();
  }
  else
  {
    switch (lecture_mode_action)
    {
      case eLmAction_LecturerConnect:
      {
        PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::DoLectureModeBaseAction: StartLectureMode, ", m_pConfName);
        *m_pLectureModeParams = rNewVideoBridgeLectureMode;
        StartLectureMode();
        break;
      }

      case eLmAction_LecturerConnect_CascadeLecturer:
      {
        PTRACE2(eLevelInfoNormal, "LECTURE_MODE:ASYMMETRIC_CASCADE: CVideoBridgeCOP::DoLectureModeBaseAction: StartCascadeLecturerMode, ", m_pConfName);
        *m_pLectureModeParams = rNewVideoBridgeLectureMode;
        StartCascadeLecturerMode();
        break;
      }

      case eLmAction_LecturerDisonnect:
      {
        PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::DoLectureModeBaseAction: EndLectureMode, ", m_pConfName);
        ResetLecturerFlowControlRate();
        *m_pLectureModeParams = rNewVideoBridgeLectureMode;
        EndLectureMode(YES);
        break;
      }

      case eLmAction_ChangeLecturer:
      {
        PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::DoLectureModeBaseAction: ChangeLecturer, ", m_pConfName);
	    ChangeLecturer(rNewVideoBridgeLectureMode, TRUE);
        break;
      }

      case eLmAction_LecturerDisonnect_CascadeLecturer:
      {
        PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::DoLectureModeBaseAction: EndCascadeLecturerMode, ", m_pConfName);
        ResetLecturerFlowControlRate();
        CVideoBridgePartyCntl* pLecturer = GetLecturer();
        *m_pLectureModeParams = rNewVideoBridgeLectureMode;
        EndCascadeLecturerMode(YES, pLecturer);
        break;
      }

      case eLmAction_ChangeLecturerToCascadeLecturer:
      {
        PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::DoLectureModeBaseAction: ChangeLecturerToCascadeLecturer, ", m_pConfName);
        ChangeLecturerToCascadeLecturer(rNewVideoBridgeLectureMode);
        break;
      }

      case eLmAction_ChangeLecturerFromCascadeLecturer:
      {
        PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::DoLectureModeBaseAction: ChangeLecturerFromCascadeLecturer, ", m_pConfName);
        ChangeLecturerFromCascadeLecturer(rNewVideoBridgeLectureMode);
        break;
      }

      case eLmAction_ChangeLecturerToAndFromCascadeLecturer:
      {
        PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::DoLectureModeBaseAction: ChangeLecturerToAndFromCascadeLecturer, ", m_pConfName);
        ChangeLecturerToAndFromCascadeLecturer(rNewVideoBridgeLectureMode);
        break;
      }

      default:
    	// Note: some enumeration value are not handled in switch. Add default to suppress warning.
    	break;
    } // switch
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::EndLectureModeBaseAction()
{
  if (m_pCopLectureModeCntl->IsPendingAction())
  {
    PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::EndLectureModeBaseAction: Executing pending action, ", m_pConfName);
    m_pCopLectureModeCntl->Dump();
    LectureModeAction             pendingAction = m_pCopLectureModeCntl->GetPendingLectureModeAction();
    CVideoBridgeLectureModeParams pendingParams = *(m_pCopLectureModeCntl->GetPendingParams());
    m_pCopLectureModeCntl->UpdateCurrentActionFromPending();
    DoLectureModeBaseAction(pendingAction, pendingParams);
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnTimerLectureModeActionFailed(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::OnTimerLectureModeActionFailed, ", m_pConfName);
  m_pCopLectureModeCntl->Dump();
  DBGPASSERT(2525);

  BYTE openCodecsAction                      = m_pCopLectureModeCntl->GetOpenCodecsAction();
  BYTE closeCodecsAction                     = m_pCopLectureModeCntl->GetCloseCodecsAction();
  BYTE movePartiesToVswAction                = m_pCopLectureModeCntl->GetMovePartiesToVswAction();
  BYTE movePartiesFromVswAction              = m_pCopLectureModeCntl->GetMovePartiesFromVswAction();
  BYTE changeModeToLecturer                  = m_pCopLectureModeCntl->GetChangeModeToLecturer();

  BYTE startCascadeLinkAsLecturerPendingMode = m_pCopLectureModeCntl->GetStartCascadeLinkAsLecturerPendingMode();
  BYTE updateCascadeLinkAsLecturer           = m_pCopLectureModeCntl->GetUpdateCascadeLinkAsLecturer();
  BYTE updateCascadeLinkAsNotLecturer        = m_pCopLectureModeCntl->GetUpdateCascadeLinkAsNotLecturer();


  if (openCodecsAction)
  {
    EndOpenLecturerCodecs();
  }
  else if (closeCodecsAction)
  {
    EndCloseLecturerCodecs();
  }
  else if (movePartiesToVswAction || movePartiesFromVswAction)
  {
    EndSwitchListenersEncoder();
  }
  else if (changeModeToLecturer)
  {
    EndChangeModeLecturerByLevelEncoder();
  }
  else if (startCascadeLinkAsLecturerPendingMode)
  {
    EndStartCascadeLinkAsLecturerPendingMode();
  }
  else if (updateCascadeLinkAsLecturer)
  {
    EndUpdateCascadeLinkAsLecturer();
  }
  else if (updateCascadeLinkAsNotLecturer)
  {
    EndUpdateCascadeLinkAsNotLecturer();
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::ApplicationActionsOnDeletePartyFromConf(const char* pDeletedPartyName)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::ApplicationActionsOnDeletePartyFromConf : Name - ", m_pConfName);

  const char* lecturerName = m_pLectureModeParams->GetLecturerName();

  if (NULL != lecturerName && NULL != pDeletedPartyName && m_pCopLectureModeCntl->IsLectureModeActive())  // VNGR-20762 && m_pCopLectureModeCntl->IsLectureModeActive()
  {
    DWORD len1 = strlen(lecturerName);
    DWORD len2 = strlen(pDeletedPartyName);
    if (len1 == len2)
    {
      if (!strncmp(lecturerName, pDeletedPartyName, len1))
      {
        CVideoBridgeLectureModeParams* pNewVideoBridgeLectureMode = new CVideoBridgeLectureModeParams;
        *pNewVideoBridgeLectureMode = *m_pLectureModeParams;
        PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::CVideoBridgeCOP::ApplicationActionsOnDeletePartyFromConf: lecturer deleted, Name - ", m_pConfName);
        SetCurrentSpeakerLecturerParams(*pNewVideoBridgeLectureMode);
        if (IsLecturerInDB(pDeletedPartyName))                                                            // if we got here pPartyCntl should be valid
        {
          PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::ApplicationActionsOnDeletePartyFromConf: removed lecturer was set by EMA , move conf to Audio Activated but keep the name in GUI, Name - ", m_pConfName);
          pNewVideoBridgeLectureMode->SetAudioActivatedLectureMode(eAudioActivated_LecturerDisonnect);
        }
        else
        {
          pNewVideoBridgeLectureMode->SetAudioActivatedLectureMode(eAudioActivatedRegular);
        }

        if (pNewVideoBridgeLectureMode->GetLecturerName()[0] == '\0')
        {
          CVideoBridgePartyCntl* pLecturer = GetLecturer();

          if (IsValidPObjectPtr(pLecturer) && ((CVideoBridgePartyCntlCOP*) pLecturer)->IsPartyCascadeLinkSupportAsymmetricEMCascade())
          {
            PTRACE2(eLevelInfoNormal, "LECTURE_MODE:ASYMMETRIC_CASCADE: CVideoBridgeCOP::ApplicationActionsOnDeletePartyFromConf: EndLectureMode - the old lecturer is Cascade Link that supports asymmetric cascade, ", m_pConfName);
            m_pCopLectureModeCntl->SetIsDisconnectedCascadeLecturerParty(YES);
            DoLectureModeBaseAction(eLmAction_LecturerDisonnect_CascadeLecturer, *pNewVideoBridgeLectureMode);
          }
          else
            DoLectureModeBaseAction(eLmAction_LecturerDisonnect, *pNewVideoBridgeLectureMode);
        }
        else
        {
          CVideoBridgePartyCntl* oldLecturer         = GetLecturer();
          CVideoBridgePartyCntl* newLecturer         = GetLecturer(*pNewVideoBridgeLectureMode);

          LectureModeAction changeLecturerBaseAction = GetLectureModeChangeLecturerBaseAction(((CVideoBridgePartyCntlCOP*) oldLecturer), ((CVideoBridgePartyCntlCOP*) newLecturer));
          PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::ApplicationActionsOnDeletePartyFromConf - change lecturer base action type:", changeLecturerBaseAction);
          if (changeLecturerBaseAction == eLmAction_ChangeLecturerToAndFromCascadeLecturer || changeLecturerBaseAction == eLmAction_ChangeLecturerFromCascadeLecturer)
            m_pCopLectureModeCntl->SetIsDisconnectedCascadeLecturerParty(YES);

          DoLectureModeBaseAction(changeLecturerBaseAction, *pNewVideoBridgeLectureMode);
        }

        POBJDELETE(pNewVideoBridgeLectureMode);
      }
    }
  }
  if(pDeletedPartyName == NULL)
	  PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::CVideoBridgeCOP::ApplicationActionsOnDeletePartyFromConf: pDeletedPartyName is NULL, Conf Name - ", m_pConfName);
  else
  {
	  if (IsLecturerInDB(pDeletedPartyName))                                                                  // if we got here pPartyCntl should be valid
	  {
		  PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::ApplicationActionsOnDeletePartyFromConf: deleted lecturer was set by EMA , Remove lecturer name from GUI, Name - ", m_pConfName);
		  SetLecturerNameInDB("");
	  }
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::ApplicationActionsOnRemovePartyFromMix(CVideoBridgePartyCntl* pPartyCntl)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::ApplicationActionsOnRemovePartyFromMix : Name - ", m_pConfName);

  if (m_pCopLectureModeCntl->IsLectureModeActive() && IsLecturer(pPartyCntl))                             // VNGR-20899
  {
    CVideoBridgeLectureModeParams* pNewVideoBridgeLectureMode = new CVideoBridgeLectureModeParams;
    *pNewVideoBridgeLectureMode = *m_pLectureModeParams;
    PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::ApplicationActionsOnRemovePartyFromMix: lecturer removed change conf to Audio Activated, Name - ", m_pConfName);
    SetCurrentSpeakerLecturerParams(*pNewVideoBridgeLectureMode);
    if (IsLecturerInDB(pPartyCntl->GetName()))                                                            // if we got here pPartyCntl should be valid
    {
      PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::ApplicationActionsOnRemovePartyFromMix: removed lecturer was set by EMA , move conf to Audio Activated but keep the name in GUI, Name - ", m_pConfName);
      pNewVideoBridgeLectureMode->SetAudioActivatedLectureMode(eAudioActivated_LecturerDisonnect);
    }
    else if (pNewVideoBridgeLectureMode->IsAudioActivatedLectureMode() != (BYTE)eAudioActivated_LecturerDisonnect)
    {
      pNewVideoBridgeLectureMode->SetAudioActivatedLectureMode(eAudioActivatedRegular);
    }

    if (pNewVideoBridgeLectureMode->GetLecturerName()[0] == '\0')
    {
      if (IsValidPObjectPtr(pPartyCntl) && ((CVideoBridgePartyCntlCOP*) pPartyCntl)->IsPartyCascadeLinkSupportAsymmetricEMCascade())
      {
        PTRACE2(eLevelInfoNormal, "LECTURE_MODE:ASYMMETRIC_CASCADE: CVideoBridgeCOP::ApplicationActionsOnRemovePartyFromMix: EndLectureMode - the old lecturer is Cascade Link that supports asymmetric cascade, ", m_pConfName);
        m_pCopLectureModeCntl->SetIsDisconnectedCascadeLecturerParty(YES);
        DoLectureModeBaseAction(eLmAction_LecturerDisonnect_CascadeLecturer, *pNewVideoBridgeLectureMode);
      }
      else
        DoLectureModeBaseAction(eLmAction_LecturerDisonnect, *pNewVideoBridgeLectureMode);
    }
    else
    {
      CVideoBridgePartyCntl* newLecturer              = GetLecturer(*pNewVideoBridgeLectureMode);
      LectureModeAction      changeLecturerBaseAction = GetLectureModeChangeLecturerBaseAction(((CVideoBridgePartyCntlCOP*) pPartyCntl), ((CVideoBridgePartyCntlCOP*) newLecturer));
      PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::ApplicationActionsOnRemovePartyFromMix - change lecturer base action type:", changeLecturerBaseAction);
      if (changeLecturerBaseAction == eLmAction_ChangeLecturerToAndFromCascadeLecturer || changeLecturerBaseAction == eLmAction_ChangeLecturerFromCascadeLecturer)
        m_pCopLectureModeCntl->SetIsDisconnectedCascadeLecturerParty(YES);

      DoLectureModeBaseAction(changeLecturerBaseAction, *pNewVideoBridgeLectureMode);
    }

    POBJDELETE(pNewVideoBridgeLectureMode);
  }
  StartAutoLayout();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::ApplicationActionsOnAddPartyToMix(CVideoBridgePartyCntl* pPartyCntl)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::ApplicationActionsOnAddPartyToMix : Name - ", m_pConfName);

  if (!IsValidPObjectPtr(pPartyCntl)) {
    PASSERT_AND_RETURN(1);
  }

  if (m_pLectureModeParams->GetLectureModeType() == eLectureModeRegular)
  {
    CVideoBridgeLectureModeParams* pNewVideoBridgeLectureMode = new CVideoBridgeLectureModeParams;
    *pNewVideoBridgeLectureMode = *m_pLectureModeParams;

    // check if the new party is the lecturer in DB
    if (pNewVideoBridgeLectureMode->IsAudioActivatedLectureMode())
    {
      if (IsLecturerInDB(pPartyCntl->GetName())) // if we got here pPartyCntl should be valid
      {
        PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::ApplicationActionsOnAddPartyToMix: Defined lecturer Added to mix (set audio activate to false and start lecture mode), Name - ", m_pConfName);
        CCommConf*          pCommConf             = (CCommConf*)m_pConf->GetCommConf();
        CLectureModeParams* pApiLectureModeParams = pCommConf->GetLectureMode();
        const char*         DBLecturerName        = pApiLectureModeParams->GetLecturerName();
        pNewVideoBridgeLectureMode->SetLecturerName(DBLecturerName);
        pNewVideoBridgeLectureMode->SetAudioActivatedLectureMode(eAudioActivatedFalse);
        if (m_pCopLectureModeCntl->IsLectureModeActive())
        {
          // we are already in lecture mode - change lecturer
          CVideoBridgePartyCntl* oldLecturer              = GetLecturer();
          LectureModeAction      changeLecturerBaseAction = GetLectureModeChangeLecturerBaseAction(((CVideoBridgePartyCntlCOP*) oldLecturer), ((CVideoBridgePartyCntlCOP*) pPartyCntl));
          PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::ApplicationActionsOnAddPartyToMix - change lecturer base action type:", changeLecturerBaseAction);
          DoLectureModeBaseAction(changeLecturerBaseAction, *pNewVideoBridgeLectureMode);
        }
        else
        {
          PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::ApplicationActionsOnAddPartyToMix : ELSE IsAudioActivatedLectureMode() - ", pNewVideoBridgeLectureMode->IsAudioActivatedLectureMode());

          // first lecturer - start lecture mode
          if (IsValidPObjectPtr(pPartyCntl) && ((CVideoBridgePartyCntlCOP*) pPartyCntl)->IsPartyCascadeLinkSupportAsymmetricEMCascade())
          {
            PTRACE2(eLevelInfoNormal, "LECTURE_MODE:ASYMMETRIC_CASCADE: CVideoBridgeCOP::ApplicationActionsOnAddPartyToMix: Start lecture mode - the new lecturer is Cascade Link that supports asymmetric cascade, ", m_pConfName);
            DoLectureModeBaseAction(eLmAction_LecturerConnect_CascadeLecturer, *pNewVideoBridgeLectureMode);
          }
          else
            DoLectureModeBaseAction(eLmAction_LecturerConnect, *pNewVideoBridgeLectureMode);
        }
      }
    }
    else if (IsLecturer(pPartyCntl))
    {
      PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::ApplicationActionsOnAddPartyToMix: lecturer Added to mix, Name - ", m_pConfName);
      if (m_pCopLectureModeCntl->IsLectureModeActive())
      {
        CVideoBridgePartyCntl* oldLecturer              = GetLecturer();
        LectureModeAction      changeLecturerBaseAction = GetLectureModeChangeLecturerBaseAction(((CVideoBridgePartyCntlCOP*) oldLecturer), ((CVideoBridgePartyCntlCOP*) pPartyCntl));
        PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::ApplicationActionsOnAddPartyToMix - lecturer Added to mix change lecturer base action type:", changeLecturerBaseAction);
        DoLectureModeBaseAction(changeLecturerBaseAction, *pNewVideoBridgeLectureMode);
      }
      else
      {
        if (IsValidPObjectPtr(pPartyCntl) && ((CVideoBridgePartyCntlCOP*) pPartyCntl)->IsPartyCascadeLinkSupportAsymmetricEMCascade())
        {
          PTRACE2(eLevelInfoNormal, "LECTURE_MODE:ASYMMETRIC_CASCADE: CVideoBridgeCOP::ApplicationActionsOnAddPartyToMix: lecturer Added to mix- the new lecturer is Cascade Link that supports asymmetric cascade, ", m_pConfName);
          DoLectureModeBaseAction(eLmAction_LecturerConnect_CascadeLecturer, *pNewVideoBridgeLectureMode);
        }
        else
          DoLectureModeBaseAction(eLmAction_LecturerConnect, *pNewVideoBridgeLectureMode);
      }
    }

    POBJDELETE(pNewVideoBridgeLectureMode);
  }
  else if (m_pCopLectureModeCntl->IsAudioActivatedLectureMode())
  {
    CVideoBridgeLectureModeParams* pNewVideoBridgeLectureMode = new CVideoBridgeLectureModeParams;
    *pNewVideoBridgeLectureMode = *m_pLectureModeParams;
    // check if the new party is the lecturer in DB
    CCommConf*          pCommConf                             = (CCommConf*)m_pConf->GetCommConf();
    CLectureModeParams* pApiLectureModeParams                 = pCommConf->GetLectureMode();
    const char*         DBLecturerName                        = pApiLectureModeParams->GetLecturerName();

    if (IsLecturerInDB(pPartyCntl->GetName())) // if we got here pPartyCntl should be valid
    {
      PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::ApplicationActionsOnAddPartyToMix: Defined lecturer Added to mix (set audio activate to false and start lecture mode), Name - ", m_pConfName);
      if (IsValidPObjectPtr(pPartyCntl) && ((CVideoBridgePartyCntlCOP*) pPartyCntl)->IsPartyCascadeLinkSupportAsymmetricEMCascade())
      {
        pNewVideoBridgeLectureMode->SetLecturerName(DBLecturerName);
        pNewVideoBridgeLectureMode->SetAudioActivatedLectureMode(eAudioActivatedFalse);
        PTRACE2(eLevelInfoNormal, "LECTURE_MODE:ASYMMETRIC_CASCADE: CVideoBridgeCOP::ApplicationActionsOnAddPartyToMix: Start lecture mode - the new lecturer is Cascade Link that supports asymmetric cascade, ", m_pConfName);
        DoLectureModeBaseAction(eLmAction_LecturerConnect_CascadeLecturer, *pNewVideoBridgeLectureMode);
      }
    }
    else
    {
      PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::ApplicationActionsOnAddPartyToMix: not Defined lecturer in DB DBLecturerName - ", DBLecturerName);
    }

    POBJDELETE(pNewVideoBridgeLectureMode);
  }
  StartAutoLayout();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::EndCurrentLectureModeAction()
{
  LectureModeAction lecture_mode_action = m_pCopLectureModeCntl->GetCurrentLectureModeAction();
  switch (lecture_mode_action)
  {
    case eLmAction_LecturerConnect:
    {
      PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::EndCurrentLectureModeAction: EndStartLectureMode, ", m_pConfName);
      EndStartLectureMode();
      break;
    }

    case eLmAction_LecturerDisonnect:
    {
      PTRACE2(eLevelInfoNormal, "LECTURE_MODE: LECTURE_MODE: CVideoBridgeCOP::EndCurrentLectureModeAction: EndEndLectureMode, ", m_pConfName);
      EndEndLectureMode(YES);
      break;
    }

    case eLmAction_ChangeLecturer:
    {
      PTRACE2(eLevelInfoNormal, "LECTURE_MODE: LECTURE_MODE: LECTURE_MODE: CVideoBridgeCOP::EndCurrentLectureModeAction: EndChangeLecturer, ", m_pConfName);
      EndChangeLecturer(YES);
      break;
    }

    case eLmAction_LecturerConnect_CascadeLecturer:
    {
      PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::EndCurrentLectureModeAction: EndCurrentLectureModeActionForConnectCascadeLecturer, ", m_pConfName);
      EndCurrentLectureModeActionForCascadeLecturer(lecture_mode_action);
      break;
    }

    case eLmAction_LecturerDisonnect_CascadeLecturer:
    {
      PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::EndCurrentLectureModeAction: EndEndCascadeLecturerMode, ", m_pConfName);
      EndEndCascadeLecturerMode();
      break;
    }

    case eLmAction_ChangeLecturerToCascadeLecturer:
    {
      PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::EndCurrentLectureModeAction: eLmAction_ChangeLecturerToCascadeLecturer EndCurrentLectureModeActionForChangeLecturerOfCascadeLecturer, ", m_pConfName);
      EndCurrentLectureModeActionForCascadeLecturer(lecture_mode_action);  break;
      break;
    }

    case eLmAction_ChangeLecturerFromCascadeLecturer:
    {
      PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::EndCurrentLectureModeAction: eLmAction_ChangeLecturerFromCascadeLecturer EndCurrentLectureModeActionForChangeLecturerOfCascadeLecturer, ", m_pConfName);
      EndCurrentLectureModeActionForCascadeLecturer(lecture_mode_action);
      break;
    }

    case eLmAction_ChangeLecturerToAndFromCascadeLecturer:
    {
      PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::EndCurrentLectureModeAction: eLmAction_ChangeLecturerToAndFromCascadeLecturer EndCurrentLectureModeActionForChangeLecturerOfCascadeLecturer, ", m_pConfName);
      EndCurrentLectureModeActionForCascadeLecturer(lecture_mode_action);
      break;
    }


    default:
    {
      PTRACE2INT(eLevelInfoNormal, "LECTURE_MODE: LECTURE_MODE: LECTURE_MODE: CVideoBridgeCOP::EndCurrentLectureModeAction: do nothing, lecture_mode_action = ", lecture_mode_action);
      break;
    }
  } // switch
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::UpdateCopEncoderPartiesLayoutInDB(CLayout* layout, DWORD levelEncoderEntityId)
{
	if (IsValidPObjectPtr(layout))
	{
		CBridgePartyList::iterator _end = m_pPartyList->end();
		for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
		{
			// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
			CVideoBridgePartyCntlCOP* pPartyCntl = (CVideoBridgePartyCntlCOP*)_itr->second;
			if (pPartyCntl)
			{
				DWORD partyLevelEncoderEntityId = pPartyCntl->GetCopEncoderEntityId();
				if (partyLevelEncoderEntityId == levelEncoderEntityId)
				{
					// update DB for party
					CSegment vidLayoutSeg;
					DWORD    isUpdateLayoutAsPrivateInDB = NO;
					if (!(layout->Serialize(PARTY_lev, &vidLayoutSeg)))
					{
						CTaskApp* pParty = pPartyCntl->GetPartyTaskApp();
						m_pConfApi->UpdateDB(pParty, CPPARTYLAYOUT, isUpdateLayoutAsPrivateInDB, 0, &vidLayoutSeg);
					}
					else
						DBGPASSERT(1);
				}
			}
		}
	}
	else
		PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::UpdateCopEncoderPartiesLayoutInDB layout is not valid!!!!", levelEncoderEntityId);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::UpdateCopPartyLayoutInDB(CLayout* layout, CVideoBridgePartyCntlCOP* pPartyCntl)
{
  if (IsValidPObjectPtr(layout))
  {
    CSegment vidLayoutSeg;
    DWORD    isUpdateLayoutAsPrivateInDB = NO;
    if (!(layout->Serialize(PARTY_lev, &vidLayoutSeg)))
    {
      CTaskApp* pParty = pPartyCntl->GetPartyTaskApp();
      m_pConfApi->UpdateDB(pParty, CPPARTYLAYOUT, isUpdateLayoutAsPrivateInDB, 0, &vidLayoutSeg);
    }
  }
  else
    PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::UpdateCopPartyLayoutInDB layout is not valid!!!!");
}

////////////////////////////////////////////////////////////////////////////
BYTE CVideoBridgeCOP::IsLecturerReady()
{
  BYTE isLecturerReady = YES;

  if (m_pLectureModeParams->GetLecturerName()[0] == '\0')
  {
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::IsLecturerReady: invalid lecturer`s name (audio activated and no active speaker(?)-->only update DB), Name - ", m_pConfName);
    isLecturerReady = NO;
    return isLecturerReady;
  }

  // this function begins lecture mode
  CVideoBridgePartyCntlCOP* pLecturer = (CVideoBridgePartyCntlCOP*)(GetLecturer());
  if (pLecturer == NULL)
  {
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::IsLecturerReady: lecturer not found (not connected to bridge-->only update DB), Name - ", m_pConfName);
    isLecturerReady = NO;
    return isLecturerReady;
  }

  isLecturerReady = pLecturer->IsReadyToStartLecturer();
  return isLecturerReady;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::DoPendingLectureModeActionOnEndOfChangeLayout()
{
  BYTE wait = m_pCopLectureModeCntl->GetWaitingToEndChangeLayout();

  if (wait == WAIT_API)
  {
    // do lecture mode action - that was delayed from INSWITCH state
    PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::DoPendingLectureModeActionOnEndOfChangeLayout WAIT_API, ConfName - ", m_pConfName);
    // get lecture mode params from DB
    CSegment*           pParamSeg              = new CSegment;
    CLectureModeParams* pResrvationLectureMode = new CLectureModeParams;

    m_pCopLectureModeCntl->SetWaitingToEndChangeLayout(NO);

    // do lecture mode action
    *pResrvationLectureMode = *(m_pCopLectureModeCntl->GetWaitingToEndChangeLayoutParams());
    pResrvationLectureMode->Serialize(NATIVE, *pParamSeg);
    OnConfSetLectureModeCONNECTED(pParamSeg);

    POBJDELETE(pParamSeg);
    POBJDELETE(pResrvationLectureMode);
  }
  else if (wait == WAIT_BRIDGE)
  {
    // do lecture mode action - that was delayed from INSWITCH state
    PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::DoPendingLectureModeActionOnEndOfChangeLayout WAIT_BRIDGE, ConfName - ", m_pConfName);

    m_pCopLectureModeCntl->SetWaitingToEndChangeLayout(NO);

    if (m_pCopLectureModeCntl->IsPendingAction())
    {
      PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::DoPendingLectureModeActionOnEndOfChangeLayout: Executing pending action, ", m_pConfName);
      m_pCopLectureModeCntl->Dump();
      LectureModeAction             pendingAction = m_pCopLectureModeCntl->GetPendingLectureModeAction();
      CVideoBridgeLectureModeParams pendingParams = *(m_pCopLectureModeCntl->GetPendingParams());
      m_pCopLectureModeCntl->UpdateCurrentActionFromPending();
      DoLectureModeBaseAction(pendingAction, pendingParams);
    }
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::HandleFlowControlRequestForVSWEncoder(CVideoBridgePartyCntlCOP* pFlowCntlInitiatorPartyCntl, DWORD newBitRate, CLPRParams* pLprParams)
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::HandleFlowControlRequestForVSWEncoder");

  DWORD encoderFlowControlRate   = 0;
  DWORD encoderOriginalVideoRate = 0;
  DWORD levelEncoderEntityId     = pFlowCntlInitiatorPartyCntl->GetCopEncoderEntityId();

  encoderFlowControlRate   = m_pCopEncoderVsw->GetEncoderFlowControlRate();
  encoderOriginalVideoRate = m_pCopEncoderVsw->GetOutVideoRate();

  if (encoderOriginalVideoRate < newBitRate)
  {
    CMedString mstr;
    mstr << "\nEncoder video rate = " << encoderOriginalVideoRate;
    mstr << ", Party's flow control request rate = "<< newBitRate;
    FPTRACE2(eLevelError, "CVideoBridgeCOP::HandleFlowControlRequestForVSWEncoder invalid - the requested flow control rate is bigger than the encoder video rate we will ignore request:", mstr.GetString());
    return;
  }

  ((CVideoBridgePartyCntlCOP*)pFlowCntlInitiatorPartyCntl)->SetPartyFlowControlRate(newBitRate);

  // in case the party is connected to the PCM encoder
  if (pFlowCntlInitiatorPartyCntl->IsConnectedOrConnectingPCM())
  {
    PTRACE2(eLevelError, "CVideoBridgeCOP::HandleFlowControlRequestForVSWEncoder Party is connected to PCM, Name - ", m_pConfName);
    // In case of PCM we will update the PCM encoder with the party's request
    DWORD pcmEncoderFlowControlRate = m_pPCMEncoderCntl->GetEncoderFlowControlRate();
    if (pcmEncoderFlowControlRate != newBitRate) // ONLY ONE PARTY CONNECTED TO THE PCM
    {
      pcmEncoderFlowControlRate = newBitRate;
      PTRACE2INT(eLevelError, "CVideoBridgeCOP::HandleFlowControlRequestForVSWEncoder Party is connected to PCM. PCM encoder new FlowControl Rate =", newBitRate);
      m_pPCMEncoderCntl->UpdateEncoderFlowControlRate(pcmEncoderFlowControlRate);
    }

    if (pLprParams != NULL)
      pFlowCntlInitiatorPartyCntl->ForwardFlowControlCommand(pcmEncoderFlowControlRate, pLprParams);
    return;
  }

  DWORD newFlowControlRate = FindLowestFlowControlRateForLevelEncoder(levelEncoderEntityId);
  if (encoderFlowControlRate != newFlowControlRate)
  {
    CMedString mstr;
    mstr << "VSW Encoder Index,  ";
    mstr << ", New Flow Control Rate = "<< newFlowControlRate;
    FPTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::HandleFlowControlRequestForVSWEncoder: ", mstr.GetString());

    // In case of VSW the update encoder flow control rate will just be saved and not cause for update encoder request
    m_pCopEncoderVsw->UpdateEncoderFlowControlRate(newFlowControlRate);

    // Send Request to the Lecturer to transmit rate according to the new flow control request
    CVideoBridgePartyCntl* pLecturer = GetLecturer();
    if (IsValidPObjectPtr(pLecturer))
    {
      ((CVideoBridgePartyCntlCOP*)pLecturer)->ForwardFlowControlCommand(newFlowControlRate, NULL /*pLprParams*/, FALSE);
    }

    if (pLprParams != NULL)
      pFlowCntlInitiatorPartyCntl->ForwardFlowControlCommand(newFlowControlRate, pLprParams);
  }
  else
  {
    if (pLprParams != NULL)
    {
      PTRACE2(eLevelError, "CVideoBridgeCOP::HandleFlowControlRequestForVSWEncoder do not update encoder rate but update party with LPR params, Name - ", m_pConfName);
      pFlowCntlInitiatorPartyCntl->ForwardFlowControlCommand(newFlowControlRate, pLprParams);
    }
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::UpdateFlowControlRateWhenMovingParticipantsToAndFromVSWEncoder(BYTE toVsw)
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::UpdateFlowControlRateWhenMovingParticipantsToAndFromVSWEncoder");

  if (toVsw)
  {
    // moved all the listenters to the VSW encoder only the lecturer left on its encoder, update the encoder accordingly
    CVideoBridgePartyCntl* pLecturer = GetLecturer();
    if (IsValidPObjectPtr(pLecturer))
    {
      DWORD lecturerFCRate         = ((CVideoBridgePartyCntlCOP*)pLecturer)->GetPartyFlowControlRate();
      DWORD encoderFlowControlRate = m_pCopLevelEncoders[m_lecturerlevelEncoderIndex]->GetEncoderFlowControlRate();
      if (lecturerFCRate != encoderFlowControlRate)
      {
        PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::UpdateFlowControlRateWhenMovingParticipantsToAndFromVSWEncoder, update the lecturer encoder with the update flow control rate: ", lecturerFCRate);
        m_pCopLevelEncoders[m_lecturerlevelEncoderIndex]->UpdateEncoderFlowControlRate(lecturerFCRate);
      }
    }
  }
  else
  {
    // moved all the participants from the vsw encoder to level encoder, we will reset the VSW encoder FC rate
    // update the lecturer to stop the flow control constraint
    PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::UpdateFlowControlRateWhenMovingParticipantsToAndFromVSWEncoder reset the vsw encoder flow control");
    if (IsValidPObjectPtr(m_pCopEncoderVsw))
      m_pCopEncoderVsw->UpdateEncoderFlowControlRate(0);

    ResetLecturerFlowControlRate();
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::ResetLecturerFlowControlRate()
{
  CVideoBridgePartyCntl* pLecturer = GetLecturer();
  if (IsValidPObjectPtr(pLecturer))
  {
    PTRACE2(eLevelError, "CVideoBridgeCOP::ResetLecturerFlowControlRate send request to the lecturer with reset flow control rate, Name - ", pLecturer->GetName());
    ((CVideoBridgePartyCntlCOP*)pLecturer)->ForwardFlowControlCommand(0, NULL, FALSE);
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfLecturerDecoderTimerCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "LECTURE_MODE: OnConfLecturerDecoderTimerCONNECTED, ", m_pConfName);
  SendChangeLayoutOnLecturerDecoderSync();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfLecturerDecoderTimerINSWITCH(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "LECTURE_MODE: OnConfLecturerDecoderTimerINSWITCH, ", m_pConfName);
  // will send change layout at the end of switch
  m_pCopLectureModeCntl->SetWaitingToLecturerDecoderSync(NO);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::GetCroppingValues(DWORD& croppingHorr, DWORD& croppingVer)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::GetCroppingValues", m_pConfName);

  croppingHorr = 50;
  croppingVer  = 50;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::AdjustRequiredResolotion(ECopDecoderResolution& newRequiredResolution, ECopDecoderResolution newPartyMaxDecoderResolution, DWORD newPartyAlgorithm, CObjString& cstr, DWORD newArtId, ECopDecoderResolution copDecoderResolutionFromEncoder, BYTE isNeedToConsiderOutRes)
{
  // In MPMX 4CIF/W4CIF decoder can support up to CIF resolution in H263
  AdjustRequiredResolotionToMaxH263Resolution(newRequiredResolution, newPartyAlgorithm, newArtId);

  // required res = min (newRequiredResolution, newPartyInitialDecoderResolution)
  // incase party caps are smaller then required resolution
  // --> set required resolution according to party caps
  if (newRequiredResolution < newPartyMaxDecoderResolution)
  {
    newRequiredResolution = newPartyMaxDecoderResolution;
    PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::AdjustRequiredResolotion newRequiredResolution before required resolution has been updated to party initial", newRequiredResolution);
  }

  // Only when the max level is HD720@50 we will ask 50fps
  AdjustRequiredResolotionToConfMaxResolution(newRequiredResolution);

  if ((copDecoderResolutionFromEncoder != COP_decoder_resolution_Last) && isNeedToConsiderOutRes)
  {
    if (newRequiredResolution < copDecoderResolutionFromEncoder) // the required is greater than the out resolution
    {
      PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::AdjustRequiredResolotion newRequiredResolution: ", newRequiredResolution);
      PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::AdjustRequiredResolotion copDecoderResolutionFromEncoder: ", copDecoderResolutionFromEncoder);
      newRequiredResolution = copDecoderResolutionFromEncoder;
    }
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::AdjustRequiredResolotionToMaxH263Resolution(ECopDecoderResolution& newRequiredResolution, DWORD& newPartyAlgorithm, DWORD newArtId)
{
  if ((IsFeatureSupportedBySystem(eFeatureCOP)) && ((newRequiredResolution == COP_decoder_resolution_4CIF25) || (newRequiredResolution == COP_decoder_resolution_W4CIF25)) && (H263 == newPartyAlgorithm))
  {
    PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::AdjustRequiredResolotionToMaxH263Resolution In MPMX the 4CIF/W4CIF decoder can decoder up to CIF in H263 : ", newArtId);
    newRequiredResolution =  COP_decoder_resolution_CIF25;
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::AdjustRequiredResolotionToConfMaxResolution(ECopDecoderResolution& newRequiredResolution)
{
  if (newRequiredResolution == COP_decoder_resolution_HD720p50 || newRequiredResolution == COP_decoder_resolution_4CIF50)
  {
    ECopEncoderMaxResolution encoder_max_resolution = GetCopEncoderMaxResolution();
    if (encoder_max_resolution != COP_encoder_max_resolution_HD720p50)
    {
      if (newRequiredResolution == COP_decoder_resolution_HD720p50)
      {
        PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::AdjustRequiredResolotionToMaxResolutio this isn't HD720@50 conference the required resolution will be updated to HD720@25 ", m_pConfName);
        newRequiredResolution = COP_decoder_resolution_HD720p25;
      }
      else
      {
        PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::AdjustRequiredResolotionToMaxResolutio this isn't HD720@50 conference the required resolution will be updated to 4CIF@25 ", m_pConfName);
        newRequiredResolution = COP_decoder_resolution_4CIF25;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////
BYTE CVideoBridgeCOP::IsDSPSmartSwitchNeeded(CLayout oldLayout)
{
	BOOL bDSPSmartSwitchSysFlagOn = YES;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("IS_COP_DSP_SMART_SWITCH_ON", bDSPSmartSwitchSysFlagOn);

	if (IsFeatureSupportedBySystem(eFeatureCOP) && bDSPSmartSwitchSysFlagOn)
	{
		CPartyImageLookupTable* pPartyImageLookupTable = GetPartyImageLookupTable();
		for (int i = 0; i < NUM_OF_COP_DECODERS; ++i)
		{
			eChangeLayoutActionsStatus actionStatusForChangeMode = m_changeLayoutActions->GetChangeLayoutActionStatusForDecoder(i, eCopDecoderAction_ReCapToParty);
			if (actionStatusForChangeMode == eActionNeeded)
			{
				DWORD artPartyId   = m_changeLayoutActions->GetChangeLayoutActionArtIdForDecoder(i, eCopDecoderAction_ReCapToParty);
				WORD  numOldSubImg = oldLayout.GetNumberOfSubImages();
				for (WORD j = 0; j < numOldSubImg; ++j)
				{
					CVidSubImage* pVidSubImage = oldLayout[j];
					if (pVidSubImage)
					{
						DWORD partyRscId = pVidSubImage->GetImageId();
						if (partyRscId)
						{
							CImage* pImage = pPartyImageLookupTable->GetPartyImage(partyRscId);
							PASSERTSTREAM(!pImage, "CVideoBridgeCOP::IsDSPSmartSwitchNeeded - Failed, The lookup table doesn't have an element, PartyId:" << partyRscId);

							if (pImage && pImage->GetArtPartyId() == artPartyId)
								return YES;
						}
					}
				}
			}
		}
	}

	return NO;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::StartDSPSmartSwitch()
{
  // 1. Start timer of 150 ms
  StartTimer(COP_DSP_SMART_SWITCH_TIME_OUT, COP_DSP_SMART_SWITCH_TIME_VALUE);
  m_IsDSPSmartSwitchTimerSet = TRUE;

  // 2. Send DSPSmartSwichChangeLayout request with the new layout
  SendDSPSmartSwitchChangeLayoutToAllLevelEncoders();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::UpdateDSPSmartSwitchImagesParam(CLayout& newLayout, CLayout& oldLayout)
{
	// to go over all the sub images and if the art party id wasn't in the old layout set the ids with DSP_SMART_SWITCH_DUMMY_ID
	WORD numSubImg = newLayout.GetNumberOfSubImages();
	for (WORD i = 0; i < numSubImg; i++)
	{
		CVidSubImage* pVidSubImage = newLayout[i];
		if (pVidSubImage)
		{
			DWORD partyRscId = pVidSubImage->GetImageId();
			if (partyRscId)
			{
				CImage* pImage = GetPartyImageLookupTable()->GetPartyImage(partyRscId);
				PASSERTSTREAM(!pImage, "CVideoBridgeCOP::UpdateDSPSmartSwitchImagesParam - Failed, The lookup table doesn't have an element, PartyId:" << partyRscId);

				if (pImage)
				{
					DWORD artPartyId = pImage->GetArtPartyId();

					// try to find in old layout
					WORD numOldSubImg        = oldLayout.GetNumberOfSubImages();
					BYTE partyWasInOldLayout = NO;
					for (WORD j = 0; j < numOldSubImg && !partyWasInOldLayout; j++)
					{
						CVidSubImage* pOldVidSubImage = oldLayout[j];
						if (pOldVidSubImage)
						{
							DWORD partyRscId = pOldVidSubImage->GetImageId();
							if (partyRscId)
							{
								CImage* pOldImage = GetPartyImageLookupTable()->GetPartyImage(partyRscId);
								PASSERTSTREAM(!pImage, "CVideoBridgeCOP::UpdateDSPSmartSwitchImagesParam - Failed, The lookup table doesn't have an element, PartyId:" << partyRscId);

								if (pOldImage && pOldImage->GetArtPartyId() == artPartyId)
									partyWasInOldLayout = YES;
							}
						}
					}

					if (!partyWasInOldLayout)
					{
						TRACEINTO << "CVideoBridgeCOP::UpdateDSPSmartSwitchImagesParam - The party wasn't in old layout, so we will update the Id's with DSP_SMART_SWITCH_DUMMY_ID, ArtPartyId:" << artPartyId;

						CVideoBridgePartyCntlCOP* pCurrParty = (CVideoBridgePartyCntlCOP*)m_pPartyList->Find(artPartyId);
						if (pCurrParty)
						{
							// for the MPMX DSP smart switch solution
							pCurrParty->SetDspSmartSwitchConnectionId(DSP_SMART_SWITCH_DUMMY_ID);
							pCurrParty->SetDspSmartSwitchEntityId(DSP_SMART_SWITCH_DUMMY_ID);
						}
					}
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::SendDSPSmartSwitchChangeLayoutToAllLevelEncoders()
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::SendDSPSmartSwitchChangeLayoutToAllLevelEncoders");

  if (!IsActiveLectureMode())
  {
    // send the new layout to all encoders
    for (WORD encoder_index = 0; encoder_index < NUM_OF_LEVEL_ENCODERS; encoder_index++)
    {
      m_pCopLevelEncoders[encoder_index]->DSPSmartSwitchChangeLayout(m_ConfLayout, m_pVisualEffects, m_pSiteNameInfo, GetAudioSpeakerPlaceInLayout(), m_SendChangeLayoutAnyWay);
    }

    if (m_pcmConnected)
      m_pPCMEncoderCntl->DSPSmartSwitchChangeLayout(m_ConfLayout, m_pVisualEffects, NULL, GetAudioSpeakerPlaceInLayout(), m_SendChangeLayoutAnyWay);
  }
  else // active lecture mode send the dsp smart switch only to lecturer
  {
    PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::SendDSPSmartSwitchChangeLayoutToAllLevelEncoders lecture mode active");
    for (WORD encoder_index = 0; encoder_index < NUM_OF_LEVEL_ENCODERS; encoder_index++)
    {
      DWORD levelEncoderEntityId = m_pCopLevelEncoders[encoder_index]->GetPartyRsrcID();
      if (encoder_index == m_lecturerlevelEncoderIndex)
      {
        if (IsOnlyLecturerConnected())
        {
          PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::SendDSPSmartSwitchChangeLayoutToAllLevelEncoders only lecturer connected we wont send the DSPSmartSwitchChangeLayout, encoder_index = ", encoder_index);
        }
        else
        {
          PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::SendDSPSmartSwitchChangeLayoutToAllLevelEncoders send change layout to lecturer, encoder_index = ", encoder_index);
          // send conf layout to lecturer encoder
          m_pCopLevelEncoders[encoder_index]->DSPSmartSwitchChangeLayout(m_ConfLayout, m_pVisualEffects, m_pSiteNameInfo, GetAudioSpeakerPlaceInLayout(), m_SendChangeLayoutAnyWay);
        }
      }
    }

    if (m_pcmConnected)
    {
      if (!IsLecturerConnectedToPCM() || IsOnlyLecturerConnected())
        PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::SendDSPSmartSwitchChangeLayoutToAllLevelEncoders only lecturer connected or not listener connected to PCM we wont send the DSPSmartSwitchChangeLayout");
      else
        m_pPCMEncoderCntl->DSPSmartSwitchChangeLayout(m_ConfLayout, m_pVisualEffects, NULL, GetAudioSpeakerPlaceInLayout(), m_SendChangeLayoutAnyWay);
    }
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnTimerEndDSPSmartSwitchINSWITCH(CSegment* pParam)
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::OnTimerEndDSPSmartSwitchINSWITCH");
  m_IsDSPSmartSwitchTimerSet = FALSE;
  m_IsDSPSmartSwitch         = FALSE;
  StartSwitch();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::StartSwitchTimer()
{
  DWORD swithTout = SWITCH_TIME_OUT_COP_VALUE;
  if (m_IsSmartSwitch)
  {
    DWORD smartSwitchTout = GetSmartSwitchTout();
    if (SWITCH_TIME_OUT_COP_VALUE <= smartSwitchTout)
    {
      swithTout = smartSwitchTout + 1*SECOND;
    }
  }

  if (m_IsDSPSmartSwitch)
    swithTout = swithTout + COP_DSP_SMART_SWITCH_TIME_VALUE;

  PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::StartSwitchTimer - updated TOUT = ", swithTout);
  StartTimer(SWITCH_TOUT, swithTout);
}

////////////////////////////////////////////////////////////////////////////
BYTE CVideoBridgeCOP::IsRemoteNeedSmartSwitchAccordingToVendor(DWORD artPartyId)
{
  BYTE isPartyneedSmartSwitchAccordingToVendor = NO;
  CVideoBridgePartyCntlCOP* pPartyCntl = (CVideoBridgePartyCntlCOP*)GetPartyCntl(artPartyId);
  if (IsValidPObjectPtr(pPartyCntl))
  {
    isPartyneedSmartSwitchAccordingToVendor = pPartyCntl->IsRemoteNeedSmartSwitchAccordingToVendor();
    PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::IsPartyn  = ", isPartyneedSmartSwitchAccordingToVendor);
  }
  return isPartyneedSmartSwitchAccordingToVendor;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::SetIsRemoteNeedSmartSwitchAccordingToVendor(CSegment* pParam)
{
	PTRACE2(eLevelError, "CVideoBridgeCOP::SetIsRemoteNeedSmartSwitchAccordingToVendor Name - ", m_pConfName);
	CParty* pParty = NULL;
	*pParam >> (DWORD&)pParty;
	if (IsValidPObjectPtr(pParty))
	{
		CVideoBridgePartyCntlCOP* pPartyCntl = (CVideoBridgePartyCntlCOP*)GetPartyCntl(pParty->GetPartyRsrcID());
		if (pPartyCntl)
		{
			pPartyCntl->SetIsRemoteNeedSmartSwitchAccordingToVendor(YES);
			return;
		}
	}
	FPTRACE(eLevelError, "CVideoBridgeCOP::SetIsRemoteNeedSmartSwitchAccordingToVendor - invalid bridge party control");
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnTimerDelayDecoderSyncINSWITCH(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnTimerDelayDecoderSyncINSWITCH, timer DELAY_DECODER_SYNC_TIMER jumped, ConfName - ", m_pConfName);
  // check if the first stage actions of OPEN CONNECT and SYNC finished
  BOOL isOpenStageFinished  = AreAllOpenConnectSyncDecActionsCompleted();
  // Bug VNGR-18504 fix we received that a decoder was synced and we didnt verify that there are still close actions
  BOOL isCloseStageFinished = AreAllCloseDisconnnectChangeModeDecActionsCompleted();
  // if it finished we can continue to the second stage
  if (isOpenStageFinished && isCloseStageFinished)
  {
    EndChangeLayoutActions();
  }
  else
  {
    if (isOpenStageFinished)
      PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::OnTimerDelayDecoderSyncINSWITCH there are still open connect actions");
    else
      PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::OnTimerDelayDecoderSyncINSWITCH there are still close disconnect actions ");
  }
}

////////////////////////////////////////////////////////////////////////////
ECopDecoderResolution CVideoBridgeCOP::GetDecoderResolutionFromEncoder(CVideoBridgePartyCntlCOP* partyCntl)
{
  DWORD levelEncoderEntityId                 = partyCntl->GetCopEncoderEntityId();
  WORD  levelEncoderIndex                    = GetCopEncoderIndexFromEntityId(levelEncoderEntityId);
  eVideoFrameRate       videoFrameRate       = eVideoFrameRateDUMMY;
  ECopDecoderResolution copDecoderResolution = COP_decoder_resolution_Last;

  if (levelEncoderIndex <= NUM_OF_LEVEL_ENCODERS-1)
  {
    copDecoderResolution = m_pCopLevelEncoders[levelEncoderIndex]->GetCopDecoderResolutionFromVideoParams();
    videoFrameRate       = m_pCopLevelEncoders[levelEncoderIndex]->GetVidFrameRate();
    PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::GetDecoderResolutionFromEncoder,", copDecoderResolution);
  }
  else if (levelEncoderIndex == VSW_ENCODER_INDEX)
  {
    copDecoderResolution = m_pCopEncoderVsw->GetCopDecoderResolutionFromVideoParams();
    videoFrameRate       = m_pCopEncoderVsw->GetVidFrameRate();
    PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::GetDecoderResolutionFromEncoder vsw encoder, ", copDecoderResolution);
  }
  return copDecoderResolution;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::EventModeIntraPreviewReq(PartyRsrcID partyId)
{
	CSegment seg;
	seg << partyId;
	DispatchEvent(REQUEST_PREVIEW_INTRA, &seg);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfContentBridgeStopPresentationCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CCVideoBridgeCOP::OnConfContentBridgeStopPresentationCONNECTED : ConfName - ", m_pConfName);
  IgnoreIntraFilteringAfterStopContent();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfContentBridgeStopPresentationINSWITCH(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfContentBridgeStopPresentationINSWITCH : ConfName - ", m_pConfName);
  IgnoreIntraFilteringAfterStopContent();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfContentBridgeStartPresentationCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfContentBridgeStartPresentationCONNECTED : ConfName - ", m_pConfName);
  IgnoreIntraFilteringAfterStartContent();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfContentBridgeStartPresentationINSWITCH(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfContentBridgeStartPresentationINSWITCH : ConfName - ", m_pConfName);
  IgnoreIntraFilteringAfterStartContent();
}

////////////////////////////////////////////////////////////////////////////
bool CVideoBridgeCOP::IsIntraSuppressEnabled(WORD intra_suppression_type) const
{
  return m_enableIntraSupress[intra_suppression_type];
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::EnableIntraSuppress(WORD intra_suppression_type)
{
	if (SUPPRESS_TYPE_ALL == intra_suppression_type)
	{
		for (int j = 0; j < NUM_OF_INTRA_SUPPRESS_TYPES; j++)
		{
			m_enableIntraSupress[j] = true;
		}
	}
	else
	{
		m_enableIntraSupress[intra_suppression_type] = true;
	}

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
			pPartyCntl->EnableIntraSuppress(SUPPRESS_TYPE_ALL);
	}

	for (DWORD encoder_index = 0; encoder_index < NUM_OF_LEVEL_ENCODERS; encoder_index++)
		m_pCopLevelEncoders[encoder_index]->EnableIntraSuppress(SUPPRESS_TYPE_ALL);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::DisableIntraSuppress(WORD intra_suppression_type)
{
	if (SUPPRESS_TYPE_ALL == intra_suppression_type)
	{
		for (int j = 0; j < NUM_OF_INTRA_SUPPRESS_TYPES; j++)
		{
			m_enableIntraSupress[j] = false;
		}
	}
	else
	{
		m_enableIntraSupress[intra_suppression_type] = false;
	}

	// inform all party controls
	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
			pPartyCntl->DisableIntraSuppress(SUPPRESS_TYPE_ALL);
	}

	for (DWORD encoder_index = 0; encoder_index < NUM_OF_LEVEL_ENCODERS; encoder_index++)
		m_pCopLevelEncoders[encoder_index]->DisableIntraSuppress(SUPPRESS_TYPE_ALL);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::IgnoreIntraFilteringAfterStopContent()
{
  CSmallString sstr;
  sstr << "StartTimer DISABLE_INTRA_SUPPRESS_AFTER_STOP_CONTENT_TIMER for " << DISABLE_INTRA_SUPPRESS_AFTER_STOP_CONTENT_TOUT/SECOND << " seconds , conf = " << m_pConfName;
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::IgnoreIntraFilteringAfterStopContent: ConfName - ", sstr.GetString());
  DisableIntraSuppress(SUPPRESS_TYPE_ALL);
  StartTimer(DISABLE_INTRA_SUPPRESS_AFTER_STOP_CONTENT_TIMER, DISABLE_INTRA_SUPPRESS_AFTER_STOP_CONTENT_TOUT);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::IgnoreIntraFilteringAfterStartContent()
{
  CSmallString sstr;
  sstr << "StartTimer DISABLE_INTRA_SUPPRESS_AFTER_START_CONTENT_TIMER for " << DISABLE_INTRA_SUPPRESS_AFTER_STOP_CONTENT_TOUT/SECOND << " seconds , conf = " << m_pConfName;
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::IgnoreIntraFilteringAfterStopContent: ConfName - ", sstr.GetString());
  DisableIntraSuppress(SUPPRESS_TYPE_ALL);
  // curently use same time out as stop  - DISABLE_INTRA_SUPPRESS_AFTER_STOP_CONTENT_TOUT
  StartTimer(DISABLE_INTRA_SUPPRESS_AFTER_START_CONTENT_TIMER, DISABLE_INTRA_SUPPRESS_AFTER_STOP_CONTENT_TOUT);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnTimerDisableIntraAfterStopContent(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnTimerDisableIntraAfterStopContent : ConfName - ", m_pConfName);
  EnableIntraSuppress(SUPPRESS_TYPE_ALL);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnTimerDisableIntraAfterStartContent(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnTimerDisableIntraAfterStartContent : ConfName - ", m_pConfName);
  EnableIntraSuppress(SUPPRESS_TYPE_ALL);
}

////////////////////////////////////////////////////////////////////////////
WORD CVideoBridgeCOP::UpdateEncoderVideoParamsForAsymmetricCascade(CBridgePartyVideoOutParams& pOutVideoParams, WORD encoder_index) const
{
  PTRACE(eLevelInfoNormal, "LECTURE_MODE:ASYMMETRIC_CASCADE:CVideoBridgeCOP::UpdateEncoderVideoParamsForAsymmetricCascade");

  CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();
  if (!CPObject::IsValidPObjectPtr(pCommConf))
  {
    PASSERT(111);
    return FALSE;
  }

  CCOPConfigurationList* pCOPConfigurationList = pCommConf->GetCopConfigurationList();
  if (!CPObject::IsValidPObjectPtr(pCOPConfigurationList))
  {
    PASSERT(111);
    return FALSE;
  }

  CCopVideoParams* pCopEncoderLevelParams = pCOPConfigurationList->GetVideoMode(encoder_index);
  if (!CPObject::IsValidPObjectPtr(pCopEncoderLevelParams))
  {
    PASSERT(111);
    return FALSE;
  }

  // The Encoder level original params according to GUI
  CBridgePartyVideoOutParams* pLevelEncoderOutVideoParams = new CBridgePartyVideoOutParams();
  CCopVideoModeTable          videoModeTable;
  videoModeTable.GetCOPEncoderBridgePartyVideoOutParams(pCopEncoderLevelParams, *pLevelEncoderOutVideoParams);

  DWORD newVideoAlgo                    = pOutVideoParams.GetVideoAlgorithm();
  DWORD originalEncoderVideoAlgo        = pLevelEncoderOutVideoParams->GetVideoAlgorithm();
  BYTE  isUpdateSucceed                 = NO;
  BYTE  isPALAccordingToOriginalEncoder = IsPalFrameRate(pLevelEncoderOutVideoParams->GetVidFrameRate());
  switch (newVideoAlgo)
  {
    case H264:
    {
      DWORD maxMBPS             = pOutVideoParams.GetMBPS();
      DWORD maxFS               = pOutVideoParams.GetFS();
      DWORD originalEncoderFS   = INVALID;
      DWORD originalEncoderMBPS = INVALID;
      if (originalEncoderVideoAlgo == H264)
      {
        originalEncoderFS   = pLevelEncoderOutVideoParams->GetFS();
        originalEncoderMBPS = pLevelEncoderOutVideoParams->GetMBPS();
      }
      else // H263/H261
      {
        pLevelEncoderOutVideoParams->GetMaxMBPSAndFSFromH263H261Params(originalEncoderFS, originalEncoderMBPS);
      }

      if (maxMBPS > originalEncoderMBPS)
      {
        PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::UpdateEncoderVideoParamsForAsymmetricCascade update MBPS according to original encoder maxMBPS, ", originalEncoderMBPS);
        maxMBPS = originalEncoderMBPS;
      }

      if (maxFS > originalEncoderFS)
      {
        PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::UpdateEncoderVideoParamsForAsymmetricCascade update FS according to original encoder maxMBPS, ", originalEncoderFS);
        maxFS = originalEncoderFS;
      }

      isUpdateSucceed = videoModeTable.UpdateH264VideoParamsAccordingToSupportredCopEncoderFormat(pOutVideoParams, maxFS, maxMBPS, isPALAccordingToOriginalEncoder);
      if (!isUpdateSucceed)
        PASSERT(1);

      break;
    }

    case H263:
    case H261:
    {
      eVideoResolution originalEncoderResolution = eVideoResolutionDummy;
      eVideoResolution videoResolution           = pOutVideoParams.GetVideoResolution();
      if (originalEncoderVideoAlgo == H264)
      {
        originalEncoderResolution = TranslateFSToH263H261CopEncoderResolutions(pLevelEncoderOutVideoParams->GetFS());
      }
      else // H263/H261
      {
        originalEncoderResolution = pLevelEncoderOutVideoParams->GetVideoResolution();
      }

      if (videoResolution > originalEncoderResolution)
      {
        PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::UpdateEncoderVideoParamsForAsymmetricCascade update H263 resolution according to original resolution, ", originalEncoderResolution);
        videoResolution = originalEncoderResolution;
      }

      if (newVideoAlgo == H263)
        isUpdateSucceed = videoModeTable.UpdateH263VideoParamsAccordingToSupportredCopEncoderFormat(pOutVideoParams, videoResolution, isPALAccordingToOriginalEncoder);
      else
        isUpdateSucceed = videoModeTable.UpdateH261VideoParamsAccordingToSupportredCopEncoderFormat(pOutVideoParams, videoResolution, isPALAccordingToOriginalEncoder);

      break;
    }

    default:
    {
      PASSERT(1);
      break;
    }
  } // switch

  pOutVideoParams.DumpVideoParams();

  return TRUE;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnLevelEncoderVideoOutUpdatedCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnLevelEncoderVideoOutUpdatedCONNECTED, ConfName - ", m_pConfName);
  OnLevelEncoderVideoOutUpdated(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnLevelEncoderVideoOutUpdatedINSWITCH(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnLevelEncoderVideoOutUpdatedCONNECTED, ConfName - ", m_pConfName);
  OnLevelEncoderVideoOutUpdated(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnLevelEncoderVideoOutUpdated(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnLevelEncoderVideoOutUpdated, ConfName - ", m_pConfName);
  WORD            receivedStatus = statOK;
  EMediaDirection eMediaDirection;
  DWORD           encoderIndex;
  CTaskApp*       pParty         = NULL;
  *pParam >> (void*&)pParty;
  *pParam >> (WORD&)receivedStatus;
  *pParam >> (WORD&)eMediaDirection;
  *pParam >> (DWORD&)encoderIndex;

  if (receivedStatus != statOK)
  {
    PASSERT(receivedStatus);
  }

  if (IsActiveLectureMode() && m_pCopLectureModeCntl->GetUpdateCascadeLinkAsLecturer())
  {
    if (encoderIndex == m_lecturerlevelEncoderIndex)
    {
      m_pCopCascadeLinkLecturerEncoderWasUpdated = YES;
      if (IsEndUpdateCascadeLinkAsLecturer())
        EndUpdateCascadeLinkAsLecturer();
    }
  }
  else if (m_pCopLectureModeCntl->GetUpdateCascadeLinkAsNotLecturer())
  {
    PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::OnLevelEncoderVideoOutUpdated, UpdateCascadeLinkAsNotLecturer encoderIndex= - ", encoderIndex);
    CVideoBridgePartyCntl* prevCascadeLinkLecturer = GetPrevCascadeLinkLecturer();
    if (encoderIndex == m_copEncoderIndexOfCascadeLinkLecturer)   // levelEncoderIndex)
    {
      PTRACE(eLevelInfoNormal, "LECTURE_MODE:ASYMMETRIC_CASCADE CVideoBridgeCOP::OnLevelEncoderVideoOutUpdated:  UpdateCascadeLinkAsNotLecturerOnRecevingUpdateVideoOutParams");
      m_pCopCascadeLinkLecturerEncoderWasUpdated = YES;
      if (IsEndUpdateCascadeLinkAsLecturer())
        EndUpdateCascadeLinkAsNotLecturer();
    }
    else
      PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::OnLevelEncoderVideoOutUpdated, UpdateCascadeLinkAsNotLecturer m_copEncoderIndexOfCascadeLinkLecturer= - ", m_copEncoderIndexOfCascadeLinkLecturer);
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::StartCascadeLecturerMode()
{
  // check params
  PTRACE2(eLevelInfoNormal, "LECTURE_MODE:ASYMMETRIC_CASCADE: CVideoBridgeCOP::StartCascadeLecturerMode: Name - ", m_pConfName);
  if (NULL == m_pLectureModeParams)
  {
    PTRACE2(eLevelError, "CVideoBridge::StartCascadeLecturerMode : invalid lecture mode params, Name - ", m_pConfName);
    DBGPASSERT_AND_RETURN(1);
  }

  if (m_pLectureModeParams->GetLecturerName()[0] == '\0')
  {
    PTRACE2(eLevelError, "CVideoBridge::StartCascadeLecturerMode : invalid lecturer`s name (audio activated and no active speaker(?)-->only update DB), Name - ", m_pConfName);
    m_pCopLectureModeCntl->UpdateCurrentParams(*m_pLectureModeParams);
    UpdateConfDBLectureMode();
    DBGPASSERT_AND_RETURN(2);
  }

  if (m_pLectureModeParams->GetLecturerName() == NIL(const char))
  {
    PTRACE2(eLevelError, "CVideoBridge::StartCascadeLecturerMode : invalid lecturer`s name, Name - ", m_pConfName);
    DBGPASSERT_AND_RETURN(3);
  }

  if (!m_pLectureModeParams->IsLectureModeOn()) // the lecture mode signal must be ON
  {
    PTRACE2(eLevelError, "CVideoBridge::StartCascadeLecturerMode : lecturer mode must be on, Name - ", m_pConfName);
    DBGPASSERT_AND_RETURN(4);
  }

  CVideoBridgePartyCntlCOP* pLecturer = (CVideoBridgePartyCntlCOP*)(GetLecturer());
  if (pLecturer == NULL)
  {
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::StartCascadeLecturerMode: lecturer not found (not connected to bridge-->only update DB), Name - ", m_pConfName);
    m_pCopLectureModeCntl->UpdateCurrentParams(*m_pLectureModeParams);
    UpdateConfDBLectureMode();
    DBGPASSERT_AND_RETURN(5);
  }

  if (IsLecturerReady() == NO)
  {
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::StartCascadeLecturerMode: lecturer not ready only update DB, Name - ", m_pConfName);
    m_pCopLectureModeCntl->UpdateCurrentParams(*m_pLectureModeParams);
    UpdateConfDBLectureMode();
    DBGPASSERT_AND_RETURN(6);
  }

  // print connection before start the lecture mode
  DumpConnnections("StartCascadeLecturerMode");

  // set active
  m_pCopLectureModeCntl->SetLectureModeActive(YES);
  m_pCopLectureModeCntl->StartCascadeLecturerMode(*m_pLectureModeParams);


  // start lecturer layout timer
  if (m_pLectureModeParams->GetLectureModeType() == eLectureModeRegular && m_pLectureModeParams->GetIsTimerOn())
  {
//    StartTimer(LECTURE_MODE_TOUT, m_pLectureModeParams->GetTimerInterval()*SECOND);
   // PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::StartCascadeLecturerMode: start LECTURE_MODE_TOUT - ", m_pConfName);
  }

  if (!((CVideoBridgePartyCntlCOP*) pLecturer)->IsPartyCascadeLinkSupportAsymmetricEMCascade())
  {
    PASSERT(7); // TODO change to start lecture mode?
  }

  StartCascadeLinkAsLecturerPendingMode(pLecturer);

  UpdateConfDBLectureMode();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::UpdateCascadeLinkAsLecturer(CVideoBridgePartyCntlCOP* pLecturer)
{
  PTRACE2(eLevelInfoNormal, "LECTURE_MODE:ASYMMETRIC_CASCADE: CVideoBridgeCOP::UpdateCascadeLinkAsLecturer: Name - ", pLecturer->GetName());
  UpdatePartyCascadeLecturerMode(pLecturer, YES);
  StartTimer(COP_LECTURE_MODE_ACTION_TIME_OUT, 2*SECOND);
  m_pCopLectureModeCntl->StartUpdateCascadeLinkAsLecturer();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::UpdateCascadeLinkAsNotLecturer(CVideoBridgePartyCntlCOP* pLecturer)
{
  PTRACE2(eLevelInfoNormal, "LECTURE_MODE:ASYMMETRIC_CASCADE: CVideoBridgeCOP::UpdateCascadeLinkAsNotLecturer: Name - ", pLecturer->GetName());
  UpdatePartyCascadeLecturerMode(pLecturer, NO);
  StartTimer(COP_LECTURE_MODE_ACTION_TIME_OUT, 2*SECOND);
  m_pCopLectureModeCntl->StartUpdateCascadeLinkAsNotLecturer();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::UpdateCascadeLinkAsNotLecturerForDisconnectingParty()
{
  PTRACE(eLevelInfoNormal, "LECTURE_MODE:ASYMMETRIC_CASCADE: CVideoBridgeCOP::UpdateCascadeLinkAsNotLecturer");
  UpdateCascadeLinkAsNotLecturerOnDisconnectingCascadeLink();
  StartTimer(COP_LECTURE_MODE_ACTION_TIME_OUT, 2*SECOND);
  m_pCopLectureModeCntl->StartUpdateCascadeLinkAsNotLecturer();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::UpdatePartyCascadeLecturerMode(CVideoBridgePartyCntlCOP* pLecturer, BYTE isSetCascadeAsLecturer)
{
	m_pConfApi->CopCascadeLinkLectureMode(pLecturer->GetPartyRsrcID(), isSetCascadeAsLecturer);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::EndUpdateCascadeLinkAsLecturer()
{
  PTRACE2COND(IsActiveLectureMode(), eLevelInfoNormal, "LECTURE_MODE:ASYMMETRIC_CASCADE: CVideoBridgeCOP::EndUpdateCascadeLinkAsLecturer: Name - ", m_pConfName);

  DeleteTimer(COP_LECTURE_MODE_ACTION_TIME_OUT);
  m_pCopLectureModeCntl->EndUpdateCascadeLinkAsLecturer();
  CVideoBridgePartyCntl* pLecturer = GetLecturer();
  if (pLecturer == NULL)
  {
    PTRACE2COND(IsActiveLectureMode(), eLevelInfoNormal, "LECTURE_MODE:ASYMMETRIC_CASCADE: CVideoBridgeCOP::EndUpdateCascadeLinkAsLecturer: lecturer not found, Name - ", m_pConfName);
    PASSERT(101);
    EndCurrentLectureModeAction();
    return;
  }

  LectureModeAction lectureModeAction = m_pCopLectureModeCntl->GetCurrentLectureModeAction();
  switch (lectureModeAction)
  {
    case eLmAction_LecturerConnect_CascadeLecturer:
    {
      m_isChangeLayoutBecauseOfLectureModeFlows = TRUE;
      ChangeLayout(m_layoutType);
      EndStartCascadeLecturerMode();
      break;
    }

    case eLmAction_ChangeLecturerToCascadeLecturer:
    {
      m_isChangeLayoutBecauseOfLectureModeFlows = TRUE;
      ChangeLayout(m_layoutType);
      EndChangeLecturerToCascadeLecturer();
      break;
    }

    case eLmAction_ChangeLecturerToAndFromCascadeLecturer:
    {
      m_isChangeLayoutBecauseOfLectureModeFlows = TRUE;
      ChangeLayout(m_layoutType);
      EndChangeLecturerToAndFromCascadeLecturer();
      break;
    }

    default:
    {
      PTRACE2COND(IsActiveLectureMode(), eLevelInfoNormal, "LECTURE_MODE:ASYMMETRIC_CASCADE: CVideoBridgeCOP::EndUpdateCascadeLinkAsLecturer: received during wrong action, Name - ", m_pConfName);
      PASSERT(102);
    }
  } // switch
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::UpdateCascadeLinkAsLecturerOnRecevingUpdateVideoOutParams(CBridgePartyVideoOutParams* pBridgePartyVideoParams, CVideoBridgePartyCntlCOP* pLecturer)
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::UpdateCascadeLinkAsLecturerOnRecevingUpdateVideoOutParams: LECTURE_MODE:ASYMMETRIC_CASCADE");

  BYTE isCopLinkLecturer = pBridgePartyVideoParams->GetIsCopLinkLecturer();
  BYTE isPartyCascadeLinkSupportAsymmetricEMCascade =  ((CVideoBridgePartyCntlCOP*) pLecturer)->IsPartyCascadeLinkSupportAsymmetricEMCascade();
  if (!isCopLinkLecturer)
  {
    PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::UpdateCascadeLinkAsLecturerOnRecevingUpdateVideoOutParams: LECTURE_MODE:ASYMMETRIC_CASCADE the partycntl didn't update the isCopLinkLecturer we will nee to fallback ");
    PASSERT(1);
  }

  if (!isPartyCascadeLinkSupportAsymmetricEMCascade)
  {
    PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::UpdateCascadeLinkAsLecturerOnRecevingUpdateVideoOutParams: LECTURE_MODE:ASYMMETRIC_CASCADE the party isnt set with the rightcascade params will nee to fallback ");
    PASSERT(2);
  }

  WORD cascadeLinkLecturerEncoderIndex = pBridgePartyVideoParams->GetCopResourceOfLecturerLinkIndex();
  if (cascadeLinkLecturerEncoderIndex != m_lecturerlevelEncoderIndex)
  {
    PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::UpdateCascadeLinkAsLecturerOnRecevingUpdateVideoOutParams: LECTURE_MODE:ASYMMETRIC_CASCADE inconsistent cascade link encoder index");
    PASSERT(3);
  }

  CopRsrcDesc encoderCopRsrcDesc = GetCopRsrcDesc(eLevelEncoderType, cascadeLinkLecturerEncoderIndex);
  pBridgePartyVideoParams->SetCopConnectionId(encoderCopRsrcDesc.connectionId); // the lecturer cascade will be connected to the encoder in index cascadeLinkLecturerEncoderIndex
  pBridgePartyVideoParams->SetCopPartyId(encoderCopRsrcDesc.rsrcEntityId);
  WORD isUpdateOK = UpdateEncoderVideoParamsForAsymmetricCascade(*pBridgePartyVideoParams, cascadeLinkLecturerEncoderIndex);
  if (!isUpdateOK)
    PASSERT(3);

  // 1. Update the lecturer cascade encoder parameters
  if(cascadeLinkLecturerEncoderIndex < NUM_OF_LEVEL_ENCODERS){
    m_pCopLevelEncoders[cascadeLinkLecturerEncoderIndex]->UpdateVideoOutParams(pBridgePartyVideoParams);
  }else{
	PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::UpdateCascadeLinkAsLecturerOnRecevingUpdateVideoOutParams: index of m_pCopLevelEncoders is out of range! ");
	PASSERT(4);
  }

  // 2. Update the lecturer cascade parameters
  pLecturer->UpdateVideoOutParams(pBridgePartyVideoParams);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::StartCascadeLinkAsLecturerPendingMode(CVideoBridgePartyCntlCOP* pLecturer)
{
	TRACEINTO << "LecturerName:" << pLecturer->GetName();

	StartTimer(COP_LECTURE_MODE_ACTION_TIME_OUT, 2*SECOND);

	m_pConfApi->CopStartCascadeLinkAsLecturerPendingMode(pLecturer->GetPartyRsrcID());
	m_pCopLectureModeCntl->StartCascadeLinkAsLecturerPendingMode();
}


////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::EndStartCascadeLinkAsLecturerPendingMode()
{
  PTRACE2COND(IsActiveLectureMode(), eLevelInfoNormal, "LECTURE_MODE:ASYMMETRIC_CASCADE: CVideoBridgeCOP::EndStartCascadeLinkAsLecturerPendingMode: Name - ", m_pConfName);

  DeleteTimer(COP_LECTURE_MODE_ACTION_TIME_OUT);
  m_pCopLectureModeCntl->EndStartCascadeLinkAsLecturerPendingMode();
  CVideoBridgePartyCntl* pLecturer = GetLecturer();
  if (pLecturer == NULL)
  {
    PTRACE2COND(IsActiveLectureMode(), eLevelInfoNormal, "LECTURE_MODE:ASYMMETRIC_CASCADE: CVideoBridgeCOP::EndStartCascadeLinkAsLecturerPendingMode: lecturer not found, Name - ", m_pConfName);
    PASSERT(111);
    EndCurrentLectureModeAction();
    return;
  }
  ChangeModeLecturerByLevelEncoder(((CVideoBridgePartyCntlCOP*) pLecturer));
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::UpdateEMPartyStartCascadeLinkAsLecturerPendingMode(CTaskApp* pParty, WORD cascadeLecturerCopEncoderIndex)
{
	PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::UpdateEMPartyStartCascadeLinkAsLecturerPendingMode  ConfName - ", m_pConfName);

	if (!CPObject::IsValidPObjectPtr(pParty))
	{
		PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::UpdateEMPartyStartCascadeLinkAsLecturerPendingMode : Invalid Party TaskApp!!! ConfName - ", m_pConfName);
		DBGPASSERT_AND_RETURN(1);
	}

	CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(((CParty*)pParty)->GetPartyRsrcID());
	if (pVideoPartyCntl == NULL)
	{
		PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::UpdateEMPartyStartCascadeLinkAsLecturerPendingMode  : Party Video Not Connected!!! ConfName - ", m_pConfName);
		DBGPASSERT_AND_RETURN(2);
	}

	if (IsLecturer(pVideoPartyCntl))
	{
		if (IsActiveLectureMode() && m_pCopLectureModeCntl->GetStartCascadeLinkAsLecturerPendingMode())
		{
			if (cascadeLecturerCopEncoderIndex <= NUM_OF_LEVEL_ENCODERS - 1)
			{
				m_lecturerlevelEncoderIndex = cascadeLecturerCopEncoderIndex;
				m_copEncoderIndexOfCascadeLinkLecturer = m_lecturerlevelEncoderIndex; // until there is a new cascade link lecturer

				PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::UpdateEMPartyStartCascadeLinkAsLecturerPendingMode  m_lecturerlevelEncoderIndex:", m_lecturerlevelEncoderIndex);
				EndStartCascadeLinkAsLecturerPendingMode();
			}
			else
			{
				PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::UpdateEMPartyStartCascadeLinkAsLecturerPendingMode: Lecturer connected, illegal levelEncoderIndex = ", cascadeLecturerCopEncoderIndex);
			}
		}
		else
		{
			PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::UpdateEMPartyStartCascadeLinkAsLecturerPendingMode  received on wrong lecture mode mode");
			PASSERT(101);
		}
	}
	else
	{
		PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::UpdateEMPartyStartCascadeLinkAsLecturerPendingMode  NOT LECTURER");
		PASSERT(101);
	}
}

////////////////////////////////////////////////////////////////////////////
BYTE CVideoBridgeCOP::IsEndUpdateCascadeLinkAsLecturer()
{
  BYTE isEndUpdateCascadeLinkAsLecturer = NO;
  if (m_pCopCascadeLinkLecturerEncoderWasUpdated && m_pCopCascadeLinkLecturerLinkOutParamsWereUpdated)
    isEndUpdateCascadeLinkAsLecturer = YES;

  PTRACE2INT(eLevelInfoNormal, "LECTURE_MODE:ASYMMETRIC_CASCADE: CVideoBridgeCOP::IsEndUpdateCascadeLinkAsLecturer = ", (DWORD)isEndUpdateCascadeLinkAsLecturer);

  return isEndUpdateCascadeLinkAsLecturer;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::EndStartCascadeLecturerMode()
{
  PTRACE2(eLevelError, "LECTURE_MODE: CVideoBridgeCOP::EndStartCascadeLecturerMode: Name - ", m_pConfName);
  m_pCopCascadeLinkLecturerEncoderWasUpdated        = NO;
  m_pCopCascadeLinkLecturerLinkOutParamsWereUpdated = NO;
  m_pCopLectureModeCntl->EndStartLectureMode();
  EndLectureModeBaseAction();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::EndEndCascadeLecturerMode()
{
  PTRACE2(eLevelError, "LECTURE_MODE:ASYMMETRIC_CASCADE CVideoBridgeCOP::EndEndCascadeLecturerMode: Name - ", m_pConfName);
  m_pCopLectureModeCntl->EndEndCascadeLecturerMode();

  // when we end lecture mode we need to clean the m_ConfLayout1x1
  POBJDELETE(m_ConfLayout1x1);
  m_ConfLayout1x1                                   = new CLayout(CP_LAYOUT_1X1, ""); // lecture mode layout
  m_pCopCascadeLinkLecturerEncoderWasUpdated        = NO;
  m_pCopCascadeLinkLecturerLinkOutParamsWereUpdated = NO;

  EndLectureModeBaseAction();

  // change layout
  m_isChangeLayoutBecauseOfLectureModeFlows = TRUE;
  // vngr-18616
  ChangeLayout(m_layoutType, FALSE, YES); // always send layout whn moving back to conf layout
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::EndCascadeLecturerMode(BYTE removeLecturer, CVideoBridgePartyCntl* pLecturer)
{
  PTRACE2(eLevelError, "LECTURE_MODE: CVideoBridgeCOP::EndCascadeLecturerMode: Name - ", m_pConfName);
  // if removeLecturer == 1 this mean that the conference goes to mode regular and LM is over
  // if removeLecturer == 0 this mean that the conference still in LM but temporary goes to mode samelayout
  if (NULL == m_pLectureModeParams)
  {
    PTRACE2(eLevelError, "CVideoBridgeCOP::EndCascadeLecturerMode : invalid lecture mode params, Name - ", m_pConfName);
    DBGPASSERT_AND_RETURN(1);
  }

  m_pCopLectureModeCntl->EndCascadeLecturerMode(*m_pLectureModeParams);

  DumpConnnections("EndCascadeLecturerMode");

  bool bOldLecturerValid = IsValidPObjectPtr(pLecturer);
  if (bOldLecturerValid)
  {
    if (!((CVideoBridgePartyCntlCOP*) pLecturer)->IsPartyCascadeLinkSupportAsymmetricEMCascade())
      PASSERT(2);
  }

  BYTE isDisconnectedCascadeLecturerParty = m_pCopLectureModeCntl->IsDisconnectedCascadeLecturerParty();

  if (isDisconnectedCascadeLecturerParty || !bOldLecturerValid)
  {
    PTRACE2(eLevelError, "LECTURE_MODE: CVideoBridgeCOP::EndCascadeLecturerMode: disconnecting party Name - ", m_pConfName);
    UpdateCascadeLinkAsNotLecturerForDisconnectingParty();
  }
  else
  {
    UpdateCascadeLinkAsNotLecturer(((CVideoBridgePartyCntlCOP*) pLecturer));
  }

  m_pCopLectureModeCntl->SetLectureModeActive(NO);

  //DeleteTimer(LECTURE_MODE_TOUT);

  if (removeLecturer == YES)
    m_pLectureModeParams->SetLecturerName("");

  UpdateConfDBLectureMode();
}

////////////////////////////////////////////////////////////////////////////
BYTE CVideoBridgeCOP::IsPrevCascadeLinkLecturer(CVideoBridgePartyCntl* pVideoBridgePartyCntl) const
{
  const char* lecturerName = NULL;
  const char* partyName    =  NULL;
  lecturerName = m_pCopLectureModeCntl->GetPrevCascadeAsLecturerName();

  if (pVideoBridgePartyCntl)
    partyName = pVideoBridgePartyCntl->GetName();

  if (partyName == NULL || lecturerName == NULL)
    return NO;

  BYTE is_prevCascadeLecturer = NO;

  DWORD len1 = strlen(lecturerName);
  DWORD len2 = strlen(partyName);
  if (len1 == len2)
    if (!strncmp(lecturerName, partyName, len1))
      is_prevCascadeLecturer = YES;

  return is_prevCascadeLecturer;
}

////////////////////////////////////////////////////////////////////////////
CVideoBridgePartyCntl* CVideoBridgeCOP::GetPrevCascadeLinkLecturer()
{
  const char* lecturerName = NULL;
  lecturerName = m_pCopLectureModeCntl->GetPrevCascadeAsLecturerName();
  return (CVideoBridgePartyCntl*)GetPartyCntl(lecturerName);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::UpdateCascadeLinkAsNotLecturerOnRecevingUpdateVideoOutParams(CBridgePartyVideoOutParams* pBridgePartyVideoParams, CVideoBridgePartyCntlCOP* pPrevCascadeLecturer)
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::UpdateCascadeLinkAsNotLecturerOnRecevingUpdateVideoOutParams: LECTURE_MODE:ASYMMETRIC_CASCADE");

  BYTE isCopLinkLecturer                            = pBridgePartyVideoParams->GetIsCopLinkLecturer();
  BYTE isPartyCascadeLinkSupportAsymmetricEMCascade =  ((CVideoBridgePartyCntlCOP*) pPrevCascadeLecturer)->IsPartyCascadeLinkSupportAsymmetricEMCascade();
  if (!isPartyCascadeLinkSupportAsymmetricEMCascade)
  {
    PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::UpdateCascadeLinkAsNotLecturerOnRecevingUpdateVideoOutParams: LECTURE_MODE:ASYMMETRIC_CASCADE the party isn't set with the right cascade params will need to fallback ");
    PASSERT(2);
  }

  if (isCopLinkLecturer) // it should be off in this stage
  {
    PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::UpdateCascadeLinkAsNotLecturerOnRecevingUpdateVideoOutParams: LECTURE_MODE:ASYMMETRIC_CASCADE the party video params aren't set with the right cascade params will need to fallback ");
    PASSERT(3);
  }

  WORD cascadeLinkOriginalEncoderIndex = m_copEncoderIndexOfCascadeLinkLecturer; // ((CVideoBridgePartyCntlCOP*) pPrevCascadeLecturer)->GetCopEncoderIndexOfCascadeLinkLecturer();
  CopRsrcDesc encoderCopRsrcDesc       = GetCopRsrcDesc(eLevelEncoderType, cascadeLinkOriginalEncoderIndex);
  WORD update_status                   = UpdateEncoderVideoParams(pBridgePartyVideoParams, ((CCommConf*)m_pConf->GetCommConf()), cascadeLinkOriginalEncoderIndex);
  if (!update_status)
    PASSERT(3);

  pBridgePartyVideoParams->SetCopConnectionId(encoderCopRsrcDesc.connectionId); // the lecturer cascade will be connected to the encoder in index cascadeLinkLecturerEncoderIndex
  pBridgePartyVideoParams->SetCopPartyId(encoderCopRsrcDesc.rsrcEntityId);

  // 1. Update the lecturer cascade encoder parameters
  m_pCopLevelEncoders[cascadeLinkOriginalEncoderIndex]->UpdateVideoOutParams(pBridgePartyVideoParams);

  // 2. Update the lecturer cascade parameters
  pPrevCascadeLecturer->UpdateVideoOutParams(pBridgePartyVideoParams);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::UpdateCascadeLinkAsNotLecturerOnDisconnectingCascadeLink()
{
  PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::UpdateCascadeLinkAsNotLecturerOnDisconnectingCascadeLink - LECTURE_MODE, ASYMMETRIC_CASCADE, EncoderIndexOfCascadeLinkLecturer:", m_copEncoderIndexOfCascadeLinkLecturer);

  PASSERTMSG_AND_RETURN(m_copEncoderIndexOfCascadeLinkLecturer >= NUM_OF_LEVEL_ENCODERS, "CVideoBridgeCOP::UpdateCascadeLinkAsNotLecturerOnDisconnectingCascadeLink - Failed, 'm_copEncoderIndexOfCascadeLinkLecturer' has invalid value");

  auto_ptr<CBridgePartyVideoOutParams> pOutVideoParams(new CBridgePartyVideoOutParams);

  CopRsrcDesc encoderCopRsrcDesc = GetCopRsrcDesc(eLevelEncoderType, m_copEncoderIndexOfCascadeLinkLecturer);

  WORD update_status             = UpdateEncoderVideoParams(pOutVideoParams.get(), ((CCommConf*)m_pConf->GetCommConf()), m_copEncoderIndexOfCascadeLinkLecturer);
  PASSERTMSG_AND_RETURN(!update_status, "CVideoBridgeCOP::UpdateCascadeLinkAsNotLecturerOnDisconnectingCascadeLink - Failed, 'update_status' has invalid value");

  // 1. Update the lecturer cascade encoder parameters
  m_pCopLevelEncoders[m_copEncoderIndexOfCascadeLinkLecturer]->UpdateVideoOutParams(pOutVideoParams.get());
  m_pCopCascadeLinkLecturerLinkOutParamsWereUpdated = YES; // so we wont wait for the partycntl to be updated it's disconnecting
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::EndUpdateCascadeLinkAsNotLecturer()
{
  PTRACE2COND(IsActiveLectureMode(), eLevelInfoNormal, "LECTURE_MODE:ASYMMETRIC_CASCADE: CVideoBridgeCOP::EndUpdateCascadeLinkAsNotLecturer: Name - ", m_pConfName);

  DeleteTimer(COP_LECTURE_MODE_ACTION_TIME_OUT);
  m_pCopLectureModeCntl->EndUpdateCascadeLinkAsNotLecturer();

  // move listeners from vsw encoder to  lecture level encoder
  SwitchListenersEncoder(NO);
}

////////////////////////////////////////////////////////////////////////////
LectureModeAction CVideoBridgeCOP::GetLectureModeChangeLecturerBaseAction(CVideoBridgePartyCntlCOP* oldLecturer, CVideoBridgePartyCntlCOP* newLecturer)
{
  LectureModeAction lectureModeBaseAction                     = eLmAction_ChangeLecturer;
  BYTE              isOldLecturerCascadeLinkSupportAsymmetric = NO;
  BYTE              isNewLecturerCascadeLinkSupportAsymmetric = NO;

  if (oldLecturer)
    isOldLecturerCascadeLinkSupportAsymmetric = oldLecturer->IsPartyCascadeLinkSupportAsymmetricEMCascade();

  if (newLecturer)
    isNewLecturerCascadeLinkSupportAsymmetric = newLecturer->IsPartyCascadeLinkSupportAsymmetricEMCascade();

  if (!oldLecturer && !newLecturer)
  {
    PTRACE(eLevelInfoNormal, "LECTURE_MODE:ASYMMETRIC_CASCADE: CVideoBridgeCOP::GetLectureModeChangeLecturerBaseAction: BOTH ARE NULL");
  }

  if (isOldLecturerCascadeLinkSupportAsymmetric && isNewLecturerCascadeLinkSupportAsymmetric)
  {
    lectureModeBaseAction = eLmAction_ChangeLecturerToAndFromCascadeLecturer;
  }
  else if (isOldLecturerCascadeLinkSupportAsymmetric)
  {
    lectureModeBaseAction = eLmAction_ChangeLecturerFromCascadeLecturer;
  }
  else if (isNewLecturerCascadeLinkSupportAsymmetric)
  {
    lectureModeBaseAction = eLmAction_ChangeLecturerToCascadeLecturer;
  }

  PTRACE2INT(eLevelInfoNormal, "LECTURE_MODE:ASYMMETRIC_CASCADE: CVideoBridgeCOP::GetLectureModeChangeLecturerBaseAction: lecture MODE CHANGE LECTURER BASE ACTION = ", lectureModeBaseAction);
  return lectureModeBaseAction;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::ChangeLecturerToCascadeLecturer(CVideoBridgeLectureModeParams& pNewVideoBridgeLectureMode)
{
  PTRACE2(eLevelInfoNormal, "LECTURE_MODE:ASYMMETRIC_CASCADE CVideoBridgeCOP::ChangeLecturerToCascadeLecturer: Name - ", m_pConfName);

  if (!m_pCopLectureModeCntl->IsLectureModeActive())
  {
    PTRACE(eLevelInfoNormal, "LECTURE_MODE: VideoBridgeCOP::ChangeLecturerToCascadeLecturer: IsLectureModeActive() = FALSE (current lecturer is not connected to bridge) --> go to StartLectureMode");
    *m_pLectureModeParams = pNewVideoBridgeLectureMode;
    StartCascadeLecturerMode();
    return;
  }

  // validity
  if (NULL == m_pLectureModeParams)
  {
    PTRACE2(eLevelError, "CVideoBridgeCOP::ChangeLecturerToCascadeLecturer : invalid lecture mode params, Name - ", m_pConfName);
    DBGPASSERT_AND_RETURN(1);
  }

  // this function begins lecture mode
  if (m_pLectureModeParams->GetLecturerName() == NIL(const char))
  {
    PTRACE2(eLevelError, "CVideoBridgeCOP::ChangeLecturerToCascadeLecturer : invalid lecturer`s name, Name - ", m_pConfName);
    DBGPASSERT_AND_RETURN(1);
  }

  DBGPASSERT_AND_RETURN(!m_pLectureModeParams->IsLectureModeOn()); // the lecture mode signal must be ON

  CVideoBridgePartyCntlCOP* pNewLecturer = (CVideoBridgePartyCntlCOP*)(GetLecturer(pNewVideoBridgeLectureMode));
  if (pNewLecturer == NULL)
  {
    PTRACE2(eLevelError, "VideoBridgeCOP::ChangeLecturerToCascadeLecturer: new lecturer not found in bridge --> end lecture mode and update db, Name - ", m_pConfName);
    EndLectureMode(NO);
    return;
  }

  if (pNewLecturer->IsReadyToStartLecturer() == NO) // fix VNGFE-3432
  {
    PTRACE2(eLevelInfoNormal, "VideoBridgeCOP::ChangeLecturerToCascadeLecturer: lecturer not ready only update DB, Name - ", m_pConfName);
    // DBGPASSERT_AND_RETURN(1);
    m_pCopLectureModeCntl->UpdateCurrentParams(*m_pLectureModeParams);
    UpdateConfDBLectureMode();
    return;
  }

  CVideoBridgePartyCntlCOP* pOldLecturer = (CVideoBridgePartyCntlCOP*)(GetLecturer());

  // VNGR-20010 recalculate changelayout type to verify it didn't change, in case of pending actions it might change
  LectureModeAction changeLecturerBaseAction = GetLectureModeChangeLecturerBaseAction(((CVideoBridgePartyCntlCOP*) pOldLecturer), ((CVideoBridgePartyCntlCOP*) pNewLecturer));
  if (eLmAction_ChangeLecturerToCascadeLecturer != changeLecturerBaseAction)
  {
    PTRACE2INT(eLevelError, "VideoBridgeCOP::ChangeLecturerToCascadeLecturer: recalculating the change layout type is different from eLmAction_ChangeLecturerToCascadeLecturer the update action is - ", changeLecturerBaseAction);
    PASSERT(101);
    DoLectureModeBaseAction(changeLecturerBaseAction, pNewVideoBridgeLectureMode);
    return;
  }

  if (!pNewLecturer->IsPartyCascadeLinkSupportAsymmetricEMCascade())
  {
    PASSERT(101); // TODO change to start lecturemode?
  }

  m_pCopLectureModeCntl->ChangeLecturerToCascadeLink(pNewVideoBridgeLectureMode);

  DumpConnnections("ChangeLecturer");

  SwitchListenersEncoder(NO);

  // start lecturer layout timer
  if (m_pLectureModeParams->GetLectureModeType() == eLectureModeRegular && m_pLectureModeParams->GetIsTimerOn())
  {
  //  StartTimer(LECTURE_MODE_TOUT, m_pLectureModeParams->GetTimerInterval()*SECOND);
  //  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::ChangeLecturer: start LECTURE_MODE_TOUT - ", m_pConfName);
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::EndChangeLecturerToCascadeLecturer()
{
  PTRACE2(eLevelError, "LECTURE_MODE: CVideoBridgeCOP::EndChangeLecturerToCascadeLecturer: Name - ", m_pConfName);

  m_pCopCascadeLinkLecturerEncoderWasUpdated        = NO;
  m_pCopCascadeLinkLecturerLinkOutParamsWereUpdated = NO;
  m_pCopLectureModeCntl->EndChangeLecturerToCascadeLecturer();
  UpdateConfDBLectureMode();
  EndLectureModeBaseAction();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::ChangeLecturerFromCascadeLecturer(CVideoBridgeLectureModeParams& pNewVideoBridgeLectureMode)
{
  PTRACE2(eLevelInfoNormal, "LECTURE_MODE:ASYMMETRIC_CASCADE CVideoBridgeCOP::ChangeLecturerFromCascadeLecturer: Name - ", m_pConfName);

  if (!m_pCopLectureModeCntl->IsLectureModeActive())
  {
    PTRACE(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::ChangeLecturer: IsLectureModeActive() = FALSE (current lecturer is not connected to bridge) --> go to StartLectureMode");
    *m_pLectureModeParams = pNewVideoBridgeLectureMode;
    StartLectureMode();
    return;
  }

  // validity
  if (NULL == m_pLectureModeParams)
  {
    PTRACE2(eLevelError, "CVideoBridgeCOP::ChangeLecturerFromCascadeLecturer : invalid lecture mode params, Name - ", m_pConfName);
    DBGPASSERT_AND_RETURN(1);
  }

  // this function begins lecture mode
  if (m_pLectureModeParams->GetLecturerName() == NIL(const char))
  {
    PTRACE2(eLevelError, "CVideoBridgeCOP::ChangeLecturerFromCascadeLecturer : invalid lecturer`s name, Name - ", m_pConfName);
    DBGPASSERT_AND_RETURN(1);
  }

  DBGPASSERT_AND_RETURN(!m_pLectureModeParams->IsLectureModeOn()); // the lecture mode signal must be ON

  CVideoBridgePartyCntlCOP* pNewLecturer = (CVideoBridgePartyCntlCOP*)(GetLecturer(pNewVideoBridgeLectureMode));
  if (pNewLecturer == NULL)
  {
    PTRACE2(eLevelError, "CVideoBridgeCOP::ChangeLecturerFromCascadeLecturer: new lecturer not found in bridge --> end lecture mode and update db, Name - ", m_pConfName);
    EndLectureMode(NO);
    return;
  }

  if (pNewLecturer->IsReadyToStartLecturer() == NO) // fix VNGFE-3432
  {
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::ChangeLecturerFromCascadeLecturer: lecturer not ready only update DB, Name - ", m_pConfName);
    m_pCopLectureModeCntl->UpdateCurrentParams(*m_pLectureModeParams);
    UpdateConfDBLectureMode();
    return;
  }

  CVideoBridgePartyCntlCOP* pOldLecturer      = (CVideoBridgePartyCntlCOP*)(GetLecturer());
  bool                      bOldLecturerValid = IsValidPObjectPtr(pOldLecturer);
  if (!bOldLecturerValid)
  {
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::ChangeLecturerFromCascadeLecturer: old cascade lecturer lecturer isnt valid, Name - ", m_pConfName);
  }
  // VNGR-20010 recalculate changelayout type to verify it didnt change, in case of pending actions it might change
  LectureModeAction changeLecturerBaseAction = GetLectureModeChangeLecturerBaseAction(((CVideoBridgePartyCntlCOP*) pOldLecturer), ((CVideoBridgePartyCntlCOP*) pNewLecturer));
  if (eLmAction_ChangeLecturerFromCascadeLecturer != changeLecturerBaseAction)
  {
    PTRACE2INT(eLevelError, "CVideoBridgeCOP::ChangeLecturerFromCascadeLecturer: recalculating the change layout type is different from eLmAction_ChangeLecturerFromCascadeLecturer the update action is - ", changeLecturerBaseAction);
    PASSERT(101);
    DoLectureModeBaseAction(changeLecturerBaseAction, pNewVideoBridgeLectureMode);
    return;
  }
  m_pCopLectureModeCntl->ChangeLecturerFromCascadeLink(pNewVideoBridgeLectureMode);

  DumpConnnections("ChangeLecturerFromCascadeLecturer");
  // SwitchListenersEncoder(NO);
  BYTE isDisconnectedCascadeLecturerParty = m_pCopLectureModeCntl->IsDisconnectedCascadeLecturerParty();

  if (isDisconnectedCascadeLecturerParty || !bOldLecturerValid)
  {
    PTRACE2(eLevelError, "LECTURE_MODE: CVideoBridgeCOP::ChangeLecturerFromCascadeLecturer: disconnecting party Name - ", m_pConfName);
    UpdateCascadeLinkAsNotLecturerForDisconnectingParty();
  }
  else
  {
    UpdateCascadeLinkAsNotLecturer(pOldLecturer);
  }
  // start lecturer layout timer
  if (m_pLectureModeParams->GetLectureModeType() == eLectureModeRegular && m_pLectureModeParams->GetIsTimerOn())
  {
    //StartTimer(LECTURE_MODE_TOUT, m_pLectureModeParams->GetTimerInterval()*SECOND);
   // PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::ChangeLecturerFromCascadeLecturer: start LECTURE_MODE_TOUT - ", m_pConfName);
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::EndChangeLecturerFromCascadeLecturer()
{
  PTRACE2(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::EndChangeLecturerFromCascadeLecturer: Name - ", m_pConfName);

  m_pCopLectureModeCntl->EndChangeLecturerFromCascadeLecturer();
  UpdateConfDBLectureMode();
  EndLectureModeBaseAction();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::ChangeLecturerToAndFromCascadeLecturer(CVideoBridgeLectureModeParams& pNewVideoBridgeLectureMode)
{
  PTRACE2(eLevelInfoNormal, "LECTURE_MODE:ASYMMETRIC_CASCADE CVideoBridgeCOP::ChangeLecturerToAndFromCascadeLecturer: Name - ", m_pConfName);

  if (!m_pCopLectureModeCntl->IsLectureModeActive())
  {
    PTRACE(eLevelInfoNormal, "LECTURE_MODE: CVideoBridgeCOP::ChangeLecturerToAndFromCascadeLecturer: IsLectureModeActive() = FALSE (current lecturer is not connected to bridge) --> go to StartLectureMode");
    *m_pLectureModeParams = pNewVideoBridgeLectureMode;
    StartLectureMode();
    return;
  }

  // validity
  if (NULL == m_pLectureModeParams)
  {
    PTRACE2(eLevelError, "CVideoBridgeCOP::ChangeLecturerToAndFromCascadeLecturer : invalid lecture mode params, Name - ", m_pConfName);
    DBGPASSERT_AND_RETURN(1);
  }

  // this function begins lecture mode
  if (m_pLectureModeParams->GetLecturerName() == NIL(const char))
  {
    PTRACE2(eLevelError, "CVideoBridgeCOP::ChangeLecturerToAndFromCascadeLecturer : invalid lecturer`s name, Name - ", m_pConfName);
    DBGPASSERT_AND_RETURN(1);
  }

  DBGPASSERT_AND_RETURN(!m_pLectureModeParams->IsLectureModeOn()); // the lecture mode signal must be ON

  CVideoBridgePartyCntlCOP* pNewLecturer = (CVideoBridgePartyCntlCOP*)(GetLecturer(pNewVideoBridgeLectureMode));
  if (pNewLecturer == NULL)
  {
    PTRACE2(eLevelError, "CVideoBridgeCOP::ChangeLecturerToAndFromCascadeLecturer: new lecturer not found in bridge --> end lecture mode and update db, Name - ", m_pConfName);
    EndLectureMode(NO);
    return;
  }

  if (pNewLecturer->IsReadyToStartLecturer() == NO) // fix VNGFE-3432
  {
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::ChangeLecturerToAndFromCascadeLecturer: lecturer not ready only update DB, Name - ", m_pConfName);
    m_pCopLectureModeCntl->UpdateCurrentParams(*m_pLectureModeParams);
    UpdateConfDBLectureMode();
    return;
  }

  CVideoBridgePartyCntlCOP* pOldLecturer      = (CVideoBridgePartyCntlCOP*)(GetLecturer());
  bool                      bOldLecturerValid = IsValidPObjectPtr(pOldLecturer);
  if (!bOldLecturerValid)
  {
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::ChangeLecturerToAndFromCascadeLecturer: old cascade lecturer lecturer isnt valid, Name - ", m_pConfName);
  }

  // VNGR-20010 recalculate changelayout type to verify it didn't change, in case of pending actions it might change
  LectureModeAction changeLecturerBaseAction = GetLectureModeChangeLecturerBaseAction(((CVideoBridgePartyCntlCOP*) pOldLecturer), ((CVideoBridgePartyCntlCOP*) pNewLecturer));
  if (eLmAction_ChangeLecturerToAndFromCascadeLecturer != changeLecturerBaseAction)
  {
    PTRACE2INT(eLevelError, "CVideoBridgeCOP::ChangeLecturerToAndFromCascadeLecturer: recalculating the change layout type is different from eLmAction_ChangeLecturerToAndFromCascadeLecturer the update action is - ", changeLecturerBaseAction);
    PASSERT(101);
    DoLectureModeBaseAction(changeLecturerBaseAction, pNewVideoBridgeLectureMode);
    return;
  }

  if (bOldLecturerValid && !pOldLecturer->IsPartyCascadeLinkSupportAsymmetricEMCascade())
  {
    PASSERT(102); // we shouldnt have gotten here
  }

  if (!pNewLecturer->IsPartyCascadeLinkSupportAsymmetricEMCascade())
  {
    PASSERT(103);  // we shouldnt have gotten here
  }

  m_pCopLectureModeCntl->ChangeLecturerToAndFromCascadeLink(pNewVideoBridgeLectureMode);

  DumpConnnections("ChangeLecturerToAndFromCascadeLecturer");

  BYTE isDisconnectedCascadeLecturerParty = m_pCopLectureModeCntl->IsDisconnectedCascadeLecturerParty();
  if (isDisconnectedCascadeLecturerParty || !bOldLecturerValid)
  {
    PTRACE2(eLevelError, "LECTURE_MODE: CVideoBridgeCOP::ChangeLecturerToAndFromCascadeLecturer: disconnecting party Name - ", m_pConfName);
    UpdateCascadeLinkAsNotLecturerForDisconnectingParty();
  }
  else
  {
    UpdateCascadeLinkAsNotLecturer(pOldLecturer);
  }

  // start lecturer layout timer
  if (m_pLectureModeParams->GetLectureModeType() == eLectureModeRegular && m_pLectureModeParams->GetIsTimerOn())
  {
    //StartTimer(LECTURE_MODE_TOUT, m_pLectureModeParams->GetTimerInterval()*SECOND);
   // PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::ChangeLecturerToAndFromCascadeLecturer: start LECTURE_MODE_TOUT - ", m_pConfName);
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::EndChangeLecturerToAndFromCascadeLecturer()
{
  PTRACE2(eLevelError, "LECTURE_MODE: CVideoBridgeCOP::EndChangeLecturerToAndFromCascadeLecturer: Name - ", m_pConfName);
  m_pCopCascadeLinkLecturerEncoderWasUpdated        = NO;
  m_pCopCascadeLinkLecturerLinkOutParamsWereUpdated = NO;
  m_pCopLectureModeCntl->EndChangeLecturerToAndFromCascadeLink();
  UpdateConfDBLectureMode();
  EndLectureModeBaseAction();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::EndCurrentLectureModeActionForCascadeLecturer(LectureModeAction lecture_mode_action)
{
  PTRACE2(eLevelError, "LECTURE_MODE:ASYMMETRIC_CASCADE CVideoBridgeCOP::EndCurrentLectureModeActionForCascadeLecturer: Name - ", m_pConfName);
  switch (lecture_mode_action)
  {
    case eLmAction_LecturerConnect_CascadeLecturer:
    {
      EndStartCascadeLecturerMode();
      break;
    }

    case eLmAction_ChangeLecturerToCascadeLecturer:
    {
      EndChangeLecturerToCascadeLecturer();
      break;
    }

    case eLmAction_ChangeLecturerFromCascadeLecturer:
    {
      EndChangeLecturerFromCascadeLecturer();
      break;
    }

    case eLmAction_ChangeLecturerToAndFromCascadeLecturer:
    {
      EndChangeLecturerToAndFromCascadeLecturer();
      break;
    }

    default:
    {
      PASSERT(1);
    }
  } // switch

  if (m_pCopLectureModeCntl->GetStartCascadeLinkAsLecturerPendingMode()) // INCASE WE DIDNT RECEIVE PENDING MODE FROM PARTY
  {
    PTRACE2(eLevelInfoNormal, "LECTURE_MODE:ASYMMETRIC_CASCADE CVideoBridgeCOP::EndCurrentLectureModeActionForCascadeLecturer pending failed we will change to disconnect lecture mode Name - ", m_pConfName);
    PASSERT(1);
  }
  else if (m_pCopLectureModeCntl->GetUpdateCascadeLinkAsLecturer())
  {
    PTRACE2(eLevelInfoNormal, "LECTURE_MODE:ASYMMETRIC_CASCADE CVideoBridgeCOP::EndCurrentLectureModeActionForCascadeLecturer UpdateCascadeLinkAsLecturer failed we will change to disconnect lecture mode Name - ", m_pConfName);
    PASSERT(2);
  }
  else if (m_pCopLectureModeCntl->GetUpdateCascadeLinkAsNotLecturer())
  {
    PTRACE2(eLevelInfoNormal, "LECTURE_MODE:ASYMMETRIC_CASCADE CVideoBridgeCOP::EndCurrentLectureModeActionForCascadeLecturer UpdateCascadeLinkAsNonLecturer failed we will change to disconnect lecture mode Name - ", m_pConfName);
    PASSERT(3);
  }

  CVideoBridgeLectureModeParams* pNewVideoBridgeLectureMode = new CVideoBridgeLectureModeParams;
  DoLectureModeBaseAction(eLmAction_LecturerDisonnect, *pNewVideoBridgeLectureMode);
  POBJDELETE(pNewVideoBridgeLectureMode);
}

////////////////////////////////////////////////////////////////////////////
BYTE CVideoBridgeCOP::IsUpdatedPartyToFromVSW(CVideoBridgePartyCntlCOP* pPartyCntl, BYTE toVsw)
{
  BYTE isUpdatedPartyToFromVSW = NO;
  WORD levelEncoderIndex       = (WORD)-1;
  if (IsValidPObjectPtr(pPartyCntl))
  {
    levelEncoderIndex = GetCopEncoderIndex((CVideoBridgePartyCntlCOP*)pPartyCntl);
    if ((levelEncoderIndex == m_lecturerlevelEncoderIndex && toVsw == NO) || ((levelEncoderIndex == VSW_ENCODER_INDEX) && toVsw == YES))
    {
      isUpdatedPartyToFromVSW = YES;
    }

    CMedString mstr;
    mstr << "Party: " << pPartyCntl->GetName()<< " ,Current Level Encoder = " << levelEncoderIndex
         << " ,m_lecturerlevelEncoderIndex: " << m_lecturerlevelEncoderIndex<< " 'is to VSW ? :"<< toVsw
         << ",isUpdatedPartyToFromVSW"<<isUpdatedPartyToFromVSW;
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::IsUpdatedPartyToFromVSW:  - ", mstr.GetString());
  }

  return isUpdatedPartyToFromVSW;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::SetSmartLecturerState(CVideoBridgePartyCntlCOP* pOldLecturer, CVideoBridgePartyCntlCOP* pNewLecturer)
{
  CSmartLecturerState newSmartLecturerState;

  LayoutType layoutType = m_ConfLayout->GetLayoutType();

  std::ostringstream msg;
  msg << "CVideoBridgeCOP::SetSmartLecturerState:";
  msg << "\n  ConfName                  :" <<  m_pConfName;
  msg << "\n  LayoutType                :" << LayoutTypeAsString[layoutType];
  msg << "\n  IsAutoScanSet             :" << (WORD)m_ConfLayout->IsAutoScanSet();
  if (pOldLecturer && pNewLecturer && CP_LAYOUT_1X1 == layoutType && !m_ConfLayout->IsAutoScanSet())
  {
    if (!CPObject::IsValidPObjectPtr(pOldLecturer))
    {
      msg << "\n  pOldLecturer              :NULL"; goto FUNCTION_EXIT;
    }

    if (!CPObject::IsValidPObjectPtr(pNewLecturer))
    {
      msg << "\n  pNewLecturer              :NULL"; goto FUNCTION_EXIT;
    }

    CLayout* pLayout = m_ConfLayout1x1;
    switch (m_SmartLecturerState.m_eState)
    {
      case eLmSmartState_None:
      case eLmSmartState_SameLevel1Lecturer:
      case eLmSmartState_DiffLevel1Lecturer:
        pLayout = m_ConfLayout;
        break;
      default:
    	// Note: some enumeration value are not handled in switch. Add default to suppress warning.
    	break;
    } // switch

    CVidSubImage* pVidSubImage = (*pLayout)[0];
    if (!pVidSubImage)
    {
      msg << "\n  pVidSubImage              :NULL"; goto FUNCTION_EXIT;
    }

    CImage* pImage = NULL;
    DWORD partyRscId = pVidSubImage->GetImageId();
    if (partyRscId)
    {
      pImage = GetPartyImageLookupTable()->GetPartyImage(partyRscId);
      PASSERTSTREAM(!pImage, "CBridgePartyVideoOut::SetSmartLecturerState - Failed, The lookup table doesn't have an element, PartyId:" << partyRscId);
    }

    if (!pImage)
    {
      msg << "\n  pImage                    :NULL"; goto FUNCTION_EXIT;
    }

    DWORD partyIdSeenByLecturer = pImage->GetArtPartyId();

    newSmartLecturerState.m_oldLecturer.m_PartyRsrcID          = pOldLecturer->GetPartyRsrcID();
    newSmartLecturerState.m_newLecturer.m_PartyRsrcID          = pNewLecturer->GetPartyRsrcID();

    newSmartLecturerState.m_oldLecturer.m_LevelEncoderEntityId = pOldLecturer->GetCopEncoderEntityId();
    newSmartLecturerState.m_newLecturer.m_LevelEncoderEntityId = pNewLecturer->GetCopEncoderEntityId();

    newSmartLecturerState.m_oldLecturer.m_LevelEncoderIndex    = GetCopEncoderIndexFromEntityId(newSmartLecturerState.m_oldLecturer.m_LevelEncoderEntityId);
    newSmartLecturerState.m_newLecturer.m_LevelEncoderIndex    = GetCopEncoderIndexFromEntityId(newSmartLecturerState.m_newLecturer.m_LevelEncoderEntityId);

    msg << "\n  partyRsrcIDSeenByLecturer :" <<  partyIdSeenByLecturer;
    msg << "\n  oldLecturerPartyRsrcID    :" <<  newSmartLecturerState.m_oldLecturer.m_PartyRsrcID << " (" << pOldLecturer->GetName() << ")";
    msg << "\n  newLecturerPartyRsrcID    :" <<  newSmartLecturerState.m_newLecturer.m_PartyRsrcID << " (" << pNewLecturer->GetName() << ")";
    msg << "\n  oldLevelEncoderEntityId   :" <<  newSmartLecturerState.m_oldLecturer.m_LevelEncoderEntityId;
    msg << "\n  newLevelEncoderEntityId   :" <<  newSmartLecturerState.m_newLecturer.m_LevelEncoderEntityId;
    msg << "\n  oldLevelEncoderIndex      :" <<  newSmartLecturerState.m_oldLecturer.m_LevelEncoderIndex;
    msg << "\n  newLevelEncoderIndex      :" <<  newSmartLecturerState.m_newLecturer.m_LevelEncoderIndex;

    if (newSmartLecturerState.m_newLecturer.m_PartyRsrcID == partyIdSeenByLecturer)
    {
      // If new or old lecturer connected to VSW that means that old and new lecturers are in the same level, otherwise in the different level
      if (newSmartLecturerState.m_newLecturer.m_LevelEncoderIndex == VSW_ENCODER_INDEX || newSmartLecturerState.m_oldLecturer.m_LevelEncoderIndex == VSW_ENCODER_INDEX)
        newSmartLecturerState.m_eState = (m_SmartLecturerState.m_eState == eLmSmartState_SameLevel1Lecturer || m_SmartLecturerState.m_eState == eLmSmartState_None) ? eLmSmartState_SameLevel2Lecturer : eLmSmartState_SameLevel1Lecturer;
      else
        newSmartLecturerState.m_eState = (m_SmartLecturerState.m_eState == eLmSmartState_DiffLevel1Lecturer || m_SmartLecturerState.m_eState == eLmSmartState_None) ? eLmSmartState_DiffLevel2Lecturer : eLmSmartState_DiffLevel1Lecturer;
    }
  }

FUNCTION_EXIT:

  if (newSmartLecturerState.m_eState == eLmSmartState_None)
    newSmartLecturerState.Clean();

  msg << "\n  oldSmartLectureState      :" << m_SmartLecturerState.GetStateName(eLmSmartState_Current);
  m_SmartLecturerState = newSmartLecturerState;
  msg << "\n  newSmartLectureState      :" << m_SmartLecturerState.GetStateName(eLmSmartState_Current);
  PTRACE(eLevelInfoNormal, msg.str().c_str());

  DumpLayouts(NULL);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfUpdateSiteNameInfoCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnConfUpdateSiteNameInfoCONNECTED : Name - ", m_pConfName);

  m_pSiteNameInfo->DeSerialize(NATIVE, *pParam);

  for (DWORD encoder_index = 0; encoder_index < NUM_OF_LEVEL_ENCODERS; encoder_index++)
    m_pCopLevelEncoders[encoder_index]->SetSiteNameConfiguration(m_pSiteNameInfo);

  SendChangeLayoutToAllLevelEncoders();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCOP::OnConfUpdateRefreshLayoutCONNECTED(CSegment* pParam)
{
  SendChangeLayoutToAllLevelEncoders();
}

//--------------------------------------------------------------------------
void CVideoBridgeCOP::SendMessageOverlayToNewConnectedPartyIfNeeded(CSegment* pParam)
{
	m_pMessageOverlayInfo->Dump("CVideoBridgeCOP::SendMessageOverlayToNewConnectedPartyIfNeeded");

	PartyRsrcID PartyId;
	*pParam >> PartyId;

	TRACEINTO << "ConfName:" << m_pConfName << ", PartyId:" << PartyId;

	CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(PartyId);
	PASSERT_AND_RETURN(!pPartyCntl);

	DWORD levelEncoderEntityId = ((CVideoBridgePartyCntlCOP*)pPartyCntl)->GetCopEncoderEntityId();
	WORD  levelEncoderIndex    = GetCopEncoderIndexFromEntityId(levelEncoderEntityId);
	if (levelEncoderIndex <= NUM_OF_LEVEL_ENCODERS-1)
		((CVideoBridgePartyCntl*)m_pCopLevelEncoders[levelEncoderIndex])->StartMessageOverlay(m_pMessageOverlayInfo);
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCOP::OnDisplayIndicationIconCONNECTED(CSegment* pParam)
{
  WORD iconType = MAX_NUM_TYPES_OF_ICON; *pParam >> iconType;
  WORD iconValue = 0; *pParam >> iconValue;

  if(eIconRecording!= iconType)
    return;

  m_eRecordingIcon = (EIconType)iconValue;

  PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::OnDisplayRecordingIconCONNECTED - Icon:", EIconRecordingTypeNames[m_eRecordingIcon]);

  for (WORD encoder_index = 0; encoder_index < NUM_OF_LEVEL_ENCODERS; encoder_index++)
  {
    if (!CPObject::IsValidPObjectPtr(m_pCopLevelEncoders[encoder_index]))
    {
      PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCOP::OnDisplayRecordingIconCONNECTED invalid COP encoder - m_pCopLevelEncoders index = ", encoder_index);
      continue;
    }

    CBridgePartyVideoOut* pBridgePartyVideoOut = (CBridgePartyVideoOut *)(m_pCopLevelEncoders[encoder_index]->GetBridgePartyOut());
    if (pBridgePartyVideoOut)
    {
      pBridgePartyVideoOut->SetRecordingType((EIconType)iconValue);
      pBridgePartyVideoOut->UpdateIndicationIcons(YES);
      continue;
    }
  }
}

