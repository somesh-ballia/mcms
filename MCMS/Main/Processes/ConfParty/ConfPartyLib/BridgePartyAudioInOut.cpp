//+========================================================================+
//                        BridgePartyAudioInOut.cpp                        |
//					      Copyright 2005 Polycom                           |
//                        All Rights Reserved.                             |
//-------------------------------------------------------------------------|
// FILE:       BridgePartyAudioInOut.cpp                                   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Matvey                                                      |
//-------------------------------------------------------------------------|
// Who  | Date  Aug-2005  | Description                                    |
//-------------------------------------------------------------------------|
//
//+========================================================================+

#include "BridgePartyAudioInOut.h"

#include "BridgePartyAudioParams.h"
#include "ConfAppBridgeParams.h"

#include "AudioHardwareInterface.h"
#include "AudioBridgePartyCntl.h"

#include "StatusesGeneral.h"
#include "ConfPartyGlobals.h"

#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsCardMngrIvrCntl.h"
#include "OpcodesMcmsAudio.h"

#include "SysConfig.h"
#include "SysConfigKeys.h"

#include "TraceStream.h"

#include "StlUtils.h"

#define	LEGACY_TO_SAC_KILL_TRANSLATOR_TIMER_0		   ((WORD)303)

extern CConfPartyRoutingTable* GetpConfPartyRoutingTable( void);


/////////////////////////////////////////////////////////////////////////////
// CLASS CBridgePartyAudioUniDirection
/////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CBridgePartyAudioUniDirection)

PEND_MESSAGE_MAP(CBridgePartyAudioUniDirection,CBridgePartyMediaUniDirection);

/////////////////////////////////////////////////////////////////////////////
CBridgePartyAudioUniDirection::CBridgePartyAudioUniDirection() :CBridgePartyMediaUniDirection()
{
	m_dwAudioAlgorithm		=	Au_Neutral;

	m_dwVolume				=	AUDIO_VOLUME_MIN;
	m_byNumberOfChannels	=	0;
	m_byConfSampleRate		=	0;
	m_isVideoParticipant	=	FALSE;
	m_mute_mask.ResetMask();

	m_pWaitingForUpdateParams = NULL;

	m_bIsRelaytoMix            = false;
	m_MsftClientType           = MSFT_CLIENT_NONE_MSFT;
	m_maxAverageBitrate		= 0;

	VALIDATEMESSAGEMAP
}

/////////////////////////////////////////////////////////////////////////////
CBridgePartyAudioUniDirection::CBridgePartyAudioUniDirection (const CBridgePartyAudioUniDirection& rOtherBridgePartyAudioUniDirection)
        :CBridgePartyMediaUniDirection(rOtherBridgePartyAudioUniDirection)
{
	m_dwAudioAlgorithm		=	rOtherBridgePartyAudioUniDirection.m_dwAudioAlgorithm;

	m_dwVolume				=	rOtherBridgePartyAudioUniDirection.m_dwVolume;
	m_byNumberOfChannels	=	rOtherBridgePartyAudioUniDirection.m_byNumberOfChannels;
	m_byConfSampleRate		=	rOtherBridgePartyAudioUniDirection.m_byConfSampleRate;
	m_isVideoParticipant	=	rOtherBridgePartyAudioUniDirection.m_isVideoParticipant;
	m_mute_mask				=   rOtherBridgePartyAudioUniDirection.m_mute_mask;
	m_bIsRelaytoMix          =  rOtherBridgePartyAudioUniDirection.m_bIsRelaytoMix;
	m_MsftClientType        =   rOtherBridgePartyAudioUniDirection.m_MsftClientType;
	m_maxAverageBitrate		=	rOtherBridgePartyAudioUniDirection.m_maxAverageBitrate;	// for Opus codec


	if(rOtherBridgePartyAudioUniDirection.m_pWaitingForUpdateParams)
	{
		m_pWaitingForUpdateParams = new CBridgePartyAudioParams;
		*m_pWaitingForUpdateParams = *(rOtherBridgePartyAudioUniDirection.m_pWaitingForUpdateParams);
	}
	else
	{
		m_pWaitingForUpdateParams = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////
CBridgePartyAudioUniDirection::~CBridgePartyAudioUniDirection()
{

}

/////////////////////////////////////////////////////////////////////////////
CBridgePartyAudioUniDirection& CBridgePartyAudioUniDirection::operator = (const CBridgePartyAudioUniDirection& rOtherBridgePartyAudioUniDirection)
{
	(CBridgePartyMediaUniDirection&)(*this) = (CBridgePartyMediaUniDirection&)rOtherBridgePartyAudioUniDirection;

	m_dwAudioAlgorithm		=	rOtherBridgePartyAudioUniDirection.m_dwAudioAlgorithm;

	m_dwVolume				=	rOtherBridgePartyAudioUniDirection.m_dwVolume;
	m_byNumberOfChannels	=	rOtherBridgePartyAudioUniDirection.m_byNumberOfChannels;
	m_byConfSampleRate		=	rOtherBridgePartyAudioUniDirection.m_byConfSampleRate;
	m_isVideoParticipant	=	rOtherBridgePartyAudioUniDirection.m_isVideoParticipant;
	m_mute_mask				=	rOtherBridgePartyAudioUniDirection.m_mute_mask;
	m_bIsRelaytoMix         =   rOtherBridgePartyAudioUniDirection.m_bIsRelaytoMix;
	m_MsftClientType        =   rOtherBridgePartyAudioUniDirection.m_MsftClientType;
	m_maxAverageBitrate		=	rOtherBridgePartyAudioUniDirection.m_maxAverageBitrate;

	POBJDELETE(m_pWaitingForUpdateParams);
	if(rOtherBridgePartyAudioUniDirection.m_pWaitingForUpdateParams)
	{
		m_pWaitingForUpdateParams = new CBridgePartyAudioParams;
		*m_pWaitingForUpdateParams = *rOtherBridgePartyAudioUniDirection.m_pWaitingForUpdateParams;
	}

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioUniDirection::Create(const CBridgePartyCntl* pBridgePartyCntl,
										   const CRsrcParams* pRsrcParams,
										   const CBridgePartyMediaParams* pBridgePartyMediaParams)
{
	if (m_pHardwareInterface)
		POBJDELETE(m_pHardwareInterface);

	m_pHardwareInterface = new CAudioHardwareInterface();
	m_pHardwareInterface->SetRoomId( pRsrcParams->GetRoomId() );

	CBridgePartyMediaUniDirection::Create(pBridgePartyCntl, pRsrcParams) ;
	m_dwAudioAlgorithm		=	((CBridgePartyAudioParams*)pBridgePartyMediaParams)->GetAudioAlgorithm();

	m_dwVolume				=	((CBridgePartyAudioParams*)pBridgePartyMediaParams)->GetAudioVolume();
	m_byNumberOfChannels	=	((CBridgePartyAudioParams*)pBridgePartyMediaParams)->GetNumberOfChannels();
	m_byConfSampleRate		=	((CBridgePartyAudioParams*)pBridgePartyMediaParams)->GetConfSampleRate();
	m_isVideoParticipant	=	((CBridgePartyAudioParams*)pBridgePartyMediaParams)->IsVideoParticipant();
	m_mute_mask 			= 	((CBridgePartyAudioParams*)pBridgePartyMediaParams)->GetMuteMask();
	m_maxAverageBitrate 	= 	((CBridgePartyAudioParams*)pBridgePartyMediaParams)->GetMaxAverageBitrate() * 1000;

	// temporary for the development stage
	if (m_dwAudioAlgorithm == Au_Opus_64k || m_dwAudioAlgorithm == Au_OpusStereo_128k)
	{
		if (m_maxAverageBitrate < MIN_OPUS_AVERAGE_BITRATE || m_maxAverageBitrate > MAX_OPUS_AVERAGE_BITRATE )
		{
			DWORD orgBitRate = m_maxAverageBitrate;
			if (m_dwAudioAlgorithm == Au_Opus_64k)
				m_maxAverageBitrate = 64*1000;	//????? Amir: not sure this is the correct behavior ...
			else
				m_maxAverageBitrate = 128*1000;	//????? Amir: not sure this is the correct behavior ...

			TRACEINTO << " Opus-Error: BitRate not in range, corrected from: " << orgBitRate << " to: " << m_maxAverageBitrate;
			PASSERT (101);
		}
	}



	m_bIsRelaytoMix  = ((CBridgePartyAudioInParams*)pBridgePartyMediaParams)->GetIsRelaytoMix();
	m_MsftClientType = ((CBridgePartyAudioInParams*)pBridgePartyMediaParams)->GetMsftClientType();
}
/////////////////////////////////////////////////////////////////////////////
WORD CBridgePartyAudioUniDirection::IsMuteSrc(WORD srcRequest)
{
	if(m_mute_mask.IsBitSet(srcRequest))
		return TRUE;
	else
		return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
BOOL CBridgePartyAudioUniDirection::isMuted(void)const
{
	BYTE result = NO;
	if(m_mute_mask.GetNumberOfSetBits()>0)
		result = YES;
	return result;
}
/////////////////////////////////////////////////////////////////////////////
WORD CBridgePartyAudioUniDirection::GetNetworkInterface()
{
	WORD netInterface = AUTO_INTERFACE_TYPE;
	if(m_pBridgePartyCntl)
		netInterface = m_pBridgePartyCntl->GetNetworkInterface();
	else
		PASSERT(1);
	return netInterface;

}
/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioUniDirection::Mute(WORD sourceRequest)
{
	switch (sourceRequest)
	{
		case OPERATOR:
		{
			MuteByOperator();
			
			break;
		}
		case PARTY:
		{
			MuteByParty();
			
			break;
		}
		case MCMS:
		{
			MuteByMcms();
			
			break;
		}
		case ALL:
		{
			MuteByOperator();
			MuteByParty();
			MuteByMcms();
			
			break;
		}
		default:
		{
			TRACEINTO << "CBridgePartyAudioUniDirection::Mute Name - " 
				<< m_pBridgePartyCntl->GetFullName() << " Illegal sourceRequest - "<< sourceRequest;
			PASSERT(1);
		}
	}// end switch
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioUniDirection::UnMute(WORD sourceRequest)
{
	switch (sourceRequest)
	{
		case OPERATOR:
		{
			UnMuteByOperator();
			
			break;
		}
		case PARTY:
		{
			UnMuteByParty();
			
			break;
		}
		case MCMS:
		{
			UnMuteByMcms();
			
			break;
		}
		case ALL:
		{
			UnMuteByOperator();
			UnMuteByParty();
			UnMuteByMcms();
			
			break;
		}
		default:
		{
			TRACEINTO << "CBridgePartyAudioUniDirection::UnMute Name - " 
				<< m_pBridgePartyCntl->GetFullName() << " Illegal sourceRequest - "<< sourceRequest;
			PASSERT(1);
		}
	}// end switch
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioUniDirection::MuteByOperator()
{
	m_mute_mask.SetBit(OPERATOR);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioUniDirection::MuteByParty()
{
	m_mute_mask.SetBit(PARTY);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioUniDirection::MuteByMcms()
{
	m_mute_mask.SetBit(MCMS);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioUniDirection::UnMuteByOperator()
{
	m_mute_mask.ResetBit(OPERATOR);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioUniDirection::UnMuteByParty()
{
	m_mute_mask.ResetBit(PARTY);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioUniDirection::UnMuteByMcms()
{
	m_mute_mask.ResetBit(MCMS);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioUniDirection::UpdateAudioAlgorithm(DWORD newAudioAlgorithm, DWORD maxAverageBitrate)
{
	CSegment* pSeg = new CSegment;

	*pSeg  << newAudioAlgorithm << maxAverageBitrate;

	DispatchEvent(UPDATE_AUDIO_ALGORITHM, pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioUniDirection::UpdateMute(EOnOff eOnOff, WORD srcRequest)
{
	CSegment* pSeg = new CSegment;

	*pSeg  << (BYTE)eOnOff << srcRequest;

	DispatchEvent(UPDATE_MUTE, pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioUniDirection::UpdateAudioVolume(DWORD newAudioVolume)
{
	CSegment* pSeg = new CSegment;

	*pSeg  << newAudioVolume;

	DispatchEvent(UPDATE_AUDIO_VOLUME, pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioUniDirection::IncreaseAudioVolume(BYTE increaseRate)
{
	CSegment* pSeg = new CSegment;

	*pSeg  << increaseRate;

	DispatchEvent(INCREASE_AUDIO_VOLUME, pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioUniDirection::DecreaseAudioVolume(BYTE decreaseRate)
{
	CSegment* pSeg = new CSegment;

	*pSeg  << decreaseRate;

	DispatchEvent(DECREASE_AUDIO_VOLUME, pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioUniDirection::IvrUpdateStandalone(BOOL isStandalone)
{
	CSegment* pSeg = new CSegment;

	*pSeg  << (BYTE)isStandalone;

	DispatchEvent(UPDATE_STANDALONE, pSeg);

	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioUniDirection::SaveAndSendUpdateParams()
{
	if(NULL == m_pWaitingForUpdateParams)
		return;

	//////////Algorithm
	DWORD newAudioAlgorithm = m_pWaitingForUpdateParams->GetAudioAlgorithm();

	DWORD maxAverageBitrate = 0;
	if (newAudioAlgorithm == Au_Opus_64k || newAudioAlgorithm == Au_OpusStereo_128k)
	{
		maxAverageBitrate = m_pWaitingForUpdateParams->GetMaxAverageBitrate();	// for Opus codec
		if (maxAverageBitrate < MIN_OPUS_AVERAGE_BITRATE || maxAverageBitrate > MAX_OPUS_AVERAGE_BITRATE )
		{
			DWORD orgBitRate = maxAverageBitrate;
			if (newAudioAlgorithm == Au_Opus_64k)
				maxAverageBitrate = 64*1024;	//????? Amir: not sure this is the correct behavior ...
			else
				maxAverageBitrate = 128*1024;	//????? Amir: not sure this is the correct behavior ...
			TRACEINTO << " Opus-Error: BitRate not in range, corrected from: " << orgBitRate << " to: " << maxAverageBitrate;
			PASSERT (101);
		}
	}

	// Ignore in case of the same algorithm
	if ( m_dwAudioAlgorithm != newAudioAlgorithm)
	{
		m_dwAudioAlgorithm = newAudioAlgorithm;
		((CAudioHardwareInterface *) m_pHardwareInterface)->UpdateAlgorithm(m_dwAudioAlgorithm, maxAverageBitrate);
	}

	// if Opus algorithm and BitRate was changed, need to update BitRate (new API)
	TRACEINTO << " Opus-Error: need to implement: 'change Opus BitRate' ";

	//////////Mute
	BOOL previouselyMuted = isMuted();
	m_mute_mask = m_pWaitingForUpdateParams->GetMuteMask();
	if(previouselyMuted != isMuted())
	{
		((CAudioHardwareInterface *) m_pHardwareInterface)->UpdateMute(isMuted());
	}

	//////////Volume
	DWORD newAudioVolume = m_pWaitingForUpdateParams->GetAudioVolume();
	// Ignore in case of the same volume value
	if ( m_dwVolume != newAudioVolume)
	{
		m_dwVolume = newAudioVolume;

		if (m_pBridgePartyCntl && m_pBridgePartyCntl->GetIsCallGeneratorConference() &&
			((CAudioHardwareInterface *) m_pHardwareInterface)->GetLogicalRsrcType() == eLogical_audio_encoder)
			((CAudioHardwareInterface *) m_pHardwareInterface)->CGPlayAudioReq(m_dwVolume);
		else
			((CAudioHardwareInterface *) m_pHardwareInterface)->UpdateVolume(m_dwVolume);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioUniDirection::OnAudioBridgePartyUpdateAudioAlgorithmSETUP(CSegment* pSeg)
{
	DWORD newAudioAlgorithm = 0;
	DWORD maxAverageBitrate = 0;
	*pSeg >> newAudioAlgorithm >> maxAverageBitrate;

	if ( ! CBridgePartyAudioParams::IsValidAudioAlgorithm(newAudioAlgorithm) ) {
		DBGPASSERT_AND_RETURN(1);
	}

	if(NULL == m_pWaitingForUpdateParams)
	{
		InitiateUpdateParams();
	}

	m_pWaitingForUpdateParams->SetAudioAlgorithm(newAudioAlgorithm);	// Amir: need to add maxAverageBitrate
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioUniDirection::OnAudioBridgePartyUpdateAudioAlgorithmCONNECTED(CSegment* pSeg)
{
	DWORD newAudioAlgorithm = 0;
	DWORD maxAverageBitrate = 0;

	*pSeg >> newAudioAlgorithm >> maxAverageBitrate;

	if ( ! CBridgePartyAudioParams::IsValidAudioAlgorithm(newAudioAlgorithm) ) {
		DBGPASSERT_AND_RETURN(1);
	}

	// Ignore in case of the same algorithm
	if ( m_dwAudioAlgorithm == newAudioAlgorithm)
		return;

	if (newAudioAlgorithm == Au_Opus_64k || newAudioAlgorithm == Au_OpusStereo_128k)
	{
		maxAverageBitrate *= 1000;	// need to multiply Opus bit-rate with 1000 (as designed)

		if (maxAverageBitrate < MIN_OPUS_AVERAGE_BITRATE || maxAverageBitrate > MAX_OPUS_AVERAGE_BITRATE )
		{
			DWORD orgBitRate = maxAverageBitrate;
			if (newAudioAlgorithm == Au_Opus_64k)
				maxAverageBitrate = 64*1024;	//????? Amir: not sure this is the correct behavior ...
			else
				maxAverageBitrate = 128*1024;	//????? Amir: not sure this is the correct behavior ...
			TRACEINTO << " Opus-Error: BitRate not in range, corrected from: " << orgBitRate << " to: " << maxAverageBitrate;
			PASSERT (101);
		}
		m_maxAverageBitrate = maxAverageBitrate;
	}

	m_dwAudioAlgorithm = newAudioAlgorithm;

	((CAudioHardwareInterface *) m_pHardwareInterface)->UpdateAlgorithm(m_dwAudioAlgorithm, maxAverageBitrate);
}



/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioUniDirection::OnAudioBridgePartyUpdateMuteSETUP(CSegment* pSeg)
{
	EOnOff eOnOff = eOff;
	WORD srcRequest = 0;

	*pSeg >> (BYTE&)eOnOff >> srcRequest ;

	if ( (eOff != eOnOff) && (eOn != eOnOff) ) {
		DBGPASSERT_AND_RETURN(1);
	}

	if(NULL == m_pWaitingForUpdateParams)
		InitiateUpdateParams();

	m_pWaitingForUpdateParams->SetMute(eOnOff, srcRequest);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioUniDirection::OnAudioBridgePartyUpdateMuteCONNECTED(CSegment* pSeg)
{
	EOnOff eOnOff = eOff;
	WORD srcRequest = 0;

	*pSeg >> (BYTE&)eOnOff >> srcRequest ;

	if ((eOff != eOnOff) && (eOn != eOnOff)) 
	{
		DBGPASSERT_AND_RETURN(1);
	}

	BOOL previouselyMuted = isMuted();
	
	if (eOn == eOnOff)
		Mute(srcRequest);
	else
		UnMute(srcRequest);

	if (previouselyMuted == isMuted())
		return;

	((CAudioHardwareInterface *)m_pHardwareInterface)->UpdateMute(isMuted());
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioUniDirection::OnAudioBridgePartyUpdateAudioVolumeSETUP(CSegment* pSeg)
{
	DWORD newAudioVolume;
	*pSeg >> newAudioVolume;

	if ( ! ::IsValidAudioVolume(newAudioVolume) ) {
		DBGPASSERT_AND_RETURN(1);
	}

	if(NULL == m_pWaitingForUpdateParams)
		InitiateUpdateParams();

	m_pWaitingForUpdateParams->SetVolume(newAudioVolume);

	SendUpdateAudioVolumeToDB();
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioUniDirection::OnAudioBridgePartyUpdateAudioVolumeCONNECTED(CSegment* pSeg)
{
	DWORD newAudioVolume;
	*pSeg >> newAudioVolume;

	if ( ! ::IsValidAudioVolume(newAudioVolume) ) {
		DBGPASSERT_AND_RETURN(1);
	}

	// Ignore in case of the same volume value
	if ( m_dwVolume == newAudioVolume)
		return;

	m_dwVolume = newAudioVolume;

	if (m_pBridgePartyCntl && m_pBridgePartyCntl->GetIsCallGeneratorConference() &&
		((CAudioHardwareInterface *) m_pHardwareInterface)->GetLogicalRsrcType() == eLogical_audio_encoder)
		((CAudioHardwareInterface *) m_pHardwareInterface)->CGPlayAudioReq(m_dwVolume);
	else
	((CAudioHardwareInterface *) m_pHardwareInterface)->UpdateVolume(m_dwVolume);

	SendUpdateAudioVolumeToDB();
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioUniDirection::OnAudioBridgePartyIncreaseAudioVolumeSETUP(CSegment* pSeg)
{
	BYTE increaseRate = 0;
	*pSeg >> increaseRate;

	WORD newAudioVolume  = m_dwVolume + increaseRate;

	if (newAudioVolume > AUDIO_VOLUME_MAX)
		newAudioVolume = AUDIO_VOLUME_MAX;

	if(NULL == m_pWaitingForUpdateParams)
		InitiateUpdateParams();

	m_pWaitingForUpdateParams->SetVolume(newAudioVolume);

	SendUpdateAudioVolumeToDB();
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioUniDirection::OnAudioBridgePartyIncreaseAudioVolumeCONNECTED(CSegment* pSeg)
{
	BYTE increaseRate = 0;
	*pSeg >> increaseRate;

	WORD newAudioVolume  = m_dwVolume + increaseRate;

	if (newAudioVolume > AUDIO_VOLUME_MAX)
		newAudioVolume = AUDIO_VOLUME_MAX;

	// Ignore in case of the same volume value
	if ( m_dwVolume == newAudioVolume)
		return;

	m_dwVolume = newAudioVolume;

	((CAudioHardwareInterface *) m_pHardwareInterface)->UpdateVolume(m_dwVolume);

	SendUpdateAudioVolumeToDB();
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioUniDirection::OnAudioBridgePartyDecreaseAudioVolumeSETUP(CSegment* pSeg)
{
	BYTE decreaseRate = 0;
	*pSeg >> decreaseRate;

	WORD newAudioVolume = m_dwVolume - decreaseRate;

	// Block values under AUDIO_VOLUME_MIN
	if(newAudioVolume < AUDIO_VOLUME_MIN)
	  return;

	if(decreaseRate > m_dwVolume)
	{
		newAudioVolume = AUDIO_VOLUME_MIN;
	}

	if(NULL == m_pWaitingForUpdateParams)
		InitiateUpdateParams();

	m_pWaitingForUpdateParams->SetVolume(newAudioVolume);

	SendUpdateAudioVolumeToDB();
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioUniDirection::OnAudioBridgePartyDecreaseAudioVolumeCONNECTED(CSegment* pSeg)
{
	BYTE decreaseRate = 0;
	*pSeg >> decreaseRate;

	WORD newAudioVolume = m_dwVolume - decreaseRate;

	// Block values under AUDIO_VOLUME_MIN
	if(newAudioVolume < AUDIO_VOLUME_MIN)
	  return;

	if(decreaseRate > m_dwVolume)
	{
		newAudioVolume = AUDIO_VOLUME_MIN;
	}

	// Ignore in case of the same volume value
	if ( m_dwVolume == newAudioVolume)
		return;

	m_dwVolume = newAudioVolume;

	((CAudioHardwareInterface *) m_pHardwareInterface)->UpdateVolume(m_dwVolume);

	SendUpdateAudioVolumeToDB();
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioUniDirection::OnAudioBridgePartyUpdateStandaloneCONNECTED(CSegment* pSeg)
{
	BYTE tmp;
	*pSeg >> tmp;

	BOOL isStandalone = tmp ? TRUE : FALSE;

	((CAudioHardwareInterface *) m_pHardwareInterface)->UpdateStandalone(isStandalone);
}
/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioUniDirection::UpdatePartyParams(CBridgePartyAudioParams* pUpdatePartyAudioInitParams)
{
	m_dwAudioAlgorithm			= pUpdatePartyAudioInitParams->GetAudioAlgorithm();
	m_dwVolume					= pUpdatePartyAudioInitParams->GetAudioVolume();
	m_byNumberOfChannels 		= pUpdatePartyAudioInitParams->GetNumberOfChannels();
	m_byConfSampleRate 			= pUpdatePartyAudioInitParams->GetConfSampleRate();
	m_isVideoParticipant		= pUpdatePartyAudioInitParams->IsVideoParticipant();
	m_mute_mask					= pUpdatePartyAudioInitParams->GetMuteMask();
	m_bIsRelaytoMix             = pUpdatePartyAudioInitParams->GetIsRelaytoMix();
	m_maxAverageBitrate			= pUpdatePartyAudioInitParams->GetMaxAverageBitrate() * 1000;	// currently for Opus codec
}

/////////////////////////////////////////////////////////////////////////////
// CLASS CBridgePartyAudioIn
/////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CBridgePartyAudioIn)

	ONEVENT(CONNECT_AUDIO_IN,					IDLE,			CBridgePartyAudioIn::OnAudioBridgePartyConnectIDLE)
	ONEVENT(CONNECT_AUDIO_IN,					SETUP,			CBridgePartyAudioIn::OnAudioBridgePartyConnectSETUP)
	ONEVENT(CONNECT_AUDIO_IN,					CONNECTED,		CBridgePartyAudioIn::OnAudioBridgePartyConnectCONNECTED)
	ONEVENT(CONNECT_AUDIO_IN,					DISCONNECTING,	CBridgePartyAudioIn::OnAudioBridgePartyConnectDISCONNECTING)

	ONEVENT(DISCONNECT_AUDIO_IN,				IDLE,			CBridgePartyAudioIn::OnAudioBridgePartyDisConnectIDLE)
	ONEVENT(DISCONNECT_AUDIO_IN,				SETUP,			CBridgePartyAudioIn::OnAudioBridgePartyDisConnectSETUP)
	ONEVENT(DISCONNECT_AUDIO_IN,				CONNECTED,		CBridgePartyAudioIn::OnAudioBridgePartyDisConnectCONNECTED)
	ONEVENT(DISCONNECT_AUDIO_IN,				DISCONNECTING,	CBridgePartyAudioIn::OnAudioBridgePartyDisConnectDISCONNECTING)

	ONEVENT(UPDATE_AUDIO_ALGORITHM,				SETUP,			CBridgePartyAudioIn::OnAudioBridgePartyUpdateAudioAlgorithmSETUP)
	ONEVENT(UPDATE_AUDIO_ALGORITHM,				CONNECTED,		CBridgePartyAudioIn::OnAudioBridgePartyUpdateAudioAlgorithmCONNECTED)

	ONEVENT(UPDATE_MEDIA_TYPE,				    SETUP,			CBridgePartyAudioIn::OnAudioBridgePartyUpdateMediaTypeSETUP)
	ONEVENT(UPDATE_MEDIA_TYPE,				    CONNECTED,		CBridgePartyAudioIn::OnAudioBridgePartyUpdateMediaTypeCONNECTED)

	ONEVENT(UPDATE_MUTE,						SETUP,			CBridgePartyAudioIn::OnAudioBridgePartyUpdateMuteSETUP)
	ONEVENT(UPDATE_MUTE,						CONNECTED,		CBridgePartyAudioIn::OnAudioBridgePartyUpdateMuteCONNECTED)

	ONEVENT(UPDATE_AUDIO_VOLUME,				SETUP,			CBridgePartyAudioIn::OnAudioBridgePartyUpdateAudioVolumeSETUP)
	ONEVENT(UPDATE_AUDIO_VOLUME,				CONNECTED,		CBridgePartyAudioIn::OnAudioBridgePartyUpdateAudioVolumeCONNECTED)

	ONEVENT(INCREASE_AUDIO_VOLUME,				SETUP,			CBridgePartyAudioIn::OnAudioBridgePartyIncreaseAudioVolumeSETUP)
	ONEVENT(INCREASE_AUDIO_VOLUME,				CONNECTED,		CBridgePartyAudioIn::OnAudioBridgePartyIncreaseAudioVolumeCONNECTED)

	ONEVENT(DECREASE_AUDIO_VOLUME,				SETUP,			CBridgePartyAudioIn::OnAudioBridgePartyDecreaseAudioVolumeSETUP)
	ONEVENT(DECREASE_AUDIO_VOLUME,				CONNECTED,		CBridgePartyAudioIn::OnAudioBridgePartyDecreaseAudioVolumeCONNECTED)

	ONEVENT(UPDATE_NOISE_DETECTION,				SETUP,			CBridgePartyAudioIn::NullActionFunction)
	ONEVENT(UPDATE_NOISE_DETECTION,				CONNECTED,		CBridgePartyAudioIn::OnAudioBridgePartyUpdateNoiseDetectionCONNECTED)

	ONEVENT(UPDATE_AGC_EXEC,					SETUP,			CBridgePartyAudioIn::OnAudioBridgePartyUpdateAgcSETUP)
	ONEVENT(UPDATE_AGC_EXEC,					CONNECTED,		CBridgePartyAudioIn::OnAudioBridgePartyUpdateAgcCONNECTED)

	ONEVENT(UPDATE_STANDALONE,					CONNECTED,      CBridgePartyAudioIn::OnAudioBridgePartyUpdateStandaloneCONNECTED)

	ONEVENT(AUD_DTMF_IND_VAL,					SETUP,			CBridgePartyAudioIn::OnMplDTMFIndSETUP)
	ONEVENT(AUD_DTMF_IND_VAL,					DISCONNECTING,	CBridgePartyAudioIn::OnMplDTMFIndDISCONNECTING)
	ONEVENT(AUD_DTMF_IND_VAL,					CONNECTED,		CBridgePartyAudioIn::OnMplDTMFIndCONNECTED)

	ONEVENT(ACK_IND,							SETUP,			CBridgePartyAudioIn::OnMplAckSETUP)
	ONEVENT(ACK_IND,							DISCONNECTING,	CBridgePartyAudioIn::OnMplAckDISCONNECTING)
	ONEVENT(ACK_IND,							CONNECTED,		CBridgePartyAudioIn::OnMplAckCONNECTED)

	ONEVENT(LEGACY_TO_SAC_TRANSLATOR_CONNECTED, IDLE,          CBridgePartyAudioIn::OnLegacyToSacTranslatorConnectedIDLE)
	ONEVENT(LEGACY_TO_SAC_TRANSLATOR_CONNECTED, SETUP,         CBridgePartyAudioIn::OnLegacyToSacTranslatorConnectedSETUP)
	ONEVENT(LEGACY_TO_SAC_TRANSLATOR_CONNECTED, CONNECTED,     CBridgePartyAudioIn::OnLegacyToSacTranslatorConnectedCONNECTED)
	ONEVENT(LEGACY_TO_SAC_TRANSLATOR_CONNECTED, DISCONNECTING, CBridgePartyAudioIn::OnLegacyToSacTranslatorConnectedDISCONNECTING)


	ONEVENT(LEGACY_TO_SAC_TRANSLATOR_DISCONNECTED, IDLE,          CBridgePartyAudioIn::OnLegacyToSacTranslatorDisconnectedIDLE)
	ONEVENT(LEGACY_TO_SAC_TRANSLATOR_DISCONNECTED, SETUP,         CBridgePartyAudioIn::OnLegacyToSacTranslatorDisconnectedSETUP)
	ONEVENT(LEGACY_TO_SAC_TRANSLATOR_DISCONNECTED, CONNECTED,     CBridgePartyAudioIn::OnLegacyToSacTranslatorDisconnectedCONNECTED)
	ONEVENT(LEGACY_TO_SAC_TRANSLATOR_DISCONNECTED, DISCONNECTING, CBridgePartyAudioIn::OnLegacyToSacTranslatorDisconnectedDISCONNECTING)

	//ONEVENT(LEGACY_TO_SAC_KILL_TRANSLATOR_TIMER_0,  ANYCASE, 	   CBridgePartyAudioIn::OnLegacyToSacTraslatorKillANYCASE)

	ONEVENT(UPGRADE_TO_MIX_AVC_SVC,                 IDLE,          CBridgePartyAudioIn::OnUpgradeToMixAvcSvcIDLE)
	ONEVENT(UPGRADE_TO_MIX_AVC_SVC,                 SETUP,         CBridgePartyAudioIn::OnUpgradeToMixAvcSvcSETUP)
	ONEVENT(UPGRADE_TO_MIX_AVC_SVC,                 CONNECTED,     CBridgePartyAudioIn::OnUpgradeToMixAvcSvcCONNECTED)
	ONEVENT(UPGRADE_TO_MIX_AVC_SVC,                 DISCONNECTING, CBridgePartyAudioIn::OnUpgradeToMixAvcSvcDISCONNECTING)




PEND_MESSAGE_MAP(CBridgePartyAudioIn,CBridgePartyAudioUniDirection);

/////////////////////////////////////////////////////////////////////////////
CBridgePartyAudioIn::CBridgePartyAudioIn() : CBridgePartyAudioUniDirection()
{
	m_isErrorConcealment		=	FALSE;
	m_isAGC						=	FALSE;
	m_isToneRemove				=	FALSE;
	m_isNoiseReduction			=	FALSE;
	m_isT1CptDetection			=	FALSE;
	m_isDtmfDetection			=	TRUE;
	m_isNoiseDetection			=	FALSE;
	m_isVtxSupport				=	FALSE;
	m_byCallDirection			=	0xFF;
	m_byNoiseDetectionThreshold	=	0;
    m_audioCompressedDelay = 0;
	m_isEchoSuppression			=	TRUE;
	m_isKeyboardSuppression     =   FALSE;
	m_isAutoMuteNoisyParties	=	FALSE;
	m_isAudioClarity            =   FALSE;
	m_confSpeakerChangeMode       =   E_CONF_DEFAULT_SPEAKER_CHANGE_MODE;

	m_ssrc                      =  INVALID;

	m_bIsSupportLegacyToSacTranslate = false;
	m_pLegacyToSacTranslator = NULL;
	m_bWaitingForTranslator  = false;
	m_bAckOpenDecoderRecieved = false;
	m_bAckCloseDecoderRecieved = false;
	m_closeTranslatorAckStatus = statOK;

	VALIDATEMESSAGEMAP
}

/////////////////////////////////////////////////////////////////////////////
CBridgePartyAudioIn::CBridgePartyAudioIn (const CBridgePartyAudioIn& rOtherBridgePartyAudioIn)
        :CBridgePartyAudioUniDirection(rOtherBridgePartyAudioIn)
{
	m_isErrorConcealment		=	rOtherBridgePartyAudioIn.m_isErrorConcealment;
	m_isAGC						=	rOtherBridgePartyAudioIn.m_isAGC;
	m_isToneRemove				=	rOtherBridgePartyAudioIn.m_isToneRemove;
	m_isNoiseReduction			=	rOtherBridgePartyAudioIn.m_isNoiseReduction;
	m_isT1CptDetection			=	rOtherBridgePartyAudioIn.m_isT1CptDetection;
	m_isDtmfDetection			=	rOtherBridgePartyAudioIn.m_isDtmfDetection;
	m_isNoiseDetection			=	rOtherBridgePartyAudioIn.m_isNoiseDetection;
	m_isVtxSupport				=	rOtherBridgePartyAudioIn.m_isVtxSupport;
	m_byCallDirection			=	rOtherBridgePartyAudioIn.m_byCallDirection;
	m_byNoiseDetectionThreshold	=	rOtherBridgePartyAudioIn.m_byNoiseDetectionThreshold;
	m_audioCompressedDelay      =   rOtherBridgePartyAudioIn.m_audioCompressedDelay;
	m_isEchoSuppression			=	rOtherBridgePartyAudioIn.m_isEchoSuppression;
	m_isKeyboardSuppression		=	rOtherBridgePartyAudioIn.m_isKeyboardSuppression;
	m_isAutoMuteNoisyParties	=	rOtherBridgePartyAudioIn.m_isAutoMuteNoisyParties;
	m_isAudioClarity            =   rOtherBridgePartyAudioIn.m_isAudioClarity;
	m_confSpeakerChangeMode       =   rOtherBridgePartyAudioIn.m_confSpeakerChangeMode;

	m_bIsSupportLegacyToSacTranslate = rOtherBridgePartyAudioIn.m_bIsSupportLegacyToSacTranslate;
	m_pLegacyToSacTranslator 	= NULL;
	m_bWaitingForTranslator = rOtherBridgePartyAudioIn.m_bWaitingForTranslator;
	m_bAckOpenDecoderRecieved = rOtherBridgePartyAudioIn.m_bAckOpenDecoderRecieved;
	m_bAckCloseDecoderRecieved = rOtherBridgePartyAudioIn.m_bAckCloseDecoderRecieved;
	m_closeTranslatorAckStatus = rOtherBridgePartyAudioIn.m_closeTranslatorAckStatus;

	VALIDATEMESSAGEMAP
}

/////////////////////////////////////////////////////////////////////////////
CBridgePartyAudioIn& CBridgePartyAudioIn::operator = (const CBridgePartyAudioIn& rOtherBridgePartyAudioIn)
{
	if (&rOtherBridgePartyAudioIn == this) 
		return *this;
	
	(CBridgePartyAudioUniDirection&)(*this) = (CBridgePartyAudioUniDirection&)rOtherBridgePartyAudioIn;

	m_isErrorConcealment		=	rOtherBridgePartyAudioIn.m_isErrorConcealment;
	m_isAGC						=	rOtherBridgePartyAudioIn.m_isAGC;
	m_isToneRemove				=	rOtherBridgePartyAudioIn.m_isToneRemove;
	m_isNoiseReduction			=	rOtherBridgePartyAudioIn.m_isNoiseReduction;
	m_isT1CptDetection			=	rOtherBridgePartyAudioIn.m_isT1CptDetection;
	m_isDtmfDetection			=	rOtherBridgePartyAudioIn.m_isDtmfDetection;
	m_isNoiseDetection			=	rOtherBridgePartyAudioIn.m_isNoiseDetection;
	m_isVtxSupport				=	rOtherBridgePartyAudioIn.m_isVtxSupport;
	m_byCallDirection			=	rOtherBridgePartyAudioIn.m_byCallDirection;
	m_byNoiseDetectionThreshold	=	rOtherBridgePartyAudioIn.m_byNoiseDetectionThreshold;
	m_audioCompressedDelay      =   rOtherBridgePartyAudioIn.m_audioCompressedDelay;
	m_isEchoSuppression			=	rOtherBridgePartyAudioIn.m_isEchoSuppression;
	m_isKeyboardSuppression		=	rOtherBridgePartyAudioIn.m_isKeyboardSuppression;
	m_isAutoMuteNoisyParties	=	rOtherBridgePartyAudioIn.m_isAutoMuteNoisyParties;
	m_isAudioClarity            =   rOtherBridgePartyAudioIn.m_isAudioClarity;
	m_confSpeakerChangeMode       =   rOtherBridgePartyAudioIn.m_confSpeakerChangeMode;

	m_bIsSupportLegacyToSacTranslate = rOtherBridgePartyAudioIn.m_bIsSupportLegacyToSacTranslate;
	m_pLegacyToSacTranslator 	= NULL;
	m_bWaitingForTranslator      = rOtherBridgePartyAudioIn.m_bWaitingForTranslator;
	m_bAckOpenDecoderRecieved    = rOtherBridgePartyAudioIn.m_bAckOpenDecoderRecieved;
	m_bAckCloseDecoderRecieved = rOtherBridgePartyAudioIn.m_bAckCloseDecoderRecieved;
	m_closeTranslatorAckStatus = rOtherBridgePartyAudioIn.m_closeTranslatorAckStatus;

	return *this;
}


/////////////////////////////////////////////////////////////////////////////
CBridgePartyAudioIn::~CBridgePartyAudioIn()
{

	POBJDELETE(m_pLegacyToSacTranslator);

}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::Create(const CBridgePartyCntl* pBridgePartyCntl, const CRsrcParams* pRsrcParams,
								 const CBridgePartyMediaParams* pBridgePartyMediaParams, const CConfAppBridgeParams* pConfAppBridgeParams)
{
	PTRACE2INT(eLevelInfoNormal,"CBridgePartyAudioIn::Create - BRIDGE-12931 - resource params initiated with connectionId - ", pRsrcParams->GetConnectionId());

	CBridgePartyAudioUniDirection::Create(pBridgePartyCntl, pRsrcParams, pBridgePartyMediaParams) ;

	TRACEINTO << " m_maxAverageBitrate: " << m_maxAverageBitrate;

	m_isAGC					=	((CBridgePartyAudioInParams*)pBridgePartyMediaParams)->IsAGC();
	m_isVtxSupport			=	((CBridgePartyAudioInParams*)pBridgePartyMediaParams)->IsVtxSupport();
	m_byCallDirection		=	((CBridgePartyAudioInParams*)pBridgePartyMediaParams)->GetCallDirection();
	m_audioCompressedDelay  =   ((CBridgePartyAudioInParams*)pBridgePartyMediaParams)->GetAudioCompressedDelay();
	m_isEchoSuppression     =   ((CBridgePartyAudioInParams*)pBridgePartyMediaParams)->GetEchoSuppression();
	m_isKeyboardSuppression     =   ((CBridgePartyAudioInParams*)pBridgePartyMediaParams)->GetKeyboardSuppression();
	m_isAutoMuteNoisyParties    =   ((CBridgePartyAudioInParams*)pBridgePartyMediaParams)->GetAutoMuteNoisyParties();
	m_isAudioClarity     = ((CBridgePartyAudioInParams*)pBridgePartyMediaParams)->GetIsAudioClarity();
	m_ssrc               = ((CBridgePartyAudioInParams*)pBridgePartyMediaParams)->GetSsrc();
	m_confSpeakerChangeMode = ((CBridgePartyAudioInParams*)pBridgePartyMediaParams)->GetConfSpeakerChangeMode();
	m_bIsSupportLegacyToSacTranslate = ((CBridgePartyAudioInParams*)pBridgePartyMediaParams)->GetIsSupportLegacyToSacTranslate();


	//m_isMuted overwritten in case of MuteIncoming and this is a DIAL IN party
	if ((m_byCallDirection==DIALIN) && (pConfAppBridgeParams->IsMuteIncoming()))
	{
		Mute(MCMS);
	}

	if (pConfAppBridgeParams->GetIsMuteIncomingByOperator())
	{
		Mute(OPERATOR);
	}

	m_isNoiseDetection	 		= 	(pConfAppBridgeParams->IsNoiseDetection()) ? TRUE : FALSE;
	m_byNoiseDetectionThreshold = 	pConfAppBridgeParams->GetNoiseDetectionThreshold();

	m_isErrorConcealment = TRUE; 	// currently hard coded
	m_isToneRemove 		 = TRUE; 	// currently hard coded (Future Use: might also be relevant in GW calls)
	m_isDtmfDetection	 = TRUE; 	// currently hard coded

	m_isNoiseReduction	 = FALSE; 	// not supported in system
	m_isT1CptDetection	 = FALSE; 	// Future Use: will only be relevant in T1Cas


	m_bWaitingForTranslator = false;
	m_bAckOpenDecoderRecieved = false;
	m_bAckCloseDecoderRecieved = false;
	m_closeTranslatorAckStatus = statOK;
}
/////////////////////////////////////////////////////////////////////////////
void*  CBridgePartyAudioIn::GetMessageMap()
{
	return m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void  CBridgePartyAudioIn::HandleEvent(CSegment* pMsg,DWORD msgLen,OPCODE opCode)
{
	//DispatchEvent(opCode,pMsg);
}
/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::Connect()
{
	PASSERTMSG(1, "CBridgePartyAudioIn::Connect : Illegal connect called without IVR Flag");
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::Connect(BYTE isIVR)
{
	CSegment *pSeg = new CSegment;
	*pSeg << isIVR;
	DispatchEvent(CONNECT_AUDIO_IN, pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::DisConnect()
{
	DispatchEvent(DISCONNECT_AUDIO_IN, NULL);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::UpdateNoiseDetection(EOnOff eOnOff, BYTE newNoiseDetectionThreshold)
{
	CSegment* pSeg = new CSegment;

	*pSeg  	<< (BYTE)eOnOff
			<< (BYTE)newNoiseDetectionThreshold;

	DispatchEvent(UPDATE_NOISE_DETECTION, pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::UpdateAGC(EOnOff eOnOff)
{
	CSegment* pSeg = new CSegment;

	*pSeg  	<< (BYTE)eOnOff;

	DispatchEvent(UPDATE_AGC_EXEC, pSeg);
	POBJDELETE(pSeg);
}


/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::UpdateMediaType(EMediaTypeUpdate eMediaTypeUpdate)
{
	if (eMediaTypeUpdateNotNeeded == eMediaTypeUpdate)
		return;

	BOOL isVideoParticipant = FALSE;

	if (eMediaTypeUpdateAudioToVideo == eMediaTypeUpdate)
		isVideoParticipant = TRUE;

	else if (eMediaTypeUpdateVideoToAudio == eMediaTypeUpdate)
		isVideoParticipant = FALSE;

	CSegment* pSeg = new CSegment;
	*pSeg  << (BYTE)isVideoParticipant;

	DispatchEvent(UPDATE_MEDIA_TYPE, pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::InitiateUpdateParams()
{
	POBJDELETE(m_pWaitingForUpdateParams);

	m_pWaitingForUpdateParams = new CBridgePartyAudioInParams;

	m_pWaitingForUpdateParams->SetAudioAlgorithm(m_dwAudioAlgorithm);
	m_pWaitingForUpdateParams->SetMuteMask(m_mute_mask);
	m_pWaitingForUpdateParams->SetVolume(m_dwVolume);
	((CBridgePartyAudioInParams*)m_pWaitingForUpdateParams)->SetAGC(m_isAGC);
	((CBridgePartyAudioParams*)m_pWaitingForUpdateParams)->SetVideoParticipant(m_isVideoParticipant);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::SaveAndSendUpdateParams()
{
	if(NULL == m_pWaitingForUpdateParams)
		return;

	CBridgePartyAudioUniDirection::SaveAndSendUpdateParams();

	BOOL newIsAgc = ((CBridgePartyAudioInParams*)m_pWaitingForUpdateParams)->IsAGC();

	if(newIsAgc != m_isAGC)
	{
		m_isAGC = newIsAgc;
		((CAudioHardwareInterface *) m_pHardwareInterface)->UpdateAGC(m_isAGC);
	}

	BOOL isVideoParticipant = ((CBridgePartyAudioInParams*)m_pWaitingForUpdateParams)->IsVideoParticipant();

	if(isVideoParticipant != m_isAGC)
	{
		m_isVideoParticipant = isVideoParticipant;
		((CAudioHardwareInterface *) m_pHardwareInterface)->UpdateDecoder(m_isVideoParticipant);
	}

}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::SendUpdateAudioVolumeToDB()
{
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD)m_dwVolume;
	m_pBridgePartyCntl->HandleEvent(pSeg, 0, AUDIO_IN_VOLUME_CHANGED);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnAudioBridgePartyConnectIDLE(CSegment* pParam)
{
	TRACEINTO << "Name:" << m_pBridgePartyCntl->GetFullName();

	BYTE isIVR;
	*pParam >> isIVR;

	BOOL isStandalone = isIVR ? TRUE : FALSE;

	DWORD algVolumeAdjustment = 100;
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();

	switch (m_dwAudioAlgorithm)
	{
	// G729
	case G729_8k:
		sysConfig->GetDWORDDataByKey(CFG_KEY_AUDIO_DECODER_GAIN_G729, algVolumeAdjustment);
		break;

	// G711
	case A_Law_OU:
	case U_Law_OU:
	case A_Law_OF:
	case U_Law_OF:
		sysConfig->GetDWORDDataByKey(CFG_KEY_AUDIO_DECODER_GAIN_G711, algVolumeAdjustment);
		break;

	// G722
	case G722_m1:
	case G722_m2:
	case G722_m3:
		sysConfig->GetDWORDDataByKey(CFG_KEY_AUDIO_DECODER_GAIN_G722, algVolumeAdjustment);
		break;

	// G722.1
	case Au_32k:
	case Au_24k:
	case Au_G7221_16k:
	case Au_Siren7_16k:  // Siren7 is the pre-standardized version of G722.1
		sysConfig->GetDWORDDataByKey(CFG_KEY_AUDIO_DECODER_GAIN_G722_1, algVolumeAdjustment);
		break;
	}

	if (::CPObject::IsValidPObjectPtr(m_pHardwareInterface))
	{
		m_state = SETUP;
		m_bAckOpenDecoderRecieved = false;

		// Tsahi - Call Generator SoftMCU
		BOOL isCallGeneratorConf = m_pBridgePartyCntl->GetIsCallGeneratorConference();

		m_lastReqId = ((CAudioHardwareInterface *) m_pHardwareInterface)->OpenDecoder(
			m_pBridgePartyCntl->GetNetworkInterface(),
															m_byConfSampleRate, m_byNumberOfChannels,
															m_dwAudioAlgorithm, isMuted(), m_dwVolume,
															m_isErrorConcealment, m_isAGC, m_isToneRemove,
															m_isNoiseReduction, m_isT1CptDetection,
															m_isDtmfDetection, m_isNoiseDetection,
															m_byNoiseDetectionThreshold, m_isVideoParticipant,
															m_isVtxSupport, m_isEchoSuppression,
															m_byCallDirection, isStandalone,m_audioCompressedDelay,
															m_isKeyboardSuppression, m_isAutoMuteNoisyParties,
															m_isAudioClarity, algVolumeAdjustment, m_ssrc, m_confSpeakerChangeMode,
															isCallGeneratorConf, m_bIsRelaytoMix, m_MsftClientType, m_maxAverageBitrate);

		m_lastReq = AUDIO_OPEN_DECODER_REQ;

		if(IsTranslateLegacyToSacSupported())
		{
			CreateAndConnectLegacyToSacTranslator();
			m_bWaitingForTranslator = true;
		}
	}
	else
		DBGPASSERT_AND_RETURN(1);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnAudioBridgePartyConnectSETUP(CSegment* pParam)
{
	PTRACE2(eLevelError,"CBridgePartyAudioIn::OnAudioBridgePartyConnectSETUP : Ignored!!! : Name - ",m_pBridgePartyCntl->GetFullName());
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnAudioBridgePartyConnectCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelError,"CBridgePartyAudioIn::OnAudioBidgePartyCntlConnectCONNECTED : Already connected! : Name - ",m_pBridgePartyCntl->GetFullName());

	CSegment* pSeg = new CSegment;

	*pSeg  << (BYTE)statOK;

	m_pBridgePartyCntl->HandleEvent(pSeg, 0, AUDIO_IN_CONNECTED);

	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnAudioBridgePartyConnectDISCONNECTING(CSegment* pParam)
{
	PTRACE2(eLevelError,"CBridgePartyAudioIn::OnAudioBidgePartyCntlConnectDISCONNECTING : Ignored!!! : Name - ",m_pBridgePartyCntl->GetFullName());
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnAudioBridgePartyDisConnectIDLE(CSegment* pParam)
{
	PTRACE2(eLevelError,"CBridgePartyAudioIn::OnAudioBridgePartyDisConnectIDLE : Already disconnected! : Name - ",m_pBridgePartyCntl->GetFullName());

	CSegment* pSeg = new CSegment;

	*pSeg  << (BYTE)statOK;

	m_pBridgePartyCntl->HandleEvent(pSeg, 0, AUDIO_IN_DISCONNECTED);

	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnAudioBridgePartyDisConnectSETUP(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CBridgePartyAudioIn::OnAudioBridgePartyDisConnectSETUP : Name - ",m_pBridgePartyCntl->GetFullName());

	OnAudioBridgePartyDisConnect(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnAudioBridgePartyDisConnectCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CBridgePartyAudioIn::OnAudioBridgePartyDisConnectCONNECTED : Name - ",m_pBridgePartyCntl->GetFullName());

	OnAudioBridgePartyDisConnect(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnAudioBridgePartyDisConnectDISCONNECTING(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CBridgePartyAudioIn::OnAudioBridgePartyDisConnectDISCONNECTING : Ignored!!! : Name - ",m_pBridgePartyCntl->GetFullName());
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnAudioBridgePartyDisConnect(CSegment* pParam)
{
	if (::CPObject::IsValidPObjectPtr(m_pHardwareInterface))
	{
		m_state = DISCONNECTING;
		NotifyRelayAudioStreamRemoved();
		InitDisconnectionFlags();
		m_lastReqId = ((CAudioHardwareInterface *) m_pHardwareInterface)->CloseDecoder();
		m_lastReq = AUDIO_CLOSE_DECODER_REQ;

		if(IsValidPObjectPtr(m_pLegacyToSacTranslator))
		{
			m_pLegacyToSacTranslator->Disconnect();
		}
	}
	else
	{
		DBGPASSERT_AND_RETURN(1);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnMplAckSETUP(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CBridgePartyAudioIn::OnMplAckSETUP : Name - ",m_pBridgePartyCntl->GetFullName());

	OPCODE	AckOpcode;
	DWORD	ack_seq_num;
	STATUS	status;

	*pParam >> AckOpcode >> ack_seq_num >> status;

	switch (AckOpcode)
	{
		case	AUDIO_OPEN_DECODER_REQ:
		{
			BYTE	responseStatus = statOK;
			m_bAckOpenDecoderRecieved = true;

			//In case of failure we will update BridgePartyCntl, no need to wait for translator update
			if (status != STATUS_OK)
			{
				PTRACE2(eLevelError, "CBridgePartyAudioIn::OnMplAckSETUP : AUDIO_OPEN_DECODER_REQ Received with Bad Status!!! : Name - ",m_pBridgePartyCntl->GetFullName());

				//Add assert to EMA in case of NACK
				//AddFaultAlarm("NACK on open Audio Decoder",m_pHardwareInterface->GetPartyRsrcId(),status);

				responseStatus = statAudioInOutResourceProblem; 		// statAudioInOutResourceProblem -> Inorder to initiate Kill port on AudioDecoder AudioEncoder
				CSegment* pSeg = new CSegment;
				*pSeg  << (BYTE)responseStatus;
				
				if (responseStatus == statAudioInOutResourceProblem)
					*pSeg  << (BYTE) eMipIn << (BYTE) eMipStatusFail << (BYTE) eMipOpen;

				// Inform BridgePartyCntl
				m_pBridgePartyCntl->HandleEvent(pSeg, 0, AUDIO_IN_CONNECTED);
				POBJDELETE(pSeg);
	   		}
			else
			{
				if (CanBeConnected())
					SetConnected();
			}

			break;
		}
		default:
		{
			CProcessBase * process = CProcessBase::GetProcess();
			std::string str = process->GetOpcodeAsString(AckOpcode);
			TRACEINTO << "CBridgePartyAudioIn::OnMplAckSETUP - ACK_IND Ignored! - Ack Opcode: "<< str.c_str() << " - Name: "<< m_pBridgePartyCntl->GetFullName();
		}
	}// end switch
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::SetConnected()
{
	TRACEINTO << " - Name: "<< m_pBridgePartyCntl->GetFullName();
	
	m_state = CONNECTED;
	
	// check if update required
	if (m_pWaitingForUpdateParams)
	{
		SaveAndSendUpdateParams();
		POBJDELETE(m_pWaitingForUpdateParams)
	}
	
	NotifyRelayAudioStreamChanges();//fix vngsw-326
	
	BYTE	responseStatus = statOK;
	CSegment* pSeg = new CSegment;
	*pSeg  << (BYTE)responseStatus;

	// Inform BridgePartyCntl
	m_pBridgePartyCntl->HandleEvent(pSeg, 0, AUDIO_IN_CONNECTED);

	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnMplAckDISCONNECTING(CSegment * pParam)
{
	PTRACE2(eLevelInfoNormal,"CBridgePartyAudioIn::OnMplAckDISCONNECTING : Name - ",m_pBridgePartyCntl->GetFullName());
	OPCODE	AckOpcode;
	DWORD	ack_seq_num;
	STATUS	status;


	*pParam >> AckOpcode >> ack_seq_num >> status;

	switch(AckOpcode){
	case	AUDIO_CLOSE_DECODER_REQ:
		{
			m_bAckCloseDecoderRecieved = true;
			BYTE	responseStatus = statOK;
			if ( status != STATUS_OK )
			{
				PTRACE2(eLevelError, "CBridgePartyAudioIn::OnMplAckDISCONNECTING : AUDIO_CLOSE_DECODER_REQ Received with Bad Status!!! : Name - ",m_pBridgePartyCntl->GetFullName());
				//Add assert to EMA in case of NACK
				//AddFaultAlarm("NACK on Close Audio Decoder",m_pHardwareInterface->GetPartyRsrcId(),status);
				responseStatus = statAudioInOutResourceProblem; // statAudioInResourceProblem -> Inorder to initiate Kill port on AudioDecoder AudioEncoder
			}

			SetClosePortAckStatus(responseStatus);
			CheckAndInformAllClosed();
			break;
		}
	default:
		{
			CProcessBase * process = CProcessBase::GetProcess();
			std::string str = process->GetOpcodeAsString(AckOpcode);
			TRACEINTO << "CBridgePartyAudioIn::OnMplAckDISCONNECTING - ACK_IND Ignored! - Ack Opcode: "<< str.c_str() << " - Name: "<< m_pBridgePartyCntl->GetFullName();
		}
	}// end switch
}

/////////////////////////////////////////////////////////////////////////////
void  CBridgePartyAudioIn::OnMplAckCONNECTED(CSegment* pParam)
{
	OPCODE	AckOpcode;
	DWORD  ack_seq_num;
	STATUS  status;

	*pParam >> AckOpcode >> ack_seq_num >> status;

	switch(AckOpcode){
	case	IVR_UPDATE_STANDALONE_REQ:
		{
			CSegment* pSeg = new CSegment;
			*pSeg << AckOpcode << ack_seq_num << status << *pParam;

			((CAudioBridgePartyCntl*)m_pBridgePartyCntl)->IvrNotification(ACK_IND, pSeg);
			POBJDELETE(pSeg);
			break;
		}
	default:
		{
			CProcessBase * process = CProcessBase::GetProcess();
			std::string str = process->GetOpcodeAsString(AckOpcode);
			TRACEINTO << "CBridgePartyAudioIn::OnMplAckCONNECTED - ACK_IND Ignored! - Ack Opcode: "<< str.c_str() << " - Name: "<< m_pBridgePartyCntl->GetFullName();
		}
	}// end switch
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnAudioBridgePartyUpdateAudioAlgorithmSETUP(CSegment* pParam)
{
	CBridgePartyAudioUniDirection::OnAudioBridgePartyUpdateAudioAlgorithmSETUP(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnAudioBridgePartyUpdateAudioAlgorithmCONNECTED(CSegment* pParam)
{
	CBridgePartyAudioUniDirection::OnAudioBridgePartyUpdateAudioAlgorithmCONNECTED(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnAudioBridgePartyUpdateMuteSETUP(CSegment* pParam)
{
	CBridgePartyAudioUniDirection::OnAudioBridgePartyUpdateMuteSETUP(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnAudioBridgePartyUpdateMuteCONNECTED(CSegment* pParam)
{
	CBridgePartyAudioUniDirection::OnAudioBridgePartyUpdateMuteCONNECTED(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnAudioBridgePartyUpdateAudioVolumeSETUP(CSegment* pParam)
{
	CBridgePartyAudioUniDirection::OnAudioBridgePartyUpdateAudioVolumeSETUP(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnAudioBridgePartyUpdateAudioVolumeCONNECTED(CSegment* pParam)
{
	CBridgePartyAudioUniDirection::OnAudioBridgePartyUpdateAudioVolumeCONNECTED(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnAudioBridgePartyIncreaseAudioVolumeSETUP(CSegment* pParam)
{
	CBridgePartyAudioUniDirection::OnAudioBridgePartyIncreaseAudioVolumeSETUP(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnAudioBridgePartyIncreaseAudioVolumeCONNECTED(CSegment* pParam)
{
	CBridgePartyAudioUniDirection::OnAudioBridgePartyIncreaseAudioVolumeCONNECTED(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnAudioBridgePartyDecreaseAudioVolumeSETUP(CSegment* pParam)
{
	CBridgePartyAudioUniDirection::OnAudioBridgePartyDecreaseAudioVolumeSETUP(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnAudioBridgePartyDecreaseAudioVolumeCONNECTED(CSegment* pParam)
{
	CBridgePartyAudioUniDirection::OnAudioBridgePartyDecreaseAudioVolumeCONNECTED(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnAudioBridgePartyUpdateAgcSETUP(CSegment* pParam)
{
	if(NULL == m_pWaitingForUpdateParams)
		InitiateUpdateParams();

	EOnOff eOnOff = eOff;

	*pParam >> (BYTE&)eOnOff;

	BOOL newIsAgc = (eOnOff==eOn) ? TRUE : FALSE;

	((CBridgePartyAudioInParams*) m_pWaitingForUpdateParams)->SetAGC(newIsAgc);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnAudioBridgePartyUpdateAgcCONNECTED(CSegment* pParam)
{
	EOnOff eOnOff = eOff;

	*pParam >> (BYTE&)eOnOff;

	BOOL newIsAgc = (eOnOff==eOn) ? TRUE : FALSE;

	// Ignore in case of the same values
	if ( m_isAGC == newIsAgc )
		return;

	m_isAGC = newIsAgc;

	((CAudioHardwareInterface *) m_pHardwareInterface)->UpdateAGC(m_isAGC);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnAudioBridgePartyUpdateMediaTypeSETUP(CSegment* pSeg)
{
	BOOL isVideoParticipant=FALSE;

	*pSeg >> (BYTE&)isVideoParticipant;

	if(NULL == m_pWaitingForUpdateParams)
		InitiateUpdateParams();

	((CBridgePartyAudioParams*)m_pWaitingForUpdateParams)->SetVideoParticipant(isVideoParticipant);
}


/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnAudioBridgePartyUpdateMediaTypeCONNECTED(CSegment* pParam)
{
	EOnOff eOnOff = eOff;

	*pParam >> (BYTE&)eOnOff;

	BOOL isVideoParticipant = (eOnOff==eOn) ? TRUE : FALSE;

	// Ignore in case of the same values
	if ( m_isVideoParticipant == isVideoParticipant )
		return;

	m_isVideoParticipant = isVideoParticipant;

	((CAudioHardwareInterface *) m_pHardwareInterface)->UpdateDecoder(m_isVideoParticipant);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnAudioBridgePartyUpdateNoiseDetectionCONNECTED(CSegment* pParam)
{
	EOnOff eOnOff = eOff;
	BYTE newNoiseDetectionThreshold = 0;

	*pParam >> (BYTE&)eOnOff >> newNoiseDetectionThreshold;

	BOOL newIsNoiseDetection = (eOnOff==eOn) ? TRUE : FALSE;

	if ( ! ::IsValidNoiseDetectionThreshold(newNoiseDetectionThreshold) ) {
		DBGPASSERT_AND_RETURN(1);
	}

	// Ignore in case of the same values
	if ( m_isNoiseDetection == newIsNoiseDetection && m_byNoiseDetectionThreshold == newNoiseDetectionThreshold )
		return;

	m_isNoiseDetection 			= newIsNoiseDetection;
	m_byNoiseDetectionThreshold = newNoiseDetectionThreshold;

	((CAudioHardwareInterface *) m_pHardwareInterface)->UpdateNoiseDetection(m_isNoiseDetection, m_byNoiseDetectionThreshold);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnAudioBridgePartyUpdateStandaloneCONNECTED(CSegment* pParam)
{
	CBridgePartyAudioUniDirection::OnAudioBridgePartyUpdateStandaloneCONNECTED(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnMplDTMFIndSETUP(CSegment* pParam)
{
	PTRACE2(eLevelError,"CBridgePartyAudioIn::OnMplDTMFIndSETUP : Ignored!!! : Name - ",m_pBridgePartyCntl->GetFullName());
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnMplDTMFIndDISCONNECTING(CSegment* pParam)
{
	PTRACE2(eLevelError,"CBridgePartyAudioIn::OnMplDTMFIndDISCONNECTING : Ignored!!! : Name - ",m_pBridgePartyCntl->GetFullName());
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnMplDTMFIndCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelError,"CBridgePartyAudioIn::OnMplDTMFIndCONNECTED : Name - ",m_pBridgePartyCntl->GetFullName());

	((CAudioBridgePartyCntl*)m_pBridgePartyCntl)->IvrNotification(AUD_DTMF_IND_VAL, pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::UpdatePartyInParams(CUpdatePartyAudioInitParams* pUpdatePartyAudioInitParams)
{
	CBridgePartyAudioInParams* pUpdatePartyAudioInitInParams = (CBridgePartyAudioInParams*)(pUpdatePartyAudioInitParams->GetMediaInParams());
	UpdatePartyParams(pUpdatePartyAudioInitInParams);
	m_isAGC						= pUpdatePartyAudioInitInParams->IsAGC();
	m_isVtxSupport				= pUpdatePartyAudioInitInParams->IsVtxSupport();
	m_byCallDirection			= pUpdatePartyAudioInitInParams->GetCallDirection();
	m_isEchoSuppression	 		= pUpdatePartyAudioInitInParams->GetEchoSuppression();
	m_isKeyboardSuppression 	= pUpdatePartyAudioInitInParams->GetKeyboardSuppression();
	m_isAutoMuteNoisyParties	= pUpdatePartyAudioInitInParams->GetAutoMuteNoisyParties();
	m_ssrc               		= pUpdatePartyAudioInitInParams->GetSsrc();


	//m_isMuted overwritten in case of MuteIncoming and this is a DIAL IN party
	if((m_byCallDirection==DIALIN) && ((pUpdatePartyAudioInitParams->GetConfAppParams())->IsMuteIncoming()))
	{
		Mute(MCMS);
	}
	if ((pUpdatePartyAudioInitParams->GetConfAppParams())->GetIsMuteIncomingByOperator())
	{
		Mute(OPERATOR);
	}

	m_isNoiseDetection			= (pUpdatePartyAudioInitParams->GetConfAppParams())->IsNoiseDetection() ? TRUE : FALSE;
	m_byNoiseDetectionThreshold	= (pUpdatePartyAudioInitParams->GetConfAppParams())->GetNoiseDetectionThreshold();


	m_isErrorConcealment = TRUE; 	// currently hard coded
	m_isToneRemove 		 = TRUE; 	// currently hard coded (Future Use: might also be relevant in GW calls)
	m_isDtmfDetection	 = TRUE; 	// currently hard coded

	m_isNoiseReduction	 = FALSE; 	// not supported in system
	m_isT1CptDetection	 = FALSE; 	// Future Use: will only be relevant in T1Cas

}

/////////////////////////////////////////////////////////////////////////////
bool CBridgePartyAudioIn::UpdateAudioDelay(TAudioUpdateCompressedAudioDelayReq* pstAudioDelay)
{
	if (GetState() == CONNECTED)
	{
		return ((CAudioHardwareInterface*)m_pHardwareInterface)->UpdateAudioDelay(pstAudioDelay);
	}
	else
	{
		PTRACE2INT(eLevelInfoNormal,"CBridgePartyAudioIn::UpdateAudioDelay : State != CONNECTED; State = - ", (int)GetState());
		return false;
	}
}
/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::UpdateAudioRelayParams(DWORD ssrc)
{
	PTRACE2INT(eLevelInfoNormal,"CBridgePartyAudioIn::UpdateAudioRelayParams : ssrc = ", ssrc);
	//verify that we didnt update on it already
	if(m_ssrc!=ssrc)
	{
		m_ssrc = ssrc;

		TRtpUpdateRelayReq stRelayParamsIn;
		stRelayParamsIn.unChannelType = kIpAudioChnlType;
		stRelayParamsIn.unChannelDirection = cmCapReceive;
		stRelayParamsIn.nSSRC.numOfSSRC = 1;
		stRelayParamsIn.nSSRC.ssrcList[0] = ssrc;
		stRelayParamsIn.unIvrSsrc = 0; //relevant only for audio out
		stRelayParamsIn.unIvrCsrc = 0; //relevant only for audio out


		//In case of RMX AVC party don't send to video decoder but update translator
		if(!IsSoftMcu() && !(((CAudioBridgePartyCntl*)m_pBridgePartyCntl)->GetIsVideoRelay()))
		{
			if (IsValidPObjectPtr((m_pLegacyToSacTranslator)))
			{
				m_pLegacyToSacTranslator->UpdateAudioRelayParams();
			}
		}
		else
		{
		((CAudioHardwareInterface *) m_pHardwareInterface)->UpdateAudioRelayParamsIn( &stRelayParamsIn );
		}

		NotifyRelayAudioStreamChanges();
	}
	else
	{
		PTRACE2INT(eLevelInfoNormal,"CBridgePartyAudioIn::UpdateAudioRelayParams : we already updated on ssrc = ", ssrc);

	}
}

///////////////////////////////////////////////////////////////////////////////
bool CBridgePartyAudioIn::TranslateAudioAlgoToRelayNotificationsSubCodecType(DWORD dwMcmsAudioAlgorithmOpcode, ECodecSubType& rCodecSubType, int & rBitRate, bool& rStereo)
{
	bool foundAlgorithem = true;
	
	rCodecSubType = eCodecSubTypeUndefined;
	
	switch (dwMcmsAudioAlgorithmOpcode)
	{
		case Au_SirenLPR_Scalable_32k:
		{
			rCodecSubType = eSAC;
			rBitRate = 32;
			rStereo = false;
			break;
		}
		case Au_SirenLPR_Scalable_48k:
		{
			rCodecSubType = eSAC;
			rBitRate = 48;
			rStereo = false;
			break;
		}
		case Au_SirenLPR_Scalable_64k:
		{
			rCodecSubType = eSAC;
			rBitRate = 48;
			rStereo = false;
			break;
		}
		case Au_SirenLPRS_Scalable_64k:
		{
			rCodecSubType = eSAC;
			rBitRate = 64;
			rStereo = true;
			break;
		}
		case Au_SirenLPRS_Scalable_96k:
		{
			rCodecSubType = eSAC;
			rBitRate = 96;
			rStereo = true;
			break;
		}
		case Au_SirenLPRS_Scalable_128k:
		{
			rCodecSubType = eSAC;
			rBitRate = 128;
			rStereo = true;
			break;
		}
		default:
		{
			rCodecSubType = eSAC;
			rBitRate = 48;
			rStereo = false;
			TRACEINTO << "We expect audio algorithm that is notSAC for not relay calls in Mix-mode: audioAlgorithm " << dwMcmsAudioAlgorithmOpcode;
			break;
		}
	}
	
	return foundAlgorithem;
}

///////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::NotifyRelayAudioStreamChanges()
{
	if (m_ssrc!=INVALID && m_ssrc!=0)
	{
		std::list<CMedia> listMedia;
		//currently only one ssrc
		CMedia media;

		media.m_eMediaStatus = eMediaStatusSendOnly;//in event package send is from EP point thus audio receive from mcu point
		media.m_eMediaType = eMediaAudio;
		media.m_dwID = m_ssrc;
		media.m_dwSsrcID = m_ssrc;

		ECodecSubType codecSubType = eCodecSubTypeUndefined;
		int bitRate = 0;
		bool stereo = false;
		bool legalAlgorithm = TranslateAudioAlgoToRelayNotificationsSubCodecType(m_dwAudioAlgorithm, codecSubType, bitRate, stereo);
		
		if (legalAlgorithm)
		{
			media.m_bitRate.m_iMax =bitRate;
			media.m_eCodec = codecSubType;
			media.m_bStereo = stereo;
		}
		
		media.m_eAvailable = eAvailableTrue;
		listMedia.push_back(media);
		
		CMediaList mediaList;

		mediaList.ReplaceMediaAudioIn(listMedia);

		PTRACE2(eLevelInfoNormal, "CBridgePartyAudioIn::NotifyRelayAudioStreamChanges  - ", 
			mediaList.ToString().c_str());
		
		CTaskApp* pPartyTaskApp = m_pBridgePartyCntl->GetPartyTaskApp();
		
		if (IsValidPObjectPtr(pPartyTaskApp))
		{
			CSegment* pSeg = new CSegment;
			
			mediaList.Serialize(pSeg);
			CConfApi* pConfApi = m_pBridgePartyCntl->GetConfApi();
			
			if (IsValidPObjectPtr(pConfApi))
			{
				pConfApi->UpdateDB(pPartyTaskApp, MEDIA, 0, 0, pSeg);
			}
			else
				PASSERT(1);
			
			POBJDELETE(pSeg);
		}
		else
		{
			PTRACE(eLevelInfoNormal, "CBridgePartyAudioIn::NotifyRelayAudioStreamChanges pPartyTaskApp not valid!!!!!");
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void 	CBridgePartyAudioIn::NotifyRelayAudioStreamRemoved()
{
	if(m_ssrc!=INVALID || m_ssrc!=0)
	{
		std::list<unsigned int> listMediaID;
		listMediaID.push_back(m_ssrc);

		unsigned int isUrget = 0;
		CTaskApp* pPartyTaskApp = m_pBridgePartyCntl->GetPartyTaskApp();
		if (IsValidPObjectPtr(pPartyTaskApp))
		{
			CSegment* pSeg = new CSegment;
			CStlUtils::SerializeListWithFlag(listMediaID, isUrget, pSeg);
			CConfApi* pConfApi = m_pBridgePartyCntl->GetConfApi();
			if(IsValidPObjectPtr(pConfApi) && IsValidPObjectPtr(pPartyTaskApp))
			{
				pConfApi->UpdateDB(pPartyTaskApp, MEDIA_REMOVE, 0, 0, pSeg);
				PTRACE2(eLevelInfoNormal, "CBridgePartyAudioIn::NotifyRelayAudioStreamRemoved - ", CStlUtils::ContainerToString(listMediaID).c_str());
			}
			else
			{
				PASSERT(1);
			}
		    POBJDELETE(pSeg);
		}
		else
		{
			PTRACE(eLevelInfoNormal, "CBridgePartyAudioIn::NotifyRelayAudioStreamRemoved pPartyTaskApp not valid!!!!!");
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
CBridgePartyCntl* CBridgePartyAudioIn::GetBridgePartyCntlPtr()const
{
	return m_pBridgePartyCntl;
}
/////////////////////////////////////////////////////////////////////////////
bool CBridgePartyAudioIn::IsTranslateLegacyToSacSupported()
{
	//only in mix conferences
	DWORD confMediaType = m_pBridgePartyCntl->GetConfMediaType();
	if (eMixAvcSvc != confMediaType)
	{
		//temp for mix integration
		TRACEINTO << "Not mix (AVC to SVC) conference, Translator will not be created";
		return false;

	}
	//only on RMX
	if(IsSoftMcu())
	{
		//temp for mix integration
		TRACEINTO << "In softMcu no need to translate Legacy to SAC via translator, done internally in the AMP";
		return false;
	}
	//only for non relay party
	if(((CAudioBridgePartyCntl*)m_pBridgePartyCntl)->GetIsVideoRelay())
	{
		TRACEINTO << "Translate legacy to SAC not relevant for relay party, Translator will not be created";
		return false;
	}
	//need to receive from party via init parameters
	if(!m_bIsSupportLegacyToSacTranslate)
	{
		TRACEINTO << "Translate legacy to SAC wasn't set from party, Translator will not be created";
		return false;
	}
	//translator is supported only when legacy to SAC encoder resources are allocated- error handle
	CRsrcDesc* pRsrcDesc;
	pRsrcDesc = ::GetpConfPartyRoutingTable()->GetPartyRsrcDesc(m_pHardwareInterface->GetPartyRsrcId(),eLogical_legacy_to_SAC_audio_encoder);
	if(!pRsrcDesc)
	{
		TRACEINTO << " LegacyToSac-Translator-Error - No resources to translate legacy to SAC, Translator will not be created";
		return false;
	}

	//temp
	return true;

}
/////////////////////////////////////////////////////////////////////////////

void CBridgePartyAudioIn::TranslateLegacyToSac()
{
	//if(m_pLegacyToSacTranslator)



}
/////////////////////////////////////////////////////
int CBridgePartyAudioIn::CreateAndConnectLegacyToSacTranslator()
{
	CConfPartyRoutingTable* pRoutingTable = ::GetpConfPartyRoutingTable();
	PASSERT_AND_RETURN_VALUE(!pRoutingTable, STATUS_FAIL);

	PASSERT_AND_RETURN_VALUE(!m_pBridgePartyCntl, STATUS_FAIL);
	DWORD partyRsrcId = m_pBridgePartyCntl->GetPartyRsrcID();
	DWORD confRsrcId = m_pBridgePartyCntl->GetConfRsrcID();
	CConfApi* pConfApi = m_pBridgePartyCntl->GetConfApi();

	NewLegacyToSacTranslator();

	std::auto_ptr<CTaskApi> pTaskApiTranslator(new CTaskApi(*pConfApi));
	pTaskApiTranslator->CreateOnlyApi(pConfApi->GetRcvMbx(), m_pLegacyToSacTranslator);

	//create CRsrcParams for the SAC audio encoder
	CRsrcParams sacEncoderRsrcParams(DUMMY_CONNECTION_ID, partyRsrcId,confRsrcId);

	CRsrcDesc* pSacEncoderRsrcDesc = pRoutingTable->AddStateMachinePointerToRoutingTbl(partyRsrcId,eLogical_legacy_to_SAC_audio_encoder, pTaskApiTranslator.get());
	if (!pSacEncoderRsrcDesc)   // Entry not found in Routing Table
	{
		TRACEINTO << " LegacyToSac-Translator-Error -  partyRsrcId: " << partyRsrcId;
		POBJDELETE(m_pLegacyToSacTranslator);
		PASSERT_AND_RETURN_VALUE(1, STATUS_FAIL);
	}

	sacEncoderRsrcParams.SetRsrcDesc(*pSacEncoderRsrcDesc);

	m_pLegacyToSacTranslator->Create(this, &sacEncoderRsrcParams);
	m_pLegacyToSacTranslator->Connect();

	return STATUS_OK;
}
/////////////////////////////////////////////////////
void CBridgePartyAudioIn::NewLegacyToSacTranslator()
{
	POBJDELETE(m_pLegacyToSacTranslator);
	m_pLegacyToSacTranslator = new CLegacyToSacTranslator();
}
////////////////////////////////////////////////////////////
bool CBridgePartyAudioIn::CanBeConnected()
{
	if(m_bAckOpenDecoderRecieved && !m_bWaitingForTranslator )
		return true;
	else
		return false;

}
////////////////////////////////////////////////////////////
void  CBridgePartyAudioIn::CheckAndInformAllClosed()
{
	bool bLegacyToSacTranslatorClosed = true;
	if(IsValidPObjectPtr(m_pLegacyToSacTranslator))
	{
		if(m_pLegacyToSacTranslator->IsActive())
			bLegacyToSacTranslatorClosed = false;

	}
	if (bLegacyToSacTranslatorClosed && m_bAckCloseDecoderRecieved)
		InformInClosed();
}
////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::InitDisconnectionFlags()
{
	m_bAckCloseDecoderRecieved = false;
	SetClosePortAckStatus(statOK);
	m_closeTranslatorAckStatus = statOK;

}
////////////////////////////////////////////////////////////

void  CBridgePartyAudioIn::InformInClosed()
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - State changed to IDLE";
	m_state = IDLE;


	BYTE responseStatus = GetClosePortAckStatus();
	BYTE closeTranslatorStatus = m_closeTranslatorAckStatus;
	if(statOK==responseStatus)
	{
		responseStatus = closeTranslatorStatus;
	}
	InitDisconnectionFlags();

	CSegment* pSeg = new CSegment;
			*pSeg  << (BYTE)responseStatus;
	if(responseStatus == statAudioInOutResourceProblem)
		*pSeg  << (BYTE) eMipIn << (BYTE) eMipStatusFail << (BYTE) eMipClose;

	// Inform BridgePartyCntl
	m_pBridgePartyCntl->HandleEvent(pSeg, 0, AUDIO_IN_DISCONNECTED);
	POBJDELETE(pSeg);
}
////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::RemoveLegacyToSacTranslatorFromRoutingTable()
{
	CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
	CRsrcParams* pRsrcParams = NULL;
	if(pRoutingTbl== NULL)
	{
		PASSERT_AND_RETURN(101);
	}

	if(m_pLegacyToSacTranslator )
	{
		pRsrcParams = m_pLegacyToSacTranslator->GetRsrcParams();
		if( !pRsrcParams )
		{
			PASSERT_AND_RETURN(104);
		}

		if ( STATUS_FAIL == pRoutingTbl->RemoveStateMachinePointerFromRoutingTbl(*pRsrcParams) )
		{
			DBGPASSERT(105);
		}
	}
}
////////////////////////////////////////////////////////////
EStat CBridgePartyAudioIn::AddLegacyToSacTranslatorToRoutingTable()
{
	EStat ret = statOK;
	CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
	if( NULL == pRoutingTbl )
	{
		PASSERT(100);
		return statIllegal;
	}

	DWORD partyRsrcId = m_pBridgePartyCntl->GetPartyRsrcID();
	DWORD confRsrcId = m_pBridgePartyCntl->GetConfRsrcID();
	CConfApi* pConfApi = m_pBridgePartyCntl->GetConfApi();


	std::auto_ptr<CTaskApi> pTaskApiTranslator(new CTaskApi(*pConfApi));
	pTaskApiTranslator->CreateOnlyApi(pConfApi->GetRcvMbx(), m_pLegacyToSacTranslator);

	//create CRsrcParams for the SAC audio encoder
	CRsrcParams sacEncoderRsrcParams(DUMMY_CONNECTION_ID, partyRsrcId,confRsrcId);

	CRsrcDesc* pSacEncoderRsrcDesc = pRoutingTbl->AddStateMachinePointerToRoutingTbl(partyRsrcId,eLogical_legacy_to_SAC_audio_encoder, pTaskApiTranslator.get());
	if (!pSacEncoderRsrcDesc)   // Entry not found in Routing Table
	{
		POBJDELETE(m_pLegacyToSacTranslator);
		PASSERT(101);
		return statIllegal;
	}

	return ret;
}
////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::InitAllLegacyToSacTranslatorFlags()
{
	m_bWaitingForTranslator = false;
}
////////////////////////////////////////////////////////////
//void CBridgePartyAudioIn::OnLegacyToSacTraslatorKillANYCASE(CSegment* pSeg)
//{
//	TRACEINTO << " KEREN DEBUG";
//	DestroyLegacyToSacTranslator();
//}

////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::DestroyLegacyToSacTranslator()
{
	TRACEINTO << "";
	
	if (IsValidPObjectPtr(m_pLegacyToSacTranslator))
	{
		POBJDELETE(m_pLegacyToSacTranslator);
	}
}

///////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::LegacyToSacTranslatorConnected(EStat status)
{
	CSegment* pSeg = new CSegment;
	
	*pSeg << (DWORD)status;
	
	DispatchEvent(LEGACY_TO_SAC_TRANSLATOR_CONNECTED, pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::LegacyToSacTranslatorDisconnected(EStat status)
{
	CSegment* pSeg = new CSegment;
	
	*pSeg << (DWORD)status;
	
	DispatchEvent(LEGACY_TO_SAC_TRANSLATOR_DISCONNECTED, pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::UpgradeToMixAvcSvc()
{
	m_bIsSupportLegacyToSacTranslate = true;
	if(!IsTranslateLegacyToSacSupported())
	{
		TRACEINTO << " Error: IsTranslateLegacyToSacSupported is FALSE";
		PASSERT(1);
		InformUpgradeToMixSvcAvcToBridgePartyCnrl(statIllegal);
	}
	else
	{
		TRACEINTO << " OK, keep with translator";
		DispatchEvent(UPGRADE_TO_MIX_AVC_SVC, NULL); //to see if we use same opcode
	}
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::InformUpgradeToMixSvcAvcToBridgePartyCnrl(EStat responseStatus)
{
	((CAudioBridgePartyCntl* )m_pBridgePartyCntl)->SendEndUpgradeToMixAvcSvcToPartyCntl(responseStatus);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnLegacyToSacTranslatorConnectedSETUP(CSegment* pSeg)
{
	TRACEINTO << "";
	
	m_bWaitingForTranslator = false;
	EStat receivedStatus = statOK;
	*pSeg >> (WORD&)receivedStatus;

	if (receivedStatus != statOK)
	{
		// statAudioInOutResourceProblem -> Inorder to initiate Kill port on AudioDecoder AudioEncoder
		EStat responseStatus = statAudioInOutResourceProblem;
		
		CSegment* pResSeg = new CSegment;
		
		*pResSeg  << (BYTE)responseStatus;
		*pResSeg  << (BYTE) eMipIn << (BYTE) eMipStatusFail << (BYTE) eMipOpen; //maybe different MIP (openTranslator)

 		// Inform BridgePartyCntl
		m_pBridgePartyCntl->HandleEvent(pResSeg, 0, AUDIO_IN_CONNECTED);
		
		POBJDELETE(pResSeg);
	}
	else
	{
		if (CanBeConnected())
		{
			SetConnected();
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnLegacyToSacTranslatorConnectedIDLE(CSegment* pSeg)
{
	TRACEINTO << "Error!!";
}
/////////////////////////////////////////////////////////////////////////////

void CBridgePartyAudioIn::OnLegacyToSacTranslatorConnectedCONNECTED(CSegment* pSeg)
{
	EStat receivedStatus = statOK;
	*pSeg >> (WORD&)receivedStatus;

	TRACEINTO << " - Status=" << (DWORD)receivedStatus;

	InformUpgradeToMixSvcAvcToBridgePartyCnrl(receivedStatus);

	if(statOK != receivedStatus)
	{
		if(IsValidPObjectPtr(m_pLegacyToSacTranslator))
		{
			m_pLegacyToSacTranslator->Disconnect();
		}
	}

}
/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnLegacyToSacTranslatorConnectedDISCONNECTING(CSegment* pSeg)
{
	TRACEINTO << "Do nothing";

}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnLegacyToSacTranslatorDisconnectedDISCONNECTING(CSegment* pSeg)
{
	EStat receivedStatus = statOK;
	*pSeg >> (WORD&)receivedStatus;

	TRACEINTO << ", receivedStatus:"<< receivedStatus;
	m_closeTranslatorAckStatus = receivedStatus;
	OnLegacyToSacTranslatorDisconnected();
	CheckAndInformAllClosed();
}
/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnLegacyToSacTranslatorDisconnectedIDLE(CSegment* pSeg)
{
	TRACEINTO << "Ignore we dont expect disconnect event in this state";
	DBGPASSERT(1);
}
/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnLegacyToSacTranslatorDisconnectedSETUP(CSegment* pSeg)
{
	TRACEINTO << "Ignore we dont expect disconnect event in this state";
	DBGPASSERT(1);
}
/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnLegacyToSacTranslatorDisconnectedCONNECTED(CSegment* pSeg)
{
	OnLegacyToSacTranslatorDisconnected();
	//if we will support down grade in future need to update

}
////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnLegacyToSacTranslatorDisconnected()
{
	RemoveLegacyToSacTranslatorFromRoutingTable();
	InitAllLegacyToSacTranslatorFlags();
	DestroyLegacyToSacTranslator();
}
////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnUpgradeToMixAvcSvcIDLE(CSegment* pSeg)
{
	InformUpgradeToMixSvcAvcToBridgePartyCnrl(statOK);
}
////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnUpgradeToMixAvcSvcSETUP(CSegment* pSeg)
{
	if(!IsValidPObjectPtr(m_pLegacyToSacTranslator))
	{
		CreateAndConnectLegacyToSacTranslator();
		m_bWaitingForTranslator = true;//we will wait for it as part of the connection flow
	}
	else
	{
		if(!(m_pLegacyToSacTranslator->IsActive()))//IDLE state
		{
			PASSERT(1);
		}
	}
	//answer it is upgraded, in case of failure it will part of the connection failure
	InformUpgradeToMixSvcAvcToBridgePartyCnrl(statOK);
}
////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnUpgradeToMixAvcSvcCONNECTED(CSegment* pSeg)
{
	if(!IsValidPObjectPtr(m_pLegacyToSacTranslator))
	{
		TRACEINTO << " create and connect translator";
		int status = CreateAndConnectLegacyToSacTranslator();
		if (STATUS_OK != status)
		{
			TRACEINTO << " LegacyToSac-Translator-Error : Not creating Translator";
			InformUpgradeToMixSvcAvcToBridgePartyCnrl(statIllegal);
		}
	}
	else
	{
		TRACEINTO << " connect an existing translator";
		m_pLegacyToSacTranslator->Connect();//translator already created, just connect
	}
}
////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::OnUpgradeToMixAvcSvcDISCONNECTING(CSegment* pSeg)
{
	TRACEINTO << "illegal action in Disconnecting";
	PASSERT(1);
	InformUpgradeToMixSvcAvcToBridgePartyCnrl(statIllegal);
}
////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::GetPortsOpened(std::map<eLogicalResourceTypes,bool>& isOpenedRsrcMap)
{
    CRsrcParams* pRsrcParams = GetRsrcParams();
    if (pRsrcParams)
    {
    	eLogicalResourceTypes lrt = pRsrcParams->GetLogicalRsrcType();
    	if (IsConnected())
    		isOpenedRsrcMap[lrt] = true;
    }
	if (m_pLegacyToSacTranslator && m_pLegacyToSacTranslator->IsConnected())
		isOpenedRsrcMap[eLogical_legacy_to_SAC_audio_encoder] = true;
}
/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::Export()
{
	if(m_pLegacyToSacTranslator)
	{
		if(m_pLegacyToSacTranslator->IsConnected()||(!m_pLegacyToSacTranslator->IsActive()))
		{
			//remove statemachine pointers
			RemoveLegacyToSacTranslatorFromRoutingTable();

		}
		else
		{
			TRACEINTO << "We shouldnt move during upgrade";
			PASSERT(100);
		}
	}

}
/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::RemoveConfParams()
{
	CBridgePartyMediaUniDirection::RemoveConfParams();
	if(m_pLegacyToSacTranslator)
	{
		m_pLegacyToSacTranslator->UpdateNewConfParams(INVALID);
	}

}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::Import()
{
	if(m_pLegacyToSacTranslator)
	{
		EStat stat = AddLegacyToSacTranslatorToRoutingTable();
		if(stat!=statOK)
		{
			PASSERT_AND_RETURN(1);
		}
		DWORD confRsrcId = m_pBridgePartyCntl->GetConfRsrcID();
		m_pLegacyToSacTranslator->UpdateNewConfParams(confRsrcId);

	}
}
////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::RegisterInTask(CTaskApp* myNewTask)
{
	CStateMachine::RegisterInTask(myNewTask);
	if (m_pLegacyToSacTranslator)
		m_pLegacyToSacTranslator->RegisterInTask(myNewTask);


}
// ///////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioIn::UnregisterInTask()
{
	if (m_pLegacyToSacTranslator)
		m_pLegacyToSacTranslator->UnregisterInTask();

	CStateMachine::UnregisterInTask();
}
////////////////////////////////////////////////////////////////////////////
// CLASS CBridgePartyAudioOut
/////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CBridgePartyAudioOut)

	ONEVENT(CONNECT_AUDIO_OUT,		IDLE,			CBridgePartyAudioOut::OnAudioBridgePartyConnectIDLE)
	ONEVENT(CONNECT_AUDIO_OUT,		SETUP,			CBridgePartyAudioOut::OnAudioBridgePartyConnectSETUP)
	ONEVENT(CONNECT_AUDIO_OUT,		CONNECTED,		CBridgePartyAudioOut::OnAudioBridgePartyConnectCONNECTED)
	ONEVENT(CONNECT_AUDIO_OUT,		DISCONNECTING,	CBridgePartyAudioOut::OnAudioBridgePartyConnectDISCONNECTING)

	ONEVENT(DISCONNECT_AUDIO_OUT,	IDLE,			CBridgePartyAudioOut::OnAudioBridgePartyDisConnectIDLE)
	ONEVENT(DISCONNECT_AUDIO_OUT,	SETUP,			CBridgePartyAudioOut::OnAudioBridgePartyDisConnectSETUP)
	ONEVENT(DISCONNECT_AUDIO_OUT,	CONNECTED,		CBridgePartyAudioOut::OnAudioBridgePartyDisConnectCONNECTED)
	ONEVENT(DISCONNECT_AUDIO_OUT,	DISCONNECTING,	CBridgePartyAudioOut::OnAudioBridgePartyDisConnectDISCONNECTING)

	ONEVENT(UPDATE_AUDIO_ALGORITHM,	SETUP,			CBridgePartyAudioOut::OnAudioBridgePartyUpdateAudioAlgorithmSETUP)
	ONEVENT(UPDATE_AUDIO_ALGORITHM,	CONNECTED,		CBridgePartyAudioOut::OnAudioBridgePartyUpdateAudioAlgorithmCONNECTED)

	ONEVENT(UPDATE_USE_SPEAKER_SSRC_FOR_TX,	SETUP,			CBridgePartyAudioOut::OnAudioBridgePartyUpdateUseSpeakerSsrcForTxSETUP)
	ONEVENT(UPDATE_USE_SPEAKER_SSRC_FOR_TX,	CONNECTED,		CBridgePartyAudioOut::OnAudioBridgePartyUpdateUseSpeakerSsrcForTxCONNECTED)

	ONEVENT(UPDATE_MUTE,			SETUP,			CBridgePartyAudioOut::OnAudioBridgePartyUpdateMuteSETUP)
	ONEVENT(UPDATE_MUTE,			CONNECTED,		CBridgePartyAudioOut::OnAudioBridgePartyUpdateMuteCONNECTED)

	ONEVENT(UPDATE_AUDIO_VOLUME,	SETUP,			CBridgePartyAudioOut::OnAudioBridgePartyUpdateAudioVolumeSETUP)
	ONEVENT(UPDATE_AUDIO_VOLUME,	CONNECTED,		CBridgePartyAudioOut::OnAudioBridgePartyUpdateAudioVolumeCONNECTED)

	ONEVENT(INCREASE_AUDIO_VOLUME,	SETUP,			CBridgePartyAudioOut::OnAudioBridgePartyIncreaseAudioVolumeSETUP)
	ONEVENT(INCREASE_AUDIO_VOLUME,	CONNECTED,		CBridgePartyAudioOut::OnAudioBridgePartyIncreaseAudioVolumeCONNECTED)

	ONEVENT(DECREASE_AUDIO_VOLUME,	SETUP,			CBridgePartyAudioOut::OnAudioBridgePartyDecreaseAudioVolumeSETUP)
	ONEVENT(DECREASE_AUDIO_VOLUME,	CONNECTED,		CBridgePartyAudioOut::OnAudioBridgePartyDecreaseAudioVolumeCONNECTED)

	ONEVENT(UPDATE_STANDALONE,		CONNECTED,      CBridgePartyAudioOut::OnAudioBridgePartyUpdateStandaloneCONNECTED)

	ONEVENT(IVR_PLAY_MESSAGE_REQ,           CONNECTED,      CBridgePartyAudioOut::OnAudioBridgePlayMessageCONNECTED)
	ONEVENT(IVR_STOP_PLAY_MESSAGE_REQ,      CONNECTED,      CBridgePartyAudioOut::OnAudioBridgeStopPlayMessageCONNECTED)
	ONEVENT(IVR_START_IVR_REQ,          	CONNECTED,		CBridgePartyAudioOut::OnAudioBridgeStartIvrSeqCONNECTED)
	ONEVENT(IVR_STOP_IVR_REQ,           	CONNECTED,      CBridgePartyAudioOut::OnAudioBridgeStopIvrSeqCONNECTED)
	ONEVENT(IVR_STOP_IVR_REQ,           	DISCONNECTING,  CBridgePartyAudioOut::OnAudioBridgeStopIvrSeqDISCONNECTING)
	ONEVENT(IVR_PLAY_MUSIC_REQ,				CONNECTED,		CBridgePartyAudioOut::OnAudioBridgePlayMusicCONNECTED)
	ONEVENT(IVR_STOP_PLAY_MUSIC_REQ,        CONNECTED,      CBridgePartyAudioOut::OnAudioBridgeStopPlayMusicCONNECTED)
	ONEVENT(AUDIO_PLAY_TONE_REQ,			CONNECTED,      CBridgePartyAudioOut::OnAudioBridgePlayToneCONNECTED)
	ONEVENT(AUDIO_PLAY_TONE_REQ,			DISCONNECTING,  CBridgePartyAudioOut::OnAudioBridgePlayToneDISCONNECTED)
	ONEVENT(IVR_RECORD_ROLL_CALL_REQ,		CONNECTED,      CBridgePartyAudioOut::OnAudioBridgeRecordRollCallCONNECTED)
	// IVR_STOP_RECORD_ROLL_CALL_REQ sent ANYCASE because it necessay to stop active record roll call to prevent card crash
	ONEVENT(IVR_STOP_RECORD_ROLL_CALL_REQ,	ANYCASE,  CBridgePartyAudioOut::OnAudioBridgeStopRecordRollCall)

	ONEVENT(IVR_RECORD_ROLL_CALL_IND,	SETUP,			CBridgePartyAudioOut::OnMplRecordRollCallIndSETUP)
	ONEVENT(IVR_RECORD_ROLL_CALL_IND,	DISCONNECTING,	CBridgePartyAudioOut::OnMplRecordRollCallIndDISCONNECTING)
	ONEVENT(IVR_RECORD_ROLL_CALL_IND,	CONNECTED,		CBridgePartyAudioOut::OnMplRecordRollCallIndCONNECTED)

	ONEVENT(ACK_IND,				SETUP,			CBridgePartyAudioOut::OnMplAckSETUP)
	ONEVENT(ACK_IND,				DISCONNECTING,	CBridgePartyAudioOut::OnMplAckDISCONNECTING)
	ONEVENT(ACK_IND,				CONNECTED,		CBridgePartyAudioOut::OnMplAckCONNECTED)

	ONEVENT(UPDATE_AUDIO_ON_SEEN_IMAGE, SETUP,			CBridgePartyAudioOut::OnAudioBridgePartyUpdateSeenImageSsrcSETUP)
	ONEVENT(UPDATE_AUDIO_ON_SEEN_IMAGE, CONNECTED,		CBridgePartyAudioOut::OnAudioBridgePartyUpdateSeenImageSsrcCONNECTED)
	ONEVENT(UPDATE_AUDIO_ON_SEEN_IMAGE, DISCONNECTING,	CBridgePartyAudioOut::OnAudioBridgePartyUpdateSeenImageSsrcDISCONNECTING)


PEND_MESSAGE_MAP(CBridgePartyAudioOut,CBridgePartyAudioUniDirection);


/////////////////////////////////////////////////////////////////////////////
CBridgePartyAudioOut::CBridgePartyAudioOut()
{
	for( WORD i=0; i<MAX_NUM_OF_SSRCS; i++ )
        m_ssrc[i] = 0;
    m_ivrSsrc = 0;
	m_bUseSpeakerSsrcForTx = FALSE;
	m_seenImageSSRC = INVALID;

	VALIDATEMESSAGEMAP
}

/////////////////////////////////////////////////////////////////////////////
CBridgePartyAudioOut::CBridgePartyAudioOut (const CBridgePartyAudioOut& rOtherBridgePartyAudioOut)
        :CBridgePartyAudioUniDirection(rOtherBridgePartyAudioOut)
{
    for( WORD i=0; i<MAX_NUM_OF_SSRCS; i++ )
        m_ssrc[i] = 0;
    m_ivrSsrc = rOtherBridgePartyAudioOut.m_ivrSsrc;
    m_bUseSpeakerSsrcForTx = rOtherBridgePartyAudioOut.m_bUseSpeakerSsrcForTx;

	VALIDATEMESSAGEMAP
};

/////////////////////////////////////////////////////////////////////////////
CBridgePartyAudioOut::~CBridgePartyAudioOut()
{

}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::Create(const CBridgePartyCntl* pBridgePartyCntl, const CRsrcParams* pRsrcParams,
								 const CBridgePartyMediaParams * pBridgePartyMediaParams)
{
	PTRACE2INT(eLevelInfoNormal,"CBridgePartyAudioOut::Create - BRIDGE-12931 - resource params initiated with connectionId - ", pRsrcParams->GetConnectionId());
	CBridgePartyAudioUniDirection::Create(pBridgePartyCntl, pRsrcParams, pBridgePartyMediaParams);

	TRACEINTO << " m_maxAverageBitrate: " << m_maxAverageBitrate;


	m_numOfSsrcIds = ((CBridgePartyAudioOutParams*)pBridgePartyMediaParams)->GetSsrcNum();
	for( WORD i=0; i<m_numOfSsrcIds && i<MAX_NUM_OF_SSRCS; i++ )
	    m_ssrc[i] = ((CBridgePartyAudioOutParams*)pBridgePartyMediaParams)->GetSsrc(i);
	m_ivrSsrc = ((CBridgePartyAudioOutParams*)pBridgePartyMediaParams)->GetIvrSsrc();

	m_bUseSpeakerSsrcForTx = ((CBridgePartyAudioOutParams*)pBridgePartyMediaParams)->GetUseSpeakerSsrcForTx();
}

/////////////////////////////////////////////////////////////////////////////
void*  CBridgePartyAudioOut::GetMessageMap(){
	return m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void  CBridgePartyAudioOut::HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode)
{
	DispatchEvent(opCode,pMsg);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::Connect()
{
	PASSERTMSG(1, "CBridgePartyAudioOut::Connect : Illegal connect called without IVR Flag");
}
/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::Connect(BYTE isIVR)
{
	CSegment *pSeg = new CSegment;
	*pSeg << isIVR;
	DispatchEvent(CONNECT_AUDIO_OUT, pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::DisConnect()
{
	DispatchEvent(DISCONNECT_AUDIO_OUT, NULL);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::IvrCommand(OPCODE opcode, DWORD seqNumToken, CSegment *pDataSeg)
{
	CSegment *pSeg = new CSegment;
	
	*pSeg 	<< (DWORD)seqNumToken
	 		<< *pDataSeg;
	
	DispatchEvent(opcode, pSeg);

	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::InitiateUpdateParams()
{
	POBJDELETE(m_pWaitingForUpdateParams);

	m_pWaitingForUpdateParams = new CBridgePartyAudioOutParams;

	m_pWaitingForUpdateParams->SetAudioAlgorithm(m_dwAudioAlgorithm);
	m_pWaitingForUpdateParams->SetMuteMask(m_mute_mask);
	m_pWaitingForUpdateParams->SetVolume(m_dwVolume);
	((CBridgePartyAudioOutParams*)m_pWaitingForUpdateParams)->SetSeenImageSsrc(m_seenImageSSRC);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::SaveAndSendUpdateParams()
{
	if(NULL == m_pWaitingForUpdateParams)
		return;

	CBridgePartyAudioUniDirection::SaveAndSendUpdateParams();

	// speakerIndication - update
	//////////UseSpeakerSsrcForTx
	DWORD updatedUseSpeakerSsrcForTx = ((CBridgePartyAudioOutParams*)m_pWaitingForUpdateParams)->GetUseSpeakerSsrcForTx();
	// Ignore in case of the same useSpeakerSsrcForTx value
	if ( m_bUseSpeakerSsrcForTx != updatedUseSpeakerSsrcForTx)
	{
		m_bUseSpeakerSsrcForTx = updatedUseSpeakerSsrcForTx;
		((CAudioHardwareInterface *) m_pHardwareInterface)->UpdateUseSpeakerSsrcForTx(m_bUseSpeakerSsrcForTx);
	}
	DWORD updateImageSSRC = ((CBridgePartyAudioOutParams*)m_pWaitingForUpdateParams)->GetSeenImageSsrc();
	if(m_seenImageSSRC!=updateImageSSRC)
	{
		m_seenImageSSRC = updateImageSSRC;
		SendAudioEncoderUpdateSeenImageSsrcToHardware(m_seenImageSSRC);
	}

}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::SendSequenceNumIndicationToCAM(OPCODE opcode, DWORD seqNumToken, DWORD sequenceNum)
{
	CSegment *pSeg = new CSegment;
	*pSeg << opcode << seqNumToken << sequenceNum;
	((CAudioBridgePartyCntl*)m_pBridgePartyCntl)->IvrNotification(SEQUENCE_NUM_IND, pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::SendUpdateAudioVolumeToDB()
{
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD)m_dwVolume;
	m_pBridgePartyCntl->HandleEvent(pSeg, 0, AUDIO_OUT_VOLUME_CHANGED);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnAudioBridgePartyConnectIDLE(CSegment* pParam)
{
	TRACEINTO << "Name:" << m_pBridgePartyCntl->GetFullName();

	BYTE isIVR;
	*pParam >> isIVR;

	BOOL isStandalone = isIVR ? TRUE : FALSE;

	DWORD algVolumeAdjustment = 100;
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();

	switch (m_dwAudioAlgorithm)
	{
	// G729
	case G729_8k:
		sysConfig->GetDWORDDataByKey(CFG_KEY_AUDIO_ENCODER_GAIN_G729, algVolumeAdjustment);
		break;

	// G711
	case A_Law_OU:
	case U_Law_OU:
	case A_Law_OF:
	case U_Law_OF:
		sysConfig->GetDWORDDataByKey(CFG_KEY_AUDIO_ENCODER_GAIN_G711, algVolumeAdjustment);
		break;

	// G722
	case G722_m1:
	case G722_m2:
	case G722_m3:
		sysConfig->GetDWORDDataByKey(CFG_KEY_AUDIO_ENCODER_GAIN_G722, algVolumeAdjustment);
		break;

	// G722.1
	case Au_32k:
	case Au_24k:
	case Au_G7221_16k:
	case Au_Siren7_16k:  // Siren7 is the pre-standardized version of G722.1
		sysConfig->GetDWORDDataByKey(CFG_KEY_AUDIO_ENCODER_GAIN_G722_1, algVolumeAdjustment);
		break;
	}

	if (::CPObject::IsValidPObjectPtr(m_pHardwareInterface))
	{
		m_state = SETUP;
		SetClosePortAckStatus(STATUS_OK);
		EMixModeGet eMixModeSet = E_MIX_MODE_GET_OTHERS;

		// Tsahi - Call Generator SoftMCU
		BOOL isCallGeneratorConf = m_pBridgePartyCntl->GetIsCallGeneratorConference();

		m_lastReqId = ((CAudioHardwareInterface *)m_pHardwareInterface)->OpenEncoder(
			m_pBridgePartyCntl->GetNetworkInterface(),
			m_byConfSampleRate, m_byNumberOfChannels,
			m_dwAudioAlgorithm, isMuted(), m_dwVolume,
			m_isVideoParticipant, isStandalone, algVolumeAdjustment, m_numOfSsrcIds, m_ssrc,
			eMixModeSet,m_ivrSsrc, m_bUseSpeakerSsrcForTx, isCallGeneratorConf, m_bIsRelaytoMix, m_maxAverageBitrate);

		// speakerIndication
		TRACEINTO << "speakerIndication - send OPEN_ENCODER - m_bUseSpeakerSsrcForTx:" << BOOL2STR(m_bUseSpeakerSsrcForTx);

		m_lastReq = AUDIO_OPEN_ENCODER_REQ;
	}
	else
		DBGPASSERT_AND_RETURN(1);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnAudioBridgePartyConnectSETUP(CSegment* pParam)
{
	PTRACE2(eLevelError,"CBridgePartyAudioOut::OnAudioBridgePartyConnectSETUP : Ignored!!! : Name - ",m_pBridgePartyCntl->GetFullName());
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnAudioBridgePartyConnectCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelError,"CBridgePartyAudioOut::OnAudioBidgePartyCntlConnectCONNECTED : Already connected! : Name - ",m_pBridgePartyCntl->GetFullName());

	CSegment* pSeg = new CSegment;

	*pSeg  << (BYTE)statOK;

	m_pBridgePartyCntl->HandleEvent(pSeg, 0, AUDIO_OUT_CONNECTED);

	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnAudioBridgePartyConnectDISCONNECTING(CSegment* pParam)
{
	PTRACE2(eLevelError,"CBridgePartyAudioOut::OnAudioBridgePartyConnectDISCONNECTING : Ignored!!! : Name - ",m_pBridgePartyCntl->GetFullName());
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnAudioBridgePartyDisConnectIDLE(CSegment* pParam)
{
	PTRACE2(eLevelError,"CBridgePartyAudioOut::OnAudioBridgePartyDisConnectIDLE : Already disconnected! : Name - ",m_pBridgePartyCntl->GetFullName());

	CSegment* pSeg = new CSegment;

	*pSeg  << (BYTE)statOK;

	m_pBridgePartyCntl->HandleEvent(pSeg, 0, AUDIO_OUT_DISCONNECTED);

	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnAudioBridgePartyDisConnectSETUP(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CBridgePartyAudioOut::OnAudioBridgePartyDisConnectSETUP : Name - ",m_pBridgePartyCntl->GetFullName());

	OnAudioBridgePartyDisConnect(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnAudioBridgePartyDisConnectCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CBridgePartyAudioOut::OnAudioBridgePartyDisConnectCONNECTED : Name - ",m_pBridgePartyCntl->GetFullName());

	OnAudioBridgePartyDisConnect(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnAudioBridgePartyDisConnectDISCONNECTING(CSegment* pParam)
{
	PTRACE2(eLevelError,"CBridgePartyAudioOut::OnAudioBridgePartyDisConnectDISCONNECTING : Ignored!!! : Name - ",m_pBridgePartyCntl->GetFullName());
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnAudioBridgePartyDisConnect(CSegment* pParam)
{
	if (::CPObject::IsValidPObjectPtr(m_pHardwareInterface)) {

		m_state = DISCONNECTING;

		m_lastReqId = ((CAudioHardwareInterface *) m_pHardwareInterface)->CloseEncoder();
		m_lastReq = AUDIO_CLOSE_ENCODER_REQ;
	}
	else {
		DBGPASSERT_AND_RETURN(1);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnAudioBridgePartyUpdateAudioAlgorithmSETUP(CSegment* pParam)
{
	CBridgePartyAudioUniDirection::OnAudioBridgePartyUpdateAudioAlgorithmSETUP(pParam);
}


/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnAudioBridgePartyUpdateAudioAlgorithmCONNECTED(CSegment* pParam)
{
	CBridgePartyAudioUniDirection::OnAudioBridgePartyUpdateAudioAlgorithmCONNECTED(pParam);
}

/////////////////////////////////////////////////////////////////////////////
// speakerIndication - update
void CBridgePartyAudioOut::OnAudioBridgePartyUpdateUseSpeakerSsrcForTxSETUP(CSegment* pParam)
{
	BYTE updatedUseSpeakerSsrcForTx;
	*pParam >> updatedUseSpeakerSsrcForTx;

	if(NULL == m_pWaitingForUpdateParams)
	{
		InitiateUpdateParams();
	}

	((CBridgePartyAudioOutParams*)m_pWaitingForUpdateParams)->SetUseSpeakerSsrcForTx( (BOOL)updatedUseSpeakerSsrcForTx );
}

/////////////////////////////////////////////////////////////////////////////
// speakerIndication - update
void CBridgePartyAudioOut::OnAudioBridgePartyUpdateUseSpeakerSsrcForTxCONNECTED(CSegment* pParam)
{
	BYTE updatedUseSpeakerSsrcForTx;
	*pParam >> updatedUseSpeakerSsrcForTx;

	// Ignore in case of the same algorithm
	if ( m_bUseSpeakerSsrcForTx == (BOOL)updatedUseSpeakerSsrcForTx )
		return;

	m_bUseSpeakerSsrcForTx = (BOOL)updatedUseSpeakerSsrcForTx;

	((CAudioHardwareInterface *) m_pHardwareInterface)->UpdateUseSpeakerSsrcForTx(m_bUseSpeakerSsrcForTx);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnAudioBridgePartyUpdateMuteSETUP(CSegment* pParam)
{
	CBridgePartyAudioUniDirection::OnAudioBridgePartyUpdateMuteSETUP(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnAudioBridgePartyUpdateMuteCONNECTED(CSegment* pParam)
{
	CBridgePartyAudioUniDirection::OnAudioBridgePartyUpdateMuteCONNECTED(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnAudioBridgePartyUpdateAudioVolumeSETUP(CSegment* pParam)
{
	CBridgePartyAudioUniDirection::OnAudioBridgePartyUpdateAudioVolumeSETUP(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnAudioBridgePartyUpdateAudioVolumeCONNECTED(CSegment* pParam)
{
	CBridgePartyAudioUniDirection::OnAudioBridgePartyUpdateAudioVolumeCONNECTED(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnAudioBridgePartyIncreaseAudioVolumeSETUP(CSegment* pParam)
{
	CBridgePartyAudioUniDirection::OnAudioBridgePartyIncreaseAudioVolumeSETUP(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnAudioBridgePartyIncreaseAudioVolumeCONNECTED(CSegment* pParam)
{
	CBridgePartyAudioUniDirection::OnAudioBridgePartyIncreaseAudioVolumeCONNECTED(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnAudioBridgePartyDecreaseAudioVolumeSETUP(CSegment* pParam)
{
	CBridgePartyAudioUniDirection::OnAudioBridgePartyDecreaseAudioVolumeSETUP(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnAudioBridgePartyDecreaseAudioVolumeCONNECTED(CSegment* pParam)
{
	CBridgePartyAudioUniDirection::OnAudioBridgePartyDecreaseAudioVolumeCONNECTED(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnAudioBridgePartyUpdateStandaloneCONNECTED(CSegment* pParam)
{
	CBridgePartyAudioUniDirection::OnAudioBridgePartyUpdateStandaloneCONNECTED(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnAudioBridgePlayMessageCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CBridgePartyAudioOut::OnAudioBridgePlayMessageCONNECTED : Name - ",m_pBridgePartyCntl->GetFullName());

	DWORD seqNumToken;
	*pParam >> seqNumToken;

	DWORD sequenceNum = ((CAudioHardwareInterface *) m_pHardwareInterface)->PlayMessage(pParam);

	SendSequenceNumIndicationToCAM(IVR_PLAY_MESSAGE_REQ, seqNumToken, sequenceNum);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnAudioBridgeStopPlayMessageCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CBridgePartyAudioOut::OnAudioBridgeStopPlayMessageCONNECTED : Name - ",m_pBridgePartyCntl->GetFullName());

	DWORD seqNumToken;
	*pParam >> seqNumToken;

	DWORD sequenceNum = ((CAudioHardwareInterface *) m_pHardwareInterface)->StopPlayMessage(pParam);

	SendSequenceNumIndicationToCAM(IVR_STOP_PLAY_MESSAGE_REQ, seqNumToken, sequenceNum);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnAudioBridgeStartIvrSeqCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CBridgePartyAudioOut::OnAudioBridgeStartIvrSeqCONNECTED : Name - ",m_pBridgePartyCntl->GetFullName());

	DWORD seqNumToken;
	*pParam >> seqNumToken;

	DWORD sequenceNum = ((CAudioHardwareInterface *) m_pHardwareInterface)->StartIVR(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnAudioBridgeStopIvrSeqCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CBridgePartyAudioOut::OnAudioBridgeStopIvrSeqCONNECTED : Name - ",m_pBridgePartyCntl->GetFullName());

	DWORD seqNumToken;
	*pParam >> seqNumToken;

	DWORD sequenceNum = ((CAudioHardwareInterface *) m_pHardwareInterface)->StopIVR(pParam);

	SendSequenceNumIndicationToCAM(IVR_STOP_IVR_REQ, seqNumToken, sequenceNum);
}
////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnAudioBridgeStopIvrSeqDISCONNECTING(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CBridgePartyAudioOut::OnAudioBridgeStopIvrSeqDISCONNECTING - IGNORED : Name - ",m_pBridgePartyCntl->GetFullName());
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnAudioBridgePlayMusicCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CBridgePartyAudioOut::OnAudioBridgePlayMusicCONNECTED : Name - ",m_pBridgePartyCntl->GetFullName());

	DWORD seqNumToken;
	*pParam >> seqNumToken;

	DWORD sequenceNum = ((CAudioHardwareInterface *) m_pHardwareInterface)->PlayMusic(pParam);

	SendSequenceNumIndicationToCAM(IVR_PLAY_MUSIC_REQ, seqNumToken, sequenceNum);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnAudioBridgeStopPlayMusicCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CBridgePartyAudioOut::OnAudioBridgeStopPlayMusicCONNECTED : Name - ",m_pBridgePartyCntl->GetFullName());

	DWORD seqNumToken;
	*pParam >> seqNumToken;

	DWORD sequenceNum = ((CAudioHardwareInterface *) m_pHardwareInterface)->StopPlayMusic(pParam);

	SendSequenceNumIndicationToCAM(IVR_STOP_PLAY_MUSIC_REQ, seqNumToken, sequenceNum);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnAudioBridgePlayToneCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CBridgePartyAudioOut::OnAudioBridgePlayToneCONNECTED : Name - ",m_pBridgePartyCntl->GetFullName());

	DWORD seqNumToken;
	*pParam >> seqNumToken;

	DWORD sequenceNum = ((CAudioHardwareInterface *) m_pHardwareInterface)->PlayTone(pParam);

	SendSequenceNumIndicationToCAM(AUDIO_PLAY_TONE_REQ, seqNumToken, sequenceNum);
}


void CBridgePartyAudioOut::OnAudioBridgePlayToneDISCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelError,"CBridgePartyAudioOut::OnAudioBridgePlayToneDISCONNECTED : Ignored!!! : Name - ",m_pBridgePartyCntl->GetFullName());
}
/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnAudioBridgeRecordRollCallCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CBridgePartyAudioOut::OnAudioBridgeRecordRollCallCONNECTED : Name - ",m_pBridgePartyCntl->GetFullName());

	if(m_pBridgePartyCntl->IsUniDirectionConnection())
	{
		PTRACE2(eLevelInfoNormal,"CBridgePartyAudioOut::OnAudioBridgeRecordRollCallCONNECTED Illegal in UniDirection: Name - ",m_pBridgePartyCntl->GetFullName());
	}

	DWORD seqNumToken;
	*pParam >> seqNumToken;

	DWORD sequenceNum = ((CAudioHardwareInterface *) m_pHardwareInterface)->RecordRollCall(pParam);

	SendSequenceNumIndicationToCAM(IVR_RECORD_ROLL_CALL_REQ, seqNumToken, sequenceNum);
}
/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnAudioBridgeStopRecordRollCall(CSegment* pParam)
{
	TRACEINTO << " ROLL_CALL_STOP_RECORDING (05), PartyRsrcID = " << m_pBridgePartyCntl->GetPartyRsrcID() << ", party name = " << m_pBridgePartyCntl->GetFullName();

	DWORD seqNumToken;
	*pParam >> seqNumToken;

	DWORD sequenceNum = ((CAudioHardwareInterface *) m_pHardwareInterface)->StopRecordRollCall(pParam);

	SendSequenceNumIndicationToCAM(IVR_STOP_RECORD_ROLL_CALL_REQ, seqNumToken, sequenceNum);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnMplRecordRollCallIndSETUP(CSegment* pParam)
{
	PTRACE2(eLevelError,"CBridgePartyAudioOut::OnMplRecordRollCallIndSETUP : Ignored!!! : Name - ",m_pBridgePartyCntl->GetFullName());
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnMplRecordRollCallIndDISCONNECTING(CSegment* pParam)
{
	PTRACE2(eLevelError,"CBridgePartyAudioOut::OnMplRecordRollCallIndDISCONNECTING : Ignored!!! : Name - ",m_pBridgePartyCntl->GetFullName());
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnMplRecordRollCallIndCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelError,"CBridgePartyAudioOut::OnMplRecordRollCallIndCONNECTED : Name - ",m_pBridgePartyCntl->GetFullName());

	STATUS  status;
	DWORD recordingLength;

	*pParam >> status >> recordingLength;

	CSegment* pSeg = new CSegment;
	*pSeg << status << recordingLength << *pParam;

	((CAudioBridgePartyCntl*)m_pBridgePartyCntl)->IvrNotification(IVR_RECORD_ROLL_CALL_IND, pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnMplAckSETUP(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CBridgePartyAudioOut::OnMplAckSETUP : Name - ",m_pBridgePartyCntl->GetFullName());

	OPCODE	AckOpcode;
	DWORD	ack_seq_num;
	STATUS	status;

	*pParam >> AckOpcode >> ack_seq_num >> status;

	switch (AckOpcode)
	{
		case AUDIO_OPEN_ENCODER_REQ:
		{
			BYTE	responseStatus = statOK;

			if (status != STATUS_OK)
			{
				PTRACE2(eLevelError, "CBridgePartyAudioOut::OnMplAckSETUP :  Bad Status!!! : Name - ",m_pBridgePartyCntl->GetFullName());

				//Add assert to EMA in case of NACK
				//AddFaultAlarm("NACK on open Audio Encoder",m_pHardwareInterface->GetPartyRsrcId(),status);

				responseStatus = statAudioInOutResourceProblem; // statAudioOutResourceProblem -> Inorder to initiate Kill port on AudioDecoder AudioEncoder
			}
			else
			{
				m_state = CONNECTED;
				
				// check if update required
				if (m_pWaitingForUpdateParams)
				{
					SaveAndSendUpdateParams();
					POBJDELETE(m_pWaitingForUpdateParams)
				}
			}

			CSegment* pSeg = new CSegment;
			
			*pSeg  << (BYTE)responseStatus;
			
			if (statAudioInOutResourceProblem == responseStatus)
				*pSeg  << (BYTE) eMipOut << (BYTE) eMipStatusFail << (BYTE) eMipOpen;

			// Inform BridgePartyCntl
			m_pBridgePartyCntl->HandleEvent(pSeg, 0, AUDIO_OUT_CONNECTED);

			POBJDELETE(pSeg);

			break;
		}
		default:
		{
			CProcessBase * process = CProcessBase::GetProcess();
			std::string str = process->GetOpcodeAsString(AckOpcode);
			TRACEINTO << "CBridgePartyAudioOut::OnMplAckSETUP - ACK_IND Ignored! - Ack Opcode: "<< str.c_str() << " - Name: "<< m_pBridgePartyCntl->GetFullName();
		}
	}// end switch
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnMplAckDISCONNECTING(CSegment * pParam)
{
	PTRACE2(eLevelInfoNormal,"CBridgePartyAudioOut::OnMplAckDISCONNECTING : Name - ",m_pBridgePartyCntl->GetFullName());

	OPCODE	AckOpcode;
	DWORD	ack_seq_num;
	STATUS	status;

	*pParam >> AckOpcode >> ack_seq_num >> status;

	switch(AckOpcode){
	case	AUDIO_CLOSE_ENCODER_REQ:
		{
			BYTE	responseStatus = statOK;

			if ( status != STATUS_OK )
			{
				PTRACE2(eLevelError, "CBridgePartyAudioOut::OnMplAckDISCONNECTING :  Bad Status!!! : Name - ",m_pBridgePartyCntl->GetFullName());
				//Add assert to EMA in case of NACK
				//AddFaultAlarm("NACK on Close Audio Encoder",m_pHardwareInterface->GetPartyRsrcId(),status);

				responseStatus = statAudioInOutResourceProblem; // statAudioOutResourceProblem -> Inorder to initiate Kill port on AudioDecoder AudioEncoder
			}

			SetClosePortAckStatus(responseStatus);
			m_state = IDLE;

			CSegment* pSeg = new CSegment;
			*pSeg  << (BYTE)responseStatus;
			if(responseStatus == statAudioInOutResourceProblem)
				*pSeg  << (BYTE) eMipOut << (BYTE) eMipStatusFail << (BYTE) eMipClose;

			// Inform BridgePartyCntl
			m_pBridgePartyCntl->HandleEvent(pSeg, 0, AUDIO_OUT_DISCONNECTED);

			POBJDELETE(pSeg);

			break;
		}
	default:
		{
			CProcessBase * process = CProcessBase::GetProcess();
			std::string str = process->GetOpcodeAsString(AckOpcode);
			TRACEINTO << "CBridgePartyAudioOut::OnMplAckDISCONNECTING - ACK_IND Ignored! - Ack Opcode: "<< str.c_str() << " - Name: "<< m_pBridgePartyCntl->GetFullName();
		}
	}// end switch
}

/////////////////////////////////////////////////////////////////////////////
void  CBridgePartyAudioOut::OnMplAckCONNECTED(CSegment* pParam)
{
	OPCODE	AckOpcode;
	DWORD  ack_seq_num;
	STATUS  status;

	*pParam >> AckOpcode >> ack_seq_num >> status;

	TRACEINTO << AckOpcode;

	switch(AckOpcode){
	case    IVR_RECORD_ROLL_CALL_REQ:
	{
		TRACEINTO << AckOpcode;
		CSegment* pSeg = new CSegment;
		*pSeg << AckOpcode << ack_seq_num << status << *pParam;

		((CAudioBridgePartyCntl*)m_pBridgePartyCntl)->IvrNotification(ACK_IND, pSeg);
		POBJDELETE(pSeg);

		CPartyApi* partyApi = m_pBridgePartyCntl->GetPartyApi();
		partyApi->ReceivedAckOnIvrPlayRecordReq();

		break;
	}
	case    IVR_PLAY_MESSAGE_REQ:
	case    IVR_STOP_PLAY_MESSAGE_REQ:
	case	IVR_START_IVR_REQ:
	case    IVR_STOP_IVR_REQ:
	case    IVR_PLAY_MUSIC_REQ:
	case	IVR_STOP_PLAY_MUSIC_REQ:
	case	AUDIO_PLAY_TONE_REQ:
	//case	IVR_STOP_PLAY_TONE_REQ:
	case	IVR_UPDATE_STANDALONE_REQ:
	case	IVR_STOP_RECORD_ROLL_CALL_REQ:
		{
			CSegment* pSeg = new CSegment;
			*pSeg << AckOpcode << ack_seq_num << status << *pParam;

			((CAudioBridgePartyCntl*)m_pBridgePartyCntl)->IvrNotification(ACK_IND, pSeg);
			POBJDELETE(pSeg);
			break;
		}
	default:
		{
			CProcessBase * process = CProcessBase::GetProcess();
			std::string str = process->GetOpcodeAsString(AckOpcode);
			TRACEINTO << "CBridgePartyAudioOut::OnMplAckCONNECTED - ACK_IND Ignored! - Ack Opcode: "<< str.c_str() << " - Name: "<< m_pBridgePartyCntl->GetFullName();
		}
	}// end switch
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::UpdatePartyOutParams(CUpdatePartyAudioInitParams* pUpdatePartyAudioInitParams)
{
	UpdatePartyParams((CBridgePartyAudioOutParams*)pUpdatePartyAudioInitParams->GetMediaOutParams());

	m_numOfSsrcIds = ((CBridgePartyAudioOutParams*)pUpdatePartyAudioInitParams)->GetSsrcNum();
	for( WORD i=0; i<m_numOfSsrcIds; i++ )
	    m_ssrc[i] = ((CBridgePartyAudioOutParams*)pUpdatePartyAudioInitParams)->GetSsrc(i);
	m_ivrSsrc = ((CBridgePartyAudioOutParams*)pUpdatePartyAudioInitParams)->GetIvrSsrc();

}
/////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::UpdateAudioRelayParams(DWORD numOfSsrcIds, DWORD* ssrc_array, DWORD ivrSsrc)
{
	TRtpUpdateRelayReq stRelayParamsOut;

	std::ostringstream ostr;
	ostr << "CBridgePartyAudioOut::UpdateAudioRelayParams : numOfSsrcIds = " << numOfSsrcIds;
	m_numOfSsrcIds = ((numOfSsrcIds > SSRC_LIST_SIZE) ? SSRC_LIST_SIZE : numOfSsrcIds);
	stRelayParamsOut.nSSRC.numOfSSRC = numOfSsrcIds;
	stRelayParamsOut.unChannelType = kIpAudioChnlType;
	stRelayParamsOut.unChannelDirection = cmCapTransmit;

	for( WORD i=0; i<m_numOfSsrcIds; i++ )
	{
	    m_ssrc[i] = ssrc_array[i];
	    stRelayParamsOut.nSSRC.ssrcList[i] = ssrc_array[i];
	    ostr << "\n m_ssrc[" << i << "]=" << m_ssrc[i];
	}
	ostr << "\n ivrSsrc = " <<ivrSsrc;
	m_ivrSsrc = ivrSsrc;
	stRelayParamsOut.unIvrSsrc = m_ivrSsrc;
	stRelayParamsOut.unIvrCsrc = IVR_CSRC;


	PTRACE(eLevelInfoNormal,ostr.str().c_str());

	//In case of RMX AVC we don't need to update the audio encoder on the SSRC
	if(!IsSoftMcu() && !(((CAudioBridgePartyCntl*)m_pBridgePartyCntl)->GetIsVideoRelay()))
	{
		TRACEINTO << "No need to update encoder on relay parameters on RMX for non relay party ";

	}
	else
	{
		((CAudioHardwareInterface *) m_pHardwareInterface)->UpdateAudioRelayParamsOut( &stRelayParamsOut );
	}
}
////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::GetPortsOpened(std::map<eLogicalResourceTypes,bool>& isOpenedRsrcMap)
{
    CRsrcParams* pRsrcParams = GetRsrcParams();
    if (pRsrcParams)
    {
    	eLogicalResourceTypes lrt = pRsrcParams->GetLogicalRsrcType();
    	if (IsConnected())
    		isOpenedRsrcMap[lrt] = true;
    }
}

/////////////////////////////////////////////////////////////////////////////
// speakerIndication - update
void CBridgePartyAudioOut::UpdateUseSpeakerSsrcForTx(BOOL updatedUseSpeakerSsrcForTx)
{
	CSegment* pSeg = new CSegment;

	*pSeg  << (BYTE)updatedUseSpeakerSsrcForTx;

	DispatchEvent(UPDATE_USE_SPEAKER_SSRC_FOR_TX, pSeg);
	POBJDELETE(pSeg);
}
///////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::UpdateAudioOutOnSeenImageSSRC(DWORD audioUplinkSSRCOfSeenImage)
{
	CSegment* pSeg = new CSegment;
	*pSeg  << audioUplinkSSRCOfSeenImage;
	DispatchEvent(UPDATE_AUDIO_ON_SEEN_IMAGE, pSeg);
	POBJDELETE(pSeg);
}
///////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnAudioBridgePartyUpdateSeenImageSsrcSETUP(CSegment* pParam)
{
	DWORD ssrc;
	*pParam >> ssrc;

	DWORD partyRsrcId = m_pBridgePartyCntl->GetPartyRsrcID();
	TRACEINTO<<"PartyId:" << partyRsrcId <<", ssrc:" << ssrc ;

	if(NULL == m_pWaitingForUpdateParams)
	{
		InitiateUpdateParams();
	}

	((CBridgePartyAudioOutParams*)m_pWaitingForUpdateParams)->SetSeenImageSsrc(ssrc);
}

///////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnAudioBridgePartyUpdateSeenImageSsrcCONNECTED(CSegment* pParam)
{
	DWORD ssrc;
	*pParam >> ssrc;
	DWORD partyRsrcId = m_pBridgePartyCntl->GetPartyRsrcID();

	if(m_seenImageSSRC!=ssrc)
	{
		m_seenImageSSRC = ssrc;
		TRACEINTO<<"PartyId:" << partyRsrcId <<", m_seenImageSSRC:" << m_seenImageSSRC ;
		SendAudioEncoderUpdateSeenImageSsrcToHardware(ssrc);
	}
	else
		TRACEINTO<<"PartyId:" << partyRsrcId << "no change no need to update";
}

///////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::OnAudioBridgePartyUpdateSeenImageSsrcDISCONNECTING(CSegment* pParam)
{
	DWORD partyRsrcId = m_pBridgePartyCntl->GetPartyRsrcID();
	TRACEINTO<<"PartyId:" << partyRsrcId << ", ignore in disconnecting";
}
///////////////////////////////////////////////////////////////////////////////
void CBridgePartyAudioOut::SendAudioEncoderUpdateSeenImageSsrcToHardware(DWORD ssrc)
{
	((CAudioHardwareInterface *) m_pHardwareInterface)->SendAudioEncoderUpdateImageSeenSsrc(ssrc);
}
