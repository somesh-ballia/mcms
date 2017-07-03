//+========================================================================+
//                   AudioHardwareInterface.H                              |
//					 Copyright 2005 Polycom, Inc                           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// FILE:       AudioHardwareInterface.H                                    |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Matvey                                                      |
//-------------------------------------------------------------------------|
// Who  | Date  July-2005  | Description                                   |
//-------------------------------------------------------------------------|

#ifndef _AUDIO_INTERAFCE_
#define _AUDIO_INTERAFCE_

#include "HardwareInterface.h"
#include "AudPortDefinitions.h"
#include "AudHostApiDefinitions.h"
#include "HostCommonDefinitions.h"
#include "AudRequestStructs.h"
#include "ProcessBase.h"
#include "IpRtpReq.h"

class CAudioHardwareInterface : public CHardwareInterface
{
	CLASS_TYPE_1(CAudioHardwareInterface, CHardwareInterface)

	virtual const char* NameOf() const { return "CAudioHardwareInterface"; }

public:

	CAudioHardwareInterface(ConnectionID ConnectionId = DUMMY_CONNECTION_ID,
							PartyRsrcID ParId = DUMMY_PARTY_ID,
							ConfRsrcID ConfId = DUMMY_CONF_ID,
							eLogicalResourceTypes LRT = eLogical_res_none);

	virtual ~CAudioHardwareInterface();

	virtual DWORD OpenDecoder(
							WORD wNetworkInterface,
							BYTE byConfSampleRate, BYTE byNumberOfChannels,
							DWORD dwAudioAlgorithm, BOOL isMuted, DWORD dwVolume,
							BOOL isErrorConcealment, BOOL isAGC, BOOL isToneRemove,
							BOOL isNoiseReduction, BOOL isT1CptDetection,
							BOOL isDtmfDetection, BOOL isNoiseDetection, BYTE byNoiseDetectionThreshold,
							BOOL isVideoParticipant, BOOL isVtxSupport,
							BOOL isEchoSuppression, BYTE byCallDirection, BOOL isStandalone, DWORD audioDecoderCompressedDelay,
							BOOL isKeyboardSuppression, BOOL isAutoMuteNoisyParties, BOOL isAudioClarity,
							DWORD wVolumeAdjustment,
							DWORD ssrc, WORD confSpeakerChangeMode, BOOL bIsCallGenerator, BOOL isRelayToMix,MSFT_CLIENT_ENUM eMsftClientType,
							DWORD maxAverageBitrate);

	virtual DWORD OpenEncoder(
							WORD wNetworkInterface, BYTE byConfSampleRate,
							BYTE byNumberOfChannels, DWORD dwAudioAlgorithm,
							BOOL isMuted, DWORD dwVolume,
							BOOL isVideoParticipant, BOOL isStandalone,
							DWORD wVolumeAdjustment,
							DWORD numOfSsrcIds, DWORD* ssrc_array,
							EMixModeGet eMixModeSet, DWORD ivrSsrc,
							BOOL isUseSpeakerSsrcForTx, BOOL bIsCallGenerator, BOOL isRelayToMix, DWORD maxAverageBitrate);

	virtual DWORD CloseDecoder();
	virtual DWORD UpdateDecoder(BOOL isVideoParticipant);
	virtual DWORD CloseEncoder();

	virtual void UpdateAlgorithm(DWORD dwNewAudioAlgorithm, DWORD maxAverageBitrate/*for Opus*/);
	virtual void UpdateMute(BOOL isMuted);
	virtual void UpdateVolume(DWORD dwNewVolume);
	virtual void UpdateUseSpeakerSsrcForTx(BOOL bUseSpeakerSsrc);
	virtual void UpdateNoiseDetection(BOOL isNoiseDetection, BYTE NoiseDetectionThreshold);
	virtual void UpdateStandalone(BYTE isStandalone);
	virtual void UpdateAGC(BOOL isAGC);
	virtual void CGPlayAudioReq(DWORD dwNewVolume);

	virtual void OpenConf(WORD wTalkHoldTime, BYTE byAudioMixDepth, BOOL isAutoMuteNoisyParties, WORD bReOpenConf = FALSE,WORD new_card_board_id = (WORD)-1, WORD activeSpeakerPreference = 0);
	virtual void UpdateConf(WORD wTalkHoldTime, BYTE byAudioMixDepth);
	virtual void CloseConf();
	virtual void ACLayoutChangeComplete();

	virtual bool UpdateAudioDelay(TAudioUpdateCompressedAudioDelayReq* pstAudioDelay);
	virtual bool UpdateAudioRelayParamsIn(TRtpUpdateRelayReq* pstRelayParamsIn);
	virtual bool UpdateAudioRelayParamsOut(TRtpUpdateRelayReq* pstRelayParamsOut);
	virtual DWORD SendAudioEncoderUpdateImageSeenSsrc(DWORD ssrc);

	DWORD PlayMessage(CSegment *pDataSeg);
	DWORD StopPlayMessage(CSegment *pDataSeg);
	DWORD StartIVR(CSegment *pDataSeg);
	DWORD StopIVR(CSegment *pDataSeg);
	DWORD PlayMusic(CSegment *pDataSeg);
	DWORD StopPlayMusic(CSegment *pDataSeg);
	DWORD PlayTone(CSegment* pParam);
	DWORD RecordRollCall(CSegment *pDataSeg);
	DWORD StopRecordRollCall(CSegment *pDataSeg);
	WORD  FillPlayToneParams( const char * toneBuffer, WORD numTones, SPlayToneStruct* tPlayToneStruct);
	unsigned long  GetToneId( char ch);
	WORD CheckValidityOfToneBufAndNum(const char *toneBuffer, WORD numTones)	;

	static BOOL TranslateMcmsAudioAlgorithmOpcodeToApiValues(DWORD dwMcmsAudioAlgorithmOpcode, EAudioAlgorithm &eApiAudioAlgorithm);
	static BOOL TranslateMcmsVolumeToApiValues(DWORD dwMcmsVolume, EAudioGainPreset &eApiAudioVolume);
	static BOOL TranslateMcmsSampleRateToApiValues(BYTE byMcmsSampleRate, ESampleRate &eSampleRate);

private:
	CAudioHardwareInterface(const CAudioHardwareInterface &);
	CAudioHardwareInterface & operator =(const CAudioHardwareInterface &);
};


#endif
