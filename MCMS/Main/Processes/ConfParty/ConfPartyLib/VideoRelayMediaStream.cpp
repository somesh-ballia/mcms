#include "VideoRelayMediaStream.h"
#include "TraceStream.h"

///////////////////////////////////////////////////
CVideoRelayMediaStream::CVideoRelayMediaStream()
:CRelayMediaStream()
,m_layerId(0)
,m_resolutionHeight(0)
,m_resolutionWidth(0)
,m_bIsVswStream(FALSE)
{
}

///////////////////////////////////////////////////
CVideoRelayMediaStream::CVideoRelayMediaStream(const CVideoRelayMediaStream &other)
:CRelayMediaStream(other)
,m_layerId(other.m_layerId)
,m_resolutionHeight(other.m_resolutionHeight)
,m_resolutionWidth(other.m_resolutionWidth)
,m_bIsVswStream(other.m_bIsVswStream)
{
}

///////////////////////////////////////////////////
CVideoRelayMediaStream::~CVideoRelayMediaStream()
{
}

///////////////////////////////////////////////////
CVideoRelayMediaStream& CVideoRelayMediaStream::operator=(const CVideoRelayMediaStream &other)
{
	if (this == &other)
		return *this;

	CRelayMediaStream::operator=(other);

	m_layerId = other.m_layerId;
	m_resolutionHeight = other.m_resolutionHeight;
	m_resolutionWidth = other.m_resolutionWidth;
	m_bIsVswStream = other.m_bIsVswStream;
	return *this;
}

///////////////////////////////////////////////////
bool CVideoRelayMediaStream::operator==(const CVideoRelayMediaStream& other) const
{
	if (CRelayMediaStream::operator==(other) == false)
		return false;

	if (m_layerId != other.m_layerId)
		return false;

	if (m_resolutionHeight != other.m_resolutionHeight)
		return false;

	if (m_resolutionWidth != other.m_resolutionWidth)
		return false;
	if (m_bIsVswStream != other.m_bIsVswStream)
		return false;

	return true;
}

///////////////////////////////////////////////////
bool CVideoRelayMediaStream::operator!=(const CVideoRelayMediaStream& other) const
{
	return !(*this == other);
}


///////////////////////////////////////////////////
void CVideoRelayMediaStream::SetLayerId(int id)
{
	m_layerId = id;
}
///////////////////////////////////////////////////
BOOL CVideoRelayMediaStream::IsValidParams() const
{
	if( m_resolutionHeight==0)
	{
		  TRACEINTO << " CVideoRelayMediaStream::IsValidParams INVALID m_resolutionHeight";
		  return FALSE;
	}
	if( m_resolutionWidth==0)
	{
		TRACEINTO << " CVideoRelayMediaStream::IsValidParams INVALID m_resolutionWidth";
		return FALSE;
	}

	return TRUE;

}
