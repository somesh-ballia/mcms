//+========================================================================+
//               MplMcmsProtocolSizeValidator.cpp                          |
//            Copyright 2006 Polycom Israel Ltd.				           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Ltd. and is protected by law.					   |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       MplMcmsProtocolSizeValidator.cpp                                           |
// PROGRAMMER: Keren                                                       |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |13/06/07    |                                                     |
//+========================================================================+

#include "MplMcmsProtocolSizeValidator.h"

#include "Q931Structs.h"
#include "IpMfaOpcodes.h"
#include "IpRtpInd.h"
#include "IpRtpReq.h"
#include "MrcStructs.h"

const MplMcmsProtocolSizeValidatorStruct
CMplMcmsProtocolSizeValidator::g_MplMcmsProtocolOpcodeContentStuctSize[] =
{
        // opcode                               sizeOfContentStruct
        { TB_MSG_CONNECT_REQ,	       			sizeof(TB_MSG_CONNECT_S)  },
        { TB_MSG_DISCONNECT_REQ,       			sizeof(TB_MSG_CONNECT_S)  },
        { AUDIO_OPEN_ENCODER_REQ,      			sizeof(TAudioOpenEncoderReq) },
        { AUDIO_OPEN_DECODER_REQ,      			sizeof(TAudioOpenDecoderReq) },
        { AUDIO_UPDATE_ALGORITHM_REQ,  			sizeof(TAudioUpdateAlgorithmReq) },
        { AUDIO_UPDATE_DECODER_REQ,             sizeof(TAudioUpdateDecoderReq) },
        { AUDIO_UPDATE_MUTE_REQ,       			sizeof(TAudioUpdateMuteReq) },
        { AUDIO_UPDATE_GAIN_REQ,       			sizeof(TAudioUpdateGainReq) },
        { AUDIO_UPDATE_USE_SPEAKER_SSRC_FOR_TX_REQ,sizeof(TAudioUpdateUseSpeakerSsrcForTxReq) },
        { AUDIO_UPDATE_NOISE_DETECTION_REQ, 	sizeof(TAudioUpdateNoiseDetectionReq) },
        { AUDIO_UPDATE_CONNECTION_STATUS_REQ, 	sizeof(TAudioUpdateConnectionStatusReq) },
        { AUDIO_UPDATE_AGC_REQ,        			sizeof(TAudioUpdateAgcReq) },
        { AUDIO_UPDATE_BITRATE_REQ,				sizeof(TAudioUpdateBitRateReq) },
        { AC_UPDATE_CONF_PARAMS_REQ,  			sizeof(TAcOpenConfReq) },
        { AC_OPEN_CONF_REQ,           			sizeof(TAcOpenConfReq) },
        { AC_ACTIVE_SPEAKER_IND,      			sizeof(TAcActiveSpeakersInd) },
        { AC_AUDIO_SPEAKER_IND,       			sizeof(TAcActiveSpeakersInd) },
        { CONF_MPL_CREATE_PARTY_REQ, 			sizeof(TOpenArtReq) },
        { VIDEO_ENCODER_CHANGE_LAYOUT_REQ, 		(sizeof(MCMS_CM_CHANGE_LAYOUT_S)-sizeof(MCMS_CM_IMAGE_PARAM_S*)) },
        { VIDEO_ENCODER_CHANGE_LAYOUT_ATTRIBUTES_REQ, sizeof(CHANGE_LAYOUT_ATTRIBUTES_S) },
        { VIDEO_ENCODER_UPDATE_PARAM_REQ, 		sizeof(ENCODER_PARAM_S) },
        { VIDEO_DECODER_UPDATE_PARAM_REQ, 		sizeof(DECODER_PARAM_S) },
        { VIDEO_UPDATE_DECODER_RESOLUTION_REQ,	sizeof(UPDATE_DECODER_RESOLUTION_S) },
        { ACK_IND,                     			sizeof(ACK_IND_S) },
        { NET_SETUP_REQ,               			sizeof(NET_SETUP_REQ_S) },
        { NET_SETUP_IND,               			sizeof(NET_SETUP_IND_S) },
        { NET_CONNECT_REQ,             			sizeof(NET_CONNECT_REQ_S) },
        { NET_CONNECT_IND,             			sizeof(NET_CONNECT_IND_S) },
        { NET_CLEAR_REQ,               			sizeof(NET_CLEAR_REQ_S) },
        { NET_CLEAR_IND,               			sizeof(NET_CLEAR_IND_S) },
        { NET_PROGRESS_IND,            			sizeof(NET_PROGRESS_IND_S) },
        { NET_ALERT_REQ,               			sizeof(NET_ALERT_REQ_S) },
        { NET_ALERT_IND,               			sizeof(NET_ALERT_IND_S) },
        { NET_PROCEED_IND,             			sizeof(NET_PROCEED_IND_S) },
        { NET_DISCONNECT_IND,          			sizeof(NET_DISCONNECT_IND_S) },
        { NET_DISCONNECT_ACK_REQ,     			sizeof(NET_DISCONNECT_ACK_REQ_S) },
        { NET_DISCONNECT_ACK_IND,      			sizeof(NET_DISCONNECT_ACK_IND_S) },
        { VIDEO_DECODER_SYNC_IND,      			sizeof(DECODER_SYNC_IND_S) },
        { CONFPARTY_CM_OPEN_UDP_PORT_REQ, 		sizeof(TOpenUdpPortOrUpdateUdpAddrMessageStruct) },
        { CONFPARTY_CM_CLOSE_UDP_PORT_REQ, 		sizeof(TCloseUdpPortMessageStruct) },
        { KILL_UDP_PORT_REQ,           			sizeof(TKillUdpPortMessageStruct) },
        { STARTUP_DEBUG_RECORDING_PARAM_REQ, 	sizeof(TStartupDebugRecordingParamReq) },
        { IVR_START_IVR_REQ,           			sizeof(SIVRStartIVRStruct) },
        { IVR_STOP_IVR_REQ,            			sizeof(SIVRStopIVRStruct) },
        { IVR_PLAY_MESSAGE_REQ,        			(sizeof(SIVRPlayMessageStruct)-sizeof(SIVRMediaFileParamsStruct*)) },
        { IVR_PLAY_MUSIC_REQ,          			sizeof(SIVRPlayMusicStruct) },
        { IVR_RECORD_ROLL_CALL_REQ,    			(sizeof(SIVRPlayMessageStruct)-sizeof(SIVRMediaFileParamsStruct*)) },
        { IVR_STOP_RECORD_ROLL_CALL_REQ,     	sizeof(PHYSICAL_RESOURCE_INFO_S) },
        { IVR_SHOW_SLIDE_REQ,          			(sizeof(SIVRPlayMessageStruct)-sizeof(SIVRMediaFileParamsStruct*)) },
        { IVR_STOP_SHOW_SLIDE_REQ,     			sizeof(PHYSICAL_RESOURCE_INFO_S) },
        { IVR_UPDATE_STANDALONE_REQ,   			sizeof(TAudioUpdateStandaloneReq) },
        { IVR_FAST_UPDATE_REQ,         			sizeof(PHYSICAL_RESOURCE_INFO_S) },
        { MOVE_PARTY_RESOURCE_REQ,     			sizeof(MOVE_RESOURCES_REQ_S) },
        { SM_FATAL_FAILURE_IND,     			sizeof(SWITCH_SM_KEEP_ALIVE_S) },
        { BND_CONNECTION_INIT,     			    sizeof(BND_CONNECTION_INIT_REQUEST_S) },
        { BND_ADD_CHANNEL,     			        sizeof(BND_ADD_CHANNEL_REQUEST_S) },
        { BND_END_NEGOTIATION,                  sizeof(BND_END_NEGOTIATION_INDICATION_S) },
        { BND_REMOTE_LOCAL_ALIGNMENT,           sizeof(BND_REMOTE_LOCAL_ALIGNMENT_INDICATION_S)},
        { H221_INIT_COMM,                       sizeof(H221_INIT_COMM_S) },
        { END_INIT_COMM,                        sizeof(END_INIT_COMM_S) },
        { SET_XMIT_MODE,                        sizeof(SET_XMIT_MODE_S) },
        { REMOTE_XMIT_MODE,                     sizeof(REMOTE_XMIT_MODE_S) },
        { EXCHANGE_CAPS,                        sizeof(EXCHANGE_CAPS_S) },
        { REMOTE_BAS_CAPS,                      sizeof(REMOTE_BAS_CAPS_S) },
        { H320_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ,sizeof(H320_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ) },
        { H320_RTP_UPDATE_CHANNEL_REQ,          sizeof(H320_RTP_UPDATE_CHANNEL_REQ) },
        { H320_RTP_UPDATE_CHANNEL_RATE_REQ,     sizeof(H320_RTP_UPDATE_CHANNEL_RATE_REQ) },
        { REMOTE_CI,							sizeof(REMOTE_CI)},
        { SMART_RECOVERY_UPDATE,				sizeof(SMART_RECOVERY_UPDATE_S)},
        { SLOTS_NUMBERING_CONVERSION_IND,		sizeof(SLOTS_NUMBERING_CONVERSION_TABLE_S)},
        { SYSCFG_PARAMS_REQ,					sizeof(SYSCFG_PARAMS_S) },
        { OLD_SYSCFG_PARAMS_IND,				sizeof(OLD_SYSCFG_PARAMS_IND) },
        { ENC_KEYS_INFO_REQ,					sizeof(ENC_KEYS_INFO_S) },
        { VIDEO_GRAPHICS_START_GATHERING_REQ,	sizeof(TEXT_BOX_LAYOUT_S) },
        { IP_RTP_FECC_TOKEN_IND,				sizeof(TRtpFeccTokenRequestInd) },
        { IP_RTP_FECC_KEY_IND,					sizeof(TRtpFeccTokenRequestInd) },
        { ETHERNET_SETTINGS_IND,				sizeof(ETH_SETTINGS_SPEC_S) },
        { IP_RTP_SET_FECC_PARTY_TYPE,			sizeof(FECC_PARTY_TYPE_S) },
        { UNIT_RECOVERY_IND,					sizeof(UNIT_RECOVERY_S) },
        { UNIT_RECOVERY_END_IND,				sizeof(UNIT_RECOVERY_S) },
        { UNIT_FATAL_IND,						sizeof(UNIT_RECOVERY_S) },
        { UNIT_UNFATAL_IND,						sizeof(UNIT_RECOVERY_S) },
        { CM_HIGH_CPU_USAGE_IND,				sizeof(CM_HIGH_CPU_USAGE_S) },
        { RECOVERY_REPLACEMENT_UNIT_REQ,		sizeof(RECOVERY_REPLACEMENT_UNIT_S) },
        { ALLOC_STATUS_PER_UNIT_REQ,			sizeof(ALLOC_STATUS_PER_UNIT_S)	},
        { CARD_CONFIG_REQ, 					    sizeof(CARDS_CONFIG_PARAMS_S) },
        { SET_LOG_LEVEL_REQ,                    sizeof(LOG_LEVEL_S) },
        { CARD_CONFIG_REQ,                      sizeof(CARDS_CONFIG_PARAMS_S) },
        { VIDEO_ENCODER_DSP_SMART_SWITCH_CHANGE_LAYOUT_REQ, (sizeof(MCMS_CM_CHANGE_LAYOUT_S)-sizeof(MCMS_CM_IMAGE_PARAM_S*)) },
        { VIDEO_ENCODER_ICONS_DISPLAY_REQ,      sizeof(ICONS_DISPLAY_S) },
	    { VIDEO_ENCODER_DSP_SMART_SWITCH_CHANGE_LAYOUT_REQ, sizeof(CHANGE_LAYOUT_S) },
        { CONF_PARTY_MRMP_OPEN_CHANNEL_REQ, 		sizeof(MrmpOpenChannelRequestStruct) },
        { CONF_PARTY_MRMP_UPDATE_CHANNEL_REQ, 		sizeof(MrmpOpenChannelRequestStruct) },
        { CONF_PARTY_MRMP_CLOSE_CHANNEL_REQ, 		sizeof(MrmpCloseChannelRequestStruct) },
        { CONF_PARTY_MRMP_SCP_STREAM_ACK_REQ, 		sizeof(MrmpScpAckStruct) },
        { AUDIO_UPDATE_RELAY_DEC_PARAMS_REQ,  			sizeof(TRtpUpdateRelayReq) },
        { AUDIO_UPDATE_RELAY_ENC_PARAMS_REQ,  			sizeof(TRtpUpdateRelayReq) },
        { CONF_PARTY_MRMP_SCP_IVR_STATE_NOTIFICATION_REQ, sizeof(MrmpScpIvrStateNotificationStruct) },
        {IP_CM_DTLS_START_REQ, 					sizeof(TDtlsStartStruct)},
//        {IP_CM_DTLS_END_IND, 					sizeof(mcIndCmDtlsEnd)},
        {IP_CM_DTLS_CLOSE_REQ,					sizeof(TDtlsCloseStruct)},
        { BAD_SPONTANEOUS_IND,  			sizeof(BAD_SPONTANEOUS_IND_S) },
        { CG_PLAY_AUDIO_REQ,					sizeof(TCGPlayAudioReqStruct) },
        {AUDIO_ENCODER_UPDATE_SEEN_IMAGE_SSRC,     sizeof(TAudioEncoderUpdateSeenImageSsrcReq)},
        { CONF_MPL_DELETE_PARTY_REQ, 			sizeof(TCloseArtReq) }
};

// Virtual
const char* CMplMcmsProtocolSizeValidator::NameOf(void) const
{
    return GetCompileType();
}


DWORD CMplMcmsProtocolSizeValidator::GetContentSizeByOpcode(OPCODE opcode, BYTE logicRsrcType) const
{
	if (opcode==TB_MSG_OPEN_PORT_REQ)
	{
		switch (logicRsrcType)
		{
			case eLogical_video_encoder:
			case eLogical_video_encoder_content:
		    case eLogical_COP_CIF_encoder:
		    case eLogical_COP_4CIF_encoder:
		    case eLogical_COP_VSW_encoder:
		    case eLogical_COP_PCM_encoder:
		    case eLogical_COP_HD720_encoder:
		    case eLogical_COP_HD1080_encoder:
				return sizeof(ENCODER_PARAM_S);

			case eLogical_video_decoder:
		    case eLogical_COP_Dynamic_decoder:
		    case eLogical_COP_VSW_decoder:
				return sizeof (DECODER_PARAM_S);

 		   	default:
 		   		return sizeof(TOpenArtReq);
		}
	}
	else
	{
		for (unsigned int i = 0; i < ARRAYSIZE(g_MplMcmsProtocolOpcodeContentStuctSize); i++)
	 	{
 			if (g_MplMcmsProtocolOpcodeContentStuctSize[i].opcode == opcode)
 				return g_MplMcmsProtocolOpcodeContentStuctSize[i].size;
	 	}

 		return 0;
	}
}
