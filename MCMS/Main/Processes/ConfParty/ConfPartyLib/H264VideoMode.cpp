//+========================================================================+
//                            H264VideoMode.CPP                            |
//            Copyright 2006 Polycom Israel Ltd.			   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Ltd. and is protected by law.			   |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H264VideoMode.CPP                                           |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Keren                                                       |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |13/09/06    |                                                      |
//+========================================================================+

#include  "H264VideoMode.h"
#include  "H264Util.h"
// Global variables:



H264VideoModeDetails CH264VideoMode::g_H264VideoModeTbl[MAX_H264_VIDEO_MODE] =
{
//    videoModeType          profileValue			levelValue        maxMBPS		           maxFS			      maxDPB			     maxBR   maxCPB	 maxStaticMbps
	{ eCIF30,	             H264_Profile_BaseLine,	H264_Level_2,     -1,                     -1,                    -1,                  -1,    -1,		-1},
	{ eCIF60,                H264_Profile_BaseLine,	H264_Level_2,     H264_L2_1_DEFAULT_MBPS, -1,                    -1,                  -1,    -1,	   -1},
	{ e2CIF30,               H264_Profile_BaseLine,	H264_Level_2_1,   -1,                     -1,                    -1,                  -1,    -1,		-1},
	{ eWCIF60,               H264_Profile_BaseLine,	H264_Level_2_1,   H264_WCIF60_MBPS,       -1,                    -1,                  -1,    -1,	   -1},
	{ eSD15,                 H264_Profile_BaseLine,	H264_Level_2_2,   -1,                     -1,                    -1,                  -1,    -1,		-1},
	{ eSD30,                 H264_Profile_BaseLine,	H264_Level_2_2,   H264_864_480_30_MBPS,   -1,                    -1,                  -1,    -1,		-1},
	{ eW4CIF30,              H264_Profile_BaseLine,	H264_Level_2_2,   H264_W4CIF_30_MBPS,     H264_W4CIF_FS,         -1,                  -1,    -1,		-1},
	{ eHD720Asymmetric,      H264_Profile_BaseLine,	H264_Level_2_2,   H264_L3_DEFAULT_MBPS,   -1,                    -1,                  -1,    -1,     -1},
	{ eSD60,                 H264_Profile_BaseLine,	H264_Level_3,     H264_SD_60_MBPS,        -1,                    -1,                  -1,    -1,     -1},
	{ eHD720Symmetric,       H264_Profile_BaseLine,	H264_Level_3_1,   -1,                     H264_HD720_FS,         -1,                  -1,    -1,     -1},
	{ eHD720At60Asymmetric,  H264_Profile_BaseLine,	H264_Level_3,     H264_SD_60_MBPS,        -1,                    -1,                  -1,    -1,     -1},
	{ eHD720At60Symmetric,   H264_Profile_BaseLine,	H264_Level_3_1,   H264_HD720_60_MBPS,     H264_HD720_FS,         -1,                  -1,    -1,     -1},
	{ eHD1080Asymmetric,     H264_Profile_BaseLine,	H264_Level_3_1,   -1,                     H264_HD720_FS,         -1,                  -1,    -1,     -1},
	{ eHD1080Symmetric,      H264_Profile_BaseLine,	H264_Level_3_1,   H264_HD1080_30_MBPS,    H264_HD1080_FS,         -1,                  -1,    -1,     -1},
	// For eHD1080At60Asymmetric a lower rate (H264_HD720_60_MBPS) is chosen for the slower direction.
	{ eHD1080At60Asymmetric, H264_Profile_BaseLine,	H264_Level_3_1,   H264_HD720_60_MBPS,    H264_HD720_FS,          -1,                  -1,    -1,     -1},
	{ eHD1080At60Symmetric,  H264_Profile_BaseLine,	H264_Level_4,   Get1080p60mbps(),      H264_HD1080_FS,         -1,                  -1,    -1,     -1}, //FSN-613: Dynamic Content for SVC/Mix Conf, changed to level_4
	//must be last:
	{ eLasth264VideoMode,    0,						0,			    0,                     0,                      0,                   0,     0,	         0} // Must be the last one.
};

H264VideoModeDetails CH264VideoMode::g_H264VideoModeTblForTndberg[MAX_H264_TAND_UNIQUE_MODES] =
{
//        videoModeType          profileValue			levelValue        maxMBPS		           maxFS			      maxDPB			     maxBR   maxCPB	 maxStaticMbps
		{ eHD720Symmetric,       H264_Profile_BaseLine,	H264_Level_3_1,   -1,                     -1,  			       -1,                  -1,    -1,     -1},
		{ eHD1080Asymmetric,     H264_Profile_BaseLine,	H264_Level_3_1,   -1,                     -1,  			       -1,                  -1,    -1,     -1},
        //add by Yuansong for force 720p on Tndberg MXP
        { eHD1080Symmetric,		 H264_Profile_BaseLine, H264_Level_3_1,   -1,					  -1,				   -1,					-1,    -1,	   -1},
	    { eHD1080At60Asymmetric, H264_Profile_BaseLine, H264_Level_3_1,   -1,					  -1,				   -1,					-1,    -1,	   -1},
	    { eHD1080At60Symmetric,	 H264_Profile_BaseLine, H264_Level_3_1,   -1,					  -1,				   -1,					-1,    -1,	   -1},

};


H264VideoModeDetails CH264VideoMode::g_H264AsymmetricTransmitVideoModeTbl[MAX_H264_ASYMMETRIC_MODES] =
{
	//    videoModeType          profileValue			levelValue        maxMBPS		           maxFS			      maxDPB			     maxBR   maxCPB	 maxStaticMbps
		{ eHD720Asymmetric,      H264_Profile_BaseLine,	H264_Level_3_1,   -1,                    H264_HD720_FS,          -1,                  -1,    -1,     -1},
		{ eHD720At60Asymmetric,  H264_Profile_BaseLine,	H264_Level_3_1,   H264_HD720_60_MBPS,    H264_HD720_FS,          -1,                  -1,    -1,     -1},
		{ eHD1080Asymmetric,     H264_Profile_BaseLine,	H264_Level_3_1,   H264_HD1080_30_MBPS,   H264_HD1080_FS,         -1,                  -1,    -1,     -1},
		// 1080p60debug - to be removed
		//{ eHD1080At60Asymmetric, H264_Profile_BaseLine,	H264_Level_3_1,   H264_HD1080_60_MBPS,   H264_HD1080_FS,         -1,                  -1,    -1,     -1}
		{ eHD1080At60Asymmetric, H264_Profile_BaseLine,	H264_Level_3_1,  Get1080p60mbps(),   H264_HD1080_FS,         -1,                  -1,    -1,     -1}
		//
};

H264VideoModeDetails CH264VideoMode::g_H264VideoModeTblForTIP[MAX_H264_TIP_UNIQUE_MODES] =
{
//        videoModeType          profileValue			levelValue        maxMBPS		           maxFS			      maxDPB			     maxBR   maxCPB	 maxStaticMbps
		{ eHD720Symmetric,       H264_Profile_Main,		H264_Level_3_1,   	-1,                     -1,  			       -1,                  -1,    -1,     -1},
		{ eHD1080Symmetric,      H264_Profile_Main,		H264_Level_4,     	-1,                     -1,  			       -1,                  -1,    -1,     -1},
};
///////////////////////////////////////////////////////////////////////////////////////////////////
//-------------------------- MPM Based system Desicion Matrix--------------------------------------
///MUST BE SORTED BY thresholdBitrate
//H264VideoModeThresholdStruct CH264VideoMode::g_H264MotionVideoModeThresholdTbl[MAX_H264_MOTION_VIDEO_MODE] =
//{
//	//thresholdBitrate  //videoModeType
//	 { 64000,                eCIF30},
//	 {256000,                e2CIF30},
//	 {1024000,               eSD30},
//	 {1920000,               eHD720Asymmetric}
//};
//
/////MUST BE SORTED BY thresholdBitrate
//H264VideoModeThresholdStruct CH264VideoMode::g_H264SharpnessVideoModeThresholdTbl[MAX_H264_SHARPNESS_VIDEO_MODE] =
//{
//	//thresholdBitrate  //videoModeType
//	 { 64000,                eCIF30},
//	 {256000,                eSD15},
//	 {512000,                eSD30},
//	 {1024000,               eHD720Asymmetric}
//};
////////////////////////////////////////////////////////////////////////////////////////////////////
//-------------------------- MPM+ Based system Desicion Matrix--------------------------------------
// H264VideoModeThresholdStruct CH264VideoMode::g_MpmPlusBasedH264MotionVideoModeThresholdTbl[MPM_PLUS_BASED_MAX_H264_MOTION_VIDEO_MODE] =
//{
//	//thresholdBitrate  //videoModeType
//	 { 64000,                eCIF30},
//	 {256000,                eCIF60},
//	 {384000,                eWCIF60},
//	 {1024000,               eSD60},
//	 {1920000,               eHD720At60Asymmetric}
//};
//
/////MUST BE SORTED BY thresholdBitrate
//H264VideoModeThresholdStruct CH264VideoMode::g_MpmPlusBasedH264SharpnessVideoModeThresholdTbl[MPM_PLUS_BASED_MAX_H264_SHARPNESS_VIDEO_MODE] =
//{
//	//thresholdBitrate  //videoModeType
//	 { 64000,                eCIF30},
//	 {256000,                eSD30},
//	 {384000,                eW4CIF30},
//	 {1024000,               eHD720Symmetric},
//	 {4032000,               eHD1080Asymmetric}
//};

////////////////////////////////////////////////////////////////////////////////////////////////////
//-------------------------- Breeze Based system Desicion Matrix--------------------------------------
//H264VideoModeThresholdStruct CH264VideoMode::g_BreezeBasedH264MotionVideoModeThresholdTbl[BREEZE_BASED_MAX_H264_MOTION_VIDEO_MODE] =
//{
//	//thresholdBitrate  //videoModeType
//	 { 64000,                eCIF30},
//	 {256000,                eCIF60},
//	 {384000,                eWCIF60},
//	 {1024000,               eSD60},
//	 {1920000,               eHD720At60Symmetric}
//};
//
/////MUST BE SORTED BY thresholdBitrate
//H264VideoModeThresholdStruct CH264VideoMode::g_BreezeBasedH264SharpnessVideoModeThresholdTbl[BREEZE_BASED_MAX_H264_SHARPNESS_VIDEO_MODE] =
//{
//	//thresholdBitrate  //videoModeType
//	 { 64000,                eCIF30},
//	 {256000,                eSD30},
//	 {384000,                eW4CIF30},
//	 {1024000,               eHD720Symmetric},
//	 {4032000,               eHD1080Symmetric}
//};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Constructor
CH264VideoMode::CH264VideoMode()
{
}
//--------------------------------------------------------------------------------------------------
Eh264VideoModeType CH264VideoMode::GetH264VideoMode(DWORD callRate,eVideoQuality videoQuality, Eh264VideoModeType maxVideoMode, BOOL isHighProfile)
{
  return CResRsrcCalculator::GetVideoMode(GetSystemCardsBasedMode(), callRate, videoQuality, maxVideoMode, isHighProfile);
}
//--------------------------------------------------------------------------------------------------
// The H264 parameters are returned after they are divided by the factors.
void CH264VideoMode::GetH264VideoParams(H264VideoModeDetails& h264VidModeDetails, DWORD callRate, eVideoQuality videoQuality,Eh264VideoModeType maxVideoMode, BOOL isHighProfile)
{
  h264VidModeDetails.videoModeType = eLasth264VideoMode;
  PTRACE2INT(eLevelInfoNormal, "CH264VideoMode::GetH264VideoParams - callRate:", callRate);
  Eh264VideoModeType h264VidMode = GetH264VideoMode(callRate, videoQuality, maxVideoMode, isHighProfile);
  GetH264VideoModeDetailsAccordingToType(h264VidModeDetails, h264VidMode);
}
//--------------------------------------------------------------------------------------------------
// The H264 parameters are returned after they are divided by the factors.
void CH264VideoMode::GetH264AssymetricVideoParams(H264VideoModeDetails& h264VidModeDetails, DWORD callRate, eVideoQuality videoQuality,Eh264VideoModeType maxVideoMode, BOOL isHighProfile)
{
 	h264VidModeDetails.videoModeType = eLasth264VideoMode;
 	PTRACE2INT(eLevelInfoNormal,"CH264VideoMode::GetH264AssymetricVideoParams Call rate",callRate);
 	Eh264VideoModeType h264VidMode = GetH264VideoMode(callRate, videoQuality, maxVideoMode,isHighProfile);
	if(h264VidMode == eHD720Asymmetric || h264VidMode == eHD720At60Asymmetric || h264VidMode == eHD1080Asymmetric || h264VidMode == eHD1080At60Asymmetric )
	{
		GetH264AsymmetricTransmitVideoModeDetailsAccordingToType(h264VidModeDetails, h264VidMode);
	}
	else
	{
		::GetH264VideoParams(h264VidModeDetails, callRate, videoQuality,maxVideoMode,isHighProfile);
	}

}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CH264VideoMode::GetH264VideoModeDetailsAccordingToType(H264VideoModeDetails& h264VidModeDetails, Eh264VideoModeType h264VidModeType)
{
	h264VidModeDetails.profileValue = H264_Profile_None;
	h264VidModeDetails.levelValue= 0;
 	h264VidModeDetails.maxMBPS = -1;
 	h264VidModeDetails.maxFS = -1;
 	h264VidModeDetails.maxDPB =  -1;
 	h264VidModeDetails.maxBR = -1;
 	h264VidModeDetails.maxCPB =-1;
 	h264VidModeDetails.maxStaticMbps =-1;

 	for (int i=0; i<MAX_H264_VIDEO_MODE; i++)
 	{
 		if (g_H264VideoModeTbl[i].videoModeType == h264VidModeType)
 		{
 			h264VidModeDetails.videoModeType = g_H264VideoModeTbl[i].videoModeType;
 			h264VidModeDetails.profileValue =  g_H264VideoModeTbl[i].profileValue;
 			h264VidModeDetails.levelValue =    g_H264VideoModeTbl[i].levelValue;

 			if (g_H264VideoModeTbl[i].maxMBPS != -1)
 			{
 				h264VidModeDetails.maxMBPS =       (DWORD)GetMaxMbpsAsDevision(g_H264VideoModeTbl[i].maxMBPS);
 			}
			
 			if (g_H264VideoModeTbl[i].maxFS != -1)
 			{
 				h264VidModeDetails.maxFS = (DWORD)GetMaxFsAsDevision(g_H264VideoModeTbl[i].maxFS);
 			}
			
 			if (g_H264VideoModeTbl[i].maxDPB !=-1)
 			{
 				h264VidModeDetails.maxDPB = (DWORD)GetMaxDpbAsDevision(g_H264VideoModeTbl[i].maxDPB);
 			}
			
 			if (h264VidModeDetails.maxBR !=-1)
 			{
 				h264VidModeDetails.maxBR =         (DWORD)GetMaxBrAsDevision(g_H264VideoModeTbl[i].maxBR);
 			}
			
 			if (h264VidModeDetails.maxStaticMbps !=-1)
 			{
 				h264VidModeDetails.maxStaticMbps = (DWORD)GetMaxBrAsDevision(g_H264VideoModeTbl[i].maxStaticMbps);
 			}

 			h264VidModeDetails.maxCPB = g_H264VideoModeTbl[i].maxCPB;

 			return;
 		}
 	}
	
 	PTRACE(eLevelInfoNormal,"CH264VideoMode::GetH264VideoModeDetailsAccordingToTyh264VidModeDetails, h264VidModepe - INVALID H264 video mode");
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CH264VideoMode::GetH264AsymmetricTransmitVideoModeDetailsAccordingToType(H264VideoModeDetails& h264VidModeDetails, Eh264VideoModeType h264VidModeType)
{
	h264VidModeDetails.profileValue = H264_Profile_None;
	h264VidModeDetails.levelValue= 0;
 	h264VidModeDetails.maxMBPS = -1;
 	h264VidModeDetails.maxFS = -1;
 	h264VidModeDetails.maxDPB =  -1;
 	h264VidModeDetails.maxBR = -1;
 	h264VidModeDetails.maxCPB =-1;

 	for(int i=0; i<MAX_H264_ASYMMETRIC_MODES; i++)
 	{
 		if (g_H264AsymmetricTransmitVideoModeTbl[i].videoModeType == h264VidModeType)
 		{
 			h264VidModeDetails.videoModeType = g_H264AsymmetricTransmitVideoModeTbl[i].videoModeType;
 			h264VidModeDetails.profileValue =  g_H264AsymmetricTransmitVideoModeTbl[i].profileValue;
 			h264VidModeDetails.levelValue =    g_H264AsymmetricTransmitVideoModeTbl[i].levelValue;
 			if(g_H264AsymmetricTransmitVideoModeTbl[i].maxMBPS != -1)
 			{
 				h264VidModeDetails.maxMBPS =       (DWORD)GetMaxMbpsAsDevision(g_H264AsymmetricTransmitVideoModeTbl[i].maxMBPS);
 			}
 			if(g_H264AsymmetricTransmitVideoModeTbl[i].maxFS != -1)
 			{
 				h264VidModeDetails.maxFS = (DWORD)GetMaxFsAsDevision(g_H264AsymmetricTransmitVideoModeTbl[i].maxFS);
 			}
 			if(g_H264AsymmetricTransmitVideoModeTbl[i].maxDPB !=-1)
 			{
 				h264VidModeDetails.maxDPB = (DWORD)GetMaxDpbAsDevision(g_H264AsymmetricTransmitVideoModeTbl[i].maxDPB);
 			}
 			if (g_H264AsymmetricTransmitVideoModeTbl[i].maxBR !=-1)
 			{
 				h264VidModeDetails.maxBR =         (DWORD)GetMaxBrAsDevision(g_H264AsymmetricTransmitVideoModeTbl[i].maxBR);
 			}

 			h264VidModeDetails.maxCPB = g_H264AsymmetricTransmitVideoModeTbl[i].maxCPB;

 			return;
 		}
 	}
 	PTRACE2INT(eLevelInfoNormal,"CH264VideoMode::GetH264AsymmetricTransmitVideoModeDetailsAccordingToType - INVALID H264 asymmetric video mode = ", h264VidModeType);
}
//--------------------------------------------------------------------------------------------------
// The H264 parameters are returned after they are divided by the factors.
void CH264VideoMode::GetH264VideoParamsForTand(H264VideoModeDetails& h264VidModeDetails, DWORD callRate, eVideoQuality videoQuality,Eh264VideoModeType maxVideoMode, BOOL isHighProfile)
{
 	h264VidModeDetails.videoModeType = eLasth264VideoMode;
 	PTRACE2INT(eLevelInfoNormal,"CH264VideoMode::GetH264VideoParams Call rate",callRate);
 	Eh264VideoModeType h264VidMode = GetH264VideoMode(callRate, videoQuality, maxVideoMode,isHighProfile);
 	if(h264VidMode == eHD720Symmetric ||h264VidMode == eHD1080Asymmetric 
		||h264VidMode == eHD1080Symmetric ||h264VidMode == eHD1080At60Asymmetric || h264VidMode == eHD1080At60Symmetric )
 		GetH264VideoModeDetailsAccordingToTypeForTand(h264VidModeDetails, h264VidMode);
 	else
 		GetH264VideoModeDetailsAccordingToType(h264VidModeDetails, h264VidMode);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CH264VideoMode::GetH264VideoModeDetailsAccordingToTypeForTand(H264VideoModeDetails& h264VidModeDetails, Eh264VideoModeType h264VidModeType)
{
	h264VidModeDetails.profileValue = H264_Profile_None;
	h264VidModeDetails.levelValue= 0;
 	h264VidModeDetails.maxMBPS = -1;
 	h264VidModeDetails.maxFS = -1;
 	h264VidModeDetails.maxDPB =  -1;
 	h264VidModeDetails.maxBR = -1;
 	h264VidModeDetails.maxCPB =-1;
 	h264VidModeDetails.maxStaticMbps =-1;

 	for(int i=0; i<MAX_H264_TAND_UNIQUE_MODES; i++)
 	{
 		if (g_H264VideoModeTblForTndberg[i].videoModeType == h264VidModeType)
 		{
 			h264VidModeDetails.videoModeType = g_H264VideoModeTblForTndberg[i].videoModeType;
 			h264VidModeDetails.profileValue =  g_H264VideoModeTblForTndberg[i].profileValue;
 			h264VidModeDetails.levelValue =    g_H264VideoModeTblForTndberg[i].levelValue;
 			if(g_H264VideoModeTblForTndberg[i].maxMBPS != -1)
 			{
 				h264VidModeDetails.maxMBPS =       (DWORD)GetMaxMbpsAsDevision(g_H264VideoModeTblForTndberg[i].maxMBPS);
 			}
 			if(g_H264VideoModeTblForTndberg[i].maxFS != -1)
 			{
 				h264VidModeDetails.maxFS = (DWORD)GetMaxFsAsDevision(g_H264VideoModeTblForTndberg[i].maxFS);
 			}
 			if(g_H264VideoModeTblForTndberg[i].maxDPB !=-1)
 			{
 				h264VidModeDetails.maxDPB = (DWORD)GetMaxDpbAsDevision(g_H264VideoModeTblForTndberg[i].maxDPB);
 			}
 			if (h264VidModeDetails.maxBR !=-1)
 			{
 				h264VidModeDetails.maxBR =         (DWORD)GetMaxBrAsDevision(g_H264VideoModeTblForTndberg[i].maxBR);
 			}
 			if (h264VidModeDetails.maxStaticMbps !=-1)
 			{
 				h264VidModeDetails.maxStaticMbps = (DWORD)GetMaxBrAsDevision(g_H264VideoModeTblForTndberg[i].maxStaticMbps);
 			}

 			h264VidModeDetails.maxCPB = g_H264VideoModeTblForTndberg[i].maxCPB;

 			return;
 		}
 	}
 	PTRACE(eLevelInfoNormal,"CH264VideoMode::GetH264VideoModeDetailsAccordingToTypeForTand - INVALID H264 video mode");
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CH264VideoMode::GetH264VideoModeDetailsAccordingToTypeForTIP(H264VideoModeDetails& h264VidModeDetails, Eh264VideoModeType h264VidModeType)
{
	h264VidModeDetails.profileValue = H264_Profile_None;
	h264VidModeDetails.levelValue= 0;
 	h264VidModeDetails.maxMBPS = -1;
 	h264VidModeDetails.maxFS = -1;
 	h264VidModeDetails.maxDPB =  -1;
 	h264VidModeDetails.maxBR = -1;
 	h264VidModeDetails.maxCPB =-1;
 	h264VidModeDetails.maxStaticMbps =-1;

 	for(int i=0; i<MAX_H264_TIP_UNIQUE_MODES; i++)
 	{
 		if (g_H264VideoModeTblForTIP[i].videoModeType == h264VidModeType)
 		{
 			h264VidModeDetails.videoModeType = g_H264VideoModeTblForTIP[i].videoModeType;
 			h264VidModeDetails.profileValue =  g_H264VideoModeTblForTIP[i].profileValue;
 			h264VidModeDetails.levelValue =    g_H264VideoModeTblForTIP[i].levelValue;
 			if(g_H264VideoModeTblForTIP[i].maxMBPS != -1)
 			{
 				h264VidModeDetails.maxMBPS =       (DWORD)GetMaxMbpsAsDevision(g_H264VideoModeTblForTIP[i].maxMBPS);
 			}
 			if(g_H264VideoModeTblForTIP[i].maxFS != -1)
 			{
 				h264VidModeDetails.maxFS = (DWORD)GetMaxFsAsDevision(g_H264VideoModeTblForTIP[i].maxFS);
 			}
 			if(g_H264VideoModeTblForTIP[i].maxDPB !=-1)
 			{
 				h264VidModeDetails.maxDPB = (DWORD)GetMaxDpbAsDevision(g_H264VideoModeTblForTIP[i].maxDPB);
 			}
 			if (h264VidModeDetails.maxBR !=-1)
 			{
 				h264VidModeDetails.maxBR =         (DWORD)GetMaxBrAsDevision(g_H264VideoModeTblForTIP[i].maxBR);
 			}
 			if (h264VidModeDetails.maxStaticMbps !=-1)
 			{
 				h264VidModeDetails.maxStaticMbps = (DWORD)GetMaxBrAsDevision(g_H264VideoModeTblForTIP[i].maxStaticMbps);
 			}

 			h264VidModeDetails.maxCPB = g_H264VideoModeTblForTIP[i].maxCPB;

 			return;
 		}
 	}
 	PTRACE(eLevelInfoNormal,"CH264VideoMode::GetH264VideoModeDetailsAccordingToTypeForTIP - INVALID H264 video mode");
}

void CH264VideoMode::GetH264VideoModeDetailsAccordingToType(H264VideoModeDetails& h264VidModeDetails, Eh264VideoModeType h264VidModeType, BYTE isTipMode)
{
	if(isTipMode)
		GetH264VideoModeDetailsAccordingToTypeForTIP(h264VidModeDetails, h264VidModeType);

	else
		GetH264VideoModeDetailsAccordingToType(h264VidModeDetails, h264VidModeType);
}
