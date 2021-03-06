/*
********************************************************************************
*
* Copyright (C) 2003 Polycom Israel Ltd.
* This file contains confidential information proprietary to POLYCOM ISRAEL Ltd.
* The use or disclosure of any information contained In this file without the
* written consent of an officer of POLYCOM ISRAEL Ltd is expressly forbidden.
*
********************************************************************************

*****************************************************************************

 General Description:

 Generated By: Eyal Leviav      Date: 8/18/2005

*****************************************************************************/

#ifndef __AUDIO_CTRL_INDICATION_STRUCTURES_H
#define __AUDIO_CTRL_INDICATION_STRUCTURES_H

/***** Include Files *****/

/***** Constants *****/

/***** Macros *****/

/***** Type Definition *****/

#define MAX_ACTIVE_SPEAKER_LIST	4
////////////////////////////////////
//Video and Audio speaker indication
//
////////////////////////////////////
typedef struct {
	//Conf ID is indication header
	APIU32 unVideoPartyID;
    	APIU32 unAudioPartyID;
	APIU32 unActiveSpeakerList[MAX_ACTIVE_SPEAKER_LIST];	// 0 indicate no speaker at that cell
	APIU32 unDominantSpeakerMsi;
} TAcActiveSpeakersInd;


//////////////////////////////////////
////Audio speaker indication
////
//////////////////////////////////////
//typedef struct {
//	//Conf ID is indication header
//	APIU32 unAudioPartyID;
//    APIU32 unVideoPartyID;
//
//} TAcAudioSpeakerInd;

typedef union
{
	TAcActiveSpeakersInd	tAcActiveSpeakersInd;
//	TAcAudioSpeakerInd		tAcAudioSpeakerInd;

} TAcIndicationPayload;

#define            SOURCE_NONE_DOMINANT_SPEAKER_MSI               (0xFFFFFFFF)
#define            SOURCE_ANY_DOMINANT_SPEAKER_MSI                (0xFFFFFFFE)
#define            DUMMY_DOMINANT_SPEAKER_MSI                      (0xFFFFFFF0)

/***** Public Global Variables *****/

/***** Public Functions Prototypes *****/


#endif


