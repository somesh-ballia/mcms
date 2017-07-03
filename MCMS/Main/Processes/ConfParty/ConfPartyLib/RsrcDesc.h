#ifndef _RSRCDESC
#define _RSRCDESC

#include "PObject.h"
#include "MplMcmsStructs.h"
#include "Segment.h"
#include "HostCommonDefinitions.h"
#include "ConfPartyDefines.h"

////////////////////////////////////////////////////////////////////////////
//                        CRsrcDesc
////////////////////////////////////////////////////////////////////////////
class CRsrcDesc : public CPObject
{
	CLASS_TYPE_1(CRsrcDesc, CPObject)

public:
	                         CRsrcDesc(ConnectionID connectionId = DUMMY_CONNECTION_ID, eLogicalResourceTypes lrt = eLogical_res_none);
	                         CRsrcDesc(const CRsrcDesc&);
	virtual                 ~CRsrcDesc();
	virtual const char*      NameOf() const { return "CRsrcDesc";}

	virtual const CRsrcDesc& operator=(const CRsrcDesc&);
	friend WORD              operator==(const CRsrcDesc& lhs, const CRsrcDesc& rhs);
	friend bool              operator<(const CRsrcDesc& lhs, const CRsrcDesc& rhs);

	eLogicalResourceTypes    GetLogicalRsrcType() const                    { return m_logicalRsrcType; }
	ConnectionID             GetConnectionId() const                       { return m_connectionId; }

	void                     SetLogicalRsrcType(eLogicalResourceTypes lrt) { m_logicalRsrcType = lrt; }
	void                     SetConnectionId(ConnectionID ConnectionId)    { m_connectionId = ConnectionId; }

	virtual void             Serialize(WORD format, CSegment& seg);
	virtual void             DeSerialize(WORD format, CSegment& seg);

protected:
	ConnectionID             m_connectionId;
	eLogicalResourceTypes    m_logicalRsrcType;
};

#endif // _RSRCDESC
