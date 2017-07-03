//=================================================================================================
//
//Copyright (C) 2005 POLYCOM Networks Ltd.
//This file contains confidential information proprietary to POLYCOM Networks Ltd. The use or 
//disclosure of any information contained in this file without the written consent of an officer of
//POLYCOM Networks Ltd. is expressly forbidden.
//
//=================================================================================================

//=================================================================================================
//
//Module Name:  IvrApiStructures.h
//
//
//Generated By: Amir         Date: 22.06.05
//
//=================================================================================================

#ifndef __IVRSTRUCTURESAPI_H__
#define __IVRSTRUCTURESAPI_H__

#include "SharedDefines.h"
#include "PhysicalResource.h"


#define		MAX_RCV_DTMF		40

#define 	IVR_PLAY_MSG_TO_PARTY	1
#define 	IVR_PLAY_MSG_TO_CONF	2
#define 	IVR_PLAY_MSG_MAX		3

#define 	IVR_STOP_PREV_MSG		1
#define 	IVR_APPEND_MSG			2

#define 	IVR_MEDIA_TYPE_AUDIO			1	
#define 	IVR_MEDIA_TYPE_VIDEO			2
#define 	IVR_MEDIA_TYPE_AUDIO_AND_VIDEO	3	
#define 	IVR_MEDIA_TYPE_MAX	4

#define 	IVR_ACTION_TYPE_PLAY			1
#define 	IVR_ACTION_TYPE_RECORD			2
#define 	IVR_ACTION_TYPE_SILENCE			3
#define 	IVR_ACTION_TYPE_STOP_RECORDING	4
#define 	IVR_ACTION_TYPE_MAX             5

#define 	MAX_MUSIC_SOURCE_FILES			4		// max files in one music source

#define		PRIORITY_0_CACHE_IVR			0
#define		PRIORITY_1_CACHE_IVR			1
#define		PRIORITY_2_CACHE_IVR			2
#define		PRIORITY_3_CACHE_IVR			3
#define		PRIORITY_4_CACHE_IVR			4
#define		PRIORITY_5_CACHE_IVR			5
//~~~~~~~~~~~~~~~~~~~~~~~~~
// general subjects
//~~~~~~~~~~~~~~~~~~~~~~~~~
//		Plays Message
//		Plays Music
//		Shows Slide
//		Plays Tone
//		Sends / receives DTMF
//
//		Stop Msg
//		Stop Tone
//		Stop Music
//		Stop Slide
//		Stop DTMF forward
//~~~~~~~~~~~~~~~~~~~~~~~~~


///////////////////////////////////////////////
//			Play Message (party, conf) (audio, video, clip)
///////////////////////////////////////////////


typedef struct
{
	APIU32	privateIVRMsgVolume;
	APIU32	privateIVRMusicVolume;
	APIU32	confIVRMsgVolume;
	APIU32	confIVRMusicVolume;
	APIU32	encoderConfMixVolume;
	APIU32	decoderConfMixVolume;
} SIVRStartIVRParams;


typedef struct
{
	APIU32	actionType;			// play / record / silence
	APIU32	duration;			// for recording or silence
	APIU32	playMode;			// ?
	APIU32	frequentness;		// hint for leave it in memory
	APIU32	checksum;			// name+checksum defines the file
	APIU32	verNum;				// internal (for debug)
	APIU32	rsrv[2];			// reserve for future
	APIU32	fileNameLength;		// file full pathname length
	char	fileName[MAX_FULL_PATHNAME];		// file full pathname

} SIVRMediaFileParamsStruct;

typedef struct
{
	PHYSICAL_RESOURCE_INFO_S	physicalPortDescription;
	APIU32	partyOrconfFlag;	// command to party or conf
	APIU32	stopPrevOrAppend;	// stop current PlayMessage or append at the end
	APIU32	mediaType;			// media type: a/v/a+v
	APIU32	numOfRepetition;	// num of repetition of the set (X for forever)
	APIU32	startIVRFlag;		// 1 = IC control the Start-IVR / Stop-IVR
	SIVRStartIVRParams	startIVR;	// includes parameters for Start-IVR by IC
	APIU32   videoBitRate;		// (bits per second) relevant only for show slide req in all others will be set to INVALID
	APIU8	isTipMode;			// is TIP mode – TRUE/FALSE
	APIU8	rsrv[3];			// reserve for future
	APIU32	reserve;			// reserve for future
	APIU32	numOfMediaFiles;	// num of files to play
	SIVRMediaFileParamsStruct	*mediaFiles;	// media files set

} SIVRPlayMessageStruct;



///////////////////////////////////////////////
//			IVR Play Message Ack
///////////////////////////////////////////////
		
typedef struct
{
	APIU32	status;				// failed, succeeded or terminated by command
	APIU32	rsrv[2];			// reserve for future

} SIVRPlayMessageIndStruct;



///////////////////////////////////////////////
//			Record Roll-Call Indication
///////////////////////////////////////////////
		
typedef struct
{
	APIU32	status;				// failed, succeeded or terminated by command
	APIU32	recordingLength;	// length of Roll-Call recording in seconds
	APIU32	rsrv[2];			// reserve for future

} SIVRRecordMessageIndStruct;



///////////////////////////////////////////////
//			Play Music
///////////////////////////////////////////////
		
typedef struct
{
	PHYSICAL_RESOURCE_INFO_S	physicalPortDescription;
	APIU32	partyOrconfFlag;	// command to party or conf
	APIU32	musicSourceID;		// music source ID
	APIU32	rsrv[2];			// reserve for future
	char    fileName[MAX_FULL_PATHNAME];

} SIVRPlayMusicStruct;


///////////////////////////////////////////////
//			Stop Play Music
///////////////////////////////////////////////
		
typedef struct
{
	PHYSICAL_RESOURCE_INFO_S	physicalPortDescription;
	APIU32	partyOrconfFlag;	// command to party or conf
	APIU32	rsrv[2];			// reserve for future

} SIVRStopPlayMusicStruct;


///////////////////////////////////////////////
//			SetPath
///////////////////////////////////////////////
		
typedef struct
{
	APIU32	mediaType;			// audio / video / clip / icon
	APIU32	rsrv[2];			// reserve for future
	APIU32	pathNameLength;		// full pathname length
	char	pathName[MAX_FULL_PATHNAME];		// full pathname

} SIVRSetPathStruct;


///////////////////////////////////////////////
//			EraseFile
///////////////////////////////////////////////
		
typedef struct
{
	APIU32	mediaType;			// audio / video / clip / icon
	APIU32	rsrv[2];			// reserve for future
	APIU32	pathNameLength;		// full pathname length
	char	pathName[MAX_FULL_PATHNAME];		// full pathname

} SIVREraseFileStruct;


///////////////////////////////////////////////
//			Icon Load
///////////////////////////////////////////////
		
typedef struct
{
	APIU32	iconID;				// icon ID
	APIU32	iconType;			// slot type in Video Port (icon format, etc.)
	APIU32	slotNumber;			// slotNumber in Video Port
	APIU32	frequentness;		// hint for leave it in memory
	APIU32	rsrv[2];			// reserve for future
	APIU32	fileNameLength;		// file full pathname length
	char	fileName[MAX_FULL_PATHNAME];		// file full pathname

} SIVRIconLoadStruct;



///////////////////////////////////////////////
//			Icon Text
///////////////////////////////////////////////
		
typedef struct
{
	APIU32	iconID;				// icon ID
	APIU32	iconType;			// slot type in Video Port (icon format, etc.)
	APIU32	slotNumber;			// slotNumber in Video Port
	APIU32	textID;
	APIU32	textColor;
	APIU32	bgColor;
	APIU32	fontNameLength;		// font name length
	char	fontName[MAX_FULL_PATHNAME];
	APIU32	fontSize;
	APIU32	fontAttribute;
	APIU32	frequentness;		// hint for leave it in memory
	APIU32	rsrv[10];			// reserve for future
	APIU32	textLength;			// text length without NULL terminated character (MAX 1000)
	char	*text;				// text unicode string

} SIVRIconTextStruct;


///////////////////////////////////////////////
//			Icon Show
///////////////////////////////////////////////
		
typedef struct
{
	APIU32	iconID;				// icon ID
	APIU32	iconType;			// slot type in Video Port (icon format, etc.)
	APIU32	slotNumber;			// slotNumber in Video Port
	APIU32	x_order;			// x offset
	APIU32	y_order;			// y offset
	APIU32	z_order;			// z offset
	APIU32	lastX;				// last X (in case of motion)
	APIU32	lastY;				// last Y (in case of motion)
	APIU32	speed;				// speed in case of motion
	APIU32	opacity;			// opacity to see the covered area
	APIU32	numOfRepetition;	// number of repetition for animation (X forever)
	APIU32	rsrv[2];			// reserve for future

} SIVROneIconStruct;


typedef struct
{
	APIU32	numOfIcons;			// number of icons to show
	SIVROneIconStruct *icons;	// list of icons to show

} SIVRIconShowStruct;


typedef struct
{
	APIU32	sourceID;			// Source ID to be used by Play Music
	APIU32	mediaFilesNum;		// number of aca files
	char	mediaFiles[MAX_MUSIC_SOURCE_FILES][MAX_FULL_PATHNAME];	// Music files list

} SIVRAddMusicSource;


typedef struct
{
	PHYSICAL_RESOURCE_INFO_S	physicalPortDescription;
	APIU32	partyOrconfFlag;	// command to party or conf
	APIU32	mediaType;			// media type: a/v/a+v
	SIVRStartIVRParams params;	// parameters for Start-IVR
	APIU32	rsrv[2];			// reserve for future
} SIVRStartIVRStruct;


///////////////////////////////////////////////
//			Stop IVR State
///////////////////////////////////////////////
		
typedef struct
{
	PHYSICAL_RESOURCE_INFO_S	physicalPortDescription;
	APIU32	partyOrconfFlag;	// command to party or conf
	APIU32	mediaType;			// media type: a/v/a+v
	APIU32	rsrv[2];			// reserve for future

} SIVRStopIVRStruct, SIVRStopPlayMessage;


///////////////////////////////////////////////
//			Stop Recording
///////////////////////////////////////////////

typedef struct
{
	PHYSICAL_RESOURCE_INFO_S	physicalPortDescription;

} SIVRStopRecordStruct;


#endif	// __IVRSTRUCTURESAPI_H__ 
