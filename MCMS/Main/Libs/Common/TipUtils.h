/*
 * TipUtils.h
 *
 *  Created on: Feb 21, 2011
 *      Author: shmuell
 */

#ifndef TIPUTILS_H_
#define TIPUTILS_H_

#include "TipStructs.h"

typedef enum
{
	eTipModeNone = 0,
	eTipModePossible,
	eTipModeNegotiated,
} ETipMode;

const char* GetTipAudioPositionStr(ETipAudioPosition audioPos);
const char* GetTipVideoPositionStr(ETipVideoPosition videoPos);
const char* GetTipAuxFPSStr(ETipAuxFPS auxFPS);
ETipPartyTypeAndPosition GetPartyPosition(ETipVideoPosition videoPosition);
ETipVideoPosition GetVideoPosition(ETipPartyTypeAndPosition partyPosition);
const char* GetTipAuxPositionStr(ETipAuxPosition auxPos);
const char* GetAuxCntlSubopcodeStr(EAuxCntlSubOpcode auxSubopcode);
#endif /* TIPUTILS_H_ */
