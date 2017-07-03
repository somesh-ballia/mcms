//+========================================================================+
//                   AudioBridgeInitParams.H                               |
//					 Copyright 2005 Polycom, Inc                           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// FILE:       AudioBridgeInitParams.H                                     |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Matvey                                                      |
//-------------------------------------------------------------------------|
// Who  | Date  Aug-2005  | Description                                    |
//-------------------------------------------------------------------------|

#ifndef _AUDIO_BRIDGE_INIT_PARAMS_
#define _AUDIO_BRIDGE_INIT_PARAMS_

#include "BridgeInitParams.h"

class CConf;
  
class CAudioBridgeInitParams : public CBridgeInitParams 
{
CLASS_TYPE_1(CAudioBridgeInitParams,CBridgeInitParams)
public:
	CAudioBridgeInitParams (const CConf* pConf, const char*	pConfName,ConfRsrcID confRsrcId,
							const EBridgeImplementationTypes eBridgeImplementationType,
							WORD talkHoldTime, BYTE audioMixDepth, BOOL autoMuteNoisyParties);
	virtual const char* NameOf() const { return "CAudioBridgeInitParams";}
	CAudioBridgeInitParams ();
	virtual ~CAudioBridgeInitParams ();

	void SetTalkHoldTime(WORD talkHoldTime)		{ m_wTalkHoldTime = talkHoldTime; }
	void SetAudioMixDepth(BYTE audioMixDepth)	{ m_byAudioMixDepth = audioMixDepth; }
	void SetAutoMuteNoisyParties(BOOL autoMuteNoisyParties)	{ m_isAutoMuteNoisyParties = autoMuteNoisyParties; }

	WORD GetTalkHoldTime()	const	{ return m_wTalkHoldTime; }
	BYTE GetAudioMixDepth()	const	{ return m_byAudioMixDepth;}
	BOOL GetAutoMuteNoisyParties()	const	{ return m_isAutoMuteNoisyParties;}

private:
	CAudioBridgeInitParams (const CAudioBridgeInitParams&);
	CAudioBridgeInitParams& operator = (const CAudioBridgeInitParams);

	WORD m_wTalkHoldTime;
	BYTE m_byAudioMixDepth;
	BOOL m_isAutoMuteNoisyParties;
};

#endif //_AUDIO_BRIDGE_INIT_PARAMS_


