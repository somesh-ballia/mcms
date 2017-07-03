
//+========================================================================+
//                            H264VideoMode.H                                   |
//            Copyright 2006 Polycom Israel Ltd.				           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Ltd. and is protected by law.					   |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H264VideoMode.H                                             |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Keren                                                       |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |13/11/06    |                                                     |
//+========================================================================+
#ifndef _H264VIDEOMODE
#define _H264VIDEOMODE

#include  "PObject.h"
#include  "H264.h"
#include  "ConfPartyDefines.h"
#include  "ConfPartyGlobals.h"
#include  "ResRsrcCalculator.h"


#define MAX_H264_VIDEO_MODE  eLasth264VideoMode
//MPM Based System
#define MAX_H264_MOTION_VIDEO_MODE  4
#define MAX_H264_SHARPNESS_VIDEO_MODE  4
//MPM+ Based System
#define MPM_PLUS_BASED_MAX_H264_MOTION_VIDEO_MODE  5
#define MPM_PLUS_BASED_MAX_H264_SHARPNESS_VIDEO_MODE  5
//Breeze+ Based System
#define BREEZE_BASED_MAX_H264_MOTION_VIDEO_MODE  5
#define BREEZE_BASED_MAX_H264_SHARPNESS_VIDEO_MODE  5

//For Asymmetric H264 Video Modes
#define MAX_H264_ASYMMETRIC_MODES 4

//For TAND uniqe H264 Video Modes
#define MAX_H264_TAND_UNIQUE_MODES 5

// TIP
#define MAX_H264_TIP_UNIQUE_MODES 2

//typedef struct  //moved to CResRsrcCalculator
//{
//	DWORD              thresholdBitrate;
//	Eh264VideoModeType videoModeType;
//} H264VideoModeThresholdStruct;

class CH264VideoMode : public CPObject
{
	CLASS_TYPE_1(CH264VideoMode, CPObject)
	public:

    // Constructors
    CH264VideoMode ();
    virtual ~CH264VideoMode(){}

    // Operations
    virtual const char* NameOf() const {return "CH264VideoMode";}
    void GetH264VideoParams(H264VideoModeDetails& h264VidModeDetails, DWORD callRate,eVideoQuality videoQuality, Eh264VideoModeType maxVideoMode, BOOL isHighProfile = TRUE);
    void GetH264AssymetricVideoParams(H264VideoModeDetails& h264VidModeDetails, DWORD callRate, eVideoQuality videoQuality,Eh264VideoModeType maxVideoMode, BOOL isHighProfile = TRUE);
    Eh264VideoModeType GetH264VideoMode(DWORD callRate,eVideoQuality videoQuality, Eh264VideoModeType maxVideoMode, BOOL isHighProfile = TRUE);
    void GetH264VideoModeDetailsAccordingToType(H264VideoModeDetails& h264VidModeDetails, Eh264VideoModeType h264VidModeType);
    void GetH264AsymmetricTransmitVideoModeDetailsAccordingToType(H264VideoModeDetails& h264VidModeDetails, Eh264VideoModeType h264VidModeType);
    void GetH264VideoModeDetailsAccordingToType(H264VideoModeDetails& h264VidModeDetails, Eh264VideoModeType h264VidModeType, BYTE isTipMode);

    void GetH264VideoParamsForTand(H264VideoModeDetails& h264VidModeDetails, DWORD callRate, eVideoQuality videoQuality,Eh264VideoModeType maxVideoMode, BOOL isHighProfile = TRUE);
    void GetH264VideoModeDetailsAccordingToTypeForTand(H264VideoModeDetails& h264VidModeDetails, Eh264VideoModeType h264VidModeType);

    // TIP
    void GetH264VideoModeDetailsAccordingToTypeForTIP(H264VideoModeDetails& h264VidModeDetails, Eh264VideoModeType h264VidModeType);

	private:

    // Data global array:
    static H264VideoModeDetails g_H264VideoModeTbl[MAX_H264_VIDEO_MODE];
    static H264VideoModeDetails g_H264VideoModeTblForTndberg[MAX_H264_TAND_UNIQUE_MODES];
    static H264VideoModeDetails g_H264AsymmetricTransmitVideoModeTbl[MAX_H264_ASYMMETRIC_MODES];

    // TIP
    static H264VideoModeDetails g_H264VideoModeTblForTIP[MAX_H264_TIP_UNIQUE_MODES];

	///MPM Based system decision matrix
//    static H264VideoModeThresholdStruct g_H264MotionVideoModeThresholdTbl[MAX_H264_MOTION_VIDEO_MODE];
//	static H264VideoModeThresholdStruct g_H264SharpnessVideoModeThresholdTbl[MAX_H264_SHARPNESS_VIDEO_MODE];
//	///MPM+ Based system decision matrix
//	static H264VideoModeThresholdStruct g_MpmPlusBasedH264MotionVideoModeThresholdTbl[MPM_PLUS_BASED_MAX_H264_MOTION_VIDEO_MODE];
//	static H264VideoModeThresholdStruct g_MpmPlusBasedH264SharpnessVideoModeThresholdTbl[MPM_PLUS_BASED_MAX_H264_SHARPNESS_VIDEO_MODE];
//	///Breeze Based system decision matrix
//	static H264VideoModeThresholdStruct g_BreezeBasedH264MotionVideoModeThresholdTbl[MPM_PLUS_BASED_MAX_H264_MOTION_VIDEO_MODE];
//	static H264VideoModeThresholdStruct g_BreezeBasedH264SharpnessVideoModeThresholdTbl[MPM_PLUS_BASED_MAX_H264_SHARPNESS_VIDEO_MODE];

 };




#endif //_H264VIDEOMODE

