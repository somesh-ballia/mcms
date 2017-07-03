#ifndef BRIDGE_PARTY_VIDEO_UNI_DIRECTION_H__
#define BRIDGE_PARTY_VIDEO_UNI_DIRECTION_H__

#include "BridgePartyMediaUniDirection.h"
#include "VideoBridgePartyCntl.h"

////////////////////////////////////////////////////////////////////////////
class CBridgePartyVideoUniDirection : public CBridgePartyMediaUniDirection
{
	CLASS_TYPE_1(CBridgePartyVideoUniDirection, CBridgePartyMediaUniDirection)

	virtual const char* NameOf() const
	{ return GetCompileType();}

public:

	CBridgePartyVideoUniDirection ();
	CBridgePartyVideoUniDirection(const CBridgePartyVideoUniDirection& obj);

	virtual      ~CBridgePartyVideoUniDirection ();

	virtual void Connect()    = 0;
	virtual void DisConnect() = 0;
	virtual void Create(const CBridgePartyCntl* pBridgePartyCntl, const CRsrcParams* pRsrcParams, const CBridgePartyMediaParams* pBridgePartyMediaParams);

	virtual BOOL IsConnected()    = 0;
	virtual BOOL IsDisConnected() = 0;

	virtual void UpdateVideoClarity(WORD isVideoClarityEnabled)               = 0;
	virtual void FillAllocStatus(ALLOC_STATUS_PER_PORT_S& allocStatusPerPort) = 0;

	virtual void UpdateNewConfParams(DWORD confRsrcId, const CBridgePartyMediaParams* pBridgePartyVideoParams);
	virtual void UpdatePartyParams(CBridgePartyVideoParams* pUpdatePartyVideoInitParams);
	virtual void UpdateVideoParams(CBridgePartyVideoParams* pBridgePartyVideoParams)             = 0;
	virtual void DumpAllInfoOnConnectionState(CMedString* pMedString, bool isShortPrint = false) = 0;

	virtual BYTE IsPortOpenedOn() { return m_isPortOpened; }

	ECascadePartyType GetCascadeMode();
	void              SetCascadeMode(ECascadePartyType cascadeMode);

	DWORD             GetVideoRate() const  { return m_videoBitRate; }
	eVideoFrameRate   GetVidFrameRate() const { return m_videoFrameRate; }

protected:

	virtual void SendConnectToRtp()                                              = 0;
	virtual void SaveAndSendUpdatedVideoParams(CBridgePartyVideoParams* pParams) = 0;

	// action functions
	virtual void OnVideoBridgePartyConnectIDLE(CSegment* seg)                    = 0;
	virtual void OnVideoBridgePartyConnectSETUP(CSegment* seg)                   = 0;
	virtual void OnVideoBridgePartyConnectCONNECTED(CSegment* seg)               = 0;
	virtual void OnVideoBridgePartyConnectDISCONNECTING(CSegment* seg)           = 0;

	virtual void OnVideoBridgePartyDisConnectIDLE(CSegment* seg)                 = 0;
	virtual void OnVideoBridgePartyDisConnectSETUP(CSegment* seg)                = 0;
	virtual void OnVideoBridgePartyDisConnectCONNECTED(CSegment* seg)            = 0;
	virtual void OnVideoBridgePartyDisConnectDISCONNECTING(CSegment* seg)        = 0;
	virtual void OnVideoBridgePartyDisConnect(CSegment* seg)                     = 0;

	virtual void OnMplAckSETUP(CSegment* seg)                                    = 0;
	virtual void OnMplAckDISCONNECTING(CSegment* seg)                            = 0;
	virtual void OnMplOpenPortAck(STATUS status)                                 = 0;
	virtual void OnMplConnectAck(STATUS status)                                  = 0;
	virtual void OnMplClosePortAck(STATUS status)                                = 0;

	virtual void OnVideoBridgePartyUpdateVideoParamsIDLE(CSegment* seg)          = 0;
	virtual void OnVideoBridgePartyUpdateVideoParamsSETUP(CSegment* seg)         = 0;
	virtual void OnVideoBridgePartyUpdateVideoParamsCONNECTED(CSegment* seg);
	virtual void OnVideoBridgePartyUpdateVideoParamsDISCONNECTING(CSegment* seg)  = 0;

	virtual void OnVideoBridgePartyUpdateVideoClarityIDLE(CSegment* seg)          = 0;
	virtual void OnVideoBridgePartyUpdateVideoClaritySETUP(CSegment* seg)         = 0;
	virtual void OnVideoBridgePartyUpdateVideoClarityCONNECTED(CSegment* seg)     = 0;
	virtual void OnVideoBridgePartyUpdateVideoClarityDISCONNECTING(CSegment* seg) = 0;
	virtual void OnVideoBridgePartyUpdateVideoClarity(CSegment* seg);
	virtual void OnVideoBridgePartyUpdateAutoBrightness(CSegment* seg);

protected:

	bool AreCurrentVideoParamsFitsToAllocation(bool& isHigherThanAllocated);

	void SetMaxVideoParamsAccordingToAllocation(eVideoPartyType videoPartyType);
	void FixCurrentVideoParamsAccordingToAllocationAndUpdateIfNeeded();
	void FixCurrentVideoParamsIfHigherThanAllocation();

protected:

	DWORD                          m_videoAlg;
	DWORD                          m_videoBitRate;

	eVideoResolution               m_videoResolution;
	eVideoFrameRate                m_videoQcifFrameRate;
	eVideoFrameRate                m_videoCifFrameRate;
	eVideoFrameRate                m_video4CifFrameRate;
	eVideoFrameRate                m_videoVGAFrameRate;
	eVideoFrameRate                m_videoSVGAFrameRate;
	eVideoFrameRate                m_videoXGAFrameRate;
	eVideoFrameRate                m_videoFrameRate;	// for ms_svc

	eVideoConfType                 m_videoConfType;
	ECascadePartyType              m_eCascadeMode;
	eVideoProfile                  m_profile;
	eVideoPacketPayloadFormat      m_packetPayloadFormat;
	EVideoResolutionTableType      m_eResolutionTableType;

	DWORD                          m_MBPS; // units of 500 macro blocks per second
	DWORD                          m_FS;   // units of 256 luma macro blocks
	DWORD                          m_sampleAspectRatio;
	DWORD                          m_staticMB;
	DWORD                          m_maxDPB;
	DWORD                          m_dwFrThreshold;

	CBridgePartyVideoParams*       m_pWaitingForUpdateParams;
	CBridgePartyVideoParams*       m_pMaxVideoParamsAccordingToAllocation;

	DWORD                          m_croppingHor;
	DWORD                          m_croppingVer;

	BOOL                           m_isH263Plus;
	BOOL                           m_isAutoBrightness;
	BOOL                           m_bEncodeBFramesInRTV;

	BYTE                           m_isPortOpened;
	BYTE                           m_isVideoClarityEnabled;
	BYTE                           m_bIsTipMode;
	BYTE                           m_bUseIntermediateSDResolution;

	MsSvcParamsStruct              m_msftSvcParamsStruct;
	MsFullPacsiInfoStruct          m_PACSI;
	//VP8ParamsStruct				   m_VP8ParamsStruct;
};

#endif // BRIDGE_PARTY_VIDEO_UNI_DIRECTION_H__
