#include "BridgePartyDisconnectParams.h"
#include "Segment.h"

////////////////////////////////////////////////////////////////////////////
//                        CBridgePartyDisconnectParams
////////////////////////////////////////////////////////////////////////////
CBridgePartyDisconnectParams::CBridgePartyDisconnectParams()
{
	m_partyId = 0;
	m_eMediaDirection = eNoDirection;
}

//--------------------------------------------------------------------------
CBridgePartyDisconnectParams::CBridgePartyDisconnectParams(PartyRsrcID partyId, EMediaDirection eMediaDirection)
{
	m_partyId = partyId;
	m_eMediaDirection = eMediaDirection;
}

//--------------------------------------------------------------------------
CBridgePartyDisconnectParams::CBridgePartyDisconnectParams(const CBridgePartyDisconnectParams& rhs) : CPObject(rhs)
{
	m_partyId = rhs.m_partyId;
	m_eMediaDirection = rhs.m_eMediaDirection;
}

//--------------------------------------------------------------------------
CBridgePartyDisconnectParams::~CBridgePartyDisconnectParams()
{
}

//--------------------------------------------------------------------------
CBridgePartyDisconnectParams& CBridgePartyDisconnectParams::operator=(const CBridgePartyDisconnectParams& rhs)
{
	// Operator= is not available for this class because all members are const
	return *this;
}

//--------------------------------------------------------------------------
void CBridgePartyDisconnectParams::Serialize(WORD format, CSegment& seg) const
{
	if (format == NATIVE)
	{
		seg << m_partyId;
		seg << (WORD)m_eMediaDirection;
	}
}

//--------------------------------------------------------------------------
void CBridgePartyDisconnectParams::DeSerialize(WORD format, CSegment& seg)
{
	if (format == NATIVE)
	{
		seg >> m_partyId;
		seg >> (WORD&)m_eMediaDirection;
	}
}
