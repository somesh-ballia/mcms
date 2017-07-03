#ifndef _MS_SSRC_H_
#define _MS_SSRC_H_

#include <map>
#include "PObject.h"
#include "LookupId.h"
#include "ConfPartySharedDefines.h"

#define SSRC_MIN         2000
#define SSRC_PARTY_RANGE 1000
#define SSRC_COUNT_RANGE 1000

////////////////////////////////////////////////////////////////////////////
//                        CMsSsrcAllocator
////////////////////////////////////////////////////////////////////////////
class CMsSsrcAllocator : public CPObject
{
	CLASS_TYPE_1(CMsSsrcAllocator, CPObject)

public:
	virtual const char* NameOf() const { return GetCompileType(); }

	bool Alloc(PartyRsrcID partyId, DWORD& first, DWORD& last);
	void Clear(PartyRsrcID partyId);

private:
	CLookupId<SSRC_COUNT_RANGE>  m_lookupId;
	std::map<PartyRsrcID, DWORD> m_rangeMap;
};

#endif // _MS_SSRC_H_
