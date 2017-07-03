//+========================================================================+
//                   AudioBridgePartyCntl.H                                |
//					 Copyright 2005 Polycom, Inc                           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// FILE:       AudioBridgePartyCntl.H                                      |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Matvey                                                      |
//-------------------------------------------------------------------------|
// Who  | Date  Aug-2005  | Description                                    |
//-------------------------------------------------------------------------|

#ifndef _AUDIO_BRIDGE_PARTY_CNTL_
#define _AUDIO_BRIDGE_PARTY_CNTL_

#include "BridgePartyCntl.h"
#include "UpdatePartyAudioInitParams.h"

class CBridgePartyAudioParams;

class CAudioBridgePartyCntl : public CBridgePartyCntl {
CLASS_TYPE_1(CAudioBridgePartyCntl,CBridgePartyCntl)
public:

	// States definition
	enum STATE{SETUP = (IDLE+1), CONNECTED, DISCONNECTING};

	virtual const char* NameOf() const { return "CAudioBridgePartyCntl";}
	// Constructors
	virtual ~CAudioBridgePartyCntl ();
	CAudioBridgePartyCntl ();
	CAudioBridgePartyCntl(const void* partyId, const char* partyName = NULL, PartyRsrcID partyRsrcID = 0);

	virtual void	Create (const CBridgePartyInitParams* pBridgePartyInitParams);
	virtual void    Import (const CBridgePartyInitParams* pBridgePartyInitParams);
	virtual void    Update (const CBridgePartyInitParams* pBridgePartyInitParams);

	virtual void	Destroy ();

	virtual void	Connect (BYTE isIVR);
	virtual void	DisConnect ();
	virtual void    Export ();

	virtual void*	GetMessageMap();
	virtual void	HandleEvent(CSegment *pMsg, DWORD msgLen, OPCODE opCode);

	virtual BOOL	IsConnected() const { return ( (m_state == CONNECTED) ? TRUE : FALSE ); }
	virtual void	GetPortsOpened(std::map<eLogicalResourceTypes,bool>& isOpenedRsrcMap)const;

	// Operations
	BOOL IsDirectionMuteSrcSet(EMediaDirection eMediaDirection, WORD srcRequest);
	void UpdateAudioAlgorithm(EMediaDirection eMediaDirection, DWORD newAudioAlgorithm, DWORD maxAverageBitrate);
	void UpdateMediaInType(EMediaTypeUpdate eMediaTyprUpdate);
	void UpdateUseSpeakerSsrcForTx(BOOL updatedUseSpeakerSsrcForTx);
	void IncreaseAudioVolume (EMediaDirection eMediaDirection, BYTE increaseRate);
	void DecreaseAudioVolume (EMediaDirection eMediaDirection, BYTE decreaseRate);
	void UpdateMute(EMediaDirection eMediaDirection, EOnOff eOnOff, WORD srcRequest);
	void UpdateAudioVolume(EMediaDirection eMediaDirection, DWORD newAudioVolume);
	void UpdateNoiseDetection(EOnOff eOnOff, BYTE newNoiseDetectionThreshold);
	void UpdateAGC(EOnOff eOnOff);
	void UpdateMediaType(EMediaTypeUpdate eMediaTypeUpdate);

	void IvrCommand(OPCODE opcode, DWORD seqNumToken, CSegment *pDataSeg);
	void IvrUpdateStandalone(BOOL isStandalone);
	void IvrNotification(OPCODE opcode, CSegment *pParam);

	void InitiateUpdatePartyParams(const CBridgePartyInitParams* pBridgePartyInitParams);

	bool UpdateAudioDelay(TAudioUpdateCompressedAudioDelayReq* pstAudioDelay);
	bool UpdateAudioRelayParamsIn(DWORD ssrc);
	bool UpdateAudioRelayParamsOut(DWORD numOfSsrcIds, DWORD* ssrc_array ,DWORD ivrSsrc);
	BOOL GetIsVideoRelay(){return m_bIsVideoRelay;}

	void UpgradeToMixAvcSvc();
	void SendEndUpgradeToMixAvcSvcToPartyCntl(EStat responseStatus);
	DWORD GetSSRCOfIn();
	void SendUpdateEncoderOnSeenImageSSRC(DWORD audioUplinkSSRCOfSeenImage);

protected:
	CAudioBridgePartyCntl (const CAudioBridgePartyCntl& );
	CAudioBridgePartyCntl&	operator= (const CAudioBridgePartyCntl&);

	// Audio Bridge Events
	void OnAudioBridgeConnectIDLE(CSegment* pParams);
	void OnAudioBridgeConnectSETUP(CSegment* pParams);
	void OnAudioBridgeConnectCONNECTED(CSegment* pParams);
	void OnAudioBridgeConnect(CSegment* pParams);
	void OnAudioBridgeConnectDISCONNECTING(CSegment* pParams);

	void OnAudioBridgeDisConnectSETUP(CSegment* pParams);
	void OnAudioBridgeDisConnectCONNECTED(CSegment* pParams);
	void OnAudioBridgeDisConnectDISCONNECTING(CSegment* pParams);
	void OnAudioBridgeDisConnect(CSegment* pParams);

	void OnAudioBridgeUpdateAudioAlgorithmSETUP(CSegment* pParams);
	void OnAudioBridgeUpdateAudioAlgorithmCONNECTED(CSegment* pParams);
	void OnAudioBridgeUpdateAudioAlgorithmDISCONNECTING(CSegment* pParams);
	void OnAudioBridgeUpdateAudioAlgorithm(CSegment* pParams);

	void OnAudioBridgeUpdateMediaTypeSETUP(CSegment* pParams);
	void OnAudioBridgeUpdateMediaTypeCONNECTED(CSegment* pParams);
	void OnAudioBridgeUpdateMediaTypeDISCONNECTING(CSegment* pParams);
	void OnAudioBridgeUpdateMediaType(CSegment* pParams);

	void OnAudioBridgeUpdateMuteSETUP(CSegment* pParams);
	void OnAudioBridgeUpdateMuteCONNECTED(CSegment* pParams);
	void OnAudioBridgeUpdateMuteDISCONNECTING(CSegment* pParams);
 	void OnAudioBridgeUpdateMute(CSegment* pParams);

 	void OnAudioBridgeUpdateAudioVolumeSETUP(CSegment* pParams);
	void OnAudioBridgeUpdateAudioVolumeCONNECTED(CSegment* pParams);
	void OnAudioBridgeUpdateAudioVolumeDISCONNECTING(CSegment* pParams);
	void OnAudioBridgeUpdateAudioVolume(CSegment* pParams);

	void OnAudioBridgeIncreaseAudioVolumeSETUP(CSegment* pParams);
	void OnAudioBridgeIncreaseAudioVolumeCONNECTED(CSegment* pParams);
	void OnAudioBridgeIncreaseAudioVolumeDISCONNECTING(CSegment* pParams);
	void OnAudioBridgeIncreaseAudioVolume(CSegment* pParams);

	void OnAudioBridgeDecreaseAudioVolumeSETUP(CSegment* pParams);
	void OnAudioBridgeDecreaseAudioVolumeCONNECTED(CSegment* pParams);
	void OnAudioBridgeDecreaseAudioVolumeDISCONNECTING(CSegment* pParams);
	void OnAudioBridgeDecreaseAudioVolume(CSegment* pParams);

	void OnAudioBridgeUpdateNoiseDetectionSETUP(CSegment* pParams);
	void OnAudioBridgeUpdateNoiseDetectionCONNECTED(CSegment* pParams);
	void OnAudioBridgeUpdateNoiseDetectionDISCONNECTING(CSegment* pParams);
	void OnAudioBridgeUpdateNoiseDetection(CSegment* pParams);

	void OnAudioBridgeUpdateAgcSETUP(CSegment* pParams);
	void OnAudioBridgeUpdateAgcCONNECTED(CSegment* pParams);
	void OnAudioBridgeUpdateAgcDISCONNECTING(CSegment* pParams);
	void OnAudioBridgeUpdateAgc(CSegment* pParams);

	void OnAudioBridgeUpdateUseSpeakerSsrcForTxSETUP(CSegment* pParams);
	void OnAudioBridgeUpdateUseSpeakerSsrcForTxCONNECTED(CSegment* pParams);
	void OnAudioBridgeUpdateUseSpeakerSsrcForTxDISCONNECTING(CSegment* pParams);
	void OnAudioBridgeUpdateUseSpeakerSsrcForTx(CSegment* pParams);

	void OnAudioBridgeExportSETUP(CSegment* pParams);
	void OnAudioBridgeExportCONNECTED(CSegment* pParams);
	void OnAudioBridgeExportDISCONNECTING(CSegment* pParams);

	// Audio In/Out Events
	void OnAudioInConnectedSETUP(CSegment* pParams);
	void OnAudioInConnectedCONNECTED(CSegment* pParams);
	void OnAudioInConnectedDISCONNECTING(CSegment* pParams);
	void OnAudioOutConnectedSETUP(CSegment* pParams);
	void OnAudioOutConnectedCONNECTED(CSegment* pParams);
	void OnAudioOutConnectedDISCONNECTING(CSegment* pParams);
	void OnAudioInDisConnectedSETUP(CSegment* pParams);
	void OnAudioOutDisConnectedSETUP(CSegment* pParams);
	void OnAudioInDisConnectedDISCONNECTING(CSegment* pParams);
	void OnAudioOutDisConnectedDISCONNECTING(CSegment* pParams);
	void OnAudioInDisConnectedCONNECTED(CSegment* pParams);
	void OnAudioOutDisConnectedCONNECTED(CSegment* pParams);
	void OnAudioInDisConnected(CSegment* pParams);
	void OnAudioOutDisConnected(CSegment* pParams);
	void OnAudioInVolumeChanged(CSegment* pParams);
	void OnAudioOutVolumeChanged(CSegment* pParams);

	// Timer events
	void OnTimerPartySetupSETUP(CSegment* pParams);
	void OnTimerPartySetupCONNECTED(CSegment* pParams);
	void OnTimerPartySetup(CSegment* pParams);
	void OnTimerPartyDisconnectDISCONNECTING(CSegment* pParams);
	void OnTimerPartyDisconnectCONNECTED(CSegment* pParams);
	void OnTimerPartyDisconnectSETUP(CSegment* pParams);
	void OnTimerPartyDisconnect(CSegment* pParams);

	// Internal Methods
	void AudioConnectionCompletion(CSegment* pParams, EMediaDirection eConnectedMediaDirection);
	void AudioDisConnectionCompletion(CSegment* pParams, EMediaDirection eDisConnectedMediaDirection);
	void UpdateAudioInAlgorithm(DWORD newAudioAlgorithm, DWORD maxAverageBitrate);
	void UpdateAudioOutAlgorithm(DWORD newAudioAlgorithm, DWORD maxAverageBitrate);
	void IncreaseAudioInVolume(BYTE increaseRate);
	void IncreaseAudioOutVolume(BYTE increaseRate);
	void DecreaseAudioInVolume(BYTE decreaseRate);
	void DecreaseAudioOutVolume(BYTE decreaseRate);
	void CheckIsMutedAudioIn();
	void MuteAudioIn(EOnOff eOnOff, WORD srcRequest);
	void UpdateDBMuteAudioIn(EOnOff eOnOff, WORD srcRequest);
	void MuteAudioOut(EOnOff eOnOff, WORD srcRequest);
	void UpdateAudioInVolume(DWORD newAudioVolume);
	void UpdateAudioOutVolume(DWORD newAudioVolume);

	void StartConnectProcess();
	void CreatePartyOut();
	void CreatePartyIn();

    eLogicalResourceTypes GetLogicalResourceTypeDec();
	eLogicalResourceTypes GetLogicalResourceTypeEnc();

	CUpdatePartyAudioInitParams* m_pUpdatePartyInitParams;

	BOOL                         m_bIsVideoRelay;

	PDECLAR_MESSAGE_MAP
};

#endif
