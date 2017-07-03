//+========================================================================+
//                            H263VideoMode.CPP                            |
//            Copyright 2007 Polycom Israel Ltd.				           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Ltd. and is protected by law.					   |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H263VideoMode.cpp                                           |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Oren M                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |25/11/07    |                                                     |
//+========================================================================+

#include "H263VideoMode.h"
#include "ObjString.h"
#include "ConfPartyGlobals.h"


const int rate64In100Bits = 640;
const eSystemCardsMode CH263VideoMode::g_SystemCardsModeTbl[] = {eSystemCardsMode_mpm, eSystemCardsMode_mpm_plus,eSystemCardsMode_breeze,eSystemCardsMode_mpmrx};
const eVideoQuality CH263VideoMode::g_H263VideoQualityTbl[] = {eVideoQualityMotion, eVideoQualitySharpness};
const int 			CH263VideoMode::g_H263VideoBitRateTbl[] = {1280, 2560, 3840, 19200, 61440}; // rates are in 100 bits per sec
const EFormat 		CH263VideoMode::g_H263VideoFormatTbl[]  = {kQCif, kCif, k4Cif};
const APIS8   		CH263VideoMode::g_H263VideoMpiTbl[SYSTEM_CARDS_MODE_OPTIONS][H263_VIDEO_QUALITY_OPTIONS][H263_VIDEO_RATE_OPTIONS][H263_VIDEO_FORMAT_OPTIONS] = {

	/* i system cards based mode | j video quality | k video rate | l video format 1(30), 2(15), -1(NA) */
	{////------------------MPM CARDS BASED SYSTEM ------------------/////
		{
			/* i = eSystemCardsMode_mpm j=motion k=128000  l= qcif,cif,4cif */ {1,1,-1},
			/* i = eSystemCardsMode_mpm j=motion k=256000  l= qcif,cif,4cif */ {1,1,-1},
			/* i = eSystemCardsMode_mpm j=motion k=384000  l= qcif,cif,4cif */ {1,1,-1},
			/* i = eSystemCardsMode_mpm j=motion k=1920000 l= qcif,cif,4cif */ {1,1,-1},
			/* i = eSystemCardsMode_mpm j=motion k=4M l= qcif,cif,4cif */      {1,1,-1}
		}
		,
		{
			/* i = eSystemCardsMode_mpm j=sharpness k=128000  l= qcif,cif,4cif */ {1,1,-1},
			/* i = eSystemCardsMode_mpm j=sharpness k=256000  l= qcif,cif,4cif */ {1,1,2},
			/* i = eSystemCardsMode_mpm j=sharpness k=384000  l= qcif,cif,4cif */ {1,1,2},
			/* i = eSystemCardsMode_mpm j=sharpness k=1920000 l= qcif,cif,4cif */ {1,1,2},
			/* i = eSystemCardsMode_mpm j=sharpness k=4M l= qcif,cif,4cif */      {1,1,2}
		}
	}
	,
	{////------------------MPM+ CARDS BASED SYSTEM ------------------/////
		{
			/* i = eSystemCardsMode_mpm_plus j=motion k=128000  l= qcif,cif,4cif */ {1,1,-1},
			/* i = eSystemCardsMode_mpm_plus j=motion k=256000  l= qcif,cif,4cif */ {1,1,-1},
			/* i = eSystemCardsMode_mpm_plus j=motion k=384000  l= qcif,cif,4cif */ {1,1,-1},
			/* i = eSystemCardsMode_mpm_plus j=motion k=1920000 l= qcif,cif,4cif */ {1,1,-1},
			/* i = eSystemCardsMode_mpm j=motion k=4M l= qcif,cif,4cif */           {1,1,-1}
		}
		,
		{
			/* i = eSystemCardsMode_mpm_plus j=sharpness k=128000  l= qcif,cif,4cif */ {1,1,-1},
			/* i = eSystemCardsMode_mpm_plus j=sharpness k=256000  l= qcif,cif,4cif */ {1,1,2},
			/* i = eSystemCardsMode_mpm_plus j=sharpness k=384000  l= qcif,cif,4cif */ {1,1,2},
			/* i = eSystemCardsMode_mpm_plus j=sharpness k=1920000 l= qcif,cif,4cif */ {1,1,2},
			/* i = eSystemCardsMode_mpm_plus j=sharpness k=4M l= qcif,cif,4cif */ {1,1,2}
		}
	}
	,
	{////------------------BREEZE CARDS BASED SYSTEM ------------------/////
		{
			/* i = eSystemCardsMode_mpm_plus j=motion k=128000  l= qcif,cif,4cif */ {1,1,-1},
			/* i = eSystemCardsMode_mpm_plus j=motion k=256000  l= qcif,cif,4cif */ {1,1,-1},
			/* i = eSystemCardsMode_mpm_plus j=motion k=384000  l= qcif,cif,4cif */ {1,1,-1},
			/* i = eSystemCardsMode_mpm_plus j=motion k=1920000 l= qcif,cif,4cif */ {1,1,-1},
			/* i = eSystemCardsMode_mpm j=motion k=4M l= qcif,cif,4cif */ {1,1,-1}
		}
		,
		{
			/* i = eSystemCardsMode_mpm_plus j=sharpness k=128000  l= qcif,cif,4cif */ {1,1,-1},
			/* i = eSystemCardsMode_mpm_plus j=sharpness k=256000  l= qcif,cif,4cif */ {1,1,2},
			/* i = eSystemCardsMode_mpm_plus j=sharpness k=384000  l= qcif,cif,4cif */ {1,1,2},
			/* i = eSystemCardsMode_mpm_plus j=sharpness k=1920000 l= qcif,cif,4cif */ {1,1,2},
			/* i = eSystemCardsMode_mpm_plus j=sharpness k=4M l= qcif,cif,4cif */ {1,1,2}
		}
	}
	,
	{////------------------MPM-RX CARDS BASED SYSTEM ------------------/////
		{
			/* i = eSystemCardsMode_mpm_plus j=motion k=128000  l= qcif,cif,4cif */ {1,1,-1},
			/* i = eSystemCardsMode_mpm_plus j=motion k=256000  l= qcif,cif,4cif */ {1,1,-1},
			/* i = eSystemCardsMode_mpm_plus j=motion k=384000  l= qcif,cif,4cif */ {1,1,-1},
			/* i = eSystemCardsMode_mpm_plus j=motion k=1920000 l= qcif,cif,4cif */ {1,1,-1},
			/* i = eSystemCardsMode_mpm j=motion k=4M l= qcif,cif,4cif */ {1,1,-1}
		}
		,
		{
			/* i = eSystemCardsMode_mpm_plus j=sharpness k=128000  l= qcif,cif,4cif */ {1,1,-1},
			/* i = eSystemCardsMode_mpm_plus j=sharpness k=256000  l= qcif,cif,4cif */ {1,1,2},
			/* i = eSystemCardsMode_mpm_plus j=sharpness k=384000  l= qcif,cif,4cif */ {1,1,2},
			/* i = eSystemCardsMode_mpm_plus j=sharpness k=1920000 l= qcif,cif,4cif */ {1,1,2},
			/* i = eSystemCardsMode_mpm_plus j=sharpness k=4M l= qcif,cif,4cif */ {1,1,2}
		}
	}
};

/////////////////////////////////////////////////////////////////////////////
// Constructor
CH263VideoMode::CH263VideoMode()
{
}
/////////////////////////////////////////////////////////////////////////////
// search the given value within system cards mode table and returns its location (index)
int CH263VideoMode::GetSystemCardsModeIndex(eSystemCardsMode systemCardsMode)
{
	// Tsahi TODO: remove MPM and MPM+
	for(int i=0; i<SYSTEM_CARDS_MODE_OPTIONS; i++)
		if(systemCardsMode == g_SystemCardsModeTbl[i])
			return i;
	return -1;
}
/////////////////////////////////////////////////////////////////////////////
// search the given value within video mode table and returns its location (index)
int CH263VideoMode::GetVideoQualityIndex(eVideoQuality videoQuality)
{
	for(int i=0; i<H263_VIDEO_QUALITY_OPTIONS; i++)
		if(videoQuality == g_H263VideoQualityTbl[i])
			return i;
	return -1;
}

/////////////////////////////////////////////////////////////////////////////
int CH263VideoMode::GetVideoBitRateIndex(int videoRateIn100bits)
{
	if(videoRateIn100bits <= 0 || videoRateIn100bits > g_H263VideoBitRateTbl[H263_VIDEO_RATE_OPTIONS-1])
		return -1; // invalid bitrate
	int bitRateIndex = 0;
	for(int i=0; i<H263_VIDEO_RATE_OPTIONS; i++)
	{
		if( g_H263VideoBitRateTbl[i] > videoRateIn100bits)
			break;
		else bitRateIndex = i;
	}
	return bitRateIndex;
}

/////////////////////////////////////////////////////////////////////////////
// search the given value within video format table and returns its location (index)
int CH263VideoMode::GetVideoFormatIndex(EFormat videoFormat)
{
	for(int i=0; i<H263_VIDEO_FORMAT_OPTIONS; i++)
		if(videoFormat == g_H263VideoFormatTbl[i])
			return i;
	return -1;
}

/////////////////////////////////////////////////////////////////////////////
// fills buffer in qcif and cif rates according to videoBitRate(rate is in 100 bits per sec)
void CH263VideoMode::Get263VideoCardMPI(int videoRateIn100bits, APIS8* buffer, eVideoQuality videoQuality)
{
	if(!buffer)
		return;

	if( sizeof(buffer) < H263_CIF+1 )	return;

	int videoRateInNumberOf64s = (videoRateIn100bits/rate64In100Bits + 1) * rate64In100Bits;//the tables are according to call rate
																							// which is wrong, since it is possible to ask for
																							// the FPS when content is activeted and therefore the video rate is much lower than the call ratee
																							// so with this round up we similate call rate according to the video rate.
	memset(buffer, -1, sizeof(buffer));
	eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
	int systemCardsBasedModeIndex = GetSystemCardsModeIndex(systemCardsBasedMode);
	int videoQualityIndex = GetVideoQualityIndex(videoQuality);
	int videoRateIndex    = GetVideoBitRateIndex(videoRateInNumberOf64s);
	int qcifFormatIndex   = 0;
	int cifFormatIndex    = 1;
	int cif4FormatIndex    = 2;

	if( videoQualityIndex == -1 || videoRateIndex == -1 || systemCardsBasedModeIndex == -1 ) // invalid quality or bit rate or system cards based mode
		return;

	buffer[H263_QCIF_SQCIF] = g_H263VideoMpiTbl[systemCardsBasedModeIndex][videoQualityIndex][videoRateIndex][qcifFormatIndex];
	buffer[H263_CIF] = g_H263VideoMpiTbl[systemCardsBasedModeIndex][videoQualityIndex][videoRateIndex][cifFormatIndex];
	if(systemCardsBasedMode != eSystemCardsMode_mpm/*mpm*/)
		buffer[H263_CIF_4] = g_H263VideoMpiTbl[systemCardsBasedModeIndex][videoQualityIndex][videoRateIndex][cif4FormatIndex];


	return;
}

/////////////////////////////////////////////////////////////////////////////
//(video rate is in 100 bits per sec)
APIS8 CH263VideoMode::GetH263Cif4VideoCardMPI(int videoRateIn100bits, eVideoQuality videoQuality)
{
	int videoRateInNumberOf64s = (videoRateIn100bits/rate64In100Bits + 1) * rate64In100Bits;//the tables are according to call rate
																							// which is wrong, since it is possible to ask for
																							// the FPS when content is activeted and therefore the video rate is much lower than the call ratee
																							// so with this round up we similate call rate according to the video rate.
	eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
	int systemCardsBasedModeIndex = GetSystemCardsModeIndex(systemCardsBasedMode);

	int videoQualityIndex = GetVideoQualityIndex(videoQuality);
	int videoRateIndex    = GetVideoBitRateIndex(videoRateInNumberOf64s);
	int cif4FormatIndex   = 2;

	if( videoQualityIndex == -1 || videoRateIndex == -1  || systemCardsBasedModeIndex == -1) // invalid quality or bit rate or system cards based mode
		return -1;
	else if (GetMaxVideoModeBySysCfg() == eCIF30)
		return -1;

	return g_H263VideoMpiTbl[systemCardsBasedModeIndex][videoQualityIndex][videoRateIndex][cif4FormatIndex];
}


