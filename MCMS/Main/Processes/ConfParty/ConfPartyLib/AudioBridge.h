#ifndef _AUDIO_BRIDGE_H_
#define _AUDIO_BRIDGE_H_

#include "Bridge.h"

class CAudioHardwareInterface;
class CAudioBridgeInitParams;
class CAudioBridgePartyCntl;

////////////////////////////////////////////////////////////////////////////
//                        CAudioBridge
////////////////////////////////////////////////////////////////////////////
class CAudioBridge : public CBridge
{
	CLASS_TYPE_1(CAudioBridge, CBridge)

public:

	// States definition
	enum STATE {CONNECTED = (IDLE+1), DISCONNECTING};

	              CAudioBridge();
	virtual      ~CAudioBridge ();

	virtual const char* NameOf() const { return "CAudioBridge";}

	virtual void  Create(const CAudioBridgeInitParams* pAudioBridgeInitParams);
	virtual void  Destroy();

	virtual void  HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode);
	virtual void* GetMessageMap();

	// Party Operations
	virtual void  UpdateAudioAlgorithm(PartyRsrcID partyRsrcID, EMediaDirection eMediaDirection, DWORD newAudioAlgorithm, DWORD maxAverageBitrate);
	virtual void  UpdateMediaType(PartyRsrcID partyRsrcID, EMediaTypeUpdate eMediaTypeUpdate);
	virtual void  UpdateUseSpeakerSsrcForTx(PartyRsrcID partyRsrcID,BOOL updatedUseSpeakerSsrcForTx);
	virtual void  UpdateMute(PartyRsrcID partyRsrcID, EMediaDirection eMediaDirection, EOnOff eOnOff, WORD srcRequest);
	virtual void  UpdateAudioVolume(PartyRsrcID partyRsrcID, EMediaDirection eMediaDirection, DWORD newAudioVolume);
	virtual void  IncreaseAudioVolume(PartyRsrcID partyRsrcID, EMediaDirection eMediaDirection, BYTE increaseRate);
	virtual void  DecreaseAudioVolume(PartyRsrcID partyRsrcID, EMediaDirection eMediaDirection, BYTE decreaseRate);
	virtual void  UpdateNoiseDetection(PartyRsrcID partyRsrcID, EOnOff eOnOff, BYTE newNoiseDetectionThreshold);

	// Conf Operations
	virtual void  UpdateConfParams(WORD newTalkHoldTime, BYTE newAudioMixDepth);
	virtual void  SpeakerChangeIndication(CSegment* pSpeakerChangeParams, DWORD dwSpeakerChangeOpcode);

	virtual void  IvrPartyCommand(PartyRsrcID partyID, OPCODE opcode, DWORD seqNumToken, CSegment* pDataSeg);
	virtual void  IvrConfCommand(OPCODE opcode, DWORD seqNumToken, CSegment* pDataSeg);
	virtual void  IvrUpdatePartyStandalone(PartyRsrcID partyID, BOOL isStandalone);

	virtual bool  UpdateAudioDelay(PartyRsrcID partyID, TAudioUpdateCompressedAudioDelayReq* pstAudioDelay);
	virtual bool  UpdateAudioRelayParamsIn(PartyRsrcID partyRsrcID, DWORD ssrc);
	virtual bool  UpdateAudioRelayParamsOut(PartyRsrcID partyRsrcID, DWORD numOfSsrcIds, DWORD* ssrc_array,DWORD ivrSsrc);

	virtual void UpgradeToMixAvcSvc(PartyRsrcID partyRsrcID);
	virtual void ReplyUpgradeToMixAvcSvcUponError(PartyRsrcID partyRsrcID);

protected:
	              CAudioBridge(const CAudioBridge&);
	CAudioBridge& operator =(const CAudioBridge&);

protected:

	CTaskApp*     m_pCurrentSpeakerParty;

	WORD          m_wTalkHoldTime;
	BYTE          m_byAudioMixDepth;
	BOOL          m_isAutoMuteNoisyParties;

	CAudioHardwareInterface* m_pAudioHardwareInterface;
	CSegment 	  m_seg;	// for efficient, save "new"
	eProductType  m_ePT;

	// Action Functions
	void OnConfUpdateConfParamsCONNECTED(CSegment* pParam);
	void OnConfUpdateConfParamsDISCONNECTING(CSegment* pParam);

	void OnConfUpdateACLayoutChangeCompleteCONNECTED(CSegment* pParam);

	void OnConfDisConnectConfCONNECTED(CSegment* pParam);
	void OnConfDisConnectConfDISCONNECTING(CSegment* pParam);

	void OnConfTerminateCONNECTED(CSegment* pParam);
	void OnConfTerminateDISCONNECTING(CSegment* pParam);

	void OnConfConnectPartyCONNECTED(CSegment* pParam);
	void OnConfConnectPartyDISCONNECTING(CSegment* pParam);

	void OnConfDisconnectParty(CSegment* pParam);

	void OnConfExportPartyCONNECTED(CSegment* pParam);
	void OnConfExportPartyDISCONNECTING(CSegment* pParam);

	void OnEndPartyExportCONNECTED(CSegment* pParam);
	void OnEndPartyExportDISCONNECTING(CSegment* pParam);

	void OnEndPartyConnectCONNECTED(CSegment* pParam);
	void OnEndPartyConnectDISCONNECTING(CSegment* pParam);

	void OnEndPartyDisConnect(CSegment* pParam);

	void OnActiveSpeakerChangeCONNECTED(CSegment* pParam);

	void OnConfPlayMsgCONNECT(CSegment* pParam);

	void OnConfStopPlayMsgCONNECT(CSegment* pParam);
	void OnConfStopPlayMsgDISCONNECTING(CSegment* pParam);
	void OnConfStopPlayMsg(CSegment* pParam);

	void OnConfStartIvrSeqCONNECT(CSegment* pParam);

	void OnConfStopIvrSeqCONNECT(CSegment* pParam);
	void OnConfStopIvrSeqDISCONNECTING(CSegment* pParam);
	void OnConfStopIvrSeq(CSegment* pParam);

	void OnConfPlayMusicCONNECT(CSegment* pParam);

	void OnConfStopPlayMusicCONNECT(CSegment* pParam);
	void OnConfStopPlayMusicDISCONNECTING(CSegment* pParam);
	void OnConfStopPlayMusic(CSegment* pParam);

	void OnConfUpdateAudioVolumeCONNECTED(CSegment* pParam);
	void OnConfUpdateAudioMuteCONNECTED(CSegment* pParam);
	void OnConfUpdateAgcCONNECTED(CSegment* pParam);

	// Internal Methods
	void ConnectParty(CAudioBridgePartyCntl* pBridgePartyCntl, BYTE isIVR);
	void OpenConf(WORD bReOpenConf = FALSE, WORD new_card_board_id = (WORD)-1);
	void CloseConf();
	void SpeakersChanged(CSegment* pParam);
	void SendSequenceNumIndicationToCAM(OPCODE opcode, DWORD seqNumToken, DWORD sequenceNum);
	void OnConfOpenConfReSendCONNECTED(CSegment* pParam);
	void UpdateSpeakerList(CSegment* pParam);

	//AudioVideoSync for SVC cascade in MFW
	virtual void OnVideoUpdateSeenImageCONNECTED(CSegment* pParam);


	PDECLAR_MESSAGE_MAP
};

#endif // ifndef _AUDIO_BRIDGE_H_
