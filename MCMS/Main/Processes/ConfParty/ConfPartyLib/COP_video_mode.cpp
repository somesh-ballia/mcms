

#include "COP_video_mode.h"
#include "ObjString.h"
#include "H264Util.h"
#include "H221.h"
#include "CapInfo.h"
#include "ConfPartyGlobals.h"

#include "BridgePartyVideoParams.h"
#include "H323AddPartyControl.h"


//======================================================================================================================================================================//
// Tables - static data members
//======================================================================================================================================================================//
sCopDecoderH264Mode CCopVideoModeTable::g_CopDecoderH264ModeTable[NUM_OF_DECODER_MODES] =
{
//    videoModeType                   levelValue        maxMBPS		           maxFS			      maxDPB  maxBR   maxCPB	maxSAR                 maxStaticMbps
  { COP_decoder_resolution_CIF25,    { H264_Level_2,     -1,                     -1,                    -1,    -1,    -1,   H264_ALL_LEVEL_DEFAULT_SAR, -1} },
  { COP_decoder_resolution_4CIF25,   { H264_Level_2_2,   H264_L3_DEFAULT_MBPS,   -1,                    -1,    -1,    -1,   H264_ALL_LEVEL_DEFAULT_SAR, -1} },
  { COP_decoder_resolution_4CIF50, 	 { H264_Level_2_2,   H264_SD_60_MBPS,  		 -1,                    -1,    -1,    -1,   H264_ALL_LEVEL_DEFAULT_SAR, -1} },
  { COP_decoder_resolution_W4CIF25,  { H264_Level_2_2,   H264_W4CIF_30_MBPS,     H264_W4CIF_FS,         -1,    -1,    -1,   H264_ALL_LEVEL_DEFAULT_SAR, -1} },
  { COP_decoder_resolution_HD720p25, { H264_Level_3_1,  -1,                      H264_HD720_FS,         -1,    -1,    -1,   H264_ALL_LEVEL_DEFAULT_SAR, -1} },
  { COP_decoder_resolution_HD720p50, { H264_Level_3_1,   H264_HD720_60_MBPS,     H264_HD720_FS,         -1,    -1,    -1,   H264_ALL_LEVEL_DEFAULT_SAR, -1} },
  { COP_decoder_resolution_HD108030, { H264_Level_3_1,   H264_HD1080_30_MBPS,    H264_HD1080_FS,        -1,    -1,    -1,   H264_ALL_LEVEL_DEFAULT_SAR, -1} }

};
//======================================================================================================================================================================//

sCopEncoderH264Mode CCopVideoModeTable::g_CopEncoderH264ModeTable[NUM_OF_H264_ENCODER_MODES] =
{
	//ECopLevelEncoderVideoFormat    //ECopVideoFrameRate       //levelValue       maxMBPS  maxFS maxDPB maxBR maxCPB maxSAR maxStaticMbps
  { {eCopLevelEncoderVideoFormat_QCIF,eCopVideoFrameRate_12_5}, { H264_Level_1,     -1,    -1,    -1,    -1,    -1,    -1,      -1} },
  { {eCopLevelEncoderVideoFormat_QCIF,eCopVideoFrameRate_15},   { H264_Level_1,     -1,    -1,    -1,    -1,    -1,    -1,      -1} },
  { {eCopLevelEncoderVideoFormat_QCIF,eCopVideoFrameRate_25},   { H264_Level_1_1,   -1,    -1,    -1,    -1,    -1,    -1,      -1} },
  { {eCopLevelEncoderVideoFormat_QCIF,eCopVideoFrameRate_30},   { H264_Level_1_1,   -1,    -1,    -1,    -1,    -1,    -1,      -1} },

  { {eCopLevelEncoderVideoFormat_CIF,eCopVideoFrameRate_12_5},  { H264_Level_1_2,     -1,    -1,    -1,    -1,    -1,  -1,       -1} },
  { {eCopLevelEncoderVideoFormat_CIF,eCopVideoFrameRate_15},    { H264_Level_1_2,     -1,    -1,    -1,    -1,    -1,  -1,       -1} },
  { {eCopLevelEncoderVideoFormat_CIF,eCopVideoFrameRate_25},    { H264_Level_1_2,     H264_CIF_25_MBPS,    -1,    -1,    -1,    -1,  -1,       -1} },
  { {eCopLevelEncoderVideoFormat_CIF,eCopVideoFrameRate_30},    { H264_Level_1_2,     H264_CIF_25_MBPS,    -1,    -1,    -1,    -1,  -1,       -1} },

                                                                 //levelValue       maxMBPS                  maxFS maxDPB maxBR maxCPB maxSAR maxStaticMbps
  { {eCopLevelEncoderVideoFormat_4CIF,eCopVideoFrameRate_12_5},  { H264_Level_2_2,    -1,                     -1,    -1,    -1,    -1, -1,    -1} },
  { {eCopLevelEncoderVideoFormat_4CIF,eCopVideoFrameRate_15},    { H264_Level_2_2,    -1,                     -1,    -1,    -1,    -1, -1,    -1} },
  { {eCopLevelEncoderVideoFormat_4CIF,eCopVideoFrameRate_25},    { H264_Level_2_2,     H264_SD_30_MBPS,  -1,    -1,    -1,    -1, -1,    -1} },
  { {eCopLevelEncoderVideoFormat_4CIF,eCopVideoFrameRate_30},    { H264_Level_2_2,     H264_SD_30_MBPS,  -1,    -1,    -1,    -1, -1,     -1} },


                                                                       //levelValue       maxMBPS                 maxFS maxDPB maxBR maxCPB maxSAR maxStaticMbps
  { {eCopLevelEncoderVideoFormat_4CIF_16_9,eCopVideoFrameRate_12_5}, { H264_Level_2_2,  -1,                       -1,    -1,    -1,    -1,   4,    -1} },
  { {eCopLevelEncoderVideoFormat_4CIF_16_9,eCopVideoFrameRate_15},   { H264_Level_2_2,  -1,                       -1,    -1,    -1,    -1,   5,    -1} },
  { {eCopLevelEncoderVideoFormat_4CIF_16_9,eCopVideoFrameRate_25},   { H264_Level_2_2,     H264_SD_30_MBPS,  -1,    -1,    -1,    -1,   4,    -1} },
  { {eCopLevelEncoderVideoFormat_4CIF_16_9,eCopVideoFrameRate_30},   { H264_Level_2_2,     H264_SD_30_MBPS,  -1,    -1,    -1,    -1,   5,    -1} },

                                                                   //levelValue       maxMBPS                 maxFS           maxDPB maxBR maxCPB maxSAR maxStaticMbps
  { {eCopLevelEncoderVideoFormat_HD720p,eCopVideoFrameRate_12_5}, { H264_Level_3,     H264_HD720_15_MBPS,    H264_HD720_FS,    -1,    -1,    -1,  -1,     -1} },
  { {eCopLevelEncoderVideoFormat_HD720p,eCopVideoFrameRate_15},   { H264_Level_3,     H264_HD720_15_MBPS,    H264_HD720_FS,    -1,    -1,    -1,  -1,     -1} },
  { {eCopLevelEncoderVideoFormat_HD720p,eCopVideoFrameRate_25},   { H264_Level_3_1,   -1,                    H264_HD720_FS,    -1,    -1,    -1,  -1,     -1} },
  { {eCopLevelEncoderVideoFormat_HD720p,eCopVideoFrameRate_30},   { H264_Level_3_1,   -1,                    H264_HD720_FS,    -1,    -1,    -1,  -1,     -1} },
  { {eCopLevelEncoderVideoFormat_HD720p,eCopVideoFrameRate_50},   { H264_Level_3_1,   H264_HD720_60_MBPS,    H264_HD720_FS,    -1,    -1,    -1,  -1,   -1} },
  { {eCopLevelEncoderVideoFormat_HD720p,eCopVideoFrameRate_60},   { H264_Level_3_1,   H264_HD720_60_MBPS,    H264_HD720_FS,    -1,    -1,    -1,  -1,   -1} },

  { {eCopLevelEncoderVideoFormat_HD1080p,eCopVideoFrameRate_12_5}, { H264_Level_3_1,  H264_HD1080_15_MBPS,    H264_HD1080_FS,    -1,    -1,  -1,  -1,   -1} },
  { {eCopLevelEncoderVideoFormat_HD1080p,eCopVideoFrameRate_15},   { H264_Level_3_1,  H264_HD1080_15_MBPS,    H264_HD1080_FS,    -1,    -1,  -1,  -1,   -1} },
  { {eCopLevelEncoderVideoFormat_HD1080p,eCopVideoFrameRate_25},   { H264_Level_3_1,  H264_HD1080_30_MBPS,    H264_HD1080_FS,    -1,    -1,  -1,  -1,   -1} },
  { {eCopLevelEncoderVideoFormat_HD1080p,eCopVideoFrameRate_30},   { H264_Level_3_1,  H264_HD1080_30_MBPS,    H264_HD1080_FS,    -1,    -1,  -1,  -1,   -1} }
};
//======================================================================================================================================================================//
sCopEncoderH263H261Mode CCopVideoModeTable::g_CopEncoderH263ModeTable[NUM_OF_H263_ENCODER_MODES] =
{
		//ECopLevelEncoderVideoFormat    //ECopVideoFrameRate       //videoResolution          videoFrameRate
	  { {eCopLevelEncoderVideoFormat_QCIF,eCopVideoFrameRate_12_5}, { eVideoResolutionQCIF,     eVideoFrameRate12_5FPS, -1} },
	  { {eCopLevelEncoderVideoFormat_QCIF,eCopVideoFrameRate_15}, 	{ eVideoResolutionQCIF,     eVideoFrameRate15FPS, -1} },
	  { {eCopLevelEncoderVideoFormat_QCIF,eCopVideoFrameRate_25}, 	{ eVideoResolutionQCIF,     eVideoFrameRate25FPS, -1} },
	  { {eCopLevelEncoderVideoFormat_QCIF,eCopVideoFrameRate_30}, 	{ eVideoResolutionQCIF,     eVideoFrameRate30FPS, -1} },

	  { {eCopLevelEncoderVideoFormat_CIF,eCopVideoFrameRate_12_5},	{ eVideoResolutionCIF,     	eVideoFrameRate12_5FPS, -1} },
	  { {eCopLevelEncoderVideoFormat_CIF,eCopVideoFrameRate_15}, 	{ eVideoResolutionCIF,     	eVideoFrameRate15FPS, -1} },
	  { {eCopLevelEncoderVideoFormat_CIF,eCopVideoFrameRate_25}, 	{ eVideoResolutionCIF,     	eVideoFrameRate25FPS, -1} },
	  { {eCopLevelEncoderVideoFormat_CIF,eCopVideoFrameRate_30}, 	{ eVideoResolutionCIF,     	eVideoFrameRate30FPS, -1} },

	  { {eCopLevelEncoderVideoFormat_4CIF,eCopVideoFrameRate_12_5},	{ eVideoResolution4CIF,    	eVideoFrameRate12_5FPS, -1} },
	  { {eCopLevelEncoderVideoFormat_4CIF,eCopVideoFrameRate_15}, 	{ eVideoResolution4CIF,    	eVideoFrameRate15FPS, -1} },
	  { {eCopLevelEncoderVideoFormat_4CIF,eCopVideoFrameRate_25}, 	{ eVideoResolution4CIF,    	eVideoFrameRate25FPS, -1} },
	  { {eCopLevelEncoderVideoFormat_4CIF,eCopVideoFrameRate_30}, 	{ eVideoResolution4CIF,    	eVideoFrameRate30FPS, -1} },

	  { {eCopLevelEncoderVideoFormat_4CIF_16_9,eCopVideoFrameRate_12_5},{ eVideoResolution4CIF,    	eVideoFrameRate12_5FPS, 4} },
	  { {eCopLevelEncoderVideoFormat_4CIF_16_9,eCopVideoFrameRate_15}, 	{ eVideoResolution4CIF,    	eVideoFrameRate15FPS, 5} },
	  { {eCopLevelEncoderVideoFormat_4CIF_16_9,eCopVideoFrameRate_25}, 	{ eVideoResolution4CIF,    	eVideoFrameRate25FPS, 4} },
	  { {eCopLevelEncoderVideoFormat_4CIF_16_9,eCopVideoFrameRate_30}, 	{ eVideoResolution4CIF,    	eVideoFrameRate30FPS, 5} },


};
//======================================================================================================================================================================//
sCopEncoderH263H261Mode CCopVideoModeTable::g_CopEncoderH261ModeTable[NUM_OF_H261_ENCODER_MODES] =
{
		//ECopLevelEncoderVideoFormat    //ECopVideoFrameRate       //videoResolution          videoFrameRate
	  { {eCopLevelEncoderVideoFormat_QCIF,eCopVideoFrameRate_12_5}, { eVideoResolutionQCIF,     eVideoFrameRate12_5FPS, -1} },
	  { {eCopLevelEncoderVideoFormat_QCIF,eCopVideoFrameRate_15}, 	{ eVideoResolutionQCIF,     eVideoFrameRate15FPS, -1} },
	  { {eCopLevelEncoderVideoFormat_QCIF,eCopVideoFrameRate_25}, 	{ eVideoResolutionQCIF,     eVideoFrameRate25FPS, -1} },
	  { {eCopLevelEncoderVideoFormat_QCIF,eCopVideoFrameRate_30}, 	{ eVideoResolutionQCIF,     eVideoFrameRate30FPS, -1} },

	  { {eCopLevelEncoderVideoFormat_CIF,eCopVideoFrameRate_12_5},	{ eVideoResolutionCIF,     	eVideoFrameRate12_5FPS, -1} },
	  { {eCopLevelEncoderVideoFormat_CIF,eCopVideoFrameRate_15}, 	{ eVideoResolutionCIF,     	eVideoFrameRate15FPS, -1} },
	  { {eCopLevelEncoderVideoFormat_CIF,eCopVideoFrameRate_25}, 	{ eVideoResolutionCIF,     	eVideoFrameRate25FPS, -1} },
	  { {eCopLevelEncoderVideoFormat_CIF,eCopVideoFrameRate_30}, 	{ eVideoResolutionCIF,     	eVideoFrameRate30FPS, -1} },

};
//======================================================================================================================================================================//
// costructors
CCopVideoModeTable::CCopVideoModeTable()
{
}
//======================================================================================================================================================================//
CCopVideoModeTable::~CCopVideoModeTable()
{
}
//======================================================================================================================================================================//
// API
//======================================================================================================================================================================//
WORD CCopVideoModeTable::GetEncoderH264Mode(sCopEncoderFormatConfig copEncoderFormatConfig, sCopH264VideoMode& copH264VideoMode)
{
  WORD ret_value = (WORD)-1; // not found
  copH264VideoMode.levelValue = -1;
  copH264VideoMode.maxMBPS = -1;
  copH264VideoMode.maxFS = -1;
  copH264VideoMode.maxDPB = -1;
  copH264VideoMode.maxBR = -1;
  copH264VideoMode.maxCPB = -1;
  copH264VideoMode.maxSAR = -1;
  copH264VideoMode.maxStaticMbps = -1;

  for(WORD line_index=0;line_index<NUM_OF_H264_ENCODER_MODES;line_index++)
  {
    if((copEncoderFormatConfig.encoderFormat == g_CopEncoderH264ModeTable[line_index].copEncoderFormatConfig.encoderFormat )&&
       (copEncoderFormatConfig.encoderFrameRate == g_CopEncoderH264ModeTable[line_index].copEncoderFormatConfig.encoderFrameRate))
       {
       	copH264VideoMode.levelValue =    g_CopEncoderH264ModeTable[line_index].copH264VideoMode.levelValue;
       	copH264VideoMode.maxMBPS =       g_CopEncoderH264ModeTable[line_index].copH264VideoMode.maxMBPS;
       	copH264VideoMode.maxFS =         g_CopEncoderH264ModeTable[line_index].copH264VideoMode.maxFS;
       	copH264VideoMode.maxDPB =        g_CopEncoderH264ModeTable[line_index].copH264VideoMode.maxDPB;
       	copH264VideoMode.maxBR =         g_CopEncoderH264ModeTable[line_index].copH264VideoMode.maxBR;
       	copH264VideoMode.maxCPB =        g_CopEncoderH264ModeTable[line_index].copH264VideoMode.maxCPB;
       	copH264VideoMode.maxSAR = 		 g_CopEncoderH264ModeTable[line_index].copH264VideoMode.maxSAR;
    	copH264VideoMode.maxStaticMbps = g_CopEncoderH264ModeTable[line_index].copH264VideoMode.maxStaticMbps;

     	ret_value = line_index;
    	break;
       }
  }
  if(ret_value == (WORD)-1){
    CSmallString sstr;
    sstr << " encoderFormat = " << (WORD)copEncoderFormatConfig.encoderFormat << ", encoderFrameRate = " << (WORD)copEncoderFormatConfig.encoderFrameRate ;//<< ", encoderAspectRatio  = " << (WORD)copEncoderFormatConfig.encoderAspectRatio ;
    PTRACE2(eLevelInfoNormal,"CCopVideoModeTable::GetEncoderH264Mode faild to find encoder configuration: ", sstr.GetString());
  }
  return ret_value;
}
//======================================================================================================================================================================
WORD CCopVideoModeTable::GetH264COPEncoderBridgePartyVideoOutParams(sCopEncoderFormatConfig copEncoderFormatConfig, CBridgePartyVideoOutParams& bridgePartyVideoOutParams)
{
	WORD ret_value = (WORD)-1;
	sCopH264VideoMode copH264VideoMode;
	ret_value = GetEncoderH264Mode(copEncoderFormatConfig,copH264VideoMode);
	if(ret_value!=(WORD)-1)
	{
		bridgePartyVideoOutParams.SetVideoAlgorithm(H264);
		bridgePartyVideoOutParams.SetVideoResolution(ConvertCopEncoderFormatConfigToRes(copEncoderFormatConfig));
		bridgePartyVideoOutParams.SetVidFrameRate(ConvertCopLevelEncoderVideoFrameRateToFrameRate(copEncoderFormatConfig.encoderFrameRate));

		long maxMbps = copH264VideoMode.maxMBPS;
		if (maxMbps == -1)
		{
			APIU8 level = copH264VideoMode.levelValue;
			CH264Details h264Details = level;
			maxMbps = h264Details.GetDefaultMbpsAsProduct();
		}
		bridgePartyVideoOutParams.SetMBPS(GetMaxMbpsAsDevision(maxMbps));

		long maxFs = copH264VideoMode.maxFS;
		if (maxFs == -1)
		{
			APIU8 level = copH264VideoMode.levelValue;
			CH264Details h264Details = level;
			maxFs = h264Details.GetDefaultFsAsProduct();
		}
		bridgePartyVideoOutParams.SetFS(GetMaxFsAsDevision(maxFs));

		long maxSAR = copH264VideoMode.maxSAR;
		if(maxSAR == -1)
			maxSAR = DEFAULT_SAMPLE_ASPECT_RATIO;
		bridgePartyVideoOutParams.SetSampleAspectRatio(maxSAR);

		long maxStaticMbps = copH264VideoMode.maxStaticMbps;
		if (maxStaticMbps == -1)
			maxStaticMbps = DEFAULT_STATIC_MB;
		else
			maxStaticMbps = GetMaxMbpsAsDevision(maxStaticMbps);
		bridgePartyVideoOutParams.SetStaticMB(maxStaticMbps);

		long maxDPB = copH264VideoMode.maxDPB;
		if (maxDPB == -1)
		{
			APIU8 level = copH264VideoMode.levelValue;
			CH264Details h264Details = level;
			maxDPB = h264Details.GetDefaultDpbAsProduct();
		}
		bridgePartyVideoOutParams.SetMaxDPB(maxDPB);
	}
	return ret_value;
}
//======================================================================================================================================================================//
eVideoResolution CCopVideoModeTable::ConvertCopEncoderFormatConfigToRes(sCopEncoderFormatConfig eCopEncoderFormatConfig)
{
	eVideoResolution videoResolution = eVideoResolutionDummy;
	switch(eCopEncoderFormatConfig.encoderFormat)
	{
		case(eCopLevelEncoderVideoFormat_QCIF):
		{
			videoResolution = eVideoResolutionQCIF;
			break;
		}
		case(eCopLevelEncoderVideoFormat_CIF):
		{
			switch(eCopEncoderFormatConfig.encoderFrameRate)
			{
				case eCopVideoFrameRate_12_5:
				case eCopVideoFrameRate_25:
				{
					videoResolution = eVideoResolutionCIF;
					break;
				}
				case eCopVideoFrameRate_15:
				case eCopVideoFrameRate_30:
				{
					videoResolution = eVideoResolutionSIF;
					break;
				}
				default:
				{
					CSmallString sstr;
					sstr << " encoderFormat = " << (WORD)eCopEncoderFormatConfig.encoderFormat << ", encoderFrameRate = " << (WORD)eCopEncoderFormatConfig.encoderFrameRate ;
					PTRACE2(eLevelInfoNormal,"CCopVideoModeTable::ConvertCopEncoderFormatConfigToRes invalid encoder format and frame rate: ", sstr.GetString());

				}
			}
			break;
		}
		case(eCopLevelEncoderVideoFormat_4CIF):
		case(eCopLevelEncoderVideoFormat_4CIF_16_9):
		{
			switch(eCopEncoderFormatConfig.encoderFrameRate)
			{
				case eCopVideoFrameRate_12_5:
				case eCopVideoFrameRate_25:
				{
					videoResolution = eVideoResolution4CIF;
					break;
				}
				case eCopVideoFrameRate_15:
				case eCopVideoFrameRate_30:
				{
						videoResolution = eVideoResolution4SIF;
						break;
				}
				default:
				{
					CSmallString sstr;
					sstr << " encoderFormat = " << (WORD)eCopEncoderFormatConfig.encoderFormat << ", encoderFrameRate = " << (WORD)eCopEncoderFormatConfig.encoderFrameRate ;
					PTRACE2(eLevelInfoNormal,"CCopVideoModeTable::ConvertCopEncoderFormatConfigToRes invalid encoder format and frame rate: ", sstr.GetString());
				}
			}
			break;
		}
		case eCopLevelEncoderVideoFormat_HD720p:
		{
			videoResolution = eVideoResolutionHD720;
			break;
		}
		case eCopLevelEncoderVideoFormat_HD1080p:
		{
			videoResolution = eVideoResolutionHD1080;
			break;
		}
		default:
		{
			CSmallString sstr;
			sstr << " encoderFormat = " << (WORD)eCopEncoderFormatConfig.encoderFormat;
			PTRACE2(eLevelInfoNormal,"CCopVideoModeTable::ConvertCopEncoderFormatConfigToRes invalid encoder format: ", sstr.GetString());
			PASSERT(1);

		}
	}
	return videoResolution;
}
//======================================================================================================================================================================//

eVideoFrameRate CCopVideoModeTable::ConvertCopLevelEncoderVideoFrameRateToFrameRate(ECopVideoFrameRate eCopVideoFrameRate)
{
	eVideoFrameRate videoFrameRate =  eVideoFrameRateDUMMY;
	switch(eCopVideoFrameRate)
	{
		case eCopVideoFrameRate_12_5:
		{
			videoFrameRate =eVideoFrameRate12_5FPS;
			break;
		}
		case eCopVideoFrameRate_15:
		{
			videoFrameRate =eVideoFrameRate15FPS;
			break;
		}
		case eCopVideoFrameRate_25:
		{
			videoFrameRate =eVideoFrameRate25FPS;
			break;
		}
		case eCopVideoFrameRate_30:
		{
			videoFrameRate = eVideoFrameRate30FPS;
			break;
		}
		case eCopVideoFrameRate_50:
		{
			videoFrameRate = eVideoFrameRate50FPS;
			break;
		}
		case eCopVideoFrameRate_60:
		{
			videoFrameRate = eVideoFrameRate60FPS;
			break;
		}
		default:
		{
			CSmallString sstr;
			sstr << " ECopVideoFrameRate = " << (WORD)eCopVideoFrameRate;
			PTRACE2(eLevelInfoNormal,"CCopVideoModeTable::ConvertCopLevelEncoderVideoFrameRateToFrameRate invalid COP frame rate: ", sstr.GetString());
			PASSERT(1);

		}
	}
	return videoFrameRate;
}

//======================================================================================================================================================================//
WORD CCopVideoModeTable::GetDecoderH264Mode(ECopDecoderResolution copDecoderResolution, sCopH264VideoMode& copH264VideoMode)
{  WORD ret_value = (WORD)-1; // not found
  copH264VideoMode.levelValue = -1;
  copH264VideoMode.maxMBPS = -1;
  copH264VideoMode.maxFS = -1;
  copH264VideoMode.maxDPB = -1;
  copH264VideoMode.maxBR = -1;
  copH264VideoMode.maxCPB = -1;
  copH264VideoMode.maxSAR = -1;
  copH264VideoMode.maxStaticMbps = -1;

  for(WORD line_index=0;line_index<NUM_OF_DECODER_MODES;line_index++){
    if(copDecoderResolution == g_CopDecoderH264ModeTable[line_index].copDecoderResolution ){

      copH264VideoMode.levelValue = g_CopDecoderH264ModeTable[line_index].copH264VideoMode.levelValue;
      copH264VideoMode.maxMBPS = g_CopDecoderH264ModeTable[line_index].copH264VideoMode.maxMBPS;
      copH264VideoMode.maxFS = g_CopDecoderH264ModeTable[line_index].copH264VideoMode.maxFS;
      copH264VideoMode.maxDPB = g_CopDecoderH264ModeTable[line_index].copH264VideoMode.maxDPB;
      copH264VideoMode.maxBR = g_CopDecoderH264ModeTable[line_index].copH264VideoMode.maxBR;
      copH264VideoMode.maxCPB = g_CopDecoderH264ModeTable[line_index].copH264VideoMode.maxCPB;
      copH264VideoMode.maxSAR = g_CopDecoderH264ModeTable[line_index].copH264VideoMode.maxSAR;
      copH264VideoMode.maxStaticMbps = g_CopDecoderH264ModeTable[line_index].copH264VideoMode.maxStaticMbps;

      ret_value = line_index;
      break;
    }
  }
  if(ret_value == (WORD)-1){
    CSmallString sstr;
    sstr << " copDecoderResolution = " << (WORD)copDecoderResolution;
    PTRACE2(eLevelInfoNormal,"CCopVideoModeTable::GetDecoderH264Mode faild to find decoder resolution: ", sstr.GetString());
  }
  else
  {
	  GetValuesAsDevision(copH264VideoMode);
  }
  return ret_value;
}
//======================================================================================================================================================================//
void CCopVideoModeTable::GetDecoderH263Mode(ECopDecoderResolution copDecoderResolution, int& qcifMpi, int& cifMpi,int& cif4Mpi,int& cif16Mpi)
{
	qcifMpi = -1, cifMpi = -1, cif4Mpi = -1, cif16Mpi = -1;

	switch(copDecoderResolution)
	{
		case(COP_decoder_resolution_HD108030):
		case(COP_decoder_resolution_HD720p50):
		case(COP_decoder_resolution_HD720p25):
		{
			qcifMpi = 1;
			cifMpi  = 1;
			cif4Mpi = 2;
			break;

		}
		case(COP_decoder_resolution_W4CIF25):
		case(COP_decoder_resolution_4CIF50):
		case(COP_decoder_resolution_4CIF25):
		{
			qcifMpi = 1;
			cifMpi  = 1;
			cif4Mpi = 2;
			break;
		}
		case (COP_decoder_resolution_CIF25):
		{
			qcifMpi = 1;
			cifMpi  = 1;
			break;
		}
		default:
		{
			PASSERT(copDecoderResolution);
		}
	}
}

//======================================================================================================================================================================//
void CCopVideoModeTable::GetDecoderH264ResolustionFromMbpsFs(ECopDecoderResolution& copDecoderResolution, long fs, long mbps)
{

	DWORD mbpsAsProduct = ConvertMaxMbpsToProduct(mbps);
	DWORD fsAsProduct = ConvertMaxFsToProduct(fs);
	DWORD cifMaxFSAsProduct = 512;//round up the level 1.1 fs in 256k


	if(fsAsProduct>cifMaxFSAsProduct)
	{
		//HD72030 OR HD720 50/60 or HD1080 in MPMX
		if(fsAsProduct>H264_W4CIF_FS)
		{
			if(mbpsAsProduct<H264_HD720_50_MBPS)
			{
				copDecoderResolution = COP_decoder_resolution_HD720p25;

			}
			else
			{
				//1080DECODER IN MPMX
				eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
				if((fsAsProduct>H264_HD720_FS)&&(eSystemCardsMode_breeze==systemCardsBasedMode))
				{
					copDecoderResolution =COP_decoder_resolution_HD108030;
				}
				else
				{

					copDecoderResolution = COP_decoder_resolution_HD720p50;
				}
			}

		}
		//W4CIF OR 4CIF
		else
		{
			if(fsAsProduct ==H264_W4CIF_FS)
			{
				copDecoderResolution = COP_decoder_resolution_W4CIF25;
			}
			else
			{
				if(mbpsAsProduct<H264_4CIF_50_MBPS)
				{
					copDecoderResolution = COP_decoder_resolution_4CIF25;;
				}
				else
				{
					copDecoderResolution = COP_decoder_resolution_4CIF50;
				}
			}

		}

	}
	else
	{
		copDecoderResolution = COP_decoder_resolution_CIF25;
	}
}
//======================================================================================================================================================================//
WORD CCopVideoModeTable::GetSignalingH264ModeAccordingToReservationParams(CCopVideoParams* pCopVideoParams, sCopH264VideoMode& copH264VideoMode, BOOL bIsUse1080, DWORD rate)
{
	WORD retValue = (WORD)-1;

	if (pCopVideoParams->GetProtocol() == VIDEO_PROTOCOL_H264 || pCopVideoParams->GetProtocol() == VIDEO_PROTOCOL_H264_HIGH_PROFILE)
	{
		sCopEncoderFormatConfig copEncoderFormatConfig;
		ConvertReservationParamsToCopEncoderFormatConfig(pCopVideoParams, copEncoderFormatConfig, bIsUse1080, rate);
		retValue = GetEncoderH264Mode(copEncoderFormatConfig, copH264VideoMode);
		if (retValue != (WORD)-1)
			GetValuesAsDevision(copH264VideoMode);
	}
	else
		DBGPASSERT(pCopVideoParams->GetProtocol()+1000);

	return retValue;
}
//======================================================================================================================================================================//
WORD CCopVideoModeTable::GetSignalingH263H261ModeAccordingToReservationParams(CCopVideoParams* pCopVideoParams, BOOL bIsUse1080, int& qcifMpi, int& cifMpi,int& cif4Mpi,int& cif16Mpi)
{
	WORD retValue = (WORD)-1;
	qcifMpi = -1, cifMpi = -1, cif4Mpi = -1, cif16Mpi = -1;

	BYTE protocol = pCopVideoParams->GetProtocol();
	if ((protocol == VIDEO_PROTOCOL_H263)||(protocol == VIDEO_PROTOCOL_H261))
	{
		sCopEncoderFormatConfig copEncoderFormatConfig;
		sCopH263H261VideoMode copH263H261VideoMode;
		ConvertReservationParamsToCopEncoderFormatConfig(pCopVideoParams, copEncoderFormatConfig, bIsUse1080);
		if(protocol == VIDEO_PROTOCOL_H263)
			retValue = GetEncoderH263Mode(copEncoderFormatConfig, copH263H261VideoMode);
		else
			retValue = GetEncoderH261Mode(copEncoderFormatConfig, copH263H261VideoMode);
		if (retValue != (WORD)-1)
		{
			CH323AddPartyCntl partyCntl;//can't use CPartyCntl cause it's virtual
			int ipMPI = partyCntl.TranslateVideoBridgeFrameRateToIpMpi(copH263H261VideoMode.videoFrameRate);
			ipMPI = 30/ipMPI;
			eVideoResolution videoResolution = copH263H261VideoMode.videoResolution;
			switch(videoResolution)
			{
				case (eVideoResolutionQCIF):
				{
					qcifMpi =  ipMPI;
					break;
				}
				case (eVideoResolutionCIF):
				{
					cifMpi =  ipMPI;
					break;
				}
				case (eVideoResolution4CIF):
				{
					cif4Mpi =  ipMPI;
					break;
				}
				case (eVideoResolution16CIF):
				{
					cif16Mpi =  ipMPI;
					break;
				}
				default:
				{
					   PTRACE2INT(eLevelInfoNormal,"CCopVideoModeTable::GetSignalingH263ModeAccordingToReservationParams not in use resolution:",videoResolution);
					   break;
				}

			}

		}
	}
	else
		DBGPASSERT(pCopVideoParams->GetProtocol()+1000);

	return retValue;
}
//======================================================================================================================================================================//
void CCopVideoModeTable::ConvertReservationParamsToCopEncoderFormatConfig(CCopVideoParams* pCopVideoParams, sCopEncoderFormatConfig& copEncoderFormatConfig, BOOL bIsUse1080, DWORD rate)
{
	PTRACE(eLevelInfoNormal,"CCopVideoModeTable::ConvertReservationParamsToCopEncoderFormatConfig ");	// Get format:
	copEncoderFormatConfig.encoderFormat = (ECopLevelEncoderVideoFormat)(pCopVideoParams->GetFormat());

	ECopLevelEncoderVideoFormat formatBeforeRateAdaptation = (ECopLevelEncoderVideoFormat)(pCopVideoParams->GetFormat());
	//PTRACE2INT(eLevelInfoNormal,"CCopVideoModeTable::ConvertReservationParamsToCopEncoderFormatConfig ",formatBeforeRateAdaptation);
	// Get frame rate:
	copEncoderFormatConfig.encoderFrameRate = (ECopVideoFrameRate)(pCopVideoParams->GetFrameRate());
	// Change format according to rate thresholds if needed:
	if (((pCopVideoParams->GetProtocol() == VIDEO_PROTOCOL_H264)||(pCopVideoParams->GetProtocol() == VIDEO_PROTOCOL_H264_HIGH_PROFILE) )&& (rate != 0xFFFFFFFF) && (rate < GetMinBitRateForCopLevel(copEncoderFormatConfig.encoderFormat,copEncoderFormatConfig.encoderFrameRate, pCopVideoParams->GetProtocol())))
	{
		ECopLevelEncoderVideoFormat format =  (ECopLevelEncoderVideoFormat)(GetMaxCopLevelForBitRate(rate));
		ECopVideoFrameRate frameRate = copEncoderFormatConfig.encoderFrameRate;

		copEncoderFormatConfig.encoderFormat = format;
		if((format == eCopLevelEncoderVideoFormat_HD720p || formatBeforeRateAdaptation == eCopLevelEncoderVideoFormat_HD720p)&& ((frameRate == eCopVideoFrameRate_50 )||(frameRate == eCopVideoFrameRate_60)) && rate< H264_COP_THRESHOLD_720_50)
		{
			PTRACE(eLevelInfoNormal,"CCopVideoModeTable::ConvertReservationParamsToCopEncoderFormatConfig HD720@50/60 threshold :");

			if(frameRate == eCopVideoFrameRate_50)
				copEncoderFormatConfig.encoderFrameRate =eCopVideoFrameRate_25;
			else
				copEncoderFormatConfig.encoderFrameRate =eCopVideoFrameRate_30;

			//PTRACE(eLevelInfoNormal,"CCopVideoModeTable::ConvertReservationParamsToCopEncoderFormatConfig HD720@50/60 threshold :");
		}
	}
	if (!bIsUse1080 && copEncoderFormatConfig.encoderFormat == eCopLevelEncoderVideoFormat_HD1080p)
		copEncoderFormatConfig.encoderFormat = eCopLevelEncoderVideoFormat_HD720p;




}
//======================================================================================================================================================================//
void CCopVideoModeTable::GetValuesAsDevision(sCopH264VideoMode& copH264VideoMode)
{
	if(copH264VideoMode.maxMBPS != -1)
	{
		copH264VideoMode.maxMBPS = (DWORD)GetMaxMbpsAsDevision(copH264VideoMode.maxMBPS);
	}
	if(copH264VideoMode.maxFS != -1)
	{
		copH264VideoMode.maxFS = (DWORD) GetMaxFsAsDevision(copH264VideoMode.maxFS);
	}
	if(copH264VideoMode.maxDPB !=-1)
	{
		copH264VideoMode.maxDPB = (DWORD) GetMaxDpbAsDevision(copH264VideoMode.maxDPB);
	}
	if (copH264VideoMode.maxBR !=-1)
	{
		copH264VideoMode.maxBR = (DWORD)GetMaxBrAsDevision(copH264VideoMode.maxBR);
	}
	if (copH264VideoMode.maxStaticMbps !=-1)
	{
		copH264VideoMode.maxStaticMbps = (DWORD)GetMaxMbpsAsDevision(copH264VideoMode.maxStaticMbps);
	}
}
//======================================================================================================================================================================//
void CCopVideoModeTable::GetCOPEncoderBridgePartyVideoOutParams(CCopVideoParams* pCopEncoderLevelParams,CBridgePartyVideoOutParams& pOutVideoParams)
{
	BYTE videoProtocol = pCopEncoderLevelParams->GetProtocol();
	sCopEncoderFormatConfig copEncoderFormatConfig;
	BOOL bIsUse1080 = TRUE;
	ConvertReservationParamsToCopEncoderFormatConfig(pCopEncoderLevelParams, copEncoderFormatConfig, bIsUse1080);
	switch (videoProtocol)
	{
		case (VIDEO_PROTOCOL_H264):
		{

			GetH264COPEncoderBridgePartyVideoOutParams(copEncoderFormatConfig,pOutVideoParams);
			DWORD vidBitRate = GetVideoBitRate(pCopEncoderLevelParams);
			pOutVideoParams.SetVideoBitRate(vidBitRate);
			pOutVideoParams.SetProfile(eVideoProfileBaseline);
			pOutVideoParams.SetPacketFormat(eVideoPacketPayloadFormatSingleUnit);
			break;
		}
		case (VIDEO_PROTOCOL_H264_HIGH_PROFILE):
		{
			PTRACE(eLevelInfoNormal,"CCopVideoModeTable::GetCOPEncoderBridgePartyVideoOutParams -HIGH PROFILE");
			GetH264COPEncoderBridgePartyVideoOutParams(copEncoderFormatConfig,pOutVideoParams);
			DWORD vidBitRate = GetVideoBitRate(pCopEncoderLevelParams);
			pOutVideoParams.SetVideoBitRate(vidBitRate);
			pOutVideoParams.SetProfile(eVideoProfileHigh);
			pOutVideoParams.SetPacketFormat(eVideoPacketPayloadFormatFragmentationUnit);
			break;
		}
		case(VIDEO_PROTOCOL_H263):
		case(VIDEO_PROTOCOL_H261):
		{
			BOOL bIsUse1080 = TRUE;

			GetH263H261COPEncoderBridgePartyVideoOutParams(videoProtocol, copEncoderFormatConfig,pOutVideoParams);
			DWORD vidBitRate = GetVideoBitRate(pCopEncoderLevelParams);
			pOutVideoParams.SetVideoBitRate(vidBitRate);
			break;
		}
		default:
		{
			DBGPASSERT(pCopEncoderLevelParams->GetProtocol()+1000);
		}
	}
}
//======================================================================================================================================================================//
void CCopVideoModeTable::GetCOPLecturerCodecsVideoParamsByEncoder(CCopVideoParams* pCopEncoderLevelParams,CBridgePartyVideoOutParams& pOutVideoParams)
{
	BYTE videoProtocol = pCopEncoderLevelParams->GetProtocol();
	sCopEncoderFormatConfig copEncoderFormatConfig;
	BOOL bIsUse1080 = TRUE;
	ConvertReservationParamsToCopEncoderFormatConfig(pCopEncoderLevelParams, copEncoderFormatConfig, bIsUse1080);
	switch (videoProtocol)
	{
		case (VIDEO_PROTOCOL_H264):
		{

			eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
			// In MPM+ Lecturer decoders can not decode HD1080 so we set HD720
			if((copEncoderFormatConfig.encoderFormat == eCopLevelEncoderVideoFormat_HD1080p) && (eSystemCardsMode_mpm_plus==systemCardsBasedMode))
			{
				copEncoderFormatConfig.encoderFormat = eCopLevelEncoderVideoFormat_HD720p;
				if(copEncoderFormatConfig.encoderFrameRate > eCopVideoFrameRate_25)
				{
					copEncoderFormatConfig.encoderFrameRate = eCopVideoFrameRate_25;
				}
			}
			GetH264COPEncoderBridgePartyVideoOutParams(copEncoderFormatConfig,pOutVideoParams);
			DWORD vidBitRate = GetVideoBitRate(pCopEncoderLevelParams);
			pOutVideoParams.SetVideoBitRate(vidBitRate);
			pOutVideoParams.SetProfile(eVideoProfileBaseline);

			break;
		}
		case (VIDEO_PROTOCOL_H264_HIGH_PROFILE):
		{
			GetH264COPEncoderBridgePartyVideoOutParams(copEncoderFormatConfig,pOutVideoParams);
			DWORD vidBitRate = GetVideoBitRate(pCopEncoderLevelParams);
			pOutVideoParams.SetVideoBitRate(vidBitRate);
			pOutVideoParams.SetProfile(eVideoProfileHigh);
			break;
		}
		case(VIDEO_PROTOCOL_H263):
		case(VIDEO_PROTOCOL_H261):
		{
			BOOL bIsUse1080 = TRUE;

			GetH263H261COPEncoderBridgePartyVideoOutParams(videoProtocol, copEncoderFormatConfig,pOutVideoParams);
			DWORD vidBitRate = GetVideoBitRate(pCopEncoderLevelParams);
			pOutVideoParams.SetVideoBitRate(vidBitRate);
			break;
		}
		default:
		{
			DBGPASSERT(pCopEncoderLevelParams->GetProtocol()+1000);
		}
	}
}




//======================================================================================================================================================================//
DWORD CCopVideoModeTable::GetVideoBitRate(CCopVideoParams* pCopVideoParams)
{
	int levelRate = pCopVideoParams->GetBitRate();    // GUI rate
	CCapSetInfo  lCapInfo = eUnknownAlgorithemCapCode;
	DWORD levelBitRate = lCapInfo.TranslateReservationRateToIpRate(levelRate);
	levelBitRate *= 1000;
	DWORD audioRate = CalculateAudioRate(levelBitRate);
	DWORD videoRate = levelBitRate - audioRate;
	return videoRate;
}
//======================================================================================================================================================================
WORD CCopVideoModeTable::GetH263H261COPEncoderBridgePartyVideoOutParams(BYTE videoProtocol, sCopEncoderFormatConfig copEncoderFormatConfig, CBridgePartyVideoOutParams& bridgePartyVideoOutParams)
{
	WORD ret_value = (WORD)-1;
	sCopH263H261VideoMode copH263H261VideoMode;
	if(videoProtocol==VIDEO_PROTOCOL_H263)
	{
		ret_value = GetEncoderH263Mode(copEncoderFormatConfig,copH263H261VideoMode);
		bridgePartyVideoOutParams.SetVideoAlgorithm(H263);
	}
	if(videoProtocol==VIDEO_PROTOCOL_H261)
	{
		ret_value = GetEncoderH261Mode(copEncoderFormatConfig,copH263H261VideoMode);
		bridgePartyVideoOutParams.SetVideoAlgorithm(H261);

	}
	if(ret_value!=(WORD)-1)
	{
		bridgePartyVideoOutParams.SetVideoResolution(copH263H261VideoMode.videoResolution);
		bridgePartyVideoOutParams.SetVidFrameRate(copH263H261VideoMode.videoFrameRate);
		bridgePartyVideoOutParams.SetVideoFrameRate(copH263H261VideoMode.videoResolution, copH263H261VideoMode.videoFrameRate);
		long maxSAR = copH263H261VideoMode.maxSAR;
		if(maxSAR == -1)
			maxSAR = DEFAULT_SAMPLE_ASPECT_RATIO;
		bridgePartyVideoOutParams.SetSampleAspectRatio(maxSAR);

	}
	return ret_value;
}

//======================================================================================================================================================================
WORD CCopVideoModeTable::GetEncoderH263Mode(sCopEncoderFormatConfig copEncoderFormatConfig, sCopH263H261VideoMode& copH263H261VideoMode)
{
	  WORD ret_value = (WORD)-1; // not found
	  copH263H261VideoMode.videoFrameRate =eVideoFrameRateDUMMY;
	  copH263H261VideoMode.videoResolution = eVideoResolutionDummy;
	  for(WORD line_index=0;line_index<NUM_OF_H263_ENCODER_MODES;line_index++)
	  {
		   if((copEncoderFormatConfig.encoderFormat == g_CopEncoderH263ModeTable[line_index].copEncoderFormatConfig.encoderFormat )&&
		       (copEncoderFormatConfig.encoderFrameRate == g_CopEncoderH263ModeTable[line_index].copEncoderFormatConfig.encoderFrameRate))
		       {
				   copH263H261VideoMode.videoFrameRate = g_CopEncoderH263ModeTable[line_index].copH263H261VideoMode.videoFrameRate;
				   copH263H261VideoMode.videoResolution = g_CopEncoderH263ModeTable[line_index].copH263H261VideoMode.videoResolution;
				   copH263H261VideoMode.maxSAR = g_CopEncoderH263ModeTable[line_index].copH263H261VideoMode.maxSAR;

				   ret_value = line_index;
				   break;
		       }
	  }
	  if(ret_value == (WORD)-1)
	  {
		   CSmallString sstr;
		    sstr << " encoderFormat = " << (WORD)copEncoderFormatConfig.encoderFormat << ", encoderFrameRate = " << (WORD)copEncoderFormatConfig.encoderFrameRate ;//<< ", encoderAspectRatio  = " << (WORD)copEncoderFormatConfig.encoderAspectRatio ;
		    PTRACE2(eLevelInfoNormal,"CCopVideoModeTable::GetEncoderH263Mode failed to find encoder configuration: ", sstr.GetString());
	  }
	  return ret_value;
}
//======================================================================================================================================================================
WORD CCopVideoModeTable::GetEncoderH261Mode(sCopEncoderFormatConfig copEncoderFormatConfig, sCopH263H261VideoMode& copH263H261VideoMode)
{
	  WORD ret_value = (WORD)-1; // not found
	  copH263H261VideoMode.videoFrameRate =eVideoFrameRateDUMMY;
	  copH263H261VideoMode.videoResolution = eVideoResolutionDummy;
	  for(WORD line_index=0;line_index<NUM_OF_H261_ENCODER_MODES;line_index++)
	  {
		   if((copEncoderFormatConfig.encoderFormat == g_CopEncoderH261ModeTable[line_index].copEncoderFormatConfig.encoderFormat )&&
		       (copEncoderFormatConfig.encoderFrameRate == g_CopEncoderH261ModeTable[line_index].copEncoderFormatConfig.encoderFrameRate))
		       {
				   copH263H261VideoMode.videoFrameRate = g_CopEncoderH261ModeTable[line_index].copH263H261VideoMode.videoFrameRate;
				   copH263H261VideoMode.videoResolution = g_CopEncoderH261ModeTable[line_index].copH263H261VideoMode.videoResolution;
				   copH263H261VideoMode.maxSAR = g_CopEncoderH261ModeTable[line_index].copH263H261VideoMode.maxSAR;
				   ret_value = line_index;
				   break;
		       }
	  }
	  if(ret_value == (WORD)-1)
	  {
		   CSmallString sstr;
		    sstr << " encoderFormat = " << (WORD)copEncoderFormatConfig.encoderFormat << ", encoderFrameRate = " << (WORD)copEncoderFormatConfig.encoderFrameRate ;
		    PTRACE2(eLevelInfoNormal,"CCopVideoModeTable::GetEncoderH261Mode failed to find encoder configuration: ", sstr.GetString());
	  }
	  return ret_value;
}
//======================================================================================================================================================================//
void CCopVideoModeTable::GetPartyResolutionInPCMTerms(CCopVideoParams* pCopVideoParams, pcmImageParams& answer)
{
	ECopLevelEncoderVideoFormat encoderFormat = (ECopLevelEncoderVideoFormat)(pCopVideoParams->GetFormat());

	switch(encoderFormat)
	{
		case (eCopLevelEncoderVideoFormat_QCIF):
		case (eCopLevelEncoderVideoFormat_CIF):
		{
			answer = e320x240_4x3;
			break;
		}
		case (eCopLevelEncoderVideoFormat_4CIF):
		case (eCopLevelEncoderVideoFormat_4CIF_16_9):
		{
			answer = e640x480_4x3;
			break;
		}
		case (eCopLevelEncoderVideoFormat_HD720p):
		case (eCopLevelEncoderVideoFormat_HD1080p):
		{
			answer = e1024x576_16x9;
			break;
		}
		default:
		{
			PASSERT(encoderFormat);
			PTRACE2INT(eLevelInfoNormal,"unknown format: ",encoderFormat);
			answer = ePcmIllegalImageParams;
			break;
		}
	}

}
//======================================================================================================================================================================//
BYTE CCopVideoModeTable::UpdateH264VideoParamsAccordingToSupportredCopEncoderFormat(CBridgePartyVideoOutParams& bridgePartyVideoOutParams, DWORD maxFS, DWORD maxMBPS, BYTE isPAL)
{
	CMedString mstr;
	mstr << " maxFS = " << maxFS << " maxMBPS = " << maxMBPS << " isPAL = " << isPAL;
	PTRACE2(eLevelInfoNormal,"CCopVideoModeTable::UpdateH264VideoParamsAccordingToSupportredCopEncoderFormat : ", mstr.GetString());

	sCopEncoderFormatConfig copEncoderFormatConfig;
	BYTE bFoundMode = FALSE;
	for(int line_index=NUM_OF_H264_ENCODER_MODES-1;line_index>=0;line_index--)
	{
		copEncoderFormatConfig.encoderFormat = g_CopEncoderH264ModeTable[line_index].copEncoderFormatConfig.encoderFormat;
		copEncoderFormatConfig.encoderFrameRate = g_CopEncoderH264ModeTable[line_index].copEncoderFormatConfig.encoderFrameRate;
		BYTE isPalFrameRate = IsPalFrameRate(ConvertCopLevelEncoderVideoFrameRateToFrameRate(copEncoderFormatConfig.encoderFrameRate));

		if((isPalFrameRate && isPAL) || ((!isPalFrameRate) && (!isPAL)))
		{
			GetH264COPEncoderBridgePartyVideoOutParams(copEncoderFormatConfig, bridgePartyVideoOutParams);
			if(bridgePartyVideoOutParams.GetFS()<= maxFS && bridgePartyVideoOutParams.GetMBPS()<=maxMBPS)
			{
				CMedString str;
				str << " encoderFormat = " << (WORD)copEncoderFormatConfig.encoderFormat << ", encoderFrameRate = " << (WORD)copEncoderFormatConfig.encoderFrameRate ;
				PTRACE2(eLevelInfoNormal,"CCopVideoModeTable::UpdateH264VideoParamsAccordingToSupportredCopEncoderFormat : ", str.GetString());
				bFoundMode = TRUE;
				return  bFoundMode;
			}
		}

	}
	PTRACE(eLevelInfoNormal,"CCopVideoModeTable::FindMaxH264EncoderModeAccordingToH264Param didn't find mode");

	return bFoundMode;
}
//======================================================================================================================================================================//
BYTE CCopVideoModeTable::UpdateH263VideoParamsAccordingToSupportredCopEncoderFormat(CBridgePartyVideoOutParams& bridgePartyVideoOutParams, eVideoResolution videoResolution, BYTE isPAL/*12.5,25,50*/)
{
	CMedString mstr;
	mstr << " videoResolution = " << videoResolution << " isPAL = " << isPAL;
	PTRACE2(eLevelInfoNormal,"CCopVideoModeTable::UpdateH263VideoParamsAccordingToSupportredCopEncoderFormat : ", mstr.GetString());

	sCopEncoderFormatConfig copEncoderFormatConfig;
	BYTE bFoundMode = FALSE;

	for(int line_index=NUM_OF_H263_ENCODER_MODES-1;line_index>=0;line_index--)
	{
		copEncoderFormatConfig.encoderFormat = g_CopEncoderH263ModeTable[line_index].copEncoderFormatConfig.encoderFormat;
		copEncoderFormatConfig.encoderFrameRate = g_CopEncoderH263ModeTable[line_index].copEncoderFormatConfig.encoderFrameRate;
		BYTE isPalFrameRate = IsPalFrameRate(ConvertCopLevelEncoderVideoFrameRateToFrameRate(copEncoderFormatConfig.encoderFrameRate));

		if((isPalFrameRate && isPAL) || ((!isPalFrameRate) && (!isPAL)))
		{
			GetH263H261COPEncoderBridgePartyVideoOutParams(VIDEO_PROTOCOL_H263, copEncoderFormatConfig,bridgePartyVideoOutParams);
			if(bridgePartyVideoOutParams.GetVideoResolution()<= videoResolution)
			{
				CMedString str;
				str << " encoderFormat = " << (WORD)copEncoderFormatConfig.encoderFormat << ", encoderFrameRate = " << (WORD)copEncoderFormatConfig.encoderFrameRate ;
				PTRACE2(eLevelInfoNormal,"CCopVideoModeTable::UpdateH263HVideoParamsAccordingToSupportredCopEncoderFormat : ", str.GetString());
				bFoundMode = TRUE;
				return  bFoundMode;
			}
		}

	}
	PTRACE(eLevelInfoNormal,"CCopVideoModeTable::UpdateH263HVideoParamsAccordingToSupportredCopEncoderFormat didn't find mode");
	return bFoundMode;
}
//======================================================================================================================================================================//
BYTE CCopVideoModeTable::UpdateH261VideoParamsAccordingToSupportredCopEncoderFormat(CBridgePartyVideoOutParams& bridgePartyVideoOutParams, eVideoResolution videoResolution, BYTE isPAL/*12.5,25,50*/)
{
	CMedString mstr;
	mstr << " videoResolution = " << videoResolution << " isPAL = " << isPAL;
	PTRACE2(eLevelInfoNormal,"CCopVideoModeTable::UpdateH261VideoParamsAccordingToSupportredCopEncoderFormat : ", mstr.GetString());

	sCopEncoderFormatConfig copEncoderFormatConfig;
	BYTE bFoundMode = FALSE;

	for(int line_index=NUM_OF_H261_ENCODER_MODES-1;line_index>=0;line_index--)
	{
		copEncoderFormatConfig.encoderFormat = g_CopEncoderH261ModeTable[line_index].copEncoderFormatConfig.encoderFormat;
		copEncoderFormatConfig.encoderFrameRate = g_CopEncoderH261ModeTable[line_index].copEncoderFormatConfig.encoderFrameRate;
		BYTE isPalFrameRate = IsPalFrameRate(ConvertCopLevelEncoderVideoFrameRateToFrameRate(copEncoderFormatConfig.encoderFrameRate));
		if((isPalFrameRate && isPAL) || ((!isPalFrameRate) && (!isPAL)))
		{
			GetH263H261COPEncoderBridgePartyVideoOutParams(VIDEO_PROTOCOL_H261, copEncoderFormatConfig,bridgePartyVideoOutParams);
			if(bridgePartyVideoOutParams.GetVideoResolution()<= videoResolution)
			{
				CMedString str;
				str << " encoderFormat = " << (WORD)copEncoderFormatConfig.encoderFormat << ", encoderFrameRate = " << (WORD)copEncoderFormatConfig.encoderFrameRate ;
				PTRACE2(eLevelInfoNormal,"CCopVideoModeTable::UpdateH261HVideoParamsAccordingToSupportredCopEncoderFormat : ", str.GetString());
				bFoundMode = TRUE;
				return  bFoundMode;
			}
		}

	}
	PTRACE(eLevelInfoNormal,"CCopVideoModeTable::UpdateH261HVideoParamsAccordingToSupportredCopEncoderFormat didn't find mode");
	return bFoundMode;
}
