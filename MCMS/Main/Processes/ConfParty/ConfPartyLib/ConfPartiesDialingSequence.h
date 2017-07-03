#ifndef CONFPARTIESDIALINGSEQUENCE_H_
#define CONFPARTIESDIALINGSEQUENCE_H_

#include <list>
#include <map>
#include "DataTypes.h"
#include "PObject.h"
#include "ObjString.h"
#include "ConfPartySharedDefines.h"

class CRsrvParty;
class CSegment;

// PARTY_SEQUENCE_LIST contains all different parties (H323,SIP,ISDN)
// that were generated from a specific string
typedef std::list <CRsrvParty*>	PARTY_SEQUENCE_LIST;

// SEQUENCE_DIAL_MAP_PER_CONF contains all different party vectors
// for one conf , where the key is the party Id
typedef std::map <PartyMonitorID, PARTY_SEQUENCE_LIST*> SEQUENCE_DIAL_MAP_PER_CONF;

class CConfPartiesDialingSequence: public CPObject, private CNonCopyable
{
	CLASS_TYPE_1(CConfPartiesDialingSequence, CPObject)

public:
	                            CConfPartiesDialingSequence();
	virtual                    ~CConfPartiesDialingSequence();
	virtual const char*         NameOf() const;

	void                        Serialize(CSegment& seg);
	virtual void                Serialize(std::ostringstream& ostr);
	virtual void                DeSerialize(CSegment& seg);
	virtual void                DeSerialize(std::istringstream& istr);

	void                        DumpToTrace(CObjString& cstr);
	void                        DumpPartyDetailesToTrace(CRsrvParty* pRsrvParty, CLargeString& cstr);
	void                        AddPartyListToMap(PartyMonitorID partyId, PARTY_SEQUENCE_LIST* seqVector);

	CRsrvParty*                 GetNextParty(PartyMonitorID partyId);
	PARTY_SEQUENCE_LIST*        GetListForParty(PartyMonitorID partyId);
	void                        RemoveMapEntry(PartyMonitorID partyId);
	SEQUENCE_DIAL_MAP_PER_CONF* GetDialMap() { return m_pSequenceDialMap; }
	bool                        IsMemberInSequence(PartyMonitorID partyId);
	void                        SetVoice(const BYTE btVoice, BOOL isPstnAllowed = TRUE);

private:
	SEQUENCE_DIAL_MAP_PER_CONF* m_pSequenceDialMap;

};

#endif /*CONFPARTIESDIALINGSEQUENCE_H_*/
