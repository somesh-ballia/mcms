//+========================================================================+
//                   BridgePartyAudioParams.H                              |
//					 Copyright 2005 Polycom, Inc                           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// FILE:       BridgePartyAudioParams.H                                    |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Matvey                                                      |
//-------------------------------------------------------------------------|
// Who  | Date  June-2005  | Description                                   |
//-------------------------------------------------------------------------|
//			
//+========================================================================+

#ifndef _CBridgePartyAudioParams_H_
#define _CBridgePartyAudioParams_H_

#include "BridgePartyMediaParams.h"
#include "DwordBitMask.h"
#include "ConfPartyDefines.h"
#include "IpRtpReq.h"


#define MAX_NUM_OF_SSRCS	5

/////////////////////////////////////////////////////////////////////////////
// CLASS CBridgePartyAudioParams
/////////////////////////////////////////////////////////////////////////////
class CBridgePartyAudioParams : public CBridgePartyMediaParams {
CLASS_TYPE_1(CBridgePartyAudioParams,CBridgePartyMediaParams) 
public:
	CBridgePartyAudioParams ();
	CBridgePartyAudioParams (DWORD dwAudioAlgorithm,
							 DWORD dwVolume, BYTE byNumberOfChannels, BYTE byConfSampleRate, 
							 BOOL isVideoParticipant, BOOL bIsRelayToMix, MSFT_CLIENT_ENUM eMsftClientType, DWORD maxAverageBitrate = 0);
	CBridgePartyAudioParams (const CBridgePartyAudioParams& rOtherBridgePartyAudioParams);
	virtual ~CBridgePartyAudioParams ();
	virtual void InitParams(DWORD dwAudioAlgorithm, CDwordBitMask mute_mask,
							 DWORD dwVolume, BYTE byNumberOfChannels, BYTE byConfSampleRate, 
							 BOOL isVideoParticipant, BOOL bIsRelayToMix,MSFT_CLIENT_ENUM eMsftClientType, DWORD maxAverageBitrate);
	CBridgePartyAudioParams& operator = (const CBridgePartyAudioParams& rOtherBridgePartyAudioParams);
	virtual const char* NameOf() const { return "CBridgePartyAudioParams";}

	virtual BOOL	IsValidParams() const;
	static  BOOL	IsValidAudioAlgorithm(DWORD dwAudioAlgorithm);
	static  BOOL    IsValidMediaTypeUpdate(EMediaTypeUpdate audioMediaTypeUpdate);
	static	BOOL 	IsOpusAudioAlgorithm(DWORD dwAudioAlgorithm);


	DWORD	         GetAudioAlgorithm () const { return m_dwAudioAlgorithm;}
	DWORD	         GetAudioVolume () const { return m_dwVolume;}
	BYTE	         GetNumberOfChannels () const { return m_byNumberOfChannels;}
	BYTE	         GetConfSampleRate () const { return m_byConfSampleRate;}
	BOOL	         IsVideoParticipant () const { return m_isVideoParticipant;}
	CDwordBitMask    GetMuteMask	  () const { return m_mute_mask;}
	BOOL             GetIsRelaytoMix() const { return m_bIsRelaytoMix; }
	MSFT_CLIENT_ENUM GetMsftClientType() const {return m_MsftClientType; }
	DWORD			 GetMaxAverageBitrate() const {return m_maxAverageBitrate;}

	
	void	SetAudioAlgorithm (DWORD audioAlgorithm) {m_dwAudioAlgorithm = audioAlgorithm;}
	void	SetMute(EOnOff eOnOff,WORD srcRequest);
	void	SetMuteMask(CDwordBitMask mute_mask) {m_mute_mask = mute_mask;}
	void	SetVolume (WORD volume) {m_dwVolume = volume;}
	void    SetNumberOfChannels(BYTE NumOfChannels){m_byNumberOfChannels = NumOfChannels;}
	void 	SetConfSampleRate(BYTE ConfSampleRate){m_byConfSampleRate = ConfSampleRate;}
	void    SetVideoParticipant(BOOL isVideoParty){m_isVideoParticipant = isVideoParty;}
	void    SetIsRelaytoMix(BOOL isRelayToMix) { m_bIsRelaytoMix = isRelayToMix; }
	void    SetMsftClientType(MSFT_CLIENT_ENUM msftClientType) { m_MsftClientType = msftClientType;}
	void	SetMaxAverageBitrate(DWORD maxAverageBitrate) {m_maxAverageBitrate = maxAverageBitrate;}

	void    InitPartyAudioParams(const CBridgePartyAudioParams* pBridgePartyMediaParams);
protected:

	DWORD	          m_dwAudioAlgorithm;		// Valid values: H221 A1 table values for audio commands
	CDwordBitMask 	  m_mute_mask;
	DWORD	m_dwVolume;				// Valid values: from AUDIO_VOLUME_MIN to AUDIO_VOLUME_MAX
	BYTE	m_byNumberOfChannels;	// Valid values: (AUDIO_MONO_NUM_OF_CHANNELS, AUDIO_STEREO_NUM_OF_CHANNELS) 
	BYTE	m_byConfSampleRate;		// Valid values: (AUDIO_SAMPLE_RATE_8KHZ, AUDIO_SAMPLE_RATE_16KHZ, AUDIO_SAMPLE_RATE_32KHZ,AUDIO_SAMPLE_RATE_48KHZ)
	BOOL	m_isVideoParticipant;	// Valid values: (TRUE, FALSE)

	BOOL    m_bIsRelaytoMix;  //was added for the SVC Cascade MFW feature.
	MSFT_CLIENT_ENUM  m_MsftClientType ; // Valid values: (MSFT_CLIENT_DUMMY =0, MSFT_CLIENT_MOC,  MSFT_CLIENT_LYNC, MSFT_CLIENT_AVMCU, MSFT_CLIENT_LYNC2013, MSFT_CLIENT_AVMCU2013,MSFT_CLIENT_NONE_MSFT,MSFT_CLIENT_LAST)

	DWORD	m_maxAverageBitrate;
};

/////////////////////////////////////////////////////////////////////////////
// CLASS CBridgePartyAudioInParams
/////////////////////////////////////////////////////////////////////////////
class CBridgePartyAudioInParams : public CBridgePartyAudioParams {
CLASS_TYPE_1(CBridgePartyAudioInParams,CBridgePartyAudioParams) 
public:
	CBridgePartyAudioInParams ();
	CBridgePartyAudioInParams (DWORD dwAudioAlgorithm,
							   DWORD dwVolume,			BYTE byNumberOfChannels, BYTE byConfSampleRate,
							   /*BOOL isErrorConcealment --pSysCfg??*/ BOOL isAGC/*+*/,
							   //BOOL isToneRemove/*--MGC default=YES*/,		BOOL isNoiseReduction/*--MGC default=YES*/,
							   //BOOL isT1CptDetection/*--MGC default=0*/,	BOOL isDtmfDetection/*--???*/,
							   /*BOOL isNoiseDetection --pSysCfg??*/	BOOL isVideoParticipant/*+*/,
							   BOOL isVtxSupport/*+*/,		//BOOL isEchoCancel /*--*/
							   BYTE byCallDirection/*+*/, DWORD audioCompressedDelay,
							   BOOL isEchoSuppression,BOOL isKeyboardSuppression,BOOL isAutoMuteNoisyParties,BOOL isAudioClarity,
							   DWORD ssrc=0, WORD confSpeakerChangeMode=0, BOOL bIsSupportLegacyToSacTranslate = FALSE,
							   BOOL bIsRelayToMix = FALSE,
							   MSFT_CLIENT_ENUM eMsftClientType = MSFT_CLIENT_NONE_MSFT);
	CBridgePartyAudioInParams (const CBridgePartyAudioInParams& rOtherBridgePartyAudioInParams);
	virtual ~CBridgePartyAudioInParams ();

	CBridgePartyAudioInParams& operator = (const CBridgePartyAudioInParams& rOtherBridgePartyAudioInParams);
	virtual const char* NameOf() const { return "CBridgePartyAudioInParams";}
	virtual BOOL IsValidParams() const;

	virtual void InitParams (DWORD dwAudioAlgorithm, CDwordBitMask mute_mask,
							 DWORD dwVolume, BYTE byNumberOfChannels, BYTE byConfSampleRate, 
							 BOOL isVideoParticipant, BOOL isAGC,
							 BOOL isVtxSupport, BYTE callDirection, DWORD audioCompressedDelay,
							 BOOL isEchoSuppression,BOOL isKeyboardSuppression,BOOL isAutoMuteNoisyParties,BOOL isAudioClarity,
							 DWORD ssrc=0, WORD confSpeakerChangeMode = 0, BOOL bIsSupportLegacyToSacTranslate = FALSE,
							 BOOL bIsRelayToMix = FALSE,MSFT_CLIENT_ENUM eMsftClientType = MSFT_CLIENT_NONE_MSFT, DWORD maxAverageBitrate = 0);
							
	void    InitPartyAudioInParams(const CBridgePartyAudioParams* pBridgePartyMediaParams);


	BOOL	IsAGC () const { return m_isAGC;}
	BOOL	IsVtxSupport () const { return m_isVtxSupport;}
	BYTE	GetCallDirection () const { return m_byCallDirection;}
    DWORD   GetAudioCompressedDelay() const {return m_audioCompressedDelay;}
	BOOL	GetEchoSuppression () const { return m_isEchoSuppression;}
	BOOL	GetKeyboardSuppression () const { return m_isKeyboardSuppression;}
	BOOL	GetAutoMuteNoisyParties () const { return m_isAutoMuteNoisyParties;}
	BOOL    GetIsAudioClarity() const {return m_isAudioClarity;}
    DWORD   GetSsrc() const {return m_ssrc;}
	WORD    GetConfSpeakerChangeMode() const {return m_confSpeakerChangeMode;}
	BOOL    GetIsSupportLegacyToSacTranslate() const {return m_bIsSupportLegacyToSacTranslate;}

	void	SetAGC(BOOL isAGC){ m_isAGC = isAGC; }
	void    SetAudioCompressedDelay(DWORD audioCompressedDelay){m_audioCompressedDelay =  audioCompressedDelay;}
	void	SetEchoSuppression(BOOL isEchoSuppression){ m_isEchoSuppression = isEchoSuppression; }
	void	SetKeyboardSuppression(BOOL isKeyboardSuppression){ m_isKeyboardSuppression = isKeyboardSuppression; }
	void	SetAutoMuteNoisyParties(BOOL isAutoMuteNoisyParties){ m_isAutoMuteNoisyParties = isAutoMuteNoisyParties; }
    void    SetIsAudioClarity(BOOL isAudioClarity){ m_isAudioClarity = isAudioClarity; }
	void    SetSsrc(DWORD ssrc){m_ssrc = ssrc;}
	void    SetIsSupportLegacyToSacTranslate(BOOL isSupportLegacyToSacTranslate){m_bIsSupportLegacyToSacTranslate = isSupportLegacyToSacTranslate;}

	void    SetConfSpeakerChangeMode(WORD confSpeakerChangeMode){ m_confSpeakerChangeMode = confSpeakerChangeMode; }


protected:

	// MOTI: TBD final members list
	//BOOL	m_isErrorConcealment;
	BOOL	m_isAGC;
	//BOOL	m_isToneRemove;
	//BOOL	m_isNoiseReduction;
	//BOOL	m_isT1CptDetection;
	//BOOL	m_isDtmfDetection;
//	BOOL	m_isNoiseDetection;
	BOOL	m_isVtxSupport;
	BOOL	m_isEchoSuppression;
	BOOL	m_isKeyboardSuppression;
	BOOL	m_isAutoMuteNoisyParties;
	BYTE	m_byCallDirection; // (DIALIN / DIALOUT)
//	BYTE	m_byNoiseDetectionThreshold; // 1) syscfg???; 
														  // 2) temp default (NIR) = E_NOISE_DETECTION_THRESHOLD_5
														  // 3) AB type - BYTE ?????
	DWORD	m_audioCompressedDelay; // valid value - 0-3000 msec  // PCI bug patch (to be removed in V3.x)
	BOOL    m_isAudioClarity;
	DWORD	m_ssrc;
	WORD    m_confSpeakerChangeMode;
	BOOL    m_bIsSupportLegacyToSacTranslate;
};

/////////////////////////////////////////////////////////////////////////////
// CLASS CBridgePartyAudioParams
/////////////////////////////////////////////////////////////////////////////
class CBridgePartyAudioOutParams : public CBridgePartyAudioParams {
CLASS_TYPE_1(CBridgePartyAudioOutParams,CBridgePartyAudioParams) 
public:
	CBridgePartyAudioOutParams ();
	CBridgePartyAudioOutParams ( DWORD dwAudioAlgorithm,
								 DWORD dwVolume, BYTE byNumberOfChannels, BYTE byConfSampleRate,
								 BOOL isVideoParticipant, DWORD* ssrc_array=NULL,
								 DWORD ivrSsrc = 0, BOOL isUseSpeakerSsrcForTx=FALSE, BOOL bIsRelayToMix = FALSE,
								 MSFT_CLIENT_ENUM eMsftClientType = MSFT_CLIENT_NONE_MSFT );
	CBridgePartyAudioOutParams (const CBridgePartyAudioOutParams& rOtherBridgePartyAudioOutParams);
	virtual ~CBridgePartyAudioOutParams ();
	
	CBridgePartyAudioOutParams& operator = (const CBridgePartyAudioOutParams& rOtherBridgePartyAudioOutParams);
	virtual const char* NameOf() const { return "CBridgePartyAudioOutParams";}

	virtual BOOL	IsValidParams() const;
	
	virtual void InitParams( DWORD dwAudioAlgorithm, CDwordBitMask mute_mask,
							 DWORD dwVolume, BYTE byNumberOfChannels, BYTE byConfSampleRate,
							 BOOL isVideoParticipant, DWORD numOfSsrcIds=0,
							 DWORD* ssrc_array=NULL, DWORD ivrSsrc = 0, BOOL isUseSpeakerSsrcForTx=FALSE, BOOL bIsRelayToMix = FALSE,
							 MSFT_CLIENT_ENUM eMsftClientType = MSFT_CLIENT_NONE_MSFT, DWORD maxAverageBitrate = 0);

	void InitPartyAudioOutParams(const CBridgePartyAudioParams* pBridgePartyMediaParams);

    DWORD   GetSsrcNum() const { return m_numOfSsrcIds; }
    DWORD   GetSsrc(const int idx) const;
	void    SetSsrc(const int idx, DWORD ssrc);

	DWORD 	GetIvrSsrc(){return m_ivrSsrc;}
	void 	SetIvrSsrc(DWORD ssrc){m_ivrSsrc=ssrc;}

	BOOL	GetUseSpeakerSsrcForTx () const { return m_bUseSpeakerSsrcForTx;}
	void    SetUseSpeakerSsrcForTx(BOOL isUseSpeakerSsrcForTx){m_bUseSpeakerSsrcForTx = isUseSpeakerSsrcForTx;}

	DWORD   GetSeenImageSsrc() const { return m_dwSeenImageSsrc;}
	void    SetSeenImageSsrc(DWORD ssrc) { m_dwSeenImageSsrc = ssrc;}


protected:

	DWORD m_numOfSsrcIds;
	DWORD m_ssrc[MAX_NUM_OF_SSRCS];
	DWORD m_ivrSsrc;

	// speakerIndication - update
	BOOL m_bUseSpeakerSsrcForTx;	// Valid values: (TRUE, FALSE)
	DWORD m_dwSeenImageSsrc;
};

#endif
