//+========================================================================+
//                   AudioBridgeInterface.H                                |
//					 Copyright 2005 Polycom, Inc                           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// FILE:       AudioBridgeInterface.H                                      |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: UDI                                                         |
//-------------------------------------------------------------------------|
// Who  | Date  July-2005  | Description                                   |
//-------------------------------------------------------------------------|

#ifndef _AUDIO_BRIDGE_INTERFACE_H_
#define _AUDIO_BRIDGE_INTERFACE_H_

#include "BridgeInterface.h"

class CAudioBridge;

class CAudioBridgeInterface : public CBridgeInterface 
{
CLASS_TYPE_1(CAudioBridgeInterface,CBridgeInterface)
public:
	
	CAudioBridgeInterface();
	virtual ~CAudioBridgeInterface ();
	virtual const char* NameOf() const { return "CAudioBridgeInterface";}
	
	virtual void	HandleEvent(CSegment* pMsg);

	// Audio Interface Functions
	virtual void UpdateAudioAlgorithm (PartyRsrcID partyRsrcID, EMediaDirection eMediaDirection, DWORD newAudioAlgorithm, DWORD bitRate = 0);
	virtual void UpdateMediaType (PartyRsrcID partyRsrcID, EMediaTypeUpdate eMediaTypeUpdate);
	virtual void UpdateUseSpeakerSsrcForTx(PartyRsrcID partyRsrcID, BOOL updateUseSpeakerSsrcForTx);
	virtual void UpdateMute (PartyRsrcID partyRsrcID, EMediaDirection eMediaDirection, EOnOff eOnOff, WORD srcRequest);
	virtual void UpdateAudioVolume (PartyRsrcID partyRsrcID, EMediaDirection eMediaDirection, DWORD newAudioVolume);
	virtual void IncreaseAudioVolume (PartyRsrcID partyRsrcID, EMediaDirection eMediaDirection, BYTE increaseRate);
	virtual void DecreaseAudioVolume (PartyRsrcID partyRsrcID, EMediaDirection eMediaDirection, BYTE decreaseRate);	
	virtual void UpdateNoiseDetection(PartyRsrcID partyRsrcID, EOnOff eOnOff, BYTE newNoiseDetectionThreshold);
	
	virtual void UpdateConfParams(WORD talkHoldTime,BYTE audioMixDepth);
	virtual void SpeakerChangeIndication(CSegment* pSpeakerChangeParams, DWORD dwSpeakerChangeOpcode);
	
	virtual void IvrPartyCommand(PartyRsrcID partyID, OPCODE opcode, DWORD seqNumToken, CSegment *pDataSeg);
	virtual void IvrConfCommand(OPCODE opcode, DWORD seqNumToken, CSegment *pDataSeg);
	virtual void IvrUpdatePartyStandalone(PartyRsrcID partyID, BOOL isStandalone);

	virtual bool UpdateAudioDelay(PartyRsrcID partyID, TAudioUpdateCompressedAudioDelayReq* pstAudioDelay);

	virtual bool UpdateAudioRelayParamsIn(PartyRsrcID partyRsrcID, DWORD ssrc);
	virtual bool UpdateAudioRelayParamsOut(PartyRsrcID partyRsrcID, DWORD numOfSsrcIds, DWORD* ssrc_array, DWORD ivrSsrc);

	virtual void UpgradeToMixAvcSvc(PartyRsrcID partyId);


protected:

	virtual void 	CreateImplementation(const CBridgeInitParams* pAudioBridgeInitParams) ;
	CAudioBridge*	GetBridgeImplementation() {return (CAudioBridge*)m_pBridgeImplementation;}
	
	CAudioBridgeInterface (const CAudioBridgeInterface& );
	CAudioBridgeInterface& operator= (const CAudioBridgeInterface& );
};

#endif
