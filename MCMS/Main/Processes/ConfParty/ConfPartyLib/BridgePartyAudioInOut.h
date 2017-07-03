//+========================================================================+
//                        BridgePartyAudioInOut.h                          |
//					      Copyright 2005 Polycom                           |
//                        All Rights Reserved.                             |
//-------------------------------------------------------------------------|
// FILE:       BridgePartyAudioInOut.h                                     |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Matvey                                                      |
//-------------------------------------------------------------------------|
// Who  | Date  Aug-2005  | Description                                    |
//-------------------------------------------------------------------------|
//
//+========================================================================+

#ifndef _BRIDGE_PARTY_AUDIO_INOUT_
#define _BRIDGE_PARTY_AUDIO_INOUT_

#include "BridgePartyMediaUniDirection.h"
#include "DwordBitMask.h"
#include "UpdatePartyAudioInitParams.h"
#include "LegacyToSacTranslator.h"
#include "IpRtpReq.h"

class CConfAppBridgeParams;
class CBridgePartyAudioParams;


/////////////////////////////////////////////////////////////////////////////
// CLASS CBridgePartyAudioUniDirection
/////////////////////////////////////////////////////////////////////////////
class CBridgePartyAudioUniDirection : public CBridgePartyMediaUniDirection
{
	CLASS_TYPE_1(CBridgePartyAudioUniDirection, CBridgePartyMediaUniDirection)
public:

	CBridgePartyAudioUniDirection ();
	CBridgePartyAudioUniDirection (const CBridgePartyAudioUniDirection&);
	virtual ~CBridgePartyAudioUniDirection ();
	virtual const char* NameOf() const { return "CBridgePartyAudioUniDirection";}
	CBridgePartyAudioUniDirection& operator = (const CBridgePartyAudioUniDirection& rOtherBridgePartyAudioUniDirection);

	virtual void	Create (const CBridgePartyCntl*	pBridgePartyCntl,
							const CRsrcParams* pRsrcParams,
							const CBridgePartyMediaParams* pBridgePartyMediaParams);

	virtual void    UpdatePartyParams(CBridgePartyAudioParams* pUpdatePartyAudioInitParams);


	virtual void	Connect(BYTE isIVR)		= 0;
	virtual void	DisConnect()	= 0;

	virtual BOOL	IsConnected()	= 0;
	virtual BOOL	IsConnecting()	= 0;
	virtual BOOL	IsDisConnected() { return ( (m_state == IDLE) ? TRUE : FALSE ); }
	virtual BOOL    IsDisconnecting() = 0;

	// Operations
	virtual void UpdateAudioAlgorithm(DWORD newAudioAlgorithm, DWORD maxAverageBitrate);
	virtual void UpdateMute(EOnOff eOnOff, WORD srcRequest);
	virtual void UpdateAudioVolume(DWORD newAudioVolume);
	virtual void IncreaseAudioVolume(BYTE increaseRate);
	virtual void DecreaseAudioVolume(BYTE decreaseRate);
	virtual void IvrUpdateStandalone(BOOL isStandalone);

	WORD IsMuteSrc(WORD srcRequest);
	BYTE isMuted(void)const;

	WORD GetNetworkInterface();
	BYTE GetConfSampleRate(){return m_byConfSampleRate;}
	BYTE GetNumberOfChannels(){return m_byNumberOfChannels;}
	DWORD GetVolume(){return m_dwVolume;}
	BOOL GetIsVideoParticipant(){return m_isVideoParticipant;}
	DWORD GetmaxAverageBitrate(){return m_maxAverageBitrate;}

	virtual void SendUpdateAudioVolumeToDB()	=0;

protected:

	DWORD	m_dwAudioAlgorithm;		// Valid values: H221 A1 table values for audio commands

	DWORD	m_dwVolume;				// Valid values: from AUDIO_VOLUME_MIN to AUDIO_VOLUME_MAX
	BYTE	m_byNumberOfChannels;	// Valid values: (AUDIO_MONO_NUM_OF_CHANNELS, AUDIO_STEREO_NUM_OF_CHANNELS)
	BYTE	m_byConfSampleRate;		// Valid values: (AUDIO_SAMPLE_RATE_8KHZ, AUDIO_SAMPLE_RATE_16KHZ, AUDIO_SAMPLE_RATE_32KHZ,AUDIO_SAMPLE_RATE_48KHZ)
	BOOL	m_isVideoParticipant;	// Valid values: (TRUE, FALSE)
	CDwordBitMask m_mute_mask;
	DWORD	m_maxAverageBitrate;	// currently for Opus codec were the BitRate is dynamic


	CBridgePartyAudioParams* m_pWaitingForUpdateParams;

	BOOL    m_bIsRelaytoMix;  //was added for the SVC Cascade MFW feature.
	MSFT_CLIENT_ENUM  m_MsftClientType ;


protected:

	void Mute(WORD srcRequest);
	void UnMute(WORD srcRequest);


	void MuteByOperator(void);
	void MuteByParty(void);
    void MuteByMcms(void);
	void UnMuteByOperator(void);
	void UnMuteByParty(void);
    void UnMuteByMcms(void);

	virtual void InitiateUpdateParams()	=	0;
	virtual void SaveAndSendUpdateParams();

	// Audio Bridge Party Events
	virtual void OnAudioBridgePartyConnectIDLE(CSegment* pSeg)			=	0;
	virtual void OnAudioBridgePartyConnectSETUP(CSegment* pSeg)			=	0;
	virtual void OnAudioBridgePartyConnectCONNECTED(CSegment* pSeg)		=	0;
	virtual void OnAudioBridgePartyConnectDISCONNECTING(CSegment* pSeg)	=	0;

	virtual void OnAudioBridgePartyDisConnectIDLE(CSegment* pSeg)			=	0;
	virtual void OnAudioBridgePartyDisConnectSETUP(CSegment* pSeg)			=	0;
	virtual void OnAudioBridgePartyDisConnectCONNECTED(CSegment* pSeg)		=	0;
	virtual void OnAudioBridgePartyDisConnectDISCONNECTING(CSegment* pSeg)	=	0;
	virtual void OnAudioBridgePartyDisConnect(CSegment* pSeg)				=	0;

	virtual void OnAudioBridgePartyUpdateAudioAlgorithmSETUP(CSegment* pSeg);
	virtual void OnAudioBridgePartyUpdateAudioAlgorithmCONNECTED(CSegment* pSeg);

	virtual void OnAudioBridgePartyUpdateMuteSETUP(CSegment* pSeg);
	virtual void OnAudioBridgePartyUpdateMuteCONNECTED(CSegment* pSeg);

	virtual void OnAudioBridgePartyUpdateAudioVolumeSETUP(CSegment* pSeg);
	virtual void OnAudioBridgePartyUpdateAudioVolumeCONNECTED(CSegment* pSeg);

	virtual void OnAudioBridgePartyIncreaseAudioVolumeSETUP(CSegment* pParam);
	virtual void OnAudioBridgePartyIncreaseAudioVolumeCONNECTED(CSegment* pParam);

	virtual void OnAudioBridgePartyDecreaseAudioVolumeSETUP(CSegment* pParam);
	virtual void OnAudioBridgePartyDecreaseAudioVolumeCONNECTED(CSegment* pParam);

	virtual void OnAudioBridgePartyUpdateStandaloneCONNECTED(CSegment* pSeg);

	// Ack Events
	virtual void OnMplAckSETUP(CSegment *)			=	0;
	virtual void OnMplAckDISCONNECTING(CSegment *)	=	0;
	virtual void OnMplAckCONNECTED(CSegment *)		= 	0;

	// Internal Methods


	PDECLAR_MESSAGE_MAP
};


/////////////////////////////////////////////////////////////////////////////
// CLASS CBridgePartyAudioIn
/////////////////////////////////////////////////////////////////////////////
class CBridgePartyAudioIn : public CBridgePartyAudioUniDirection
{
	CLASS_TYPE_1(CBridgePartyAudioIn, CBridgePartyAudioUniDirection)
public:

	// States definition
	enum STATE{SETUP = (IDLE+1), CONNECTED, DISCONNECTING};

	virtual const char* NameOf() const { return "CBridgePartyAudioIn";}
	// Constructors
	CBridgePartyAudioIn ();
	CBridgePartyAudioIn (const CBridgePartyAudioIn&);
	virtual ~CBridgePartyAudioIn ();
	CBridgePartyAudioIn& operator = (const CBridgePartyAudioIn& rOtherBridgePartyAudioIn);

	virtual void	Create (const CBridgePartyCntl*	pBridgePartyCntl,
							const CRsrcParams* pRsrcParams,
							const CBridgePartyMediaParams* pBridgePartyMediaParams,
							const CConfAppBridgeParams* pConfAppBridgeParams);

	virtual void    UpdatePartyInParams(CUpdatePartyAudioInitParams* pUpdatePartyAudioInitParams);


	virtual void*	GetMessageMap();
	virtual void	HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);

	virtual void	Connect();
	virtual void	Connect(BYTE isIVR);
	virtual void	DisConnect();

	//was added in the mix svc avc , need to remove the translators pointers rom routing table
	virtual void    Export();
	virtual void    RemoveConfParams();
	virtual void    Import();
	virtual void    RegisterInTask(CTaskApp* myNewTask = NULL);
	virtual void    UnregisterInTask();

	virtual BOOL	IsConnecting() {return ((m_state == SETUP) ? TRUE : FALSE ); }
	virtual BOOL	IsConnected() { return ( (m_state == CONNECTED) ? TRUE : FALSE ); }
	virtual BOOL    IsDisconnecting() {return  ( (m_state == DISCONNECTING) ? TRUE : FALSE ); }

	// Operations
	virtual void UpdateNoiseDetection(EOnOff eOnOff, BYTE newNoiseDetectionThreshold);
	virtual void UpdateAGC(EOnOff eOnOff);
	virtual void UpdateMediaType(EMediaTypeUpdate eMediaTypeUpdate);

	virtual void SendUpdateAudioVolumeToDB();

	virtual bool UpdateAudioDelay(TAudioUpdateCompressedAudioDelayReq* pstAudioDelay);
	virtual void GetPortsOpened(std::map<eLogicalResourceTypes,bool>& isOpenedRsrcMap);

	void UpdateAudioRelayParams(DWORD ssrc);

	BYTE GetCallDirection()const { return m_byCallDirection; }
	DWORD GetSSRC(){return m_ssrc;}
	CBridgePartyCntl* GetBridgePartyCntlPtr()const;

	//LegacyToSacTranslatorConnected public functions
	void LegacyToSacTranslatorConnected(EStat status);
	void LegacyToSacTranslatorDisconnected(EStat status);

	void UpgradeToMixAvcSvc();
	void InformUpgradeToMixSvcAvcToBridgePartyCnrl(EStat responseStat);



protected:

	BOOL	m_isErrorConcealment;
	BOOL	m_isAGC;
	BOOL	m_isToneRemove;
	BOOL	m_isNoiseReduction;
	BOOL	m_isT1CptDetection;
	BOOL	m_isDtmfDetection;
	BOOL	m_isNoiseDetection;
	BOOL	m_isVtxSupport;
	BOOL	m_isEchoSuppression;
	BOOL	m_isKeyboardSuppression;
	BOOL	m_isAutoMuteNoisyParties;
	BYTE	m_byCallDirection; // (DIALIN / DIALOUT)
	BYTE	m_byNoiseDetectionThreshold;	// 1) syscfg???;
											// 2) temp default (NIR) = E_NOISE_DETECTION_THRESHOLD_5
											// 3) AB type - BYTE ?????
	DWORD	m_audioCompressedDelay; // PCI bug patch (to be removed in V3.x)
	DWORD	m_isAudioClarity;
	DWORD   m_ssrc;
	WORD	m_confSpeakerChangeMode;

	// Legacy to SAC Translator members
	BOOL    m_bIsSupportLegacyToSacTranslate;
	CLegacyToSacTranslator*   m_pLegacyToSacTranslator;
	bool    m_bWaitingForTranslator;
	bool    m_bAckOpenDecoderRecieved;
	bool    m_bAckCloseDecoderRecieved;
	BYTE    m_closeTranslatorAckStatus;



	virtual void InitiateUpdateParams();
	virtual void SaveAndSendUpdateParams();

	// Audio Bridge Party Events
	virtual void OnAudioBridgePartyConnectIDLE(CSegment* pSeg);
	virtual void OnAudioBridgePartyConnectSETUP(CSegment* pSeg);
	virtual void OnAudioBridgePartyConnectCONNECTED(CSegment* pSeg);
	virtual void OnAudioBridgePartyConnectDISCONNECTING(CSegment* pSeg);

	virtual void OnAudioBridgePartyDisConnectIDLE(CSegment* pSeg);
	virtual void OnAudioBridgePartyDisConnectSETUP(CSegment* pSeg);
	virtual void OnAudioBridgePartyDisConnectCONNECTED(CSegment* pSeg);
	virtual void OnAudioBridgePartyDisConnectDISCONNECTING(CSegment* pSeg);
	virtual void OnAudioBridgePartyDisConnect(CSegment* pSeg);

	virtual void OnAudioBridgePartyUpdateAudioAlgorithmSETUP(CSegment* pSeg);
	virtual void OnAudioBridgePartyUpdateAudioAlgorithmCONNECTED(CSegment* pSeg);

	virtual void OnAudioBridgePartyUpdateMediaTypeSETUP(CSegment* pSeg);
	virtual void OnAudioBridgePartyUpdateMediaTypeCONNECTED(CSegment* pSeg);

	virtual void OnAudioBridgePartyUpdateMuteSETUP(CSegment* pSeg);
	virtual void OnAudioBridgePartyUpdateMuteCONNECTED(CSegment* pSeg);

	virtual void OnAudioBridgePartyUpdateAudioVolumeSETUP(CSegment* pSeg);
	virtual void OnAudioBridgePartyUpdateAudioVolumeCONNECTED(CSegment* pSeg);

	virtual void OnAudioBridgePartyIncreaseAudioVolumeSETUP(CSegment* pParam);
	virtual void OnAudioBridgePartyIncreaseAudioVolumeCONNECTED(CSegment* pParam);

	virtual void OnAudioBridgePartyDecreaseAudioVolumeSETUP(CSegment* pParam);
	virtual void OnAudioBridgePartyDecreaseAudioVolumeCONNECTED(CSegment* pParam);

	virtual void OnAudioBridgePartyUpdateAgcSETUP(CSegment* pParam);
	virtual void OnAudioBridgePartyUpdateAgcCONNECTED(CSegment* pParam);

	virtual void OnAudioBridgePartyUpdateNoiseDetectionCONNECTED(CSegment* pParam);

	virtual void OnAudioBridgePartyUpdateStandaloneCONNECTED(CSegment* pSeg);

	virtual void OnMplDTMFIndSETUP(CSegment* pSeg);
	virtual void OnMplDTMFIndDISCONNECTING(CSegment* pSeg);
	virtual void OnMplDTMFIndCONNECTED(CSegment* pSeg);

	virtual void OnLegacyToSacTranslatorConnectedIDLE(CSegment* pSeg);
	virtual void OnLegacyToSacTranslatorConnectedSETUP(CSegment* pSeg);
	virtual void OnLegacyToSacTranslatorConnectedCONNECTED(CSegment* pSeg);
	virtual void OnLegacyToSacTranslatorConnectedDISCONNECTING(CSegment* pSeg);

	virtual void OnLegacyToSacTranslatorDisconnectedIDLE(CSegment* pSeg);
	virtual void OnLegacyToSacTranslatorDisconnectedSETUP(CSegment* pSeg);
	virtual void OnLegacyToSacTranslatorDisconnectedCONNECTED(CSegment* pSeg);
	virtual void OnLegacyToSacTranslatorDisconnectedDISCONNECTING(CSegment* pSeg);
	void OnLegacyToSacTranslatorDisconnected();

	//virtual void OnLegacyToSacTraslatorKillANYCASE(CSegment* pSeg);

	virtual void OnUpgradeToMixAvcSvcIDLE(CSegment* pSeg);
	virtual void OnUpgradeToMixAvcSvcSETUP(CSegment* pSeg);
	virtual void OnUpgradeToMixAvcSvcCONNECTED(CSegment* pSeg);
	virtual void OnUpgradeToMixAvcSvcDISCONNECTING(CSegment* pSeg);

	// Ack Events
	virtual void OnMplAckSETUP(CSegment *);
	virtual void OnMplAckCONNECTED(CSegment*);
	virtual void OnMplAckDISCONNECTING(CSegment *);

	// Internal Events
	//notify for relay participants
	void NotifyRelayAudioStreamChanges();
	void NotifyRelayAudioStreamRemoved();

	bool TranslateAudioAlgoToRelayNotificationsSubCodecType(DWORD dwMcmsAudioAlgorithmOpcode, ECodecSubType& codecSubType, int & bitRate, bool& stereo);

	//Connect disconnect flow functions with translator legacy->SAC
	bool CanBeConnected();
	void SetConnected();
	void InformInClosed();
	void CheckAndInformAllClosed();
	void InitDisconnectionFlags();

	//Legacy to SAC internal functions
	bool IsTranslateLegacyToSacSupported();
	void TranslateLegacyToSac();
	int CreateAndConnectLegacyToSacTranslator();
	void NewLegacyToSacTranslator();
	void RemoveLegacyToSacTranslatorFromRoutingTable();
	EStat AddLegacyToSacTranslatorToRoutingTable();
	void InitAllLegacyToSacTranslatorFlags();
	void DestroyLegacyToSacTranslator();
	void UpdatePartyOnTranslatorDisconnectedIfNeeded();





	PDECLAR_MESSAGE_MAP
};

/////////////////////////////////////////////////////////////////////////////
// CLASS CBridgePartyAudioOut
/////////////////////////////////////////////////////////////////////////////
class CBridgePartyAudioOut : public CBridgePartyAudioUniDirection
{
	CLASS_TYPE_1(CBridgePartyAudioOut, CBridgePartyAudioUniDirection)
public:

	// States definition
	enum STATE{SETUP = (IDLE+1), CONNECTED, DISCONNECTING};

	virtual const char* NameOf() const { return "CBridgePartyAudioOut";}
	// Constructors
	CBridgePartyAudioOut ();
	CBridgePartyAudioOut (const CBridgePartyAudioOut&);
	virtual ~CBridgePartyAudioOut ();

	virtual void	Create (const CBridgePartyCntl*	pBridgePartyCntl,
							const CRsrcParams* pRsrcParams,
							const CBridgePartyMediaParams* pBridgePartyMediaParams);


	virtual void*	GetMessageMap();
	virtual void	HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);

	virtual void	Connect();
	virtual void	Connect(BYTE isIVR);
	virtual void	DisConnect();

	virtual void    IvrCommand(OPCODE opcode, DWORD seqNumToken, CSegment *pDataSeg);

	virtual BOOL	IsConnecting() {return ((m_state == SETUP) ? TRUE : FALSE ); }
	virtual BOOL	IsConnected() { return ( (m_state == CONNECTED) ? TRUE : FALSE ); }
	virtual BOOL    IsDisconnecting() {return  ( (m_state == DISCONNECTING) ? TRUE : FALSE ); }


	// Operations
	virtual void SendUpdateAudioVolumeToDB();

	virtual void UpdatePartyOutParams(CUpdatePartyAudioInitParams* pUpdatePartyAudioInitParams);
	virtual void UpdateUseSpeakerSsrcForTx(BOOL updatedUseSpeakerSsrcForTx);

	virtual void GetPortsOpened(std::map<eLogicalResourceTypes,bool>& isOpenedRsrcMap);

	void UpdateAudioRelayParams(DWORD numOfSsrcIds, DWORD* ssrc_array ,DWORD ivrSsrc);
	void UpdateAudioOutOnSeenImageSSRC(DWORD audioUplinkSSRCOfSeenImage);


protected:

	virtual void InitiateUpdateParams();
	virtual void SaveAndSendUpdateParams();
	virtual void SendSequenceNumIndicationToCAM(OPCODE opcode, DWORD seqNumToken, DWORD sequenceNum);
	virtual void SendAudioEncoderUpdateSeenImageSsrcToHardware(DWORD ssrc);


	// Audio Bridge Party Events
	virtual void OnAudioBridgePartyConnectIDLE(CSegment* pSeg);
	virtual void OnAudioBridgePartyConnectSETUP(CSegment* pSeg);
	virtual void OnAudioBridgePartyConnectCONNECTED(CSegment* pSeg);
	virtual void OnAudioBridgePartyConnectDISCONNECTING(CSegment* pSeg);

	virtual void OnAudioBridgePartyDisConnectIDLE(CSegment* pSeg);
	virtual void OnAudioBridgePartyDisConnectSETUP(CSegment* pSeg);
	virtual void OnAudioBridgePartyDisConnectCONNECTED(CSegment* pSeg);
	virtual void OnAudioBridgePartyDisConnectDISCONNECTING(CSegment* pSeg);
	virtual void OnAudioBridgePartyDisConnect(CSegment* pSeg);

	virtual void OnAudioBridgePartyUpdateAudioAlgorithmSETUP(CSegment* pSeg);
	virtual void OnAudioBridgePartyUpdateAudioAlgorithmCONNECTED(CSegment* pSeg);

	virtual void OnAudioBridgePartyUpdateUseSpeakerSsrcForTxSETUP(CSegment* pSeg);
	virtual void OnAudioBridgePartyUpdateUseSpeakerSsrcForTxCONNECTED(CSegment* pSeg);

	virtual void OnAudioBridgePartyUpdateMuteSETUP(CSegment* pSeg);
	virtual void OnAudioBridgePartyUpdateMuteCONNECTED(CSegment* pSeg);

	virtual void OnAudioBridgePartyUpdateAudioVolumeSETUP(CSegment* pSeg);
	virtual void OnAudioBridgePartyUpdateAudioVolumeCONNECTED(CSegment* pSeg);

	virtual void OnAudioBridgePartyIncreaseAudioVolumeSETUP(CSegment* pParam);
	virtual void OnAudioBridgePartyIncreaseAudioVolumeCONNECTED(CSegment* pParam);

	virtual void OnAudioBridgePartyDecreaseAudioVolumeSETUP(CSegment* pParam);
	virtual void OnAudioBridgePartyDecreaseAudioVolumeCONNECTED(CSegment* pParam);

	virtual void OnAudioBridgePartyUpdateStandaloneCONNECTED(CSegment* pSeg);

	virtual void OnAudioBridgePlayMessageCONNECTED(CSegment* pSeg);
	virtual void OnAudioBridgeStopPlayMessageCONNECTED(CSegment* pSeg);
	virtual void OnAudioBridgeStartIvrSeqCONNECTED(CSegment* pSeg);
	virtual void OnAudioBridgeStopIvrSeqCONNECTED(CSegment* pSeg);
	virtual void OnAudioBridgeStopIvrSeqDISCONNECTING(CSegment* pSeg);
	virtual void OnAudioBridgePlayMusicCONNECTED(CSegment* pSeg);
	virtual void OnAudioBridgeStopPlayMusicCONNECTED(CSegment* pSeg);
	virtual void OnAudioBridgePlayToneCONNECTED(CSegment* pSeg);
   virtual	void OnAudioBridgePlayToneDISCONNECTED(CSegment* pParam);
	virtual void OnAudioBridgeRecordRollCallCONNECTED(CSegment* pSeg);
	virtual void OnAudioBridgeStopRecordRollCall(CSegment* pSeg);

	virtual void OnMplRecordRollCallIndSETUP(CSegment* pSeg);
	virtual void OnMplRecordRollCallIndDISCONNECTING(CSegment* pSeg);
	virtual void OnMplRecordRollCallIndCONNECTED(CSegment* pSeg);

	// Ack Events
	virtual void OnMplAckSETUP(CSegment *);
	virtual void OnMplAckCONNECTED(CSegment *);
	virtual void OnMplAckDISCONNECTING(CSegment *);

	virtual void OnAudioBridgePartyUpdateSeenImageSsrcSETUP(CSegment* pParam);
	virtual void OnAudioBridgePartyUpdateSeenImageSsrcCONNECTED(CSegment* pParam);
	virtual void OnAudioBridgePartyUpdateSeenImageSsrcDISCONNECTING(CSegment* pParam);


    DWORD m_numOfSsrcIds;
    DWORD m_ssrc[MAX_NUM_OF_SSRCS];
    DWORD m_ivrSsrc;
    BOOL  m_bUseSpeakerSsrcForTx;	// Valid values: (TRUE, FALSE)
    DWORD m_seenImageSSRC;

	// Internal Events

	PDECLAR_MESSAGE_MAP
};


#endif
