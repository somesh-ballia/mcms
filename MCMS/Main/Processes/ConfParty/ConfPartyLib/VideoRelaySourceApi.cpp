#include "VideoRelaySourceApi.h"

///////////////////////////////////////////////////
CVideoRelaySourceApi::CVideoRelaySourceApi()
:m_channelHandle(0)
,m_syncSource(0)
,m_layerId(0)
,m_pipeId(0)
,m_isSpeaker(false)
,m_tid(0)
{}

///////////////////////////////////////////////////
CVideoRelaySourceApi::CVideoRelaySourceApi(const CVideoRelaySourceApi &other)
:CPObject(other)
,m_channelHandle(other.m_channelHandle)
,m_syncSource(other.m_syncSource)
,m_layerId(other.m_layerId)
,m_pipeId(other.m_pipeId)
,m_isSpeaker(other.m_isSpeaker)
,m_tid(other.m_tid)
{}

///////////////////////////////////////////////////
CVideoRelaySourceApi::~CVideoRelaySourceApi()
{
}

///////////////////////////////////////////////////
void CVideoRelaySourceApi::InitDefaults()
{

	//channel handle
	m_channelHandle = 0;

	//sync source
	m_syncSource = 0;

	//svc params
	m_layerId = 0;//m_svcParams.InitDefaults();

	//pipe id
	m_pipeId = 0;

	//is speaker
	m_isSpeaker = false;

	//tid
	m_tid = 0;
}

///////////////////////////////////////////////////
CVideoRelaySourceApi& CVideoRelaySourceApi::operator=(const CVideoRelaySourceApi &other)
{
	if (this == &other)
		return *this;

	InitDefaults();

	//channel handle
	m_channelHandle = other.m_channelHandle;

	//sync source
	m_syncSource = other.m_syncSource;

	//svc params
	m_layerId = other.m_layerId; //m_svcParams = other.m_svcParams;

	//pipe id
	m_pipeId = other.m_pipeId;

	//is speaker
	m_isSpeaker = other.m_isSpeaker;

	//TID
	m_tid = other.m_tid;

	return *this;
}

///////////////////////////////////////////////////
bool CVideoRelaySourceApi::IsTheSame(const CVideoRelaySourceApi& other) const
{
	//channel handle
	if (m_channelHandle != other.m_channelHandle)
		return false;

	//sync source
	if (m_syncSource != other.m_syncSource)
		return false;

	//svc params
	if (m_layerId != other.m_layerId) // if (m_svcParams != other.m_svcParams)
		return false;

	//pipe id
	if (m_pipeId != other.m_pipeId)
		return false;

	//is speaker
	if (m_isSpeaker != other.m_isSpeaker)
		return false;
	//tid
	if(m_tid !=other.m_tid)
		return false;

	return true;
}

///////////////////////////////////////////////////
bool CVideoRelaySourceApi::operator==(const CVideoRelaySourceApi& other) const
{

	//channel handle
	if (m_channelHandle != other.m_channelHandle)
		return false;

	//sync source
	if (m_syncSource != other.m_syncSource)
		return false;

	//svc params
	if (m_layerId != other.m_layerId) // if (m_svcParams != other.m_svcParams)
		return false;

	//pipe id
	if (m_pipeId != other.m_pipeId)
		return false;

	//is speaker
	if (m_isSpeaker != other.m_isSpeaker)
		return false;

	if(m_tid!=other.m_tid)
		return false;

	return true;
}

///////////////////////////////////////////////////
bool CVideoRelaySourceApi::operator!=(const CVideoRelaySourceApi& other) const
{
	return !(*this == other);
}
