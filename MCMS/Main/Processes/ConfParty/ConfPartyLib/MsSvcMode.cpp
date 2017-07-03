//-------------------------------------------------------------------------|
// FILE:       H264VideoMode.CPP                                           |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Keren                                                       |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |13/09/06    |                                                      |
//+========================================================================+

#include  "MsSvcMode.h"
#include  "H264Util.h"
// Global variables:
//notice-max bit rate -> for  hd720 MS-SVC according msft descion matrix 2500000 but as we don't have HD 108 we put 4m for now!
MsSvcVideoModeDetails CMsSvcVideoMode::g_MsSvcVideoModeTbl[MAX_MS_SVC_UNIQUE_MODES] =
{
//        videoModeType          Width			        Height        		 frame rate          aspect ratio			            minbitrate	    maxbitrate       MAX NUM OF PIXES
		{ eHD1080Symmetric,      1920,               	1088                       ,30  		,E_VIDEO_RES_ASPECT_RATIO_16_9,     1500000,         4000000,         2088960 },
		{ eHD720Symmetric,       1280,               	720                        ,30  		,E_VIDEO_RES_ASPECT_RATIO_16_9,     700000,         20000000,          921600},
		{ eW4CIF30,              960,             	    540                        ,30  		,E_VIDEO_RES_ASPECT_RATIO_16_9,     500000,         2000000,           518400 },
		{ eW4CIF30,              848,             	    480                        ,30  		,E_VIDEO_RES_ASPECT_RATIO_16_9,     400000,         1500000,           407040 },
		{ eW4CIF30,              640,             	    360                        ,30  		,E_VIDEO_RES_ASPECT_RATIO_16_9,     300000,         800000,            230400 },
		{ eW4CIF30,              640,             	    480                        ,30  		,E_VIDEO_RES_ASPECT_RATIO_4_3,      300000,         800000,            307200 },
		{ eW4CIF30,              480,             	    270                        ,30  		,E_VIDEO_RES_ASPECT_RATIO_16_9,     200000,         450000,            129600 },
		{ eW4CIF30,              424,             	    320                        ,30  		,E_VIDEO_RES_ASPECT_RATIO_4_3,      200000,          450000,           135680 },
		{ eW4CIF30,              424,             	    240                        ,30  		,E_VIDEO_RES_ASPECT_RATIO_16_9,     100000,          350000,           101760 },
		{ eCIF30,                320,             	    240                        ,30  		,E_VIDEO_RES_ASPECT_RATIO_4_3,      100000,          350000,           76800  },
		{ eCIF30,                212,             	    160                        ,30  		,E_VIDEO_RES_ASPECT_RATIO_4_3,      15000,          250000,            33920  },
		{ eCIF30,                320,             	    180                        ,30  		,E_VIDEO_RES_ASPECT_RATIO_16_9,      15000,          250000,           57600  }
};
///////////////////////////////////////////////////////////////////////////////////////////////////

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Constructor
CMsSvcVideoMode::CMsSvcVideoMode()
{
}
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// this function returns the MS SVC specific params according to rate and aspectratio if it is not E_VIDEO_RES_ASPECT_RATIO_DUMMY
void CMsSvcVideoMode::GetMsSvcVideoParamsByRate(MsSvcVideoModeDetails& MsSvcVideoModeDetails, DWORD callRate,Eh264VideoModeType maxVideoMode,DWORD aspectRatio)
{
  MsSvcVideoModeDetails.videoModeType = eInvalidModeType;
  PTRACE2INT(eLevelInfoNormal, "CMsSvcVideoMode::GetMsSvcVideoParamsByRate - callRate:", callRate);
  MsSvcVideoModeDetails.maxBitRate = -1;
  MsSvcVideoModeDetails.minBitRate = -1;
  MsSvcVideoModeDetails.maxFrameRate = -1;
  MsSvcVideoModeDetails.aspectRatio = -1;
  MsSvcVideoModeDetails.maxWidth = -1;
  MsSvcVideoModeDetails.maxHeight =-1;
  MsSvcVideoModeDetails.maxNumOfPixels =-1;
  MsSvcVideoModeDetails.videoModeType = eInvalidModeType;

  for(int i=0; i< MAX_MS_SVC_UNIQUE_MODES; i++)
  {
	if (g_MsSvcVideoModeTbl[i].videoModeType <= maxVideoMode && callRate >= g_MsSvcVideoModeTbl[i].minBitRate/*&& callRate <= g_MsSvcVideoModeTbl[i].maxBitRate */)
	{
		if(aspectRatio == E_VIDEO_RES_ASPECT_RATIO_DUMMY || aspectRatio == g_MsSvcVideoModeTbl[i].aspectRatio)
		{
			MsSvcVideoModeDetails.maxBitRate = g_MsSvcVideoModeTbl[i].maxBitRate;
			MsSvcVideoModeDetails.minBitRate = g_MsSvcVideoModeTbl[i].minBitRate;
			MsSvcVideoModeDetails.maxFrameRate = g_MsSvcVideoModeTbl[i].maxFrameRate;
			MsSvcVideoModeDetails.aspectRatio = g_MsSvcVideoModeTbl[i].aspectRatio;
			MsSvcVideoModeDetails.maxWidth = g_MsSvcVideoModeTbl[i].maxWidth;
			MsSvcVideoModeDetails.maxHeight = g_MsSvcVideoModeTbl[i].maxHeight;
			MsSvcVideoModeDetails.videoModeType = g_MsSvcVideoModeTbl[i].videoModeType;
			MsSvcVideoModeDetails.maxNumOfPixels = g_MsSvcVideoModeTbl[i].maxNumOfPixels;
		    PTRACE2INT(eLevelInfoNormal, "CMsSvcVideoMode::GetMsSvcVideoParamsByRate - found mode - min bit rate is :", MsSvcVideoModeDetails.minBitRate);
			return;

		}

	}

   }
  PTRACE(eLevelError, "CMsSvcVideoMode::GetMsSvcVideoParamsByRate - no MS SVC params found for this bit/rate and aspect ratio!!!");

}
void CMsSvcVideoMode::GetMsSvcModeIntersctionOfTwoCapsForBestMode(MsSvcVideoModeDetails& IntersctMsSvcVideoModeDetails,MsSvcVideoModeDetails First,MsSvcVideoModeDetails Second)
{
	  IntersctMsSvcVideoModeDetails.videoModeType = eInvalidModeType;
	  PTRACE(eLevelInfoNormal, "CMsSvcVideoMode::GetMsSvcModeIntersctionOfTwoCaps - ");
	  IntersctMsSvcVideoModeDetails.maxBitRate = -1;
	  IntersctMsSvcVideoModeDetails.minBitRate = -1;
	  IntersctMsSvcVideoModeDetails.maxFrameRate = -1;
	  IntersctMsSvcVideoModeDetails.aspectRatio = -1;
	  IntersctMsSvcVideoModeDetails.maxWidth = -1;
	  IntersctMsSvcVideoModeDetails.maxHeight =-1;
	  IntersctMsSvcVideoModeDetails.maxNumOfPixels =-1;
	  IntersctMsSvcVideoModeDetails.videoModeType = eInvalidModeType;
	  for(int i=0; i< MAX_MS_SVC_UNIQUE_MODES; i++)
	   {

	 	if ( First.maxWidth >= g_MsSvcVideoModeTbl[i].maxWidth
	 			&&  Second.maxWidth >= g_MsSvcVideoModeTbl[i].maxWidth
	 			&& First.maxHeight >= g_MsSvcVideoModeTbl[i].maxHeight
	 			&&  Second.maxHeight >= g_MsSvcVideoModeTbl[i].maxHeight
	 			&&  First.maxBitRate >= g_MsSvcVideoModeTbl[i].minBitRate
	 			&&  Second.maxBitRate >= g_MsSvcVideoModeTbl[i].minBitRate 
	 			&&  First.maxNumOfPixels >= g_MsSvcVideoModeTbl[i].maxNumOfPixels
	 			&&  Second.maxNumOfPixels >= g_MsSvcVideoModeTbl[i].maxNumOfPixels
	 			&& (First.aspectRatio == E_VIDEO_RES_ASPECT_RATIO_DUMMY || Second.aspectRatio == E_VIDEO_RES_ASPECT_RATIO_DUMMY ||
	 					g_MsSvcVideoModeTbl[i].aspectRatio == First.aspectRatio))
	 	     {

	 			IntersctMsSvcVideoModeDetails.maxBitRate = min(First.maxBitRate,Second.maxBitRate);
	 			IntersctMsSvcVideoModeDetails.maxBitRate = min(IntersctMsSvcVideoModeDetails.maxBitRate,g_MsSvcVideoModeTbl[i].maxBitRate);
	 			IntersctMsSvcVideoModeDetails.minBitRate = g_MsSvcVideoModeTbl[i].minBitRate;
	 			IntersctMsSvcVideoModeDetails.maxFrameRate = min(First.maxFrameRate,Second.maxFrameRate);
	 			IntersctMsSvcVideoModeDetails.aspectRatio = g_MsSvcVideoModeTbl[i].aspectRatio;
	 			IntersctMsSvcVideoModeDetails.maxWidth = g_MsSvcVideoModeTbl[i].maxWidth;
	 			IntersctMsSvcVideoModeDetails.maxHeight = g_MsSvcVideoModeTbl[i].maxHeight;
	 			IntersctMsSvcVideoModeDetails.videoModeType = g_MsSvcVideoModeTbl[i].videoModeType;
	 			IntersctMsSvcVideoModeDetails.maxNumOfPixels = g_MsSvcVideoModeTbl[i].maxNumOfPixels;
	 		    PTRACE2INT(eLevelInfoNormal, "CMsSvcVideoMode::GetMsSvcModeIntersctionOfTwoCaps - found mode - min bit rate is :", IntersctMsSvcVideoModeDetails.minBitRate);
	 			return;



	     	}

	   }

	  PTRACE(eLevelInfoNormal, "CMsSvcVideoMode::GetMsSvcModeIntersctionOfTwoCaps - mode not found!! ");

}
void CMsSvcVideoMode::ReturnMaxRateForMsAccordingToResolution(DWORD& MaxRate,DWORD Width,DWORD Height)
{

	  for(int i=0; i< MAX_MS_SVC_UNIQUE_MODES; i++)
	   {

		  if(Width <= g_MsSvcVideoModeTbl[i].maxWidth && Height <= g_MsSvcVideoModeTbl[i].maxHeight)
		  {
			  MaxRate =  g_MsSvcVideoModeTbl[i].maxBitRate;
			  return;
		  }


	   }

}

Eh264VideoModeType CMsSvcVideoMode::MinVideoModeForRes(DWORD width, DWORD height)
{
	//==========================================================================
	// Search will stop either at the first resolution that is >= the request,
	// or at the first entry, which is the default
	//==========================================================================
	int i = MAX_MS_SVC_UNIQUE_MODES - 1;
	while(i && (g_MsSvcVideoModeTbl[i].maxWidth < width || g_MsSvcVideoModeTbl[i].maxHeight < height))
	{
		--i;
	}

	return g_MsSvcVideoModeTbl[i].videoModeType;
}

Eh264VideoModeType CMsSvcVideoMode::GetBestVideoModeForVsrEntry(DWORD width, DWORD height)
{
	Eh264VideoModeType videoModeType = eCIF30;

	if(width == height){ // choose_by_width_only
		for(int i=0; i< MAX_MS_SVC_UNIQUE_MODES; i++)
		{
			  if(width >= g_MsSvcVideoModeTbl[i].maxWidth)
			  {
				  videoModeType  = g_MsSvcVideoModeTbl[i].videoModeType;
				  break;
			  }
		 }
	}else{
		videoModeType  = MinVideoModeForRes(width,height);
	}
	return videoModeType;
}


void CMsSvcVideoMode::AdjustFS(DWORD width, DWORD height, DWORD& fs)
{
	//============================================================================
	// SD res such as 480*270 should be bumped up to a more common SD resolution
	// so resources will be allocated properly.
	//============================================================================
	if (eW4CIF30 == CMsSvcVideoMode::MinVideoModeForRes(width, height))
	{
		fs = max(fs, (DWORD) H264_L2_2_DEFAULT_FS);
	}
}

//--------------------------------------------------------------------------------------------------
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


/*
 * MsSvcMode.cpp
 *
 *  Created on: Jul 30, 2013
 *      Author: nreiter
 */

void CMsSvcVideoMode::GetMsSvcVideoParamsByMaxH264VideoMode(Eh264VideoModeType maxH264VideoMode, MsSvcVideoModeDetails& MsSvcVideoModeDetails)
{
	MsSvcVideoModeDetails.videoModeType = eInvalidModeType;

	FTRACEINTO << "Trying to extract SVC Video params for Video Mode: " << maxH264VideoMode;
	MsSvcVideoModeDetails.maxBitRate = -1;
	MsSvcVideoModeDetails.minBitRate = -1;
	MsSvcVideoModeDetails.maxFrameRate = -1;
	MsSvcVideoModeDetails.aspectRatio = -1;
	MsSvcVideoModeDetails.maxWidth = -1;
	MsSvcVideoModeDetails.maxHeight =-1;
	MsSvcVideoModeDetails.maxNumOfPixels =-1;
	MsSvcVideoModeDetails.videoModeType = eInvalidModeType;

	for(int i=0; i< MAX_MS_SVC_UNIQUE_MODES; i++)
	{
		if (g_MsSvcVideoModeTbl[i].videoModeType <= maxH264VideoMode)
		{

			MsSvcVideoModeDetails.maxBitRate = g_MsSvcVideoModeTbl[i].maxBitRate;
			MsSvcVideoModeDetails.minBitRate = g_MsSvcVideoModeTbl[i].minBitRate;
			MsSvcVideoModeDetails.maxFrameRate = g_MsSvcVideoModeTbl[i].maxFrameRate;
			MsSvcVideoModeDetails.aspectRatio = g_MsSvcVideoModeTbl[i].aspectRatio;
			MsSvcVideoModeDetails.maxWidth = g_MsSvcVideoModeTbl[i].maxWidth;
			MsSvcVideoModeDetails.maxHeight = g_MsSvcVideoModeTbl[i].maxHeight;
			MsSvcVideoModeDetails.videoModeType = g_MsSvcVideoModeTbl[i].videoModeType;
			MsSvcVideoModeDetails.maxNumOfPixels = g_MsSvcVideoModeTbl[i].maxNumOfPixels;
			FPTRACE2INT(eLevelInfoNormal, "CMsSvcVideoMode::GetMsSvcVideoParamsByRate - found mode - min bit rate is :", MsSvcVideoModeDetails.minBitRate);
			return;
		}

	}
	FTRACEINTO << "no MS SVC params found for this video mode!!!!,  Video Mode: " << maxH264VideoMode;
}
