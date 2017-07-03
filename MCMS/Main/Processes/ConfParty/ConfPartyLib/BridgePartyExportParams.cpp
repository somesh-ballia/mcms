#include "BridgePartyExportParams.h"
#include "Segment.h"

////////////////////////////////////////////////////////////////////////////
//                        CBridgePartyExportParams
////////////////////////////////////////////////////////////////////////////
CBridgePartyExportParams::CBridgePartyExportParams ()
{
	m_partyId = 0;
}

//--------------------------------------------------------------------------
CBridgePartyExportParams::CBridgePartyExportParams(PartyRsrcID partyId)
{
	m_partyId = partyId;
}

//--------------------------------------------------------------------------
CBridgePartyExportParams::CBridgePartyExportParams (const CBridgePartyExportParams& rhs) : CPObject(rhs)
{
	m_partyId = rhs.m_partyId;
}

//--------------------------------------------------------------------------
CBridgePartyExportParams::~CBridgePartyExportParams()
{
}

//--------------------------------------------------------------------------
CBridgePartyExportParams& CBridgePartyExportParams::operator=(const CBridgePartyExportParams& rhs)
{
	// Operator= is not available for this class because all members are const
	return *this;
}

//--------------------------------------------------------------------------
void CBridgePartyExportParams::Serialize(WORD format, CSegment& seg) const
{
	if (format == NATIVE)
	{
		seg << m_partyId;
	}
}
//--------------------------------------------------------------------------
void CBridgePartyExportParams::DeSerialize(WORD format, CSegment& seg)
{
	if (format == NATIVE)
	{
		seg >> m_partyId;
	}
}
