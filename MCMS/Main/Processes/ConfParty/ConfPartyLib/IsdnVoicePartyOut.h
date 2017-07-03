#ifndef _PARTYVOICEOUT_H
#define _PARTYVOICEOUT_H

#include "IsdnVoiceParty.h"
#include "PartyRsrcDesc.h"

extern "C" void PartyVoiceOutEntryPoint(void* appParam);


////////////////////////////////////////////////////////////////////////////
//                        CIsdnVoicePartyOut
////////////////////////////////////////////////////////////////////////////
class CIsdnVoicePartyOut : public CIsdnVoiceParty
{
	CLASS_TYPE_1(CIsdnVoicePartyOut, CIsdnVoiceParty)

public:
	            CIsdnVoicePartyOut();
	virtual    ~CIsdnVoicePartyOut();
	const char* NameOf() const { return "CIsdnVoicePartyOut";}

	void        Create(CSegment& appParam);

protected:
	void        OnConfEstablishCallIdle(CSegment* pParam);
	void        OnConfEstablishCallSetup(CSegment* pParam);
	void        OnNetConnectSetUp(CSegment* pParam);

	// Action function utils
	void        EstablishOutCall();
	void        DestroyNetConnection(WORD chnl);
	void        SetNetSetup(CIsdnPartyRsrcDesc& pPartyRsrcDesc); // rons

	PDECLAR_MESSAGE_MAP
};

#endif /* _PARTYVOICEOUT_H */
