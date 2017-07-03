#ifndef _PARTYLIST_H__
#define _PARTYLIST_H__

#include "PartyConnection.h"
#include "PObject.h"
#include "Trace.h"
#include <vector>

typedef std::map<std::string, class CPartyConnection*> PARTYLIST;

class CPartyList : public CPObject
{
	CLASS_TYPE_1(CPartyList, CPObject)

public:
	                    CPartyList();
	virtual            ~CPartyList();
	virtual const char* NameOf() const { return "CPartyList"; }

	CPartyConnection*   find(const char* name);
	CPartyConnection*   find(const CTaskApp* pTask);
	CPartyConnection*   find(const PartyRsrcID partyId);

	CPartyConnection*   remove(const CTaskApp* pTask);
	int                 insert(CPartyConnection* pPartyConnection);
	int                 entries();

	PARTYLIST           m_PartyList;

private:
	void                Dump(const char* prefixString, CPartyConnection* pPartyConnection);
};

#endif /* _PARTYLIST_H__ */
