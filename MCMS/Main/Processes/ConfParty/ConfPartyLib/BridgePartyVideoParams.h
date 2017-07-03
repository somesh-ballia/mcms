#ifndef CBridge_Party_Video_Params_H__
#define CBridge_Party_Video_Params_H__

///////////////////////////////////////////////////////////////////////////
#include "BridgePartyMediaParams.h"
#include "VisualEffectsParams.h"
#include "VideoDefines.h"
#include "ConfPartyDefines.h"
#include "DwordBitMask.h"
#include "AckParams.h"
#include "Layout.h"

///////////////////////////////////////////////////////////////////////////
class CTelepresenseEPInfo;
class CAvcToSvcParams;

////////////////////////////////////////////////////////////////////////////
struct MsSvcParamsStruct
{
	DWORD ssrc;
	DWORD pr_id;
	DWORD nWidth;
	DWORD nHeight;

	DWORD aspectRatio;
	DWORD maxFrameRate;
	DWORD maxBitRate;     // INTEGER (1 - 19200), -- units of 100 bit/s

	MsSvcParamsStruct()
	{ memset(this, 0, sizeof(*this)); }

	bool operator ==(const MsSvcParamsStruct& obj) const
	{ return 0 == memcmp(this, &obj, sizeof(*this));}

	bool operator !=(const MsSvcParamsStruct& obj) const
	{ return !(*this == obj); }

	void Clear()
	{ *this = MsSvcParamsStruct(); }
};

///////////////////////////////////////////////////////////////////////////
std::ostream& operator <<(std::ostream& os, const MsSvcParamsStruct& obj);

///////////////////////////////////////////////////////////////////////////
CSegment& operator <<(CSegment& seg, const MsSvcParamsStruct& obj);
CSegment& operator >>(CSegment& seg, MsSvcParamsStruct& obj);

///////////////////////////////////////////////////////////////////////////
struct MsFullPacsiInfoStruct
{
	MsSvcParamsStruct pacsiInfo[3];

	MsFullPacsiInfoStruct()
	{
		for (size_t i = 0; i < ARRAYSIZE(pacsiInfo); ++i)
			pacsiInfo[i].pr_id = ~0; // mark it as INVALID
	}

	bool operator ==(const MsFullPacsiInfoStruct& obj) const
	{ return 0 == memcmp(this, &obj, sizeof(*this));}

	bool operator !=(const MsFullPacsiInfoStruct& obj) const
	{ return !(*this == obj); }
};

///////////////////////////////////////////////////////////////////////////
std::ostream& operator <<(std::ostream& os, const MsFullPacsiInfoStruct& obj);

///////////////////////////////////////////////////////////////////////////
CSegment& operator <<(CSegment& seg, const MsFullPacsiInfoStruct& obj);
CSegment& operator >>(CSegment& seg, MsFullPacsiInfoStruct& obj);

//////////////////////////////////////////////////////////////////////
class CBridgePartyVideoParams : public CBridgePartyMediaParams
{
	CLASS_TYPE_1(CBridgePartyVideoParams, CBridgePartyMediaParams)

public:

	static bool IsValidVideoAlgorithm(DWORD dwVideoAlgorithm);
	static bool IsValidVideoBitRate(DWORD dwVideoAlgorithm, eVideoConfType videoConfType);
	static bool IsValidVideoResolution(eVideoResolution resolution);
	static bool IsValidVideoFrameRate(eVideoFrameRate frameRate);
	static bool IsValidMBPS(DWORD mbps);
	static bool IsValidFS(DWORD fs);
	static bool IsValidSampleAspectRatio(DWORD sampleAspectRatio);
	static bool IsValidStaticMB(DWORD staticMB);
	static bool IsValidMaxDPB(DWORD dpb);

public:

	virtual      ~CBridgePartyVideoParams();

	virtual void Serialize(WORD format, CSegment& seg) const;
	virtual void DeSerialize(WORD format, CSegment& seg);

	virtual BOOL IsValidParams() const;

public:

	CBridgePartyVideoParams();
	CBridgePartyVideoParams(const CBridgePartyVideoParams& obj);

	CBridgePartyVideoParams& operator=(const CBridgePartyVideoParams& obj);

public:

	void SetVideoAlgorithm(DWORD videoAlg)
	{ m_videoAlg = videoAlg; }

	DWORD GetVideoAlgorithm() const
	{ return m_videoAlg; }

	void SetVideoBitRate(DWORD videoBitRate)
	{ m_videoBitRate = videoBitRate; }

	DWORD GetVideoBitRate() const
	{ return m_videoBitRate; }

	void SetVideoFrameRate(eVideoResolution resolution, eVideoFrameRate videoFrameRate);

	eVideoFrameRate GetVideoFrameRate(eVideoResolution resolution) const;

	void SetVideoResolution(eVideoResolution videoResolution)
	{ m_videoResolution = videoResolution;}

	eVideoResolution GetVideoResolution() const
	{ return m_videoResolution; }

	void SetMBPS(DWORD mbps)
	{ m_MBPS = mbps; }

	DWORD GetMBPS() const
	{ return m_MBPS; }

	void SetFS(DWORD fs)
	{ m_FS = fs; }

	DWORD GetFS() const
	{ return m_FS; }

	void SetSampleAspectRatio(DWORD sampleAspectRatio)
	{ m_sampleAspectRatio = sampleAspectRatio; }

	DWORD GetSampleAspectRatio() const
	{ return m_sampleAspectRatio; }

	void SetStaticMB(DWORD staticMB)
	{ m_staticMB = staticMB; }

	DWORD GetStaticMB() const
	{ return m_staticMB; }

	void SetIsVideoClarityEnabled(BYTE bIsVideoClarityEnabled)
	{ m_isVideoClarityEnabled = bIsVideoClarityEnabled; }

	BYTE GetIsVideoClarityEnabled() const
	{ return m_isVideoClarityEnabled; }

	void SetCopConnectionId(DWORD connectionId)
	{ m_copConnectionId = connectionId; }

	DWORD GetCopConnectionId() const
	{ return m_copConnectionId; }

	void SetCopPartyId(DWORD partyId)
	{ m_copPartyId = partyId; }

	DWORD GetCopPartyId() const
	{ return m_copPartyId; }

	void SetCopResourceIndex(WORD partyId)
	{ m_copResourceIndex = partyId; }

	WORD GetCopResourceIndex() const
	{ return m_copResourceIndex; }

	void SetCopResourceOfLecturerLinkIndex(WORD partyId)
	{ m_copResourceIndexOfCascadeLinkLecturer = partyId; }

	WORD GetCopResourceOfLecturerLinkIndex() const
	{ return m_copResourceIndexOfCascadeLinkLecturer; }

	void SetCopLinkLecturerMode(BYTE isLecturer)
	{ m_bCascadeIsLecturer = isLecturer; }

	BYTE GetIsCopLinkLecturer() const
	{ return m_bCascadeIsLecturer; }

	void SetVideConfType(eVideoConfType videoConfType)
	{ m_videoConfType = videoConfType; }

	eVideoConfType GetVideoConfType() const
	{ return m_videoConfType; }

	void SetVidFrameRate(eVideoFrameRate videoFrameRate);

	eVideoFrameRate GetVidFrameRate() const
	{ return m_videoFrameRate; }

	void SetProfile(eVideoProfile profile)
	{ m_profile = profile; }

	eVideoProfile GetProfile() const
	{ return m_profile; }

	void SetPacketFormat(eVideoPacketPayloadFormat packetFormat)
	{ m_packetFormat = packetFormat; }

	eVideoPacketPayloadFormat GetPacketFormat() const
	{ return m_packetFormat; }

	void SetMaxDPB(DWORD dpb)
	{ m_maxDPB = dpb; }

	DWORD GetMaxDPB() const
	{ return m_maxDPB; }

	void SetVideoResolutionTableType(EVideoResolutionTableType eVideoResolutionTableType)
	{ m_eVideoResolutionTableType = eVideoResolutionTableType; }

	EVideoResolutionTableType GetVideoResolutionTableType() const
	{ return m_eVideoResolutionTableType; }

	void SetVideoPartyType(eVideoPartyType videoPartyType)
	{ m_videoPartyType = videoPartyType; }

	eVideoPartyType GetVideoPartyType() const
	{ return m_videoPartyType; }

	void SetIsAutoBrightness(bool isAUtoBrightness)
	{ m_isAutoBrightness = isAUtoBrightness; }

	bool GetIsAutoBrightness() const
	{ return m_isAutoBrightness; }

	void SetIsRemoteNeedSmartSwitchAccordingToVendor(BYTE isRemotNeedSmartSwitchAccordingToVendor);

	BYTE GetIsRemoteNeedSmartSwitchAccordingToVendor() const
	{ return m_isRemoteNeedSmartSwitchAccordingToVendor; }

	void SetPartyCascadeMode(ECascadePartyType cascadeMode)
	{ m_CascadeMode = cascadeMode; }

	ECascadePartyType GetPartyCascadeType() const
	{ return m_CascadeMode; }

	void SetIsTipMode(BYTE);
	BYTE GetIsTipMode() const;

	void SetUseIntermediateSDResolution(BYTE useIntermediateSDResolution = false);
	BYTE GetUseIntermediateSDResolution() const;

	void SetIsEncodeRTVBFrame(BYTE bIsEncodeRTVBFrame = false)
	{ m_bIsEncodeRTVBFrame = bIsEncodeRTVBFrame; }

	BYTE GetIsEncodeRTVBFrame() const
	{ return m_bIsEncodeRTVBFrame; }

	void SetLPRParams(DWORD lossProtection, DWORD mtbf, DWORD congestionCeiling, DWORD fill, DWORD modeTimeout);

	CAckParams* GetAckParams()
	{ return m_pAckParams; }

	void GetMaxMBPSAndFSFromH263H261Params(DWORD& maxFS, DWORD& maxMBPS);

	// *** XCode ***

	void SetXCodeConnectionId(DWORD connectionId)
	{ m_XCodeConnectionId = connectionId; }

	DWORD GetXCodeConnectionId() const
	{ return m_XCodeConnectionId; }

	void SetXCodePartyId(DWORD partyId)
	{ m_XCodePartyId = partyId; }

	DWORD GetXCodePartyId() const
	{ return m_XCodePartyId; }

	void SetXCodeResourceIndex(WORD partyId)
	{ m_XCodeResourceIndex = partyId; }

	WORD GetXCodeResourceIndex() const
	{ return m_XCodeResourceIndex; }

	void SetIsH263Plus(bool isH263Plus)
	{ m_isH263Plus = isH263Plus; }

	bool GetIsH263Plus() const
	{ return m_isH263Plus; }

	void SetFrThreshold(DWORD dwFrThreshold)
	{ m_dwFrThreshold = dwFrThreshold; }

	DWORD GetFrThreshold() const
	{ return m_dwFrThreshold; }

	void InitPartyVideoParams(const CBridgePartyVideoParams* pBridgePartyMediaParams);
	void InitAckParams();
	void SetOnlyVideoParams(const CBridgePartyVideoParams* pBridgePartyMediaParams);
	void DumpVideoParams();

	const MsSvcParamsStruct& MsSvcParams() const
	{ return m_tSvcParameters; }

	MsSvcParamsStruct& MsSvcParams()
	{ return m_tSvcParameters; }

	const MsFullPacsiInfoStruct& PACSI() const
	{ return m_tPACSI; }

	void SetFullPacsi(const MsFullPacsiInfoStruct& pacsi)
	{ m_tPACSI = pacsi; }
	
	void SetRemoteIdent(DWORD remoteIdent)
	{ m_RemoteIdent = remoteIdent; }

	DWORD GetRemoteIdent() const 
	{ return m_RemoteIdent; }
	
protected:

	DWORD m_videoAlg;
	DWORD m_videoBitRate;

	// H263 parameters
	eVideoResolution m_videoResolution;
	eVideoFrameRate  m_videoQcifFrameRate;
	eVideoFrameRate  m_videoCifFrameRate;
	eVideoFrameRate  m_video4CifFrameRate;
	eVideoFrameRate  m_videoVGAFrameRate;
	eVideoFrameRate  m_videoSVGAFrameRate;
	eVideoFrameRate  m_videoXGAFrameRate;

	// H264 parameters
	EVideoResolutionTableType m_eVideoResolutionTableType;
	eVideoProfile             m_profile;                    // H264 profile
	eVideoPacketPayloadFormat m_packetFormat;
	eVideoPartyType           m_videoPartyType;

	DWORD m_MBPS;                       // units of 500 macro blocks per second
	DWORD m_FS;                         // units of 256 luma macro blocks
	DWORD m_staticMB;                   // units of 500
	DWORD m_sampleAspectRatio;

	BYTE m_isVideoClarityEnabled;

	CAckParams* m_pAckParams;

	// COP parameters
	DWORD m_copConnectionId;
	DWORD m_XCodeConnectionId;

	DWORD m_copPartyId;
	DWORD m_XCodePartyId;

	WORD  m_copResourceIndex;
	WORD  m_XCodeResourceIndex;

	WORD  m_copResourceIndexOfCascadeLinkLecturer;

	eVideoConfType    m_videoConfType;
	eVideoFrameRate   m_videoFrameRate;
	ECascadePartyType m_CascadeMode;

	DWORD m_dwFrThreshold;
	DWORD m_confMediaType;
	DWORD m_maxDPB;

	bool m_isAutoBrightness;
	bool m_isH263Plus;
	BYTE m_isRemoteNeedSmartSwitchAccordingToVendor;
	BYTE m_bCascadeIsLecturer;
	BYTE m_bIsTipMode;
	BYTE m_bUseIntermediateSDResolution;
	BYTE m_bIsEncodeRTVBFrame;

	CVisualEffectsParams* m_pVisualEffects;

	MsSvcParamsStruct     m_tSvcParameters;

	MsFullPacsiInfoStruct m_tPACSI;
	DWORD				  m_RemoteIdent;
};

////////////////////////////////////////////////////////////////////////////
class CBridgePartyVideoInParams : public CBridgePartyVideoParams
{
	CLASS_TYPE_1(CBridgePartyVideoInParams, CBridgePartyVideoParams)

public:

	virtual ~CBridgePartyVideoInParams();

	virtual void Serialize(WORD format, CSegment& seg) const;
	virtual void DeSerialize(WORD format, CSegment& seg);

public:

	CBridgePartyVideoInParams();

	CBridgePartyVideoInParams(
		DWORD vidAlg,
		DWORD vidBitRate,
		eVideoFrameRate videoQCifFrameRate,
		eVideoFrameRate videoCifFrameRate,
		eVideoFrameRate video4CifFrameRate,
		eVideoResolution videoResolution,
		DWORD mbps,
		DWORD fs,
		const char* pSiteName,
		CDwordBitMask muteMask,
		eTelePresencePartyType eTelePresenceMode,
		DWORD backgroundImageID,
		bool isAutoBrightness,
		eVideoFrameRate videoVGAFrameRate = eVideoFrameRateDUMMY,
		eVideoFrameRate videoSVGAFrameRate = eVideoFrameRateDUMMY,
		eVideoFrameRate videoXGAFrameRate = eVideoFrameRateDUMMY,
		BYTE isRemoteNeedSmartSwitchAccordToVendor = false,
		ECascadePartyType cascadeMode = eCascadeNone);

	CBridgePartyVideoInParams(const CBridgePartyVideoInParams& obj);

	CBridgePartyVideoInParams& operator=(const CBridgePartyVideoInParams& obj);

public:

	void InitPartyVideoInParams(const CBridgePartyVideoParams* pBridgePartyMediaParams);

	void SetSiteName(const char* pSiteName);

	const char* GetSiteName() const
	{ return m_SiteName; }

	void SetMuteMask(const CDwordBitMask& mute_mask)
	{ m_mute_mask = mute_mask; }

	CDwordBitMask GetMuteMask() const
	{ return m_mute_mask; }

	void SetBackgroundImageID(DWORD backgroundImageID);

	DWORD GetBackgroundImageID();

	void InitAvcToSvcParams();

	CAvcToSvcParams* GetAvcToSvcParams();

	void SetVisualEffects(CVisualEffectsParams* pVisualEffects);

	CVisualEffectsParams* GetVisualEffects() const
	{ return m_pVisualEffects; }

protected:

	char             m_SiteName[MAX_SITE_NAME_ARR_SIZE];
	CDwordBitMask    m_mute_mask;
	DWORD            m_backgroundImageID; // The background id is needed for video optimization - in order to fit to FPGA scaler output resolutions more accurately
	CAvcToSvcParams* m_pAvcToSvcParams;
};

////////////////////////////////////////////////////////////////////////////
class CBridgePartyVideoOutParams : public CBridgePartyVideoParams
{
public:
	CLASS_TYPE_1(CBridgePartyVideoOutParams, CBridgePartyVideoParams)

	virtual const char* NameOf() const
	{ return GetCompileType(); }

public:

	virtual ~CBridgePartyVideoOutParams();

	virtual void Serialize(WORD format, CSegment& seg) const;
	virtual void DeSerialize(WORD format, CSegment& seg);

	virtual BOOL IsValidCopParams() const;
	virtual BOOL IsValidXCodeParams() const;

public:

	CBridgePartyVideoOutParams();

	CBridgePartyVideoOutParams(
		DWORD vidAlg,
		DWORD vidBitRate,
		eVideoFrameRate videoQCifFrameRate,
		eVideoFrameRate videoCifFrameRate,
		eVideoFrameRate video4CifFrameRate,
		eVideoResolution videoResolution,
		DWORD mbps,
		DWORD fs,
		eVideoQuality videoQualityType,
		eTelePresencePartyType m_eTelePresenceMode = eTelePresencePartyNone,
		WORD isCascadeLink = NO,
		eVideoFrameRate videoFrameRate = eVideoFrameRateDUMMY,
		eLogicalResourceTypes copLrt = eLogical_res_none,
		eVideoProfile profile = eVideoProfileDummy);

	CBridgePartyVideoOutParams(const CBridgePartyVideoOutParams& rOtherBridgePartyVideoParams);

	CBridgePartyVideoOutParams& operator=(const CBridgePartyVideoOutParams& rOtherBridgePartyVideoParams);

public:

	void SetVisualEffects(CVisualEffectsParams* pVisualEffects);

	CVisualEffectsParams* GetVisualEffects() const
	{ return m_pVisualEffects; }

	void SetLayoutType(LayoutType layoutType);

	LayoutType GetLayoutType() const
	{ return m_LayoutType; }

	void SetPartyLectureModeRole(ePartyLectureModeRole partyLectureModeRole);

	ePartyLectureModeRole GetPartyLectureModeRole() const
	{ return m_partyLectureModeRole; }

	void SetIsCascadeLink(WORD isCascadeLink);

	WORD GetIsCascadeLink() const
	{ return m_isCascadeLink; }

	void SetVideoQualityType(eVideoQuality videoQuality);

	eVideoQuality GetVideoQualityType() const
	{ return m_videoQuality; }

	void SetCopLrt(eLogicalResourceTypes lrt);

	eLogicalResourceTypes GetCopLrt() const
	{ return m_copLrt; }

	void     SetReservationLayout(CLayout* pReservationLayout, int index);
	CLayout* GetReservationLayout(int index);

	void     SetPrivateReservationLayout(CLayout* pPrivateReservationLayout, int index);
	CLayout* GetPrivateReservationLayout(int index);

	void SetIsSiteNamesEnabled(BYTE bIsSiteNamesEnabled)
	{ m_isSiteNamesEnabled = bIsSiteNamesEnabled; }

	BYTE GetIsSiteNamesEnabled() const
	{ return m_isSiteNamesEnabled; }

	void InitPartyVideoOutParams(const CBridgePartyVideoParams* pBridgePartyMediaParams);
	void InitOtherVideoOutParams();

protected:

	LayoutType                  m_LayoutType;
	ePartyLectureModeRole       m_partyLectureModeRole;

	WORD                        m_isCascadeLink;
	eVideoQuality               m_videoQuality;
	CLayout*                    m_pReservation[CP_NO_LAYOUT];         // For forces in party level made on the conference layout
	CLayout*                    m_pPrivateReservation[CP_NO_LAYOUT];  // For forces in Private Layout
	eLogicalResourceTypes       m_copLrt;                             // for the cop encoder we need to now which one of the Lrt types
	BYTE                        m_isSiteNamesEnabled;
};

///////////////////////////////////////////////////////////////////////////
#endif // CBridge_Party_Video_Params_H__
