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
// FILE:       IVRPlayMessage.cpp	                                   |
//+========================================================================+


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
#include "Macros.h"
#include "Segment.h"
#include "IVRPlayMessage.h"


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
CIVRPlayMessage::CIVRPlayMessage()
{
	memset( (BYTE*)&play, 0, sizeof(SIVRPlayMessageStruct));
	play.mediaFiles = NULL;
}

/////////////////////////////////////////////////////////////////
CIVRPlayMessage::~CIVRPlayMessage()
{
	PDELETEA(play.mediaFiles);
}


/////////////////////////////////////////////////////////////////
void CIVRPlayMessage::Serialize( CSegment* seg )
{
	seg->Put((BYTE*)(&play.physicalPortDescription),sizeof(PHYSICAL_RESOURCE_INFO_S));
	*seg << (DWORD) play.partyOrconfFlag; 
	*seg << (DWORD) play.stopPrevOrAppend; 
	*seg << (DWORD) play.mediaType; 
	*seg << (DWORD) play.numOfRepetition; 
	*seg << (DWORD) play.startIVRFlag; 
	seg->Put((BYTE*)(&play.startIVR),sizeof(SIVRStartIVRParams));
	*seg << (DWORD) play.videoBitRate;
	*seg << (BYTE) play.isTipMode;
	*seg << (BYTE) play.rsrv[0];
	*seg << (BYTE) play.rsrv[1];
	*seg << (BYTE) play.rsrv[2];
	*seg << (DWORD) play.reserve;
	*seg << (DWORD) play.numOfMediaFiles; 

	DWORD mediaFiles = play.numOfMediaFiles;
	int i;
	for (i = 0; i < (int)mediaFiles; i++)
	{
		SIVRMediaFileParamsStruct* media = &(play.mediaFiles[i]);
		*seg << (DWORD) media->actionType; 
		*seg << (DWORD) media->duration; 
		*seg << (DWORD) media->playMode; 
		*seg << (DWORD) media->frequentness; 
		*seg << (DWORD) media->checksum; 
		*seg << (DWORD) media->verNum; 
		*seg << (DWORD) media->rsrv[0]; 
		*seg << (DWORD) media->rsrv[1]; 
		*seg << (DWORD) media->fileNameLength; 
		seg->Put((BYTE*)media->fileName, (WORD)MAX_FULL_PATHNAME);
	}
}


/////////////////////////////////////////////////////////////////
void CIVRPlayMessage::DeSerialize( CSegment* seg)
{
	DWORD tmp = 0;
	BYTE  tmp_byte = 0;

	seg->Get((BYTE*)(&play.physicalPortDescription),sizeof(PHYSICAL_RESOURCE_INFO_S));

	*seg >> tmp;
	play.partyOrconfFlag = tmp;
	*seg >> tmp;
	play.stopPrevOrAppend = tmp;
	*seg >> tmp;
	play.mediaType = tmp;
	*seg >> tmp;
	play.numOfRepetition = tmp;
	*seg >> tmp;
	play.startIVRFlag = tmp;
	seg->Get((BYTE*)(&play.startIVR),sizeof(SIVRStartIVRParams));
	*seg >> tmp;
	play.videoBitRate = tmp;
	*seg >> tmp_byte;
	play.isTipMode = tmp_byte;
	*seg >> tmp_byte;
	play.rsrv[0] = tmp_byte;
	*seg >> tmp_byte;
	play.rsrv[1] = tmp_byte;
	*seg >> tmp_byte;
	play.rsrv[2] = tmp_byte;
	*seg >> tmp;
	play.reserve = tmp;
	*seg >> tmp;
	play.numOfMediaFiles = tmp;

	DWORD mediaFiles = play.numOfMediaFiles;
	PDELETEA(play.mediaFiles);
	play.mediaFiles = new SIVRMediaFileParamsStruct[mediaFiles];
	memset((BYTE*)(play.mediaFiles), 0, mediaFiles*sizeof(SIVRMediaFileParamsStruct));

	int i;
	for (i = 0; i < (int)mediaFiles; i++)
	{
		SIVRMediaFileParamsStruct* media = &(play.mediaFiles[i]);
		memset( (BYTE*)media, 0, sizeof(SIVRMediaFileParamsStruct) );
		*seg >> tmp;
		media->actionType = tmp;
		*seg >> tmp;
		media->duration = tmp;
		*seg >> tmp;
		media->playMode = tmp;
		*seg >> tmp;
		media->frequentness = tmp; 
		*seg >> tmp;
		media->checksum = tmp;
		*seg >> tmp;
		media->verNum = tmp;
		*seg >> tmp;
		media->rsrv[0] = tmp;
		*seg >> tmp;
		media->rsrv[1] = tmp;
		*seg >> tmp;
		media->fileNameLength = tmp; 
		seg->Get((BYTE*)media->fileName, (WORD)MAX_FULL_PATHNAME);
		if (0 == media->fileNameLength)
			media->fileName[0] = '\0';
	}
}

