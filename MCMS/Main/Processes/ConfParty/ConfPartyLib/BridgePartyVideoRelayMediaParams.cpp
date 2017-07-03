#include "BridgePartyVideoRelayMediaParams.h"
#include "TraceStream.h"
#include "ConfPartySharedDefines.h"

// ------------------------------------------------------------
CBridgePartyVideoRelayMediaParams::CBridgePartyVideoRelayMediaParams ()
{
	m_pStreamsList.clear();
	m_bIsSupportSvcAvcTranslate = true;		// xxxAmir: need to change to false upon Party implementation.
	m_pipeIdForIvrSlide = 0;
	m_bIsCascadeLink = false;
	m_channelHandle = INVALID_CHANNEL_HANDLE;
	m_scpRequestSequenceNumber= 0;
	m_bIsReady = false;
}

// ------------------------------------------------------------
CBridgePartyVideoRelayMediaParams::CBridgePartyVideoRelayMediaParams (const CBridgePartyVideoRelayMediaParams& rOther)
        :CBridgePartyMediaParams(rOther)
{
	// ===== 1. simple members
	m_channelHandle				= rOther.m_channelHandle;
	m_bIsReady					= rOther.m_bIsReady;
	m_scpRequestSequenceNumber	= rOther.m_scpRequestSequenceNumber;
	m_mute_mask					= rOther.m_mute_mask;
	m_bIsSupportSvcAvcTranslate = rOther.m_bIsSupportSvcAvcTranslate;
	m_pipeIdForIvrSlide         = rOther.m_pipeIdForIvrSlide;
	m_bIsCascadeLink            = rOther.m_bIsCascadeLink;

	// ===== 2. pointers' list
	m_pStreamsList.clear();

	std::list<CRelayMediaStream *>::const_iterator itr = rOther.m_pStreamsList.begin();
	while ( itr != rOther.m_pStreamsList.end() )
	{
		CRelayMediaStream* pNewItem = NULL;
		if (*itr)
		{
				pNewItem = (*itr)->NewCopy();

		}
		m_pStreamsList.push_back(pNewItem);

		itr++;
	}

}

// ------------------------------------------------------------
CBridgePartyVideoRelayMediaParams::~CBridgePartyVideoRelayMediaParams ()
{
	ClearStreamsList();
}

// ------------------------------------------------------------
CBridgePartyVideoRelayMediaParams& CBridgePartyVideoRelayMediaParams::operator = (const CBridgePartyVideoRelayMediaParams& rOther)
{
	if ( &rOther == this )
		return *this;

	// ===== 1. Base
	CBridgePartyMediaParams::operator =(rOther);

	// ===== 2. simple members
	m_channelHandle				= rOther.m_channelHandle;
	m_bIsReady					= rOther.m_bIsReady;
	m_scpRequestSequenceNumber	= rOther.m_scpRequestSequenceNumber;
	m_mute_mask					= rOther.m_mute_mask;
	m_bIsSupportSvcAvcTranslate = rOther.m_bIsSupportSvcAvcTranslate;
	m_pipeIdForIvrSlide         = rOther.m_pipeIdForIvrSlide;
	m_bIsCascadeLink            = rOther.m_bIsCascadeLink;


	// ===== 3. pointers' list
	ClearStreamsList();

	std::list<CRelayMediaStream*>::const_iterator itr = rOther.m_pStreamsList.begin();
	while ( itr != rOther.m_pStreamsList.end() )
	{
		CRelayMediaStream* pNewItem = NULL;
		if (*itr)
		{
			//pNewItem = new CRelayMediaStream( *((CRelayMediaStream*)(*itr)) );
			pNewItem = (CRelayMediaStream*)(*itr)->NewCopy();
		}
		m_pStreamsList.push_back(pNewItem);

		itr++;
	}


	return *this;
}

// ------------------------------------------------------------
void CBridgePartyVideoRelayMediaParams::ClearStreamsList()
{
	std::list<CRelayMediaStream*>::const_iterator itr = m_pStreamsList.begin();
	while (itr != m_pStreamsList.end())
	{
		if (*itr)
		{
			delete *itr;
		}
		itr++;
	}

	m_pStreamsList.clear();
}

// ------------------------------------------------------------

BOOL CBridgePartyVideoRelayMediaParams::IsValidParams()  const
{
//	if(m_channelHandle==INVALID)
//	{
//	    TRACEINTO << " CBridgePartyVideoRelayMediaParams::IsValidParams INVALID m_channelHandle" ;
//	  return FALSE;
//	}
//	std::list<CRelayMediaStream*>::const_iterator itr = m_pStreamsList.begin();
//	while (itr != m_pStreamsList.end())
//	{
//		if (!(*itr)->IsValidParams())
//		{
//			  TRACEINTO << " CBridgePartyVideoRelayMediaParams::IsValidParams INVALID CRelayMediaStream SSRC:",(*itr)->GetSsrc() ;
//			  return FALSE;
//		}
//	}
  return TRUE;
}
// ---------------------------------------------------------------

