#include "VideoOperationPoint.h"
#include <sstream>


VideoOperationPoint::VideoOperationPoint()
:m_layerId(0)
,m_tid(0)
,m_did(0)
,m_qid(0)
,m_frameWidth(0)
,m_frameHeight(0)
,m_frameRate(0)
,m_maxBitRate(0)
,m_videoProfile(0)
,m_streamId(0)
,m_rsrcLevel(eResourceLevel_CIF)
{
	//layer id
	//tid
	//did
	//qid
	//frame width
	//frame height
	//frame rate
	//max bit rate
	//video profile
	//stream id
}

///////////////////////////////////////////////////
VideoOperationPoint::VideoOperationPoint(const VideoOperationPoint &other)
:m_layerId(other.m_layerId)
,m_tid(other.m_tid)
,m_did(other.m_did)
,m_qid(other.m_qid)
,m_frameWidth(other.m_frameWidth)
,m_frameHeight(other.m_frameHeight)
,m_frameRate(other.m_frameRate)
,m_maxBitRate(other.m_maxBitRate)
,m_videoProfile(other.m_videoProfile)
,m_streamId(other.m_streamId)
,m_rsrcLevel(other.m_rsrcLevel)
{

	//layer id

	//tid

	//did

	//qid

	//frame width

	//frame height

	//frame rate

	//max bit rate

	//video profile

	//stream id
}
///////////////////////////////////////////////////
VideoOperationPoint::~VideoOperationPoint()
{
}
///////////////////////////////////////////////////
void VideoOperationPoint::InitDefaults()
{
	memset((void*)this, 0, sizeof(VideoOperationPoint));

	//layer id
	m_layerId = 0;

	//tid
	m_tid = 0;

	//did
	m_did = 0;

	//qid
	m_qid = 0;

	//frame width
	m_frameWidth = 0;

	//frame height
	m_frameHeight = 0;

	//frame rate
	m_frameRate = 0;

	//max bit rate
	m_maxBitRate = 0;

	//video profile
	m_videoProfile = 0;

	//stream id
	m_streamId = 0;

	m_rsrcLevel = eResourceLevel_CIF;
}

