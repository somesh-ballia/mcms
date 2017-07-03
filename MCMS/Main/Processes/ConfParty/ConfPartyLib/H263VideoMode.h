
//+========================================================================+
//                            H263VideoMode.H                                   |
//            Copyright 2007 Polycom Israel Ltd.				           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Ltd. and is protected by law.					   |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H263VideoMode.h                                             |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Oren M                                                       |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |25/11/07    |                                                     |
//+========================================================================+
#ifndef _H263VIDEOMODE
#define _H263VIDEOMODE

#include  "PObject.h"
#include  "H263.h"
#include  "ConfPartyDefines.h"
#include  "IpChannelParams.h"
#include  "ConfPartyGlobals.h"


class CH263VideoMode : public CPObject
{
CLASS_TYPE_1(CH263VideoMode, CPObject)
public:
    // Constructor
    CH263VideoMode();
    virtual ~CH263VideoMode(){}

    // Operations
    virtual const char* NameOf() const {return "CH263VideoMode";}

    static void  Get263VideoCardMPI(int videoRateIn100bits, APIS8* buffer, eVideoQuality videoQuality); // quality - sharpness or motion
    static APIS8 GetH263Cif4VideoCardMPI(int videoRateIn100bits, eVideoQuality videoQuality);


private:
	static int GetSystemCardsModeIndex(eSystemCardsMode systemCardsMode);
	static int GetVideoQualityIndex(eVideoQuality videoQuality);
	static int GetVideoBitRateIndex(int videoRateIn64bits);
	static int GetVideoFormatIndex(EFormat videoFormat);

    // Data global array:
	static const int SYSTEM_CARDS_MODE_OPTIONS = 4;
    static const int H263_VIDEO_QUALITY_OPTIONS = 2;	// sharpness, motion
    static const int H263_VIDEO_RATE_OPTIONS = 5; 		// 0-1280, 1280-2560, 2560-3840, 3840-19200, 19200-40960
    static const int H263_VIDEO_FORMAT_OPTIONS = 3; 	// qcif, cif, 4cif

    static const eSystemCardsMode g_SystemCardsModeTbl[SYSTEM_CARDS_MODE_OPTIONS];
    static const eVideoQuality	g_H263VideoQualityTbl[H263_VIDEO_QUALITY_OPTIONS];
    static const int 			g_H263VideoBitRateTbl[H263_VIDEO_RATE_OPTIONS];
    static const EFormat		g_H263VideoFormatTbl[H263_VIDEO_FORMAT_OPTIONS];

    static const APIS8			g_H263VideoMpiTbl[SYSTEM_CARDS_MODE_OPTIONS][H263_VIDEO_QUALITY_OPTIONS][H263_VIDEO_RATE_OPTIONS][H263_VIDEO_FORMAT_OPTIONS];
};

#endif //_H263VIDEOMODE

