#include "VideoBridgePartyCntl.h"
#include "TextOnScreenMngr.h"
#include "PartyApi.h"
#include "VisualEffectsParams.h"
#include "BridgePartyVideoIn.h"
#include "BridgePartyVideoOut.h"
#include "StatusesGeneral.h"
#include "VideoBridge.h"
#include "OpcodesMcmsCardMngrIvrCntl.h"
#include "OpcodesMcmsVideo.h"
#include "VideoBridgePartyInitParams.h"
#include "VideoApiDefinitions.h"
#include "ManagerApi.h"
#include "Gathering.h"
#include "IpServiceListManager.h"
#include "IntraSuppression.h"
#include "Macros.h"
#include "ConfPartyDefines.h"
#include "SIPParty.h"
#include "IVRPlayMessage.h"
#include "EnumsToStrings.h"

// ~~~~~~~~~~~~~~ Global functions ~~~~~~~~~~~~~~~~~~~~~~~~~~
extern CConfPartyRoutingTable* GetpConfPartyRoutingTable();
extern CIpServiceListManager*  GetIpServiceListMngr();


// Time-out values
#define VIDEO_BRDG_PARTY_SETUP_TOUT_VALUE        2000   // adjusted to 20 second, to adjust new mechanism that send ack after unit recovered (was 700)
#define VIDEO_BRDG_PARTY_DISCONNECT_TOUT_VALUE   2000   // VNVG-24004 set to 20 to avoid kill port - adjusted to 10 second, to adjust new mechanism that send ack after unit recovered (was 500)

#define MESSAGE_OVERLAY_TIMER                    ((WORD)206)
#define MESSAGE_OVERLAY_TIMER_OUT_VALUE          1*SECOND

#define MULTIPLE_TELEPRESENCE_CONNECTIONS_TIMER                    ((WORD)207)
#define MULTIPLE_TELEPRESENCE_CONNECTIONS_TIMER_OUT_VALUE          5*SECOND

//VNGR-26449 - unencrypted conference message
#define SECURE_MESSAGE_TIMER      ((WORD)208)
#define MESSAGE_OVERLAY_END_TIMER ((WORD)209)

#define TELEPRESENCE_INFO_UPDATE_TIMER ((WORD)210)
#define TELEPRESENCE_INFO_UPDATE_TIMEOUT  1*SECOND

PBEGIN_MESSAGE_MAP(CVideoBridgePartyCntl)
	ONEVENT(VIDCONNECT,                               IDLE,                 CVideoBridgePartyCntl::OnVideoBridgeConnectIDLE)
	ONEVENT(VIDCONNECT,                               SETUP,                CVideoBridgePartyCntl::OnVideoBridgeConnectSETUP)
	ONEVENT(VIDCONNECT,                               CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgeConnectCONNECTED)
	ONEVENT(VIDCONNECT,                               CONNECTED_STANDALONE, CVideoBridgePartyCntl::OnVideoBridgeConnectCONNECTED_STANDALONE)
	ONEVENT(VIDCONNECT,                               DISCONNECTING,        CVideoBridgePartyCntl::OnVideoBridgeConnectDISCONNECTING)

	ONEVENT(VIDDISCONNECT,                            IDLE,                 CVideoBridgePartyCntl::OnVideoBridgeDisconnectIDLE)
	ONEVENT(VIDDISCONNECT,                            SETUP,                CVideoBridgePartyCntl::OnVideoBridgeDisconnectSETUP)
	ONEVENT(VIDDISCONNECT,                            CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgeDisconnectCONNECTED)
	ONEVENT(VIDDISCONNECT,                            CONNECTED_STANDALONE, CVideoBridgePartyCntl::OnVideoBridgeDisconnectCONNECTED_STANDALONE)
	ONEVENT(VIDDISCONNECT,                            DISCONNECTING,        CVideoBridgePartyCntl::OnVideoBridgeDisconnectDISCONNECTING)

	ONEVENT(VIDEO_EXPORT,                             SETUP,                CVideoBridgePartyCntl::OnVideoBridgeExportSETUP)
	ONEVENT(VIDEO_EXPORT,                             CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgeExportCONNECTED)
	ONEVENT(VIDEO_EXPORT,                             CONNECTED_STANDALONE, CVideoBridgePartyCntl::OnVideoBridgeExportCONNECTED_STANDALONE)
	ONEVENT(VIDEO_EXPORT,                             DISCONNECTING,        CVideoBridgePartyCntl::OnVideoBridgeExportDISCONNECTING)

	ONEVENT(VIDEO_OUT_CONNECTED,                      SETUP,                CVideoBridgePartyCntl::OnVideoOutConnectedSETUP)
	ONEVENT(VIDEO_OUT_CONNECTED,                      CONNECTED,            CVideoBridgePartyCntl::OnVideoOutConnectedCONNECTED)
	ONEVENT(VIDEO_OUT_CONNECTED,                      DISCONNECTING,        CVideoBridgePartyCntl::OnVideoOutConnectedDISCONNECTING)

	ONEVENT(VIDEO_IN_CONNECTED,                       SETUP,                CVideoBridgePartyCntl::OnVideoInConnectedSETUP)
	ONEVENT(VIDEO_IN_CONNECTED,                       CONNECTED,            CVideoBridgePartyCntl::OnVideoInConnectedCONNECTED)
	ONEVENT(VIDEO_IN_CONNECTED,                       DISCONNECTING,        CVideoBridgePartyCntl::OnVideoInConnectedDISCONNECTING)

	ONEVENT(VIDEO_IN_SYNCED,                          SETUP,                CVideoBridgePartyCntl::OnVideoInSyncedSETUP)
	ONEVENT(VIDEO_IN_SYNCED,                          CONNECTED,            CVideoBridgePartyCntl::OnVideoInSyncedCONNECTED)

	ONEVENT(VIDEO_OUT_DISCONNECTED,                   SETUP,                CVideoBridgePartyCntl::OnVideoOutDisconnectedSETUP)
	ONEVENT(VIDEO_OUT_DISCONNECTED,                   DISCONNECTING,        CVideoBridgePartyCntl::OnVideoOutDisconnectedDISCONNECTING)
	ONEVENT(VIDEO_OUT_DISCONNECTED,                   CONNECTED,            CVideoBridgePartyCntl::OnVideoOutDisconnectedCONNECTED)

	ONEVENT(VIDEO_IN_DISCONNECTED,                    SETUP,                CVideoBridgePartyCntl::OnVideoInDisconnectedSETUP)
	ONEVENT(VIDEO_IN_DISCONNECTED,                    CONNECTED,            CVideoBridgePartyCntl::OnVideoInDisconnectedCONNECTED)
	ONEVENT(VIDEO_IN_DISCONNECTED,                    DISCONNECTING,        CVideoBridgePartyCntl::OnVideoInDisconnectedDISCONNECTING)

	ONEVENT(UPDATE_VIDEO_IN_PARAMS,                   SETUP,                CVideoBridgePartyCntl::OnVideoBridgeUpdateVideoInParamsSETUP)
	ONEVENT(UPDATE_VIDEO_IN_PARAMS,                   CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgeUpdateVideoInParamsCONNECTED)
	ONEVENT(UPDATE_VIDEO_IN_PARAMS,                   CONNECTED_STANDALONE, CVideoBridgePartyCntl::OnVideoBridgeUpdateVideoInParamsCONNECTED_STANDALONE)
	ONEVENT(UPDATE_VIDEO_IN_PARAMS,                   DISCONNECTING,        CVideoBridgePartyCntl::OnVideoBridgeUpdateVideoInParamsDISCONNECTING)

	ONEVENT(UPDATE_VIDEO_OUT_PARAMS,                  SETUP,                CVideoBridgePartyCntl::OnVideoBridgeUpdateVideoOutParamsSETUP)
	ONEVENT(UPDATE_VIDEO_OUT_PARAMS,                  CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgeUpdateVideoOutParamsCONNECTED)
	ONEVENT(UPDATE_VIDEO_OUT_PARAMS,                  CONNECTED_STANDALONE, CVideoBridgePartyCntl::OnVideoBridgeUpdateVideoOutParamsCONNECTED_STANDALONE)
	ONEVENT(UPDATE_VIDEO_OUT_PARAMS,                  DISCONNECTING,        CVideoBridgePartyCntl::OnVideoBridgeUpdateVideoOutParamsDISCONNECTING)

	ONEVENT(PARTY_VIDEO_IN_UPDATED,                   SETUP,                CVideoBridgePartyCntl::OnVideoInUpdatedSETUP)
	ONEVENT(PARTY_VIDEO_IN_UPDATED,                   CONNECTED,            CVideoBridgePartyCntl::OnVideoInUpdatedCONNECTED)
	ONEVENT(PARTY_VIDEO_IN_UPDATED,                   CONNECTED_STANDALONE, CVideoBridgePartyCntl::OnVideoInUpdatedCONNECTED_STANDALONE)
	ONEVENT(PARTY_VIDEO_IN_UPDATED,                   DISCONNECTING,        CVideoBridgePartyCntl::OnVideoInUpdatedDISCONNECTING)

	ONEVENT(PARTY_VIDEO_OUT_UPDATED,                  SETUP,                CVideoBridgePartyCntl::OnVideoOutUpdatedSETUP)
	ONEVENT(PARTY_VIDEO_OUT_UPDATED,                  CONNECTED,            CVideoBridgePartyCntl::OnVideoOutUpdatedCONNECTED)
	ONEVENT(PARTY_VIDEO_OUT_UPDATED,                  CONNECTED_STANDALONE, CVideoBridgePartyCntl::OnVideoOutUpdatedCONNECTED_STANDALONE)
	ONEVENT(PARTY_VIDEO_OUT_UPDATED,                  DISCONNECTING,        CVideoBridgePartyCntl::OnVideoOutUpdatedDISCONNECTING)

	ONEVENT(PARTY_IMAGE_UPDATED,                      SETUP,                CVideoBridgePartyCntl::OnPartyImageUpdatedSETUP)
	ONEVENT(PARTY_IMAGE_UPDATED,                      CONNECTED,            CVideoBridgePartyCntl::OnPartyImageUpdatedCONNECTED)
	ONEVENT(PARTY_IMAGE_UPDATED,                      CONNECTED_STANDALONE, CVideoBridgePartyCntl::OnPartyImageUpdatedCONNECTED_STANDALONE)
	ONEVENT(PARTY_IMAGE_UPDATED,                      DISCONNECTING,        CVideoBridgePartyCntl::OnPartyImageUpdatedDISCONNECTING)

	ONEVENT(ADDIMAGE,                                 SETUP,                CVideoBridgePartyCntl::OnVideoBridgeAddImageSETUP)
	ONEVENT(ADDIMAGE,                                 CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgeAddImageCONNECTED)
	ONEVENT(ADDIMAGE,                                 CONNECTED_STANDALONE, CVideoBridgePartyCntl::NullActionFunction)
	ONEVENT(ADDIMAGE,                                 DISCONNECTING,        CVideoBridgePartyCntl::NullActionFunction)

	ONEVENT(UPDATEIMAGE,                              SETUP,                CVideoBridgePartyCntl::OnVideoBridgeUpdateImage)
	ONEVENT(UPDATEIMAGE,                              CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgeUpdateImage)
	ONEVENT(UPDATEIMAGE,                              CONNECTED_STANDALONE, CVideoBridgePartyCntl::NullActionFunction)
	ONEVENT(UPDATEIMAGE,                              DISCONNECTING,        CVideoBridgePartyCntl::NullActionFunction)

	ONEVENT(FASTUPDATE,                               SETUP,                CVideoBridgePartyCntl::OnVideoBridgeFastUpdateSETUP)
	ONEVENT(FASTUPDATE,                               CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgeFastUpdateCONNECTED)
	ONEVENT(FASTUPDATE,                               CONNECTED_STANDALONE, CVideoBridgePartyCntl::OnVideoBridgeFastUpdateCONNECTED_STANDALONE)
	ONEVENT(FASTUPDATE,                               DISCONNECTING,        CVideoBridgePartyCntl::NullActionFunction)

	ONEVENT(MUTEIMAGE,                                SETUP,                CVideoBridgePartyCntl::OnVideoBridgeMuteImageSETUP)
	ONEVENT(MUTEIMAGE,                                CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgeMuteImageCONNECTED)
	ONEVENT(MUTEIMAGE,                                CONNECTED_STANDALONE, CVideoBridgePartyCntl::NullActionFunction)
	ONEVENT(MUTEIMAGE,                                DISCONNECTING,        CVideoBridgePartyCntl::NullActionFunction)

	ONEVENT(UNMUTEIMAGE,                              SETUP,                CVideoBridgePartyCntl::OnVideoBridgeUnMuteImageSETUP)
	ONEVENT(UNMUTEIMAGE,                              CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgeUnMuteImageCONNECTED)
	ONEVENT(UNMUTEIMAGE,                              CONNECTED_STANDALONE, CVideoBridgePartyCntl::NullActionFunction)
	ONEVENT(UNMUTEIMAGE,                              DISCONNECTING,        CVideoBridgePartyCntl::NullActionFunction)

	ONEVENT(SPEAKERS_CHANGED,                         SETUP,                CVideoBridgePartyCntl::OnVideoBridgeChangeSpeakersSETUP)
	ONEVENT(SPEAKERS_CHANGED,                         CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgeChangeSpeakersCONNECTED)
	ONEVENT(SPEAKERS_CHANGED,                         CONNECTED_STANDALONE, CVideoBridgePartyCntl::NullActionFunction)
	ONEVENT(SPEAKERS_CHANGED,                         DISCONNECTING,        CVideoBridgePartyCntl::NullActionFunction)

	ONEVENT(AUDIO_SPEAKER_CHANGED,                    SETUP,                CVideoBridgePartyCntl::OnVideoBridgeChangeAudioSpeakerSETUP)
	ONEVENT(AUDIO_SPEAKER_CHANGED,                    CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgeChangeAudioSpeakerCONNECTED)
	ONEVENT(AUDIO_SPEAKER_CHANGED,                    CONNECTED_STANDALONE, CVideoBridgePartyCntl::NullActionFunction)
	ONEVENT(AUDIO_SPEAKER_CHANGED,                    DISCONNECTING,        CVideoBridgePartyCntl::NullActionFunction)

	ONEVENT(DELIMAGE,                                 IDLE,                 CVideoBridgePartyCntl::NullActionFunction)
	ONEVENT(DELIMAGE,                                 SETUP,                CVideoBridgePartyCntl::OnVideoBridgeDelImageSETUP)
	ONEVENT(DELIMAGE,                                 CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgeDelImageCONNECTED)
	ONEVENT(DELIMAGE,                                 CONNECTED_STANDALONE, CVideoBridgePartyCntl::NullActionFunction)
	ONEVENT(DELIMAGE,                                 DISCONNECTING,        CVideoBridgePartyCntl::NullActionFunction)

	ONEVENT(CHANGECONFLAYOUT,                         IDLE,                 CVideoBridgePartyCntl::NullActionFunction)
	ONEVENT(CHANGECONFLAYOUT,                         SETUP,                CVideoBridgePartyCntl::OnVideoBridgeChangeConfLayoutSETUP)
	ONEVENT(CHANGECONFLAYOUT,                         CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgeChangeConfLayoutCONNECTED)
	ONEVENT(CHANGECONFLAYOUT,                         CONNECTED_STANDALONE, CVideoBridgePartyCntl::NullActionFunction)
	ONEVENT(CHANGECONFLAYOUT,                         DISCONNECTING,        CVideoBridgePartyCntl::NullActionFunction)

	ONEVENT(CHANGEPARTYLAYOUT_TPROOM_SUBLINK,         IDLE,                 CVideoBridgePartyCntl::NullActionFunction)
	ONEVENT(CHANGEPARTYLAYOUT_TPROOM_SUBLINK,         SETUP,                CVideoBridgePartyCntl::OnVideoBridgeChangeLayoutOfTPRoomSublinkSETUP)
	ONEVENT(CHANGEPARTYLAYOUT_TPROOM_SUBLINK,         CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgeChangeLayoutOfTPRoomSublinkCONNECTED)
	ONEVENT(CHANGEPARTYLAYOUT_TPROOM_SUBLINK,         CONNECTED_STANDALONE, CVideoBridgePartyCntl::OnVideoBridgeChangeLayoutOfTPRoomSublinkSTANDALONE)
	ONEVENT(CHANGEPARTYLAYOUT_TPROOM_SUBLINK,         DISCONNECTING,        CVideoBridgePartyCntl::NullActionFunction)

	ONEVENT(END_CHANGE_LAYOUT,                        ANYCASE,              CVideoBridgePartyCntl::OnVideoOutEndChangeLayout)

	ONEVENT(CHANGEPARTYLAYOUT,                        SETUP,                CVideoBridgePartyCntl::OnVideoBridgeChangePartyLayoutSETUP)
	ONEVENT(CHANGEPARTYLAYOUT,                        CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgeChangePartyLayoutCONNECTED)
	ONEVENT(CHANGEPARTYLAYOUT,                        CONNECTED_STANDALONE, CVideoBridgePartyCntl::OnVideoBridgeChangePartyLayoutCONNECTED_STANDALONE)
	ONEVENT(CHANGEPARTYLAYOUT,                        DISCONNECTING,        CVideoBridgePartyCntl::OnVideoBridgeChangePartyLayoutDISCONNECTING)

	ONEVENT(CHANGEPARTYPRIVATELAYOUT,                 SETUP,                CVideoBridgePartyCntl::OnVideoBridgeChangePartyPrivateLayoutSETUP)
	ONEVENT(CHANGEPARTYPRIVATELAYOUT,                 CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgeChangePartyPrivateLayoutCONNECTED)
	ONEVENT(CHANGEPARTYPRIVATELAYOUT,                 CONNECTED_STANDALONE, CVideoBridgePartyCntl::OnVideoBridgeChangePartyPrivateLayoutCONNECTED_STANDALONE)
	ONEVENT(CHANGEPARTYPRIVATELAYOUT,                 DISCONNECTING,        CVideoBridgePartyCntl::NullActionFunction)

	ONEVENT(SETPARTYPRIVATELAYOUTONOFF,               SETUP,                CVideoBridgePartyCntl::OnVideoBridgeSetPrivateLayoutOnOffSETUP)
	ONEVENT(SETPARTYPRIVATELAYOUTONOFF,               CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgeSetPrivateLayoutOnOffCONNECTED)
	ONEVENT(SETPARTYPRIVATELAYOUTONOFF,               CONNECTED_STANDALONE, CVideoBridgePartyCntl::OnVideoBridgeSetPrivateLayoutOnOffCONNECTED_STANDALONE)
	ONEVENT(SETPARTYPRIVATELAYOUTONOFF,               DISCONNECTING,        CVideoBridgePartyCntl::NullActionFunction)

	ONEVENT(UPDATEVISUALEFFECTS,                      SETUP,                CVideoBridgePartyCntl::OnVideoBridgeChangeVisualEffectsSETUP)
	ONEVENT(UPDATEVISUALEFFECTS,                      CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgeChangeVisualEffectsCONNECTED)
	ONEVENT(UPDATEVISUALEFFECTS,                      CONNECTED_STANDALONE, CVideoBridgePartyCntl::OnVideoBridgeChangeVisualEffectsCONNECTED_STANDALONE)
	ONEVENT(UPDATEVISUALEFFECTS,                      DISCONNECTING,        CVideoBridgePartyCntl::NullActionFunction)

	ONEVENT(IVR_SHOW_SLIDE_REQ,                       CONNECTED_STANDALONE, CVideoBridgePartyCntl::OnVideoBridgeShowSlideCONNECTED_STANDALONE)
	ONEVENT(IVR_SHOW_SLIDE_REQ,                       CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgeShowSlideCONNECTED)
	ONEVENT(IVR_STOP_SHOW_SLIDE_REQ,                  CONNECTED_STANDALONE, CVideoBridgePartyCntl::OnVideoBridgeStopShowSlideCONNECTED_STANDALONE)
	ONEVENT(IVR_STOP_SHOW_SLIDE_REQ,                  CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgeStopShowSlideCONNECTED)
	ONEVENT(IVR_JOIN_CONF_VIDEO,                      CONNECTED_STANDALONE, CVideoBridgePartyCntl::OnVideoBridgeJoinConfCONNECTED_STANDALONE)
	ONEVENT(IVR_JOIN_CONF_VIDEO,                      CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgeJoinConfCONNECTED)
	ONEVENT(IVR_JOIN_CONF_VIDEO,                      IDLE				  , CVideoBridgePartyCntl::OnVideoBridgeJoinConfIDLE)

	ONEVENT(VIDEO_GRAPHIC_OVERLAY_START_REQ,          CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgeStartPLCCONNECTED)
	ONEVENT(VIDEO_GRAPHIC_OVERLAY_STOP_REQ,           CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgeStopPLCCONNECTED)
	ONEVENT(PLC_SETPARTYPRIVATELAYOUTTYPE,            CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgePLC_SetPrivateLayoutTypeCONNECTED)
	ONEVENT(PLC_RETURNPARTYTOCONFLAYOUT,              CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgePLC_ReturnToConfLayoutCONNECTED)
	ONEVENT(PLC_FORCECELLZERO,                        CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgePLC_ForceCellCONNECTED)
	ONEVENT(PLC_CANCELALLPRIVATELAYOUTFORCES,         CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgePLC_CancelAllPrivateLayoutForcesCONNECTED)

	ONEVENT(UPDATE_PARTY_LECTURE_MODE_ROLE,           SETUP,                CVideoBridgePartyCntl::OnVideoBridgeUpdatePartyLectureModeRoleSETUP)
	ONEVENT(UPDATE_PARTY_LECTURE_MODE_ROLE,           CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgeUpdatePartyLectureModeRoleCONNECTED)
	ONEVENT(UPDATE_PARTY_LECTURE_MODE_ROLE,           CONNECTED_STANDALONE, CVideoBridgePartyCntl::OnVideoBridgeUpdatePartyLectureModeRoleCONNECTED_STANDALONE)
	ONEVENT(UPDATE_PARTY_LECTURE_MODE_ROLE,           DISCONNECTING,        CVideoBridgePartyCntl::OnVideoBridgeUpdatePartyLectureModeRoleDISCONNECTING)

	ONEVENT(DELETED_PARTY_FROM_CONF,                  SETUP,                CVideoBridgePartyCntl::OnVideoBridgeDeletePartyFromConfSETUP)
	ONEVENT(DELETED_PARTY_FROM_CONF,                  CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgeDeletePartyFromConfCONNECTED)
	ONEVENT(DELETED_PARTY_FROM_CONF,                  CONNECTED_STANDALONE, CVideoBridgePartyCntl::NullActionFunction)
	ONEVENT(DELETED_PARTY_FROM_CONF,                  DISCONNECTING,        CVideoBridgePartyCntl::NullActionFunction)
	ONEVENT(DELETED_PARTY_FROM_CONF,                  IDLE,                 CVideoBridgePartyCntl::NullActionFunction)

	ONEVENT(VIDREFRESH,                               SETUP,                CVideoBridgePartyCntl::OnVideoOutVideoRefresh)
	ONEVENT(VIDREFRESH,                               CONNECTED,            CVideoBridgePartyCntl::OnVideoOutVideoRefresh)
	ONEVENT(VIDREFRESH,                               DISCONNECTING,        CVideoBridgePartyCntl::NullActionFunction)
	ONEVENT(VIDREFRESH,                               CONNECTED_STANDALONE, CVideoBridgePartyCntl::NullActionFunction)

	ONEVENT(SEND_H239_VIDEO_CAPS,                     SETUP,                CVideoBridgePartyCntl::OnVideoInSendH239Caps)
	ONEVENT(SEND_H239_VIDEO_CAPS,                     CONNECTED,            CVideoBridgePartyCntl::OnVideoInSendH239Caps)
	ONEVENT(SEND_H239_VIDEO_CAPS,                     DISCONNECTING,        CVideoBridgePartyCntl::NullActionFunction)

	ONEVENT(VIDEO_DECODER_SYNC,                       SETUP,                CVideoBridgePartyCntl::OnVideoInVideoDecoderSyncChanged)
	ONEVENT(VIDEO_DECODER_SYNC,                       CONNECTED,            CVideoBridgePartyCntl::OnVideoInVideoDecoderSyncChanged)
	ONEVENT(VIDEO_DECODER_SYNC,                       DISCONNECTING,        CVideoBridgePartyCntl::NullActionFunction)

	ONEVENT(PARTYLAYOUTCHANGED,                       SETUP,                CVideoBridgePartyCntl::OnVideoOutPartyLayoutChanged)
	ONEVENT(PARTYLAYOUTCHANGED,                       CONNECTED,            CVideoBridgePartyCntl::OnVideoOutPartyLayoutChanged)
	ONEVENT(PARTYLAYOUTCHANGED,                       DISCONNECTING,        CVideoBridgePartyCntl::NullActionFunction)

	ONEVENT(PRIVATELAYOUT_ONOFF_CHANGED,              SETUP,                CVideoBridgePartyCntl::OnVideoOutPrivateLayoutOnOffChanged)
	ONEVENT(PRIVATELAYOUT_ONOFF_CHANGED,              CONNECTED,            CVideoBridgePartyCntl::OnVideoOutPrivateLayoutOnOffChanged)
	ONEVENT(PRIVATELAYOUT_ONOFF_CHANGED,              DISCONNECTING,        CVideoBridgePartyCntl::NullActionFunction)

	ONEVENT(VIDEO_BRDG_PARTY_SETUP_TOUT,              SETUP,                CVideoBridgePartyCntl::OnTimerPartySetupSETUP)
	ONEVENT(VIDEO_BRDG_PARTY_SETUP_TOUT,              CONNECTED,            CVideoBridgePartyCntl::OnTimerPartySetupCONNECTED)

	ONEVENT(VIDEO_BRDG_PARTY_DISCONNECT_TOUT,         SETUP,                CVideoBridgePartyCntl::OnTimerPartyDisconnectSETUP)
	ONEVENT(VIDEO_BRDG_PARTY_DISCONNECT_TOUT,         CONNECTED,            CVideoBridgePartyCntl::OnTimerPartyDisconnectCONNECTED)
	ONEVENT(VIDEO_BRDG_PARTY_DISCONNECT_TOUT,         DISCONNECTING,        CVideoBridgePartyCntl::OnTimerPartyDisconnectDISCONNECTING)

	ONEVENT(SET_SITE_AND_VISUAL_NAME,                 IDLE,                 CVideoBridgePartyCntl::OnSetSiteNameConnected)
	ONEVENT(SET_SITE_AND_VISUAL_NAME,                 SETUP,                CVideoBridgePartyCntl::OnSetSiteNameConnected)
	ONEVENT(SET_SITE_AND_VISUAL_NAME,                 CONNECTED,            CVideoBridgePartyCntl::OnSetSiteNameConnected)
	ONEVENT(SET_SITE_AND_VISUAL_NAME,                 CONNECTED_STANDALONE, CVideoBridgePartyCntl::OnSetSiteNameConnected)
	ONEVENT(SET_SITE_AND_VISUAL_NAME,                 DISCONNECTING,        CVideoBridgePartyCntl::OnSetSiteNameDISCONNECTING)

	ONEVENT(UPDATE_DECODER_DETECTED_MODE,             SETUP,                CVideoBridgePartyCntl::NullActionFunction)
	ONEVENT(UPDATE_DECODER_DETECTED_MODE,             CONNECTED,            CVideoBridgePartyCntl::OnVideoInUpdateDecoderDetectedModeCONNECTED)
	ONEVENT(UPDATE_DECODER_DETECTED_MODE,             CONNECTED_STANDALONE, CVideoBridgePartyCntl::NullActionFunction)
	ONEVENT(UPDATE_DECODER_DETECTED_MODE,             DISCONNECTING,        CVideoBridgePartyCntl::NullActionFunction)

	ONEVENT(DISPLAY_TEXT_ON_SCREEN,                   SETUP,                CVideoBridgePartyCntl::NullActionFunction)
	ONEVENT(DISPLAY_TEXT_ON_SCREEN,                   CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgeDisplayTextOnScreenCONNECTED)
	ONEVENT(DISPLAY_TEXT_ON_SCREEN,                   CONNECTED_STANDALONE, CVideoBridgePartyCntl::NullActionFunction)
	ONEVENT(DISPLAY_TEXT_ON_SCREEN,                   DISCONNECTING,        CVideoBridgePartyCntl::NullActionFunction)

	ONEVENT(UPDATE_VIDEO_CLARITY,                     IDLE,                 CVideoBridgePartyCntl::OnVideoBridgeUpdateVideoClarityIDLE)
	ONEVENT(UPDATE_VIDEO_CLARITY,                     SETUP,                CVideoBridgePartyCntl::OnVideoBridgeUpdateVideoClaritySETUP)
	ONEVENT(UPDATE_VIDEO_CLARITY,                     CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgeUpdateVideoClarityCONNECTED)
	ONEVENT(UPDATE_VIDEO_CLARITY,                     CONNECTED_STANDALONE, CVideoBridgePartyCntl::OnVideoBridgeUpdateVideoClarityCONNECTED_STANDALONE)
	ONEVENT(UPDATE_VIDEO_CLARITY,                     DISCONNECTING,        CVideoBridgePartyCntl::OnVideoBridgeUpdateVideoClarityDISCONNECTING)

	ONEVENT(UPDATE_AUTO_BRIGHTNESS,                   IDLE,                 CVideoBridgePartyCntl::OnVideoBridgeUpdateAutoBrightnessIDLE)
	ONEVENT(UPDATE_AUTO_BRIGHTNESS,                   SETUP,                CVideoBridgePartyCntl::OnVideoBridgeUpdateAutoBrightnessSETUP)
	ONEVENT(UPDATE_AUTO_BRIGHTNESS,                   CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgeUpdateAutoBrightnessCONNECTED)
	ONEVENT(UPDATE_AUTO_BRIGHTNESS,                   CONNECTED_STANDALONE, CVideoBridgePartyCntl::OnVideoBridgeUpdateAutoBrightnessCONNECTED_STANDALONE)
	ONEVENT(UPDATE_AUTO_BRIGHTNESS,                   DISCONNECTING,        CVideoBridgePartyCntl::OnVideoBridgeUpdateAutoBrightnessDISCONNECTING)

	ONEVENT(CHANGE_SPEAKER_NOTATION_PCM_FECC,         IDLE,                 CVideoBridgePartyCntl::OnVideoBridgeChangeSpeakerNotationForPcmFeccIDLE)
	ONEVENT(CHANGE_SPEAKER_NOTATION_PCM_FECC,         SETUP,                CVideoBridgePartyCntl::OnVideoBridgeChangeSpeakerNotationForPcmFeccSETUP)
	ONEVENT(CHANGE_SPEAKER_NOTATION_PCM_FECC,         CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgeChangeSpeakerNotationForPcmFeccCONNECTED)
	ONEVENT(CHANGE_SPEAKER_NOTATION_PCM_FECC,         CONNECTED_STANDALONE, CVideoBridgePartyCntl::OnVideoBridgeChangeSpeakerNotationForPcmFeccCONNECTED_STANDALONE)
	ONEVENT(CHANGE_SPEAKER_NOTATION_PCM_FECC,         DISCONNECTING,        CVideoBridgePartyCntl::OnVideoBridgeChangeSpeakerNotationForPcmFeccDISCONNECTING)

	ONEVENT(VIDEO_BRDG_COP_PARTY_INTRA_SUPPRESS_TOUT, SETUP,                CVideoBridgePartyCntl::OnTimerPartyIntraSuppressed)
	ONEVENT(VIDEO_BRDG_COP_PARTY_INTRA_SUPPRESS_TOUT, CONNECTED,            CVideoBridgePartyCntl::OnTimerPartyIntraSuppressed)
	ONEVENT(VIDEO_BRDG_COP_PARTY_INTRA_SUPPRESS_TOUT, CONNECTED_STANDALONE, CVideoBridgePartyCntl::OnTimerPartyIntraSuppressed)
	ONEVENT(VIDEO_BRDG_COP_PARTY_INTRA_SUPPRESS_TOUT, DISCONNECTING,        CVideoBridgePartyCntl::OnTimerPartyIntraSuppressed)

	ONEVENT(VIDEO_BRDG_COP_ENCODER_IGNORE_INTRA_TOUT, IDLE,                 CVideoBridgePartyCntl::NullActionFunction)
	ONEVENT(VIDEO_BRDG_COP_ENCODER_IGNORE_INTRA_TOUT, SETUP,                CVideoBridgePartyCntl::OnTimerIgnoreIntraSETUP)
	ONEVENT(VIDEO_BRDG_COP_ENCODER_IGNORE_INTRA_TOUT, CONNECTED,            CVideoBridgePartyCntl::OnTimerIgnoreIntraCONNECTED)
	ONEVENT(VIDEO_BRDG_COP_ENCODER_IGNORE_INTRA_TOUT, DISCONNECTING,        CVideoBridgePartyCntl::NullActionFunction)

	ONEVENT(INDICATION_ICONS_CHANGE,                  ANYCASE,              CVideoBridgePartyCntl::NullActionFunction)
	ONEVENT(INDICATION_ICONS_CHANGE,                  CONNECTED_STANDALONE, CVideoBridgePartyCntl::OnVideoBridgeIndicationIconsChangeCONNECTED)
	ONEVENT(INDICATION_ICONS_CHANGE,                  CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgeIndicationIconsChangeCONNECTED)

	ONEVENT(UPDATEONIMAGESVCTOAVC,                    SETUP,                CVideoBridgePartyCntl::OnVideoBridgeUpdateOnImageSvcToAvc)
	ONEVENT(UPDATEONIMAGESVCTOAVC,                    CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgeUpdateOnImageSvcToAvc)
	ONEVENT(UPDATEONIMAGESVCTOAVC,                    CONNECTED_STANDALONE, CVideoBridgePartyCntl::NullActionFunction)
	ONEVENT(UPDATEONIMAGESVCTOAVC,                    DISCONNECTING,        CVideoBridgePartyCntl::NullActionFunction)

	ONEVENT(MESSAGE_OVERLAY_TIMER,                    CONNECTED,            CVideoBridgePartyCntl::OnTimerMessageOverlayCONNECTED)
	ONEVENT(MESSAGE_OVERLAY_TIMER,                    DISCONNECTING,        CVideoBridgePartyCntl::NullActionFunction)
	ONEVENT(MESSAGE_OVERLAY_TIMER,                    CONNECTED_STANDALONE, CVideoBridgePartyCntl::NullActionFunction)

	ONEVENT(MULTIPLE_TELEPRESENCE_CONNECTIONS_TIMER,  CONNECTED,            CVideoBridgePartyCntl::OnTimerMultipleTPConnectionsCONNECTED)
	ONEVENT(MULTIPLE_TELEPRESENCE_CONNECTIONS_TIMER,  DISCONNECTING,        CVideoBridgePartyCntl::NullActionFunction)

	ONEVENT(TELEPRESENCE_INFO_UPDATE_TIMER,           CONNECTED,            CVideoBridgePartyCntl::OnTimerTelepresenceInfoUpdateCONNECTED)
	ONEVENT(TELEPRESENCE_INFO_UPDATE_TIMER,           CONNECTED_STANDALONE, CVideoBridgePartyCntl::OnTimerTelepresenceInfoUpdateCONNECTED)
	ONEVENT(TELEPRESENCE_INFO_UPDATE_TIMER,           DISCONNECTING,        CVideoBridgePartyCntl::NullActionFunction)

	ONEVENT(SECURE_MESSAGE_TIMER,                     CONNECTED,            CVideoBridgePartyCntl::OnTimerSecureMessageCONNECTED)
	ONEVENT(SECURE_MESSAGE_TIMER,                     CONNECTED_STANDALONE, CVideoBridgePartyCntl::NullActionFunction)
	ONEVENT(SECURE_MESSAGE_TIMER,                     DISCONNECTING,        CVideoBridgePartyCntl::NullActionFunction)

	ONEVENT(MESSAGE_OVERLAY_END_TIMER,                CONNECTED,            CVideoBridgePartyCntl::OnTimerMessageOverlayEndCONNECTED)
	ONEVENT(MESSAGE_OVERLAY_END_TIMER,                CONNECTED_STANDALONE, CVideoBridgePartyCntl::OnTimerMessageOverlayEndCONNECTED)
	ONEVENT(MESSAGE_OVERLAY_END_TIMER,                DISCONNECTING,        CVideoBridgePartyCntl::NullActionFunction)

	ONEVENT(UPGRADE_TO_MIX_AVC_SVC,          	      CONNECTED,            CVideoBridgePartyCntl::OnPartyUpgradeAvcToSvcTranslatorCONNECTED)
	ONEVENT(UPGRADE_TO_MIX_AVC_SVC,              	  CONNECTED_STANDALONE, CVideoBridgePartyCntl::OnPartyUpgradeAvcToSvcTranslatorCONNECTED_STANDALONE)
	ONEVENT(UPGRADE_TO_MIX_AVC_SVC,              	  SETUP,				CVideoBridgePartyCntl::OnPartyUpgradeAvcToSvcTranslatorSETUP)
	ONEVENT(UPGRADE_TO_MIX_AVC_SVC,              	  DISCONNECTING,        CVideoBridgePartyCntl::OnPartyUpgradeAvcToSvcTranslatorDISCONNECTING)

	ONEVENT(UPDATE_LAYOUT_HANDLER_TYPE,                 IDLE,                 CVideoBridgePartyCntl::NullActionFunction)
	ONEVENT(UPDATE_LAYOUT_HANDLER_TYPE,                 SETUP,                CVideoBridgePartyCntl::OnVideoBridgeUpdateLayoutHandlerTypeSETUP)
	ONEVENT(UPDATE_LAYOUT_HANDLER_TYPE,                 CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgeUpdateLayoutHandlerTypeCONNECTED)
	ONEVENT(UPDATE_LAYOUT_HANDLER_TYPE,                 CONNECTED_STANDALONE, CVideoBridgePartyCntl::OnVideoBridgeUpdateLayoutHandlerTypeCONNECTED_STANDALONE)
	ONEVENT(UPDATE_LAYOUT_HANDLER_TYPE,                 DISCONNECTING,        CVideoBridgePartyCntl::NullActionFunction)

	ONEVENT(PARTY_RESUMING_FROM_HOLD,                     CONNECTED,            CVideoBridgePartyCntl::OnPartyResumeFromHoldCONNECTED)
	ONEVENT(PARTY_RESUMING_FROM_HOLD,                     ANYCASE,            CVideoBridgePartyCntl::OnPartyResumeFromHoldANYCASE)

	ONEVENT(SET_TELEPRESENCE_LAYOUT_MODE,         IDLE,                 CVideoBridgePartyCntl::OnVideoBridgeUpdateTelepresenceLayoutModeIDLE)
	ONEVENT(SET_TELEPRESENCE_LAYOUT_MODE,         SETUP,                CVideoBridgePartyCntl::OnVideoBridgeUpdateTelepresenceLayoutModeSETUP)
	ONEVENT(SET_TELEPRESENCE_LAYOUT_MODE,         CONNECTED,            CVideoBridgePartyCntl::OnVideoBridgeUpdateTelepresenceLayoutModeCONNECTED)
	ONEVENT(SET_TELEPRESENCE_LAYOUT_MODE,         CONNECTED_STANDALONE, CVideoBridgePartyCntl::OnVideoBridgeUpdateTelepresenceLayoutModeCONNECTED_STANDALONE)
	ONEVENT(SET_TELEPRESENCE_LAYOUT_MODE,         DISCONNECTING,        CVideoBridgePartyCntl::NullActionFunction)

PEND_MESSAGE_MAP(CVideoBridgePartyCntl, CBridgePartyCntl);


////////////////////////////////////////////////////////////////////////////
//                        CVideoBridgePartyCntl
////////////////////////////////////////////////////////////////////////////
CVideoBridgePartyCntl::CVideoBridgePartyCntl()
{
	m_pPartySouceInCellZero               = NULL;
	m_pUpdatePartyInitParams              = NULL;
	m_resync                              = 0;
	m_bIsAfterMove                        = false;
	m_bIsPcmOvrlyMsg                      = false;
	m_bVideoInSyncedReadyForGathering     = false;
	m_bVideoOutConnectedReadyForGathering = false;
	m_IsPartyNoiseSuppressed              = false;
	m_PartyIntraRequestsTime              = new DWORD_VECTOR;
	m_isIntraForLinksSuppressed           = false;
	m_isIntraSupressionEnabled            = true;
	m_isPermanentSecureMessageForParty    = false; // VNGR-26449 - unencrypted conference message
	m_numOfUnencrypted                    = 0;
	m_isMessageOverlayPermanent           = false;
	m_messageOverlayText                  = "";
	m_MS_masterPartyRsrcID                =(PartyRsrcID)0;
	m_MSaudioLocalMsi                     = DUMMY_DOMINANT_SPEAKER_MSI;
	m_MsAvMcuIndex                        = 0;
	memset(m_ITPSiteName, 0, H243_NAME_LEN);
	m_bIsResumingIVR                      = 0;
	m_telepresenceLayoutMode = eTelePresenceLayoutManual;
}

//--------------------------------------------------------------------------
CVideoBridgePartyCntl::~CVideoBridgePartyCntl()
{
	PDELETE(m_PartyIntraRequestsTime);
}

//--------------------------------------------------------------------------
void* CVideoBridgePartyCntl::GetMessageMap()
{
	return (void*)m_msgEntries;
}

//--------------------------------------------------------------------------
CVideoBridgePartyCntl::CVideoBridgePartyCntl(const CVideoBridgePartyCntl& rOtherBridgePartyCntl)
	                    :CBridgePartyCntl(rOtherBridgePartyCntl)
{
	m_pPartySouceInCellZero               = rOtherBridgePartyCntl.m_pPartySouceInCellZero;
	m_bVideoInSyncedReadyForGathering     = rOtherBridgePartyCntl.m_bVideoInSyncedReadyForGathering;
	m_bVideoOutConnectedReadyForGathering = rOtherBridgePartyCntl.m_bVideoOutConnectedReadyForGathering;
	m_IsPartyNoiseSuppressed              = rOtherBridgePartyCntl.m_IsPartyNoiseSuppressed;
	m_PartyIntraRequestsTime              = new DWORD_VECTOR;

	DWORD_VECTOR::iterator itr = rOtherBridgePartyCntl.m_PartyIntraRequestsTime->begin();
	while (itr != rOtherBridgePartyCntl.m_PartyIntraRequestsTime->end())
	{
		DWORD currentTime = *itr;
		m_PartyIntraRequestsTime->push_back(currentTime);
		itr++;
	}

	m_isIntraForLinksSuppressed        = rOtherBridgePartyCntl.m_isIntraForLinksSuppressed;
	m_isIntraSupressionEnabled         = rOtherBridgePartyCntl.m_isIntraSupressionEnabled;

	// VNGR-26449 - unencrypted conference message
	m_isPermanentSecureMessageForParty = rOtherBridgePartyCntl.m_isPermanentSecureMessageForParty;
	m_numOfUnencrypted                 = rOtherBridgePartyCntl.m_numOfUnencrypted;
	m_isMessageOverlayPermanent        = rOtherBridgePartyCntl.m_isMessageOverlayPermanent;
	m_messageOverlayText               = rOtherBridgePartyCntl.m_messageOverlayText;
	m_MS_masterPartyRsrcID             = rOtherBridgePartyCntl.m_MS_masterPartyRsrcID;
	m_MSaudioLocalMsi                  = rOtherBridgePartyCntl.m_MSaudioLocalMsi;
	m_MsAvMcuIndex                     = rOtherBridgePartyCntl.m_MsAvMcuIndex;
	m_telepresenceLayoutMode 		   = rOtherBridgePartyCntl.m_telepresenceLayoutMode;
}

//--------------------------------------------------------------------------
CVideoBridgePartyCntl& CVideoBridgePartyCntl::operator=(const CVideoBridgePartyCntl& rOtherBridgePartyCntl)
{
	if (&rOtherBridgePartyCntl == this)
		return *this;

	m_pPartySouceInCellZero               = rOtherBridgePartyCntl.m_pPartySouceInCellZero;
	m_bVideoInSyncedReadyForGathering     = rOtherBridgePartyCntl.m_bVideoInSyncedReadyForGathering;
	m_bVideoOutConnectedReadyForGathering = rOtherBridgePartyCntl.m_bVideoOutConnectedReadyForGathering;
	m_IsPartyNoiseSuppressed              = rOtherBridgePartyCntl.m_IsPartyNoiseSuppressed;

	// VNGR-26449 - unencrypted conference message
	m_isPermanentSecureMessageForParty    = rOtherBridgePartyCntl.m_isPermanentSecureMessageForParty;
	m_numOfUnencrypted                    = rOtherBridgePartyCntl.m_numOfUnencrypted;
	m_isMessageOverlayPermanent           = rOtherBridgePartyCntl.m_isMessageOverlayPermanent;
	m_messageOverlayText                  = rOtherBridgePartyCntl.m_messageOverlayText;

	if (rOtherBridgePartyCntl.m_PartyIntraRequestsTime)
	{
		PDELETE(m_PartyIntraRequestsTime);
		m_PartyIntraRequestsTime = new DWORD_VECTOR;
		DWORD_VECTOR::iterator itr = rOtherBridgePartyCntl.m_PartyIntraRequestsTime->begin();
		while (itr != rOtherBridgePartyCntl.m_PartyIntraRequestsTime->end())
		{
			DWORD currentTime = *itr;
			m_PartyIntraRequestsTime->push_back(currentTime);
			itr++;
		}
	}

	m_isIntraSupressionEnabled = rOtherBridgePartyCntl.m_isIntraSupressionEnabled;
	m_MS_masterPartyRsrcID        = rOtherBridgePartyCntl.m_MS_masterPartyRsrcID;
	m_MSaudioLocalMsi = rOtherBridgePartyCntl.m_MSaudioLocalMsi;
	m_MsAvMcuIndex = rOtherBridgePartyCntl.m_MsAvMcuIndex;
	m_telepresenceLayoutMode = rOtherBridgePartyCntl.m_telepresenceLayoutMode;

	(CBridgePartyCntl&)(*this) = (CBridgePartyCntl&)rOtherBridgePartyCntl;
	return *this;
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::Create(const CVideoBridgePartyInitParams* pVideoBridgePartyInitParams)
{
	PASSERT_AND_RETURN(!pVideoBridgePartyInitParams);
	PASSERT_AND_RETURN(!pVideoBridgePartyInitParams->GetMediaInParams() && !pVideoBridgePartyInitParams->GetMediaOutParams());

	CConfPartyRoutingTable* pRoutingTable = ::GetpConfPartyRoutingTable();
	PASSERT_AND_RETURN(!pRoutingTable);

	// Create base params
	CBridgePartyCntl::Create(pVideoBridgePartyInitParams);
	SetITPSiteName(pVideoBridgePartyInitParams->GetSiteName());
	m_MS_masterPartyRsrcID = pVideoBridgePartyInitParams->GetMsMasterPartyRsrcId();

	m_MSaudioLocalMsi = pVideoBridgePartyInitParams->GetMsAudioLocalMsi();
	m_MsAvMcuIndex  = pVideoBridgePartyInitParams->GetMsAvMcuIndex();

	m_telepresenceLayoutMode = GetPartyTelepresenceLayoutModeConfiguration();
	// TELEPRESENCE_LAYOUTS
	if(m_telepresenceLayoutMode <= eTelePresenceLayoutCpParticipantsPriority)
		TRACEINTO << "TELEPRESENCE_LAYOUTS_DEBUG m_telepresenceLayoutMode = " << TelePresenceLayoutModeToString(m_telepresenceLayoutMode);
	else
		PASSERT(m_telepresenceLayoutMode);

	m_pUpdatePartyInitParams = NULL;
	PTRACE2INT(eLevelInfoNormal,"CVideoBridgePartyCntl::Create m_MS_masterPartyRsrcID ",m_MS_masterPartyRsrcID);
	PTRACE2INT(eLevelInfoNormal,"CVideoBridgePartyCntl::Create m_MsAvMcuIndex ",m_MsAvMcuIndex);
	CRsrcParams rsrcParams(DUMMY_CONNECTION_ID, m_partyRsrcID, m_confRsrcID);

	CBridgePartyVideoInParams* videoInParams =
		(CBridgePartyVideoInParams*)pVideoBridgePartyInitParams->GetMediaInParams();

	if (videoInParams)
	{
		NewPartyIn();
		SetTelepresenceInfo(videoInParams->GetTelePresenceEPInfo());
		std::auto_ptr<CTaskApi> pTaskApiVideoIn(new CTaskApi(*m_pConfApi));
		pTaskApiVideoIn->CreateOnlyApi(m_pConfApi->GetRcvMbx(), m_pBridgePartyIn);
		CRsrcDesc* pRsrcDesc = pRoutingTable->AddStateMachinePointerToRoutingTbl(m_partyRsrcID,
			eLogical_video_decoder, pTaskApiVideoIn.get());

		if (!pRsrcDesc)   // Entry not found in Routing Table
		{
			POBJDELETE(m_pBridgePartyIn);
			PASSERT(1);
		}
		else
		{
			rsrcParams.SetRsrcDesc(*pRsrcDesc);
			((CBridgePartyVideoIn*)(m_pBridgePartyIn))->Create(this, &rsrcParams, videoInParams);
			//Update DB when connecting muted party
			CheckIsMutedVideoInAndUpdateDB();
		}
	}

	CBridgePartyVideoOutParams* videoOutParams =
		(CBridgePartyVideoOutParams*)pVideoBridgePartyInitParams->GetMediaOutParams();

	if (videoOutParams)
	{
		NewPartyOut();

		if (!videoInParams)
			SetTelepresenceInfo(videoOutParams->GetTelePresenceEPInfo());

		std::auto_ptr<CTaskApi> pTaskApiVideoOut(new CTaskApi(*m_pConfApi));
		pTaskApiVideoOut->CreateOnlyApi(m_pConfApi->GetRcvMbx(), m_pBridgePartyOut);
		CRsrcDesc* pRsrcDesc = pRoutingTable->AddStateMachinePointerToRoutingTbl(m_partyRsrcID,
			eLogical_video_encoder, pTaskApiVideoOut.get());

		if (!pRsrcDesc)   // Entry not found in Routing Table
		{
			POBJDELETE(m_pBridgePartyOut);
			PASSERT(1);
		}
		else
		{
			rsrcParams.SetRsrcDesc(*pRsrcDesc);
			((CBridgePartyVideoOut*)(m_pBridgePartyOut))->Create(this, &rsrcParams, videoOutParams);
			((CBridgePartyVideoOut*)(m_pBridgePartyOut))->SetSiteNameInfo(pVideoBridgePartyInitParams->GetSiteNameInfo());
		}
	}

	TRACEINTO << " ITP_PARTY_IDENT" << " DEBUG 00 , " << m_partyConfName;

	if (IsTelepresenceLayoutsManagedInternally() &&
		(1 == m_telepresenceInfo.GetWaitForUpdate()) &&
		(eTelePresencePartyNone == m_telepresenceInfo.GetEPtype()))
	{
		TRACEINTO << " StartTimer(TELEPRESENCE_INFO_UPDATE_TIMER,TELEPRESENCE_INFO_UPDATE_TIMEOUT)";
		StartTimer(TELEPRESENCE_INFO_UPDATE_TIMER,TELEPRESENCE_INFO_UPDATE_TIMEOUT);
	}
}

//--------------------------------------------------------------------------
bool CVideoBridgePartyCntl::IsTelepresenceLayoutsManagedInternally()
{
  bool retVal = false;
  CCommConf* pCommConf = (CCommConf*)(m_pBridge->GetConf())->GetCommConf();
  if(NULL != pCommConf && pCommConf->GetManageTelepresenceLayoutInternaly()){
    retVal = true;
  }
  return retVal;
  //m_bManageTelepresenceLayoutsInternally = pCommConf->GetManageTelepresenceLayoutInternaly();
  //CCommConf* pCommConf = (CCommConf*)(m_pBridge->GetConf())->GetCommConf();
}
//--------------------------------------------------------------------------
void  CVideoBridgePartyCntl::OnTimerTelepresenceInfoUpdateCONNECTED(CSegment* pParam)
{
  TRACEINTO << " ITP_PARTY_IDENT" << " DEBUG 01* " << "PartyId:" << m_partyRsrcID;

  ///m_telepresenceInfo.SetWaitForUpdate(FALSE);
  ///CTelepresenseEPInfo* pTelepresenseEPInfo = new CTelepresenseEPInfo(m_telepresenceInfo);

  // things to do when reaching timeout for pending update TelepresenseEPInfo
  DWORD roomId = m_telepresenceInfo.GetRoomID();
  ((CVideoBridgeCP*)m_pBridge)->OnEndWaitingForTelepresenseEPInfo(m_partyRsrcID, roomId); ///UpdateVidBrdgTelepresenseEPInfo(m_partyRsrcID,pTelepresenseEPInfo);

  ///POBJDELETE(pTelepresenseEPInfo);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::NewPartyOut()
{
	PDELETE(m_pBridgePartyOut);
	m_pBridgePartyOut = new CBridgePartyVideoOut();
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::NewPartyIn()
{
	PDELETE(m_pBridgePartyIn);
	m_pBridgePartyIn = new CBridgePartyVideoIn();
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::Update(const CVideoBridgePartyInitParams* pVideoBridgePartyInitParams)
{
	PASSERT_AND_RETURN(!pVideoBridgePartyInitParams);
	PASSERT_AND_RETURN(!pVideoBridgePartyInitParams->GetMediaInParams() && !pVideoBridgePartyInitParams->GetMediaOutParams());

	CConfPartyRoutingTable* pRoutingTable = ::GetpConfPartyRoutingTable();
	PASSERT_AND_RETURN(!pRoutingTable);

	CRsrcParams rsrcParams(DUMMY_CONNECTION_ID, m_partyRsrcID, m_confRsrcID);

	CBridgePartyVideoInParams* videoInParams = (CBridgePartyVideoInParams*)pVideoBridgePartyInitParams->GetMediaInParams();
	if (videoInParams)
	{
		if (!m_pBridgePartyIn)
		{
			NewPartyIn();

			std::auto_ptr<CTaskApi> pTaskApiVideoIn(new CTaskApi(*m_pConfApi));
			pTaskApiVideoIn->CreateOnlyApi(m_pConfApi->GetRcvMbx(), m_pBridgePartyIn);
			CRsrcDesc* pRsrcDesc = pRoutingTable->AddStateMachinePointerToRoutingTbl(m_partyRsrcID, eLogical_video_decoder, pTaskApiVideoIn.get());
			if (!pRsrcDesc)   // Entry not found in Routing Table
			{
				POBJDELETE(m_pBridgePartyIn);
				PASSERT(1);
			}
			else
			{
				rsrcParams.SetRsrcDesc(*pRsrcDesc);
				((CBridgePartyVideoIn*)(m_pBridgePartyIn))->Create(this, &rsrcParams, videoInParams);
				//Update DB when connecting muted party
				CheckIsMutedVideoInAndUpdateDB();
			}
		}

		// If we are in disconnecting state we need to save the In Init params
		if (m_state == DISCONNECTING || (m_state == CONNECTED && m_pBridgePartyIn && m_pBridgePartyIn->GetState() == CBridgePartyVideoIn::DISCONNECTING))
		{
			if (m_pUpdatePartyInitParams != NULL)
			{
				if (m_pUpdatePartyInitParams->GetMediaInParams() != NULL)
					m_pUpdatePartyInitParams->InitiateMediaInParams((CBridgePartyVideoInParams*)pVideoBridgePartyInitParams->GetMediaInParams());
			}
			else
			{
				InitiateUpdatePartyParams(pVideoBridgePartyInitParams);
			}
		}
	}

	CBridgePartyVideoOutParams* videoOutParams = (CBridgePartyVideoOutParams*)pVideoBridgePartyInitParams->GetMediaOutParams();
	if (videoOutParams)
	{
		// If audio out wasn't created already...
		if (!m_pBridgePartyOut)
		{
			NewPartyOut();

			std::auto_ptr<CTaskApi> pTaskApiVideoOut(new CTaskApi(*m_pConfApi));
			pTaskApiVideoOut->CreateOnlyApi(m_pConfApi->GetRcvMbx(), m_pBridgePartyOut);
			CRsrcDesc* pRsrcDesc = pRoutingTable->AddStateMachinePointerToRoutingTbl(m_partyRsrcID, eLogical_video_encoder, pTaskApiVideoOut.get());
			if (!pRsrcDesc)   // Entry not found in Routing Table
			{
				POBJDELETE(m_pBridgePartyOut);
				PASSERT(1);
			}
			else
			{
				rsrcParams.SetRsrcDesc(*pRsrcDesc);
				((CBridgePartyVideoOut*)(m_pBridgePartyOut))->Create(this, &rsrcParams, videoOutParams);
				((CBridgePartyVideoOut*)(m_pBridgePartyOut))->SetSiteNameInfo(pVideoBridgePartyInitParams->GetSiteNameInfo());
			}
		}

		// If we are in disconnecting state we need to save the Out Init params
		if (m_state == DISCONNECTING || (m_state == CONNECTED && m_pBridgePartyOut && m_pBridgePartyOut->GetState() == CBridgePartyVideoOut::DISCONNECTING))
		{
			if (m_pUpdatePartyInitParams != NULL)
			{
				if (m_pUpdatePartyInitParams->GetMediaOutParams() != NULL)
					m_pUpdatePartyInitParams->InitiateMediaOutParams((CBridgePartyVideoOutParams*)pVideoBridgePartyInitParams->GetMediaOutParams());
			}
			else
			{
				InitiateUpdatePartyParams(pVideoBridgePartyInitParams);
			}
		}
	}

	SetSiteName(pVideoBridgePartyInitParams->GetSiteName());
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::Import(const CBridgePartyInitParams* pBridgePartyInitParams)
{
	m_bIsAfterMove = TRUE;
	UpdateNewConfParams(pBridgePartyInitParams);

	CConfPartyRoutingTable* pRoutingTable = ::GetpConfPartyRoutingTable();
	PASSERT_AND_RETURN(!pRoutingTable);

	CRsrcParams rsrcParams(DUMMY_CONNECTION_ID, m_partyRsrcID, m_confRsrcID);

	CBridgePartyVideoInParams* videoInParams = (CBridgePartyVideoInParams*)pBridgePartyInitParams->GetMediaInParams();

	if (videoInParams)
	{
		bool isPartyVideoInExist = true;
		if (!m_pBridgePartyIn)
		{
			isPartyVideoInExist = false;
			NewPartyIn();
			//Update DB when connecting muted party
			CheckIsMutedVideoInAndUpdateDB();
		}

		SetTelepresenceInfo(videoInParams->GetTelePresenceEPInfo());

		std::auto_ptr<CTaskApi> pTaskApiVideoIn(new CTaskApi(*m_pConfApi));
		pTaskApiVideoIn->CreateOnlyApi(m_pConfApi->GetRcvMbx(), m_pBridgePartyIn);
		CRsrcDesc* pRsrcDesc = pRoutingTable->AddStateMachinePointerToRoutingTbl(m_partyRsrcID, eLogical_video_decoder, pTaskApiVideoIn.get());
		if (!pRsrcDesc)   // Entry not found in Routing Table
		{
			POBJDELETE(m_pBridgePartyIn);
			PASSERT(1);
		}
		else
		{
			if (isPartyVideoInExist)
			{
				((CBridgePartyVideoIn*)(m_pBridgePartyIn))->Import();

				if (!m_pBridgePartyIn->IsConnected())
					((CBridgePartyVideoIn*)(m_pBridgePartyIn))->UpdateNewConfParams(m_confRsrcID, videoInParams);
				else
					((CBridgePartyVideoIn*)(m_pBridgePartyIn))->UpdateNewConfParamsForOpenedPortAfterMove(m_confRsrcID, videoInParams);
			}
			else
			{
				rsrcParams.SetRsrcDesc(*pRsrcDesc);
				((CBridgePartyVideoIn*)(m_pBridgePartyIn))->Create(this, &rsrcParams, videoInParams);
			}
		}
	}

	CBridgePartyVideoOutParams* videoOutParams = (CBridgePartyVideoOutParams*)pBridgePartyInitParams->GetMediaOutParams();
	if (videoOutParams)
	{
		bool isPartyVideoOutExist = true;
		if (!m_pBridgePartyOut)
		{
			NewPartyOut();
			isPartyVideoOutExist = false;
		}

		if (!videoInParams) SetTelepresenceInfo(videoOutParams->GetTelePresenceEPInfo());

		std::auto_ptr<CTaskApi> pTaskApiVideoOut(new CTaskApi(*m_pConfApi));
		pTaskApiVideoOut->CreateOnlyApi(m_pConfApi->GetRcvMbx(), m_pBridgePartyOut);
		CRsrcDesc* pRsrcDesc = pRoutingTable->AddStateMachinePointerToRoutingTbl(m_partyRsrcID, eLogical_video_encoder, pTaskApiVideoOut.get());
		if (!pRsrcDesc)   // Entry not found in Routing Table
		{
			POBJDELETE(m_pBridgePartyOut);
			PASSERT(1);
		}
		else
		{
			if (isPartyVideoOutExist)
			{
				if (!m_pBridgePartyOut->IsConnected())
					((CBridgePartyVideoOut*)(m_pBridgePartyOut))->UpdateNewConfParams(m_confRsrcID, videoOutParams);
				else
					((CBridgePartyVideoOut*)(m_pBridgePartyOut))->UpdateNewConfParamsForOpenedPortAfterMove(m_confRsrcID, videoOutParams);
			}
			else
			{
				rsrcParams.SetRsrcDesc(*pRsrcDesc);
				((CBridgePartyVideoOut*)(m_pBridgePartyOut))->Create(this, &rsrcParams, videoOutParams);
			}

			((CBridgePartyVideoOut*)(m_pBridgePartyOut))->SetSiteNameInfo(pBridgePartyInitParams->GetSiteNameInfo());
		}
	}
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::ImportLegacy(const CBridgePartyInitParams* pBridgePartyInitParams, const CVideoBridgePartyCntl* pOldPartyCntl)
{
	m_bIsAfterMove = TRUE;

	UpdateNewConfParams(pBridgePartyInitParams);

	CConfPartyRoutingTable* pRoutingTable = ::GetpConfPartyRoutingTable();
	PASSERT_AND_RETURN(!pRoutingTable);

	bool delete_pOldPartyCntl = false;

	CRsrcParams rsrcParams(DUMMY_CONNECTION_ID, m_partyRsrcID, m_confRsrcID);

	// VNGR-26574 - RL>>VER 7.8>>Core dump in confparty
	// moving videoOutParams to before in - because we want to delete pOldPartyCntl before creating new pOldPartyCntl
	CBridgePartyVideoOutParams* videoOutParams = (CBridgePartyVideoOutParams*)pBridgePartyInitParams->GetMediaOutParams();
	if (videoOutParams)
	{
		NewPartyOut();  // In Move of Legacy the In and the Out are NULL, to be able to create them according to destination Bridge Type
		SetTelepresenceInfo(videoOutParams->GetTelePresenceEPInfo());
		std::auto_ptr<CTaskApi> pTaskApiVideoOut(new CTaskApi(*m_pConfApi));
		pTaskApiVideoOut->CreateOnlyApi(m_pConfApi->GetRcvMbx(), m_pBridgePartyOut);
		CRsrcDesc* pRsrcDesc = pRoutingTable->AddStateMachinePointerToRoutingTbl(m_partyRsrcID, eLogical_video_encoder, pTaskApiVideoOut.get());
		if (!pRsrcDesc) // Entry not found in Routing Table
		{
			POBJDELETE(m_pBridgePartyOut);
			PASSERT(1);
		}
		else
		{
			if (pOldPartyCntl->GetBridgePartyOut()) // Old PartyCntl has BridgePartyOut
			{
				if (!pOldPartyCntl->GetBridgePartyOut()->IsConnected()) // Old PartyCntlOut was NOT connected
				{
					rsrcParams.SetRsrcDesc(*pRsrcDesc);
					((CBridgePartyVideoOut*)(m_pBridgePartyOut))->Create(this, &rsrcParams, videoOutParams);
					((CBridgePartyVideoOut*)(m_pBridgePartyOut))->CleanAllLayoutsAndPrivateSettings();
					((CBridgePartyVideoOut*)(m_pBridgePartyOut))->CopyOldStateForMove((pOldPartyCntl->GetBridgePartyOut())->GetState());
					((CBridgePartyVideoOut*)(m_pBridgePartyOut))->UpdateNewConfParams(m_confRsrcID, videoOutParams);
				}
				else // Old PartyCntlOut was already connected
				{
					rsrcParams.SetRsrcDesc(*pRsrcDesc);
					((CBridgePartyVideoOut*)(m_pBridgePartyOut))->Create(this, &rsrcParams, videoOutParams);
					((CBridgePartyVideoOut*)(m_pBridgePartyOut))->CleanAllLayoutsAndPrivateSettings();
					((CBridgePartyVideoOut*)(m_pBridgePartyOut))->CopyOldStateForMove((pOldPartyCntl->GetBridgePartyOut())->GetState());  // Here we always copy CONNECTED state
					((CBridgePartyVideoOut*)(m_pBridgePartyOut))->UpdateNewConfParamsForOpenedPortAfterMove(m_confRsrcID, videoOutParams);
				}
			}
			else // Create new PartyOUT that will be connected to HW
			{
				rsrcParams.SetRsrcDesc(*pRsrcDesc);
				((CBridgePartyVideoOut*)(m_pBridgePartyOut))->Create(this, &rsrcParams, videoOutParams);
			}

			((CBridgePartyVideoOut*)(m_pBridgePartyOut))->SetSiteNameInfo(pBridgePartyInitParams->GetSiteNameInfo());
		}
	}

	CBridgePartyVideoInParams* videoInParams = (CBridgePartyVideoInParams*)pBridgePartyInitParams->GetMediaInParams();
	if (videoInParams)
	{
		NewPartyIn(); // In Move of Legacy the In and the Out are NULL, to be able to create them according to destination Bridge Type
		if (!videoOutParams)
			SetTelepresenceInfo(videoInParams->GetTelePresenceEPInfo());

		std::auto_ptr<CTaskApi> pTaskApiVideoIn(new CTaskApi(*m_pConfApi));
		pTaskApiVideoIn->CreateOnlyApi(m_pConfApi->GetRcvMbx(), m_pBridgePartyIn);
		CRsrcDesc* pRsrcDesc = pRoutingTable->AddStateMachinePointerToRoutingTbl(m_partyRsrcID, eLogical_video_decoder, pTaskApiVideoIn.get());
		if (!pRsrcDesc) // Entry not found in Routing Table
		{
			POBJDELETE(m_pBridgePartyIn);
			PASSERT(1);
		}
		else
		{
			if (pOldPartyCntl->GetBridgePartyIn()) // Old PartyCntl has BridgePartyIn
			{
				if (!pOldPartyCntl->GetBridgePartyIn()->IsConnected()) // Old PartyCntlIn was NOT connected
				{
					rsrcParams.SetRsrcDesc(*pRsrcDesc);
					WORD oldPartyInState = (pOldPartyCntl->GetBridgePartyIn())->GetState();
					((CBridgePartyVideoIn*)(m_pBridgePartyIn))->CopyAvcToSvcTranslatorsParams( *((CBridgePartyVideoIn*)pOldPartyCntl->GetBridgePartyIn()) );
					POBJDELETE(pOldPartyCntl);
					delete_pOldPartyCntl = true;
					((CBridgePartyVideoIn*)(m_pBridgePartyIn))->Create(this, &rsrcParams, videoInParams);
					((CBridgePartyVideoIn*)(m_pBridgePartyIn))->CopyOldStateForMove(oldPartyInState);
					((CBridgePartyVideoIn*)(m_pBridgePartyIn))->Import();	// because of AvcToSvcTranslate need to update the translators
					((CBridgePartyVideoIn*)(m_pBridgePartyIn))->UpdateNewConfParams(m_confRsrcID, videoInParams);
				}
				else // Old PartyCntlIn was already connected
				{
					rsrcParams.SetRsrcDesc(*pRsrcDesc);
					WORD oldPartyInState = (pOldPartyCntl->GetBridgePartyIn())->GetState();
					((CBridgePartyVideoIn*)(m_pBridgePartyIn))->CopyAvcToSvcTranslatorsParams( *((CBridgePartyVideoIn*)pOldPartyCntl->GetBridgePartyIn()) );
					POBJDELETE(pOldPartyCntl);
					delete_pOldPartyCntl = true;
					((CBridgePartyVideoIn*)(m_pBridgePartyIn))->CreateForMove(this, &rsrcParams, videoInParams);
					((CBridgePartyVideoIn*)(m_pBridgePartyIn))->CopyOldStateForMove(oldPartyInState); // copy CONNECTED state
					((CBridgePartyVideoIn*)(m_pBridgePartyIn))->Import();	// because of AvcToSvcTranslate need to update the translators
					((CBridgePartyVideoIn*)(m_pBridgePartyIn))->UpdateNewConfParamsForOpenedPortAfterMove(m_confRsrcID, videoInParams);
				}
			}
			else // Create new PartyIN that will be connected to HW !!! NO CreateForMove is needed !!!! want to keep the synclost since new
			{
				rsrcParams.SetRsrcDesc(*pRsrcDesc);
				((CBridgePartyVideoIn*)(m_pBridgePartyIn))->Create(this, &rsrcParams, videoInParams);
			}
			//Update DB when connecting muted party
			CheckIsMutedVideoInAndUpdateDB();
		}
	}

	if (!delete_pOldPartyCntl)
	{
		POBJDELETE(pOldPartyCntl);
	}
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::InitiateUpdatePartyParams(const CBridgePartyInitParams* pBridgePartyInitParams)
{
	POBJDELETE(m_pUpdatePartyInitParams);
	m_pUpdatePartyInitParams = new CUpdatePartyVideoInitParams(*pBridgePartyInitParams);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::Destroy()
{
	TRACEINTO << m_partyConfName << ", State:" << StateToString();

	CBridgePartyCntl::Destroy();
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::Setup()
{
	// In case we are in CONNECTED state and the second video direction start to connect -
	// will remain in CONNECTED state.
	if (m_state != CONNECTED)
		m_state = SETUP;

	// If video IN is not connected or in Setup state...
	bool isConnectBridgePartyIn  = (m_pBridgePartyIn && !m_pBridgePartyIn->IsConnected() && !m_pBridgePartyIn->IsConnecting());

	// If video OUT is not connected or in Setup state...
	bool isConnectBridgePartyOut = (m_pBridgePartyOut && !m_pBridgePartyOut->IsConnected() && !m_pBridgePartyOut->IsConnecting());

	TRACEINTO << m_partyConfName << ", State:" << StateToString() << ", isConnectBridgePartyIn:" << isConnectBridgePartyIn << ", isConnectBridgePartyOut:" << isConnectBridgePartyOut;

	StartTimer(VIDEO_BRDG_PARTY_SETUP_TOUT, VIDEO_BRDG_PARTY_SETUP_TOUT_VALUE);

	if (isConnectBridgePartyIn)
		m_pBridgePartyIn->Connect();

	if (isConnectBridgePartyOut)
		m_pBridgePartyOut->Connect();
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::Connect(BYTE isIVR, BYTE isContentHD1080Supported)
{
	CSegment* pSeg = new CSegment;
	*pSeg << isIVR << isContentHD1080Supported;
	DispatchEvent(VIDCONNECT, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::DisConnect()
{
	DispatchEvent(VIDDISCONNECT, NULL);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::Export()
{
	DispatchEvent(VIDEO_EXPORT, NULL);
}

//--------------------------------------------------------------------------
CLayout* CVideoBridgePartyCntl::GetReservationLayout() const
{
	return (m_pBridgePartyOut) ? ((CBridgePartyVideoOut*)m_pBridgePartyOut)->GetReservationLayout() : NULL;
}

//--------------------------------------------------------------------------
CLayout* CVideoBridgePartyCntl::GetCurrentLayout()
{
	return (m_pBridgePartyOut) ? ((CBridgePartyVideoOut*)m_pBridgePartyOut)->GetCurrentLayout() : NULL;
}

//--------------------------------------------------------------------------
CVisualEffectsParams* CVideoBridgePartyCntl::GetPartyVisualEffects() const
{
	return (m_pBridgePartyOut) ? ((CBridgePartyVideoOut*)m_pBridgePartyOut)->GetPartyVisualEffects() : NULL;
}

//--------------------------------------------------------------------------
WORD CVideoBridgePartyCntl::GetIsPrivateLayout() const
{
	return (m_pBridgePartyOut) ? ((CBridgePartyVideoOut*)m_pBridgePartyOut)->GetIsPrivateLayout() : 0;
}

//--------------------------------------------------------------------------
CLayout* CVideoBridgePartyCntl::GetPrivateReservationLayout() const
{
	return (m_pBridgePartyOut) ? ((CBridgePartyVideoOut*)m_pBridgePartyOut)->GetPrivateReservationLayout() : NULL;
}

//--------------------------------------------------------------------------
const CImage* CVideoBridgePartyCntl::GetPartyImage() const
{
	return (m_pBridgePartyIn) ? ((CBridgePartyVideoIn*)m_pBridgePartyIn)->GetPartyImage() : NULL;
}

//--------------------------------------------------------------------------
EIconType CVideoBridgePartyCntl::GetRecordingType() const
{
	return (m_pBridgePartyOut) ? ((CBridgePartyVideoOut*)m_pBridgePartyOut)->GetRecordingType() : E_ICON_REC_OFF;
}

//--------------------------------------------------------------------------
WORD CVideoBridgePartyCntl::GetIsAudioParticipantsIconActive() const
{
	//return (m_pBridgePartyOut) ? ((CBridgePartyVideoOut*)m_pBridgePartyOut)->GetIsAudioParticipantsIconActive() : 0;
	if(m_pBridgePartyOut)
	{
		//A new party is connected, show the audio participants icon if the conf have audio only participant
		if(	((CBridgePartyVideoOut*)m_pBridgePartyOut)->GetIsAudioIconToSentAfterOpenPort())
		{
			return ((CBridgePartyVideoOut*)m_pBridgePartyOut)->GetNumAudioParticipantsInConf();
		}
		//else, return the value in video bridge out
		return ((CBridgePartyVideoOut*)m_pBridgePartyOut)->GetIsAudioParticipantsIconActive();
	}
	else
		return NO;
}

//--------------------------------------------------------------------------
WORD CVideoBridgePartyCntl::GetNumAudioParticipantsInConf() const
{
	return (m_pBridgePartyOut) ? ((CBridgePartyVideoOut*)m_pBridgePartyOut)->GetNumAudioParticipantsInConf() : 0;
}

//--------------------------------------------------------------------------
BOOL CVideoBridgePartyCntl::isInGatheringMode() const
{
	return (m_pBridgePartyOut) ? ((CBridgePartyVideoOut*)m_pBridgePartyOut)->isInGatheringMode() : NO;
}

//--------------------------------------------------------------------------
bool  CVideoBridgePartyCntl::GetIsForce1X1() const
{
	return (m_pBridgePartyOut) ? ((CBridgePartyVideoOut*)m_pBridgePartyOut)->GetIsForce1x1() : false;
}
//--------------------------------------------------------------------------
DWORD CVideoBridgePartyCntl::GetVideoMSI() const
{
	DWORD msi = 0;

	if (!IsAVMCUParty())
		return msi;

	if(m_pBridgePartyIn)
	{
		const CImage* pImage = ((CBridgePartyVideoIn*)m_pBridgePartyIn)->GetPartyImage();
		if (pImage)
			msi = pImage->GetVideoMSI() ;
		else
		{ TRACEINTO << "NULL pImage, Name: " << m_partyConfName; }
	}
	return msi;
	//return (m_pBridgePartyIn) ? ((CBridgePartyVideoIn*)m_pBridgePartyIn)->GetPartyImage()->GetVideoMSI() : 0;
}
//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::SetVideoMSI(DWORD videoMSI)
{
	if (!IsAVMCUParty())
		return;

	if(m_pBridgePartyIn)
	{
		CImage* pImage = (CImage*)(((CBridgePartyVideoIn*)m_pBridgePartyIn)->GetPartyImage());
		if(pImage)
			pImage->setMSI(videoMSI);
		else
		{ TRACEINTO << "NULL pImage, Name: " << m_partyConfName; }
	}
}
//--------------------------------------------------------------------------
LayoutType CVideoBridgePartyCntl::GetConfLayoutType() const
{
	CLayout* pBrdgLayout = ((CVideoBridge*)m_pBridge)->GetReservationLayout();
	return (pBrdgLayout) ? pBrdgLayout->GetLayoutType() : CP_NO_LAYOUT;
}

//--------------------------------------------------------------------------
BYTE CVideoBridgePartyCntl::IsConfSameLayout() const
{
	return ((CVideoBridge*)m_pBridge)->IsSameLayout();
}

//--------------------------------------------------------------------------
BYTE CVideoBridgePartyCntl::IsImageInPartiesLayout(DWORD partyRscId)
{
	CLayout* currentLayout = GetCurrentLayout();
	return (currentLayout) ? (currentLayout->FindImagePlaceInLayout(partyRscId) != AUTO) : FALSE;
}

//--------------------------------------------------------------------------
LayoutType CVideoBridgePartyCntl::GetPartyCurrentLayoutType() const
{
	return (m_pBridgePartyOut) ? ((CBridgePartyVideoOut*)m_pBridgePartyOut)->GetPartyCurrentLayoutType() : CP_NO_LAYOUT;
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::UpdateSelfMute(RequestPriority who, EOnOff eOnOff)
{
	if (m_pBridgePartyIn)
	{
		((CBridgePartyVideoIn*)m_pBridgePartyIn)->UpdateSelfMute(who, eOnOff);

		DWORD updateStatus;
		switch (who)
		{
			case MCMS_Prior:
				{ updateStatus = (eOnOff == eOff) ? 0x0F00000E : 0x0F000010; break; }

			case OPERATOR_Prior:
				{ updateStatus = (eOnOff == eOff) ? 0x0000000E : 0x00000010; break; }

			case PARTY_Prior:
				{ updateStatus = (eOnOff == eOff) ? 0xF000000E : 0xF0000010; break; }

			default:
				{ return; /* no need to send updateDB */ }
		} // switch

		m_pConfApi->UpdateDB(m_pParty, MUTE_STATE, updateStatus);
		return;
	}

	TRACEINTO << m_partyConfName << " - Failed, 'm_pBridgePartyIn' is invalid";
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::UpdateVideoInParams(CBridgePartyVideoParams* pBridgePartyVideoParams)
{
	CSegment seg;
	pBridgePartyVideoParams->Serialize(NATIVE, seg);
	DispatchEvent(UPDATE_VIDEO_IN_PARAMS, &seg);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::UpdateVideoOutParams(CBridgePartyVideoParams* pBridgePartyVideoParams)
{
	CSegment seg;
	pBridgePartyVideoParams->Serialize(NATIVE, seg);
	DispatchEvent(UPDATE_VIDEO_OUT_PARAMS, &seg);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::UpdatePartyTelePresenceMode(eTelePresencePartyType partyNewTelePresenceMode)
{
	if (m_pBridgePartyOut)
		((CBridgePartyVideoOut*)(m_pBridgePartyOut))->UpdatePartyTelePresenceMode(partyNewTelePresenceMode);

	if (m_pBridgePartyIn)
		((CBridgePartyVideoIn*)(m_pBridgePartyIn))->UpdatePartyTelePresenceMode(partyNewTelePresenceMode);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::ForwardVINToParty(WORD mcuNumber, WORD terminalNumber, PartyRsrcID partyId)
{
	m_pPartyApi->SendVIN(mcuNumber, terminalNumber, partyId);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::IvrCommand(OPCODE opcode, CSegment* pDataSeg)
{
	DispatchEvent(opcode, pDataSeg);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::IvrNotification(OPCODE opcode, CSegment* pParam)
{
	m_pConfApi->IvrPartyNotification(m_partyRsrcID, m_pParty, m_name, opcode, pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::PCMNotification(OPCODE opcode, CSegment* pParam)
{
	m_pConfApi->PCMPartyNotification(m_partyRsrcID, opcode, pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::PLC_SetPartyPrivateLayout(LayoutType newPrivateLayoutType)
{
	CSegment seg;
	seg << (byte)newPrivateLayoutType;
	DispatchEvent(PLC_SETPARTYPRIVATELAYOUTTYPE, &seg);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::VENUS_SetPartyPrivateLayout(LayoutType newPrivateLayoutType)
{
	CSegment seg;
	seg << (byte)newPrivateLayoutType;
	DispatchEvent(VENUS_SETPARTYPRIVATELAYOUTTYPE, &seg);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::PLC_PartyReturnToConfLayout()
{
	DispatchEvent(PLC_RETURNPARTYTOCONFLAYOUT);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::PLC_ForceToCell(char* partyImageToSee, BYTE cellToForce)
{
	CSegment seg;
	seg << partyImageToSee << cellToForce;
	DispatchEvent(PLC_FORCECELLZERO, &seg);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::PLC_CancelAllPrivateLayoutForces()
{
	DispatchEvent(PLC_CANCELALLPRIVATELAYOUTFORCES);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::VideoConnectionCompletion(CSegment* pParams, EMediaDirection eConnectedMediaDirection)
{
	PASSERT_AND_RETURN(!m_pBridgePartyIn && !m_pBridgePartyOut);

	EStat           receivedStatus             = statOK;
	EMediaDirection ConnectedDirection         = eNoDirection;
	BOOL            isVideoConnectionCompleted = FALSE;

	*pParams >> (BYTE&)receivedStatus;

	if (statOK != receivedStatus)
	{
		DeleteTimer(VIDEO_BRDG_PARTY_SETUP_TOUT);

		DumpMcuInternalProblemDetailed((BYTE)eConnectedMediaDirection, eMipStatusFail, eMipVideo);

		// Inform Video Bridge about connection failure
		m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, VIDEO_BRIDGE_MSG, ENDCONNECTPARTY, receivedStatus, FALSE, eNoDirection, pParams);
		TRACEINTO << m_partyConfName << ", Status:" << receivedStatus << " - Failed, because received invalid status" << ", direction: " << (DWORD)eConnectedMediaDirection;
		return;
	}

	// Video-in is connected
	if (eMediaIn == eConnectedMediaDirection)
	{
		if (IsUniDirectionConnection(eMediaIn))
			isVideoConnectionCompleted = TRUE, ConnectedDirection = eMediaIn;
		else if (m_pBridgePartyOut && (m_pBridgePartyOut->IsConnected() || m_pBridgePartyOut->IsDisconnecting()))  // vngr-19742 in case we started to open both but one side is disconnecting we wont receive that its connected but disconnected thus we need to finish the connection of the party(the other side that finished to connect)
			isVideoConnectionCompleted = TRUE, ConnectedDirection = eMediaInAndOut;
	}

	// Video-out is connected
	if (eMediaOut == eConnectedMediaDirection)
	{
		m_bVideoOutConnectedReadyForGathering = true;
		TryStartGathering();

		if (IsUniDirectionConnection(eMediaOut))
			isVideoConnectionCompleted = TRUE, ConnectedDirection = eMediaOut;
		else if (m_pBridgePartyIn && (m_pBridgePartyIn->IsConnected() || m_pBridgePartyIn->IsDisconnecting()))     // vngr-19742 in case we started to open both but one side is disconnecting we wont receive that its connected but disconnected thus we need to finish the connection of the party(the other side that finished to connect)
			isVideoConnectionCompleted = TRUE, ConnectedDirection = eMediaInAndOut;
	}

	TRACEINTO << m_partyConfName << ", isVideoConnectionCompleted:" << (int)isVideoConnectionCompleted << ", MediaDirection:" << ConnectedDirection;

	if (TRUE == isVideoConnectionCompleted)
	{
		DeleteTimer(VIDEO_BRDG_PARTY_SETUP_TOUT);

		m_state = CONNECTED;

		// Inform Video Bridge
		m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, VIDEO_BRIDGE_MSG, ENDCONNECTPARTY, statOK, FALSE, ConnectedDirection);
	}
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::VideoDisConnectionCompletion(CSegment* pParams, EMediaDirection eDisConnectedMediaDirection)
{
	PASSERT_AND_RETURN(!m_pBridgePartyIn && !m_pBridgePartyOut);

	BOOL            isVideoDisConnectionCompleted = FALSE;
	BOOL            isHalfDisconnection           = FALSE;
	EStat           receivedStatus                = statOK;
	EMediaDirection ConnectedDirection            = eNoDirection;
	BYTE            videoOutClosePortStatus       = statOK;
	BYTE            videoInClosePortStatus        = statOK;

	*pParams >> (BYTE&)receivedStatus;

	// *** 1. Check if this is FULL disconnection or not - Full disconnection is when all connected directions disconnects!!! ****

	// Video-in is disconnected
	if (eMediaIn == eDisConnectedMediaDirection)
	{
		// Check if only Video in was connected or if both directions were connected but video-out was already disconnected.
		if (IsUniDirectionConnection(eMediaIn) || (m_pBridgePartyOut && m_pBridgePartyOut->IsDisConnected()))
			isVideoDisConnectionCompleted = TRUE;
	}

	// Video-out is disconnected
	if (eMediaOut == eDisConnectedMediaDirection)
	{
		DWORD    isPrivate = NO;
		CSegment vidLayoutSeg;

		// Check if only Video out was connected or if both directions were connected but video-in was already disconnected.
		if (IsUniDirectionConnection(eMediaOut) || (m_pBridgePartyIn && m_pBridgePartyIn->IsDisConnected()))
			isVideoDisConnectionCompleted = TRUE;

		if (GetCurrentLayout() && !(GetCurrentLayout()->Serialize(PARTY_lev, &vidLayoutSeg)))
		{ }
		else
			DBGPASSERT(1);
	}

	// *** 2. If this is full disconnection  ***

	if (TRUE == isVideoDisConnectionCompleted)
	{
		DeleteTimer(VIDEO_BRDG_PARTY_DISCONNECT_TOUT);

		ConnectedDirection = eNoDirection;
		m_state            = IDLE;

		SetDisConnectingDirectionsReq(eNoDirection);

		if (m_pBridgePartyIn)
			videoInClosePortStatus = m_pBridgePartyIn->GetClosePortAckStatus();

		if (m_pBridgePartyOut)
			videoOutClosePortStatus = m_pBridgePartyOut->GetClosePortAckStatus();

		// for debug info in case of the "MCU internal problem"
		BYTE failureCauseDirection = eMipNoneDirction;

		if (videoInClosePortStatus != STATUS_OK)
			receivedStatus = (EStat)videoInClosePortStatus, failureCauseDirection = eMipIn;
		else
			receivedStatus = (EStat)videoOutClosePortStatus, failureCauseDirection = eMipOut;

		// Inform Video Bridge In case of problem
		if (statVideoInOutResourceProblem == receivedStatus)
		{
			CSegment* pSeg = new CSegment;
			*pSeg << (BYTE)receivedStatus << (BYTE)failureCauseDirection << (BYTE)eMipStatusFail << (BYTE)eMipClose;

			if (GetConnectionFailureCause() == statOK)
				DumpMcuInternalProblemDetailed((BYTE)failureCauseDirection, eMipStatusFail, eMipVideo);

			m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, VIDEO_BRIDGE_MSG, ENDDISCONNECTPARTY, receivedStatus, FALSE, ConnectedDirection, pSeg);
			POBJDELETE(pSeg);
		}
		else  // In case we received connect req while disconnecting
		{
			if (m_pUpdatePartyInitParams)
			{
				TRACEINTO << m_partyConfName << " - Both direction were disconnected, now start connection process";
				StartConnectProcess();
			}
			else
			{
				TRACEINTO << m_partyConfName << " - Both direction were disconnected, now state is IDLE";
				DestroyPartyInOut(ConnectedDirection);

				// Inform Video Bridge - Add the direction connected state
				m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, VIDEO_BRIDGE_MSG, ENDDISCONNECTPARTY, receivedStatus, FALSE, ConnectedDirection);
			}
		}
	}

	// *** 3. If this is not full disconnection we need to check if half disconnection
	// (according to the disconnection req we received from partyCntl)   ****

	else    // Incase both directio were connected or in setup and only one direction was disconnected ,according to the disconnect direction we recieved from the partyCntl
	{
		BYTE failureCauseDirection = eMipNoneDirction;

		// Incase VideoOut disconnected
		if (m_pBridgePartyOut && (eDisConnectedMediaDirection == eMediaOut) && (m_pBridgePartyIn && ((m_pBridgePartyIn->IsConnected()) || (m_pBridgePartyIn->IsConnecting()))) && GetDisconnectingDirectionsReq() == eMediaOut)
		{
			isHalfDisconnection     = TRUE;
			ConnectedDirection      = eMediaIn;

			// For debug in case of MCU internal problem
			videoOutClosePortStatus = m_pBridgePartyOut->GetClosePortAckStatus();
			if (videoOutClosePortStatus != STATUS_OK)
			{
				TRACEINTO << m_partyConfName << ", status:" << videoOutClosePortStatus << " - The encoder 'close port' status is invalid";
				receivedStatus        = (EStat)videoOutClosePortStatus;
				failureCauseDirection = eMipOut;
			}
		}
		// Incase VideoIn disconnected
		else if (m_pBridgePartyIn && (eDisConnectedMediaDirection == eMediaIn) && (m_pBridgePartyOut && ((m_pBridgePartyOut->IsConnected()) || (m_pBridgePartyOut->IsConnecting()))) && GetDisconnectingDirectionsReq() == eMediaIn)
		{
			isHalfDisconnection    = TRUE;
			ConnectedDirection     = eMediaOut;

			// For debug in case of MCU internal problem
			videoInClosePortStatus = m_pBridgePartyIn->GetClosePortAckStatus();
			if (videoInClosePortStatus != STATUS_OK)
			{
				TRACEINTO << m_partyConfName << ", status:" << videoInClosePortStatus << " - The decoder 'close port' status is invalid";
				receivedStatus        = (EStat)videoInClosePortStatus;
				failureCauseDirection = eMipIn;
			}
		}

		if (isHalfDisconnection)
		{
			DeleteTimer(VIDEO_BRDG_PARTY_DISCONNECT_TOUT);
			SetDisConnectingDirectionsReq(eNoDirection);

			// Inform Video Bridge In case of problem
			if (statVideoInOutResourceProblem == receivedStatus)
			{
				CSegment* pSeg = new CSegment;
				*pSeg << (BYTE)receivedStatus << (BYTE)failureCauseDirection << (BYTE)eMipStatusFail <<(BYTE)eMipClose;
				m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, VIDEO_BRIDGE_MSG, ENDDISCONNECTPARTY, receivedStatus, FALSE, ConnectedDirection, pSeg);
				POBJDELETE(pSeg);
			}
			else
			{
				// if received connect req while disconnecting
				if (m_pUpdatePartyInitParams)
				{
					TRACEINTO << m_partyConfName << " - Start connection process";

					DestroyPartyInOut(ConnectedDirection);
					StartConnectProcess();
				}
				else
				{
					TRACEINTO << m_partyConfName << " - Disconnected only one direction, the state remain the same";

					DestroyPartyInOut(ConnectedDirection);

					TRACEINTO << "ConnectedDirection: " << ConnectedDirection;
					// Inform Video Bridge - Add the direction connected state
					m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, VIDEO_BRIDGE_MSG, ENDDISCONNECTPARTY, receivedStatus, FALSE, ConnectedDirection);
				}
			}
		}
	}
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::StartConnectProcess()
{
	if ((m_pUpdatePartyInitParams->GetMediaInParams()))
	{
		if (!m_pBridgePartyIn)
		{
			CreatePartyIn();
			SetTelepresenceInfo(((CBridgePartyVideoInParams*)m_pUpdatePartyInitParams->GetMediaInParams())->GetTelePresenceEPInfo());
			//Update DB when connecting muted party
			CheckIsMutedVideoInAndUpdateDB();
		}
		else
			((CBridgePartyVideoIn*)(m_pBridgePartyIn))->UpdatePartyInParams(m_pUpdatePartyInitParams);
	}

	if ((m_pUpdatePartyInitParams->GetMediaOutParams()))
	{
		if (!m_pBridgePartyOut)
		{
			CreatePartyOut();
			if (!(m_pUpdatePartyInitParams->GetMediaInParams()))
				SetTelepresenceInfo(((CBridgePartyVideoOutParams*)m_pUpdatePartyInitParams->GetMediaOutParams())->GetTelePresenceEPInfo());
		}
		else
			((CBridgePartyVideoOut*)(m_pBridgePartyOut))->UpdatePartyOutParams(m_pUpdatePartyInitParams);
	}

	Connect(m_pUpdatePartyInitParams->GetIsIVR());
	POBJDELETE(m_pUpdatePartyInitParams);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::CreatePartyIn()
{
	CConfPartyRoutingTable* pRoutingTable = ::GetpConfPartyRoutingTable();
	PASSERT_AND_RETURN(!pRoutingTable);

	CRsrcParams rsrcParams(DUMMY_CONNECTION_ID, m_partyRsrcID, m_confRsrcID);

	NewPartyIn();

	std::auto_ptr<CTaskApi> pTaskApiVideoIn(new CTaskApi(*m_pConfApi));
	pTaskApiVideoIn->CreateOnlyApi(m_pConfApi->GetRcvMbx(), m_pBridgePartyIn);
	CRsrcDesc* pRsrcDesc = pRoutingTable->AddStateMachinePointerToRoutingTbl(m_partyRsrcID, eLogical_video_decoder, pTaskApiVideoIn.get());
	if (!pRsrcDesc)   // Entry not found in Routing Table
	{
		POBJDELETE(m_pBridgePartyIn);
		PASSERT(1);
	}
	else
	{
		rsrcParams.SetRsrcDesc(*pRsrcDesc);
		((CBridgePartyVideoIn*)(m_pBridgePartyIn))->Create(this, &rsrcParams, m_pUpdatePartyInitParams->GetMediaInParams());
		//Update DB when connecting muted party
		CheckIsMutedVideoInAndUpdateDB();
	}
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::CreatePartyOut()
{
	CConfPartyRoutingTable* pRoutingTable = ::GetpConfPartyRoutingTable();
	PASSERT_AND_RETURN(!pRoutingTable);

	CRsrcParams rsrcParams(DUMMY_CONNECTION_ID, m_partyRsrcID, m_confRsrcID);

	NewPartyOut();

	std::auto_ptr<CTaskApi> pTaskApiVideoOut(new CTaskApi(*m_pConfApi));
	pTaskApiVideoOut->CreateOnlyApi(m_pConfApi->GetRcvMbx(), m_pBridgePartyOut);
	CRsrcDesc* pRsrcDesc = pRoutingTable->AddStateMachinePointerToRoutingTbl(m_partyRsrcID, eLogical_video_encoder, pTaskApiVideoOut.get());
	if (!pRsrcDesc)   // Entry not found in Routing Table
	{
		POBJDELETE(m_pBridgePartyOut);
		PASSERT(1);
	}
	else
	{
		rsrcParams.SetRsrcDesc(*pRsrcDesc);
		((CBridgePartyVideoOut*)(m_pBridgePartyOut))->Create(this, &rsrcParams, m_pUpdatePartyInitParams->GetMediaOutParams());
	}
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::AddImage(const CTaskApp* pParty)
{
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pParty));
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD)pParty;
	DispatchEvent(ADDIMAGE, pSeg);
	POBJDELETE(pSeg);
}
//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::UpdateImage()
{
	DispatchEvent(UPDATEIMAGE, NULL);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::UpdateIndicationIcons(BOOL bUseSharedMemForIndicationIcon)
{
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD)bUseSharedMemForIndicationIcon;

	DispatchEvent(INDICATION_ICONS_CHANGE, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::FastUpdate(void)
{
	DispatchEvent(FASTUPDATE, NULL);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::MuteImage(void)
{
	DispatchEvent(MUTEIMAGE, NULL);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::UnMuteImage(void)
{
	DispatchEvent(UNMUTEIMAGE, NULL);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::ChangeSpeakers(const CTaskApp* pNewVideoSpeaker, const CTaskApp* pNewAudioSpeaker)
{
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD)pNewVideoSpeaker;
	*pSeg << (DWORD)pNewAudioSpeaker;
	DispatchEvent(SPEAKERS_CHANGED, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::ChangeAudioSpeaker(const CTaskApp* pNewSpeaker)
{
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD)pNewSpeaker;
	DispatchEvent(AUDIO_SPEAKER_CHANGED, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::DelImage(const CTaskApp* pParty)
{
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD)pParty;
	DispatchEvent(DELIMAGE, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::ChangeConfLayout(CLayout* pConfLayout, BYTE bAnyway /*= 0*/)
{
	CSegment* pSeg = new CSegment;
	if (pConfLayout != NULL)
		*pSeg << (DWORD)pConfLayout;
	else
		*pSeg << (DWORD)0;

	*pSeg << (BYTE)0; // for SendChangeLayoutAlways param

	DispatchEvent(CHANGECONFLAYOUT, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
// EMB_MLA_Guy
void CVideoBridgePartyCntl::ChangeLayoutOfTPRoomSublink(CLayout* pConfLayout, BORDERS_PARAM_S* pTPSpecificBorders)
{
	CSegment* pSeg = new CSegment;
	if (pConfLayout != NULL)
		*pSeg << (DWORD)pConfLayout;
	else
		*pSeg << (DWORD)0;
	TRACEINTO << "partyId:" << GetPartyRsrcID() << ", pTPSpecificBorders- " << (void*)pTPSpecificBorders;
	if (pTPSpecificBorders != NULL)
		*pSeg << (DWORD)pTPSpecificBorders;
	else
		*pSeg << (DWORD)0;

	DispatchEvent(CHANGEPARTYLAYOUT_TPROOM_SUBLINK, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::ChangePartyLayout(CVideoLayout& newVideoLayout)
{
	CSegment* pSeg = new CSegment;
	newVideoLayout.Serialize(NATIVE, *pSeg);
	DispatchEvent(CHANGEPARTYLAYOUT, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::ChangePartyPrivateLayout(CVideoLayout& newVideoLayout)
{
	CSegment* pSeg = new CSegment;
	newVideoLayout.Serialize(NATIVE, *pSeg);
	DispatchEvent(CHANGEPARTYPRIVATELAYOUT, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::ChangeLayoutPrivatePartyButtonOnly(WORD isPrivate)
{
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD)isPrivate;
	DispatchEvent(SETPARTYPRIVATELAYOUTONOFF, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::ActionsAfterTelepresenceModeChanged(WORD isPrivate)
{
	TRACECOND_AND_RETURN(!m_pBridgePartyOut, "CVideoBridgePartyCntl::ActionsAfterTelepresenceModeChanged - Failed, " << m_partyConfName);

	((CBridgePartyVideoOut*)m_pBridgePartyOut)->SetTelepresenceModeChanged(TRUE);

	ChangeLayoutPrivatePartyButtonOnly(isPrivate);
}
//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::UpdateVisualEffects(CVisualEffectsParams* pVisualEffects, BOOL bInternalUpdateOnly)
{
	CSegment* pSeg = new CSegment;
	pVisualEffects->Serialize(NATIVE, *pSeg);
	*pSeg << (BYTE)bInternalUpdateOnly;
	DispatchEvent(UPDATEVISUALEFFECTS, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
ePartyLectureModeRole CVideoBridgePartyCntl::GetPartyLectureModeRole() const
{
	return (m_pBridgePartyOut) ? ((CBridgePartyVideoOut*)m_pBridgePartyOut)->GetPartyLectureModeRole() : eREGULAR;
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::SetPartyLectureModeRole(ePartyLectureModeRole partyLectureModeRole)
{
	CSegment* pSeg = new CSegment;
	*pSeg << (BYTE)partyLectureModeRole;
	DispatchEvent(UPDATE_PARTY_LECTURE_MODE_ROLE, pSeg);
	POBJDELETE(pSeg);
}

// ------------------------------------------------------------
void CVideoBridgePartyCntl::DeletePartyFromConf(const char* pDeletedPartyName)
{
	CSegment* pSeg = new CSegment;
	*pSeg<< pDeletedPartyName;
	DispatchEvent(DELETED_PARTY_FROM_CONF, pSeg);
	POBJDELETE(pSeg);
}
// ------------------------------------------------------------
void CVideoBridgePartyCntl::SendMsgToScreenPerParty(CTextOnScreenMngr* pTextMsgList, DWORD timeout)
{
	CSegment* pSeg = new CSegment;
	pTextMsgList->Serialize(NATIVE, *pSeg);
	*pSeg << timeout;
	DispatchEvent(DISPLAY_TEXT_ON_SCREEN, pSeg);
	POBJDELETE(pSeg);
}
// ------------------------------------------------------------
void CVideoBridgePartyCntl::StopMsgToScreenPerParty()
{
	TRACECOND_AND_RETURN(!m_pBridgePartyOut, "CVideoBridgePartyCntl::StopMsgToScreenPerParty - Failed, " << m_partyConfName);
	((CBridgePartyVideoOut*)m_pBridgePartyOut)->StopDisplayPartyTextOnScreen();
}

// ------------------------------------------------------------
void CVideoBridgePartyCntl::SendVSRToSipParty(ST_VSR_MUTILPLE_STREAMS* multipleVsr)
{
	TRACECOND_AND_RETURN(!m_pPartyApi, "CVideoBridgePartyCntl::SendVSRToSipParty - Failed - No valid m_pPartyApi, " << m_partyConfName);
	TRACEINTO << " Party Name: " << m_partyConfName;
	m_pPartyApi->SendMultipeVsrToSipParty(multipleVsr);
}
//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeConnectIDLE(CSegment* pParam)
{
	if (!m_pBridgePartyIn && !m_pBridgePartyOut)
	{
		m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, VIDEO_BRIDGE_MSG, ENDCONNECTPARTY, statInconsistent, FALSE, eNoDirection);
		return;
	}

	BYTE isIVR;
	*pParam >> isIVR;

	TRACEINTO << m_partyConfName << ", IsIVR:" << (int)isIVR;

	if (isIVR)
	{
		m_state = CONNECTED_STANDALONE;

		EMediaDirection eDirection = eNoDirection;
		if (m_pBridgePartyIn)
			eDirection |= eMediaIn;

		if (m_pBridgePartyOut)
			eDirection |= eMediaOut;

		// Inform Video Bridge
		m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, VIDEO_BRIDGE_MSG, ENDCONNECTPARTY_IVR_MODE, statOK, FALSE, eDirection);
	}
	else
	{
		if (m_pBridgePartyIn)
		{
			TRACEINTO << m_partyConfName << " - Turn on the l-video-sync-loss flag";
			// Turn on the l-video-sync-loss flag in case of No IVR
			m_pConfApi->UpdateDB(m_pParty, H221, LOCALVIDEO << 16, 1);
		}

		Setup();
	}
}

//--------------------------------------------------------------------------
// Incase one of the video channels (In/Out) is in setup state and now connects the second direction.
void CVideoBridgePartyCntl::OnVideoBridgeConnectSETUP(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoBridgeConnect(pParam);
}

//--------------------------------------------------------------------------
// In case one of the video channels (In/Out) is already connected and now connects the second direction.
void CVideoBridgePartyCntl::OnVideoBridgeConnectCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_partyConfName
	          << ", IsBridgePartyInConnected:"  << ((m_pBridgePartyIn && m_pBridgePartyIn->IsConnected()) ? "1" : "0")
	          << ", IsBridgePartyOutConnected:" << ((m_pBridgePartyOut && m_pBridgePartyOut->IsConnected()) ? "1" : "0")
	          << ", IsAfterMove:" << (bool)m_bIsAfterMove;

	if (m_pBridgePartyIn && m_pBridgePartyOut && m_pBridgePartyOut->IsConnected() && m_pBridgePartyIn->IsConnected())
	{
		if (m_bIsAfterMove)
		{
			// send change layout
			((CBridgePartyVideoOut*)m_pBridgePartyOut)->BuildLayout();
			// Send VCU request to EP
			HandleEvent(NULL, 0, VIDREFRESH);
			// Add party to conference mix
			CSegment* pSeg = new CSegment;
			*pSeg << (DWORD)GetPartyRsrcID();/*m_pParty->GetPartyId()*/
			m_pBridge->HandleEvent(pSeg, 0, ENDIMPPARTY);
			POBJDELETE(pSeg);
		}

		// Inform Video Bridge
		m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, VIDEO_BRIDGE_MSG, ENDCONNECTPARTY, statOK, FALSE, eMediaInAndOut);
	}
	else
	{
		if (m_pBridgePartyIn && m_pBridgePartyIn->IsConnected())
		{
			if (m_bIsAfterMove)
			{
				// Send VCU request to EP
				HandleEvent(NULL, 0, VIDREFRESH);
				// Add party to conference mix
				CSegment* pSeg = new CSegment;
				*pSeg << (DWORD)GetPartyRsrcID();/*m_pParty->GetPartyId();*/
				m_pBridge->HandleEvent(pSeg, 0, ENDIMPPARTY);
				POBJDELETE(pSeg);
				// Inform Video Bridge
				m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, VIDEO_BRIDGE_MSG, ENDCONNECTPARTY, statOK, FALSE, eMediaIn);
			}

			OnVideoBridgeConnect(pParam);
		}
		else
		{
			if (m_pBridgePartyOut && m_pBridgePartyOut->IsConnected())
			{
				if (m_bIsAfterMove)
				{
					// send change layout
					((CBridgePartyVideoOut*)m_pBridgePartyOut)->BuildLayout();
					// Inform Video Bridge
					m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, VIDEO_BRIDGE_MSG, ENDCONNECTPARTY, statOK, FALSE, eMediaOut);
				}
			}

			OnVideoBridgeConnect(pParam);
		}
	}
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeConnect(CSegment* pParam)
{
	if (!m_pBridgePartyIn && !m_pBridgePartyOut)
	{
		m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, VIDEO_BRIDGE_MSG, ENDCONNECTPARTY, statInconsistent, FALSE);
		return;
	}

	BYTE isIVR;
	*pParam >> isIVR;

	TRACEINTO << m_partyConfName;

	// In this stage we shouldn't receive connect with IVR
	PASSERT(isIVR);

	if (m_pBridgePartyIn && ((CBridgePartyVideoIn*)m_pBridgePartyIn)->IsPartyImageSyncLoss())
	{
		// Turn on the l-video-sync-loss flag in case of No IVR
		m_pConfApi->UpdateDB(m_pParty, H221, LOCALVIDEO << 16, 1);
		TRACEINTO << m_partyConfName << " - Turn on the l-video-sync-loss flag";
	}

	Setup();
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeConnectCONNECTED_STANDALONE(CSegment* pParam)
{
	if (!m_pBridgePartyIn && !m_pBridgePartyOut)
	{
		m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, VIDEO_BRIDGE_MSG, ENDCONNECTPARTY, statInconsistent, FALSE);
		return;
	}

	BYTE isIVR;
	*pParam >> isIVR;

	TRACEINTO << m_partyConfName << ", IsIVR:" << (int)isIVR;

	if (isIVR)
	{
		EMediaDirection eDirection = eNoDirection;
		if (m_pBridgePartyIn)
			eDirection |= eMediaIn;

		if (m_pBridgePartyOut)
			eDirection |= eMediaOut;

		// Inform Video Bridge
		m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, VIDEO_BRIDGE_MSG, ENDCONNECTPARTY_IVR_MODE, statOK, FALSE, eDirection);
	}
	else
	{
		Setup();
	}
}

//--------------------------------------------------------------------------
// Incase both video direction are now disconnecting and we received connect command
// we will save the isIVR parameter but wait till the disconnecting process ends before we
// start connect process again.
void CVideoBridgePartyCntl::OnVideoBridgeConnectDISCONNECTING(CSegment* pParam)
{
	if (!m_pUpdatePartyInitParams)
	{
		m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, VIDEO_BRIDGE_MSG, ENDCONNECTPARTY, statInconsistent, FALSE);
		return;
	}

	TRACEINTO << m_partyConfName << " - Wait to end of disconnect before we start connect again";

	BYTE IsIVR;
	*pParam >> IsIVR;
	m_pUpdatePartyInitParams->SetIsIVR(IsIVR);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoOutConnectedSETUP(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	VideoConnectionCompletion(pParam, eMediaOut);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoOutConnectedCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;

	// In case video out connected after video in (we are in connected state)
	// we will update detected mode in video out.
	UpdateDecoderDetectedMode();
	VideoConnectionCompletion(pParam, eMediaOut);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoOutConnectedDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << m_partyConfName << " - Ignored";
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoInConnectedSETUP(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	VideoConnectionCompletion(pParam, eMediaIn);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoInConnectedCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	VideoConnectionCompletion(pParam, eMediaIn);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoInConnectedDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << m_partyConfName << " - Ignored";
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoInSyncedSETUP(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoInSynced(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoInSyncedCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoInSynced(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoInSynced(CSegment* pParam)
{
	UpdateDecoderDetectedMode();
	// Send Msg To Video Bridge
	m_pConfApi->PartyVideoInSynced(m_pParty, VIDEO_BRIDGE_MSG, statOK);

	m_bVideoInSyncedReadyForGathering = true;
	TryStartGathering();
    if(!pParam)return;
    if(m_pPartyApi)
    {
    	CSegment *pSeg = new CSegment(*pParam);
    	m_pPartyApi->HandlePartyExternalEvent(pSeg, VIDEO_IN_SYNCED);
    }
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::TryStartGathering()
{
	if (m_bVideoInSyncedReadyForGathering && m_bVideoOutConnectedReadyForGathering)
	{
		if (GetCascadeLinkMode() == NONE)
		{
			TRACEINTO << m_partyConfName;
			CSegment* seg = new CSegment;
			*seg << (DWORD)m_pParty;
			m_pConfApi->SendMsg(seg, GATHERING_PARTY_CONNECTED_MSG);
		}
		else
			TRACEINTO << m_partyConfName << " - Cascade links are excluded from gathering phase";
	}
}
//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeDisconnectCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoBridgeDisconnect(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeDisconnectCONNECTED_STANDALONE(CSegment* pParam)
{
	EMediaDirection eMediaDirectionDisconnected = GetDisconnectingDirectionsReq();

	TRACEINTO << m_partyConfName << ", MediaDirectionDisconnected:" << eMediaDirectionDisconnected;

	CConfPartyRoutingTable* pRoutingTable = ::GetpConfPartyRoutingTable();
	PASSERT(!pRoutingTable);

	if (pRoutingTable)
	{
		if ((eMediaDirectionDisconnected & eMediaOut) && m_pBridgePartyOut)
		{
			CRsrcParams* pRsrcParams = m_pBridgePartyOut->GetRsrcParams();
			PASSERT(!pRsrcParams);

			if (pRsrcParams)
				DBGPASSERT(STATUS_FAIL == pRoutingTable->RemoveStateMachinePointerFromRoutingTbl(*pRsrcParams));

			POBJDELETE(m_pBridgePartyOut);
		}

		if ((eMediaDirectionDisconnected & eMediaIn) && m_pBridgePartyIn)
		{
			CRsrcParams* pRsrcParams = m_pBridgePartyIn->GetRsrcParams();
			PASSERT(!pRsrcParams);

			if (pRsrcParams)
				DBGPASSERT(STATUS_FAIL == pRoutingTable->RemoveStateMachinePointerFromRoutingTbl(*pRsrcParams));

			POBJDELETE(m_pBridgePartyIn);
		}
	}

	// Calculate remaining connected direction
	EMediaDirection eMediaDirectionConnected = eNoDirection;

	if (m_pBridgePartyIn)
		eMediaDirectionConnected |= eMediaIn;

	if (m_pBridgePartyOut)
		eMediaDirectionConnected |= eMediaOut;

	TRACEINTO << m_partyConfName << ", MediaDirectionConnected:" << eMediaDirectionConnected;

	// If OUT or IN exist then state will remain CONNECTED_STANDALONE
	if (eMediaDirectionConnected == eNoDirection)
		m_state = IDLE; // both direction disconnected

	// Inform Video Bridge about connected direction
	m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, VIDEO_BRIDGE_MSG, ENDDISCONNECTPARTY, statOK, FALSE, eMediaDirectionConnected);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeDisconnectIDLE(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;

	if (CPObject::IsValidPObjectPtr(m_pConfApi) && CPObject::IsValidPObjectPtr(m_pParty))
		m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, VIDEO_BRIDGE_MSG, ENDDISCONNECTPARTY, statOK, FALSE, eNoDirection);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeDisconnectSETUP(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoBridgeDisconnect(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeDisconnectDISCONNECTING(CSegment* pParams)
{
	TRACEINTO << m_partyConfName << " - Party is already disconnecting";
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeDisconnect(CSegment* pParams)
{
	BOOL IsVideoFullyDisconnect = FALSE;

	StartTimer(VIDEO_BRDG_PARTY_DISCONNECT_TOUT, VIDEO_BRDG_PARTY_DISCONNECT_TOUT_VALUE);

	switch (GetDisconnectingDirectionsReq())
	{
		case eMediaIn:
		{
			// Disconnect Video In
			if (m_pBridgePartyIn)   // If party out is connecting(SETUP) or connected(CONNECTED), this is not full disconnection of party cntl
			{
				if (!m_pBridgePartyOut || (m_pBridgePartyOut && !m_pBridgePartyOut->IsConnecting() && !m_pBridgePartyOut->IsConnected()))
					IsVideoFullyDisconnect = TRUE;

				// Incase we are in setup state and out direction is already connected
				// but the in direction is still in setup-->we will change the state to CONNECTED ,
				// stop the setup timer and won't send ack on connect to partyCntl only ack on disconnect
				if (m_pBridgePartyOut && m_pBridgePartyOut->IsConnected() && m_state == SETUP)
				{
					TRACEINTO << m_partyConfName << " - Change state to CONNECT";
					m_state = CONNECTED;
					DeleteTimer(VIDEO_BRDG_PARTY_SETUP_TOUT);
				}

				m_pBridgePartyIn->DisConnect();
			}
      else	//VNGFE-6525
      {
    	  TRACEINTO << "m_pBridgePartyIn does not exist - no need to disconnect (VIDEO_BRDG_PARTY_DISCONNECT_TOUT is not relevant --> deleted)";
    	  DeleteTimer(VIDEO_BRDG_PARTY_DISCONNECT_TOUT);
      }

			break;
		}

		case eMediaOut:
		{                         // Disconnect Video Out
			if (m_pBridgePartyOut)  // If party In in state of CONNECTED or SETUP , this is not full disconnection of party cntl
			{
				if (!m_pBridgePartyIn || (m_pBridgePartyIn && !m_pBridgePartyIn->IsConnecting() && !m_pBridgePartyIn->IsConnected()))
					IsVideoFullyDisconnect = TRUE;

				// Incase we are in setup state and IN direction is already connected
				// but the OUT direction is still in setup--> we will change the state to CONNECTED,
				// stop the setup timer and won't send ack on connect to partyCntl only ack on disconnect
				if (m_pBridgePartyIn && m_pBridgePartyIn->IsConnected() && m_state == SETUP)
				{
					TRACEINTO << m_partyConfName << " - Party-in is connected, change state to CONNECT";
					m_state = CONNECTED;
					DeleteTimer(VIDEO_BRDG_PARTY_SETUP_TOUT);
				}

				m_pBridgePartyOut->DisConnect();
			}
      else	//VNGFE-6525
      {
    	  TRACEINTO << "m_pBridgePartyOut does not exist - no need to disconnect (VIDEO_BRDG_PARTY_DISCONNECT_TOUT is not relevant --> deleted)";
    	  DeleteTimer(VIDEO_BRDG_PARTY_DISCONNECT_TOUT);
      }

			break;
		}

		case eMediaInAndOut:
		{                         // Disconnect Video In&Out
			if (m_pBridgePartyIn)
				m_pBridgePartyIn->DisConnect();

			if (m_pBridgePartyOut)
				m_pBridgePartyOut->DisConnect();

			IsVideoFullyDisconnect = TRUE;

      if (!m_pBridgePartyIn && !m_pBridgePartyOut)	//VNGFE-6525
      {
    	  TRACEINTO << "pBridgePartyIn & m_pBridgePartyOut do not exist - no need to disconnect (VIDEO_BRDG_PARTY_DISCONNECT_TOUT is not relevant --> deleted)";
    	  DeleteTimer(VIDEO_BRDG_PARTY_DISCONNECT_TOUT);
      }

			break;
		}

		default:
		{
      TRACEINTO << "no need to disconnect (VIDEO_BRDG_PARTY_DISCONNECT_TOUT is not relevant --> deleted)";
      DeleteTimer(VIDEO_BRDG_PARTY_DISCONNECT_TOUT);	//VNGFE-6525

			DBGPASSERT(1);
		}
	} // switch

	if (IsVideoFullyDisconnect)
	{
		if (m_state == SETUP)
			DeleteTimer(VIDEO_BRDG_PARTY_SETUP_TOUT);

		m_state = DISCONNECTING;
	}
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoOutDisconnectedSETUP(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoOutDisconnected(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoOutDisconnectedCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoOutDisconnected(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoOutDisconnectedDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoOutDisconnected(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoOutDisconnected(CSegment* pParam)
{
	CConfPartyRoutingTable* pRoutingTable = ::GetpConfPartyRoutingTable();
	PASSERT_AND_RETURN(!pRoutingTable);
	PASSERT_AND_RETURN(!m_pBridgePartyOut);
	PASSERT_AND_RETURN(!m_pBridgePartyOut->IsDisConnected());

	CRsrcParams* pRsrcParams = m_pBridgePartyOut->GetRsrcParams();
	PASSERT_AND_RETURN(!pRsrcParams);
	PASSERT_AND_RETURN(STATUS_FAIL == pRoutingTable->RemoveStateMachinePointerFromRoutingTbl(*pRsrcParams));

	VideoDisConnectionCompletion(pParam, eMediaOut);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoInDisconnectedSETUP(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoInDisconnected(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoInDisconnectedCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoInDisconnected(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoInDisconnectedDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoInDisconnected(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoInDisconnected(CSegment* pParam)
{
	CConfPartyRoutingTable* pRoutingTable = ::GetpConfPartyRoutingTable();
	PASSERT_AND_RETURN(!pRoutingTable);
	PASSERT_AND_RETURN(!m_pBridgePartyIn);
	PASSERT_AND_RETURN(!m_pBridgePartyIn->IsDisConnected());

	CRsrcParams* pRsrcParams = m_pBridgePartyIn->GetRsrcParams();
	PASSERT_AND_RETURN(!pRsrcParams);
	PASSERT_AND_RETURN(STATUS_FAIL == pRoutingTable->RemoveStateMachinePointerFromRoutingTbl(*pRsrcParams));

	VideoDisConnectionCompletion(pParam, eMediaIn);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeExportCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	m_bIsAfterMove = FALSE;

	if (m_pBridgePartyOut)
	{
		((CBridgePartyVideoOut*)m_pBridgePartyOut)->CleanAllLayouts();
		// retuen CBridgePartyVideoOut to state CONNECT
		((CBridgePartyVideoOut*)m_pBridgePartyOut)->Export();
	}

	OnVideoBridgeExportCONNECTED_STANDALONE(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeExportSETUP(CSegment* pParam)
{
	PASSERTSTREAM(1, "Illegal move, because party in SETUP, " << m_partyConfName);

	m_pConfApi->PartyBridgeResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, ENDEXPORTPARTY, statIllegal, FALSE);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeExportDISCONNECTING(CSegment* pParam)
{
	PASSERTSTREAM(1, "Illegal move, because party in DISCONNECTIN, " << m_partyConfName);

	m_pConfApi->PartyBridgeResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, ENDEXPORTPARTY, statIllegal, FALSE);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeExportCONNECTED_STANDALONE(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;

	CConfPartyRoutingTable* pRoutingTable = ::GetpConfPartyRoutingTable();
	PASSERT_AND_RETURN(!pRoutingTable);

	if (m_pBridgePartyIn)
	{
		((CBridgePartyVideoIn*)m_pBridgePartyIn)->Export();

		CRsrcParams* pRsrcParams = m_pBridgePartyIn->GetRsrcParams();
		PASSERT_AND_RETURN(!pRsrcParams);
		PASSERT_AND_RETURN(STATUS_FAIL == pRoutingTable->RemoveStateMachinePointerFromRoutingTbl(*pRsrcParams));
	}

	if (m_pBridgePartyOut)
	{
		CRsrcParams* pRsrcParams = m_pBridgePartyOut->GetRsrcParams();
		PASSERT_AND_RETURN(!pRsrcParams);
		PASSERT_AND_RETURN(STATUS_FAIL == pRoutingTable->RemoveStateMachinePointerFromRoutingTbl(*pRsrcParams));
	}

	m_pConfApi->PartyBridgeResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, ENDEXPORTPARTY, statOK, FALSE);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeUpdateVideoInParamsSETUP(CSegment* pParam)
{
	OnVideoBridgeUpdateVideoInParams(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeUpdateVideoInParamsCONNECTED(CSegment* pParam)
{
	OnVideoBridgeUpdateVideoInParams(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeUpdateVideoInParamsCONNECTED_STANDALONE(CSegment* pParam)
{
	OnVideoBridgeUpdateVideoInParams(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeUpdateVideoInParamsDISCONNECTING(CSegment* pParam)
{
	if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
	{
		if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams->GetMediaInParams()))
		{
			std::auto_ptr<CBridgePartyVideoParams> pBridgePartyVideoParams(new CBridgePartyVideoParams);
			pBridgePartyVideoParams->DeSerialize(NATIVE, *pParam);
			EStat receivedStatus = m_pUpdatePartyInitParams->UpdateVideoInParams(pBridgePartyVideoParams.get());

			// we send ack to party and bridge before we update the In params
			m_pConfApi->SendResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_IN_UPDATED, receivedStatus, FALSE);
		}
		else
		{
			TRACEINTO << m_partyConfName << " - Failed, MediaInParams is invalid";
			m_pConfApi->SendResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_IN_UPDATED, statIllegal, 0);
		}
	}
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeUpdateVideoInParams(CSegment* pParam)
{
	if (CPObject::IsValidPObjectPtr(m_pBridgePartyIn))
	{
		TRACEINTO << m_partyConfName << ", State:" << StateToString();

		std::auto_ptr<CBridgePartyVideoParams> pBridgePartyVideoParams(new CBridgePartyVideoParams);
		pBridgePartyVideoParams->DeSerialize(NATIVE, *pParam);
		((CBridgePartyVideoIn*)m_pBridgePartyIn)->UpdateVideoParams(pBridgePartyVideoParams.get());
	}
	else
	{
		TRACEINTO << m_partyConfName << ", State:" << StateToString() << " - Failed, pBridgePartyIn is invalid";
		m_pConfApi->SendResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_IN_UPDATED, statIllegal, 0);
	}
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeUpdateVideoOutParamsSETUP(CSegment* pParam)
{
	OnVideoBridgeUpdateVideoOutParams(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeUpdateVideoOutParamsCONNECTED(CSegment* pParam)
{
	OnVideoBridgeUpdateVideoOutParams(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeUpdateVideoOutParamsCONNECTED_STANDALONE(CSegment* pParam)
{
	OnVideoBridgeUpdateVideoOutParams(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeUpdateVideoOutParamsDISCONNECTING(CSegment* pParam)
{
	std::auto_ptr<CSegment> pSeg(new CSegment);

	BOOL IsExtParams = FALSE;

	if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
	{
		if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams->GetMediaOutParams()))
		{
			std::auto_ptr<CBridgePartyVideoParams> pBridgePartyVideoParams(new CBridgePartyVideoParams);
			pBridgePartyVideoParams->DeSerialize(NATIVE, *pParam);

			EStat receivedStatus = m_pUpdatePartyInitParams->UpdateVideoOutParams(pBridgePartyVideoParams.get());

			// If there is valid ack params we need to send with the response to party the Ack params.
			if (receivedStatus == statOK && pBridgePartyVideoParams->GetAckParams())
			{
				IsExtParams = TRUE;
				*pSeg << IsExtParams;

				CAckParams ackParams(*pBridgePartyVideoParams->GetAckParams());
				ackParams.Serialize(NATIVE, *pSeg);
			}
			else
			{
				*pSeg << IsExtParams;
			}

			// we send ack to party and bridge before we update the Out params
			m_pConfApi->SendResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_OUT_UPDATED, receivedStatus, FALSE, pSeg.get());
		}
		else
		{
			*pSeg << IsExtParams;
			TRACEINTO << m_partyConfName << " - Failed, MediaOutParams is invalid";
			m_pConfApi->SendResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_OUT_UPDATED, statIllegal, 0, pSeg.get());
		}
	}
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeUpdateVideoOutParams(CSegment* pParam)
{
	if (CPObject::IsValidPObjectPtr(m_pBridgePartyOut))
	{
		TRACEINTO << m_partyConfName << ", State:" << StateToString();

		CBridgePartyVideoParams params;
		params.DeSerialize(NATIVE, *pParam);

		((CBridgePartyVideoOut*)m_pBridgePartyOut)->UpdateVideoParams(&params);
	}
	else
	{
		TRACEINTO << m_partyConfName << ", State:" << StateToString() << " - Failed, 'pBridgePartyOut' is invalid";

		CSegment seg;
		seg << false; // no extra parameters

		m_pConfApi->SendResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_OUT_UPDATED, statIllegal, 0, &seg);
	}
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoInUpdatedSETUP(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoInUpdated(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoInUpdatedCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoInUpdated(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoInUpdatedCONNECTED_STANDALONE(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoInUpdated(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoInUpdatedDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << m_partyConfName << " - Ignored";
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoInUpdated(CSegment* pParam)
{
	EStat receivedStatus = statOK;
	*pParam >> (BYTE&)receivedStatus;

	// Inform Video Bridge
	m_pConfApi->SendResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_IN_UPDATED, receivedStatus, FALSE);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnPartyImageUpdatedSETUP(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnPartyImageUpdated(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnPartyImageUpdatedCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnPartyImageUpdated(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnPartyImageUpdatedCONNECTED_STANDALONE(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnPartyImageUpdated(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnPartyImageUpdatedDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << m_partyConfName << " - Ignored";
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnPartyImageUpdated(CSegment* pParam)
{
	EStat receivedStatus = statOK;
	*pParam >> (BYTE&)receivedStatus;

	// Inform Video Bridge
	m_pConfApi->SendResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, PARTY_IMAGE_UPDATED, receivedStatus, FALSE);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoOutUpdatedSETUP(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoOutUpdated(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoOutUpdatedCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoOutUpdated(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoOutUpdatedCONNECTED_STANDALONE(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoOutUpdated(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoOutUpdatedDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << m_partyConfName << " - Ignored";
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoOutUpdated(CSegment* pParam)
{
	EStat receivedStatus = statOK;
	BOOL  IsExtParams    = FALSE;

	*pParam >> (BYTE&)receivedStatus >> (BYTE&)IsExtParams;

	CSegment* pSeg       = new CSegment;

	if (receivedStatus == statOK && IsExtParams)
	{
		CAckParams ackParams;
		ackParams.DeSerialize(NATIVE, *pParam);

		// If there is valid ack params we need to send with the response to party the Ack params.
		IsExtParams = TRUE;
		*pSeg << (BYTE)IsExtParams;
		ackParams.Serialize(NATIVE, *pSeg);
	}
	else
		*pSeg << (BYTE)IsExtParams;

	// Inform Video Bridge
	m_pConfApi->SendResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_OUT_UPDATED, receivedStatus, FALSE, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoInVideoDecoderSyncChanged(CSegment* pParam)
{
	WORD isSynced = NO;
	*pParam >> isSynced;

	DWORD par     = LOCALVIDEO;
	par <<= 16;
	par  |= 1;
	m_pConfApi->UpdateDB(m_pParty, H221, par, isSynced);

	if (isSynced)
		m_pConfApi->VideoInSinched(m_pParty);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeFastUpdateSETUP(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoBridgeFastUpdate(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeFastUpdateCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoBridgeFastUpdate(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeFastUpdateCONNECTED_STANDALONE(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoBridgeFastUpdate(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeFastUpdate(CSegment* pParam)
{
	TRACECOND_AND_RETURN(!m_pBridgePartyOut, "Failed for OUT, " << m_partyConfName << ", State:" << StateToString());
	((CBridgePartyVideoOut*)m_pBridgePartyOut)->VideoRefresh();

	// AvctoSVC intra (for debug)
	// SendIntraToAvcToSvcIfExists();
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeAddImageCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoBridgeAddImage(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeAddImageSETUP(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoBridgeAddImage(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeAddImage(CSegment* pParam)
{
	CTaskApp* pAddedParty;

	*pParam >> (DWORD&)pAddedParty;
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pAddedParty));

	TRACECOND_AND_RETURN(!m_pBridgePartyOut, "Failed, " << m_partyConfName << ", State:" << StateToString());
	((CBridgePartyVideoOut*)m_pBridgePartyOut)->AddImage(pAddedParty);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeUpdateImage(CSegment* pParam)
{
	TRACECOND_AND_RETURN(!m_pBridgePartyOut, "Failed, " << m_partyConfName << ", State:" << StateToString());
	((CBridgePartyVideoOut*)m_pBridgePartyOut)->UpdateImage();
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeIndicationIconsChangeCONNECTED(CSegment* pParam)
{
	BOOL bUseSharedMemForIndicationIcon = FALSE;
	*pParam >> bUseSharedMemForIndicationIcon;

	if (!m_pBridgePartyOut)
	{
		TRACEINTO << m_partyConfName << ", State:" << StateToString() << " - Failed, 'm_pBridgePartyOut' is invalid";
		return;
	}

	((CBridgePartyVideoOut*)m_pBridgePartyOut)->UpdateIndicationIcons(bUseSharedMemForIndicationIcon);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeMuteImageSETUP(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoBridgeMuteImage(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeMuteImageCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoBridgeMuteImage(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeMuteImage(CSegment* pParam)
{
	TRACECOND_AND_RETURN(!m_pBridgePartyOut, "Failed, " << m_partyConfName << ", State:" << StateToString());
	((CBridgePartyVideoOut*)m_pBridgePartyOut)->MuteImage();
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeUnMuteImageSETUP(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoBridgeUnMuteImage(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeUnMuteImageCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoBridgeUnMuteImage(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeUnMuteImage(CSegment* pParam)
{
	TRACECOND_AND_RETURN(!m_pBridgePartyOut, "Failed, " << m_partyConfName << ", State:" << StateToString());
	((CBridgePartyVideoOut*)m_pBridgePartyOut)->UnMuteImage();
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeChangeSpeakersSETUP(CSegment* pParam)
{
	OnVideoBridgeChangeSpeakers(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeChangeSpeakersCONNECTED(CSegment* pParam)
{
	OnVideoBridgeChangeSpeakers(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeChangeSpeakers(CSegment* pParam)
{
	CTaskApp* pNewVideoSpeaker;
	CTaskApp* pNewAudioSpeaker;

	*pParam >>(DWORD&)pNewVideoSpeaker;
	*pParam >>(DWORD&)pNewAudioSpeaker;

	TRACECOND_AND_RETURN(!m_pBridgePartyOut, "Failed, " << m_partyConfName << ", State:" << StateToString());
	((CBridgePartyVideoOut*)m_pBridgePartyOut)->ChangeSpeakers(pNewVideoSpeaker, pNewAudioSpeaker);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeChangeAudioSpeakerSETUP(CSegment* pParam)
{
	OnVideoBridgeChangeAudioSpeaker(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeChangeAudioSpeakerCONNECTED(CSegment* pParam)
{
	OnVideoBridgeChangeAudioSpeaker(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeChangeAudioSpeaker(CSegment* pParam)
{
	CTaskApp* pNewSpeaker;
	*pParam >>(DWORD&)pNewSpeaker;

	TRACECOND_AND_RETURN(!m_pBridgePartyOut, "Failed, " << m_partyConfName << ", State:" << StateToString());
	((CBridgePartyVideoOut*)m_pBridgePartyOut)->ChangeAudioSpeaker(pNewSpeaker);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeDelImageSETUP(CSegment* pParam)
{
	OnVideoBridgeDelImage(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeDelImageCONNECTED(CSegment* pParam)
{
	OnVideoBridgeDelImage(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeDelImage(CSegment* pParam)
{
	CTaskApp* pDelParty;
	*pParam >> (DWORD&)pDelParty;

	DBGPASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pDelParty));

	TRACECOND_AND_RETURN(!m_pBridgePartyOut, "Failed, " << m_partyConfName << ", State:" << StateToString());
	((CBridgePartyVideoOut*)m_pBridgePartyOut)->DelImage(pDelParty);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeChangeConfLayoutSETUP(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoBridgeChangeConfLayout(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeChangeConfLayoutCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoBridgeChangeConfLayout(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeChangeConfLayout(CSegment* pParam)
{
	TRACECOND_AND_RETURN(!m_pBridgePartyOut, "Failed, " << m_partyConfName << ", State:" << StateToString());
	((CBridgePartyVideoOut*)m_pBridgePartyOut)->ChangeConfLayout(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeChangeLayoutOfTPRoomSublinkSETUP(CSegment* pParam)
{
	TRACEINTO << "partyId:" << GetPartyRsrcID() << ", " << m_partyConfName;
	OnVideoBridgeChangeLayoutOfTPRoomSublink(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeChangeLayoutOfTPRoomSublinkCONNECTED(CSegment* pParam)
{
	TRACEINTO << "partyId:" << GetPartyRsrcID() << ", " << m_partyConfName;
	OnVideoBridgeChangeLayoutOfTPRoomSublink(pParam);
}
//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeChangeLayoutOfTPRoomSublinkSTANDALONE(CSegment* pParam) //TEMP for debug
{
	TRACEINTO << "EMB_MLA_OLGA - partyId:" << GetPartyRsrcID() << ", " << m_partyConfName;
}
//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeChangeLayoutOfTPRoomSublink(CSegment* pParam)
{
	TRACECOND_AND_RETURN(!m_pBridgePartyOut, "Failed, " << m_partyConfName << ", State:" << StateToString());
	((CBridgePartyVideoOut*)m_pBridgePartyOut)->ChangeLayoutOfTPRoomSublink(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeChangePartyLayoutSETUP(CSegment* pParam)
{
	OnVideoBridgeChangePartyLayout(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeChangePartyLayoutCONNECTED(CSegment* pParam)
{
	OnVideoBridgeChangePartyLayout(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeChangePartyLayoutCONNECTED_STANDALONE(CSegment* pParam)
{
	PASSERTMSG(1, "Illegal State");
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeChangePartyLayoutDISCONNECTING(CSegment* pParam)
{
	PASSERTMSG(1, "Illegal State");
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeChangePartyLayout(CSegment* pParam)
{
	TRACECOND_AND_RETURN(!m_pBridgePartyOut, "Failed, " << m_partyConfName << ", State:" << StateToString());

	CVideoLayout newVideoLayout;
	newVideoLayout.DeSerialize(NATIVE, *pParam);
	((CBridgePartyVideoOut*)m_pBridgePartyOut)->ChangePartyLayout(newVideoLayout);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeChangePartyPrivateLayoutSETUP(CSegment* pParam)
{
	OnVideoBridgeChangePartyPrivateLayout(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeChangePartyPrivateLayoutCONNECTED(CSegment* pParam)
{
	OnVideoBridgeChangePartyPrivateLayout(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeChangePartyPrivateLayoutCONNECTED_STANDALONE(CSegment* pParam)
{
	PASSERTMSG(1, "Illegal State");
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeChangePartyPrivateLayout(CSegment* pParam)
{
	TRACECOND_AND_RETURN(!m_pBridgePartyOut, "Failed, " << m_partyConfName << ", State:" << StateToString());

	CVideoLayout newVideoLayout;
	newVideoLayout.DeSerialize(NATIVE, *pParam);
	((CBridgePartyVideoOut*)m_pBridgePartyOut)->ChangePartyPrivateLayout(newVideoLayout);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoOutVideoRefresh(CSegment* pParam)
{
	WORD ignore_filtering       = FALSE;
	WORD isRequestFromRemoteMGC = FALSE;
	DWORD  decoderMSSvcSsrcID = INVALID;
	DWORD  decoderMSSvcPriorityID   = INVALID;

	if (pParam != NULL)
	{
		*pParam >> ignore_filtering >> isRequestFromRemoteMGC >> decoderMSSvcSsrcID >> decoderMSSvcPriorityID;

		TRACEINTO << m_partyConfName
		          << ", IsIgnoreFiltering:"         << (int)ignore_filtering
		          << ", IsIntraSuppressEnabled:"    << (int)IsIntraSuppressEnabled(SUPPRESS_TYPE_ALL)
		          << ", IsRequestFromRemoteMGC:"    << (int)isRequestFromRemoteMGC
		          << ", IsIntraForLinksSuppressed:" << (int)m_isIntraForLinksSuppressed
		          << ", decoderMSSvcSsrcID: "       << (int)decoderMSSvcSsrcID
		          << ", decoderMSSvcPriorityID: "   << (int)decoderMSSvcPriorityID;


		if (ignore_filtering)
		{
			// Do nothing...
		}
		else if (IsIntraSuppressEnabled(SUPPRESS_TYPE_ALL) == false)
		{
			// Do nothing...
		}
		else if (isRequestFromRemoteMGC)
		{
			if (isRequestFromRemoteMGC)
			{
				if (m_isIntraForLinksSuppressed == TRUE)
				{
					// Requesting party is link to MGC and intra for links is supressed
					m_isIntraRequestReceivedFromWhileSuppressed = TRUE; // this flag indicates that when the timer will expire we will send new intra and activate the timer again
					return;
				}
				else
				{
					// Requesting party is link to MGC, intra requests from links will be ignored for next 10 seconds
					m_isIntraForLinksSuppressed = TRUE;

					DWORD IgnoreIntraDuration = 10;
					CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey("COP_ENCODER_IGNORE_INTRA_DURATION_IN_SECONDS", IgnoreIntraDuration);
					StartTimer(VIDEO_BRDG_COP_ENCODER_IGNORE_INTRA_TOUT, IgnoreIntraDuration*SECOND);
				}
			}
		}
	}

	m_pPartyApi->VideoRefresh(ignore_filtering, decoderMSSvcSsrcID, decoderMSSvcPriorityID);
}

// --------------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnTimerIgnoreIntraSETUP(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;;
	OnTimerIgnoreIntra();
}

// --------------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnTimerIgnoreIntraCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;;
	OnTimerIgnoreIntra();
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnTimerIgnoreIntra()
{
	m_isIntraForLinksSuppressed = FALSE;
	// will send intra and activate the filter mechanism again
	if (m_isIntraRequestReceivedFromWhileSuppressed == TRUE)
	{
		m_isIntraRequestReceivedFromWhileSuppressed = FALSE;
		m_pPartyApi->VideoRefresh();
	}
}

//--------------------------------------------------------------------------
BYTE CVideoBridgePartyCntl::IsPartyIntraSuppressed()
{
	BYTE ans = FALSE;
	if (IsIntraSuppressionSupportedForThisParty() == TRUE)
	{
		if (!m_IsPartyNoiseSuppressed)
		{
			DWORD timePeriodInSeconds             = 10;
			DWORD maxIntraRequestsPerInterval     = GetMaxIntraRequestsPerInterval();
			DWORD dwIntraRequestsSuppressDuration = GetIntraSuppressionDurationInSeconds();

			if (maxIntraRequestsPerInterval != 0)
			{
				DWORD currentTime = SystemGetTickCount().GetSeconds();
				DWORD gapBetweenIntras;
				DWORD_VECTOR::iterator itr = m_PartyIntraRequestsTime->begin();
				while (itr != m_PartyIntraRequestsTime->end())
				{
					gapBetweenIntras = currentTime- *itr;
					if (gapBetweenIntras > timePeriodInSeconds)
					{
						m_PartyIntraRequestsTime->erase(itr);
					}
					else
						itr++;
				}

				m_PartyIntraRequestsTime->push_back(currentTime);
				DWORD partyNumIntraRequestsInInterval = m_PartyIntraRequestsTime->size();
				if (partyNumIntraRequestsInInterval >= maxIntraRequestsPerInterval)
				{
					m_IsPartyNoiseSuppressed = TRUE;

					TRACEINTO << m_partyConfName << ", SuppressDuration:" << dwIntraRequestsSuppressDuration;
					StartTimer(VIDEO_BRDG_COP_PARTY_INTRA_SUPPRESS_TOUT, dwIntraRequestsSuppressDuration*SECOND);
					m_PartyIntraRequestsTime->clear();
					m_pConfApi->UpdateDB(m_pParty, PARTY_INTRA_SUPPRESS, m_IsPartyNoiseSuppressed);
				}
			}
		}
		else
		{
			ans = TRUE;
		}
	}

	TRACEINTO << m_partyConfName << ", IsPartyIntraSuppressed:" << ans;
	return ans;
}

//--------------------------------------------------------------------------
BOOL CVideoBridgePartyCntl::IsPartyValidForLayoutIndication()
{

	if (IsVideoRelayParty())	// not displaying ICON on SVC EP (currently)
 		return FALSE;

	if(GetCascadeLinkMode() != NONE)
		return FALSE;

	CVideoBridge* pBridge = (CVideoBridge*)GetBridge();
	CTelepresenceLayoutMngr* pTeleLayoutMngr = NULL;
	if(pBridge)
		pTeleLayoutMngr = pBridge->GetTelepresenceLayoutMngr();

	if (pTeleLayoutMngr)
	{
		const char* mainScreenPartyName = pTeleLayoutMngr->GetRoomInfo(m_telepresenceInfo.GetRoomID()).GetMainLinkPartyName();
		if(strcmp(mainScreenPartyName, GetITPSiteName()) != 0)
		{
			TRACEINTO<<"Party: "<<GetITPSiteName()<<" is not the ITP main screen, only show indication for main screen";
			return FALSE;

		}
	}

	return TRUE;

}


//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnTimerPartyIntraSuppressed(CSegment* pParams)
{
	TRACEINTO << m_partyConfName << ", State:" << StateToString();

	m_IsPartyNoiseSuppressed = FALSE;
	m_pConfApi->UpdateDB(m_pParty, PARTY_INTRA_SUPPRESS, m_IsPartyNoiseSuppressed);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoInSendH239Caps(CSegment* pParam)
{
	m_pPartyApi->SendH239VideoCaps();
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgePLC_SetPrivateLayoutTypeCONNECTED(CSegment* pParam)
{
	TRACECOND_AND_RETURN(!m_pBridgePartyOut, "Failed, " << m_partyConfName << ", State:" << StateToString());

	BYTE newPrivateLayoutType;
	*pParam >> newPrivateLayoutType;

	TRACEINTO << m_partyConfName << ", State:" << StateToString() << ", Layout:" << LayoutTypeAsString[newPrivateLayoutType];
	((CBridgePartyVideoOut*)m_pBridgePartyOut)->PLC_SetPartyPrivateLayoutType((LayoutType)newPrivateLayoutType);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgePLC_ForceCellCONNECTED(CSegment* pParam)
{
	TRACECOND_AND_RETURN(!m_pBridgePartyOut, "Failed, " << m_partyConfName << ", State:" << StateToString());

	char forcedImagePartyName[H243_NAME_LEN];
	*pParam >> forcedImagePartyName;
	forcedImagePartyName[H243_NAME_LEN-1] = '\0';
	BYTE cellToForce = 0;
	*pParam >> cellToForce;

	TRACEINTO << m_partyConfName << ", State:" << StateToString() << ", forcedImagePartyName:" << forcedImagePartyName << ", cellToForce:" << (int)cellToForce;
	((CBridgePartyVideoOut*)m_pBridgePartyOut)->PLC_ForceToCell(forcedImagePartyName, cellToForce);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgePLC_CancelAllPrivateLayoutForcesCONNECTED(CSegment* pParam)
{
	TRACECOND_AND_RETURN(!m_pBridgePartyOut, "Failed, " << m_partyConfName << ", State:" << StateToString());

	((CBridgePartyVideoOut*)m_pBridgePartyOut)->PLC_CancelAllPrivateLayoutForces();
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgePLC_ReturnToConfLayoutCONNECTED(CSegment* pParam)
{
	TRACECOND_AND_RETURN(!m_pBridgePartyOut, "Failed, " << m_partyConfName << ", State:" << StateToString());

	((CBridgePartyVideoOut*)m_pBridgePartyOut)->PLC_ReturnToConfLayout();
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeSetPrivateLayoutOnOffSETUP(CSegment* pParam)
{
	OnVideoBridgeSetPrivateLayoutOnOff(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeSetPrivateLayoutOnOffCONNECTED(CSegment* pParam)
{
	OnVideoBridgeSetPrivateLayoutOnOff(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeSetPrivateLayoutOnOffCONNECTED_STANDALONE(CSegment* pParam)
{
	PASSERTMSG(1, "Illegal State");
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeSetPrivateLayoutOnOff(CSegment* pParam)
{
	TRACECOND_AND_RETURN(!m_pBridgePartyOut, "Failed, " << m_partyConfName << ", State:" << StateToString());

	DWORD isPrivate;
	*pParam >> isPrivate;

	((CBridgePartyVideoOut*)m_pBridgePartyOut)->ChangeLayoutPrivatePartyButtonOnly(isPrivate);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeChangeVisualEffectsSETUP(CSegment* pParam)
{
	OnVideoBridgeChangeVisualEffects(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeChangeVisualEffectsCONNECTED(CSegment* pParam)
{
	OnVideoBridgeChangeVisualEffects(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeChangeVisualEffectsCONNECTED_STANDALONE(CSegment* pParam)
{
	OnVideoBridgeChangeVisualEffects(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeChangeVisualEffects(CSegment* pParam)
{
	TRACECOND_AND_RETURN(!m_pBridgePartyOut, "Failed, " << m_partyConfName << ", State:" << StateToString());

	CVisualEffectsParams visualEffects;
	visualEffects.DeSerialize(NATIVE, *pParam);

	BYTE bInternalUpdateOnly = FALSE;
	*pParam >> bInternalUpdateOnly;
	if(m_pBridgePartyOut)
		((CBridgePartyVideoOut*)m_pBridgePartyOut)->UpdateVisualEffects(&visualEffects, bInternalUpdateOnly);
	if(m_pBridgePartyIn)
		((CBridgePartyVideoIn*)m_pBridgePartyIn)->UpdateVisualEffects(&visualEffects, bInternalUpdateOnly);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeUpdatePartyLectureModeRoleSETUP(CSegment* pParam)
{
	OnVideoBridgeUpdatePartyLectureModeRole(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeUpdatePartyLectureModeRoleCONNECTED(CSegment* pParam)
{
	OnVideoBridgeUpdatePartyLectureModeRole(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeUpdatePartyLectureModeRoleCONNECTED_STANDALONE(CSegment* pParam)
{
	OnVideoBridgeUpdatePartyLectureModeRole(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeUpdatePartyLectureModeRoleDISCONNECTING(CSegment* pParam)
{
	BYTE tmp;
	ePartyLectureModeRole newPartyLectureModeRole = eREGULAR;

	*pParam >> tmp;

	newPartyLectureModeRole = (ePartyLectureModeRole)tmp;
	if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
	{
		if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams->GetMediaOutParams()))
		{
			m_pUpdatePartyInitParams->UpdateLectureModeRole(newPartyLectureModeRole);
		}
		else
		{
			TRACEINTO << m_partyConfName << " - Failed, MediaOutParams is invalid";
		}
	}
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeUpdatePartyLectureModeRole(CSegment* pParam)
{
	TRACECOND_AND_RETURN(!m_pBridgePartyOut, "Failed, " << m_partyConfName << ", State:" << StateToString());

	BYTE partyLectureModeRole;
	*pParam >> partyLectureModeRole;

	if (m_state == DISCONNECTING)
	{
		PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams));
		PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams->GetMediaOutParams()));
		m_pUpdatePartyInitParams->UpdateLectureModeRole((ePartyLectureModeRole)partyLectureModeRole);
		return;
	}

	((CBridgePartyVideoOut*)m_pBridgePartyOut)->UpdateLectureModeRole((ePartyLectureModeRole)partyLectureModeRole);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeDeletePartyFromConfSETUP(CSegment* pParam)
{
	OnVideoBridgeDeletePartyFromConf(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeDeletePartyFromConfCONNECTED(CSegment* pParam)
{
	OnVideoBridgeDeletePartyFromConf(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeDeletePartyFromConf(CSegment* pParam)
{
	TRACECOND_AND_RETURN(!m_pBridgePartyOut, "Failed, " << m_partyConfName << ", State:" << StateToString());

	char deletedPartyName[H243_NAME_LEN];
	*pParam >> deletedPartyName;
	deletedPartyName[H243_NAME_LEN-1] = '\0';

	((CBridgePartyVideoOut*)m_pBridgePartyOut)->DeletePartyFromConf(deletedPartyName);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoOutPartyLayoutChanged(CSegment* pParam)
{
	TRACEINTO << m_partyConfName << ", State:" << StateToString();

	CSegment vidLayoutSeg;
	if (!GetCurrentLayout()->Serialize(PARTY_lev, &vidLayoutSeg))
	{
		DWORD isUpdateLayoutAsPrivateInDB = IsUpdateLayoutAsPrivateInDB();
		m_pConfApi->UpdateDB(m_pParty, CPPARTYLAYOUT, isUpdateLayoutAsPrivateInDB, 0, &vidLayoutSeg);
		m_pConfApi->UpdateDB(m_pParty, PRIVATEON, isUpdateLayoutAsPrivateInDB);
	}
	else
	{
		PASSERT(1);
	}

	CTaskApp*     pSouceInCellZero = NULL;
	CVidSubImage* subImageCellZero = (*GetCurrentLayout())[0];
	DWORD         partyRscId       = (subImageCellZero) ? subImageCellZero->GetImageId() : 0;
	if (partyRscId)
	{
		CImage* pImage = GetPartyImageLookupTable()->GetPartyImage(partyRscId);
		PASSERTSTREAM(!pImage, "Failed, The lookup table doesn't have an element, PartyId:" << partyRscId);

		pSouceInCellZero = (pImage) ? (CTaskApp*)pImage->GetVideoSource() : NULL;
	}

	if (pSouceInCellZero == NULL)
		TRACEINTO << m_partyConfName << ", State:" << StateToString() << ", pSouceInCellZero:NULL";

	// in PCM when going out of fecc menu - we have to update the party
	// about the image in cell 0, even if it is the same as m_pPartySouceInCellZero
	BYTE alwaysSendVin = FALSE;
	if (pParam)
		*pParam >> alwaysSendVin;

	if ((m_pPartySouceInCellZero != pSouceInCellZero || alwaysSendVin) && pSouceInCellZero != NULL)
	{
		m_pPartySouceInCellZero = pSouceInCellZero;
		m_pConfApi->PartyVideoBridgeImageInCellZeroChanged(VIDEO_BRIDGE_MSG, m_pParty, m_pPartySouceInCellZero);
	}
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoOutPrivateLayoutOnOffChanged(CSegment* pParam)
{
	WORD isPrivateLayoutOn = NO;
	*pParam >> isPrivateLayoutOn;

	m_pConfApi->UpdateDB(m_pParty, PRIVATEON, isPrivateLayoutOn);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeShowSlideCONNECTED_STANDALONE(CSegment* pParam)
{
	TRACECOND_AND_RETURN(!m_pBridgePartyOut, "CVideoBridgePartyCntl::OnVideoBridgeShowSlideCONNECTED_STANDALONE - Failed, " << m_partyConfName << ", State:" << StateToString());
	//  // getting the party ID
	//    CIVRPlayMessage*       pIVRPlayMessage = new CIVRPlayMessage;
	//    SIVRPlayMessageStruct* pPlayMsg        = &(pIVRPlayMessage->play);
	//    memset((char*)pPlayMsg, 0, sizeof(SIVRPlayMessageStruct));

//	if ((GetNetworkInterface() == SIP_INTERFACE_TYPE) && (m_pBridge->GetConf()->GetCommConf()->isExternalIvrControl()))
//	{
//		CPartyCntl* pPartyCntl = (m_pBridge->GetConf())->GetPartyCntl(GetName());
//		CParty * pParty = reinterpret_cast<CParty*>(pPartyCntl->GetPartyTaskApp());	  ;
//		CIvrCntlExternal *ext_ivr_cntl = reinterpret_cast<CIvrCntlExternal*>(pParty->GetIvrCntl());
//		CIVRPlayMessage* pIVRPlayMessage = new CIVRPlayMessage;
//		pIVRPlayMessage->DeSerialize(pParam);
//		std::string slide_filename;
//		WORD duration = 0;
//		ext_ivr_cntl->GetRequestedExternalFile(pIVRPlayMessage->play.mediaType, slide_filename, duration);
//		SAFE_COPY( pIVRPlayMessage->play.mediaFiles[0].fileName, slideName);
//		pParam->Reset();
//		pIVRPlayMessage->Serialize(pParam);
//	}
	//CConfParty* pConfParty = pCommConf->GetCurrentParty(GetName());

	((CBridgePartyVideoOut*)m_pBridgePartyOut)->ShowSlide(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeShowSlideCONNECTED(CSegment* pParam)
{
	TRACECOND_AND_RETURN(!m_pBridgePartyOut, "CVideoBridgePartyCntl::OnVideoBridgeShowSlideCONNECTED - Failed, " << m_partyConfName << ", State:" << StateToString());
	TRACECOND_AND_RETURN(!m_bIsResumingIVR, "invalid state - not in resume call flow , " << m_partyConfName << ", State:" << StateToString());
	//**** save in and out states for resuming when joining conf after the resumed ivr is ended.
	WORD video_out_state_after_resume = m_pBridgePartyOut->GetState();
	BOOL is_video_out_resumed = video_out_state_after_resume == CBridgePartyVideoOut::CONNECTED || video_out_state_after_resume == CBridgePartyVideoOut::CHANGE_LAYOUT;
	TRACECOND_AND_RETURN(!is_video_out_resumed, "BridgePartyVideoOut is in invalid state: " << ((CBridgePartyVideoOut*)m_pBridgePartyOut)->GetStateAsString(video_out_state_after_resume) << ", party name: " << m_partyConfName );
	((CBridgePartyVideoOut*)m_pBridgePartyOut)->ShowSlideForIvrResume(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeStopShowSlideCONNECTED_STANDALONE(CSegment* pParam)
{
	TRACECOND_AND_RETURN(!m_pBridgePartyOut, "Failed, " << m_partyConfName << ", State:" << StateToString());
	((CBridgePartyVideoOut*)m_pBridgePartyOut)->StopShowSlide(pParam);
}



//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeStopShowSlideCONNECTED(CSegment* pParam)
{
	TRACECOND_AND_RETURN(!m_pBridgePartyOut, "Failed, " << m_partyConfName << ", State:" << StateToString());
	TRACECOND_AND_RETURN(!m_bIsResumingIVR, "invalid state - not in resume call flow , " << m_partyConfName << ", State:" << StateToString());
	((CBridgePartyVideoOut*)m_pBridgePartyOut)->StopShowSlideForIvrResume(pParam);

}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeJoinConfCONNECTED_STANDALONE(CSegment* pParam)         // IVR_JOIN_CONF_VIDEO
{
	if (m_pBridgePartyIn)
	{
		// Turn on the l-video-sync-loss flag
		TRACEINTO << m_partyConfName << ", State:" << StateToString() << " - Turn-on the l-video-sync-loss";
		m_pConfApi->UpdateDB(m_pParty, H221, LOCALVIDEO << 16, 1);
	}
	else
	{
		TRACEINTO << m_partyConfName << ", State:" << StateToString() << " - Start setup process";
	}

	Setup();
}
//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeJoinConfCONNECTED(CSegment* pParam)         // IVR_JOIN_CONF_VIDEO - after "resume" during IVR
{
	BOOL isAfterIvrResume =  GetPartyResumeFromHoldInIVR();

	PASSERTMSG_AND_RETURN(!isAfterIvrResume, " unexpected CAM message for 'join conf' during CONNECTED state.")
	BOOL isVideoInSynced = ((CBridgePartyVideoIn*)m_pBridgePartyIn)->GetLastSyncStatus() == STATUS_OK;
	TRACEINTO << "IVR_JOIN_CONF_VIDEO in CONNECTED state, is resuming after IVR-" << (int)isAfterIvrResume << ", is video in synced: " << (int)isVideoInSynced;
	if (isVideoInSynced)
	{
		CSegment cseg;
		cseg << GetPartyRsrcID();
		// Inform Video Bridge of IVR completion by party, after CAM sent this message for ending IVR
		m_pConfApi->PartyBridgeResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, END_PARTY_IVR_MODE_ON_RESUME, statOK, FALSE, eNoDirection, &cseg);
		TRACEINTO << " resetting m_bIsResumingIVR";
		m_bIsResumingIVR = FALSE;
	}
}
//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeJoinConfIDLE(CSegment* pParam)         // IVR_JOIN_CONF_VIDEO
{
	TRACEINTO << "IVR_JOIN_CONF_VIDEO in IDLE state";
}


//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeStartPLCCONNECTED(CSegment* pParam)
{
	TRACECOND_AND_RETURN(!m_pBridgePartyOut, "Failed, " << m_partyConfName << ", State:" << StateToString());
	((CBridgePartyVideoOut*)m_pBridgePartyOut)->StartPLC(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeStopPLCCONNECTED(CSegment* pParam)
{
	TRACECOND_AND_RETURN(!m_pBridgePartyOut, "Failed, " << m_partyConfName << ", State:" << StateToString());
	((CBridgePartyVideoOut*)m_pBridgePartyOut)->StopPLC(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnTimerPartySetupSETUP(CSegment* pParams)
{
	TRACESTRFUNC(eLevelError) << m_partyConfName;
	OnTimerPartySetup(pParams);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnTimerPartySetupCONNECTED(CSegment* pParams)
{
	TRACESTRFUNC(eLevelError) << m_partyConfName;
	OnTimerPartySetup(pParams);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnTimerPartySetup(CSegment* pParams)
{
	CMedString encoderString, decoderString;
	CMedString logStr;
	logStr << m_partyConfName << ", State:" << StateToString();

	if (CPObject::IsValidPObjectPtr(m_pBridgePartyOut))
	{
		logStr << "\n  CBridgePartyVideoOut State: ";
		((CBridgePartyVideoOut*)m_pBridgePartyOut)->DumpAllInfoOnConnectionState(&logStr);
		((CBridgePartyVideoOut*)m_pBridgePartyOut)->DumpAllInfoOnConnectionState(&encoderString, true);
	}

	if (CPObject::IsValidPObjectPtr(m_pBridgePartyIn))
	{
		logStr << "\n  CBridgePartyVideoIn State : ";
		((CBridgePartyVideoIn*)m_pBridgePartyIn)->DumpAllInfoOnConnectionState(&logStr);
		((CBridgePartyVideoIn*)m_pBridgePartyIn)->DumpAllInfoOnConnectionState(&decoderString, true);
	}

	TRACESTRFUNC(eLevelError) << logStr.GetString();

	// for debug info in case of the "MCU internal problem"
	BYTE failureCauseDirection = eMipNoneDirction;
	BYTE failureCauseAction    = eMipNoAction;
	GetRsrcProbAdditionalInfoOnVideoTimerSetup(failureCauseDirection, failureCauseAction);

	CSegment* pSeg = new CSegment;
	*pSeg << (BYTE)statVideoInOutResourceProblem << (BYTE)failureCauseDirection << (BYTE)eMipTimer << (BYTE)failureCauseAction;
	DumpMcuInternalProblemDetailed(failureCauseDirection, eMipTimer, eMipVideo);

	// Inform Video Bridge of statVideoInOutResourceProblem -> Inorder to initiate Kill port on VideoEncoder+Decoder
	m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, VIDEO_BRIDGE_MSG, ENDCONNECTPARTY, statVideoInOutResourceProblem, FALSE, eNoDirection, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnTimerPartyDisconnectSETUP(CSegment* pParams)
{
	TRACEINTO << m_partyConfName;
	OnTimerPartyDisconnect(pParams);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnTimerPartyDisconnectCONNECTED(CSegment* pParams)
{
	TRACEINTO << m_partyConfName;
	OnTimerPartyDisconnect(pParams);
}

//--------------------------------------------------------------------------
// In case of unidirection - one direction is still connected and one direction is disconnecting.
void CVideoBridgePartyCntl::OnTimerPartyDisconnect(CSegment* pParams)
{
	TRACEINTO << m_partyConfName << ", State:" << StateToString();

	EMediaDirection eConnectedDirection   = eNoDirection;
	BYTE failureCauseDirection = eMipNoneDirction;

	// We know that in this case one direction must be connected or in setup state.
	if (GetDisconnectingDirectionsReq() == eMediaOut)
	{
		eConnectedDirection   = eMediaIn;
		failureCauseDirection = eMipOut;  // for debug info in case of the "MCU internal problem"
	}

	if (GetDisconnectingDirectionsReq() == eMediaIn)
	{
		eConnectedDirection   = eMediaOut;
		failureCauseDirection = eMipIn;   // for debug info in case of the "MCU internal problem"
	}

	RemoveResourcesFromRoutingTable();

	CSegment* pSeg = new CSegment;
	*pSeg << (BYTE)statVideoInOutResourceProblem << (BYTE)failureCauseDirection << (BYTE)eMipTimer << (BYTE)eMipClose;

	if (GetConnectionFailureCause() == statOK)
		DumpMcuInternalProblemDetailed(failureCauseDirection, eMipTimer, eMipVideo);

	// We need to delete partyIn/partyOut incase of disconnect timeout failure
	DestroyPartyInOut(eConnectedDirection);

	CParty *pParty = GetLookupTableParty()->Get(m_partyRsrcID); // for BRIDGE-10762
	PASSERTMSG_AND_RETURN(pParty == NULL, "CVideoBridgePartyCntl::OnTimerPartyDisconnect - Error: Party not exists in lookup table!");

	// Inform Video Bridge of statVideoInOutResourceProblem -> Inorder to initiate Kill port on AudioEncoder+Decoder
	m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, VIDEO_BRIDGE_MSG, ENDDISCONNECTPARTY, statVideoInOutResourceProblem, FALSE, eConnectedDirection, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnTimerPartyDisconnectDISCONNECTING(CSegment* pParams)
{
	TRACEINTO << m_partyConfName << ", State:" << StateToString();

	m_state = IDLE;

	// for debug info in case of the "MCU internal problem"
	BYTE failureCauseDirection = eMipNoneDirction;
	if (CPObject::IsValidPObjectPtr(m_pBridgePartyIn) && !m_pBridgePartyIn->IsDisConnected())
		failureCauseDirection = eMipIn;
	else
		failureCauseDirection = eMipOut;

	PASSERT(!CPObject::IsValidPObjectPtr(m_pBridgePartyIn) && !CPObject::IsValidPObjectPtr(m_pBridgePartyOut));

	RemoveResourcesFromRoutingTable();

	std::auto_ptr<CSegment> pSeg(new CSegment);
	*pSeg << (BYTE)statVideoInOutResourceProblem << (BYTE)failureCauseDirection << (BYTE)eMipTimer << (BYTE)eMipClose;

	if (GetConnectionFailureCause() == statOK)
		DumpMcuInternalProblemDetailed(failureCauseDirection, eMipTimer, eMipVideo);

	CParty *pParty = GetLookupTableParty()->Get(m_partyRsrcID); // for BRIDGE-10762
	PASSERTMSG_AND_RETURN(pParty == NULL, "CVideoBridgePartyCntl::OnTimerPartyDisconnectDISCONNECTING - Error: Party not exists in lookup table!");

	// Inform Video Bridge of statVideoInOutResourceProblem -> Inorder to initiate Kill port on VideoEncoder+Decoder
	m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, VIDEO_BRIDGE_MSG, ENDDISCONNECTPARTY, statVideoInOutResourceProblem, FALSE, eNoDirection, pSeg.get());
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::SetSiteName(const char* visualName)
{
	if (visualName != NULL)
	{
		// SetITPSiteName(visualName);
		CSegment* pSeg = new CSegment;
		*pSeg << visualName;
		DispatchEvent(SET_SITE_AND_VISUAL_NAME, pSeg);
		POBJDELETE(pSeg);
	}
}
//--------------------------------------------------------------------------
const char* CVideoBridgePartyCntl::GetSiteName()
{
	if (m_pBridgePartyIn)
		return ((CBridgePartyVideoIn*)m_pBridgePartyIn)->GetSiteName();
	else
		return GetName();
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::SetITPSiteName(const char* visualName)
{
	strcpy_safe(m_ITPSiteName, visualName);
}

//--------------------------------------------------------------------------
const char* CVideoBridgePartyCntl::GetITPSiteName()
{
	if (strlen(m_ITPSiteName) > 0)
		return m_ITPSiteName;
	else
		return GetName();
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnSetSiteNameConnected(CSegment* pParam)
{
	char visualName[H243_NAME_LEN];
	*pParam >> visualName;
	visualName[H243_NAME_LEN-1] = '\0';

	TRACEINTO << m_partyConfName << ", SiteName:" << visualName;

	SetITPSiteName(visualName);

	if (m_pBridgePartyIn)
		((CBridgePartyVideoIn*)m_pBridgePartyIn)->SetSiteName(visualName);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnSetSiteNameDISCONNECTING(CSegment* pParams)
{
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::UpdateDecoderDetectedMode()
{
	TRACECOND_AND_RETURN(!m_pBridgePartyOut, "Failed, " << m_partyConfName << ", State:" << StateToString());

	const CImage* pPartyImage = (m_pBridgePartyIn) ? ((CBridgePartyVideoIn*)m_pBridgePartyIn)->GetPartyImage() : NULL;
	TRACECOND_AND_RETURN(!pPartyImage, "Failed, " << m_partyConfName << ", State:" << StateToString());

	DWORD newDecoderDetectedModeWidth               = pPartyImage->GetDecoderDetectedModeWidth();
	DWORD newDecoderDetectedModeHeight              = pPartyImage->GetDecoderDetectedModeHeight();
	DWORD newDecoderDetectedSampleAspectRatioWidth  = pPartyImage->GetDecoderDetectedSampleAspectRatioWidth();
	DWORD newDecoderDetectedSampleAspectRatioHeight = pPartyImage->GetDecoderDetectedSampleAspectRatioHeight();

	CBridgePartyVideoOut* pPartyVideoOut            = (CBridgePartyVideoOut*)(m_pBridgePartyOut);
	DWORD curDecoderDetectedModeWidth               = pPartyVideoOut->GetDecoderDetectedModeWidth();
	DWORD curDecoderDetectedModeHeight              = pPartyVideoOut->GetDecoderDetectedModeHeight();
	DWORD curDecoderDetectedSampleAspectRatioWidth  = pPartyVideoOut->GetDecoderDetectedSampleAspectRatioWidth();
	DWORD curDecoderDetectedSampleAspectRatioHeight = pPartyVideoOut->GetDecoderDetectedSampleAspectRatioHeight();

	if ((curDecoderDetectedModeWidth != newDecoderDetectedModeWidth) ||
	    (curDecoderDetectedModeHeight != newDecoderDetectedModeHeight) ||
	    (curDecoderDetectedSampleAspectRatioWidth != newDecoderDetectedSampleAspectRatioWidth) ||
	    (curDecoderDetectedSampleAspectRatioHeight != newDecoderDetectedSampleAspectRatioHeight))
	{
		TRACEINTO << m_partyConfName << ", State:" << StateToString();
		((CBridgePartyVideoOut*)(m_pBridgePartyOut))->UpdateDecoderDetectedMode(newDecoderDetectedModeWidth, newDecoderDetectedModeHeight, newDecoderDetectedSampleAspectRatioWidth, newDecoderDetectedSampleAspectRatioHeight);

		// check for VNGR-21714 scenario...
		// Inform Video Bridge
		// Bridge-11406
		CSegment* pSeg = new CSegment;
		// first adding a flag for "extra params" then the value for "resolution only update"
		*pSeg << (BYTE)FALSE;
		m_pConfApi->SendResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, PARTY_IMAGE_UPDATED, STATUS_OK, FALSE, pSeg);
		POBJDELETE(pSeg);
	}
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::ResyncVideoSource()
{
	TRACEINTO << m_partyConfName << ", State:" << StateToString() << ", Resync:" << m_resync;

	if (m_resync != 1)
	{
		// Party not marked as new video source (ask for intra anyway)
		// fix for vngr-3756 - exception in ConfParty
		return;
	}

	m_resync = 0;

	PASSERT_AND_RETURN(!m_pBridgePartyIn);

	((CBridgePartyVideoIn*)m_pBridgePartyIn)->ReSync();
}

//--------------------------------------------------------------------------
BYTE CVideoBridgePartyCntl::IsWaitingForChangeLayoutAck()
{
	TRACECOND_AND_RETURN_VALUE(!m_pBridgePartyOut, "Failed, " << m_partyConfName << ", State:" << StateToString(), 0);
	return ((CBridgePartyVideoOut*)m_pBridgePartyOut)->IsInswitch();
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoOutEndChangeLayout(CSegment* pParam)
{
	CSegment vidLayoutSeg;
	if (!GetCurrentLayout()->Serialize(PARTY_lev, &vidLayoutSeg))
		m_pConfApi->UpdateDB(m_pParty, CPPARTYLAYOUT, 0, 0, &vidLayoutSeg);
	else
		DBGPASSERT(1);

	DWORD receivedStatus = 0;
	*pParam >> receivedStatus;

	TRACEINTO << m_partyConfName << ", State:" << StateToString() << ", Status:" << receivedStatus;

	m_pConfApi->PartyBridgeResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, END_CHANGE_LAYOUT, receivedStatus, FALSE);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoInUpdateDecoderDetectedModeCONNECTED(CSegment* pParam)
{
	UpdateDecoderDetectedMode();
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::GetRsrcProbAdditionalInfoOnVideoTimerSetup(BYTE& failureCauseDirection, BYTE& failureCauseAction)
{
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(m_pBridgePartyIn) && !CPObject::IsValidPObjectPtr(m_pBridgePartyOut));

	failureCauseDirection = eMipNoneDirction;
	failureCauseAction    = eMipNoAction;

	if (CPObject::IsValidPObjectPtr(m_pBridgePartyIn) && !m_pBridgePartyIn->IsConnected())
	{
		failureCauseDirection = eMipIn;
		if (!((CBridgePartyVideoIn*)m_pBridgePartyIn)->IsAckOnOpenPort())
			failureCauseAction = eMipOpen;
		else
			failureCauseAction = eMipConnect;
	}
	else
	{
		failureCauseDirection = eMipOut;
		if (CPObject::IsValidPObjectPtr(m_pBridgePartyOut) && !((CBridgePartyVideoOut*)m_pBridgePartyOut)->IsPortOpenedOn())
			failureCauseAction = eMipConnect;
		else
			failureCauseAction = eMipOpen;
	}
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeDisplayTextOnScreenCONNECTED(CSegment* pParam)
{
	CTextOnScreenMngr TextMsgList;
	DWORD             timeout = 0;
	TextMsgList.DeSerialize(NATIVE, *pParam);
	*pParam >> timeout;

	TRACECOND_AND_RETURN(!m_pBridgePartyOut, "Failed, " << m_partyConfName << ", State:" << StateToString());
	((CBridgePartyVideoOut*)m_pBridgePartyOut)->DisplayPartyTextOnScreen(TextMsgList, timeout);
}

//--------------------------------------------------------------------------
bool CVideoBridgePartyCntl::IsBridgePartyVideoOutStateIsConnected()
{
	if (!m_pBridgePartyOut)
		return false;

	return ((CBridgePartyVideoOut*)m_pBridgePartyOut)->IsStateIsConnected();
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::DisplayGatheringOnScreen(CGathering* pGathering)
{
	if (m_pBridgePartyOut)
		((CBridgePartyVideoOut*)m_pBridgePartyOut)->DisplayGathering(pGathering);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::SetVisualEffectsParams(CVisualEffectsParams* pVisualEffects)
{
	if (m_pBridgePartyOut)
		((CBridgePartyVideoOut*)m_pBridgePartyOut)->SetVisualEffectsParams(pVisualEffects);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::UpdateAutoScanOrder(CAutoScanOrder* pAutoScanOrder)
{
	if (m_pBridgePartyOut)
		((CBridgePartyVideoOut*)m_pBridgePartyOut)->UpdateAutoScanOrder(pAutoScanOrder);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::UpdateMessageOverlay(CMessageOverlayInfo* pMessageOverlayInfo)
{
	if (m_pBridgePartyOut)
		((CBridgePartyVideoOut*)m_pBridgePartyOut)->UpdateMessageOverlay(pMessageOverlayInfo);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::UpdateMessageOverlayStop() // VNGR-15750
{
	if (m_pBridgePartyOut)
		((CBridgePartyVideoOut*)m_pBridgePartyOut)->UpdateMessageOverlayStop();
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::StartMessageOverlay(CMessageOverlayInfo* pMessageOverlayInfo)
{
	// added by huiyu.
	CSegment* pSeg = new CSegment;
	pMessageOverlayInfo->Serialize(NATIVE, *pSeg);
	StartTimer(MESSAGE_OVERLAY_TIMER, MESSAGE_OVERLAY_TIMER_OUT_VALUE, pSeg);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnTimerMessageOverlayCONNECTED(CSegment* pParam)
{
/*
  if (IsValidTimer(MESSAGE_OVERLAY_TIMER))
  {
    DeleteTimer(MESSAGE_OVERLAY_TIMER);
  }
*/
	CMessageOverlayInfo* pMessageOverlayInfo = new CMessageOverlayInfo;
	pMessageOverlayInfo->DeSerialize(NATIVE, *pParam);

	if (m_pBridgePartyOut)
		((CBridgePartyVideoOut*)m_pBridgePartyOut)->UpdateMessageOverlay(pMessageOverlayInfo);

	POBJDELETE(pMessageOverlayInfo);
}
//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnTimerMultipleTPConnectionsCONNECTED(CSegment* pParam)
{
	TRACEINTO << "PartyName:" << GetName() << " - Timer completed";
	CSegment* pSeg = new CSegment;
	TRACEINTO << " ITP_PARTY_IDENT" << " DEBUG 04" << " PartyName:" << GetName();
	*pSeg << GetPartyRsrcID();
	m_pBridge->HandleEvent(pSeg, 0, TELEPRESENCE_PARTY_CONNECTION_TIMER);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::StartTelepresenceConnectionTimer()
{
	TRACEINTO << "PartyName:" << GetName() << " - Starting timer to allow for all telepresence links to connect";
	TRACEINTO << " ITP_PARTY_IDENT" << " DEBUG 03" <<  "  PartyName:" << GetName();
	StartTimer(MULTIPLE_TELEPRESENCE_CONNECTIONS_TIMER, MULTIPLE_TELEPRESENCE_CONNECTIONS_TIMER_OUT_VALUE);
}
//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::StopTelepresenceConnectionTimer()
{
	TRACEINTO << "PartyName:" << GetName() << " - Stop timer to allow for all telepresence links to connect";
	DeleteTimer(MULTIPLE_TELEPRESENCE_CONNECTIONS_TIMER);
}
//--------------------------------------------------------------------------
// VNGR-26449 - unencrypted conference message
void CVideoBridgePartyCntl::OnTimerSecureMessageCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;

	CMessageOverlayInfo* pMessageOverlayInfo = new CMessageOverlayInfo;
	pMessageOverlayInfo->DeSerialize(NATIVE, *pParam);

	// check if message overlay is permanent
	if (GetIsMessageOverlayPermanent() == TRUE)
	{
		pMessageOverlayInfo->SetMessageText(GetMessageOverlayText());
		pMessageOverlayInfo->SetMessageOnOff(TRUE);
	}
	else
		pMessageOverlayInfo->SetMessageOnOff(FALSE);

	UpdateMessageOverlay(pMessageOverlayInfo);

	POBJDELETE(pMessageOverlayInfo);
}

//--------------------------------------------------------------------------
// VNGR-26449 - unencrypted conference message
void CVideoBridgePartyCntl::OnTimerMessageOverlayEndCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;

	CMessageOverlayInfo* pMessageOverlayInfo = new CMessageOverlayInfo;
	pMessageOverlayInfo->DeSerialize(NATIVE, *pParam);

	SetMessageOverlayText(pMessageOverlayInfo->GetMessageText());
	if (pMessageOverlayInfo->GetDisplaySpeedType() == eStatic)
	{
		SetIsMessageOverlayPermanent(TRUE);
	}
	else
	{
		SetIsMessageOverlayPermanent(FALSE);
	}

	if (GetIsPermanentSecureMessageForParty() == true)
		StartSecureMessageTimer(GetNumOfUnencrypted(), pMessageOverlayInfo);

	POBJDELETE(pMessageOverlayInfo);
}

//--------------------------------------------------------------------------
// VNGR-26449 - unencrypted conference message
void CVideoBridgePartyCntl::StartSecureMessageTimer(int numOfUnencrypted, CMessageOverlayInfo* pMessageOverlayInfo)
{
	TRACEINTO << m_partyConfName;

	StopSecureMessageTimer();

	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	int messageDuration = 0;
	std::string key;
	if (0 == numOfUnencrypted)
	{
		key = "DISPLAY_ENCRYPTED_MESSAGE_TIMER_FOR_ENCRYPT_WHEN_POSSIBLE";
	}
	else
	{
		key = "DISPLAY_UNENCRYPTED_MESSAGE_TIMER_FOR_ENCRYPT_WHEN_POSSIBLE";
	}

	sysConfig->GetIntDataByKey(key, messageDuration);

	// check if the message is permanent
	if (messageDuration == -1)
	{
		// setFlag
		// check which message the party should get
		if (numOfUnencrypted == 0)
			pMessageOverlayInfo->SetMessageText("The conference is secured");
		else
			pMessageOverlayInfo->SetMessageText("The conference is not secured");

		pMessageOverlayInfo->SetMessageOnOff(TRUE);
		pMessageOverlayInfo->SetMessageDisplaySpeedType(eStatic);
		SetIsPermanentSecureMessageForParty(TRUE);
		StartMessageOverlay(pMessageOverlayInfo);
	}
	// check if the message disabled
	else if (messageDuration == 0)
	{
		// VNGE-27007
		if (GetIsMessageOverlayPermanent() == FALSE)
		{
			pMessageOverlayInfo->SetMessageOnOff(FALSE);
			StartMessageOverlay(pMessageOverlayInfo);
		}
	}
	// after the timer expires the message disappears
	else
	{
		// check which message the party should get
		if (numOfUnencrypted == 0)
			pMessageOverlayInfo->SetMessageText("The conference is secured");
		else
			pMessageOverlayInfo->SetMessageText("The conference is not secured");

		pMessageOverlayInfo->SetMessageOnOff(TRUE);
		pMessageOverlayInfo->SetMessageDisplaySpeedType(eStatic);
		CSegment* pSeg = new CSegment;
		pMessageOverlayInfo->Serialize(NATIVE, *pSeg);
		StartMessageOverlay(pMessageOverlayInfo);
		StartTimer(SECURE_MESSAGE_TIMER, messageDuration*SECOND, pSeg);
	}
}

//--------------------------------------------------------------------------
// VNGR-26449 - unencrypted conference message
void CVideoBridgePartyCntl::StartMessageOverlayTimer(int numOfUnencrypted, CMessageOverlayInfo* pMessageOverlayInfo)
{
	TRACEINTO << m_partyConfName;

	SetNumOfUnencrypted(numOfUnencrypted);
	SetMessageOverlayText(pMessageOverlayInfo->GetMessageText());
	if (pMessageOverlayInfo->GetDisplaySpeedType() == eStatic)
		SetIsMessageOverlayPermanent(TRUE);
	else
		SetIsMessageOverlayPermanent(FALSE);

	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	int messageDuration = 0;
	std::string key;
	if (0 == numOfUnencrypted)
	{
		key = "DISPLAY_ENCRYPTED_MESSAGE_TIMER_FOR_ENCRYPT_WHEN_POSSIBLE";
	}
	else
	{
		key = "DISPLAY_UNENCRYPTED_MESSAGE_TIMER_FOR_ENCRYPT_WHEN_POSSIBLE";
	}

	sysConfig->GetIntDataByKey(key, messageDuration);
	// check if secure message is permanent
	if (messageDuration == -1)
	{
		// calculate the time of message overlay
		WORD messageOverlayTime = CalcMessageOverlayTime(pMessageOverlayInfo);
		// if message overlay is not permanent
		TRACEINTO << m_partyConfName << ", messageOverlayTime:" << messageOverlayTime;

		if (messageOverlayTime != 0)
		{
			// startTimer with the clac to know when to return secure message
			CSegment* pSeg = new CSegment;
			pMessageOverlayInfo->Serialize(NATIVE, *pSeg);
			StartTimer(MESSAGE_OVERLAY_END_TIMER, messageOverlayTime*SECOND, pSeg);
		}
	}
}

//--------------------------------------------------------------------------
WORD CVideoBridgePartyCntl::CalcMessageOverlayTime(CMessageOverlayInfo* pMessageOverlayInfo)
{
	WORD time = 0;
	WORD numOfCharsInScreen = 0;
	WORD timeTakesOneCharThroughScreen = 0;
	WORD messageLen = (pMessageOverlayInfo->GetMessageText()).length();

	switch (pMessageOverlayInfo->GetDisplaySpeedType())
	{
		case eSlow:
			timeTakesOneCharThroughScreen = 13;
			break;

		case eFast:
			timeTakesOneCharThroughScreen = 7;
			break;

		// message overlay is permanent, but also the secure message.
		default:
			return 0;
	} // switch

	switch (pMessageOverlayInfo->GetFontSize_Old())
	{
		case eSmall:
			numOfCharsInScreen = 104;
			break;

		case eMedium:
			numOfCharsInScreen = 62;
			break;

		case eLarge:
			numOfCharsInScreen = 40;
			break;
	} // switch

	time = timeTakesOneCharThroughScreen*(1+(messageLen/numOfCharsInScreen));
	time = time*(pMessageOverlayInfo->GetNumOfRepetitions());
	return time;
}

//--------------------------------------------------------------------------
// VNGR-26449 - unencrypted conference message
void CVideoBridgePartyCntl::StopSecureMessageTimer()
{
	if (IsValidTimer(SECURE_MESSAGE_TIMER))
		DeleteTimer(SECURE_MESSAGE_TIMER);
}

//--------------------------------------------------------------------------
// VNGR-26449 - unencrypted conference message
bool CVideoBridgePartyCntl::GetIsSecureMessageTimerWorks()
{
	if (IsValidTimer(SECURE_MESSAGE_TIMER))
		return TRUE;
	return FALSE;
}

//--------------------------------------------------------------------------
// VNGR-26449 - unencrypted conference message
void CVideoBridgePartyCntl::StopMessageOverlayEndTimer()
{
	if (IsValidTimer(MESSAGE_OVERLAY_END_TIMER))
		DeleteTimer(MESSAGE_OVERLAY_END_TIMER);
}

//--------------------------------------------------------------------------
// VNGR-26449 - unencrypted conference message
bool CVideoBridgePartyCntl::GetIsMessageOverlayEndTimerWorks()
{
	if (IsValidTimer(MESSAGE_OVERLAY_END_TIMER))
		return TRUE;
	return FALSE;
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::UpdateSiteNameInfo(CSiteNameInfo* pSiteNameInfo)
{
	if (m_pBridgePartyOut)
		((CBridgePartyVideoOut*)m_pBridgePartyOut)->UpdateSiteNameInfo(pSiteNameInfo);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::RefreshLayout()
{
	if (m_pBridgePartyOut)
		((CBridgePartyVideoOut*)m_pBridgePartyOut)->RefreshLayout();
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeUpdateVideoClarityIDLE(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoBridgeUpdateVideoClarity(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeUpdateVideoClaritySETUP(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoBridgeUpdateVideoClarity(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeUpdateVideoClarityCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoBridgeUpdateVideoClarity(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeUpdateVideoClarityCONNECTED_STANDALONE(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoBridgeUpdateVideoClarity(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeUpdateVideoClarityDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	WORD isVideoClarityEnabled = NO;
	*pParam >> isVideoClarityEnabled;
	// If we got a connect request while disconnecting the connect parameters are saved in m_pUpdatePartyInitParams, we need to update them with the updated video clarity value
	if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
	{
		if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams->GetMediaOutParams()))
			((CBridgePartyVideoOutParams*)m_pUpdatePartyInitParams->GetMediaOutParams())->SetIsVideoClarityEnabled((BYTE)isVideoClarityEnabled);

		if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams->GetMediaInParams()))
			((CBridgePartyVideoInParams*)m_pUpdatePartyInitParams->GetMediaInParams())->SetIsVideoClarityEnabled((BYTE)isVideoClarityEnabled);
	}
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeUpdateVideoClarity(CSegment* pParam)
{
	WORD isVideoClarityEnabled = NO;
	*pParam >> isVideoClarityEnabled;
	if (CPObject::IsValidPObjectPtr(m_pBridgePartyIn))
		((CBridgePartyVideoIn*)m_pBridgePartyIn)->UpdateVideoClarity(isVideoClarityEnabled);

	if (CPObject::IsValidPObjectPtr(m_pBridgePartyOut))
		((CBridgePartyVideoOut*)m_pBridgePartyOut)->UpdateVideoClarity(isVideoClarityEnabled);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::UpdateVideoClarity(WORD isVideoClarity)
{
	CSegment* pSeg = new CSegment;
	*pSeg << isVideoClarity;
	DispatchEvent(UPDATE_VIDEO_CLARITY, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeUpdateAutoBrightnessIDLE(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoBridgeUpdateAutoBrightness(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeUpdateAutoBrightnessSETUP(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoBridgeUpdateAutoBrightness(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeUpdateAutoBrightnessCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoBridgeUpdateAutoBrightness(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeUpdateAutoBrightnessCONNECTED_STANDALONE(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoBridgeUpdateAutoBrightness(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeUpdateAutoBrightnessDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	WORD isAutoBrightnessEnabled = NO;
	*pParam >> isAutoBrightnessEnabled;
	// If we got a connect request while disconnecting the connect parameters are saved in m_pUpdatePartyInitParams, we need to update them with the updated video clarity value
	if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
	{
		if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams->GetMediaOutParams()))
			((CBridgePartyVideoOutParams*)m_pUpdatePartyInitParams->GetMediaOutParams())->SetIsAutoBrightness((bool)isAutoBrightnessEnabled);

		if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams->GetMediaInParams()))
			((CBridgePartyVideoInParams*)m_pUpdatePartyInitParams->GetMediaInParams())->SetIsAutoBrightness((bool)isAutoBrightnessEnabled);
	}
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeUpdateAutoBrightness(CSegment* pParam)
{
	WORD isAutoBrightnessEnabled = NO;
	*pParam >> isAutoBrightnessEnabled;
	if (CPObject::IsValidPObjectPtr(m_pBridgePartyIn))
	{
		((CBridgePartyVideoIn*)m_pBridgePartyIn)->UpdateAutoBrightness(isAutoBrightnessEnabled);
	}
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::UpdateAutoBrightness(WORD isAutoBrightness)
{
	CSegment* pSeg = new CSegment;
	*pSeg << isAutoBrightness;
	DispatchEvent(UPDATE_AUTO_BRIGHTNESS, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeChangeSpeakerNotationForPcmFeccIDLE(CSegment* pParam)
{
	TRACEINTO << m_partyConfName << " - Ignored";
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeChangeSpeakerNotationForPcmFeccSETUP(CSegment* pParam)
{
	TRACEINTO << m_partyConfName << " - Ignored";
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeChangeSpeakerNotationForPcmFeccCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	DWORD imageId = INVALID;
	*pParam >> imageId;
	if (CPObject::IsValidPObjectPtr(m_pBridgePartyOut))
		((CBridgePartyVideoOut*)m_pBridgePartyOut)->ChangeSpeakerNotationForPcmFecc(imageId);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeChangeSpeakerNotationForPcmFeccCONNECTED_STANDALONE(CSegment* pParam)
{
	TRACEINTO << m_partyConfName << " - Ignored";
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeChangeSpeakerNotationForPcmFeccDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << m_partyConfName << " - Ignored";
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::ChangeSpeakerNotationForPcmFecc(DWORD imageId)
{
	CSegment* pSeg = new CSegment;
	*pSeg << imageId;
	DispatchEvent(CHANGE_SPEAKER_NOTATION_PCM_FECC, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::RemoveResourcesFromRoutingTable()
{
	TRACEINTO << m_partyConfName;
	if (GetDisconnectingDirectionsReq() == eMediaOut)
	{
		CConfPartyRoutingTable* pRoutingTable = ::GetpConfPartyRoutingTable();

		PASSERT_AND_RETURN(pRoutingTable == NULL);
		PASSERT_AND_RETURN(!m_pBridgePartyOut);
		PASSERT_AND_RETURN(!m_pBridgePartyOut->IsDisConnected());

		CRsrcParams* pRsrcParams = m_pBridgePartyOut->GetRsrcParams();
		PASSERT_AND_RETURN(!pRsrcParams);

		if (STATUS_FAIL == pRoutingTable->RemoveStateMachinePointerFromRoutingTbl(*pRsrcParams))
			DBGPASSERT(105);
	}

	if (GetDisconnectingDirectionsReq() == eMediaIn)
	{
		CConfPartyRoutingTable* pRoutingTable = ::GetpConfPartyRoutingTable();

		PASSERT_AND_RETURN(pRoutingTable == NULL);
		PASSERT_AND_RETURN(!m_pBridgePartyIn);
		PASSERT_AND_RETURN(!m_pBridgePartyIn->IsDisConnected());

		CRsrcParams* pRsrcParams = m_pBridgePartyIn->GetRsrcParams();
		PASSERT_AND_RETURN(!pRsrcParams);

		if (STATUS_FAIL == pRoutingTable->RemoveStateMachinePointerFromRoutingTbl(*pRsrcParams))
			DBGPASSERT(105);
	}
}

//--------------------------------------------------------------------------
BOOL CVideoBridgePartyCntl::IsUpdateLayoutAsPrivateInDB()
{
	BOOL isUpdateLayoutAsPrivateInDB = NO;
	if (GetIsPrivateLayout() && GetPartyLectureModeRole() != eLISTENER && GetPartyLectureModeRole() != eCOLD_LISTENER)
		isUpdateLayoutAsPrivateInDB = YES;

	return isUpdateLayoutAsPrivateInDB;
}

//--------------------------------------------------------------------------
DWORD CVideoBridgePartyCntl::GetOutVideoRate()
{
	DWORD outVideoRate = 0;
	if (CPObject::IsValidPObjectPtr(m_pBridgePartyOut))
	{
		outVideoRate = ((CBridgePartyVideoOut*)m_pBridgePartyOut)->GetVideoRate();
	}
	else
		PASSERT(101);

	return outVideoRate;
}

//--------------------------------------------------------------------------
DWORD CVideoBridgePartyCntl::GetLastVideoInSyncStatus() const
{
	DWORD last_status = (DWORD)(-1);
	if (m_pBridgePartyIn)
	{
		last_status = ((CBridgePartyVideoIn*)m_pBridgePartyIn)->GetLastSyncStatus();
	}
	else
	{
		TRACESTRFUNC(eLevelError) << m_partyConfName << " - Failed, m_pBridgePartyIn is NULL";
	}

	TRACEINTO << m_partyConfName << ", Status:" << last_status;
	return last_status;
}

//--------------------------------------------------------------------------
char* CVideoBridgePartyCntl::StateToString()
{
  switch (m_state)
  {
    case IDLE                 : return "STATE_IDLE";
    case SETUP                : return "STATE_SETUP";
    case CONNECTED_STANDALONE : return "STATE_CONNECTED_STANDALONE";
    case CONNECTED            : return "STATE_CONNECTED";
    case DISCONNECTING        : return "STATE_DISCONNECTING";
    case EXPORT               : return "STATE_EXPORT";
    case ALLOCATE             : return "STATE_ALLOCATE";
    case DEALLOCATE           : return "STATE_DEALLOCATE";
    case DISCONNECTED         : return "STATE_DISCONNECTED";
  }
  return "STATE_UNDEFINED";
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::UpdateOnImageSvcToAvcTranslate()
{
	DispatchEvent(UPDATEONIMAGESVCTOAVC, NULL);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeUpdateOnImageSvcToAvc(CSegment* pParam)
{
	TRACECOND_AND_RETURN(!m_pBridgePartyOut, "Failed, " << m_partyConfName << ", State:" << StateToString());
	((CBridgePartyVideoOut*)m_pBridgePartyOut)->UpdateOnImageSvcToAvcTranslate();
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::UpdateVideoBridgeOnNonRelayImageAvcToSvcTranslated()
{
	m_pConfApi->UpdateOnNonRelayImageAvcToSvcTranslated(m_pParty, VIDEO_BRIDGE_MSG, statOK);
}

//--------------------------------------------------------------------------
CVideoOperationPointsSet* CVideoBridgePartyCntl::GetConfVideoOperationPointsSet() const
{
	if (IsValidPObjectPtr(m_pBridge))
	{
		return ((CVideoBridgeCP*)m_pBridge)->GetConfVideoOperationPointsSet();
	}

	PASSERT(101);
	return NULL;
}

//--------------------------------------------------------------------------
//void CVideoBridgePartyCntl::SendIntraToAvcToSvcIfExists()
//{
/*	if (IsTranslatorAvcSvcExists())
  {
    PTRACE(eLevelWarn, "CVideoBridgePartyCntl::SendIntraToAvcToSvcIfExists -  send intra request to AvcToSvcTranslator Encoder");
    ((CBridgePartyVideoIn*)m_pBridgePartyIn)->SendRelayIntraRequestToAvcToSvcTranslator((DWORD)eAvcToSvcIntraAll);
  }
  else
    PTRACE(eLevelWarn, "CVideoBridgePartyCntl::SendIntraToAvcToSvcIfExists -  AvcToSvcTranslator not exists");
*/
//}

//--------------------------------------------------------------------------
bool CVideoBridgePartyCntl::IsTranslatorAvcSvcExists()
{
	if (m_pBridgePartyIn)
	{
		return ((CBridgePartyVideoIn*)m_pBridgePartyIn)->IsTranslatorAvcSvcExists();
	}
	else
		TRACESTRFUNC(eLevelError) << m_partyConfName << " - Failed, m_pBridgePartyIn is NULL";

	return false;
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::SendRelayIntraRequestToAvcToSvcTranslator(const std::list<unsigned int>& listSsrc, bool bIsGDR)
{
	// DWORD ssrc = listSsrc.front(); // bridge-7550
	if (m_pBridgePartyIn)
	{
		std::list<unsigned int>::const_iterator it = listSsrc.begin();	// bridge-7550
		for ( ; it != listSsrc.end(); ++it)
		{
			DWORD ssrc = *it;
			((CBridgePartyVideoIn*)m_pBridgePartyIn)->SendRelayIntraRequestToAvcToSvcTranslator(ssrc);

			// return ((CBridgePartyVideoIn*)m_))->SendRelayIntraRequestToAvcToSvcTranslator(ssrc);	// bridge-7550
		}
	}
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::SetTelepresenceInfo(const CTelepresenseEPInfo* tpInfo)
{
	if (CPObject::IsValidPObjectPtr(tpInfo))
	{
		m_telepresenceInfo = *tpInfo;
		m_telepresenceInfo.TestValidityAndCorrectParams();

		//update Telepresence EP info - link #
		if (eTelePresencePartyCTS != m_telepresenceInfo.GetEPtype() && eTelePresencePartyNone != m_telepresenceInfo.GetEPtype() && eTelePresencePartyInactive != m_telepresenceInfo.GetEPtype())
		{
			m_telepresenceInfo.UpdateLinkNumFromName(m_ITPSiteName);	// parse name in order to get link #
		}

		TRACEINTO << "RoomId:" << m_telepresenceInfo.GetRoomID()
		          << ", EP Type:" << m_telepresenceInfo.GetEPtype() << ", link #:" << m_telepresenceInfo.GetLinkNum() << " out of "
		          << m_telepresenceInfo.GetNumOfLinks() << " links, IsCascadeMultiLink:" << ((m_telepresenceInfo.GetLinkRole()) ? "TRUE" : "FALSE");

		m_telepresenceInfo.Dump();

		if (m_telepresenceInfo.GetEPtype() == eTelePresencePartyNone && m_telepresenceInfo.GetLinkRole() == 0)
		{
			m_telepresenceInfo.SetLinkNum(0);
			m_telepresenceInfo.SetNumOfLinks(1);
		}
	}
	else
	{
		PASSERTMSG(TRUE, "Received NULL pointer for CTelepresenseEPInfo value");
		CTelepresenseEPInfo no_info;
		m_telepresenceInfo = no_info;
	}
}
//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::UpdateLayoutHandlerType()
{
	DispatchEvent(UPDATE_LAYOUT_HANDLER_TYPE, NULL);
}


//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeUpdateLayoutHandlerTypeSETUP(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoBridgeUpdateLayoutHandlerType(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeUpdateLayoutHandlerTypeCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoBridgeUpdateLayoutHandlerType(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeUpdateLayoutHandlerTypeCONNECTED_STANDALONE(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoBridgeUpdateLayoutHandlerType(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::OnVideoBridgeUpdateLayoutHandlerType(CSegment* pParam)
{
	TRACECOND_AND_RETURN(!m_pBridgePartyOut, "Failed, " << m_partyConfName << ", State:" << StateToString());
	((CBridgePartyVideoOut*)m_pBridgePartyOut)->UpdateLayoutHandlerType();
}
//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::ForwardFlowControlCommand(DWORD newRate, CLPRParams* LprParams)
{
	PASSERTSTREAM(1, "Should be implemented in derived class");
}
void CVideoBridgePartyCntl::CheckIsMutedVideoInAndUpdateDB()
{
	TRACEINTO << m_partyConfName;
	if (CPObject::IsValidPObjectPtr(m_pBridgePartyIn))
	{
		bool bIsMuted = ((CBridgePartyVideoIn *)m_pBridgePartyIn)->IsMuted();
		if(bIsMuted)
		{
			if( ((CBridgePartyVideoIn *)m_pBridgePartyIn)->IsMuteByParty())
			{
				TRACEINTO << m_partyConfName << "Update DB on PARTY mute";
				DWORD updateStatus;
				updateStatus = 0xF0000010;
				m_pConfApi->UpdateDB(m_pParty, MUTE_STATE, updateStatus);

			}
			if(((CBridgePartyVideoIn *)m_pBridgePartyIn)->IsMuteByMCMS())
			{
				TRACEINTO << m_partyConfName << "Update DB on MCMS mute";
				DWORD updateStatus;
				updateStatus =0x0F000010;
				m_pConfApi->UpdateDB(m_pParty, MUTE_STATE, updateStatus);
			}
			if(((CBridgePartyVideoIn *)m_pBridgePartyIn)->IsMuteByOperator())
			{
				TRACEINTO << m_partyConfName << "Update DB on OPERATOR mute";
				DWORD updateStatus;
				updateStatus =0x00000010;
				m_pConfApi->UpdateDB(m_pParty, MUTE_STATE, updateStatus);

			}
		}
	}
}
//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::UpgradeAvcToSvcTranslator( CAvcToSvcParams *pAvcToSvcParams )
{
	TRACEINTO << " -  connect AVC to SVC translator";

	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD)pAvcToSvcParams;
	DispatchEvent(UPGRADE_TO_MIX_AVC_SVC, pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoBridgePartyCntl::OnPartyUpgradeAvcToSvcTranslatorCONNECTED(CSegment* pParam)         // IVR_JOIN_CONF_VIDEO
{
	TRACEINTO;
	OnPartyUpgradeAvcToSvcTranslator( pParam );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoBridgePartyCntl::OnPartyUpgradeAvcToSvcTranslatorCONNECTED_STANDALONE(CSegment* pParam)         // IVR_JOIN_CONF_VIDEO
{
	TRACEINTO;
	OnPartyUpgradeAvcToSvcTranslator( pParam );;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoBridgePartyCntl::OnPartyUpgradeAvcToSvcTranslatorSETUP(CSegment* pParam)         // IVR_JOIN_CONF_VIDEO
{
	TRACEINTO;
	OnPartyUpgradeAvcToSvcTranslator( pParam );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoBridgePartyCntl::OnPartyUpgradeAvcToSvcTranslator(CSegment* pParam)         // IVR_JOIN_CONF_VIDEO
{
	if (!m_pBridgePartyIn)
	{
		TRACEINTO << " - Error: m_pBridgePartyIn==NULL";	// it is not always error when IN not exists
		ReplayUpgradeSvcToAvcTranslate(statIllegal);
	}
	else
	{

	  if(NULL == pParam){
		TRACEINTO << " - pParam==NULL";	// it is not always error when IN not exists
		ReplayUpgradeSvcToAvcTranslate(statIllegal);
		PASSERT(104);
		return;
	  }
		CAvcToSvcParams *pAvcToSvcParams;
		*pParam >> (DWORD&)pAvcToSvcParams;
		((CBridgePartyVideoIn*)m_pBridgePartyIn)->UpgradeAvcToSvcTranslator( pAvcToSvcParams );
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoBridgePartyCntl::OnPartyUpgradeAvcToSvcTranslatorDISCONNECTING(CSegment* pParam)         // IVR_JOIN_CONF_VIDEO
{
		ReplayUpgradeSvcToAvcTranslate(statIllegal);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoBridgePartyCntl::OnPartyResumeFromHoldCONNECTED(CSegment* pParam)
{
	BOOL isResumeIntoIVR = FALSE;
	*pParam >> isResumeIntoIVR;
	TRACEINTO << "is resuming into IVR: " << (int)isResumeIntoIVR;
	m_bIsResumingIVR = isResumeIntoIVR;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoBridgePartyCntl::OnPartyResumeFromHoldANYCASE(CSegment* pParam)
{
	TRACEINTO << "received in state:" << GetStateAsString() << ". nothing to do.";
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CVideoBridgePartyCntl::GetPartyResumeFromHoldInIVR()
{
	return m_bIsResumingIVR;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoBridgePartyCntl::ReplayUpgradeSvcToAvcTranslate( EStat status )
{
	TRACEINTO << " - Upgrade AVC-SVC translator, Status: " << (DWORD)status << ", PartyId: " << GetPartyRsrcID() <<  ", ConfId: " << GetConfRsrcID();

	if (m_pConfApi)
		m_pConfApi->ReplayUpgradeSvcAvcTranslate( m_partyRsrcID, VIDEO, status );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoBridgePartyCntl::ReplayDowngradeSvcToAvcTranslate( EStat status )
{
	TRACEINTO << " - Dwongrade AVC-SVC translator, Status: " << (DWORD)status << ", PartyId: " << GetPartyRsrcID() <<  ", ConfId: " << GetConfRsrcID();

//	if (m_pConfApi)
//		m_pConfApi->ReplayDowngradeSvcAvcTranslate( m_partyRsrcID, VIDEO, status );
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool CVideoBridgePartyCntl::IsRecordingLinkParty()
{

  const CCommConf* pCommConf = m_pBridge->GetConf()->GetCommConf();//::GetpConfDB()->GetCurrentConf(m_pBridge->GetConf()->GetConfId());
  PASSERTMSG_AND_RETURN_VALUE(!pCommConf, "Failed, 'pCommConf' is NULL", false);

  CConfParty* pConfParty = pCommConf->GetCurrentParty(m_name);
  PASSERTMSG_AND_RETURN_VALUE(!pConfParty, "Failed, 'pConfParty' is NULL", false);

  bool RC = (pConfParty->GetRecordingLinkParty() == YES) ? true : false;
  if (RC)
  	PTRACE2(eLevelInfoNormal, "CVideoBridgePartyCntl::IsRecordingLinkParty recording link is , ", m_name);
  return RC;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool  CVideoBridgePartyCntl::IsAVMCUMain() const
{
	bool isAVMCUMaster = false;
	if(m_MS_masterPartyRsrcID && (m_MS_masterPartyRsrcID == m_partyRsrcID))
	{
		ON(isAVMCUMaster);
	}
	return isAVMCUMaster;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool  CVideoBridgePartyCntl::IsEPFromSameAVMCU(PartyRsrcID masterPartyRsrcID)
{
	bool IsEPFromSameAVMCU = false;
	if(m_MS_masterPartyRsrcID && (m_MS_masterPartyRsrcID == masterPartyRsrcID))
	{
		ON(IsEPFromSameAVMCU);
	}
	return IsEPFromSameAVMCU;

}
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool CVideoBridgePartyCntl::IsAVMCUParty() const
{
	bool IsAVMCUParty = false;
	PASSERTMSG_AND_RETURN_VALUE(!m_pBridge, "Failed, 'm_pBridge' is NULL", false);

	CAVMCUMngr* pAVMCUMngr = ((CVideoBridge*)m_pBridge)->GetAVMCUMngr();
	if(!pAVMCUMngr)
	{
		return IsAVMCUParty;
	}

	if(m_MS_masterPartyRsrcID)
	{
		if(m_MS_masterPartyRsrcID == pAVMCUMngr->GetMasterPartyRsrcID())
		   ON(IsAVMCUParty);
		else
			PASSERT(m_MS_masterPartyRsrcID);
	}
	return IsAVMCUParty;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//eFeatureRssDialin
bool CVideoBridgePartyCntl::IsLastLayoutForRLLocked()
{
  bool  ret = false;
  const CCommConf* pCommConf = m_pBridge->GetConf()->GetCommConf();
  PASSERTMSG_AND_RETURN_VALUE(!pCommConf, "Failed, 'pCommConf' is NULL", false);

  CConfParty* pConfParty = pCommConf->GetCurrentParty(m_name);
  PASSERTMSG_AND_RETURN_VALUE(!pConfParty, "Failed, 'pConfParty' is NULL", false);
  if(YES == pConfParty->GetRecordingLinkParty())
  {
  	enSrsVideoLayoutType	videoLayout = (enSrsVideoLayoutType) pConfParty->GetLastLayoutForRL();
	// 1X1 or 1X2, locked
	if(eSrsVideoLayoutAuto != videoLayout)
	{
		ret = true;
	}
  }
  if (ret)
  	PTRACE2(eLevelInfoNormal, "CVideoBridgePartyCntl::IsLastLayoutForRLLocked, recording link is locked! - ", m_name);
  return ret;
}

ETelePresenceLayoutMode CVideoBridgePartyCntl::GetPartyTelepresenceLayoutModeConfiguration()const
{
	// currently taken from Conf configuration, in the future can be different for each party

	ETelePresenceLayoutMode retVal = eTelePresenceLayoutManual;
	CCommConf* pCommConf = (CCommConf*)(m_pBridge->GetConf())->GetCommConf();
	if(NULL != pCommConf){
		retVal = (ETelePresenceLayoutMode)(pCommConf->GetTelePresenceLayoutMode());
	}else{
		DBGPASSERT(1);
	}

	// TELEPRESENCE_LAYOUTS
	if(retVal <= eTelePresenceLayoutCpParticipantsPriority)
		TRACEINTO << "TELEPRESENCE_LAYOUTS_DEBUG retVal = " << TelePresenceLayoutModeToString(retVal);
	else
		PASSERT(retVal);


	return retVal;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////


PBEGIN_MESSAGE_MAP(CVideoBridgePartyCntlVSW)

	ONEVENT(VIDEO_ENCODER_SYNC_IND, IDLE,                 CVideoBridgePartyCntlVSW::NullActionFunction)
	ONEVENT(VIDEO_ENCODER_SYNC_IND, CONNECTED,            CVideoBridgePartyCntlVSW::OnVideoOutSyncIndCONNECTED)
	ONEVENT(VIDEO_ENCODER_SYNC_IND, CONNECTED_STANDALONE, CVideoBridgePartyCntlVSW::NullActionFunction)
	ONEVENT(VIDEO_ENCODER_SYNC_IND, SETUP,                CVideoBridgePartyCntlVSW::NullActionFunction)
	ONEVENT(VIDEO_ENCODER_SYNC_IND, DISCONNECTING,        CVideoBridgePartyCntlVSW::NullActionFunction)

	ONEVENT(VIDREFRESH,             CONNECTED_STANDALONE, CVideoBridgePartyCntlVSW::OnVideoOutVideoRefreshCONNECTED_STANDALONE)
	ONEVENT(FASTUPDATE,             CONNECTED_STANDALONE, CVideoBridgePartyCntlVSW::OnVideoBridgeFastUpdateCONNECTED_STANDALONE)

PEND_MESSAGE_MAP(CVideoBridgePartyCntlVSW, CVideoBridgePartyCntl);

////////////////////////////////////////////////////////////////////////////
//                        CVideoBridgePartyCntlVSW
////////////////////////////////////////////////////////////////////////////
CVideoBridgePartyCntlVSW::CVideoBridgePartyCntlVSW() : CVideoBridgePartyCntl()
{
	m_partyFlowControlRate = 0;     // 0 means no flow control was received for this party
	m_bIsCascadeParty      = FALSE; // FALSE means regular party
}

//--------------------------------------------------------------------------
CVideoBridgePartyCntlVSW::~CVideoBridgePartyCntlVSW()
{
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlVSW::Create(const CVideoBridgePartyInitParams* pBridgePartyInitParams)
{
	PASSERT_AND_RETURN(NULL == pBridgePartyInitParams);

	if (!pBridgePartyInitParams->GetMediaInParams() && !pBridgePartyInitParams->GetMediaOutParams())
	{
		PASSERT_AND_RETURN(102);
	}

	CConfPartyRoutingTable* pRoutingTable = ::GetpConfPartyRoutingTable();
	PASSERT_AND_RETURN(NULL == pRoutingTable);

	// Create base params
	CBridgePartyCntl::Create(pBridgePartyInitParams);

	BOOL bVideoInFailure = FALSE, bVideoOutFailure = FALSE;
	CRsrcParams* pRsrcParams = new CRsrcParams;
	pRsrcParams->SetConfRsrcId(m_confRsrcID);
	pRsrcParams->SetPartyRsrcId(m_partyRsrcID);

	CRsrcDesc* pRsrcDesc = NULL;

	CBridgePartyVideoInParams* videoInParams = (CBridgePartyVideoInParams*)pBridgePartyInitParams->GetMediaInParams();

	if (videoInParams)
	{
		m_pBridgePartyIn = new CBridgePartyVideoInVSW;

		CTaskApi* pTaskApiVideoIn = new CTaskApi(*m_pConfApi);

		pTaskApiVideoIn->CreateOnlyApi(m_pConfApi->GetRcvMbx(), m_pBridgePartyIn);
		pRsrcDesc = pRoutingTable->AddStateMachinePointerToRoutingTbl(m_partyRsrcID, eLogical_video_decoder, pTaskApiVideoIn);
		BOOL isDecoderEntryFoundInRoutingTable = FALSE;
		if (!pRsrcDesc) // Entry not found in Routing Table
		{
			CCommConf* pCommConf = (CCommConf*)(m_pBridge->GetConf())->GetCommConf();
			if (pCommConf)
			{
				BYTE isEQConf = pCommConf->GetEntryQ();
				if (isEQConf) // VNGR-12644 in version 2000c in case of EQ VSW Ra will allocate dummy connection id for encoder and decoder
				{
					pRsrcDesc = pRoutingTable->AddStateMachinePointerToRoutingTbl(m_partyRsrcID, eLogical_VSW_dummy_decoder, pTaskApiVideoIn);
					if (pRsrcDesc)
					{
						isDecoderEntryFoundInRoutingTable = TRUE;
					}
				}
			}
		}
		else
		{
			isDecoderEntryFoundInRoutingTable = TRUE;
		}

		if (!isDecoderEntryFoundInRoutingTable)
		{
			bVideoInFailure = TRUE;
			POBJDELETE(m_pBridgePartyIn);
			PASSERT(104);
		}
		else
		{
			pRsrcParams->SetRsrcDesc(*pRsrcDesc);
			((CBridgePartyVideoInVSW*)(m_pBridgePartyIn))->Create(this, pRsrcParams, pBridgePartyInitParams->GetMediaInParams());
		}

		POBJDELETE(pTaskApiVideoIn);
	}

	CBridgePartyVideoOutParams* videoOutParams = (CBridgePartyVideoOutParams*)pBridgePartyInitParams->GetMediaOutParams();

	if (videoOutParams)
	{
		m_pBridgePartyOut = new CBridgePartyVideoOutVSW;
		CTaskApi* pTaskApiVideoOut = new CTaskApi(*m_pConfApi);
		pTaskApiVideoOut->CreateOnlyApi(m_pConfApi->GetRcvMbx(), m_pBridgePartyOut);
		pRsrcDesc = pRoutingTable->AddStateMachinePointerToRoutingTbl(m_partyRsrcID, eLogical_video_encoder, pTaskApiVideoOut);
		BOOL isEncoderEntryFoundInRoutingTable = FALSE;
		if (!pRsrcDesc) // Entry not found in Routing Table
		{
			CCommConf* pCommConf = (CCommConf*)(m_pBridge->GetConf())->GetCommConf();
			if (pCommConf)
			{
				BYTE isEQConf = pCommConf->GetEntryQ();
				if (isEQConf) // VNGR-12644 in version 2000c in case of EQ VSW Ra will allocate dummy connection id for encoder and decoder
				{
					pRsrcDesc = pRoutingTable->AddStateMachinePointerToRoutingTbl(m_partyRsrcID, eLogical_VSW_dummy_encoder, pTaskApiVideoOut);
					if (pRsrcDesc)
					{
						isEncoderEntryFoundInRoutingTable = TRUE;
					}
				}
			}
		}
		else
		{
			isEncoderEntryFoundInRoutingTable = TRUE;
		}

		if (!isEncoderEntryFoundInRoutingTable)
		{
			bVideoOutFailure = TRUE;
			POBJDELETE(m_pBridgePartyOut);
			PASSERT(105);
		}
		else
		{
			pRsrcParams->SetRsrcDesc(*pRsrcDesc);
			((CBridgePartyVideoOutVSW*)(m_pBridgePartyOut))->Create(this, pRsrcParams, pBridgePartyInitParams->GetMediaOutParams());
		}

		POBJDELETE(pTaskApiVideoOut);
	}

	if (TRUE == bVideoInFailure)
		TRACESTRFUNC(eLevelError) << GetFullName() << " - Video-In creation failure";

	if (TRUE == bVideoOutFailure)
		TRACESTRFUNC(eLevelError) << GetFullName() << " - Video-Out creation failure";

	SetSiteName(pBridgePartyInitParams->GetSiteName());


	POBJDELETE(pRsrcParams);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlVSW::Update(const CVideoBridgePartyInitParams* pBridgePartyInitParams)
{
	PASSERT_AND_RETURN(NULL == pBridgePartyInitParams);

	if (!pBridgePartyInitParams->GetMediaInParams() && !pBridgePartyInitParams->GetMediaOutParams())
	{
		PASSERT_AND_RETURN(102);
	}

	CConfPartyRoutingTable* pRoutingTable = ::GetpConfPartyRoutingTable();
	PASSERT_AND_RETURN(NULL == pRoutingTable);

	BOOL bVideoInFailure = FALSE, bVideoOutFailure = FALSE;
	CRsrcParams* pRsrcParams = new CRsrcParams;
	pRsrcParams->SetConfRsrcId(m_confRsrcID);
	pRsrcParams->SetPartyRsrcId(m_partyRsrcID);

	CRsrcDesc* pRsrcDesc = NULL;

	CBridgePartyVideoInParams* videoInParams = (CBridgePartyVideoInParams*)pBridgePartyInitParams->GetMediaInParams();

	if (videoInParams)
	{
		if (!m_pBridgePartyIn)
		{
			m_pBridgePartyIn = new CBridgePartyVideoInVSW;

			CTaskApi* pTaskApiVideoIn = new CTaskApi(*m_pConfApi);
			pTaskApiVideoIn->CreateOnlyApi(m_pConfApi->GetRcvMbx(), m_pBridgePartyIn);
			pRsrcDesc = pRoutingTable->AddStateMachinePointerToRoutingTbl(m_partyRsrcID, eLogical_video_decoder, pTaskApiVideoIn);
			BOOL isDecoderEntryFoundInRoutingTable = FALSE;
			if (!pRsrcDesc) // Entry not found in Routing Table
			{
				CCommConf* pCommConf = (CCommConf*)(m_pBridge->GetConf())->GetCommConf();
				if (pCommConf)
				{
					BYTE isEQConf = pCommConf->GetEntryQ();
					if (isEQConf) // VNGR-12644 in version 2000c in case of EQ VSW Ra will allocate dummy connection id for encoder and decoder
					{
						pRsrcDesc = pRoutingTable->AddStateMachinePointerToRoutingTbl(m_partyRsrcID, eLogical_VSW_dummy_decoder, pTaskApiVideoIn);
						if (pRsrcDesc)
						{
							isDecoderEntryFoundInRoutingTable = TRUE;
						}
					}
				}
			}
			else
			{
				isDecoderEntryFoundInRoutingTable = TRUE;
			}

			if (!isDecoderEntryFoundInRoutingTable)
			{
				bVideoInFailure = TRUE;
				POBJDELETE(m_pBridgePartyIn);
				PASSERT(104);
			}
			else
			{
				pRsrcParams->SetRsrcDesc(*pRsrcDesc);
				((CBridgePartyVideoInVSW*)(m_pBridgePartyIn))->Create(this, pRsrcParams, pBridgePartyInitParams->GetMediaInParams());
			}

			POBJDELETE(pTaskApiVideoIn);
		}

		// If we are in disconnecting state we need to save the In Init params
		if (m_state == DISCONNECTING || (m_state == CONNECTED && m_pBridgePartyIn && m_pBridgePartyIn->GetState() == DISCONNECTING))
		{
			if (m_pUpdatePartyInitParams != NULL)
			{
				if (!m_pUpdatePartyInitParams->GetMediaInParams())
				{
					m_pUpdatePartyInitParams->InitiateMediaInParams((CBridgePartyVideoInParams*)pBridgePartyInitParams->GetMediaInParams());
				}
			}
			else
			{
				InitiateUpdatePartyParams(pBridgePartyInitParams);
			}
		}
	}

	CBridgePartyVideoOutParams* videoOutParams = (CBridgePartyVideoOutParams*)pBridgePartyInitParams->GetMediaOutParams();

	if (videoOutParams)
	{
		// If audio out wasn't created already...
		if (!m_pBridgePartyOut)
		{
			m_pBridgePartyOut = new CBridgePartyVideoOutVSW;
			CTaskApi* pTaskApiVideoOut = new CTaskApi(*m_pConfApi);
			pTaskApiVideoOut->CreateOnlyApi(m_pConfApi->GetRcvMbx(), m_pBridgePartyOut);
			pRsrcDesc = pRoutingTable->AddStateMachinePointerToRoutingTbl(m_partyRsrcID, eLogical_video_encoder, pTaskApiVideoOut);
			BOOL isEncoderEntryFoundInRoutingTable = FALSE;
			if (!pRsrcDesc) // Entry not found in Routing Table
			{
				CCommConf* pCommConf = (CCommConf*)(m_pBridge->GetConf())->GetCommConf();
				if (pCommConf)
				{
					BYTE isEQConf = pCommConf->GetEntryQ();
					if (isEQConf) // VNGR-12644 in version 2000c in case of EQ VSW Ra will allocate dummy connection id for encoder and decoder
					{
						pRsrcDesc = pRoutingTable->AddStateMachinePointerToRoutingTbl(m_partyRsrcID, eLogical_VSW_dummy_encoder, pTaskApiVideoOut);
						if (pRsrcDesc)
						{
							isEncoderEntryFoundInRoutingTable = TRUE;
						}
					}
				}
			}
			else
			{
				isEncoderEntryFoundInRoutingTable = TRUE;
			}

			if (!isEncoderEntryFoundInRoutingTable)
			{
				bVideoOutFailure = TRUE;
				POBJDELETE(m_pBridgePartyOut);
				PASSERT(105);
			}
			else
			{
				pRsrcParams->SetRsrcDesc(*pRsrcDesc);
				((CBridgePartyVideoOutVSW*)(m_pBridgePartyOut))->Create(this, pRsrcParams, pBridgePartyInitParams->GetMediaOutParams());
			}

			POBJDELETE(pTaskApiVideoOut);
		}

		// If we are in disconnecting state we need to save the Out Init params
		if (m_state == DISCONNECTING || (m_state == CONNECTED && m_pBridgePartyOut && m_pBridgePartyOut->GetState() == DISCONNECTING))
		{
			if (m_pUpdatePartyInitParams != NULL)
			{
				if (!m_pUpdatePartyInitParams->GetMediaOutParams())
				{
					m_pUpdatePartyInitParams->InitiateMediaOutParams((CBridgePartyVideoOutParams*)pBridgePartyInitParams->GetMediaOutParams());
				}
			}
			else
			{
				InitiateUpdatePartyParams(pBridgePartyInitParams);
			}
		}
	}

	if (TRUE == bVideoInFailure)
		TRACESTRFUNC(eLevelError) << GetFullName() << " - Video-In creation failure";

	if (TRUE == bVideoOutFailure)
		TRACESTRFUNC(eLevelError) << GetFullName() << " - Video-Out creation failure";

	SetSiteName(pBridgePartyInitParams->GetSiteName());

	POBJDELETE(pRsrcParams);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlVSW::CreatePartyIn()
{
	CConfPartyRoutingTable* pRoutingTable   = ::GetpConfPartyRoutingTable();
	BOOL                    bVideoInFailure = FALSE;
	CRsrcParams*            pRsrcParams     = new CRsrcParams; AUTO_DELETE(pRsrcParams);

	pRsrcParams->SetConfRsrcId(m_confRsrcID);
	pRsrcParams->SetPartyRsrcId(m_partyRsrcID);

	NewPartyIn();

	CTaskApi*  pTaskApiVideoIn = new CTaskApi(*m_pConfApi);
	pTaskApiVideoIn->CreateOnlyApi(m_pConfApi->GetRcvMbx(), m_pBridgePartyIn);
	CRsrcDesc* pRsrcDesc = pRoutingTable->AddStateMachinePointerToRoutingTbl(m_partyRsrcID, eLogical_video_decoder, pTaskApiVideoIn);
	BOOL isDecoderEntryFoundInRoutingTable = FALSE;
	if (!pRsrcDesc) // Entry not found in Routing Table
	{
		CCommConf* pCommConf = (CCommConf*)(m_pBridge->GetConf())->GetCommConf();
		if (pCommConf)
		{
			BYTE isEQConf = pCommConf->GetEntryQ();
			if (isEQConf) // VNGR-12644 in version 2000c in case of EQ VSW Ra will allocate dummy connection id for encoder and decoder
			{
				pRsrcDesc = pRoutingTable->AddStateMachinePointerToRoutingTbl(m_partyRsrcID, eLogical_VSW_dummy_decoder, pTaskApiVideoIn);
				if (pRsrcDesc)
				{
					isDecoderEntryFoundInRoutingTable = TRUE;
				}
			}
		}
	}
	else
	{
		isDecoderEntryFoundInRoutingTable = TRUE;
	}

	if (!isDecoderEntryFoundInRoutingTable)
	{
		bVideoInFailure = TRUE;
		POBJDELETE(m_pBridgePartyIn);
		PASSERT(104);
	}
	else
	{
		pRsrcParams->SetRsrcDesc(*pRsrcDesc);
		((CBridgePartyVideoInVSW*)(m_pBridgePartyIn))->Create(this, pRsrcParams, m_pUpdatePartyInitParams->GetMediaInParams());
	}

	POBJDELETE(pTaskApiVideoIn);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlVSW::CreatePartyOut()
{
	CConfPartyRoutingTable* pRoutingTable    = ::GetpConfPartyRoutingTable();

	BOOL bVideoOutFailure = FALSE;
	CRsrcParams* pRsrcParams = new CRsrcParams; AUTO_DELETE(pRsrcParams);
	pRsrcParams->SetConfRsrcId(m_confRsrcID);
	pRsrcParams->SetPartyRsrcId(m_partyRsrcID);
	CRsrcDesc* pRsrcDesc = NULL;
	NewPartyOut();
	CTaskApi*  pTaskApiVideoOut = new CTaskApi(*m_pConfApi);
	pTaskApiVideoOut->CreateOnlyApi(m_pConfApi->GetRcvMbx(), m_pBridgePartyOut);
	pRsrcDesc = pRoutingTable->AddStateMachinePointerToRoutingTbl(m_partyRsrcID, eLogical_video_encoder, pTaskApiVideoOut);
	BOOL isEncoderEntryFoundInRoutingTable   = FALSE;
	if (!pRsrcDesc) // Entry not found in Routing Table
	{
		CCommConf* pCommConf = (CCommConf*)(m_pBridge->GetConf())->GetCommConf();
		if (pCommConf)
		{
			BYTE isEQConf = pCommConf->GetEntryQ();
			if (isEQConf) // VNGR-12644 in version 2000c in case of EQ VSW Ra will allocate dummy connection id for encoder and decoder
			{
				pRsrcDesc = pRoutingTable->AddStateMachinePointerToRoutingTbl(m_partyRsrcID, eLogical_VSW_dummy_encoder, pTaskApiVideoOut);
				if (pRsrcDesc)
				{
					isEncoderEntryFoundInRoutingTable = TRUE;
				}
			}
		}
	}
	else
	{
		isEncoderEntryFoundInRoutingTable = TRUE;
	}

	if (!isEncoderEntryFoundInRoutingTable)
	{
		bVideoOutFailure = TRUE;
		POBJDELETE(m_pBridgePartyOut);
		PASSERT(105);
	}
	else
	{
		pRsrcParams->SetRsrcDesc(*pRsrcDesc);
		((CBridgePartyVideoOutVSW*)(m_pBridgePartyOut))->Create(this, pRsrcParams, m_pUpdatePartyInitParams->GetMediaOutParams());
	}

	POBJDELETE(pTaskApiVideoOut);
}

//--------------------------------------------------------------------------
LayoutType CVideoBridgePartyCntlVSW::GetConfLayoutType()  const
{
	return CP_LAYOUT_1X1;
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlVSW::OnVideoBridgeFastUpdate(CSegment* pParam)
{
	if (CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator)
	{
		TRACEINTO << m_partyConfName << " - Ask refresh from bridge";
		m_pConfApi->VideoRefresh(GetPartyRsrcID()/*m_pParty->GetPartyId()*/);
	}
	else
	{
		// CallG_Keren
		TRACEINTO << m_partyConfName << " - In Call Generator call the CP function";
		CVideoBridgePartyCntl::OnVideoBridgeFastUpdate(pParam);
	}
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlVSW::OnVideoBridgeFastUpdateCONNECTED_STANDALONE(CSegment* pParam)
{
	TRACEINTO << m_partyConfName << " - Ask slide refresh from MFA";
	HandleEvent(NULL, 0, VIDREFRESH);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlVSW::OnVideoOutSyncIndCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;

	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD)m_partyRsrcID;
	m_pBridge->HandleEvent(pSeg, 0, VIDREFRESH_VSW_ENC);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::MarkAsNewVideoSource(WORD resync)
{
	TRACEINTO << m_partyConfName;
	m_resync = resync;
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlVSW::OnVideoOutVideoRefreshCONNECTED_STANDALONE(CSegment* pParam)
{
	if (m_pBridgePartyOut)
	{
		TRACEINTO << m_partyConfName << " - Will request video refresh of slide";
		((CBridgePartyVideoOut*)m_pBridgePartyOut)->VideoRefresh();
	}
	else
		TRACESTRFUNC(eLevelError) << m_partyConfName << " - Failed, m_pBridgePartyOut is invalid";
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlVSW::ResetImage0(DWORD partyRscId)
{
	if (m_pBridgePartyOut)
		((CBridgePartyVideoOutVSW*)m_pBridgePartyOut)->ResetImage0(partyRscId);
	else
		TRACESTRFUNC(eLevelError) << m_partyConfName << " - Failed, m_pBridgePartyOut is invalid";
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlVSW::ResetRsrvImage0(DWORD partyRscId)
{
	if (m_pBridgePartyOut)
		((CBridgePartyVideoOutVSW*)m_pBridgePartyOut)->ResetRsrvImage0(partyRscId);
	else
		TRACESTRFUNC(eLevelError) << m_partyConfName << " - Failed, m_pBridgePartyOut is invalid";
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlVSW::SetPartyForce(CVideoLayout& pPartyLayout)
{
	if (m_pBridgePartyOut)
		((CBridgePartyVideoOutVSW*)m_pBridgePartyOut)->SetPartyForce(pPartyLayout);
	else
		TRACESTRFUNC(eLevelError) << m_partyConfName << " - Failed, m_pBridgePartyOut is invalid";
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlVSW::ForwardFlowControlCommand(DWORD newRate, CLPRParams* LprParams)
{
	TRACEINTO << m_partyConfName << ", LprParams:" << (DWORD)LprParams;

	m_pPartyApi->SendFlowControlToCs(newRate, FALSE, LprParams);
}

//--------------------------------------------------------------------------
DWORD CVideoBridgePartyCntlVSW::GetMaxIntraRequestsPerInterval()
{
	DWORD dwMaxIntraRequestsPerInterval = 7;
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	sysConfig->GetDWORDDataByKey("VSW_MAX_INTRA_REQUESTS_PER_INTERVAL", dwMaxIntraRequestsPerInterval);
	return dwMaxIntraRequestsPerInterval;
}

//--------------------------------------------------------------------------
DWORD CVideoBridgePartyCntlVSW::GetIntraSuppressionDurationInSeconds()
{
	DWORD dwIntraRequestsSuppressDuration = 10;
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	sysConfig->GetDWORDDataByKey("VSW_INTRA_SUPPRESSION_DURATION_IN_SECONDS", dwIntraRequestsSuppressDuration);
	return dwIntraRequestsSuppressDuration;
}


PBEGIN_MESSAGE_MAP(CVideoBridgePartyCntlContent)

	ONEVENT(VIDCONNECT,                 IDLE,           CVideoBridgePartyCntlContent::OnVideoBridgeConnectIDLE)
	ONEVENT(VIDCONNECT,                 ALLOCATE,       CVideoBridgePartyCntlContent::OnVideoBridgeConnectALLOCATE)
	ONEVENT(VIDCONNECT,                 SETUP,          CVideoBridgePartyCntlContent::OnVideoBridgeConnectSETUP)
	ONEVENT(VIDCONNECT,                 CONNECTED,      CVideoBridgePartyCntlContent::OnVideoBridgeConnectCONNECTED)
	ONEVENT(VIDCONNECT,                 DISCONNECTING,  CVideoBridgePartyCntlContent::OnVideoBridgeConnectDISCONNECTING)
	ONEVENT(VIDCONNECT,                 DISCONNECTED,   CVideoBridgePartyCntlContent::OnVideoBridgeConnectDISCONNECTED)
	ONEVENT(VIDCONNECT,                 DEALLOCATE,     CVideoBridgePartyCntlContent::OnVideoBridgeConnectDEALLOCATE)

	ONEVENT(VIDDISCONNECT,              IDLE,           CVideoBridgePartyCntlContent::NullActionFunction)
	ONEVENT(VIDDISCONNECT,              ALLOCATE,       CVideoBridgePartyCntlContent::OnVideoBridgeDisconnectALLOCATE)
	ONEVENT(VIDDISCONNECT,              SETUP,          CVideoBridgePartyCntlContent::OnVideoBridgeDisconnectSETUP)
	ONEVENT(VIDDISCONNECT,              CONNECTED,      CVideoBridgePartyCntlContent::OnVideoBridgeDisconnectCONNECTED)
	ONEVENT(VIDDISCONNECT,              DISCONNECTING,  CVideoBridgePartyCntlContent::OnVideoBridgeDisconnectDISCONNECTING)
	ONEVENT(VIDDISCONNECT,              DISCONNECTED,   CVideoBridgePartyCntlContent::OnVideoBridgeDisconnectDISCONNECTED)
	ONEVENT(VIDDISCONNECT,              DEALLOCATE,     CVideoBridgePartyCntlContent::OnVideoBridgeDisconnectDEALLOCATE)

	ONEVENT(VIDDESTROY,                 IDLE,           CVideoBridgePartyCntlContent::OnVideoBridgeDestroyIDLE)
	ONEVENT(VIDDESTROY,                 ALLOCATE,       CVideoBridgePartyCntlContent::OnVideoBridgeDestroyALLOCATE)
	ONEVENT(VIDDESTROY,                 SETUP,          CVideoBridgePartyCntlContent::OnVideoBridgeDestroySETUP)
	ONEVENT(VIDDESTROY,                 CONNECTED,      CVideoBridgePartyCntlContent::OnVideoBridgeDestroyCONNECTED)
	ONEVENT(VIDDESTROY,                 DISCONNECTING,  CVideoBridgePartyCntlContent::OnVideoBridgeDestroyDISCONNECTING)
	ONEVENT(VIDDESTROY,                 DISCONNECTED,   CVideoBridgePartyCntlContent::OnVideoBridgeDestroyDISCONNECTED)
	ONEVENT(VIDDESTROY,                 DEALLOCATE,     CVideoBridgePartyCntlContent::OnVideoBridgeDestroyDEALLOCATE)

	ONEVENT(DEALLOCATE_PARTY_RSRC_IND,  DEALLOCATE,     CVideoBridgePartyCntlContent::OnRsrcDeallocatePartyRspDEALLOCATE)
	ONEVENT(ALLOCATE_PARTY_RSRC_IND,    ALLOCATE,       CVideoBridgePartyCntlContent::OnRsrcAllocatePartyRspALLOCATE)

	// Timers events
	ONEVENT(RADISCONNECTTOUT,           DEALLOCATE,     CVideoBridgePartyCntlContent::OnTimerRADisconnectDEALLOCATE)

PEND_MESSAGE_MAP(CVideoBridgePartyCntlContent, CVideoBridgePartyCntl);

////////////////////////////////////////////////////////////////////////////
//                        CVideoBridgePartyCntlContent
////////////////////////////////////////////////////////////////////////////
CVideoBridgePartyCntlContent::CVideoBridgePartyCntlContent (CBridge* pBridge)
{
	CVideoBridgePartyCntl::CVideoBridgePartyCntl();

	m_isConnect                = NO;  // StartContent Or First Legacy Party
	m_isDisconnect             = NO;  // StopContent
	m_isDestroy                = NO;  // DeleteConference
	m_isContentHD1080Supported = FALSE;

	SetBridge(pBridge);

	// assign party monitor for the use of allocating resources for the CVideoBridgePartyCntlContent
	CCommConf* pCommConf = (CCommConf*)(m_pBridge->GetConf())->GetCommConf();
	DWORD partyMonitorIdForContentDecoder = pCommConf->NextPartyId();

	m_pConfApi = new CConfApi;
	m_pConfApi->CreateOnlyApi(((CConf*)(m_pBridge->GetConf()))->GetRcvMbx(), this);
	m_pConfApi->SetLocalMbx(((CConf*)(m_pBridge->GetConf()))->GetLocalQueue());

	m_pPartyAllocatedRsrc = NULL;
	m_monitorPartyId      = partyMonitorIdForContentDecoder;
	CSmallString str;
	const char*  confName = m_pBridge->GetConf()->GetName();
	str << confName << ",##I_AM_THE_CONTENT_DECODER";
	strncpy(m_partyConfName, str.GetString(), BRIDGE_PARTY_CONF_NAME_SIZE - 1);
	m_partyConfName[BRIDGE_PARTY_CONF_NAME_SIZE - 1] = '\0';
}

//--------------------------------------------------------------------------
CVideoBridgePartyCntlContent::CVideoBridgePartyCntlContent(const CVideoBridgePartyCntlContent& rOtherBridgePartyCntlContent) : CVideoBridgePartyCntl(rOtherBridgePartyCntlContent)
{
	*this = rOtherBridgePartyCntlContent;
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CVideoBridgePartyCntlContent::~CVideoBridgePartyCntlContent ()
{
	POBJDELETE(m_pPartyAllocatedRsrc);
	POBJDELETE(m_pConfApi);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::OnVideoBridgeConnectIDLE(CSegment* pParam)
{
	BYTE isIVR;
	BYTE isContentHD1080Supported;
	*pParam >> isIVR >> isContentHD1080Supported;

	TRACEINTO << m_partyConfName << ", isIVR:" << (WORD)isIVR << ", isContentHD1080Supported:" << (WORD)isContentHD1080Supported;

	m_isContentHD1080Supported = isContentHD1080Supported;
	SetConnectDisconnectDestroyFlags(eContentDecoderAction_Connect);

	Allocate();
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::Allocate()
{
	m_state = ALLOCATE;
	StartTimer(RACONNECTTOUT, RA_DISCONNECT_TIME * SECOND);
	AllocateResources();
}

// we need to set this function to be used both by partycntl and bridgepartycntl - a global func to
// surve for RA OR to create a dummy partycntl only to send this message
// or to set a partyCntl as member of this class
//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::AllocateResources()
{
	TRACEINTO << "isContentHD1080Supported:" << (DWORD)m_isContentHD1080Supported;

	if (!((CVideoBridge*)GetBridge())->IsXCodeConf())
	{
		ALLOC_PARTY_REQ_PARAMS_S allocatePartyParams;
		memset(&allocatePartyParams, 0, sizeof(allocatePartyParams));

		allocatePartyParams.monitor_conf_id  = GetBridge()->GetConf()->GetMonitorConfId();                // m_monitorConfId;
		allocatePartyParams.monitor_party_id = m_monitorPartyId;
		allocatePartyParams.party_id         = GetLookupIdParty()->Alloc();
		allocatePartyParams.networkPartyType = eIP_network_party_type;                                    // No extra resources are needed as for ISDN
		if ((eCascadeOptimizeResolutionEnum)m_isContentHD1080Supported == e_res_1080_60fps)
			allocatePartyParams.videoPartyType = eCP_Content_for_Legacy_Decoder_HD1080_60FS_video_party_type;
		else if (m_isContentHD1080Supported != 0)
			allocatePartyParams.videoPartyType = eCP_Content_for_Legacy_Decoder_HD1080_video_party_type;
		else
			allocatePartyParams.videoPartyType = eCP_Content_for_Legacy_Decoder_video_party_type;           // For now SD resource

		allocatePartyParams.sessionType              = eCP_session;
		allocatePartyParams.serviceId                = 0;                                                 // As default m_ServiceId in PartyCntl;
		allocatePartyParams.optionsMask              = 0;
		allocatePartyParams.allocationPolicy         = eAllocateAllRequestedResources;
		allocatePartyParams.isWaitForRsrcAndAskAgain = NO;

		CCommConf* pConf = ::GetpConfDB()->GetCurrentConf(GetBridge()->GetConf()->GetMonitorConfId());
		if (pConf)                                                                                        // use the conf service id for multiple service env
		{
			const char* conf_serv_name = pConf->GetServiceNameForMinParties();
			CConfIpParameters* pServiceParams = ::GetIpServiceListMngr()->GetRelevantService(conf_serv_name, H323_INTERFACE_TYPE);
			if (pServiceParams != NULL)
				allocatePartyParams.serviceId = pServiceParams->GetServiceId();
		}

		TRACEINTO << m_partyConfName << allocatePartyParams;

		CSegment* seg = new CSegment;
		seg->Put((BYTE*)(&allocatePartyParams), sizeof(allocatePartyParams));
		STATUS res = SendReqToResourceAllocator(seg, ALLOCATE_PARTY_RSRC_REQ);
		PASSERT(res);
	}
	else
	{
		CPartyRsrcDesc* pPartyAllocatedRsrc = ((CVideoBridge*)GetBridge())->GetXCodeContentDecoderPartyRsrc();
		PASSERT_AND_RETURN(!pPartyAllocatedRsrc);

		DWORD decoderMonitorPartyID = ((CVideoBridge*)GetBridge())->GetContentDecoderXCodeMonitorPartyId();
		SetMonitoringPartyId(decoderMonitorPartyID);
		CSegment* pSeg = new CSegment;
		pPartyAllocatedRsrc->Serialize(NATIVE, *pSeg);
		DispatchEvent(ALLOCATE_PARTY_RSRC_IND, pSeg);
		POBJDELETE(pPartyAllocatedRsrc);
	}
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::OnRsrcAllocatePartyRspALLOCATE(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CVideoBridgePartyCntlContent::OnRsrcAllocatePartyRspALLOCATE: ", m_partyConfName);

	if (IsValidTimer(RACONNECTTOUT))
		DeleteTimer(RACONNECTTOUT);

	CPartyRsrcDesc* pPartyAllocatedRsrc = new CPartyRsrcDesc; AUTO_DELETE(pPartyAllocatedRsrc);
	if(!((CVideoBridge*)GetBridge())->IsXCodeConf())
	{
		pPartyAllocatedRsrc->DeSerialize(SERIALEMBD, *pParam);
	}
	else
		pPartyAllocatedRsrc->DeSerialize(NATIVE, *pParam);

	DWORD status = pPartyAllocatedRsrc->GetStatus();
	if (status != STATUS_OK)
	{
		CMedString* pStr = new CMedString;
		*pStr <<"ALLOC_PARTY_IND_PARAMS_S:\n"
				<< "status     =   "<< CProcessBase::GetProcess()->GetStatusAsString(status).c_str();
		PTRACE(eLevelInfoNormal, pStr->GetString());
		POBJDELETE(pStr);

		HandleRsrcAllocatePartyFailure();
		return;
	}

	ResetDecoderFailureStatusInConf();

	m_pPartyAllocatedRsrc = new CPartyRsrcDesc(*pPartyAllocatedRsrc);

	InsertPartyResourcesToGlobalRsrcRoutingTbl();

	POBJDELETE(pPartyAllocatedRsrc);

	if (YES == IsDestroy())
	{
		PTRACE2(eLevelInfoNormal, "CVideoBridgePartyCntlContent::OnRsrcAllocatePartyRspALLOCATE: Conf Deleted while Allocate==>Deallocate", m_partyConfName);
		DeAllocate();
	}
	else
	{
		if (YES == IsDisconnect())
		{
			m_state = DISCONNECTED;
			ResetConnectDisconnectDestroyFlags();
		}

		else
		{
			SetDecoderParamsBeforeConnect();

			Setup();
		}
	}
}

void CVideoBridgePartyCntlContent::SetDecoderParamsBeforeConnect()
{
	BYTE  contentAlgorithem = ((CVideoBridgeCPContent*)GetBridge())->GetContentProtocol();
	DWORD contentRate       = ((CVideoBridgeCPContent*)GetBridge())->GetContentRate();

	CDwordBitMask muteMask;
	muteMask.ResetMask();

	DWORD contentFS   = 0;
	DWORD contentMBPS = 0;
	eVideoResolution videoResolution = eVideoResolutionDummy;

	switch ((eCascadeOptimizeResolutionEnum)m_isContentHD1080Supported)
	{
	case e_res_1080_60fps:
		videoResolution = eVideoResolutionHD1080;
		contentFS   = H264_HD1080_FS_AS_DEVISION;
		contentMBPS = 980;  // The division operation is heavy(H264_L4_2_DEFAULT_MBPS/CUSTOM_MAX_MBPS_FACTOR)=491520/500=980
		break;
	case e_res_1080_30fps:
		videoResolution = eVideoResolutionHD1080;
		contentFS   = H264_HD1080_FS_AS_DEVISION;
		contentMBPS = 490;  // The division operation is heavy(H264_L4_DEFAULT_MBPS/CUSTOM_MAX_MBPS_FACTOR)=245760/500=490
		break;
	case e_res_1080_15fps:
		videoResolution = eVideoResolutionHD1080;
		contentFS   = H264_HD1080_FS_AS_DEVISION;
		contentMBPS = 245;  // The division operation is heavy(H264_L4_DEFAULT_MBPS/CUSTOM_MAX_MBPS_FACTOR)/2=245760/500/2=245
		break;
	default:
		videoResolution = eVideoResolutionSVGA; /*for H263 Res_Ratio16*/
		contentFS   = H264_HD720_FS_AS_DEVISION;
		contentMBPS = 108;                  // The division operation is heavy(H264_L3_1_DEFAULT_MBPS/CUSTOM_MAX_MBPS_FACTOR)/2=108000/500/2=108
		break;
	}

	CBridgePartyVideoInParams* pInVideoParams = new CBridgePartyVideoInParams(contentAlgorithem, contentRate, eVideoFrameRate30FPS,
	                                                                          eVideoFrameRate30FPS, eVideoFrameRate15FPS, videoResolution,
	                                                                          contentMBPS, contentFS, NULL, muteMask, eTelePresencePartyNone, 0,
	                                                                          eVideoFrameRate15FPS, eVideoFrameRate10FPS, eVideoFrameRate7_5FPS);

	// ATTENTION !!! this string "##I_AM_THE_CONTENT_DECODER" for the decoder (site)name used by the Encoder
	// to set the full content of the presentation video without cropping it. Do NOT change it !!
	// This is a temporary patch to avoid API changes until V4.1.1(Yoella)"
	// The Encoder also ignore this SiteName and Do NOT show it on Screen
	CBridgePartyInitParams* pBridgePartyInitParams = new CBridgePartyInitParams("##I_AM_THE_CONTENT_DECODER", NULL, GetPartyRsrcId(), DUMMY_ROOM_ID,
	                                                                            AUTO_INTERFACE_TYPE, pInVideoParams, NULL, this, NULL, NONE);

	pBridgePartyInitParams->SetBridge(this->GetBridge());
	pBridgePartyInitParams->SetConfRsrcID(GetConfRsrcId());

	CVideoBridgePartyInitParams videoBridgePartyInitParams(*pBridgePartyInitParams);

	Create(&videoBridgePartyInitParams);

	POBJDELETE(pInVideoParams);
	POBJDELETE(pBridgePartyInitParams);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::InsertPartyResourcesToGlobalRsrcRoutingTbl()
{
	// we need to set this function to be used both by partycntl and bridgepartycntl - a global func to
	// surve for RA OR to create a dummy partycntl only to send this message
	// or to set a partyCntl as member of this class

	CConfPartyRoutingTable* pRoutingTable = ::GetpConfPartyRoutingTable();
	PASSERTSTREAM_AND_RETURN(!pRoutingTable, "PartyId:" << GetPartyRsrcId() << " does not exist");

	// Add general Party Entry to Routing Table
	CPartyRsrcRoutingTblKey routingKey = CPartyRsrcRoutingTblKey(DUMMY_CONNECTION_ID, GetPartyRsrcId(), eLogical_res_none);
	pRoutingTable->AddPartyRsrcDesc(routingKey);

	// Add ptr to Conf for general party entry
	CTaskApi* pTaskApi                 = new CTaskApi(*GetBridge()->GetConfApi());
	pTaskApi->CreateOnlyApi(GetBridge()->GetConfApi()->GetRcvMbx(), NULL);
	pRoutingTable->AddStateMachinePointerToRoutingTbl(GetPartyRsrcId(), eLogical_res_none, pTaskApi);

	// Add ,for each party resource, entry to Routing Table
	m_pPartyAllocatedRsrc->InsertToGlobalRsrcRoutingTbl();

	POBJDELETE(pTaskApi);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::Setup()
{
	UpdateVideoInParamsOnChange(); // For cases that the content caps had been changed
	CVideoBridgePartyCntl::Setup();
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::UpdateVideoInParamsOnChange()
{
	// Content Rate and Algorithem might changed while m_pBridgePartyIn is Disconnected ,before connecting the ContentDecoder we have to update the
	// Created ContentDecoder,with the new Algo and rate, it is NOT a regular update while the decoder is ON.
	DWORD contentAlgorithem = ((CVideoBridgeCPContent*)GetBridge())->GetContentProtocol();
	DWORD contentRate       = ((CVideoBridgeCPContent*)GetBridge())->GetContentRate();

	CSegment* pSeg = new CSegment;
	*pSeg << contentAlgorithem << contentRate << m_isContentHD1080Supported;

	if (CPObject::IsValidPObjectPtr(m_pBridgePartyIn))
		((CBridgePartyVideoInContent*)m_pBridgePartyIn)->UpdateVideoParams(pSeg);

	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::HandleRsrcAllocatePartyFailure()
{
	TRACESTRFUNC(eLevelError) << m_partyConfName;
	m_state = IDLE;
	m_pConfApi->ContenetDecoderRsrcAllocFailure(NULL, VIDEO_BRIDGE_MSG, statOK);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::ResetDecoderFailureStatusInConf()
{
	TRACESTRFUNC(eLevelError) << m_partyConfName;
	m_pConfApi->ContentDecoderResetFailStatus(NULL, VIDEO_BRIDGE_MSG, statOK);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::OnVideoBridgeConnectALLOCATE(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	SetConnectDisconnectDestroyFlags(eContentDecoderAction_Connect);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::OnVideoBridgeConnectSETUP(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	SetConnectDisconnectDestroyFlags(eContentDecoderAction_Connect);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::OnVideoBridgeConnectCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_partyConfName << " - ContentDecoder is already connected,simulate as it send us sync";
	SetConnectDisconnectDestroyFlags(eContentDecoderAction_Connect);
	OnVideoInSynced(pParam); // simulate sync to update the Legacy partyLayout
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::OnVideoBridgeConnectDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	SetConnectDisconnectDestroyFlags(eContentDecoderAction_Connect);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::OnVideoBridgeConnectDISCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	SetConnectDisconnectDestroyFlags(eContentDecoderAction_Connect);
	Setup();
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::OnVideoBridgeConnectDEALLOCATE(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	SetConnectDisconnectDestroyFlags(eContentDecoderAction_Connect);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::DeAllocate()
{
	m_state = DEALLOCATE;
	DeAllocateResources();
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::DeAllocateResources()
{
	if (m_pPartyAllocatedRsrc && (m_pPartyAllocatedRsrc->GetStatus() == STATUS_OK))
	{
		TRACEINTO << m_partyConfName;
		m_state = DEALLOCATE;
		StartTimer(RADISCONNECTTOUT, RA_DISCONNECT_TIME * SECOND);
		CreateAndSendDeallocatePartyResources();
	}
	else
	{
		TRACEINTO << m_partyConfName << " - Don't send message to RA, the allocation failed or ContentDecoder was destroyed NO rsrcs to DeAllocated";
		DBGPASSERT(1);
		m_pConfApi->PartyBridgeResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, END_DESTROY_CONTENT_DECODER, statVideoInOutResourceProblem, FALSE, eNoDirection);
	}
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::CreateAndSendDeallocatePartyResources()
{
	if (!CPObject::IsValidPObjectPtr(m_pPartyAllocatedRsrc))
		return;

	DEALLOC_PARTY_REQ_PARAMS_S deallocatePartyParams;
	memset(&deallocatePartyParams, 0, sizeof(DEALLOC_PARTY_REQ_PARAMS_S));

	deallocatePartyParams.monitor_conf_id      = GetBridge()->GetConf()->GetMonitorConfId();
	deallocatePartyParams.monitor_party_id     = m_monitorPartyId;
	deallocatePartyParams.force_kill_all_ports = FALSE;

	if (m_isFaulty == 1) // Invoking KillPort process in RA.
	{
		TRACEINTO << m_partyConfName << " - Faulty flag is ON";
		DWORD rsrcsWithProblems_constValue = 6; // (eLogical_audio_encoder,eLogical_audio_decoder,eLogical_video_encoder,eLogical_video_decoder,eLogical_rtp,Logical_ip_signaling)

		// Faulty resources
		DWORD connectionId = 0;
		connectionId = m_pPartyAllocatedRsrc->GetConnectionId(eLogical_audio_encoder);
		deallocatePartyParams.rsrcsWithProblems[eLogical_audio_encoder].connectionId    = connectionId;
		deallocatePartyParams.rsrcsWithProblems[eLogical_audio_encoder].logicalRsrcType = eLogical_audio_encoder;

		connectionId = m_pPartyAllocatedRsrc->GetConnectionId(eLogical_audio_decoder);
		deallocatePartyParams.rsrcsWithProblems[eLogical_audio_decoder].connectionId    = connectionId;
		deallocatePartyParams.rsrcsWithProblems[eLogical_audio_decoder].logicalRsrcType = eLogical_audio_decoder;

		connectionId = m_pPartyAllocatedRsrc->GetConnectionId(eLogical_rtp);
		deallocatePartyParams.rsrcsWithProblems[eLogical_rtp].connectionId              = connectionId;
		deallocatePartyParams.rsrcsWithProblems[eLogical_rtp].logicalRsrcType           = eLogical_rtp;

		connectionId = m_pPartyAllocatedRsrc->GetConnectionId(eLogical_video_encoder);
		deallocatePartyParams.rsrcsWithProblems[eLogical_video_encoder].connectionId    = connectionId;
		deallocatePartyParams.rsrcsWithProblems[eLogical_video_encoder].logicalRsrcType = eLogical_video_encoder;

		connectionId = m_pPartyAllocatedRsrc->GetConnectionId(eLogical_video_decoder);
		deallocatePartyParams.rsrcsWithProblems[eLogical_video_decoder].connectionId    = connectionId;
		deallocatePartyParams.rsrcsWithProblems[eLogical_video_decoder].logicalRsrcType = eLogical_video_decoder;


		deallocatePartyParams.is_problem_with_UDP_ports = 1;
		deallocatePartyParams.numOfRsrcsWithProblems    = rsrcsWithProblems_constValue;
	}

	std::ostringstream msg;
	msg <<"DEALLOC_PARTY_REQ_PARAMS_S(" << m_partyConfName << ")"
			<< "\n  monitorConfId          :" << deallocatePartyParams.monitor_conf_id
			<< "\n  monitorPartyId         :" << deallocatePartyParams.monitor_party_id
			<< "\n  serviceName            :" << (char*)deallocatePartyParams.serviceName
			<< "\n  resetArtUnitOnKillPort :" << (WORD)deallocatePartyParams.resetArtUnitOnKillPort;  // Always NO - should be 0 from memset Should be YES only in PartyCntl function

	TRACEINTO << msg.str().c_str();

	CSegment* seg = new CSegment;
	seg->Put((BYTE*)(&deallocatePartyParams), sizeof(DEALLOC_PARTY_REQ_PARAMS_S));

	STATUS res = SendReqToResourceAllocator(seg, DEALLOCATE_PARTY_RSRC_REQ);
	PASSERT(res);
}

//--------------------------------------------------------------------------
STATUS CVideoBridgePartyCntlContent::SendReqToResourceAllocator(CSegment* seg, OPCODE opcode)
{
	CManagerApi api(eProcessResource);
	const StateMachineDescriptor stateMachine = GetStateMachineDescriptor();
	STATUS res = api.SendMsg(seg, opcode, &m_pConfApi->GetRcvMbx(), &stateMachine);
	return res;
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::OnRsrcDeallocatePartyRspDEALLOCATE(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;

	if (IsValidTimer(RADISCONNECTTOUT))
		DeleteTimer(RADISCONNECTTOUT);

	DWORD structLen = sizeof(DEALLOC_PARTY_IND_PARAMS_S);
	DEALLOC_PARTY_IND_PARAMS_S tDeallocatePartyIndParams;
	memset(&tDeallocatePartyIndParams, 0, structLen);
	pParam->Get((BYTE*)(&tDeallocatePartyIndParams), structLen);

	TRACEINTO << m_partyConfName << ", status:" << CProcessBase::GetProcess()->GetStatusAsString(tDeallocatePartyIndParams.status).c_str();

	if (tDeallocatePartyIndParams.status != STATUS_OK)
	{
		DBGPASSERT(tDeallocatePartyIndParams.status);
	}

	if (NO == IsDestroy())  // We are here in case that connect fail and we have decided to deallocate and now Deallocate success or fail, anyway send response to the bridge
	{
		ResetConnectDisconnectDestroyFlags();
		m_state = IDLE;
		m_pConfApi->PartyBridgeResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, END_DESTROY_CONTENT_DECODER, tDeallocatePartyIndParams.status, FALSE, eNoDirection);
		if (((CVideoBridge*)GetBridge())->IsXCodeConf())
			m_pConfApi->PartyBridgeResponseMsg(m_pParty, XCODE_BRDG_MSG, END_DESTROY_CONTENT_DECODER, tDeallocatePartyIndParams.status, FALSE, eNoDirection);
	}
	else // Destroy (DeleteConf)
	{
		// remove party resources from global resource/routing table
		if (::CPObject::IsValidPObjectPtr(m_pPartyAllocatedRsrc))
		{
			m_pConfApi->PartyBridgeResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, END_DESTROY_CONTENT_DECODER, statOK, FALSE, eNoDirection);
			m_pPartyAllocatedRsrc->DeleteFromGlobalRsrcRoutingTbl();
			POBJDELETE(m_pPartyAllocatedRsrc);
		}
		else
			PASSERTMSG(GetPartyRsrcId(), "m_pPartyAllocatedRsrc not valid");
	}
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::OnTimerRADisconnectDEALLOCATE(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	DBGPASSERT(1);

	if (NO == IsDestroy())
	{
		ResetConnectDisconnectDestroyFlags();
		m_state = IDLE;
		m_pConfApi->PartyBridgeResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, END_DESTROY_CONTENT_DECODER, statVideoInOutResourceProblem, FALSE, eNoDirection);
		if (((CVideoBridge*)GetBridge())->IsXCodeConf())
			m_pConfApi->PartyBridgeResponseMsg(m_pParty, XCODE_BRDG_MSG, END_DESTROY_CONTENT_DECODER, statVideoInOutResourceProblem, FALSE, eNoDirection);
	}
	else // Destroy (DeleteConf)
	{
		// remove party resources from global resource/routing table
		if (::CPObject::IsValidPObjectPtr(m_pPartyAllocatedRsrc))
		{
			m_pConfApi->PartyBridgeResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, END_DESTROY_CONTENT_DECODER, statOK, FALSE, eNoDirection);
			m_pPartyAllocatedRsrc->DeleteFromGlobalRsrcRoutingTbl();
			POBJDELETE(m_pPartyAllocatedRsrc);
		}
		else
			PASSERTMSG(GetPartyRsrcId(), "m_pPartyAllocatedRsrc not valid");
	}
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::OnVideoBridgeDisconnectSETUP(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	SetConnectDisconnectDestroyFlags(eContentDecoderAction_Disconnect);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::OnVideoBridgeDisconnectALLOCATE(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	SetConnectDisconnectDestroyFlags(eContentDecoderAction_Disconnect);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::OnVideoBridgeDisconnectCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	SetConnectDisconnectDestroyFlags(eContentDecoderAction_Disconnect);
	OnVideoBridgeDisconnect(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::OnVideoBridgeDisconnectDEALLOCATE(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	SetConnectDisconnectDestroyFlags(eContentDecoderAction_Disconnect);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::OnVideoBridgeDisconnectDISCONNECTING(CSegment* pParams)
{
	TRACEINTO << m_partyConfName << " - Content Decoder is already disconnecting";
	SetConnectDisconnectDestroyFlags(eContentDecoderAction_Disconnect);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::OnVideoBridgeDisconnectDISCONNECTED(CSegment* pParams)
{
	TRACEINTO << m_partyConfName << " - Content Decoder is already disconned";
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::OnVideoBridgeDisconnect(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;

	StartTimer(VIDEO_BRDG_PARTY_DISCONNECT_TOUT, VIDEO_BRDG_PARTY_DISCONNECT_TOUT_VALUE);

	// Disconnect Video In
	if (m_pBridgePartyIn)
		m_pBridgePartyIn->DisConnect();

	if (m_state == SETUP)
		DeleteTimer(VIDEO_BRDG_PARTY_SETUP_TOUT);

	m_state = DISCONNECTING;
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::OnVideoBridgeDestroyIDLE(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	m_pConfApi->PartyBridgeResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, END_DESTROY_CONTENT_DECODER, statOK, FALSE, eNoDirection);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::OnVideoBridgeDestroySETUP(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	SetConnectDisconnectDestroyFlags(eContentDecoderAction_Destroy);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::OnVideoBridgeDestroyALLOCATE(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	SetConnectDisconnectDestroyFlags(eContentDecoderAction_Destroy);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::OnVideoBridgeDestroyCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	SetConnectDisconnectDestroyFlags(eContentDecoderAction_Destroy);
	OnVideoBridgeDisconnect(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::OnVideoBridgeDestroyDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	SetConnectDisconnectDestroyFlags(eContentDecoderAction_Destroy);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::OnVideoBridgeDestroyDISCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	SetConnectDisconnectDestroyFlags(eContentDecoderAction_Destroy);
	DeAllocate();
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::OnVideoBridgeDestroyDEALLOCATE(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	SetConnectDisconnectDestroyFlags(eContentDecoderAction_Destroy);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::Create(const CVideoBridgePartyInitParams* pBridgePartyInitParams)
{
	if (NULL == pBridgePartyInitParams) {
		PASSERT_AND_RETURN(101);
	}

	if (!pBridgePartyInitParams->GetMediaInParams()) {
		PASSERT_AND_RETURN(102);
	}

	CConfPartyRoutingTable* pRoutingTable = ::GetpConfPartyRoutingTable();
	if (NULL == pRoutingTable) {
		PASSERT_AND_RETURN(103);
	}

	// Create base params to set a new function
	// CBridgePartyCntl::Create(pBridgePartyInitParams);exceptions since NO party and conf
	strncpy(m_name, pBridgePartyInitParams->GetPartyName(), H243_NAME_LEN-1);
	m_name[H243_NAME_LEN -1] = '\0';

	SetFullName(pBridgePartyInitParams->GetPartyName(), pBridgePartyInitParams->GetConfName());

	m_pBridge           = (CBridge*)(pBridgePartyInitParams->GetBridge());
	m_pParty            = (CTaskApp*)(pBridgePartyInitParams->GetParty());
	m_partyRsrcID       = pBridgePartyInitParams->GetPartyRsrcID();
	m_confRsrcID        = pBridgePartyInitParams->GetConfRsrcID();
	m_wNetworkInterface = pBridgePartyInitParams->GetNetworkInterface();
	m_bCascadeLinkMode  = pBridgePartyInitParams->GetCascadeLinkMode();

	CRsrcParams* pRsrcParams  = new CRsrcParams;
	pRsrcParams->SetConfRsrcId(m_confRsrcID);
	pRsrcParams->SetPartyRsrcId(m_partyRsrcID);

	CRsrcDesc* pRsrcDesc      = NULL;

	m_pBridgePartyIn = new CBridgePartyVideoInContent;
	CTaskApi* pTaskApiVideoIn = new CTaskApi(*m_pConfApi);
	pTaskApiVideoIn->CreateOnlyApi(m_pConfApi->GetRcvMbx(), m_pBridgePartyIn);
	pRsrcDesc        = pRoutingTable->AddStateMachinePointerToRoutingTbl(m_partyRsrcID, eLogical_video_decoder, pTaskApiVideoIn);

	if (!pRsrcDesc) // Entry not found in Routing Table
	{
		POBJDELETE(m_pBridgePartyIn);
		TRACESTRFUNC(eLevelError) << m_partyConfName << " - Video-In creation failure";
		PASSERT(104);
	}
	else
	{
		pRsrcParams->SetRsrcDesc(*pRsrcDesc);
		((CBridgePartyVideoIn*)(m_pBridgePartyIn))->Create(this, pRsrcParams, pBridgePartyInitParams->GetMediaInParams());
	}

	POBJDELETE(pTaskApiVideoIn);
	POBJDELETE(pRsrcParams);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::VideoConnectionCompletion(CSegment* pParams, EMediaDirection eConnectedMediaDirection)
{
	if (!m_pBridgePartyIn) {
		DBGPASSERT_AND_RETURN(1);
	}

	BOOL  isVideoConnectionCompleted = FALSE;
	EStat receivedStatus             = statOK;

	*pParams >> (BYTE&)receivedStatus;

	TRACEINTO << m_partyConfName << ", Status:" << (WORD)receivedStatus;
	if (statOK != receivedStatus)
	{
		DeleteTimer(VIDEO_BRDG_PARTY_SETUP_TOUT);
		DumpMcuInternalProblemDetailed((BYTE)eConnectedMediaDirection, eMipStatusFail, eMipVideo);

		// VNGR-13446 - ERROR handling fix - conf was NOT terminated
		ON(m_isFaulty);
		DeAllocate();
		return;
	}

	DeleteTimer(VIDEO_BRDG_PARTY_SETUP_TOUT);

	m_state = CONNECTED;

	if (YES == IsDisconnect())   // case of destroy or disconnect
		DisConnect();

	else
		m_pConfApi->PartyBridgeResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, ENDCONNECT_CONTENT_DECODER, statOK, FALSE, eNoDirection);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::VideoDisConnectionCompletion(CSegment* pParams, EMediaDirection eDisConnectedMediaDirection)
{
	TRACEINTO << m_partyConfName;

	if (!m_pBridgePartyIn || !(eMediaIn == eDisConnectedMediaDirection)) {
		DBGPASSERT_AND_RETURN(1);
	}

	EStat receivedStatus         = statOK;
	BYTE  videoInClosePortStatus = statOK;

	*pParams >> (BYTE&)receivedStatus;

	DeleteTimer(VIDEO_BRDG_PARTY_DISCONNECT_TOUT);
	m_state = DISCONNECTED;

	videoInClosePortStatus = m_pBridgePartyIn->GetClosePortAckStatus();

	// for debug info in case of the "MCU internal problem"
	BYTE failureCauseDirection = eMipNoneDirction;

	if (videoInClosePortStatus != STATUS_OK)
	{
		receivedStatus = (EStat)videoInClosePortStatus;
		failureCauseDirection = eMipIn;
	}

	// Inform Video Bridge In case of problem
	if (statVideoInOutResourceProblem == receivedStatus)
	{
		CSegment* pSeg = new CSegment;
		*pSeg << (BYTE)receivedStatus << (BYTE)failureCauseDirection << (BYTE)eMipStatusFail <<(BYTE)eMipClose;
		if (GetConnectionFailureCause() == statOK)
		{
			DumpMcuInternalProblemDetailed((BYTE)failureCauseDirection, eMipStatusFail, eMipVideo);
		}

		m_pConfApi->PartyBridgeResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, END_DISCONNECT_CONTENT_DECODER, receivedStatus, FALSE, eNoDirection, pSeg);
		POBJDELETE(pSeg);
	}
	else
	{
		if (YES == IsDestroy())
			DeAllocate();

		else
		{
			if (YES == IsConnect()) // In case we received connect req while disconnecting
			{
				TRACEINTO << m_partyConfName << " - Video In was disconnected, connect received, start Connect process";
				Setup();
			}
			else // Destroy is NO and Disconnect is YES
			{
				TRACEINTO << m_partyConfName << " - Both Direction were disconnected, state is IDLE";
				// Inform Video Bridge - Add the direction connected state
				m_pConfApi->PartyBridgeResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, END_DISCONNECT_CONTENT_DECODER, receivedStatus, FALSE, eNoDirection);
			}
		}
	}
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::OnTimerPartySetup(CSegment* pParams)
{
	CMedString encoderString, decoderString;
	CMedString logStr;
	logStr << "CVideoBridgePartyCntlContent::OnTimerPartySetup ==> Deallocate the content decoder: Name - " << m_partyConfName;

	if (CPObject::IsValidPObjectPtr(m_pBridgePartyIn))
	{
		logStr <<" CBridgePartyVideoIn State: ";
		((CBridgePartyVideoIn*)m_pBridgePartyIn)->DumpAllInfoOnConnectionState(&logStr);
		((CBridgePartyVideoIn*)m_pBridgePartyIn)->DumpAllInfoOnConnectionState(&decoderString, true);
	}

	logStr <<"\nm_partyRsrcID = " << m_partyRsrcID << " , m_confRsrcID = " << m_confRsrcID;
	PTRACE(eLevelError, logStr.GetString());

	// Add Fault to EMA
	std::string faultString = "Video Decoder:";
	faultString += decoderString.GetString();
	CBridgePartyMediaUniDirection::AddFaultAlarm(faultString.c_str(), m_partyRsrcID, STATUS_OK, true);

	// for debug info in case of the "MCU internal problem"
	BYTE failureCauseDirection = eMipNoneDirction;
	BYTE failureCauseAction    = eMipNoAction;
	CSegment* pSeg = NULL;
	GetRsrcProbAdditionalInfoOnVideoTimerSetup(failureCauseDirection, failureCauseAction);
	ON(m_isFaulty);
	DeAllocate();
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::OnTimerPartyDisconnectSETUP(CSegment* pParams)
{
	TRACEINTO << m_partyConfName;
	// DO NOTHING since when Disconnect arrive on setup ue gust set flags and did NOT open timer like in father!!!!
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::OnTimerPartyDisconnectCONNECTED(CSegment* pParams)
{
	TRACEINTO << m_partyConfName;
	// DO NOTHING since when Disconnect arrive on connect ,state is set to disconnecting!!!!
}

//--------------------------------------------------------------------------
// NEED TO GO OVER ALL ENDDISCONNECTPARTY and block it !!!!!
void CVideoBridgePartyCntlContent::OnTimerPartyDisconnectDISCONNECTING(CSegment* pParams)
{
	TRACEINTO << m_partyConfName << ", PartyId:" << m_partyRsrcID << ", ConfId:" << m_confRsrcID;

	// Add Fault to EMA
	CBridgePartyMediaUniDirection::AddFaultAlarm("Did not receive all ACKs in Video disconnection", m_partyRsrcID, STATUS_OK, true);

	m_state = DISCONNECTED;

	// Inform Video Bridge
	CSegment* pSeg = new CSegment;
	*pSeg << (BYTE)statTout << (BYTE)eMipIn << (BYTE)eMipStatusFail << (BYTE)eMipClose;
	m_pConfApi->PartyBridgeResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, END_DISCONNECT_CONTENT_DECODER, statTout, FALSE, eNoDirection, pSeg);
	POBJDELETE(pSeg);

	PASSERT(!CPObject::IsValidPObjectPtr(m_pBridgePartyIn) && !CPObject::IsValidPObjectPtr(m_pBridgePartyOut));

	// VNGFE-6359 D.K. Inform Conference to disconnect Video Bridge with TIMEOUT status
	m_pConfApi->EndVidBrdgDisConnect(statTout);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::OnVideoOutVideoRefresh(CSegment* pParam)
{
	m_pConfApi->ContenetDecoderSyncLost(NULL, VIDEO_BRIDGE_MSG, statOK);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::OnVideoInSynced(CSegment* pParam)
{
	m_pConfApi->ContentDecoderVideoInSynced(NULL, VIDEO_BRIDGE_MSG, statOK);

	if(((CVideoBridge*)GetBridge())->IsXCodeConf())
	{
		m_pConfApi->ContentDecoderVideoInSynced(NULL, XCODE_BRDG_MSG, statOK);
	}
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::UpdateDecoderDetectedMode()
{
	BOOL isXcodeConf = ((CVideoBridge*)GetBridge())->IsXCodeConf();

	TRACEINTO << m_partyConfName << ", PartyId: " << m_partyRsrcID
		<< ", ConfId: " << m_confRsrcID << ", isXcodeConf: " << (WORD)isXcodeConf;

	if (isXcodeConf)
	{
		m_pConfApi->ContentDecoderVideoInSynced(NULL, XCODE_BRDG_MSG, statOK);
	}
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::SetConnectDisconnectDestroyFlags(eContentDecoderAction action)
{
	switch (action)
	{
		case eContentDecoderAction_Connect:
		{
			if (NO == m_isDestroy)
			{
				m_isDisconnect = NO;
				m_isConnect    = YES;
			}

			break;
		}

		case eContentDecoderAction_Disconnect:
		{
			if (NO == m_isDestroy)
			{
				m_isDisconnect = YES;
				m_isConnect    = NO;
			}

			break;
		}

		case eContentDecoderAction_Destroy:
		{
			m_isDestroy    = YES;
			m_isDisconnect = YES;
			break;
		}

		default:
			TRACESTRFUNC(eLevelError) << m_partyConfName << " - Error, action is not valid";
			break;
	} // switch
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::ResetConnectDisconnectDestroyFlags()
{
	m_isDisconnect = NO;
	m_isConnect    = NO;
	m_isDestroy    = NO;
}

//--------------------------------------------------------------------------
PartyRsrcID CVideoBridgePartyCntlContent::GetPartyRsrcId()
{
	if (m_pPartyAllocatedRsrc)
		return m_pPartyAllocatedRsrc->GetPartyRsrcId();

	return 0;
}

//--------------------------------------------------------------------------
ConfRsrcID CVideoBridgePartyCntlContent::GetConfRsrcId()
{
	if (m_pPartyAllocatedRsrc)
		return m_pPartyAllocatedRsrc->GetConfRsrcId();

	return 0;
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::Destroy()
{
	DispatchEvent(VIDDESTROY, NULL);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlContent::OnVideoInDisconnected(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	VideoDisConnectionCompletion(pParam, eMediaIn);
}

//--------------------------------------------------------------------------
BOOL CVideoBridgePartyCntlContent::IsContentDecoderSynced()
{
	BOOL isContentDecoderSynced = NO;
	if (m_pBridgePartyIn && (!((CBridgePartyVideoIn*)m_pBridgePartyIn)->IsPartyImageSyncLoss()))
		isContentDecoderSynced = YES;

	return isContentDecoderSynced;
}
PBEGIN_MESSAGE_MAP(CVideoBridgePartyCntlLegacy)

	ONEVENT(ADD_CONTENT_IMAGE, SETUP,                 CVideoBridgePartyCntlLegacy::OnVideoBridgeAddContentImageSETUP)
	ONEVENT(ADD_CONTENT_IMAGE, CONNECTED,             CVideoBridgePartyCntlLegacy::OnVideoBridgeAddContentImageCONNECTED)
	ONEVENT(ADD_CONTENT_IMAGE, CONNECTED_STANDALONE,  CVideoBridgePartyCntlLegacy::NullActionFunction)
	ONEVENT(ADD_CONTENT_IMAGE, DISCONNECTING,         CVideoBridgePartyCntlLegacy::NullActionFunction)

	ONEVENT(DEL_CONTENT_IMAGE, SETUP,                 CVideoBridgePartyCntlLegacy::OnVideoBridgeDelContentImageSETUP)
	ONEVENT(DEL_CONTENT_IMAGE, CONNECTED,             CVideoBridgePartyCntlLegacy::OnVideoBridgeDelContentImageCONNECTED)
	ONEVENT(DEL_CONTENT_IMAGE, CONNECTED_STANDALONE,  CVideoBridgePartyCntlLegacy::NullActionFunction)
	ONEVENT(DEL_CONTENT_IMAGE, DISCONNECTING,         CVideoBridgePartyCntlLegacy::NullActionFunction)

PEND_MESSAGE_MAP(CVideoBridgePartyCntlLegacy, CVideoBridgePartyCntl);

//--------------------------------------------------------------------------
CVideoBridgePartyCntlLegacy::CVideoBridgePartyCntlLegacy()
{
}

//--------------------------------------------------------------------------
CVideoBridgePartyCntlLegacy& CVideoBridgePartyCntlLegacy::operator=(const CVideoBridgePartyCntl& rVideoBridgePartyCntl)
{
	if (&rVideoBridgePartyCntl == (CVideoBridgePartyCntl*)this)
		return *this;

	(CVideoBridgePartyCntl&)(*this) = (CVideoBridgePartyCntl&)rVideoBridgePartyCntl;

	return *this;
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlLegacy::SetParamsRelatedToMove(BYTE oldPartyState)
{
	m_state = oldPartyState;
}

//--------------------------------------------------------------------------
CVideoBridgePartyCntlLegacy::~CVideoBridgePartyCntlLegacy()
{ }

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlLegacy::NewPartyOut()
{
	m_pBridgePartyOut = new CBridgePartyVideoOutLegacy();
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlLegacy::AddContentImage()
{
	CSegment* pSeg = new CSegment;

	DispatchEvent(ADD_CONTENT_IMAGE, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlLegacy::OnVideoBridgeAddContentImageSETUP(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoBridgeAddContentImage(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlLegacy::OnVideoBridgeAddContentImageCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoBridgeAddContentImage(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlLegacy::OnVideoBridgeAddContentImage(CSegment* pParam)
{
	if (m_pBridgePartyOut)
		((CBridgePartyVideoOutLegacy*)m_pBridgePartyOut)->AddContentImage();
	else
		TRACEINTO << m_partyConfName << " - Failed, m_pBridgePartyOut is invalid";
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlLegacy::DelContentImage()
{
	CSegment* pSeg = new CSegment;
	DispatchEvent(DEL_CONTENT_IMAGE, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlLegacy::OnVideoBridgeDelContentImageCONNECTED(CSegment* pParam)
{
	OnVideoBridgeDelContentImage(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlLegacy::OnVideoBridgeDelContentImageSETUP(CSegment* pParam)
{
	OnVideoBridgeDelContentImage(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlLegacy::OnVideoBridgeDelContentImage(CSegment* pParam)
{
	if (m_pBridgePartyOut)
		((CBridgePartyVideoOutLegacy*)m_pBridgePartyOut)->DelContentImage();
	else
		TRACEINTO << m_partyConfName << " - Failed, m_pBridgePartyOut is invalid";
}

//--------------------------------------------------------------------------
BOOL CVideoBridgePartyCntlLegacy::IsUpdateLayoutAsPrivateInDB()
{
	BOOL isUpdateLayoutAsPrivateInDB = NO;
	if (YES == ((CVideoBridgeCPContent*)GetBridge())->IsContentImageNeedToBeAdded()) // If we are in active content stage
	{
		if (GetIsPrivateLayout())
			isUpdateLayoutAsPrivateInDB = YES;
	}
	else
	{
		isUpdateLayoutAsPrivateInDB = CVideoBridgePartyCntl::IsUpdateLayoutAsPrivateInDB();
	}

	return isUpdateLayoutAsPrivateInDB;
}


PBEGIN_MESSAGE_MAP(CVideoBridgePartyCntlCOP)

	ONEVENT(PCM_CONNECTED,                    IDLE,                 CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(PCM_CONNECTED,                    SETUP,                CVideoBridgePartyCntlCOP::OnVideoOutConnectedToPCMSETUP)
	ONEVENT(PCM_CONNECTED,                    CONNECTED,            CVideoBridgePartyCntlCOP::OnVideoOutConnectedToPCMCONNECTED)
	ONEVENT(PCM_CONNECTED,                    CONNECTED_STANDALONE, CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(PCM_CONNECTED,                    DISCONNECTING,        CVideoBridgePartyCntlCOP::OnVideoOutConnectedToPCMDISCONNECTING)

	ONEVENT(PCM_DISCONNECTED,                 SETUP,                CVideoBridgePartyCntlCOP::OnVideoOutDisconnectedFromPCMSETUP)
	ONEVENT(PCM_DISCONNECTED,                 CONNECTED,            CVideoBridgePartyCntlCOP::OnVideoOutDisconnectedFromPCMCONNECTED)
	ONEVENT(PCM_DISCONNECTED,                 DISCONNECTING,        CVideoBridgePartyCntlCOP::OnVideoOutDisconnectedFromPCMDISCONNECTING)

	ONEVENT(PCM_UPDATED,                      IDLE,                 CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(PCM_UPDATED,                      SETUP,                CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(PCM_UPDATED,                      CONNECTED,            CVideoBridgePartyCntlCOP::OnPCMVideoOutUpdated)
	ONEVENT(PCM_UPDATED,                      CONNECTED_STANDALONE, CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(PCM_UPDATED,                      DISCONNECTING,        CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(VIDCONNECT,                       SETUP,                CVideoBridgePartyCntlCOP::OnVideoBridgeConnectSETUP)
	ONEVENT(VIDCONNECT,                       CONNECTED,            CVideoBridgePartyCntlCOP::OnVideoBridgeConnectCONNECTED)
	ONEVENT(VIDEO_EXPORT,                     SETUP,                CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(VIDEO_EXPORT,                     CONNECTED,            CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(VIDEO_EXPORT,                     CONNECTED_STANDALONE, CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(VIDEO_EXPORT,                     DISCONNECTING,        CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(VIDEO_IN_SYNCED,                  SETUP,                CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(VIDEO_IN_SYNCED,                  CONNECTED,            CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(VIDEO_IN_SYNCED,                  DISCONNECTING,        CVideoBridgePartyCntlCOP::NullActionFunction)

	ONEVENT(ADDIMAGE,                         SETUP,                CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(ADDIMAGE,                         CONNECTED,            CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(ADDIMAGE,                         DISCONNECTING,        CVideoBridgePartyCntlCOP::NullActionFunction)

	ONEVENT(FASTUPDATE,                       SETUP,                CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(FASTUPDATE,                       CONNECTED,            CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(FASTUPDATE,                       CONNECTED_STANDALONE, CVideoBridgePartyCntlCOP::OnVideoBridgeFastUpdateCONNECTED_STANDALONE)
	ONEVENT(FASTUPDATE,                       DISCONNECTING,        CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(SPEAKERS_CHANGED,                 SETUP,                CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(SPEAKERS_CHANGED,                 CONNECTED,            CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(SPEAKERS_CHANGED,                 DISCONNECTING,        CVideoBridgePartyCntl::NullActionFunction)

	ONEVENT(AUDIO_SPEAKER_CHANGED,            SETUP,                CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(AUDIO_SPEAKER_CHANGED,            CONNECTED,            CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(AUDIO_SPEAKER_CHANGED,            DISCONNECTING,        CVideoBridgePartyCntlCOP::NullActionFunction)

	ONEVENT(DELIMAGE,                         SETUP,                CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(DELIMAGE,                         CONNECTED,            CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(DELIMAGE,                         DISCONNECTING,        CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(CHANGEPARTYLAYOUT,                SETUP,                CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(CHANGEPARTYLAYOUT,                CONNECTED,            CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(CHANGEPARTYLAYOUT,                CONNECTED_STANDALONE, CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(CHANGEPARTYLAYOUT,                DISCONNECTING,        CVideoBridgePartyCntlCOP::NullActionFunction)

	ONEVENT(CHANGEPARTYPRIVATELAYOUT,         SETUP,                CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(CHANGEPARTYPRIVATELAYOUT,         CONNECTED,            CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(CHANGEPARTYPRIVATELAYOUT,         CONNECTED_STANDALONE, CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(CHANGEPARTYPRIVATELAYOUT,         DISCONNECTING,        CVideoBridgePartyCntlCOP::NullActionFunction)

	ONEVENT(SETPARTYPRIVATELAYOUTONOFF,       SETUP,                CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(SETPARTYPRIVATELAYOUTONOFF,       CONNECTED,            CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(SETPARTYPRIVATELAYOUTONOFF,       CONNECTED_STANDALONE, CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(SETPARTYPRIVATELAYOUTONOFF,       DISCONNECTING,        CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(VIDEO_GRAPHIC_OVERLAY_START_REQ,  CONNECTED,            CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(VIDEO_GRAPHIC_OVERLAY_STOP_REQ,   CONNECTED,            CVideoBridgePartyCntlCOP::NullActionFunction)

	ONEVENT(PLC_SETPARTYPRIVATELAYOUTTYPE,    CONNECTED,            CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(PLC_RETURNPARTYTOCONFLAYOUT,      CONNECTED,            CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(PLC_FORCECELLZERO,                CONNECTED,            CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(PLC_CANCELALLPRIVATELAYOUTFORCES, CONNECTED,            CVideoBridgePartyCntlCOP::NullActionFunction)

	ONEVENT(DISPLAY_TEXT_ON_SCREEN,           CONNECTED,            CVideoBridgePartyCntlCOP::NullActionFunction)

	ONEVENT(UPDATE_VIDEO_CLARITY,             IDLE,                 CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(UPDATE_VIDEO_CLARITY,             SETUP,                CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(UPDATE_VIDEO_CLARITY,             CONNECTED,            CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(UPDATE_VIDEO_CLARITY,             CONNECTED_STANDALONE, CVideoBridgePartyCntlCOP::NullActionFunction)
	ONEVENT(UPDATE_VIDEO_CLARITY,             DISCONNECTING,        CVideoBridgePartyCntlCOP::NullActionFunction)

PEND_MESSAGE_MAP(CVideoBridgePartyCntlCOP, CVideoBridgePartyCntl);

////////////////////////////////////////////////////////////////////////////
//                        CVideoBridgePartyCntlCOP
////////////////////////////////////////////////////////////////////////////
CVideoBridgePartyCntlCOP::CVideoBridgePartyCntlCOP()
{
	m_partyFlowControlRate = 0; // 0 means no flow control was received for this party
}

//--------------------------------------------------------------------------
CVideoBridgePartyCntlCOP::CVideoBridgePartyCntlCOP(const CVideoBridgePartyCntlCOP& rOtherBridgePartyCntl)
	: CVideoBridgePartyCntl(rOtherBridgePartyCntl)
{
	m_partyFlowControlRate = rOtherBridgePartyCntl.m_partyFlowControlRate;
}

//--------------------------------------------------------------------------
CVideoBridgePartyCntlCOP& CVideoBridgePartyCntlCOP::operator=(const CVideoBridgePartyCntlCOP& rVideoBridgePartyCntl)
{
	if (&rVideoBridgePartyCntl == (CVideoBridgePartyCntl*)this)
		return *this;

	(CVideoBridgePartyCntl&)(*this) = (CVideoBridgePartyCntl&)rVideoBridgePartyCntl;
	return *this;
}

//--------------------------------------------------------------------------
CVideoBridgePartyCntlCOP::~CVideoBridgePartyCntlCOP()
{
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlCOP::NewPartyOut()
{
	m_pBridgePartyOut = new CBridgePartyVideoOutCOP();
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlCOP::NewPartyIn()
{
	m_pBridgePartyIn = new CBridgePartyVideoInCOP();
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlCOP::Create(const CVideoBridgePartyInitParams* pVideoBridgePartyInitParams)
{
	PASSERT_AND_RETURN(!pVideoBridgePartyInitParams);
	PASSERT_AND_RETURN(!pVideoBridgePartyInitParams->GetMediaInParams() && !pVideoBridgePartyInitParams->GetMediaOutParams());

	CConfPartyRoutingTable* pRoutingTable = ::GetpConfPartyRoutingTable();  // The routing table is relevant only for the out
	PASSERT_AND_RETURN(!pRoutingTable);

	// Create base params
	CBridgePartyCntl::Create(pVideoBridgePartyInitParams);

	m_pUpdatePartyInitParams = NULL;

	CRsrcParams rsrcParams(DUMMY_CONNECTION_ID, m_partyRsrcID, m_confRsrcID);

	// Create BridgePartyVideoIn
	CBridgePartyVideoInParams* videoInParams = (CBridgePartyVideoInParams*)pVideoBridgePartyInitParams->GetMediaInParams();
	if (videoInParams)
	{
		NewPartyIn();
		((CBridgePartyVideoIn*)(m_pBridgePartyIn))->Create(this, &rsrcParams, videoInParams);
	}

	// Create BridgePartyVideoOut
	CBridgePartyVideoOutParams* videoOutParams = (CBridgePartyVideoOutParams*)pVideoBridgePartyInitParams->GetMediaOutParams();
	if (videoOutParams)
	{
		NewPartyOut();
		std::auto_ptr<CTaskApi> pTaskApiVideoOut(new CTaskApi(*m_pConfApi));
		pTaskApiVideoOut->CreateOnlyApi(m_pConfApi->GetRcvMbx(), m_pBridgePartyOut);

		CRsrcDesc* pRsrcDesc = pRoutingTable->AddStateMachinePointerToRoutingTbl(m_partyRsrcID, eLogical_COP_dummy_encoder, pTaskApiVideoOut.get());
		if (!pRsrcDesc)   // Entry not found in Routing Table
		{
			POBJDELETE(m_pBridgePartyOut);
			PASSERT(1);
		}
		else
		{
			rsrcParams.SetRsrcDesc(*pRsrcDesc);
			((CBridgePartyVideoOut*)(m_pBridgePartyOut))->Create(this, &rsrcParams, videoOutParams);
		}
	}
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlCOP::VideoRefresh()
{
	DispatchEvent(VIDREFRESH, NULL);
}

//--------------------------------------------------------------------------
WORD CVideoBridgePartyCntlCOP::IsValidState(WORD state) const
{
	PASSERTSTREAM_AND_RETURN_VALUE(state == EXPORT, m_partyConfName << ", state:" << state, FALSE);

	return TRUE;
}

//--------------------------------------------------------------------------
WORD CVideoBridgePartyCntlCOP::IsValidEvent(OPCODE event) const
{
	switch (event)
	{
		case ADDIMAGE:
		case MUTEIMAGE:
		case UNMUTEIMAGE:
		case SPEAKERS_CHANGED:
		case AUDIO_SPEAKER_CHANGED:
		case DELIMAGE:
		case CHANGECONFLAYOUT:
		case END_CHANGE_LAYOUT:
		case CHANGEPARTYLAYOUT:
		case CHANGEPARTYPRIVATELAYOUT:
		case SETPARTYPRIVATELAYOUTONOFF:
		case VIDEO_GRAPHIC_OVERLAY_START_REQ:
		case VIDEO_GRAPHIC_OVERLAY_STOP_REQ:
		case PLC_SETPARTYPRIVATELAYOUTTYPE:
		case PLC_RETURNPARTYTOCONFLAYOUT:
		case PLC_FORCECELLZERO:
		case PLC_CANCELALLPRIVATELAYOUTFORCES:
		case PRIVATELAYOUT_ONOFF_CHANGED:
		{
			PASSERTSTREAM_AND_RETURN_VALUE(1, m_partyConfName << ", event:" << event, FALSE);
			break;
		}
	} // switch

	return TRUE;
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlCOP::SetDecoderConnectionIdInImage(DWORD decoderConnectionId)
{
	if (m_pBridgePartyIn)
		((CBridgePartyVideoInCOP*)m_pBridgePartyIn)->SetDecoderConnectionIdInImage(decoderConnectionId);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlCOP::SetDecoderPartyIdInImage(DWORD decoderPartyId)
{
	if (m_pBridgePartyIn)
		((CBridgePartyVideoInCOP*)m_pBridgePartyIn)->SetDecoderPartyIdInImage(decoderPartyId);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlCOP::OnVideoBridgeConnectIDLE(CSegment* pParam)
{
	if (!m_pBridgePartyIn && !m_pBridgePartyOut)
	{
		m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, VIDEO_BRIDGE_MSG, ENDCONNECTPARTY, statInconsistent, FALSE, eNoDirection);
		return;
	}

	BYTE isIVR;
	*pParam >> isIVR;

	TRACEINTO << m_partyConfName << ", IsIVR:" << (int)isIVR;

	if (isIVR)
	{
		m_state = CONNECTED_STANDALONE;

		EMediaDirection eDirection = eNoDirection;
		if (m_pBridgePartyIn)
			eDirection |= eMediaIn;

		if (m_pBridgePartyOut)
			eDirection |= eMediaOut;

		// Inform Video Bridge
		m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, VIDEO_BRIDGE_MSG, ENDCONNECTPARTY_IVR_MODE, statOK, FALSE, eDirection);
	}
	else
	{
		Setup();
	}
}

//--------------------------------------------------------------------------
// Incase one of the video channels (In/Out) is already connected and now connects the second direction.
void CVideoBridgePartyCntlCOP::OnVideoBridgeConnectCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	if (m_pBridgePartyIn && m_pBridgePartyOut && m_pBridgePartyOut->IsConnected() && m_pBridgePartyIn->IsConnected())
	{
		// Inform Video Bridge
		m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, VIDEO_BRIDGE_MSG, ENDCONNECTPARTY, statOK, FALSE, eMediaInAndOut);
	}
	else
	{
		OnVideoBridgeConnect(pParam);
	}
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlCOP::OnVideoBridgeConnect(CSegment* pParam)
{
	if (!m_pBridgePartyIn && !m_pBridgePartyOut)
	{
		m_pConfApi->PartyBridgeResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, ENDCONNECTPARTY, statInconsistent, FALSE);
		return;
	}

	BYTE isIVR;
	*pParam >> isIVR;

	// In this stage we shouldn't receive connect with IVR
	DBGPASSERT(isIVR);

	TRACEINTO << m_partyConfName;
	Setup();
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlCOP::OnVideoBridgeDisconnectCONNECTED_STANDALONE(CSegment* pParam)
{
	CVideoBridgePartyCntl::OnVideoBridgeDisconnectCONNECTED_STANDALONE(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlCOP::OnVideoInDisconnected(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;

	PASSERT_AND_RETURN(!m_pBridgePartyIn);
	PASSERT_AND_RETURN(!m_pBridgePartyIn->IsDisConnected());

	VideoDisConnectionCompletion(pParam, eMediaIn);
}

//--------------------------------------------------------------------------
DWORD CVideoBridgePartyCntlCOP::GetCopEncoderEntityId()
{
	DWORD encoderEntity = DUMMY_PARTY_ID;
	if (m_pBridgePartyOut)
		return ((CBridgePartyVideoOutCOP*)m_pBridgePartyOut)->GetCopEncoderId();

	TRACEINTO << m_partyConfName << " - Party does not connected to video-out";
	return DUMMY_PARTY_ID;
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlCOP::OnVideoBridgeJoinConfCONNECTED_STANDALONE(CSegment* pParam)        // IVR_JOIN_CONF_VIDEO
{
	TRACEINTO << m_partyConfName << " - Start setup process";
	Setup();
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlCOP::GetCurrentCopDecoderResolution(DWORD& algorithm, ECopDecoderResolution& copDecoderResolution)
{
	((CBridgePartyVideoInCOP*)m_pBridgePartyIn)->GetCurrentCopDecoderResolution(algorithm, copDecoderResolution);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlCOP::GetInitialCopDecoderResolution(DWORD& algorithm, ECopDecoderResolution& copDecoderResolution)
{
	((CBridgePartyVideoInCOP*)m_pBridgePartyIn)->GetInitialCopDecoderResolution(algorithm, copDecoderResolution);
}

//--------------------------------------------------------------------------
BYTE CVideoBridgePartyCntlCOP::IsSyncWithDecoderResolution()
{
	return ((CBridgePartyVideoInCOP*)m_pBridgePartyIn)->IsSyncWithDecoderResolution();
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlCOP::GetInParamFromPartyCntl(CBridgePartyVideoInParams* pInVideoParams)
{
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(m_pBridgePartyIn));
	((CBridgePartyVideoInCOP*)m_pBridgePartyIn)->GetInVidParams(pInVideoParams);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlCOP::Update(const CVideoBridgePartyInitParams* pBridgePartyInitParams)
{
	PASSERT_AND_RETURN(!pBridgePartyInitParams);
	PASSERT_AND_RETURN(!pBridgePartyInitParams->GetMediaInParams() && !pBridgePartyInitParams->GetMediaOutParams());

	CConfPartyRoutingTable* pRoutingTable = ::GetpConfPartyRoutingTable();
	PASSERT_AND_RETURN(!pRoutingTable);

	CLargeString cstr;

	BOOL bVideoInFailure = FALSE, bVideoOutFailure = FALSE;
	CRsrcParams* pRsrcParams = new CRsrcParams;
	pRsrcParams->SetConfRsrcId(m_confRsrcID);
	pRsrcParams->SetPartyRsrcId(m_partyRsrcID);

	CRsrcDesc* pRsrcDesc = NULL;

	CBridgePartyVideoInParams* videoInParams = (CBridgePartyVideoInParams*)pBridgePartyInitParams->GetMediaInParams();

	cstr << "CVideoBridgePartyCntlCOP::Update - " << GetFullName() << "\n";
	cstr << "m_state = " << m_state <<"\n";
	if (videoInParams)
	{
		cstr << "videoInParams is Valid \n";
		if (!m_pBridgePartyIn)
		{
			cstr << "m_pBridgePartyIn is NULL , create new CBridgePartyVideoIn \n";

			NewPartyIn();

			if (m_pBridgePartyIn)
				((CBridgePartyVideoIn*)(m_pBridgePartyIn))->Create(this, pRsrcParams, pBridgePartyInitParams->GetMediaInParams());
			else
				PASSERTMSG(1, "new m_pBridgePartyIn failed");
		}

		if (m_pBridgePartyIn)
			cstr << "m_pBridgePartyIn->GetState is: " << m_pBridgePartyIn->GetState() << "\n";

		// If we are in disconnecting state we need to save the In Init params
		if (m_state == DISCONNECTING || (m_state == CONNECTED && m_pBridgePartyIn && m_pBridgePartyIn->GetState() == CBridgePartyVideoIn::DISCONNECTING))
		{
			cstr << "m_state == DISCONNECTING ||(m_state == CONNECTED && m_pBridgePartyIn && m_pBridgePartyIn->GetState() == DISCONNECTING) \n";

			if (m_pUpdatePartyInitParams != NULL)
			{
				cstr << "m_pUpdatePartyInitParams != NULL \n";
				if (!m_pUpdatePartyInitParams->GetMediaInParams())
				{
					cstr << "m_pUpdatePartyInitParams->InitiateMediaInParams \n";
					m_pUpdatePartyInitParams->InitiateMediaInParams((CBridgePartyVideoInParams*)pBridgePartyInitParams->GetMediaInParams());
				}
			}
			else
			{
				cstr << "m_pUpdatePartyInitParams == NULL , InitiateUpdatePartyParams \n";
				InitiateUpdatePartyParams(pBridgePartyInitParams);
			}
		}
	}

	CBridgePartyVideoOutParams* videoOutParams = (CBridgePartyVideoOutParams*)pBridgePartyInitParams->GetMediaOutParams();

	if (videoOutParams)
	{
		cstr << "videoOutParams is Valid \n";
		// If audio out wasn't created already...
		if (!m_pBridgePartyOut)
		{
			cstr << "m_pBridgePartyOut is NULL , create new CBridgePartyVideoOut \n";

			NewPartyOut();

			CTaskApi* pTaskApiVideoOut = new CTaskApi(*m_pConfApi);
			pTaskApiVideoOut->CreateOnlyApi(m_pConfApi->GetRcvMbx(), m_pBridgePartyOut);
			pRsrcDesc = pRoutingTable->AddStateMachinePointerToRoutingTbl(m_partyRsrcID, eLogical_COP_dummy_encoder, pTaskApiVideoOut);
			if (!pRsrcDesc)   // Entry not found in Routing Table
			{
				bVideoOutFailure = TRUE;
				POBJDELETE(m_pBridgePartyOut);
				PASSERT(105);
			}
			else
			{
				pRsrcParams->SetRsrcDesc(*pRsrcDesc);
				((CBridgePartyVideoOut*)(m_pBridgePartyOut))->Create(this, pRsrcParams, pBridgePartyInitParams->GetMediaOutParams());
			}

			POBJDELETE(pTaskApiVideoOut);
		}

		if (m_pBridgePartyOut)
		{
			cstr << "m_pBridgePartyOut->GetState is: " << m_pBridgePartyOut->GetState() << "\n";
		}
		else
		{
			cstr << "m_pBridgePartyOut is null\n";
		}

		// If we are in disconnecting state we need to save the Out Init params
		if (m_state == DISCONNECTING || (m_state == CONNECTED && m_pBridgePartyOut && m_pBridgePartyOut->GetState() == CBridgePartyVideoOut::DISCONNECTING))
		{
			cstr << "m_state == DISCONNECTING ||(m_state == CONNECTED && m_pBridgePartyOut && m_pBridgePartyOut->GetState() == DISCONNECTING) \n";

			if (m_pUpdatePartyInitParams != NULL)
			{
				cstr << "m_pUpdatePartyInitParams != NULL \n";
				if (!m_pUpdatePartyInitParams->GetMediaOutParams())
				{
					cstr << "m_pUpdatePartyInitParams->InitiateMediaOutParams \n";
					m_pUpdatePartyInitParams->InitiateMediaOutParams((CBridgePartyVideoOutParams*)pBridgePartyInitParams->GetMediaOutParams());
				}
			}
			else
			{
				cstr << "m_pUpdatePartyInitParams == NULL , InitiateUpdatePartyParams \n";
				InitiateUpdatePartyParams(pBridgePartyInitParams);
			}
		}
	}

	if (TRUE == bVideoInFailure)
		PTRACE2(eLevelError, "CVideoBridgePartyCntlCOP::Update - Video-In creation failure : Name - ", GetFullName());

	if (TRUE == bVideoOutFailure)
		PTRACE2(eLevelError, "CVideoBridgePartyCntlCOP::Update - Video-Out creation failure : Name - ", GetFullName());

	PTRACE(eLevelInfoNormal, cstr.GetString());

	SetSiteName(pBridgePartyInitParams->GetSiteName());

	POBJDELETE(pRsrcParams);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlCOP::OnVideoOutConnectedToPCMSETUP(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlCOP::OnVideoOutConnectedToPCMCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoOutConnectedToPCM(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlCOP::OnVideoOutConnectedToPCMDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlCOP::OnVideoOutConnectedToPCM(CSegment* pParam)
{
	EStat receivedStatus = statOK;
	*pParam >> (BYTE&)receivedStatus;
	m_pConfApi->PartyBridgeResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, PCM_CONNECTED, receivedStatus, FALSE);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlCOP::OnVideoOutDisconnectedFromPCMSETUP(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlCOP::OnVideoOutDisconnectedFromPCMCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnVideoOutDisconnectedFromPCM(pParam);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlCOP::OnVideoOutDisconnectedFromPCMDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlCOP::OnVideoOutDisconnectedFromPCM(CSegment* pParam)
{
	EStat receivedStatus = statOK;
	*pParam >> (BYTE&)receivedStatus;
	m_pConfApi->PartyBridgeResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, PCM_DISCONNECTED, receivedStatus, FALSE);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlCOP::OnPCMVideoOutUpdated(CSegment* pParam)
{
	EStat receivedStatus = statOK;
	*pParam >> (BYTE&)receivedStatus;
	m_pConfApi->PartyBridgeResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, PCM_UPDATED, receivedStatus, FALSE);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlCOP::ConnectToPCMEncoder(DWORD pcmEncoderConnectionId, DWORD pcmEncoderPartyId)
{
	PASSERT_AND_RETURN(!m_pBridgePartyOut);
	((CBridgePartyVideoOutCOP*)m_pBridgePartyOut)->ConnectToPCMEncoder(pcmEncoderConnectionId, pcmEncoderPartyId);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlCOP::DisconnectFromPCMEncoder()
{
	PASSERT_AND_RETURN(!m_pBridgePartyOut);
	((CBridgePartyVideoOutCOP*)m_pBridgePartyOut)->DisconnectFromPCMEncoder();
}

//--------------------------------------------------------------------------
BYTE CVideoBridgePartyCntl::IsConnectedState() const
{
	return (m_state == CONNECTED) ? YES : NO;
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlCOP::OnVideoBridgeFastUpdate(CSegment* pParam)
{
	if (eLECTURER == GetPartyLectureModeRole())
	{
		TRACEINTO << m_partyConfName << " - Ask refresh from Lecture";
		m_pConfApi->VideoRefresh(GetPartyRsrcID()/*m_pParty->GetPartyId()*/);
	}
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlCOP::OnVideoBridgeFastUpdateCONNECTED_STANDALONE(CSegment* pParam)
{
	if (m_pBridgePartyOut)
		((CBridgePartyVideoOutCOP*)m_pBridgePartyOut)->VideoRefresh();
	else
		TRACEINTO << m_partyConfName << " - Failed 'm_pBridgePartyOut' is invalid";
}

//--------------------------------------------------------------------------
BYTE CVideoBridgePartyCntlCOP::IsUpdateEncoderActive()
{
	if (m_pBridgePartyOut)
		return ((CBridgePartyVideoOutCOP*)m_pBridgePartyOut)->IsInUpdateEncoder();

	return NO;
}

//--------------------------------------------------------------------------
BOOL CVideoBridgePartyCntlCOP::IsVideoInDisconnected()
{
	if (!CPObject::IsValidPObjectPtr(m_pBridgePartyIn))
		return TRUE;

	if (m_pBridgePartyIn->IsDisConnected())
		return TRUE;

	return FALSE;
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlCOP::CopPartyConfVideoRefresh()
{
	TRACEINTO << m_partyConfName << ", IsPartyIntraSuppressed:" << (int)IsPartyIntraSuppressed();
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlCOP::UpdateLevelEncoderInDB(WORD levelEncoder)
{
	TRACEINTO << m_partyConfName << ", levelEncoder:" << levelEncoder;
	m_pConfApi->UpdateDB(m_pParty, EVENT_MODE_LEVEL, levelEncoder);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlCOP::ForwardFlowControlCommand(DWORD newRate, CLPRParams* LprParams, BYTE forOutChannel)
{
	newRate = newRate/100;
	TRACEINTO << m_partyConfName << ", LprParams:" << ((LprParams) ? "YES" : "NO") << ", newRate:" << newRate;
	m_pPartyApi->SendFlowControlToCs(newRate, forOutChannel, LprParams);
}

//--------------------------------------------------------------------------
BOOL CVideoBridgePartyCntlCOP::IsConnectedOrConnectingPCM()
{
	if (m_pBridgePartyOut)
		return ((CBridgePartyVideoOutCOP*)m_pBridgePartyOut)->IsConnectedOrConnectingPCM();

	return FALSE;
}

//--------------------------------------------------------------------------
BYTE CVideoBridgePartyCntlCOP::IsReadyToStartLecturer() const
{
	if (m_state == CONNECTED_STANDALONE)
	{
		TRACEINTO << m_partyConfName << " - NO, party is in IVR";
		return NO;
	}

	if (!CPObject::IsValidPObjectPtr(m_pBridgePartyIn))
	{
		TRACEINTO << m_partyConfName << " - NO, 'm_pBridgePartyIn' is not valid";
		return NO;
	}

	if (!m_pBridgePartyIn->IsConnected())
	{
		TRACEINTO << m_partyConfName << " - NO, 'm_pBridgePartyIn' is not connected";
		return NO;
	}

	if (!CPObject::IsValidPObjectPtr(m_pBridgePartyOut))
	{
		TRACEINTO << m_partyConfName << " - NO, 'm_pBridgePartyOut' is not valid";
		return NO;
	}

	if (!m_pBridgePartyOut->IsConnected())
	{
		TRACEINTO << m_partyConfName << " - NO, 'm_pBridgePartyOut' is not connected";
		return NO;
	}

	if (m_state != CONNECTED)
	{
		TRACEINTO << m_partyConfName << " - NO, party does not connected";
		return NO;
	}

	const CImage* pImage = ((CBridgePartyVideoIn*)m_pBridgePartyIn)->GetPartyImage();
	PASSERT_AND_RETURN_VALUE(!pImage, NO);
	if (pImage->isMuted())
	{
		TRACEINTO << m_partyConfName << " - NO, 'm_pBridgePartyIn' party image is muted";
		return NO;
	}

	return YES;
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlCOP::UpdatePartyOnStopFlowControlConstraint()
{
	TRACEINTO << m_partyConfName;
	m_pPartyApi->RemoveSelfFlowControlConstraint();
}

//--------------------------------------------------------------------------
DWORD CVideoBridgePartyCntlCOP::GetDecoderConnectionIdInImage()
{
	if (m_pBridgePartyIn)
		return ((CBridgePartyVideoInCOP*)m_pBridgePartyIn)->GetDecoderConnectionIdInImage();

	return INVALID;
}

//--------------------------------------------------------------------------
DWORD CVideoBridgePartyCntlCOP::GetDecoderPartyIdInImage()
{
	if (m_pBridgePartyIn)
		return ((CBridgePartyVideoInCOP*)m_pBridgePartyIn)->GetDecoderPartyIdInImage();

	return INVALID;
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlCOP::SetDspSmartSwitchConnectionId(DWORD decoderConnectionId)
{
	if (m_pBridgePartyIn)
		((CBridgePartyVideoInCOP*)m_pBridgePartyIn)->SetDspSmartSwitchConnectionId(decoderConnectionId);
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlCOP::SetDspSmartSwitchEntityId(DWORD decoderPartyId)
{
	if (m_pBridgePartyIn)
		((CBridgePartyVideoInCOP*)m_pBridgePartyIn)->SetDspSmartSwitchEntityId(decoderPartyId);
}

//--------------------------------------------------------------------------
BYTE CVideoBridgePartyCntlCOP::IsRemoteNeedSmartSwitchAccordingToVendor()
{
	if (m_pBridgePartyIn)
		return ((CBridgePartyVideoInCOP*)m_pBridgePartyIn)->IsRemoteNeedSmartSwitchAccordingToVendor();

	return NO;
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntlCOP::SetIsRemoteNeedSmartSwitchAccordingToVendor(BYTE isRemoteNeedSmartSwitchAccordingToVendor)
{
	if (m_pBridgePartyIn)
		((CBridgePartyVideoInCOP*)m_pBridgePartyIn)->SetIsRemoteNeedSmartSwitchAccordingToVendor(isRemoteNeedSmartSwitchAccordingToVendor);
}

//--------------------------------------------------------------------------
DWORD CVideoBridgePartyCntlCOP::GetMaxIntraRequestsPerInterval()
{
	DWORD dwMaxIntraRequestsPerInterval = 3;
	CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey("EVENT_MODE_MAX_INTRA_REQUESTS_PER_INTERVAL", dwMaxIntraRequestsPerInterval);
	return dwMaxIntraRequestsPerInterval;
}

//--------------------------------------------------------------------------
DWORD CVideoBridgePartyCntlCOP::GetIntraSuppressionDurationInSeconds()
{
	DWORD dwIntraRequestsSuppressDuration = 10;
	CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey("EVENT_MODE_INTRA_SUPPRESSION_DURATION_IN_SECONDS", dwIntraRequestsSuppressDuration);
	return dwIntraRequestsSuppressDuration;
}

//--------------------------------------------------------------------------
bool CVideoBridgePartyCntl::IsIntraSuppressEnabled(WORD intra_suppression_type) const
{
	return m_isIntraSupressionEnabled;
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::EnableIntraSuppress(WORD intra_suppression_type)
{
	TRACEINTO << m_partyConfName;
	m_isIntraSupressionEnabled = true;
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::DisableIntraSuppress(WORD intra_suppression_type)
{
	TRACEINTO << m_partyConfName;
	m_isIntraSupressionEnabled = false;
}

//--------------------------------------------------------------------------
void CVideoBridgePartyCntl::GetPortsOpened(std::map<eLogicalResourceTypes,bool>& isOpenedRsrcMap)const
{
	if (m_pBridgePartyIn)
		m_pBridgePartyIn->GetPortsOpened(isOpenedRsrcMap);

	if (m_pBridgePartyOut)
		m_pBridgePartyOut->GetPortsOpened(isOpenedRsrcMap);
}

//--------------------------------------------------------------------------
BYTE CVideoBridgePartyCntlCOP::IsPartyCascadeLinkSupportAsymmetricEMCascade()
{
	BYTE isPartyCascadeLinkSupportAsymmetricEMCascade = NO;
	if (m_pBridgePartyIn)
	{
		ECascadePartyType cascadePartyType = ((CBridgePartyVideoUniDirection*)m_pBridgePartyIn)->GetCascadeMode();
		TRACEINTO << GetFullName() << ", cascadeType:" << cascadePartyType;
		if ((cascadePartyType == eCascadeSlaveToRmxSupportSmartCascade) || (cascadePartyType == eCascadeSlaveToRmx1000SupportSmartCascade))
			isPartyCascadeLinkSupportAsymmetricEMCascade = YES;
	}

	return isPartyCascadeLinkSupportAsymmetricEMCascade;
}

//--------------------------------------------------------------------------
WORD CVideoBridgePartyCntlCOP::GetCopEncoderIndexOfCascadeLinkLecturer()
{
	WORD copEncoderIndexOfCascadeLinkLecturer = (0xFFFF);
	if (IsPartyCascadeLinkSupportAsymmetricEMCascade())
	{
		if (m_pBridgePartyOut)
			copEncoderIndexOfCascadeLinkLecturer = ((CBridgePartyVideoOutCOP*)m_pBridgePartyOut)->GetCopEncoderIndexOfCascadeLinkLecturer();
	}

	return copEncoderIndexOfCascadeLinkLecturer;
}
////////////////////////////////////////////////////////////////////////////
//                        VideoBridgePartyCntlXCode
////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CVideoBridgePartyCntlXCode)
  ONEVENT(VIDREFRESH,                         ANYCASE,        CVideoBridgePartyCntlXCode::NullActionFunction)

PEND_MESSAGE_MAP(CVideoBridgePartyCntlXCode,CVideoBridgePartyCntlCOP);

CVideoBridgePartyCntlXCode::CVideoBridgePartyCntlXCode()
{
	m_partyFlowControlRate = 0; // 0 means no flow control was received for this party
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CVideoBridgePartyCntlXCode::CVideoBridgePartyCntlXCode(const CVideoBridgePartyCntlXCode& rOtherBridgePartyCntl)
                         :CVideoBridgePartyCntlCOP(rOtherBridgePartyCntl)
{
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CVideoBridgePartyCntlXCode& CVideoBridgePartyCntlXCode::operator=(const CVideoBridgePartyCntlXCode& rVideoBridgePartyCntl)
{
  if (&rVideoBridgePartyCntl == (CVideoBridgePartyCntl*)this)
    return *this;

  (CVideoBridgePartyCntl&)(*this) = (CVideoBridgePartyCntl&)rVideoBridgePartyCntl;
  return *this;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CVideoBridgePartyCntlXCode::~CVideoBridgePartyCntlXCode()
{
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CVideoBridgePartyCntlXCode::NewPartyOut()
{
  m_pBridgePartyOut = new CBridgePartyVideoOutXcode();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CVideoBridgePartyCntlXCode::NewPartyIn()
{
  m_pBridgePartyIn = NULL;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CVideoBridgePartyCntlXCode::Create(const CVideoBridgePartyInitParams* pVideoBridgePartyInitParams)
{
  PASSERT_AND_RETURN(!pVideoBridgePartyInitParams);
  PASSERT_AND_RETURN(!pVideoBridgePartyInitParams->GetMediaOutParams());

  CConfPartyRoutingTable* pRoutingTable = ::GetpConfPartyRoutingTable();  // The routing table is relevant only for the out
  PASSERT_AND_RETURN(!pRoutingTable);

  // Create base params
  CBridgePartyCntl::Create(pVideoBridgePartyInitParams);

  m_pUpdatePartyInitParams = NULL;

  CRsrcParams rsrcParams(DUMMY_CONNECTION_ID, m_partyRsrcID, m_confRsrcID);

  // Do not  Create BridgePartyVideoIn

  // Create BridgePartyVideoOut
  CBridgePartyVideoOutParams* videoOutParams = (CBridgePartyVideoOutParams*)pVideoBridgePartyInitParams->GetMediaOutParams();
  if (videoOutParams)
  {
    NewPartyOut();
    std::auto_ptr<CTaskApi> pTaskApiVideoOut(new CTaskApi(*m_pConfApi));
    pTaskApiVideoOut->CreateOnlyApi(m_pConfApi->GetRcvMbx(), m_pBridgePartyOut);

    CRsrcDesc* pRsrcDesc = pRoutingTable->AddStateMachinePointerToRoutingTbl(m_partyRsrcID, eLogical_video_encoder_content, pTaskApiVideoOut.get());
    if (!pRsrcDesc)   // Entry not found in Routing Table
    {
      POBJDELETE(m_pBridgePartyOut);
      PASSERT(1);
    }
    else
    {
      rsrcParams.SetRsrcDesc(*pRsrcDesc);
      ((CBridgePartyVideoOut*)(m_pBridgePartyOut))->Create(this, &rsrcParams, videoOutParams);
    }
  }
}// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CVideoBridgePartyCntlXCode::Setup()
{
  // In case we are in CONNECTED state and the second video direction start to connect -
  // will remain in CONNECTED state.
  if (m_state != CONNECTED)
    m_state = SETUP;

  // If video OUT is not connected or in Setup state...
  bool isConnectBridgePartyOut = (m_pBridgePartyOut && !m_pBridgePartyOut->IsConnected() && !m_pBridgePartyOut->IsConnecting());

  TRACEINTO << "CVideoBridgePartyCntlXCode::Setup - " << m_partyConfName << ", State:" << StateToString() <<  ", isConnectBridgePartyOut:" << isConnectBridgePartyOut;

  StartTimer(VIDEO_BRDG_PARTY_SETUP_TOUT, VIDEO_BRDG_PARTY_SETUP_TOUT_VALUE);


  if (isConnectBridgePartyOut)
    m_pBridgePartyOut->Connect();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CVideoBridgePartyCntlXCode::Update(const CVideoBridgePartyInitParams* pBridgePartyInitParams)
{

}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CVideoBridgePartyCntlXCode::VideoConnectionCompletion(CSegment* pParams, EMediaDirection eConnectedMediaDirection)
{
	PASSERT_AND_RETURN(!m_pBridgePartyOut);

	EStat           receivedStatus             = statOK;
	EMediaDirection ConnectedDirection         = eNoDirection;
	BOOL            isVideoConnectionCompleted = FALSE;

	*pParams >> (BYTE&)receivedStatus;

	if (statOK != receivedStatus)
	{
		DeleteTimer(VIDEO_BRDG_PARTY_SETUP_TOUT);

		DumpMcuInternalProblemDetailed((BYTE)eConnectedMediaDirection, eMipStatusFail, eMipVideo);

		// Inform Video Bridge about connection failure
		m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, XCODE_BRDG_MSG , ENDCONNECTPARTY, receivedStatus, FALSE, eNoDirection, pParams);
		TRACEINTO << "CVideoBridgePartyCntlXCode::VideoConnectionCompletion - Failed, because recieved invalid status, status:" << receivedStatus << ", " << m_partyConfName;
		return;
	}
	// Video-out is connected
	if (eMediaOut == eConnectedMediaDirection)
	{
		if (IsUniDirectionConnection(eMediaOut))
			isVideoConnectionCompleted = TRUE, ConnectedDirection = eMediaOut;
	}

	TRACEINTO << "CVideoBridgePartyCntlXCode::VideoConnectionCompletion - " << m_partyConfName << ", isVideoConnectionCompleted:" << (int)isVideoConnectionCompleted << ", ConnectedDirection:" << ConnectedDirection;

	if (TRUE == isVideoConnectionCompleted)
	{
		DeleteTimer(VIDEO_BRDG_PARTY_SETUP_TOUT);

		m_state = CONNECTED;

		// Inform Video Bridge
		m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, XCODE_BRDG_MSG , ENDCONNECTPARTY, statOK, FALSE, ConnectedDirection);
	}
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CVideoBridgePartyCntlXCode::VideoDisConnectionCompletion(CSegment* pParams, EMediaDirection eDisConnectedMediaDirection)
{
	PASSERT_AND_RETURN(!m_pBridgePartyIn && !m_pBridgePartyOut);

	BOOL            isVideoDisConnectionCompleted = FALSE;
	BOOL            isHalfDisconnection           = FALSE;
	EStat           receivedStatus                = statOK;
	EMediaDirection ConnectedDirection            = eNoDirection;

	BYTE videoOutClosePortStatus                  = statOK;
	BYTE videoInClosePortStatus                   = statOK;


	*pParams >> (BYTE&)receivedStatus;


	OPCODE brdgOpcode = XCODE_BRDG_MSG;

	// *** 1. Check if this is FULL disconnection or not - Full disconnection is when all connected directions disconnects!!! ****

	// Video-in is disconnected
	PASSERT_AND_RETURN(eMediaIn == eDisConnectedMediaDirection);

	// Video-out is disconnected
	if (eMediaOut == eDisConnectedMediaDirection)
	{
		if (IsUniDirectionConnection(eMediaOut))
			isVideoDisConnectionCompleted = TRUE;
	}

	// *** 2. If this is full disconnection  ***

	if (TRUE == isVideoDisConnectionCompleted)
	{
		DeleteTimer(VIDEO_BRDG_PARTY_DISCONNECT_TOUT);

		ConnectedDirection = eNoDirection;
		m_state            = IDLE;

		SetDisConnectingDirectionsReq(eNoDirection);

		if (m_pBridgePartyOut)
			videoOutClosePortStatus = m_pBridgePartyOut->GetClosePortAckStatus();

		// for debug info in case of the "MCU internal problem"
		BYTE failureCauseDirection = eMipNoneDirction;

		if ( videoOutClosePortStatus != STATUS_OK)
			receivedStatus = (EStat)videoOutClosePortStatus, failureCauseDirection = eMipOut;

		// Inform Video Bridge In case of problem
		if (statVideoInOutResourceProblem == receivedStatus)
		{
			CSegment* pSeg = new CSegment;
			*pSeg << (BYTE)receivedStatus << (BYTE)failureCauseDirection << (BYTE)eMipStatusFail << (BYTE)eMipClose;

			if (GetConnectionFailureCause() == statOK)
				DumpMcuInternalProblemDetailed((BYTE)failureCauseDirection, eMipStatusFail, eMipVideo);

			m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, brdgOpcode, ENDDISCONNECTPARTY, receivedStatus, FALSE, ConnectedDirection, pSeg);
			POBJDELETE(pSeg);
		}
		else  // In case we received connect req while disconnecting
		{
			if (m_pUpdatePartyInitParams)
			{
				PTRACE2(eLevelInfoNormal, "CVideoBridgePartyCntlXCode::VideoDisConnectionCompletion - Both direction were disconnected, now start connection process, ", m_partyConfName);
				StartConnectProcess();
			}
			else
			{
				PTRACE2(eLevelInfoNormal, "CVideoBridgePartyCntlXCode::VideoDisConnectionCompletion - Both direction were disconnected, now state is IDLE, ", m_partyConfName);
				DestroyPartyInOut(ConnectedDirection);

				// Inform Video Bridge - Add the direction connected state
				m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, brdgOpcode, ENDDISCONNECTPARTY, receivedStatus, FALSE, ConnectedDirection);
			}
		}
	}
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
DWORD CVideoBridgePartyCntlXCode::GetXCodeEncoderEntityId()
{
	 DWORD encoderEntity = DUMMY_PARTY_ID;
	  if(m_pBridgePartyOut)
	    return ((CBridgePartyVideoOutXcode*)m_pBridgePartyOut)->GetXCodeEncoderId();

	  PTRACE2(eLevelInfoNormal, "CCVideoBridgePartyCntlXCode::GetXCodeEncoderEntityId - Party does not connected to video-out, ", m_partyConfName);
	  return DUMMY_PARTY_ID;
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
WORD CVideoBridgePartyCntlXCode::GetXCodeEncoderIndex()
{
	eXcodeRsrcType eXcodeEncoderIndex = MAX_CONTENT_XCODE_RSRCS;
	if (m_pBridgePartyOut)
	{
		eXcodeEncoderIndex =((CBridgePartyVideoOutXcode*)m_pBridgePartyOut)->GetXCodeEncoderIndex();
		return eXcodeEncoderIndex;
	}
	PTRACE2(eLevelInfoNormal, "CVideoBridgePartyCntlXCode::GetXCodeEncoderIndex - Party does not connected to video-out, ", m_partyConfName);

	return (WORD)eXcodeEncoderIndex;
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
DWORD CVideoBridgePartyCntlXCode::GetXCodeEncoderConnectionID()
{
	DWORD xCodeEncoderConnectionID = DUMMY_CONNECTION_ID;
	if (m_pBridgePartyOut)
	{
		xCodeEncoderConnectionID =((CBridgePartyVideoOutXcode*)m_pBridgePartyOut)->GetXCodeEncoderConnectionId();
	}
	return xCodeEncoderConnectionID;
}
 // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//    Action Functions

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CVideoBridgePartyCntlXCode::OnVideoBridgeConnectIDLE(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CVideoBridgePartyCntlXCode::OnVideoBridgeConnectIDLE, Party Name: ",GetFullName());
	OnVideoBridgeConnect(pParam);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Incase one of the video channels (In/Out) is in setup state and now connects the second direction.
void CVideoBridgePartyCntlXCode::OnVideoBridgeConnectSETUP(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgePartyCntlXCode::OnVideoBridgeConnectSETUP - Party is already in connecting process to XCode bridge,Do Nothing!!!!!, Party Name:  ", m_partyConfName);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CVideoBridgePartyCntlXCode::OnVideoBridgeConnect(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CVideoBridgePartyCntlXCode::OnVideoBridgeConnect, Party Name: ",GetFullName());
	if (!m_pBridgePartyOut)
	{
		m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, XCODE_BRDG_MSG, ENDCONNECTPARTY, statInconsistent, FALSE, eNoDirection);
		return;
	}
	Setup();
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CVideoBridgePartyCntlXCode::OnVideoBridgeConnectCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CVideoBridgePartyCntlXCode::OnVideoBridgeConnectCONNECTED - Party is already in connected to XCode bridge,Do Nothing!!!!!, Party Name:  ", m_partyConfName);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CVideoBridgePartyCntlXCode::OnTimerPartyDisconnect(CSegment* pParams)
{
  TRACEINTO << "CVideoBridgePartyCntlXCode::OnTimerPartyDisconnect - " << m_partyConfName << ", State:" << StateToString();

  EMediaDirection eConnectedDirection   = eNoDirection;
  BYTE            failureCauseDirection = eMipOut;

  RemoveResourcesFromRoutingTable();

  CSegment* pSeg = new CSegment;
  *pSeg << (BYTE)statVideoInOutResourceProblem << (BYTE)failureCauseDirection << (BYTE)eMipTimer << (BYTE)eMipClose;

  if (GetConnectionFailureCause() == statOK)
    DumpMcuInternalProblemDetailed(failureCauseDirection, eMipTimer, eMipVideo);

  // We need to delete partyIn/partyOut incase of disconnect timeout failure
  DestroyPartyInOut(eConnectedDirection);

  // Inform Video Bridge of statVideoInOutResourceProblem -> Inorder to initiate Kill port on AudioEncoder+Decoder
  m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, XCODE_BRDG_MSG, ENDDISCONNECTPARTY, statVideoInOutResourceProblem, FALSE, eConnectedDirection, pSeg);
  POBJDELETE(pSeg);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CVideoBridgePartyCntlXCode::OnTimerPartyDisconnectDISCONNECTING(CSegment* pParams)
{
  TRACEINTO << "CVideoBridgePartyCntl::OnTimerPartyDisconnectDISCONNECTING - " << m_partyConfName << ", State:" << StateToString();

  m_state = IDLE;

  BYTE failureCauseDirection = eMipOut;

  PASSERT(!CPObject::IsValidPObjectPtr(m_pBridgePartyOut));

  RemoveResourcesFromRoutingTable();

  CSegment* pSeg = new CSegment;
  *pSeg << (BYTE)statVideoInOutResourceProblem << (BYTE)failureCauseDirection << (BYTE)eMipTimer << (BYTE)eMipClose;

  if (GetConnectionFailureCause() == statOK)
    DumpMcuInternalProblemDetailed(failureCauseDirection, eMipTimer, eMipVideo);

  // Inform Video Bridge of statVideoInOutResourceProblem -> Inorder to initiate Kill port on VideoEncoder+Decoder
  m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, XCODE_BRDG_MSG, ENDDISCONNECTPARTY, statVideoInOutResourceProblem, FALSE, eNoDirection, pSeg);
  POBJDELETE(pSeg);
}
//--------------------------------------------------------------------------
void CVideoBridgePartyCntlXCode::OnTimerPartySetup(CSegment* pParams)
{
	CMedString encoderString, decoderString;
	CMedString logStr;
	logStr << m_partyConfName << ", State:" << StateToString();

	if (CPObject::IsValidPObjectPtr(m_pBridgePartyOut))
	{
		logStr << "\n  CBridgePartyVideoOut State: ";
		((CBridgePartyVideoOut*)m_pBridgePartyOut)->DumpAllInfoOnConnectionState(&logStr);
		((CBridgePartyVideoOut*)m_pBridgePartyOut)->DumpAllInfoOnConnectionState(&encoderString, true);
	}


	TRACESTRFUNC(eLevelError) << logStr.GetString();

	// for debug info in case of the "MCU internal problem"
	BYTE failureCauseDirection = eMipNoneDirction;
	BYTE failureCauseAction    = eMipNoAction;
	GetRsrcProbAdditionalInfoOnVideoTimerSetup(failureCauseDirection, failureCauseAction);

	CSegment* pSeg = new CSegment;
	*pSeg << (BYTE)statVideoInOutResourceProblem << (BYTE)failureCauseDirection << (BYTE)eMipTimer << (BYTE)failureCauseAction;
	DumpMcuInternalProblemDetailed(failureCauseDirection, eMipTimer, eMipVideo);

	// Inform Video Bridge of statVideoInOutResourceProblem -> Inorder to initiate Kill port on VideoEncoder+Decoder
	m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, XCODE_BRDG_MSG, ENDCONNECTPARTY, statVideoInOutResourceProblem, FALSE, eNoDirection, pSeg);
	POBJDELETE(pSeg);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CVideoBridgePartyCntlXCode::OnVideoBridgeDisconnect(CSegment* pParams)
{
  BOOL IsVideoFullyDisconnect = FALSE;
  PTRACE2(eLevelInfoNormal, "CVideoBridgePartyCntlXCode::OnVideoBridgeDisconnect, Party Name: - ", m_partyConfName);

  StartTimer(VIDEO_BRDG_PARTY_DISCONNECT_TOUT, VIDEO_BRDG_PARTY_DISCONNECT_TOUT_VALUE);

  switch (GetDisconnectingDirectionsReq())
  {
    case eMediaIn:
    {
     PASSERT(1);
      break;
    }

    case eMediaOut:
    {                         // Disconnect Video Out
      if (m_pBridgePartyOut)  // If party In in state of CONNECTED or SETUP , this is not full disconnection of party cntl
      {
    	  IsVideoFullyDisconnect = TRUE;

    	  // Incase we are in setup state and IN direction is already connected
    	  // but the OUT direction is still in setup--> we will change the state to CONNECTED,
    	  // stop the setup timer and won't send ack on connect to partyCntl only ack on disconnect
    	  if (m_pBridgePartyOut->IsConnected() && m_state == SETUP)
    	  {
    		  PTRACE2(eLevelInfoNormal, "CVideoBridgePartyCntlXCode::OnVideoBridgeDisconnect, Party-Out is in SETUP - Change state to CONNECT - ", m_partyConfName);
    		  m_state = CONNECTED;
    		  DeleteTimer(VIDEO_BRDG_PARTY_SETUP_TOUT);
    	  }
    	  m_pBridgePartyOut->DisConnect();
      }
      break;
    }

    case eMediaInAndOut:
    {
      if (m_pBridgePartyOut)
        m_pBridgePartyOut->DisConnect();

      IsVideoFullyDisconnect = TRUE;
      break;
    }

    default:
    {
      DBGPASSERT(1);
    }
  } // switch

  if (IsVideoFullyDisconnect)
  {
    if (m_state == SETUP)
      DeleteTimer(VIDEO_BRDG_PARTY_SETUP_TOUT);

    m_state = DISCONNECTING;
  }
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CVideoBridgePartyCntlXCode::OnVideoBridgeDisconnectIDLE(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgePartyCntl::OnVideoBridgeDisconnectIDLEXCode - ", m_partyConfName);

  if (CPObject::IsValidPObjectPtr(m_pConfApi) && CPObject::IsValidPObjectPtr(m_pParty))
    m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, XCODE_BRDG_MSG, ENDDISCONNECTPARTY, statOK, FALSE, eNoDirection);
}
//=====================================================================================================================//
// TELEPRESENCE_LAYOUTS
void CVideoBridgePartyCntl::UpdateTelepresenceLayoutMode(ETelePresenceLayoutMode newLayoutMode)
{
	CSegment seg;
	seg << (DWORD)newLayoutMode;
	DispatchEvent(SET_TELEPRESENCE_LAYOUT_MODE, &seg);
}
//=====================================================================================================================//
void  CVideoBridgePartyCntl::OnVideoBridgeUpdateTelepresenceLayoutModeIDLE(CSegment* pParam)
{
	if(NULL == pParam){
		DBGPASSERT(1);
		return;
	}
	DWORD tmpLayoutMode = 0;
	*pParam >> tmpLayoutMode;
	ETelePresenceLayoutMode newLayoutMode = (ETelePresenceLayoutMode)(tmpLayoutMode);

	TRACEINTO << " TELEPRESENCE_LAYOUTS_DEBUG newLayoutMode = " << TelePresenceLayoutModeToString(newLayoutMode);

	ChangeTelepresenceLayoutMode(newLayoutMode,false);

}
//=====================================================================================================================//
void  CVideoBridgePartyCntl::OnVideoBridgeUpdateTelepresenceLayoutModeSETUP(CSegment* pParam)
{
	if(NULL == pParam){
		DBGPASSERT(1);
		return;
	}
	DWORD tmpLayoutMode = 0;
	*pParam >> tmpLayoutMode;
	ETelePresenceLayoutMode newLayoutMode = (ETelePresenceLayoutMode)(tmpLayoutMode);

	TRACEINTO << " TELEPRESENCE_LAYOUTS_DEBUG newLayoutMode = " << TelePresenceLayoutModeToString(newLayoutMode);

	ChangeTelepresenceLayoutMode(newLayoutMode,false);
}
//=====================================================================================================================//
void  CVideoBridgePartyCntl::OnVideoBridgeUpdateTelepresenceLayoutModeCONNECTED(CSegment* pParam)
{
	if(NULL == pParam){
		DBGPASSERT(1);
		return;
	}
	DWORD tmpLayoutMode = 0;
	*pParam >> tmpLayoutMode;
	ETelePresenceLayoutMode newLayoutMode = (ETelePresenceLayoutMode)(tmpLayoutMode);

	TRACEINTO << " TELEPRESENCE_LAYOUTS_DEBUG newLayoutMode = " << TelePresenceLayoutModeToString(newLayoutMode);

	ChangeTelepresenceLayoutMode(newLayoutMode,true);
}
//=====================================================================================================================//
void  CVideoBridgePartyCntl::OnVideoBridgeUpdateTelepresenceLayoutModeCONNECTED_STANDALONE(CSegment* pParam)
{
	if(NULL == pParam){
		DBGPASSERT(1);
		return;
	}
	DWORD tmpLayoutMode = 0;
	*pParam >> tmpLayoutMode;
	ETelePresenceLayoutMode newLayoutMode = (ETelePresenceLayoutMode)(tmpLayoutMode);

	TRACEINTO << " TELEPRESENCE_LAYOUTS_DEBUG newLayoutMode = " << TelePresenceLayoutModeToString(newLayoutMode);

	ChangeTelepresenceLayoutMode(newLayoutMode,false);
}
//=====================================================================================================================//
// TELEPRESENCE_LAYOUTS
void CVideoBridgePartyCntl::ChangeTelepresenceLayoutMode(ETelePresenceLayoutMode newTelepresenceLayoutMode, bool init_change_layout)
{
	if(newTelepresenceLayoutMode == m_telepresenceLayoutMode){
		TRACEINTO << " no change in telepresence layout mode, TelepresenceLayoutMode = " << TelePresenceLayoutModeToString(newTelepresenceLayoutMode);
		return;
	}

	TRACEINTO << " TELEPRESENCE_LAYOUTS_DEBUG newLayoutMode = " << TelePresenceLayoutModeToString(newTelepresenceLayoutMode) << " , init_change_layout = " << (WORD)init_change_layout;

	m_telepresenceLayoutMode = newTelepresenceLayoutMode;

	if (m_pBridgePartyOut){
		((CBridgePartyVideoOut*)(m_pBridgePartyOut))->UpdateLayoutHandlerType();
		if(init_change_layout && m_pBridgePartyOut->IsConnected()){
			BOOL bUseSharedMemForChangeLayoutReq = false;
			BYTE alwaysSendToHardware = false;
			((CBridgePartyVideoOut*)(m_pBridgePartyOut))->BuildLayout(bUseSharedMemForChangeLayoutReq, alwaysSendToHardware);
		}
	}
}
//=====================================================================================================================//
