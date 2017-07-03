//+========================================================================+
//                   BridgePartyAudioParams.CPP                            |
//					 Copyright 2005 Polycom, Inc                           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// FILE:       BridgePartyAudioParams.CPP                                  |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Matvey                                                      |
//-------------------------------------------------------------------------|
// Who  | Date  June-2005  | Description                                   |
//-------------------------------------------------------------------------|
//
//+========================================================================+


#include "BridgePartyAudioParams.h"
#include "H221.h"
#include "ConfPartyGlobals.h"
#include "TraceStream.h"

/////////////////////////////////////////////////////////////////////////////
// CLASS CBridgePartyAudioParams
/////////////////////////////////////////////////////////////////////////////
CBridgePartyAudioParams::CBridgePartyAudioParams ()
{
	m_dwAudioAlgorithm		=	Au_Neutral;
	m_mute_mask.ResetMask();
	m_dwVolume				=	AUDIO_VOLUME_MIN;
	m_byNumberOfChannels	=	0;
	m_byConfSampleRate		=	0;
	m_bIsRelaytoMix         =   FALSE;
	m_MsftClientType        =   MSFT_CLIENT_NONE_MSFT;
	m_maxAverageBitrate		=	0;
}

// ------------------------------------------------------------
CBridgePartyAudioParams::CBridgePartyAudioParams (DWORD dwAudioAlgorithm, /*BOOL isMuted, WORD muteSrcRequest,*/
												  DWORD dwVolume, BYTE byNumberOfChannels,
												  BYTE byConfSampleRate, BOOL isVideoParticipant, BOOL bIsRelayToMix, MSFT_CLIENT_ENUM eMsftClientType, DWORD maxAverageBitrate)
{
	m_dwAudioAlgorithm		=	dwAudioAlgorithm;
	m_mute_mask.ResetMask();
	m_dwVolume				=	dwVolume;
	m_byNumberOfChannels	=	byNumberOfChannels;
	m_byConfSampleRate		=	byConfSampleRate;
	m_isVideoParticipant	=	isVideoParticipant;
	m_bIsRelaytoMix         =   bIsRelayToMix;
	m_MsftClientType        =   eMsftClientType;
	m_maxAverageBitrate		=	maxAverageBitrate;
}

// ------------------------------------------------------------
CBridgePartyAudioParams::CBridgePartyAudioParams (const CBridgePartyAudioParams& rOtherBridgePartyAudioParams)
		:CBridgePartyMediaParams(rOtherBridgePartyAudioParams)
{
	m_dwAudioAlgorithm		=	rOtherBridgePartyAudioParams.m_dwAudioAlgorithm;
	m_mute_mask				= 	rOtherBridgePartyAudioParams.m_mute_mask;
	m_dwVolume				=	rOtherBridgePartyAudioParams.m_dwVolume;
	m_byNumberOfChannels	=	rOtherBridgePartyAudioParams.m_byNumberOfChannels;
	m_byConfSampleRate		=	rOtherBridgePartyAudioParams.m_byConfSampleRate;
	m_isVideoParticipant	=	rOtherBridgePartyAudioParams.m_isVideoParticipant;
	m_bIsRelaytoMix         =   rOtherBridgePartyAudioParams.m_bIsRelaytoMix;
	m_MsftClientType        =   rOtherBridgePartyAudioParams.m_MsftClientType;
	m_maxAverageBitrate		=	rOtherBridgePartyAudioParams.m_maxAverageBitrate;
}

// ------------------------------------------------------------
CBridgePartyAudioParams::~CBridgePartyAudioParams ()
{
}

// ------------------------------------------------------------
void CBridgePartyAudioParams::InitParams (DWORD dwAudioAlgorithm, /*BOOL isMuted, WORD muteSrcRequest,*/CDwordBitMask mute_mask,
												  DWORD dwVolume, BYTE byNumberOfChannels,
												  BYTE byConfSampleRate, BOOL isVideoParticipant, BOOL bIsRelayToMix,MSFT_CLIENT_ENUM eMsftClientType, DWORD maxAverageBitrate)
{
	m_dwAudioAlgorithm		=	dwAudioAlgorithm;
	m_mute_mask				= 	mute_mask;
	m_dwVolume				=	dwVolume;
	m_byNumberOfChannels	=	byNumberOfChannels;
	m_byConfSampleRate		=	byConfSampleRate;
	m_isVideoParticipant	=	isVideoParticipant;
	m_bIsRelaytoMix         =   bIsRelayToMix;
	m_MsftClientType        =   eMsftClientType;
	m_maxAverageBitrate		=	maxAverageBitrate;

//	TRACEINTO << " isRelayMix=" << (WORD)m_bIsRelaytoMix;
}

// ------------------------------------------------------------
CBridgePartyAudioParams& CBridgePartyAudioParams::operator = (const CBridgePartyAudioParams& rOtherBridgePartyAudioParams)
{
	if ( &rOtherBridgePartyAudioParams == this )
		return *this;

	CBridgePartyMediaParams::operator = (rOtherBridgePartyAudioParams);

	m_dwAudioAlgorithm		=	rOtherBridgePartyAudioParams.m_dwAudioAlgorithm;
	m_mute_mask				= 	rOtherBridgePartyAudioParams.m_mute_mask;
	m_dwVolume				=	rOtherBridgePartyAudioParams.m_dwVolume;
	m_byNumberOfChannels	=	rOtherBridgePartyAudioParams.m_byNumberOfChannels;
	m_byConfSampleRate		=	rOtherBridgePartyAudioParams.m_byConfSampleRate;
	m_isVideoParticipant	=	rOtherBridgePartyAudioParams.m_isVideoParticipant;
    m_bIsRelaytoMix         =   rOtherBridgePartyAudioParams.m_bIsRelaytoMix;
    m_MsftClientType        =   rOtherBridgePartyAudioParams.m_MsftClientType;
    m_maxAverageBitrate		=	rOtherBridgePartyAudioParams.m_maxAverageBitrate;

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CBridgePartyAudioParams::IsValidParams() const
{
	TRACECOND_AND_RETURN_VALUE(!CBridgePartyAudioParams::IsValidAudioAlgorithm(m_dwAudioAlgorithm), "Algorithm:" << m_dwAudioAlgorithm << " - Failed, invalid audio algorithm", FALSE);
	TRACECOND_AND_RETURN_VALUE(!::IsValidAudioVolume(m_dwVolume), "Volume:" << m_dwVolume << " - Failed, invalid audio volume", FALSE);
	TRACECOND_AND_RETURN_VALUE(!::IsValidAudioSampleRate(m_byConfSampleRate), "Rate:" << m_byConfSampleRate << " - Failed, invalid audio sample rate", FALSE);
	TRACECOND_AND_RETURN_VALUE(!::IsValidMsftClientType(m_MsftClientType), "Type:" << m_MsftClientType << " - Failed, invalid MSFT client type", FALSE);

	if ((AUDIO_MONO_NUM_OF_CHANNELS != m_byNumberOfChannels) && (AUDIO_STEREO_NUM_OF_CHANNELS != m_byNumberOfChannels)) // Only mono is supported now
		return FALSE;
	if ( IsOpusAudioAlgorithm(m_dwAudioAlgorithm) )
		if (FALSE == ::IsValidOpusCodecBitRate(m_maxAverageBitrate))
			return FALSE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CBridgePartyAudioParams::IsOpusAudioAlgorithm(DWORD dwAudioAlgorithm)
{
	if ((Au_Opus_64k == dwAudioAlgorithm) || (Au_OpusStereo_128k == dwAudioAlgorithm))
		return TRUE;
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CBridgePartyAudioParams::IsValidAudioAlgorithm(DWORD dwAudioAlgorithm)
{
	BOOL resultValue = FALSE;

	switch (dwAudioAlgorithm)
	{
		case A_Law_OU:
		case U_Law_OU:
		case A_Law_OF:
		case U_Law_OF:
		case G728:
		case G729_8k:
		case G722_m1:
		case A_Law_48:
		case U_Law_48:
		case G722_m2:
		case G722_m3:
		case Au_G722_Stereo_128:
		case Au_G7221_16k:
		case Au_24k:
		case Au_32k:
		case Au_Siren7_16k:
		case Au_Siren7_24k:
		case Au_Siren7_32k:
		case G723_1_Command:
		case Au_Siren14_24k:
		case Au_Siren14_32k:
		case Au_Siren14_48k:
		case G7221_AnnexC_24k:
		case G7221_AnnexC_32k:
		case G7221_AnnexC_48k:
		case G719_32k:
		case G719_48k:
		case G719_64k:
		case G719S_64k:
		case G719S_96k:
		case G719S_128k:
		case Au_Siren22_32k:
		case Au_Siren22_48k:
		case Au_Siren22_64k:
		case Au_Siren22S_64k:
		case Au_Siren22S_96k:
		case Au_Siren22S_128k:
		case Au_Siren14S_48k:
		case Au_Siren14S_56k:
		case Au_Siren14S_64k:
		case Au_Siren14S_96k:
		case Au_SirenLPR_32k:
		case Au_SirenLPR_48k:
		case Au_SirenLPR_64k:
		case Au_SirenLPRS_64k:
		case Au_SirenLPRS_96k:
		case Au_SirenLPRS_128k:
		case Au_AAC_LD:// TIP
		// Scalable audio
		case Au_SirenLPR_Scalable_32k:
		case Au_SirenLPR_Scalable_48k:
		case Au_SirenLPR_Scalable_64k:
		case Au_SirenLPRS_Scalable_64k:
		case Au_SirenLPRS_Scalable_96k:
		case Au_SirenLPRS_Scalable_128k:
		// iLBC
		case Au_iLBC_13k:
		case Au_iLBC_15k:
		// Opus
		case Au_Opus_64k:
		case Au_OpusStereo_128k:
		{
			resultValue = TRUE;
			break;
		}
	}

	return resultValue;
}


/////////////////////////////////////////////////////////////////////////////
BOOL CBridgePartyAudioParams::IsValidMediaTypeUpdate(EMediaTypeUpdate audioMediaTypeUpdate)
{
	BOOL resultValue = FALSE;

	switch(audioMediaTypeUpdate)
	{
		case eMediaTypeUpdateNotNeeded:
		case eMediaTypeUpdateAudioToVideo:
		case eMediaTypeUpdateVideoToAudio:

		{
			resultValue = TRUE;
			break;
		}
	}

	return resultValue;
}

// ------------------------------------------------------------
void CBridgePartyAudioParams::SetMute(EOnOff eOnOff,WORD srcRequest)
{
	if(eOff == eOnOff)
	{
		switch(srcRequest){

		case	OPERATOR:
			{
				m_mute_mask.ResetBit(OPERATOR);
				break;
			}
		case	PARTY:
			{
				m_mute_mask.ResetBit(PARTY);
				break;
			}
		case	MCMS:
			{
				m_mute_mask.ResetBit(MCMS);
				break;
			}
		case	ALL:
			{
				m_mute_mask.ResetBit(OPERATOR);
				m_mute_mask.ResetBit(PARTY);
				m_mute_mask.ResetBit(MCMS);
				break;
			}
		default:
			{
				TRACEINTO << "CBridgePartyAudioParams::SetMute - Illegal sourceRequest - "<< srcRequest;
			}
		}// end switch
	}
	else if(eOn == eOnOff)
	{
		switch(srcRequest){

		case	OPERATOR:
			{
				m_mute_mask.SetBit(OPERATOR);
				break;
			}
		case	PARTY:
			{
				m_mute_mask.SetBit(PARTY);
				break;
			}
		case	MCMS:
			{
				m_mute_mask.SetBit(MCMS);
				break;
			}
		case	ALL:
			{
				m_mute_mask.SetBit(OPERATOR);
				m_mute_mask.SetBit(PARTY);
				m_mute_mask.SetBit(MCMS);
				break;
			}
		default:
			{
				TRACEINTO << "CBridgePartyAudioParams::SetMute - Illegal sourceRequest - "<< srcRequest;
			}
		}// end switch
	}
}

// ------------------------------------------------------------
void CBridgePartyAudioParams::InitPartyAudioParams(const CBridgePartyAudioParams* pBridgePartyMediaParams)
{
	m_dwAudioAlgorithm		=	pBridgePartyMediaParams->m_dwAudioAlgorithm;
	m_mute_mask				= 	pBridgePartyMediaParams->m_mute_mask;
	m_dwVolume				=	pBridgePartyMediaParams->m_dwVolume;
	m_byNumberOfChannels	=	pBridgePartyMediaParams->m_byNumberOfChannels;
	m_byConfSampleRate		=	pBridgePartyMediaParams->m_byConfSampleRate;
	m_isVideoParticipant	=	pBridgePartyMediaParams->m_isVideoParticipant;
	m_bIsRelaytoMix         =   pBridgePartyMediaParams->m_bIsRelaytoMix;
	m_MsftClientType        =   pBridgePartyMediaParams->m_MsftClientType;
	m_maxAverageBitrate		= 	pBridgePartyMediaParams->m_maxAverageBitrate;

//	TRACEINTO << " isRelayMix=" << (WORD)m_bIsRelaytoMix;
}

/////////////////////////////////////////////////////////////////////////////
// CLASS CBridgePartyAudioInParams
/////////////////////////////////////////////////////////////////////////////
CBridgePartyAudioInParams::CBridgePartyAudioInParams () : CBridgePartyAudioParams()
{
	m_isAGC					=	FALSE;
	m_isVtxSupport			=	FALSE;
	m_byCallDirection		=	0xFF;
	m_isEchoSuppression     =   FALSE;
	m_isKeyboardSuppression =   FALSE;
	m_isAutoMuteNoisyParties =  FALSE;
	m_bIsSupportLegacyToSacTranslate = FALSE;
	m_isVideoParticipant	=	FALSE;
}

// ------------------------------------------------------------
CBridgePartyAudioInParams::CBridgePartyAudioInParams (DWORD dwAudioAlgorithm,
													  DWORD dwVolume, BYTE byNumberOfChannels, BYTE byConfSampleRate,
													  BOOL isAGC, BOOL isVideoParticipant,
													  BOOL isVtxSupport, BYTE callDirection, DWORD audioCompressedDelay,
													  BOOL isEchoSuppression,BOOL isKeyboardSuppression, BOOL isAutoMuteNoisyParties,
													  BOOL isAudioClarity, DWORD ssrc/*=0*/,
													  WORD confSpeakerChangeMode, BOOL bIsSupportLegacyToSacTranslate, BOOL bIsRelayToMix, MSFT_CLIENT_ENUM eMsftClientType) :
	CBridgePartyAudioParams(dwAudioAlgorithm, dwVolume, byNumberOfChannels, byConfSampleRate, isVideoParticipant, bIsRelayToMix,eMsftClientType)
{
	m_isAGC					=	isAGC;
	m_isVtxSupport			=	isVtxSupport;
	m_byCallDirection		=	callDirection;
	m_audioCompressedDelay  =   audioCompressedDelay;
	m_isEchoSuppression 	=   isEchoSuppression;
	m_isKeyboardSuppression =   isKeyboardSuppression;
	m_isAutoMuteNoisyParties =  isAutoMuteNoisyParties;
	m_isAudioClarity        =   isAudioClarity;
	m_ssrc					=	ssrc;
	m_confSpeakerChangeMode =   confSpeakerChangeMode;
	m_bIsSupportLegacyToSacTranslate = bIsSupportLegacyToSacTranslate;
}

// ------------------------------------------------------------
CBridgePartyAudioInParams::CBridgePartyAudioInParams (const CBridgePartyAudioInParams& rOtherBridgePartyAudioInParams)
		:CBridgePartyAudioParams(rOtherBridgePartyAudioInParams)
{
	m_isAGC					=	rOtherBridgePartyAudioInParams.m_isAGC;
	m_isVtxSupport			=	rOtherBridgePartyAudioInParams.m_isVtxSupport;
	m_byCallDirection		=	rOtherBridgePartyAudioInParams.m_byCallDirection;
	m_audioCompressedDelay  =   rOtherBridgePartyAudioInParams.m_audioCompressedDelay;
	m_isEchoSuppression     =   rOtherBridgePartyAudioInParams.m_isEchoSuppression;
	m_isKeyboardSuppression =   rOtherBridgePartyAudioInParams.m_isKeyboardSuppression;
	m_isAutoMuteNoisyParties =  rOtherBridgePartyAudioInParams.m_isAutoMuteNoisyParties;
	m_isAudioClarity        =   rOtherBridgePartyAudioInParams.m_isAudioClarity;
	m_ssrc					=	rOtherBridgePartyAudioInParams.m_ssrc;
	m_confSpeakerChangeMode =   rOtherBridgePartyAudioInParams.m_confSpeakerChangeMode;
	m_bIsSupportLegacyToSacTranslate = rOtherBridgePartyAudioInParams.m_bIsSupportLegacyToSacTranslate;
}

// ------------------------------------------------------------
CBridgePartyAudioInParams::~CBridgePartyAudioInParams ()
{
}

// ------------------------------------------------------------
CBridgePartyAudioInParams& CBridgePartyAudioInParams::operator = (const CBridgePartyAudioInParams& rOtherBridgePartyAudioInParams)
{
	if ( &rOtherBridgePartyAudioInParams == this )
		return *this;

	CBridgePartyAudioParams::operator = (rOtherBridgePartyAudioInParams);

	m_isAGC					=	rOtherBridgePartyAudioInParams.m_isAGC;
	m_isVtxSupport			=	rOtherBridgePartyAudioInParams.m_isVtxSupport;
	m_byCallDirection		=	rOtherBridgePartyAudioInParams.m_byCallDirection;
	m_audioCompressedDelay  =   rOtherBridgePartyAudioInParams.m_audioCompressedDelay;
	m_isEchoSuppression     =   rOtherBridgePartyAudioInParams.m_isEchoSuppression;
	m_isKeyboardSuppression =   rOtherBridgePartyAudioInParams.m_isKeyboardSuppression;
	m_isAutoMuteNoisyParties =  rOtherBridgePartyAudioInParams.m_isAutoMuteNoisyParties;
	m_isAudioClarity        =   rOtherBridgePartyAudioInParams.m_isAudioClarity;
	m_ssrc					=	rOtherBridgePartyAudioInParams.m_ssrc;
    m_confSpeakerChangeMode =   rOtherBridgePartyAudioInParams.m_confSpeakerChangeMode;
    m_bIsSupportLegacyToSacTranslate = rOtherBridgePartyAudioInParams.m_bIsSupportLegacyToSacTranslate;

	return *this;
}

// ------------------------------------------------------------
BOOL CBridgePartyAudioInParams::IsValidParams()  const
{

	if ( FALSE == CBridgePartyAudioParams::IsValidParams() )
		return FALSE;

	if ( (DIALIN != m_byCallDirection) && (DIALOUT != m_byCallDirection) )
		return FALSE;

	return TRUE;
}

// ------------------------------------------------------------
void CBridgePartyAudioInParams::InitParams  (DWORD dwAudioAlgorithm, CDwordBitMask mute_mask,
											 DWORD dwVolume, BYTE byNumberOfChannels, BYTE byConfSampleRate,
											 BOOL isVideoParticipant, BOOL isAGC,
											 BOOL isVtxSupport, BYTE callDirection, DWORD audioCompressedDelay,
											 BOOL isEchoSuppression,BOOL isKeyboardSuppression, BOOL isAutoMuteNoisyParties, BOOL isAudioClarity,
											 DWORD ssrc/*=0*/, WORD confSpeakerChangeMode, BOOL bIsSupportLegacyToSacTranslate, BOOL bIsRelayToMix ,MSFT_CLIENT_ENUM eMsftClientType, DWORD maxAverageBitrate)
{
	CBridgePartyAudioParams::InitParams(dwAudioAlgorithm, mute_mask, dwVolume, byNumberOfChannels, byConfSampleRate, isVideoParticipant, bIsRelayToMix,eMsftClientType, maxAverageBitrate);

	m_isAGC					=	isAGC;
	m_isVtxSupport			=	isVtxSupport;
	m_byCallDirection		=	callDirection;
	m_audioCompressedDelay  =   audioCompressedDelay;
	m_isEchoSuppression     =   isEchoSuppression;
	m_isKeyboardSuppression =   isKeyboardSuppression;
	m_isAutoMuteNoisyParties =  isAutoMuteNoisyParties;
	m_isAudioClarity        =	isAudioClarity;
	m_ssrc					= 	ssrc;
	m_confSpeakerChangeMode   = confSpeakerChangeMode;
	m_bIsSupportLegacyToSacTranslate = bIsSupportLegacyToSacTranslate;
	m_isVideoParticipant    =   isVideoParticipant;
}

// ------------------------------------------------------------
void CBridgePartyAudioInParams::InitPartyAudioInParams(const CBridgePartyAudioParams* pBridgePartyMediaParams)
{
	InitPartyAudioParams(pBridgePartyMediaParams);

	m_isAGC					=	((CBridgePartyAudioInParams*)pBridgePartyMediaParams)->m_isAGC;
	m_isVtxSupport			=	((CBridgePartyAudioInParams*)pBridgePartyMediaParams)->m_isVtxSupport;
	m_byCallDirection		=	((CBridgePartyAudioInParams*)pBridgePartyMediaParams)->m_byCallDirection;
	m_audioCompressedDelay  =   ((CBridgePartyAudioInParams*)pBridgePartyMediaParams)->m_audioCompressedDelay;
	m_isEchoSuppression     =   ((CBridgePartyAudioInParams*)pBridgePartyMediaParams)->m_isEchoSuppression;
	m_isKeyboardSuppression =   ((CBridgePartyAudioInParams*)pBridgePartyMediaParams)->m_isKeyboardSuppression;
	m_isAutoMuteNoisyParties =  ((CBridgePartyAudioInParams*)pBridgePartyMediaParams)->m_isAutoMuteNoisyParties;
	m_isAudioClarity        =   ((CBridgePartyAudioInParams*)pBridgePartyMediaParams)->m_isAudioClarity;
	m_ssrc					=   ((CBridgePartyAudioInParams*)pBridgePartyMediaParams)->m_ssrc;
	m_confSpeakerChangeMode =   ((CBridgePartyAudioInParams*)pBridgePartyMediaParams)->m_confSpeakerChangeMode;
	m_bIsSupportLegacyToSacTranslate = ((CBridgePartyAudioInParams*)pBridgePartyMediaParams)->m_bIsSupportLegacyToSacTranslate;
}

/////////////////////////////////////////////////////////////////////////////
// CLASS CBridgePartyAudioOutParams
/////////////////////////////////////////////////////////////////////////////
CBridgePartyAudioOutParams::CBridgePartyAudioOutParams ()
{
	for (int i=0; i<MAX_NUM_OF_SSRCS; i++)
	{
		m_ssrc[i] = 0;
	};
	m_ivrSsrc = 0;


	m_bUseSpeakerSsrcForTx	=	FALSE;
	m_dwSeenImageSsrc = INVALID;
}

// ------------------------------------------------------------
CBridgePartyAudioOutParams::CBridgePartyAudioOutParams ( DWORD dwAudioAlgorithm,
														 DWORD dwVolume, BYTE byNumberOfChannels,
														 BYTE byConfSampleRate, BOOL isVideoParticipant,
														 DWORD* ssrc_array/*=NULL*/, DWORD ivrSsrc /*= 0*/,
														 BOOL isUseSpeakerSsrcForTx/*=FALSE*/, BOOL bIsRelayToMix /*= FALSE*/ , MSFT_CLIENT_ENUM eMsftClientType) :
	CBridgePartyAudioParams(dwAudioAlgorithm, dwVolume, byNumberOfChannels, byConfSampleRate, isVideoParticipant, bIsRelayToMix,eMsftClientType)
{
	if (NULL == ssrc_array)
	{
		for (int i=0; i<MAX_NUM_OF_SSRCS; i++)
		{
			m_ssrc[i] = 0;
		}
	}
	else
	{
		for (int i=0; i<MAX_NUM_OF_SSRCS; i++)
		{
			m_ssrc[i] = ssrc_array[i];
		}
	}

	m_ivrSsrc = ivrSsrc;
	m_bUseSpeakerSsrcForTx = isUseSpeakerSsrcForTx;
}

// ------------------------------------------------------------
CBridgePartyAudioOutParams::CBridgePartyAudioOutParams (const CBridgePartyAudioOutParams& rOtherBridgePartyAudioOutParams)
		:CBridgePartyAudioParams(rOtherBridgePartyAudioOutParams)
{
	for (int i=0; i<MAX_NUM_OF_SSRCS; i++)
	{
		m_ssrc[i] = rOtherBridgePartyAudioOutParams.m_ssrc[i];
	}

	m_ivrSsrc = rOtherBridgePartyAudioOutParams.m_ivrSsrc;
	m_bUseSpeakerSsrcForTx = rOtherBridgePartyAudioOutParams.m_bUseSpeakerSsrcForTx;
	m_dwSeenImageSsrc =rOtherBridgePartyAudioOutParams.m_dwSeenImageSsrc;
}

// ------------------------------------------------------------
CBridgePartyAudioOutParams::~CBridgePartyAudioOutParams ()
{
}

// ------------------------------------------------------------
CBridgePartyAudioOutParams& CBridgePartyAudioOutParams::operator = (const CBridgePartyAudioOutParams& rOtherBridgePartyAudioOutParams)
{
	if ( &rOtherBridgePartyAudioOutParams == this )
		return *this;

	CBridgePartyAudioParams::operator = (rOtherBridgePartyAudioOutParams);

	for (int i=0; i<MAX_NUM_OF_SSRCS; i++)
	{
		m_ssrc[i] = rOtherBridgePartyAudioOutParams.m_ssrc[i];
	}

	m_ivrSsrc = rOtherBridgePartyAudioOutParams.m_ivrSsrc;
	m_bUseSpeakerSsrcForTx = rOtherBridgePartyAudioOutParams.m_bUseSpeakerSsrcForTx;
	m_dwSeenImageSsrc = rOtherBridgePartyAudioOutParams.m_dwSeenImageSsrc;

	return *this;
}
// ------------------------------------------------------------
BOOL CBridgePartyAudioOutParams::IsValidParams() const
{
	return CBridgePartyAudioParams::IsValidParams();
}

// ------------------------------------------------------------
void CBridgePartyAudioOutParams::InitParams( DWORD dwAudioAlgorithm, CDwordBitMask mute_mask,
						 	 	 	 	 	 DWORD dwVolume, BYTE byNumberOfChannels, BYTE byConfSampleRate,
						 	 	 	 	 	 BOOL isVideoParticipant, DWORD aNumOfSsrcIds,
						 	 	 	 	 	 DWORD* ssrc_array/*=NULL*/, DWORD ivrSsrc/* = 0*/,
						 	 	 	 	 	 BOOL isUseSpeakerSsrcForTx/*=FALSE*/, BOOL bIsRelayToMix /*= FALSE*/,MSFT_CLIENT_ENUM eMsftClientType, DWORD maxAverageBitrate)
{
	CBridgePartyAudioParams::InitParams(dwAudioAlgorithm, mute_mask, dwVolume, byNumberOfChannels, byConfSampleRate, isVideoParticipant, bIsRelayToMix, eMsftClientType, maxAverageBitrate);

	m_numOfSsrcIds = aNumOfSsrcIds;

	for (int i=0; i<MAX_NUM_OF_SSRCS; i++)
	{
		m_ssrc[i] = 0;
	}

	for (DWORD i=0; i<MAX_NUM_OF_SSRCS && i<m_numOfSsrcIds; i++)
	{
		if (ssrc_array[i])
		{
			m_ssrc[i] = ssrc_array[i];
		}
	}

	m_ivrSsrc = ivrSsrc;
	m_bUseSpeakerSsrcForTx = isUseSpeakerSsrcForTx;

}

// ------------------------------------------------------------
void CBridgePartyAudioOutParams::InitPartyAudioOutParams(const CBridgePartyAudioParams* pBridgePartyMediaParams)
{
	InitPartyAudioParams(pBridgePartyMediaParams);

	for (int i=0; i<MAX_NUM_OF_SSRCS; i++)
	{
		m_ssrc[i] = ((CBridgePartyAudioOutParams*)pBridgePartyMediaParams)->m_ssrc[i];
	}

	m_ivrSsrc = ((CBridgePartyAudioOutParams*)pBridgePartyMediaParams)->m_ivrSsrc;
	m_bUseSpeakerSsrcForTx = ((CBridgePartyAudioOutParams*)pBridgePartyMediaParams)->m_bUseSpeakerSsrcForTx;
	m_dwSeenImageSsrc = ((CBridgePartyAudioOutParams*)pBridgePartyMediaParams)->m_dwSeenImageSsrc;
}

// ------------------------------------------------------------
DWORD CBridgePartyAudioOutParams::GetSsrc(const int idx) const
{
	if ( (0 <= idx) && (MAX_NUM_OF_SSRCS > idx) )
	{
		return m_ssrc[idx];
	}

	return 0;
}

// ------------------------------------------------------------
void CBridgePartyAudioOutParams::SetSsrc(const int idx, DWORD ssrc)
{
	if ( (0 <= idx) && (MAX_NUM_OF_SSRCS > idx) )
	{
		m_ssrc[idx] = ssrc;
	}
}
