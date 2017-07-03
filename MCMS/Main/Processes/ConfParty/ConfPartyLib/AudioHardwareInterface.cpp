#include "AudioHardwareInterface.h"
#include "ConfPartyDefines.h"
#include "H221.h"
#include "InterfaceType.h"
#include "ConfigHelper.h"
#include "AcRequestStructs.h"
#include "AcIndicationStructs.h"
#include "OpcodesMcmsCardMngrIvrCntl.h"
#include "OpcodesMcmsAudio.h"
#include "OpcodesMcmsAudioCntl.h"
#include "OpcodesMcmsInternal.h"
#include "ArtDefinitions.h"
#include "DecoderResolutionTable.h"
#include "SysConfigKeys.h"

extern CDecoderResolutionTable* GetpDecoderResolutionTable();

/////////////////////////////////////////////////////////////////////////////
CAudioHardwareInterface::CAudioHardwareInterface(ConnectionID ConnectionId,
	PartyRsrcID ParId,
	ConfRsrcID ConfId,
	eLogicalResourceTypes LRT)
{
	PTRACE2INT(eLevelInfoNormal,"CAudioHardwareInterface::CAudioHardwareInterface - BRIDGE-12931 - resource params initiated with connectionId - ", ConnectionId);
	m_pRsrcParams = new CRsrcParams(ConnectionId, ParId, ConfId, LRT);
}

/////////////////////////////////////////////////////////////////////////////
CAudioHardwareInterface::~CAudioHardwareInterface()
{
}

/////////////////////////////////////////////////////////////////////////////
DWORD CAudioHardwareInterface::OpenDecoder(
	WORD wNetworkInterface,
	BYTE byConfSampleRate, BYTE byNumberOfChannels,
	DWORD dwAudioAlgorithm, BOOL isMuted, DWORD dwVolume,
	BOOL isErrorConcealment, BOOL isAGC, BOOL isToneRemove,
	BOOL isNoiseReduction, BOOL isT1CptDetection,
	BOOL isDtmfDetection, BOOL isNoiseDetection, BYTE byNoiseDetectionThreshold,
	BOOL isVideoParticipant, BOOL isVtxSupport,
	BOOL isEchoSuppression, BYTE byCallDirection, BOOL isStandalone,
	DWORD audioDecoderCompressedDelay,
	BOOL isKeyboardSuppression,
	BOOL isAutoMuteNoisyParties,
	BOOL isAudioClarity,
	DWORD wVolumeAdjustment,
	DWORD ssrc, WORD confSpeakerChangeMode,
	BOOL bIsCallGenerator, BOOL isRelayToMix, MSFT_CLIENT_ENUM eMsftClientType,
	DWORD maxAverageBitrate)
{
	TAudioOpenDecoderReq audioOpenDecoderStruct;
	memset(&audioOpenDecoderStruct, 0, sizeof(audioOpenDecoderStruct));

	audioOpenDecoderStruct.bnCompressedAudioDelay     = FALSE;
	audioOpenDecoderStruct.nCompressedAudioDelayValue = 0;

	CDecoderResolutionTable* pDecoderResolutionTable = ::GetpDecoderResolutionTable();
	if (pDecoderResolutionTable != NULL && audioDecoderCompressedDelay > 0)
	{
		audioOpenDecoderStruct.bnCompressedAudioDelay     = TRUE;
		audioOpenDecoderStruct.nCompressedAudioDelayValue = audioDecoderCompressedDelay;
	}

	// (1) Set Network Type
	if (ISDN_INTERFACE_TYPE == wNetworkInterface)
	{
		if (isVideoParticipant)
			audioOpenDecoderStruct.enNetworkType = E_NETWORK_TYPE_ISDN;
		else
			audioOpenDecoderStruct.enNetworkType = E_NETWORK_TYPE_PSTN_E1;
	}
	else   // IP
	{
		audioOpenDecoderStruct.enNetworkType = E_NETWORK_TYPE_IP;
	}

	// (2) Set Audio Connection Status
	audioOpenDecoderStruct.enAudioConnectionStatus = E_AUDIO_CONNECTION_STATUS_MIX; // Moti: temp always send MIX

	// (3) Set Media Type
	if (isVideoParticipant)
		audioOpenDecoderStruct.enPartyMediaType = E_PARTY_MEDIA_TYPE_VIDEO;
	else
		audioOpenDecoderStruct.enPartyMediaType = E_PARTY_MEDIA_TYPE_AUDIO;

	// (4) Set Call Direction
	switch (byCallDirection)
	{
		case DIALIN:
		{
			audioOpenDecoderStruct.enCallDirection = E_CALL_DIRECTION_DIAL_IN;
			break;
		}
		case DIALOUT:
		{
			audioOpenDecoderStruct.enCallDirection = E_CALL_DIRECTION_DIAL_OUT;
			break;
		}
		default:
		{
			audioOpenDecoderStruct.enCallDirection = INVALID;
		}
	}

	// (5) Set Number of Channels
	switch (byNumberOfChannels)
	{
		case AUDIO_MONO_NUM_OF_CHANNELS:
		{
			audioOpenDecoderStruct.nNumberOfChannels = 1;
			break;
		}
		case AUDIO_STEREO_NUM_OF_CHANNELS:
		{
			audioOpenDecoderStruct.nNumberOfChannels = 2;
			break;
		}
		default:
		{
			audioOpenDecoderStruct.nNumberOfChannels = INVALID;
		}
	}

	// (6) Set Conf Sample Rate
	ESampleRate eSampleRate;
	if (!TranslateMcmsSampleRateToApiValues(byConfSampleRate, eSampleRate))
	{
		PASSERTSTREAM_AND_RETURN_VALUE(true, "Rate:" << byConfSampleRate << " - Invalid sample rate", -1);
	}

	audioOpenDecoderStruct.enConfSampleRate = eSampleRate;

	// (7) Set Audio Algorithm
	EAudioAlgorithm eApiAudioAlgorithm;
	if (!TranslateMcmsAudioAlgorithmOpcodeToApiValues(dwAudioAlgorithm, eApiAudioAlgorithm))
	{
		PASSERTSTREAM_AND_RETURN_VALUE(true, "Algorithm:" << dwAudioAlgorithm << " - Invalid audio algorithm", -1);
	}

	audioOpenDecoderStruct.enDecoderAlgorithm = eApiAudioAlgorithm;

	// (8) Set Audio Volume
	EAudioGainPreset eApiAudioVolume;
	if (!TranslateMcmsVolumeToApiValues(dwVolume, eApiAudioVolume))
	{
		PASSERTSTREAM_AND_RETURN_VALUE(true, "Volume:" << dwVolume << " - Invalid audio volume", -1);
	}

	audioOpenDecoderStruct.enDecoderGain = eApiAudioVolume;

	// (9) Set Noise Detection Threshold
	audioOpenDecoderStruct.enNoiseDetectionThreshold = byNoiseDetectionThreshold;

	// (10) Set flags
	audioOpenDecoderStruct.bnDecoderMute           = isMuted;
	audioOpenDecoderStruct.bnVtxSupport            = isVtxSupport;
	audioOpenDecoderStruct.bnErrorConcealment      = isErrorConcealment;
	audioOpenDecoderStruct.bnAgc                   = isAGC;
	audioOpenDecoderStruct.bnToneRemove            = isToneRemove;
	audioOpenDecoderStruct.bnNoiseReduction        = isNoiseReduction;
	audioOpenDecoderStruct.bnT1CptDetection        = isT1CptDetection;
	audioOpenDecoderStruct.bnDtmfDetection         = isDtmfDetection;
	audioOpenDecoderStruct.bnNoiseDetection        = isNoiseDetection;
	audioOpenDecoderStruct.bnStandalone            = isStandalone;
	audioOpenDecoderStruct.bnEchoSuppression       = isEchoSuppression;
	audioOpenDecoderStruct.bnKeyboardSuppression   = isKeyboardSuppression;
	audioOpenDecoderStruct.bnAudioNewPLC           = GetSystemCfgFlag<BOOL>(CFG_KEY_SET_AUDIO_PLC);
	audioOpenDecoderStruct.bnAudioClarity          = isAudioClarity;
	audioOpenDecoderStruct.enConfSpeakerChangeMode = confSpeakerChangeMode;
	audioOpenDecoderStruct.nEchoDelayMsec          = GetSystemCfgFlag<DWORD>(CFG_KEY_PSTN_ECHO_DELAY_MILLI_SECOND);
	audioOpenDecoderStruct.nSSRC                   = ssrc;

	//FSN-128 - Selective Mixing
	audioOpenDecoderStruct.enSelectiveMixing = isAutoMuteNoisyParties;

	audioOpenDecoderStruct.bnRelayToMix = isRelayToMix;
	audioOpenDecoderStruct.unMsftClient = eMsftClientType;

	// (11) adjust audio volume per algorithm (g729)
	audioOpenDecoderStruct.unDecoderGain = wVolumeAdjustment;

	// Fill parameters for call generator SoftMCU
	audioOpenDecoderStruct.tCallGeneratorParams.bIsCallGenerator = bIsCallGenerator;
	audioOpenDecoderStruct.tCallGeneratorParams.reserved[0]      = 0;
	audioOpenDecoderStruct.tCallGeneratorParams.reserved[1]      = 0;
	audioOpenDecoderStruct.tCallGeneratorParams.reserved[2]      = 0;

	// fill parameters for Opus codec
	audioOpenDecoderStruct.tOpusAudioParams.nBitRate = maxAverageBitrate; // Opus bit-rate

	CSegment msg;

	
	//fill the buffer with the content
	msg.Put((BYTE*)(&audioOpenDecoderStruct), sizeof(TAudioOpenDecoderReq));
	if (m_pRsrcParams) PTRACE2INT(eLevelInfoNormal,"CAudioHardwareInterface::OpenDecoder - BRIDGE-12931 - resource params initiated with connectionId - ", m_pRsrcParams->GetConnectionId());
	DWORD reqId = SendMsgToMPL(AUDIO_OPEN_DECODER_REQ, &msg);
	return reqId;
}

/*UpdateDecoder was added by Yoella and agreed with Avihay B. to support updatePartyType(Audio--->Video/ Video--->Audio)
   We have agreed to define the UpdateEncoder struct as a new struct (diff from OpenEncoder struct since we want to avoid updates that will be send from now but was NOT sent in the past)
   We will add any new param we would like to update, from now on,so the struct will be extended with each new param to update .the struct we will also try to support the old opcodes
   the separet update messeges(update Algo,UpdateMute)
   This was done to be able to update more params in the future with the same struct.*/

///////////////////////////////////////////////////////////////////////////
DWORD CAudioHardwareInterface::UpdateDecoder(BOOL isVideoParticipant)
{
	TAudioUpdateDecoderReq audioUpdateDecoderStruct;

	if (isVideoParticipant)
		audioUpdateDecoderStruct.enPartyMediaType = E_PARTY_MEDIA_TYPE_VIDEO;
	else
		audioUpdateDecoderStruct.enPartyMediaType = E_PARTY_MEDIA_TYPE_AUDIO;

	CSegment msg;

	//fill the buffer with the content
	
	msg.Put((BYTE*)(&audioUpdateDecoderStruct), sizeof(TAudioUpdateDecoderReq));
	if (m_pRsrcParams) PTRACE2INT(eLevelInfoNormal,"CAudioHardwareInterface::UpdateDecoder - BRIDGE-12931 - resource params initiated with connectionId - ", m_pRsrcParams->GetConnectionId());
	DWORD reqId = SendMsgToMPL(AUDIO_UPDATE_DECODER_REQ, &msg);
	return reqId;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CAudioHardwareInterface::CloseDecoder()
{
	TAudioCloseDecoderReq audioCloseDecoderStruct;

	// set its fields
	audioCloseDecoderStruct.nDummy = INVALID;

	CSegment msg;

	//fill the buffer with the content
	msg.Put((BYTE*)&audioCloseDecoderStruct, sizeof(audioCloseDecoderStruct));

	// send the message
	DWORD reqId = SendMsgToMPL(AUDIO_CLOSE_DECODER_REQ, &msg);
	return reqId;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CAudioHardwareInterface::OpenEncoder(
	WORD wNetworkInterface, BYTE byConfSampleRate,
	BYTE byNumberOfChannels, DWORD dwAudioAlgorithm,
	BOOL isMuted, DWORD dwVolume, BOOL isVideoParticipant, BOOL isStandalone,
	DWORD wVolumeAdjustment,
	DWORD numOfSsrcIds, DWORD* ssrc_array,
	EMixModeGet eMixModeSet, DWORD ivrSsrc,
	BOOL isUseSpeakerSsrcForTx, BOOL bIsCallGenerator, BOOL isRelayToMix, DWORD maxAverageBitrate)
{
	TAudioOpenEncoderReq audioOpenEncoderStruct;
	memset(&audioOpenEncoderStruct, 0, sizeof(audioOpenEncoderStruct));

	// (1) Set Network Type
	if (ISDN_INTERFACE_TYPE == wNetworkInterface)
	{
		if (isVideoParticipant)
			audioOpenEncoderStruct.enNetworkType = E_NETWORK_TYPE_ISDN;
		else
			audioOpenEncoderStruct.enNetworkType = E_NETWORK_TYPE_PSTN_E1; //Udi TBD according to ISND service
	}
	else // IP
		audioOpenEncoderStruct.enNetworkType = E_NETWORK_TYPE_IP;

	// (2) Set Number of Channels
	switch (byNumberOfChannels)
	{
		case AUDIO_MONO_NUM_OF_CHANNELS:
			audioOpenEncoderStruct.nNumberOfChannels = 1;
			break;

		case AUDIO_STEREO_NUM_OF_CHANNELS:
			audioOpenEncoderStruct.nNumberOfChannels = 2;
			break;

		default:
			audioOpenEncoderStruct.nNumberOfChannels = INVALID;
			break;
	}

	// (3) Set Conf Sample Rate
	ESampleRate eSampleRate;
	if (!TranslateMcmsSampleRateToApiValues(byConfSampleRate, eSampleRate))
	{
		PASSERTSTREAM_AND_RETURN_VALUE(true, "Rate:" << byConfSampleRate << " - Invalid sample rate", -1);
	}

	audioOpenEncoderStruct.enConfSampleRate = eSampleRate;

	// (4) Set Audio Algorithm
	EAudioAlgorithm eApiAudioAlgorithm;

	if (!TranslateMcmsAudioAlgorithmOpcodeToApiValues(dwAudioAlgorithm, eApiAudioAlgorithm))
	{
		PASSERTSTREAM_AND_RETURN_VALUE(true, "Algorithm:" << dwAudioAlgorithm << " - Invalid audio algorithm", -1);
	}

	audioOpenEncoderStruct.enEncoderAlgorithm = eApiAudioAlgorithm;

	// (5) Set Audio Volume
	EAudioGainPreset eApiAudioVolume;
	if (!TranslateMcmsVolumeToApiValues(dwVolume, eApiAudioVolume))
	{
		PASSERTSTREAM_AND_RETURN_VALUE(true, "Volume:" << dwVolume << " - Invalid audio volume", -1);
	}

	audioOpenEncoderStruct.enEncoderGain = eApiAudioVolume;

	// (6) Set Mute flag
	audioOpenEncoderStruct.bnEncoderMute = isMuted;

	// (7) Set Standalone flag
	audioOpenEncoderStruct.bnStandalone = isStandalone;

	// (8) Set SSRC list
	audioOpenEncoderStruct.stSSRC.numOfSSRC = numOfSsrcIds;
	for (DWORD i = 0; i < numOfSsrcIds; ++i)
		audioOpenEncoderStruct.stSSRC.ssrcList[i] = ssrc_array[i];
	// (9) Set UseSpeakerSsrcForTx flag

	audioOpenEncoderStruct.bUseSpeakerSsrcForTx = isUseSpeakerSsrcForTx;
	audioOpenEncoderStruct.enEMixModeGet        = eMixModeSet;
	audioOpenEncoderStruct.unIvrSsrc            = ivrSsrc;
	audioOpenEncoderStruct.unIvrCsrc            = IVR_CSRC;
	audioOpenEncoderStruct.bnRelayToMix         = isRelayToMix;

	// (8) adjust audio volume per algorithm (g729)
	audioOpenEncoderStruct.unEncoderGain = wVolumeAdjustment;

	// fill parameters for Opus codec
	audioOpenEncoderStruct.tOpusAudioParams.nBitRate = maxAverageBitrate; // Opus bit-rate

	// Fill parameters for call generator SoftMCU
	audioOpenEncoderStruct.tCallGeneratorParams.bIsCallGenerator = bIsCallGenerator;
	audioOpenEncoderStruct.tCallGeneratorParams.reserved[0]      = 0;
	audioOpenEncoderStruct.tCallGeneratorParams.reserved[1]      = 0;
	audioOpenEncoderStruct.tCallGeneratorParams.reserved[2]      = 0;

	CSegment msg;

	//fill the buffer with the content
	msg.Put((BYTE*)(&audioOpenEncoderStruct), sizeof(TAudioOpenEncoderReq));

	if (m_pRsrcParams) PTRACE2INT(eLevelInfoNormal,"CAudioHardwareInterface::OpenEncoder - BRIDGE-12931 - resource params initiated with connectionId - ", m_pRsrcParams->GetConnectionId());
	DWORD reqId = SendMsgToMPL(AUDIO_OPEN_ENCODER_REQ, &msg);
	return reqId;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CAudioHardwareInterface::CloseEncoder()
{
	TAudioCloseEncoderReq audioCloseEncoderStruct;

	// set its fields
	audioCloseEncoderStruct.nDummy = INVALID;

	CSegment msg;

	//fill the buffer with the content
	msg.Put((BYTE*)&audioCloseEncoderStruct, sizeof(audioCloseEncoderStruct));

	// send the message
	DWORD reqId = SendMsgToMPL(AUDIO_CLOSE_ENCODER_REQ, &msg);
	return reqId;
}

/////////////////////////////////////////////////////////////////////////////
void CAudioHardwareInterface::UpdateMute(BOOL isMuted)
{
	TAudioUpdateMuteReq audioUpdateMuteStruct;

	eLogicalResourceTypes eLRT = GetLogicalRsrcType();

	switch (eLRT)
	{
		case eLogical_audio_encoder:
		case eLogical_relay_audio_encoder:
		{
			audioUpdateMuteStruct.enAudioChannelType = E_AUDIO_CHANNEL_TYPE_ENCODER;
			break;
		}

		case eLogical_audio_decoder:
		case eLogical_relay_audio_decoder:
		{
			audioUpdateMuteStruct.enAudioChannelType = E_AUDIO_CHANNEL_TYPE_DECODER;
			break;
		}
		default:
		{
			audioUpdateMuteStruct.enAudioChannelType = INVALID;
		}
	}

	audioUpdateMuteStruct.bnMute = isMuted;

	CSegment msg;

	//fill the buffer with the content
	msg.Put((BYTE*)&audioUpdateMuteStruct, sizeof(audioUpdateMuteStruct));

	// send the message
	SendMsgToMPL(AUDIO_UPDATE_MUTE_REQ, &msg);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioHardwareInterface::UpdateAlgorithm(DWORD dwNewAudioAlgorithm, DWORD maxAverageBitrate)
{
	TAudioUpdateAlgorithmReq audioUpdateAlgorithmStruct;

	EAudioAlgorithm eApiAudioAlgorithm;
	if (!TranslateMcmsAudioAlgorithmOpcodeToApiValues(dwNewAudioAlgorithm, eApiAudioAlgorithm))
	{
		PASSERTSTREAM_AND_RETURN(true, "Algorithm:" << dwNewAudioAlgorithm << " - Invalid audio algorithm");
	}

	audioUpdateAlgorithmStruct.enAudioAlgorithm = eApiAudioAlgorithm;

	switch (eApiAudioAlgorithm)
	{
		// G729
		case G729_8k:
			audioUpdateAlgorithmStruct.unCodecGain = GetSystemCfgFlag<DWORD>(CFG_KEY_AUDIO_ENCODER_GAIN_G729);
			break;

		// G711
		case A_Law_OU:
		case U_Law_OU:
		case A_Law_OF:
		case U_Law_OF:
			audioUpdateAlgorithmStruct.unCodecGain = GetSystemCfgFlag<DWORD>(CFG_KEY_AUDIO_ENCODER_GAIN_G711);
			break;

		// G722
		case G722_m1:
		case G722_m2:
		case G722_m3:
			audioUpdateAlgorithmStruct.unCodecGain = GetSystemCfgFlag<DWORD>(CFG_KEY_AUDIO_ENCODER_GAIN_G722);
			break;

		// G722.1
		case Au_32k:
		case Au_24k:
		case Au_G7221_16k:
		case Au_Siren7_16k:  // Siren7 is the pre-standardized version of G722.1
			audioUpdateAlgorithmStruct.unCodecGain = GetSystemCfgFlag<DWORD>(CFG_KEY_AUDIO_ENCODER_GAIN_G722_1);
			break;

	case    Au_Opus_64k:
	case    Au_OpusStereo_128k:
		// need to implement Opus codec here? Currently take the default value (100)
		break;

		default:
			audioUpdateAlgorithmStruct.unCodecGain = 100;
			break;
	}


	// fill parameters for Opus codec
	audioUpdateAlgorithmStruct.tOpusAudioParams.nBitRate = maxAverageBitrate; // Opus bit-rate

	CSegment msg;
	msg.Put((BYTE*)&audioUpdateAlgorithmStruct, sizeof(audioUpdateAlgorithmStruct));
	SendMsgToMPL(AUDIO_UPDATE_ALGORITHM_REQ, &msg);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioHardwareInterface::UpdateVolume(DWORD dwNewVolume)
{
	TAudioUpdateGainReq audioUpdateGainStruct;

	eLogicalResourceTypes eLRT = GetLogicalRsrcType();

	switch (eLRT)
	{
		case eLogical_audio_encoder:
		case eLogical_relay_audio_encoder:
		{
			audioUpdateGainStruct.enAudioChannelType = E_AUDIO_CHANNEL_TYPE_ENCODER;
			break;
		}
		case eLogical_audio_decoder:
		case eLogical_relay_audio_decoder:
		{
			audioUpdateGainStruct.enAudioChannelType = E_AUDIO_CHANNEL_TYPE_DECODER;
			break;
		}
		default:
		{
			audioUpdateGainStruct.enAudioChannelType = INVALID;
		}
	}

	EAudioGainPreset eApiAudioVolume;
	if (!TranslateMcmsVolumeToApiValues(dwNewVolume, eApiAudioVolume))
	{
		PASSERTSTREAM_AND_RETURN(true, "Volume:" << dwNewVolume << " - Invalid audio volume");
	}

	audioUpdateGainStruct.enGain = eApiAudioVolume;

	CSegment msg;

	//fill the buffer with the content
	msg.Put((BYTE*)&audioUpdateGainStruct, sizeof(audioUpdateGainStruct));

	// send the message
	SendMsgToMPL(AUDIO_UPDATE_GAIN_REQ, &msg);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioHardwareInterface::CGPlayAudioReq(DWORD dwNewVolume)
{
	TCGPlayAudioReqStruct callGenPlayAudioReqStruct;
	memset(&callGenPlayAudioReqStruct, 0, sizeof(callGenPlayAudioReqStruct));

	EAudioGainPreset eApiAudioVolume;
	if (!TranslateMcmsVolumeToApiValues(dwNewVolume, eApiAudioVolume))
	{
		PASSERTSTREAM_AND_RETURN(true, "Volume:" << dwNewVolume << " - Invalid audio volume");
	}

	callGenPlayAudioReqStruct.enAudioGain       = eApiAudioVolume;
	callGenPlayAudioReqStruct.unNumOfRepetition = 0; // Repeat forever

	memset(callGenPlayAudioReqStruct.fileName, 0, sizeof(callGenPlayAudioReqStruct.fileName));
	strncpy(callGenPlayAudioReqStruct.fileName, (MCU_MRMX_DIR+"/res/cg/audio/cg_audio_").c_str(), sizeof(callGenPlayAudioReqStruct.fileName)-1);
    char file_num[8] = "";
    snprintf(file_num, 8, "%d", dwNewVolume);
	strcat(callGenPlayAudioReqStruct.fileName, file_num);
	strcat(callGenPlayAudioReqStruct.fileName, ".wav");

	callGenPlayAudioReqStruct.fileNameLength = strlen(callGenPlayAudioReqStruct.fileName);

	CSegment msg;

	//fill the buffer with the content
	msg.Put((BYTE*)&callGenPlayAudioReqStruct, sizeof(callGenPlayAudioReqStruct));

	// send the message
	SendMsgToMPL(CG_PLAY_AUDIO_REQ, &msg);
}

/////////////////////////////////////////////////////////////////////////////
// speakerIndication - update
void CAudioHardwareInterface::UpdateUseSpeakerSsrcForTx(BOOL bUseSpeakerSsrc)
{
	TAudioUpdateUseSpeakerSsrcForTxReq audioUpdateUseSpeakerSsrcForTxStruct;

	audioUpdateUseSpeakerSsrcForTxStruct.bUseSpeakerSsrcForTx = bUseSpeakerSsrc;

	CSegment msg;

	//fill the buffer with the content
	msg.Put((BYTE*)&audioUpdateUseSpeakerSsrcForTxStruct, sizeof(audioUpdateUseSpeakerSsrcForTxStruct));

	// send the message
	SendMsgToMPL(AUDIO_UPDATE_USE_SPEAKER_SSRC_FOR_TX_REQ, &msg);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioHardwareInterface::UpdateNoiseDetection(BOOL isNoiseDetection, BYTE NoiseDetectionThreshold)
{
	TAudioUpdateNoiseDetectionReq audioUpdateNoiseDetectionStruct;

	audioUpdateNoiseDetectionStruct.bnNoiseDetection          = isNoiseDetection;
	audioUpdateNoiseDetectionStruct.enNoiseDetectionThreshold = NoiseDetectionThreshold;

	CSegment msg;

	//fill the buffer with the content
	msg.Put((BYTE*)&audioUpdateNoiseDetectionStruct, sizeof(audioUpdateNoiseDetectionStruct));

	// send the message
	SendMsgToMPL(AUDIO_UPDATE_NOISE_DETECTION_REQ, &msg);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioHardwareInterface::UpdateAGC(BOOL isAGC)
{
	TAudioUpdateAgcReq audioUpdateAgcStruct;

	audioUpdateAgcStruct.bnAgc = isAGC;

	CSegment msg;

	//fill the buffer with the content
	msg.Put((BYTE*)&audioUpdateAgcStruct, sizeof(audioUpdateAgcStruct));

	// send the message
	SendMsgToMPL(AUDIO_UPDATE_AGC_REQ, &msg);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioHardwareInterface::OpenConf(WORD wTalkHoldTime, BYTE byAudioMixDepth, BOOL isAutoMuteNoisyParties, WORD bReOpenConf,WORD new_card_board_id, WORD activeSpeakerPreference)
{
	TAcOpenConfReq openConfStruct;

	// set its fields
	openConfStruct.bunVideoSpeaker      = 1;
	openConfStruct.unTalkHoldTime       = wTalkHoldTime;
	openConfStruct.bunAudioSpeaker      = 1;
	openConfStruct.unAudioSpeakerWindow = wTalkHoldTime;
	openConfStruct.unReqAudioMixDepth   = byAudioMixDepth;

	//Feature Speak change threshold
	//1.	Default speaker change	- for talk hold time 3 or 5
	//2.	Fast speaker change  - for talk hold time 1.5
	openConfStruct.enConfSpeakerChangeMode = wTalkHoldTime <= 150 ? E_CONF_FAST_SPEAKER_CHANGE_MODE : E_CONF_DEFAULT_SPEAKER_CHANGE_MODE;

	//FSN-128 - Selective Mixing
	openConfStruct.bunSelectiveMixing = isAutoMuteNoisyParties;

	// BRIDGE-9818
	openConfStruct.unLastSpeakerPreference = (APIU32)activeSpeakerPreference;

	CSegment msg;

	//fill the buffer with the content
	msg.Put((BYTE*)&openConfStruct, sizeof(openConfStruct));

	// send the message
	if (bReOpenConf)
		SendMsgToMPL(AC_OPEN_CONF_RESEND, &msg, new_card_board_id);
	else
		SendMsgToMPL(AC_OPEN_CONF_REQ, &msg);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioHardwareInterface::UpdateConf(WORD wTalkHoldTime, BYTE byAudioMixDepth)
{
	TAcUpdateConfParamsReq updateConfStruct;

	// set its fields
	updateConfStruct.bunVideoSpeaker      = 1;
	updateConfStruct.unTalkHoldTime       = wTalkHoldTime;
	updateConfStruct.bunAudioSpeaker      = 1;
	updateConfStruct.unAudioSpeakerWindow = wTalkHoldTime;
	updateConfStruct.unReqAudioMixDepth   = byAudioMixDepth;

	CSegment msg;

	//fill the buffer with the content
	msg.Put((BYTE*)&updateConfStruct, sizeof(updateConfStruct));

	// send the message
	SendMsgToMPL(AC_UPDATE_CONF_PARAMS_REQ, &msg);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioHardwareInterface::CloseConf()
{
	TAcCloseConfReq closeConfStruct;

	// set its fields
	closeConfStruct.unDummy = INVALID;

	CSegment msg;

	//fill the buffer with the content
	msg.Put((BYTE*)&closeConfStruct, sizeof(closeConfStruct));

	// send the message
	SendMsgToMPL(AC_CLOSE_CONF_REQ, &msg);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioHardwareInterface::ACLayoutChangeComplete()
{
	SendMsgToMPL(AC_LAYOUT_CHANGE_COMPLETE_REQ, NULL);
}

/////////////////////////////////////////////////////////////////////////////
DWORD CAudioHardwareInterface::PlayTone(CSegment* pParam)
{
	std::string toneBuffer;

	DWORD sequenceNum = 0;
	//We send TONE_LIST_SIZE_MAX tones at a time
	//Translating each "tone buffer" as follow: for each tone we will send silence and then the tone
	//                                          and at the end of the buffer add a silence tone as well
	*pParam >> toneBuffer;

	WORD numTones = toneBuffer.length();

	SPlayToneStruct tPlayToneStruct;
	memset(&tPlayToneStruct, 0, sizeof(tPlayToneStruct));

	WORD rStatus = FillPlayToneParams(toneBuffer.c_str(), numTones, &tPlayToneStruct);

	TRACEINTO << "NumberOfTones:" << tPlayToneStruct.numOfTones;

	if (rStatus < E_AUDIO_TONE_STATUS_FAIL_NUM_OF_TONE_0)
	{
		CSegment msg;
		msg.Put((BYTE*)&tPlayToneStruct, sizeof(tPlayToneStruct));
		sequenceNum = SendMsgToMPL(AUDIO_PLAY_TONE_REQ, &msg);
	}

	if (rStatus != E_AUDIO_TONE_STATUS_OK)
	{
		DBGPASSERT(rStatus);
	}
	return sequenceNum;
}

/////////////////////////////////////////////////////////////////////////////
WORD CAudioHardwareInterface::FillPlayToneParams(const char* toneBuffer, WORD numTones, SPlayToneStruct* tPlayToneStruct)
{
	DWORD rToneSilenceDuration = 20;
	DWORD rToneGainValue       = 4;

	rToneSilenceDuration = GetSystemCfgFlag<DWORD>("SILNECE_DURATION_FOR_DTMF_FORWARDING");
	rToneGainValue = GetSystemCfgFlag<DWORD>("GAIN_AUDIO_FOR_DTMF_FORWARDING");

	TRACEINTO << "ToneNumber:" << numTones << ", ToneBuffer:" << DUMPSTR(toneBuffer) << ", ToneSilenceDuration:" << rToneSilenceDuration << ", ToneGainValue:" << rToneGainValue;

	WORD rStatus = CheckValidityOfToneBufAndNum(toneBuffer, numTones);
	if (rStatus != E_AUDIO_TONE_STATUS_OK)
	{
		TRACEINTO << "Invalid Tone number or toneBuffer";
		return rStatus;
	}

	int rCurentToneNum    = 0;
	int rPlayToneInd      = 0;
	int rLimitHighToneNum = ((TONE_LIST_SIZE_MAX / 2) - 1);

	/*Check that number of tones is permitted*/
	if (numTones > rLimitHighToneNum)
	{
		rCurentToneNum = rLimitHighToneNum;
		rStatus = E_AUDIO_TONE_STATUS_FAIL_TONE_NUMBER_BIGGER_THEN31;
		TRACEINTO << "CurrentToneNumner:" << rCurentToneNum << " - The number of tone is bigger then limit";
	}
	else
	{
		rCurentToneNum = numTones;
		TRACEINTO << "CurrentToneNumner:" << rCurentToneNum;
	}

	/*Start fill the tone: for example: "*2" should be "silence silence * silence 2 silence" */

	int rCurrentToneAndGapNum = (rCurentToneNum * 2) + 1;  // Tone,silence and plus silence at the and
	//First Silence Tone Should be 100 mili cause of the problem of DTMF detection

	for (int rBuffInd = 0; (rBuffInd < rCurentToneNum || rPlayToneInd < rCurrentToneAndGapNum); rPlayToneInd++)
	{
		/*In case when index is 0 or odd put silence tone*/
		if (!(rPlayToneInd % 2))
		{
			tPlayToneStruct->tone[rPlayToneInd].tTone    = E_AUDIO_TONE_SILENCE; // Tone Type (EAudioTone)
			tPlayToneStruct->tone[rPlayToneInd].duration = rToneSilenceDuration; // duration of note (in Hsyncs)
		}
		else
		{
			/*Otherwise put tone from buffer tone*/
			tPlayToneStruct->tone[rPlayToneInd].tTone    = GetToneId(toneBuffer[rBuffInd]); // Tone Type (EAudioTone)
			tPlayToneStruct->tone[rPlayToneInd].duration = TONE_DURATION_DIGIT;             // duration of note (in Hsyncs)
			rBuffInd++;
		}
		tPlayToneStruct->tone[rPlayToneInd].toneGain = rToneGainValue;
	}

	/*Put tone number including gaps*/
	tPlayToneStruct->numOfTones = rPlayToneInd;

	/*Put number of repetition in our case we play only once (took from MGC)*/
	tPlayToneStruct->numOfRepetition = PLAY_TONES_ONCE;

	return rStatus;
}

/////////////////////////////////////////////////////////////////////////////
unsigned long CAudioHardwareInterface::GetToneId(char ch)
{
	switch (ch)
	{
		case '0': return E_AUDIO_TONE_DTMF_0;
		case '1': return E_AUDIO_TONE_DTMF_1;
		case '2': return E_AUDIO_TONE_DTMF_2;
		case '3': return E_AUDIO_TONE_DTMF_3;
		case '4': return E_AUDIO_TONE_DTMF_4;
		case '5': return E_AUDIO_TONE_DTMF_5;
		case '6': return E_AUDIO_TONE_DTMF_6;
		case '7': return E_AUDIO_TONE_DTMF_7;
		case '8': return E_AUDIO_TONE_DTMF_8;
		case '9': return E_AUDIO_TONE_DTMF_9;
		case '*': return E_AUDIO_TONE_DTMF_STAR;
		case '#': return E_AUDIO_TONE_DTMF_PAUND;
		case '$': return E_AUDIO_TONE_DTMF_PAUND;
		case 'A': return E_AUDIO_TONE_DTMF_A;
		case 'B': return E_AUDIO_TONE_DTMF_B;
		case 'C': return E_AUDIO_TONE_DTMF_C;
		case 'D': return E_AUDIO_TONE_DTMF_D;
	}
	return E_AUDIO_TONE_SILENCE;
}

/////////////////////////////////////////////////////////////////////////////
WORD CAudioHardwareInterface::CheckValidityOfToneBufAndNum(const char* toneBuffer, WORD numTones)
{
	if (0 == numTones)
	{
		TRACEINTOLVLERR << "ToneNumber:0 - Failed, illegal tones number";
		return E_AUDIO_TONE_STATUS_FAIL_NUM_OF_TONE_0;
	}

	for (int i = 0; i < numTones; i++)
	{
		if ((toneBuffer[i] <= '9') && (toneBuffer[i] >= '0'))
			continue;
		if ((toneBuffer[i] != '#') && (toneBuffer[i] != '*'))
		{
			if (CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator)
			{
				TRACEINTOLVLERR << "Tone:" << toneBuffer[i] << " - Failed, illegal DTMF value";
				return E_AUDIO_TONE_STATUS_FAIL_INCORRECT_TONE;
			}
			else
			{
				if ((toneBuffer[i] != 'A') && (toneBuffer[i] != 'B') && (toneBuffer[i] != 'C'))
				{
					TRACEINTOLVLERR << "Tone:" << toneBuffer[i] << " - Failed, illegal DTMF value for Call Generator";
					return E_AUDIO_TONE_STATUS_FAIL_INCORRECT_TONE;
				}
			}
		}
	}
	return E_AUDIO_TONE_STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CAudioHardwareInterface::PlayMessage(CSegment* pDataSeg)
{
	return SendMsgToMPL(IVR_PLAY_MESSAGE_REQ, pDataSeg);
}

/////////////////////////////////////////////////////////////////////////////
DWORD CAudioHardwareInterface::StopPlayMessage(CSegment* pDataSeg)
{
	return SendMsgToMPL(IVR_STOP_PLAY_MESSAGE_REQ, pDataSeg);
}

/////////////////////////////////////////////////////////////////////////////
DWORD CAudioHardwareInterface::StartIVR(CSegment* pDataSeg)
{
	return SendMsgToMPL(IVR_START_IVR_REQ, pDataSeg);
}

/////////////////////////////////////////////////////////////////////////////
DWORD CAudioHardwareInterface::StopIVR(CSegment* pDataSeg)
{
	return SendMsgToMPL(IVR_STOP_IVR_REQ, pDataSeg);
}

/////////////////////////////////////////////////////////////////////////////
DWORD CAudioHardwareInterface::PlayMusic(CSegment* pDataSeg)
{
	return SendMsgToMPL(IVR_PLAY_MUSIC_REQ, pDataSeg);
}

/////////////////////////////////////////////////////////////////////////////
DWORD CAudioHardwareInterface::StopPlayMusic(CSegment* pDataSeg)
{
	return SendMsgToMPL(IVR_STOP_PLAY_MUSIC_REQ, pDataSeg);
}

/////////////////////////////////////////////////////////////////////////////
DWORD CAudioHardwareInterface::RecordRollCall(CSegment* pDataSeg)
{
	return SendMsgToMPL(IVR_RECORD_ROLL_CALL_REQ, pDataSeg);
}
/////////////////////////////////////////////////////////////////////////////
DWORD CAudioHardwareInterface::StopRecordRollCall(CSegment *pDataSeg)
{
	return SendMsgToMPL( IVR_STOP_RECORD_ROLL_CALL_REQ, pDataSeg );
}
/////////////////////////////////////////////////////////////////////////////
void CAudioHardwareInterface::UpdateStandalone(BYTE isStandalone)
{
	TAudioUpdateStandaloneReq audioUpdateStandaloneStruct;

	audioUpdateStandaloneStruct.bnStandalone = isStandalone;

	CSegment msg;
	msg.Put((BYTE*)&audioUpdateStandaloneStruct, sizeof(audioUpdateStandaloneStruct));
	SendMsgToMPL(IVR_UPDATE_STANDALONE_REQ, &msg);
}

/////////////////////////////////////////////////////////////////////////////
BOOL CAudioHardwareInterface::TranslateMcmsAudioAlgorithmOpcodeToApiValues(DWORD dwMcmsAudioAlgorithmOpcode, EAudioAlgorithm& eApiAudioAlgorithm)
{
	switch (dwMcmsAudioAlgorithmOpcode)
	{
		case A_Law_OU                  : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_G711A_64; return TRUE;
		case U_Law_OU                  : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_G711U_64; return TRUE;
		case A_Law_OF                  : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_G711A_56; return TRUE;
		case U_Law_OF                  : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_G711U_56; return TRUE;
		case G728                      : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_G728; return TRUE;
		case G729_8k                   : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_G729; return TRUE;
		case A_Law_48                  : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_G711A_48; return TRUE;
		case U_Law_48                  : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_G711U_48; return TRUE;
		case G722_m1                   : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_G722_64; return TRUE;
		case G722_m2                   : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_G722_56; return TRUE;
		case G722_m3                   : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_G722_48; return TRUE;
		case Au_G722_Stereo_128        : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_G722_Stereo_128; return TRUE;
		case Au_G7221_16k              : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_G7221_16; return TRUE;
		case Au_24k                    : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_G7221_24; return TRUE;
		case Au_32k                    : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_G7221_32; return TRUE;
		case Au_Siren7_16k             : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_SIREN7_16; return TRUE;
		case Au_Siren7_24k             : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_SIREN7_24; return TRUE;
		case Au_Siren7_32k             : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_SIREN7_32; return TRUE;
		case G723_1_Command            : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_G723_63; return TRUE;
		case Au_Siren14_24k            : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_SIREN14_24; return TRUE;
		case Au_Siren14_32k            : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_SIREN14_32; return TRUE;
		case Au_Siren14_48k            : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_SIREN14_48; return TRUE;
		case G7221_AnnexC_24k          : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_G7221C_24; return TRUE;
		case G7221_AnnexC_32k          : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_G7221C_32; return TRUE;
		case G7221_AnnexC_48k          : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_G7221C_48; return TRUE;
		case Au_Siren14S_48k           : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_SIREN14S_48; return TRUE;
		case Au_Siren14S_56k           : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_SIREN14S_56; return TRUE;
		case Au_Siren14S_64k           : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_SIREN14S_64; return TRUE;
		case Au_Siren14S_96k           : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_SIREN14S_96; return TRUE;
		case Au_Siren22S_128k          : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_SIREN22S_128; return TRUE;
		case Au_Siren22S_96k           : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_SIREN22S_96; return TRUE;
		case Au_Siren22S_64k           : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_SIREN22S_64; return TRUE;
		case Au_Siren22_64k            : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_SIREN22_64; return TRUE;
		case Au_Siren22_48k            : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_SIREN22_48; return TRUE;
		case Au_Siren22_32k            : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_SIREN22_32; return TRUE;
		case Au_SirenLPR_32k           : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_SIRENLPR_32; return TRUE;
		case Au_SirenLPR_48k           : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_SIRENLPR_48; return TRUE;
		case Au_SirenLPR_64k           : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_SIRENLPR_64; return TRUE;
		case Au_SirenLPRS_64k          : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_SIRENLPRStereo_64; return TRUE;
		case Au_SirenLPRS_96k          : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_SIRENLPRStereo_96; return TRUE;
		case Au_SirenLPRS_128k         : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_SIRENLPRStereo_128; return TRUE;
		case G719_64k                  : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_G719_64; return TRUE;
		case G719_48k                  : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_G719_48; return TRUE;
		case G719_32k                  : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_G719_32; return TRUE;
		case G719S_128k                : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_G719S_128; return TRUE;
		case G719S_96k                 : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_G719S_96; return TRUE;
		case G719S_64k                 : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_G719S_64; return TRUE;
		case Au_AAC_LD                 : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_AAC_LD; return TRUE;
		case Au_SirenLPR_Scalable_32k  : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_SAC_32; return TRUE;
		case Au_SirenLPR_Scalable_48k  : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_SAC_48; return TRUE;
		case Au_SirenLPR_Scalable_64k  : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_SAC_64; return TRUE;
		case Au_SirenLPRS_Scalable_64k : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_SAC_Stereo_64; return TRUE;
		case Au_SirenLPRS_Scalable_96k : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_SAC_Stereo_96; return TRUE;
		case Au_SirenLPRS_Scalable_128k: eApiAudioAlgorithm = E_AUDIO_ALGORITHM_SAC_Stereo_128; return TRUE;
		case Au_iLBC_13k               : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_iLBC_13; return TRUE;
		case Au_iLBC_15k               : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_iLBC_15; return TRUE;
    // Opus
    	case Au_Opus_64k	    	   : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_OPUS; return TRUE;
  		case Au_OpusStereo_128k	       : eApiAudioAlgorithm = E_AUDIO_ALGORITHM_OPUS_Stereo; ; return TRUE;

    }

	eApiAudioAlgorithm = E_AUDIO_ALGORITHM_DUMMY;
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CAudioHardwareInterface::TranslateMcmsVolumeToApiValues(DWORD dwMcmsVolume, EAudioGainPreset& eApiAudioVolume)
{
	switch (dwMcmsVolume)
	{
		case 0 : eApiAudioVolume = E_AUDIO_GAIN_PRESET_0; return TRUE;
		case 1 : eApiAudioVolume = E_AUDIO_GAIN_PRESET_1; return TRUE;
		case 2 : eApiAudioVolume = E_AUDIO_GAIN_PRESET_2; return TRUE;
		case 3 : eApiAudioVolume = E_AUDIO_GAIN_PRESET_3; return TRUE;
		case 4 : eApiAudioVolume = E_AUDIO_GAIN_PRESET_4; return TRUE;
		case 5 : eApiAudioVolume = E_AUDIO_GAIN_PRESET_5; return TRUE;
		case 6 : eApiAudioVolume = E_AUDIO_GAIN_PRESET_6; return TRUE;
		case 7 : eApiAudioVolume = E_AUDIO_GAIN_PRESET_7; return TRUE;
		case 8 : eApiAudioVolume = E_AUDIO_GAIN_PRESET_8; return TRUE;
		case 9 : eApiAudioVolume = E_AUDIO_GAIN_PRESET_9; return TRUE;
		case 10: eApiAudioVolume = E_AUDIO_GAIN_PRESET_10; return TRUE;
	}

	eApiAudioVolume = E_AUDIO_GAIN_PRESET_DUMMY;
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CAudioHardwareInterface::TranslateMcmsSampleRateToApiValues(BYTE byMcmsSampleRate, ESampleRate& eSampleRate)
{
	switch (byMcmsSampleRate)
	{
		case AUDIO_SAMPLE_RATE_8KHZ : eSampleRate = E_SAMPLE_RATE_8KHZ; return TRUE;
		case AUDIO_SAMPLE_RATE_16KHZ: eSampleRate = E_SAMPLE_RATE_16KHZ; return TRUE;
		case AUDIO_SAMPLE_RATE_32KHZ: eSampleRate = E_SAMPLE_RATE_32KHZ; return TRUE;
		case AUDIO_SAMPLE_RATE_48KHZ: eSampleRate = E_SAMPLE_RATE_48KHZ; return TRUE;
	}

	eSampleRate = E_SAMPLE_RATE_DUMMY;
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
bool CAudioHardwareInterface::UpdateAudioDelay(TAudioUpdateCompressedAudioDelayReq* pstAudioDelay)
{
	CSegment msg;
	msg.Put((BYTE*)pstAudioDelay, sizeof(TAudioUpdateCompressedAudioDelayReq));
	SendMsgToMPL(AUDIO_UPDATE_COMPRESSED_AUDIO_DELAY_REQ, &msg);
	return true;
}

/////////////////////////////////////////////////////////////////////////////
bool CAudioHardwareInterface::UpdateAudioRelayParamsIn(TRtpUpdateRelayReq* pstRelayParamsIn)
{
	CSegment msg;
	msg.Put((BYTE*)pstRelayParamsIn, sizeof(TRtpUpdateRelayReq));
	SendMsgToMPL(AUDIO_UPDATE_RELAY_DEC_PARAMS_REQ, &msg);
	return true;
}

/////////////////////////////////////////////////////////////////////////////
bool CAudioHardwareInterface::UpdateAudioRelayParamsOut(TRtpUpdateRelayReq* pstRelayParamsOut)
{
	CSegment msg;
	msg.Put((BYTE*)pstRelayParamsOut, sizeof(TRtpUpdateRelayReq));
	SendMsgToMPL(AUDIO_UPDATE_RELAY_ENC_PARAMS_REQ, &msg);
	return true;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CAudioHardwareInterface::SendAudioEncoderUpdateImageSeenSsrc(DWORD ssrc)
{
	TAudioEncoderUpdateSeenImageSsrcReq tUpdateSeenImageSsrcReq;
	memset(&tUpdateSeenImageSsrcReq, 0, sizeof(tUpdateSeenImageSsrcReq));

	tUpdateSeenImageSsrcReq.unSSRC = ssrc;

	CSegment msg;
	msg.Put((BYTE*)&tUpdateSeenImageSsrcReq, sizeof(tUpdateSeenImageSsrcReq));
	return SendMsgToMPL(AUDIO_ENCODER_UPDATE_SEEN_IMAGE_SSRC, &msg);
}

