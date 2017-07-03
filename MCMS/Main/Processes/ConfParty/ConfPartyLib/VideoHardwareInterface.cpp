#include "VideoHardwareInterface.h"

#include "VideoStructs.h"
#include "VideoBridge.h"
#include "ConfPartyGlobals.h"
#include "IVRPlayMessage.h"
#include "IvrApiStructures.h"
#include "IVRSlidesList.h"
#include "OpcodesMcmsCardMngrIvrCntl.h"
#include "OpcodesMcmsVideo.h"
#include "OpcodesMcmsCardMngrTB.h"
#include "H264.h"
#include "IConv.h"
#include "UnicodeDefines.h"
#include "TextOnScreenMngr.h"
#include "DecoderResolutionTable.h"
#include "Gathering.h"

#include "SysConfigKeys.h"
#include "ConfigHelper.h"

#include "VideoApiDefinitionsStrings.h"
#include "H264Util.h"
#include "SiteNameInfo.h"
#include "MrcStructs.h"
#include "VideoRelaySourcesParams.h"

#include "SVCToAVCTranslator.h"
#include "AvcToSvcTranslator.h"

#include "LayoutSharedMemoryMap.h"
#include "LibsCommonHelperFuncs.h"

#include "ConfPartyProcess.h"

#include "Trace.h"


extern CDecoderResolutionTable* GetpDecoderResolutionTable();

#define COLOR_GRAY_YUV 0xFF3F857C            // Gray color (YUV Format)


typedef struct _CellBoarders
{
	BOOL ucRight;   // boolean
	BOOL ucLeft;    // boolean
	BOOL ucUp;      // boolean
	BOOL ucDown;    // boolean
} CellBoarders;

typedef struct _LayoutCellBorders
{
	ELayoutType  layout_type;
	CellBoarders cell_borders[16];
}LayoutCellBorders;

LayoutCellBorders gInitCellBoder[E_VIDEO_LAYOUT_LAST - 1] =
{
	{E_VIDEO_LAYOUT_DUMMY, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_1X1, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_1X2, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_2X1, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_2X2, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_3X3, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_4X4, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_1P5, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_1P7, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_1X2HOR, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_1X2VER, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_1P2VER, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_1P2HOR, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_1P3HOR_UP, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_1P3VER, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_1P4HOR, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_1P4VER, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_1P8CENT, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_1P8UP, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_1P8HOR_UP, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_1P2HOR_UP, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_1P3HOR, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_1P4HOR_UP, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_1P12, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_2P8, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_1X1_QCIF, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_FLEX_1X2, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_FLEX_1P2HOR_RIGHT, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_FLEX_1P2HOR_LEFT, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_FLEX_1P2HOR_UP_RIGHT, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_FLEX_1P2HOR_UP_LEFT, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_FLEX_2X2_UP_RIGHT, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_FLEX_2X2_UP_LEFT, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_FLEX_2X2_DOWN_RIGHT, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_FLEX_2X2_DOWN_LEFT, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_FLEX_2X2_RIGHT, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_FLEX_2X2_LEFT, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_OVERLAY_1P1, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_OVERLAY_1P2, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_OVERLAY_1P3, {{1, 1, 1, 1}}},

	{E_VIDEO_LAYOUT_OVERLAY_ITP_1P2, {{1, 1, 1, 1}, {0, 1, 1, 1}, {1, 0, 1, 1}}},
	{E_VIDEO_LAYOUT_OVERLAY_ITP_1P3, {{1, 1, 1, 1}, {0, 1, 1, 1}, {0, 0, 1, 1}, {1, 0, 1, 1}}},
	{E_VIDEO_LAYOUT_OVERLAY_ITP_1P4, {{1, 1, 1, 1}, {0, 1, 1, 1}, {0, 0, 1, 1}, {0, 0, 1, 1}, {1, 0, 1, 1}}},
	{E_VIDEO_LAYOUT_1TOP_LEFT_P8, {{1, 1, 1, 1}}},
	{E_VIDEO_LAYOUT_2TOP_P8, {{1, 1, 1, 1}}},
};



////////////////////////////////////////////////////////////////////////////
//                        CVideoHardwareInterface
////////////////////////////////////////////////////////////////////////////
CVideoHardwareInterface::CVideoHardwareInterface(ConnectionID ConnectionId, PartyRsrcID ParId, ConfRsrcID ConfId, eLogicalResourceTypes LRT)
{
	m_pRsrcParams                      = new CRsrcParams(ConnectionId, ParId, ConfId, LRT);
	m_pOriginalIVRShowSlidePlayMessage = NULL;
	AllocateIconv();
	m_nVideoFastUpdateReq			    = 0;
	m_nIvrFastUpdateReq					= 0;
}

////////////////////////////////////////////////////////////////////////////
CVideoHardwareInterface::~CVideoHardwareInterface()
{
	POBJDELETE(m_pRsrcParams);

	if (m_pOriginalIVRShowSlidePlayMessage)
		PDELETE(m_pOriginalIVRShowSlidePlayMessage);

	DeallocatIconv();
}

////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::TrafficShapingEncoderParamsUpdate(ENCODER_PARAM_S& enc) const
{
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	DBGPASSERT_AND_RETURN(!pSysConfig);

	BOOL isTrafficShapingEnabled = NO;
	std::string key = CFG_KEY_ENABLE_RTP_TRAFFIC_SHAPING;
	if(pSysConfig->GetBOOLDataByKey(key, isTrafficShapingEnabled) && isTrafficShapingEnabled)
	{
		//==========================================================================
		// MTP adjustment, keep this section before the bitrate reduction section,
		// in order to use original bitrate for the adjustment
		//==========================================================================
		DWORD MTUFactor = 0;
		DWORD MTUMin = 0;
		std::string MTUFactorKey	= CFG_KEY_TRAFFIC_SHAPING_MTU_FACTOR;
		std::string MTUMinKey 		= CFG_KEY_TRAFFIC_SHAPING_MTU_MIN;
		if (pSysConfig->GetDWORDDataByKey(MTUMinKey, MTUMin) && pSysConfig->GetDWORDDataByKey(MTUFactorKey, MTUFactor) && MTUFactor > 0)
		{
			DWORD MTUFactored = enc.nBitRate / MTUFactor;
			MTUFactored = max(MTUMin, min(MTUFactored, static_cast<DWORD>(enc.nMTUSize)));
			TRACEINTO << " according to configuration of MTU factor["<< MTUFactor <<"] MTU is reduced from [" << enc.nMTUSize << "] to [" << MTUFactored << "]";
			enc.nMTUSize = MTUFactored;
		}

		//====================
		// Bitrate reduction
		//====================
		DWORD bitrateReduction = 0;
		std::string key = CFG_KEY_VIDEO_BIT_RATE_REDUCTION_PERCENT;
		if (pSysConfig->GetDWORDDataByKey(key, bitrateReduction) && bitrateReduction < 100 && bitrateReduction > 0)
		{
			// Keeping calculation in integers for performance
			//==================================================
			DWORD bitrateFactored 			= enc.nBitRate * 100;
			DWORD bitratePercentFactored	= bitrateFactored / 100; // Basically it's enc.nBitRate, but please leave this calculation for clarity
			bitrateFactored -= bitratePercentFactored * bitrateReduction;
			bitrateFactored /= 100;
			TRACEINTO << " according to configuration bitrate is reduced from [" << enc.nBitRate << "] to [" << bitrateFactored << "]";
			enc.nBitRate = bitrateFactored;
		}
	}
}

////////////////////////////////////////////////////////////////////////////
DWORD CVideoHardwareInterface::SendOpenEncoder(
	DWORD videoAlg,
	DWORD videoBitRate,
	eVideoFrameRate videoQCifFrameRate,
	eVideoFrameRate videoCifFrameRate,
	eVideoFrameRate video4CifFrameRate,
	DWORD decoderDetectedModeWidth,
	DWORD decoderDetectedModeHeight,
	DWORD decoderDetectedSampleAspectRatioWidth,
	DWORD decoderDetectedSampleAspectRatioHeight,
	eVideoResolution videoResolution,
	DWORD mbps,
	DWORD fs,
	DWORD sampleAspectRatio,
	DWORD staticMB,
	eVideoQuality videoQuality,
	BYTE isVideoClarityEnabled,
	eVideoConfType videoConfType,
	DWORD dpb,
	BOOL isLinkEncoder,
	DWORD parsingMode,
	eTelePresencePartyType eTelePresenceMode,
	eVideoFrameRate resolutionFrameRate,
	EVideoResolutionTableType eVideoResolutionTableType /* = E_VIDEO_RESOLUTION_TABLE_REGULAR*/,
	DWORD horizontalCroppingPercentage /* = INVALID*/,
	DWORD verticalCroppingPercentage /* = INVALID*/,
	eVideoProfile profile /* = eVideoProfileBaseline*/,
	eVideoPacketPayloadFormat packetPayloadFormat /* = eVideoPacketPayloadFormatSingleUnit*/,
	BYTE bIsTipMode /* = false*/,
	BYTE bUseIntermediateSDResolution /* = false*/,
	BOOL isEncodeBFrames /* = false*/,
	DWORD dwFrThreshold,
	FontTypesEnum fontType /* = ftDefault*/,
	BOOL bIs263Plus /* = FALSE*/,
	BOOL bIsCallGenerator,
	BYTE bFollowSpeakerOn1X1,
	DWORD ssrc /* = 0*/,
	DWORD height /* = 0*/,
	DWORD width /* = 0*/,
	DWORD prId)
{
	ENCODER_PARAM_S params;
	FillEncoderParams(params, FALSE,
	                  videoAlg,
	                  videoBitRate,
	                  videoQCifFrameRate,
	                  videoCifFrameRate,
	                  video4CifFrameRate,
	                  decoderDetectedModeWidth,
	                  decoderDetectedModeHeight,
	                  decoderDetectedSampleAspectRatioWidth,
	                  decoderDetectedSampleAspectRatioHeight,
	                  videoResolution,
	                  mbps,
	                  fs,
	                  sampleAspectRatio,
	                  staticMB,
	                  eTelePresenceMode,
	                  videoQuality,
	                  isVideoClarityEnabled,
	                  videoConfType,
	                  parsingMode,
	                  dpb,
	                  isLinkEncoder,
	                  resolutionFrameRate,
	                  eVideoResolutionTableType,
	                  horizontalCroppingPercentage,
	                  verticalCroppingPercentage,
	                  profile,
	                  packetPayloadFormat,
	                  bIsTipMode,
	                  bUseIntermediateSDResolution,
	                  isEncodeBFrames,
	                  dwFrThreshold,
	                  fontType,
	                  bIs263Plus,
	                  bIsCallGenerator,
	                  bFollowSpeakerOn1X1);

	CDecoderResolutionTable* pDecoderResolutionTable = ::GetpDecoderResolutionTable();
	if (pDecoderResolutionTable)
		pDecoderResolutionTable->AddEncoderToTable(GetConnectionId());

	if (videoAlg == MS_SVC)
	{
		if (IsValidMsftSVCParams(width, height))
			fillEncoderMsftSVCParams(params,ssrc, prId, videoConfType, width, height, resolutionFrameRate, videoAlg, videoBitRate,videoResolution, mbps, dpb, profile, packetPayloadFormat);
		else
			TRACEINTO << "MS_SVC parameters not initialized, ignored.";
	}

	if (videoAlg == VP8)
	{
		ResetParamsForVP8Encoder(params);	// reset H263 params, reset most H264 params
	}

	CSegment msg;
	msg.Put((BYTE*)(&params), sizeof(params));
	return SendMsgToMPL(TB_MSG_OPEN_PORT_REQ, &msg);
}

///////////////////////////////////////////////////////////////////////////
DWORD CVideoHardwareInterface::SendUpdateEncoder(ENCODER_PARAM_S& params, bool open/* = false*/)
{
	TRACEINTO << " Debug for VP8, need to be removed!";

	if (open)
	{
		CDecoderResolutionTable* pTable = ::GetpDecoderResolutionTable();

		if (pTable)
			pTable->AddEncoderToTable(GetConnectionId());
	}
	if (params.nProtocol == VP8)
	{
		ResetParamsForVP8Encoder(params);	// reset H263 params, reset most H264 params
	}


	CSegment msg;
	msg.Put((BYTE*)(&params), sizeof(params));
	return SendMsgToMPL(open ? TB_MSG_OPEN_PORT_REQ : VIDEO_ENCODER_UPDATE_PARAM_REQ, &msg);
}

////////////////////////////////////////////////////////////////////////////
//#if 0
DWORD CVideoHardwareInterface::SendUpdateEncoder(
	BOOL isLPR,
	DWORD videoAlg,
	DWORD videoBitRate,
	eVideoFrameRate videoQCifFrameRate,
	eVideoFrameRate videoCifFrameRate,
	eVideoFrameRate video4CifFrameRate,
	DWORD decoderDetectedModeWidth,
	DWORD decoderDetectedModeHeight,
	DWORD decoderDetectedSampleAspectRatioWidth,
	DWORD decoderDetectedSampleAspectRatioHeight,
	eVideoResolution videoResolution,
	DWORD mbps,
	DWORD fs,
	DWORD sampleAspectRatio,
	DWORD staticMB,
	eTelePresencePartyType eTelePresenceMode,
	eVideoQuality videoQuality,
	BYTE isVideoClarityEnabled,
	eVideoConfType videoConfType,
	DWORD dpb,
	BOOL isLinkEncoder,
	eVideoFrameRate resolutionFrameRate,
	EVideoResolutionTableType eVideoResolutionTableType,
	DWORD horizontalCroppingPercentage,
	DWORD verticalCroppingPercentage,
	eVideoProfile profile,
	eVideoPacketPayloadFormat packetPayloadFormat,
	BYTE bIsTipMode,
	BYTE bUseIntermediateSDResolution,
	BOOL bEncodeBFrames,
	DWORD dwFrThreshold,
	FontTypesEnum fontType /* = ftDefault*/,
	BOOL bIs263Plus /* = FALSE */,
	BOOL bIsCallGenerator,
	BYTE bFollowSpeakerOn1X1,
	DWORD ssrc /* = 0*/,
	DWORD height /* = 0*/,
	DWORD width /* = 0*/,
	DWORD prId)
{
	ENCODER_PARAM_S tEncoderStruct;
	FillEncoderParams(tEncoderStruct, isLPR,
	                  videoAlg,
	                  videoBitRate,
	                  videoQCifFrameRate,
	                  videoCifFrameRate,
	                  video4CifFrameRate,
	                  decoderDetectedModeWidth,
	                  decoderDetectedModeHeight,
	                  decoderDetectedSampleAspectRatioWidth,
	                  decoderDetectedSampleAspectRatioHeight,
	                  videoResolution,
	                  mbps,
	                  fs,
	                  sampleAspectRatio,
	                  staticMB,
	                  eTelePresenceMode,
	                  videoQuality,
	                  isVideoClarityEnabled,
	                  videoConfType,
	                  E_PARSING_MODE_CP,                  // update not valin in vsw
	                  dpb,
	                  isLinkEncoder,
	                  resolutionFrameRate,
	                  eVideoResolutionTableType,
	                  horizontalCroppingPercentage,
	                  verticalCroppingPercentage,
	                  profile,
	                  packetPayloadFormat,
	                  bIsTipMode,
	                  bUseIntermediateSDResolution,
	                  bEncodeBFrames,
	                  dwFrThreshold,
	                  fontType,
                      bIs263Plus,
                      bIsCallGenerator,
                      bFollowSpeakerOn1X1);

	if (videoAlg == MS_SVC)
	{
		if (IsValidMsftSVCParams(width, height))
			fillEncoderMsftSVCParams(tEncoderStruct,ssrc, prId, videoConfType, width, height, resolutionFrameRate, videoAlg, videoBitRate,videoResolution, mbps, dpb, profile, packetPayloadFormat);
		else
			TRACEINTO << "MS_SVC parameters not initialized, ignored.";
	}

	if (videoAlg == VP8)
	{
		ResetParamsForVP8Encoder(tEncoderStruct);	// reset H263 params, reset most H264 params
	}

	CSegment msg;
	msg.Put((BYTE*)(&tEncoderStruct), sizeof(ENCODER_PARAM_S));
	return SendMsgToMPL(VIDEO_ENCODER_UPDATE_PARAM_REQ, &msg);
}
//#endif
////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::FillEncoderParams(
	ENCODER_PARAM_S& tEncoderStruct,
	BOOL isLPR,
	DWORD videoAlg,
	DWORD videoBitRate,
	eVideoFrameRate videoQCifFrameRate,
	eVideoFrameRate videoCifFrameRate,
	eVideoFrameRate video4CifFrameRate,
	DWORD decoderDetectedModeWidth,
	DWORD decoderDetectedModeHeight,
	DWORD decoderDetectedSampleAspectRatioWidth,
	DWORD decoderDetectedSampleAspectRatioHeight,
	eVideoResolution videoResolution,
	DWORD mbps,
	DWORD fs,
	DWORD sampleAspectRatio,
	DWORD staticMB,
	eTelePresencePartyType eTelePresenceMode,
	eVideoQuality videoQuality,
	BYTE isVideoClarityEnabled,
	eVideoConfType videoConfType,
	DWORD parsingMode,
	DWORD dpb,
	BOOL isLinkEncoder,
	eVideoFrameRate resolutionFrameRate,
	EVideoResolutionTableType eVideoResolutionTableType,
	DWORD horizontalCroppingPercentage,
	DWORD verticalCroppingPercentage,
	eVideoProfile profile,
	eVideoPacketPayloadFormat packetPayloadFormat,
	BYTE bIsTipMode,
	BYTE bUseIntermediateSDResolution,
	BOOL bEncodeBFrames,
	DWORD dwFrThreshold,
	FontTypesEnum fontType /* = ftDefault*/,
	BOOL bIs263Plus,
	BOOL bIsCallGenerator,
	BYTE bFollowSpeakerOn1X1)
{
	memset(&tEncoderStruct, 0, sizeof(ENCODER_PARAM_S));

	tEncoderStruct.nVideoConfType                         = TranslateVideoConfTypeToApi(videoConfType);
	tEncoderStruct.nVideoEncoderType                      = E_VIDEO_ENCODER_DUMMY;
	tEncoderStruct.nBitRate                               = videoBitRate;
	tEncoderStruct.nProtocol                              = TranslateVideoProtocolToApi(videoAlg);
	tEncoderStruct.tH264VideoParams.nMBPS                 = mbps;
	tEncoderStruct.tH264VideoParams.nFS                   = fs;
	tEncoderStruct.tH264VideoParams.nStaticMB             = staticMB;
	tEncoderStruct.tH264VideoParams.nProfile              = TranslateVideoProfileToApi(profile);
	tEncoderStruct.tH264VideoParams.unPacketPayloadFormat = TranslatePacketPayloadFormatToApi(packetPayloadFormat);
	tEncoderStruct.bIsLinkEncoder            = isLinkEncoder;

	// COP
	tEncoderStruct.tH264VideoParams.nResolutionWidth     = TranslateResolutionToResWidth(videoResolution);                            // For COP feature
	tEncoderStruct.tH264VideoParams.nResolutionHeight    = TranslateResolutionToResHeight(videoResolution);                           // For COP feature
	tEncoderStruct.tH264VideoParams.nResolutionFrameRate = TranslateVideoFrameRateToApi(resolutionFrameRate);                         // For COP feature
	tEncoderStruct.tH263_H261VideoParams.nQcifFrameRate  = TranslateVideoFrameRateToApi(videoQCifFrameRate);
	tEncoderStruct.tH263_H261VideoParams.nCifFrameRate   = TranslateVideoFrameRateToApi(videoCifFrameRate);
	tEncoderStruct.tH263_H261VideoParams.n4CifFrameRate  = TranslateVideoFrameRateToApi(video4CifFrameRate);
	tEncoderStruct.tH263_H261VideoParams.b263HighBbIntra = IsH263HighBbIntra();                                                       // Default should be NO !! H263_HIGH_BIT_BUDGET_INTRA when set to YES (1) the H263 intra that sent will be bigger.(the intra will be SOFTER) China request for soft intra
	tEncoderStruct.tH263_H261VideoParams.bIs263Plus      = bIs263Plus;  // Default should be NO !!

	tEncoderStruct.nSampleAspectRatio                                           = sampleAspectRatio;
	tEncoderStruct.tDecoderDetectedMode.nDecoderDetectedModeWidth               = decoderDetectedModeWidth;
	tEncoderStruct.tDecoderDetectedMode.nDecoderDetectedModeHeight              = decoderDetectedModeHeight;
	tEncoderStruct.tDecoderDetectedMode.nDecoderDetectedSampleAspectRatioWidth  = decoderDetectedSampleAspectRatioWidth;
	tEncoderStruct.tDecoderDetectedMode.nDecoderDetectedSampleAspectRatioHeight = decoderDetectedSampleAspectRatioHeight;
	tEncoderStruct.nResolutionTableType                                         = eVideoResolutionTableType;
	tEncoderStruct.nParsingMode                                                 = parsingMode;
	tEncoderStruct.nTelePresenceMode                                            = TranslateTelePresenceModeToApi(eTelePresenceMode);
	tEncoderStruct.nMTUSize                                                     = GetMTUSize(isLPR, bIsTipMode);
	tEncoderStruct.nVideoQualityType                                            = TranslateVideoQualityToApi(videoQuality, videoConfType);
	tEncoderStruct.bIsVideoClarityEnabled                                       = isVideoClarityEnabled;
	tEncoderStruct.tCroppingParams.nHorizontalCroppingPercentage                = horizontalCroppingPercentage;
	tEncoderStruct.tCroppingParams.nVerticalCroppingPercentage                  = verticalCroppingPercentage;
	tEncoderStruct.bIsTipMode                                                   = bIsTipMode;
	tEncoderStruct.bUseIntermediateSDResolution                                 = IsIntermediateSDResolution(bUseIntermediateSDResolution);
	tEncoderStruct.bRtvEnableBFrames                                            = bEncodeBFrames;
	tEncoderStruct.nFontType                                                    = IsFeatureSupportedBySystem(eFeatureFontTypes) ? fontType : ftDefault;
	tEncoderStruct.nFrThreshold                                                 = dwFrThreshold;

	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();

	string sFpsMode;
	pSysConfig->GetDataByKey("PAL_NTSC_VIDEO_OUTPUT", sFpsMode);
	tEncoderStruct.nFpsMode = TranslateSysConfigStringToFpsMode(sFpsMode);

	if (videoAlg != MS_SVC)
	{
		UpdateMaxDpbFromMaxFs(dpb, fs);
		tEncoderStruct.tH264VideoParams.unMaxDPB = dpb;
		BYTE isVsw = !(parsingMode == E_PARSING_MODE_CP);
		tEncoderStruct.nEncoderResolutionRatio = TranslateToVideoResolutionRatio(videoAlg, videoResolution, fs, mbps, videoConfType, isVsw);
	}

	BOOL isH264MbIntraRefresh = NO;
	pSysConfig->GetBOOLDataByKey(H264_MB_INTRA_REFRESH, isH264MbIntraRefresh);

	tEncoderStruct.bEnableMbRefresh = isH264MbIntraRefresh;
	tEncoderStruct.nMaxSingleTransferFrameSize = GetSystemCfgFlag<DWORD>(CFG_KEY_MAX_SINGLE_TRANSFER_FRAME_SIZE_BITS);

	// Fill parameters for call generator SoftMCU
	tEncoderStruct.tCallGeneratorParams.bIsCallGenerator = bIsCallGenerator;
	tEncoderStruct.tCallGeneratorParams.reserved[0] = 0;
	tEncoderStruct.tCallGeneratorParams.reserved[1] = 0;
	tEncoderStruct.tCallGeneratorParams.reserved[2] = 0;

	tEncoderStruct.bFollowSpeaker            = bFollowSpeakerOn1X1;

	//====================================================================
	// Traffic shaping - only bitrate reduction is handled in the bridge
	//====================================================================
	if (!bIsTipMode)
	{
		TrafficShapingEncoderParamsUpdate(tEncoderStruct);
	}
}

////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::fillEncoderMsftSVCParams(
	ENCODER_PARAM_S& params,
	DWORD ssrcID, // Ssrc for T0 – Encoder will use Ssrc+1 and +2 for T1,T2
	DWORD prID,   // PrId for T0 – Encoder will use PrId+1 and +2 for T1,T2
	eVideoConfType videoConfType,
	DWORD nResolutionWidth,
	DWORD nResolutionHeight,
	eVideoFrameRate frameRate,
	DWORD videoAlg,
	DWORD maxBitrate,
	eVideoResolution videoResolution,
	DWORD mbps,
	DWORD dpb,
	eVideoProfile profile, // 1 \u2013 baseline, 2 \u2013 High
	eVideoPacketPayloadFormat packetPayloadFormat)
{
	const DWORD svcFs = GetFsForSvcLync(nResolutionWidth, nResolutionHeight, true);
	const DWORD svcMbps = (DWORD)GetMaxMbpsAsDevision((DWORD)(svcFs * TranslateVideoFrameRateToNumeric(frameRate)));
	const DWORD svcMaxDpb = CH264Details::GetMaxDpbFromMaxFs(svcFs);

	params.nEncoderResolutionRatio   = TranslateToVideoResolutionRatio(videoAlg, videoResolution, params.tH264VideoParams.nFS, svcMbps, videoConfType, false);

	params.tH264VideoParams.unMaxDPB = (svcMaxDpb > dpb || dpb == (DWORD)(-1)) ? svcMaxDpb : dpb;
	params.tH264VideoParams.nFS      = GetMaxFsAsDevision(svcFs);
	params.tH264VideoParams.nMBPS    = svcMbps;

	params.tH264SvcVideoParams.nProfile              = TranslateVideoProfileToApi(profile);
	params.tH264SvcVideoParams.unPacketPayloadFormat = TranslatePacketPayloadFormatToApi(packetPayloadFormat);
	params.tH264SvcVideoParams.unPrID                = prID;
	params.tH264SvcVideoParams.unSsrcID              = ssrcID;
	params.tH264SvcVideoParams.nResolutionHeight     = nResolutionHeight;
	params.tH264SvcVideoParams.nResolutionWidth      = nResolutionWidth;

	// taken fom the svctoavctranslator hardcoded value
	params.tCroppingParams.nHorizontalCroppingPercentage = 50;
	params.tCroppingParams.nVerticalCroppingPercentage   = 50;

	fillEncoderMsftSVCTemporaryLayersParams(params,frameRate,maxBitrate);



	TRACEINTO
		<< "\n fs = "<< svcFs
		<< "\n maxDPB = " << dpb
		<< "\n videoBitRate = " << maxBitrate
		<< "\n FrameRate = " << TranslateVideoFrameRateToNumeric(frameRate)
		<< "\n FrameWidth = " << nResolutionWidth
		<< "\n FrameHeight = " << nResolutionHeight;
}
////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::fillEncoderMsftSVCTemporaryLayersParams(ENCODER_PARAM_S& encoder,eVideoFrameRate frameRate, DWORD maxBitrate)
{
	//////////////////
	///Romem
	//////////////////
	DWORD numOfLayers = 1;
	DWORD T0_BITRATE_PERCENT, T1_BITRATE_PERCENT, T2_BITRATE_PERCENT;

	// setting temporal layer frame rate and bitrate.
	// bitrates are claculated as max bit rate * 1000 (tranlsate to bps) * bitrate % for layer / 100

	//PATCH according to System_flag set layers to 3 or 2

	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	DWORD dwNumOfLayers;
	pSysConfig->GetDWORDDataByKey("LYNC2013_PATCH_NUM_OF_LAYERS", dwNumOfLayers);

	if (3 == dwNumOfLayers)
	{
		switch (frameRate)
		{
		case eVideoFrameRate30FPS:
			numOfLayers = 3;
			T0_BITRATE_PERCENT = 50;
			T1_BITRATE_PERCENT = 25;
			T2_BITRATE_PERCENT = 25;
			encoder.tH264SvcVideoParams.atSvcTemporalLayer[0].nResolutionFrameRate = CVideoHardwareInterface::TranslateVideoFrameRateToApi(eVideoFrameRate7_5FPS);
			encoder.tH264SvcVideoParams.atSvcTemporalLayer[1].nResolutionFrameRate = CVideoHardwareInterface::TranslateVideoFrameRateToApi(eVideoFrameRate15FPS);
			encoder.tH264SvcVideoParams.atSvcTemporalLayer[2].nResolutionFrameRate = CVideoHardwareInterface::TranslateVideoFrameRateToApi(eVideoFrameRate30FPS);
			encoder.tH264SvcVideoParams.atSvcTemporalLayer[0].nBitRate = T0_BITRATE_PERCENT * maxBitrate /100;
			encoder.tH264SvcVideoParams.atSvcTemporalLayer[1].nBitRate = (T1_BITRATE_PERCENT + T0_BITRATE_PERCENT) * maxBitrate /100;
			encoder.tH264SvcVideoParams.atSvcTemporalLayer[2].nBitRate = /*(T1_BITRATE_PERCENT + T0_BITRATE_PERCENT + T2_BITRATE_PERCENT) /100 * */  maxBitrate;
			break;

		case eVideoFrameRate15FPS:
			numOfLayers = 2;
			encoder.tH264SvcVideoParams.atSvcTemporalLayer[0].nResolutionFrameRate = CVideoHardwareInterface::TranslateVideoFrameRateToApi(eVideoFrameRate7_5FPS);
			encoder.tH264SvcVideoParams.atSvcTemporalLayer[1].nResolutionFrameRate = CVideoHardwareInterface::TranslateVideoFrameRateToApi(eVideoFrameRate15FPS);
			T0_BITRATE_PERCENT = 55;
			T1_BITRATE_PERCENT = 45;
			encoder.tH264SvcVideoParams.atSvcTemporalLayer[0].nBitRate = T0_BITRATE_PERCENT * maxBitrate /100;
			encoder.tH264SvcVideoParams.atSvcTemporalLayer[1].nBitRate = /*(T1_BITRATE_PERCENT + T0_BITRATE_PERCENT) /100 * */ maxBitrate;
			break;

		case eVideoFrameRate7_5FPS:
			numOfLayers = 1;
			encoder.tH264SvcVideoParams.atSvcTemporalLayer[0].nResolutionFrameRate = CVideoHardwareInterface::TranslateVideoFrameRateToApi(eVideoFrameRate7_5FPS);
			encoder.tH264SvcVideoParams.atSvcTemporalLayer[0].nBitRate = maxBitrate;
			break;

		default:
			PASSERTSTREAM(true, "received unsupported frame rate (eVideoFrameRate): " << (int)frameRate);
		}
	}
	else if (2 == dwNumOfLayers)
	{
		switch (frameRate)
		{
		case eVideoFrameRate30FPS:
			numOfLayers = 2;
			T0_BITRATE_PERCENT = 55;
			T1_BITRATE_PERCENT = 45;
			encoder.tH264SvcVideoParams.atSvcTemporalLayer[0].nResolutionFrameRate = CVideoHardwareInterface::TranslateVideoFrameRateToApi(eVideoFrameRate15FPS);
			encoder.tH264SvcVideoParams.atSvcTemporalLayer[1].nResolutionFrameRate = CVideoHardwareInterface::TranslateVideoFrameRateToApi(eVideoFrameRate30FPS);
			encoder.tH264SvcVideoParams.atSvcTemporalLayer[0].nBitRate = T0_BITRATE_PERCENT *  maxBitrate /100;
			encoder.tH264SvcVideoParams.atSvcTemporalLayer[1].nBitRate = maxBitrate;//(T1_BITRATE_PERCENT + T0_BITRATE_PERCENT) * maxBitrate /100;
			break;

		case eVideoFrameRate15FPS:
			numOfLayers = 1;

			encoder.tH264SvcVideoParams.atSvcTemporalLayer[0].nResolutionFrameRate = CVideoHardwareInterface::TranslateVideoFrameRateToApi(eVideoFrameRate15FPS);
			encoder.tH264SvcVideoParams.atSvcTemporalLayer[0].nBitRate = maxBitrate;
			break;

		default:
			PASSERTSTREAM(true, "received unsupported frame rate (eVideoFrameRate): " << (int)maxBitrate);
		}
	}
	else
		TRACEINTO << "LYNC2013_PATCH_NUM_OF_LAYERS is NOT 2 or 3";

	encoder.tH264SvcVideoParams.unNumberOfTemporalLayers = numOfLayers;

	TRACEINTO
	        << "\n System flag LYNC2013_PATCH_NUM_OF_LAYERS is set to "<< dwNumOfLayers
			<< "\n chosen number of layers = "<< numOfLayers;

	//====================================================================
	// Traffic shaping - only bitrate reduction is handled in the bridge
	//====================================================================
	TrafficShapingEncoderParamsUpdate(encoder);
}
////////////////////////////////////////////////////////////////////////////
DWORD CVideoHardwareInterface::SendOpenEncoderAvcToSvc(CAvcToSvcOpenEncoder* openEncoder)
{
	// fill encoder struct params
	ENCODER_PARAM_S tEncoderStruct;
	memset(&tEncoderStruct, 0, sizeof(ENCODER_PARAM_S));

	FillOpenEncoderAvcToSvcParams(openEncoder, &tEncoderStruct);

	// send Msg to MPL
	CSegment* pMsg = new CSegment;
	pMsg->Put((BYTE*)(&tEncoderStruct), sizeof(ENCODER_PARAM_S));
	DWORD     reqId = SendMsgToMPL(TB_MSG_OPEN_PORT_REQ, pMsg);
	POBJDELETE(pMsg);
	return reqId;
}

////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::FillOpenEncoderAvcToSvcParams(CAvcToSvcOpenEncoder* openEncoder, ENCODER_PARAM_S* encoderStruct)
{
	OPEN_TRANSLATOR_ENCODER* pOE        = &openEncoder->oe;
	CSysConfig*              pSysConfig = CProcessBase::GetProcess()->GetSysConfig();

	encoderStruct->nVideoConfType    = TranslateVideoConfTypeToApi(pOE->videoConfType);
	encoderStruct->nVideoEncoderType = E_VIDEO_ENCODER_NORMAL;
	encoderStruct->nBitRate          = pOE->videoBitRate;
	encoderStruct->nProtocol         = TranslateVideoProtocolToApi(pOE->videoAlg);

	// H263/261 no relevant xxx
	// encoderStruct->tH263_H261VideoParams

	// H264_VIDEO_PARAM_S
/*	encoderStruct->tH264VideoParams.nMBPS = xxx;
  encoderStruct->tH264VideoParams.nFS = xxx;
  encoderStruct->tH264VideoParams.nStaticMB = xxx;
  encoderStruct->tH264VideoParams.nResolutionWidth = xxx;
  encoderStruct->tH264VideoParams.nResolutionHeight = xxx;
  encoderStruct->tH264VideoParams.nResolutionFrameRate = xxx;
  encoderStruct->tH264VideoParams.nProfile = xxx;
  encoderStruct->tH264VideoParams.unPacketPayloadFormat = xxx;
  encoderStruct->tH264VideoParams.unMaxDPB = xxx;
*/
	encoderStruct->nSampleAspectRatio = pOE->sampleAspectRatio;

	// DECODER_DETECTED_MODE_PARAM_S
	encoderStruct->tDecoderDetectedMode.nDecoderDetectedModeWidth               = 0;
	encoderStruct->tDecoderDetectedMode.nDecoderDetectedModeHeight              = 0;
	encoderStruct->tDecoderDetectedMode.nDecoderDetectedSampleAspectRatioWidth  = 0;
	encoderStruct->tDecoderDetectedMode.nDecoderDetectedSampleAspectRatioHeight = 0;

	encoderStruct->nResolutionTableType   = pOE->videoResolutionTableType;
	encoderStruct->nParsingMode           = pOE->parsingMode;
	encoderStruct->nMTUSize               = GetMTUSize(false, false);
	encoderStruct->nTelePresenceMode      = TranslateTelePresenceModeToApi(eTelePresencePartyNone);
	encoderStruct->nVideoQualityType      = E_VIDEO_QUALITY_DUMMY;
	encoderStruct->bIsVideoClarityEnabled = NO;

	string sFpsMode;
	pSysConfig->GetDataByKey("PAL_NTSC_VIDEO_OUTPUT", sFpsMode);
	encoderStruct->nFpsMode = TranslateSysConfigStringToFpsMode(sFpsMode);

	// CROPPING_PARAM_S
	//encoderStruct->tCroppingParams.nHorizontalCroppingPercentage = -1;
	//encoderStruct->tCroppingParams.nVerticalCroppingPercentage   = -1;
	encoderStruct->tCroppingParams.nHorizontalCroppingPercentage = pOE->tCroppingParams.nHorizontalCroppingPercentage;
	encoderStruct->tCroppingParams.nVerticalCroppingPercentage   = pOE->tCroppingParams.nVerticalCroppingPercentage;
	encoderStruct->bEnableMbRefresh = GetSystemCfgFlag<bool>(H264_MB_INTRA_REFRESH);

	DWORD fs = GetFsForAvcToSvc(pOE->tH264SvcVideoParams.nResolutionWidth, pOE->tH264SvcVideoParams.nResolutionHeight);

	encoderStruct->nEncoderResolutionRatio = TranslateToVideoResolutionRatio(pOE->videoAlg, eVideoResolutionHD720, fs, 0, pOE->videoConfType, 0);
	encoderStruct->nMaxSingleTransferFrameSize = GetSystemCfgFlag<DWORD>(CFG_KEY_MAX_SINGLE_TRANSFER_FRAME_SIZE_BITS);
	encoderStruct->bIsTipMode                   = pOE->bIsTipMode;
	encoderStruct->bIsLinkEncoder               = FALSE;
	encoderStruct->bUseIntermediateSDResolution = pOE->bUseIntermediateSDResolution;

	//====================================================================
	// Traffic shaping - only bitrate reduction is handled in the bridge
	//====================================================================
	if (!pOE->bIsTipMode)
	{
		TrafficShapingEncoderParamsUpdate(*encoderStruct);
	}

	// H264_SVC_VIDEO_PARAM_S
	memcpy(&encoderStruct->tH264SvcVideoParams, &pOE->tH264SvcVideoParams, sizeof(H264_SVC_VIDEO_PARAM_S));

	eVideoProfile vidProfile = (eVideoProfile)pOE->tH264SvcVideoParams.nProfile;
	encoderStruct->tH264SvcVideoParams.nProfile = TranslateVideoProfileToApi(vidProfile);
	for (DWORD i = 0; i < encoderStruct->tH264SvcVideoParams.unNumberOfTemporalLayers; i++)
	{
		eVideoFrameRate resFrameRate = (eVideoFrameRate)pOE->tH264SvcVideoParams.atSvcTemporalLayer[i].nResolutionFrameRate;
		encoderStruct->tH264SvcVideoParams.atSvcTemporalLayer[i].nResolutionFrameRate = TranslateVideoFrameRateToApi(resFrameRate);
	}

	// Fill parameters for call generator SoftMCU
	encoderStruct->tCallGeneratorParams.bIsCallGenerator = pOE->bIsCallGenerator;
	encoderStruct->tCallGeneratorParams.reserved[0] = 0;
	encoderStruct->tCallGeneratorParams.reserved[1] = 0;
	encoderStruct->tCallGeneratorParams.reserved[2] = 0;
}

////////////////////////////////////////////////////////////////////////////
DWORD CVideoHardwareInterface::SendOpenDecoder(DWORD videoAlg,
                                               DWORD videoBitRate,
                                               eVideoResolution videoResolution,
                                               eVideoFrameRate videoQCifFrameRate,
                                               eVideoFrameRate videoCifFrameRate,
                                               eVideoFrameRate video4CifFrameRate,
                                               DWORD mbps,
                                               DWORD fs,
                                               DWORD sampleAspectRatio,
                                               DWORD staticMB,
                                               BYTE isVSW,
                                               DWORD backgroundImageID,
                                               BYTE isVideoClarityEnabled,
                                               eVideoConfType videoConfType,
                                               DWORD dpb,
                                               DWORD parsingMode,
                                               eTelePresencePartyType eTelePresenceMode,
                                               BOOL isAutoBrightness,
                                               eVideoFrameRate videoVGAFrameRate,
                                               eVideoFrameRate videoSVGAFrameRate,
                                               eVideoFrameRate videoXGAFrameRate,
                                               EVideoDecoderType decoderType,
                                               eVideoFrameRate resolutionFrameRate,
                                               eVideoProfile profile,
                                               eVideoPacketPayloadFormat packetPayloadFormat,
                                               BYTE bIsTipMode,
                                               BOOL bIs263Plus,BOOL bIsCallGenerator,
                                               DWORD ssrc,
                                               DWORD height,
                                               DWORD width)
{
	DECODER_PARAM_S tDecoderStruct;
	FillDecoderParams(tDecoderStruct, videoAlg,
	                  videoBitRate,
	                  videoResolution,
	                  videoQCifFrameRate,
	                  videoCifFrameRate,
	                  video4CifFrameRate,
	                  mbps,
	                  fs,
	                  sampleAspectRatio,
	                  staticMB,
	                  backgroundImageID,
	                  isVideoClarityEnabled,
	                  videoConfType,
	                  parsingMode,
	                  dpb,
	                  eTelePresenceMode,
	                  isAutoBrightness,
	                  videoVGAFrameRate,
	                  videoSVGAFrameRate,
	                  videoXGAFrameRate,
	                  decoderType,
	                  profile,
	                  packetPayloadFormat,
	                  bIsTipMode,
                      bIs263Plus,
                      bIsCallGenerator);

	DWORD decoderFrameRateLimit = INVALID;
	DWORD sd30MaxMBPS           = 81;	// The division operation is heavy,(H264_L3_DEFAULT_MBPS/CUSTOM_MAX_MBPS_FACTOR)
	// PCI bug patch (to be removed in V3.x)
	if ((mbps >= sd30MaxMBPS) && (videoAlg == H264) && parsingMode == E_PARSING_MODE_CP)
	{
		if (eTelePresenceMode != eTelePresencePartyNone)
			decoderFrameRateLimit = 20;
		else
		{
			CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
			sysConfig->GetDWORDDataByKey("MAX_TRANSMITED_FRAME_RATE_FROM_DECODER", decoderFrameRateLimit);
		}

		CDecoderResolutionTable* pDecoderResolutionTable = ::GetpDecoderResolutionTable();
		if (pDecoderResolutionTable)
			pDecoderResolutionTable->AddDecoderToTable(GetRsrcParams());
	}

	tDecoderStruct.nDecoderFrameRateLimit = (videoAlg == RTV) ? 30 : decoderFrameRateLimit;

	tDecoderStruct.nDecoderResolutionRatio = TranslateToVideoResolutionRatio(videoAlg, videoResolution, fs, mbps, videoConfType, isVSW);
	if (E_VIDEO_DECODER_CONTENT == decoderType)
	{
		// VNGR-20319 - We should consider HD1080 content decoder as RESOLUTION_RATIO_64
		if (videoResolution == eVideoResolutionHD1080)
			tDecoderStruct.nDecoderResolutionRatio = RESOLUTION_RATIO_64;
		else
			tDecoderStruct.nDecoderResolutionRatio = RESOLUTION_RATIO_16;
	}
	if (videoAlg == MS_SVC)
	{
		if (IsValidMsftSVCParams(width, height))
			fillDecoderMsftSVCParams(tDecoderStruct, videoConfType, videoAlg, width, height, videoBitRate, resolutionFrameRate, videoResolution,dpb);
		else
			TRACEINTO << "MS_SVC parameters not initialized, ignored.";
	}
	if (videoAlg == VP8)
	{
		ResetParamsForVP8Decoder(tDecoderStruct);	// reset H263 params, reset most H264 params
	}

	CSegment* pMsg = new CSegment;
	pMsg->Put((BYTE*)(&tDecoderStruct), sizeof(DECODER_PARAM_S));
	DWORD reqId = SendMsgToMPL(TB_MSG_OPEN_PORT_REQ, pMsg);
	POBJDELETE(pMsg);
	return reqId;
}

////////////////////////////////////////////////////////////////////////////
DWORD CVideoHardwareInterface::SendUpdateDecoder(DWORD videoAlg,
                                                 DWORD videoBitRate,
                                                 eVideoResolution videoResolution,
                                                 eVideoFrameRate videoQCifFrameRate,
                                                 eVideoFrameRate videoCifFrameRate,
                                                 eVideoFrameRate video4CifFrameRate,
                                                 DWORD mbps,
                                                 DWORD fs,
                                                 DWORD sampleAspectRatio,
                                                 DWORD staticMB,
                                                 DWORD backgroundImageID,
                                                 BYTE isVideoClarityEnabled,
                                                 eVideoConfType videoConfType,
                                                 DWORD dpb,
                                                 eTelePresencePartyType eTelePresenceMode,
                                                 BOOL isAutoBrightness,
                                                 eVideoFrameRate videoVGAFrameRate,
                                                 eVideoFrameRate videoSVGAFrameRate,
                                                 eVideoFrameRate videoXGAFrameRate,
                                                 EVideoDecoderType decoderType,
                                                 eVideoFrameRate resolutionFrameRate,
                                                 eVideoProfile profile,
                                                 eVideoPacketPayloadFormat packetPayloadFormat,
                                                 BYTE bIsTipMode,
                                                 BOOL bIs263Plus,BOOL bIsCallGenerator,
                                                 DWORD ssrc,
                                                 DWORD height,
                                                 DWORD width)
{
	DECODER_PARAM_S tDecoderStruct;
	FillDecoderParams(tDecoderStruct, videoAlg,
	                  videoBitRate,
	                  videoResolution,
	                  videoQCifFrameRate,
	                  videoCifFrameRate,
	                  video4CifFrameRate,
	                  mbps,
	                  fs,
	                  sampleAspectRatio,
	                  staticMB,
	                  backgroundImageID,
	                  isVideoClarityEnabled,
	                  videoConfType,
	                  E_PARSING_MODE_CP,
	                  dpb,
	                  eTelePresenceMode,
	                  isAutoBrightness,
	                  videoVGAFrameRate,
	                  videoSVGAFrameRate,
	                  videoXGAFrameRate,
	                  decoderType,
	                  profile,
	                  packetPayloadFormat,
	                  bIsTipMode,
                      bIs263Plus,
                      bIsCallGenerator);
	if (videoAlg == MS_SVC)
	{
		if (IsValidMsftSVCParams(width, height))
			fillDecoderMsftSVCParams(tDecoderStruct, videoConfType, videoAlg, width, height, videoBitRate, resolutionFrameRate, videoResolution,dpb);
		else
			TRACEINTO << "MS_SVC parameters not initialized, ignored.";
	}


	if (videoAlg == VP8)
	{
		ResetParamsForVP8Decoder(tDecoderStruct);	// reset H263 params, reset most H264 params
	}

	CSegment* pMsg = new CSegment;
	pMsg->Put((BYTE*)(&tDecoderStruct), sizeof(DECODER_PARAM_S));
	DWORD reqId = SendMsgToMPL(VIDEO_DECODER_UPDATE_PARAM_REQ, pMsg);
	POBJDELETE(pMsg);
	return reqId;
}

void CVideoHardwareInterface::ResetParamsForVP8Decoder(DECODER_PARAM_S& tDecoderStruct)
{
	TRACEINTO << " Reset all H263 parameters and part of H264 parameters";	// amir 7-5

	// Reset H263 params
	memset(&(tDecoderStruct.tH263_H261VideoParams), 0, sizeof(H263_H261_VIDEO_PARAM_S));

	// Reset most H264 Params
	APIS32 nMBPS_temp = tDecoderStruct.tH264VideoParams.nMBPS;
	APIS32 nFS_temp = tDecoderStruct.tH264VideoParams.nFS;
	memset(&(tDecoderStruct.tH264VideoParams), 0, sizeof(H264_VIDEO_PARAM_S));
	tDecoderStruct.tH264VideoParams.nMBPS = nMBPS_temp;
	tDecoderStruct.tH264VideoParams.nFS = nFS_temp;
}

////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::ResetParamsForVP8Encoder(ENCODER_PARAM_S& tEncoderStruct)
{
	TRACEINTO << " Reset all H263 parameters and part of H264 parameters";	// amir 7-5

	// Reset H263 params
	memset(&(tEncoderStruct.tH263_H261VideoParams), 0, sizeof(H263_H261_VIDEO_PARAM_S));

	// Reset most H264 Params
	APIS32 nMBPS_temp = tEncoderStruct.tH264VideoParams.nMBPS;
	APIS32 nFS_temp = tEncoderStruct.tH264VideoParams.nFS;
	memset(&(tEncoderStruct.tH264VideoParams), 0, sizeof(H264_VIDEO_PARAM_S));
	tEncoderStruct.tH264VideoParams.nMBPS = nMBPS_temp;
	tEncoderStruct.tH264VideoParams.nFS = nFS_temp;
}


////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::FillDecoderParams(DECODER_PARAM_S& tDecoderStruct,
                                                DWORD videoAlg,
                                                DWORD videoBitRate,
                                                eVideoResolution videoResolution,
                                                eVideoFrameRate videoQCifFrameRate,
                                                eVideoFrameRate videoCifFrameRate,
                                                eVideoFrameRate video4CifFrameRate,
                                                DWORD mbps,
                                                DWORD fs,
                                                DWORD sampleAspectRatio,
                                                DWORD staticMB,
                                                DWORD backgroundImageID,
                                                BYTE isVideoClarityEnabled,
                                                eVideoConfType videoConfType,
                                                DWORD parsingMode,
                                                DWORD dpb,
                                                eTelePresencePartyType eTelePresenceMode,
                                                BOOL isAutoBrightness,
                                                eVideoFrameRate videoVGAFrameRate,
                                                eVideoFrameRate videoSVGAFrameRate,
                                                eVideoFrameRate videoXGAFrameRate,
                                                EVideoDecoderType decoderType,
                                                eVideoProfile profile,
                                                eVideoPacketPayloadFormat packetPayloadFormat,
                                                BYTE bIsTipMode,
                                                BOOL bIs263Plus,
                                                BOOL bIsCallGenerator)
{
	memset(&tDecoderStruct, 0, sizeof(DECODER_PARAM_S));

	tDecoderStruct.nVideoConfType             = TranslateVideoConfTypeToApi(videoConfType);
	tDecoderStruct.nVideoDecoderType          = decoderType;                      // E_VIDEO_DECODER_NORMAL,E_VIDEO_DECODER_CONTENT
	tDecoderStruct.nProtocol                  = TranslateVideoProtocolToApi(videoAlg);
	tDecoderStruct.nBitRate                   = videoBitRate;
	tDecoderStruct.tH264VideoParams.nMBPS     = mbps;
	tDecoderStruct.tH264VideoParams.nFS       = fs;
	tDecoderStruct.tH264VideoParams.nStaticMB = staticMB;
	UpdateMaxDpbFromMaxFs(dpb, fs);
	tDecoderStruct.tH264VideoParams.unMaxDPB = dpb;

	// tmp for the COP integration
	tDecoderStruct.tH264VideoParams.nResolutionWidth      = 0;                    // For COP feature
	tDecoderStruct.tH264VideoParams.nResolutionHeight     = 0;                    // For COP feature
	tDecoderStruct.tH264VideoParams.nResolutionFrameRate  = E_VIDEO_FPS_DUMMY;    // For COP feature relevant for the encoder only
	tDecoderStruct.tH264VideoParams.nProfile              = TranslateVideoProfileToApi(profile);
	tDecoderStruct.tH264VideoParams.unPacketPayloadFormat = TranslatePacketPayloadFormatToApi(packetPayloadFormat);
	tDecoderStruct.tH263_H261VideoParams.nQcifFrameRate   = TranslateVideoFrameRateToApi(videoQCifFrameRate);
	tDecoderStruct.tH263_H261VideoParams.nCifFrameRate    = TranslateVideoFrameRateToApi(videoCifFrameRate);
	tDecoderStruct.tH263_H261VideoParams.n4CifFrameRate   = TranslateVideoFrameRateToApi(video4CifFrameRate);
	tDecoderStruct.tH263_H261VideoParams.nVGAFrameRate    = TranslateVideoFrameRateToApi(videoVGAFrameRate);
	tDecoderStruct.tH263_H261VideoParams.nSVGAFrameRate   = TranslateVideoFrameRateToApi(videoSVGAFrameRate);
	tDecoderStruct.tH263_H261VideoParams.nXGAFrameRate    = TranslateVideoFrameRateToApi(videoXGAFrameRate);
	tDecoderStruct.tH263_H261VideoParams.b263HighBbIntra  = IsH263HighBbIntra();                                                                     // Default should be NO !! H263_HIGH_BIT_BUDGET_INTRA when set to YES (1) the H263 intra that sent will be bigger.(the intra will be SOFTER) China request for soft intra
	tDecoderStruct.tH263_H261VideoParams.bIs263Plus       = bIs263Plus;  // Default should be NO !!
	tDecoderStruct.nSampleAspectRatio                     = sampleAspectRatio;
	tDecoderStruct.bInterSync                             = TRUE;                                                                                    // Carmel Version 1 - DBC2 not supported therefor always true
	tDecoderStruct.nParsingMode                           = parsingMode;                                                                             // update not valid in vsw
	tDecoderStruct.bIsTipMode                             = bIsTipMode;
	tDecoderStruct.nBackgroundImageId                     = backgroundImageID;
	tDecoderStruct.bIsVideoClarityEnabled                 = isVideoClarityEnabled;
	tDecoderStruct.nDecoderResolutionRatio                = TranslateToVideoResolutionRatio(videoAlg, videoResolution, fs, mbps, videoConfType, NO); // update not valid in vsw

	// VNGR-14321 take AutoBrightness flag from System.cfg only for NORMAL decoder not for content decoder
	BOOL bSetAutoBrightness = (E_VIDEO_DECODER_CONTENT == decoderType) ? FALSE : isAutoBrightness;
	tDecoderStruct.bIsAutoBrightnessEnabled = bSetAutoBrightness;

	DWORD decoderFrameRateLimit = INVALID;
	DWORD sd30MaxMBPS           = 81;                                             // The division operation is heavy,(H264_L3_DEFAULT_MBPS/CUSTOM_MAX_MBPS_FACTOR)
	// PCI bug patch (to be removed in V3.x)
	if ((MBPS >= sd30MaxMBPS) && (videoAlg == H264) && parsingMode == E_PARSING_MODE_CP)
	{
		if (eTelePresenceMode != eTelePresencePartyNone)
			decoderFrameRateLimit = 20;
		else
		{
			CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
			sysConfig->GetDWORDDataByKey("MAX_TRANSMITED_FRAME_RATE_FROM_DECODER", decoderFrameRateLimit);
		}
	}

	tDecoderStruct.nDecoderFrameRateLimit = decoderFrameRateLimit;

	// Fill parameters for call generator SoftMCU
	tDecoderStruct.tCallGeneratorParams.bIsCallGenerator = bIsCallGenerator;
	tDecoderStruct.tCallGeneratorParams.reserved[0] = 0;
	tDecoderStruct.tCallGeneratorParams.reserved[1] = 0;
	tDecoderStruct.tCallGeneratorParams.reserved[2] = 0;
}

void CVideoHardwareInterface::fillDecoderMsftSVCParams(DECODER_PARAM_S& tDecoderStruct, eVideoConfType videoConfType, DWORD videoAlg,
		DWORD nResolutionWidth, DWORD nResolutionHeight,DWORD maxBitrate, eVideoFrameRate frameRate, eVideoResolution videoResolution, DWORD dpb)
{
	DWORD svcBitrate = /*1000 * */maxBitrate;
	DWORD svcFs = GetFsForSvcLync(nResolutionWidth, nResolutionHeight, YES);
	DWORD svcMaxDpb = CH264Details::GetMaxDpbFromMaxFs(svcFs);
	DWORD svcMbps = (DWORD)GetMaxMbpsAsDevision((DWORD)(svcFs * TranslateVideoFrameRateToNumeric(frameRate)));

	tDecoderStruct.tH264VideoParams.unMaxDPB = ((svcMaxDpb > dpb) || (dpb == (DWORD)-1))? svcMaxDpb : dpb;
	tDecoderStruct.tH264VideoParams.nFS       = GetMaxFsAsDevision(svcFs);
	tDecoderStruct.nDecoderResolutionRatio = TranslateToVideoResolutionRatio(videoAlg, videoResolution, tDecoderStruct.tH264VideoParams.nFS, svcMbps, videoConfType, FALSE);
	tDecoderStruct.tH264VideoParams.nMBPS  = svcMbps;
	TRACEINTO << " \n fs = "<< svcFs << "\n mbps = " << svcMbps
			  << " \n videoBitRate = " << svcBitrate
			  << " \n FrameRate = " << TranslateVideoFrameRateToNumeric(frameRate)
			  << " \n FrameWidth = " << nResolutionWidth
			  << " \n FrameHeight = " << nResolutionHeight;

}
////////////////////////////////////////////////////////////////////////////
DWORD CVideoHardwareInterface::SendCloseEncoder()
{
	CDecoderResolutionTable* pDecoderResolutionTable = ::GetpDecoderResolutionTable();
	if (pDecoderResolutionTable)
		pDecoderResolutionTable->RemoveEncoderFromTable(GetConnectionId());

	return SendMsgToMPL(TB_MSG_CLOSE_PORT_REQ, NULL);
}

////////////////////////////////////////////////////////////////////////////
DWORD CVideoHardwareInterface::SendCloseDecoder()
{
	CDecoderResolutionTable* pDecoderResolutionTable = ::GetpDecoderResolutionTable();
	if (pDecoderResolutionTable)
		pDecoderResolutionTable->RemoveDecoderFromTable(GetConnectionId());

	return SendMsgToMPL(TB_MSG_CLOSE_PORT_REQ, NULL);
}

////////////////////////////////////////////////////////////////////////////
DWORD CVideoHardwareInterface::SendConnect(ConnectionID ConnectionID1, ConnectionID ConnectionID2, DWORD PartyRcrsID1, DWORD PartyRcrsID2)
{
	TB_MSG_CONNECT_S tConnecttStruct;
	memset(&tConnecttStruct, 0, sizeof(TB_MSG_CONNECT_S));

	tConnecttStruct.physical_port1.connection_id = ConnectionID1;
	tConnecttStruct.physical_port1.party_id      = PartyRcrsID1;
	tConnecttStruct.physical_port2.connection_id = ConnectionID2;
	tConnecttStruct.physical_port2.party_id      = PartyRcrsID2;

	CSegment* pMsg = new CSegment;
	pMsg->Put((BYTE*)(&tConnecttStruct), sizeof(TB_MSG_CONNECT_S));

	DWORD reqId = SendMsgToMPL(TB_MSG_CONNECT_REQ, pMsg);
	POBJDELETE(pMsg);
	return reqId;
}

////////////////////////////////////////////////////////////////////////////
DWORD CVideoHardwareInterface::SendDisconnect(ConnectionID ConnectionID1, ConnectionID ConnectionID2, DWORD PartyRcrsID1, DWORD PartyRcrsID2)
{
	TB_MSG_CONNECT_S tConnecttStruct;
	memset(&tConnecttStruct, 0, sizeof(TB_MSG_CONNECT_S));

	tConnecttStruct.physical_port1.connection_id = ConnectionID1;
	tConnecttStruct.physical_port1.party_id      = PartyRcrsID1;
	tConnecttStruct.physical_port2.connection_id = ConnectionID2;
	tConnecttStruct.physical_port2.party_id      = PartyRcrsID2;

	CSegment* pMsg = new CSegment;
	pMsg->Put((BYTE*)(&tConnecttStruct), sizeof(TB_MSG_CONNECT_S));

	DWORD reqId = SendMsgToMPL(TB_MSG_DISCONNECT_REQ, pMsg);
	POBJDELETE(pMsg);
	return reqId;
}

////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::SendFastUpdate()
{
	m_nVideoFastUpdateReq++;
	TRACEINTO << "Counter: id:" << GetPartyRsrcId() << ", num of video fast update req:" << m_nVideoFastUpdateReq << ", num of ivr fast update req:" << m_nIvrFastUpdateReq;
	SendMsgToMPL(VIDEO_FAST_UPDATE_REQ, NULL);
}

////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::SendIvrFastUpdate()
{
	m_nIvrFastUpdateReq++;
	SendMsgToMPL(IVR_FAST_UPDATE_REQ, NULL);
}

////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::SendChangeIndications(const CLayout* pLayout)
{
	const ICONS_DISPLAY_S& tIconsDisplay = pLayout->indications();
	BYTE* pBuffer = const_cast<BYTE*>(reinterpret_cast<const BYTE*>(&tIconsDisplay));

	CSegment msg;
	msg.Put(pBuffer, sizeof(ICONS_DISPLAY_S));
	SendMsgToMPL(VIDEO_ENCODER_ICONS_DISPLAY_REQ, &msg);
}

////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::UpdateIndicationIconSharedMemory(const CLayout* pLayout)
{
	const ICONS_DISPLAY_S& tIconsDisplay = pLayout->indications();

	
	// Fill Indication Icon entry
	CIndicationIconEntry pIndicationIconEntry;
	pIndicationIconEntry.SetIndicationIconId(GetPartyRsrcId());	//currently we use party resource ID for Indication Icon ID
	pIndicationIconEntry.SetIsChanged(TRUE);
	pIndicationIconEntry.SetConfRsrcId(GetConfRsrcId());
	pIndicationIconEntry.SetPartyRsrcId(GetPartyRsrcId());
	pIndicationIconEntry.SetConnectionId(GetConnectionId());
	pIndicationIconEntry.SetIndicationIconParams(tIconsDisplay);

	// Update Indication Icon Shared Memory
	CIndicationIconSharedMemoryMap* pIndicationIconSharedMemoryMap = ((CConfPartyProcess*) CProcessBase::GetProcess())->GetIndicationIconSharedMemory();;
	if (NULL == pIndicationIconSharedMemoryMap)
	{
		PASSERT_AND_RETURN(102);
	}
	pIndicationIconSharedMemoryMap->AddOrUpdate(pIndicationIconEntry);
}


////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::SendChangeIndicationsOrUpdate(const CLayout* pLayout,BOOL bUseSharedMemForIndicationIcon)
{
	if(bUseSharedMemForIndicationIcon)
		UpdateIndicationIconSharedMemory(pLayout);
	else
		SendChangeIndications(pLayout);
}



////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::ChangeLayoutSendOrUpdate(const CLayout* pLayout, CVisualEffectsParams* pVisualEffects, CSiteNameInfo* pSiteNameInfo,
													   DWORD speakerPlaceInLayout, eVideoResolution videoEncoderResolution, DWORD decoderDetectedModeWidth,
													   DWORD decoderDetectedModeHeight, DWORD decoderDetectedSampleAspectRatioWidth,
													   DWORD decoderDetectedSampleAspectRatioHeight, DWORD videoAlg, DWORD fs, DWORD mbps,
													   eVideoConfType videoConfType, BYTE isSiteNamesEnabled, BYTE isTelePresenceMode, BOOL bUseSharedMemForChangeLayoutReq, BYTE isVSW)
{
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pLayout));

	bool newVisualEffects = false;
	if (!CPObject::IsValidPObjectPtr(pVisualEffects))
	{
		TRACEINTO << "New CVisualEffectsParams object was created for VSW";
		pVisualEffects = new CVisualEffectsParams();
		newVisualEffects = true;
	}



	MCMS_CM_CHANGE_LAYOUT_S tChangeLayoutStruct;
	memset(&tChangeLayoutStruct, 0xff, sizeof(MCMS_CM_CHANGE_LAYOUT_S));

	ELayoutType layoutType = TranslateVideoLayoutTypeToApi(pLayout->GetLayoutType());
	tChangeLayoutStruct.nLayoutType                                                  = (BYTE)layoutType;
	tChangeLayoutStruct.nEncoderResolutionRatio                                      = (BYTE)TranslateToVideoResolutionRatio(videoAlg, videoEncoderResolution, fs, mbps, videoConfType, isVSW);
	tChangeLayoutStruct.nDecoderDetectedMode.nDecoderDetectedModeWidth               = (DWORD)decoderDetectedModeWidth;
	tChangeLayoutStruct.nDecoderDetectedMode.nDecoderDetectedModeHeight              = (DWORD)decoderDetectedModeHeight;
	tChangeLayoutStruct.nDecoderDetectedMode.nDecoderDetectedSampleAspectRatioWidth  = (DWORD)decoderDetectedSampleAspectRatioWidth;
	tChangeLayoutStruct.nDecoderDetectedMode.nDecoderDetectedSampleAspectRatioHeight = (DWORD)decoderDetectedSampleAspectRatioHeight;

	tChangeLayoutStruct.tIconsDisplay = pLayout->indications();

	ECascadePartyType cascadeModeOfFirstImage = eCascadeNone;

	CPartyImageLookupTable* pPartyImageLookupTable = GetPartyImageLookupTable();
	WORD numOfImages = pLayout->GetNumberOfSubImages();
	tChangeLayoutStruct.atImageParam = new MCMS_CM_IMAGE_PARAM_S[numOfImages];
	memset((BYTE*)(tChangeLayoutStruct.atImageParam), 0xff, numOfImages*sizeof(MCMS_CM_IMAGE_PARAM_S));

	int percentageThressoldGeneral = -1;
	int percentageThressoldPanoramic = -1;
	GetCroppingPercentageThressold(percentageThressoldGeneral, percentageThressoldPanoramic, isTelePresenceMode);

	for (int i = 0; i < numOfImages; i++)
	{
		DWORD decoderConnectionId                         = INVALID;
		DWORD decoderPartyId                              = INVALID;
		DWORD artPartyId                                  = INVALID;
		BYTE imageResolutionRatio                         = RESOLUTION_RATIO_0;
		DWORD imageDecoderDetectedModeWidth               = DEFAULT_DECODER_DETECTED_MODE_WIDTH;
		DWORD imageDecoderDetectedModeHeight              = DEFAULT_DECODER_DETECTED_MODE_HEIGHT;
		DWORD imageDecoderDetectedSampleAspectRatioWidth  = DEFAULT_DECODER_DETECTED_SAMPLE_ASPECT_RATIO_WIDTH;
		DWORD imageDecoderDetectedSampleAspectRatioHeight = DEFAULT_DECODER_DETECTED_SAMPLE_ASPECT_RATIO_HEIGHT;
		ERelativeSizeOfImageInLayout sizeOfImageInLayout  = E_NOT_IN_LAYOUT;
		BOOL bContentDecoder = FALSE;

		// site name send to card decoded in UCS-2BE, 16 bit encoding, null terminating = 00
		tChangeLayoutStruct.atImageParam[i].siteName[0] = '\0';
		tChangeLayoutStruct.atImageParam[i].siteName[1] = '\0';


		const CVidSubImage* pVidSubImage = (*pLayout)[i];
		if (pVidSubImage)
		{
			DWORD partyRscId = pVidSubImage->GetImageId();
			if (partyRscId)
			{
				const CImage* pImage = pPartyImageLookupTable->GetPartyImage(partyRscId);
				PASSERTSTREAM(!pImage, "CVideoHardwareInterface::ChangeLayoutSendOrUpdate - Failed, The lookup table doesn't have an element, PartyId:" << partyRscId);

				if (pImage)
				{
					if (i == 0)
						cascadeModeOfFirstImage = pImage->GetCascadeMode();

					decoderConnectionId                         = pImage->GetConnectionId();
					decoderPartyId                              = pImage->GetPartyRsrcId();
					artPartyId                                  = pImage->GetArtPartyId();  // In CP same as decoderPartyId in cop the party that connected to the cop decoder decoderPartyId

					sizeOfImageInLayout                         = TranslateImageSizeToScale(pVidSubImage->GetSizeX(), pVidSubImage->GetSizeY());
					imageDecoderDetectedModeWidth               = pImage->GetDecoderDetectedModeWidth();
					imageDecoderDetectedModeHeight              = pImage->GetDecoderDetectedModeHeight();
					DWORD fs = pImage->GetFS();
					DWORD mbps = pImage->GetMBPS();
					imageResolutionRatio                        =  (BYTE)TranslateToVideoResolutionRatio(pImage->GetVideoAlgo(), pImage->GetVideoResolution(), fs, mbps, videoConfType, isVSW);
					imageDecoderDetectedSampleAspectRatioWidth  = pImage->GetDecoderDetectedSampleAspectRatioWidth();
					imageDecoderDetectedSampleAspectRatioHeight = pImage->GetDecoderDetectedSampleAspectRatioHeight();
					BOOL bShow = (pSiteNameInfo && pSiteNameInfo->GetDisplayMode() != eSiteNameOff);

					//< bug BRIDGE-1215
					if(i != 0 && (IsOverlayLayout(layoutType) || IsITPOverlayLayout(layoutType))){
						CSmallString str;
						str << "PartyId: " << partyRscId << ".";
						PTRACE2(eLevelInfoNormal, "CVideoHardwareInterface::ChangeLayoutSendOrUpdate - Overlay cells should not have site names, ", str.GetString());

						bShow = FALSE;
					}
					//>

					if ((0 == i) && (!strncmp((char*)pImage->GetSiteName(), "##I_AM_THE_CONTENT_DECODER", strlen("##I_AM_THE_CONTENT_DECODER"))))
					{
						bContentDecoder = TRUE;
					}

					// Patch related to the fact that the site name is used to identify the content decoder in the ENCODER (VNGR-18774)
					// and site name does NOT send in Telepresence
					// Content decoder is always on cell 0
					if (bShow || bContentDecoder)
					{
						TranslateSiteNameToUCS2(pImage->GetSiteName(), tChangeLayoutStruct.atImageParam[i].siteName, MAX_SITE_NAME_SIZE);
					}
				}
			}
		}
		else
			TRACEINTO << "No vidSubImage object found for image " << i << ", using default values.";


		tChangeLayoutStruct.atImageParam[i].tDecoderPhysicalId.connection_id                             = decoderConnectionId;
		tChangeLayoutStruct.atImageParam[i].tDecoderPhysicalId.party_id                                  = decoderPartyId;
		tChangeLayoutStruct.atImageParam[i].nArtPartyId                                                  = artPartyId;
		tChangeLayoutStruct.atImageParam[i].nDecoderSizeInLayout                                         = (BYTE)sizeOfImageInLayout;
		tChangeLayoutStruct.atImageParam[i].nDecoderResolutionRatio                                      = imageResolutionRatio;
		tChangeLayoutStruct.atImageParam[i].nDecoderDetectedMode.nDecoderDetectedModeWidth               = imageDecoderDetectedModeWidth;
		tChangeLayoutStruct.atImageParam[i].nDecoderDetectedMode.nDecoderDetectedModeHeight              = imageDecoderDetectedModeHeight;
		tChangeLayoutStruct.atImageParam[i].nDecoderDetectedMode.nDecoderDetectedSampleAspectRatioWidth  = imageDecoderDetectedSampleAspectRatioWidth;
		tChangeLayoutStruct.atImageParam[i].nDecoderDetectedMode.nDecoderDetectedSampleAspectRatioHeight = imageDecoderDetectedSampleAspectRatioHeight;

		if (bContentDecoder)
		{
			// Bridge-15649
			tChangeLayoutStruct.atImageParam[i].nThressoldCroppingOnImage                                = 0; // 0 = do not crop
		}
		else
		{
			tChangeLayoutStruct.atImageParam[i].nThressoldCroppingOnImage                                = FillTherssoldCroppingOnImage(percentageThressoldGeneral, percentageThressoldPanoramic, i, layoutType);
		}
	}// end for


	bool ableCroppingOnImage = IsAbleCroppingOnImage(layoutType, cascadeModeOfFirstImage, videoConfType);
	if (ableCroppingOnImage == false)
	{
		tChangeLayoutStruct.atImageParam[0].nThressoldCroppingOnImage = 0; // 0 = do not crop
	}

	FillChangeLayoutAttributes(pLayout,tChangeLayoutStruct.tChangeLayoutAttributes, pVisualEffects, pSiteNameInfo, speakerPlaceInLayout, videoConfType);

	//Removed in V7.8 since MPM is not supported - pDecoderResolutionTable is allocated only in eSystemCardsMode_mpm (otherwise null).
	/*  CDecoderResolutionTable* pDecoderResolutionTable = ::GetpDecoderResolutionTable();
  if (pDecoderResolutionTable)
    pDecoderResolutionTable->UpdateTable(tChangeLayoutStruct, GetConnectionId());
	 */

	//Change Layout Improvement - Layout Shared Memory (CL-SM)
	if (!bUseSharedMemForChangeLayoutReq)
		SendChangeLayout(tChangeLayoutStruct);
	else
		UpdateLayoutSharedMemory(tChangeLayoutStruct);

	PDELETEA(tChangeLayoutStruct.atImageParam);
	if(newVisualEffects){
		PDELETE(pVisualEffects);
	}
}

////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::SendChangeLayout(MCMS_CM_CHANGE_LAYOUT_S& tChangeLayoutStruct)
{
	CSegment* pMsg = new CSegment;
   	WORD numOfImages = CLibsCommonHelperFuncs::GetNumbSubImg(tChangeLayoutStruct.nLayoutType);

	pMsg->Put((BYTE*)(&tChangeLayoutStruct), sizeof(MCMS_CM_CHANGE_LAYOUT_S) - sizeof(MCMS_CM_IMAGE_PARAM_S*));
	pMsg->Put((BYTE*)(&tChangeLayoutStruct.atImageParam[0]), numOfImages*(sizeof(MCMS_CM_IMAGE_PARAM_S)));

	SendMsgToMPL(VIDEO_ENCODER_CHANGE_LAYOUT_REQ, pMsg);

	POBJDELETE(pMsg);
}

////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::UpdateLayoutSharedMemory(MCMS_CM_CHANGE_LAYOUT_S& tChangeLayoutStruct)
{
	// Fill layout entry
	CLayoutEntry pLayoutEntry;
	pLayoutEntry.SetLayoutId(GetPartyRsrcId());	//currently we use party resource ID for layout ID
	pLayoutEntry.SetIsChanged(TRUE);
	pLayoutEntry.SetConfRsrcId(GetConfRsrcId());
	pLayoutEntry.SetPartyRsrcId(GetPartyRsrcId());
	pLayoutEntry.SetConnectionId(GetConnectionId());
	pLayoutEntry.SetChangeLayoutParams(tChangeLayoutStruct);

	// Update Layout Shared Memory
	CLayoutSharedMemoryMap* pLayoutSharedMemoryMap = ((CConfPartyProcess*) CProcessBase::GetProcess())->GetLayoutSharedMemory();;
	if (NULL == pLayoutSharedMemoryMap)
	{
		PASSERT_AND_RETURN(101);
	}
	pLayoutSharedMemoryMap->AddOrUpdate(pLayoutEntry);
}

////////////////////////////////////////////////////////////////////////////
int CVideoHardwareInterface::FillTherssoldCroppingOnImage(int percentageThressoldGeneral, int percentageThressoldPanoramic, int cellIndex, ELayoutType layoutType)
{
	int percentageThressold = -1; //crop always

	if (percentageThressoldGeneral == percentageThressoldPanoramic) // no meaning to panoramic or general because it is the same
		percentageThressold = percentageThressoldGeneral;

	else
	{
		bool isPanoramicCell = IsPanoramicCell(layoutType, cellIndex);
		if (isPanoramicCell == true)
			percentageThressold = percentageThressoldPanoramic;
		else // regular cell
			percentageThressold = percentageThressoldGeneral;
	}

	TRACEINTO << "cropping percentage Thressold: " << percentageThressold;
	return percentageThressold;
}

////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::GetCroppingPercentageThressold(int& percentageThressoldGeneral, int& percentageThressoldPanoramic, BYTE isTelePresenceMode)
{

	if (isTelePresenceMode == 1)
	{
		percentageThressoldGeneral = -1; //crop always
		percentageThressoldPanoramic = -1; //crop always
	}

	else
	{
		CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
		sysConfig->GetIntDataByKey("CROPPING_PERCENTAGE_THRESHOLD_GENERAL", percentageThressoldGeneral);
		sysConfig->GetIntDataByKey("CROPPING_PERCENTAGE_THRESHOLD_PANORAMIC", percentageThressoldPanoramic);
	}

}

////////////////////////////////////////////////////////////////////////////
bool CVideoHardwareInterface::IsAbleCroppingOnImage(ELayoutType layoutType, ECascadePartyType cascadeMode, eVideoConfType videoConfType)
{
	if (layoutType == E_VIDEO_LAYOUT_1X1 && cascadeMode != eCascadeNone && (videoConfType == eVideoConfTypeCopHD108025fps || videoConfType == eVideoConfTypeCopHD72050fps))
	{
		TRACEINTO << "disable cropping on image";
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////
bool CVideoHardwareInterface::IsPanoramicCell(ELayoutType layoutType, int cellIndex)
{

	bool isPanoramic = false;

	switch (layoutType)
	{

	  //non panoramic
	  case E_VIDEO_LAYOUT_1X1:
	  case E_VIDEO_LAYOUT_1X2:
	  case E_VIDEO_LAYOUT_2X1:
	  case E_VIDEO_LAYOUT_2X2:
	  case E_VIDEO_LAYOUT_3X3:
	  case E_VIDEO_LAYOUT_4X4:
	  case E_VIDEO_LAYOUT_1P5:
	  case E_VIDEO_LAYOUT_1P7:
	  case E_VIDEO_LAYOUT_1P12:
	  case E_VIDEO_LAYOUT_2P8:
	  case E_VIDEO_LAYOUT_1X1_QCIF:
	  case E_VIDEO_LAYOUT_OVERLAY_1P1:
	  case E_VIDEO_LAYOUT_OVERLAY_1P2:
	  case E_VIDEO_LAYOUT_OVERLAY_1P3:
	  case E_VIDEO_LAYOUT_OVERLAY_ITP_1P2:
	  case E_VIDEO_LAYOUT_OVERLAY_ITP_1P3:
	  case E_VIDEO_LAYOUT_OVERLAY_ITP_1P4:
	  case E_VIDEO_LAYOUT_FLEX_1X2:
	  case E_VIDEO_LAYOUT_FLEX_2X2_UP_RIGHT:
	  case E_VIDEO_LAYOUT_FLEX_2X2_UP_LEFT:
	  case E_VIDEO_LAYOUT_FLEX_2X2_DOWN_RIGHT:
	  case E_VIDEO_LAYOUT_FLEX_2X2_DOWN_LEFT:
	  case E_VIDEO_LAYOUT_FLEX_2X2_RIGHT:
	  case E_VIDEO_LAYOUT_FLEX_2X2_LEFT:
	  case E_VIDEO_LAYOUT_1TOP_LEFT_P8:
	  case E_VIDEO_LAYOUT_2TOP_P8:
		  break;

	  // 1 cell is panoramic
	  case E_VIDEO_LAYOUT_1P2VER:
	  case E_VIDEO_LAYOUT_1P2HOR:
	  case E_VIDEO_LAYOUT_1P3HOR_UP:
	  case E_VIDEO_LAYOUT_1P3VER:
	  case E_VIDEO_LAYOUT_1P4HOR:
	  case E_VIDEO_LAYOUT_1P4VER:
	  case E_VIDEO_LAYOUT_1P8CENT:
	  case E_VIDEO_LAYOUT_1P8UP:
	  case E_VIDEO_LAYOUT_1P8HOR_UP:
	  case E_VIDEO_LAYOUT_1P2HOR_UP:
	  case E_VIDEO_LAYOUT_1P3HOR:
	  case E_VIDEO_LAYOUT_1P4HOR_UP:
	  case E_VIDEO_LAYOUT_FLEX_1P2HOR_RIGHT:
	  case E_VIDEO_LAYOUT_FLEX_1P2HOR_LEFT:
	  case E_VIDEO_LAYOUT_FLEX_1P2HOR_UP_RIGHT:
	  case E_VIDEO_LAYOUT_FLEX_1P2HOR_UP_LEFT:

	  {
		  if (cellIndex == 0) //The panoramic cell is the first cell
			  isPanoramic = true;
		  break;
	  }

	  //2 cells are panoramic
	  case E_VIDEO_LAYOUT_1X2HOR:
	  case E_VIDEO_LAYOUT_1X2VER:
	  {
		  if (cellIndex == 0 || cellIndex == 1) //The panoramic cells are the first and the second cells
			  isPanoramic = true;
		  break;
	  }

	  default:
	  {
		  if (layoutType != 0)
		  {
			DBGFPASSERT(layoutType);
		  }
		  else
		  {
			DBGFPASSERT(1001);
		  }
	  }
	} // switch

	TRACEINTO << "layout: " << layoutType << ", cellIndex: " << cellIndex << ", isPanoramicCell: " <<  isPanoramic;

	return isPanoramic;
}

////////////////////////////////////////////////////////////////////////////
BYTE CVideoHardwareInterface::IsOverlayOrITPLayout(ELayoutType layoutType)
{
	return (IsITPOverlayLayout(layoutType) || IsOverlayLayout(layoutType));
}

////////////////////////////////////////////////////////////////////////////
BYTE CVideoHardwareInterface::IsITPOverlayLayout(ELayoutType layoutType)
{
	return ((layoutType == E_VIDEO_LAYOUT_OVERLAY_ITP_1P4) || (layoutType == E_VIDEO_LAYOUT_OVERLAY_ITP_1P3) || (layoutType == E_VIDEO_LAYOUT_OVERLAY_ITP_1P2));
}

////////////////////////////////////////////////////////////////////////////
BYTE CVideoHardwareInterface::IsOverlayLayout(ELayoutType layoutType)
{
	return ((layoutType == E_VIDEO_LAYOUT_OVERLAY_1P1) || (layoutType == E_VIDEO_LAYOUT_OVERLAY_1P2) || (layoutType == E_VIDEO_LAYOUT_OVERLAY_1P3));
}
////////////////////////////////////////////////////////////////////////////

void CVideoHardwareInterface::FillChangeLayoutAttributes(const CLayout* pLayout, CHANGE_LAYOUT_ATTRIBUTES_S& pChangeLayoutAttibutesStruct, CVisualEffectsParams* pVisualEffects, CSiteNameInfo* pSiteNameInfo, DWORD speakerPlaceInLayout, eVideoConfType videoConfType)
{
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pVisualEffects));

	if (speakerPlaceInLayout != INVALID)
		speakerPlaceInLayout++; // Send Value to HW 1-16 and not 0-15

	// if Border disabled - send width 0
	eLayoutBorderWidth layoutBorderWidthToSend = pVisualEffects->IslayoutBorderEnable() ? pVisualEffects->GetlayoutBorderWidth() : eLayoutBorderNone;

	// if no speaker in layout - send speaker notation width 0
	eLayoutBorderWidth speakerBorderWidthToSend = (INVALID == speakerPlaceInLayout) ? eLayoutBorderNone : eLayoutBorderNormal;

	memset(&pChangeLayoutAttibutesStruct, 0, sizeof(CHANGE_LAYOUT_ATTRIBUTES_S));

	pChangeLayoutAttibutesStruct.tBorder.tVisualAttributes.nColor     = pVisualEffects->GetlayoutBorderColorYUV();
	pChangeLayoutAttibutesStruct.tBorder.tVisualAttributes.nThickness = (DWORD)TranslatBorderWidthToApi(layoutBorderWidthToSend);
	pChangeLayoutAttibutesStruct.tBorder.tVisualAttributes.nTexture   = (DWORD)INVALID;     // border texture currently always INVALID;

	BORDER_PARAM_S sCurrentCellTelepresenceBorder, sNoTelepresenceSpecificBorder = gTelepresenceCellBorders[eTelepresenceBordersNotSpecified];
	ELayoutType layoutType = TranslateVideoLayoutTypeToApi(pLayout->GetLayoutType());
	bool bTelepresenceBordersSpecified = false;
	bool bTelepresenceSpecificBorderFoundForCell = false;
	for (int i = 0; i < MAX_SUB_IMAGES_IN_LAYOUT; i++)
	{
		const CVidSubImage* pVidSubImage = (*pLayout)[i];


		sCurrentCellTelepresenceBorder = (pVisualEffects->GetSpecifiedBorders() != NULL)? pVisualEffects->GetSpecifiedBorders()->tBorderEdges[i] : sNoTelepresenceSpecificBorder;
		bTelepresenceSpecificBorderFoundForCell = !(sCurrentCellTelepresenceBorder == sNoTelepresenceSpecificBorder);
		if (bTelepresenceSpecificBorderFoundForCell)
		{
			TRACEINTO << "Found specific borders for viewer party cell " << i;
			pChangeLayoutAttibutesStruct.tBorder.tBorderEdges[i] = sCurrentCellTelepresenceBorder;
			bTelepresenceBordersSpecified = true;
		}
		else
		{
			if ((pVidSubImage && (!(IsOverlayOrITPLayout(layoutType)) && (layoutBorderWidthToSend != eLayoutBorderNone)))

					|| (pVidSubImage && IsOverlayLayout(layoutType))
			)
			{
				pChangeLayoutAttibutesStruct.tBorder.tBorderEdges[i].ucRight = 1;
				pChangeLayoutAttibutesStruct.tBorder.tBorderEdges[i].ucLeft  = 1;
				pChangeLayoutAttibutesStruct.tBorder.tBorderEdges[i].ucUp    = 1;
				pChangeLayoutAttibutesStruct.tBorder.tBorderEdges[i].ucDown  = 1;
			}

			// else if (pVidSubImage &&
			if (pVidSubImage && IsITPOverlayLayout(layoutType))
			{
				TRACEINTO << "Layout:" << layoutType;
				pChangeLayoutAttibutesStruct.tBorder.tBorderEdges[i].ucRight = gInitCellBoder[layoutType].cell_borders[i].ucRight;
				pChangeLayoutAttibutesStruct.tBorder.tBorderEdges[i].ucLeft  = gInitCellBoder[layoutType].cell_borders[i].ucLeft;
				pChangeLayoutAttibutesStruct.tBorder.tBorderEdges[i].ucUp    = gInitCellBoder[layoutType].cell_borders[i].ucUp;
				pChangeLayoutAttibutesStruct.tBorder.tBorderEdges[i].ucDown  = gInitCellBoder[layoutType].cell_borders[i].ucDown;
			}
		}
	}

	if (layoutBorderWidthToSend == eLayoutBorderNone)
	{
		if (IsOverlayOrITPLayout(layoutType))
			pChangeLayoutAttibutesStruct.tBorder.tVisualAttributes.nThickness = E_LAYOUT_BORDER_WIDTH_NORMAL;
	}
	// EMB_MLA_GUY borders (for filmstrip TP layouts) need a width > 0 to be displayed
	if (bTelepresenceBordersSpecified && (pChangeLayoutAttibutesStruct.tBorder.tVisualAttributes.nThickness == E_LAYOUT_BORDER_WIDTH_DUMMY))
	{
		TRACEINTO <<  "setting borders for telepresence layout. prev color value is " << pChangeLayoutAttibutesStruct.tBorder.tVisualAttributes.nColor ;
		pChangeLayoutAttibutesStruct.tBorder.tVisualAttributes.nThickness =	E_LAYOUT_BORDER_WIDTH_NORMAL;
		/*DWORD dwBorderColorYUV=0;
		pVisualEffects->TranslateRGBColorToYUV(0x66ff99, dwBorderColorYUV);*/
		TRACEINTO <<  "setting borders for telepresence layout borders. color value is " << pChangeLayoutAttibutesStruct.tBorder.tVisualAttributes.nColor
				<< " setting to color " << pVisualEffects->GetSpeakerNotationColorYUV();
		// BRIDGE-15700 - color for filmstrip border and speaker notations should be identical
		pChangeLayoutAttibutesStruct.tBorder.tVisualAttributes.nColor = pVisualEffects->GetSpeakerNotationColorYUV();//dwBorderColorYUV;
	}

	if (pVisualEffects->IsSpeakerNotationEnable())
	{
		pChangeLayoutAttibutesStruct.tSpeaker.nSpeakerImageID              = (DWORD)speakerPlaceInLayout;
		pChangeLayoutAttibutesStruct.tSpeaker.tVisualAttributes.nColor     = pVisualEffects->GetSpeakerNotationColorYUV();
		pChangeLayoutAttibutesStruct.tSpeaker.tVisualAttributes.nThickness = TranslatBorderWidthToApi(speakerBorderWidthToSend);
		pChangeLayoutAttibutesStruct.tSpeaker.tVisualAttributes.nTexture   = (DWORD)INVALID;  // speaker notation texture currently always INVALID
	}

	pChangeLayoutAttibutesStruct.tBackground.nColor   = pVisualEffects->GetBackgroundColorYUV();
	pChangeLayoutAttibutesStruct.tBackground.nImageId = pVisualEffects->GetBackgroundImageID();

	BOOL bSiteNameEnabled = FALSE;
	if (GetSystemCardsBasedMode() == eSystemCardsMode_mpm_plus)
		bSiteNameEnabled = pVisualEffects->IsSiteNamesEnable();
	else
		bSiteNameEnabled = (pSiteNameInfo && pSiteNameInfo->GetDisplayMode() != eSiteNameOff);

	if (pSiteNameInfo != NULL)
	{
		if (bSiteNameEnabled == TRUE)
		{
			if (pVisualEffects->GetBackgroundImageID() == 1 || pVisualEffects->GetBackgroundImageID() == 2)
			{
				pSiteNameInfo->EnableBorder(TRUE);
			}

			FillSiteNamesParams(pChangeLayoutAttibutesStruct.tSitenames, pSiteNameInfo);
		}
	}

	FillFadeInFadeOutParams(pChangeLayoutAttibutesStruct.tFadeInFadeOut, videoConfType);
}

////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::FillChangeLayoutAttributes(const CLayout* pLayout, MCMS_CM_CHANGE_LAYOUT_ATTRIBUTES_S& pChangeLayoutAttibutesStruct, CVisualEffectsParams* pVisualEffects, CSiteNameInfo* pSiteNameInfo, DWORD speakerPlaceInLayout, eVideoConfType videoConfType)
{
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pVisualEffects));

	if (speakerPlaceInLayout != INVALID)
		speakerPlaceInLayout++;                                                               // Send Value to HW 1-16 and not 0-15

	// if Border disabled - send width 0
	eLayoutBorderWidth layoutBorderWidthToSend = pVisualEffects->IslayoutBorderEnable() ? pVisualEffects->GetlayoutBorderWidth() : eLayoutBorderNone;

	// if no speaker in layout - send speaker notation width 0
	eLayoutBorderWidth speakerBorderWidthToSend = (INVALID == speakerPlaceInLayout) ? eLayoutBorderNone : eLayoutBorderNormal;

	memset(&pChangeLayoutAttibutesStruct, 0, sizeof(MCMS_CM_CHANGE_LAYOUT_ATTRIBUTES_S));

	pChangeLayoutAttibutesStruct.tBorder.tVisualAttributes.nColor     = pVisualEffects->GetlayoutBorderColorYUV();
	pChangeLayoutAttibutesStruct.tBorder.tVisualAttributes.nThickness = (DWORD)TranslatBorderWidthToApi(layoutBorderWidthToSend);
// /  pChangeLayoutAttibutesStruct.tBorder.tVisualAttributes.nTexture   = (DWORD) INVALID;    // border texture currently always INVALID;

	if (pVisualEffects->IsSpeakerNotationEnable())
	{
		pChangeLayoutAttibutesStruct.tSpeaker.nSpeakerImageID              = (DWORD)speakerPlaceInLayout;
		pChangeLayoutAttibutesStruct.tSpeaker.tVisualAttributes.nColor     = pVisualEffects->GetSpeakerNotationColorYUV();
		pChangeLayoutAttibutesStruct.tSpeaker.tVisualAttributes.nThickness = TranslatBorderWidthToApi(speakerBorderWidthToSend);
// /    pChangeLayoutAttibutesStruct.tSpeaker.tVisualAttributes.nTexture   = (DWORD) INVALID; // speaker notation texture currently always INVALID
	}

	BORDER_PARAM_S sCurrentCellTelepresenceBorder, sNoTelepresenceSpecificBorder = gTelepresenceCellBorders[eTelepresenceBordersNotSpecified];
	ELayoutType layoutType = TranslateVideoLayoutTypeToApi(pLayout->GetLayoutType());
	// EMB_MLA_GUY update cell borders according to TP layout (26/6/14 currently for speaker mode, in filmstrip only)
	std::ostringstream msg;
	DumpTelepresenceBordersInfo(msg, pVisualEffects->GetSpecifiedBorders());
	TRACEINTO << msg.str().c_str();
	bool bTelepresenceBordersSpecified = false;
	for (int i = 0; i < MAX_SUB_IMAGES_IN_LAYOUT; i++)
	{
		const CVidSubImage* pVidSubImage = (*pLayout)[i];
		sCurrentCellTelepresenceBorder = (pVisualEffects->GetSpecifiedBorders() != NULL)? pVisualEffects->GetSpecifiedBorders()->tBorderEdges[i] : sNoTelepresenceSpecificBorder;
		bool bTelepresenceSpecificBorderFoundForCell = !(sCurrentCellTelepresenceBorder == sNoTelepresenceSpecificBorder);

		if (bTelepresenceSpecificBorderFoundForCell)
		{
			TRACEINTO << "Found specific borders for viewer party cell " << i;
			pChangeLayoutAttibutesStruct.tBorder.tBorderEdges[i].ucRight = sCurrentCellTelepresenceBorder.ucRight;
			pChangeLayoutAttibutesStruct.tBorder.tBorderEdges[i].ucLeft  = sCurrentCellTelepresenceBorder.ucLeft;
			pChangeLayoutAttibutesStruct.tBorder.tBorderEdges[i].ucUp    = sCurrentCellTelepresenceBorder.ucUp;
			pChangeLayoutAttibutesStruct.tBorder.tBorderEdges[i].ucDown  = sCurrentCellTelepresenceBorder.ucDown;
			bTelepresenceBordersSpecified = true;
		}
		else
		{
			if ((pVidSubImage && (!(IsOverlayLayout(layoutType) || IsITPOverlayLayout(layoutType)) && (layoutBorderWidthToSend != eLayoutBorderNone)))
					|| (pVidSubImage && IsOverlayLayout(layoutType)))
			{
				pChangeLayoutAttibutesStruct.tBorder.tBorderEdges[i].ucRight = 1;
				pChangeLayoutAttibutesStruct.tBorder.tBorderEdges[i].ucLeft  = 1;
				pChangeLayoutAttibutesStruct.tBorder.tBorderEdges[i].ucUp    = 1;
				pChangeLayoutAttibutesStruct.tBorder.tBorderEdges[i].ucDown  = 1;
			}

			// else if (pVidSubImage &&
			if (pVidSubImage && IsITPOverlayLayout(layoutType))
			{
				pChangeLayoutAttibutesStruct.tBorder.tBorderEdges[i].ucRight = gInitCellBoder[layoutType].cell_borders[i].ucRight;
				pChangeLayoutAttibutesStruct.tBorder.tBorderEdges[i].ucLeft  = gInitCellBoder[layoutType].cell_borders[i].ucLeft;
				pChangeLayoutAttibutesStruct.tBorder.tBorderEdges[i].ucUp    = gInitCellBoder[layoutType].cell_borders[i].ucUp;
				pChangeLayoutAttibutesStruct.tBorder.tBorderEdges[i].ucDown  = gInitCellBoder[layoutType].cell_borders[i].ucDown;
			}
		}
	}
	// EMB_MLA_GUY borders (for filmstrip TP layouts) need a width > 0 to be displayed
	if (bTelepresenceBordersSpecified && (pChangeLayoutAttibutesStruct.tBorder.tVisualAttributes.nThickness == E_LAYOUT_BORDER_WIDTH_DUMMY))
	{
		pChangeLayoutAttibutesStruct.tBorder.tVisualAttributes.nThickness =	E_LAYOUT_BORDER_WIDTH_NORMAL;
		/*DWORD dwBorderColorYUV=0;
		pVisualEffects->TranslateRGBColorToYUV(0x66ff99, dwBorderColorYUV);*/
		TRACEINTO <<  "setting borders for telepresence layout borders. color value is " << pChangeLayoutAttibutesStruct.tBorder.tVisualAttributes.nColor
				<< " setting to color " << pVisualEffects->GetSpeakerNotationColorYUV();
		// BRIDGE-15700 - color for filmstrip border and speaker notations should be identical
		pChangeLayoutAttibutesStruct.tBorder.tVisualAttributes.nColor = pVisualEffects->GetSpeakerNotationColorYUV();//dwBorderColorYUV;
	}
	pChangeLayoutAttibutesStruct.tBackground.nColor   = pVisualEffects->GetBackgroundColorYUV();
	pChangeLayoutAttibutesStruct.tBackground.nImageId = pVisualEffects->GetBackgroundImageID();

	BOOL bSiteNameEnabled = FALSE;
	if (GetSystemCardsBasedMode() == eSystemCardsMode_mpm_plus)
		bSiteNameEnabled = pVisualEffects->IsSiteNamesEnable();
	else
		bSiteNameEnabled = (pSiteNameInfo && pSiteNameInfo->GetDisplayMode() != eSiteNameOff);

	if (pSiteNameInfo != NULL)
	{
		if (bSiteNameEnabled == TRUE)
		{
			if (pVisualEffects->GetBackgroundImageID() == 1 || pVisualEffects->GetBackgroundImageID() == 2)
			{
				pSiteNameInfo->EnableBorder(TRUE);
			}
			FillSiteNamesParams(pChangeLayoutAttibutesStruct.tSitenames, pSiteNameInfo);
		}
	}

	FillFadeInFadeOutParams(pChangeLayoutAttibutesStruct.tFadeInFadeOut, videoConfType);
}

////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::FillSiteNamesParams(SITENAMES_PARAM_S& pSiteNamesParams, CSiteNameInfo* pSiteNameInfo)
{
	FontColor font_color;
	TRACEINTO << "IsBorderEnalbed:" << (int)pSiteNameInfo->IsEnalbeBorder();
	if (pSiteNameInfo->IsEnalbeBorder() == TRUE)
		font_color = TranslateMessageOverlayColorTypeToApi((eMessageOverlayColorType)pSiteNameInfo->GetColorType());
	else
		font_color = TranslateTextColorTypeToApi((eTextColorType)pSiteNameInfo->GetTextColorType());

	pSiteNamesParams.nTextColor       = font_color.nTextColor;
	pSiteNamesParams.nBackgroundColor = font_color.nBackgroundColor;
	pSiteNamesParams.nFontSize        = pSiteNameInfo->GetFontSize();
	pSiteNamesParams.nTransparency    = pSiteNameInfo->GetTransParence();
	pSiteNamesParams.nShadowWidth     = 1;
	pSiteNamesParams.nStripHeight     = 16;
	pSiteNameInfo->GetCoordinateXY(pSiteNamesParams.nSiteNamesHorPosition, pSiteNamesParams.nSiteNamesVerPosition);
}

////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::FillSiteNamesParams(MCMS_CM_SITENAMES_PARAM_S& pSiteNamesParams, CSiteNameInfo* pSiteNameInfo)
{
	FontColor font_color;
	TRACEINTO << "IsBorderEnalbed:" << (int)pSiteNameInfo->IsEnalbeBorder();
	if (pSiteNameInfo->IsEnalbeBorder() == TRUE)
		font_color = TranslateMessageOverlayColorTypeToApi((eMessageOverlayColorType)pSiteNameInfo->GetColorType());
	else
		font_color = TranslateTextColorTypeToApi((eTextColorType)pSiteNameInfo->GetTextColorType());

	pSiteNamesParams.nTextColor       = font_color.nTextColor;
	pSiteNamesParams.nBackgroundColor = font_color.nBackgroundColor;
	pSiteNamesParams.nFontSize        = pSiteNameInfo->GetFontSize();
	pSiteNamesParams.nTransparency    = pSiteNameInfo->GetTransParence();
	pSiteNamesParams.nShadowWidth     = 1;
	pSiteNamesParams.nStripHeight     = 16;
	pSiteNameInfo->GetCoordinateXY(pSiteNamesParams.nSiteNamesHorPosition, pSiteNamesParams.nSiteNamesVerPosition);
}

////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::FillFadeInFadeOutParams(FADE_IN_FADE_OUT_PARAM_S& pFadeInFadeOutParams, eVideoConfType videoConfType)
{
	BOOL IsFadeInFadeOut = FALSE;
	if ((videoConfType != eVideoConfTypeCopHD108025fps) && (videoConfType != eVideoConfTypeCopHD72050fps))
		CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("FADE_IN_FADE_OUT", IsFadeInFadeOut);

	pFadeInFadeOutParams.nEffectDurationMsec = (IsFadeInFadeOut) ? 500 : 0;
}

////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::SendChangeLayoutAttributes(const CLayout* pLayout, CVisualEffectsParams* pVisualEffects, DWORD speakerPlaceInLayout, CSiteNameInfo* pSiteNameInfo, eVideoConfType videoConfType)
{
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pVisualEffects));

	CHANGE_LAYOUT_ATTRIBUTES_S tChangeLayoutAttributesStruct;
	memset(&tChangeLayoutAttributesStruct, 0, sizeof(CHANGE_LAYOUT_ATTRIBUTES_S));
	FillChangeLayoutAttributes(pLayout, tChangeLayoutAttributesStruct, pVisualEffects, pSiteNameInfo, speakerPlaceInLayout, videoConfType);

	CSegment* pMsg = new CSegment;
	pMsg->Put((BYTE*)(&tChangeLayoutAttributesStruct), sizeof(CHANGE_LAYOUT_ATTRIBUTES_S));
	SendMsgToMPL(VIDEO_ENCODER_CHANGE_LAYOUT_ATTRIBUTES_REQ, pMsg);
	POBJDELETE(pMsg);
}

////////////////////////////////////////////////////////////////////////////
STATUS CVideoHardwareInterface::SendShowSlide(CSegment* pDataSeg, eVideoResolution videoResolution, DWORD videoAlg, DWORD videoBitRate, DWORD fs, DWORD mbps, BYTE isTipMode)
{
	// Saving the original CIVRPlayMessage message sent from CAM for future updates
	if (m_pOriginalIVRShowSlidePlayMessage)
		PDELETE(m_pOriginalIVRShowSlidePlayMessage);

	m_pOriginalIVRShowSlidePlayMessage = new CIVRPlayMessage;
	m_pOriginalIVRShowSlidePlayMessage->DeSerialize(pDataSeg);
	return ShowSlide(videoResolution, videoAlg, videoBitRate, fs, mbps, isTipMode);
}


////////////////////////////////////////////////////////////////////////////
STATUS CVideoHardwareInterface::SendUpdateSlide(eVideoResolution videoResolution, DWORD videoAlg, DWORD videoBitRate, DWORD fs, DWORD mbps, BYTE isTipMode)
{
	PASSERT_AND_RETURN_VALUE(!m_pOriginalIVRShowSlidePlayMessage, STATUS_FAIL);

	return ShowSlide(videoResolution, videoAlg, videoBitRate, fs, mbps, isTipMode);
}
////////////////////////////////////////////////////////////////////////////
STATUS CVideoHardwareInterface::ShowSlide(eVideoResolution videoResolution, DWORD videoAlg, DWORD videoBitRate, DWORD fs, DWORD mbps, BYTE isTipMode)
{
	// Create copy of CIVRPlayMessage - this object will be modified with additional params - inorder to avoid changing original saved message from CAM for future updates
	CIVRPlayMessage* pIVRShowSlidePlayMessageToBeSent = new CIVRPlayMessage;
	CSegment*        pTmpDataSeg                      = new CSegment;

	m_pOriginalIVRShowSlidePlayMessage->Serialize(pTmpDataSeg);
	pIVRShowSlidePlayMessageToBeSent->DeSerialize(pTmpDataSeg);
	POBJDELETE(pTmpDataSeg);

	SIVRPlayMessageStruct* pPlayMsg = &(pIVRShowSlidePlayMessageToBeSent->play);

	CIVRSlidesList* pSlidesList = ::GetpSlidesList();
	if (!pSlidesList)
	{
		PASSERT(1);
		POBJDELETE(pIVRShowSlidePlayMessageToBeSent);
		return STATUS_FAIL;
	}

	if (!pPlayMsg->mediaFiles)
	{
		PASSERT(2);
		POBJDELETE(pIVRShowSlidePlayMessageToBeSent);
		return STATUS_FAIL;
	}

	WORD   checkSum;
	DWORD  slideDataSize; // currently not in use we pass dummy to function
	char*  pFullSlideName = new char[MAX_FULL_PATHNAME];


	STATUS status = pSlidesList->GetSlideParams((pPlayMsg->mediaFiles)->fileName, videoResolution, videoAlg, TranslateFSToMacroBlocksUnits(videoAlg, fs), TranslateMBPSToMacroBlocksUnits(videoAlg, mbps), pFullSlideName, MAX_FULL_PATHNAME, checkSum, slideDataSize,isTipMode);
	if (status != STATUS_OK)
	{
		TRACEINTO << "status:" << status << " - Failed to get slide params";
		POBJDELETE(pIVRShowSlidePlayMessageToBeSent);
		PDELETEA(pFullSlideName);
		return status;
	}

	// update struct params
	strncpy(pPlayMsg->mediaFiles->fileName, pFullSlideName, sizeof(pPlayMsg->mediaFiles->fileName) - 1);
	pPlayMsg->mediaFiles->fileName[sizeof(pPlayMsg->mediaFiles->fileName) - 1] = '\0';

	pPlayMsg->videoBitRate = videoBitRate;
	pPlayMsg->mediaFiles->checksum = checkSum;
	pPlayMsg->mediaFiles->fileNameLength = strlen(pFullSlideName);
	pPlayMsg->isTipMode = isTipMode;

	CSegment* pSeg = new CSegment;
	pIVRShowSlidePlayMessageToBeSent->Serialize(pSeg);
	SendMsgToMPL(IVR_SHOW_SLIDE_REQ, pSeg);

	POBJDELETE(pIVRShowSlidePlayMessageToBeSent);
	PDELETEA(pFullSlideName);
	POBJDELETE(pSeg);
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::SendStopShowSlide(CSegment* pDataSeg)
{
	SendMsgToMPL(IVR_STOP_SHOW_SLIDE_REQ, pDataSeg);
	if (m_pOriginalIVRShowSlidePlayMessage)
		PDELETE(m_pOriginalIVRShowSlidePlayMessage);
}
////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::SendStartPLC(CSegment* pDataSeg)
{
	SendMsgToMPL(VIDEO_GRAPHIC_OVERLAY_START_REQ, pDataSeg);
}

////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::SendStopPLC(CSegment* pDataSeg)
{
	SendMsgToMPL(VIDEO_GRAPHIC_OVERLAY_STOP_REQ, pDataSeg);
}

////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::SendStopSiteNameDisplay()
{
	SendMsgToMPL(VIDEO_ENCODER_SITE_NAMES_OFF, NULL);
}

////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::SendStopTextDisplay()
{
	SendMsgToMPL(VIDEO_GRAPHICS_HIDE_TEXT_BOX_REQ, NULL);
}

////////////////////////////////////////////////////////////////////////////
ELayoutType CVideoHardwareInterface::TranslateVideoLayoutTypeToApi(LayoutType layout)
{
	switch (layout)
	{
		case CP_LAYOUT_1X1                  : return E_VIDEO_LAYOUT_1X1;
		case CP_LAYOUT_1X2                  : return E_VIDEO_LAYOUT_1X2;
		case CP_LAYOUT_2X1                  : return E_VIDEO_LAYOUT_2X1;
		case CP_LAYOUT_2X2                  : return E_VIDEO_LAYOUT_2X2;
		case CP_LAYOUT_3X3                  : return E_VIDEO_LAYOUT_3X3;
		case CP_LAYOUT_1P5                  : return E_VIDEO_LAYOUT_1P5;
		case CP_LAYOUT_1P7                  : return E_VIDEO_LAYOUT_1P7;
		case CP_LAYOUT_1x2VER               : return E_VIDEO_LAYOUT_1X2VER;
		case CP_LAYOUT_1x2HOR               : return E_VIDEO_LAYOUT_1X2HOR;
		case CP_LAYOUT_1P2HOR               : return E_VIDEO_LAYOUT_1P2HOR;
		case CP_LAYOUT_1P2VER               : return E_VIDEO_LAYOUT_1P2VER;
		case CP_LAYOUT_1P3HOR               : return E_VIDEO_LAYOUT_1P3HOR;
		case CP_LAYOUT_1P3VER               : return E_VIDEO_LAYOUT_1P3VER;
		case CP_LAYOUT_1P4HOR               : return E_VIDEO_LAYOUT_1P4HOR;
		case CP_LAYOUT_1P4VER               : return E_VIDEO_LAYOUT_1P4VER;
		case CP_LAYOUT_1P8CENT              : return E_VIDEO_LAYOUT_1P8CENT;
		case CP_LAYOUT_1P8UP                : return E_VIDEO_LAYOUT_1P8UP;
		case CP_LAYOUT_1P2HOR_UP            : return E_VIDEO_LAYOUT_1P2HOR_UP;
		case CP_LAYOUT_1P3HOR_UP            : return E_VIDEO_LAYOUT_1P3HOR_UP;
		case CP_LAYOUT_1P4HOR_UP            : return E_VIDEO_LAYOUT_1P4HOR_UP;
		case CP_LAYOUT_1P8HOR_UP            : return E_VIDEO_LAYOUT_1P8HOR_UP;
		case CP_LAYOUT_4X4                  : return E_VIDEO_LAYOUT_4X4;
		case CP_LAYOUT_2P8                  : return E_VIDEO_LAYOUT_2P8;
		case CP_LAYOUT_1P12                 : return E_VIDEO_LAYOUT_1P12;
		case CP_LAYOUT_1X2_FLEX             : return E_VIDEO_LAYOUT_FLEX_1X2;
		case CP_LAYOUT_1P2HOR_RIGHT_FLEX    : return E_VIDEO_LAYOUT_FLEX_1P2HOR_RIGHT;
		case CP_LAYOUT_1P2HOR_LEFT_FLEX     : return E_VIDEO_LAYOUT_FLEX_1P2HOR_LEFT;
		case CP_LAYOUT_1P2HOR_UP_RIGHT_FLEX : return E_VIDEO_LAYOUT_FLEX_1P2HOR_UP_RIGHT;
		case CP_LAYOUT_1P2HOR_UP_LEFT_FLEX  : return E_VIDEO_LAYOUT_FLEX_1P2HOR_UP_LEFT;
		case CP_LAYOUT_2X2_UP_RIGHT_FLEX    : return E_VIDEO_LAYOUT_FLEX_2X2_UP_RIGHT;
		case CP_LAYOUT_2X2_UP_LEFT_FLEX     : return E_VIDEO_LAYOUT_FLEX_2X2_UP_LEFT;
		case CP_LAYOUT_2X2_DOWN_RIGHT_FLEX  : return E_VIDEO_LAYOUT_FLEX_2X2_DOWN_RIGHT;
		case CP_LAYOUT_2X2_DOWN_LEFT_FLEX   : return E_VIDEO_LAYOUT_FLEX_2X2_DOWN_LEFT;
		case CP_LAYOUT_2X2_RIGHT_FLEX       : return E_VIDEO_LAYOUT_FLEX_2X2_RIGHT;
		case CP_LAYOUT_2X2_LEFT_FLEX        : return E_VIDEO_LAYOUT_FLEX_2X2_LEFT;
		case CP_LAYOUT_OVERLAY_1P1          : return E_VIDEO_LAYOUT_OVERLAY_1P1;
		case CP_LAYOUT_OVERLAY_1P2          : return E_VIDEO_LAYOUT_OVERLAY_1P2;
		case CP_LAYOUT_OVERLAY_1P3          : return E_VIDEO_LAYOUT_OVERLAY_1P3;
		case CP_LAYOUT_OVERLAY_ITP_1P2      : return E_VIDEO_LAYOUT_OVERLAY_ITP_1P2;
		case CP_LAYOUT_OVERLAY_ITP_1P3      : return E_VIDEO_LAYOUT_OVERLAY_ITP_1P3;
		case CP_LAYOUT_OVERLAY_ITP_1P4      : return E_VIDEO_LAYOUT_OVERLAY_ITP_1P4;
		case CP_LAYOUT_1TOP_LEFT_P8         : return E_VIDEO_LAYOUT_1TOP_LEFT_P8;
		case CP_LAYOUT_2TOP_P8              : return E_VIDEO_LAYOUT_2TOP_P8;
		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
	}
	PASSERT(layout+1000);
	return E_VIDEO_LAYOUT_DUMMY;
}

////////////////////////////////////////////////////////////////////////////
EVideoProtocol CVideoHardwareInterface::TranslateVideoProtocolToApi(DWORD videoProtocol)
{
	switch (videoProtocol)
	{
		case H261:
			return E_VIDEO_PROTOCOL_H261;
		case H263:
			return E_VIDEO_PROTOCOL_H263;
		case H264:
			return E_VIDEO_PROTOCOL_H264;
		case RTV:
			return E_VIDEO_PROTOCOL_RTV;
		case SVC:
			return E_VIDEO_PROTOCOL_SVC;
		case MS_SVC:
			return E_VIDEO_PROTOCOL_MS_SVC;
		case VP8:
			return E_VIDEO_PROTOCOL_VP8;

		default:
			FPASSERT(videoProtocol+1000);
			return E_VIDEO_PROTOCOL_DUMMY;
	}
}

////////////////////////////////////////////////////////////////////////////
ETelePresenceMode CVideoHardwareInterface::TranslateTelePresenceModeToApi(eTelePresencePartyType eTelePresenceMode)
{
	switch (eTelePresenceMode)
	{
		case eTelePresencePartyNone: return E_TELEPRESENCE_MODE_NONE;
		case eTelePresencePartyRPX : return E_TELEPRESENCE_MODE_RPX;
		case eTelePresencePartyFlex: return E_TELEPRESENCE_MODE_FLEX;
		case eTelePresencePartyMaui: return E_TELEPRESENCE_MODE_FLEX;	//N.A. - Maui behaves the same as Flex other than zoomOut for initial integration.
		case eTelePresencePartyCTS : return E_TELEPRESENCE_MODE_FLEX; //cts behave the same as FLEX and also the changes are only relvant form mpm plus which is not supporeted anyway in mpmx.

		default:
			FPASSERT(eTelePresenceMode+1000);
			return E_TELEPRESENCE_MODE_DUMMY;
	}
}

////////////////////////////////////////////////////////////////////////////
EVideoFrameRate CVideoHardwareInterface::TranslateVideoFrameRateToApi(eVideoFrameRate videoFrameRate)
{
	switch (videoFrameRate)
	{
		case eVideoFrameRateDUMMY  : return E_VIDEO_FPS_DUMMY;
		case eVideoFrameRate60FPS  : return E_VIDEO_60_FPS;
		case eVideoFrameRate50FPS  : return E_VIDEO_50_FPS;
		case eVideoFrameRate30FPS  : return E_VIDEO_30_FPS;
		case eVideoFrameRate25FPS  : return E_VIDEO_25_FPS;
		case eVideoFrameRate20FPS  : return E_VIDEO_20_FPS;
		case eVideoFrameRate15FPS  : return E_VIDEO_15_FPS;
		case eVideoFrameRate12_5FPS: return E_VIDEO_12_5_FPS;
		case eVideoFrameRate10FPS  : return E_VIDEO_10_FPS;
		case eVideoFrameRate7_5FPS : return E_VIDEO_7_5_FPS;
		case eVideoFrameRate6FPS   : return E_VIDEO_6_FPS;
		case eVideoFrameRate5FPS   : return E_VIDEO_5_FPS;
		case eVideoFrameRate3FPS   : return E_VIDEO_3_FPS;

		default:
			FPASSERT(videoFrameRate+1000);
			return E_VIDEO_FPS_DUMMY;
	}
}

////////////////////////////////////////////////////////////////////////////
float CVideoHardwareInterface::TranslateVideoFrameRateToNumeric(eVideoFrameRate videoFrameRate)
{
	switch (videoFrameRate)
	{
		case eVideoFrameRateDUMMY  : return 0.0f;
		case eVideoFrameRate60FPS  : return 60.0f;
		case eVideoFrameRate50FPS  : return 50.0f;
		case eVideoFrameRate30FPS  : return 30.0f;
		case eVideoFrameRate25FPS  : return 25.0f;
		case eVideoFrameRate20FPS  : return 20.0f;
		case eVideoFrameRate15FPS  : return 15.0f;
		case eVideoFrameRate12_5FPS: return 12.5f;
		case eVideoFrameRate10FPS  : return 10.0f;
		case eVideoFrameRate7_5FPS : return 7.5f;
		case eVideoFrameRate6FPS   : return 6.0f;
		case eVideoFrameRate5FPS   : return 5.0f;
		case eVideoFrameRate3FPS   : return 3.0f;

		default:
			FPASSERT(videoFrameRate+1000);
			return 0.0f;
	}
}

////////////////////////////////////////////////////////////////////////////
EVideoResolution CVideoHardwareInterface::TranslateVideoResolutionToApi(eVideoResolution videoResolution)
{
	switch (videoResolution)
	{
		case eVideoResolutionDummy : return E_VIDEO_RES_DUMMY;
		case eVideoResolutionQCIF  : return E_VIDEO_RES_QCIF;
		case eVideoResolutionCIF   : return E_VIDEO_RES_CIF;
		case eVideoResolutionVGA   : return E_VIDEO_RES_VGA;
		case eVideoResolution4SIF  : return E_VIDEO_RES_4CIF;
		case eVideoResolution4CIF  : return E_VIDEO_RES_4CIF;
		case eVideoResolution525SD : return E_VIDEO_RES_525SD;
		case eVideoResolution625SD : return E_VIDEO_RES_625SD;
		case eVideoResolutionSVGA  : return E_VIDEO_RES_SVGA;
		case eVideoResolutionXGA   : return E_VIDEO_RES_XGA;
		case eVideoResolutionHD720 : return E_VIDEO_RES_HD;
		case eVideoResolution16CIF : return E_VIDEO_RES_16CIF;
		case eVideoResolutionSIF   : return E_VIDEO_RES_SIF;
		case eVideoResolutionQVGA  : return E_VIDEO_RES_QVGA;
		case eVideoResolutionHD1080: return E_VIDEO_RES_1080HD;

		default:
			FPASSERT(videoResolution+1000);
			return E_VIDEO_RES_DUMMY;
	}
}

////////////////////////////////////////////////////////////////////////////
DWORD CVideoHardwareInterface::TranslateResolutionToResWidth(eVideoResolution resolution)
{
	for (int i = 0; i < eVideoResolutionLast; i++)
		if (ResoltionWidthAndHeight[i].eResolution == resolution)
			return ResoltionWidthAndHeight[i].resWidth;

	FPASSERT(resolution+1000);
	return 0;
}

////////////////////////////////////////////////////////////////////////////
DWORD CVideoHardwareInterface::TranslateResolutionToResHeight(eVideoResolution resolution)
{
	for (int i = 0; i < eVideoResolutionLast; i++)
		if (ResoltionWidthAndHeight[i].eResolution == resolution)
			return ResoltionWidthAndHeight[i].resHeight;

	FPASSERT(resolution+1000);
	return 0;
}

////////////////////////////////////////////////////////////////////////////
ERelativeSizeOfImageInLayout CVideoHardwareInterface::TranslateImageSizeToScale(WORD size_X, WORD size_Y)
{
	if (size_X > (CIF_X_SIZE >> 1) || size_Y > (CIF_Y_SIZE >> 1))
	{
		return E_FULL_SCREEN;
	}
	else
	{
		if (size_X > (CIF_X_SIZE >> 2) || size_Y > (CIF_Y_SIZE >> 2))
			return E_QUARTER_SCREEN;
		else
			return E_QQUARTER_SCREEN;
	}

	return E_NOT_IN_LAYOUT;
}

////////////////////////////////////////////////////////////////////////////
ELayoutBorderWidth CVideoHardwareInterface::TranslatBorderWidthToApi(eLayoutBorderWidth borderWidth)
{
	switch (borderWidth)
	{
		case eLayoutBorderNone  : return E_LAYOUT_BORDER_WIDTH_DUMMY;
		case eLayoutBorderThin  : return E_LAYOUT_BORDER_WIDTH_THIN;
		case eLayoutBorderNormal: return E_LAYOUT_BORDER_WIDTH_NORMAL;
		case eLayoutBorderThick : return E_LAYOUT_BORDER_WIDTH_THICK;

		default:
			FPASSERT(borderWidth+1000);
			return E_LAYOUT_BORDER_WIDTH_DUMMY;
	}
}

////////////////////////////////////////////////////////////////////////////
DWORD CVideoHardwareInterface::GetFsForAvcToSvc(DWORD width, DWORD hight)
{
	DWORD fs = ((width * hight) / 256) / CUSTOM_MAX_FS_FACTOR;

	if (!fs)
		fs = 1;

	return fs;
}

////////////////////////////////////////////////////////////////////////////
DWORD CVideoHardwareInterface::GetFsForSvcLync(DWORD width, DWORD height, BOOL adjustFs)
{
	DWORD fs = (width * height) / CUSTOM_MAX_FS_FACTOR;

	if (adjustFs)
		CMsSvcVideoMode::AdjustFS(width, height, fs);

	if (!fs)
		fs = 1;

	return fs;
}

////////////////////////////////////////////////////////////////////////////
DWORD CVideoHardwareInterface::TranslateToVideoResolutionRatio(DWORD videoAlg, eVideoResolution videoResolution, DWORD fs, DWORD mbps, eVideoConfType videoConfType, bool isVSW)
{
	DWORD vidResRatioType = RESOLUTION_RATIO_0;

	if (isVSW)
	{
		eProductType productType = CProcessBase::GetProcess()->GetProductType();
		
		if ((eProductTypeNinja == productType) ||
			(eProductTypeSoftMCU == productType) ||
			(eProductTypeEdgeAxis == productType))
			vidResRatioType = INVALID;            //BRIDGE-14290, Ninja and SoftMCU doesn't need encoder/decoder for VSW party. MP Proxy won't open enc/dec for INVALID value.
		else
			vidResRatioType = RESOLUTION_RATIO_1; // In VSW Conference the resolution ratio is the same for CP CIF party
	}

	else
	{
		switch (videoAlg)
		{
			case H264:
			case RTV:
			case SVC:
			case MS_SVC:
			{
				const DWORD qcifMaxFS = 1; // The division operation is heavy, (H264_L1_DEFAULT_FS/CUSTOM_MAX_FS_FACTOR) + (H264_L1_DEFAULT_FS%CUSTOM_MAX_FS_FACTOR? 1 : 0);
				const DWORD cifMaxFS = 2; // The division operation is heavy, (H264_L1_1_DEFAULT_FS/CUSTOM_MAX_FS_FACTOR) + (H264_L1_1_DEFAULT_FS%CUSTOM_MAX_FS_FACTOR? 1 : 0);
				const DWORD sdMaxFS = 9;
				const DWORD hd720MaxFS = 15;

				if (fs > hd720MaxFS)
					vidResRatioType = RESOLUTION_RATIO_64;
				else if (fs > sdMaxFS)
					vidResRatioType = RESOLUTION_RATIO_16;
				else if (fs > cifMaxFS)
					vidResRatioType = RESOLUTION_RATIO_4;
				else if (fs > qcifMaxFS)
					vidResRatioType = RESOLUTION_RATIO_1;

				break;
			}

			default:
			{
				switch (videoResolution)
				{
					case eVideoResolutionQCIF:
						vidResRatioType = RESOLUTION_RATIO_0;
						break;

					case eVideoResolutionQVGA:
					case eVideoResolutionCIF:
					case eVideoResolutionSIF:
						vidResRatioType = (videoConfType == eVideoConfTypeCP) ? RESOLUTION_RATIO_4 : RESOLUTION_RATIO_1;
						break;

					case eVideoResolutionVGA:
					case eVideoResolution4SIF:
					case eVideoResolution4CIF:
					case eVideoResolution525SD:
					case eVideoResolution625SD:
						vidResRatioType = (videoConfType == eVideoConfTypeCP) ? RESOLUTION_RATIO_16 : RESOLUTION_RATIO_4;
						break;

					case eVideoResolutionSVGA:
					case eVideoResolutionXGA:
					case eVideoResolutionHD720:
					case eVideoResolution16CIF:
						vidResRatioType = RESOLUTION_RATIO_16;
						break;

					case eVideoResolutionHD1080:
						vidResRatioType = RESOLUTION_RATIO_64;
						break;

					default:
						break;
				}

				break;
			}
		}
	}

	return vidResRatioType;
}

////////////////////////////////////////////////////////////////////////////
DWORD CVideoHardwareInterface::TranslateFSToMacroBlocksUnits(DWORD videoProtocol, DWORD fs)
{
	// The FS is in units of 256 luma macro blocks, we send the actual FS macro blocks
	DWORD H264fs = INVALID;
	if (videoProtocol == H264 || videoProtocol == RTV || videoProtocol == SVC)
		H264fs = fs * CUSTOM_MAX_FS_FACTOR;
	else if (videoProtocol == MS_SVC)
		H264fs = fs;

	return H264fs;
}

////////////////////////////////////////////////////////////////////////////
DWORD CVideoHardwareInterface::TranslateMBPSToMacroBlocksUnits(DWORD videoProtocol, DWORD mbps)
{
	// The MBPS is in units of units of 500 macro blocks per second, we send the actual MBPS in units of macro blocks per second
	DWORD H264mbps = INVALID;
	if (videoProtocol == H264 || videoProtocol == RTV || videoProtocol == SVC)
		H264mbps = mbps * CUSTOM_MAX_MBPS_FACTOR;
	else if (videoProtocol == MS_SVC)
		H264mbps = mbps;

	return H264mbps;
}

////////////////////////////////////////////////////////////////////////////
STATUS CVideoHardwareInterface::AllocateIconv(const char* toEncoding, const char* fromEncoding)
{
	m_pIconv = iconv_open(toEncoding, fromEncoding);

	if (m_pIconv == (iconv_t)-1)
	{
		PASSERTMSG(errno, "Failed to allocate iconv descriptor");
		return STATUS_FAIL;
	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CVideoHardwareInterface::DeallocatIconv()
{
	int ret_val = iconv_close(m_pIconv);
	if (ret_val == -1)
	{
		PASSERTMSG(errno, "Failed to deallocate iconv descriptor");
		return STATUS_FAIL;
	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
WORD CVideoHardwareInterface::ConvertStingEncoding(const char* from_buffer, DWORD from_buffer_len, char* to_buffer, DWORD to_buffer_len)
{
	if (m_pIconv == (iconv_t)-1)
	{
		TRACEINTO << "Failed, iconv descriptor not allocated";
		return 0;
	}

	if (from_buffer == NULL || from_buffer_len == 0)
	{
		return 0;
	}

	if (to_buffer == NULL || to_buffer_len == 0)
	{
		TRACEINTO << "Failed, target buffer size is 0";
		return 0;
	}

	char**  read_buffer    = (char**)&from_buffer;
	size_t  in_char_len    = from_buffer_len;
	size_t* in_char_count  = &in_char_len;
	char**  write_buffer   = &to_buffer;
	size_t  out_char_len   = to_buffer_len;
	size_t* out_char_count = &out_char_len;
	size_t  res            = iconv(m_pIconv, read_buffer, in_char_count, write_buffer, out_char_count);
	if (res == (size_t)-1)
	{
		PASSERTMSG(errno, "Failed to convert buffer");
		return 0;
	}

	WORD num_of_char_wrote = to_buffer_len - *out_char_count;

	return num_of_char_wrote;
}

////////////////////////////////////////////////////////////////////////////
int CVideoHardwareInterface::TranslateSiteNameToUCS2(const char* ascii_buffer, char* ucs2_buffer, WORD ucs2_buffer_size)
{
	WORD ascii_buffer_len = strlen(ascii_buffer);
	if (ascii_buffer_len > MAX_SITE_NAME_ARR_SIZE)
	{
		PASSERT(ascii_buffer_len);
		return ascii_buffer_len;
	}

	return TranslateCaptionNameToUCS2(ascii_buffer, ucs2_buffer, ucs2_buffer_size);
}

////////////////////////////////////////////////////////////////////////////
int CVideoHardwareInterface::TranslateMessageOverlayToUCS2(const char* ascii_buffer, char* ucs2_buffer, WORD ucs2_buffer_size)
{
	WORD ascii_buffer_len = strlen(ascii_buffer);
	if (ascii_buffer_len > ucs2_buffer_size)
	{
		PASSERT(ascii_buffer_len);
		return ascii_buffer_len;
	}

	return TranslateCaptionNameToUCS2(ascii_buffer, ucs2_buffer, ucs2_buffer_size);
}

////////////////////////////////////////////////////////////////////////////
int CVideoHardwareInterface::TranslateCaptionNameToUCS2(const char* ascii_buffer, char* ucs2_buffer, WORD ucs2_buffer_size)
{
	WORD ascii_buffer_len  = strlen(ascii_buffer);
	WORD  temp_buffer_size = ascii_buffer_len*2;
	char* temp_buffer      = new char[temp_buffer_size];

	memset(ucs2_buffer, 0, ucs2_buffer_size);
	memset(temp_buffer, 0, temp_buffer_size);

	WORD ucs2_buffer_len = ConvertStingEncoding(ascii_buffer, ascii_buffer_len, temp_buffer, temp_buffer_size);
	if (ucs2_buffer_len > 0)
	{
		ucs2_buffer_len = min(ucs2_buffer_len, ucs2_buffer_size);
		memcpy(ucs2_buffer, temp_buffer, ucs2_buffer_len);
	}

	delete[] temp_buffer;
	return ucs2_buffer_len;
}

////////////////////////////////////////////////////////////////////////////
EVideoQualityType CVideoHardwareInterface::TranslateVideoQualityToApi(eVideoQuality videoQuality, eVideoConfType videoConfType)
{
	if (videoConfType == eVideoConfTypeCP) // the video quality is relevant to CP conferences only
	{
		switch (videoQuality)
		{
			case eVideoQualityAuto     : return E_VIDEO_QUALITY_SHARPNESS;
			case eVideoQualitySharpness: return E_VIDEO_QUALITY_SHARPNESS;
			case eVideoQualityMotion   : return E_VIDEO_QUALITY_MOTION;

			default:
				FPASSERT(videoQuality+1000);
				break;
		}

	}

	return E_VIDEO_QUALITY_DUMMY;
}

////////////////////////////////////////////////////////////////////////////
ETextDisplaySpeed CVideoHardwareInterface::TranslateMessageOverlayDisplaySpeedToApi(eMessageOverlaySpeedType speedType)
{
	switch (speedType)
	{
		case eSlow  : return E_TEXT_DISPLAY_SPEED_SLOW;
		case eFast  : return E_TEXT_DISPLAY_SPEED_FAST;
		case eStatic: return E_TEXT_DISPLAY_SPEED_STATIC;

		default:
			FPASSERT(speedType+1000);
			return E_TEXT_DISPLAY_SPEED_DUMMY;
	}
}

////////////////////////////////////////////////////////////////////////////
CVideoHardwareInterface::FontColor CVideoHardwareInterface::CVideoHardwareInterface::TranslateTextColorTypeToApi(eTextColorType colorType)
{
	FontColor apiColor;
	apiColor.nTextColor       = 0xFF0000;
	apiColor.nBackgroundColor = 0xFFFFFF;

	switch (colorType)
	{
		case eRed_font        : apiColor.nTextColor = 0xFF0000; break;
		case eBlue_font       : apiColor.nTextColor = 0x0026FF; break;
		case eSkyBlue_font    : apiColor.nTextColor = 0X00BDF4; break;
		case eGreen_font      : apiColor.nTextColor = 0X007F0E; break;
		case eYellowGreen_font: apiColor.nTextColor = 0X00FF21; break;
		case eLightYellow_font: apiColor.nTextColor = 0XFAFFAA; break;
		case eYellow_font     : apiColor.nTextColor = 0XFFFA1B; break;
		case eBlack_font      : apiColor.nTextColor = 0X000000; break;
		case eWhite_font      : apiColor.nTextColor = 0XFFFFFF; break;
		default:
		{
			PASSERTSTREAM(1, "colorType:" << (DWORD)colorType << " - Failed, unknown color type");
		}
	}
	return apiColor;
}

////////////////////////////////////////////////////////////////////////////
CVideoHardwareInterface::FontColor CVideoHardwareInterface::TranslateMessageOverlayColorTypeToApi(eMessageOverlayColorType colorType)
	{
	FontColor apiColor;
	apiColor.nTextColor       = 0x000000;
	apiColor.nBackgroundColor = 0xFFFFFF;

	switch (colorType)
	{
		case eWhite_font_on_light_blue_background: { apiColor.nTextColor = 0xFFFFFF; apiColor.nBackgroundColor = 0xADD8E6; break; }
		case eWhite_font_on_black_background     : { apiColor.nTextColor = 0xFFFFFF; apiColor.nBackgroundColor = 0x000000; break; }
		case eWhite_font_on_gray_background      : { apiColor.nTextColor = 0xFFFFFF; apiColor.nBackgroundColor = 0x808080; break; }
		case eWhite_font_on_red_background       : { apiColor.nTextColor = 0xFFFFFF; apiColor.nBackgroundColor = 0xFF0000; break; }
		case eWhite_font_on_orange_background    : { apiColor.nTextColor = 0xFFFFFF; apiColor.nBackgroundColor = 0xFFA500; break; }
		case eWhite_font_on_blue_background      : { apiColor.nTextColor = 0xFFFFFF; apiColor.nBackgroundColor = 0x000080; break; }
		case eWhite_font_on_olive_background     : { apiColor.nTextColor = 0xFFFFFF; apiColor.nBackgroundColor = 0x808000; break; }
		case eWhite_font_on_green_background     : { apiColor.nTextColor = 0xFFFFFF; apiColor.nBackgroundColor = 0x008000; break; }
		case eWhite_font_on_purple_background    : { apiColor.nTextColor = 0xFFFFFF; apiColor.nBackgroundColor = 0x800080; break; }
		case eRed_font_on_white_background       : { apiColor.nTextColor = 0xFF0000; apiColor.nBackgroundColor = 0xFFFFFF; break; }
		case eWhite_font_on_deep_brown_background: { apiColor.nTextColor = 0xFFFFFF; apiColor.nBackgroundColor = 0x800000; break; }
		case eWhite_font_on_brown_background     : { apiColor.nTextColor = 0xFFFFFF; apiColor.nBackgroundColor = 0x9A7057; break; }
		case eYellow_font_on_black_background    : { apiColor.nTextColor = 0xFFFF00; apiColor.nBackgroundColor = 0x000000; break; }
		case eYellow_font_on_deep_blue_background: { apiColor.nTextColor = 0xFFFF00; apiColor.nBackgroundColor = 0x000080; break; }
		case eLightBlue_font_on_black_background : { apiColor.nTextColor = 0x00FFFF; apiColor.nBackgroundColor = 0x000000; break; }
		case eBlue_font_on_white_background      : { apiColor.nTextColor = 0x0000FF; apiColor.nBackgroundColor = 0xFFFFFF; break; }
		case eGreen_font_on_black_background     : { apiColor.nTextColor = 0x00FF00; apiColor.nBackgroundColor = 0x000000; break; }
		case eGreyGreen_font_on_white_background : { apiColor.nTextColor = 0x008000; apiColor.nBackgroundColor = 0xFFFFFF; break; }
		case eBlack_font_on_gray_background      : { apiColor.nTextColor = 0x000000; apiColor.nBackgroundColor = 0xC0C0C0; break; }
		case eBlack_font_on_white_background     : { apiColor.nTextColor = 0x000000; apiColor.nBackgroundColor = 0xFFFFFF; break; }
		default:
		{
			PASSERTSTREAM(1, "colorType:" << (DWORD)colorType << " - Failed, unknown color type");
		}
	}
	return apiColor;
}

////////////////////////////////////////////////////////////////////////////
size_t CVideoHardwareInterface::GetMTUSize(bool isLprOn, bool bIsTipMode/* = false*/)
{
	const char* key = isLprOn ? "MTU_SIZE_DURING_LPR" : "MTU_SIZE";

	size_t mtuSize = GetSystemCfgFlag<size_t>(key);

	if (mtuSize < 400 || mtuSize > 1440)  // the valid MTU size is between 400 and 1440 include
	{
		FTRACEINTO << "mtuSize:" << mtuSize << " - Invalid MTU_SIZE from system.cfg, we will send the default MTU_SIZE";
		mtuSize = 1440;
	}

	return mtuSize;
}

////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::SendMessageOverlayToDisplay(CSegment* pParam)
{
	CMessageOverlayInfo* pMessageOverlayInfo = new CMessageOverlayInfo;

	pMessageOverlayInfo->DeSerialize(NATIVE, *pParam);
	pMessageOverlayInfo->Dump("CVideoHardwareInterface::SendMessageOverlayToDisplay");

	TEXT_BOX_DISPLAY_S tTextBoxDisplayStruct;
	memset(&tTextBoxDisplayStruct, 0xff, sizeof(TEXT_BOX_DISPLAY_S));

	tTextBoxDisplayStruct.tTextBoxParams.nPosition = pMessageOverlayInfo->GetDisplayPositionType();

	tTextBoxDisplayStruct.tTextBoxParams.nNumberOfTextLines               = 1;
	tTextBoxDisplayStruct.tTextBoxParams.nPresentationMode                = E_TEXT_BOX_PRESENTATION_MODE_STATIC;
	tTextBoxDisplayStruct.tTextBoxParams.tTextBoxSizeLocation.nUpperLeftX = -1;
	tTextBoxDisplayStruct.tTextBoxParams.tTextBoxSizeLocation.nUpperLeftY = -1;
	tTextBoxDisplayStruct.tTextBoxParams.tTextBoxSizeLocation.nWidth      = -1;
	tTextBoxDisplayStruct.tTextBoxParams.tTextBoxSizeLocation.nHeight     = -1;
	tTextBoxDisplayStruct.tTextBoxParams.nTextBoxType                     = E_TEXT_BOX_TYPE_MESSAGE_OVERLAY;
	tTextBoxDisplayStruct.tTextBoxParams.nDisplaySpeed                    = TranslateMessageOverlayDisplaySpeedToApi((eMessageOverlaySpeedType)pMessageOverlayInfo->GetDisplaySpeedType());
	tTextBoxDisplayStruct.tTextBoxParams.nMessageRepetitionCount          = pMessageOverlayInfo->GetNumOfRepetitions();
	tTextBoxDisplayStruct.atTextLineParams[0].acTextLine[0]               = '\0';
	tTextBoxDisplayStruct.atTextLineParams[0].acTextLine[1]               = '\0';

	TranslateMessageOverlayToUCS2((char*)(pMessageOverlayInfo->GetMessageText()).c_str(), tTextBoxDisplayStruct.atTextLineParams[0].acTextLine, MAX_MESSAGE_OVERLAY_STRING_LENGTH);

	FontColor font_color = TranslateMessageOverlayColorTypeToApi((eMessageOverlayColorType)pMessageOverlayInfo->GetColorType());
	tTextBoxDisplayStruct.atTextLineParams[0].nTextColor       = font_color.nTextColor;
	tTextBoxDisplayStruct.atTextLineParams[0].nBackgroundColor = font_color.nBackgroundColor;

	tTextBoxDisplayStruct.atTextLineParams[0].nTransparency = pMessageOverlayInfo->GetTransparency();

	tTextBoxDisplayStruct.atTextLineParams[0].nShadowWidth = 0;
	tTextBoxDisplayStruct.atTextLineParams[0].nAlignment   = E_TEXT_ALIGNMENT_CENTER;
	tTextBoxDisplayStruct.atTextLineParams[0].nFontSize = pMessageOverlayInfo->GetFontSize();

	if (pMessageOverlayInfo->GetMessageOn())
	{
		CSegment* pMsg = new CSegment;
		pMsg->Put((BYTE*)(&tTextBoxDisplayStruct), sizeof(TEXT_BOX_PARAMS_S) + sizeof(TEXT_LINE_PARAMS_S));
		SendMsgToMPL(VIDEO_GRAPHICS_SHOW_TEXT_BOX_REQ, pMsg);
		POBJDELETE(pMsg);
	}
	else
		SendMsgToMPL(VIDEO_GRAPHICS_HIDE_TEXT_BOX_REQ, NULL);

	POBJDELETE(pMessageOverlayInfo);
}

////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::SendTextToDisplay(CTextOnScreenMngr& pTextMsgList)
{
	DWORD TextLen = MIN_(pTextMsgList.GetNumOfTextLines(), MAX_TEXT_LINES_IN_MESSAGE);

	TEXT_BOX_DISPLAY_S tTextBoxDisplayStruct;
	memset(&tTextBoxDisplayStruct, 0xff, sizeof(TEXT_BOX_DISPLAY_S));

	tTextBoxDisplayStruct.tTextBoxParams.nNumberOfTextLines               = TextLen;
	tTextBoxDisplayStruct.tTextBoxParams.nPresentationMode                = E_TEXT_BOX_PRESENTATION_MODE_STATIC;
	tTextBoxDisplayStruct.tTextBoxParams.nPosition                        = E_TEXT_BOX_POSITION_UP_LEFT; // E_TEXT_BOX_POSITION_MIDDLE_CENTER;
	tTextBoxDisplayStruct.tTextBoxParams.tTextBoxSizeLocation.nUpperLeftX = -1;
	tTextBoxDisplayStruct.tTextBoxParams.tTextBoxSizeLocation.nUpperLeftY = -1;
	tTextBoxDisplayStruct.tTextBoxParams.tTextBoxSizeLocation.nHeight     = 0;
	tTextBoxDisplayStruct.tTextBoxParams.tTextBoxSizeLocation.nWidth      = 0;
	tTextBoxDisplayStruct.tTextBoxParams.nTextBoxType                     = E_TEXT_BOX_TYPE_IN_CELL;
	tTextBoxDisplayStruct.tTextBoxParams.nPosition                        = E_TEXT_BOX_POSITION_MIDDLE_CENTER;
	tTextBoxDisplayStruct.tTextBoxParams.nDisplaySpeed                    = E_TEXT_DISPLAY_SPEED_SLOW;
	tTextBoxDisplayStruct.tTextBoxParams.nMessageRepetitionCount          = 1;

	CTextOnScreenMsg* CurrMsgLine = pTextMsgList.GetFirst();

	for (DWORD i = 0; i < TextLen; i++)
	{
		if (CurrMsgLine)
		{
			TRACEINTO << "MsgNumber:" << i << ", Text:" << CurrMsgLine->GetTextLine();

			// site name send to card decoded in UCS-2BE, 16 bit encoding, null terminating = 00
			tTextBoxDisplayStruct.atTextLineParams[i].acTextLine[0] = '\0';
			tTextBoxDisplayStruct.atTextLineParams[i].acTextLine[1] = '\0';
			TranslateSiteNameToUCS2(CurrMsgLine->GetTextLine(), tTextBoxDisplayStruct.atTextLineParams[i].acTextLine, MAX_SITE_NAME_SIZE);
			tTextBoxDisplayStruct.atTextLineParams[i].nTextColor       = (DWORD)(CurrMsgLine->GetTextColor());
			tTextBoxDisplayStruct.atTextLineParams[i].nBackgroundColor = (DWORD)(CurrMsgLine->GetBackgroundColor());
			tTextBoxDisplayStruct.atTextLineParams[i].nTransparency    = (DWORD)(CurrMsgLine->GetTransparency());
			tTextBoxDisplayStruct.atTextLineParams[i].nShadowWidth     = (DWORD)(CurrMsgLine->GetShadowWidth());
			tTextBoxDisplayStruct.atTextLineParams[i].nAlignment       = (DWORD)(CurrMsgLine->GetAlignment());
			tTextBoxDisplayStruct.atTextLineParams[i].nFontType        = (DWORD)(CurrMsgLine->GetFontType());
			tTextBoxDisplayStruct.atTextLineParams[i].nFontSize        = (DWORD)(E_TEXT_FONT_SIZE_MEDIUM);
		}

		CurrMsgLine = pTextMsgList.GetNext();
	}

	CSegment* pMsg = new CSegment;
	pMsg->Put((BYTE*)(&tTextBoxDisplayStruct), sizeof(TEXT_BOX_PARAMS_S) + TextLen*sizeof(TEXT_LINE_PARAMS_S));
	SendMsgToMPL(VIDEO_GRAPHICS_SHOW_TEXT_BOX_REQ, pMsg);
	POBJDELETE(pMsg);
}

////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::StopGathering()
{
	SendMsgToMPL(VIDEO_GRAPHICS_STOP_GATHERING_REQ, NULL);
}

////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::SendGatheringToDisplay(CGathering* pGathering, const char* pszPartyName)
{
	if (pGathering->IsEndGathering() || pGathering->IsHideGatheringText(pszPartyName))
	{
		SendMsgToMPL(VIDEO_GRAPHICS_STOP_GATHERING_REQ, NULL);
		return;
	}

	int bFullRendering = pGathering->IsNeedFullRendering(pszPartyName) ? 1 : 0;
	int SHIFT_Y        = 40;
	int iShiftX        = pGathering->GetShiftX();

	TEXT_BOX_LAYOUT_S stGathering;
	memset(&stGathering, 0, sizeof(TEXT_BOX_LAYOUT_S));

	stGathering.tTextBoxSizeLocation.nUpperLeftX = iShiftX + 28;
	stGathering.tTextBoxSizeLocation.nUpperLeftY = 28;
	stGathering.tTextBoxSizeLocation.nWidth      = 900;
	stGathering.tTextBoxSizeLocation.nHeight     = 664;

	stGathering.nNumOfBoxes             = 8;
	stGathering.nBackgroundTransparency = pGathering->GetBackGroundTransparency();
	stGathering.nBackgroundColor        = COLOR_YUV_BLACK;

	stGathering.atTextBoxDisplay[0].tTextBoxParams.bIsNewData                       = bFullRendering; // 1;
	stGathering.atTextBoxDisplay[0].tTextBoxParams.nNumberOfTextLines               = 1;
	stGathering.atTextBoxDisplay[0].tTextBoxParams.nPosition                        = 0;
	stGathering.atTextBoxDisplay[0].tTextBoxParams.nPresentationMode                = E_TEXT_BOX_PRESENTATION_MODE_STATIC;
	stGathering.atTextBoxDisplay[0].tTextBoxParams.tTextBoxSizeLocation.nUpperLeftX = iShiftX + 48;
	stGathering.atTextBoxDisplay[0].tTextBoxParams.tTextBoxSizeLocation.nUpperLeftY = 58 + SHIFT_Y;
	stGathering.atTextBoxDisplay[0].tTextBoxParams.tTextBoxSizeLocation.nWidth      = 120;
	stGathering.atTextBoxDisplay[0].tTextBoxParams.tTextBoxSizeLocation.nHeight     = 48;
	stGathering.atTextBoxDisplay[0].atTextLineParams[0].nAlignment                  = E_TEXT_ALIGNMENT_LEFT;
	stGathering.atTextBoxDisplay[0].atTextLineParams[0].nBackgroundColor            = 0;
	stGathering.atTextBoxDisplay[0].atTextLineParams[0].nFontSize                   = E_TEXT_FONT_SIZE_MEDIUM;
	stGathering.atTextBoxDisplay[0].atTextLineParams[0].nFontType                   = TEXT_FONT_TYPE_NULL;
	stGathering.atTextBoxDisplay[0].atTextLineParams[0].nShadowWidth                = 0;
	stGathering.atTextBoxDisplay[0].atTextLineParams[0].nTextColor                  = COLOR_YUV_WHITE;
	stGathering.atTextBoxDisplay[0].atTextLineParams[0].nTransparency               = 100;

	stGathering.atTextBoxDisplay[1].tTextBoxParams.bIsNewData                       = bFullRendering;
	stGathering.atTextBoxDisplay[1].tTextBoxParams.nNumberOfTextLines               = 1;
	stGathering.atTextBoxDisplay[1].tTextBoxParams.nPosition                        = 0;
	stGathering.atTextBoxDisplay[1].tTextBoxParams.nPresentationMode                = E_TEXT_BOX_PRESENTATION_MODE_STATIC;
	stGathering.atTextBoxDisplay[1].tTextBoxParams.tTextBoxSizeLocation.nUpperLeftX = iShiftX + 28;
	stGathering.atTextBoxDisplay[1].tTextBoxParams.tTextBoxSizeLocation.nUpperLeftY = 58 + SHIFT_Y;
	stGathering.atTextBoxDisplay[1].tTextBoxParams.tTextBoxSizeLocation.nWidth      = 880;
	stGathering.atTextBoxDisplay[1].tTextBoxParams.tTextBoxSizeLocation.nHeight     = 96;
	pGathering->GetTitleStr(stGathering.atTextBoxDisplay[1].atTextLineParams[0].acTextLine, MAX_SITE_NAME_SIZE);
	stGathering.atTextBoxDisplay[1].atTextLineParams[0].nAlignment                  = E_TEXT_ALIGNMENT_CENTER;
	stGathering.atTextBoxDisplay[1].atTextLineParams[0].nBackgroundColor            = 0;
	stGathering.atTextBoxDisplay[1].atTextLineParams[0].nFontSize                   = E_TEXT_FONT_SIZE_LARGE;
	stGathering.atTextBoxDisplay[1].atTextLineParams[0].nFontType                   = TEXT_FONT_TYPE_NULL;
	stGathering.atTextBoxDisplay[1].atTextLineParams[0].nShadowWidth                = 0;
	stGathering.atTextBoxDisplay[1].atTextLineParams[0].nTextColor                  = COLOR_YUV_WHITE;
	stGathering.atTextBoxDisplay[1].atTextLineParams[0].nTransparency               = 100;

	stGathering.atTextBoxDisplay[2].tTextBoxParams.bIsNewData                       = bFullRendering;
	stGathering.atTextBoxDisplay[2].tTextBoxParams.nNumberOfTextLines               = 1;
	stGathering.atTextBoxDisplay[2].tTextBoxParams.nPosition                        = 0;
	stGathering.atTextBoxDisplay[2].tTextBoxParams.nPresentationMode                = E_TEXT_BOX_PRESENTATION_MODE_STATIC;
	stGathering.atTextBoxDisplay[2].tTextBoxParams.tTextBoxSizeLocation.nUpperLeftX = iShiftX + 28;
	stGathering.atTextBoxDisplay[2].tTextBoxParams.tTextBoxSizeLocation.nUpperLeftY = 122 + SHIFT_Y;
	stGathering.atTextBoxDisplay[2].tTextBoxParams.tTextBoxSizeLocation.nWidth      = 900;
	stGathering.atTextBoxDisplay[2].tTextBoxParams.tTextBoxSizeLocation.nHeight     = 46;
	pGathering->GetOrganizerStr(stGathering.atTextBoxDisplay[2].atTextLineParams[0].acTextLine, MAX_SITE_NAME_SIZE);
	stGathering.atTextBoxDisplay[2].atTextLineParams[0].nAlignment                  = E_TEXT_ALIGNMENT_CENTER;
	stGathering.atTextBoxDisplay[2].atTextLineParams[0].nBackgroundColor            = 0;
	stGathering.atTextBoxDisplay[2].atTextLineParams[0].nFontSize                   = E_TEXT_FONT_SIZE_MEDIUM;
	stGathering.atTextBoxDisplay[2].atTextLineParams[0].nFontType                   = TEXT_FONT_TYPE_NULL;
	stGathering.atTextBoxDisplay[2].atTextLineParams[0].nShadowWidth                = 0;
	stGathering.atTextBoxDisplay[2].atTextLineParams[0].nTextColor                  = COLOR_YUV_WHITE;
	stGathering.atTextBoxDisplay[2].atTextLineParams[0].nTransparency               = 100;

	stGathering.atTextBoxDisplay[3].tTextBoxParams.bIsNewData                       = bFullRendering;
	stGathering.atTextBoxDisplay[3].tTextBoxParams.nNumberOfTextLines               = 1;
	stGathering.atTextBoxDisplay[3].tTextBoxParams.nPosition                        = 0;
	stGathering.atTextBoxDisplay[3].tTextBoxParams.nPresentationMode                = E_TEXT_BOX_PRESENTATION_MODE_STATIC;
	stGathering.atTextBoxDisplay[3].tTextBoxParams.tTextBoxSizeLocation.nUpperLeftX = iShiftX + 28;
	stGathering.atTextBoxDisplay[3].tTextBoxParams.tTextBoxSizeLocation.nUpperLeftY = 158 + SHIFT_Y;
	stGathering.atTextBoxDisplay[3].tTextBoxParams.tTextBoxSizeLocation.nWidth      = 900;
	stGathering.atTextBoxDisplay[3].tTextBoxParams.tTextBoxSizeLocation.nHeight     = 46;
	pGathering->GetTimesStr(stGathering.atTextBoxDisplay[3].atTextLineParams[0].acTextLine, MAX_SITE_NAME_SIZE);
	stGathering.atTextBoxDisplay[3].atTextLineParams[0].nAlignment                  = E_TEXT_ALIGNMENT_CENTER;
	stGathering.atTextBoxDisplay[3].atTextLineParams[0].nBackgroundColor            = 0;
	stGathering.atTextBoxDisplay[3].atTextLineParams[0].nFontSize                   = E_TEXT_FONT_SIZE_MEDIUM;
	stGathering.atTextBoxDisplay[3].atTextLineParams[0].nFontType                   = TEXT_FONT_TYPE_NULL;
	stGathering.atTextBoxDisplay[3].atTextLineParams[0].nShadowWidth                = 0;
	stGathering.atTextBoxDisplay[3].atTextLineParams[0].nTextColor                  = 0xFFEB8080;
	stGathering.atTextBoxDisplay[3].atTextLineParams[0].nTransparency               = 100;

	stGathering.atTextBoxDisplay[4].tTextBoxParams.bIsNewData                       = bFullRendering;
	stGathering.atTextBoxDisplay[4].tTextBoxParams.nNumberOfTextLines               = 1;
	stGathering.atTextBoxDisplay[4].tTextBoxParams.nPosition                        = 0;
	stGathering.atTextBoxDisplay[4].tTextBoxParams.nPresentationMode                = E_TEXT_BOX_PRESENTATION_MODE_STATIC;
	stGathering.atTextBoxDisplay[4].tTextBoxParams.tTextBoxSizeLocation.nUpperLeftX = iShiftX + 28;
	stGathering.atTextBoxDisplay[4].tTextBoxParams.tTextBoxSizeLocation.nUpperLeftY = 214 + SHIFT_Y;
	stGathering.atTextBoxDisplay[4].tTextBoxParams.tTextBoxSizeLocation.nWidth      = 900;
	stGathering.atTextBoxDisplay[4].tTextBoxParams.tTextBoxSizeLocation.nHeight     = 46;
	pGathering->GetFreeText1Str(stGathering.atTextBoxDisplay[4].atTextLineParams[0].acTextLine, MAX_SITE_NAME_SIZE);
	stGathering.atTextBoxDisplay[4].atTextLineParams[0].nAlignment                  = E_TEXT_ALIGNMENT_CENTER;
	stGathering.atTextBoxDisplay[4].atTextLineParams[0].nBackgroundColor            = 0;
	stGathering.atTextBoxDisplay[4].atTextLineParams[0].nFontSize                   = E_TEXT_FONT_SIZE_MEDIUM;
	stGathering.atTextBoxDisplay[4].atTextLineParams[0].nFontType                   = TEXT_FONT_TYPE_NULL;
	stGathering.atTextBoxDisplay[4].atTextLineParams[0].nShadowWidth                = 0;
	stGathering.atTextBoxDisplay[4].atTextLineParams[0].nTextColor                  = COLOR_YUV_ORANGE;
	stGathering.atTextBoxDisplay[4].atTextLineParams[0].nTransparency               = 100;

	stGathering.atTextBoxDisplay[5].tTextBoxParams.bIsNewData                       = 1;
	stGathering.atTextBoxDisplay[5].tTextBoxParams.nNumberOfTextLines               = 8;
	stGathering.atTextBoxDisplay[5].tTextBoxParams.nPosition                        = 0;
	stGathering.atTextBoxDisplay[5].tTextBoxParams.nPresentationMode                = E_TEXT_BOX_PRESENTATION_MODE_STATIC;
	stGathering.atTextBoxDisplay[5].tTextBoxParams.tTextBoxSizeLocation.nUpperLeftX = iShiftX + 238;
	stGathering.atTextBoxDisplay[5].tTextBoxParams.tTextBoxSizeLocation.nUpperLeftY = 272 + SHIFT_Y;
	stGathering.atTextBoxDisplay[5].tTextBoxParams.tTextBoxSizeLocation.nWidth      = 306;
	stGathering.atTextBoxDisplay[5].tTextBoxParams.tTextBoxSizeLocation.nHeight     = 246;
	pGathering->GetParticipantStr(0, stGathering.atTextBoxDisplay[5].atTextLineParams[0].acTextLine, MAX_SITE_NAME_SIZE);
	stGathering.atTextBoxDisplay[5].atTextLineParams[0].nAlignment                  = E_TEXT_ALIGNMENT_LEFT;
	stGathering.atTextBoxDisplay[5].atTextLineParams[0].nBackgroundColor            = 0;
	stGathering.atTextBoxDisplay[5].atTextLineParams[0].nFontSize                   = E_TEXT_FONT_SIZE_SMALL;
	stGathering.atTextBoxDisplay[5].atTextLineParams[0].nFontType                   = TEXT_FONT_TYPE_NULL;
	stGathering.atTextBoxDisplay[5].atTextLineParams[0].nShadowWidth                = 0;
	stGathering.atTextBoxDisplay[5].atTextLineParams[0].nTextColor                  = COLOR_YUV_WHITE;
	stGathering.atTextBoxDisplay[5].atTextLineParams[0].nTransparency               = 100;
	pGathering->GetParticipantStr(1, stGathering.atTextBoxDisplay[5].atTextLineParams[1].acTextLine, MAX_SITE_NAME_SIZE);
	stGathering.atTextBoxDisplay[5].atTextLineParams[1].nAlignment                  = E_TEXT_ALIGNMENT_LEFT;
	stGathering.atTextBoxDisplay[5].atTextLineParams[1].nBackgroundColor            = 0;
	stGathering.atTextBoxDisplay[5].atTextLineParams[1].nFontSize                   = E_TEXT_FONT_SIZE_SMALL;
	stGathering.atTextBoxDisplay[5].atTextLineParams[1].nFontType                   = TEXT_FONT_TYPE_NULL;
	stGathering.atTextBoxDisplay[5].atTextLineParams[1].nShadowWidth                = 0;
	stGathering.atTextBoxDisplay[5].atTextLineParams[1].nTextColor                  = COLOR_YUV_WHITE;
	stGathering.atTextBoxDisplay[5].atTextLineParams[1].nTransparency               = 100;
	pGathering->GetParticipantStr(2, stGathering.atTextBoxDisplay[5].atTextLineParams[2].acTextLine, MAX_SITE_NAME_SIZE);
	stGathering.atTextBoxDisplay[5].atTextLineParams[2].nAlignment                  = E_TEXT_ALIGNMENT_LEFT;
	stGathering.atTextBoxDisplay[5].atTextLineParams[2].nBackgroundColor            = 0;
	stGathering.atTextBoxDisplay[5].atTextLineParams[2].nFontSize                   = E_TEXT_FONT_SIZE_SMALL;
	stGathering.atTextBoxDisplay[5].atTextLineParams[2].nFontType                   = TEXT_FONT_TYPE_NULL;
	stGathering.atTextBoxDisplay[5].atTextLineParams[2].nShadowWidth                = 0;
	stGathering.atTextBoxDisplay[5].atTextLineParams[2].nTextColor                  = COLOR_YUV_WHITE;
	stGathering.atTextBoxDisplay[5].atTextLineParams[2].nTransparency               = 100;
	pGathering->GetParticipantStr(3, stGathering.atTextBoxDisplay[5].atTextLineParams[3].acTextLine, MAX_SITE_NAME_SIZE);
	stGathering.atTextBoxDisplay[5].atTextLineParams[3].nAlignment                  = E_TEXT_ALIGNMENT_LEFT;
	stGathering.atTextBoxDisplay[5].atTextLineParams[3].nBackgroundColor            = 0;
	stGathering.atTextBoxDisplay[5].atTextLineParams[3].nFontSize                   = E_TEXT_FONT_SIZE_SMALL;
	stGathering.atTextBoxDisplay[5].atTextLineParams[3].nFontType                   = TEXT_FONT_TYPE_NULL;
	stGathering.atTextBoxDisplay[5].atTextLineParams[3].nShadowWidth                = 0;
	stGathering.atTextBoxDisplay[5].atTextLineParams[3].nTextColor                  = COLOR_YUV_WHITE;
	stGathering.atTextBoxDisplay[5].atTextLineParams[3].nTransparency               = 100;
	pGathering->GetParticipantStr(4, stGathering.atTextBoxDisplay[5].atTextLineParams[4].acTextLine, MAX_SITE_NAME_SIZE);
	stGathering.atTextBoxDisplay[5].atTextLineParams[4].nAlignment                  = E_TEXT_ALIGNMENT_LEFT;
	stGathering.atTextBoxDisplay[5].atTextLineParams[4].nBackgroundColor            = 0;
	stGathering.atTextBoxDisplay[5].atTextLineParams[4].nFontSize                   = E_TEXT_FONT_SIZE_SMALL;
	stGathering.atTextBoxDisplay[5].atTextLineParams[4].nFontType                   = TEXT_FONT_TYPE_NULL;
	stGathering.atTextBoxDisplay[5].atTextLineParams[4].nShadowWidth                = 0;
	stGathering.atTextBoxDisplay[5].atTextLineParams[4].nTextColor                  = COLOR_YUV_WHITE;
	stGathering.atTextBoxDisplay[5].atTextLineParams[4].nTransparency               = 100;
	pGathering->GetParticipantStr(5, stGathering.atTextBoxDisplay[5].atTextLineParams[5].acTextLine, MAX_SITE_NAME_SIZE);
	stGathering.atTextBoxDisplay[5].atTextLineParams[5].nAlignment                  = E_TEXT_ALIGNMENT_LEFT;
	stGathering.atTextBoxDisplay[5].atTextLineParams[5].nBackgroundColor            = 0;
	stGathering.atTextBoxDisplay[5].atTextLineParams[5].nFontSize                   = E_TEXT_FONT_SIZE_SMALL;
	stGathering.atTextBoxDisplay[5].atTextLineParams[5].nFontType                   = TEXT_FONT_TYPE_NULL;
	stGathering.atTextBoxDisplay[5].atTextLineParams[5].nShadowWidth                = 0;
	stGathering.atTextBoxDisplay[5].atTextLineParams[5].nTextColor                  = COLOR_YUV_WHITE;
	stGathering.atTextBoxDisplay[5].atTextLineParams[5].nTransparency               = 100;
	pGathering->GetParticipantStr(6, stGathering.atTextBoxDisplay[5].atTextLineParams[6].acTextLine, MAX_SITE_NAME_SIZE);
	stGathering.atTextBoxDisplay[5].atTextLineParams[6].nAlignment                  = E_TEXT_ALIGNMENT_LEFT;
	stGathering.atTextBoxDisplay[5].atTextLineParams[6].nBackgroundColor            = 0;
	stGathering.atTextBoxDisplay[5].atTextLineParams[6].nFontSize                   = E_TEXT_FONT_SIZE_SMALL;
	stGathering.atTextBoxDisplay[5].atTextLineParams[6].nFontType                   = TEXT_FONT_TYPE_NULL;
	stGathering.atTextBoxDisplay[5].atTextLineParams[6].nShadowWidth                = 0;
	stGathering.atTextBoxDisplay[5].atTextLineParams[6].nTextColor                  = COLOR_YUV_WHITE;
	stGathering.atTextBoxDisplay[5].atTextLineParams[6].nTransparency               = 100;
	pGathering->GetParticipantStr(7, stGathering.atTextBoxDisplay[5].atTextLineParams[7].acTextLine, MAX_SITE_NAME_SIZE);
	stGathering.atTextBoxDisplay[5].atTextLineParams[7].nAlignment                  = E_TEXT_ALIGNMENT_LEFT;
	stGathering.atTextBoxDisplay[5].atTextLineParams[7].nBackgroundColor            = 0;
	stGathering.atTextBoxDisplay[5].atTextLineParams[7].nFontSize                   = E_TEXT_FONT_SIZE_SMALL;
	stGathering.atTextBoxDisplay[5].atTextLineParams[7].nFontType                   = TEXT_FONT_TYPE_NULL;
	stGathering.atTextBoxDisplay[5].atTextLineParams[7].nShadowWidth                = 0;
	stGathering.atTextBoxDisplay[5].atTextLineParams[7].nTextColor                  = COLOR_YUV_WHITE;
	stGathering.atTextBoxDisplay[5].atTextLineParams[7].nTransparency               = 100;

	stGathering.atTextBoxDisplay[6].tTextBoxParams.bIsNewData                       = 1;
	stGathering.atTextBoxDisplay[6].tTextBoxParams.nNumberOfTextLines               = 8;
	stGathering.atTextBoxDisplay[6].tTextBoxParams.nPosition                        = 0;
	stGathering.atTextBoxDisplay[6].tTextBoxParams.nPresentationMode                = E_TEXT_BOX_PRESENTATION_MODE_STATIC;
	stGathering.atTextBoxDisplay[6].tTextBoxParams.tTextBoxSizeLocation.nUpperLeftX = iShiftX + 542;
	stGathering.atTextBoxDisplay[6].tTextBoxParams.tTextBoxSizeLocation.nUpperLeftY = 272 + SHIFT_Y;
	stGathering.atTextBoxDisplay[6].tTextBoxParams.tTextBoxSizeLocation.nWidth      = 400;
	stGathering.atTextBoxDisplay[6].tTextBoxParams.tTextBoxSizeLocation.nHeight     = 246;
	pGathering->GetVideoPartsStr(stGathering.atTextBoxDisplay[6].atTextLineParams[0].acTextLine, MAX_SITE_NAME_SIZE);
	stGathering.atTextBoxDisplay[6].atTextLineParams[0].nAlignment                  = E_TEXT_ALIGNMENT_LEFT;
	stGathering.atTextBoxDisplay[6].atTextLineParams[0].nBackgroundColor            = 0;
	stGathering.atTextBoxDisplay[6].atTextLineParams[0].nFontSize                   = E_TEXT_FONT_SIZE_SMALL;
	stGathering.atTextBoxDisplay[6].atTextLineParams[0].nFontType                   = TEXT_FONT_TYPE_NULL;
	stGathering.atTextBoxDisplay[6].atTextLineParams[0].nShadowWidth                = 0;
	stGathering.atTextBoxDisplay[6].atTextLineParams[0].nTextColor                  = COLOR_YUV_WHITE;
	stGathering.atTextBoxDisplay[6].atTextLineParams[0].nTransparency               = 100;
	pGathering->GetAudioPartsStr(stGathering.atTextBoxDisplay[6].atTextLineParams[1].acTextLine, MAX_SITE_NAME_SIZE);
	stGathering.atTextBoxDisplay[6].atTextLineParams[1].nAlignment                  = E_TEXT_ALIGNMENT_LEFT;
	stGathering.atTextBoxDisplay[6].atTextLineParams[1].nBackgroundColor            = 0;
	stGathering.atTextBoxDisplay[6].atTextLineParams[1].nFontSize                   = E_TEXT_FONT_SIZE_SMALL;
	stGathering.atTextBoxDisplay[6].atTextLineParams[1].nFontType                   = TEXT_FONT_TYPE_NULL;
	stGathering.atTextBoxDisplay[6].atTextLineParams[1].nShadowWidth                = 0;
	stGathering.atTextBoxDisplay[6].atTextLineParams[1].nTextColor                  = COLOR_YUV_WHITE;
	stGathering.atTextBoxDisplay[6].atTextLineParams[1].nTransparency               = 100;
	stGathering.atTextBoxDisplay[6].atTextLineParams[2].acTextLine[0]               = '\0';
	stGathering.atTextBoxDisplay[6].atTextLineParams[2].acTextLine[1]               = '\0';
	stGathering.atTextBoxDisplay[6].atTextLineParams[2].nAlignment                  = E_TEXT_ALIGNMENT_LEFT;
	stGathering.atTextBoxDisplay[6].atTextLineParams[2].nBackgroundColor            = 0;
	stGathering.atTextBoxDisplay[6].atTextLineParams[2].nFontSize                   = E_TEXT_FONT_SIZE_SMALL;
	stGathering.atTextBoxDisplay[6].atTextLineParams[2].nFontType                   = TEXT_FONT_TYPE_NULL;
	stGathering.atTextBoxDisplay[6].atTextLineParams[2].nShadowWidth                = 0;
	stGathering.atTextBoxDisplay[6].atTextLineParams[2].nTextColor                  = COLOR_YUV_WHITE;
	stGathering.atTextBoxDisplay[6].atTextLineParams[2].nTransparency               = 100;
	pGathering->GetAccessNumberStr(stGathering.atTextBoxDisplay[6].atTextLineParams[3].acTextLine, MAX_SITE_NAME_SIZE);
	stGathering.atTextBoxDisplay[6].atTextLineParams[3].nAlignment                  = E_TEXT_ALIGNMENT_LEFT;
	stGathering.atTextBoxDisplay[6].atTextLineParams[3].nBackgroundColor            = 0;
	stGathering.atTextBoxDisplay[6].atTextLineParams[3].nFontSize                   = E_TEXT_FONT_SIZE_SMALL;
	stGathering.atTextBoxDisplay[6].atTextLineParams[3].nFontType                   = TEXT_FONT_TYPE_NULL;
	stGathering.atTextBoxDisplay[6].atTextLineParams[3].nShadowWidth                = 0;
	stGathering.atTextBoxDisplay[6].atTextLineParams[3].nTextColor                  = COLOR_YUV_WHITE;
	stGathering.atTextBoxDisplay[6].atTextLineParams[3].nTransparency               = 100;
	pGathering->GetIPStr(stGathering.atTextBoxDisplay[6].atTextLineParams[4].acTextLine, MAX_SITE_NAME_SIZE);
	stGathering.atTextBoxDisplay[6].atTextLineParams[4].nAlignment                  = E_TEXT_ALIGNMENT_LEFT;
	stGathering.atTextBoxDisplay[6].atTextLineParams[4].nBackgroundColor            = 0;
	stGathering.atTextBoxDisplay[6].atTextLineParams[4].nFontSize                   = E_TEXT_FONT_SIZE_SMALL;
	stGathering.atTextBoxDisplay[6].atTextLineParams[4].nFontType                   = TEXT_FONT_TYPE_NULL;
	stGathering.atTextBoxDisplay[6].atTextLineParams[4].nShadowWidth                = 0;
	stGathering.atTextBoxDisplay[6].atTextLineParams[4].nTextColor                  = COLOR_YUV_WHITE;
	stGathering.atTextBoxDisplay[6].atTextLineParams[4].nTransparency               = 100;
	pGathering->GetISDNStr(stGathering.atTextBoxDisplay[6].atTextLineParams[5].acTextLine, MAX_SITE_NAME_SIZE);
	stGathering.atTextBoxDisplay[6].atTextLineParams[5].nAlignment                  = E_TEXT_ALIGNMENT_LEFT;
	stGathering.atTextBoxDisplay[6].atTextLineParams[5].nBackgroundColor            = 0;
	stGathering.atTextBoxDisplay[6].atTextLineParams[5].nFontSize                   = E_TEXT_FONT_SIZE_SMALL;
	stGathering.atTextBoxDisplay[6].atTextLineParams[5].nFontType                   = TEXT_FONT_TYPE_NULL;
	stGathering.atTextBoxDisplay[6].atTextLineParams[5].nShadowWidth                = 0;
	stGathering.atTextBoxDisplay[6].atTextLineParams[5].nTextColor                  = COLOR_YUV_WHITE;
	stGathering.atTextBoxDisplay[6].atTextLineParams[5].nTransparency               = 100;
	pGathering->GetPSTNStr(stGathering.atTextBoxDisplay[6].atTextLineParams[6].acTextLine, MAX_SITE_NAME_SIZE);
	stGathering.atTextBoxDisplay[6].atTextLineParams[6].nAlignment                  = E_TEXT_ALIGNMENT_LEFT;
	stGathering.atTextBoxDisplay[6].atTextLineParams[6].nBackgroundColor            = 0;
	stGathering.atTextBoxDisplay[6].atTextLineParams[6].nFontSize                   = E_TEXT_FONT_SIZE_SMALL;
	stGathering.atTextBoxDisplay[6].atTextLineParams[6].nFontType                   = TEXT_FONT_TYPE_NULL;
	stGathering.atTextBoxDisplay[6].atTextLineParams[6].nShadowWidth                = 0;
	stGathering.atTextBoxDisplay[6].atTextLineParams[6].nTextColor                  = COLOR_YUV_WHITE;
	stGathering.atTextBoxDisplay[6].atTextLineParams[6].nTransparency               = 100;
	stGathering.atTextBoxDisplay[6].atTextLineParams[7].acTextLine[0]               = '\0';
	stGathering.atTextBoxDisplay[6].atTextLineParams[7].acTextLine[1]               = '\0';
	stGathering.atTextBoxDisplay[6].atTextLineParams[7].nAlignment                  = E_TEXT_ALIGNMENT_LEFT;
	stGathering.atTextBoxDisplay[6].atTextLineParams[7].nBackgroundColor            = 0;
	stGathering.atTextBoxDisplay[6].atTextLineParams[7].nFontSize                   = E_TEXT_FONT_SIZE_SMALL;
	stGathering.atTextBoxDisplay[6].atTextLineParams[7].nFontType                   = TEXT_FONT_TYPE_NULL;
	stGathering.atTextBoxDisplay[6].atTextLineParams[7].nShadowWidth                = 0;
	stGathering.atTextBoxDisplay[6].atTextLineParams[7].nTextColor                  = COLOR_YUV_WHITE;
	stGathering.atTextBoxDisplay[6].atTextLineParams[7].nTransparency               = 100;

	stGathering.atTextBoxDisplay[7].tTextBoxParams.bIsNewData                       = bFullRendering;
	stGathering.atTextBoxDisplay[7].tTextBoxParams.nNumberOfTextLines               = 2;
	stGathering.atTextBoxDisplay[7].tTextBoxParams.nPosition                        = 0;
	stGathering.atTextBoxDisplay[7].tTextBoxParams.nPresentationMode                = E_TEXT_BOX_PRESENTATION_MODE_STATIC;
	stGathering.atTextBoxDisplay[7].tTextBoxParams.tTextBoxSizeLocation.nUpperLeftX = iShiftX + 28;
	stGathering.atTextBoxDisplay[7].tTextBoxParams.tTextBoxSizeLocation.nUpperLeftY = 542 + SHIFT_Y;
	stGathering.atTextBoxDisplay[7].tTextBoxParams.tTextBoxSizeLocation.nWidth      = 900;
	stGathering.atTextBoxDisplay[7].tTextBoxParams.tTextBoxSizeLocation.nHeight     = 80;
	pGathering->GetFreeText2Str(stGathering.atTextBoxDisplay[7].atTextLineParams[0].acTextLine, MAX_SITE_NAME_SIZE);
	stGathering.atTextBoxDisplay[7].atTextLineParams[0].nAlignment                  = E_TEXT_ALIGNMENT_CENTER;
	stGathering.atTextBoxDisplay[7].atTextLineParams[0].nBackgroundColor            = 0;
	stGathering.atTextBoxDisplay[7].atTextLineParams[0].nFontSize                   = E_TEXT_FONT_SIZE_SMALL;
	stGathering.atTextBoxDisplay[7].atTextLineParams[0].nFontType                   = TEXT_FONT_TYPE_NULL;
	stGathering.atTextBoxDisplay[7].atTextLineParams[0].nShadowWidth                = 0;
	stGathering.atTextBoxDisplay[7].atTextLineParams[0].nTextColor                  = COLOR_YUV_WHITE;
	stGathering.atTextBoxDisplay[7].atTextLineParams[0].nTransparency               = 100;
	pGathering->GetFreeText3Str(stGathering.atTextBoxDisplay[7].atTextLineParams[1].acTextLine, MAX_SITE_NAME_SIZE);
	stGathering.atTextBoxDisplay[7].atTextLineParams[1].nAlignment                  = E_TEXT_ALIGNMENT_CENTER;
	stGathering.atTextBoxDisplay[7].atTextLineParams[1].nBackgroundColor            = 0;
	stGathering.atTextBoxDisplay[7].atTextLineParams[1].nFontSize                   = E_TEXT_FONT_SIZE_SMALL;
	stGathering.atTextBoxDisplay[7].atTextLineParams[1].nFontType                   = TEXT_FONT_TYPE_NULL;
	stGathering.atTextBoxDisplay[7].atTextLineParams[1].nShadowWidth                = 0;
	stGathering.atTextBoxDisplay[7].atTextLineParams[1].nTextColor                  = COLOR_YUV_WHITE;
	stGathering.atTextBoxDisplay[7].atTextLineParams[1].nTransparency               = 100;

	CSegment* pMsg = new CSegment;
	pMsg->Put((BYTE*)(&stGathering), sizeof(TEXT_BOX_LAYOUT_S));
	SendMsgToMPL(VIDEO_GRAPHICS_START_GATHERING_REQ, pMsg);

	POBJDELETE(pMsg);
}

////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::SendDisplayRecordingIcon(EIconType eRecordingIcon)
{
	TRACEINTO << "Icon:" << EIconRecordingTypeNames[eRecordingIcon];

	CSegment* pMsg = new CSegment;
	ICON_PARAMS_S tIconParamsStruct = {eRecordingIcon};
	pMsg->Put((BYTE*)&tIconParamsStruct, sizeof(tIconParamsStruct));
	SendMsgToMPL(VIDEO_GRAPHICS_SHOW_ICON_REQ, pMsg);
	POBJDELETE(pMsg);
}

////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::SendUpdateDecoderResolution(DWORD decoderConnectionId, DWORD OnOffResolution0, DWORD OnOffResolution1, DWORD OnOffResolution4)
{
	UPDATE_DECODER_RESOLUTION_S tUpdateDecoderResolutionStruct;
	memset(&tUpdateDecoderResolutionStruct, 0xff, sizeof(UPDATE_DECODER_RESOLUTION_S));

	tUpdateDecoderResolutionStruct.tDecoderPhysicalId.connection_id = decoderConnectionId;
	tUpdateDecoderResolutionStruct.tDecoderPhysicalId.party_id      = DUMMY_PARTY_ID;
	tUpdateDecoderResolutionStruct.nOnOffResolution0                = OnOffResolution0;
	tUpdateDecoderResolutionStruct.nOnOffResolution1                = OnOffResolution1;
	tUpdateDecoderResolutionStruct.nOnOffResolution4                = OnOffResolution4;

	CSegment* pMsg = new CSegment;
	pMsg->Put((BYTE*)(&tUpdateDecoderResolutionStruct), sizeof(UPDATE_DECODER_RESOLUTION_S));
	SendMsgToMPL(VIDEO_UPDATE_DECODER_RESOLUTION_REQ, pMsg);

	POBJDELETE(pMsg);
}

////////////////////////////////////////////////////////////////////////////
EVideoConfType CVideoHardwareInterface::TranslateVideoConfTypeToApi(eVideoConfType videoConfType)
{
	switch (videoConfType)
	{
		case eVideoConfTypeDummy         : return E_VIDEO_CONF_TYPE_DUMMY;
		case eVideoConfTypeCP            : return E_VIDEO_CONF_TYPE_CP;
		case eVideoConfTypeVSW           : return E_VIDEO_CONF_TYPE_VSW;
		case eVideoConfTypeCopHD108025fps: return E_VIDEO_CONF_TYPE_COP_HD1080_25FPS;
		case eVideoConfTypeCopHD72050fps : return E_VIDEO_CONF_TYPE_COP_HD720_50FPS;
		case eVideoConfTypeLast          : return E_VIDEO_CONF_TYPE_LAST;

		default:
			FPASSERT(videoConfType+1000);
			return E_VIDEO_CONF_TYPE_DUMMY;
	}
}

////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::SendConnectPCMMenu(ConnectionID PcmEncoderConnectionId, DWORD pcmMenuId)
{
	TB_MSG_CONNECT_PCM_S tConnectPcmStruct;
	memset(&tConnectPcmStruct, 0, sizeof(tConnectPcmStruct));

	tConnectPcmStruct.physical_port1.connection_id = PcmEncoderConnectionId;
	tConnectPcmStruct.physical_port1.party_id      = GetPartyRsrcId();
	tConnectPcmStruct.pcm_process_id               = pcmMenuId;

	CSegment msg;
	msg.Put((BYTE*)(&tConnectPcmStruct), sizeof(tConnectPcmStruct));
	SendMsgToMPL(TB_MSG_CONNECT_PCM_REQ, &msg);
}

////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::SendDisconnectPCMMenu(ConnectionID PcmEncoderConnectionId, DWORD pcmMenuId)
{
	TB_MSG_CONNECT_PCM_S tConnectPcmStruct;
	memset(&tConnectPcmStruct, 0, sizeof(tConnectPcmStruct));

	tConnectPcmStruct.physical_port1.connection_id = PcmEncoderConnectionId;
	tConnectPcmStruct.physical_port1.party_id      = GetPartyRsrcId();
	tConnectPcmStruct.pcm_process_id               = pcmMenuId;

	CSegment msg;
	msg.Put((BYTE*)(&tConnectPcmStruct), sizeof(tConnectPcmStruct));
	SendMsgToMPL(TB_MSG_DISCONNECT_PCM_REQ, &msg);
}

////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::SendKillPort()
{
	SendMsgToMPL(KILL_PORT_REQ, NULL);
}

////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::UpdateMaxDpbFromMaxFs(DWORD& maxDPB, DWORD maxFS)
{
	DWORD dpb_from_fs = CH264Details::GetMaxDpbFromMaxFs(maxFS * 256);
	if (dpb_from_fs > maxDPB || maxDPB == DWORD(-1))
	{
		FTRACEINTO << "maxDPB:" << maxDPB << ", dpb_from_fs:" << dpb_from_fs;
		maxDPB = dpb_from_fs;
	}
	else
	{
		FTRACEINTO << "maxDPB:" << maxDPB << ", dpb_from_fs:" << dpb_from_fs << " - max DPB from channel is higher, so no update";
	}
}

////////////////////////////////////////////////////////////////////////////
EProfileType CVideoHardwareInterface::TranslateVideoProfileToApi(eVideoProfile videoProfile)
{
	EProfileType apiProfileType = E_PROFILE_DUMMY;

	switch (videoProfile)
	{
		case eVideoProfileBaseline: return E_PROFILE_BASELINE;
		case eVideoProfileHigh    : return E_PROFILE_HIGH;
		case eVideoProfileRtv     : return E_PROFILE_RTV_ADVANCED;
		case eVideoProfileMain    : return E_PROFILE_MAIN;
		case eVideoProfileDummy   : return E_PROFILE_DUMMY;

		default:
			FPASSERT(videoProfile+1000);
			return E_PROFILE_DUMMY;
	}
}

////////////////////////////////////////////////////////////////////////////
EPacketPayloadFormat CVideoHardwareInterface::TranslatePacketPayloadFormatToApi(eVideoPacketPayloadFormat VideoPacketPayloadFormat)
{
	switch (VideoPacketPayloadFormat)
	{
		case eVideoPacketPayloadFormatSingleUnit       : return E_PACKET_PAYLOAD_FORMAT_SINGLE_UNIT;
		case eVideoPacketPayloadFormatFragmentationUnit: return E_PACKET_PAYLOAD_FORMAT_FRAGMENTATION_UNIT;
		case eVideoPacketPayloadFormatDummy            : return E_PACKET_PAYLOAD_FORMAT_DUMMY;

		default:
			FPASSERT(VideoPacketPayloadFormat+1000);
			return E_PACKET_PAYLOAD_FORMAT_DUMMY;
	}
}

////////////////////////////////////////////////////////////////////////////
bool CVideoHardwareInterface::IsH263HighBbIntra()
{
	return GetSystemCfgFlagInt<bool>("H263_HIGH_BIT_BUDGET_INTRA");
}

////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::SendDSPSmartSwitchChangeLayout(
	CLayout* pLayout,
	CVisualEffectsParams* pVisualEffects,
	CSiteNameInfo* pSiteNameInfo,
	DWORD speakerPlaceInLayout,
	eVideoResolution videoEncoderResolution,
	DWORD decoderDetectedModeWidth, DWORD decoderDetectedModeHeight,
	DWORD decoderDetectedSampleAspectRatioWidth, DWORD decoderDetectedSampleAspectRatioHeight,
	DWORD videoAlg, DWORD fs, DWORD mbps,
	eVideoConfType videoConfType,
	BYTE isVSW, BYTE isSiteNamesEnabled)
{
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pLayout));

	bool newVisualEffects = false;
	if (!CPObject::IsValidPObjectPtr(pVisualEffects))
	{
		TRACEINTO << "New CVisualEffectsParams object was created for VSW";
		pVisualEffects = new CVisualEffectsParams();
		newVisualEffects = true;
	}

	MCMS_CM_CHANGE_LAYOUT_S tChangeLayoutStruct;
	memset(&tChangeLayoutStruct, 0xff, sizeof(MCMS_CM_CHANGE_LAYOUT_S));

	tChangeLayoutStruct.nLayoutType                                                  = (BYTE)TranslateVideoLayoutTypeToApi(pLayout->GetLayoutType());
	tChangeLayoutStruct.nEncoderResolutionRatio                                      = (BYTE)TranslateToVideoResolutionRatio(videoAlg, videoEncoderResolution, fs, mbps, videoConfType, isVSW);
	tChangeLayoutStruct.nDecoderDetectedMode.nDecoderDetectedModeWidth               = (DWORD)decoderDetectedModeWidth;
	tChangeLayoutStruct.nDecoderDetectedMode.nDecoderDetectedModeHeight              = (DWORD)decoderDetectedModeHeight;
	tChangeLayoutStruct.nDecoderDetectedMode.nDecoderDetectedSampleAspectRatioWidth  = (DWORD)decoderDetectedSampleAspectRatioWidth;
	tChangeLayoutStruct.nDecoderDetectedMode.nDecoderDetectedSampleAspectRatioHeight = (DWORD)decoderDetectedSampleAspectRatioHeight;

	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	CPartyImageLookupTable* pPartyImageLookupTable = GetPartyImageLookupTable();
	WORD numOfImages = pLayout->GetNumberOfSubImages();
	tChangeLayoutStruct.atImageParam = new MCMS_CM_IMAGE_PARAM_S[numOfImages];
	memset((BYTE*)(tChangeLayoutStruct.atImageParam), 0xff, numOfImages*sizeof(MCMS_CM_IMAGE_PARAM_S));

	for (int i = 0; i < numOfImages; i++)
	{
		DWORD decoderConnectionId                         = INVALID;
		DWORD decoderPartyId                              = INVALID;
		DWORD artPartyId                                  = INVALID;
		BYTE  imageResolutionRatio                        = RESOLUTION_RATIO_0;
		DWORD imageDecoderDetectedModeWidth               = DEFAULT_DECODER_DETECTED_MODE_WIDTH;
		DWORD imageDecoderDetectedModeHeight              = DEFAULT_DECODER_DETECTED_MODE_HEIGHT;
		DWORD imageDecoderDetectedSampleAspectRatioWidth  = DEFAULT_DECODER_DETECTED_SAMPLE_ASPECT_RATIO_WIDTH;
		DWORD imageDecoderDetectedSampleAspectRatioHeight = DEFAULT_DECODER_DETECTED_SAMPLE_ASPECT_RATIO_HEIGHT;
		ERelativeSizeOfImageInLayout sizeOfImageInLayout  = E_NOT_IN_LAYOUT;

		// site name send to card decoded in UCS-2BE, 16 bit encoding, null terminating = 00
		tChangeLayoutStruct.atImageParam[i].siteName[0] = '\0';
		tChangeLayoutStruct.atImageParam[i].siteName[1] = '\0';

		CVidSubImage* pVidSubImage = (*pLayout)[i];
		if (pVidSubImage)
		{
			DWORD partyRscId = pVidSubImage->GetImageId();
			if (partyRscId)
			{
				const CImage* pImage = pPartyImageLookupTable->GetPartyImage(partyRscId);
				PASSERTSTREAM(!pImage, "CVideoHardwareInterface::SendDSPSmartSwitchChangeLayout - Failed, The lookup table doesn't have an element, PartyId:" << partyRscId);

				if (pImage)
				{
					decoderConnectionId                         = pImage->GetDspSmartSwitchConnectionId();
					decoderPartyId                              = pImage->GetDspSmartSwitchImageEntityId();
					artPartyId                                  = pImage->GetArtPartyId();                                          // In CP same as decoderPartyId in cop the party that connected to the cop decoder decoderPartyId
					sizeOfImageInLayout                         = TranslateImageSizeToScale(pVidSubImage->GetSizeX(), pVidSubImage->GetSizeY());
					imageResolutionRatio                        = (BYTE)TranslateToVideoResolutionRatio(pImage->GetVideoAlgo(), pImage->GetVideoResolution(), pImage->GetFS(), pImage->GetMBPS(), videoConfType, isVSW);
					imageDecoderDetectedModeWidth               = pImage->GetDecoderDetectedModeWidth();
					imageDecoderDetectedModeHeight              = pImage->GetDecoderDetectedModeHeight();
					imageDecoderDetectedSampleAspectRatioWidth  = pImage->GetDecoderDetectedSampleAspectRatioWidth();
					imageDecoderDetectedSampleAspectRatioHeight = pImage->GetDecoderDetectedSampleAspectRatioHeight();
					if (pSiteNameInfo && pSiteNameInfo->GetDisplayMode() != eSiteNameOff)
						TranslateSiteNameToUCS2(pImage->GetSiteName(), tChangeLayoutStruct.atImageParam[i].siteName, MAX_SITE_NAME_SIZE);
				}
			}
		}

		tChangeLayoutStruct.atImageParam[i].tDecoderPhysicalId.connection_id                             = decoderConnectionId;
		tChangeLayoutStruct.atImageParam[i].tDecoderPhysicalId.party_id                                  = decoderPartyId;
		tChangeLayoutStruct.atImageParam[i].nArtPartyId                                                  = artPartyId;
		tChangeLayoutStruct.atImageParam[i].nDecoderSizeInLayout                                         = sizeOfImageInLayout;
		tChangeLayoutStruct.atImageParam[i].nDecoderResolutionRatio                                      = imageResolutionRatio;
		tChangeLayoutStruct.atImageParam[i].nDecoderDetectedMode.nDecoderDetectedModeWidth               = imageDecoderDetectedModeWidth;
		tChangeLayoutStruct.atImageParam[i].nDecoderDetectedMode.nDecoderDetectedModeHeight              = imageDecoderDetectedModeHeight;
		tChangeLayoutStruct.atImageParam[i].nDecoderDetectedMode.nDecoderDetectedSampleAspectRatioWidth  = imageDecoderDetectedSampleAspectRatioWidth;
		tChangeLayoutStruct.atImageParam[i].nDecoderDetectedMode.nDecoderDetectedSampleAspectRatioHeight = imageDecoderDetectedSampleAspectRatioHeight;
	}

	FillChangeLayoutAttributes(pLayout, tChangeLayoutStruct.tChangeLayoutAttributes, pVisualEffects, pSiteNameInfo, speakerPlaceInLayout, videoConfType);

// Removed in V7.8 since MPM is not supported - pDecoderResolutionTable is allocated only in eSystemCardsMode_mpm (otherwise null).
/*  CDecoderResolutionTable* pDecoderResolutionTable = ::GetpDecoderResolutionTable();
  if (pDecoderResolutionTable)
    pDecoderResolutionTable->UpdateTable(tChangeLayoutStruct, GetConnectionId());
*/

	CSegment* pMsg = new CSegment;
	pMsg->Put((BYTE*)(&tChangeLayoutStruct), sizeof(MCMS_CM_CHANGE_LAYOUT_S) - sizeof(MCMS_CM_IMAGE_PARAM_S*));
	pMsg->Put((BYTE*)(&tChangeLayoutStruct.atImageParam[0]), numOfImages*(sizeof(MCMS_CM_IMAGE_PARAM_S)));


	SendMsgToMPL(VIDEO_ENCODER_DSP_SMART_SWITCH_CHANGE_LAYOUT_REQ, pMsg);

	POBJDELETE(pMsg);
	PDELETEA(tChangeLayoutStruct.atImageParam);

	if(newVisualEffects){
		POBJDELETE(pVisualEffects);
	}
}

////////////////////////////////////////////////////////////////////////////
// Message sent from VideoBridge
void CVideoHardwareInterface::SendAddVideoOperationPointSet(CVideoOperationPointsSet* videoOperationPointsSet)
{
	PTRACE(eLevelInfoNormal, "CVideoHardwareInterface::SendAddVideoOperationPointSet");
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(videoOperationPointsSet));
	VIDEO_OPERATION_POINT_SET_S videoOperationPointSetStruct;
	FillVideoOperationSetParams(videoOperationPointSetStruct, videoOperationPointsSet);
	CSegment* pMsg = new CSegment;
	pMsg->Put((BYTE*)(&videoOperationPointSetStruct), sizeof(VIDEO_OPERATION_POINT_SET_S));
	SendMsgToMPL(ADD_VIDEO_OPERATION_POINT_SET, pMsg);
	POBJDELETE(pMsg);
}

////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::SendRemoveVideoOperationPointSet(CVideoOperationPointsSet* videoOperationPointsSet)
{
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(videoOperationPointsSet));
	VIDEO_OPERATION_POINT_SET_S videoOperationPointSetStruct;
	FillVideoOperationSetParams(videoOperationPointSetStruct, videoOperationPointsSet);
	CSegment* pMsg = new CSegment;
	pMsg->Put((BYTE*)(&videoOperationPointSetStruct), sizeof(VIDEO_OPERATION_POINT_SET_S));
	SendMsgToMPL(REMOVE_VIDEO_OPERATION_POINT_SET, pMsg);
	POBJDELETE(pMsg);
}

////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::FillVideoOperationSetParams(VIDEO_OPERATION_POINT_SET_S& videoOperationPointSetStruct, CVideoOperationPointsSet* videoOperationPointsSet)
{
	memset(&videoOperationPointSetStruct, 0, sizeof(VIDEO_OPERATION_POINT_SET_S));

	videoOperationPointSetStruct.operationPointSetId = videoOperationPointsSet->GetSetId();
	const std::list <VideoOperationPoint>* operationPointList   = videoOperationPointsSet->GetOperationPointsList();
	int numOfOperationPoints = operationPointList->size();
	if (numOfOperationPoints > MAX_NUM_OPERATION_POINTS_PER_SET)
	{
		PASSERT(numOfOperationPoints);
		numOfOperationPoints = MAX_NUM_OPERATION_POINTS_PER_SET;
	}

	videoOperationPointSetStruct.numberOfOperationPoints = numOfOperationPoints;
	std::list <VideoOperationPoint>::const_iterator itr = operationPointList->begin();
	for (int i = 0; itr != operationPointList->end() && i < MAX_NUM_OPERATION_POINTS_PER_SET; ++itr, i++)
	{
		videoOperationPointSetStruct.tVideoOperationPoints[i].layerId     = (*itr).m_layerId;
		videoOperationPointSetStruct.tVideoOperationPoints[i].Tid         = (*itr).m_tid;
		videoOperationPointSetStruct.tVideoOperationPoints[i].Did         = (*itr).m_did;
		videoOperationPointSetStruct.tVideoOperationPoints[i].Qid         = (*itr).m_qid;
		videoOperationPointSetStruct.tVideoOperationPoints[i].Pid         = 0; // Currently not in use keren to check
		videoOperationPointSetStruct.tVideoOperationPoints[i].profile     = (*itr).m_videoProfile;
		videoOperationPointSetStruct.tVideoOperationPoints[i].level       = 0; // currently not needed in MRMP
		videoOperationPointSetStruct.tVideoOperationPoints[i].frameWidth  = (*itr).m_frameWidth;
		videoOperationPointSetStruct.tVideoOperationPoints[i].frameHeight = (*itr).m_frameHeight;
		videoOperationPointSetStruct.tVideoOperationPoints[i].frameRate   = (*itr).m_frameRate;
		videoOperationPointSetStruct.tVideoOperationPoints[i].maxBitRate  = (*itr).m_maxBitRate;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
bool CVideoHardwareInterface::IsIntermediateSDResolution(bool bUseIntermediateSDResolution)
{
	return GetSystemCfgFlagInt<bool>("USE_INTERMEDIATE_SD_RESOLUTION") ? bUseIntermediateSDResolution : false;
}

///////////////////////////////////////////////////////////////////////////////////////////
void CVideoHardwareInterface::SendVideoRelaySourcesRequest(CVideoRelaySourcesParams* pVideoRelaySourcesParams)
{
//	pVideoRelaySourcesParams->Dump();
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pVideoRelaySourcesParams));
	std::list <CVideoRelaySourceApi> vidStreamList = pVideoRelaySourcesParams->GetVideoSourcesList();

	DWORD numOfStreams = vidStreamList.size();
	int   messLen;
	if (numOfStreams > 1)
	{
		messLen = (sizeof(VideoRelaySourcesParams)) + (numOfStreams-1)*(sizeof(VideoRealySourceParams));
	}
	else
	{
		messLen = sizeof(VideoRelaySourcesParams);
	}

	VideoRelaySourcesParams* pSourcesParams = (VideoRelaySourcesParams*)new BYTE[messLen];

	pSourcesParams->unChannelHandle             = pVideoRelaySourcesParams->GetChannelHandle();
	pSourcesParams->unSequenseNumber            = pVideoRelaySourcesParams->GetSeqNum();
	pSourcesParams->unSourceOperationPointSetId = pVideoRelaySourcesParams->GetSourceOperationPointSetId();
	pSourcesParams->nNumberOfSourceParams       = numOfStreams;

	VideoRealySourceParams* streamInfo = ((VideoRealySourceParams*)&(pSourcesParams->tVideoRelaySourceParamsArray[0]));

	std::list <CVideoRelaySourceApi>::iterator itr = vidStreamList.begin();

	for (; itr != vidStreamList.end(); itr++)
	{
		streamInfo->unChannelHandle = (*itr).GetChannelHandle();
		streamInfo->unSyncSource    = (*itr).GetSyncSource();
		streamInfo->unLayerId       = (*itr).GetLayerId();
		streamInfo->unPipeId        = (*itr).GetPipeId();
		streamInfo->bIsSpeaker      = (*itr).GetIsSpeaker();
		streamInfo->unTId           = (*itr).GetTid();

		streamInfo++;
	}

	// //////////////////////////
/*	std::ostringstream pContentStr;
    pContentStr << "\nCONTENT after Keren's filling: VideoRelaySourcesParams:";
    pContentStr << "\n  unChannelHandle              :" << pSourcesParams->unChannelHandle;
    pContentStr << "\n  unSequenseNumber             :" << pSourcesParams->unSequenseNumber;
    pContentStr << "\n  unSourceOperationPointSetId  :" << pSourcesParams->unSourceOperationPointSetId;
    pContentStr << "\n  nNumberOfSourceParams        :" << pSourcesParams->nNumberOfSourceParams;
    numOfStreams = pSourcesParams->nNumberOfSourceParams;
    streamInfo = ((VideoRealySourceParams* )&(pSourcesParams->tVideoRelaySourceParamsArray[0]));
    for(DWORD i=0;i<numOfStreams;i++)
    {
         pContentStr << "\n  VideoRealySourceParams["<<i<<"]";
        pContentStr << "\n\t unChannelHandle: "<< pSourcesParams->tVideoRelaySourceParamsArray[i].unChannelHandle;
        pContentStr << "\n\t unSyncSource   : "<< pSourcesParams->tVideoRelaySourceParamsArray[i].unSyncSource;
        pContentStr << "\n\t unLayerId      : "<< pSourcesParams->tVideoRelaySourceParamsArray[i].unLayerId;
        pContentStr << "\n\t unPipeId       : "<< pSourcesParams->tVideoRelaySourceParamsArray[i].unPipeId;
        pContentStr << "\n\t bIsSpeaker     : "<< pSourcesParams->tVideoRelaySourceParamsArray[i].bIsSpeaker;

        pContentStr << "\n\t streamInfo old val: "<< streamInfo;
        pContentStr << "\n\t sizeof(VideoRealySourceParams): "<< sizeof(VideoRealySourceParams);

//        streamInfo ++ ;//= streamInfo+sizeof(VideoRealySourceParams);
        ++streamInfo;
        pContentStr << "\n\t streamInfo new val: "<< streamInfo;
    }
    TRACEINTOFUNC << "Keren 2 " << pContentStr.str().c_str();*/
	// ///////////////////////////

	CSegment* pMsg = new CSegment;

	pMsg->Put((BYTE*)(pSourcesParams), messLen);
	SendMsgToMPL(CONF_PARTY_MRMP_VIDEO_SOURCES_REQ, pMsg);
	POBJDELETE(pMsg);
	delete [] pSourcesParams;
}

///////////////////////////////////////////////////////////////////////////////////////////
//DWORD CVideoHardwareInterface::SendMixSVCtoAVCOpenOutChannelRequest(CMrmpOpenChannelRequestContainer* pOpenOutChannelParams)
//{
//	CSegment* pMsg    = new CSegment;
//	int       messLen = sizeof (MrmpOpenChannelRequestStruct);
//	pMsg->Put((BYTE*)(&pOpenOutChannelParams->m_openChannel), messLen);
//
//	DWORD reqId = SendMsgToMPL(CONF_PARTY_MRMP_OPEN_CHANNEL_REQ, pMsg);
//	POBJDELETE(pMsg);
//	return reqId;
//}

///////////////////////////////////////////////////////////////////////////////////////////
//DWORD CVideoHardwareInterface::SendMixSVCtoAVCCloseOutChannelRequest(ConnectionID decoderConnectionID, PartyRsrcID decoderPartyID, DWORD channelHandle)
//{
//	MrmpCloseChannelRequestStruct* pCloseOutChannelParams = new MrmpCloseChannelRequestStruct;
//	memset(pCloseOutChannelParams, 0, sizeof(MrmpCloseChannelRequestStruct));
//
//	pCloseOutChannelParams->tMrmpCloseChannelRequestMessage.channelHandle 		= channelHandle;
//	pCloseOutChannelParams->tMrmpCloseChannelRequestMessage.m_channelType       = kSvcAvcChnlType;
//	pCloseOutChannelParams->tMrmpCloseChannelRequestMessage.m_channelDirection  = eMediaOut;
//	pCloseOutChannelParams->physicalId[0].connection_id = decoderConnectionID; //@#@ - physicalId
//	pCloseOutChannelParams->physicalId[0].party_id      = decoderPartyID;
//
//	CSegment* pMsg    = new CSegment;
//	int       messLen = sizeof (MrmpCloseChannelRequestStruct);
//	pMsg->Put((BYTE*)(pCloseOutChannelParams), messLen);
//
//	DWORD reqId = SendMsgToMPL(CONF_PARTY_MRMP_CLOSE_CHANNEL_REQ, pMsg);
//	POBJDELETE(pMsg);
//	return reqId;
//}

