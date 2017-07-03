#include "RsrcDesc.h"

////////////////////////////////////////////////////////////////////////////
//                        CRsrcDesc
////////////////////////////////////////////////////////////////////////////
CRsrcDesc::CRsrcDesc(ConnectionID connectionId, eLogicalResourceTypes lrt)
{
	m_connectionId    = connectionId;
	m_logicalRsrcType = lrt;
}

//--------------------------------------------------------------------------
CRsrcDesc::~CRsrcDesc()
{
}

//--------------------------------------------------------------------------
CRsrcDesc::CRsrcDesc(const CRsrcDesc& rhs) : CPObject(rhs)
{
	m_connectionId    = rhs.m_connectionId;
	m_logicalRsrcType = rhs.m_logicalRsrcType;
}

//--------------------------------------------------------------------------
const CRsrcDesc& CRsrcDesc::operator=(const CRsrcDesc& rhs)
{
	if (&rhs == this)
		return *this;

	m_connectionId    = rhs.m_connectionId;
	m_logicalRsrcType = rhs.m_logicalRsrcType;

	return *this;
}

//--------------------------------------------------------------------------
WORD operator==(const CRsrcDesc& lhs, const CRsrcDesc& rhs)
{
	return (lhs.m_connectionId == rhs.m_connectionId && lhs.m_logicalRsrcType == rhs.m_logicalRsrcType);
}

//--------------------------------------------------------------------------
bool operator<(const CRsrcDesc& lhs, const CRsrcDesc& rhs)
{
	if (lhs.m_connectionId != rhs.m_connectionId)
		return lhs.m_connectionId < rhs.m_connectionId;

	return lhs.m_logicalRsrcType < rhs.m_logicalRsrcType;
}

//--------------------------------------------------------------------------
void CRsrcDesc::Serialize(WORD format, CSegment& seg)
{
	if (format == NATIVE)
		seg << m_connectionId << (DWORD)m_logicalRsrcType;
}

//--------------------------------------------------------------------------
void CRsrcDesc::DeSerialize(WORD format, CSegment& seg)
{
	if (format == NATIVE)
		seg >> m_connectionId >> (DWORD&)m_logicalRsrcType;
}
