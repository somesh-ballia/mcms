#include "RelayMediaStream.h"

///////////////////////////////////////////////////
CRelayMediaStream::CRelayMediaStream()
:m_ssrc(0)
,m_bIsSpecificSourceCsrc(false)
{
}

///////////////////////////////////////////////////
CRelayMediaStream::CRelayMediaStream(const CRelayMediaStream &other)
:CPObject(other),
 m_ssrc(other.m_ssrc)
,m_bIsSpecificSourceCsrc(other.m_bIsSpecificSourceCsrc)
{
}

///////////////////////////////////////////////////
CRelayMediaStream::~CRelayMediaStream()
{
}

///////////////////////////////////////////////////
CRelayMediaStream& CRelayMediaStream::operator=(const CRelayMediaStream &other)
{
	if (this == &other)
		return *this;

	m_ssrc = other.m_ssrc;
	m_bIsSpecificSourceCsrc = other.m_bIsSpecificSourceCsrc;

	return *this;
}

///////////////////////////////////////////////////
bool CRelayMediaStream::operator==(const CRelayMediaStream& other) const
{
	if (m_ssrc != other.m_ssrc)
		return false;

	if (m_bIsSpecificSourceCsrc != other.m_bIsSpecificSourceCsrc)
		return false;

	return true;
}

///////////////////////////////////////////////////
bool CRelayMediaStream::operator!=(const CRelayMediaStream& other) const
{
	return !(*this == other);
}
