#ifndef _BRIDGEPARTYLIST_H__
#define _BRIDGEPARTYLIST_H__

#include "PObject.h"
#include "BridgePartyCntl.h"
#include "ConfPartySharedDefines.h"

typedef std::map<PartyRsrcID, CBridgePartyCntl* > BRIDGE_PARTY_LIST;

////////////////////////////////////////////////////////////////////////////
//                        CBridgePartyList
////////////////////////////////////////////////////////////////////////////
class CBridgePartyList : public CPObject, public BRIDGE_PARTY_LIST
{
	CLASS_TYPE_1(CBridgePartyList, CPObject)

public:
	                  CBridgePartyList() {}
	                  CBridgePartyList(const CBridgePartyList& rBridgePartyList);
	virtual          ~CBridgePartyList() {}

	const char*       NameOf() const { return "CBridgePartyList"; }

	CBridgePartyList& operator=(const CBridgePartyList& rOther);

	CBridgePartyCntl* Find(PartyRsrcID partyId) const;
	CBridgePartyCntl* Find(const char* name) const;
	CBridgePartyCntl* Find(const CParty* pParty) const;

	EStat             Insert(CBridgePartyCntl* pBridgePartyCntl);
	CBridgePartyCntl* Remove(PartyRsrcID partyId);

	void              ClearAndDestroy(void);

	CBridgePartyList& CopyContentBridgeList(const CBridgePartyList& rOtherBridgePartyList, CBridge* pBridge);
};

inline std::ostream& operator<<(std::ostream& os, const CBridgePartyList& in)
{
	os << "Size:" << in.size() << " {";
	CBridgePartyList::const_iterator _itr, _end = in.end();
	for (_itr = in.begin(); _itr != _end; ++_itr)
		os << _itr->first << ",";
	os << "}";
	return os;
}


#endif /* _BRIDGEPARTYLIST_H__ */
