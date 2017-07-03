/*
 * RtvVideoMode.h
 *
 *  Created on: Sep 14, 2010
 *      Author: inga
 */

#ifndef RTVVIDEOMODE_H_
#define RTVVIDEOMODE_H_

#include  "PObject.h"
#include  "ConfPartyApiDefines.h"
#include  "ConfPartyDefines.h"
#include  "H264VideoMode.h"

#define MAX_RTV_VIDEO_MODE  eLastRtvVideoMode

typedef struct
{
	DWORD              thresholdBitrate;
	ERtvVideoModeType  videoModeType;
} RTVVideoModeThresholdStruct;

class CRtvVideoMode : public CPObject
{
	CLASS_TYPE_1(CRtvVideoMode, CPObject)
	public:

    // Constructors
    CRtvVideoMode ();
    virtual ~CRtvVideoMode(){}

    virtual const char* NameOf() const {return "CH264VideoMode";}

    static void GetRtvVideoParams(RTVVideoModeDetails& rtvVidModeDetails,Eh264VideoModeType videoModeType);
    static DWORD GetRtvVideoParamsAndRate(RTVVideoModeDetails& rtvVidModeDetails,Eh264VideoModeType videoModeType,eVideoQuality videoQuality);
    static void GetRtvQcifVideoParams(RTVVideoModeDetails& rtvVidModeDetails);

	private:

    static RTVVideoModeDetails g_RtvVideoModeTbl[MAX_RTV_VIDEO_MODE];
    static H264ToRTVVideoModes g_H264VideoModeTypesTranslateToRtvVideoModeTyps[MAX_H264_VIDEO_MODE];
};

#endif /* RTVVIDEOMODE_H_ */
