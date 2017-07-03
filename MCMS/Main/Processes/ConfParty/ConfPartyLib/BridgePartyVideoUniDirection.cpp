#include "BridgePartyVideoUniDirection.h"

#include "VideoHardwareInterface.h"
#include "TaskApi.h"
#include "HostCommonDefinitions.h"
#include "H264Util.h"

#include "StatusesGeneral.h"

#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsVideo.h"
#include "OpcodesMcmsCardMngrTB.h"

#include "TraceStream.h"

////////////////////////////////////////////////////////////////////////////
CBridgePartyVideoUniDirection::CBridgePartyVideoUniDirection()
	: CBridgePartyMediaUniDirection()
	, m_videoAlg(0)
	, m_videoBitRate(0)

	, m_videoResolution(eVideoResolutionDummy)

	, m_videoQcifFrameRate(eVideoFrameRateDUMMY)
	, m_videoCifFrameRate(eVideoFrameRateDUMMY)
	, m_video4CifFrameRate(eVideoFrameRateDUMMY)
	, m_videoVGAFrameRate(eVideoFrameRateDUMMY)
	, m_videoSVGAFrameRate(eVideoFrameRateDUMMY)
	, m_videoXGAFrameRate(eVideoFrameRateDUMMY)
	, m_videoFrameRate(eVideoFrameRateDUMMY)

	, m_videoConfType(eVideoConfTypeDummy) //should be initialized in the derived class
	, m_eCascadeMode(eCascadeNone)
	, m_profile(eVideoProfileDummy)
	, m_packetPayloadFormat(eVideoPacketPayloadFormatDummy)
	, m_eResolutionTableType(E_VIDEO_RESOLUTION_TABLE_REGULAR)

	, m_MBPS(INVALID)
	, m_FS(INVALID)
	, m_sampleAspectRatio(DEFAULT_SAMPLE_ASPECT_RATIO)
	, m_staticMB(DEFAULT_STATIC_MB)
	, m_maxDPB(INVALID)
	, m_dwFrThreshold(0)

	, m_pWaitingForUpdateParams(NULL)
	, m_pMaxVideoParamsAccordingToAllocation(NULL)

	, m_croppingHor(INVALID)
	, m_croppingVer(INVALID)

	, m_isH263Plus(false)
	, m_isAutoBrightness(false)
	, m_bEncodeBFramesInRTV(false)

	, m_isPortOpened(false)
	, m_isVideoClarityEnabled(false)
	, m_bIsTipMode(false)
	, m_bUseIntermediateSDResolution(false)
{}

////////////////////////////////////////////////////////////////////////////
CBridgePartyVideoUniDirection::CBridgePartyVideoUniDirection(const CBridgePartyVideoUniDirection& obj)
	: CBridgePartyMediaUniDirection(obj)
{
	m_videoAlg              = obj.m_videoAlg;
	m_videoBitRate          = obj.m_videoBitRate;
	m_eCascadeMode          = obj.m_eCascadeMode;
	m_sampleAspectRatio     = obj.m_sampleAspectRatio;
	m_isVideoClarityEnabled = obj.m_isVideoClarityEnabled;
	m_videoConfType         = obj.m_videoConfType;
	m_isPortOpened          = obj.m_isPortOpened;
	m_croppingHor           = obj.m_croppingHor;
	m_croppingVer           = obj.m_croppingVer;
	m_isAutoBrightness      = obj.m_isAutoBrightness;
	m_videoResolution       = obj.m_videoResolution;
	m_videoFrameRate        = obj.m_videoFrameRate;
	m_isH263Plus            =  false;

	if (m_videoAlg == H264 || m_videoAlg == MS_SVC)
	{
		m_videoQcifFrameRate  = eVideoFrameRateDUMMY;
		m_videoCifFrameRate   = eVideoFrameRateDUMMY;
		m_video4CifFrameRate  = eVideoFrameRateDUMMY;
		m_videoVGAFrameRate   = eVideoFrameRateDUMMY;
		m_videoSVGAFrameRate  = eVideoFrameRateDUMMY;
		m_videoXGAFrameRate   = eVideoFrameRateDUMMY;
		m_MBPS                = obj.m_MBPS;
		m_FS                  = obj.m_FS;
		m_staticMB            = obj.m_staticMB;
		m_profile             = obj.m_profile;
		m_packetPayloadFormat = obj.m_packetPayloadFormat;
		m_maxDPB              = obj.m_maxDPB;
		m_bIsTipMode          = obj.m_bIsTipMode;
		m_bEncodeBFramesInRTV = false;
		m_msftSvcParamsStruct = obj.m_msftSvcParamsStruct;
	}
	else
	{
		m_videoQcifFrameRate  = obj.m_videoQcifFrameRate;
		m_videoCifFrameRate   = obj.m_videoCifFrameRate;
		m_video4CifFrameRate  = obj.m_video4CifFrameRate;
		m_videoVGAFrameRate   = obj.m_videoVGAFrameRate;
		m_videoSVGAFrameRate  = obj.m_videoSVGAFrameRate;
		m_videoXGAFrameRate   = obj.m_videoXGAFrameRate;
		m_MBPS                = INVALID;
		m_FS                  = INVALID;
		m_staticMB            = DEFAULT_STATIC_MB;
		m_packetPayloadFormat = eVideoPacketPayloadFormatDummy;
		m_profile             = eVideoProfileDummy;
		m_maxDPB              = INVALID;
		m_isH263Plus          = obj.m_isH263Plus;
		m_bIsTipMode          = false;
		m_bEncodeBFramesInRTV = obj.m_bEncodeBFramesInRTV;
	}

	m_bUseIntermediateSDResolution = obj.m_bUseIntermediateSDResolution;
	m_dwFrThreshold                = obj.m_dwFrThreshold;

	m_pWaitingForUpdateParams = NULL;
	m_pMaxVideoParamsAccordingToAllocation = NULL;

	if (obj.m_pWaitingForUpdateParams)
	{
		m_pWaitingForUpdateParams = new CBridgePartyVideoParams;
		*m_pWaitingForUpdateParams = *(obj.m_pWaitingForUpdateParams);
	}

	if (obj.m_pMaxVideoParamsAccordingToAllocation)
	{
		m_pMaxVideoParamsAccordingToAllocation = new CBridgePartyVideoParams;
		*m_pMaxVideoParamsAccordingToAllocation = *(obj.m_pMaxVideoParamsAccordingToAllocation);
	}
}

////////////////////////////////////////////////////////////////////////////
CBridgePartyVideoUniDirection::~CBridgePartyVideoUniDirection()
{
	POBJDELETE(m_pWaitingForUpdateParams);
	POBJDELETE(m_pMaxVideoParamsAccordingToAllocation);
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoUniDirection::Create(const CBridgePartyCntl* pBridgePartyCntl, const CRsrcParams* pRsrcParams, const CBridgePartyMediaParams* pMediaParams)
{
	m_pHardwareInterface = new CVideoHardwareInterface;

	CBridgePartyMediaUniDirection::Create(pBridgePartyCntl, pRsrcParams) ;

	const CBridgePartyVideoParams* pVideoParams = (CBridgePartyVideoParams*)pMediaParams;

	m_videoAlg              = pVideoParams->GetVideoAlgorithm();
	m_videoBitRate          = pVideoParams->GetVideoBitRate();
	m_eCascadeMode          = pVideoParams->GetPartyCascadeType();
	m_videoResolution       = pVideoParams->GetVideoResolution();
	m_sampleAspectRatio     = pVideoParams->GetSampleAspectRatio();
	m_isVideoClarityEnabled = pVideoParams->GetIsVideoClarityEnabled();
	m_isH263Plus            = pVideoParams->GetIsH263Plus();
	m_videoFrameRate        = pVideoParams->GetVidFrameRate();

	TRACEINTO << "ConfName:"  << m_pBridgePartyCntl->GetConfName() << ", PartyName:" << m_pBridgePartyCntl->GetName() << ", eCascadeMode:" << (DWORD)m_eCascadeMode << ", videoAlg:" << m_videoAlg;

	if (m_videoAlg == H264 || m_videoAlg == MS_SVC)
	{
		m_videoQcifFrameRate    = eVideoFrameRateDUMMY;
		m_videoCifFrameRate     = eVideoFrameRateDUMMY;
		m_video4CifFrameRate    = eVideoFrameRateDUMMY;
		m_videoVGAFrameRate     = eVideoFrameRateDUMMY;
		m_videoSVGAFrameRate    = eVideoFrameRateDUMMY;
		m_videoXGAFrameRate     = eVideoFrameRateDUMMY;
		m_MBPS                  = pVideoParams->GetMBPS();
		m_FS                    = pVideoParams->GetFS();
		m_staticMB              = pVideoParams->GetStaticMB();
		m_packetPayloadFormat   = pVideoParams->GetPacketFormat();
		m_profile               = pVideoParams->GetProfile();
		m_maxDPB                = pVideoParams->GetMaxDPB();
		m_bIsTipMode            = pVideoParams->GetIsTipMode();
		m_bEncodeBFramesInRTV   = false;
		m_msftSvcParamsStruct   = pVideoParams->MsSvcParams();

		TRACEINTO << m_msftSvcParamsStruct;
	}
	else if (m_videoAlg == VP8)
	{
		m_videoQcifFrameRate    = eVideoFrameRateDUMMY;
		m_videoCifFrameRate     = eVideoFrameRateDUMMY;
		m_video4CifFrameRate    = eVideoFrameRateDUMMY;
		m_videoVGAFrameRate     = eVideoFrameRateDUMMY;
		m_videoSVGAFrameRate    = eVideoFrameRateDUMMY;
		m_videoXGAFrameRate     = eVideoFrameRateDUMMY;
		m_MBPS                  = pVideoParams->GetMBPS();
		m_FS                    = pVideoParams->GetFS();
		m_staticMB              = pVideoParams->GetStaticMB();
		m_packetPayloadFormat   = pVideoParams->GetPacketFormat();
		m_profile               = pVideoParams->GetProfile();
		m_maxDPB                = pVideoParams->GetMaxDPB();
		m_bIsTipMode            = pVideoParams->GetIsTipMode();
		m_bEncodeBFramesInRTV   = false;
		memset(&m_msftSvcParamsStruct,0, sizeof(MsSvcParamsStruct));
		TRACEINTO << " VP8: m_MBPS: " << (DWORD)m_MBPS << ", m_FS: " << (DWORD)m_FS << ", video party type: " << eVideoPartyTypeNames[pVideoParams->GetVideoPartyType()];
	}
	else if (m_videoAlg == RTV)
	{
		m_videoQcifFrameRate	= eVideoFrameRateDUMMY;
		m_videoCifFrameRate		= eVideoFrameRateDUMMY;
		m_video4CifFrameRate	= eVideoFrameRateDUMMY;
		m_videoVGAFrameRate     = eVideoFrameRateDUMMY;
		m_videoSVGAFrameRate    = eVideoFrameRateDUMMY;
		m_videoXGAFrameRate     = eVideoFrameRateDUMMY;
		m_MBPS                  = pVideoParams->GetMBPS();
		m_FS                    = pVideoParams->GetFS();
		m_staticMB              = DEFAULT_STATIC_MB;
		m_packetPayloadFormat   = pVideoParams->GetPacketFormat();
		m_profile               = eVideoProfileDummy;
		m_bEncodeBFramesInRTV   = pVideoParams->GetIsEncodeRTVBFrame();
	}
	else
	{
		m_videoQcifFrameRate = pVideoParams->GetVideoFrameRate(eVideoResolutionQCIF);
		m_videoCifFrameRate  = pVideoParams->GetVideoFrameRate(eVideoResolutionCIF);
		m_video4CifFrameRate = pVideoParams->GetVideoFrameRate(eVideoResolution4CIF);
		m_videoVGAFrameRate  = pVideoParams->GetVideoFrameRate(eVideoResolutionVGA);
		m_videoSVGAFrameRate = pVideoParams->GetVideoFrameRate(eVideoResolutionSVGA);
		m_videoXGAFrameRate  = pVideoParams->GetVideoFrameRate(eVideoResolutionXGA);

		m_MBPS     = INVALID;
		m_FS       = INVALID;
		m_staticMB = DEFAULT_STATIC_MB;

		m_packetPayloadFormat = eVideoPacketPayloadFormatDummy;
		m_profile             = eVideoProfileDummy;
		m_maxDPB              = INVALID;
		m_bEncodeBFramesInRTV = false;
	}

	m_bUseIntermediateSDResolution = pVideoParams->GetUseIntermediateSDResolution();
	m_dwFrThreshold                = pVideoParams->GetFrThreshold();

	SetMaxVideoParamsAccordingToAllocation(pVideoParams->GetVideoPartyType());

	TRACEINTO << " VP8: after SetMaxVideoParamsAccordingToAllocation m_MBPS: " << (DWORD)m_MBPS << ", m_FS: " << (DWORD)m_FS;

}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoUniDirection::SetMaxVideoParamsAccordingToAllocation(eVideoPartyType videoPartyType)
{
	if (videoPartyType == eVideo_party_type_dummy)
		return;

	Eh264VideoModeType h264VidMode = TranslateCPVideoPartyTypeToMaxH264VideoModeType(videoPartyType);
	CH264VideoMode* pH264VidMode = new CH264VideoMode();
	H264VideoModeDetails h264VidModeDetails;

	if ((h264VidMode == eHD720Asymmetric || h264VidMode == eHD720At60Asymmetric || h264VidMode == eHD1080Asymmetric || h264VidMode == eHD1080At60Asymmetric) && strstr(NameOf(),"Out"))
		pH264VidMode->GetH264AsymmetricTransmitVideoModeDetailsAccordingToType(h264VidModeDetails, h264VidMode);
	else
		pH264VidMode->GetH264VideoModeDetailsAccordingToType(h264VidModeDetails, h264VidMode, m_bIsTipMode);

	POBJDELETE(pH264VidMode);

	CH264Details h264Details(h264VidModeDetails.levelValue);

	m_pMaxVideoParamsAccordingToAllocation = new CBridgePartyVideoParams();

	if (h264VidModeDetails.maxMBPS == -1)
		m_pMaxVideoParamsAccordingToAllocation->SetMBPS(h264Details.GetDefaultMbpsAsDevision());
	else
		m_pMaxVideoParamsAccordingToAllocation->SetMBPS(h264VidModeDetails.maxMBPS);

	if (h264VidModeDetails.maxFS == -1)
		m_pMaxVideoParamsAccordingToAllocation->SetFS(h264Details.GetDefaultFsAsDevision());
	else
		m_pMaxVideoParamsAccordingToAllocation->SetFS(h264VidModeDetails.maxFS);

	m_pMaxVideoParamsAccordingToAllocation->SetVideoPartyType(videoPartyType);
	m_pMaxVideoParamsAccordingToAllocation->SetVidFrameRate(m_videoFrameRate);

	if (m_videoAlg == MS_SVC)
	{
		// romem
		Eh264VideoModeType e264VideoModeType = translateToH264VideoModeVideoPartyType(videoPartyType);
		MsSvcVideoModeDetails MsSvcVideoModeDetails;
		CMsSvcVideoMode::GetMsSvcVideoParamsByMaxH264VideoMode(e264VideoModeType, MsSvcVideoModeDetails);
		if(eInvalidModeType != MsSvcVideoModeDetails.videoModeType)
		{
			m_pMaxVideoParamsAccordingToAllocation->MsSvcParams().nWidth = MsSvcVideoModeDetails.maxWidth;
			m_pMaxVideoParamsAccordingToAllocation->MsSvcParams().nHeight = MsSvcVideoModeDetails.maxHeight;
			m_pMaxVideoParamsAccordingToAllocation->MsSvcParams().aspectRatio = MsSvcVideoModeDetails.aspectRatio;
			m_pMaxVideoParamsAccordingToAllocation->MsSvcParams().maxFrameRate = MsSvcVideoModeDetails.maxFrameRate;
			m_pMaxVideoParamsAccordingToAllocation->MsSvcParams().maxBitRate  = m_msftSvcParamsStruct.maxBitRate;
			m_pMaxVideoParamsAccordingToAllocation->MsSvcParams().ssrc   = m_msftSvcParamsStruct.ssrc;
			m_pMaxVideoParamsAccordingToAllocation->MsSvcParams().pr_id   = m_msftSvcParamsStruct.pr_id;
			TRACEINTO << " MS_SVC params reset to allocation value: " << m_pMaxVideoParamsAccordingToAllocation->MsSvcParams();
		}
		else
			TRACEINTO << " Invalid MS-SVC Video Mode Type";
	}


}

////////////////////////////////////////////////////////////////////////////
bool CBridgePartyVideoUniDirection::AreCurrentVideoParamsFitsToAllocation(bool& isHigherThanAllocated)
{
	TRACEINTO << " PartyName:" << m_pBridgePartyCntl->GetName();
	isHigherThanAllocated = false;

	if (!IsValidPObjectPtr(m_pMaxVideoParamsAccordingToAllocation))
		return true;

	bool ans = true;
	bool show_assert = false;

	std::ostringstream msg;
	msg << PRETTY_FUNCTION << ":\nCurrent Params:";

	DWORD fsAccordingAllocation = m_pMaxVideoParamsAccordingToAllocation->GetFS();
	DWORD mbpsAccordingAllocation = m_pMaxVideoParamsAccordingToAllocation->GetMBPS();
	DWORD currentResolutionRatio = ((CVideoHardwareInterface*)m_pHardwareInterface)->TranslateToVideoResolutionRatio(m_videoAlg,m_videoResolution, m_FS, m_MBPS, m_videoConfType);
	DWORD allocatedResolutionRatio = ((CVideoHardwareInterface*)m_pHardwareInterface)->TranslateToVideoResolutionRatio(H264,eVideoResolutionDummy, fsAccordingAllocation, mbpsAccordingAllocation, m_videoConfType);

	eVideoPartyType currentVideoPartyType   = eVideo_party_type_dummy;
	eVideoPartyType allocatedVideoPartyType = m_pMaxVideoParamsAccordingToAllocation->GetVideoPartyType();
	CSmallString strVideoAlgorithm;

	switch (m_videoAlg)
	{
		case H261:
			strVideoAlgorithm = "H261";
			break;
		case H263:
			strVideoAlgorithm = "H263";
			break;
		case H264:
			strVideoAlgorithm = "H264";
			break;
		case RTV:
			strVideoAlgorithm = "RTV";
			break;
		case MS_SVC:
			strVideoAlgorithm = "MS_SVC";
			break;
		case VP8:
			strVideoAlgorithm = "VP8";
			break;
	}

	if (m_videoAlg == H264 || m_videoAlg == RTV || m_videoAlg == MS_SVC || m_videoAlg == VP8 /*amir K for VP8*/)
	{
		DWORD fsForCalc = m_FS * CUSTOM_MAX_FS_FACTOR;
		DWORD mbpsForCalc = (max(m_staticMB,m_MBPS)) * CUSTOM_MAX_MBPS_FACTOR;

		currentVideoPartyType = ::GetCPH264ResourceVideoPartyType(fsForCalc, mbpsForCalc, (m_videoAlg == RTV));

		msg
			<< "\n  protocol                 :" << strVideoAlgorithm.GetString()
			<< "\n  fs                       :" << m_FS << " (" << m_FS * CUSTOM_MAX_FS_FACTOR << ")"
			<< "\n  mbps                     :" << m_MBPS << " (" << mbpsForCalc << ")"
			<< "\n  resolution ratio         :" << currentResolutionRatio
			<< "\n  video party type         :" << eVideoPartyTypeNames[currentVideoPartyType]
			<< "\n  staticMbps               :" << m_staticMB;
	}
	else
	{
		BYTE is4cif = (m_video4CifFrameRate	!= eVideoFrameRateDUMMY);
		if(m_videoAlg == VIDEO_PROTOCOL_H261 && GetSystemCardsBasedMode() == eSystemCardsMode_mpmrx)
			currentVideoPartyType = eCP_H261_CIF_equals_H264_HD1080_video_party_type;
		else
			currentVideoPartyType = ::GetH261H263ResourcesPartyType(is4cif);

		msg
			<< "\n  protocol                 :" << strVideoAlgorithm.GetString()
			<< "\n  is4CIF                   :" << (int)is4cif
			<< "\n  resolution ratio         :" << currentResolutionRatio
			<< "\n  video party type         :" << eVideoPartyTypeNames[currentVideoPartyType];
	}

	msg
		<< "\nAccording allocation Params:"
		<< "\n  protocol                 :" << strVideoAlgorithm.GetString()
		<< "\n  fs                       :" << fsAccordingAllocation << " (" << fsAccordingAllocation * CUSTOM_MAX_FS_FACTOR << ")"
		<< "\n  mbps                     :" << mbpsAccordingAllocation << " (" << mbpsAccordingAllocation * CUSTOM_MAX_MBPS_FACTOR << ")"
		<< "\n  resolution ratio         :" << allocatedResolutionRatio
		<< "\n  video party type         :" << eVideoPartyTypeNames[allocatedVideoPartyType]
		<< "\n---------------------------";

	if (m_videoAlg == H264 || m_videoAlg == RTV || m_videoAlg == MS_SVC || m_videoAlg == VP8) //Amir7-5
	{
		if (m_FS < fsAccordingAllocation)
		{
			msg << "\nFrame size is lower than allocation --> return FALSE";
			ans = false;
		}
		else if (currentResolutionRatio < allocatedResolutionRatio)
		{
			msg << "\nResolution ratio is lower than allocation --> return FALSE";
			ans = false;
		}
		else if (m_FS > fsAccordingAllocation || currentResolutionRatio > allocatedResolutionRatio || currentVideoPartyType > allocatedVideoPartyType)
		{
			msg << "\nCurrent params are higher than allocation (illegal) --> return FALSE";
			ans = true, show_assert = true;
			isHigherThanAllocated = true;
		}
		else
		{
			msg << "\nCurrent params are same to allocation --> return true";
			isHigherThanAllocated = true;
			ans = true;
		}
	}
	else // H263 / H261
	{
		if (currentResolutionRatio < allocatedResolutionRatio)
		{
			msg << "\nResolution ratio is lower than allocation --> return FALSE";
			ans = false;
		}
		else if (currentVideoPartyType == allocatedVideoPartyType)
		{
			msg << "\nCurrent party type is same to allocation --> return true";
			ans = true;
		}
		else if (currentVideoPartyType < allocatedVideoPartyType)
		{
			msg << "\nCurrent video party type is different to allocation --> return FALSE";
			ans = false;
		}
		else if (currentResolutionRatio > allocatedResolutionRatio)
		{
			msg << "\nResolution ratio is higher than allocation --> return FALSE";
			isHigherThanAllocated = true;
		}
		else
		{
			msg << "\nCurrent params are higher than allocation (illegal) --> return FALSE";
			ans = true, show_assert = true;
			isHigherThanAllocated = true;
		}
	}

	PTRACE(eLevelInfoNormal, msg.str().c_str());
	PASSERTMSG_AND_RETURN_VALUE(show_assert, "Current params are higher than allocation (illegal)", true);//???

	return ans;
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoUniDirection::FixCurrentVideoParamsAccordingToAllocationAndUpdateIfNeeded()
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	bool isHigherThanAllocated = false;
	bool fitsToAllocation = AreCurrentVideoParamsFitsToAllocation(isHigherThanAllocated);

	if (!fitsToAllocation) // it means current < allocation => we open at allocated and update to current
	{
		TRACEINTO << " VP8:in !fitsToAllocation  m_MBPS: " << (DWORD)m_MBPS << ", m_FS: " << (DWORD)m_FS;

		// 1) Fill currentBridgePartyVideoParams with current video params
		// 2) Set current video params according to allocation --> this will be sent at open_port
		// 3) Send UpdateVideoParams with parameters from (1) --> this will be sent as update_encoder / decoder after open_port ack
		CBridgePartyVideoParams savedParams;
		savedParams.SetVideoAlgorithm(m_videoAlg);
		savedParams.SetVideoBitRate(m_videoBitRate);
		savedParams.SetVideoResolutionTableType(m_eResolutionTableType);
		savedParams.SetFS(m_FS);
		savedParams.SetMBPS(m_MBPS);
		savedParams.SetVidFrameRate(m_videoFrameRate);
		savedParams.SetVideoFrameRate(eVideoResolutionQCIF, m_videoQcifFrameRate);
		savedParams.SetVideoFrameRate(eVideoResolutionCIF, m_videoCifFrameRate);
		savedParams.SetVideoFrameRate(eVideoResolution4CIF, m_video4CifFrameRate);
		savedParams.SetVideoFrameRate(eVideoResolutionVGA, m_videoVGAFrameRate);
		savedParams.SetVideoFrameRate(eVideoResolutionSVGA, m_videoSVGAFrameRate);
		savedParams.SetVideoFrameRate(eVideoResolutionXGA, m_videoXGAFrameRate);
		savedParams.SetVideoResolution(m_videoResolution);
		savedParams.SetSampleAspectRatio(m_sampleAspectRatio);
		savedParams.SetStaticMB(m_staticMB);
		savedParams.SetMaxDPB(m_maxDPB);
		savedParams.SetIsVideoClarityEnabled (m_isVideoClarityEnabled);
		savedParams.SetProfile(m_profile);
		savedParams.SetPacketFormat(m_packetPayloadFormat);
		savedParams.SetPartyCascadeMode(m_eCascadeMode);
		savedParams.SetIsTipMode(m_bIsTipMode);
		savedParams.SetIsEncodeRTVBFrame(m_bEncodeBFramesInRTV);
		savedParams.SetUseIntermediateSDResolution(m_bUseIntermediateSDResolution);
		savedParams.SetFrThreshold(m_dwFrThreshold);
		savedParams.MsSvcParams() = m_msftSvcParamsStruct;

		m_MBPS               = m_pMaxVideoParamsAccordingToAllocation->GetMBPS();
		m_FS                 = m_pMaxVideoParamsAccordingToAllocation->GetFS();;
		m_videoFrameRate     = m_pMaxVideoParamsAccordingToAllocation->GetVidFrameRate();
		m_msftSvcParamsStruct = m_pMaxVideoParamsAccordingToAllocation->MsSvcParams();

		m_sampleAspectRatio  = DEFAULT_SAMPLE_ASPECT_RATIO;
		m_staticMB           = DEFAULT_STATIC_MB;
		m_videoQcifFrameRate = eVideoFrameRateDUMMY;
		m_videoCifFrameRate  = eVideoFrameRateDUMMY;
		m_video4CifFrameRate = eVideoFrameRateDUMMY;
		m_videoVGAFrameRate  = eVideoFrameRateDUMMY;
		m_videoSVGAFrameRate = eVideoFrameRateDUMMY;
		m_videoXGAFrameRate  = eVideoFrameRateDUMMY;
		m_videoResolution    = eVideoResolutionDummy;

		if (H263==m_videoAlg || H261==m_videoAlg || (m_FS > 15 && (m_videoAlg == RTV || m_videoAlg == MS_SVC)))
		{
			TRACEINTO << "FS " << m_FS << " is too large for video algorithm " << m_videoAlg << ", resetting to H264";
			// set dummy params for open port - then will be override in update
			m_videoAlg = H264;
			m_profile  = eVideoProfileBaseline;
			m_packetPayloadFormat = eVideoPacketPayloadFormatSingleUnit;
			// currentParams.MsSvcParams().Clear(); // do NOT clear the saved parameters: BRIDGE-11421
		}

		if (m_videoAlg == VP8)
		{
			const DWORD fs = savedParams.GetFS();
			const DWORD mbps = savedParams.GetMBPS();
			if (0 == fs || 0 == mbps)
			{
				TRACEINTO << " saved FS: " << fs << " , saved MBPS: " << mbps;
				PASSERTMSG_AND_RETURN(101, " FS or MBPS is 0");
			}
		}
		TRACEINTO << "Self Send UpdateVideoParams\nSaved ms-svc params:" << savedParams.MsSvcParams() << "\nOpening with allocated:" << m_msftSvcParamsStruct;
		UpdateVideoParams(&savedParams);
	}
	else if (isHigherThanAllocated)
	{
		TRACEINTO << " VP8:in isHigherThanAllocated  m_MBPS: " << (DWORD)m_MBPS << ", m_FS: " << (DWORD)m_FS;

		// error case: we need to set current by allocation
		FixCurrentVideoParamsIfHigherThanAllocation();
	}
}

//------------------------------------------------------------
void CBridgePartyVideoUniDirection::FixCurrentVideoParamsIfHigherThanAllocation()
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	// currently this function is doing only specific bug fix - because we short of time (must be deliverred today ...)
	// VNGR-22728 - open port in H263 4 sif although allocation is h264 sd

	if (!IsValidPObjectPtr(m_pMaxVideoParamsAccordingToAllocation))
		return;

	DWORD fsAccordingAllocation = m_pMaxVideoParamsAccordingToAllocation->GetFS();
	DWORD mbpsAccordingAllocation = m_pMaxVideoParamsAccordingToAllocation->GetMBPS();

	DWORD currentResolutionRatio = ((CVideoHardwareInterface*)m_pHardwareInterface)->TranslateToVideoResolutionRatio(m_videoAlg,m_videoResolution, m_FS, m_MBPS, m_videoConfType);
	DWORD allocatedResolutionRatio = ((CVideoHardwareInterface*)m_pHardwareInterface)->TranslateToVideoResolutionRatio(H264,eVideoResolutionDummy, fsAccordingAllocation, mbpsAccordingAllocation, m_videoConfType);

	eVideoPartyType videoPartyType = eVideo_party_type_dummy;
	eVideoPartyType allocatedVideoPartyType = m_pMaxVideoParamsAccordingToAllocation->GetVideoPartyType();

	if (m_videoAlg == H264 || m_videoAlg == RTV || m_videoAlg == MS_SVC)
	{
		DWORD fsForCalc = m_FS * CUSTOM_MAX_FS_FACTOR;
		DWORD mbpsForCalc = (m_videoAlg == MS_SVC)? m_MBPS : max(m_staticMB,m_MBPS);
		mbpsForCalc *= CUSTOM_MAX_MBPS_FACTOR;

		videoPartyType = ::GetCPH264ResourceVideoPartyType(fsForCalc, mbpsForCalc, (m_videoAlg == RTV));

	}
	else
	{
		BYTE is4cif = (m_video4CifFrameRate	!= eVideoFrameRateDUMMY);
		videoPartyType = ::GetH261H263ResourcesPartyType(is4cif);
	}

	if (m_videoAlg == H264 || m_videoAlg == RTV || m_videoAlg == MS_SVC)
	{
		if(videoPartyType > allocatedVideoPartyType)
		{
			PTRACE(eLevelInfoNormal,"CBridgePartyVideoUniDirection::FixCurrentVideoParamsIfHigherThanAllocation - fixing fs and mbps");
			m_MBPS = mbpsAccordingAllocation;
			m_FS = fsAccordingAllocation;
			m_msftSvcParamsStruct = m_pMaxVideoParamsAccordingToAllocation->MsSvcParams();
		}
	}

	// VNGR-22728 - open port in H263 4 sif although allocation is h264 sd
	if(m_videoAlg == H263)
	{
		if(videoPartyType > allocatedVideoPartyType)
		{
			eVideoPartyType videoPartyType4Cif = eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type;
			if(allocatedVideoPartyType < videoPartyType4Cif && videoPartyType4Cif <= videoPartyType)
			{
				m_video4CifFrameRate  = eVideoFrameRateDUMMY;
				m_videoResolution = eVideoResolutionCIF;
				PTRACE(eLevelInfoNormal,"CBridgePartyVideoUniDirection::FixCurrentVideoParamsIfHigherThanAllocation - remove H263 4CIF");
			}
		}

		if(m_video4CifFrameRate == eVideoFrameRateDUMMY && m_videoResolution == eVideoResolution4CIF)
		{
			m_videoResolution = eVideoResolutionCIF;
			PTRACE(eLevelInfoNormal,"CBridgePartyVideoUniDirection::FixCurrentVideoParamsIfHigherThanAllocation - remove H263 4CIF and Correct it to CIF");
		}
	}
}

void CBridgePartyVideoUniDirection::UpdateNewConfParams(DWORD confRsrcId, const CBridgePartyMediaParams* pMediaParams)
{
	CBridgePartyMediaUniDirection::UpdateNewConfParams(confRsrcId);

	const CBridgePartyVideoParams& params = *reinterpret_cast<const CBridgePartyVideoParams*>(pMediaParams);

	m_videoAlg             = params.GetVideoAlgorithm();
	m_videoBitRate         = params.GetVideoBitRate();
	m_eResolutionTableType = params.GetVideoResolutionTableType();
	m_eCascadeMode         = params.GetPartyCascadeType();
	m_videoResolution      = params.GetVideoResolution();

	MsSvcParamsStruct newMsSvcParams = params.MsSvcParams();

	if (m_videoAlg == H264 || m_videoAlg == MS_SVC)
	{
		m_videoQcifFrameRate = eVideoFrameRateDUMMY;
		m_videoCifFrameRate  = eVideoFrameRateDUMMY;
		m_video4CifFrameRate = eVideoFrameRateDUMMY;
		m_videoVGAFrameRate  = eVideoFrameRateDUMMY;
		m_videoSVGAFrameRate = eVideoFrameRateDUMMY;
		m_videoXGAFrameRate  = eVideoFrameRateDUMMY;

		m_MBPS                = params.GetMBPS();
		m_FS                  = params.GetFS();
		m_staticMB            = params.GetStaticMB();
		m_packetPayloadFormat = params.GetPacketFormat();
		m_profile             = params.GetProfile();
		m_maxDPB              = params.GetMaxDPB();
		m_bIsTipMode          = params.GetIsTipMode();
		m_bEncodeBFramesInRTV = false;
		m_msftSvcParamsStruct = newMsSvcParams;

		TRACEINTO << m_msftSvcParamsStruct;
	}
	else if(m_videoAlg == RTV)
	{
		m_videoQcifFrameRate	= eVideoFrameRateDUMMY;
		m_videoCifFrameRate		= eVideoFrameRateDUMMY;
		m_video4CifFrameRate	= eVideoFrameRateDUMMY;
		m_videoVGAFrameRate     = eVideoFrameRateDUMMY;
		m_videoSVGAFrameRate    = eVideoFrameRateDUMMY;
		m_videoXGAFrameRate     = eVideoFrameRateDUMMY;
		m_MBPS                  = params.GetMBPS();
		m_FS                    = params.GetFS();
		m_staticMB              = DEFAULT_STATIC_MB;
		m_packetPayloadFormat   = params.GetPacketFormat();
		m_profile               = eVideoProfileDummy;
		m_bEncodeBFramesInRTV	= params.GetIsEncodeRTVBFrame();
		m_msftSvcParamsStruct = newMsSvcParams;
		TRACEINTO << m_msftSvcParamsStruct;
	}
	else
	{
		m_videoQcifFrameRate  = params.GetVideoFrameRate(eVideoResolutionQCIF);
		m_videoCifFrameRate   = params.GetVideoFrameRate(eVideoResolutionCIF);
		m_video4CifFrameRate  = params.GetVideoFrameRate(eVideoResolution4CIF);
		m_videoVGAFrameRate   = params.GetVideoFrameRate(eVideoResolutionVGA);
		m_videoSVGAFrameRate  = params.GetVideoFrameRate(eVideoResolutionSVGA);
		m_videoXGAFrameRate   = params.GetVideoFrameRate(eVideoResolutionXGA);
		m_MBPS                = INVALID;
		m_FS                  = INVALID;
		m_staticMB            = DEFAULT_STATIC_MB;
		m_packetPayloadFormat = eVideoPacketPayloadFormatDummy;
		m_profile             = eVideoProfileDummy;
		m_maxDPB              = INVALID;
		m_bIsTipMode          = false;
		m_bEncodeBFramesInRTV = false;
	}

	m_dwFrThreshold = params.GetFrThreshold();
	m_sampleAspectRatio = params.GetSampleAspectRatio();
	m_isVideoClarityEnabled = params.GetIsVideoClarityEnabled();
	m_bUseIntermediateSDResolution = params.GetUseIntermediateSDResolution();
}

////////////////////////////////////////////////////////////////////////////
void  CBridgePartyVideoUniDirection::OnVideoBridgePartyUpdateVideoParamsCONNECTED(CSegment* pParam)
{
	TRACEINTO << "CBridgePartyVideoUniDirection::OnVideoBridgePartyUpdateVideoParamsCONNECTED - ConfName:" << m_pBridgePartyCntl->GetConfName() << ", PartyName:" << m_pBridgePartyCntl->GetName();

	CBridgePartyVideoParams params;
	params.DeSerialize(NATIVE, *pParam);
	SaveAndSendUpdatedVideoParams(&params);
}

////////////////////////////////////////////////////////////////////////////
void  CBridgePartyVideoUniDirection::UpdatePartyParams(CBridgePartyVideoParams* pParams)
{
	m_videoAlg              = pParams->GetVideoAlgorithm();
	m_videoBitRate          = pParams->GetVideoBitRate();
	m_videoResolution       = pParams->GetVideoResolution(); //from cop version the resolution can be relevant to h264 as well
	m_eCascadeMode          = pParams->GetPartyCascadeType();
	m_sampleAspectRatio     = pParams->GetSampleAspectRatio();
	m_isVideoClarityEnabled = pParams->GetIsVideoClarityEnabled();
	m_isH263Plus            = pParams->GetIsH263Plus();
	m_state                 = IDLE;

	const MsSvcParamsStruct& newMsSvcParams = pParams->MsSvcParams();

	TRACEINTO << "ConfName:" << m_pBridgePartyCntl->GetConfName() << ", PartyName:" << m_pBridgePartyCntl->GetName() << ", eCascadeMode:" << (DWORD)m_eCascadeMode << ", videoAlg:" << m_videoAlg;

	if (m_videoAlg == H264 || m_videoAlg == MS_SVC)
	{
		m_videoQcifFrameRate = eVideoFrameRateDUMMY;
		m_videoCifFrameRate  = eVideoFrameRateDUMMY;
		m_video4CifFrameRate = eVideoFrameRateDUMMY;
		m_videoVGAFrameRate  = eVideoFrameRateDUMMY;
		m_videoSVGAFrameRate = eVideoFrameRateDUMMY;
		m_videoXGAFrameRate  = eVideoFrameRateDUMMY;

		m_MBPS                = pParams->GetMBPS();
		m_FS                  = pParams->GetFS();
		m_staticMB            = pParams->GetStaticMB();
		m_packetPayloadFormat = pParams->GetPacketFormat();
		m_profile             = pParams->GetProfile();
		m_msftSvcParamsStruct = newMsSvcParams;

		TRACEINTO << m_msftSvcParamsStruct;
	}
	else if (m_videoAlg == RTV)
	{
		m_videoQcifFrameRate = eVideoFrameRateDUMMY;
		m_videoCifFrameRate  = eVideoFrameRateDUMMY;
		m_video4CifFrameRate = eVideoFrameRateDUMMY;
		m_videoVGAFrameRate  = eVideoFrameRateDUMMY;
		m_videoSVGAFrameRate = eVideoFrameRateDUMMY;
		m_videoXGAFrameRate  = eVideoFrameRateDUMMY;

		m_MBPS                = pParams->GetMBPS();
		m_FS                  = pParams->GetFS();
		m_staticMB            = DEFAULT_STATIC_MB;
		m_packetPayloadFormat = pParams->GetPacketFormat();
		m_profile             = eVideoProfileDummy;
		m_maxDPB              = pParams->GetMaxDPB();
		m_bIsTipMode          = pParams->GetIsTipMode();
		m_bEncodeBFramesInRTV = pParams->GetIsEncodeRTVBFrame();
		m_msftSvcParamsStruct = newMsSvcParams;

		TRACEINTO << m_msftSvcParamsStruct;
	}
	else
	{
		m_videoQcifFrameRate = pParams->GetVideoFrameRate(eVideoResolutionQCIF);
		m_videoCifFrameRate  = pParams->GetVideoFrameRate(eVideoResolutionCIF);
		m_video4CifFrameRate = pParams->GetVideoFrameRate(eVideoResolution4CIF);
		m_videoVGAFrameRate  = pParams->GetVideoFrameRate(eVideoResolutionVGA);
		m_videoSVGAFrameRate = pParams->GetVideoFrameRate(eVideoResolutionSVGA);
		m_videoXGAFrameRate  = pParams->GetVideoFrameRate(eVideoResolutionXGA);

		m_MBPS                = INVALID;
		m_FS                  = INVALID;
		m_staticMB            = DEFAULT_STATIC_MB;
		m_packetPayloadFormat = eVideoPacketPayloadFormatDummy;
		m_profile             = eVideoProfileDummy;
		m_maxDPB              = INVALID;
		m_bIsTipMode          = false;
		m_bEncodeBFramesInRTV = false;
	}

	m_bUseIntermediateSDResolution = pParams->GetUseIntermediateSDResolution();
	m_dwFrThreshold                = pParams->GetFrThreshold();
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoUniDirection::OnVideoBridgePartyUpdateVideoClarity(CSegment* pParam)
{
	WORD tmpIsVideoClarityEnabled = NO;
	*pParam >> tmpIsVideoClarityEnabled;

	if ((BYTE)tmpIsVideoClarityEnabled!= m_isVideoClarityEnabled)
		m_isVideoClarityEnabled = (BYTE)tmpIsVideoClarityEnabled;
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoUniDirection::OnVideoBridgePartyUpdateAutoBrightness(CSegment* pParam)
{
	WORD tmpIsAutoBrightnessEnabled = NO;
	*pParam >> tmpIsAutoBrightnessEnabled;

	if ((BYTE)tmpIsAutoBrightnessEnabled!= m_isAutoBrightness)
		m_isAutoBrightness = (BYTE)tmpIsAutoBrightnessEnabled;
}

////////////////////////////////////////////////////////////////////////////
ECascadePartyType CBridgePartyVideoUniDirection::GetCascadeMode()
{
	return m_eCascadeMode;
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoUniDirection::SetCascadeMode(ECascadePartyType cascadeMode)
{
	TRACEINTO << "oldCascadeMode:" << (DWORD)m_eCascadeMode << ", newCascadeMode:" << (DWORD)cascadeMode;
	m_eCascadeMode = cascadeMode;
}
