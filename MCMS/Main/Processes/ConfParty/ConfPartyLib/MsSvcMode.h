/*
 * MsSvcMode.h
 *
 *  Created on: Jul 29, 2013
 *      Author: nreiter
 */

#ifndef MSSVCMODE_H_
#define MSSVCMODE_H_

#include  "PObject.h"
#include  "H264.h"
#include  "ConfPartyDefines.h"
#include  "ConfPartyGlobals.h"
#include  "ResRsrcCalculator.h"


#define MAX_H264_VIDEO_MODE  eLasth264VideoMode


#define MAX_MS_SVC_UNIQUE_MODES 12

//typedef struct  //moved to CResRsrcCalculator
//{
//	DWORD              thresholdBitrate;
//	Eh264VideoModeType videoModeType;
//} H264VideoModeThresholdStruct;

class CMsSvcVideoMode : public CPObject
{
	CLASS_TYPE_1(CMsSvcVideoMode, CPObject)
	public:

    // Constructors
	CMsSvcVideoMode ();
    virtual ~CMsSvcVideoMode(){}

    // Operations
    virtual const char* NameOf() const {return "CMsSvcVideoMode";}
    void GetMsSvcVideoParamsByRate(MsSvcVideoModeDetails& MsSvcVideoModeDetails, DWORD callRate,Eh264VideoModeType maxVideoMode,DWORD aspectRatio = E_VIDEO_RES_ASPECT_RATIO_DUMMY);
    void GetMsSvcModeIntersctionOfTwoCapsForBestMode(MsSvcVideoModeDetails& IntersctMsSvcVideoModeDetails,MsSvcVideoModeDetails First,MsSvcVideoModeDetails Second);
    void ReturnMaxRateForMsAccordingToResolution(DWORD& MaxRate,DWORD Width,DWORD Height);
  //  void GetH264VideoParams(H264VideoModeDetails& h264VidModeDetails, DWORD callRate,eVideoQuality videoQuality, Eh264VideoModeType maxVideoMode, BOOL isHighProfile = TRUE);
   // void GetH264AssymetricVideoParams(H264VideoModeDetails& h264VidModeDetails, DWORD callRate, eVideoQuality videoQuality,Eh264VideoModeType maxVideoMode, BOOL isHighProfile = TRUE);
   // Eh264VideoModeType GetH264VideoMode(DWORD callRate,eVideoQuality videoQuality, Eh264VideoModeType maxVideoMode, BOOL isHighProfile = TRUE);
    //void GetH264VideoModeDetailsAccordingToType(H264VideoModeDetails& h264VidModeDetails, Eh264VideoModeType h264VidModeType);

    static Eh264VideoModeType	MinVideoModeForRes(DWORD width, DWORD height);
    static void 				AdjustFS(DWORD width, DWORD height, DWORD& fs);
    static Eh264VideoModeType GetBestVideoModeForVsrEntry(DWORD width, DWORD height);
    static void GetMsSvcVideoParamsByMaxH264VideoMode(Eh264VideoModeType maxH264VideoMode, MsSvcVideoModeDetails& MsSvcVideoModeDetails);


	private:

    // Data global array:
    static MsSvcVideoModeDetails g_MsSvcVideoModeTbl[MAX_MS_SVC_UNIQUE_MODES];



 };




#endif /* MSSVCMODE_H_ */
