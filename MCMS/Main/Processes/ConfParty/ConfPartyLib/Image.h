#ifndef _CImage_H_
#define _CImage_H_

#include <list>
#include "PObject.h"
#include "Segment.h"
#include "Macros.h"
#include "DwordBitMask.h"
#include "VideoDefines.h"
#include "ConfPartyDefines.h"
#include "VideoRelayMediaStream.h"
#include "VideoRelayInOutMediaStream.h"
#include "VideoStructs.h"
#include "AcIndicationStructs.h"


class COstrStream;
class CTaskApp;

void SetSizesOfImage(WORD& X_sixe,WORD& Y_sixe, LayoutType layoutType);
WORD GetNumbSubImg(const LayoutType layoutType);
WORD GetOldLayoutType(const LayoutType layoutType);
BYTE isLayoutWithSpeakerPicture(const LayoutType layoutType);
WORD GetNegativeCoord(const WORD wCoord);

////////////////////////////////////////////////////////////////////////////
//                        BufferedIndicationState
////////////////////////////////////////////////////////////////////////////
template <typename TEnum, TEnum defaultValue = static_cast<TEnum>(0)>
class BufferedIndicationState
{
public:
	//typedef TEnum State_t;

	BufferedIndicationState() : set_(defaultValue), displayed_(defaultValue) { }

	TEnum queryForDisplay() const { return displayed_; }

	void  update(TEnum newState)  { set_ = newState; }
	bool  updateDisplay(bool bAnyWay = true)
	{
		bool updated = shouldDisplay(bAnyWay);
		if (updated)
			displayed_ = set_;

		return updated;
	}

private:
	bool shouldDisplay(bool bAnyWay = true) const
	{
		return bAnyWay ? set_ != displayed_ : set_ > displayed_;
	}

private:
	TEnum set_;
	TEnum displayed_;
};

typedef BufferedIndicationState<eRtcpPacketLossStatus> NetworkQualityState;
typedef std::list<CVideoRelayInMediaStream*>           CVideoRelayMediaStreamList;
typedef std::list<CRelayMediaStream*>                  CRelayMediaStreamList;

////////////////////////////////////////////////////////////////////////////
//                        CImage
////////////////////////////////////////////////////////////////////////////
class CImage : public CPObject
{
	CLASS_TYPE_1(CImage, CPObject)

public:
								CImage();
	                            CImage(DWORD source_connection_id,
	                                   DWORD source_partyRsrc_id,
	                                   CTaskApp* pVidSrc,
	                                   const char* pSiteName,
	                                   const char* pVidSrcName,
	                                   DWORD decoderDetectedModeWidth,
	                                   DWORD decoderDetectedModeHeight,
	                                   DWORD decoderDetectedSampleAspectRatioWidth,
	                                   DWORD decoderDetectedSampleAspectRatioHeight,
	                                   eVideoResolution videoResolution,
	                                   DWORD videoAlg,
	                                   WORD FS,
	                                   WORD MBPS,
	                                   CDwordBitMask muteMask,
	                                   DWORD dspSmartSwitchImageEntityId,
	                                   DWORD dspSmartSwitchConnectionId,
	                                   CRelayMediaStreamList& listVideoMediaStreams,
	                                   ECascadePartyType cascadeMode = eCascadeNone,
	                                   BOOL isVideoRelayImage = FALSE,
	                                   DWORD videoRelayInchannelHandle = INVALID);
	                           ~CImage();
							   
	virtual const char*         NameOf() const { return "CImage"; }

	BYTE                        isMuted() const;
	CDwordBitMask               GetMuteMask() const                               { return m_mute_mask; }

	void                        MuteByOperator()                                  { m_mute_mask.SetBit(OPERATOR_Prior); }
	void                        MuteByChairman()                                  { m_mute_mask.SetBit(CHAIRMAN_Prior); }
	void                        MuteByPartyMCV()                                  { m_mute_mask.SetBit(PARTY_Prior); }
	void                        MuteByMcms()                                      { m_mute_mask.SetBit(MCMS_Prior); }

	void                        UnMuteByOperator()                                { m_mute_mask.ResetBit(OPERATOR_Prior); }
	void                        UnMuteByChairman()                                { m_mute_mask.ResetBit(CHAIRMAN_Prior); }
	void                        UnMuteByPartyMCV()                                { m_mute_mask.ResetBit(PARTY_Prior); }
	void                        UnMuteByMcms()                                    { m_mute_mask.ResetBit(MCMS_Prior); }

	void                        SyncLost()                                        { m_mute_mask.SetBit(SYNC_LOST_Prior); }
	void                        SyncFound(DWORD decoderDetectedModeWidth = DEFAULT_DECODER_DETECTED_MODE_WIDTH, DWORD decoderDetectedModeHeight = DEFAULT_DECODER_DETECTED_MODE_HEIGHT, DWORD decoderDetectedSampleAspectRatioWidth = DEFAULT_DECODER_DETECTED_SAMPLE_ASPECT_RATIO_WIDTH, DWORD decoderDetectedSampleAspectRatioHeight = DEFAULT_DECODER_DETECTED_SAMPLE_ASPECT_RATIO_HEIGHT, DWORD  decoderDectedMSSvcSsrcID = INVALID, DWORD  decoderDetectedMSSvcPriorityID = INVALID);
	BYTE                        isSyncLost() const                                { return m_mute_mask.IsBitSet(SYNC_LOST_Prior); }

	BYTE                        IsMuteByOperator()                                { return m_mute_mask.IsBitSet(OPERATOR_Prior); }
	BYTE                        IsMuteByMCMS()                                    { return m_mute_mask.IsBitSet(MCMS_Prior); }
	BYTE                        IsMuteByParty()                                   { return m_mute_mask.IsBitSet(PARTY_Prior); }
	DWORD                       GetPartyRsrcId() const                            { return m_source_partyRsrc_id; }
	DWORD                       GetArtPartyId() const                             { return m_source_ArtParty_id; }
	const CTaskApp*             GetVideoSource() const                            { return m_pVidSrc; }
	const char*                 GetVideoSourceName() const                        { return m_vidSrcName; }

	const char*                 GetSiteName() const                               { return m_siteName; }
	void                        SetSiteName(const char* pSiteName)                { strcpy_safe(m_siteName, pSiteName); }

	DWORD                       GetDecoderDetectedModeWidth() const               { return m_decoderDetectedModeWidth; }
	DWORD                       GetDecoderDetectedModeHeight() const              { return m_decoderDetectedModeHeight; }
	DWORD                       GetDecoderDetectedSampleAspectRatioWidth() const  { return m_decoderDetectedSampleAspectRatioWidth; }
	DWORD                       GetDecoderDetectedSampleAspectRatioHeight() const { return m_decoderDetectedSampleAspectRatioHeight; }

	void                        SetDecoderDetectedParams(DWORD decoderDetectedModeWidth, DWORD decoderDetectedModeHeight, DWORD decoderDetectedSampleAspectRatioWidth, DWORD decoderDetectedSampleAspectRatioHeight,DWORD  decoderDectedMSSvcSsrcID = INVALID, DWORD  decoderDetectedMSSvcPriorityID = INVALID);
	BOOL                        CompareDecoderDetectedParams(const CImage& other) const;

	eVideoResolution            GetVideoResolution() const                        { return m_videoResolution; }

	DWORD                       GetVideoAlgo() const                              { return m_videoAlg; }
	void                        SetVideoAlgo(BYTE alg)                            { m_videoAlg = alg; }

	DWORD                       GetFS() const                                     { return m_FS; }
	DWORD                       GetMBPS() const                                   { return m_MBPS; }

	void                        Serialize(WORD format, CSegment& Seg);
	void                        DeSerialize(WORD format, CSegment& Seg);

	void                        UpdateVideoParams(DWORD videoAlg, eVideoResolution videoResolution, WORD MBPS, WORD FS); // When updating the decoder we need to update it's image as well

	DWORD                       GetConnectionId() const                           { return m_source_connection_id; }
	void                        SetConnectionId(DWORD connId) ;//                    { m_source_connection_id = connId; }

	void                        SetCopDecoderEntityID(DWORD entityID)             { m_source_partyRsrc_id = entityID; }
	DWORD                       GetCopDecoderEntityID() const                     { return m_source_partyRsrc_id; }

	void                        SetCopDecoderConnectionID(DWORD connID)           { m_source_connection_id = connID; }
	DWORD                       GetCopDecoderConnectionID() const                 { return m_source_connection_id; }

	void                        SetDspSmartSwitchEntityId(DWORD entityID)         { m_dspSmartSwitchEntityId = entityID; }
	DWORD                       GetDspSmartSwitchImageEntityId() const            { return m_dspSmartSwitchEntityId; }

	void                        SetDspSmartSwitchConnectionId(DWORD connID)       { m_dspSmartSwitchConnectionId = connID; }
	DWORD                       GetDspSmartSwitchConnectionId() const             { return m_dspSmartSwitchConnectionId; }

	void                        SetCascadeMode(ECascadePartyType cascadeMode)     { m_eCascadeMode = cascadeMode; }
	ECascadePartyType           GetCascadeMode() const                            { return m_eCascadeMode; }

	void                        SetOutOfSync(bool bValue)                         { m_bOutOfSync = bValue; }
	bool                        GetOutOfSync() const                              { return m_bOutOfSync; }

	BOOL                        IsVideoRelayImage() const                         { return m_isVideoRelayImage; }
	DWORD                       GetVideoRelayInChannelHandle() const              { return m_videoRelayInchannelHandle; }

	CVideoRelayMediaStreamList  GetVideoRelayMediaStreamsList() const             { return m_listVideoRelayMediaStreams; }
	bool                        UpdateRelayParams(DWORD channelHandle, const CRelayMediaStreamList& listVideoMediaStreams, CDwordBitMask muteMask);
	bool                        IsVideoMediaStreamChanged(const CRelayMediaStreamList& listVideoMediaStreams);
	bool                        GetVideoMediaStreamInListAccordingToSsrc(DWORD ssrcId, CVideoRelayInMediaStream* & videoMediaStream);
	bool                        IsVideoStreamMuted(DWORD ssrc)const;
	void                        GetSsrcList(std::list<DWORD> & rList)const;
	// AV MCU
	DWORD                       GetVideoMSI() const                                     {return m_videoMSI;}
	void                        setMSI(DWORD videoMSI)                                  {m_videoMSI = videoMSI;}
    // Lync 2013
	DWORD                       GetDecoderDetectedSvcSsrcID() const                             {return m_decoderDetectedMSSvcSsrcID;}
	void                        setDecoderDetectedSsrcID(DWORD decoderDetectedMSSvcSsrcID)               {m_decoderDetectedMSSvcSsrcID = decoderDetectedMSSvcSsrcID;}
	DWORD                       GetDecoderDetectedMSSvcPriorityID() const                               {return m_decoderDetectedMSSvcPriorityID;}
	void                        setDecoderDetectedMSSvcPriorityID(DWORD decoderDetectedMSSvcPriorityID)                 {m_decoderDetectedMSSvcPriorityID = decoderDetectedMSSvcPriorityID;}

	typedef                     BufferedIndicationState<eRtcpPacketLossStatus> NetworkQualityState;

	NetworkQualityState&        networkQualityPerCell() const                     { return m_cellNQ; }
	NetworkQualityState&        networkQualityPerLayout() const                   { return m_selfNQ; }

	// for AvcToSvc
	void 						AddAndChangeRelayParams(DWORD channelHandle, CVideoRelayInMediaStream *videoMediaStreams, bool bIsVideoRelayImage);
	void 						DeleteAndChangeRelayParams(DWORD specificSsrc = 0xFFFFFFFF);

protected:
	void                        DeleteVideoRelayMediaStreamsList();
	bool 						DeleteVideoRelayMediaStreamsListSpecific( DWORD ssrc );

protected:
	DWORD                       m_source_connection_id;
	DWORD                       m_source_partyRsrc_id;        // CP-the ID of the party that is seen in the image,in COP the ID of the Entity seen in the image
	DWORD                       m_source_ArtParty_id;         // CP - NOT in use , COP the ID of the party that is seen in the image
	CTaskApp* const             m_pVidSrc;
	CDwordBitMask               m_mute_mask;
	DWORD                       m_decoderDetectedModeWidth;
	DWORD                       m_decoderDetectedModeHeight;
	DWORD                       m_decoderDetectedSampleAspectRatioWidth;
	DWORD                       m_decoderDetectedSampleAspectRatioHeight;
	eVideoResolution            m_videoResolution;
	DWORD                       m_videoAlg;
	WORD                        m_FS;
	WORD                        m_MBPS;
	char                        m_siteName[MAX_SITE_NAME_ARR_SIZE];
	char                        m_vidSrcName[H243_NAME_LEN];  // as of cached from the GetVideoSource()->GetName();

	// Event Mode MPMx DSP smart switch
	DWORD                       m_dspSmartSwitchEntityId;     // In use of event mode only for the DSP MPMX smart switch solution
	DWORD                       m_dspSmartSwitchConnectionId; // In use of event mode only for the DSP MPMX smart switch solution
	ECascadePartyType           m_eCascadeMode;
	bool                        m_bOutOfSync;

	// the latest "network quality" to compare with
	mutable NetworkQualityState m_cellNQ; // per cell: relates to Video IN only
	mutable NetworkQualityState m_selfNQ; // per layout: incorporates the indication from both Video IN and OUT

	//Video Relay
	BOOL                        m_isVideoRelayImage;
	DWORD                       m_videoRelayInchannelHandle;
	CVideoRelayMediaStreamList  m_listVideoRelayMediaStreams;

	// AV MCU
	DWORD                       m_videoMSI;
	//  Lync 2013
	DWORD                       m_decoderDetectedMSSvcSsrcID;
	DWORD                       m_decoderDetectedMSSvcPriorityID;

private:
	friend BYTE                 operator==(const CImage& left, const CImage& rigth);


	friend class                CBridgePartyVideoIn;
	friend class                CBridgePartyVideoInCOP;
	friend class                CBridgePartyVideoRelayIn;
};


////////////////////////////////////////////////////////////////////////////
//                        CPartyImageLookupTable
////////////////////////////////////////////////////////////////////////////
class CPartyImageLookupTable : public CPObject
{
	CLASS_TYPE_1(CPartyImageLookupTable, CPObject)

public:
	                            CPartyImageLookupTable();
	                           ~CPartyImageLookupTable();

	virtual const char*         NameOf() const { return "CPartyImageLookupTable"; }

	CImage*                     GetPartyImage(DWORD partyRscId);

	friend class                CImage;

private:
	void                        AddPartyImage(DWORD partyRscIdrByName , CImage* pImage);
	void                        DelPartyImage(DWORD partyRscId);
	void                        DumpLookupTable(const char* functionName);

private:
	std::map<DWORD, CImage*>    m_lookupTable;
};



#endif //_CImage_H_

