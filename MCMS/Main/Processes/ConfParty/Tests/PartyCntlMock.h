
#ifndef _PARTYCNTLMOCK_H__
#define _PARTYCNTLMOCK_H__

#include "PartyCntl.h"

class CPartyCntlMock: public CPartyCntl
{
public:
	virtual BYTE IsRemoteAndLocalCapSetHasContent(eToPrint toPrint = eToPrintNo) const { return NO; }
	virtual BYTE IsRemoteAndLocalHasHDContent1080() const { return NO; }
	virtual BYTE IsRemoteAndLocalHasHDContent720() const { return NO; }
	virtual BYTE IsLegacyContentParty() { return !IsRemoteAndLocalCapSetHasContent(); }
	virtual BYTE IsRemoteAndLocalHasHighProfileContent() const { return NO; }
	virtual DWORD GetPossibleContentRate() const { return 0; }

	void CreateMock(PartyRsrcID partyId, CTaskApp* pParty, char* name)
	{
		m_pPartyAllocatedRsrc = new CPartyRsrcDesc;
		m_pPartyAllocatedRsrc->SetPartyRsrcId(partyId);
		SetParty(pParty);
		SetPartyName(name);
	}
};


#endif //_PARTYCNTLMOCK_H__
