//+========================================================================+
//            Copyright 2005 Polycom Networking Ltd.                       |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Networking Ltd. and is protected by law.         |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       ConfAppFeatureObject.cpp  	                               |
// SUBSYSTEM:  ConfParty                                                   |
//+========================================================================+


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

#include "StateMachine.h"
#include "StatusesGeneral.h"
#include "IvrApiCommandCreator.h"
#include "IvrApiStructures.h"

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
// ##############################################################
// ################									#############
// ################		CConfAppFeatureObject		#############
// ################									#############
// ##############################################################
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
int CIvrApiCommandCreator::PreparePlayMessageBase()
{
	return 0;
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
int CIvrApiCommandCreator::PreparePlayMessageConf()
{
	return 0;
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
int CIvrApiCommandCreator::PreparePlayMessageParty( SIVRPlayMessageStruct* pm )
{
	//pm->partyOrconfFlag		= PARTY;		// command to party or conf
	//pm->partyOrConfID		= partyID;		// party or conf ID
	//pm->stopPrevOrAppend	= MODE_APPEND;	// stop current PlayMessage or append at the end
	//pm->mediaType			= IVR_MEDIA_TYPE_AUDIO;			// media type: a/v/a+v
	pm->numOfRepetition		= 0;			// num of repetition of the set (X for forever)
//	pm->maxRecordTime		= 0;			// in Hsyncs (4-5 seconds)
//  pm->meanOfRecordEnd		= END_RECORD_ROLL_CALL_TIME;	// can be ended by dtmf, end time or by vad.
//	pm->vadRecordSilenceTime		= 1;			// the quiet period of time that the vad will wait for end record recognition.
//  pm->startRecordTimeTone	= 1;			// tone at beginning of recording 
//  pm->endRecordTimeTone	= 1;			// tone at end of recording
	pm->isTipMode			= FALSE;		// is TIP mode
	pm->rsrv[0]				= 0;			// reserve for future
	pm->rsrv[1]				= 0;			// reserve for future
	pm->rsrv[2]				= 0;			// reserve for future
	pm->reserve				= 0;			// reserve for future
	pm->numOfMediaFiles		= 1;			// num of files to play
	
	// pm->mediaFiles->xxx;	// media files set
	//
	if (pm->numOfMediaFiles == 0)
		return STATUS_FAIL;
	
	pm->mediaFiles = new SIVRMediaFileParamsStruct[pm->numOfMediaFiles];
	int i;
	for (i = 0; i < (int)pm->numOfMediaFiles; i++)
	{
		SIVRMediaFileParamsStruct* one = &pm->mediaFiles[i];
		//one->actionType		= IVR_ACTION_PLAY;			// play / record / silence
		one->duration			= 10;						// for recording or silence
		//one->playMode;			= 0;						// ?
//		one->confMixVolume		= 0;						// conf  MIX volume while playing this message
//		one->confMsgVolume		= 0;						// conf  MSG volume while playing this message
//		one->confMusicVolume		= 0;						// conf  MUS volume while playing this message
//		one->partyMsgVolume		= 10;						// party MSG volume while playing this message
//		one->partyMusicVolume	= 0;						// party MUS volume while playing this message
		one->frequentness		= 10;						// hint for leave it in memory
		//one->checksum;			= 9999;						// name+checksum defines the file
		one->verNum				= 2;						// internal (for debug)
		one->rsrv[0]				= 0;						// reserve for future
		one->rsrv[1]				= 0;						// reserve for future
		//strncpy( one->fileName[MAX_FULL_PATHNAME], "", 10);			// file full pathname
		//one->fileNameLength		= strlen(fileName);			// file full pathname length
	}

	

	return 0;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
