#include "BridgePartyVideoParams.h"

#include "H221.h"
#include "H264.h"
#include "H264Util.h"

#include "VideoApiDefinitions.h"
#include "HostCommonDefinitions.h"

#include "COP_ConfParty_Defs.h"

#include "IpCommon.h"
#include "ObjString.h"

#include "ConfPartyGlobals.h"
#include "AvcToSvcParams.h"

#include "TraceStream.h"
#include "PrettyTable.h"

///////////////////////////////////////////////////////////////////////////
std::ostream& operator <<(std::ostream& os, const MsSvcParamsStruct& obj)
{
	return os
		<< "\n ssrc        :" << obj.ssrc
		<< "\n pr_id       :" << (long)obj.pr_id
		<< "\n nWidth      :" << obj.nWidth
		<< "\n nHeight     :" << obj.nHeight
		<< "\n aspectRatio :" << obj.aspectRatio
		<< "\n maxFrameRate:" << obj.maxFrameRate
		<< "\n maxBitRate  :" << obj.maxBitRate;
}

///////////////////////////////////////////////////////////////////////////
CSegment& operator <<(CSegment& seg, const MsSvcParamsStruct& obj)
{
	return seg
		<< obj.ssrc
		<< obj.pr_id
		<< obj.nWidth
		<< obj.nHeight
		<< obj.aspectRatio
		<< obj.maxFrameRate
		<< obj.maxBitRate;
}

///////////////////////////////////////////////////////////////////////////
CSegment& operator >>(CSegment& seg, MsSvcParamsStruct& obj)
{
	return seg
		>> obj.ssrc
		>> obj.pr_id
		>> obj.nWidth
		>> obj.nHeight
		>> obj.aspectRatio
		>> obj.maxFrameRate
		>> obj.maxBitRate;
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator <<(std::ostream& os, const MsFullPacsiInfoStruct& obj)
{
	CPrettyTable<size_t, DWORD, long, DWORD, DWORD, DWORD, DWORD, DWORD> t("#", "ssrc", "pr_id", "width", "height", "aspect ratio", "max framerate", "max bitrate");
	t.SetCaption("Full PACSI Info");

	for (size_t i = 0; i < ARRAYSIZE(obj.pacsiInfo); ++i)
	{
		const MsSvcParamsStruct& pacsi = obj.pacsiInfo[i];
		t.Add(i, pacsi.ssrc, pacsi.pr_id, pacsi.nWidth, pacsi.nHeight, pacsi.aspectRatio, pacsi.maxFrameRate, pacsi.maxBitRate);
	}
	os << t.Get();

	return os;
}

///////////////////////////////////////////////////////////////////////////
CSegment& operator <<(CSegment& seg, const MsFullPacsiInfoStruct& obj)
{
	for (size_t i = 0; i < ARRAYSIZE(obj.pacsiInfo); ++i)
		seg << obj.pacsiInfo[i];

	return seg;
}

///////////////////////////////////////////////////////////////////////////
CSegment& operator >>(CSegment& seg, MsFullPacsiInfoStruct& obj)
{
	for (size_t i = 0; i < ARRAYSIZE(obj.pacsiInfo); ++i)
		seg >> obj.pacsiInfo[i];

	return seg;
}

////////////////////////////////////////////////////////////////////////////
CBridgePartyVideoParams::CBridgePartyVideoParams()
	: CBridgePartyMediaParams()

	, m_videoAlg(0)
	, m_videoBitRate(0)

	, m_videoResolution(eVideoResolutionDummy)

	, m_videoQcifFrameRate(eVideoFrameRateDUMMY)
	, m_videoCifFrameRate(eVideoFrameRateDUMMY)
	, m_video4CifFrameRate(eVideoFrameRateDUMMY)
	, m_videoVGAFrameRate(eVideoFrameRateDUMMY)
	, m_videoSVGAFrameRate(eVideoFrameRateDUMMY)
	, m_videoXGAFrameRate(eVideoFrameRateDUMMY)

	, m_eVideoResolutionTableType(E_VIDEO_RESOLUTION_TABLE_REGULAR)
	, m_profile(eVideoProfileDummy)
	, m_packetFormat(eVideoPacketPayloadFormatDummy)
	, m_videoPartyType(eVideo_party_type_dummy)

	, m_MBPS(0)
	, m_FS(0)
	, m_staticMB(DEFAULT_STATIC_MB)
	, m_sampleAspectRatio(DEFAULT_SAMPLE_ASPECT_RATIO)

	, m_isVideoClarityEnabled(false)

	, m_pAckParams(NULL)

	, m_copConnectionId(DUMMY_CONNECTION_ID)
	, m_XCodeConnectionId(DUMMY_CONNECTION_ID)

	, m_copPartyId(DUMMY_PARTY_ID)
	, m_XCodePartyId(DUMMY_PARTY_ID)

	, m_copResourceIndex(-1)
	, m_XCodeResourceIndex(-1)

	, m_copResourceIndexOfCascadeLinkLecturer(-1)

	, m_videoConfType(eVideoConfTypeDummy)
	, m_videoFrameRate(eVideoFrameRateDUMMY)
	, m_CascadeMode(eCascadeNone)

	, m_dwFrThreshold(0)
	, m_confMediaType(0) // should be replaced with one of the confmediatype
	, m_maxDPB(0)

	, m_isAutoBrightness(false)
	, m_isH263Plus(false)
	, m_isRemoteNeedSmartSwitchAccordingToVendor(false)
	, m_bCascadeIsLecturer(false)
	, m_bIsTipMode(false)
	, m_bUseIntermediateSDResolution(false)
	, m_bIsEncodeRTVBFrame(false)

	, m_pVisualEffects(NULL)
    , m_RemoteIdent(Regular)
{}

////////////////////////////////////////////////////////////////////////////
CBridgePartyVideoParams::CBridgePartyVideoParams(const CBridgePartyVideoParams& obj)
	: CBridgePartyMediaParams(obj)
{
	if (obj.m_pAckParams)
		m_pAckParams = new CAckParams(*obj.m_pAckParams);
	else
		m_pAckParams = NULL;

	m_videoAlg                                 = obj.m_videoAlg;
	m_videoBitRate                             = obj.m_videoBitRate;
	m_videoQcifFrameRate                       = obj.m_videoQcifFrameRate;
	m_videoCifFrameRate                        = obj.m_videoCifFrameRate;
	m_video4CifFrameRate                       = obj.m_video4CifFrameRate;
	m_videoVGAFrameRate                        = obj.m_videoVGAFrameRate;
	m_videoSVGAFrameRate                       = obj.m_videoSVGAFrameRate;
	m_videoXGAFrameRate                        = obj.m_videoXGAFrameRate;
	m_videoResolution                          = obj.m_videoResolution;
	m_MBPS                                     = obj.m_MBPS;
	m_FS                                       = obj.m_FS;
	m_sampleAspectRatio                        = obj.m_sampleAspectRatio;
	m_staticMB                                 = obj.m_staticMB;
	m_isVideoClarityEnabled                    = obj.m_isVideoClarityEnabled;
	m_eVideoResolutionTableType                = obj.m_eVideoResolutionTableType;
	m_copConnectionId                          = obj.m_copConnectionId;
	m_copPartyId                               = obj.m_copPartyId;
	m_copResourceIndex                         = obj.m_copResourceIndex;
	m_videoConfType                            = obj.m_videoConfType;
	m_videoFrameRate                           = obj.m_videoFrameRate;
	m_profile                                  = obj.m_profile;
	m_packetFormat                             = obj.m_packetFormat;
	m_videoPartyType                           = obj.m_videoPartyType;
	m_maxDPB                                   = obj.m_maxDPB;
	m_isAutoBrightness                         = obj.m_isAutoBrightness;
	m_isH263Plus                               = obj.m_isH263Plus;
	m_isRemoteNeedSmartSwitchAccordingToVendor = obj.m_isRemoteNeedSmartSwitchAccordingToVendor;
	m_CascadeMode                              = obj.m_CascadeMode;
	m_copResourceIndexOfCascadeLinkLecturer    = obj.m_copResourceIndexOfCascadeLinkLecturer;
	m_bCascadeIsLecturer                       = obj.m_bCascadeIsLecturer;
	m_bIsTipMode                               = obj.m_bIsTipMode; // TIP
	m_bUseIntermediateSDResolution             = obj.m_bUseIntermediateSDResolution;
	m_bIsEncodeRTVBFrame                       = obj.m_bIsEncodeRTVBFrame;
	m_dwFrThreshold                            = obj.m_dwFrThreshold;
	m_confMediaType                            = obj.m_confMediaType;
	m_tSvcParameters                           = obj.m_tSvcParameters;
    m_RemoteIdent							   = obj.m_RemoteIdent;
}

////////////////////////////////////////////////////////////////////////////
CBridgePartyVideoParams::~CBridgePartyVideoParams()
{
	POBJDELETE(m_pAckParams);
}

////////////////////////////////////////////////////////////////////////////
CBridgePartyVideoParams& CBridgePartyVideoParams::operator =(const CBridgePartyVideoParams& obj)
{
	if (this == &obj)
		return *this;

	CBridgePartyMediaParams::operator=(obj);

	POBJDELETE(m_pAckParams);

	if (obj.m_pAckParams)
		m_pAckParams = new CAckParams(*obj.m_pAckParams);
	else
		m_pAckParams = NULL;

	m_videoAlg                                 = obj.m_videoAlg;
	m_videoBitRate                             = obj.m_videoBitRate;
	m_videoQcifFrameRate                       = obj.m_videoQcifFrameRate;
	m_videoCifFrameRate                        = obj.m_videoCifFrameRate;
	m_video4CifFrameRate                       = obj.m_video4CifFrameRate;
	m_videoVGAFrameRate                        = obj.m_videoVGAFrameRate;
	m_videoSVGAFrameRate                       = obj.m_videoSVGAFrameRate;
	m_videoXGAFrameRate                        = obj.m_videoXGAFrameRate;
	m_videoResolution                          = obj.m_videoResolution;
	m_MBPS                                     = obj.m_MBPS;
	m_FS                                       = obj.m_FS;
	m_sampleAspectRatio                        = obj.m_sampleAspectRatio;
	m_staticMB                                 = obj.m_staticMB;
	m_isVideoClarityEnabled                    = obj.m_isVideoClarityEnabled;
	m_eVideoResolutionTableType                = obj.m_eVideoResolutionTableType;
	m_videoPartyType                           = obj.m_videoPartyType;
	m_copConnectionId                          = obj.m_copConnectionId;
	m_copPartyId                               = obj.m_copPartyId;
	m_copResourceIndex                         = obj.m_copResourceIndex;
	m_videoConfType                            = obj.m_videoConfType;
	m_videoFrameRate                           = obj.m_videoFrameRate;
	m_profile                                  = obj.m_profile;
	m_packetFormat                             = obj.m_packetFormat;
	m_videoPartyType                           = obj.m_videoPartyType;
	m_maxDPB                                   = obj.m_maxDPB;
	m_isAutoBrightness                         = obj.m_isAutoBrightness;
	m_isH263Plus                               = obj.m_isH263Plus;
	m_isRemoteNeedSmartSwitchAccordingToVendor = obj.m_isRemoteNeedSmartSwitchAccordingToVendor;
	m_CascadeMode                              = obj.m_CascadeMode;
	m_copResourceIndexOfCascadeLinkLecturer    = obj.m_copResourceIndexOfCascadeLinkLecturer;
	m_bCascadeIsLecturer                       = obj.m_bCascadeIsLecturer;
	m_bIsTipMode                               = obj.m_bIsTipMode; // TIP
	m_bUseIntermediateSDResolution             = obj.m_bUseIntermediateSDResolution;
	m_bIsEncodeRTVBFrame                       = obj.m_bIsEncodeRTVBFrame;
	m_dwFrThreshold                            = obj.m_dwFrThreshold;
	m_confMediaType                            = obj.m_confMediaType;
	m_tSvcParameters                           = obj.m_tSvcParameters;
    m_RemoteIdent							   = obj.m_RemoteIdent;
	return *this;
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoParams::Serialize(WORD format, CSegment& seg) const
{
	if (format != NATIVE)
		return;

	seg
		<< m_videoAlg
		<< m_videoBitRate
		<< (DWORD)m_videoResolution
		<< (DWORD)m_videoQcifFrameRate
		<< (DWORD)m_videoCifFrameRate
		<< (DWORD)m_video4CifFrameRate
		<< (DWORD)m_videoVGAFrameRate
		<< (DWORD)m_videoSVGAFrameRate
		<< (DWORD)m_videoXGAFrameRate
		<< m_MBPS
		<< m_FS
		<< (DWORD)m_sampleAspectRatio
		<< (DWORD)m_staticMB
		<< (DWORD)m_eVideoResolutionTableType
		<< m_isVideoClarityEnabled
		<< m_copConnectionId
		<< m_copPartyId
		<< m_copResourceIndex
		<< (DWORD)m_videoConfType
		<< (DWORD)m_videoFrameRate
		<< (DWORD)m_profile
		<< (DWORD)m_packetFormat
		<< (DWORD)m_videoPartyType
		<< m_maxDPB
		<< m_isAutoBrightness
		<< m_isH263Plus
		<< m_isRemoteNeedSmartSwitchAccordingToVendor
		<< (BYTE)m_CascadeMode
		<< m_copResourceIndexOfCascadeLinkLecturer
		<< m_bCascadeIsLecturer

		// TIP
		<< m_bIsTipMode
		<< m_bUseIntermediateSDResolution
		<< m_bIsEncodeRTVBFrame
		<< m_dwFrThreshold

		<< m_tSvcParameters
		<< m_tPACSI;

	if (m_pAckParams)
	{
		seg << true;;
		m_pAckParams->Serialize(format, seg);
	}
	else
		seg << false;

	if (m_telepresenseEPInfo)
	{
		seg << true;
		m_telepresenseEPInfo->Serialize(format, seg);
	}
	else
		seg << false << (WORD)m_eTelePresenceMode;
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoParams::DeSerialize(WORD format, CSegment& seg)
{
	if (format != NATIVE)
		return;

	seg
		>> m_videoAlg
		>> m_videoBitRate
		>> (DWORD&)m_videoResolution
		>> (DWORD&)m_videoQcifFrameRate
		>> (DWORD&)m_videoCifFrameRate
		>> (DWORD&)m_video4CifFrameRate
		>> (DWORD&)m_videoVGAFrameRate
		>> (DWORD&)m_videoSVGAFrameRate
		>> (DWORD&)m_videoXGAFrameRate
		>> m_MBPS
		>> m_FS
		>> m_sampleAspectRatio
		>> m_staticMB
		>> (DWORD&)m_eVideoResolutionTableType
		>> m_isVideoClarityEnabled
		>> m_copConnectionId
		>> m_copPartyId
		>> m_copResourceIndex
		>> (DWORD&)m_videoConfType
		>> (DWORD&)m_videoFrameRate
		>> (DWORD&)m_profile
		>> (DWORD&)m_packetFormat
		>> (DWORD&)m_videoPartyType
		>> m_maxDPB
		>> m_isAutoBrightness
		>> m_isH263Plus
		>> m_isRemoteNeedSmartSwitchAccordingToVendor;

	BYTE tmp;
	seg >> tmp;
	m_CascadeMode = (ECascadePartyType)tmp;

	seg
		>> m_copResourceIndexOfCascadeLinkLecturer
		>> m_bCascadeIsLecturer

		>> m_bIsTipMode
		>> m_bUseIntermediateSDResolution
		>> m_bIsEncodeRTVBFrame
		>> m_dwFrThreshold

		>> m_tSvcParameters
		>> m_tPACSI;

	bool hasData;

	seg >> hasData;
	if (hasData)
	{
		m_pAckParams = new CAckParams;
		m_pAckParams->DeSerialize(format, seg);
	}

	seg >> hasData;
	if (hasData)
	{
		if (!m_telepresenseEPInfo)
			m_telepresenseEPInfo = new CTelepresenseEPInfo;

		m_telepresenseEPInfo->DeSerialize(format, seg);
		m_eTelePresenceMode = m_telepresenseEPInfo->GetEPtype();
	}
	else
		seg >> (WORD&)m_eTelePresenceMode;
}

////////////////////////////////////////////////////////////////////////////
BOOL CBridgePartyVideoParams::IsValidParams() const
{
	if (!CBridgePartyVideoParams::IsValidVideoAlgorithm(m_videoAlg))
	{
		TRACEINTO << "Invalid Video Algorithm: "<< m_videoAlg;
		return false;
	}

	if (false == CBridgePartyVideoParams::IsValidVideoBitRate(m_videoBitRate, m_videoConfType))
	{
		TRACEINTO << "Invalid Video Bit Rate: "<< m_videoBitRate;
		return false;
	}

	if (m_videoAlg == H264 || m_videoAlg == RTV)
	{
		if (!CBridgePartyVideoParams::IsValidMBPS(m_MBPS))
		{
			TRACEINTO << "Invalid MBPS: "<< m_MBPS;
			return false;
		}

		if (!CBridgePartyVideoParams::IsValidFS(m_FS))
		{
			TRACEINTO << "Invalid FS: "<< m_FS;
			return false;
		}

		if (!CBridgePartyVideoParams::IsValidSampleAspectRatio(m_sampleAspectRatio))
		{
			TRACEINTO << "Invalid SampleAspectRatio: "<< m_sampleAspectRatio;
			return false;
		}

		if (!CBridgePartyVideoParams::IsValidStaticMB(m_staticMB))
		{
			TRACEINTO << "Invalid staticMB: "<< m_staticMB;
			return false;
		}

		if (m_videoAlg == H264 && !CBridgePartyVideoParams::IsValidMaxDPB(m_maxDPB))
		{
			TRACEINTO << "Invalid maxDPB: "<< m_maxDPB;
			return false;
		}
	}
	else if (m_videoAlg == MS_SVC)
	{
		if (!CBridgePartyVideoParams::IsValidSampleAspectRatio(m_sampleAspectRatio))
		{
			TRACEINTO << "Invalid SampleAspectRatio: "<< m_sampleAspectRatio;
			return false;
		}

		if (!CBridgePartyVideoParams::IsValidStaticMB(m_staticMB))
		{
			TRACEINTO << "Invalid staticMB: "<< m_staticMB;
			return false;
		}

		if (!CBridgePartyVideoParams::IsValidVideoFrameRate(m_videoFrameRate))
		{
			TRACEINTO << "Invalid Cif Video Frame Rate: "<< m_videoCifFrameRate;
		}
	}
	else
	{
		bool isValidFrameRate = false;

		if (!CBridgePartyVideoParams::IsValidVideoResolution(m_videoResolution))
		{
			TRACEINTO << "Invalid Video Resolution: "<< m_videoResolution;
			return false;
		}

		if (!CBridgePartyVideoParams::IsValidVideoFrameRate(m_videoQcifFrameRate))
		{
			TRACEINTO << "Invalid QCif Video Frame Rate: "<< m_videoQcifFrameRate;
		}
		else
			isValidFrameRate = true;

		if (!CBridgePartyVideoParams::IsValidVideoFrameRate(m_videoCifFrameRate))
		{
			TRACEINTO << " CBridgePartyVideoParams::IsValidParams - Invalid Cif Video Frame Rate: "<< m_videoCifFrameRate;
		}
		else
			isValidFrameRate = true;

		if (!CBridgePartyVideoParams::IsValidVideoFrameRate(m_video4CifFrameRate))
		{
			TRACEINTO << "Invalid 4Cif Video Frame Rate: "<< m_video4CifFrameRate;
		}
		else
			isValidFrameRate = true;

		if (!CBridgePartyVideoParams::IsValidVideoFrameRate(m_videoVGAFrameRate))
		{
			TRACEINTO << "Invalid VGA Video Frame Rate: "<< m_videoVGAFrameRate;
		}
		else
			isValidFrameRate = true;

		if (!CBridgePartyVideoParams::IsValidVideoFrameRate(m_videoSVGAFrameRate))
		{
			TRACEINTO << "Invalid SVGA Video Frame Rate: "<< m_videoSVGAFrameRate;
		}
		else
			isValidFrameRate = true;

		if (!CBridgePartyVideoParams::IsValidVideoFrameRate(m_videoXGAFrameRate))
		{
			TRACEINTO << "Invalid XGA Video Frame Rate: "<< m_videoXGAFrameRate;
		}
		else
			isValidFrameRate = true;

		if (!isValidFrameRate)
			return false; // if no valid frame rate found return false.
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////
bool CBridgePartyVideoParams::IsValidVideoAlgorithm(DWORD dwVideoAlgorithm)
{
	switch (dwVideoAlgorithm)
	{
		case H261:
		case H263:
		case RTV:
		case H264:
		case MS_SVC:
		case VP8://N.A. DEBUG VP8
			return true;

		default:
			return false;
	}
}

////////////////////////////////////////////////////////////////////////////
bool CBridgePartyVideoParams::IsValidVideoBitRate(DWORD dwBitRate, eVideoConfType videoConfType)
{
	// Range is (0, 6M]
	if (!dwBitRate || dwBitRate > 6144000)
		return false;

	//for non-CP mode, we support up to 6144K
	//for CP mode, MPMRX cards supports upto 6144K, others up to 4096K
	return (eVideoConfTypeCP != videoConfType || IsFeatureSupportedBySystem(eFeatureLineRate_6M) || dwBitRate <= 4096000);
}

////////////////////////////////////////////////////////////////////////////
bool CBridgePartyVideoParams::IsValidVideoResolution(eVideoResolution resolution)
{
	switch (resolution)
	{
		case eVideoResolutionQCIF:
		case eVideoResolutionCIF:
		case eVideoResolutionVGA:
		case eVideoResolution4SIF:
		case eVideoResolution4CIF:
		case eVideoResolution525SD:
		case eVideoResolution625SD:
		case eVideoResolutionSVGA:
		case eVideoResolutionXGA:
		case eVideoResolutionHD720:
		case eVideoResolution16CIF:
		case eVideoResolutionSIF:
		case eVideoResolutionQVGA:
		case eVideoResolutionHD1080:
			return true;

		default:
			return false;
	}
}

////////////////////////////////////////////////////////////////////////////
bool CBridgePartyVideoParams::IsValidVideoFrameRate(eVideoFrameRate frameRate)
{
	switch (frameRate)
	{
		case eVideoFrameRate60FPS:
		case eVideoFrameRate50FPS:
		case eVideoFrameRate30FPS:
		case eVideoFrameRate25FPS:
		case eVideoFrameRate15FPS:
		case eVideoFrameRate12_5FPS:
		case eVideoFrameRate10FPS:
		case eVideoFrameRate7_5FPS:
		case eVideoFrameRate6FPS:
		case eVideoFrameRate5FPS:
		case eVideoFrameRate3FPS:
			return true;

		default:
			return false;
	}
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoParams::InitPartyVideoParams(const CBridgePartyVideoParams* pParams)
{
	m_videoAlg                              = pParams->GetVideoAlgorithm();
	m_videoBitRate                          = pParams->GetVideoBitRate();
	m_videoQcifFrameRate                    = pParams->GetVideoFrameRate(eVideoResolutionQCIF);
	m_videoCifFrameRate                     = pParams->GetVideoFrameRate(eVideoResolutionCIF);
	m_video4CifFrameRate                    = pParams->GetVideoFrameRate(eVideoResolution4CIF);
	m_videoVGAFrameRate                     = pParams->GetVideoFrameRate(eVideoResolutionVGA);
	m_videoSVGAFrameRate                    = pParams->GetVideoFrameRate(eVideoResolutionSVGA);
	m_videoXGAFrameRate                     = pParams->GetVideoFrameRate(eVideoResolutionXGA);
	m_videoResolution                       = pParams->GetVideoResolution();
	m_MBPS                                  = pParams->GetMBPS();
	m_FS                                    = pParams->GetFS();
	m_sampleAspectRatio                     = pParams->GetSampleAspectRatio();
	m_staticMB                              = pParams->GetStaticMB();
	m_isVideoClarityEnabled                 = pParams->GetIsVideoClarityEnabled();
	m_copConnectionId                       = pParams->GetCopConnectionId();
	m_copPartyId                            = pParams->GetCopPartyId();
	m_copResourceIndex                      = pParams->GetCopResourceIndex();
	m_videoConfType                         = pParams->GetVideoConfType();
	m_videoFrameRate                        = pParams->GetVidFrameRate();
	m_profile                               = pParams->GetProfile();
	m_packetFormat                          = pParams->GetPacketFormat();
	m_maxDPB                                = pParams->GetMaxDPB();
	m_CascadeMode                           = pParams->GetPartyCascadeType();
	m_copResourceIndexOfCascadeLinkLecturer = pParams->GetCopResourceOfLecturerLinkIndex();
	m_bCascadeIsLecturer                    = pParams->GetIsCopLinkLecturer();
	m_bUseIntermediateSDResolution          = pParams->GetUseIntermediateSDResolution();
	m_bIsEncodeRTVBFrame                    = pParams->GetIsEncodeRTVBFrame();
	m_dwFrThreshold                         = pParams->GetFrThreshold();

	SetTelePresenceEPInfo(pParams->GetTelePresenceEPInfo());
	SetTelePresenceMode(pParams->GetTelePresenceMode());

	m_confMediaType                         = pParams->m_confMediaType;
	m_tSvcParameters                        = pParams->MsSvcParams();
  m_RemoteIdent							    = pParams->GetRemoteIdent();
}

////////////////////////////////////////////////////////////////////////////
bool CBridgePartyVideoParams::IsValidMBPS(DWORD mbps)
{
	// The division operation is heavy, (H264_L5_1_DEFAULT_MBPS/CUSTOM_MAX_MBPS_FACTOR)+(H264_L5_1_DEFAULT_MBPS%CUSTOM_MAX_MBPS_FACTOR? 1: 0);
	const DWORD maxMBPS = 1967;

	return ((mbps <= (WORD)maxMBPS) && (0 < mbps));
}

////////////////////////////////////////////////////////////////////////////
bool CBridgePartyVideoParams::IsValidFS(DWORD fs)
{
	// The division operation is heavy, (H264_L5_1_DEFAULT_FS/CUSTOM_MAX_FS_FACTOR)+(H264_L5_1_DEFAULT_FS%CUSTOM_MAX_FS_FACTOR? 1: 0);
	const DWORD maxFS = 144;

	return ((fs <= (WORD)maxFS) && (0 < fs));
}

////////////////////////////////////////////////////////////////////////////
bool CBridgePartyVideoParams::IsValidSampleAspectRatio(DWORD sampleAspectRatio)
{
	return sampleAspectRatio <= 255;
}

////////////////////////////////////////////////////////////////////////////
bool CBridgePartyVideoParams::IsValidStaticMB(DWORD staticMB)
{
	// The division operation is heavy, (H264_L5_1_DEFAULT_MBPS/CUSTOM_MAX_MBPS_FACTOR)+(H264_L5_1_DEFAULT_MBPS%CUSTOM_MAX_MBPS_FACTOR? 1: 0);
	const DWORD maxStaticMB = 1967;

	return (staticMB <= (WORD)maxStaticMB);
}

////////////////////////////////////////////////////////////////////////////
eVideoFrameRate CBridgePartyVideoParams::GetVideoFrameRate(eVideoResolution resolution) const
{
	switch (resolution)
	{
		case eVideoResolutionQCIF: return m_videoQcifFrameRate;
		case eVideoResolutionCIF:  return m_videoCifFrameRate;
		case eVideoResolution4CIF: return m_video4CifFrameRate;
		case eVideoResolutionVGA:  return m_videoVGAFrameRate;
		case eVideoResolutionSVGA: return m_videoSVGAFrameRate;
		case eVideoResolutionXGA:  return m_videoXGAFrameRate;

		default:
			PASSERT(1);
			return eVideoFrameRateDUMMY;
	}
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoParams::SetVideoFrameRate(eVideoResolution resolution, eVideoFrameRate videoFrameRate)
{
	switch (resolution)
	{
		case  eVideoResolutionQCIF: m_videoQcifFrameRate  = videoFrameRate; break;
		case  eVideoResolutionCIF : m_videoCifFrameRate   = videoFrameRate; break;
		case  eVideoResolution4CIF: m_video4CifFrameRate  = videoFrameRate; break;
		case  eVideoResolutionVGA : m_videoVGAFrameRate   = videoFrameRate; break;
		case  eVideoResolutionSVGA: m_videoSVGAFrameRate  = videoFrameRate; break;
		case  eVideoResolutionXGA : m_videoXGAFrameRate   = videoFrameRate; break;

		default:
			PASSERT(1);
			break;
	}
}

////////////////////////////////////////////////////////////////////////////-
void CBridgePartyVideoParams::SetLPRParams(DWORD lossProtection, DWORD mtbf, DWORD congestionCeiling, DWORD fill, DWORD modeTimeout)
{
	if (!(CPObject::IsValidPObjectPtr(m_pAckParams)))
		InitAckParams();
	m_pAckParams->SetLPRParams(lossProtection, mtbf, congestionCeiling, fill, modeTimeout);
}

////////////////////////////////////////////////////////////////////////////--
void CBridgePartyVideoParams::InitAckParams()
{
	POBJDELETE(m_pAckParams);
	m_pAckParams = new CAckParams;
}

////////////////////////////////////////////////////////////////////////////--
void CBridgePartyVideoParams::SetVidFrameRate(eVideoFrameRate videoFrameRate)
{
	m_videoFrameRate = videoFrameRate;
	if ((m_videoAlg == MS_SVC) && (m_videoFrameRate == eVideoFrameRateDUMMY))
		TRACEINTO << "MS_SVC video frame rate is set to 0!";
}

////////////////////////////////////////////////////////////////////////////
// this function set only video params - to enable copy between in <--> out
void CBridgePartyVideoParams::SetOnlyVideoParams(const CBridgePartyVideoParams* pParams)
{
	m_videoAlg                                 = pParams->GetVideoAlgorithm();
	m_videoBitRate                             = pParams->GetVideoBitRate();
	m_videoQcifFrameRate                       = pParams->GetVideoFrameRate(eVideoResolutionQCIF);
	m_videoCifFrameRate                        = pParams->GetVideoFrameRate(eVideoResolutionCIF);
	m_video4CifFrameRate                       = pParams->GetVideoFrameRate(eVideoResolution4CIF);
	m_videoVGAFrameRate                        = pParams->GetVideoFrameRate(eVideoResolutionVGA);
	m_videoSVGAFrameRate                       = pParams->GetVideoFrameRate(eVideoResolutionSVGA);
	m_videoXGAFrameRate                        = pParams->GetVideoFrameRate(eVideoResolutionXGA);
	m_videoResolution                          = pParams->GetVideoResolution();
	m_MBPS                                     = pParams->GetMBPS();
	m_FS                                       = pParams->GetFS();
	m_sampleAspectRatio                        = pParams->GetSampleAspectRatio();
	m_staticMB                                 = pParams->GetStaticMB();
	m_isVideoClarityEnabled                    = pParams->GetIsVideoClarityEnabled();
	m_videoConfType                            = pParams->GetVideoConfType();
	m_videoFrameRate                           = pParams->GetVidFrameRate();
	m_profile                                  = pParams->GetProfile();
	m_packetFormat                             = pParams->GetPacketFormat();
	m_maxDPB                                   = pParams->GetMaxDPB();
	m_isAutoBrightness                         = pParams->GetIsAutoBrightness();
	m_isH263Plus                               = pParams->GetIsH263Plus();
	m_isRemoteNeedSmartSwitchAccordingToVendor = pParams->GetIsRemoteNeedSmartSwitchAccordingToVendor();
	m_CascadeMode                              = pParams->GetPartyCascadeType();
	m_bIsTipMode                               = pParams->GetIsTipMode();
	m_bUseIntermediateSDResolution             = pParams->GetUseIntermediateSDResolution();
	m_bIsEncodeRTVBFrame                       = pParams->GetIsEncodeRTVBFrame();
	m_dwFrThreshold                            = pParams->GetFrThreshold();
    m_RemoteIdent							   = pParams->GetRemoteIdent();
}

////////////////////////////////////////////////////////////////////////////
// set only video params - to enable copy between in <--> out
void CBridgePartyVideoParams::DumpVideoParams()
{
	CMedString mstr;
	mstr << "m_videoAlg = ";
	if (m_videoAlg == H264)
	{
		mstr << "H264";
	}
	else if (m_videoAlg == H263)
	{
		mstr << "H263";
	}
	else if (m_videoAlg == H261)
	{
		mstr << "H261";
	}
	else if (m_videoAlg == SVC)
	{
		mstr << "SVC";
	}
	else if (m_videoAlg == MS_SVC)
	{
		mstr << "MS_SVC";
	}
	else
	{
		mstr << "Unknown";
	}

	mstr << "\n";

	mstr << "m_videoBitRate = " << (DWORD)m_videoBitRate/1000 << "\n";

	mstr << "m_MBPS = " << (DWORD)m_MBPS << "\n";
	mstr << "m_FS = " << (DWORD)m_FS << "\n";

	mstr << "m_videoResolution = ";
	if (m_videoResolution == eVideoResolutionDummy)
	{
		mstr << "eVideoResolutionDummy";
	}
	else if (m_videoResolution == eVideoResolutionQCIF)
	{
		mstr << "eVideoResolutionQCIF";
	}
	else if (m_videoResolution == eVideoResolutionCIF)
	{
		mstr << "eVideoResolutionCIF";
	}
	else if (m_videoResolution == eVideoResolutionVGA)
	{
		mstr << "eVideoResolutionVGA";
	}
	else if (m_videoResolution == eVideoResolution4SIF)
	{
		mstr << "eVideoResolution4SIF";
	}
	else if (m_videoResolution == eVideoResolution4CIF)
	{
		mstr << "eVideoResolution4CIF";
	}
	else if (m_videoResolution == eVideoResolution525SD)
	{
		mstr << "eVideoResolution525SD";
	}
	else if (m_videoResolution == eVideoResolution625SD)
	{
		mstr << "eVideoResolution625SD";
	}
	else if (m_videoResolution == eVideoResolutionSVGA)
	{
		mstr << "eVideoResolutionSVGA";
	}
	else if (m_videoResolution == eVideoResolutionXGA)
	{
		mstr << "eVideoResolutionXGA";
	}
	else if (m_videoResolution == eVideoResolutionHD720)
	{
		mstr << "eVideoResolutionHD720";
	}
	else if (m_videoResolution == eVideoResolution16CIF)
	{
		mstr << "eVideoResolution16CIF";
	}
	else if (m_videoResolution == eVideoResolutionSIF)
	{
		mstr << "eVideoResolutionSIF";
	}
	else if (m_videoResolution == eVideoResolutionQVGA)
	{
		mstr << "eVideoResolutionQVGA";
	}
	else if (m_videoResolution == eVideoResolutionHD1080)
	{
		mstr << "eVideoResolutionHD1080";
	}
	else
	{
		mstr << "Unknown";
	}

	mstr << "\n";

	mstr << "m_sampleAspectRatio = " << (DWORD)m_sampleAspectRatio << "\n";

	mstr << "m_videoQcifFrameRate = " << (DWORD)m_videoQcifFrameRate << "\n";
	mstr << "m_videoCifFrameRate = " << (DWORD)m_videoCifFrameRate << "\n";
	mstr << "m_video4CifFrameRate = " << (DWORD)m_video4CifFrameRate << "\n";
	mstr << "m_videoVGAFrameRate = " << (DWORD)m_videoVGAFrameRate << "\n";
	mstr << "m_videoSVGAFrameRate = " << (DWORD)m_videoSVGAFrameRate << "\n";
	mstr << "m_videoXGAFrameRate = " << (DWORD)m_videoXGAFrameRate << "\n";
	mstr << "m_videoResolution = " << (DWORD)m_videoResolution << "\n";
	mstr << "m_staticMB = " << (DWORD)m_staticMB << "\n";
	mstr << "m_isVideoClarityEnabled = " << (DWORD)m_isVideoClarityEnabled << "\n";
	mstr << "m_videoConfType = " ;
    
	if (m_videoConfType == eVideoConfTypeDummy)
	{
		mstr << "eVideoConfTypeDummy";
	}
	else if (m_videoConfType == eVideoConfTypeCP)
	{
		mstr << "eVideoConfTypeCP";
	}
	else if (m_videoConfType == eVideoConfTypeVSW)
	{
		mstr << "eVideoConfTypeVSW";
	}
	else if (m_videoConfType == eVideoConfTypeCopHD108025fps)
	{
		mstr << "eVideoConfTypeCopHD108025fps";
	}
	else if (m_videoConfType == eVideoConfTypeCopHD72050fps)
	{
		mstr << "eVideoConfTypeCopHD72050fps";
	}
	else
	{
		mstr << "Unknown";
	}

	mstr << "\n";

	mstr << "m_videoFrameRate = " << (DWORD)m_videoFrameRate << "\n";
	if (m_profile == eVideoProfileDummy)
	{
		mstr << "eVideoProfileDummy\n";
	}
	else if (m_profile == eVideoProfileBaseline)
	{
		mstr << "eVideoProfileBaseline\n";
	}
	else if (m_profile == eVideoProfileHigh)
	{
		mstr << "eVideoProfileHigh\n";
	}
	else if (m_profile == eVideoProfileMain)
	{
		mstr << "eVideoProfileMain";
	}
	else
	{
		mstr << "Unknown";
	}

	if (m_packetFormat == eVideoPacketPayloadFormatDummy)
	{
		mstr << "eVideoPacketPayloadFormatDummy\n";
	}
	else if (m_packetFormat == eVideoPacketPayloadFormatSingleUnit)
	{
		mstr << "eVideoPacketPayloadFormatSingleUnit\n";
	}
	else if (m_packetFormat == eVideoPacketPayloadFormatFragmentationUnit)
	{
		mstr << "eVideoPacketPayloadFormatFragmentationUnit\n";
	}
	else
	{
		mstr << "Unknown";
	}

	mstr << "m_maxDPB = " << (DWORD)m_maxDPB << "\n";
	mstr << "m_bUseIntermediateSDResolution = " << (WORD) m_bUseIntermediateSDResolution << "\n";
	mstr << "m_bIsEncodeRTVBFrame = " << (WORD) m_bIsEncodeRTVBFrame << "\n";
	mstr << "m_dwFrThreshold = " << (WORD) m_dwFrThreshold  << "\n";
	mstr << "m_RemoteIdent = " << (DWORD)m_RemoteIdent << "\n";
	
	PTRACE2(eLevelInfoNormal, "CBridgePartyVideoParams::DumpVideoParams:\n", mstr.GetString());
}


////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoParams::SetIsRemoteNeedSmartSwitchAccordingToVendor(BYTE isRemotNeedSmartSwitchAccordingToVendor)
{
	m_isRemoteNeedSmartSwitchAccordingToVendor = isRemotNeedSmartSwitchAccordingToVendor;
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoParams::GetMaxMBPSAndFSFromH263H261Params(DWORD& maxFS, DWORD& maxMBPS)
{
	PASSERTSTREAM_AND_RETURN(m_videoAlg != H263 && m_videoAlg != H261, "Unexpected VideoAlg:" << m_videoAlg);

	if (m_videoResolution == eVideoResolution4CIF)
	{
		maxFS = H264_L2_2_DEFAULT_FS;
		if ((m_videoFrameRate == eVideoFrameRate30FPS) || (m_videoFrameRate == eVideoFrameRate25FPS))
		{
			maxMBPS = H264_SD_30_MBPS;
		}
		else if ((m_videoFrameRate == eVideoFrameRate15FPS) || (m_videoFrameRate == eVideoFrameRate12_5FPS))
		{
			maxMBPS = H264_L2_2_DEFAULT_MBPS;
		}
		else
			PASSERT(m_videoFrameRate);
	}
	else if (m_videoResolution == eVideoResolutionCIF)
	{
		maxFS = H264_L1_2_DEFAULT_FS;
		if ((m_videoFrameRate == eVideoFrameRate30FPS) || (m_videoFrameRate == eVideoFrameRate25FPS))
		{
			maxMBPS = H264_CIF_25_MBPS;
		}
		else if ((m_videoFrameRate == eVideoFrameRate15FPS) || (m_videoFrameRate == eVideoFrameRate12_5FPS))
		{
			maxMBPS = H264_L1_2_DEFAULT_MBPS;
		}
		else
			PASSERT(m_videoFrameRate);
	}
	else if (m_videoResolution == eVideoResolutionQCIF)
	{
		maxFS = H264_L1_1_DEFAULT_FS;
		if ((m_videoFrameRate == eVideoFrameRate30FPS) || (m_videoFrameRate == eVideoFrameRate25FPS))
		{
			maxMBPS = H264_L1_DEFAULT_MBPS;
		}
		else if ((m_videoFrameRate == eVideoFrameRate15FPS) || (m_videoFrameRate == eVideoFrameRate12_5FPS))
		{
			maxMBPS = H264_L1_1_DEFAULT_MBPS;
		}
		else
			PASSERT(m_videoFrameRate);
	}
	else
		PASSERT(m_videoResolution);

	maxFS   = GetMaxFsAsDevision(maxFS);
	maxMBPS = GetMaxMbpsAsDevision(maxMBPS);
}

////////////////////////////////////////////////////////////////////////////
CBridgePartyVideoInParams::CBridgePartyVideoInParams ()
	: CBridgePartyVideoParams()
{
	m_SiteName[0] = '\0';
	m_backgroundImageID = 0;
	m_pAvcToSvcParams = NULL;
}

////////////////////////////////////////////////////////////////////////////
CBridgePartyVideoInParams::CBridgePartyVideoInParams(
	DWORD vidAlg, DWORD vidBitRate,
	eVideoFrameRate videoQCifFrameRate, eVideoFrameRate videoCifFrameRate, eVideoFrameRate video4CifFrameRate,
	eVideoResolution videoResolution, DWORD mbps, DWORD fs,
	const char* pSiteName,
	CDwordBitMask muteMask,
	eTelePresencePartyType eTelePresenceMode,
	DWORD backgroundImageID,
	bool isAutoBrightness,
	eVideoFrameRate videoVGAFrameRate, eVideoFrameRate videoSVGAFrameRate, eVideoFrameRate videoXGAFrameRate,
	BYTE isRemoteNeedSmartSwitchAccordToVendor,
	ECascadePartyType cascadeMode /* = eCascadeNone*/)
{
	if (pSiteName)
	{
		strncpy(m_SiteName, pSiteName, MAX_SITE_NAME_ARR_SIZE - 1);
		m_SiteName[MAX_SITE_NAME_ARR_SIZE - 1] = '\0';
	}
	else
		m_SiteName[0] = '\0';

	m_videoAlg           = vidAlg;
	m_videoBitRate       = vidBitRate;
	m_videoQcifFrameRate = videoQCifFrameRate;
	m_videoCifFrameRate  = videoCifFrameRate;
	m_video4CifFrameRate = video4CifFrameRate;
	m_videoVGAFrameRate  = videoVGAFrameRate;
	m_videoSVGAFrameRate = videoSVGAFrameRate;
	m_videoXGAFrameRate  = videoXGAFrameRate;
	m_videoResolution    = videoResolution;
	m_MBPS               = mbps;
	m_FS                 = fs;
	m_mute_mask          = muteMask;

//	if (!m_telepresenseEPInfo)
//		m_telepresenseEPInfo = new CTelepresenseEPInfo;

	SetTelePresenceMode(eTelePresenceMode);

	m_isAutoBrightness                         = isAutoBrightness;
	m_backgroundImageID                        = backgroundImageID;
	m_isRemoteNeedSmartSwitchAccordingToVendor = isRemoteNeedSmartSwitchAccordToVendor;
	m_CascadeMode                              = cascadeMode;
	m_pAvcToSvcParams                          = NULL;
}

////////////////////////////////////////////////////////////////////////////
CBridgePartyVideoInParams::CBridgePartyVideoInParams(const CBridgePartyVideoInParams& obj)
	: CBridgePartyVideoParams(obj)
{
	strncpy(m_SiteName, obj.m_SiteName, MAX_SITE_NAME_ARR_SIZE);

	m_mute_mask                                = obj.m_mute_mask;
	m_backgroundImageID                        = obj.m_backgroundImageID;
	m_isRemoteNeedSmartSwitchAccordingToVendor = obj.m_isRemoteNeedSmartSwitchAccordingToVendor;

	if (obj.m_pAvcToSvcParams)
		m_pAvcToSvcParams = new CAvcToSvcParams(*obj.m_pAvcToSvcParams);
	else
		m_pAvcToSvcParams = NULL;
}

////////////////////////////////////////////////////////////////////////////
CBridgePartyVideoInParams::~CBridgePartyVideoInParams()
{
	PDELETE(m_pAvcToSvcParams);
}

////////////////////////////////////////////////////////////////////////////
CBridgePartyVideoInParams& CBridgePartyVideoInParams::operator =(const CBridgePartyVideoInParams& rOtherBridgePartyVideoParams)
{
	if (this == &rOtherBridgePartyVideoParams)
		return *this;

	CBridgePartyVideoParams::operator=(rOtherBridgePartyVideoParams);

	strncpy(m_SiteName, rOtherBridgePartyVideoParams.m_SiteName, MAX_SITE_NAME_ARR_SIZE);

	m_mute_mask         = rOtherBridgePartyVideoParams.m_mute_mask;
	m_backgroundImageID = rOtherBridgePartyVideoParams.m_backgroundImageID;

	if (rOtherBridgePartyVideoParams.m_pAvcToSvcParams)
		m_pAvcToSvcParams = new CAvcToSvcParams(*rOtherBridgePartyVideoParams.m_pAvcToSvcParams);
	else
		m_pAvcToSvcParams = NULL;

	return *this;
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoInParams::SetSiteName(const char* pSiteName)
{
	if (pSiteName)
	{
		strncpy(m_SiteName, pSiteName, sizeof(m_SiteName) - 1);
		m_SiteName[sizeof(m_SiteName) - 1] = 0;
	}
	else
		m_SiteName[0] = 0;
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoInParams::InitPartyVideoInParams(const CBridgePartyVideoParams* pBridgePartyMediaParams)
{
	InitPartyVideoParams(pBridgePartyMediaParams);

	SetSiteName((((CBridgePartyVideoInParams*)pBridgePartyMediaParams)->m_SiteName));
	SetMuteMask((((CBridgePartyVideoInParams*)pBridgePartyMediaParams)->m_mute_mask));
	//m_eTelePresenceMode = ((((CBridgePartyVideoInParams*)pBridgePartyMediaParams)->m_eTelePresenceMode));
	m_backgroundImageID = ((((CBridgePartyVideoInParams*)pBridgePartyMediaParams)->m_backgroundImageID));
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoInParams::Serialize(WORD format, CSegment& seg) const
{
	CBridgePartyVideoParams::Serialize(format, seg);

	if (format == NATIVE)
	{
		DWORD NameLen = strlen((char*)m_SiteName);
		seg << NameLen;
		seg.Put((BYTE*)m_SiteName, NameLen);
		seg << (DWORD)m_backgroundImageID;
	}
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoInParams::DeSerialize(WORD format, CSegment& seg)
{
	CBridgePartyVideoParams::DeSerialize(format, seg);

	if (format == NATIVE)
	{
		DWORD NameLen;
		seg >> NameLen;
		seg.Get((BYTE*)m_SiteName, NameLen);
		m_SiteName[NameLen] = 0;
		seg >> m_backgroundImageID;
	}
}

////////////////////////////////////////////////////////////////////////////
DWORD CBridgePartyVideoInParams::GetBackgroundImageID()
{
	return m_backgroundImageID;
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoInParams::SetBackgroundImageID(DWORD backgroundImageID)
{
	m_backgroundImageID = backgroundImageID;
}

CAvcToSvcParams* CBridgePartyVideoInParams::GetAvcToSvcParams()
{
	return m_pAvcToSvcParams;
}

///////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoInParams::InitAvcToSvcParams()
{
	PDELETE(m_pAvcToSvcParams);
	m_pAvcToSvcParams = new CAvcToSvcParams();
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoInParams::SetVisualEffects(CVisualEffectsParams* pVisualEffects)
{
	m_pVisualEffects = pVisualEffects;
	if (pVisualEffects)
	{
		PTRACE2INT(eLevelInfoNormal, "CBridgePartyVideoInParams::SetVisualEffects() - BackgroundImageID = ", pVisualEffects->GetBackgroundImageID());
		PTRACE2INT(eLevelInfoNormal, "CBridgePartyVideoInParams::SetVisualEffects() - BackgroundColorYUV = ", pVisualEffects->GetBackgroundColorYUV());
	}
	else
		PTRACE(eLevelInfoNormal, "CBridgePartyVideoInParams::SetVisualEffects() - pVisualEffects = NULL");
}

////////////////////////////////////////////////////////////////////////////
//                        CBridgePartyVideoOutParams
////////////////////////////////////////////////////////////////////////////
CBridgePartyVideoOutParams::CBridgePartyVideoOutParams()
	: CBridgePartyVideoParams()
{
	m_pVisualEffects       = NULL;
	m_LayoutType           = CP_NO_LAYOUT;
	m_partyLectureModeRole = eREGULAR;

	m_isCascadeLink        = NO;
	m_videoQuality         = eVideoQualitySharpness; // Sharpness is the default value
	m_isSiteNamesEnabled   = YES;
  m_RemoteIdent			 = Regular;

	for (int i = (int)CP_LAYOUT_1X1; i < (int)CP_NO_LAYOUT; ++i)
		m_pReservation[i] = NULL;

	for (int i = (int)CP_LAYOUT_1X1; i < (int)CP_NO_LAYOUT; ++i)
		m_pPrivateReservation[i] = NULL;

	m_copLrt = eLogical_res_none;
}

////////////////////////////////////////////////////////////////////////////
CBridgePartyVideoOutParams::CBridgePartyVideoOutParams(
	DWORD vidAlg, DWORD vidBitRate,
	eVideoFrameRate videoQCifFrameRate, eVideoFrameRate videoCifFrameRate, eVideoFrameRate video4CifFrameRate, eVideoResolution videoResolution,
	DWORD mbps, DWORD fs, eVideoQuality videoQuality,
	eTelePresencePartyType eTelePresenceMode,
	WORD isCascadeLink,
	eVideoFrameRate videoFrameRate,
	eLogicalResourceTypes copLrt,
	eVideoProfile profile)
{
	m_videoAlg             = vidAlg;
	m_videoBitRate         = vidBitRate;
	m_videoQcifFrameRate   = videoQCifFrameRate;
	m_videoCifFrameRate    = videoCifFrameRate;
	m_video4CifFrameRate   = video4CifFrameRate;
	m_videoResolution      = videoResolution;
	m_pVisualEffects       = NULL;
	m_LayoutType           = CP_NO_LAYOUT;
	m_partyLectureModeRole = eREGULAR;
	m_MBPS                 = mbps;
	m_FS                   = fs;
//	if (!IsValidPObjectPtr(m_telepresenseEPInfo))
//		m_telepresenseEPInfo = new CTelepresenseEPInfo;
	SetTelePresenceMode(eTelePresenceMode);
	m_isCascadeLink        = isCascadeLink;
	m_videoQuality         = videoQuality;
	m_isSiteNamesEnabled   = YES;
	m_copLrt               = copLrt;

	for (int i = (int)CP_LAYOUT_1X1; i < (int)CP_NO_LAYOUT; ++i)
		m_pReservation[i] = NULL;

	for (int i = (int)CP_LAYOUT_1X1; i < (int)CP_NO_LAYOUT; ++i)
		m_pPrivateReservation[i] = NULL;
}

////////////////////////////////////////////////////////////////////////////
CBridgePartyVideoOutParams::CBridgePartyVideoOutParams (const CBridgePartyVideoOutParams& rOtherBridgePartyVideoParams)
: CBridgePartyVideoParams(rOtherBridgePartyVideoParams)
{
	m_pVisualEffects       = rOtherBridgePartyVideoParams.m_pVisualEffects;
	m_LayoutType           = rOtherBridgePartyVideoParams.m_LayoutType;
	m_partyLectureModeRole = rOtherBridgePartyVideoParams.m_partyLectureModeRole;
	m_isCascadeLink        = rOtherBridgePartyVideoParams.m_isCascadeLink;
	m_videoQuality         = rOtherBridgePartyVideoParams.m_videoQuality;
	m_isSiteNamesEnabled   = rOtherBridgePartyVideoParams.m_isSiteNamesEnabled;
	m_copLrt               = rOtherBridgePartyVideoParams.m_copLrt;
    m_RemoteIdent		   = rOtherBridgePartyVideoParams.m_RemoteIdent;
}

////////////////////////////////////////////////////////////////////////////
CBridgePartyVideoOutParams::~CBridgePartyVideoOutParams ()
{
	for (int i = (int)CP_LAYOUT_1X1; i < (int)CP_NO_LAYOUT; ++i)
		POBJDELETE(m_pReservation[i]);

	for (int i = (int)CP_LAYOUT_1X1; i < (int)CP_NO_LAYOUT; ++i)
		POBJDELETE(m_pPrivateReservation[i]);
}

////////////////////////////////////////////////////////////////////////////
CBridgePartyVideoOutParams& CBridgePartyVideoOutParams::operator =(const CBridgePartyVideoOutParams& rOtherBridgePartyVideoParams)
{
	if (this == &rOtherBridgePartyVideoParams)
		return *this;

	CBridgePartyVideoParams::operator=(rOtherBridgePartyVideoParams);

	m_pVisualEffects       = rOtherBridgePartyVideoParams.m_pVisualEffects;
	m_LayoutType           = rOtherBridgePartyVideoParams.m_LayoutType;
	m_partyLectureModeRole = rOtherBridgePartyVideoParams.m_partyLectureModeRole;
	m_isCascadeLink        = rOtherBridgePartyVideoParams.m_isCascadeLink;
	m_videoQuality         = rOtherBridgePartyVideoParams.m_videoQuality;
	m_copLrt               = rOtherBridgePartyVideoParams.m_copLrt;
    m_RemoteIdent		   = rOtherBridgePartyVideoParams.m_RemoteIdent;

	for (int i = (int)CP_LAYOUT_1X1; i < (int)CP_NO_LAYOUT; ++i)
	{
		if (rOtherBridgePartyVideoParams.m_pReservation[i])
			m_pReservation[i] = new CLayout(*rOtherBridgePartyVideoParams.m_pReservation[i]);
	}

	for (int i = (int)CP_LAYOUT_1X1; i < (int)CP_NO_LAYOUT; ++i)
	{
		if (rOtherBridgePartyVideoParams.m_pPrivateReservation[i])
			m_pPrivateReservation[i] = new CLayout(*rOtherBridgePartyVideoParams.m_pPrivateReservation[i]);
	}

	return *this;
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoOutParams::SetVisualEffects(CVisualEffectsParams* pVisualEffects)
{
	m_pVisualEffects = pVisualEffects;

	if (pVisualEffects)
		TRACEINTO << "BackgroundImageID:" << pVisualEffects->GetBackgroundImageID();
	else
		TRACEINTO << "CBridgePartyVideoOutParams::SetVisualEffects() - pVisualEffects = NULL";
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoOutParams::SetLayoutType(LayoutType layoutType)
{
	m_LayoutType = layoutType;
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoOutParams::SetPartyLectureModeRole(ePartyLectureModeRole partyLectureModeRole)
{
	m_partyLectureModeRole = partyLectureModeRole;
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoOutParams::SetIsCascadeLink(WORD isCascadeLink)
{
	m_isCascadeLink = isCascadeLink;
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoOutParams::SetCopLrt(eLogicalResourceTypes lrt)
{
	m_copLrt = lrt;
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoOutParams::SetVideoQualityType(eVideoQuality videoQuality)
{
	m_videoQuality = videoQuality;
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoOutParams::SetReservationLayout(CLayout* pReservationLayout, int index)
{
	if (!m_pReservation[index])
		m_pReservation[index] = pReservationLayout;

	else
		*m_pReservation[index] = *pReservationLayout;
}

////////////////////////////////////////////////////////////////////////////
CLayout* CBridgePartyVideoOutParams::GetReservationLayout(int index)
{
	return m_pReservation[index];
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoOutParams::SetPrivateReservationLayout(CLayout* privateReservationLayout, int index)
{
	if (!m_pPrivateReservation[index])
		m_pPrivateReservation[index] = privateReservationLayout;

	else
		*m_pPrivateReservation[index] = *privateReservationLayout;
}

////////////////////////////////////////////////////////////////////////////
CLayout* CBridgePartyVideoOutParams::GetPrivateReservationLayout(int index)
{
	CLayout* layout = m_pPrivateReservation[index];
	return layout;
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoOutParams::InitPartyVideoOutParams(const CBridgePartyVideoParams* pParams)
{
	InitPartyVideoParams(pParams);

	m_pVisualEffects = new CVisualEffectsParams;

	if (((CBridgePartyVideoOutParams*)pParams)->m_pVisualEffects)
		*m_pVisualEffects = *(((CBridgePartyVideoOutParams*)pParams)->m_pVisualEffects);

	else
		TRACEINTO << "pBridgePartyMediaParams->GetVisualEffects() is NULL";

	TRACEINTO
		<< "BackgroundColor:" << m_pVisualEffects->GetBackgroundColorYUV()
		<< ", BackgroundImageID:" << m_pVisualEffects->GetBackgroundImageID();

	const CBridgePartyVideoOutParams& outParams = *(CBridgePartyVideoOutParams*)pParams;

	m_LayoutType           = outParams.m_LayoutType;
	m_partyLectureModeRole = outParams.m_partyLectureModeRole;
	m_isCascadeLink        = outParams.m_isCascadeLink;
	m_videoQuality         = outParams.m_videoQuality;
	m_copLrt               = outParams.m_copLrt;
    m_RemoteIdent		   = outParams.m_RemoteIdent;

	for (LayoutType i = CP_LAYOUT_1X1; i < CP_NO_LAYOUT; ++i)
	{
		if (outParams.m_pReservation[i])
			m_pReservation[i] = new CLayout(*outParams.m_pReservation[i]);
	}

	for (LayoutType i = CP_LAYOUT_1X1; i < CP_NO_LAYOUT; ++i)
	{
		if (outParams.m_pPrivateReservation[i])
			m_pPrivateReservation[i] = new CLayout(*outParams.m_pPrivateReservation[i]);
	}
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoOutParams::Serialize(WORD format, CSegment& seg) const
{
	CBridgePartyVideoParams::Serialize(format, seg);

	if (format == NATIVE)
	{
		seg << (void*)m_pVisualEffects;
		seg << (DWORD)m_LayoutType;
		seg << (DWORD)m_partyLectureModeRole;
		seg << m_isCascadeLink;
		seg << (DWORD)m_videoQuality;
		seg << (DWORD)m_copLrt;
	}
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoOutParams::DeSerialize(WORD format, CSegment& seg)
{
	CBridgePartyVideoParams::DeSerialize(format, seg);

	if (format == NATIVE)
	{
		seg >> (void*&)m_pVisualEffects;
		seg >> (DWORD&)m_LayoutType;
		seg >> (DWORD&)m_partyLectureModeRole;
		seg >> m_isCascadeLink;
		seg >> (DWORD&)m_videoQuality;
		seg >> (DWORD&)m_copLrt;
	}
}

////////////////////////////////////////////////////////////////////////////
BOOL CBridgePartyVideoOutParams::IsValidCopParams() const
{
	TRACEINTO << "m_copResourceIndex:" << m_copResourceIndex;
	return m_copResourceIndex < NUM_OF_LEVEL_ENCODERS;
}
////////////////////////////////////////////////////////////////////////////
BOOL CBridgePartyVideoOutParams::IsValidXCodeParams() const
{
	TRACEINTO << "m_XCodeResourceIndex:" <<  m_XCodeResourceIndex;
	return m_XCodeResourceIndex < eXcodeContentDecoder;
}

////////////////////////////////////////////////////////////////////////////
bool CBridgePartyVideoParams::IsValidMaxDPB(DWORD dpb)
{
	return (dpb && dpb <= H264_L5_1_DEFAULT_DPB) || (dpb == ~0u);
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoParams::SetIsTipMode(BYTE bIsTipMode)
{
	m_bIsTipMode = bIsTipMode;
}

////////////////////////////////////////////////////////////////////////////
BYTE CBridgePartyVideoParams::GetIsTipMode() const
{
	return m_bIsTipMode;
}
////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoParams::SetUseIntermediateSDResolution(BYTE useIntermediateSDResolution)
{
	m_bUseIntermediateSDResolution = useIntermediateSDResolution;
}
////////////////////////////////////////////////////////////////////////////
BYTE CBridgePartyVideoParams::GetUseIntermediateSDResolution() const
{
	return m_bUseIntermediateSDResolution;
}

///////////////////////////////////////////////////////////////////////////

