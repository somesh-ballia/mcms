//+========================================================================+
//                   AudioBridgeInitParams.CPP                             |
//					 Copyright 2005 Polycom, Inc                           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// FILE:       AudioBridgeInitParams.CPP                                   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Matvey                                                      |
//-------------------------------------------------------------------------|
// Who  | Date  Aug-2005  | Description                                    |
//-------------------------------------------------------------------------|


#include "AudioBridgeInitParams.h"


// ------------------------------------------------------------
CAudioBridgeInitParams::CAudioBridgeInitParams():
	CBridgeInitParams(),m_wTalkHoldTime(0),m_byAudioMixDepth(0),m_isAutoMuteNoisyParties(FALSE)
{

}

// ------------------------------------------------------------
CAudioBridgeInitParams::CAudioBridgeInitParams (const CConf* pConf, const char*	pConfName,ConfRsrcID confRsrcId,
												const EBridgeImplementationTypes eBridgeImplementationType,
												WORD talkHoldTime, BYTE audioMixDepth, BOOL autoMuteNoisyParties):
	CBridgeInitParams(pConf, pConfName,confRsrcId, eBridgeImplementationType),
	m_wTalkHoldTime(talkHoldTime), m_byAudioMixDepth(audioMixDepth), m_isAutoMuteNoisyParties(autoMuteNoisyParties)
{
	
}

// ------------------------------------------------------------
CAudioBridgeInitParams::~CAudioBridgeInitParams ()
{
	
}




