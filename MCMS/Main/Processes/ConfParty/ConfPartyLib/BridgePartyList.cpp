#include "BridgePartyList.h"
#include "ContentBridge.h"

template <typename _T> void delete_second(_T& T) { delete T.second; }

////////////////////////////////////////////////////////////////////////////
//                        CBridgePartyList
////////////////////////////////////////////////////////////////////////////
CBridgePartyList::CBridgePartyList(const CBridgePartyList& rBridgePartyList) : CPObject(), BRIDGE_PARTY_LIST(rBridgePartyList)
{
}

////////////////////////////////////////////////////////////////////////////
CBridgePartyList& CBridgePartyList::operator=(const CBridgePartyList& rOtherBridgePartyList)
{
	if (&rOtherBridgePartyList == this)
		return *this;

	BRIDGE_PARTY_LIST::operator=(rOtherBridgePartyList);

	return *this;
}

////////////////////////////////////////////////////////////////////////////
CBridgePartyList& CBridgePartyList::CopyContentBridgeList(const CBridgePartyList& rOtherBridgePartyList, CBridge* pBridge)
{
	CBridgePartyList::const_iterator _end = rOtherBridgePartyList.end();
	for (CBridgePartyList::const_iterator _itr = rOtherBridgePartyList.begin(); _itr != _end; ++_itr)
	{
		CContentBridgePartyCntl* pBridgePartyCntl = new CContentBridgePartyCntl();
		*pBridgePartyCntl = *(CContentBridgePartyCntl*)_itr->second;
		pBridgePartyCntl->SetMyNewPointerToBridge(pBridge);
		Insert(pBridgePartyCntl);
	}
	return *this;
}

////////////////////////////////////////////////////////////////////////////
CBridgePartyCntl* CBridgePartyList::Find(PartyRsrcID partyId) const
{
	CBridgePartyList::const_iterator _itr = find(partyId);
	if (_itr != end())
		return (CBridgePartyCntl*)_itr->second;
	//PASSERTSTREAM(1, "PartyId:" << partyId);
	return NULL;
}

////////////////////////////////////////////////////////////////////////////
CBridgePartyCntl* CBridgePartyList::Find(const char* name) const
{
	PASSERT_AND_RETURN_VALUE(!name, NULL);
	TRACEINTO << "PartyName:" << name;

	CBridgePartyList::const_iterator _itr, _end = end();
	for (_itr = begin(); _itr != _end; ++_itr)
		if (!strcmp(_itr->second->GetName(), name))
			return _itr->second;

	return NULL;
}

////////////////////////////////////////////////////////////////////////////
CBridgePartyCntl* CBridgePartyList::Find(const CParty* pParty) const
{
	PASSERT_AND_RETURN_VALUE(!pParty, NULL);

	CBridgePartyList::const_iterator _itr, _end = end();
	for (_itr = begin(); _itr != _end; ++_itr)
		if (_itr->second->GetPartyTaskApp() == (CTaskApp*)pParty)
			return _itr->second;
	return NULL;
}

////////////////////////////////////////////////////////////////////////////
CBridgePartyCntl* CBridgePartyList::Remove(PartyRsrcID partyId)
{
	CBridgePartyList::iterator _itr = find(partyId);
	if (_itr == end())
	{
		CSmallString str;
		str << "PartyId:" << partyId << " - Failed, invalid party id";
		TRACEINTO << str.GetString();
		return NULL;
	}

	CBridgePartyCntl* pBridgePartyCntl = _itr->second;
	erase(_itr);
	return pBridgePartyCntl;
}

////////////////////////////////////////////////////////////////////////////
EStat CBridgePartyList::Insert(CBridgePartyCntl* pBridgePartyCntl)
{
	PASSERT_AND_RETURN_VALUE(!pBridgePartyCntl, statInconsistent);

	PartyRsrcID partyId = pBridgePartyCntl->GetPartyRsrcID();
	PASSERT_AND_RETURN_VALUE(partyId <= 0 || partyId > MAX_RSRC_PARTY_ID, statInconsistent);

	std::pair<CBridgePartyList::iterator, bool> rc = insert(std::make_pair(partyId, pBridgePartyCntl));

	PASSERTSTREAM(rc.second == false, "PartyId:" << partyId << " - Failed, invalid party id");

	return rc.second == false ? statIllegal : statOK;
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyList::ClearAndDestroy(void)
{
	std::for_each(begin(), end(), &delete_second<CBridgePartyList::value_type>);

	clear();
}
