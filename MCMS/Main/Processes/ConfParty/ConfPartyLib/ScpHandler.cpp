

#include "ScpHandler.h"
#include "ConfPartyOpcodes.h"
#include "ScpNotificationWrapper.h"
#define ILLEGAL_LAYER_ID -1

/////////////////////////////////////////////////////////////////////////////
CScpHandler::CScpHandler()
{
	m_pScpStreamCurrentReq	= new CMrmpScpStreamsRequestStructWrap;
	m_pScpStreamNewReq		= new CMrmpScpStreamsRequestStructWrap;
	m_pPartyApi = NULL;

	m_lastScpNotificationLocalSeqNum				= 0;
	m_lastScpStreamsNotificationLocalSeqNum			= 0;
	m_lastScpIvrStateNotification.m_localSeqNum		= 0;
	m_lastScpIvrStateNotification.m_ivrState		= eIvrStateIllegal;
	m_lastScpPipesMappingNotificationLocalSeqNum	= 0;
}

/////////////////////////////////////////////////////////////////////////////
CScpHandler::~CScpHandler()
{
	delete m_pScpStreamCurrentReq;
	delete m_pScpStreamNewReq;
}

/////////////////////////////////////////////////////////////////////////////
CScpHandler::CScpHandler(const CScpHandler &other)
:CPObject(other)
{
	m_pPartyApi = other.m_pPartyApi;
	m_lastScpNotificationLocalSeqNum				= other.m_lastScpNotificationLocalSeqNum;
	m_lastScpStreamsNotificationLocalSeqNum			= other.m_lastScpStreamsNotificationLocalSeqNum;
	m_lastScpIvrStateNotification.m_localSeqNum		= other.m_lastScpIvrStateNotification.m_localSeqNum;
	m_lastScpIvrStateNotification.m_ivrState 		= other.m_lastScpIvrStateNotification.m_ivrState;
	m_lastScpPipesMappingNotificationLocalSeqNum	= other.m_lastScpPipesMappingNotificationLocalSeqNum;

	m_pScpStreamCurrentReq = new CMrmpScpStreamsRequestStructWrap;
	*m_pScpStreamCurrentReq = *other.m_pScpStreamCurrentReq;

	m_pScpStreamNewReq = new CMrmpScpStreamsRequestStructWrap;
	*m_pScpStreamNewReq = *other.m_pScpStreamNewReq;
}

/////////////////////////////////////////////////////////////////////////////
void CScpHandler::Init(CPartyApi* pPartyApi)
{
	m_pPartyApi = pPartyApi;
}

/////////////////////////////////////////////////////////////////////////////
unsigned int CScpHandler::CalculateDesiredBitRateRec(APIS32 aNumberOfMediaStreams, VIDEO_OPERATION_POINT_SET_S* vopS,int* layerId,unsigned int maxVideoBitRate)
{
	int i;
	unsigned int totalBitrate=0;
	unsigned int frameRate;
	unsigned int frameHeight;
	unsigned int frameWidth;
	unsigned int frameBitrate;
	int numberOfStreams = aNumberOfMediaStreams;
	int flag = false;
	int index = ILLEGAL_LAYER_ID;//-1;

	for(i=0; i<numberOfStreams; ++i)
	{
		if (layerId[i]!=ILLEGAL_LAYER_ID/*-1*/)
		{
			totalBitrate += vopS->tVideoOperationPoints[layerId[i]].maxBitRate;
		}
	}

	if (totalBitrate <= maxVideoBitRate)
	{
		TRACEINTOFUNC << "totalBitrate (" << totalBitrate << ") <= maxVideoBitRate (" << maxVideoBitRate << ")";
		return totalBitrate;
	}

	TRACEINTOFUNC << "totalBitrate " << totalBitrate << ", need to reduce stream bit rate";

	for (i=numberOfStreams-1; i>=0 && flag==false; --i)
	{
		if (layerId[i] > 0)
		{
			layerId[i]--;
			index=i;
			flag=true;
			TRACEINTOFUNC << "Reduced stream bit rate index: " << index;
		}
	}

	if (!flag)
	{
		TRACEINTOFUNC << "It stinks that all layers are down to zero";
		
		return 0; // the total bitrate couldn't be resolved even when all streams are down to zero
	}
	
	totalBitrate = CalculateDesiredBitRateRec(numberOfStreams, vopS,layerId,maxVideoBitRate);

	return totalBitrate;
}

unsigned int CScpHandler::GetRidOfStreams(CMrmpStreamDescWrap* apMediaStreams, APIS32 aNumberOfMediaStreams, unsigned int maxVideoBitRate)
{
	int i;
	unsigned int totalBitrate=0;
	int numberOfStreams = aNumberOfMediaStreams;
	bool flag = false;
	int index = ILLEGAL_LAYER_ID;//-1;
	CMrmpStreamDescWrap* pStreamDesc = apMediaStreams;

	for (i=0; i<numberOfStreams && flag==false; ++i)
	{
		totalBitrate += pStreamDesc[i].GetBitRate();
		
		if (totalBitrate > maxVideoBitRate)
		{
			flag=true;
			index=i;
			totalBitrate -= pStreamDesc[i].GetBitRate();
		}
	}
	
	if (flag)
	{
		for (i=numberOfStreams-1; i>=index; --i)
		{
			pStreamDesc[i].SetIsLegal(false);
		}

		TRACEINTOFUNC << "##!! avc_vsw_relay Removing stream upto num: " << index;
	}
	
	return totalBitrate;
}

unsigned int CScpHandler::CalculateDesiredBitRate(CMrmpStreamDescWrap* apMediaStreams, APIS32 aNumberOfMediaStreams, CIpComMode*  apTargetMode,CSipCaps* apSipLocalCaps, BYTE aIsScpRequest)
{
	STREAM_GROUP_S* pSendStreamGroup;
	int i;
	unsigned int totalBitrate=0;
	int maxVideoBitRate;
	unsigned int frameRate;
    unsigned int frameHeight;
	unsigned int frameWidth;
	unsigned int frameBitrate;
	int numberOfStreams = aNumberOfMediaStreams;
    DWORD contentRate = apTargetMode->GetMediaBitRate(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
	CMrmpStreamDescWrap* pStreamDesc = apMediaStreams;
	int* layerId = new int[numberOfStreams];
	VIDEO_OPERATION_POINT_SET_S* vopS = apTargetMode->GetOperationPoints(cmCapVideo, cmCapReceive,kRolePeople);
	
	if (vopS==NULL)
	{
		TRACEINTOFUNC << "VopS == NULL";
    	delete[] layerId;
        return totalBitrate;
	}
	
	// get layer id for each stream
	CLargeString msgStr;

	msgStr << "CScpHandler::CalculateDesiredBitRate: NumberOfStreams: " << numberOfStreams << "\n";
	
    for (i=0; i<numberOfStreams; ++i)
    {
    	frameRate = pStreamDesc[i].GetFrameRate();
		
        if (aIsScpRequest)
        {
    	    frameRate = ::GetFrameRateInSignalingValues(frameRate);
        }
		
    	frameHeight  = pStreamDesc[i].GetFrameHeight();
    	frameWidth   = pStreamDesc[i].GetFrameWidth();
    	frameBitrate = pStreamDesc[i].GetBitRate();
		
    	DWORD partyId = apTargetMode->GetPartyId();
		
 		layerId[i] = ::GetLayerId(*vopS, frameWidth, frameHeight, frameRate, frameBitrate, cmCapTransmit, partyId, &msgStr);
		
		if (layerId[i] == ILLEGAL_LAYER_ID)
		{
			msgStr<<"##!! avc_vsw_relay Can't find a valid operation point for partyId:"<<partyId<<" for stream i:"<<i;
			PTRACE2(eLevelInfoNormal,"info:",msgStr.GetString());
			TRACEINTOFUNC << "Can't find a valid operation point for stream i: " << i << " frameHeigth=" << frameHeight
			        << " frameWidth=" << frameWidth << " frameRate=" << frameRate << " frameBitrate=" << frameBitrate;
		}
		else
		{
			PTRACE2(eLevelInfoNormal,"info: ", msgStr.GetString());
		}
    }

    maxVideoBitRate = apSipLocalCaps->GetMaxVideoBitRate() - contentRate;
	
    if (maxVideoBitRate < 0)
    {// even content cannot be displayed - @@@ what shall we do in this case?
        maxVideoBitRate = 0;
    }

    TRACEINTOFUNC << "maxVideoBitRate: " << maxVideoBitRate
    			  << "\n pSipLocalCaps->GetMaxVideoBitRate(): " << apSipLocalCaps->GetMaxVideoBitRate()
    			  << "\n contentRate: " << contentRate;

    maxVideoBitRate = maxVideoBitRate/10;
	totalBitrate = CalculateDesiredBitRateRec(numberOfStreams, vopS,layerId, maxVideoBitRate);

    for (i=0; i<numberOfStreams; ++i)
    {
    	pStreamDesc[i].SetFrameRate(vopS->tVideoOperationPoints[layerId[i]].frameRate);
    	pStreamDesc[i].SetFrameHeight(vopS->tVideoOperationPoints[layerId[i]].frameHeight);
    	pStreamDesc[i].SetFrameWidth(vopS->tVideoOperationPoints[layerId[i]].frameWidth);
    	pStreamDesc[i].SetBitRate(vopS->tVideoOperationPoints[layerId[i]].maxBitRate);
    	pStreamDesc[i].SetIsLegal(TRUE);
    }
	
    if (totalBitrate == 0)
    {		
    	totalBitrate = GetRidOfStreams(apMediaStreams, numberOfStreams, maxVideoBitRate);

		TRACEINTOFUNC << "TotalBitrate: " << totalBitrate;
    }

    for (i=0; i<numberOfStreams; ++i)
    {
    	TRACEINTOFUNC << "LayerId[" << i << "]: " << layerId[i];

    	frameRate    = pStreamDesc[i].GetFrameRate();
    	frameHeight  = pStreamDesc[i].GetFrameHeight();
    	frameWidth   = pStreamDesc[i].GetFrameWidth();
    	frameBitrate = pStreamDesc[i].GetBitRate();

    	TRACEINTOFUNC << "Final frameRate:      " << frameRate
    				  << "\nFinal frameHeight:  " << frameHeight
    				  << "\nFinal frameWidth:   " << frameWidth
    				  << "\nFinal frameBitrate: " << frameBitrate;
    }
	
    delete [] layerId;

    return totalBitrate;
}

/////////////////////////////////////////////////////////////////////////////
void CScpHandler::UpdateScm(CMrmpStreamDescWrap* apMediaStreams,
							APIS32 aNumberOfMediaStreams,
							CIpComMode*  pTargetMode,
							cmCapDirection direction)
{
	TRACEINTOFUNC << "Updating SCM with new streams, num of streams: " << aNumberOfMediaStreams;

	std::list <StreamDesc> tmpStreamsList;
	tmpStreamsList.clear();

	for (APIS32 i=0; i<aNumberOfMediaStreams; ++i)
	{
		StreamDesc tmpDesc;
		tmpDesc.InitDefaults();

		tmpDesc.m_pipeIdSsrc = apMediaStreams[i].GetFramePipeIdSsrc();//GetFrameSourceIdSsrc();//???
		tmpDesc.m_width = apMediaStreams[i].GetFrameWidth();
		tmpDesc.m_height = apMediaStreams[i].GetFrameHeight();
		tmpDesc.m_frameRate = apMediaStreams[i].GetFrameRate();
		tmpDesc.m_payloadType = (payload_en)105;
		tmpDesc.m_scpNotificationParams.m_pipeId = tmpDesc.m_pipeIdSsrc;
		tmpDesc.m_priority = apMediaStreams[i].GetPriority();
        tmpDesc.m_isLegal = apMediaStreams[i].GetIsLegal();

		if (apMediaStreams[i].GetSpecificSourceSsrc())
		{
			tmpDesc.m_specificSourceSsrc = true;
			tmpDesc.m_sourceIdSsrc = apMediaStreams[i].GetFrameSourceIdSsrc();
		}
		else
		{
			tmpDesc.m_specificSourceSsrc = false;
			tmpDesc.m_sourceIdSsrc = 0;
		}

		tmpStreamsList.push_back(tmpDesc);
	}

	CMediaModeH323 &rMediaModeH323 = pTargetMode->GetMediaMode(cmCapVideo, direction);
	rMediaModeH323.SetStreamsList(tmpStreamsList, pTargetMode->GetPartyId());

	pTargetMode->Dump("CScpHandler::UpdateScm _scp_flow_ - after update:",eLevelInfoNormal);
}

/////////////////////////////////////////////////////////////////////////////
bool CScpHandler::HandleScpRequest(CMrmpScpStreamsRequestStructWrap &aScpStreamReq, CIpComMode*  pTargetMode,CSipCaps* pSipLocalCaps)
{
	unsigned int totalBitrate;
	PTRACE(eLevelInfoNormal, "CScpHandler::HandleScpRequest ");

	if (NULL == m_pPartyApi)
	{
		PTRACE(eLevelInfoNormal,"_scp_flow_ CScpHandler::HandleScpRequest - m_pPartyApi is NULL");
		PASSERT(1);
		return FALSE;
	}

	*m_pScpStreamNewReq = aScpStreamReq;
	m_pScpStreamNewReq->Print();
    bool needToSendNotification = false;

    if (m_pScpStreamCurrentReq->GetSequenceNumber() > m_pScpStreamNewReq->GetSequenceNumber())
	{
    	TRACEINTOFUNC << "New scp request received with sequence number: " << m_pScpStreamNewReq->GetSequenceNumber()
    				  << "\nWhile lastScpRequestSeqNumber is:" << m_pScpStreamCurrentReq->GetSequenceNumber();
	    return false;
	}

    totalBitrate = CalculateDesiredBitRate(m_pScpStreamNewReq->GetMediaStreams(), m_pScpStreamNewReq->GetNumberOfMediaStreams(),
                                           pTargetMode,pSipLocalCaps, TRUE);

    UpdateScm(m_pScpStreamNewReq->GetMediaStreams(), m_pScpStreamNewReq->GetNumberOfMediaStreams(), pTargetMode, cmCapTransmit);

	*m_pScpStreamCurrentReq = *m_pScpStreamNewReq;
    m_notificationsMap.clear();

	SendAckForReq();

	return true;
}

/////////////////////////////////////////////////////////////////////////////
bool CScpHandler::HandleScpNotificationInd(CScpNotificationWrapper &aScpNotifyInd, CIpComMode* pTargetMode)
{
	PTRACE(eLevelInfoNormal, "CScpHandler::HandleScpNotificationInd");

	CMediaModeH323 &rMediaModeH323 = pTargetMode->GetMediaMode(cmCapVideo, cmCapReceive);
	bool bIsStreamExists = rMediaModeH323.UpdateStreamsScpNotificationParams(&aScpNotifyInd);
	if (false == bIsStreamExists)
	{
		TRACEINTOFUNC << "No stream exists (channelHandle: " << aScpNotifyInd.m_channelHandle << ", seqNum: " << aScpNotifyInd.m_sequenceNumber << ")";
	}

	SendAckForNotification(aScpNotifyInd.m_sequenceNumber);

	return true;
}

/////////////////////////////////////////////////////////////////////////////
void CScpHandler::SendAckForReq()
{
	if (NULL == m_pPartyApi)
	{
		TRACEINTOFUNC << "m_pPartyApi is NULL";
		PASSERT(1);
		return;
	}

	PTRACE(eLevelInfoNormal,"CScpHandler::SendAckForReq ");
	m_pPartyApi->SendAckForScpReq(m_pScpStreamNewReq->GetSequenceNumber());
}

/////////////////////////////////////////////////////////////////////////////
void CScpHandler::SendAckForNotification(APIU32 sequenceNumber)
{
	if (NULL == m_pPartyApi)
	{
		TRACEINTOFUNC << "m_pPartyApi is NULL";
		PASSERT(1);
		return;
	}

	PTRACE(eLevelInfoNormal,"CScpHandler::SendAckForNotification");

	CSegment* pSeg = new CSegment;
	*pSeg << sequenceNumber;

	m_pPartyApi->SendAckForScpNotificationInd(pSeg);
}

/////////////////////////////////////////////////////////////////
unsigned int CScpHandler::GetLastScpRequestSeqNumber()
{
	return m_pScpStreamCurrentReq->GetSequenceNumber();
}

/////////////////////////////////////////////////////////////////
unsigned int CScpHandler::GetLastScpStreamsNotificationLocalSeqNumber()
{
	return m_lastScpStreamsNotificationLocalSeqNum;
}

/////////////////////////////////////////////////////////////////
unsigned int CScpHandler::GetLastScpIvrStateNotificationLocalSeqNumber()
{
	return m_lastScpIvrStateNotification.m_localSeqNum;
}

/////////////////////////////////////////////////////////////////
unsigned int CScpHandler::GetLastScpPipesMappingNotificationLocalSeqNumber()
{
	return m_lastScpPipesMappingNotificationLocalSeqNum;
}

/////////////////////////////////////////////////////////////////
void CScpHandler::UpdateLastScpStreamsNotificationLocalSeqNumber()
{
	// Update the global counter
	// Update the streams notification counter
	m_lastScpStreamsNotificationLocalSeqNum = IncreaseScpNotificationLocalSeqNumber();
}

/////////////////////////////////////////////////////////////////
void CScpHandler::UpdateLastScpPipesMappingNotificationLocalSeqNumber()
{
	// Update the global counter
	// Update the pipes mapping notification counter
	m_lastScpPipesMappingNotificationLocalSeqNum = IncreaseScpNotificationLocalSeqNumber();
}
/////////////////////////////////////////////////////////////////
unsigned int CScpHandler::IncreaseScpNotificationLocalSeqNumber()
{
	++m_lastScpNotificationLocalSeqNum;
	return m_lastScpNotificationLocalSeqNum;
}

/////////////////////////////////////////////////////////////////
void CScpHandler::UpdateNotificationsMap(const CScpNotificationWrapper &notifyReq)
{
	ESipMediaChannelType mediaChannelType = ::GetSipMediaChannelType(cmCapData, kRolePeople);

	m_notificationsMap[mediaChannelType].clear();
	m_notificationsMap[mediaChannelType].assign(notifyReq.m_pipes.begin(), notifyReq.m_pipes.end());
}
/////////////////////////////////////////////////////////////////
void CScpHandler::UpdateLastScpIvrStateNotification(eIvrState newState)
{
	m_lastScpIvrStateNotification.m_localSeqNum = IncreaseScpNotificationLocalSeqNumber();
	if (newState != m_lastScpIvrStateNotification.m_ivrState)
		m_lastScpIvrStateNotification.m_ivrState = newState;
	else //Eitan - just for debug
		PASSERTMSG(m_lastScpIvrStateNotification.m_localSeqNum,"CScpHandler::UpdateLastScpIvrStateNotification new ivr state equals current ivr state - should not get here");
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
CMrmpScpStreamsRequestStructWrap::CMrmpScpStreamsRequestStructWrap()
{
	m_ChannelHandle=0;
	m_SequenceNumber=0;
	m_NumberOfMediaStream=0;
	m_mediaStreams=NULL;
}

/////////////////////////////////////////////////////////////////////////////
CMrmpScpStreamsRequestStructWrap& CMrmpScpStreamsRequestStructWrap::operator= (const CMrmpScpStreamsRequestStructWrap &other)
{
	if ( (m_NumberOfMediaStream != 0) && (NULL != m_mediaStreams) )
	{
		delete [] m_mediaStreams;
		m_mediaStreams = NULL;
	}

  	m_ChannelHandle			= other.m_ChannelHandle;
  	m_SequenceNumber		= other.m_SequenceNumber;
  	m_NumberOfMediaStream	= other.m_NumberOfMediaStream;

	if (0 < m_NumberOfMediaStream)
	{
		m_mediaStreams = new CMrmpStreamDescWrap[m_NumberOfMediaStream];

		for(int i=0; i<m_NumberOfMediaStream; ++i)
		{
			m_mediaStreams[i] = other.m_mediaStreams[i];
		}
	}

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CMrmpScpStreamsRequestStructWrap& CMrmpScpStreamsRequestStructWrap::operator= (const MrmpScpStreamsRequestStruct &other)
{
    if ( (m_NumberOfMediaStream != 0) && (NULL != m_mediaStreams) )
    {
        delete [] m_mediaStreams;
        m_mediaStreams = NULL;
    }

    m_ChannelHandle         = other.unChannelHandle;
    m_SequenceNumber        = other.unSequenseNumber;
    m_NumberOfMediaStream   = other.nNumberOfMediaStream;

    if (0 < m_NumberOfMediaStream)
    {
        m_mediaStreams = new CMrmpStreamDescWrap[m_NumberOfMediaStream];

        for(int i=0; i<m_NumberOfMediaStream; ++i)
        {
            m_mediaStreams[i] = other.mediaStreams[i];
        }
    }

    return *this;
}

/////////////////////////////////////////////////////////////////////////////
CMrmpScpStreamsRequestStructWrap::~CMrmpScpStreamsRequestStructWrap()
{
	if ( (m_NumberOfMediaStream != 0) && (NULL != m_mediaStreams) )
	{
		delete [] m_mediaStreams;
		m_mediaStreams = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////
void  CMrmpScpStreamsRequestStructWrap::Serialize(WORD format,CSegment& seg)
{
	if(format ==  NATIVE )
	{
		seg <<(DWORD)m_ChannelHandle;
	    seg <<(DWORD)m_SequenceNumber;
		seg <<(DWORD)m_NumberOfMediaStream;

		if(m_NumberOfMediaStream > 100)
		{
			PTRACE(eLevelInfoNormal,"_scp_flow_ CMrmpScpStreamsRequestStructWrap::Serialize truncating!!!");

			m_NumberOfMediaStream = 100;
		}

		if ( (m_NumberOfMediaStream > 0) && (NULL != m_mediaStreams) )
		{

			for(int i=0; i<m_NumberOfMediaStream; ++i)
			{
				m_mediaStreams[i].Serialize(format,seg);
			}
		}
	} // end if(format ==  NATIVE )
}

/////////////////////////////////////////////////////////////////////////////
void  CMrmpScpStreamsRequestStructWrap::Print()
{
	TRACEINTOFUNC	<< "\nm_ChannelHandle:       " << m_ChannelHandle
					<< "\nm_SequenceNumber:      " << m_SequenceNumber
					<< "\nm_NumberOfMediaStream: " << m_NumberOfMediaStream;

	if (m_NumberOfMediaStream > 100)
	{
		TRACEINTOFUNC << "m_NumberOfMediaStream(" << m_NumberOfMediaStream << ") > 100";
		m_NumberOfMediaStream = 1;
	}

	if (m_mediaStreams != NULL)
	{
		for (int i=0; i<m_NumberOfMediaStream; ++i)
		{
			m_mediaStreams[i].Print();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void  CMrmpScpStreamsRequestStructWrap::DeSerialize(WORD format,CSegment& seg)
{

	int i;

	if ( (m_NumberOfMediaStream != 0) && (NULL != m_mediaStreams) )
	{
		delete [] m_mediaStreams;
		m_mediaStreams = NULL;
	}

	if(format ==  NATIVE )
	{
		seg >>(DWORD&)m_ChannelHandle;
	    seg >>(DWORD&)m_SequenceNumber;
		seg >>(DWORD&)m_NumberOfMediaStream;

		if(m_NumberOfMediaStream > 100)
		{
			PTRACE(eLevelInfoNormal,"_scp_flow_ CMrmpScpStreamsRequestStructWrap::DeSerialize truncating!!!");
			m_NumberOfMediaStream = 100;
		}

		if (m_NumberOfMediaStream > 0)
		{
			m_mediaStreams=new CMrmpStreamDescWrap[m_NumberOfMediaStream];
			for(i=0; i<m_NumberOfMediaStream; ++i)
			{
				m_mediaStreams[i].DeSerialize(format,seg);
			}
		}

	}
}

/////////////////////////////////////////////////////////////////////////////
CMrmpStreamDescWrap::CMrmpStreamDescWrap()
{
	m_ChannelType=1;
	m_PayloadType=20;
	m_SpecificSourceSsrc=0;
	m_BitRate=800;
	m_FrameRate=200;
	m_Height=50;
    m_Width=20;
	m_PipeIdSsrc=10;
    m_SourceIdSsrc=15;
    m_Priority=0;
    m_isLegal = TRUE;
}

/////////////////////////////////////////////////////////////////////////////
CMrmpStreamDescWrap& CMrmpStreamDescWrap::operator= (const CMrmpStreamDescWrap &other)
{
	m_ChannelType			= other.m_ChannelType;
	m_PayloadType			= other.m_PayloadType;
	m_SpecificSourceSsrc	= other.m_SpecificSourceSsrc;
	m_BitRate				= other.m_BitRate;
	m_FrameRate				= other.m_FrameRate;
	m_Height				= other.m_Height;
	m_Width					= other.m_Width;
	m_PipeIdSsrc			= other.m_PipeIdSsrc;
	m_SourceIdSsrc			= other.m_SourceIdSsrc;
	m_Priority				= other.m_Priority;
    m_isLegal               = other.m_isLegal;

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CMrmpStreamDescWrap& CMrmpStreamDescWrap::operator= (const MrmpStreamDesc &other)
{
    m_ChannelType           = other.unChannelType;
    m_PayloadType           = other.unPayloadType;
    m_SpecificSourceSsrc    = other.unSpecificSourceSsrc;
    m_BitRate               = other.unBitRate;
    m_FrameRate             = other.unFrameRate;
    m_Height                = other.unHeight;
    m_Width                 = other.unWidth;
    m_PipeIdSsrc            = other.unPipeIdSsrc;
    m_SourceIdSsrc          = other.unSourceIdSsrc;
    m_Priority              = other.unPriority;
    m_isLegal               = TRUE;

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CMrmpStreamDescWrap& CMrmpStreamDescWrap::operator= (const StreamDesc &other)
{
    m_ChannelType           = other.m_type;
    m_PayloadType           = other.m_payloadType;
    m_SpecificSourceSsrc    = other.m_specificSourceSsrc;
    m_BitRate               = other.m_bitRate;
    m_FrameRate             = other.m_frameRate;
    m_Height                = other.m_height;
    m_Width                 = other.m_width;
    m_PipeIdSsrc            = other.m_pipeIdSsrc;
    m_SourceIdSsrc          = other.m_sourceIdSsrc;
    m_Priority				= other.m_priority;
    m_isLegal               = other.m_isLegal;

    return *this;
}

/////////////////////////////////////////////////////////////////////////////
void  CMrmpStreamDescWrap::Print()
{
	TRACEINTOFUNC	<< "\nm_ChannelType:        " << m_ChannelType
					<< "\nm_PayloadType:        " << m_PayloadType
					<< "\nm_SpecificSourceSsrc: " << m_SpecificSourceSsrc
					<< "\nm_BitRate:            " << m_BitRate
					<< "\nm_FrameRate:          " << m_FrameRate
					<< "\nm_Height:             " << m_Height
					<< "\nm_Width:              " << m_Width
					<< "\nm_PipeIdSsrc:         " << m_PipeIdSsrc
					<< "\nm_SourceIdSsrc:       " << m_SourceIdSsrc
					<< "\nm_Priority:           " << m_Priority
	                << "\nm_isLegal:              " << m_isLegal;
}

/////////////////////////////////////////////////////////////////////////////
void  CMrmpStreamDescWrap::Serialize(WORD format,CSegment& seg) const
{
	if(format ==  NATIVE )
	{
		seg <<(DWORD)m_ChannelType;
		seg <<(DWORD)m_PayloadType;
	    seg <<(DWORD)m_SpecificSourceSsrc;
		seg <<(DWORD)m_BitRate;
	    seg <<(DWORD)m_FrameRate;
		seg <<(DWORD)m_Height;
	    seg <<(DWORD)m_Width;
		seg <<(DWORD)m_PipeIdSsrc;
	    seg <<(DWORD)m_SourceIdSsrc;
	    seg <<(DWORD)m_Priority;
        seg <<(DWORD)m_isLegal;
	}
}

/////////////////////////////////////////////////////////////////////////////
void  CMrmpStreamDescWrap::DeSerialize(WORD format,CSegment& seg)
{

	if(format ==  NATIVE )
	{
        DWORD tmpIsLegal = 0;

		seg >>(DWORD&)m_ChannelType;
		seg >>(DWORD&)m_PayloadType;
	    seg >>(DWORD&)m_SpecificSourceSsrc;
		seg >>(DWORD&)m_BitRate;
	    seg >>(DWORD&)m_FrameRate;
		seg >>(DWORD&)m_Height;
	    seg >>(DWORD&)m_Width;
		seg >>(DWORD&)m_PipeIdSsrc;
	    seg >>(DWORD&)m_SourceIdSsrc;
	    seg >>(DWORD&)m_Priority;
        seg >>(DWORD&)tmpIsLegal;

        m_isLegal = (bool)tmpIsLegal;

//	    m_FrameRate=::GetFrameRateInSignalingValues(m_FrameRate);
	}
}
