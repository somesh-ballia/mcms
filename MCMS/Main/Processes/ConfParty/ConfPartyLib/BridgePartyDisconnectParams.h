#ifndef _CBridgePartyDisconnectParams_H_
#define _CBridgePartyDisconnectParams_H_

#include "PObject.h"
#include "RsrcDesc.h"
#include "ConfPartyDefines.h"

class CSegment;

////////////////////////////////////////////////////////////////////////////
//                        CBridgePartyDisconnectParams
////////////////////////////////////////////////////////////////////////////
class CBridgePartyDisconnectParams : public CPObject
{
	CLASS_TYPE_1(CBridgePartyDisconnectParams, CPObject)

public:
	CBridgePartyDisconnectParams();
	CBridgePartyDisconnectParams(PartyRsrcID partyId, EMediaDirection eMediaDirection = eMediaInAndOut);
	CBridgePartyDisconnectParams(const CBridgePartyDisconnectParams& rhs);
	virtual ~CBridgePartyDisconnectParams();

	virtual const char* NameOf() const { return "CBridgePartyDisconnectParams";}
	CBridgePartyDisconnectParams& operator =(const CBridgePartyDisconnectParams& rhs);

	PartyRsrcID     GetPartyId() const   { return m_partyId; }
	EMediaDirection GetMediaDirection()  { return m_eMediaDirection; }

	virtual void    Serialize(WORD format, CSegment& seg) const;
	virtual void    DeSerialize(WORD format, CSegment& seg);

private:
	PartyRsrcID     m_partyId;
	EMediaDirection m_eMediaDirection;
};

#endif
