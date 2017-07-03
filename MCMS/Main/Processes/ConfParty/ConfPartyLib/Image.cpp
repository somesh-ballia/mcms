#include <sstream>
#include "Image.h"
#include "H221.h"
#include "ConfPartyDefines.h"
#include "TaskApp.h"
#include "Party.h"
#include "TraceStream.h"
#include "SipUtils.h"
#include "ConfPartyGlobals.h"
#include "VideoDefines.h"

//--------------------------------------------------------------------------
void SetSizesOfImage(WORD& X_sixe, WORD& Y_sixe, LayoutType layoutType)
{
	switch (layoutType)
	{
		case CP_NO_LAYOUT:
		{
			X_sixe = AUTO;
			Y_sixe = AUTO;
			break;
		}
		case CP_LAYOUT_1X1:
		{
			X_sixe = CIF_X_SIZE;
			Y_sixe = CIF_Y_SIZE;
			break;
		}
		case CP_LAYOUT_1X2:
		{
			X_sixe = QCIF_X_SIZE;
			Y_sixe = QCIF_Y_SIZE;
			break;
		}
		case CP_LAYOUT_2X1:
		{
			X_sixe = QCIF_X_SIZE;
			Y_sixe = QCIF_Y_SIZE;
			break;
		}
		case CP_LAYOUT_1X2_FLEX:
		{
			X_sixe = CIF_X_SIZE/2;
			Y_sixe = 2*CIF_Y_SIZE/3;
			break;
		}
		case CP_LAYOUT_2X2:
		{
			X_sixe = QCIF_X_SIZE;
			Y_sixe = QCIF_Y_SIZE;
			break;
		}
		case CP_LAYOUT_3X3:
		{
			X_sixe = CIF_X_SIZE/3;
			Y_sixe = CIF_Y_SIZE/3;
			break;
		}
		case CP_LAYOUT_1P5:
		{
			X_sixe = CIF_X_SIZE/3;
			Y_sixe = CIF_Y_SIZE/3;
			break;
		}
		case CP_LAYOUT_1P7:
		{
			X_sixe = CIF_X_SIZE/4;
			Y_sixe = CIF_Y_SIZE/4;
			break;
		}
		case CP_LAYOUT_1x2VER:
		{
			X_sixe = CIF_X_SIZE/2;
			Y_sixe = CIF_Y_SIZE;
			break;
		}
		case CP_LAYOUT_1x2HOR:
		{
			X_sixe = CIF_X_SIZE;
			Y_sixe = CIF_Y_SIZE/2;
			break;
		}
		case CP_LAYOUT_1P2VER:
		{
			X_sixe = CIF_X_SIZE/2;
			Y_sixe = CIF_Y_SIZE/2;
			break;
		}
		case CP_LAYOUT_1P2HOR_UP:
		case CP_LAYOUT_1P2HOR:
		{
			X_sixe = CIF_X_SIZE/2;
			Y_sixe = CIF_Y_SIZE/2;
			break;
		}
		case CP_LAYOUT_1P2HOR_UP_RIGHT_FLEX:
		case CP_LAYOUT_1P2HOR_UP_LEFT_FLEX:
		case CP_LAYOUT_1P2HOR_RIGHT_FLEX:
		case CP_LAYOUT_1P2HOR_LEFT_FLEX:
		case CP_LAYOUT_2X2_UP_RIGHT_FLEX:
		case CP_LAYOUT_2X2_UP_LEFT_FLEX:
		case CP_LAYOUT_2X2_DOWN_RIGHT_FLEX:
		case CP_LAYOUT_2X2_DOWN_LEFT_FLEX:
		case CP_LAYOUT_2X2_RIGHT_FLEX:
		case CP_LAYOUT_2X2_LEFT_FLEX:
		{
			X_sixe = 3*CIF_X_SIZE/8;
			Y_sixe = CIF_Y_SIZE/2;
			break;
		}
		case CP_LAYOUT_1P3HOR_UP:
		case CP_LAYOUT_1P3HOR:
		{
			X_sixe = CIF_X_SIZE/3;
			Y_sixe = CIF_Y_SIZE/3;
			break;
		}
		case CP_LAYOUT_1P3VER:
		{
			X_sixe = CIF_X_SIZE/3;
			Y_sixe = CIF_Y_SIZE/3;
			break;
		}
		case CP_LAYOUT_1P8CENT:
		case CP_LAYOUT_1P8UP:
		case CP_LAYOUT_1P8HOR_UP:
		case CP_LAYOUT_1P4HOR_UP:
		case CP_LAYOUT_1P4HOR:
		{
			X_sixe = CIF_X_SIZE/4;
			Y_sixe = CIF_Y_SIZE/4;
			break;
		}
		case CP_LAYOUT_1P4VER:
		{
			X_sixe = CIF_X_SIZE/4;
			Y_sixe = CIF_Y_SIZE/4;
			break;
		}
		case CP_LAYOUT_4X4:
		case CP_LAYOUT_2P8:
		case CP_LAYOUT_2TOP_P8:
		case CP_LAYOUT_1P12:
		{
			X_sixe = CIF_X_SIZE/4;
			Y_sixe = CIF_Y_SIZE/4;
			break;
		}

		case CP_LAYOUT_OVERLAY_ITP_1P4:
		case CP_LAYOUT_OVERLAY_ITP_1P3:
		case CP_LAYOUT_OVERLAY_ITP_1P2:
		case CP_LAYOUT_OVERLAY_1P3:
		case CP_LAYOUT_OVERLAY_1P2:
		case CP_LAYOUT_OVERLAY_1P1:
		{
			X_sixe = QCIF_X_SIZE;
			Y_sixe = QCIF_Y_SIZE;
			break;
		}

		case CP_LAYOUT_1TOP_LEFT_P8:
		{
			X_sixe = CIF_X_SIZE/4;
			Y_sixe = CIF_Y_SIZE/3;
			break;
		}

		default:
		{
			DBGFPASSERT(layoutType+1);
		}
	} // switch
}

//--------------------------------------------------------------------------
BYTE isLayoutWithSpeakerPicture(const LayoutType layoutType)
{
	BYTE res = NO;
	switch (layoutType)
	{
		case CP_LAYOUT_1P5:
		case CP_LAYOUT_1P7:
		case CP_LAYOUT_1P2VER:
		case CP_LAYOUT_1P2HOR:
		case CP_LAYOUT_1P3HOR:
		case CP_LAYOUT_1P3VER:
		case CP_LAYOUT_1P4HOR:
		case CP_LAYOUT_1P8CENT:
		case CP_LAYOUT_1P8UP:
		case CP_LAYOUT_1TOP_LEFT_P8:
		case CP_LAYOUT_1P4VER:
		case CP_LAYOUT_1P2HOR_UP:
		case CP_LAYOUT_1P3HOR_UP:
		case CP_LAYOUT_1P4HOR_UP:
		case CP_LAYOUT_1P8HOR_UP:
		case CP_LAYOUT_1P2HOR_RIGHT_FLEX:
		case CP_LAYOUT_1P2HOR_LEFT_FLEX:
		case CP_LAYOUT_1P2HOR_UP_RIGHT_FLEX:
		case CP_LAYOUT_1P2HOR_UP_LEFT_FLEX:
			res = YES;
			break;

		case CP_LAYOUT_2P8:
		case CP_LAYOUT_1P12:
		case CP_LAYOUT_2TOP_P8:
			res = YES;
			break;

		case CP_LAYOUT_OVERLAY_1P1:
		case CP_LAYOUT_OVERLAY_1P2:
		case CP_LAYOUT_OVERLAY_1P3:
		case CP_LAYOUT_OVERLAY_ITP_1P2:
		case CP_LAYOUT_OVERLAY_ITP_1P3:
		case CP_LAYOUT_OVERLAY_ITP_1P4:
			res = YES;
			break;

		default:
			break;
	}

	return res;
}

//--------------------------------------------------------------------------
WORD GetOldLayoutType(const LayoutType layoutType)
{
	switch (layoutType)
	{
		case CP_LAYOUT_1X1                 : return 0;
		case CP_LAYOUT_1X2                 : return 1;
		case CP_LAYOUT_2X1                 : return 4;
		case CP_LAYOUT_2X2                 : return 5;
		case CP_LAYOUT_3X3                 : return 6;
		case CP_LAYOUT_1P5                 : return 7;
		case CP_LAYOUT_1P7                 : return ONE_SEVEN;
		case CP_LAYOUT_1x2VER              : return ONE_TWO_VER;
		case CP_LAYOUT_1x2HOR              : return ONE_TWO_HOR;
		case CP_LAYOUT_1P2VER              : return ONE_PLUS_TWO_VER;
		case CP_LAYOUT_1P2HOR              : return ONE_PLUS_TWO_HOR;
		case CP_LAYOUT_1P3HOR              : return ONE_PLUS_THREE_HOR;
		case CP_LAYOUT_1P3VER              : return ONE_PLUS_THREE_VER;
		case CP_LAYOUT_1P4HOR              : return ONE_PLUS_FOUR_HOR;
		case CP_LAYOUT_1P4VER              : return ONE_PLUS_FOUR_VER;
		case CP_LAYOUT_1P8CENT             : return ONE_PLUS_EIGHT_CENTRAL;
		case CP_LAYOUT_1P8UP               : return ONE_PLUS_EIGHT_UPPER;
		case CP_LAYOUT_1P2HOR_UP           : return ONE_PLUS_TWO_HOR_UPPER;
		case CP_LAYOUT_1P3HOR_UP           : return ONE_PLUS_THREE_HOR_UPPER;
		case CP_LAYOUT_1P4HOR_UP           : return ONE_PLUS_FOUR_HOR_UPPER;
		case CP_LAYOUT_1P8HOR_UP           : return ONE_PLUS_EIGTH;
		case CP_LAYOUT_4X4                 : return FOUR_FOUR;
		case CP_LAYOUT_2P8                 : return TWO_PLUS_EIGHT;
		case CP_LAYOUT_1P12                : return ONE_PLUS_TWELVE;
		case CP_LAYOUT_1X2_FLEX            : return ONE_TWO_FLEX;
		case CP_LAYOUT_1P2HOR_RIGHT_FLEX   : return ONE_PLUS_TWO_HOR_R_FLEX;
		case CP_LAYOUT_1P2HOR_LEFT_FLEX    : return ONE_PLUS_TWO_HOR_L_FLEX;
		case CP_LAYOUT_1P2HOR_UP_RIGHT_FLEX: return ONE_PLUS_TWO_HOR_UP_R_FLEX;
		case CP_LAYOUT_1P2HOR_UP_LEFT_FLEX : return ONE_PLUS_TWO_HOR_UP_L_FLEX;
		case CP_LAYOUT_2X2_UP_RIGHT_FLEX   : return TWO_TWO_UP_R_FLEX;
		case CP_LAYOUT_2X2_UP_LEFT_FLEX    : return TWO_TWO_UP_L_FLEX;
		case CP_LAYOUT_2X2_DOWN_RIGHT_FLEX : return TWO_TWO_DOWN_R_FLEX;
		case CP_LAYOUT_2X2_DOWN_LEFT_FLEX  : return TWO_TWO_DOWN_L_FLEX;
		case CP_LAYOUT_2X2_RIGHT_FLEX      : return TWO_TWO_R_FLEX;
		case CP_LAYOUT_2X2_LEFT_FLEX       : return TWO_TWO_L_FLEX;
		case CP_LAYOUT_OVERLAY_1P1         : return ONE_PLUS_ONE_OVERLAY;
		case CP_LAYOUT_OVERLAY_1P2         : return ONE_PLUS_TWO_OVERLAY;
		case CP_LAYOUT_OVERLAY_1P3         : return ONE_PLUS_THREE_OVERLAY;
		case CP_LAYOUT_OVERLAY_ITP_1P2     : return ONE_PLUS_TWO_OVERLAY_ITP;
		case CP_LAYOUT_OVERLAY_ITP_1P3     : return ONE_PLUS_THREE_OVERLAY_ITP;
		case CP_LAYOUT_OVERLAY_ITP_1P4     : return ONE_PLUS_FOUR_OVERLAY_ITP;
		case CP_LAYOUT_1TOP_LEFT_P8        : return ONE_TOP_LEFT_PLUS_EIGHT;
		case CP_LAYOUT_2TOP_P8             : return TWO_TOP_PLUS_EIGHT;
		default:
			break;
	}

	FPASSERT_AND_RETURN_VALUE(layoutType, 0);
	return 0;
}

//--------------------------------------------------------------------------
WORD GetNegativeCoord(const WORD wCoord)
{
	WORD wResult = 0;
	wResult = ~wCoord;
	wResult = wResult + 1;
	wResult = wResult & 0xFFFF;
	return wResult;
}
CImage::CImage()
	: m_source_connection_id(INVALID)
	, m_source_partyRsrc_id(INVALID)
	, m_source_ArtParty_id(INVALID)
	, m_pVidSrc(NULL)
	, m_decoderDetectedModeWidth(DEFAULT_DECODER_DETECTED_MODE_WIDTH)
	, m_decoderDetectedModeHeight(DEFAULT_DECODER_DETECTED_MODE_HEIGHT)
	, m_decoderDetectedSampleAspectRatioWidth(DEFAULT_DECODER_DETECTED_SAMPLE_ASPECT_RATIO_WIDTH)
	, m_decoderDetectedSampleAspectRatioHeight(DEFAULT_DECODER_DETECTED_SAMPLE_ASPECT_RATIO_HEIGHT)
	, m_videoResolution(eVideoResolutionDummy)
	, m_videoAlg(0), m_FS(0), m_MBPS(0)
	, m_dspSmartSwitchEntityId(INVALID)
	, m_dspSmartSwitchConnectionId(INVALID)
	, m_eCascadeMode(eCascadeNone),	m_isVideoRelayImage(FALSE),m_videoRelayInchannelHandle(INVALID),m_videoMSI(0xFFFFFFFF)
	, m_decoderDetectedMSSvcSsrcID(INVALID),  m_decoderDetectedMSSvcPriorityID(INVALID)

{
	m_mute_mask.ResetMask();
	m_siteName[0] = 0;
	m_vidSrcName[0] = 0;
	DeleteVideoRelayMediaStreamsList();
}
CImage::CImage(
	DWORD source_connection_id,
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
	ECascadePartyType cascadeMode/* = eCascadeNone*/,
	BOOL isVideoRelayImage/*=FALSE*/,
	DWORD videoRelayInchannelHandle/*=INVALID*/)
	: m_source_connection_id(source_connection_id)
	, m_source_partyRsrc_id(source_partyRsrc_id)
	, m_source_ArtParty_id(source_partyRsrc_id)
	, m_pVidSrc(pVidSrc)
	, m_mute_mask(muteMask)
	, m_decoderDetectedModeWidth(decoderDetectedModeWidth)
	, m_decoderDetectedModeHeight(decoderDetectedModeHeight)
	, m_decoderDetectedSampleAspectRatioWidth(decoderDetectedSampleAspectRatioWidth)
	, m_decoderDetectedSampleAspectRatioHeight(decoderDetectedSampleAspectRatioHeight)
	, m_videoResolution(videoResolution)
	, m_videoAlg(videoAlg)
	, m_FS(FS)
	, m_MBPS(MBPS)
	, m_dspSmartSwitchEntityId(dspSmartSwitchImageEntityId)
	, m_dspSmartSwitchConnectionId(dspSmartSwitchConnectionId)
	, m_eCascadeMode(cascadeMode)
	, m_isVideoRelayImage(isVideoRelayImage)
	, m_videoRelayInchannelHandle(videoRelayInchannelHandle)
	,m_videoMSI(0xFFFFFFFF)
{
	m_bOutOfSync = TRUE;

	strcpy_safe(m_siteName, pSiteName);
	strcpy_safe(m_vidSrcName, pVidSrcName);
	std::ostringstream ostr;
	ostr << ", VideoRelayMediaStream:";
	for (CRelayMediaStreamList::iterator _ii = listVideoMediaStreams.begin(); _ii != listVideoMediaStreams.end(); ++_ii)
	{
		CVideoRelayInMediaStream* pVideoMediaStream = (CVideoRelayInMediaStream*)(*_ii)->NewCopy();
		ostr << std::endl << "ssrc:" << pVideoMediaStream->GetSsrc() << ", layerId:" << ((WORD)(pVideoMediaStream->GetLayerId()));
		m_listVideoRelayMediaStreams.push_back(pVideoMediaStream);
	}
	TRACEINTO
		<< "source_connection_id:"  << m_source_connection_id
		<< ", source_partyRsrc_id:" << m_source_partyRsrc_id
		<< ", siteName:"            << m_siteName
		<< ", vidSrcName:"          << m_vidSrcName
		<< ostr.str().c_str();
	GetPartyImageLookupTable()->AddPartyImage(m_source_ArtParty_id, this);
}

//--------------------------------------------------------------------------
CImage::~CImage()
{
	TRACEINTO
		<< "source_connection_id:"  << m_source_connection_id
		<< ", source_partyRsrc_id:" << m_source_partyRsrc_id
		<< ", siteName:"            << m_siteName
		<< ", vidSrcName:"          << m_vidSrcName;
	GetPartyImageLookupTable()->DelPartyImage(m_source_ArtParty_id);
	DeleteVideoRelayMediaStreamsList();
}

//--------------------------------------------------------------------------
void CImage::SyncFound(DWORD decoderDetectedModeWidth, DWORD decoderDetectedModeHeight, DWORD decoderDetectedSampleAspectRatioWidth, DWORD decoderDetectedSampleAspectRatioHeight,DWORD  decoderDectedMSSvcSsrcID, DWORD  decoderDetectedMSSvcPriorityID)
{
	SetDecoderDetectedParams(decoderDetectedModeWidth, decoderDetectedModeHeight, decoderDetectedSampleAspectRatioWidth, decoderDetectedSampleAspectRatioHeight, decoderDectedMSSvcSsrcID, decoderDetectedMSSvcPriorityID);
	m_mute_mask.ResetBit(SYNC_LOST_Prior);
}

//--------------------------------------------------------------------------
void CImage::DeSerialize(WORD format, CSegment& Seg)
{
	switch (format)
	{
		case SERIALEMBD:
		{
			DBGPASSERT(1);
			return;
		}
		case NATIVE:
		{
			Seg >> (DWORD&)m_pVidSrc;
			return;
		}
	}
	DBGPASSERT(2);
}

//--------------------------------------------------------------------------
void CImage::Serialize(WORD format, CSegment& Seg)
{
	switch (format)
	{
		case SERIALEMBD:
		{
			DBGPASSERT(1);
			return;
		}
		case NATIVE:
		{
			Seg << (DWORD)m_pVidSrc;
			return;
		}
	}
	DBGPASSERT(2);
}

//--------------------------------------------------------------------------
void CImage::SetDecoderDetectedParams(DWORD decoderDetectedModeWidth, DWORD decoderDetectedModeHeight, DWORD decoderDetectedSampleAspectRatioWidth, DWORD decoderDetectedSampleAspectRatioHeight,DWORD  decoderDectedMSSvcSsrcID, DWORD  decoderDetectedMSSvcPriorityID)

{
	m_decoderDetectedModeWidth = decoderDetectedModeWidth;
	m_decoderDetectedModeHeight = decoderDetectedModeHeight;
	m_decoderDetectedSampleAspectRatioWidth = decoderDetectedSampleAspectRatioWidth;
	m_decoderDetectedSampleAspectRatioHeight = decoderDetectedSampleAspectRatioHeight;
	m_decoderDetectedMSSvcSsrcID = decoderDectedMSSvcSsrcID;
	m_decoderDetectedMSSvcPriorityID = decoderDetectedMSSvcPriorityID;
}

//--------------------------------------------------------------------------
void CImage::UpdateVideoParams(DWORD videoAlg, eVideoResolution videoResolution, WORD MBPS, WORD FS)
{
	// When updating the decoder we need to update it's image as well
	m_videoAlg        = videoAlg;
	m_videoResolution = videoResolution;
	m_MBPS            = MBPS;
	m_FS              = FS;
}

//--------------------------------------------------------------------------
BOOL CImage::CompareDecoderDetectedParams(const CImage& other) const
{
	return ((m_decoderDetectedModeWidth == other.m_decoderDetectedModeWidth) &&
	        (m_decoderDetectedModeHeight == other.m_decoderDetectedModeHeight));
}

//--------------------------------------------------------------------------
BYTE operator==(const CImage& left, const CImage& rigth)
{
	if (left.m_source_ArtParty_id == rigth.m_source_ArtParty_id)
	{
		if (left.m_pVidSrc == rigth.m_pVidSrc)
			return YES;
		else
			DBGFPASSERT(1);
	}
	return NO;
}

//--------------------------------------------------------------------------
bool CImage::UpdateRelayParams(DWORD channelHandle, const CRelayMediaStreamList& listVideoMediaStreams, CDwordBitMask muteMask)
{
	bool isImageChanged = false;
	if (m_videoRelayInchannelHandle != channelHandle || m_mute_mask.GetDWORD() != muteMask.GetDWORD())
		isImageChanged = true;
	else
		isImageChanged = IsVideoMediaStreamChanged(listVideoMediaStreams);

	if (isImageChanged)
	{
		m_videoRelayInchannelHandle = channelHandle;
		m_mute_mask                 = muteMask;

		DeleteVideoRelayMediaStreamsList();
		std::ostringstream ostr;
		ostr << "CImage::UpdateRelayParams  : channelHandle=" << channelHandle << std::endl;

		for (CRelayMediaStreamList::const_iterator _ii = listVideoMediaStreams.begin(); _ii != listVideoMediaStreams.end(); ++_ii)
		{
			CVideoRelayInMediaStream* pVideoMediaStream = (CVideoRelayInMediaStream*)(*_ii)->NewCopy();
			m_listVideoRelayMediaStreams.push_back(pVideoMediaStream);
			//temp
			ostr << "ssrc = " << pVideoMediaStream->GetSsrc() << ", layerId=" << (WORD)pVideoMediaStream->GetLayerId()
				 << ", notificationType=" << pVideoMediaStream->m_scpPipe.m_notificationType << ", reason=" << pVideoMediaStream->m_scpPipe.m_reason << std::endl;
		}
		PTRACE(eLevelInfoNormal, ostr.str().c_str());
	}

	return isImageChanged;
}

//--------------------------------------------------------------------------
bool CImage::IsVideoMediaStreamChanged(const CRelayMediaStreamList& listVideoMediaStreams)
{
	if (listVideoMediaStreams.size() != m_listVideoRelayMediaStreams.size())
	{
		PTRACE(eLevelInfoNormal, "CImage::IsVideoMediaStreamChanged different list size");
		return true;
	}

	for (CRelayMediaStreamList::const_iterator _ii = listVideoMediaStreams.begin(); _ii != listVideoMediaStreams.end(); ++_ii)
	{
		bool isVideoMediaStreamInList = true;
		CVideoRelayInMediaStream* videoMediaStream = NULL;
		CVideoRelayInMediaStream* videoMediaStreamSource = (CVideoRelayInMediaStream*)(*_ii);
		int ssrc = videoMediaStreamSource->GetSsrc();
		isVideoMediaStreamInList = GetVideoMediaStreamInListAccordingToSsrc(ssrc, videoMediaStream);
		if (!isVideoMediaStreamInList)
		{
			PTRACE(eLevelInfoNormal, "CImage::IsVideoMediaStreamChanged different ssrc");
			return true;
		}

		BYTE  layerId  = videoMediaStreamSource->GetLayerId();
		DWORD reHeight = videoMediaStreamSource->GetResolutionHeight();
		DWORD resWidth = videoMediaStreamSource->GetResolutionWidth();
		if (videoMediaStream && (layerId != videoMediaStream->GetLayerId() || reHeight != videoMediaStream->GetResolutionHeight() || resWidth != videoMediaStream->GetResolutionWidth()))
		{
			PTRACE(eLevelInfoNormal, "CImage::IsVideoMediaStreamChanged same ssrc: different layerid or resolution");
			return true;
		}

        if( videoMediaStream && (videoMediaStreamSource->m_scpPipe.m_notificationType != videoMediaStream->m_scpPipe.m_notificationType) ) //TODO
        {
            TRACEINTOFUNC << " ssrc = " << ssrc << " has different notifyType: new="
                          << GetScpNotificationTypeEnumValueAsString((eScpNotificationType)videoMediaStreamSource->m_scpPipe.m_notificationType)
                          << ", old=" << GetScpNotificationTypeEnumValueAsString((eScpNotificationType)videoMediaStream->m_scpPipe.m_notificationType);
            return true;
        }
	}

	return false;
}

//--------------------------------------------------------------------------
bool CImage::GetVideoMediaStreamInListAccordingToSsrc(DWORD ssrcId, CVideoRelayInMediaStream*& videoMediaStream)
{
	for (CVideoRelayMediaStreamList::iterator _ii = m_listVideoRelayMediaStreams.begin(); _ii != m_listVideoRelayMediaStreams.end(); ++_ii)
	{
		if ((*_ii)->GetSsrc() == ssrcId)
		{
			videoMediaStream = (*_ii);
			return true;
		}
	}

	return false;
}

//--------------------------------------------------------------------------
void CImage::DeleteVideoRelayMediaStreamsList()
{
	for (CVideoRelayMediaStreamList::iterator _ii = m_listVideoRelayMediaStreams.begin(); _ii != m_listVideoRelayMediaStreams.end(); ++_ii)
		delete (*_ii);
	m_listVideoRelayMediaStreams.clear();
}

bool CImage::DeleteVideoRelayMediaStreamsListSpecific( DWORD ssrc )
{
	int bAllDeleted = true;
	for (CVideoRelayMediaStreamList::iterator _ii = m_listVideoRelayMediaStreams.begin(); _ii != m_listVideoRelayMediaStreams.end(); ++_ii)
	{
		if ((*_ii)->GetSsrc() == ssrc)
			delete (*_ii);
		else
			bAllDeleted = false;
	}
	if (bAllDeleted)
		m_listVideoRelayMediaStreams.clear();
	return bAllDeleted;
}
//--------------------------------------------------------------------------

BYTE CImage::isMuted()const
{
	BYTE result = NO;
	if(m_mute_mask.GetNumberOfSetBits()>0)
		result = YES;

	if( IsVideoRelayImage() )
	{
        bool all_streams_muted = true;
        for(CVideoRelayMediaStreamList::const_iterator it = m_listVideoRelayMediaStreams.begin(); it != m_listVideoRelayMediaStreams.end(); ++it)
        {
            CVideoRelayInMediaStream* pCurStream = (*it);
            if( pCurStream->m_scpPipe.m_notificationType != eStreamCannotBeProvided )
            {
                all_streams_muted = false;
                break;
            }
        }
        if( all_streams_muted )
        {
            TRACEINTOFUNC << "All party streams muted :" << GetPartyRsrcId();
            result = true;
        }
	}
	return result;
}
//--------------------------------------------------------------------------

bool CImage::IsVideoStreamMuted(DWORD ssrc)const
{
    if( !IsVideoRelayImage() )
        return false;

    for(CVideoRelayMediaStreamList::const_iterator it = m_listVideoRelayMediaStreams.begin(); it != m_listVideoRelayMediaStreams.end(); ++it)
    {
        if( (*it)->GetSsrc() == ssrc && (*it)->m_scpPipe.m_notificationType == eStreamCannotBeProvided )
        {
            TRACEINTOFUNC << "DEBUG_SCP: StreamCannotBeProvided , ssrc=" << ssrc;
            return true;
        }
    }
    return false;
}
//--------------------------------------------------------------------------

 void CImage::GetSsrcList(std::list<DWORD> & rListSsrc)const
{
	CVideoRelayMediaStreamList::const_iterator it =  m_listVideoRelayMediaStreams.begin();
	for ( ; it != m_listVideoRelayMediaStreams.end(); ++it)
	{
		rListSsrc.push_back((*it)->GetSsrc());
	}

}

void      CImage::SetConnectionId(DWORD connId)
{
	m_source_connection_id = connId;
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CImage::AddAndChangeRelayParams(DWORD channelHandle, CVideoRelayInMediaStream *videoMediaStreams, bool bIsVideoRelayImage)
{

	std::ostringstream ostr;
	ostr << "CImage::AddAndChangeRelayParams  : channelHandle=" << channelHandle << std::endl;

	m_videoRelayInchannelHandle = channelHandle;
	m_isVideoRelayImage = bIsVideoRelayImage;

	CVideoRelayInMediaStream* videoMediaStream = new CVideoRelayInMediaStream();

		videoMediaStream->m_scpPipe.m_notificationType = videoMediaStreams->m_scpPipe.m_notificationType;
		videoMediaStream->SetSsrc(				videoMediaStreams->GetSsrc());
		videoMediaStream->SetLayerId(			videoMediaStreams->GetLayerId());
		videoMediaStream->SetResolutionHeight(	videoMediaStreams->GetResolutionHeight());
		videoMediaStream->SetResolutionWidth(	videoMediaStreams->GetResolutionWidth());

	m_listVideoRelayMediaStreams.push_back( videoMediaStream );

	//temp (xxx - need to be removed)
	ostr << "ssrc = " << videoMediaStream->GetSsrc() << ", layerId=" << (WORD)videoMediaStream->GetLayerId() << std::endl;
	PTRACE(eLevelInfoNormal, ostr.str().c_str());
//	std::cout<<ostr.str().c_str();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CImage::DeleteAndChangeRelayParams( DWORD specificSsrc )
{
	PTRACE(eLevelInfoNormal, "CImage::DeleteAndChangeRelayParams - Delete All Relay Streams of this Avc Party" );

	if ((DWORD)-1 == specificSsrc)
	{
		m_videoRelayInchannelHandle = INVALID;
		m_isVideoRelayImage = false;
		DeleteVideoRelayMediaStreamsList();
	}
	else
	{
		bool bAllDeleted = DeleteVideoRelayMediaStreamsListSpecific( specificSsrc );
		if (bAllDeleted)
		{
			m_videoRelayInchannelHandle = INVALID;
			m_isVideoRelayImage = false;
		}
	}
}

////////////////////////////////////////////////////////////////////////////
//                        CPartyImageLookupTable
////////////////////////////////////////////////////////////////////////////
CPartyImageLookupTable::CPartyImageLookupTable()
{
}

//--------------------------------------------------------------------------
CPartyImageLookupTable::~CPartyImageLookupTable()
{
}

//--------------------------------------------------------------------------
void CPartyImageLookupTable::AddPartyImage(DWORD partyRscId, CImage* pImage)
{
#ifdef LOOKUP_TABLE_DEBUG_TRACE
	TRACESTRFUNC(eLevelError) << "ImageId:" << partyRscId;
#endif

	std::pair<std::map<DWORD, CImage*>::iterator, bool> rc = m_lookupTable.insert(std::make_pair(partyRscId, pImage));
	PASSERTSTREAM_AND_RETURN(!rc.second, "CPartyImageLookupTable::AddPartyImage - Failed, Element already exists in lookup table, key:" << partyRscId);

#ifdef LOOKUP_TABLE_DEBUG_TRACE
	DumpLookupTable(__PRETTY_FUNCTION__);
#endif
}

//--------------------------------------------------------------------------
void CPartyImageLookupTable::DelPartyImage(DWORD partyRscId)
{
#ifdef LOOKUP_TABLE_DEBUG_TRACE
	TRACESTRFUNC(eLevelError) << "ImageId:" << partyRscId;
#endif

	std::map<DWORD, CImage*>::iterator _ii = m_lookupTable.find(partyRscId);
	PASSERTSTREAM_AND_RETURN(_ii == m_lookupTable.end(), "CPartyImageLookupTable::DelPartyImage - Failed, The lookup table doesn't have an element, key:" << partyRscId);

	m_lookupTable.erase(_ii);

#ifdef LOOKUP_TABLE_DEBUG_TRACE
	DumpLookupTable(__PRETTY_FUNCTION__);
#endif
}

//--------------------------------------------------------------------------
CImage* CPartyImageLookupTable::GetPartyImage(DWORD partyRscId)
{
	std::map<DWORD, CImage*>::const_iterator _ii = m_lookupTable.find(partyRscId);
	if (_ii == m_lookupTable.end())
	{
		DumpLookupTable("GetPartyImage");
		PASSERTSTREAM_AND_RETURN_VALUE(1, "Failed, The lookup table doesn't have an element, ImageId:" << partyRscId, NULL);
	}
	return _ii->second;
}

//--------------------------------------------------------------------------
void CPartyImageLookupTable::DumpLookupTable(const char* functionName)
{
	std::ostringstream msg;
	msg << "called from " << functionName;
	for (std::map<DWORD, CImage*>::const_iterator _ii = m_lookupTable.begin(); _ii != m_lookupTable.end(); ++_ii)
		msg << "\n  " << std::dec << _ii->first << ", " << std::hex << _ii->second;
	TRACESTRFUNC(eLevelError) << msg.str().c_str();
}


