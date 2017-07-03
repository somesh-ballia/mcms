//+========================================================================+
//                    UpdatePartyAudioInitParams.cpp                            |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       UpdatePartyAudioInitParams.cpp                                   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Inga                                                        |
//-------------------------------------------------------------------------|
// Who  | Date  June-2005  | Description                                   |
//-------------------------------------------------------------------------|
//			
//+========================================================================+


#include "UpdatePartyAudioInitParams.h"
#include "ConfPartyApiDefines.h"
// ------------------------------------------------------------
CUpdatePartyAudioInitParams::CUpdatePartyAudioInitParams()
{
	
}
// ------------------------------------------------------------
CUpdatePartyAudioInitParams::CUpdatePartyAudioInitParams(const CBridgePartyInitParams& PartyInitParams)
:CUpdatePartyInitParams(PartyInitParams)
{
	if(PartyInitParams.GetMediaInParams())
	{
		m_UpdateInParams = new CBridgePartyAudioInParams();
		((CBridgePartyAudioInParams*)m_UpdateInParams)->InitPartyAudioInParams((CBridgePartyAudioParams*)PartyInitParams.GetMediaInParams());
	}	
		
	if(PartyInitParams.GetMediaOutParams())	
	{
		m_UpdateOutParams = new CBridgePartyAudioOutParams();
		((CBridgePartyAudioOutParams*)m_UpdateOutParams)->InitPartyAudioOutParams((CBridgePartyAudioParams*)PartyInitParams.GetMediaOutParams());
	}

}
// ------------------------------------------------------------
CUpdatePartyAudioInitParams::~CUpdatePartyAudioInitParams ()
{	
	
	
}
// ------------------------------------------------------------
void CUpdatePartyAudioInitParams::AllocateInParams()
{
	if(m_UpdateInParams==NULL)
		m_UpdateInParams = new CBridgePartyAudioInParams;
}
// ------------------------------------------------------------
void CUpdatePartyAudioInitParams::AllocateOutParams()
{
	if(m_UpdateOutParams==NULL)
		m_UpdateOutParams = new CBridgePartyAudioOutParams;
}
// ------------------------------------------------------------
void CUpdatePartyAudioInitParams::InitiateMediaInParams(const CBridgePartyAudioParams* pBridgePartyMediaParams)
{
	AllocateInParams();
	
	((CBridgePartyAudioInParams*)m_UpdateInParams)->InitPartyAudioInParams(pBridgePartyMediaParams);
}
// ------------------------------------------------------------
void CUpdatePartyAudioInitParams::InitiateMediaOutParams(const CBridgePartyAudioParams* pBridgePartyMediaParams)
{
	AllocateOutParams();
	
	((CBridgePartyAudioOutParams*)m_UpdateOutParams)->InitPartyAudioOutParams(pBridgePartyMediaParams);
}
// ------------------------------------------------------------
void CUpdatePartyAudioInitParams::UpdateInitAudioAlgorithm(EMediaDirection eMediaDirection,DWORD newAudioAlgorithm, DWORD maxAverageBitrate)
{
	PTRACE2(eLevelInfoNormal, "CUpdatePartyAudioInitParams::UpdateInitAudioAlgorithm  - Save the new audio algorithm: ", m_pPartyName);    
	
	switch (eMediaDirection) {
	case eMediaIn: {
		if (CPObject::IsValidPObjectPtr(m_UpdateInParams))
		{
			((CBridgePartyAudioParams*)m_UpdateInParams)->SetAudioAlgorithm(newAudioAlgorithm);
			((CBridgePartyAudioParams*)m_UpdateInParams)->SetMaxAverageBitrate(maxAverageBitrate);
		}
		else
			PTRACE2(eLevelInfoNormal, "CUpdatePartyAudioInitParams::UpdateInitAudioAlgorithm : FAILED!!! m_UpdateInParams is invalid : ", m_pPartyName);    

		break;
					}
	case eMediaOut: {
		if (CPObject::IsValidPObjectPtr(m_UpdateInParams))
		{
			((CBridgePartyAudioParams*)m_UpdateInParams)->SetAudioAlgorithm(newAudioAlgorithm);
			((CBridgePartyAudioParams*)m_UpdateInParams)->SetMaxAverageBitrate(maxAverageBitrate);
		}
		else
			PTRACE2(eLevelInfoNormal, "CUpdatePartyAudioInitParams::UpdateInitAudioAlgorithm : FAILED!!! m_UpdateOutParams is invalid : ", m_pPartyName);    
				
		break;
					}
	case eMediaInAndOut: {
		if (CPObject::IsValidPObjectPtr(m_UpdateInParams))
		{
			((CBridgePartyAudioParams*)m_UpdateInParams)->SetAudioAlgorithm(newAudioAlgorithm);
			((CBridgePartyAudioParams*)m_UpdateInParams)->SetMaxAverageBitrate(maxAverageBitrate);
		}
		else
			PTRACE2(eLevelInfoNormal, "CUpdatePartyAudioInitParams::UpdateInitAudioAlgorithm : FAILED!!! m_UpdateInParams is invalid : ", m_pPartyName);    

		if (CPObject::IsValidPObjectPtr(m_UpdateInParams))
		{
			((CBridgePartyAudioParams*)m_UpdateInParams)->SetAudioAlgorithm(newAudioAlgorithm);
			((CBridgePartyAudioParams*)m_UpdateInParams)->SetMaxAverageBitrate(maxAverageBitrate);
		}
		else
			PTRACE2(eLevelInfoNormal, "CUpdatePartyAudioInitParams::UpdateInitAudioAlgorithm: FAILED!!! m_UpdateOutParams is invalid : ", m_pPartyName);    
				
		break;
						 }
	default: {
		DBGPASSERT (1);
			 }
	}
}

// ------------------------------------------------------------
void CUpdatePartyAudioInitParams::UpdateMediaInType(EMediaTypeUpdate eMediaTypeUpdate)
{
	TRACEINTO <<  " NewType is " <<  EMediaTypeUpdateNames[eMediaTypeUpdate]  << "  for party " <<  m_pPartyName ;

	if (eMediaTypeUpdateNotNeeded == eMediaTypeUpdate)
		return;

	BOOL isVideoParticipant = FALSE;

	if (eMediaTypeUpdateAudioToVideo == eMediaTypeUpdate)
		isVideoParticipant = TRUE;

	else if (eMediaTypeUpdateVideoToAudio == eMediaTypeUpdate)
		isVideoParticipant = FALSE;

	if (CPObject::IsValidPObjectPtr(m_UpdateInParams))
		((CBridgePartyAudioParams*)m_UpdateInParams)->SetVideoParticipant(isVideoParticipant);

}
// ------------------------------------------------------------
void CUpdatePartyAudioInitParams::UpdateInitMute(EMediaDirection eMediaDirection,EOnOff eOnOff, WORD srcRequest)
{	
	PTRACE2(eLevelInfoNormal, "CUpdatePartyAudioInitParams::UpdateInitMute  - Save the Mute param: ", m_pPartyName);    
	
	if ( (eOff != eOnOff) && (eOn != eOnOff) ) {
		DBGPASSERT_AND_RETURN(1);
	}
	
	switch (eMediaDirection) {
	case eMediaIn: {
		if (CPObject::IsValidPObjectPtr(m_UpdateInParams))
			((CBridgePartyAudioParams*)m_UpdateInParams)->SetMute(eOnOff, srcRequest);
		else
			PTRACE2(eLevelInfoNormal, "CUpdatePartyAudioInitParams::UpdateInitMute : FAILED!!! m_UpdateInParams is invalid : ", m_pPartyName);    
		break;
				   }
	case eMediaOut: {
		if (CPObject::IsValidPObjectPtr(m_UpdateOutParams))
			((CBridgePartyAudioParams*)m_UpdateOutParams)->SetMute(eOnOff, srcRequest);
		else
			PTRACE2(eLevelInfoNormal, "CUpdatePartyAudioInitParams::UpdateInitMute : FAILED!!! m_UpdateOutParams is invalid : ", m_pPartyName);    
		break;
					}
	case eMediaInAndOut: {
		if (CPObject::IsValidPObjectPtr(m_UpdateInParams))
			((CBridgePartyAudioParams*)m_UpdateInParams)->SetMute(eOnOff, srcRequest);
		else
			PTRACE2(eLevelInfoNormal, "CUpdatePartyAudioInitParams::UpdateInitMute : FAILED!!! m_UpdateInParams is invalid : ", m_pPartyName);    

		if (CPObject::IsValidPObjectPtr(m_UpdateOutParams))
			((CBridgePartyAudioParams*)m_UpdateOutParams)->SetMute(eOnOff, srcRequest);
		else
			PTRACE2(eLevelInfoNormal, "CUpdatePartyAudioInitParams::UpdateInitMute : FAILED!!! m_UpdateOutParams is invalid : ", m_pPartyName);    

		break;
						 }
	default: {
		DBGPASSERT (1);
			 }
	}
}
// ------------------------------------------------------------
void CUpdatePartyAudioInitParams::UpdateInitVolume(EMediaDirection eMediaDirection,DWORD newAudioVolume)
{
	
	PTRACE2(eLevelInfoNormal, "CUpdatePartyAudioInitParams::UpdateInitVolume  - Save the new Volume param: ", m_pPartyName);    
	
	if ( ! ::IsValidAudioVolume(newAudioVolume) ) {
		DBGPASSERT_AND_RETURN(1);
	}
		
	switch (eMediaDirection) {
	case eMediaIn: {
		if (CPObject::IsValidPObjectPtr(m_UpdateInParams))
			((CBridgePartyAudioParams*)m_UpdateInParams)->SetVolume(newAudioVolume);
		else
			PTRACE2(eLevelInfoNormal, "CUpdatePartyAudioInitParams::UpdateInitVolume : FAILED!!! m_UpdateInParams is invalid : ", m_pPartyName);    
		
		break;
				   }
   case eMediaOut: {
		if (CPObject::IsValidPObjectPtr(m_UpdateOutParams))
			((CBridgePartyAudioParams*)m_UpdateOutParams)->SetVolume(newAudioVolume);
		else
			PTRACE2(eLevelInfoNormal, "CUpdatePartyAudioInitParams::UpdateInitVolume : FAILED!!! m_UpdateOutParams is invalid : ", m_pPartyName);    
		
		break;
				   }
   case eMediaInAndOut: {
		if (CPObject::IsValidPObjectPtr(m_UpdateInParams))
			((CBridgePartyAudioParams*)m_UpdateInParams)->SetVolume(newAudioVolume);
		else
			PTRACE2(eLevelInfoNormal, "CUpdatePartyAudioInitParams::UpdateInitVolume : FAILED!!! m_UpdateInParams is invalid : ", m_pPartyName);    
		
		if (CPObject::IsValidPObjectPtr(m_UpdateOutParams))
			((CBridgePartyAudioParams*)m_UpdateOutParams)->SetVolume(newAudioVolume);
		else
			PTRACE2(eLevelInfoNormal, "CUpdatePartyAudioInitParams::UpdateInitVolume : FAILED!!! m_UpdateOutParams is invalid : ", m_pPartyName);    
			
		break;
						 }
	default: {
		DBGPASSERT (1);
			 }
	}			   
}

// ------------------------------------------------------------
void CUpdatePartyAudioInitParams::IncreaseInitVolume(EMediaDirection eMediaDirection,BYTE increaseRate)			
{			
	
	PTRACE2(eLevelInfoNormal, "CUpdatePartyAudioInitParams::IncreaseInitVolume - Save the Increase Volume param: ", m_pPartyName);    
		
	WORD newAudioVolume;
	
	switch (eMediaDirection) {
	case eMediaIn: {
		if (CPObject::IsValidPObjectPtr(m_UpdateInParams))
		{
			newAudioVolume = ((CBridgePartyAudioParams*)m_UpdateInParams)->GetAudioVolume() + increaseRate;
			
			if (newAudioVolume > AUDIO_VOLUME_MAX)
				newAudioVolume = AUDIO_VOLUME_MAX;
			
			((CBridgePartyAudioParams*)m_UpdateInParams)->SetVolume(newAudioVolume);
		}
		else
			PTRACE2(eLevelInfoNormal, "CUpdatePartyAudioInitParams::IncreaseInitVolume  : FAILED!!! m_UpdateInParams is invalid : ", m_pPartyName);    

		break;
				   }
	case eMediaOut: {
		if (CPObject::IsValidPObjectPtr(m_UpdateOutParams))
		{
			newAudioVolume = ((CBridgePartyAudioParams*)m_UpdateOutParams)->GetAudioVolume() + increaseRate;
			
			if (newAudioVolume > AUDIO_VOLUME_MAX)
				newAudioVolume = AUDIO_VOLUME_MAX;
			
			((CBridgePartyAudioParams*)m_UpdateOutParams)->SetVolume(newAudioVolume);
		}
		else
			PTRACE2(eLevelInfoNormal, "CUpdatePartyAudioInitParams::IncreaseInitVolume  : FAILED!!! m_UpdateOutParams is invalid : ", m_pPartyName);    

		break;
					}
	case eMediaInAndOut: {
		if (CPObject::IsValidPObjectPtr(m_UpdateInParams))
		{
			newAudioVolume = ((CBridgePartyAudioParams*)m_UpdateInParams)->GetAudioVolume() + increaseRate;
			
			if (newAudioVolume > AUDIO_VOLUME_MAX)
				newAudioVolume = AUDIO_VOLUME_MAX;
			
			((CBridgePartyAudioParams*)m_UpdateInParams)->SetVolume(newAudioVolume);
		}
		else
			PTRACE2(eLevelInfoNormal, "CUpdatePartyAudioInitParams::IncreaseInitVolume  : FAILED!!! m_UpdateInParams is invalid : ", m_pPartyName);    

		if (CPObject::IsValidPObjectPtr(m_UpdateOutParams))
		{
			newAudioVolume = ((CBridgePartyAudioParams*)m_UpdateOutParams)->GetAudioVolume() + increaseRate;
			
			if (newAudioVolume > AUDIO_VOLUME_MAX)
				newAudioVolume = AUDIO_VOLUME_MAX;
		
			((CBridgePartyAudioParams*)m_UpdateOutParams)->SetVolume(newAudioVolume);
		}
		else
			PTRACE2(eLevelInfoNormal, "CUpdatePartyAudioInitParams::IncreaseInitVolume  : FAILED!!! m_UpdateOutParams is invalid : ", m_pPartyName);    
		
		break;
						 }
	default: {
		DBGPASSERT (1);
			 }
	}	

}
// ------------------------------------------------------------
void CUpdatePartyAudioInitParams::DecreaseInitVolume(EMediaDirection eMediaDirection,BYTE decreaseRate)
{
	PTRACE2(eLevelInfoNormal, "CUpdatePartyAudioInitParams::DecreaseInitVolume - Save the Decreased Volume param: ", m_pPartyName);    
	
	WORD newAudioVolume;
	
	switch (eMediaDirection) {
	case eMediaIn: {
		if (CPObject::IsValidPObjectPtr(m_UpdateInParams))
		{
			if(decreaseRate > ((CBridgePartyAudioParams*)m_UpdateInParams)->GetAudioVolume())
				newAudioVolume = AUDIO_VOLUME_MIN;
			else
				newAudioVolume = ((CBridgePartyAudioParams*)m_UpdateInParams)->GetAudioVolume() - decreaseRate;
			
			// Block values under AUDIO_VOLUME_MIN
			if(newAudioVolume < AUDIO_VOLUME_MIN)
	  			return;
			
			((CBridgePartyAudioParams*)m_UpdateInParams)->SetVolume(newAudioVolume);
		}
		else
			PTRACE2(eLevelInfoNormal, "CUpdatePartyAudioInitParams::DecreaseInitVolume : FAILED!!! m_UpdateInParams is invalid : ", m_pPartyName);    

		break;
				   }
	case eMediaOut: {
		if (CPObject::IsValidPObjectPtr(m_UpdateOutParams))
		{
			if(decreaseRate > ((CBridgePartyAudioParams*)m_UpdateOutParams)->GetAudioVolume())
				newAudioVolume = AUDIO_VOLUME_MIN;
			else
				newAudioVolume = ((CBridgePartyAudioParams*)m_UpdateOutParams)->GetAudioVolume() - decreaseRate;
			
			// Block values under AUDIO_VOLUME_MIN
			if(newAudioVolume < AUDIO_VOLUME_MIN)
	  			return;
			
			((CBridgePartyAudioParams*)m_UpdateOutParams)->SetVolume(newAudioVolume);
		}
		else
			PTRACE2(eLevelInfoNormal, "CUpdatePartyAudioInitParams::DecreaseInitVolume : FAILED!!! m_UpdateOutParams is invalid : ", m_pPartyName);    

		
		break;
					}
	case eMediaInAndOut: {
		if (CPObject::IsValidPObjectPtr(m_UpdateInParams))
		{
			if(decreaseRate > ((CBridgePartyAudioParams*)m_UpdateInParams)->GetAudioVolume())
				newAudioVolume = AUDIO_VOLUME_MIN;
			else
				newAudioVolume = ((CBridgePartyAudioParams*)m_UpdateInParams)->GetAudioVolume() - decreaseRate;
			
			// Block values under AUDIO_VOLUME_MIN
			if(newAudioVolume < AUDIO_VOLUME_MIN)
	  			return;
			
			((CBridgePartyAudioParams*)m_UpdateInParams)->SetVolume(newAudioVolume);
		}
		else
			PTRACE2(eLevelInfoNormal, "CUpdatePartyAudioInitParams::DecreaseInitVolume : FAILED!!! m_UpdateInParams is invalid : ", m_pPartyName);    

		if (CPObject::IsValidPObjectPtr(m_UpdateOutParams))
		{
			if(decreaseRate > ((CBridgePartyAudioParams*)m_UpdateOutParams)->GetAudioVolume())
				newAudioVolume = AUDIO_VOLUME_MIN;
			else
				newAudioVolume = ((CBridgePartyAudioParams*)m_UpdateOutParams)->GetAudioVolume() - decreaseRate;
			
			// Block values under AUDIO_VOLUME_MIN
			if(newAudioVolume < AUDIO_VOLUME_MIN)
	  			return;
			
			((CBridgePartyAudioParams*)m_UpdateOutParams)->SetVolume(newAudioVolume);
		}
		else
			PTRACE2(eLevelInfoNormal, "CUpdatePartyAudioInitParams::DecreaseInitVolume : FAILED!!! m_UpdateOutParams is invalid : ", m_pPartyName);    

		break;
						 }
	default: {
		DBGPASSERT (1);
			 }
	}
}
// ------------------------------------------------------------
void CUpdatePartyAudioInitParams::UpdateInitNoiseDetection(EOnOff eOnOff, BYTE newNoiseDetectionThreshold)
{
	PTRACE2(eLevelInfoNormal, "CUpdatePartyAudioInitParams::UpdateInitNoiseDetection - Save the Noise Detection param: ", m_pPartyName);   

	BOOL newIsNoiseDetection = (eOnOff==eOn) ? TRUE : FALSE; 
	
	if ( ! ::IsValidNoiseDetectionThreshold(newNoiseDetectionThreshold) ) {
		DBGPASSERT_AND_RETURN(1);
	}
		
	if (CPObject::IsValidPObjectPtr(m_UpdateInParams))
	{
		m_pConfAppBridgeParams->SetNoiseDetection(newIsNoiseDetection);
		m_pConfAppBridgeParams->SetNoiseDetectionThreshold(newNoiseDetectionThreshold);		
	}
	else
		PTRACE2(eLevelInfoNormal, "CUpdatePartyAudioInitParams::UpdateInitNoiseDetection : FAILED!!! m_UpdateInParams is invalid : ", m_pPartyName);    
	
}
// ------------------------------------------------------------
void CUpdatePartyAudioInitParams::UpdateInitAGC(EOnOff eOnOff)
{	
	PTRACE2(eLevelInfoNormal, "CUpdatePartyAudioInitParams::UpdateInitAGC - Save the AGC param: ", m_pPartyName);   
	
	BOOL newIsAgc = (eOnOff==eOn) ? TRUE : FALSE;
	
	if (CPObject::IsValidPObjectPtr(m_UpdateInParams))
		((CBridgePartyAudioInParams*)m_UpdateInParams)->SetAGC(newIsAgc);
	else
		PTRACE2(eLevelInfoNormal, "CUpdatePartyAudioInitParams::UpdateInitAGC : FAILED!!! m_UpdateInParams is invalid : ", m_pPartyName);    
	
}

// ------------------------------------------------------------
void CUpdatePartyAudioInitParams::UpdateUseSpeakerSsrcForTx(BOOL useSpeakerSsrcForTx)
{
	TRACEINTO << "Save the UseSpeakerSsrcForTx param: " <<  m_pPartyName;

	if (CPObject::IsValidPObjectPtr(m_UpdateOutParams))
		((CBridgePartyAudioOutParams*)m_UpdateOutParams)->SetUseSpeakerSsrcForTx(useSpeakerSsrcForTx);
	else
		TRACEINTO << "FAILED!!! m_UpdateOutParams is invalid : " <<  m_pPartyName;

}
