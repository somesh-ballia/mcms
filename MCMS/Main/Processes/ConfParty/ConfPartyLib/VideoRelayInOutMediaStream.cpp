#include "VideoRelayInOutMediaStream.h"

///////////////////////////////////////////////////
CVideoRelayOutMediaStream::CVideoRelayOutMediaStream()
:CVideoRelayMediaStream()
,m_csrc(0),m_priority(0),m_bTemporalScalabilitySupported(false)
{
}

///////////////////////////////////////////////////
CVideoRelayOutMediaStream::CVideoRelayOutMediaStream(const CVideoRelayOutMediaStream &other)
:CVideoRelayMediaStream(other)
,m_csrc(other.m_csrc), m_priority(other.m_priority),m_bTemporalScalabilitySupported(other.m_bTemporalScalabilitySupported)
{
}

///////////////////////////////////////////////////
CVideoRelayOutMediaStream::~CVideoRelayOutMediaStream()
{
}

///////////////////////////////////////////////////
CVideoRelayOutMediaStream& CVideoRelayOutMediaStream::operator=(const CVideoRelayOutMediaStream &other)
{
	if (this == &other)
		return *this;

	CVideoRelayMediaStream::operator=(other);

	m_csrc = other.m_csrc;
	m_priority = other.m_priority;


	return *this;
}

///////////////////////////////////////////////////
bool CVideoRelayOutMediaStream::operator==(const CVideoRelayOutMediaStream& other) const
{
	if (CVideoRelayMediaStream::operator==(other) == false)
		return false;

	if (m_csrc != other.m_csrc)
		return false;
	if(m_priority != other.m_priority)
		return false;

	return true;
}

///////////////////////////////////////////////////
bool CVideoRelayOutMediaStream::operator!=(const CVideoRelayOutMediaStream& other) const
{
	return !(*this == other);
}
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////

///////////////////////////////////////////////////
CVideoRelayInMediaStream::CVideoRelayInMediaStream()
:CVideoRelayMediaStream(),
 m_scpPipe()
{
}

///////////////////////////////////////////////////
CVideoRelayInMediaStream::CVideoRelayInMediaStream(const CVideoRelayInMediaStream &other)
:CVideoRelayMediaStream(other)
,m_scpPipe(other.m_scpPipe)
{
}

///////////////////////////////////////////////////
CVideoRelayInMediaStream::~CVideoRelayInMediaStream()
{
}

///////////////////////////////////////////////////
CVideoRelayInMediaStream& CVideoRelayInMediaStream::operator=(const CVideoRelayInMediaStream &other)
{
    if (this == &other)
        return *this;

    CVideoRelayMediaStream::operator=(other);

    m_scpPipe = other.m_scpPipe;

    return *this;
}

///////////////////////////////////////////////////
bool CVideoRelayInMediaStream::operator==(const CVideoRelayInMediaStream& other) const
{
    if (CVideoRelayMediaStream::operator==(other) == false)
        return false;

    if ( !(m_scpPipe == other.m_scpPipe) )
        return false;

    return true;
}

///////////////////////////////////////////////////
bool CVideoRelayInMediaStream::operator!=(const CVideoRelayInMediaStream& other) const
{
    return !(*this == other);
}
