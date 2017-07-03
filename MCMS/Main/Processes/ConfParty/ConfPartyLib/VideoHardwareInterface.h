#ifndef VIDEO_HARDWARE_INTERAFCE_H__
#define VIDEO_HARDWARE_INTERAFCE_H__

///////////////////////////////////////////////////////////////////////////
#include "HardwareInterface.h"

#include "VideoApiDefinitions.h"
#include "VideoDefines.h"
#include "HostCommonDefinitions.h"

#include "VisualEffectsParams.h"
#include "VideoStructs.h"

#include "UnicodeDefines.h"
#include "ConfPartyApiDefines.h"

#include "TextOnScreenMngr.h"

#include "VideoOperationPointsSet.h"
#include "VideoRelaySourcesParams.h"

#include <iconv.h>
#include <errno.h>

///////////////////////////////////////////////////////////////////////////
class CLayout;
class CVisualEffectsParams;
class CIVRPlayMessage;
class CGathering;
class CSiteNameInfo;
class CAvcToSvcOpenEncoder;


bool operator==(BORDER_PARAM_S& first, const BORDER_PARAM_S& second);
//MCMS_CM_BORDER_PARAM_S& operator=(MCMS_CM_BORDER_PARAM_S& dest, const BORDER_PARAM_S& source);
///////////////////////////////////////////////////////////////////////////
class CVideoHardwareInterface: public CHardwareInterface
{
	friend class CBridgePartyVideoOut;

	CLASS_TYPE_1(CVideoHardwareInterface, CHardwareInterface)

	virtual const char* NameOf() const
	{ return GetCompileType(); }

	virtual ~CVideoHardwareInterface();

private: // prohibit copying

	CVideoHardwareInterface(const CVideoHardwareInterface &);
	CVideoHardwareInterface& operator=(const CVideoHardwareInterface &);

public:

	CVideoHardwareInterface(
		ConnectionID ConnectionId = DUMMY_CONNECTION_ID,
		PartyRsrcID ParId = DUMMY_PARTY_ID,
		ConfRsrcID ConfId = DUMMY_CONF_ID,
		eLogicalResourceTypes LRT = eLogical_res_none);

	struct FontColor
	{
		DWORD nTextColor;
		DWORD nBackgroundColor;
	};

	DWORD SendOpenEncoder(
		DWORD videoAlg, DWORD videoBitRate,
		eVideoFrameRate videoQCifFrameRate, eVideoFrameRate videoCifFrameRate, eVideoFrameRate video4CifFrameRate,
		DWORD decoderDetectedModeWidth, DWORD decoderDetectedModeHeight,
		DWORD decoderDetectedSampleAspectRatioWidth, DWORD decoderDetectedSampleAspectRatioHeight,
		eVideoResolution videoResolution,
		DWORD mbps, DWORD fs, DWORD sampleAspectRatio, DWORD staticMB,
		eVideoQuality videoQuality,
		BYTE isVideoClarityEnabled,
		eVideoConfType videoConfType,
		DWORD dpb, BOOL isLinkEncoder,
		DWORD parsingMode = E_PARSING_MODE_CP,
		eTelePresencePartyType eTelePresenceMode = eTelePresencePartyNone,
		eVideoFrameRate resolutionFrameRate = eVideoFrameRateDUMMY,
		EVideoResolutionTableType eVideoResolutionTableType = E_VIDEO_RESOLUTION_TABLE_REGULAR,
		DWORD horizontalCroppingPercentage = INVALID, DWORD verticalCroppingPercentage = INVALID,
		eVideoProfile profile = eVideoProfileBaseline,
		eVideoPacketPayloadFormat packetPayloadFormat = eVideoPacketPayloadFormatSingleUnit,
		BYTE bIsTipMode = FALSE,
		BYTE bUseIntermediateSDResolution = FALSE,
		BOOL isEncodeBFrames = FALSE,
		DWORD dwFrThreshold = 0,
		FontTypesEnum fontType = ftDefault,
		BOOL bIs263Plus = FALSE,
		BOOL bIsCallGenerator = FALSE,
		BYTE followSpeakerOn1x1 = NO,
		DWORD ssrc = 0, DWORD height = 0, DWORD width = 0, DWORD prID = 1);

	// Sends either OPEN or UPDATE request
	DWORD SendUpdateEncoder(ENCODER_PARAM_S& params, bool open = false);

//#if 0
	DWORD SendUpdateEncoder(
		BOOL isLPR,
		DWORD NewAlg, DWORD videoBitRate,
		eVideoFrameRate videoQCifFrameRate, eVideoFrameRate videoCifFrameRate, eVideoFrameRate video4CifFrameRate,
		DWORD decoderDetectedModeWidth, DWORD decoderDetectedModeHeight,
		DWORD decoderDetectedSampleAspectRatioWidth, DWORD decoderDetectedSampleAspectRatioHeight,
		eVideoResolution videoResolution,
		DWORD mbps, DWORD fs, DWORD sampleAspectRatio, DWORD staticMB,
		eTelePresencePartyType eTelePresenceMode,
		eVideoQuality videoQuality,
		BYTE isVideoClarityEnabled,
		eVideoConfType videoConfType,
		DWORD dpb,
		BOOL isLinkEncoder,
		eVideoFrameRate resolutionFrameRate = eVideoFrameRateDUMMY,
		EVideoResolutionTableType eVideoResolutionTableType = E_VIDEO_RESOLUTION_TABLE_REGULAR,
		DWORD horizontalCroppingPercentage = INVALID, DWORD verticalCroppingPercentage = INVALID,
		eVideoProfile profile = eVideoProfileBaseline,
		eVideoPacketPayloadFormat packetPayloadFormat = eVideoPacketPayloadFormatSingleUnit,
		BYTE bIsTipMode = FALSE,
		BYTE bUseIntermediateSDResolution = FALSE,
		BOOL isEncodeBFrames = FALSE,
		DWORD dwFrThreshold = 0,
		FontTypesEnum fontType = ftDefault,
		BOOL bIs263Plus = FALSE,
		BOOL bIsCallGenerator = FALSE,
		BYTE followSpeakerOn1x1 = NO,
		DWORD ssrc = 0, DWORD height = 0, DWORD width = 0, DWORD prID = 1);
//#endif

	void FillEncoderParams(
		ENCODER_PARAM_S& tEncoderStruct,
		BOOL isLPR,
		DWORD NewAlg, DWORD videoBitRate,
		eVideoFrameRate videoQCifFrameRate, eVideoFrameRate videoCifFrameRate, eVideoFrameRate video4CifFrameRate,
		DWORD decoderDetectedModeWidth, DWORD decoderDetectedModeHeight,
		DWORD decoderDetectedSampleAspectRatioWidth, DWORD decoderDetectedSampleAspectRatioHeight,
		eVideoResolution videoResolution,
		DWORD mbps, DWORD fs, DWORD sampleAspectRatio, DWORD staticMB,
		eTelePresencePartyType eTelePresenceMode, eVideoQuality videoQuality,
		BYTE isVideoClarityEnabled,
		eVideoConfType videoConfType,
		DWORD parsingMode,
		DWORD dpb,
		BOOL isLinkEncoder,
		eVideoFrameRate resolutionFrameRate = eVideoFrameRateDUMMY,
		EVideoResolutionTableType eVideoResolutionTableType = E_VIDEO_RESOLUTION_TABLE_REGULAR,
		DWORD horizontalCroppingPercentage = INVALID,
		DWORD verticalCroppingPercentage = INVALID,
		eVideoProfile profile = eVideoProfileBaseline,
		eVideoPacketPayloadFormat packetPayloadFormat = eVideoPacketPayloadFormatSingleUnit,
		BYTE bIsTipMode = FALSE,
		BYTE bUseIntermediateSDResolution = FALSE,
		BOOL bEncodeBFrames = FALSE,
		DWORD dwFrThreshold = 0,
		FontTypesEnum fontType = ftDefault,
		BOOL bIs263Plus = FALSE,
		BOOL bIsCallGenerator = FALSE,
		BYTE followSpeakerOn1x1 = NO);

	void fillEncoderMsftSVCParams(
		ENCODER_PARAM_S& tEncoderStruct,
		DWORD ssrcID, // Ssrc for T0 – Encoder will use Ssrc+1 and +2 for T1,T2
		DWORD prID,
		eVideoConfType videoConfType,
		DWORD nResolutionWidth, DWORD nResolutionHeight,
		eVideoFrameRate frameRate,
		DWORD videoAlg, DWORD maxBitrate,
		eVideoResolution videoResolution,
		DWORD mbps, DWORD dpb,
		eVideoProfile profile,
		eVideoPacketPayloadFormat packetPayloadFormat); // EPacketPayloadFormat values ?

	void fillEncoderMsftSVCTemporaryLayersParams(ENCODER_PARAM_S& encoder,eVideoFrameRate frameRate, DWORD maxBitrate);

	DWORD SendCloseEncoder();

	DWORD SendOpenEncoderAvcToSvc(CAvcToSvcOpenEncoder* openEncoder);

	void FillOpenEncoderAvcToSvcParams(CAvcToSvcOpenEncoder* openEncoder, ENCODER_PARAM_S* encoderStruct);

	DWORD SendOpenDecoder(
		DWORD videoAlg, DWORD videoBitRate,
		eVideoResolution videoResolution,
		eVideoFrameRate videoQCifFrameRate, eVideoFrameRate videoCifFrameRate, eVideoFrameRate video4CifFrameRate,
		DWORD mbps, DWORD fs, DWORD sampleAspectRatio, DWORD staticMB,
		BYTE isVSW, DWORD backgroundImageID, BYTE isVideoClarityEnabled,
		eVideoConfType videoConfType, DWORD dpb,
		DWORD parsingMode = E_PARSING_MODE_CP,
		eTelePresencePartyType eTelePresenceMode = eTelePresencePartyNone,
		BOOL isAutoBrightness = FALSE,
		eVideoFrameRate videoVGAFrameRate = eVideoFrameRateDUMMY, eVideoFrameRate videoSVGAFrameRate = eVideoFrameRateDUMMY,
		eVideoFrameRate videoXGAFrameRate = eVideoFrameRateDUMMY, EVideoDecoderType decoderType = E_VIDEO_DECODER_NORMAL,
		eVideoFrameRate resolutionFrameRate = eVideoFrameRateDUMMY, eVideoProfile profile = eVideoProfileBaseline,
		eVideoPacketPayloadFormat packetPayloadFormat = eVideoPacketPayloadFormatSingleUnit,
		BYTE bIsTipMode = FALSE, BOOL bIs263Plus = FALSE, BOOL bIsCallGenerator = FALSE,
		DWORD ssrc = 0, DWORD height = 0, DWORD width = 0);

	DWORD SendUpdateDecoder(
		DWORD NewAlg, DWORD videoBitRate,
		eVideoResolution videoResolution, eVideoFrameRate videoQCifFrameRate, eVideoFrameRate videoCifFrameRate, eVideoFrameRate video4CifFrameRate,
		DWORD mbps, DWORD fs, DWORD sampleAspectRatio, DWORD staticMB,
		DWORD backgroundImageID,
		BYTE isVideoClarityEnabled,
		eVideoConfType videoConfType,
		DWORD dpb,
		eTelePresencePartyType eTelePresenceMode = eTelePresencePartyNone,
		BOOL isAutoBrightness = FALSE,
		eVideoFrameRate videoVGAFrameRate = eVideoFrameRateDUMMY, eVideoFrameRate videoSVGAFrameRate = eVideoFrameRateDUMMY, eVideoFrameRate videoXGAFrameRate = eVideoFrameRateDUMMY,
		EVideoDecoderType decoderType = E_VIDEO_DECODER_NORMAL, eVideoFrameRate resolutionFrameRate = eVideoFrameRateDUMMY, eVideoProfile profile =
				eVideoProfileBaseline, eVideoPacketPayloadFormat packetPayloadFormat = eVideoPacketPayloadFormatSingleUnit, BYTE bIsTipMode = FALSE,
				BOOL bIs263Plus = FALSE, BOOL bIsCallGenerator = FALSE, DWORD ssrc = 0, DWORD height = 0, DWORD width = 0);

	void FillDecoderParams(DECODER_PARAM_S& tDecoderStruct, DWORD NewAlg, DWORD videoBitRate, eVideoResolution videoResolution,
		eVideoFrameRate videoQCifFrameRate, eVideoFrameRate videoCifFrameRate, eVideoFrameRate video4CifFrameRate, DWORD mbps, DWORD fs,
		DWORD sampleAspectRatio, DWORD staticMB, DWORD backgroundImageID, BYTE isVideoClarityEnabled, eVideoConfType videoConfType, DWORD parsingMode,
		DWORD dpb, eTelePresencePartyType eTelePresenceMode = eTelePresencePartyNone, BOOL isAutoBrightness = FALSE,
		eVideoFrameRate videoVGAFrameRate = eVideoFrameRateDUMMY, eVideoFrameRate videoSVGAFrameRate = eVideoFrameRateDUMMY,
		eVideoFrameRate videoXGAFrameRate = eVideoFrameRateDUMMY, EVideoDecoderType decoderType = E_VIDEO_DECODER_NORMAL, eVideoProfile profile =
			eVideoProfileBaseline, eVideoPacketPayloadFormat packetPayloadFormat = eVideoPacketPayloadFormatSingleUnit, BYTE bIsTipMode = FALSE,
			BOOL bIs263Plus = FALSE, BOOL bIsCallGenerator = FALSE);

	void ResetParamsForVP8Decoder(DECODER_PARAM_S& tDecoderStruct);
	void ResetParamsForVP8Encoder(ENCODER_PARAM_S& tEncoderStruct);

	void FillVideoOperationSetParams(VIDEO_OPERATION_POINT_SET_S& tVideoOperationPointSet, CVideoOperationPointsSet* videoOperationPointsSet);

	void fillDecoderMsftSVCParams(
		DECODER_PARAM_S& tDecoderStruct,
		eVideoConfType videoConfType,
		DWORD videoAlg,
		DWORD nResolutionWidth, DWORD nResolutionHeight,
		DWORD maxBitrate, eVideoFrameRate frameRate, eVideoResolution videoResolution,
		DWORD dpb);
	bool IsValidMsftSVCParams(DWORD nResolutionWidth, DWORD nResolutionHeight) { return (nResolutionWidth != 0) && (nResolutionHeight != 0); }

	DWORD SendCloseDecoder();

	void SendFastUpdate();
	void SendIvrFastUpdate();

	DWORD SendConnect(ConnectionID ConnectionID1, ConnectionID ConnectionID2, DWORD PartyRcrsID1, DWORD PartyRcrsID2);
	DWORD SendDisconnect(ConnectionID ConnectionID1, ConnectionID ConnectionID2, DWORD PartyRcrsID1, DWORD PartyRcrsID2);

	void SendChangeIndications(const CLayout* pLayout);
	void UpdateIndicationIconSharedMemory(const CLayout* pLayout);
    void SendChangeIndicationsOrUpdate(const CLayout* pLayout,BOOL bUseSharedMemForIndicationIcon = NO);

	void ChangeLayoutSendOrUpdate(
		const CLayout* pLayout, CVisualEffectsParams* pVisualEffects, CSiteNameInfo* pSiteNameInfo,
		DWORD speakerPlaceInLayout,
		eVideoResolution encoderResolution,
		DWORD decoderDetectedModeWidth, DWORD decoderDetectedModeHeight,
		DWORD decoderDetectedSampleAspectRatioWidth, DWORD decoderDetectedSampleAspectRatioHeight,
		DWORD videoAlg, DWORD fs, DWORD mbps,
		eVideoConfType videoConfType,
		BYTE isSiteNamesEnabled, BYTE isTelepresenceMode,
		BOOL bUseSharedMemForChangeLayoutReq = NO, BYTE isVSW = NO);

	void UpdateLayoutSharedMemory(MCMS_CM_CHANGE_LAYOUT_S& tChangeLayoutStruct);

	void SendChangeLayout(MCMS_CM_CHANGE_LAYOUT_S& tChangeLayoutStruct);
	void SendChangeLayoutAttributes(const CLayout* pLayout, CVisualEffectsParams* pVisualEffects, DWORD speakerPlaceInLayout, CSiteNameInfo* pSiteNameInfo, eVideoConfType videoConfType);

	STATUS SendShowSlide(CSegment *pDataSeg, eVideoResolution resolution, DWORD videoAlg, DWORD videoBitRate, DWORD fs, DWORD mbps, BYTE isTipMode = NO);
	STATUS SendUpdateSlide(eVideoResolution videoResolution, DWORD videoAlg, DWORD videoBitRate, DWORD fs, DWORD mbps, BYTE isTipMode = NO);

	void SendStopShowSlide(CSegment *pDataSeg);
	void SendStartPLC(CSegment *pDataSeg);
	void SendStopPLC(CSegment *pDataSeg);
	void SendStopSiteNameDisplay();
	void SendStopTextDisplay();
	void SendTextToDisplay(CTextOnScreenMngr& pTextMsgList);
	void SendMessageOverlayToDisplay(CSegment* pParam);
	void SendGatheringToDisplay(CGathering* pGathering, const char* pszPartyName);
	void StopGathering();
	void SendDisplayRecordingIcon(EIconType eRecordingIcon);
	void SendUpdateDecoderResolution(DWORD decoderCOnnectionId, DWORD OnOffResolution0, DWORD OnOffResolution1, DWORD OnOffResolution4);
	void SendConnectPCMMenu(ConnectionID PcmEncoderConnectionId, DWORD pcmMenuId);
	void SendDisconnectPCMMenu(ConnectionID PcmEncoderConnectionId, DWORD pcmMenuId);
	void SendKillPort();

	void SendDSPSmartSwitchChangeLayout(
		CLayout* pLayout, CVisualEffectsParams* pVisualEffects, CSiteNameInfo* pSiteNameInfo,
		DWORD speakerPlaceInLayout,
		eVideoResolution videoEncoderResolution,
		DWORD decoderDetectedModeWidth, DWORD decoderDetectedModeHeight,
		DWORD decoderDetectedSampleAspectRatioWidth, DWORD decoderDetectedSampleAspectRatioHeight,
		DWORD videoAlg, DWORD fs, DWORD mbps,
		eVideoConfType videoConfType, BYTE isVSW, BYTE isSiteNamesEnabled);

	void SendAddVideoOperationPointSet(CVideoOperationPointsSet* videoOperationPointsSet);
	void SendRemoveVideoOperationPointSet(CVideoOperationPointsSet* videoOperationPointsSet);
	void SendVideoRelaySourcesRequest(CVideoRelaySourcesParams* pVideoRelaySourcesParams);

	static DWORD GetFsForAvcToSvc(DWORD width, DWORD hight);
	static DWORD GetFsForSvcLync(DWORD width, DWORD height, BOOL adjustFs = NO);

	static DWORD TranslateToVideoResolutionRatio(DWORD videoAlg, eVideoResolution videoResolution, DWORD fs, DWORD mbps, eVideoConfType videoConfType, bool isVSW = false);
	static float TranslateVideoFrameRateToNumeric(eVideoFrameRate videoFrameRate);

private:

	static ETextDisplaySpeed TranslateMessageOverlayDisplaySpeedToApi(eMessageOverlaySpeedType speedType);
	FontColor TranslateMessageOverlayColorTypeToApi(eMessageOverlayColorType colorType);

	FontColor TranslateTextColorTypeToApi(eTextColorType colorType);
	ELayoutType TranslateVideoLayoutTypeToApi(LayoutType layout);

	static EVideoProtocol    TranslateVideoProtocolToApi(DWORD videoProtocol);
	static ETelePresenceMode TranslateTelePresenceModeToApi(eTelePresencePartyType eTelePresenceMode);
	static EVideoFrameRate   TranslateVideoFrameRateToApi(eVideoFrameRate videoFrameRate);
	static EVideoResolution  TranslateVideoResolutionToApi(eVideoResolution videoResolution);
	static DWORD             TranslateResolutionToResWidth(eVideoResolution resolution);
	static DWORD             TranslateResolutionToResHeight(eVideoResolution resolution);

	ERelativeSizeOfImageInLayout TranslateImageSizeToScale(WORD size_X, WORD size_Y);
	DWORD TranslateFSToMacroBlocksUnits(DWORD videoProtocol, DWORD fs);
	DWORD TranslateMBPSToMacroBlocksUnits(DWORD videoProtocol, DWORD mbps);

	static ELayoutBorderWidth   TranslatBorderWidthToApi(eLayoutBorderWidth borderWidth);
	static EVideoQualityType    TranslateVideoQualityToApi(eVideoQuality videoQuality, eVideoConfType videoConfType);
	static EVideoConfType       TranslateVideoConfTypeToApi(eVideoConfType videoConfType);
	static EProfileType         TranslateVideoProfileToApi(eVideoProfile videoProfile);
	static EPacketPayloadFormat TranslatePacketPayloadFormatToApi(eVideoPacketPayloadFormat VideoPacketPayloadFormat);

	static void  UpdateMaxDpbFromMaxFs(DWORD& maxDPB, DWORD maxFS);
	static size_t GetMTUSize(bool isLprOn, bool bIsTipMode = false);

	static bool IsH263HighBbIntra();

	static bool IsIntermediateSDResolution(bool bUseIntermediateSDResolution);

	int TranslateSiteNameToUCS2(const char* unicode_site_name, char* ucs2_site_name_buffer, WORD ucs2_buffer_len);
	int TranslateMessageOverlayToUCS2(const char* unicode_site_name, char* ucs2_site_name_buffer, WORD ucs2_buffer_len);
	int TranslateCaptionNameToUCS2(const char* unicode_site_name, char* ucs2_site_name_buffer, WORD ucs2_buffer_len);
	BYTE IsOverlayOrITPLayout(ELayoutType layoutType);
	BYTE IsITPOverlayLayout(ELayoutType layoutType);
	BYTE IsOverlayLayout(ELayoutType layoutType);

	void FillChangeLayoutAttributes(
		const CLayout* pLayout,
		CHANGE_LAYOUT_ATTRIBUTES_S& pChangeLayoutAtttibutesStruct,
		CVisualEffectsParams* pVisualEffects, CSiteNameInfo* pSiteNameInfo,
		DWORD speakerPlaceInLayout, eVideoConfType videoConfType);

	void FillChangeLayoutAttributes(
		const CLayout* pLayout,
		MCMS_CM_CHANGE_LAYOUT_ATTRIBUTES_S& pChangeLayoutAttibutesStruct,
		CVisualEffectsParams* pVisualEffects, CSiteNameInfo* pSiteNameInfo,
		DWORD speakerPlaceInLayout, eVideoConfType videoConfType);

	void FillSiteNamesParams(SITENAMES_PARAM_S& pSiteNamesParams, CSiteNameInfo* pSiteNameInfo);
	void FillSiteNamesParams(MCMS_CM_SITENAMES_PARAM_S& pSiteNamesParams, CSiteNameInfo* pSiteNameInfo);
	void FillFadeInFadeOutParams(FADE_IN_FADE_OUT_PARAM_S& pFadeInFadeOutParams, eVideoConfType videoConfType);

	STATUS ShowSlide(eVideoResolution videoResolution, DWORD videoAlg, DWORD videoBitRate, DWORD fs, DWORD mbps, BYTE isTipMode = NO);

	STATUS AllocateIconv(const char* toEncoding = VIDEO_UNIT_SITE_NAMES_ENCODE_TYPE, const char* fromEncoding = MCMS_INTERNAL_STRING_ENCODE_TYPE);
	STATUS DeallocatIconv();
	WORD ConvertStingEncoding(const char* from_buffer, DWORD from_buffer_len, char* to_buffer, DWORD to_buffer_len);
	bool IsAbleCroppingOnImage(ELayoutType layoutType, ECascadePartyType cascadeMode, eVideoConfType videoConfType);
	bool IsPanoramicCell(ELayoutType layoutType, int cellIndex);
	void GetCroppingPercentageThressold(int& percentageThressoldGeneral, int& percentageThressoldPanoramic, BYTE isTelePresenceMode);
	int FillTherssoldCroppingOnImage(int percentageThressoldGeneral, int percentageThressoldPanoramic, int cellIndex, ELayoutType layoutType);
	void TrafficShapingEncoderParamsUpdate(ENCODER_PARAM_S& enc) const;

private:

	CIVRPlayMessage* m_pOriginalIVRShowSlidePlayMessage; //Must be saved to allow sending on UpdateVideoOutParams while in SLIDE
	iconv_t m_pIconv;

	DWORD m_nVideoFastUpdateReq;
	DWORD m_nIvrFastUpdateReq;
};

///////////////////////////////////////////////////////////////////////////
#endif //VIDEO_HARDWARE_INTERAFCE_H__
