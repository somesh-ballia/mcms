/*
 * TipUtils.cpp
 *
 *  Created on: Feb 21, 2011
 *      Author: shmuell
 */
#include "TipUtils.h"
/////////////////////////////////////////////////////////////////////////////
const char* GetTipAudioPositionStr(ETipAudioPosition audioPos)
{
	if (audioPos==eTipAudioPosCenter)
		return "AudioCenter";
	else if (audioPos==eTipAudioPosLeft)
		return "AudioLeft";
	else if (audioPos==eTipAudioPosRight)
		return "AudioRight";
	else if (audioPos==eTipAudioPosAux)
		return "AudioAux";
	else if (audioPos==eTipAudioPosLegacy)
		return "AudioLegacy";
	else if (audioPos==eTipAudioPosLast)
		return "AudioLast";

	return "UnknownAudioPosition";
}
/////////////////////////////////////////////////////////////////////////////
const char* GetTipVideoPositionStr(ETipVideoPosition videoPos)
{
	if (videoPos==eTipVideoPosCenter)
		return "VideoCenter";
	else if (videoPos==eTipVideoPosLeft)
		return "VideoLeft";
	else if (videoPos==eTipVideoPosRight)
		return "VideoRight";
	else if (videoPos==eTipVideoPosAux5Fps)
		return "VideoAux5Fps";
	else if (videoPos==eTipVideoPosAux30Fps)
		return "VideoAux30Fps";
	else if (videoPos==eTipVideoPosLegacyCenter)
		return "VideoLegacyCenter";
	else if (videoPos==eTipVideoPosLegacyLeft)
		return "VideoLegacyLeft";
	else if (videoPos==eTipVideoPosLegacyRight)
		return "VideoLegacyRight";
	else if (videoPos==eTipVideoPosLast)
		return "VideoLast";

	return "UnknownPosition";
}
/////////////////////////////////////////////////////////////////////////////
const char* GetTipAuxFPSStr(ETipAuxFPS auxFPS)
{
	if (auxFPS==eTipAux1FPS)
		return "Aux 1FPS";
	else if (auxFPS==eTipAux5FPS)
		return "Aux 5FPS";
	else if (auxFPS==eTipAux30FPS)
		return "Aux 30FPS";
	else if (auxFPS==eTipAuxNone)
		return "Aux None";
	return "Unknown Aux";
}
/////////////////////////////////////////////////////////////////////////////
ETipPartyTypeAndPosition GetPartyPosition(ETipVideoPosition videoPosition)
{
	switch (videoPosition)
	{
		case eTipVideoPosCenter:
		case eTipVideoPosAux5Fps:
		case eTipVideoPosAux30Fps:
			return eTipMasterCenter;
		case eTipVideoPosLeft:
			return eTipSlaveLeft;
		case eTipVideoPosRight:
			return eTipSlaveRigth;
		case eTipVideoPosLegacyCenter:
		case eTipVideoPosLegacyLeft:
		case eTipVideoPosLegacyRight:
		default:
			return eTipNone;
	}
}
/////////////////////////////////////////////////////////////////////////////
ETipVideoPosition GetVideoPosition(ETipPartyTypeAndPosition partyPosition)
{
	switch (partyPosition)
	{
		case eTipMasterCenter:
			return eTipVideoPosCenter;
		case eTipSlaveLeft:
			return eTipVideoPosLeft;
		case eTipSlaveRigth:
			return eTipVideoPosRight;
		default:
			return eTipVideoPosLast;
	}
}
/////////////////////////////////////////////////////////////////////////////
const char* GetTipAuxPositionStr(ETipAuxPosition auxPos)
{
	if (auxPos==eTipAuxPosition1or5FPS)
		return "1or5FPS";
	else if (auxPos==eTipAuxPosition30FPS)
		return "30FPS";
	else if (auxPos==eTipAuxPositionNone)
		return "None";
	else if (auxPos==eTipAuxPositionLast)
		return "eTipAuxPositionLast";
	return "UnknownPosition";
}
/////////////////////////////////////////////////////////////////////////////
const char* GetAuxCntlSubopcodeStr(EAuxCntlSubOpcode auxSubopcode)
{
	if (auxSubopcode==AuxCntlRequest)
		return "Request";
	else if (auxSubopcode==AuxCntlRequestGranted)
		return "Request Granted";
	else if (auxSubopcode==AuxCntlRequestDenied)
		return "Request Denied";
	else if (auxSubopcode==AuxCntlRelease)
		return "Release";
	else if (auxSubopcode==AuxCntlReleaseAck)
		return "Release Ack";
	else if (auxSubopcode==AuxCntlMsgUnknown)
		return "Message Unknown";
	return "Unknown";
}
