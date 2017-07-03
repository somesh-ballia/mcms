/*
 * vp8VideoMode.cpp
 *
 *  Created on: Mar 31, 2014
 *      Author: nalster
 */



#include  "vp8VideoMode.h"
#include  "H264Util.h"








VP8VideoModeDetails CVP8VideoMode::g_VP8VideoModeTbl[MAX_VP8_UNIQUE_MODES] =
{

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
//	videoModeType		Width		Height		FrameRate		AspectRatio							MaxFS		MBPS		MaxNumPixels		MinBitRate		MaxBitRate 		   //
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
	{eHD720Symmetric,	1280,		720,		30,				E_VIDEO_RES_ASPECT_RATIO_16_9,		3600,		108000,		921600,				700000,			20000000	},
	{eW4CIF30,			640,		480,		30,				E_VIDEO_RES_ASPECT_RATIO_4_3 ,		1200,		36000 ,		307200,				300000,			800000		},
	{eW4CIF30,			640,		360,		30,				E_VIDEO_RES_ASPECT_RATIO_16_9,		900 ,		27000 ,		230400,				300000,			800000		},
	{eCIF30,			320,		240,		30,				E_VIDEO_RES_ASPECT_RATIO_4_3 ,		300 ,		9000  ,		76800 ,				100000,			350000		},
	{eCIF30,			320,		180,		30,				E_VIDEO_RES_ASPECT_RATIO_16_9,		225 ,		6750  ,		57600 ,				15000 ,			250000		},

};


//------------------------------//
//            CTORs			    //
//------------------------------//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CVP8VideoMode::CVP8VideoMode ()
{

}



void CVP8VideoMode::GetVp8VideoParamsByRate(VP8VideoModeDetails& VP8VideoModeDetails, DWORD callRate,Eh264VideoModeType maxVideoMode,DWORD aspectRatio)
{
	PTRACE2INT(eLevelInfoNormal, "CVP8VideoMode::GetVp8VideoParamsByRate - callRate:", callRate);
	VP8VideoModeDetails.maxBitRate = -1;
	VP8VideoModeDetails.minBitRate = -1;
	VP8VideoModeDetails.maxFrameRate = -1;
	VP8VideoModeDetails.aspectRatio = -1;
	VP8VideoModeDetails.maxWidth = -1;
	VP8VideoModeDetails.maxHeight =-1;
	VP8VideoModeDetails.maxNumOfPixels =-1;
	VP8VideoModeDetails.videoModeType = eInvalidModeType;



	for(int i=0; i< MAX_VP8_UNIQUE_MODES; i++)
	{

		PTRACE2INT(eLevelInfoNormal, "CVP8VideoMode::GetVp8VideoParamsByRate - found mode - min bit rate is :callRate = ", callRate);
		PTRACE2INT(eLevelInfoNormal, "CVP8VideoMode::GetVp8VideoParamsByRate - found mode - min bit rate is :maxVideoMode = ", maxVideoMode);
		PTRACE2INT(eLevelInfoNormal, "CVP8VideoMode::GetVp8VideoParamsByRate - found mode - min bit rate is :g_VP8VideoModeTbl[i].videoModeType = ", g_VP8VideoModeTbl[i].videoModeType);
		PTRACE2INT(eLevelInfoNormal, "CVP8VideoMode::GetVp8VideoParamsByRate - found mode - min bit rate is :g_VP8VideoModeTbl[i].minBitRate = ", g_VP8VideoModeTbl[i].minBitRate);
		PTRACE2INT(eLevelInfoNormal, "CVP8VideoMode::GetVp8VideoParamsByRate - found mode - min bit rate is :g_VP8VideoModeTbl[i].maxBitRate = ", g_VP8VideoModeTbl[i].maxBitRate);

		if (g_VP8VideoModeTbl[i].videoModeType <= maxVideoMode && callRate >= g_VP8VideoModeTbl[i].minBitRate && callRate <= g_VP8VideoModeTbl[i].maxBitRate)
		{
			if(aspectRatio == E_VIDEO_RES_ASPECT_RATIO_DUMMY || aspectRatio == g_VP8VideoModeTbl[i].aspectRatio)
			{
				VP8VideoModeDetails.maxBitRate 		= min(g_VP8VideoModeTbl[i].maxBitRate,callRate) ;
				VP8VideoModeDetails.minBitRate 		= min(g_VP8VideoModeTbl[i].minBitRate,callRate) ;
				VP8VideoModeDetails.maxFrameRate 	= g_VP8VideoModeTbl[i].maxFrameRate;
				VP8VideoModeDetails.aspectRatio 	= g_VP8VideoModeTbl[i].aspectRatio;
				VP8VideoModeDetails.maxWidth 		= g_VP8VideoModeTbl[i].maxWidth;
				VP8VideoModeDetails.maxHeight 		= g_VP8VideoModeTbl[i].maxHeight;
				VP8VideoModeDetails.videoModeType 	= g_VP8VideoModeTbl[i].videoModeType;
				VP8VideoModeDetails.maxNumOfPixels 	= g_VP8VideoModeTbl[i].maxNumOfPixels;
				VP8VideoModeDetails.maxFS			= g_VP8VideoModeTbl[i].maxFS;
				VP8VideoModeDetails.maxMBPS 		= g_VP8VideoModeTbl[i].maxMBPS;

				PTRACE2INT(eLevelInfoNormal, "CVP8VideoMode::GetVp8VideoParamsByRate - found mode - min bit rate is :", VP8VideoModeDetails.minBitRate);
				PTRACE2INT(eLevelInfoNormal, "CVP8VideoMode::GetVp8VideoParamsByRate - found mode - VP8VideoModeDetails.maxMBPS is :", VP8VideoModeDetails.maxMBPS);
				PTRACE2INT(eLevelInfoNormal, "CVP8VideoMode::GetVp8VideoParamsByRate - found mode - VP8VideoModeDetails.maxFS is :", VP8VideoModeDetails.maxFS);
				return;

			}

		}

	}
	PTRACE(eLevelError, "CVP8VideoMode::GetVp8VideoParamsByRate - no VP8 params found for this bit/rate and aspect ratio!!!");

}

