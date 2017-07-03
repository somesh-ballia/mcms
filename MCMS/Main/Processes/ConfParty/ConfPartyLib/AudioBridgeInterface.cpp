//+========================================================================+
//                   AudioBridgeInterface.CPP                              |
//					 Copyright 2005 Polycom, Inc                           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// FILE:       AudioBridgeInterface.CPP                                    |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: UDI                                                         |
//-------------------------------------------------------------------------|
// Who  | Date  July-2005  | Description                                   |
//-------------------------------------------------------------------------|

#include "AudioBridgeInterface.h"
#include "AudioBridge.h"

/////////////////////////////////////////////////////////////////////////////
CAudioBridgeInterface::CAudioBridgeInterface()
{
	
}

/////////////////////////////////////////////////////////////////////////////
CAudioBridgeInterface::~CAudioBridgeInterface()
{

}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgeInterface::CreateImplementation(const CBridgeInitParams* pAudioBridgeInitParams) 
{
	if (m_pBridgeImplementation)
		POBJDELETE(m_pBridgeImplementation);
		 
	switch(pAudioBridgeInitParams->GetBridgeImplementationType())
	{
	case eAudio_Bridge_V1:
		{	  
			m_pBridgeImplementation = new CAudioBridge;
			break;
		}
	default: 
		{
			PASSERT(1);
			return;
		}
	}
	
	if(GetBridgeImplementation())
		GetBridgeImplementation()->Create((CAudioBridgeInitParams*)pAudioBridgeInitParams);
	else
		PASSERT(1);

}

/////////////////////////////////////////////////////////////////////////////
void  CAudioBridgeInterface::HandleEvent(CSegment* pMsg)
{										
	PASSERT_AND_RETURN(!pMsg);
	OPCODE opcode;
	*pMsg >> opcode;
	if (m_pBridgeImplementation)
	{
		((CAudioBridge*)m_pBridgeImplementation)->HandleEvent(pMsg, 0, opcode);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgeInterface::UpdateAudioAlgorithm(PartyRsrcID partyRsrcID, EMediaDirection eMediaDirection, 
												 DWORD newAudioAlgorithm, DWORD bitRate)
{
	if(GetBridgeImplementation())
		GetBridgeImplementation()->UpdateAudioAlgorithm(partyRsrcID, eMediaDirection, newAudioAlgorithm, bitRate);
	else
		PASSERT(1);
}


/////////////////////////////////////////////////////////////////////////////
void CAudioBridgeInterface::UpdateMediaType(PartyRsrcID partyRsrcID, EMediaTypeUpdate eMediaTypeUpdate)
{
	if(GetBridgeImplementation())
		GetBridgeImplementation()->UpdateMediaType(partyRsrcID, eMediaTypeUpdate);
	else
		PASSERT(1);
}


/////////////////////////////////////////////////////////////////////////////
// speakerIndication - update
void CAudioBridgeInterface::UpdateUseSpeakerSsrcForTx(PartyRsrcID partyRsrcID, BOOL updatedUseSpeakerSsrcForTx)
{
	if(GetBridgeImplementation())
		GetBridgeImplementation()->UpdateUseSpeakerSsrcForTx(partyRsrcID, updatedUseSpeakerSsrcForTx);
	else
		PASSERT(1);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgeInterface::UpdateMute(PartyRsrcID partyRsrcID,EMediaDirection eMediaDirection, EOnOff eOnOff, WORD srcRequest)
{
	if(GetBridgeImplementation())
		GetBridgeImplementation()->UpdateMute(partyRsrcID, eMediaDirection, eOnOff, srcRequest);
	else
		PASSERT(1);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgeInterface::UpdateAudioVolume (PartyRsrcID partyRsrcID, EMediaDirection eMediaDirection, DWORD newAudioVolume)
{
	if(GetBridgeImplementation())
		GetBridgeImplementation()->UpdateAudioVolume(partyRsrcID, eMediaDirection, newAudioVolume);
	else
		PASSERT(1);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgeInterface::IncreaseAudioVolume (PartyRsrcID partyRsrcID, EMediaDirection eMediaDirection, BYTE increaseRate)
{
	if(GetBridgeImplementation())
		GetBridgeImplementation()->IncreaseAudioVolume(partyRsrcID, eMediaDirection, increaseRate);
	else
		PASSERT(1);	
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgeInterface::DecreaseAudioVolume (PartyRsrcID partyRsrcID, EMediaDirection eMediaDirection, BYTE decreaseRate)
{
	if(GetBridgeImplementation())
		GetBridgeImplementation()->DecreaseAudioVolume(partyRsrcID, eMediaDirection, decreaseRate);
	else
		PASSERT(1);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgeInterface::UpdateNoiseDetection(PartyRsrcID partyRsrcID, EOnOff eOnOff, BYTE newNoiseDetectionThreshold )
{
	if(GetBridgeImplementation())
		GetBridgeImplementation()->UpdateNoiseDetection(partyRsrcID, eOnOff, newNoiseDetectionThreshold);
	else
		PASSERT(1);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgeInterface::UpdateConfParams(WORD talkHoldTime,BYTE audioMixDepth)
{
	if(GetBridgeImplementation())
		GetBridgeImplementation()->UpdateConfParams(talkHoldTime, audioMixDepth);
	else
		PASSERT(1);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgeInterface::SpeakerChangeIndication(CSegment* pSpeakerChangeParams, DWORD dwSpeakerChangeOpcode)
{
	if(GetBridgeImplementation())
		GetBridgeImplementation()->SpeakerChangeIndication(pSpeakerChangeParams, dwSpeakerChangeOpcode);
	else
		PASSERT(1);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgeInterface::IvrPartyCommand(PartyRsrcID partyID, OPCODE opcode, DWORD seqNumToken, CSegment *pDataSeg)
{
	if(GetBridgeImplementation())
		GetBridgeImplementation()->IvrPartyCommand(partyID, opcode, seqNumToken, pDataSeg);
	else
		PASSERT(1);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgeInterface::IvrConfCommand(OPCODE opcode, DWORD seqNumToken, CSegment *pDataSeg)
{
	if(GetBridgeImplementation())
		GetBridgeImplementation()->IvrConfCommand(opcode, seqNumToken,  pDataSeg);
	else
		PASSERT(1);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgeInterface::IvrUpdatePartyStandalone(PartyRsrcID partyID, BOOL isStandalone)
{
	if(GetBridgeImplementation())
		GetBridgeImplementation()->IvrUpdatePartyStandalone(partyID, isStandalone);
	else
		PASSERT(1);
}

/////////////////////////////////////////////////////////////////////////////
bool CAudioBridgeInterface::UpdateAudioDelay(PartyRsrcID partyID, TAudioUpdateCompressedAudioDelayReq* pstAudioDelay)
{
	CAudioBridge* pAudioBridge = GetBridgeImplementation();
	if (pAudioBridge)
		return pAudioBridge->UpdateAudioDelay(partyID, pstAudioDelay);
	else
	{
		PASSERT(1);
		return false;
	}
}
/////////////////////////////////////////////////////////////////////////////

bool CAudioBridgeInterface::UpdateAudioRelayParamsIn(PartyRsrcID partyRsrcID, DWORD ssrc)
{
	CAudioBridge* pAudioBridge = GetBridgeImplementation();
	if (pAudioBridge)
		return pAudioBridge->UpdateAudioRelayParamsIn(partyRsrcID, ssrc);
	else
	{
		PASSERT(1);
		return false;
	}
}
/////////////////////////////////////////////////////////////////////////////
bool CAudioBridgeInterface::UpdateAudioRelayParamsOut(PartyRsrcID partyRsrcID, DWORD numOfSsrcIds, DWORD* ssrc_array,DWORD ivrSsrc)
{
	CAudioBridge* pAudioBridge = GetBridgeImplementation();
	if (pAudioBridge)
		return pAudioBridge->UpdateAudioRelayParamsOut(partyRsrcID, numOfSsrcIds, ssrc_array, ivrSsrc);
	else
	{
		PASSERT(1);
		return false;
	}
}
///////////////////////////////////////////////////////////////////////////////
void CAudioBridgeInterface::UpgradeToMixAvcSvc(PartyRsrcID partyRsrcID)
{
	CAudioBridge* pAudioBridge = GetBridgeImplementation();
	if (pAudioBridge)
		return pAudioBridge->UpgradeToMixAvcSvc(partyRsrcID);
	else
	{
		PASSERT(1);
	}
}

