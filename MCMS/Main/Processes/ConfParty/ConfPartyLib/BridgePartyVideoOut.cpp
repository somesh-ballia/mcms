#include "BridgePartyVideoOut.h"

#include "TextOnScreenMngr.h"
#include "StatusesGeneral.h"
#include "Layout.h"
#include "LayoutHandler.h"

#include "OpcodesMcmsCardMngrIvrCntl.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsVideo.h"
#include "OpcodesMcmsCardMngrTB.h"

#include "HostCommonDefinitions.h"
#include "ConfPartyGlobals.h"

#include "Gathering.h"
#include "VideoApiDefinitionsStrings.h"
#include "SiteNameInfo.h"
#include "ConfPartyProcess.h"

#include "VideoHardwareInterface.h"
#include "H264Util.h"

#include "SysConfigKeys.h"
#include "ConfigHelper.h"

#include "TraceStream.h"
#include "EnumsToStrings.h"

///////////////////////////////////////////////////////////////////////////
// Time-out values
#define VIDEO_OUT_CHANGE_LAYOUT_TOUT  15 	//Change Layout Improvement (CL-ACK) - timer was changed to 150 msec since MplApi doesn't forward ACK_IND anymore	//8*SECOND  //8 second timeout
#define SITE_NAME_DISPLAY_TOUT        10*SECOND // was 5*SECOND changed from v4.1c
#define SLIDE_INTRA_TIMER             SECOND/2
#define ENCODER_UPDATE_PARAM_TOUT     ((WORD)302)
#define GATHERING_TMP_TIMER_5         5*SECOND

//~~~~~~~~~~~~~~ Global functions ~~~~~~~~~~~~~~~~~~~~~~~~~~
extern CConfPartyRoutingTable* GetpConfPartyRoutingTable(void);
extern CMessageOverlayInfo * GetMessageOverlayInfo();
LayoutType GetNewLayoutType(const BYTE oldLayoutType);
///////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CBridgePartyVideoOut) ONEVENT(CONNECT_VIDEO_OUT ,IDLE ,CBridgePartyVideoOut::OnVideoBridgePartyConnectIDLE)
ONEVENT(CONNECT_VIDEO_OUT ,SLIDE ,CBridgePartyVideoOut::OnVideoBridgePartyConnectSLIDE)
ONEVENT(CONNECT_VIDEO_OUT ,SETUP ,CBridgePartyVideoOut::OnVideoBridgePartyConnectSETUP)
ONEVENT(CONNECT_VIDEO_OUT ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgePartyConnectCONNECTED)
ONEVENT(CONNECT_VIDEO_OUT ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgePartyConnectCHANGELAYOUT)
ONEVENT(CONNECT_VIDEO_OUT ,DISCONNECTING ,CBridgePartyVideoOut::OnVideoBridgePartyConnectDISCONNECTING)

ONEVENT(DISCONNECT_VIDEO_OUT ,IDLE ,CBridgePartyVideoOut::OnVideoBridgePartyDisConnectIDLE)
ONEVENT(DISCONNECT_VIDEO_OUT ,SLIDE ,CBridgePartyVideoOut::OnVideoBridgePartyDisConnectSLIDE)
ONEVENT(DISCONNECT_VIDEO_OUT ,SETUP ,CBridgePartyVideoOut::OnVideoBridgePartyDisConnectSETUP)
ONEVENT(DISCONNECT_VIDEO_OUT ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgePartyDisConnectCONNECTED)
ONEVENT(DISCONNECT_VIDEO_OUT ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgePartyDisConnectCHANGELAYOUT)
ONEVENT(DISCONNECT_VIDEO_OUT ,DISCONNECTING ,CBridgePartyVideoOut::OnVideoBridgePartyDisConnectDISCONNECTING)

ONEVENT(ACK_IND ,IDLE ,CBridgePartyVideoOut::OnMplAckIDLE)
ONEVENT(ACK_IND ,SLIDE ,CBridgePartyVideoOut::OnMplAckSLIDE)
ONEVENT(ACK_IND ,SETUP ,CBridgePartyVideoOut::OnMplAckSETUP)
ONEVENT(ACK_IND ,CONNECTED ,CBridgePartyVideoOut::OnMplAckCONNECTED)
ONEVENT(ACK_IND ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnMplAckCHANGELAYOUT)
ONEVENT(ACK_IND ,DISCONNECTING ,CBridgePartyVideoOut::OnMplAckDISCONNECTING)

ONEVENT(FASTUPDATE ,IDLE ,CBridgePartyVideoOut::NullActionFunction)
ONEVENT(FASTUPDATE ,SLIDE ,CBridgePartyVideoOut::OnVideoBridgePartyFastUpdateSLIDE)
ONEVENT(FASTUPDATE ,SETUP ,CBridgePartyVideoOut::OnVideoBridgePartyFastUpdateSETUP)
ONEVENT(FASTUPDATE ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgePartyFastUpdateCONNECTED)
ONEVENT(FASTUPDATE ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgePartyFastUpdateCHANGELAYOUT)
ONEVENT(FASTUPDATE ,DISCONNECTING ,CBridgePartyVideoOut::NullActionFunction)

ONEVENT(UPDATE_VIDEO_OUT_PARAMS ,IDLE ,CBridgePartyVideoOut::OnVideoBridgePartyUpdateVideoParamsIDLE)
ONEVENT(UPDATE_VIDEO_OUT_PARAMS ,SLIDE ,CBridgePartyVideoOut::OnVideoBridgePartyUpdateVideoParamsSLIDE)
ONEVENT(UPDATE_VIDEO_OUT_PARAMS ,SETUP ,CBridgePartyVideoOut::OnVideoBridgePartyUpdateVideoParamsSETUP)
ONEVENT(UPDATE_VIDEO_OUT_PARAMS ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgePartyUpdateVideoParamsCONNECTED)
ONEVENT(UPDATE_VIDEO_OUT_PARAMS ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgePartyUpdateVideoParamsCHANGELAYOUT)
ONEVENT(UPDATE_VIDEO_OUT_PARAMS ,DISCONNECTING ,CBridgePartyVideoOut::OnVideoBridgePartyUpdateVideoParamsDISCONNECTING)

ONEVENT(ADDIMAGE ,SETUP ,CBridgePartyVideoOut::NullActionFunction)
ONEVENT(ADDIMAGE ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgePartyAddImageCONNECTED)
ONEVENT(ADDIMAGE ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgePartyNotifyCHANGELAYOUT)
ONEVENT(ADDIMAGE ,DISCONNECTING ,CBridgePartyVideoOut::NullActionFunction)

ONEVENT(UPDATEIMAGE ,SETUP ,CBridgePartyVideoOut::NullActionFunction)
ONEVENT(UPDATEIMAGE ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgePartyUpdateImageCONNECTED)
ONEVENT(UPDATEIMAGE ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgePartyUpdateImageCHANGELAYOUT)
ONEVENT(UPDATEIMAGE ,DISCONNECTING ,CBridgePartyVideoOut::NullActionFunction)

ONEVENT(DELIMAGE ,IDLE ,CBridgePartyVideoOut::NullActionFunction)
ONEVENT(DELIMAGE ,SETUP ,CBridgePartyVideoOut::NullActionFunction)
ONEVENT(DELIMAGE ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgePartyDelImageCONNECTED)
ONEVENT(DELIMAGE ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgePartyNotifyCHANGELAYOUT)
ONEVENT(DELIMAGE ,DISCONNECTING ,CBridgePartyVideoOut::NullActionFunction)

ONEVENT(MUTEIMAGE ,SETUP ,CBridgePartyVideoOut::NullActionFunction)
ONEVENT(MUTEIMAGE ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgePartyMuteImageCONNECTED)
ONEVENT(MUTEIMAGE ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgePartyNotifyCHANGELAYOUT)
ONEVENT(MUTEIMAGE ,DISCONNECTING ,CBridgePartyVideoOut::NullActionFunction)

ONEVENT(UNMUTEIMAGE ,SETUP ,CBridgePartyVideoOut::NullActionFunction)
ONEVENT(UNMUTEIMAGE ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgePartyUnMuteImageCONNECTED)
ONEVENT(UNMUTEIMAGE ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgePartyNotifyCHANGELAYOUT)
ONEVENT(UNMUTEIMAGE ,DISCONNECTING ,CBridgePartyVideoOut::NullActionFunction)

ONEVENT(SPEAKERS_CHANGED ,SETUP ,CBridgePartyVideoOut::NullActionFunction)
ONEVENT(SPEAKERS_CHANGED ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgePartyChangeSpeakersCONNECTED)
ONEVENT(SPEAKERS_CHANGED ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgePartyChangeSpeakersCHANGELAYOUT)
ONEVENT(SPEAKERS_CHANGED ,DISCONNECTING ,CBridgePartyVideoOut::NullActionFunction)

ONEVENT(AUDIO_SPEAKER_CHANGED ,SETUP ,CBridgePartyVideoOut::NullActionFunction)
ONEVENT(AUDIO_SPEAKER_CHANGED ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgePartyChangeAudioSpeakerCONNECTED)
ONEVENT(AUDIO_SPEAKER_CHANGED ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgePartyChangeAudioSpeakerCHANGELAYOUT)
ONEVENT(AUDIO_SPEAKER_CHANGED ,DISCONNECTING ,CBridgePartyVideoOut::NullActionFunction)

ONEVENT(CHANGECONFLAYOUT ,SETUP ,CBridgePartyVideoOut::NullActionFunction)
ONEVENT(CHANGECONFLAYOUT ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgePartyChangeConfLayoutCONNECTED)
ONEVENT(CHANGECONFLAYOUT ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgePartyChangeConfLayoutCHANGELAYOUT)
ONEVENT(CHANGECONFLAYOUT ,DISCONNECTING ,CBridgePartyVideoOut::NullActionFunction)

ONEVENT(CHANGEPARTYLAYOUT_TPROOM_SUBLINK ,SETUP ,CBridgePartyVideoOut::NullActionFunction)
ONEVENT(CHANGEPARTYLAYOUT_TPROOM_SUBLINK ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgeChangeLayoutOfTPRoomSublinkCONNECTED)
ONEVENT(CHANGEPARTYLAYOUT_TPROOM_SUBLINK ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgeChangeLayoutOfTPRoomSublinkCHANGELAYOUT)
ONEVENT(CHANGEPARTYLAYOUT_TPROOM_SUBLINK ,DISCONNECTING ,CBridgePartyVideoOut::NullActionFunction)

ONEVENT(CHANGEPARTYLAYOUT ,SETUP ,CBridgePartyVideoOut::NullActionFunction)
ONEVENT(CHANGEPARTYLAYOUT ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgePartyChangePartyLayoutCONNECTED)
ONEVENT(CHANGEPARTYLAYOUT ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgePartyChangePartyLayoutCHANGELAYOUT)
ONEVENT(CHANGEPARTYLAYOUT ,DISCONNECTING ,CBridgePartyVideoOut::NullActionFunction)

ONEVENT(CHANGEPARTYPRIVATELAYOUT ,SLIDE ,CBridgePartyVideoOut::OnVideoBridgePartyChangePrivateLayoutSLIDE)
ONEVENT(CHANGEPARTYPRIVATELAYOUT ,SETUP ,CBridgePartyVideoOut::OnVideoBridgePartyChangePrivateLayoutSETUP)
ONEVENT(CHANGEPARTYPRIVATELAYOUT ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgePartyChangePrivateLayoutCONNECTED)
ONEVENT(CHANGEPARTYPRIVATELAYOUT ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgePartyChangePrivateLayoutCHANGELAYOUT)
ONEVENT(CHANGEPARTYPRIVATELAYOUT ,DISCONNECTING ,CBridgePartyVideoOut::NullActionFunction)

ONEVENT(SETPARTYPRIVATELAYOUTONOFF ,SETUP ,CBridgePartyVideoOut::NullActionFunction)
ONEVENT(SETPARTYPRIVATELAYOUTONOFF ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgePartySetPrivateLayoutOnOffCONNECTED)
ONEVENT(SETPARTYPRIVATELAYOUTONOFF ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgePartySetPrivateLayoutOnOffCHANGELAYOUT)
ONEVENT(SETPARTYPRIVATELAYOUTONOFF ,DISCONNECTING ,CBridgePartyVideoOut::NullActionFunction)

ONEVENT(ENCODER_RESOLUTION_CHANGED ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgePartyEncoderResolutionChangedCONNECTED)
ONEVENT(ENCODER_RESOLUTION_CHANGED ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgePartyEncoderResolutionChangedCHANGELAYOUT)
/*Unmark by Romem*/
ONEVENT(UPDATEVISUALEFFECTS ,IDLE ,CBridgePartyVideoOut::OnVideoBridgePartyChangeVisualEffectsIDLE)
ONEVENT(UPDATEVISUALEFFECTS ,SLIDE ,CBridgePartyVideoOut::OnVideoBridgePartyChangeVisualEffectsSLIDE)
ONEVENT(UPDATEVISUALEFFECTS ,SETUP ,CBridgePartyVideoOut::OnVideoBridgePartyChangeVisualEffectsSETUP)
ONEVENT(UPDATEVISUALEFFECTS ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgePartyChangeVisualEffectsCONNECTED)
ONEVENT(UPDATEVISUALEFFECTS ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgePartyChangeVisualEffectsCHANGELAYOUT)
/*Unmark by Romem*/
ONEVENT(IVR_SHOW_SLIDE_REQ ,IDLE ,CBridgePartyVideoOut::OnVideoBridgePartyShowSlideIDLE)
ONEVENT(IVR_SHOW_SLIDE_REQ ,SLIDE ,CBridgePartyVideoOut::OnVideoBridgePartyShowSlideSLIDE)

ONEVENT(IVR_STOP_SHOW_SLIDE_REQ ,IDLE ,CBridgePartyVideoOut::OnVideoBridgePartyStopShowSlideIDLE)
ONEVENT(IVR_STOP_SHOW_SLIDE_REQ ,SLIDE ,CBridgePartyVideoOut::OnVideoBridgePartyStopShowSlideSLIDE)

ONEVENT(VIDEO_GRAPHIC_OVERLAY_START_REQ ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgePartyStartPLC)
ONEVENT(VIDEO_GRAPHIC_OVERLAY_START_REQ ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgePartyStartPLC)

ONEVENT(VIDEO_GRAPHIC_OVERLAY_STOP_REQ ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgePartyStopPLC)
ONEVENT(VIDEO_GRAPHIC_OVERLAY_STOP_REQ ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgePartyStopPLC)

ONEVENT(PLC_SETPARTYPRIVATELAYOUTTYPE ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgePartyPLC_SetPrivateLayoutTypeCONNECTED)
ONEVENT(PLC_SETPARTYPRIVATELAYOUTTYPE ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgePartyPLC_SetPrivateLayoutTypeCHANGELAYOUT)

ONEVENT(PLC_RETURNPARTYTOCONFLAYOUT ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgePartyPLC_ReturnToConfLayoutCONNECTED)
ONEVENT(PLC_RETURNPARTYTOCONFLAYOUT ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgePartyPLC_ReturnToConfLayoutCHANGELAYOUT)

ONEVENT(PLC_FORCECELLZERO ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgePartyPLC_ForceCellCONNECTED)
ONEVENT(PLC_FORCECELLZERO ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgePartyPLC_ForceCellCHANGELAYOUT)
ONEVENT(PLC_CANCELALLPRIVATELAYOUTFORCES ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgePartyPLC_CancelAllPrivateLayoutForcesCONNECTED)
ONEVENT(PLC_CANCELALLPRIVATELAYOUTFORCES ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgePartyPLC_CancelAllPrivateLayoutForcesCHANGELAYOUT)

ONEVENT(UPDATE_PARTY_LECTURE_MODE_ROLE ,IDLE ,CBridgePartyVideoOut::OnVideoBridgePartyUpdateLectureModeRoleIDLE)
ONEVENT(UPDATE_PARTY_LECTURE_MODE_ROLE ,SLIDE ,CBridgePartyVideoOut::OnVideoBridgePartyUpdateLectureModeRoleSLIDE)
ONEVENT(UPDATE_PARTY_LECTURE_MODE_ROLE ,SETUP ,CBridgePartyVideoOut::OnVideoBridgePartyUpdateLectureModeRoleSETUP)
ONEVENT(UPDATE_PARTY_LECTURE_MODE_ROLE ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgePartyUpdateLectureModeRoleCONNECTED)
ONEVENT(UPDATE_PARTY_LECTURE_MODE_ROLE ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgePartyUpdateLectureModeRoleCHANGELAYOUT)
ONEVENT(UPDATE_PARTY_LECTURE_MODE_ROLE ,DISCONNECTING ,CBridgePartyVideoOut::NullActionFunction)

ONEVENT(DELETED_PARTY_FROM_CONF ,SETUP ,CBridgePartyVideoOut::NullActionFunction)
ONEVENT(DELETED_PARTY_FROM_CONF ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgePartyDeletePartyFromConfCONNECTED)
ONEVENT(DELETED_PARTY_FROM_CONF ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgePartyDeletePartyFromConfCHANGELAYOUT)
ONEVENT(DELETED_PARTY_FROM_CONF ,DISCONNECTING ,CBridgePartyVideoOut::NullActionFunction)

ONEVENT(SITE_NAME_DISPLAY_OFF ,IDLE ,CBridgePartyVideoOut::NullActionFunction)
ONEVENT(SITE_NAME_DISPLAY_OFF ,SETUP ,CBridgePartyVideoOut::NullActionFunction)
ONEVENT(SITE_NAME_DISPLAY_OFF ,CONNECTED ,CBridgePartyVideoOut::OnSiteNameToutCONNECTED)
ONEVENT(SITE_NAME_DISPLAY_OFF ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnSiteNameToutCHANGE_LAYOUT)
ONEVENT(SITE_NAME_DISPLAY_OFF ,DISCONNECTING ,CBridgePartyVideoOut::NullActionFunction)

ONEVENT(VIDEO_OUT_CHANGELAYOUT_TIMEOUT ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnTimerChangeLayoutCHANGELAYOUT)
ONEVENT(VIDEO_OUT_CHANGELAYOUT_TIMEOUT ,DISCONNECTING ,CBridgePartyVideoOut::NullActionFunction)

ONEVENT(UPDATE_DECODER_DETECTED_MODE ,SETUP ,CBridgePartyVideoOut::OnVideoBridgeUpdateDecoderDetectedModeSETUP)
ONEVENT(UPDATE_DECODER_DETECTED_MODE ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgeUpdateDecoderDetectedModeCONNECTED)
ONEVENT(UPDATE_DECODER_DETECTED_MODE ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgeUpdateDecoderDetectedModeCHANGELAYOUT)

ONEVENT(SLIDE_INTRA_TOUT ,SLIDE ,CBridgePartyVideoOut::OnTimerSlideIntraSLIDE)
ONEVENT(SLIDE_INTRA_TOUT ,ANYCASE ,CBridgePartyVideoOut::NullActionFunction)

ONEVENT(DISPLAY_TEXT_ON_SCREEN ,SETUP ,CBridgePartyVideoOut::NullActionFunction)
ONEVENT(DISPLAY_TEXT_ON_SCREEN ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgeDisplayTextOnScreenCONNECT)
ONEVENT(DISPLAY_TEXT_ON_SCREEN ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgeDisplayTextOnScreenCHANGE_LAYOUT)
ONEVENT(DISPLAY_TEXT_ON_SCREEN ,DISCONNECTING ,CBridgePartyVideoOut::NullActionFunction)

ONEVENT(TEXT_ON_SCREEN_DISPLAY_OFF ,IDLE ,CBridgePartyVideoOut::NullActionFunction)
ONEVENT(TEXT_ON_SCREEN_DISPLAY_OFF ,SETUP ,CBridgePartyVideoOut::NullActionFunction)
ONEVENT(TEXT_ON_SCREEN_DISPLAY_OFF ,SLIDE ,CBridgePartyVideoOut::NullActionFunction)
ONEVENT(TEXT_ON_SCREEN_DISPLAY_OFF ,CONNECTED ,CBridgePartyVideoOut::OnTextDisplayToutCONNECTED)
ONEVENT(TEXT_ON_SCREEN_DISPLAY_OFF ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnTextDisplayToutCHANGE_LAYOUT)
ONEVENT(TEXT_ON_SCREEN_DISPLAY_OFF ,DISCONNECTING ,CBridgePartyVideoOut::NullActionFunction)

ONEVENT(UPDATE_VIDEO_CLARITY ,IDLE ,CBridgePartyVideoOut::OnVideoBridgePartyUpdateVideoClarityIDLE)
ONEVENT(UPDATE_VIDEO_CLARITY ,SLIDE ,CBridgePartyVideoOut::OnVideoBridgePartyUpdateVideoClaritySLIDE)
ONEVENT(UPDATE_VIDEO_CLARITY ,SETUP ,CBridgePartyVideoOut::OnVideoBridgePartyUpdateVideoClaritySETUP)
ONEVENT(UPDATE_VIDEO_CLARITY ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgePartyUpdateVideoClarityCONNECTED)
ONEVENT(UPDATE_VIDEO_CLARITY ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgePartyUpdateVideoClarityCHANGE_LAYOUT)
ONEVENT(UPDATE_VIDEO_CLARITY ,DISCONNECTING ,CBridgePartyVideoOut::OnVideoBridgePartyUpdateVideoClarityDISCONNECTING)

ONEVENT(UPDATE_PARTY_TELEPRESENCE_MODE ,IDLE ,CBridgePartyVideoOut::OnVideoBridgeUpdatePartyTelePresenceModeIDLE)
ONEVENT(UPDATE_PARTY_TELEPRESENCE_MODE ,SLIDE ,CBridgePartyVideoOut::OnVideoBridgeUpdatePartyTelePresenceModeSLIDE)
ONEVENT(UPDATE_PARTY_TELEPRESENCE_MODE ,SETUP ,CBridgePartyVideoOut::OnVideoBridgeUpdatePartyTelePresenceModeSETUP)
ONEVENT(UPDATE_PARTY_TELEPRESENCE_MODE ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgeUpdatePartyTelePresenceModeCONNECTED)
ONEVENT(UPDATE_PARTY_TELEPRESENCE_MODE ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgeUpdatePartyTelePresenceModeCHANGE_LAYOUT)
ONEVENT(UPDATE_PARTY_TELEPRESENCE_MODE ,DISCONNECTING ,CBridgePartyVideoOut::OnVideoBridgeUpdatePartyTelePresenceModeDISCONNECTING)

ONEVENT(SET_MESSAGE_OVERLAY ,IDLE ,CBridgePartyVideoOut::NullActionFunction)
ONEVENT(SET_MESSAGE_OVERLAY ,SLIDE ,CBridgePartyVideoOut::NullActionFunction)
ONEVENT(SET_MESSAGE_OVERLAY ,SETUP ,CBridgePartyVideoOut::NullActionFunction)
ONEVENT(SET_MESSAGE_OVERLAY ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgeUpdateMessageOverlayCONNECTED)
ONEVENT(SET_MESSAGE_OVERLAY ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgeUpdateMessageOverlayCHANGE_LAYOUT)
ONEVENT(SET_MESSAGE_OVERLAY ,DISCONNECTING ,CBridgePartyVideoOut::NullActionFunction)

ONEVENT(SET_HIDE_TEXT_BOX ,IDLE ,CBridgePartyVideoOut::NullActionFunction) //VNGR-15750
ONEVENT(SET_HIDE_TEXT_BOX ,SLIDE ,CBridgePartyVideoOut::NullActionFunction)
ONEVENT(SET_HIDE_TEXT_BOX ,SETUP ,CBridgePartyVideoOut::NullActionFunction)
ONEVENT(SET_HIDE_TEXT_BOX ,CONNECTED ,CBridgePartyVideoOut::SendStopTextDisplay)
ONEVENT(SET_HIDE_TEXT_BOX ,CHANGE_LAYOUT ,CBridgePartyVideoOut::SendStopTextDisplay)
ONEVENT(SET_HIDE_TEXT_BOX ,DISCONNECTING ,CBridgePartyVideoOut::NullActionFunction)

ONEVENT(CHANGE_SPEAKER_NOTATION_PCM_FECC ,IDLE ,CBridgePartyVideoOut::OnVideoBridgeChangeSpeakerNotationForPcmFeccIDLE)
ONEVENT(CHANGE_SPEAKER_NOTATION_PCM_FECC ,SETUP ,CBridgePartyVideoOut::OnVideoBridgeChangeSpeakerNotationForPcmFeccSETUP)
ONEVENT(CHANGE_SPEAKER_NOTATION_PCM_FECC ,SLIDE ,CBridgePartyVideoOut::OnVideoBridgeChangeSpeakerNotationForPcmFeccSLIDE)
ONEVENT(CHANGE_SPEAKER_NOTATION_PCM_FECC ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgeChangeSpeakerNotationForPcmFeccCONNECTED)
ONEVENT(CHANGE_SPEAKER_NOTATION_PCM_FECC ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgeChangeSpeakerNotationForPcmFeccCHANGE_LAYOUT)
ONEVENT(CHANGE_SPEAKER_NOTATION_PCM_FECC ,DISCONNECTING ,CBridgePartyVideoOut::OnVideoBridgeChangeSpeakerNotationForPcmFeccDISCONNECTING)

ONEVENT(ENCODER_UPDATE_PARAM_TOUT ,IDLE ,CBridgePartyVideoOut::NullActionFunction)
ONEVENT(ENCODER_UPDATE_PARAM_TOUT ,SLIDE ,CBridgePartyVideoOut::NullActionFunction)
ONEVENT(ENCODER_UPDATE_PARAM_TOUT ,SETUP ,CBridgePartyVideoOut::NullActionFunction)
ONEVENT(ENCODER_UPDATE_PARAM_TOUT ,CONNECTED ,CBridgePartyVideoOut::OnTimerEncoderUpdateCONNECTED)
ONEVENT(ENCODER_UPDATE_PARAM_TOUT ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnTimerEncoderUpdateCHANGE_LAYOUT)
ONEVENT(ENCODER_UPDATE_PARAM_TOUT ,DISCONNECTING ,CBridgePartyVideoOut::NullActionFunction)

ONEVENT(VENUS_SETPARTYPRIVATELAYOUTTYPE ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgePartyVENUS_SetPrivateLayoutTypeCONNECTED)
ONEVENT(VENUS_SETPARTYPRIVATELAYOUTTYPE ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgePartyVENUS_SetPrivateLayoutTypeCHANGELAYOUT)

ONEVENT(ENCODER_RECURRENT_INTRA_REQ_TIMEOUT,CONNECTED ,CBridgePartyVideoOut::OnTimerRecurrentIntraRequest)
ONEVENT(ENCODER_RECURRENT_INTRA_REQ_TIMEOUT,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnTimerRecurrentIntraRequest)
ONEVENT(ENCODER_RECURRENT_INTRA_REQ_TIMEOUT,DISCONNECTING ,CBridgePartyVideoOut::NullActionFunction)

ONEVENT(AUTO_SCAN_TIMER ,IDLE ,CBridgePartyVideoOut::NullActionFunction)
ONEVENT(AUTO_SCAN_TIMER ,SLIDE ,CBridgePartyVideoOut::NullActionFunction)
ONEVENT(AUTO_SCAN_TIMER ,SETUP ,CBridgePartyVideoOut::NullActionFunction)
ONEVENT(AUTO_SCAN_TIMER ,CONNECTED ,CBridgePartyVideoOut::OnAutoScanTimerCONNECTED)
ONEVENT(AUTO_SCAN_TIMER ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnAutoScanTimerCHANGE_LAYOUT)
ONEVENT(AUTO_SCAN_TIMER ,DISCONNECTING ,CBridgePartyVideoOut::NullActionFunction)

ONEVENT(SET_AUTOSCAN_ORDER ,IDLE ,CBridgePartyVideoOut::NullActionFunction)
ONEVENT(SET_AUTOSCAN_ORDER ,SETUP ,CBridgePartyVideoOut::OnConfSetAutoScanOrderSETUP)
ONEVENT(SET_AUTOSCAN_ORDER ,SLIDE ,CBridgePartyVideoOut::OnConfSetAutoScanOrderSLIDE)
ONEVENT(SET_AUTOSCAN_ORDER ,CONNECTED ,CBridgePartyVideoOut::OnConfSetAutoScanOrderCONNECTED)
ONEVENT(SET_AUTOSCAN_ORDER ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnConfSetAutoScanOrderCHANGE_LAYOUT)
ONEVENT(SET_AUTOSCAN_ORDER ,DISCONNECTING ,CBridgePartyVideoOut::NullActionFunction)

ONEVENT(SET_SITE_NAME ,IDLE ,CBridgePartyVideoOut::OnVideoBridgePartyChangeSiteNameIDLE)
ONEVENT(SET_SITE_NAME ,SLIDE ,CBridgePartyVideoOut::OnVideoBridgePartyChangeSiteNameSLIDE)
ONEVENT(SET_SITE_NAME ,SETUP ,CBridgePartyVideoOut::OnVideoBridgePartyChangeSiteNameSETUP)
ONEVENT(SET_SITE_NAME ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgePartyChangeSiteNameCONNECTED)
ONEVENT(SET_SITE_NAME ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgePartyChangeSiteNameCHANGELAYOUT)

ONEVENT(REFRESHLAYOUT ,IDLE ,CBridgePartyVideoOut::OnVideoBridgePartyRefreshLayoutIDLE)
ONEVENT(REFRESHLAYOUT ,SLIDE ,CBridgePartyVideoOut::OnVideoBridgePartyRefreshLayoutSLIDE)
ONEVENT(REFRESHLAYOUT ,SETUP ,CBridgePartyVideoOut::OnVideoBridgePartyRefreshLayoutSETUP)
ONEVENT(REFRESHLAYOUT ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgePartyRefreshLayoutCONNECTED)
ONEVENT(REFRESHLAYOUT ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgePartyRefreshLayoutCHANGELAYOUT)

ONEVENT(INDICATION_ICONS_CHANGE ,ANYCASE ,CBridgePartyVideoOut::NullActionFunction)
ONEVENT(INDICATION_ICONS_CHANGE ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgePartyIndicationIconsChangeCONNECTED)
ONEVENT(INDICATION_ICONS_CHANGE ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgePartyIndicationIconsChangeCONNECTED)

ONEVENT(PARTY_AUDIO_NUMBER_INDICATION_DISPLAY_TOUT, CONNECTED, CBridgePartyVideoOut::OnTimerAudioNumberHiden)

ONEVENT(UPDATEONIMAGESVCTOAVC ,SETUP ,CBridgePartyVideoOut::NullActionFunction)
ONEVENT(UPDATEONIMAGESVCTOAVC ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgePartyUpdateOnImageSvcToAvcCONNECTED)
ONEVENT(UPDATEONIMAGESVCTOAVC ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgePartyUpdateOnImageSvcToAvcCHANGELAYOUT)
ONEVENT(UPDATEONIMAGESVCTOAVC ,DISCONNECTING ,CBridgePartyVideoOut::NullActionFunction)

ONEVENT(UPDATE_LAYOUT_HANDLER_TYPE ,IDLE ,CBridgePartyVideoOut::OnVideoBridgePartyUpdateLayoutHandlerTypeIDLE)
ONEVENT(UPDATE_LAYOUT_HANDLER_TYPE ,SLIDE ,CBridgePartyVideoOut::OnVideoBridgePartyUpdateLayoutHandlerTypeSLIDE)
ONEVENT(UPDATE_LAYOUT_HANDLER_TYPE ,SETUP ,CBridgePartyVideoOut::OnVideoBridgePartyUpdateLayoutHandlerTypeSETUP)
ONEVENT(UPDATE_LAYOUT_HANDLER_TYPE ,CONNECTED ,CBridgePartyVideoOut::OnVideoBridgePartyUpdateLayoutHandlerTypeCONNECTED)
ONEVENT(UPDATE_LAYOUT_HANDLER_TYPE ,CHANGE_LAYOUT ,CBridgePartyVideoOut::OnVideoBridgePartyUpdateLayoutHandlerTypeCHANGELAYOUT)
ONEVENT(UPDATE_LAYOUT_HANDLER_TYPE ,DISCONNECTING ,CBridgePartyVideoOut::NullActionFunction)
PEND_MESSAGE_MAP(CBridgePartyVideoOut,CBridgePartyMediaUniDirection)
;

////////////////////////////////////////////////////////////////////////////
CBridgePartyVideoOut::CBridgePartyVideoOut() :
				CBridgePartyVideoUniDirection()
{
	m_state = IDLE;
	m_decoderDetectedModeWidth = DEFAULT_DECODER_DETECTED_MODE_WIDTH;
	m_decoderDetectedModeHeight = DEFAULT_DECODER_DETECTED_MODE_HEIGHT;
	m_decoderDetectedSampleAspectRatioWidth = DEFAULT_DECODER_DETECTED_SAMPLE_ASPECT_RATIO_WIDTH;
	m_decoderDetectedSampleAspectRatioHeight = DEFAULT_DECODER_DETECTED_SAMPLE_ASPECT_RATIO_HEIGHT;
	m_tmpSpeakerNotationForPcmFeccImageId = INVALID;

	m_layoutChangedWhileWaitingForAck = false;
	m_visualEffectsChangedWhileWaitingForAck = false;
	m_resolutionChangedWhileWaitingForAck = false;
	m_imageResolutionChangedWhileWaitingForAck = false;
	m_sitenameChangedWhileWaitingForAck = false;

	for (int i = CP_LAYOUT_1X1; i < CP_NO_LAYOUT; ++i)
		m_pReservation[i] = NULL;

	for (int i = CP_LAYOUT_1X1; i < CP_NO_LAYOUT; ++i)
		m_pPrivateReservation[i] = NULL;

	m_pCurrHandler = NULL;
	m_pCurrentView = NULL;
	m_PrivatelayoutType = CP_NO_LAYOUT;
	m_isPrivate = false;
	m_isPrivateChanged = false;
	m_pPartyVisualEffects = NULL;
	m_pPartyPcmFeccVisualEffects = NULL;
	m_partyLectureModeRole = eREGULAR;
	m_pLastSpeakerNotationParty = NULL;
	m_eTelePresenceMode = eTelePresencePartyNone;
	m_videoQuality = eVideoQualitySharpness; // Sharpness is the default value
	m_videoConfType = eVideoConfTypeCP;
	m_isForce1x1 = false;
	m_bEnableReStopGathering = true;
	m_waitForUpdateEncoderAck = false;
	m_isSiteNamesEnabled = true;
	m_isLprActive = false;
	m_pSiteNameInfo = NULL;
	m_bUseIntermediateSDResolution = false;
	m_bEncodeBFramesInRTV = false;
	m_dwFrThreshold = 0;
	m_fontType = ftDefault;

	m_nTimerFastUpdateReq = 0;
	m_telepresenceModeChanged = false;
	m_layoutTPRoomSublinkChangedWhileWaitingForAck = false;

	m_recordingType = E_ICON_REC_OFF;

	m_isAudioParticipantsIconActived = NO;
	m_numAudioParticipantsInConf = 0;
	m_isAudioIconToSentAfterOpenPort = NO;
	m_isInGatheringMode = NO;

	m_bIsFollowSpeakerOn1X1 = NO;

  VALIDATEMESSAGEMAP;
}

//--------------------------------------------------------------------------
CBridgePartyVideoOut::~CBridgePartyVideoOut()
{
	POBJDELETE(m_pCurrHandler);
	POBJDELETE(m_pCurrentView);

	for (int i = CP_LAYOUT_1X1; i < CP_NO_LAYOUT; ++i)
		POBJDELETE(m_pReservation[i]);

	for (int i = CP_LAYOUT_1X1; i < CP_NO_LAYOUT; ++i)
		POBJDELETE(m_pPrivateReservation[i]);

	POBJDELETE(m_pPartyVisualEffects);
	POBJDELETE(m_pPartyPcmFeccVisualEffects);
	POBJDELETE(m_pSiteNameInfo);

	//Change Layout Improvement - Layout Shared Memory (CL-SM)
	RemoveFromLayoutSharedMemory();

	RemoveFromIndicationIconSharedMemory();
	
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::CleanAllLayouts()
{
	for (int i = CP_LAYOUT_1X1; i < CP_NO_LAYOUT; ++i)
		m_pReservation[i]->CleanUp();

	for (int i = CP_LAYOUT_1X1; i < CP_NO_LAYOUT; ++i)
		m_pPrivateReservation[i]->CleanUp();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::CleanAllLayoutsAndPrivateSettings()
{
	CleanAllLayouts();
	m_PrivatelayoutType = CP_NO_LAYOUT;
	SetIsPrivateLayout(false);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::Create(const CBridgePartyCntl* pBridgePartyCntl, const CRsrcParams* pRsrcParams,
				const CBridgePartyMediaParams* pBridgePartyVideoOutParams)
{
	CBridgePartyVideoUniDirection::Create(pBridgePartyCntl, pRsrcParams, pBridgePartyVideoOutParams);

	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	CBridgePartyVideoOutParams* pVideoOutParams = (CBridgePartyVideoOutParams*) pBridgePartyVideoOutParams;

	if (pVideoOutParams->GetVisualEffects())
	{
		m_pPartyVisualEffects = new CVisualEffectsParams(*pVideoOutParams->GetVisualEffects());
	}
	else
	{
		m_pPartyVisualEffects = new CVisualEffectsParams();
	}

	ModifyVisuallEffectsForAVMCUParty(m_pPartyVisualEffects);

	ModifyVisuallEffectsForAVMCUParty(m_pPartyVisualEffects);

	m_pSiteNameInfo = new CSiteNameInfo();

	CreatePartyPcmFeccVisualEffects();

	for (int i = CP_LAYOUT_1X1; i < CP_NO_LAYOUT; ++i)
	{
		POBJDELETE(m_pReservation[i]);
		m_pReservation[i] = new CLayout((LayoutType) i, m_pBridgePartyCntl->GetConfName());
	}

	CLayout* pActivePrivateLayout = NULL;

	for (int i = CP_LAYOUT_1X1; i < CP_NO_LAYOUT; ++i)
	{
		POBJDELETE(m_pPrivateReservation[i]);
		m_pPrivateReservation[i] = new CLayout((LayoutType) i, m_pBridgePartyCntl->GetConfName());
		CLayout* pLocallayout = pVideoOutParams->GetPrivateReservationLayout(i);

		if (pLocallayout)
		{
			*m_pPrivateReservation[i] = *pLocallayout;

			if (m_pPrivateReservation[i]->isActive())
			{
				pActivePrivateLayout = m_pPrivateReservation[i];

				TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", Active Private layout discovered:" << *pActivePrivateLayout;
			}
		}
	}

	m_partyLectureModeRole = pVideoOutParams->GetPartyLectureModeRole();
	m_pCurrentView = new CLayout(GetConfLayoutType(), m_pBridgePartyCntl->GetConfName());

	// Romem 26.5.08 - see if force layout 1*1 during Cascade Link Establishment is permitted in system.cfg
	BOOL bEable1X1LayoutInCascadeLinkEstablishment = true;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("FORCE_1X1_LAYOUT_ON_CASCADED_LINK_CONNECTION",
					bEable1X1LayoutInCascadeLinkEstablishment);

	if (bEable1X1LayoutInCascadeLinkEstablishment == false)
		m_isForce1x1 = false;
	else
		m_isForce1x1 = pVideoOutParams->GetIsCascadeLink();

	if (m_isForce1x1)
		SetPrivateLayoutForParty(CP_LAYOUT_1X1);

	SetLayoutHandler();

	if (pActivePrivateLayout)
	{
		if (RejectChangeLayoutRequestBecauseOfApplications())
		{
			TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Reject Party Layout from reservation";
		}
		else
		{
			m_PrivatelayoutType = pActivePrivateLayout->GetLayoutType();
			SetIsPrivateLayout(true);
			pActivePrivateLayout->SetCurrActiveLayout(YES);
		}
	}

	CVideoBridge* pBridge = (CVideoBridge*) (m_pBridgePartyCntl->GetBridge());

	DWORD croppingHor, croppingVer;
	pBridge->GetCroppingValues(croppingHor, croppingVer);

	m_croppingVer = croppingVer;
	m_croppingHor = croppingHor;
	m_eTelePresenceMode = pVideoOutParams->GetTelePresenceMode();
	m_videoQuality = pVideoOutParams->GetVideoQualityType();
	m_eResolutionTableType = pVideoOutParams->GetVideoResolutionTableType();
	m_isSiteNamesEnabled = pVideoOutParams->GetIsSiteNamesEnabled();
	m_bUseIntermediateSDResolution = pVideoOutParams->GetUseIntermediateSDResolution();
	m_bEncodeBFramesInRTV = pVideoOutParams->GetIsEncodeRTVBFrame();
	m_fontType = GetFontTypeFromConf();
	m_dwFrThreshold = ((CBridgePartyVideoOutParams*) pBridgePartyVideoOutParams)->GetFrThreshold();

	//FSN-489, followSpeaker is enabled for HW MCU when 1)System flag is YES 2) System flag is AUTO and all layout icon is disabled.
	if (eProductFamilySoftMcu != CProcessBase::GetProcess()->GetProductFamily())
	{
		const CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
		if (pSysConfig)
		{
			std::string followSpeakerOn1x1;
			pSysConfig->GetDataByKey(CFG_KEY_FOLLOW_SPEAKER_ON_1X1, followSpeakerOn1x1);

			if (followSpeakerOn1x1 == "YES")
				m_bIsFollowSpeakerOn1X1 = YES;
			else if (followSpeakerOn1x1 == "AUTO")
			{
				CConf* pConf = pBridge->GetConf();
				PASSERT_AND_RETURN(!pConf);
				const CCommConf* pCommConf = pConf->GetCommConf();
				PASSERT_AND_RETURN(!pCommConf);
				if (!(pCommConf->GetEnableAudioParticipantsIcon() || pCommConf->GetEnableSelfNetworkQualityIcon()
								|| pCommConf->GetEnableRecordingIcon()))
					m_bIsFollowSpeakerOn1X1 = YES;
			}
		}

	}

}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::SetLayoutHandler()
{

	POBJDELETE(m_pCurrHandler);

	CVideoBridge* pVideoBridge = (CVideoBridge*) (m_pBridgePartyCntl->GetBridge());
	CTelepresenseEPInfo telepresenceEPInfo = ((CVideoBridgePartyCntl*) m_pBridgePartyCntl)->GetTelepresenceInfo();

	BOOL bTelepresenceOnOff = pVideoBridge->GetTelepresenceOnOff();
	BOOL bManageTelepresenceLayoutsInternally = pVideoBridge->GetManageTelepresenceLayoutsInternally();
	BOOL linkRole = telepresenceEPInfo.GetLinkRole();
	ETelePresenceLayoutMode telepresenceLayoutMode = ((CVideoBridgePartyCntl*) m_pBridgePartyCntl)->GetTelepresenceLayoutMode();

	TRACEINTO << "PartyRsrcId: " << m_pBridgePartyCntl->GetPartyRsrcID() << "\nTelepresenceOnOff: " << (WORD) bTelepresenceOnOff
					<< "\nManageTelepresenceLayoutsInternally: " << (WORD) bManageTelepresenceLayoutsInternally << "\nLinkRoll: "
					<< (WORD) telepresenceEPInfo.GetLinkRole() << "\ttelepresenceLayoutMode: "
					<< TelePresenceLayoutModeToString(telepresenceLayoutMode);

	if (!bTelepresenceOnOff || !bManageTelepresenceLayoutsInternally || // Telepresence = OFF or manage Telepresence layouts by MLA
					telepresenceEPInfo.GetLinkRole() == 1 || m_isForce1x1) // Telepresence links get the regular CLayoutHandler
	{
		m_pCurrHandler = new CLayoutHandler(pVideoBridge, (CVideoBridgePartyCntl*) m_pBridgePartyCntl);
		TRACEINTO << " set CLayoutHandler (regular), PartyRsrcId: " << m_pBridgePartyCntl->GetPartyRsrcID();
	}
	else	// Telepresence = ON and manage Telepresence layouts internally
	{
		switch (telepresenceLayoutMode)
		{
			case eTelePresenceLayoutRoomSwitch:
			{
				m_pCurrHandler = new CLayoutHandlerTelepresenceRoomSwitch(pVideoBridge, (CVideoBridgePartyCntl*) m_pBridgePartyCntl);
				TRACEINTO << "TELEPRESENCE_LAYOUTS_DEBUG set CLayoutHandlerTelepresenceRoomSwitch, PartyRsrcId: " << m_pBridgePartyCntl->GetPartyRsrcID();
				break;
			}
			case eTelePresenceLayoutCpSpeakerPriority:
				m_pCurrHandler = new CLayoutHandlerTelepresenceSpeakerModeCP(pVideoBridge, (CVideoBridgePartyCntl*) m_pBridgePartyCntl);
				TRACEINTO << " TELEPRESENCE_LAYOUTS_DEBUG set CLayoutHandlerTelepresenceSpeakerModeCP, PartyRsrcId: " << m_pBridgePartyCntl->GetPartyRsrcID();
				break;

			case eTelePresenceLayoutCpParticipantsPriority:
			{
				m_pCurrHandler = new CLayoutHandlerTelepresencePartyModeCP(pVideoBridge, (CVideoBridgePartyCntl*) m_pBridgePartyCntl);
				TRACEINTO << " TELEPRESENCE_LAYOUTS_DEBUG set CLayoutHandlerTelepresencePartyModeCP, PartyRsrcId: " << m_pBridgePartyCntl->GetPartyRsrcID();
				break;
			}
			case eTelePresenceLayoutManual:
			case eTelePresenceLayoutContinuousPresence:
			default:
			{
				m_pCurrHandler = new CLayoutHandler(pVideoBridge, (CVideoBridgePartyCntl*) m_pBridgePartyCntl);
				TRACEINTO << " telepresenceLayoutMode not managed by MCU set CLayoutHandler (regular), PartyRsrcId: "
								<< m_pBridgePartyCntl->GetPartyRsrcID();
			}
		}
	}
}
//--------------------------------------------------------------------------
void CBridgePartyVideoOut::Connect()
{
	DispatchEvent(CONNECT_VIDEO_OUT, NULL);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::DisConnect()
{
	DispatchEvent(DISCONNECT_VIDEO_OUT, NULL);
}

//--------------------------------------------------------------------------
// BuildLayout() Parameters:
// * bSendChangeLayoutAlways: will always send the new layout to hardware - default = false
// * bUseSharedMemForChangeLayoutReq: use shared memory mechanism for change layout request (Change Layout Improvement - CL-SM)
// 	 - update Layout shared memory and send a collective request to MplApi with all relevant Layout Ids (currently partyRsrcId)
// 	 (MplApi will get the change layout parameters from the shared memory for each party in list and send a request per id in list).
BYTE CBridgePartyVideoOut::BuildLayout(BOOL bUseSharedMemForChangeLayoutReq, BYTE bSendChangeLayoutAlways)
{
	DeleteTimer(AUTO_SCAN_TIMER );

	bool isNotationChanged = false;
	if (m_pLastSpeakerNotationParty != GetLastActiveAudioSpeakerRequestFromBridge())
	{
		isNotationChanged = true;
		m_pLastSpeakerNotationParty = GetLastActiveAudioSpeakerRequestFromBridge();
	}

	LayoutType oldLayoutType = m_pCurrentView->GetLayoutType();

	CVideoBridge* pVideoBridge = (CVideoBridge*) (m_pBridgePartyCntl->GetBridge());
	PASSERT_AND_RETURN_VALUE(!pVideoBridge, false);

	BOOL isManagedByEmbeddedMLA = (pVideoBridge->GetManageTelepresenceLayoutsInternally() && pVideoBridge->GetTelepresenceOnOff());
	const CTelepresenseEPInfo& telepresenceEPInfo = ((CVideoBridgePartyCntl*) m_pBridgePartyCntl)->GetTelepresenceInfo();
	ETelePresenceLayoutMode tpMode = ((CVideoBridgePartyCntl*) m_pBridgePartyCntl)->GetTelepresenceLayoutMode();

	if (telepresenceEPInfo.GetLinkNum() != 0
					&& (tpMode == eTelePresenceLayoutCpSpeakerPriority || tpMode == eTelePresenceLayoutCpParticipantsPriority))
	{
		if (m_layoutTPRoomSublinkChangedWhileWaitingForAck) //BRIDGE-14793 - CL sent to sublink before ACK of update encoder req arrived, so need to resend
		{
			TRACEINTO << "EMB_MLA_OLGA - partyId:" << m_pBridgePartyCntl->GetPartyRsrcID()
                      << " - need to send change layout request for this sublink party because it was blocked before";

			BOOL bSent = false;
			SendChangeLayoutToHardware(bUseSharedMemForChangeLayoutReq, false, &bSent);
			if (bSent)
				m_layoutTPRoomSublinkChangedWhileWaitingForAck = false;
		}
		else
			TRACEINTO << "EMB_MLA_OLGA - partyId:" << m_pBridgePartyCntl->GetPartyRsrcID()
                      << " - don't build layout because we act only on the main link party for telepresence parties";

		return false;
	}

	bool isLayoutChanged = m_pCurrHandler->BuildLayout();

	LayoutType newLayoutType = GetPartyCurrentLayoutType();	//added for BRIDGE-13167
	bool bSendStartGathering = false;

	// VNGR-21714 - resolution change on a cascade link doesn't initiate a layout change - check resolution.
	DWORD prevZeroCellImageId = (oldLayoutType == CP_LAYOUT_1X1) ? m_pCurrentView->GetSubImageNum(0)->GetImageId() : 0;
	DWORD currZeroCellImageId = (newLayoutType == CP_LAYOUT_1X1) ? m_pCurrentView->GetSubImageNum(0)->GetImageId() : 0;

	BOOL isCascadeLink1x1Layout = (currZeroCellImageId != 0) && (prevZeroCellImageId != 0)
					&& (m_pBridgePartyCntl->GetCascadeLinkMode() != NONE);
	if (isCascadeLink1x1Layout)
	{
		CPartyImageLookupTable* pPartyImageLookupTable = GetPartyImageLookupTable();

		CImage* pPrevZeroCellImage = pPartyImageLookupTable->GetPartyImage(prevZeroCellImageId);
		PASSERTSTREAM(!pPrevZeroCellImage, "PartyId:" << prevZeroCellImageId);

		CImage* pCurrZeroCellImage = pPartyImageLookupTable->GetPartyImage(currZeroCellImageId);
		PASSERTSTREAM(!pCurrZeroCellImage, "PartyId:" << currZeroCellImageId);

		if (pPrevZeroCellImage && pCurrZeroCellImage)
			isLayoutChanged |= !pCurrZeroCellImage->CompareDecoderDetectedParams((const CImage&) *pPrevZeroCellImage);
	}

	if (isLayoutChanged || bSendChangeLayoutAlways)
	{
		if (!bUseSharedMemForChangeLayoutReq)
		{
			SendChangeLayoutToHardware();
		}
		else	//Change Layout Improvement - Layout Shared Memory (CL-SM)
		{
			BOOL bSent = false;
			SendChangeLayoutToHardware(bUseSharedMemForChangeLayoutReq, false, &bSent);

			if (bSent)
			{
				// Update list of changed layouts in VideoBridge
				CVideoBridge* pVideoBridge = (CVideoBridge*) (m_pBridgePartyCntl->GetBridge());
				pVideoBridge->AddChangedLayoutIdToVector(m_pBridgePartyCntl->GetPartyRsrcID());	//currently we use party resource ID for layout ID
			}
		}

		if ((oldLayoutType == CP_LAYOUT_1X1 || newLayoutType == CP_LAYOUT_1X1) && oldLayoutType != newLayoutType)
			bSendStartGathering = true;
	}
	else if (isNotationChanged)
	{
		SendChangeLayoutAttributesToHardware(); // Video speaker timer in the VideoCard invoked the msg
	}

	std::ostringstream msg;
	msg << "CBridgePartyVideoOut::BuildLayout:" << "\n  ConfName                       :" << m_pBridgePartyCntl->GetFullName()
					<< "\n  oldLayoutType                  :" << LayoutTypeAsString[oldLayoutType]
					<< "\n  newLayoutType                  :" << LayoutTypeAsString[newLayoutType]
					<< "\n  bSendChangeLayoutAlways        :" << (bSendChangeLayoutAlways ? "YES" : "NO")
					<< "\n  bSendStartGathering            :" << (bSendStartGathering ? "YES" : "NO");

	PTRACE(eLevelInfoNormal, msg.str().c_str());

	if ((isLayoutChanged == true) || (true == m_isPrivateChanged))
	{
		m_pBridgePartyCntl->HandleEvent(NULL, 0, PARTYLAYOUTCHANGED);
		m_isPrivateChanged = false;
	}

	if (bSendStartGathering)
	{
		CGathering* pGathering = GetGathering();
		if (pGathering)
		{
			bool bIsNeedFullRendering = pGathering->IsNeedFullRendering(m_pBridgePartyCntl->GetName());
			pGathering->SetIsNeedFullRendering(true, m_pBridgePartyCntl->GetName());
			((CVideoHardwareInterface*) m_pHardwareInterface)->SendGatheringToDisplay(pGathering, m_pBridgePartyCntl->GetName());

			if (pGathering->IsEndGathering())
				m_isInGatheringMode = NO;

			pGathering->SetIsNeedFullRendering(bIsNeedFullRendering, m_pBridgePartyCntl->GetName());

		}
	}

	if (m_pCurrHandler->GetAutoScanTimeOut() /*&& (oldLayoutType != newLayoutType)*/)
		StartTimer(AUTO_SCAN_TIMER, m_pCurrHandler->GetAutoScanTimeOut() * SECOND);

	return isLayoutChanged;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnAutoScanTimerCONNECTED(CSegment* pParam)
{
	std::ostringstream msg;
	msg << "CBridgePartyVideoOut::OnAutoScanTimerCONNECTED - Dump of 'm_pAutoScanParams':";
	msg << "\n  PartyName      :" << m_pBridgePartyCntl->GetName();
	m_pCurrHandler->DumpImagesVectorAutoScan(msg);
	PTRACE(eLevelInfoNormal, msg.str().c_str());

	// VNGR-25709 fix - if there are no or one party, the layout doesn't change so we just return.
	if (m_pCurrHandler->GetImagesVectorAutoScanSize() <= 1)
	{
		if (m_pCurrHandler->GetAutoScanTimeOut())
			StartTimer(AUTO_SCAN_TIMER, m_pCurrHandler->GetAutoScanTimeOut() * SECOND);
		return;
	} // VNGR-25709 fix END

	//~~ Create copy of current layout before changing it
	CLayout* pPreviouseLayout = new CLayout(*m_pCurrentView);

	m_pCurrHandler->FillAutoScanImageInLayout(*m_pCurrentView);
	if (*pPreviouseLayout != *m_pCurrentView)
	{
		SendChangeLayoutToHardware();
		m_pBridgePartyCntl->HandleEvent(NULL, 0, PARTYLAYOUTCHANGED);
		m_isPrivateChanged = false;
	}

	if (m_pCurrHandler->GetAutoScanTimeOut())
		StartTimer(AUTO_SCAN_TIMER, m_pCurrHandler->GetAutoScanTimeOut() * SECOND);

	POBJDELETE(pPreviouseLayout);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnAutoScanTimerCHANGE_LAYOUT(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	ON(m_layoutChangedWhileWaitingForAck);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnConfSetAutoScanOrderSETUP(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	OnConfSetAutoScanOrder(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnConfSetAutoScanOrderSLIDE(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	OnConfSetAutoScanOrder(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnConfSetAutoScanOrderCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	OnConfSetAutoScanOrder(pParam);
	// if scan order has been changed while auto scan is active
	// start change layout again in order to set new images in auto scan cell
	if (IsValidTimer(AUTO_SCAN_TIMER ))
	{
		BOOL bUseSharedMemForChangeLayoutReq = true;
		BuildLayout(bUseSharedMemForChangeLayoutReq);
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnConfSetAutoScanOrderCHANGE_LAYOUT(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	OnConfSetAutoScanOrder(pParam);
	ON(m_layoutChangedWhileWaitingForAck);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnConfSetAutoScanOrder(CSegment* pParam)
{
	CAutoScanOrder* pAutoScanOrder = new CAutoScanOrder;
	pAutoScanOrder->DeSerialize(NATIVE, *pParam);

	if (m_pCurrHandler)
		m_pCurrHandler->SetAutoScanOrder(pAutoScanOrder);

	POBJDELETE(pAutoScanOrder);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::UpdateAutoScanOrder(CAutoScanOrder* pAutoScanOrder)
{
	CSegment* pSeg = new CSegment;
	pAutoScanOrder->Serialize(NATIVE, *pSeg);
	DispatchEvent(SET_AUTOSCAN_ORDER, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::Export()
{
	// VNGR-26258 fix
	std::auto_ptr<CSegment> pSeg(new CSegment);
	*pSeg << (DWORD) 0;
	((CVideoHardwareInterface*) m_pHardwareInterface)->SendStopPLC(pSeg.get());
	// END VNGR-26258

	if (m_state == CHANGE_LAYOUT)
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName()
						<< " - Party is going to move to another conference and need to end change layout";
		m_state = CONNECTED;
		if (IsValidTimer(VIDEO_OUT_CHANGELAYOUT_TIMEOUT))
			DeleteTimer(VIDEO_OUT_CHANGELAYOUT_TIMEOUT);
	}

	if (m_state == SLIDE)
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName()
						<< " - Party is going to move to another conference and need to end SLIDE state";
		m_state = IDLE;
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::BuildLayoutAndSendOnEncoderResolutionChange()
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	BOOL bUseSharedMemForChangeLayoutReq = false;
	BYTE alwaysSendToHardware = true;
	BuildLayout(bUseSharedMemForChangeLayoutReq, alwaysSendToHardware);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::SendConnectToRtp()
{
	if (!m_pHardwareInterface)
		PASSERT_AND_RETURN(101);

	ConnectionID rtpConnectionId = DUMMY_CONNECTION_ID;
	ConnectionID encoderConnectionId = m_pHardwareInterface->GetConnectionId();
	DWORD encoderRsrcPartyId = m_pHardwareInterface->GetPartyRsrcId();
	DWORD rtpRsrcPartyId = m_pHardwareInterface->GetPartyRsrcId();

	// Eitan - ISDN party has no rtp - connect MUX instead?
	CRsrcDesc *pRsrcDesc;
	if (m_pBridgePartyCntl->GetNetworkInterface() == ISDN_INTERFACE_TYPE)
		pRsrcDesc = ::GetpConfPartyRoutingTable()->GetPartyRsrcDesc(m_pHardwareInterface->GetPartyRsrcId(), eLogical_mux);
	else
		pRsrcDesc = ::GetpConfPartyRoutingTable()->GetPartyRsrcDesc(m_pHardwareInterface->GetPartyRsrcId(), eLogical_rtp);

	if (pRsrcDesc)
		rtpConnectionId = pRsrcDesc->GetConnectionId();
	else
		DBGPASSERT(101);
	m_lastReqId = ((CVideoHardwareInterface*) m_pHardwareInterface)->SendConnect(rtpConnectionId, encoderConnectionId,
					rtpRsrcPartyId, encoderRsrcPartyId);
	m_lastReq = TB_MSG_CONNECT_REQ;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::SendOpenEncoder()
{
	PASSERT_AND_RETURN(!m_pHardwareInterface);

	m_isPortOpened = true;

	// VNGR-15880 -
	// in case current video params are lower than the actual allocated port,
	// --> send open_port with parameters of allocated port, and then send update with current params
	// (dsp can not update parameters to higher parameters than in open_port)
	FixCurrentVideoParamsAccordingToAllocationAndUpdateIfNeeded();

	((CVideoBridge*) m_pBridgePartyCntl->GetBridge())->GetCroppingValues(m_croppingHor, m_croppingVer);

	m_lastReqId = SendUpdateEncoder(true);

	m_lastReq = TB_MSG_OPEN_PORT_REQ;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::SendCloseEncoder()
{
	PASSERT_AND_RETURN(!m_pHardwareInterface);

	OFF(m_isPortOpened);
	m_lastReqId = ((CVideoHardwareInterface*) m_pHardwareInterface)->SendCloseEncoder();
	m_lastReq = TB_MSG_CLOSE_PORT_REQ;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::SendChangeLayoutToHardware(BOOL bUseSharedMemForChangeLayoutReq, BYTE isVSW, BOOL *bSent)
{
	if (bSent)
	{
		*bSent = false;
	}

	// Romem 9.7.13
	if (m_waitForUpdateEncoderAck)
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName()
						<< " - Do not send change layout to DSP on waiting to ack on update encoder from DSP";
		return;
	}
	// vngr-7017 "bombing" logger on change layout - change trace to DEBUG level
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	m_pSiteNameInfo->Dump("CBridgePartyVideoOut::SendChangeLayoutToHardware");

	PASSERT_AND_RETURN(!m_pHardwareInterface);

	// Bug fix VNGR-2080 block sending empty layout to MPL.
	if (m_pCurrentView->isLayoutEmpty())
	{
		// PTRACE2(eLevelInfoNormal,"CBridgePartyVideoOut::SendChangeLayoutToHardware : Layout is empty no need to send to HW, Name - ",m_pBridgePartyCntl->GetFullName());
		// return;
	}

	DWORD speakerPlaceInLayout = GetAudioSpeakerPlaceInLayout();

	// changeLayout timeout - debug assert and return to state connect when end
	StartTimer(VIDEO_OUT_CHANGELAYOUT_TIMEOUT, VIDEO_OUT_CHANGE_LAYOUT_TOUT);

	StartSiteNamesOffTimer();
	m_state = CHANGE_LAYOUT;
	CVideoHardwareInterface* pHWInt = (CVideoHardwareInterface*) m_pHardwareInterface;
	DWORD fs = m_FS;
	DWORD mbps = m_MBPS;
	/*
	 if (m_videoAlg == MS_SVC)
	 {
	 fs = pHWInt->GetFsForSvcLync(m_msftSvcParamsStruct.nWidth, m_msftSvcParamsStruct.nHeight);
	 mbps = (DWORD)GetMaxMbpsAsDevision((DWORD)(fs * pHWInt->TranslateVideoFrameRateToNumeric(m_videoFrameRate)));
	 fs  = GetMaxFsAsDevision(fs);
	 TRACEINTO << "MSSVC fs: " << fs << ", mbps: " << mbps;
	 }
	 */

	//get if CONF is in telepresence mode
	BYTE isTelePresenceMode = 0;
	CVideoBridge* pBridge = (CVideoBridge*) m_pBridgePartyCntl->GetBridge();
	const CCommConf* pCommConf = NULL;
	if (pBridge)
	{
		CConf* pConf = pBridge->GetConf();
		if (pConf)
		{
			pCommConf = pConf->GetCommConf();
			if (pCommConf)
			{
				isTelePresenceMode = pCommConf->GetIsTelePresenceMode();
			}
		}
	}

	pHWInt->ChangeLayoutSendOrUpdate(m_pCurrentView, m_pPartyVisualEffects, m_pSiteNameInfo, speakerPlaceInLayout,
					m_videoResolution, m_decoderDetectedModeWidth, m_decoderDetectedModeHeight,
					m_decoderDetectedSampleAspectRatioWidth, m_decoderDetectedSampleAspectRatioHeight, m_videoAlg, fs, mbps,
					m_videoConfType, m_isSiteNamesEnabled, isTelePresenceMode, bUseSharedMemForChangeLayoutReq, isVSW);

	if (bSent)
	{
		*bSent = true;
	}

	if (m_isAudioIconToSentAfterOpenPort)
	{
		if (GetNumAudioParticipantsInConf() > 0)
			SetAudioParticipantsIconActive(TRUE);
		else
			SetAudioParticipantsIconActive(FALSE);

		if (GetIsAudioParticipantsIconActive())
		{
//			CVideoBridge* pBridge = (CVideoBridge*)m_pBridgePartyCntl->GetBridge();
//			PASSERT_AND_RETURN(!pBridge);
//			CConf* pConf = pBridge->GetConf();
//			PASSERT_AND_RETURN(!pConf);
//			const CCommConf* pCommConf = pConf->GetCommConf();
			PASSERT_AND_RETURN(!pCommConf);
			if (eIconDisplayOnChange == pCommConf->GetAudioParticipantsIconDisplayMode())
				if (!isInGatheringMode() && ((CVideoBridgePartyCntl*) m_pBridgePartyCntl)->IsPartyValidForLayoutIndication())
				{
					WORD timeInterval = pCommConf->GetAudioParticipantsIconDuration();
					TRACEINTO << " ,display mode is eIconDisplayOnChange, duration is: " << timeInterval;
					StartTimer(PARTY_AUDIO_NUMBER_INDICATION_DISPLAY_TOUT, timeInterval * SECOND);
				}

		}
	}

	//m_isAudioIconToSentAfterOpenPort is TRUE when 1) a party just connect or 2) party is in gathering mode
	if (!isInGatheringMode())   //Not in gathering mode
		m_isAudioIconToSentAfterOpenPort = NO;
	else
	{
		m_isAudioIconToSentAfterOpenPort = YES;
		DeleteTimer(PARTY_AUDIO_NUMBER_INDICATION_DISPLAY_TOUT);
	}
}

//--------------------------------------------------------------------------
//Change Layout Improvement - Layout Shared Memory (CL-SM)
void CBridgePartyVideoOut::RemoveFromLayoutSharedMemory()
{
	PASSERT_AND_RETURN(!m_pBridgePartyCntl);

	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	CLayoutSharedMemoryMap* pLayoutSharedMemoryMap = ((CConfPartyProcess*) CProcessBase::GetProcess())->GetLayoutSharedMemory();

	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pLayoutSharedMemoryMap));

	PASSERT_AND_RETURN(!m_pHardwareInterface);

	pLayoutSharedMemoryMap->Remove(m_pHardwareInterface->GetPartyRsrcId());
}

//--------------------------------------------------------------------------
// Indication Icon change Improvement - Indication Icon Shared Memory (CL-SM)
void CBridgePartyVideoOut::RemoveFromIndicationIconSharedMemory()
{
	PASSERT_AND_RETURN(!m_pBridgePartyCntl);

	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	CIndicationIconSharedMemoryMap* pIndicationIconMemoryMap = ((CConfPartyProcess*) CProcessBase::GetProcess())->GetIndicationIconSharedMemory();

	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pIndicationIconMemoryMap));

	PASSERT_AND_RETURN(!m_pHardwareInterface);

	pIndicationIconMemoryMap->Remove(m_pHardwareInterface->GetPartyRsrcId());
}


//--------------------------------------------------------------------------
void CBridgePartyVideoOut::SendIndicationIconsChangeToHardware(BOOL bUseSharedMemForIndicationIcon)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	PASSERT_AND_RETURN(!m_pHardwareInterface);

	m_pCurrentView->indications().dumpIndications();

	((CVideoHardwareInterface*) m_pHardwareInterface)->SendChangeIndicationsOrUpdate(m_pCurrentView,
					bUseSharedMemForIndicationIcon);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::StartSiteNamesOffTimer()
{
	CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	BOOL bSiteNamesAlwaysOn = false;
	std::string key = "SITE_NAMES_ALWAYS_ON";
	sysConfig->GetBOOLDataByKey(key, bSiteNamesAlwaysOn);

	eSystemCardsMode cardMode = GetSystemCardsBasedMode();

	BOOL bStartTimer = false;
	if (cardMode == eSystemCardsMode_mpm_plus)
	{
		if (!bSiteNamesAlwaysOn)
			bStartTimer = true;
	}
	else if (m_pSiteNameInfo->GetDisplayMode() == eSiteNameAuto)
		bStartTimer = true;
	if (bStartTimer == true)
		StartTimer(SITE_NAME_DISPLAY_OFF, SITE_NAME_DISPLAY_TOUT);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::SendChangeLayoutAttributesToHardware(BYTE isFromAudioSpeakerChange)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	PASSERT_AND_RETURN(!m_pHardwareInterface);

	DWORD speakerPlaceInLayout = GetAudioSpeakerPlaceInLayout();

	// Send The new Notation Only
	// 1. If we are here because of VideoSpeaker that changed (Remove or Update the Notation)
	// 2. If we are here because of AudioSpeaker that changed and the new speaker is NOT in the layout
	// so we have to Remove the notation by SendChangeLayoutAttributes with invalid speaker
	// if the case is that this is an AudioSpeaker change of VideoParty the msg from Video calc will set the Speaker Notation.(any order before or after)
	if ((isFromAudioSpeakerChange && (INVALID == speakerPlaceInLayout)) || !isFromAudioSpeakerChange)
	{
		((CVideoHardwareInterface*) m_pHardwareInterface)->SendChangeLayoutAttributes(m_pCurrentView, m_pPartyVisualEffects,
						speakerPlaceInLayout, m_pSiteNameInfo, m_videoConfType);
	}
	else
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Message does not sent since party is NOT in the layout";
	}
	SendUpdateCroppingToHardware();
}

// ------------------------------------------------------------
void CBridgePartyVideoOut::SendUpdateCroppingToHardware()
{
	const DWORD oldCroppingHor = m_croppingHor;
	const DWORD oldCroppingVer = m_croppingVer;

	CVideoBridge* pBridge = (CVideoBridge*) (m_pBridgePartyCntl->GetBridge());

	DWORD croppingHor, croppingVer;
	pBridge->GetCroppingValues(croppingHor, croppingVer);

	m_croppingVer = croppingVer;
	m_croppingHor = croppingHor;

	TRACEINTO << "CroppingVertical:" << m_croppingVer << ", CroppingHorizontal:" << m_croppingHor << ", OldCropVertical:"
					<< oldCroppingVer << ", OldCropHorizontal:" << oldCroppingHor << ", " << m_pBridgePartyCntl->GetFullName();

	if (m_croppingHor != oldCroppingHor || m_croppingVer != oldCroppingVer)
		SendUpdateEncoder();
}

//--------------------------------------------------------------------------
// The function receives the place of the AudioSpeaker in the layout
// the active speaker from amongst ALL the parties in the conference.
DWORD CBridgePartyVideoOut::GetAudioSpeakerPlaceInLayout()
{
	if (!m_pPartyVisualEffects)
		return INVALID;

	//Step 1: check speaker Notation enabled
	if (m_pPartyVisualEffects->IsSpeakerNotationEnable() == 0)
		return INVALID;

	//Step 2: Speaker Notation should not be activated in layout 1x1
	WORD numSubImage = m_pCurrentView->GetNumberOfSubImages();
	if (numSubImage == 1)
		return INVALID;

	PASSERT_AND_RETURN_VALUE(!m_pBridgePartyCntl, INVALID);

	CVideoBridge* pVideoBridge = (CVideoBridge*) (m_pBridgePartyCntl->GetBridge());
	PASSERT_AND_RETURN_VALUE(!pVideoBridge, INVALID);

	//Step 3: Find the Audio Speaker
	CTaskApp* pLastActiveAudioSpeaker = pVideoBridge->GetLastActiveAudioSpeakerRequest();
	if (pLastActiveAudioSpeaker == NULL)
		return INVALID;

	//Step 4: Look for Audio Speaker in layout
	CPartyImageLookupTable* pPartyImageLookupTable = GetPartyImageLookupTable();

	for (WORD imageNumber = 0; imageNumber < numSubImage; imageNumber++)
	{
		CVidSubImage* pVidSubImage = (*m_pCurrentView)[imageNumber];
		if (!CPObject::IsValidPObjectPtr(pVidSubImage))
			continue;

		DWORD partyRscId = pVidSubImage->GetImageId();
		if (!partyRscId)
			continue;

		CImage* pImage = pPartyImageLookupTable->GetPartyImage(partyRscId);
		PASSERTSTREAM(!pImage, "PartyId:" << partyRscId);

		if (pImage && pImage->GetVideoSource() == pLastActiveAudioSpeaker)
		{
			//Step 4:Speaker found in layout -> return place in array
			return (imageNumber);
		}
	}

	//Step 5: Audio Speaker NOT found in layout -> return INVALID
	return INVALID;
}

//--------------------------------------------------------------------------
CLayout* CBridgePartyVideoOut::GetPrivateReservationLayout(void) const
{
	if (m_PrivatelayoutType == CP_NO_LAYOUT)
	{
		PASSERTMSG(1, "Invalid private layout type: m_PrivatelayoutType == CP_NO_LAYOUT");
		return NULL;
	}
	return m_pPrivateReservation[m_PrivatelayoutType];
}

//--------------------------------------------------------------------------
LayoutType CBridgePartyVideoOut::GetPartyCurrentLayoutType(void) const
{
	LayoutType newLayoutType;

	if (m_partyLectureModeRole == eLISTENER || m_partyLectureModeRole == eCOLD_LISTENER)
	{
		newLayoutType = CP_LAYOUT_1X1;
	}
	else if (m_isPrivate)
	{
		newLayoutType = m_PrivatelayoutType;
		if (newLayoutType >= CP_NO_LAYOUT)
		{
			PASSERTSTREAM(1,
							m_pBridgePartyCntl->GetFullName() << ", LayoutType:" << newLayoutType << " - Private Layout type is illegal");
		}
	}
	else
	{
		newLayoutType = GetConfLayoutType();
		if (newLayoutType >= CP_NO_LAYOUT)
		{
			PASSERTSTREAM(1,
							m_pBridgePartyCntl->GetFullName() << ", LayoutType:" << newLayoutType << " - Conference Layout type is illegal");
		}
	}

	return newLayoutType;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::SetIsPrivateLayout(bool isPrivate)
{
	if (isPrivate != m_isPrivate)
		m_isPrivateChanged = true;

	m_isPrivate = isPrivate;

	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", IsPrivateChanged:" << (int) m_isPrivateChanged << ", IsPrivateLayout:"
					<< m_isPrivate;
}

//--------------------------------------------------------------------------
BOOL CBridgePartyVideoOut::IsConnected()
{
	return (m_state == CONNECTED || m_state == CHANGE_LAYOUT);
}

//--------------------------------------------------------------------------
bool CBridgePartyVideoOut::IsStateIsConnected()
{
	return m_state == CONNECTED;
}

//--------------------------------------------------------------------------
LayoutType CBridgePartyVideoOut::GetConfLayoutType() const
{
	return ((CVideoBridgePartyCntl*) m_pBridgePartyCntl)->GetConfLayoutType();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::RemoveConfParams()
{
	CBridgePartyMediaUniDirection::RemoveConfParams();

	OFF(m_layoutChangedWhileWaitingForAck);
	OFF(m_visualEffectsChangedWhileWaitingForAck);
	OFF(m_resolutionChangedWhileWaitingForAck);
	OFF(m_imageResolutionChangedWhileWaitingForAck);
	OFF(m_sitenameChangedWhileWaitingForAck);
	m_tmpSpeakerNotationForPcmFeccImageId = INVALID;

	POBJDELETE(m_pPartyVisualEffects);
	POBJDELETE(m_pPartyPcmFeccVisualEffects);

	POBJDELETE(m_pCurrHandler);
	POBJDELETE(m_pCurrentView);

	for (int i = CP_LAYOUT_1X1; i < CP_NO_LAYOUT; i++)
		POBJDELETE(m_pReservation[i]);
	for (int i = CP_LAYOUT_1X1; i < CP_NO_LAYOUT; i++)
		POBJDELETE(m_pPrivateReservation[i]);

	m_isPrivate = false;
	m_isPrivateChanged = false;
	m_partyLectureModeRole = eREGULAR;
	m_isForce1x1 = false;
	m_waitForUpdateEncoderAck = false;
	m_isLprActive = false;

	m_telepresenceModeChanged = false;
	m_layoutTPRoomSublinkChangedWhileWaitingForAck = false;

	m_recordingType = E_ICON_REC_OFF;
	m_isAudioParticipantsIconActived = NO;
	m_numAudioParticipantsInConf = 0;
	m_isAudioIconToSentAfterOpenPort = NO;
	m_isInGatheringMode = NO;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::UpdateNewConfParamsForOpenedPortAfterMove(ConfRsrcID confRsrcId,
				const CBridgePartyMediaParams* pBridgePartyVideoOutParams)
{
	CBridgePartyMediaUniDirection::UpdateNewConfParams(confRsrcId);
	UpdateNewConfVideoRelatedParamsAfterMove(pBridgePartyVideoOutParams);
	SaveAndSendUpdatedVideoParams((CBridgePartyVideoParams*) pBridgePartyVideoOutParams);

	SetRecordingType(((CVideoBridge*) m_pBridgePartyCntl->GetBridge())->GetRecordingIcon()); //Dmitry: Display recording icon if need
	SetNumAudioParticipantsInConf(((CVideoBridge*) m_pBridgePartyCntl->GetBridge())->GetNumAudioParticipants());
	m_isAudioIconToSentAfterOpenPort = YES; //to prevent conf to hide the audio icon before this party build layout.
	InitIsGatheringModeEnabled();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::UpdateNewConfVideoRelatedParamsAfterMove(const CBridgePartyMediaParams* pBridgePartyVideoOutParams)
{
	TRACEINTO << " ";

	POBJDELETE(m_pPartyVisualEffects);
	POBJDELETE(m_pPartyPcmFeccVisualEffects);

	CBridgePartyVideoOutParams* pVideoOutParams = (CBridgePartyVideoOutParams*) pBridgePartyVideoOutParams;
	CVisualEffectsParams* pVisualEffectsParams = pVideoOutParams->GetVisualEffects();
	if (pVisualEffectsParams != NULL)
	{
		m_pPartyVisualEffects = new CVisualEffectsParams(*pVisualEffectsParams);
		CreatePartyPcmFeccVisualEffects();
	}

	for (int i = CP_LAYOUT_1X1; i < CP_NO_LAYOUT; i++)
	{
		POBJDELETE(m_pReservation[i]);
		m_pReservation[i] = new CLayout((LayoutType) i, m_pBridgePartyCntl->GetConfName());
	}

	for (int i = CP_LAYOUT_1X1; i < CP_NO_LAYOUT; i++)
	{
		POBJDELETE(m_pPrivateReservation[i]);
		m_pPrivateReservation[i] = new CLayout((LayoutType) i, m_pBridgePartyCntl->GetConfName());
	}

	POBJDELETE(m_pCurrentView);
	m_pCurrentView = new CLayout(GetConfLayoutType(), m_pBridgePartyCntl->GetConfName());

	SetLayoutHandler();

	m_partyLectureModeRole = pVideoOutParams->GetPartyLectureModeRole();
	m_isSiteNamesEnabled = pVideoOutParams->GetIsSiteNamesEnabled();

	// Romem 26.5.08 - see if force layout 1*1 during Cascade Link Establishment is permitted in system.cfg
	BOOL bEable1X1LayoutInCascadeLinkEstablishment = true;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("FORCE_1X1_LAYOUT_ON_CASCADED_LINK_CONNECTION",
					bEable1X1LayoutInCascadeLinkEstablishment);
	if (bEable1X1LayoutInCascadeLinkEstablishment == false)
		m_isForce1x1 = NO;
	else
		m_isForce1x1 = pVideoOutParams->GetIsCascadeLink();

	if (m_isForce1x1)
	{
		SetPrivateLayoutForParty(CP_LAYOUT_1X1);
	}
	else
	{
		if (IsNeedPersonalLayoutForGathering())
			SetGatheringLayoutForParty();
	}
	m_isH263Plus = pVideoOutParams->GetIsH263Plus();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::UpdateNewConfParams(ConfRsrcID confRsrcId, const CBridgePartyMediaParams* pBridgePartyVideoOutParams)
{
	TRACEINTO << " ";

	CBridgePartyVideoUniDirection::UpdateNewConfParams(confRsrcId, pBridgePartyVideoOutParams);

	POBJDELETE(m_pPartyVisualEffects);
	POBJDELETE(m_pPartyPcmFeccVisualEffects);
	CBridgePartyVideoOutParams* pVideoOutParams = (CBridgePartyVideoOutParams*) pBridgePartyVideoOutParams;
	CVisualEffectsParams* pVisualEffectsParams = pVideoOutParams->GetVisualEffects();
	if (pVisualEffectsParams != NULL)
	{
		m_pPartyVisualEffects = new CVisualEffectsParams(*pVisualEffectsParams);
		m_pPartyPcmFeccVisualEffects = new CVisualEffectsParams(*m_pPartyVisualEffects);
	}

	for (int i = CP_LAYOUT_1X1; i < CP_NO_LAYOUT; i++)
	{
		POBJDELETE(m_pReservation[i]);
		m_pReservation[i] = new CLayout((LayoutType) i, m_pBridgePartyCntl->GetConfName());
	}

	for (int i = CP_LAYOUT_1X1; i < CP_NO_LAYOUT; i++)
	{
		POBJDELETE(m_pPrivateReservation[i]);
		m_pPrivateReservation[i] = new CLayout((LayoutType) i, m_pBridgePartyCntl->GetConfName());
	}

	POBJDELETE(m_pCurrentView);
	m_pCurrentView = new CLayout(GetConfLayoutType(), m_pBridgePartyCntl->GetConfName());

	SetLayoutHandler();

	m_partyLectureModeRole = pVideoOutParams->GetPartyLectureModeRole();
	m_isSiteNamesEnabled = pVideoOutParams->GetIsSiteNamesEnabled();

	// Romem 26.5.08 - see if force layout 1*1 during Cascade Link Establishment is permitted in system.cfg
	BOOL bEable1X1LayoutInCascadeLinkEstablishment = true;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("FORCE_1X1_LAYOUT_ON_CASCADED_LINK_CONNECTION",
					bEable1X1LayoutInCascadeLinkEstablishment);
	if (bEable1X1LayoutInCascadeLinkEstablishment == false)
		m_isForce1x1 = NO;
	else
		m_isForce1x1 = pVideoOutParams->GetIsCascadeLink();

	if (m_isForce1x1)
		SetPrivateLayoutForParty(CP_LAYOUT_1X1);
	else
	{
		if (IsNeedPersonalLayoutForGathering())
			SetGatheringLayoutForParty();
	}
	m_isH263Plus = pVideoOutParams->GetIsH263Plus();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::AddImage(const CTaskApp* pParty)
{
	DBGPASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pParty));
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD) pParty;
	DispatchEvent(ADDIMAGE, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::UpdateImage()
{
	DispatchEvent(UPDATEIMAGE, NULL);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::VideoRefresh()
{
	DispatchEvent(FASTUPDATE, NULL);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::MuteImage()
{
	DispatchEvent(MUTEIMAGE, NULL);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::UnMuteImage()
{
	DispatchEvent(UNMUTEIMAGE, NULL);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::UpdateIndicationIcons(BOOL bUseSharedMemForIndicationIcon)
{
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD) bUseSharedMemForIndicationIcon;
	DispatchEvent(INDICATION_ICONS_CHANGE, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
FontTypesEnum CBridgePartyVideoOut::GetFontTypeFromConf() const
{
	if (!m_pBridgePartyCntl)
		return ftDefault;

	const CVideoBridge* pBridge = static_cast<CVideoBridge*>(m_pBridgePartyCntl->GetBridge());

	if (!pBridge)
		return ftDefault;

	const CConf* pConf = pBridge->GetConf();
	if (!pConf)
		return ftDefault;

	return static_cast<FontTypesEnum>(pConf->GetCommConf()->GetFontType());
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::ChangeAudioSpeaker(const CTaskApp* pNewSpeaker)
{
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD) pNewSpeaker;
	DispatchEvent(AUDIO_SPEAKER_CHANGED, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::ChangeSpeakers(const CTaskApp* pNewVideoSpeaker, const CTaskApp* pNewAudioSpeaker)
{
	DBGPASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(const_cast<CTaskApp*>(pNewVideoSpeaker)));
	DBGPASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(const_cast<CTaskApp*>(pNewAudioSpeaker)));
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD) pNewVideoSpeaker;
	*pSeg << (DWORD) pNewAudioSpeaker;
	DispatchEvent(SPEAKERS_CHANGED, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::DelImage(const CTaskApp* pParty)
{
	DBGPASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pParty));
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD) pParty;
	DispatchEvent(DELIMAGE, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::ChangeConfLayout(CSegment* pParam)
{
	DispatchEvent(CHANGECONFLAYOUT, pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::ChangeLayoutOfTPRoomSublink(CSegment* pParam)
{
	DispatchEvent(CHANGEPARTYLAYOUT_TPROOM_SUBLINK, pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::ChangePartyLayout(CVideoLayout& newVideoLayout)
{
	CSegment* pSeg = new CSegment;
	newVideoLayout.Serialize(NATIVE, *pSeg);
	DispatchEvent(CHANGEPARTYLAYOUT, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::ChangePartyPrivateLayout(CVideoLayout& newVideoLayout)
{
	TRACEINTO << "State:" << (int) m_state;

	CSegment* pSeg = new CSegment;
	newVideoLayout.Serialize(NATIVE, *pSeg);
	DispatchEvent(CHANGEPARTYPRIVATELAYOUT, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::ChangeLayoutPrivatePartyButtonOnly(WORD isPrivate)
{
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD) isPrivate;

	DispatchEvent(SETPARTYPRIVATELAYOUTONOFF, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::UpdateVisualEffects(CVisualEffectsParams* pVisualEffects, BYTE bInternalUpdateOnly)
{
	CSegment* pSeg = new CSegment;

	ModifyVisuallEffectsForAVMCUParty(pVisualEffects);
	pVisualEffects->Serialize(NATIVE, *pSeg);
	*pSeg << (BYTE) bInternalUpdateOnly;
	DispatchEvent(UPDATEVISUALEFFECTS, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::UpdateVideoParams(CBridgePartyVideoParams* pBridgePartyVideoParams)
{
	CSegment seg;
	pBridgePartyVideoParams->Serialize(NATIVE, seg);

	DispatchEvent(UPDATE_VIDEO_OUT_PARAMS, &seg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::ShowSlide(CSegment *pDataSeg)
{
	DispatchEvent(IVR_SHOW_SLIDE_REQ, pDataSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::ShowSlideForIvrResume(CSegment* pDataSeg)
{
	OnVideoBridgePartyShowSlide(pDataSeg);
}
//--------------------------------------------------------------------------
void CBridgePartyVideoOut::StopShowSlide(CSegment *pDataSeg)
{
	DispatchEvent(IVR_STOP_SHOW_SLIDE_REQ, pDataSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::StopShowSlideForIvrResume(CSegment* pDataSeg)
{
	TRACEINTO << " state will be set to CONNECTED";
	OnVideoBridgePartyStopShowSlide(pDataSeg);
	m_state = CONNECTED;
}
//--------------------------------------------------------------------------
void CBridgePartyVideoOut::StartPLC(CSegment *pDataSeg)
{
	DispatchEvent(VIDEO_GRAPHIC_OVERLAY_START_REQ, pDataSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::StopPLC(CSegment *pDataSeg)
{
	DispatchEvent(VIDEO_GRAPHIC_OVERLAY_STOP_REQ, pDataSeg);
}

//--------------------------------------------------------------------------
const char* CBridgePartyVideoOut::GetStateAsString(WORD state)
{
	switch (state)
	{
	case IDLE:
		return "IDLE";
		break;
	case SLIDE:
		return "SLIDE";
		break;
	case CONNECTED:
		return "CONNECTED";
		break;
	case CHANGE_LAYOUT:
		return "CHANGE_LAYOUT";
		break;
	case DISCONNECTING:
		return "DISCONNECTING";
		break;
	case SETUP:
		return "SETUP";
		break;
	default:
		return "UNKNOWN";
	}
}
//--------------------------------------------------------------------------
void CBridgePartyVideoOut::DisplayRecordingIcon(EIconType eRecordingIcon)
{
	CSegment* pSeg = new CSegment;
	*pSeg << (WORD) eRecordingIcon;

	TRACEINTO << "State:" << GetStateAsString(m_state);

	DispatchEvent(VIDEO_GRAPHICS_SHOW_ICON, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::PLC_SetPartyPrivateLayoutType(LayoutType newPrivateLayoutType)
{
	CSegment* pSeg = new CSegment;
	*pSeg << (BYTE) newPrivateLayoutType;

	DispatchEvent(PLC_SETPARTYPRIVATELAYOUTTYPE, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::PLC_ReturnToConfLayout()
{
	DispatchEvent(PLC_RETURNPARTYTOCONFLAYOUT);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::PLC_ForceToCell(char* partyImageToSee, BYTE cellToForce)
{
	CSegment* pSeg = new CSegment;
	*pSeg << partyImageToSee;
	*pSeg << cellToForce;

	DispatchEvent(PLC_FORCECELLZERO, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::PLC_CancelAllPrivateLayoutForces()
{
	DispatchEvent(PLC_CANCELALLPRIVATELAYOUTFORCES, NULL);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::UpdateLectureModeRole(ePartyLectureModeRole partyLectureModeRole)
{
	CSegment* pSeg = new CSegment;

	*pSeg << (BYTE) partyLectureModeRole;

	DispatchEvent(UPDATE_PARTY_LECTURE_MODE_ROLE, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::DeletePartyFromConf(const char* pDeletedPartyName)
{
	CSegment* pSeg = new CSegment;

	*pSeg << pDeletedPartyName;

	DispatchEvent(DELETED_PARTY_FROM_CONF, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::DisplayPartyTextOnScreen(CTextOnScreenMngr& TextMsgList, DWORD timeout)
{
	CSegment *pSeg = new CSegment;
	TextMsgList.Serialize(NATIVE, *pSeg);
	*pSeg << timeout;
	DispatchEvent(DISPLAY_TEXT_ON_SCREEN, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::StopDisplayPartyTextOnScreen()
{
	DispatchEvent(TEXT_ON_SCREEN_DISPLAY_OFF);
}

//--------------------------------------------------------------------------
BOOL CBridgePartyVideoOut::RejectChangeLayoutRequestBecauseOfApplications()
{
	return (m_partyLectureModeRole == eLISTENER || m_partyLectureModeRole == eCOLD_LISTENER);
}

//--------------------------------------------------------------------------
BOOL CBridgePartyVideoOut::RejectPLCRequestBecauseOfApplications(OPCODE requestOpcode)
{
	BYTE isSameLayoutConf = ((CVideoBridgePartyCntl*) m_pBridgePartyCntl)->IsConfSameLayout();

	if (m_partyLectureModeRole == eLISTENER || m_partyLectureModeRole == eCOLD_LISTENER || isSameLayoutConf)
	{
		CSegment* pSeg = new CSegment;
		*pSeg << requestOpcode;

		((CVideoBridgePartyCntl*) m_pBridgePartyCntl)->IvrNotification(REJECT_PLC, pSeg);

		POBJDELETE(pSeg);
		TRACEINTO << " - Reject PLC, isSameLayout: " << (DWORD) isSameLayoutConf << ", partyLectureMode: "
						<< (DWORD) m_partyLectureModeRole;

		return true;
	}

	return false;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::DeleteAllPartyForces(const char* partyName)
{
	DBGPASSERT_AND_RETURN(!partyName);

	for (int i = CP_LAYOUT_1X1; i < CP_NO_LAYOUT; i++)
	{
		m_pReservation[i]->RemovePartyForce(partyName);
		m_pPrivateReservation[i]->RemovePartyForce(partyName);
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::UpdatePartyTelePresenceMode(eTelePresencePartyType partyNewTelePresenceMode)
{
	CSegment* pSeg = new CSegment;

	*pSeg << (WORD) partyNewTelePresenceMode;

	DispatchEvent(UPDATE_PARTY_TELEPRESENCE_MODE, pSeg);
	POBJDELETE(pSeg)
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::DumpAllInfoOnConnectionState(CMedString* pMedString, bool isShortPrint)
{
	if (!isShortPrint && m_state == IDLE)
		*pMedString << "IDLE";
	else if (!isShortPrint && m_state == SLIDE)
		*pMedString << "SLIDE";
	else if (!isShortPrint && m_state == CONNECTED)
		*pMedString << "CONNECTED";
	else if (!isShortPrint && m_state == CHANGE_LAYOUT)
		*pMedString << "CHANGE_LAYOUT";
	else if (!isShortPrint && m_state == DISCONNECTING)
		*pMedString << "DISCONNECTING";
	else if (m_state == SETUP)
	{
		if (!isShortPrint)
			*pMedString << "SETUP : ";

		if (!m_isPortOpened)
			*pMedString << "No ack TB_MSG_CONNECT_REQ";
		else
			*pMedString << "Got ack TB_MSG_CONNECT_REQ,No ack on TB_MSG_OPEN_PORT_REQ";
	}
	else
		*pMedString << "UNKNOWN STATE";
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyConnectIDLE(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	m_state = SETUP;
	SetClosePortAckStatus(STATUS_OK);
	SendConnectToRtp();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyConnectSLIDE(CSegment* pParam)
{
	PASSERTSTREAM(1, m_pBridgePartyCntl->GetFullName() << " - Event is ignored because of illegal state");
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyConnectSETUP(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Event is ignored";
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyConnectCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Already connected";

	CSegment* pSeg = new CSegment;

	*pSeg << (BYTE) statOK;

	m_pBridgePartyCntl->HandleEvent(pSeg, 0, VIDEO_OUT_CONNECTED);

	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyConnectCHANGELAYOUT(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Already connected";

	CSegment* pSeg = new CSegment;

	*pSeg << (BYTE) statOK;

	m_pBridgePartyCntl->HandleEvent(pSeg, 0, VIDEO_OUT_CONNECTED);

	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyConnectDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Already connected";
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyDisConnectIDLE(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Already disconnected";

	CSegment* pSeg = new CSegment;

	*pSeg << (BYTE) statOK;

	m_pBridgePartyCntl->HandleEvent(pSeg, 0, VIDEO_OUT_DISCONNECTED);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyDisConnectSLIDE(CSegment* pParam)
{
	PASSERTSTREAM(1, m_pBridgePartyCntl->GetFullName() << " - Received disconnect while slide still active");

	m_state = IDLE;

	CSegment* pSeg = new CSegment;

	*pSeg << (BYTE) statOK;

	m_pBridgePartyCntl->HandleEvent(pSeg, 0, VIDEO_OUT_DISCONNECTED);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyDisConnectSETUP(CSegment* pParam)
{
	if (!m_isPortOpened)
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Port never opened (might be Connected)";
		//Port never opened - Disconnecting while setup or because of no ack or bad status in ack
		m_state = IDLE;
		CSegment *pMsg = new CSegment;
		*pMsg << (BYTE) statOK;
		m_pBridgePartyCntl->HandleEvent(pMsg, 0, VIDEO_OUT_DISCONNECTED);
		POBJDELETE(pMsg);
	}
	else
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName();

		OnVideoBridgePartyDisConnect(pParam);
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyDisConnectCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	OnVideoBridgePartyDisConnect(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyDisConnectCHANGELAYOUT(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	DeleteTimer(VIDEO_OUT_CHANGELAYOUT_TIMEOUT);

	OnVideoBridgePartyDisConnect(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyDisConnectDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Event is ignored";
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyDisConnect(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	m_state = DISCONNECTING;
	// Always return to Conference layout
	SetIsPrivateLayout(NO);
	POBJDELETE(m_pCurrentView);
	m_pCurrentView = new CLayout(GetConfLayoutType(), m_pBridgePartyCntl->GetConfName());

	SendCloseEncoder();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnMplOpenPortAck(STATUS status)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", Status:" << status;

	if (TB_MSG_CONNECT_REQ == m_lastReq) // BRIDGE-12215
	{
		// Ack on OPEN while the last request was CONNECT
		// this may happen in case of fast connect-disconnect-connect and the Ack from the first connect arrives after the second connect
		TRACEINTO << " Warning: Old Ack in new connect, ignore this Ack, PartyRsrcId: "
						<< (DWORD) m_pHardwareInterface->GetPartyRsrcId();
		return;
	}
	if (status != STATUS_OK)
	{
		// Status not OK, send answer to VideoBridge;
		CSegment* pSeg = new CSegment;
		*pSeg << (BYTE) statVideoInOutResourceProblem << (BYTE) eMipOut << (BYTE) eMipStatusFail << (BYTE) eMipOpen;

		// Inform BridgePartyCntl of statVideoInOutResourceProblem -> Inorder to initiate Kill port on VideoEncoder VideoDecoder
		m_pBridgePartyCntl->HandleEvent(pSeg, 0, VIDEO_OUT_CONNECTED);
		POBJDELETE(pSeg);
	}
	else
	{
		m_state = CONNECTED;

		CSegment* pMsg = new CSegment;
		*pMsg << (BYTE) statOK;
		m_pBridgePartyCntl->HandleEvent(pMsg, 0, VIDEO_OUT_CONNECTED);
		POBJDELETE(pMsg);

		SetRecordingType(((CVideoBridge*) m_pBridgePartyCntl->GetBridge())->GetRecordingIcon());
		SetNumAudioParticipantsInConf(((CVideoBridge*) m_pBridgePartyCntl->GetBridge())->GetNumAudioParticipants());
		m_isAudioIconToSentAfterOpenPort = YES; //to prevent conf to hide the audio icon before this party build layout.
		InitIsGatheringModeEnabled();

		//Block the sending change layout in EMB MLA on open port ACK in case number of links isn't updated yet
		CVideoBridge* pBridge = (CVideoBridge*) m_pBridgePartyCntl->GetBridge();
		PASSERT_AND_RETURN(!pBridge);

		CConf* pConf = pBridge->GetConf();
		PASSERT_AND_RETURN(!pConf);

		const CCommConf* pCommConf = pConf->GetCommConf();
		PASSERT_AND_RETURN(!pCommConf);

		if (pCommConf->GetManageTelepresenceLayoutInternaly() == YES && pCommConf->GetIsTelePresenceMode() == true
						&& ((CVideoBridgePartyCntl*) m_pBridgePartyCntl)->GetTelepresenceInfo().GetNumOfLinks() == (DWORD) -1)
		{
			TRACEINTO << "Block the sending change layout in EMB MLA on open port ACK in case number of links isn't updated yet";
		}
		else
		{
			BuildLayout();
		}

		eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
		if (IsFeatureSupportedBySystem(eFeatureEncoderRecurrentIntra) && (m_videoConfType == eVideoConfTypeCP))
		{
			DWORD encoderRequestIntraToutValue = 0;
			CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
			if (m_bIsTipMode)
			{
				sysConfig->GetDWORDDataByKey("ENCODER_TIP_RECURRENT_INTRA_REQ_MINUTES", encoderRequestIntraToutValue);
			}
			else
			{
				sysConfig->GetDWORDDataByKey("ENCODER_RECURRENT_INTRA_REQ_MINUTES", encoderRequestIntraToutValue);
			}

			if (encoderRequestIntraToutValue)
			{
				StartTimer(ENCODER_RECURRENT_INTRA_REQ_TIMEOUT, encoderRequestIntraToutValue * 60 * SECOND);
			}
		}

		if (m_pWaitingForUpdateParams)
		{
			SaveAndSendUpdatedVideoParams(m_pWaitingForUpdateParams);
			POBJDELETE(m_pWaitingForUpdateParams);
		}
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnMplConnectAck(STATUS status)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << "Status:" << status;

	if (status != STATUS_OK)
	{
		// Status not OK, send answer to VideoBridge;
		CSegment* pSeg = new CSegment;
		*pSeg << (BYTE) statVideoInOutResourceProblem << (BYTE) eMipOut << (BYTE) eMipStatusFail << (BYTE) eMipConnect;

		// Inform BridgePartyCntl of statVideoInOutResourceProblem -> In order to initiate Kill port on VideoEncoder VideoDecoder
		m_pBridgePartyCntl->HandleEvent(pSeg, 0, VIDEO_OUT_CONNECTED);

		POBJDELETE(pSeg);
		return;
	}

	SendOpenEncoder();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnMplClosePortAck(STATUS status)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", Status:" << status;

	BYTE responseStatus = statOK;

	if (status != STATUS_OK)
		responseStatus = statVideoInOutResourceProblem;

	SetClosePortAckStatus(responseStatus);
	m_state = IDLE;
	DeleteTimer(ENCODER_RECURRENT_INTRA_REQ_TIMEOUT);

	CSegment* pSeg = new CSegment;
	*pSeg << (BYTE) responseStatus;
	if (responseStatus == statVideoInOutResourceProblem)
		*pSeg << (BYTE) eMipOut << (BYTE) eMipStatusFail << (BYTE) eMipClose;

	// Inform BridgePartyCntl
	m_pBridgePartyCntl->HandleEvent(pSeg, 0, VIDEO_OUT_DISCONNECTED);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnMplChangeLayoutAck(STATUS status)
{
	// vngr-7017 "bombing" logger on change layout - change trace to DEBUG level
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", Status:" << status;

	DeleteTimer(VIDEO_OUT_CHANGELAYOUT_TIMEOUT);

	if (status != STATUS_OK)
	{
		// vngr-23833 : RL assert - reducede to trace - no active action , did not caused a real problem in RL
		EXCEPTION_TRACE(status, "CBridgePartyVideoOut::OnMplChangeLayoutAck - status != STATUS_OK");
		//DBGPASSERT(status);
	}

	m_state = CONNECTED;

	BYTE wasNewLayoutSent = false;

	if (m_imageResolutionChangedWhileWaitingForAck)
	{
		BOOL bUseSharedMemForChangeLayoutReq = false;
		BYTE alwaysSendToHardware = true; // will always send the new layout to hardware
		BuildLayout(bUseSharedMemForChangeLayoutReq, alwaysSendToHardware);
		wasNewLayoutSent = true;
	}

	if (m_resolutionChangedWhileWaitingForAck && !wasNewLayoutSent)
	{
		BuildLayoutAndSendOnEncoderResolutionChange();
		wasNewLayoutSent = true;
	}

	if (m_layoutChangedWhileWaitingForAck && !wasNewLayoutSent && m_telepresenceModeChanged)
	{
		BOOL bUseSharedMemForChangeLayoutReq = false;
		BYTE alwaysSendToHardware = true; // will always send the new layout to hardware
		BuildLayout(bUseSharedMemForChangeLayoutReq, alwaysSendToHardware);
		wasNewLayoutSent = true;
	}

	if (m_layoutChangedWhileWaitingForAck && !wasNewLayoutSent)
	{
		wasNewLayoutSent = BuildLayout();
	}

	if (m_visualEffectsChangedWhileWaitingForAck && !wasNewLayoutSent)
	{
		SendChangeLayoutAttributesToHardware();	//might be sent when not required (when there is no speaker in layout and the speaker indication params change - harmless)
	}
	if (m_tmpSpeakerNotationForPcmFeccImageId != INVALID)
	{
		DWORD tmpSpeakerNotationForPcmFeccImageId = m_tmpSpeakerNotationForPcmFeccImageId;
		m_tmpSpeakerNotationForPcmFeccImageId = INVALID;
		ChangeSpeakerNotationForPcmFecc(tmpSpeakerNotationForPcmFeccImageId);
	}

	if (m_sitenameChangedWhileWaitingForAck)
	{
		if (m_pSiteNameInfo->GetDisplayMode() == eSiteNameOff)
		{
			((CVideoHardwareInterface*) m_pHardwareInterface)->SendStopSiteNameDisplay();
		}
		else
			SendChangeLayoutToHardware();
	}
	OFF(m_resolutionChangedWhileWaitingForAck);
	OFF(m_layoutChangedWhileWaitingForAck);
	OFF(m_visualEffectsChangedWhileWaitingForAck);
	OFF(m_imageResolutionChangedWhileWaitingForAck);
	OFF(m_sitenameChangedWhileWaitingForAck);

	m_telepresenceModeChanged = false;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnMplAckIDLE(CSegment* pParam)
{
	OPCODE AckOpcode;
	DWORD ack_seq_num;
	STATUS status;
	*pParam >> AckOpcode >> ack_seq_num >> status;

	switch (AckOpcode)
	{
	case IVR_STOP_SHOW_SLIDE_REQ: // CAM sends StopShowSlide we change the state from Slide to Idle
	{
		CSegment* pSeg = new CSegment;
		*pSeg << AckOpcode << ack_seq_num << status << *pParam;

		((CVideoBridgePartyCntl*) m_pBridgePartyCntl)->IvrNotification(ACK_IND, pSeg);
		POBJDELETE(pSeg);
		break;
	}
	default:
	{
		TRACEINTO << m_pBridgePartyCntl->GetName() << ", AckOpcode:" << CProcessBase::GetProcess()->GetOpcodeAsString(AckOpcode)
						<< " - Ignored";
		break;
	}
	} // end switch
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnMplAckSLIDE(CSegment* pParam)
{
	OPCODE AckOpcode;
	DWORD ack_seq_num;
	STATUS status;
	*pParam >> AckOpcode >> ack_seq_num >> status;

	switch (AckOpcode)
	{
	case IVR_SHOW_SLIDE_REQ:
	{
		CSegment* pSeg = new CSegment;
		*pSeg << AckOpcode << ack_seq_num << status << *pParam;

		((CVideoBridgePartyCntl*) m_pBridgePartyCntl)->IvrNotification(ACK_IND, pSeg);
		POBJDELETE(pSeg);
		break;
	}
	default:
	{
		TRACEINTO << m_pBridgePartyCntl->GetName() << ", AckOpcode:" << CProcessBase::GetProcess()->GetOpcodeAsString(AckOpcode)
						<< " - Ignored";
		break;
	}
	} // end switch
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnMplAckSETUP(CSegment* pParam)
{
	OPCODE AckOpcode;
	DWORD ack_seq_num;
	STATUS status;
	*pParam >> AckOpcode >> ack_seq_num >> status;

	switch (AckOpcode)
	{
	case TB_MSG_CONNECT_REQ:
	{
		OnMplConnectAck(status);
		break;
	}
	case TB_MSG_OPEN_PORT_REQ:
	{
		OnMplOpenPortAck(status);
		break;
	}
	case IVR_SHOW_SLIDE_REQ:
	case IVR_STOP_SHOW_SLIDE_REQ:
	{
		CSegment* pSeg = new CSegment;
		*pSeg << AckOpcode << ack_seq_num << status << *pParam;

		((CVideoBridgePartyCntl*) m_pBridgePartyCntl)->IvrNotification(ACK_IND, pSeg);
		POBJDELETE(pSeg);
		break;
	}
	default:
	{
		TRACEINTO << m_pBridgePartyCntl->GetName() << ", AckOpcode:" << CProcessBase::GetProcess()->GetOpcodeAsString(AckOpcode)
						<< " - Ignored";
		break;
	}
	} // end switch
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnMplAckDISCONNECTING(CSegment* pParam)
{
	OPCODE AckOpcode;
	DWORD ack_seq_num;
	STATUS status;
	*pParam >> AckOpcode >> ack_seq_num >> status;
	switch (AckOpcode)
	{
	case TB_MSG_CLOSE_PORT_REQ:
	{
		OnMplClosePortAck(status);
		break;
	}
	default:
	{
		TRACEINTO << m_pBridgePartyCntl->GetName() << ", AckOpcode:" << CProcessBase::GetProcess()->GetOpcodeAsString(AckOpcode)
						<< " - Ignored";
		break;
	}
	} // end switch
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnMplAckCHANGELAYOUT(CSegment* pParam)
{
	OPCODE AckOpcode;
	DWORD ack_seq_num;
	STATUS status;
	*pParam >> AckOpcode >> ack_seq_num >> status;
	switch (AckOpcode)
	{
	case VIDEO_ENCODER_CHANGE_LAYOUT_REQ:
	{
		OnMplChangeLayoutAck(status);
		break;
	}

	case VIDEO_GRAPHIC_OVERLAY_START_REQ:
	case VIDEO_GRAPHIC_OVERLAY_STOP_REQ:
	case IVR_SHOW_SLIDE_REQ:
	case IVR_STOP_SHOW_SLIDE_REQ:
	{
		CSegment* pSeg = new CSegment;
		*pSeg << AckOpcode << ack_seq_num << status << *pParam;

		((CVideoBridgePartyCntl*) m_pBridgePartyCntl)->IvrNotification(ACK_IND, pSeg);

		POBJDELETE(pSeg);
		break;
	}

	case VIDEO_ENCODER_UPDATE_PARAM_REQ:
	{
		DeleteTimer(ENCODER_UPDATE_PARAM_TOUT );
		if (m_waitForUpdateEncoderAck == true)
		{
			TRACEINTO << m_pBridgePartyCntl->GetName() << ", WaitForUpdateEncoderAck:true"
							<< " - Received ack on VIDEO_ENCODER_UPDATE_PARAM_REQ";

			m_waitForUpdateEncoderAck = false;
			if (status == STATUS_OK)
			{
				DispatchEvent(ENCODER_RESOLUTION_CHANGED, NULL);
			}
			else
			{
				// at this stage (V7) we do nothing, add error handling later if we need
				PASSERT(status);
			}
		}
		break;
	}

	default:
	{
		TRACEINTO << m_pBridgePartyCntl->GetName() << ", AckOpcode:" << CProcessBase::GetProcess()->GetOpcodeAsString(AckOpcode)
						<< " - Ignored";
		break;
	}
	} // end switch
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnMplAckCONNECTED(CSegment* pParam)
{
	OPCODE AckOpcode;
	DWORD ack_seq_num;
	STATUS status;
	*pParam >> AckOpcode >> ack_seq_num >> status;

	PASSERTSTREAM_AND_RETURN(!m_pBridgePartyCntl || !IsValidPObjectPtr(m_pBridgePartyCntl),
					"AckOpcode: " << AckOpcode << ", Status:" << status);

	switch (AckOpcode)
	{
	case VIDEO_GRAPHIC_OVERLAY_START_REQ:
	case VIDEO_GRAPHIC_OVERLAY_STOP_REQ:
	case IVR_SHOW_SLIDE_REQ:
	case IVR_STOP_SHOW_SLIDE_REQ:
	{
		CSegment* pSeg = new CSegment;
		*pSeg << AckOpcode << ack_seq_num << status << *pParam;
		((CVideoBridgePartyCntl*) m_pBridgePartyCntl)->IvrNotification(ACK_IND, pSeg);
		POBJDELETE(pSeg);
		break;
	}

	case VIDEO_GRAPHICS_START_GATHERING_REQ:
	{
		if (status == STATUS_OK)
		{
			m_bEnableReStopGathering = true;
		}
		else if (status == STATUS_FAIL)
		{
			CGathering* pGathering = GetGathering();
			if (pGathering)
			{
				pGathering->SetIsNeedFullRendering(true, m_pBridgePartyCntl->GetName());
				pGathering->SetDisplayed(false, m_pBridgePartyCntl->GetName());
			}
		}
		break;
	}

	case VIDEO_GRAPHICS_STOP_GATHERING_REQ:
	{
		if (status == STATUS_OK)
		{
			m_bEnableReStopGathering = true;
		}
		else if (status == STATUS_FAIL)
		{
			if (m_bEnableReStopGathering)
			{
				((CVideoHardwareInterface*) m_pHardwareInterface)->StopGathering();
				m_bEnableReStopGathering = false;
			}
			else
			{
				TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", AckOpcode:VIDEO_GRAPHICS_STOP_GATHERING_REQ, Status:"
								<< status;
			}
		}
		break;
	}

	case VIDEO_ENCODER_UPDATE_PARAM_REQ:
	{
		DeleteTimer(ENCODER_UPDATE_PARAM_TOUT );
		CGathering* pGathering = GetGathering();
		if (pGathering)
		{
			pGathering->SetIsNeedFullRendering(true, m_pBridgePartyCntl->GetName());
			pGathering->ShowGatheringText(m_pBridgePartyCntl->GetName());
		}

		if (m_waitForUpdateEncoderAck == true)
		{
			TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", AckOpcode:VIDEO_ENCODER_UPDATE_PARAM_REQ, Status:" << status;

			m_waitForUpdateEncoderAck = false;
			if (status == STATUS_OK)
			{
				DispatchEvent(ENCODER_RESOLUTION_CHANGED, NULL);
			}
			else
			{
				// at this stage (V7) we do nothing, add error handling later if we need
				PASSERT(status);
			}
		}
		break;
	}

	default:
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", AckOpcode:" << AckOpcode << " - Event is ignored";
		break;
	}
	} // end switch
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyUpdateVideoParamsIDLE(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	CBridgePartyVideoParams params;
	params.DeSerialize(NATIVE, *pParam);

	EStat responseStatus = statOK;

	m_videoAlg = params.GetVideoAlgorithm();
	m_videoBitRate = params.GetVideoBitRate();
	m_videoResolution = params.GetVideoResolution(); // In COP the resolution isn't relevant onlt for H263
	const MsSvcParamsStruct& newMsSvcParams = params.MsSvcParams();

	if (m_videoAlg == H264 || m_videoAlg == MS_SVC || m_videoAlg == VP8)
	{
		m_videoQcifFrameRate = eVideoFrameRateDUMMY;
		m_videoCifFrameRate = eVideoFrameRateDUMMY;
		m_video4CifFrameRate = eVideoFrameRateDUMMY;
		m_MBPS = params.GetMBPS();
		m_FS = params.GetFS();
		m_sampleAspectRatio = params.GetSampleAspectRatio();
		m_staticMB = params.GetStaticMB();
		m_videoFrameRate = params.GetVidFrameRate();
		m_profile = params.GetProfile();
		m_packetPayloadFormat = params.GetPacketFormat();
		m_maxDPB = params.GetMaxDPB();
		m_bIsTipMode = params.GetIsTipMode();
		m_bUseIntermediateSDResolution = params.GetUseIntermediateSDResolution();
		m_bEncodeBFramesInRTV = false;
		m_dwFrThreshold = params.GetFrThreshold();
		m_msftSvcParamsStruct = newMsSvcParams;
		if (m_videoAlg == MS_SVC)
			{TRACEINTO << m_msftSvcParamsStruct;}
	}
	else if (m_videoAlg == RTV)
	{
		m_videoQcifFrameRate = eVideoFrameRateDUMMY;
		m_videoCifFrameRate = eVideoFrameRateDUMMY;
		m_video4CifFrameRate = eVideoFrameRateDUMMY;
		m_MBPS = params.GetMBPS();
		m_FS = params.GetFS();
		m_sampleAspectRatio = DEFAULT_SAMPLE_ASPECT_RATIO;
		m_staticMB = DEFAULT_STATIC_MB;
		m_profile = eVideoProfileDummy;
		m_packetPayloadFormat = eVideoPacketPayloadFormatDummy;
		m_maxDPB = INVALID;
		m_bUseIntermediateSDResolution = params.GetUseIntermediateSDResolution();
		m_bEncodeBFramesInRTV = params.GetIsEncodeRTVBFrame();
		m_dwFrThreshold = params.GetFrThreshold();
		m_msftSvcParamsStruct = newMsSvcParams;
		//TRACEINTO << m_msftSvcParamsStruct;
	}
	else
	{
		m_videoQcifFrameRate = params.GetVideoFrameRate(eVideoResolutionQCIF);
		m_videoCifFrameRate = params.GetVideoFrameRate(eVideoResolutionCIF);
		m_video4CifFrameRate = params.GetVideoFrameRate(eVideoResolution4CIF);
		m_MBPS = INVALID;
		m_FS = INVALID;
		m_sampleAspectRatio = DEFAULT_SAMPLE_ASPECT_RATIO;
		m_staticMB = DEFAULT_STATIC_MB;
		m_profile = eVideoProfileDummy;
		m_packetPayloadFormat = eVideoPacketPayloadFormatDummy;
		m_maxDPB = INVALID;
		m_bIsTipMode = false;
		m_bUseIntermediateSDResolution = false;
		m_bEncodeBFramesInRTV = false;
		m_dwFrThreshold = params.GetFrThreshold();
	}

	if (m_videoAlg == VP8)	// VP8 New Protocol
	{
		TRACEINTO << " Debug Info: VP8 Protocol, PartyRsrcId: " << (DWORD)m_pBridgePartyCntl->GetPartyRsrcID();
		// get VP8 specific params
	}

	CSegment msg;
	msg << (BYTE) responseStatus;

	if (responseStatus == statOK)
	{
		// Need to add Ack params to Acknowledge msg to party
		if (params.GetAckParams())
		{
			msg << true;
			params.GetAckParams()->Serialize(NATIVE, msg);
		}
		else
			msg << false;
	}

	m_pBridgePartyCntl->HandleEvent(&msg, 0, PARTY_VIDEO_OUT_UPDATED);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyUpdateVideoParamsSLIDE(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	CBridgePartyVideoParams params;
	params.DeSerialize(NATIVE, *pParam);

	DWORD newVideoAlg = params.GetVideoAlgorithm();
	DWORD newVideoBitRate = params.GetVideoBitRate();
	eVideoFrameRate newVideoQCifFrameRate = params.GetVideoFrameRate(eVideoResolutionQCIF);
	eVideoFrameRate newVideoCifFrameRate = params.GetVideoFrameRate(eVideoResolutionCIF);
	eVideoFrameRate newVideo4CifFrameRate = params.GetVideoFrameRate(eVideoResolution4CIF);
	eVideoResolution newVideoResolution = params.GetVideoResolution();
	DWORD newMBPS = params.GetMBPS();
	DWORD newFS = params.GetFS();
	DWORD newSampleAspectRatio = params.GetSampleAspectRatio();
	DWORD newStaticMB = params.GetStaticMB();
	eVideoProfile newProfile = params.GetProfile();
	eVideoPacketPayloadFormat newPacketPayloadFormat = params.GetPacketFormat();
	DWORD newMaxDPB = params.GetMaxDPB();
	BYTE bIsTipMode = params.GetIsTipMode();
	eVideoFrameRate newVideoFrameRate = params.GetVidFrameRate();
	const MsSvcParamsStruct& newMsSvcParams = params.MsSvcParams();

	EStat responseStatus = statOK;
	const bool changed_MsSvc = (newVideoAlg == MS_SVC && newMsSvcParams.ssrc && m_msftSvcParamsStruct != newMsSvcParams);

	if (m_videoAlg == newVideoAlg && m_videoBitRate == newVideoBitRate && m_videoQcifFrameRate == newVideoQCifFrameRate
					&& m_videoCifFrameRate == newVideoCifFrameRate && m_video4CifFrameRate == newVideo4CifFrameRate
					&& m_videoResolution == newVideoResolution && m_sampleAspectRatio == newSampleAspectRatio
					&& m_staticMB == newStaticMB && m_profile == newProfile && m_packetPayloadFormat == newPacketPayloadFormat
					&& m_maxDPB == newMaxDPB && m_bIsTipMode == bIsTipMode && !changed_MsSvc)
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - No real change in Params";
	}
	else
	{
		// save new params
		m_videoAlg = newVideoAlg;
		m_videoBitRate = newVideoBitRate;
		m_videoResolution = newVideoResolution; // In COP the resolution isn't relevant onlt for H263
		if (m_videoAlg == H264 || m_videoAlg == MS_SVC)
		{
			m_videoQcifFrameRate = eVideoFrameRateDUMMY;
			m_videoCifFrameRate = eVideoFrameRateDUMMY;
			m_video4CifFrameRate = eVideoFrameRateDUMMY;
			m_MBPS = newMBPS;
			m_FS = newFS;
			m_sampleAspectRatio = newSampleAspectRatio;
			m_staticMB = newStaticMB;
			m_profile = newProfile;
			m_packetPayloadFormat = newPacketPayloadFormat;
			m_maxDPB = newMaxDPB;
			m_bIsTipMode = bIsTipMode;
			m_bUseIntermediateSDResolution = params.GetUseIntermediateSDResolution();
			m_bEncodeBFramesInRTV = false;
			m_videoFrameRate = newVideoFrameRate;
			m_msftSvcParamsStruct = newMsSvcParams;
			TRACEINTO << m_msftSvcParamsStruct;
		}
		else if (m_videoAlg == RTV)
		{
			m_videoQcifFrameRate = eVideoFrameRateDUMMY;
			m_videoCifFrameRate = eVideoFrameRateDUMMY;
			m_video4CifFrameRate = eVideoFrameRateDUMMY;
			m_MBPS = newMBPS;
			m_FS = newFS;
			m_sampleAspectRatio = DEFAULT_SAMPLE_ASPECT_RATIO;
			m_staticMB = DEFAULT_STATIC_MB;
			m_profile = eVideoProfileDummy;
			m_packetPayloadFormat = eVideoPacketPayloadFormatDummy;
			m_maxDPB = INVALID;
			m_bUseIntermediateSDResolution = params.GetUseIntermediateSDResolution();
			m_bEncodeBFramesInRTV = params.GetIsEncodeRTVBFrame();
			m_msftSvcParamsStruct = newMsSvcParams;
			TRACEINTO << m_msftSvcParamsStruct;
		}
		else
		{
			m_videoQcifFrameRate = newVideoQCifFrameRate;
			m_videoCifFrameRate = newVideoCifFrameRate;
			m_video4CifFrameRate = newVideo4CifFrameRate;
			m_MBPS = INVALID;
			m_FS = INVALID;
			m_sampleAspectRatio = DEFAULT_SAMPLE_ASPECT_RATIO;
			m_staticMB = DEFAULT_STATIC_MB;
			m_profile = eVideoProfileDummy;
			m_packetPayloadFormat = eVideoPacketPayloadFormatDummy;
			m_maxDPB = INVALID;
			m_bIsTipMode = false;
			m_bUseIntermediateSDResolution = false;
			m_bEncodeBFramesInRTV = false;
		}

		PASSERT_AND_RETURN(!m_pHardwareInterface);
		CVideoHardwareInterface* pHWInt = (CVideoHardwareInterface*) m_pHardwareInterface;
		DWORD fs = m_FS;
		DWORD mbps = m_MBPS;
		if (m_videoAlg == MS_SVC)
		{
			fs = pHWInt->GetFsForSvcLync(m_msftSvcParamsStruct.nWidth, m_msftSvcParamsStruct.nHeight);
			mbps = (DWORD) GetMaxMbpsAsDevision((DWORD) (fs * pHWInt->TranslateVideoFrameRateToNumeric(m_videoFrameRate)));
			TRACEINTO << "MSSVC fs:" << fs << ", mbps:" << mbps;
		}

		STATUS status = pHWInt->SendUpdateSlide(m_videoResolution, m_videoAlg, m_videoBitRate, fs, mbps, m_bIsTipMode);

		if (STATUS_OK != status)
		{
			PASSERTMSG(status, "Could Not Send Show Slide with Updated Params");
			responseStatus = statIllegal;
		}
	}

	CSegment msg;
	msg << (BYTE) responseStatus;

	// Need to add Ack params to Acknowledge msg to party
	if (params.GetAckParams())
	{
		msg << true;
		params.GetAckParams()->Serialize(NATIVE, msg);
	}
	else
		msg << false;

	m_pBridgePartyCntl->HandleEvent(&msg, 0, PARTY_VIDEO_OUT_UPDATED);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyUpdateVideoParamsSETUP(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	CBridgePartyVideoParams params;
	params.DeSerialize(NATIVE, *pParam);

	if (!m_isPortOpened) // means we are waiting for Ack on ConnectReq - enough to update params that will be sent in OpenPort
	{
		m_videoAlg = params.GetVideoAlgorithm();
		m_videoBitRate = params.GetVideoBitRate();
		m_videoResolution = params.GetVideoResolution(); // In COP the resolution isn't relevant onlt for H263

		const MsSvcParamsStruct& newMsSvcParams = params.MsSvcParams();

		if (m_videoAlg == H264 || m_videoAlg == MS_SVC)
		{
			m_videoQcifFrameRate = eVideoFrameRateDUMMY;
			m_videoCifFrameRate = eVideoFrameRateDUMMY;
			m_video4CifFrameRate = eVideoFrameRateDUMMY;
			m_MBPS = params.GetMBPS();
			m_FS = params.GetFS();
			m_sampleAspectRatio = params.GetSampleAspectRatio();
			m_staticMB = params.GetStaticMB();
			m_profile = params.GetProfile();
			m_packetPayloadFormat = params.GetPacketFormat();
			m_maxDPB = params.GetMaxDPB();
			m_bIsTipMode = params.GetIsTipMode();
			m_bUseIntermediateSDResolution = params.GetUseIntermediateSDResolution();
			m_videoFrameRate = params.GetVidFrameRate();
			m_bEncodeBFramesInRTV = false;
			m_dwFrThreshold = params.GetFrThreshold();
			m_msftSvcParamsStruct = newMsSvcParams;
			TRACEINTO << m_msftSvcParamsStruct;
		}
		else if (m_videoAlg == RTV)
		{
			m_videoQcifFrameRate = eVideoFrameRateDUMMY;
			m_videoCifFrameRate = eVideoFrameRateDUMMY;
			m_video4CifFrameRate = eVideoFrameRateDUMMY;
			m_MBPS = params.GetMBPS();
			m_FS = params.GetFS();
			m_sampleAspectRatio = DEFAULT_SAMPLE_ASPECT_RATIO;
			m_staticMB = DEFAULT_STATIC_MB;
			m_profile = eVideoProfileDummy;
			m_packetPayloadFormat = eVideoPacketPayloadFormatDummy;
			m_maxDPB = INVALID;
			m_bUseIntermediateSDResolution = params.GetUseIntermediateSDResolution();
			m_bEncodeBFramesInRTV = params.GetIsEncodeRTVBFrame();
			m_dwFrThreshold = params.GetFrThreshold();
			m_msftSvcParamsStruct = newMsSvcParams;
			TRACEINTO << m_msftSvcParamsStruct;
		}
		else
		{
			m_videoQcifFrameRate = params.GetVideoFrameRate(eVideoResolutionQCIF);
			m_videoCifFrameRate = params.GetVideoFrameRate(eVideoResolutionCIF);
			m_video4CifFrameRate = params.GetVideoFrameRate(eVideoResolution4CIF);
			m_MBPS = INVALID;
			m_FS = INVALID;
			m_sampleAspectRatio = DEFAULT_SAMPLE_ASPECT_RATIO;
			m_staticMB = DEFAULT_STATIC_MB;
			m_profile = eVideoProfileDummy;
			m_packetPayloadFormat = eVideoPacketPayloadFormatDummy;
			m_maxDPB = INVALID;
			m_bIsTipMode = false;
			m_bUseIntermediateSDResolution = false;
			m_bEncodeBFramesInRTV = false;
			m_dwFrThreshold = 0;
		}

		CSegment msg;
		msg << BYTE(statOK);

		// Need to add Ack params to Acknowledge msg to party
		if (params.GetAckParams())
		{
			msg << true;
			params.GetAckParams()->Serialize(NATIVE, msg);
		}
		else
			msg << false;

		m_pBridgePartyCntl->HandleEvent(&msg, 0, PARTY_VIDEO_OUT_UPDATED);
	}
	else
	{
		if (m_pWaitingForUpdateParams)
			POBJDELETE(m_pWaitingForUpdateParams);

		m_pWaitingForUpdateParams = new CBridgePartyVideoParams;
		*m_pWaitingForUpdateParams = params;
		TRACEINTO << "MsSvcParams:" << m_pWaitingForUpdateParams->MsSvcParams();
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyUpdateVideoParamsCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	CBridgePartyVideoUniDirection::OnVideoBridgePartyUpdateVideoParamsCONNECTED(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyUpdateVideoParamsCHANGELAYOUT(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	CBridgePartyVideoParams* pBridgePartyVideoParams = new CBridgePartyVideoParams;
	pBridgePartyVideoParams->DeSerialize(NATIVE, *pParam);

	SaveAndSendUpdatedVideoParams(pBridgePartyVideoParams);

	POBJDELETE(pBridgePartyVideoParams);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::SaveAndSendUpdatedVideoParams(CBridgePartyVideoParams* pParams)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	if (!pParams)
	{
		PASSERTMSG(1, "Internal Error received invalid params");

		CSegment msg;
		msg << BYTE(statIllegal) << false;
		m_pBridgePartyCntl->HandleEvent(&msg, 0, PARTY_VIDEO_OUT_UPDATED);

		return;
	}

	DWORD newVideoAlg = pParams->GetVideoAlgorithm();
	DWORD newVideoBitRate = pParams->GetVideoBitRate();
	eVideoFrameRate newVideoQCifFrameRate = pParams->GetVideoFrameRate(eVideoResolutionQCIF);
	eVideoFrameRate newVideoCifFrameRate = pParams->GetVideoFrameRate(eVideoResolutionCIF);
	eVideoFrameRate newVideo4CifFrameRate = pParams->GetVideoFrameRate(eVideoResolution4CIF);
	eVideoResolution newVideoResolution = pParams->GetVideoResolution();
	DWORD newMBPS = pParams->GetMBPS();
	DWORD newFS = pParams->GetFS();
	DWORD newSampleAspectRatio = pParams->GetSampleAspectRatio();
	DWORD newStaticMB = pParams->GetStaticMB();
	DWORD newMaxDPB = pParams->GetMaxDPB();
	EVideoResolutionTableType newResolutionTableType = pParams->GetVideoResolutionTableType();
	eVideoProfile newProfile = pParams->GetProfile();
	eVideoPacketPayloadFormat newPacketPayloadFormat = pParams->GetPacketFormat();
	BYTE bIsTipMode = pParams->GetIsTipMode();
	BYTE bUseIntermediateSDResolution = pParams->GetUseIntermediateSDResolution();
	BYTE bEncodeBFramesInRTV = pParams->GetIsEncodeRTVBFrame();
	CVideoBridge* pBridge = (CVideoBridge*) m_pBridgePartyCntl->GetBridge();
	DWORD dwFrThreshold = pParams->GetFrThreshold();
	const MsSvcParamsStruct& newMsSvcParams = pParams->MsSvcParams();
	const MsFullPacsiInfoStruct& newPACSI = pParams->PACSI();

	DWORD croppingHor, croppingVer;
	pBridge->GetCroppingValues(croppingHor, croppingVer);

	if (newVideoAlg == H264 || newVideoAlg == RTV)
	{
		newVideoQCifFrameRate = eVideoFrameRateDUMMY;
		newVideoCifFrameRate = eVideoFrameRateDUMMY;
		newVideo4CifFrameRate = eVideoFrameRateDUMMY;
	}

	const bool changedMsSvc = MS_SVC == newVideoAlg && newMsSvcParams.ssrc && m_msftSvcParamsStruct != newMsSvcParams;
	const bool changedPACSI = MS_SVC == newVideoAlg && m_PACSI != newPACSI;

	if (changedMsSvc)
		TRACEINTO << "MsSvcParams:" << newMsSvcParams;

	if (changedPACSI)
		TRACEINTO << "changedPACSI = " << changedPACSI << ", newPACSI:" << newPACSI;



	if (m_videoAlg == newVideoAlg && m_videoBitRate == newVideoBitRate && m_videoQcifFrameRate == newVideoQCifFrameRate
					&& m_videoCifFrameRate == newVideoCifFrameRate && m_video4CifFrameRate == newVideo4CifFrameRate
					&& m_videoResolution == newVideoResolution && m_MBPS == newMBPS && m_FS == newFS
					&& m_sampleAspectRatio == newSampleAspectRatio && m_staticMB == newStaticMB
					&& m_eResolutionTableType == newResolutionTableType && m_croppingHor == croppingHor
					&& m_croppingVer == croppingVer && m_profile == newProfile && m_packetPayloadFormat == newPacketPayloadFormat
					&& m_maxDPB == newMaxDPB && m_bIsTipMode == bIsTipMode && m_bEncodeBFramesInRTV == bEncodeBFramesInRTV
					&& GetFontType() == GetFontTypeFromConf() && !changedMsSvc && !changedPACSI)
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - No real change in Params";
	}
	else
	{
		PASSERT_AND_RETURN(!m_pHardwareInterface);

		DWORD oldFS = m_FS;
		DWORD newFS_temp = newFS; // TODO - temporary- not needed if MS_SVC FS values are safe to keep in the m_FS member (currently these values are kept separate)

		DWORD oldResolutionRatio = ((CVideoHardwareInterface*) m_pHardwareInterface)->TranslateToVideoResolutionRatio(m_videoAlg,
						m_videoResolution, oldFS, m_MBPS, m_videoConfType);
		DWORD newResolutionRatio = ((CVideoHardwareInterface*) m_pHardwareInterface)->TranslateToVideoResolutionRatio(newVideoAlg,
						newVideoResolution, newFS_temp, newMBPS, m_videoConfType);

		m_videoAlg = newVideoAlg;
		m_videoBitRate = newVideoBitRate;
		m_eResolutionTableType = newResolutionTableType;
		m_videoResolution = newVideoResolution; // In COP the resolution isn't relevant onlt for H263
		m_fontType = GetFontTypeFromConf();
		m_isH263Plus = pParams->GetIsH263Plus();

		switch (m_videoAlg)
		{
		case H264:
		case MS_SVC:
		case RTV:
		case VP8:	// amir 7-5
			m_videoQcifFrameRate = eVideoFrameRateDUMMY;
			m_videoCifFrameRate = eVideoFrameRateDUMMY;
			m_video4CifFrameRate = eVideoFrameRateDUMMY;

			m_MBPS = newMBPS;
			m_FS = newFS;

			m_bUseIntermediateSDResolution = bUseIntermediateSDResolution;

			if (m_videoAlg == RTV)
			{
				m_staticMB = DEFAULT_STATIC_MB;
				m_profile = eVideoProfileDummy;
				m_packetPayloadFormat = eVideoPacketPayloadFormatDummy;
				m_sampleAspectRatio = DEFAULT_SAMPLE_ASPECT_RATIO;
				m_maxDPB = INVALID;
				m_bEncodeBFramesInRTV = bEncodeBFramesInRTV;
			}
			else
			{
				m_bIsTipMode = bIsTipMode;

				m_staticMB = newStaticMB;
				m_profile = newProfile;
				m_packetPayloadFormat = newPacketPayloadFormat;
				m_sampleAspectRatio = newSampleAspectRatio;
				m_maxDPB = newMaxDPB;
				m_bEncodeBFramesInRTV = false;
			}

			m_dwFrThreshold = pParams->GetFrThreshold();
			m_videoFrameRate = pParams->GetVidFrameRate();
			m_msftSvcParamsStruct = newMsSvcParams;
			TRACEINTO << m_msftSvcParamsStruct;
			break;

		default:
			m_videoQcifFrameRate = newVideoQCifFrameRate;
			m_videoCifFrameRate = newVideoCifFrameRate;
			m_video4CifFrameRate = newVideo4CifFrameRate;

			if (m_video4CifFrameRate == eVideoFrameRateDUMMY && m_videoCifFrameRate != eVideoFrameRateDUMMY)
			{
				m_videoResolution = eVideoResolutionCIF;
				TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Force H263 resolution to be CIF";
			}

			m_MBPS = INVALID;
			m_FS = INVALID;
			m_profile = eVideoProfileDummy;
			m_packetPayloadFormat = eVideoPacketPayloadFormatDummy;
			m_sampleAspectRatio = DEFAULT_SAMPLE_ASPECT_RATIO;
			m_staticMB = DEFAULT_STATIC_MB;
			m_maxDPB = INVALID;
			m_bIsTipMode = false;
			m_bUseIntermediateSDResolution = false;
			m_bEncodeBFramesInRTV = false;
			m_dwFrThreshold = 0;

			break;
		}

		m_croppingHor = croppingHor;
		m_croppingVer = croppingVer;

		bool isLprActrive_old = m_isLprActive;
		m_isLprActive = pParams->GetAckParams() && pParams->GetAckParams()->IsLprOn();

		if (isLprActrive_old != m_isLprActive)
			TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", isLprActive:" << m_isLprActive
							<< " - Lost Packet status has changed";

		if (changedMsSvc)
		{
			m_msftSvcParamsStruct = newMsSvcParams;
			TRACEINTO << m_msftSvcParamsStruct;
		}

		if (changedPACSI)
			m_PACSI = pParams->PACSI();
		// Modify visual effects for AV-MCU Party
		ModifyVisuallEffectsForAVMCUParty(m_pPartyVisualEffects);

		SendUpdateEncoder();

		if (oldResolutionRatio != newResolutionRatio)
		{
			TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", oldResolutionRatio:" << oldResolutionRatio
							<< ", newResolutionRatio:" << newResolutionRatio;

			m_waitForUpdateEncoderAck = true;
			StartTimer(ENCODER_UPDATE_PARAM_TOUT, 8 * SECOND);
		}
	}

	CSegment msg;
	msg << BYTE(statOK);

	if (pParams->GetAckParams())
	{
		msg << true;
		pParams->GetAckParams()->Serialize(NATIVE, msg);
	}
	else
		msg << false;

	m_pBridgePartyCntl->HandleEvent(&msg, 0, PARTY_VIDEO_OUT_UPDATED);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyUpdateVideoParamsDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Event is ignored";
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyFastUpdateSLIDE(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	PASSERT_AND_RETURN(!m_pHardwareInterface);

	((CVideoHardwareInterface*) m_pHardwareInterface)->SendIvrFastUpdate();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyFastUpdateSETUP(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Event is ignored";
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyFastUpdateCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	OnVideoBridgePartyFastUpdate(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyFastUpdateCHANGELAYOUT(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	OnVideoBridgePartyFastUpdate(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyFastUpdate(CSegment* pParam)
{
	PASSERT_AND_RETURN(!m_pHardwareInterface);
	((CVideoHardwareInterface*) m_pHardwareInterface)->SendFastUpdate();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyAddImageCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	CTaskApp* pAddedParty;

	*pParam >> (DWORD&) pAddedParty;
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pAddedParty));

	BOOL bUseSharedMemForChangeLayoutReq = true;
	BuildLayout(bUseSharedMemForChangeLayoutReq);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyUpdateImageCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	BOOL bUseSharedMemForChangeLayoutReq = true;
	BYTE alwaysSendToHardware = true;
	BuildLayout(bUseSharedMemForChangeLayoutReq, alwaysSendToHardware);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyUpdateImageCHANGELAYOUT(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	ON(m_imageResolutionChangedWhileWaitingForAck);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyNotifyCHANGELAYOUT(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	ON(m_layoutChangedWhileWaitingForAck);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyUnMuteImageCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	BOOL bUseSharedMemForChangeLayoutReq = true;
	BuildLayout(bUseSharedMemForChangeLayoutReq);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyMuteImageCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	BOOL bUseSharedMemForChangeLayoutReq = true;
	BuildLayout(bUseSharedMemForChangeLayoutReq);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyEncoderResolutionChangedCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	BuildLayoutAndSendOnEncoderResolutionChange();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyEncoderResolutionChangedCHANGELAYOUT(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	ON(m_resolutionChangedWhileWaitingForAck);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyChangeAudioSpeakerCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	CTaskApp* pNewAudioSpeaker;
	*pParam >> (DWORD&) pNewAudioSpeaker;

	if (m_pCurrentView->GetNumberOfSubImages() != 1 && m_pPartyVisualEffects->IsSpeakerNotationEnable()
					&& (pNewAudioSpeaker != m_pLastSpeakerNotationParty))
	{
		BYTE isFromAudioSpeakerChange = YES;
		SendChangeLayoutAttributesToHardware(isFromAudioSpeakerChange); //Remove Notation
		m_pLastSpeakerNotationParty = NULL;
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyChangeAudioSpeakerCHANGELAYOUT(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	CTaskApp* pNewAudioSpeaker;
	*pParam >> (DWORD&) pNewAudioSpeaker;

	if (m_pCurrentView->GetNumberOfSubImages() != 1 && m_pPartyVisualEffects->IsSpeakerNotationEnable())
	{ // Only Speaker Notation Changed
		ON(m_visualEffectsChangedWhileWaitingForAck);
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyChangeSpeakersCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	CTaskApp* pNewVideoSpeaker;
	CTaskApp* pNewAudioSpeaker;

	*pParam >> (DWORD&) pNewVideoSpeaker;
	*pParam >> (DWORD&) pNewAudioSpeaker;

	BOOL bUseSharedMemForChangeLayoutReq = true;
	BuildLayout(bUseSharedMemForChangeLayoutReq);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyChangeSpeakersCHANGELAYOUT(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	CTaskApp* pNewVideoSpeaker;
	CTaskApp* pNewAudioSpeaker;

	*pParam >> (DWORD&) pNewVideoSpeaker;
	*pParam >> (DWORD&) pNewAudioSpeaker;

	ON(m_layoutChangedWhileWaitingForAck);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyDelImageCONNECTED(CSegment* pParam)
{
	// vngr-7017 "bombing" logger on change layout - change trace to DEBUG level
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	CTaskApp* pDelParty;
	*pParam >> (DWORD&) pDelParty;

	DBGPASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pDelParty));

	BOOL bUseSharedMemForChangeLayoutReq = true;
	BuildLayout(bUseSharedMemForChangeLayoutReq);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyChangeConfLayoutCONNECTED(CSegment* pParam)
{
	if (RejectChangeLayoutRequestBecauseOfApplications())
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Rejected because lecturerMode is eLISTENER or eCOLD_LISTENER";
		return;
	}

	if (GetIsPrivateLayout())
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Rejected because private layout is selected";
		return;
	}

	LayoutType confLayoutType = GetConfLayoutType();

	// VSGNINJA-881
	if (m_isForce1x1 && (CP_LAYOUT_1X1 != confLayoutType))
	{
		SetPrivateLayoutForParty(CP_LAYOUT_1X1);
	}

	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	DWORD dwStub = 0;
	BYTE bAnyway = 0;
	*pParam >> dwStub;
	*pParam >> bAnyway;

	BOOL bUseSharedMemForChangeLayoutReq = true;
	BuildLayout(bUseSharedMemForChangeLayoutReq, bAnyway);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyChangeConfLayoutCHANGELAYOUT(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	if (RejectChangeLayoutRequestBecauseOfApplications() || GetIsPrivateLayout())
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Rejected";
		return;
	}

	LayoutType confLayoutType = GetConfLayoutType();

	// VSGNINJA-881
	if (m_isForce1x1 && (CP_LAYOUT_1X1 != confLayoutType))
	{
		SetPrivateLayoutForParty(CP_LAYOUT_1X1);
	}

	ON(m_layoutChangedWhileWaitingForAck);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyChangePartyLayoutCONNECTED(CSegment* pParam)
{
	CVideoLayout layout;
	layout.DeSerialize(NATIVE, *pParam);

	LayoutType newLayoutType = GetNewLayoutType(layout.GetScreenLayout());
	const bool wasPrivate = GetIsPrivateLayout();

	if (RejectChangeLayoutRequestBecauseOfApplications())
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Rejected";
		return;
	}

	LayoutType confLayoutType = GetConfLayoutType();

	if (!wasPrivate && (newLayoutType != confLayoutType)) //this means it is an inactive layout - that we are just updating forces
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Inactive layout (just updating forces)";
		m_pReservation[newLayoutType]->SetLayout(layout, PARTY_lev);
		return;
	}

	if (!wasPrivate && (*m_pReservation[confLayoutType]) == layout)
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Received same layout";
		return;
	}

	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", LayoutType:" << (int) confLayoutType;

	if (m_isForce1x1 && (CP_LAYOUT_1X1 != newLayoutType))
	{
		TRACEINTO << "This is force layout 1X1 by system flag and cascading link slave in conference layout.";
		return;
	}

	SetIsPrivateLayout(NO);

	m_pReservation[confLayoutType]->SetLayoutType(newLayoutType);
	m_pReservation[confLayoutType]->SetLayout(layout, PARTY_lev);

	BuildLayout();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgeChangeLayoutOfTPRoomSublinkCHANGELAYOUT(CSegment* pParam)
{
	TRACEINTO << "received telepresence layout for " << m_pBridgePartyCntl->GetFullName() << " rsrc. ID "
					<< m_pBridgePartyCntl->GetPartyRsrcID();
	CLayout* pSublinkLayout = NULL;
	BORDERS_PARAM_S* pTelepresenceCellBorders = NULL;

	*pParam >> (DWORD&) pSublinkLayout;
	*pParam >> (DWORD&) pTelepresenceCellBorders;

	if (CPObject::IsValidPObjectPtr(pSublinkLayout))
	{
		CLayout* pNewView = new CLayout(*pSublinkLayout);
		POBJDELETE(m_pCurrentView);
		m_pCurrentView = pNewView;
		// EMB_MLA_GUY update cell borders according to TP layout (26/6/14 currently for speaker mode, in filmstrip only)
		std::ostringstream msg;
		DumpTelepresenceBordersInfo(msg, pTelepresenceCellBorders);
		TRACEINTO << msg.str().c_str();
		m_pPartyVisualEffects->SetSpecifiedBorders(pTelepresenceCellBorders);

		m_layoutTPRoomSublinkChangedWhileWaitingForAck = true;
	}
	else
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Failed, layout object is not valid";
	}
	//???????????????????????????
	/*TRACEINTO << m_pBridgePartyCntl->GetFullName();

	 if (RejectChangeLayoutRequestBecauseOfApplications() || GetIsPrivateLayout())
	 {
	 TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Rejected";
	 return;
	 }

	 LayoutType confLayoutType = GetConfLayoutType();

	 // VSGNINJA-881
	 if ( m_isForce1x1 && ( CP_LAYOUT_1X1 != confLayoutType  ) )
	 {
	 SetPrivateLayoutForParty(CP_LAYOUT_1X1);
	 }

	 TRACEINTO << m_pBridgePartyCntl->GetFullName();

	 ON(m_layoutChangedWhileWaitingForAck);*/
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgeChangeLayoutOfTPRoomSublinkCONNECTED(CSegment* pParam)
{
	TRACEINTO << "received telepresence layout for " << m_pBridgePartyCntl->GetFullName() << " rsrc. ID "
					<< m_pBridgePartyCntl->GetPartyRsrcID();
	CLayout* pSublinkLayout = NULL;
	BORDERS_PARAM_S* pTelepresenceCellBorders = NULL;

	*pParam >> (DWORD&) pSublinkLayout;
	*pParam >> (DWORD&) pTelepresenceCellBorders;
	if (CPObject::IsValidPObjectPtr(pSublinkLayout))
	{
		CLayout* pNewView = new CLayout(*pSublinkLayout);
		// layout comparison for TP sublinks is checked in the CLayoutHandlerTelepresenceCP object
		bool isSameLayout = false; // *pNewView == *m_pCurrentView;
		POBJDELETE(m_pCurrentView);
		m_pCurrentView = pNewView;
		// EMB_MLA_GUY update cell borders according to TP layout (26/6/14 currently for speaker mode, in filmstrip only)
		std::ostringstream msg;
		DumpTelepresenceBordersInfo(msg, pTelepresenceCellBorders);
		TRACEINTO << msg.str().c_str();
		m_pPartyVisualEffects->SetSpecifiedBorders(pTelepresenceCellBorders);
		if (isSameLayout)
		{
			TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Layout did not changed";
			// don't send double change layout to video card
			// inform bridge party control for end change layout
			OnMplChangeLayoutAck(STATUS_OK);
		}
		else
		{
			// layout changed
			BOOL bUseSharedMemForChangeLayoutReq = false;
			BOOL bSent = false;
			//// get filmstrip? fill borders
			SendChangeLayoutToHardware(bUseSharedMemForChangeLayoutReq, NO, &bSent);
			if (!bSent)
			{
				m_layoutTPRoomSublinkChangedWhileWaitingForAck = true;
				TRACESTRFUNC(eLevelWarn) << "EMB_MLA_OLGA - partyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << " change layout isn't sent";
			}
		}
	}
	else
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Failed, layout is not valid";
	}
}
//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyChangePartyLayoutCHANGELAYOUT(CSegment* pParam)
{
	CVideoLayout layout;
	layout.DeSerialize(NATIVE, *pParam);

	LayoutType newLayoutType = GetNewLayoutType(layout.GetScreenLayout());

	if (RejectChangeLayoutRequestBecauseOfApplications())
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Rejected";
		return;
	}

	LayoutType confLayoutType = GetConfLayoutType();

	if (newLayoutType != confLayoutType)			//this means it is an inactive layout - that we are just updating forces
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Conference level handling";
		m_pReservation[newLayoutType]->SetLayout(layout, PARTY_lev);
		return;
	}

	const bool wasPrivate = GetIsPrivateLayout();

	if (!wasPrivate && (*m_pReservation[confLayoutType]) == layout)
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Received same layout";
		return;
	}

	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", LayoutType:" << (int) confLayoutType;

	SetIsPrivateLayout(false);

	m_pReservation[confLayoutType]->SetLayout(layout, PARTY_lev);

	ON(m_layoutChangedWhileWaitingForAck);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyChangePrivateLayoutSLIDE(CSegment* pParam)
{
	CVideoLayout layout;
	layout.DeSerialize(NATIVE, *pParam);

	if (RejectChangeLayoutRequestBecauseOfApplications())
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Rejected";
		return;
	}

	const bool wasPrivate = GetIsPrivateLayout();

	LayoutType newLayoutType = GetNewLayoutType(layout.GetScreenLayout());

	if (newLayoutType >= CP_NO_LAYOUT)
	{
		PASSERTMSG(101, "Illegal Layout type received");
		return;
	}

	if (wasPrivate)
	{
		if ((m_PrivatelayoutType != CP_NO_LAYOUT) && ((*m_pPrivateReservation[m_PrivatelayoutType]) == layout))
		{
			TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Received same layout";
			return;
		}
	}

	m_pPrivateReservation[newLayoutType]->SetLayout(layout, PARTY_lev, 1);

	//case when send layout just for setting reservations
	if (!layout.IsActive())
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Layout is not active";
		return;
	}

	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", LayoutType:" << (int) newLayoutType;

	m_pPrivateReservation[newLayoutType]->SetCurrActiveLayout(YES);
	SetIsPrivateLayout(true);

	if (m_PrivatelayoutType != CP_NO_LAYOUT && m_PrivatelayoutType != newLayoutType)
		m_pPrivateReservation[m_PrivatelayoutType]->SetCurrActiveLayout(NO);

	m_PrivatelayoutType = newLayoutType;
	m_pBridgePartyCntl->HandleEvent(NULL, 0, PARTYLAYOUTCHANGED);
	m_isPrivateChanged = false;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyChangePrivateLayoutSETUP(CSegment* pParam)
{
	CVideoLayout layout;
	layout.DeSerialize(NATIVE, *pParam);

	if (RejectChangeLayoutRequestBecauseOfApplications())
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Rejected";
		return;
	}

	const bool wasPrivate = GetIsPrivateLayout();

	LayoutType newLayoutType = GetNewLayoutType(layout.GetScreenLayout());

	if (newLayoutType >= CP_NO_LAYOUT)
	{
		PASSERTMSG(101, "Illegal Layout type received");
		return;
	}

	if (wasPrivate)
	{
		if ((m_PrivatelayoutType != CP_NO_LAYOUT) && ((*m_pPrivateReservation[m_PrivatelayoutType]) == layout))
		{
			TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Received same layout";
			return;
		}
	}

	m_pPrivateReservation[newLayoutType]->SetLayout(layout, PARTY_lev, 1);

	//case when send layout just for setting reservations
	if (!layout.IsActive())
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Layout is not active";
		return;
	}

	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", LayoutType:" << (int) newLayoutType;

	m_pPrivateReservation[newLayoutType]->SetCurrActiveLayout(YES);
	SetIsPrivateLayout(true);

	if (m_PrivatelayoutType != CP_NO_LAYOUT && m_PrivatelayoutType != newLayoutType)
		m_pPrivateReservation[m_PrivatelayoutType]->SetCurrActiveLayout(NO);

	m_PrivatelayoutType = newLayoutType;
	m_pBridgePartyCntl->HandleEvent(NULL, 0, PARTYLAYOUTCHANGED);
	m_isPrivateChanged = false;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyChangePrivateLayoutCONNECTED(CSegment* pParam)
{
	CVideoLayout layout;
	layout.DeSerialize(NATIVE, *pParam);

	if (RejectChangeLayoutRequestBecauseOfApplications())
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Rejected";
		return;
	}

	const bool wasPrivate = GetIsPrivateLayout();

	LayoutType newLayoutType = GetNewLayoutType(layout.GetScreenLayout());

	if (newLayoutType >= CP_NO_LAYOUT)
	{
		PASSERTMSG(101, "Illegal Layout type received");
		return;
	}
	if (wasPrivate)
	{
		if ((m_PrivatelayoutType != CP_NO_LAYOUT) && ((*m_pPrivateReservation[m_PrivatelayoutType]) == layout))
		{
			TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Received same layout";
			return;
		}
	}

	if (m_isForce1x1 && (CP_LAYOUT_1X1 != newLayoutType))
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName()
						<< " - This is force layout 1X1 by system flag and cascading link in private layout";
		return;
	}

	m_pPrivateReservation[newLayoutType]->SetLayout(layout, PARTY_lev, 1);

	//case when send layout just for setting reservations
	if (!layout.IsActive())
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Layout is not active";
		return;
	}

	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", LayoutType:" << (int) newLayoutType;

	m_pPrivateReservation[newLayoutType]->SetCurrActiveLayout(YES);
	SetIsPrivateLayout(true);

	if (m_PrivatelayoutType != CP_NO_LAYOUT && m_PrivatelayoutType != newLayoutType)
		m_pPrivateReservation[m_PrivatelayoutType]->SetCurrActiveLayout(NO);

	m_PrivatelayoutType = newLayoutType;
	BuildLayout();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyChangePrivateLayoutCHANGELAYOUT(CSegment* pParam)
{
	CVideoLayout layout;
	layout.DeSerialize(NATIVE, *pParam);

	if (RejectChangeLayoutRequestBecauseOfApplications())
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Rejected";
		return;
	}

	const bool wasPrivate = GetIsPrivateLayout();

	LayoutType newLayoutType = GetNewLayoutType(layout.GetScreenLayout());
	if (newLayoutType >= CP_NO_LAYOUT)
	{
		PASSERTMSG(101, "Illegal Layout type received");
		return;
	}

	if (wasPrivate)
	{
		// Tsahi - VNGR-24624
		if ((m_PrivatelayoutType != CP_NO_LAYOUT) && (*m_pPrivateReservation[m_PrivatelayoutType]) == layout)
		{
			TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Received same layout";
			return;
		}
	}

	m_pPrivateReservation[newLayoutType]->SetLayout(layout, PARTY_lev, 1);

	//case when send layout just for setting reservations
	if (!layout.IsActive())
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Layout is not active";
		return;
	}

	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", LayoutType:" << (int) newLayoutType;

	m_pPrivateReservation[newLayoutType]->SetCurrActiveLayout(YES);
	SetIsPrivateLayout(true);

	if (m_PrivatelayoutType != CP_NO_LAYOUT && m_PrivatelayoutType != newLayoutType)
		m_pPrivateReservation[m_PrivatelayoutType]->SetCurrActiveLayout(NO);

	m_PrivatelayoutType = newLayoutType;
	ON(m_layoutChangedWhileWaitingForAck);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartySetPrivateLayoutOnOffCONNECTED(CSegment* pParam)
{
	DWORD isPrivate;

	*pParam >> isPrivate;

	if (RejectChangeLayoutRequestBecauseOfApplications())
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Rejected";
		return;
	}
	if (m_isForce1x1)
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Rejected - Party is  a Cascaded link and forced to 1X1 layout";
		return;
	}

	if (GetIsPrivateLayout() == isPrivate)
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - No change in private layout, ignoring request";
	}

	SetIsPrivateLayout(isPrivate);

	if (isPrivate == NO) // In this case - we're returning to conference layout
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", TelepresenceModeChanged:" << (WORD) m_telepresenceModeChanged
						<< " - Returning to conf layout";
		if (m_telepresenceModeChanged)
		{
			BOOL bUseSharedMemForChangeLayoutReq = false;
			BYTE alwaysSendToHardware = true; // will always send the new layout to hardware
			BuildLayout(bUseSharedMemForChangeLayoutReq, alwaysSendToHardware);
		}
		else
			BuildLayout();
	}
	else // Case we're first time in Private layout or only button pushed
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", TelepresenceModeChanged:" << (WORD) m_telepresenceModeChanged
						<< " - Moving to private layout";

		if (m_PrivatelayoutType != CP_NO_LAYOUT) // Means - we will set the last active private layout.
		{
			TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Last active private layout";
			BuildLayout();
		}
		else // Means - this is the first time in Private layout - and the user just pressed the button
		{ // In this case we'll apply the conference layout and make it the first active private layout
			TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Make conf layout to private layout";

			const LayoutType newLayoutType = GetConfLayoutType();

			if (newLayoutType != CP_NO_LAYOUT)
				m_pPrivateReservation[newLayoutType]->SetCurrActiveLayout(YES);

			m_PrivatelayoutType = newLayoutType;

			CSegment* pSeg = new CSegment;
			*pSeg << (WORD) YES;
			m_pBridgePartyCntl->HandleEvent(pSeg, 0, PRIVATELAYOUT_ONOFF_CHANGED);
			POBJDELETE(pSeg);
		}
	}

	m_telepresenceModeChanged = false;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartySetPrivateLayoutOnOffCHANGELAYOUT(CSegment* pParam)
{
	DWORD isPrivate;

	*pParam >> isPrivate;

	if (RejectChangeLayoutRequestBecauseOfApplications())
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Rejected";
		return;
	}
	if (m_isForce1x1)
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Rejected - Party is  a Cascaded link and forced to 1X1 layout";
		return;
	}

	if (GetIsPrivateLayout() == isPrivate)
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - No change in private layout, ignoring request";
	}

	SetIsPrivateLayout(isPrivate);

	if (isPrivate == NO) // In this case - we're returning to conference layout
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Returning to conf layout";
		ON(m_layoutChangedWhileWaitingForAck);
	}
	else // Case we're first time in Private layout or only button pushed
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Moving to private layout";

		if (m_PrivatelayoutType != CP_NO_LAYOUT) // Means - we will set the last active private layout.
		{
			TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Last active private layout";
			ON(m_layoutChangedWhileWaitingForAck);
		}
		else // Means - this is the first time in Private layout - and the user just pressed the button
		{ // In this case we'll apply the conference layout and make it the first active private layout
			TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Make conf layout to private layout";

			const LayoutType newLayoutType = GetConfLayoutType();

			if (newLayoutType != CP_NO_LAYOUT)
				m_pPrivateReservation[newLayoutType]->SetCurrActiveLayout(YES);

			m_PrivatelayoutType = newLayoutType;

			CSegment* pSeg = new CSegment;
			*pSeg << (WORD) YES;
			m_pBridgePartyCntl->HandleEvent(pSeg, 0, PRIVATELAYOUT_ONOFF_CHANGED);
			POBJDELETE(pSeg);
		}
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyStartPLC(CSegment* pParam)
{
	if (RejectPLCRequestBecauseOfApplications(VIDEO_GRAPHIC_OVERLAY_START_REQ))
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Rejected";
		return;
	}
	else
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName();
		((CVideoHardwareInterface *) m_pHardwareInterface)->SendStartPLC(pParam);
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyStopPLC(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	((CVideoHardwareInterface *) m_pHardwareInterface)->SendStopPLC(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyPLC_SetPrivateLayoutTypeCONNECTED(CSegment* pParam)
{
	if (RejectPLCRequestBecauseOfApplications(PLC_SETPARTYPRIVATELAYOUTTYPE))
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Rejected";
		return;
	}

	OnVideoBridgePartyPLC_SetPrivateLayoutType(pParam);

	BuildLayout();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyPLC_SetPrivateLayoutTypeCHANGELAYOUT(CSegment* pParam)
{
	if (RejectPLCRequestBecauseOfApplications(PLC_SETPARTYPRIVATELAYOUTTYPE))
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Rejected";
		return;
	}

	OnVideoBridgePartyPLC_SetPrivateLayoutType(pParam);

	ON(m_layoutChangedWhileWaitingForAck);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyPLC_SetPrivateLayoutType(CSegment* pParam)
{
	BYTE tmp;
	LayoutType newPrivateLayoutType = CP_NO_LAYOUT;

	*pParam >> tmp;

	newPrivateLayoutType = (LayoutType) tmp;

	const bool wasPrivate = GetIsPrivateLayout();

	if (newPrivateLayoutType >= CP_NO_LAYOUT)
	{
		PASSERTMSG(101, "Illegal Layout type received");
		return;
	}

	if (wasPrivate)
	{
		if (m_PrivatelayoutType == newPrivateLayoutType)
		{
			TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Received same layout";
			return;
		}
	}

	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", LayoutType:" << (int) newPrivateLayoutType;

	m_pPrivateReservation[newPrivateLayoutType]->SetCurrActiveLayout(YES);
	SetIsPrivateLayout(true);

	if (m_PrivatelayoutType != CP_NO_LAYOUT && m_PrivatelayoutType != newPrivateLayoutType)
		m_pPrivateReservation[m_PrivatelayoutType]->SetCurrActiveLayout(NO);

	m_PrivatelayoutType = newPrivateLayoutType;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyVENUS_SetPrivateLayoutTypeCONNECTED(CSegment* pParam)
{
	if (RejectPLCRequestBecauseOfApplications(VENUS_SETPARTYPRIVATELAYOUTTYPE))
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Rejected";
		return;
	}

	OnVideoBridgePartyVENUS_SetPrivateLayoutType(pParam);

	BuildLayout();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyVENUS_SetPrivateLayoutTypeCHANGELAYOUT(CSegment* pParam)
{
	if (RejectPLCRequestBecauseOfApplications(VENUS_SETPARTYPRIVATELAYOUTTYPE))
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Rejected";
		return;
	}

	OnVideoBridgePartyVENUS_SetPrivateLayoutType(pParam);

	ON(m_layoutChangedWhileWaitingForAck);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyVENUS_SetPrivateLayoutType(CSegment* pParam)
{
	BYTE tmp;
	LayoutType newPrivateLayoutType = CP_NO_LAYOUT;

	*pParam >> tmp;

	newPrivateLayoutType = (LayoutType) tmp;

	const bool wasPrivate = GetIsPrivateLayout();

	if (newPrivateLayoutType >= CP_NO_LAYOUT)
	{
		PASSERTMSG(101, "Illegal Layout type received");
		return;
	}

	if (wasPrivate)
	{
		if (m_PrivatelayoutType == newPrivateLayoutType)
		{
			TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Received same layout";
			return;
		}
	}

	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", LayoutType:" << (int) newPrivateLayoutType;

	m_pPrivateReservation[newPrivateLayoutType]->SetCurrActiveLayout(YES);
	SetIsPrivateLayout(true);

	if (m_PrivatelayoutType != CP_NO_LAYOUT && m_PrivatelayoutType != newPrivateLayoutType)
		m_pPrivateReservation[m_PrivatelayoutType]->SetCurrActiveLayout(NO);

	m_PrivatelayoutType = newPrivateLayoutType;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyPLC_ReturnToConfLayoutCONNECTED(CSegment* pParam)
{
	OnVideoBridgePartyPLC_ReturnToConfLayout(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyPLC_ReturnToConfLayoutCHANGELAYOUT(CSegment* pParam)
{
	OnVideoBridgePartyPLC_ReturnToConfLayout(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyPLC_ReturnToConfLayout(CSegment* pParam)
{
	if (RejectPLCRequestBecauseOfApplications(PLC_RETURNPARTYTOCONFLAYOUT))
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Rejected";
		return;
	}

	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD) false;
	DispatchEvent(SETPARTYPRIVATELAYOUTONOFF, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyPLC_ForceCellCONNECTED(CSegment* pParam)
{
	if (RejectPLCRequestBecauseOfApplications(PLC_FORCECELLZERO))
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Rejected";
		return;
	}

	OnVideoBridgePartyPLC_ForceCell(pParam);

	BuildLayout();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyPLC_ForceCellCHANGELAYOUT(CSegment* pParam)
{
	if (RejectPLCRequestBecauseOfApplications(PLC_FORCECELLZERO))
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Rejected";
		return;
	}

	OnVideoBridgePartyPLC_ForceCell(pParam);

	ON(m_layoutChangedWhileWaitingForAck);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyPLC_ForceCell(CSegment* pParam)
{
	char forcedImagePartyName[H243_NAME_LEN];
	BYTE cellToForce = 0;
	*pParam >> forcedImagePartyName;
	*pParam >> cellToForce;

	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", ForcedImagePartyName:" << forcedImagePartyName;

	const bool wasPrivate = GetIsPrivateLayout();

	if (!wasPrivate) // In this case we'll apply the conference layout and make it the active private layout
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Make conf layout to private layout";

		const LayoutType newLayoutType = GetConfLayoutType();

		if (newLayoutType != CP_NO_LAYOUT)
		{
			m_pPrivateReservation[newLayoutType]->SetCurrActiveLayout(YES);
			SetIsPrivateLayout(true);
		}

		m_PrivatelayoutType = newLayoutType;

		CSegment* pSeg = new CSegment;
		*pSeg << (WORD) YES;
		m_pBridgePartyCntl->HandleEvent(pSeg, 0, PRIVATELAYOUT_ONOFF_CHANGED);
		POBJDELETE(pSeg);
	}
	// klocwork - Romem
	if (GetPrivateReservationLayout() == NULL)
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Invalid private layout";
		return;
	}

	BYTE isExist = (GetPrivateReservationLayout())->isSetInOtherCell(forcedImagePartyName);

	if (isExist != 0xFF && isExist != 0)
	{
		if (NULL != (*GetPrivateReservationLayout())[isExist])
		{
			(*GetPrivateReservationLayout())[isExist]->SetForceAttributes(AUTO_Prior, DEFAULT_Activ);
			(*GetPrivateReservationLayout())[isExist]->RemovePartyForceName();
		}
		else
		{
			TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", Invalid layout:" << (int) isExist;
		}
	}
	if (cellToForce > 15)
	{
		DBGPASSERT(1);
		cellToForce = 0;
	}

	if (NULL != (*GetPrivateReservationLayout())[cellToForce])
	{

		(*GetPrivateReservationLayout())[cellToForce]->SetForceAttributes(OPERATOR_Prior, FORCE_PRIVATE_PARTY_Active);
		(*GetPrivateReservationLayout())[cellToForce]->SetPartyForceName(forcedImagePartyName);
	}
	else
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", Invalid layout:" << (int) isExist;
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyPLC_CancelAllPrivateLayoutForcesCONNECTED(CSegment* pParam)
{
	if (RejectPLCRequestBecauseOfApplications(PLC_CANCELALLPRIVATELAYOUTFORCES))
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Rejected";
		return;
	}

	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	OnVideoBridgePartyPLC_CancelAllPrivateLayoutForces(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyPLC_CancelAllPrivateLayoutForcesCHANGELAYOUT(CSegment* pParam)
{
	if (RejectPLCRequestBecauseOfApplications(PLC_CANCELALLPRIVATELAYOUTFORCES))
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Rejected";
		return;
	}

	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	OnVideoBridgePartyPLC_CancelAllPrivateLayoutForces(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyPLC_CancelAllPrivateLayoutForces(CSegment* pParam)
{
	for (int i = CP_LAYOUT_1X1; i < CP_NO_LAYOUT; ++i)
		m_pPrivateReservation[i]->ClearAllForces();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyChangeVisualEffectsCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	CVisualEffectsParams visualEffects;
	visualEffects.DeSerialize(NATIVE, *pParam);
	BYTE bInternalUpdateOnly = false;
	*pParam >> bInternalUpdateOnly;

	BYTE isChanged = true;

	if (visualEffects == (*m_pPartyVisualEffects))
		isChanged = false;

	if (isChanged)
	{
		if (m_pPartyVisualEffects->VisualEffectsDifferOnlyInSpeakerIndicationParams(&visualEffects))
		{
			DWORD oldSpeakerPlaceInLayout = GetAudioSpeakerPlaceInLayout();
			*m_pPartyVisualEffects = visualEffects; // we want to update the member even if it is not relevant at the moment it will be in future
			DWORD newSpeakerPlaceInLayout = GetAudioSpeakerPlaceInLayout();
			if (oldSpeakerPlaceInLayout == INVALID && newSpeakerPlaceInLayout == INVALID) // no real change therefore no need to send to hw
			{
				isChanged = false;
			}
		}
	}

	if (isChanged)
	{
		*m_pPartyVisualEffects = visualEffects;
		if (false == bInternalUpdateOnly)
			SendChangeLayoutAttributesToHardware();
		// else - if (bInternalUpdateOnly == true) --> don't send to hardware here, hardware will be updated by full change layout request (CVideoBridgeCP::OnConfTurnOnOffTelePresence)
	}
	else
		SendUpdateCroppingToHardware();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyChangeVisualEffectsCHANGELAYOUT(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	CVisualEffectsParams visualEffects;
	visualEffects.DeSerialize(NATIVE, *pParam);

	if (!(visualEffects == *m_pPartyVisualEffects))
	{
		*m_pPartyVisualEffects = visualEffects;
		if (((CVideoBridgePartyCntl*) m_pBridgePartyCntl)->IsAVMCUParty())
			m_pPartyVisualEffects->SetBackgroundImageID(0);
		ON(m_visualEffectsChangedWhileWaitingForAck);
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyChangeVisualEffectsIDLE(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	CVisualEffectsParams visualEffects;
	visualEffects.DeSerialize(NATIVE, *pParam);

	*m_pPartyVisualEffects = visualEffects;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyChangeVisualEffectsSLIDE(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	CVisualEffectsParams visualEffects;
	visualEffects.DeSerialize(NATIVE, *pParam);

	*m_pPartyVisualEffects = visualEffects;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyChangeVisualEffectsSETUP(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	CVisualEffectsParams visualEffects;
	visualEffects.DeSerialize(NATIVE, *pParam);

	*m_pPartyVisualEffects = visualEffects;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::SiteNameSerialization(CSegment* pParam)
{
	CSiteNameInfo siteNameInfo;
	siteNameInfo.DeSerialize(NATIVE, *pParam);

	*m_pSiteNameInfo = siteNameInfo;
}
//--------------------------------------------------------------------------
void CBridgePartyVideoOut::ModifyVisuallEffectsForAVMCUParty(CVisualEffectsParams* pPartyVisualEffects)
{
	PASSERT_AND_RETURN(!pPartyVisualEffects);
	if (((CVideoBridgePartyCntl*) m_pBridgePartyCntl)->IsAVMCUParty())
	{
		TRACEINTO << " Modify Visual Effects for AV-MCU Party: " << m_pBridgePartyCntl->GetFullName();
		DWORD colorBlack = 0x00108080;
		DWORD colorSkin0 = 0xFF519077;
		//pPartyVisualEffects->SetSpeakerNotationEnable(NO);
		pPartyVisualEffects->SetlayoutBorderEnable(NO);
		pPartyVisualEffects->SetlayoutBorderWidth(eLayoutBorderNone);
		pPartyVisualEffects->SetBackgroundColorRGB(0);
		pPartyVisualEffects->SetBackgroundColorYUV(colorSkin0);
		pPartyVisualEffects->SetBackgroundImageID(0);
	}
}
//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyChangeSiteNameIDLE(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	SiteNameSerialization(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyChangeSiteNameSLIDE(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	SiteNameSerialization(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyChangeSiteNameSETUP(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	SiteNameSerialization(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyChangeSiteNameCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	SiteNameSerialization(pParam);

	if (m_pSiteNameInfo->GetDisplayMode() == eSiteNameOff)
	{
		((CVideoHardwareInterface*) m_pHardwareInterface)->SendStopSiteNameDisplay();
		return;
	}
	SendChangeLayoutToHardware();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyChangeSiteNameCHANGELAYOUT(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	CSiteNameInfo siteNameInfo;
	siteNameInfo.DeSerialize(NATIVE, *pParam);
	*m_pSiteNameInfo = siteNameInfo;
	ON(m_sitenameChangedWhileWaitingForAck);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyRefreshLayoutIDLE(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Event is ignored";
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyRefreshLayoutSLIDE(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Event is ignored";
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyRefreshLayoutSETUP(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Event is ignored";
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyRefreshLayoutCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	SendChangeLayoutToHardware();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyRefreshLayoutCHANGELAYOUT(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	ON(m_visualEffectsChangedWhileWaitingForAck);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyIndicationIconsChangeCONNECTED(CSegment* pParam)
{
	BOOL bUseSharedMemForIndicationIcon = FALSE;
	*pParam >> bUseSharedMemForIndicationIcon;

	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	if (m_pCurrHandler->BuildLayoutIndications(*m_pCurrentView))
		SendIndicationIconsChangeToHardware(bUseSharedMemForIndicationIcon);

	//If audio participant indication is actived by conf, then conf will be responsibe to hidden it.
	if (GetIsAudioParticipantsIconActive() && IsValidTimer(PARTY_AUDIO_NUMBER_INDICATION_DISPLAY_TOUT))
		DeleteTimer(PARTY_AUDIO_NUMBER_INDICATION_DISPLAY_TOUT);

}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnTimerAudioNumberHiden(CSegment* pParam)
{
	TRACEINTO << " ,audio layout indication";

	if (isInGatheringMode())
	{
		//BRIDGE-11671,Sometimes, the timer is started when the party is not in gathering mode
		//So if party is in gathering mode currently, ingore this event so that we could see the icon after gathering mode ends.
		TRACEINTO << "do not hidden the audio icon since we are in gathering mode";
		return;
	}

	WORD isIconActive = NO;

	SetAudioParticipantsIconActive(isIconActive);

	CSegment* pSeg = new CSegment;
	BOOL bUseSharedMemForIndicationIcon = FALSE; //do not use share memory if it is triggered from internal
	*pSeg << (DWORD) bUseSharedMemForIndicationIcon;

	DispatchEvent(INDICATION_ICONS_CHANGE, pSeg);

	PDELETE(pSeg);

}

//--------------------------------------------------------------------------
BOOL CBridgePartyVideoOut::isInGatheringMode()
{
	TRACEINTO << "isInGatheringMode is: " << (int) m_isInGatheringMode;
	return m_isInGatheringMode;
}

void CBridgePartyVideoOut::InitIsGatheringModeEnabled()
{
	CVideoBridge* pBridge = (CVideoBridge*) m_pBridgePartyCntl->GetBridge();
	PASSERT_AND_RETURN(!pBridge);
	CConf* pConf = pBridge->GetConf();
	PASSERT_AND_RETURN(!pConf);
	const CCommConf* pCommConf = pConf->GetCommConf();
	m_isInGatheringMode = pCommConf->IsGatheringEnabled();
	if (((CVideoBridgePartyCntl*) m_pBridgePartyCntl)->IsAVMCUParty())
		m_isInGatheringMode = NO;
	TRACEINTO << "isInGatheringMode is init as: " << (int) m_isInGatheringMode;
}

//--------------------------------------------------------------------------
BOOL CBridgePartyVideoOut::isAllowConfToHiddenAudioIcon()
{
	CVideoBridge* pBridge = (CVideoBridge*) m_pBridgePartyCntl->GetBridge();
	PASSERT_AND_RETURN_VALUE(!pBridge, YES);
	CConf* pConf = pBridge->GetConf();
	PASSERT_AND_RETURN_VALUE(!pConf, YES);
	const CCommConf* pCommConf = pConf->GetCommConf();
	PASSERT_AND_RETURN_VALUE(!pCommConf, YES);

	if (eIconDIsplayPermanent == pCommConf->GetAudioParticipantsIconDisplayMode()) //the audio numbe change to 0
		return YES;

	BOOL willPartyControlHiddenIcon = IsValidTimer(PARTY_AUDIO_NUMBER_INDICATION_DISPLAY_TOUT);

	if (m_isAudioIconToSentAfterOpenPort || willPartyControlHiddenIcon)
		return NO;

	return YES;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyShowSlideIDLE(CSegment *pParam)
{
	OnVideoBridgePartyShowSlide(pParam);
}
//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyShowSlide(CSegment *pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", state: " << GetStateAsString(m_state);

	STATUS status = STATUS_OK;
	CVideoHardwareInterface* pHWInt = (CVideoHardwareInterface*) m_pHardwareInterface;
	DWORD fs = m_FS;
	DWORD mbps = m_MBPS;
	if (m_videoAlg == MS_SVC)
	{
		fs = pHWInt->GetFsForSvcLync(m_msftSvcParamsStruct.nWidth, m_msftSvcParamsStruct.nHeight);
		mbps = (DWORD) GetMaxMbpsAsDevision((DWORD) (fs * pHWInt->TranslateVideoFrameRateToNumeric(m_videoFrameRate)));
		TRACEINTO << "MSSVC fs:" << fs << ", mbps:" << mbps;
	}
	status = pHWInt->SendShowSlide(pParam, m_videoResolution, m_videoAlg, m_videoBitRate, fs, mbps, m_bIsTipMode);

	if (STATUS_OK == status)
	{
		m_state = SLIDE;
	}
	StartTimer(SLIDE_INTRA_TOUT, SLIDE_INTRA_TIMER);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnTimerSlideIntraSLIDE(CSegment *pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	OnVideoBridgePartyFastUpdateSLIDE(NULL);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::SetSiteNameInfo(const CSiteNameInfo* pSiteNameInfo)
{
	if (pSiteNameInfo != NULL)
		*m_pSiteNameInfo = *pSiteNameInfo;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutVSW::OnTimerSlideIntraSLIDE(CSegment *pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	OnVideoBridgePartyFastUpdateSLIDE(NULL);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyShowSlideSLIDE(CSegment *pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Event is ignored";
}
// ------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyStopShowSlideIDLE(CSegment *pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Event is ignored";
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyStopShowSlideSLIDE(CSegment *pParam)
{
	OnVideoBridgePartyStopShowSlide(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyStopShowSlide(CSegment *pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", state: " << GetStateAsString(m_state);

	((CVideoHardwareInterface*) m_pHardwareInterface)->SendStopShowSlide(pParam);

	m_state = IDLE;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyUpdateLectureModeRoleCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	OnVideoBridgePartyUpdateLectureModeRole(pParam);

	BuildLayout();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyUpdateLectureModeRoleSETUP(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	OnVideoBridgePartyUpdateLectureModeRole(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyUpdateLectureModeRoleSLIDE(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	OnVideoBridgePartyUpdateLectureModeRole(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyUpdateLectureModeRoleIDLE(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	OnVideoBridgePartyUpdateLectureModeRole(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyUpdateLectureModeRoleCHANGELAYOUT(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	OnVideoBridgePartyUpdateLectureModeRole(pParam);

	ON(m_layoutChangedWhileWaitingForAck);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyUpdateLectureModeRole(CSegment* pParam)
{
	BYTE tmp;
	*pParam >> tmp;
	ePartyLectureModeRole newPartyLectureModeRole = (ePartyLectureModeRole) tmp;

	m_partyLectureModeRole = newPartyLectureModeRole;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyDeletePartyFromConfCONNECTED(CSegment* pParam)
{
	// vngr-7017 "bombing" logger on change layout - change trace to DEBUG level
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	OnVideoBridgePartyDeletePartyFromConf(pParam);

	m_pBridgePartyCntl->HandleEvent(NULL, 0, PARTYLAYOUTCHANGED);
	m_isPrivateChanged = false;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyDeletePartyFromConfCHANGELAYOUT(CSegment* pParam)
{
	OnVideoBridgePartyDeletePartyFromConf(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgeUpdateDecoderDetectedModeSETUP(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	DWORD decoderDetectedModeWidth;
	DWORD decoderDetectedModeHeight;
	DWORD decoderDetectedSampleAspectRatioWidth;
	DWORD decoderDetectedSampleAspectRatioHeight;

	*pParam >> decoderDetectedModeWidth;
	*pParam >> decoderDetectedModeHeight;
	*pParam >> decoderDetectedSampleAspectRatioWidth;
	*pParam >> decoderDetectedSampleAspectRatioHeight;

	m_decoderDetectedModeWidth = decoderDetectedModeWidth;
	m_decoderDetectedModeHeight = decoderDetectedModeHeight;
	m_decoderDetectedSampleAspectRatioWidth = decoderDetectedSampleAspectRatioWidth;
	m_decoderDetectedSampleAspectRatioHeight = decoderDetectedSampleAspectRatioHeight;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgeUpdateDecoderDetectedModeCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	DWORD decoderDetectedModeWidth;
	DWORD decoderDetectedModeHeight;
	DWORD decoderDetectedSampleAspectRatioWidth;
	DWORD decoderDetectedSampleAspectRatioHeight;

	*pParam >> decoderDetectedModeWidth;
	*pParam >> decoderDetectedModeHeight;
	*pParam >> decoderDetectedSampleAspectRatioWidth;
	*pParam >> decoderDetectedSampleAspectRatioHeight;

	if (m_decoderDetectedModeWidth != decoderDetectedModeWidth || m_decoderDetectedModeHeight != decoderDetectedModeHeight
					|| m_decoderDetectedSampleAspectRatioWidth != decoderDetectedSampleAspectRatioWidth
					|| m_decoderDetectedSampleAspectRatioHeight != decoderDetectedSampleAspectRatioHeight)
	{
		m_decoderDetectedModeWidth = decoderDetectedModeWidth;
		m_decoderDetectedModeHeight = decoderDetectedModeHeight;
		m_decoderDetectedSampleAspectRatioWidth = decoderDetectedSampleAspectRatioWidth;
		m_decoderDetectedSampleAspectRatioHeight = decoderDetectedSampleAspectRatioHeight;

		SendUpdateEncoder();

		// VNGFE-6914
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Decoder Detected Mode resolution changed";
		m_waitForUpdateEncoderAck = true;
		StartTimer(ENCODER_UPDATE_PARAM_TOUT, 8 * SECOND);
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgeUpdateDecoderDetectedModeCHANGELAYOUT(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	DWORD decoderDetectedModeWidth;
	DWORD decoderDetectedModeHeight;
	DWORD decoderDetectedSampleAspectRatioWidth;
	DWORD decoderDetectedSampleAspectRatioHeight;

	*pParam >> decoderDetectedModeWidth;
	*pParam >> decoderDetectedModeHeight;
	*pParam >> decoderDetectedSampleAspectRatioWidth;
	*pParam >> decoderDetectedSampleAspectRatioHeight;

	if (m_decoderDetectedModeWidth != decoderDetectedModeWidth || m_decoderDetectedModeHeight != decoderDetectedModeHeight
					|| m_decoderDetectedSampleAspectRatioWidth != decoderDetectedSampleAspectRatioWidth
					|| m_decoderDetectedSampleAspectRatioHeight != decoderDetectedSampleAspectRatioHeight)
	{
		m_decoderDetectedModeWidth = decoderDetectedModeWidth;
		m_decoderDetectedModeHeight = decoderDetectedModeHeight;
		m_decoderDetectedSampleAspectRatioWidth = decoderDetectedSampleAspectRatioWidth;
		m_decoderDetectedSampleAspectRatioHeight = decoderDetectedSampleAspectRatioHeight;

		SendUpdateEncoder();

		// VNGFE-6914
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Decoder Detected Mode resolution changed";
		m_imageResolutionChangedWhileWaitingForAck = true;
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyDeletePartyFromConf(CSegment* pParam)
{
	char deletedPartyName[H243_NAME_LEN];
	*pParam >> deletedPartyName;
	deletedPartyName[H243_NAME_LEN - 1] = '\0';

	DeleteAllPartyForces(deletedPartyName);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnTimerVideoOutSetupSETUP(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Start disconnect process";
	DBGPASSERT(101);
	DisConnect();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnTimerVideoOutDisconnectionDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - End disconnect process although we did not receive ACK on Close Port";
	DBGPASSERT(101);

	OFF(m_isPortOpened);
	m_state = IDLE;
	CSegment *pMsg = new CSegment;
	*pMsg << (STATUS) STATUS_OK;
	m_pBridgePartyCntl->HandleEvent(pMsg, 0, VIDEO_OUT_DISCONNECTED);
	POBJDELETE(pMsg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnTimerChangeLayoutCHANGELAYOUT(CSegment* pParam)
{
// Change Layout Improvement - MplApi doesn't forward ACK_IND on CHANGE_LAYOUT_REQ anymore. (CL-ACK)
// Instead, we continue after 150 msec timer as if ACK_IND received.
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Continuing actions as if Acknowledge for ChangeLayoutReq received";

	OnMplChangeLayoutAck(STATUS_OK);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnSiteNameToutCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	OnSiteNameTout(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnSiteNameToutCHANGE_LAYOUT(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	OnSiteNameTout(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnSiteNameTout(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	((CVideoHardwareInterface*) m_pHardwareInterface)->SendStopSiteNameDisplay();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::GetBridgePartyVideoParams(CBridgePartyVideoParams& bridgePartyVideoParams) const
{
	bridgePartyVideoParams.SetVideoAlgorithm(m_videoAlg);
	bridgePartyVideoParams.SetFS(m_FS);
	bridgePartyVideoParams.SetMBPS(m_MBPS);
	bridgePartyVideoParams.SetVideoBitRate(m_videoBitRate);
	bridgePartyVideoParams.SetVideoFrameRate(eVideoResolutionQCIF, m_videoQcifFrameRate);
	bridgePartyVideoParams.SetVideoFrameRate(eVideoResolutionCIF, m_videoCifFrameRate);
	bridgePartyVideoParams.SetVideoFrameRate(eVideoResolution4CIF, m_video4CifFrameRate);
	bridgePartyVideoParams.SetVideoResolution(m_videoResolution);
	bridgePartyVideoParams.SetIsH263Plus(m_isH263Plus);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::GetBridgePartyVideoOutParams(CBridgePartyVideoOutParams& bridgePartyVideoOutParams) const
{
	bridgePartyVideoOutParams.SetVideoAlgorithm(m_videoAlg);
	bridgePartyVideoOutParams.SetFS(m_FS);
	bridgePartyVideoOutParams.SetMBPS(m_MBPS);
	bridgePartyVideoOutParams.SetVideoBitRate(m_videoBitRate);
	bridgePartyVideoOutParams.SetVideoFrameRate(eVideoResolutionQCIF, m_videoQcifFrameRate);
	bridgePartyVideoOutParams.SetVideoFrameRate(eVideoResolutionCIF, m_videoCifFrameRate);
	bridgePartyVideoOutParams.SetVideoFrameRate(eVideoResolution4CIF, m_video4CifFrameRate);
	bridgePartyVideoOutParams.SetVideoResolution(m_videoResolution);
	bridgePartyVideoOutParams.SetTelePresenceMode(m_eTelePresenceMode);
	bridgePartyVideoOutParams.SetIsH263Plus(m_isH263Plus);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::UpdateDecoderDetectedMode(DWORD decoderDetectedModeWidth, DWORD decoderDetectedModeHeight,
				DWORD decoderDetectedSampleAspectRatioWidth, DWORD decoderDetectedSampleAspectRatioHeight)
{
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD) decoderDetectedModeWidth;
	*pSeg << (DWORD) decoderDetectedModeHeight;
	*pSeg << (DWORD) decoderDetectedSampleAspectRatioWidth;
	*pSeg << (DWORD) decoderDetectedSampleAspectRatioHeight;

	DispatchEvent(UPDATE_DECODER_DETECTED_MODE, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
BYTE CBridgePartyVideoOut::IsInswitch()
{
	return (m_state == CHANGE_LAYOUT) ? YES : NO;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::SetVisualEffectsParams(CVisualEffectsParams* pVisualEffects)
{
	PASSERT_AND_RETURN(!pVisualEffects);
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", BkgImageId:" << pVisualEffects->GetBackgroundImageID();
	POBJDELETE(m_pPartyVisualEffects);
	ModifyVisuallEffectsForAVMCUParty(pVisualEffects);
	m_pPartyVisualEffects = new CVisualEffectsParams(pVisualEffects);
	if (NULL != m_pPartyPcmFeccVisualEffects)
	{
		POBJDELETE(m_pPartyPcmFeccVisualEffects);
		m_pPartyPcmFeccVisualEffects = new CVisualEffectsParams(pVisualEffects);
	}
	SendChangeLayoutAttributesToHardware();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::UpdatePartyOutParams(CUpdatePartyVideoInitParams* pUpdatePartyVideoInitParams)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	CBridgePartyVideoOutParams* pUpdatePartyVideoInitOutParams =
					(CBridgePartyVideoOutParams*) pUpdatePartyVideoInitParams->GetMediaOutParams();

	UpdatePartyParams(pUpdatePartyVideoInitOutParams);

	// Set to default params
	m_isPrivate = false;
	m_isPrivateChanged = false;
	m_PrivatelayoutType = CP_NO_LAYOUT;
	m_pCurrHandler = NULL;
	m_pCurrentView = NULL;

	POBJDELETE(m_pPartyVisualEffects);
	if (pUpdatePartyVideoInitOutParams->GetVisualEffects())
		m_pPartyVisualEffects = new CVisualEffectsParams(*pUpdatePartyVideoInitOutParams->GetVisualEffects());
	else
		m_pPartyVisualEffects = new CVisualEffectsParams();

	for (int i = CP_LAYOUT_1X1; i < CP_NO_LAYOUT; i++)
	{
		POBJDELETE(m_pReservation[i]);
		if (pUpdatePartyVideoInitParams->GetReservationLayout(i))
		{
			m_pReservation[i] = new CLayout(*(pUpdatePartyVideoInitParams->GetReservationLayout(i)));
		}
		else
		{
			m_pReservation[i] = new CLayout((LayoutType) i, m_pBridgePartyCntl->GetConfName());
		}
	}

	for (int i = CP_LAYOUT_1X1; i < CP_NO_LAYOUT; i++)
	{
		POBJDELETE(m_pPrivateReservation[i]);
		if (pUpdatePartyVideoInitParams->GetPrivateReservationLayout(i))
		{
			m_pPrivateReservation[i] = new CLayout(*(pUpdatePartyVideoInitParams->GetPrivateReservationLayout(i)));
		}
		else
		{
			m_pPrivateReservation[i] = new CLayout((LayoutType) i, m_pBridgePartyCntl->GetConfName());
		}
	}

	m_partyLectureModeRole = pUpdatePartyVideoInitOutParams->GetPartyLectureModeRole();
	m_pCurrentView = new CLayout(GetConfLayoutType(), m_pBridgePartyCntl->GetConfName());
	m_isForce1x1 = NO;
	m_videoQuality = pUpdatePartyVideoInitOutParams->GetVideoQualityType();
	m_isSiteNamesEnabled = pUpdatePartyVideoInitOutParams->GetIsSiteNamesEnabled();
	m_isH263Plus = pUpdatePartyVideoInitOutParams->GetIsH263Plus();

	SetLayoutHandler();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgeDisplayTextOnScreenCONNECT(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	OnVideoBridgeDisplayTextOnScreen(pParam);

}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgeDisplayTextOnScreenCHANGE_LAYOUT(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	OnVideoBridgeDisplayTextOnScreen(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgeDisplayTextOnScreen(CSegment* pParam)
{
	CTextOnScreenMngr TextMsgList;
	DWORD timeout = 0;
	TextMsgList.DeSerialize(NATIVE, *pParam);
	*pParam >> timeout;

	PASSERT_AND_RETURN(!m_pHardwareInterface);

	if (timeout)
		StartTextOnScreenTimer(timeout);

	((CVideoHardwareInterface*) m_pHardwareInterface)->SendTextToDisplay(TextMsgList);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::StartTextOnScreenTimer(DWORD timeout)
{
	StartTimer(TEXT_ON_SCREEN_DISPLAY_OFF, timeout);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnTextDisplayToutCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	OnTextDisplayTout(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnTextDisplayToutCHANGE_LAYOUT(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	OnTextDisplayTout(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnTextDisplayTout(CSegment* pParam)
{
	((CVideoHardwareInterface*) m_pHardwareInterface)->SendStopTextDisplay();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::SendStopTextDisplay(CSegment* pParam)  //VNGR-15750
{
	((CVideoHardwareInterface*) m_pHardwareInterface)->SendStopTextDisplay();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::SetPrivateLayoutForParty(LayoutType layoutType)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	CLayout* pLayout = new CLayout(layoutType, m_pBridgePartyCntl->GetConfName());
	CVideoLayout* layout = new CVideoLayout(GetOldLayoutType(layoutType), YES);

	SetIsPrivateLayout(true);

	m_pPrivateReservation[layoutType]->ClearAllForces();
	m_pPrivateReservation[layoutType]->ClearAllImageSources();
	m_pPrivateReservation[layoutType]->SetLayout(*layout, PARTY_lev, 1);
	m_pPrivateReservation[layoutType]->SetCurrActiveLayout(YES);

	m_PrivatelayoutType = layoutType;

	POBJDELETE(m_pCurrentView);
	POBJDELETE(layout);
	m_pCurrentView = pLayout;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::SetGatheringLayoutForParty(bool bNullImage /*= true*/)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	CVideoLayout* pGatheringLayout = NULL;
	if (bNullImage)
		pGatheringLayout = CVideoLayout::CreateGatheringLayout();
	else
		pGatheringLayout = CVideoLayout::CreateNonGatheringLayout();

	if (pGatheringLayout)
	{
		LayoutType layoutType = GetNewLayoutType(pGatheringLayout->m_screenLayout);
		CLayout* pLayout = new CLayout(layoutType, m_pBridgePartyCntl->GetConfName());

		SetIsPrivateLayout(true);

		m_pPrivateReservation[layoutType]->SetLayout(*pGatheringLayout, PARTY_lev, 1);
		m_pPrivateReservation[layoutType]->SetCurrActiveLayout(YES);

		m_PrivatelayoutType = layoutType;

		POBJDELETE(m_pCurrentView);
		POBJDELETE(pGatheringLayout);
		m_pCurrentView = pLayout;
	}
}

//--------------------------------------------------------------------------
CGatheringManager* CBridgePartyVideoOut::GetGatheringManager()
{
	return m_pBridgePartyCntl->GetBridge()->GetConf()->GetGatheringManager();
}

//--------------------------------------------------------------------------
bool CBridgePartyVideoOut::IsNeedPersonalLayoutForGathering()
{
	CGatheringManager* pGatheringManager = GetGatheringManager();
	if (IsValidPObjectPtr(pGatheringManager))
	{
		if (pGatheringManager->IsGatheringEnabled())
		{
			bool bNeedParticipantGathering = pGatheringManager->IsNeedParticipantGathering();
			TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", IsNeedPersonalLayoutForGathering:"
							<< (int) bNeedParticipantGathering;
			return bNeedParticipantGathering;
		}
	}

	return false;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::UpdateVideoClarity(WORD isVideoClarityEnabled)
{
	CSegment* pSeg = new CSegment;
	*pSeg << isVideoClarityEnabled;
	DispatchEvent(UPDATE_VIDEO_CLARITY, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyUpdateVideoClarityIDLE(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	OnVideoBridgePartyUpdateVideoClarity(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyUpdateVideoClaritySLIDE(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	OnVideoBridgePartyUpdateVideoClarity(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyUpdateVideoClaritySETUP(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	BYTE oldVideoClarity = m_isVideoClarityEnabled;
	OnVideoBridgePartyUpdateVideoClarity(pParam);
	if (oldVideoClarity != m_isVideoClarityEnabled)
	{
		if (m_isPortOpened) //means we got Ack on ConnectReq and already send the open request- we need to update the encoder in update request. In the other case the updated value will be send part of the open request cause it wasnt sent yet
		{
			if (CPObject::IsValidPObjectPtr(m_pWaitingForUpdateParams))
			{
				//We will save the updated video clarity as part of update parameters
				m_pWaitingForUpdateParams->SetIsVideoClarityEnabled(m_isVideoClarityEnabled);
			}
			else
			{
				//We will create update parameters with the updated video clarity value
				m_pWaitingForUpdateParams = new CBridgePartyVideoOutParams(m_videoAlg, m_videoBitRate, m_videoQcifFrameRate,
								m_videoCifFrameRate, m_video4CifFrameRate, m_videoResolution, m_MBPS, m_FS, m_videoQuality,
								m_eTelePresenceMode, m_isForce1x1);
				m_pWaitingForUpdateParams->SetIsH263Plus(m_isH263Plus);
			}
		}
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyUpdateVideoClarityCHANGE_LAYOUT(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	SaveAndSendUpdateVideoClarity(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyUpdateVideoClarityCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	SaveAndSendUpdateVideoClarity(pParam);

}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyUpdateVideoClarityDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Event is ignored";
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgeChangeSpeakerNotationForPcmFeccIDLE(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Event is ignored";
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgeChangeSpeakerNotationForPcmFeccSETUP(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Event is ignored";
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgeChangeSpeakerNotationForPcmFeccCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Event is ignored";

	DWORD imageId = 0xFFFFFFFF;
	*pParam >> imageId;
	if (imageId > 15)
	{
		SendChangeLayoutAttributesToHardware();
		CSegment* pSeg = new CSegment;
		*pSeg << (BYTE) true;
		m_pBridgePartyCntl->HandleEvent(pSeg, 0, PARTYLAYOUTCHANGED);
		POBJDELETE(pSeg);
		m_isPrivateChanged = false;
	}
	else
	{
		PASSERT_AND_RETURN(!IsValidPObjectPtr(m_pPartyPcmFeccVisualEffects));
		((CVideoHardwareInterface*) m_pHardwareInterface)->SendChangeLayoutAttributes(m_pCurrentView,
						m_pPartyPcmFeccVisualEffects, imageId, m_pSiteNameInfo, m_videoConfType);
	}

}
//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgeChangeSpeakerNotationForPcmFeccCHANGE_LAYOUT(CSegment* pParam)
{
	DWORD imageId = INVALID;
	*pParam >> imageId;

	m_tmpSpeakerNotationForPcmFeccImageId = imageId;
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", NewSpeakerId:" << m_tmpSpeakerNotationForPcmFeccImageId;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgeChangeSpeakerNotationForPcmFeccSLIDE(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Event is ignored";
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgeChangeSpeakerNotationForPcmFeccDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Event is ignored";
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnTimerEncoderUpdateCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	DBGPASSERT(m_waitForUpdateEncoderAck);

	m_waitForUpdateEncoderAck = false;
	// Because of AT&T bug of double change layout messages, we send change layout after update is done or timer pops up
	SendChangeLayoutToHardware();

}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnTimerEncoderUpdateCHANGE_LAYOUT(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	DBGPASSERT(m_waitForUpdateEncoderAck);

	m_waitForUpdateEncoderAck = false;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnTimerRecurrentIntraRequest(CSegment* pParam)
{
	PASSERT_AND_RETURN(!m_pHardwareInterface);
	m_nTimerFastUpdateReq++;

	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", PartyId:" << m_pHardwareInterface->GetPartyRsrcId()
					<< ", TimerFastUpdateReq:" << m_nTimerFastUpdateReq;

	// Sending intra request to encoder
	((CVideoHardwareInterface*) m_pHardwareInterface)->SendFastUpdate();

	DWORD encoderRequestIntraToutValue = 0;
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();

	if (m_bIsTipMode)
		sysConfig->GetDWORDDataByKey("ENCODER_TIP_RECURRENT_INTRA_REQ_MINUTES", encoderRequestIntraToutValue);
	else
		sysConfig->GetDWORDDataByKey("ENCODER_RECURRENT_INTRA_REQ_MINUTES", encoderRequestIntraToutValue);

	if (encoderRequestIntraToutValue)
		StartTimer(ENCODER_RECURRENT_INTRA_REQ_TIMEOUT, encoderRequestIntraToutValue * 60 * SECOND);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::ChangeSpeakerNotationForPcmFecc(DWORD imageId)
{
	CSegment* pSeg = new CSegment;
	*pSeg << imageId;
	DispatchEvent(CHANGE_SPEAKER_NOTATION_PCM_FECC, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::SaveAndSendUpdateVideoClarity(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	const BYTE oldVideoClarity = m_isVideoClarityEnabled;
	OnVideoBridgePartyUpdateVideoClarity(pParam);

	if (oldVideoClarity != m_isVideoClarityEnabled)
		SendUpdateEncoder();
}

//--------------------------------------------------------------------------

bool CBridgePartyVideoOut::InitH264SvcVideoParams(H264_SVC_VIDEO_PARAM_S& h264, const MsSvcParamsStruct& svc) const
{
	h264.unPrID = svc.pr_id;

	if (h264.unPrID == INVALID)
		return false;

	h264.unSsrcID = svc.ssrc;
	h264.nResolutionHeight = svc.nHeight;
	h264.nResolutionWidth = svc.nWidth;

	h264.nProfile = CVideoHardwareInterface::TranslateVideoProfileToApi(m_profile);
	h264.unPacketPayloadFormat = CVideoHardwareInterface::TranslatePacketPayloadFormatToApi(m_packetPayloadFormat);

	// *** setting temporal layer frame rate and bitrate ***

	const DWORD layersNumber = GetSystemCfgFlag<DWORD>("LYNC2013_PATCH_NUM_OF_LAYERS");
	PASSERTSTREAM_AND_RETURN_VALUE(layersNumber < 2 || layersNumber > 3, "LYNC2013_PATCH_NUM_OF_LAYERS should be either 2 or 3",
					false);

	static const eVideoFrameRate frameRates[3] =
	{ eVideoFrameRate7_5FPS, eVideoFrameRate15FPS, eVideoFrameRate30FPS };

	static const int bitRatePercent[3][3] =
	{
	{ 100, 0, 0 }, // 1 layer
					{ 55, 100, 0 }, // 2 layers
					{ 50, 75, 100 }, // 3 layers
					};

	typedef std::pair<DWORD, eVideoFrameRate> Key; // { Maximum Layers #, Video Frame Rate }
	typedef std::pair<size_t, size_t> Value; // { Actual Layers #, offset into frame rates array }
	typedef std::map<Key, Value> ParamsMap;

	static ParamsMap m;

	if (m.empty())
	{
		m.insert(std::make_pair(std::make_pair(3, eVideoFrameRate30FPS), std::make_pair(3, 0)));
		m.insert(std::make_pair(std::make_pair(3, eVideoFrameRate15FPS), std::make_pair(2, 0)));
		m.insert(std::make_pair(std::make_pair(3, eVideoFrameRate7_5FPS), std::make_pair(1, 0)));

		m.insert(std::make_pair(std::make_pair(2, eVideoFrameRate30FPS), std::make_pair(2, 1)));
		m.insert(std::make_pair(std::make_pair(2, eVideoFrameRate15FPS), std::make_pair(1, 0)));
	}

	const eVideoFrameRate videoFrameRate =
					svc.maxFrameRate ?
									CPartyCntl::TranslateIntegerFrameRateToVideoBridgeFrameRate(svc.maxFrameRate) :
									m_videoFrameRate;
	TRACEINTO << svc << "\n videoFrameRate:" << EVideoFrameRateNames[videoFrameRate];

	ParamsMap::const_iterator it = m.find(std::make_pair(layersNumber, videoFrameRate));
	PASSERTSTREAM_AND_RETURN_VALUE(it == m.end(), "Layers:" << layersNumber << ", VideoFrameRate:" << (int)videoFrameRate, false);

	DWORD& layers = h264.unNumberOfTemporalLayers;
	layers = it->second.first;

	const size_t& offset = it->second.second;

	for (size_t i = 0; i < layers; ++i)
	{
		h264.atSvcTemporalLayer[i].nResolutionFrameRate = CVideoHardwareInterface::TranslateVideoFrameRateToApi(
						frameRates[i + offset]);
		h264.atSvcTemporalLayer[i].nBitRate = bitRatePercent[layers - 1][i]
						* (svc.maxBitRate ? svc.maxBitRate : m_videoBitRate / 100);
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoOut::FillEncoderParamsForMsSvc(ENCODER_PARAM_S& encoder) const
{
	const DWORD svcFs = CVideoHardwareInterface::GetFsForSvcLync(m_msftSvcParamsStruct.nWidth, m_msftSvcParamsStruct.nHeight,
					true);
	const DWORD svcMbps = GetMaxMbpsAsDevision(
					(DWORD) (svcFs * CVideoHardwareInterface::TranslateVideoFrameRateToNumeric(m_videoFrameRate)));
	const DWORD svcMaxDpb = CH264Details::GetMaxDpbFromMaxFs(svcFs);

	encoder.nEncoderResolutionRatio = CVideoHardwareInterface::TranslateToVideoResolutionRatio(m_videoAlg, m_videoResolution,
					encoder.tH264VideoParams.nFS, svcMbps, m_videoConfType, false);

	encoder.tH264VideoParams.unMaxDPB = (svcMaxDpb > m_maxDPB || m_maxDPB == INVALID) ? svcMaxDpb : m_maxDPB;
	encoder.tH264VideoParams.nFS = GetMaxFsAsDevision(svcFs);
	encoder.tH264VideoParams.nMBPS = svcMbps;

	// taken fom the svctoavctranslator hardcoded value
	encoder.tCroppingParams.nHorizontalCroppingPercentage = 50;
	encoder.tCroppingParams.nVerticalCroppingPercentage = 50;

	InitH264SvcVideoParams(encoder.tH264SvcVideoParams, m_msftSvcParamsStruct);
	bool anyFilled = false;

	for (size_t i = 0; i < ARRAYSIZE(encoder.tMsSvcPacsiParams); ++i)
		anyFilled |= InitH264SvcVideoParams(encoder.tMsSvcPacsiParams[i], m_PACSI.pacsiInfo[i]);

	if (!anyFilled)
		encoder.tMsSvcPacsiParams[0] = encoder.tH264SvcVideoParams;

	reinterpret_cast<CVideoHardwareInterface*>(m_pHardwareInterface)->fillEncoderMsftSVCTemporaryLayersParams(encoder,
					m_videoFrameRate, m_videoBitRate);
}

///////////////////////////////////////////////////////////////////////////
DWORD CBridgePartyVideoOut::SendUpdateEncoder(bool open/* = false*/)/* const */
{
	PASSERTMSG_AND_RETURN_VALUE(!m_pHardwareInterface, "CBridgePartyVideoOut::SendUpdateEncoder - m_pHardwareInterface is null!",
					0);
	ENCODER_PARAM_S params;
	memset(&params, 0, sizeof(params));

	params.nVideoConfType = CVideoHardwareInterface::TranslateVideoConfTypeToApi(m_videoConfType);
	params.nVideoEncoderType = E_VIDEO_ENCODER_DUMMY;
	params.nBitRate = m_videoBitRate;
	params.nProtocol = CVideoHardwareInterface::TranslateVideoProtocolToApi(m_videoAlg);
	params.tH264VideoParams.nMBPS = m_MBPS;
	params.tH264VideoParams.nFS = m_FS;
	params.tH264VideoParams.nStaticMB = m_staticMB;
	params.tH264VideoParams.nProfile = CVideoHardwareInterface::TranslateVideoProfileToApi(m_profile);
	params.tH264VideoParams.unPacketPayloadFormat = CVideoHardwareInterface::TranslatePacketPayloadFormatToApi(
					m_packetPayloadFormat);
	params.bIsLinkEncoder = (m_pBridgePartyCntl->GetCascadeLinkMode() != NONE);

	// COP
	params.tH264VideoParams.nResolutionWidth = CVideoHardwareInterface::TranslateResolutionToResWidth(m_videoResolution); // For COP feature
	params.tH264VideoParams.nResolutionHeight = CVideoHardwareInterface::TranslateResolutionToResHeight(m_videoResolution); // For COP feature
	params.tH264VideoParams.nResolutionFrameRate = CVideoHardwareInterface::TranslateVideoFrameRateToApi(
					eVideoConfTypeVSW != m_videoConfType ? m_videoFrameRate : eVideoFrameRateDUMMY); // For COP feature

	params.tH263_H261VideoParams.nQcifFrameRate = CVideoHardwareInterface::TranslateVideoFrameRateToApi(m_videoQcifFrameRate);
	params.tH263_H261VideoParams.nCifFrameRate = CVideoHardwareInterface::TranslateVideoFrameRateToApi(m_videoCifFrameRate);
	params.tH263_H261VideoParams.n4CifFrameRate = CVideoHardwareInterface::TranslateVideoFrameRateToApi(m_video4CifFrameRate);

	// Default should be NO !! H263_HIGH_BIT_BUDGET_INTRA when set to YES (1) the H263 intra that sent will be bigger.(the intra will be SOFTER) China request for soft intra
	params.tH263_H261VideoParams.b263HighBbIntra = CVideoHardwareInterface::IsH263HighBbIntra();
	params.tH263_H261VideoParams.bIs263Plus = m_isH263Plus; // Default should be NO !!

	DECODER_DETECTED_MODE_PARAM_S& d = params.tDecoderDetectedMode;

	d.nDecoderDetectedModeWidth = m_decoderDetectedModeWidth;
	d.nDecoderDetectedModeHeight = m_decoderDetectedModeHeight;
	d.nDecoderDetectedSampleAspectRatioWidth = m_decoderDetectedSampleAspectRatioWidth;
	d.nDecoderDetectedSampleAspectRatioHeight = m_decoderDetectedSampleAspectRatioHeight;

	params.nResolutionTableType = m_eResolutionTableType;
	params.nSampleAspectRatio = (eVideoConfTypeVSW != m_videoConfType) ? m_sampleAspectRatio : DEFAULT_SAMPLE_ASPECT_RATIO;
	params.nParsingMode = (eVideoConfTypeVSW != m_videoConfType) ? E_PARSING_MODE_CP : E_PARSING_MODE_PSEUDO_VSW;
	params.nTelePresenceMode = CVideoHardwareInterface::TranslateTelePresenceModeToApi(m_eTelePresenceMode);
	params.nMTUSize = CVideoHardwareInterface::GetMTUSize(eVideoConfTypeVSW != m_videoConfType ? m_isLprActive : false,
					m_bIsTipMode);
	params.nVideoQualityType = CVideoHardwareInterface::TranslateVideoQualityToApi(m_videoQuality, m_videoConfType);
	params.bIsTipMode = m_bIsTipMode;
	params.bRtvEnableBFrames = m_bEncodeBFramesInRTV;
	params.nFontType = IsFeatureSupportedBySystem(eFeatureFontTypes) ? GetFontType() : ftDefault;
	params.nFrThreshold = m_dwFrThreshold;
	params.bUseIntermediateSDResolution = CVideoHardwareInterface::IsIntermediateSDResolution(m_bUseIntermediateSDResolution);
	params.bIsVideoClarityEnabled = m_isVideoClarityEnabled;
	params.tCroppingParams.nHorizontalCroppingPercentage = m_croppingHor;
	params.tCroppingParams.nVerticalCroppingPercentage = m_croppingVer;

	const std::string modeFPS(GetSystemCfgFlag<std::string>("PAL_NTSC_VIDEO_OUTPUT"));
	params.nFpsMode = TranslateSysConfigStringToFpsMode(modeFPS);

	if (m_videoAlg != MS_SVC)
	{
		CVideoHardwareInterface::UpdateMaxDpbFromMaxFs(m_maxDPB, m_FS);
		params.tH264VideoParams.unMaxDPB = m_maxDPB;
		params.nEncoderResolutionRatio = CVideoHardwareInterface::TranslateToVideoResolutionRatio(m_videoAlg, m_videoResolution,
						m_FS, m_MBPS, m_videoConfType, false);
	}

	params.bEnableMbRefresh = GetSystemCfgFlag<bool>(H264_MB_INTRA_REFRESH);
	params.nMaxSingleTransferFrameSize = GetSystemCfgFlag<DWORD>(CFG_KEY_MAX_SINGLE_TRANSFER_FRAME_SIZE_BITS);
	params.tCallGeneratorParams.bIsCallGenerator = m_pBridgePartyCntl->GetIsCallGeneratorConference();
	params.bFollowSpeaker = m_bIsFollowSpeakerOn1X1;

	if (m_videoAlg == MS_SVC)
		FillEncoderParamsForMsSvc(params);

	//====================================================================
	// Traffic shaping - only bitrate reduction is handled in the bridge
	//====================================================================
	CVideoHardwareInterface& hwInt = *reinterpret_cast<CVideoHardwareInterface*>(m_pHardwareInterface); // Ptr checked at function entry
	if (!m_bIsTipMode)
	{
		hwInt.TrafficShapingEncoderParamsUpdate(params);
	}

	return hwInt.SendUpdateEncoder(params, open);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgeUpdatePartyTelePresenceModeIDLE(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	OnVideoBridgeUpdatePartyTelePresenceMode(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgeUpdatePartyTelePresenceModeSLIDE(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	OnVideoBridgeUpdatePartyTelePresenceMode(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgeUpdatePartyTelePresenceModeSETUP(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	BYTE oldTelePresenceMode = m_eTelePresenceMode;
	OnVideoBridgeUpdatePartyTelePresenceMode(pParam);
	if (oldTelePresenceMode != m_eTelePresenceMode)
	{
		if (m_isPortOpened) // means we got Ack on ConnectReq and already send the open request- we need to update the encoder in update request. In the other case the updated value will be send part of the open request cause it wasnt sent yet
		{
			if (CPObject::IsValidPObjectPtr(m_pWaitingForUpdateParams))
			{
				// We will save the updated TelePresence mode as part of update parameters
				((CBridgePartyVideoOutParams*) m_pWaitingForUpdateParams)->SetTelePresenceMode(m_eTelePresenceMode);
			}
			else
			{
				// We will create update parameters with the updated TelePresenceMode value
				m_pWaitingForUpdateParams = new CBridgePartyVideoOutParams(m_videoAlg, m_videoBitRate, m_videoQcifFrameRate,
								m_videoCifFrameRate, m_video4CifFrameRate, m_videoResolution, m_MBPS, m_FS, m_videoQuality,
								m_eTelePresenceMode, m_isForce1x1, eVideoFrameRateDUMMY, eLogical_res_none, m_profile);

				m_pWaitingForUpdateParams->SetIsH263Plus(m_isH263Plus);
				m_pWaitingForUpdateParams->SetIsTipMode(m_bIsTipMode);
			}
		}
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgeUpdatePartyTelePresenceModeCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	SaveAndSendUpdatePartyTelePresenceMode(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgeUpdatePartyTelePresenceModeCHANGE_LAYOUT(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	SaveAndSendUpdatePartyTelePresenceMode(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgeUpdatePartyTelePresenceModeDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Event is ignored";
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgeUpdatePartyTelePresenceMode(CSegment* pParam)
{
	WORD tmpITelePresenceModeType = eTelePresencePartyNone;
	*pParam >> tmpITelePresenceModeType;
	if ((eTelePresencePartyType) tmpITelePresenceModeType != m_eTelePresenceMode)
		m_eTelePresenceMode = (eTelePresencePartyType) tmpITelePresenceModeType;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::SaveAndSendUpdatePartyTelePresenceMode(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	BYTE oldTelePresenceMode = m_eTelePresenceMode;
	OnVideoBridgeUpdatePartyTelePresenceMode(pParam);

	CVideoBridge* pBridge = (CVideoBridge*) (m_pBridgePartyCntl->GetBridge());
	DWORD croppingHor, croppingVer;
	pBridge->GetCroppingValues(croppingHor, croppingVer);
	if (oldTelePresenceMode != m_eTelePresenceMode || croppingHor != m_croppingHor || croppingVer != m_croppingVer)
	{
		m_croppingHor = croppingHor;
		m_croppingVer = croppingVer;

		if (m_isPortOpened)
			SendUpdateEncoder();
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::UpdateMessageOverlay(CMessageOverlayInfo* pMessageOverlayInfo)
{
	CSegment *pSeg = new CSegment;
	pMessageOverlayInfo->Serialize(NATIVE, *pSeg);
	DispatchEvent(SET_MESSAGE_OVERLAY, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::UpdateMessageOverlayStop()  //VNGR-15750
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	DispatchEvent(SET_HIDE_TEXT_BOX, NULL);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgeUpdateMessageOverlayCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	PASSERT_AND_RETURN(!m_pHardwareInterface);
	((CVideoHardwareInterface*) m_pHardwareInterface)->SendMessageOverlayToDisplay(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgeUpdateMessageOverlayCHANGE_LAYOUT(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	PASSERT_AND_RETURN(!m_pHardwareInterface);
	((CVideoHardwareInterface*) m_pHardwareInterface)->SendMessageOverlayToDisplay(pParam);
}

//--------------------------------------------------------------------------
CGathering* CBridgePartyVideoOut::GetGathering()
{
	const CConf* pConf = m_pBridgePartyCntl->GetConf();
	PASSERT_AND_RETURN_VALUE(!pConf, NULL);

	return pConf->GetGathering(m_pBridgePartyCntl->GetName());
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::DisplayGathering(CGathering* pGathering)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", ConnectionState:" << m_state;

	const char* pszPartyName = m_pBridgePartyCntl->GetName();
	if (pGathering->IsForceDisplay(pszPartyName) || pGathering->CheckTimeToShow(pszPartyName))
	{
		if (m_state == CONNECTED)
		{
			((CVideoHardwareInterface*) m_pHardwareInterface)->SendGatheringToDisplay(pGathering, pszPartyName);
			pGathering->SetDisplayed(true, pszPartyName);

			if (pGathering->IsEndGathering())
				m_isInGatheringMode = NO;
		}
		else
		{
			pGathering->SetDisplayed(false, pszPartyName);
		}
	}
	else
	{
		if (pGathering->IsEndGathering())
		{
			pGathering->SetDisplayed(false, pszPartyName);
			TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - It is not time to STOP Gathering";
		}
		else
		{
			pGathering->SetDisplayed(true, pszPartyName);
			TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - It is not time to SHOW Gathering";
		}
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::CreatePartyPcmFeccVisualEffects()
{
	m_pPartyPcmFeccVisualEffects = new CVisualEffectsParams(*m_pPartyVisualEffects);
	m_pPartyPcmFeccVisualEffects->SetUseYUVcolor(true);
	m_pPartyPcmFeccVisualEffects->SetSpeakerNotationEnable(true);
	m_pPartyPcmFeccVisualEffects->SetSpeakerNotationColorYUV(COLOR_YUV_LIGHT_GREEN);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::FillAllocStatus(ALLOC_STATUS_PER_PORT_S& allocStatusPerPort)
{
	allocStatusPerPort.tPortPhysicalId.physical_id.resource_type = ePhysical_video_encoder;
	allocStatusPerPort.tPortPhysicalId.connection_id = m_pHardwareInterface->GetConnectionId();
	allocStatusPerPort.tPortPhysicalId.party_id = m_pHardwareInterface->GetPartyRsrcId();

	DWORD parsingMode = (IsVsw() != true) ? E_PARSING_MODE_CP : E_PARSING_MODE_PSEUDO_VSW;

	((CVideoHardwareInterface*) m_pHardwareInterface)->FillEncoderParams(allocStatusPerPort.tEncoderParam, m_isLprActive,
					m_videoAlg, m_videoBitRate, m_videoQcifFrameRate, m_videoCifFrameRate, m_video4CifFrameRate,
					m_decoderDetectedModeWidth, m_decoderDetectedModeHeight, m_decoderDetectedSampleAspectRatioWidth,
					m_decoderDetectedSampleAspectRatioHeight, m_videoResolution, m_MBPS, m_FS, m_sampleAspectRatio, m_staticMB,
					m_eTelePresenceMode, m_videoQuality, m_isVideoClarityEnabled, m_videoConfType, parsingMode, m_maxDPB,
					(m_pBridgePartyCntl->GetCascadeLinkMode() != NONE), eVideoFrameRateDUMMY, m_eResolutionTableType,
					m_croppingHor, m_croppingVer, m_profile, m_packetPayloadFormat);
	portStatusEnum portStatus = IsConnected() ? ePortOpen : ePortClose;

	allocStatusPerPort.nPortStatus = portStatus;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::UpdateSiteNameInfo(CSiteNameInfo* pSiteNameInfo)
{
	CSegment* pSeg = new CSegment;
	pSiteNameInfo->Serialize(NATIVE, *pSeg);
	DispatchEvent(SET_SITE_NAME, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::RefreshLayout()
{
	DispatchEvent(REFRESHLAYOUT, NULL);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::UpdateOnImageSvcToAvcTranslate()
{
	DispatchEvent(UPDATEONIMAGESVCTOAVC, NULL);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyUpdateOnImageSvcToAvcCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetName();
	BuildLayout();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyUpdateOnImageSvcToAvcCHANGELAYOUT(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetName();
	ON(m_imageResolutionChangedWhileWaitingForAck);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::GetPortsOpened(std::map<eLogicalResourceTypes, bool>& isOpenedRsrcMap)
{
	if (IsConnected())
		isOpenedRsrcMap[eLogical_video_encoder] = true;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::UpdateLayoutHandlerType()
{
	DispatchEvent(UPDATE_LAYOUT_HANDLER_TYPE, NULL);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyUpdateLayoutHandlerTypeSETUP(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	OnVideoBridgePartyUpdateLayoutHandlerType(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyUpdateLayoutHandlerTypeIDLE(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	OnVideoBridgePartyUpdateLayoutHandlerType(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyUpdateLayoutHandlerTypeSLIDE(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	OnVideoBridgePartyUpdateLayoutHandlerType(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyUpdateLayoutHandlerTypeCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	OnVideoBridgePartyUpdateLayoutHandlerType(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyUpdateLayoutHandlerTypeCHANGELAYOUT(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	OnVideoBridgePartyUpdateLayoutHandlerType(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOut::OnVideoBridgePartyUpdateLayoutHandlerType(CSegment* pParam)
{
	SetLayoutHandler();
}
PBEGIN_MESSAGE_MAP(CBridgePartyVideoOutVSW) ONEVENT(CHANGECONFLAYOUT, ANYCASE , CBridgePartyVideoOutVSW::NullActionFunction)
ONEVENT(CHANGECONFLAYOUT, CONNECTED , CBridgePartyVideoOutVSW::OnVideoBridgePartyChangeConfLayoutCONNECTED)
ONEVENT(CHANGECONFLAYOUT, CHANGE_LAYOUT , CBridgePartyVideoOutVSW::OnVideoBridgePartyChangeConfLayoutCHANGELAYOUT)

ONEVENT(DISCONNECT_VIDEO_OUT, CHANGE_LAYOUT , CBridgePartyVideoOutVSW::OnVideoBridgePartyDisConnectCHANGELAYOUT)

ONEVENT(VIDEO_ENCODER_SYNC_IND, ANYCASE , CBridgePartyVideoOutVSW::NullActionFunction)
ONEVENT(VIDEO_ENCODER_SYNC_IND, CONNECTED , CBridgePartyVideoOutVSW::OnMplEncoderSyncIndCONNECTED)
ONEVENT(VIDEO_ENCODER_SYNC_IND, CHANGE_LAYOUT, CBridgePartyVideoOutVSW::OnMplEncoderSyncIndCHANGELAYOUT)

ONEVENT(SLIDE_INTRA_TOUT, SLIDE, CBridgePartyVideoOutVSW::OnTimerSlideIntraSLIDE)
PEND_MESSAGE_MAP(CBridgePartyVideoOutVSW, CBridgePartyVideoOut)
;

////////////////////////////////////////////////////////////////////////////
//                        CBridgePartyVideoOutVSW
////////////////////////////////////////////////////////////////////////////
CBridgePartyVideoOutVSW::CBridgePartyVideoOutVSW() :
				CBridgePartyVideoOut()
{
	m_videoConfType = eVideoConfTypeVSW;
}

//--------------------------------------------------------------------------
CBridgePartyVideoOutVSW::~CBridgePartyVideoOutVSW()
{
}

//--------------------------------------------------------------------------
// in vsw conf the layout is build in video bridge level and not in party out
// BuildLayout will not do anything
BYTE CBridgePartyVideoOutVSW::BuildLayout(BYTE bSendChangeLayoutAlways)
{
	// to check - remove from connection flow (this function should not be called)
	// currently added to with VSW layout handler to avoid empty layout with first party
	bool isLayoutChanged = m_pCurrHandler->BuildLayout();
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", isLayoutChanged:" << (int) isLayoutChanged;
	return isLayoutChanged;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutVSW::SendOpenEncoder()
{
	PASSERT_AND_RETURN(!m_pHardwareInterface);

	ON(m_isPortOpened);
	BYTE isVideoClarityEnabled = NO; //the video clarity feature is relevant only in CP

	// Tsahi - Call Generator SoftMCU
	BOOL isCallGeneratorConf = m_pBridgePartyCntl->GetIsCallGeneratorConference();

	m_lastReqId = ((CVideoHardwareInterface*) m_pHardwareInterface)->SendOpenEncoder(m_videoAlg, m_videoBitRate,
					m_videoQcifFrameRate, m_videoCifFrameRate, m_video4CifFrameRate, m_decoderDetectedModeWidth,
					m_decoderDetectedModeHeight, m_decoderDetectedSampleAspectRatioWidth,
					m_decoderDetectedSampleAspectRatioHeight, m_videoResolution, m_MBPS, m_FS, DEFAULT_SAMPLE_ASPECT_RATIO,
					m_staticMB, m_videoQuality, m_isVideoClarityEnabled, m_videoConfType, m_maxDPB,
					(m_pBridgePartyCntl->GetCascadeLinkMode() != NONE), E_PARSING_MODE_PSEUDO_VSW, m_eTelePresenceMode,
					eVideoFrameRateDUMMY, m_eResolutionTableType, INVALID, INVALID, eVideoProfileBaseline,
					eVideoPacketPayloadFormatSingleUnit, false, false, false, 0, // the method's *default* values
					GetFontType(), m_isH263Plus, isCallGeneratorConf, m_bIsFollowSpeakerOn1X1);

	m_lastReq = TB_MSG_OPEN_PORT_REQ;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutVSW::OnMplOpenPortAck(STATUS status)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", Status:" << status;

	CSegment* pSeg = new CSegment;

	if (status != STATUS_OK)
	{
		//Add assert to EMA in case of NACK
		//AddFaultAlarm("NACK on open video encoder",m_pHardwareInterface->GetPartyRsrcId(), status);
		// Inform BridgePartyCntl of statVideoInOutResourceProblem -> Inorder to initiate Kill port on VideoEncoder VideoDecoder
		*pSeg << (BYTE) statVideoInOutResourceProblem << (BYTE) eMipOut << (BYTE) eMipStatusFail << (BYTE) eMipOpen;

	}
	else
	{
		m_state = CONNECTED;
		*pSeg << (BYTE) statOK;
	}

	m_pBridgePartyCntl->HandleEvent(pSeg, 0, VIDEO_OUT_CONNECTED);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutVSW::OnMplChangeLayoutAck(STATUS status)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	DeleteTimer(VIDEO_OUT_CHANGELAYOUT_TIMEOUT);

	if (status != STATUS_OK)
	{
		DBGPASSERT(status);
	}

	m_state = CONNECTED;

	// inform bridge of change layout completed
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD) status;
	// Inform BridgePartyCntl
	m_pBridgePartyCntl->HandleEvent(pSeg, 0, END_CHANGE_LAYOUT);
	POBJDELETE(pSeg);

	if (m_layoutChangedWhileWaitingForAck)
	{
		BYTE isVsw = YES;
		BOOL bUseSharedMemForChangeLayoutReq = false;
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Layout changed while waiting for ACK, send new layout";
		SendChangeLayoutToHardware(bUseSharedMemForChangeLayoutReq, isVsw);
	}

	OFF(m_resolutionChangedWhileWaitingForAck);
	OFF(m_layoutChangedWhileWaitingForAck);
	OFF(m_visualEffectsChangedWhileWaitingForAck);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutVSW::StartSiteNamesOffTimer()
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Site names not active in VSW";
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutVSW::OnSiteNameTout(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Site names not active in VSW";
}

//--------------------------------------------------------------------------
BOOL CBridgePartyVideoOutVSW::RejectPLCRequestBecauseOfApplications(OPCODE requestOpcode)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Always reject PLC in VSW";

	CSegment* pSeg = new CSegment;
	*pSeg << requestOpcode;

	((CVideoBridgePartyCntl*) m_pBridgePartyCntl)->IvrNotification(REJECT_PLC, pSeg);

	POBJDELETE(pSeg);

	return true;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutVSW::OnVideoBridgePartyChangeConfLayoutCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	CLayout* pConfLayout = NULL;

	*pParam >> (DWORD&) pConfLayout;

	if (CPObject::IsValidPObjectPtr(pConfLayout))
	{
		CLayout* pNewView = new CLayout(*pConfLayout);
		if (*pNewView == *m_pCurrentView)
		{
			TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Layout did not changed";
			POBJDELETE(m_pCurrentView);
			m_pCurrentView = pNewView;
			// don't send double change layout to video card
			// inform bridge party control for end change layout
			OnMplChangeLayoutAck(STATUS_OK);
		}
		else
		{
			// layout changed
			POBJDELETE(m_pCurrentView);
			m_pCurrentView = pNewView;
			BYTE isVsw = YES;
			BOOL bUseSharedMemForChangeLayoutReq = false;
			SendChangeLayoutToHardware(bUseSharedMemForChangeLayoutReq, isVsw);
		}
	}
	else
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Failed, conf layout is not valid";
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutVSW::OnVideoBridgePartyChangeConfLayoutCHANGELAYOUT(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	CLayout* pConfLayout = NULL;

	*pParam >> (DWORD&) pConfLayout;

	if (CPObject::IsValidPObjectPtr(pConfLayout))
	{
		CLayout* pNewView = new CLayout(*pConfLayout);
		POBJDELETE(m_pCurrentView);
		m_pCurrentView = pNewView;
		ON(m_layoutChangedWhileWaitingForAck);
	}
	else
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Failed, conf layout is not valid";
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutVSW::OnVideoBridgePartyDisConnectCHANGELAYOUT(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	DeleteTimer(VIDEO_OUT_CHANGELAYOUT_TIMEOUT);

	OnVideoBridgePartyDisConnect(pParam);

	// inform bridge of change layout completed - to end change layout
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD) statOK;
	// Inform BridgePartyCntl
	m_pBridgePartyCntl->HandleEvent(pSeg, 0, END_CHANGE_LAYOUT);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutVSW::OnMplEncoderSyncIndCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	m_pBridgePartyCntl->HandleEvent(NULL, 0, VIDEO_ENCODER_SYNC_IND);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutVSW::OnMplEncoderSyncIndCHANGELAYOUT(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Event is ignored";
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutVSW::UpdateNewConfParams(ConfRsrcID confRsrcId,
				const CBridgePartyMediaParams* pBridgePartyVideoOutParams)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	CBridgePartyVideoUniDirection::UpdateNewConfParams(confRsrcId, pBridgePartyVideoOutParams);

	m_videoResolution = ((CBridgePartyVideoOutParams*) pBridgePartyVideoOutParams)->GetVideoResolution();

	POBJDELETE(m_pPartyVisualEffects);
	CBridgePartyVideoOutParams* pVideoOutParams = (CBridgePartyVideoOutParams*) pBridgePartyVideoOutParams;
	CVisualEffectsParams* pVisualEffectsParams = pVideoOutParams->GetVisualEffects();
	if (pVisualEffectsParams != NULL)
		m_pPartyVisualEffects = new CVisualEffectsParams(*pVisualEffectsParams);

	for (int i = CP_LAYOUT_1X1; i < CP_NO_LAYOUT; i++)
	{
		POBJDELETE(m_pReservation[i]);
		m_pReservation[i] = new CLayout((LayoutType) i, m_pBridgePartyCntl->GetConfName());
	}

	for (int i = CP_LAYOUT_1X1; i < CP_NO_LAYOUT; i++)
	{
		POBJDELETE(m_pPrivateReservation[i]);
		m_pPrivateReservation[i] = new CLayout((LayoutType) i, m_pBridgePartyCntl->GetConfName());
	}

	POBJDELETE(m_pCurrentView);
	m_pCurrentView = new CLayout(GetConfLayoutType(), m_pBridgePartyCntl->GetConfName());

	SetLayoutHandler();

	m_partyLectureModeRole = pVideoOutParams->GetPartyLectureModeRole();
	m_videoQuality = pVideoOutParams->GetVideoQualityType();
	m_isH263Plus = pVideoOutParams->GetIsH263Plus();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutVSW::SetLayoutHandler()
{
	POBJDELETE(m_pCurrHandler);
	m_pCurrHandler = new CLayoutHandlerVSW((CVideoBridge*) (m_pBridgePartyCntl->GetBridge()),
					(CVideoBridgePartyCntl*) m_pBridgePartyCntl);
}

//--------------------------------------------------------------------------
CTaskApp* CBridgePartyVideoOut::GetLastActiveAudioSpeakerRequestFromBridge()
{
	CLayoutHandler* pCurrentHandler = m_pCurrHandler;
	if (pCurrentHandler)
	{
		if (pCurrentHandler->GetpVideoBridge())
			return pCurrentHandler->GetpVideoBridge()->GetLastActiveAudioSpeakerRequest();
	}
	return NULL; //DBGASSERT
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutVSW::ResetImage0(DWORD partyRscId)
{
	CVidSubImage* SubImage0 = m_pCurrentView->GetSubImageNum(0);
	if (CPObject::IsValidPObjectPtr(SubImage0))
	{
		if (partyRscId == SubImage0->GetImageId())
		{
			SubImage0->SetImageId(0);
			SubImage0->ClearForce(AUTO_Prior);
		}
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutVSW::ResetRsrvImage0(DWORD partyRscId)
{
	LayoutType confLayoutType = GetConfLayoutType();
	CVidSubImage* SubRsrvImage0 = m_pReservation[confLayoutType]->GetSubImageNum(0);
	if (CPObject::IsValidPObjectPtr(SubRsrvImage0))
	{
		if (partyRscId == SubRsrvImage0->GetImageId())
		{
			SubRsrvImage0->SetImageId(0);
			SubRsrvImage0->ClearForce(AUTO_Prior);
		}
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutVSW::SetPartyForce(CVideoLayout& pPartyLayout)
{
	LayoutType confLayoutType = GetConfLayoutType();
	PASSERT_AND_RETURN(confLayoutType != CP_LAYOUT_1X1);
	m_pReservation[confLayoutType]->SetLayout(pPartyLayout, PARTY_lev);
}
PBEGIN_MESSAGE_MAP(CBridgePartyVideoOutLegacy)

ONEVENT(ADD_CONTENT_IMAGE ,SETUP ,CBridgePartyVideoOutLegacy::NullActionFunction)
ONEVENT(ADD_CONTENT_IMAGE ,CONNECTED ,CBridgePartyVideoOutLegacy::OnVideoBridgePartyAddContentImageCONNECTED)
ONEVENT(ADD_CONTENT_IMAGE ,CHANGE_LAYOUT ,CBridgePartyVideoOutLegacy::OnVideoBridgePartyAddContentImageCHANGELAYOUT)
ONEVENT(ADD_CONTENT_IMAGE ,DISCONNECTING ,CBridgePartyVideoOutLegacy::NullActionFunction)

ONEVENT(DEL_CONTENT_IMAGE ,IDLE ,CBridgePartyVideoOutLegacy::NullActionFunction)
ONEVENT(DEL_CONTENT_IMAGE ,SETUP ,CBridgePartyVideoOutLegacy::NullActionFunction)
ONEVENT(DEL_CONTENT_IMAGE ,CONNECTED ,CBridgePartyVideoOutLegacy::OnVideoBridgePartyDelContentImageCONNECTED)
ONEVENT(DEL_CONTENT_IMAGE ,CHANGE_LAYOUT ,CBridgePartyVideoOutLegacy::OnVideoBridgePartyDelContentImageCHANGELAYOUT)
ONEVENT(DEL_CONTENT_IMAGE ,DISCONNECTING ,CBridgePartyVideoOutLegacy::NullActionFunction)

PEND_MESSAGE_MAP(CBridgePartyVideoOutLegacy,CBridgePartyVideoOut)
;

////////////////////////////////////////////////////////////////////////////
//                        CBridgePartyVideoOutLegacy
////////////////////////////////////////////////////////////////////////////
CBridgePartyVideoOutLegacy::CBridgePartyVideoOutLegacy() :
				CBridgePartyVideoOut()
{
	m_LastVideoOnlyLayout = CP_NO_LAYOUT;
	m_IsLastVideoLayoutPrivate = NO;
	m_IsOriginalLayoutSaved = NO;
}

//--------------------------------------------------------------------------
CBridgePartyVideoOutLegacy::~CBridgePartyVideoOutLegacy()
{
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutLegacy::AddContentImage()
{
	CSegment* pSeg = new CSegment;

	DispatchEvent(ADD_CONTENT_IMAGE, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutLegacy::OnVideoBridgePartyAddContentImageCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	SaveLastVideoOnlyLayoutAndSetDefaultLayout();

	BOOL bUseSharedMemForChangeLayoutReq = false;
	BuildLayout(bUseSharedMemForChangeLayoutReq);

	HideGatheringText();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutLegacy::OnVideoBridgePartyAddContentImageCHANGELAYOUT(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	SaveLastVideoOnlyLayoutAndSetDefaultLayout();
	ON(m_layoutChangedWhileWaitingForAck);

	HideGatheringText();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutLegacy::DelContentImage()
{
	CSegment* pSeg = new CSegment;

	DispatchEvent(DEL_CONTENT_IMAGE, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutLegacy::OnVideoBridgePartyDelContentImageCONNECTED(CSegment* pParam)
{
	// FEATURE: "Display Content to Legacy EP in Telepresence Conference"
	CVideoBridge* pBridge = (CVideoBridge*) m_pBridgePartyCntl->GetBridge();
	PASSERT_AND_RETURN(!pBridge);

	CTelepresenceLayoutMngr* pTelepresenceLayoutMngr = pBridge->GetTelepresenceLayoutMngr();

	TRACEINTO << m_pBridgePartyCntl->GetName() << ", IsTelepresence:" << (pTelepresenceLayoutMngr ? "1" : "0");

	if (pTelepresenceLayoutMngr && pTelepresenceLayoutMngr->IsDisplayContentToLegacyEP())
	{
		CConf* pConf = pBridge->GetConf();
		PASSERT_AND_RETURN(!pConf);

		CConfParty* pConfParty = pConf->GetCommConf()->GetCurrentParty(m_pBridgePartyCntl->GetName());
		PASSERT_AND_RETURN(!pConfParty);

		SetIsPrivateLayout(NO);
		pConfParty->SetIsPrivateLayout(NO);

		//Bridge-10703
		BOOL bManageTelepresenceLayoutsInternally = false;
		CVideoBridge* pVideoBridge = (CVideoBridge*) (m_pBridgePartyCntl->GetBridge());

		if (pVideoBridge)
			bManageTelepresenceLayoutsInternally = pVideoBridge->GetManageTelepresenceLayoutsInternally();
		else
		{
			PASSERT(1);
		}

		if (!bManageTelepresenceLayoutsInternally)
		{
			TRACEINTO << "MLA manages the layout of legacy EP changing to conference layout only (no build layout)";
			return;
		}
	}

	RestoreLastVideoOnlyLayout();

	BOOL bUseSharedMemForChangeLayoutReq = false;
	BuildLayout(bUseSharedMemForChangeLayoutReq);

	ShowGatheringText();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutLegacy::OnVideoBridgePartyDelContentImageCHANGELAYOUT(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	RestoreLastVideoOnlyLayout();
	ON(m_layoutChangedWhileWaitingForAck);

	ShowGatheringText();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutLegacy::HideGatheringText()
{
	CGathering* pGathering = GetGathering();
	if (pGathering)
	{
		if (!pGathering->IsInitiatedByUser())
		{
			const char* pszPartyName = m_pBridgePartyCntl->GetName();
			TRACEINTO << m_pBridgePartyCntl->GetFullName();

			pGathering->HideGatheringText(pszPartyName);
		}
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutLegacy::ShowGatheringText()
{
	CGathering* pGathering = GetGathering();
	if (pGathering)
	{
		const char* pszPartyName = m_pBridgePartyCntl->GetName();
		TRACEINTO << m_pBridgePartyCntl->GetFullName();

		pGathering->ShowGatheringText(pszPartyName);
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutLegacy::DisplayGathering(CGathering* pGathering)
{
	const char* pszPartyName = m_pBridgePartyCntl->GetName();

	TRACEINTO << "PartyName:" << pszPartyName << ", ConnectionState:" << m_state;

	EGatheringDisplayState eDisplayState = pGathering->GetGatheringDisplayState(pszPartyName);
	bool bHidden = (eDisplayState != eGatheringDisplayStateDisplayed);
	bool bRemoveGatheringText = (pGathering->IsHideGatheringText(pszPartyName) || pGathering->IsEndGathering());

	if (bRemoveGatheringText != bHidden || pGathering->IsForceDisplay(pszPartyName) || pGathering->CheckTimeToShow(pszPartyName))
	{
		if (m_state == CONNECTED || m_state == CHANGE_LAYOUT)
		{
			if (IsContentImageNeedToBeAdded() && !bRemoveGatheringText && !pGathering->IsInitiatedByUser())
			{
				TRACEINTO << "Content is Legacy, Gathering text will not displayed";
			}
			else
			{
				if (bRemoveGatheringText && bHidden)
				{
					TRACEINTO << "Gathering was hidden";
				}
				else
				{
					((CVideoHardwareInterface*) m_pHardwareInterface)->SendGatheringToDisplay(pGathering, pszPartyName);
					if (pGathering->IsEndGathering())
						m_isInGatheringMode = NO;

					if (pGathering->IsEndGathering())
					{
						pGathering->SetGatheringDisplayState(pszPartyName, eGatheringDisplayStateEnded);
						TRACEINTO << "Set Gathering display state, State:eGatheringDisplayStateEnded";
					}
					else if (pGathering->IsHideGatheringText(pszPartyName))
					{
						pGathering->SetGatheringDisplayState(pszPartyName, eGatheringDisplayStateHidden);
						TRACEINTO << "Set Gathering display state, State:eGatheringDisplayStateHidden";
					}
					else
					{
						pGathering->SetGatheringDisplayState(pszPartyName, eGatheringDisplayStateDisplayed);
						TRACEINTO << "Set Gathering display state, State:eGatheringDisplayStateDisplayed";
					}
				}
			}
			pGathering->SetDisplayed(true, pszPartyName);
		}
		else
		{
			pGathering->SetDisplayed(false, pszPartyName);
		}
	}
	else
	{
		if (pGathering->IsEndGathering())
		{
			pGathering->SetDisplayed(false, pszPartyName);
			TRACEINTO << "PartyName:" << pszPartyName << " - It is not time to STOP Gathering";
		}
		else
		{
			pGathering->SetDisplayed(true, pszPartyName);
			TRACEINTO << "PartyName:" << pszPartyName << " - It is not time to SHOW Gathering";
		}
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutLegacy::SetLayoutHandler()
{
	POBJDELETE(m_pCurrHandler);
	// BRIDGE-11211 - lagacy & Telepresence
	CVideoBridgeCP* pVideoBridge = (CVideoBridgeCP*) (m_pBridgePartyCntl->GetBridge());
	BOOL bTelepresenceOnOff = pVideoBridge->GetTelepresenceOnOff();
	BOOL bManageTelepresenceLayoutsInternally = pVideoBridge->GetManageTelepresenceLayoutsInternally();
	CTelepresenseEPInfo telepresenceEPInfo = ((CVideoBridgePartyCntl*) m_pBridgePartyCntl)->GetTelepresenceInfo();
	BOOL bIsConfInActiveContentPresentation = pVideoBridge->IsConfInActiveContentPresentation();
	CTelepresenceLayoutMngr* pTelepresenceLayoutMngr = pVideoBridge->GetTelepresenceLayoutMngr();
	BOOL bIsDisplayContentToLegacyEP = FALSE;
	if (pTelepresenceLayoutMngr)
		bIsDisplayContentToLegacyEP = pTelepresenceLayoutMngr->IsDisplayContentToLegacyEP();

	TRACEINTO << "PartyRsrcId: " << m_pBridgePartyCntl->GetPartyRsrcID() << "\nTelepresenceOnOff: " << (WORD) bTelepresenceOnOff
					<< "\nManageTelepresenceLayoutsInternally: " << (WORD) bManageTelepresenceLayoutsInternally << "\nLinkRoll: "
					<< (WORD) telepresenceEPInfo.GetLinkRole() << "\nbIsConfInActiveContentPresentation: "
					<< (WORD) bIsConfInActiveContentPresentation << "\nbIsDisplayContentToLegacyEP: "
					<< (WORD) bIsDisplayContentToLegacyEP;

	if (bTelepresenceOnOff && bManageTelepresenceLayoutsInternally && (telepresenceEPInfo.GetLinkRole() != 1))//Telepresence = ON, manage Telepresence layouts by RMX and regular party (not cascade link)
		if (bIsConfInActiveContentPresentation && bIsDisplayContentToLegacyEP)// there is active content in conference at the moment and FORCE_LEGACY_EP_CONTENT_LAYOUT_ON_TELEPRESENCE = YES
		{
			// CLayoutHandlerLegacy
			m_pCurrHandler = new CLayoutHandlerLegacy((CVideoBridge*) (m_pBridgePartyCntl->GetBridge()),
							(CVideoBridgePartyCntl*) m_pBridgePartyCntl);
			TRACEINTO << " set CLayoutHandlerLegacy, PartyRsrcId: " << m_pBridgePartyCntl->GetPartyRsrcID();
		}
		else	// no active content in conference at the moment or FORCE_LEGACY_EP_CONTENT_LAYOUT_ON_TELEPRESENCE = NO
		{
			ETelePresenceLayoutMode telepresenceLayoutMode = ((CVideoBridgePartyCntl*) m_pBridgePartyCntl)->GetTelepresenceLayoutMode();
			switch (telepresenceLayoutMode)
			{
				case eTelePresenceLayoutRoomSwitch:
				{
					m_pCurrHandler = new CLayoutHandlerTelepresenceRoomSwitch(pVideoBridge, (CVideoBridgePartyCntl*) m_pBridgePartyCntl);
					TRACEINTO << "TELEPRESENCE_LAYOUTS_DEBUG set CLayoutHandlerTelepresenceRoomSwitch, PartyRsrcId: " << m_pBridgePartyCntl->GetPartyRsrcID();
					break;
				}
				case eTelePresenceLayoutCpSpeakerPriority:
					m_pCurrHandler = new CLayoutHandlerTelepresenceSpeakerModeCP(pVideoBridge, (CVideoBridgePartyCntl*) m_pBridgePartyCntl);
					TRACEINTO << " TELEPRESENCE_LAYOUTS_DEBUG set CLayoutHandlerTelepresenceSpeakerModeCP, PartyRsrcId: " << m_pBridgePartyCntl->GetPartyRsrcID();
					break;

				case eTelePresenceLayoutCpParticipantsPriority:
				{
					m_pCurrHandler = new CLayoutHandlerTelepresencePartyModeCP(pVideoBridge, (CVideoBridgePartyCntl*) m_pBridgePartyCntl);
					TRACEINTO << " TELEPRESENCE_LAYOUTS_DEBUG set CLayoutHandlerTelepresencePartyModeCP, PartyRsrcId: " << m_pBridgePartyCntl->GetPartyRsrcID();
					break;
				}
				case eTelePresenceLayoutManual:
				case eTelePresenceLayoutContinuousPresence:
				default:
				{
					m_pCurrHandler = new CLayoutHandler(pVideoBridge, (CVideoBridgePartyCntl*) m_pBridgePartyCntl);
					TRACEINTO << " telepresenceLayoutMode not managed by MCU set CLayoutHandler (regular), PartyRsrcId: "
									<< m_pBridgePartyCntl->GetPartyRsrcID();
				}
			}
		}
	else	// no telepresence or not managed by RMX or link role
	{
		// CLayoutHandlerLegacy
		m_pCurrHandler = new CLayoutHandlerLegacy((CVideoBridge*) (m_pBridgePartyCntl->GetBridge()),
						(CVideoBridgePartyCntl*) m_pBridgePartyCntl);
		TRACEINTO << " set CLayoutHandlerLegacy, PartyRsrcId: " << m_pBridgePartyCntl->GetPartyRsrcID();
	}
}
//--------------------------------------------------------------------------
void CBridgePartyVideoOutLegacy::SaveLastVideoOnlyLayoutAndSetDefaultLayout()
{
	const bool isPrivateLayout = GetIsPrivateLayout();
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", isPrivateLayout:" << isPrivateLayout << ", isForce1x1:" << m_isForce1x1;

	if (true == isPrivateLayout && m_isForce1x1) // only for cascade link save the 1x1
		return;

	if (YES == IsContentImageNeedToBeAdded())
	{
		if (NO == IsOriginalLayoutSaved())
		{
			SetIsLastVideoLayoutPrivate(isPrivateLayout);
			if (isPrivateLayout)
			{
				// In case we are in private layout there are case were the current view is not updated with the private layout as in lecturemode case
				CLayout* privateReservationLayout = GetPrivateReservationLayout();
				if (IsValidPObjectPtr(privateReservationLayout))
					SetLastVideoOnlyLayout(privateReservationLayout->GetLayoutType());
			}
			else
			{
				SetLastVideoOnlyLayout(m_pCurrentView->GetLayoutType());
			}

			SetIsOriginalLayoutSaved(YES);
		}

		CVideoBridge* pVideoBridge = (CVideoBridge*) m_pBridgePartyCntl->GetBridge();

		LayoutType defaultLayoutType = pVideoBridge->GetLegacyContentDefaultLayout();

		if (isPrivateLayout)
		{
			CLayout* pCurrLayout = GetCurrentLayout();
			LayoutType currentLayoutType = pCurrLayout->GetLayoutType();

			if (defaultLayoutType == CP_LAYOUT_1P4VER && currentLayoutType == CP_LAYOUT_1P4VER)
			{
				CImage* pImage = pVideoBridge->GetPartyImage(*pCurrLayout, 0);
				DWORD dwArtPartyID = (pImage) ? pImage->GetArtPartyId() : INVALID;

				if (dwArtPartyID == INVALID)
					SetGatheringLayoutForParty(false);
			}
		}

		SetPrivateLayoutForParty(defaultLayoutType);
	}
}
//--------------------------------------------------------------------------
void CBridgePartyVideoOutLegacy::OnVideoBridgePartyDisConnect(CSegment* pSeg)
{
	PASSERT_AND_RETURN(m_IsOriginalLayoutSaved && m_LastVideoOnlyLayout >= CP_NO_LAYOUT);
	TRACEINTO << "IsOriginalLayoutSaved: " << (WORD) m_IsOriginalLayoutSaved << ", IsLastVideoLayoutPrivate: "
					<< (WORD) m_IsLastVideoLayoutPrivate;
	// if the there is a previous private layout that was active before the legacy content layout, we'll restore the info in the DB
	if (m_IsOriginalLayoutSaved && m_IsLastVideoLayoutPrivate)
	{
		TRACEINTO << "restoring last private layout: " << LayoutTypeAsString[m_LastVideoOnlyLayout];
		CSegment vidLayoutSeg;
		CLayout *prevLayout = new CLayout(m_LastVideoOnlyLayout, m_pBridgePartyCntl->GetConfName());
		if (!prevLayout->Serialize(PARTY_lev, &vidLayoutSeg))
		{
			m_pBridgePartyCntl->GetConfApi()->UpdateDB(m_pBridgePartyCntl->GetPartyTaskApp(), CPPARTYLAYOUT,
							m_IsLastVideoLayoutPrivate, 0, &vidLayoutSeg);
			m_pBridgePartyCntl->GetConfApi()->UpdateDB(m_pBridgePartyCntl->GetPartyTaskApp(), PRIVATEON,
							m_IsLastVideoLayoutPrivate);
		}
		else
		{
			PASSERT(1);
		}
	}
	// otherwise, just inactivate the current private layout
	else
		m_pBridgePartyCntl->GetConfApi()->UpdateDB(m_pBridgePartyCntl->GetPartyTaskApp(), PRIVATEON, (DWORD) 0);

	CBridgePartyVideoOut::OnVideoBridgePartyDisConnect(pSeg);
}
//--------------------------------------------------------------------------
void CBridgePartyVideoOutLegacy::RestoreLastVideoOnlyLayout()
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	CGathering* pGathering = GetGathering();

	bool bPrivateGatheringLayout = false;
	if (pGathering)
	{
		if (pGathering->IsPrivateLayout())
			bPrivateGatheringLayout = true;
	}
	if (IsLastVideoLayoutPrivate() && bPrivateGatheringLayout)
	{
		LayoutType lastVideoOnlyLayoutType = GetLastVideoOnlyLayout();
		SetGatheringLayoutForParty(true);
		SetPrivateLayoutForParty(lastVideoOnlyLayoutType);
	}
	else if (m_isForce1x1)
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Party is Cascade link, force 1X1 layout";
		SetPrivateLayoutForParty(CP_LAYOUT_1X1);
	}
	else
	{
		SetIsPrivateLayout(NO);
	}

	SetIsOriginalLayoutSaved(NO);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutLegacy::OnMplOpenPortAck(STATUS status)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", Status:" << status;

	if (status != STATUS_OK)
	{
		CSegment* pSeg = new CSegment;
		*pSeg << (BYTE) statVideoInOutResourceProblem << (BYTE) eMipOut << (BYTE) eMipStatusFail << (BYTE) eMipOpen;

		// Inform BridgePartyCntl of statVideoInOutResourceProblem -> Inorder to initiate Kill port on VideoEncoder VideoDecoder
		m_pBridgePartyCntl->HandleEvent(pSeg, 0, VIDEO_OUT_CONNECTED);
		POBJDELETE(pSeg);
	}
	else
	{
		m_state = CONNECTED;

		CSegment *pMsg = new CSegment;
		*pMsg << (BYTE) statOK;
		m_pBridgePartyCntl->HandleEvent(pMsg, 0, VIDEO_OUT_CONNECTED);
		POBJDELETE(pMsg);

		SetRecordingType(((CVideoBridge*) m_pBridgePartyCntl->GetBridge())->GetRecordingIcon());
		SetNumAudioParticipantsInConf(((CVideoBridge*) m_pBridgePartyCntl->GetBridge())->GetNumAudioParticipants());
		m_isAudioIconToSentAfterOpenPort = YES; //to prevent conf to hide the audio icon before this party build layout.
		InitIsGatheringModeEnabled();

		SaveLastVideoOnlyLayoutAndSetDefaultLayout();
		BuildLayout();

		if (m_pWaitingForUpdateParams)
		{
			SaveAndSendUpdatedVideoParams(m_pWaitingForUpdateParams);
			POBJDELETE(m_pWaitingForUpdateParams);
		}
	}
}

//--------------------------------------------------------------------------
BOOL CBridgePartyVideoOutLegacy::RejectChangeLayoutRequestBecauseOfApplications()
{
	if (m_partyLectureModeRole == eLISTENER || m_partyLectureModeRole == eCOLD_LISTENER)
	{
		if (!IsContentImageNeedToBeAdded())
			return true;
	}

	return false;
}

//--------------------------------------------------------------------------
LayoutType CBridgePartyVideoOutLegacy::GetPartyCurrentLayoutType() const
{
	LayoutType newLayoutType;

	if (m_partyLectureModeRole == eLISTENER || m_partyLectureModeRole == eCOLD_LISTENER)
	{
		if (!IsContentImageNeedToBeAdded())
		{
			newLayoutType = CP_LAYOUT_1X1;
		}
		else if (m_isPrivate)
		{
			newLayoutType = m_PrivatelayoutType;
		}
		else
		{
			newLayoutType = GetConfLayoutType();
		}
	}

	else if (m_isPrivate)
	{
		newLayoutType = m_PrivatelayoutType;
	}
	else
	{
		newLayoutType = GetConfLayoutType();
	}

	return newLayoutType;
}

//--------------------------------------------------------------------------
BOOL CBridgePartyVideoOutLegacy::IsContentImageNeedToBeAdded() const
{
	return ((CVideoBridgeCPContent*) m_pBridgePartyCntl->GetBridge())->IsContentImageNeedToBeAdded();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutLegacy::SetIsOriginalLayoutSaved(BOOL yesNo)
{
	m_IsOriginalLayoutSaved = yesNo;
}

//--------------------------------------------------------------------------
BOOL CBridgePartyVideoOutLegacy::IsOriginalLayoutSaved() const
{
	return m_IsOriginalLayoutSaved;
}
///////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CBridgePartyVideoOutCOP)

ONEVENT(ACK_IND, UPDATE_ENCODER, CBridgePartyVideoOutCOP::OnMplAckUPDATE_ENCODER)
ONEVENT(ACK_IND, CONNECTING_PCM, CBridgePartyVideoOutCOP::OnMplAckCONNECTING_PCM)
ONEVENT(ACK_IND, DISCONNECTING_PCM, CBridgePartyVideoOutCOP::OnMplAckDISCONNECTING_PCM)

ONEVENT(DISCONNECT_VIDEO_OUT, UPDATE_ENCODER, CBridgePartyVideoOutCOP::OnVideoBridgePartyDisConnectUPDATE_ENCODER)
ONEVENT(DISCONNECT_VIDEO_OUT, CONNECTING_PCM, CBridgePartyVideoOutCOP::OnVideoBridgePartyDisConnectCONNECTING_PCM)
ONEVENT(DISCONNECT_VIDEO_OUT, PCM, CBridgePartyVideoOutCOP::OnVideoBridgePartyDisConnectPCM)
ONEVENT(DISCONNECT_VIDEO_OUT, DISCONNECTING_PCM, CBridgePartyVideoOutCOP::OnVideoBridgePartyDisConnectDISCONNECTING_PCM)

ONEVENT(UPDATE_VIDEO_OUT_PARAMS, UPDATE_ENCODER, CBridgePartyVideoOutCOP::OnVideoBridgePartyUpdateVideoParamsUPDATE_ENCODER)
ONEVENT(UPDATE_VIDEO_OUT_PARAMS, CONNECTING_PCM, CBridgePartyVideoOutCOP::OnVideoBridgePartyUpdateVideoParamsCONNECTING_PCM)
ONEVENT(UPDATE_VIDEO_OUT_PARAMS, PCM, CBridgePartyVideoOutCOP::OnVideoBridgePartyUpdateVideoParamsPCM)
ONEVENT(UPDATE_VIDEO_OUT_PARAMS, DISCONNECTING_PCM, CBridgePartyVideoOutCOP::OnVideoBridgePartyUpdateVideoParamsDISCONNECTING_PCM)

ONEVENT(CONNECT_PARTY_TO_PCM, IDLE, CBridgePartyVideoOutCOP::OnVideoBridgeConnectPartyToPcmIDLE)
ONEVENT(CONNECT_PARTY_TO_PCM, SETUP, CBridgePartyVideoOutCOP::OnVideoBridgeConnectPartyToPcmSETUP)
ONEVENT(CONNECT_PARTY_TO_PCM, SLIDE, CBridgePartyVideoOutCOP::OnVideoBridgeConnectPartyToPcmSLIDE)
ONEVENT(CONNECT_PARTY_TO_PCM, CONNECTED, CBridgePartyVideoOutCOP::OnVideoBridgeConnectPartyToPcmCONNECTED)
ONEVENT(CONNECT_PARTY_TO_PCM, DISCONNECTING, CBridgePartyVideoOutCOP::NullActionFunction)
ONEVENT(CONNECT_PARTY_TO_PCM, UPDATE_ENCODER, CBridgePartyVideoOutCOP::OnVideoBridgeConnectPartyToPcmUPDATE_ENCODER)
ONEVENT(CONNECT_PARTY_TO_PCM, CONNECTING_PCM, CBridgePartyVideoOutCOP::OnVideoBridgeConnectPartyToPcmCONNECTING_PCM)
ONEVENT(CONNECT_PARTY_TO_PCM, PCM, CBridgePartyVideoOutCOP::OnVideoBridgeConnectPartyToPcmPCM)
ONEVENT(CONNECT_PARTY_TO_PCM, DISCONNECTING_PCM, CBridgePartyVideoOutCOP::OnVideoBridgeConnectPartyToPcmDISCONNECTING_PCM)

ONEVENT(DISCONNECT_PARTY_FROM_PCM, CONNECTING_PCM, CBridgePartyVideoOutCOP::OnVideoBridgeDisconnectPartyFromPcmCONNECTING_PCM)
ONEVENT(DISCONNECT_PARTY_FROM_PCM, PCM, CBridgePartyVideoOutCOP::OnVideoBridgeDisconnectPartyFromPcm)
ONEVENT(DISCONNECT_PARTY_FROM_PCM, ANYCASE, CBridgePartyVideoOutCOP::NullActionFunction)

PEND_MESSAGE_MAP(CBridgePartyVideoOutCOP,CBridgePartyVideoOut)
;

////////////////////////////////////////////////////////////////////////////
CBridgePartyVideoOutCOP::CBridgePartyVideoOutCOP() :
				CBridgePartyVideoOut()
{
	m_videoConfType = eVideoConfTypeCopHD108025fps; // This is the default if it eVideoConfTypeCopHD72050fps we will receive via the initparams
	m_copEncoderConnectionId = DUMMY_CONNECTION_ID;
	m_copEncoderPartyId = DUMMY_PARTY_ID;
	m_copEncoderIndex = (0xFFFF);

	m_pcmEncoderConnectionId = DUMMY_CONNECTION_ID;
	m_pcmEncoderEntityId = DUMMY_PARTY_ID;

	m_needToConnectToPcmEncoder = false;
	m_needToDisconnectFromPcmEncoder = false;

	m_disconnectAckReceived = false;

	m_copResourceIndexOfCascadeLinkLecturer = (0xFFFF);
	m_bCascadeIsLecturer = false;
}

//--------------------------------------------------------------------------
CBridgePartyVideoOutCOP::~CBridgePartyVideoOutCOP()
{
}
// ---------------------------------------------------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::Create(const CBridgePartyCntl* pBridgePartyCntl, const CRsrcParams* pRsrcParams,
				const CBridgePartyMediaParams* pBridgePartyVideoOutParams)
{
	CBridgePartyVideoOut::Create(pBridgePartyCntl, pRsrcParams, pBridgePartyVideoOutParams); // Some of the things aren't relevant in cop but I dont think cause problem keren

	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	CBridgePartyVideoOutParams* pVideoOutParams = (CBridgePartyVideoOutParams*) pBridgePartyVideoOutParams;
	// get updated with the connection Id and party Id of the cop encoder this party will be connected to
	m_copEncoderConnectionId = ((CBridgePartyVideoParams*) pBridgePartyVideoOutParams)->GetCopConnectionId();
	m_copEncoderPartyId = ((CBridgePartyVideoParams*) pBridgePartyVideoOutParams)->GetCopPartyId();
	m_copEncoderIndex = ((CBridgePartyVideoParams*) pBridgePartyVideoOutParams)->GetCopResourceIndex();
	m_videoFrameRate = pVideoOutParams->GetVidFrameRate(); // relevant only in cop conferences
	m_videoConfType = pVideoOutParams->GetVideoConfType();
	m_copResourceIndexOfCascadeLinkLecturer = pVideoOutParams->GetCopResourceOfLecturerLinkIndex();
	m_bCascadeIsLecturer = pVideoOutParams->GetIsCopLinkLecturer();
}

//--------------------------------------------------------------------------
WORD CBridgePartyVideoOutCOP::IsValidState(WORD state) const
{
	WORD valid_state = true;
	switch (state)
	{
	case CHANGE_LAYOUT:
	{
		valid_state = false;
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Failed, state is not valid";
		PASSERT(1);
		break;
	}
	} // switch

	return valid_state;
}

//--------------------------------------------------------------------------
BOOL CBridgePartyVideoOutCOP::IsConnected()
{
	return (m_state == CONNECTED || m_state == UPDATE_ENCODER || m_state == CONNECTING_PCM || m_state == PCM
					|| m_state == DISCONNECTING_PCM);
}

//--------------------------------------------------------------------------
BOOL CBridgePartyVideoOutCOP::IsConnectedOrConnectingPCM()
{
	return (m_state == CONNECTING_PCM || m_state == PCM);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::SendConnectToRtp()
{
	PASSERT_AND_RETURN(!m_pHardwareInterface);

	ConnectionID rtpConnectionId = DUMMY_CONNECTION_ID;
	ConnectionID encoderConnectionId = m_copEncoderConnectionId;
	PartyRsrcID encoderRsrcPartyId = m_copEncoderPartyId;
	PartyRsrcID rtpRsrcPartyId = m_pHardwareInterface->GetPartyRsrcId();

	CRsrcDesc* pRsrcDesc;
	// Eitan - ISDN party has no rtp - connect MUX instead?
	if (m_pBridgePartyCntl->GetNetworkInterface() == ISDN_INTERFACE_TYPE)
		pRsrcDesc = ::GetpConfPartyRoutingTable()->GetPartyRsrcDesc(m_pHardwareInterface->GetPartyRsrcId(), eLogical_mux);
	else
		pRsrcDesc = ::GetpConfPartyRoutingTable()->GetPartyRsrcDesc(m_pHardwareInterface->GetPartyRsrcId(), eLogical_rtp);

	if (pRsrcDesc)
	{
		rtpConnectionId = pRsrcDesc->GetConnectionId();
	}
	else
	{
		DBGPASSERT(101);
	}

	m_lastReqId = ((CVideoHardwareInterface*) m_pHardwareInterface)->SendConnect(rtpConnectionId, encoderConnectionId,
					rtpRsrcPartyId, encoderRsrcPartyId);
	m_lastReq = TB_MSG_CONNECT_REQ;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnMplConnectAckSETUP(STATUS status)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", Status:" << status;

	if (status != STATUS_OK)
	{
		// Add assert to EMA in case of NACK
		AddFaultAlarm("NACK on Connect video encoder", m_pHardwareInterface->GetPartyRsrcId(), status);

		CSegment* pSeg = new CSegment;
		*pSeg << (BYTE) statVideoInOutResourceProblem << (BYTE) eMipOut << (BYTE) eMipStatusFail << (BYTE) eMipConnect;

		// Inform BridgePartyCntl of statVideoInOutResourceProblem -> Inorder to initiate Kill port on VideoEncoder VideoDecoder
		m_pBridgePartyCntl->HandleEvent(pSeg, 0, VIDEO_OUT_CONNECTED);

		POBJDELETE(pSeg);
		return;
	}
	else
	{
		if (m_pWaitingForUpdateParams)
		{
			DWORD copEncoderConnectionId = m_pWaitingForUpdateParams->GetCopConnectionId();
			DWORD copEncoderPartyId = m_pWaitingForUpdateParams->GetCopPartyId();
			if ((m_copEncoderConnectionId != copEncoderConnectionId) || (m_copEncoderPartyId != copEncoderPartyId))
			{
				TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Update video-out params";
				// 1. Disconnect From RTP with the prev Encoder Prams.
				SendDisconnectFromRtp();
				// 2. Change state
				m_state = UPDATE_ENCODER;
				// 3. save the new Encoder params
				m_copEncoderConnectionId = copEncoderConnectionId;
				m_copEncoderPartyId = copEncoderPartyId;
			}
			else
			{
				TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Update video-out (only save) params";
				m_state = CONNECTED;
				SaveAndSendUpdatedVideoParams(m_pWaitingForUpdateParams);
				POBJDELETE(m_pWaitingForUpdateParams);
			}
		}
		else
		{
			m_state = CONNECTED;
		}

		CSegment* pMsg = new CSegment;
		*pMsg << (BYTE) statOK;
		m_pBridgePartyCntl->HandleEvent(pMsg, 0, VIDEO_OUT_CONNECTED);
		POBJDELETE(pMsg);

		if (m_needToConnectToPcmEncoder)
		{
			DispatchEvent(CONNECT_PARTY_TO_PCM, NULL);
		}
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnMplOpenPortAck(STATUS status)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Event is ignored";
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnVideoBridgePartyDisConnectSETUP(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	OnVideoBridgePartyDisConnect(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnVideoBridgePartyDisConnectUPDATE_ENCODER(CSegment* pParam)
{
	m_state = DISCONNECTING;
	if (m_disconnectAckReceived)
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName();
		SendDisconnectFromRtp();
	}
	else
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName()
						<< " - Did not receive disconnect_ack in update encoder (disconnect art<->prev encoder), change state to DISCONNECTING and handle the disconnect_ack in this state";
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnVideoBridgePartyDisConnectCONNECTING_PCM(CSegment* pParam)
{
	//inform bridge and pcm mngr on party disconnection from pcm
	STATUS status = STATUS_OK;
	OnPCMDisconnectAck(status);
	SendAckIndToPCM(VIDEO_OUT_DISCONNECTED, 0, status);

	m_state = DISCONNECTING;
	if (m_disconnectAckReceived)
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName();
		DisconnectRtpFromPCMEncoder();
	}
	else
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName()
						<< " - Did not receive disconnect_ack in connecting pcm process (disconnect art<->encoder), change state to DISCONNECTING and handle the disconnect_ack in this state";
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnVideoBridgePartyDisConnectPCM(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
//inform bridge and pcm mngr on party disconnection from pcm
	STATUS status = STATUS_OK;
	OnPCMDisconnectAck(status);
	SendAckIndToPCM(VIDEO_OUT_DISCONNECTED, 0, status);

	m_state = DISCONNECTING;
	DisconnectRtpFromPCMEncoder();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnVideoBridgePartyDisConnectDISCONNECTING_PCM(CSegment* pParam)
{
	//inform bridge and pcm mngr on party disconnection from pcm
	STATUS status = STATUS_OK;
	OnPCMDisconnectAck(status);
	SendAckIndToPCM(VIDEO_OUT_DISCONNECTED, 0, status);

	if (m_disconnectAckReceived)
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName();
		OnVideoBridgePartyDisConnect(pParam);
	}
	else
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName()
						<< " - Did not receive disconnect_ack in disconnecting pcm process (disconnect art<->pcm_encoder), change state to DISCONNECTING and handle the disconnect_ack in this state";
		m_state = DISCONNECTING;
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnVideoBridgePartyDisConnect(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	m_state = DISCONNECTING;
	SendDisconnectFromRtp();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::SendDisconnectFromRtp()
{
	PASSERT_AND_RETURN(!m_pHardwareInterface);

	ConnectionID rtpConnectionId = DUMMY_CONNECTION_ID;
	ConnectionID encoderConnectionId = m_copEncoderConnectionId;
	PartyRsrcID encoderRsrcPartyId = m_copEncoderPartyId;
	PartyRsrcID rtpRsrcPartyId = m_pHardwareInterface->GetPartyRsrcId();

	CRsrcDesc* pRsrcDesc;
	// Eitan - ISDN party has no rtp - connect MUX instead?
	if (m_pBridgePartyCntl->GetNetworkInterface() == ISDN_INTERFACE_TYPE)
		pRsrcDesc = ::GetpConfPartyRoutingTable()->GetPartyRsrcDesc(m_pHardwareInterface->GetPartyRsrcId(), eLogical_mux);
	else
		pRsrcDesc = ::GetpConfPartyRoutingTable()->GetPartyRsrcDesc(m_pHardwareInterface->GetPartyRsrcId(), eLogical_rtp);

	if (pRsrcDesc)
	{
		rtpConnectionId = pRsrcDesc->GetConnectionId();
	}
	else
	{
		DBGPASSERT(101);
	}

	((CVideoHardwareInterface*) m_pHardwareInterface)->SendDisconnect(rtpConnectionId, encoderConnectionId, rtpRsrcPartyId,
					encoderRsrcPartyId);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnMplAckDISCONNECTING(CSegment* pParam)
{
	OPCODE AckOpcode;
	DWORD ack_seq_num;
	STATUS status;
	*pParam >> AckOpcode >> ack_seq_num >> status;
	switch (AckOpcode)
	{
	case TB_MSG_DISCONNECT_REQ:
	{
		OnMplDisconnectAckDISCONNECTING(status);
		break;
	}

	default:
	{
		TRACEINTO << m_pBridgePartyCntl->GetName() << ", AckOpcode:" << CProcessBase::GetProcess()->GetOpcodeAsString(AckOpcode)
						<< " - Ignored";
		break;
	}
	} // end switch
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnMplDisconnectAckDISCONNECTING(STATUS status)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", Status:" << status;

	BYTE responseStatus = statOK;
	if (status != STATUS_OK)
	{
		// Add assert to EMA in case of NACK
		AddFaultAlarm("NACK on Disconnect video encoder", m_pHardwareInterface->GetPartyRsrcId(), status);
		responseStatus = statVideoInOutResourceProblem; // statVideoInOutResourceProblem -> Inorder to initiate Kill port on VideoEncoder VideoDecoder
	}

	m_state = IDLE;

	CSegment* pSeg = new CSegment;
	*pSeg << (BYTE) responseStatus;
	if (responseStatus == statVideoInOutResourceProblem)
		*pSeg << (BYTE) eMipOut << (BYTE) eMipStatusFail << (BYTE) eMipDisconnect;

	SetClosePortAckStatus(responseStatus);

	// Inform BridgePartyCntl
	m_pBridgePartyCntl->HandleEvent(pSeg, 0, VIDEO_OUT_DISCONNECTED);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnMplAckSETUP(CSegment* pParam)
{
	OPCODE AckOpcode;
	DWORD ack_seq_num;
	STATUS status;
	*pParam >> AckOpcode >> ack_seq_num >> status;

	switch (AckOpcode)
	{
	case TB_MSG_CONNECT_REQ:
	{
		OnMplConnectAckSETUP(status);
		break;
	}

	case IVR_SHOW_SLIDE_REQ:
	case IVR_STOP_SHOW_SLIDE_REQ:
	{
		CSegment* pSeg = new CSegment;
		*pSeg << AckOpcode << ack_seq_num << status << *pParam;

		((CVideoBridgePartyCntl*) m_pBridgePartyCntl)->IvrNotification(ACK_IND, pSeg);

		POBJDELETE(pSeg);

		break;
	}

	default:
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", AckOpcode:"
						<< CProcessBase::GetProcess()->GetOpcodeAsString(AckOpcode) << " - Ignored";
		break;
	}
	} // end switch
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnVideoBridgePartyUpdateVideoParamsCONNECTED(CSegment* pParam)
{
	CBridgePartyVideoParams params;
	params.DeSerialize(NATIVE, *pParam);

	// get updated with the connection Id and party Id of the cop encoder this party will be connected to
	DWORD copEncoderConnectionId = params.GetCopConnectionId();
	DWORD copEncoderPartyId = params.GetCopPartyId();
	WORD copEncoderIndex = params.GetCopResourceIndex();
	WORD copResourceIndexOfCascadeLinkLecturer = params.GetCopResourceOfLecturerLinkIndex();
	BYTE cascadeIsLecturer = params.GetIsCopLinkLecturer();

	m_copEncoderIndex = copEncoderIndex;
	m_copResourceIndexOfCascadeLinkLecturer = copResourceIndexOfCascadeLinkLecturer;
	m_bCascadeIsLecturer = cascadeIsLecturer;
	m_isH263Plus = params.GetIsH263Plus();

	TRACEINTO << "OldCopEncoderConnectionId:" << m_copEncoderConnectionId << ", NewCopEncoderConnectionId:"
					<< copEncoderConnectionId << ", OldCopEncoderPartyId:" << m_copEncoderPartyId << ", NewCopEncoderPartyId:"
					<< copEncoderPartyId;

	if ((copEncoderConnectionId != m_copEncoderConnectionId) || (copEncoderPartyId != m_copEncoderPartyId))
	{
		// 1. Disconnect From RTP with the prev Encoder Prams.
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Disconnection old Encoder from RTP";
		SendDisconnectFromRtp();
		// 2. Change state
		m_state = UPDATE_ENCODER;
		// 3. save the new Encoder connection Id and party Id
		m_copEncoderConnectionId = copEncoderConnectionId;
		m_copEncoderPartyId = copEncoderPartyId;

		// save the new encoder parameters
		if (m_pWaitingForUpdateParams)
			POBJDELETE(m_pWaitingForUpdateParams);

		m_pWaitingForUpdateParams = new CBridgePartyVideoParams;
		*m_pWaitingForUpdateParams = params;
	}
	else
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName()
						<< " - There is no change in the encoder, the party is connected to, we shouldn't get Update video out params request";
		CSegment msg;
		msg << BYTE(statOK) << false;
		m_pBridgePartyCntl->HandleEvent(&msg, 0, PARTY_VIDEO_OUT_UPDATED);
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::SaveAndSendUpdatedVideoParams(CBridgePartyVideoParams* pBridgePartyVideoParams)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	BOOL IsAckParams = false;
	CAckParams* pAckParams = NULL;

	if (!pBridgePartyVideoParams)
	{
		PASSERTMSG(1, "Internal Error received invalid params");

		CSegment msg;
		msg << BYTE(statIllegal) << false;
		m_pBridgePartyCntl->HandleEvent(&msg, 0, PARTY_VIDEO_OUT_UPDATED);
		return;
	}

	DWORD newVideoAlg = pBridgePartyVideoParams->GetVideoAlgorithm();
	DWORD newVideoBitRate = pBridgePartyVideoParams->GetVideoBitRate();
	eVideoFrameRate newVideoQCifFrameRate = pBridgePartyVideoParams->GetVideoFrameRate(eVideoResolutionQCIF);
	eVideoFrameRate newVideoCifFrameRate = pBridgePartyVideoParams->GetVideoFrameRate(eVideoResolutionCIF);
	eVideoFrameRate newVideo4CifFrameRate = pBridgePartyVideoParams->GetVideoFrameRate(eVideoResolution4CIF);
	eVideoResolution newVideoResolution = pBridgePartyVideoParams->GetVideoResolution();
	DWORD newMBPS = pBridgePartyVideoParams->GetMBPS();
	DWORD newFS = pBridgePartyVideoParams->GetFS();
	DWORD newSampleAspectRatio = pBridgePartyVideoParams->GetSampleAspectRatio();
	DWORD newStaticMB = pBridgePartyVideoParams->GetStaticMB();
	eVideoFrameRate videoFrameRate = pBridgePartyVideoParams->GetVidFrameRate(); // relevant only in cop conferences
	eVideoConfType videoConfType = pBridgePartyVideoParams->GetVideoConfType();
	DWORD newMaxDPB = pBridgePartyVideoParams->GetMaxDPB();
	WORD copEncoderIndex = pBridgePartyVideoParams->GetCopResourceIndex();
	WORD copResourceIndexOfCascadeLinkLecturer = pBridgePartyVideoParams->GetCopResourceOfLecturerLinkIndex();
	BYTE cascadeIsLecturer = pBridgePartyVideoParams->GetIsCopLinkLecturer();

	if (m_videoAlg == newVideoAlg && m_videoBitRate == newVideoBitRate && m_videoQcifFrameRate == newVideoQCifFrameRate
					&& m_videoCifFrameRate == newVideoCifFrameRate && m_video4CifFrameRate == newVideo4CifFrameRate
					&& m_videoResolution == newVideoResolution && m_MBPS == newMBPS && m_FS == newFS
					&& m_sampleAspectRatio == newSampleAspectRatio && m_staticMB == newStaticMB
					&& videoFrameRate == m_videoFrameRate && videoConfType == m_videoConfType && m_maxDPB == newMaxDPB
					&& m_copEncoderIndex == copEncoderIndex
					&& m_copResourceIndexOfCascadeLinkLecturer == copResourceIndexOfCascadeLinkLecturer
					&& m_bCascadeIsLecturer == cascadeIsLecturer)
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - No Real Change In Params";

		CSegment msg; // WE WILL INFORM ANYWAY BECAUSE THE THE CONNECTION ID AND PARTY ID CHANGED
		msg << (BYTE) statOK;

		if (pBridgePartyVideoParams->GetAckParams())
		{
			msg << true;
			pBridgePartyVideoParams->GetAckParams()->Serialize(NATIVE, msg);
		}
		else
			msg << false;

		m_pBridgePartyCntl->HandleEvent(&msg, 0, PARTY_VIDEO_OUT_UPDATED);
	}
	else
	{
		m_videoAlg = newVideoAlg;
		m_videoBitRate = newVideoBitRate;
		m_videoResolution = newVideoResolution; // In COP the resolution isn't relevant onlt for H263
		m_videoFrameRate = videoFrameRate;
		m_videoConfType = videoConfType;
		m_copEncoderIndex = copEncoderIndex;
		m_copResourceIndexOfCascadeLinkLecturer = copResourceIndexOfCascadeLinkLecturer;
		m_bCascadeIsLecturer = cascadeIsLecturer;

		if (m_videoAlg == H264)
		{
			m_videoQcifFrameRate = eVideoFrameRateDUMMY;
			m_videoCifFrameRate = eVideoFrameRateDUMMY;
			m_video4CifFrameRate = eVideoFrameRateDUMMY;
			m_MBPS = newMBPS;
			m_FS = newFS;
			m_staticMB = newStaticMB;
			m_maxDPB = newMaxDPB;
		}
		else
		{
			m_videoQcifFrameRate = newVideoQCifFrameRate;
			m_videoCifFrameRate = newVideoCifFrameRate;
			m_video4CifFrameRate = newVideo4CifFrameRate;
			m_MBPS = INVALID;
			m_FS = INVALID;
			m_staticMB = DEFAULT_STATIC_MB;
			m_maxDPB = INVALID;
			m_isH263Plus = pBridgePartyVideoParams->GetIsH263Plus();
		}

		m_sampleAspectRatio = newSampleAspectRatio;

		CSegment msg;
		msg << BYTE(statOK);

		if (pBridgePartyVideoParams->GetAckParams())
		{
			msg << true;
			pBridgePartyVideoParams->GetAckParams()->Serialize(NATIVE, msg);
		}
		else
			msg << false;

		m_pBridgePartyCntl->HandleEvent(&msg, 0, PARTY_VIDEO_OUT_UPDATED);
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnMplAckUPDATE_ENCODER(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	OPCODE AckOpcode;
	DWORD ack_seq_num;
	STATUS status;
	*pParam >> AckOpcode >> ack_seq_num >> status;

	switch (AckOpcode)
	{
	case TB_MSG_CONNECT_REQ:
	{
		m_disconnectAckReceived = false;
		OnMplConnectAckUPDATE_ENCODER(status);
		break;
	}

	case TB_MSG_DISCONNECT_REQ:
	{
		m_disconnectAckReceived = true;
		OnMplDisconnectAckUPDATE_ENCODER(status);
		break;
	}

	default:
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", AckOpcode:"
						<< CProcessBase::GetProcess()->GetOpcodeAsString(AckOpcode) << " - Ignored";
		break;
	}
	} // end switch
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnMplDisconnectAckUPDATE_ENCODER(STATUS status)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", Status:" << status;

	BYTE responseStatus = statOK;
	if (status != STATUS_OK)
	{
		PASSERT(101);
		CSegment* pMsg = new CSegment;
		*pMsg << (BYTE) statIllegal;
		m_pBridgePartyCntl->HandleEvent(pMsg, 0, PARTY_VIDEO_OUT_UPDATED);
		POBJDELETE(pMsg);
		// TODO once we define the error handeling flows we should understand what should be the state in this stage!!

		// vngr-13157
		// prevent from CBridgePartyVideoOutCOP to stuck in UPDATE_ENCODER state
		SendConnectToRtp();
	}

	SendConnectToRtp(); // the new encoder params should be already updated
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnMplConnectAckUPDATE_ENCODER(STATUS status)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", Status:" << status;

	BYTE responseStatus = statOK;
	if (status != STATUS_OK)
	{
		PASSERT(101);
		CSegment* pMsg = new CSegment;
		*pMsg << (BYTE) statIllegal;
		m_pBridgePartyCntl->HandleEvent(pMsg, 0, PARTY_VIDEO_OUT_UPDATED);
		POBJDELETE(pMsg);
		// TODO once we define the error handeling flows we should understand what should be the state in this stage!!

		// vngr-13157
		// prevent from CBridgePartyVideoOutCOP to stuck in UPDATE_ENCODER state
		m_state = CONNECTED;
	}
	else
	{
		m_state = CONNECTED;
		if (m_pWaitingForUpdateParams)
		{
			DWORD copEncoderConnectionId = m_pWaitingForUpdateParams->GetCopConnectionId();
			DWORD copEncoderPartyId = m_pWaitingForUpdateParams->GetCopPartyId();
			WORD copEncoderIndex = m_pWaitingForUpdateParams->GetCopResourceIndex();
			WORD copResourceIndexOfCascadeLinkLecturer = m_pWaitingForUpdateParams->GetCopResourceOfLecturerLinkIndex();
			BYTE cascadeIsLecturer = m_pWaitingForUpdateParams->GetIsCopLinkLecturer();

			if ((m_copEncoderConnectionId != copEncoderConnectionId) || (m_copEncoderPartyId != copEncoderPartyId))
			{
				TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Update video-out params we connect to a new encoder";
				// 1. Disconnect From RTP with the prev Encoder Prams.
				SendDisconnectFromRtp();
				// 2. Change state
				m_state = UPDATE_ENCODER;
				// 3. save the new Encoder params
				m_copEncoderConnectionId = copEncoderConnectionId;
				m_copEncoderPartyId = copEncoderPartyId;
				m_copEncoderIndex = copEncoderIndex;
				m_copResourceIndexOfCascadeLinkLecturer = copResourceIndexOfCascadeLinkLecturer;
				m_bCascadeIsLecturer = cascadeIsLecturer;
			}
			else
			{
				TRACEINTO << m_pBridgePartyCntl->GetFullName()
								<< " - Update video-out (only save) params we connect to a new encoder";
				SaveAndSendUpdatedVideoParams(m_pWaitingForUpdateParams);
				POBJDELETE(m_pWaitingForUpdateParams);
			}
		}
		else
		{
			CSegment* pMsg = new CSegment;
			*pMsg << (BYTE) statOK;

			// add IsAckParams false to prevent CSegment asserts
			BYTE IsAckParams = false;
			*pMsg << (BYTE) IsAckParams;

			m_pBridgePartyCntl->HandleEvent(pMsg, 0, PARTY_VIDEO_OUT_UPDATED);
			POBJDELETE(pMsg);
		}

		if (m_needToConnectToPcmEncoder)
		{
			DispatchEvent(CONNECT_PARTY_TO_PCM, NULL);
		}
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnVideoBridgePartyUpdateVideoParamsIDLE(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	CBridgePartyVideoParams* pBridgePartyVideoParams = new CBridgePartyVideoParams;
	pBridgePartyVideoParams->DeSerialize(NATIVE, *pParam);

	CAckParams* pAckParams = NULL;
	BOOL IsAckParams = false;
	EStat responseStatus = statOK;

	if (!(CPObject::IsValidPObjectPtr(pBridgePartyVideoParams)))
	{
		PASSERTMSG(1, "Internal Error receive invalid params");
		responseStatus = statIllegal;
	}
	else
	{
		m_videoAlg = pBridgePartyVideoParams->GetVideoAlgorithm();
		m_videoBitRate = pBridgePartyVideoParams->GetVideoBitRate();
		m_videoResolution = pBridgePartyVideoParams->GetVideoResolution(); // In COP the resolution isn't relevant only for H263
		m_videoFrameRate = pBridgePartyVideoParams->GetVidFrameRate(); // relevant only in cop conferences
		m_videoConfType = pBridgePartyVideoParams->GetVideoConfType();
		m_copEncoderConnectionId = pBridgePartyVideoParams->GetCopConnectionId();
		m_copEncoderPartyId = pBridgePartyVideoParams->GetCopPartyId();
		m_copEncoderIndex = pBridgePartyVideoParams->GetCopResourceIndex();
		m_copResourceIndexOfCascadeLinkLecturer = pBridgePartyVideoParams->GetCopResourceOfLecturerLinkIndex();
		m_bCascadeIsLecturer = pBridgePartyVideoParams->GetIsCopLinkLecturer();

		if (m_videoAlg == H264)
		{
			m_videoQcifFrameRate = eVideoFrameRateDUMMY;
			m_videoCifFrameRate = eVideoFrameRateDUMMY;
			m_video4CifFrameRate = eVideoFrameRateDUMMY;
			m_MBPS = pBridgePartyVideoParams->GetMBPS();
			m_FS = pBridgePartyVideoParams->GetFS();
			m_staticMB = pBridgePartyVideoParams->GetStaticMB();
			m_maxDPB = pBridgePartyVideoParams->GetMaxDPB();
		}
		else
		{
			m_videoQcifFrameRate = pBridgePartyVideoParams->GetVideoFrameRate(eVideoResolutionQCIF);
			m_videoCifFrameRate = pBridgePartyVideoParams->GetVideoFrameRate(eVideoResolutionCIF);
			m_video4CifFrameRate = pBridgePartyVideoParams->GetVideoFrameRate(eVideoResolution4CIF);
			m_MBPS = INVALID;
			m_FS = INVALID;
			m_staticMB = DEFAULT_STATIC_MB;
			m_maxDPB = INVALID;
			m_isH263Plus = pBridgePartyVideoParams->GetIsH263Plus();
		}

		m_sampleAspectRatio = pBridgePartyVideoParams->GetSampleAspectRatio();
	}

	CSegment* pMsg = new CSegment;
	*pMsg << (BYTE) responseStatus;

	if (responseStatus == statOK)
	{
		// Need to add Ack params to Acknowledge msg to party
		if (pBridgePartyVideoParams->GetAckParams())
		{
			IsAckParams = true;
			*pMsg << (BYTE) IsAckParams;

			pAckParams = new CAckParams(*pBridgePartyVideoParams->GetAckParams());
			pAckParams->Serialize(NATIVE, *pMsg);
		}
		else
			*pMsg << (BYTE) IsAckParams;
	}

	m_pBridgePartyCntl->HandleEvent(pMsg, 0, PARTY_VIDEO_OUT_UPDATED);

	POBJDELETE(pMsg);
	POBJDELETE(pBridgePartyVideoParams);
	POBJDELETE(pAckParams);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnVideoBridgePartyUpdateVideoParamsSLIDE(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	CBridgePartyVideoParams* pBridgePartyVideoParams = new CBridgePartyVideoParams;
	pBridgePartyVideoParams->DeSerialize(NATIVE, *pParam);

	CAckParams* pAckParams = NULL;
	BOOL IsAckParams = false;

	if (!(CPObject::IsValidPObjectPtr(pBridgePartyVideoParams)))
	{
		PASSERTMSG(1, "Internal Error receive invalid params");

		CSegment* pMsg = new CSegment;
		*pMsg << (BYTE) statIllegal;

		m_pBridgePartyCntl->HandleEvent(pMsg, 0, PARTY_VIDEO_OUT_UPDATED);

		POBJDELETE(pMsg);
		POBJDELETE(pBridgePartyVideoParams);

		return;
	}

	DWORD newVideoAlg = pBridgePartyVideoParams->GetVideoAlgorithm();
	DWORD newVideoBitRate = pBridgePartyVideoParams->GetVideoBitRate();
	eVideoFrameRate newVideoQCifFrameRate = pBridgePartyVideoParams->GetVideoFrameRate(eVideoResolutionQCIF);
	eVideoFrameRate newVideoCifFrameRate = pBridgePartyVideoParams->GetVideoFrameRate(eVideoResolutionCIF);
	eVideoFrameRate newVideo4CifFrameRate = pBridgePartyVideoParams->GetVideoFrameRate(eVideoResolution4CIF);
	eVideoResolution newVideoResolution = pBridgePartyVideoParams->GetVideoResolution();
	DWORD newMBPS = pBridgePartyVideoParams->GetMBPS();
	DWORD newFS = pBridgePartyVideoParams->GetFS();
	DWORD newSampleAspectRatio = pBridgePartyVideoParams->GetSampleAspectRatio();
	DWORD newStaticMB = pBridgePartyVideoParams->GetStaticMB();
	eVideoFrameRate videoFrameRate = pBridgePartyVideoParams->GetVidFrameRate(); // relevant only in cop conferences
	eVideoConfType videoConfType = pBridgePartyVideoParams->GetVideoConfType();
	DWORD copEncoderConnectionId = pBridgePartyVideoParams->GetCopConnectionId();
	DWORD copEncoderPartyId = pBridgePartyVideoParams->GetCopPartyId();
	DWORD newMaxDPB = pBridgePartyVideoParams->GetMaxDPB();
	WORD copEncoderIndex = pBridgePartyVideoParams->GetCopResourceIndex();
	WORD copResourceIndexOfCascadeLinkLecturer = pBridgePartyVideoParams->GetCopResourceOfLecturerLinkIndex();
	BYTE cascadeIsLecturer = pBridgePartyVideoParams->GetIsCopLinkLecturer();

	STATUS status = STATUS_OK;
	EStat responseStatus = statOK;

	if (m_videoAlg == newVideoAlg && m_videoBitRate == newVideoBitRate && m_videoQcifFrameRate == newVideoQCifFrameRate
					&& m_videoCifFrameRate == newVideoCifFrameRate && m_video4CifFrameRate == newVideo4CifFrameRate
					&& m_videoResolution == newVideoResolution && m_sampleAspectRatio == newSampleAspectRatio
					&& m_staticMB == newStaticMB && m_videoFrameRate == videoFrameRate && m_videoConfType == videoConfType
					&& m_copEncoderConnectionId == copEncoderConnectionId && m_copEncoderPartyId == copEncoderPartyId
					&& m_maxDPB == newMaxDPB && m_copEncoderIndex == copEncoderIndex
					&& m_copResourceIndexOfCascadeLinkLecturer == copResourceIndexOfCascadeLinkLecturer
					&& m_bCascadeIsLecturer == cascadeIsLecturer)
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - No real change in params";
	}
	else
	{
		// save new params
		m_videoAlg = newVideoAlg;
		m_videoBitRate = newVideoBitRate;
		m_videoResolution = newVideoResolution; // In COP the resolution isn't relevant onlt for H263
		m_videoFrameRate = videoFrameRate;
		m_videoConfType = videoConfType;
		m_copEncoderConnectionId = copEncoderConnectionId;
		m_copEncoderPartyId = copEncoderPartyId;
		m_copEncoderIndex = copEncoderIndex;
		m_copResourceIndexOfCascadeLinkLecturer = copResourceIndexOfCascadeLinkLecturer;
		m_bCascadeIsLecturer = cascadeIsLecturer;

		if (m_videoAlg == H264)
		{
			m_videoQcifFrameRate = eVideoFrameRateDUMMY;
			m_videoCifFrameRate = eVideoFrameRateDUMMY;
			m_video4CifFrameRate = eVideoFrameRateDUMMY;
			m_MBPS = newMBPS;
			m_FS = newFS;
			m_staticMB = newStaticMB;
			m_maxDPB = newMaxDPB;
		}
		else
		{
			m_videoQcifFrameRate = newVideoQCifFrameRate;
			m_videoCifFrameRate = newVideoCifFrameRate;
			m_video4CifFrameRate = newVideo4CifFrameRate;
			// m_videoResolution    = newVideoResolution;
			m_MBPS = INVALID;
			m_FS = INVALID;
			m_staticMB = DEFAULT_STATIC_MB;
			m_maxDPB = INVALID;
			m_isH263Plus = pBridgePartyVideoParams->GetIsH263Plus();
		}

		m_sampleAspectRatio = newSampleAspectRatio;

		PASSERT_AND_RETURN(!m_pHardwareInterface);

		status = ((CVideoHardwareInterface*) m_pHardwareInterface)->SendUpdateSlide(m_videoResolution, m_videoAlg, m_videoBitRate,
						m_FS, m_MBPS, m_bIsTipMode);

		if (STATUS_OK != status)
		{
			PASSERTMSG(2, "Could Not Send Show Slide with Updated Params");
			responseStatus = statIllegal;
		}
	}

	CSegment* pMsg = new CSegment;
	*pMsg << (BYTE) responseStatus;

	// Need to add Ack params to Acknowledge msg to party
	if (pBridgePartyVideoParams->GetAckParams())
	{
		IsAckParams = true;
		*pMsg << (BYTE) IsAckParams;

		pAckParams = new CAckParams(*pBridgePartyVideoParams->GetAckParams());
		pAckParams->Serialize(NATIVE, *pMsg);
	}
	else
		*pMsg << (BYTE) IsAckParams;

	m_pBridgePartyCntl->HandleEvent(pMsg, 0, PARTY_VIDEO_OUT_UPDATED);

	POBJDELETE(pMsg);
	POBJDELETE(pBridgePartyVideoParams);
	POBJDELETE(pAckParams);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnVideoBridgePartyUpdateVideoParamsSETUP(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	OnVideoBridgePartyUpdateVideoParams(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnVideoBridgePartyUpdateVideoParamsUPDATE_ENCODER(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	OnVideoBridgePartyUpdateVideoParams(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnVideoBridgePartyUpdateVideoParamsCONNECTING_PCM(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	OnVideoBridgePartyUpdateVideoParams(pParam);
	if (m_pWaitingForUpdateParams)
	{
		// save the new Encoder connection Id and party Id
		m_copEncoderConnectionId = m_pWaitingForUpdateParams->GetCopConnectionId();
		m_copEncoderPartyId = m_pWaitingForUpdateParams->GetCopPartyId();
		SaveAndSendUpdatedVideoParams(m_pWaitingForUpdateParams);
		CSegment* pMsg = new CSegment;
		*pMsg << (BYTE) statOK;
		m_pBridgePartyCntl->HandleEvent(pMsg, 0, PCM_UPDATED);
		POBJDELETE(pMsg);
		POBJDELETE(m_pWaitingForUpdateParams);
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnVideoBridgePartyUpdateVideoParamsPCM(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	OnVideoBridgePartyUpdateVideoParams(pParam);
	if (m_pWaitingForUpdateParams)
	{
		// save the new Encoder connection Id and party Id
		m_copEncoderConnectionId = m_pWaitingForUpdateParams->GetCopConnectionId();
		m_copEncoderPartyId = m_pWaitingForUpdateParams->GetCopPartyId();
		SaveAndSendUpdatedVideoParams(m_pWaitingForUpdateParams);
		CSegment* pMsg = new CSegment;
		*pMsg << (BYTE) statOK;
		m_pBridgePartyCntl->HandleEvent(pMsg, 0, PCM_UPDATED);
		POBJDELETE(pMsg);
		POBJDELETE(m_pWaitingForUpdateParams);
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnVideoBridgePartyUpdateVideoParamsDISCONNECTING_PCM(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	OnVideoBridgePartyUpdateVideoParams(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnVideoBridgePartyUpdateVideoParams(CSegment* pParam)
{
	CBridgePartyVideoParams* pBridgePartyVideoParams = new CBridgePartyVideoParams;
	pBridgePartyVideoParams->DeSerialize(NATIVE, *pParam);

	if (!(CPObject::IsValidPObjectPtr(pBridgePartyVideoParams)))
	{
		PASSERTMSG(1, "Internal Error receive invalid params");
		CSegment* pMsg = new CSegment;
		*pMsg << (BYTE) statIllegal;
		m_pBridgePartyCntl->HandleEvent(pMsg, 0, PARTY_VIDEO_OUT_UPDATED);
		POBJDELETE(pMsg);
		POBJDELETE(pBridgePartyVideoParams);
		return;
	}

	DWORD copEncoderConnectionId = pBridgePartyVideoParams->GetCopConnectionId();
	DWORD copEncoderPartyId = pBridgePartyVideoParams->GetCopPartyId();
	WORD copEncoderIndex = pBridgePartyVideoParams->GetCopResourceIndex();
	WORD copResourceIndexOfCascadeLinkLecturer = pBridgePartyVideoParams->GetCopResourceOfLecturerLinkIndex();
	BYTE cascadeIsLecturer = pBridgePartyVideoParams->GetIsCopLinkLecturer();

	if (copEncoderConnectionId != m_copEncoderConnectionId || copEncoderPartyId != m_copEncoderPartyId
					|| copEncoderIndex != m_copEncoderIndex
					|| copResourceIndexOfCascadeLinkLecturer != m_copResourceIndexOfCascadeLinkLecturer
					|| cascadeIsLecturer != m_bCascadeIsLecturer) // just if the party is connected to new encoder we should get update video out params
	{
		if (m_pWaitingForUpdateParams)
			POBJDELETE(m_pWaitingForUpdateParams);

		m_pWaitingForUpdateParams = new CBridgePartyVideoParams;
		*m_pWaitingForUpdateParams = *pBridgePartyVideoParams;
	}

	POBJDELETE(pBridgePartyVideoParams);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnPCMConnectAck(STATUS status)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", Status:" << status;

	if (status != STATUS_OK)
	{
		// Add assert to EMA in case of NACK
		AddFaultAlarm("NACK on Connect PCM encoder", m_pHardwareInterface->GetPartyRsrcId(), status);

		CSegment* pSeg = new CSegment;
		*pSeg << (BYTE) statVideoInOutResourceProblem << (BYTE) eMipOut << (BYTE) eMipStatusFail << (BYTE) eMipConnect;

		// Inform BridgePartyCntl of statVideoInOutResourceProblem -> Inorder to initiate Kill port on VideoEncoder VideoDecoder
		m_pBridgePartyCntl->HandleEvent(pSeg, 0, PCM_CONNECTED);

		POBJDELETE(pSeg);
		return;
	}
	else
	{
		m_state = PCM;

		CSegment* pMsg = new CSegment;
		*pMsg << (BYTE) statOK;
		m_pBridgePartyCntl->HandleEvent(pMsg, 0, PCM_CONNECTED);
		POBJDELETE(pMsg);

		if (m_needToDisconnectFromPcmEncoder)
			DispatchEvent(DISCONNECT_PARTY_FROM_PCM, NULL);
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnPCMDisconnectAck(STATUS status)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", Status:" << status;

	if (status != STATUS_OK)
	{
		// Add assert to EMA in case of NACK
		AddFaultAlarm("NACK on Connect PCM encoder", m_pHardwareInterface->GetPartyRsrcId(), status);

		CSegment* pSeg = new CSegment;
		*pSeg << (BYTE) statVideoInOutResourceProblem << (BYTE) eMipOut << (BYTE) eMipStatusFail << (BYTE) eMipDisconnect;

		// Inform BridgePartyCntl of statVideoInOutResourceProblem -> Inorder to initiate Kill port on VideoEncoder VideoDecoder
		m_pBridgePartyCntl->HandleEvent(pSeg, 0, PCM_CONNECTED);

		POBJDELETE(pSeg);
		return;
	}
	else
	{
		m_state = CONNECTED;

		CSegment* pMsg = new CSegment;
		*pMsg << (BYTE) statOK;
		m_pBridgePartyCntl->HandleEvent(pMsg, 0, PCM_DISCONNECTED);
		POBJDELETE(pMsg);

		if (m_pWaitingForUpdateParams)
		{
			DWORD copEncoderConnectionId = m_pWaitingForUpdateParams->GetCopConnectionId();
			DWORD copEncoderPartyId = m_pWaitingForUpdateParams->GetCopPartyId();
			WORD copEncoderIndex = m_pWaitingForUpdateParams->GetCopResourceIndex();
			WORD copResourceIndexOfCascadeLinkLecturer = m_pWaitingForUpdateParams->GetCopResourceOfLecturerLinkIndex();
			BYTE cascadeIsLecturer = m_pWaitingForUpdateParams->GetIsCopLinkLecturer();

			if ((m_copEncoderConnectionId != copEncoderConnectionId) || (m_copEncoderPartyId != copEncoderPartyId))
			{
				TRACEINTO << m_pBridgePartyCntl->GetFullName()
								<< " - Update video-out params received while we were disconnecting party from PCM -> update encoder";
				// 1. Disconnect From RTP with the prev Encoder Prams.
				SendDisconnectFromRtp();
				// 2. Change state
				m_state = UPDATE_ENCODER;
				// 3. save the new Encoder params
				m_copEncoderConnectionId = copEncoderConnectionId;
				m_copEncoderPartyId = copEncoderPartyId;
				m_copEncoderIndex = copEncoderIndex;
				m_copResourceIndexOfCascadeLinkLecturer = copResourceIndexOfCascadeLinkLecturer;
				m_bCascadeIsLecturer = cascadeIsLecturer;
			}
			else
			{
				TRACEINTO << m_pBridgePartyCntl->GetFullName()
								<< " - Update video-out params received while we were disconnecting party from PCM (cop encoder id not changed, only params changed)";
				SaveAndSendUpdatedVideoParams(m_pWaitingForUpdateParams);
				POBJDELETE(m_pWaitingForUpdateParams);
			}
		}
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::ConnectToPCMEncoder(DWORD pcmEncoderConnectionId, DWORD pcmEncoderPartyId)
{
	m_pcmEncoderConnectionId = pcmEncoderConnectionId;
	m_pcmEncoderEntityId = pcmEncoderPartyId;
	m_needToConnectToPcmEncoder = true;
	DispatchEvent(CONNECT_PARTY_TO_PCM, NULL);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnVideoBridgeConnectPartyToPcmIDLE(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Event is ignored";
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnVideoBridgeConnectPartyToPcmSLIDE(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Event is ignored";
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnVideoBridgeConnectPartyToPcmSETUP(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Event is ignored";
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnVideoBridgeConnectPartyToPcmCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	OnVideoBridgeConnectPartyToPcm(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnVideoBridgeConnectPartyToPcmDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Event is ignored";
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnVideoBridgeConnectPartyToPcmUPDATE_ENCODER(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Event is ignored";
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnVideoBridgeConnectPartyToPcmCONNECTING_PCM(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	PASSERT(1); // should not get here!!!
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnVideoBridgeConnectPartyToPcmPCM(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	PASSERT(1); // should not get here!!!
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnVideoBridgeConnectPartyToPcmDISCONNECTING_PCM(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	PASSERT(1); // should not get here!!!
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnVideoBridgeConnectPartyToPcm(CSegment* pParam)
{
	m_state = CONNECTING_PCM;
	SendDisconnectFromRtp();
	m_needToConnectToPcmEncoder = false;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::ConnectRtpToPCMEncoder()
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	PASSERT_AND_RETURN(!m_pHardwareInterface);

	ConnectionID rtpConnectionId = DUMMY_CONNECTION_ID;
	ConnectionID pcmEncoderConnectionId = m_pcmEncoderConnectionId;
	PartyRsrcID pcmEncoderRsrcPartyId = m_pcmEncoderEntityId;
	PartyRsrcID rtpRsrcPartyId = m_pHardwareInterface->GetPartyRsrcId();

	CRsrcDesc* pRsrcDesc;
	// Eitan - ISDN party has no rtp - connect MUX instead?
	if (m_pBridgePartyCntl->GetNetworkInterface() == ISDN_INTERFACE_TYPE)
		pRsrcDesc = ::GetpConfPartyRoutingTable()->GetPartyRsrcDesc(m_pHardwareInterface->GetPartyRsrcId(), eLogical_mux);
	else
		pRsrcDesc = ::GetpConfPartyRoutingTable()->GetPartyRsrcDesc(m_pHardwareInterface->GetPartyRsrcId(), eLogical_rtp);

	if (pRsrcDesc)
	{
		rtpConnectionId = pRsrcDesc->GetConnectionId();
	}
	else
	{
		DBGPASSERT(101);
	}

	((CVideoHardwareInterface*) m_pHardwareInterface)->SendConnect(rtpConnectionId, pcmEncoderConnectionId, rtpRsrcPartyId,
					pcmEncoderRsrcPartyId);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::DisconnectFromPCMEncoder()
{
	m_needToDisconnectFromPcmEncoder = true;
	DispatchEvent(DISCONNECT_PARTY_FROM_PCM, NULL);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnVideoBridgeDisconnectPartyFromPcmCONNECTING_PCM(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Event is ignored";
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnVideoBridgeDisconnectPartyFromPcm(CSegment* pParam)
{
	m_state = DISCONNECTING_PCM;
	DisconnectRtpFromPCMEncoder();
	m_needToDisconnectFromPcmEncoder = false;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::DisconnectRtpFromPCMEncoder()
{
	PASSERT_AND_RETURN(!m_pHardwareInterface);

	ConnectionID rtpConnectionId = DUMMY_CONNECTION_ID;
	ConnectionID pcmEncoderConnectionId = m_pcmEncoderConnectionId;
	PartyRsrcID pcmEncoderRsrcPartyId = m_pcmEncoderEntityId;
	PartyRsrcID rtpRsrcPartyId = m_pHardwareInterface->GetPartyRsrcId();

	CRsrcDesc* pRsrcDesc;
	// Eitan - ISDN party has no rtp - connect MUX instead?
	if (m_pBridgePartyCntl->GetNetworkInterface() == ISDN_INTERFACE_TYPE)
		pRsrcDesc = ::GetpConfPartyRoutingTable()->GetPartyRsrcDesc(m_pHardwareInterface->GetPartyRsrcId(), eLogical_mux);
	else
		pRsrcDesc = ::GetpConfPartyRoutingTable()->GetPartyRsrcDesc(m_pHardwareInterface->GetPartyRsrcId(), eLogical_rtp);

	if (pRsrcDesc)
	{
		rtpConnectionId = pRsrcDesc->GetConnectionId();
	}
	else
	{
		DBGPASSERT(101);
	}

	((CVideoHardwareInterface*) m_pHardwareInterface)->SendDisconnect(rtpConnectionId, pcmEncoderConnectionId, rtpRsrcPartyId,
					pcmEncoderRsrcPartyId);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnMplAckCONNECTING_PCM(CSegment* pParam)
{
	OPCODE AckOpcode;
	DWORD ack_seq_num;
	STATUS status;
	*pParam >> AckOpcode >> ack_seq_num >> status;

	// in this stage(4.6.1) no error handling just print assert to log
	if (status != STATUS_OK)
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", AckOpcode:"
						<< CProcessBase::GetProcess()->GetOpcodeAsString(AckOpcode) << ", Status:"
						<< CProcessBase::GetProcess()->GetStatusAsString(status).c_str();
		PASSERT(status);
	}

	switch (AckOpcode)
	{
	case TB_MSG_DISCONNECT_REQ:
	{
		m_disconnectAckReceived = true;
		ConnectRtpToPCMEncoder();
		break;
	}

	case TB_MSG_CONNECT_REQ:
	{
		m_disconnectAckReceived = false;
		OnPCMConnectAck(status);
		SendAckIndToPCM(PARTY_CONNECTED_TO_PCM_ENCODER, ack_seq_num, status);
		break;
	}

	default:
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", AckOpcode:"
						<< CProcessBase::GetProcess()->GetOpcodeAsString(AckOpcode) << " - Ignored";
		break;
	}
	} // switch
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::OnMplAckDISCONNECTING_PCM(CSegment* pParam)
{
	OPCODE AckOpcode;
	DWORD ack_seq_num;
	STATUS status;
	*pParam >> AckOpcode >> ack_seq_num >> status;

	// in this stage(4.6.1) no error handling just print assert to log
	if (status != STATUS_OK)
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", AckOpcode:"
						<< CProcessBase::GetProcess()->GetOpcodeAsString(AckOpcode) << ", Status:"
						<< CProcessBase::GetProcess()->GetStatusAsString(status).c_str();
		PASSERT(status);
	}

	switch (AckOpcode)
	{
	case TB_MSG_DISCONNECT_REQ:
	{
		m_disconnectAckReceived = true;
		SendConnectToRtp();
		break;
	}
	case TB_MSG_CONNECT_REQ:
	{
		m_disconnectAckReceived = false;
		OnPCMDisconnectAck(STATUS_OK);
		SendAckIndToPCM(PARTY_DISCONNECTED_FROM_PCM_ENCODER, ack_seq_num, status);
		break;
	}
	default:
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", AckOpcode:"
						<< CProcessBase::GetProcess()->GetOpcodeAsString(AckOpcode) << " - Ignored";
		break;
	}
	} // switch
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutCOP::SendAckIndToPCM(OPCODE AckOpcode, DWORD ack_seq_num, STATUS status)
{
	CSegment* pSeg = new CSegment;
	*pSeg << status;
	((CVideoBridgePartyCntl*) m_pBridgePartyCntl)->PCMNotification(AckOpcode, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
BYTE CBridgePartyVideoOutCOP::IsInUpdateEncoder() const
{
	BYTE isInUpdateEncoder = NO;
	if (m_state == UPDATE_ENCODER)
		isInUpdateEncoder = YES;

	return isInUpdateEncoder;
}
PBEGIN_MESSAGE_MAP(CBridgePartyVideoOutXcode)

ONEVENT(FASTUPDATE, ANYCASE, CBridgePartyVideoOutXcode::NullActionFunction)

PEND_MESSAGE_MAP(CBridgePartyVideoOutXcode,CBridgePartyVideoOut)
;

////////////////////////////////////////////////////////////////////////////
//                        CBridgePartyVideoOutXcode
////////////////////////////////////////////////////////////////////////////
CBridgePartyVideoOutXcode::CBridgePartyVideoOutXcode() :
				CBridgePartyVideoOutCOP()
{
	m_xcodeEncoderIndex = eXcodeEncoderDummy;
	m_xcodeEncoderConnectionId = 0;
	m_xcodeEncoderPartyId = 0;
}

//--------------------------------------------------------------------------
CBridgePartyVideoOutXcode::~CBridgePartyVideoOutXcode()
{
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutXcode::Create(const CBridgePartyCntl* pBridgePartyCntl, const CRsrcParams* pRsrcParams,
				const CBridgePartyMediaParams* pBridgePartyVideoOutParams)
{
	CBridgePartyVideoOut::Create(pBridgePartyCntl, pRsrcParams, pBridgePartyVideoOutParams);
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	// get updated with the connection Id and party Id of the XCode encoder this party will be connected to
	m_xcodeEncoderConnectionId = ((CBridgePartyVideoParams*) pBridgePartyVideoOutParams)->GetXCodeConnectionId();
	m_xcodeEncoderPartyId = ((CBridgePartyVideoParams*) pBridgePartyVideoOutParams)->GetXCodePartyId();
	m_xcodeEncoderIndex = (eXcodeRsrcType) (((CBridgePartyVideoParams*) pBridgePartyVideoOutParams)->GetXCodeResourceIndex());
	m_videoConfType = ((CBridgePartyVideoOutParams*) pBridgePartyVideoOutParams)->GetVideoConfType();
}

//--------------------------------------------------------------------------
WORD CBridgePartyVideoOutXcode::IsValidState(WORD state) const
{
	WORD valid_state = true;
	switch (state)
	{
	case CHANGE_LAYOUT:
	{
		valid_state = false;
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Received CHANGE_LAYOUT message in not valid state";
		PASSERT(1);
		break;
	}
	case SLIDE:
	{
		valid_state = false;
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Received SLIDE in message not valid state";
		PASSERT(1);
		break;
	}
	}
	return valid_state;
}

//--------------------------------------------------------------------------
BOOL CBridgePartyVideoOutXcode::IsConnected()
{
	return (m_state == CONNECTED);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutXcode::SendConnectToRtp()
{
	PASSERT_AND_RETURN(!m_pHardwareInterface);

	ConnectionID encoderConnectionId, rtpConnectionId;
	encoderConnectionId = rtpConnectionId = DUMMY_CONNECTION_ID;

	DWORD encoderRsrcPartyId, rtpRsrcPartyId;
	encoderRsrcPartyId = rtpRsrcPartyId = DUMMY_PARTY_ID;

	encoderConnectionId = m_xcodeEncoderConnectionId;
	encoderRsrcPartyId = m_xcodeEncoderPartyId;
	rtpRsrcPartyId = m_pHardwareInterface->GetPartyRsrcId();

	CRsrcDesc* pRsrcDesc;
	// Eitan - ISDN party has no rtp - connect MUX instead?
	if (m_pBridgePartyCntl->GetNetworkInterface() == ISDN_INTERFACE_TYPE)
		pRsrcDesc = ::GetpConfPartyRoutingTable()->GetPartyRsrcDesc(m_pHardwareInterface->GetPartyRsrcId(), eLogical_mux);
	else
		pRsrcDesc = ::GetpConfPartyRoutingTable()->GetPartyRsrcDesc(m_pHardwareInterface->GetPartyRsrcId(), eLogical_rtp);

	if (pRsrcDesc)
	{
		rtpConnectionId = pRsrcDesc->GetConnectionId();
	}
	else
	{
		DBGPASSERT(101);
	}

	m_lastReqId = ((CVideoHardwareInterface*) m_pHardwareInterface)->SendConnect(rtpConnectionId, encoderConnectionId,
					rtpRsrcPartyId, encoderRsrcPartyId);
	m_lastReq = TB_MSG_CONNECT_REQ;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutXcode::OnMplConnectAckSETUP(STATUS status)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", Status:" << status;

	if (status != STATUS_OK)
	{
		// Add assert to EMA in case of NACK
		AddFaultAlarm("NACK on Connect video encoder", m_pHardwareInterface->GetPartyRsrcId(), status);

		CSegment* pSeg = new CSegment;
		*pSeg << (BYTE) statVideoInOutResourceProblem << (BYTE) eMipOut << (BYTE) eMipStatusFail << (BYTE) eMipConnect;

		// Inform BridgePartyCntl of statVideoInOutResourceProblem -> Inorder to initiate Kill port on VideoEncoder VideoDecoder
		m_pBridgePartyCntl->HandleEvent(pSeg, 0, VIDEO_OUT_CONNECTED);

		POBJDELETE(pSeg);
		return;
	}
	else
	{
		if (m_pWaitingForUpdateParams)
		{
			TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Update VideoOut only";
			SaveAndSendUpdatedVideoParams(m_pWaitingForUpdateParams);
			POBJDELETE(m_pWaitingForUpdateParams);
		}

		m_state = CONNECTED;
		CSegment* pMsg = new CSegment;
		*pMsg << (BYTE) statOK;
		m_pBridgePartyCntl->HandleEvent(pMsg, 0, VIDEO_OUT_CONNECTED);
		POBJDELETE(pMsg);
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutXcode::OnMplOpenPortAck(STATUS status)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Event is ignored";
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutXcode::OnVideoBridgePartyDisConnectSETUP(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	OnVideoBridgePartyDisConnect(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutXcode::OnVideoBridgePartyDisConnect(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	m_state = DISCONNECTING;
	SendDisconnectFromRtp();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutXcode::SendDisconnectFromRtp()
{
	PASSERT_AND_RETURN(!m_pHardwareInterface);

	ConnectionID encoderConnectionId, rtpConnectionId;
	encoderConnectionId = rtpConnectionId = DUMMY_CONNECTION_ID;

	DWORD encoderRsrcPartyId, rtpRsrcPartyId;
	encoderRsrcPartyId = rtpRsrcPartyId = DUMMY_PARTY_ID;

	encoderConnectionId = m_xcodeEncoderConnectionId;
	encoderRsrcPartyId = m_xcodeEncoderPartyId;
	rtpRsrcPartyId = m_pHardwareInterface->GetPartyRsrcId();

	CRsrcDesc* pRsrcDesc;
	// Eitan - ISDN party has no rtp - connect MUX instead?
	if (m_pBridgePartyCntl->GetNetworkInterface() == ISDN_INTERFACE_TYPE)
		pRsrcDesc = ::GetpConfPartyRoutingTable()->GetPartyRsrcDesc(m_pHardwareInterface->GetPartyRsrcId(), eLogical_mux);
	else
		pRsrcDesc = ::GetpConfPartyRoutingTable()->GetPartyRsrcDesc(m_pHardwareInterface->GetPartyRsrcId(), eLogical_rtp);

	if (pRsrcDesc)
	{
		rtpConnectionId = pRsrcDesc->GetConnectionId();
	}
	else
	{
		DBGPASSERT(101);
	}

	m_lastReqId = ((CVideoHardwareInterface*) m_pHardwareInterface)->SendDisconnect(rtpConnectionId, encoderConnectionId,
					rtpRsrcPartyId, encoderRsrcPartyId);
	m_lastReq = TB_MSG_DISCONNECT_REQ;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutXcode::OnMplAckDISCONNECTING(CSegment* pParam)
{
	OPCODE AckOpcode;
	DWORD ack_seq_num;
	STATUS status;
	*pParam >> AckOpcode >> ack_seq_num >> status;
	switch (AckOpcode)
	{
	case TB_MSG_DISCONNECT_REQ:
	{
		OnMplDisconnectAckDISCONNECTING(status);
		break;
	}

	default:
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", AckOpcode:"
						<< CProcessBase::GetProcess()->GetOpcodeAsString(AckOpcode);
		break;
	}
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutXcode::OnMplDisconnectAckDISCONNECTING(STATUS status)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", Status:" << status;

	BYTE responseStatus = statOK;
	if (status != STATUS_OK)
	{
		// Add assert to EMA in case of NACK
		AddFaultAlarm("NACK on Disconnect video encoder", m_pHardwareInterface->GetPartyRsrcId(), status);
		responseStatus = statVideoInOutResourceProblem; // statVideoInOutResourceProblem -> Inorder to initiate Kill port on VideoEncoder VideoDecoder
	}

	m_state = IDLE;

	CSegment* pSeg = new CSegment;
	*pSeg << (BYTE) responseStatus;
	if (responseStatus == statVideoInOutResourceProblem)
		*pSeg << (BYTE) eMipOut << (BYTE) eMipStatusFail << (BYTE) eMipDisconnect;

	SetClosePortAckStatus(responseStatus);

	// Inform BridgePartyCntl
	m_pBridgePartyCntl->HandleEvent(pSeg, 0, VIDEO_OUT_DISCONNECTED);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoOutXcode::OnMplAckSETUP(CSegment* pParam)
{
	OPCODE AckOpcode;
	DWORD ack_seq_num;
	STATUS status;
	*pParam >> AckOpcode >> ack_seq_num >> status;

	switch (AckOpcode)
	{
	case TB_MSG_CONNECT_REQ:
	{
		OnMplConnectAckSETUP(status);
		break;
	}

	default:
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", AckOpcode:"
						<< CProcessBase::GetProcess()->GetOpcodeAsString(AckOpcode);
		break;
	}
	}
}

