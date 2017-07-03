#ifndef _CBridgePartyExportParams_H_
#define _CBridgePartyExporttParams_H_

#include "PObject.h"
#include "RsrcDesc.h"

class CSegment;

////////////////////////////////////////////////////////////////////////////
//                        CBridgePartyExportParams
////////////////////////////////////////////////////////////////////////////
class CBridgePartyExportParams : public CPObject
{
	CLASS_TYPE_1(CBridgePartyExportParams, CPObject)

public:
	CBridgePartyExportParams();
	CBridgePartyExportParams(PartyRsrcID partyId);
	CBridgePartyExportParams(const CBridgePartyExportParams& rhs);
	virtual ~CBridgePartyExportParams();

	virtual const char* NameOf() const { return "CBridgePartyExportParams"; }
	CBridgePartyExportParams& operator=(const CBridgePartyExportParams& rhs);

	PartyRsrcID  GetPartyId() const { return m_partyId;}

	virtual void Serialize(WORD format, CSegment& seg) const;
	virtual void DeSerialize(WORD format, CSegment& seg);

private:
	PartyRsrcID  m_partyId;
};

#endif
