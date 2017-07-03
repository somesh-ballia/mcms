#include "MsSsrc.h"
#include "TraceStream.h"

////////////////////////////////////////////////////////////////////////////
//                        CMsSsrcAllocator
////////////////////////////////////////////////////////////////////////////
bool CMsSsrcAllocator::Alloc(PartyRsrcID partyId, DWORD& first, DWORD& last)
{
	DWORD rangeId = m_lookupId.Alloc();
	if (rangeId == 0)
		return false;
	m_rangeMap.insert(std::make_pair(partyId, rangeId));
	first = SSRC_MIN+(rangeId-1)*SSRC_PARTY_RANGE;
	last  = first+SSRC_PARTY_RANGE-1;
	TRACEINTO << "PartyId:" << partyId << ", FirstMsSsrc:" << first << ", LastMsSsrc:" <<  last;
	return true;
}

//--------------------------------------------------------------------------
void CMsSsrcAllocator::Clear(PartyRsrcID partyId)
{
	std::map<PartyRsrcID, DWORD>::iterator _itr = m_rangeMap.find(partyId);
	TRACECOND_AND_RETURN(_itr == m_rangeMap.end(), "PartyId:" << partyId << " - MsSsrc does not allocated for this party");

	m_lookupId.Clear(_itr->second);
	m_rangeMap.erase(partyId);
}
