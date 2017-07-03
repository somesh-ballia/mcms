/*
 * vp8VideoMode.h
 *
 *  Created on: Mar 30, 2014
 *      Author: nalster
 */

#ifndef VP8VIDEOMODE_H_
#define VP8VIDEOMODE_H_


#include  "PObject.h"
#include  "H264.h"
#include  "ConfPartyDefines.h"
#include  "ConfPartyGlobals.h"
#include  "ResRsrcCalculator.h"


#define MAX_H264_VIDEO_MODE  eLasth264VideoMode

#define MAX_VP8_UNIQUE_MODES 5


class CVP8VideoMode : public CPObject
{
	CLASS_TYPE_1(CVP8VideoMode, CPObject)
	public:

    // Constructors
	CVP8VideoMode ();
    virtual ~CVP8VideoMode(){}

    // Operations
    virtual const char* NameOf() const {return "CVP8VideoMode";}
    void GetVp8VideoParamsByRate(VP8VideoModeDetails& VP8VideoModeDetails, DWORD callRate,Eh264VideoModeType maxVideoMode,DWORD aspectRatio = E_VIDEO_RES_ASPECT_RATIO_DUMMY);
    void GetVp8ModeIntersctionOfTwoCapsForBestMode(VP8VideoModeDetails& IntersctVP8VideoModeDetails,VP8VideoModeDetails First,VP8VideoModeDetails Second);
    void ReturnMaxRateForMsAccordingToResolution(DWORD& MaxRate,DWORD Width,DWORD Height);


    //static Eh264VideoModeType	MinVideoModeForRes(DWORD width, DWORD height);

	private:

    // Data global array:
    static VP8VideoModeDetails g_VP8VideoModeTbl[MAX_VP8_UNIQUE_MODES];

 };


#endif /* VP8VIDEOMODE_H_ */
