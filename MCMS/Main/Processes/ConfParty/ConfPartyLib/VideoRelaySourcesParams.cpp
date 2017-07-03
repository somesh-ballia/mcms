#include <sstream>
#include "VideoRelaySourcesParams.h"

///////////////////////////////////////////////////
CVideoRelaySourcesParams::CVideoRelaySourcesParams()
:m_channelHandle(0)
,m_seqNum(0)
,m_sourceOperationPointSetId(0)
{
	//channel handle
	//video sources
	m_videoSources.clear();

	//seq num
	//source operation point set id
}

///////////////////////////////////////////////////
CVideoRelaySourcesParams::CVideoRelaySourcesParams(const CVideoRelaySourcesParams &other)
:CPObject(other)
,m_channelHandle(other.m_channelHandle)
,m_seqNum(other.m_seqNum)
,m_sourceOperationPointSetId(other.m_sourceOperationPointSetId)
{

	//channel handle

	//video sources
	m_videoSources.assign(other.m_videoSources.begin(),
				other.m_videoSources.end());

	//seq num

	//source operation point set id
}

///////////////////////////////////////////////////
CVideoRelaySourcesParams::~CVideoRelaySourcesParams()
{

	//video sources
	m_videoSources.clear();
}

///////////////////////////////////////////////////
void CVideoRelaySourcesParams::InitDefaults()
{

	//channel handle
	m_channelHandle = 0;

	//video sources
	m_videoSources.clear();

	//seq num
	m_seqNum = 0;

	//source operation point set id
	m_sourceOperationPointSetId = 0;
}

///////////////////////////////////////////////////
CVideoRelaySourcesParams& CVideoRelaySourcesParams::operator=(const CVideoRelaySourcesParams &other)
{
	if (this == &other)
		return *this;

	InitDefaults();

	//channel handle
	m_channelHandle = other.m_channelHandle;

	//video sources
	m_videoSources.clear();
	m_videoSources.assign(other.m_videoSources.begin(), other.m_videoSources.end());

	//seq num
	m_seqNum = other.m_seqNum;

	//source operation point set id
	m_sourceOperationPointSetId = other.m_sourceOperationPointSetId;

	return *this;
}

///////////////////////////////////////////////////
bool CVideoRelaySourcesParams::IsTheSame(const CVideoRelaySourcesParams& other) const
{
	//channel handle
	if (m_channelHandle != other.m_channelHandle)
		return false;

	//video sources
	if (m_videoSources.size() != other.m_videoSources.size() )
		return false;
	if (!std::equal(m_videoSources.begin(),
					m_videoSources.end(),
					other.m_videoSources.begin()))
		 return false;

	//seq num
	if (m_seqNum != other.m_seqNum)
		return false;

	//source operation point set id
	if (m_sourceOperationPointSetId != other.m_sourceOperationPointSetId)
		return false;

	return true;
}

///////////////////////////////////////////////////
bool CVideoRelaySourcesParams::operator==(const CVideoRelaySourcesParams& other) const
{

	//channel handle
	if (m_channelHandle != other.m_channelHandle)
		return false;

	//video sources
	if (m_videoSources.size() != other.m_videoSources.size() )
		return false;

	if (!std::equal(m_videoSources.begin(),
					m_videoSources.end(),
					other.m_videoSources.begin()))
		 return false;

	//seq num
	if (m_seqNum != other.m_seqNum)
		return false;

	//source operation point set id
	if (m_sourceOperationPointSetId != other.m_sourceOperationPointSetId)
		return false;

	return true;
}

///////////////////////////////////////////////////
bool CVideoRelaySourcesParams::operator!=(const CVideoRelaySourcesParams& other) const
{
	return !(*this == other);
}

///////////////////////////////////////////////////
void CVideoRelaySourcesParams::Dump()const
{
    std::ostringstream str;

    str << "CVideoRelaySourcesParams::Dump";
    str << "\nm_channelHandle: " << m_channelHandle;
    str << "\nm_seqNum: " << m_seqNum;
    str << "\nm_sourceOperationPointSetId: " << m_sourceOperationPointSetId;
    std::list <CVideoRelaySourceApi>::const_iterator videoSourcesIter = m_videoSources.begin();
    for( ; videoSourcesIter != m_videoSources.end(); ++videoSourcesIter)
    {
        str << "\n\t m_channelHandle: " << videoSourcesIter->GetChannelHandle();
        str << "\n\t m_syncSource: " << videoSourcesIter->GetSyncSource();
        str << "\n\t m_layerId: " << (WORD)videoSourcesIter->GetLayerId();
        str << "\n\t m_pipeId: " << videoSourcesIter->GetPipeId();
        str << "\n\t m_isSpeaker: " << videoSourcesIter->GetIsSpeaker() << "\n";
        str << "\n\t m_tid: " << videoSourcesIter->GetTid() << "\n";
    }
    PTRACE(eLevelInfoNormal,str.str().c_str());
}
///////////////////////////////////////////////////
int CVideoRelaySourcesParams::GetNumSources()
{
	int numSources = m_videoSources.size();
	return numSources;
}
bool CVideoRelaySourcesParams::IsVideoRelaySourcesHasSource(unsigned int ssrc)
{
	bool isVideoRelaySourcesHasSource = false;
	std::list <CVideoRelaySourceApi>::const_iterator videoSourcesIter = m_videoSources.begin();
    for( ; videoSourcesIter != m_videoSources.end(); ++videoSourcesIter)
    {
    	if(videoSourcesIter->GetSyncSource() == ssrc)
    	{
    		isVideoRelaySourcesHasSource = true;
    		break;
    	}
    }
    return isVideoRelaySourcesHasSource;
}



