/*
 * RtvVideoMode.cpp
 *
 *  Created on: Sep 14, 2010
 *      Author: inga
 */
#include  "RtvVideoMode.h"


H264ToRTVVideoModes CRtvVideoMode::g_H264VideoModeTypesTranslateToRtvVideoModeTyps[MAX_H264_VIDEO_MODE] =
{
		//  H264videoModeType           //RtvVideoModeType
		{ eCIF30,	             e_rtv_CIF30},
		{ eCIF60,                e_rtv_CIF30},
		{ e2CIF30,               e_rtv_CIF30},
		{ eWCIF60,               e_rtv_CIF30},
		{ eSD15,                 e_rtv_VGA15},
		{ eSD30,                 e_rtv_VGA30},
		{ eSD60,                 e_rtv_HD720Symmetric},
		{ eW4CIF30,              e_rtv_VGA30},
		{ eHD720Asymmetric,      e_rtv_HD720Asymmetric},
		{ eHD720Symmetric,       e_rtv_HD720Symmetric},
		{ eHD720At60Asymmetric,  e_rtv_HD720Symmetric},
		{ eHD720At60Symmetric,   e_rtv_HD720Symmetric},
		{ eHD1080Asymmetric,     e_rtv_HD720Symmetric},
		{ eHD1080Symmetric,      e_rtv_HD720Symmetric},
		{ eHD1080At60Asymmetric, e_rtv_HD720Symmetric},
		{ eHD1080At60Symmetric,  e_rtv_HD720Symmetric},		
		//must be last:
		{ eLasth264VideoMode,    eLastRtvVideoMode }
};

RTVVideoModeDetails CRtvVideoMode::g_RtvVideoModeTbl[MAX_RTV_VIDEO_MODE] =
{
	//    videoModeType          Width		           Height			     FR
	{  e_rtv_QCIF15,		 176,				   144,					 15},
	{  e_rtv_QCIF30,		 176,				   144,					 30},
	{  e_rtv_CIF15,			 352,                  288,                  15},
	{  e_rtv_CIF30,	         352,                  288,                  30},
	{  e_rtv_2CIF30,	     352,                  576,                  30},
	{  e_rtv_WCIF30,         512,				   288,					 30},
	{  e_rtv_VGA15,          640,                  480,					 15},
	{  e_rtv_VGA30,          640,                  480,                  30},
	{  e_rtv_SD15,	         720,                  576,                  15},
	{  e_rtv_SD30,	         720,                  576,                  30},
	{  e_rtv_HD720Asymmetric,720,                  576,                  30},
	{  e_rtv_W4CIF30,        1024 ,                576,                  30},
	{  e_rtv_HD720Symmetric, 1280,                 720,                  30},
	//must be last:
	{  eLastRtvVideoMode,    0,						0,					 0 }
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Constructor
CRtvVideoMode::CRtvVideoMode()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CRtvVideoMode::GetRtvVideoParams(RTVVideoModeDetails& rtvVidModeDetails,Eh264VideoModeType videoModeType)
{
	rtvVidModeDetails.videoModeType = eLastRtvVideoMode;
	rtvVidModeDetails.Width = 0;
	rtvVidModeDetails.Height = 0;
	rtvVidModeDetails.FR = 0;

	for(int i=0; i<MAX_H264_VIDEO_MODE; i++)
	{
		if(g_H264VideoModeTypesTranslateToRtvVideoModeTyps[i].H264VideoModeType == videoModeType)
		{
			rtvVidModeDetails.videoModeType	= g_H264VideoModeTypesTranslateToRtvVideoModeTyps[i].RtvVideoModeType;

			for(int j=0; j<MAX_RTV_VIDEO_MODE; j++)
			{
				if(g_RtvVideoModeTbl[j].videoModeType == rtvVidModeDetails.videoModeType)
				{
					rtvVidModeDetails.Width = g_RtvVideoModeTbl[j].Width;
					rtvVidModeDetails.Height = g_RtvVideoModeTbl[j].Height;
					rtvVidModeDetails.FR = g_RtvVideoModeTbl[j].FR;

					return;
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CRtvVideoMode::GetRtvVideoParamsAndRate(RTVVideoModeDetails& rtvVidModeDetails,Eh264VideoModeType videoModeType,eVideoQuality videoQuality)
{
	eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();

	DWORD ThresholdRate = CResRsrcCalculator::GetRateAccordingToVideoModeType(systemCardsBasedMode,videoQuality,videoModeType);
	GetRtvVideoParams(rtvVidModeDetails,videoModeType);

	return ThresholdRate;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CRtvVideoMode::GetRtvQcifVideoParams(RTVVideoModeDetails& rtvVidModeDetails)
{
	rtvVidModeDetails.videoModeType = e_rtv_QCIF30;
	rtvVidModeDetails.Width = 0;
	rtvVidModeDetails.Height = 0;
	rtvVidModeDetails.FR = 0;

	for(int j=0; j<MAX_RTV_VIDEO_MODE; j++)
	{
		if(g_RtvVideoModeTbl[j].videoModeType == rtvVidModeDetails.videoModeType)
		{
			rtvVidModeDetails.Width = g_RtvVideoModeTbl[j].Width;
			rtvVidModeDetails.Height = g_RtvVideoModeTbl[j].Height;
			rtvVidModeDetails.FR = g_RtvVideoModeTbl[j].FR;

			return;

		}

	}
}
