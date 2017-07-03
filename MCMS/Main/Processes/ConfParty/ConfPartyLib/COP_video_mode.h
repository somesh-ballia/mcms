#ifndef _COP_VIDEO_MODE_H_
#define _COP_VIDEO_MODE_H_

#include "CopVideoParams.h"

#include "COP_Layout_definitions.h"
#include "ConfPartyApiDefines.h"
#include "IpChannelParams.h"
#include "BridgePartyVideoParams.h"

typedef struct
{
  ECopLevelEncoderVideoFormat encoderFormat;
  ECopVideoFrameRate encoderFrameRate;
 }sCopEncoderFormatConfig;

 //all the ECopLevelEncoderVideoFormat except HD720 support all the frame rates not include 50fps and 60fps
 // HD720 supports all include 50fps and 60fps
#define NUM_OF_H264_ENCODER_MODES ( ((DWORD)eCopLevelEncoderVideoFormat_Last-1) * ((DWORD)eCopVideoFrameRate_Last-2)) + (DWORD)eCopVideoFrameRate_Last
#define NUM_OF_H263_ENCODER_MODES 16 //QCIF/CIF/4CIF/4CIF16:9 12.5/15/25/30
#define NUM_OF_H261_ENCODER_MODES 8 //QCIF/CIF 12.5/15/25/30


typedef struct
{
  long 	    levelValue;
  long      maxMBPS;
  long      maxFS;
  long      maxDPB;
  long      maxBR;
  long      maxCPB;
  long      maxSAR;
  long      maxStaticMbps;
}sCopH264VideoMode;

typedef struct
{
  sCopEncoderFormatConfig copEncoderFormatConfig;
  sCopH264VideoMode copH264VideoMode;
}sCopEncoderH264Mode;

typedef struct
{
  ECopDecoderResolution copDecoderResolution;
  sCopH264VideoMode copH264VideoMode;
}sCopDecoderH264Mode;


typedef struct
{
	eVideoResolution videoResolution;
	eVideoFrameRate  videoFrameRate;
	long      maxSAR;
}sCopH263H261VideoMode;

typedef struct
{
  sCopEncoderFormatConfig     copEncoderFormatConfig;
  sCopH263H261VideoMode       copH263H261VideoMode;
}sCopEncoderH263H261Mode;


class CCopVideoModeTable : public CPObject
{
	CLASS_TYPE_1(CCopVideoModeTable ,CPObject)
public:
  // costructors
  CCopVideoModeTable();
  virtual ~CCopVideoModeTable();
  // CPObject
  virtual const char* NameOf() const { return "CCopVideoModeTable"; };


  // API
  WORD GetEncoderH264Mode(sCopEncoderFormatConfig copEncoderFormatConfig, sCopH264VideoMode& copH264VideoMode);
  WORD GetEncoderH263Mode(sCopEncoderFormatConfig copEncoderFormatConfig, sCopH263H261VideoMode& copH263h261VideoMode);
  WORD GetEncoderH261Mode(sCopEncoderFormatConfig copEncoderFormatConfig, sCopH263H261VideoMode& copH263H261VideoMode);

  WORD GetDecoderH264Mode(ECopDecoderResolution copDecoderResolution, sCopH264VideoMode& copH264VideoMode);
  void GetDecoderH264ResolustionFromMbpsFs(ECopDecoderResolution& copDecoderResolution, long fs,long mbps);
  void GetDecoderH263Mode(ECopDecoderResolution copDecoderResolution, int& qcifMpi, int& cifMpi,int& cif4Mpi,int& cif16Mpi);

  WORD GetH264COPEncoderBridgePartyVideoOutParams(sCopEncoderFormatConfig copEncoderFormatConfig, CBridgePartyVideoOutParams& bridgePartyVideoOutParams);
  WORD GetH263H261COPEncoderBridgePartyVideoOutParams(BYTE videoProtocol,sCopEncoderFormatConfig copEncoderFormatConfig, CBridgePartyVideoOutParams& bridgePartyVideoOutParams);

  eVideoResolution ConvertCopEncoderFormatConfigToRes(sCopEncoderFormatConfig eCopEncoderFormatConfig);
  eVideoFrameRate  ConvertCopLevelEncoderVideoFrameRateToFrameRate(ECopVideoFrameRate eCopVideoFrameRate);

  WORD GetSignalingH264ModeAccordingToReservationParams(CCopVideoParams* pCopVideoParams, sCopH264VideoMode& copH264VideoMode, BOOL bIsUse1080, DWORD rate = 0xFFFFFFFF);
  WORD GetSignalingH263H261ModeAccordingToReservationParams(CCopVideoParams* pCopVideoParams, BOOL bIsUse1080, int& qcifMpi, int& cifMpi,int& cif4Mpi,int& cif16Mpi);

  void ConvertReservationParamsToCopEncoderFormatConfig(CCopVideoParams* pCopVideoParams, sCopEncoderFormatConfig& copEncoderFormatConfig, BOOL bIsUse1080, DWORD rate = 0xFFFFFFFF);
  void GetValuesAsDevision(sCopH264VideoMode& copH264VideoMode);
  void GetCOPEncoderBridgePartyVideoOutParams(CCopVideoParams* pCopEncoderLevelParams,CBridgePartyVideoOutParams& pOutVideoParams);
  DWORD GetVideoBitRate(CCopVideoParams* pCopVideoParams);

  void GetPartyResolutionInPCMTerms(CCopVideoParams* pCopVideoParams, pcmImageParams& answer);

  void GetCOPLecturerCodecsVideoParamsByEncoder(CCopVideoParams* pCopEncoderLevelParams,CBridgePartyVideoOutParams& pOutVideoParams);

  BYTE UpdateH264VideoParamsAccordingToSupportredCopEncoderFormat(CBridgePartyVideoOutParams& bridgePartyVideoOutParams, DWORD maxFS, DWORD maxMBPS, BYTE isPAL);
  BYTE UpdateH263VideoParamsAccordingToSupportredCopEncoderFormat(CBridgePartyVideoOutParams& bridgePartyVideoOutParams, eVideoResolution videoResolution, BYTE isPAL);
  BYTE UpdateH261VideoParamsAccordingToSupportredCopEncoderFormat(CBridgePartyVideoOutParams& bridgePartyVideoOutParams, eVideoResolution videoResolution, BYTE isPAL);



  //void
private:

  static sCopEncoderH264Mode g_CopEncoderH264ModeTable[NUM_OF_H264_ENCODER_MODES];
  static sCopEncoderH263H261Mode g_CopEncoderH263ModeTable[NUM_OF_H263_ENCODER_MODES];
  static sCopEncoderH263H261Mode g_CopEncoderH261ModeTable[NUM_OF_H261_ENCODER_MODES];

  static sCopDecoderH264Mode g_CopDecoderH264ModeTable[NUM_OF_DECODER_MODES];
};


#endif //_COP_VIDEO_MODE_H_

//    CCopVideoModeTable videoModeTable;
//    videoModeTable.GetH264COPEncoderBridgePartyVideoOutParams(pCopEncoderLevelParams, *pOutVideoParams);

